
#include "cconartist/connection_manager.h"
#include <stdlib.h>
#include <string.h>

static int compare_key(const char *ip, int remote_port, const connection_info_t *b)
{
    int cmp = strcmp(ip, b->remote_ip);
    if (cmp == 0)
        return remote_port - b->remote_port;
    return cmp;
}

static ssize_t search_connection(connection_manager_t *mgr, const char *ip, int remote_port, int *found)
{
    size_t left = 0, right = mgr->count;
    *found = 0;
    while (left < right)
    {
        size_t mid = (left + right) / 2;
        int    cmp = compare_key(ip, remote_port, mgr->connections[mid]);
        if (cmp == 0)
        {
            *found = 1;
            return mid;
        }
        else if (cmp < 0)
            right = mid;
        else
            left = mid + 1;
    }
    return left;
}

int connection_manager_init(connection_manager_t *mgr, size_t initial_capacity)
{
    mgr->connections = (connection_info_t **)calloc(initial_capacity, sizeof(connection_info_t *));
    mgr->count       = 0;
    mgr->capacity    = initial_capacity;
    uv_mutex_init(&mgr->mutex);
    return 0;
}

void connection_manager_destroy(connection_manager_t *mgr)
{
    uv_mutex_lock(&mgr->mutex);
    for (size_t i = 0; i < mgr->count; i++)
        free(mgr->connections[i]);
    free(mgr->connections);
    uv_mutex_unlock(&mgr->mutex);
    uv_mutex_destroy(&mgr->mutex);
}

connection_info_t *connection_manager_find_or_create(connection_manager_t *mgr, const char *ip, int remote_port, int local_port, connection_type_t type)
{
    uv_mutex_lock(&mgr->mutex);
    int     found;
    ssize_t idx = search_connection(mgr, ip, remote_port, &found);
    if (found)
    {
        connection_info_t *info = mgr->connections[idx];
        info->state             = STATE_CONNECTED;
        info->last_active       = uv_hrtime();
        uv_mutex_unlock(&mgr->mutex);
        return info;
    }
    if (mgr->count >= mgr->capacity)
    {
        mgr->capacity *= 2;
        mgr->connections = (connection_info_t **)realloc(mgr->connections, mgr->capacity * sizeof(connection_info_t *));
    }
    connection_info_t *info = (connection_info_t *)calloc(1, sizeof(connection_info_t));
    strncpy(info->remote_ip, ip, sizeof(info->remote_ip) - 1);
    info->remote_port = remote_port;
    info->local_port  = local_port;
    info->type        = type;
    info->last_active = uv_hrtime();
    info->state       = STATE_CONNECTED;
    memmove(&mgr->connections[idx + 1], &mgr->connections[idx], (mgr->count - idx) * sizeof(connection_info_t *));
    mgr->connections[idx] = info;
    mgr->count++;
    uv_mutex_unlock(&mgr->mutex);
    return info;
}

void connection_manager_mark_disconnected(connection_manager_t *mgr, const char *ip, int remote_port)
{
    uv_mutex_lock(&mgr->mutex);
    int     found;
    ssize_t idx = search_connection(mgr, ip, remote_port, &found);
    if (found)
    {
        mgr->connections[idx]->state       = STATE_DISCONNECTED;
        mgr->connections[idx]->last_active = uv_hrtime();
    }
    uv_mutex_unlock(&mgr->mutex);
}
