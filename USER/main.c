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
int grasp_flag = 1;    // 抓取标志，为1才抓取，为0才摆放
extern volatile uint8_t res;
extern volatile u8 USART3_RX_completed;

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
    delay_ms(200);
    Action_2_Claw_Open(600, 0, 10.0f);
    delay_ms(200);
     //Stepper_Action_PickUp();//抬起
   // Stepper_Action_PutDown();//放下
   
    //Action_4_Arm_Lower(100, 0, 60.0f); 

while(1){  
  // UART4_SendString((u8*)"UART4 is Working!\r\n");
		//遥控手柄控制
char msg1[64];  // 足够容纳格式化后的字符串
sprintf(msg1, "send_data: %f\r\n", Encoders.speedA);
UART4_SendString((u8*)msg1);
    
		if(NRF24L01_RxPacket(tmp_buf)==0){
			MAX_Motor_Speed = 4*tmp_buf[1];
			Motor_Speed_Left = 4*(tmp_buf[2]+tmp_buf[3]-256);
            Motor_Speed_Right = 4*(tmp_buf[2]-tmp_buf[3]);
            Command = tmp_buf[6];
            if(Command == RECORD_GPS) {
                tx_frame.status |= 0x04;
                //printf("status_data: %d \n", tx_frame.status);
            }
            
		}
        //来自jetson orin nano的指令
		else{
            if(USART3_RX_completed ==1){
            // 提取数据
//                uint8_t header = USART3_RX_BUF[0];      // 应该是0x55
//                uint8_t left_pwm = USART3_RX_BUF[1];    // 左PWM
//                uint8_t right_pwm = USART3_RX_BUF[2];   // 右PWM
//                uint8_t cmd = USART3_RX_BUF[3];         // 指令
//                uint8_t checksum = USART3_RX_BUF[4];    // 校验和
//            
//            // 通过UART4显示收到的数据
//                char msg[64];
//                sprintf(msg, "Received: %02X %02X %02X %02X %02X\r\n",
//                    header, left_pwm, right_pwm, cmd, checksum);
//                UART4_SendString((u8*)msg);
                Motor_Speed_Left = 8*(int8_t)USART3_RX_BUF[1];
                Motor_Speed_Right = 8*(int8_t)USART3_RX_BUF[2];
                Command = USART3_RX_BUF[3];
                USART3_RX_completed = 0;
            }
         }
            Motor_Move(Motor_Speed_Left, Motor_Speed_Right);
            switch(Command){
                case COMMAND_GRASP:
                    if(1 == grasp_flag){
                    Stepper_Action_PickUp();
                    grasp_flag=0;
                    tx_frame.status |= 0x02;
                }
                 break;
                case COMMAND_RELEASE:
                    if(0 == grasp_flag){
                    Stepper_Action_PutDown();
                    grasp_flag=1;
                    tx_frame.status |= 0x01;
                }
                 break;
                default:
                 break;
		}
    }

}

