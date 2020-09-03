/* MODULE NAME: dns_cmm.h
 * PURPOSE:
 *		This module provide functions receive TCP or UDP packets
 *       functions transmit TCP or UDP packets
 *
 *
 * NOTES:
 *
 * History:
 *       Date          -- Modifier,   Reason
 *       2002-09-06    -- Wiseway , created
 *   	2002-10-24    -- Wiseway   modified for convention
 *
 * Copyright(C)      Accton Corporation, 2002
 */

#ifndef DNS_CMM_H
#define DNS_CMM_H

/*the following macros is used for receive tcp message as states 		*/
#define DNS_RX_STAT_LEN1    	0   /* waiting 1st byte of length   	*/
#define DNS_RX_STAT_LEN2   		1   /* waiting 2nd byte of length   	*/
#define DNS_RX_STAT_DATA    	2   /* waiting message data      		*/
#define DNS_RX_STAT_FINL		3

#define DNS_FAILURE          	-1
#define DNS_SUCCESS          	0


typedef struct DNS_RxStruct_S {
    int		stat;					/* what data receiving now      	*/
    int		rx_buf_len;				/*The number of number of bytes to be received  */
    int		rx_len;					/* message have been received   	*/
    int		size;					/* size of this buffer          	*/
    I8_T	*pRx_Buf;				/*a pointer to a buf used fo receiving request from remote client*/
} DNS_RxRec_T, *DNS_Rx_PTR ;

typedef struct DNS_TxStruct_S {
    int		tx_buf_len;        		/* The number of number of bytes in tx_buf  */
    int		tx_len;             	/* data have been sent          	*/
    int		size  ;             	/* size of this bufefr          	*/
    I8_T	*pTx_Buf;           	/* a pointer to a buf used to store response from resolver      */
} DNS_TxRec_T, *DNS_Tx_PTR ;
/*****************************************************/

/* FUNCTION NAME : DNS_TxTCPMsg
 *
 * PURPOSE:
 *		This function is used for sending TCP message to remote client or server.
 *
 *
 * INPUT:
 *		int			-- the socket to send message
 *		DNS_Tx_PTR	-- the struct records the info for sending message.
 *
 * OUTPUT:
 *		DNS_Tx_PTR	-- the struct records the info after sending message.
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *		DNS_OK :success.
 *
 * NOTES:
 *		This function will be called by DNS_Proxy_Send  in case of TCP.
 */
int DNS_TxTCPMsg(int sock,
               		  DNS_Tx_PTR tcp_tx_ptr
               		 );


/* FUNCTION NAME : DNS_TxUDPMsg
 *
 * PURPOSE:
 *		This function is used for sending UDP message to remote client or server.
 *
 * INPUT:
 *		int               -- the socket to send message
 *		DNS_Tx_PTR        -- a pointer to a struct to transmit udp message
 *		struct sockaddr * -- recipient's address
 * OUTPUT:
 *		DNS_Tx_PTR        -- a pointer to a struct which records the information
 *							after sending udp message.
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 *
 * NOTES:
 *		This function will be called by DNS_Proxy_Send  in case of UDP.
 */
int DNS_TxUDPMsg(int sock,DNS_Tx_PTR udp_tx_ptr,struct sockaddr *to);


/* FUNCTION NAME : DNS_TCP_RxMsg
 * PURPOSE:
 *		This function receives tcp message.
 *
 * INPUT:
 *		int	       -- TCP socket to receive message from
 *		DNS_Rx_PTR -- a pointer to a struct to receive tcp message
 * OUTPUT:
 *      DNS_Rx_PTR -- a pointer to a struct who records the status for receive tcp message.
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 *
 * NOTES:
 *		This function will be called by DNS_PROXY_Daemon or DNS_Rsv_Daemon.
 */
int DNS_RxTCPMsg(int sock,
                     DNS_Rx_PTR tcp_rx_ptr
                     );

/* FUNCTION NAME : DNS_UDP_RxMsg
 *
 * PURPOSE:
 *		This function receives udp message.
 *
 * INPUT:
 *		int        -- UDP socket to receive message from
 *		DNS_Rx_PTR -- a pointer to a struct to receive udp message
 *
 * OUTPUT:
 *		struct sockaddr -- where to copy sender's addr.
 *		DNS_Rx_PTR      -- a pointer to a struct which records the status
 *							for receiving udp message
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 *
 * NOTES:
 *		This function will be called by DNS_PROXY_Daemon or DNS_Rsv_Daemon.
 */
int DNS_RxUDPMsg(int sock,
                   struct sockaddr *from,
                   DNS_Rx_PTR udp_rx_ptr
                   );


/*
 * FUNCTION NAME : DNS_Name2Lower
 *
 * PURPOSE:
 *		convert a domain name to lower case
 *
 * INPUT:
 *		const I8_T* -- up case or lower case
 *
 * OUTPUT:
 *		I8_T* -- all in lower case
 *
 * RETURN:
 *		none
 *
 * NOTES:
 *		none
 *
 */
void 	DNS_Name2Lower(const I8_T* name,I8_T* rname);

/*
 * FUNCTION NAME : DNS_Tick2Ttl
 *
 * PURPOSE:
 *		convert a absolute ticks to ttl ( in second )
 *
 * INPUT:
 *		UI32_T -- (absolute)
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		int -- in second
 *		0 -- if input tick lower than current tick.
 *
 * NOTES:
 *		none
 *
 */
UI32_T		DNS_Tick2Ttl(UI32_T tick);

/*
 * FUNCTION NAME : DNS_Ttl2Ticks
 *
 * PURPOSE:
 *		compute ttl (in second) to absolute ticks.
 *
 * INPUT:
 *		int -- in second
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		UI32_T ttl in ticks.
 *		0 -- if ttl lower than 0
 *
 * NOTES:
 *		none
 */
UI32_T	DNS_Ttl2Ticks(int ttl);


/* FUNCTION NAME : DNS_ProxyStop
 *
 * PURPOSE:
 *		This function disables the proxy function by
 *		1.terminate the proxy daemon,
 *		2.freed space dynamically for the proxy.
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none
 */
void DNS_PROXY_Stop(void);


/* FUNCTION NAME : DNS_PROXY_Init
 *
 * PURPOSE:
 *		This function initializes the dns proxy task variables,
 *		creates a interal socket used for comunnition between proxy and local resolver
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *	    none
 */
void DNS_PROXY_Init(void);



/* FUNCTION NAME : DNS_PROXY_CallBack
 *
 * PURPOSE:
 *		This function will copy resp_buf to proxy's space, send a packet to internal_sock notifying proxy to get response.
 *
 *
 * INPUT:
 *      int
 *		const I8_T
 *		UI32_T
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none;
 *
 * NOTES:
 *		This routine is called by DNS_PROXY_Daemon.
 */
void DNS_PROXY_CallBack(int proxy_req_id, const I8_T *resp_buf, UI32_T buf_len);

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
    UI32_T current_time
);

#endif  /* #ifndef DNS_CMM_H */
