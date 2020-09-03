
#include "alu_client_private.h"

#ifdef PLATFORM_WINDOWS
static void
UnixTimeToFileTime(
    time_t t,
    LPFILETIME pft)
{
    // Note that LONGLONG is a 64-bit value
    LONGLONG ll;

    ll = Int32x32To64(t, 10000000) + 116444736000000000;
    pft->dwLowDateTime = (DWORD)ll;
    pft->dwHighDateTime = ll >> 32;
}

static void
UnixTimeToSystemTime(
    time_t t,
    LPSYSTEMTIME pst)
{
    FILETIME ft;

    UnixTimeToFileTime(t, &ft);
    FileTimeToSystemTime(&ft, pst);
}

struct tm *
localtime_r(
    const time_t *timer,
    struct tm *result)
{
    SYSTEMTIME st;
    UnixTimeToSystemTime(*timer, &st);

    memset(result, 0, sizeof(*result));

    result->tm_year = st.wYear - 1900;
    result->tm_mon = st.wMonth - 1;
    result->tm_mday = st.wDay;
    result->tm_wday = st.wDayOfWeek;
    result->tm_hour = st.wHour;
    result->tm_min = st.wMinute;
    result->tm_sec = st.wSecond;

    return result;
}
#endif /* PLATFORM_WINDOWS */

static const char *alu_client_log_err_levels[] = {
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

void
alu_client_log(
    const alu_client_logger_t *logger,
    int level,
    const char *func,
    int line,
    const char *fmt, ...)
{
    va_list     args;

    struct tm   occurred_time;

    char       *p;
    char        errstr[300];

    int         len;

    if (!logger || logger->fd < 0) {
        return;
    }

    if (ALU_CLIENT_LOG_LEVEL_LAST < level) {
        level = ALU_CLIENT_LOG_LEVEL_LAST;
    }

    if (logger->level < level) {
        return;
    }

    p = errstr;
    len = 0;

    {
        time_t  sec;

        time(&sec);

        localtime_r(&sec, &occurred_time);

        occurred_time.tm_year += 1900;
        occurred_time.tm_mon += 1;
    }

    {
        len = snprintf(p, sizeof(errstr), "%04d-%02d-%02d %02d:%02d:%02d %s ",
                       occurred_time.tm_year, occurred_time.tm_mon, occurred_time.tm_mday,
                       occurred_time.tm_hour, occurred_time.tm_min, occurred_time.tm_sec,
                       alu_client_log_err_levels[level]);
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
            *sp = '\0';
            //ASSERT(*sp == '\0');

            len = sizeof(errstr) - 1;
        }
        else if (len < 0)
        {
            ASSERT(0);
            return;
        }
    }

    ASSERT(len < sizeof(errstr));

    p += len;

    va_start(args, fmt);
    vsnprintf(p, sizeof(errstr) - len, fmt, args);
    errstr[sizeof(errstr) - 1] = '\0';
    va_end(args);

#if defined(_MSC_VER)
# define write(FileHandle, Buf, MaxCharCount) _write(FileHandle, Buf, MaxCharCount)
#endif

    write(logger->fd, errstr, strlen(errstr));

#if defined(_MSC_VER)
# undef write
#endif
}
