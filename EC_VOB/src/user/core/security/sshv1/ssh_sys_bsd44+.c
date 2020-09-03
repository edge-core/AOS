/* $Id: ssh_sys_bsd44+.c,v 1.34.2.7 2001/02/15 20:59:36 erh Exp $ */

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


/*#include <errno.h>*/
#include <fcntl.h>
/*Isiah. 2002-05-27 .
  mask pwd.h,beacuse not found in vxworks */
/*#include <pwd.h>*/
#include <string.h>
/*#include <stdio.h>*/
#include <stdlib.h>
/*#include <unistd.h>*/
#include <netdb.h>
/*Isiah. 2002-05-27 .replace sys/time.h with time.h */
/*#include <sys/time.h>*/
/*#include <time.h>*/
#include <sys/types.h>
/*Isiah. 2002-05-27 
	Use phase2 socket, mask sys/socket.h and netinet/in.h,
	add "skt_vx.h" and "socket.h" */
/*#include <sys/socket.h>*/
#include "skt_vx.h"
#include "socket.h"
/*Isiah. 2002-05-27 .
  mask sys/param.h,beacuse not found in vxworks */
/*#include <sys/param.h>*/
/*Isiah. 2002-05-27 .
  mask sys/ucred.h,beacuse not found in vxworks */
/*#include <sys/ucred.h>*/
/*Isiah. 2002-05-27 .
  mask sys/mount.h,beacuse not found in vxworks */
/*#include <sys/mount.h>*/
/*#include <sys/mman.h>*/
/*#include <netinet/in.h>*/
#include <arpa/inet.h>


#include "options.h"

#include "sshd.h"
#include "ssh_buffer.h"
#include "ssh_crypto.h"
/*#include "ssh_paths.h"*/
#include "ssh_sys.h"
#include "ssh_util.h"
/*#include "ssh_parse.h"*/
/*#include "ssh_keyfile.h"*/
/*isiah 2002-05-27*/
#include "ssh_types.h"
//#include "netcfg.h"
#include "sshd_type.h"
#include "iproute.h"
#include "sysfun.h"
#include "sshd_vm.h"
#include "ssh_cipher.h"
#include "cli_mgr.h"

extern int printf(const char *_format, ...);

/*static char *childenv[16];*/		/* XXX */
/*static char shell[MAXINTERP + 1];*/

/*
 * ssh_sys_allocpty: doesn't actually allocate the pty on 4.4BSD!
 *
 *                Instead, it sets everything up, and ssh_sys_execcmd uses a
 *                function from libutil which includes the pty allocation.
 *                This means the other end may think we successfully
 *                allocated a pty, but then failed to execute the requested
 *                command, if, for example, we're out of ptys.  NBD.
 */
int ssh_sys_allocpty(struct sshd_context *context, char *term, int y_row, int x_col,
			int x_pix, int y_pix, u_int8_t *modes, size_t msize) 
{
/*Isiah. 2002-05-27 */
    /* Set flag: ssh_sys_execcmd will check this and allocate a pty then. */
    context->usepty = 1;

    return(0);
}

/*
 * ssh_sys_setbufsize: Set socket buffer size to increase throughput.
 */
#if 0 /*isiah.2002-10-22*/
void ssh_sys_setbufsize(struct sshd_context *context, int fd0, int fd1) {
  int size;
  int a, b, c, d;
    size = context->max_sock_bufsize;
    while (1) {
	size -= 2048;
        if (size <= 0) {
	    break;
	}
	a = setsockopt(fd0, SOL_SOCKET, SO_SNDBUF, &size, sizeof(int));
	b = setsockopt(fd0, SOL_SOCKET, SO_RCVBUF, &size, sizeof(int));
	c = setsockopt(fd1, SOL_SOCKET, SO_SNDBUF, &size, sizeof(int));
	d = setsockopt(fd1, SOL_SOCKET, SO_RCVBUF, &size, sizeof(int));
	if (a == 0 && b == 0 && c == 0 && d == 0)
	    break;
    }
    if (size < SSH_MIN_READSIZE)
	context->sock_bufsize = SSH_MIN_READSIZE;
    else
	context->sock_bufsize = size;
}
#endif /* #if 0 */

/*
 * ssh_sys_execcmd: execute a command.
 */
int ssh_sys_execcmd(struct sshd_context *context, char *cmd) 
{
/* Isiah.2002-05-29 */
/* rewrite ssh_sys_execcmd() function.
   connect to telnetd.*/
    struct sockaddr_in sockaddr_pty;
    int pty, i;
    UI32_T  tnsh_ip, tnsh_port, user_ip, user_port;
    UI32_T  remote_tnsh_ip, remote_tnsh_port, local_user_ip, local_user_port;

    if ((pty = socket(AF_INET4 , SOCK_STREAM, IP_PROT_TCP)) < 0)
    {
        printf("socket(pty) create fail.\n");
        return -1;
    }

    sockaddr_pty.sin_family = AF_INET4;
    sockaddr_pty.sin_port = TNSHD_SERVICE_SOCKET_PORT_NUMBER;

    {
    	UI32_T	tmp_ip, tmp_port;
    	if ( SSHD_VM_GetLocalSessionName(context->s, &tmp_ip, &tmp_port) == FALSE )
    	{
    	    /*isiah.2003-01-27*/
    	    s_close(pty);
    	    return -1;
    	}
        sockaddr_pty.sin_addr = tmp_ip;
    }


    /*  try to connect to TNSH 3 times  */
    for (i=0; i < 3; i++) 
    {
        sockaddr_pty.sin_port = htons(TNSHD_SERVICE_SOCKET_PORT_NUMBER);
//        DBG_PrintText3 ("\n tnpd_main : Connect to IP(%08x),port(%d)\n",
//            (int) sin.sin_addr, (int) sin.sin_addr);

        /*  sin.sin_port = htons(tnsh_get_dport()); */
        if (connect(pty, (struct sockaddr *)&sockaddr_pty, sizeof(sockaddr_pty)) >= 0)
            break;

        SYSFUN_Sleep(1<<6);
    }

    if ( i < 3 )
    {
        int size, len;
	    int ret;
		char c;
	    
        if ( SSHD_VM_GetLocalSessionName(pty, &tnsh_ip, &tnsh_port) == FALSE )
        {
            s_close(pty);
            return -1;
        }
        if ( SSHD_VM_GetRemoteSessionName(pty, &remote_tnsh_ip, &remote_tnsh_port) == FALSE )
        {
            s_close(pty);
            return -1;
        }
        if ( SSHD_VM_GetLocalSessionName(context->s, &local_user_ip, &local_user_port) == FALSE )
        {
            s_close(pty);
            return -1;
        }
        if ( SSHD_VM_GetRemoteSessionName(context->s, &user_ip, &user_port) == FALSE )
        {
            s_close(pty);
            return -1;
        }
        if ( SSHD_VM_SetSessionPair(remote_tnsh_port, tnsh_port, local_user_port, user_ip, user_port) == FALSE )
        {
            s_close(pty);
            return -1;
        }

        context->child_stdin = pty;
	    context->child_stdout = pty;
	    context->child_stderr = pty;
	    len = sizeof(int);
	    ret = getsockopt(pty, SOL_SOCKET, SO_RCVBUF, &size, &len);
	    if (ret < 0)
        {
		    context->sock_bufsize = SSH_MIN_READSIZE;
        }
	    else
        {
		    context->sock_bufsize = size;
        }
		c = 1;
		send(pty, &c, 1, 0);
        return (0);
    }
    else
    {
        /*isiah.2003-01-27*/
        s_close(pty);
        return -1;
    }
}

/*
 * PRNG wrapper functions.  Replace with calls to your favorite PRNG if
 * desired.
 *
 * A note on randinit/randclean:  If you have hardware randomness, you
 * may want to modify randinit to use it.  On NetBSD, we use the system
 * RNG/PRNG, specifying the device node which generates to a PRNG if no
 * "hard" random data is available so that we don't block at startup.  We
 * do mix in some other gunk just in case, but you *really* want the kernel
 * random source turned on -- no great effort is made elsewise.
 *
 * In such an implementation, randclean just zeroes out the internal PRNG
 * state for neatness/safety.  But an alternate implementation might try to
 * preserve PRNG state across runs; then randclean would save the state, and
 * randinit would load it, hopefully mixing in some new seed randomness.
 *
 * XXX 02/15/2001 - Cause ssh to exit immediately if we don't have a
 *    device from which to get random data.  
 */

#ifdef UNSAFE_RANDOM
#undef UNSAFE_RANDOM
#endif
 
/*
 * ssh_sys_randinit:	initialize random number generator.
 */
void ssh_sys_randinit()
{
	u_int8_t ubuf[128];

	ssh_rand_feed(ubuf, sizeof(ubuf));
    memset(ubuf, 0, sizeof(ubuf));
}

/*
 * ssh_sys_randclean:       clean up (possibly save) PRNG state.
 */
#if 0 /*isiah.2002-10-22*/
void ssh_sys_randclean()
{
    ssh_rand_clean();
}

/*
 * ssh_sys_randadd:	Add some external randomness to the mix.  Must be
 *			fast, called during normal operation.
 */
void ssh_sys_randadd() 
{
	u_int8_t ubuf[128];

	ssh_rand_feed(ubuf, sizeof(ubuf));
	memset(ubuf, 0, sizeof(ubuf));

}
#endif /* #if 0 */
    
void ssh_sys_exit() 
{
	sshd_context_t		*ssh_context=NULL;
	UI32_T	tid;

	ssh_context = (sshd_context_t *)SSHD_VM_GetContextAddress();

    if ( ssh_context != NULL )
    {
        if (ssh_context->cipher->key_data)
        {
            free(ssh_context->cipher->key_data);
        }
	memset(ssh_context->cipher,0,sizeof(ssh_cipher_t));
	ssh_context->usepty = 0;
	if ( ssh_context->username )
	{
	    free(ssh_context->username);
	    ssh_context->username = NULL;
	}

        if (ssh_context->s > 0)
		s_close(ssh_context->s);

        if (ssh_context->tmp_hostkey)
        {
            EVP_PKEY_free(ssh_context->tmp_hostkey);
        }
        if (ssh_context->tmp_serverkey)
        {
            EVP_PKEY_free(ssh_context->tmp_serverkey);
        }
    
        memset(ssh_context->session_id, 0, 16);
        memset(ssh_context->session_key, 0, SSH_SESSION_KEY_SIZE);
        packet_cleanup(&ssh_context->in_pc);
        packet_cleanup(&ssh_context->out_pc);
    }

	tid = SYSFUN_TaskIdSelf();
	SSHD_VM_ResetTaskID(tid);
/* isiah.2004-01-02*/
/*move session number to CLI */
    CLI_MGR_DecreaseRemoteSession();
/*    SSHD_VM_SetCreatedSessionNumber( SSHD_VM_GetCreatedSessionNumber()-1 );*/

	SYSFUN_DeleteTask (0);
}

int ssh_sys_configuration(struct sshd_context *context) 
{
	context->max_sock_bufsize = SSH_MAX_READSIZE;

    context->max_packet_size = SSH_MAX_PACKETSIZE;

    if ( SSHD_VM_GetHostkey(context) )
    {
        return (0);
    }
    printf("sshd:get hostkey error!!\n");
    return (1);
}



