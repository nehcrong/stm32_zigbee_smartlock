#include "usart.h" 
#include "zigbee.h"
#include "stm32f10x.h"

#define HOST_ADDR ((uint16_t) 0x0000)

//����HEX���ݼ�����壺
unsigned char ZigTxBuf[64] = {0xfc,0x00,0x03,0x02,0x00,0x00,0xaa};//���������͵���Ϣ֡Ԥ���á�
unsigned char CMD_READ[4] = {0xfe,0x01,0xfe,0xff};
unsigned char ZigRxCon=0;
unsigned char ZigRxBuf[64];

void ZigRxBufClr(void)	  //���ջ�����0
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
* @brief Zigbee�ն�������������Ϣ������
* @param len:������Ϣ���ֽ�����
* @param event:��Ϣ�¼����ͣ�0��ע���¼���1�������¼���2���޸������¼���3������ָ���¼���4��ɾ��ָ���¼�����
* @param mark:��ע��Ϣ��
* @retval ��
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
