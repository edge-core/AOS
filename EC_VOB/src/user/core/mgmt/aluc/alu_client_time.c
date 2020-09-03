
#include "alu_client_private.h"

#include "sys_bld.h"
#include "sys_time.h"

// return seconds since 00:00 hours, Jan 1, 1970 UTC
time_t
alu_client_time_get_utc_time()
{
    struct tm timeinfo;
    int   year;
    int   month;
    int   day;
    int   hour;
    int   minute;
    int   second;

    SYS_TIME_GetRealTimeClock(&year, &month, &day, &hour, &minute, &second);

    memset(&timeinfo, 0, sizeof(timeinfo));
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;

    return mktime(&timeinfo);
}

// return seconds since system boot
uint64_t
alu_client_time_get_up_time()
{
    uint64_t up_time;

#if (SYS_CPNT_SYS_TIME_ACCUMULATED_SYSTEM_UP_TIME == TRUE)

    SYS_TIME_AccumulatedUpTime_Get(&up_time);

    up_time *= 60;
#else
    UI32_T ticks;

    SYS_TIME_GetSystemUpTimeByTick(&ticks);

    up_time = ticks / SYS_BLD_TICKS_PER_SECOND;
#endif

    return up_time;
}
