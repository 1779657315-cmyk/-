#include "esp8266.h"
#include <stdlib.h>

/* Private variables ---------------------------------------------------------*/
uint8_t WIFI_RceBuffer[WIFI_MAX_INDEX];   //WIFI接收缓冲区
volatile bool wifi_receive_update = false;
volatile uint16_t wifi_rx_read_index = 0;
volatile uint16_t wifi_rx_write_index = 0;
volatile bool wifi_rx_overflow = false;

static uint16_t WIFI_Buffer_Available(void);
static uint8_t WIFI_Buffer_FindString(const char *target);
static void WIFI_Buffer_DiscardAll(void);

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

	// 丢弃旧回包，防止残留数据影响后续检测
	WIFI_Buffer_DiscardAll();
	
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
			WIFI_Buffer_DiscardAll();
			return 0;
		}
	}

	wifi_receive_update = false;
	tick_start = HAL_GetTick();

	while(HAL_GetTick() - tick_start <= timeout_ms)
	{
		if(WIFI_Buffer_FindString(target))
		{
			WIFI_Buffer_DiscardAll();
			return 1;
		}

		if(wifi_receive_update)
		{
			wifi_receive_update = false;
			tick_start = HAL_GetTick();
		}
	}

	WIFI_Buffer_DiscardAll();
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
		//HAL_UART_Transmit_IT(&huart1,WIFI_RceBuffer,Size);
		
		// 重新启动 ESP8266 DMA 接收
		WIFI_Receive_DMA_Set(Size);
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
        WIFI_Receive_DMA_Set(0);
	}
}

/* ESP8266 receive ring buffer -----------------------------------------------*/

/**
  * @brief  获取 WIFI 接收环形缓冲区当前可读数据长度。
  * @retval 当前可读字节数。
  */
static uint16_t WIFI_Buffer_Available(void)
{
	if(wifi_rx_write_index >= wifi_rx_read_index)
	{
		return wifi_rx_write_index - wifi_rx_read_index;
	}
	else
	{
		return WIFI_MAX_INDEX - wifi_rx_read_index + wifi_rx_write_index;
	}
}

/**
  * @brief  在 WIFI 接收环形缓冲区中查找目标字符串。
  * @param  target: 需要查找的目标字符串。
  * @retval 0 表示未找到，1 表示找到。
  */
static uint8_t WIFI_Buffer_FindString(const char *target)
{
	uint16_t available;
	uint16_t target_len;
	uint16_t i;
	uint16_t j;
	uint16_t index;

	if(target == NULL)
	{
		return 0;
	}

	target_len = strlen(target);
	if(target_len == 0)
	{
		return 1;
	}

	available = WIFI_Buffer_Available();
	if(available < target_len)
	{
		return 0;
	}

	for(i = 0; i <= available - target_len; i++)
	{
		for(j = 0; j < target_len; j++)
		{
			index = wifi_rx_read_index + i + j;
			if(index >= WIFI_MAX_INDEX)
			{
				index -= WIFI_MAX_INDEX;
			}

			if(WIFI_RceBuffer[index] != (uint8_t)target[j])
			{
				break;
			}
		}

		if(j == target_len)
		{
			return 1;
		}
	}

	return 0;
}

/**
  * @brief  丢弃 WIFI 接收环形缓冲区中当前已接收但未消费的数据。
  * @retval 无
  */
static void WIFI_Buffer_DiscardAll(void)
{
	wifi_rx_read_index = wifi_rx_write_index;
	wifi_receive_update = false;
}

/**
  * @brief  获取从当前写指针开始的连续可写空间。
  * @note   DMA 只能写连续内存，因此这里返回的不是总剩余空间，
  *         而是本次 DMA 可以安全写入的连续长度。
  * @retval 当前连续可写字节数；0 表示缓冲区已满或无安全连续空间。
  */
static uint16_t WIFI_Buffer_LinearFree(void)
{
	if(wifi_rx_write_index >= wifi_rx_read_index)
	{
		if(wifi_rx_read_index == 0)
		{
			return WIFI_MAX_INDEX - wifi_rx_write_index - 1;
		}
		else
		{
			return WIFI_MAX_INDEX - wifi_rx_write_index;
		}
	}
	else
	{
		return wifi_rx_read_index - wifi_rx_write_index - 1;
	}
}

/**
  * @brief  更新 WIFI 接收写指针，并重新启动 USART3 ReceiveToIdle DMA。
  * @param  Size: 本次 DMA 接收事件收到的数据长度；首次启动时传 0。
  * @retval 0 表示失败，1 表示成功。
  */
uint8_t WIFI_Receive_DMA_Set(uint16_t Size)
{
	uint16_t remain_index;
	uint32_t next_write_index;

	next_write_index = wifi_rx_write_index + Size;
	if(next_write_index >= WIFI_MAX_INDEX)
	{
		next_write_index -= WIFI_MAX_INDEX;
	}
	wifi_rx_write_index = (uint16_t)next_write_index;

	remain_index = WIFI_Buffer_LinearFree();
	if(remain_index == 0)
	{
		wifi_rx_overflow = true;
		return 0;
	}

	//重启DMA中断接收
	if(HAL_UARTEx_ReceiveToIdle_DMA(&huart3,&WIFI_RceBuffer[wifi_rx_write_index],remain_index) != HAL_OK)
	{
		return 0;
	}
	//仅空闲/满可触发中断
	__HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT);
	return 1;
}

/**
  * @brief  从 WIFI 接收环形缓冲区读取指定长度数据。
  * @param  read_length: 需要读取的数据长度，单位字节。
  * @retval 0 表示失败，1 表示成功。
  */
uint8_t WIFI_Buffer_read(uint16_t read_length)
{
	uint8_t *read_buffer;

	if(read_length == 0)
	{
		return 1;
	}

	if(read_length > WIFI_Buffer_Available())
	{
		return 0;
	}

	read_buffer = malloc(read_length);
	if(read_buffer == NULL)
	{
		return 0;
	}

	if(wifi_rx_read_index + read_length > WIFI_MAX_INDEX)  // 溢出场景
	{
		uint16_t second_len = wifi_rx_read_index + read_length - WIFI_MAX_INDEX; //从头开始要读的长度
		uint16_t first_len = WIFI_MAX_INDEX - wifi_rx_read_index;

		memcpy(read_buffer,&WIFI_RceBuffer[wifi_rx_read_index],first_len);
		memcpy(read_buffer + first_len,WIFI_RceBuffer,second_len);
		wifi_rx_read_index = second_len;
	}
	else //不溢出
	{
		memcpy(read_buffer,&WIFI_RceBuffer[wifi_rx_read_index],read_length);
		wifi_rx_read_index += read_length;
		if(wifi_rx_read_index == WIFI_MAX_INDEX)
		{
			wifi_rx_read_index = 0;
		}
	}
	/*
	command_get()  通过read_buffer获取功能码
	*/
	free(read_buffer);
	return 1;
}
