#ifndef Key_H
#define Key_H
#include "sys.h"

extern u8 key_value;

#define KEY1 		PEin(0)   	
#define KEY2 		PEin(1)		
#define KEY3		PEin(2)

void KEY_Init(void);
u8 KEY_Scan(u8 mode);
#endif
