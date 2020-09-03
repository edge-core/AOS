//
//  http_worker.c
//  http
//
//  Created by JunYing Yeh on 2014/7/16.
//
//

#include "http_loc.h"

static void http_worker_init(HTTP_Worker_T *worker);
static void http_worker_close(HTTP_Worker_T *worker);

static void http_worker_delete_sessions(CONNECTION_STATE_T type);
static void http_worker_delete_connection(HTTP_Worker_T *worker, CONNECTION_STATE_T type, L_INET_AddrIp_T *ip_addr);

static void http_worker_master_init(HTTP_Worker_T *worker);
static void http_worker_master_close(HTTP_Worker_T *worker);

static void http_worker_shutdown(HTTP_Worker_T *worker, CONNECTION_STATE_T type);

static BOOL_T http_worker_start_http(HTTP_Worker_T *worker);

#if (SYS_CPNT_HTTPS == TRUE)
static BOOL_T http_worker_start_https(HTTP_Worker_T *worker);
#endif /* SYS_CPNT_HTTPS */

static void http_worker_process_command(HTTP_Event_T *event);
static int http_worker_set_socket_option(HTTP_Worker_T *worker, int sock);
static void http_worker_new_connection(HTTP_Event_T *event);
static void http_worker_send_reply(int fd, int status, const char *message);

#define HTTP_WORKER_FMT "%s(%d): "
#define HTTP_WORKER_ARG ((worker->kind == HTTP_WORKER_MASTER) ? "Master" : "Worker"), worker->tid

void http_worker_main(HTTP_Worker_T *worker)
{
    UI32_T      tid = SYSFUN_TaskIdSelf();

    worker->tid = tid;

    http_worker_init(worker);

    while (1)
    {
        if (worker->close)
        {
            HTTP_LOG_ERROR(HTTP_LOG_LEVEL_INFO, HTTP_LOG_MSG_WORKER, 1,
                           HTTP_WORKER_FMT"close worker",
                           HTTP_WORKER_ARG);
            break;
        }

        http_event_rocess_events(worker->event_ctx, 2000);
    }

    http_worker_close(worker);

    HTTP_LOG_ERROR(HTTP_LOG_LEVEL_INFO, HTTP_LOG_MSG_WORKER, 1,
                   HTTP_WORKER_FMT"leave main loop",
                   HTTP_WORKER_ARG);
}

int HTTP_TASK_StartHttpService();
int HTTP_TASK_StartHttpsService();
static void http_worker_init(HTTP_Worker_T *worker)
{
    int         ret;

    HTTP_LOG_ERROR(HTTP_LOG_LEVEL_INFO, HTTP_LOG_MSG_WORKER, 1,
                   HTTP_WORKER_FMT"init",
                   HTTP_WORKER_ARG);

    worker->cmd_channel[0]  = -1;
    worker->cmd_channel[1]  = -1;
    worker->http_fd         = -1;
    worker->https_fd        = -1;

    worker->event_ctx = http_event_new_context();
    if (worker->event_ctx == NULL)
    {
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_INFO, HTTP_LOG_MSG_MEMORY, 1,
                       HTTP_WORKER_FMT"Failed to allocate event context",
                       HTTP_WORKER_ARG);
        goto fail;
    }

    ret = socketpair(AF_LOCAL, SOCK_STREAM, 0, worker->cmd_channel);
    if (ret != 0)
    {
        char errstr[64] = {0};
        strerror_r(errno, errstr, sizeof(errstr));
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_INFO, HTTP_LOG_MSG_SOCKET, 1,
                       HTTP_WORKER_FMT"socketpair(%s)",
                       HTTP_WORKER_ARG, errstr);

        goto fail;
    }

    {
        HTTP_Event_T event;

        memset(&event, 0, sizeof(event));

        event.event_type    = HTTP_EVENT_READ;
        event.fd            = worker->cmd_channel[1];
        event.handler       = http_worker_process_command;
        event.data          = worker;

        http_event_add(worker->event_ctx, &event, HTTP_TIMER_INFINITE);
    }

    if (worker->kind == HTTP_WORKER_MASTER)
    {
        http_worker_master_init(worker);
    }

    if (0)
    {
    fail:
        worker->close = 1;
    }
}

static void http_worker_close(HTTP_Worker_T *worker)
{
    HTTP_LOG_ERROR(HTTP_LOG_LEVEL_INFO, HTTP_LOG_MSG_WORKER, 1,
                   HTTP_WORKER_FMT"init",
                   HTTP_WORKER_ARG);

    if (worker->kind == HTTP_WORKER_MASTER)
    {
        http_worker_master_close(worker);
    }

    if (worker->event_ctx)
    {
        http_event_free_context(&worker->event_ctx);
    }

    if (0 < worker->cmd_channel[0])
    {
        HTTP_closesocket(worker->cmd_channel[0]);
        worker->cmd_channel[0] = -1;
    }

    if (0 < worker->cmd_channel[1])
    {
        HTTP_closesocket(worker->cmd_channel[1]);
        worker->cmd_channel[1] = -1;
    }

    worker->tid = 0;
}

static void http_worker_master_init(HTTP_Worker_T *worker)
{
    HTTP_LOG_ERROR(HTTP_LOG_LEVEL_INFO, HTTP_LOG_MSG_WORKER, 1,
                   HTTP_WORKER_FMT"init",
                   HTTP_WORKER_ARG);

    if (http_worker_start_http(worker) == FALSE)
    {
        goto fail;
    }

#if (SYS_CPNT_HTTPS == TRUE)
    if (http_worker_start_https(worker) == FALSE)
    {
        goto fail;
    }
#endif /* SYS_CPNT_HTTPS */

    if (0)
    {
    fail:
        worker->close = 1;
    }
}

static void http_worker_delete_sessions(CONNECTION_STATE_T type)
{
    HTTP_Session_T http_session;
    memset(&http_session, 0, sizeof(http_session));

    while(TRUE == HTTP_OM_Get_Next_User_Connection_Info(&http_session, SYSFUN_GetSysTick()))
    {
        if (http_session.protocol == type)
        {
            HTTP_OM_Delete_User_Connection_Info(http_session.session_id);
        }
    }
}


static void http_worker_delete_connection(HTTP_Worker_T *worker, CONNECTION_STATE_T type, L_INET_AddrIp_T *ip_addr)
{
    HTTP_INSTANCE_PTR_T conn, tconn;
    int i = 0;

    for (conn = worker->connections.list_ptr.links.first_node.in; conn != NULL; conn = tconn)
    {
        HTTP_Connection_T *http_connection = (HTTP_Connection_T *) conn;

        tconn = conn->links.next.in;

        if (http_connection->conn_state != type)
        {
            continue;
        }

        {
            L_INET_AddrIp_T         local_inetaddr;
            struct sockaddr_in6     local_sockaddr;
            socklen_t               sockaddrlen;
            int                     rc;

            sockaddrlen = sizeof(local_sockaddr);
            rc = getsockname(http_connection->fds[HTTP_CONN_FD_NET], (struct sockaddr *) &local_sockaddr, &sockaddrlen);

            if(rc < 0)
            {
                continue;
            }

            L_INET_SockaddrToInaddr((struct sockaddr *)&local_sockaddr, &local_inetaddr);

            if (L_INET_CompareInetAddr((L_INET_Addr_T *) &local_inetaddr, (L_INET_Addr_T *) ip_addr, 0) != 0)
            {
                continue;
            }
        }

        {
            HTTP_Event_T ev;
            memset(&ev, 0, sizeof(ev));

            http_connection->done = 1;
            http_connection->keepalive = 0;
            ev.data = http_connection;
            http_connection_finalize(&ev);
        }
    }
}

static void http_worker_shutdown(HTTP_Worker_T *worker, CONNECTION_STATE_T type)
{
    ASSERT(worker != NULL);
    ASSERT(type == HTTP_CONNECTION || type == HTTPS_CONNECTION);

    if (type == HTTP_CONNECTION && 0 <= worker->http_fd)
    {
        HTTP_Event_T event;
        memset(&event, 0, sizeof(event));

        event.fd = worker->http_fd;
        event.event_type = HTTP_EVENT_READ;
        http_event_set_removeable(worker->event_ctx, &event);

        HTTP_closesocket(worker->http_fd);
        worker->http_fd = -1;
    }
    else if (type == HTTPS_CONNECTION && 0 <= worker->https_fd)
    {
        HTTP_Event_T event;
        memset(&event, 0, sizeof(event));

        event.fd = worker->https_fd;
        event.event_type = HTTP_EVENT_READ;
        http_event_set_removeable(worker->event_ctx, &event);

        HTTP_closesocket(worker->https_fd);
        worker->https_fd = -1;
    }

    {
        HTTP_INSTANCE_PTR_T conn, tconn;

        for (conn = worker->connections.list_ptr.links.first_node.in; conn != NULL; conn = tconn)
        {
            HTTP_Connection_T *http_connection = (HTTP_Connection_T *) conn;

            tconn = conn->links.next.in;

            if (http_connection->conn_state == type)
            {
                HTTP_Event_T ev;
                memset(&ev, 0, sizeof(ev));

                http_connection->done = 1;
                http_connection->keepalive = 0;
                ev.data = http_connection;
                http_connection_finalize(&ev);
            }
        }
    }
}

static BOOL_T http_worker_start_http(HTTP_Worker_T *worker)
{
    HTTP_STATE_T    current_http_state;
    HTTP_Event_T    event;

    if (0 <= worker->http_fd)
    {
        return TRUE;
    }

    current_http_state = HTTP_MGR_Get_Http_Status();
    if (current_http_state == HTTP_STATE_ENABLED)
    {
        worker->http_fd = HTTP_TASK_StartHttpService();

        if (worker->http_fd == -1)
        {
            HTTP_LOG_ERROR(HTTP_LOG_LEVEL_CRIT, HTTP_LOG_MSG_SOCKET, 1,
                           HTTP_WORKER_FMT"Failed to start HTTP service",
                           HTTP_WORKER_ARG);
            goto fail;
        }

        memset(&event, 0, sizeof(event));
        event.event_type = HTTP_EVENT_READ;
        event.data = worker;
        event.fd = worker->http_fd;
        event.handler = http_worker_new_connection;
        http_event_add(worker->event_ctx, &event, HTTP_TIMER_INFINITE);
    }

    if (0)
    {
    fail:
        return FALSE;
    }

    return TRUE;
}

#if (SYS_CPNT_HTTPS == TRUE)
static BOOL_T http_worker_start_https(HTTP_Worker_T *worker)
{
    SECURE_HTTP_STATE_T current_secure_http_state;
    HTTP_Event_T    event;

    if (0 <= worker->https_fd)
    {
        return TRUE;
    }

    current_secure_http_state = HTTP_MGR_Get_Secure_Http_Status();
    if (SECURE_HTTP_STATE_ENABLED == current_secure_http_state)
    {
        worker->https_fd = HTTP_TASK_StartHttpsService();

        if (worker->https_fd == -1)
        {
            HTTP_LOG_ERROR(HTTP_LOG_LEVEL_CRIT, HTTP_LOG_MSG_SOCKET, 1,
                           HTTP_WORKER_FMT"Failed to start HTTPS service",
                           HTTP_WORKER_ARG);
            goto fail;
        }

        memset(&event, 0, sizeof(event));
        event.event_type = HTTP_EVENT_READ;
        event.data = worker;
        event.fd = worker->https_fd;
        event.handler = http_worker_new_connection;
        http_event_add(worker->event_ctx, &event, HTTP_TIMER_INFINITE);
    }

    if (0)
    {
    fail:
        return FALSE;
    }

    return TRUE;
}
#endif /* SYS_CPNT_HTTPS */

static void http_worker_master_close(HTTP_Worker_T *worker)
{
    HTTP_LOG_ERROR(HTTP_LOG_LEVEL_INFO, HTTP_LOG_MSG_WORKER, 1,
                   HTTP_WORKER_FMT"close",
                   HTTP_WORKER_ARG);

    if (0 <= worker->http_fd)
    {
        HTTP_closesocket(worker->http_fd);
        worker->http_fd = -1;
    }

    if (0 <= worker->https_fd)
    {
        HTTP_closesocket(worker->https_fd);
        worker->https_fd = -1;
    }
}

static void http_worker_process_command(HTTP_Event_T *event)
{
    HTTP_Worker_T   *worker;
    int             ret;

    worker = (HTTP_Worker_T *)event->data;

    ASSERT(worker != NULL);

    ret = recv(event->fd,
               worker->received_buffer.raw + worker->received_count,
               sizeof(worker->received_buffer) - worker->received_count,
               0);

    if (ret < 0)
    {
        char errstr[64] = {0};
        strerror_r(errno, errstr, sizeof(errstr));
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_CRIT, HTTP_LOG_MSG_SOCKET, 1,
                       HTTP_WORKER_FMT"recv(%s)",
                       HTTP_WORKER_ARG, errstr);

        worker->close = 1;

        return;
    }

    if (ret == 0)
    {
        worker->close = 1;
        return; // disconnected ??
    }

    worker->received_count += ret;

#define PACKET_SHIFT_N_BYTE(n)                                  \
    memmove(&worker->received_buffer,                           \
            (char *) &worker->received_buffer + (n),            \
            worker->received_count - (n));                      \
    worker->received_count -= (n);

    while (0 < worker->received_count)
    {
        switch (worker->received_buffer.packet.command)
        {
            case HTTP_WORKER_CLOSE:
            {
                ASSERT(1 <= worker->received_count);

                http_worker_shutdown(worker, HTTP_CONNECTION);
                http_worker_shutdown(worker, HTTPS_CONNECTION);

                worker->close = 1;

                PACKET_SHIFT_N_BYTE(1);

                break;
            }

            case HTTP_WORKER_START_HTTP:
            {
                ASSERT(1 <= worker->received_count);

                if (worker->kind == HTTP_WORKER_MASTER)
                {
                    http_worker_start_http(worker);
                }

                PACKET_SHIFT_N_BYTE(1);

                break;
            }

            case HTTP_WORKER_SHUTDOWN_HTTP:
            {
                ASSERT(1 <= worker->received_count);

                http_worker_shutdown(worker, HTTP_CONNECTION);
                http_worker_delete_sessions(HTTP_CONNECTION);

                PACKET_SHIFT_N_BYTE(1);

                break;
            }

            case HTTP_WORKER_RESTART_HTTP:
            {
                ASSERT(1 <= worker->received_count);

                http_worker_shutdown(worker, HTTP_CONNECTION);

                if (worker->kind == HTTP_WORKER_MASTER)
                {
                    http_worker_start_http(worker);
                }

                PACKET_SHIFT_N_BYTE(1);

                break;
            }

#if (SYS_CPNT_HTTPS == TRUE)
            case HTTP_WORKER_START_HTTPS:
            {
                ASSERT(1 <= worker->received_count);

                if (worker->kind == HTTP_WORKER_MASTER)
                {
                    http_worker_start_https(worker);
                }

                PACKET_SHIFT_N_BYTE(1);

                break;
            }

            case HTTP_WORKER_SHUTDOWN_HTTPS:
            {
                ASSERT(1 <= worker->received_count);

                http_worker_shutdown(worker, HTTPS_CONNECTION);
                http_worker_delete_sessions(HTTPS_CONNECTION);

                PACKET_SHIFT_N_BYTE(1);

                break;
            }

            case HTTP_WORKER_RESTART_HTTPS:
            {
                ASSERT(1 <= worker->received_count);

                http_worker_shutdown(worker, HTTPS_CONNECTION);

                if (worker->kind == HTTP_WORKER_MASTER)
                {
                    http_worker_start_https(worker);
                }

                PACKET_SHIFT_N_BYTE(1);

                break;
            }
#endif /* SYS_CPNT_HTTPS */

        case HTTP_WORKER_NEW_CONNECTION:
        {
            size_t except_size = sizeof(worker->received_buffer.packet.command) +
                                 sizeof(worker->received_buffer.packet.u.new_connection);
            HTTP_Connection_T   *http_connection;

            if (worker->received_count < except_size)
            {
                return;
            }

            http_connection = worker->received_buffer.packet.u.new_connection.connection;

            HTTP_LOG_ERROR(HTTP_LOG_LEVEL_INFO, HTTP_LOG_MSG_WORKER, 1,
                           HTTP_WORKER_FMT"add new connection, fd(%d)",
                           HTTP_WORKER_ARG,
                           http_connection->fds[HTTP_CONN_FD_NET]);

            http_connection->worker = worker;
            HTTP_LIST_InsertEnd(&worker->connections, &http_connection->super);

            event->event_type = HTTP_EVENT_READ;

            event->fd = http_connection->fds[HTTP_CONN_FD_NET];
            event->data = http_connection;
            event->handler = http_connection_init;
            http_connection_init(event);

                PACKET_SHIFT_N_BYTE(except_size);

                break;
            }

            case HTTP_WORKER_DEL_CONNECTION:
            {
                size_t except_size = sizeof(worker->received_buffer.packet.command) +
                                     sizeof(worker->received_buffer.packet.u.del_connection);
                HTTP_Connection_T   *http_connection;

                if (worker->received_count < except_size)
                {
                    return;
                }

                {
                    char ip_addr_str[100];

                    L_INET_InaddrToString((L_INET_Addr_T *) &worker->received_buffer.packet.u.del_connection.ip_addr, ip_addr_str, sizeof(ip_addr_str));

                    HTTP_LOG_ERROR(HTTP_LOG_LEVEL_INFO, HTTP_LOG_MSG_WORKER, 1,
                                   HTTP_WORKER_FMT"del connection, IP Addr(%s)",
                                   HTTP_WORKER_ARG,
                                   ip_addr_str);
                }

                http_worker_delete_connection(worker, HTTP_CONNECTION, &worker->received_buffer.packet.u.del_connection.ip_addr);
            #if (SYS_CPNT_HTTPS == TRUE)
                http_worker_delete_connection(worker, HTTPS_CONNECTION, &worker->received_buffer.packet.u.del_connection.ip_addr);
            #endif /* SYS_CPNT_HTTPS */

                PACKET_SHIFT_N_BYTE(except_size);

                break;
            }

            default:
            {
                break;
            }
        }
    }
#undef PACKET_SHIFT_N_BYTE
}

int http_worker_ctrl(HTTP_Worker_T *worker, HTTP_COMMAND_T command, void *params)
{
    int             ret = -1;

    switch (command)
    {
        case HTTP_WORKER_START_HTTP:
        case HTTP_WORKER_SHUTDOWN_HTTP:
        case HTTP_WORKER_START_HTTPS:
        case HTTP_WORKER_SHUTDOWN_HTTPS:
        case HTTP_WORKER_RESTART_HTTP:
        case HTTP_WORKER_RESTART_HTTPS:
        case HTTP_WORKER_CLOSE:
        {
            ret = send(worker->cmd_channel[0], (char *) &command, sizeof(command), 0);
            break;
        }

        case HTTP_WORKER_NEW_CONNECTION:
        {
            HTTP_COMMAND_PACKET_T packet = {0};

            packet.command = command;
            packet.u.new_connection.connection = (HTTP_Connection_T *)params;

            ret = send(worker->cmd_channel[0], (char *) &packet, sizeof(packet.command) + sizeof(packet.u.new_connection), 0);
            break;
        }

        case HTTP_WORKER_DEL_CONNECTION:
        {
            HTTP_COMMAND_PACKET_T packet = {0};

            packet.command = command;
            packet.u.del_connection.ip_addr = *(L_INET_AddrIp_T *)params;

            ret = send(worker->cmd_channel[0], (char *) &packet, sizeof(packet.command) + sizeof(packet.u.del_connection), 0);
            break;
        }

        default:
        {
            break;
        }
    }

    return ret;
}

/* FUNCTION NAME : http_worker_set_socket_option
 * PURPOSE : Set socket option:
 *           1. TCP keep-alive mechanism to detect client is alive or not.
 *              For more detail about the following definition, please see:
 *              http://tldp.org/HOWTO/html_single/TCP-Keepalive-HOWTO/
 * INPUT   : worker -- Worker information.
 *           sock   -- Socket ID.
 * OUTPUT  : None.
 * RETURN  : 0      -- Success.
 *           Others -- Error. Value returned from last setsockopt function call.
 * NOTE    : None.
 */
static int http_worker_set_socket_option(HTTP_Worker_T *worker, int sock)
{
    int ret;
    int value;

    value = 1;

    ret = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value));

    if (ret < 0)
    {
        char errstr[64] = {0};
        strerror_r(errno, errstr, sizeof(errstr));
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_ERROR, HTTP_LOG_MSG_SOCKET, 1,
                       HTTP_WORKER_FMT"setsockopt(%d) fail: %s",
                       HTTP_WORKER_ARG, sock, errstr);
        return ret;
    }

    value = HTTP_CFG_TCP_KEEP_ALIVE_COUNT;
    ret = setsockopt(sock, SOL_TCP, TCP_KEEPCNT, &value, sizeof(value));

    if (ret < 0)
    {
        char errstr[64] = {0};
        strerror_r(errno, errstr, sizeof(errstr));
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_ERROR, HTTP_LOG_MSG_SOCKET, 1,
                       HTTP_WORKER_FMT"setsockopt(%d) fail: %s",
                       HTTP_WORKER_ARG, sock, errstr);
        return ret;
    }

    value = HTTP_CFG_TCP_KEEP_ALIVE_IDLE;
    ret = setsockopt(sock, SOL_TCP, TCP_KEEPIDLE, &value, sizeof(value));

    if (ret < 0)
    {
        char errstr[64] = {0};
        strerror_r(errno, errstr, sizeof(errstr));
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_ERROR, HTTP_LOG_MSG_SOCKET, 1,
                       HTTP_WORKER_FMT"setsockopt(%d) fail: %s",
                       HTTP_WORKER_ARG, sock, errstr);
        return ret;
    }

    value = HTTP_CFG_TCP_KEEP_ALIVE_INTERVAL;
    ret = setsockopt(sock, SOL_TCP, TCP_KEEPINTVL, &value, sizeof(value));

    if (ret < 0)
    {
        char errstr[64] = {0};
        strerror_r(errno, errstr, sizeof(errstr));
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_ERROR, HTTP_LOG_MSG_SOCKET, 1,
                       HTTP_WORKER_FMT"setsockopt(%d) fail: %s",
                       HTTP_WORKER_ARG, sock, errstr);
        return ret;
    }

    return ret;
}

static void http_worker_new_connection(HTTP_Event_T *event)
{
    struct sockaddr_in6     client_addr;
    int                     newsockfd;
    socklen_t               clilen;
    HTTP_Worker_T          *worker;
    HTTP_Connection_T      *http_connection;

    worker = (HTTP_Worker_T *)event->data;

    clilen = sizeof(client_addr);
    newsockfd = accept(event->fd, (struct sockaddr *)&client_addr, &clilen);

    if (newsockfd < 0)
    {
        char errstr[64] = {0};
        strerror_r(errno, errstr, sizeof(errstr));
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_CRIT, HTTP_LOG_MSG_SOCKET, 1,
                       HTTP_WORKER_FMT"accept(%s)",
                       HTTP_WORKER_ARG, errstr);

        worker->close = 1;
        return;
    }

    HTTP_LOG_ERROR(HTTP_LOG_LEVEL_INFO, HTTP_LOG_MSG_CONNECTION, 1,
                   HTTP_WORKER_FMT"New connection, fd(%d)",
                   HTTP_WORKER_ARG, newsockfd);

#if (SYS_CPNT_MGMT_IP_FLT == TRUE && SYS_CPNT_WEBAUTH != TRUE)
    if (HTTP_TASK_IsValidMgmtRemoteAddress(newsockfd) != TRUE)
    {
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_INFO, HTTP_LOG_MSG_CONNECTION, 1,
            HTTP_WORKER_FMT"Disconnect the connection because it doesn't come from authenticated address, fd(%d)",
            HTTP_WORKER_ARG, newsockfd);

        http_worker_send_reply(newsockfd, R_UNAUTHORIZED, "Unauthorized Address");
        HTTP_closesocket(newsockfd);
        return;
    }
#endif /* SYS_CPNT_MGMT_IP_FLT && NOT SYS_CPNT_WEBAUTH */

#if (SYS_CPNT_CLUSTER == TRUE)
    if (HTTP_TASK_GetClusterMemberIp(newsockfd) != TRUE)
    {
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_INFO, HTTP_LOG_MSG_CONNECTION, 1,
            HTTP_WORKER_FMT"Disconnect the connection because it doesn't come from cluster commander, fd(%d)",
            HTTP_WORKER_ARG, newsockfd);

        http_worker_send_reply(newsockfd, R_UNAUTHORIZED, "Unauthorized Address");
        HTTP_closesocket(newsockfd);
        return;
    }
#endif /* SYS_CPNT_CLUSTER */

    if (0 <= newsockfd)
    {
        if (0 != http_worker_set_socket_option(worker, newsockfd))
        {
            HTTP_LOG_ERROR(HTTP_LOG_LEVEL_NOTICE, HTTP_LOG_MSG_CONNECTION, 1,
                           HTTP_WORKER_FMT"Cannot set keep-alive option on socket, disconnect, fd(%d)",
                           HTTP_WORKER_ARG, newsockfd);

            http_worker_send_reply(newsockfd, R_ERROR, "Internal error: cannot set keep-alive option on socket.");
            HTTP_closesocket(newsockfd);
            return;
        }

        http_connection = (HTTP_Connection_T *)HTTP_MGR_AllocateConnectionObject(newsockfd);

        if (NULL == http_connection)
        {
            HTTP_LOG_ERROR(HTTP_LOG_LEVEL_NOTICE, HTTP_LOG_MSG_CONNECTION, 1,
                           HTTP_WORKER_FMT"Connection pool full, disconnect, fd(%d)",
                           HTTP_WORKER_ARG, newsockfd);

            http_worker_send_reply(newsockfd, R_ERROR, "Connection pool full, try again later.");
            HTTP_closesocket(newsockfd);
            return;
        }

        http_connection_onnew(http_connection);

        http_connection->fds[HTTP_CONN_FD_NET] = newsockfd;

        if (event->fd == worker->http_fd)
        {
            http_connection->conn_state = HTTP_CONNECTION;
        }
        else
        {
            ASSERT(event->fd == worker->https_fd);
            http_connection->conn_state = HTTPS_CONNECTION;
        }

        http_scheduler_new_connection(http_connection);
    }
}

static void http_worker_send_reply(int fd, int status, const char *message)
{
#define SEND_TEXT(text) send(fd, text, strlen(text), 0);

    char content_length[100];

    SEND_TEXT("HTTP/1.0 ");

    switch (status)
    {
    case R_BAD_REQUEST:
    {
        SEND_TEXT("400 Bad Request");
        break;
    }
    case R_UNAUTHORIZED:
    {
        SEND_TEXT("401 Unauthorized");
        break;
    }
    case R_FORBIDDEN:
    {
        SEND_TEXT("403 Forbidden");
        break;
    }
    case R_METHOD_NA:
    {
        SEND_TEXT("405 Method Not Allowed");
        break;
    }
    case R_NOT_FOUND:
    {
        SEND_TEXT("404 Not Found");
        break;
    }
    case R_ERROR:
    {
        SEND_TEXT("500 Internal Server Error");
        break;
    }
    default:
        SEND_TEXT("400 Bad Request");
        break;
    }

    SEND_TEXT("\r\n");

    snprintf(content_length, sizeof(content_length), "Content-Length: %d\r\n", strlen("<html><body></body></html>") + strlen(message));
    SEND_TEXT(content_length);
    SEND_TEXT("Content-Type: text/html\r\n\r\n");
    SEND_TEXT("<html><body>");
    SEND_TEXT(message);
    SEND_TEXT("</body></html>");

#undef SEND_TEXT
}