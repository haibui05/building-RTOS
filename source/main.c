/*
 * main.c
 *
 *  Created on: Feb 11, 2026
 *      Author: haihbv
 */

#include "stm32f1xx_system.h"
#include "led.h"
#include "kernel.h"

uint32_t task0_runner, task1_runner, task2_runner;

void task0(void);
void task1(void);
void task2(void);

void light_on(void);
void light_off(void);

int main(void)
{
	// systick_init();

	usart1_init(115200);
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
		task0_runner++;
		light_on();
	}
}

void task1(void)
{
	for (;;)
	{
		task1_runner++;
		light_off();
	}
}

void task2(void)
{
	for (;;)
	{
		task2_runner++;
	}
}
