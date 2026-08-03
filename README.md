# MiniRTOS

MiniRTOS 是一个用于学习 Cortex-M 和 RTOS 内核原理的最小实现。项目会参考
FreeRTOS 的设计思路，逐步复刻任务栈、调度器、上下文切换、延时和任务间通信，
但不会直接复制 FreeRTOS 源码。

> 这是学习项目，不是可用于生产环境的实时操作系统。目前只实现了启动第一个
> 任务，还没有实现运行时任务切换。

## 当前进度

已经实现：

- 双向链表及按值插入、队尾插入、删除等基本操作；
- 简单的多优先级就绪列表；
- TCB、任务入口和任务参数的初始化；
- Cortex-M3 初始任务栈帧；
- 使用 SVC 和异常返回启动第一个任务；
- STM32F103 示例工程，已完成实板烧录验证，PC13 按预期每 500 ms 翻转。

尚未实现：

- PendSV 上下文切换；
- SysTick 调度节拍；
- 时间片和同优先级轮转；
- 阻塞/延时列表；
- 临界区和中断优先级管理；
- 空闲任务、任务删除、队列、信号量等功能。

## 目录结构

```text
MiniRTOS/
├── include/                 公共头文件
├── kernel/                  与 CPU 无关的内核代码
│   ├── mini_list.c
│   ├── mini_scheduler.c
│   └── mini_task.c
├── portable/
│   ├── ARM_CM3/             Cortex-M3 移植层
│   │   ├── port.c           初始任务栈构造
│   │   └── portasm.S        SVC 和首任务启动
│   └── heap/                内存管理实验代码
├── example/MiniRTOS_F103/   STM32F103 + HAL 示例工程
├── docs/                    学习笔记
└── tests/                   主机侧单元测试代码
```

第一次阅读时不需要逐个文件通读。建议先跟着
[首任务启动学习指南](docs/first-task-start.md)，沿着实际执行路径理解
`main → 初始栈帧 → SVC → task_a`，其余文件暂时可以跳过。

## 首任务是怎样启动的

复位后，`main()` 运行在线程模式并使用 MSP。调度器选出第一个任务后，执行流程为：

```text
main（Thread + MSP）
        |
        | mini_port_start_first_task(task_sp)
        v
      SVC #0
        |
        v
SVC_Handler（Handler + MSP）
        | 恢复 r4-r11
        | 设置 PSP
        | CONTROL.SPSEL = 1
        | EXC_RETURN = 0xFFFFFFFD
        v
task_a（Thread + PSP）
```

进入 SVC 前，移植层会从向量表恢复初始 MSP。调度器不会返回 `main()`，因此这样可以
回收启动过程和 `main()` 调用链占用的主栈空间，这与 FreeRTOS Cortex-M3 移植层的
处理思路一致。

Cortex-M3 在异常返回时会自动恢复 `r0-r3、r12、lr、pc、xPSR`。因此初始化任务时，
需要提前在任务栈中伪造一个硬件异常栈帧；`r4-r11` 不由硬件自动保存，交给 RTOS
移植层保存和恢复。

初始栈帧布局如下，栈向低地址增长：

```text
高地址（传给 mini_tcb_init 的 stack_top）
+------------------+
| xPSR             | Thumb 状态位为 1
| PC               | 任务入口
| LR               | 任务异常返回保护函数
| R12              |
| R3               |
| R2               |
| R1               |
| R0               | 任务参数；异常返回前 PSP 指向这里
+------------------+ 硬件异常栈帧
| R11              |
| R10              |
| R9               |
| R8               |
| R7               |
| R6               |
| R5               |
| R4               | TCB 保存的 stack_pointer 指向这里
+------------------+ 软件保存区
低地址
```

### MSP 与 PSP

| 栈指针 | 当前用途 |
| --- | --- |
| MSP | 启动代码、`main()` 和所有异常/中断处理函数 |
| PSP | RTOS 任务在线程模式下的独立任务栈 |

中断始终使用 MSP，任务使用 PSP，可以避免中断处理过程占用或破坏某个任务的栈。

## 构建 STM32F103 示例

需要安装：

- CMake 3.22 或更高版本；
- Ninja；
- GNU Arm Embedded Toolchain，且 `arm-none-eabi-gcc` 位于 `PATH`。

```bash
cd example/MiniRTOS_F103
cmake --preset Debug
cmake --build --preset Debug
```

生成文件位于：

```text
example/MiniRTOS_F103/build/Debug/MiniRTOS_F103.elf
```

当前版本已经完成 STM32F103 实板烧录验证：第一个任务可以正常运行，并每 500 ms
翻转一次 PC13。当前示例只有一个任务，因此这个现象验证了任务初始栈和首任务启动
路径，但还不能验证抢占或上下文切换。

## 与 FreeRTOS 概念的对应关系

| MiniRTOS | FreeRTOS 中的相近概念 | 作用 |
| --- | --- | --- |
| `MiniTCB_t` | `TCB_t` | 保存任务栈顶、优先级和状态链表节点 |
| `mini_port_init_stack()` | `pxPortInitialiseStack()` | 构造任务初始上下文 |
| `mini_port_start_first_task()` | `prvPortStartFirstTask()` | 触发 SVC，启动首任务 |
| `SVC_Handler` | `vPortSVCHandler()` | 恢复首任务上下文 |
| `ready_lists` | `pxReadyTasksLists` | 按优先级保存就绪任务 |

这里只做概念对应，两边的数据结构和接口目前并不兼容。

## 重要约定

- Cortex-M 异常入口要求栈正确对齐。示例任务栈使用 `aligned(8)` 保证 8 字节对齐；
- 传入 `mini_tcb_init()` 的 `stack_top` 是栈数组末尾之后的位置，例如
  `&task_stack[128]`；
- 任务函数不能返回。若任务意外返回，会进入 `mini_task_exit_error()` 并关闭中断；
- `SVC_Handler` 由 `portable/ARM_CM3/portasm.S` 实现。若重新使用 STM32CubeMX
  生成代码，需要确保 `stm32f1xx_it.c` 中没有第二个同名实现；
- 当前移植层仅针对无 FPU 的 Cortex-M3，不包含 Cortex-M4F/M7 的浮点上下文；
- 当前 SVC 只用于启动首任务，还没有实现 SVC 号分发。

## 推荐的后续学习顺序

1. 增加“当前任务”指针，并规定 TCB 的栈顶指针字段布局；
2. 实现 PendSV：保存旧任务 PSP 和 `r4-r11`，选择新任务，再恢复上下文；
3. 将 PendSV 设置为最低异常优先级；
4. 接入 SysTick，只在 tick 中请求 PendSV，不直接执行任务切换；
5. 实现同优先级轮转和延时列表；
6. 使用 BASEPRI 实现适合 Cortex-M3 的内核临界区；
7. 最后再加入空闲任务、动态内存、队列和信号量。

示例工程目前使用 STM32 HAL 的 SysTick 维护 `HAL_Delay()`。接入内核 tick 时，需要
统一 HAL tick 与调度器 tick 的职责，避免同时定义两个 `SysTick_Handler`。

## 项目定位

MiniRTOS 的重点是让每一步上下文切换都可以阅读、反汇编和单步调试。它不是
FreeRTOS 的替代品，也不隶属于 FreeRTOS 项目。需要稳定性、完整功能或安全认证时，
请使用成熟 RTOS。
