//
//  http_socketpair.h
//  http
//
//  Created by Emma Lin on 2014/8/8.
//
//

#ifndef _HTTP_HEADER_SOCKETPAIR_H_
#define _HTTP_HEADER_SOCKETPAIR_H_

#include "http_type.h"
#include "l_lib.h"

//#ifdef PLATFORM_UNIX
//#  include "http_socketpair_unix.h"
//#endif

#ifdef PLATFORM_WINDOWS
#  include "http_socketpair_win.h"
#endif

#if __cplusplus
extern "C" {
#endif


#if __cplusplus
}
#endif

#endif /* _HTTP_HEADER_SOCKETPAIR_H_ */
