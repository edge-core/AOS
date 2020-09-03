//
//  bio_sock_os.c
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

static int bio_socket_os_open(bio_socket_t *bs, int socket_family, int socket_type, int protocol);
static int bio_socket_os_close(bio_socket_t *bs);
static int bio_socket_os_is_open(bio_socket_t *bs);
static int bio_socket_os_read(bio_socket_t *bs, char *buf, size_t len, int flags);
static int bio_socket_os_write(bio_socket_t *bs, char *buf, size_t len, int flags);
static int bio_socket_os_set_option(bio_socket_t *bs, bio_socket_option_t *option);

// TODO: Bad idea
#ifdef OS_WINDOWS
#define close(fd) closesocket(fd)
#endif

bio_socket_impl_t bio_socket_os_impl_get()
{
    bio_socket_impl_t impl = {0};

    impl.open = bio_socket_os_open;
    impl.close = bio_socket_os_close;
    impl.is_open = bio_socket_os_is_open;
    impl.read = bio_socket_os_read;
    impl.write = bio_socket_os_write;
    impl.set_option = bio_socket_os_set_option;

    return impl;
}

static int bio_socket_os_open(bio_socket_t *bs, int socket_family, int socket_type, int protocol)
{
    if (bio_socket_os_is_open(bs)) {
        bio_socket_os_close(bs);
    }

    bs->fd = socket(socket_family, socket_type, protocol);
    return bs->fd;
}

static int bio_socket_os_close(bio_socket_t *bs)
{
    if (bio_socket_os_is_open(bs)) {
        close(bs->fd);
        bs->fd = -1;
    }

    return 0;
}

static int bio_socket_os_is_open(bio_socket_t *bs)
{
    return (0 <= bs->fd) ? 1 : 0;
}

static int bio_socket_os_read(bio_socket_t *bs, char *buf, size_t len, int flags)
{
    return recv(bs->fd, buf, len, flags);
}

static int bio_socket_os_write(bio_socket_t *bs, char *buf, size_t len, int flags)
{
// TODO: Bad idea
#ifdef OS_MACOSX
    int set = 1;
    setsockopt(bs->fd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
#endif

    return send(bs->fd, buf, len, flags);
}

static int bio_socket_os_set_option(bio_socket_t *bs, bio_socket_option_t *option)
{
    switch (option->optname) {
        case SO_SNDTIMEO:
        case SO_RCVTIMEO:
        {
#ifdef OS_WINDOWS
            DWORD timeout = option->optval.timeout.milliseconds;
#else
            struct timeval timeout = {option->optval.timeout.milliseconds / 1000,
                (option->optval.timeout.milliseconds % 1000) * 1000};
#endif

            return setsockopt(bs->fd, SOL_SOCKET, option->optname, &timeout, sizeof(timeout));
        }

        default:
            break;
    }
    return -1;
}
