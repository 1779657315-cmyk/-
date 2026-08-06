#include "led.h"

void GPIO_RGB_LED(void)
{
		HAL_GPIO_WritePin(GPIOF, LED_G_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOF, LED_B_Pin | LED_R_Pin, GPIO_PIN_SET);
        HAL_Delay(500);
		HAL_GPIO_WritePin(GPIOF, LED_B_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOF, LED_G_Pin | LED_R_Pin, GPIO_PIN_SET);
		HAL_Delay(500);
		HAL_GPIO_WritePin(GPIOF, LED_R_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOF, LED_G_Pin | LED_B_Pin, GPIO_PIN_SET);
		HAL_Delay(500);
}

void GPIO_LED_OPEN(void)
{		
    HAL_GPIO_WritePin(GPIOF, LED_G_Pin | LED_B_Pin | LED_R_Pin, GPIO_PIN_RESET);
}

void GPIO_LED_CLOSE(void)
{		//默认低电平，故高电平熄灭
    HAL_GPIO_WritePin(GPIOF, LED_G_Pin | LED_B_Pin | LED_R_Pin, GPIO_PIN_SET);
}
