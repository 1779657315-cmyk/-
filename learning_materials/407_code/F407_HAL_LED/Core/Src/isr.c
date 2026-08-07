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

uint8_t AT[]="AT\r\n";
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == SW1_Pin)
	{
		GPIO_LED_OPEN();
		HAL_UART_Transmit_DMA(&huart3,AT,sizeof(AT) - 1);
	}
	else if(GPIO_Pin == SW3_Pin)
	{
		GPIO_LED_CLOSE();
	}
	
	
}


/*                   外部中断                     */

////////////////////////////////////////////////////////////////////////






/*                   串口接事件中断回调入口                     */

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
		if(huart == &huart1)  //串口中断
		{
			printf("串口1已收到不定长数据%d个\r\n",Size);
			HAL_UART_Transmit_IT(&huart1,CH340_RceBuffer,Size); //串口回显操作
			HAL_UARTEx_ReceiveToIdle_IT(&huart1,CH340_RceBuffer,CH340_MAX_LENGTH);
		}
		
		else if(huart == &huart3) //WIFI_DMA中断
		{
			printf("串口3已收到不定长数据%d个\r\n",Size);
			HAL_UART_Transmit_IT(&huart1,WIFI_RceBuffer,Size); //WIFI转移串口回显操作
			HAL_UARTEx_ReceiveToIdle_DMA(&huart3,WIFI_RceBuffer,WIFI_MAX_INDEX);
		}
		
}
	

/**
  * @brief UART 错误回调函数
  * @note 当 UART 出现错误时触发
  *       - USART3: 清除错误标志，并重新开启 DMA 接收
  * @param huart: UART 句柄指针
  * @retval 无
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart3)
    {
        // 清除帧错误标志
        __HAL_UART_CLEAR_FEFLAG(&huart3);

        // 重新启动 ESP8266 DMA 接收
        HAL_UARTEx_ReceiveToIdle_DMA(&huart3,WIFI_RceBuffer,WIFI_MAX_INDEX);
    }
}


	/*                   串口中断                     */




