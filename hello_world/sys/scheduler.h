/*
 * Copyright (c) 2023 Piotr Pryga
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SYS_SCHEDULER_H__
#define __SYS_SCHEDULER_H__

#include "../tools/slist.h"

struct thread_t;

/* @brief Initialize scheduler
 *
 * The function is responsible for scheduler initialization. It expectst to get pointes to main thread and idle thread.
 * Idle thread is used in case there is no other thread that is ready to run and current one is ending.
 * 
 * @param main_thread Pointer to main thread object
 * @param idle_thread Pointer to idle thread object
 */
void scheduler_init(thread_t *main_thread, thread_t *idle_thread);

/* @brief Cleanup of an ending thread in scheduler
 * 
 * The function does end of a thread processing in scheudler. If ending thread is current thread it will swap it to next
 * ready thread.
 * 
 * @param thread Pointer to ending thread
 * 
 */
void sched_thread_end(thread_t *thread);

/* @brief Join a particular thread 
 *
 * The function executes join operation to a thread. The calling thread that is current thread, will be put into wait
 * queue of the thread. Then current thread is swapped. When the thread ends the waiting thread will be rescheduled.
 * 
 * The function returns when the joined thread is ended
 * 
 * @param thread Pointer to thread to join to.
 */
void sched_thread_join(thread_t *thread);

/* @brief Add a thread to a ready threads pool
 *
 * @param thread Pointer to thread object to add to ready threads pool
 */
void sched_ready_enqueu(thread_t *thread);

/* @brief Get current thread
 *
 * @return Pointer to current thread object
 */
thread_t *sched_current_thread_get();

#endif /* __SYS_SCHEDULER_H__ */
