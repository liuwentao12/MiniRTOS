#ifndef MINI_TASK_H
#define MINI_TASK_H

#include <stdint.h>
#include "mini_list.h"
#include "mini_port.h"

typedef struct MiniTCB
{
    uint32_t *stack_pointer;
    uint32_t priority;
    MiniListItem_t state_item;
} MiniTCB_t;

void mini_tcb_init(MiniTCB_t *tcb, uint32_t *stack_top, MiniTaskFunction_t entry, void *argument, uint32_t priority);

#endif