/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)tftp.h	8.1 (Berkeley) 6/2/93
 */

#ifndef _ARPA_TFTP_H_
#define	_ARPA_TFTP_H_

/*
 * Trivial File Transfer Protocol (IEN-133)
 */

 
#define OPCODE_LEN      2           /* option code length */
#define BLKNUMBER_LEN   2           /* blk number length */
#define	SEGSIZE		512		/* data segment size */


#define TFTP_OP_BLKSIZE_STR         "blksize"
#define TFTP_OP_BLKSIZE_VALUE_LEN   6


/*
 * Packet types.
 */
enum PACKET_TYPE_E
{
	RRQ = 1,		// read request
	WRQ,			// write request
	DATA,			// data packet
	ACK,			// acknowledgement
    ERROR_TFTP,			// error code
    OACK 
};



struct tftphdr {
	unsigned short th_opcode;		/* packet type */
	union {
		unsigned short tu_block;	/* block # */
		unsigned short tu_code;		/* error code */
		char tu_stuff[1];			/* request packet stuff */
	} th_u;
	char th_data[1];				/* data or error string */
};

#define	th_block	th_u.tu_block
#define	th_code		th_u.tu_code
#define	th_stuff    th_u.tu_stuff
#define	th_msg		th_data

/*
 * Error codes.
 */
enum ERROR_E
{
	EUNDEF = 1,		// not defined
	ENOTFOUND,		// file not found
	EACCESS,		// access violation
	ENOSPACE,		// disk full or allocation exceeded
	EBADOP,			// illegal TFTP operation
	EBADID,			// unknown transfer ID
	EEXISTS,		// file already exists
	ENOUSER,		// no such user
	// custom error code, chengcw, 2001/3/30
	ETIMEOUT,		// send/receive timeout
	ESEND,			// send error
	ERECEIVE,		// receive error
	ESOCKETOPEN,	// socket open error
	ESOCKETBIND,	// socket bind error
	ECANCEL,        // user cancel download/upload
	E_BUF_SIZE_EXCEEDS  // buffer size not enough
};

#endif /* !_ARPA_TFTP_H_ */
