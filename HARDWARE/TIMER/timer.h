#ifndef __Timer_H
#define __Timer_H


#include "sys.h"
#include "oled.h"
#include "delay.h"
#include "key.h"
#include "power.h"
#include "24l01.h"
#include "stepper.h"

void TIM6_init(u16 arr,u16 psc);
void TIM7_init(u16 arr,u16 psc);

#endif
