/*
 * Copyright (c) 2023 Piotr Pryga
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "irq.h"
#include "spin_lock.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** This spin lock can't be used by interrupts because it can hang indefinitely */
void spin_lock(spin_lock_t *lock)
{
	asm("1:     movs    r2, %[sp_locked]\n\t"
	    "       ldrex   r1, [%[lock]]\n\t" /* Load sinlock value */
	    "       teq     r1, r2\n\t" /* Check if it is locked */
	    "       itee    eq\n\t" /* If it is locked:*/
	    "       wfeeq\n\t" /*      - wait for other thread to release lock */
	    "       strexne r3, r2, [%[lock]]\n\t" /* else:
						    *      - attempt to store the value */
	    "       teqne   r3, #1\n\t" /*      - check store result */
	    /*      - If value was not stored:*/
	    "       beq     1b\n\t" /*              - branch to beginning */
	    "       dmb\n\t"
	    :
	    : [lock] "r"(&lock->lock), [sp_locked] "i"(SPIN_LOCK_LOCKED)
	    : "cc", "r1", "r2", "r3", "memory");
}

void spin_unlock(spin_lock_t *lock)
{
	asm("       dmb\n\t"
	    "       str    %[sp_unlocked], [%[lock]]\n\t"
	    "       dsb\n\t"
	    "       sev\n\t"
	    :
	    : [lock] "r"(&lock->lock), [sp_unlocked] "r"(SPIN_LOCK_UNLOCKED)
	    : "cc", "memory");
}

/** @brief Acquire a spinlock but doen't call WFE in case of a wait.
 * 
 * This function is desired to be used in IRQ context. It executes real busy loop instead of use of WFE. 
 * Example of such csase is a single processor system and an attempt to acquire the lock from an IRQ. 
 * In such case IRQs are disabled and puitting the CPU in a sleeep mode would create a deadlock.
 * 
 * @pram lock Pointer to instance of a spin lock that is going to be locked.
 */
static void spin_lock_no_wfe(spin_lock_t *lock)
{
	asm("1:     movs    r2, %[sp_locked]\n\t"
	    "       ldrex   r1, [%[lock]]\n\t" /* Load sinlock value */
	    "       teq     r1, r2\n\t" /* Check if it is locked */
	    "       beq	    1b\n\t" /* If it is locked:*/
	    "       strex   r3, r2, [%[lock]]\n\t" /* else:
						    *      - attempt to store the value */
	    "       teq     r3, #1\n\t" /*      - check store result */
	    /*      - If value was not stored:*/
	    "       beq     1b\n\t" /*              - branch to beginning */
	    "       dmb\n\t"
	    :
	    : [lock] "r"(&lock->lock), [sp_locked] "i"(SPIN_LOCK_LOCKED)
	    : "cc", "r1", "r2", "r3", "memory");
}

void spin_lock_irq(spin_lock_t *lock)
{
	irq_disable();
	spin_lock_no_wfe(lock);
}

/** @brief Lock a spinlock but doen't call SEV.
 * 
 * This function is desired to be used in case of a spin lock was acquired with spin_lock_no_wfe.
 * In such case there is no need to call SEV because a CPU can't be wainting for an event.
 * 
 * @pram sp_lock Pointer to instance of a spin lock that is going to be locked.
 */
void spin_unlock_no_sev(spin_lock_t *lock)
{
	asm("       dmb\n\t"
	    "       str    %[sp_unlocked], [%[lock]]\n\t"
	    "       dsb\n\t"
	    :
	    : [lock] "r"(&lock->lock), [sp_unlocked] "r"(SPIN_LOCK_UNLOCKED)
	    : "cc", "memory");
}

void spin_unlock_irq(spin_lock_t *sp_lock)
{
	spin_unlock_no_sev(sp_lock);
	irq_enable();
}

uint32_t spin_lock_irq_store(spin_lock_t *lock)
{
	uint32_t flags;

	flags = irq_disable_store();
	spin_lock(lock);

	return flags;
}

void spin_unlock_irq_restore(spin_lock_t *lock, uint32_t flags)
{
	spin_unlock(lock);
	irq_enable_restore(flags);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
