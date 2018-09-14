#include "usart.h"	
#include "idworld.h"
#include "zigbee.h"
#include "stm32f10x.h"

volatile extern unsigned char ExtiFlag;

static void NVIC_Configuration(void)
{
	/* 配置指纹传感器UART中断控制器NVIC */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//嵌套向量中断控制器组选择
	NVIC_InitStructure.NVIC_IRQChannel = FP_USART_IRQ;	//配置USART1为中断源
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	//抢断优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	//子优先级为1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	//使能中断
	NVIC_Init(&NVIC_InitStructure);	//初始化配置NVIC
	
	/* 配置ZigBee通讯UART中断控制器NVIC */
	NVIC_InitStructure.NVIC_IRQChannel = Zig_USART_IRQ;	//配置USART2为中断源
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;	//子优先级为2,其他设置不变
	NVIC_Init(&NVIC_InitStructure);	//初始化配置NVIC
}


void USART_Config(void)
{
	/*指纹传感器UART设置*/
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	FP_USART_GPIO_APBxClkCmd(FP_USART_GPIO_CLK, ENABLE);	// 打开串口GPIO 的时钟	
	FP_USART_APBxClkCmd(FP_USART_CLK, ENABLE);	// 打开串口外设的时钟	
	// 将USART Tx 的GPIO 配置为推挽复用模式
	GPIO_InitStructure.GPIO_Pin = FP_USART_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(FP_USART_TX_GPIO_PORT, &GPIO_InitStructure);
	// 将USART Rx 的GPIO 配置为浮空输入模式
	GPIO_InitStructure.GPIO_Pin = FP_USART_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(FP_USART_RX_GPIO_PORT, &GPIO_InitStructure);	
	// 配置串口的工作参数
	USART_InitStructure.USART_BaudRate = FP_USART_BAUDRATE;	// 配置波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	// 配置 针数据字长
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	// 配置停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;	// 配置校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	// 配置硬件流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	// 配置工作模式，收发一起
	USART_Init(FP_USARTx, &USART_InitStructure);	// 完成指纹串口的初始化配置
	
	/*ZigBee通讯UART设置*/
	Zig_USART_GPIO_APBxClkCmd(Zig_USART_GPIO_CLK, ENABLE);	// 打开串口GPIO 的时钟	
	Zig_USART_APBxClkCmd(Zig_USART_CLK, ENABLE);	// 打开串口外设的时钟	
	// 将USART Tx 的GPIO 配置为推挽复用模式
	GPIO_InitStructure.GPIO_Pin = Zig_USART_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(Zig_USART_TX_GPIO_PORT, &GPIO_InitStructure);
	// 将USART Rx 的GPIO 配置为浮空输入模式
	GPIO_InitStructure.GPIO_Pin = Zig_USART_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(Zig_USART_RX_GPIO_PORT, &GPIO_InitStructure);	
	// 配置串口的工作参数
	USART_InitStructure.USART_BaudRate = Zig_USART_BAUDRATE;	// 配置波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	// 配置 针数据字长
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	// 配置停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;	// 配置校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	// 配置硬件流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	// 配置工作模式，收发一起
	USART_Init(Zig_USARTx, &USART_InitStructure);	// 完成通讯串口的初始化配置
	
	// 串口中断优先级配置
	NVIC_Configuration();
	
	// 使能串口接收中断
	USART_ITConfig(FP_USARTx, USART_IT_RXNE, ENABLE);
	USART_ITConfig(Zig_USARTx, USART_IT_RXNE, ENABLE);		

	// 使能串口
	USART_Cmd(FP_USARTx, ENABLE);
	USART_Cmd(Zig_USARTx, ENABLE);
}



void USART_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
{
	/* 发送一个字节数据到USART */
	USART_SendData(pUSARTx,ch);
	
	/* 等待发送数据寄存器为空 */
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
