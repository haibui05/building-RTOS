#include "os_kernel.h"
#include "stm32f1xx_interrupt.h"
#include "stm32f1xx_tim.h"

#define STACK_SIZE 100 /* 100 word */
#define MAX_PERIODIC_THREAD 5

static uint32_t millis_prescaler = 0;

TCB_t *currentPointer;

TCB_t TCBs[NUMBER_THREADS];

/*** Stack 100 i.e 400 bytes***/
static uint32_t TCB_Stack[NUMBER_THREADS][STACK_SIZE];
static uint32_t periodic_tick;

void rtos_scheduler_round_robin(void)
{
 periodic_tick++;
 if ((periodic_tick % 100) == 0)
 {
   (*task3)();
 }
 if ((periodic_tick % 200) == 0)
 {
	 (*task4)();
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
	/* Sleep Time */
	for (uint32_t i = 0; i < NUMBER_THREADS; i++) {
		TCBs[i].sleepTime= 0;
	}
  /* Enable global interrupt */
  __enable_irq();
}

void rtos_kernel_init(void)
{
  millis_prescaler = (SYSTEM_CORE_CLOCK / 1000); // if 72000000 = 1 second then 72000 = 1ms
	rtos_periodic_task_init(&periodic_event_execute, 1000, 6);
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
  
  rtos_kernel_scheduler_launch(); /* launch scheduler in os_kernel.s */
}

/* During exception r0,r1,r2,r3,r12,lr,pc,psr are automatically saved onto the stack */
/* This function is writed in os_kernel.s */
void SysTick_Handler(void)
{
	volatile uint32_t *pICSR = (volatile uint32_t*)(0xE000ED04); /* trigger pending PendSV */
	*pICSR |= (1U << 28);
}

/*************************************************************************
 * Implement semaphore for synchronization between threads in RTOS scheduler
*************************************************************************/
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
	__disable_irq();
	while (*semaphore <= 0)
	{
		__enable_irq();
		__disable_irq();
	}
	*semaphore = *semaphore - 1;  /* decrement semaphore */
	__enable_irq();
}

void rtos_cooperative_semaphore_take(uint32_t *semaphore)
{
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


/*************************************************************************
 * Implement threads control block for periodic threads in RTOS scheduler
*************************************************************************/
/*** Periodic TCBs ***/
#define NUMBER_OF_PERIODIC_THREADS 3

typedef void (*taskT)(void);

typedef struct
{
	taskT name_task;
	uint32_t period;
	uint32_t time_remaning;
} PeriodicTask_t;

static PeriodicTask_t TaskPeriodic[NUMBER_OF_PERIODIC_THREADS];
static uint32_t number_of_period_thread;

//uint8_t rtos_kernel_add_periodic_threads	(void (*task0)(void), uint32_t period1, \
//																					void (*task1)(void), uint32_t period2)
//{
//	max_period = (period1 > period2) ? period1 : period2;
//	min_period = (period1 < period2) ? period1 : period2;
//	
//	TaskPeriodic[0].name_task = task0;
//	TaskPeriodic[0].period = period1;
//	TaskPeriodic[1].name_task = task1;
//	TaskPeriodic[1].period = period2;
//	return 1;
//}

uint8_t rtos_kernel_add_periodic_threads (void (*task)(void), uint32_t period)
{
	if ((number_of_period_thread == MAX_PERIODIC_THREAD) || (period == 0))
	{
		return 0;
	}
	TaskPeriodic[number_of_period_thread].name_task = task;
	TaskPeriodic[number_of_period_thread].period = period;
	TaskPeriodic[number_of_period_thread].time_remaning = period - 1;
	number_of_period_thread++;
	return 1;
}

void periodic_event_execute(void)
{
	for (uint32_t i = 0; i < number_of_period_thread; i++)
	{
		if (TaskPeriodic[i].time_remaning == 0)
		{
			TaskPeriodic[i].name_task();
			TaskPeriodic[i].time_remaning = TaskPeriodic[i].period - 1;
		}
		else
		{
			TaskPeriodic[i].time_remaning--;
		}
	}
	
	for (uint32_t i = 0; i < NUMBER_THREADS; i++)\
	{
		if (TCBs[i].sleepTime > 0)
		{
			TCBs[i].sleepTime--;
		}
	}
}

void rtos_periodic_scheduler_round_robin(void)
{
 currentPointer = currentPointer->nextStackPointer;
}

void rtos_periodic_scheduler_round_robin_with_sleep(void)
{
 currentPointer = currentPointer->nextStackPointer;
 while (currentPointer->sleepTime > 0) {
	currentPointer = currentPointer->nextStackPointer;
 }
}


void (*periodic_threads)(void);

void rtos_periodic_task_init(void (*task)(), uint32_t freq, uint8_t priority)
{
	__disable_irq();
	periodic_threads = task;
	clock_enable_APB1(RCC_APB1_TIM2, CLOCK_ON);
	clock_enable_APB2(RCC_APB2_GPIOA| RCC_APB2_AFIO, CLOCK_ON);
	TIM2_CR1 |= (0 << 8); // clock division
	TIM2_CR1 |= (0 << 4); // direction: counter mode up
	TIM2_CR1 |= (1 << 7); // ARPE
	TIM2_PSC = 8 - 1;		// 1us
	TIM2_ARR = ( 1000000 / freq ) - 1;    // one s
	TIM2_CNT = 0; /* clear counter*/
	TIM2_CR1 |= (1 << 0); /* enable timer */
	TIM2_DIER |= (1U << 0); /* enable timer interrupt */
	NVIC_SetPriority(TIM2_IRQn, priority);
	NVIC->ISER[0] |= (1U << 28); /* enable timer interrupt in NVIC */
	__enable_irq();
}

void TIM2_IRQHandler(void)
{
	/* clear update flag */
	TIM2_SR &= ~(1U << 0);
	(*periodic_threads)();
}

/*************************************************************************
 * Implement thread sleep
*************************************************************************/
void rtos_thread_sleep(uint32_t sleep_time)
{
	__disable_irq();
	currentPointer->sleepTime = sleep_time;
	__enable_irq();
	osYield();
}

/*************************************************************************
 * Mailbox
*************************************************************************/
static uint8_t mailbox_data_available;
static uint32_t mailbox_data;
static uint32_t mailbox_semaphore;

void rtos_mailbox_init(void)
{
	mailbox_data_available = 0;
	mailbox_data = 0;
	rtos_semaphore_init(&mailbox_semaphore, 0);
}

void rtos_mailbox_send(uint32_t data)
{
	__disable_irq();
	if (mailbox_data_available)
	{
		__enable_irq();
		return;
	}
	mailbox_data = data;
	mailbox_data_available = 1;
	__enable_irq();
	rtos_semaphore_give(&mailbox_semaphore);
}

uint32_t rtos_mailbox_receive(void)
{
	rtos_cooperative_semaphore_take(&mailbox_semaphore);
	uint32_t data;
	__disable_irq();
	data = mailbox_data;
	mailbox_data_available = 0;
	__enable_irq();
	return data;
}

/*************************************************************************
 * FIFO
*************************************************************************/
#define FIFO_SIZE 15
uint32_t fifo_buffer[FIFO_SIZE];
uint32_t put_index;
uint32_t get_index;
uint32_t current_fifo_size;
uint32_t lost_data;

void rtos_fifo_init(void)
{
	put_index = 0;
	get_index = 0;
	rtos_semaphore_init(&current_fifo_size, 0);
	lost_data = 0;
}
int8_t rtos_fifo_add(uint32_t data)
{
	if (current_fifo_size == FIFO_SIZE)
	{
		lost_data++;
		return -1; /* FIFO full */
	}
	fifo_buffer[put_index] = data;
	put_index = (put_index + 1) % FIFO_SIZE;
	rtos_semaphore_give(&current_fifo_size);
	return 1; /* success */
}
uint32_t rtos_fifo_read(void)
{
	uint32_t data;
	rtos_cooperative_semaphore_take(&current_fifo_size);
	__disable_irq();
	data = fifo_buffer[get_index];
	get_index = (get_index + 1) % FIFO_SIZE;
	__enable_irq();
	return data;
}

/*************************************************************************
 * Queue
*************************************************************************/
void queue_init(Queue_Type *queue)
{
	queue->put_index = 0;
	queue->get_index = 0;
	queue->current_size = 0;
	rtos_semaphore_init(&queue->current_size, 0);
	queue->lock = 0;
}

int8_t queue_send(Queue_Type *queue, uint32_t data)
{
	rtos_semaphore_take(&queue->lock);

	if (queue->current_size == QUEUE_SIZE)
	{
		rtos_semaphore_give(&queue->lock);
		return -1; /* Queue full */
	}

	queue->buffer[queue->put_index] = data;
	queue->put_index = (queue->put_index + 1) % QUEUE_SIZE;
	rtos_semaphore_give(&queue->lock);
	return 1; /* success */
}

int8_t queue_receive(Queue_Type *queue, uint32_t *data)
{
	rtos_semaphore_take(&queue->lock);

	if (queue->current_size == 0)
	{
		rtos_semaphore_give(&queue->lock);
		return -1; /* Queue empty */
	}

	*data = queue->buffer[queue->get_index];
	queue->get_index = (queue->get_index + 1) % QUEUE_SIZE;
	rtos_semaphore_give(&queue->lock);
	return 1; /* success */
}


/*************************************************************************
 * Implement Sporadic threads
*************************************************************************/
uint32_t *edge_semaphore;

void gpio_external_interrupt_init(void)
{
	__disable_irq();
	clock_enable_APB2(RCC_APB2_GPIOC | RCC_APB2_AFIO, CLOCK_ON);
	gpio_init(GPIOC, GPIO_PIN_13, GPIO_MODE_INPUT_PU, GPIO_SPEED_50MHZ);
	exti_init(GPIOC_SOURCE, GPIO_PIN_SOURCE_13, EXTI_MODE_FALLING);
	NVIC_SetPriority(EXTI15_10_IRQn, 15);
	__enable_irq();
}

void rtos_egde_trigger_init(uint32_t *semaphore)
{
	edge_semaphore = semaphore;
	gpio_external_interrupt_init();
}

/*************************************************************************
 * Implement RTOS kernel API
*************************************************************************/
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

