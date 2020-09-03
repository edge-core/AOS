/* $Id: ssh_auth.h,v 1.2.4.1 2000/08/25 09:32:04 tls Exp $ */

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

#ifndef _SSH_AUTH_H
#define _SSH_AUTH_H

#define SSH_AUTH_RHOSTS		1
#define SSH_AUTH_RSA		2
#define SSH_AUTH_PASSWORD	3
#define SSH_AUTH_RHOSTS_RSA	4

/* Bitmask of supported authentication methods: */
void set_supported_auths(u_int32_t *);
int auths_supported();

#if 0 /*isiah.1001-10-22*/
/* Auth number->name */
char *auth_name(int);
/* Auth name->number */
int auth_number(char *);
#endif

#endif /* _SSH_AUTH_H */
