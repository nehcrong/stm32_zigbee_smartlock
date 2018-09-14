#include "stm32f10x.h"
#include "stm32f10x_pwr.h"
#include <stdio.h>
#include <stdbool.h>
//#include <stdlib.h>
#include "string.h"
//#include "sys.h"
#include "iic.h"
#include "port.h"
#include "delay.h"
#include "timer.h"
#include "exti.h"
#include "usart.h"
#include "flash.h"
#include "idworld.h"   //ָ�������ļ�
#include "zigbee.h"
#define TIMEOUT ((uint32_t) 0x200000)//ͨѶ��ʱʱ�Ρ�
#define FLASH_PAGE_SIZE ((uint16_t)0x400)//дFLASHҳ����
#define WRITE_START_ADDR ((uint32_t)0x08008000)//дFLASH��ַ��

volatile extern unsigned char ExtiFlag;//�ж��¼���־��0:���жϣ�1��ָ��ʶ���жϣ�2���������������жϣ�3������ģ���жϡ���
extern unsigned int InputVaild;//������Ч���Ʊ�����
unsigned int error;//��֤���������
uint32_t Timeout;//�ȴ�ͨѶ��ʱʱ�䡣
unsigned char code;//�����ļ�ֵ���档
unsigned char PWD[16]={0,2,0,0,3,5,0,0,0,0,0,0,0,0,0,6};//����ṹ���飺1-15λΪ����洢������16λΪ��Ч����λ������ʼ����6��0��
unsigned char PWDBuf[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//�����޸Ļ��棬1-15λΪ����������16λΪ0/1��ʾ�˳�/���������޸�״̬������1��ʾ���������λ����
unsigned char InputBuf[16];//�����뻺�棬�������16λ��
unsigned int InputCon=0;//�������������
int CommandFlag=0;//�����¼�������־��
void Change_PWD(void);//�޸����롣
void InputClr(void);//��������뻺�����������
unsigned char Finger_Verify(void);//ָ����֤��
void Input_Handle(void);//������������
void Input_Event(void);//���������¼���
void Iot_Register(void);//Zigbee������ע�ᡣ
void Iot_Handle(void);//ZigBee����ͨѶ����
unsigned char Command_Event(char cmd);//����ָ���¼���
unsigned char Add_FingerPrint(char StoreID);//����ָ��ģ�塣
void UnLock(void);//��������
void Buzzer(uint32_t t);//��������ʾ����
void Decode_Input(unsigned char* pCode);//����IIC����ļ�ֵ��
bool IsContain(unsigned char *a,unsigned char *b,int al,int bl);//��֤�����뻺���Ƿ�������룬��λ���빦�ܡ�
static void SYSCLKConfig_STOP(void);//ͣ�����Ѻ�����ϵͳʱ�ӡ�

void Delay(uint32_t nCount)
{
  for(; nCount != 0; nCount--);
}

int main(void)
{	
	USART_Config();
  PortInit();
	I2C_KEYBOARD_Init();
	Timer_Init();
	Iot_Register();
	//GPIO_SetBits(GPIOB,GPIO_Pin_8);
  EXTI_Config();
	//InternalFlash_Test();
	//WriteProtect_Release();
	//t[0]=Flash_Write(PWD,sizeof(PWD));
	//Flash_Read(PWDBuf,sizeof(PWDBuf));
	//Zig_Set();

	while(1)
	{
		if(error<3)
		{	
			switch(ExtiFlag)
			{
				case 0: break;
				case 1: Finger_Verify();break;
				case 2: Input_Handle();break;
				case 3: Iot_Handle();break;
				default: ExtiFlag=0;
			}
		}
		else
		{
			
		}
		if(InputVaild>=MAX_INTERVAL)//�����޲����ȴ���ʱ���رռ��̱���,������б�־������ͣ��״̬��
		{
			BackLight_OFF;
			InputClr();
			CommandFlag=0;
			PWDBuf[15]=0;
			Buzzer(0x05010000);//��������ֹͣģʽ��ʾ����
			PWR_EnterSTOPMode(PWR_Regulator_LowPower,PWR_STOPEntry_WFI);//����ֹͣģʽ�����õ�ѹ������Ϊ�͹���ģʽ���ȴ��жϻ��ѡ�
			SYSCLKConfig_STOP();//ͣ�����Ѻ�����ϵͳʱ�ӡ�
		}
	}
}

/**
* @brief ָ����֤����������������ָͨ�ƽ�����
* @param ��
* @retval 0����֤�ɹ�������������ο��ĵ���
*/
unsigned char Finger_Verify(void)
 {	
	ExtiFlag=0;//����ж��¼���־��	
/*-------------���Ͳɼ�ָ��ͼ��ָ��--------------*/	
	Timeout=TIMEOUT;
	SEND_CMD_GET_IMAGE();
	while((RxCon!=26)||(RxBuf[4]!=0x20)||(RxBuf[8]!=0x00))
	{
		if((Timeout--)==0)
		{
			
			return RxBuf[8];
		}
	}
/*-------------��������ָ������ָ��--------------*/		
	Timeout=TIMEOUT;
	SEND_CMD_GENERATE0();
	while((RxCon!=26)||(RxBuf[4]!=0x60)||(RxBuf[8]!=0x00))
	{	
		if((Timeout--)==0)
		{
			
			return RxBuf[8];
		}
	}
/*-------------���ͶԱ�ָ��ģ��ָ��--------------*/	
	Timeout=TIMEOUT;
	SEND_CMD_SEARCH();
	while((RxCon!=26)||(RxBuf[4]!=0x63)||(RxBuf[8]!=0x00))
	{	
		if((Timeout--)==0)
		{
			Buzzer(0x07070000);
			error++;//�Ա�ģ��ʧ������������һ��
			return RxBuf[8];
		}
	}
	unsigned char id=RxBuf[10];//�ݴ����ָ��ID��
	UnLock();//ͨ�������������������
	Zig_BasicPost(0x09,0x01,id);//֪ͨ����������ָ��ID��
	return 0;
}

/**
* @brief �������밴��Ԥ������������������
* @param ��
* @retval ��
*/
void Input_Handle(void)
{	
	BackLight_ON;//�������̱��⡣
	I2C_BufferRead(IIC_ReadBuf, 0x08, 2);//�봥������оƬWT5700ͨѶ����ȡ��Ӧ�Ĵ���ֵ��
	if(InputVaild>=MIN_INTERVAL)//������Ч�����������ʱ������������Ч��
	{	
		Buzzer(0x01000000);//�����ɹ���ʾ����
		Decode_Input(IIC_ReadBuf);//����Ĵ���ֵ�����ڱ���code��
		//USART_SendByte(Zig_USARTx,code);
		//USART_SendByte(FP_USARTx,code);
		if(code<=9||InputCon>0)//��һ�ΰ���Ϊ���ִ��������¼���
		{Input_Event();}
		else if(code==10||code==11)//��һ�ΰ���Ϊ*��#�������塣
		{}
		InputVaild=0;//��ȡһ�μ�ֵ��������Ч������0��
		Timer_Start();//���ö�ʱ����������Ч����������
	}
	else//�ٴδ�������ʱ��������Ч����û�ﵽʱ��������������0��
	{InputVaild=0;}
	ExtiFlag=0;
}

/**
* @brief �����¼��ۺϴ������ڷ�����������ĸ��ֲ�����
* @param ��
* @retval ��
*/
void Input_Event(void)
{
	if(InputCon<15&&code<=9)//����λ��С��15��Ϊ�����򱣴������뻺�档
	{
		InputBuf[InputCon]=code;
		InputCon++;
	}
	else if(code==10)//���⵽����*��β����֤���벢����ָ�������־��
	{
		if(CommandFlag==0)
		{
			if(IsContain(InputBuf,PWD,InputCon,PWD[15]))
			{CommandFlag=1;}
			else//��������򷢳����棬�������+1��
			{
				error++;
				
			}
		}
		else//����ָ�������ٴΰ���*��ȡ��ָ�����롣
		{
			CommandFlag=0;
		}
		InputClr();//������뻺�档
		PWDBuf[15]=0;//�޸���������а���*��ȡ�������޸ġ�
	}
	else if(code==11)//���⵽����#��β���ύ����������ָ�
	{
		if(CommandFlag==0&&PWDBuf[15]==0)//��ָ���־�������޸ı�־��Ϊ0�������ͨ���������
		{
			if(IsContain(InputBuf,PWD,InputCon,PWD[15]))//��֤����
			{				
				UnLock();//������ȷ�������
			}
			else//��������򷢳����棬�������+1��
			{
				error++;
				Buzzer(0x05050000);
			}
			InputClr();	//������뻺�档
		}
		else if(CommandFlag==0&&PWDBuf[15]>=1)//��ָ���־Ϊ0�������޸ı�־Ϊ����1����������޸ġ�
		{
			Change_PWD();
		}
		else if(CommandFlag==1&&InputCon==1)//��ָ���־Ϊ1��ָ����λ��Ϊ1�򴥷�ָ���¼���
		{
			Command_Event(InputBuf[0]);
			InputClr();//������뻺�档
			CommandFlag=0;//���ָ���־��
		}
		else//����Ƿ���������־�����������
		{
			InputClr();
			CommandFlag=0;
		}
	}
	else//����λ������15λ��Ϊ������룬�����־���棬����+1��
	{
		
		error++;
		InputClr();
		CommandFlag=0;
	}
}

/**
* @brief Zigbee������ָ�������ָ���ʽ�ο��ĵ���
* @param ��
* @retval ��
*/
void Iot_Handle(void)
{
	
	ExtiFlag=0;//����ж��¼���־��	
/*-------------�ȴ���������--------------*/
	Timeout=TIMEOUT;
	while((ZigRxBuf[0]!=0xA5)||(ZigRxCon!=20))
	{
		if((Timeout--)==0)
		{	
			ZigRxBufClr();
			return;
		}
	}

/*-------------��֤�ܳײ�ִ����Ӧָ��--------------*/	
	if((ZigRxBuf[2]==0xfd)&&(IsContain(ZigRxBuf,PWD,19,PWD[15])))
		switch(ZigRxBuf[19])
		{
			case 0x01://�������
				{
				  Zig_BasicPost(0x09,0x01,0x00);//֪ͨ���������ɹ���
					UnLock();
					break;
				}
			case 0x02://�޸��������
				{
					for(int i=0;i<16;i++)//���ݱ��20-36Ϊ�޸ĵ����롣
						{
							PWD[i]=ZigRxBuf[i+20];
						}
					Flash_Write(PWD,sizeof(PWD));//���޸ĺ������д��Flash��
					Zig_BasicPost(0x09,0x02,0x00);//֪ͨ�����޸�����ɹ���
					break;
				}
			case 0x03:
				{
					Command_Event(1);
					break;
				}
			case 0x04://ɾ��ָ�����
				{
					Zig_BasicPost(0x09,0x04,Command_Event(0));//ִ�в�֪ͨ������
					break;
				}
			default:break;
		}
	ZigRxBufClr();
}

/**
* @brief Zigbee������ע�ắ���������ύע����Ϣ��������
* @param ��
* @retval ��
*/
void Iot_Register(void)
{
	SysTick_Delay_Ms(500);//��ʱ0.5s�ȴ�Zigbee������
	Zig_ReadInfo();//��ȡZigbeeģ��������Ϣ��
	Timeout=TIMEOUT;
	while((ZigRxBuf[0]!=0xfb)||(ZigRxCon!=46))//�ȴ���ȡȫ�����ݡ�
	{
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//��ȡʧ�ܣ�������������ʾ��
			return;
		}
	}
	unsigned int i;
	for(i=0;i<46;i++)//�����ݸ��Ƶ�Zigbee���ͻ��棬׼������������
		{ZigTxBuf[i+11]=ZigRxBuf[i];}
	ZigRxBufClr();//���Zigbee���ջ��档
	Zig_BasicPost(0x37,0x00,0x00);//����ע����Ϣ��������
	Buzzer(0x01010000);//���ͳɹ�������ʾ��
}

/**
* @brief ���������֧��9���������¡�
* @param cmd�������롣
* @retval 0������ִ�гɹ�������������ο��ĵ���
*/
unsigned char Command_Event(char cmd)
{	
	switch(cmd)
	{
/*-------------����0��ɾ��1-9ָ��ģ��--------------*/	
		case 0:
		{
			Timeout=TIMEOUT;
			SEND_CMD_DEL_CHAR();
			while((RxCon!=26)||(RxBuf[4]!=0x44)||(RxBuf[8]!=0x00))
			{
				if((Timeout--)==0)
				{
					Buzzer(0x05050000);//ʧ�ܱ�����
					return RxBuf[8];
				}
			}
			Buzzer(0x01010000);//ɾ��ָ����Ϣ�ɹ�������ʾ��
			return 0;
		}
/*-------------����1������ָ��ģ��--------------*/	
		case 1:
		{
			Timeout=TIMEOUT;
			SEND_CMD_GET_EMPTY_ID();//��ָ��鿴��ע��ID��
			while((RxCon!=26)||(RxBuf[4]!=0x45)||(RxBuf[8]!=0x00)||(RxBuf[10]>0x09))
			{
				if((Timeout--)==0)
				{
					Buzzer(0x05050000);//ʧ�ܱ�����
					return RxBuf[8];
				}
			}
			return Add_FingerPrint(RxBuf[10]);//ִ�����ָ��ģ�岽�衣
		}
/*-------------����2��׼���޸Ĺ���Ա����--------------*/	
		case 2:
		{
			{PWDBuf[15]=1;}//����׼���޸������־���´���������Ϊ�޸����롣
			return 0;
		}
/*-------------����3�����û�ȡ�����ܸ���ָ��--------------*/
/*-------------����4����ȫ��������--------------*/
/*-------------����5���޸�Zigbee��������--------------*/
/*-------------����6���޸�Zigbee�������--------------*/
/*-------------����7���ֶ�ע��Zigbee������Ϣ--------------*/
		case 7:
		{
			Iot_Register();
		}
		return 0;
		default:return 1;//û�ҵ������1��
	}
}

/**
* @brief ���ָ��ģ�庯��������ע��ָ����Ϣ��Ĭ������洢9��ָ�ơ�
* @param StoreID������ע���ָ�Ʊ�š�
* @retval 0�������ɹ�������������ο�idworld�ĵ���
*/
unsigned char Add_FingerPrint(char StoreID)
{
/*-------------��1�βɼ�ָ��ͼ��--------------*/	
	Timeout=TIMEOUT*20;
	SEND_CMD_SLED_CTRL(1);//��ָ�Ʋɼ������⡣
	ExtiFlag=0;
	while(ExtiFlag!=1)//�ȴ���Ӧ����ָ��
	{
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//ʧ�ܱ�����
			return RxBuf[8];
		}
	}
	Timeout=TIMEOUT;
	SEND_CMD_GET_IMAGE();
	while((RxCon!=26)||(RxBuf[4]!=0x20)||(RxBuf[8]!=0x00))
	{
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//ʧ�ܱ�����
			return RxBuf[8];
		}
	}
/*-------------��1������ָ������--------------*/		
	Timeout=TIMEOUT;
	SEND_CMD_GENERATE0();
	while((RxCon!=26)||(RxBuf[4]!=0x60)||(RxBuf[8]!=0x00))
	{	
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//ʧ�ܱ�����
			return RxBuf[8];
		}
	}	
	SEND_CMD_SLED_CTRL(0);//�ر�ָ�Ʋɼ������⡣
	SysTick_Delay_Ms(1000);//��ʱ1S��
	Buzzer(0x01000000);//����������ʾ��
	
/*-------------��2�βɼ�ָ��ͼ��--------------*/
	Timeout=TIMEOUT*20;
	SEND_CMD_SLED_CTRL(1);//��ָ�Ʋɼ������⡣
	ExtiFlag=0;
	while(ExtiFlag!=1)//�ȴ���Ӧ����ָ��
	{
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//ʧ�ܱ�����
			return RxBuf[8];
		}
	}
	Timeout=TIMEOUT;
	SEND_CMD_GET_IMAGE();
	while((RxCon!=26)||(RxBuf[4]!=0x20)||(RxBuf[8]!=0x00))
	{
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//ʧ�ܱ�����
			return RxBuf[8];
		}
	}
/*-------------��2������ָ������--------------*/		
	Timeout=TIMEOUT;
	SEND_CMD_GENERATE1();
	while((RxCon!=26)||(RxBuf[4]!=0x60)||(RxBuf[8]!=0x00))
	{	
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//ʧ�ܱ�����
			return RxBuf[8];
		}
	}
	SEND_CMD_SLED_CTRL(0);//�ر�ָ�Ʋɼ������⡣
	SysTick_Delay_Ms(1000);//��ʱ1S��
	Buzzer(0x01000000);//����������ʾ��
	
/*-------------��3�βɼ�ָ��ͼ��--------------*/
	Timeout=TIMEOUT*20;
	SEND_CMD_SLED_CTRL(1);//��ָ�Ʋɼ������⡣
	ExtiFlag=0;
	while(ExtiFlag!=1)//�ȴ���Ӧ����ָ��
	{
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//ʧ�ܱ�����
			return RxBuf[8];
		}
	}	
	Timeout=TIMEOUT;
	SEND_CMD_GET_IMAGE();
	while((RxCon!=26)||(RxBuf[4]!=0x20)||(RxBuf[8]!=0x00))
	{
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//ʧ�ܱ�����
			return RxBuf[8];
		}
	}
/*-------------��3������ָ������--------------*/		
	Timeout=TIMEOUT;
	SEND_CMD_GENERATE2();
	while((RxCon!=26)||(RxBuf[4]!=0x60)||(RxBuf[8]!=0x00))
	{	
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//ʧ�ܱ�����
			return RxBuf[8];
		}
	}
	SEND_CMD_SLED_CTRL(0);//�ر�ָ�Ʋɼ������⡣
	SysTick_Delay_Ms(1000);//��ʱ1S��
	ExtiFlag=0;//���жϱ�־��������˳�����ʱ��ʶ��
/*-------------�ϲ�3��ָ����������--------------*/	
	Timeout=TIMEOUT;
	SEND_CMD_MERGE();
	while((RxCon!=26)||(RxBuf[4]!=0x61)||(RxBuf[8]!=0x00))
	{	
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//ʧ�ܱ�����
			return RxBuf[8];
		}
	}
	
/*-------------�Զ���Ŵ洢ָ��--------------*/	
	Timeout=TIMEOUT;
	SEND_CMD_STORE_CHAR(StoreID);
	while((RxCon!=26)||(RxBuf[4]!=0x61)||(RxBuf[8]!=0x00))
	{	
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//ʧ�ܱ�����
			return RxBuf[8];
		}
	}
	Buzzer(0x01010000);//�洢�ɹ���������ʾ��
	return 0;
}

void Change_PWD(void)
{
	if(PWDBuf[15]==1&&InputCon>=6)//�����޸ı�־λΪ1�����ǵ�һ�����������룬������λ������ڻ����6λ��
	{
		memcpy(PWDBuf,InputBuf,InputCon);//�������������������뻺�档
		PWDBuf[15]=InputCon;//�������޸ı�־λ���¸�ֵ����ʾ����λ����
		InputClr();//��¼���������뻺�棬׼���´����롣
	}
	else if(PWDBuf[15]==InputCon&&InputCon>=6)//�����־λ����1�����ǵڶ������������룬���������������λ����һ���ٽ��жԱȡ�
	{
		if(IsContain(PWDBuf,InputBuf,PWDBuf[15],InputCon))//�Ա�����������������Ƿ�һ�¡�
		{
			memcpy(PWD,PWDBuf,16);//���һ���������븲�Ǿ����롣
		}
		PWDBuf[15]=0;	//�޸�����������޸ı�־�����
		InputClr();
	}
	else//����������벻����Ҫ����������漰��־�����������޸ġ�
	{
		InputClr();
		PWDBuf[15]=0;
	}
}

/**
* @brief WT5700оƬ��������������뺯����֧��12��ֵ������ֵ�����ȫ�ֱ���code��
* @param pCode:���ֽ�ԭ�롣
* @retval ��
*/
void Decode_Input(unsigned char* pCode)
{
	switch(*pCode)
	{
		case 0x00:
		{
			switch(*++pCode)
			{
				case 0x08:code=1;break;
				case 0x10:code=2;break;
				case 0x04:code=4;break;
				case 0x20:code=5;break;
				case 0x02:code=7;break;
				case 0x80:code=8;break;
				case 0x01:code=10;break;
				case 0x40:code=0;break;
			}			
		}
		break;
		case 0x01:code=3;break;
		case 0x02:code=6;break;
		case 0x04:code=9;break;
		case 0x08:code=11;break;
	}
}
/**
* @brief ����������Ĭ�Ͻ���ʱ��5s��
* @param ��
* @retval ��
*/
void UnLock(void)
{
	CLUTCH_ON;
	SEND_CMD_SLED_CTRL(1);
	Buzzer(0x01010101);
	CLUTCH_RESET;
	SysTick_Delay_Ms(5000);
	CLUTCH_OFF;
	SEND_CMD_SLED_CTRL(0);
	Buzzer(0x01010300);
	CLUTCH_RESET;
	error=0;
}

/**
* @brief ����������뻺�档
* @param ��
* @retval ��
*/
void InputClr(void)
{
	while(InputCon>0)
	{
		InputBuf[--InputCon]=0;
	}		
}

/**
* @brief ��������ʾ�����������4�Σ�ÿ�����0.2S��
* @param m��t��32λ�����Ʒ����������룬ÿ8λΪһ�η���ʱ�䣬��λ0.1S��
* @retval ��
*/
void Buzzer(uint32_t t)
{
	for(int i=24;i>=0;i-=8)
	{
		uint32_t temp=(t>>i)&0x000000ff;
		if(temp>0)
		{			
			BUZZER_ON;
			SysTick_Delay_Ms(temp*100);
			BUZZER_OFF;
			SysTick_Delay_Ms(100);
		}
	}
}

/**
* @brief �����麯���������ж������ַ���ǰ��λ�Ƿ���ȡ�
* @param *a���ַ���aָ�룻
* @param *b���ַ���bָ�룻
* @param al��a�ıȽ�λ����
* @param bl��b�ıȽ�λ����
* @retval true��a��ǰalλ����b��ǰblλ��false��a��ǰalλ������b��ǰblλ��
* @retval false��a��ǰalλ������b��ǰblλ��
*/
bool IsContain(unsigned char *a,unsigned char *b,int al,int bl)
{
	int i=0,j=0;
	while(j<=(al-bl))
	{
		if(i<bl)
		{
			if(a[i+j]==b[i])
			{i++;}
			else
			{
				i=0;
				j++;
			}
		}
		else
		return true;
	}
	return false;
}

/**
* @brief ͣ�����Ѻ�����ϵͳʱ��: ʹ�� HSE, PLL����ѡ��PLL ��Ϊϵͳʱ�ӡ�
* @param None
* @retval None
*/
static void SYSCLKConfig_STOP(void)
{
	RCC_HSEConfig(RCC_HSE_ON);//ʹ�� HSE
	while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);//�ȴ� HSE ׼��������
	RCC_PLLCmd(ENABLE);//ʹ�� PLL��
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);//�ȴ� PLL ׼��������
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);//ѡ��PLL ��Ϊϵͳʱ��Դ��
	while (RCC_GetSYSCLKSource() != 0x08);//�ȴ�PLL ��ѡ��Ϊϵͳʱ��Դ��
}
