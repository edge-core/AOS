/************************************************************************/
/*                                                                      */
/*   MODULE: psys.h                                                     */
/*   PRODUCT: pSH+                                                      */
/*   PURPOSE:                                                           */
/*   DATE:  08/05/1992                                                  */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*              Copyright 1992, Integrated Systems Inc.                 */
/*                      ALL RIGHTS RESERVED                             */
/*                                                                      */
/*   This computer program is the property of Integrated Systems Inc.   */
/*   Santa Clara, California, U.S.A. and may not be copied              */
/*   in any form or by any means, whether in part or in whole,          */
/*   except under license expressly granted by Integrated Systems Inc.  */
/*                                                                      */
/*   All copies of this program, whether in part or in whole, and       */
/*   whether modified or not, must display this and all other           */
/*   embedded copyright and ownership notices in full.                  */
/*                                                                      */
/************************************************************************/
#ifndef __psys_h
#define __psys_h

#define MAX_CMDOPT  24
#define X_OK        1
#define W_OK        2
#define R_OK        4
#define F_OK        0

#define isascii(x)  ((unsigned)(x)<=0177)
#define toascii(x)  ((x)&0177)

#define fd_SET(n, p) \
        ((p)->fds_bits[0] |= (((unsigned long) 0x80000000) >> (n)))
#define fd_ISSET(n, p) \
        ((p)->fds_bits[0] & (((unsigned long) 0x80000000) >> (n)))

/*
 * Internal directory structure.
 */
#define FS_NODE_LEN     32      /** max for ANY file system type **/

typedef struct pdir {
    unsigned long  fn;
    char           name[FS_NODE_LEN + 1];
} pdir_t;

/*
 * read_dir() directory structure.
 */
#define MAXNAMLEN       255

struct pdate_t {
    unsigned short year;
    unsigned char  month;
    unsigned char  day;
};

typedef struct pdate_t pdate_t;

struct ptime_t {
    unsigned char  unused;
    unsigned char  hour;
    unsigned char  minute;
    unsigned char  second;
};
typedef struct ptime_t ptime_t;

struct  tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    char    *tm_zone;
    long    tm_gmtoff;
};

#define TM_SUNDAY       0
#define TM_MONDAY       1
#define TM_TUESDAY      2
#define TM_WEDNESDAY    3
#define TM_THURSDAY     4
#define TM_FRIDAY       5
#define TM_SATURDAY     6

#define TM_JANUARY      0
#define TM_FEBRUARY     1
#define TM_MARCH        2
#define TM_APRIL        3
#define TM_MAY          4
#define TM_JUNE         5
#define TM_JULY         6
#define TM_AUGUST       7
#define TM_SEPTEMBER    8
#define TM_OCTOBER      9
#define TM_NOVEMBER     10
#define TM_DECEMBER     11
#define TM_YEAR_BASE    1900

#define EPOCH_YEAR      1970
#define EPOCH_WDAY      TM_THURSDAY

#define SECSPERMIN      60
#define MINSPERHOUR     60
#define HOURSPERDAY     24
#define DAYSPERWEEK     7
#define DAYSPERNYEAR    365
#define DAYSPERLYEAR    366
#define SECSPERHOUR     (SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY      ((long) SECSPERHOUR * HOURSPERDAY)
#define MONSPERYEAR     12

#define SECS_PER_MIN    SECSPERMIN
#define MINS_PER_HOUR   MINSPERHOUR
#define HOURS_PER_DAY   HOURSPERDAY
#define DAYS_PER_WEEK   DAYSPERWEEK
#define DAYS_PER_NYEAR  DAYSPERNYEAR
#define DAYS_PER_LYEAR  DAYSPERLYEAR
#define SECS_PER_HOUR   SECSPERHOUR
#define SECS_PER_DAY    SECSPERDAY
#define MONS_PER_YEAR   MONSPERYEAR

#define SEC_PER_MIN     60
#define SEC_PER_HOUR    (60*60)
#define SEC_PER_DAY     (24*60*60)
#define SEC_PER_YEAR    (365*24*60*60)
#define LEAP_TO_70      (70/4)
#define FEB28           (58)

#define year_size(A)    (((A) % 4) ? 365 : 366)
#define isleap(y)       (((y) % 4) == 0 && ((y) % 100) != 0 || ((y) % 400) == 0)

#endif  /* !__psys_h */
/*
 * CHANGE LOG:
 *     Created Aug 17, 1992 by Tuyen Nguyen
 */
