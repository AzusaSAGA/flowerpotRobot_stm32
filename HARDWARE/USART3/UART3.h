#ifndef __UART3_H
#define __UART3_H

#include "sys.h"
// 接收 Jetson 发来的 5 字节帧: [帧头][左PWM][右PWM][指令][校验]
#define USART3_REC_LEN  5  	//定义最大接收字节数 200
#define EN_USART3_RX 	1//使能（1）/禁止（0）串口1接收
#define SEND_DMA    0    // 使用DMA发送数据
#define SEND_NO_DMA 1    // 不使用DMA发送数据

#pragma pack(push,1)   //保存当前对齐方式，并设置为1字节对齐
// STM32 发送给 Jetson 的 11 字节帧
typedef struct {
	uint8_t header;         // Byte 0: [帧头] 0x55
	int8_t data1;          // Byte 1: [左编码器]
	int8_t data2;          // Byte 2: [右编码器]
	uint8_t status;         // Byte 3: [状态位] (低3位有效)
	uint8_t checksum;       // Byte 4: [校验位]
}USART3_Frame;
#pragma pack(pop)      // 恢复之前的对齐方式

extern USART3_Frame tx_frame;  // 编码器数据
extern volatile u8 USART3_RX_BUF[USART3_REC_LEN];
extern volatile u8 USART3_RX_completed;

void uart3_init(unsigned long baudrate);
#if SEND_DMA
void DMA1_Stream3_Config(void);
void USART3_Send_DMA(void);
#endif
void Prepare_Frame(int8_t value1, int8_t value2);
void USART3_Send_Data(uint8_t *data, uint8_t length);


#endif

//------------------End of File----------------------------

