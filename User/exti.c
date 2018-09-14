#include "exti.h"
#include "idworld.h"
#include "usart.h"
#include "delay.h"
#include "stm32f10x.h"

volatile unsigned char ExtiFlag=0;

static void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
  //EXTI中断设置：
	/* 配置NVIC 为优先级组1 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	/* 配置中断源：按键1 */
	NVIC_InitStructure.NVIC_IRQChannel = CH1_INT_EXTI_IRQ;
	/* 配置抢占优先级：1 */
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	/* 配置子优先级：1 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	/* 使能中断通道 */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* 配置中断源：按键2，其他使用上面相关配置 */
	NVIC_InitStructure.NVIC_IRQChannel = CH2_INT_EXTI_IRQ;
	NVIC_Init(&NVIC_InitStructure);
}

void EXTI_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	/*开启按键GPIO 口的时钟*/
	RCC_APB2PeriphClockCmd(CH1_INT_GPIO_CLK,ENABLE);

	/* 配置 NVIC 中断*/
	NVIC_Configuration();

	/*--------------------------CH1 配置---------------------*/
	/* 选择按键用到的GPIO */
	GPIO_InitStructure.GPIO_Pin = CH1_INT_GPIO_PIN;
	/* 配置为浮空输入 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(CH1_INT_GPIO_PORT, &GPIO_InitStructure);

	/* 选择EXTI 的信号源 */
	GPIO_EXTILineConfig(CH1_INT_EXTI_PORTSOURCE, \
	CH1_INT_EXTI_PINSOURCE);
	EXTI_InitStructure.EXTI_Line = CH1_INT_EXTI_LINE;

	/* EXTI 为中断模式 */
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	/* 上升沿中断 */
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	/* 使能中断 */
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/*--------------------------CH2 配置------------------*/
	/* 选择按键用到的GPIO */
	GPIO_InitStructure.GPIO_Pin = CH2_INT_GPIO_PIN;
	/* 配置为浮空输入 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(CH2_INT_GPIO_PORT, &GPIO_InitStructure);

	/* 选择EXTI 的信号源 */
	GPIO_EXTILineConfig(CH2_INT_EXTI_PORTSOURCE, \
	CH2_INT_EXTI_PINSOURCE);
	EXTI_InitStructure.EXTI_Line = CH2_INT_EXTI_LINE;

	/* EXTI 为中断模式 */
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	/* 下降沿中断 */
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	/* 使能中断 */
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
}


void CH1_IRQHandler(void)
{
	//确保是否产生了EXTI Line 中断
	if (EXTI_GetITStatus(CH1_INT_EXTI_LINE) != RESET) 
	{
		ExtiFlag=1;
		//清除中断标志位
		//USART_SendByte( IDWORLD_USARTx,0x01);
		EXTI_ClearITPendingBit(CH1_INT_EXTI_LINE);
	}	
    
}
    
void CH2_IRQHandler(void)
{
    //确保是否产生了EXTI Line 中断
    if (EXTI_GetITStatus(CH2_INT_EXTI_LINE) != RESET) 
		{
			ExtiFlag=2;
			//USART_SendByte( IDWORLD_USARTx,0x02);
			//清除中断标志位
			EXTI_ClearITPendingBit(CH2_INT_EXTI_LINE);
    }
}
