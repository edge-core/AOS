//
//  http_event_select.h
//  http
//
//  Created by JunYing Yeh on 2014/7/10.
//
//

#ifndef _HTTP_HEADER_EVENT_SELECT_H_
#define _HTTP_HEADER_EVENT_SELECT_H_

#include "http_type.h"
#include "http_config.h"

#if __cplusplus
extern "C" {
#endif

enum
{
    HTTP_EVENT_SEL_CHECK_IGN_REMOVE_BIT,
    HTTP_EVENT_SEL_CHECK_ALL
};

void *http_event_select_new();
void http_event_select_free(void ** context_pp);

#if (HTTP_CFG_EVENT_DEBUG == 1)
int http_event_select_add_event(void *context, const HTTP_Event_T *ev, size_t timer_sec, const char *function);
#else
int http_event_select_add_event(void *context, const HTTP_Event_T *ev, size_t timer_sec);
#endif

int http_event_select_del_event(void *context, const HTTP_Event_T *ev);
int http_event_select_set_removeable(void *context, const HTTP_Event_T *ev);

int http_event_select_process_events(void *context, size_t timer_ms);

#if (HTTP_CFG_EVENT_DEBUG == 1)
void http_event_select_dbg_check(const void *context, int flags);
#else
#define http_event_select_dbg_check(c, f)
#endif

#if __cplusplus
}
#endif

#endif
