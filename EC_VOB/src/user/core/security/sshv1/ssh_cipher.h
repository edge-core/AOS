/* $Id: ssh_cipher.h,v 1.16.4.1 2000/08/25 09:32:06 tls Exp $ */

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


#ifndef _SSH_CIPHER_H
#define _SSH_CIPHER_H

#include "sshd.h"

#define SSH_CIPHER_NONE		0
#define SSH_CIPHER_IDEA		1
#define SSH_CIPHER_DES		2
#define SSH_CIPHER_3DES		3
#define SSH_CIPHER_RC4		5
//isiah.
//#define SSH_CIPHER_BLOWFISH	6

typedef struct ssh_cipher {
	int type;
	void * (*initialize) (void *);
			/* takes session key, returns allocated key_data */
	void (*encrypt) (u_int8_t *, u_int8_t *, int, void *);
			/* clear, enc, length, key_data */
	void (*decrypt) (u_int8_t *, u_int8_t *, int, void *);
			/* enc, clear, length, key_data */
	void *key_data;
} ssh_cipher_t;

/* Bitmask of supported ciphers. */
void set_supported_ciphers(u_int32_t *);
int cipher_supported();

/* Cipher number->name */
#if 0 /*isiah.1001-10-22*/
char *cipher_name(int);
/* Cipher name->number */
int cipher_number(char *);
#endif /* #if 0 */

int set_cipher_type(sshd_context_t *, int);

#endif /* _SSH_CIPHER_H */
