//
//  http_connection.c
//  http
//
//  Created by JunYing Yeh on 2014/7/6.
//
//

#include "http_loc.h"

#define s_close(s) HTTP_closesocket(s)

static void http_connection_close(HTTP_Connection_T *http_connection);

void http_connection_init(HTTP_Event_T *event)
{
    int                 ret;
    HTTP_Connection_T   *http_connection;
    int                 socks[2];

    ASSERT(event != NULL);

    http_connection = (HTTP_Connection_T *) event->data;

    ASSERT(http_connection != NULL);

    ASSERT(0 < http_connection->fds[HTTP_CONN_FD_NET]);
    ASSERT(http_connection->fds[HTTP_CONN_FD_SYSCB_SND] == 0);
    ASSERT(http_connection->fds[HTTP_CONN_FD_SYSCB_RCV] == 0);

    ASSERT(http_connection->req == NULL);
    ASSERT(http_connection->res == NULL);

    HTTP_LOG_ERROR(HTTP_LOG_LEVEL_INFO, HTTP_LOG_MSG_CONNECTION, 1,
        "Init connection, fd(%d)",
        http_connection->fds[HTTP_CONN_FD_NET]);

    ret = socketpair(AF_LOCAL, SOCK_STREAM, 0, socks);

    if (ret != 0)
    {
        char errstr[64] = {0};
        strerror_r(errno, errstr, sizeof(errstr));
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_CRIT, HTTP_LOG_MSG_SOCKET, 1,
                       "socketpair(%s)", errstr);

        goto fail;
    }

    http_connection->fds[HTTP_CONN_FD_SYSCB_SND] = socks[0];
    http_connection->fds[HTTP_CONN_FD_SYSCB_RCV] = socks[1];

    if (http_connection->req == NULL)
    {
        http_connection->req = http_request_new();
        if (http_connection->req == NULL)
        {
            goto fail;
        }
    }

    // TODO: Move the code in http_connection_init for allocating response memory
    //       as http_init_request
    if (http_connection->res == NULL)
    {
        http_connection->res = (HTTP_Response_T *) L_MM_Malloc(sizeof(HTTP_Response_T),
                                                               L_MM_USER_ID2(SYS_MODULE_HTTP,
                                                                             HTTP_TYPE_TRACE_ID_HTTP_PROCESS_REQUEST));
        if (http_connection->res == NULL)
        {
            HTTP_LOG_ERROR(HTTP_LOG_LEVEL_CRIT, HTTP_LOG_MSG_MEMORY, 1,
                           "%s", "res.envcfg(Out of memory)");
            goto fail;
        }

        memset(http_connection->res, 0, sizeof(HTTP_Response_T));
    }

    http_connection_dbg_check(http_connection);

    event->handler = http_connection_reuse;
    http_connection_reuse(event);

    return;

fail:
    http_connection->done = 1;
    http_connection->keepalive = 0;

    event->handler = http_connection_finalize;
    http_connection_finalize(event);
}

void http_connection_reuse(HTTP_Event_T *event)
{
    HTTP_Connection_T  *http_connection;
    HTTP_Worker_T      *worker;
    int                 i;

    ASSERT(event != NULL);

    http_connection = (HTTP_Connection_T *)event->data;

    ASSERT(http_connection != NULL);
    http_connection_dbg_check(http_connection);

    worker = http_connection->worker;

    HTTP_LOG_ERROR(HTTP_LOG_LEVEL_INFO, HTTP_LOG_MSG_CONNECTION, 1,
                   "Reuse connection, fd(%d)",
                   http_connection->fds[HTTP_CONN_FD_NET]);

    for (i = 0; i < _countof(http_connection->fds); ++ i)
    {
        if (0 <= http_connection->fds[i])
        {
            HTTP_Event_T _ev;

            memset(&_ev, 0, sizeof(_ev));

            _ev.event_type = HTTP_EVENT_READ;
            _ev.fd = http_connection->fds[i];
            http_event_set_removeable(worker->event_ctx, &_ev);
        }
    }

    http_connection->done = 0;
    http_connection->keepalive = 1;
    http_connection->special_workaround = 0;
    ASSERT(http_connection->wait_syscb == 0);
    http_connection->wait_syscb = 0;

#if (SYS_CPNT_CLUSTER == TRUE)
    if (0 <= http_connection->socket_relay_id)
    {
        s_close(http_connection->socket_relay_id);
        http_connection->socket_relay_id = -1;
    }

    http_connection->is_relaying = FALSE;
#endif /* SYS_CPNT_CLUSTER */

    // TODO: Move the code in http_connection_reuse for reset request memory
    //       as http_xxx_request
    ASSERT(http_connection->req != NULL);

    http_connection->req->connection = http_connection;
    http_connection->res->connection = http_connection;

    http_connection->req->status = R_REQUEST_OK;

    memset(http_connection->req->request_uri, 0, sizeof(http_connection->req->request_uri));
    memset(http_connection->req->header_in, 0, sizeof(http_connection->req->header_in));

    http_connection->req->header_length = 0;

    http_connection->req->header_idx = 0;
    http_connection->req->header_end_idx = 0;
    http_connection->req->body_cnt = 0;
    http_connection->req->body_index = 0;
    http_connection->req->access_privilege = 0;
    http_connection->req->multipart = 0;

    ASSERT(http_connection->req->envcfg != NULL);
    ASSERT(http_connection->req->query != NULL);

    free_env(http_connection->req->envcfg);
    free_env(http_connection->req->query);

    // TODO: Move the code in http_connection_reuse for reset response memory
    //       as http_xxx_response
    ASSERT(http_connection->res != NULL);

    memset(http_connection->res->send_buf, 0, sizeof(http_connection->res->send_buf));
    http_connection->res->send_len = 0;
    http_connection->res->transfer_encoding = HTTP_TRANSFER_ENCODING_CHUNKED;
    http_connection->res->is_send_eof = 0;

    if (http_connection->res->headers_out.content_type_n)
    {
        json_decref(http_connection->res->headers_out.content_type_n);
        http_connection->res->headers_out.content_type_n = NULL;
    }

    if (http_connection->res->headers_out.cookies_n)
    {
        json_decref(http_connection->res->headers_out.cookies_n);
        http_connection->res->headers_out.cookies_n = NULL;
    }

    memset(&http_connection->res->headers_out, 0, sizeof(http_connection->res->headers_out));

    http_connection->req->major_version = HTTP_DFLT_VERSION_MAJOR_VERSION;
    http_connection->req->minor_version = HTTP_DFLT_VERSION_MINOR_VERSION;

    http_connection->req->fd = http_connection->fds[HTTP_CONN_FD_NET];
    http_connection->res->fd = http_connection->fds[HTTP_CONN_FD_NET];

    L_BIO_socket_init((bio_socket_impl_t*)BIO_SOCKET_OS_DEPENDENT, &http_connection->req->bfd);
    L_BIO_socket_assign_fd(&http_connection->req->bfd, http_connection->req->fd);

    L_BIO_socket_init((bio_socket_impl_t*)BIO_SOCKET_OS_DEPENDENT, &http_connection->res->bfd);
    L_BIO_socket_assign_fd(&http_connection->res->bfd, http_connection->res->fd);

    {
        bio_socket_option_t option = {0};

        option.optname = SO_RCVTIMEO;
        option.optval.timeout.milliseconds = 20000;

        if (HTTP_socket_set_option(&http_connection->req->bfd, &option) != 0)
        {
            char errstr[64] = {0};
            strerror_r(errno, errstr, sizeof(errstr));
            HTTP_LOG_ERROR(HTTP_LOG_LEVEL_DEBUG, HTTP_LOG_MSG_SOCKET, 1,
                           "setsockopt(%s)", errstr);

            goto fail;
        }

        option.optname = SO_SNDTIMEO;
        option.optval.timeout.milliseconds = 20000;

        if (HTTP_socket_set_option(&http_connection->res->bfd, &option) != 0)
        {
            char errstr[64] = {0};
            strerror_r(errno, errstr, sizeof(errstr));
            HTTP_LOG_ERROR(HTTP_LOG_LEVEL_DEBUG, HTTP_LOG_MSG_SOCKET, 1,
                           "setsockopt(%s)", errstr);

            goto fail;
        }
    }

    http_connection_dbg_check(http_connection);

    ASSERT(event->data != NULL);

    event->event_type = HTTP_EVENT_READ;
    event->fd = http_connection->fds[HTTP_CONN_FD_NET];
    event->data = http_connection;
    event->handler = http_process_request;
    http_event_add(http_connection->worker->event_ctx, event, HTTP_TIMER_INFINITE);
    return;

fail:
    http_connection->done = 1;
    http_connection->keepalive = 0;

    event->handler = http_connection_finalize;
    http_connection_finalize(event);
}

void http_connection_finalize(HTTP_Event_T *event)
{
    HTTP_Connection_T   *http_connection = (HTTP_Connection_T*) event->data;

    HTTP_LOG_ERROR(HTTP_LOG_LEVEL_INFO, HTTP_LOG_MSG_CONNECTION, 1,
        "Finalize connection, fd(%d)",
        http_connection->fds[HTTP_CONN_FD_NET]);

    if (event->data == NULL)
    {
        ASSERT(0);
        return;
    }

    http_connection_dbg_check(http_connection);

    if (http_connection->special_workaround)
    {
        goto leave_no_need_free;
    }

    if (!http_connection->done)
    {
        return;
    }

    if (!http_connection->keepalive)
    {
        goto end;
    }

    http_connection_onclose(http_connection);
    http_connection_onreuse(http_connection);

    event->handler = http_connection_reuse;
    http_connection_reuse(event);

    return;

end:
    http_connection_close(http_connection);
    event->data = NULL; // FIXME: ?? It shall not need to check this, event->data must be aviable

leave_no_need_free:
    MM_kill(SYSFUN_TaskIdSelf());
}

/**----------------------------------------------------------------------
 * Close http_connection. All resource (include socket) will be cleanup.
 * ---------------------------------------------------------------------- */
static void http_connection_close(HTTP_Connection_T *http_connection)
{
    HTTP_Worker_T  *worker;
    int             i;

    http_connection_dbg_check(http_connection);

    HTTP_LOG_ERROR(HTTP_LOG_LEVEL_INFO, HTTP_LOG_MSG_CONNECTION, 1,
        "release connection, fd(%d)",
        http_connection->fds[HTTP_CONN_FD_NET]);

    ASSERT(http_connection->done == 1);

    cgi_main_close(http_connection);

    ASSERT((http_connection->req == NULL) ||
           ((http_connection->req->multipart_data == NULL) &&
            (http_connection->req->boundary == NULL) &&
            (http_connection->req->user_ctx == NULL)));

    ASSERT((http_connection->res == NULL) ||
           (http_connection->res->user_ctx == NULL));

    http_connection_dbg_check(http_connection);

    worker = http_connection->worker;

    for (i = 0; i < _countof(http_connection->fds); ++ i)
    {
        if (0 <= http_connection->fds[i])
        {
            HTTP_Event_T _ev;

            memset(&_ev, 0, sizeof(_ev));

            _ev.event_type = HTTP_EVENT_READ;
            _ev.fd = http_connection->fds[i];
            http_event_set_removeable(worker->event_ctx, &_ev);
        }
    }

    http_request_free(&http_connection->req);

    if (http_connection->res)
    {
        if (http_connection->res->headers_out.content_type_n)
        {
            json_decref(http_connection->res->headers_out.content_type_n);
            http_connection->res->headers_out.content_type_n = NULL;
        }

        if (http_connection->res->headers_out.cookies_n)
        {
            json_decref(http_connection->res->headers_out.cookies_n);
            http_connection->res->headers_out.cookies_n = NULL;
        }

        L_MM_Free(http_connection->res);
        http_connection->res = NULL;
    }

#if (SYS_CPNT_CLUSTER == TRUE)
    if (0 <= http_connection->socket_relay_id)
    {
        s_close(http_connection->socket_relay_id);
        http_connection->socket_relay_id = -1;
    }

    http_connection->is_relaying = FALSE;
#endif /* SYS_CPNT_CLUSTER */

    for (i = 0; i < _countof(http_connection->fds); ++ i)
    {
        if (0 <= http_connection->fds[i])
        {
            shutdown(http_connection->fds[i], SHUT_WR);
            s_close(http_connection->fds[i]);

            if (i != HTTP_CONN_FD_NET)
            {
                http_connection->fds[i] = -1;
            }
        }
    }

    http_connection_onclose(http_connection);

    HTTP_LIST_Remove(&worker->connections, &http_connection->super);

    HTTP_MGR_FreeConnectionObject(http_connection);
}

#if (HTTP_CFG_CONNECTION_DEBUG == 1)
void http_connection_dbg_check(HTTP_Connection_T *http_connection)
{
#if __LP64__
    void *freed_ptr = (void *) 0xCCCCCCCCCCCCCCCC;
#else
    void *freed_ptr = (void *) 0xCCCCCCCC;
#endif

    ASSERT(http_connection != NULL);

    ASSERT(0 < http_connection->fds[HTTP_CONN_FD_NET]);
    ASSERT(0 < http_connection->fds[HTTP_CONN_FD_SYSCB_RCV]);
    ASSERT(0 < http_connection->fds[HTTP_CONN_FD_SYSCB_SND]);

    ASSERT(http_connection->req != NULL);
    ASSERT(http_connection->req != freed_ptr);

    ASSERT(http_connection->res != NULL);
    ASSERT(http_connection->res != freed_ptr);

    ASSERT(http_connection->worker != NULL);
    ASSERT(http_connection->worker->event_ctx != NULL);
}
#endif /* HTTP_CFG_CONNECTION_DEBUG */

void http_connection_onnew(HTTP_Connection_T *http_connection)
{
    ASSERT(http_connection != NULL);

    http_connection->stat.total_request ++;
    gettimeofday(&http_connection->stat.start_time, NULL);
}

void http_connection_onreuse(HTTP_Connection_T *http_connection)
{
    ASSERT(http_connection != NULL);

    http_connection->stat.total_request ++;
    http_connection->stat.total_reused ++;

    gettimeofday(&http_connection->stat.start_time, NULL);
}

void http_connection_onclose(HTTP_Connection_T *http_connection)
{
    struct timeval now;
    struct timeval spend;

    ASSERT(http_connection != NULL);

    gettimeofday(&now, NULL);

    ASSERT(timercmp(&now, &http_connection->stat.start_time, >=));

    timersub(&now, &http_connection->stat.start_time, &spend);
    timeradd(&http_connection->stat.total_process, &spend,
             &http_connection->stat.total_process);

    HTTP_LOG_ERROR(HTTP_LOG_LEVEL_INFO, HTTP_LOG_MSG_CONNECTION, 1,
                   "[%d] spend time(%ld sec %d usec)",
                   http_connection->fds[HTTP_CONN_FD_NET],
                   spend.tv_sec, spend.tv_usec);
}
