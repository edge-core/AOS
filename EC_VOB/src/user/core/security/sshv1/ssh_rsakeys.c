/* $Id: ssh_rsakeys.c,v 1.21.2.2 2001/02/11 04:58:53 tls Exp $ */

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


/*
 * This file contains functions to handle generation of the server key,
 * reading of the host key and parsing of the session key.
 * It is used by the sshd server. (rename to sshd_rsakeys?)
 */

/*#include <errno.h>*/
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
/*Isiah. 2002-05-27 .replace sys/time.h with time.h */
/*#include <sys/time.h>*/
/*#include <time.h>*/
#include <sys/types.h>
/*#include <sys/stat.h>*/
/*#include <unistd.h>*/

/*#include <stdio.h>*/
#include "rsa.h"

#include "options.h"

#include "sshd.h"
#include "ssh_buffer.h"
#include "ssh_cipher.h"
#include "ssh_datatypes.h"
/*#include "ssh_keyfile.h"*/
/*#include "ssh_logging.h"*/
/*#include "ssh_parse.h"*/
#include "ssh_rsakeys.h"
#include "ssh_sys.h"
#include "ssh_util.h"
#include "sshd_vm.h"

extern int printf(const char *_format, ...);



int init_serverkey(struct sshd_context *context) 
{
    if ( SSHD_VM_GetServerkey(context) )
    {
        return (0);
    }
    printf("sshd : get serverkey error \n");
    return(1);
}

int decrypt_session_key(struct sshd_context *context, struct ssh_mpint *mpi) {
  int len1, len2;
  int i;
  char *buf1, *buf2;
  int buf1_size, buf2_size;
  int h_bits, h_bytes, s_bits, s_bytes;
  ssh_RSA *key1, *key2;

    h_bits = bignum_num_bits(ssh_rsa_npart(context->hostkey));
    h_bytes = bignum_num_bytes(ssh_rsa_npart(context->hostkey));
    s_bits = bignum_num_bits(ssh_rsa_npart(context->serverkey));
    s_bytes = bignum_num_bytes(ssh_rsa_npart(context->serverkey));

    if ((mpi->bits > h_bits) && (mpi->bits > s_bits)) {
/*	SSH_ERROR("%s\n%s\n",
	    "decrypt_session_key: programming error: RSA keys too small.",
	    "Multi block decryption not implemented.");*/
	return(1);
    }

    /* Note: this code depends on the server and host key lengths differing.*/
	/* XXX does the security of the double encryption also depend on it? */
    if (h_bits > s_bits) {
/*	SSH_DLOG(4, ("key1 is hostkey, key2 is serverkey.\n"));*/
	buf1_size = h_bytes;
	buf2_size = s_bytes;
	key1 = context->hostkey;
	key2 = context->serverkey;
    } else {
/*	SSH_DLOG(4, ("key1 is serverkey, key2 is hostkey.\n"));*/
	buf1_size = s_bytes;
	buf2_size = h_bytes;
	key1 = context->serverkey;
	key2 = context->hostkey;
    }

    if ((buf1 = malloc(buf1_size)) == NULL) {
/*	SSH_ERROR("decrypt_session_key: malloc(buf1) failed: %s\n",
							strerror(errno));*/
	return(1);
    }

    /* Decrypt with key #1 */
    if ((len1 = ssh_rsa_private_decrypt(
			(mpi->bits + 7) / 8, mpi->data, buf1, key1)) < 0) {
/*	SSH_ERROR("decrypt_session_key: key #1 decryption failed\n");*/
	free(buf1);
	return(1);
    }

    if ((buf2 = malloc(buf2_size)) == NULL) {
/*	SSH_ERROR("decrypt_session_key: malloc(buf2) failed: %s\n",
							strerror(errno));*/
	memset(buf1, 0, buf1_size);
	free(buf1);
	return(1);
    }

    /* Then decrypt with key #2 */
    len2 = ssh_rsa_private_decrypt(len1, buf1, buf2, key2);
    memset(buf1, 0, buf1_size);
    free(buf1);
    if (len2 < 0) {
/*	SSH_ERROR("decrypt_session_key: key #2 decryption failed\n");*/
	free(buf2);
	return(1);
    }

    if (len2 != SSH_SESSION_KEY_SIZE) {
/*	SSH_ERROR(
		"decrypt_session_key: session key is wrong size: %d\n", len2);*/
	memset(buf2, 0, buf2_size);
	free(buf2);
	return(1);
    }

    /* Then XOR the first 16 bytes with the 16 bytes from the session_id. */
    for (i = 0; i < 16 ; i++)
	context->session_key[i] = buf2[i] ^ context->session_id[i];
    /* and copy the rest. */
    for (i = 16 ; i < SSH_SESSION_KEY_SIZE ; i++)
	context->session_key[i] = buf2[i];

    memset(buf2, 0, buf2_size);
    free(buf2);
    
    return(0);
}

