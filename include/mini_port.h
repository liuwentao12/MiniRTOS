#ifndef MINI_PORT_H
#define MINI_PORT_H

#include <stdint.h>

typedef void (*MiniTaskFunction_t)(void *argument);

uint32_t *mini_port_init_stack(uint32_t *stack_top, MiniTaskFunction_t entry, void *argument);
__attribute__((noreturn)) void mini_port_start_first_task(uint32_t *stack_pointer);

#endif
