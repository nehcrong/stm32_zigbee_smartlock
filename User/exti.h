#ifndef __EXTI_H
#define __EXTI_H
#include "stm32f10x_exti.h"
//外部中断引脚定义
#define CH1_INT_GPIO_PORT GPIOA
#define CH1_INT_GPIO_CLK (RCC_APB2Periph_GPIOA\
                            |RCC_APB2Periph_AFIO)
#define CH1_INT_GPIO_PIN GPIO_Pin_0
#define CH1_INT_EXTI_PORTSOURCE GPIO_PortSourceGPIOA
#define CH1_INT_EXTI_PINSOURCE GPIO_PinSource0
#define CH1_INT_EXTI_LINE EXTI_Line0
#define CH1_INT_EXTI_IRQ EXTI0_IRQn

#define CH1_IRQHandler EXTI0_IRQHandler


#define CH2_INT_GPIO_PORT GPIOA
#define CH2_INT_GPIO_CLK (RCC_APB2Periph_GPIOA\
                            |RCC_APB2Periph_AFIO)
#define CH2_INT_GPIO_PIN GPIO_Pin_1
#define CH2_INT_EXTI_PORTSOURCE GPIO_PortSourceGPIOA
#define CH2_INT_EXTI_PINSOURCE GPIO_PinSource1
#define CH2_INT_EXTI_LINE EXTI_Line1
#define CH2_INT_EXTI_IRQ EXTI1_IRQn

#define CH2_IRQHandler EXTI1_IRQHandler



void EXTI_Config(void);

#endif
