#ifndef MINI_SCHEDULER_H
#define MINI_SCHEDULER_H

#include "mini_task.h"

#define MINI_MAX_PRIORITIES 5U

void mini_scheduler_init(void);
void mini_scheduler_add_ready(MiniTCB_t *tcb);
MiniTCB_t *mini_scheduler_select_next(void);

#endif