/* $Id: ssh_none.h,v 1.3.4.1 2000/08/25 09:32:11 tls Exp $ */

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


#ifndef _SSH_NONE_H
#define _SSH_NONE_H

struct ssh_cipher;

void *ssh_none_initialize(void *);
void ssh_none_encrypt(u_int8_t *, u_int8_t *, int, void *);
void ssh_none_decrypt(u_int8_t *, u_int8_t *, int, void *);
void ssh_none_attach(struct ssh_cipher *);

#endif /* _SSH_NONE_H */
