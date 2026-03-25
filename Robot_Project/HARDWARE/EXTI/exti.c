#include "exti.h"
#include "delay.h" 
#include "key.h"

//外部中断初始化程序
//初始化PE2~4,PA0为中断输入.
void EXTIX_Init(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
	
	KEY_Init(); //按键对应的IO口初始化
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//使能SYSCFG时钟
	
 
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource8);//PE2 连接到中断线2
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource12);//PE3 连接到中断线3
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource13);//PE4 连接到中断线4
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource14);//PA0 连接到中断线0
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource15);
	
  /* 配置EXTI_Line8 */
  EXTI_InitStructure.EXTI_Line = EXTI_Line8|EXTI_Line12|EXTI_Line13|EXTI_Line14|EXTI_Line15;//LINE8
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //上升沿触发
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能LINE0
  EXTI_Init(&EXTI_InitStructure);//配置
 
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn|EXTI15_10_IRQn;//外部中断0
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//抢占优先级0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//子优先级2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
  NVIC_Init(&NVIC_InitStructure);//配置
}

//外部中断9_5服务程序
void EXTI9_5_IRQHandler(void)
{
	delay_ms(10);	//消抖
	if (EXTI_GetITStatus(EXTI_Line8) != RESET){
		if(0==KEY1){
			key_value=1;
		}
	 EXTI_ClearITPendingBit(EXTI_Line8); //清除LINE0上的中断标志位 
	}
}	

//外部中断15_10服务程序
void EXTI15_10_IRQHandler(void)
{
	delay_ms(10);	//消抖
	if (EXTI_GetITStatus(EXTI_Line12) != RESET){
		if(0==KEY2){
			key_value=2;
		}	 
	 EXTI_ClearITPendingBit(EXTI_Line12);//清除LINE2上的中断标志位 
	}else if(EXTI_GetITStatus(EXTI_Line13) != RESET){
		if(0==KEY3){
			key_value=3;
		}	 
		EXTI_ClearITPendingBit(EXTI_Line13);//清除LINE2上的中断标志位 
	}
    EXTI_ClearITPendingBit(EXTI_Line15);//清除LINE2上的中断标志位 
}














