//
//  http_event_select.c
//  http
//
//  Created by JunYing Yeh on 2014/7/10.
//
//

#include "http_loc.h"

typedef struct
{
    HTTP_Event_T    events[HTTP_MAX_EVENT];

    fd_set          read_fd_set;
    fd_set          write_fd_set;

    size_t          max_event;
} HTTP_EVENT_SEL_CTX_T, * HTTP_EVENT_SEL_CTX_PTR_T;

static void http_event_select_pack_events(HTTP_EVENT_SEL_CTX_PTR_T ctx);
static void http_event_select_event_copy(const HTTP_Event_T *src, HTTP_Event_T *dst, struct timeval *tv);

void *http_event_select_new()
{
    HTTP_EVENT_SEL_CTX_PTR_T    context;

    context = (HTTP_EVENT_SEL_CTX_PTR_T)L_MM_Malloc(sizeof(HTTP_EVENT_SEL_CTX_T),
                                                    HTTP_TYPE_TRACE_ID_HTTP_PROCESS_REQUEST);
    if (context != NULL)
    {
        memset(context, 0, sizeof(HTTP_EVENT_SEL_CTX_T));
    }

    return context;
}

void http_event_select_free(void ** context_pp)
{
    HTTP_EVENT_SEL_CTX_PTR_T    ptr;

    ASSERT(context_pp != NULL);
    if (context_pp == NULL)
    {
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_CRIT, HTTP_LOG_MSG_PARAMETER, 1,
                       "context_pp(%s)", "Invalid paameter: null pointer");
        return;
    }

    if (*context_pp == NULL)
    {
        return;
    }

    ptr = * (HTTP_EVENT_SEL_CTX_T **) context_pp;

    L_MM_Free(ptr);
    *context_pp = NULL;
}

#if (HTTP_CFG_EVENT_DEBUG == 1)
int http_event_select_add_event(void *context, const HTTP_Event_T *ev, size_t timer_sec, const char *function)
#else
int http_event_select_add_event(void *context, const HTTP_Event_T *ev, size_t timer_sec)
#endif
{
    HTTP_EVENT_SEL_CTX_PTR_T    ctx = (HTTP_EVENT_SEL_CTX_PTR_T) context;
    struct timeval              timer = {HTTP_TIMER_INFINITE, 0};

    int i;

    ASSERT(ev != NULL);
    ASSERT(ev->data != NULL);

    if (ev->handler == NULL)
    {
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_CRIT, HTTP_LOG_MSG_PARAMETER, 1,
                       "ev.handler(%s)", "Invalid paameter: null pointer");
        return -1;
    }

    if (timer_sec != HTTP_TIMER_INFINITE)
    {
        struct timeval now;
        struct timeval tv;
        tv.tv_sec = timer_sec;
        tv.tv_usec = 0;

        gettimeofday(&now, NULL);
        timeradd(&now, &tv, &timer);
    }

    for (i = 0; i < ctx->max_event; ++ i)
    {
        if (ctx->events[i].handler)
        {
            if (ctx->events[i].fd == ev->fd && ctx->events[i].event_type == ev->event_type)
            {
                if (ev->event_type == HTTP_EVENT_READ)
                {
                    FD_SET(ev->fd, &ctx->read_fd_set);
                }
                else if (ev->event_type == HTTP_EVENT_WRITE)
                {
                    FD_SET(ev->fd, &ctx->write_fd_set);
                }

                http_event_select_event_copy(ev, &ctx->events[i], &timer);

#if (HTTP_CFG_EVENT_DEBUG == 1)
                strncpy(ctx->events[i].function, function, sizeof(ctx->events[i].function) - 1);
                ctx->events[i].function[sizeof(ctx->events[i].function) - 1] = '\0';

                HTTP_LOG_ERROR(HTTP_LOG_LEVEL_DEBUG, HTTP_LOG_MSG_EVENT, 1,
                               "%s: Replace event[%d], fd(%d)", function, i, ev->fd);
#endif /* HTTP_CFG_EVENT_DEBUG */

                http_event_select_dbg_check(ctx, HTTP_EVENT_SEL_CHECK_IGN_REMOVE_BIT);

                return 0;
            }
        }
    }

    if (ctx->max_event == _countof(ctx->events))
    {
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_INFO, HTTP_LOG_MSG_EVENT, 1,
                       "%s", "Event full");
        ASSERT(0);
        return -1;
    }

    ASSERT(ctx->events[ctx->max_event].handler == NULL);

    if (ev->event_type == HTTP_EVENT_READ)
    {
        ASSERT(!FD_ISSET(ev->fd, &ctx->read_fd_set));
        FD_SET(ev->fd, &ctx->read_fd_set);
    }
    else if (ev->event_type == HTTP_EVENT_WRITE)
    {
        ASSERT(!FD_ISSET(ev->fd, &ctx->write_fd_set));
        FD_SET(ev->fd, &ctx->write_fd_set);
    }

    http_event_select_event_copy(ev, &ctx->events[ctx->max_event], &timer);

#if (HTTP_CFG_EVENT_DEBUG == 1)
    strncpy(ctx->events[i].function, function, sizeof(ctx->events[i].function) - 1);
    ctx->events[i].function[sizeof(ctx->events[i].function) - 1] = '\0';

    HTTP_LOG_ERROR(HTTP_LOG_LEVEL_DEBUG, HTTP_LOG_MSG_EVENT, 1,
                   "%s: Add event[%lu], fd(%d)", function, ctx->max_event, ev->fd);
#endif /* HTTP_CFG_EVENT_DEBUG */

    ++ ctx->max_event;

    http_event_select_dbg_check(ctx, HTTP_EVENT_SEL_CHECK_IGN_REMOVE_BIT);

    return 0;
}

int http_event_select_del_event(void *context, const HTTP_Event_T *ev)
{
    HTTP_EVENT_SEL_CTX_PTR_T    ctx = (HTTP_EVENT_SEL_CTX_PTR_T)context;
    int found = -1;

    ASSERT(ev != NULL);

    {
        size_t  i;

        for (i = 0; i < ctx->max_event; ++ i)
        {
            if (ctx->events[i].fd == ev->fd && ctx->events[i].event_type == ev->event_type)
            {
                found = i;
                break;
            }
        }
    }

    if (found != -1)
    {
        ASSERT(0 < ctx->max_event);
        ASSERT(0 <= found);

        ASSERT(ctx->events[found].fd == ev->fd &&
               ctx->events[found].event_type == ev->event_type);

        if (ctx->events[found].event_type == HTTP_EVENT_READ)
        {
            FD_CLR(ctx->events[found].fd, &ctx->read_fd_set);
        }
        else if (ctx->events[found].event_type == HTTP_EVENT_WRITE)
        {
            FD_CLR(ctx->events[found].fd, &ctx->write_fd_set);
        }

        if (found != ctx->max_event -1)
        {
            ctx->events[found] = ctx->events[ctx->max_event - 1];
        }

        memset(&ctx->events[ctx->max_event - 1], 0, sizeof(ctx->events[0]));
        ctx->max_event -= 1;

#if (HTTP_CFG_EVENT_DEBUG == 1)
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_DEBUG, HTTP_LOG_MSG_EVENT, 1,
                       "Delete event[%d], fd(%d)", found, ev->fd);
#endif /* HTTP_CFG_EVENT_DEBUG */
    }

    http_event_select_dbg_check(ctx, HTTP_EVENT_SEL_CHECK_ALL);
    return 0;
}

int http_event_select_set_removeable(void *context, const HTTP_Event_T *ev)
{
    HTTP_EVENT_SEL_CTX_PTR_T    ctx = (HTTP_EVENT_SEL_CTX_PTR_T)context;
    int found = -1;

    ASSERT(ev != NULL);

    {
        size_t  i;

        for (i = 0; i < ctx->max_event; ++ i)
        {
            if (ctx->events[i].fd == ev->fd && ctx->events[i].event_type == ev->event_type)
            {
                found = i;
                break;
            }
        }
    }

    if (found != -1 && ctx->events[found].remove == 0)
    {
        ASSERT((size_t)found < ctx->max_event);

        if (ctx->events[found].event_type == HTTP_EVENT_READ)
        {
            ASSERT(FD_ISSET(ctx->events[found].fd, &ctx->read_fd_set));
            FD_CLR(ctx->events[found].fd, &ctx->read_fd_set);
        }
        else if (ev->event_type == HTTP_EVENT_WRITE)
        {
            ASSERT(FD_ISSET(ctx->events[found].fd, &ctx->read_fd_set));
            FD_CLR(ctx->events[found].fd, &ctx->write_fd_set);
        }

        ctx->events[found].remove = 1;

#if (HTTP_CFG_EVENT_DEBUG == 1)
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_DEBUG, HTTP_LOG_MSG_EVENT, 1,
                       "Set removeable[%d], fd(%d)", found, ev->fd);
#endif /* HTTP_CFG_EVENT_DEBUG */
    }

    return 0;
}

// 1. get current time
// 2. for each event |ev| do
//        if ev is timeout
//            call ev.handler and then remove this event
//
// 3. min_offset = INIF
//    for each event |ev| do
//        if max_fd < ev.fd
//            max_fd = ev.fd
//        if (offset = now - ev) < min_offset
//            min_offset = offset
// 4. select(max_fd, ..., min_offset)

int http_event_select_process_timers(void *context)
{
    HTTP_EVENT_SEL_CTX_PTR_T    ctx = (HTTP_EVENT_SEL_CTX_PTR_T)context;

    HTTP_Event_T                *ev;
    struct timeval              now;
    int                         i;

    gettimeofday(&now, NULL);

    if (ctx->max_event == 0)
    {
        return 0;
    }

    for (i = ctx->max_event - 1; 0 <= i; -- i)
    {
        ev = &ctx->events[i];

        if (ev->remove == 1)
        {
            continue;
        }

        /* Skip the deleted event
         */
        if (ctx->max_event <= i)
        {
            continue;
        }

        ASSERT(ev->handler != NULL);

        if (ev->tv.tv_sec == HTTP_TIMER_INFINITE)
        {
            continue;
        }

        if (timercmp(&ev->tv, &now, <=))
        {
            HTTP_Event_T    new_ev = *ev;

#if (HTTP_CFG_EVENT_DEBUG == 1)
            HTTP_LOG_ERROR(HTTP_LOG_LEVEL_DEBUG, HTTP_LOG_MSG_EVENT, 1,
                           "%s >> call event[%i].handler for timeout, fd(%d)",
                           new_ev.function,
                           i, new_ev.fd);

            ev->last_access_time = now;
#endif /* HTTP_CFG_EVENT_DEBUG */

            ASSERT(ev->remove == 0);
            new_ev.remove = 1;

            new_ev.timeout = 1;

            if (new_ev.timeout_fn)
            {
                new_ev.timeout_fn(&new_ev);
            }
            else
            {
                new_ev.handler(&new_ev);
            }

            if (ev->remove == 1 && new_ev.remove == 1)
            {
                http_event_select_set_removeable(ctx, ev);
            }
            else if (new_ev.remove == 0)
            {
                struct timeval fixme = { HTTP_CFG_LONG_POLLING_TIMEOUT_SEC, 0 };

                timeradd(&now, &fixme, &ev->tv);
            }
        }
    }

    http_event_select_pack_events(ctx);

    return 0;
}

int http_event_select_process_events(void *context, size_t timer_ms)
{
    HTTP_EVENT_SEL_CTX_PTR_T    ctx = (HTTP_EVENT_SEL_CTX_PTR_T)context;

    struct timeval              now;

    struct timeval              tv;

    int                         max_fd = 0;
    int                         ready;
    int                         i;
    HTTP_Event_T                *ev;
    fd_set                      read_fd_set;
    fd_set                      write_fd_set;

    http_event_select_pack_events(ctx);

    gettimeofday(&now, NULL);

    tv.tv_sec  = timer_ms / 1000;
    tv.tv_usec = timer_ms % 1000;

    // FIXME: don't put here ?
    http_event_select_process_timers(context);

    for (i = 0; i < ctx->max_event; i++)
    {
        struct timeval dt_tv;

        ev = &ctx->events[i];

        ASSERT(ev->handler);

        if (ev->remove == 1)
        {
            continue;
        }

        if (max_fd < ev->fd)
        {
            max_fd = ev->fd; // TODO: keep this value to ctx to save time
        }

        if (ev->tv.tv_sec == HTTP_TIMER_INFINITE)
        {
            continue;
        }

        if (timercmp(&ev->tv, &now, <=))
        {
            timerclear(&tv);
        }
        else
        {
            timersub(&ev->tv, &now, &dt_tv);

            if (timercmp(&dt_tv, &tv, <))
            {
                tv = dt_tv;

                ASSERT(0 < tv.tv_sec || 0 < tv.tv_usec);
            }
        }
    }

//#if (HTTP_CFG_EVENT_DEBUG == 1)
//    HTTP_LOG_ERROR(HTTP_LOG_LEVEL_DEBUG, HTTP_LOG_MSG_EVENT, 1,
//                   "update the min timer(%d sec, %d usec)",
//                   tv.tv_sec, tv.tv_usec);
//#endif /* HTTP_CFG_EVENT_DEBUG */

    read_fd_set = ctx->read_fd_set;
    write_fd_set = ctx->write_fd_set;

    if (max_fd)
    {
        ready = select(max_fd + 1, &read_fd_set, &write_fd_set, 0, &tv);
    }
    else
    {
        // TODO: We need using the spying for SYSFUN ? Then it can use
        //       different behavior under test and real daemon.
        // TODO: Don't depend to SYSFUN ?? Because it can not cross platform !
#ifdef OS_WINDOWS
        Sleep(tv.tv_sec * 1000 + tv.tv_usec / 1000);
        ready = 0;
#else
        ready = select(0, 0, 0, 0, &tv);
#endif
    }

    if (ready < 0)
    {
        char errstr[64] = {0};
        strerror_r(errno, errstr, sizeof(errstr));
        HTTP_LOG_ERROR(HTTP_LOG_LEVEL_CRIT, HTTP_LOG_MSG_SOCKET, 1,
                       "select(%s)", errstr);

        return -1;
    }

    if (ready == 0)
    {
        return 0;
    }

    for (i = 0; i < ctx->max_event; ++ i)
    {
        ev = &ctx->events[i];
        if (!ev->handler)
        {
            continue;
        }

        if (ev->event_type == HTTP_EVENT_READ)
        {
            if (FD_ISSET(ev->fd, &read_fd_set))
            {
                HTTP_Event_T new_ev = *ev;

                if (new_ev.remove)
                {
                    continue;
                }

#if (HTTP_CFG_EVENT_DEBUG == 1)
                HTTP_LOG_ERROR(HTTP_LOG_LEVEL_DEBUG, HTTP_LOG_MSG_EVENT, 1,
                               "%s >> call event[%i].handler for reading, fd(%d)",
                               new_ev.function,
                               i, new_ev.fd);

                ev->last_access_time = now;
#endif /* HTTP_CFG_EVENT_DEBUG */

                new_ev.ready = 1;
                new_ev.handler(&new_ev);

                if (new_ev.remove == 1)
                {
                    http_event_select_set_removeable(ctx, ev);
                }
            }
        }
        else if (ev->event_type == HTTP_EVENT_WRITE)
        {
            if (FD_ISSET(ev->fd, &write_fd_set))
            {
                HTTP_Event_T new_ev = *ev;

                if (new_ev.remove)
                {
                    continue;
                }

#if (HTTP_CFG_EVENT_DEBUG == 1)
                HTTP_LOG_ERROR(HTTP_LOG_LEVEL_DEBUG, HTTP_LOG_MSG_EVENT, 1,
                               "%s >> call event[%i].handler for writting, fd(%d)",
                               new_ev.function,
                               i, new_ev.fd);

                ev->last_access_time = now;
#endif /* HTTP_CFG_EVENT_DEBUG */

                new_ev.ready = 1;
                new_ev.handler(&new_ev);

                if (new_ev.remove == 1)
                {
                    http_event_select_set_removeable(ctx, ev);
                }
            }
        }
    }

    http_event_select_pack_events(ctx);
    return 0;
}

static void http_event_select_pack_events(HTTP_EVENT_SEL_CTX_PTR_T ctx)
{
    size_t                      i;
    HTTP_Event_T                *ev;

    for (i = 0; i < ctx->max_event; )
    {
        ev = &ctx->events[i];
        if (ev->remove)
        {
#if (HTTP_CFG_EVENT_DEBUG == 1)
            HTTP_LOG_ERROR(HTTP_LOG_LEVEL_DEBUG, HTTP_LOG_MSG_EVENT, 1,
                           "Delete event[%d], fd(%d)", i, ev->fd);
#endif /* HTTP_CFG_EVENT_DEBUG */

            if (ev->event_type == HTTP_EVENT_READ)
            {
                FD_CLR(ev->fd, &ctx->read_fd_set);
            }
            else if (ev->event_type == HTTP_EVENT_WRITE)
            {
                FD_CLR(ev->fd, &ctx->write_fd_set);
            }

            if (i != ctx->max_event - 1)
            {
                *ev = ctx->events[ctx->max_event - 1];
            }

            memset(&ctx->events[ctx->max_event - 1], 0, sizeof(ctx->events[0]));

            ASSERT(0 < ctx->max_event);
            ctx->max_event -= 1;

            continue;
        }

        ++ i;
    }

    http_event_select_dbg_check(ctx, HTTP_EVENT_SEL_CHECK_ALL);
}

static void http_event_select_event_copy(const HTTP_Event_T *src, HTTP_Event_T *dst, struct timeval *tv)
{
    ASSERT(src != NULL);
    ASSERT(dst != NULL);

    memset(dst, 0, sizeof(*dst));

    dst->event_type = src->event_type;
    dst->fd = src->fd;
    dst->data = src->data;

    dst->tv = *tv;

    dst->handler = src->handler;
    dst->timeout_fn = src->timeout_fn;
}

#if (HTTP_CFG_EVENT_DEBUG == 1)
void http_event_select_dbg_check(const void *context, int flags)
{
    HTTP_EVENT_SEL_CTX_PTR_T    ctx = (HTTP_EVENT_SEL_CTX_PTR_T)context;

    {
        size_t  i;

        for (i = 0; i < _countof(ctx->events); ++ i)
        {
            if (i < ctx->max_event)
            {
                ASSERT(0 <= ctx->events[i].fd);
                ASSERT(ctx->events[i].handler != NULL);

                if (flags != HTTP_EVENT_SEL_CHECK_IGN_REMOVE_BIT)
                {
                    ASSERT(ctx->events[i].remove == 0);
                }
            }
            else
            {
                ASSERT(ctx->events[i].handler == NULL);
            }
        }
    }

    {
        int max_event_idx = -1;
        int i;
        for (i = 0; i < _countof(ctx->events); ++ i)
        {
            if (ctx->events[i].handler != 0)
            {
                max_event_idx = i;
            }
        }

        ASSERT((max_event_idx == -1 && ctx->max_event == 0) ||
               (0 <= max_event_idx && max_event_idx == ctx->max_event - 1));
        ASSERT(ctx->max_event <= _countof(ctx->events));
    }

    {
        size_t  i;

        for (i = 0; i < ctx->max_event; ++ i)
        {
            ASSERT(0 <= ctx->events[i].tv.tv_sec);
            ASSERT(0 <= ctx->events[i].tv.tv_usec);
            ASSERT(ctx->events[i].ready == 0);
            ASSERT(ctx->events[i].timeout == 0);

            if (flags != HTTP_EVENT_SEL_CHECK_IGN_REMOVE_BIT)
            {
                ASSERT(ctx->events[i].remove == 0);
            }
        }
    }

#ifdef _MSC_VER
    if (flags != HTTP_EVENT_SEL_CHECK_IGN_REMOVE_BIT)
    {
        ASSERT(ctx->max_event == ctx->read_fd_set.fd_count + ctx->write_fd_set.fd_count);
    }
#endif

    if (flags != HTTP_EVENT_SEL_CHECK_IGN_REMOVE_BIT)
    {
        size_t  i;

        fd_set  read_fd_set  = ctx->read_fd_set;
        fd_set  write_fd_set = ctx->write_fd_set;
        fd_set  nil_set;

        FD_ZERO(&nil_set);

        for (i = 0; i < ctx->max_event; ++ i)
        {
            if (ctx->events[i].event_type == HTTP_EVENT_READ)
            {
                ASSERT((ctx->events[i].remove == 0 && FD_ISSET(ctx->events[i].fd, &read_fd_set)) ||
                       (ctx->events[i].remove == 1 && !FD_ISSET(ctx->events[i].fd, &read_fd_set)));
                FD_CLR(ctx->events[i].fd, &read_fd_set);
            }
            else
            {
                ASSERT((ctx->events[i].remove == 0 && FD_ISSET(ctx->events[i].fd, &write_fd_set)) ||
                    (ctx->events[i].remove == 1 && !FD_ISSET(ctx->events[i].fd, &write_fd_set)));
                FD_CLR(ctx->events[i].fd, &write_fd_set);
            }
        }

#ifdef _MSC_VER
        ASSERT(read_fd_set.fd_count == 0);
        ASSERT(write_fd_set.fd_count == 0);
#else
        ASSERT(memcmp(&read_fd_set, &nil_set, sizeof(read_fd_set)) == 0);
        ASSERT(memcmp(&write_fd_set, &nil_set, sizeof(write_fd_set)) == 0);
#endif
    }

    {
        size_t          i;

        struct timeval  now;
        struct timeval  dt;
        struct timeval  max_alive_time = {60, 0};

        gettimeofday(&now, NULL);

        for (i = 0; i < ctx->max_event; ++ i)
        {
            if (timercmp(&ctx->events[i].tv, &now, <))
            {
                timersub(&now, &ctx->events[i].tv, &dt);

                ASSERT(timercmp(&dt, &max_alive_time, <));
            }
        }
    }
}
#endif /* HTTP_CFG_EVENT_DEBUG */
