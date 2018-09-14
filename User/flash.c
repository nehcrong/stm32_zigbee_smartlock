#include "stm32f10x.h"
#include "flash.h"
#include "delay.h"

int InternalFlash_Test(void)
{
uint32_t EraseCounter = 0x00; //记录要擦除多少页
uint32_t Address = 0x00; //记录写入的地址
	uint16_t Data=0xabcd;
 //uint32_t Data = 0x3210ABCD; //记录写入的数据
 uint32_t NbrOfPage = 0x00; //记录写入多少页

 FLASH_Status FLASHStatus = FLASH_COMPLETE; //记录每次擦除的结果
 int MemoryProgramStatus = 0;//记录整个测试结果


 /* 解锁 */
 FLASH_Unlock();

 /* 计算要擦除多少页 */
 NbrOfPage = (DATA_END_ADDR - DATA_START_ADDR) / FLASH_PAGE_SIZE;

 /* 清空所有标志位 */
 FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR |
FLASH_FLAG_WRPRTERR);

 /* 按页擦除*/
 for (EraseCounter = 0; (EraseCounter < NbrOfPage) &&
(FLASHStatus == FLASH_COMPLETE); EraseCounter++) {
 FLASHStatus = FLASH_ErasePage(DATA_START_ADDR +
(FLASH_PAGE_SIZE * EraseCounter));

 }

 /* 向内部FLASH 写入数据 */
 Address = DATA_START_ADDR;

 while ((Address < DATA_END_ADDR) && (FLASHStatus == FLASH_COMPLETE)) {
 FLASHStatus = FLASH_ProgramHalfWord(Address, Data);
 Address = Address + 2;
 }

 FLASH_Lock();
 /* 检查写入的数据是否正确 */
 Address = DATA_START_ADDR;

 while ((Address < DATA_END_ADDR) && (MemoryProgramStatus != 1)) {
 if ((*(__IO uint32_t*) Address) != Data) {
 MemoryProgramStatus = 1;
 }
 Address += 2;
 }
 return MemoryProgramStatus;
 }




int a[6];
/**
* @brief InternalFlash_Test,对内部FLASH 进行读写测试
* @param None
* @retval None
*/
int Flash_Write(uint8_t *pdata,int len)
{
	int Numb=0;
	uint16_t temp = 0;
	uint32_t EraseCounter = 0x00; //擦除页数
	uint32_t Address = DATA_START_ADDR; //写入的地址
	uint32_t NbrOfPage = 0x00; //写入页数
	FLASH_Status FLASHStatus = FLASH_COMPLETE; //记录每次擦除的结果
	int MemoryProgramStatus = 0;//数据写入是否正确：0正确，1错误

	/* 解锁 */
	FLASH_Unlock();

	/* 计算要擦除多少页 */
	NbrOfPage = (DATA_END_ADDR - DATA_START_ADDR) / FLASH_PAGE_SIZE;

	/* 清空所有标志位 */
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR |
	FLASH_FLAG_WRPRTERR);

	/* 按页擦除*/
	for (EraseCounter = 0; (EraseCounter < NbrOfPage)&&(FLASHStatus == FLASH_COMPLETE); EraseCounter++) 
	{FLASHStatus = FLASH_ErasePage(DATA_START_ADDR+(FLASH_PAGE_SIZE * EraseCounter));}

	/* 向内部FLASH 写入数据 */
	while ((Address < DATA_END_ADDR) && (FLASHStatus == FLASH_COMPLETE) && (Numb<len)) 
	{
		temp = (uint16_t)pdata[Numb];
		FLASHStatus = FLASH_ProgramHalfWord(Address, temp);
		Address+=2;
		Numb++;
	}

	FLASH_Lock();

	/* 检查写入的数据是否正确 */
	Address = DATA_START_ADDR;
	Numb=0;
//	while ((Address < DATA_END_ADDR) && (MemoryProgramStatus != 1) && Numb<len)
//	{
//		if ((*(__IO uint32_t*) Address+=2) != pdata[Numb++]) 
//		{
//			MemoryProgramStatus = 1;
//		}
//	}
	return MemoryProgramStatus;
}

void Flash_Read(uint8_t *pdata,int len)
{
	int Numb=0;
	uint16_t temp;
	uint32_t Address = DATA_START_ADDR; //读取的地址
	while ((Address < DATA_END_ADDR) &&  Numb<=len)
	{
		temp=*(__IO uint16_t*) Address;
		pdata[Numb]=temp;
		Numb++;
		Address+=2;
	}
}

void WriteProtect_Release(void)
{
	if (FLASH_GetWriteProtectionOptionByte() != 0xFFFFFFFF ) 
	{
		//解除对FLASH_CR 寄存器的访问限制
		FLASH_Unlock();
			
		/* 擦除所有选项字节的内容 */
		FLASH_EraseOptionBytes();

		/* 对所有页解除 */
		FLASH_EnableWriteProtection(0xFFFFFFFF);
		
		/* 复位芯片，以使选项字节生效 */
		NVIC_SystemReset();
	}
}
