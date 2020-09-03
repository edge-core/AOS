/* $Id: ssh_3des.h,v 1.5.4.1 2000/08/25 09:32:03 tls Exp $ */

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


#ifndef _SSH_3DES_H
#define _SSH_3DES_H

#include "des.h"
struct ssh_cipher;

typedef struct {
	des_key_schedule des_ks[3];
	des_cblock	 des_ivec[6];	/* Two directions, same key! */
} ssh_3des_t;

void ssh_3des_attach(struct ssh_cipher *);
ssh_3des_t *ssh_3des_initialize(u_int8_t *);
void ssh_3des_encrypt(u_int8_t *, u_int8_t *, int, ssh_3des_t *);
void ssh_3des_decrypt(u_int8_t *, u_int8_t *, int, ssh_3des_t *);

#endif /* _SSH_3DES_H */
