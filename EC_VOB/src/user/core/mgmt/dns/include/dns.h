/* Module Name: dns.h
 *
 * Purpose:
 *		This package define dns packet data structure and interface for local dns request.
 *
 * Notes:
 *
 * History:
 *    09/06/02        -- simon zhou, Create
 *    11/08/02        -- Isiah, porting to ACP@2.0.
 *
 * Copyright(C)      Accton Corporation, 1999, 2000
 */

#ifndef DNS_H
#define DNS_H

/*isiah.2002-11-08*/
#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
/*#include <types.h>*/
/*#include <vxTypesOld.h>*/
/*#include <in.h>*/
#include <semLib.h>
#include <ioLib.h>
#include <taskLib.h>
#include <sockLib.h>
/*#include <netLib.h>*/
#include <netdb.h>		/*include this header file for hostent struct*/
#include <inetLib.h>	/*include this header file for address transfer function and such*/
#include <socket.h>
#include <sockfunc.h>
#include <tickLib.h>
#include <sysLib.h>
#endif /* end of #if 0 */

#include "sys_type.h"

typedef int 		(*FUNCPTR)();	   /* ptr to function returning int */
/*
 * Structures returned by network data base library.  All addresses are
 * supplied in host order, and returned in network order (suitable for
 * use in system calls).
 */
struct	hostent {
	char	*h_name;	/* official name of host */
	char	**h_aliases;	/* alias list */
	int	h_addrtype;	/* host address type */
	int	h_length;	/* length of address */
	char	**h_addr_list;	/* list of addresses from name server */
#define	h_addr	h_addr_list[0]	/* address, for backward compatiblity */
        unsigned int h_ttl;	/* Time to Live in Seconds for this entry */
};


enum DNS_ResService_E
{
	DNS_RES_SERVICE_RECURSIVE = 1,
	DNS_RES_SERVICE_ITERATIVE,
	DNS_RES_SERVICE_MIX
};

enum DNS_ResResetStatus_E
{
	DNS_RES_RESET_OTHER = 1,
	DNS_RES_RESET,
	DNS_RES_RESET_INITIAL,
	DNS_RES_RESET_RUNNING
};


#define ERROR -1

#ifdef __cplusplus
extern "C" {
#endif	/*	__cpluscplus	*/

/*
 * FUNCTION NAME : DNS_Init
 *
 * PURPOSE:
 *		DNS module entry
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
int DNS_Init(void);

#if 0
/*
 * FUNCTION NAME : gethostbyname
 *
 * PURPOSE:
 *		common dns interface
 *
 * INPUT:
 *		const I8_T* -- query name
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
struct hostent* gethostbyname(const I8_T* name);

/*
 * FUNCTION NAME : gethostbyaddr
 *
 * PURPOSE:
 *		common dns interface
 *
 * INPUT:
 *		const I8_T* -- ip address
 *	 	int -- sizeof addr
 *		int -- interface type
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		hostent* ghp
 *
 * NOTES:
 *		none
 *
 */
struct hostent* gethostbyaddr(const I8_T* addr, int len, int type);

/*
 * FUNCTION NAME : gethostbyname_r
 *
 * PURPOSE:
 *		gethostbyname,but won't free hostent space
 *
 * INPUT:
 *		const I8_T* -- query name
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		hostent* hp
 *
 * NOTES:
 *		none
 *
 */
struct hostent* gethostbyname_r(const I8_T* name);


/*
 * FUNCTION NAME : gethostbyaddr_r
 *
 * PURPOSE:
 *		gethostbyaddr,but won't free hostent space
 *
 * INPUT:
 *		const I8_T* -- ip address in string format
 *		int -- sizeof addr
 *		int -- type of interface
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		hostent* ghp
 *
 * NOTES:
 *		none
 *
 */
struct hostent* gethostbyaddr_r(const I8_T* addr, int len, int type);
#endif /*end of #if 0 */

#ifdef	__cpluscplus
}
#endif	/* __cpluscplus */

#endif	/* #ifndef DNS_H */
