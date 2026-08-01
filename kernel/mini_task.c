#include "mini_task.h"


void mini_tcb_init(MiniTCB_t *tcb,uint32_t priority)
{
    tcb->priority=priority;
    mini_list_item_init(&tcb->state_item);
    tcb->state_item.owner=tcb;
}