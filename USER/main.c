#include "main.h"

u16 MAX_Motor_Speed=0;
int Motor_Speed_Left=0; //左
int Motor_Speed_Right=0; //右
u8 Command=0;  //机械臂和机械爪指令

extern int TIM3_sumcode;
extern int TIM4_sumcode;
extern int Encoder_Timer3_sum;
extern int Encoder_Timer4_sum;

float alpha = 0.2;  //一阶低通滤波参数，用于“慢启动”
//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
int main(void)																																																																																																																																																																																																
{
	//初始化
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init(168);		   //初始化延时函数
	LED_Init();				    //初始化LED端口
	BEEP_Init();
	OLED_Init();
	EC11_Init();
	Stepper_Init();
    Limit_Init(); 
	Motor_Init();
	NRF24L01_Init();
	//USART1_Init(9600);
	uart3_init(115200);
	#if SEND_DMA
	DMA1_Stream3_Config();
	#endif
	TIM3_Encoder_Init(65535,1-1);//编码器1函数初始化
	TIM4_Encoder_Init(65535,1-1);//编码器2函数初始化
	POWER_Init();
	TIM6_init(100-1,8400-1); //0.01s定时中断
	TIM7_init(100-1,8400-1); //0.01s定时中断
	
   Home_All_Simultaneously();//复位
//	
//	while(1)
//	{
//			dir3=1;
//			dir4=0;
//		for(int i=0; i < 1000; i++)             	// 输出3200个脉冲信号
//		{
//			step3=1;
//			step4=1;
//			delay_us(1);
//			step3=0;
//			step4=0;
//			delay_us(1);
//		}
//	}
//	

// 等待电机主板开机完毕！！！
    //delay_ms(500);    

    // 测试1：让机械爪(USART1)松开10圈，速度1000
   // Emm_V5_Pos_Control(MOTOR_ID_CLAW, DIR_CCW, 1000, 0, 32000);

// Action_1_Claw_Grab();


//OLED_ShowNum(0,0,10,4,8,1);
//				delay_ms(10);
//				OLED_Refresh();
/* --- 编码电机 --- */
 
//Motor_Move(int PWM_left,int PWM_right)


/////* --- 步进电机  --- */
//Action_2_Claw_Open(300, 0, 14.0f);

//delay_ms(3000); 
//    /* * 动作 1：【机械爪抓取】
//     * 参数：600转/分，0加速，最多转动 15 圈
//     * 逻辑：碰到目标物体（限位器触发）瞬间急停，死死抱住！
//     */
//    Action_1_Claw_Grab(300, 0, 20.0f);
//    delay_ms(3000); 
//    
//    // 抓稳后，等待 1 秒，确保物体不晃动
//     Action_4_Arm_Lower(300, 0, 85.0f);
//     delay_ms(3000); 
//   

    /* * 动作 2：【机械臂抬起】
     * 参数：100转/分 (重载慢速)，0加速，最大行程 90 度
     * 逻辑：将物体抬升，直到撞到最高点限位器 (PB8) 完美刹停
     */
   // Action_3_Arm_Lift(300, 0, 90.0f);
    
    // 抬起后，模拟底盘正在移动到目标放置区，等待 3 秒
    
   



	
	while(1){
		//遥控手柄控制
		if(NRF24L01_RxPacket(tmp_buf)==0){
			MAX_Motor_Speed = 4*tmp_buf[1];
			Motor_Speed_Left=4*(tmp_buf[2]+tmp_buf[3]-256);
			Motor_Speed_Right=4*(tmp_buf[2]-tmp_buf[3]);
			Command=tmp_buf[6];
		}
		//来自jetson orin nano的指令
		else{
			if(1 == USART3_RX_completed){
				Motor_Speed_Left = Motor_Speed_Left*(1-alpha)
										+ 8*(int8_t)USART3_RX_BUF[0]*alpha;
				Motor_Speed_Right = Motor_Speed_Right*(1-alpha)
										+ 8*(int8_t)USART3_RX_BUF[1]*alpha;
				Command = USART3_RX_BUF[2];
				OLED_ShowNum(0,80,Motor_Speed_Left,4,8,1);
				OLED_ShowNum(0,90,Motor_Speed_Right,4,8,1);
				OLED_ShowNum(0,100,Command,1,8,1);
				delay_ms(10);
				OLED_Refresh();

				USART3_RX_completed = 0;
			}
		}
		Motor_Move(Motor_Speed_Left,Motor_Speed_Right);
//		switch(Command){
//				case 1:
//					Stepper_Claw_Move(55000,1,5);//0为闭合，1为张开
//					break;
//				case 2:
//					Stepper_Arm_Move(350000,0,5); //0为下去  脉冲350000
//					break;
//				case 3:
//					Stepper_Claw_Move(55000,0,5);//0为闭合，1为张开
//					break;
//				case 4:
//					Stepper_Arm_Move(350000,1,5); //0为下去  脉冲350000
//					break;
//				default:
//					break;
//		}
	}
	return 0;
}		
