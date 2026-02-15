#include "os_kernel.h"
#include "stm32f1xx_interrupt.h"

static uint32_t millis_prescaler = 0;

TCB_t *currentPointer;

TCB_t TCBs[NUMBER_THREADS];

/*** Stack 100 i.e 400 bytes***/
static uint32_t TCB_Stack[NUMBER_THREADS][STACK_SIZE];
static uint32_t periodic_tick;

void rtos_scheduler_round_robin(void)
{
 if ((++periodic_tick) == 100)
 {
   (*task3)();
   periodic_tick = 0;
 }
 currentPointer = currentPointer->nextStackPointer;
}

void rtos_kernel_release(void)
{
  /* clear systick current value register */
  volatile uint32_t *pSYST_CVR = (volatile uint32_t *)(0xE000E018);
  *pSYST_CVR = 0;
  
  /* trigger systick */
  SCB->ICSR = (1U << 26); /* set pending */
}

void rtos_kernel_stack_init(int thread_number)
{
	TCBs[thread_number].stackPointer = &TCB_Stack[thread_number][STACK_SIZE - 16];
	
	/* Set bit 24 in PSR (Tbit) to 1 to operate in thumb mode */
	TCB_Stack[thread_number][STACK_SIZE - 1] = (1U << 24);
	
	/* Dummy value of 0xAAAA AAAA to stack content */
	TCB_Stack[thread_number][STACK_SIZE - 3] = 0xAAAAAAAA; // R14 - LR
	TCB_Stack[thread_number][STACK_SIZE - 4] = 0xAAAAAAAA; // R12
	TCB_Stack[thread_number][STACK_SIZE - 5] = 0xAAAAAAAA; // R3
	TCB_Stack[thread_number][STACK_SIZE - 6] = 0xAAAAAAAA; // R2
	TCB_Stack[thread_number][STACK_SIZE - 7] = 0xAAAAAAAA; // R1 
	TCB_Stack[thread_number][STACK_SIZE - 8] = 0xAAAAAAAA; // R0
	
	TCB_Stack[thread_number][STACK_SIZE - 9] = 0xAAAAAAAA; // R11
	TCB_Stack[thread_number][STACK_SIZE - 10] = 0xAAAAAAAA; // R10
	TCB_Stack[thread_number][STACK_SIZE - 11] = 0xAAAAAAAA; // R9
	TCB_Stack[thread_number][STACK_SIZE - 12] = 0xAAAAAAAA; // R8
	TCB_Stack[thread_number][STACK_SIZE - 13] = 0xAAAAAAAA; // R7
	TCB_Stack[thread_number][STACK_SIZE - 14] = 0xAAAAAAAA; // R6
	TCB_Stack[thread_number][STACK_SIZE - 15] = 0xAAAAAAAA; // R5
	TCB_Stack[thread_number][STACK_SIZE - 16] = 0xAAAAAAAA; // R4
}

void rtos_kernel_stack_add_threads( void (*task0)(void), void (*task1)(void), void (*task2)(void) )
{
  /* Disable global interrupt */
  __disable_irq(); /* disable de tranh bi vo hieu hoa cac thao tac ben trong */
  
  TCBs[0].nextStackPointer = &TCBs[1];
  TCBs[1].nextStackPointer = &TCBs[2];
  TCBs[2].nextStackPointer = &TCBs[0];
  
  /* Initialize stack for thread 0 */
  rtos_kernel_stack_init(0);
  
  /* Initialize program counter */
  TCB_Stack[0][STACK_SIZE - 2] = (uint32_t)(task0);
  
  /* Initialize stack for thread 1 */
  rtos_kernel_stack_init(1);
  
  /* Initialize program counter */
  TCB_Stack[1][STACK_SIZE - 2] = (uint32_t)(task1);
  
  /* Initialize stack for thread 2 */
  rtos_kernel_stack_init(2);
  
  /* Initialize program counter */
  TCB_Stack[2][STACK_SIZE - 2] = (uint32_t)(task2);
  
  /* Start from thread 0 */
  currentPointer = &TCBs[0];
  
  
  /* Enable global interrupt */
  __enable_irq();
}

/* This function is writed in os_kernel.s */
//__attribute__((naked)) void rtos_kernel_scheduler_launch()
//{
//  /* Load address of currentPointer into R0 */
//  __asm("LDR R0, =currentPointer");
//  /* Load R2 from address equal to R0, i.e R2 = currentPointer */
//  __asm("LDR R2, [R0]");
//  /* Load cortex-M SP from address equal to R2, i.e SP = currentPointer->stackPointer */
//  __asm("LDR SP, [R2]");
//  /* Restore R4-R11 */
//  __asm("POP {R4-R11}");
//  /* Restore R0-R3 */
//  __asm("POP {R0-R3}");
//  /* Skip R12 */
//  __asm("POP {R12}");
//  /* Skip LR */
//  __asm("ADD SP, SP, #4");
//  /* Create a new start location by popping LR */
//  __asm("POP {LR}");
//  /* Skip PSR by adding 4 to SP */
//  __asm("ADD SP, SP, #4");
//  /* Enable global interrupt */
//  __asm("CPSIE I");
//  /* Return from exception */
//  __asm("BX LR");
//} 

void rtos_kernel_init(void)
{
  millis_prescaler = (BUS_FREQUENCY / 1000); // if 72000000 = 1 second then 72000 = 1ms
}

void rtos_kernel_launch(uint32_t quanta)
{
  volatile uint32_t *pSYST_CSR = (volatile uint32_t*)(SYSTICK_BASE + 0x00); // Systick control and status register
	volatile uint32_t *pSYST_RVR = (volatile uint32_t*)(SYSTICK_BASE + 0x04); // Systick reload value register
	volatile uint32_t *pSYST_CVR = (volatile uint32_t*)(SYSTICK_BASE + 0x08); // Systick current value register
  
  *pSYST_CSR = 0; /* reset systick */
  
  
  *pSYST_CVR = 0; /* clear systick current value register */
  
  *pSYST_RVR = (quanta * millis_prescaler) - 1; /* load quanta */
  
  NVIC_SetPriority(SysTick_IRQn, 15); /* set systick to low priority */
  
  *pSYST_CSR |= (1U << 0); /* enable systick */
  
  *pSYST_CSR |= (1U << 1); /* enable systick interrupt */
  
  *pSYST_CSR |= (1U << 2); /* select interal clock */
  
  rtos_kernel_scheduler_launch(); /* launch scheduler */
}

/* During exception r0,r1,r2,r3,r12,lr,pc,psr are automatically saved onto the stack */
/* This function is writed in os_kernel.s */
void SysTick_Handler(void)
{
	volatile uint32_t *pICSR = (volatile uint32_t*)(0xE000ED04); /* trigger pending PendSV */
	*pICSR |= (1U << 28);
}
//__attribute__((naked)) void SysTick_Handler(void)
//{
//  /* 
//   * 1. suspend current thread 
//   */
//  /* disable global interrupt */
//  __asm("CPSID  I");
//  
//  /* save r4, r5, r6, r7, r8, r9, r10, r11 */
//  __asm("PUSH {R4-R11}");
//  /* load address of current pointer into r0 */
//  __asm("LDR R0, =currentPointer");
//  /* load r1 from address equals r0, i.e. r1 = currentPointer */
//  __asm("LDR R1,[R0]");
//  /* store cortex-M stack pointer at address equals r1, i.e. save SP into TCB */
//  __asm("STR SP,[R1]");
//  
//  /* 
//   * 2. choose next thread 
//   */
//  /* load r1 from a location 4 bytes above address r1, i.e r1 = current->nextStackPointer */
//  // __asm("LDR R1,[R1,#4]");
//  
//  __asm("PUSH {R0, LR}");
//  __asm("BL rtos_scheduler_round_robin");
//  __asm("POP {R0, LR}");
//  
//  // R1 = currentPointer
//  __asm("LDR R1,[R0]");
//  // sp = currentPointer->stackPointer
//  __asm("LDR SP,[R1]");
//  
//  /* store r1 at address equals r0, i.e currentPointer = r1 */
//  __asm("STR R1,[R0]");
//  /* load cortex-M stack pointer at adress equals r1, i.e SP = currentPointer->stackPointer*/
//  __asm("LDR SP,[R1]"); 
//  /* restoring r4, r5, r6, r7, r8, r9, r10, r11 */
//  __asm("POP {R4-R11}");
//  /* enable global interrupt */
//  __asm("CPSIE  I");
//  /* return from exception and restore r0, r1, r2, r3, r12, lr, pc, psr */
//  __asm("BX LR");
//}

/* implement semaphores in scheduler */
void rtos_semaphore_init(uint32_t *semaphore, uint32_t value)
{
	*semaphore = value;
}

void rtos_semaphore_give(uint32_t *semaphore)
{
	__disable_irq();
	*semaphore = *semaphore + 1;
	__enable_irq();
}

void rtos_semaphore_take(uint32_t *semaphore)
{
  /* old */
  // __disable_irq();
  // while (*semaphore <= 0)
  // {
  //   __disable_irq();
  //   __enable_irq();
  // }
  // __enable_irq();

  /* new */
	__disable_irq();
	while (*semaphore <= 0)
	{
		__enable_irq();
		rtos_kernel_release();  /* yield CPU to other tasks */
		__disable_irq();
	}
	*semaphore = *semaphore - 1;  /* decrement semaphore */
	__enable_irq();
}

void osKernelInit(void)
{
  rtos_kernel_init();
}

void osYield(void)
{
  rtos_kernel_release();
}

void osKernelAddThreads( void (*task0)(void), void (*task1)(void), void (*task2)(void) )
{
  rtos_kernel_stack_add_threads(task0, task1, task2);
}

uint8_t osKernelLaunch(uint32_t quanta)
{
  rtos_kernel_launch(quanta);
  return 0;
}

