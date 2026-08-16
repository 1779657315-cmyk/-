# CLAUDE.md

此文件为 Claude Code (claude.ai/code) 在本仓库中工作时提供指导。

## 概述

STM32F407ZGTx 固件项目,由 STM32CubeMX 6.15.0 生成,使用 STM32 HAL(`FW_F4 V1.28.3`)和 FreeRTOS(CMSIS-RTOS v2 API,`heap_4.c`)。工具链为 Keil MDK-ARM(uVision)V5.32。这是一个学习/实验项目:主要目的是通过 UART 驱动 ESP8266,并练习 FreeRTOS 的各种原语。

## 构建

**没有命令行构建、lint 或测试工具**——没有 Makefile、CMake 或 CI。构建通过 Windows 上的 Keil uVision IDE 完成:

- 在 Keil uVision 中打开 `MDK-ARM/F407_HAL_LED.uvprojx`,然后在那里编译/烧录。
- 源文件集合和包含路径由 `.uvprojx` 文件定义,并镜像在 `.mxproject`(`[PreviousUsedKeilFiles]`)中。项目通过 STM32CubeMX 中的 `F407_HAL_LED.ioc` 编辑;从 CubeMX 重新生成会重写生成出的源文件。

行尾符很重要:`.gitattributes`/`.editorconfig` 对 `Core/**/*.{c,h}`、`*.ioc`、`.mxproject` 和 `MDK-ARM/*.{uvprojx,uvoptx}` 强制使用 CRLF。

## 架构

### STM32CubeMX 重新生成模型(关键)

`Core/` 下的大多数文件是 CubeMX **生成**的。当你从 `.ioc` 重新生成时,CubeMX 会覆盖它们,只保留成对的 `/* USER CODE BEGIN x */` / `/* USER CODE END x */` 标记之间的代码。绝不要在生成文件里、标记之外添加逻辑,否则会丢失。

CubeMX **不**管理的手写文件(可自由编辑):

- `Core/Src/Init.c` / `Core/Inc/Init.h` —— 中心包含枢纽 + 初始化辅助
- `Core/Src/led.c` / `Core/Inc/led.h` —— RGB LED 控制
- `Core/Src/esp8266.c` / `Core/Inc/esp8266.h` —— ESP8266 AT 驱动 + 接收环形缓冲区
- `Core/Src/isr.c` / `Core/Inc/isr.h` —— 中断回调粘合层

### 包含结构

`Init.h` 是事实上的总包含头:它包含 `string.h`、`stdio.h`、`stdbool.h`,以及每个外设头文件(`gpio.h`、`tim.h`、`usart.h`、`dma.h`、`main.h`,还有 `led.h`/`esp8266.h`/`isr.h`)。用户模块包含 `Init.h`(或包含它的某个头)。`main.h` 和 `Init.h` 互相包含,只能靠包含保护来消解——新增头文件时要注意这一点。

`printf` 在 `Init.c` 中通过 `fputc()` 重定向到 `huart1`(USART1),所以所有 `printf` 输出都走 115200 波特率的调试串口。

### 外设 / 引脚映射

| 外设 | 引脚 | 作用 |
|------|------|------|
| USART1 | PA9 (TX)、PA10 (RX) | 调试/CH340 控制台,115200,RX 通过 `HAL_UARTEx_ReceiveToIdle_IT` 接收进 `CH340_RceBuffer[64]` 并回显 |
| USART3 | PB10 (TX)、PB11 (RX) | ESP8266,115200,RX 通过 `HAL_UARTEx_ReceiveToIdle_DMA`(DMA1_Stream1、Ch4) |
| RGB LED | PF6 (R)、PF7 (G)、PF8 (B) | 低电平点亮:写 `GPIO_PIN_RESET` 亮灯 |
| WIFI_ENABLE | PE2 | ESP8266 电源使能(输出) |
| WIFI_RST | PG15 | ESP8266 复位(输出) |
| SW1 | PA0 (EXTI0) | 按键 —— 点亮 LED |
| SW3 | PC13 (EXTI13) | 按键 —— 熄灭 LED |
| TIM6 | — | 1 ms 基准定时器(Prescaler 83、Period 999);启动目前被注释掉 |
| TIM7 | — | 用作 HAL `uwTick` 时基(`stm32f4xx_hal_timebase_tim.c`) |

时钟:HSE 25 MHz → PLL → 168 MHz SYSCLK(`PLLM=25、PLLN=336、PLLP=2`),APB1 /4,APB2 /2。

### ESP8266 驱动(`esp8266.c`)

基于 USART3 的分层 AT 指令客户端:

- **环形缓冲区**:2048 字节的 `WIFI_RceBuffer`,带 `volatile` 读/写索引。DMA 填充线性空闲区(`WIFI_Receive_DMA_Set()`),`ESP8266_UART_RxEventCallback()` 推进写索引并重启 DMA。`WIFI_Buffer_FindString()` 在环形缓冲区里扫描目标字符串;`ESP8266_WaitResponse()` 阻塞等待它。
- **AT 阶段**:`ESP8266_Reset` → `ESP8266_ATE_SET` → `ESP8266_wifi_connect` → `ESP8266_tcp_connect` → `ESP8266_transparent_mode_enter`。`WIFI_init()` 把它们串起来。WiFi/TCP 凭据是 `esp8266.h` 里的宏(`WIFI_SSID`、`WIFI_PASSWORD`、`TCP_HOST`、`TCP_PORT`)。
- `WIFI_init()` 目前在 `main.c` 里**被注释掉**(在 `USER CODE BEGIN 2` 块中),所以默认不运行 ESP8266 初始化序列。

### 中断分发(`isr.c`)

HAL 回调的统一分发点:

- `HAL_GPIO_EXTI_Callback` —— SW1 → `GPIO_LED_OPEN()`,SW3 → `GPIO_LED_CLOSE()`。
- `HAL_UARTEx_RxEventCallback` —— 路由 `huart1`(回显 + 重启 IT RX)或 `huart3`(委托给 `ESP8266_UART_RxEventCallback`)。
- `HAL_UART_ErrorCallback` —— 委托给 `ESP8266_UART_ErrorCallback`(清 FE 标志、重启 DMA RX)。

### FreeRTOS(`freertos.c`)

三个任务加全套同步原语,全部通过 CMSIS-RTOS v2(`osThreadNew`、`osThreadFlagsWait/Set` 等):

- `LED_open_close`(高)—— 等待 `FLAG1` 线程标志,然后打印。
- `usart1_show`(普通)—— 周期性 `printf`。
- `myTask03`(低)—— 给 `LED_open_closeHandle` 设置 `FLAG1`。
- 另外创建:`myMutex01`、二进制信号量、计数信号量、一个 1 s 周期软件定时器(`Callback01`)、一个 16 槽 `uint16_t` 消息队列,以及一个事件标志对象。

`FreeRTOSConfig.h` 将 `configTOTAL_HEAP_SIZE` 设为 15360 字节。

## Git 注意事项

`MDK-ARM/` 的构建产物(`*.o`、`*.crf`、`*.axf`、`*.map`、`*.hex` 等)被 git 跟踪,每次编译都会显示为已修改——没有 `.gitignore` 排除它们。本仓库位于更大的 `learning_materials/` 工作区中;邻近有一个未跟踪的 `../liarbry/RTOS/` 目录和其他学习资料,位于本项目之外。
