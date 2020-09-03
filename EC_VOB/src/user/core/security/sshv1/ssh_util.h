/* $Id: ssh_util.h,v 1.22.2.1 2000/08/25 09:32:19 tls Exp $ */

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

#ifndef _SSH_UTIL_H
#define _SSH_UTIL_H

/*Isiah. 2002-05-27 .
  mask termios.h,beacuse not found in vxworks */
/*#include <termios.h>*/

#include "sshd.h"

char *build_version();
u_int32_t ssh_crc32(u_int8_t *, size_t);
u_int32_t ssh_crc32_initial(u_int8_t *, size_t);
u_int32_t ssh_crc32_final(u_int8_t *, size_t, u_int32_t);
u_int32_t ssh_crc32_partial(u_int8_t *, size_t, u_int32_t);

int detect_attack(unsigned char *, u_int32_t, unsigned char *);

#if 0 /*isiah.2002-10-22*/
void free_mpint(struct ssh_mpint *);

char *authorized_keys_file(struct sshd_context *context);
#endif /* #if 0 */

#endif /* _SSH_UTIL_H */
