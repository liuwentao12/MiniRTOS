#ifndef MINI_LIST_H
#define MINI_LIST_H

#include<stdio.h>

typedef struct MiniListItem
{
    __uint32_t value;
    struct MiniListItem *next;
    struct MiniLidtItem *prev;
    void *owner;
}MiniListItem_t;


#endif
