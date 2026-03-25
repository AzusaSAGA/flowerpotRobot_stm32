#include "motor.h"

void Motor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_BaseStruct;
    TIM_OCInitTypeDef TIM_OCStruct;

    // ===== 1. 使能时钟 =====
    // 开启 GPIOC (PWM) 和 GPIOE (EN) 时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOE, ENABLE);
    // 关键修改：TIM8 挂载在 APB2 总线上 (168MHz)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);

    // ===== 2. 使能端 GPIO 初始化 =====
    GPIO_InitStruct.GPIO_Pin = MOTOR_EN_L_PIN | MOTOR_EN_R_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; 
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    // 假设 EN_L 和 EN_R 都在 GPIOE，如果是不同 PORT 需要分开写
    GPIO_Init(GPIOE, &GPIO_InitStruct);

    // 默认安全状态：失能 (置1)
    GPIO_SetBits(GPIOE, MOTOR_EN_L_PIN | MOTOR_EN_R_PIN);

    // ===== 3. PWM端复用 GPIO 初始化 =====
    GPIO_InitStruct.GPIO_Pin = MOTOR_PWML2_PIN | MOTOR_PWML1_PIN | MOTOR_PWMR1_PIN | MOTOR_PWMR2_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;      
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(MOTOR_PWML1_PORT, &GPIO_InitStruct); // 假设都在 GPIOC

    // 连接引脚到 TIM8
    GPIO_PinAFConfig(MOTOR_PWML2_PORT, MOTOR_PWML2_PINSRC, MOTOR_PWM_AF); // PC6 -> TIM8_CH1
    GPIO_PinAFConfig(MOTOR_PWML1_PORT, MOTOR_PWML1_PINSRC, MOTOR_PWM_AF); // PC7 -> TIM8_CH2
    GPIO_PinAFConfig(MOTOR_PWMR1_PORT, MOTOR_PWMR1_PINSRC, MOTOR_PWM_AF); // PC8 -> TIM8_CH3
    GPIO_PinAFConfig(MOTOR_PWMR2_PORT, MOTOR_PWMR2_PINSRC, MOTOR_PWM_AF); // PC9 -> TIM8_CH4

    // ===== 4. TIM8 PWM 基础配置 =====
    // 关键修改：APB2 是 168MHz，所以分频器用 168-1
    TIM_BaseStruct.TIM_Prescaler = 168 - 1;        // 168 MHz -> 1 MHz
    TIM_BaseStruct.TIM_Period = MOTOR_PWM_MAX - 1; // PWM 1 kHz
    TIM_BaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_BaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM8, &TIM_BaseStruct);

    // ===== 5. TIM8 PWM 通道配置 (CH1~CH4) =====
    TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCStruct.TIM_Pulse = 0;
    TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    
    // TIM8 的互补通道配置（即使不用也要设个默认值，防止高级定时器出妖蛾子）
    TIM_OCStruct.TIM_OutputNState = TIM_OutputNState_Disable;
    TIM_OCStruct.TIM_OCNPolarity = TIM_OCNPolarity_High;
    TIM_OCStruct.TIM_OCIdleState = TIM_OCIdleState_Reset;
    TIM_OCStruct.TIM_OCNIdleState = TIM_OCNIdleState_Reset;

    TIM_OC1Init(TIM8, &TIM_OCStruct); // L2
    TIM_OC2Init(TIM8, &TIM_OCStruct); // L1
    TIM_OC3Init(TIM8, &TIM_OCStruct); // R1
    TIM_OC4Init(TIM8, &TIM_OCStruct); // R2

    // 使能预装载寄存器
    TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM8, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM8, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Enable);

    // ===== 6. 开启输出 =====
    // 极其关键：高级定时器 TIM8 必须开启 MOE 主输出使能！
    TIM_CtrlPWMOutputs(TIM8, ENABLE);
    TIM_Cmd(TIM8, ENABLE);
}


//控制方向和速度
void Motor_Move(int PWM_left,int PWM_right){ 
    GPIO_ResetBits(MOTOR_EN_L_PORT, MOTOR_EN_L_PIN);
    if(PWM_left>0){
        TIM_SetCompare2(TIM8, 0);
        TIM_SetCompare1(TIM8, PWM_left);
    }else {
        TIM_SetCompare2(TIM8, -PWM_left);
        TIM_SetCompare1(TIM8, 0);
    }
    
    GPIO_ResetBits(MOTOR_EN_R_PORT, MOTOR_EN_R_PIN);
    if(PWM_right>0){
        TIM_SetCompare3(TIM8, PWM_right);            
        TIM_SetCompare4(TIM8, 0);
    }else {
        TIM_SetCompare3(TIM8, 0);            
        TIM_SetCompare4(TIM8, -PWM_right);
    }
}




