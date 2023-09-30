/*
 * Copyright (c) 2023 Piotr Pryga
 *
 * SPDX-License-Identifier: Apache-2.0
 */

    .syntax unified
    .arch armv7e-m

    @  .equ   FPU_ADDR, FPU
    @  .equ   FPCCR_OFFSET, 0x4 /* CMSIS doesn't provide this value as a macro. Its only commend in FPU_Type */
    @  .equ   FPCCR_ADDR, FPU_ADDR + FPCCR_OFFSET

    .global PendSV_Handler
    .type   PendSV_Handler, %function
PendSV_Handler:
    /* Disable interrupts for time of thread context switch */
    cpsid   i

    /* Context part saved bu CPU on current stack
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
     * Context part saved by exception handler
     * +------+
     * |  R14 |
     * |  R11 |
     * |  R10 |
     * |  R9  |
     * |  R8  |
     * |  R7  |
     * |  R6  |
     * |  R5  |
     * |  R4  | <- top of stack (SP points here)
     * |      | <- After end of frame storage
     * +------+
     */

    /* When a thread context switch happens last PSP should be pointing to the interrupted thread SP. 
     *
     * NOTE: Other possibility is to use stack pointer stored in thread context, but if SP is changed to something 
     *       else what with other registers state?
     */
    mrs     r0, psp
    isb

    stmdb   r0!,{r4-r11, r14}

    /* Get current thread */
    ldr     r1, =g_current_thread
    ldr     r2, [r1]
    /* Get SP in thread context */
    ldr     r1, =__thread_t_ctx_ptr_stack_ptr_OFFSET
    str     r0, [r2, r1]

    /* Load next thread SP from its context */
    ldr     r1, =g_next_thread;
    ldr     r2, [r1]
    /* Get SP stored in thread context */
    ldr     r1, =__thread_t_ctx_ptr_stack_ptr_OFFSET
    ldr     r0, [r2, r1]

    /* Restore context saved by exception handler */
    ldmia   r0!, {r4-r11, r14}
    msr     psp, r0

    /* Store new current thread, use only register r0-r3 */
    ldr     r3, =g_current_thread
    ldr     r2, =g_next_thread
    ldr     r2, [r2]
    str     r2, [r3]

    /* Enable interruprs */
    cpsie   i

    /* Return from exception, that means LR has to hold one of predefined values.
     *
     * NOTE: Use of LR here requires the value or r14(LR) stored in past on stack is correct one including FPU use.
     * That puts a requirement that the initial stack of a therad is prepared in correct way. The SW stored part
     * has to setup correct EXC_RETURN code.
     */
    bx      lr
    .size   PendSV_Handler, . - PendSV_Handler
