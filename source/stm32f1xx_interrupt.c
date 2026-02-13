#include "stm32f1xx_interrupt.h"

inline void NVIC_EnableIRQ(IRQn_t IRQn)
{
    if (IRQn >= 0)
    {
        NVIC->ISER[IRQn >> 5] = (1U << (IRQn & 0x1F));
    }
}

inline void NVIC_SetPriority(IRQn_t IRQn, uint32_t priority)
{
  if (IRQn < 0)
  {
    /* Core exception */
    SCB->SHPR[((uint32_t)IRQn & 0xF) - 4] = (uint8_t)(priority << (8 - __NVIC_PRIO_BITS));
  }
  else
  {
    /* Peripheral interrupt */
    NVIC->IPR[IRQn] = (uint8_t)(priority << (8 - __NVIC_PRIO_BITS));
  }
}

