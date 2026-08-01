#include "mini_list.h"

void mini_list_init(MiniList_t *list)
{
    list->count = 0;
    list->end.next = &list->end;
    list->end.prev = &list->end;
    list->end.owner = NULL;
    list->end.value = UINT32_MAX;
}

void mini_list_item_init(MiniListItem_t *item)
{
    item->value = 0;
    item->next = NULL;
    item->prev = NULL;
    item->owner = NULL;
}

void mini_list_insert_end(MiniList_t *list, MiniListItem_t *item)
{
    MiniListItem_t *last = list->end.prev;
 
    item->prev = last;
    item->next = &list->end;

    last->next = item;
    list->end.prev = item;

    list->count++;
}
