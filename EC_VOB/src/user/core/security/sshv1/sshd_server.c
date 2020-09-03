/* $Id: sshd_server.c,v 1.21.2.1 2000/08/25 09:32:24 tls Exp $ */

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

/* This file contains routines that drive the flow of */
/* an sshd server which has just received an incoming */
/* connection. */

#include <signal.h>
/*Isiah. 2002-05-27 .replace sys/time.h with time.h */
/*#include <sys/time.h>*/
/*#include <time.h>*/
#include <sys/types.h>
/*#include <unistd.h>*/

#include "options.h"

#include "sshd.h"
#include "ssh_sys.h"
#include "ssh_util.h"

/*isiah. 2002-05-29 */
#include "sysfun.h"
#include "sshd_type.h"
#include "sshd_vm.h"

extern int printf(const char *_format, ...);
extern int Accton_s_close(int sid);

/*
 * doServer: start conversation with client.
 *
 * Note: This function should NOT return!
 */
int doServer(struct sshd_context *context) 
{
    int retval;
/*    struct itimerval itval;*/
/*    int listened_socket, accepted_socket, sock_rc, myQid;*/

    SSHD_VM_SetSshConnectionStatus(NEGOTIATION_STARTED);
    
/* Check for compatible versions. */
    if ((retval = get_version(context)) != 0) 
	{
		s_close(context->s);
		context->s = -1;
		ssh_sys_exit();
    }

    /* ------------------------ */

    /* ok, looks like a ssh client.  Negociate encryption of the connection. */
    if ((retval = start_encryption(context)) != 0) 
	{
		s_close(context->s);
		context->s = -1;
		ssh_sys_exit();
    }

    /* ------------------------ */

    SSHD_VM_SetSshConnectionStatus(AUTHENTICATION_STARTED);
    
    /* Get user name from client. */
    if ((retval = get_user(context)) < 0) 
	{
		s_close(context->s);
		context->s = -1;
		ssh_sys_exit();
    }

    /* See if the user is authorized. */
    if ((retval = auth_user(context)) != 0) 
	{
		s_close(context->s);
		context->s = -1;
		ssh_sys_exit();
    }


    /* ------------------------ */

    SSHD_VM_SetSshConnectionStatus(SESSION_STARTED);
    
    /* Do preliminary setup, before executing a shell. */
    if ((retval = doPrepOps(context)) != 0) 
	{
		s_close(context->s);
		context->s = -1;
		ssh_sys_exit();
    }

    /* If we're here the client requested an EXEC and it was done. */
    /* Move into interactive mode. */
    if ((retval = doInteractive(context)) != 0) 
	{
		s_close(context->s);
		context->s = -1;
		ssh_sys_exit();
    }

    /* ------------------------ */

    /* All done. */
    ssh_sys_exit();
    return 0;
    /* NOTREACHED */
//    exit(0);
}

