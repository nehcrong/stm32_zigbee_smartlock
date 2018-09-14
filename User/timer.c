#include "timer.h"

unsigned int InputVaild=10000;

void Timer_Start(void)
{
	TIM_Cmd(COM_TIM, ENABLE);
}

void COM_TIM_Config(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	// ������ʱ��ʱ��,���ڲ�ʱ��CK_INT=72M
	COM_TIM_APBxClock_FUN(COM_TIM_CLK, ENABLE);

	// �Զ���װ�ؼĴ����ܵ�ֵ(����ֵ)
	TIM_TimeBaseStructure.TIM_Period=COM_TIM_PREIOD;

	// �ۼ�TIM_Period ��Ƶ�ʺ����һ�����»����ж�
	// ʱ��Ԥ��Ƶ��Ϊ71����������������ʱ��CK_CNT = CK_INT / (71+1)=1M
	TIM_TimeBaseStructure.TIM_Prescaler= 71;
	// ʱ�ӷ�Ƶ����
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;

	// ����������ģʽ
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;

	// �ظ���������ֵ
	TIM_TimeBaseStructure.TIM_RepetitionCounter=0;

	// ��ʼ����ʱ��
	TIM_TimeBaseInit(COM_TIM, &TIM_TimeBaseStructure);

	// ����������жϱ�־λ
	TIM_ClearFlag(COM_TIM, TIM_FLAG_Update);

	// �����������ж�
	TIM_ITConfig(COM_TIM,TIM_IT_Update,ENABLE);

	// ʹ�ܼ�����
	TIM_Cmd(COM_TIM, ENABLE);
}

// �ж����ȼ�����
void COM_TIM_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	// �����ж���Ϊ0
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	// �����ж���Դ
	NVIC_InitStructure.NVIC_IRQChannel = COM_TIM_IRQ;
	// ���������ȼ�Ϊ 0
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	// ������ռ���ȼ�Ϊ3
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
