#include "idworld.h"
#include "delay.h"
#include "usart.h" 
#include "stm32f10x.h"

unsigned int AddNewFlag=0;     //��ӱ�־ 0δ���  ����0��ӳɹ� ��ֵΪ�����ָ�Ƶĺ���
unsigned int IdentifyFlag=0;   //0ʶ��ʶ��  ����0ʶ��ɹ�����ֵΪʶ�����ָ�ƺ���
unsigned int  CurrentNum=0;    //��ǰָ�Ƹ���
unsigned int  SearchNum=0;   //�������

unsigned char RxCon=0;
unsigned char RxBuf[26];


//IDWORLDָ��ģ��ͨ��Э�鶨��
unsigned char PREFIX[4] = {0x55,0xAA,0x00,0x00};  //���ݰ�ͷ
unsigned char CMD_GET_IMAGE[22] = {0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0x01}; //�ɼ�ָ��ͼ�����ImageBuffer
unsigned char CMD_GENERATE0[22] = {0x60,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x61,0x01}; //��ImageBuffer����ȡģ������RamBuffer0
unsigned char CMD_GENERATE1[22] = {0x60,0x00,0x02,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x62,0x01}; //��ImageBuffer����ȡģ������RamBuffer1
unsigned char CMD_GENERATE2[22] = {0x60,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x63,0x01}; //��ImageBuffer����ȡģ������RamBuffer2
unsigned char CMD_MERGE[22] = {0x61,0x00,0x03,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x66,0x01}; //��3��RamBuffer�����ݺϲ�Ϊһ��ģ��
unsigned char CMD_GET_EMPTY_ID[22] = {0x45,0x00,0x04,0x00,0x01,0x00,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x52,0x01};//��ѯģ���1-9���׸����ñ�š�
unsigned char CMD_SEARCH[22] = {0x63,0x00,0x06,0x00,0x00,0x00,0x01,0x00,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x72,0x01}; //��1-9��ָ��ģ��Ա���֤
unsigned char CMD_STORE_CHAR[22] = {0x40,0x00,0x04,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x44,0x01}; //��RamBuffer0��ģ������ģ���1��ָ��λ��
unsigned char CMD_DEL_CHAR[22] = {0x44,0x00,0x04,0x00,0x01,0x00,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x51,0x01};//ɾ��ģ�����1-9��ָ��ģ��
unsigned char CMD_SLED_CTRL[22] = {0x24,0x00,0x02,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x26,0x01};//����ָ��ģ��ƹ�
//volatile unsigned char FINGER_NUM;

void RxBufClr(void)	  //���ջ�����0
{
  unsigned char i=0;
  for(i=0;i<26;i++)
  {
    RxBuf[i]=0;
  }
  RxCon=0;
}


//���Ͱ�ͷ
void SEND_PREFIX(void)  //��������ͷ
{
   unsigned char  i;

   RxBufClr();
   for(i=0;i<4;i++)  
   {
     USART_SendByte(FP_USARTx,PREFIX[i]);   
   }
}

//FINGERPRINT_���ָ��ͼ������
void SEND_CMD_GET_IMAGE(void)
{
  unsigned char i;
  
  SEND_PREFIX(); //����ͨ��Э���ͷ
  for(i=0;i<22;i++) //��������
     USART_SendByte(FP_USARTx,CMD_GET_IMAGE[i]);
}

//��ͼ��ת��������������Buffer0��
void SEND_CMD_GENERATE0(void)
{
  unsigned char i;
      
  SEND_PREFIX(); //����ͨ��Э���ͷ         
  for(i=0;i<22;i++)   //�������� ��ͼ��ת���� ������ �����buffer0
  {
    USART_SendByte(FP_USARTx,CMD_GENERATE0[i]);
  }
}

//��ͼ��ת��������������Buffer1��
void SEND_CMD_GENERATE1(void)
{
   unsigned char i;
   
   SEND_PREFIX(); //����ͨ��Э���ͷ
   for(i=0;i<22;i++)   //�������� ��ͼ��ת���� ������ ����� CHAR_buffer1
   {
	  USART_SendByte(FP_USARTx,CMD_GENERATE1[i]);
   }
}

//��ͼ��ת��������������Buffer2��
void SEND_CMD_GENERATE2(void)
{
   unsigned char i;
   
   SEND_PREFIX(); //����ͨ��Э���ͷ
   for(i=0;i<22;i++)   //�������� ��ͼ��ת���� ������ ����� CHAR_buffer1
   {
	  USART_SendByte(FP_USARTx,CMD_GENERATE2[i]);
   }
}

//��3��RamBuffer�����ݺϲ�Ϊһ��ģ��
void SEND_CMD_MERGE(void)
{
   unsigned char i;    
   
   SEND_PREFIX(); //����ͨ��Э���ͷ
   for(i=0;i<22;i++) //����ϲ�ָ��ģ��
   {
     USART_SendByte(FP_USARTx,CMD_MERGE[i]);   
   }
}

void SEND_CMD_GET_EMPTY_ID(void)
{
	unsigned char i;      
  SEND_PREFIX(); //����ͨ��Э���ͷ
  for(i=0;i<22;i++) //����ϲ�ָ��ģ��
     USART_SendByte(FP_USARTx,CMD_GET_EMPTY_ID[i]);   
}

//ɾ��ָ��ģ�����1-9ָ��ģ��
void SEND_CMD_DEL_CHAR(void)
{
  unsigned char i;    
  
  SEND_PREFIX(); //����ͨ��Э���ͷ
  for(i=0;i<22;i++) //����ϲ�ָ��ģ��
     USART_SendByte(FP_USARTx,CMD_DEL_CHAR[i]);   
}



//��1-9��ָ��ģ��Ա���֤
void SEND_CMD_SEARCH(void)
{
  unsigned char i;	   

  SEND_PREFIX(); //����ͨ��Э���ͷ
  for(i=0;i<22;i++)
  {
    USART_SendByte(FP_USARTx,CMD_SEARCH[i]);   
  }

}



void SEND_CMD_STORE_CHAR(unsigned char storeID)
{
  unsigned char i;	
  unsigned short int sum=0xff;  
  CMD_STORE_CHAR[4]=storeID;
  for(i=0;i<20;i++)
      sum=sum+CMD_STORE_CHAR[i];
  CMD_STORE_CHAR[20]=sum&0x00ff;
  CMD_STORE_CHAR[21]=(sum&0xff00)>>8;  
  SEND_PREFIX(); //����ͨ��Э���ͷ
  for(i=0;i<22;i++)
  {
    USART_SendByte(FP_USARTx,CMD_STORE_CHAR[i]);
  }
}

void SEND_CMD_SLED_CTRL(unsigned char sled)
{
		unsigned char i;
    CMD_SLED_CTRL[4]=sled;
    if(sled==0x01)
    {
        CMD_SLED_CTRL[20]=0x26;
        CMD_SLED_CTRL[21]=0x01;
    }
    else
    {   
        CMD_SLED_CTRL[20]=0x25;
        CMD_SLED_CTRL[21]=0x01;
    }
  SEND_PREFIX(); //����ͨ��Э���ͷ
  for(i=0;i<22;i++)
  {
    USART_SendByte(FP_USARTx,CMD_SLED_CTRL[i]);
  }
  
}


