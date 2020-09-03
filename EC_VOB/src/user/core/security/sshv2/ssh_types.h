#ifndef SSH_TYPES_H
#define SSH_TYPES_H

#include "sys_type.h"

#ifndef u_int
#define u_int   unsigned int
#endif

#ifndef u_char
#define u_char  unsigned char
#endif

#ifndef u_int32_t
#define u_int32_t   UI32_T
#endif

#ifndef u_short
#define u_short unsigned short
#endif

#ifndef u_long
#define u_long  unsigned long
#endif

#ifndef u_int64_t
#define u_int64_t   unsigned long long
#endif

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 	64
#endif

#ifndef EAGAIN
#define	EAGAIN		-28		/* No more processes */
#endif

#ifndef EINTR
#define	EINTR		-29		/* Interrupted system call */
#endif

#ifndef u_int8_t
#define u_int8_t    UI8_T
#endif

#ifndef u_int16_t
#define u_int16_t   UI16_T
#endif

#ifndef IPPORT_RESERVED
#define	IPPORT_RESERVED		1024
#endif

#ifndef IPPORT_USERRESERVED
#define	IPPORT_USERRESERVED	5000
#endif

#ifndef AF_UNSPEC
#define	AF_UNSPEC	0
#endif
typedef unsigned char sig_atomic_t;

#ifndef UINT_MAX
#define UINT_MAX    0xffffffffU
#endif

#ifndef	MAX
#define MAX(x1,x2)	((x1) >= (x2) ? (x1) : (x2))
#endif

#ifndef	MIN
#define MIN(x1,x2)	((x1) <= (x2) ? (x1) : (x2))
#endif

#ifndef roundup
/* macros for Unix routines */
#define roundup(x, y)   ((((x)+((y)-1))/(y))*(y))
#endif

#ifndef strsep
#define strsep(s,d) xstrsep((s),(d))
#endif

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * This is an strsep function that returns a null field for adjacent
 * separators.  This is the same as the 4.4BSD strsep, but different from the
 * one in the GNU libc.
 */
char *xstrsep(char **str, const char *delim);

#ifdef	__cplusplus
}
#endif

struct fatal_cleanup {
	struct fatal_cleanup *next;
	void (*proc) (void *);
	void *context;
};

#endif
