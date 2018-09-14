#include "delay.h"

void SysTick_Delay_Us( __IO uint32_t us)//us延时
{
	uint32_t i;
	SysTick_Config(SystemCoreClock/1000000);

	for (i=0; i<us; i++) {
	// 当计数器的值减小到0 的时候，CRTL 寄存器的位16 会置1
	while ( !((SysTick->CTRL)&(1<<16)) );
	}
	// 关闭SysTick 定时器
	SysTick->CTRL &=~SysTick_CTRL_ENABLE_Msk;
}


void SysTick_Delay_Ms( __IO uint32_t ms)//ms延时
{
	uint32_t i;
	SysTick_Config(SystemCoreClock/1000);

	for (i=0; i<ms; i++) {
	// 当计数器的值减小到0 的时候，CRTL 寄存器的位16 会置1
	// 当置1 时，读取该位会清0
	while ( !((SysTick->CTRL)&(1<<16)) );
	}
	// 关闭SysTick 定时器
	SysTick->CTRL &=~ SysTick_CTRL_ENABLE_Msk;
}
