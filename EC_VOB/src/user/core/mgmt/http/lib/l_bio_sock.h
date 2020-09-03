//
//  http_sock.h
//  http_ssl
//
//  Created by JunYing Yeh on 2014/5/1.
//
//

#ifndef lib_sock_h
#define lib_sock_h

#include "openssl/ssl.h"

#if __cplusplus
extern "C" {
#endif

    typedef struct bio_socket_st bio_socket_t;

    typedef struct
    {
        int level;
        int optname;
        union {
            struct timeout_st {
                int milliseconds;
            } timeout; // SO_SNDTIMEO, SO_RCVTIMEO
        } optval;
    } bio_socket_option_t;

    typedef enum
    {
        BIO_SOCKET_NULL         = 0,
        BIO_SOCKET_OS_DEPENDENT = 1,
        BIO_SOCKET_OPENSSL      = 2,
        BIO_SOCKET_PREDEFINED_MAX
    } bio_socket_predefined_impl_t;

    typedef struct
    {
        int (*open)(bio_socket_t *, int socket_family, int socket_type, int protocol);
        int (*close)(bio_socket_t *);
        int (*is_open)(bio_socket_t *);
        int (*read)(bio_socket_t *, char *buf, size_t len, int flags);
        int (*write)(bio_socket_t *, char *buf, size_t len, int flags);
        int (*set_option)(bio_socket_t *, bio_socket_option_t *option);
    } bio_socket_impl_t;

    struct bio_socket_st
    {
        int fd;
        SSL *ssl;

        int auto_create;
        bio_socket_impl_t impl;
    };

    int L_BIO_socket_init(bio_socket_impl_t *impl, bio_socket_t *);
    int L_BIO_socket_reinit(bio_socket_impl_t *impl, bio_socket_t *);

    bio_socket_t * L_BIO_socket_new(bio_socket_impl_t *impl);
    void L_BIO_socket_free(bio_socket_t *);

    int L_BIO_socket_open(bio_socket_t *, int socket_family, int socket_type, int protocol);
    int L_BIO_socket_close(bio_socket_t *);
    int L_BIO_socket_is_open(bio_socket_t *);

    int L_BIO_socket_assign_fd(bio_socket_t *bs, int fd);
    int L_BIO_socket_assign_ssl(bio_socket_t *bs, SSL * ssl);

    int L_BIO_socket_read(bio_socket_t *, char *buf, size_t len, int flags);
    int L_BIO_socket_write(bio_socket_t *, char *buf, size_t len, int flags);
    int L_BIO_socket_set_option(bio_socket_t *, bio_socket_option_t *option);

//int L_SOCK_createsocket(int socket_family, int socket_type, int protocol);
int L_SOCK_closesocket(int s);
//int L_SOCK_setsendtimeout(int s, int timeout_ms);
//int L_SOCK_setrecvtimeout(int s, int timeout_ms);
//int L_SOCK_readsocket(int s, char *buf, size_t len, int flags);
//int L_SOCK_writesocket(int s, char *buf, size_t len, int flags);

//#if defined(OS_WINDOWS) && !defined(__CYGWIN32__)
//#  define HTTP_get_last_socket_error()	WSAGetLastError()
//#  define HTTP_clear_socket_error()     WSASetLastError(0)
//#  define HTTP_readsocket(s,b,n)        recv((s),(b),(n),0)
//#  define HTTP_writesocket(s,b,n)       send((s),(b),(n),0)
//#  define HTTP_EADDRINUSE               WSAEADDRINUSE
//#else
//#  define HTTP_get_last_socket_error()	ssl_errno
//#  define HTTP_clear_socket_error()     ssl_errno=0
//#  define HTTP_ioctlsocket(a,b,c)       ioctl(a,b,c)
//#  define HTTP_closesocket(s)           close(s)
//#  define HTTP_readsocket(s,b,n)        recv((s),(b),(n),0)
//#  define HTTP_writesocket(s,b,n)       send((s),(b),(n),0)
//#endif

#if __cplusplus
}
#endif

#endif