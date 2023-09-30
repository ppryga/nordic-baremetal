/*
 * Copyright (c) 2023 Piotr Pryga
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef __SYS_IRQ_H__
#define __SYS_IRQ_H__

static void irq_disable()
{
	asm volatile("       cpsid   i"
		     : /* no output */
		     : /* no input */
		     : "memory", "cc");
}

static void irq_enable()
{
	asm volatile("       cpsie   i"
		     : /* no output */
		     : /* no input */
		     : "memory", "cc");
}

static uint32_t irq_disable_store()
{
	uint32_t flags;

	asm volatile("       mrs     %[flags], primask\n\t"
		     "       cpsid   i"
		     : [flags] "=r"(flags)
		     : /* no input */
		     : "memory", "cc");

	return flags;
}

static void irq_enable_restore(uint32_t flags)
{
	asm volatile("       msr     primask, %[flags]"
		     : /* no output */
		     : [flags] "r"(flags)
		     : "memory", "cc");
}

#endif /* __SYS_IRQ_H__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */
