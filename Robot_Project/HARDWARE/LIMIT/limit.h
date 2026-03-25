#ifndef LIMIT_H
#define LIMIT_H
#include "sys.h"
#include "stepper.h"

/* --- 引脚宏定义 --- */
#define LIMIT_ARM_PIN    GPIO_Pin_8   // 机械臂抬起限位器 PB8
#define LIMIT_CLAW_PIN   GPIO_Pin_9   // 机械爪抓取限位器 PB9
#define LIMIT_PORT       GPIOB
#define LIMIT_CLK        RCC_AHB1Periph_GPIOB

/* --- 状态逻辑 --- */
#define LIMIT_TRIGGERED  0  // 碰到限位器
#define LIMIT_FREE       1  // 未碰到限位器

#ifdef __cplusplus
extern "C" {
#endif

void Limit_Init(void);          // 初始化 GPIO
uint8_t Limit_Read_Arm(void);   // 返回机械臂限位状态 (0:触发, 1:未触发)
uint8_t Limit_Read_Claw(void);  // 返回机械爪限位状态 (0:触发, 1:未触发)

#ifdef __cplusplus
}
#endif


#endif
