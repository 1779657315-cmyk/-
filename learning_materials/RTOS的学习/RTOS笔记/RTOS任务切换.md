
任务切换本质：
1.保存老任务执行现场
2.恢复新任务执行现场

## 1.任务现场介绍

以STM32F407（Cortex-M4F）+ FreeRTOS为例：

每个任务都有一个独立的任务栈，用于记录：

1. 正常程序运行需要的栈数据
	1. 局部变量
	2. 函数参数
	3. 函数返回地址
	4. 临时数据
	5. 函数调用时需要保护的寄存器
2. 任务切换时保存的 CPU 上下文
	1. R0-R12
	2. LR
	3. PC
	4. xPSR

| 寄存器              | 作用             | 谁保存                 | 保存到哪                      |
| ---------------- | -------------- | ------------------- | ------------------------- |
| `通用寄存器R0-R3`     | 函数参数、临时数据      | CPU 自动              | 当前任务自己的任务栈                |
| `通用寄存器R4-R11`    | 通用寄存器，需跨函数保持   | ***FreeRTOS 手动***   | 当前任务自己的任务栈                |
| `临时寄存器R12`       | 临时寄存器          | CPU 自动              | 当前任务自己的任务栈                |
| `返回地址寄存器LR`      | 返回地址相关         | CPU 自动              | 当前任务自己的任务栈                |
| `程序计数器PC`        | 记录任务执行到哪条指令    | CPU 自动              | 当前任务自己的任务栈                |
| `程序状态寄存器xPSR`    | CPU 状态信息       | CPU 自动              | 当前任务自己的任务栈                |
| ***`任务栈指针PSP`*** | ***指向当前任务栈顶*** | ***FreeRTOS 手动记录*** | ***`TCB->pxTopOfStack`*** |

![[栈帧布局.png]]
	这些合起来构成任务最核心的 **CPU 上下文**。

## 2.任务切换流程

**FREERTOS手动实现：**
1. 保存现场：
	1. 将寄存器r4-r11压入栈中
	2. 将PSP任务栈指针保存到TCB中
2. 还原现场：
	1. 将TCB中的PSP取出
	2. 将r4-r11弹栈

```
进入 PendSV
    │
    ↓
CPU自动保存一部分现场
    │
    ↓
xPortPendSVHandler()
    │
    ├── ① 保存当前任务剩余现场
    │
    ├── ② 保存当前任务PSP到TCB
    │
    ├── ③ vTaskSwitchContext()
    │       ↓
    │    切换 pxCurrentTCB
    │
    ├── ④ 从新任务TCB取栈顶
    │
    ├── ⑤ 恢复新任务现场
    │
    └── ⑥ 异常返回
            ↓
        CPU自动恢复剩余现场
```

## 3.源码实现
该部分操作均为汇编语言实现，原因如下：
***==为了精确控制 CPU 寄存器和栈，避免编译器自动生成代码破坏任务现场。==***

汇编源码：
```asm
void xPortPendSVHandler(void)
{
    __asm volatile
    (
        /* =========================
           保存 TaskA
           ========================= */

        "mrs r0, psp                 \n"  // r0 = TaskA PSP

        "ldr r3, =pxCurrentTCB       \n"
        "ldr r2, [r3]                \n"  // r2 = TaskA TCB

        "stmdb r0!, {r4-r11, r14}    \n"  // 保存TaskA寄存器

        "str r0, [r2]                \n"  // TaskA TCB->pxTopOfStack = r0


        /* =========================
           调度
           ========================= */

        "bl vTaskSwitchContext       \n"  // pxCurrentTCB: A → B


        /* =========================
           恢复 TaskB
           ========================= */

        "ldr r1, [r3]                \n"  // r1 = TaskB TCB
        "ldr r0, [r1]                \n"  // r0 = TaskB pxTopOfStack

        "ldmia r0!, {r4-r11, r14}    \n"  // 恢复TaskB寄存器

        "msr psp, r0                 \n"  // 恢复TaskB PSP

        "bx r14                      \n"  // 异常返回
                                        // CPU自动恢复剩余寄存器
    );
}
```

## 4.任务管理块TCB

```c
typedef struct tskTaskControlBlock
{
    volatile StackType_t *pxTopOfStack;

    ListItem_t xStateListItem;
    ListItem_t xEventListItem;

    UBaseType_t uxPriority;

    StackType_t *pxStack;

    char pcTaskName[ configMAX_TASK_NAME_LEN ];

    ...
} tskTCB;

typedef tskTCB TCB_t;
```

以下成员为核心成员：

| 成员               | 作用                 |
| ---------------- | ------------------ |
| `pxTopOfStack`   | 保存当前任务栈顶位置         |
| `xStateListItem` | 把任务挂进就绪/阻塞/挂起等状态链表 |
| `xEventListItem` | 把任务挂进事件等待链表        |
| `uxPriority`     | 任务优先级              |
| `pxStack`        | 指向任务栈的起始位置         |
| `pcTaskName`     | 任务名称               |
|                  |                    |
内核

## 5.任务切换与链表
链表存在本质：

任务创建
   ↓
创建 TCB + 任务栈
   ↓
TCB 内嵌 ListItem_t
   ↓
ListItem_t 挂入 ReadyList / DelayList 等链表
   ↓
链表通过 pxIndex 找到目标 ListItem_t
   ↓
ListItem_t.pvOwner 找到对应 TCB
   ↓
pxCurrentTCB 切换到目标 TCB
   ↓
TCB->pxTopOfStack 找到目标任务现场
   ↓
恢复 PSP + 寄存器
   ↓
目标任务继续运行