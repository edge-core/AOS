/* MODULE NAME: dns_hostlib.h
 * PURPOSE:
 *		This module provide functions to deal with local host table
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

#ifndef DNS_HOSTLIB_H
#define DNS_HOSTLIB_H

#include "dns_type.h"

#ifndef	MAX
#define MIN(x, y)		((x) < (y) ? (x) : (y))
#endif

#ifndef	MIN
#define MAX(x, y)		((x) > (y) ? (x) : (y))
#endif



/* FUNCTION NAME : DNS_HOSTLIB_HostTblInit
 *
 * PURPOSE:
 *		Initialize th local host table. Set hostList with two default host entries.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		none
 *
 * NOTES:
 *		none
 */
void DNS_HOSTLIB_HostTblInit (void);

/* FUNCTION NAME :DNS_OM_HostAdd
 *
 * PURPOSE:
 *		add a host to the host table
 *		This routine adds a host name to the local host table.
 *		The host table has one entry per Internet address.
 *		More than one name may be used for an address.
 *		Additional host names are added as aliases.
 *
 * INPUT:
 *		I8_T * -- host name
 *		I8_T * -- host addr in standard Internet format
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		DNS_OK, or DNS_ERROR if the host table is full, the host name is already entered,
 *		the parameters are invalid, or memory is insufficient.
 *
 * NOTES:
 *		none
 */
int DNS_OM_HostAdd(I8_T *hostName,
                    I8_T *hostAddr);


/* FUNCTION NAME : DNS_HOSTLIB_HostShow
 *
 * PURPOSE:
 *		This routine prints a list of remote hosts, along with their Internet addresses and aliases
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		none
 *
 * NOTES:
 *		 none
 */
void DNS_HOSTLIB_HostShow(void);



/* FUNCTION NAME : DNS_HOSTLIB_HostDelete
 *
 * PURPOSE:
 *		This routine deletes a <name,ip address> from the local host table.  If <name> is
 *		a host name, only the ip address is deleted.  If <name> is a host name alias,
 *		the alias and ip address are deleted.
 *
 * INPUT:
 *		I8_T * -- host name or alias
 *		I8_T * -- host addr in standard Internet format
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		DNS_OK, of DNS_ERROR if not find the entry.
 *
 * NOTES:
 *		Afeter this operation, this cases may occurs:
 *		only hostname or hostname&alias exists ,the ip addresses have all been deleted.
 *		none
 */
int DNS_HOSTLIB_HostDelete(char *name, L_INET_AddrIp_T *addr_p);

/* FUNCTION NAME : DNS_HOSTLIB_HostGetByAddr
 *
 * PURPOSE:
 *		This routine finds the host name by its Internet address and copies it to
 *		<name>.  The buffer <name> should be preallocated with (MAXHOSTNAMELEN + 1)
 *		bytes of memory and is NULL-terminated unless insufficient space is
 *		provided.
 *		This routine does not look for aliases.  Host names are limited to
 *		MAXHOSTNAMELEN (from hostLib.h) characters.
 *
 * INPUT:
 *		const I8_T * -- inet address of host.
 *
 * OUTPUT:
 *		I8_T *       -- buffer to hold name..
 *
 * RETURN:
 *		DNS_OK, or DNS_ERROR if buffer is invalid or the host is unknown
 *
 * NOTES:
 *		 none
 */
int DNS_HOSTLIB_HostGetByAddr(const I8_T * addr, I8_T *name);


/* FUNCTION NAME : DNS_HOSTLIB_HostTblSearchByAddr
 *
 * PURPOSE:
 *		This routine finds the host name by its Internet address and copies it to
 *		<name>.  The buffer <name> should be preallocated with (MAXHOSTNAMELEN + 1)
 *		bytes of memory and is NULL-terminated unless insufficient space is
 *		provided.
 *
 * INPUT:
 *		struct in_addr -- inet address of host
 *
 * OUTPUT:
 *		I8_T * --  buffer to hold name
 *
 * RETURN:
 *		DNS_OK , or DNS_ERROR if not find.
 *
 * NOTES:
 *		none
 */
int DNS_HOSTLIB_HostTblSearchByAddr(L_INET_AddrIp_T *netAddr_p, I8_T *name);

/* FUNCTION NAME : DNS_HOSTLIB_HostGetByName
 *
 * PURPOSE:
 *		This routine returns a list of the Internet address of a host that has
 *		been added to the host table by DNS_MGR_HostAdd(), and store it in the
 *		addr[].
 *
 * INPUT:
 *		const I8_T *   -- host name or alias.
 *
 * OUTPUT:
 *		struct in_addr --Ip addr array where the searched IP addrs will be put.
 *
 * RETURN:
 *		int
 *
 * NOTES:
 *		none
 */
int DNS_HOSTLIB_HostGetByName(const char *name, L_INET_AddrIp_T addr_ar[]);

/* FUNCTION NAME : DNS_HOSTLIB_HostTblSearchByName
 *
 * PURPOSE:
 *		This routine returns a list of the Internet address of a host that has
 *		been added to the host table by DNS_MGR_HostAdd(), and store it in the
 *		addr[].
 *
 * INPUT:
 *		const I8_T *   -- host name to be searched.
 *		struct in_addr -- Ip addr to be searched.
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		DNS_OK, or DNS_ERROR if not find the name int the hostList.
 *
 * NOTES:
 *		This function is called by called by hostGetByName.
 */
int DNS_HOSTLIB_HostTblSearchByName(const char *name, L_INET_AddrIp_T addr_ar[], int *addr_count_p);

/* FUNCTION NAME : DNS_HOSTLIB_SetHostName
 *
 * PURPOSE:
 *		This routine sets the target machine's symbolic name, which can be used
 *		for identification.
 *
 * INPUT:
 *		const I8_T * -- machine name
 *		int          -- length of name
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		DNS_OK, or DNS_ERROR if nameLen is larger then MAXHOSTNAMELEN
 *
 * NOTES:
 *		none
 */
int DNS_HOSTLIB_SetHostName(const I8_T *name, int nameLen);

/* FUNCTION NAME :	DNS_HOSTLIB_GetHostName
 *
 * PURPOSE:
 *		This routine gets the target machine's symbolic name, which can be used
 *		for identification.
 *
 * INPUT:
 *		int    -- length of name
 *
 * OUTPUT:
 *		I8_T * -- buffer to hold machine name .
 *
 * RETURN:
 *		DNS_OK, or DNS_ERROR if nameLen is smaller then the lenth of targetName.
 *
 * NOTES:
 *		 none
 */
int DNS_HOSTLIB_GetHostName(I8_T *name, int nameLen);

/* FUNCTION NAME : DNS_HOSTLIB_HostsClear
 *
 * PURPOSE:
 *		This routine clears all the entries in the hostList .
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		none
 *
 * NOTES:
 *		none
 */
void DNS_HOSTLIB_HostsClear(void);


/* FUNCTION NAME : DNS_HOSTLIB_HostNameDelete
 *
 * PURPOSE:
 *		This routine deletes a host name from the local host table.  If <name> is
 *		a host name, the host entry is deleted.  If <name> is a host name alias,
 *		only the alias is deleted.
 *
 * INPUT:
 *		I8_T * -- host name or alias
 *
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		DNS_OK, of DNS_ERROR if not find the entry.
 *
 * NOTES:
 *		none
 */
int DNS_HOSTLIB_HostNameDelete(char * name)	;


/* FUNCTION NAME : DNS_HOSTLIB_AddHostEntry
 * PURPOSE:
 *		This funciton add the specified struct to the local host table.
 *
 *
 * INPUT:
 *		HostEntry_PTR -- a pointer to a struct to be added to the local host table.
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		 This function will be called by OM module.
 */
int DNS_HOSTLIB_AddHostEntry(HostEntry_PTR host_entry_t_p);


/* FUNCTION NAME : DNS_HOSTLIB_GetNextHostEntry
 * PURPOSE:
 *		This funciton get the specified  according to the index.
 *
 *
 * INPUT:
 *		int *host_index_p -- previous active index of HostEntry_T struct.
 *
 * OUTPUT:
 *		int *host_index_p -- current active index of HostEntry_T struct.
 *		HostEntry_PTR *  -- a pointer to a HostEntry_T variable to store the returned value.
 *
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		This function will be called by OM module.
 *		The initial value is -1.
 */
int DNS_HOSTLIB_GetNextHostEntry(int *host_index_p,HostEntry_PTR dns_host_entry_t_p);



/* FUNCTION NAME : DNS_HOSTLIB_GetHostEntryBySnmp
 * PURPOSE:
 *		This funciton get the specified  according to the index.
 *
 *
 * INPUT:
 *		I8_T   *hostname_p   --  current active host name.
 *      I8_T   *hostaddr_p   --  current active host addr in standard Internet format
 *
 * OUTPUT:
 *		none.
 *
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *      This function will be called by DNS_MGR_GetDnsHostEntryBySnmp
 */
int DNS_HOSTLIB_GetHostEntryBySnmp(char *hostname_p, char *hostaddr_p);



/* FUNCTION NAME : DNS_HOSTLIB_GetNextHostEntryBySnmp
 * PURPOSE:
 *		This funciton get next the specified  according to the index.
 *
 *
 * INPUT:
 *		I8_T   *hostname_p   --  current active host name.
 *      I8_T   *hostaddr_p   --  current active host addr in standard Internet format.
 *
 * OUTPUT:
 *		I8_T   *hostname_p   --  next active host name.
 *      I8_T   *hostaddr_p   --  next active host addr in standard Internet format.
 *
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *      This function will be called by DNS_MGR_GetDnsNextHostEntryBySnmp
 */
int DNS_HOSTLIB_GetNextHostEntryBySnmp(UI8_T *hostname_p, UI8_T *hostaddr_p);



/* FUNCTION NAME : DNS_HOSTLIB_GetAliasNameBySnmp
 * PURPOSE:
 *		This funciton get the specified  according to the index.
 *
 *
 * INPUT:
 *		I8_T   *hostname_p   --  current active host name.
 *      I8_T   *aliasname_p  --  current active alias name.
 *
 * OUTPUT:
 *		none.
 *
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *      This function will be called by DNS_MGR_GetDnsAliasNameBySnmp
 */
int DNS_HOSTLIB_GetAliasNameBySnmp(I8_T *hostname_p, I8_T *aliasname_p);



/* FUNCTION NAME : DNS_HOSTLIB_GetNextAliasNameBySnmp
 * PURPOSE:
 *		This funciton get next the specified  according to the index.
 *
 *
 * INPUT:
 *		I8_T   *hostname_p   --  current active host name.
 *      I8_T   *aliasname_p  --  current active alias name.
 *
 * OUTPUT:
 *		I8_T   *hostname_p   --  next active host name.
 *      I8_T   *aliasname_p  --  next active alias name.
 *
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *      This function will be called by DNS_MGR_GetNextDnsAliasNameBySnmp
 */
int DNS_HOSTLIB_GetNextAliasNameBySnmp(I8_T *hostname_p, I8_T *aliasname_p);


/* PENDING:
 * This seems to be a linked list which sorts name length first,
 * and then name value, possibly used by SNMP.
 * This code causes a PGRelief warning pgr0060 on the variable "prevHostent_P",
 * as it may be NULL but is used to point (->) to its content.
 * If this code for sure will not be used, it will be removed from the code.
 */
#if 0  /*!*/ /* PENDING: DNS_HOSTLIB_*DnsHostEntryByNameAndIndex; suspected unused code */
/* FUNCTION NAME : DNS_HOSTLIB_GetNextDnsHostEntryByNameAndIndex
 * PURPOSE:
 *		This funciton get next the specified  according to the index.
 *
 *
 * INPUT:
 *		I8_T    *hostname_p --  current active host name.
 *      I32_T   *index_p    --  current active index of host addr for host name.
 *      I8_T    *hostaddr_p --  current active host addr in standard Internet format.
 *
 * OUTPUT:
 *		I8_T    *hostname_p --  next active host name.
 *      I32_T   *index_p    --  next active index of host addr for host name.
 *      I8_T    *hostaddr_p --  next active host addr in standard Internet format.
 *
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *      This function will be called by DNS_MGR_GetNextDnsHostEntryByNameAndIndex
 */
int DNS_HOSTLIB_GetNextDnsHostEntryByNameAndIndex(char *hostname_p, I32_T *index_p, char *hostaddr_p);

/* FUNCTION NAME : DNS_HOSTLIB_SetDnsHostEntryByNameAndIndex
 * PURPOSE:
 *  This function adds a IP address to the host table list by host name & index.
 *
 *
 * INPUT:
 *	*hostname_p --  host name.
 *      index       --  index of host addr for host name.
 *      ip_addr     --  ip addr will be added as a host name.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *      This function will be called by DNS_MGR_SetDnsHostEntryByNameAndIndex.
 */
int DNS_HOSTLIB_SetDnsHostEntryByNameAndIndex(char *hostname_p, UI32_T index, L_INET_AddrIp_T *ip_addr);

/*maggie liu, ES4827G-FLF-ZZ-00243*/
/* FUNCTION NAME : DNS_HOSTLIB_DeleteDnsHostEntryByNameAndIndex
 * PURPOSE:
 *  This function delete a IP address to the host table list by host name & index.
 *
 *
 * INPUT:
 *	*hostname_p --  host name.
 *      index       --  index of host addr for host name.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *      This function will be called by DNS_MGR_SetDnsHostEntryByNameAndIndex.
 */
int DNS_HOSTLIB_DeleteDnsHostEntryByNameAndIndex(char *hostname_p, UI32_T index);

/* FUNCTION NAME : DNS_HOSTLIB_GetDnsHostEntryByNameAndIndex
 * PURPOSE:
 *  This function get a IP address to the host table list by host name & index.
 *
 *
 * INPUT:
 *		I8_T    *hostname_p --  host name.
 *      UI32_T  index       --  index of host addr for host name.
 *
 * OUTPUT:
 *      I8_T    *hostaddr_p --  host addr in standard Internet format.
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *      This function will be called by DNS_MGR_SetDnsHostEntryByNameAndIndex.
 */
int DNS_HOSTLIB_GetDnsHostEntryByNameAndIndex(char *hostname_p, UI32_T index, char *hostaddr_p);
#endif  /*!*/ /* 0; PENDING: DNS_HOSTLIB_*DnsHostEntryByNameAndIndex; suspected unused code */


/* FUNCTION NAME :DNS_HOSTLIB_HostAdd
 *
 * PURPOSE:
 *		add a host to the host table
 *		This routine adds a host name to the local host table.
 *		The host table has one entry per Internet address.
 *		More than one name may be used for an address.
 *		Additional host names are added as aliases.
 *
 * INPUT:
 *		I8_T * -- host name
 *		I8_T * -- host addr in standard Internet format
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		DNS_OK, or DNS_ERROR if the host table is full, the host name is already entered,
 *		the parameters are invalid, or memory is insufficient.
 *
 * NOTES:
 *		none
 */
int DNS_HOSTLIB_HostAdd(char *hostName, L_INET_AddrIp_T *hostAddr_p);


/* FUNCTION NAME : DNS_HOSTLIB_CreateDnsHostEntry
 * PURPOSE: To create a new host entry in dnsHostEntry table.
 * INPUT  : host_idx   -- index of dnsHostEntry (1-based)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 *          2. default name is 'hostname1', 'hostname2', 'hostname3'...
 */
BOOL_T DNS_HOSTLIB_CreateDnsHostEntry(UI32_T host_idx);

/* FUNCTION NAME : DNS_HOSTLIB_DestroyDnsHostEntry
 * PURPOSE: To destroy a host entry in dnsHostEntry table.
 * INPUT  : host_idx   -- index of dnsHostEntry
 *                        (1-based, key to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_HOSTLIB_DestroyDnsHostEntry(UI32_T host_idx);

/* FUNCTION NAME : DNS_HOSTLIB_SetDnsHostEntry
 * PURPOSE: To modify a hostname to the dnsHostEntry table.
 * INPUT  : host_idx   -- index of dnsHostEntry
 *                        (1-based, key to search the entry)
 *          hostname_p -- pointer to hostname content
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 *          2. hostname_p[0] == '\0' is not valid
 */
BOOL_T DNS_HOSTLIB_SetDnsHostEntry(UI32_T host_idx, char *hostname_p);

/* FUNCTION NAME : DNS_HOSTLIB_GetDnsHostEntry
 * PURPOSE: To get entry from the dnsHostEntry table.
 * INPUT  : host_idx   -- index of dnsHostEntry
 *                        (1-based, key to search the entry)
 * OUTPUT : hostname_p -- pointer to hostname content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_HOSTLIB_GetDnsHostEntry(UI32_T host_idx, char *hostname_p);

/* FUNCTION NAME : DNS_HOSTLIB_GetNextDnsHostEntry
 * PURPOSE: To get next entry from the dnsHostEntry table.
 * INPUT  : host_idx_p   -- index of dnsHostEntry
 *                          (1-based, 0 to get the first,
 *                           key to search the entry)
 * OUTPUT : host_idx_p   -- next index of dnsHostEntry
 *          hostname_p -- pointer to hostname content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_HOSTLIB_GetNextDnsHostEntry(UI32_T *host_idx_p, char *hostname_p);

/* FUNCTION NAME : DNS_HOSTLIB_SetDnsHostAddrEntry
 * PURPOSE: To add/delete an ip address to/from the dnsHostAddrEntry table.
 * INPUT  : host_idx  -- index of dnsHostAddrEntry to operate with (1-based)
 *		    addr_p    -- ip address content
 *		    is_add    -- true to add/ false to delete
 *                       (host_idx, addr_p are
 *                        keys to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. sorted by ip address type & content.
 *          2. only support ipv4 now.
 *          3. for SNMP.
 *          4. 0.0.0.0 is not a valid input.
 *          5. one addr can not be added to two different host entry.
 */
BOOL_T DNS_HOSTLIB_SetDnsHostAddrEntry(
    UI32_T host_idx, L_INET_AddrIp_T *addr_p, BOOL_T is_add);

/* FUNCTION NAME : DNS_HOSTLIB_GetDnsHostAddrEntry
 * PURPOSE: To get entry from the dnsHostAddrEntry table.
 * INPUT  : host_idx  -- index of dnsHostAddrEntry to get from (1-based)
 *		    addr_p    -- ip address content
 *                       (host_idx, addr_p are
 *                        keys to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. sorted by ip address type & content.
 *          2. only support ipv4 now.
 *          3. for SNMP.
 */
BOOL_T DNS_HOSTLIB_GetDnsHostAddrEntry(
    UI32_T host_idx, L_INET_AddrIp_T *addr_p);

/* FUNCTION NAME : DNS_HOSTLIB_GetNextDnsHostAddrEntry
 * PURPOSE: To get next entry from the dnsHostAddrEntry table.
 * INPUT  : host_idx_p  -- index of dnsHostAddrEntry to get from
 *                         (1-based, 0 to get the first)
 *		    addr_p      -- ip address content
 *                         (host_idx_p, addr_p are
 *                          keys to search the entry)
 * OUTPUT : host_idx_p  -- next index of dnsHostAddrEntry
 *          addr_p      -- next ip address content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. sorted by ip address type & content.
 *          2. only support ipv4 now.
 *          3. for SNMP.
 */
BOOL_T DNS_HOSTLIB_GetNextDnsHostAddrEntry(
    UI32_T *host_idx_p, L_INET_AddrIp_T *addr_p);

#endif 	/* #ifndef DNS_HOSTLIB_H */
