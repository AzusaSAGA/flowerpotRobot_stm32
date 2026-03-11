#ifndef MOTOR_H
#define MOTOR_H

#include "sys.h"


// ==========================================
// ===== 硬件引脚宏定义 (底层修改只需动这里) =====
// ==========================================

// --- 使能端 (0能动) ---
#define MOTOR_EN_L_PORT     GPIOE
#define MOTOR_EN_L_PIN      GPIO_Pin_14

#define MOTOR_EN_R_PORT     GPIOE
#define MOTOR_EN_R_PIN      GPIO_Pin_15

// --- 左电机 PWM (TIM8) ---
#define MOTOR_PWML1_PORT    GPIOC
#define MOTOR_PWML1_PIN     GPIO_Pin_7  
#define MOTOR_PWML1_PINSRC  GPIO_PinSource7
#define MOTOR_PWML2_PORT    GPIOC
#define MOTOR_PWML2_PIN     GPIO_Pin_6  
#define MOTOR_PWML2_PINSRC  GPIO_PinSource6

// --- 右电机 PWM (TIM8) ---
#define MOTOR_PWMR1_PORT    GPIOC
#define MOTOR_PWMR1_PIN     GPIO_Pin_8  
#define MOTOR_PWMR1_PINSRC  GPIO_PinSource8
#define MOTOR_PWMR2_PORT    GPIOC
#define MOTOR_PWMR2_PIN     GPIO_Pin_9  
#define MOTOR_PWMR2_PINSRC  GPIO_PinSource9

// 统一复用映射到 TIM8
#define MOTOR_PWM_AF        GPIO_AF_TIM8

// ===== PWM 参数 =====
#define MOTOR_PWM_MAX       1000

// ==========================================
// ===== 对外 API 接口 =======================
// ==========================================

// 初始化电机外设
void Motor_Init(void);

// 控制小车运动 (输入范围: -1000 ~ +1000)
// PWM_left > 0 左轮正转， < 0 左轮反转
// PWM_right > 0 右轮正转， < 0 右轮反转
void Motor_Move(int PWM_left, int PWM_right);


#endif
