/*
 * Author: Tatu Ylonen <ylo@cs.hut.fi>
 * Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 *                    All rights reserved
 * Password authentication.  This file contains the functions to check whether
 * the password is valid for the user.
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 *
 * Copyright (c) 1999 Dug Song.  All rights reserved.
 * Copyright (c) 2000 Markus Friedl.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "includes.h"
#include "userauth_pmgr.h"
#include "sshd_vm.h"
RCSID("$OpenBSD: auth-passwd.c,v 1.27 2002/05/24 16:45:16 stevesk Exp $");

#include "packet.h"
#include "log.h"
#include "servconf.h"
#include "auth.h"

//extern ServerOptions options;

/*
 * Tries to authenticate the user using password.  Returns true if
 * authentication succeeds.
 */
int
auth_password(Authctxt *authctxt, const char *password)
{
    USERAUTH_AuthResult_T   auth_result;
    UI32_T                  conn_id;
    I32_T                   connection_in;
    I32_T                   connection_out;
    UI32_T                  rem_port;
    L_INET_AddrIp_T         rem_ip_addr;

    /* Get connected CLI session ID
     */
    SSHD_VM_GetSshConnectionSockId(&connection_in, &connection_out);
    SSHD_VM_GetRemoteSessionName(connection_in, &rem_ip_addr, &rem_port);

    SSHD_VM_GetSshConnectionId(&conn_id);

    if ( USERAUTH_PMGR_LoginAuthBySessionType(authctxt->user,
                                              password,
                                              USERAUTH_SESSION_SSH,
                                              conn_id,
                                              &rem_ip_addr,
                                              &auth_result) == TRUE )
    {
        SSHD_VM_SetSshConnectionAuthResult(&auth_result);
        SSHD_VM_SetSshConnectionPassword(password);
        return 1;
    }
    else
    {
        (authctxt->failures)++;	
        return 0;
    }

#if 0
	struct passwd * pw = authctxt->pw;

	/* deny if no user. */
	if (pw == NULL)
		return 0;
	if (pw->pw_uid == 0 && options.permit_root_login != PERMIT_YES)
		return 0;
	if (*password == '\0' && options.permit_empty_passwd == 0)
		return 0;
#ifdef KRB5
	if (options.kerberos_authentication == 1) {
		int ret = auth_krb5_password(authctxt, password);
		if (ret == 1 || ret == 0)
			return ret;
		/* Fall back to ordinary passwd authentication. */
	}
#endif
#ifdef KRB4
	if (options.kerberos_authentication == 1) {
		int ret = auth_krb4_password(authctxt, password);
		if (ret == 1 || ret == 0)
			return ret;
		/* Fall back to ordinary passwd authentication. */
	}
#endif
#ifdef BSD_AUTH
	if (auth_userokay(pw->pw_name, authctxt->style, "auth-ssh",
	    (char *)password) == 0)
		return 0;
	else
		return 1;
#else
	/* Check for users with no password. */
	if (strcmp(password, "") == 0 && strcmp(pw->pw_passwd, "") == 0)
		return 1;
	else {
		/* Encrypt the candidate password using the proper salt. */
		char *encrypted_password = crypt(password,
		    (pw->pw_passwd[0] && pw->pw_passwd[1]) ?
		    pw->pw_passwd : "xx");
		/*
		 * Authentication is accepted if the encrypted passwords
		 * are identical.
		 */
		return (strcmp(encrypted_password, pw->pw_passwd) == 0);
	}
#endif
#endif
}