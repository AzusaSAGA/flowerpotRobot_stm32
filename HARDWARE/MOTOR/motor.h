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

// ===== 枚举定义 =====
typedef enum
{
    MOTOR_LEFT = 0,
    MOTOR_RIGHT
} Motor_ID;

typedef enum
{
    MOTOR_STOP_FREE = 0, 
    MOTOR_BRAKE,         
    MOTOR_FORWARD,       
    MOTOR_REVERSE        
} Motor_Mode;

// ===== API =====
void Motor_Init(void);
//void Motor_Control(Motor_ID motor, Motor_Mode mode, uint8_t speed_percent);

void Motor_Move(int PWM_left,int PWM_right);


// ==========================================
// ===== 上层应用 API：小车运动姿态控制 ======
// ==========================================
// 参数 speed 范围: 0 ~ 1000 (代表 0% ~ 100% 占空比)
//void Car_Forward(int speed);
//void Car_Backward(int speed);
//void Car_TurnLeft(int speed);
//void Car_TurnRight(int speed);
//void Car_Stop(void);



#endif
