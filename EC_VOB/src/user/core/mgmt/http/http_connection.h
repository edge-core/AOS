//
//  http_connection.h
//  http
//
//  Created by JunYing Yeh on 2014/7/6.
//
//

#ifndef _HTTP_HEADER_CONNECTION_H_
#define _HTTP_HEADER_CONNECTION_H_

#include "http_type.h"
#include "http_config.h"

#if __cplusplus
extern "C" {
#endif

void http_connection_init(HTTP_Event_T *event);
void http_connection_reuse(HTTP_Event_T *event);
void http_connection_finalize(HTTP_Event_T *event);

void http_connection_onnew(HTTP_Connection_T *http_connection);
void http_connection_onreuse(HTTP_Connection_T *http_connection);
void http_connection_onclose(HTTP_Connection_T *http_connection);

#if (HTTP_CFG_CONNECTION_DEBUG == 1)
void http_connection_dbg_check(HTTP_Connection_T *http_connection);
#else
#define http_connection_dbg_check(c)
#endif

#if __cplusplus
}
#endif

#endif /* _HTTP_HEADER_CONNECTION_H_ */
