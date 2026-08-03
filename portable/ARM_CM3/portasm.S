.syntax unified
.cpu cortex-m3
.thumb

.section .text.mini_port, "ax", %progbits
.align 2

.global mini_port_start_first_task
.type mini_port_start_first_task, %function
.thumb_func
mini_port_start_first_task:
    /*
     * r0 holds the first task's saved stack pointer. Exception entry does not
     * modify r0, so SVC_Handler can use it to locate the software stack frame.
     */

    /* Reclaim the main stack, as the scheduler will never return to main(). */
    ldr r1, =0xE000ED08
    ldr r1, [r1]
    ldr r1, [r1]
    msr msp, r1

    /* SVC is configurable-priority, so PRIMASK must not mask it. */
    cpsie i
    dsb
    isb
    svc #0

    /* SVC_Handler returns directly to the task, so this is never reached. */
.Lstart_first_task_unexpected_return:
    b .Lstart_first_task_unexpected_return

.size mini_port_start_first_task, .-mini_port_start_first_task

.global SVC_Handler
.type SVC_Handler, %function
.thumb_func
SVC_Handler:
    /* Restore the registers saved by software. r0 then points at hardware frame. */
    ldmia r0!, {r4-r11}
    msr psp, r0

    /* Thread mode is privileged and uses PSP; Handler mode continues using MSP. */
    movs r0, #2
    msr control, r0
    isb

    /* Return to Thread mode using PSP. The CPU restores r0-r3, r12, lr, pc, xPSR. */
    ldr lr, =0xFFFFFFFD
    bx lr

.size SVC_Handler, .-SVC_Handler

.section .note.GNU-stack, "", %progbits
