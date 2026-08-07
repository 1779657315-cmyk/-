#include "esp8266.h"

void ESP8266_Init(void)
{
    HAL_GPIO_WritePin(WIFI_RST_GPIO_Port,WIFI_RST_Pin,GPIO_PIN_RESET);
		HAL_Delay(100);
    HAL_GPIO_WritePin(WIFI_RST_GPIO_Port,WIFI_RST_Pin,GPIO_PIN_SET);
		HAL_Delay(500);
}

