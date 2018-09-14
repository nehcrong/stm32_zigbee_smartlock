#include "iic.h"

uint8_t KEYBOARD_ADDRESS=0xe0;
uint8_t IIC_ReadBuf[2];
uint8_t SetCode[4]={0x62,0x00,0xd3,0x03};//WT5700配置码，参考其datasheet。

static void I2C_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 使能与 I2C 有关的时钟 */
	KEYBOARD_I2C_APBxClock_FUN ( KEYBOARD_I2C_CLK, ENABLE );
	KEYBOARD_I2C_GPIO_APBxClock_FUN ( KEYBOARD_I2C_GPIO_CLK, ENABLE );

	/* I2C_SCL、I2C_SDA*/
	GPIO_InitStructure.GPIO_Pin = KEYBOARD_I2C_SCL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD; // 开漏输出
	GPIO_Init(KEYBOARD_I2C_SCL_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = KEYBOARD_I2C_SDA_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD; // 开漏输出
	GPIO_Init(KEYBOARD_I2C_SDA_PORT, &GPIO_InitStructure);
}

/**
* @brief I2C 工作模式配置
* @param 无
* @retval 无
*/
static void I2C_Mode_Config(void)
{
	I2C_InitTypeDef I2C_InitStructure;

	/* I2C 配置 */
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;

	/* 高电平数据稳定，低电平数据变化 SCL 时钟线的占空比 */
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;

	I2C_InitStructure.I2C_OwnAddress1 =I2Cx_OWN_ADDRESS7;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable ;

	/* I2C 的寻址模式 */
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

	/* 通信速率 */
	I2C_InitStructure.I2C_ClockSpeed = I2C_Speed;

	/* I2C 初始化 */
	I2C_Init(KEYBOARD_I2Cx, &I2C_InitStructure);

	/* 使能 I2C */
	I2C_Cmd(KEYBOARD_I2Cx, ENABLE);
}

/***************************************************************/
/*通讯等待超时时间*/
#define I2CT_FLAG_TIMEOUT ((uint32_t)0x1000)
#define I2CT_LONG_TIMEOUT ((uint32_t)(10 * I2CT_FLAG_TIMEOUT))
uint32_t I2CTimeout;

/**
* @brief I2C 等待事件超时的情况下会调用这个函数来处理
* @param errorCode：错误代码，可以用来定位是哪个环节出错.
* @retval 返回0，表示IIC 读取失败.
*/
static uint32_t I2C_TIMEOUT_UserCallback(uint8_t errorCode)
{
	/* 使用串口printf 输出错误信息，方便调试 */
	//KEYBOARD_ERROR("I2C 等待超时!errorCode = %d",errorCode);
	return 0;
}
/**
* @brief 写一个字节到I2C KEYBOARD 中
* @param pBuffer:缓冲区指针
* @param WriteAddr:写地址
* @retval 正常返回1，异常返回0
*/
uint32_t I2C_KEYBOARD_ByteWrite(u8* pBuffer, u8 WriteAddr)
{
	/* 产生I2C 起始信号 */
	I2C_GenerateSTART(KEYBOARD_I2Cx, ENABLE);

	/*设置超时等待时间*/
	I2CTimeout = I2CT_FLAG_TIMEOUT;
	/* 检测 EV5 事件并清除标志*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(0);
	}

	/* 发送KEYBOARD 设备地址 */
	I2C_Send7bitAddress(KEYBOARD_I2Cx, KEYBOARD_ADDRESS,I2C_Direction_Transmitter);

	I2CTimeout = I2CT_FLAG_TIMEOUT;
	/* 检测 EV6 事件并清除标志*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx,
	I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(1);
	}
	//WT5700特有的通讯需求
	I2C_SendData(KEYBOARD_I2Cx, 0x00);
	I2CTimeout = I2CT_FLAG_TIMEOUT;
	/* 检测 EV8 事件并清除标志*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(2);
	}
	/* 发送要写入的KEYBOARD 内部地址(即KEYBOARD 内部存储器的地址) */
	I2C_SendData(KEYBOARD_I2Cx, WriteAddr);

	I2CTimeout = I2CT_FLAG_TIMEOUT;
	/* 检测 EV8 事件并清除标志*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(2);
	}
	/* 发送一字节要写入的数据 */
	I2C_SendData(KEYBOARD_I2Cx, *pBuffer);

	I2CTimeout = I2CT_FLAG_TIMEOUT;
	/* 检测 EV8 事件并清除标志*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx,
	I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(3);
	}

	/* 发送停止信号 */
	I2C_GenerateSTOP(KEYBOARD_I2Cx, ENABLE);

	return 1;
}

/**
* @brief 从KEYBOARD 里面读取一块数据
* @param pBuffer:存放从KEYBOARD 读取的数据的缓冲区指针
* @param ReadAddr:接收数据的KEYBOARD 的地址
* @param NumByteToRead:要从KEYBOARD 读取的字节数
* @retval 正常返回1，异常返回0
*/
uint8_t I2C_BufferRead(uint8_t* pBuffer, uint8_t ReadAddr,u16 NumByteToRead)
{
	I2CTimeout = I2CT_LONG_TIMEOUT;

	while (I2C_GetFlagStatus(KEYBOARD_I2Cx, I2C_FLAG_BUSY))
	{
		if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(9);
	}

	/* 产生I2C 起始信号 */
	I2C_GenerateSTART(KEYBOARD_I2Cx, ENABLE);

	I2CTimeout = I2CT_FLAG_TIMEOUT;

	/* 检测 EV5 事件并清除标志*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(10);
	}

	/* 发送KEYBOARD 设备地址 */
	I2C_Send7bitAddress(KEYBOARD_I2Cx,KEYBOARD_ADDRESS,I2C_Direction_Transmitter);

	I2CTimeout = I2CT_FLAG_TIMEOUT;

	/* 检测 EV6 事件并清除标志*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx,
	I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(11);
	}
	/*通过重新设置PE 位清除EV6 事件 */
	I2C_Cmd(KEYBOARD_I2Cx, ENABLE);
	
	//WT5700特有的通讯需求
	I2C_SendData(KEYBOARD_I2Cx, 0x00);
	I2CTimeout = I2CT_FLAG_TIMEOUT;
	/* 检测 EV8 事件并清除标志*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(2);
	}

	/* 发送要读取的KEYBOARD 内部地址(即KEYBOARD 内部存储器的地址) */
	I2C_SendData(KEYBOARD_I2Cx, ReadAddr);

	I2CTimeout = I2CT_FLAG_TIMEOUT;

	/* 检测 EV8 事件并清除标志*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(12);
	}
	/* 产生第二次I2C 起始信号 */
	I2C_GenerateSTART(KEYBOARD_I2Cx, ENABLE);

	I2CTimeout = I2CT_FLAG_TIMEOUT;

	/* 检测 EV5 事件并清除标志*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(13);
	}
	/* 发送KEYBOARD 设备地址 */
	I2C_Send7bitAddress(KEYBOARD_I2Cx, KEYBOARD_ADDRESS, I2C_Direction_Receiver);

	I2CTimeout = I2CT_FLAG_TIMEOUT;

	/* 检测 EV6 事件并清除标志*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(14);
	}
	/* 读取NumByteToRead 个数据*/
	while (NumByteToRead)
	{
	/*若NumByteToRead=1，表示已经接收到最后一个数据了，
	发送非应答信号，结束传输*/
	if (NumByteToRead == 1)
	{
	/* 发送非应答信号 */
	I2C_AcknowledgeConfig(KEYBOARD_I2Cx, DISABLE);

	/* 发送停止信号 */
	I2C_GenerateSTOP(KEYBOARD_I2Cx, ENABLE);
	}

	I2CTimeout = I2CT_LONG_TIMEOUT;
	while (I2C_CheckEvent(KEYBOARD_I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED)==0)
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(3);
	}
	{
	/*通过I2C，从设备中读取一个字节的数据 */
	*pBuffer = I2C_ReceiveData(KEYBOARD_I2Cx);

	/* 存储数据的指针指向下一个地址 */
	pBuffer++;

	/* 接收数据自减 */
	NumByteToRead--;
	}
	}

	/* 使能应答，方便下一次I2C 传输 */
	I2C_AcknowledgeConfig(KEYBOARD_I2Cx, ENABLE);
	return 1;
}

/**
* @brief I2C 外设(KEYBOARD)初始化
* @param 无
* @retval 无
*/
void I2C_KEYBOARD_Init(void)
{
	uint8_t addr=0x00;
	I2C_GPIO_Config();
	I2C_Mode_Config();
	for(int i=0;i<4;i++)
	{I2C_KEYBOARD_ByteWrite(SetCode+i,addr+i);}//设置WT5700输入模式：单点触摸；灵敏度：20CLOCK
}

