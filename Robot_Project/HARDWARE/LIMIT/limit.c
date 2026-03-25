#include "LIMIT.h"
#include "stepper.h"
#include "delay.h"
/**
  * @brief  初始化限位器引脚 (PB8, PB9)
  * @note   配置为上拉输入。未碰到时为高电平(1)，碰到接地为低电平(0)
  */
void Limit_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(LIMIT_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = LIMIT_ARM_PIN | LIMIT_CLAW_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;        // 输入模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;        // 必须开启上拉电阻
    GPIO_Init(LIMIT_PORT, &GPIO_InitStructure);
}

uint8_t Limit_Read_Arm(void) {
    return GPIO_ReadInputDataBit(LIMIT_PORT, LIMIT_ARM_PIN);
}

uint8_t Limit_Read_Claw(void) {
    return GPIO_ReadInputDataBit(LIMIT_PORT, LIMIT_CLAW_PIN);
}