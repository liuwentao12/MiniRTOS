#include "mini_port.h"

#define MINI_INITIAL_XPSR        0x01000000UL
#define MINI_START_ADDRESS_MASK  0xFFFFFFFEUL

static __attribute__((noreturn)) void mini_task_exit_error(void)
{
    /* A task function must never return because it has no caller to return to. */
    __asm volatile ("cpsid i" ::: "memory");

    while (1)
    {
    }
}

/*
 * Cortex-M3 task stack after initialisation (low address at the bottom):
 *
 *   r4-r11                         software-saved frame
 *   r0-r3, r12, lr, pc, xPSR      hardware exception frame
 *
 * SVC restores r4-r11. Exception return restores the hardware frame.
 */
uint32_t *mini_port_init_stack(uint32_t *stack_top, MiniTaskFunction_t entry, void *argument)
{
    uint32_t *stack_pointer = stack_top;

    *(--stack_pointer) = MINI_INITIAL_XPSR; /* xPSR: Thumb state. */
    *(--stack_pointer) = ((uint32_t)(uintptr_t)entry) & MINI_START_ADDRESS_MASK; /* PC. */
    *(--stack_pointer) = (uint32_t)(uintptr_t)mini_task_exit_error; /* LR. */
    *(--stack_pointer) = 0U; /* R12. */
    *(--stack_pointer) = 0U; /* R3. */
    *(--stack_pointer) = 0U; /* R2. */
    *(--stack_pointer) = 0U; /* R1. */
    *(--stack_pointer) = (uint32_t)(uintptr_t)argument; /* R0. */

    /* R11 through R4 are restored by the SVC/PendSV software handler. */
    *(--stack_pointer) = 0U; /* R11. */
    *(--stack_pointer) = 0U; /* R10. */
    *(--stack_pointer) = 0U; /* R9. */
    *(--stack_pointer) = 0U; /* R8. */
    *(--stack_pointer) = 0U; /* R7. */
    *(--stack_pointer) = 0U; /* R6. */
    *(--stack_pointer) = 0U; /* R5. */
    *(--stack_pointer) = 0U; /* R4. */

    return stack_pointer;
}
