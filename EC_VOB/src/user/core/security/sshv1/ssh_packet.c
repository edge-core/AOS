/* $Id: ssh_packet.c,v 1.45.2.3 2000/08/27 21:53:42 erh Exp $ */

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
#include <stdlib.h>
#include <string.h>
/*#include <stdio.h>*/
#include <sys/types.h>
/* Isiah.2002-05-27. replace sys/uio.h with net/uio.h,
   because uio.h in net directory in VxWorks */
//#include <sys/uio.h>
#include <net/uio.h>
/*#include <unistd.h>*/

#include "rsa.h"

#include "options.h"

#include "sshd.h"
#include "ssh_buffer.h"
#include "ssh_cipher.h"
#include "ssh_crypto.h"
#include "ssh_datatypes.h"
#include "ssh_messages.h"
#include "ssh_packet.h"
#include "ssh_rsakeys.h"
#include "ssh_util.h"

extern int Accton_s_ioctl(int sid, int cmd, void *data,int dlen);
extern int Accton_send(int sid, byte *data,int len,int flags);

/*Isiah. 2002-05-28 */
#include "ssh_def.h"

void packet_init(struct ssh_packet *pc) {
    memset(pc, 0, sizeof(struct ssh_packet));
    pc->pktdone = 1;
    return;
}

/*
 * Reads from the socket into in_pc.p_enc.
 * Returns:	-1 on error
 *		>0 Enough data for process_packet
 *			Ensures len isn't too big also.
 *			Returns amount of data in the read buffer.
 *		0 Not enought data yet.
 *
 *	If flag is set to PKT_WAITALL, "enough data" is defined as an
 *	entire packet.
 */
int ssh_read_packet(struct sshd_context *context, struct ssh_packet *pc, int flag) 
{
    int r_len, pad_len;
    u_int32_t  dlen;
/*    int flg;*/
/*Isiah.2002-05-28 */
	int on = 1, off = 0;
	int ret_errno;

    /* First time through: initialize stuff. */
    if (pc->p_enc == NULL)
	pc->p_enc = buf_alloc(NULL, 8192, &ret_errno);

    /* Make sure we start out non-blocking. */
/*Isiah.2002-05-28 */
/*replaced fcntl() with s_ioctl()*/
/*	flg = fcntl(context->s, F_GETFL);
    fcntl(context->s, F_SETFL, flg | O_NONBLOCK);*/
	s_ioctl(context->s,SIOCNBIO,&on,sizeof(int));

readdata:
    /* Read (more) data. */
    r_len = buf_readfile(pc->p_enc, context->s, -1);
/*Isiah.2002-07-19*/
	if (r_len < 0 && r_len != EAGAIN)
/*	    return(-1);*/
        return(r_len);
    if (r_len == 0) 
    {
/*	    SSH_DLOG(4, ("read_packet: EOF\n"));*/
/*    	errno = EPIPE;
	    return(-1);*/
	    return(EPIPE);
    }

/*    SSH_DLOG(4, ("read_packet: Read data length: %d\n", r_len));*/

    /* Not enough to figure out the data length yet? */
    if (buf_alllen(pc->p_enc) < 4) 
    {
/*	    SSH_DLOG(4, ("read_packet: Ultra-short packet.  Saving.\n"));*/
	    if (flag & PKT_WAITALL) 
        {
/*Isiah.2002-05-28 */
/*replaced fcntl() with s_ioctl()*/
/*	        flg = fcntl(context->s, F_GETFL, &flg);
	        fcntl(context->s, F_SETFL, flg & ~O_NONBLOCK);*/
        	s_ioctl(context->s,SIOCNBIO,&off,sizeof(int));
	        goto readdata;		/* try reading more data. */
	    }
	    return(0);
    }

    /* Grab data-length-w/o-padding field from the packet. */
    /* Note: this includes packet type, data, and CRC. */
    buf_get_int32(pc->p_enc, &dlen);
    buf_rewind(pc->p_enc);

    /* Is the length too big? */
    if (dlen > (256 * 1024)) 
    {
/*	    SSH_DLOG(1, ("Packet length too big: %d\n", dlen));*/
/*	    errno = E2BIG;
	    return(-1);*/
	    return(E2BIG);
    }

    pad_len = 8 - (dlen % 8);

    /* Allocate more space for the input buffer, if needed. */
    if (buf_grow(pc->p_enc, 4 + pad_len + dlen, &ret_errno) != 0) 
    {
        int tmp_err = ret_errno;
/*	    SSH_ERROR("Unable to grow buffer to size %d: %s\n", dlen, 
		    				strerror(errno));*/
	    ret_errno = tmp_err;
/*	    return(-1);*/
        return(ret_errno);
    }

    /* Not enough data for the given length yet? */
    if (buf_alllen(pc->p_enc) < (4 + pad_len + dlen)) 
    {
	    if (flag & PKT_WAITALL) 
        {
/*Isiah.2002-05-28 */
/*replaced fcntl() with s_ioctl()*/
/*	        flg = fcntl(context->s, F_GETFL);
	        fcntl(context->s, F_SETFL, flg & ~O_NONBLOCK);*/
        	s_ioctl(context->s,SIOCNBIO,&off,sizeof(int));
	        goto readdata;		/* try reading more data. */
	    }
	    return(0);
    }

    if (flag & PKT_WAITALL) 
    {
/*Isiah.2002-05-28 */
/*replaced fcntl() with s_ioctl()*/
/*	    flg = fcntl(context->s, F_GETFL);
	    fcntl(context->s, F_SETFL, flg | O_NONBLOCK);*/
       	s_ioctl(context->s,SIOCNBIO,&on,sizeof(int));
    }
    return(buf_alllen(pc->p_enc));
}


/*
 * Decode the next packet from the p_enc buffer into the p_clr buffer.
 *
 * Decrypts, checksums and decompresses into p_clr.
 * DEBUG and IGNORE packets are discarded.
 *
 * Returns:	-1 on error
 *		0 if no packet.  (Not enough data)
 *		>0 if a packet is ready.  (length of packet)
 */
int process_packet(struct sshd_context *context, struct ssh_packet *pc) {
  int pad_len;
  u_int32_t dlen, crc;
  struct ssh_buf *tmp_clr;
  int ret_errno;

    buf_clear(pc->p_clr);
    pc->pktdone = 0;
    pc->p_type = SSH_MSG_NONE;

    if (buf_alllen(pc->p_enc) < 4) {
/*	SSH_DLOG(4, ("process_packet: Ultra-short packet.  Saving.\n"));*/
	return(0);
    }

    /* Grab data-length-w/o-padding field from the packet. */
    /* Note: this includes packet type, data, and CRC. */
    buf_get_int32(pc->p_enc, &dlen);

    pad_len = 8 - (dlen % 8);

    /* Not enough data for the given length yet? */
    if (buf_alllen(pc->p_enc) < (4 + pad_len + dlen)) {
	/* Be sure to keep the length field for the next call: */
	buf_rewind(pc->p_enc);
	return(0);
    }

    /* -- Now we know that there's enough data for an entire packet. -- */

/*Isiah. 2002-06-04 */
#if 0
    if (is_debug_level(6)) {
/*	SSH_DLOG(5, ("Encrypted packet:\n"));*/
	buf_printn(pc->p_enc, 4 + pad_len + dlen);
    }
#endif /*isiah. end of #if 0 */

    /* Allocate space for decryption. */
    /* If compressing decrypt into an extra buffer. */
    if (context->compressing)
	tmp_clr = buf_alloc(NULL, dlen + pad_len, &ret_errno);
    else
	tmp_clr = pc->p_clr = buf_alloc(pc->p_clr, pad_len + dlen, &ret_errno);
    if (tmp_clr == NULL) {
/*	SSH_ERROR("process_packet: buf_alloc for decryption failed: %s\n",
							strerror(errno));
	return(-1);*/
    return(ret_errno);
    }

    /* We feed each encrypted packet to a function which detects Ariel 
       Futoransky's CRC compensation attack. */

    if (context->cipher->type != SSH_CIPHER_NONE) {
	if(detect_attack(buf_data(pc->p_enc), dlen + pad_len, NULL) != 0) {
	    SEND_DISCONNECT(context,
			    "CRC compensation attack detected, therefore connection closed.");
	    return(-1);
	}
    }

    /* Decrypt from the input buffer into the decrypted packet buffer. */
    if (context->cipher->decrypt)
	context->cipher->decrypt(buf_data(pc->p_enc), buf_data(tmp_clr),
			dlen + pad_len, context->cipher->key_data);
    else
	memcpy(buf_data(tmp_clr), buf_data(pc->p_enc), dlen+pad_len);

    /* Fix the length of the decrypted buffer. */
    if ( (ret_errno = buf_adjlen(tmp_clr, dlen + pad_len)) != 0) {
/*	SSH_ERROR("process_packet: unable to adj decrypted buf len: %s\n",
							strerror(errno));*/
	buf_cleanup(tmp_clr);
	free(tmp_clr);
	if (tmp_clr != pc->p_clr) {
	    buf_cleanup(pc->p_clr);
	    free(pc->p_clr);
	}
	tmp_clr = pc->p_clr = NULL;
/*	return(-1);*/
	return(ret_errno);
    }

/*Isiah. 2002-06-04 */
#if 0
    if (is_debug_level(6)) {
	fprintf(stderr, "Decrypted packet:\n");
	buf_print(tmp_clr);
    }
#endif /*isiah. end of #if 0 */

    /* Remove this packet from the input buffer. */
    buf_get_skip(pc->p_enc, dlen + pad_len);	/* Already got the len */
    /* Trim the already used, still encrypted packet. */
    buf_trim(pc->p_enc, -1);

    /* Calculate crc.  dlen is length of type, data and crc: */
    crc = ssh_crc32(buf_data(tmp_clr), dlen - 4 + pad_len);
    if (crc != ntohl(*(u_int32_t *)(buf_data(tmp_clr) + pad_len + dlen - 4))) {
/*	SSH_DLOG(2, ("Bad CRC. %d\n", crc));*/
	return(-1);
    }
    buf_adjlen(tmp_clr, -4);		/* cut CRC */

    /* Skip the padding. */
/*    SSH_DLOG(5, ("padlen:%d\n", pad_len));*/
    buf_get_skip(tmp_clr, pad_len);

    /* If compressing, decompress into p_clr. */
/*Isiah. 2002-05-27 
  Don't support compress now */
#if 0
    if (context->compressing) 
    {
        int ret;
	    /* Allocate space for the decrypted data. */
	    if ((pc->p_clr = buf_alloc(pc->p_clr, SSH_MAX_PACKETSIZE, &ret_errno)) == NULL) 
        {
/*	        SSH_ERROR("process_packet: buf_alloc for inflate failed: %s\n",
			    				strerror(errno));
	        return(-1);*/
	        return(ret_errno);
	    }
	    context->inz.next_in = buf_data(tmp_clr);/* After padding. */
	    context->inz.avail_in = dlen - 4;	/* Only type and data, but NOT! CRC */
	    context->inz.next_out = buf_data(pc->p_clr);
	    context->inz.avail_out = buf_size(pc->p_clr);

    	do 
        {
	        ret = inflate(&(context->inz), Z_PARTIAL_FLUSH);
	        if (ret == Z_OK)
	        	continue;
	        if (ret != Z_BUF_ERROR) 
            {
/*	            SSH_DLOG(1, ("process_packet: inflate failed: %d\n", ret));*/
        		return(-1);
	        }
	        if (context->inz.avail_out == 0) 
            {
/*        		SSH_DLOG(1, 
		        	("process_packet: inflate out of output space.\n"));*/
        		return(-1);
	        }
    	} while (ret != Z_BUF_ERROR) ;
	    if (context->inz.avail_in != 0) 
        {
/*	        SSH_DLOG(1, ("process_packet: incomplete inflation: %d\n", 
			    		context->inz.avail_in));*/
    	    return(-1);
	    }

	    /* Fix the length of the buffer. */
	    buf_adjlen(pc->p_clr, context->inz.next_out - buf_data(pc->p_clr));

	    /* Cleanup compressed cleartext buffer. */
	    buf_cleanup(tmp_clr);
    }
#endif 
/* Isiah.2002-05-27 .
   end of #if 0 */
    /* -- p_clr should now contain the packet type and data. */
    /* -- it may also contain padding before this, */
    /* -- but w/ get_loc set correctly */

    /* Grab packet type and trim it (and the padding) from the front. */
    buf_get_int8(pc->p_clr, &(pc->p_type));
    buf_trim(pc->p_clr, -1);		/* Cut off the padding and pkt type */

/*    SSH_DLOG(3, ("received packet type: %d\n", pc->p_type));*/

    /* Discard any debug or ignore packets. */
    if (pc->p_type == SSH_MSG_DEBUG || pc->p_type == SSH_MSG_IGNORE) {
	if (pc->p_type == SSH_MSG_DEBUG) {
/*	    SSH_DLOG(5, ("process_packet: DEBUG packet received.\n"));
	    SSH_DLOG(3, ("%s\n", buf_data(pc->p_clr)));*/
	}
	pc->pktdone = 0;
	buf_clear(pc->p_clr);
	return(0);		/* return no-packet. */
    }

    /* All done */
    pc->pktdone = 1;
    return(buf_alllen(pc->p_clr) + 1);	/* +1 for zero-data packets. */
}

/*
 * Packet layout:
 *	32-bit packet length, not including padding and length.
 *	1-8 bytes of padding.
 *	8-bit packet type.
 *	data.
 *	32-bit crc.
 */
/*
 * Turns raw data into a ssh_buf and sends a packet.
 * This function does not return until the packet has finished sending.
 */
int xmit_int32(struct sshd_context *context, u_int8_t ptype, u_int32_t val,
							int flag) {
  struct ssh_buf buf;
  int ret;

    memset(&buf, 0, sizeof(struct ssh_buf));

    /* Turn the raw data into a ssh_buf. */
    if (buf_alloc(&buf, sizeof(u_int32_t), &ret) == NULL) {
/*	SSH_ERROR("xmit_int32: Unable to buf_alloc: %s\n", strerror(errno));
	return(-1);*/
	return(ret);
    }
    if (buf_put_int32(&buf, val) != 0) {
/*	SSH_ERROR("xmit_int32: Unable to bufferize data: %s\n", 
						strerror(errno));*/
	buf_cleanup(&buf);
	return(-1);
    }

    /* Send it. */
    ret = xmit_packet(context, ptype, &buf, PKT_WAITALL|flag);
    buf_cleanup(&buf);
    return(ret);
}

int xmit_data(struct sshd_context *context,
		u_int8_t ptype, u_int8_t *rawbuf, size_t len, int flag) {
  struct ssh_buf buf;
  int ret;

    memset(&buf, 0, sizeof(struct ssh_buf));

    /* Turn the raw data into a ssh_buf. */
    if (buf_alloc(&buf, len, &ret) == NULL) {
/*	SSH_ERROR("xmit_data: Unable to buf_alloc(%d): %s\n", len, 
						strerror(errno));
	return(-1);*/
	return(ret);
    }
    if (rawbuf != NULL) {
	if (buf_put_binstr(&buf, rawbuf, len) != 0) {
/*	    SSH_ERROR("xmit_data: Unable to bufferize data: %s\n", 
						strerror(errno));*/
	    buf_cleanup(&buf);
	    return(-1);
	}
    }

    /* Send it. */
    ret = xmit_packet(context, ptype, &buf, PKT_WAITALL|flag);
    buf_cleanup(&buf);
    return(ret);
}


/*
 * Takes a buffer, turns it into a packet and starts xmitting it.
 * The packet in transit is stored in the context.  If packet_done(context->pc)
 * is 0 then this must not be called with a non-null buf.  If it is 0
 * then calling with a NULL buf will send more of it.
 *
 * buf is returned unchanged.
 */

int xmit_packet(struct sshd_context *context,
			u_int8_t ptype, struct ssh_buf *buf, int flag) {
  int ret;
  struct ssh_buf *newb, *cb;
  int padlen;/*, size;*/
  u_int32_t crc, len;

    if (buf)
/*	SSH_DLOG(4, ("xmit_packet: type=%d len=%d\n", ptype, buf_alllen(buf)));*/
        ;

    if (!packet_done(&(context->out_pc)) && buf) {
/*	SSH_ERROR("xmit_packet: more data+packet not done yet.\n");*/
	return(-1);
    }

    /* If packet is partially sent, continue it. */
    if (!packet_done(&(context->out_pc))) {
/*	SSH_DLOG(4, ("xmit_packet: continuing previous xmit.\n"));*/
	goto doxmit;
    }

    if (context->compressing) 
    {
/*Isiah. 2002-05-27 
  Don't support compress now */
#if 0
	    if (is_debug_level(6)) 
        {
/*	        SSH_DLOG(6, ("xmit_packet/uncompressed: "));*/
	        buf_print(buf);		/* rest of data. */
	    }
	    /* Set up the output for the compression. */
	    size = (buf_alllen(buf) + sizeof(u_int8_t)) * 1.001 + 12;
	    if ((cb = buf_alloc(NULL, size, &ret)) == NULL) 
        {
/*	        SSH_ERROR("xmit_packet: buf_alloc for deflate failed: %s\n",
			    				strerror(errno));
	        return(-1);*/
	        return(ret);
	    }
	    context->outz.next_out = buf_data(cb);
	    context->outz.avail_out = size;

	    /* Toss in the packet type byte. */
	    context->outz.next_in = &ptype;
	    context->outz.avail_in = sizeof(u_int8_t);
	    if ((ret = deflate(&(context->outz), Z_NO_FLUSH)) != Z_OK) 
        {
/*	        SSH_ERROR("xmit_packet: deflate #1 failed: %d\n", ret);*/
	        buf_cleanup(cb);
	        return(-1);
	    }

	    /* and the data. */
	    context->outz.next_in = buf_alldata(buf);
	    context->outz.avail_in = buf_alllen(buf);
	    if ((ret = deflate(&(context->outz), Z_PARTIAL_FLUSH)) != Z_OK) 
        {
/*	        SSH_ERROR("xmit_packet: deflate #2 failed: %d\n", ret);*/
    	    buf_cleanup(cb);
	        return(-1);
	    }

	    /* Set the length of compressed data. */
	    buf_adjlen(cb, size - context->outz.avail_out);
	    len = buf_alllen(cb) + 4;	/* data + type in the buffer + crc. */
#endif
/* Isiah.2002-05-27 .
   end of #if 0 */
    } 
    else 
    {
	    len = buf_alllen(buf) + 1 + 4;  /* data in buf + type + crc */
    }

    if (len > context->max_packet_size) {
/*	SSH_ERROR("xmit_packet: packet too big:%d\n", len);*/
	if (cb)
	    buf_cleanup(cb);
	return(-1);
    }

    padlen = 8 - (len % 8);

    /* Initialize buffer for un-encrypted packet. */
    if ((newb = buf_alloc(context->out_pc.p_clr, padlen + len, &ret)) == NULL) {
/*	SSH_ERROR("xmit_packet: unable to buf_alloc p_clr: %s\n", 
						strerror(errno));*/
	if (cb)
	    buf_cleanup(cb);
/*	return(-1);*/
    return(ret);
    }
    context->out_pc.p_clr = newb;

    /* Initialize buffer for encrypted data. */
    if ((newb = buf_alloc(context->out_pc.p_enc, 4 + padlen + len, &ret)) == NULL) {
/*	SSH_ERROR("xmit_packet: unable to buf_alloc p_enc: %s\n", 
						strerror(errno));*/
	if (cb)
	    buf_cleanup(cb);
	buf_clear(context->out_pc.p_clr);
/*	return(-1);*/
    return(ret);
    }
    context->out_pc.p_enc = newb;

    /* Put the length in the final packet (unencrypted). */
    buf_put_int32(context->out_pc.p_enc, len);

    /* Generate the padding. */
/*    SSH_DLOG(*/ /*5*/ /*4, ("generating padding:%d\n", padlen));*/
    if (context->cipher->type == SSH_CIPHER_NONE)
	memset(buf_data(context->out_pc.p_clr), 0, padlen);
    else
	ssh_rand_bytes(padlen, buf_data(context->out_pc.p_clr));
    buf_adjlen(context->out_pc.p_clr, padlen);

    /* Append payload data. */
    if (!context->compressing) {	/* Packet type, if not compressed in */
	buf_put_byte(context->out_pc.p_clr, ptype);
	buf_append(context->out_pc.p_clr, buf);
    } else {
	buf_append(context->out_pc.p_clr, cb);
	buf_cleanup(cb);
    }
    
    /* Calculate CRC and append to unencrypted buffer. */
    crc = ssh_crc32(buf_alldata(context->out_pc.p_clr),
			buf_alllen(context->out_pc.p_clr));
/*    SSH_DLOG(*/ /*5*/ /*4, ("xmit_packet: crc=%x\n", crc));*/
    buf_put_int32(context->out_pc.p_clr, crc);

/*Isiah. 2002-06-04 */
#if 0
    if (is_debug_level(6)) {
/*	SSH_DLOG(6, ("xmit_packet/clear: "));*/
	buf_print(context->out_pc.p_enc);	/* length */
	buf_print(context->out_pc.p_clr);	/* rest of data. */
    }
#endif /*isiah. end of #if 0 */

    /* Encrypt the payload into the final packet. */
    if (context->cipher->encrypt)
	context->cipher->encrypt(buf_alldata(context->out_pc.p_clr),
			buf_endofdata(context->out_pc.p_enc),
			buf_alllen(context->out_pc.p_clr),
			context->cipher->key_data);
    else
	memcpy(buf_endofdata(context->out_pc.p_enc),
			buf_alldata(context->out_pc.p_clr),
			buf_alllen(context->out_pc.p_clr));
    buf_adjlen(context->out_pc.p_enc, buf_alllen(context->out_pc.p_clr));


    /* Clear the unencrypted buffer. */
    buf_clear(context->out_pc.p_clr);

/*Isiah. 2002-06-04 */
#if 0
    if (is_debug_level(6)) {
/*	SSH_DLOG(6, ("xmit_packet/enc: "));*/
	buf_print(context->out_pc.p_enc);
    }
#endif /*isiah. end of #if 0 */

doxmit:
    /* Set the socket to blocking if the caller expects the */
    /* xmit to fully complete before we return. */
    if (flag & PKT_WAITALL) {
/*      int flg;*/
/*Isiah. 2002-05-28 */
        int off = 0;
/* replaced fcntl() with s_ioctl() */
/*	flg = fcntl(context->s, F_GETFL);
	fcntl(context->s, F_SETFL, flg & ~O_NONBLOCK);*/
    s_ioctl(context->s,SIOCNBIO,&off, sizeof(int));
    }
    
    ret = write(context->s, buf_data(context->out_pc.p_enc),
					buf_len(context->out_pc.p_enc));

/*    SSH_DLOG(4, ("xmit_packet: wrote %d bytes.\n", ret));*/

    if (ret < 0) {
	if (ret != EPIPE)
		;
/*	    SSH_ERROR("xmit_packet: write failed: %s\n", strerror(ret));*/
    } else {
	/* (current count)+(already used) == (total len) */
	if ( (ret + (buf_data(context->out_pc.p_enc) -
		     buf_alldata(context->out_pc.p_enc) ) )
		== buf_alllen(context->out_pc.p_enc)) {
/*	    SSH_DLOG(4, ("xmit_packet: done.\n"));*/
	    context->out_pc.pktdone = 1;
	    ret = 0;
	} else {
/*	    SSH_DLOG(4, ("xmit_packet: skipping %d\n", ret));*/
	    buf_get_skip(context->out_pc.p_enc, ret);
	    context->out_pc.pktdone = 0;
	    if (ret == 0)
		ret = 1;	/* XXX fixup. */
	}
	
    }

    /* Make the socket non-blocking again, if we changed it. */
    if (flag & PKT_WAITALL) {
/*      int flg;*/
/* isiah.2002-05-28 */
/*        int on = 1;*/
/* replaced fcntl() with s_ioctl() */
/*	flg = fcntl(context->s, F_GETFL);
	fcntl(context->s, F_SETFL, flg & O_NONBLOCK);*/
    //s_ioctl(context->s,SIOCNBIO,&on,sizeof(int));
    }
    return(ret);
}


/* --------------------------------------------------------- */

#if 0 /*isiah.2002-10-22*/
int packet_init_get(struct ssh_packet *pc) {
    if ((pc->p_clr == NULL) || (pc->pktdone == 0)) {
/*	SSH_ERROR("Attempted packet_init_get without a packet.\n");*/
	return(1);
    }
    buf_rewind(pc->p_clr);
    return(0);
}
#endif /* #if 0 */

/*
 * packet_get_binstr: get a "binary" string.  Referred to as a "string" in
 *			the SSH RFC.
 *
 * Note: caller must free *val.
 */
int packet_get_binstr(struct ssh_packet *pc, u_int8_t **val, size_t *len) {
  u_int32_t dlen;
    if (buf_get_int32(pc->p_clr, &dlen) != 0) {
/*	SSH_ERROR("packet_get_binstr: get_int32 failed.\n");*/
	return(1);
    }
    if (buf_get_nbytes(pc->p_clr, dlen, val) != 0) {
/*	SSH_ERROR("packet_get_binstr: get_nbytes failed.\n");*/
	return(1);
    }
    (*val)[dlen] = '\0';	/* Null terminate */
				/* get_nbytes allocs space for this */
    if (len)
	*len = dlen;
    return(0);
}

/*
 * packet_get_mpint: Get a multi-precision int.
 *
 * Note: caller must free val->data.
 */
int packet_get_mpint(struct ssh_packet *pc, struct ssh_mpint *val) {
  u_int16_t nbits;
  size_t dlen;

    if (buf_get_int16(pc->p_clr, &nbits) != 0) {
/*	SSH_ERROR("packet_get_mpint: Unable to get nbits\n");*/
	return(1);
    }

    dlen = (nbits + 7) / 8;

    if (buf_get_nbytes(pc->p_clr, dlen, &(val->data)) != 0) {
/*	SSH_ERROR("packet_get_mpint: get_nbytes failed.\n");*/
	return(1);
    }
    val->bits = nbits;

    return(0);
}

/* --------------------------- *
 * Routines to build a packet: *
 * --------------------------- */

#if 0 /*isiah.2002-10-22*/
/*
 * packet_init_put: initialize ptrs and sizes.  Allocate a buffer if needed.
 */
int packet_init_put(struct ssh_packet *pc, int size) {
    int ret_errno;
    
    if (size < 1024)
	size = 1024;
    if (size > SSH_MAX_PACKETSIZE)
	size = SSH_MAX_PACKETSIZE;
    if (pc->p_clr == NULL) {
	if ((pc->p_clr = buf_alloc(NULL, size, &ret_errno)) == NULL) {
/*	    SSH_ERROR("packet_init_put: buf_alloc failed: %s\n", 
						strerror(errno));*/
	    return(1);
	}
    }
    buf_clear(pc->p_clr);
    if (buf_grow(pc->p_clr, size, &ret_errno) != 0) {
/*	SSH_ERROR("packet_init_out: buffer_grow failed: %s\n", 
						strerror(errno));*/
	return(1);
    }
    return(0);
}
#endif /*#if 0*/

/*
 * packet_cleanup: Free all data allocated in the packet.
 */
int packet_cleanup(struct ssh_packet *pc) {
    if (pc->p_clr) {
	buf_cleanup(pc->p_clr);
	free(pc->p_clr);
    }
    if (pc->p_enc) {
	buf_cleanup(pc->p_enc);
	free(pc->p_enc);
    }
    memset(pc, 0, sizeof(struct ssh_packet));
    return(0);
}
