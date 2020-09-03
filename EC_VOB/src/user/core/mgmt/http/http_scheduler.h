//
//  http_scheduler.h
//  http
//
//  Created by JunYing Yeh on 2014/7/24.
//
//

#ifndef _HTTP_HEADER_SCHEDULER_H_
#define _HTTP_HEADER_SCHEDULER_H_

#include "http_type.h"

#if __cplusplus
extern "C" {
#endif

void http_scheduler_new_connection(HTTP_Connection_T *http_connection);

#if __cplusplus
}
#endif

#endif /* _HTTP_HEADER_SCHEDULER_H_ */
