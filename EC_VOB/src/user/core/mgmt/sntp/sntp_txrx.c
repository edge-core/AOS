/* static char SccsId[] = "+-<>?!SNTP_TXRX.C   22.1  06/05/02  09:00:00";
 * ------------------------------------------------------------------------
 *  FILE NAME  -  _SNTP_TXRX.H
 * ------------------------------------------------------------------------
 *  ABSTRACT:
 *
 *  Modification History:
 *  Modifier           Date        Description
 *  -----------------------------------------------------------------------
 *  S.K.Yang		  06-05-2002  Created
 * ------------------------------------------------------------------------
 *  Copyright(C)				Accton Corporation, 1999
 * ------------------------------------------------------------------------
 */
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <errno.h>

#include "l_stdlib.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "sntp_txrx.h"
#include "sntp_dbg.h"
#include "sntp_type.h"

extern int      errno;

/* naming constant */
unsigned short  SNTP_RECEIVE_LOCAL_PORT;                /* Local port */
UI8_T           SNTP_CLIENT_REQUEST;                    /* SNTP indicator */
static int      sntpFd;								    /* sntp file desc*/

typedef struct SNTP_TXRX_SockAddr_S
{
    union
    {
        struct sockaddr_in  inaddr_4;
        struct sockaddr_in6 inaddr_6;
    } s_addr_u;

    socklen_t saddr_len;
    
}SNTP_TXRX_SockAddr_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_TXRX_Init
 *------------------------------------------------------------------------------
 * PURPOSE  : Tx/Rx initialize   Initialize receive local port,version and mode of sntp
 *			  and initialize socket
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void SNTP_TXRX_Init(void)
{
    SNTP_RECEIVE_LOCAL_PORT = 1024;
	SNTP_CLIENT_REQUEST = (SNTP_VN_3| SNTP_MODE_3);	/* client request designator */

   /*if (sntpFd = socket(AF_INET, SOCK_DGRAM, 0)<0)
	*{
	*	printf("\nSNTP Init Failed : Can't open socket");
	*}
	*/
    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_TXRX_ReInit
 *------------------------------------------------------------------------------
 * PURPOSE  : Tx/Rx Reinitialize   socket ,version number, packet mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void SNTP_TXRX_ReInit(void)
{
    SNTP_RECEIVE_LOCAL_PORT = 1024;
	SNTP_CLIENT_REQUEST = (SNTP_VN_3 | SNTP_MODE_3);	/* client request designator */

    if (sntpFd > 0)
    {
        close(sntpFd);
    }

    if ((sntpFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("\nSNTP Init Failed : Can't open socket");
    }

    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_TXRX_Client
 *------------------------------------------------------------------------------
 * PURPOSE  : Using this routine as SNTP brocast/unicast client
 * INPUT    : ServerIpaddress: 1.if 0 then, implies brocast mode
 *							   2.input must be network order
 *			  Delaytime : Timeout when wait a response from server , in seconds.
 * OUTPUT   : ServerIpaddress : 1.If input is 0 ,then if it has received broadcast sntp packet form
 *								  server , fill with the ipaddress.
 *								2.time : Total seconds since 1900/01/01 00:00 from SNTP server.
 *							    3.tick : Current tick of time.
 * RETURN   : SNTP_TXRX_MSG_SUCCESS : Receive successfully,
 *			  SNTP_TXRX_MSG_INVALID_PARAMETER
 *			  SNTP_TXRX_MSG_FAIL
 *			  SNTP_TXRX_MSG_TIMEOUT
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SNTP_TXRX_STATUS_E SNTP_TXRX_Client(L_INET_AddrIp_T *ServerIpaddress,
                                    UI32_T  Delaytime,
                                    UI32_T  *time,
                                    UI32_T  *tick)
{
    SNTP_TXRX_SockAddr_T     sbinlocal;
    SNTP_TXRX_SockAddr_T     fromlocal;
    struct sockaddr     *sbin=(struct sockaddr*) &sbinlocal;
    struct sockaddr     *from=(struct sockaddr*) &fromlocal;
    struct timeval      timeout;                        /* used in "select" macro */
    fd_set              recvFD;
    unsigned short      local_port = SNTP_RECEIVE_LOCAL_PORT;
    float               time_delay;
    int                 count;
    UI32_T              tick_tmp;
    UI32_T              sntpBroadcastServer;            /* preferred broadcast server */
    float               sntp_WorkTicks = 0.0;
    float               delta_sntp_WorkTicks = 0.0;
    SNTP_PACKET         sntpUpdate;
    SNTP_PACKET         sntpRequest;

    memset((char *)&sbinlocal, 0, sizeof(sbinlocal));
    memset((char *)&fromlocal, 0, sizeof(fromlocal));

    switch(ServerIpaddress->type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
        {
            struct sockaddr_in *sbin4=(struct sockaddr_in*) &sbinlocal;
            
            sbin4->sin_family=AF_INET;
            sbin4->sin_port=L_STDLIB_Hton16((unsigned short)local_port);
            sbin4->sin_addr.s_addr = L_STDLIB_Hton32(INADDR_ANY);
            sbinlocal.saddr_len = sizeof(struct sockaddr_in);
            fromlocal.saddr_len = sizeof(struct sockaddr_in);
        }
            break;

        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
        {
            struct sockaddr_in6 *sbin6=(struct sockaddr_in6*) &sbinlocal;
            
            sbin6->sin6_family=AF_INET6;
            sbin6->sin6_port=L_STDLIB_Hton16((unsigned short)local_port);
            memcpy((void *)&sbin6->sin6_addr, (void *)&in6addr_any, sizeof(in6addr_any));
            sbinlocal.saddr_len = sizeof(struct sockaddr_in6);
            fromlocal.saddr_len = sizeof(struct sockaddr_in6);
        }
            break;

        case L_INET_ADDR_TYPE_UNKNOWN:
            /*broadcast mode
             *It does not support at present
             */
            break;

        default:
            return SNTP_TXRX_MSG_FAIL;
            break;
    }

    
    if (ServerIpaddress->addrlen == 0)/* broadcast mode */
    {
#if 0/*It does not support at present*/
        from->sin_addr.s_addr = L_STDLIB_Hton32(INADDR_ANY);
        sbin->sin_port = L_STDLIB_Hton16(SNTP_PORT);
#endif        
    }
    else/* unicast mode */
    {
        if(L_INET_InaddrToSockaddr(ServerIpaddress, SNTP_PORT, sizeof(fromlocal), from)!=TRUE)
        {
            if (DBG_SNTP_TURN_MESSAGE_ON_OFF)
            {
                printf("%s:Server is invalid\n",  __FUNCTION__);
            }
        }
    }

    /* Open a UDP socket */
    if ((sntpFd = socket(sbin->sa_family, SOCK_DGRAM, 0)) < 0)
    {
        if (DBG_SNTP_TURN_MESSAGE_ON_OFF)
        {
            printf(" %s:%d, errno-%d\n", __FUNCTION__, __LINE__, errno);
        }

        return SNTP_TXRX_MSG_FAIL;
    }

    /* bind socket */
    if (bind(sntpFd, sbin, sbinlocal.saddr_len) < 0)
    {
        /* Unable to obtain a broadcast socket so exit */
        close(sntpFd);

        if (DBG_SNTP_TURN_MESSAGE_ON_OFF)
        {
            printf(" %s:%d, errno-%d\n", __FUNCTION__, __LINE__, errno);
        }

        return SNTP_TXRX_MSG_FAIL;
    }

    /* if it is not in broadcast mode , then send a packet out */
    if (ServerIpaddress->addrlen != 0)
    {
        memset((char *) &sntpRequest, 0, sizeof(sntpRequest));
        sntpRequest.leapVerMode = SNTP_CLIENT_REQUEST;

        /*Get the system tick when send a request out in order to acquist the actual time
    	 *in unicast mode
     	 */
        sntp_WorkTicks = (float)SYSFUN_GetSysTick();

        /* Send request to sntp server. */

        if(sendto(sntpFd, (char *) &sntpRequest, sizeof(sntpRequest), 0,
               from, fromlocal.saddr_len)<0)
        {
            close(sntpFd);
            
            if (DBG_SNTP_TURN_MESSAGE_ON_OFF)
            {
                printf("Send to Server Fail\n");
            }
            
            return SNTP_TXRX_MSG_FAIL;
        }
    }

    /* set time out flag */
    timeout.tv_sec = (long)Delaytime;
    timeout.tv_usec = (long)0;

    /* set select flag */
    FD_ZERO(&recvFD);
    FD_SET(sntpFd, &recvFD);

    /* Listen if a SNTP packet is coming in */
    while (TRUE)
    {
        count = select(sntpFd + 1, &recvFD, NULL, NULL, &timeout);

        if (count == -1)
        {
            /* select again if interrupt occured */
            if ((EINTR == errno) || (EWOULDBLOCK == errno))
            {
                if (DBG_SNTP_TURN_MESSAGE_ON_OFF)
                {
        		    printf(" %s:%d, timeout-%ld\n",__FUNCTION__, __LINE__, timeout.tv_sec);
                }

                continue;
            }
        }
        else
        {
            break;
        }
    }

    if (count == -1)                                    /* An error occur */
    {
        close(sntpFd);

        if (DBG_SNTP_TURN_MESSAGE_ON_OFF)
        {
            printf(" %s:%d, errno-%d\n", __FUNCTION__, __LINE__, errno);
        }

        return SNTP_TXRX_MSG_FAIL;
    }
    else if (count == 0)                                /* Waiting timeout */
    {
        close(sntpFd);
        return SNTP_TXRX_MSG_TIMEOUT;
    }
    else if (FD_ISSET(sntpFd, &recvFD))                 /* A SNTP packet is coming in */
    {
   	   	recvfrom(sntpFd,(char *) &sntpUpdate, sizeof(SNTP_PACKET), 0, from, &fromlocal.saddr_len);

        /* Get the second between sending and receiveing in unicast mode */
   	   	tick_tmp= SYSFUN_GetSysTick(); /* This is the tick count of receiving packet */
        *tick = tick_tmp;
        delta_sntp_WorkTicks = (float)tick_tmp - sntp_WorkTicks;

        if (ServerIpaddress->addrlen == 0)                      /* In multicast mode */
        {
#if 0/*It does not support broadcast mode*/         
            sntpBroadcastServer = L_STDLIB_Ntoh32(from->sin_addr.s_addr);
            *ServerIpaddress = sntpBroadcastServer;
            *time = L_STDLIB_Ntoh32(sntpUpdate.transmitTimestampSec);
#endif            
        }
        else    /* In unicast mode ,and assume the transmition delay is same as
                 * receiver delay */
        {
            /* Assume system tick is 0.01 sec */
	    	if ((time_delay=(delta_sntp_WorkTicks/SYS_BLD_TICKS_PER_SECOND/2))>=1.0)
	    	{
                *time = L_STDLIB_Ntoh32(sntpUpdate.transmitTimestampSec) + (UI32_T) time_delay;
            }
            else
            {
                *time = L_STDLIB_Ntoh32(sntpUpdate.transmitTimestampSec);
            }
        }
    }

    close(sntpFd);

    return SNTP_TXRX_MSG_SUCCESS;
}

/* Debug use */
void SNTP_TXRX_SetLocalPort(unsigned int LocalPort)
{
    SNTP_RECEIVE_LOCAL_PORT = LocalPort;
    if (DBG_SNTP_TURN_MESSAGE_ON_OFF)
    {
        printf("\nDBG SNTP TXRX: Local port is %d", SNTP_RECEIVE_LOCAL_PORT);
    }

    return;
}

/* Debug use */
void SNTP_TXRX_SetVersion(UI8_T Version)
{
    switch (Version)
    {
        case 1:
            Version = SNTP_VN_1;
            break;

        case 2:
            Version = SNTP_VN_2;
            break;

        case 3:
            Version = SNTP_VN_3;
            break;

        default:
            Version = SNTP_VN_1;
            break;
    }

    SNTP_CLIENT_REQUEST = (Version | SNTP_MODE_3);
    if (DBG_SNTP_TURN_MESSAGE_ON_OFF)
    {
        printf("\nDBG SNTP TXRX: Version is %x", Version);
    }

    return;
}
