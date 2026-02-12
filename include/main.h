#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

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

#endif /* __MAIN_H__ */
