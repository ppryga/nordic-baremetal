/*
 * Copyright (c) 2023 Piotr Pryga
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mutex.h"

void mutex_init(mutex_t *mutex)
{
	/* There is an assumption that there is no value store in the mutex hence ordinary store is
	 * enough. */
	asm("1:     LDR     r1, =%[lock_const]\n\t"  /* Load MUTEX_LOCKED into r1 */
	    "       LDREX   r2, [%[lock_value]]\n\t" /* Load-ex value of the mutext stored in memory
						      */
	    "       CMP     r1, r2\n\t"		     /* Compare loaded data to lock constant */
	    "       ITTE    NE\n\t" /* If not locked - then execute next two instructions */
	    "       STREXNE r2, r1, [%[lock_value]]\n\t" /*      - Store-ex locked value in mutex
							    memory */
	    "       CMPNE   r2, #1\n\t" /*      - Check result of store operation. */
	    "       BEQ     2f\n\t"	/* else
					 *      - jump to label 2.
					 */
	    "       IT      EQ\n\t"	/* Mind that this if-then block refers to CMPNE above,
					 * the BEQ is an else in former if-then-else.
					 */
	    "       BEQ     1b\n\t"
	    "       DMB\n\t"
	    "       BX      LR\n\t"
	    "2:     WFI\n\t"
	    "       B       1b\n\t"
	    :
	    : [lock_const] "i"(MUTEX_LOCKED), [lock_value] "r"(&mutex->lock)
	    :);
}

void mutex_lock(mutex_t *mutex)
{
	asm("1:     LDR     r1, =%[lock_const]\n\t"  /* Load MUTEX_LOCKED into r1 */
	    "       LDREX   r2, [%[lock_value]]\n\t" /* Load-ex value of the mutext stored in memory
						      */
	    "       CMP     r1, r2\n\t"		     /* Compare loaded data to lock constant */
	    "       ITTE    NE\n\t" /* If not locked - then execute next two instructions */
	    "       STREXNE r2, r1, [%[lock_value]]\n\t" /*      - Store-ex locked value in mutex
							    memory */
	    "       CMPNE   r2, #1\n\t" /*      - Check result of store operation. */
	    "       BEQ     2f\n\t"	/* else
					 *      - jump to label 2.
					 */
	    "       IT      EQ\n\t"	/* Mind that this if-then block refers to CMPNE above,
					 * the BEQ is an else in former if-then-else.
					 */
	    "       BEQ     1b\n\t"
	    "       DMB\n\t"
	    "       BX      LR\n\t"
	    "2:     WFI\n\t"
	    "       B       1b\n\t"
	    :
	    : [lock_const] "i"(MUTEX_LOCKED), [lock_value] "r"(&mutex->lock)
	    : "r1", "r2");
}

void mutex_unlock(mutex_t *mutex)
{
}