/*
 * main.c
 *
 *  Created on: Feb 11, 2026
 *      Author: haihbv
 */

#include "stm32f1xx_system.h"
#include "led.h"
#include "kernel.h"

static volatile uint32_t coopertive_task0_runner, task1_runner, task2_runner, periodic_task3_runner;

void task0(void);
void task1(void);
void task2(void);
void task3(void);

void light_on(void);
void light_off(void);

static void set_up_hardware(void)
{
	usart1_init(115200);
	tim2_init();
	led_init();
}

int main(void)
{
	set_up_hardware();

	/***os kernel***/
	rtos_kernel_init();
	rtos_kernel_stack_add_threads(&task0, &task1, &task2);
	rtos_kernel_launch(10); /* set round-robin time to 10ms */

	while (1)
	{
		/* should never be here */
	}
}

void light_on(void)
{
	printf("light on\n");
}

void light_off(void)
{
	printf("light off\n");
}

void task0(void)
{
	for (;;)
	{
		coopertive_task0_runner++;
		// light_on();
		rtos_kernel_release();
	}
}

void task1(void)
{
	for (;;)
	{
		task1_runner++;
		// light_off();
	}
}

void task2(void)
{
	for (;;)
	{
		task2_runner++;
	}
}

void task3(void)
{
  periodic_task3_runner++;
}
