/*
 * Copyright (c) 2023 Piotr Pryga
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef __SYS_SPIN_LOCK_H__
#define __SYS_SPIN_LOCK_H__
typedef enum SYS_SPIN_LOCK_STATE {
	SPIN_LOCK_UNLOCKED,
	SPIN_LOCK_LOCKED,
} SPIN_LOCK_STATE_T;

typedef struct sys_spin_lock {
	uint32_t lock;
} spin_lock_t;

/* @brief Acquire spin lock, call WFE in case of wait
 *
 * This function may not be used in IRQ context. It puts CPU into WFE state in case it can't acquire the lock.
 * That would froze CPU until extenal IRQ happens and cause a deadlock if called when IRQs are disabled.
 * 
 * Function returns when the spin lock is acquired.
 * 
 * @param lock Pointer to spin lock instance to be acquired
 */
void spin_lock(spin_lock_t *lock);

/* @brief Release spin lock
 *
 * @param lock Pointer to spin lock instance to be released
 */
void spin_unlock(spin_lock_t *lock);

/** @brief Disable a interrupts and acruire a spin lock
 * 
 * Function steps are executed in sqequence: disable IRQs then acquire the spin lock.
 * That has consequences, an attempt to recursive lock of a spin lock will deadlock.
 *
 * @param lock Pointer to spin lock instance to be released
 *
 */
void spin_lock_irq(spin_lock_t *lock);

/** @brief Release a spin lock and enable a interrupts
 * 
 * It doesn't call SEV, so will not wakeup context waiting in spin_lock().
 *
 * @param lock Pointer to spin lock instance to be released
 *
 */
void spin_unlock_irq(spin_lock_t *lock);

/** @brief Disable a interrupts, acruire a spin lock and return state of interrupt mask.
 * 
 * Function steps are executed in sqequence: disable IRQs then acquire the spin lock.
 * That has consequences, an attempt to recursive lock of a spin lock will deadlock.
 *
 * @param lock Pointer to spin lock instance to be released
 * 
 * @return uint32_t State of interrupts mask
 */
uint32_t spin_lock_irq_store(spin_lock_t *lock);

/** @brief Release a spin lock, enable interrupts and restore state of interrupt mask.
 * 
 * It doesn't call SEV, so will not wakeup context waiting in spin_lock().
 *
 * @param lock Pointer to spin lock instance to be released
 * @param flags State of interrupts mask to restore
 */
void spin_unlock_irq_restore(spin_lock_t *lock, uint32_t flags);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SYS_SPIN_LOCK_H__ */
