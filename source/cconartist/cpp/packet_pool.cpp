#include "cconartist/packet_pool.h"

#include <string.h>

namespace ncore
{
    packet_pool_t *packet_pool_create(size_t pool_size)
    {
        if (pool_size > INT16_MAX)
            return NULL;
        packet_pool_t *pool = (packet_pool_t *)malloc(sizeof(packet_pool_t));
        pool->packets       = (packet_t *)calloc(pool_size, sizeof(packet_t));
        pool->free_list     = (int16_t *)malloc(pool_size * sizeof(int16_t));
        pool->capacity      = pool_size;
        pool->top           = (int16_t)pool_size - 1;
        for (int16_t i = 0; i < (int16_t)pool_size; i++)
            pool->free_list[i] = i;
        uv_mutex_init(&pool->mutex);
        return pool;
    }

    void packet_pool_destroy(packet_pool_t *pool)
    {
        uv_mutex_destroy(&pool->mutex);
        free(pool->packets);
        free(pool->free_list);
        free(pool);
    }

    packet_t *packet_acquire(packet_pool_t *pool)
    {
        uv_mutex_lock(&pool->mutex);
        if (pool->top < 0)
        {
            uv_mutex_unlock(&pool->mutex);
            return NULL;
        }
        int16_t idx = pool->free_list[pool->top--];
        uv_mutex_unlock(&pool->mutex);
        packet_t *pkt = &pool->packets[idx];
        pkt->conn     = NULL;
        pkt->size     = 0;
        return pkt;
    }

    void packet_release(packet_pool_t *pool, packet_t *pkt)
    {
        uv_mutex_lock(&pool->mutex);
        int16_t idx                  = (int16_t)(pkt - pool->packets);
        pool->free_list[++pool->top] = idx;
        uv_mutex_unlock(&pool->mutex);
    }
}  // namespace ncore
