/* $Id: ssh_sys.h,v 1.12.2.1 2000/08/25 09:32:16 tls Exp $ */

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


/*
 * System dependant function prototypes.
 *	These are functions that need to be implemented
 *	differently depending on the system we're running on.
 */

//Isiah.
#include "ssh_types.h"
#include <sys/types.h>
/*Isiah. 2002-05-27 .
  mask pwd.h,beacuse not found in vxworks */
/*#include <pwd.h>*/
/*Isiah. 2002-05-27 .
  mask termios.h,beacuse not found in vxworks */
/*#include <termios.h>*/

#include "ssh_crypto.h"

/* As in "struct ssh_password" -> "struct passwd" */
/*#define ssh_password	passwd*/

int ssh_sys_allocpty(struct sshd_context *, char *, int, int, int, int, u_int8_t *, size_t);
#if 0 /*isiah.2002-10-22*/
void ssh_sys_setbufsize(struct sshd_context *, int, int);
#endif /* #if 0 */
int ssh_sys_execcmd(struct sshd_context *, char *);
void ssh_sys_exit();
void ssh_sys_randinit();
#if 0 /*isiah.2002-10-22*/
void ssh_sys_randclean();
void ssh_sys_randadd();
#endif /* #if 0 */

/*Isiah.2002-05-27*/
//int ssh_sys_set_tty_modes(struct termios *, u_int8_t *, int);
int ssh_sys_configuration(struct sshd_context *);
int ssh_sys_checkpw(char *, char *);
/*isiah.2002-06-04*/
#if 0
char *ssh_sys_load_keyfile(char *, size_t *);
int ssh_sys_writepid(char *);
int ssh_sys_daemonize();
int ssh_sys_readfile(char *, struct ssh_buf **);
#endif /*isiah. end of #if 0 */

