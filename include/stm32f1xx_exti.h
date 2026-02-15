/*
 * main.c
 *
 *  Created on: Feb 14, 2026
 *      Author: haihbv
 */
#ifndef __STM32F1xx_EXTI_H
#define __STM32F1xx_EXTI_H

#include "stm32f1xx_system.h"

typedef struct
{
  volatile uint32_t IMR;
  volatile uint32_t EMR;
  volatile uint32_t RTSR;
  volatile uint32_t FTSR;
  volatile uint32_t SWIER;
  volatile uint32_t PR;
} EXTI_TypeDef;

#define EXTI_BASE   (0x40010400U)
#define EXTI        ((EXTI_TypeDef *)EXTI_BASE)

typedef enum
{
	EXTI_MODE_RISING = 0x00,
	EXTI_MODE_FALLING = 0x01,
	EXTI_BOTH_TRIGGER = 0x02
} EXTI_Mode_t;

typedef enum
{
	GPIOA_SOURCE = 1,
	GPIOB_SOURCE,
	GPIOC_SOURCE
} EXTI_Port_t;

#define GPIO_PIN_SOURCE_0 (0U)
#define GPIO_PIN_SOURCE_1 (1U)
#define GPIO_PIN_SOURCE_2 (2U)
#define GPIO_PIN_SOURCE_3 (3U)
#define GPIO_PIN_SOURCE_4 (4U)
#define GPIO_PIN_SOURCE_5 (5U)
#define GPIO_PIN_SOURCE_6 (6U)
#define GPIO_PIN_SOURCE_7 (7U)
#define GPIO_PIN_SOURCE_8 (8U)
#define GPIO_PIN_SOURCE_9 (9U)
#define GPIO_PIN_SOURCE_10 (10U)
#define GPIO_PIN_SOURCE_11 (11U)
#define GPIO_PIN_SOURCE_12 (12U)
#define GPIO_PIN_SOURCE_13 (13U)
#define GPIO_PIN_SOURCE_14 (14U)
#define GPIO_PIN_SOURCE_15 (15U)

void exti_line0_init(void);
void exti_init(EXTI_Port_t port_source, uint16_t pin_source, EXTI_Mode_t exti_mode);
void EXTI0_IRQHandler(void);

#endif /* __STM32F1xx_EXTI_H */
