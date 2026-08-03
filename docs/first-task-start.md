# 首任务启动学习指南

这份笔记只回答一个问题：CPU 怎样从 `main()` 进入 `task_a()`？

先不要通读整个项目，也不用先理解完整 FreeRTOS。当前阶段只沿着下面五个节点学习：

```text
创建任务
   ↓
构造初始栈帧
   ↓
选择首任务
   ↓
触发并处理 SVC
   ↓
异常返回到 task_a
```

学完的判断标准不是“记住每条汇编”，而是你能解释：

1. 为什么任务第一次运行前，栈里已经有 `PC` 和 `xPSR`；
2. 为什么 `r4-r11` 由软件恢复，其余寄存器由 CPU 恢复；
3. 为什么任务使用 PSP，而中断使用 MSP；
4. 为什么 `0xFFFFFFFD` 不是普通函数地址。

## 第一站：只看任务长什么样

先看以下位置：

- `example/MiniRTOS_F103/Core/Src/main.c` 中的 `task_a()`；
- 同文件中的 `task_a_tcb` 和 `task_a_stack`；
- `include/mini_task.h` 中的 `MiniTCB_t`。

此时只建立三个概念：

- `task_a()` 是将来要执行的普通 C 函数；
- `task_a_stack` 是这个任务自己的栈；
- `task_a_tcb.stack_pointer` 记录任务暂停或尚未启动时的栈顶位置。

示例栈包含 128 个 `uint32_t`，共 512 字节。Cortex-M 栈向低地址增长，所以传给
`mini_tcb_init()` 的是数组末尾之后的 `&task_a_stack[128]`，而不是数组开头。

这一站暂时不看汇编。

## 第二站：观察伪造出来的初始现场

接下来只看：

- `kernel/mini_task.c` 中的 `mini_tcb_init()`；
- `portable/ARM_CM3/port.c` 中的 `mini_port_init_stack()`。

新任务从来没有真正运行过，因此不存在可以“恢复”的历史寄存器。RTOS 的做法是
提前在栈中摆好一份假的现场，让 CPU 误以为这个任务只是刚从异常中返回。

在当前示例中，初始化完成后应有：

```text
task_a_tcb.stack_pointer == &task_a_stack[112]
```

因为初始现场共占 16 个 32 位字：

| 相对 `stack_pointer` 的下标 | 内容 | 恢复者 |
| --- | --- | --- |
| 0～7 | `r4-r11` | SVC 汇编代码 |
| 8～12 | `r0-r3、r12` | Cortex-M3 硬件 |
| 13 | `lr` | Cortex-M3 硬件 |
| 14 | `pc`，即 `task_a` 地址 | Cortex-M3 硬件 |
| 15 | `xPSR = 0x01000000` | Cortex-M3 硬件 |

这里最值得理解的是 `r0`：ARM 函数调用约定使用 `r0` 传递第一个参数，所以栈帧中
的初始 `r0` 就是传给任务函数的 `argument`。

### 建议的第一次断点

在 `main.c` 的 `mini_tcb_init()` 下一行设置断点，然后观察：

```text
task_a_tcb.stack_pointer
&task_a_stack[112]
task_a_stack[112] 到 task_a_stack[127]
```

重点确认：

- 两个地址相同；
- 最后一个字是 `0x01000000`；
- 倒数第二个字是 `task_a` 的偶数地址；
- `task_a_stack` 和栈顶地址都是 8 字节对齐的。

任务入口地址最低位被清零不是丢失了 Thumb 状态。异常返回使用 `xPSR.T` 恢复
Thumb 状态，这正是 `0x01000000` 的作用。

## 第三站：调度器当前只做“选一个”

然后看：

- `kernel/mini_scheduler.c` 中的 `mini_scheduler_add_ready()`；
- `mini_scheduler_select_next()`；
- `main.c` 中取得 `selected_task` 的几行代码。

当前调度器会从最高优先级的非空就绪列表中取出一个 TCB。它现在不会进行轮转，
也不会在任务运行期间重新选择任务。

所以此时应准确地把项目状态描述成：

> 能选择并启动首任务，但还不能进行上下文切换。

不需要在这一阶段研究 PendSV、时间片或任务延时。

## 第四站：沿着汇编走一遍

现在再打开 `portable/ARM_CM3/portasm.S`，只分成两段看。

### `mini_port_start_first_task()`

进入函数时：

```text
r0 = selected_task->stack_pointer
CPU = Thread mode
当前栈 = MSP
```

这段代码先从 VTOR 找到向量表，再从向量表第一个字恢复复位时的初始 MSP。原因是
启动成功后不会再返回 `main()`，可以把启动阶段占用的主栈全部回收。

随后执行 `svc #0`。SVC 是一次同步异常，CPU 会进入 Handler mode，并从向量表跳转
到 `SVC_Handler`。异常入口不会改变 `r0` 的最终可见值，因此处理函数仍能从 `r0`
拿到任务栈指针。

### `SVC_Handler`

处理函数首先执行：

```asm
ldmia r0!, {r4-r11}
```

它从任务栈恢复 8 个软件保存寄存器。写回符号 `!` 会让 `r0` 增加 32 字节，所以
这条指令之后：

```text
r0 = &task_a_stack[120]
```

这里正是硬件异常栈帧的起点，也就是初始 `r0` 所在的位置。随后把这个地址写入
PSP，并设置线程模式使用 PSP。

最后：

```asm
ldr lr, =0xFFFFFFFD
bx  lr
```

`0xFFFFFFFD` 是 Cortex-M 的 EXC_RETURN 特殊值，含义是：

- 返回 Thread mode；
- 返回后使用 PSP；
- 恢复普通的非浮点硬件栈帧。

CPU 识别到这个特殊值后，不会跳到地址 `0xFFFFFFFC`，而是从 PSP 自动弹出
`r0-r3、r12、lr、pc、xPSR`。恢复完成后：

```text
PC      = task_a
R0      = argument
PSP     = &task_a_stack[128]
CONTROL = 2
IPSR    = 0（Thread mode）
```

这就是任务第一次开始执行的瞬间。

## 第五站：用调试器证明执行链

建议按顺序设置四个断点，不要一开始就在汇编中逐条乱跳：

1. `mini_tcb_init()` 返回后的下一行；
2. `mini_port_start_first_task`；
3. `SVC_Handler`；
4. `task_a` 第一行。

每站只记录下面这些值：

| 位置 | 应重点观察 |
| --- | --- |
| 初始化任务后 | `task_a_tcb.stack_pointer` 和 16 个栈字 |
| 启动函数入口 | `r0`、MSP、PSP |
| SVC 入口 | `IPSR = 11`、`r0`、MSP |
| `ldmia` 之后 | `r0` 增加 32、`r4-r11` 被恢复 |
| `task_a` 入口 | `IPSR = 0`、PSP 到达原栈顶、PC 为任务入口 |

执行 `msr msp` 后，IDE 显示的 C 调用栈可能突然不完整。这不是程序崩溃，而是我们
主动丢弃了 `main()` 的旧栈帧。此时以寄存器、反汇编和断点为准。

## 三个小实验

完成上面的单步验证后，可以用小改动巩固理解。

### 实验一：验证 R0 传参

创建一个 `uint32_t blink_period = 200U`，把 `&blink_period` 作为任务参数传入，在
`task_a()` 中读取它。观察初始栈帧下标 8、任务入口的 `r0` 和 `argument` 是否一致。

### 实验二：观察软件恢复寄存器

调试时临时把初始 `r4-r11` 填成不同的明显数值，例如 `0x44444444`、
`0x55555555`。在 `ldmia` 前后观察寄存器窗口。实验完成后恢复为 0。

### 实验三：验证任务不能返回

临时让一个测试任务直接返回，观察它进入 `mini_task_exit_error()`。这说明初始栈帧
中的 LR 确实被 CPU 恢复了。完成后恢复无限循环，正式任务不能返回。

## 暂时不用学的文件

理解首任务启动时，可以先跳过：

- STM32 HAL 驱动目录；
- 链接脚本的大部分内容；
- `portable/heap/`；
- 空的 PendSV 处理函数；
- CMake 的绝大多数配置。

你只需要知道启动文件的向量表会把 SVC 指向 `SVC_Handler`，而 CMake 会把
`portasm.S` 编译和链接进固件。

## 进入 PendSV 前的自测

如果不看代码也能回答下面问题，就可以进入下一阶段：

1. `task_a_tcb.stack_pointer` 为什么初始指向数组下标 112？
2. `ldmia r0!, {r4-r11}` 后，`r0` 为什么刚好指向硬件栈帧？
3. 为什么不能把 PSP 留在 `r4` 所在地址后直接跳转到任务？
4. SVC 处理函数执行时使用 MSP 还是 PSP？
5. `bx lr` 为什么能恢复八个寄存器并进入 `task_a()`？

下一阶段实现 PendSV 时，本质上只是把这条单向恢复链扩展成：

```text
保存当前任务 → 选择下一个任务 → 恢复下一个任务
```

首任务启动理解透以后，PendSV 就不会再是一整段陌生汇编。
