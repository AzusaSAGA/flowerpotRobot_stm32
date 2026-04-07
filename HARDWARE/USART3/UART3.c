// 用于接收来自jetson orin nano的数据,同时发送编码器的数据
#include "UART3.h"
#include <stdio.h>

volatile u8 USART3_RX_BUF[USART3_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
volatile u8 USART3_RX_INDEX = 0;
volatile u8 USART3_RX_completed=0;       //接收完成状态标记
// 错误标志变量
volatile uint8_t USART3_RX_ERROR = 0;   // 校验错误标志
volatile uint8_t USART3_RX_ORE = 0;     // 溢出错误标志


volatile uint8_t res = 0;
volatile uint8_t calc_sum = 0;

/////////////////////
USART3_Frame tx_frame;  // stm32给jetson发送的数据包
/////////////////////

/******************************************************************
 * 函数: uart3_init
 * 功能: 初始化串口3（配置中断）
 ******************************************************************/
void uart3_init(unsigned long baudrate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE); //使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);//使能USART1时钟
 
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_USART3); //GPIOA9复用为USART1
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_USART3); //GPIOA10复用为USART1
	
	//USART1端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOD,&GPIO_InitStructure); //初始化PA9，PA10

   //USART1 初始化设置
	USART_InitStructure.USART_BaudRate = baudrate;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART3, &USART_InitStructure); //初始化串口1
	
	USART_Cmd(USART3, ENABLE);  //使能串口1 
	
	//USART_ClearFlag(USART1, USART_FLAG_TC);
	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启相关中断

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、
}
/////////////////////////////////////
// 组装 STM32 -> Jetson 的 5 字节数据包
void Prepare_Frame(uint32_t value1, uint32_t value2)
{
	uint8_t *p = (uint8_t*)&tx_frame;
	uint8_t sum = 0;

	tx_frame.header = 0x55; // 帧头
	tx_frame.data1 = value1;
	tx_frame.data2 = value2;

	// 计算校验和（前 10 字节之和）
	for(int i = 0; i < 10; i++) {
	    sum += p[i];
	}
	tx_frame.checksum = sum;
}
/*

*/
void USART3_Send_Data(uint8_t *data, uint8_t length)
{
	for(uint8_t i = 0; i < length; i++)
	{
		// 等待发送寄存器空
		while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);

		// 发送数据
		USART_SendData(USART3, data[i]);
	}

	// 等待发送完成
	while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
}


void USART3_IRQHandler(void)
{
    // 处理正常接收中断
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        res = USART_ReceiveData(USART3);
        // ==========================================
        // ★ 核心透传测试：无论收到什么，立刻用 UART4 甩给电脑！
        // ==========================================
        //uart4_send_byte(res);
        
        if (USART3_RX_completed == 0) 
        {
            if (USART3_RX_INDEX == 0) 
            {
                if (res == 0x55)  // 等待正确的帧头
                {
                    USART3_RX_BUF[USART3_RX_INDEX++] = res;
                }
            } 
            else 
            {
                USART3_RX_BUF[USART3_RX_INDEX++] = res;
                
                if(USART3_RX_INDEX >= 5 ) //USART3_REC_LEN
                {
                   calc_sum = (USART3_RX_BUF[0] + USART3_RX_BUF[1] + 
                                       USART3_RX_BUF[2] + USART3_RX_BUF[3]) & 0xFF;
                    
                    if(calc_sum == USART3_RX_BUF[4]) 
                    {
                        USART3_RX_completed = 1;  // ? 只设置标志，不发送
                    }
                    else
                    {
                        USART3_RX_ERROR = 1;  // ? 设置错误标志
                    }
                    
                    USART3_RX_INDEX = 0;
                }
            }
        }
    }
    
}

