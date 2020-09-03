#include "sys_type.h"
#include "netsnmp_port.h"
//#include <timers.h>
#if 0
#include "sys_time.h"
#include "socket.h"
#endif

#if 0
 int gettimeofday(struct timeval *tv, struct timezone *tz)
 {
//  clockid_t         clock_id;
//  struct timespec res;
    UI32_T sys_up_time;
  
    /* unit is tick */
    MIB2_MGR_GetSysUpTime(&sys_up_time);
  
  //clock_getres(CLOCK_REALTIME, &res );
  
    tv->tv_sec = (long) (sys_up_time/100);
  tv->tv_usec = 0;

  return 0;
 }
#endif

#if 0
char * strcasestr (register const char *s, register const char *find)
{
    register char c, sc;
    register UI32_T len;

    if ((c = *find++) != 0)
    {
		c = tolower(c);
        len = strlen(find);
        do
        {
            do
            {
                if ((sc = *s++) == 0)
                    return NULL;
            } while (tolower(sc) != c);
        } while (strncasecmp(s, find, len) != 0);
        s--;
    }
    return (char *)s;
}
#endif

