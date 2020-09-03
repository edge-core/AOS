//
//  http_socketpair_win.h
//  http
//
//  Created by Emma Lin on 2014/8/8.
//
//

#ifndef _HTTP_HEADER_SOCKETPAIR_WIN_H_
#define _HTTP_HEADER_SOCKETPAIR_WIN_H_

#if __cplusplus
extern "C" {
#endif

int socketpair(int af, int type, int protocol, int socks[2]);

#if __cplusplus
}
#endif


#endif /* _HTTP_HEADER_SOCKETPAIR_WIN_H_ */
