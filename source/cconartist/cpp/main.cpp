#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ccore/c_target.h"
#include "clibuv/uv.h"

#include "cconartist/channel.h"
#include "cconartist/conman.h"
#include "cconartist/packet_pool.h"
#include "cconartist/udp_send_pool.h"
#include "cconartist/tcp_write_pool.h"

#define INITIAL_CONN_CAPACITY 128
#define PACKET_POOL_SIZE      1024

namespace ncore
{
    struct server_context_t
    {
        uv_loop_t            *loop;
        channel_t            *channel_packets_out;  // Libuv → UI
        channel_t            *channel_packets_in;   // UI → Libuv
        connection_manager_t *conn_mgr;
        packet_pool_t        *packet_pool;
        uv_async_t            async_send;       // NEW: async handle for sending packets
        uv_udp_t              udp_send_handle;  // NEW: UDP handle for sending packets
        udp_send_pool_t      *udp_send_pool;
        tcp_write_pool_t     *tcp_write_pool;
    };

    void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
    {
        buf->base = (char *)malloc(suggested_size);
        buf->len  = suggested_size;
    }

    void after_write(uv_write_t *req, int status)
    {
        free(req->data);
        free(req);
    }

    void on_tcp_close(uv_handle_t *handle) { free(handle); }

    void on_tcp_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
    {
        connection_info_t *info = (connection_info_t *)client->data;
        server_context_t  *ctx  = info->server_context;
        if (nread > 0)
        {
            packet_t *pkt = packet_acquire(ctx->packet_pool);
            if (pkt)
            {
                pkt->conn = info;
                pkt->size = nread;
                memcpy(pkt->data, buf->base, nread);
                channel_push(ctx->channel_packets_out, pkt);

                info->last_active = uv_hrtime();

                printf("Received TCP packet from %s:%d, size: %zu\n", info->remote_ip, info->remote_port, pkt->size);
            }
            else
            {
                // Failed to acquire packet, consider logging or handling this case
            }
        }
        else if (nread == UV_EOF)
        {
            connection_manager_mark_disconnected(ctx->conn_mgr, info);
            uv_close((uv_handle_t *)client, on_tcp_close);
        }
        if (buf->base)
            free(buf->base);
    }

    void on_new_tcp_connection(uv_stream_t *server, int status)
    {
        if (status < 0)
            return;
        uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
        uv_tcp_init(server->loop, client);
        if (uv_accept(server, (uv_stream_t *)client) == 0)
        {
            server_context_t *ctx = (server_context_t *)server->data;

            struct sockaddr_storage addr;
            int                     len = sizeof(addr);
            uv_tcp_getpeername(client, (struct sockaddr *)&addr, &len);
            sockaddr const *saddr = (struct sockaddr *)&addr;

            sockaddr_in const *addrin      = (struct sockaddr_in *)&addr;
            int                remote_port = ntohs(addrin->sin_port);

            struct sockaddr_storage local;
            len = sizeof(local);
            uv_tcp_getsockname(client, (struct sockaddr *)&local, &len);
            int local_port = ntohs(((struct sockaddr_in *)&local)->sin_port);

            connection_info_t *info = connection_manager_alloc(ctx->conn_mgr);
            if (saddr->sa_family == AF_INET)
            {
                // Handle IPv4 address extraction
                uint32_t const *ipv4 = (uint32_t const *)&((struct sockaddr_in *)saddr)->sin_addr.s_addr;
                info->remote_ip[0]   = ipv4[0];
                info->remote_ip[1]   = 0;
                info->remote_ip[2]   = 0;
                info->remote_ip[3]   = 0;
            }
            else if (saddr->sa_family == AF_INET6)
            {
                // Handle IPv6 address extraction
                uint32_t const *ipv6 = (uint32_t const *)&((struct sockaddr_in6 *)saddr)->sin6_addr.s6_addr[0];
                info->remote_ip[0]   = ipv6[0];
                info->remote_ip[1]   = ipv6[1];
                info->remote_ip[2]   = ipv6[2];
                info->remote_ip[3]   = ipv6[3];
            }
            info->remote_port = remote_port;
            info->local_port  = local_port;
            info->type        = CONN_UDP;

            info = connection_manager_commit(ctx->conn_mgr, info);
            memcpy(&info->remote_addr, saddr, sizeof(struct sockaddr_storage));

            info->server_context = ctx;
            info->handle         = client;
            client->data         = ctx;
            uv_read_start((uv_stream_t *)client, alloc_buffer, on_tcp_read);
        }
        else
        {
            uv_close((uv_handle_t *)client, on_tcp_close);
        }
    }

    void start_tcp_server(server_context_t *ctx, int port)
    {
        uv_tcp_t *server = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
        uv_tcp_init(ctx->loop, server);
        struct sockaddr_in addr;
        uv_ip4_addr("0.0.0.0", port, &addr);  // Bind to all interfaces
        uv_tcp_bind(server, (const struct sockaddr *)&addr, 0);
        server->data = ctx;
        uv_listen((uv_stream_t *)server, 128, on_new_tcp_connection);
    }

    void on_udp_recv(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags)
    {
        if (nread > 0 && addr)
        {
            server_context_t *ctx = (server_context_t *)handle->data;
            packet_t         *pkt = packet_acquire(ctx->packet_pool);
            if (pkt)
            {
                connection_info_t *info = connection_manager_alloc(ctx->conn_mgr);
                if (addr->sa_family == AF_INET)
                {
                    // Handle IPv4 address extraction
                    uint32_t const *ipv4 = (uint32_t const *)&((struct sockaddr_in *)addr)->sin_addr.s_addr;
                    info->remote_ip[0]   = ipv4[0];
                    info->remote_ip[1]   = 0;
                    info->remote_ip[2]   = 0;
                    info->remote_ip[3]   = 0;
                }
                else if (addr->sa_family == AF_INET6)
                {
                    // Handle IPv6 address extraction
                    uint32_t const *ipv6 = (uint32_t const *)&((struct sockaddr_in6 *)addr)->sin6_addr.s6_addr[0];
                    info->remote_ip[0]   = ipv6[0];
                    info->remote_ip[1]   = ipv6[1];
                    info->remote_ip[2]   = ipv6[2];
                    info->remote_ip[3]   = ipv6[3];
                }
                info->remote_port = ntohs(((struct sockaddr_in *)addr)->sin_port);
                info->type        = CONN_UDP;

                info = connection_manager_commit(ctx->conn_mgr, info);
                memcpy(&info->remote_addr, addr, sizeof(struct sockaddr_storage));

                pkt->conn = info;
                pkt->size = nread;
                memcpy(pkt->data, buf->base, nread);
                channel_push(ctx->channel_packets_out, pkt);

                // printf("Received UDP packet from %s:%d, size: %zu\n", ip, port, pkt->size);
            }
        }
        if (buf->base)
            free(buf->base);
    }

    void start_udp_server(server_context_t *ctx, int port)
    {
        uv_udp_t *udp = (uv_udp_t *)malloc(sizeof(uv_udp_t));
        uv_udp_init(ctx->loop, udp);
        struct sockaddr_in addr;
        uv_ip4_addr("0.0.0.0", port, &addr);
        uv_udp_bind(udp, (const struct sockaddr *)&addr, UV_UDP_REUSEADDR);
        udp->data = ctx;
        uv_udp_recv_start(udp, alloc_buffer, on_udp_recv);
    }

    // Async callback for sending packets

    void on_tcp_write_done(uv_write_t *req, int status)
    {
        if (req->data)
            free(req->data);
        server_context_t *ctx = (server_context_t *)req->handle->data;
        write_pool_release(ctx->tcp_write_pool, req);
    }

    void on_udp_send_done(uv_udp_send_t *req, int status)
    {
        server_context_t *ctx = (server_context_t *)req->handle->data;
        send_pool_release(ctx->udp_send_pool, req);
    }

    void on_async_send(uv_async_t *handle)
    {
        server_context_t *ctx = (server_context_t *)handle->data;
        while (1)
        {
            packet_t *pkt = (packet_t *)channel_pop(ctx->channel_packets_in);
            if (!pkt)
                break;

            if (pkt->conn && pkt->conn->state == STATE_CONNECTED)
            {
                if (pkt->conn->type == CONN_TCP)
                {
                    if (pkt->conn->handle)
                    {
                        uv_write_t *req = write_pool_acquire(ctx->tcp_write_pool);
                        if (req)
                        {
                            // TODO also reuse uv_buf_t from a pool
                            uv_buf_t buf = uv_buf_init((char *)malloc(pkt->size), pkt->size);
                            memcpy(buf.base, pkt->data, pkt->size);
                            req->data = buf.base;
                            uv_write(req, (uv_stream_t *)pkt->conn->handle, &buf, 1, on_tcp_write_done);
                        }
                        else
                        {
                            fprintf(stderr, "Send pool exhausted for TCP!\n");
                        }
                    }
                }
                else if (pkt->conn->type == CONN_UDP)
                {
                    // TODO also reuse uv_buf_t from a pool
                    uv_buf_t       buf      = uv_buf_init(pkt->data, pkt->size);
                    uv_udp_send_t *send_req = send_pool_acquire(ctx->udp_send_pool);
                    if (send_req)
                    {
                        uv_udp_send(send_req, &ctx->udp_send_handle, &buf, 1, (const struct sockaddr *)&pkt->conn->remote_addr, on_udp_send_done);
                    }
                    else
                    {
                        fprintf(stderr, "Send pool exhausted for UDP!\n");
                    }
                }
            }
            packet_release(ctx->packet_pool, pkt);
        }
    }

    void ui_thread(void *arg)
    {
        server_context_t *ctx = (server_context_t *)arg;
        while (1)
        {
            packet_t *pkt = (packet_t *)channel_pop(ctx->channel_packets_out);
            printf("[UI] Received packet from %s:%d size=%zu\n", pkt->conn->remote_ip, pkt->conn->remote_port, pkt->size);

            // Example: send response back
            packet_t *out_pkt = packet_acquire(ctx->packet_pool);
            if (out_pkt)
            {
                out_pkt->conn   = pkt->conn;
                const char *msg = "Hello from UI!";
                out_pkt->size   = strlen(msg);
                memcpy(out_pkt->data, msg, out_pkt->size);
                channel_push(ctx->channel_packets_in, out_pkt);
                uv_async_send(&ctx->async_send);  // Signal event loop immediately
            }
            packet_release(ctx->packet_pool, pkt);
        }
    }
}  // namespace ncore

using namespace ncore;

int main()
{
    uv_loop_t *loop = uv_default_loop();
    channel_t  channel_packets_out, channel_packets_in;
    channel_init(&channel_packets_out, 1024);
    channel_init(&channel_packets_in, 1024);
    connection_manager_t conn_mgr;
    connection_manager_init(&conn_mgr, INITIAL_CONN_CAPACITY);
    packet_pool_t *packet_pool = packet_pool_create(PACKET_POOL_SIZE);

    server_context_t ctx;                            // = {loop, &channel_packets_out, &channel_packets_in, &conn_mgr, packet_pool, };
    ctx.loop                = loop;                  // Event loop
    ctx.channel_packets_out = &channel_packets_out;  // Libuv → UI
    ctx.channel_packets_in  = &channel_packets_in;   // UI → Libuv
    ctx.conn_mgr            = &conn_mgr;             // Connection manager
    ctx.packet_pool         = packet_pool;           // Packet pool
    ctx.udp_send_pool       = (udp_send_pool_t *)malloc(sizeof(udp_send_pool_t));
    ctx.tcp_write_pool      = (tcp_write_pool_t *)malloc(sizeof(tcp_write_pool_t));

    send_pool_init(ctx.udp_send_pool);
    write_pool_init(ctx.tcp_write_pool);

    uv_async_init(loop, &ctx.async_send, on_async_send);
    ctx.async_send.data = &ctx;

    uv_udp_init(loop, &ctx.udp_send_handle);

    uv_thread_t ui;
    uv_thread_create(&ui, ui_thread, &ctx);

    int tcp_ports[] = {31330, 31372};
    int udp_ports[] = {31370, 31371};

    printf("Starting servers...\n");

    for (int i = 0; i < (int)DARRAYSIZE(tcp_ports); i++)
    {
        printf("Starting TCP server on port %d\n", tcp_ports[i]);
        start_tcp_server(&ctx, tcp_ports[i]);
    }
    for (int i = 0; i < (int)DARRAYSIZE(udp_ports); i++)
    {
        printf("Starting UDP server on port %d\n", udp_ports[i]);
        start_udp_server(&ctx, udp_ports[i]);
    }

    uv_run(loop, UV_RUN_DEFAULT);

    connection_manager_destroy(&conn_mgr);
    channel_destroy(&channel_packets_out);
    channel_destroy(&channel_packets_in);
    packet_pool_destroy(packet_pool);
    return 0;
}
