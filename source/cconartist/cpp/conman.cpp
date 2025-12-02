#include "cconartist/conman.h"

#include <string.h>

namespace ncore
{
    // Connections are identified by:
    // - type
    // - local_port
    // - remote_port
    // - remote_ip
    static int compare_key(connection_info_t *a, const connection_info_t *b)
    {
        if (a->type < b->type)
            return -1;
        else if (a->type > b->type)
            return 1;

        if (a->local_port < b->local_port)
            return -1;
        else if (a->local_port > b->local_port)
            return 1;

        if (a->remote_port < b->remote_port)
            return -1;
        else if (a->remote_port > b->remote_port)
            return 1;

        for (int i = 0; i < 4; i++)
        {
            if (a->remote_ip[i] != b->remote_ip[i])
            {
                if (a->remote_ip[i] < b->remote_ip[i])
                    return -1;
                else
                    return 1;
            }
        }

        return 0;
    }

    static ssize_t search_connection(connection_manager_t *mgr, connection_info_t *info, int *found)
    {
        size_t left = 0, right = mgr->count;
        *found = 0;
        while (left < right)
        {
            size_t mid = (left + right) / 2;
            int    cmp = compare_key(info, mgr->connections[mid]);
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
        mgr->connections     = (connection_info_t **)calloc(initial_capacity, sizeof(connection_info_t *));
        mgr->free_connection = NULL;
        mgr->count           = 0;
        mgr->capacity        = initial_capacity;
        uv_mutex_init(&mgr->mutex);
        return 0;
    }

    void connection_manager_destroy(connection_manager_t *mgr)
    {
        uv_mutex_lock(&mgr->mutex);
        for (int i = 0; i < mgr->count; i++)
            free(mgr->connections[i]);
        free(mgr->connections);
        uv_mutex_unlock(&mgr->mutex);
        uv_mutex_destroy(&mgr->mutex);
    }

    connection_info_t *connection_manager_alloc(connection_manager_t *mgr)
    {
        uv_mutex_lock(&mgr->mutex);
        if (mgr->free_connection)
        {
            connection_info_t *info = mgr->free_connection;
            mgr->free_connection    = NULL;
            uv_mutex_unlock(&mgr->mutex);
            return info;
        }
        uv_mutex_unlock(&mgr->mutex);
        connection_info_t *info = (connection_info_t *)malloc(sizeof(connection_info_t));
        memset(info, 0, sizeof(connection_info_t));
        return info;
    }

    void connection_manager_release(connection_manager_t *mgr, connection_info_t *info)
    {
        uv_mutex_lock(&mgr->mutex);
        if (mgr->free_connection)
        {
            free(mgr->free_connection);
        }
        mgr->free_connection = info;
        uv_mutex_unlock(&mgr->mutex);
    }

    connection_info_t *connection_manager_commit(connection_manager_t *mgr, connection_info_t *info)
    {
        uv_mutex_lock(&mgr->mutex);
        int     found;
        ssize_t idx = search_connection(mgr, info, &found);
        if (found)
        {
            connection_info_t *existing = mgr->connections[idx];
            connection_manager_release(mgr, info);
            existing->state       = STATE_CONNECTED;
            existing->last_active = uv_hrtime();
            uv_mutex_unlock(&mgr->mutex);
            return existing;
        }
        if (mgr->count >= mgr->capacity)
        {
            mgr->capacity *= 2;
            mgr->connections = (connection_info_t **)realloc(mgr->connections, mgr->capacity * sizeof(connection_info_t *));
        }
        info->last_active = uv_hrtime();
        info->state       = STATE_CONNECTED;
        memmove(&mgr->connections[idx + 1], &mgr->connections[idx], (mgr->count - idx) * sizeof(connection_info_t *));
        mgr->connections[idx] = info;
        mgr->count++;
        uv_mutex_unlock(&mgr->mutex);
        return info;
    }

    void connection_manager_mark_disconnected(connection_manager_t *mgr, connection_info_t *info)
    {
        uv_mutex_lock(&mgr->mutex);
        int     found;
        ssize_t idx = search_connection(mgr, info, &found);
        if (found)
        {
            mgr->connections[idx]->state       = STATE_DISCONNECTED;
            mgr->connections[idx]->last_active = uv_hrtime();
        }
        uv_mutex_unlock(&mgr->mutex);
    }

}  // namespace ncore
