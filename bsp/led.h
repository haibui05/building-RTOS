#ifndef __LED_H
#define __LED_H

#include "main.h"
#include "stm32f1xx_gpio.h"
#include "stm32f1xx_rcc.h"

void led_init(void);
void led_on(void);
void led_off(void);

#endif /* __LED_H */
