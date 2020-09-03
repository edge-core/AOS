/* $Id: sshd_auth.c,v 1.26.2.1 2000/08/25 09:32:20 tls Exp $ */

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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
/*Isiah. 2002-05-27 .
  mask pwd.h,beacuse not found in vxworks */
/*#include <pwd.h>*/

#include <md5ssl.h>

#include "options.h"

#include "sshd.h"
#include "ssh_buffer.h"
#include "ssh_packet.h"
/*#include "ssh_parse.h"*/
#include "ssh_messages.h"
#include "ssh_sys.h"

extern int Accton_select(int fdp1, fd_set *rfds,fd_set *wfds,fd_set *efds,struct timeval *timeout);

/* Isiah.2002-06-04 */
#include "sshd_vm.h"

/* Local function prototypes: */
#ifdef WITH_AUTH_RSA
int auth_user_rsa(sshd_context_t *context);
#endif
#ifdef WITH_AUTH_PASSWORD
int auth_user_password(sshd_context_t *context);
#endif

/* Routines to get and authenticate the user. */

/*
 * get_user: Listen for CMSG_USER message from the client.
 *
 *	Args: s		socket to talk on.
 */
int get_user(sshd_context_t *context) 
{
	size_t ulen;
	struct ssh_packet *pc;
	int num;
/*Isiah.2002-06-11*/
    fd_set  readfds;
    struct 	timeval     timeout;

/*    SSH_DLOG(5, ("get_user: start\n"));*/

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
            SEND_DISCONNECT(context, "Authentication has timed out.\n");
            return (1);
        }
    }

    do 
	{
		if ((num = ssh_read_packet(context, pc, PKT_WAITALL)) > 0)
			num = process_packet(context, pc);
    } while (num == 0);
    if (num < 0)
		return(1);

    /* Only CMSG_USER is valid now. */
    if (packet_type(pc) == SSH_CMSG_USER) 
	{

		/* Save the user-name for later. */
		/* don't give any indication of existance yet. */
		if (packet_get_binstr(pc, &(context->username), &ulen) != 0) 
		{
			SEND_DISCONNECT(context, "Failed to get username.");
			return(1);
		}
		context->username[ulen] = '\0';		/* make sure it's terminated.*/
/*Isiah.2002-07-01*/
        SSHD_VM_SetSshConnectionUsername(context->username);
		return(0);
    } 
	else 
	{
		SEND_DISCONNECT(context, "Received packet is not for username.\n");
		return(1);
    }
}

/*
 * auth_user: implements actual authentication.
 */
int auth_user(sshd_context_t *context) 
{
	int retval, num;
	int tries;
/* Isiah.2002-06-04 */
	UI32_T	sshd_retries;
/*Isiah.2002-06-11*/
    fd_set  readfds;
    struct 	timeval     timeout;
    char tmpbuf[60];



    tries = 0;

    if(ssh_sys_checkpw(context->username, "") == 0) 
	{
        /*isiah.2002-10-24*/
        /* save no password */
        strcpy(context->password,"");
		if ((retval = SEND_SUCCESS(context)) != 0) 
		{
/*			SSH_ERROR("Unable to xmit success in auth_user:%d\n", retval);*/
			return(1);
		}
		return(0);
    }

    sshd_retries = SSHD_VM_GetAuthenticationRetries();
    
/*isiah.2002-07-18 ,for OpenSSH issue
    do 
	{*/
        if((retval = SEND_FAILURE(context)) != 0) 
		{
/* 			SSH_ERROR("Unable to xmit failure in auth_user:%d\n", retval);*/
			return(1);
		}

    do
    {
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
                SEND_DISCONNECT(context, "Authentication has timed out.\n");
                return (1);
            }
        }
		
		do 
		{
			if ((num = ssh_read_packet(context, &context->in_pc, PKT_WAITALL)) > 0)
			{
				num = process_packet(context, &context->in_pc);
			}
		} while (num == 0);
		if (num < 0)
 			return(1);

		switch(context->in_pc.p_type) 
		{
#ifdef WITH_AUTH_PASSWORD
			case SSH_CMSG_AUTH_PASSWORD:
				if ((retval = auth_user_password(context)) != 0) 
				{
//					SSH_DLOG(2, ("password authentication failed.\n"));
					if (retval < 0)
						return(retval);
				} 
				else
					return(0);
				break;
#endif
#ifdef WITH_AUTH_RSA
			case SSH_CMSG_AUTH_RSA:
				if ((retval = auth_user_rsa(context)) != 0) 
				{
//					SSH_DLOG(2, ("RSA authentication failed.\n"));
					if (retval < 0)
						return(retval);
				} 
				else
					return(0);
				break;
#endif
			default: /*nothing*/;
		}

		tries++;
/*Isiah.2002-06-4*/
	} while ( tries < sshd_retries );
/*	} while (tries < 8);*/	/* XXX Adjust this number. */
				/* XXX perhaps allow only one password try. */
				/* XXX and a list of RSA keys tried. */

    sprintf(tmpbuf, "Maximum number of authentication attempts (%d) exceeded.\n", sshd_retries);
    SEND_DISCONNECT(context, tmpbuf);
    return(1);
}

#ifdef WITH_AUTH_RSA
int auth_user_rsa(sshd_context_t *context) {
  int ret, retval;
  u_int8_t *recv_md5;
  u_int8_t calc_md5[16];
  char challenge[32];
  char *encr_chal;
  int encr_len;
  MD5_CTX md5ctx;
  ssh_BIGNUM *n, *challenge_as_bn;
  ssh_RSA *key;
  struct ssh_buf xbuf;

    ret = 0;
    recv_md5 = NULL;
    n = challenge_as_bn = NULL;
    key = NULL;
    memset(&xbuf, 0, sizeof(struct ssh_buf));

    /* Grab the public key modulus that the client sent. */
    if (packet_get_bignum(&context->in_pc, &n) != 0) {
/*	SSH_DLOG(3, ("auth_user_rsa: AUTH_RSA msg has bad n.\n"));*/
	SEND_DISCONNECT(context, "Bad SSH_AUTH_RSA message\n");
	ret = -1 ; goto out;
    }

    /* Make sure it's at least big enought to encrypt the challenge. */
    if (bignum_num_bytes(n) < 32) {
/*	SSH_DLOG(3, ("auth_user_rsa: key too short.\n"));*/
	ret = 1; goto out;
    }

    /* See if it's in our list of valid keys. */
    if (lookup_authorized_key(context, n, &key) != 0) {
/*	SSH_DLOG(3, ("auth_user_rsa: key not found.\n"));*/
	ret = 1 ; goto out;
    }

    /* Generate the challenge */
    ssh_rand_bytes(32, challenge);

    /* Calculate the MD5 of the challenge+session id. */
    MD5_Init(&md5ctx);
    MD5_Update(&md5ctx, challenge, 32);
    MD5_Update(&md5ctx, context->session_id, 16);
    MD5_Final(calc_md5, &md5ctx);

    /* Encrypt the challenge and send it. */
    encr_chal = malloc(bignum_num_bytes(ssh_rsa_npart(key)));
    if (encr_chal == NULL) {
/*	SSH_ERROR("auth_user_rsa: Unable to alloc mem.\n");*/
	SEND_DISCONNECT(context, "auth_user_rsa: Unable to alloc mem.\n");
	ret = -1 ; goto out;
    }

    if ((encr_len = ssh_rsa_public_encrypt(32, challenge, encr_chal, key)) < 32) {
/*	SSH_ERROR("auth_user_rsa: Unable to encrypt challenge.\n");*/
	SEND_DISCONNECT(context,
			"auth_user_rsa: Unable to encrypt challenge.\n");
	ret = -1 ; goto out;
    }

    /* Transmit the challenge as a mpint (bignum) */
    if (bignum_bin2bn(&challenge_as_bn, encr_chal, encr_len) != 0) {
/*	SSH_ERROR("auth_user_rsa: bin2bn on challenge failed.\n");*/
	SEND_DISCONNECT(context, "auth_user_rsa: bin2bn on challenge failed.\n");
	ret = -1; goto out;
    }

    if (buf_alloc(&xbuf, encr_len + 10, &ret) == NULL) {		/* + extra to prevent realloc in buf_put_ */
/*	SSH_ERROR("auth_user_rsa: Unable to alloc buf for xmit.\n");*/
	SEND_DISCONNECT(context, "auth_user_rsa: Unable to alloc buf for xmit.\n");
/*	ret = -1;*/ goto out;
    }

    if (buf_put_bignum(&xbuf, challenge_as_bn) != 0) {
/*	SSH_ERROR("auth_user_rsa: Unable to put challenge in buffer.\n");*/
	SEND_DISCONNECT(context, "auth_user_rsa: Unable to put challenge in buffer.\n");
	ret = -1; goto out;
    }

    if (xmit_packet(context, SSH_SMSG_AUTH_RSA_CHALLENGE, &xbuf, PKT_WAITALL) < 0) {
/*	SSH_DLOG(2, ("auth_user_rsa: unable to xmit challenge.\n"));*/
	ret = -1 ; goto out;
    }

    if (ssh_read_packet(context, &(context->in_pc), PKT_WAITALL) <= 0) {
/*	SSH_DLOG(2, ("auth_user_rsa: unable to read rsa response.\n"));*/
	SEND_DISCONNECT(context, "auth_user_rsa: read rsa response failed.\n");
	ret = -1 ; goto out;
    }
    if (process_packet(context, &(context->in_pc)) <= 0) {
/*	SSH_DLOG(2, ("auth_user_rsa: unable to process rsa response packet.\n"));*/
	SEND_DISCONNECT(context, "auth_user_rsa: decode rsa response packet failed.\n");
	ret = -1 ; goto out;
    }

    if (packet_type(&(context->in_pc)) != SSH_CMSG_AUTH_RSA_RESPONSE) {
/*	SSH_DLOG(2, ("auth_user_rsa: bad packet type for rsa response (%d).\n", packet_type(&(context->in_pc))));*/
	SEND_DISCONNECT(context, "auth_user_rsa: expected AUTH_RSA_RESPONSE.\n");
	ret = -1 ; goto out;
    }

    /* Grab the MD5 response from the client. */
    if (packet_get_nbytes(&context->in_pc, 16, &recv_md5) != 0) {
/*	SSH_DLOG(2, ("auth_user_rsa: bad rsa response.\n"));*/
	ret = 1 ; goto out;
    }

    /* Compare it to our calculated one. */
    if (memcmp(recv_md5, calc_md5, 16) != 0) {
	if ((retval = SEND_FAILURE(context)) != 0)
/*            SSH_ERROR("Unable to xmit failure in auth_user_rsa: %d\n", retval);*/
                ;
	ret = 1; goto out;
    }

    if((retval = SEND_SUCCESS(context)) != 0) {
/*	SSH_ERROR("Unable to xmit success in auth_user_rsa: %d\n", retval);*/
	ret = 1; goto out;
    }

    /* All ok */
/*    SSH_DLOG(3, ("auth_user_rsa: authentication accepted.\n"));*/

out:
    if (recv_md5) {
	memset(recv_md5, 0, 16);
	free(recv_md5);
    }
    if (n)
	bignum_free(n);
    if (key)
	ssh_rsa_free(key);
    memset(calc_md5, 0, 16);
    return(ret);
}

#endif /* WITH_AUTH_RSA */

#ifdef WITH_AUTH_PASSWORD
int auth_user_password(sshd_context_t *context) 
{
    int retval;
    size_t pwlen;
    u_int8_t *password;

    if (packet_get_binstr(&context->in_pc, &password, &pwlen) != 0) 
    {
/*	    SSH_DLOG(1, ("Unable to get password: %s\n", strerror(errno)));*/
	    SEND_DISCONNECT(context, "Failed to get password.");
	    return(1);
    }

    if (ssh_sys_checkpw(context->username, password) != 0) 
    {
	    if ((retval = SEND_FAILURE(context)) != 0)
/*            SSH_ERROR("Unable to xmit failure in auth_user: %d\n", retval);*/
                ;
        /*isiah.2003-01-21*/
        free(password);
	    return(1);
    }

    if((retval = SEND_SUCCESS(context)) != 0) 
    {
/*	    SSH_ERROR("Unable to xmit success in auth_user: %d\n", retval);*/
	    /*isiah.2003-01-21*/
	    free(password);
	    return(1);
    }
/*Isiah. 2002-06-05 */
/* keep password */
    memset(context->password, 0, SYS_ADPT_MAX_PASSWORD_LEN+1);
    strncpy(context->password,password, SYS_ADPT_MAX_PASSWORD_LEN);

    free(password);

    return(0);
}
#endif /* WITH_AUTH_PASSWORD */
