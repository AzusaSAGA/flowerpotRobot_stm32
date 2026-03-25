#ifndef POWER_H
#define POWER_H

#include "sys.h"
#include "delay.h"
#include "oled.h"

void  POWER_Init(void);
static u16 Get_Adc(u8 ch);
static u16 Get_Adc_Average(u8 ch,u8 times);
u8 Get_voltage();
void show_voltage(u8 mode);

#endif