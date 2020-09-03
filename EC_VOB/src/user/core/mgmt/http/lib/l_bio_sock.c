//
//  l_sock.c
//  http_ssl
//
//  Created by JunYing Yeh on 2014/5/1.
//
//

#ifndef L_USE_SOCK
#define L_USE_SOCK
#endif

#include <stdint.h>
#include "l_config.h"
#include "l_core.h"
#include "l_bio_sock.h"
#include "l_bio_sock_os.h"
#include "l_bio_sock_openssl.h"

//static void http_sock_init()
//{
//#ifdef OS_WINDOWS
//    static int init = 0;
//    WSADATA wsadata;
//
//    L_CORE_READ_LOCK(L_SOCK);
//    if (init) {
//        L_CORE_READ_UNLOCK(L_SOCK);
//        return;
//    }
//    L_CORE_READ_UNLOCK(L_SOCK);
//
//    L_CORE_WRITE_LOCK(L_SOCK);
//    init = 1;
//    L_CORE_WRITE_UNLOCK(L_SOCK);
//
//    if (WSAStartup (MAKEWORD (2, 0), &wsadata) != 0)
//    {
//        //
//    }
//#endif
//}
//
//int L_SOCK_createsocket(int socket_family, int socket_type, int protocol)
//{
//    http_sock_init();
//
//    return socket(socket_family, socket_type, protocol);
//}

int L_SOCK_closesocket(int sock)
{
#ifdef OS_WINDOWS
    return closesocket(sock);
#else
    return close(sock);
#endif
}

//int L_SOCK_setsendtimeout(int sock, int timeout_ms)
//{
//#ifdef OS_WINDOWS
//    DWORD timeout = timeout_ms;
//#else
//    struct timeval timeout = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
//#endif
//
//    return setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
//}
//
//int L_SOCK_setrecvtimeout(int sock, int timeout_ms)
//{
//#ifdef OS_WINDOWS
//    DWORD timeout = timeout_ms;
//#else
//    struct timeval timeout = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
//#endif
//
//    return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
//}
//
//int L_SOCK_readsocket(int s, char *buf, size_t len, int flags)
//{
//    return recv(s, buf, len, flags);
//}
//
//int L_SOCK_writesocket(int s, char *buf, size_t len, int flags)
//{
//    return send(s, buf, len, flags);
//}

static int bio_socket_impl_null_open(bio_socket_t *bs, int socket_family, int socket_type, int protocol) {
    return -1;
}

static int bio_socket_impl_null_close(bio_socket_t *bs) {
    return 0;
}

static int bio_socket_impl_null_is_open(bio_socket_t *bs) {
    return 0;
}

static int bio_socket_impl_null_read(bio_socket_t *bs, char *buf, size_t len, int flags) {
    return 0;
}

static int bio_socket_impl_null_write(bio_socket_t *bs, char *buf, size_t len, int flags) {
    return 0;
}

static int bio_socket_impl_null_set_option(bio_socket_t *bs, bio_socket_option_t *option) {
    return -1;
}

static bio_socket_impl_t bio_socket_null_impl =
{
    bio_socket_impl_null_open,
    bio_socket_impl_null_close,
    bio_socket_impl_null_is_open,
    bio_socket_impl_null_read,
    bio_socket_impl_null_write,
    bio_socket_impl_null_set_option
};

bio_socket_impl_t bio_socket_null_impl_get()
{
    return bio_socket_null_impl;
}

bio_socket_impl_t bio_socket_predefined_impl_get(int predefined)
{
    bio_socket_impl_t impl = {0};

    switch (predefined) {
        case BIO_SOCKET_NULL:
            impl = bio_socket_null_impl_get();
            break;

        case BIO_SOCKET_OS_DEPENDENT:
            impl = bio_socket_os_impl_get();
            break;

        case BIO_SOCKET_OPENSSL:
            impl = bio_socket_openssl_impl_get();
            break;

        default:
            assert(0);
            break;
    }

    return impl;
}

int L_BIO_socket_init(bio_socket_impl_t *impl, bio_socket_t *bs)
{
    if (!bs) {
        return -1;
    }

    memset(bs, 0, sizeof(*bs));
    bs->fd = -1;
    bs->ssl = NULL;

    if ((uintptr_t)impl < BIO_SOCKET_PREDEFINED_MAX) {
        bs->impl = bio_socket_predefined_impl_get((int)(uintptr_t)impl);
    }
    else {
        bs->impl = *impl;
    }

    return 0;
}

int L_BIO_socket_reinit(bio_socket_impl_t *impl, bio_socket_t *bs)
{
    bio_socket_t old_bs;
    int ret;

    if (!bs) {
        return -1;
    }

    old_bs = *bs;

    ret = L_BIO_socket_init(impl, bs);
    if (ret) {
        bs->fd = old_bs.fd;
        bs->ssl = old_bs.ssl;
    }

    return ret;
}

bio_socket_t * L_BIO_socket_new(bio_socket_impl_t *impl)
{
    bio_socket_t *ret;

    ret = malloc(sizeof(*ret));
    if (!ret) {
        return NULL;
    }

    L_BIO_socket_init(impl, ret);

    ret->auto_create = 1;

    return ret;
}

void L_BIO_socket_free(bio_socket_t *bs)
{
    if (!bs) {
        return;
    }

    L_BIO_socket_close(bs);

    if (bs->auto_create == 1)
    {
        free(bs);
        bs->auto_create = 0;
    }
}

int L_BIO_socket_open(bio_socket_t *bs, int socket_family, int socket_type, int protocol)
{
    if (!bs) {
        return -1;
    }

    if (!bs->impl.open) {
        return bio_socket_null_impl.open(bs, socket_family, socket_type, protocol);
    }

    return bs->impl.open(bs, socket_family, socket_type, protocol);
}

int L_BIO_socket_close(bio_socket_t *bs)
{
    if (!bs) {
        return -1;
    }

    if (!bs->impl.close) {
        return bio_socket_null_impl.close(bs);
    }

    return bs->impl.close(bs);
}

int L_BIO_socket_is_open(bio_socket_t *bs)
{
    if (!bs) {
        return -1;
    }

    if (!bs->impl.is_open) {
        return bio_socket_null_impl.is_open(bs);
    }

    return bs->impl.is_open(bs);
}

int L_BIO_socket_assign_fd(bio_socket_t *bs, int fd)
{
    if (!bs) {
        return -1;
    }

    if (0 <= bs->fd && 0 <= fd) {
        return -1;
    }

    L_BIO_socket_reinit((void *)BIO_SOCKET_OS_DEPENDENT, bs);

//    if (L_BIO_socket_is_open(bs)) {
//        L_BIO_socket_close(bs);
//    }

    bs->fd = fd;

    return 0;
}

int L_BIO_socket_assign_ssl(bio_socket_t *bs, SSL * ssl)
{
    if (!bs) {
        return -1;
    }

    if (bs->ssl && ssl) {
        return -1;
    }

    L_BIO_socket_reinit((void *)BIO_SOCKET_OPENSSL, bs);

    bs->ssl = ssl;

    if (bs->ssl) {
        bs->fd = SSL_get_fd(bs->ssl);
    }

    return 0;
}

int L_BIO_socket_read(bio_socket_t *bs, char *buf, size_t len, int flags)
{
    if (!bs) {
        return -1;
    }

    if (!bs->impl.read) {
        return bio_socket_null_impl.read(bs, buf, len, flags);
    }

    return bs->impl.read(bs, buf, len, flags);
}

int L_BIO_socket_write(bio_socket_t *bs, char *buf, size_t len, int flags)
{
    if (!bs) {
        return -1;
    }

    if (!bs->impl.write) {
        return bio_socket_null_impl.write(bs, buf, len, flags);
    }

    return bs->impl.write(bs, buf, len, flags);
}

int L_BIO_socket_set_option(bio_socket_t *bs, bio_socket_option_t *option)
{
    if (!bs) {
        return -1;
    }

    if (!bs->impl.set_option) {
        return bio_socket_null_impl.set_option(bs, option);
    }

    return bs->impl.set_option(bs, option);
}
