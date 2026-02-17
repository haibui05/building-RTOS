/*
 * stm32f1xx_gpio.c
 *
 *  Created on: Feb 10, 2026
 *      Author: haihbv
 */

#include "stm32f1xx_gpio.h"

void gpio_init(GPIO_TypeDef *GPIOx, UINT16 pin, GPIOMode_Type mode, GPIOSpeed_Type speed)
{
	uint32_t pos = 0x00, tmp_reg = 0x00;
	
	for (pos = 0; pos < 16; pos++)
	{
			if (pin & (1 << pos))
			{
				// reset
				if (pos < 8)
				{
					GPIOx->CRL &= ~(0xfu << (pos * 4));
				}
				else
				{
					GPIOx->CRH &= ~(0xfu << ( (pos - 8) * 4 ));
				}
			}
			
			switch (mode)
			{
				case GPIO_MODE_OUTPUT_PP:
					tmp_reg = (UINT32)(speed << 0) | (0x00 << 2);
					break;
				case GPIO_MODE_OUTPUT_OD:
					tmp_reg = (UINT32)(speed << 0) | (0x01 << 2);
					break;
				case GPIO_MODE_AF_PP:
					tmp_reg = (UINT32)(speed << 0) | (0x02 << 2);
					break;
				case GPIO_MODE_AF_OD:
					tmp_reg = (UINT32)(speed << 0) | (0x03 << 2);
					break;
				case GPIO_MODE_INPUT_ANALOG:
					tmp_reg = (0x00 << 0) | (0x00 << 2);
					break;
				case GPIO_MODE_INPUT_FLOATING:
					tmp_reg = (0x00 << 0) | (0x01 << 2);
					break;
				case GPIO_MODE_INPUT_PU:
					tmp_reg = (0x00 << 0) | (0x02 << 2);
					GPIOx->ODR |= (1 << pos);
					break;
				case GPIO_MODE_INPUT_PD:
					tmp_reg = (0x00 << 0) | (0x02 << 2);
					GPIOx->ODR &= ~(1 << pos);
					break;
			}
			
			if (pos < 8)
			{
				GPIOx->CRL |= (tmp_reg << (pos * 4));
			}
			else 
			{
				GPIOx->CRH |= (tmp_reg << ( (pos - 8) * 4 ));
			}
	}
}
void gpio_write_pin(GPIO_TypeDef *GPIOx, UINT16 pin, BitAction bit_state)
{
	if (bit_state == BIT_RESET)
	{
		GPIOx->BRR = pin;
	}
	else
	{
		GPIOx->BSRR = pin;
	}
}

uint8_t gpio_read_pin(GPIO_TypeDef *GPIOx, UINT16 pin)
{
	return (GPIOx->IDR & pin) ? 1 : 0;
}
