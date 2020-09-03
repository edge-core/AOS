/* MODULE NAME: dns_pmgr.c
 * PURPOSE:
 *    PMGR implement for dns.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    11/28/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_DNS == TRUE)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_bld.h"
#include "dns_pmgr.h"
#include "dns_mgr.h"
#include "dns_om.h"
#include "sys_module.h"
#include "l_mm.h"
#include "sysfun.h"
#include "sys_module.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

static void DNS_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val);

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : DNS_PMGR_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initiate resource used in the calling process, means the process that use
 *    this pmgr functions should call this init.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    DNS_ERROR --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DNS_PMGR_InitiateProcessResource(void)
{
    if(SYSFUN_GetMsgQ(SYS_BLD_APP_PROTOCOL_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_AddDomainName
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : DNS_OK/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_AddDomainName(char *domain_name_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Config_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    strncpy(data_p->DnsIpDomainName, domain_name_p, sizeof(data_p->DnsIpDomainName)-1);
    data_p->DnsIpDomainName[sizeof(data_p->DnsIpDomainName)-1] = '\0';

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_ADD_DOMAIN_NAME,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

    return (int) DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_AddDomainNameToList
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_AddDomainNameToList(char *domain_name_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IpDomain_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_IpDomain_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    strncpy(data_p->DnsIpDomainName, domain_name_p, sizeof(data_p->DnsIpDomainName)-1);
    data_p->DnsIpDomainName[sizeof(data_p->DnsIpDomainName)-1] = '\0';

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_ADD_DOMAIN_NAME_TO_LIST,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IpDomain_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

    return (int) DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_AddNameServer
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : L_INET_AddrIp_T addr_p
 * OUTPUT	:
 * RETURN   :
 *              DNS_ERROR :failure,
 *              DNS_OK    :success.
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_AddNameServer(L_INET_AddrIp_T *addr_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_NAME_SERVER_Entry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_NAME_SERVER_Entry_T    *data_p;

    if(addr_p->addrlen == 0)
        return DNS_ERROR;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    memcpy(&data_p->serveraddr, addr_p, sizeof(L_INET_AddrIp_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_ADD_NAME_SERVER,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_NAME_SERVER_Entry_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

    return DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_ClearDnsCache
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_ClearDnsCache(void)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_CLEAR_DNS_CACHE,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

    return (int) DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_ClearHosts
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T DNS_PMGR_ClearHosts(void)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_CLEAR_HOSTS,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

    return (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_DeleteAllNameServer
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_DeleteAllNameServer(void)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_DELETE_ALL_NAME_SERVER,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

    return (int) DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_DeleteDomainName
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_DeleteDomainName(void)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_DELETE_DOMAIN_NAME,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

    return (int) DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_DeleteDomainNameFromList
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_DeleteDomainNameFromList(char *domain_name_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IpDomain_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_IpDomain_T   *data_p;

    if(domain_name_p == NULL)
        return DNS_ERROR;
    data_p = DNS_MGR_MSG_DATA(msg_p);
    strncpy(data_p->DnsIpDomainName, domain_name_p, sizeof(data_p->DnsIpDomainName)-1);
    data_p->DnsIpDomainName[sizeof(data_p->DnsIpDomainName)-1] = '\0';

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_DELETE_DOMAIN_NAME_FROM_LIST,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IpDomain_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

    return (int) DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_DeleteNameServer
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_DeleteNameServer(L_INET_AddrIp_T *addr_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_NAME_SERVER_Entry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_NAME_SERVER_Entry_T   *data_p;

    if(addr_p->addrlen == 0)
        return DNS_ERROR;

     data_p = DNS_MGR_MSG_DATA(msg_p);

    memcpy(&data_p->serveraddr, addr_p, sizeof(L_INET_AddrIp_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_DELETE_NAME_SERVER,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_NAME_SERVER_Entry_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

    return (int) DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_DisableDomainLookup
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T DNS_PMGR_DisableDomainLookup(void)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;


    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_DISABLE_DOMAIN_LOOKUP,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

    return (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_EnableDomainLookup
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T DNS_PMGR_EnableDomainLookup(void)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;


    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_ENABLE_DOMAIN_LOOKUP,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

    return (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetCacheEntryForSNMP
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T DNS_PMGR_GetCacheEntryForSNMP(I32_T index, DNS_CacheRecord_T *cache_entry)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Cache_Record_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Cache_Record_T   *data_p;

    if(cache_entry == NULL)
        return DNS_ERROR;
    data_p = DNS_MGR_MSG_DATA(msg_p);

    data_p->index = index;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_CACHE_ENTRY_FOR_SNMP,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Cache_Record_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Cache_Record_T),
                      (UI32_T)DNS_ERROR);

    memcpy(cache_entry,&data_p->cache, sizeof(DNS_CacheRecord_T));

    return (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDefaultDnsResConfigSbeltEntry
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDefaultDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResConfigSbeltEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_OM_IPCMsg_ResConfigSbeltEntry_T   *data_p;

    if(dns_res_config_sbelt_entry_t_p == NULL)
        return DNS_ERROR;
     data_p = DNS_MGR_MSG_DATA(msg_p);
    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_DEFAULT_DNS_RES_CONFIG_SBEL_ENTRY,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResConfigSbeltEntry_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResConfigSbeltEntry_T),
                      (UI32_T)DNS_ERROR);

    memcpy(dns_res_config_sbelt_entry_t_p, data_p, sizeof(DNS_ResConfigSbeltEntry_T));

    return (int) DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsAliasNameBySnmp
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsAliasNameBySnmp(I8_T *hostname_p, I8_T *aliasname_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_AliasName_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_AliasName_T   *data_p;

    if((hostname_p == NULL) || (aliasname_p == NULL))
        return DNS_ERROR;
    data_p = DNS_MGR_MSG_DATA(msg_p);

    strcpy((char *)hostname_p, (char *)data_p->hostname);
    strcpy((char *)aliasname_p, (char *)data_p->aliasname);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_ALIAS_NAME_BY_SNMP,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_AliasName_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_AliasName_T),
                      (UI32_T)DNS_ERROR);

    return (int) DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsHostEntry
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsHostEntry(int host_index_p,HostEntry_PTR dns_host_entry_t_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HostEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_HostEntry_T   *data_p;

    if(dns_host_entry_t_p == NULL)
        return DNS_ERROR;
     data_p = DNS_MGR_MSG_DATA(msg_p);
     data_p->index = host_index_p;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_HOST_ENTRY,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HostEntry_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HostEntry_T),
                      (UI32_T)DNS_ERROR);

    memcpy(dns_host_entry_t_p, &data_p->host_entry, sizeof(HostEntry_T));
    return (int) DNS_MGR_MSG_RETVAL(msg_p);
}

/* PENDING:
 * This seems to be a linked list which sorts name length first,
 * and then name value, possibly used as an SNMP index.
 * This code causes a PGRelief warning pgr0060 on the variable "prevHostent_P",
 * as it may be NULL but is used to point (->) to its content.
 * Because an SNMP index with a name of maximum 128 characters may exceed
 * the maximum sub-OID quantity of 128, SNMP now uses "dnsHostTable"
 * and "dnsHostAddrTable".  If this "...DnsHostEntryByNameAndIndex" code
 * for sure will not be used, it will be removed from the code.
 */
#if 0  /*!*/ /* PENDING: DNS_PMGR_*DnsHostEntryByNameAndIndex; suspected unused code */
/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsHostEntryByNameAndIndex
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsHostEntryByNameAndIndex(I8_T *hostname_p, UI32_T index, I8_T *hostaddr_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HostEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_HostEntry_T   *data_p;

    if((NULL==hostname_p)||(NULL==hostaddr_p))
	{
		return DNS_ERROR;
	}
     data_p = DNS_MGR_MSG_DATA(msg_p);
    data_p->index = index;
    strcpy((char *)data_p->hostname, (char *)hostname_p);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_HOST_ENTRY_BY_NAME_AND_INDEX,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HostEntry_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HostEntry_T),
                      (UI32_T)DNS_ERROR);

    strcpy((char *)hostaddr_p, (char *)data_p->hostaddr);

    return (int) DNS_MGR_MSG_RETVAL(msg_p);
}
#endif  /*!*/ /* 0; PENDING: DNS_PMGR_*DnsHostEntryByNameAndIndex; suspected unused code */

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsHostEntryBySnmp
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsHostEntryBySnmp(I8_T *hostname_p, I8_T *hostaddr_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_HostEntry_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    strcpy((char *)data_p->hostname , (char *)hostname_p);
    strcpy((char *)data_p->hostaddr, (char *)hostaddr_p);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_HOST_ENTRY_BY_SNMP,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HostEntry_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

    return (int) DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsIpDomain
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsIpDomain(char* ipdomain)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Config_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_IP_DOMAIN,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T),
                      (UI32_T)DNS_ERROR);

    if ( DNS_OK == (I16_T) DNS_MGR_MSG_RETVAL(msg_p))
    {
        strcpy(ipdomain, (char *)data_p->DnsIpDomainName);
        return DNS_OK;
    }
    return DNS_ERROR;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsLocalMaxRequests
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int  DNS_PMGR_GetDnsLocalMaxRequests(int *dns_config_local_max_requests_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Config_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_LOCAL_MAX_REQUESTS,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T),
                      (UI32_T)DNS_ERROR);

    *dns_config_local_max_requests_p = data_p->DnsMaxLocalRequests;
     return DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsResCacheBadCaches
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsResCacheBadCaches(int *dns_res_cache_bad_caches_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CacheConfig_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_CacheConfig_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_RES_BAD_CACHES,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CacheConfig_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CacheConfig_T),
                      (UI32_T)DNS_ERROR);

    *dns_res_cache_bad_caches_p = data_p->cache_bad_caches;
     return DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsResCacheGoodCaches
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsResCacheGoodCaches(int *dns_res_good_caches_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CacheConfig_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_CacheConfig_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_RES_GOOD_CACHES,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CacheConfig_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CacheConfig_T),
                      (UI32_T)DNS_ERROR);

    *dns_res_good_caches_p = data_p->cache_good_caches;
     return DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsResCacheMaxTTL
 * ------------------------------------------------------------------------|
 * FUNCTION :  This fuction will be called by snmp module.
 *  					If the resolver does not implement a TTL ceiling, the value
 *						 of this field should be zero.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsResCacheMaxTTL(UI32_T *dns_res_cache_max_ttl_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CacheConfig_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_CacheConfig_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_DNS_RES_CACHE_MAX_TTL,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CacheConfig_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CacheConfig_T),
                      (UI32_T)DNS_ERROR);

    *dns_res_cache_max_ttl_p = data_p->cache_max_ttl_32;
     return DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsResCacheStatus
 * ------------------------------------------------------------------------|
 * FUNCTION :  This function is used for getting the cache status.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsResCacheStatus(int *dns_res_cache_status_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CacheConfig_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_CacheConfig_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_DNS_RES_CACHE_STATUS,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CacheConfig_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CacheConfig_T),
                      (UI32_T)DNS_ERROR);

    *dns_res_cache_status_p = data_p->cache_status;
     return DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsResConfigMaxCnames
 * ------------------------------------------------------------------------|
 * FUNCTION :  Limit on how many CNAMEs the resolver should allow
 *  before deciding that there's a CNAME loop.  Zero means
 *  that resolver has no explicit CNAME limit.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsResConfigMaxCnames(int *dns_resconfig_max_cnames_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfig_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_ResConfig_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_DNS_RES_MAX_CNAMES,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfig_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfig_T),
                      (UI32_T)DNS_ERROR);

    *dns_resconfig_max_cnames_p = data_p->dnsResConfigMaxCnames;
     return DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsResConfigReset
 * ------------------------------------------------------------------------|
 * FUNCTION :  This function gets the time elapsed since it started.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsResConfigReset(int *config_reset_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CONFIG_RESET_TIME_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_CONFIG_RESET_TIME_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_DNS_RES_CONFIG_RESET,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfig_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfig_T),
                      (UI32_T)DNS_ERROR);

    *config_reset_p = data_p->config_reset_time;
     return DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsResConfigResetTime
 * ------------------------------------------------------------------------|
 * FUNCTION :  This function gets the time elapsed since it started.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsResConfigImplementIdent(UI8_T* dnsResConfigImplementIdent_p)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfig_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_ResConfig_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_DNS_RES_IMPLEMENTIDENT,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfig_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfig_T),
                      (UI32_T)DNS_ERROR);

    strcpy((char *)dnsResConfigImplementIdent_p, (char *)data_p->dnsResConfigImplementIdent);

    return DNS_MGR_MSG_RETVAL(msg_p);
}
int DNS_PMGR_GetDnsResConfigResetTime(int *config_reset_time_p)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CONFIG_RESET_TIME_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_CONFIG_RESET_TIME_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_DNS_RES_RESET_TIME,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CONFIG_RESET_TIME_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CONFIG_RESET_TIME_T),
                      (UI32_T)DNS_ERROR);

    *config_reset_time_p = data_p->config_reset_time;
     return DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsResConfigService
 * ------------------------------------------------------------------------|
 * FUNCTION : Get kind of DNS resolution service provided .
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsResConfigService(int *dns_res_config_service_p)
{
		UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfig_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_ResConfig_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_DNS_RES_SERVICE,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfig_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfig_T),
                      (UI32_T)DNS_ERROR);

    *dns_res_config_service_p = data_p->dnsResConfigService;
     return DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsResConfigSbeltEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :  This funciton get the specified DnsResConfigSbeltEntry according to the index.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_ResConfigSbeltEntry_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_DNS_RES_SBEL_ENTRY,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T),
                      (UI32_T)DNS_ERROR);

    if ( DNS_OK == (UI16_T) DNS_MGR_MSG_RETVAL(msg_p))
    {
        memcpy (dns_res_config_sbelt_entry_t_p, &data_p->data, sizeof (DNS_MGR_IPCMsg_ResConfigSbeltEntry_T));
        return TRUE;
    }

    return DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsResConfigUpTime
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsResConfigUpTime(int *config_up_time_p)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CONFIG_RESET_TIME_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_CONFIG_RESET_TIME_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_CONFIG_UP_TIME,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CONFIG_RESET_TIME_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CONFIG_RESET_TIME_T),
                      (UI32_T)DNS_ERROR);

    *config_up_time_p = data_p->config_reset_time;
     return DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsStatus
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsStatus(void)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_STATUS,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

     return DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsStatus
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T DNS_PMGR_SetDnsStatus(int status)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Config_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);
    data_p->DnsStatus = status;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_SET_STATUS,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

     return DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsResCacheMaxTTL
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsResCacheMaxTTL(int *dns_res_cache_max_ttl_p)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CacheConfig_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_CacheConfig_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);
    data_p->cache_max_ttl = *dns_res_cache_max_ttl_p;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_SET_RES_CACHE_MAX_TTL,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CacheConfig_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

     return (int)DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsResCacheStatus
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsResCacheStatus (int *dns_res_cache_status_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CacheConfig_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_CacheConfig_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);
    data_p->cache_status = *dns_res_cache_status_p;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_SET_RES_CACHE_STATUS,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CacheConfig_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

     return (int)DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsResConfigMaxCnames
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsResConfigMaxCnames(UI32_T dns_resconfig_max_cnames_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfig_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_ResConfig_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);
    data_p->dnsResConfigMaxCnames = dns_resconfig_max_cnames_p;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_SET_RES_MAX_CNAMES,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfig_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

     return (int)DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsResConfigReset
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsResConfigReset(int *config_reset_p)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfig_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_ResConfig_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);
    data_p->dnsResConfigReset = *config_reset_p;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_SET_RES_RESET,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfig_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

     return (int)DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsResConfigSbeltEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_ResConfigSbeltEntry_T    *data_p;

    if(dns_res_config_sbelt_entry_t_p == NULL)
    		return DNS_ERROR;

    data_p = DNS_MGR_MSG_DATA(msg_p);
    memcpy(&data_p->data,dns_res_config_sbelt_entry_t_p, sizeof(DNS_ResConfigSbeltEntry_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_SET_RES_SBELT_ENTRY,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

     return DNS_MGR_MSG_RETVAL(msg_p);
}


/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsResConfigSbeltName
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsResConfigSbeltName(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_ResConfigSbeltEntry_T    *data_p;

    if(dns_res_config_sbelt_entry_t_p == NULL)
    		return DNS_ERROR;

    data_p = DNS_MGR_MSG_DATA(msg_p);
    memcpy(&data_p->data,dns_res_config_sbelt_entry_t_p, sizeof(DNS_ResConfigSbeltEntry_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_SET_RES_SBELT_NAME,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

     return DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsResConfigSbeltRecursion
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsResConfigSbeltRecursion(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_ResConfigSbeltEntry_T    *data_p;

    if(dns_res_config_sbelt_entry_t_p == NULL)
    		return DNS_ERROR;

    data_p = DNS_MGR_MSG_DATA(msg_p);
    memcpy(&data_p->data,dns_res_config_sbelt_entry_t_p, sizeof(DNS_ResConfigSbeltEntry_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_SET_RES_SBELT_RECURSION,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

     return DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsResConfigSbeltStatus
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsResConfigSbeltStatus(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_ResConfigSbeltEntry_T    *data_p;

    if(dns_res_config_sbelt_entry_t_p == NULL)
    		return DNS_ERROR;

    data_p = DNS_MGR_MSG_DATA(msg_p);
    memcpy(&data_p->data,dns_res_config_sbelt_entry_t_p, sizeof(DNS_ResConfigSbeltEntry_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_SET_RES_SBELT_STATUS,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

     return DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsServConfigRecurs
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsServConfigRecurs(int *config_recurs_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ProxyConfig_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_ProxyConfig_T    *data_p;

    if(config_recurs_p == NULL)
    		return DNS_ERROR;

    data_p = DNS_MGR_MSG_DATA(msg_p);
    data_p->dnsServConfigRecurs = *config_recurs_p;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_SET_SERV_CONFIG_RECURS,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ProxyConfig_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

     return (int)DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsServConfigReset
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsServConfigReset(int *dns_serv_config_reset_p)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ProxyConfig_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_ProxyConfig_T    *data_p;

    if(dns_serv_config_reset_p == NULL)
    		return DNS_ERROR;

    data_p = DNS_MGR_MSG_DATA(msg_p);
    data_p->dnsServConfigReset = *dns_serv_config_reset_p;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_SET_SERV_CONFIG_RESET,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ProxyConfig_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

     return (int)DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextCacheEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T DNS_PMGR_GetNextCacheEntry(I32_T *index, DNS_CacheRecord_T *cache_entry)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Cache_Record_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Cache_Record_T    *data_p;

    if(cache_entry == NULL)
    		return FALSE;

    data_p = DNS_MGR_MSG_DATA(msg_p);
    data_p->index = *index;
    memcpy(&data_p->cache, cache_entry, sizeof(DNS_CacheRecord_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NEXT_CACHE_ENTRY,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Cache_Record_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Cache_Record_T),
                      (UI32_T)FALSE);

    if ( TRUE == (UI16_T) DNS_MGR_MSG_RETVAL(msg_p))
    {
         *index = data_p->index;
        memcpy(cache_entry, &data_p->cache, sizeof(DNS_CacheRecord_T));
    }
    return (BOOL_T)DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextCacheEntryForSNMP
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T DNS_PMGR_GetNextCacheEntryForSNMP(I32_T *index, DNS_CacheRecord_T *cache_entry)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Cache_Record_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Cache_Record_T    *data_p;

    if(cache_entry == NULL)
    	return DNS_ERROR;

    data_p = DNS_MGR_MSG_DATA(msg_p);
    data_p->index = *index;
    data_p->cache = *cache_entry;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NEXT_CACHE_ENTRY_FOR_SNMP,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Cache_Record_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Cache_Record_T),
                      (UI32_T)DNS_ERROR);

    *index = data_p->index;
    *cache_entry = data_p->cache;

     return (BOOL_T)DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextDnsAliasNameBySnmp
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetNextDnsAliasNameBySnmp(I8_T *hostname_p, I8_T *aliasname_p)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_AliasName_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_AliasName_T    *data_p;

    if((NULL==hostname_p)||(NULL==aliasname_p))
	{
		return DNS_ERROR;
	}

    data_p = DNS_MGR_MSG_DATA(msg_p);
    strcpy((char *)data_p->hostname, (char *)hostname_p);
    strcpy((char *)data_p->aliasname, (char *)aliasname_p);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NEXT_ALIAS_NAME_BY_SNMP,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_AliasName_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_AliasName_T),
                      (UI32_T)DNS_ERROR);

     return (int)DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextDnsHostEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetNextDnsHostEntry(int *host_index_p,HostEntry_PTR dns_host_entry_t_p)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_GetNextRunningHostEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_GetNextRunningHostEntry_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    if ((NULL==host_index_p)||(NULL==dns_host_entry_t_p)||((*host_index_p)<-1))
		return DNS_ERROR;

    data_p->index = *host_index_p;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NEXT_HOST_ENTRY,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_GetNextRunningHostEntry_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_GetNextRunningHostEntry_T),
                      (UI32_T)DNS_ERROR);

   if ( DNS_OK == (UI16_T) DNS_MGR_MSG_RETVAL(msg_p))
    {
        *host_index_p = data_p->index;
        memcpy (dns_host_entry_t_p, &data_p->host_entry, sizeof (HostEntry_T));
    }
	return  DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextDnsHostEntryByNameAndIndex
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetNextDnsHostEntryByNameAndIndex(char *hostname_p, I32_T *index_p, I8_T *hostaddr_p)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HostEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_HostEntry_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    if((NULL==hostname_p)||(NULL==hostaddr_p)||(NULL==index_p))
	{
		return DNS_ERROR;
	}
	memset(data_p, 0, sizeof(DNS_MGR_IPCMsg_HostEntry_T));
    data_p->index = *index_p;
    strncpy((char *)data_p->hostname, (char *)hostname_p, MAXHOSTNAMELEN); /*maggie liu, ES4827G-FLF-ZZ-00243*/

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NEXT_HOST_ENTRY_BY_NAME_INDEX,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HostEntry_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HostEntry_T),
                      (UI32_T)DNS_ERROR);

   if ( DNS_OK == (UI16_T) DNS_MGR_MSG_RETVAL(msg_p))
    {
        *index_p = data_p->index;
        strncpy((char *)hostname_p, (char *)data_p->hostname, MAXHOSTNAMELEN); /*maggie liu, ES4827G-FLF-ZZ-00243*/
        strncpy((char *)hostaddr_p, (char *)data_p->hostaddr, 20);
    }
 		return (UI16_T) DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextRunningDnsHostEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  DNS_PMGR_GetNextRunningDnsHostEntry(int *host_index_p,HostEntry_PTR dns_host_entry_t_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_GetNextRunningHostEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_GetNextRunningHostEntry_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    if ((NULL==host_index_p)||(NULL==dns_host_entry_t_p)||((*host_index_p)<-1))
		return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    data_p->index = *host_index_p;


    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NEXT_RUNNING_HOST_ENTRY,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_GetNextRunningHostEntry_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_GetNextRunningHostEntry_T),
                      (UI32_T)SYS_TYPE_GET_RUNNING_CFG_FAIL);

   if ( SYS_TYPE_GET_RUNNING_CFG_SUCCESS == (UI16_T) DNS_MGR_MSG_RETVAL(msg_p))
    {
        *host_index_p = data_p->index;
        memcpy (dns_host_entry_t_p, &data_p->host_entry, sizeof (HostEntry_T));
    }
 		return DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextRunningDomainNameList
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  DNS_PMGR_GetNextRunningDomainNameList(I8_T *dns_ip_domain_name)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Config_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    if (NULL==dns_ip_domain_name)
        return DNS_ERROR;

    strcpy((char *)data_p->DnsIpDomainName, (char *)dns_ip_domain_name);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NEXT_RUNNING_DOMAIN_NAME_LIST,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T),
                      (UI32_T)SYS_TYPE_GET_RUNNING_CFG_FAIL);
    strcpy((char *)dns_ip_domain_name, (char *)data_p->DnsIpDomainName);
    return (UI16_T) DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextRunningNameServerList
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  DNS_PMGR_GetNextRunningNameServerList(L_INET_AddrIp_T *ip_p)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_ResConfigSbeltEntry_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    if (NULL == ip_p)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    memcpy(&data_p->data.dnsResConfigSbeltAddr, ip_p, sizeof(L_INET_AddrIp_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NEXT_RUNNING_NAME_SERVER_LIST,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T),
                      (UI32_T)SYS_TYPE_GET_RUNNING_CFG_FAIL);

    memcpy(ip_p, &data_p->data.dnsResConfigSbeltAddr, sizeof(L_INET_AddrIp_T));

 	return  DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetRunningDnsIpDomain
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  DNS_PMGR_GetRunningDnsIpDomain(UI8_T *ipdomain)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Config_T    *data_p;

        data_p = DNS_MGR_MSG_DATA(msg_p);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_RUNNING_DNS_IP_DOMAIN,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T),
                      (UI32_T)SYS_TYPE_GET_RUNNING_CFG_FAIL);
    strcpy((char *)ipdomain, (char *)data_p->DnsIpDomainName);

 	return  DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetRunningDnsStatus
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  DNS_PMGR_GetRunningDnsStatus(UI32_T *state)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Config_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);
    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_RUNNING_DNS_STATUS,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T),
                      (UI32_T)SYS_TYPE_GET_RUNNING_CFG_FAIL);
    *state = data_p->DnsStatus;

	return  DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextDomainNameList
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T DNS_PMGR_GetNextDomainNameList(I8_T *dns_ip_domain_name)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Config_T    *data_p;

    if ( dns_ip_domain_name == NULL )
    {
        return DNS_ERROR;
    }
    data_p = DNS_MGR_MSG_DATA(msg_p);

    strcpy((char *)data_p->DnsIpDomainName, (char *)dns_ip_domain_name);
    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NEXT_DOMAIN_NAME_LIST,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T),
                      (UI32_T)SYS_TYPE_GET_RUNNING_CFG_FAIL);
    strcpy((char *)dns_ip_domain_name, (char *)data_p->DnsIpDomainName);

 	return  (BOOL_T)DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextDnsResConfigSbeltEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetNextDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_ResConfigSbeltEntry_T    *data_p;

    if ( dns_res_config_sbelt_entry_t_p == NULL )
    {
        return DNS_ERROR;
    }
    data_p = DNS_MGR_MSG_DATA(msg_p);

    memcpy(&data_p->data, dns_res_config_sbelt_entry_t_p, sizeof(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NEXT_RES_CONFIG_SBELT_ENTRY,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T),
                      (UI32_T)DNS_ERROR);
    memcpy(dns_res_config_sbelt_entry_t_p, &data_p->data, sizeof(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T));

 	return  (BOOL_T)DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_HostAdd
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_HostAdd(char *hostName, L_INET_AddrIp_T *hostAddr_p)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HOST_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_HOST_T    *data_p; /*maggie liu, 2008-11-20*/

    data_p = DNS_MGR_MSG_DATA(msg_p);
    strncpy(data_p->hostname, hostName, sizeof(data_p->hostname)-1);
    data_p->hostname[sizeof(data_p->hostname)-1] = '\0';

    memcpy(&data_p->hostaddr, hostAddr_p, sizeof(L_INET_AddrIp_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_HOST_ADD,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HOST_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

 	return  DNS_MGR_MSG_RETVAL(msg_p);
}

/* PENDING:
 * This seems to be a linked list which sorts name length first,
 * and then name value, possibly used as an SNMP index.
 * This code causes a PGRelief warning pgr0060 on the variable "prevHostent_P",
 * as it may be NULL but is used to point (->) to its content.
 * Because an SNMP index with a name of maximum 128 characters may exceed
 * the maximum sub-OID quantity of 128, SNMP now uses "dnsHostTable"
 * and "dnsHostAddrTable".  If this "...DnsHostEntryByNameAndIndex" code
 * for sure will not be used, it will be removed from the code.
 */
#if 0  /*!*/ /* PENDING: DNS_PMGR_*DnsHostEntryByNameAndIndex; suspected unused code */
/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsHostEntryByNameAndIndex
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsHostEntryByNameAndIndex(I8_T *hostname_p, UI32_T index, L_INET_AddrIp_T *ip_addr)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Set_Host_Entry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Set_Host_Entry_T    *data_p;

    if((NULL==hostname_p))
	{
		return DNS_ERROR;
	}
    data_p = DNS_MGR_MSG_DATA(msg_p);
    strcpy((char *)data_p->hostname, (char *)hostname_p);
    data_p->index = index;
    memcpy(&data_p->ip_addr, ip_addr, sizeof(L_INET_AddrIp_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_SET_HOST_ENTRY_BY_NAME_AND_INDEX,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);


 	return  DNS_MGR_MSG_RETVAL(msg_p);
}
#endif  /*!*/ /* 0; PENDING: DNS_PMGR_*DnsHostEntryByNameAndIndex; suspected unused code */

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextDnsResCounterByOpcodeEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetNextDnsResCounterByOpcodeEntry(DNS_ResCounterByOpcodeEntry_T *dns_res_counter_by_opcode_entry_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResCounterByOpcodeEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_ResCounterByOpcodeEntry_T    *data_p;

    if(NULL==dns_res_counter_by_opcode_entry_p)
	{
		return DNS_ERROR;
	}
	data_p = DNS_MGR_MSG_DATA(msg_p);

    memcpy(&data_p->data, dns_res_counter_by_opcode_entry_p, sizeof(DNS_ResCounterByOpcodeEntry_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NEXT_RES_COUNTER_BY_OPCODE_ENTRY,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResCounterByOpcodeEntry_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResCounterByOpcodeEntry_T),
                      (UI32_T)DNS_ERROR);
    memcpy(dns_res_counter_by_opcode_entry_p, &data_p->data, sizeof(DNS_ResCounterByOpcodeEntry_T));

 	return  DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextDnsResCounterByRcodeEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetNextDnsResCounterByRcodeEntry(DNS_ResCounterByRcodeEntry_T *dns_res_counter_by_rcode_entry_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResCounterByRcodeEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_ResCounterByRcodeEntry_T    *data_p;

    if(NULL==dns_res_counter_by_rcode_entry_p)
	{
		return DNS_ERROR;
	}
	data_p = DNS_MGR_MSG_DATA(msg_p);

    memcpy(&data_p->data, dns_res_counter_by_rcode_entry_p, sizeof(DNS_ResCounterByRcodeEntry_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NEXT_RES_COUNTER_BY_RCODE_ENTRY,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResCounterByRcodeEntry_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResCounterByRcodeEntry_T),
                      (UI32_T)DNS_ERROR);
    memcpy(dns_res_counter_by_rcode_entry_p, &data_p->data, sizeof(DNS_ResCounterByRcodeEntry_T));

 	return (int)DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextDnsServCounterEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetNextDnsServCounterEntry(DNS_ServCounterEntry_T *dns_serv_counter_entry_t_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ServCounterEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_ServCounterEntry_T    *data_p;

    if(NULL==dns_serv_counter_entry_t_p)
	{
		return DNS_ERROR;
	}
	data_p = DNS_MGR_MSG_DATA(msg_p);

    memcpy(&data_p->serverdata, dns_serv_counter_entry_t_p, sizeof(DNS_ServCounterEntry_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NEXT_DNS_SERVER_COUNTER_ENTRY,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ServCounterEntry_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ServCounterEntry_T),
                      (UI32_T)DNS_ERROR);

    if(DNS_MGR_MSG_RETVAL(msg_p)!= DNS_ERROR)
        memcpy(dns_serv_counter_entry_t_p, &data_p->serverdata, sizeof(DNS_ServCounterEntry_T));

 	return  DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextNameServerByIndex
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T DNS_PMGR_GetNextNameServerByIndex(UI32_T *index, L_INET_AddrIp_T *ip)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Next_Name_Server_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Next_Name_Server_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    data_p->index = *index;
    memcpy(&data_p->ip, ip, sizeof(L_INET_AddrIp_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NEXT_NAME_SERVER_BY_INDEX,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Next_Name_Server_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Next_Name_Server_T),
                      (UI32_T)DNS_ERROR);
    *index = data_p->index;
    memcpy(ip, &data_p->ip, sizeof(L_INET_AddrIp_T));

 	return  (BOOL_T)DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_HostDelete
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_HostDelete(char *name, L_INET_AddrIp_T *addr_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HOST_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_HOST_T    *data_p;

    if((name == NULL) || (addr_p == NULL))
        return DNS_ERROR;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    strncpy(data_p->hostname, name, sizeof(data_p->hostname)-1);
    data_p->hostname[sizeof(data_p->hostname)-1] = '\0';

    memcpy(&data_p->hostaddr, addr_p , sizeof(L_INET_AddrIp_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_HOST_DELETE,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HOST_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

 	return  (int)DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_HostNameDelete
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_HostNameDelete(char * name)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HOST_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_HOST_T    *data_p;

    if(name == NULL)
        return DNS_ERROR;
    data_p = DNS_MGR_MSG_DATA(msg_p);

    strncpy(data_p->hostname, name, sizeof(data_p->hostname)-1);
    data_p->hostname[sizeof(data_p->hostname)-1] = '\0';

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_HOST_NAME_DELETE,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HOST_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

 	return  (int)DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetNameServerByIndex
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetNameServerByIndex(UI32_T index, L_INET_AddrIp_T *ip_addr, BOOL_T is_add)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Next_Name_Server_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Next_Name_Server_T *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    data_p->index = index;
    memcpy(&data_p->ip, ip_addr, sizeof(L_INET_AddrIp_T));
    data_p->is_add=is_add;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_SET_NAME_SERVER_BY_INDEX,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Next_Name_Server_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

    return (int)DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_HostNameToIp
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
I32_T DNS_PMGR_HostNameToIp(UI8_T *hostname, UI32_T family, L_INET_AddrIp_T hostip_ar[])
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Host2Ip_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Host2Ip_T *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    /* added_by peter_yu, or DNS_RESOLVER_ExtractIp will return DNS_ERROR */
    memset(data_p, 0, sizeof(DNS_MGR_IPCMsg_Host2Ip_T));

    strncpy(data_p->hostname, (char *)hostname, sizeof(data_p->hostname)-1);
    data_p->hostname[ sizeof(data_p->hostname)-1 ] = '\0';
    data_p->family = family;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_HOSTNAMETOIP,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Host2Ip_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Host2Ip_T),
                      (UI32_T)DNS_ERROR);

    memcpy(hostip_ar, data_p->ipaddr, sizeof(data_p->ipaddr));

    return DNS_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME : DNS_PMGR_SetDnsResConfigSbeltPref
 * PURPOSE:
 *  This funciton set the specified DnsResConfigSbeltEntry according to the index.
 *
 *
 *
 * INPUT:
 *  DNS_ResConfigSbeltEntry_T * -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the value to be set
 *          INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *  none.
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *		DNS_OK :success
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
int DNS_PMGR_SetDnsResConfigSbeltPref(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_ResConfigSbeltEntry_T *data_p;

    if(dns_res_config_sbelt_entry_t_p == NULL)
        return DNS_ERROR;

    data_p = DNS_MGR_MSG_DATA(msg_p);
    memcpy(&data_p->data, dns_res_config_sbelt_entry_t_p, sizeof(DNS_ResConfigSbeltEntry_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_SET_RES_CONFIG_SBELT_PREF,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

    return (int)DNS_MGR_MSG_RETVAL(msg_p);
}

int DNS_PMGR_GetDnsServConfigReset(int *dns_serv_config_reset_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ProxyConfig_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_ProxyConfig_T *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_SERV_CONFIG_RESET,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ProxyConfig_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ProxyConfig_T), /*ES3628BT-FLF-ZZ-00514*/
                      (UI32_T)DNS_ERROR);

    if(DNS_MGR_MSG_RETVAL(msg_p) != DNS_ERROR)
        *dns_serv_config_reset_p = data_p->dnsServConfigReset;

    return (int)DNS_MGR_MSG_RETVAL(msg_p);
}

int DNS_PMGR_GetDnsResCounterByOpcodeEntry(DNS_ResCounterByOpcodeEntry_T *dns_res_counter_by_opcode_entry_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResCounterByOpcodeEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_ResCounterByOpcodeEntry_T *data_p;

    if(dns_res_counter_by_opcode_entry_p == NULL)
        return DNS_ERROR;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_RES_COUNTER_BY_OPCODE_ENTRY,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResCounterByOpcodeEntry_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResCounterByOpcodeEntry_T),
                      (UI32_T)DNS_ERROR);

    if(DNS_MGR_MSG_RETVAL(msg_p) != DNS_ERROR)
        memcpy(dns_res_counter_by_opcode_entry_p, &data_p->data, sizeof(DNS_ResCounterByOpcodeEntry_T));

    return (int)DNS_MGR_MSG_RETVAL(msg_p);
}

BOOL_T DNS_PMGR_CheckNameServerIp(L_INET_AddrIp_T *ipaddress_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CheckNameServerIP_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_CheckNameServerIP_T *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    memcpy(&data_p->serverip, ipaddress_p, sizeof(L_INET_AddrIp_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_CHECK_NAME_SERVER_IP,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CheckNameServerIP_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

    return (int)DNS_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME : DNS_PMGR_CreateDomainNameListEntry
 * PURPOSE: To create a new dnsDomainListEntry in dnsDomainListEntry table.
 * INPUT  : idx   -- index of dnsDomainListEntry (1-based)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 *          2. default name is 'domainname1', 'domainname2', ...
 */
BOOL_T DNS_PMGR_CreateDomainNameListEntry(UI32_T idx)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_IdxNameStr_T *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    data_p->index = idx;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_CREATE_DOMAIN_NAME_LIST_ENTRY,
                     msg_p,
                     DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T),
                     DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                     (UI32_T) FALSE);

    return (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME : DNS_PMGR_DestroyDomainNameListEntry
 * PURPOSE: To destroy a dnsDomainListEntry in dnsDomainListTable.
 * INPUT  : idx   -- index of dnsDomainListEntry
 *                   (1-based, key to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_PMGR_DestroyDomainNameListEntry(UI32_T idx)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_IdxNameStr_T *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    data_p->index = idx;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_DESTROY_DOMAIN_NAME_LIST_ENTRY,
                     msg_p,
                     DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T),
                     DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                     (UI32_T) FALSE);

    return (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME : DNS_MGR_SetDomainNameListEntry
 * PURPOSE: To modify a domain name in the dnsDomainListEntry table.
 * INPUT  : idx           -- index of entry.
 *                           (1-based, key to search the entry)
 *		    domain_name_p -- domain name content.
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 *          2. domain_name_p[0] == '\0' is not valid
 */
BOOL_T DNS_PMGR_SetDomainNameListEntry(UI32_T idx, I8_T *domain_name_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_IdxNameStr_T *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    data_p->index = idx;
    memcpy(data_p->name_str, domain_name_p, sizeof(data_p->name_str));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_SET_DOMAIN_NAME_LIST_ENTRY,
                     msg_p,
                     DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T),
                     DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                     (UI32_T) FALSE);

    return (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p);
}

#if 0
/* FUNCTION NAME : DNS_PMGR_GetDomainNameListByIndex
 * PURPOSE: To get entry from the dnsDomainListEntry table.
 * INPUT  : idx           -- index of dnsDomainListEntry (1-based)
 * OUTPUT : domain_name_p -- pointer to domain name content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_PMGR_GetDomainNameListByIndex(UI32_T idx, I8_T *domain_name_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_IdxNameStr_T *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    data_p->index = idx;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_DOMAIN_NAME_LIST_BY_INDEX,
                     msg_p,
                     DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T),
                     DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T),
                     (BOOL_T) FALSE);

    if (TRUE == (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p))
    {
        memcpy(domain_name_p, data_p->name_str, sizeof(data_p->name_str));
    }

    return (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME : DNS_PMGR_GetNextDomainNameListByIndex
 * PURPOSE: To get next entry from the dnsDomainListEntry table.
 * INPUT  : idx           -- index of dnsDomainListEntry (1-based)
 * OUTPUT : idx           -- next index of dnsDomainListEntry
 *          domain_name_p -- pointer to domain name content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_PMGR_GetNextDomainNameListByIndex(UI32_T *idx, I8_T *domain_name_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_IdxNameStr_T *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    data_p->index = idx;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NEXT_DOMAIN_NAME_LIST_BY_INDEX,
                     msg_p,
                     DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T),
                     DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T),
                     (BOOL_T) FALSE);

    if (TRUE == (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p))
    {
        memcpy(domain_name_p, data_p->name_str, sizeof(data_p->name_str));
    }

    return (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p);
}
#endif

/* FUNCTION NAME : DNS_PMGR_CreateDnsHostEntry
 * PURPOSE: To create a new host entry in dnsHostEntry table.
 * INPUT  : host_idx   -- index of dnsHostEntry
 *                        (1-based, key to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 *          2. default name is 'hostname1', 'hostname2', 'hostname3'...
 */
BOOL_T DNS_PMGR_CreateDnsHostEntry(UI32_T host_idx)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_IdxNameStr_T *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    data_p->index = host_idx;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_CREATE_DNS_HOST_ENTRY,
                     msg_p,
                     DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T),
                     DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                     (UI32_T) FALSE);

    return (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME : DNS_PMGR_DestroyDnsHostEntry
 * PURPOSE: To destroy a host entry in dnsHostEntry table.
 * INPUT  : host_idx   -- index of dnsHostEntry
 *                        (1-based, key to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_PMGR_DestroyDnsHostEntry(UI32_T host_idx)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_IdxNameStr_T *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    data_p->index = host_idx;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_DESTROY_DNS_HOST_ENTRY,
                     msg_p,
                     DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T),
                     DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                     (UI32_T) FALSE);

    return (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME : DNS_PMGR_SetDnsHostEntry
 * PURPOSE: To modify a hostname to the dnsHostEntry table.
 * INPUT  : host_idx   -- index of dnsHostEntry
 *                        (1-based, key to search the entry)
 *		    hostname_p -- pointer to hostname content
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 *          2. hostname_p[0] == '\0' is not valid
 */
BOOL_T DNS_PMGR_SetDnsHostEntry(UI32_T host_idx, I8_T *hostname_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_IdxNameStr_T *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    data_p->index = host_idx;
    memcpy(data_p->name_str, hostname_p, sizeof(data_p->name_str));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_SET_DNS_HOST_ENTRY,
                     msg_p,
                     DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T),
                     DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                     (UI32_T) FALSE);

    return (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME : DNS_PMGR_GetDnsHostEntryForSnmp
 * PURPOSE: To get entry from the dnsHostEntry table.
 * INPUT  : host_idx   -- index of dnsHostEntry
 *                        (1-based, key to search the entry)
 * OUTPUT : hostname_p -- pointer to hostname content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_PMGR_GetDnsHostEntryForSnmp(UI32_T host_idx, I8_T *hostname_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_IdxNameStr_T *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    data_p->index = host_idx;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_DNS_HOST_ENTRY_FOR_SNMP,
                     msg_p,
                     DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T),
                     DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T),
                     (UI32_T) FALSE);

    if (TRUE == (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p))
    {
        memcpy(hostname_p, data_p->name_str, sizeof(data_p->name_str));
    }

    return (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME : DNS_PMGR_GetNextDnsHostEntryForSnmp
 * PURPOSE: To get next entry from the dnsHostEntry table.
 * INPUT  : host_idx_p   -- index of dnsHostEntry
 *                          (1-based, 0 to get the first)
 * OUTPUT : host_idx_P   -- next index of dnsHostEntry
 *          hostname_p   -- pointer to hostname content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_PMGR_GetNextDnsHostEntryForSnmp(UI32_T *host_idx_p, I8_T *hostname_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_IdxNameStr_T *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    data_p->index = *host_idx_p;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NEXT_DNS_HOST_ENTRY_FOR_SNMP,
                     msg_p,
                     DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T),
                     DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T),
                     (UI32_T) FALSE);

    if (TRUE == (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p))
    {
        *host_idx_p = data_p->index;
        memcpy(hostname_p, data_p->name_str, sizeof(data_p->name_str));
    }

    return (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME : DNS_PMGR_SetDnsHostAddrEntry
 * PURPOSE: To add/delete an ip address to/from the dnsHostAddrEntry table.
 * INPUT  : host_idx  -- index of dnsHostAddrEntry to operate with (1-based)
 *		    addr_p    -- ip address content
 *                       (host_idx, addr_p are
 *                        keys to search the entry)
 *		    is_add    -- true to add/ false to delete
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. sorted by ip address type & content.
 *          2. only support ipv4 now.
 *          3. for SNMP.
 *          4. 0.0.0.0 is not a valid input.
 *          5. one addr can not be added to two different host entry.
 */
BOOL_T DNS_PMGR_SetDnsHostAddrEntry(
    UI32_T host_idx, L_INET_AddrIp_T *addr_p, BOOL_T is_add)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HostAddrEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_HostAddrEntry_T *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    data_p->index       = host_idx;
    data_p->is_add      = is_add;
    memcpy(&data_p->addr, addr_p, sizeof(data_p->addr));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_SET_DNS_HOST_IP_ENTRY,
                     msg_p,
                     DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HostAddrEntry_T),
                     DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                     (UI32_T) FALSE);

    return (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME : DNS_PMGR_GetDnsHostAddrEntry
 * PURPOSE: To get entry from the dnsHostAddrEntry table.
 * INPUT  : host_idx  -- index of dnsHostAddrEntry to get from (1-based)
 *		    addr_p    -- ip address content
 *                       (host_idx, addr_type, addr_p are
 *                        keys to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. sorted by ip address type & content.
 *          2. only support ipv4 now.
 *          3. for SNMP.
 */
BOOL_T DNS_PMGR_GetDnsHostAddrEntry(
    UI32_T host_idx, L_INET_AddrIp_T *addr_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HostAddrEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_HostAddrEntry_T *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    data_p->index       = host_idx;
    memcpy(&data_p->addr, addr_p, sizeof(data_p->addr));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_DNS_HOST_IP_ENTRY,
                     msg_p,
                     DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HostAddrEntry_T),
                     DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                     (UI32_T) FALSE);

    return (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME : DNS_PMGR_GetNextDnsHostAddrEntry
 * PURPOSE: To get next entry from the dnsHostAddrEntry table.
 * INPUT  : host_idx_p  -- index of dnsHostAddrEntry to get from
 *                         (1-based, 0 to get the first)
 *		    addr_p      -- ip address content
 *                         (host_idx_p, addr_type_p, addr_p are
 *                          keys to search the entry)
 * OUTPUT : host_idx_p  -- next index of dnsHostAddrEntry
 *          addr_p      -- next ip address content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. sorted by ip address type & content.
 *          2. only support ipv4 now.
 *          3. for SNMP.
 */
BOOL_T DNS_PMGR_GetNextDnsHostAddrEntry(
    UI32_T *host_idx_p, L_INET_AddrIp_T *addr_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HostAddrEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_HostAddrEntry_T *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    data_p->index       = *host_idx_p;
    memcpy(&data_p->addr, addr_p, sizeof(data_p->addr));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NEXT_DNS_HOST_IP_ENTRY,
                     msg_p,
                     DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HostAddrEntry_T),
                     DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HostAddrEntry_T),
                     (UI32_T) FALSE);

    if (TRUE == (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p))
    {
        *host_idx_p  = data_p->index;
        memcpy(addr_p, &data_p->addr, sizeof(data_p->addr));
    }

    return (BOOL_T) DNS_MGR_MSG_RETVAL(msg_p);
}


/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_DeleteDnsCacheRR
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_DeleteDnsCacheRR(I8_T * name)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HOST_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_HOST_T    *data_p;

    if(name == NULL)
        return DNS_ERROR;
    data_p = DNS_MGR_MSG_DATA(msg_p);

    strcpy(data_p->hostname, (char*)name);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_HOST_NAME_CACHE_DELETE,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HOST_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

 	return  (int)DNS_MGR_MSG_RETVAL(msg_p);
}

/* River@May 7, 2008, add nslookup mib */

int DNS_PMGR_GetNextLookupCtlTable(DNS_Nslookup_CTRL_T *CTRL_table)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_CTRL_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Nslookup_CTRL_T    *data_p;

    if ( CTRL_table == NULL )
    {
        return DNS_ERROR;
    }
    data_p = DNS_MGR_MSG_DATA(msg_p);

    memcpy(&data_p->data, CTRL_table, sizeof(DNS_MGR_IPCMsg_Nslookup_CTRL_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NEXT_LOOKUPCTL_TABLE,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_CTRL_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_CTRL_T),
                      (UI32_T)DNS_ERROR);
    memcpy(CTRL_table, &data_p->data, sizeof(DNS_MGR_IPCMsg_Nslookup_CTRL_T));

 	return  (BOOL_T)DNS_MGR_MSG_RETVAL(msg_p);

}

int DNS_PMGR_GetLookupCtlTable(DNS_Nslookup_CTRL_T *CTRL_table)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_CTRL_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Nslookup_CTRL_T    *data_p;

    if ( CTRL_table == NULL )
    {
        return DNS_ERROR;
    }
    data_p = DNS_MGR_MSG_DATA(msg_p);

    memcpy(&data_p->data, CTRL_table, sizeof(DNS_MGR_IPCMsg_Nslookup_CTRL_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_LOOKUPCTL_TABLE,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_CTRL_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_CTRL_T),
                      (UI32_T)DNS_ERROR);
    memcpy(CTRL_table, &data_p->data, sizeof(DNS_MGR_IPCMsg_Nslookup_CTRL_T));

 	return  (BOOL_T)DNS_MGR_MSG_RETVAL(msg_p);


}

int DNS_PMGR_SetDNSCtlTable_TargetAddressType(DNS_Nslookup_CTRL_T *CTRL_table)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_CTRL_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Nslookup_CTRL_T    *data_p;

    if ( CTRL_table == NULL )
    {
        return DNS_ERROR;
    }
    data_p = DNS_MGR_MSG_DATA(msg_p);

    memcpy(&data_p->data, CTRL_table, sizeof(DNS_MGR_IPCMsg_Nslookup_CTRL_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_SET_CTLTABLE_TARGETADDRESSTYPE,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_CTRL_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

 	return  (BOOL_T)DNS_MGR_MSG_RETVAL(msg_p);


}

int DNS_PMGR_SetDNSCtlTable_TargetAddress(DNS_Nslookup_CTRL_T *CTRL_table)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_CTRL_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Nslookup_CTRL_T    *data_p;

    if ( CTRL_table == NULL )
    {
        return DNS_ERROR;
    }
    data_p = DNS_MGR_MSG_DATA(msg_p);

    memcpy(&data_p->data, CTRL_table, sizeof(DNS_MGR_IPCMsg_Nslookup_CTRL_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_SET_CTLTABLE_TARGETADDRESS,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_CTRL_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

 	return  (BOOL_T)DNS_MGR_MSG_RETVAL(msg_p);

}

int DNS_PMGR_SetDNSCtlTable_RowStatus(DNS_Nslookup_CTRL_T *CTRL_table)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_CTRL_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Nslookup_CTRL_T    *data_p;

    if ( CTRL_table == NULL )
    {
        return DNS_ERROR;
    }
    data_p = DNS_MGR_MSG_DATA(msg_p);

    memcpy(&data_p->data, CTRL_table, sizeof(DNS_MGR_IPCMsg_Nslookup_CTRL_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_SET_CTLTABLE_ROWSTATUS,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_CTRL_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

 	return  (BOOL_T)DNS_MGR_MSG_RETVAL(msg_p);

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_PMGR_CreateSystemNslookupCtlEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Create nslookup control entry.
 * INPUT    : ctl_entry_p  -- nslookup control entry
 * OUTPUT   : ctl_index_p  -- 0-based nslookup control entry index
 * RETUEN   : DNS_OK/DNS_ERROR
 * NOTES    : If create entry success, it will start nslookup.
 * ------------------------------------------------------------------------
 */
int
DNS_PMGR_CreateSystemNslookupCtlEntry(
    DNS_Nslookup_CTRL_T *ctl_entry_p,
    UI32_T *ctl_index_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CreateNslookupCtlEntry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_CreateNslookupCtlEntry_T    *data_p;

    if (NULL == ctl_entry_p)
    {
        return DNS_ERROR;
    }

    data_p = DNS_MGR_MSG_DATA(msg_p);

    memcpy(&data_p->ctl_entry, ctl_entry_p, sizeof(data_p->ctl_entry));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_CREATE_SYSTEM_NSLOOKUP_CTL_ENTRY,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CreateNslookupCtlEntry_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CreateNslookupCtlEntry_T),
                      (UI32_T)DNS_ERROR);

    *ctl_index_p = data_p->ctl_index;

    return  (BOOL_T)DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_PMGR_GetNslookupCtlEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get nslookup control entry by index.
 * INPUT    : ctl_index       -- 0-based nslookup control entry index
 * OUTPUT   : ctl_entry_p     -- nslookup control entry
 * RETUEN   : DNS_OK/DNS_ERROR
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
int
DNS_PMGR_GetNslookupCtlEntryByIndex(
    UI32_T ctl_index,
    DNS_Nslookup_CTRL_T *ctl_entry_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_GetNslookupCtlEntryByIndex_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_GetNslookupCtlEntryByIndex_T    *data_p;

    if (NULL == ctl_entry_p)
    {
        return DNS_ERROR;
    }

    data_p = DNS_MGR_MSG_DATA(msg_p);

    data_p->ctl_index = ctl_index;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NSLOOKUP_CTL_ENTRY_BY_INDEX,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_GetNslookupCtlEntryByIndex_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_GetNslookupCtlEntryByIndex_T),
                      (UI32_T)DNS_ERROR);

    memcpy(ctl_entry_p, &data_p->ctl_entry, sizeof(DNS_Nslookup_CTRL_T));

    return  (BOOL_T)DNS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_PMGR_GetNslookupResultEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get nslookup result entry by index.
 * INPUT    : ctl_index        -- 0-based nslookup control entry index
 *            result_index     -- 0-based nslookup result entry index
 * OUTPUT   : results_entry_p  -- nslookup result entry
 * RETUEN   : DNS_OK/DNS_ERROR
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
int
DNS_PMGR_GetNslookupResultEntryByIndex(
    UI32_T ctl_index,
    UI32_T result_index,
    DNS_Nslookup_Result_T *results_entry_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_GetNslookupResultEntryByIndex_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_GetNslookupResultEntryByIndex_T    *data_p;

    if (NULL == results_entry_p)
    {
        return DNS_ERROR;
    }

    data_p = DNS_MGR_MSG_DATA(msg_p);

    data_p->ctl_index = ctl_index;
    data_p->result_index = result_index;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NSLOOKUP_RESULT_ENTRY_BY_INDEX,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_GetNslookupResultEntryByIndex_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_GetNslookupResultEntryByIndex_T),
                      (UI32_T)DNS_ERROR);

    memcpy(results_entry_p, &data_p->result_entry, sizeof(DNS_Nslookup_Result_T));

    return  (BOOL_T)DNS_MGR_MSG_RETVAL(msg_p);
}

int DNS_PMGR_GetNextLookupResultTable(DNS_Nslookup_Result_T *Result_table)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_RESULT_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Nslookup_RESULT_T    *data_p;

    if ( Result_table == NULL )
    {
        return DNS_ERROR;
    }
    data_p = DNS_MGR_MSG_DATA(msg_p);

    memcpy(&data_p->data, Result_table, sizeof(DNS_MGR_IPCMsg_Nslookup_RESULT_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NEXT_LOOKUPRESULT_TABLE,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_RESULT_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_RESULT_T),
                      (UI32_T)DNS_ERROR);

    memcpy(Result_table, &data_p->data, sizeof(DNS_MGR_IPCMsg_Nslookup_RESULT_T));

 	return  (BOOL_T)DNS_MGR_MSG_RETVAL(msg_p);

}

int DNS_PMGR_GetLookupResultTable(DNS_Nslookup_Result_T *Result_table)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_RESULT_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_Nslookup_RESULT_T    *data_p;

    if ( Result_table == NULL )
    {
        return DNS_ERROR;
    }
    data_p = DNS_MGR_MSG_DATA(msg_p);

    memcpy(&data_p->data, Result_table, sizeof(DNS_MGR_IPCMsg_Nslookup_RESULT_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_LOOKUPRESULT_TABLE,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_RESULT_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_RESULT_T),
                      (UI32_T)DNS_ERROR);

    memcpy(Result_table, &data_p->data, sizeof(DNS_MGR_IPCMsg_Nslookup_RESULT_T));

 	return  (BOOL_T)DNS_MGR_MSG_RETVAL(msg_p);


}

int DNS_PMGR_Nslookup_DeleteEntry(UI32_T index)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_DELETE_NSLOOKUP_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_DELETE_NSLOOKUP_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    data_p->index = index;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_DELETE_ENTRY,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_DELETE_NSLOOKUP_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

 	return  (BOOL_T)DNS_MGR_MSG_RETVAL(msg_p);

}

BOOL_T DNS_PMGR_GetNslookupTimeOut(UI32_T *timeout, UI32_T index)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_GET_TIMEOUT_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_GET_TIMEOUT_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    data_p->index = index;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_TIMEOUT,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_GET_TIMEOUT_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_GET_TIMEOUT_T),
                      (UI32_T)DNS_ERROR);

    *timeout = data_p->timeout;
 	return  (BOOL_T)DNS_MGR_MSG_RETVAL(msg_p);

}

BOOL_T NS_PMGR_GetNslookupPurgeTime(UI32_T *purge_time)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_PURGE_TIME_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_PURGE_TIME_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_PURGE_TIME,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_PURGE_TIME_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_PURGE_TIME_T),
                      (UI32_T)DNS_ERROR);

    *purge_time = data_p->purge_time;
 	return  (BOOL_T)DNS_MGR_MSG_RETVAL(msg_p);

}
BOOL_T DNS_PMGR_SetNslookupPurgeTime(UI32_T purge_time)
{
	UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_PURGE_TIME_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_PURGE_TIME_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    data_p->purge_time = purge_time;

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_SET_PURGE_TIME,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_PURGE_TIME_T),
                      DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)DNS_ERROR);

 	return  (BOOL_T)DNS_MGR_MSG_RETVAL(msg_p);

}

BOOL_T DNS_PMGR_GetNameServerList(L_INET_AddrIp_T *serveraddr_p)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_NAME_SERVER_Entry_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    DNS_MGR_IPCMsg_NAME_SERVER_Entry_T    *data_p;

    data_p = DNS_MGR_MSG_DATA(msg_p);

    memcpy(&data_p->serveraddr, serveraddr_p, sizeof(L_INET_AddrIp_T));

    DNS_PMGR_SendMsg(DNS_MGR_IPC_CMD_GET_NAME_SERVER_LIST,
                      msg_p,
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_NAME_SERVER_Entry_T),
                      DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_NAME_SERVER_Entry_T),
                      (UI32_T)FALSE);

    memcpy(serveraddr_p, &data_p->serveraddr, sizeof(L_INET_AddrIp_T));

 	return  (BOOL_T)DNS_MGR_MSG_RETVAL(msg_p);
}

/* LOCAL SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_PMGR_SendMsg
 *-------------------------------------------------------------------------
 * PURPOSE : To send an IPC message.
 * INPUT   : cmd       - the DNS message command.
 *           msg_p     - the buffer of the IPC message.
 *           req_size  - the size of DNS request message.
 *           res_size  - the size of DNS response message.
 *           ret_val   - the return value to set when IPC is failed.
 * OUTPUT  : msg_p     - the response message.
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
static void DNS_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val)
{
    UI32_T ret;

    msg_p->cmd = SYS_MODULE_DNS;
    msg_p->msg_size = req_size;

    DNS_MGR_MSG_CMD(msg_p) = cmd;

    ret = SYSFUN_SendRequestMsg(ipcmsgq_handle,  /* pgr0695, return value of statement block in macro */
                                msg_p,
                                SYSFUN_TIMEOUT_WAIT_FOREVER,
                                SYSFUN_SYSTEM_EVENT_IPCMSG,
                                res_size,
                                msg_p);

    /* If IPC is failed, set return value as ret_val.
     */

    if (ret != SYSFUN_OK)
    {
    DNS_MGR_MSG_RETVAL(msg_p) = ret_val;
    }
}
/* End of this file */

#endif /* #if (SYS_CPNT_DNS == TRUE) */

