/* $Id: ssh_rsakeys.h,v 1.6.4.2 2001/02/11 04:58:53 tls Exp $ */

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


#ifndef _SSH_RSAKEYS_H
#define _SSH_RSAKEYS_H

#include "sshd.h"

/*Isiah. 2002-06-04 */
#if 0
void regen_serverkey(void);
#endif /*isiah. end of #if 0 */
/* ISiah.2002-06-03 */
//int init_serverkey();
int init_serverkey(struct sshd_context *);
//int read_hostkey();
int decrypt_session_key(struct sshd_context *, struct ssh_mpint *);

#endif /* _SSH_RSAKEYS_H */
