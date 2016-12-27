#ifndef __LED_H
#define __LED_H	 

#include "stm32f10x.h"

#define LED_RED(n)		GPIO_WriteBit(GPIOC,GPIO_Pin_6,(BitAction)n)
#define LED_GREEN(n)		GPIO_WriteBit(GPIOC,GPIO_Pin_0,(BitAction)n)

void LED_Init(void);		//≥ı ºªØ
		 				    
#endif
