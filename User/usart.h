#ifndef __USART_H
#define __USART_H
#include "stdio.h"
#include "stm32f10x_usart.h"

//指纹模块串口：
#define FP_USARTx USART1
#define FP_USART_CLK RCC_APB2Periph_USART1
#define FP_USART_APBxClkCmd RCC_APB2PeriphClockCmd
#define FP_USART_BAUDRATE 115200

//通讯串口：
#define Zig_USARTx USART2
#define Zig_USART_CLK RCC_APB1Periph_USART2
#define Zig_USART_APBxClkCmd RCC_APB1PeriphClockCmd
#define Zig_USART_BAUDRATE 115200

// 指纹模块串口 GPIO 引脚宏定义：
#define FP_USART_GPIO_CLK (RCC_APB2Periph_GPIOA)
#define FP_USART_GPIO_APBxClkCmd RCC_APB2PeriphClockCmd
#define FP_USART_TX_GPIO_PORT GPIOA
#define FP_USART_TX_GPIO_PIN GPIO_Pin_9
#define FP_USART_RX_GPIO_PORT GPIOA
#define FP_USART_RX_GPIO_PIN GPIO_Pin_10
#define FP_USART_IRQ USART1_IRQn
#define FP_USART_IRQHandler USART1_IRQHandler

// 通讯串口 GPIO 引脚宏定义：
#define Zig_USART_GPIO_CLK (RCC_APB2Periph_GPIOA)
#define Zig_USART_GPIO_APBxClkCmd RCC_APB2PeriphClockCmd
#define Zig_USART_TX_GPIO_PORT GPIOA
#define Zig_USART_TX_GPIO_PIN GPIO_Pin_2
#define Zig_USART_RX_GPIO_PORT GPIOA
#define Zig_USART_RX_GPIO_PIN GPIO_Pin_3
#define Zig_USART_IRQ USART2_IRQn
#define Zig_USART_IRQHandler USART2_IRQHandler

void USART_Config(void);
void USART_SendByte( USART_TypeDef * pUSARTx, uint8_t ch);
void FP_USART_IRQHandler(void);
void Zig_USART_IRQHandler(void);

#endif
