/*
 * main.c
 *
 *  Created on: Feb 17, 2026
 *      Author: haihbv
 */

#include "stm32f1xx_system.h"
#include "led.h"
#include "os_kernel.h"

void task0(void);
void task1(void);
void task2(void);

uint32_t main_edge_semaphore, sporadic_runner, task1_runner;
volatile uint32_t task2_runner;
uint32_t rev_buffer_data[16];
Queue_Type mainQueue;

int main(void)
{
	usart1_init(115200);
	led_init();
	/*** Initialize Kernel ***/
	rtos_kernel_init();
	// rtos_semaphore_init(&main_edge_semaphore, 0);
	// rtos_egde_trigger_init(&main_edge_semaphore);
	queue_init(&mainQueue);
	/* add threads */
	rtos_kernel_stack_add_threads(&task0, &task1, &task2);
	rtos_kernel_launch(10); /* set round-robin time to 10ms */
	while (1)
		;
}

__attribute__((noreturn)) void task0(void)
{
	static uint32_t counter = 0;
	for (;;)
	{
		sporadic_runner++;

		if (queue_send(&mainQueue, counter) > 0)
		{
			usart1_send_string("Queue send\n");
			counter++;
		}
		else
		{
			usart1_send_string("Queue full\n");
		}
		rtos_thread_sleep(1000u);
	}
}

__attribute__((noreturn)) void task1(void)
{
	for (;;)
	{
		uint32_t data;
		for (;;)
		{
			if (queue_receive(&mainQueue, &data) > 0)
			{
				usart1_send_string("Queue recv\n");
				/* debug */
				rev_buffer_data[task1_runner % 16] = data;
				task1_runner++;
			}
			else
			{
				rtos_kernel_release();
			}
		}
	}
}

__attribute__((noreturn)) void task2(void)
{
	for (;;)
	{
		task2_runner++;
		rtos_thread_sleep(1);
	}
}
