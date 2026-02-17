/*
 * main.c
 *
 *  Created on: Feb 11, 2026
 *      Author: haihbv
 */

#include "stm32f1xx_system.h"
#include "led.h"
#include "os_kernel.h"

static volatile uint32_t task0_runner, task1_runner, task2_runner \
	, periodic3_runner, periodic4_runner, periodic5_runner;
static uint32_t semaphore_0, semaphore_1, semaphore_2;

char text0[] = "Task0 running\r\n";
char text1[] = "Task1 running\r\n";
char text2[] = "Task2 running\r\n";
uint32_t value = 777;
uint32_t rev_buffer_data[15] = {0};
 
void task0(void);
void task1(void);
void task2(void);
void task3(void);
void task5(void);

void light_on(void);
void light_off(void);

int main(void)
{
	usart1_init(115200);
	led_init();

	/*** semaphore ***/
	// rtos_semaphore_init(&semaphore_0, 1);
	// rtos_semaphore_init(&semaphore_1, 0);
	// rtos_semaphore_init(&semaphore_2, 0);
	
	/*** Initialize Kernel ***/
	rtos_kernel_init();
	rtos_fifo_init();
	// rtos_mailbox_init();
	/* add threads */
	rtos_kernel_stack_add_threads(&task0, &task1, &task2);
	// rtos_kernel_add_periodic_threads(&task3, 100, &task4, 200);
	// rtos_kernel_add_periodic_threads(&task3, 500);
	// rtos_kernel_add_periodic_threads(&task4, 1000);
	// rtos_kernel_add_periodic_threads(&task5, 1500);
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
		task0_runner++;
		rtos_fifo_add(task0_runner);
		rtos_thread_sleep( 500u );
		// rtos_mailbox_send(value);
		// rtos_cooperative_semaphore_take(&semaphore_0);
		// usart1_send_string(text0);
		// rtos_semaphore_give(&semaphore_1);
		// rtos_thread_sleep(200u);
	}
}

__attribute__((noreturn)) void task1(void)
{
	for (;;)
	{
		task1_runner++;
		static uint32_t index;
		rev_buffer_data[index] = rtos_fifo_read();
		if (index > 9)
				index = 0;
		else 
				index++;
		// receive_data = rtos_mailbox_receive();
		// rtos_cooperative_semaphore_take(&semaphore_1);
		// usart1_send_string(text1);
		// rtos_semaphore_give(&semaphore_2);
	}
}

__attribute__((noreturn)) void task2(void)
{
	for (;;)
	{
		task2_runner++;
		// rtos_cooperative_semaphore_take(&semaphore_2);
		// usart1_send_string(text2);
		// rtos_semaphore_give(&semaphore_0);
	}
}

void task3(void)
{
 	periodic3_runner++;
	led_blue_toggle();
}

void task4(void)
{
	periodic4_runner++;
	led_white_toggle();
}

void task5(void)
{
	periodic5_runner++;
	led_red_toggle();
}
