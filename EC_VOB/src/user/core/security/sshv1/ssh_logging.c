/* $Id: ssh_logging.c,v 1.10.2.1 2000/08/25 09:32:10 tls Exp $ */

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

/*isiah.2002-10-21*/
#if 0

/*Isiah. 2002-05-27 .
  mask syslog.h,beacuse not found in vxworks */
/*#include <syslog.h>*/
#include <stdarg.h>
/*#include <stdio.h>*/
#include <sys/types.h>
/*#include <unistd.h>*/

#include "options.h"

/*#include "ssh_logging.h"*/

int debugpid;
int debuglevel;
int debug_to_stderr;
static int loginitdone = 0;


int logclose() {

    loginitdone = 0;
    return(0);
}

int logit(char *fmt, ...)
{
    va_list args;
    int retval = 0;

    va_start(args, fmt);
    if (debug_to_stderr < 2)
		;
    if (debug_to_stderr) {
#ifndef NO_FP_API
	fprintf(stderr, "sshd[%d]: ", debugpid);
	vfprintf(stderr, fmt, args);
#else
	printf("sshd[%d]: ", debugpid);
	vprintf(fmt, args);
#endif
    }
    va_end(args);
    return(retval);
}
#endif /* end of #if 0 */



