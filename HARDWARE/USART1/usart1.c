//#include "uart4.h"
//u8 UART4_RX_BUF[UART4_REC_LEN];       //接收缓冲,最大UART4_REC_LEN个字节.
//u16 UART4_RX_STA = 0;                 //接收状态标记	
//u8 res = 0;
//void UART4_Init(unsigned long baudrate)
//{
//    GPIO_InitTypeDef GPIO_InitStructure;
//    USART_InitTypeDef USART_InitStructure;
//    NVIC_InitTypeDef NVIC_InitStructure;
//	
//    // 1. 使能时钟
//    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); //使能GPIOC时钟
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE); //使能UART4时钟 (UART4挂载在APB1上)
// 
//    // 2. 串口4对应引脚复用映射
//    GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_UART4); //GPIOC10复用为UART4
//    GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_UART4); //GPIOC11复用为UART4
//	
//    // 3. UART4端口配置 (PC10, PC11)
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; //GPIOC10与GPIOC11
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;   //复用功能
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //速度50MHz
//    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
//    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   //上拉
//    GPIO_Init(GPIOC, &GPIO_InitStructure);         //初始化PC10，PC11

//    // 4. UART4 初始化设置
//    USART_InitStructure.USART_BaudRate = baudrate; //波特率设置
//    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //字长为8位数据格式
//    USART_InitStructure.USART_StopBits = USART_StopBits_1; //一个停止位
//    USART_InitStructure.USART_Parity = USART_Parity_No; //无奇偶校验位
//    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件数据流控制
//    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //收发模式
//    USART_Init(UART4, &USART_InitStructure); //初始化串口4
//	
//    USART_Cmd(UART4, ENABLE);  //使能串口4 
//	
//    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE); //开启接收中断

//    // 5. UART4 NVIC 配置
//    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn; //串口4中断通道
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //抢占优先级3
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;        //子优先级3
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           //IRQ通道使能
//    NVIC_Init(&NVIC_InitStructure); //根据指定的参数初始化VIC寄存器
//}

//void UART4_IRQHandler(void)
//{
//    if(USART_GetITStatus(UART4, USART_IT_TXE) != RESET){   
//        USART_SendData(UART4, tmp_buf[1]); 
//        USART_ClearITPendingBit(UART4, USART_IT_TXE); 
//    } else if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET){
//        res = USART_ReceiveData(UART4);
//        USART_ClearITPendingBit(UART4, USART_IT_RXNE);
//    }
//    USART_ClearITPendingBit(UART4, USART_IT_ORE); // 清除溢出中断标志
//}
