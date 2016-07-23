#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"



extern void TIM2_Int_Init(u16 arr,u16 psc);
extern void TIM3_PWM_Init(u16 arr,u16 psc);
extern void TIM5_Cap_Init(u16 arr,u16 psc);
#endif

