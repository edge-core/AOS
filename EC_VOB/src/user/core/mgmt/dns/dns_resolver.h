/* Module Name: dns_resolver.h
 * Purpose: This package provide api for access DNS resolver.
 * Notes:
 * History:
 *    09/06/02        -- simon zhou, Create
 * Copyright(C)      Accton Corporation, 1999, 2000
 */

#ifndef DNS_RESOLVER_H
#define DNS_RESOLVER_H

#ifdef __cplusplus
extern "C" {
#endif	/*	__cpluscplus	*/

/* define followed for support all TCP & UDP queries */
#define DNS_UDP_QUERIES
#define DNS_TCP_QUERIES

#define DNS_HOSTENT_NUM 			5

/*isiah,MAXHOSTIPNUM is 8, why is DNS_HOSTENT_ALIAS_NUM 5 ?? :-(*/
/*#define DNS_HOSTENT_ALIAS_NUM 		5*/
#define DNS_HOSTENT_ALIAS_NUM 		8
#define DNS_HOSTENT_IP_NUM 			(DNS_HOSTENT_ALIAS_NUM+1)

#define DNS_HOSTENT_ALIAS_SIZE 		(DNS_HOSTENT_ALIAS_NUM*4)
#define DNS_HOSTENT_IP_SIZE 		(DNS_HOSTENT_IP_NUM*4)

#define DNS_HOSTENT_IP 				1
#define DNS_HOSTENT_CNAME 			2
#define DNS_HOSTENT_ALIAS 			3

#define DNS_TCP_UDP					0
#define DNS_UDP_ONLY				1

#define  DNS_RESOLVER_DEFAULT_PORT    53

typedef struct DNS_Sbelt_S
{
    UI8_T nsdomain[256];
    UI8_T cname[256];
    DNS_TYPE_SockAddr_T sin;    /* addr */
	int ra;						/* recursive available ?*/
	int priority;				/* priority to use this server */
	int status;					/* send? wait? 		*/
	unsigned int ttl;			/* for received NS cache */
	struct DNS_Sbelt_S* next_p;
}DNS_Sbelt_T,DNS_Slist_T;

typedef enum
{
	DNS_SLIST_STATUS_OK,
	DNS_SLIST_STATUS_READ,
	DNS_SLIST_STATUS_WRITE
}DNS_SbeltStatus_E;


enum DNS_Resolver_E
{
    DNS_SEARCH_CACHE= 1,
	DNS_SEND_QUERY=2
};

/*
 * FUNCTION NAME : DNS_RESOLVER_DebugShowHdr
 *
 * PURPOSE:
 *		show dns header
 *
 * INPUT:
 *		DNS_Hdr_T* -- dns pkt header
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		none
 *
 * NOTES:
 *		none
 *
 */
void	DNS_RESOLVER_DebugShowHdr(DNS_Hdr_T* hdr);

/*
 * FUNCTION NAME : DNS_RESOLVER_DebugShowRR
 *
 * PURPOSE:
 *		show rr
 *
 * INPUT:
 *		UI8_T* -- pointer to rr
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		none
 *
 * NOTES:
 *		none
 *
 */
void	DNS_RESOLVER_DebugShowRR(UI8_T* prr);

/*
 * FUNCTION NAME : DNS_RESOLVER_DebugShowHostent
 *
 * PURPOSE:
 *		show hostent
 *
 * INPUT:
 *		struct hostent* -- hostent to be show
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		none
 *
 * NOTES:
 *		none
 *
 */
void	DNS_RESOLVER_DebugShowHostent(struct hostent* hp);

/*
 * FUNCTION NAME : DNS_RESOLVER_DebugShowIp
 *
 * PURPOSE:
 *		DBG
 * INPUT:
 *		I8_T* --
 *		UI32_T --
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		none
 *
 * NOTES:
 *		none
 *
 */
void	DNS_RESOLVER_DebugShowIp(I8_T* show,UI32_T ip);

/*
 * FUNCTION NAME : DNS_ResolverDebugPrintIp
 *
 * PURPOSE:
 *		Debug. This function will transate IP addr with UI32_t type to string
 *		and displays it on console port.
 *
 * INPUT:
 *		UI32_T -- pip
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		none
 *
 * NOTES:
 *		This function is just for test.
 *
 */
void DNS_RESOLVER_DebugPrintIp(UI32_T pip);

/*
 * FUNCTION NAME : DNS_RESOLVER_DebugShowSendList
 *
 * PURPOSE:
 *		DBG
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		none
 * RETURN: 	none
 * NOTES:	none
 *
 */
void	DNS_RESOLVER_DebugShowSendList(void);

/*
 * FUNCTION NAME : DNS_MGR_ShowGlobal
 *
 * PURPOSE:
 *		DBG
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		none
 *
 * NOTES:
 *		none
 *
 */
void	DNS_RESOLVER_DebugShowGlobal(void);

/*
 * FUNCTION NAME : DNS_RESOLVER_DebugTest
 *
 * PURPOSE:
 *		DBG
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		none
 *
 * NOTES:
 *		none
 *
 */
void 	DNS_RESOLVER_DebugTest(void);

/*
 * FUNCTION NAME : DNS_RESOLVER_SbeltInit
 *
 * PURPOSE:
 *		initiate sbelt list
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		none
 *
 * NOTES:
 *		none
 *
 */
int		DNS_RESOLVER_SbeltInit(void);

/*
 * FUNCTION NAME : DNS_RESOLVER_Init
 *
 * PURPOSE:
 *		initiate dns resolver,include cache
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		none
 *
 * NOTES:
 *		none
 *
 */
int		DNS_RESOLVER_Init(void);

/*
 * FUNCTION NAME : DNS_RESOLVER_Disable
 *
 * PURPOSE:
 *		disable dns resolver
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		none
 *
 * NOTES:
 *		none
 *
 */
void 	DNS_RESOLVER_Disable(void);

/*
 * FUNCTION NAME : DNS_RESOLVER_Process
 *
 * PURPOSE:
 *		interface to dns proxy
 *
 * INPUT:
 *		int --
 *		UI8_T* -- proxy received data
 *		UI32_T -- full request pkt length
 *		UI8_T -- request transmit protocol type
 *		FUNCPTR -- callback function
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		none
 *
 * NOTES:
 *		none
 *
 */
int		DNS_RESOLVER_Process(int inter_id,UI8_T* data,UI32_T rlen,
							  FUNCPTR routine);



/*
 * FUNCTION NAME : DNS_RESOLVER_Task
 *
 * PURPOSE:
 *		dns resolver task
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		none
 *
 * NOTES:
 *		none
 *
 */
void DNS_RESOLVER_Task(void);

/*
 * FUNCTION NAME : DNS_RESOLVER_HostentFree
 *
 * PURPOSE:
 *		free hostent
 *
 * INPUT:
 *		struct hostent* -- hostent node to be free
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		none
 *
 * NOTES:
 *		none
 *
 */
void DNS_RESOLVER_HostentFree(struct hostent* hp);

/*
 * FUNCTION NAME : DNS_RESOLVER_SearchHosTable()
 *
 * PURPOSE:
 *      Search hostname whether exist in
 *      hostbyname haveing ip domain added.
 *
 * INPUT:
 *      UI8_T   *name   --  string of hostname.
 *      int family -- spec AF_INET(V4), AF_INET6(V6), AF_UNSPEC(V4/V6)
 *
 * OUTPUT:
 *      L_INET_AddrIp_T hostip[] --  host address.
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 *
 */
int DNS_RESOLVER_SearchHostTable(const UI8_T* name, UI32_T af_family, L_INET_AddrIp_T hostip_ar[]);

/*
 * FUNCTION NAME : DNS_RESOLVER_GetHostByName()
 *
 * PURPOSE:
 *          common dns interface
 *
 * INPUT:
 *          UI8_T   *hostname   --  string of hostname.
 *          int family          --  spec AF_INET(V4), AF_INET6(V6), AF_UNSPEC(V4/V6)
 *          type                --  DNS_SEND_QUERY or DNS_SEARCH_CACHE
 *
 * OUTPUT:
 *          L_INET_AddrIp_T hostip_ar[] --  host address,
 *                                          at most MAXHOSTIPNUM,
 *                                          "addrlen == 0" means no more.
 *
 * RETURN:
 *          DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      This function do two behavior depend on type.
 *      If DNS_SEARCH_CACHE type, search cache.
 *      If DNS_SEND_QUERY type, query to name-server.
 */
int DNS_RESOLVER_GetHostByName(const UI8_T* name, UI32_T af_family, UI32_T type, L_INET_AddrIp_T hostip_ar[]);

#ifdef	__cpluscplus
}
#endif	/* __cpluscplus */

#endif	/* #ifndef DNS_RESOLVER_H */
