//
//  http_time_win.c
//  http
//
//  Created by JunYing Yeh on 2014/7/28.
//
//

#include "http_loc.h"

#ifdef PLATFORM_WINDOWS

static void
UnixTimeToFileTime(time_t t, LPFILETIME pft)
{
    // Note that LONGLONG is a 64-bit value
    LONGLONG ll;

    ll = Int32x32To64(t, 10000000) + 116444736000000000;
    pft->dwLowDateTime = (DWORD)ll;
    pft->dwHighDateTime = ll >> 32;
}

static void
UnixTimeToSystemTime(time_t t, LPSYSTEMTIME pst)
{
    FILETIME ft;

    UnixTimeToFileTime(t, &ft);
    FileTimeToSystemTime(&ft, pst);
}

int
gettimeofday(struct timeval *tp, void *tzp)
{
    uint64_t  intervals;
    FILETIME  ft;

    GetSystemTimeAsFileTime(&ft);

    /*
     * A file time is a 64-bit value that represents the number
     * of 100-nanosecond intervals that have elapsed since
     * January 1, 1601 12:00 A.M. UTC.
     *
     * Between January 1, 1970 (Epoch) and January 1, 1601 there were
     * 134744 days,
     * 11644473600 seconds or
     * 11644473600,000,000,0 100-nanosecond intervals.
     *
     * See also MSKB Q167296.
     */

    intervals = ((uint64_t) ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    intervals -= 116444736000000000;

    tp->tv_sec = (long) (intervals / 10000000);
    tp->tv_usec = (long) ((intervals % 10000000) / 10);

    return 0;
}

struct tm *
localtime_r(const time_t *timer, struct tm *result)
{
   SYSTEMTIME st;
   UnixTimeToSystemTime(*timer, &st);

   memset(result, 0, sizeof(*result));

   result->tm_year  = st.wYear - 1900;
   result->tm_mon = st.wMonth - 1;
   result->tm_mday = st.wDay;
   result->tm_wday = st.wDayOfWeek;
   result->tm_hour = st.wHour;
   result->tm_min = st.wMinute;
   result->tm_sec = st.wSecond;

   return result;
}

#endif //PLATFORM_WINDOWS