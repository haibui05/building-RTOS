/*
 * stm32f1xx_systick.h
 *
 *  Created on: Feb 11, 2026
 *      Author: haihbv
 */
#ifndef __STM32F1xx_SYSTICK_H
#define __STM32F1xx_SYSTICK_H

#include "main.h"

extern void systick_init(void);
extern uint32_t get_tick(void);
extern void delay_ms(uint32_t delay);

#endif /* __STM32F1xx_SYSTICK_H */
