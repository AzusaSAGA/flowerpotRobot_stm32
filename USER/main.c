#include "main.h"

u16 MAX_Motor_Speed=0;
int Motor_Speed_Left=0; //左
int Motor_Speed_Right=0; //右
u8 Command=0;  //机械臂和机械爪/是否记录GPS指令

uint32_t remote_timeout_cnt = 0;
extern int TIM3_sumcode;
extern int TIM4_sumcode;
extern int Encoder_Timer3_sum;
extern int Encoder_Timer4_sum;
extern EncoderStruct Encoders;

// 如果完全由手柄控制机器人，按键1表示记录GPS，按键3表示抓取，按键4表示放置花盆
#define RECORD_GPS       0x01  // 这个手柄按键1被按下，表示要记录GPS

#define COMMAND_GRASP    0x03  // 抓取 (低4位)0000 0011
#define COMMAND_RELEASE  0x04  // 释放 (低4位)0000 0100
#define COMMAND_COMMON   0x05  // 闲置 (低4位)0000 0101


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
    uart4_init(115200);
	#if SEND_DMA
	DMA1_Stream3_Config();
	#endif
	TIM3_Encoder_Init(65535,1-1);//编码器1函数初始化
	TIM4_Encoder_Init(65535,1-1);//编码器2函数初始化
	POWER_Init();
	TIM6_init(100-1,8400-1); //0.01s定时中断
	TIM7_init(100-1,8400-1); //0.01s定时中断
	
   Home_All_Simultaneously();//复位
   Action_2_Claw_Open(600, 0, 14.0f);
   
   Stepper_Action_PickUp();   //抓取 (Pick)
  
/////* --- 步进电机  --- */
    
//    Stepper_Action_PutDown();  //下放 (Place)

while(1){
		//遥控手柄控制
		if(NRF24L01_RxPacket(tmp_buf)==0){
			MAX_Motor_Speed = 4*tmp_buf[1];
			Motor_Speed_Left = 4*(tmp_buf[2]+tmp_buf[3]-256);
            Motor_Speed_Right = 4*(tmp_buf[2]-tmp_buf[3]);
            Command = tmp_buf[6];
            if(Command == RECORD_GPS) {
                tx_frame.status |= 0x04;
            }
		}
        //来自jetson orin nano的指令
		else{
            if(1 == USART3_RX_completed){
				Motor_Speed_Left = 8*(int8_t)USART3_RX_BUF[1];
				Motor_Speed_Right = 8*(int8_t)USART3_RX_BUF[2];
				Command = USART3_RX_BUF[3];
				OLED_ShowNum(0,80,Motor_Speed_Left,4,8,1);
				OLED_ShowNum(0,90,Motor_Speed_Right,4,8,1);
				OLED_ShowNum(0,100,Command,1,8,1);
				delay_ms(10);
				OLED_Refresh();

				USART3_RX_completed = 0;
			}
        }
        Motor_Move(Motor_Speed_Left, Motor_Speed_Right);
        switch(Command){
            case COMMAND_GRASP:
                Stepper_Action_PickUp();
                tx_frame.status |= 0x02;
                break;
            case COMMAND_RELEASE:
                Stepper_Action_PutDown();
                tx_frame.status |= 0x01;
                break;
            default:
                break;
		}
        printf("data: %d %d\n", Encoders.speedA,Encoders.speedB);
    }
//    return 0;
}

