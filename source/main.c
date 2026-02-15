/*
 * main.c
 *
 *  Created on: Feb 11, 2026
 *      Author: haihbv
 */

#include "stm32f1xx_system.h"
#include "led.h"
#include "os_kernel.h"

static volatile uint32_t coopertive_task0_runner, task1_runner, task2_runner \
	, periodic_task3_runner, timer_periodic_task4_runner;

static uint32_t semaphore_1, semaphore_2;

void task0(void);
void task1(void);
void task2(void);
void task3(void);

void light_on(void);
void light_off(void);

static void set_up_hardware(void)
{
	usart1_init(115200);
	tim2_init(TIM2_PERIOD_100Ms);
	led_init();
}

void TIM2_IRQHandler(void)
{
	/* clear update flag */
	TIM2_SR &= ~(1U << 0);
	timer_periodic_task4_runner++;
}

int main(void)
{
	set_up_hardware();

	/*** semaphore ***/
	rtos_semaphore_init(&semaphore_1, 1);
	rtos_semaphore_init(&semaphore_2, 0);
	/*** os kernel ***/
	rtos_kernel_init();
	rtos_kernel_stack_add_threads(&task0, &task1, &task2);
	rtos_kernel_launch(10);/* set round-robin time to 10ms */

	for ( ;; ) {
		/* should never be here */
	}
}

void light_on(void)
{
	usart1_send_string("light on\n");
}

void light_off(void)
{
	usart1_send_string("light off\n");
}

__attribute__((noreturn)) void task0(void)
{
	for (;;)
	{
		coopertive_task0_runner++;
		// light_on();
		rtos_kernel_release();
	}
}

__attribute__((noreturn)) void task1(void)
{
	for (;;)
	{
		rtos_semaphore_wait(&semaphore_1);
		task1_runner++;
		light_off();
		rtos_semaphore_set(&semaphore_2);
	}
}

__attribute__((noreturn)) void task2(void)
{
	for (;;)
	{
		rtos_semaphore_wait(&semaphore_2);
		task2_runner++;
		light_on();
		rtos_semaphore_set(&semaphore_1);
	}
}

void task3(void)
{
  periodic_task3_runner++;
}
