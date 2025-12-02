#include "cconartist/channel.h"

namespace ncore
{
    int channel_init(channel_t *ch, size_t capacity)
    {
        if (uv_mutex_init(&ch->mutex) != 0)
            return -1;
        if (uv_cond_init(&ch->cond) != 0)
        {
            uv_mutex_destroy(&ch->mutex);
            return -1;
        }
        ch->nodes = (node_t *)calloc(capacity, sizeof(node_t));
        if (!ch->nodes)
            return -1;
        ch->capacity = capacity;
        ch->size     = 0;
        ch->head = ch->tail = NULL;
        ch->free_list       = &ch->nodes[0];
        for (size_t i = 0; i < capacity - 1; i++)
        {
            ch->nodes[i].next = &ch->nodes[i + 1];
        }
        ch->nodes[capacity - 1].next = NULL;
        return 0;
    }

    void channel_destroy(channel_t *ch)
    {
        uv_mutex_destroy(&ch->mutex);
        uv_cond_destroy(&ch->cond);
        free(ch->nodes);
    }

    int channel_push(channel_t *ch, void *data)
    {
        uv_mutex_lock(&ch->mutex);
        while (ch->size >= ch->capacity)
        {
            uv_cond_wait(&ch->cond, &ch->mutex);
        }
        node_t *n     = ch->free_list;
        ch->free_list = n->next;
        n->data       = data;
        n->next       = NULL;
        if (ch->tail)
            ch->tail->next = n;
        else
            ch->head = n;
        ch->tail = n;
        ch->size++;
        uv_cond_signal(&ch->cond);
        uv_mutex_unlock(&ch->mutex);
        return 0;
    }

    void *channel_pop(channel_t *ch)
    {
        uv_mutex_lock(&ch->mutex);
        while (ch->size == 0)
        {
            uv_cond_wait(&ch->cond, &ch->mutex);
        }
        node_t *n = ch->head;
        ch->head  = n->next;
        if (!ch->head)
            ch->tail = NULL;
        ch->size--;
        void *data    = n->data;
        n->next       = ch->free_list;
        ch->free_list = n;
        uv_cond_signal(&ch->cond);
        uv_mutex_unlock(&ch->mutex);
        return data;
    }

}  // namespace ncore
