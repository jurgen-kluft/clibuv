#ifndef CHANNEL_H
#define CHANNEL_H

#include "clibuv/uv.h"
#include <stddef.h>

typedef struct node
{
    void        *data;
    struct node *next;
} node_t;

typedef struct
{
    uv_mutex_t mutex;
    uv_cond_t  cond;
    node_t    *head;
    node_t    *tail;
    node_t    *free_list;
    node_t    *nodes;
    size_t     size;
    size_t     capacity;
} channel_t;

int   channel_init(channel_t *ch, size_t capacity);
void  channel_destroy(channel_t *ch);
int   channel_push(channel_t *ch, void *data);
void *channel_pop(channel_t *ch);

#endif
