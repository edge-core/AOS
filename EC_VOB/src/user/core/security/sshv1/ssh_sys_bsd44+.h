/* 	$Id: ssh_sys_bsd44+.h,v 1.1.4.1 2000/08/25 09:32:17 tls Exp $ */

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


#ifndef _SSH_SYS_BSD44PLUS_H
#define _SSH_SYS_BSD44PLUS_H

/*Isiah. 2002-05-27 .
  mask sys/param.h,beacuse not found in vxworks */
/*#include <sys/param.h>*/
#include "ssh_types.h"
#include <sys/types.h>

#if (defined(__NetBSD_Version__) && (__NetBSD_Version >= 0104060000)) 
/* NetBSD 1.4 has bswap functions in assembler */
#define FLIP_BYTES(bytes, howmany) \
{ \
  u_int32_t flip; \
    for(flip = (u_int32_t*)bytes; (u_int8_t *)flip < \
	    (u_int8_t)(bytes + howmany); flip++;) \
	*flip = bswap32(*flip); \
}
#else
/* This may well come out of the compiler and run just as fast, and it avoids
   casts which may not pass EGCS's pedantic "strict alias" checking... */
#define FLIP_BYTES(bytes, howmany) \
{ \
  long where = 0; \
  u_int8_t holder[4]; \
\
    while(where < howmany) { \
	holder[0] = bytes[where + 3]; \
	holder[3] = bytes[where]; \
	holder[1] = bytes[where + 2]; \
	holder[2] = bytes[where + 1]; \
        bytes[where++] = holder[0]; \
	bytes[where++] = holder[1]; \
	bytes[where++] = holder[2]; \
	bytes[where++] = holder[3]; \
    } \
}
#endif /* __NetBSD_Version_ */
#endif /* _SSH_SYS_BSD44+_H */
