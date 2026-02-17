/*
 * stm32f1xx_usart.c
 *
 *  Created on: Feb 10, 2026
 *      Author: haihbv
 */

#include "stm32f1xx_usart.h"
#include "stm32f1xx_gpio.h"
#include "stm32f1xx_rcc.h"

#include <stdio.h>

void usart1_init(uint32_t baud_rate)
{
	// base address of usart1
	__IO uint32_t *pUSART_CR1 = (__IO uint32_t *) (0x40013800 + 0x0C);
	__IO uint32_t *pUSART_CR2 = (__IO uint32_t *) (0x40013800 + 0x10);
	__IO uint32_t *pUSART_BRR = (__IO uint32_t *) (0x40013800 + 0x08);
	
	// enable periph usart
	clock_enable_APB2(RCC_APB2_GPIOA | RCC_APB2_USART1, CLOCK_ON);
	
	// configure the pin for usart
	gpio_init(GPIOA, GPIO_PIN_9, GPIO_MODE_AF_PP, GPIO_SPEED_50MHZ);
	gpio_init(GPIOA, GPIO_PIN_10, GPIO_MODE_INPUT_FLOATING, GPIO_SPEED_50MHZ);
	
	// configure usart
	*pUSART_BRR = ((SYSTEM_CORE_CLOCK + baud_rate/2U) / baud_rate);
	
	*pUSART_CR1 |= (0 << 12); //  1 Start bit, 8 Data bits, n Stop bit
	*pUSART_CR1 |= (0 << 10); // 	0: Parity control disabled
	*pUSART_CR1 |= (0 << 9); //// even parity
	*pUSART_CR1 |= ( (1 << 3) | (1 << 2) ); // enable TX & RX
	
	*pUSART_CR2 |= (0 << 12);
	
	*pUSART_CR1 |= (1 << 13); // enable usart
}
void usart1_send_char(char c)
{
	__IO uint32_t *pUSART_SR = (__IO uint32_t *) (0x40013800 + 0x00);
	__IO uint32_t *pUSART_DR = (__IO uint32_t *) (0x40013800 + 0x04);
	while (!READ_BIT(*pUSART_SR, (1 << 7)));
	*pUSART_DR = (uint8_t)(c & 0xFF);
}

void usart1_send_string(char *str)
{
	while (*str)
	{
		usart1_send_char(*str++);
	}
}

uint8_t usart1_receive_byte(void)
{
	uint8_t temp;
	
	__IO uint32_t *pUSART_SR = (__IO uint32_t *) (0x40013800 + 0x00);
	__IO uint32_t *pUSART_DR = (__IO uint32_t *) (0x40013800 + 0x04);
	while (!READ_BIT(*pUSART_SR, (1 << 5)));
	temp = (uint8_t)*pUSART_DR; 
	
	return temp;
}

__attribute__((noreturn, unused)) static void _sys_exit(int x)
{
    (void)x;
    for ( ;; );
}

FILE __stdout;

int fputc(int ch, FILE *f)
{
    (void)f;
    if (ch == '\n')
    {
        usart1_send_char('\n');
    }
    usart1_send_char((char)ch);
    return ch;
}

