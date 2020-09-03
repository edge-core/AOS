//
//  http_log.h
//  http
//
//  Created by JunYing Yeh on 2014/7/18.
//
//

#ifndef _HTTP_HEADER_LOG_H_
#define _HTTP_HEADER_LOG_H_

#include "http_type.h"

#if __cplusplus
extern "C" {
#endif

enum
{
    HTTP_LOG_LOCK   = 1,
    HTTP_LOG_UNLOCK = 2,
    HTTP_LOG_READ   = 4,
    HTTP_LOG_WRITE  = 8
};

typedef enum
{
    HTTP_LOG_LEVEL_EMERG    = 0,
    HTTP_LOG_LEVEL_ALERT    = 1,
    HTTP_LOG_LEVEL_CRIT     = 2,
    HTTP_LOG_LEVEL_ERROR    = 3,
    HTTP_LOG_LEVEL_WARN     = 4,
    HTTP_LOG_LEVEL_NOTICE   = 5,
    HTTP_LOG_LEVEL_INFO     = 6,
    HTTP_LOG_LEVEL_DEBUG    = 7,
    HTTP_LOG_LEVEL_UNKNOWN  = 8,
    HTTP_LOG_LEVEL_LAST     = HTTP_LOG_LEVEL_UNKNOWN
} HTTP_LOG_LEVEL_T;

typedef enum
{
#define MSGTYPE(no, str)    HTTP_LOG_MSG_##no,
#include "http_msgtype.h"
    HTTP_LOG_MSG_UNKNOWN,
    HTTP_LOG_MSG_LAST = HTTP_LOG_MSG_UNKNOWN
} HTTP_LOG_MSG_TYPE_T;

typedef enum
{
    HTTP_LOG_LOGGER_SYSLOG  = 0,
    HTTP_LOG_LOGGER_CONSOLE = 1,
    HTTP_LOG_LOGGER_FILE    = 2,
    HTTP_LOG_LOGGER_UNKNOWN = 3,
    HTTP_LOG_LOGGER_LAST    = HTTP_LOG_LOGGER_UNKNOWN
} HTTP_LOG_LOGGER_T;

// FIXME: It shall not need to export. Check others.
typedef void (* HTTP_LOG_LOGGER_FN_T) (const char *, UI32_T, HTTP_LOG_LEVEL_T, int);

typedef struct
{
    unsigned char           enabled[4];
} HTTP_LOG_TYPE_T;

typedef struct
{
    HTTP_LOG_LOGGER_FN_T    logger_fn;
    HTTP_LOG_LEVEL_T        level;
    HTTP_LOG_TYPE_T         types;
} HTTP_LOG_CONFIG_T;

#define HTTP_LOG_DFLT_LEVEL     HTTP_LOG_LEVEL_ERROR

#define HTTP_LOG_ERROR(level, msgtype, funcno, fmt, ...) \
    http_log_core(SYS_MODULE_HTTP, level, msgtype, funcno, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define http_log_error(module, level, msgtype, funcno, fmt, ...) \
    http_log_core(module, level, msgtype, funcno, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

void http_log_init();
void http_log_close();

void http_log_core(UI32_T module, HTTP_LOG_LEVEL_T level, HTTP_LOG_MSG_TYPE_T msgtype, int funcno, const char *func, int line, const char *fmt, ...);
void http_log_dump();

void http_log_config_reset_to_default();
BOOL_T http_log_set_logger(HTTP_LOG_LOGGER_T logger);
BOOL_T http_log_set_level(int level);
BOOL_T http_log_set_level_by_string(const char *level);
BOOL_T http_log_enable_debug(HTTP_LOG_TYPE_T *types);

#if __cplusplus
}
#endif

#endif