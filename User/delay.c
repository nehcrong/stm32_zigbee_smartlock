#include "delay.h"

void SysTick_Delay_Us( __IO uint32_t us)//us��ʱ
{
	uint32_t i;
	SysTick_Config(SystemCoreClock/1000000);

	for (i=0; i<us; i++) {
	// ����������ֵ��С��0 ��ʱ��CRTL �Ĵ�����λ16 ����1
	while ( !((SysTick->CTRL)&(1<<16)) );
	}
	// �ر�SysTick ��ʱ��
	SysTick->CTRL &=~SysTick_CTRL_ENABLE_Msk;
}


void SysTick_Delay_Ms( __IO uint32_t ms)//ms��ʱ
{
	uint32_t i;
	SysTick_Config(SystemCoreClock/1000);

	for (i=0; i<ms; i++) {
	// ����������ֵ��С��0 ��ʱ��CRTL �Ĵ�����λ16 ����1
	// ����1 ʱ����ȡ��λ����0
	while ( !((SysTick->CTRL)&(1<<16)) );
	}
	// �ر�SysTick ��ʱ��
	SysTick->CTRL &=~ SysTick_CTRL_ENABLE_Msk;
}
