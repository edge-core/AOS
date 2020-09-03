/* $Id: ssh_parse.h,v 1.6.2.1 2000/08/25 09:32:14 tls Exp $ */

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

#if 0 /*isiah.2002-10-22*/

#ifndef _SSH_PARSE_H
#define _SSH_PARSE_H
/*
 * Various functions to parse data.
 */
#include "ssh_crypto.h"

/* Type of keyfile to generate in encode_keyfile */
#define FRESSH_PUB_KEYFILE	1
#define FRESSH_PRIV_KEYFILE	2
#define FSECURE_PUB_KEYFILE	FRESSH_PUB_KEYFILE
#define FSECURE_PRIV_KEYFILE	3

/* Data parsing functions: */
int decode_keyfile(char *, int, char *, int, ssh_RSA **, char **, int *);
int encode_keyfile(int, char **, char *, int, ssh_RSA *, char *);

int encode_public_keyfile(char ** , ssh_RSA *, char *);
int decode_public_keyfile(char *buf, int len, ssh_RSA **key, char **comment);

int decode_fressh_keyfile(char *, int, char *, int, ssh_RSA **, char **);
int encode_fressh_keyfile(char **, char *, int, ssh_RSA *, char *);

int decode_fsecure_keyfile(char *, int, char *, int, ssh_RSA **, char **);
int encode_fsecure_keyfile(char **, char *, int, ssh_RSA *, char *);

int lookup_authorized_key(sshd_context_t *, ssh_BIGNUM *, ssh_RSA **);

#endif /* _SSH_PARSE_H */

#endif /* #if 0 */
