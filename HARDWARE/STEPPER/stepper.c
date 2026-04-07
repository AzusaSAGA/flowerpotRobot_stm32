#include "stepper.h"
#include "LIMIT.h"
#include "delay.h"

///* ================== 串口底层发送 (极度稳定版) ================== */
//static void usart_SendCmd(USART_TypeDef* USARTx, uint8_t *cmd, uint8_t len) {
//    for(uint8_t i = 0; i < len; i++) {
//        USARTx->DR = cmd[i];
//        // 采用标准死等模式，直到当前字节被塞入移位寄存器
//        while((USARTx->SR & USART_FLAG_TXE) == 0); 
//    }
//    // 确保这 5 个或 13 个字节真真切切顺着电线发完，再允许单片机干别的事
//    while((USARTx->SR & USART_FLAG_TC) == 0); 
//}
/* ================== 串口通信 ================== */
static void usart_SendCmd(USART_TypeDef* USARTx, uint8_t *cmd, uint8_t len) {
    for(uint8_t i = 0; i < len; i++) {
        uint16_t timeout = 0;
        USARTx->DR = (cmd[i] & (uint16_t)0x01FF);
        while(!(USARTx->SR & USART_FLAG_TXE)) {
            if(++timeout > 8000) break; 
        }
    }
}
/**
  * @brief  步进电机与串口底层外设初始化
  * @note   USART1 (PA9/PA10) -> 机械爪 ; USART2 (PD5/PD6) -> 机械臂
  */
void Stepper_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); 

    // 机械爪 USART1 (PA9/PA10)
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 机械臂 USART2 (PD5/PD6)
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx; 
    USART_Init(USART1, &USART_InitStructure);
    USART_Init(USART2, &USART_InitStructure);

    USART_Cmd(USART1, ENABLE);
    USART_Cmd(USART2, ENABLE);
}

/* ==================  EMM 控制协议 ================== */
/**
  * @brief  向 EMM 驱动器发送位置模式控制指令 (0xFD)
  * @param  motor_id: 目标电机ID (MOTOR_ID_ARM 或 MOTOR_ID_CLAW)
  * @param  dir:      转动方向 (DIR_CW 或 DIR_CCW)
  * @param  vel:      目标转速 (0-3000 RPM)
  * @param  acc:      加速度 (0为直接启动，最大255)
  * @param  clk:      要发送的总脉冲数量 (0 ~ 4294967295)
  */
void Emm_V5_Pos_Control(uint8_t motor_id, uint8_t dir, uint16_t vel, uint8_t acc, uint32_t clk) {
    uint8_t cmd[13] = {0};
    cmd[0]  = 0x01;                       // 地址
    cmd[1]  = 0xFD;                       // 功能码
    cmd[2]  = dir;                        // 方向
    cmd[3]  = (uint8_t)(vel >> 8);        
    cmd[4]  = (uint8_t)(vel >> 0);        
    cmd[5]  = acc;                        
    cmd[6]  = (uint8_t)(clk >> 24);       
    cmd[7]  = (uint8_t)(clk >> 16);
    cmd[8]  = (uint8_t)(clk >> 8);
    cmd[9]  = (uint8_t)(clk >> 0);        
    cmd[10] = 0x00;                       // 相对位置运动
    cmd[11] = 0x00;                       
    cmd[12] = 0x6B;                       
    
    if (motor_id == MOTOR_ID_ARM) usart_SendCmd(USART2, cmd, 13);
    else usart_SendCmd(USART1, cmd, 13);
}
/**
  * @brief  向 EMM 驱动器发送瞬间急停指令 (0xFE)
  * @param  motor_id: 目标电机ID (MOTOR_ID_ARM 或 MOTOR_ID_CLAW)
  */
void Emm_V5_Stop_Now(uint8_t motor_id) {
    uint8_t cmd[5] = {0x01, 0xFE, 0x98, 0x00, 0x6B};
    if (motor_id == MOTOR_ID_ARM) usart_SendCmd(USART2, cmd, 5);
    else usart_SendCmd(USART1, cmd, 5);
}

/* ================== 核心：串口驱动智能等待函数 ================== */
/**
  * @brief  挂起 MCU 等待电机走完，同时带有抗干扰的硬限位急停拦截逻辑
  * @param  motor_id:     执行此动作的电机 (MOTOR_ID_ARM 或 MOTOR_ID_CLAW)
  * @param  dir:          当前运动方向 (决定是否启用限位拦截)
  * @param  vel:          目标速度 RPM
  * @param  acc:          加速度
  * @param  clk:          预设脉冲数
  * @param  wait_time_ms: 超时/最大等待时间 (ms)
  */
static void Smart_Motor_Pos_Run(uint8_t motor_id, uint8_t dir, uint16_t vel, uint8_t acc, uint32_t clk, uint32_t wait_time_ms) {
    // 1. 通过串口发送动作指令给 EMM 驱动器
    Emm_V5_Pos_Control(motor_id, dir, vel, acc, clk);

   // 2. 毫秒级限位监控循环
    uint32_t t = 0;
    while (t < wait_time_ms) {
        
        // --- 机械爪硬限位拦截 (加入方向豁免) ---
        // 只有在 dir == DIR_CW (抓紧方向) 时，才去管限位器！
        if (dir == DIR_CW && motor_id == MOTOR_ID_CLAW && Limit_Read_Claw() == 0) {
            for(uint8_t i = 0; i < 3; i++) {
                Emm_V5_Stop_Now(MOTOR_ID_CLAW); 
                delay_ms(2);
            }
            break; 
        }
        
        // --- 机械臂硬限位拦截 (加入方向豁免) ---
        // 只有在 dir == DIR_CW (抬起方向) 时，才去管限位器！
        if (dir == DIR_CW && motor_id == MOTOR_ID_ARM && Limit_Read_Arm() == 0) {
            for(uint8_t i = 0; i < 3; i++) {
                Emm_V5_Stop_Now(MOTOR_ID_ARM);  
                delay_ms(2);
            }
            break;
        }

        delay_ms(1);
        t++;
    }
    
//    // 3. 超时兜底保险
//    if (t >= wait_time_ms) {
//        Emm_V5_Stop_Now(motor_id);
//    }
}
/* ==========================================================
 * 底层物理常量换算 (基于 200步进电机 * 16细分)
 * ========================================================== */
// 机械爪：1圈 = 200 * 16 = 3200 脉冲
#define CLAW_PULSE_PER_REV   3200.0f

// 机械臂：1度 = (1/360) * 200步 * 16细分 * 200减速比 ≈ 1777.778 脉冲
#define ARM_PULSE_PER_DEG    1777.778f

/* ==========================================================
 * 四大独立业务动作 (动态传参 + 自动时间推算)
 * ========================================================== */

void Action_1_Claw_Grab(uint16_t rpm, uint8_t acc, float max_revs) {
    uint32_t pulse = (uint32_t)(max_revs * CLAW_PULSE_PER_REV);
    
    // 自动计算需要的时间(毫秒)：时间 = (圈数 / 每分钟转速) * 60秒 * 1000
    // 乘以 1.2 是为了留出 20% 的时间余量，防止被提前截断
    uint32_t timeout_ms = (uint32_t)((max_revs * 60000.0f / rpm) );
    if(timeout_ms < 500) timeout_ms = 500; // 兜底最低2秒
    
    Smart_Motor_Pos_Run(MOTOR_ID_CLAW, DIR_CW, rpm, acc, pulse, timeout_ms);
}

void Action_2_Claw_Open(uint16_t rpm, uint8_t acc, float revs) {
    uint32_t pulse = (uint32_t)(revs * CLAW_PULSE_PER_REV);
    
    uint32_t wait_time_ms = (uint32_t)((revs * 60000.0f / rpm) );
    if(wait_time_ms < 500) wait_time_ms = 500;
    
    Smart_Motor_Pos_Run(MOTOR_ID_CLAW, DIR_CCW, rpm, acc, pulse, wait_time_ms);
}

void Action_3_Arm_Lift(uint16_t rpm, uint8_t acc, float max_angle) {
    uint32_t pulse = (uint32_t)(max_angle * ARM_PULSE_PER_DEG);
    
    // 机械臂电机实际转的圈数 = 角度 * 200减速比 / 360
    float motor_revs = max_angle * 200.0f / 360.0f;
    uint32_t timeout_ms = (uint32_t)((motor_revs * 60000.0f / rpm) );
    if(timeout_ms < 3000) timeout_ms = 3000; // 兜底最低5秒
    
    Smart_Motor_Pos_Run(MOTOR_ID_ARM, DIR_CW, rpm, acc, pulse, timeout_ms);
}

void Action_4_Arm_Lower(uint16_t rpm, uint8_t acc, float angle) {
    uint32_t pulse = (uint32_t)(angle * ARM_PULSE_PER_DEG);
    
    float motor_revs = angle * 200.0f / 360.0f;//* 1.2f
    uint32_t wait_time_ms = (uint32_t)((motor_revs * 60000.0f / rpm) );
    if(wait_time_ms < 1000) wait_time_ms = 1000;
    
    Smart_Motor_Pos_Run(MOTOR_ID_ARM, DIR_CCW, rpm, acc, pulse, wait_time_ms);
}

/* ==========================================================
 * 拾取、放置与复位 (保留机械形变缓冲)
 * ========================================================== */
void Stepper_Action_PickUp(void) {
    Action_4_Arm_Lower(200, 0, 20.0f); //105.0f
    delay_ms(100); // 停稳避震
    Action_1_Claw_Grab(300, 0, 20.0f); 
    delay_ms(100); // 夹紧确认避震
    Action_3_Arm_Lift(200, 0, 50.0f); //120.0f
}

void Stepper_Action_PutDown(void) {
    Action_4_Arm_Lower(200, 0, 20.0f);
    delay_ms(100); 
    Action_2_Claw_Open(300, 0, 10.0f); 
    delay_ms(100); 
    Action_3_Arm_Lift(200, 0, 50.0f); 
}

void Stepper_Action_Idle(void) {
    
}


/* ==========================================================
 * 复位动作：双电机并行归零 (开机初始化寻找零点)
 * ========================================================== */

void Home_All_Simultaneously(void) {
    uint8_t arm_done = 0;
    uint8_t claw_done = 0;
    uint32_t timeout_cnt = 0; 
    uint8_t claw_shake_cnt = 0; 
    uint8_t arm_shake_cnt = 0;  

    // 启动归零
    if (Limit_Read_Claw() != 0) { 
        Emm_V5_Pos_Control(MOTOR_ID_CLAW, DIR_CW, 120, 0, 999999);
    } else claw_done = 1;

    if (Limit_Read_Arm() != 0) {
        Emm_V5_Pos_Control(MOTOR_ID_ARM, DIR_CW, 300, 0, 999999);
    } else arm_done = 1;

    // 监控限位
    while ((!arm_done || !claw_done) && timeout_cnt < 20000) {
        if (!claw_done && Limit_Read_Claw() == 0) {
            claw_shake_cnt++;
            if (claw_shake_cnt > 15) { 
                for(uint8_t i = 0; i < 3; i++) { Emm_V5_Stop_Now(MOTOR_ID_CLAW); delay_ms(2); }
                claw_done = 1;
            }
        } else if (!claw_done) claw_shake_cnt = 0;
        
        if (!arm_done && Limit_Read_Arm() == 0) {
            arm_shake_cnt++;
            if (arm_shake_cnt > 15) { 
                for(uint8_t i = 0; i < 3; i++) { Emm_V5_Stop_Now(MOTOR_ID_ARM); delay_ms(2); }
                arm_done = 1;
            }
        } else if (!arm_done) arm_shake_cnt = 0;
        
        delay_ms(1);
        timeout_cnt++;
    }
    
    // 超时兜底
    if (timeout_cnt >= 20000) {
        if (!claw_done) for(uint8_t i=0; i<3; i++) { Emm_V5_Stop_Now(MOTOR_ID_CLAW); delay_ms(2); }
        if (!arm_done) for(uint8_t i=0; i<3; i++) { Emm_V5_Stop_Now(MOTOR_ID_ARM); delay_ms(2); }
    }
    
    // ==========================================
    // ★ 核心绝杀：解除驱动器的“急停锁定”状态
    // ==========================================
    delay_ms(300); // 1. 先等电机卸力，刹车余震消散
    
    // 2. 给两个电机各发一条“转动 0 脉冲”的空指令。
    // 这骗过了驱动器，让它以为要执行新动作，从而退出 0xFE 的锁定状态，但电机原地不动！
    Emm_V5_Pos_Control(MOTOR_ID_CLAW, DIR_CCW, 100, 0, 0); 
    Emm_V5_Pos_Control(MOTOR_ID_ARM, DIR_CCW, 100, 0, 0); 
    
    delay_ms(200); // 3. 唤醒完毕，状态机刷新，准备接收真实指令
}

//void Home_All_Simultaneously(void) {
//    uint8_t arm_done = 0;
//    uint8_t claw_done = 0;
//    uint32_t timeout_cnt = 0; // 超时计数器，防止限位器损坏导致死机

//    // 1. 下发超大脉冲指令，让它向着限位器(CW)一直转
//    // 注意：归零时速度一定要慢，防止撞击过猛。这里统一使用 120 RPM
//    if (Limit_Read_Claw() != 0) { // 限位器未触发时启动
//        Emm_V5_Pos_Control(MOTOR_ID_CLAW, DIR_CW, 120, 0, 999999);
//    } else {
//        claw_done = 1;
//    }

//    if (Limit_Read_Arm() != 0) {
//        Emm_V5_Pos_Control(MOTOR_ID_ARM, DIR_CW, 120, 0, 999999);
//    } else {
//        arm_done = 1;
//    }

//    // 2. 并行死循环监控，微秒级急停响应 (设定最长等待时间 20 秒)
//    while ((!arm_done || !claw_done) && timeout_cnt < 20000) {
//        
//        // --- 机械爪碰到限位器 ---
//        if (!claw_done && Limit_Read_Claw() == 0) {
//            // 抗干扰 3 连发，防止串口丢包撞坏结构
//            for(uint8_t i = 0; i < 3; i++) {
//                Emm_V5_Stop_Now(MOTOR_ID_CLAW); 
//                delay_ms(2);
//            }
//            claw_done = 1;
//        }
//        
//        // --- 机械臂碰到限位器 ---
//        if (!arm_done && Limit_Read_Arm() == 0) {
//            // 抗干扰 3 连发急停
//            for(uint8_t i = 0; i < 3; i++) {
//                Emm_V5_Stop_Now(MOTOR_ID_ARM);  
//                delay_ms(2);
//            }
//            arm_done = 1;
//        }
//        
//        delay_ms(1);
//        timeout_cnt++;
//    }
//    
//    // 3. 超时兜底保险 (如果20秒还没碰到限位器，强制停下，保护电机)
//    if (timeout_cnt >= 20000) {
//        if (!claw_done) {
//            for(uint8_t i = 0; i < 3; i++) { Emm_V5_Stop_Now(MOTOR_ID_CLAW); delay_ms(2); }
//        }
//        if (!arm_done) {
//            for(uint8_t i = 0; i < 3; i++) { Emm_V5_Stop_Now(MOTOR_ID_ARM); delay_ms(2); }
//        }
//    }
//    
//    // 4. 归零完成，电机锁定，给 500ms 缓冲防止机械抖动
//    delay_ms(300); 
//}