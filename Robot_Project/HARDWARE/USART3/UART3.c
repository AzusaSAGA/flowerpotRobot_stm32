// 用于接收来自jetson orin nano的数据,同时发送编码器的数据
#include "UART3.h"

volatile u8 USART3_RX_BUF[USART3_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
volatile u8 USART3_RX_INDEX = 0;
volatile u8 USART3_RX_completed=0;       //接收完成状态标记

extern int Encoder_Timer3_sum;
extern int Encoder_Timer4_sum;

USART3_Frame tx_frame;  // 编码器数据

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
#if SEND_DMA
void DMA1_Stream3_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	// 使能DMA1时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

	// 配置DMA1 Stream3 (USART3_TX)
	DMA_DeInit(DMA1_Stream3);
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART3->DR;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&tx_frame;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = sizeof(tx_frame);
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream3, &DMA_InitStructure);

	// 使能USART3 DMA TX请求
	USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
}
#endif

void Prepare_Frame(float value1, float value2)
{
	uint8_t *p = (uint8_t*)&tx_frame;
	uint8_t sum = 0;

	// 设置帧头
	tx_frame.header = 0x55;

	// 设置两个32位数据
	tx_frame.data1 = value1;
	tx_frame.data2 = value2;

	// 计算校验和（前9字节的和）
	for(int i = 0; i < 9; i++) {
	sum += p[i];
	}
	tx_frame.checksum = sum;
}
#if SEND_DMA
void USART3_Send_DMA(void)
{
	// 等待DMA空闲
	while(DMA_GetCmdStatus(DMA1_Stream3) != DISABLE);

	// 重新设置传输长度
	DMA_SetCurrDataCounter(DMA1_Stream3, sizeof(tx_frame));

	// 使能DMA流
	DMA_Cmd(DMA1_Stream3, ENABLE);
}
#endif

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
	if(USART_GetITStatus(USART3,USART_IT_RXNE)!=RESET){
        USART3_RX_BUF[USART3_RX_INDEX++]=USART_ReceiveData(USART3);		
		//检查是否收到完整帧
		if(USART3_RX_INDEX >=3){
			USART3_RX_INDEX = 0;
			//可以再加个校验
			USART3_RX_completed = 1;
		}
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	}
	USART_ClearITPendingBit(USART3,USART_IT_ORE);
}
