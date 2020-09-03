#ifndef _CONFIG_AGENT_HEADER_AUTO_CONFIG_H_
#define _CONFIG_AGENT_HEADER_AUTO_CONFIG_H_

#if defined(_WIN32) && !defined(WIN32) && !defined(__CYGWIN32__)
#  define WIN32
#endif

#if (defined(WIN32) || defined(WIN16)) && !defined(__CYGWIN32__)
#  define CONFIG_AGENT_OS_WINDOWS
#  define CONFIG_AGENT_PLATFORM_WINDOWS
#endif

#if defined(__APPLE__) || defined(__MACH__)
#  define CONFIG_AGENT_OS_MACOS
#  define CONFIG_AGENT_PLATFORM_UNIX
#endif

#if defined(__linux__)
#  define CONFIG_AGENT_OS_LINUX
#  define CONFIG_AGENT_PLATFORM_UNIX
#endif

#if defined(__FreeBSD__)
#  define OS_FREEBSD
#  define CONFIG_AGENT_PLATFORM_UNIX
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

#if defined(CONFIG_AGENT_OS_WINDOWS)
#  define HAVE_WINDOWS_H        1
#  define HAVE_WINSOCK2_H       1
#endif

#if defined(CONFIG_AGENT_OS_MACOS)
#  define HAVE_UNISTD_H         1
#  define HAVE_ARPA_INET_H      1
#  define HAVE_NETINET_IN_H     1
#  define HAVE_SYS_SOCKET_H     1
#  define HAVE_SYS_TIME_H       1
#endif

#if defined(CONFIG_AGENT_OS_LINUX)
#  undef HAVE_MEMORY_H
#  define HAVE_ARPA_INET_H      1
#  define HAVE_NETINET_IN_H     1
#  define HAVE_SYS_SOCKET_H     1
#endif

#endif
