#ifndef __ISR_H__
#define __ISR_H__

/* Includes ------------------------------------------------------------------*/
#include "Init.h"

void TIM6_LOOP(void);
void loop_10ms_task(void);
void loop_10s_task(void);

#endif
