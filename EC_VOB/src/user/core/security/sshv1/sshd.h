/* $Id: sshd.h,v 1.56.2.2 2001/02/11 04:58:53 tls Exp $ */

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


#ifndef _SSHD_H
#define _SSHD_H

//Isiah.
//#include <sys/ttycom.h>
#include "ssh_types.h"
#include <sys/types.h>
/*Isiah. 2002-05-27 
	Use phase2 socket, mask sys/socket.h and netinet/in.h,
	add "skt_vx.h" and "socket.h" */
/*#include <netinet/in.h>*/
#include "skt_vx.h"
#include "socket.h"


/*#include <stdio.h>*/

/*Isiah.2002-05-27*/
/* Don't support compress now */
/*#define alloc_func	z_alloc_func
#define free_func	z_free_func
#include <zlib.h>
#undef alloc_func
#undef free_func*/

#include <rsa.h>

#include "options.h"
/*isiah.2002-07-11*/
/*#include "time.h"*/
#include "ssh_crypto.h"
/*#include "ssh_logging.h"*/
#include "ssh_packet.h"
#include "ssh_sys.h"

/*Isiah.2002-06-05*/
#include "sys_adpt.h"
#include <evp.h>

struct ssh_cipher;

/* Version: */
#define PROTO_MAJOR	1
#define PROTO_MINOR	5
//#define SSHD_REV	"FreSSH.0.3"
/*Isiah.2002-06-13*/
#define SSHD_REV	"SSH.0.1"

/* Size of stuff: (in bytes) */
#define SSH_COOKIE_SIZE		8
#define SSH_SESSION_KEY_SIZE	32

/* Default values for some options. */
/*#define SSHD_HOSTKEY		"/etc/sshd.key"
#define SSHD_PIDFILE		"/var/run/sshd.pid"
#define SSHD_PORT		"22"

#define SSHD_REGENCONNS		16384
#define SSHD_REGENINTERVAL	1200
#define SSHD_SERVERKEYBITS	768*/

/* Minimum size buffer to use for reads. */
#define SSH_MIN_READSIZE	8192
#define SSH_MAX_READSIZE	(100 * 1024)
#define SSH_CRCCOMP_MAX_READSIZE	(64 * 1024)
/* -------------------------------------- */

typedef int flag;
struct sshd_options {
	char *hostkey;		/* Filename of host key. */
	flag keepalive;
	char *address;	/* Address to listen on. */
	char *port;	/* Port to listen on. */
	char *pidfile;
};

typedef struct sshd_context 
{
//	struct sshd_options opts;

    u_int32_t supported_ciphers;
	u_int32_t supported_auths;
	int listen_socket;
	ssh_RSA *serverkey;
	ssh_RSA *hostkey;
	EVP_PKEY *tmp_hostkey;
	EVP_PKEY *tmp_serverkey;

	int s;				/* socket */
	struct sockaddr_in saddr;	/* address of connecting party */
	int child_stdin;
	int child_stdout;
	int child_stderr;
	int max_packet_size;
	int sock_bufsize;
	int max_sock_bufsize;
	struct ssh_cipher *cipher;	/* type, enc/dec functions, key data */
	struct ssh_packet in_pc;
	struct ssh_packet out_pc;

//Isiah.
//	z_stream inz;
//	z_stream outz;
	int compressing;

	u_int8_t cookie[8];	/* Cookie used during initial negotiations. */
	u_int8_t session_id[16];
	u_int8_t session_key[SSH_SESSION_KEY_SIZE];

	u_int8_t *username;
/*	struct ssh_password pwent;*/
	int usepty;
//	u_int8_t *term;
//	u_int8_t *modes;
	size_t msize;
//	struct winsize win;
/* Isiah. 2002-06-05 */
    u_int8_t    password[SYS_ADPT_MAX_PASSWORD_LEN+1];
} sshd_context_t;
		

/* sshd_intro.c functions: */
int get_version(sshd_context_t *);
int start_encryption(sshd_context_t *);
int send_serverkeys(sshd_context_t *);
int get_sessionkey(sshd_context_t *);

/* sshd_auth.c functions: */
int get_user(sshd_context_t *);
int auth_user(sshd_context_t *);

/* sshd_prep.c functions: */
int doPrepOps(sshd_context_t *);

/* sshd_iactive.c functions: */
int doInteractive(sshd_context_t *);

/* sshd_main.c functions: */
/*Isiah. 2002-06-04 */
#if 0
void doquit(int);
int cleanup(int);
#endif /*Isiah. end of #if 0 */
int main(int, char **);
int setup(int);
/*int read_conffile(char *);*/

/*isiah.2002-06-04*/
#if 0
int OpenListenSocket(struct in_addr, sshd_context_t *);
#endif /*isiah. end of #if 0 */

/*void set_defaults();*/
int dolisten();
int doServer(struct sshd_context *);

#endif /* _SSHD_H */
