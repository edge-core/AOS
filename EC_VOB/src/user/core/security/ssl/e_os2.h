/* e_os2.h */

#ifndef HEADER_E_OS2_H
#define HEADER_E_OS2_H
#include "linlux_platform.h"

/*isiah.2004-01-08*/
/*isiah.2003-01-15*/
#if 0 
#define time_t  unsigned long
#endif
//#define size_t  unsigned long
//#define NULL    0
//#define pid_t   long

/* does not need this for linux platform, 
 *  use <time.h> instead.
 */
#if 0
struct tm
	{
	int tm_sec;	/* seconds after the minute	- [0, 59] */
	int tm_min;	/* minutes after the hour	- [0, 59] */
	int tm_hour;	/* hours after midnight		- [0, 23] */
	int tm_mday;	/* day of the month		- [1, 31] */
	int tm_mon;	/* months since January		- [0, 11] */
	int tm_year;	/* years since 1900	*/
	int tm_wday;	/* days since Sunday		- [0, 6] */
	int tm_yday;	/* days since January 1		- [0, 365] */
	int tm_isdst;	/* Daylight Saving Time flag */
	};
#endif

#include <opensslconf.h> /* OPENSSL_UNISTD */

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef MSDOS
# define OPENSSL_UNISTD_IO <io.h>
# define OPENSSL_DECLARE_EXIT extern void exit(int);
#else
# define OPENSSL_UNISTD_IO OPENSSL_UNISTD
# define OPENSSL_DECLARE_EXIT /* declared in unistd.h */
#endif

/* Definitions of OPENSSL_GLOBAL and OPENSSL_EXTERN,
   to define and declare certain global
   symbols that, with some compilers under VMS, have to be defined and
   declared explicitely with globaldef and globalref.  On other OS:es,
   these macros are defined with something sensible. */

#if defined(VMS) && !defined(__DECC)
# define OPENSSL_EXTERN globalref
# define OPENSSL_GLOBAL globaldef
#else
# define OPENSSL_EXTERN extern
# define OPENSSL_GLOBAL
#endif

#ifdef  __cplusplus
}
#endif
#endif

