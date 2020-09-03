/* $Id: ssh_crypto_openssl.h,v 1.10.2.1 2000/08/25 09:32:07 tls Exp $ */

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


#ifndef _SSH_CRYPTO_OPENSSL_H
#define _SSH_CRYPTO_OPENSSL_H

/*#include <stdio.h>*/
#include <rsa.h>

#include "ssh_datatypes.h"

/*
 * Definitions of the ssh_BIGNUM and ssh_RSA wrapper structures
 * and prototypes/macros for ways to work with them.
 */

/* These structures exists to help maintain the type checking. */
/* going from ssh_FOO->FOO is easy. */
/* going from FOO->ssh_FOO needs conversion functions (ssh_rsa_?part) */
typedef struct {
	BIGNUM num;
} ssh_BIGNUM;
typedef struct {
	RSA rsa;
} ssh_RSA;

/* This mess of *'s and &'s is here to make it possible to get the */
/* address of each of these variables.  It is needed because taking */
/* the address of a variable that has been casted doesn't work. */
/* ssh_rsa_Xpart(struct ssh_RSA *key) */
#define ssh_rsa_npart(key)	(*(ssh_BIGNUM **)(&(((key)->rsa).n)))
#define ssh_rsa_epart(key)	(*(ssh_BIGNUM **)(&(((key)->rsa).e)))
#define ssh_rsa_dpart(key)	(*(ssh_BIGNUM **)(&(((key)->rsa).d)))
#define ssh_rsa_ppart(key)	(*(ssh_BIGNUM **)(&(((key)->rsa).p)))
#define ssh_rsa_qpart(key)	(*(ssh_BIGNUM **)(&(((key)->rsa).q)))
#define ssh_rsa_dmppart(key)	(*(ssh_BIGNUM **)(&(((key)->rsa).dmp1)))
#define ssh_rsa_dmqpart(key)	(*(ssh_BIGNUM **)(&(((key)->rsa).dmq1)))
#define ssh_rsa_iqmppart(key)	(*(ssh_BIGNUM **)(&(((key)->rsa).iqmp)))

inline void ssh_rsa_free(ssh_RSA *key);
inline ssh_RSA *ssh_rsa_new();
inline ssh_RSA *ssh_rsa_generate_key(int, int, void *, char *);
int ssh_key_fixup(ssh_RSA *);
inline int ssh_rsa_private_decrypt(int, char *, char *, ssh_RSA *);
inline int ssh_rsa_public_encrypt(int, char *, char *, ssh_RSA *);
inline void bignum_free(ssh_BIGNUM *num);
inline int bignum_compare(ssh_BIGNUM *num1, ssh_BIGNUM *num2);
inline int bignum_num_bits(ssh_BIGNUM *num);
inline int bignum_num_bytes(ssh_BIGNUM *num);
inline int bignum_bn2bin(ssh_BIGNUM *num, char *buf);
inline int bignum_bin2bn(ssh_BIGNUM **num, u_int8_t *data, u_int16_t bytes);
inline char *bignum_bn2hex(ssh_BIGNUM *num);
inline int bignum_hex2bn(ssh_BIGNUM **num, char *buf);
inline char *bignum_bn2dec(ssh_BIGNUM *num);
inline int bignum_dec2bn(ssh_BIGNUM **num, char *buf);
int bignum_to_mpint(ssh_BIGNUM *, struct ssh_mpint *);
int mpint_to_bignum(struct ssh_mpint *, ssh_BIGNUM **);

#endif
