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
	, periodic1_runner, periodic2_runner\
	, timer_periodic_runner;

static uint32_t semaphore_1, semaphore_2;

void task0(void);
void task1(void);
void task2(void);
void task3(void);

void light_on(void);
void light_off(void);

void TIM2_IRQHandler(void)
{
	/* clear update flag */
	TIM2_SR &= ~(1U << 0);
	timer_periodic_runner++;
	led_red_toggle();
}

int main(void)
{
	usart1_init(115200);
	tim2_init(TIM2_PERIOD_1000Ms);
	led_init();

	/*** semaphore ***/
	rtos_semaphore_init(&semaphore_1, 1);
	rtos_semaphore_init(&semaphore_2, 0);
	/*** os kernel ***/
	rtos_kernel_init();
	rtos_kernel_stack_add_threads(&task0, &task1, &task2);
	rtos_kernel_add_periodic_threads(&task3, 100, &task4, 200);
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
		// rtos_kernel_release();
	}
}

__attribute__((noreturn)) void task1(void)
{
	for (;;)
	{
		// rtos_semaphore_take(&semaphore_1);
		task1_runner++;
		// light_off();
		// rtos_semaphore_give(&semaphore_2);
	}
}

__attribute__((noreturn)) void task2(void)
{
	for (;;)
	{
		// rtos_semaphore_take(&semaphore_2);
		task2_runner++;
		// light_on();
		// rtos_semaphore_give(&semaphore_1);
	}
}

void task4(void)
{
	periodic2_runner++;
	led_white_toggle();
}

void task3(void)
{
 	periodic1_runner++;
	led_blue_toggle();
}
