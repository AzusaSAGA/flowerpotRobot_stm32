#ifndef __UART3_H
#define __UART3_H
#include "UART4.h"
#include "stm32f4xx.h"
#include "sys.h"

// 接收 Jetson 发来的 5 字节帧: [帧头][左PWM][右PWM][指令][校验]
#define USART3_REC_LEN  5  	//定义最大接收字节数 200

extern volatile u8 USART3_RX_BUF[USART3_REC_LEN];
extern volatile u8 USART3_RX_completed;
extern volatile uint8_t USART3_RX_ERROR;   // 添加
extern volatile uint8_t USART3_RX_ORE;     // 添加

///////////////////////////
#pragma pack(push,1)   //保存当前对齐方式，并设置为1字节对齐
// STM32 发送给 Jetson 的 5 字节帧
typedef struct {
	uint8_t header;         // Byte 0: [帧头] 0x55
	uint32_t data1;          // Byte 1: [左编码器]
	uint32_t data2;          // Byte 2: [右编码器]
	uint8_t status;         // Byte 3: [状态位] (低3位有效)
	uint8_t checksum;       // Byte 4: [校验位]
}USART3_Frame;
#pragma pack(pop)      // 恢复之前的对齐方式
///////////////
extern USART3_Frame tx_frame;  // 编码器数据
void Prepare_Frame(uint32_t value1, uint32_t value2);
void USART3_Send_Data(uint8_t *data, uint8_t length);
///////////////////////////

/* 函数声明 */
void uart3_init(unsigned long baudrate);

#endif


//------------------End of File----------------------------

