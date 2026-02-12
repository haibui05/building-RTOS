/*
 * stm32f1xx_dwt.c
 *
 *  Created on: Feb 11, 2026
 *      Author: haihbv
 */
#include "stm32f1xx_dwt.h"

void dwt_init(void)
{
	DWT_DEMCR |= (1 << 24);  // TRCENA
	DWT_CYCCNT = 0;
	DWT_CTRL |= 1; // CYCCNTENA
	
	__asm volatile ("dsb");
	__asm volatile ("isb");
}
void delay_us(uint32_t us)
{
	volatile uint32_t start = DWT_CYCCNT;
	uint32_t tick = us * (SYSTEM_CORE_CLOCK / 1000000u);
	while ((DWT_CYCCNT - start) < tick);
}
