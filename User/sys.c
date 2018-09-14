#include "sys.h"
#include "exti.h"
#include "usart.h"	

void NVIC_Configuration(void)
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
	
	//USART中断设置：
		/* 嵌套向量中断控制器组选择 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	/* 配置USART 为中断源 */
	NVIC_InitStructure.NVIC_IRQChannel = FP_USART_IRQ;
	/* 抢断优先级为1 */
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	/* 子优先级为1 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	/* 使能中断 */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	/* 初始化配置NVIC */
	NVIC_Init(&NVIC_InitStructure);
}
