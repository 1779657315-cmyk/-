#include "esp8266.h"

/* Private variables ---------------------------------------------------------*/
uint8_t WIFI_RceBuffer[WIFI_MAX_INDEX];   //WIFI接收缓冲区
volatile bool wifi_receive_update = false;
volatile uint16_t wifi_rx_read_index = 0;
volatile uint16_t wifi_rx_write_index = 0;

/* Application layer ---------------------------------------------------------*/

/**
  * @brief  ESP8266 业务初始化：复位、连接 WiFi、连接 TCP，并进入透传模式。
  * @retval 无
  */
void WIFI_init(void)  //业务初始化(不包含外设配置)
{
//1.完成重启
		if(!ESP8266_Reset()) return;
//2.关闭回显
		if(!ESP8266_ATE_SET(false)) return;
//3.完成WIFI连接
		if(!ESP8266_wifi_connect()) return;
//4.完成TCP/IP连接
		if(!ESP8266_tcp_connect()) return;
//5.切换透传模式
		if(!ESP8266_transparent_mode_enter()) return;
//6.使能接收环形缓冲区
	
//7.进行初始化配置握手
			
}

/* ESP8266 low-level transport -----------------------------------------------*/

/**
  * @brief  发送 AT 指令，自动在指令末尾追加 "\r\n"。
  * @param  cmd: 不包含 "\r\n" 的 AT 指令字符串。
  * @retval 0 表示失败，1 表示成功。
  */
uint8_t ESP8266_AT_Send(const char *cmd)
{
	int len;

	// 清空接收缓冲区，防止残留数据影响后续检测   目前无意义
	memset(WIFI_RceBuffer, 0, WIFI_MAX_INDEX);
	
	// 格式化 AT 指令：在结尾添加回车换行
	char buf[128];
	len = snprintf(buf, sizeof(buf), "%s\r\n", cmd);

	if(len <= 0 || len >= sizeof(buf))
	{
		return 0;
	}
	
	// 发送指令到 ESP8266
	return ESP8266_SendBytes((const uint8_t *)buf, (uint16_t)len);
			
}
		
/**
  * @brief  通过 USART3 阻塞发送原始字节数据。
  * @param  data: 待发送数据指针。
  * @param  len: 待发送数据长度。
  * @retval 0 表示失败，1 表示成功。
  */
uint8_t ESP8266_SendBytes(const uint8_t *data, uint16_t len)
{	
	if(HAL_UART_Transmit(&huart3,(uint8_t*)data,len,HAL_MAX_DELAY) != HAL_OK)
	{
		return 0;
	}
	return 1;	
}

/**
  * @brief  阻塞等待 ESP8266 回包中出现目标字符串。
  * @param  target: 需要查找的目标字符串，例如 "OK"、"ready"、">"。
  * @param  timeout_ms: 单次等待超时时间，单位 ms；收到新回包后会刷新计时。
  * @retval 0 表示超时或未找到目标字符串，1 表示成功找到。
  */
uint8_t ESP8266_WaitResponse(const char *target, uint32_t timeout_ms)
{
	uint32_t tick_start = HAL_GetTick();

	while(!wifi_receive_update)
	{
		if(HAL_GetTick() - tick_start > timeout_ms)
		{
			memset(WIFI_RceBuffer, 0, WIFI_MAX_INDEX);
			return 0;
		}
	}

	wifi_receive_update = false;
	tick_start = HAL_GetTick();

	while(HAL_GetTick() - tick_start <= timeout_ms)
	{
		WIFI_RceBuffer[WIFI_MAX_INDEX - 1] = '\0';

		if(strstr((char *)WIFI_RceBuffer, target) != NULL)
		{
			memset(WIFI_RceBuffer, 0, WIFI_MAX_INDEX);
			return 1;
		}

		if(wifi_receive_update)
		{
			wifi_receive_update = false;
			tick_start = HAL_GetTick();
		}
	}

	memset(WIFI_RceBuffer, 0, WIFI_MAX_INDEX);
	return 0;
}

/* ESP8266 AT stage ----------------------------------------------------------*/

/**
  * @brief  复位 ESP8266，并等待模块输出 ready。
  * @retval 0 表示失败，1 表示成功。
  */
uint8_t ESP8266_Reset(void)
{
		const char *AT="AT+RST";  //重启
		if(!ESP8266_AT_Send(AT))
		{
			printf("ESP8266_Reset send failed\r\n");
			return 0;
		}
		if(!ESP8266_WaitResponse("ready", 5000))
		{
			printf("ESP8266_Reset wait ready timeout\r\n");
			return 0;
		}
		printf("ESP8266_Reset success\r\n");
		return 1;
}
		
/**
  * @brief  连接指定 WiFi 热点。
  * @retval 0 表示失败，1 表示成功。
  */
uint8_t ESP8266_wifi_connect(void)
{
		static const char *command = "AT+CWJAP=\"" WIFI_SSID "\",\"" WIFI_PASSWORD "\"";
		if(!ESP8266_AT_Send(command))
		{
			printf("ESP8266_wifi_connect send failed\r\n");
			return 0;
		}
		if(!ESP8266_WaitResponse("OK", 15000))
		{
			printf("ESP8266_wifi_connect wait OK timeout\r\n");
			return 0;
		}
		printf("ESP8266_wifi_connect success\r\n");
		return 1;
}
		
/**
  * @brief  连接指定 TCP 服务器。
  * @retval 0 表示失败，1 表示成功。
  */
uint8_t ESP8266_tcp_connect(void)
{
		static const char *command = "AT+CIPSTART=\"TCP\",\"" TCP_HOST "\"," TCP_PORT;
		if(!ESP8266_AT_Send(command))
		{
			printf("ESP8266_tcp_connect send failed\r\n");
			return 0;
		}
		if(!ESP8266_WaitResponse("OK", 5000))
		{
			printf("ESP8266_tcp_connect wait OK timeout\r\n");
			return 0;
		}
		printf("ESP8266_tcp_connect success\r\n");
		return 1;
}

/**
  * @brief  进入 TCP 透传模式。
  * @retval 0 表示失败，1 表示成功。
  */
uint8_t ESP8266_transparent_mode_enter(void)
{
	if(!ESP8266_AT_Send("AT+CIPMODE=1"))
	{
		printf("ESP8266_transparent_mode_enter CIPMODE send failed\r\n");
		return 0;
	}
	if(!ESP8266_WaitResponse("OK", 5000))
	{
		printf("ESP8266_transparent_mode_enter wait CIPMODE OK timeout\r\n");
		return 0;
	}

	if(!ESP8266_AT_Send("AT+CIPSEND"))
	{
		printf("ESP8266_transparent_mode_enter CIPSEND send failed\r\n");
		return 0;
	}
	if(!ESP8266_WaitResponse(">", 5000))
	{
		printf("ESP8266_transparent_mode_enter wait > timeout\r\n");
		return 0;
	}

	printf("ESP8266_transparent_mode_enter success\r\n");
	return 1;
}

/**
  * @brief  设置 ESP8266 AT 指令回显开关。
  * @param  model: true 打开回显，false 关闭回显。
  * @retval 0 表示失败，1 表示成功。
  */
uint8_t ESP8266_ATE_SET(bool model)
{
		if(model)
		{
			const char *AT="ATE1";  //打开回显
			if(!ESP8266_AT_Send(AT))
			{
				printf("ESP8266_ATE_SET ATE1 send failed\r\n");
				return 0;
			}
		}
		else
		{
			const char *AT="ATE0";	//关闭回显
			if(!ESP8266_AT_Send(AT))
			{
				printf("ESP8266_ATE_SET ATE0 send failed\r\n");
				return 0;
			}
		}

		if(!ESP8266_WaitResponse("OK", 5000))
		{
			printf("ESP8266_ATE_SET wait OK timeout\r\n");
			return 0;
		}
		printf("ESP8266_ATE_SET success\r\n");
		return 1;
}

/* ESP8266 UART interrupt callbacks ------------------------------------------*/

/**
  * @brief  USART3 ReceiveToIdle DMA 接收事件处理。
  * @param  huart: UART 句柄。
  * @param  Size: 本次接收到的数据长度。
  * @retval 无
  */
void ESP8266_UART_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if(huart == &huart3)
	{
		wifi_receive_update = true;

		//WIFI转移串口回显操作
		HAL_UART_Transmit_IT(&huart1,WIFI_RceBuffer,Size); 
		
		// 重新启动 ESP8266 DMA 接收
		HAL_UARTEx_ReceiveToIdle_DMA(&huart3,WIFI_RceBuffer,WIFI_MAX_INDEX);
	}
}


/**
  * @brief  USART3 错误回调处理，清除错误并重新启动 DMA 接收。
  * @param  huart: UART 句柄。
  * @retval 无
  */
void ESP8266_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart3)
    {
        // 清除帧错误标志
        __HAL_UART_CLEAR_FEFLAG(&huart3);

        // 重新启动 ESP8266 DMA 接收
        HAL_UARTEx_ReceiveToIdle_DMA(&huart3,WIFI_RceBuffer,WIFI_MAX_INDEX);
	    }
}
