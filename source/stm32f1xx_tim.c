/*
 * stm32f1xx_tim.c
 *
 *  Created on: Feb 11, 2026
 *      Author: haihbv
 */

#include "stm32f1xx_tim.h"
#include "stm32f1xx_gpio.h"
#include "stm32f1xx_rcc.h"

void tim2_init(void)
{
	// enable clock
	clock_enable_APB1(RCC_APB1_TIM2, CLOCK_ON);
	clock_enable_APB2(RCC_APB2_GPIOA| RCC_APB2_AFIO, CLOCK_ON);
	
	TIM2_CR1 |= (0 << 8); // clock division
	TIM2_CR1 |= (0 << 4); // direction: counter mode up
	TIM2_CR1 |= (1 << 7); // ARPE
	
	TIM2_PSC = 72 - 1;		// 1 MHz
	TIM2_ARR = 0xFFFF;
	
	TIM2_CR1 |= (1 << 0); // counter enable
}

void tim2_delay_ms(uint32_t cnt)
{
	while (cnt--) {
		TIM2_CNT = 0;
		while (TIM2_CNT < 1000);
	}
}

void tim2_delay_us(uint32_t cnt)
{
	TIM2_CNT = 0;
	while (TIM2_CNT < cnt);
}
