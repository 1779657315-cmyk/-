`QueueHandle_t` 就是 **FreeRTOS 队列句柄类型**
即：一个指向队列控制块 `Queue_t` 的指针。

```c
QueueHandle_t  mcx_Queue;          //等价于 `Queue_t` * mcx_Queue;
mcx_Queue = xQueueCreate(5, sizeof(int));
```

