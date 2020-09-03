#ifndef _ALU_CLIENT_TYPE_HEADER_H_
#define _ALU_CLIENT_TYPE_HEADER_H_

#include "alu_core.h"
#include <stdint.h>

#include "sys_type.h"    // TODO: can we remove this ?
#include "jansson.h"


enum {
    ALUC_PROCESS_UNSPEC,
    ALUC_PROCESS_MASTER,
    ALUC_PROCESS_SLAVE
};

enum {
    ALUC_FILE_WATCH_CHECK_TIMESTAMPS_AND_SIZE,
    ALUC_FILE_WATCH_CHECK_CONTENT,
    ALUC_FILE_WATCH_CHECK_TIMESTAMPS_AND_SIZE_THEN_CONTENT
};
#define ALUC_FILE_WATCH_CHECK_OP_MASK   0x000000ff
#define ALUC_FILE_WATCH_IGNORE_FIRST    0x00000100

enum {
    ALUC_FILE_WATCH_EVENT_CREATE,
    ALUC_FILE_WATCH_EVENT_CHANGE,
    ALUC_FILE_WATCH_EVENT_DELETE,
    ALUC_FILE_WATCH_EVENT_DELETE_SELF
};

enum {
    ALUC_FILE_WATCH_STATE_UNSPEC,
    ALUC_FILE_WATCH_STATE_NO_EXIST,
    ALUC_FILE_WATCH_STATE_EXIST
};

#define ALUC_FILE_WATCH_DFLT_FLAGS       ALUC_FILE_WATCH_CHECK_CONTENT
#define ALUC_FILE_WATCH_DFLT_HASH_SIZE   32
#define ALUC_FILE_WATCH_DFLT_INTERVAL    1

struct alu_init_arg
{
    int                             process;
};

typedef struct alu_init_arg alu_init_arg_t;

struct alu_client_license_stat
{
    uint64_t                        total_time;         // only for time-limit license
    uint64_t                        used_time;          // only for time-limit license
    uint64_t                        last_updated_time;
};

typedef struct alu_client_license_stat alu_client_license_stats_t;

struct alu_client_license
{
    char                            license_number[sizeof("xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx")];

    unsigned                        trial_time;
    uint32_t                        trial_period_days;
    unsigned                        port_limited : 1;
    unsigned char                   enabled_port_bmp[16];

    alu_client_license_stats_t      stats;
};

typedef struct alu_client_license alu_client_license_t;

struct alu_client_active_license
{
    unsigned int                    valid : 1;

    uint64_t                        start_record_time;  // valid if expire=0
    alu_client_license_t            license;
};

typedef struct alu_client_active_license alu_client_active_license_t;

struct alu_client_doc
{
    json_t                         *doc;
    unsigned int                    drity : 1;
};

typedef struct alu_client_doc alu_client_doc_t;

struct alu_client_storage
{
    uint16_t                        major_version;
    uint16_t                        minor_version;
    uint16_t                        micro_version;

    uint64_t                        last_write_time;

    uint8_t                         sha256[32];

    alu_client_doc_t                data;
};

typedef struct alu_client_storage alu_client_storage_t;

struct alu_client_file_watch {
    int                             flags;
    int                             interval;
    int                             curr_polling;

    char                            path[260];

    alu_file_info_t                 sb;
    int                             hash_size;
    u_char                         *hash;
    int                             has_file;

    void                           *data;

    uint32_t                        notify_count;

    int(*stat_fn)(struct alu_client_file_watch *ctx, alu_file_info_t *sb);
    int(*hash_fn)(struct alu_client_file_watch *ctx, u_char *hash);

    int(*notify_fn)(struct alu_client_file_watch *ctx, int event_no);
};

typedef struct alu_client_file_watch alu_client_file_watch_t;

struct alu_client_logger {
    int                             fd;
    int                             level;
};

typedef struct alu_client_logger alu_client_logger_t;

struct alu_client_ctx
{
    alu_init_arg_t                  arg;

    int                             process;
    int                             pid;

    alu_checker_ctx_t               chk;

    unsigned char                  *lic_buffer;
    size_t                          lic_buffer_max_size;
    size_t                          lic_buffer_len;

    unsigned char                  *ac_buffer;
    size_t                          ac_buffer_max_size;
    size_t                          ac_buffer_len;

    BOOL_T                          delay_error;
    int                             random_delay_time;
    int                             expire_interval;

    alu_client_active_license_t     active_license;

    alu_client_storage_t            storage;

    json_t                         *cfg;

    alu_client_logger_t             logger;

    int(*validate_from_file)(struct alu_client_ctx *ctx);
    int(*success)(struct alu_client_ctx *ctx);
    int(*error)(struct alu_client_ctx *ctx);
};

typedef struct alu_client_ctx alu_client_ctx_t;

#define ALUC_RANGE_UNSPEC          UINT32_MAX

struct alu_client_range
{
    UI32_T begin;
    UI32_T end;
};

typedef struct alu_client_range alu_client_range_t;

struct alu_client_range_extract
{
    alu_client_range_t              range;
    char                            str[300];
    int                             str_rlen;       // remained length
    char                           *str_tail;
    char                            sep[5];
};

typedef struct alu_client_range_extract alu_client_range_extract_t;

#endif /* _ALU_CLIENT_TYPE_HEADER_H_ */
