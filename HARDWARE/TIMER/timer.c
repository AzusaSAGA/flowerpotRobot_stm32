#include "timer.h"
#include "encode.h"
#include "UART3.h"

extern int Motor_Speed_Left;
extern int Motor_Speed_Right;
extern int Command;
extern Encode encode;
extern EncoderStruct Encoders;

int Encoder_Timer3_sum=0;//定时器3编码器总数
int Encoder_Timer4_sum=0;//定时器4编码器总数

void TIM6_init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);  ///使能TIM7时钟
	
  TIM_TimeBaseInitStructure.TIM_Period = arr;   //自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseInit(TIM6,&TIM_TimeBaseInitStructure);//初始化TIM7
	
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE); //允许定时器6更新中断
	TIM_Cmd(TIM6,ENABLE); //初始化时先不开启定时器7
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM6_DAC_IRQn; //定时器6中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void TIM6_DAC_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM6,TIM_IT_Update)==SET){
		
	}
	TIM_ClearITPendingBit(TIM6,TIM_IT_Update);
}

void TIM7_init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,ENABLE);  ///使能TIM7时钟
	
    TIM_TimeBaseInitStructure.TIM_Period = arr;   //自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseInit(TIM7,&TIM_TimeBaseInitStructure);//初始化TIM7
	
	TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE); //允许定时器6更新中断
	TIM_Cmd(TIM7,ENABLE); //初始化时先不开启定时器7
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM7_IRQn; //定时器6中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//用来在OLED上显示参数
void TIM7_IRQHandler(void)                          //得到编码器素的和位置
{
	if(TIM_GetITStatus(TIM7,TIM_IT_Update)==SET){
		Solve_EncoData(0.01);    
		Prepare_Frame(Encoders.speedA, Encoders.speedB);
		#if SEND_DMA 
		USART3_Send_DMA();
		#endif
		USART3_Send_Data((uint8_t*)&tx_frame, sizeof(tx_frame));
        tx_frame.status &= 0x00;
		OLED_Show3FNum(0,30,Encoders.speedA,1,3,16,1);
//		OLED_ShowNum(0,10,Encoder_Timer3_sum,4,8,1);
//		OLED_ShowNum(0,20,Encoder_Timer4_sum,4,8,1);
		//OLED_ShowNum(0,20,tmp_buf[1],3,8,1);
//		OLED_ShowNum(0,30,tmp_buf[2],3,8,1);
//		OLED_ShowNum(0,40,tmp_buf[3],3,8,1);
//		OLED_ShowNum(40,40,Motor_Speed_Left,3,8,1);
//		OLED_ShowNum(40,50,Motor_Speed_Right,3,8,1);
//		if(0!=Command){
//		OLED_ShowNum(70,50,Command,1,8,1);
//		}
//		//show_voltage(1);
//		
//		OLED_Refresh();
	}
	TIM_ClearITPendingBit(TIM7,TIM_IT_Update);        //清除中断标志位
}


