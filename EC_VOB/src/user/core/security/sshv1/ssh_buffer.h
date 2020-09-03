/* $Id: ssh_buffer.h,v 1.16.2.1 2000/08/25 09:32:05 tls Exp $ */

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


#ifndef _SSH_BUFFER_H
#define _SSH_BUFFER_H

#include "ssh_crypto.h"

/*
 * Various functions to deal with buffers.
 */

/*
 * data:	actual data buffer
 * dlen:	length of real data in memory
 * dsize:	allocate size of the buffer
 * get_loc:	location in the buffer that the *get* functions work from.
 *		*put* function always append to the end of the buffer.
 */
struct ssh_buf {
    u_int8_t	*data;
    int		dlen;
    int		dsize;
    u_int8_t	*get_loc;
};

/* All functions set errno to indicate the error. */

/* If buf==NULL, allocate buffer, init and return.  Else just init and return*/
struct ssh_buf *buf_alloc(struct ssh_buf *buf, int size, int *ret_errno);
int buf_grow(struct ssh_buf *buf, int size, int *ret_errno);
#if 0 /*isiah.1001-10-22*/
int buf_makeavail(struct ssh_buf *buf, int space);
#endif /* #if 0 */
void buf_clear(struct ssh_buf *buf);
void buf_cleanup(struct ssh_buf *buf);

int buf_append(struct ssh_buf *dstbuf, struct ssh_buf *srcbuf);
#if 0 /*isiah.1001-10-22*/
void buf_print(struct ssh_buf *buf);
void buf_printn(struct ssh_buf *buf, int nbytes);
#endif /* #if 0 */

/* Actual data in the buffer: */
#define buf_data(b)	((b)->get_loc)
#define buf_alldata(b)	((b)->data)
/* Size of the data in the buffer: */
#define buf_len(b)	((b)->dlen - ((b)->get_loc - (b)->data))
#define buf_alllen(b)	((b)->dlen)
/* Total size of the buffer. */
#define buf_size(b)	((b)->dsize)
/* Size available for more data. */
#define buf_avail(b)	(buf_size(b) - buf_alllen(b))
/* Rewind pointer: */
#define buf_rewind(b)	{ (b)->get_loc = (b)->data; }
#define buf_endofdata(b) (buf_alldata((b)) + buf_alllen((b)))

/* Adjust the size of the data in the buffer. */
/*	i.e. if you just wrote some data into the buffer using buf_data */
int buf_adjlen(struct ssh_buf *buf, int delta);

/* Remove bytes from the beginning of the buffer.  Copy rest of data over. */
int buf_trim(struct ssh_buf *buf, int nbytes);

int buf_get_skip(struct ssh_buf *buf, int nbytes);
int buf_get_int32(struct ssh_buf *buf, u_int32_t *ret);
int buf_get_int16(struct ssh_buf *buf, u_int16_t *ret);
int buf_get_int8(struct ssh_buf *buf, u_int8_t *ret);

int buf_get_nbytes(struct ssh_buf *buf, int nbytes, u_int8_t **val);
int buf_get_unk_bytes(struct ssh_buf *buf, u_int8_t **val, int *len);
int buf_get_binstr(struct ssh_buf *buf, u_int8_t **vals, u_int32_t *len);
int buf_get_asciiz(struct ssh_buf *buf, char **astring, int *len);
int buf_get_line(struct ssh_buf *buf, char **astring, int *len);
int buf_get_bignum(struct ssh_buf *buf, ssh_BIGNUM **num);

int buffer_expand(struct ssh_buf *buf, size_t size, char *funcname);

int buf_put_byte(struct ssh_buf *buf, u_int8_t val);
int buf_put_nbytes(struct ssh_buf *buf, int nbytes, u_int8_t *vals);
int buf_put_int32(struct ssh_buf *buf, u_int32_t val);
int buf_put_int16(struct ssh_buf *buf, u_int16_t val);
#define buf_put_int8(buf,val)	buf_put_byte(buf,val)
int buf_put_binstr(struct ssh_buf *buf, u_int8_t *vals, u_int32_t len);
int buf_put_asciiz(struct ssh_buf *buf, char *astring);
int buf_put_bignum(struct ssh_buf *buf, ssh_BIGNUM *num);
int buf_put_rsa_publickey(struct ssh_buf *buf, ssh_RSA *key);

/* Read from filedescriptor d into buf. if size==-1 read until EOF or EOB */
int buf_readfile(struct ssh_buf *buf, int d, int size);

#endif /* _SSH_BUFFER_H */
