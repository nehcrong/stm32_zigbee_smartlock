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
#include "idworld.h"   //指纹驱动文件
#include "zigbee.h"
#define TIMEOUT ((uint32_t) 0x200000)//通讯超时时次。
#define FLASH_PAGE_SIZE ((uint16_t)0x400)//写FLASH页数。
#define WRITE_START_ADDR ((uint32_t)0x08008000)//写FLASH地址。

volatile extern unsigned char ExtiFlag;//中断事件标志（0:无中断；1：指纹识别中断；2：触摸按键输入中断；3：联网模块中断。）
extern unsigned int InputVaild;//输入有效控制变量。
unsigned int error;//认证错误计数。
uint32_t Timeout;//等待通讯超时时间。
unsigned char code;//解码后的键值缓存。
unsigned char PWD[16]={0,2,0,0,3,5,0,0,0,0,0,0,0,0,0,6};//密码结构数组：1-15位为密码存储区，第16位为有效密码位数，初始密码6个0。
unsigned char PWDBuf[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//密码修改缓存，1-15位为缓存区，第16位为0/1表示退出/进入密码修改状态，大于1表示缓存的密码位数。
unsigned char InputBuf[16];//输入码缓存，最大输入16位。
unsigned int InputCon=0;//输入码计数器。
int CommandFlag=0;//命令事件触发标志。
void Change_PWD(void);//修改密码。
void InputClr(void);//清空输入码缓存与计数器。
unsigned char Finger_Verify(void);//指纹验证。
void Input_Handle(void);//触摸按键处理。
void Input_Event(void);//触发输入事件。
void Iot_Register(void);//Zigbee物联网注册。
void Iot_Handle(void);//ZigBee物联通讯处理。
unsigned char Command_Event(char cmd);//触发指令事件。
unsigned char Add_FingerPrint(char StoreID);//增加指纹模板。
void UnLock(void);//开锁程序。
void Buzzer(uint32_t t);//蜂鸣器提示声。
void Decode_Input(unsigned char* pCode);//解码IIC传输的键值。
bool IsContain(unsigned char *a,unsigned char *b,int al,int bl);//验证输入码缓存是否包含密码，虚位密码功能。
static void SYSCLKConfig_STOP(void);//停机唤醒后配置系统时钟。

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
		if(InputVaild>=MAX_INTERVAL)//键盘无操作等待超时，关闭键盘背光,清空所有标志，进入停机状态。
		{
			BackLight_OFF;
			InputClr();
			CommandFlag=0;
			PWDBuf[15]=0;
			Buzzer(0x05010000);//即将进入停止模式提示音。
			PWR_EnterSTOPMode(PWR_Regulator_LowPower,PWR_STOPEntry_WFI);//进入停止模式，设置电压调节器为低功耗模式，等待中断唤醒。
			SYSCLKConfig_STOP();//停机唤醒后配置系统时钟。
		}
	}
}

/**
* @brief 指纹验证解锁函数，用于普通指纹解锁。
* @param 无
* @retval 0：验证成功，其他错误码参考文档。
*/
unsigned char Finger_Verify(void)
 {	
	ExtiFlag=0;//清除中断事件标志。	
/*-------------发送采集指纹图像指令--------------*/	
	Timeout=TIMEOUT;
	SEND_CMD_GET_IMAGE();
	while((RxCon!=26)||(RxBuf[4]!=0x20)||(RxBuf[8]!=0x00))
	{
		if((Timeout--)==0)
		{
			
			return RxBuf[8];
		}
	}
/*-------------发送生成指纹特征指令--------------*/		
	Timeout=TIMEOUT;
	SEND_CMD_GENERATE0();
	while((RxCon!=26)||(RxBuf[4]!=0x60)||(RxBuf[8]!=0x00))
	{	
		if((Timeout--)==0)
		{
			
			return RxBuf[8];
		}
	}
/*-------------发送对比指纹模板指令--------------*/	
	Timeout=TIMEOUT;
	SEND_CMD_SEARCH();
	while((RxCon!=26)||(RxBuf[4]!=0x63)||(RxBuf[8]!=0x00))
	{	
		if((Timeout--)==0)
		{
			Buzzer(0x07070000);
			error++;//对比模板失败则错误计数加一。
			return RxBuf[8];
		}
	}
	unsigned char id=RxBuf[10];//暂存解锁指纹ID。
	UnLock();//通过以上三步骤则解锁。
	Zig_BasicPost(0x09,0x01,id);//通知主机解锁的指纹ID。
	return 0;
}

/**
* @brief 单次输入按键预处理，并控制输入间隔。
* @param 无
* @retval 无
*/
void Input_Handle(void)
{	
	BackLight_ON;//开启键盘背光。
	I2C_BufferRead(IIC_ReadBuf, 0x08, 2);//与触摸键盘芯片WT5700通讯，读取相应寄存器值。
	if(InputVaild>=MIN_INTERVAL)//输入有效变量大于最低时间间隔则输入有效。
	{	
		Buzzer(0x01000000);//按键成功提示音。
		Decode_Input(IIC_ReadBuf);//解码寄存器值，存于变量code。
		//USART_SendByte(Zig_USARTx,code);
		//USART_SendByte(FP_USARTx,code);
		if(code<=9||InputCon>0)//第一次按键为数字触发输入事件。
		{Input_Event();}
		else if(code==10||code==11)//第一次按键为*或#触发门铃。
		{}
		InputVaild=0;//读取一次键值后将输入有效变量清0。
		Timer_Start();//设置定时器，输入有效变量递增。
	}
	else//再次触发输入时若输入有效变量没达到时间间隔，则重新清0。
	{InputVaild=0;}
	ExtiFlag=0;
}

/**
* @brief 输入事件综合处理，用于分流键盘输入的各种操作。
* @param 无
* @retval 无
*/
void Input_Event(void)
{
	if(InputCon<15&&code<=9)//输入位数小于15且为数字则保存于输入缓存。
	{
		InputBuf[InputCon]=code;
		InputCon++;
	}
	else if(code==10)//如检测到输入*结尾，验证密码并设置指令输入标志。
	{
		if(CommandFlag==0)
		{
			if(IsContain(InputBuf,PWD,InputCon,PWD[15]))
			{CommandFlag=1;}
			else//密码错误则发出警告，错误计数+1。
			{
				error++;
				
			}
		}
		else//进入指令输入再次按下*则取消指令输入。
		{
			CommandFlag=0;
		}
		InputClr();//清除输入缓存。
		PWDBuf[15]=0;//修改密码过程中按下*则取消密码修改。
	}
	else if(code==11)//如检测到输入#结尾，提交输入的密码或指令。
	{
		if(CommandFlag==0&&PWDBuf[15]==0)//如指令标志与密码修改标志都为0则进行普通密码解锁。
		{
			if(IsContain(InputBuf,PWD,InputCon,PWD[15]))//验证密码
			{				
				UnLock();//密码正确则解锁。
			}
			else//密码错误则发出警告，错误计数+1。
			{
				error++;
				Buzzer(0x05050000);
			}
			InputClr();	//清除输入缓存。
		}
		else if(CommandFlag==0&&PWDBuf[15]>=1)//如指令标志为0且密码修改标志为大于1则进行密码修改。
		{
			Change_PWD();
		}
		else if(CommandFlag==1&&InputCon==1)//如指令标志为1且指令码位数为1则触发指令事件。
		{
			Command_Event(InputBuf[0]);
			InputClr();//清除输入缓存。
			CommandFlag=0;//清除指令标志。
		}
		else//输入非法命令将命令标志及缓存清除。
		{
			InputClr();
			CommandFlag=0;
		}
	}
	else//按键位数超过15位视为输错密码，清除标志缓存，错误+1。
	{
		
		error++;
		InputClr();
		CommandFlag=0;
	}
}

/**
* @brief Zigbee物联网指令处理函数，指令格式参考文档。
* @param 无
* @retval 无
*/
void Iot_Handle(void)
{
	
	ExtiFlag=0;//清除中断事件标志。	
/*-------------等待缓存数据--------------*/
	Timeout=TIMEOUT;
	while((ZigRxBuf[0]!=0xA5)||(ZigRxCon!=20))
	{
		if((Timeout--)==0)
		{	
			ZigRxBufClr();
			return;
		}
	}

/*-------------验证密匙并执行相应指令--------------*/	
	if((ZigRxBuf[2]==0xfd)&&(IsContain(ZigRxBuf,PWD,19,PWD[15])))
		switch(ZigRxBuf[19])
		{
			case 0x01://解锁命令。
				{
				  Zig_BasicPost(0x09,0x01,0x00);//通知主机解锁成功。
					UnLock();
					break;
				}
			case 0x02://修改密码命令。
				{
					for(int i=0;i<16;i++)//数据编号20-36为修改的密码。
						{
							PWD[i]=ZigRxBuf[i+20];
						}
					Flash_Write(PWD,sizeof(PWD));//将修改后的密码写入Flash。
					Zig_BasicPost(0x09,0x02,0x00);//通知主机修改密码成功。
					break;
				}
			case 0x03:
				{
					Command_Event(1);
					break;
				}
			case 0x04://删除指纹命令。
				{
					Zig_BasicPost(0x09,0x04,Command_Event(0));//执行并通知主机。
					break;
				}
			default:break;
		}
	ZigRxBufClr();
}

/**
* @brief Zigbee物联网注册函数，用于提交注册信息给主机。
* @param 无
* @retval 无
*/
void Iot_Register(void)
{
	SysTick_Delay_Ms(500);//延时0.5s等待Zigbee启动。
	Zig_ReadInfo();//读取Zigbee模块所有信息。
	Timeout=TIMEOUT;
	while((ZigRxBuf[0]!=0xfb)||(ZigRxCon!=46))//等待读取全部数据。
	{
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//读取失败，两声长笛音提示。
			return;
		}
	}
	unsigned int i;
	for(i=0;i<46;i++)//将数据复制到Zigbee发送缓存，准备发往主机。
		{ZigTxBuf[i+11]=ZigRxBuf[i];}
	ZigRxBufClr();//清除Zigbee接收缓存。
	Zig_BasicPost(0x37,0x00,0x00);//发送注册信息至主机。
	Buzzer(0x01010000);//发送成功蜂鸣提示。
}

/**
* @brief 命令处理函数，支持9个命令如下。
* @param cmd：命令码。
* @retval 0：命令执行成功，其它返回码参考文档。
*/
unsigned char Command_Event(char cmd)
{	
	switch(cmd)
	{
/*-------------命令0：删除1-9指纹模板--------------*/	
		case 0:
		{
			Timeout=TIMEOUT;
			SEND_CMD_DEL_CHAR();
			while((RxCon!=26)||(RxBuf[4]!=0x44)||(RxBuf[8]!=0x00))
			{
				if((Timeout--)==0)
				{
					Buzzer(0x05050000);//失败报警。
					return RxBuf[8];
				}
			}
			Buzzer(0x01010000);//删除指纹信息成功蜂鸣提示。
			return 0;
		}
/*-------------命令1：新增指纹模板--------------*/	
		case 1:
		{
			Timeout=TIMEOUT;
			SEND_CMD_GET_EMPTY_ID();//发指令查看可注册ID。
			while((RxCon!=26)||(RxBuf[4]!=0x45)||(RxBuf[8]!=0x00)||(RxBuf[10]>0x09))
			{
				if((Timeout--)==0)
				{
					Buzzer(0x05050000);//失败报警。
					return RxBuf[8];
				}
			}
			return Add_FingerPrint(RxBuf[10]);//执行添加指纹模板步骤。
		}
/*-------------命令2：准备修改管理员密码--------------*/	
		case 2:
		{
			{PWDBuf[15]=1;}//设置准备修改密码标志，下次输入则视为修改密码。
			return 0;
		}
/*-------------命令3：设置或取消智能更新指纹--------------*/
/*-------------命令4：安全锁定门锁--------------*/
/*-------------命令5：修改Zigbee联网密码--------------*/
/*-------------命令6：修改Zigbee联网组号--------------*/
/*-------------命令7：手动注册Zigbee联网信息--------------*/
		case 7:
		{
			Iot_Register();
		}
		return 0;
		default:return 1;//没找到命令返回1。
	}
}

/**
* @brief 添加指纹模板函数，用于注册指纹信息，默认允许存储9个指纹。
* @param StoreID：申请注册的指纹编号。
* @retval 0：操作成功，其他错误码参考idworld文档。
*/
unsigned char Add_FingerPrint(char StoreID)
{
/*-------------第1次采集指纹图像--------------*/	
	Timeout=TIMEOUT*20;
	SEND_CMD_SLED_CTRL(1);//打开指纹采集器背光。
	ExtiFlag=0;
	while(ExtiFlag!=1)//等待感应到手指。
	{
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//失败报警。
			return RxBuf[8];
		}
	}
	Timeout=TIMEOUT;
	SEND_CMD_GET_IMAGE();
	while((RxCon!=26)||(RxBuf[4]!=0x20)||(RxBuf[8]!=0x00))
	{
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//失败报警。
			return RxBuf[8];
		}
	}
/*-------------第1次生成指纹特征--------------*/		
	Timeout=TIMEOUT;
	SEND_CMD_GENERATE0();
	while((RxCon!=26)||(RxBuf[4]!=0x60)||(RxBuf[8]!=0x00))
	{	
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//失败报警。
			return RxBuf[8];
		}
	}	
	SEND_CMD_SLED_CTRL(0);//关闭指纹采集器背光。
	SysTick_Delay_Ms(1000);//延时1S。
	Buzzer(0x01000000);//蜂鸣继续提示。
	
/*-------------第2次采集指纹图像--------------*/
	Timeout=TIMEOUT*20;
	SEND_CMD_SLED_CTRL(1);//打开指纹采集器背光。
	ExtiFlag=0;
	while(ExtiFlag!=1)//等待感应到手指。
	{
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//失败报警。
			return RxBuf[8];
		}
	}
	Timeout=TIMEOUT;
	SEND_CMD_GET_IMAGE();
	while((RxCon!=26)||(RxBuf[4]!=0x20)||(RxBuf[8]!=0x00))
	{
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//失败报警。
			return RxBuf[8];
		}
	}
/*-------------第2次生成指纹特征--------------*/		
	Timeout=TIMEOUT;
	SEND_CMD_GENERATE1();
	while((RxCon!=26)||(RxBuf[4]!=0x60)||(RxBuf[8]!=0x00))
	{	
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//失败报警。
			return RxBuf[8];
		}
	}
	SEND_CMD_SLED_CTRL(0);//关闭指纹采集器背光。
	SysTick_Delay_Ms(1000);//延时1S。
	Buzzer(0x01000000);//蜂鸣继续提示。
	
/*-------------第3次采集指纹图像--------------*/
	Timeout=TIMEOUT*20;
	SEND_CMD_SLED_CTRL(1);//打开指纹采集器背光。
	ExtiFlag=0;
	while(ExtiFlag!=1)//等待感应到手指。
	{
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//失败报警。
			return RxBuf[8];
		}
	}	
	Timeout=TIMEOUT;
	SEND_CMD_GET_IMAGE();
	while((RxCon!=26)||(RxBuf[4]!=0x20)||(RxBuf[8]!=0x00))
	{
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//失败报警。
			return RxBuf[8];
		}
	}
/*-------------第3次生成指纹特征--------------*/		
	Timeout=TIMEOUT;
	SEND_CMD_GENERATE2();
	while((RxCon!=26)||(RxBuf[4]!=0x60)||(RxBuf[8]!=0x00))
	{	
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//失败报警。
			return RxBuf[8];
		}
	}
	SEND_CMD_SLED_CTRL(0);//关闭指纹采集器背光。
	SysTick_Delay_Ms(1000);//延时1S。
	ExtiFlag=0;//将中断标志清零避免退出函数时误识别。
/*-------------合并3次指纹特征数据--------------*/	
	Timeout=TIMEOUT;
	SEND_CMD_MERGE();
	while((RxCon!=26)||(RxBuf[4]!=0x61)||(RxBuf[8]!=0x00))
	{	
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//失败报警。
			return RxBuf[8];
		}
	}
	
/*-------------自动编号存储指纹--------------*/	
	Timeout=TIMEOUT;
	SEND_CMD_STORE_CHAR(StoreID);
	while((RxCon!=26)||(RxBuf[4]!=0x61)||(RxBuf[8]!=0x00))
	{	
		if((Timeout--)==0)
		{
			Buzzer(0x05050000);//失败报警。
			return RxBuf[8];
		}
	}
	Buzzer(0x01010000);//存储成功，蜂鸣提示。
	return 0;
}

void Change_PWD(void)
{
	if(PWDBuf[15]==1&&InputCon>=6)//密码修改标志位为1，则是第一次输入新密码，新密码位数需大于或等于6位。
	{
		memcpy(PWDBuf,InputBuf,InputCon);//将输入的新密码存于密码缓存。
		PWDBuf[15]=InputCon;//将密码修改标志位重新赋值，表示密码位数。
		InputClr();//记录完毕清除输入缓存，准备下次输入。
	}
	else if(PWDBuf[15]==InputCon&&InputCon>=6)//密码标志位大于1，则是第二次输入新密码，两次输入的新密码位数需一致再进行对比。
	{
		if(IsContain(PWDBuf,InputBuf,PWDBuf[15],InputCon))//对比两次输入的新密码是否一致。
		{
			memcpy(PWD,PWDBuf,16);//如果一致则将新密码覆盖旧密码。
		}
		PWDBuf[15]=0;	//修改密码后将密码修改标志清除。
		InputClr();
	}
	else//输入的新密码不符合要求则清除缓存及标志，结束密码修改。
	{
		InputClr();
		PWDBuf[15]=0;
	}
}

/**
* @brief WT5700芯片触摸键盘输入解码函数，支持12键值，解码值存放于全局变量code。
* @param pCode:两字节原码。
* @retval 无
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
* @brief 解锁函数，默认解锁时间5s。
* @param 无
* @retval 无
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
* @brief 清除键盘输入缓存。
* @param 无
* @retval 无
*/
void InputClr(void)
{
	while(InputCon>0)
	{
		InputBuf[--InputCon]=0;
	}		
}

/**
* @brief 蜂鸣器提示声函数，最多4段，每声间隔0.2S。
* @param m：t：32位二进制蜂鸣器响声码，每8位为一段发声时间，单位0.1S。
* @retval 无
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
* @brief 密码检查函数，用于判断两个字符串前几位是否相等。
* @param *a：字符串a指针；
* @param *b：字符串b指针；
* @param al：a的比较位数；
* @param bl：b的比较位数。
* @retval true：a的前al位包含b的前bl位；false：a的前al位不包含b的前bl位。
* @retval false：a的前al位不包含b的前bl位。
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
* @brief 停机唤醒后配置系统时钟: 使能 HSE, PLL并且选择PLL 作为系统时钟。
* @param None
* @retval None
*/
static void SYSCLKConfig_STOP(void)
{
	RCC_HSEConfig(RCC_HSE_ON);//使能 HSE
	while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);//等待 HSE 准备就绪。
	RCC_PLLCmd(ENABLE);//使能 PLL。
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);//等待 PLL 准备就绪。
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);//选择PLL 作为系统时钟源。
	while (RCC_GetSYSCLKSource() != 0x08);//等待PLL 被选择为系统时钟源。
}
