/* MODULE NAME: dns_cmm.c
 *
 * PURPOSE:
 *       This module provide functions receive TCP or UDP packets
 *       functions transmit TCP or UDP packets
 *
 * NOTES:
 *
 * History:
 *       Date          -- Modifier,   Reason
 *       2002-09-06    -- Wiseway , created
 *       2002-10-24    -- Wiseway   modified for convention
 *
 * Copyright(C)      Accton Corporation, 2002
 */

/* isiah.2004-01-06. remove all compile warring message.*/
#include "sys_type.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_DNS == TRUE)

#include "sysfun.h"
#include "sys_module.h"
#include "l_mm.h"
#include <stdlib.h>
#include <ctype.h>

#include "dns.h"
#include "dns_type.h"
#include "dns_cmm.h"

/* FUNCTION NAME : DNS_TxTCPMsg
 * PURPOSE:
 *      This function is used for sending TCP message to remote client or server.
 *
 * INPUT:
 *      int        -- the socket to send message
 *      DNS_Tx_PTR -- the struct records the info for sending message.
 *
 * OUTPUT:
 *      DNS_Tx_PTR -- the struct records the info after sending message.
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK :success.
 *
 * NOTES:
 *      This function will be called by DNS_Proxy_Send  in case of TCP.
 */
int DNS_TxTCPMsg(int sock,DNS_Tx_PTR tcp_tx_ptr)
{
    int n = 0;
    if(tcp_tx_ptr->pTx_Buf == NULL)
    {
        return DNS_OK;
    }
    n = send(sock, tcp_tx_ptr->pTx_Buf + tcp_tx_ptr->tx_len, tcp_tx_ptr->tx_buf_len - tcp_tx_ptr->tx_len, 0);
    if(n == ERROR)
    {
        return DNS_ERROR;
    }
    else
    {
        tcp_tx_ptr->tx_len += n;
        return DNS_OK;
    }

}


/* FUNCTION NAME : DNS_TxUDPMsg
 * PURPOSE:
 *      This function is used for sending UDP message to remote client or server.
 *
 * INPUT:
 *      int               -- the socket to send message
 *      DNS_Tx_PTR        -- a pointer to a struct to transmit udp message
 *      struct sockaddr * -- recipient's address
 *
 * OUTPUT:
 *      DNS_Tx_PTR        -- a pointer to a struct which records the information after sending udp message.
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK    :success.
 *
 * NOTES:
 *      This function will be called by DNS_Proxy_Send  in case of UDP.
 */
int DNS_TxUDPMsg(int sock,DNS_Tx_PTR udp_tx_ptr,struct sockaddr *to)
{
    int cnt;
    UI32_T sock_len = 0;

    if(to->sa_family== AF_INET)
    {
        sock_len=sizeof(struct sockaddr_in);
    }
    else if (to->sa_family== AF_INET6)
    {
        sock_len=sizeof(struct sockaddr_in6);
    }

    cnt = sendto(sock, udp_tx_ptr->pTx_Buf, udp_tx_ptr->tx_buf_len, 0, to, sock_len);

    if(cnt == ERROR)
    {
        /* UDP_send_failed; */
        return DNS_ERROR;
    }
    else
    {
        udp_tx_ptr->tx_len = cnt;
        return DNS_OK;
    /*  return cnt; */
    }
}


/* FUNCTION NAME : DNS_TCP_RxMsg
 * PURPOSE:
 *      This function receives tcp message.
 *
 * INPUT:
 *      int        -- TCP socket to receive message from
 *      DNS_Rx_PTR -- a pointer to a struct to receive tcp message
 *
 * OUTPUT:
 *      DNS_Rx_PTR -- a pointer to a struct who records the status for receive tcp message.
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK    :success.
 *
 * NOTES:
 *      This function will be called by DNS_PROXY_Daemon or DNS_Rsv_Daemon.
 */
int   DNS_RxTCPMsg(int sock,DNS_Rx_PTR tcp_rx_ptr)
{
    int n = 0;
    I8_T rxlen[2];

    if(tcp_rx_ptr->stat == DNS_RX_STAT_LEN1)
    {
        n = recv(sock, rxlen, 2, 0);
        if(n == ERROR)
        {
            return DNS_ERROR;
        }
        else if(n == 0)
        {
            return DNS_OK;
        }
        else if(n == 1)
        {
            tcp_rx_ptr->rx_buf_len = ((int)(rxlen[0] & 0xff)) * 256;
            tcp_rx_ptr->stat = DNS_RX_STAT_LEN2;
            return DNS_OK;
        }
        else if(n == 2)
        {
            tcp_rx_ptr->rx_buf_len = ((int)(rxlen[0] & 0xff)) * 256;
            tcp_rx_ptr->rx_buf_len += (int)(rxlen[1] & 0xff);
            tcp_rx_ptr->size = tcp_rx_ptr->rx_buf_len + 1;
            tcp_rx_ptr->pRx_Buf = (I8_T *)L_MM_Malloc(tcp_rx_ptr->size, L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_RXTCPMSG));
            if(tcp_rx_ptr->pRx_Buf == NULL)
            {
                return DNS_ERROR;
            }
            tcp_rx_ptr->stat = DNS_RX_STAT_DATA;
        }
        else
        {
            return DNS_ERROR;
        }
    }   /* fi LEN1 */

    if(tcp_rx_ptr->stat == DNS_RX_STAT_LEN2)
    {
        n = recv(sock, rxlen, 1, 0);
        if(n == ERROR)
        {
            return DNS_ERROR;
        }
        else if(n == 0)
        {
            return DNS_OK;
        }
        else if(n == 1)
        {
            tcp_rx_ptr->rx_buf_len += (int)(rxlen[0] & 0xff);
            tcp_rx_ptr->size = tcp_rx_ptr->rx_buf_len + 1;
            tcp_rx_ptr->pRx_Buf = (I8_T *)L_MM_Malloc(tcp_rx_ptr->size, L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_RXTCPMSG));
            if(tcp_rx_ptr->pRx_Buf == NULL)
            {
                return DNS_ERROR;
            }
            tcp_rx_ptr->stat = DNS_RX_STAT_DATA;
        }
        else
        {
            return DNS_ERROR;
        }
    }   /* fi LEN2 */

    if(tcp_rx_ptr->stat == DNS_RX_STAT_DATA)
    {
        /* added NULL check
         */
        if (tcp_rx_ptr->pRx_Buf == NULL)
        {
            return DNS_ERROR;
        }

        n = recv(sock, tcp_rx_ptr->pRx_Buf + tcp_rx_ptr->rx_len, tcp_rx_ptr->size - tcp_rx_ptr->rx_len, 0);
        if(n == ERROR)
        {
            if(tcp_rx_ptr->pRx_Buf)
            {
                L_MM_Free(tcp_rx_ptr->pRx_Buf);
                tcp_rx_ptr->pRx_Buf = NULL;
            }
            return DNS_ERROR;
        }
        else
        {
            tcp_rx_ptr->rx_len += n;
            if(tcp_rx_ptr->rx_len == tcp_rx_ptr->rx_buf_len)
            {
                tcp_rx_ptr->stat = DNS_RX_STAT_FINL;
                return DNS_OK;
            }
            else
            {
                return DNS_OK;
            }
        }

    }   /* fi DATA */

    return DNS_OK;
}


/* FUNCTION NAME : DNS_UDP_RxMsg
 * PURPOSE:
 *      This function receives udp message.
 *
 * INPUT:
 *      int        -- UDP socket to receive message from
 *      DNS_Rx_PTR -- a pointer to a struct to receive udp message
 *
 * OUTPUT:
 *      struct sockaddr -- where to copy sender's addr.
 *      DNS_Rx_PTR      -- a pointer to a struct which records the status for
 *                          receiving udp message
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK    :success.
 *
 * NOTES:
 *      This function will be called by DNS_PROXY_Daemon or DNS_Rsv_Daemon.
 */
int  DNS_RxUDPMsg(int sock,
                   struct sockaddr *from,
                   DNS_Rx_PTR udp_rx_ptr
                   )
{
    int n = 0;
    socklen_t pFromLen = sizeof(DNS_TYPE_SockAddr_T);

    if(udp_rx_ptr->size == 0)
    {
        udp_rx_ptr->size = 512;
        udp_rx_ptr->pRx_Buf = (I8_T *)L_MM_Malloc(udp_rx_ptr->size, L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_RXUDPMSG));
        if(udp_rx_ptr->pRx_Buf == NULL)
        {
            return DNS_ERROR;
        }
    }

    n = recvfrom(sock, udp_rx_ptr->pRx_Buf, udp_rx_ptr->size, 0, from, &pFromLen);  /*maggie liu remove warning*/

    if(n < 0)
    {
        L_MM_Free(udp_rx_ptr->pRx_Buf);
        udp_rx_ptr->pRx_Buf = NULL;
        return DNS_ERROR;
    }
    udp_rx_ptr->rx_buf_len = udp_rx_ptr->rx_len = n;

    return DNS_OK;
}


/*
 * FUNCTION NAME : DNS_Name2Lower
 *
 * PURPOSE:
 *      convert a domain name to lower case
 *
 * INPUT:
 *      const I8_T* -- up case or lower case
 *
 * OUTPUT:
 *      I8_T* -- all in lower case
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 *
 */
void DNS_Name2Lower(const I8_T* name,I8_T* rname)
{
    const I8_T* ptr = name;

    /* more efficient code maybe: */
    /* while(*rname++ = tolower(*ptr++)); rname = '\0'; */

    while('\0' != *ptr)
    {
        *rname = tolower(*ptr);
        ptr++;
        rname++;
    }

    *rname = '\0';
}


/*
 * FUNCTION NAME : DNS_Tick2Ttl
 *
 * PURPOSE:
 *      convert a absolute ticks to ttl ( in second )
 *
 * INPUT:
 *      UI32_T -- (absolute)
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      int -- in second
 *      0 -- if input tick lower than current tick.
 *
 * NOTES:
 *      none
 *
 */
UI32_T DNS_Tick2Ttl(UI32_T tick)
{
    UI32_T  ret;
    UI32_T  sys_tick = 0;

    sys_tick = SYSFUN_GetSysTick();

    if( tick > sys_tick)
    {
        ret = tick - sys_tick;
        return ret/DNS_SYS_CLK;
    }
    else
    {
        return 0;
    }
}


/*
 * FUNCTION NAME : DNS_Ttl2Ticks
 *
 * PURPOSE:
 *      compute ttl (in second) to absolute ticks.
 *
 * INPUT:
 *      int -- in second
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      UI32_T ttl in ticks.
 *      0 -- if ttl lower than 0
 *
 * NOTES:
 *      none
 */
UI32_T DNS_Ttl2Ticks(int ttl)
{
    if(ttl < 0)
        return 0;

/*isiah*/
/*  return tickGet()+ttl*DNS_SYS_CLK;   */
    return SYSFUN_GetSysTick()+ttl*DNS_SYS_CLK;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_CMM_IsAgeOut32
 *-------------------------------------------------------------------------
 * PURPOSE  : Check is age out or not, the time is 32 bits.
 * INPUT    : base_time      -- base time
 *          : expired_time   -- expired time
 *          : current_time   -- current time
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    :
 *            C: current time
 *            B: base time
 *            E: expired time
 *
 *            Case 1:
 *            ---(C1)---<B>---(C2)---<E>---(C3)---
 *            C1 and C3, age out
 *            Case 2:
 *            ---(C1)---<E>---(C2)---<B>---(C3)---
 *            C2, age out
 * ------------------------------------------------------------------------
 */
BOOL_T
DNS_CMM_IsAgeOut32(
    UI32_T base_time,
    UI32_T expired_time,
    UI32_T current_time)
{
    if ((I32_T)(expired_time - base_time) > 0)
    {
        if (((I32_T)(current_time - expired_time) > 0) ||
            ((I32_T)(base_time - current_time) > 0))
        {
            return TRUE;
        }
    }
    else
    {
        if (((I32_T)(current_time - expired_time) > 0) &&
            ((I32_T)(base_time - current_time) > 0))
        {
            return TRUE;
        }
    }

    return FALSE;
}

#endif /* #if (SYS_CPNT_DNS == TRUE) */

