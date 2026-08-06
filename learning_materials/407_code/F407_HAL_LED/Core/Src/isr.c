#include "isr.h"

/*                   定时器中断                     */
//以定时器6启动单线程调度循环，周期1ms
void TIM6_LOOP(void)  
{

}
//中断回调函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)
      {
        TIM6_LOOP();
      }
}

void loop_10ms_task(void)
{

}

void loop_10s_task(void)
{

}

/*                   定时器中断                     */



////////////////////////////////////////////////////////////////////////









/*                   外部中断                     */


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == SW1_Pin)
	{
		GPIO_LED_OPEN();
	}
	else if(GPIO_Pin == SW3_Pin)
	{
		GPIO_LED_CLOSE();
	}
	
	
}


/*                   外部中断                     */

