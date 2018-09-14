#include "usart.h" 
#include "zigbee.h"
#include "stm32f10x.h"

#define HOST_ADDR ((uint16_t) 0x0000)

//常用HEX数据及命令定义：
unsigned char ZigTxBuf[64] = {0xfc,0x00,0x03,0x02,0x00,0x00,0xaa};//向主机发送的消息帧预设置。
unsigned char CMD_READ[4] = {0xfe,0x01,0xfe,0xff};
unsigned char ZigRxCon=0;
unsigned char ZigRxBuf[64];

void ZigRxBufClr(void)	  //接收缓存清0
{
	while(ZigRxCon>0)
		ZigRxBuf[--ZigRxCon]=0;
}

void Zig_ReadInfo(void)
{
	unsigned int i;
	for(i=0;i<4;i++)
		USART_SendByte(Zig_USARTx,CMD_READ[i]);
}


/**
* @brief Zigbee终端向主机发送消息函数。
* @param len:发送消息的字节数。
* @param event:消息事件类型（0：注册事件；1：解锁事件；2：修改密码事件；3：增加指纹事件；4：删除指纹事件）。
* @param mark:备注信息。
* @retval 无
*/
void Zig_BasicPost(unsigned char len,unsigned char event,unsigned char mark)
{
	ZigTxBuf[1]=len;
	ZigTxBuf[9]=event;
	ZigTxBuf[10]=mark;
	unsigned int i;
	for(i=0;i<len+2;i++)
		USART_SendByte(Zig_USARTx,ZigTxBuf[i]);
}

void Zig_Set(void)
{
	
}
