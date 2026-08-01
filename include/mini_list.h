#ifndef MINI_LIST_H
#define MINI_LIST_H

#include <stdint.h>
#include<stddef.h>

typedef struct MiniListItem
{
    uint32_t value;
    struct MiniListItem *next;
    struct MiniListItem *prev;
    void *owner;
} MiniListItem_t;

typedef struct Mini_list
{
    uint32_t count;
    MiniListItem_t end;
} MiniList_t;

void mini_list_init(MiniList_t *list);
void mini_list_item_init(MiniListItem_t *item);
void mini_list_insert_end(MiniList_t *list,MiniListItem_t *item);
void mini_list_insert(MiniList_t *list, MiniListItem_t *item);

#endif
