#ifndef PACKET_POOL_H
#define PACKET_POOL_H

#include "clibuv/uv.h"

#include <stddef.h>
#include <stdint.h>

#include "cconartist/connection_manager.h"

#define MAX_PACKET_SIZE 500

typedef struct
{
    connection_info_t *conn;
    size_t             size;
    char               data[MAX_PACKET_SIZE];
} packet_t;

typedef struct
{
    packet_t  *packets;
    int16_t   *free_list;
    int16_t    top;
    size_t     capacity;
    uv_mutex_t mutex;
} packet_pool_t;

packet_pool_t *packet_pool_create(size_t pool_size);
void           packet_pool_destroy(packet_pool_t *pool);
packet_t      *packet_acquire(packet_pool_t *pool);
void           packet_release(packet_pool_t *pool, packet_t *pkt);

#endif
