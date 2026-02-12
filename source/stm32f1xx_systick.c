/*
 * stm32f1xx_systick.c
 *
 *  Created on: Feb 11, 2026
 *      Author: haihbv
 */

#include "stm32f1xx_systick.h"

#define SYSTICK_BASE 0xE000E010U

#define SYS_TIMER_LOAD (SYSTEM_CORE_CLOCK / 1000u)

static volatile uint32_t g_current_tick;
static volatile uint32_t current_tick;
static volatile uint32_t tick_freq = 1;

void SysTick_Handler(void);
extern void systick_init(void)
{
	volatile uint32_t *pSYST_CSR = (volatile uint32_t*)(SYSTICK_BASE + 0x00); // Systick control and status register
	volatile uint32_t *pSYST_RVR = (volatile uint32_t*)(SYSTICK_BASE + 0x04); // Systick reload value register
	volatile uint32_t *pSYST_CVR = (volatile uint32_t*)(SYSTICK_BASE + 0x08); // Systick current value register
	// volatile uint32_t *pSYST_CALIB = (volatile uint32_t*)(0xE000E01C); // Systick calibration
	
	*pSYST_RVR = SYS_TIMER_LOAD - 1;
	*pSYST_CVR = 0;
	*pSYST_CSR |= (1U << 2); 
	*pSYST_CSR |= (1U << 1);
	*pSYST_CSR |= (1U << 0);
	
	__enable_irq(); /* enable global interrupt */
}
extern uint32_t get_tick(void)
{
	__disable_irq();
	current_tick = g_current_tick;
	__enable_irq();
	return current_tick;
}
extern void delay_ms(uint32_t delay)
{
	uint32_t tick_start = get_tick();
	uint32_t wait = delay;
	if (wait < 0xFFFFFFFFU)
	{
		wait += (uint32_t)(tick_freq);
	}
	while ((get_tick() - tick_start) < wait);
}

void SysTick_Handler(void)
{
	g_current_tick += tick_freq;
}
