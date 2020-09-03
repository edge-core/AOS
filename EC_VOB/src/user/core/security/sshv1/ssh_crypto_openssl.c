/*	$Id: ssh_crypto_openssl.c,v 1.28.2.1 2000/08/25 09:32:07 tls Exp $ */

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

#include <stdlib.h>
/*#include <errno.h>*/
#include <string.h>

#include <rand.h>

#include <opensslv.h>

#if OPENSSL_VERSION_NUMBER >= 0x00903000L
#define TO_CBLOCK(x)	(des_cblock *)x
#else
#define	TO_CBLOCK(x)	x
#endif

#include "options.h"
#include "ssh_crypto.h"
#include "ssh_crypto_openssl.h"
#include "ssh_cipher.h"
/*#include "ssh_logging.h"*/

inline void ssh_rsa_free(ssh_RSA *key) {
    RSA_free(&(key->rsa));
}
inline ssh_RSA *ssh_rsa_new() {
    return((ssh_RSA *)RSA_new());
}

inline ssh_RSA *ssh_rsa_generate_key(
			int numbits, int exp, void *cb, char *cbarg) {
    return((ssh_RSA *)RSA_generate_key(numbits, exp, cb, cbarg));
}

/*
 * Make sure a key has all the entries
 * filled in correctly.  If not, calculate them.
 *
 * Calculates dmp1, dmq1, iqmp.
 */
int ssh_key_fixup(ssh_RSA *key) {
  BIGNUM *temp;
  BN_CTX *ctx;

    if (key->rsa.q == NULL || key->rsa.p == NULL)
	return(1);

    temp = BN_new();
    ctx = BN_CTX_new();

    key->rsa.dmp1 = BN_new();
    BN_sub(temp, key->rsa.p, BN_value_one());
    BN_mod(key->rsa.dmp1, key->rsa.d, temp, ctx);

    key->rsa.dmq1 = BN_new();
    BN_sub(temp, key->rsa.q, BN_value_one());
    BN_mod(key->rsa.dmq1, key->rsa.d, temp, ctx);

    key->rsa.iqmp = BN_new();
    BN_mod_inverse(key->rsa.iqmp, key->rsa.q, key->rsa.p, ctx);

    return(0);
}

inline int ssh_rsa_private_decrypt(int len, char *from,
					char *buf, ssh_RSA *key) {
    return(RSA_private_decrypt(len, from, buf, &(key->rsa), RSA_PKCS1_PADDING));
}

inline int ssh_rsa_public_encrypt(int len, char *from,
					char *buf, ssh_RSA *key) {
    return(RSA_public_encrypt(len, from, buf, &(key->rsa), RSA_PKCS1_PADDING));
}

/* ---------------------- */

inline void bignum_free(ssh_BIGNUM *num) {
    return(BN_clear_free(&(num->num)));
}

inline int bignum_compare(ssh_BIGNUM *num1, ssh_BIGNUM *num2) {
    return(BN_cmp(&(num1->num), &(num2->num)));
}

inline int bignum_num_bits(ssh_BIGNUM *num) {
    return(BN_num_bits(&(num->num)));
}
inline int bignum_num_bytes(ssh_BIGNUM *num) {
    return(BN_num_bytes(&(num->num)));
}
/* bignum_bn2bin:
 *	returns number of bytes used.
 */
inline int bignum_bn2bin(ssh_BIGNUM *num, char *buf) {
    return(BN_bn2bin(&(num->num), buf));
}
inline int bignum_bin2bn(ssh_BIGNUM **num, u_int8_t *data, u_int16_t bytes) {
  BIGNUM *new_bn;
    new_bn = BN_bin2bn(data, bytes, &((*num)->num));
    if (new_bn == NULL)
	return(-1);
    (BIGNUM *)*num = new_bn;
    return(0);
}
inline char *bignum_bn2hex(ssh_BIGNUM *num) {
    return(BN_bn2hex(&(num->num)));
}
inline int bignum_hex2bn(ssh_BIGNUM **num, char *buf) {
    return(BN_hex2bn((BIGNUM **)num, buf));
}
inline char *bignum_bn2dec(ssh_BIGNUM *num) {
    return(BN_bn2dec(&(num->num)));
}
inline int bignum_dec2bn(ssh_BIGNUM **num, char *buf) {
    return(BN_dec2bn((BIGNUM **)num, buf));
}

/*
 * bignum_to_mpint: convert from a BIGNUM to a ssh_mpint.
 *		    mpi should point to an empty mpint.
 *
 * Returns -1 on error.
 */
int bignum_to_mpint(ssh_BIGNUM *bn, struct ssh_mpint *mpi) {
    if ((mpi->data = malloc(BN_num_bytes(&(bn->num)))) == NULL) {
/*	SSH_ERROR("bignum_to_mpint: malloc failed: %s\n", strerror(errno));*/
	mpi->bits = 0;
	return(-1);
    }
    mpi->bits = BN_num_bits(&(bn->num));
    return(BN_bn2bin(&(bn->num), mpi->data));
}

/*
 * mpint_to_bignum: convert from a ssh_mpint to a BIGNUM
 *		bn will be allocated.
 *
 * Returns -1 on error.
 */
int mpint_to_bignum(struct ssh_mpint *mpi, ssh_BIGNUM **bn) {
    if ((*bn = (ssh_BIGNUM *)BN_bin2bn(mpi->data, (mpi->bits + 7) / 8, NULL))
								== NULL) {
/*	SSH_ERROR("mpint_to_bignum: BN_bin2bn failed.\n");*/
	return(-1);
    }
    return(0);
}

/* -------------
 * Functions defined in ssh_crypto.h:
 * -------------
 */

/*
 * ssh_rand_feed:        feed some seed data to the random number generator.
 *                      Note: buffer is cleared before returning.
 */
void ssh_rand_feed(u_int8_t *buf, size_t len) {
    RAND_seed(buf, len);
    memset(buf, 0, len);
}

/*
 * ssh_rand_bytes:       fill a buffer with random data.
 */
void ssh_rand_bytes(size_t len, u_int8_t *buf) {
    RAND_bytes(buf, len);
}

/*
 * ssh_rand_clean:	clean up PRNG private data.
 */
void ssh_rand_clean()
{
    RAND_cleanup();
}

#ifdef WITH_CIPHER_DES

void ssh_des_attach(struct ssh_cipher *c) {
    c->type = SSH_CIPHER_DES;
    c->initialize = (void *)ssh_des_initialize;
    c->encrypt = (void *)ssh_des_encrypt;
    c->decrypt = (void *)ssh_des_decrypt;
    c->key_data = NULL;
}

ssh_des_t *ssh_des_initialize(u_int8_t *session_key) {

  int err;
  des_cblock key;
  ssh_des_t *key_data;

    err = 0;

    key_data = malloc(sizeof(ssh_des_t));
    if (key_data == NULL) return NULL;

    memcpy(key, session_key, sizeof(des_cblock));
    des_set_odd_parity(TO_CBLOCK(key));
    if (!des_is_weak_key(TO_CBLOCK(key)))
      (void)des_set_key(TO_CBLOCK(key), key_data->des_ks);
    else err = 1;

    memset(key_data->des_ivec[0], 0, sizeof(key_data->des_ivec[0]));
    memset(key_data->des_ivec[1], 0, sizeof(key_data->des_ivec[1]));

    if (!err)
	return key_data;
    else {
	memset(key, 0, sizeof(key));
	memset(key_data, 0, sizeof(key_data));
	free(key_data);
	return NULL;
    }
}

void ssh_des_encrypt(u_int8_t *clear, u_int8_t *enc, int length,
		    ssh_des_t *key_data) {

  des_ncbc_encrypt(clear, enc, length, key_data->des_ks,
		   TO_CBLOCK(key_data->des_ivec[0]), DES_ENCRYPT);
}

void ssh_des_decrypt(u_int8_t *enc, u_int8_t *clear, int length,
		    ssh_des_t *key_data) {

    des_ncbc_encrypt(enc, clear, length, key_data->des_ks, 
		     TO_CBLOCK(key_data->des_ivec[1]), DES_DECRYPT);
}

#endif /* WITH_CIPHER_DES */

#ifdef WITH_CIPHER_3DES

void ssh_3des_attach(struct ssh_cipher *c) {
    c->type = SSH_CIPHER_3DES;
    c->initialize = (void *)ssh_3des_initialize;
    c->encrypt = (void *)ssh_3des_encrypt;
    c->decrypt = (void *)ssh_3des_decrypt;
    c->key_data = NULL;
}

ssh_3des_t *ssh_3des_initialize(u_int8_t *session_key) {

  int i;
  des_cblock key[3];
  ssh_3des_t *key_data;

    key_data = malloc(sizeof(ssh_3des_t));
    if (key_data == NULL) return NULL;


    for(i=0; i < 3; i++) {
	memcpy(key[i], session_key, sizeof(des_cblock));
	des_set_odd_parity(TO_CBLOCK(key[i]));
	if (des_is_weak_key(TO_CBLOCK(key[i]))) break;
	(void)des_set_key(TO_CBLOCK(key[i]), key_data->des_ks[i]);
	session_key += sizeof(des_cblock);
    }

    memset(key_data->des_ivec[0], 0, sizeof(key_data->des_ivec[0]));
    memset(key_data->des_ivec[1], 0, sizeof(key_data->des_ivec[1]));
    memset(key_data->des_ivec[2], 0, sizeof(key_data->des_ivec[2]));
    memset(key_data->des_ivec[3], 0, sizeof(key_data->des_ivec[3]));
    memset(key_data->des_ivec[4], 0, sizeof(key_data->des_ivec[4]));
    memset(key_data->des_ivec[5], 0, sizeof(key_data->des_ivec[5]));

    if(i == 3)	/* We did all three keys okay */
	return key_data;
    else {
	for (i = 0; i < 3; i++)
	    memset(key[i], 0, sizeof(key[i]));
	memset(key_data, 0, sizeof(key_data));
	free(key_data);
        return NULL;
    }
}

void ssh_3des_encrypt(u_int8_t *clear, u_int8_t *enc, int length,
                     ssh_3des_t *key_data) {

    des_ncbc_encrypt(clear, enc, length, key_data->des_ks[0],
		     TO_CBLOCK(key_data->des_ivec[0]), DES_ENCRYPT);
    des_ncbc_encrypt(enc, enc, length, key_data->des_ks[1],
		     TO_CBLOCK(key_data->des_ivec[1]), DES_DECRYPT);
    des_ncbc_encrypt(enc, enc, length, key_data->des_ks[2],
		     TO_CBLOCK(key_data->des_ivec[2]), DES_ENCRYPT);
}

void ssh_3des_decrypt(u_int8_t *enc, u_int8_t *clear, int length,
		     ssh_3des_t *key_data) {

    des_ncbc_encrypt(enc, clear, length, key_data->des_ks[2],
		     TO_CBLOCK(key_data->des_ivec[3]), DES_DECRYPT);
    des_ncbc_encrypt(clear, clear, length, key_data->des_ks[1],
		     TO_CBLOCK(key_data->des_ivec[4]), DES_ENCRYPT);
    des_ncbc_encrypt(clear, clear, length, key_data->des_ks[0],
		     TO_CBLOCK(key_data->des_ivec[5]), DES_DECRYPT);
   
}

#endif /* WITH_CIPHER_3DES */

#ifdef WITH_CIPHER_BLOWFISH
void ssh_blowfish_attach(struct ssh_cipher *c) {
    c->type = SSH_CIPHER_BLOWFISH;
    c->initialize = (void *)ssh_blowfish_initialize;
    c->encrypt = (void *)ssh_blowfish_encrypt;
    c->decrypt = (void *)ssh_blowfish_decrypt;
    c->key_data = NULL;
}

ssh_blowfish_t *ssh_blowfish_initialize(u_int8_t *session_key) {

  ssh_blowfish_t *key_data;

    key_data = malloc(sizeof(ssh_blowfish_t));
    if (key_data == NULL) return NULL;

    BF_set_key(&key_data->bf_ks, 32, session_key); /* XXX hardcoded len? */

    memset(key_data->bf_iv[0], 0, sizeof(key_data->bf_iv[0]));
    memset(key_data->bf_iv[1], 0, sizeof(key_data->bf_iv[1]));

    return key_data;
}

void ssh_blowfish_encrypt(u_int8_t *clear, u_int8_t *enc, int length,
			  ssh_blowfish_t *key_data) {

    FLIP_BYTES(clear, length);
    BF_cbc_encrypt(clear, enc, length, &key_data->bf_ks,
                        key_data->bf_iv[0], BF_ENCRYPT);
    FLIP_BYTES(enc, length);

}

void ssh_blowfish_decrypt(u_int8_t *enc, u_int8_t *clear, int length,
			  ssh_blowfish_t *key_data) {
    FLIP_BYTES(enc, length);
    BF_cbc_encrypt(enc, clear, length, &key_data->bf_ks,
                        key_data->bf_iv[1], BF_DECRYPT);
    FLIP_BYTES(clear, length);
}

#endif /* WITH_CIPHER_BLOWFISH */
