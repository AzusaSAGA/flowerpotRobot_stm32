#include "UART4.h"  
#include "sys.h"


#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif

//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数，修改为 UART4
int fputc(int ch, FILE *f)
{ 	
	while((UART4->SR&0X40)==0);//循环发送,直到发送完毕   
	UART4->DR = (u8) ch;      
	return ch;
}
#endif
 
#if EN_UART4_RX   //如果使能了接收
	
u8 UART4_RX_BUF[UART4_REC_LEN];     //接收缓冲,最大UART4_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 UART4_RX_STA=0;       //接收状态标记	

//初始化IO 串口4 
//bound:波特率
void uart4_init(u32 bound){
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
	
    // 1. 使能GPIOC和UART4时钟 (注意：UART4挂载在APB1上)
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
 
    // 2. 串口4对应引脚复用映射 PC10, PC11
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_UART4); 
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_UART4); 
	
    // 3. UART4端口配置
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;     //复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //速度50MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;   //推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;     //上拉
    GPIO_Init(GPIOC, &GPIO_InitStructure); 

    // 4. UART4 初始化设置
    USART_InitStructure.USART_BaudRate = bound; //波特率设置
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1; //一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No; //无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
    USART_Init(UART4, &USART_InitStructure); //初始化串口4
	
    USART_Cmd(UART4, ENABLE);  //使能串口4 
	
#if EN_UART4_RX	
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE); //开启相关中断

    // 5. UART4 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn; //串口4中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  //子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);	

#endif
}

// 修改为 UART4 中断服务程序
void UART4_IRQHandler(void)                	
{
	u8 Res;
#if SYSTEM_SUPPORT_OS 		
	OSIntEnter();    
#endif
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res = USART_ReceiveData(UART4);	//读取接收到的数据
		
		if((UART4_RX_STA&0x8000)==0)//接收未完成
		{
			if(UART4_RX_STA&0x4000)//接收到了0x0d
			{
				if(Res!=0x0a)UART4_RX_STA=0;//接收错误,重新开始
				else UART4_RX_STA|=0x8000;	//接收完成了 
			}
			else //还没收到0X0D
			{	
				if(Res==0x0d)UART4_RX_STA|=0x4000;
				else
				{
					UART4_RX_BUF[UART4_RX_STA&0X3FFF]=Res ;
					UART4_RX_STA++;
					if(UART4_RX_STA>(UART4_REC_LEN-1))UART4_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}   		 
  } 
#if SYSTEM_SUPPORT_OS 	
	OSIntExit();  											 
#endif
} 
    
// ==========================================================
// 封装函数：通过 UART4 (printf) 打印 Jetson 传来的解析数据
// 参数：
//   Motor_Speed_Left: Jetson 下发的左轮速度
//   Motor_Speed_Right : Jetson 下发的右轮速度
//   Command: Jetson 下发的状态/动作字节 (Byte 3)
// ==========================================================
// ==========================================================
// 封装函数：UART4 全栈数据综合看板 (支持整型与浮点型混合)
// ==========================================================
  
//printf("int_data: %d\r\n", data);


#endif



