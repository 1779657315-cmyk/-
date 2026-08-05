#include "Init.h"
void all_init(void)
{
    HAL_TIM_Base_Start_IT(&htim6);  //定时器使能

}
