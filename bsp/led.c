#include "led.h"

void led_init(void) {
	clock_enable_APB2(RCC_APB2_GPIOA, CLOCK_ON);
	
	gpio_init(GPIOA, GPIO_PIN_0, GPIO_MODE_OUTPUT_PP, GPIO_SPEED_50MHZ);
	gpio_init(GPIOA, GPIO_PIN_1, GPIO_MODE_OUTPUT_PP, GPIO_SPEED_50MHZ);
	gpio_init(GPIOA, GPIO_PIN_2, GPIO_MODE_OUTPUT_PP, GPIO_SPEED_50MHZ);
}

void led_on(void) { gpio_write_pin(GPIOA, GPIO_PIN_0, BIT_RESET); }

void led_off(void) { gpio_write_pin(GPIOA, GPIO_PIN_0, BIT_SET); }

void led_red_toggle(void)
{
	GPIOA->ODR ^= GPIO_PIN_0;
}
void led_white_toggle(void)
{
	GPIOA->ODR ^= GPIO_PIN_1;
}
void led_blue_toggle(void)
{
	GPIOA->ODR ^= GPIO_PIN_2;
}
