#include "sys.h"
#include "exti.h"
#include "usart.h"	

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
  //EXTI�ж����ã�
	/* ����NVIC Ϊ���ȼ���1 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	/* �����ж�Դ������1 */
	NVIC_InitStructure.NVIC_IRQChannel = CH1_INT_EXTI_IRQ;
	/* ������ռ���ȼ���1 */
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	/* ���������ȼ���1 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	/* ʹ���ж�ͨ�� */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* �����ж�Դ������2������ʹ������������� */
	NVIC_InitStructure.NVIC_IRQChannel = CH2_INT_EXTI_IRQ;
	NVIC_Init(&NVIC_InitStructure);
	
	//USART�ж����ã�
		/* Ƕ�������жϿ�������ѡ�� */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	/* ����USART Ϊ�ж�Դ */
	NVIC_InitStructure.NVIC_IRQChannel = FP_USART_IRQ;
	/* �������ȼ�Ϊ1 */
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	/* �����ȼ�Ϊ1 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	/* ʹ���ж� */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	/* ��ʼ������NVIC */
	NVIC_Init(&NVIC_InitStructure);
}
