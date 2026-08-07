#include "Init.h"
void all_init(void)  //未调用
{
    HAL_TIM_Base_Start_IT(&htim6);  //定时器使能

}


int fputc(int ch,FILE *f) //重定向
{
	HAL_UART_Transmit(&huart1,(uint8_t *)&ch,1,HAL_MAX_DELAY);
	return ch;
}
