/* MODULE NAME: dns_pmgr.h
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

#ifndef DNS_PMGR_H
#define DNS_PMGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "dns_mgr.h"
/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

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
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DNS_PMGR_InitiateProcessResource(void);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_AddDomainName
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_AddDomainName(char *domain_name_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_AddDomainNameToList
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_AddDomainNameToList(char *domain_name_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_AddNameServer
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_AddNameServer(L_INET_AddrIp_T *addr_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_ClearDnsCache
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_ClearDnsCache(void);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_ClearHosts
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T DNS_PMGR_ClearHosts(void);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_DeleteAllNameServer
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_DeleteAllNameServer(void);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_DeleteDomainName
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_DeleteDomainName(void);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_DeleteDomainNameFromList
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_DeleteDomainNameFromList(char *domain_name_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_DeleteNameServer
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : L_INET_AddrIp_T addr
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_DeleteNameServer(L_INET_AddrIp_T *addr_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_DisableDomainLookup
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T DNS_PMGR_DisableDomainLookup(void);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_EnableDomainLookup
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T DNS_PMGR_EnableDomainLookup(void);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetCacheEntryForSNMP
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T DNS_PMGR_GetCacheEntryForSNMP(I32_T index, DNS_CacheRecord_T *cache_entry);


/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDefaultDnsResConfigSbeltEntry
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDefaultDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsAliasNameBySnmp
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsAliasNameBySnmp(I8_T *hostname_p, I8_T *aliasname_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsHostEntry
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsHostEntry(int host_index_p,HostEntry_PTR dns_host_entry_t_p);

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
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsHostEntryByNameAndIndex(I8_T *hostname_p, UI32_T index, I8_T *hostaddr_p);
#endif  /*!*/ /* 0; PENDING: DNS_PMGR_*DnsHostEntryByNameAndIndex; suspected unused code */

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsHostEntryBySnmp
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsHostEntryBySnmp(I8_T *hostname_p, I8_T *hostaddr_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsIpDomain
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsIpDomain(char* ipdomain);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsLocalMaxRequests
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int  DNS_PMGR_GetDnsLocalMaxRequests(int *dns_config_local_max_requests_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsResCacheBadCaches
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsResCacheBadCaches(int *dns_res_cache_bad_caches_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsResCacheGoodCaches
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used for getting the number of rrs the
 *      			resolver has cached successfully.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsResCacheGoodCaches(int *dns_res_good_caches_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsResCacheMaxTTL
 * ------------------------------------------------------------------------|
 * FUNCTION :  This fuction will be called by snmp module.
 *  					If the resolver does not implement a TTL ceiling, the value
 *						 of this field should be zero.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsResCacheMaxTTL(UI32_T *dns_res_cache_max_ttl_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsResCacheStatus
 * ------------------------------------------------------------------------|
 * FUNCTION :  This function is used for getting the cache status.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsResCacheStatus(int *dns_res_cache_status_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsResConfigMaxCnames
 * ------------------------------------------------------------------------|
 * FUNCTION :  Limit on how many CNAMEs the resolver should allow
 *  before deciding that there's a CNAME loop.  Zero means
 *  that resolver has no explicit CNAME limit.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsResConfigMaxCnames(int *dns_resconfig_max_cnames_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsResConfigReset
 * ------------------------------------------------------------------------|
 * FUNCTION :  This function gets the time elapsed since it started.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsResConfigReset(int *config_reset_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsResConfigReset
 * ------------------------------------------------------------------------|
 * FUNCTION :  This function gets the time elapsed since it started.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_MGR_GetDnsResConfigResetTime(int *config_reset_time_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsResConfigService
 * ------------------------------------------------------------------------|
 * FUNCTION : Get kind of DNS resolution service provided .
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsResConfigService(int *dns_res_config_service_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsResConfigSbeltEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :  This funciton get the specified DnsResConfigSbeltEntry according to the index.
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsResConfigUpTime
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsResConfigUpTime(int *config_up_time_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetDnsStatus
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetDnsStatus(void);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsStatus
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T DNS_PMGR_SetDnsStatus(int status);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsResCacheMaxTTL
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsResCacheMaxTTL(int *dns_res_cache_max_ttl_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsResCacheStatus
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsResCacheStatus (int *dns_res_cache_status_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsResConfigMaxCnames
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsResConfigMaxCnames(UI32_T dns_resconfig_max_cnames_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsResConfigReset
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsResConfigReset(int *config_reset_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsResConfigSbeltEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsResConfigSbeltName
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsResConfigSbeltName(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_MGR_SetDnsResConfigSbeltPref
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_MGR_SetDnsResConfigSbeltPref(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsResConfigSbeltRecursion
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsResConfigSbeltRecursion(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsResConfigSbeltStatus
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsResConfigSbeltStatus(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsServConfigRecurs
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsServConfigRecurs(int *config_recurs_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetDnsServConfigReset
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsServConfigReset(int *dns_serv_config_reset_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextCacheEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T DNS_PMGR_GetNextCacheEntry(I32_T *index, DNS_CacheRecord_T *cache_entry);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextCacheEntryForSNMP
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T DNS_PMGR_GetNextCacheEntryForSNMP(I32_T *index, DNS_CacheRecord_T *cache_entry);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextDnsAliasNameBySnmp
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetNextDnsAliasNameBySnmp(I8_T *hostname_p, I8_T *aliasname_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextDnsHostEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetNextDnsHostEntry(int *host_index_p,HostEntry_PTR dns_host_entry_t_p);

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
 * ROUTINE NAME - DNS_PMGR_GetNextDnsHostEntryByNameAndIndex
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetNextDnsHostEntryByNameAndIndex(char *hostname_p, I32_T *index_p, I8_T *hostaddr_p);
#endif  /*!*/ /* 0; PENDING: DNS_PMGR_*DnsHostEntryByNameAndIndex; suspected unused code */

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextRunningDnsHostEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  DNS_PMGR_GetNextRunningDnsHostEntry(int *host_index_p,HostEntry_PTR dns_host_entry_t_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextRunningDomainNameList
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  DNS_PMGR_GetNextRunningDomainNameList(I8_T *dns_ip_domain_name);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextRunningNameServerList
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  DNS_PMGR_GetNextRunningNameServerList(L_INET_AddrIp_T *ip_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetRunningDnsIpDomain
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  DNS_PMGR_GetRunningDnsIpDomain(UI8_T *ipdomain);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetRunningDnsStatus
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  DNS_PMGR_GetRunningDnsStatus(UI32_T *state);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextDomainNameList
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T DNS_PMGR_GetNextDomainNameList(I8_T *dns_ip_domain_name);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextDnsResConfigSbeltEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetNextDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_HostAdd
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_HostAdd(char *hostName, L_INET_AddrIp_T *hostAddr_p);

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
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetDnsHostEntryByNameAndIndex(I8_T *hostname_p, UI32_T index, L_INET_AddrIp_T *ip_addr);
#endif  /*!*/ /* 0; PENDING: DNS_PMGR_*DnsHostEntryByNameAndIndex; suspected unused code */

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextDnsResCounterByOpcodeEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetNextDnsResCounterByOpcodeEntry(DNS_ResCounterByOpcodeEntry_T *dns_res_counter_by_opcode_entry_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextDnsResCounterByRcodeEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetNextDnsResCounterByRcodeEntry(DNS_ResCounterByRcodeEntry_T *dns_res_counter_by_rcode_entry_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextDnsServCounterEntry
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_GetNextDnsServCounterEntry(DNS_ServCounterEntry_T *dns_serv_counter_entry_t_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_GetNextNameServerByIndex
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T DNS_PMGR_GetNextNameServerByIndex(UI32_T *index, L_INET_AddrIp_T *ip);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_HostDelete
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_HostDelete(char *name, L_INET_AddrIp_T *addr_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_HostNameDelete
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_HostNameDelete(char * name);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_SetNameServerByIndex
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
int DNS_PMGR_SetNameServerByIndex(UI32_T index, L_INET_AddrIp_T *ip_addr, BOOL_T is_add);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_HostNameToIp
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set name server by index
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   :
 * NOTE		: none
 * ------------------------------------------------------------------------*/
I32_T DNS_PMGR_HostNameToIp(UI8_T *hostname, UI32_T family, L_INET_AddrIp_T hostip_ar[]);

int DNS_PMGR_SetDnsResConfigSbeltPref(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);

int DNS_PMGR_GetDnsServConfigReset(int *dns_serv_config_reset_p) ;
int DNS_PMGR_GetDnsResCounterByOpcodeEntry(DNS_ResCounterByOpcodeEntry_T *dns_res_counter_by_opcode_entry_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DNS_PMGR_CheckNameServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : check input address is in suitable range.
 * INPUT    : ipaddress
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *			  FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T DNS_PMGR_CheckNameServerIp(L_INET_AddrIp_T *ipaddress_p);

/* River@May 7, 2008, add nslookup mib */


int DNS_PMGR_GetNextLookupCtlTable(DNS_Nslookup_CTRL_T *CTRL_table);
int DNS_PMGR_GetLookupCtlTable(DNS_Nslookup_CTRL_T *CTRL_table);

int DNS_PMGR_SetDNSCtlTable_TargetAddressType(DNS_Nslookup_CTRL_T *CTRL_table);

int DNS_PMGR_SetDNSCtlTable_TargetAddress(DNS_Nslookup_CTRL_T *CTRL_table);

int DNS_PMGR_SetDNSCtlTable_RowStatus(DNS_Nslookup_CTRL_T *CTRL_table);

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
    UI32_T *ctl_index_p
);

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
    DNS_Nslookup_CTRL_T *ctl_entry_p
);

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
    DNS_Nslookup_Result_T *results_entry_p
);

int DNS_PMGR_GetNextLookupResultTable(DNS_Nslookup_Result_T *Result_table);

int DNS_PMGR_GetLookupResultTable(DNS_Nslookup_Result_T *Result_table);

int DNS_PMGR_Nslookup_DeleteEntry(UI32_T index);

BOOL_T DNS_PMGR_GetNslookupTimeOut(UI32_T *timeout, UI32_T index);
BOOL_T NS_PMGR_GetNslookupPurgeTime(UI32_T *purge_time);
BOOL_T DNS_PMGR_SetNslookupPurgeTime(UI32_T purge_time);

/* FUNCTION NAME : DNS_PMGR_CreateDomainNameListEntry
 * PURPOSE: To create a new dnsDomainListEntry in dnsDomainListTable.
 * INPUT  : idx   -- index of dnsDomainListEntry
 *                   (1-based, key to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 *          2. default name is 'domainname1', 'domainname2', ...
 */
BOOL_T DNS_PMGR_CreateDomainNameListEntry(UI32_T idx);

/* FUNCTION NAME : DNS_PMGR_DestroyDomainNameListEntry
 * PURPOSE: To destroy a dnsDomainListEntry in dnsDomainListTable.
 * INPUT  : idx   -- index of dnsDomainListEntry
 *                   (1-based, key to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_PMGR_DestroyDomainNameListEntry(UI32_T idx);

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
BOOL_T DNS_PMGR_SetDomainNameListEntry(UI32_T idx, I8_T *domain_name_p);

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
BOOL_T DNS_PMGR_CreateDnsHostEntry(UI32_T host_idx);

/* FUNCTION NAME : DNS_PMGR_DestroyDnsHostEntry
 * PURPOSE: To destroy a host entry in dnsHostEntry table.
 * INPUT  : host_idx   -- index of dnsHostEntry
 *                        (1-based, key to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_PMGR_DestroyDnsHostEntry(UI32_T host_idx);

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
BOOL_T DNS_PMGR_SetDnsHostEntry(UI32_T host_idx, I8_T *hostname_p);

/* FUNCTION NAME : DNS_PMGR_GetDnsHostEntryForSnmp
 * PURPOSE: To get entry from the dnsHostEntry table.
 * INPUT  : host_idx   -- index of dnsHostEntry
 *                        (1-based, key to search the entry)
 * OUTPUT : hostname_p -- pointer to hostname content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_PMGR_GetDnsHostEntryForSnmp(UI32_T host_idx, I8_T *hostname_p);

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
BOOL_T DNS_PMGR_GetNextDnsHostEntryForSnmp(UI32_T *host_idx_p, I8_T *hostname_p);

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
    UI32_T host_idx, L_INET_AddrIp_T *addr_p, BOOL_T is_add);

/* FUNCTION NAME : DNS_PMGR_GetDnsHostAddrEntry
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
BOOL_T DNS_PMGR_GetDnsHostAddrEntry(
    UI32_T host_idx, L_INET_AddrIp_T *addr_p);

/* FUNCTION NAME : DNS_PMGR_GetNextDnsHostAddrEntry
 * PURPOSE: To get next entry from the dnsHostAddrEntry table.
 * INPUT  : host_idx_p  -- index of dnsHostAddrEntry to get from
 *                         (1-based, 0 to get the first)
 *		    addr_p      -- ip address content
 *                         (host_idx_p, addr_p are
 *                          keys to search the entry)
 * OUTPUT : host_idx_p  -- next index of dnsHostAddrEntry
 *		    addr_p      -- next ip address content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. sorted by ip address type & content.
 *          2. only support ipv4 now.
 *          3. for SNMP.
 */
BOOL_T DNS_PMGR_GetNextDnsHostAddrEntry(
    UI32_T *host_idx_p, L_INET_AddrIp_T *addr_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - DNS_PMGR_DeleteDnsCacheRR
 * ------------------------------------------------------------------------|
 * FUNCTION :
 * INPUT    : None
 * OUTPUT	:
 * RETURN   : TRUE/DNS_ERROR
 * NOTE		:
 * ------------------------------------------------------------------------*/
int DNS_PMGR_DeleteDnsCacheRR(I8_T * name);

BOOL_T DNS_PMGR_GetNameServerList(L_INET_AddrIp_T *serveraddr_p);

#endif  /* #ifndef DNS_PMGR_H */
