/* MODULE NAME: dns_mgr.h
 * PURPOSE:
 *   Initialize the resource and provide some functions for the dns module.
 *   Functions provide service for snmp module and CLI is included in this file.
 *
 * NOTES:
 *
 * History:
 *       Date		-- Modifier,	Reason
 *       2002-09-06	-- Simon zhou	Created
 *       2002-10-27	-- Wiseway,	modified for convention.
 *
 * Copyright(C)      Accton Corporation, 2002
 */

#ifndef DNS_MGR_H
#define DNS_MGR_H

#include "dns_type.h"
#include "sysfun.h"
#include "l_inet.h"

#define DNS_MGR_IPC_RESULT_OK    (0)
#define DNS_MGR_IPC_RESULT_FAIL  (-1)

/* The commands for IPC message.
 */
enum {
    DNS_MGR_IPC_CMD_ADD_DOMAIN_NAME=1,
    DNS_MGR_IPC_CMD_ADD_DOMAIN_NAME_TO_LIST,
    DNS_MGR_IPC_CMD_ADD_NAME_SERVER,
    DNS_MGR_IPC_CMD_CLEAR_DNS_CACHE,
    DNS_MGR_IPC_CMD_CLEAR_HOSTS,
    DNS_MGR_IPC_CMD_DELETE_ALL_NAME_SERVER,
    DNS_MGR_IPC_CMD_DELETE_DOMAIN_NAME,
    DNS_MGR_IPC_CMD_DELETE_DOMAIN_NAME_FROM_LIST,
    DNS_MGR_IPC_CMD_DELETE_NAME_SERVER,
    DNS_MGR_IPC_CMD_DISABLE_DOMAIN_LOOKUP,
    DNS_MGR_IPC_CMD_ENABLE_DOMAIN_LOOKUP,
    DNS_MGR_IPC_CMD_GET_CACHE_ENTRY_FOR_SNMP,
    DNS_MGR_IPC_CMD_GET_DEFAULT_DNS_RES_CONFIG_SBEL_ENTRY,
    DNS_MGR_IPC_CMD_GET_ALIAS_NAME_BY_SNMP,
    DNS_MGR_IPC_CMD_GET_HOST_ENTRY,

#if 0  /*!*/ /* PENDING: DNS_MGR_*DnsHostEntryByNameAndIndex; suspected unused code */
    DNS_MGR_IPC_CMD_GET_HOST_ENTRY_BY_NAME_AND_INDEX,
#endif

    DNS_MGR_IPC_CMD_GET_HOST_ENTRY_BY_SNMP,
    DNS_MGR_IPC_CMD_GET_IP_DOMAIN,
    DNS_MGR_IPC_CMD_GET_LOCAL_MAX_REQUESTS,
    DNS_MGR_IPC_CMD_GET_RES_BAD_CACHES,
    DNS_MGR_IPC_CMD_GET_RES_GOOD_CACHES,
    DNS_MGR_IPC_CMD_GET_DNS_RES_CACHE_MAX_TTL,
    DNS_MGR_IPC_CMD_GET_DNS_RES_CACHE_STATUS,
    DNS_MGR_IPC_CMD_GET_DNS_RES_MAX_CNAMES,
    DNS_MGR_IPC_CMD_GET_DNS_RES_IMPLEMENTIDENT,
    DNS_MGR_IPC_CMD_GET_DNS_RES_CONFIG_RESET,
    DNS_MGR_IPC_CMD_GET_DNS_RES_RESET_TIME,
    DNS_MGR_IPC_CMD_GET_DNS_RES_SERVICE,
    DNS_MGR_IPC_CMD_GET_DNS_RES_SBEL_ENTRY,
    DNS_MGR_IPC_CMD_GET_CONFIG_UP_TIME,
    DNS_MGR_IPC_CMD_GET_STATUS,
    DNS_MGR_IPC_CMD_SET_STATUS,
    DNS_MGR_IPC_CMD_SET_RES_CACHE_MAX_TTL,
    DNS_MGR_IPC_CMD_SET_RES_CACHE_STATUS,
    DNS_MGR_IPC_CMD_SET_RES_MAX_CNAMES,
    DNS_MGR_IPC_CMD_SET_RES_RESET,
    DNS_MGR_IPC_CMD_SET_RES_SBELT_ENTRY,
    DNS_MGR_IPC_CMD_SET_RES_SBELT_NAME,
    DNS_MGR_IPC_CMD_SET_RES_SBELT_PREF,
    DNS_MGR_IPC_CMD_SET_RES_SBELT_RECURSION,
    DNS_MGR_IPC_CMD_SET_RES_SBELT_STATUS,
    DNS_MGR_IPC_CMD_SET_SERV_CONFIG_RECURS,
    DNS_MGR_IPC_CMD_SET_SERV_CONFIG_RESET,
    DNS_MGR_IPC_CMD_GET_NEXT_CACHE_ENTRY,
    DNS_MGR_IPC_CMD_GET_NEXT_CACHE_ENTRY_FOR_SNMP,
    DNS_MGR_IPC_CMD_GET_NEXT_ALIAS_NAME_BY_SNMP,
    DNS_MGR_IPC_CMD_GET_NEXT_HOST_ENTRY,
    DNS_MGR_IPC_CMD_GET_NEXT_HOST_ENTRY_BY_NAME_INDEX,
    DNS_MGR_IPC_CMD_GET_NEXT_RUNNING_HOST_ENTRY,
    DNS_MGR_IPC_CMD_GET_NEXT_RUNNING_DOMAIN_NAME_LIST,
    DNS_MGR_IPC_CMD_GET_NEXT_RUNNING_NAME_SERVER_LIST,
    DNS_MGR_IPC_CMD_GET_RUNNING_DNS_IP_DOMAIN,
    DNS_MGR_IPC_CMD_GET_RUNNING_DNS_STATUS,
    DNS_MGR_IPC_CMD_GET_NEXT_DOMAIN_NAME_LIST,
    DNS_MGR_IPC_CMD_GET_NEXT_RES_CONFIG_SBELT_ENTRY,
    DNS_MGR_IPC_CMD_HOST_ADD,

#if 0  /*!*/ /* PENDING: DNS_MGR_*DnsHostEntryByNameAndIndex; suspected unused code */
    DNS_MGR_IPC_CMD_SET_HOST_ENTRY_BY_NAME_AND_INDEX,
#endif

    DNS_MGR_IPC_CMD_GET_RES_COUNTER_BY_OPCODE_ENTRY,
    DNS_MGR_IPC_CMD_GET_NEXT_RES_COUNTER_BY_OPCODE_ENTRY,
    DNS_MGR_IPC_CMD_GET_NEXT_RES_COUNTER_BY_RCODE_ENTRY,
    DNS_MGR_IPC_CMD_GET_NEXT_DNS_SERVER_COUNTER_ENTRY,
    DNS_MGR_IPC_CMD_GET_NEXT_NAME_SERVER_BY_INDEX,
    DNS_MGR_IPC_CMD_HOST_DELETE,
    DNS_MGR_IPC_CMD_HOST_NAME_DELETE,
    DNS_MGR_IPC_CMD_SET_NAME_SERVER_BY_INDEX,
    DNS_MGR_IPC_CMD_HOSTNAMETOIP,
    DNS_MGR_IPC_CMD_SET_RES_CONFIG_SBELT_PREF,
    DNS_MGR_IPC_CMD_GET_SERV_CONFIG_RESET,
    DNS_MGR_IPC_CMD_CHECK_NAME_SERVER_IP,
    DNS_MGR_IPC_CMD_GET_NEXT_LOOKUPCTL_TABLE,
    DNS_MGR_IPC_CMD_GET_LOOKUPCTL_TABLE,
    DNS_MGR_IPC_CMD_SET_CTLTABLE_TARGETADDRESSTYPE,
    DNS_MGR_IPC_CMD_SET_CTLTABLE_TARGETADDRESS,
    DNS_MGR_IPC_CMD_SET_CTLTABLE_ROWSTATUS,
    DNS_MGR_IPC_CMD_CREATE_SYSTEM_NSLOOKUP_CTL_ENTRY,
    DNS_MGR_IPC_CMD_GET_NSLOOKUP_CTL_ENTRY_BY_INDEX,
    DNS_MGR_IPC_CMD_GET_NSLOOKUP_RESULT_ENTRY_BY_INDEX,
    DNS_MGR_IPC_CMD_GET_NEXT_LOOKUPRESULT_TABLE,
    DNS_MGR_IPC_CMD_GET_LOOKUPRESULT_TABLE,
    DNS_MGR_IPC_CMD_DELETE_ENTRY,
    DNS_MGR_IPC_CMD_GET_TIMEOUT,
    DNS_MGR_IPC_CMD_GET_PURGE_TIME,
    DNS_MGR_IPC_CMD_SET_PURGE_TIME,
    DNS_MGR_IPC_CMD_CREATE_DOMAIN_NAME_LIST_ENTRY,
    DNS_MGR_IPC_CMD_DESTROY_DOMAIN_NAME_LIST_ENTRY,
    DNS_MGR_IPC_CMD_SET_DOMAIN_NAME_LIST_ENTRY,
    DNS_MGR_IPC_CMD_CREATE_DNS_HOST_ENTRY,
    DNS_MGR_IPC_CMD_DESTROY_DNS_HOST_ENTRY,
    DNS_MGR_IPC_CMD_SET_DNS_HOST_ENTRY,
    DNS_MGR_IPC_CMD_GET_DNS_HOST_ENTRY_FOR_SNMP,
    DNS_MGR_IPC_CMD_GET_NEXT_DNS_HOST_ENTRY_FOR_SNMP,
    DNS_MGR_IPC_CMD_SET_DNS_HOST_IP_ENTRY,
    DNS_MGR_IPC_CMD_GET_DNS_HOST_IP_ENTRY,
    DNS_MGR_IPC_CMD_GET_NEXT_DNS_HOST_IP_ENTRY,
    DNS_MGR_IPC_CMD_HOST_NAME_CACHE_DELETE,
    DNS_MGR_IPC_CMD_GET_NAME_SERVER_LIST,
};


/* MACRO DEFINITIONS
 */
/*-------------------------------------------------------------------------
 * MACRO NAME - DNS_MGR_GET_MSGBUFSIZE
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of DNS message that contains the specified
 *           type of data.
 * INPUT   : type_name - the type name of data for the message.
 * OUTPUT  : None
 * RETURN  : The size of DNS message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define DNS_MGR_GET_MSGBUFSIZE(type_name) \
    ((uintptr_t) & ((DNS_MGR_IPCMsg_T *)0)->data + sizeof(type_name))

/*-------------------------------------------------------------------------
 * MACRO NAME - DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of DNS message that has no data block.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The size of DNS message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
    sizeof(DNS_MGR_IPCMsg_Type_T)

/*-------------------------------------------------------------------------
 * MACRO NAME - DNS_MGR_MSG_CMD
 *              DNS_MGR_MSG_RETVAL
 *-------------------------------------------------------------------------
 * PURPOSE : Get the DNS command/return value of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The DNS command/return value; it's allowed to be used as lvalue.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define DNS_MGR_MSG_CMD(msg_p)    (((DNS_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define DNS_MGR_MSG_RETVAL(msg_p) (((DNS_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.result)
#define DNS_MGR_MSG_RETVAL_INT(msg_p) (((DNS_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.result_int)

/*-------------------------------------------------------------------------
 * MACRO NAME - DNS_MGR_MSG_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the data block of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The pointer of the data block.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define DNS_MGR_MSG_DATA(msg_p)   ((void *)&((DNS_MGR_IPCMsg_T *)(msg_p)->msg_buf)->data)



/* DATA TYPE DECLARATIONS
 *
 *  Message declarations for IPC.
 */
typedef union
{
    UI32_T  cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
    UI32_T  result; /* for response */
    int         result_int;
} DNS_MGR_IPCMsg_Type_T;


typedef struct
{
I8_T		dnsResConfigImplementIdent[MAXSIZE_dnsResConfigImplementIdent+1]; /*The implementation identification string for the
                                                                      resolver software in use on the system, for example; `RES-2.1'"*/
  int		dnsResConfigService;   /* Kind of DNS resolution service provided:
                                          recursiveOnly(1) indicates a stub resolver.
                                          iterativeOnly(2) indicates a normal full service resolver.
                                          recursiveAndIterative(3) indicates a full-service
                                          resolver which performs a mix of recursive and iterative queries*/
  int		dnsResConfigMaxCnames; /*Limit on how many CNAMEs the resolver should allow
                                         before deciding that there's a CNAME loop.  Zero means
                                         that resolver has no explicit CNAME limit.*/
  UI32_T	dnsResConfigUpTime;    /*If the resolver has a persistent state (e.g., a
                                         process), this value will be the time elapsed since it
                                         started.  For software without persistant state, this value will be 0  */
  UI32_T	dnsResConfigResetTime;
  int		dnsResConfigReset;     /*Status/action object to reinitialize any persistant
                                         resolver state.  When set to reset(2), any persistant
                                         resolver state (such as a process) is reinitialized as if
                                         the resolver had just been started.  This value will
                                         never be returned by a read operation.  When read, one of
                                         the following values will be returned:
                                         other(1) - resolver in some unknown state;
                                         initializing(3) - resolver (re)initializing;
                                         running(4) - resolver currently running*/
 int		dnsResConfigQueryOrder;  /* DNS_QUERY_LOCAL_FIRST:Query static host table first;
                                              DNS_QUERY_DNS_FIRST: Query DNS server first
                                              DNS_QUERY_ DNS_ONLY: Query DNS server only
                                              default: DNS_QUERY_LOCAL_FIRST*/
} DNS_MGR_IPCMsg_ResConfig_T;

typedef struct
{
    DNS_ResConfigSbeltEntry_T    data;
} DNS_MGR_IPCMsg_ResConfigSbeltEntry_T;

typedef struct
{
    UI8_T	dnsServConfigImplementIdent[MAXSIZE_dnsServConfigImplementIdent+1]; /*The implementation identification string for the DNS
                                                                              server software in use on the system, for example;
                                                                               "DNS-2.1'" */

    int		dnsServConfigRecurs;  /*This represents the recursion services offered by this
                                          name server.  The values that can be read or written are:
                                          available(1) - performs recursion on requests from clients.
                                          restricted(2) - recursion is performed on requests only
                                          from certain clients, for example; clients on an access control list.
                                          It is not supported in our system.
                                          unavailable(3) - recursion is not available.  */
   UI32_T	dnsServConfigUpTime;  /*If the server has a persistent state (e.g., a process),
                                          this value will be the time elapsed since it started.
                                          For software without persistant state, this value will  be zero */
   UI32_T	dnsServConfigResetTime;
   int		dnsServConfigReset;    /*Status/action object to reinitialize any persistant name
                                          server state.  When set to reset(2), any persistant
                                          name server state (such as a process) is reinitialized as
                                          if the name server had just been started.  This value
                                          will never be returned by a read operation.  When read,
                                          one of the following values will be returned:
                                          other(1) - server in some unknown state;
                                          initializing(3) - server (re)initializing;
                                          running(4) - server currently running */
   int		dnsServConfigMaxRequests; /*proxy can receive up to 20 requests simultaneously.range 1:20. default:10*/
   int		dnsServConfigCurrentNumberOfRequests; /*currently received number of requests in proxy*/
   int		dnsServEnabled;      /*0 disabled,1 enabled, 2 stopped; added by wiseway 2002-10-09*/
   int		dnsServStatus;

} DNS_MGR_IPCMsg_ProxyConfig_T;

typedef struct
{
    char hostname[256];
    UI32_T index;
    L_INET_AddrIp_T ip_addr;
} DNS_MGR_IPCMsg_Set_Host_Entry_T;

typedef struct
{
	I32_T					index;
	DNS_CacheRecord_T		cache;
}DNS_MGR_IPCMsg_Cache_Record_T;

typedef struct
{
    I8_T    hostname[SYS_ADPT_DNS_MAX_NAME_LENGTH+1];
    I8_T    aliasname[SYS_ADPT_DNS_MAX_NAME_LENGTH+1];
    I32_T		index;
} DNS_MGR_IPCMsg_AliasName_T;

typedef struct
{
    DNS_ResCounterByOpcodeEntry_T  data;
} DNS_MGR_IPCMsg_ResCounterByOpcodeEntry_T;

typedef struct
{
	HostEntry_T		host_entry;
	I32_T			index;
	char  			hostname[MAXHOSTNAMELEN];  /*maggie liu, ES4827G-FLF-ZZ-00243*/
	char                      hostaddr[20];
} DNS_MGR_IPCMsg_HostEntry_T;

typedef struct
{
	HostEntry_T		 host_entry;
	int			        index;
} DNS_MGR_IPCMsg_GetNextRunningHostEntry_T;


typedef struct
{
    L_INET_AddrIp_T          serveraddr;
} DNS_MGR_IPCMsg_NAME_SERVER_Entry_T;

typedef struct
{
	int	config_reset_time;
}DNS_MGR_IPCMsg_CONFIG_RESET_TIME_T;

typedef struct
{
    int	config_reset;
}DNS_MGR_IPCMsg_CONFIG_RESET_T;

typedef struct
{
  int 		DnsOptimize;
  UI32_T		DnsStatus;             /*default (1) enabled; (0) disabled*/ 	/* pass */
  char		DnsIpDomainName[DNS_MAX_NAME_LENGTH+1];
  int 		DnsMaxLocalRequests;       /*resolver can deal with up to 10 requests simultaneously.range 1:10 default 5*/
  int 		DnsDebugStatus;        /*default (0) disabled ; (1)enabled  */
  UI32_T 	DnsTimeOut;  /*range 1:15 (in seconds).default 12*/
} DNS_MGR_IPCMsg_Config_T;

typedef struct
{
    DNS_ServCounterEntry_T   serverdata;
} DNS_MGR_IPCMsg_ServCounterEntry_T;


typedef struct
{
    L_INET_AddrIp_T hostaddr ;
    char hostname[SYS_ADPT_DNS_MAX_NAME_LENGTH+1];
}DNS_MGR_IPCMsg_HOST_T;

typedef struct
{
   int		cache_status;				/* 1 enable cache, 2 disable cache */
   int  	cache_max_ttl;			/* the max ttl that cache permits  */
   int		cache_good_caches;
   int		cache_bad_caches;
   int		cache_max_entries;            /*the max  entries in cache, range: 1280..6400 default :2560        */
   UI32_T cache_max_ttl_32;             /* rich , for DNS get us 32 but set use int......too bad...*/
} DNS_MGR_IPCMsg_CacheConfig_T;

typedef struct
{
    DNS_ResCounterByRcodeEntry_T    data;
} DNS_MGR_IPCMsg_ResCounterByRcodeEntry_T;


typedef struct
{
    I8_T domain_name[DNS_MAX_NAME_LENGTH+1];
} DNS_MGR_IPCMsg_DomainName_T;

typedef struct
{
    UI32_T family;
    L_INET_AddrIp_T ipaddr[SYS_ADPT_MAX_NBR_OF_DNS_HOST_IP];
    char hostname[128];
} DNS_MGR_IPCMsg_Host2Ip_T;

typedef struct
{
    L_INET_AddrIp_T serverip;
} DNS_MGR_IPCMsg_CheckNameServerIP_T;

typedef struct
{
    UI32_T index;
    L_INET_AddrIp_T ip;
    BOOL_T          is_add;
} DNS_MGR_IPCMsg_Next_Name_Server_T;

typedef struct
{
    char 	DnsIpDomainName[DNS_MAX_NAME_LENGTH+1];
} DNS_MGR_IPCMsg_IpDomain_T;

typedef struct
{
    UI32_T  index;
    char 	name_str[DNS_MAX_NAME_LENGTH+1];
} DNS_MGR_IPCMsg_IdxNameStr_T;

typedef struct
{
    UI32_T          index;
    L_INET_AddrIp_T addr;
    BOOL_T          is_add;
} DNS_MGR_IPCMsg_HostAddrEntry_T;

typedef struct
{
    DNS_Nslookup_CTRL_T ctl_entry;
    UI32_T              ctl_index;
} DNS_MGR_IPCMsg_CreateNslookupCtlEntry_T;

typedef struct
{
    DNS_Nslookup_CTRL_T ctl_entry;
    UI32_T              ctl_index;
} DNS_MGR_IPCMsg_GetNslookupCtlEntryByIndex_T;

typedef struct
{
    DNS_Nslookup_Result_T result_entry;
    UI32_T                ctl_index;
    UI32_T                result_index;
} DNS_MGR_IPCMsg_GetNslookupResultEntryByIndex_T;

typedef union
{
    DNS_MGR_IPCMsg_CacheConfig_T				        cache_Config;
    DNS_MGR_IPCMsg_ResConfig_T					    res_config;
    DNS_MGR_IPCMsg_CONFIG_RESET_TIME_T	                reset_time;
    DNS_MGR_IPCMsg_CONFIG_RESET_T               reset_value;
    DNS_MGR_IPCMsg_ResConfigSbeltEntry_T                    resconfigsbel;
    DNS_MGR_IPCMsg_Config_T							gconfig;
    DNS_MGR_IPCMsg_HOST_T						    host;
    DNS_MGR_IPCMsg_IpDomain_T                                       ipdomain;
    DNS_MGR_IPCMsg_Set_Host_Entry_T                              host_entry_set;
    DNS_MGR_IPCMsg_ResCounterByOpcodeEntry_T            opcode_entry;
    DNS_MGR_IPCMsg_ResCounterByRcodeEntry_T              rcode_entry;
    DNS_MGR_IPCMsg_ServCounterEntry_T                           server_counter;
    DNS_MGR_IPCMsg_Next_Name_Server_T                       next_name_server;
    DNS_MGR_IPCMsg_NAME_SERVER_Entry_T                     name_server;
    DNS_MGR_IPCMsg_HostEntry_T                                      host_entry;
    DNS_MGR_IPCMsg_DomainName_T                                 domain_name;
    DNS_MGR_IPCMsg_AliasName_T                                      alias_name;
    DNS_MGR_IPCMsg_Cache_Record_T                                   cache_record;
    DNS_MGR_IPCMsg_ProxyConfig_T                                    proxy_config;
    DNS_MGR_IPCMsg_Host2Ip_T                                           host2ip;
    DNS_MGR_IPCMsg_CheckNameServerIP_T                        nameserverip;
    DNS_MGR_IPCMsg_IdxNameStr_T                 idx_namestr;
    DNS_MGR_IPCMsg_HostAddrEntry_T              hostip_entry;
    DNS_MGR_IPCMsg_CreateNslookupCtlEntry_T         create_nslookup_ctl_entry;
    DNS_MGR_IPCMsg_GetNslookupCtlEntryByIndex_T     get_nslookup_ctl_entry_by_index;
    DNS_MGR_IPCMsg_GetNslookupResultEntryByIndex_T  get_nslookup_result_entry_by_index;
} DNS_MGR_IPCMsg_Data_T;

typedef struct
{
    DNS_MGR_IPCMsg_Type_T    type;
    DNS_MGR_IPCMsg_Data_T    data;
} DNS_MGR_IPCMsg_T;


typedef struct
{
    DNS_Nslookup_CTRL_T data;
} DNS_MGR_IPCMsg_Nslookup_CTRL_T;


typedef struct
{
    DNS_Nslookup_Result_T data;
} DNS_MGR_IPCMsg_Nslookup_RESULT_T;

typedef struct
{
    UI32_T index;
} DNS_MGR_IPCMsg_DELETE_NSLOOKUP_T;

typedef struct
{
    UI32_T index;
    UI32_T timeout;
} DNS_MGR_IPCMsg_GET_TIMEOUT_T;

typedef struct
{
    UI32_T purge_time;
} DNS_MGR_IPCMsg_PURGE_TIME_T;

/* FUNCTION NAME:  DNS_MGR_Init
 * PURPOSE:
 *          Initiate the semaphore for DNS objects
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in DNS_Init.
 */
BOOL_T DNS_MGR_Init(void);


/*maggie liu*/
/* FUNCTION NAME:  DNS_MGR_ResetConfig
 * PURPOSE:
 *          Initiate the configure information for DNS
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function is invoked in DNS_TASK_EnterTransitionMode.
 */
void DNS_MGR_ResetConfig();

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DNS_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void DNS_MGR_Create_InterCSC_Relation(void);

/*the following APIs is used for Cache Mibs  */

/* FUNCTION NAME : DNS_MGR_SetDnsResCacheStatus
 * PURPOSE:
 *		This function is used for initializing dns cache
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- 1 enable cache,
 *			2 enable cache,
 *			3 clear the contents in the cache
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		This function will be called by DNS_CACHE_Init and snmp module.
 *
 */
int DNS_MGR_SetDnsResCacheStatus (int *dns_res_cache_status_p);

/* FUNCTION NAME : DNS_MGR_GetDnsResCacheStatus
 * PURPOSE:
 *		This function is used for getting the cache status.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the returned cache status
 *				enabled(1),
 *				disabled(2)
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResCacheStatus(int *dns_res_cache_status_p);

/* FUNCTION NAME : DNS_MGR_SetDnsResCacheMaxTTL
 * PURPOSE:
 *		This function is used for setting Maximum Time-To-Live for RRs in this cache.
 *		If the resolver does not implement a TTL ceiling, the value of this field should be zero.
 *
 * INPUT:
 *		int * -- a pointer to a variable for storing Maximum Time-To-Live for RRs in the cache
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_SetDnsResCacheMaxTTL(int *dns_res_cache_max_ttl_p);


/* FUNCTION NAME : DNS_MGR_GetDnsResCacheMaxTTL
 * PURPOSE:
 *		This fuction will be called by snmp module.
 *		If the resolver does not implement a TTL ceiling, the value of this field should be zero.
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		UI32_T * -- a pointer to a variable for storing the returned Maximum Time-To-Live for RRs in the cache
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		This function will be called by DNS_CACHE_Init and snmp module.
 */
 int DNS_MGR_GetDnsResCacheMaxTTL(UI32_T *dns_res_cache_max_ttl_p);


/* FUNCTION NAME : DNS_MGR_GetDnsResCacheGoodCaches
 * PURPOSE:
 *		This function is used for getting the number of rrs the resolver has cached successfully.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable for storing the number of rrs the resolver has cache successfully
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResCacheGoodCaches(int *dns_res_good_caches_p);


/* FUNCTION NAME : DNS_MGR_GetDnsResCacheBadCaches
 * PURPOSE:
 *		This function is used for getting the number of RRs the resolver has refused to cache.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to storing the returned number of rrs the resolver has refused to cache
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_MGR_GetDnsResCacheBadCaches(int *dns_res_cache_bad_caches_p);



/*the following APIs is used for Proxy Mibs                                                   */

/* FUNCTION NAME : DNS_MGR_GetDnsServConfigImplementIdent
 * PURPOSE:
 *		This function gets the implementation identification string for the DNS server software in use on the system
 *
 *
 * INPUT:
 *			I8_T*-- a pointer to a string to storing  the implementation
 *				identification string for the DNS server software
 *				in use on the system.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServConfigImplementIdent(I8_T *dns_serv_config_implement_ident_p);

/* FUNCTION NAME : DNS_GR_SetDnsServConfigRecurs
 * PURPOSE:
 *		This function sets the recursion services offered by this name server.
 *
 *
 * INPUT:
 *		none.
 * OUTPUT:
 *			int *-- A pointer to a variable to store the value to be set.
 *			This represents the recursion services offered by this
 *			name server.  The values that can be read or written are:
 *			available(1) - performs recursion on requests from clients.
 *			restricted(2) - recursion is performed on requests only
 *			from certain clients, for example; clients on an access
 *			control list.  It is not supported currently.
 *			unavailable(3) - recursion is not available.
 *
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_MGR_SetDnsServConfigRecurs(int *config_recurs_p);



/* FUNCTION NAME : DNS_MGR_SetDnsServConfigRecurs
 * PURPOSE:
 *		This function gets the recursion services offered by this name server.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- A pointer to a variable to store the returned value.
 *			This represents the recursion services offered by this
 *			name server.  The values that can be read or written are:
 *			available(1) - performs recursion on requests from clients.
 *			restricted(2) - recursion is performed on requests only
 *			from certain clients, for example; clients on an access
 *			control list.  It is not supported currently.
 *			unavailable(3) - recursion is not available.
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		 In our DNS, recursion is always available.
 */
int DNS_MGR_GetDnsServConfigRecurs(int *config_recurs_p);



/* FUNCTION NAME : DNS_MGR_GetDnsServConfigUpTime
 * PURPOSE:
 *		This function get the up time since the server started.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		    UI32_T* -- a pointer to a variable to store the returned
 *			   value about the up time since the server started
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *
 */
int DNS_MGR_GetDnsServConfigUpTime(UI32_T *dns_serv_config_up_time_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServConfigResetTime
 *
 * PURPOSE:
 *		This function gets the time elapsed since the last time the name server was `reset.'
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		UI32_T * -- a pointer to a variable to hold the reset time.
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_MGR_GetDnsServConfigResetTime( UI32_T *dns_serv_config_reset_time);



/* FUNCTION NAME : DNS_MGR_SetDnsServConfigReset
 * PURPOSE:
 *		This function reinitialize any persistant name server state.
 *
 *
 * INPUT:
 *		int * -- When set to reset(2), any persistant name server state
 *			(such as a process) is reinitialized as if the name
 *			server had just been started
 *
 * OUTPUT:
 *		none.
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_MGR_SetDnsServConfigReset(int *dns_serv_config_reset_p);



/* FUNCTION NAME : DNS_MGR_GetDnsServConfigReset
 * PURPOSE:
 *		This funtion gets any persistant name server state.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *			other(1) - server in some unknown state;
 *			initializing(3) - server (re)initializing;
 *			running(4) - server currently running
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *
 */
int DNS_MGR_GetDnsServConfigReset(int *dns_serv_config_reset_p);



 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterAuthAns
 *
 * PURPOSE:
 *		This function gets the Number of queries which were authoritatively answered.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to hold the returned.
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_MGR_GetDnsServCounterAuthAns(int *dns_serv_counter_auth_ans_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterAuthNoNames
 *
 * PURPOSE:
 *	This function gets the Number of queries for which `authoritative no such name'
 *	responses were made.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServCounterAuthNoNames(int *dns_serv_counter_auth_ans_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterAuthNoDataResps
 *
 * PURPOSE:
 *		This function gets the Number of queries for which `authoritative no such data'
 *	       (empty answer) responses were made
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_MGR_GetDnsServCounterAuthNoDataResps(int *dns_serv_counter_auth_no_data_resps_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterNonAuthDatas
 *
 * PURPOSE:
 *	This function gets the Number of queries which were non-authoritatively
 *	answered (cached data)
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServCounterNonAuthDatas(int *dns_serv_counter_non_auth_datas_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterNonAuthNoDatas
 *
 * PURPOSE:
 *		This funciton gets the number of Number of queries which were non-authoritatively
         answered with no data (empty answer)
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_MGR_GetDnsServCounterNonAuthNoDatas(UI32_T *dns_serv_counter_non_auth_no_datas_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterReferrals
 *
 * PURPOSE:
 *		This function gets the Number of requests that were referred to other servers
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		 int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServCounterReferrals(int *dns_serv_counter_referrals_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterErrors
 *
 * PURPOSE:
 *	This function gets the Number of requests the server has processed that were
 *	answered with errors (RCODE values other than 0 and 3).
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_MGR_GetDnsServCounterErrors(int *dns_serv_counter_errors_p);

/*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterRelNames
 *
 * PURPOSE:
 *	This function gets the Number of requests received by the server for names that
 *	are only 1 label long (text form - no internal dots)
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServCounterRelNames(int *dns_serv_counter_rel_names_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterReqRefusals
 *
 * PURPOSE:
 *		This function gets the Number of DNS requests refused by the server.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		 int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_MGR_GetDnsServCounterReqRefusals(int *dns_serv_counter_req_refusals_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterReqUnparses
 *
 * PURPOSE:
 *		This function gets the Number of requests received which were unparseable
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServCounterReqUnparses(int *dns_serv_counter_req_unparses_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterOtherErrors
 *
 * PURPOSE:
 *	This function gets the Number of requests which were aborted for other (local)
 *      server errors
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		 int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServCounterOtherErrors(int *dns_serv_counter_other_errors_p);

/*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterEntry
 *
 * PURPOSE:
 *		This function gets the dnsServCounterEntry according the specified index.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		DNS_ServCounterEntry_T * -- a pointer to a DNS_ServCounterEntry_T variable to store the returned entry
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_MGR_GetDnsServCounterEntry(DNS_ServCounterEntry_T *dns_serv_counter_entry_t_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetNextDnsServCounterEntry
 *
 * PURPOSE:
 *		This function gets the dnsServCounterEntry next to the specified index.
 *
 * INPUT:
 *		DNS_ServCounterEntry_T * -- INDEX { dnsServCounterOpCode, dnsServCounterQClass, dnsServCounterQType, dnsServCounterTransport }
 *
 * OUTPUT:
 *		DNS_ServCounterEntry_T * -- a pointer to a DNS_ServCounterEntry_T variable to store the returned entry
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		The initial input for index is {0,0,0,0}.
 */
int DNS_MGR_GetNextDnsServCounterEntry(DNS_ServCounterEntry_T *dns_serv_counter_entry_t_p);

/*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfAuthAns
 *
 * PURPOSE:
 *		This function gets the number of requests the server has processed which
 *      originated from a resolver on the same host for which  there has been an
 *      authoritative answer.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterSelfAuthAns(int *dns_serv_opt_counter_self_auth_ans_p);

/*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfAuthNoNames
 *
 * PURPOSE:
 *	This function gets the number of requests the server has processed which
 *	originated from a resolver on the same host for which
 *	there has been an authoritative no such name answer given.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_MGR_GetDnsServOptCounterSelfAuthNoNames(int *dns_serv_opt_counter_self_auth_no_names_p);

/*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfAuthNoDataResps
 *
 * PURPOSE:
 *	This function gets the number of requests the server has processed which
 *	originated from a resolver on the same host for which
 *	there has been an authoritative no such data answer (empty answer) made.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_MGR_GetDnsServOptCounterSelfAuthNoDataResps(int *dns_serv_opt_counter_self_auth_no_data_resps);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfNonAuthDatas
 *
 * PURPOSE:
 *	Number of requests the server has processed which
 *	originated from a resolver on the same host for which a
 *	non-authoritative answer (cached data) was made
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterSelfNonAuthDatas(int *dns_serv_opt_counter_self_non_auth_datas_p);


 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfNonAuthNoDatas
 *
 * PURPOSE:
 *	Number of requests the server has processed which
 *	originated from a resolver on the same host for which a
 *	non-authoritative, no such data' response was made
 *	(empty answer).
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterSelfNonAuthNoDatas(int *dns_serv_opt_counter_self_non_auth_no_datas_p);

/*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfReferrals
 *
 * PURPOSE:
 *	Number of queries the server has processed which
 *	originated from a resolver on the same host and were
 *	referred to other servers
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterSelfReferrals(int *dns_serv_opt_counter_self_referrals);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfErrors
 *
 * PURPOSE:
 *		Number of requests the server has processed which
 *       originated from a resolver on the same host which have
 *      been answered with errors (RCODEs other than 0 and 3).
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_MGR_GetDnsServOptCounterSelfErrors(int *dns_serv_opt_counter_self_errors_p);

/*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfRelNames
 *
 * PURPOSE:
 *	Number of requests received for names that are only 1
 *	label long (text form - no internal dots) the server has
 *	processed which originated from a resolver on the same
 *	host.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterSelfRelNames( int *dns_serv_opt_counter_self_rel_names);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfReqRefusals
 *
 * PURPOSE:
 *	Number of DNS requests refused by the server which
 *	originated from a resolver on the same host
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_MGR_GetDnsServOptCounterSelfReqRefusals(int *dns_serv_opt_counter_self_req_refusals_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfReqUnparses
 *
 * PURPOSE:
 *	Number of requests received which were unparseable and
 *	which originated from a resolver on the same host.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterSelfReqUnparses(int *dns_serv_opt_counter_self_req_unparses_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfOtherErrors
 *
 * PURPOSE:
 *	Number of requests which were aborted for other (local)
 *	server errors and which originated on the same host.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --   a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterSelfOtherErrors(int *dns_serv_opt_counter_self_other_errors_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsAuthAns
 *
 * PURPOSE:
 *	Number of queries originating from friends which were
 *	authoritatively answered.  The definition of friends is
 *	a locally defined matter
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --   a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_MGR_GetDnsServOptCounterFriendsAuthAns(int *dns_serv_opt_counter_friends_auth_ans_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsAuthNoNames
 *
 * PURPOSE:
 *	Number of queries originating from friends, for which
 *	authoritative `no such name' responses were made.  The
 *	definition of friends is a locally defined matter.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterFriendsAuthNoNames(int *dns_serv_opt_counter_friends_auth_no_names);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsAuthNoDataResps
 *
 * PURPOSE:
 *	Number of queries originating from friends for which
 *	authoritative no such data (empty answer) responses were
 *	made.  The definition of friends is a locally defined
 *	matter
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		UI32_T * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_MGR_GetDnsServOptCounterFriendsAuthNoDataResps(UI32_T *dns_serv_opt_counter_friends_auth_no_data_resps);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsNonAuthDatas
 *
 * PURPOSE:
 *	Number of queries originating from friends which were
 *	non-authoritatively answered (cached data). The
 *	definition of friends is a locally defined matter.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		UI32_T * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterFriendsNonAuthDatas(UI32_T *dns_serv_opt_counter_friends_non_auth_datas_p);


/* FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsNonAuthNoDatas
 * PURPOSE:
 *		Number of queries originating from friends which were
 *		non-authoritatively answered with no such data (empty
 *		answer)
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		UI32_T * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterFriendsNonAuthNoDatas(UI32_T *dns_serv_opt_counter_friends_non_auth_no_datas_p);



/* FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsReferrals
 * PURPOSE:
 *		Number of requests which originated from friends that
 *		were referred to other servers.  The definition of
 *		friends is a locally defined matter.
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		UI32_T * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterFriendsReferrals( UI32_T *dns_serv_opt_counter_friends_referrals_p);

/* FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsErrors
 * PURPOSE:
 *		Number of requests the server has processed which
 *		originated from friends and were answered with errors
 *		(RCODE values other than 0 and 3).  The definition of
 *		friends is a locally defined matter.
 * INPUT:
 *
 *
 * OUTPUT:
 *		UI32_T* -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterFriendsErrors(UI32_T *dns_serv_opt_counter_friends_errors_p);



/* FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsRelNames
 * PURPOSE:
 *		Number of requests received for names from friends that
 *		are only 1 label long (text form - no internal dots) the
 *		server has processed
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		UI32_T * -- a pointer to a variable to store the result.
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterFriendsRelNames(UI32_T *dns_serv_opt_counter_friends_rel_names_p);


/* FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsReqRefusals
 * PURPOSE:
 *		This function gets the number of DNS requests refused by the server which were
 *		received from `friends'.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		UI32_T * -- a pointer to the variable to store the returned value
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterFriendsReqRefusals(UI32_T *dns_serv_opt_counter_friends_req_refusals_p);

/* FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsReqUnparses
 * PURPOSE:
 *		Number of requests received which were unparseable and
 *		which originated from `friends'
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		UI32_T * -- a pointer to the variable to store the returned value.
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterFriendsReqUnparses(UI32_T *dns_serv_opt_counter_friends_req_unparses_p);



/* FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsOtherErrors
 * PURPOSE:
 *		Number of requests which were aborted for other (local)
 *		server errors and which originated from `friends'.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		UI32_T * -- a pointer to the variable to store the returned value
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterFriendsOtherErrors(UI32_T *dns_serv_opt_counter_friends_other_errors_p);

/* FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsOtherErrors
 * PURPOSE:
 *		Number of requests which were aborted for other (local)
 *		server errors and which originated from `friends'.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		UI32_T * -- a pointer to the variable to store the returned value
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_MGR_GetDnsResConfigImplementIdent(I8_T *dns_res_config_implement_ident_p);


/* FUNCTION NAME : DNS_MGR_GetDnsResConfigService
 * PURPOSE:
 *		Get kind of DNS resolution service provided	.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *			 recursiveOnly(1) indicates a stub resolver.
 *			iterativeOnly(2) indicates a normal full service resolver.
 *			recursiveAndIterative(3) indicates a full-service
 *			resolver which performs a mix of recursive and iterative queries.
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResConfigService(int *dns_res_config_service_p);

/* FUNCTION NAME : DNS_MGR_GetDnsResConfigMaxCnames
 * PURPOSE:
 *		Limit on how many CNAMEs the resolver should allow
 *		before deciding that there's a CNAME loop.  Zero means
 *		that resolver has no explicit CNAME limit.
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResConfigMaxCnames(int *dns_resconfig_max_cnames_p);

/* FUNCTION NAME : DNS_MGR_SetDnsResConfigSbeltEntry
 * PURPOSE:
 *		This funciton get the specified DnsResConfigSbeltEntry according to the index.
 *
 *
 *
 * INPUT:
 *		DNS_ResConfigSbeltEntry_T * -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the value to be set
 *  									 INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_MGR_SetDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);

/* FUNCTION NAME : DNS_MGR_GetDnsResConfigSbeltEntry
 * PURPOSE:
 *		This funciton get the specified DnsResConfigSbeltEntry according to the index.
 *
 *
 * INPUT:
 *		DNS_ResConfigSbeltEntry_T* -- INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *		DNS_ResConfigSbeltEntry_T* -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the returned value
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		This function will be called by DNS_RESOLVER_Init and snmp module.
 *
 */
int DNS_MGR_GetDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);

/* FUNCTION NAME : DNS_MGR_GetDnsResConfigSbeltEntry
 * PURPOSE:
 *		This funciton get the specified DnsResConfigSbeltEntry according to the index.
 *
 *
 * INPUT:
 *		DNS_ResConfigSbeltEntry_T* -- INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *		DNS_ResConfigSbeltEntry_T* -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the returned value
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		This function will be called by DNS_CACHE_Init and snmp module.
 *		The initial input of index is 0 for dnsResConfigSbeltAddr.
 */
int DNS_MGR_GetNextDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);



/* FUNCTION NAME : DNS_MGR_GetDnsResConfigUpTime
 * PURPOSE:
 *		If the resolver has a persistent state (e.g., a
 *		process), this value will be the time elapsed since it
 *		started.  For software without persistant state, this
 *		value will be 0.
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to storing config up time
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResConfigUpTime(int *config_up_time_p);

/* FUNCTION NAME : DNS_MGR_GetDnsResConfigResetTime
 * PURPOSE:
 *		This function gets the time elapsed since it started.
 *
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to storing the returned config reset time
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		 This function will be called by  snmp module.
 */
int DNS_MGR_GetDnsResConfigResetTime(int *config_reset_time_p);

/* FUNCTION NAME : DNS_MGR_SetDnsResConfigReset
 * PURPOSE:
 *		This function reinitialize any persistant resolver state.
 *
 *
 *
 * INPUT:
 *		int * -- a pointer to a variable stored with the reset value,2 means reset
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_MGR_SetDnsResConfigReset( int *config_reset_p);

/* FUNCTION NAME : DNS_MGR_GetDnsResConfigReset
 * PURPOSE:
 *		This function gets any persistant resolver state.
 *
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable for storing the returned resolver state
 *			other(1) - resolver in some unknown state;
 *			initializing(3) - resolver (re)initializing;
 *			running(4) - resolver currently running.
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		 This function will be called by  snmp module.
 */
int DNS_MGR_GetDnsResConfigReset(int *config_reset_p);


/* FUNCTION NAME : DNS_MGR_SetDnsResConfigQueryOrder
 * PURPOSE:
 *		This function  set the query order.
 *
 *
 *
 * INPUT:
 *		int * -- a pointer to a variable storing the value to be set
 *			DNS_QUERY_LOCAL_FIRST:Query static host table first;
 *			DNS_QUERY_DNS_FIRST: Query DNS server first
 *			default: DNS_QUERY_LOCAL_FIRST
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_SetDnsResConfigQueryOrder(int *dns_res_config_query_order_p);

/* FUNCTION NAME : DNS_MGR_GetDnsResConfigQueryOrder
 * PURPOSE:
 *		This function  get the query order.
 *
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the returned value about query order
 *			DNS_QUERY_LOCAL_FIRST:Query static host table first;
 *			DNS_QUERY_DNS_FIRST: Query DNS server first
 *			DNS_QUERY_ DNS_ONLY: Query DNS server only
 *			default: DNS_QUERY_LOCAL_FIRST
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		 This function will be called by DNS_RESOLVER_Init.
 */
int DNS_MGR_GetDnsResConfigQueryOrder(int *dns_res_config_query_order_p);


/* FUNCTION NAME : DNS_MGR_GetDnsResCounterByOpcodeEntry
 * PURPOSE:
 *		This function gets dnsResCounterByOpcodeEntry according to the index.
 *
 *
 *
 * INPUT:
 *		DNS_ResCounterByOpcodeEntry_T * -- Given as the index of the entry.
 *
 * OUTPUT:
 *		DNS_ResCounterByOpcodeEntry_T * -- a pointer to a variable t store the returned entry
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK   :succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_MGR_GetDnsResCounterByOpcodeEntry(DNS_ResCounterByOpcodeEntry_T *dns_res_counter_by_opcode_entry_p);

/* FUNCTION NAME : DNS_MGR_GetNextDnsResCounterByOpcodeEntry
 * PURPOSE:
 *		This function gets dnsResCounterByOpcodeEntry next to the specified index.
 *
 *
 *
 * INPUT:
 *		DNS_ResCounterByOpcodeEntry_T * -- Given to provide the index of the entry.
 *
 * OUTPUT:
 *		DNS_ResCounterByOpcodeEntry_T * -- a pointer to a variable to store the returned entry
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK   :succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_MGR_GetNextDnsResCounterByOpcodeEntry(DNS_ResCounterByOpcodeEntry_T *dns_res_counter_by_opcode_entry_p);

/* FUNCTION NAME : DNS_MGR_GetDnsResCounterByRcodeEntry
 * PURPOSE:
 *		This function gets the number of responses for the specified index.
 *
 *
 *
 * INPUT:
 *		DNS_ResCounterByRcodeEntry_T * -- Given as a index for the entry.
 *										  INDEX { dnsResCounterByRcodeCode }
 * OUTPUT:
 *		DNS_ResCounterByRcodeEntry_T * --a pointer to a DnsResCounterByRcodeEntry variable to store the returned entry
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_MGR_GetDnsResCounterByRcodeEntry(DNS_ResCounterByRcodeEntry_T *dns_res_counter_by_rcode_entry_p);

/* FUNCTION NAME : DNS_MGR_GetNextDnsResCounterByRcodeEntry
 * PURPOSE:
 *		This function gets the number of responses for the index next to the specified index.
 *
 *
 *
 * INPUT:
 *		DNS_ResCounterByRcodeEntry_T * -- Given as the index of the entry.
 *										 INDEX { dnsResCounterByRcodeCode }
 * OUTPUT:
 *		DNS_ResCounterByRcodeEntry_T * -- a pointer to a DnsResCounterByRcodeEntry variable to store the returned entry
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_MGR_GetNextDnsResCounterByRcodeEntry(DNS_ResCounterByRcodeEntry_T *dns_res_counter_by_rcode_entry_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsResCounterNonAuthDataResps
 *
 * PURPOSE:
 *		This functin gets the number of requests made by the resolver for which a
 *      non-authoritative answer (cached data) was received.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsResCounterNonAuthDataResps(int *dns_res_counter_non_auth_data_resps_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsResCounterNonAuthNoDataResps
 *
 * PURPOSE:
 *		This fucniton gets Number of requests made by the resolver for which a
         non-authoritative answer - no such data response (empty answer) was received
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_MGR_GetDnsResCounterNonAuthNoDataResps(int *dns_res_counter_non_auth_no_data_resps_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsResCounterMartians
 *
 * PURPOSE:
 *		This function gets the number of responses received which were received from
 *      servers that the resolver does not think it asked.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		 int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsResCounterMartians(int *dns_res_counter_martians_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsResCounterRecdResponses
 *
 * PURPOSE:
 *		This function gets Number of responses received to all queries.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --   a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsResCounterRecdResponses(int *dns_res_counter_recd_responses_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsResCounterUnparseResps
 *
 * PURPOSE:
 *		This function gets Number of responses received which were unparseable.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		 int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_MGR_GetDnsResCounterUnparseResps(int *dns_res_counter_unparse_resps_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsResCounterFallbacks
 *
 * PURPOSE:
 *		This function gets the number of times the resolver had to fall back to its seat belt information.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsResCounterFallbacks(int *dns_res_counter_fallbacks_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsResOptCounterReferals
 *
 * PURPOSE:
 *		This function gets the number of responses which were
 *		eived from servers redirecting query to another server.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsResOptCounterReferals(int *dns_res_opt_counter_referals_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsResOptCounterRetrans
 *
 * PURPOSE:
 *		This function gets the number requests retransmitted for all reasons
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsResOptCounterRetrans(int *dns_res_opt_counter_retrans_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsResOptCounterNoResponses
 *
 * PURPOSE:
 *		This function gets the number of queries that were retransmitted because of no response
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsResOptCounterNoResponses(int *dns_res_opt_counter_no_responses_p);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsResOptCounterRootRetrans
 *
 * PURPOSE:
 *		This function gets the number of queries that were retransmitted that were to root servers.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsResOptCounterRootRetrans(int *dns_res_opt_counter_root_retrans_p);




 /*
 * FUNCTION NAME : DNS_MGR_GetDnsResOptCounterInternals
 *
 * PURPOSE:
 *		This function gets the number of requests internally generated by the resolver.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsResOptCounterInternals(int *dns_res_opt_counter_internals);

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsResOptCounterInternalTimeOuts
 *
 * PURPOSE:
 *		This function gets the number of requests internally generated which timed out.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by snmp module.
 */
int DNS_MGR_GetDnsResOptCounterInternalTimeOuts(int *dns_res_opt_counter_internal_time_outs_p);


#if 0
/* FUNCTION NAME : DNS_MGR_HostTblInit
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
 *		This function will be called by snmp module.
 */
void DNS_MGR_HostTblInit(void);
#endif



 /* FUNCTION NAME : DNS_MGR_HostGetByName
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
 *		This function will be called by DNS_RESOLVER_Init.
 */
int DNS_MGR_HostGetByName(const char * name, L_INET_AddrIp_T addr_ar[]);


/* FUNCTION NAME : DNS_MGR_HostGetByAddr
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
 *		This function will be called by resolver.
 */
int DNS_MGR_HostGetByAddr(const I8_T * addr,I8_T * name);

/* FUNCTION NAME : DNS_MGR_ClearHosts
 * PURPOSE:
 *		This routine prints a list of remote hosts, along with their Internet addresses and aliases
 *
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		 This function will be called by CLI command " clear host [*] ".
 */
BOOL_T DNS_MGR_ClearHosts(void);


/* FUNCTION NAME : DNS_MGR_GetNextDnsHostEntry
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
 *		This function will be called by Config module.
 *		The initial value is -1.
 *      This function will be called by CLI command "show hosts"
 */
int DNS_MGR_GetNextDnsHostEntry(int *host_index_p,HostEntry_PTR dns_host_entry_t_p);


/* FUNCTION NAME : DNS_MGR_GetDnsHostEntry
 * PURPOSE:
 *		This funciton get the specified  according to the index.
 *
 *
 * INPUT:
 *		int host_index    -- current active index of HostEntry_T struct.
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
 *		This function will be called by Config module.
 *      This function will be called by web and snmp
 */
int DNS_MGR_GetDnsHostEntry(int host_index_p,HostEntry_PTR dns_host_entry_t_p);


/* FUNCTION NAME : DNS_MGR_AddDnsHostEntry
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
 *		 This function will be called by Config module.
 */
int DNS_MGR_AddDnsHostEntry(HostEntry_PTR dns_host_entry_t_p);



/* FUNCTION NAME : DNS_MGR_EnableDomainLookup
 * PURPOSE:
 *  This fcuntion enable DNS including resolver and proxy.
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  none.
 *  FALSE :failure,
 *  TRUE  :success.
 *
 * NOTES:
 *      This function will be called by CLI command "ip domain-lookup".
 */
BOOL_T DNS_MGR_EnableDomainLookup(void);

/* FUNCTION NAME : DNS_MGR_DisableDomainLookup
 * PURPOSE:
 *  This fcuntion disable DNS including resolver and proxy.
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  none.
 *
 * NOTES:
 *      This function will be called by CLI command "no ip domain-lookup".
 */
BOOL_T DNS_MGR_DisableDomainLookup(void);

/* FUNCTION NAME : DNS_MGR_AddDomainName
 * PURPOSE:
 *		To define a default domain name that the ACCTON Switch software uses to
 *		complete unqualified host names (names without a dotted-decimal domain name)
 *
 *
 * INPUT:
 *		I8_T * -- a domain name to be added
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 * NOTES:
 *		 This function will be called by CLI command "ip domain-name".
 */
int DNS_MGR_AddDomainName(char *domain_name_p);

/* FUNCTION NAME : DNS_MGR_DeleteDomainName
 * PURPOSE:
 *		Delete the default domain name.
 *
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		DNS_OK:success;
 *
 * NOTES:
 *		 This function will be called by CLI command "no ip domain-name".
 */
int DNS_MGR_DeleteDomainName(void);

/* FUNCTION NAME : DNS_MGR_AddDomainNameToList
 * PURPOSE:
 *		If there is no domain list, the domain name that you specified with the ip
 *		domain-name global configuration command is used. If there is a domain list,
 *		the default domain name is not used. The ip domain-list command is similar
 *		to the ip domain-name command, except that with the ip domain-list command
 *		you can define a list of domains, each to be tried in turn.
 * INPUT:
 *		I8_T * -- a domain name to be added to the domain name lsit.
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by CLI command "ip domain-list"
 */
int DNS_MGR_AddDomainNameToList(char *domain_name_p);

/* FUNCTION NAME : DNS_MGR_DeleteDomainNameToList
 * PURPOSE:
 *		If there is no domain list, the domain name that you specified with the ip
 *		domain-name global configuration command is used. If there is a domain list,
 *		the default domain name is not used. The ip domain-list command is similar
 *		to the ip domain-name command, except that with the ip domain-list command
 *		you can define a list of domains, each to be tried in turn.
 * INPUT:
 *		I8_T * -- a domain name to be deleted to the domain name lsit
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by CLI comamnd "no ip domain-list"
 */
int DNS_MGR_DeleteDomainNameFromList(char *domain_name_p);

/* FUNCTION NAME : DNS_MGR_AddNameServer
 * PURPOSE:
 *		This function adds a name server IP address to the name server list.
 *
 *
 * INPUT:
 *		L_INET_AddrIp_T  * -- a pointer to a ip addr will be added as a name server;
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		This function will be called by CLI command "ip name-servert".
 */
int DNS_MGR_AddNameServer(L_INET_AddrIp_T *addr_p);

/* FUNCTION NAME : DNS_MGR_DeleteNameServer
 * PURPOSE:
 *		This function deletes a name server entry from
 *		the name server list accordint to the IP address of the name server.
 *
 * INPUT:
 *		I8_T * -- a pointer to a ip addr , whose related name server will be deleted;
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		This function will be called by CLI command "no ip name-servert".
 */
int DNS_MGR_DeleteNameServer(L_INET_AddrIp_T *addr_p);

/* FUNCTION NAME : DNS_MGR_ClearDnsCache
 * PURPOSE:
 *     This function delete the contents of the dns cache.The default
 *     configuration will be used.
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *		DNS_OK :success
 * NOTES:
 *		This function will be called by configuration sub moudle.
 *		his function will be called by CLI command "clear dns cache"
 */
int DNS_MGR_ClearDnsCache(void);

/* FUNCTION NAME : DNS_MGR_ShowDnsConfig
 * PURPOSE:
 *		This function shows the dns configuration.
 *
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		 This function will be called by CLI module.
 */
BOOL_T DNS_MGR_ShowDnsConfig(void);

/* FUNCTION NAME : DNS_MGR_ShowDnsDatabase
 * PURPOSE:
 *		This funciton is used for showing  cache database and status.
 *		Every field except link should be displayed, and the index of this cache
 *		entry should also be displayed.
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
 *		 This function will be called by CLI module.
 */
BOOL_T DNS_MGR_ShowDnsDatabase(void);

 /*
 * FUNCTION NAME : DNS_MGR_ShowDnsCache
 *
 * PURPOSE:
 *	This function is used for showing  cache database and status.
 *	Every field except link should be displayed, and the index of this cache
 *	entry should also be displayed.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		This function will be called by CLI module.
 */
BOOL_T DNS_MGR_ShowDnsCache(void);

 /* FUNCTION NAME : DNS_MGR_DebugOpen
 * PURPOSE:
 *		This fcuntion enables the displaying of debugging information.
 *
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
 *		This function will be called by CLI module.
 */
BOOL_T DNS_MGR_DebugOpen(void);

 /*
 * FUNCTION NAME : DNS_MGR_DEBUG_CLOSE
 *
 * PURPOSE:
 *		This functin stops showing debug information enabled by the function DNS_MGR_DEBUG_OPEN.
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
 *		This function will be called by CLI module.
 */
BOOL_T DNS_MGR_DebugClose(void);

/* FUNCTION NAME : DNS_MGR_SetDnsResCacheMaxEntries
 * PURPOSE:
 *		This fcuntion set the max number of entries in cache.
 *
 *
 * INPUT:
 *		int * -- a pointer to a variable for storing the number of
 *				max entries in cache. range:1280-6400. default:2560
 *
 * OUTPUT:
 *		none.
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_SetDnsResCacheMaxEntries(int *dns_res_cache_max_entries_p);

/* FUNCTION NAME : DNS_MGR_GetDnsResCacheMaxEntries
 * PURPOSE:
 *		This fcuntion get the max number of entries in cache.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable for storing the returned value
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		 This function will be called by  snmp module.
 */
int  DNS_MGR_GetDnsResCacheMaxEntries(int *dns_res_cache_max_entries_p);

/* FUNCTION NAME : DNS_MGR_SetDnsServConfigMaxRequests
 * PURPOSE:
 *		This fcuntion set the max number of requests in proxy.
 *
 *
 * INPUT:
 *		int * -- a pointer to a variable for storing the number of
 *			max requests in proxy. range:1-20. default:10
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *
 */
int DNS_MGR_SetDnsServConfigMaxRequests(int *dns_serv_config_max_requests_p);

/* FUNCTION NAME :DNS_MGR_GetDnsServConfigMaxRequests
 * PURPOSE:
 *		This fcuntion gets the max number of requests in proxy.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable for storing the returned value
 *
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		This function will be called by  snmp module.
 */
int  DNS_MGR_GetDnsServConfigMaxRequests(int *dns_res_cache_max_entries_p);

/* FUNCTION NAME : DNS_MGR_SetDnsLocalMaxRequests
 * PURPOSE:
 *		This funciton set the max number of local requests that resolver
 *		can deal with.
 *
 *
 * INPUT:
 *		int * -- a pointer to a variable for storing the number of
 *				local max requests. range:1..10. default:5
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_MGR_SetDnsLocalMaxRequests(int *dns_config_local_max_requests_p);


/* FUNCTION NAME : DNS_MGR_GetDnsLocalMaxRequests
 * PURPOSE:
 *		This funciton set the max number of local requests that resolver
 *		can deal with.
 *
 *
 * INPUT:
 *		none.
 * OUTPUT:
 *		int * -- a pointer to a variable for storing the returned number of
 *				local max requests. range:1..10. default:5
 *
 * RETURN:
 *
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int  DNS_MGR_GetDnsLocalMaxRequests(int *dns_config_local_max_requests_p);

/* FUNCTION NAME : DNS_MGR_SetDnsLocalMaxRequests
 * PURPOSE:
 *		This fcuntion sets time out value (seconds) for requests.
 *
 *
 *
 * INPUT:
 *		UI32_T*--a pointer to a variable for storing timeout value
 *			for requests. range:1..20 . default:5 seconds
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int  DNS_MGR_SetDnsTimeOut(UI32_T *dns_time_out_p);

/* FUNCTION NAME : DNS_MGR_SetDnsLocalMaxRequests
 * PURPOSE:
 *		This fcuntion gets the time out value (seconds) for requests.
 *
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		UI32_T*	--	a pointer to a variable for storing the returned value
 *				range 1:20; default: 5 seconds
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_MGR_GetDnsTimeOut(UI32_T *dns_time_out_p);

/* FUNCTION NAME : DNS_MGR_ShowNameServerList
 * PURPOSE:
 *		This fcuntion displays the nam server informaion.
 *
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
 *		This function will be called by  snmp module.
 */
BOOL_T DNS_MGR_ShowNameServerList(void);

/* FUNCTION NAME : DNS_MGR_GetDnsResCounterFallbacks
 * PURPOSE:
 *		This function gets the number of times the resolver had to fall back to its seat belt information.
 *
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		 This function will be called by CLI module.
 */
BOOL_T DNS_MGR_ShowDomainNameList(void);


/* FUNCTION NAME : DNS_MGR_HostShow
 * PURPOSE:
 *		This routine prints a list of remote hosts, along with their Internet addresses and aliases
 *
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
BOOL_T DNS_MGR_HostShow(void);

/* FUNCTION NAME : DNS_MGR_HostAdd
 * PURPOSE:
 *		add a host to the host table
 *
 *
 *
 * INPUT:
 *      I8_T * hostname -- host name
 *      L_INET_AddrIp_T *hostaddr_p --  host addr in standard Internet format
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		DNS_ERROR :failure,
 *		DNS_OK    :success.
 * NOTES:
 *		 This function will be called by CLI command "ip host"
 */
int DNS_MGR_HostAdd(char* hostname, L_INET_AddrIp_T *hostaddr_p);

/* FUNCTION NAME : DNS_MGR_HostDelete
 * PURPOSE:
 *		This routine deletes a host name from the local host table.  If <name> is
 *		a host name, the host entry is deleted.  If <name> is a host name alias,
 *		the alias is deleted.
 *
 * INPUT:
 *		I8_T * -- host name or alias
 *		I8_T * -- host addr in standard Internet format
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		DNS_OK, of DNS_ERROR if not find the entry..
 *
 * NOTES:
 *		 This function will be called by CLI command "no ip host".
 */
int DNS_MGR_HostDelete(char* name, L_INET_AddrIp_T *addr_p);


/* FUNCTION NAME : DNS_MGR_HostNameDelete
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
 *		This function will be called by CLI command "clear host [name]"
 */
int DNS_MGR_HostNameDelete(char * name);



/* FUNCTION NAME : DNS_MGR_SetHostName
 *
 * PURPOSE:
 *		This routine sets the target machine's symbolic name, which can be used
 *		for identification.
 *
 * INPUT:
 *		const I8_T * -- machine name
 *		int          -- length of name
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		DNS_OK, or DNS_ERROR if nameLen is larger then MAXHOSTNAMELEN
 *
 * NOTES:
 *		none
 */
int DNS_MGR_SetHostName(const I8_T * name,	int nameLen);


/* FUNCTION NAME :	DNS_MGR_GetHostName
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
int DNS_MGR_GetHostName(I8_T *name, int nameLen);

/* FUNCTION NAME : DNS_MGR_ServCounterInc
 *
 * PURPOSE:
 *		Server's counter add 1(index with leaf)
 *
 * INPUT:
 *		int.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
BOOL_T DNS_MGR_ServCounterInc(int leaf);

/* FUNCTION NAME : DNS_MGR_ServOptCounterInc
 *
 * PURPOSE:
 * 		Server counter add 1 (index by leaf)
 *
 * INPUT:
 *		int -- .
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
BOOL_T DNS_MGR_ServOptCounterInc(int leaf);


/* FUNCTION NAME : DNS_MGR_ResOptCounterInc
 *
 * PURPOSE:
 * 		Resolver's opt counter add 1 (index by leaf)
 *
 * INPUT:
 *		int -- leaf.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
BOOL_T DNS_MGR_ResOptCounterInc(int leaf);

/* FUNCTION NAME : DNS_MGR_ResCounterInc
 *
 * PURPOSE:
 *		Resolver counter add 1 (index by leaf)
 *
 * INPUT:
 *		int -- leaf.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
BOOL_T DNS_MGR_ResCounterInc(int leaf);

/* FUNCTION NAME : DNS_MGR_ResCounterByOpcodeInc
 *
 * PURPOSE:
 *		resolver counter add 1 index by opcode
 *
 * INPUT:
 *		int -- identify whether a request or response
 *		int -- index
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
BOOL_T DNS_MGR_ResCounterByOpcodeInc(int qr,int opcode);

/* FUNCTION NAME : DNS_MGR_ResCounterByRcodeInc
 *
 * PURPOSE:
 *		resolver counter add 1 (index by rcode)
 *
 * INPUT:
 *		int -- index
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
BOOL_T DNS_MGR_ResCounterByRcodeInc(int rcode);

/* FUNCTION NAME : DNS_MGR_GetDnsDebugStatus
 *
 * PURPOSE:
 * 		Get whether server debug status is open.
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
 *		none.
 */
int	DNS_MGR_GetDnsDebugStatus(void);

/* FUNCTION NAME : DNS_MGR_ShowCounter
 *
 * PURPOSE:
 *		show dns counter.
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
 *		none.
 */
BOOL_T DNS_MGR_ShowCounter(void);

/* FUNCTION NAME : DNS_MGR_GetDnsStatus
 *
 * PURPOSE:
 * 		Get dns status
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		DNS_ENABLE or DNS_DISABLE.
 *
 * NOTES:
 *		This function will be called by CLI command "show dns"
 */
int DNS_MGR_GetDnsStatus(void);

/* FUNCTION NAME : DNS_MGR_SetDnsStatus
 *
 * PURPOSE:
 * 		set dns status
 *
 * INPUT:
 *		int -- status.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
BOOL_T DNS_MGR_SetDnsStatus(int status);

/* FUNCTION NAME : DNS_MGR_GetDnsIpDomain
 *
 * PURPOSE:
 *		Get dns ip domain name.
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      char* -- ipdomain name .
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *		DNS_OK :success
 *
 * NOTES:
 *		This function will be called by CLI command "show dns"
 */
int DNS_MGR_GetDnsIpDomain(char* ipdomain);

/* FUNCTION NAME : DNS_MGR_DnsUptimeInit
 *
 * PURPOSE:
 *		Initiate dns uptime
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
 *		none.
 */
BOOL_T DNS_MGR_DnsUptimeInit(void);

/* FUNCTION NAME : DNS_MGR_DnsResetTimeInit
 *
 * PURPOSE:
 *		initiate dns reset time
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
 *		none.
 */
BOOL_T DNS_MGR_DnsResetTimeInit(void);

/* FUNCTION NAME : DNS_MGR_GetDnsSbelt
 *
 * PURPOSE:
 *		Get dns sbelt.
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      DNS_ResConfigSbelt_T* -- dns sbelt
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
DNS_ResConfigSbelt_T* DNS_MGR_GetDnsSbelt(void);

/* FUNCTION NAME : DNS_MGR_ResCounterInit
 *
 * PURPOSE:
 * 		initiate dns resolver counter
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
 *		none.
 */
BOOL_T DNS_MGR_ResCounterInit(void);

/* FUNCTION NAME : DNS_MGR_ServCounterInit
 *
 * PURPOSE:
 *		initiate dns server counter.
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
 *		none.
 */
BOOL_T DNS_MGR_ServCounterInit(void);

/* FUNCTION NAME : DNS_MGR_GetServStatus
 *
 * PURPOSE:
 *		get dns server status
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		int -- dns server status
 *
 * NOTES:
 *		none.
 */
int DNS_MGR_GetServStatus(void);

/* FUNCTION NAME : DNS_MGR_SetServStatus
 *
 * PURPOSE:
 *		ser dns server status
 *
 * INPUT:
 *		int -- dns server status
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
BOOL_T DNS_MGR_SetServStatus(int status);

/* FUNCTION NAME : DNS_MGR_GetServCurrentRequestNumber
 *
 * PURPOSE:
 *		get server current request number.
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		int -- current server request number
 *
 * NOTES:
 *		none.
 */
int DNS_MGR_GetServCurrentRequestNumber(void);

/* FUNCTION NAME : DNS_MGR_ServCurrentRequestNumberInc
 *
 * PURPOSE:
 *		current server request number add 1
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
 *		none.
 */
BOOL_T DNS_MGR_ServCurrentRequestNumberInc(void);

/* FUNCTION NAME : DNS_MGR_ServCurrentRequestNumberDec
 *
 * PURPOSE:
 *		Dec current request number
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
 *		none.
 */
BOOL_T DNS_MGR_ServCurrentRequestNumberDec(void);

/* FUNCTION NAME : DNS_MGR_SetServResetStatus
 *
 * PURPOSE:
 *		Ser server reset status.
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
 *		none.
 */
BOOL_T DNS_MGR_SetServResetStatus(int status);

/* FUNCTION NAME : DNS_MGR_SetServServiceEnable
 * PURPOSE:
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *		none.
 */
BOOL_T DNS_MGR_SetServServiceEnable(int enable);

/* FUNCTION NAME : DNS_MGR_GetServServiceEnable
 *
 * PURPOSE:
 *		Get whether server is enable
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *      none.
 */
int DNS_MGR_GetServServiceEnable(void);

/* FUNCTION NAME : DNS_MGR_ServUpTimeInit
 *
 * PURPOSE:
 *		initiate server uptime
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *      none.
 */
BOOL_T DNS_MGR_ServUpTimeInit(void);

/* FUNCTION NAME : DNS_MGR_ServResetTimeInit
*
 * PURPOSE:
 *		initiate server reset time
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
BOOL_T DNS_MGR_ServResetTimeInit(void);



/* FUNCTION NAME:  DNS_MGR_EnterMasterMode
 * PURPOSE:
 *          This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the DNS subsystem will enter the
 *          Master Operation mode.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          1. If "System Boot Configruation File" does not exist, the system database and
 *				switch will be initiated to the factory default value.
 *			2. DNS will handle network requests only when this subsystem
 *				is in the Master Operation mode
 *			3. This function is invoked in DNS_INIT_EnterMasterMode.
 */
BOOL_T DNS_MGR_EnterMasterMode(void);



/* FUNCTION NAME:  DNS_MGR_EnterTransitionMode
 * PURPOSE:
 *          This function forces this subsystem enter the Transition Operation mode.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *			.
 */
BOOL_T DNS_MGR_EnterTransitionMode(void);



/* FUNCTION	NAME : DNS_MGR_SetTransitionMode
 * PURPOSE:
 *		Set transition mode.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		None.
 */
void DNS_MGR_SetTransitionMode(void);



/* FUNCTION NAME:  DNS_MGR_EnterSlaveMode
 * PURPOSE:
 *          This function forces this subsystem enter the Slave Operation mode.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          In Slave Operation mode, any network requests
 *          will be ignored.
 */
void DNS_MGR_EnterSlaveMode(void);



/* FUNCTION	NAME : DNS_MGR_GetOperationMode
 * PURPOSE:
 *		Get current sshd operation mode (master / slave / transition).
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *          SYS_TYPE_Stacking_Mode_T - opmode.
 *
 * NOTES:
 *		None.
 */
SYS_TYPE_Stacking_Mode_T DNS_MGR_GetOperationMode(void);



/* FUNCTION NAME:  DNS_MGR_BackdoorFunction
 * PURPOSE:
 *          Display back door available function and accept user seletion.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void DNS_MGR_BackdoorFunction();



/* FUNCTION NAME:  DNS_MGR_HostNameToIp
 * PURPOSE:
 *          This function is get host ip from host name.
 *
 * INPUT:
 *          UI8_T   *hostname   --  string of hostname.
 *
 * OUTPUT:
 *          UI32_T  *hostip --  host address with ip network order.
 *
 * RETURN:
 *          TRUE    -- return value is valid.
 *          FALSE   -- return value is invalid.
 * NOTES:
 *          .
 */
int DNS_MGR_HostNameToIp(UI8_T *hostname, UI32_T family, L_INET_AddrIp_T hostip_ar[]);



/* FUNCTION NAME : DNS_MGR_GetNextDomainNameList
 * PURPOSE:
 *		This function get next the dns domain name from list.
 *
 *
 *
 * INPUT:
 *		I8_T *dns_ip_domain_name    --  current dommain name of list.
 *
 * OUTPUT:
 *		I8_T *dns_ip_domain_name    --  next dommain name of list.
 *
 * RETURN:
 *		TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *		 the initial name is empty string.
 *      This function will be called by CLI aommand "show dns"
 */
BOOL_T DNS_MGR_GetNextDomainNameList(char *dns_ip_domain_name);



/* FUNCTION NAME : DNS_MGR_GetDomainNameList
 * PURPOSE:
 *		This function get the dns domain name from list.
 *
 *
 *
 * INPUT:
 *		I8_T *dns_ip_domain_name    --  current dommain name of list.
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *      This function will be called by snmp module.
 */
BOOL_T DNS_MGR_GetDomainNameList(I8_T *dns_ip_domain_name);



/* FUNCTION NAME : DNS_MGR_GetNextNameServerList
 * PURPOSE:
 *		This function get next the domain name server from list.
 *
 *
 * INPUT:
 *      L_INET_AddrIp_T *ip  --  current doamin name server ip.
 *
 * OUTPUT:
 *      L_INET_AddrIp_T *ip  --  next doamin name server ip.
 *
 * RETURN:
 *		TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *		The initial ip value is zero.
 *      This function will be called by CLI command "show dns"
 */
BOOL_T DNS_MGR_GetNextNameServerList(L_INET_AddrIp_T *ip_p);



/* FUNCTION NAME : DNS_MGR_GetNameServerList
 * PURPOSE:
 *		This function get the domain name server from list.
 *
 *
 * INPUT:
 *		UI32_T *ip  --  current doamin name server ip.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *      This function will be called by snmp module.
 */
BOOL_T DNS_MGR_GetNameServerList(L_INET_AddrIp_T *ip);


/* FUNCTION NAME : DNS_MGR_GetNextCacheEntry
 * PURPOSE:
 *		This function get next entry from dns cache.
 *
 *
 * INPUT:
 *		UI32_T *index   --  current index of cache entry.
 *
 * OUTPUT:
 *      UI32_T *index                  --  next index of cache entry.
 *      DNS_CacheRecord_T *cache_entry   --  next cache entry.
 *
 * RETURN:
 *		TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *		The initial index value is -1.
 *      This function will be called by CLI command "show dns cache"
 */
BOOL_T DNS_MGR_GetNextCacheEntry(I32_T *index, DNS_CacheRecord_T *cache_entry);

/* FUNCTION NAME : DNS_MGR_GetNextCacheEntryForSNMP
 * PURPOSE:
 *		This function get next entry from dns cache.
 *
 *
 * INPUT:
 *		UI32_T *index   --  current index of cache entry.
 *
 * OUTPUT:
 *      UI32_T *index                  --  next index of cache entry.
 *      DNS_CacheRecord_T *cache_entry   --  next cache entry.
 *
 * RETURN:
 *		TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *		The initial index value is 0.
 *      This function will be used by SNMP.
 */
BOOL_T DNS_MGR_GetNextCacheEntryForSNMP(I32_T *index, DNS_CacheRecord_T *cache_entry);


/* FUNCTION NAME : DNS_MGR_GetCacheEntryForSNMP
 * PURPOSE:
 *		This function get entry from dns cache.
 *
 *
 * INPUT:
 *		UI32_T index   --  current index of cache entry.
 *
 * OUTPUT:
 *      DNS_CacheRecord_T *cache_entry   --  current cache entry.
 *
 * RETURN:
 *		TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *      This function will be called by snmp module, initial index=0.
 */
BOOL_T DNS_MGR_GetCacheEntryForSNMP(I32_T index, DNS_CacheRecord_T *cache_entry);


/* FUNCTION NAME:  DNS_MGR_GetRunningDnsStatus
 * PURPOSE:
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific Sshd Status with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T * - Dns Status.
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  DNS_MGR_GetRunningDnsStatus(UI32_T *state);



/* FUNCTION NAME:  DNS_MGR_GetNextRunningDnsHostEntry
 * PURPOSE:
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific Sshd Status with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *		    int *host_index_p -- previous active index of HostEntry_T struct.
 *
 * OUTPUT:
 *		    int *host_index_p -- current active index of HostEntry_T struct.
 *		    HostEntry_PTR *  -- a pointer to a HostEntry_T variable to store the returned value.
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 *		    The initial value is -1.
 */
SYS_TYPE_Get_Running_Cfg_T  DNS_MGR_GetNextRunningDnsHostEntry(int *host_index_p,HostEntry_PTR dns_host_entry_t_p);



/* FUNCTION NAME:  DNS_MGR_GetNextRunningDomainNameList
 * PURPOSE:
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific Sshd Status with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *		    I8_T *dns_ip_domain_name    --  current dommain name of list.
 *
 * OUTPUT:
 *		    I8_T *dns_ip_domain_name    --  next dommain name of list.
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 *		    the initial name is empty string.
 */
SYS_TYPE_Get_Running_Cfg_T  DNS_MGR_GetNextRunningDomainNameList(char *dns_ip_domain_name);



/* FUNCTION NAME:  DNS_MGR_GetNextRunningNameServerList
 * PURPOSE:
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific Sshd Status with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *		    UI32_T *ip  --  current doamin name server ip.
 *
 * OUTPUT:
 *          UI32_T *ip  --  next doamin name server ip.
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 *		    The initial ip value is zero.
 */
SYS_TYPE_Get_Running_Cfg_T  DNS_MGR_GetNextRunningNameServerList(L_INET_AddrIp_T *ip_p);



/* FUNCTION NAME:  DNS_MGR_GetRunningDnsIpDomain
 * PURPOSE:
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific Sshd Status with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI8_T * -- ipdomain name .
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  DNS_MGR_GetRunningDnsIpDomain(UI8_T *ipdomain);



/* FUNCTION NAME : DNS_MGR_SetDnsResConfigMaxCnames
 * PURPOSE:
 *  Limit on how many CNAMEs the resolver should allow
 *  before deciding that there's a CNAME loop.  Zero means
 *  that resolver has no explicit CNAME limit.
 * INPUT:
 *  UI32_T  -- dns_resconfig_max_cnames
 *
 * OUTPUT:
 *  none.
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by snmp module.
 */
int DNS_MGR_SetDnsResConfigMaxCnames(UI32_T dns_resconfig_max_cnames_p);



/* FUNCTION NAME : DNS_MGR_SetDnsResConfigSbeltName
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
int DNS_MGR_SetDnsResConfigSbeltName(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);



/* FUNCTION NAME : DNS_MGR_SetDnsResConfigSbeltRecursion
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
int DNS_MGR_SetDnsResConfigSbeltRecursion(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);



/* FUNCTION NAME : DNS_MGR_SetDnsResConfigSbeltPref
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
int DNS_MGR_SetDnsResConfigSbeltPref(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);



/* FUNCTION NAME : DNS_MGR_SetDnsResConfigSbeltStatus
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
int DNS_MGR_SetDnsResConfigSbeltStatus(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);



/* FUNCTION NAME : DNS_MGR_GetDefaultDnsResConfigSbeltEntry
 * PURPOSE:
 *  This funciton get the specified DnsResConfigSbeltEntry according to the index.
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  DNS_ResConfigSbeltEntry_T* -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the returned value
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDefaultDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);



/* FUNCTION NAME : DNS_MGR_GetDnsHostEntryBySnmp
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
 *      This function will be called by snmp
 */
int DNS_MGR_GetDnsHostEntryBySnmp(char *hostname_p, char *hostaddr_p);



/* FUNCTION NAME : DNS_MGR_GetNextDnsHostEntryBySnmp
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
 *      This function will be called by snmp
 *		    the initial name is empty string.
 *		    The initial ip value is 0.0.0.0.
 */
int DNS_MGR_GetNextDnsHostEntryBySnmp(UI8_T *hostname_p, UI8_T *hostaddr_p);



/* FUNCTION NAME : DNS_MGR_GetDnsAliasNameBySnmp
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
 *      This function will be called by snmp
 */
int DNS_MGR_GetDnsAliasNameBySnmp(I8_T *hostname_p, I8_T *aliasname_p);



/* FUNCTION NAME : DNS_MGR_GetNextDnsAliasNameBySnmp
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
 *      This function will be called by snmp
 *		    the initial name is empty string.
 */
int DNS_MGR_GetNextDnsAliasNameBySnmp(I8_T *hostname_p, I8_T *aliasname_p);



/* FUNCTION NAME : DNS_MGR_SetNameServerByIndex
 * PURPOSE:
 *  This function adds a name server IP address to the name server list by index.
 *
 *
 * INPUT:
 *      index   --  index of name server.
 *      ip_addr --  ip addr will be added or deleded.
 *      is_add  --  add or delete
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *      This function will be called by SNMP.
 */
int DNS_MGR_SetNameServerByIndex(UI32_T index, L_INET_AddrIp_T* ip_addr, BOOL_T is_add);



/* FUNCTION NAME : DNS_MGR_GetNameServerByIndex
 * PURPOSE:
 *		This function get the domain name server by index from list.
 *
 *
 * INPUT:
 *      UI32_T  index   --  index of name server.
 *		UI32_T  *ip     --  current doamin name server ip.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *      This function will be called by snmp module.
 */
BOOL_T DNS_MGR_GetNameServerByIndex(UI32_T index, L_INET_AddrIp_T *ip);



/* FUNCTION NAME : DNS_MGR_GetNextNameServerByIndex
 * PURPOSE:
 *		This function get next the domain name server by indx from list.
 *
 *
 * INPUT:
 *      UI32_T  *index  --  index of current doamin name server ip.
 *
 * OUTPUT:
 *      UI32_T  *index  --  index of next doamin name server ip.
 *      UI32_T  *ip     --  next doamin name server ip.
 *
 * RETURN:
 *		TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *		The initial ip value is zero.
 *      This function will be called by SNMP.
 */
BOOL_T DNS_MGR_GetNextNameServerByIndex(UI32_T *index, L_INET_AddrIp_T *ip);



/* FUNCTION NAME : DNS_MGR_DeleteAllNameServer
 * PURPOSE:
 *  This function deletes all name server entry from
 *  the name server list .
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *      This function will be called by CLI command "no ip name-server".
 */
int DNS_MGR_DeleteAllNameServer(void);



/* PENDING:
 * This seems to be a linked list which sorts name length first,
 * and then name value, possibly used by SNMP.
 * This code causes a PGRelief warning pgr0060 on the variable "prevHostent_P",
 * as it may be NULL but is used to point (->) to its content.
 * If this code for sure will not be used, it will be removed from the code.
 */
#if 0  /*!*/ /* PENDING: DNS_MGR_*DnsHostEntryByNameAndIndex; suspected unused code */
/* FUNCTION NAME : DNS_MGR_GetNextDnsHostEntryByNameAndIndex
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
 *      This function will be called by snmp
 *		    the initial name is empty string.
 *          The initial index is 0.
 *		    The initial ip value is 0.0.0.0.
 */
int DNS_MGR_GetNextDnsHostEntryByNameAndIndex(char *hostname_p, I32_T *index_p, char *hostaddr_p);



/* FUNCTION NAME : DNS_MGR_SetDnsHostEntryByNameAndIndex
 * PURPOSE:
 *  This function set a IP address to the host table list by host name & index.
 *
 *
 * INPUT:
 *      *hostname_p --  host name.
 *      index       --  index of host addr for host name.
 *      ip_addr     --  ip addr will be added as a name server.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *      This function will be called by SNMP.
 */
int DNS_MGR_SetDnsHostEntryByNameAndIndex(char *hostname_p, UI32_T index, L_INET_AddrIp_T *ip_addr);



/* FUNCTION NAME : DNS_MGR_GetDnsHostEntryByNameAndIndex
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
 *      This function will be called by SNMP.
 */
int DNS_MGR_GetDnsHostEntryByNameAndIndex(char *hostname_p, UI32_T index, char *hostaddr_p);
#endif  /*!*/ /* 0; PENDING: DNS_MGR_*DnsHostEntryByNameAndIndex; suspected unused code */


/* River@May 7, 2008, add nslookup mib */

/* FUNCTION NAME : DNS_MGR_GetNextLookupCtlTable
 *
 * PURPOSE:
 *  This function get next Nslookup control table .
 *
 * INPUT:
 *  CTRL_table    --  Nslookup control table
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_OK    -- :success
 *  DNS_ERROR -- :failure
 *
 * NOTES:
 *  This function will be called by rfc2925-nslookup.
 *
 */

int DNS_MGR_GetNextLookupCtlTable(DNS_Nslookup_CTRL_T *CTRL_table);


/* FUNCTION NAME : DNS_MGR_GetLookupCtlTable
 *
 * PURPOSE:
 *  This function get Nslookup control table .
 *
 * INPUT:
 *  CTRL_table    --  Nslookup control table
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_OK    -- :success
 *  DNS_ERROR -- :failure
 *
 * NOTES:
 *  This function will be called by rfc2925-nslookup.
 *
 */

int DNS_MGR_GetLookupCtlTable(DNS_Nslookup_CTRL_T *CTRL_table);

/* FUNCTION NAME : DNS_MGR_SetDNSCtlTable_TargetAddressType
 *
 * PURPOSE:
 *  This function set Nslookup control table TargetAddressType.
 *
 * INPUT:
 *  CTRL_table    --  Nslookup control table
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_OK    -- :success
 *  DNS_ERROR -- :failure
 *
 * NOTES:
 *  This function will be called by rfc2925-nslookup.
 *
 */

int DNS_MGR_SetDNSCtlTable_TargetAddressType(DNS_Nslookup_CTRL_T *CTRL_table);


/* FUNCTION NAME : DNS_MGR_SetDNSCtlTable_TargetAddress
 *
 * PURPOSE:
 *  This function set Nslookup control table TargetAddress.
 *
 * INPUT:
 *  CTRL_table    --  Nslookup control table
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_OK    -- :success
 *  DNS_ERROR -- :failure
 *
 * NOTES:
 *  This function will be called by rfc2925-nslookup.
 *
 */

int DNS_MGR_SetDNSCtlTable_TargetAddress(DNS_Nslookup_CTRL_T *CTRL_table);


/* FUNCTION NAME : DNS_MGR_SetDNSCtlTable_RowStatus
 *
 * PURPOSE:
 *  This function set Nslookup control table RowStatus.
 *
 * INPUT:
 *  CTRL_table    --  Nslookup control table
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_OK     -- :success
 *  DNS_ERROR  -- :failure
 *
 * NOTES:
 *  This function will be called by rfc2925-nslookup.
 *
 */
int DNS_MGR_SetDNSCtlTable_RowStatus(DNS_Nslookup_CTRL_T *CTRL_table);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_MGR_CreateSystemNslookupCtlEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Create nslookup control entry which control owner is system.
 * INPUT    : ctl_entry_p  -- nslookup control entry
 * OUTPUT   : ctl_index_p  -- 0-based nslookup control entry index
 * RETUEN   : DNS_OK/DNS_ERROR
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
int
DNS_MGR_CreateSystemNslookupCtlEntry(
    DNS_Nslookup_CTRL_T *ctl_entry_p,
    UI32_T *ctl_index_p
);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_MGR_GetNslookupCtlEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get nslookup control entry by index.
 * INPUT    : ctl_index       -- 0-based nslookup control entry index
 * OUTPUT   : ctl_entry_p     -- nslookup control entry
 * RETUEN   : DNS_OK/DNS_ERROR
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
int
DNS_MGR_GetNslookupCtlEntryByIndex(
    UI32_T ctl_index,
    DNS_Nslookup_CTRL_T *ctl_entry_p
);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_MGR_GetNslookupResultEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get nslookup result entry by index.
 * INPUT    : ctl_index       -- 0-based nslookup control entry index
 *            result_index     -- 0-based nslookup result entry index
 * OUTPUT   : results_entry_p  -- nslookup result entry
 * RETUEN   : DNS_OK/DNS_ERROR
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
int
DNS_MGR_GetNslookupResultEntryByIndex(
    UI32_T ctl_index,
    UI32_T result_index,
    DNS_Nslookup_Result_T *results_entry_p
);

/* FUNCTION NAME : DNS_MGR_SetResResetStatus
 *
 * PURPOSE:
 *  Set resolver reset status.
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  none.
 *
 * NOTES:
 *  none.
 */
BOOL_T DNS_MGR_SetResResetStatus(int status);



/* FUNCTION NAME - DNS_MGR_HandleHotInsertion
 *
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is inserted at a time.
 */
void DNS_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);



/* FUNCTION NAME - DNS_MGR_HandleHotRemoval
 *
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is removed at a time.
 */
void DNS_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DNS_MGR_CheckNameServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : check input address is in suitable range.
 * INPUT    : ipaddress
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *			  FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T DNS_MGR_CheckNameServerIp(L_INET_AddrIp_T *ipaddress_p);

/* FUNCTION NAME : DNS_Nslookup_TargetAddrCheck
 * PURPOSE:
 *  This function  check nslookup control table length.
 *
 *
 * INPUT:
 *     addr_len   --  nslookup target address length.
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  RC_DNS_ERROR :failure,
 *  RC_DNS_OK    :success.
 * NOTES:
 *
 */
BOOL_T DNS_Nslookup_TargetAddrCheck(void *addr_len);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_MGR_HandleIPCReqMsg
 *-------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for dns mgr.
 * INPUT   : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT  : ipcmsg_p  --  output response ipc message buffer
 * RETUEN  : TRUE  - need to send response.
 *           FALSE - not need to send response.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T DNS_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *ipcmsg_p);

/* FUNCTION NAME : DNS_MGR_CreateDomainNameListEntry
 * PURPOSE: To create a new dnsDomainListEntry in dnsDomainListTable.
 * INPUT  : idx   -- index of dnsDomainListEntry
 *                   (1-based, key to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 *          2. default name is 'domainname1', 'domainname2', ...
 */
BOOL_T DNS_MGR_CreateDomainNameListEntry(UI32_T idx);

/* FUNCTION NAME : DNS_MGR_DestroyDomainNameListEntry
 * PURPOSE: To destroy a dnsDomainListEntry in dnsDomainListTable.
 * INPUT  : idx   -- index of dnsDomainListEntry
 *                   (1-based, key to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_MGR_DestroyDomainNameListEntry(UI32_T idx);

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
BOOL_T DNS_MGR_SetDomainNameListEntry(UI32_T idx, char *domain_name_p);

/* FUNCTION NAME : DNS_MGR_CreateDnsHostEntry
 * PURPOSE: To create a new host entry in dnsHostEntry table.
 * INPUT  : host_idx   -- index of dnsHostEntry
 *                        (1-based, key to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 *          2. default name is 'hostname1', 'hostname2', 'hostname3'...
 */
BOOL_T DNS_MGR_CreateDnsHostEntry(UI32_T host_idx);

/* FUNCTION NAME : DNS_MGR_DestroyDnsHostEntry
 * PURPOSE: To destroy a host entry in dnsHostEntry table.
 * INPUT  : host_idx   -- index of dnsHostEntry
 *                        (1-based, key to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_MGR_DestroyDnsHostEntry(UI32_T host_idx);

/* FUNCTION NAME : DNS_MGR_SetDnsHostEntry
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
BOOL_T DNS_MGR_SetDnsHostEntry(UI32_T host_idx, char *hostname_p);

/* FUNCTION NAME : DNS_MGR_GetDnsHostEntryForSnmp
 * PURPOSE: To get entry from the dnsHostEntry table.
 * INPUT  : host_idx   -- index of dnsHostEntry
 *                        (1-based, key to search the entry)
 * OUTPUT : hostname_p -- pointer to hostname content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_MGR_GetDnsHostEntryForSnmp(UI32_T host_idx, char *hostname_p);

/* FUNCTION NAME : DNS_MGR_GetNextDnsHostEntryForSnmp
 * PURPOSE: To get next entry from the dnsHostEntry table.
 * INPUT  : host_idx_p   -- index of dnsHostEntry
 *                          (1-based, 0 to get the first,
 *                           key to search the entry)
 * OUTPUT : host_idx_p   -- next index of dnsHostEntry
 *          hostname_p   -- pointer to hostname content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_MGR_GetNextDnsHostEntryForSnmp(UI32_T *host_idx_p, char *hostname_p);

/* FUNCTION NAME : DNS_MGR_SetDnsHostAddrEntry
 * PURPOSE: To add/delete an ip address to/from the dnsHostAddrEntry table.
 * INPUT  : host_idx  -- index of dnsHostAddrEntry to operate with (1-based)
 *		    addr_p    -- ip address content
 *                       (host_idx, addr_type, addr_p are
 *                        keys to search the entry)
 *		    is_add    -- true to add/ false to delete
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. sorted by ip address type & content.
 *          2. only support ipv4 now.
 *          3. for SNMP.
 *          4. 0.0.0.0 is not valid input.
 *          5. one addr can not be added to two different host entry.
 */
BOOL_T DNS_MGR_SetDnsHostAddrEntry(
    UI32_T host_idx, L_INET_AddrIp_T *addr_p, BOOL_T is_add);

/* FUNCTION NAME : DNS_MGR_GetDnsHostAddrEntry
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
BOOL_T DNS_MGR_GetDnsHostAddrEntry(
    UI32_T host_idx, L_INET_AddrIp_T *addr_p);

/* FUNCTION NAME : DNS_MGR_GetNextDnsHostAddrEntry
 * PURPOSE: To get next entry from the dnsHostAddrEntry table.
 * INPUT  : host_idx_p  -- index of dnsHostAddrEntry to get from
 *                         (1-based, 0 to get the first)
 *		    addr_p      -- ip address content
 *                         (host_idx_p, addr_type_p, addr_p are
 *                          keys to search the entry)
 * OUTPUT : host_idx_p  -- next index of dnsHostAddrEntry
 *          addr_type_p -- next ip address type (ex, v4/v6)
 *		    addr_p      -- next ip address content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. sorted by ip address type & content.
 *          2. only support ipv4 now.
 *          3. for SNMP.
 */
BOOL_T DNS_MGR_GetNextDnsHostAddrEntry(
    UI32_T *host_idx_p, L_INET_AddrIp_T *addr_p);

#endif  /* #ifndef DNS_MGR_H */
