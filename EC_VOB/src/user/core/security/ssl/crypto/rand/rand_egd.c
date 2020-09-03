/* crypto/rand/rand_egd.c */
/* Written by Ulf Moeller for the OpenSSL project. */
/* ====================================================================
 * Copyright (c) 1998-2000 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

#include <openssl/rand.h>

/* Query the EGD <URL: http://www.lothar.com/tech/crypto/>.
 */

#if defined(WIN32) || defined(VMS) || defined(__VMS)
int RAND_egd(const char *path)
	{
	return(-1);
	}

int RAND_egd_bytes(const char *path,int bytes)
	{
	return(-1);
	}
#else
#include <opensslconf.h>
/*isiah.2002-07-11*/
/*#include OPENSSL_UNISTD*/
//#include <sys/types.h>
/* Isiah : 2001-12-21
	Use phase2 socket, mask sys/socket.h 
	add "skt_vx.h" and "socket.h" */
/*#include <sys/socket.h>*/
#include "linlux_platform.h"
#ifdef SSL_LINUX
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#else
#include    "skt_vx.h"
#include    "socket.h"
#endif /*SSL_LINUX*/

/* Isiah : define AF_UNIX,phase2 "#define PF_LOCAL AF_LOCAL",but not define AF_LOCAL */
#if 0 /*build for linux platform*/
#define	AF_LOCAL	1		/* local to host (pipes, portals) */
#define	AF_UNIX		AF_LOCAL	/* backward compatibility */
#endif

#ifndef NO_SYS_UN_H
/* Isiah. change sys/un.h to Tronado streams/un.h 
#include <sys/un.h> */
/*isiah.2003-0116*/
/*#include <streams/un.h>*/
struct	sockaddr_un {
	unsigned char	sun_len;		/* sockaddr len including null */
	unsigned char	sun_family;		/* AF_UNIX */
	char	sun_path[104];		/* path name (gag) */
};
#else
struct	sockaddr_un {
	short	sun_family;		/* AF_UNIX */
	char	sun_path[108];		/* path name (gag) */
};
#endif /* NO_SYS_UN_H */
#include <string.h>

/*isiah.2004-01-08*/
#include <e_os.h>
#ifndef offsetof
#  define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

int RAND_egd(const char *path)
	{
	int ret = -1;
	struct sockaddr_un addr;
	int len, num;
	int fd = -1;
	unsigned char buf[256];

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	if (strlen(path) > sizeof(addr.sun_path))
		return (-1);
	strcpy(addr.sun_path,path);
	len = offsetof(struct sockaddr_un, sun_path) + strlen(path);
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1) return (-1);
	if (connect(fd, (struct sockaddr *)&addr, len) == -1) goto err;
	buf[0] = 1;
	buf[1] = 255;
	write(fd, buf, 2);
	if (read(fd, buf, 1) != 1) goto err;
	if (buf[0] == 0) goto err;
	num = read(fd, buf, 255);
	if (num < 1) goto err;
	RAND_seed(buf, num);
	if (RAND_status() == 1)
		ret = num;
 err:
/*isiah.2004-01-08*/
#ifndef NO_FP_API
	if (fd != -1) close(fd);
#else
#ifdef SSL_LINUX
	if (fd != -1) close(fd);
#else
	if (fd != -1) s_close(fd);
#endif /*SSL_LINUX*/
#endif
	return(ret);
	}

int RAND_egd_bytes(const char *path,int bytes)
	{
	int ret = 0;
	struct sockaddr_un addr;
	int len, num;
	int fd = -1;
	unsigned char buf[255];

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	if (strlen(path) > sizeof(addr.sun_path))
		return (-1);
	strcpy(addr.sun_path,path);
	len = offsetof(struct sockaddr_un, sun_path) + strlen(path);
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1) return (-1);
	if (connect(fd, (struct sockaddr *)&addr, len) == -1) goto err;

	while(bytes > 0)
	    {
	    buf[0] = 1;
	    buf[1] = bytes < 255 ? bytes : 255;
	    write(fd, buf, 2);
	    if (read(fd, buf, 1) != 1)
		{
		ret=-1;
		goto err;
		}
	    if(buf[0] == 0)
		goto err;
	    num = read(fd, buf, buf[0]);
	    if (num < 1)
		{
		ret=-1;
		goto err;
		}
	    RAND_seed(buf, num);
	    if (RAND_status() != 1)
		{
		ret=-1;
		goto err;
		}
	    ret += num;
	    bytes-=num;
	    }
 err:
/*isiah.2004-01-08*/
#ifndef NO_FP_API
	if (fd != -1) close(fd);
#else
#ifdef SSL_LINUX
	if (fd != -1) close(fd);
#else
	if (fd != -1) s_close(fd);
#endif /*SSL_LINUX*/
#endif
	return(ret);
	}


#endif
