#ifndef __UART3_H
#define __UART3_H
#define USART3_REC_LEN  			3  	//定义最大接收字节数 200
#define EN_USART3_RX 			1		//使能（1）/禁止（0）串口1接收
#define SEND_DMA    0    // 使用DMA发送数据
#define SEND_NO_DMA 1    // 不使用DMA发送数据

#include "sys.h"

#pragma pack(push,1)   //保存当前对齐方式，并设置为1字节对齐
typedef struct {
	uint8_t header;
	float data1;    // 左轮速度
	float data2;    // 右轮速度
	uint8_t checksum;
}USART3_Frame;
#pragma pack(pop)      // 恢复之前的对齐方式

extern USART3_Frame tx_frame;  // 编码器数据

void uart3_init(unsigned long baudrate);
#if SEND_DMA
void DMA1_Stream3_Config(void);
void USART3_Send_DMA(void);
#endif
void Prepare_Frame(float value1, float value2);
void USART3_Send_Data(uint8_t *data, uint8_t length);
extern volatile u8 USART3_RX_BUF[USART3_REC_LEN];
extern volatile u8 USART3_RX_completed;
#endif

//------------------End of File----------------------------

