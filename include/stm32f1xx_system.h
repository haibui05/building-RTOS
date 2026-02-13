/*
 * stm32f1xx_system.h
 *
 *  Created on: Feb 11, 2026
 *      Author: haihbv
 */
#ifndef __STM32F1xx_SYSTEM_H
#define __STM32F1xx_SYSTEM_H

#define __IO volatile
#define __IM volatile const

#define UINT32 uint32_t
#define UINT16 uint16_t
#define UINT8 uint8_t

#define SET_BIT(REG, BIT) ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT) ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT) ((REG) & (BIT))
#define WRITE_BIT(REG, VAL) ((REG) = (VAL))

#define INTERRUPT_DISABLE()   \
    do                        \
    {                         \
        uint32_t val = 0x20;  \
        __asm volatile(       \
            "msr BASEPRI, %0" \
            :                 \
            : "r"(val)        \
            : "memory");      \
    } while (0)

#define INTERRUPT_ENABLE()    \
    do                        \
    {                         \
        uint32_t val = 0x00;  \
        __asm volatile(       \
            "msr BASEPRI, %0" \
            :                 \
            : "r"(val)        \
            : "memory");      \
    } while (0)

#define __disable_irq() INTERRUPT_DISABLE()
#define __enable_irq() INTERRUPT_ENABLE()

#define SYSTEM_CORE_CLOCK 72000000U
		
typedef enum
{
    ENABLE = 0x00,
    DISABLE = 0x01
} BitState;

typedef enum
{
    BIT_RESET = 0,
    BIT_SET
} BitAction;

#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"

#include "stm32f1xx_rcc.h"
#include "stm32f1xx_gpio.h"
#include "stm32f1xx_usart.h"
#include "stm32f1xx_tim.h"
#include "stm32f1xx_systick.h"
#include "stm32f1xx_dwt.h"

#endif /* __STM32F1xx_SYSTEM_H */
