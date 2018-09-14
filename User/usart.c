#include "usart.h"	
#include "idworld.h"
#include "zigbee.h"
#include "stm32f10x.h"

volatile extern unsigned char ExtiFlag;

static void NVIC_Configuration(void)
{
	/* ����ָ�ƴ�����UART�жϿ�����NVIC */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//Ƕ�������жϿ�������ѡ��
	NVIC_InitStructure.NVIC_IRQChannel = FP_USART_IRQ;	//����USART1Ϊ�ж�Դ
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	//�������ȼ�Ϊ1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	//�����ȼ�Ϊ1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	//ʹ���ж�
	NVIC_Init(&NVIC_InitStructure);	//��ʼ������NVIC
	
	/* ����ZigBeeͨѶUART�жϿ�����NVIC */
	NVIC_InitStructure.NVIC_IRQChannel = Zig_USART_IRQ;	//����USART2Ϊ�ж�Դ
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;	//�����ȼ�Ϊ2,�������ò���
	NVIC_Init(&NVIC_InitStructure);	//��ʼ������NVIC
}


void USART_Config(void)
{
	/*ָ�ƴ�����UART����*/
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	FP_USART_GPIO_APBxClkCmd(FP_USART_GPIO_CLK, ENABLE);	// �򿪴���GPIO ��ʱ��	
	FP_USART_APBxClkCmd(FP_USART_CLK, ENABLE);	// �򿪴��������ʱ��	
	// ��USART Tx ��GPIO ����Ϊ���츴��ģʽ
	GPIO_InitStructure.GPIO_Pin = FP_USART_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(FP_USART_TX_GPIO_PORT, &GPIO_InitStructure);
	// ��USART Rx ��GPIO ����Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = FP_USART_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(FP_USART_RX_GPIO_PORT, &GPIO_InitStructure);	
	// ���ô��ڵĹ�������
	USART_InitStructure.USART_BaudRate = FP_USART_BAUDRATE;	// ���ò�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	// ���� �������ֳ�
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	// ����ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;	// ����У��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	// ����Ӳ��������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	// ���ù���ģʽ���շ�һ��
	USART_Init(FP_USARTx, &USART_InitStructure);	// ���ָ�ƴ��ڵĳ�ʼ������
	
	/*ZigBeeͨѶUART����*/
	Zig_USART_GPIO_APBxClkCmd(Zig_USART_GPIO_CLK, ENABLE);	// �򿪴���GPIO ��ʱ��	
	Zig_USART_APBxClkCmd(Zig_USART_CLK, ENABLE);	// �򿪴��������ʱ��	
	// ��USART Tx ��GPIO ����Ϊ���츴��ģʽ
	GPIO_InitStructure.GPIO_Pin = Zig_USART_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(Zig_USART_TX_GPIO_PORT, &GPIO_InitStructure);
	// ��USART Rx ��GPIO ����Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = Zig_USART_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(Zig_USART_RX_GPIO_PORT, &GPIO_InitStructure);	
	// ���ô��ڵĹ�������
	USART_InitStructure.USART_BaudRate = Zig_USART_BAUDRATE;	// ���ò�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	// ���� �������ֳ�
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	// ����ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;	// ����У��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	// ����Ӳ��������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	// ���ù���ģʽ���շ�һ��
	USART_Init(Zig_USARTx, &USART_InitStructure);	// ���ͨѶ���ڵĳ�ʼ������
	
	// �����ж����ȼ�����
	NVIC_Configuration();
	
	// ʹ�ܴ��ڽ����ж�
	USART_ITConfig(FP_USARTx, USART_IT_RXNE, ENABLE);
	USART_ITConfig(Zig_USARTx, USART_IT_RXNE, ENABLE);		

	// ʹ�ܴ���
	USART_Cmd(FP_USARTx, ENABLE);
	USART_Cmd(Zig_USARTx, ENABLE);
}



void USART_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
{
	/* ����һ���ֽ����ݵ�USART */
	USART_SendData(pUSARTx,ch);
	
	/* �ȴ��������ݼĴ���Ϊ�� */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
}


void FP_USART_IRQHandler(void)
{
	if(USART_GetFlagStatus(FP_USARTx, USART_FLAG_RXNE) == SET)
	{
		RxBuf[RxCon++]=USART_ReceiveData(FP_USARTx);
		USART_ClearFlag(FP_USARTx,USART_FLAG_RXNE);
	}
}

void Zig_USART_IRQHandler(void)
{	
	if(USART_GetFlagStatus(Zig_USARTx, USART_FLAG_RXNE) == SET)
	{
		ExtiFlag=3;
		ZigRxBuf[ZigRxCon++]=USART_ReceiveData(Zig_USARTx);
		USART_ClearFlag(Zig_USARTx,USART_FLAG_RXNE);
	}
}
