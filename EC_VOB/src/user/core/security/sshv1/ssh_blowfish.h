/* $Id: ssh_blowfish.h,v 1.1.4.1 2000/08/25 09:32:04 tls Exp $ */

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


#if 0 /*isiah.1001-10-22*/
#ifndef _SSH_BLOWFISH_H
#define _SSH_BLOWFISH_H

#include "blowfish.h"
//Isiah.
#include "ssh_types.h"
#include <sys/types.h>

struct ssh_cipher;

typedef u_int8_t	bf_block[8];

typedef struct {
	BF_KEY		bf_ks;
	bf_block	bf_iv[2];
} ssh_blowfish_t;

void ssh_blowfish_attach(struct ssh_cipher *);
ssh_blowfish_t *ssh_blowfish_initialize(u_int8_t *);
void ssh_blowfish_encrypt(u_int8_t *, u_int8_t *, int, ssh_blowfish_t *);
void ssh_blowfish_decrypt(u_int8_t *, u_int8_t *, int, ssh_blowfish_t *);

#endif /* _SSH_BLOWFISH_H */
#endif /* #if 0 */