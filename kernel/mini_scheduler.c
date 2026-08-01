#include "mini_scheduler.h"
#include "mini_list.h"
#include"mini_task.h"
static MiniList_t ready_lists[MINI_MAX_PRIORITIES];

void mini_scheduler_init(void)
{
    for (uint32_t i = 0; i < MINI_MAX_PRIORITIES; i++)
    {
        mini_list_init(&ready_lists[i]);
    }
}

void mini_scheduler_add_ready(MiniTCB_t *tcb)
{
    assert(tcb->priority < MINI_MAX_PRIORITIES);

    mini_list_insert_end(&ready_lists[tcb->priority],&tcb->state_item);
}