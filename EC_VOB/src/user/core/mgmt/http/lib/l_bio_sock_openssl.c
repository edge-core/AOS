//
//  l_bio_sock_openssl.c
//  http_ssl
//
//  Created by JunYing Yeh on 2014/5/2.
//
//

#ifndef L_USE_SOCK
#define L_USE_SOCK
#endif

#include "l_config.h"
#include "l_core.h"
#include "l_bio_sock.h"

static int bio_socket_openssl_read(bio_socket_t *bs, char *buf, size_t len, int flags);
static int bio_socket_openssl_write(bio_socket_t *bs, char *buf, size_t len, int flags);

bio_socket_impl_t bio_socket_openssl_impl_get()
{
    bio_socket_impl_t impl = {0};

//    impl.open = bio_socket_os_open;
//    impl.close = bio_socket_os_close;
//    impl.is_open = bio_socket_os_is_open;
    impl.read = bio_socket_openssl_read;
    impl.write = bio_socket_openssl_write;
//    impl.set_option = bio_socket_os_set_option;

    return impl;
}

static int bio_socket_openssl_read(bio_socket_t *bs, char *buf, size_t len, int flags)
{
    if (bs->ssl == NULL)
    {
        return -1;
    }

    return SSL_read(bs->ssl, buf, len);
}

static int bio_socket_openssl_write(bio_socket_t *bs, char *buf, size_t len, int flags)
{
    if (bs->ssl == NULL)
    {
        return -1;
    }

    return SSL_write(bs->ssl, buf, len);
}