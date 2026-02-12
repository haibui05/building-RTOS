/*
 * stm32f1xx_rcc.c
 *
 *  Created on: Feb 10, 2026
 *      Author: haihbv
 */

#include "stm32f1xx_rcc.h"

__attribute__((constructor)) void system_init_clock(void) // attribute help function is run before main
{
  // Configure system core clock up to 72MHz
  volatile uint32_t *pFLASH_ACR = (volatile uint32_t *)(FLASH_BASE);
  volatile uint32_t *pRCC_CR = (volatile uint32_t *)(RCC_BASE + 0x00);
  volatile uint32_t *pRCC_CFGR = (volatile uint32_t *)(RCC_BASE + 0x04);

  // 1. Configure Flash Latency and Prefetch
  *pFLASH_ACR &= ~(UINT32)(0x7 << 0); // Clear LATENCY bits
  *pFLASH_ACR |= (2 << 0);            // Set 2 wait states for 72MHz
  *pFLASH_ACR |= (1 << 4);            // Enable prefetch buffer

  // 2. Enable HSE and wait for the HSE to be ready
  *pRCC_CR |= (1 << 16); // HSE on
  while (!(*pRCC_CR & (1 << 17)))
    ; // wait HSE ready

  // 3. Configure the prescaler for AHB, APB1 and APB2
  *pRCC_CFGR &= ~(UINT32)(0xF << 4);  // AHB clear
  *pRCC_CFGR &= ~(UINT32)(0x7 << 8);  // APB1 clear
  *pRCC_CFGR &= ~(UINT32)(0x7 << 11); // APB2 clear

  *pRCC_CFGR |= (0 << 11);
  *pRCC_CFGR |= (4 << 8);
  *pRCC_CFGR |= (0 << 4);

  // 4. Configure the PLL
  *pRCC_CFGR &= ~(UINT32)(1 << 16);   // Clear PLLSRC
  *pRCC_CFGR |= (1 << 16);            // PLL source = HSE
  *pRCC_CFGR &= ~(UINT32)(0xF << 18); // Clear PLLMUL
  *pRCC_CFGR |= (7 << 18);            // PLL x9 (HSE 8MHz * 9 = 72MHz)

  // 5. Enable the PLL and wait for it to be ready
  *pRCC_CR |= (1 << 24);
  while (!(*pRCC_CR & (1 << 25)))
    ; // wait PLL locked

  // 6. Select PLL as system clock
  *pRCC_CFGR &= ~(UINT32)(3 << 0); // Clear SW
  *pRCC_CFGR |= (2 << 0);          // PLL used as system clock
  while (((*pRCC_CFGR >> 2) & 3) != 2)
    ; // Wait until PLL is used
}

void clock_enable_APB2(uint32_t clock_enable, ClockState clock_state) {
  if (clock_state == CLOCK_ON) {
    RCC_APB2ENR |= clock_enable;
  } else {
    RCC_APB2ENR &= ~(clock_enable);
  }
}
void clock_enable_APB1(uint32_t clock_enable, ClockState clock_state) {
  if (clock_state == CLOCK_ON) {
    RCC_APB1ENR |= clock_enable;
  } else {
    RCC_APB1ENR &= ~(clock_enable);
  }
}
