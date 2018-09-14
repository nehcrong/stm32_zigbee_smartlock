#include "stm32f10x.h"
#include "flash.h"
#include "delay.h"

int InternalFlash_Test(void)
{
uint32_t EraseCounter = 0x00; //��¼Ҫ��������ҳ
uint32_t Address = 0x00; //��¼д��ĵ�ַ
	uint16_t Data=0xabcd;
 //uint32_t Data = 0x3210ABCD; //��¼д�������
 uint32_t NbrOfPage = 0x00; //��¼д�����ҳ

 FLASH_Status FLASHStatus = FLASH_COMPLETE; //��¼ÿ�β����Ľ��
 int MemoryProgramStatus = 0;//��¼�������Խ��


 /* ���� */
 FLASH_Unlock();

 /* ����Ҫ��������ҳ */
 NbrOfPage = (DATA_END_ADDR - DATA_START_ADDR) / FLASH_PAGE_SIZE;

 /* ������б�־λ */
 FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR |
FLASH_FLAG_WRPRTERR);

 /* ��ҳ����*/
 for (EraseCounter = 0; (EraseCounter < NbrOfPage) &&
(FLASHStatus == FLASH_COMPLETE); EraseCounter++) {
 FLASHStatus = FLASH_ErasePage(DATA_START_ADDR +
(FLASH_PAGE_SIZE * EraseCounter));

 }

 /* ���ڲ�FLASH д������ */
 Address = DATA_START_ADDR;

 while ((Address < DATA_END_ADDR) && (FLASHStatus == FLASH_COMPLETE)) {
 FLASHStatus = FLASH_ProgramHalfWord(Address, Data);
 Address = Address + 2;
 }

 FLASH_Lock();
 /* ���д��������Ƿ���ȷ */
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
* @brief InternalFlash_Test,���ڲ�FLASH ���ж�д����
* @param None
* @retval None
*/
int Flash_Write(uint8_t *pdata,int len)
{
	int Numb=0;
	uint16_t temp = 0;
	uint32_t EraseCounter = 0x00; //����ҳ��
	uint32_t Address = DATA_START_ADDR; //д��ĵ�ַ
	uint32_t NbrOfPage = 0x00; //д��ҳ��
	FLASH_Status FLASHStatus = FLASH_COMPLETE; //��¼ÿ�β����Ľ��
	int MemoryProgramStatus = 0;//����д���Ƿ���ȷ��0��ȷ��1����

	/* ���� */
	FLASH_Unlock();

	/* ����Ҫ��������ҳ */
	NbrOfPage = (DATA_END_ADDR - DATA_START_ADDR) / FLASH_PAGE_SIZE;

	/* ������б�־λ */
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR |
	FLASH_FLAG_WRPRTERR);

	/* ��ҳ����*/
	for (EraseCounter = 0; (EraseCounter < NbrOfPage)&&(FLASHStatus == FLASH_COMPLETE); EraseCounter++) 
	{FLASHStatus = FLASH_ErasePage(DATA_START_ADDR+(FLASH_PAGE_SIZE * EraseCounter));}

	/* ���ڲ�FLASH д������ */
	while ((Address < DATA_END_ADDR) && (FLASHStatus == FLASH_COMPLETE) && (Numb<len)) 
	{
		temp = (uint16_t)pdata[Numb];
		FLASHStatus = FLASH_ProgramHalfWord(Address, temp);
		Address+=2;
		Numb++;
	}

	FLASH_Lock();

	/* ���д��������Ƿ���ȷ */
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
	uint32_t Address = DATA_START_ADDR; //��ȡ�ĵ�ַ
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
		//�����FLASH_CR �Ĵ����ķ�������
		FLASH_Unlock();
			
		/* ��������ѡ���ֽڵ����� */
		FLASH_EraseOptionBytes();

		/* ������ҳ��� */
		FLASH_EnableWriteProtection(0xFFFFFFFF);
		
		/* ��λоƬ����ʹѡ���ֽ���Ч */
		NVIC_SystemReset();
	}
}
