/*
 * Copyright (c) 2023 Piotr Pryga
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <drivers/nrfx_common.h>

#include "thread.h"
#include "scheduler.h"
#include "spin_lock.h"
#include "../tools/misc.h"
#include "../tools/slist.h"

#define THREAD_MAX_NUM 4
#define THREAD_MAX_TOTAL THREAD_MAX_NUM + 2 /* Add main and Idle threads to total count */

#define THREAD_DEBUG_ENABLED 1 /* TODO move into KConfig in future */

/* Threads are statically allocated but they are not accessed directly in array.
 * Threads are formed into an single linked list to give fast access to unused instances.
 */
static thread_t m_thread[THREAD_MAX_TOTAL];

/** @brief Free thread objects pool.
 * 
 * Use this pool to get new thread object to be returned to thread create caller.
 * If the pool is exhausted new thread can't be created.
 * Put a thread object to the pool when thread ends or is aborted.
 * 
 * @note The number of instnces in the pool must match number of contexts in m_free_ctx_pool.
 */
static slist_t m_free_thread_pool;

static thread_t *m_idle_thread;
#define IDLE_STACK_SIZE                                                                            \
	(FUNCTION_FRAME_SIZE_TOTAL +                                                               \
	 128) /* Except frame size added small amount of memory just in case */
THREAD_STACK_STATIC(idle_thread, IDLE_STACK_SIZE);

/* Debug variable */
#ifdef THREAD_DEBUG_ENABLED
static thread_t *m_main_thread;
static uint32_t m_thread_end_count;
#endif /* THREAD_DEBUG_ENABLED */

static void m_thread_cleanup();
static thread_t *main_thread_init();
static void thread_ctx_init(thread_ctx_t *ctx, thread_handler_t handler, stack_ptr_t stack_ptr,
			    uint32_t stack_size);
static void idle_thread();
static int idle_thread_init();

/* @brief this functiun should not be calle anywhere. It is just a place holder for asm inline.
 *
 * It creates globaly accessible data that store offsets of some of members in thread related types.
 * These global data are accessible in assembly files like one that is responsible for thread swap.
 * 
 * This is a workaround to make generation of offets to be done on build time and make possible
 * to use those from assembler code. For "C" code use offsetof() instead.
 */
void __thread_symbols_offsets()
{
	GEN_ASM_OFFSET_SYM(thread_t, ctx_ptr);
	GEN_ASM_OFFSET_NESTED_SYM(thread_t, ctx_ptr, stack_ptr);
}

static void m_thread_cleanup()
{
#ifdef THREAD_DEBUG_ENABLED
	m_thread_end_count++;
#endif /* THREAD_DEBUG_ENABLED */

	thread_t *current = sched_current_thread_get();

	if ((current->ctx_ptr.status & THREAD_STATUS_ENDED) != 0) {
		/* Attempt to end already dead thread - maybe assert? */
		return;
	}

	current->ctx_ptr.status |= THREAD_STATUS_ENDED;

	sched_thread_end(current);

	/* We can't get back here. In such case there were no thread swap done. */
	assert(false);

	while (1) {
	}
}

static thread_t *main_thread_init()
{
	/* Lock is not needed here because this must be called from system initialization code,
	 * hence no thread switching may happen.
	 */
	slist_node_t *thread_node = slist_head_get(&m_free_thread_pool);
	if (thread_node == NULL) {
		return NULL;
	}

	thread_t *thread = THREAD_OBJECT_GET(thread_node);
	thread_ctx_t *ctx = &thread->ctx_ptr;
	/* Main thread stack is not stored at init because the main thread is run first
	 * by startup code, not by scheduler. Any value put here, will be overwritten
	 * by first scheduler call when main thread is preempted and stack stored before 
	 * context switch.
	 */
	ctx->stack_ptr = NULL;
	ctx->status = THREAD_STATUS_ACTIVE;

	/* Setup firts executing thread */
	thread_node->next = NULL;

	return thread;
}

static void idle_thread()
{
	/* Currently does nothing except busy looping until interrupted and switched to other thread */
	while (1) {
	};
}

static int idle_thread_init()
{
	slist_node_t *idle_thread_node = slist_head_get(&m_free_thread_pool);
	if (idle_thread_node == NULL) {
		return -ENOMEM;
	}

	m_idle_thread = THREAD_OBJECT_GET(idle_thread_node);
	thread_ctx_t *idle_ctx = &m_idle_thread->ctx_ptr;
	assert(idle_ctx != NULL);

	thread_ctx_init(idle_ctx, idle_thread, stack_idle_thread, sizeof(stack_idle_thread));

	idle_thread_node->next = NULL;

	idle_ctx->status &= (~THREAD_STATUS_STARTING);
	/* What flad to use for idle stack that is ready but not in a ready threads pool? */
	return 0;
}

static void thread_ctx_init(thread_ctx_t *ctx, thread_handler_t handler, stack_ptr_t stack_ptr,
			    uint32_t stack_size)
{
	/* Stack if filled bottom-up. On create there is stored initail function frame so
	 * adjust actual pointer to avoid overwrite it. The function frame is expected by 
	 * scheduler. The SP will point to end of initial function frame.
	 */
	ctx->stack_ptr = (uint32_t *)(stack_ptr + stack_size - FUNCTION_FRAME_SIZE_TOTAL);

	/* Hardware stored part of stack frame is 8 registers from bottom of the frame
	 * to point to R0 in HW stored frame */
	hw_function_frame_t *hw_frame =
		(hw_function_frame_t *)(stack_ptr + stack_size - FUNCTION_FRAME_HW_STORED_SIZE);

	hw_frame->xpsr = 0x01000000;
	hw_frame->pc = (uint32_t)handler;
	hw_frame->lr = (uint32_t)m_thread_cleanup; /* Return to thread mode with PSP */
#if defined(THREAD_DEBUG_ENABLED)
	hw_frame->r12 = 0xFF0C;
	hw_frame->r3 = 0xFF03;
	hw_frame->r2 = 0xFF02;
	hw_frame->r1 = 0xFF01;
	hw_frame->r0 = 0xFF00;
#endif /* THREAD_DEBUG_ENABLED */

	/* SW function frame is at top of the stack */
	sw_function_frame_t *sw_frame = (sw_function_frame_t *)ctx->stack_ptr;
	/* It must be set here, because initial thread handler frame must be correctly formed.
	 * This value is used by the context switch exception to return from handler.
	 */
	sw_frame->r14 = 0xFFFFFFFD;
#if defined(THREAD_DEBUG_ENABLED)
	sw_frame->r11 = 0xFF0B;
	sw_frame->r10 = 0xFF0A;
	sw_frame->r9 = 0xFF09;
	sw_frame->r8 = 0xFF08;
	sw_frame->r7 = 0xFF07;
	sw_frame->r6 = 0xFF06;
	sw_frame->r5 = 0xFF05;
	sw_frame->r4 = 0xFF04;
#endif /* THREAD_DEBUG_ENABLED */

	ctx->status = THREAD_STATUS_STARTING;
}

int thread_init()
{
	/* Lock is not needed here because this must be called from system initialization code,
	 * hence no thread switching may happen.
	 */
	slist_init(&m_free_thread_pool);

	/* Put all contexts into a free context pool*/
	for (int idx = 0; idx < THREAD_MAX_TOTAL; idx++) {
		m_thread[idx].ctx_ptr.status = THREAD_STATUS_NONE;
		slist_init(&m_thread[idx].wait_queue);
		slist_tail_put(&m_free_thread_pool, &m_thread[idx].list_node);
	}

	thread_t *main_thread = main_thread_init();
	if (main_thread == NULL) {
		return -ENOMEM;
	}

#ifdef THREAD_DEBUG_ENABLED
	m_main_thread = main_thread;
#endif /* THREAD_DEBUG_ENABLED */

	idle_thread_init();

	scheduler_init(main_thread, m_idle_thread);
	return 0;
}

int thread_create(thread_t **thread, thread_handler_t handler, stack_ptr_t stack_ptr,
		  uint32_t stack_size)
{
	assert(handler);
	assert(stack_ptr);
	assert(stack_size != 0);

	/* TODO: Add lock here, there is possible race while getting context from multiple execution contexts */
	slist_node_t *thread_node = slist_head_get(&m_free_thread_pool);

	if (thread_node == NULL) {
		return -ENOMEM;
	}

	thread_t *new_thread = THREAD_OBJECT_GET(thread_node);
	thread_ctx_t *ctx = &new_thread->ctx_ptr;
	assert(ctx->status == THREAD_STATUS_NONE);

	thread_ctx_init(ctx, handler, stack_ptr, stack_size);

	thread_node->next = NULL;
	sched_ready_enqueu(new_thread);

	*thread = new_thread;

	ctx->status &= (~THREAD_STATUS_STARTING);

	return 0;
}

int thread_join(thread_t *thread)
{
	if (thread->ctx_ptr.status & (THREAD_STATUS_NONE | THREAD_STATUS_ENDED)) {
		return 0;
	} else if (thread == sched_current_thread_get()) {
		/* Can't join active thread from itself. That is not allowed due to deadlock.
		 */
		return -EDEADLK;
	}

	/* TODO: check if call isn't from ISR */
	sched_thread_join(thread);

	return 0;
	/* TODO: in future add timeout: sleeping queue for wakeups and system clock to keep passing time. */
}

void thread_free_put(thread_t *thread)
{
	slist_tail_put(&m_free_thread_pool, &thread->list_node);
}
