#ifndef _NTP_RECVBUFF_H
#define _NTP_RECVBUFF_H

/*
 * recvbuf memory management
 */
#define RECV_INIT	10	/* 10 buffers initially */
#define RECV_LOWAT	3	/* when we're down to three buffers get more */
#define RECV_INC	5	/* get 5 more at a time */
#define RECV_TOOMANY	40	/* this is way too many buffers */

/*
 * Format of a recvbuf.  These are used by the asynchronous receive
 * routine to store incoming packets and related information.
 */

/*
 *  the maximum length NTP packet contains the NTP header, one Autokey
 *  request, one Autokey response and the MAC. Assuming certificates don't
 *  get too big, the maximum packet length is set arbitrarily at 1000.
 */
#define	RX_BUFF_SIZE	1000		/* hail Mary */

struct recvbuf {
	struct recvbuf *next;   /* next buffer in chain */
	struct sockaddr_in recv_srcadr;
	/*SOCKET	fd;*/ 	    /* fd on which it was received */
	l_fp recv_time;			/* time of arrival */
	int recv_length;		/* number of octets received */
	NTP_PACKET recv_pkt;
	};

struct recvbuf *NTP_RECVBUFF_GetFirstBuf();


/*  Get a free buffer (typically used so an async
 *  read can directly place data into the buffer
 *
 *  The buffer is removed from the free list. Make sure
 *  you put it back with freerecvbuf() or
 */
 struct recvbuf *NTP_RECVBUFF_GetFreeBuffer(void);

void NTP_Recvbuff_Init(void);

/*   Add a buffer to the full list
 */
void NTP_RECVBUFF_AddRecvBuffer(struct recvbuf *rb);

#endif /* defined __recvbuff_h */

