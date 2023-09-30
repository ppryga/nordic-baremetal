/*
 * Copyright (c) 2023 Piotr Pryga
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SYS_THREAD_H__
#define __SYS_THREAD_H__

#include <stdint.h>
#include <stdbool.h>

#include "../tools/to_string.h"
#include "../tools/slist.h"
#include "../tools/misc.h"

/* Defult value of stack size for new threads */
#define THREAD_STACK_SIZE 1024

/* Default value of stack size for main thread */
#define MAIN_THREAD_STACK_SIZE 512

#define FUNCTION_FRAME_HW_STORED_SIZE 32 /* 8 registers */
#define FUNCTION_FRAME_SW_STORED_SIZE 36 /* 9 registers */
#define FUNCTION_FRAME_SIZE_TOTAL (FUNCTION_FRAME_HW_STORED_SIZE + FUNCTION_FRAME_SW_STORED_SIZE)

#define THREAD_STACK_STATIC(name, size)                                                            \
	static uint8_t stack_##name[size]                                                          \
		__attribute__((section(".stack." TO_STRING(name)), aligned(8)))

/* Structure describing function frame stored on a stack by Cortex-M core when
 * exception attempted to be handled.
 *
 * Stack after store of registers by hardware:
 * +------+
 * |      | Initial stack pointer
 * | xPSR |
 * |  PC  |
 * |  LR  |
 * |  R12 |
 * |  R3  |
 * |  R2  |
 * |  R1  |
 * |  R0  | <- Stack pointer after enter the exception handler
 * +------+
 *
 * NOTE: See that registers in struct definition are in revers order, stack grops towards lower
 * addresses.
 */
typedef struct {
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t pc;
	uint32_t xpsr;
} hw_function_frame_t;

/* Structure describing function frame stored on a stack by software, just after
 * a Cortex-M core stored @see hw_function_frame contents.
 *
 * Stack after store of registers by exception handler on exception entry:
 * +------+
 * |  R4  |
 * |  R5  |
 * |  R6  |
 * |  R7  |
 * |  R8  |
 * |  R9  |
 * |  R10 |
 * |  R11 |
 * |  R14 | <- Stack pointer at end of frame storage
 * +------+
 *
 * NOTE: See that registers in struct definition are in revers order, stack grops towards lower
 * addresses.
 */
typedef struct {
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t r14;
} sw_function_frame_t;

typedef void (*thread_handler_t)(void);
typedef uint8_t *stack_ptr_t;

typedef enum {
	/* Contex for the status and other resources are released */
	THREAD_STATUS_NONE = BIT(0),
	/* Thread is under initialization, not yet queued into ready pool */
	THREAD_STATUS_STARTING = BIT(1),
	/* Thread was initialized and is queued in ready pool */
	THREAD_STATUS_READY = BIT(1),
	/* Thread is currently executing */
	THREAD_STATUS_ACTIVE = BIT(2),
	/* Thread is waiting for an event */
	THREAD_STATUS_PENDING = BIT(3),
	/* Thread is waiting in a join another thread */
	THREAD_STATUS_WAITING = BIT(4),
	/* Thread had ended */
	THREAD_STATUS_ENDED = BIT(5),
	THREAD_STATUS_MAX
} THREAD_STATUS_T;

typedef struct sys_thread_ctx {
	volatile uint32_t *stack_ptr;
	THREAD_STATUS_T status;
	slist_node_t list_node;
} thread_ctx_t;

typedef uint32_t sys_thread_id_t;
typedef struct sys_thread {
	/* Thread context data, these are internal information that can change without API version update. */
	thread_ctx_t ctx_ptr;
	/* Wait queue for threads that can called thread_join() */
	slist_t wait_queue;
	sys_thread_id_t id;
	slist_node_t list_node;
} thread_t;

#define THREAD_T_CTX_PTR_OFFSET offsetof(thread_t, ctx_ptr)
#define THREAD_CTX_T_STACK_PTR_OFFSET offsetof(thread_ctx_t, stack_ptr)

/** @brief Get thread context that is container for list_node, slist_node_t pointer.
 *
 * @param thread_node_ptr Pointer to a slist_node_t that container is going to be extrated.
 */
#define THREAD_CONTEXT_GET(thread_node_ptr) CONTAINER_OF(thread_node_ptr, thread_ctx_t, list_node)

#define THREAD_OBJECT_GET(thread_node_ptr) CONTAINER_OF(thread_node_ptr, thread_t, list_node)

/* @brief Initialize threads subsystem
 * 
 * @return 0 Subsystem initialized successfully
 *         -ENOMEM Not enough memory for initialization
 */
int thread_init();

/* @brief Ceate a new thread
 *
 * @param [out] thread Pointer to store a pointer to created thread object
 * @param handler Thread function
 * @param stack_ptr Pointer to thread stack
 * @param stack_size Size of the thread stack
 * 
 * @return 0 Thread created
 *         -ENOMEM Not enough memory to allocate new thread object
 */
int thread_create(thread_t **thread, thread_handler_t handler, stack_ptr_t stack_ptr,
		  uint32_t stack_size);

/* @brief Join thread 
 * 
 * Function returns when the thread ends. In case it is still running the current thread is put into waiting queue and
 * swapped. 
 * 
 * @param thread Pointer to thread to join
 */
int thread_join(thread_t *thread);

/* @brief Put a thread into free thread objects pool 
 *
 * @param thread Pointer to thread to store in free thread objects pool
 */
void thread_free_put(thread_t *thread);

#endif /* __SYS_THREAD_H__ */
