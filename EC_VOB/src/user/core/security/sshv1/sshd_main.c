/* $Id: sshd_main.c,v 1.47.2.4 2001/02/11 04:58:54 tls Exp $ */

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


/*-
 * Copyright (c) 1983, 1988, 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* This file contains routines to do the initial setup: read */
/* configuration files.  Open necessary sockets.  Listen for */
/* incoming connections. */

#include <err.h>
/*#include <errno.h>*/
#include <fcntl.h>
#include <string.h>
#include <signal.h>
/*Isiah. 2002-05-27 .
  mask syslog.h,beacuse not found in vxworks */
/*#include <syslog.h>*/
#include <stdlib.h>
/*#include <unistd.h>*/
#include <netdb.h>
#include <sys/types.h>
/*Isiah. 2002-05-27 .replace sys/time.h with time.h */
/*#include <sys/time.h>*/
/*#include <time.h>*/
/*Isiah. 2002-05-27 .
  mask sys/resource.h,beacuse not found in vxworks */
/*#include <sys/resource.h>*/
/*Isiah. 2002-05-27 
	Use phase2 socket, mask sys/socket.h and netinet/in.h,
	add "skt_vx.h" and "socket.h" */
/*#include <sys/socket.h>*/
#include "skt_vx.h"
#include "socket.h"
#include <sys/wait.h>
/*#include <netinet/in.h>*/
#include <arpa/inet.h>

#include "options.h"

#include "sshd.h"
#include "ssh_auth.h"
#include "ssh_cipher.h"
/*#include "ssh_logging.h"*/
#include "ssh_rsakeys.h"
#include "ssh_sys.h"
#include "ssh_util.h"

/*Isiah.2002-05-28*/
#include "sys_bld.h"
#include "sysfun.h"
#include "ip_task.h"
#include "iproute.h"
#include "sshd_type.h"
#include "sshd_mgr.h"
#include "sshd_vm.h"

#include "sys_module.h"
#include "syslog_type.h"
#include "eh_type.h"
#include "eh_mgr.h"

extern int printf(const char *_format, ...);

int sshd_main() 
{
    if (setup(0) != 0)
	{
		ssh_sys_exit();
	}

    if ( dolisten() !=0 )
    {
        ssh_sys_exit();
    }
    
    return (0);
}

/*
 * setup: Read configuration info.
 *	Args: how	0 = clean setup.
 *			!0 = re-read config.
 */
int setup(int how) 
{
	sshd_context_t		*ssh_context;

	ssh_context = (sshd_context_t *)SSHD_VM_GetContextAddress();
	
	if ( ssh_context == NULL )
	{
	    return(1);
	}

    if (ssh_sys_configuration(ssh_context) != 0)
	{
	    return(1);
	}

	if (init_serverkey(ssh_context) != 0)
	{
	    return(1);
	}

    ssh_sys_randinit();

    set_supported_ciphers(&(ssh_context->supported_ciphers));

    set_supported_auths(&(ssh_context->supported_auths));
    
    if ( (SSHD_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE) || (SSHD_VM_GetSshdStatus() != SSHD_STATE_ENABLED) )
    {
        return (1);
    }

	return(0);
}



/*
 * dolisten: Wait for a client to connect.
 */
int dolisten() 
{
	int listened_socket, accepted_socket, sock_rc, myQid;
	sshd_context_t		*ssh_context;

	myQid = P2_MY_QID();
    sock_rc = dup_sktlookup(myQid, &listened_socket, &accepted_socket);
    if (sock_rc)
    {
        UI8_T   *arg_p = "lookup";
        
        EH_MGR_Handle_Exception1(SYS_MODULE_SSH, dolisten_FUNC_NO, EH_TYPE_MSG_SOCKET_OP_FAILED, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT), arg_p);
		ssh_sys_exit();
    }
        
    SYSFUN_EnableRoundRobin();

	ssh_context = (sshd_context_t *)SSHD_VM_GetContextAddress();
    if ( ssh_context == NULL )
    {
        /*isiah.2003-01-27*/
        s_close(accepted_socket);
        return (1);
    }

	memset(ssh_context->cipher,0,sizeof(ssh_cipher_t));
   	ssh_context->s = accepted_socket;
    (*ssh_context).cipher->type = -1;
   	packet_init(&ssh_context->out_pc);
    packet_init(&ssh_context->in_pc);
   	ssh_context->compressing = 0;
	ssh_context->usepty = 0;

    doServer(ssh_context);		/* Talk to the client. */

    return(0);
}


