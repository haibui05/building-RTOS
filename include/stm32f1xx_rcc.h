/*
 * stm32f1xx_rcc.h
 *
 *  Created on: Feb 10, 2026
 *      Author: haihbv
 */

#ifndef __STM32F1xx_RCC_H
#define __STM32F1xx_RCC_H

#include "main.h"


typedef enum
{
	CLOCK_ON,
	CLOCK_OFF
} ClockState;

void system_init_clock(void);
void clock_enable_APB2(uint32_t clock_enable, ClockState clock_state);
void clock_enable_APB1(uint32_t clock_enable, ClockState clock_state);


#define RCC_BASE  					0x40021000U
#define FLASH_BASE 					0x40022000U

#define RCC_APB2_AFIO       ((uint32_t)0x00000001)
#define RCC_APB2_GPIOA      ((uint32_t)0x00000004)
#define RCC_APB2_GPIOB      ((uint32_t)0x00000008)
#define RCC_APB2_GPIOC      ((uint32_t)0x00000010)
#define RCC_APB2_ADC1       ((uint32_t)0x00000200)
#define RCC_APB2_ADC2       ((uint32_t)0x00000400)
#define RCC_APB2_TIM1       ((uint32_t)0x00000800)
#define RCC_APB2_SPI1       ((uint32_t)0x00001000)
#define RCC_APB2_USART1     ((uint32_t)0x00004000)

#define RCC_APB1_TIM2       ((uint32_t)0x00000001)
#define RCC_APB1_TIM3       ((uint32_t)0x00000002)
#define RCC_APB1_TIM4       ((uint32_t)0x00000004)
#define RCC_APB1_WWDG       ((uint32_t)0x00000800)
#define RCC_APB1_SPI2       ((uint32_t)0x00004000)
#define RCC_APB1_SPI3       ((uint32_t)0x00008000)
#define RCC_APB1_USART2     ((uint32_t)0x00020000)
#define RCC_APB1_USART3     ((uint32_t)0x00040000)
#define RCC_APB1_I2C1       ((uint32_t)0x00200000)
#define RCC_APB1_I2C2       ((uint32_t)0x00400000)
#define RCC_APB1_USB        ((uint32_t)0x00800000)
#define RCC_APB1_CAN1       ((uint32_t)0x02000000)
#define RCC_APB1_CAN2       ((uint32_t)0x04000000)
#define RCC_APB1_BKP        ((uint32_t)0x08000000)
#define RCC_APB1_PWR        ((uint32_t)0x10000000)

#define RCC_APB2ENR  				(*(volatile uint32_t*)0x40021018)
#define RCC_APB1ENR  				(*(volatile uint32_t*)0x4002101C)

#endif // __STM32F1xx_RCC_H
