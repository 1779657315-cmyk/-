/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "Init.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, LED_R_Pin|LED_B_Pin|LED_G_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : LED_R_Pin LED_B_Pin LED_G_Pin */
  GPIO_InitStruct.Pin = LED_R_Pin|LED_B_Pin|LED_G_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */
void GPIO_RGB_LED(void)
{
		HAL_GPIO_WritePin(GPIOF, LED_G_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOF, LED_B_Pin | LED_R_Pin, GPIO_PIN_SET);
    HAL_Delay(1500);
		HAL_GPIO_WritePin(GPIOF, LED_B_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOF, LED_G_Pin | LED_R_Pin, GPIO_PIN_SET);
		HAL_Delay(1500);
		HAL_GPIO_WritePin(GPIOF, LED_R_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOF, LED_G_Pin | LED_B_Pin, GPIO_PIN_SET);
		HAL_Delay(1500);
}

void GPIO_LED_CLOSE(void)
{		//默认低电平，故高电平熄灭
    HAL_GPIO_WritePin(GPIOF, LED_G_Pin | LED_B_Pin | LED_R_Pin, GPIO_PIN_SET);
}
/* USER CODE END 2 */
