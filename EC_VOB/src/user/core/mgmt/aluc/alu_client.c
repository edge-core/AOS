#ifndef LLONG_MAX
#include <limits.h>
#endif

#ifndef LLONG_MAX
#  define LLONG_MAX    9223372036854775807LL
#endif

#ifndef LLONG_MIN
#  define LLONG_MIN    (-LLONG_MAX - 1LL)
#endif

#include "l_threadgrp.h"
#include "sys_adpt.h"
#include "sys_type.h"
#include "sys_bld.h"
#include "sysfun.h"
#include <sys/file.h>

#include "stktplg_mgr.h"
#include "stktplg_pom.h"
#include "stktplg_pmgr.h"

#include "swctrl_pmgr.h"
#include "swctrl_pom.h"

#include "swdrv.h"
#include "cmgr.h"
#include "fs.h"

#include "alu_client.h"
#include "alu_client_private.h"
#include "alu_file.h"

#define ALU_CLIENT_FILE_CHECK_TIMER_INTERVAL         1
#define ALU_CLIENT_EXPIRE_CHECK_TIMER_INTERVAL      60
#define ALU_CLIENT_BSON_CHECK_TIMER_INTERVAL         1

#define ALU_CLIENT_MIN_DELAY_TIME                   10
#define ALU_CLIENT_MAX_DELAY_TIME                   30

#define ALU_CLIENT_DFLT_LOG_LEVEL                   ALU_CLIENT_LOG_LEVEL_INFO

#if defined(_MSC_VER)
# define close(fd) _close(fd)
#endif

#if defined(_MSC_VER)
# define fileno(fd) _fileno(fd)
#endif

static void
alu_client_on_check_expired(
    alu_client_ctx_t *ctx
);

static int
alu_client_on_license_file_changed(
    alu_client_file_watch_t *file,
    int event_no
);

static int
alu_client_on_activation_code_file_changed(
    alu_client_file_watch_t *file,
    int event_no
);

#if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE == TRUE)
static int
alu_client_on_bson_changed(
    alu_client_file_watch_t *file,
    int event_no
);

static int
alu_client_bson_sha256(
    alu_client_file_watch_t *ctx,
    u_char *hash
);
#endif /* SYS_CPNT_FS_SUPPORT_DATA_STORAGE */

static int
alu_client_validate_file(
    alu_client_ctx_t *ctx
);

static alu_client_ctx_t *
alu_client_new();

static void
alu_client_destroy(
    alu_client_ctx_t *ctx
);

static int
alu_client_init(
    alu_client_ctx_t *ctx
);

static void
alu_client_clear(
    alu_client_ctx_t *ctx
);

static BOOL_T
alu_client_license_is_time_limit(
   const alu_client_license_t *lic
);

static BOOL_T
alu_client_active_is_expired(
    const alu_client_active_license_t *active
);

static const char *
alu_client_find_replace_license(
    const alu_client_ctx_t *ctx
);

static int
alu_client_load_or_new_storage(
    alu_client_ctx_t *ctx
);

static int
alu_client_restore_active_license_to_db(
    alu_client_ctx_t *ctx
);

static void
alu_client_fetch_active_license(
    alu_client_ctx_t *ctx
);

static int
alu_client_updated_license_after_checker(
    alu_client_ctx_t *ctx
);

static int
alu_client_swap_active(
    alu_client_ctx_t *ctx
);

static int
alu_client_check_expire(
    alu_client_ctx_t *ctx
);

static int
alu_client_validate_from_file(
    alu_client_ctx_t *ctx
);

static BOOL_T
alu_client_is_accept_port(
    alu_client_ctx_t *ctx,
    uint32_t ifindex
);

static int
alu_client_apply_action(
    alu_client_ctx_t *ctx
);

static int
alu_client_success(
    alu_client_ctx_t *ctx
);

static int
alu_client_error(
    alu_client_ctx_t *ctx
);

static int
alu_client_set_port_status(
    alu_client_ctx_t *ctx,
    uint32_t ifindex,
    BOOL_T status
);

static int
alu_client_range_extract_begin(
    alu_client_range_extract_t *ctx
);

static int
alu_client_range_extract_flush(
    alu_client_range_extract_t *ctx
);

static int
alu_client_range_extract_add(
    alu_client_range_extract_t *ctx,
    UI32_T n
);

static int
alu_client_range_extract_end(
    alu_client_range_extract_t *ctx
);

static void
alu_client_master_process(
    alu_client_ctx_t *ctx
);

static alu_fd_t
alu_client_create_master_pidfile(
    alu_client_ctx_t *ctx
);

static void
alu_client_on_check_expired(
    alu_client_ctx_t *ctx)
{
    alu_client_check_expire(ctx);
}

static int
alu_client_on_license_file_changed(
    alu_client_file_watch_t *file,
    int event_no)
{
    FILE *fp;

    alu_client_ctx_t *ctx = (alu_client_ctx_t *) file->data;

    ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_INFO, "Detected the license file %s",
        event_no == ALUC_FILE_WATCH_EVENT_CHANGE ? "changed" :
        event_no == ALUC_FILE_WATCH_EVENT_DELETE ? "deleted" :
        event_no == ALUC_FILE_WATCH_EVENT_CREATE ? "created" :
        event_no == ALUC_FILE_WATCH_EVENT_DELETE_SELF ? "deleted-self" : "unknown");

    if ((event_no == ALUC_FILE_WATCH_EVENT_CREATE || event_no == ALUC_FILE_WATCH_EVENT_CHANGE) && alu_file_size(&file->sb) < ctx->lic_buffer_max_size) {
        fp = fopen(file->path, "rb");
        if (fp == NULL) {
            ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_ERROR, "Failed to open license file (%d): %s", errno, strerror(errno));
            return -1;
        }

        ctx->lic_buffer_len = fread(ctx->lic_buffer, 1, alu_file_size(&file->sb), fp);
        if (ctx->lic_buffer_len != alu_file_size(&file->sb)) {
            ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_ERROR, "Failed to read license file (%d): %s", errno, strerror(errno));
            fclose(fp);
            return -1;
        }

        fclose(fp);
    }
    else {
        ctx->lic_buffer_len = 0;
    }

    alu_client_validate_file(ctx);

    return 0;
}

static int
alu_client_on_activation_code_file_changed(
    alu_client_file_watch_t *file,
    int event_no)
{
    FILE *fp;

    alu_client_ctx_t *ctx = (alu_client_ctx_t *)file->data;

    ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_INFO, "Detected the license activation-code file %s",
                   event_no == ALUC_FILE_WATCH_EVENT_CHANGE ? "changed" :
                   event_no == ALUC_FILE_WATCH_EVENT_DELETE ? "deleted" :
                   event_no == ALUC_FILE_WATCH_EVENT_CREATE ? "created" :
                   event_no == ALUC_FILE_WATCH_EVENT_DELETE_SELF ? "deleted-self" : "unknown");

    if ((event_no == ALUC_FILE_WATCH_EVENT_CREATE || event_no == ALUC_FILE_WATCH_EVENT_CHANGE)
        && alu_file_size(&file->sb) < ctx->ac_buffer_max_size) {
        fp = fopen(file->path, "rb");
        if (fp == NULL) {
            ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_ERROR, "Failed to open file (%d): %s", errno, strerror(errno));
            return -1;
        }

        ctx->ac_buffer_len = fread(ctx->ac_buffer, 1, alu_file_size(&file->sb), fp);
        if (ctx->ac_buffer_len != alu_file_size(&file->sb)) {
            ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_ERROR, "Failed to read file (%d): %s", errno, strerror(errno));
            fclose(fp);
            return -1;
        }

        fclose(fp);
    }
    else {
        ctx->ac_buffer_len = 0;
    }

    alu_client_validate_file(ctx);

    return 0;
}

#if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE == TRUE)
static int
alu_client_on_bson_changed(
    alu_client_file_watch_t *file,
    int event_no)
{
    alu_client_ctx_t *ctx = (alu_client_ctx_t *)file->data;

    if (memcmp(ctx->storage.sha256, file->hash, file->hash_size) == 0) {
        ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_DEBUG, "Fake BSON file changed event, do donothing");
        return ALU_OK;
    }

    ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_INFO, "Detected the BSON file %s",
        event_no == ALUC_FILE_WATCH_EVENT_CHANGE ? "changed" :
        event_no == ALUC_FILE_WATCH_EVENT_DELETE ? "deleted" :
        event_no == ALUC_FILE_WATCH_EVENT_CREATE ? "created" :
        event_no == ALUC_FILE_WATCH_EVENT_DELETE_SELF ? "deleted-self" : "unknown");

    ASSERT(file->hash_size == sizeof(ctx->storage.sha256));
    memcpy(ctx->storage.sha256, file->hash, sizeof(ctx->storage.sha256));

    alu_client_load_storage(ctx);

    ctx->active_license.valid = 0;
    alu_client_validate_file(ctx);

    // The BSON file (may) be changed after validation.
    // To avoid the notification-loop, sync the hash back to FileWatch.
    memcpy(file->hash, ctx->storage.sha256, file->hash_size);

    return 0;
}

static int
alu_client_bson_sha256(
    alu_client_file_watch_t *file,
    u_char *hash)
{
    int rc = 0;

    uint8_t *file_data = NULL;
    UI32_T  file_size;

    alu_client_ctx_t *ctx = (alu_client_ctx_t *)file->data;

    file_size = FS_NonVolatileDataStorage_GetSize(FS_TYPE_DATA_STORAGE_ID_ALU);

    file_data = (u_char *)calloc(file_size, 1);
    if (!file_data) {
        ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_ERROR, "Failed to allocate memory for keeping storage data");
        goto error;
    }

    if (FS_NonVolatileDataStorage_Read(FS_TYPE_DATA_STORAGE_ID_ALU, &file_size, file_data) != TRUE) {
        ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_ERROR, "Failed to read storage");
        goto error;
    }

    SHA256(file_data, file_size, hash);

    if (0) {
    error:
        if (0 <= rc) {
            rc = -1;
        }
    }

    if (file_data) {
        free(file_data);
    }

    return rc;
}
#endif /* SYS_CPNT_FS_SUPPORT_DATA_STORAGE */

static int
alu_client_validate_file(
    alu_client_ctx_t *ctx)
{
    int rc;

    ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_INFO, "Start to validate the license");

    rc = ctx->validate_from_file(ctx);

    alu_client_swap_active(ctx);

    ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_INFO, "Validate as (%s) license { Valid(%s), Invalid(%s), Trial(%s) }",
        (ctx->active_license.valid == 1) ? "good" : "bad", ALU_IS_VALID(&ctx->chk) ? "Yes" : "No",
        ALU_IS_INVALID(&ctx->chk) ? "Yes" : "No", ALU_IS_TRIAL(&ctx->chk) ? "Yes" : "No");

    if (ctx->active_license.valid == 1) {
        ctx->success(ctx);
    }
    else {
        ctx->error(ctx);
    }

    return rc;
}

static alu_client_ctx_t *
alu_client_new() {
    alu_client_ctx_t *ctx;

    ctx = (alu_client_ctx_t *) calloc(sizeof(alu_client_ctx_t), 1);
    if (!ctx) {
        return NULL;
    }

    if (alu_client_init(ctx) != 0) {
        goto error;
    }

    return ctx;

error:
    alu_client_destroy(ctx);
    return NULL;
}

static void
alu_client_destroy(
    alu_client_ctx_t *ctx) {

    if (!ctx) {
        return;
    }

    alu_client_clear(ctx);

    free(ctx);
}

static int
alu_client_init(
    alu_client_ctx_t *ctx)
{
    memset(ctx, 0, sizeof(*ctx));

    ctx->process = ALUC_PROCESS_UNSPEC;

    ctx->lic_buffer = (unsigned char *)calloc(2048, 1);
    if (ctx->lic_buffer == NULL) {
        return -1;
    }

    ctx->lic_buffer_max_size = 2048;
    ctx->lic_buffer_len = 0;

    ctx->ac_buffer = (unsigned char *)calloc(256, 1);
    if(ctx->ac_buffer == NULL) {
        return -1;
    }

    ctx->ac_buffer_max_size = 256;
    ctx->ac_buffer_len = 0;

    ctx->delay_error = FALSE;

    ctx->validate_from_file = alu_client_validate_from_file;
    ctx->success = alu_client_success;
    ctx->error = alu_client_error;

    //// default version of storage
    ctx->storage.major_version = 1;
    ctx->storage.minor_version = 0;
    ctx->storage.micro_version = 0;

    ctx->logger.fd = -1;
    ctx->logger.level = ALU_CLIENT_DFLT_LOG_LEVEL;

    return 0;
}

static void
alu_client_clear(
    alu_client_ctx_t *ctx)
{
    if (!ctx) {
        return;
    }

    if (ctx->lic_buffer) {
        free(ctx->lic_buffer);
    }

    if (ctx->ac_buffer) {
        free(ctx->ac_buffer);
    }

    if (ctx->storage.data.doc) {
        json_decref(ctx->storage.data.doc);
        ctx->storage.data.doc = NULL;
    }

    if (2 < ctx->logger.fd) {
        close(ctx->logger.fd);
        ctx->logger.fd = -1;
    }
}

static BOOL_T
alu_client_license_is_time_limit(
   const alu_client_license_t *lic)
{

    ASSERT(lic);
    if (!lic) {
        return TRUE;
    }

    return (lic->trial_time) ? TRUE : FALSE;
}

static BOOL_T
alu_client_active_is_expired(
    const alu_client_active_license_t *active)
{
    return (active->license.stats.used_time >= active->license.stats.total_time) ? TRUE : FALSE;
}

// ASSUMPTION: if 'total-time' of license is smaller then ALU_MAX_TRIAL_DAYS_IN_SECONDS,
//             the license will be regarded as trial license, elsewise be regraded as formal license
//             the trial license be first privilege for replacement.
static const char *
alu_client_find_replace_license(
    const alu_client_ctx_t *ctx)
{
    const char *key;
    json_t *value;
    const char *trial = NULL;
    const char *regular = NULL;

    int64_t trial_last = LLONG_MAX;
    int64_t regular_last = LLONG_MAX;

    json_t *active;
    json_t *licenses;

    const char *active_lic_no = NULL;

    active = alu_client_doc_get_active(&ctx->storage.data);
    licenses = alu_client_doc_get_licenses(&ctx->storage.data);

    if (!licenses) {
        return NULL;
    }

    if (active) {
        json_t *license_number;

        license_number = json_object_get(active, "license-no");
        if (license_number && json_typeof(license_number)) {
            active_lic_no = json_string_value(license_number);
        }
    }

    json_object_foreach(licenses, key, value) {
        json_t *last_updated_time = json_object_get(value, "last-updated-time");
        json_t *total_time = json_object_get(value, "total-time");

        if (active_lic_no && strcmp(active_lic_no, key) == 0) {
            continue;
        }

        if (!last_updated_time || json_typeof(last_updated_time) != JSON_INTEGER ||
            !total_time || json_typeof(total_time) != JSON_INTEGER) {
            trial = key;
            break;
        }

        if (json_integer_value(total_time) <= (ALU_MAX_TRIAL_DAYS_IN_SECONDS / 60)) {
            if (json_integer_value(last_updated_time) < trial_last) {
                trial = key;
                trial_last = json_integer_value(last_updated_time);
            }
        }
        else {
            if (json_integer_value(last_updated_time) < regular_last) {
                regular = key;
                regular_last = json_integer_value(last_updated_time);
            }
        }
    }

    return trial ? trial : (regular ? regular : NULL);
}

#if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE == TRUE)
int
alu_client_save_storage(
    alu_client_ctx_t *ctx)
{
    int rc = 0;

    bson_t* bson_file;
    uint32_t ds_size;

    if (!ctx->storage.data.drity) {
        return 0;
    }

    ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_INFO, "Save storage");

    ds_size = FS_NonVolatileDataStorage_GetSize(FS_TYPE_DATA_STORAGE_ID_ALU);

again:
    bson_file = alu_client_storage_dumpb(ctx);

    if (!bson_file) {
        return -1;
    }

    if (ds_size < bson_file->len) {
        const char *removed;
        uint32_t bson_file_len = bson_file->len;

        bson_destroy(bson_file);
        bson_file = NULL;

        removed = alu_client_find_replace_license(ctx);
        if (removed) {
            ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_INFO, "License %s be removed from history", removed);
            alu_client_doc_remove_license(&ctx->storage.data, removed);
            goto again;
        }
        else {
            ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_ERROR,
                "The size of data storage (%lu) is too small to save data (%lu)", ds_size, bson_file_len);
            return -1;  // over !
        }
    }

    rc = FS_NonVolatileDataStorage_Write(FS_TYPE_DATA_STORAGE_ID_ALU, bson_file->len, (u_char*)bson_get_data(bson_file)) ? 0 : -1;

    SHA256((u_char*)bson_get_data(bson_file), bson_file->len, ctx->storage.sha256);

    bson_destroy(bson_file);

    ctx->storage.data.drity = 0;

    return rc;
}

int
alu_client_load_storage(
    alu_client_ctx_t *ctx)
{
    int rc = 0;

    uint8_t *file_data = NULL;
    UI32_T  file_size;

    file_size = FS_NonVolatileDataStorage_GetSize(FS_TYPE_DATA_STORAGE_ID_ALU);

    file_data = (u_char *)calloc(file_size, 1);
    if (!file_data) {
        ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_ERROR, "Failed to allocate memory for keeping storage data");
        goto error;
    }

    if (FS_NonVolatileDataStorage_Read(FS_TYPE_DATA_STORAGE_ID_ALU, &file_size, file_data) != TRUE) {
        ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_ERROR, "Failed to read storage");
        goto error;
    }

    rc = alu_client_storage_loadb(ctx, file_data, file_size);

    if (0) {
    error:
        if (0 <= rc) {
            rc = -1;
        }
    }

    if (file_data) {
        free(file_data);
    }

    return rc;
}
#endif /* SYS_CPNT_FS_SUPPORT_DATA_STORAGE */

static int
alu_client_load_or_new_storage(
    alu_client_ctx_t *ctx)
{
    int rc = -1;

#if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE == TRUE)
    rc = alu_client_load_storage(ctx);
#endif /* SYS_CPNT_FS_SUPPORT_DATA_STORAGE */

    if (rc != 0 && ctx->storage.data.doc == NULL) {
        ctx->storage.major_version = 1;
        ctx->storage.minor_version = 0;
        ctx->storage.micro_version = 0;

        ctx->storage.data.doc = alu_client_doc_new();
        ctx->storage.data.drity = 1;
    }

    return rc;
}

static int
alu_client_restore_active_license_to_db(
    alu_client_ctx_t *ctx)
{
    alu_client_active_license_t *active;
    uint64_t up_time;

    active = &ctx->active_license;

    ASSERT(active && active->valid == 1 && alu_client_license_is_time_limit(&active->license));
    if (!active || !active->valid || !alu_client_license_is_time_limit(&active->license)) {
        return -1;
    }

    up_time = alu_client_time_get_up_time();

    ASSERT(active->start_record_time <= up_time);

    if (up_time < active->start_record_time) {
        active->start_record_time = up_time;  // restart !
    }

    active->license.stats.used_time += (up_time - active->start_record_time);

    ASSERT(active->license.stats.last_updated_time <= up_time);
    active->license.stats.last_updated_time = up_time;

    return alu_client_doc_write_active(&ctx->storage.data, active);
}

static void
alu_client_fetch_active_license(
    alu_client_ctx_t *ctx)
{
    alu_client_license_t *lic;

    json_t *json_lic;
    json_t *json_act;
    json_t *json_act_lic_no;
    uint64_t up_time;

    ctx->active_license.valid = 1;
    lic = &ctx->active_license.license;

    memset(lic, 0, sizeof(*lic));

    memcpy(lic->license_number, ctx->chk.license_number, sizeof(lic->license_number));
    lic->trial_time = ctx->chk.trial_time;
    lic->trial_period_days = ctx->chk.trial_period_days;

    // fetch from db
    json_lic = alu_client_doc_get_license(&ctx->storage.data, ctx->chk.license_number);

    if (json_lic) {
        lic->stats.used_time = json_integer_value( json_object_get(json_lic, "used-time") );
        lic->stats.total_time = ctx->chk.trial_period_days * 24 * 60 * 60;
        lic->stats.last_updated_time = json_integer_value( json_object_get(json_lic, "last-updated-time") );
    }
    else {
        lic->stats.used_time = 0;
        lic->stats.total_time = ctx->chk.trial_period_days * 24 * 60 * 60;

        up_time = alu_client_time_get_up_time();
        lic->stats.last_updated_time = up_time;
    }

    json_act = alu_client_doc_get_active(&ctx->storage.data);
    json_act_lic_no = json_object_get(json_act, "license-no");

    if (json_act && json_act_lic_no && json_typeof(json_act_lic_no) == JSON_STRING
        && strcmp(ctx->chk.license_number, json_string_value(json_act_lic_no)) == 0) {
        ctx->active_license.start_record_time = json_integer_value( json_object_get(json_act, "start-time") );
    }
    else {
        ctx->active_license.start_record_time = lic->stats.last_updated_time;
    }
}

static int
alu_client_updated_license_after_checker(
    alu_client_ctx_t *ctx)
{
    // If active license is different with the file (current be loaded).
    // Restore the usage to database, before changing the active.
    if (ctx->active_license.valid == 1 &&
        strcmp(ctx->active_license.license.license_number, ctx->chk.license_number) != 0 &&
        alu_client_license_is_time_limit(&ctx->active_license.license)) {

        alu_client_restore_active_license_to_db(ctx);
    }

    alu_client_fetch_active_license(ctx);

    if (alu_client_license_is_time_limit(&ctx->active_license.license) &&
        alu_client_active_is_expired(&ctx->active_license)) {

        ctx->active_license.valid = 0;
        alu_client_doc_write_active(&ctx->storage.data, &ctx->active_license);
    }

    // New license, start to record
    if (ctx->active_license.valid == 1 &&
        !alu_client_doc_get_license(&ctx->storage.data, ctx->chk.license_number)) {

        ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_INFO,
            "New license %s, active at %llu",
            ctx->active_license.license.license_number, ctx->active_license.license.stats.last_updated_time);
    }

    alu_client_doc_write_active(&ctx->storage.data, &ctx->active_license);

#if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE == TRUE)
    // sync to flash
    alu_client_save_storage(ctx);
#endif /* SYS_CPNT_FS_SUPPORT_DATA_STORAGE */

    return 0;
}

static int
alu_client_swap_active(
    alu_client_ctx_t *ctx)
{
    if (ALU_IS_VALID(&ctx->chk)) {
        return alu_client_updated_license_after_checker(ctx);
    }
    else {
        // clean active
        ctx->active_license.valid = 0;
        alu_client_doc_write_active(&ctx->storage.data, &ctx->active_license);

#if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE == TRUE)
        // sync to flash
        alu_client_save_storage(ctx);
#endif /* SYS_CPNT_FS_SUPPORT_DATA_STORAGE */

        return 0;
    }
}

static int
alu_client_check_expire(
    alu_client_ctx_t *ctx)
{
    alu_client_active_license_t *active = &ctx->active_license;

    if (active->valid == 1 && alu_client_license_is_time_limit(&active->license)) {
        uint64_t up_time;
        uint64_t used_time;
        time_t utc_time;

        up_time = alu_client_time_get_up_time();

        ASSERT(active->start_record_time <= up_time);

        // Current up-time before start-time of the active license !
        // It shall be impossible. If happen, change the start-time of the active license.
        if (up_time < active->start_record_time) {
            ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_ERROR,
                "current.upTime(%llu) before active.startTime(%llu), change active.startTime to %llu",
                up_time, active->start_record_time, up_time);

            active->start_record_time = up_time;
        }

        used_time = active->license.stats.used_time + ( up_time - active->start_record_time );

        ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_DEBUG,
            "current.upTime(%llu), active{ startTime(%llu), usedTime(%llu), total_time(%llu) }, already.usedTime(%llu = %llu + (%llu-%llu))",
            up_time, active->start_record_time, active->license.stats.used_time, active->license.stats.total_time,
            used_time, active->license.stats.used_time, up_time, active->start_record_time);

        utc_time = alu_client_time_get_utc_time();

        if (active->license.stats.total_time <= used_time || ctx->chk.license_valid_end_date < utc_time) {
            ctx->error(ctx);

            active->license.stats.used_time = used_time;
            active->license.stats.last_updated_time = up_time;

            alu_client_doc_write_license(&ctx->storage.data, &active->license);

            active->valid = 0;
            alu_client_doc_write_active(&ctx->storage.data, active);

#if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE == TRUE)
            // sync to flash
            alu_client_save_storage(ctx);
#endif /* SYS_CPNT_FS_SUPPORT_DATA_STORAGE */
        }
    }

    return 0;
}

static int
alu_client_validate_from_file(
    alu_client_ctx_t *ctx)
{
    int32_t         ret;
    time_t          now;
    STKTPLG_MGR_Switch_Info_T switch_info = {0};
    UI8_T           cpu_mac[6] = {0};

    alu_keys_t      keys;

    {
        UI32_T unit_id = 0;
        char buf[40] = {0};

        snprintf(buf, sizeof(buf) - 1, "valid-state=Validating license...");

        STKTPLG_POM_GetMyUnitID(&unit_id);
        if (FS_RETURN_OK != FS_WriteFile(unit_id,
                                         (unsigned char *)SYS_ADPT_LICENSE_RESULT_FILE_NAME,
                                         (UI8_T *)"aluc",
                                         FS_FILE_TYPE_LICENSE,
                                         (unsigned char *)buf,
                                         sizeof(buf), 0))
        {
            ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_ERROR, "Failed to save license.res");
        }
    }

    alu_checker_ctx_init(&ctx->chk);
    alu_checker_ctx_set_file(&ctx->chk, ctx->lic_buffer, ctx->lic_buffer_len);

    if (FALSE == STKTPLG_POM_GetLocalUnitBaseMac(cpu_mac)) {
        ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_ERROR, "Failed to get CPU MAC address");
    }

    alu_checker_ctx_set_sw_info(&ctx->chk, ALU_SWITCH_INFO_CPU_MAC_ADDRESS, cpu_mac, 6);

    switch_info.sw_unit_index = 1;
    if (STKTPLG_PMGR_GetSwitchInfo(&switch_info) != TRUE) {
        ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_ERROR, "Failed to get switch info");
    }

    alu_checker_ctx_set_sw_info(&ctx->chk, ALU_SWITCH_INFO_OPCODE_VERSION,
        switch_info.sw_opcode_ver, sizeof(switch_info.sw_opcode_ver) - 1);

    time(&now);
    alu_checker_ctx_set_sw_info(&ctx->chk, ALU_SWITCH_INFO_CURRENT_TIME, &now, sizeof(now));

    memset(&keys, 0, sizeof(keys));
    alu_get_keys(&keys.rsa[0].public_key, &keys.rsa[1].public_key, &keys.shared_key);
    keys.shared_key_len = strlen((char *)keys.shared_key);

    alu_checker_ctx_set_sw_info(&ctx->chk, ALU_SWITCH_INFO_KEYS, &keys, sizeof(keys));

    alu_checker_ctx_set_sw_info(&ctx->chk, ALU_SWITCH_INFO_OPCODE_MODE, "Legacy", -1);

    alu_checker_ctx_set_sw_info(&ctx->chk, ALU_SWITCH_INFO_ACTIVATION_CODE, ctx->ac_buffer, ctx->ac_buffer_len);


    ret = alu_checker_validate(&ctx->chk);

    if (ctx->chk.invalid_flag.reasons != 0) {
        char  reasons[200] = {0};
        char *p = reasons;
        int   len = sizeof(reasons);
        int   fmlen;
        int   truncate = 0;

#define INVALID_FLAG_TYPE(_bit, _str) {                                                     \
        if (ctx->chk.invalid_flag.reason._bit && 0 < len) {                                 \
            fmlen = snprintf(p, len, "%s%s", (len == sizeof(reasons) ? "" : ","), _str);    \
            if (fmlen < 0 || len <= fmlen) {                                                \
                *p = '\0';                                                                  \
                truncate = 1;                                                               \
            } else {                                                                        \
                p += fmlen;                                                                 \
                len -= fmlen;                                                               \
            }                                                                               \
        }                                                                                   \
    }
#include <alu_invalid_flag_type.h>

        if (truncate) {
            if (len < 4) {
                p = &reasons[sizeof(reasons) - 4];
            }

            *p++ = '.';
            *p++ = '.';
            *p++ = '.';
            *p = '\0';
        }

        ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_INFO,
            "Ivalid license with the reasons(0x%" PRIx64 " = %s)", ctx->chk.invalid_flag.reasons, reasons);
    }

    return ret;
}

static BOOL_T
alu_client_is_accept_port(
    alu_client_ctx_t *ctx,
    uint32_t ifindex)
{
    if (!ctx->active_license.valid) {
        return FALSE;
    }

    // valid license
    if (!ctx->chk.port_limited) {
        return TRUE;
    }

    // port-limited license
    if (0 == ifindex || _countof(ctx->chk.enabled_port_bmp) <= ((ifindex - 1) / 8)) {
        return FALSE;
    }

#define ALU_CLIENT_TEST(bitarray, bit) (bitarray[(bit) / 8] & (1 << ((bit) % 8)))

    return ALU_CLIENT_TEST(ctx->chk.enabled_port_bmp, ifindex - 1) ? TRUE : FALSE;
}

static int
alu_client_apply_action(
    alu_client_ctx_t *ctx)
{
    UI32_T ifindex;

    alu_client_range_extract_t enable_port;
    alu_client_range_extract_t disable_port;

    alu_client_range_extract_begin(&enable_port);
    alu_client_range_extract_begin(&disable_port);

    for (ifindex = 1; ifindex <= SYS_ADPT_TOTAL_NBR_OF_LPORT; ++ifindex) {
        BOOL_T status;

        status = alu_client_is_accept_port(ctx, ifindex);

        if (status) {
            UI32_T shutdown_reason;

            // If the port had been shutdown by alu-client, re-eanble it
            if (SWCTRL_POM_GetPortStatus(ifindex, &shutdown_reason) && (shutdown_reason & SWCTRL_PORT_STATUS_SET_BY_SW_LICENSE)) {

                alu_client_range_extract_add(&enable_port, ifindex);

                alu_client_set_port_status(ctx, ifindex, status);
            }
        }
        else {

            alu_client_range_extract_add(&disable_port, ifindex);

            alu_client_set_port_status(ctx, ifindex, status);
        }
    }

    alu_client_range_extract_end(&enable_port);
    alu_client_range_extract_end(&disable_port);

    if (*enable_port.str) {
        ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_INFO, "Enable port (%s)", enable_port.str);
    }

    if (*disable_port.str) {
        ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_INFO, "Disable port (%s)", disable_port.str);
    }

    // every N second, set limited port ...
    ctx->delay_error = TRUE;
    ctx->random_delay_time = (rand() % (ALU_CLIENT_MAX_DELAY_TIME - ALU_CLIENT_MIN_DELAY_TIME))
        + ALU_CLIENT_MIN_DELAY_TIME;

    return ALU_OK;
}

static int
alu_client_success(
    alu_client_ctx_t *ctx)
{
    {
        UI32_T unit_id = 0;
        char buf[200] = {0},
             time_buf[sizeof("YYYY-MM-DD hh:mm:ss") + 1] = {0};
        struct tm end_date;

        strftime(time_buf, sizeof(time_buf) - 1, "%Y-%m-%d %H:%M:%S", localtime(&ctx->chk.license_valid_end_date));

        gmtime_r(&ctx->chk.license_valid_end_date, &end_date);

        snprintf(buf, sizeof(buf) - 1,
            "valid-state=Your license is active.\r\n\tLicense valid till: %s\r\n\t%s: %s\r\n"
            "max-valid-version=%lu",
            time_buf,
            ALU_V1_HEADER_LICENSE_NUMBER, ctx->active_license.license.license_number,
            ((end_date.tm_year - 100) * 10) + (end_date.tm_mon / 3) + 1);

        STKTPLG_POM_GetMyUnitID(&unit_id);
        if (FS_RETURN_OK != FS_WriteFile(unit_id,
                                         (unsigned char *)SYS_ADPT_LICENSE_RESULT_FILE_NAME,
                                         (UI8_T *)"aluc",
                                         FS_FILE_TYPE_LICENSE,
                                         (unsigned char *)buf,
                                         sizeof(buf), 0))
        {
            ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_ERROR, "Failed to save license.res");
        }
    }

    return alu_client_apply_action(ctx);
}

static int
alu_client_error(
    alu_client_ctx_t *ctx)
{
    {
        UI32_T unit_id = 0;
        char buf[40] = {0};

        snprintf(buf, sizeof(buf) - 1, "valid-state=No active license available.");

        STKTPLG_POM_GetMyUnitID(&unit_id);
        if (FS_RETURN_OK != FS_WriteFile(unit_id,
                                         (unsigned char *)SYS_ADPT_LICENSE_RESULT_FILE_NAME,
                                         (UI8_T *)"aluc",
                                         FS_FILE_TYPE_LICENSE,
                                         (unsigned char *)buf,
                                         sizeof(buf), 0))
        {
            ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_ERROR, "Failed to save license.res");
        }
    }

    srand( (unsigned int)time(NULL) );

    ctx->delay_error = TRUE;
    ctx->random_delay_time = (rand()%(ALU_CLIENT_MAX_DELAY_TIME - ALU_CLIENT_MIN_DELAY_TIME))
                                                    + ALU_CLIENT_MIN_DELAY_TIME;

    ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_INFO, "Port will be shutdown after %d seconds",
        ctx->random_delay_time);

    return 0;
}

static int
alu_client_set_port_status(
    alu_client_ctx_t *ctx,
    uint32_t ifindex,
    BOOL_T status)
{
    SWCTRL_Lport_Type_T lport_type;
    UI32_T unit, port, trunk_id;

    lport_type = SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

    if (lport_type == SWCTRL_LPORT_NORMAL_PORT ||
        lport_type == SWCTRL_LPORT_TRUNK_PORT_MEMBER)
    {
        if (!SWDRV_SetPortStatusForLicense(unit, port, status)) {
            ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_ERROR, "SWDRV_SetPortStatusForLicense(%lu, %lu, %s) failed",
                unit, port, status == TRUE ? "TRUE" : "FALSE");
        }

        CMGR_SetPortStatus(ifindex, status, SWCTRL_PORT_STATUS_SET_BY_SW_LICENSE);
    }

    return ALU_OK;
}

static int
alu_client_range_extract_begin(
    alu_client_range_extract_t *ctx)
{
    memset(ctx, 0, sizeof(*ctx));

    ctx->range.begin = ALUC_RANGE_UNSPEC;
    ctx->range.end = ALUC_RANGE_UNSPEC;
    ctx->str_tail = ctx->str;
    ctx->str_rlen = sizeof(ctx->str);
    ctx->sep[0] = ',';
    ctx->sep[1] = '\0';

    return ALU_OK;
}

static int
alu_client_range_extract_flush(
    alu_client_range_extract_t *ctx)
{
    int fmlen;

    if (ctx->range.begin == ALUC_RANGE_UNSPEC) {
        return ALU_OK;
    }

#define sep (ctx->str_tail == ctx->str ? "" : ctx->sep)

    if (ctx->range.begin == ctx->range.end) {
        fmlen = snprintf(ctx->str_tail, ctx->str_rlen, "%s%lu", sep, ctx->range.begin);
    }
    else {
        fmlen = snprintf(ctx->str_tail, ctx->str_rlen, "%s%lu-%lu", sep, ctx->range.begin, ctx->range.end);
    }

#undef sep

    if (fmlen < 0 || ctx->str_rlen <= fmlen) {
        *ctx->str_tail = '\0';
        return ALU_ERROR;
    }
    else {
        ctx->str_rlen -= fmlen;
        ctx->str_tail += fmlen;
    }

    ctx->range.begin = ctx->range.end = ALUC_RANGE_UNSPEC;

    return ALU_OK;
}

static int
alu_client_range_extract_add(
    alu_client_range_extract_t *ctx,
    UI32_T n)
{
    if (ctx->range.begin == ALUC_RANGE_UNSPEC) {
        ctx->range.begin = ctx->range.end = n;
    }

    if ((ctx->range.end + 1) < n) {
        alu_client_range_extract_flush(ctx);

        ctx->range.begin = ctx->range.end = n;
    }
    else {
        ctx->range.end = n;
    }

    return ALU_OK;
}

static int
alu_client_range_extract_end(
    alu_client_range_extract_t *ctx)
{

    alu_client_range_extract_flush(ctx);

    return ALU_OK;
}

static int
alu_client_create_log_file(
    const char *path)
{
    FILE *file = NULL;

    file = fopen(path, "a");
    if (!file) {
        return -1;
    }

    return fileno(file);
}

static int
alu_client_on_cfg_file_changed(
    alu_client_file_watch_t *file,
    int event_no)
{
    FILE *f;
    alu_client_ctx_t *ctx = (alu_client_ctx_t *) file->data;

    json_error_t error;
    json_t *cfg;

    json_t *json_log_path;
    json_t *json_log_level;

    f = fopen(file->path, "r");

    if (!f) {
        if (ctx->cfg) {
            json_decref(ctx->cfg);
            ctx->cfg = NULL;
        }
        goto cfg_reset_to_default;
    }

    cfg = json_loadf(f, 0, &error);

    if (ctx->cfg) {
        json_decref(ctx->cfg);
        ctx->cfg = NULL;
    }

    ctx->cfg = cfg;

    if (!ctx->cfg) {
        goto cfg_reset_to_default;
    }

    json_log_level = json_object_get(cfg, "logger.level");
    if (json_log_level && json_typeof(json_log_level) == JSON_INTEGER) {
        int64_t lvl = json_integer_value(json_log_level);

        if (lvl <= ALU_CLIENT_LOG_LEVEL_LAST) {
            ctx->logger.level = (int)lvl;
        }
    }

    json_log_path = json_object_get(cfg, "logger.path");
    if (json_log_path && json_typeof(json_log_path) == JSON_STRING) {

        if (2 < ctx->logger.fd) {
            close(ctx->logger.fd);
        }

        if (strcmp(json_string_value(json_log_path), "console") == 0) {
            ctx->logger.fd = 1;
        }
        if (strcmp(json_string_value(json_log_path), "file") == 0) {
            ctx->logger.fd = alu_client_create_log_file(ALU_CLIENT_LOG_FILE_PATH);
        }
        else {
            ctx->logger.fd = alu_client_create_log_file(json_string_value(json_log_path));
        }

        ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_DEBUG, "hello");
    }

    return 0;

cfg_reset_to_default:
    // logger
    if (2 < ctx->logger.fd) {
        close(ctx->logger.fd);
        ctx->logger.fd = -1;
    }

    ctx->logger.level = ALU_CLIENT_DFLT_LOG_LEVEL;

    return 0;
}

static void
alu_client_master_process(
    alu_client_ctx_t *ctx)
{
#if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE == TRUE)
    alu_client_file_watch_t *fw_bson;
#endif

    alu_client_file_watch_t *fw_config;
    alu_client_file_watch_t *fw_license;
    alu_client_file_watch_t *fw_ac;

    alu_client_load_or_new_storage(ctx);

    ctx->error(ctx);

#if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE == TRUE)
    fw_bson = alu_client_file_watch_new("aluc.bson",                            // path, here is a fake path
                                        alu_client_on_bson_changed,             // notify callback
                                        ctx,                                    // customized data
                                        ALUC_FILE_WATCH_CHECK_CONTENT | ALUC_FILE_WATCH_IGNORE_FIRST,
                                        ALU_CLIENT_BSON_CHECK_TIMER_INTERVAL,   // interval (-1: use the default value)
                                        -1);                                    // hash size (-1: use the default value (32) for sha256)

    fw_bson->hash_fn = alu_client_bson_sha256;
#endif /* SYS_CPNT_FS_SUPPORT_DATA_STORAGE */

    fw_config = alu_client_file_watch_new(ALU_CLIENT_CFG_FILE_PATH,             // path
                                          alu_client_on_cfg_file_changed,       // notify callback
                                          ctx,                                  // customized data
                                          ALUC_FILE_WATCH_CHECK_CONTENT,        // flags
                                          -1,                                   // interval (-1: use default value)
                                          -1);                                  // hash size (-1: used default (32) value for sha256)

    fw_license = alu_client_file_watch_new(SYS_ADPT_LICENSE_FILE_PATH SYS_ADPT_LICENSE_FILE_NAME,
                                           alu_client_on_license_file_changed,
                                           ctx,
                                           ALUC_FILE_WATCH_CHECK_TIMESTAMPS_AND_SIZE,
                                           ALU_CLIENT_FILE_CHECK_TIMER_INTERVAL,
                                           -1);

    fw_ac = alu_client_file_watch_new(SYS_ADPT_LICENSE_FILE_PATH SYS_ADPT_LICENSE_AC_FILE_NAME,
                                           alu_client_on_activation_code_file_changed,
                                           ctx,
                                           ALUC_FILE_WATCH_CHECK_TIMESTAMPS_AND_SIZE,
                                           ALU_CLIENT_FILE_CHECK_TIMER_INTERVAL,
                                           -1);

    while(1) {
#if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE == TRUE)
        alu_client_file_watch(fw_bson);
#endif /* SYS_CPNT_FS_SUPPORT_DATA_STORAGE */
        alu_client_file_watch(fw_config);
        alu_client_file_watch(fw_license);
        alu_client_file_watch(fw_ac);

        if (ctx->expire_interval == ALU_CLIENT_EXPIRE_CHECK_TIMER_INTERVAL) {
            alu_client_on_check_expired(ctx);
            ctx->expire_interval = 0;
        }
        ctx->expire_interval++;

        if (ctx->delay_error == TRUE) {
            if (ctx->random_delay_time == 0) {
                alu_client_apply_action(ctx);
                if (ctx->random_delay_time == 0) {
                    ctx->delay_error = FALSE;
                }
            }
            else {
                ctx->random_delay_time--;
            }
        }

        SYSFUN_Sleep(100);
    }

#if (SYS_CPNT_FS_SUPPORT_DATA_STORAGE == TRUE)
    alu_client_file_watch_free(fw_bson);
#endif /* SYS_CPNT_FS_SUPPORT_DATA_STORAGE */
    alu_client_file_watch_free(fw_config);
    alu_client_file_watch_free(fw_license);
    alu_client_file_watch_free(fw_ac);
}

static alu_fd_t
alu_client_create_master_pidfile(
    alu_client_ctx_t *ctx)
{
#if (defined(PLATFORM_WINDOWS))
    BOOL    fSuccess = FALSE;
    DWORD   pid;
    DWORD   dwNumBytesWritten;
    char    buff[30];

    // Create the file, open for both read and write.
    alu_fd_t hFile = CreateFile(TEXT(ALU_CLIENT_PID_FILE_PATH),
                                GENERIC_READ | GENERIC_WRITE,   // dwDesiredAccess
                                FILE_SHARE_READ,                // dwShareMode
                                NULL,                           // lpSecurityAttributes
                                CREATE_ALWAYS,                  // dwCreationDisposition
                                0,                              // not overlapped index/O
                                NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_DEBUG, "CreateFile failed (%d)", GetLastError());
        return hFile;
    }

    pid = GetCurrentProcessId();
    dwNumBytesWritten = 0;

    snprintf(buff, sizeof(buff), "%d", pid);
    ctx->pid = pid;

    fSuccess = WriteFile(hFile,
                         buff,
                         strlen(buff),
                         &dwNumBytesWritten,
                         NULL);  // sync operation.
    if (!fSuccess) {
        ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_ERROR, "WriteFile failed (%d)", GetLastError());
    }

    return hFile;
#else
    alu_fd_t    fd = -1;
    int         lock;
    pid_t       pid;
    char        buff[30];
    int         flags = O_CREAT | O_RDWR;

#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif

    fd = open(ALU_CLIENT_PID_FILE_PATH, flags, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_ERROR, "fopen faild (%d) %s\r\n", errno, strerror(errno));
        return -1;
    }

    lock = flock(fd, LOCK_EX);
    if (lock == -1) {
        ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_ERROR, "flock faild (%d) %s\r\n", errno, strerror(errno));
        close(fd);
        return -1;
    }

    pid = getpid();
    ctx->pid = (int)pid;
    snprintf(buff, sizeof(buff), "%d", (int)pid);
    write(fd, buff, strlen(buff));

    return fd;
#endif /* PLATFORM_WINDOWS */
}

void
alu_client_main(
    void *param)
{
    alu_client_ctx_t   *ctx;
    alu_fd_t            pid_fd;

    assert(param);

    ctx = alu_client_new();

    if (!ctx) {
        return;
    }

    memcpy(&ctx->arg, param, sizeof(alu_init_arg_t));
    free(param);
    param = NULL;

check_master_again:
    pid_fd = alu_client_create_master_pidfile(ctx);

    if (pid_fd != ALU_INVALID_FILE) {
        ctx->process = ALUC_PROCESS_MASTER;
    }
    else {
        ctx->process = ALUC_PROCESS_SLAVE;
    }

    if (ctx->process != ALUC_PROCESS_MASTER) {
        SYSFUN_Sleep(100);
        goto check_master_again;
    }

    ALU_CLIENT_LOG(&ctx->logger, ALU_CLIENT_LOG_LEVEL_INFO, "Process %d start", ctx->pid);

    alu_client_master_process(ctx);

    alu_client_destroy(ctx);
}

UI32_T ALU_Client_Task_CreateTask(int proc)
{
    UI32_T thread_id;
    char *thread_name = NULL;
    alu_init_arg_t *arg_p=NULL;

#if 0
    /* For project that is under development and does not enable
     * license check yet
     */
    return 0;
#endif

    arg_p = (alu_init_arg_t *)malloc(sizeof(alu_init_arg_t));
    if (arg_p == NULL)
    {
        printf("%s, %d:failed to malloc arg_p for %d bytes.", __FUNCTION__, __LINE__, (int)sizeof(alu_init_arg_t));
        return 0;
    }

    switch (proc) {
        case ALU_L2_L4_PROC:
            thread_name = SYS_BLD_ALU_L2_L4_PROC_THREAD_NAME;
            break;
        case ALU_CORE_UITL_PROC:
            thread_name = SYS_BLD_ALU_CORE_UTIL_THERAD_NAME;
            break;
        case ALU_CLI_PROC:
            thread_name = SYS_BLD_ALU_CLI_THREAD_NAME;
            break;
        default:
            printf("%s: Can't get thread name. \r\b", __FUNCTION__);
            break;
    }

    /* create a thread for CSCA task only when it needs to take care of timer
     * event.
     */

    arg_p->process = proc;
    if(SYSFUN_SpawnThread(SYS_BLD_ALU_THREAD_PRIORITY,
                          SYS_BLD_ALU_THREAD_SCHED_POLICY,
                          thread_name,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          alu_client_main,
                          arg_p,
                          &thread_id)!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SpawnThread fail.\n", __FUNCTION__);
    }

    return thread_id;
}

#if defined(_MSC_VER)
# undef close
#endif


#if defined(_MSC_VER)
# undef fileno
#endif
