#ifndef __FLASH_H
#define __FLASH_H
#include "stm32f10x_flash.h"

#if defined (STM32F10X_HD) || defined (STM32F10X_HD_VL) || defined (STM32F10X_CL) || defined (STM32F10X_XL)
#define FLASH_PAGE_SIZE ((uint16_t)0x800)//2048
#else
#define FLASH_PAGE_SIZE ((uint16_t)0x400)//1024
#endif

#define DATA_START_ADDR ((uint32_t)0x08008000)
#define DATA_END_ADDR ((uint32_t)0x08008400)

int Flash_Write(uint8_t *pdata,int len);
void Flash_Read(uint8_t *pdata,int len);
void WriteProtect_Release(void);
int InternalFlash_Test(void);

#endif
