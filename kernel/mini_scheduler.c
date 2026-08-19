#include "mini_scheduler.h"
#include "mini_list.h"
#include "mini_task.h"
#include <assert.h>

static MiniList_t ready_lists[MINI_MAX_PRIORITIES];
static MiniTCB_t *current_task = NULL;

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

MiniTCB_t *mini_scheduler_select_next(void)
{
    for (uint32_t priority = MINI_MAX_PRIORITIES; priority > 0U; priority--)
    {
        uint32_t index = priority - 1U;
        if(!mini_list_is_empty(&ready_lists[index]))
        {
            return (MiniTCB_t *)mini_list_front_owner(&ready_lists[index]);
        }
    }
    return NULL;
}

MiniTCB_t *mini_scheduler_get_current(void)
{
    return current_task;
}