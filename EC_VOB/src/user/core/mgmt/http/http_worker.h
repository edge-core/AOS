//
//  http_worker.h
//  http
//
//  Created by JunYing Yeh on 2014/7/16.
//
//

#ifndef _HTTP_HEADER_WORKER_H_
#define _HTTP_HEADER_WORKER_H_

#include "http_type.h"

#if __cplusplus
extern "C" {
#endif

void http_worker_main(HTTP_Worker_T *worker);
int http_worker_ctrl(HTTP_Worker_T *worker, HTTP_COMMAND_T command, void *params);

#if __cplusplus
}
#endif

#endif /* _HTTP_HEADER_WORKER_H_ */
