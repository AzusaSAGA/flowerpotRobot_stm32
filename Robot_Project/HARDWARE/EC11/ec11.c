#include "ec11.h"

u8 EC11_Num1 = 50;

//EC11_KEY1按下后执行事件
void EC11_KEY1_click_do(){

}
 
//定时器1配置
void TIM1_Int_Init(u16 arr, u16 psc)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	RCC_APB1PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);//TIM2时钟使能    
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler = psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE); //使能指定的TIM7中断,允许更新中断
 
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
 
	TIM_Cmd(TIM1, ENABLE);//开启定时器4
}
 
//定时器1中断服务程序		    
void TIM1_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)//是更新中断
	{
		EC11_Hander1(Encoder_EC11_Scan1());
 
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);  //清除TIM4更新中断标志    
	}
}
 
//*******************************************************************/
//功能：初始化EC11旋转编码器相关参数
//形参：EC11旋转编码器的类型-->>  unsigned char Set_EC11_TYPE  <<--  ：0----一定位对应一脉冲；1（或非0）----两定位对应一脉冲。
//返回：无
//详解：对EC11旋转编码器的连接IO口做IO口模式设置。以及将相关的变量进行初始化
//*******************************************************************/
void EC11_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	//EC11AB相编码器初始化
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9|GPIO_Pin_11;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	//EC11按键初始化
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
 
	GPIO_SetBits(GPIOE, GPIO_Pin_9|GPIO_Pin_11);
 
	//避免上电时EC11旋钮位置不确定导致一次动作误判
	EC11_A1_Last = EC11_A1;
	EC11_B1_Last = EC11_B1;
	
	TIM1_Int_Init(9,8399);	//初始化定时器1 1ms中断
}
//*******************************************************************/
//功能：根据对应形参EC11_Value实现对应功能
//形参：EC11_Value
//返回：无
//详解：旋转编码器的功能封装函数，可以跟据自己的想法填入对应实现的功能
//*******************************************************************/
void EC11_Hander1(u8 EC11_Value)
{
	if (EC11_Value == 1) //正转
	{
		//--------编码器正转动作代码--------//
		EC11_Num1++;
	}
	else if (EC11_Value == 2)    //反转
	{
		//--------编码器反转动作代码--------//
		EC11_Num1--;
	}
	else if (EC11_Value == 3)    //按压一次
	{
		//--------编码器按键按压一次代码--------//
		EC11_KEY1_click_do();
	}	
}
 
//*******************************************************************/
//功能：扫描EC11旋转编码器的动作并将参数返回给动作分析函数使用
//形参：EC11旋转编码器的类型-->>  unsigned char Set_EC11_TYPE  <<--  ：0----一定位对应一脉冲；1（或非0）----两定位对应一脉冲
//返回：EC11旋转编码器的扫描结果-->>  char ScanResult  -->>  0：无动作；1：正转； -1：反转；2：只按下按键；3：按着按键正转；-3：按着按键反转
//详解：只扫描EC11旋转编码器有没有动作，不关心是第几次按下按键或长按或双击。返回值直接作为形参传给 [ void Encoder_EC11_Analyze(char EC11_Value); ] 函数使用
//*******************************************************************/
char Encoder_EC11_Scan1(void)
{
	//以下储存A、B上一次值的变量声明为静态全局变量，方便对EC11对应的IO口做初始化
	//  static char EC11_A_Last = 0;
	//  static char EC11_B_Last = 0;
	char ScanResult = 0;    //返回编码器扫描结果，用于分析编码器的动作
	
	//返回值的取值：   0：无动作；      1：正转；   2：反转；  
	//                 3：只按下按键；  4：双击；   5：长按
	//======================================================//
              
	if (EC11_A1 != EC11_A1_Last)   //以A为时钟，B为数据。正转时AB反相，反转时AB同相
	{
		if (EC11_A1 == 0)
		{
			if (EC11_B1 == 1)      //只需要采集A的上升沿或下降沿的任意一个状态，若A下降沿时B为1，正转                    
				 ScanResult = 1;     //正转
			else                    //反转
				 ScanResult = 2;
			}
			EC11_A1_Last = EC11_A1;   //更新编码器上一个状态暂存变量
			EC11_B1_Last = EC11_B1;   //更新编码器上一个状态暂存变量
	}
	
	//EC11按键处理
	if(0==EC11_KEY1){
		delay_ms(10);
		if(0==EC11_KEY1 && 1==EC11_KEY1_Last){
		ScanResult = 3;
		EC11_KEY1_Last = 0;
		}
	}
	return ScanResult;
}

