//
//  l_config_os.h
//  http_ssl
//
//  Created by JunYing Yeh on 2014/5/1.
//
//

#ifndef lib_config_os_h
#define lib_config_os_h

//#ifdef L_USE_SOCK
#ifndef L_NO_SOCK

#  if defined(HAVE_WINSOCK2_H)
#    include <WinSock2.h>
#    include <Ws2ipdef.h>
#    include <WS2tcpip.h>
#  endif

#  if defined(HAVE_SYS_SOCKET_H)
#    include <sys/socket.h>
#  endif

#  if defined(HAVE_NETINET_IN_H)
#    include <netinet/in.h>
#  endif

#  if defined(HAVE_ARPA_INET_H)
#    include <arpa/inet.h>
#  endif

#  if defined(SYS_TIME_H)
#    include <sys/time.h>
#  endif

#endif // L_USE_SOCK

#if  defined(HAVE_WINDOWS_H)
#  include <Windows.h>
#endif

#if  defined(HAVE_LIMITS_H)
#  include <limits.h>
#endif

#if  defined(HAVE_STDIO_H)
#  include <stdio.h>
#endif

#if  defined(HAVE_STDLIB_H)
#  include <stdlib.h>
#endif

#if  defined(HAVE_MEMORY_H)
#  include <memory.h>
#endif

#if  defined(HAVE_STRING_H)
#  include <string.h>
#endif

#if  defined(HAVE_ERRNO_H)
#  include <errno.h>
#endif

#if defined(HAVE_TIME_H)
#  include <time.h>
#endif

#if defined(HAVE_ASSERT_H)
#  include <assert.h>
#endif

#if  defined(HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#if  defined(HAVE_CTYPE_H)
#include <ctype.h>
#endif

#if  defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif

#ifdef _MSC_VER
#  define STATIC_INLINE static __inline
#else
#  define STATIC_INLINE static inline
#endif

#ifdef _MSC_VER
#  ifndef snprintf
#    define snprintf _snprintf
#  endif

#  ifndef ssize_t
#    define ssize_t int
#  endif

#  ifndef strncasecmp
#    define strncasecmp(dest, src, len) _stricmp(dest, src)
#  endif

#  ifndef strerror_r
#    define strerror_r(errno, buf, len) strerror_s(buf, len, errno)
#  endif

#  ifndef AF_LOCAL
#    //define AF_LOCAL AF_UNIX
#    define AF_LOCAL AF_INET
#  endif

#  ifndef EWOULDBLOCK
#    define EWOULDBLOCK EAGAIN
#  endif

#  ifndef SHUT_WR
#    define SHUT_WR SD_SEND
#  endif
#endif

#endif
