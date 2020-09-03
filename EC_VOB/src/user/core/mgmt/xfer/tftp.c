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
 */
/******************************************************************************
 * chengcw, 2001/4/11                                                         *
 * modify error message                                                       *
 *****************************************************************************/
/******************************************************************************
 * chengcw, 2001/3/30                                                         *
 * Add error code and error message                                           *
 *****************************************************************************/

#ifndef lint
#if 0
static char sccsid[] = "@(#)tftp.c	8.1 (Berkeley) 6/6/93";
#endif
static const char rcsid[] =
  "$FreeBSD: src/usr.bin/tftp/tftp.c,v 1.5 1999/08/28 01:06:24 peter Exp $";
#endif /* not lint */

/* Many bug fixes are from Jim Guyton <guyton@rand-unix> */

/*
 * TFTP User Program -- Protocol Machines
 */

/*#include <psos.h>    */
#include <stdlib.h> 
#include <string.h>
#include <sys/cdefs.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "sysfun.h"
//#include "iproute.h"
//#include "skt_vx.h"
//#include "socket.h"
#include "arpa_tftp.h"
#include "tftp.h"
#include "leaf_es3626a.h"
#include "sys_dflt.h"
#include "l_stdlib.h"

/*******************************************************************************
 *                           Naming Constant                                   *
 ******************************************************************************/
//#define	__P(x)			x		/* cwcheng */
#define	TFTP_TIMEOUT	5		/* secs between rexmt's */
#define	TFTP_PORT		69		/* cwcheng */

#define SYS_DFLT_TFTP_BLKSIZE    512

#define SYS_DFLT_TFTP_OP_BLKSIZE     1432
#define PKTSIZE			(SYS_DFLT_TFTP_OP_BLKSIZE + OPCODE_LEN + BLKNUMBER_LEN)

#define  errno 0

/*******************************************************************************
 *                            Local Variable                                   *
 ******************************************************************************/
static struct sockaddr_storage peeraddr;
static int f = -1;
static short port = TFTP_PORT; /* cwcheng */
static int trace = 0; /* cwcheng */
static int rexmtval = TFTP_TIMEOUT;
static int maxtimeout = 0; /* cwcheng */

static char buf[PKTSIZE];
static char ackbuf[PKTSIZE];  /* cwcheng */
static int timeout;

// chengcw, 2001/3/30
static BOOL_T progress;
static int error_code;
static char tftp_message[MAXSIZE_fileCopyTftpErrMsg + 1];

static SYS_TYPE_CallBack_T *tftp_transmitt_callback;

/*******************************************************************************
 *                      Static Function Prototype                              *
 ******************************************************************************/
static unsigned long xmitfile(char *, char *, char *, unsigned long);
static unsigned long recvfile(char *, char *, BOOL_T, char *, unsigned long);
static void nak __P((int));
static int makerequest __P((int, const char *, struct tftphdr *, const char *));
static void tpacket __P((const char *, struct tftphdr *, int));
static int synchnet __P((int));
static size_t TFTP_GetPeeraddrSize();
static void TFTP_NotifyTransmittingStatus(UI32_T percent);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TFTP_SetCallback
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function. The registered function will
 *           be called while tftp is transmitting file.
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *
 * -------------------------------------------------------------------------*/
void TFTP_SetCallback(void (*fun)(UI32_T percent))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(tftp_transmitt_callback);

}

int TFTP_open(L_INET_AddrIp_T *inaddr_p)
{
    struct sockaddr *sockaddr_p;
    struct sockaddr_in sin4_p;
    struct sockaddr_in6 sin6_p;
    int sockaddr_len;

    error_code = 0;
    memset ((char *)&sockaddr_p, 0, sizeof (sockaddr_p));

    switch(inaddr_p->type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
        {
            memset(&sin4_p, 0, sizeof(sin4_p));
            sin4_p.sin_family = AF_INET;
            sin4_p.sin_port = htons(SYS_DFLT_TFTP_PORT);
            sockaddr_len = sizeof(struct sockaddr_in);
            sockaddr_p = (struct sockaddr *)&sin4_p;
        }
            break;

        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
        {
            memset(&sin6_p, 0, sizeof(sin6_p));
            sin6_p.sin6_family = AF_INET6;
            sin6_p.sin6_port = htons(SYS_DFLT_TFTP_PORT);
            sockaddr_len = sizeof(struct sockaddr_in6);
            sockaddr_p = (struct sockaddr *)&sin6_p;
        }
            break;

        default:
            error_code = EBADOP;
            return 1;
    }

    f = socket(sockaddr_p->sa_family, SOCK_DGRAM, IPPROTO_UDP);

    if (f < 0)
    {
        /* create socket error */
        error_code = ESOCKETOPEN;
        return 1;
    }

    if (bind(f, sockaddr_p, sockaddr_len) < 0)
    {
        /* bind error */
        close(f);
        f = -1;
        error_code = ESOCKETBIND;
        return 1;
    }

    return 0;
}

int TFTP_close()
{
    if (f >= 0) 
    {
        close(f);
        progress = FALSE;//beck,add
    }

    f = -1;
    return 0;
}

/*
 * Send file(s).
 */
unsigned long TFTP_put(L_INET_AddrIp_T *inaddr_p,
                       char * filename,
                       char * mode,
                       char * buffer,
                       unsigned long buffer_size,
                       unsigned long retry_times,
                       unsigned long timeout)
{
    memset ((char *)&peeraddr, 0, sizeof (peeraddr));
    L_INET_InaddrToSockaddr(inaddr_p, port, sizeof(struct sockaddr_in6), (struct sockaddr *)&peeraddr);
    rexmtval = timeout;
    maxtimeout = retry_times * timeout;
    return xmitfile(filename, mode, buffer, buffer_size);
}

/*
 * Receive file(s).
 */
unsigned long TFTP_get(L_INET_AddrIp_T *inaddr_p, 
                       char * filename, 
                       char * mode, 
                       BOOL_T get_whole_file, 
                       char * buffer, 
                       unsigned long buffer_size, 
                       unsigned long retry_times,
                       unsigned long timeout)
{
    memset ((char *)&peeraddr, 0, sizeof (peeraddr));
    L_INET_InaddrToSockaddr(inaddr_p, port, sizeof(struct sockaddr_in6), (struct sockaddr *)&peeraddr);
    rexmtval = timeout;
    maxtimeout = retry_times * timeout;
    return recvfile(filename, mode, get_whole_file, buffer, buffer_size);
}

/*
 * Send the requested file.
 */
static unsigned long xmitfile(
    char *name,
    char *mode,
    char *buffer,
    unsigned long buffer_size)
{
    register struct tftphdr *ap;	   /* data and ack packets */
    struct tftphdr *dp;
    register int n;
    volatile int size;
    volatile unsigned short block;
    #if 0 /* comment out for compiler warning:variable 'convert' set but not used */
    volatile int convert;
    #endif

    volatile unsigned long amount; /* size of transmitted data in byte */
    struct sockaddr_storage from;
    socklen_t fromlen;
    size_t peeraddrlen = TFTP_GetPeeraddrSize();
    fd_set read_mask;
    struct timeval wait;
    char *op_ptr, *p_invalid;/*maggie porting it from ECN330*/
    unsigned long blksize;/*maggie porting it from ECN330*/
    BOOL_T firsttrip=1;
    BOOL_T first_ack_or_oack=0;
    
    dp = (struct tftphdr *)buf; /* cwcheng */
    ap = (struct tftphdr *)ackbuf;
    #if 0 /* comment out for compiler warning:variable 'convert' set but not used */
    convert = !strcmp(mode, "netascii");
    #endif
    block = 0;
    amount = 0;
    blksize = SYS_DFLT_TFTP_BLKSIZE; /*default tftp blksize *//*maggie porting it from ECN330*/
    
    do
    {
        first_ack_or_oack = 0;

        if (firsttrip == 1)
        {
            size = makerequest(WRQ, name, dp, mode) - 4;
        }
        else
        {
            if (buffer_size - amount < blksize)
            {
                size = buffer_size -amount;
            }
            else
            {
                size = blksize;
            }

            memcpy(dp->th_data, buffer, size);
            buffer += size;

            if (size < 0) 
            {
                nak(errno + 100);
                break;
            }
            dp->th_opcode = htons((unsigned short)DATA);
            dp->th_block = htons(block);
        }

        timeout = 0;

send_data:
        TFTP_NotifyTransmittingStatus(amount * 100 / buffer_size);

        if (trace)
        {
            tpacket("sent", dp, size + 4);
        }

        n = sendto(f, (char *)dp, size + 4, 0, (struct sockaddr *)&peeraddr, peeraddrlen);

        if (n != size + 4) 
        {
            /* sendto error */
            error_code = ESEND;
            amount = 0;
            goto abort;
        }

        for ( ; ; )
        {
            if (progress == TRUE) 
            {
                error_code = ECANCEL;
                amount = 0;
                goto abort;
            }

            // set select interval
            wait.tv_sec = rexmtval;
            wait.tv_usec = 0;

            // zero read mask
            FD_ZERO(&read_mask);

            // set read mask
            FD_SET(f, &read_mask);

            n = select(FD_SETSIZE, &read_mask, (fd_set *)0, (fd_set *)0, &wait);

            /* timeout to wait a ack packet from server
             */
            if (n == 0)
            {
                timeout += rexmtval;

                if (timeout <= maxtimeout)
                {
                    /* retry
                     */
                    goto send_data;
                }
                else /* max timeout is reached */
                {
                    error_code = ETIMEOUT;
                    amount = 0;
                    goto abort;
                }
            }

            /* select error
             */
            if (n < 0)
            {
                error_code = EUNDEF;
                amount = 0;
                goto abort;
            }

            fromlen = (socklen_t)sizeof(from);

            // receive ack packet from server
            n = recvfrom(f, ackbuf, sizeof(ackbuf), 0, (struct sockaddr *)&from, &fromlen);

            if (n < 0)
            { /* recvfrom error */
                error_code = ERECEIVE;
                amount = 0;
                goto abort;
            }

            if( AF_INET == from.ss_family)
            {
                ((struct sockaddr_in *)&peeraddr)->sin_port = ((struct sockaddr_in *)&from)->sin_port;
            }
            else
            {
                ((struct sockaddr_in6 *)&peeraddr)->sin6_port = ((struct sockaddr_in6 *)&from)->sin6_port;
            }

            if (trace)
            {
                tpacket("received", ap, n);
            }

            /* should verify packet came from server */

            if (ntohs(ap->th_opcode) == ERROR_TFTP)
            {
                if (trace)
                {
                    /*SYSFUN_Debug_Printf("Error code %d: %s\n", ap->th_code, ap->th_msg);*/
                }
                error_code = ntohs(ap->th_code) + 1;
                if(error_code == EUNDEF)
                {
                    UI32_T  len = 0;
                    len = strlen(ap->th_data);
                    if(len > MAXSIZE_fileCopyTftpErrMsg)
                    {
                        len=MAXSIZE_fileCopyTftpErrMsg;
                    }
                    memcpy(tftp_message,ap->th_data,len);
                    tftp_message[len] = '\0';
                }
                amount = 0;
                goto abort;
            }

            if (ntohs(ap->th_opcode) == ACK) 
            {
                int j;

                if (ntohs(ap->th_block) == 0 && firsttrip == 1)
                {   /*ACK for WRQ*/
                    first_ack_or_oack = 1;
                    break;
                }

                if (ntohs(ap->th_block) == block) 
                {
                    amount += size;    /*success*/
                    break;
                }
                /*
                 * On an error, try to synchronize
                 * both sides.
                 */
                j = synchnet(f);

                if (j && trace) 
                {
                    /*SYSFUN_Debug_Printf("discarded %d packets\n", j);*/
                }
                if (ntohs(ap->th_block) == (block-1))
                {
                    goto send_data;
                }
            }
            else if (ntohs(ap->th_opcode) == OACK)
            {
                /* lin feng
                 * Wed 07/20/2005
                 * tftp blksize option
                 */
                size = n - OPCODE_LEN;
                op_ptr = ap->th_u.tu_stuff;
                blksize = 0;

                first_ack_or_oack = 1;

                /* get options */
                while(size > 0)
                {
                    if('\0' != *op_ptr)
                    {
                        /* blksize */
                        if(0 == strcmp(op_ptr, TFTP_OP_BLKSIZE_STR))
                        {
                            op_ptr += strlen(op_ptr) + 1;
                            size -= sizeof(op_ptr) + 1;
                            if ('\0' != *op_ptr)
                            {
                                blksize = strtol(op_ptr, &p_invalid, 10);
                                op_ptr += strlen(op_ptr) + 1;
                                size   -= sizeof(op_ptr) + 1;
                            }
                            else
                            {
                                op_ptr++;
                                size--;
                            }
                        }
                        else if (0)
                        {
                            /* add other tftp option here. */
                        }
                        else {
                            op_ptr += strlen(op_ptr) + 1;
                            size   -= sizeof(op_ptr) + 1;
                        }
                    }
                    else
                    {
                        size--;
                        op_ptr++;
                    }
                }

                /* check options */
                if ((blksize < 60) || (blksize > SYS_DFLT_TFTP_OP_BLKSIZE))
                {
                    error_code = E_BUF_SIZE_EXCEEDS;
                    goto abort;
                }
                block = 0;
                break;
            }
        }

        firsttrip = 0;
        block ++;
    } while (size == blksize || first_ack_or_oack == 1);
    if (amount == buffer_size)
    {
        TFTP_NotifyTransmittingStatus(100);
    }
abort:
    return amount;
}

/*
 * Receive a file.
 */
static unsigned long recvfile(
    char *name,
    char *mode,
    BOOL_T get_whole_file,
    char *buffer,
    unsigned long buffer_size)
{
    register struct tftphdr *ap;
    struct tftphdr *dp;
    register int n;
    volatile int size, firsttrip;
    volatile unsigned long amount; /* size of received data in byte */
    volatile unsigned short block;
    struct sockaddr_storage from;
    socklen_t fromlen;
    size_t peeraddrlen = TFTP_GetPeeraddrSize();
    fd_set read_mask;
    char *op_ptr, *p_invalid;
    struct timeval wait;
    #if 0 /* comment out for compiler warning:variable 'convert' set but not used */
    volatile int convert;		/* true if converting crlf -> lf */
    #endif
    int blksize;
    
    dp = (struct tftphdr *)buf; /* cwcheng */
    ap = (struct tftphdr *)ackbuf;
    #if 0 /* comment out for compiler warning:variable 'convert' set but not used */
    convert = !strcmp(mode, "netascii");
    #endif
    block = 1;
    firsttrip = 1;
    amount = 0;
    blksize = SYS_DFLT_TFTP_BLKSIZE;

    do 
    {
        if (firsttrip) 
        {
            size = makerequest(RRQ, name, ap, mode);
            firsttrip = 0;
        }
        else 
        {
            ap->th_opcode = htons((unsigned short)ACK);
            ap->th_block = htons(block);
            size = 4;
            block ++;
        }

        timeout = 0;
        //(void) setjmp(timeoutbuf);
send_ack:
        TFTP_NotifyTransmittingStatus(amount * 100 / buffer_size);

        if (trace)
        {
            tpacket("sent", ap, size);
        }

        // send packet
        // first time, send RRQ packet
        // others, send ack packet
        if (sendto(f, ackbuf, size, 0, (struct sockaddr *)&peeraddr, peeraddrlen) != size)
        {
            /* sendto error */
            error_code = ESEND;
            amount = 0;
            goto abort;
        }
        //beck,test
        //SYSFUN_Sleep(1);
        for ( ; ; ) 
        {
            if (progress == TRUE) 
            {
                error_code = ECANCEL;
                amount = 0;
                goto abort; //beck,add
            }

            // set select interval
            wait.tv_sec = rexmtval;
            wait.tv_usec = 0;

            // zero read mask
            FD_ZERO(&read_mask);

            // set read mask
            FD_SET(f, &read_mask);

            n = select(FD_SETSIZE, &read_mask, (fd_set *)0, (fd_set *)0, &wait);

            /* timeout to wait a data packet from server
             */
            if (n == 0)
            {
                timeout += rexmtval;

                if (timeout <= maxtimeout)
                {
                    /* retry
                     */
                    goto send_ack;
                }
                else /* max timeout is reached */
                {
                    error_code = ETIMEOUT;
                    amount = 0;
                    goto abort;
                }
            }

            /* select error
             */
            if (n < 0)
            {
                error_code = EUNDEF;
                amount = 0;
                goto abort;
            }

            fromlen = (socklen_t)sizeof(from);

            // read from server
            n = recvfrom(f, (char *)dp, PKTSIZE, 0, (struct sockaddr *)&from, &fromlen);

            if (n < 0) 
            { 
                /* recvfrom error */
                error_code = ERECEIVE;
                amount =0;
                goto abort_no_ack;
            }

            if( AF_INET == from.ss_family)
            {
                ((struct sockaddr_in *)&peeraddr)->sin_port = ((struct sockaddr_in *)&from)->sin_port;
            }
            else
            {
                ((struct sockaddr_in6 *)&peeraddr)->sin6_port = ((struct sockaddr_in6 *)&from)->sin6_port;
            }

            if (trace)
            {
                tpacket("received", dp, n);
            }

            /* should verify client address */

           if (ntohs(dp->th_opcode) == ERROR_TFTP)
            {
                if (trace)
                {
                    /*SYSFUN_Debug_Printf("Error code %d: %s\n", dp->th_code, dp->th_msg);*/
                }

                error_code = ntohs(dp->th_code) + 1;
                if(error_code == EUNDEF)
                {
                    UI32_T  len = 0;
                    len = strlen(dp->th_data);
                    if(len > MAXSIZE_fileCopyTftpErrMsg)
                    {
                        len=MAXSIZE_fileCopyTftpErrMsg;
                    }
                    memcpy(tftp_message,dp->th_data,len);
                    tftp_message[len] = '\0';
                }
                amount = 0;
                goto abort_no_ack;
            }

            if (ntohs(dp->th_opcode) == DATA)
            {
                int j;

                if (ntohs(dp->th_block) == block) 
                {
                    break;/* have next packet */
                }
                /*
                 * On an error, try to synchronize
                 * both sides.
                 */
                j = synchnet(f);

                if (j && trace) 
                {
                    /*SYSFUN_Debug_Printf("discarded %d packets\n", j);*/
                }

                if (ntohs(dp->th_block) == (block-1)) 
                {
                    goto send_ack;	/* resend ack */
                }
            }
            else if (ntohs(dp->th_opcode) == OACK)
            {
                /* lin feng
                 * Wed 07/20/2005
                 * tftp blksize option
                 */
                size = n - OPCODE_LEN;
                op_ptr=dp->th_stuff;
                blksize = 0;			 

                /* get options */
                while(size > 0)
                {
                    if('\0' != *op_ptr)
                    {
                        /* blksize */
                        if(0 == strcmp(op_ptr, TFTP_OP_BLKSIZE_STR))
                        {
                            op_ptr += strlen(op_ptr) + 1;
                            size -= sizeof(op_ptr) + 1;
                            if ('\0' != *op_ptr)
                            {
                                blksize = strtoul(op_ptr, &p_invalid, 10);
                                op_ptr += strlen(op_ptr) + 1;
                                size   -= sizeof(op_ptr) + 1;
                            }
                            else
                            {
                                op_ptr++;
                                size--;
                            }
                        }
                        else if (0)
                        {
                            /* add other tftp option here. */
                        }
                        else {
                            op_ptr += strlen(op_ptr) + 1;
                            size   -= sizeof(op_ptr) + 1;
                        }
                    }
                    else
                    {
                        size--;
                        op_ptr++;
                    }
                }

                /* check options */
                if ((blksize < 60) || (blksize > SYS_DFLT_TFTP_OP_BLKSIZE))
                {
                    error_code = E_BUF_SIZE_EXCEEDS;
                    goto abort;
                }
                /* ack blk 0 */
                ap->th_opcode = htons((unsigned short)ACK);
                ap->th_block = htons((unsigned short)(0));
                size = 4;
                goto send_ack;
            }
        }

        size = n - 4;

        if (TRUE == get_whole_file)
        {
            if (amount + size > buffer_size)
            {
                error_code = E_BUF_SIZE_EXCEEDS;
                /*
                nak(errno + 100);
                nak(errno + 100);
                */
                amount = 0;
                goto abort;
            }
        }
        else
        {
            /* In get a piece of file mode,
             * buffer_size indicates the wanted size of buffer
             */
            if (amount + size > buffer_size)
            {
                size = buffer_size - amount;
            }
        }

        memcpy(buffer, dp->th_data, size); //cwcheng
        buffer += size; //cwcheng  
        amount += size;
    } while (size == blksize);

    if (amount == buffer_size)
    {
        TFTP_NotifyTransmittingStatus(100);
    }

abort:/* ok to ack, since user */
    if (0 == error_code && TRUE == get_whole_file)
    {
        ap->th_opcode = htons((unsigned short)ACK);
        ap->th_block = htons(block);
    
        // send ack packet
        (void) sendto(f, ackbuf, 4, 0, (struct sockaddr *)&peeraddr, peeraddrlen);
    }
    else if (0 == error_code && FALSE == get_whole_file)
    {
        nak(ECANCEL);
    }
    else
    {
        nak(error_code);
    }

abort_no_ack:
    return amount;
}

static int makerequest(
    int request,
    const char *name,
    struct tftphdr *tp,
    const char *mode)
{
    register char *cp;
    UI8_T blksize[TFTP_OP_BLKSIZE_VALUE_LEN];

    tp->th_opcode = htons((unsigned short)request);
    cp = tp->th_stuff;
    strcpy(cp, name);
    cp += strlen(name);
    *cp++ = '\0';
    strcpy(cp, mode);
    cp += strlen(mode);
    *cp++ = '\0';
    /* blksize */
    strcpy(cp, TFTP_OP_BLKSIZE_STR);
    cp += strlen(TFTP_OP_BLKSIZE_STR);
    *cp++ = '\0';
    sprintf((char *)blksize, "%d", SYS_DFLT_TFTP_OP_BLKSIZE);
    strcpy(cp, (char *)blksize);
    cp += strlen((char *)blksize);
    *cp++ = '\0';
    return (cp - (char *)tp);
}

struct errmsg 
{
    int	e_code;
    char *e_msg;
} errmsgs[] = {
    { EUNDEF,		"TFTP Download Error: Undefined error code" },
    { ENOTFOUND,	"TFTP Download Error: File not found" },
    { EACCESS,		"TFTP Download Error: Access violation" },
    { ENOSPACE,		"TFTP Download Error: Disk full or allocation exceeded" },
    { EBADOP,		"TFTP Download Error: Illegal TFTP operation" },
    { EBADID,		"TFTP Download Error: Unknown transfer ID" },
    { EEXISTS,		"TFTP Download Error: File already exists" },
    { ENOUSER,		"TFTP Download Error: No such user" },
    { ETIMEOUT,		"TFTP Download Error: Send/Receive timeout" },
    { ESEND,		"TFTP Download Error: Send error" },
    { ERECEIVE,		"TFTP Download Error: Receive error" },
    { ESOCKETOPEN,	"TFTP Download Error: Socket open error" },
    { ESOCKETBIND,	"TFTP Download Error: Socket bind error" },
    { ECANCEL,		"TFTP Download Error: User cancel the tftp action" },
    { E_BUF_SIZE_EXCEEDS, "TFTP Download Error: File size exceeded"},
    { -1,               0 }
};

/*
 * Send a nak packet (error message).
 * Error code passed in is one of the
 * standard TFTP codes, or a UNIX errno
 * offset by 100.
 */
static void nak(int error)
{
    const int MAX_TFTP_ERROR_CODE = 7; /* RFC 1350 */
    register struct errmsg *pe;
    register struct tftphdr *tp;
    int length;
    char *strerror();
    size_t peeraddrlen = TFTP_GetPeeraddrSize();

    /* no error
     */
    if (0 == error)
    {
        return;
    }

    tp = (struct tftphdr *)ackbuf;
    tp->th_opcode = htons((unsigned short)ERROR_TFTP);

    /* because we shift the error by 1 so we need to minus 1 each getting the value
     */
    tp->th_code = htons((unsigned short) ((MAX_TFTP_ERROR_CODE < (error-1)) ? (EUNDEF-1) : (error-1))  );

    for (pe = errmsgs; pe->e_code >= 0; pe++)
    {
        if (pe->e_code == error)
        break;
    }
    if (pe->e_code < 0) 
    {
        pe->e_msg = strerror(error - 100);
        tp->th_code = EUNDEF-1;
    }

    strcpy(tp->th_msg, pe->e_msg);
    length = strlen(pe->e_msg) + 5; /* includes terminal char */

    if (trace)
    {
        tpacket("sent", tp, length);
    }

    if (sendto(f, ackbuf, length, 0, (struct sockaddr *)&peeraddr, peeraddrlen) != length)
    {
        ;
        /*warn("nak"); */
    }
}

static void tpacket(
    const char *s,
    struct tftphdr *tp,
    int n)
{
//  static char *opcodes[] =
//  { "#0", "RRQ", "WRQ", "DATA", "ACK", "ERROR_TFTP" };
    register char *cp, *file;
    unsigned short op = ntohs(tp->th_opcode);

    if (op < RRQ || op > ERROR_TFTP)
    {
        ;/*SYSFUN_Debug_Printf("%s opcode=%x ", s, op);*/
    }
    else
    {
        ;/*SYSFUN_Debug_Printf("%s %s ", s, opcodes[op]);*/
    }
    switch (op) 
    {
        case RRQ:
        case WRQ:
            n -= 2;
            file = cp = tp->th_stuff;
            cp = strchr(cp, '\0');
            /*SYSFUN_Debug_Printf("<file=%s, mode=%s>\n", file, cp + 1);*/
            break;

        case DATA:
            /*SYSFUN_Debug_Printf("<block=%d, %d bytes>\n", ntohs(tp->th_block), n - 4);*/
            break;

        case ACK:
            /*SYSFUN_Debug_Printf("<block=%d>\n", ntohs(tp->th_block));*/
            break;

        case ERROR_TFTP:
            /*SYSFUN_Debug_Printf("<code=%d, msg=%s>\n", ntohs(tp->th_code), tp->th_msg);*/
            break;
    }
    return;

    file; /* workaround for compiler warning:variable 'file' set but not used */
}

int
synchnet(f)
    int	f;		/* socket to flush */
{
    int i = 0, j = 0;
    char rbuf[PKTSIZE];
    struct sockaddr_storage from;
    socklen_t fromlen;

    while (1) 
    {
        /* cwcheng: PNA doesn't support this flag		(void) ioctlsocket(f, FIONREAD, &i); */
        if (i) 
        {
            j++;
            fromlen = (socklen_t)sizeof from;
            (void) recvfrom(f, rbuf, sizeof (rbuf), 0, (struct sockaddr *)&from, &fromlen);
        }
        else
        {
            return(j);
        }
    }
}

void TFTP_SetProgressBar(BOOL_T flag)
{
    progress = flag;
}

// chengcw, 2001/3/30
void TFTP_GetErrorMsg(char *error_message)
{
    memcpy(error_message,tftp_message,strlen(tftp_message));
    memset(tftp_message,0,sizeof(tftp_message));
}
/* 2002/12/05, erica 
 */
int TFTP_GetErrorCode()
{
    return error_code;
}

static size_t TFTP_GetPeeraddrSize()
{
    switch(peeraddr.ss_family)
    {
        case AF_INET:
            return sizeof(struct sockaddr_in);

        case AF_INET6:
            return sizeof(struct sockaddr_in6);

        default:
            return 0;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TFTP_NotifyTransmittingStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the file is transmitting.
 * INPUT   : percent - File tansmitt status.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void TFTP_NotifyTransmittingStatus(UI32_T percent)
{
    SYS_TYPE_CallBack_T  *fun_list;

    for(fun_list = tftp_transmitt_callback; fun_list != NULL; fun_list = fun_list->next)
    {
        fun_list->func(percent);
    }
}
