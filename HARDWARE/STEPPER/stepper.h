#ifndef STEPPER_H
#define STEPPER_H
#include "sys.h"
#include <stdbool.h>

extern uint8_t flag_grasp_done;
extern uint8_t flag_place_done;

/* ==========================================================
 * [搬花业务核心数值配置区]
 * ========================================================== */
// --- 电机 ID 定义 --- 
#define MOTOR_ID_ARM          0    // 机械臂 (绑定USART2)
#define MOTOR_ID_CLAW         1    // 机械爪 (绑定USART1)

// --- 方向定义 ---
#define DIR_CW   0   // 机械爪:抓取(向限位器) | 机械臂:抬起(向限位器)
#define DIR_CCW  1   // 机械爪:松开(离限位器) | 机械臂:放下(离限位器)

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================================
 * 底层与协议接口声明
 * ========================================================== */
void Stepper_Init(void);
void Emm_V5_Pos_Control(uint8_t motor_id, uint8_t dir, uint16_t vel, uint8_t acc, uint32_t clk);
void Emm_V5_Stop_Now(uint8_t motor_id);
void Home_All_Simultaneously(void);

/* ==========================================================
 * 四大独立动作接口 (支持在 main 函数中动态传入物理量)
 * ========================================================== */

/**
  * @brief  动作1：机械爪抓取 (CW方向，碰 PB9 限位器急停)
  * @param  rpm:      目标转速。取值范围: 1 ~ 3000 (RPM)。建议值: 600。
  * @param  acc:      加速度。取值范围: 0 ~ 255。0代表直接启动，数值越大加速越柔和。建议值: 0。
  * @param  max_revs: 最大允许转动圈数。取值范围: > 0 (float)。作为防堵死超时保护距离，实际碰到限位器会提前停。建议值: 15.0f。
  */
void Action_1_Claw_Grab(uint16_t rpm, uint8_t acc, float max_revs);

/**
  * @brief  动作2：机械爪松开 (CCW方向，精确走完设定圈数停止)
  * @param  rpm:      目标转速。取值范围: 1 ~ 3000 (RPM)。建议值: 600。
  * @param  acc:      加速度。取值范围: 0 ~ 255。建议值: 0。
  * @param  revs:     需要精确转动的圈数。取值范围: > 0 (float)。建议值: 10.0f。
  */
void Action_2_Claw_Open(uint16_t rpm, uint8_t acc, float revs);

/**
  * @brief  动作3：机械臂抬起 (CW方向，碰 PB8 限位器急停)
  * @param  rpm:      目标转速。取值范围: 1 ~ 3000 (RPM)。重载建议慢速，建议值: 100~200。
  * @param  acc:      加速度。取值范围: 0 ~ 255。带减速器建议加柔性启动保护齿轮，建议值: 50。
  * @param  max_angle:最大允许抬升角度。取值范围: > 0 (float度)。作为保护距离，碰限位提前停。建议值: 90.0f。
  */
void Action_3_Arm_Lift(uint16_t rpm, uint8_t acc, float max_angle);

/**
  * @brief  动作4：机械臂放下 (CCW方向，精确走完设定角度停止)
  * @param  rpm:      目标转速。取值范围: 1 ~ 3000 (RPM)。建议值: 100~200。
  * @param  acc:      加速度。取值范围: 0 ~ 255。建议值: 50。
  * @param  angle:    需要精确放下的角度。取值范围: > 0 (float度)。建议值: 85.0f。
  */
void Action_4_Arm_Lower(uint16_t rpm, uint8_t acc, float angle);

/* =======================================================
 * 拾取、放置与闲置封装
 * ======================================================= */

void Stepper_Action_PickUp(void);   //抓取 (Pick)
void Stepper_Action_PutDown(void);  //下放 (Place)
void Stepper_Action_Idle(void);// 闲置


#ifdef __cplusplus
}
#endif

#endif


