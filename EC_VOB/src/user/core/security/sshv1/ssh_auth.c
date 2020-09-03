/* $Id: ssh_auth.c,v 1.8.2.1 2000/08/25 09:32:03 tls Exp $ */

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


/*Isiah 2002-05-27*/
/*#include <strings.h>*/
#include <string.h>

#include "options.h"

#include "sshd.h"
#include "ssh_auth.h"

/*isiah. 2002-06-03 */
#include "sshd_vm.h"

#if !defined(WITH_AUTH_RHOSTS) && !defined(WITH_AUTH_RSA)
#if !defined(WITH_AUTH_PASSWORD) && !defined(WITH_AUTH_RHOSTS_RSA)
#error You must define at least one authentication type.
#endif
#endif

void set_supported_auths(u_int32_t *arg) {

	*arg = 0;
#ifdef WITH_AUTH_RHOSTS
	*arg |= (1 << SSH_AUTH_RHOSTS);
#endif
#ifdef WITH_AUTH_RSA
	*arg |= (1 << SSH_AUTH_RSA);
#endif
#ifdef WITH_AUTH_PASSWORD
	*arg |= (1 << SSH_AUTH_PASSWORD);
#endif
#ifdef WITH_AUTH_RHOSTS_RSA
	*arg |= (1 << SSH_AUTH_RHOSTS_RSA);
#endif
}

int auths_supported() 
{
	sshd_context_t		*ssh_context;

	ssh_context = (sshd_context_t *)SSHD_VM_GetContextAddress();

	if ( ssh_context == NULL )
	{
	    return 0;
	}

	return(ssh_context->supported_auths);
}

#if 0 /*isiah.1001-10-22*/
char *auth_name(int anum) {
    switch(anum) {
    case SSH_AUTH_RHOSTS:
	return("rhosts");
    case SSH_AUTH_RSA:
	return("rsa");
    case SSH_AUTH_PASSWORD:
	return("password");
    case SSH_AUTH_RHOSTS_RSA:
	return("rhosts-rsa");
    default:
	return("unknown");
    }
}

int auth_number(char *aname) {
    
    if (strcasecmp(aname, "rhosts") == 0)
	return(SSH_AUTH_RHOSTS);
    if (strcasecmp(aname, "rsa") == 0)
	return(SSH_AUTH_RSA);
    if (strcasecmp(aname, "password") == 0)
	return(SSH_AUTH_PASSWORD);
    if (strcasecmp(aname, "rhosts-rsa") == 0)
	return(SSH_AUTH_RHOSTS_RSA);
    if (strcasecmp(aname, "rhostsrsa") == 0)
	return(SSH_AUTH_RHOSTS_RSA);
    return(-1);
}
#endif
