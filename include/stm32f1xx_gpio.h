/*
 * stm32f1xx_gpio.h
 *
 *  Created on: Feb 10, 2026
 *      Author: haihbv
 */

#ifndef __STM32F1xx__GPIO_H
#define __STM32F1xx__GPIO_H

#include "stm32f1xx_system.h"

#define GPIO_PIN_0   	   	((uint16_t)0x0001)  
#define GPIO_PIN_1   	   	((uint16_t)0x0002)  
#define GPIO_PIN_2   	   	((uint16_t)0x0004)  
#define GPIO_PIN_3   	   	((uint16_t)0x0008)  
#define GPIO_PIN_4   	   	((uint16_t)0x0010)  
#define GPIO_PIN_5   	   	((uint16_t)0x0020)  
#define GPIO_PIN_6   	   	((uint16_t)0x0040)  
#define GPIO_PIN_7   	   	((uint16_t)0x0080)  
#define GPIO_PIN_8   	   	((uint16_t)0x0100)  
#define GPIO_PIN_9   	   	((uint16_t)0x0200)  
#define GPIO_PIN_10   	 	((uint16_t)0x0400)  
#define GPIO_PIN_11   	 	((uint16_t)0x0800)  
#define GPIO_PIN_12   	 	((uint16_t)0x1000)  
#define GPIO_PIN_13   	 	((uint16_t)0x2000)  
#define GPIO_PIN_14   	 	((uint16_t)0x4000)  
#define GPIO_PIN_15   	 	((uint16_t)0x8000) 

typedef struct
{
	volatile uint32_t CRL; // 0x00
	volatile uint32_t CRH; // 0x04
	volatile uint32_t IDR;
	volatile uint32_t ODR;
	volatile uint32_t BSRR;
	volatile uint32_t BRR;
	volatile uint32_t LCKR;
} GPIO_TypeDef;

typedef enum
{
	GPIO_MODE_INPUT_FLOATING = 0x00,
	GPIO_MODE_OUTPUT_PP = 0x01,
	GPIO_MODE_OUTPUT_OD = 0x02,
	GPIO_MODE_AF_PP = 0x03,
	GPIO_MODE_AF_OD = 0x04,
	GPIO_MODE_INPUT_ANALOG = 0x05,
	GPIO_MODE_INPUT_PU = 0x06,
	GPIO_MODE_INPUT_PD = 0x07
} GPIOMode_Type;

typedef enum
{
	GPIO_SPEED_10MHZ = 0x00U,
	GPIO_SPEED_2MHZ = 0x01U,
	GPIO_SPEED_50MHZ = 0x02U,
} GPIOSpeed_Type;

#define GPIOA_BASE (0x40010800U)
#define GPIOB_BASE (0x40010C00U)
#define GPIOC_BASE (0x40011000U)

#define GPIOA ( (GPIO_TypeDef *)GPIOA_BASE )
#define GPIOB ( (GPIO_TypeDef *)GPIOB_BASE )
#define GPIOC ( (GPIO_TypeDef *)GPIOC_BASE )

void gpio_init(GPIO_TypeDef *GPIOx, uint16_t pin, GPIOMode_Type mode, GPIOSpeed_Type speed);
void gpio_write_pin(GPIO_TypeDef *GPIOx, uint16_t pin, BitAction bit_state);
uint8_t gpio_read_pin(GPIO_TypeDef *GPIOx, uint16_t pin);

#endif // __STM32F1xx__GPIO_H
