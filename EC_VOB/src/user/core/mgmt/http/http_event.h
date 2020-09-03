//
//  http_event.h
//  http
//
//  Created by JunYing Yeh on 2014/7/10.
//
//

#ifndef _HTTP_HEADER_EVENT_H_
#define _HTTP_HEADER_EVENT_H_

#include "http_type.h"
#include "http_event_select.h"

#define http_event_new_context()            http_event_select_new()
#define http_event_free_context(pp)         http_event_select_free(pp)

#if (HTTP_CFG_EVENT_DEBUG == 1)
#  define http_event_add(c, ev, t)          http_event_select_add_event(c, ev, t, __FUNCTION__)
#else
#  define http_event_add(c, ev, t)          http_event_select_add_event(c, ev, t)
#endif

#define http_event_delete(c, ev)            http_event_select_del_event(c, ev)
#define http_event_set_removeable(c, ev)    http_event_select_set_removeable(c, ev)

#define http_event_rocess_events(c, ms)     http_event_select_process_events(c, ms)

#define http_event_dbg_check(c)             http_event_select_dbg_check_ctx(c, HTTP_EVENT_SEL_CHECK_ALL)

#if __cplusplus
extern "C" {
#endif

#if __cplusplus
}
#endif

#endif
