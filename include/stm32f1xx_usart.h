/*
 * stm32f1xx_usart.h
 *
 *  Created on: Feb 10, 2026
 *      Author: haihbv
 */

#ifndef __STM32F1xx_USART_H
#define __STM32F1xx_USART_H

#include "main.h"

typedef struct
{
  volatile uint32_t SR;
  volatile uint32_t DR;
  volatile uint32_t BRR;
  volatile uint32_t CR1;
  volatile uint32_t CR2;
  volatile uint32_t CR3;
  volatile uint32_t GTPR;
} USART_TypeDef;

#define USART1_BASE 0x40013800U
#define USART2_BASE 0x40004400U
#define USART3_BASE 0x40004800U

#define USART1 ((USART_TypeDef *)USART1_BASE)
#define USART2 ((USART_TypeDef *)USART2_BASE)
#define USART3 ((USART_TypeDef *)USART3_BASE)

void usart1_init(uint32_t baud_rate);
void usart1_send_char(char c);
void usart1_send_string(char *str);
uint8_t usart1_receive_byte(void);

#endif // __STM32F1xx_USART_H
