/*
 * stm32f1xx_tim.h
 *
 *  Created on: Feb 11, 2026
 *      Author: haihbv
 */

#ifndef __STM32F1xx_TIM_H
#define __STM32F1xx_TIM_H

#include "main.h"

#define TIM2_BASE 		(0x40000000U)
#define TIM2_CR1   		(*(volatile uint32_t *)(TIM2_BASE + 0x00U))
#define TIM2_CR2			(*(volatile uint32_t *)(TIM2_BASE + 0x04U))
#define TIM2_SMCR			(*(volatile uint32_t *)(TIM2_BASE + 0x08U))
#define TIM2_DIER			(*(volatile uint32_t *)(TIM2_BASE + 0x0CU))
#define TIM2_SR				(*(volatile uint32_t *)(TIM2_BASE + 0x10U))
#define TIM2_EGR			(*(volatile uint32_t *)(TIM2_BASE + 0x14U))
#define TIM2_CCMR1		(*(volatile uint32_t *)(TIM2_BASE + 0x18U))
#define TIM2_CCMR2		(*(volatile uint32_t *)(TIM2_BASE + 0x1CU))
#define TIM2_CCER			(*(volatile uint32_t *)(TIM2_BASE + 0x20U))
#define TIM2_CNT			(*(volatile uint32_t *)(TIM2_BASE + 0x24U))
#define TIM2_PSC			(*(volatile uint32_t *)(TIM2_BASE + 0x28U))
#define TIM2_ARR			(*(volatile uint32_t *)(TIM2_BASE + 0x2CU))
#define TIM2_CCR1			(*(volatile uint32_t *)(TIM2_BASE + 0x34U))
#define TIM2_CCR2			(*(volatile uint32_t *)(TIM2_BASE + 0x38U))
#define TIM2_CCR3			(*(volatile uint32_t *)(TIM2_BASE + 0x3CU))
#define TIM2_CCR4			(*(volatile uint32_t *)(TIM2_BASE + 0x40U))
#define TIM2_DCR			(*(volatile uint32_t *)(TIM2_BASE + 0x48U))
#define TIM2_DMAR			(*(volatile uint32_t *)(TIM2_BASE + 0x4CU))
	
void tim2_init(void);
void tim2_delay_ms(uint32_t cnt);
void tim2_delay_us(uint32_t cnt);

#endif // __STM32F1xx_TIM_H
