/* $Id: ssh_none.c,v 1.8.4.1 2000/08/25 09:32:11 tls Exp $ */

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
#include <string.h>
#include <sys/types.h>

#include "options.h"

#include "sshd.h"
#include "ssh_cipher.h"
#include "ssh_none.h"

void ssh_none_attach(ssh_cipher_t *c) {
    c->type = SSH_CIPHER_NONE;
    c->initialize = ssh_none_initialize;
    c->encrypt = ssh_none_encrypt;
    c->decrypt = ssh_none_decrypt;
    c->key_data = NULL;
}

void *ssh_none_initialize(void *session_key) {
    return malloc(64 * sizeof(char));
}

void ssh_none_encrypt(u_int8_t *clear, u_int8_t *enc, int length,
                     void *key_data) {
    (void)memcpy(enc, clear, length);
}

void ssh_none_decrypt(u_int8_t *enc, u_int8_t *clear, int length,
		     void *key_data) {
    (void)memcpy(clear, enc, length);
}
