#ifndef __LED_H
#define __LED_H

#include "stm32f1xx_system.h"

void led_init(void);
void led_on(void);
void led_off(void);
void led_red_toggle(void);
void led_white_toggle(void);
void led_blue_toggle(void);

#endif /* __LED_H */
