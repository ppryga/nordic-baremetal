/*
 * Copyright (c) 2023 Piotr Pryga
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Scheduler module manages threads queue and schedules execution */

#include <stdint.h>
#include <assert.h>

#include <drivers/include/nrfx_systick.h>

#include "spin_lock.h"
#include "thread.h"
#include "scheduler.h"

/** @brief Free thread objects pool.
 * 
 * Use this pool to store thread object that are ready to be scheduled and executed.
 * If the pool is empty there is no thread to be scheduled.
 */
static slist_t m_thread_ready_pool;

/* Current implementation of Round-robin scheduler is based on ready threads pool.
 *
 * Currently executed thread is stored in global variable g_current_thread.
 * That works for single core scheduler, hence the implementation doesn't support any multicore CPU/SOC.
 *
 * The the head of the ready threads pool is a next thread to be executed. 
 * At very beginning there is only main thread that is started by system initialization code.
 * The main thread is set directly to g_current_thread. It is added to ready threads pool when it
 * is swaped with next thread.
 * 
 * When new thread is created it is appended to end of the ready threads pool.
 * On systick instancts scheduler is executed. The next thread is get from ready pool.
 * Current thread, if not ending, is inserted into ready threads pool again.
 * Then thread swap happens. Then pend_sv interrupt is fired and actuall context switch happens.
 */

thread_t *g_current_thread = NULL;
thread_t *g_next_thread = NULL;
static thread_t *m_idle_thread;

static spin_lock_t m_sched_lock;

/* For debuggin purposes */
uint64_t tick_cnt = 0;

static void sched_threads_waiting_resume(slist_t *wait_queue);
static bool schedule(bool is_ending);

void swap_threads()
{
	g_current_thread->ctx_ptr.status &= (~THREAD_STATUS_ACTIVE);
	g_next_thread->ctx_ptr.status |= THREAD_STATUS_ACTIVE;

	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
	__DSB();
	__ISB();
}

void SysTick_Handler(void)
{
	tick_cnt++;

	/* Lock it to avoid race when other irq happens */
	spin_lock_irq(&m_sched_lock);

	if (schedule(false)) {
		swap_threads();
	}

	/* Spinlock can be released here because if there is a context switch it happens after the systick handler
	 * exits. PendSV interrupt has same or lower priority than Systick, hence is tail-chained but doesn't preempt
	 * the Systick handler.
	 */
	spin_unlock_irq(&m_sched_lock);

	/* Restart timer to switch tasks. Mind that there is a actual thread swap hence the tick may not be to short
	 * to do not get stuck in Systick interrupt.
	 */
	SysTick->LOAD = (0xFFFF);
}

void scheduler_init(thread_t *main_thread, thread_t *idle_thread)
{
	assert(main_thread != NULL);
	assert(idle_thread != NULL);

	slist_init(&m_thread_ready_pool);

	/* Initialize current thread to main_thread. There may not be any thread before call to this function. */
	g_current_thread = main_thread;
	m_idle_thread = idle_thread;

	/* Initialize systic to run round-robin scheduler 
         * 
         * TODO: Change systick to RCT to decrease power consumption.
         */
	nrfx_systick_init();

	//	SCB->SHP[11] = (uint8_t)(0x01 << (8U - __NVIC_PRIO_BITS));
	/* Set long time - just for now. Later change it to configurable time slice. */
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
	SysTick->LOAD = (0x1 << 24);
}

/* @brief Get next thread to execute
 *
 * The function never returns NULL. In case there are no ready threads it returns idle thread.
 */
thread_t *ready_next_get()
{
	slist_node_t *thread_node = slist_head_get(&m_thread_ready_pool);
	if (thread_node == NULL) {
		return m_idle_thread;
	}

	/* Its removed from ready queue, detach it from the queue */
	thread_node->next = NULL;

	return THREAD_OBJECT_GET(thread_node);
}

static thread_t *ready_next_peek()
{
	slist_node_t *thread_node = slist_head_peek(&m_thread_ready_pool);
	if (thread_node != NULL) {
		return THREAD_OBJECT_GET(thread_node);
	}

	return NULL;
}

static thread_t *ready_next_remove()
{
	/* Use get function for head remove to returns the removed therad pointer */
	slist_node_t *thread_node = slist_head_get(&m_thread_ready_pool);

	if (thread_node == NULL) {
		return NULL;
	} else {
		return THREAD_OBJECT_GET(thread_node);
	}
}

static bool schedule(bool is_ending)
{
	thread_t *next_thread = ready_next_peek();

	/* In case there is no new thread in a ready pool and the current thread isn't ending skip swap operation.
	 * If next thread is idle thread is should never end.
	 */
	if (next_thread == NULL && is_ending == false) {
		return false;
	}

	g_next_thread = ready_next_get();
	assert(next_thread == g_next_thread || next_thread == NULL);

	/* Put current thread into ready queue again in case its not ending and not idle thread. */
	if (is_ending == false && g_current_thread != m_idle_thread) {
		/* Put next node at end of rady list for next re-schedule. */
		sched_ready_enqueu(g_current_thread);
	}

	return true;
}

/* @brief The function adds thread to ready threads pool
 *
 * The function doesn't acquire any spin_lock. It must be quarted by caller
 * or used within scheduler context where scheduler lock is used.
 */
void sched_ready_enqueu(thread_t *thread)
{
	slist_tail_put(&m_thread_ready_pool, &thread->list_node);

	thread->ctx_ptr.status |= THREAD_STATUS_WAITING;
}

void sched_ready_remove(thread_t *thread)
{
	slist_node_t *node = slist_head_peek(&m_thread_ready_pool);

	/* If a thread is head of ready threads pool, remove it and return */
	if (node == &thread->list_node) {
		slist_head_remove(&m_thread_ready_pool);

		return;
	}

	/* The thread isn't head of the ready threads pool, check remaining nodes */
	slist_node_t *node_next = slist_next_peek(node);

	do {
		/* If thread is next node in the pool, remove next and return */
		if (node_next == &thread->list_node) {
			slist_next_remove(&m_thread_ready_pool, node);

			return;
		}

		/* Next node becomes current node now. Continue loop until thread is found or we reach end of list */
		node = node_next;
		node_next = slist_next_peek(node_next);
	} while (node_next != NULL);
}

void sched_thread_end(thread_t *thread)
{
	/* A thread function has returned, hence this isn't called from interrupt context.
	 * Anyway execution may be interrupted by e.g. Systick and we are updating sheduling
	 * queues, so we have to lock access to those and disable interrupts until we are done.
	 * At end there is an attempt to swap to new thread.
	 */
	/* Lock it to avoid race when other irq happens */
	spin_lock_irq(&m_sched_lock);

	/* Rresume all threads that waited (called thread_join) on this thread */
	sched_threads_waiting_resume(&thread->wait_queue);

	/* Cleanup thread object and add it to free threads pool */
	thread->list_node.next = NULL;

	/* When we end current thread, the re-schedule is mandatory, in other case just remove the thread from
	 * ready threads pool.
	 */
	if (thread == g_current_thread) {
		bool swap = schedule(true);

		if (swap) {
			swap_threads();
		}
	} else {
		sched_ready_remove(thread);
	}

	/* TODO: this is wrong. There is a release of a thread that is still current thread.
	 * It could be ended and alive at once if the thread is current thread. It works because contents of context
	 * are not cleaned up before swap happens.
	 */
	thread_free_put(thread);
	thread->ctx_ptr.status |= THREAD_STATUS_ENDED;

	/* Unlock the IRQ to take pending thread swap. There is no point of return to this function after
	 * we unlock IRQs and PendSV is taken.
	 */
	spin_unlock_irq(&m_sched_lock);
}

static void sched_threads_waiting_resume(slist_t *wait_queue)
{
	assert(wait_queue);

	slist_node_t *waiting_thread_node = slist_head_get(wait_queue);
	thread_t *waiting_thread;

	while (waiting_thread_node != NULL) {
		waiting_thread = THREAD_OBJECT_GET(waiting_thread_node);
		waiting_thread->ctx_ptr.status &= (~THREAD_STATUS_WAITING);

		waiting_thread_node->next = NULL;

		sched_ready_enqueu(waiting_thread);
		waiting_thread_node = slist_head_get(wait_queue);
	}
}

thread_t *sched_current_thread_get()
{
	return g_current_thread;
}

void sched_thread_join(thread_t *thread)
{
	/* Lock it to avoid race when other irq happens */
	spin_lock_irq(&m_sched_lock);

	/* Put current thread into wait queue of thread to join */
	slist_tail_put(&thread->wait_queue, &g_current_thread->list_node);

	bool swap = schedule(false);

	/* It would be wired that swap is false here. It would mean there is no other ready thread and we join theread
	 * that is waiting for other thread or event and there is no idle thread. May not happen!
	 */
	if (swap == true) {
		swap_threads();
	}

	thread->ctx_ptr.status |= THREAD_STATUS_WAITING;

	/* Unlock irqs to take PendingSV to swap threads. If returns here the thread has been woken up from wait and
	 * thread to join has ended.
	 */
	spin_unlock_irq(&m_sched_lock);
}
