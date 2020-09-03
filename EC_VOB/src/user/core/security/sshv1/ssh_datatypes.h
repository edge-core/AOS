/* $Id: ssh_datatypes.h,v 1.4.4.1 2000/08/25 09:32:08 tls Exp $ */

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


#ifndef _SSH_DATATYPES_H
#define _SSH_DATATYPES_H

//Isiah.
#include "ssh_types.h"
#include <sys/types.h>

/* Note: this is _NOT_ the format of these data on the wire! */
typedef struct ssh_binstr {
	u_int32_t len;		/* host byte order. */
	u_int8_t *data;
} ssh_binstr_t;

typedef struct ssh_mpint {
	u_int16_t bits;		/* host byte order. */
	u_int8_t *data;
} ssh_mpint_t;

#endif /* _SSH_DATATYPES_H */
