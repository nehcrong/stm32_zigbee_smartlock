#include "timer.h"

unsigned int InputVaild=10000;

void Timer_Start(void)
{
	TIM_Cmd(COM_TIM, ENABLE);
}

void COM_TIM_Config(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	// 开启定时器时钟,即内部时钟CK_INT=72M
	COM_TIM_APBxClock_FUN(COM_TIM_CLK, ENABLE);

	// 自动重装载寄存器周的值(计数值)
	TIM_TimeBaseStructure.TIM_Period=COM_TIM_PREIOD;

	// 累计TIM_Period 个频率后产生一个更新或者中断
	// 时钟预分频数为71，则驱动计数器的时钟CK_CNT = CK_INT / (71+1)=1M
	TIM_TimeBaseStructure.TIM_Prescaler= 71;
	// 时钟分频因子
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;

	// 计数器计数模式
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;

	// 重复计数器的值
	TIM_TimeBaseStructure.TIM_RepetitionCounter=0;

	// 初始化定时器
	TIM_TimeBaseInit(COM_TIM, &TIM_TimeBaseStructure);

	// 清除计数器中断标志位
	TIM_ClearFlag(COM_TIM, TIM_FLAG_Update);

	// 开启计数器中断
	TIM_ITConfig(COM_TIM,TIM_IT_Update,ENABLE);

	// 使能计数器
	TIM_Cmd(COM_TIM, ENABLE);
}

// 中断优先级配置
void COM_TIM_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	// 设置中断组为0
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	// 设置中断来源
	NVIC_InitStructure.NVIC_IRQChannel = COM_TIM_IRQ;
	// 设置主优先级为 0
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	// 设置抢占优先级为3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void COM_TIM_IRQHandler (void)
{
	if ( TIM_GetITStatus( COM_TIM, TIM_IT_Update) != RESET )
	{
		InputVaild++;
		if(InputVaild>=MAX_INTERVAL)
		{TIM_Cmd(COM_TIM, DISABLE);}
		TIM_ClearITPendingBit(COM_TIM , TIM_FLAG_Update);
	}
}

void Timer_Init(void)
{
	COM_TIM_Config();
	COM_TIM_NVIC_Config();
}
