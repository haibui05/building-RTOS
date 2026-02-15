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

#endif /* __STM32F1xx_EXTI_H */
