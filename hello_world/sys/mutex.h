/*
 * Copyright (c) 2023 Piotr Pryga
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum MUTEX_STATE {
	MUTEX_UNLOCKED,
        MUTEX_LOCKED
} MUTEX_STATE_T;

typedef struct sys_mutex {
	uint32_t lock; /* ARM7-M TRM requires the muthex lock to be 4 bytes aligned */
} mutex_t __attribute__((aligned(4)));

void mutex_init(mutex_t *mutex);
void mutex_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);

#ifdef __cplusplus
}
#endif /* __cplusplus */
