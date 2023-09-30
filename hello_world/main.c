/*
 * Copyright (c) 2023 Piotr Pryga
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <drivers/include/nrfx_systick.h>
#include <drivers/nrfx_common.h>
#include <drivers/nrfx_errors.h>

#include "drivers/uart.h"

#include "sys/thread.h"
#include "sys/spin_lock.h"

/* TODO check why globals are not cleaned or initialized */
THREAD_STACK_STATIC(thread1, THREAD_STACK_SIZE);
THREAD_STACK_STATIC(thread2, THREAD_STACK_SIZE);

uint32_t thread1_entry_counter;
uint32_t thread2_entry_counter;

uint64_t test = 0xFFFFFFFFFFFFFFFF;
typedef struct test_32_align_2{
	uint8_t move_alignment[3];
	uint32_t test_32;
} __attribute__((packed)) test_32_align_2_t;

test_32_align_2_t test_32 = { .move_alignment = { 0xEE, 0xEE, 0xEE }, .test_32=0xFFFFFFFF };

uint32_t read_value_32;

uint64_t read_value;

spin_lock_t lock = { .lock = 0x0 };

void test_load_64bit()
{
	asm(
		"ldrd	r0, r1, [%[test_value]]\n\t" : : [test_value] "r"(&test)
	);

	if (test != 0xFFFFFF00000000 && test!=0x506070801020304) {
		printf("Thread 1 read: %lld\r\n", test);
	}
}

void test_load_32bit()
{
	// asm(
	// 	"ldr	%[read_value_32], [%[test_value]]\n\t" :[read_value_32] "=r"(read_value_32) : [test_value] "r"(&test_32.test_32)
	// );
	read_value_32 = test_32.test_32;

	if (read_value_32 != 0xFFFFFFFF && read_value_32 != 0x01020304) {
		printf("Thread 1 read: %lx\r\n", test_32.test_32);
	}
}
bool clear = false;

void test_store_64bit()
{
	if (clear) {
		asm(
			"ldr	r0, =0x00\n\t"
			"ldr	r1, =0xFFFFFFFF\n\t"
			"strd	r0, r1, [%[test_value]]\n\t" :: [test_value] "r" (&test)
		);
		clear = false;
	} else {
		asm(
			"ldr	r0, =0x01020304\n\t"
			"ldr	r1, =0x05060708\n\t"
			"strd	r0, r1, [%[test_value]]\n\t" :: [test_value] "r" (&test)
		);
		clear = true;
	}
}

static uint32_t g_values[] =
    {
        0x37b91364u,
        0x1c5970efu,
        0x536c76bau,
        0x0a10207fu,
        0x71043c77u,
        0x4db84a83u,
        0x27cf0273u,
        0x74a15a69u,
    };
static uint32_t g_limit = 0xffffff00u;

void test_store32()
{
	uint32_t value = 0xFAFA;
	uint32_t index = 0;
        
	for (int i = 0; i < 10000000; i++)
        {
            index = (index * index + 1) % 65521;            // Next pseudorandom index
            //value = g_values[index & 7];                    // Value to store

            test_32.test_32 = value;                        // Nonatomic store
        }
}

void test_load32()
{
	uint32_t value;

	for (int i = 0; i < 10000000; i++)
        {
            value = test_32.test_32;                 // Nonatomic load

            if (value * value < g_limit)
		printf("Thread 1 read: %lx\r\n", test_32.test_32);
        }
}

void test_store_32bit()
{
	if (clear) {
		// asm(
		// 	"ldr	r0, =0xFFFFFFFF\n\t"
		// 	"str	r0, [%[test_value]]\n\t" :: [test_value] "r" (&test_32.test_32)
		// );
		test_32.test_32 = 0xFFFFFFFF;
		clear = false;
	} else {
		// asm(
		// 	"ldr	r0, =0x01020304\n\t"
		// 	"str	r0, [%[test_value]]\n\t" :: [test_value] "r" (&test_32.test_32)
		// );
		test_32.test_32 = 0x01020304;
		clear = true;
	}
}

#define barrier() asm volatile("":::"memory")

int a = 1;

void foo()
{
	while(a)
	barrier();
}

void bar()
{
	a = 0;
}

void thread_1()
{
	uint32_t counter = 0;
	uint32_t flags;

	while (1) {
		for (int idx = 0; idx < 100000; idx++) {
			/* temporary delay loop */
		}

		flags = spin_lock_irq_store(&lock);

		thread1_entry_counter++;

		printf("Thread 1: %ld\r\n", thread1_entry_counter);

		/* After 100 repetitions break the loop and end thread. */		
		if (counter >= 100) {
			spin_unlock_irq_restore(&lock, flags);
			break;
		} else {
			counter++;
		}
		spin_unlock_irq_restore(&lock, flags);
	}
}

void thread_2()
{
	uint32_t flags;

	bar();

	while (1) {
		flags = spin_lock_irq_store(&lock);

		for (int idx = 0; idx < 100000; idx++) {
			/* temporary delay loop */
		}
		
		thread2_entry_counter++;
		printf("Thread 2: %ld\r\n", thread2_entry_counter);

		spin_unlock_irq_restore(&lock, flags);
	}
}

/* Done global for debugging purposes, to make it visible no matter of execution context. */
thread_t *thr_1, *thr_2;

int main(void)
{

	test_store32();
	test_load32();

	/* Needed some debug outputs, so went for uart. */
	uarte_init();

	memset(stack_thread1, 0xBA, sizeof(stack_thread1));
	memset(stack_thread2, 0xBA, sizeof(stack_thread2));

	thread_create(&thr_1, thread_1, stack_thread1, sizeof(stack_thread1));
	thread_create(&thr_2, thread_2, stack_thread2, sizeof(stack_thread2));

	uint32_t main_thread_entry_counter = 0;

	while (1) {
		main_thread_entry_counter++;

		printf("Main thread: %ld\r\n", main_thread_entry_counter);

		/* Just for test, after a number of loops call join for one of threads. 
		 * The main thread should go so sleep until the thread has completed and
		 * executed thread cleanup function.
		 */
		if(main_thread_entry_counter > 300) {
			/* Will sleep until thr_1 is done.*/
			thread_join(thr_1);
			
			for (int idx = 0; idx < 100000; idx++) {
			/* temporary delay loop */
			}
		}
	}

	return 0;
}
