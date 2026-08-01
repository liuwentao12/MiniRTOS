#include "mini_list.h"
#include <stdbool.h>

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

bool mini_list_is_empty(const MiniList_t *list)
{
    return list->count == 0;
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

void mini_list_insert(MiniList_t *list, MiniListItem_t *item)
{
    MiniListItem_t *current = list->end.next;
    while (current != &list->end && current->value <= item->value)
    {
        current = current->next;
    }
    item->next = current;
    item->prev = current->prev;
    item->prev->next = item;
    current->prev = item;
}

void mini_list_remove(MiniList_t *list, MiniListItem_t *item)
{
    assert(list->count > 0);

    item->prev->next = item->next;
    item->next->prev = item->prev;
    list->count--;
}

MiniListItem_t *mini_list_front(MiniList_t *list)
{
    if (mini_list_is_empty(list))
    {
        return NULL;
    }
    return list->end.next;
}

MiniListItem_t *mini_list_pop_front(MiniList_t *list)
{
    MiniListItem_t *item = mini_list_front(list);
    if (item != NULL)
    {
        mini_list_remove(list, item);
    }
    return item;
}

void *mini_list_front_owner(MiniList_t *list)
{
    MiniListItem_t *item = mini_list_front(list);
    if(item == NULL)
    {
        return NULL;
    }
    return item->owner;
}

void mini_tcb_init(MiniTCB_t *tcb,uint32_t priority)
{
    tcb->priority=priority;
    mini_list_item_init(&tcb->state_item);
    tcb->state_item.owner=tcb;
}