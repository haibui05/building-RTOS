/*
 * stm32f1xx_systick.h
 *
 *  Created on: Feb 11, 2026
 *      Author: haihbv
 */
#ifndef __STM32F1xx_SYSTICK_H
#define __STM32F1xx_SYSTICK_H

#include "stm32f1xx_system.h"

#define SYSTICK_BASE 0xE000E010U

#define SYS_TIMER_LOAD (SYSTEM_CORE_CLOCK / 1000u)

extern void systick_init(void);
extern uint32_t get_tick(void);
extern void delay_ms(uint32_t delay);
void SysTick_Handler(void);

#endif /* __STM32F1xx_SYSTICK_H */
