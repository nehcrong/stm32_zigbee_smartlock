#ifndef __TIMER_H
#define __TIMER_H
#include "stm32f10x.h"
#include "stm32f10x_tim.h"

#define COM_TIM TIM2
#define COM_TIM_APBxClock_FUN RCC_APB1PeriphClockCmd
#define COM_TIM_CLK RCC_APB1Periph_TIM2
#define COM_TIM_PREIOD 1000
#define COM_TIM_IRQ TIM2_IRQn
#define COM_TIM_IRQHandler TIM2_IRQHandler

#define MIN_INTERVAL ((uint32_t) 100)
#define MAX_INTERVAL ((uint32_t) 20000)
void Timer_Init(void);
void Timer_Start(void);

#endif
