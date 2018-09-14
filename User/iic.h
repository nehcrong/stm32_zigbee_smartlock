#ifndef __IIC_H
#define __IIC_H
#include "stm32f10x.h"
#include "stm32f10x_i2c.h"

/**************************I2C �������壬I2C1 ��I2C2*********************/
#define KEYBOARD_I2Cx I2C1
#define KEYBOARD_I2C_APBxClock_FUN RCC_APB1PeriphClockCmd
#define KEYBOARD_I2C_CLK RCC_APB1Periph_I2C1
#define KEYBOARD_I2C_GPIO_APBxClock_FUN RCC_APB2PeriphClockCmd
#define KEYBOARD_I2C_GPIO_CLK RCC_APB2Periph_GPIOB
#define KEYBOARD_I2C_SCL_PORT GPIOB
#define KEYBOARD_I2C_SCL_PIN GPIO_Pin_6
#define KEYBOARD_I2C_SDA_PORT GPIOB
#define KEYBOARD_I2C_SDA_PIN GPIO_Pin_7

/* STM32 I2C ����ģʽ */
#define I2C_Speed 400000 //*

/* �����ַֻҪ��STM32 ��ҵ�I2C ������ַ��һ������ */
#define I2Cx_OWN_ADDRESS7 0X0A

/* AT24C01/02 ÿҳ��8 ���ֽ� */
#define I2C_PageSize 8

extern uint8_t IIC_ReadBuf[];
void I2C_KEYBOARD_Init(void);
uint8_t I2C_BufferRead(uint8_t* pBuffer, uint8_t ReadAddr,u16 NumByteToRead);

#endif
