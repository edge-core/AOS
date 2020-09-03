/* $Id: ssh_packet.h,v 1.19.2.1 2000/08/25 09:32:13 tls Exp $ */

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

#ifndef _SSH_PACKET_H
#define _SSH_PACKET_H

/*#include <stdio.h>*/
#include "rsa.h"

#include "ssh_datatypes.h"

struct sshd_context;

#define PKT_WAITALL		0x01	/* Wait for complete packet operation */


/* Macros: */
#define SEND_FAILURE(c)		xmit_data((c), SSH_SMSG_FAILURE, NULL, 0, PKT_WAITALL)
#define SEND_SUCCESS(c)		xmit_data((c), SSH_SMSG_SUCCESS, NULL, 0, PKT_WAITALL)
#define SEND_DEBUG(c,p)		xmit_data((c), SSH_MSG_DEBUG, p, strlen(p), PKT_WAITALL)
#define SEND_DEBUGX(c,p,l)	xmit_data((c), SSH_MSG_DEBUG, p, l, PKT_WAITALL)
#define SEND_DISCONNECT(c,p)	xmit_data((c), SSH_MSG_DISCONNECT, p, strlen(p), PKT_WAITALL)

#define SSH_MAX_PACKETSIZE	(256 * 1024)

/*
 * This structure is used to hold information about a packet in memory.
 * This is NOT the form a packet takes on the wire.
 */
struct ssh_packet {
	u_char p_type;		/* Packet type. */
	struct ssh_buf *p_clr; /* Decrypted data of packet. */
	struct ssh_buf *p_enc; /* Buffer holding the original packet. */
	int pktdone;
};

#define packet_type(pc)		((pc)->p_type)
#define packet_data(pc)		buf_data((pc)->p_clr)
#define packet_alldata(pc)	buf_alldata((pc)->p_clr)
#define packet_len(pc)		buf_len((pc)->p_clr)
#define packet_done(pc)		((pc)->pktdone)

void packet_init(struct ssh_packet *);

int ssh_read_packet(struct sshd_context *, struct ssh_packet *, int);
int process_packet(struct sshd_context *, struct ssh_packet *);
int xmit_int32(struct sshd_context *, u_int8_t, u_int32_t, int);
int xmit_data(struct sshd_context *, u_int8_t, u_int8_t *, size_t, int);
int xmit_packet(struct sshd_context *, unsigned char, struct ssh_buf *, int);

/* Packet processing functions. */

/* packet reading: */
#if 0 /*isiah.2002-10-22*/
int packet_init_get(struct ssh_packet *);
#endif /* #if 0 */

/* packet_get_byte: get a single byte. */
#define packet_get_byte(pc,val)		buf_get_int8((pc)->p_clr, (val))
/* packet_get_int32: get a 32-bit integer in host order. */
#define packet_get_int32(pc,val)	buf_get_int32((pc)->p_clr, (val))
/* packet_get_nbytes: get a specified number of bytes. */
/* 	Note: caller must free *val. */
#define packet_get_nbytes(pc,nbytes,val)	\
			buf_get_nbytes((pc)->p_clr, (nbytes), (val))
#define packet_get_unk_bytes(pc,val,len)	\
			buf_get_unk_bytes((pc)->p_clr, (val), (len))
int packet_get_binstr(struct ssh_packet *, u_int8_t **, size_t *);
int packet_get_mpint(struct ssh_packet *, struct ssh_mpint *);
#define packet_get_bignum(pc, num) buf_get_bignum((pc)->p_clr, num)


/* packet building: */
#if 0 /*isiah.2002-10-22*/
int packet_init_put(struct ssh_packet *, int);
#endif /* #if 0 */
#define packet_put_byte(pc,val)		buf_put_byte((pc)->p_clr, val)
#define packet_put_nbytes(pc,nb,vals)	buf_put_nbytes((pc)->p_clr, nb, vals)
#define packet_put_int32(pc,val)	buf_put_int32((pc)->p_clr, val)
#define packet_put_int16(pc,val)	buf_put_int16((pc)->p_clr, val)
#define packet_put_binstr(pc,vals,len)	buf_put_binstr((pc)->p_clr, vals, len)
#define packet_put_mpint(pc,mpi)	buf_put_mpint((pc)->p_clr, mpi)
#define packet_put_rsa_publickey(pc,r)	buf_put_rsa_publickey((pc)->p_clr, r)

int packet_cleanup(struct ssh_packet *);

#endif /* _SSH_PACKET_H */
