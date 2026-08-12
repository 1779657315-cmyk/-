#ifndef __ESP8266_H__
#define __ESP8266_H__

/* Includes ------------------------------------------------------------------*/
#include "Init.h"

/* Config --------------------------------------------------------------------*/
#define WIFI_SSID  			"205"
#define WIFI_PASSWORD   "Knight17"
#define TCP_HOST       	"192.168.66.100"
#define TCP_PORT        "8080"
#define WIFI_MAX_INDEX  2048 //WIFI缓冲区最大长度

/* Buffer --------------------------------------------------------------------*/
extern uint8_t WIFI_RceBuffer[WIFI_MAX_INDEX];
extern volatile uint16_t wifi_rx_read_index;
extern volatile uint16_t wifi_rx_write_index;

/* Application layer ---------------------------------------------------------*/
void WIFI_init(void);

/* ESP8266 receive ring buffer -----------------------------------------------*/
uint8_t WIFI_Receive_DMA_Set(uint16_t Size);
uint8_t WIFI_Buffer_read(uint16_t read_length);

/* ESP8266 AT stage ----------------------------------------------------------*/
uint8_t ESP8266_Reset(void);
uint8_t ESP8266_ATE_SET(bool model);
uint8_t ESP8266_wifi_connect(void);
uint8_t ESP8266_tcp_connect(void);
uint8_t ESP8266_transparent_mode_enter(void);

/* ESP8266 low-level transport -----------------------------------------------*/
uint8_t ESP8266_AT_Send(const char *cmd);
uint8_t ESP8266_WaitResponse(const char *target, uint32_t timeout_ms);
uint8_t ESP8266_SendBytes(const uint8_t *data, uint16_t len);

/* ESP8266 UART interrupt callbacks ------------------------------------------*/
void ESP8266_UART_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);
void ESP8266_UART_ErrorCallback(UART_HandleTypeDef *huart);

#endif
