/*
 * main.c
 *
 *  Created on: Feb 11, 2026
 *      Author: haihbv
 */

#include "main.h"
#include "stm32f1xx_rcc.h"
#include "stm32f1xx_gpio.h"
#include "stm32f1xx_usart.h"
#include "stm32f1xx_tim.h"
#include "stm32f1xx_systick.h"
#include "stm32f1xx_dwt.h"

#include "led.h"

void light_on(void);
void light_off(void);

int main(void) {
	systick_init();
	
	usart1_init(115200);
	
	led_init();
	
	unsigned char global_var = 0;
	
	if (global_var == 1)
	{
		light_on();
	}
	else
	{
		light_off();
	}
	
	for ( ;; );
}

void light_on(void) {
	while (1) {
		led_on();
		printf("light on\n");
		delay_ms(1000);
	}
}

void light_off(void) {
	while (1) {
		led_off();
		printf("light off\n");
		delay_ms(1000);
	}
}
