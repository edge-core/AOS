/* $Id: ssh_sys_posix.c,v 1.11.2.1 2000/08/25 09:32:18 tls Exp $ */

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
#include <string.h>
#include <stdlib.h>
/*Isiah. 2002-05-27 .
  mask termios.h,beacuse not found in vxworks */
/*#include <termios.h>*/
#include <sys/types.h>
/*#include <unistd.h>*/
/*Isiah. 2002-05-27 .
  mask pwd.h,beacuse not found in vxworks */
/*#include <pwd.h>*/

#include "options.h"

#include "sshd.h"
#include "ssh_buffer.h"
#include "ssh_util.h"
/*#include "ssh_tty.h"*/

/*Isiah.2002-06-04*/
#include <md5ssl.h>
#include "sys_adpt.h"
#include "userauth.h"

/*
 * ssh_sys_set_tty_modes: Set the flags in the termios structure according to
 *		the tty flags passed in modes.
 */
/*isiah.2002-05-27*/
#if 0
int ssh_sys_set_tty_modes(struct termios *termp, u_int8_t *modes, int msize) {
  u_int8_t op;
  u_int8_t arg8;
  u_int32_t arg32;
    while(msize > 0) {
	op = (u_int8_t)*(modes);
	if (op == SSH_TTY_OP_END) 
	    break;
	else if (op <= 127) {
	    if (msize < 2) {
/*		SSH_DLOG(1, ("Corrupted tty modes packet.\n"));*/
		return(1);
		break;
	    }
	    arg8 = (u_int8_t)*(modes + 1);
	    modes += 2;
	    msize -= 2;
	} else if (op <= 159) {
	    if (msize < 5) {
/*		SSH_DLOG(1, ("Corrupted tty modes packet.\n"));*/
		return(1);
		break;
	    }
	    arg32 = (u_int32_t)*(modes + 1);
	    modes += 5;
	    msize -= 5;
	} else
	    break;

	switch (op) {
	case SSH_TTY_VINTR:	termp->c_cc[VINTR] = arg8;	break;
	case SSH_TTY_VQUIT:	termp->c_cc[VQUIT] = arg8;	break;
	case SSH_TTY_VERASE:	termp->c_cc[VERASE] = arg8;	break;
	case SSH_TTY_VKILL:	termp->c_cc[VKILL] = arg8;	break;
	case SSH_TTY_VEOF:	termp->c_cc[VEOF] = arg8;	break;
	case SSH_TTY_VEOL:	termp->c_cc[VEOL] = arg8;	break;
	case SSH_TTY_VEOL2:	termp->c_cc[VEOL2] = arg8;	break;
	case SSH_TTY_VSTART:	termp->c_cc[VSTART] = arg8;	break;
	case SSH_TTY_VSTOP:	termp->c_cc[VSTOP] = arg8;	break;
	case SSH_TTY_VSUSP:	termp->c_cc[VSUSP] = arg8;	break;
	case SSH_TTY_VDSUSP:	termp->c_cc[VDSUSP] = arg8;	break;
	case SSH_TTY_VREPRINT:	termp->c_cc[VREPRINT] = arg8;	break;
	case SSH_TTY_VWERASE:	termp->c_cc[VWERASE] = arg8;	break;
	case SSH_TTY_VLNEXT:	termp->c_cc[VLNEXT] = arg8;	break;
	case SSH_TTY_VFLUSH:
	case SSH_TTY_VSWITCH:
		/* unimplemented. */ break;
	case SSH_TTY_VSTATUS:	termp->c_cc[VSTATUS] = arg8;	break;
	case SSH_TTY_VDISCARD:	termp->c_cc[VDISCARD] = arg8;	break;

	/* Input flags: */
	case SSH_TTY_IGNPAR:	SET_IFLAG(termp, arg8, IGNPAR);	break;
	case SSH_TTY_PARMRK:	SET_IFLAG(termp, arg8, PARMRK);	break;
	case SSH_TTY_INPCK:	SET_IFLAG(termp, arg8, INPCK);	break;
	case SSH_TTY_ISTRIP:	SET_IFLAG(termp, arg8, ISTRIP);	break;
	case SSH_TTY_INLCR:	SET_IFLAG(termp, arg8, INLCR);	break;
	case SSH_TTY_IGNCR:	SET_IFLAG(termp, arg8, IGNCR);	break;
	case SSH_TTY_ICRNL:	SET_IFLAG(termp, arg8, ICRNL);	break;
	case SSH_TTY_IUCLC:
		/* unimplemented. */ break;
	case SSH_TTY_IXON:	SET_IFLAG(termp, arg8, IXON);	break;
	case SSH_TTY_IXANY:	SET_IFLAG(termp, arg8, IXANY);	break;
	case SSH_TTY_IXOFF:	SET_IFLAG(termp, arg8, IXOFF);	break;
	case SSH_TTY_IMAXBEL:	SET_IFLAG(termp, arg8, IMAXBEL); break;

	case SSH_TTY_ISIG:	SET_LFLAG(termp, arg8, ISIG);	break;
	case SSH_TTY_ICANON:	SET_LFLAG(termp, arg8, ICANON);	break;
	case SSH_TTY_XCASE:
		/* unimplemented. */ break;
	case SSH_TTY_ECHO:	SET_LFLAG(termp, arg8, ECHO);	break;
	case SSH_TTY_ECHOE:	SET_LFLAG(termp, arg8, ECHOE);	break;
	case SSH_TTY_ECHOK:	SET_LFLAG(termp, arg8, ECHOK);	break;
	case SSH_TTY_ECHONL:	SET_LFLAG(termp, arg8, ECHONL);	break;
	case SSH_TTY_NOFLSH:	SET_LFLAG(termp, arg8, NOFLSH);	break;
	case SSH_TTY_TOSTOP:	SET_LFLAG(termp, arg8, TOSTOP);	break;
	case SSH_TTY_IEXTEN:	SET_LFLAG(termp, arg8, IEXTEN);	break;
	case SSH_TTY_ECHOCTL:	SET_LFLAG(termp, arg8, ECHOCTL);	break;
	case SSH_TTY_ECHOKE:	SET_LFLAG(termp, arg8, ECHOKE);	break;
	case SSH_TTY_PENDIN:	SET_LFLAG(termp, arg8, PENDIN);	break;

	/* Output flags: */
	case SSH_TTY_OPOST:	SET_OFLAG(termp, arg8, OPOST);	break;
	case SSH_TTY_OLCUC:
		/* unimplemented. */ break;
	case SSH_TTY_ONLCR:	SET_OFLAG(termp, arg8, ONLCR);	break;
	case SSH_TTY_OCRNL:	SET_OFLAG(termp, arg8, OCRNL);	break;
	case SSH_TTY_ONOCR:	SET_OFLAG(termp, arg8, ONOCR);	break;
	case SSH_TTY_ONLRET:	SET_OFLAG(termp, arg8, ONLRET);	break;

	case SSH_TTY_CS7:
		if (arg8) {
		    termp->c_cflag &= ~CSIZE;
		    termp->c_cflag |= CS7;
		}
		break;
	case SSH_TTY_CS8:
		if (arg8) {
		    termp->c_cflag &= ~CSIZE;
		    termp->c_cflag |= CS8;
		}
	case SSH_TTY_PARENB:	SET_CFLAG(termp, arg8, PARENB);	break;
	case SSH_TTY_PARODD:	SET_CFLAG(termp, arg8, PARODD);	break;
	default:
		break;
	}
    }
    return(0);
}
#endif /*isiah. end of #if 0*/


int ssh_sys_checkpw(char *uname, char *pw) 
{
/*Isiah.2002-06-04*/
#if 0
	USERAUTH_LoginLocal_T	login_user;
	UI8_T	encrypted_password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN+1] = {0};
	MD5_CTX	md5ctx;
    I32_T   radius_privilege;
    I32_T   auth, enable;
#else 
    UI32_T  priv;
#endif 
    
//	memset(&login_user, 0, sizeof(login_user));
//	strcpy(login_user.username, uname);
#if 1
    if ( USERAUTH_LoginAuth(uname, pw, &priv) == TRUE )
    {
        return 0;
    }
    else
    {
        return -1;
    }
#else
	/* USERAUTH_AUTH_LOCAL */
	if ( USERAUTH_GetLoginLocalUser(&login_user) )
	{
		memset(encrypted_password, 0, sizeof(encrypted_password));
		MD5_Init(&md5ctx);
		MD5_Update(&md5ctx, pw, strlen(pw));
		MD5_Final(encrypted_password, &md5ctx);
//		L_MD5_MDString(encrypted_password, password, strlen(password));
      
		if ( memcmp(login_user.password, encrypted_password, SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN) == 0 )
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}

#if 0
    /* USERAUTH_AUTH_RADIUS */
    if ( RADIUS_Auth_Check(uname, pw, &radius_privilege) == 0 ) /*success*/
    {
        if ( (radius_privilege == AUTH_ADMINISTRATIVE) || (radius_privilege == AUTH_LOGIN) )
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }

    /*  USERAUTH_AUTH_TACACS */
    auth = TACACS_MGR_Auth_Check(uname, pw);
    if ( auth == 1 )
    {
        enable = TACACS_MGR_Auth_Enable_Requests(password);
        if ( (enable == 1) || (enable == 2) )
        {
            return 0;
        }
    }
#endif   
#endif  
	return -1;


}


/*
 * ssh_sys_readfile:
 *	Open and read a file into a buffer.
 */
#if 0
int ssh_sys_readfile(char *filename, struct ssh_buf **buf) 
{
}
#endif 
