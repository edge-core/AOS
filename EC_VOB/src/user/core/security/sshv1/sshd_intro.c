/* $Id: sshd_intro.c,v 1.43.2.2 2000/10/24 16:45:31 erh Exp $ */

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


/* Functions used during non-encrypted introduction */
/* phase of the ssh connection.  These are used to  */
/* negociate the protocol version and to set the    */
/* parameters to encrypt the rest of the interaction. */

/*#include <errno.h>*/
/*#include <stdio.h>*/
#include <string.h>
#include <signal.h>
#include <stdlib.h>
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

/*#include <unistd.h>*/

#include <md5ssl.h>

#include "options.h"

#include "sshd.h"
#include "ssh_auth.h"
#include "ssh_buffer.h"
#include "ssh_cipher.h"
#include "ssh_crypto.h"
#include "ssh_datatypes.h"
#include "ssh_messages.h"
#include "ssh_packet.h"
#include "ssh_rsakeys.h"
#include "ssh_sys.h"
#include "ssh_util.h"

/*Isiah. 2002-05-28 */
#include "ssh_def.h"
#include "sysfun.h"
#include "sshd_mgr.h"
#include "sshd_vm.h"

#define RECV_BUFSIZE	100

extern int sscanf( const char *buffer, const char *format, ... );


/*
 * check_version_compat: check compatibility between versions.
 *			set any necessary emulation parameters.
 */
int check_version_compat(struct sshd_context *context,
				int major, int minor, char *softver) 
{
    static char *c = "Protocol mismatch.";
    int s_major, s_minor, s_point;
    int s_date;
    int setlowsize;

    setlowsize = 0;

    if ((major == PROTO_MAJOR) && (minor == PROTO_MINOR)) 
    {
	    /* Look for an F-secure softver string. */
	    /* They can't handle really big packets. */
	    if (sscanf(softver, "%d.%d.%d", &s_major, &s_minor, &s_point) == 3) 
	    {
/*	        SSH_DLOG(4, ("check_version_compat: F-SECURE version string\n"));*/
	        do 
	        {
		        if (s_major > 1)
		            break;		/* Assume it's been fixed. */
		        if (s_major == 1 && s_minor > 2)
		            break;		/* Assume it's been fixed. */
#if 0
		        if (s_major == 1 && s_minor == 2 && s_point > FOO)
		            break;
#endif
		        setlowsize = 1;
		        break;
	        } while (0);
	    }
	    if (sscanf(softver, "TTSSH-%d.%d", &s_major, &s_minor) == 2) 
	    {
/*	        SSH_DLOG(4, ("check_version_compat: TTSSH version string\n"));*/
	        do 
	        {
		        if (s_major > 1)
		            break;		/* Assume it's been fixed. */
		        if (s_minor > 2)
		            break;		/* Assume it's been fixed. */
		        setlowsize = 1;
	        } while (0);
	    }
	    else if (sscanf(softver, "NetBSD_Secure_Shell-%d", &s_date) == 1)
	    {
/*	        SSH_DLOG(4, ("NetBSD OpenSSH version string\n"));*/
	        setlowsize = 1;  /* XXX Check the date string when openssh is fixed. */
	    }
	    if (setlowsize) 
	    {
	        context->max_sock_bufsize = SSH_CRCCOMP_MAX_READSIZE;
/*	        SSH_DLOG(4, ("check_version_compat: max_sock_bufsize set to:%d\n",
			    		context->max_sock_bufsize));*/
	    }
	    return(0);
    }

    write(context->s, c, strlen(c));
    return(1);
}



/*
 * get_version: talk to client and negotiate a protocol version.
 */
int get_version(sshd_context_t *context) 
{
    char *verstr;
    char abuf[RECV_BUFSIZE];
    int count;
    int major, minor;
    char softver[RECV_BUFSIZE];
/*    struct itimerval itval;*/
/*Isiah.2002-06-11*/
    fd_set  readfds;
    struct 	timeval     timeout;
    int     ret_errno;


    /* Send version string ("SSH-#.#-foo") */
    verstr = build_version();
	while ( (ret_errno = write(context->s, verstr, strlen(verstr))) < 0) 
    {
	    if (ret_errno == ENOBUFS) 
        {
            SYSFUN_Sleep(100);
            continue;
	    }
/*	    SSH_DLOG(3, 
		    ("Client disconnected before we could send the version.\n"));*/
		/*isiah.2003-01-21*/
		free(verstr);
	    return(1);
    }
    /*isiah.2003-01-21*/
	free(verstr);

    /* listen for response. */
/*isiah.2002-06-11*/
    FD_ZERO(&readfds);
    FD_SET(context->s, &readfds);
   	timeout.tv_sec = SSHD_VM_GetNegotiationTimeout();     /*  no.  of seconds  */
    timeout.tv_usec = 0;    /*  no. of micro seconds  */
    if ( select(context->s + 1, &readfds, NULL, NULL, &timeout) <= 0 )
    {
        write(context->s,"Negotiation has timed out.\n",strlen("Negotiation has timed out.\n"));
        return (-1);
    }

    while ((count = read(context->s, abuf, RECV_BUFSIZE - 1)) < 0) 
    {
	    if (count == EWOULDBLOCK)
	        continue;
/*	    SSH_DLOG(3, ("Client disconnected before sending its version.\n"));*/
	    return(1);
    }

    if (count == (RECV_BUFSIZE - 1)) 
    {
/*	    SSH_DLOG(2, ("Caution: client filled get_version recv buffer.\n"));*/
    }
    abuf[count] = '\0';

    if (sscanf(abuf, "SSH-%d.%d-%[^\n]\n", &major, &minor, softver) != 3) 
    {
/*	    SSH_DLOG(4, ("Client version response didn't pass sscanf\n"));
	    SSH_DLOG(4, ("buf=%s\n", abuf));*/
	    major = -1;	/* force incompatibility. */
    }

/*isiah.2002-06-10*/
    if ( (SSHD_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE) || (SSHD_VM_GetSshdStatus() != SSHD_STATE_ENABLED) )
    {
        return (-1);
    }

    if (check_version_compat(context, major, minor, softver) == 0) 
    {
/*	    SSH_DLOG(3, ("Client connected with version %d.%d-%s\n",
	    					major, minor, softver));*/
/*Isiah.2002-06-28*/
        SSHD_VM_SetSshConnectionVersion(major,minor);	    					
	    return(0);
    } 
    else 
    {
	    /* Incompatible versions.  Error message has already been sent. */
/*        SSH_DLOG(3, ("Incompatible client version %d.%d-%s\n",
						major, minor, softver));*/
	    return(-1);
    }
}

/*
 * start_encryption: figure out how to encrypt the connection.
 */
int start_encryption(sshd_context_t *context) 
{
	int retval;

/*    SSH_DLOG(4, ("start_encryption\n"));*/
    if (send_serverkeys(context) != 0)
		return(1);

    if (get_sessionkey(context) != 0)
		return(1);

    /* Tell client we're ready to go. */
    if ((retval = SEND_SUCCESS(context)) != 0) 
	{
/*		SSH_ERROR("Unable to send encryption-on success msg:%d\n", retval);*/
		return(1);
    }

/*isiah.2002-06-10*/
    if ( (SSHD_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE) || (SSHD_VM_GetSshdStatus() != SSHD_STATE_ENABLED) )
    {
        return (1);
    }

    return(0);
}

/*
 * send_serverkeys: Create and send PUBLIC_KEY message.
 *		Determine session_id.
 */
int send_serverkeys(sshd_context_t *context) 
{
    struct ssh_buf buf;
    u_int8_t *abuf, *apos;
    u_int32_t sup_ciphers, sup_auths;
    MD5_CTX md5ctx;
    int plen;
    int ret_errno;

/*    SSH_DLOG(4, ("send_serverkeys\n"));*/

    memset(&buf, 0, sizeof(struct ssh_buf));

    plen =
	/* cookie */
	8 +
	/* Server key bits, exponent and modulus. */
	4 + 2 + bignum_num_bytes(ssh_rsa_npart(context->serverkey)) +
	2 + bignum_num_bytes(ssh_rsa_epart(context->serverkey)) +
	/* Host key bits, exponent and modulus. */
	4 + 2 + bignum_num_bytes(ssh_rsa_npart(context->hostkey)) +
	2 + bignum_num_bytes(ssh_rsa_epart(context->hostkey)) +
	/* Protocol flags, supported ciphers and auths */
	4 + 4 + 4;

    if (buf_alloc(&buf, plen, &ret_errno) == NULL) 
    {
/*	    SSH_ERROR("send_serverkey: Unable to init PUBLIC_KEY packet.\n");*/
	    return(1);
    }

    /* Generate the random cookie. */
    ssh_rand_bytes(8, context->cookie);

    /*
     * Build the packet:
     */

    /* Copy in the cookie. */
    buf_put_nbytes(&buf, 8, context->cookie);

    /* Drop in the server key. */
    buf_put_rsa_publickey(&buf, context->serverkey);

    /* Drop in the host key. */
    buf_put_rsa_publickey(&buf, context->hostkey);

    /* XXX no protocol flags for now? */
    buf_put_int32(&buf, 0);

    /* Fill in supported ciphers. */
    sup_ciphers = cipher_supported();
    buf_put_int32(&buf, sup_ciphers);

    /* Fill in supported authentication methods. */
    sup_auths = auths_supported();
    buf_put_int32(&buf, sup_auths);

    /* Generate the session id: */
    plen = bignum_num_bytes(ssh_rsa_npart(context->serverkey)) +
			bignum_num_bytes(ssh_rsa_npart(context->hostkey)) + 8;
    if ((abuf = malloc(plen)) == NULL) 
    {
/*	    SSH_ERROR("send_serverkeys: malloc for session_id failed: %s\n",
		    					strerror(errno));*/
	    buf_cleanup(&buf);
	    return(1);
    }
    bignum_bn2bin(ssh_rsa_npart(context->hostkey), abuf);
    apos = abuf + bignum_num_bytes(ssh_rsa_npart(context->hostkey));
    bignum_bn2bin(ssh_rsa_npart(context->serverkey), apos);
    apos += bignum_num_bytes(ssh_rsa_npart(context->serverkey));
    memcpy(apos, context->cookie, 8);

    MD5_Init(&md5ctx);
    MD5_Update(&md5ctx, abuf, plen);
    MD5_Final(context->session_id, &md5ctx);
    free(abuf);

    /* transmit packet. */
    if ((xmit_packet(context, SSH_SMSG_PUBLIC_KEY, &buf, PKT_WAITALL)) != 0) 
    {
/*	    SSH_DLOG(1, ("send_serverkeys: Unable to send public key\n"));*/
	    buf_cleanup(&buf);
	    return(1);
    }
    buf_cleanup(&buf);
    return(0);
}


/*
 * get_sessionkey: Listen for and decode SESSION_KEY message from client.
 *
 *	Args: s		socket to talk on.
 */
int get_sessionkey(sshd_context_t *context) 
{
    struct ssh_mpint enc_session_key;
    u_int8_t cipher_type;
    u_int32_t pflags;
    u_int8_t *cookie;
    struct ssh_packet *pc;
    int num;
/*Isiah.2002-06-11*/
    fd_set  readfds;
    struct 	timeval     timeout;

/*    SSH_DLOG(5, ("get_sessionkey: start\n"));*/

    pc = &(context->in_pc);

/*isiah.2002-06-11*/
    FD_ZERO(&readfds);
    FD_SET(context->s, &readfds);
    timeout.tv_sec = SSHD_VM_GetNegotiationTimeout();     /*  no.  of seconds  */
    timeout.tv_usec = 0;    /*  no. of micro seconds  */
    if ( select(context->s + 1, &readfds, NULL, NULL, &timeout) <= 0 )
    {
        context = (sshd_context_t *)SSHD_VM_GetContextAddress();
        if (context==NULL)
        {
            return (1);
        }
        else
        {
            SEND_DISCONNECT(context, "Negotiation has timed out.\n");
            return (1);
        }
    }

    do 
    {
	    if ((num = ssh_read_packet(context, pc, PKT_WAITALL)) > 0)
	        num = process_packet(context, pc);
    } while (num == 0);
    if (num < 0) 
    {
/*	    SSH_DLOG(3, ("Unable to get packet for session key.\n"));*/
	    return(1);
    }

    if (packet_type(pc) != SSH_CMSG_SESSION_KEY) 
    {
	    SEND_DISCONNECT(context, "Received packet is not for encryption key.\n");
	    return(1);
    }

    /* Get data out of packet. */
    /* Note: be sure to free enc_session_key->data and cookie. */
    if ((packet_get_byte(pc, &cipher_type) |
	packet_get_nbytes(pc, 8, &cookie) |
	packet_get_mpint(pc, &enc_session_key) |
	packet_get_int32(pc, &pflags)) != 0) 
	{
/*	    SSH_DLOG(1, ("get_sessionkey: packet error.\n"));*/
	    if (cookie) 
	        free(cookie);
	    if (enc_session_key.data) 
	        free(enc_session_key.data);
	    return(1);
    }

    if (!((1 << cipher_type) && cipher_supported())) 
    {
/*	    SSH_DLOG(1, ("Client tried to use unsupported cipher type.\n"));*/
	    free(enc_session_key.data);
	    free(cookie);
	    return(1);
    }

    if (memcmp(cookie, context->cookie, SSH_COOKIE_SIZE) != 0) 
    {
/*	    SSH_DLOG(1, ("Non-matching cookie returned\n"));*/
	    free(enc_session_key.data);
	    free(cookie);
	    return(1);
    }
    free(cookie);

    /* XXX check pflags?   Nothing to do with it for now. */

    if (decrypt_session_key(context, &enc_session_key) != 0) 
    {
/*	    SSH_DLOG(1, ("Failed to decrypt session key.\n"));*/
	    memset(enc_session_key.data, 0, (enc_session_key.bits + 7) / 8);
	    free(enc_session_key.data);
	    return(1);
    }
    memset(enc_session_key.data, 0, (enc_session_key.bits + 7) / 8);
    free(enc_session_key.data);

    if (set_cipher_type(context, cipher_type) != 0) 
    {
/*	    SSH_DLOG(1, ("Unable to set cipher type.\n"));*/
	    return(1);
    }

    if ((context->cipher->key_data =
		context->cipher->initialize(context->session_key)) == NULL) 
	{
/*	    SSH_DLOG(1, ("Unable to initialize cipher:%s\n", 
	    					cipher_name(cipher_type)));*/
	    return(1);
    }

/*    SSH_DLOG(5, ("get_sessionkey: finished.\n"));*/
    return(0);
}

