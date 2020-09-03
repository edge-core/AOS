/* $Id: ssh_cipher.c,v 1.15.2.1 2000/08/25 09:32:06 tls Exp $ */

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

/* routines to implement the various crypt routines. */

#include "options.h"
#include "sshd.h"
#include "ssh_cipher.h"

/*isiah. 2002-06-03 */
#include "sshd_vm.h"


/*
 * set_supported_ciphers: set the bitmask for supported ciphers.
 */
void set_supported_ciphers(u_int32_t *arg) {

	*arg = 0;
#ifdef WITH_CIPHER_NONE
	*arg = (1 << SSH_CIPHER_NONE);
#endif
#ifdef WITH_CIPHER_IDEA
	*arg |= (1 << SSH_CIPHER_IDEA);
#endif
#ifdef WITH_CIPHER_DES
	*arg |= (1 << SSH_CIPHER_DES);
#endif
#ifdef WITH_CIPHER_3DES
	*arg |= (1 << SSH_CIPHER_3DES);
#endif
#ifdef WITH_CIPHER_RC4
	*arg |= (1 << SSH_CIPHER_RC4);
#endif
#ifdef WITH_CIPHER_BLOWFISH
	*arg |= (1 << SSH_CIPHER_BLOWFISH);
#endif
}

int cipher_supported() 
{
	sshd_context_t		*ssh_context;

	ssh_context = (sshd_context_t *)SSHD_VM_GetContextAddress();

	if ( ssh_context == NULL )
	{
	    return 0;
	}

	return(ssh_context->supported_ciphers);
}

#if 0 /*isiah.1001-10-22*/
char *cipher_name(int cipher_type) {

#ifdef WITH_CIPHER_NONE
    if (cipher_type == SSH_CIPHER_NONE)
	return("none");
#endif
#ifdef WITH_CIPHER_DES
    if (cipher_type == SSH_CIPHER_DES)
	return("des");
#endif
#ifdef WITH_CIPHER_3DES
    if (cipher_type == SSH_CIPHER_3DES)
	return("3des");
#endif
#ifdef WITH_CIPHER_BLOWFISH
    if (cipher_type == SSH_CIPHER_BLOWFISH)
	return("blowfish");
#endif

    return("unknown");
}
#endif /* #if 0 */

/*
 * set_cipher_type: set up the context for the given cipher type.
 */
int set_cipher_type(sshd_context_t *context, int type) {

    switch (type) {
#ifdef WITH_CIPHER_NONE
    case SSH_CIPHER_NONE:
	ssh_none_attach(context->cipher);
	break;
#endif
#ifdef WITH_CIPHER_DES
    case SSH_CIPHER_DES:
	ssh_des_attach(context->cipher);
	break;
#endif
#ifdef WITH_CIPHER_3DES
    case SSH_CIPHER_3DES:
	ssh_3des_attach(context->cipher);
	break;
#endif
#ifdef WITH_CIPHER_BLOWFISH
    case SSH_CIPHER_BLOWFISH:
	ssh_blowfish_attach(context->cipher);
	break;
#endif
    default:
/*	SSH_DLOG(1, ("Unknown cipher type: %d\n", type));*/
	return(1);
    }
    return(0);
}

