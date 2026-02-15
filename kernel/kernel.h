/*
 * kernelh
 *
 *  Created on: Feb 13, 2026
 *      Author: haihbv
 */

#ifndef __KERNEL_H
#define __KERNEL_H

#include "stm32f1xx_system.h"

#define NUMBER_THREADS 3
#define STACK_SIZE 100 /* 100 word */
#define BUS_FREQUENCY (72000000U)

/* TCB structure definition */
struct TCB
{
	uint32_t *stackPointer;
	struct TCB *nextStackPointer;
};

typedef struct TCB TCB_t;

extern TCB_t *currentPointer;

extern TCB_t TCBs[NUMBER_THREADS];

extern void rtos_kernel_stack_init(int thread_number);
extern void rtos_kernel_release(void);

/**
 * @brief RTOS kernel initialization
 */
extern void rtos_kernel_init(void);

/**
 * @brief Add threads to the RTOS kernel
 */
extern void rtos_kernel_stack_add_threads(void (*task0)(void), void (*task1)(void), void (*task2)(void));

/**
 * @brief Scheduler launch function
 */
extern void rtos_kernel_scheduler_launch(void);

/**
 * @brief Launch RTOS with quanta time (ms)
 */
extern void rtos_kernel_launch(uint32_t quanta);
extern void task3(void);
extern void rtos_scheduler_round_robin(void);

void osKernelInit(void);
void osKernelAddThreads(void (*task0)(void), void (*task1)(void), void (*task2)(void));
uint8_t osKernelLaunch(uint32_t quanta);

#endif /* __KERNEL_H */
