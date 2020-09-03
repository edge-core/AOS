
#include "alu_client_private.h"

alu_client_file_watch_t *
alu_client_file_watch_new(
    const char *path,
    int(*notify_fn)(alu_client_file_watch_t *ctx, int event_no),
    void *data,
    int flags,
    int interval,
    int hash_size)
{
    alu_client_file_watch_t *file;

    file = (alu_client_file_watch_t *)calloc(sizeof(alu_client_file_watch_t), 1);
    if (!file) {
        goto error;
    }

    file->flags = ALUC_FILE_WATCH_DFLT_FLAGS;
    file->interval = ALUC_FILE_WATCH_DFLT_INTERVAL;
    file->curr_polling = ALUC_FILE_WATCH_DFLT_INTERVAL;
    file->hash_size = ALUC_FILE_WATCH_DFLT_HASH_SIZE;
    file->hash_fn = alu_client_file_watch_sha256;
    file->stat_fn = alu_client_file_watch_stat;

    file->has_file = ALUC_FILE_WATCH_STATE_UNSPEC;

    file->data = data;
    file->notify_fn = notify_fn;

    if (path) {
        strncpy(file->path, path, sizeof(file->path));
        if (file->path[sizeof(file->path) - 1] != '\0') {
            // path too long
            goto error;
        }
    }

    file->flags = flags;

    if (0 < interval) {
        file->interval = interval;
    }

    if (0 < hash_size) {
        file->hash_size = hash_size;
    }

    if (0 < file->hash_size) {
        file->hash = (u_char *)calloc(file->hash_size, 1);
        if (!file->hash) {
            goto error;
        }
    }

    return file;

error:
    alu_client_file_watch_free(file);
    return NULL;
}

void
alu_client_file_watch_free(
    alu_client_file_watch_t *file)
{
    if (!file) {
        return;
    }

    if (file->hash) {
        free(file->hash);
    }

    free(file);
}

void
alu_client_file_watch(
    alu_client_file_watch_t *file)
{
    int changed = 0;
    int event_no = ALUC_FILE_WATCH_EVENT_CHANGE;

    if (file->curr_polling < file->interval) {
        file->curr_polling++;
        return;
    }

    file->curr_polling = 0;

    if ((file->flags & ALUC_FILE_WATCH_CHECK_OP_MASK) == ALUC_FILE_WATCH_CHECK_TIMESTAMPS_AND_SIZE) {
        alu_file_info_t sb;

        if (file->stat_fn(file, &sb)) {
            // file not exist

            if (file->has_file == ALUC_FILE_WATCH_STATE_EXIST) {
                memset(&file->sb, 0, sizeof(file->sb));
                file->has_file = ALUC_FILE_WATCH_STATE_NO_EXIST;

                changed = ALUC_FILE_WATCH_STATE_NO_EXIST;
                event_no = ALUC_FILE_WATCH_EVENT_DELETE;
            }
        }
        else {
            if (file->has_file == ALUC_FILE_WATCH_STATE_NO_EXIST) {
                file->has_file = ALUC_FILE_WATCH_STATE_EXIST;
                memcpy(&file->sb, &sb, sizeof(file->sb));

                changed = 1;
                event_no = ALUC_FILE_WATCH_EVENT_CREATE;
            }
            else {
                file->has_file = ALUC_FILE_WATCH_STATE_EXIST;

                if (alu_file_mtime(&file->sb) != alu_file_mtime(&sb)) {
                    memcpy(&file->sb, &sb, sizeof(file->sb));

                    changed = 1;
                    event_no = ALUC_FILE_WATCH_EVENT_CHANGE;
                }
            }
        }
    }

    if ((file->flags & ALUC_FILE_WATCH_CHECK_OP_MASK) == ALUC_FILE_WATCH_CHECK_CONTENT) {

        u_char *hash = (u_char*)calloc(file->hash_size, 1);
        if (!hash) {
            return;
        }

        if (file->hash_fn(file, hash)) {
            if (file->has_file == ALUC_FILE_WATCH_STATE_EXIST) {
                file->has_file = ALUC_FILE_WATCH_STATE_NO_EXIST;
                memset(file->hash, 0, file->hash_size);

                changed = 1;
                event_no = ALUC_FILE_WATCH_EVENT_DELETE;
            }
        }
        else {
            if (file->has_file == ALUC_FILE_WATCH_STATE_NO_EXIST) {
                file->has_file = ALUC_FILE_WATCH_STATE_EXIST;
                memcpy(file->hash, hash, file->hash_size);

                changed = 1;
                event_no = ALUC_FILE_WATCH_EVENT_CREATE;
            }
            else {
                file->has_file = ALUC_FILE_WATCH_STATE_EXIST;

                if (memcmp(file->hash, hash, file->hash_size) != 0) {
                    memcpy(file->hash, hash, file->hash_size);

                    changed = 1;
                    event_no = ALUC_FILE_WATCH_EVENT_CHANGE;
                }
            }
        }

        free(hash);
    }

    if (changed) {
        file->notify_count += 1;

        if (file->notify_count == 1 && file->flags & ALUC_FILE_WATCH_IGNORE_FIRST) {
            ;
        }
        else {
            file->notify_fn(file, event_no);
        }
    }
}

int
alu_client_file_watch_sha256(
    alu_client_file_watch_t *ctx, u_char *hash)
{
    FILE *file = NULL;

    SHA256_CTX sha256;
    const int bufSize = 1024;
    char* buffer = NULL;
    int bytesRead = 0;

    int rc = 0;

    file = fopen(ctx->path, "rb");
    if (!file) {
        rc = -1;
        goto error;
    }

    buffer = (char*)malloc(bufSize);
    if (!buffer) {
        rc = -1;
        goto error;
    }

    SHA256_Init(&sha256);

    while ((bytesRead = fread(buffer, 1, bufSize, file))) {
        SHA256_Update(&sha256, buffer, bytesRead);
    }

    SHA256_Final(hash, &sha256);

error:
    if (file) {
        fclose(file);
    }

    if (buffer) {
        free(buffer);
    }

    return rc;
}

int
alu_client_file_watch_stat(
    alu_client_file_watch_t *ctx,
    alu_file_info_t *sb)
{
    return alu_file_info(ctx->path, sb);
}
