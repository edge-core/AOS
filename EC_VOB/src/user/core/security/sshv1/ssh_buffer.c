/* $Id: ssh_buffer.c,v 1.27.2.1 2000/08/25 09:32:04 tls Exp $ */

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
 * Various functions to deal with buffers.
 */
/*#include <assert.h>*/
/*#include <errno.h>*/
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
/* Isiah. 2002-05-27. replace sys/uio.h with net/uio.h,
   because uio.h in net directory in VxWorks */
//#include <sys/uio.h>
#include <net/uio.h>
/*#include <unistd.h>*/

#include "options.h"

#include "ssh_buffer.h"
/*#include "ssh_logging.h"*/

/*isiah. 2002-05-28 */
#include "ssh_types.h"
#include "skt_vx.h"
#include "socket.h"
#include "ssh_def.h"

extern int Accton_recv(int sid, byte *data, int len, int flags);

/* Buffer management functions: */

/*
 * Allocate a buffer.  (re)Initialize it.
 */
struct ssh_buf *buf_alloc(struct ssh_buf *buf, int size, int *ret_errno) 
{
    struct ssh_buf *tmpbuf;
    u_int8_t *ndata;

    if (size < 0) 
    {
/*	    SSH_DLOG(3, ("buf_alloc: bad size: %d\n", size));*/
	    *ret_errno = EINVAL;
	    return(NULL);
    }

    /* Allocate the actual structure, if necessary. */
    if (!buf)
	    buf = tmpbuf = calloc(1, sizeof(struct ssh_buf));
    if (!buf) 
    {
/*	    SSH_DLOG(2, ("buf_alloc: malloc of ssh_buf failed: %s\n", 
		    				strerror(errno)));*/
	    *ret_errno = ENOMEM;
	    return(NULL);
    }

    /* Fill in the fields of the buffer. */
/* XXX see if anything depends on this. */
/* XXX if not then we can relax some of the memset(..,0,...) calls */
    if (buf->data)
	    ndata = realloc(buf->data, size);
    else
	    ndata = malloc(size);

    if (ndata == NULL) 
    {
/*	    SSH_DLOG(2, ("buf_alloc: alloc of buffer failed: %s\n", 
		    				strerror(errno)));*/
	    if (tmpbuf);
	        free(tmpbuf);	/* if we allocated a ssh_buf struct above */
	    *ret_errno = ENOMEM;
	    return(NULL);
    }
    buf->data = ndata;
    buf->get_loc = buf->data;
    buf->dsize = size;
    buf->dlen = 0;

    *ret_errno = 0;
    return(buf);
}

/*
 * Grow a buffer to a specified size.
 */
int buf_grow(struct ssh_buf *buf, int size, int *ret_errno)
{
  u_int8_t *odata;

	*ret_errno = 0;
	if (size < buf->dsize)
		return(0);

	odata = buf->data;
	buf->data = realloc(buf->data, size);
	if (buf->data == NULL) {
/*		SSH_DLOG(2, ("buf_grow: realloc failed: %s\n", strerror(errno)));*/
		buf->data = odata;
		*ret_errno = ENOSPC;
		return(1);
	}

	buf->dsize = size;

	/* Readjust pointers if the start of data moved. */
	if (buf->data != odata) 
		buf->get_loc = buf->data + (buf->get_loc - odata);

	return(0);
}

/*
 * Make a certain amount of space available for use.
 *
 * Rounds up to next highest 1024 bytes.
 */
#if 0 /*isiah.1001-10-22*/
int buf_makeavail(struct ssh_buf *buf, int space)
{
	int newsize;
	int ret_errno;

	/* First try trimming used space */
	buf_trim(buf, -1);

	if (buf_avail(buf) < space)
	{
		/* Guess we have to try allocating more memory */

		newsize = buf->dlen + (space - buf_avail(buf));
		newsize = roundup(newsize, 1024);
		if (buf_grow(buf, newsize, &ret_errno) != 0)
		{
			return(1);
		}
	}

	return(0);
}
#endif /* #if 0 */
/*
 * Zero out space associated with a buffer and reset pointers.
 */
void buf_clear(struct ssh_buf *buf)
{
	if (!buf)
		return;
	if (buf->data)
		memset(buf->data, 0, buf->dsize);
	buf->get_loc = buf->data;
	buf->dlen = 0;
}

/*
 * Destroy a buffer and cleanup any space associated with it.
 *
 * Note: this _does_not_free_ the actual ssh_buf structure.
 */
void buf_cleanup(struct ssh_buf *buf) 
{
    buf_clear(buf);
    if (buf->data)
		free(buf->data);
    memset(buf, 0, sizeof(struct ssh_buf));
}

/*
 * Append the contents of one buffer to the other.
 * The source buffer is unmodified.
 *
 * The destination buffer is expanded as necessary.
 */
int buf_append(struct ssh_buf *dstbuf, struct ssh_buf *srcbuf) {
    if (buffer_expand(dstbuf, buf_alllen(dstbuf) + buf_alllen(srcbuf),
							"buf_append") != 0)
	return(1);
    memcpy(buf_endofdata(dstbuf), buf_alldata(srcbuf), buf_alllen(srcbuf));
    dstbuf->dlen += buf_alllen(srcbuf);
    return(0);
}

/*
 * Adjust the length of data in the buffer.
 *
 * Use this to fix the length of the data after
 * writing directly into the buffer.
 *
 * Note: this could cause get_loc > data+dlen, but that's ok.
 */
int buf_adjlen(struct ssh_buf *buf, int delta) {
    if ((buf->dlen + delta) < 0 || (buf->dlen + delta) > buf->dsize) {
/*	SSH_DLOG(3, ("buf_adjlen: len w/delta out of range: %d+%d:%d\n",
					buf->dlen, delta, buf->dsize));*/
/*	errno = EINVAL;
	return(1);*/
	return(EINVAL);
    }
    buf->dlen += delta;
    return(0);
}

/*
 * Trim bytes from the beginning of the buffer.
 * Copy any extra data to the beginning.
 *
 * If nbytes==-1 this functions trims any used up data.
 */
int buf_trim(struct ssh_buf *buf, int nbytes) {
	if (buf->dlen < nbytes) {
/*		SSH_DLOG(3, ("buf_trim: nbytes to trim > length of data.\n"));*/
/*		errno = EINVAL;*/
		return(1);
	}

	/* Auto trim used up data: */
	if (nbytes < 0)
		nbytes = buf->get_loc - buf->data;

	if (nbytes > 0)
	{
		memcpy(buf->data, buf->data + nbytes, buf->dlen - nbytes);
		buf->dlen -= nbytes;
	}
	if ((buf->get_loc - buf->data) < nbytes)
		buf->get_loc = buf->data;	/* sketchy... */
	else
		buf->get_loc -= nbytes;
	return(0);
}

/*
 * Don't get anything, just skip the specified number of bytes.
 */
int buf_get_skip(struct ssh_buf *buf, int nbytes) {
    if (!buf->data ||
	((buf->data + buf->dlen) < (buf->get_loc + nbytes))) {
/*	SSH_DLOG(3, ("buf_get_skip: out of data. %d\n", nbytes));*/
/*	errno = EAGAIN;*/
	return(1);
    }
    buf->get_loc += nbytes;
    return(0);
}

int buf_get_int32(struct ssh_buf *buf, u_int32_t *ret) {
  u_int32_t tmp;

    if (!buf->data ||
	((buf->data + buf->dlen) < (buf->get_loc + sizeof(u_int32_t)))) {
/*	SSH_DLOG(3, ("buf_get_int32: out of data.\n"));*/
/*	errno = EAGAIN;*/
	return(1);
    }

    memcpy(&tmp, buf->get_loc, sizeof(u_int32_t));
    *ret = ntohl(tmp);

    buf->get_loc += sizeof(u_int32_t);
    return(0);
}

int buf_get_int16(struct ssh_buf *buf, u_int16_t *ret) {
  u_int16_t tmp;

    if (!buf->data ||
	((buf->data + buf->dlen) < (buf->get_loc + sizeof(u_int16_t)))) {
/*	SSH_DLOG(3, ("buf_get_int16: out of data.\n"));*/
/*	errno = EAGAIN;*/
	return(1);
    }

    memcpy(&tmp, buf->get_loc, sizeof(u_int16_t));
    *ret = ntohs(tmp);

    buf->get_loc += sizeof(u_int16_t);
    return(0);
}

int buf_get_int8(struct ssh_buf *buf, u_int8_t *ret) {
    if (!buf->data ||
	((buf->data + buf->dlen) < (buf->get_loc + sizeof(u_int8_t)))) {
/*	SSH_DLOG(3, ("buf_get_int8: out of data.\n"));*/
/*	errno = EAGAIN;*/
	return(1);
    }
    *ret = *(u_int8_t *)(buf->get_loc);
    buf->get_loc += sizeof(u_int8_t);
    return(0);
}

/*
 * buf_get_nbytes: Get a number of bytes.
 *	val is allocated to more than nbytes to allow it to be
 *	null terminated later.
 */
int buf_get_nbytes(struct ssh_buf *buf, int nbytes, u_int8_t **val) {
    if ((buf->get_loc + nbytes) > (buf->data + buf->dlen)) {
	/* out of data. */
/*	SSH_DLOG(3, ("buf_get_nbytes: out of data.\n"));*/
	return(1);
    }

    /* XXX perhaps make this check *val == NULL ? */
    if ((*val = malloc(nbytes + 2)) == NULL) {
/*	SSH_ERROR("buf_get_nbytes: Unable to allocate mem: %s\n", 
		  strerror(errno));*/
	return(1);
    }
    memcpy(*val, (u_int8_t *)(buf->get_loc), nbytes);
    buf->get_loc += nbytes;
    return(0);
}

/*
 * Get an unknown number of bytes from the buffer. 
 * (i.e. all of them.)
 *
 * Note: caller must free *val.
 */
int buf_get_unk_bytes(struct ssh_buf *buf, u_int8_t **val, int *len) {
    if (buf->get_loc < (buf->data + buf->dlen)) {
	*len = ((buf->data + buf->dlen) - buf->get_loc);
	/* Length + 2, in case caller wants to null-terminate. */
	if ((*val = malloc(*len + 2)) == NULL) {
/*	    SSH_ERROR("buf_get_unk_bytes: Unable to allocate mem: %s\n",
							strerror(errno));*/
	    *len = 0;
	    return(1);
	}
	memcpy(*val, (u_int8_t *)(buf->get_loc), *len);
	buf->get_loc = buf->data + buf->dlen;
    } else {
	*len = 0;
	*val = NULL;
    }
    return(0);
}

/*
 * Get a ssh style binary string from the buffer.
 */
int buf_get_binstr(struct ssh_buf *buf, u_int8_t **vals, u_int32_t *len) {
    if (buf_get_int32(buf, len) != 0) {
/*	SSH_DLOG(3, ("buf_get_binstr: out of data.\n"));*/
/*	errno = EAGAIN;*/
	return(1);
    }
    if (buf_len(buf) < *len) {
/*	SSH_DLOG(3, ("buf_get_binstr: binstr len(%d) > buf_len(%d)",
						*len, buf_len(buf)));*/
/*	errno = EAGAIN;*/
	return(1);
    }
    if (!(*vals)) {
	/* len + 2 in case caller want to 0 terminate. */
	if (( (*vals) = malloc(*len + 2)) == NULL)
	    return(1);
    }
    memcpy(*vals, buf->get_loc, *len);
    buf->get_loc += *len;
    return(0);
}

/*
 * Get a zero terminated string from the buffer.
 */
int buf_get_asciiz(struct ssh_buf *buf, char **astring, int *len) {
  int slen;
    if (buf->get_loc > (buf->data + buf->dlen)) {
/*	SSH_DLOG(3, ("buf_get_asciiz: out of data.\n"));*/
/*	errno = EAGAIN;*/
	return(1);
    }
    if (memchr(buf->get_loc, 0, buf_len(buf)) == NULL)
	slen = buf_len(buf) + 1;
    else
	slen = strlen(buf->get_loc) + 1;
 
    if (!(*astring)) {
	if (((*astring) = malloc(slen * sizeof(char))) == NULL)
	    return(1);
    }

    strncpy(*astring, buf->get_loc, slen);
    if (len)
	*len = slen;
    buf->get_loc += slen;
    return(0);
}

/*
 * Get a newline terminated string from the buffer.
 *	Note: this ignores \0's within the line.
 */
int buf_get_line(struct ssh_buf *buf, char **astring, int *len) {
  int slen;
  u_int8_t *newline;
    if (buf->get_loc > (buf->data + buf->dlen)) {
/*	SSH_DLOG(3, ("buf_get_line: out of data.\n"));*/
/*	errno = EAGAIN;*/
	return(1);
    }

    if ((newline = memchr(buf->get_loc, '\n', buf_len(buf))) == NULL)
	slen = buf_len(buf) + 1;
    else
	slen = (newline - (buf->get_loc)) + 1;

    if (!(*astring)) {
	if (((*astring) = malloc(slen * sizeof(char))) == NULL)
	    return(1);
    }

    memcpy(*astring, buf->get_loc, slen - 1);
    (*astring)[slen] = '\0';
    if (len)
	*len = slen;
    buf->get_loc += slen;
    return(0);
}

int buf_get_bignum(struct ssh_buf *buf, ssh_BIGNUM **num) {
  u_int16_t bits, bytes;
    if (buf_get_int16(buf, &bits) != 0)
	return(1);
    bytes = (bits + 7) / 8;
    if ((bytes > buf_len(buf)) ||
	(bignum_bin2bn(num, buf->get_loc, (bits + 7) / 8) != 0) ) {
	buf->get_loc -= sizeof(u_int16_t);
	return(1);
    }
    buf->get_loc += bytes;
    return(0);
}

int buffer_expand(struct ssh_buf *buf, size_t size, char *funcname) {
  u_int8_t *newbuf;
  int newsize;
    while ((buf->dlen + size) > buf->dsize) {
	newsize = ((buf->dsize / 1024) + 1) * 1024;
	newbuf = realloc(buf->data, newsize);
	if (newbuf == NULL) {
/*	    SSH_ERROR("%s: realloc to %d failed: %s\n",
					funcname, newsize, strerror(errno));*/
	    return(1);
	}
	buf->data = newbuf;
	buf->dsize = newsize;
    }
    return(0);
}

/*
 * Add a single byte to the end of the buffer.
 */
int buf_put_byte(struct ssh_buf *buf, u_int8_t val) {
    if (buffer_expand(buf, sizeof(u_int8_t), "buf_put_byte") != 0)
	return(1);
    *(buf->data + buf->dlen) = val;
    buf->dlen += sizeof(u_int8_t);
    return(0);
}

/*
 * Put a number of bytes to the end of the buffer.
 */
int buf_put_nbytes(struct ssh_buf *buf, int nbytes, u_int8_t *vals) {
    if (buffer_expand(buf, nbytes * sizeof(u_int8_t), "buf_put_nbytes") != 0)
	return(1);
    memcpy(buf->data + buf->dlen, vals, nbytes * sizeof(u_int8_t));
    buf->dlen += nbytes * sizeof(u_int8_t);
    return(0);
}

/*
 * Put a 32-bit integer to the end of the buffer.
 */
int buf_put_int32(struct ssh_buf *buf, u_int32_t val) {
  u_int32_t tmp;

    if (buffer_expand(buf, sizeof(u_int32_t), "buf_put_int32") != 0)
	return(1);

    tmp = htonl(val);
    memcpy(buf->data + buf->dlen, &tmp, sizeof(u_int32_t));

    buf->dlen += sizeof(u_int32_t);
    return(0);
}

/*
 * Put a 16-bit integer to the end of the buffer.
 */
int buf_put_int16(struct ssh_buf *buf, u_int16_t val) {
  u_int16_t tmp;

    if (buffer_expand(buf, sizeof(u_int16_t), "buf_put_int16") != 0)
	return(1);

    tmp = htons(val);

    memcpy(buf->data + buf->dlen, &tmp, sizeof(u_int16_t));
    buf->dlen += sizeof(u_int16_t);

    return(0);
}

/*
 * Put a ssh style binary string to the end of the buffer.
 */
int buf_put_binstr(struct ssh_buf *buf, u_int8_t *vals, u_int32_t len) {
  u_int32_t tmp;

    if (buffer_expand(buf, (len + 4) * sizeof(u_int8_t), "buf_put_binstr") != 0)
	return(1);

    tmp = htonl(len);

    memcpy(buf->data + buf->dlen, &tmp, sizeof(u_int32_t));
    memcpy(buf->data + buf->dlen + sizeof(u_int32_t), vals, len);
    buf->dlen += len + sizeof(u_int32_t);
    return(0);
}

/*
 * Put a zero terminated ascii scring.
 */
int buf_put_asciiz(struct ssh_buf *buf, char *astring) {
  int slen;
    slen = strlen(astring) + 1;
    if (buffer_expand(buf, slen * sizeof(char), "buf_put_asciiz") != 0)
	return(1);
    strncpy(buf->data + buf->dlen, astring, slen);
    buf->dlen += slen;
    return(0);
}

/*
 * Put a mpint to the end of the buffer.
 */
int buf_put_mpint(struct ssh_buf *buf, struct ssh_mpint *val) {
  int len;
    len = (val->bits + 7) / 8;
    if (buf_put_int16(buf, val->bits) != 0)
	return(1);
    /* XXX if this fails, remove the int16? */
    return(buf_put_nbytes(buf, len, val->data));
}

/*
 * Put a BIGNUM.  Looks just like an mpint.
 */
int buf_put_bignum(struct ssh_buf *buf, ssh_BIGNUM *num) {
    if (buf_put_int16(buf, bignum_num_bits(num)) != 0)
	return(1);
    if (buffer_expand(buf, bignum_num_bytes(num), "buf_put_bignum") != 0) {
	/* XXX trim the int16? */
	return(1);
    }
    buf->dlen += bignum_bn2bin(num, buf->data + buf->dlen);
    return(0);
}

/*
 * Put a RSA public key with each component as a mpint to the end of the buffer.
 */
int buf_put_rsa_publickey(struct ssh_buf *buf, ssh_RSA *key) {
  struct ssh_mpint mpi;
    if (buf_put_int32(buf, bignum_num_bits(ssh_rsa_npart(key))) != 0) {
/*	SSH_DLOG(2, ("buf_put_rsa_publickey: unable to put num bits.\n"));*/
	return(1);
    }

    if (bignum_to_mpint(ssh_rsa_epart(key), &mpi) < 0) {
/*	SSH_DLOG(2, ("buf_put_rsa_publickey: unable to convert exponent to mpint\n"));*/
	return(1);
    }
    if (buf_put_mpint(buf, &mpi) != 0) {
/*	SSH_DLOG(2, ("buf_put_rsa_publickey: unable to put exponent.\n"));*/
	free(mpi.data); mpi.data = NULL;
	return(1);
    }
    free(mpi.data); mpi.data = NULL;

    if (bignum_to_mpint(ssh_rsa_npart(key), &mpi) < 0) {
/*	SSH_DLOG(2, ("buf_put_rsa_publickey: unable to convert modulus to mpint\n"));*/
	return(1);
    }
    if (buf_put_mpint(buf, &mpi) != 0) {
/*	SSH_DLOG(2, ("buf_put_rsa_publickey: unable to put modulus.\n"));*/
	free(mpi.data); mpi.data = NULL;
	return(1);
    }
    free(mpi.data); mpi.data = NULL;

    return(0);
}

/*
 * Reads from d into buf.  Returns total length of buf.
 */
int buf_readfile(struct ssh_buf *buf, int d, int size)
{
    int ret;
    
	if (!buf) 
	{
/*		errno = EINVAL;
		return(-1);*/
		return(EINVAL);
	}
	if (size < 0)
		size = buf_avail(buf);
	if (size > buf_avail(buf)) 
	{
		if (buf_grow(buf, size + buf_alllen(buf), &ret) != 0) 
		{
/*			SSH_ERROR("buf_readfile: buf_grow failed: %s\n", strerror(errno));
			return(-1);*/
			return(ret);
		}
	}

	if ((ret = read(d, buf_endofdata(buf), size)) < 0)
	{
	    /*isiah.2002-07-11*/
		if (ret == EWOULDBLOCK)
			ret = EAGAIN;
/*		if (ret != EAGAIN)
			SSH_ERROR("buf_readfile: read failed: %s\n", strerror(errno));
		return(-1);*/
		return(ret);
	}
	if (ret == 0)
		return(0);
	buf->dlen += ret;
	return(buf->dlen);
}

/*
 * Fills a buffer with data from d.
 * It will try to read as much data as there is room for.
 *
 * If that fails then it will try a blocking read to get at least
 * <size> bytes read.
 *
 * If either read it interrupted by a signal we return.
 *
 * Returns:
 *		-1 on error
 *		0 on EOF
 *		amount of data read
 */
#if 0 /*isiah.1001-10-22*/

int buf_fillbuf(struct ssh_buf *buf, int d, int size)
{
	int total_read;
	int ret;
	int tmp_err;
	int old_flag;
/*isiah.2002-05-28 */
    int on = 1, off = 0;

	/*assert(buf != NULL);*/
	if(!(buf != NULL))
	    return 0;

	if (size > buf_avail(buf)) 
    {
		if (buf_grow(buf, size + buf_alllen(buf), &ret) != 0) 
        {
			tmp_err = ret;
/*			SSH_ERROR("buf_readfile: buf_grow failed: %s\n", strerror(errno));*/
			ret = tmp_err;
/*			return(-1);*/
            return(ret);
		}
	}

	if ((ret = read(d, buf_endofdata(buf), buf_avail(buf))) < 0)
	{
		tmp_err = ret;
		if (ret == EWOULDBLOCK)
		{
/*			SSH_DLOG(4, ("read(#1) got interrupted\n"));*/
			ret = tmp_err;
/*			return(-1);*/
            return(ret);
		}
		if (ret != EAGAIN)
		{
/*			SSH_ERROR("read failed: %s\n", strerror(errno));*/
			ret = tmp_err;
/*			return(-1);*/
            return(ret);
		}

		/* errno == EAGAIN, no data yet, so try blocking read below */
		total_read = 0;

	} 
    else 
    {
		buf->dlen += ret;

		/* Check for EOF */
		if (ret == 0)
			return(0);

		total_read = ret;
	}

	if (size <= total_read)
	{
		return(total_read);
	}

	/* If we haven't read enough try a blocking read. */
/*isiah. 2002-05-28 */
/* repleaced fcntl() with s_ioctl() */
/*    old_flag = fcntl(d, F_GETFL);
	fcntl(d, F_SETFL, old_flag & ~O_NONBLOCK);*/
    s_ioctl(d,SIOCNBIO,&off,sizeof(int));

	while (size > total_read)
	{
		if ((ret = read(d, buf_endofdata(buf), size)) < 0)
		{
			tmp_err = ret;

			/* Restore previous blocking behaviour */
/*isiah. 2002-05-28 ???????*/
/* repleaced fcntl() with s_ioctl() */
/*			fcntl(d, F_SETFL, old_flag);*/
            //s_ioctl(d,SIOCNBIO,&on,sizeof(int));

			if (tmp_err == EWOULDBLOCK)
			{
/*				SSH_DLOG(4, ("read(#2) got interrupted\n"));*/
				ret = EWOULDBLOCK;

				if (total_read == 0)
					return(-1);		/* same as read() */

				return(total_read);
			}

			/* Note: on errors we might still have read something */
/*			SSH_ERROR("buf_readfile: read failed: %s\n", strerror(errno));*/
			ret = tmp_err;
/*			return(-1);*/
            return(ret);
		}

		total_read += ret;
		buf->dlen += ret;
	}

	/* Restore previous blocking behaviour */
/*isiah. 2002-05-28 ???????*/
/* repleaced fcntl() with s_ioctl() */
/*	fcntl(d, F_SETFL, old_flag);*/
    //s_ioctl(d,SIOCNBIO,&on,sizeof(int));
	
	return(total_read);
}

/*
 * Print out the contents of a buffer.
 */
void buf_print(struct ssh_buf *buf)
{
  int i;
  u_int8_t *f;
	if (buf_alllen(buf) == 0) {
#ifndef NO_FP_API
		fprintf(stderr, "(empty)\n");
#else
		printf("(empty)\n");
#endif
		return;
	}

	f = buf_alldata(buf);
	for (i = 0 ; i < buf_alllen(buf); i++)
#ifndef NO_FP_API
		fprintf(stderr, "%02x ", f[i]);
	fprintf(stderr, "\n");
#else
		printf("%02x ", f[i]);
	printf("\n");
#endif
}

void buf_printn(struct ssh_buf *buf, int nbytes)
{
  int olen;
	olen = buf->dlen;
	buf->dlen = (nbytes > buf->dlen) ? buf->dlen : nbytes;
	buf_print(buf);
	buf->dlen = olen;
}
#endif /* #if 0 */
