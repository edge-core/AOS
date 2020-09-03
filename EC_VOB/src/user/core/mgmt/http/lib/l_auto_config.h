//
//  http_auto_config.h
//  http_ssl
//
//  Created by JunYing Yeh on 2014/5/1.
//
//

#ifndef lib_auto_config_h
#define lib_auto_config_h

#if defined(_WIN32) && !defined(WIN32) && !defined(__CYGWIN32__)
#  define WIN32
#endif

#if (defined(WIN32) || defined(WIN16)) && !defined(__CYGWIN32__)
#  ifndef WINDOWS
#    define OS_WINDOWS
#  endif
#  ifndef MSDOS
#    define OS_MSDOS
#  endif
#  define PLATFORM_WINDOWS
#endif

#if defined(__APPLE__) || defined(__MACH__)
#  define OS_MACOSX
#  define PLATFORM_UNIX
#endif

#if defined(__linux__)
#  define OS_LINUX
#  define PLATFORM_UNIX
#endif

#if defined(__FreeBSD__)
#  define OS_FREEBSD
#  define PLATFORM_UNIX
#endif

#define HAVE_LIMITS_H           1
#define HAVE_STDIO_H            1
#define HAVE_STDLIB_H           1
#define HAVE_MEMORY_H           1
#define HAVE_STRING_H           1
#define HAVE_ERRNO_H            1
#define HAVE_TIME_H             1
#define HAVE_ASSERT_H           1
#define HAVE_CTYPE_H            1
#define HAVE_FCNTL_H            1

#if defined(OS_WINDOWS)
#  define HAVE_WINDOWS_H        1
#  define HAVE_WINSOCK2_H       1
#endif

#if defined(OS_MACOSX)
#  define HAVE_UNISTD_H         1
#  define HAVE_ARPA_INET_H      1
#  define HAVE_NETINET_IN_H     1
#  define HAVE_SYS_SOCKET_H     1
#  define HAVE_SYS_TIME_H       1
#endif

#if defined(OS_LINUX)
#  undef HAVE_MEMORY_H
#  define HAVE_ARPA_INET_H      1
#  define HAVE_NETINET_IN_H     1
#  define HAVE_SYS_SOCKET_H     1
#endif

#endif
