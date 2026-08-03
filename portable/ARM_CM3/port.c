#include "mini_port.h"

#define MINI_INITIAL_XPSR 0x01000000UL
#define MINI_START_ADDRESS_MASK 0xFFFFFFFEUL

static void mini_task_exit_error(void)
{
    while (1)
    {
        
    }
    
}

/*
R0：任务函数第一个参数
R1～R3、R12：临时数据
R4～R11：任务切换时由 RTOS 手动保存
LR：函数返回地址
PC：下一条执行指令的地址
xPSR：CPU 运行状态
*/
uint32_t *mini_port_init_stack(uint32_t *stack_top, MiniTaskFunction_t entry, void *argument)
{
    uint32_t *stack_pointer = stack_top;

    stack_pointer--;
    *stack_pointer = MINI_INITIAL_XPSR;   /* xPSR */

    stack_pointer--;
    *stack_pointer =((uint32_t)(uintptr_t)entry) & MINI_START_ADDRESS_MASK; /* PC */

    stack_pointer--;
    *stack_pointer =(uint32_t)(uintptr_t)mini_task_exit_error; /* LR */

    stack_pointer--;
    *stack_pointer = 0U;                  /* R12 */

    stack_pointer--;
    *stack_pointer = 0U;                  /* R3 */

    stack_pointer--;
    *stack_pointer = 0U;                  /* R2 */

    stack_pointer--;
    *stack_pointer = 0U;                  /* R1 */

    stack_pointer--;
    *stack_pointer = (uint32_t)(uintptr_t)argument; /* R0 */

    stack_pointer--;
    *stack_pointer = 0U; /* R11 */

    stack_pointer--;
    *stack_pointer = 0U; /* R10 */

    stack_pointer--;
    *stack_pointer = 0U; /* R9 */

    stack_pointer--;
    *stack_pointer = 0U; /* R8 */

    stack_pointer--;
    *stack_pointer = 0U; /* R7 */

    stack_pointer--;
    *stack_pointer = 0U; /* R6 */

    stack_pointer--;
    *stack_pointer = 0U; /* R5 */

    stack_pointer--;
    *stack_pointer = 0U; /* R4 */

    return stack_pointer;
}
