#ifndef CHANNEL_H
#define CHANNEL_H

#include "clibuv/uv.h"

namespace ncore
{
    struct node_t
    {
        void   *data;
        node_t *next;
    };

    struct channel_t
    {
        uv_mutex_t mutex;
        uv_cond_t  cond;
        node_t    *head;
        node_t    *tail;
        node_t    *free_list;
        node_t    *nodes;
        size_t     size;
        size_t     capacity;
    };

    int   channel_init(channel_t *ch, size_t capacity);
    void  channel_destroy(channel_t *ch);
    int   channel_push(channel_t *ch, void *data);
    void *channel_pop(channel_t *ch);

}  // namespace ncore

#endif
