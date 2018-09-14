#ifndef __PORT_H
#define __PORT_H


#define LED1 PCout(13)// PC13


extern void Init_LEDpin(void);


#define RLED_ON         GPIO_ResetBits(GPIOC,GPIO_Pin_13)
#define RLED_OFF        GPIO_SetBits(GPIOC,GPIO_Pin_13)
#define BackLight_ON    GPIO_ResetBits(GPIOB,GPIO_Pin_12)
#define BackLight_OFF   GPIO_SetBits(GPIOB,GPIO_Pin_12)
#define BUZZER_ON       GPIO_SetBits(GPIOA,GPIO_Pin_4) 
#define BUZZER_OFF      GPIO_ResetBits(GPIOA,GPIO_Pin_4)
#define IDWORLD_ON      GPIO_SetBits(GPIOA,GPIO_Pin_6) 
#define IDWORLD_OFF     GPIO_ResetBits(GPIOA,GPIO_Pin_6)  
#define CLUTCH_ON		    {GPIO_SetBits(GPIOB,GPIO_Pin_8);GPIO_ResetBits(GPIOB,GPIO_Pin_9);}
#define CLUTCH_OFF      {GPIO_SetBits(GPIOB,GPIO_Pin_9);GPIO_ResetBits(GPIOB,GPIO_Pin_8);}
#define CLUTCH_RESET    {GPIO_ResetBits(GPIOB,GPIO_Pin_8);GPIO_ResetBits(GPIOB,GPIO_Pin_9);}
#define digitalToggle(p,i)  {p->ODR ^=i;}

void PortInit(void);  //所有端口初始化;

#endif

