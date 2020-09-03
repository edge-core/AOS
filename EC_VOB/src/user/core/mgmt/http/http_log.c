//
//  http_log.c
//  http
//
//  Created by JunYing Yeh on 2014/7/18.
//
//
#include <pthread.h>

#include "http_loc.h"

#if (SYS_CPNT_SYSLOG == TRUE)
#include "syslog_mgr.h"
#include "syslog_pmgr.h"
#include "eh_type.h"
#include "eh_mgr.h"
#endif /* SYS_CPNT_SYSLOG */

#include "sys_time.h"

#define HTTP_LOG_DEBUG

#define HTTP_LOG_THREAD
#define HTTP_LOG_LOG_PREFIX_STR_LEN     64

static void http_log_write_log_nothing(const char *log, UI32_T module, HTTP_LOG_LEVEL_T level, int funcno);
static void http_log_write_log_to_syslog(const char *log, UI32_T module, HTTP_LOG_LEVEL_T level, int funcno);
static void http_log_write_log_to_console(const char *log, UI32_T module, HTTP_LOG_LEVEL_T level, int funcno);

static const char *http_log_err_levels[] = {
    "EMERG",
    "ALERT",
    "CRIT",
    "ERROR",
    "WARN",
    "NOTICE",
    "INFO",
    "DEBUG",
    ""
};

static const char *http_log_err_msgtypes[] = {
#define MSGTYPE(no, str)    str,
#include "http_msgtype.h"
    ""
};

static const char *http_log_module[] = {
    SYSMOD_LIST(MODULE_NAME)
};

#ifdef HTTP_LOG_THREAD
static pthread_rwlock_t       http_log_rwlock;
#endif

static void (*http_log_lock_fn)(int)   = NULL;
static void (*http_log_unlock_fn)(int) = NULL;
static int  http_log_lock_count = 0;

static void http_log_lock(int flags);
static void http_log_unlock(int flags);

static HTTP_LOG_CONFIG_T http_log_config;

// TODO: Is really use [HTTP_LOG_LOGGER_LAST] ?? use [] instead of ?
static void (*http_log_logger_fn[HTTP_LOG_LOGGER_LAST])(const char *, UI32_T, HTTP_LOG_LEVEL_T, int);

/* EXPORTED SUBPROGRAM BODIES
 */
void http_log_init()
{
#ifdef HTTP_LOG_THREAD
    {
        int rc;

        rc = pthread_rwlock_init(&http_log_rwlock, NULL);
    }
#endif

    // HTTP_OM_InitLog(); // call this api in HTTP_OM_InitateSystemResource()

    http_log_lock(HTTP_LOG_WRITE); // TODO: Is really need to LOCK in init function ?

    {
        UI32_T i;

        for (i = 0; i < _countof(http_log_logger_fn); ++i)
        {
            http_log_logger_fn[i] = http_log_write_log_nothing;
        }
    }

    http_log_logger_fn[HTTP_LOG_LOGGER_SYSLOG]  = http_log_write_log_to_syslog;
    http_log_logger_fn[HTTP_LOG_LOGGER_CONSOLE] = http_log_write_log_to_console;

    http_log_unlock(HTTP_LOG_UNLOCK);
}

void http_log_config_reset_to_default()
{
    http_log_lock(HTTP_LOG_WRITE);

    http_log_config.logger_fn = http_log_write_log_nothing;

    http_log_config.level = HTTP_LOG_DFLT_LEVEL;

    memset(&http_log_config.types, 0xff, sizeof(http_log_config.types));

    http_log_unlock(HTTP_LOG_UNLOCK);
}

BOOL_T http_log_set_logger(HTTP_LOG_LOGGER_T logger)
{
    if (logger < 0 || _countof(http_log_logger_fn) <= logger)
    {
        return FALSE;
    }

    http_log_lock(HTTP_LOG_WRITE);

    http_log_config.logger_fn = http_log_logger_fn[logger];

    http_log_unlock(HTTP_LOG_UNLOCK);

    return TRUE;
}

BOOL_T http_log_set_level(int level)
{
    if (level < HTTP_LOG_LEVEL_EMERG || HTTP_LOG_LEVEL_DEBUG < level)
    {
        return FALSE;
    }

    http_log_lock(HTTP_LOG_WRITE);

    http_log_config.level = (HTTP_LOG_LEVEL_T) level;

    http_log_unlock(HTTP_LOG_UNLOCK);

    return TRUE;
}

BOOL_T http_log_set_level_by_string(const char *level)
{
    UI32_T lv;

    if (level == NULL)
    {
        return FALSE;
    }

    for (lv = 0; lv < _countof(http_log_err_levels); ++lv)
    {
        if (strcasecmp(level, http_log_err_levels[lv]) == 0)
        {
            return http_log_set_level(lv);
        }
    }

    return FALSE;
}

BOOL_T http_log_enable_debug(HTTP_LOG_TYPE_T *types)
{
    if (types == NULL)
    {
        return FALSE;
    }

    http_log_lock(HTTP_LOG_WRITE);

    memcpy(&http_log_config.types, types, sizeof(http_log_config.types));

    http_log_unlock(HTTP_LOG_UNLOCK);

    return TRUE;
}

void http_log_close()
{
}

int http_log_set_lock_fn(void (*lock_fn)(int))
{
    if (http_log_lock_count != 0)
    {
        return -1;
    }

    http_log_lock_fn = lock_fn;
    return 0;
}

int http_log_set_unlock_fn(void (*unlock_fn)(int))
{
    if (http_log_lock_count != 0)
    {
        return -1;
    }

    http_log_unlock_fn = unlock_fn;
    return 0;
}

void http_log_core(UI32_T module, HTTP_LOG_LEVEL_T level, HTTP_LOG_MSG_TYPE_T msgtype, int funcno, const char *func, int line, const char *fmt, ...)
{
    va_list     args;

    struct tm   occurred_time;

    char       *p;
    char        errstr[HTTP_LOG_LOG_PREFIX_STR_LEN + HTTP_CFG_LOG_MAX_MESSAGE_STR_LEN + 1];

    int         len;

    if (SYS_MODULE_UNKNOWN < module)
    {
        module = SYS_MODULE_UNKNOWN;
    }

    if (HTTP_LOG_LEVEL_LAST < level)
    {
        level = HTTP_LOG_LEVEL_LAST;
    }

    if (HTTP_LOG_MSG_LAST < msgtype)
    {
        msgtype = HTTP_LOG_MSG_LAST;
    }

    if (http_log_config.level < level || HTTP_IS_BIT_OFF(http_log_config.types.enabled, msgtype))
    {
        return;
    }

    p = errstr;
    len = 0;

    {
        time_t  sec;

        time(&sec);

        localtime_r(&sec, &occurred_time);

        occurred_time.tm_year += 1900;
        occurred_time.tm_mon  += 1;
    }

    {
        len = snprintf(p, sizeof(errstr), "%04d-%02d-%02d %02d:%02d:%02d %s [%s %s %d(%s:%d)] ",
                       occurred_time.tm_year, occurred_time.tm_mon, occurred_time.tm_mday,
                       occurred_time.tm_hour, occurred_time.tm_min, occurred_time.tm_sec,
                       http_log_err_levels[level],
                       http_log_module[module], http_log_err_msgtypes[msgtype], funcno, func, line);
        errstr[sizeof(errstr) - 1] = '\0';

        if (sizeof(errstr) <= len)
        {
            /* Cut the tail
             */
            char *sp = &errstr[sizeof(errstr) - 5];

            *sp++ = '^';
            *sp++ = ')';
            *sp++ = ']';
            *sp++ = ' ';

            ASSERT(*sp == '\0');

            len = sizeof(errstr) - 1;
        }
    }

    ASSERT(len < sizeof(errstr));

    p += len;

    va_start(args, fmt);
    vsnprintf(p, sizeof(errstr) - len, fmt, args);
    errstr[sizeof(errstr) - 1] = '\0';
    va_end(args);

    {
        http_log_lock(HTTP_LOG_READ);

        ASSERT(http_log_config.logger_fn);

        http_log_config.logger_fn(errstr, module, level, funcno);

        http_log_unlock(HTTP_LOG_UNLOCK);
    }

    {
        HTTP_OM_AddLog(&occurred_time, level, msgtype, func, line, p);
    }

#if (SYS_CPNT_EH == TRUE)
    EH_PMGR_Handle_Exception1(SYS_MODULE_HTTP, funcno, msgtype, level, errstr);
#endif /* SYS_CPNT_EH */
}

/* LOCAL SUBPROGRAM BODIES
 */
static void http_log_lock(int flags)
{
    if (http_log_lock_fn)
    {
        http_log_lock_fn(flags);
    }
    else
    {
#ifdef HTTP_LOG_THREAD
        if (flags & HTTP_LOG_READ)
        {
            pthread_rwlock_rdlock(&http_log_rwlock);
        }
        else if (flags & HTTP_LOG_WRITE)
        {
            pthread_rwlock_wrlock(&http_log_rwlock);
        }
#endif /* HTTP_LOG_THREAD */
    }

    http_log_lock_count ++;
}

static void http_log_unlock(int flags)
{
    http_log_lock_count --;

    if (http_log_unlock_fn)
    {
        http_log_unlock_fn(flags);
    }
    else
    {
#ifdef HTTP_LOG_THREAD
        pthread_rwlock_unlock(&http_log_rwlock);
#endif /* HTTP_LOG_THREAD */
    }
}

static void http_log_write_log_nothing(const char *log, UI32_T module, HTTP_LOG_LEVEL_T level, int funcno)
{
}

static void http_log_write_log_to_syslog(const char *log, UI32_T module, HTTP_LOG_LEVEL_T level, int funcno)
{
#if (SYS_CPNT_SYSLOG == TRUE)

    SYSLOG_OM_RecordOwnerInfo_T record;

    ASSERT(log);

    memset(&record, 0, sizeof(record));

    record.level = level;
    record.module_no = module;
    record.function_no = funcno;
    record.error_no = 0;
    SYSLOG_PMGR_AddFormatMsgEntry(&record, HTTP_LOG_MESSAGE_INDEX, log, 0, 0);

#endif /* SYS_CPNT_SYSLOG */
}

static void http_log_write_log_to_console(const char *log, UI32_T module, HTTP_LOG_LEVEL_T level, int funcno)
{
    ASSERT(log);

    printf("%s\r\n", log);
}
