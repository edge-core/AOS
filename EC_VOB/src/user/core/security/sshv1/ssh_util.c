/* $Id: ssh_util.c,v 1.37.2.2 2001/02/11 05:12:25 tls Exp $ */

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


#include <string.h>
/*#include <stdio.h>*/
#include <stdlib.h>
/*#include <unistd.h>*/
#include <sys/types.h>
/*Isiah. 2002-05-27 
	Use phase2 socket, mask sys/socket.h and netinet/in.h,
	add "skt_vx.h" and "socket.h" */
/*#include <sys/socket.h>*/
#include "skt_vx.h"
#include "socket.h"
/*#include <netinet/in_systm.h>*/
/*#include <netinet/in.h>*/
/*#include <netinet/ip.h>*/
/*#include <netinet/tcp.h>*/

#include "rand.h"

#include "options.h"

#include "sshd.h"
/*#include "ssh_paths.h"*/
#include "ssh_util.h"

extern int sprintf( char *buffer, const char *format, ... );


/*
 * build_version: build an appropriate version string.
 *		caller must free the returned string.
 */
char *build_version() 
{
    char *c;
#define __VERSTRSIZE 30
	c = malloc(sizeof(char) * __VERSTRSIZE);
	sprintf(c, "SSH-%d.%d-%s\n",
				PROTO_MAJOR, PROTO_MINOR, SSHD_REV);
#undef __VERSTRSIZE
	return(c);
}

u_int32_t ssh_crc32(u_int8_t *buf, size_t len) 
{
    return(ssh_crc32_final(buf, len, 0));
}

u_int32_t ssh_crc32_initial(u_int8_t *buf, size_t len) 
{
    return(ssh_crc32_partial(buf, len, 0));
}

u_int32_t ssh_crc32_final(u_int8_t *buf, size_t len, u_int32_t crc) 
{
    return(ssh_crc32_partial(buf, len, crc));
}

u_int32_t ssh_crc32_partial(u_int8_t *buf, size_t len, u_int32_t crc) 
{
    u_int8_t *where;

    const u_int32_t table[] = {
	0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
	0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
	0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c, 
	0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c };

    for (where = buf ; where < buf + len; where++) 
    {
	    crc ^= *where;
	    crc = (crc >> 4) ^ table[crc & 0xf];
	    crc = (crc >> 4) ^ table[crc & 0xf];
    }
    return (crc);
}

#if 0 /*isiah.2002-10-22*/
void free_mpint(struct ssh_mpint *mpi) 
{
    if (mpi->data) 
    {
	    memset(mpi->data, 0, (mpi->bits + 7) / 8);
	    free(mpi->data);
    }
    memset(mpi, 0, sizeof(struct ssh_mpint));
}

/*
 * Determine the filename to use for the authorized_keys file.
 *
 * Caller must free the returned string.
 */
char *authorized_keys_file(struct sshd_context *context) 
{
}
#endif /* #if 0 */
