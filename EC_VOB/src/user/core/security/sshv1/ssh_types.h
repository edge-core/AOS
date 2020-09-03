/* Isiah. 2002-05-27 */
#ifndef SSH_TYPES_H
#define SSH_TYPES_H

#include <sys/types.h>
#include "socket.h"


#ifndef u_int32_t
#define u_int32_t unsigned long
#endif

#ifndef u_int16_t
#define u_int16_t unsigned short
#endif

#ifndef u_int8_t
#define u_int8_t unsigned char
#endif

typedef	unsigned char	u_char;

#if 0
struct passwd
{
	char	*pw_nsme;
    char    *pw_passwd;
    uid_t   pw_uid;
    gid_t   pw_gid;
    time_t  pw_change;
    char    *pw_class;
};

#define MAXINTERP	32		/* max interpreter file name length */
#define MAXPATHLEN	PATH_MAX
#define PATH_MAX	1024	/* max bytes in pathname */
#define MAXLOGNAME	17		/* max login name length (incl. NUL) */
#define NSIG		32		/* number of old signals (counting 0) */
#endif


#if 0
struct timeval
{
	long tv_sec;		/* seconds */
	long tv_usec;		/* and microseconds */
};


#define ITIMER_REAL		0
#define ITIMER_VIRTUAL	1
#define ITIMER_PROF		2

struct itimerval
{
	struct timeval it_interval;		/* timer interval */
	struct timeval it_value;		/* current value */
};
#endif


/* macros for Unix routines */
#define roundup(x, y)   ((((x)+((y)-1))/(y))*(y))


/*isiah.2002-07-11*/
#ifndef ENOMEM
#define	ENOMEM		-26		/* Not enough core */
#endif
#ifndef ENOSPC
#define	ENOSPC		-27		/* No space left on device */
#endif
#ifndef EAGAIN
#define	EAGAIN		-28		/* No more processes */
#endif
#ifndef EINTR
#define	EINTR		-29		/* Interrupted system call */
#endif
#ifndef E2BIG
#define	E2BIG		-30		/* Arg list too long */
#endif



#endif /* ifndef SSH_TYPES_H */