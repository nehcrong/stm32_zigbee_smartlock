#include "idworld.h"
#include "delay.h"
#include "usart.h" 
#include "stm32f10x.h"

unsigned int AddNewFlag=0;     //添加标志 0未添加  大于0添加成功 数值为添加新指纹的号码
unsigned int IdentifyFlag=0;   //0识别识别  大于0识别成功，数值为识别出的指纹号码
unsigned int  CurrentNum=0;    //当前指纹个数
unsigned int  SearchNum=0;   //搜索结果

unsigned char RxCon=0;
unsigned char RxBuf[26];


//IDWORLD指纹模块通信协议定义
unsigned char PREFIX[4] = {0x55,0xAA,0x00,0x00};  //数据包头
unsigned char CMD_GET_IMAGE[22] = {0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0x01}; //采集指纹图像存于ImageBuffer
unsigned char CMD_GENERATE0[22] = {0x60,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x61,0x01}; //从ImageBuffer中提取模板存放于RamBuffer0
unsigned char CMD_GENERATE1[22] = {0x60,0x00,0x02,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x62,0x01}; //从ImageBuffer中提取模板存放于RamBuffer1
unsigned char CMD_GENERATE2[22] = {0x60,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x63,0x01}; //从ImageBuffer中提取模板存放于RamBuffer2
unsigned char CMD_MERGE[22] = {0x61,0x00,0x03,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x66,0x01}; //将3个RamBuffer中数据合并为一个模板
unsigned char CMD_GET_EMPTY_ID[22] = {0x45,0x00,0x04,0x00,0x01,0x00,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x52,0x01};//查询模板库1-9中首个可用编号。
unsigned char CMD_SEARCH[22] = {0x63,0x00,0x06,0x00,0x00,0x00,0x01,0x00,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x72,0x01}; //与1-9号指纹模板对比验证
unsigned char CMD_STORE_CHAR[22] = {0x40,0x00,0x04,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x44,0x01}; //将RamBuffer0中模板存放与模板库1号指纹位置
unsigned char CMD_DEL_CHAR[22] = {0x44,0x00,0x04,0x00,0x01,0x00,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x51,0x01};//删除模板库中1-9号指纹模板
unsigned char CMD_SLED_CTRL[22] = {0x24,0x00,0x02,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x26,0x01};//开启指纹模块灯光
//volatile unsigned char FINGER_NUM;

void RxBufClr(void)	  //接收缓存清0
{
  unsigned char i=0;
  for(i=0;i<26;i++)
  {
    RxBuf[i]=0;
  }
  RxCon=0;
}


//发送包头
void SEND_PREFIX(void)  //发送命令头
{
   unsigned char  i;

   RxBufClr();
   for(i=0;i<4;i++)  
   {
     USART_SendByte(FP_USARTx,PREFIX[i]);   
   }
}

//FINGERPRINT_获得指纹图像命令
void SEND_CMD_GET_IMAGE(void)
{
  unsigned char i;
  
  SEND_PREFIX(); //发送通信协议包头
  for(i=0;i<22;i++) //发送命令
     USART_SendByte(FP_USARTx,CMD_GET_IMAGE[i]);
}

//将图像转换成特征码存放在Buffer0中
void SEND_CMD_GENERATE0(void)
{
  unsigned char i;
      
  SEND_PREFIX(); //发送通信协议包头         
  for(i=0;i<22;i++)   //发送命令 将图像转换成 特征码 存放在buffer0
  {
    USART_SendByte(FP_USARTx,CMD_GENERATE0[i]);
  }
}

//将图像转换成特征码存放在Buffer1中
void SEND_CMD_GENERATE1(void)
{
   unsigned char i;
   
   SEND_PREFIX(); //发送通信协议包头
   for(i=0;i<22;i++)   //发送命令 将图像转换成 特征码 存放在 CHAR_buffer1
   {
	  USART_SendByte(FP_USARTx,CMD_GENERATE1[i]);
   }
}

//将图像转换成特征码存放在Buffer2中
void SEND_CMD_GENERATE2(void)
{
   unsigned char i;
   
   SEND_PREFIX(); //发送通信协议包头
   for(i=0;i<22;i++)   //发送命令 将图像转换成 特征码 存放在 CHAR_buffer1
   {
	  USART_SendByte(FP_USARTx,CMD_GENERATE2[i]);
   }
}

//将3个RamBuffer中数据合并为一个模板
void SEND_CMD_MERGE(void)
{
   unsigned char i;    
   
   SEND_PREFIX(); //发送通信协议包头
   for(i=0;i<22;i++) //命令合并指纹模版
   {
     USART_SendByte(FP_USARTx,CMD_MERGE[i]);   
   }
}

void SEND_CMD_GET_EMPTY_ID(void)
{
	unsigned char i;      
  SEND_PREFIX(); //发送通信协议包头
  for(i=0;i<22;i++) //命令合并指纹模版
     USART_SendByte(FP_USARTx,CMD_GET_EMPTY_ID[i]);   
}

//删除指纹模块里的1-9指纹模版
void SEND_CMD_DEL_CHAR(void)
{
  unsigned char i;    
  
  SEND_PREFIX(); //发送通信协议包头
  for(i=0;i<22;i++) //命令合并指纹模版
     USART_SendByte(FP_USARTx,CMD_DEL_CHAR[i]);   
}



//与1-9号指纹模板对比验证
void SEND_CMD_SEARCH(void)
{
  unsigned char i;	   

  SEND_PREFIX(); //发送通信协议包头
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
  SEND_PREFIX(); //发送通信协议包头
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
  SEND_PREFIX(); //发送通信协议包头
  for(i=0;i<22;i++)
  {
    USART_SendByte(FP_USARTx,CMD_SLED_CTRL[i]);
  }
  
}


