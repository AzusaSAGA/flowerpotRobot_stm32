#include "UART4.h"  
#include "sys.h"


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
}

void USART_SendData_String(USART_TypeDef* USARTx, u8 *str)
{
    while(*str)
    {
        while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET); // 等待发送完成
        USART_SendData(USARTx, *str++);
    }
    while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
}
// 核心函数：发送字符串
void UART4_SendString(u8 *str)
{
    while(*str)  // 当指针指向的字符不是 '\0' 时
    {
        while(USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET);  // 等待发送完成
        USART_SendData(UART4, *str++);  // 发送字符并移动指针
    }
}

void uart4_send_byte(u8 data)
{
    while(USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET);
    USART_SendData(UART4, data);
}

void uart4_send_string(u8 *str)
{
    while(*str)
    {
        uart4_send_byte(*str++);
    }
}

// 发送数字并换行
void UART4_SendNumberln(int num)
{
    char str[20];
    sprintf(str, "%d ", num);
    UART4_SendString((u8*)str);
}

// 发送十六进制格式
void UART4_SendHex(uint8_t num)
{
    char str[10];
    sprintf(str, "%02X ", num);
    UART4_SendString((u8*)str);
}
 



