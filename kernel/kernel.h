/*
 * kernelh
 *
 *  Created on: Feb 13, 2026
 *      Author: haihbv
 */
 
#ifndef __KERNEL_H
#define __KERNEL_H

#include "stm32f1xx_system.h"

extern void rtos_kernel_scheduler_launch();
extern void rtos_kernel_stack_init( int thread_number );
/**
 * @brief RTOS kernel initialization
 */
extern void rtos_kernel_init( void );

/**
 * @brief Add threads to the RTOS kernel
 */
extern void rtos_kernel_stack_add_threads( void (*task0)(void), void (*task1)(void), void (*task2)(void) );

/**
 * @brief Launch RTOS with quanta time (ms)
 */
extern void rtos_kernel_launch( uint32_t quanta );

void osKernelInit(void);
void osKernelAddThreads( void (*task0)(void), void (*task1)(void), void (*task2)(void) );
uint8_t osKernelLaunch(uint32_t quanta);

#endif /* __KERNEL_H */
