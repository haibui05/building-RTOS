/*
 * stm32f1xx_dwt.h
 *
 *  Created on: Feb 11, 2026
 *      Author: haihbv
 */
#ifndef __STM32F1xx_DWT_H
#define __STM32F1xx_DWT_H

#include "main.h"

#define DWT_DEMCR   (*(volatile uint32_t*)0xE000EDFC)
#define DWT_CTRL   (*(volatile uint32_t*)0xE0001000)
#define DWT_CYCCNT (*(volatile uint32_t*)0xE0001004)

extern void dwt_init(void);
extern void delay_us(uint32_t us);

#endif /* __STM32F1xx_DWT_H */
