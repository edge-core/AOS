/* $Id: ssh_des.h,v 1.1.4.1 2000/08/25 09:32:08 tls Exp $ */

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


#ifndef _SSH_DES_H
#define _SSH_DES_H

#include "des.h"
struct ssh_cipher;

typedef struct {
	des_key_schedule des_ks;
	des_cblock	 des_ivec[2];	/* Two directions, same key! */
} ssh_des_t;

void ssh_des_attach(struct ssh_cipher *);
ssh_des_t *ssh_des_initialize(u_int8_t *);
void ssh_des_encrypt(u_int8_t *, u_int8_t *, int, ssh_des_t *);
void ssh_des_decrypt(u_int8_t *, u_int8_t *, int, ssh_des_t *);

#endif /* _SSH_DES_H */
