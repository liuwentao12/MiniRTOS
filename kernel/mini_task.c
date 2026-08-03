#include "mini_task.h"

void mini_tcb_init(MiniTCB_t *tcb, uint32_t *stack_top, MiniTaskFunction_t entry, void *argument, uint32_t priority)
{
    tcb->stack_pointer = mini_port_init_stack(stack_top, entry, argument);
    
    tcb->priority = priority;

    mini_list_item_init(&tcb->state_item);
    tcb->state_item.owner = tcb;
}