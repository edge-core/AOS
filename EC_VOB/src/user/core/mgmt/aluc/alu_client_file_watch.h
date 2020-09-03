#ifndef _ALU_CLIENT_FILE_WATCH_HEADER_H_
#define _ALU_CLIENT_FILE_WATCH_HEADER_H_

#if __cplusplus
extern "C" {
#endif

#include "alu_client_private.h"

alu_client_file_watch_t *
alu_client_file_watch_new(
    const char *path,
    int(*notify_fn)(alu_client_file_watch_t *ctx, int event_no),
    void *data,
    int flags,
    int interval,
    int hash_size
);

void
alu_client_file_watch_free(
    alu_client_file_watch_t *file
);

int
alu_client_file_watch_sha256(
    alu_client_file_watch_t *ctx,
    u_char *hash
);

int
alu_client_file_watch_stat(
    alu_client_file_watch_t *ctx,
    alu_file_info_t *sb
);

void
alu_client_file_watch(
    alu_client_file_watch_t *file
);

#if __cplusplus
}
#endif

#endif /* _ALU_CLIENT_FILE_WATCH_HEADER_H_ */
