#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include "clibuv/uv.h"

#include <stddef.h>
#include <stdint.h>

typedef enum
{
    CONN_TCP,
    CONN_UDP
} connection_type_t;
typedef enum
{
    STATE_CONNECTED,
    STATE_DISCONNECTED
} connection_state_t;

typedef struct
{
    char                    remote_ip[64];
    int                     remote_port;
    int                     local_port;
    connection_type_t       type;
    uint64_t                last_active;
    connection_state_t      state;
    uv_tcp_t               *handle;       // For TCP
    struct sockaddr_storage remote_addr;  // For UDP
} connection_info_t;

typedef struct
{
    connection_info_t **connections;
    size_t              count;
    size_t              capacity;
    uv_mutex_t          mutex;
} connection_manager_t;

int                connection_manager_init(connection_manager_t *mgr, size_t initial_capacity);
void               connection_manager_destroy(connection_manager_t *mgr);
connection_info_t *connection_manager_find_or_create(connection_manager_t *mgr, const char *ip, int remote_port, int local_port, connection_type_t type);
void               connection_manager_mark_disconnected(connection_manager_t *mgr, const char *ip, int remote_port);

#endif
