//
//  http_time.h
//  http
//
//  Created by JunYing Yeh on 2014/7/28.
//
//

#ifndef _HTTP_HEADER_TIME_H_
#define _HTTP_HEADER_TIME_H_

#include "http_type.h"
#include "l_lib.h"

//#ifdef PLATFORM_UNIX
//#  include "http_time_unix.h"
//#endif

#ifdef PLATFORM_WINDOWS
#  include "http_time_win.h"
#endif

#if __cplusplus
extern "C" {
#endif


#if __cplusplus
}
#endif

#endif /* _HTTP_HEADER_TIME_H_ */
