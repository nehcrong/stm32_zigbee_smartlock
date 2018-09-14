#include "iic.h"

uint8_t KEYBOARD_ADDRESS=0xe0;
uint8_t IIC_ReadBuf[2];
uint8_t SetCode[4]={0x62,0x00,0xd3,0x03};//WT5700�����룬�ο���datasheet��

static void I2C_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* ʹ���� I2C �йص�ʱ�� */
	KEYBOARD_I2C_APBxClock_FUN ( KEYBOARD_I2C_CLK, ENABLE );
	KEYBOARD_I2C_GPIO_APBxClock_FUN ( KEYBOARD_I2C_GPIO_CLK, ENABLE );

	/* I2C_SCL��I2C_SDA*/
	GPIO_InitStructure.GPIO_Pin = KEYBOARD_I2C_SCL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD; // ��©���
	GPIO_Init(KEYBOARD_I2C_SCL_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = KEYBOARD_I2C_SDA_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD; // ��©���
	GPIO_Init(KEYBOARD_I2C_SDA_PORT, &GPIO_InitStructure);
}

/**
* @brief I2C ����ģʽ����
* @param ��
* @retval ��
*/
static void I2C_Mode_Config(void)
{
	I2C_InitTypeDef I2C_InitStructure;

	/* I2C ���� */
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;

	/* �ߵ�ƽ�����ȶ����͵�ƽ���ݱ仯 SCL ʱ���ߵ�ռ�ձ� */
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;

	I2C_InitStructure.I2C_OwnAddress1 =I2Cx_OWN_ADDRESS7;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable ;

	/* I2C ��Ѱַģʽ */
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

	/* ͨ������ */
	I2C_InitStructure.I2C_ClockSpeed = I2C_Speed;

	/* I2C ��ʼ�� */
	I2C_Init(KEYBOARD_I2Cx, &I2C_InitStructure);

	/* ʹ�� I2C */
	I2C_Cmd(KEYBOARD_I2Cx, ENABLE);
}

/***************************************************************/
/*ͨѶ�ȴ���ʱʱ��*/
#define I2CT_FLAG_TIMEOUT ((uint32_t)0x1000)
#define I2CT_LONG_TIMEOUT ((uint32_t)(10 * I2CT_FLAG_TIMEOUT))
uint32_t I2CTimeout;

/**
* @brief I2C �ȴ��¼���ʱ������»�����������������
* @param errorCode��������룬����������λ���ĸ����ڳ���.
* @retval ����0����ʾIIC ��ȡʧ��.
*/
static uint32_t I2C_TIMEOUT_UserCallback(uint8_t errorCode)
{
	/* ʹ�ô���printf ���������Ϣ��������� */
	//KEYBOARD_ERROR("I2C �ȴ���ʱ!errorCode = %d",errorCode);
	return 0;
}
/**
* @brief дһ���ֽڵ�I2C KEYBOARD ��
* @param pBuffer:������ָ��
* @param WriteAddr:д��ַ
* @retval ��������1���쳣����0
*/
uint32_t I2C_KEYBOARD_ByteWrite(u8* pBuffer, u8 WriteAddr)
{
	/* ����I2C ��ʼ�ź� */
	I2C_GenerateSTART(KEYBOARD_I2Cx, ENABLE);

	/*���ó�ʱ�ȴ�ʱ��*/
	I2CTimeout = I2CT_FLAG_TIMEOUT;
	/* ��� EV5 �¼��������־*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(0);
	}

	/* ����KEYBOARD �豸��ַ */
	I2C_Send7bitAddress(KEYBOARD_I2Cx, KEYBOARD_ADDRESS,I2C_Direction_Transmitter);

	I2CTimeout = I2CT_FLAG_TIMEOUT;
	/* ��� EV6 �¼��������־*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx,
	I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(1);
	}
	//WT5700���е�ͨѶ����
	I2C_SendData(KEYBOARD_I2Cx, 0x00);
	I2CTimeout = I2CT_FLAG_TIMEOUT;
	/* ��� EV8 �¼��������־*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(2);
	}
	/* ����Ҫд���KEYBOARD �ڲ���ַ(��KEYBOARD �ڲ��洢���ĵ�ַ) */
	I2C_SendData(KEYBOARD_I2Cx, WriteAddr);

	I2CTimeout = I2CT_FLAG_TIMEOUT;
	/* ��� EV8 �¼��������־*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(2);
	}
	/* ����һ�ֽ�Ҫд������� */
	I2C_SendData(KEYBOARD_I2Cx, *pBuffer);

	I2CTimeout = I2CT_FLAG_TIMEOUT;
	/* ��� EV8 �¼��������־*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx,
	I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(3);
	}

	/* ����ֹͣ�ź� */
	I2C_GenerateSTOP(KEYBOARD_I2Cx, ENABLE);

	return 1;
}

/**
* @brief ��KEYBOARD �����ȡһ������
* @param pBuffer:��Ŵ�KEYBOARD ��ȡ�����ݵĻ�����ָ��
* @param ReadAddr:�������ݵ�KEYBOARD �ĵ�ַ
* @param NumByteToRead:Ҫ��KEYBOARD ��ȡ���ֽ���
* @retval ��������1���쳣����0
*/
uint8_t I2C_BufferRead(uint8_t* pBuffer, uint8_t ReadAddr,u16 NumByteToRead)
{
	I2CTimeout = I2CT_LONG_TIMEOUT;

	while (I2C_GetFlagStatus(KEYBOARD_I2Cx, I2C_FLAG_BUSY))
	{
		if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(9);
	}

	/* ����I2C ��ʼ�ź� */
	I2C_GenerateSTART(KEYBOARD_I2Cx, ENABLE);

	I2CTimeout = I2CT_FLAG_TIMEOUT;

	/* ��� EV5 �¼��������־*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(10);
	}

	/* ����KEYBOARD �豸��ַ */
	I2C_Send7bitAddress(KEYBOARD_I2Cx,KEYBOARD_ADDRESS,I2C_Direction_Transmitter);

	I2CTimeout = I2CT_FLAG_TIMEOUT;

	/* ��� EV6 �¼��������־*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx,
	I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(11);
	}
	/*ͨ����������PE λ���EV6 �¼� */
	I2C_Cmd(KEYBOARD_I2Cx, ENABLE);
	
	//WT5700���е�ͨѶ����
	I2C_SendData(KEYBOARD_I2Cx, 0x00);
	I2CTimeout = I2CT_FLAG_TIMEOUT;
	/* ��� EV8 �¼��������־*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(2);
	}

	/* ����Ҫ��ȡ��KEYBOARD �ڲ���ַ(��KEYBOARD �ڲ��洢���ĵ�ַ) */
	I2C_SendData(KEYBOARD_I2Cx, ReadAddr);

	I2CTimeout = I2CT_FLAG_TIMEOUT;

	/* ��� EV8 �¼��������־*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(12);
	}
	/* �����ڶ���I2C ��ʼ�ź� */
	I2C_GenerateSTART(KEYBOARD_I2Cx, ENABLE);

	I2CTimeout = I2CT_FLAG_TIMEOUT;

	/* ��� EV5 �¼��������־*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(13);
	}
	/* ����KEYBOARD �豸��ַ */
	I2C_Send7bitAddress(KEYBOARD_I2Cx, KEYBOARD_ADDRESS, I2C_Direction_Receiver);

	I2CTimeout = I2CT_FLAG_TIMEOUT;

	/* ��� EV6 �¼��������־*/
	while (!I2C_CheckEvent(KEYBOARD_I2Cx,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(14);
	}
	/* ��ȡNumByteToRead ������*/
	while (NumByteToRead)
	{
	/*��NumByteToRead=1����ʾ�Ѿ����յ����һ�������ˣ�
	���ͷ�Ӧ���źţ���������*/
	if (NumByteToRead == 1)
	{
	/* ���ͷ�Ӧ���ź� */
	I2C_AcknowledgeConfig(KEYBOARD_I2Cx, DISABLE);

	/* ����ֹͣ�ź� */
	I2C_GenerateSTOP(KEYBOARD_I2Cx, ENABLE);
	}

	I2CTimeout = I2CT_LONG_TIMEOUT;
	while (I2C_CheckEvent(KEYBOARD_I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED)==0)
	{
	if ((I2CTimeout--) == 0) return I2C_TIMEOUT_UserCallback(3);
	}
	{
	/*ͨ��I2C�����豸�ж�ȡһ���ֽڵ����� */
	*pBuffer = I2C_ReceiveData(KEYBOARD_I2Cx);

	/* �洢���ݵ�ָ��ָ����һ����ַ */
	pBuffer++;

	/* ���������Լ� */
	NumByteToRead--;
	}
	}

	/* ʹ��Ӧ�𣬷�����һ��I2C ���� */
	I2C_AcknowledgeConfig(KEYBOARD_I2Cx, ENABLE);
	return 1;
}

/**
* @brief I2C ����(KEYBOARD)��ʼ��
* @param ��
* @retval ��
*/
void I2C_KEYBOARD_Init(void)
{
	uint8_t addr=0x00;
	I2C_GPIO_Config();
	I2C_Mode_Config();
	for(int i=0;i<4;i++)
	{I2C_KEYBOARD_ByteWrite(SetCode+i,addr+i);}//����WT5700����ģʽ�����㴥���������ȣ�20CLOCK
}

