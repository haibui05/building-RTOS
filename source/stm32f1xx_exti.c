#include "stm32f1xx_exti.h"

void exti_line0_init(void)
{
	clock_enable_APB2(RCC_APB2_AFIO, CLOCK_ON);
	
	AFIO->EXTICR[0] &= ~(0xFU << 0);
	AFIO->EXTICR[0] |= (0U << 0);
	
	EXTI->IMR |= (1U << 0);
	
	EXTI->RTSR |= (1U << 0);
	EXTI->FTSR &= ~(1U << 0);
	
	NVIC_EnableIRQ(EXTI0_IRQn);
}

void exti_init(EXTI_Port_t port_source, uint16_t pin_source, EXTI_Mode_t exti_mode)
{
	uint8_t val_port = 0x00, tmp_pin_source = 0x00;
	if (port_source == GPIOA_SOURCE) {
		val_port = 0;
	} else if (port_source == GPIOB_SOURCE) {
		val_port = 1;
	} else if (port_source == GPIOC_SOURCE) {
		val_port = 2;
	} else return;
	
	tmp_pin_source = (pin_source % 4);
	/* config AFIO */
	if (pin_source < 4)
	{ 
		/* AFIO_EXTICR1 */
		AFIO->EXTICR[0] &= ~(0xFU << (tmp_pin_source * 4));
		AFIO->EXTICR[0] |= ((uint32_t)val_port << (tmp_pin_source * 4));
	} else if (pin_source < 8) {
		/* AFIO_EXTICR2 */
		AFIO->EXTICR[1] &= ~(0xFU << (tmp_pin_source * 4));
		AFIO->EXTICR[1] |= ((uint32_t)val_port << (tmp_pin_source * 4));
	} else if (pin_source < 12) {
		/* AFIO_EXTICR3 */
		AFIO->EXTICR[2] &= ~(0xFU << (tmp_pin_source * 4));
		AFIO->EXTICR[2] |= ((uint32_t)val_port << (tmp_pin_source * 4));
	} else if (pin_source < 16) {
		/* AFIO_EXTICR4 */
		AFIO->EXTICR[3] &= ~(0xFU << (tmp_pin_source * 4));
		AFIO->EXTICR[3] |= ((uint32_t)val_port << (tmp_pin_source * 4));
	} else return;
	
	EXTI->IMR |= (1U << pin_source); /* unmask */
	
	/* trigger */
	switch (exti_mode)
	{
		case EXTI_MODE_RISING:
			EXTI->RTSR |= (1U << pin_source);
			EXTI->FTSR &= ~(1U << pin_source);
			break;
		case EXTI_MODE_FALLING:
			EXTI->RTSR &= ~(1U << pin_source);
			EXTI->FTSR |= (1U << pin_source);
			break;
		case EXTI_BOTH_TRIGGER:
			EXTI->RTSR |= (1U << pin_source);
			EXTI->FTSR |= (1U << pin_source);
			break;
	}
	
	/* NVIC */
	if (pin_source < 5)
	{
 		NVIC->ISER[0] |= (1U << (6 + pin_source));
	}
	else if (pin_source < 10)
	{
		NVIC->ISER[0] |= (1U << 23);
	}
	else if (pin_source < 16)
	{
		NVIC->ISER[1] |= (1U << (40 - 32));
	}
	else return;
}

void EXTI0_IRQHandler(void)
{
  if (EXTI->PR & (1U << 0))
  {
		EXTI->PR |= (1U << 0);
		/* Code */
		// GPIOC->ODR ^= GPIO_PIN_13;
  }
}
