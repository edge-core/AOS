/* $Id: ssh_logging.h,v 1.6.2.1 2000/08/25 09:32:10 tls Exp $ */

/*
 * Copyright 1999 RedBack Networks, Incorporated.
 * All rights reserved.
 *
 * This software is not in the public domain.  It is distributed 
 * under the terms of the license in the file LICENSE in the
 * same directory as this file.  If you have received a copy of this
 * software without the LICENSE file (which means that whoever gave
 * you this software violated its license) you may obtain a copy from
 * http://www.panix.com/~tls/LICENSE.txt
 */


#ifndef _SSH_LOGGING_H
#define _SSH_LOGGING_H

/*isiah.2002-10-21*/
#if 0
/*Isiah. 2002-05-27 .
  mask syslog.h,beacuse not found in vxworks */
/*#include <syslog.h>*/
#include <stdarg.h>
#ifndef DEBUG
#define DEBUG 2
#endif

extern int debuglevel;

#define SSH_DLOG(l, args)	if (debuglevel >= l) logit args

#define SSH_ERROR 		logit
#endif /* end of #if 0 */

/*Isiah. 2002-06-04 */
#if 0
int loginit(char *, int);
int logclose();
int logit(char *, ...);
#endif /*isiah. end of #if 0 */

/*Isiah. 2002-06-04 */
#if 0
void debug_nostderr();
void debug_inc(int);
void debug_dec(int);
int is_debug_level(int);
#endif /*isiah. end of #if 0 */

#endif /* _SSH_LOGGING_H */
