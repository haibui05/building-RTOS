#ifndef __STM32F1xx_AFIO_H
#define __STM32F1xx_AFIO_H

#include "stm32f1xx_system.h"

typedef struct
{
  volatile uint32_t EVCR;
  volatile uint32_t MAPR;
  volatile uint32_t EXTICR[4];
  volatile uint32_t MAPR2;
} AFIO_TypeDef;

#define AFIO_BASE   (0x40010000U)
#define AFIO        ((AFIO_TypeDef *)AFIO_BASE)

#endif /* __STM32F1xx_AFIO_H */
