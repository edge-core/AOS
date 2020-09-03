/* MODULE NAME: dns_om.c
 * PURPOSE:
 *   Initialize the database resource and provide some get/set functions for accessing the
 *   dns database.
 *
 * NOTES:
 *
 * History:
 *       Date          -- Modifier,   Reason
 *       2002-10-23    -- Wiseway , created for convention.
 *       2002-11-08    -- Isiah, porting to ACP@2.0
 *
 * Copyright(C)      Accton Corporation, 2002
 */

/* INCLUDE FILE DECLARATIONS
 */
/* isiah.2004-01-06. remove all compile warring message.*/
#include "sys_type.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_DNS == TRUE)

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*!!PENDING: will be moved to cnmlib or systime
 */
#include <time.h>

#include "sys_adpt.h"
#include "sys_dflt.h"

#include "sys_module.h"
#include "l_mm.h"
#include "sysfun.h"
#include "l_inet.h"

#include "dns.h"
#include "dns_om.h"
#include "dns_type.h"
#include "dns_cache.h"
#include "dns_hostlib.h"
#include "dns_resolver.h"
#include "dns_cmm.h"

extern I32_T dns_inet_addr(register const char *cp);
extern struct hostent* gethostbyname(const I8_T* name, UI32_T *rc);

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#ifndef _countof
#define _countof(_Ary) (sizeof(_Ary)/sizeof(*_Ary))
#endif

#define DNS_OM_SBELT_TABLE_ENTRY_MALLOC()    (DNS_ResConfigSbelt_T *)L_MM_Malloc(sizeof(DNS_ResConfigSbelt_T), L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_OM_SBELT_TABLE_ENTRY))
#define DNS_OM_SBELT_TABLE_ENTRY_FREE(buf_p) L_MM_Free(buf_p)
#define DNS_OM_IP_DOMAIN_LIST_ENTRY_MALLOC()    (DNS_IpDomain_T*)L_MM_Malloc(sizeof(DNS_IpDomain_T), L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_OM_IP_DOMAIN_LIST_ENTRY))
#define DNS_OM_IP_DOMAIN_LIST_ENTRY_FREE(buf_p) L_MM_Free(buf_p)

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void DNS_OM_Name2Lower(const char* name, char* rname);
static int DNS_OM_LocalGetDomainNameListEntrySmallestIndex(void);
static BOOL_T DNS_OM_LocalAddDomainNameToListWithIndex(UI32_T idx, char *domain_name_p);
static BOOL_T DNS_OM_LocalIsDomainNameInList(char *domain_name_p);
static DNS_IpDomain_T *DNS_OM_LocalFindDomainNameListEntryByIndex(UI32_T idx);
static void DNS_OM_LocalGetDefDomainName(char *domain_name_p);
static void DNS_OM_LocalFreeDomainNameList(void);

static DNS_Nslookup_CTRL_T *
DNS_OM_LocalGetNslookupCtlEntryByIndex(
    UI32_T ctl_index
);

static DNS_Nslookup_Result_T *
DNS_OM_LocalGetNslookupResultEntryByIndex(
    UI32_T ctl_index,
    UI32_T result_index
);

/* STATIC VARIABLE DECLARATIONS
 */
/*isiah.*/
/*static SEM_ID  dns_om_semaphore_id;*/
static UI32_T   dns_om_semaphore_id;
static UI32_T   dns_om_resolver_task_id;

static DNS_Config_T gdns_config ;

static DNS_Config_T gdns_config_original_data=
                {
                        0,              /* optimize */
                        DNS_ENABLE,     /* status */
                        "",     /* indent */
                        DNS_DEF_LOCAL_REQUEST,      /* max local request */
                        0,              /* debug status */
                        DNS_DEF_TIME_OUT,               /* timeout */
                        NULL,           /* domain list */

                        {               /* resolver part */
                        DNS_RES_CFG_IMPL_ID,    /* indent */
                        VAL_dnsResConfigService_recursiveAndIterative,              /* Service */
                        0,              /* MAX cname */
                        NULL,           /* sbelt */
                        0,              /* uptime */
                        0,              /* reset time */
                        VAL_dnsResConfigReset_other,                /*reset status,other */
                        DNS_QUERY_LOCAL_FIRST   /* query order */
                        },

                        {
                        DNS_SERV_CFG_IMPL_ID,   /* proxy part */
                        VAL_dnsServConfigRecurs_available,              /* recursivible */
                        0,              /* uptime */
                        0,              /* reset time */
                        VAL_dnsServConfigReset_other,               /* reset status */
                        DNS_DEF_SERVER_REQUEST,             /* max request */
                        0,              /* current request */
                        1,              /* service enable */
                        DNS_SERV_INIT   /* server status */
                        },

                        {               /* cache part */
                        VAL_dnsResCacheStatus_enabled,      /* cache status,1 for enable */
                        SYS_ADPT_MAX_TTL_FOR_RRS_IN_CACHE,          /* max ttl, sendond */
                        0,              /* good caches */
                        0,              /* bad caches */
                        DNS_DEF_CACHE_SIZE      /* max entries */
                        }
                };

static DNS_ResCounter_T    gdns_res_counter;
static DNS_ProxyCounter_T  gdns_proxy_counter;

static  UI32_T  number_of_name_server_entry =   0;
static  UI32_T  number_of_domain_name_list  =   0;

/* River@May 7, 2008, add nslookup mib */
static DNS_Nslookup_CTRL_T      nslookup_ctrl_table[DNS_DEF_NSLOOKUP_REQUEST];
static DNS_Nslookup_Result_T    nslookup_result_table[DNS_DEF_NSLOOKUP_REQUEST][DNS_MAXNSLOOKUPHOSTIPNUM]; /*maggie liu, NSLOOKUP*/
static UI32_T                   nslookup_table_timeout[DNS_DEF_NSLOOKUP_REQUEST];
static UI32_T                   nslookup_purgetime;

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:  DNS_OM_Init
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
 *          This function is invoked in DNS_OM_Init.
 */
BOOL_T DNS_OM_Init(void)
{
    UI32_T orig_priority;

    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_DNS_OM, &dns_om_semaphore_id) != SYSFUN_OK)
    {
        printf("\n%s:get om sem id fail.\n", __FUNCTION__);
        return FALSE;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);
    /* initiate dns resolver counter */
    memset(&gdns_res_counter,0,sizeof(struct DNS_ResCounter_S));
    /* initiate dns server counter */
    memset(&gdns_proxy_counter,0,sizeof(struct DNS_ProxyCounter_S));

     /*maggie liu*/
    memcpy(&gdns_config, &gdns_config_original_data, sizeof(DNS_Config_T));

    /*maggie liu, NSLOOKUP*/
    nslookup_purgetime = DNS_DEF_NSLOOKUP_PTIME;
    memset(&nslookup_ctrl_table, 0, sizeof(nslookup_ctrl_table));
    memset(&nslookup_result_table, 0, sizeof(nslookup_result_table));
    memset(&nslookup_table_timeout, 0, sizeof(nslookup_table_timeout));
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return TRUE;
}

 /*maggie liu*/
/* FUNCTION NAME:  DNS_OM_ResetConfig
 * PURPOSE:
 *          Set the configure information to default for DNS
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
 *          This function is invoked in DNS_MGR_ResetConfig().
 */
void DNS_OM_ResetConfig()
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    memcpy(&gdns_config, &gdns_config_original_data, sizeof(DNS_Config_T));
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

}

/*the following is from liuheming */


/* FUNCTION NAME : DNS_OM_GetDnsServConfigImplementIdent
 * PURPOSE:
 *      This function gets the implementation identification string for the DNS server software in use on the system
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      I8_T * -- a pointer to a string to storing  the implementation
 *                identification string for the DNS server software
 *                in use on the system.
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetDnsServConfigImplementIdent(I8_T *dns_serv_config_implement_ident_p)
{
    UI32_T orig_priority;

    if(dns_serv_config_implement_ident_p == NULL)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    strcpy((char *)dns_serv_config_implement_ident_p, (char *)gdns_config.DnsProxyConfig.dnsServConfigImplementIdent);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_SetDnsServConfigRecurs
 * PURPOSE:
 *      This function sets the recursion services offered by this name server.
 *
 *
 * INPUT:
 *      int * -- A pointer to a variable to store the value to be set.
 *              This represents the recursion services offered by this
 *              name server.  The values that can be read or written are:
 *              available(1) - performs recursion on requests from clients.
 *              restricted(2) - recursion is performed on requests only
 *              from certain clients, for example; clients on an access
 *              control list.  It is not supported currently.
 *              unavailable(3) - recursion is not available.
 * OUTPUT:
 *
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_SetDnsServConfigRecurs(int *config_recurs_p)
{
    UI32_T orig_priority;

    if(config_recurs_p == NULL)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsProxyConfig.dnsServConfigRecurs = *config_recurs_p;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_SetDnsServConfigRecurs
 * PURPOSE:
 *      This function gets the recursion services offered by this name server.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      int * -- A pointer to a variable to store the returned value.
 *              This represents the recursion services offered by this
 *              name server.  The values that can be read or written are:
 *              available(1) - performs recursion on requests from clients.
 *              restricted(2) - recursion is performed on requests only
 *              from certain clients, for example; clients on an access
 *              control list.  It is not supported currently.
 *              unavailable(3) - recursion is not available.
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetDnsServConfigRecurs(int *config_recurs_p)
{
    UI32_T orig_priority;

    if(config_recurs_p == NULL)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *config_recurs_p = gdns_config.DnsProxyConfig.dnsServConfigRecurs;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_GetDnsServConfigUpTime
 * PURPOSE:
 *      This function get the up time since the server started.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *          UI32_T* -- a pointer to a variable to store the returned
 *                     value about the up time since the server started
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetDnsServConfigUpTime(UI32_T          *dns_serv_config_up_time_p)
{
    UI32_T orig_priority;

    if(dns_serv_config_up_time_p == NULL)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    if (DNS_SYS_CLK!=0)
    {
        *dns_serv_config_up_time_p = (SYSFUN_GetSysTick()-gdns_config.DnsProxyConfig.dnsServConfigUpTime)/DNS_SYS_CLK;
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_OK;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_ERROR;
}

/* FUNCTION NAME : DNS_OM_GetDnsServConfigResetTime
 * PURPOSE:
 *      This function gets the time elapsed since the last time the name server was `reset.'
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T* -- a pointer to a variable to stored the returned
 *                 value about the time elapsed since the last time
 *                 the name server was reset.
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetDnsServConfigResetTime(UI32_T          *dns_serv_config_reset_time)
{
    UI32_T orig_priority;

    if(dns_serv_config_reset_time == NULL)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    if (DNS_SYS_CLK!=0)
    {
        *dns_serv_config_reset_time = (SYSFUN_GetSysTick()-gdns_config.DnsProxyConfig.dnsServConfigResetTime)/DNS_SYS_CLK;
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_OK;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_ERROR;
}

/* FUNCTION NAME : DNS_OM_SetDnsServConfigReset
 * PURPOSE:
 *      This function reinitialize any persistant name server state.
 *
 *
 * INPUT:
 *      int * -- When set to reset(2), any persistant name server state
 *              (such as a process) is reinitialized as if the name
 *              server had just been started
 *
 * OUTPUT:
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *      Other values is inhibited.
 */
int DNS_OM_SetDnsServConfigReset(int *dns_serv_config_reset_p)
{
    UI32_T orig_priority;

    if(dns_serv_config_reset_p == NULL)
    {
        return DNS_ERROR;
    }

    if (*dns_serv_config_reset_p!=VAL_dnsServConfigReset_reset)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsProxyConfig.dnsServConfigReset = VAL_dnsServConfigReset_initializing;
    gdns_config.DnsProxyConfig.dnsServStatus=DNS_SERV_STATUS_STOP;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_GetDnsServConfigReset
 * PURPOSE:
 *      This funtion gets any persistant name server state.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to store the result
 *              other(1) - server in some unknown state;
 *              initializing(3) - server (re)initializing;
 *              running(4) - server currently running
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetDnsServConfigReset(int *dns_serv_config_reset_p)
{
    UI32_T orig_priority;

    if(dns_serv_config_reset_p == NULL)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_config_reset_p = gdns_config.DnsProxyConfig.dnsServConfigReset;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

 /*
 * FUNCTION NAME : DNS_OM_GetDnsServConfigResetTime
 *
 * PURPOSE:
 *      This function gets the time elapsed since the last time the name server was `reset.'
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      UI32_T * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServCounterAuthAns(int *dns_serv_counter_auth_ans_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_counter_auth_ans_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_counter_auth_ans_p = gdns_proxy_counter.dnsServCounterAuthAns;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsServCounterAuthNoNames
 *
 * PURPOSE:
 *      This function gets the Number of queries for which `authoritative no such name'
 *      responses were made.
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServCounterAuthNoNames(UI32_T *dns_serv_counter_auth_no_names_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_counter_auth_no_names_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_counter_auth_no_names_p = gdns_proxy_counter.dnsServCounterAuthNoNames;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsServCounterAuthNoDataResps
 *
 * PURPOSE:
 *      This function gets the Number of queries for which `authoritative no such data'
 *       (empty answer) responses were made
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServCounterAuthNoDataResps(int *dns_serv_counter_auth_no_data_resps_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_counter_auth_no_data_resps_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_counter_auth_no_data_resps_p = gdns_proxy_counter.dnsServCounterAuthNoDataResps;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}


 /*
 * FUNCTION NAME : DNS_OM_GetDnsServCounterNonAuthDatas
 *
 * PURPOSE:
 *      This function gets the Number of queries which were non-authoritatively
 *      answered (cached data)
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServCounterNonAuthDatas(int *dns_serv_counter_non_auth_datas_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_counter_non_auth_datas_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_counter_non_auth_datas_p = gdns_proxy_counter.dnsServCounterNonAuthDatas;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsServCounterNonAuthNoDatas
 *
 * PURPOSE:
 *      This funciton gets the number of Number of queries which were non-authoritatively
         answered with no data (empty answer)
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServCounterNonAuthNoDatas(UI32_T *dns_serv_counter_non_auth_no_datas_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_counter_non_auth_no_datas_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_counter_non_auth_no_datas_p = gdns_proxy_counter.dnsServCounterNonAuthNoDatas;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

 /*
 * FUNCTION NAME : DNS_OM_GetDnsServCounterReferrals
 *
 * PURPOSE:
 *      This function gets the Number of requests that were referred to other servers
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *       int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServCounterReferrals(int *dns_serv_counter_referrals_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_counter_referrals_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_counter_referrals_p = gdns_proxy_counter.dnsServCounterReferrals;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsServCounterErrors
 *
 * PURPOSE:
 *      This function gets the Number of requests the server has processed that were
 *      answered with errors (RCODE values other than 0 and 3).
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServCounterErrors(int *dns_serv_counter_errors_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_counter_errors_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_counter_errors_p = gdns_proxy_counter.dnsServCounterErrors;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsServCounterRelNames
 *
 * PURPOSE:
 *      This function gets the Number of requests received by the server for names that
 *       are only 1 label long (text form - no internal dots)
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServCounterRelNames(int *dns_serv_counter_rel_names_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_counter_rel_names_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_counter_rel_names_p = gdns_proxy_counter.dnsServCounterRelNames;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsServCounterReqRefusals
 *
 * PURPOSE:
 *      This function gets the Number of DNS requests refused by the server.
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *       int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServCounterReqRefusals(int *dns_serv_counter_req_refusals_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_counter_req_refusals_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_counter_req_refusals_p = gdns_proxy_counter.dnsServCounterReqRefusals;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsServCounterReqUnparses
 *
 * PURPOSE:
 *      This function gets the Number of requests received which were unparseable
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServCounterReqUnparses(int *dns_serv_counter_req_unparses_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_counter_req_unparses_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_counter_req_unparses_p = gdns_proxy_counter.dnsServCounterReqUnparses;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsServCounterOtherErrors
 *
 * PURPOSE:
 *      This function gets the Number of requests which were aborted for other (local)
 *      server errors
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *       int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServCounterOtherErrors(UI32_T *dns_serv_counter_other_errors_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_counter_other_errors_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_counter_other_errors_p = gdns_proxy_counter.dnsServCounterOtherErrors;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_SetDnsServConfigMaxRequests
 * PURPOSE:
 *      This fcuntion set the max number of requests in proxy.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      int * -- a pointer to a variable for storing the number of
 *              max requests in proxy. range:1-20. default:10
 *
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_SetDnsServConfigMaxRequests(int *dns_serv_config_max_requests_p)
{
    UI32_T orig_priority;

    if(dns_serv_config_max_requests_p == NULL)
        return DNS_ERROR;

    if ((*dns_serv_config_max_requests_p<DNS_MIN_SERVER_REQUEST)||(*dns_serv_config_max_requests_p>DNS_MAX_SERVER_REQUEST))
        return DNS_ERROR;       /*DNS_ERROR*/

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsProxyConfig.dnsServConfigMaxRequests = *dns_serv_config_max_requests_p;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/* FUNCTION NAME :DNS_OM_GetDnsServConfigMaxRequests
 * PURPOSE:
 *      This fcuntion gets the max number of requests in proxy.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      int * -- a pointer to a variable for storing the returned value
 *
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetDnsServConfigMaxRequests(int *dns_serv_config_max_requests_p)
{
    UI32_T orig_priority;

    if(dns_serv_config_max_requests_p == NULL)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_config_max_requests_p = gdns_config.DnsProxyConfig.dnsServConfigMaxRequests;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_SetDnsResCacheStatus
 * PURPOSE:
 *      This function is used for initializing dns cache
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      int * -- 1 enable cache,
 *               2 enable cache,
 *               3 clear the contents in the cache
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_SetDnsResCacheStatus (int *dns_res_cache_status_p)
{
    UI32_T orig_priority;

    if(dns_res_cache_status_p == NULL)
        return DNS_ERROR;
    if ((*dns_res_cache_status_p<VAL_dnsResCacheStatus_enabled)||(*dns_res_cache_status_p>VAL_dnsResCacheStatus_clear))
        return DNS_ERROR;       /*DNS_ERROR*/

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsResCache.cache_status = *dns_res_cache_status_p;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_GetDnsResCacheStatus
 * PURPOSE:
 *      This function is used for getting the cache status.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to store the returned cache status
 *               enabled(1),
 *               disabled(2)
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResCacheStatus(int *dns_res_cache_status_p)
{
    UI32_T orig_priority;

    if(dns_res_cache_status_p == NULL)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_res_cache_status_p = gdns_config.DnsResCache.cache_status;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_SetDnsResCacheMaxTTL
 * PURPOSE:
 *      This function is used for setting Maximum Time-To-Live for RRs in this cache.
 *      If the resolver does not implement a TTL ceiling, the value of this field should be zero.
 *
 * INPUT:
 *      int * -- a pointer to a variable for storing Maximum Time-To-Live for RRs in the cache
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_SetDnsResCacheMaxTTL(int *dns_res_cache_max_ttl_p)
{
    UI32_T orig_priority;

    if(dns_res_cache_max_ttl_p == NULL)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsResCache.cache_max_ttl = *dns_res_cache_max_ttl_p;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

 /* FUNCTION NAME : DNS_OM_GetDnsResCacheMaxTTL
 * PURPOSE:
 *      This fuction will be called by snmp module.
 *      If the resolver does not implement a TTL ceiling, the value of this field should be zero.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T * -- a pointer to a variable for storing the returned Maximum Time-To-Live for RRs in the cache
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResCacheMaxTTL(UI32_T *dns_res_cache_max_ttl_p)
{
    UI32_T orig_priority;

    if(dns_res_cache_max_ttl_p == NULL)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_res_cache_max_ttl_p = gdns_config.DnsResCache.cache_max_ttl;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_GetDnsResCacheGoodCaches
 * PURPOSE:
 *      This function is used for getting the number of rrs the resolver has cached successfully.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      int * -- a pointer to a variable for storing the number of rrs the resolver has cache successfully
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResCacheGoodCaches(int *dns_res_good_caches_p)
{
    UI32_T orig_priority;

    if(dns_res_good_caches_p == NULL)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_res_good_caches_p = gdns_config.DnsResCache.cache_good_caches;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_GetDnsResCacheBadCaches
 * PURPOSE:
 *      This function is used for getting the number of RRs the resolver has refused to cache.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to storing the returned number of rrs the resolver has refused to cache
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResCacheBadCaches(int *dns_res_cache_bad_caches_p)
{
    UI32_T orig_priority;

    if(dns_res_cache_bad_caches_p == NULL)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_res_cache_bad_caches_p = gdns_config.DnsResCache.cache_bad_caches;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
/*  semGive(&configSem);*/

    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_SetDnsResCacheMaxEntries
 * PURPOSE:
 *      This fcuntion set the max number of entries in cache.
 *
 *
 * INPUT:
 *      int * -- a pointer to a variable for storing the number of
 *              max entries in cache. range:1280-6400. default:2560
 *
 * OUTPUT:
 *      none.
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_SetDnsResCacheMaxEntries(int *dns_res_cache_max_entries_p)
{
    UI32_T orig_priority;

    if(dns_res_cache_max_entries_p == NULL)
    {
        return DNS_ERROR;
    }
    if ((*dns_res_cache_max_entries_p<DNS_MIN_CACHE_SIZE)||(*dns_res_cache_max_entries_p>DNS_MAX_CACHE_SIZE))
        return DNS_ERROR; /*DNS_ERROR*/

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsResCache.cache_max_entries = *dns_res_cache_max_entries_p;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_SetDnsResCacheMaxEntries
 * PURPOSE:
 *      This fcuntion get the max number of entries in cache.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      int * -- a pointer to a variable for storing the returned value
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResCacheMaxEntries(int *dns_res_cache_max_entries_p)
{
    UI32_T orig_priority;

    if(dns_res_cache_max_entries_p == NULL)
    {
        return DNS_ERROR;
    }
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_res_cache_max_entries_p = gdns_config.DnsResCache.cache_max_entries;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*the above is from liuheming */

/* FUNCTION NAME : DNS_OM_EnableDomainLookup
 * PURPOSE:
 *      This fcuntion enable DNS including resolver and proxy.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *
 */
void DNS_OM_EnableDomainLookup(void)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsStatus = DNS_ENABLE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
/*isiah.remove*/
/*  DNS_Init();*/
}

/* FUNCTION NAME : DNS_OM_DisableDomainLookup
 * PURPOSE:
 *      This fcuntion disable DNS including resolver and proxy.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *
 */
void DNS_OM_DisableDomainLookup(void)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsStatus = DNS_DISABLE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

/*isiah.move to DNS_TASK_ResolverMain() */
/*  DNS_RESOLVER_Disable(); */
}


/* FUNCTION NAME : DNS_OM_AddNameServer
 * PURPOSE:
 *      This function adds a name server IP address to the name server list.
 *
 *
 * INPUT:
 *      L_INET_AddrIp_T addr -- a ip addr will be added as a name server;
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_AddNameServer(L_INET_AddrIp_T *addr_p)
{
    DNS_ResConfigSbelt_T   *sbelt_table_p,*sbelt_new_node_p;
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */

    sbelt_table_p=gdns_config.DnsResConfig.dns_res_config_sbelt_table_p;

    if(NULL==sbelt_table_p)
    {
        sbelt_new_node_p=DNS_OM_SBELT_TABLE_ENTRY_MALLOC();
        if (NULL==sbelt_new_node_p)
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
            return DNS_ERROR;
        }

        memcpy(&sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr, addr_p, sizeof(L_INET_AddrIp_T));
        sbelt_new_node_p->next_p=NULL;
        sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltName[0]='\0';
        sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltClass=0;
        sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltSubTree[0]='\0';
        sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltPref=1;
        sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltStatus=0;
        sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltRecursion = DNS_RES_SERVICE_MIX;/*isiah*/
        gdns_config.DnsResConfig.dns_res_config_sbelt_table_p=sbelt_new_node_p;
        /*isiah*/
        number_of_name_server_entry = 1;
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_OK;
    }
    else
    {
        /* deal with the first node in the list
         */
        if (L_INET_CompareInetAddr((L_INET_Addr_T *) & sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr,
            (L_INET_Addr_T *) addr_p, 0) == 0)
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
            return DNS_ERROR;
        }

        while( NULL!=sbelt_table_p->next_p )
        {
            sbelt_table_p=sbelt_table_p->next_p;
            if (L_INET_CompareInetAddr((L_INET_Addr_T *) & sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr,
                (L_INET_Addr_T *) addr_p, 0) == 0)
            {
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
                return DNS_ERROR;
            }
        }

        if( number_of_name_server_entry < MAX_NBR_OF_NAME_SERVER_TABLE_SIZE )/*isiah*/
        {
            sbelt_new_node_p=DNS_OM_SBELT_TABLE_ENTRY_MALLOC();
            if (NULL==sbelt_new_node_p)
            {
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
                return DNS_ERROR;
            }

            memcpy(&sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr, addr_p, sizeof(L_INET_AddrIp_T));
            sbelt_new_node_p->next_p=NULL;
            sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltName[0]='\0';
            sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltClass=0;
            sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltSubTree[0]='\0';
            sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltPref=1;
            sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltStatus=0;
            sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltRecursion = DNS_RES_SERVICE_MIX;/*isiah*/
            sbelt_table_p->next_p=sbelt_new_node_p;

            number_of_name_server_entry += 1;
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
            return DNS_OK;
        }
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_ERROR;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_DeleteNameServer
 * PURPOSE:
 *      This function deletes a name server entry from
 *      the name server list accordint to the IP address of the name server.
 *
 * INPUT:
 *      I8_T * -- a pointer to a ip addr , whose related name server will be deleted;
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_DeleteNameServer(L_INET_AddrIp_T *addr_p)
{
    DNS_ResConfigSbelt_T   *sbelt_table_p,*prev_p;
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */

    sbelt_table_p=gdns_config.DnsResConfig.dns_res_config_sbelt_table_p;
    prev_p=sbelt_table_p;
    if(NULL==sbelt_table_p)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_ERROR;
    }
    else
    {
        if (L_INET_CompareInetAddr((L_INET_Addr_T *) & sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr,
            (L_INET_Addr_T *) addr_p, 0) == 0)
        {
            gdns_config.DnsResConfig.dns_res_config_sbelt_table_p=sbelt_table_p->next_p;
            DNS_OM_SBELT_TABLE_ENTRY_FREE(sbelt_table_p);
            sbelt_table_p = NULL;
            number_of_name_server_entry -= 1;/*isiah.*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
            return DNS_OK;
        }

        while  (sbelt_table_p->next_p)
        {
            prev_p=sbelt_table_p;
            sbelt_table_p=sbelt_table_p->next_p;
            if (L_INET_CompareInetAddr((L_INET_Addr_T *) & sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr,
                (L_INET_Addr_T *) addr_p, 0) == 0)
            {
                prev_p->next_p=sbelt_table_p->next_p;
                DNS_OM_SBELT_TABLE_ENTRY_FREE(sbelt_table_p);
                sbelt_table_p = NULL;
                number_of_name_server_entry -= 1;/*isiah.*/
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
                return DNS_OK; /*isiah.*/
            }
        }
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_ERROR;
 }

/* FUNCTION NAME : DNS_OM_ShowNameServerList
 * PURPOSE:
 *      This fcuntion displays the nam server informaion.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *
 */
void DNS_OM_ShowNameServerList(void)
{
    DNS_ResConfigSbelt_T   *sbelt_table_p;
    char ip[L_INET_MAX_IPADDR_STR_LEN+1];

    sbelt_table_p=gdns_config.DnsResConfig.dns_res_config_sbelt_table_p;
    while  (sbelt_table_p)
    {
        L_INET_InaddrToString((L_INET_Addr_T*)&sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr,
                              ip, sizeof(ip));

        printf("\n\tip\t:%s\n\tname\t:%s",ip,sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltName);
        sbelt_table_p=sbelt_table_p->next_p;
    }
}


/* FUNCTION NAME : DNS_OM_DebugOpen
 * PURPOSE:
 *      This fcuntion enables the displaying of debugging information.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *
 */
void DNS_OM_DebugOpen(void)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);
    gdns_config.DnsDebugStatus = DNS_ENABLE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_DebugClose
 * PURPOSE:
 *      This fcuntion disables the displaying of debugging information.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *
 */
void  DNS_OM_DebugClose(void)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);
    gdns_config.DnsDebugStatus = DNS_DISABLE;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_ClearDnsCache
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
 *      DNS_ERROR : failure,
 *      DNS_OK    : success
 * NOTES:
 *      This function will be called by configuration sub moudle.
 */
void DNS_OM_ClearDnsCache(void)
{
    UI32_T orig_priority;
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    DNS_CACHE_Reset();
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}


/* FUNCTION NAME : DNS_OM_ShowDnsDatabase
 * PURPOSE:
 *      This funciton is used for showing  cache database and status.
 *      Every field except link should be displayed, and the index of this cache
 *      entry should also be displayed.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
void  DNS_OM_ShowDnsDatabase(void)
{
    DNS_CACHE_ShowDatabase();
}

/* FUNCTION NAME : DNS_OM_ShowDnsCache
 * PURPOSE:
 *      This funciton is used for showing  cache database and status.
 *      Every field except link should be displayed, and the index of this cache
 *      entry should also be displayed.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success     .
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
void  DNS_OM_ShowDnsCache(void)
{
    DNS_CACHE_ShowDatabase();
}


/* FUNCTION NAME : DNS_OM_SetDnsLocalMaxRequests
 * PURPOSE:
 *      This funciton set the max number of local requests that resolver
 *      can deal with.
 *
 *
 * INPUT:
 *      int * -- a pointer to a variable for storing the number of
 *              local max requests. range:1..10. default:5
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_SetDnsLocalMaxRequests(int *dns_config_local_max_requests_p)
{
    UI32_T orig_priority;

    if (NULL==dns_config_local_max_requests_p)
        return DNS_ERROR;
    if ((*dns_config_local_max_requests_p<DNS_MIN_LOCAL_REQUEST)||(*dns_config_local_max_requests_p>DNS_MAX_LOCAL_REQUEST))
        return DNS_ERROR;    /*DNS_OUT_OF_RANGE*/

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsMaxLocalRequests = *dns_config_local_max_requests_p;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_GetDnsLocalMaxRequests
 * PURPOSE:
 *      This funciton set the max number of local requests that resolver
 *      can deal with.
 *
 *
 * INPUT:
 *      none.
 * OUTPUT:
 *      int * -- a pointer to a variable for storing the returned number of
 *              local max requests. range:1..10. default:5
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int  DNS_OM_GetDnsLocalMaxRequests(int *dns_config_local_max_requests_p)
{
    UI32_T orig_priority;

    if (NULL==dns_config_local_max_requests_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_config_local_max_requests_p = gdns_config.DnsMaxLocalRequests;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}


/*the following APIs is used for Resolver Mibs                         */

/* FUNCTION NAME : DNS_OM_GetDnsResConfigImplementIdent
 * PURPOSE:
 *      The implementation identification string for the
 *      resolver software in use on the system.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      I8_T * -- a pointer to the variable to store the returned value
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResConfigImplementIdent(I8_T *dns_res_config_implement_ident_p)
{
    UI32_T orig_priority;

    if (NULL==dns_res_config_implement_ident_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    strcpy((char *)dns_res_config_implement_ident_p,(char *)gdns_config.DnsResConfig.dnsResConfigImplementIdent);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

   return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_GetDnsResConfigService
 * PURPOSE:
 *      Get kind of DNS resolution service provided .
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to store the result
 *              recursiveOnly(1) indicates a stub resolver.
 *              iterativeOnly(2) indicates a normal full service resolver.
 *              recursiveAndIterative(3) indicates a full-service
 *              resolver which performs a mix of recursive and iterative queries.
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResConfigService(int *dns_res_config_service_p)
{
    UI32_T orig_priority;

    if (NULL==dns_res_config_service_p)
        return DNS_ERROR;
    else
    {
        orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
        *dns_res_config_service_p=gdns_config.DnsResConfig.dnsResConfigService;
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_OK;
    }
}



/* FUNCTION NAME : DNS_OM_GetDnsResConfigMaxCnames
 * PURPOSE:
 *      Limit on how many CNAMEs the resolver should allow
 *      before deciding that there's a CNAME loop.  Zero means
 *      that resolver has no explicit CNAME limit.
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResConfigMaxCnames(int *dns_resconfig_max_cnames_p)
{
    UI32_T orig_priority;

    if (NULL==dns_resconfig_max_cnames_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_resconfig_max_cnames_p=gdns_config.DnsResConfig.dnsResConfigMaxCnames;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_SetDnsResConfigSbeltEntry
 * PURPOSE:
 *      This funciton get the specified DnsResConfigSbeltEntry according to the index.
 *
 *
 *
 * INPUT:
 *      DNS_ResConfigSbeltEntry_T * -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the value to be set
 *                                   INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *      DNS_ResConfigSbeltEntry_T * -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the returned value.
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_SetDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
    DNS_ResConfigSbelt_T   *sbelt_table_p,*sbelt_new_node_p;
    L_INET_AddrIp_T ip_addr;
    UI32_T orig_priority;

    memset(&ip_addr, 0, sizeof(ip_addr));

    if (NULL==dns_res_config_sbelt_entry_t_p)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    sbelt_table_p=gdns_config.DnsResConfig.dns_res_config_sbelt_table_p;
    if (NULL==sbelt_table_p)
    {
        sbelt_new_node_p=DNS_OM_SBELT_TABLE_ENTRY_MALLOC();
        if (NULL==sbelt_new_node_p)
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
            return DNS_ERROR;
        }
        memcpy(&sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr, &dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltAddr, sizeof(L_INET_AddrIp_T));
        sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltClass=dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltClass;
        strcpy((char *)sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltSubTree, (char *)dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltSubTree);

        sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltName[0]='\0';
        sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltPref=1;
        sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltStatus=0;
        sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltRecursion = DNS_RES_SERVICE_MIX;

        sbelt_new_node_p->next_p=NULL;
        gdns_config.DnsResConfig.dns_res_config_sbelt_table_p=sbelt_new_node_p;
        number_of_name_server_entry = 1;
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_OK;
    }
    else
    {
        /* deal with the first node in the list
         */
        if (L_INET_CompareInetAddr((L_INET_Addr_T *) & sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr,
            (L_INET_Addr_T *) &ip_addr, 0) == 0)
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
            return DNS_ERROR;
        }

        while( NULL!=sbelt_table_p->next_p )
        {
            sbelt_table_p=sbelt_table_p->next_p;
            if (L_INET_CompareInetAddr((L_INET_Addr_T *) & sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr,
                (L_INET_Addr_T *) &ip_addr, 0) == 0)
            {
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
                return DNS_ERROR;
            }
        }

        if( number_of_name_server_entry < MAX_NBR_OF_NAME_SERVER_TABLE_SIZE )/*isiah*/
        {
            sbelt_new_node_p=DNS_OM_SBELT_TABLE_ENTRY_MALLOC();
            if (NULL==sbelt_new_node_p)
            {
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
                return DNS_ERROR;
            }

            memcpy(&sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr, &dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltAddr, sizeof(L_INET_AddrIp_T));
            sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltClass=dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltClass;
            strcpy((char *)sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltSubTree, (char *)dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltSubTree);

            sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltName[0]='\0';
            sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltPref=1;
            sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltStatus=0;
            sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltRecursion = DNS_RES_SERVICE_MIX;
            sbelt_new_node_p->next_p=NULL;
            sbelt_table_p->next_p=sbelt_new_node_p;
            number_of_name_server_entry += 1;
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
            return DNS_OK;
        }
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_ERROR;
}


/* FUNCTION NAME : DNS_OM_GetDnsResConfigSbeltEntry
 * PURPOSE:
 *      This funciton get the specified DnsResConfigSbeltEntry according to the index.
 *
 *
 * INPUT:
 *      DNS_ResConfigSbeltEntry_T* -- INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *      DNS_ResConfigSbeltEntry_T* -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the returned value
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
    DNS_ResConfigSbelt_T   *sbelt_table_p;
    L_INET_AddrIp_T ip_addr;
    UI32_T orig_priority;

    memset(&ip_addr, 0, sizeof(ip_addr));

    if (NULL==dns_res_config_sbelt_entry_t_p)
        return DNS_ERROR;

    sbelt_table_p=gdns_config.DnsResConfig.dns_res_config_sbelt_table_p;
    if (NULL==sbelt_table_p)
        return DNS_ERROR;

/* isiah.2004-01-06. remove all compile warring message.*/
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */

    memcpy(&ip_addr, &dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltAddr, sizeof(L_INET_AddrIp_T));

    while  (sbelt_table_p)
    {
        if (L_INET_CompareInetAddr((L_INET_Addr_T *) & sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr,
            (L_INET_Addr_T *) &ip_addr, 0) == 0)
        {
           strcpy((char *)dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltName,(char *)sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltName);
           dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltRecursion=sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltRecursion;
           dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltPref=sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltPref;
           dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltStatus=sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltStatus;
           SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
           return DNS_OK;
        }
        sbelt_table_p=sbelt_table_p->next_p;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_ERROR;
}

static int DNS_OM_Local_GetNextDnsResConfigSbeltEntry_seme(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
    DNS_ResConfigSbelt_T   *sbelt_table_p;
    L_INET_AddrIp_T ip_addr;

    if (NULL==dns_res_config_sbelt_entry_t_p)
        return DNS_ERROR;

    sbelt_table_p=gdns_config.DnsResConfig.dns_res_config_sbelt_table_p;
    if (NULL==sbelt_table_p)
    {
        return DNS_ERROR;
    }

    memset(&ip_addr,0, sizeof(ip_addr));

    memcpy(&ip_addr, &dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltAddr, sizeof(L_INET_AddrIp_T));

    if (ip_addr.addrlen == 0)
    {
        memcpy(&dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltAddr,
               &sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr,
               sizeof(L_INET_AddrIp_T));
        dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltSubTree[0]='\0';
        dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltClass=0;
        strcpy((char *)dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltName, (char *)sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltName);
        dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltRecursion=sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltRecursion;
        dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltPref=sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltPref;
        dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltStatus=sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltStatus;
        return DNS_OK;
    }

    while  (sbelt_table_p)
    {
        if (L_INET_CompareInetAddr((L_INET_Addr_T *) & sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr,
            (L_INET_Addr_T *) &ip_addr, 0) == 0)
        {
            if (NULL!=sbelt_table_p->next_p)
            {
                memcpy(&dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltAddr,
                       &sbelt_table_p->next_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr,
                       sizeof(L_INET_AddrIp_T));
                dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltSubTree[0]='\0';
                dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltClass=0;
                strcpy((char *)dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltName, (char *)sbelt_table_p->next_p->dns_res_config_sbelt_entry.dnsResConfigSbeltName);
                dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltRecursion=sbelt_table_p->next_p->dns_res_config_sbelt_entry.dnsResConfigSbeltRecursion;
                dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltPref=sbelt_table_p->next_p->dns_res_config_sbelt_entry.dnsResConfigSbeltPref;
                dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltStatus=sbelt_table_p->next_p->dns_res_config_sbelt_entry.dnsResConfigSbeltStatus;
                return DNS_OK;
            }
            else
            {
                return DNS_ERROR;
            }
        }
        sbelt_table_p=sbelt_table_p->next_p;
    }
    return DNS_ERROR;
}


/* FUNCTION NAME : DNS_OM_GetNextDnsResConfigSbeltEntry
 * PURPOSE:
 *      This function gets the DNS_ResConfigSbeltEntry_T  next to the specified index.
 *
 *
 * INPUT:
 *      DNS_ResConfigSbeltEntry_T * -- INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *      DNS_ResConfigSbeltEntry_T * -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the returned value
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetNextDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
    UI32_T orig_priority;
    int ret;

    if (NULL==dns_res_config_sbelt_entry_t_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    ret = DNS_OM_Local_GetNextDnsResConfigSbeltEntry_seme(dns_res_config_sbelt_entry_t_p);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return ret;
}


/* FUNCTION NAME : DNS_OM_GetDnsResConfigUpTime
 * PURPOSE:
 *      If the resolver has a persistent state (e.g., a
 *      process), this value will be the time elapsed since it
 *      started.  For software without persistant state, this
 *      value will be 0.
 * INPUT:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to storing config up time
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResConfigUpTime(int *config_up_time_p)
{
    UI32_T orig_priority;

   if (NULL== config_up_time_p)
     return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
   *config_up_time_p = (SYSFUN_GetSysTick() - gdns_config.DnsResConfig.dnsResConfigUpTime)/DNS_SYS_CLK;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

   return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_GetDnsResConfigResetTime
 * PURPOSE:
 *      This function gets the time elapsed since it started.
 *
 *
 *
 * INPUT:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to storing the returned config reset time
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResConfigResetTime(int *config_reset_time_p)
{
    UI32_T orig_priority;

    if (NULL==config_reset_time_p)
        return DNS_ERROR;
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *config_reset_time_p = (SYSFUN_GetSysTick() - gdns_config.DnsResConfig.dnsResConfigResetTime)/DNS_SYS_CLK;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_SetDnsResConfigReset
 * PURPOSE:
 *      This function reinitialize any persistant resolver state.
 *
 *
 *
 * INPUT:
 *      int * -- a pointer to a variable stored with the reset value,2 means reset
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_SetDnsResConfigReset(int *config_reset_p)
{
    UI32_T orig_priority;

    if (NULL==config_reset_p)
        return DNS_ERROR;

/*isiah.2003-10-30*/
    if(*config_reset_p != VAL_dnsResConfigReset_reset)
    {
        return DNS_ERROR;
    }
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsResConfig.dnsResConfigReset = VAL_dnsResConfigReset_reset;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_SetDnsResConfigQueryOrder
 * PURPOSE:
 *      This function  set the query order.
 *
 *
 *
 * INPUT:
 *      int * -- a pointer to a variable storing the value to be set
 *              DNS_QUERY_LOCAL_FIRST:Query static host table first;
 *              DNS_QUERY_DNS_FIRST: Query DNS server first
 *              default: DNS_QUERY_LOCAL_FIRST
 *              0 means uses default configuration
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_SetDnsResConfigQueryOrder(int *dns_res_config_query_order_p)
{
    UI32_T orig_priority;

    if (NULL==dns_res_config_query_order_p)
        return DNS_ERROR;

    if ((*dns_res_config_query_order_p<0)||(*dns_res_config_query_order_p>3))
        return DNS_ERROR;   /*DNS_OUT_OF_RANGE*/

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    if (*dns_res_config_query_order_p==0)
        *dns_res_config_query_order_p=DNS_QUERY_LOCAL_FIRST;
    else
        gdns_config.DnsResConfig.dnsResConfigQueryOrder=*dns_res_config_query_order_p;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_GetDnsResConfigQueryOrder
 * PURPOSE:
 *      This function  get the query order.
 *
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to store the returned value about query order
 *              DNS_QUERY_LOCAL_FIRST:Query static host table first;
 *              DNS_QUERY_DNS_FIRST: Query DNS server first
 *              DNS_QUERY_ DNS_ONLY: Query DNS server only
 *              default: DNS_QUERY_LOCAL_FIRST
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResConfigQueryOrder(int *dns_res_config_query_order_p)
{
    UI32_T orig_priority;

    if (NULL==dns_res_config_query_order_p)
        return DNS_ERROR;
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_res_config_query_order_p=gdns_config.DnsResConfig.dnsResConfigQueryOrder;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_GetDnsResConfigReset
 * PURPOSE:
 *      This function gets any persistant resolver state.
 *
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      int * -- a pointer to a variable for storing the returned resolver state
 *              other(1) - resolver in some unknown state;
 *              initializing(3) - resolver (re)initializing;
 *              running(4) - resolver currently running.
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResConfigReset(int *config_reset_p)
{
    UI32_T orig_priority;

    if (NULL==config_reset_p)
        return DNS_ERROR;
    else
    {
        orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
        *config_reset_p=gdns_config.DnsResConfig.dnsResConfigReset;
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_OK;
    }
}



/* FUNCTION NAME : DNS_OM_GetDnsResCounterNonAuthDataResps
 * PURPOSE:
 *      This functin gets the number of requests made by the resolver for which a
 *      non-authoritative answer (cached data) was received.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResCounterNonAuthDataResps(int *dns_res_counter_non_auth_data_resps_p)
{
    UI32_T orig_priority;

    if (NULL == dns_res_counter_non_auth_data_resps_p)
        return DNS_ERROR;
    else
    {
        orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
        *dns_res_counter_non_auth_data_resps_p = gdns_res_counter.dnsResCounterNonAuthDataResps;
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    }

    return DNS_OK;

}

/* FUNCTION NAME : DNS_OM_GetDnsResCounterNonAuthNoDataResps
 * PURPOSE:
 *      This fucniton gets Number of requests made by the resolver for which a
 *      non-authoritative answer - no such data response (empty answer) was received
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResCounterNonAuthNoDataResps(int *dns_res_counter_non_auth_no_data_resps_p)
{
    UI32_T orig_priority;

    if(NULL == dns_res_counter_non_auth_no_data_resps_p)
        return DNS_ERROR;
    else
    {
        orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
        *dns_res_counter_non_auth_no_data_resps_p = gdns_res_counter.dnsResCounterNonAuthNoDataResps;
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    }
    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_GetDnsResCounterMartians
 * PURPOSE:
 *      This function gets the number of responses received which were received from
 *      servers that the resolver does not think it asked.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResCounterMartians(UI32_T *dns_res_counter_martians_p)
{
    UI32_T orig_priority;

    if(NULL == dns_res_counter_martians_p)
        return DNS_ERROR;
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_res_counter_martians_p = gdns_res_counter.dnsResCounterMartians;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_GetDnsResCounterRecdResponses
 * PURPOSE:
 *      This function gets Number of responses received to all queries.
 *
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResCounterRecdResponses(UI32_T *dns_res_counter_recd_responses_p)
{
    UI32_T orig_priority;

    if(NULL == dns_res_counter_recd_responses_p)
        return DNS_ERROR;
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_res_counter_recd_responses_p = gdns_res_counter.dnsResCounterRecdResponses;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_GetDnsResCounterUnparseResps
 * PURPOSE:
 *      This function gets Number of responses received which were unparseable.
 *
 *
 *
 * INPUT:
 *      int * -- a pointer to a variable to store the result
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResCounterUnparseResps(UI32_T *dns_res_counter_unparse_resps_p)
{
    UI32_T orig_priority;

    if(NULL == dns_res_counter_unparse_resps_p)
        return DNS_ERROR;
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_res_counter_unparse_resps_p = gdns_res_counter.dnsResCounterUnparseResps;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_GetDnsResCounterFallbacks
 * PURPOSE:
 *      This function gets the number of times the resolver had to fall back to its seat belt information.
 *
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResCounterFallbacks(int *dns_res_counter_fallbacks_p)
{
    UI32_T orig_priority;

    if(NULL == dns_res_counter_fallbacks_p)
        return DNS_ERROR;
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_res_counter_fallbacks_p = gdns_res_counter.dnsResCounterFallbacks;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsResOptCounterReferals
 *
 * PURPOSE:
 *      This function gets the number of responses which were
 *      eived from servers redirecting query to another server.
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsResOptCounterReferals(int *dns_res_opt_counter_referals_p)
{
    UI32_T orig_priority;

    if(NULL == dns_res_opt_counter_referals_p)
        return DNS_ERROR;
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_res_opt_counter_referals_p = gdns_res_counter.dnsResOptCounterReferals;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsResOptCounterRetrans
 *
 * PURPOSE:
 *      This function gets the number requests retransmitted for all reasons
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsResOptCounterRetrans(int *dns_res_opt_counter_retrans_p)
{
    UI32_T orig_priority;

    if(NULL == dns_res_opt_counter_retrans_p)
        return DNS_ERROR;
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_res_opt_counter_retrans_p = gdns_res_counter.dnsResOptCounterRetrans;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

 /*
 * FUNCTION NAME : DNS_OM_GetDnsResOptCounterNoResponses
 *
 * PURPOSE:
 *      This function gets the number of queries that were retransmitted because of no response
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsResOptCounterNoResponses(UI32_T *dns_res_opt_counter_no_responses_p)
{
    UI32_T orig_priority;

    if(NULL == dns_res_opt_counter_no_responses_p)
        return DNS_ERROR;
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_res_opt_counter_no_responses_p = gdns_res_counter.dnsResOptCounterNoResponses;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}


/*
 * FUNCTION NAME : DNS_OM_GetDnsResOptCounterRootRetrans
 *
 * PURPOSE:
 *      This function gets the number of queries that were retransmitted that were to root servers.
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsResOptCounterRootRetrans(int *dns_res_opt_counter_root_retrans_p)
{
    UI32_T orig_priority;

    if(NULL == dns_res_opt_counter_root_retrans_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_res_opt_counter_root_retrans_p = gdns_res_counter.dnsResOptCounterRootRetrans;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsResOptCounterInternals
 *
 * PURPOSE:
 *      This function gets the number of requests internally generated by the resolver.
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsResOptCounterInternals(int *dns_res_opt_counter_internals)
{
    UI32_T orig_priority;

    if(NULL == dns_res_opt_counter_internals)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_res_opt_counter_internals = gdns_res_counter.dnsResOptCounterInternals;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsResOptCounterInternalTimeOuts
 *
 * PURPOSE:
 *      This function gets the number of requests internally generated which timed out.
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsResOptCounterInternalTimeOuts(int *dns_res_opt_counter_internal_time_outs_p)
{
    UI32_T orig_priority;

    if(NULL == dns_res_opt_counter_internal_time_outs_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_res_opt_counter_internal_time_outs_p = gdns_res_counter.dnsResOptCounterInternalTimeOuts;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_AddDomainName
 * PURPOSE:
 *      To define a default domain name that the ACCTON Switch software uses to
 *      complete unqualified host names (names without a dotted-decimal domain name)
 *
 *
 * INPUT:
 *      I8_T * -- a domain name to be added
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR:failure;
 *      DNS_OK:succsess;
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_AddDomainName(char *domain_name_p)
{
    UI32_T orig_priority;
    int len;

    if (NULL==domain_name_p)
        return DNS_ERROR;

    len=strlen(domain_name_p);
    if ((len>DNS_MAX_NAME_LENGTH)||(len==0))
    {
        printf("Domain name length is wrong!\n");
        return DNS_ERROR;   /*DNS_OUT_OF_RANGE*/
    }
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    strcpy((char *)gdns_config.DnsIpDomainName, domain_name_p);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_DeleteDomainName
 * PURPOSE:
 *      Delete the default domain name.
 *
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_OK:success;
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_DeleteDomainName(void)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsIpDomainName[0]='\0';
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_AddDomainNameToList
 * PURPOSE:
 *      If there is no domain list, the domain name that you specified with the ip
 *      domain-name global configuration command is used. If there is a domain list,
 *      the default domain name is not used. The ip domain-list command is similar
 *      to the ip domain-name command, except that with the ip domain-list command
 *      you can define a list of domains, each to be tried in turn.
 * INPUT:
 *      I8_T * -- a domain name to be added to the domain name lsit.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR:failure;
 *      DNS_OK:succsess;
 *
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_AddDomainNameToList(char *domain_name_p)
{
    UI32_T orig_priority;
    int     ret = DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    if (TRUE == DNS_OM_LocalAddDomainNameToListWithIndex(0, domain_name_p))
		    	{
        ret = DNS_OK;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return ret;
}

/* FUNCTION NAME : DNS_OM_DeleteDomainNameToList
 * PURPOSE:
 *      If there is no domain list, the domain name that you specified with the ip
 *      domain-name global configuration command is used. If there is a domain list,
 *      the default domain name is not used. The ip domain-list command is similar
 *      to the ip domain-name command, except that with the ip domain-list command
 *      you can define a list of domains, each to be tried in turn.
 * INPUT:
 *      I8_T * -- a domain name to be deleted to the domain name lsit
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR:failure;
 *      DNS_OK:succsess;
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_DeleteDomainNameFromList(char *domain_name_p)
{
    UI32_T orig_priority;
    DNS_IpDomain_T *prev_p,*dns_ip_domain_list_head_p;
    char str_lower_case[DNS_MAX_NAME_LENGTH+1],cur_lower_case[DNS_MAX_NAME_LENGTH+1];
    int len=strlen((char *)domain_name_p);

    if ((len>DNS_MAX_NAME_LENGTH)||(len==0))
    {
        printf("Domain name length is error!\n");
        return DNS_ERROR;
    }

    DNS_OM_Name2Lower(domain_name_p,str_lower_case);
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    dns_ip_domain_list_head_p=gdns_config.DnsIpDomainList_p;
    prev_p=dns_ip_domain_list_head_p;
    if(NULL==dns_ip_domain_list_head_p)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_ERROR;
    }
    else
    {
        DNS_OM_Name2Lower(dns_ip_domain_list_head_p->DnsIpDomainName,cur_lower_case);
        if (0==strcmp((char *)str_lower_case, (char *)cur_lower_case))
        {
            gdns_config.DnsIpDomainList_p=dns_ip_domain_list_head_p->next_p;
            DNS_OM_SBELT_TABLE_ENTRY_FREE(dns_ip_domain_list_head_p);
            dns_ip_domain_list_head_p = NULL;

            number_of_domain_name_list -= 1;/*isiah*/
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
            return DNS_OK;
        }
        while  (dns_ip_domain_list_head_p->next_p)
        {
            prev_p=dns_ip_domain_list_head_p;
            dns_ip_domain_list_head_p=dns_ip_domain_list_head_p->next_p;
            DNS_OM_Name2Lower(dns_ip_domain_list_head_p->DnsIpDomainName,cur_lower_case);
            if (0==strcmp((char *)str_lower_case, (char *)cur_lower_case))
            {
                prev_p->next_p=dns_ip_domain_list_head_p->next_p;
                DNS_OM_SBELT_TABLE_ENTRY_FREE(dns_ip_domain_list_head_p);
                dns_ip_domain_list_head_p = NULL;

                number_of_domain_name_list -= 1;/*isiah*/
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
                return DNS_OK;
            }
        }
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_ERROR;
}


/* FUNCTION NAME : DNS_OM_ShowDomainNameList
 * PURPOSE:
 *      This function shows the dns domain name list.
 *
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *       This function will be called by CLI module.
 */
BOOL_T DNS_OM_ShowDomainNameList(void)
{
    DNS_IpDomain_T*dns_ip_domain_list_head_p;
    dns_ip_domain_list_head_p=gdns_config.DnsIpDomainList_p;

    while  (dns_ip_domain_list_head_p!=NULL)
    {
        printf("\n%s",dns_ip_domain_list_head_p->DnsIpDomainName);
        dns_ip_domain_list_head_p=dns_ip_domain_list_head_p->next_p;
    }

    return TRUE;
}


/* FUNCTION NAME : DNS_OM_ShowDnsConfig
 * PURPOSE:
 *      This function shows the dns configuration.
 *
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *       This function will be called by CLI module.
 */
void DNS_OM_ShowDnsConfig(void)
{
    I8_T dns_config[1024];

    UI32_T          cur_ticks=SYSFUN_GetSysTick();

    sprintf((char *)dns_config,
           "\n\n\tDNS CONFIG:\t\t\t\n\nDnsOptimize\t\t\t\t:%d\nDnsStatus\t\t\t\t:%d"
           "\nDnsIpDomainName\t\t\t\t:%s\nDnsMaxLocalRequests\t\t\t:%d,"
           "\nDnsDebugStatus\t\t\t\t:%d,\nDnsTimeOut\t\t\t\t:%ld",
            gdns_config.DnsOptimize,
            gdns_config.DnsStatus,
            gdns_config.DnsIpDomainName,
            gdns_config.DnsMaxLocalRequests,
            gdns_config.DnsDebugStatus,
            (long)gdns_config.DnsTimeOut);
    printf("%s",dns_config);
    printf("\nDomain Name List");
    DNS_OM_ShowDomainNameList();

    sprintf((char *)dns_config,
           "\n\n\n\tDNS CONFIG RESOLVER:\n\nImplementation identification string\t:%s"
           "\ndnsResConfigService\t\t\t:%d,"
           "\ndnsResConfigUpTime\t\t\t:%lu,"
           "\ndnsResConfigResetTime\t\t\t:%lu,"
           "\ndnsResConfigReset\t\t\t:%u,"
           "\ndnsResConfigQueryOrder\t\t\t:%u",
           gdns_config.DnsResConfig.dnsResConfigImplementIdent,
           gdns_config.DnsResConfig.dnsResConfigService,
           (unsigned long)(cur_ticks-gdns_config.DnsResConfig.dnsResConfigUpTime)/DNS_SYS_CLK,
           (unsigned long)(cur_ticks-gdns_config.DnsResConfig.dnsResConfigResetTime)/DNS_SYS_CLK,
           gdns_config.DnsResConfig.dnsResConfigReset,
           gdns_config.DnsResConfig.dnsResConfigQueryOrder);
    printf("%s",dns_config);

    sprintf((char *)dns_config,
           "\n\n\n\tDNS CONFIG PROXY:\n\nImplementation identification string\t:%s,"
            "\ndnsServConfigUpTime\t\t\t:%lu,"
            "\ndnsServConfigResetTime\t\t\t:%lu,"
            "\ndnsServConfigReset\t\t\t:%u, "
            "\ndnsServConfigMaxRequests\t\t:%u,"
            "\ndnsServConfigCurrentNumberOfRequests\t:%u,"
            "\ndnsServEnabled\t\t\t\t:%u,"
            "\ndnsServStatus\t\t\t\t:%u",
           gdns_config.DnsProxyConfig.dnsServConfigImplementIdent,
           (unsigned long)(cur_ticks-gdns_config.DnsProxyConfig.dnsServConfigUpTime)/DNS_SYS_CLK,
           (unsigned long)(cur_ticks-gdns_config.DnsProxyConfig.dnsServConfigResetTime)/DNS_SYS_CLK,
           gdns_config.DnsProxyConfig.dnsServConfigReset,
           gdns_config.DnsProxyConfig.dnsServConfigMaxRequests,
           gdns_config.DnsProxyConfig.dnsServConfigCurrentNumberOfRequests,
           gdns_config.DnsProxyConfig.dnsServEnabled,
           gdns_config.DnsProxyConfig.dnsServStatus
           );
    printf("\nName Server List:");
    DNS_OM_ShowNameServerList();
    printf("%s",dns_config);

    sprintf((char *)dns_config,
            "\n\n\n\tDNS CONFIG CACHE:\n"
            "\nCache Max Entries\t\t\t:%u"
             "\nCache Status\t\t\t\t:%d,\nCache Max TTL\t\t\t\t:%lu\n"
/*             "\nCache Good Caches\t\t\t:%u,\nCache Bad Caches\t\t\t:%u\n" */,
           gdns_config.DnsResCache.cache_max_entries,
           gdns_config.DnsResCache.cache_status,
           (unsigned long)gdns_config.DnsResCache.cache_max_ttl
/*           ,gdns_config.DnsResCache.cache_good_caches,
           gdns_config.DnsResCache.cache_bad_caches*/
           );
    printf("%s", dns_config);
}

/* FUNCTION NAME : DNS_OM_SetDnsLocalMaxRequests
 * PURPOSE:
 *      This fcuntion sets time out value (seconds) for requests.
 *
 *
 *
 * INPUT:
 *      UI32_T* --  a pointer to a variable for storing timeout value
 *                  for requests. range:1..20 . default:5 seconds
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR:failure;
 *      DNS_OK:succsess;
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int  DNS_OM_SetDnsTimeOut(UI32_T* dns_time_out_p)
{
    UI32_T orig_priority;

    if (NULL==dns_time_out_p)
        return DNS_ERROR;

    if ((*dns_time_out_p<DNS_MIN_TIME_OUT)||(*dns_time_out_p>DNS_MAX_TIME_OUT))
        return DNS_ERROR;   /*DNS_OUT_OF_RANGE*/

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsTimeOut=*dns_time_out_p;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_SetDnsLocalMaxRequests
 * PURPOSE:
 *      This fcuntion gets the time out value (seconds) for requests.
 *
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T * -- a pointer to a variable for storing the returned value
 *                  range 1:20; default: 5 seconds
 * RETURN:
 *      DNS_ERROR:failure;
 *      DNS_OK:succsess;
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_GetDnsTimeOut(UI32_T *dns_time_out_p)
{
    UI32_T orig_priority;

    if (NULL==dns_time_out_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_time_out_p=gdns_config.DnsTimeOut;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsErrors
 * PURPOSE:
 *      Number of requests the server has processed which
 *      originated from friends and were answered with errors
 *      (RCODE values other than 0 and 3).  The definition of
 *      friends is a locally defined matter.
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T* -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_ERROR:failure;
 *      DNS_OK:succsess;
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_GetDnsServOptCounterFriendsErrors(UI32_T* dns_serv_opt_counter_friends_errors_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_opt_counter_friends_errors_p)
        return DNS_ERROR;
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_friends_errors_p = gdns_proxy_counter.dnsServOptCounterFriendsErrors;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsReferrals
 * PURPOSE:
 *      Number of requests which originated from friends that
 *      were referred to other servers.  The definition of
 *      friends is a locally defined matter.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_ERROR:failure;
 *      DNS_OK:succsess;
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_GetDnsServOptCounterFriendsReferrals(UI32_T *dns_serv_opt_counter_friends_referrals_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_opt_counter_friends_referrals_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_friends_referrals_p = gdns_proxy_counter.dnsServOptCounterFriendsReferrals;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsOtherErrors
 * PURPOSE:
 *      Number of requests which were aborted for other (local)
 *      server errors and which originated from `friends'.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T * -- a pointer to the variable to store the returned value
 *
 * RETURN:
 *      DNS_ERROR:failure;
 *      DNS_OK:succsess;
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_GetDnsServOptCounterFriendsOtherErrors(UI32_T *dns_serv_opt_counter_friends_other_errors_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_opt_counter_friends_other_errors_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_friends_other_errors_p = gdns_proxy_counter.dnsServOptCounterFriendsOtherErrors;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsReqUnparses
 * PURPOSE:
 *      Number of requests received which were unparseable and
 *      which originated from `friends'
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T * -- a pointer to the variable to store the returned value.
 *
 * RETURN:
 *      DNS_ERROR:failure;
 *      DNS_OK:succsess;
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_GetDnsServOptCounterFriendsReqUnparses(UI32_T *dns_serv_opt_counter_friends_req_unparses_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_opt_counter_friends_req_unparses_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_friends_req_unparses_p = gdns_proxy_counter.dnsServOptCounterFriendsReqUnparses;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsReqRefusals
 * PURPOSE:
 *      This function gets the number of DNS requests refused by the server which were
 *      received from `friends'.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T * -- a pointer to the variable to store the returned value
 *
 * RETURN:
 *      DNS_ERROR:failure;
 *      DNS_OK:succsess;
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_GetDnsServOptCounterFriendsReqRefusals(UI32_T *dns_serv_opt_counter_friends_req_refusals_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_opt_counter_friends_req_refusals_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_friends_req_refusals_p = gdns_proxy_counter.dnsServOptCounterFriendsReqRefusals;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsRelNames
 * PURPOSE:
 *      Number of requests received for names from friends that
 *      are only 1 label long (text form - no internal dots) the
 *      server has processed
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T * -- a pointer to a variable to store the result.
 *
 * RETURN:
 *      DNS_ERROR:failure;
 *      DNS_OK:succsess;
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_GetDnsServOptCounterFriendsRelNames(UI32_T *dns_serv_opt_counter_friends_rel_names_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_opt_counter_friends_rel_names_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_friends_rel_names_p = gdns_proxy_counter.dnsServOptCounterFriendsRelNames;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsNonAuthNoDatas
 * PURPOSE:
 *      Number of queries originating from friends which were
 *      non-authoritatively answered with no such data (empty
 *      answer)
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_ERROR:failure;
 *      DNS_OK:succsess;
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_GetDnsServOptCounterFriendsNonAuthNoDatas(UI32_T *dns_serv_opt_counter_friends_non_auth_no_datas_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_opt_counter_friends_non_auth_no_datas_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_friends_non_auth_no_datas_p = gdns_proxy_counter.dnsServOptCounterFriendsNonAuthNoDatas;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_GetDnsResCounterByRcodeEntry
 * PURPOSE:
 *      This function gets the number of responses for the specified index.
 *
 *
 *
 * INPUT:
 *      DNS_ResCounterByRcodeEntry_T * -- Given as a index for the entry.
 *                                        INDEX { dnsResCounterByRcodeCode }
 * OUTPUT:
 *      DNS_ResCounterByRcodeEntry_T * --a pointer to a DnsResCounterByRcodeEntry variable to store the returned entry
 *
 * RETURN:
 *      DNS_ERROR:failure;
 *      DNS_OK:succsess;
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_GetDnsResCounterByRcodeEntry(DNS_ResCounterByRcodeEntry_T *dns_res_counter_by_rcode_entry_p)
{
    UI32_T orig_priority;
    int i;

    if ((NULL == dns_res_counter_by_rcode_entry_p ) ||
        ((dns_res_counter_by_rcode_entry_p->dnsResCounterByRcodeCode) > 5))
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */

    if(NULL == dns_res_counter_by_rcode_entry_p )
    {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
	    return DNS_ERROR;
    }
	else
	{
		i = dns_res_counter_by_rcode_entry_p->dnsResCounterByRcodeCode;
		if(i>5)
		{
                    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
		    return DNS_ERROR;
		}

		dns_res_counter_by_rcode_entry_p->dnsResCounterByRcodeResponses = gdns_res_counter.dnsResCounterByRcodeTable[i].dnsResCounterByRcodeResponses;
	}
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_GetNextDnsResCounterByRcodeEntry
 * PURPOSE:
 *      This function gets the number of responses for the index next to the specified index.
 *
 *
 *
 * INPUT:
 *      DNS_ResCounterByRcodeEntry_T * -- Given as the index of the entry.
 *                                       INDEX { dnsResCounterByRcodeCode }
 * OUTPUT:
 *      DNS_ResCounterByRcodeEntry_T * -- a pointer to a DnsResCounterByRcodeEntry variable to store the returned entry
 *
 * RETURN:
 *      DNS_ERROR:failure;
 *      DNS_OK:succsess;
 *
 * NOTES:
 *      This function will be called by snmp module.
 *      The initial input for the index is 0.
 */
int DNS_OM_GetNextDnsResCounterByRcodeEntry(DNS_ResCounterByRcodeEntry_T *dns_res_counter_by_rcode_entry_p)
{
    UI32_T orig_priority;
    int i = dns_res_counter_by_rcode_entry_p->dnsResCounterByRcodeCode;

    if(NULL == dns_res_counter_by_rcode_entry_p || i > 5)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    dns_res_counter_by_rcode_entry_p->dnsResCounterByRcodeResponses = gdns_res_counter.dnsResCounterByRcodeTable[i].dnsResCounterByRcodeResponses;
    (dns_res_counter_by_rcode_entry_p->dnsResCounterByRcodeCode)++;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_GetDnsResCounterByOpcodeEntry
 * PURPOSE:
 *      This function gets dnsResCounterByOpcodeEntry according to the index.
 *
 *
 *
 * INPUT:
 *      DNS_ResCounterByOpcodeEntry_T * -- Given as the index of the entry.
 *
 * OUTPUT:
 *      DNS_ResCounterByOpcodeEntry_T * -- a pointer to a variable t store the returned entry
 *
 * RETURN:
 *      DNS_ERROR:failure;
 *      DNS_OK   :succsess;
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_GetDnsResCounterByOpcodeEntry(DNS_ResCounterByOpcodeEntry_T *dns_res_counter_by_opcode_entry_p)
{
    UI32_T orig_priority;

    int i = dns_res_counter_by_opcode_entry_p->dnsResCounterByOpcodeCode;

    if(NULL == dns_res_counter_by_opcode_entry_p || i>3)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    dns_res_counter_by_opcode_entry_p->dnsResCounterByOpcodeQueries = gdns_res_counter.dnsResCounterByOpcodeTable[i].dnsResCounterByOpcodeQueries;
    dns_res_counter_by_opcode_entry_p->dnsResCounterByOpcodeResponses = gdns_res_counter.dnsResCounterByOpcodeTable[i].dnsResCounterByOpcodeResponses;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_GetNextDnsResCounterByOpcodeEntry
 * PURPOSE:
 *      This function gets dnsResCounterByOpcodeEntry next to the specified index.
 *
 *
 *
 * INPUT:
 *      DNS_ResCounterByOpcodeEntry_T * -- Given to provide the index of the entry.
 *
 * OUTPUT:
 *      DNS_ResCounterByOpcodeEntry_T * -- a pointer to a variable to store the returned entry
 *
 * RETURN:
 *      DNS_ERROR:failure;
 *      DNS_OK   :succsess;
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_GetNextDnsResCounterByOpcodeEntry(DNS_ResCounterByOpcodeEntry_T *dns_res_counter_by_opcode_entry_p)
{
    UI32_T orig_priority;

    int i = dns_res_counter_by_opcode_entry_p->dnsResCounterByOpcodeCode;

    if((NULL == dns_res_counter_by_opcode_entry_p) || (i>=3))
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    dns_res_counter_by_opcode_entry_p->dnsResCounterByOpcodeQueries = gdns_res_counter.dnsResCounterByOpcodeTable[i].dnsResCounterByOpcodeQueries;
    dns_res_counter_by_opcode_entry_p->dnsResCounterByOpcodeResponses = gdns_res_counter.dnsResCounterByOpcodeTable[i].dnsResCounterByOpcodeResponses;
    (dns_res_counter_by_opcode_entry_p->dnsResCounterByOpcodeCode)++;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_GetDnsServCounterEntry
 * PURPOSE:
 *      This function gets the dnsServCounterEntry according the specified index.
 *
 *
 *
 * INPUT:
 *      DNS_ServCounterEntry_T * -- Given to provide the index of the entry.
 *
 * OUTPUT:
 *      DNS_ServCounterEntry_T * -- a pointer to a DNS_ServCounterEntry_T variable to store the returned entry
 *
 * RETURN:
 *      DNS_ERROR:failure;
 *      DNS_OK   :succsess;
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_GetDnsServCounterEntry(DNS_ServCounterEntry_T *dns_serv_counter_entry_t_p)
{
   int i;
   UI32_T orig_priority;

   if (NULL==dns_serv_counter_entry_t_p)
       return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
   for (i=0;i<DNS_MAX_OP_CODE_NUMBER;i++)
        if ((gdns_proxy_counter.dnsServCounterTable[i].dnsServCounterOpCode==dns_serv_counter_entry_t_p->dnsServCounterOpCode)&&
            (gdns_proxy_counter.dnsServCounterTable[i].dnsServCounterQClass==dns_serv_counter_entry_t_p->dnsServCounterQClass)&&
            (gdns_proxy_counter.dnsServCounterTable[i].dnsServCounterQType==dns_serv_counter_entry_t_p->dnsServCounterQType)&&
            (gdns_proxy_counter.dnsServCounterTable[i].dnsServCounterTransport==dns_serv_counter_entry_t_p->dnsServCounterTransport))
        {
            dns_serv_counter_entry_t_p->dnsServCounterRequests=gdns_proxy_counter.dnsServCounterTable[i].dnsServCounterRequests;
            dns_serv_counter_entry_t_p->dnsServCounterResponses=gdns_proxy_counter.dnsServCounterTable[i].dnsServCounterResponses;
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
            return DNS_OK;
        }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_ERROR;
}



/* FUNCTION NAME : DNS_OM_GetNextDnsServCounterEntry
 * PURPOSE:
 *      This function gets the dnsServCounterEntry next to the specified index.
 *
 *
 *
 * INPUT:
 *      DNS_ServCounterEntry_T * -- Given to provide the index of the entry.
 *
 * OUTPUT:
 *      DNS_ServCounterEntry_T * -- a pointer to a DNS_ServCounterEntry_T variable to store the returned entry
 *
 * RETURN:
 *      DNS_ERROR:failure;
 *      DNS_OK   :succsess;
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_GetNextDnsServCounterEntry(DNS_ServCounterEntry_T *dns_serv_counter_entry_t_p)
{
    int i;
    UI32_T orig_priority;

    if (NULL==dns_serv_counter_entry_t_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    if ((0==dns_serv_counter_entry_t_p->dnsServCounterOpCode)&&
        (0==dns_serv_counter_entry_t_p->dnsServCounterQClass)&&
        (0==dns_serv_counter_entry_t_p->dnsServCounterQType)&&
        (0==dns_serv_counter_entry_t_p->dnsServCounterTransport))
    {
        dns_serv_counter_entry_t_p->dnsServCounterOpCode=gdns_proxy_counter.dnsServCounterTable[0].dnsServCounterOpCode;
        dns_serv_counter_entry_t_p->dnsServCounterQClass=gdns_proxy_counter.dnsServCounterTable[0].dnsServCounterQClass;
        dns_serv_counter_entry_t_p->dnsServCounterQType= gdns_proxy_counter.dnsServCounterTable[0].dnsServCounterQType;
        dns_serv_counter_entry_t_p->dnsServCounterTransport=gdns_proxy_counter.dnsServCounterTable[0].dnsServCounterTransport;
        dns_serv_counter_entry_t_p->dnsServCounterRequests=gdns_proxy_counter.dnsServCounterTable[0].dnsServCounterRequests;
        dns_serv_counter_entry_t_p->dnsServCounterResponses=gdns_proxy_counter.dnsServCounterTable[0].dnsServCounterResponses;
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_OK;
    }
    else
    {
        for (i=0;i<(DNS_MAX_OP_CODE_NUMBER-1);i++)
            if ((gdns_proxy_counter.dnsServCounterTable[i].dnsServCounterOpCode==dns_serv_counter_entry_t_p->dnsServCounterOpCode)&&
                (gdns_proxy_counter.dnsServCounterTable[i].dnsServCounterQClass==dns_serv_counter_entry_t_p->dnsServCounterQClass)&&
                (gdns_proxy_counter.dnsServCounterTable[i].dnsServCounterQType==dns_serv_counter_entry_t_p->dnsServCounterQType)&&
                (gdns_proxy_counter.dnsServCounterTable[i].dnsServCounterTransport==dns_serv_counter_entry_t_p->dnsServCounterTransport))
            break;

        if (i<(DNS_MAX_OP_CODE_NUMBER-1))
        {
            dns_serv_counter_entry_t_p->dnsServCounterOpCode=gdns_proxy_counter.dnsServCounterTable[i+1].dnsServCounterOpCode;
            dns_serv_counter_entry_t_p->dnsServCounterQClass=gdns_proxy_counter.dnsServCounterTable[i+1].dnsServCounterQClass;
            dns_serv_counter_entry_t_p->dnsServCounterQType= gdns_proxy_counter.dnsServCounterTable[i+1].dnsServCounterQType;
            dns_serv_counter_entry_t_p->dnsServCounterTransport=gdns_proxy_counter.dnsServCounterTable[i+1].dnsServCounterTransport;
            dns_serv_counter_entry_t_p->dnsServCounterRequests=gdns_proxy_counter.dnsServCounterTable[i+1].dnsServCounterRequests;
            dns_serv_counter_entry_t_p->dnsServCounterResponses=gdns_proxy_counter.dnsServCounterTable[i+1].dnsServCounterResponses;
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
            return DNS_OK;
        }
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_ERROR;
    }
}

/* FUNCTION NAME : DNS_OM_GetDnsServOptCounterSelfAuthAns
 * PURPOSE:
 *      This function gets the dnsServCounterEntry next to the specified index.
 *
 *
 *
 * INPUT:
 *      DNS_ServCounterEntry_T * -- Given to provide the index of the entry.
 *
 * OUTPUT:
 *      DNS_ServCounterEntry_T * -- a pointer to a DNS_ServCounterEntry_T variable to store the returned entry
 *
 * RETURN:
 *      DNS_ERROR:failure;
 *      DNS_OK   :succsess;
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_GetDnsServOptCounterSelfAuthAns(int *dns_serv_opt_counter_self_auth_ans_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_opt_counter_self_auth_ans_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_self_auth_ans_p = gdns_proxy_counter.dnsServOptCounterSelfAuthAns;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterSelfAuthNoNames
 *
 * PURPOSE:
 *      This function gets the number of requests the server has processed which
        originated from a resolver on the same host for which
        there has been an authoritative no such name answer given.
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServOptCounterSelfAuthNoNames(int *dns_serv_opt_counter_self_auth_no_names_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_opt_counter_self_auth_no_names_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_self_auth_no_names_p = gdns_proxy_counter.dnsServOptCounterSelfAuthNoNames;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterSelfAuthNoDataResps
 *
 * PURPOSE:
 *      This function gets the number of requests the server has processed which
 *       originated from a resolver on the same host for which
 *       there has been an authoritative no such data answer (empty answer) made.
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServOptCounterSelfAuthNoDataResps(int *dns_serv_opt_counter_self_auth_no_data_resps)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_opt_counter_self_auth_no_data_resps)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_self_auth_no_data_resps = gdns_proxy_counter.dnsServOptCounterSelfAuthNoDataResps;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterSelfNonAuthDatas
 *
 * PURPOSE:
 *       Number of requests the server has processed which
          originated from a resolver on the same host for which a
          non-authoritative answer (cached data) was made
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServOptCounterSelfNonAuthDatas(int *dns_serv_opt_counter_self_non_auth_datas_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_opt_counter_self_non_auth_datas_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_self_non_auth_datas_p = gdns_proxy_counter.dnsServOptCounterSelfNonAuthDatas;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterSelfNonAuthNoDatas
 *
 * PURPOSE:
 *      Number of requests the server has processed which
 *      originated from a resolver on the same host for which a
 *      non-authoritative, no such data' response was made
 *      (empty answer).
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServOptCounterSelfNonAuthNoDatas(int *dns_serv_opt_counter_self_non_auth_no_datas_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_opt_counter_self_non_auth_no_datas_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_self_non_auth_no_datas_p = gdns_proxy_counter.dnsServOptCounterSelfNonAuthNoDatas;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterSelfReferrals
 *
 * PURPOSE:
 *      Number of queries the server has processed which
 *       originated from a resolver on the same host and were
 *       referred to other servers
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServOptCounterSelfReferrals( int *dns_serv_opt_counter_self_referrals)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_opt_counter_self_referrals)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_self_referrals = gdns_proxy_counter.dnsServOptCounterSelfReferrals;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterSelfErrors
 *
 * PURPOSE:
 *      Number of requests the server has processed which
 *       originated from a resolver on the same host which have
 *      been answered with errors (RCODEs other than 0 and 3).
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServOptCounterSelfErrors(int *dns_serv_opt_counter_self_errors_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_opt_counter_self_errors_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_self_errors_p = gdns_proxy_counter.dnsServOptCounterSelfErrors;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterSelfRelNames
 *
 * PURPOSE:
 *      Number of requests received for names that are only 1
 *      label long (text form - no internal dots) the server has
 *       processed which originated from a resolver on the same
 *       host.
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServOptCounterSelfRelNames(int *dns_serv_opt_counter_self_rel_names_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_opt_counter_self_rel_names_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_self_rel_names_p = gdns_proxy_counter.dnsServOptCounterSelfRelNames;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterSelfReqRefusals
 *
 * PURPOSE:
 *      Number of DNS requests refused by the server which
 *      originated from a resolver on the same host
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServOptCounterSelfReqRefusals(int *dns_serv_opt_counter_self_req_refusals_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_opt_counter_self_req_refusals_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_self_req_refusals_p = gdns_proxy_counter.dnsServOptCounterSelfReqRefusals;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterSelfReqUnparses
 *
 * PURPOSE:
 *      Number of requests received which were unparseable and
 *      which originated from a resolver on the same host.
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServOptCounterSelfReqUnparses(int *dns_serv_opt_counter_self_req_unparses_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_opt_counter_self_req_unparses_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_self_req_unparses_p = gdns_proxy_counter.dnsServOptCounterSelfReqUnparses;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterSelfOtherErrors
 *
 * PURPOSE:
 *      Number of requests which were aborted for other (local)
 *      server errors and which originated on the same host.
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * --   a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServOptCounterSelfOtherErrors(int *dns_serv_opt_counter_self_other_errors_p)
{
    UI32_T orig_priority;
    if(NULL == dns_serv_opt_counter_self_other_errors_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_self_other_errors_p = gdns_proxy_counter.dnsServOptCounterSelfOtherErrors;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}


/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsAuthAns
 *
 * PURPOSE:
 *      Number of queries originating from friends which were
 *      authoritatively answered.  The definition of friends is
 *      a locally defined matter
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      int * --   a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServOptCounterFriendsAuthAns(int *dns_serv_opt_counter_friends_auth_ans_p)
{
    UI32_T orig_priority;
    if(NULL == dns_serv_opt_counter_friends_auth_ans_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_friends_auth_ans_p = gdns_proxy_counter.dnsServOptCounterFriendsAuthAns;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}

/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsAuthNoNames
 *
 * PURPOSE:
 *      Number of queries originating from friends, for which
 *      authoritative `no such name' responses were made.  The
 *      definition of friends is a locally defined matter.
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServOptCounterFriendsAuthNoNames(int *dns_serv_opt_counter_friends_auth_no_names_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_opt_counter_friends_auth_no_names_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_friends_auth_no_names_p = gdns_proxy_counter.dnsServOptCounterFriendsAuthNoNames;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}


/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsAuthNoDataResps
 *
 * PURPOSE:
 *      Number of queries originating from friends for which
 *      authoritative no such data (empty answer) responses were
 *      made.  The definition of friends is a locally defined
 *      matter
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      UI32_T * --  a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_OK/DNS_ERROR
 *
 * NOTES:
 *      none
 */
int DNS_OM_GetDnsServOptCounterFriendsAuthNoDataResps(UI32_T *dns_serv_opt_counter_friends_auth_no_data_resps_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_opt_counter_friends_auth_no_data_resps_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_friends_auth_no_data_resps_p = gdns_proxy_counter.dnsServOptCounterFriendsAuthNoDataResps;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsNonAuthDatas
 * PURPOSE:
 *      Number of queries originating from friends which were
 *      non-authoritatively answered with no such data (empty
 *      answer)
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T * -- a pointer to a variable to store the result
 *
 * RETURN:
 *      DNS_ERROR:failure;
 *      DNS_OK:succsess;
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_GetDnsServOptCounterFriendsNonAuthDatas(UI32_T *dns_serv_opt_counter_friends_non_auth_datas_p)
{
    UI32_T orig_priority;

    if(NULL == dns_serv_opt_counter_friends_non_auth_datas_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *dns_serv_opt_counter_friends_non_auth_datas_p = gdns_proxy_counter.dnsServOptCounterFriendsNonAuthDatas;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_ResCounterByRcodeInc
 *
 * PURPOSE:
 *      resolver counter add 1 (index by rcode)
 *
 * INPUT:
 *      int -- index
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
void DNS_OM_ResCounterByRcodeInc(int rcode)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */

    if(rcode > 6)
        rcode = 6;

    gdns_res_counter.dnsResCounterByRcodeTable[rcode].dnsResCounterByRcodeResponses++;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_ResCounterByOpcodeInc
 *
 * PURPOSE:
 *      resolver counter add 1 index by opcode
 *
 * INPUT:
 *      int -- identify whether a request or response
 *      int -- index
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
void DNS_OM_ResCounterByOpcodeInc(int qr,int opcode)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    if(opcode > 2)
        opcode = 2;

    if(DNS_QR_RESP == qr)
        gdns_res_counter.dnsResCounterByOpcodeTable[opcode].dnsResCounterByOpcodeResponses++;
    else
        gdns_res_counter.dnsResCounterByOpcodeTable[opcode].dnsResCounterByOpcodeQueries++;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_ResCounterInc
 *
 * PURPOSE:
 *      Resolver counter add 1 (index by leaf)
 *
 * INPUT:
 *      int -- leaf.
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
void DNS_OM_ResCounterInc(int leaf)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    switch(leaf)
    {
    case LEAF_dnsResCounterNonAuthDataResps:
        gdns_res_counter.dnsResCounterNonAuthDataResps++;
        break;

    case LEAF_dnsResCounterNonAuthNoDataResps:
        gdns_res_counter.dnsResCounterNonAuthNoDataResps++;
        break;

    case LEAF_dnsResCounterMartians:
        gdns_res_counter.dnsResCounterMartians++;
        break;

    case LEAF_dnsResCounterRecdResponses:
        gdns_res_counter.dnsResCounterRecdResponses++;
        break;

    case LEAF_dnsResCounterUnparseResps:
        gdns_res_counter.dnsResCounterUnparseResps++;
        break;

    case LEAF_dnsResCounterFallbacks:
        gdns_res_counter.dnsResCounterFallbacks++;
        break;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_ResOptCounterInc
 *
 * PURPOSE:
 *      Resolver's opt counter add 1 (index by leaf)
 *
 * INPUT:
 *      int -- leaf.
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
void DNS_OM_ResOptCounterInc(int leaf)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    switch(leaf)
    {
    case LEAF_dnsResOptCounterReferals:
        gdns_res_counter.dnsResOptCounterReferals++;
        break;

    case LEAF_dnsResOptCounterRetrans:
        gdns_res_counter.dnsResOptCounterRetrans++;
        break;

    case LEAF_dnsResOptCounterNoResponses:
        gdns_res_counter.dnsResOptCounterRetrans++;
        break;

    case LEAF_dnsResOptCounterRootRetrans:
        gdns_res_counter.dnsResOptCounterRootRetrans++;
        break;

    case LEAF_dnsResOptCounterInternals:
        gdns_res_counter.dnsResOptCounterInternals++;
        break;

    case LEAF_dnsResOptCounterInternalTimeOuts:
        gdns_res_counter.dnsResOptCounterInternalTimeOuts++;
        break;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_ServOptCounterInc
 *
 * PURPOSE:
 *      Server counter add 1 (index by leaf)
 *
 * INPUT:
 *      int -- .
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
void DNS_OM_ServOptCounterInc(int leaf)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    switch(leaf)
    {
    case LEAF_dnsServOptCounterSelfAuthAns:
        gdns_proxy_counter.dnsServOptCounterSelfAuthAns++;
        break;

    case LEAF_dnsServOptCounterSelfAuthNoNames:
        gdns_proxy_counter.dnsServOptCounterSelfAuthNoNames++;
        break;

    case LEAF_dnsServOptCounterSelfAuthNoDataResps:
        gdns_proxy_counter.dnsServOptCounterSelfAuthNoDataResps++;
        break;

    case LEAF_dnsServOptCounterSelfNonAuthDatas:
        gdns_proxy_counter.dnsServOptCounterSelfNonAuthDatas++;
        break;

    case LEAF_dnsServOptCounterSelfNonAuthNoDatas:
        gdns_proxy_counter.dnsServOptCounterSelfNonAuthNoDatas++;
        break;

    case LEAF_dnsServOptCounterSelfReferrals:
        gdns_proxy_counter.dnsServOptCounterSelfReferrals++;
        break;

    case LEAF_dnsServOptCounterSelfErrors:
        gdns_proxy_counter.dnsServOptCounterSelfErrors++;
        break;

    case LEAF_dnsServOptCounterSelfRelNames:
        gdns_proxy_counter.dnsServOptCounterSelfRelNames++;
        break;

    case LEAF_dnsServOptCounterSelfReqRefusals:
        gdns_proxy_counter.dnsServOptCounterSelfReqRefusals++;
        break;

    case LEAF_dnsServOptCounterSelfReqUnparses:
        gdns_proxy_counter.dnsServOptCounterSelfReqUnparses++;
        break;

    case LEAF_dnsServOptCounterSelfOtherErrors:
        gdns_proxy_counter.dnsServOptCounterSelfOtherErrors++;
        break;

    /* friend part */
    case LEAF_dnsServOptCounterFriendsAuthAns:
        gdns_proxy_counter.dnsServOptCounterFriendsAuthAns++;
        break;

    case LEAF_dnsServOptCounterFriendsAuthNoNames:
        gdns_proxy_counter.dnsServOptCounterFriendsAuthNoNames++;
        break;

    case LEAF_dnsServOptCounterFriendsAuthNoDataResps:
        gdns_proxy_counter.dnsServOptCounterFriendsAuthNoDataResps++;
        break;

    case LEAF_dnsServOptCounterFriendsNonAuthDatas:
        gdns_proxy_counter.dnsServOptCounterFriendsNonAuthDatas++;
        break;

    case LEAF_dnsServOptCounterFriendsNonAuthNoDatas:
        gdns_proxy_counter.dnsServOptCounterFriendsNonAuthNoDatas++;
        break;

    case LEAF_dnsServOptCounterFriendsReferrals:
        gdns_proxy_counter.dnsServOptCounterFriendsReferrals++;
        break;

    case LEAF_dnsServOptCounterFriendsErrors:
        gdns_proxy_counter.dnsServOptCounterFriendsErrors++;
        break;

    case LEAF_dnsServOptCounterFriendsRelNames:
        gdns_proxy_counter.dnsServOptCounterFriendsRelNames++;
        break;

    case LEAF_dnsServOptCounterFriendsReqRefusals:
        gdns_proxy_counter.dnsServOptCounterFriendsReqRefusals++;
        break;

    case LEAF_dnsServOptCounterFriendsReqUnparses:
        gdns_proxy_counter.dnsServOptCounterFriendsReqUnparses++;
        break;

    case LEAF_dnsServOptCounterFriendsOtherErrors:
        gdns_proxy_counter.dnsServOptCounterFriendsOtherErrors++;
        break;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_ServCounterInc
 *
 * PURPOSE:
 *      Server's counter add 1(index with leaf)
 *
 * INPUT:
 *      int.
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
void DNS_OM_ServCounterInc(int leaf)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    switch(leaf)
    {
    case LEAF_dnsServCounterAuthAns:
        gdns_proxy_counter.dnsServCounterAuthAns++;
        break;

    case LEAF_dnsServCounterAuthNoNames:
        gdns_proxy_counter.dnsServCounterAuthNoNames++;
        break;

    case LEAF_dnsServCounterAuthNoDataResps:
        gdns_proxy_counter.dnsServCounterAuthNoDataResps++;
        break;

    case LEAF_dnsServCounterNonAuthDatas:
        gdns_proxy_counter.dnsServCounterNonAuthDatas++;
        break;

    case LEAF_dnsServCounterNonAuthNoDatas:
        gdns_proxy_counter.dnsServCounterNonAuthNoDatas++;
        break;

    case LEAF_dnsServCounterReferrals:
        gdns_proxy_counter.dnsServCounterReferrals++;
        break;

    case LEAF_dnsServCounterErrors:
        gdns_proxy_counter.dnsServCounterErrors++;
        break;

    case LEAF_dnsServCounterRelNames:
        gdns_proxy_counter.dnsServCounterRelNames++;
        break;

    case LEAF_dnsServCounterReqRefusals:
        gdns_proxy_counter.dnsServCounterReqRefusals++;
        break;

    case LEAF_dnsServCounterReqUnparses:
        gdns_proxy_counter.dnsServCounterReqUnparses++;
        break;

    case LEAF_dnsServCounterOtherErrors:
        gdns_proxy_counter.dnsServCounterOtherErrors++;
        break;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_GetDnsDebugStatus
 *
 * PURPOSE:
 *      Get whether server debug status is open.
 *
 * INPUT:
 *      none.
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
void DNS_OM_GetDnsDebugStatus(int *status)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *status = gdns_config.DnsDebugStatus;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_ShowCounter
 *
 * PURPOSE:
 *      show dns counter.
 *
 * INPUT:
 *      none.
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
void DNS_OM_ShowCounter(void)
{
    int i;

    printf("\n\t\tDNS_SHOW COUNTER\t\t\n");
    printf("\n-------------------------------------------------------------\n");
    printf("DNS internal request timeout\t:%ld", (long)gdns_res_counter.dnsResOptCounterInternalTimeOuts);
    printf("\nDNS total internal request\t:%ld", (long)gdns_res_counter.dnsResOptCounterInternals);
    printf("\nDNS resolver re-transmit\t:%ld", (long)gdns_res_counter.dnsResOptCounterRootRetrans);
    printf("\nDNS sent request no answer\t:%ld", (long)gdns_res_counter.dnsResOptCounterNoResponses);
    printf("\nDNS request to root server\t:%ld", (long)gdns_res_counter.dnsResOptCounterRootRetrans);
    printf("\nDNS received referals\t\t:%ld", (long)gdns_res_counter.dnsResOptCounterReferals);
    printf("\nDNS fallback\t\t\t:%ld", (long)gdns_res_counter.dnsResCounterFallbacks);
    printf("\nDNS received unparsible answer\t:%ld", (long)gdns_res_counter.dnsResCounterUnparseResps);
    printf("\nDNS received response\t\t:%ld", (long)gdns_res_counter.dnsResCounterRecdResponses);
    printf("\nDNS martians\t\t\t:%ld", (long)gdns_res_counter.dnsResCounterMartians);
    printf("\nDNS nonauth no data\t\t:%ld", (long)gdns_res_counter.dnsResCounterNonAuthNoDataResps);
    printf("\nDNS nonauth data\t\t:%ld", (long)gdns_res_counter.dnsResCounterNonAuthDataResps);

    printf("\n\nDNS statistics by rcode:");
    printf("\nOK\tFORMAT\tSRVFAIL\tNAMEERR\tNOSUPP\tREFUSE\tUNKNOWN\n");
    for(i=0;i<DNS_MAX_R_CODE_NUMBER;i++)
        printf("%-4d\t",gdns_res_counter.dnsResCounterByRcodeTable[i].dnsResCounterByRcodeResponses);

    printf("\n\nDNS statistics by opcode:");
    printf("\nQUERIES\t:QUERY\tIQUERY\tOTHER\t\n");
    for(i=0;i<DNS_MAX_OP_CODE_NUMBER;i++)
    {
        printf("\t%-4d",gdns_res_counter.dnsResCounterByOpcodeTable[i].dnsResCounterByOpcodeQueries);
    }
    printf("\nRESPONSE:QUERY\tIQUERY\tOTHER\t\n");
    for(i=0;i<DNS_MAX_OP_CODE_NUMBER;i++)
    {
        printf("\t%-4d",gdns_res_counter.dnsResCounterByOpcodeTable[i].dnsResCounterByOpcodeResponses);
    }

    printf("\n");

    printf("\nDNS self nonauth no data\t:%ld", (long)gdns_proxy_counter.dnsServOptCounterSelfNonAuthNoDatas);
    printf("\nDNS self 1 label request\t:%ld", (long)gdns_proxy_counter.dnsServOptCounterSelfRelNames);
    printf("\nDNS local request refused\t:%ld", (long)gdns_proxy_counter.dnsServOptCounterSelfReqRefusals);
    printf("\nDNS local request unparse\t:%ld", (long)gdns_proxy_counter.dnsServOptCounterSelfReqUnparses);
    printf("\nDNS local request other error\t:%ld", (long)gdns_proxy_counter.dnsServOptCounterSelfOtherErrors);
    /* other error */
    printf("\n");

    printf("\nDNS proxy request unparse\t:%ld", (long)gdns_proxy_counter.dnsServOptCounterFriendsReqUnparses);
    printf("\nDNS proxy request refused\t:%ld", (long)gdns_proxy_counter.dnsServOptCounterFriendsReqRefusals);
    printf("\nDNS proxy 1 label request\t:%ld", (long)gdns_proxy_counter.dnsServOptCounterFriendsRelNames);
    printf("\nDNS proxy other error\t\t:%ld", (long)gdns_proxy_counter.dnsServOptCounterFriendsOtherErrors);
    printf("\n");
}

/* FUNCTION NAME : DNS_OM_GetDnsStatus
 *
 * PURPOSE:
 *      Get dns status
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      *status
 *
 * RETURN:
 *      TRUE
 *
 * NOTES:
 *      none.
 */
void DNS_OM_GetDnsStatus(int *status)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *status = gdns_config.DnsStatus;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_SetDnsStatus
 *
 * PURPOSE:
 *      set dns status
 *
 * INPUT:
 *      int -- status.
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
void DNS_OM_SetDnsStatus(int status)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsStatus = status;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_GetDnsIpDomain
 *
 * PURPOSE:
 *      Get dns ip domain name.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      char* -- ipdomain name .
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *      none.
 */
int DNS_OM_GetDnsIpDomain(char* ipdomain)
{
    UI32_T orig_priority;

    if(NULL == ipdomain || NULL == gdns_config.DnsIpDomainName)
        return DNS_ERROR;
    /*isiah*/
    if( strcmp((char *)gdns_config.DnsIpDomainName,"") == 0 )
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    strcpy(ipdomain, (char *)gdns_config.DnsIpDomainName);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_DnsUptimeInit
 *
 * PURPOSE:
 *      Initiate dns uptime
 *
 * INPUT:
 *      none.
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
void DNS_OM_DnsUptimeInit(void)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsResConfig.dnsResConfigUpTime = SYSFUN_GetSysTick();
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_DnsResetTimeInit
 *
 * PURPOSE:
 *      initiate dns reset time
 *
 * INPUT:
 *      none.
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
void DNS_OM_DnsResetTimeInit(void)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsResConfig.dnsResConfigResetTime = SYSFUN_GetSysTick();
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_GetDnsSbelt
 *
 * PURPOSE:
 *      Get dns sbelt.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      DNS_ResConfigSbelt_T* -- dns sbelt
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *      none.
 */
DNS_ResConfigSbelt_T* DNS_OM_GetDnsSbelt(void)
{
    return gdns_config.DnsResConfig.dns_res_config_sbelt_table_p;
}

/* FUNCTION NAME : DNS_OM_ResCounterInit
 *
 * PURPOSE:
 *      initiate dns resolver counter
 *
 * INPUT:
 *      none.
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
void DNS_OM_ResCounterInit(void)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);
    memset(&gdns_res_counter,0,sizeof(struct DNS_ResCounter_S));
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_ServCounterInit
 *
 * PURPOSE:
 *      initiate dns server counter.
 *
 * INPUT:
 *      none.
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
void DNS_OM_ServCounterInit()
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);
    memset(&gdns_proxy_counter,0,sizeof(struct DNS_ProxyCounter_S));
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_GetServStatus
 *
 * PURPOSE:
 *      get dns server status
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      int -- dns server status
 *
 * NOTES:
 *      none.
 */
int DNS_OM_GetServStatus(void)
{
    return gdns_config.DnsProxyConfig.dnsServStatus;
}

/* FUNCTION NAME : DNS_OM_SetServStatus
 *
 * PURPOSE:
 *      ser dns server status
 *
 * INPUT:
 *      int -- dns server status
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
void DNS_OM_SetServStatus(int status)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsProxyConfig.dnsServStatus = status;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_GetServCurrentRequestNumber
 *
 * PURPOSE:
 *      get server current request number.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      int -- current server request number
 *
 * NOTES:
 *      none.
 */
int DNS_OM_GetServCurrentRequestNumber(void)
{
    return gdns_config.DnsProxyConfig.dnsServConfigCurrentNumberOfRequests;
}

/* FUNCTION NAME : DNS_OM_ServCurrentRequestNumberInc
 *
 * PURPOSE:
 *      current server request number add 1
 *
 * INPUT:
 *      none.
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
void DNS_OM_ServCurrentRequestNumberInc(void)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsProxyConfig.dnsServConfigCurrentNumberOfRequests++;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_ServCurrentRequestNumberDec
 *
 * PURPOSE:
 *      Dec current request number
 *
 * INPUT:
 *      none.
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
void DNS_OM_ServCurrentRequestNumberDec()
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsProxyConfig.dnsServConfigCurrentNumberOfRequests--;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_SetServResetStatus
 *
 * PURPOSE:
 *      Ser server reset status.
 *
 * INPUT:
 *      none.
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
void DNS_OM_SetServResetStatus(int status)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsProxyConfig.dnsServConfigReset = status;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_SetServServiceEnable
 *
 * PURPOSE:
 *      set dns server service whether is enable
 *
 * INPUT:
 *      int -- 0 ,disableb.
 *            1 ,enabled.
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *      none.
 */
void DNS_OM_SetServServiceEnable(int enable)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsProxyConfig.dnsServEnabled = enable;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_GetServServiceEnable
 *
 * PURPOSE:
 *      Get whether server is enable
 *
 * INPUT:
 *      none.
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
int DNS_OM_GetServServiceEnable(void)
{
    return gdns_config.DnsProxyConfig.dnsServEnabled;
}

/* FUNCTION NAME : DNS_OM_ServUpTimeInit
 *
 * PURPOSE:
 *      initiate server uptime
 *
 * INPUT:
 *      none.
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
void DNS_OM_ServUpTimeInit(void)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsProxyConfig.dnsServConfigUpTime=SYSFUN_GetSysTick();
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_ServResetTimeInit
*
 * PURPOSE:
 *      initiate server reset time
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *      none
 */
void DNS_OM_ServResetTimeInit(void)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsProxyConfig.dnsServConfigResetTime=SYSFUN_GetSysTick();
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}



/* FUNCTION NAME : DNS_OM_SetResolverTaskId
 * PURPOSE:
 *      Set resolver task id.
 *
 * INPUT:
 *      UI32_T  tid --  Resolver task id.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *          TRUE
 *
 * NOTES:
 *      None.
 */
BOOL_T DNS_OM_SetResolverTaskId(UI32_T tid)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */
    UI32_T orig_priority;
    /* BODY */

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    dns_om_resolver_task_id = tid;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return TRUE;
}



/* FUNCTION NAME : DNS_OM_GetResolverTaskId
 * PURPOSE:
 *      Get resolver task id.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T *tid --  Resolver task id.
 *
 * RETURN:
 *          TRUE
 *
 * NOTES:
 *      None.
 */
BOOL_T DNS_OM_GetResolverTaskId(UI32_T *tid)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */
    UI32_T orig_priority;
    /* BODY */
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    *tid = dns_om_resolver_task_id;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return TRUE;
}



/* FUNCTION NAME : DNS_OM_GetNextDomainNameList
 * PURPOSE:
 *      This function get next the dns domain name from list.
 *
 *
 *
 * INPUT:
 *      I8_T *dns_ip_domain_name    --  current dommain name of list.
 *
 * OUTPUT:
 *      I8_T *dns_ip_domain_name    --  next dommain name of list.
 *
 * RETURN:
 *      TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *       the initial name is empty string.
 */
BOOL_T DNS_OM_GetNextDomainNameList(char *dns_ip_domain_name)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */
    DNS_IpDomain_T *dns_ip_domain_list_head_p;
    UI32_T orig_priority;

    /* BODY */
    if ( dns_ip_domain_name == NULL )
    {
        return FALSE;
    }
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */

    dns_ip_domain_list_head_p = gdns_config.DnsIpDomainList_p;
    if ( dns_ip_domain_list_head_p == NULL )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return FALSE;
    }

    if ( strcmp((char *)dns_ip_domain_name, "") == 0 )
    {
        strcpy((char *)dns_ip_domain_name, (char *)dns_ip_domain_list_head_p->DnsIpDomainName);
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return TRUE;
    }
    else
    {
        while ( dns_ip_domain_list_head_p != NULL )
        {
            if ( strcmp((char *)dns_ip_domain_name, (char *)dns_ip_domain_list_head_p->DnsIpDomainName) == 0 )
            {
                dns_ip_domain_list_head_p = dns_ip_domain_list_head_p->next_p;
                if ( dns_ip_domain_list_head_p != NULL )
                {
                    strcpy((char *)dns_ip_domain_name, (char *)dns_ip_domain_list_head_p->DnsIpDomainName);
                    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
                    return TRUE;
                }
                else
                {
                    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
                    return FALSE;
                }
            }
            else
            {
                dns_ip_domain_list_head_p = dns_ip_domain_list_head_p->next_p;
            }
        }
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return FALSE;
    }
}

/* FUNCTION NAME : DNS_OM_GetDomainNameList
 * PURPOSE:
 *      This function get the dns domain name from list.
 *
 *
 *
 * INPUT:
 *      I8_T *dns_ip_domain_name    --  current dommain name of list.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *
 */
BOOL_T DNS_OM_GetDomainNameList(I8_T *dns_ip_domain_name)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */
    DNS_IpDomain_T *dns_ip_domain_list_head_p;
    UI32_T orig_priority;

    /* BODY */
    if ( dns_ip_domain_name == NULL )
    {
        return FALSE;
    }
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */

    dns_ip_domain_list_head_p = gdns_config.DnsIpDomainList_p;
    if ( dns_ip_domain_list_head_p == NULL )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return FALSE;
    }

    if ( strcmp((char *)dns_ip_domain_name, "") == 0 )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return FALSE;
    }
    else
    {
        while ( dns_ip_domain_list_head_p != NULL )
        {
            if ( strcmp((char *)dns_ip_domain_name, (char *)dns_ip_domain_list_head_p->DnsIpDomainName) == 0 )
            {
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
                return TRUE;
            }
            else
            {
                dns_ip_domain_list_head_p = dns_ip_domain_list_head_p->next_p;
            }
        }
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return FALSE;
    }
}

/* FUNCTION NAME : DNS_OM_GetNextNameServerList
 * PURPOSE:
 *      This function get next the domain name server from list.
 *
 *
 * INPUT:
 *      UI32_T *ip  --  current doamin name server ip.
 *
 * OUTPUT:
 *      UI32_T *ip  --  next doamin name server ip.
 *
 * RETURN:
 *      TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *      The initial ip value is zero.
 */
BOOL_T DNS_OM_GetNextNameServerList(L_INET_AddrIp_T *ip_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */
    DNS_ResConfigSbelt_T   *sbelt_table_p;
    UI32_T orig_priority;

    /* BODY */
    if ( ip_p == NULL )
    {
        return FALSE;
    }
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */

    sbelt_table_p=gdns_config.DnsResConfig.dns_res_config_sbelt_table_p;
    if ( sbelt_table_p == NULL )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return FALSE;
    }

    if ( ip_p->addrlen == 0 )
    {
        memcpy(ip_p, &sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr, sizeof(L_INET_AddrIp_T));
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return TRUE;
    }
    else
    {
        while ( sbelt_table_p != NULL )
        {
            if (L_INET_CompareInetAddr((L_INET_Addr_T *) ip_p,
                (L_INET_Addr_T *) & sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr,
                 0) == 0)
            {
                sbelt_table_p = sbelt_table_p->next_p;
                if ( sbelt_table_p != NULL )
                {
                    memcpy(ip_p, &sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr, sizeof(L_INET_AddrIp_T));
                    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
                    return TRUE;
                }
                else
                {
                    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
                    return FALSE;
                }
            }
            else
            {
                sbelt_table_p = sbelt_table_p->next_p;
            }
        }
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return FALSE;
    }
}

/* FUNCTION NAME : DNS_OM_GetNameServerList
 * PURPOSE:
 *      This function get next the domain name server from list.
 *
 *
 * INPUT:
 *      UI32_T *ip  --  current doamin name server ip.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *
 */
BOOL_T DNS_OM_GetNameServerList(L_INET_AddrIp_T *ip_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */
    DNS_ResConfigSbelt_T   *sbelt_table_p;
    UI32_T orig_priority;

    /* BODY */
    if ( ip_p == NULL )
    {
        return FALSE;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    sbelt_table_p=gdns_config.DnsResConfig.dns_res_config_sbelt_table_p;
    if ( sbelt_table_p == NULL )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return FALSE;
    }

    if ( ip_p->addrlen == 0 )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return FALSE;
    }
    else
    {
        while ( sbelt_table_p != NULL )
        {
            if (L_INET_CompareInetAddr((L_INET_Addr_T *) ip_p,
                (L_INET_Addr_T *) & sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr,
                 0) == 0)
            {
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
                return TRUE;
            }
            else
            {
                sbelt_table_p = sbelt_table_p->next_p;
            }
        }
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return FALSE;
    }
}



/* FUNCTION NAME : DNS_OM_SetResResetStatus
 *
 * PURPOSE:
 *      Set resolver reset status.
 *
 * INPUT:
 *      none.
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
void DNS_OM_SetResResetStatus(int status)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsResConfig.dnsResConfigReset = status;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_SetDnsResConfigMaxCnames
 * PURPOSE:
 *      Limit on how many CNAMEs the resolver should allow
 *      before deciding that there's a CNAME loop.  Zero means
 *      that resolver has no explicit CNAME limit.
 * INPUT:
 *      UI32_T  -- dns_resconfig_max_cnames
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_SetDnsResConfigMaxCnames(UI32_T dns_resconfig_max_cnames_p)
{
    UI32_T orig_priority;


    if(0 < dns_resconfig_max_cnames_p)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    gdns_config.DnsResConfig.dnsResConfigMaxCnames = dns_resconfig_max_cnames_p;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;

}



/* FUNCTION NAME : DNS_OM_SetDnsResConfigSbeltName
 * PURPOSE:
 *      This funciton set the specified DnsResConfigSbeltEntry according to the index.
 *
 *
 *
 * INPUT:
 *      DNS_ResConfigSbeltEntry_T * -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the value to be set
 *                                   INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_SetDnsResConfigSbeltName(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
    DNS_ResConfigSbelt_T   *sbelt_table_p;
    L_INET_AddrIp_T ip_addr;
    UI32_T orig_priority;

    memset(&ip_addr, 0, sizeof(ip_addr));

    if (NULL==dns_res_config_sbelt_entry_t_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    sbelt_table_p=gdns_config.DnsResConfig.dns_res_config_sbelt_table_p;
    if (NULL==sbelt_table_p)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_ERROR;
    }

    memcpy(&ip_addr, &dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltAddr, sizeof(L_INET_AddrIp_T));
    while(sbelt_table_p)
    {
        if (L_INET_CompareInetAddr((L_INET_Addr_T *) & sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr,
            (L_INET_Addr_T *) &ip_addr, 0) == 0)
        {
            strcpy((char *)sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltName, (char *)dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltName);
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
            return DNS_OK;
        }
        sbelt_table_p=sbelt_table_p->next_p;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_ERROR;
}



/* FUNCTION NAME : DNS_OM_SetDnsResConfigSbeltRecursion
 * PURPOSE:
 *      This funciton set the specified DnsResConfigSbeltEntry according to the index.
 *
 *
 *
 * INPUT:
 *      DNS_ResConfigSbeltEntry_T * -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the value to be set
 *                                   INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_SetDnsResConfigSbeltRecursion(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
    DNS_ResConfigSbelt_T   *sbelt_table_p;
    L_INET_AddrIp_T ip_addr;
    UI32_T orig_priority;

    memset(&ip_addr, 0, sizeof(ip_addr));

    if (NULL==dns_res_config_sbelt_entry_t_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    sbelt_table_p=gdns_config.DnsResConfig.dns_res_config_sbelt_table_p;
    if (NULL==sbelt_table_p)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_ERROR;
    }

    memcpy(&ip_addr, &dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltAddr, sizeof(ip_addr));

    while(sbelt_table_p)
    {
        if (L_INET_CompareInetAddr((L_INET_Addr_T *) & sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr,
            (L_INET_Addr_T *) &ip_addr, 0) == 0)
        {
            sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltRecursion=dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltRecursion;
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
            return DNS_OK;
        }
        sbelt_table_p=sbelt_table_p->next_p;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_ERROR;
}



/* FUNCTION NAME : DNS_OM_SetDnsResConfigSbeltPref
 * PURPOSE:
 *      This funciton set the specified DnsResConfigSbeltEntry according to the index.
 *
 *
 *
 * INPUT:
 *      DNS_ResConfigSbeltEntry_T * -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the value to be set
 *                                   INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_SetDnsResConfigSbeltPref(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
    DNS_ResConfigSbelt_T   *sbelt_table_p;
    L_INET_AddrIp_T ip_addr;
    UI32_T orig_priority;

    memset(&ip_addr, 0, sizeof(ip_addr));

    if (NULL==dns_res_config_sbelt_entry_t_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    sbelt_table_p=gdns_config.DnsResConfig.dns_res_config_sbelt_table_p;
    if (NULL==sbelt_table_p)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_ERROR;
    }

    memcpy(&ip_addr, &dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltAddr, sizeof(ip_addr));
    while(sbelt_table_p)
    {
        if (L_INET_CompareInetAddr((L_INET_Addr_T *) & sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr,
            (L_INET_Addr_T *) &ip_addr, 0) == 0)
        {
            sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltPref=dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltPref;
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
            return DNS_OK;
        }
        sbelt_table_p=sbelt_table_p->next_p;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_ERROR;
}



/* FUNCTION NAME : DNS_OM_SetDnsResConfigSbeltStatus
 * PURPOSE:
 *      This funciton set the specified DnsResConfigSbeltEntry according to the index.
 *
 *
 *
 * INPUT:
 *      DNS_ResConfigSbeltEntry_T * -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the value to be set
 *                                   INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success
 *
 * NOTES:
 *       This function will be called by snmp module.
 */
int DNS_OM_SetDnsResConfigSbeltStatus(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
    DNS_ResConfigSbelt_T   *sbelt_table_p;
    L_INET_AddrIp_T ip_addr;
    UI32_T orig_priority;

    memset(&ip_addr, 0, sizeof(ip_addr));

    if (NULL==dns_res_config_sbelt_entry_t_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    sbelt_table_p=gdns_config.DnsResConfig.dns_res_config_sbelt_table_p;
    if (NULL==sbelt_table_p)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_ERROR;
    }

    memcpy(&ip_addr, &dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltAddr, sizeof(ip_addr));
    while(sbelt_table_p)
    {
        if (L_INET_CompareInetAddr((L_INET_Addr_T *) & sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr,
            (L_INET_Addr_T *) &ip_addr, 0) == 0)
        {
            sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltStatus=dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltStatus;
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
            return DNS_OK;
        }
        sbelt_table_p=sbelt_table_p->next_p;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_ERROR;
}



/* FUNCTION NAME : DNS_OM_GetDefaultDnsResConfigSbeltEntry
 * PURPOSE:
 *      This funciton get the specified DnsResConfigSbeltEntry according to the index.
 *
 *
 * INPUT:
 *      DNS_ResConfigSbeltEntry_T* -- INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *      DNS_ResConfigSbeltEntry_T* -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the returned value
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *      DNS_OK    : success.
 * NOTES:
 *       This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDefaultDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
    UI32_T orig_priority;

    if (NULL==dns_res_config_sbelt_entry_t_p)
        return DNS_ERROR;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    strcpy((char *)dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltName,"");
    dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltRecursion = DNS_RES_SERVICE_MIX;
    dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltPref = 1;
    dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltStatus = 0;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}



/* FUNCTION NAME : DNS_OM_SetNameServerByIndex
 * PURPOSE:
 *  This function adds a name server IP address to the name server list by index.
 *
 *
 * INPUT:
 *      UI32_T  index   --  index of name server.
 *      UI32_T  ip_addr --  ip addr will be added as a name server;
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 *  DNS_SERVER_EXISTED : ip already exist
 * NOTES:
 *      This function will be called by SNMP.
 */
int DNS_OM_SetNameServerByIndex(UI32_T index, L_INET_AddrIp_T *addr_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    DNS_ResConfigSbelt_T   *sbelt_table_p,*sbelt_new_node_p;
    UI32_T                  l_index = 1;
    UI32_T orig_priority;

    /* BODY */
    if( index > number_of_name_server_entry+1 )
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    sbelt_table_p=gdns_config.DnsResConfig.dns_res_config_sbelt_table_p;
    if(NULL==sbelt_table_p)     /* add first name server, index muust be 1 */
    {
        if( index != 1 )
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
            return DNS_ERROR;
        }
        sbelt_new_node_p=DNS_OM_SBELT_TABLE_ENTRY_MALLOC();
        if (NULL==sbelt_new_node_p)
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
            return DNS_ERROR;
        }

        memcpy(&sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr, addr_p, sizeof(L_INET_AddrIp_T));
        sbelt_new_node_p->next_p=NULL;
        sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltName[0]='\0';
        sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltClass=0;
        sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltSubTree[0]='\0';
        sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltPref=1;
        sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltStatus=0;
        sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltRecursion = DNS_RES_SERVICE_MIX;/*isiah*/
        gdns_config.DnsResConfig.dns_res_config_sbelt_table_p=sbelt_new_node_p;
        /*isiah*/
        number_of_name_server_entry = 1;
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_OK;
    }
    else
    {
        if( index != 1 )
        {
            /* deal with the first node in the list
             */
            if (L_INET_CompareInetAddr((L_INET_Addr_T *) & sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr,
                (L_INET_Addr_T *) addr_p, 0) == 0)
            {
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
                return DNS_SERVER_EXISTED;
            }
        }

        l_index+=1;
        while( NULL!=sbelt_table_p->next_p )
        {
            sbelt_table_p=sbelt_table_p->next_p;
            if( index != l_index )
            {
                if (L_INET_CompareInetAddr((L_INET_Addr_T *) & sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr,
                    (L_INET_Addr_T *) addr_p, 0) == 0)
                {
                    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
                    return DNS_SERVER_EXISTED;
                }
            }
            l_index += 1;
        }

        if( index == number_of_name_server_entry+1 )
        {
            sbelt_new_node_p=DNS_OM_SBELT_TABLE_ENTRY_MALLOC();
            if (NULL==sbelt_new_node_p)
            {
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
                return DNS_ERROR;
            }

            memcpy(&sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr, addr_p, sizeof(L_INET_AddrIp_T));
            sbelt_new_node_p->next_p=NULL;
            sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltName[0]='\0';
            sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltClass=0;
            sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltSubTree[0]='\0';
            sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltPref=1;
            sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltStatus=0;
            sbelt_new_node_p->dns_res_config_sbelt_entry.dnsResConfigSbeltRecursion = DNS_RES_SERVICE_MIX;/*isiah*/
            sbelt_table_p->next_p=sbelt_new_node_p;
            /*DNS_RESOLVER_SbeltInit();         */
            number_of_name_server_entry += 1;
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
            return DNS_OK;
        }
        else
        {
            sbelt_table_p=gdns_config.DnsResConfig.dns_res_config_sbelt_table_p;
            if( index == 1 )
            {
                memcpy(&sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr, addr_p, sizeof(L_INET_AddrIp_T));
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
                return DNS_OK;
            }
            else
            {
                l_index = 2;
                while( NULL!=sbelt_table_p->next_p )
                {
                    sbelt_table_p=sbelt_table_p->next_p;
                    if( index == l_index )
                    {
                        memcpy(&sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr, addr_p, sizeof(L_INET_AddrIp_T));
                        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
                        return DNS_OK;
                    }
                    l_index += 1;
                }
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
                return DNS_ERROR;
            }
        }
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}



/* FUNCTION NAME : DNS_OM_GetNameServerByIndex
 * PURPOSE:
 *      This function get the domain name server by index from list.
 *
 *
 * INPUT:
 *      UI32_T  index   --  index of name server.
 *      UI32_T  *ip     --  current doamin name server ip.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *      This function will be called by snmp module.
 */
BOOL_T DNS_OM_GetNameServerByIndex(UI32_T index, L_INET_AddrIp_T *ip)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    DNS_ResConfigSbelt_T   *sbelt_table_p;
    UI32_T                  l_index = 1;
    UI32_T orig_priority;

    /* BODY */
    if ( ip == NULL )
    {
        return FALSE;
    }

    if((index == 0)|| (index > number_of_name_server_entry))
    {
        return FALSE;
    }


    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */
    sbelt_table_p=gdns_config.DnsResConfig.dns_res_config_sbelt_table_p;
    if ( sbelt_table_p == NULL )
    {
        memset(ip, 0, sizeof(L_INET_AddrIp_T));
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return FALSE;
    }

    while ( sbelt_table_p != NULL )
    {
        if( index == l_index )
        {
            memcpy(ip, &sbelt_table_p->dns_res_config_sbelt_entry.dnsResConfigSbeltAddr, sizeof(L_INET_AddrIp_T));
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
            return TRUE;
        }
        else
        {
            sbelt_table_p = sbelt_table_p->next_p;
            l_index += 1;
        }
    }
    memset(ip, 0, sizeof(L_INET_AddrIp_T));
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return TRUE;
}


 /*maggie liu, ES4827G-FLF-ZZ-00244*/
/* FUNCTION NAME : DNS_OM_DeleteNameServerAll
 * PURPOSE:
 *  This function delete all name server IP address from the name server list.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *      This function will be called by CLI and CGI.
 */
int DNS_OM_DeleteNameServerAll()
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    DNS_ResConfigSbelt_T    *sbelt_table_p;
    UI32_T orig_priority;

    /* BODY */
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */

    sbelt_table_p=gdns_config.DnsResConfig.dns_res_config_sbelt_table_p;
    if(NULL==sbelt_table_p)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_OK;
    }
    else
    {
        while(sbelt_table_p)
        {
            gdns_config.DnsResConfig.dns_res_config_sbelt_table_p=sbelt_table_p->next_p;
            DNS_OM_SBELT_TABLE_ENTRY_FREE(sbelt_table_p);
            sbelt_table_p=gdns_config.DnsResConfig.dns_res_config_sbelt_table_p;
            number_of_name_server_entry -= 1;
        }
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_OK;
    }
}

/*maggie liu for es4827g-FLF-00244*/
/* FUNCTION NAME : DNS_OM_DeleteNameServerByIndex
 * PURPOSE:
 *  This function delete a name server IP address to the name server list by index.
 *
 *
 * INPUT:
 *      UI32_T  index   --  index of name server.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *      This function will be called by SNMP.
 */
int DNS_OM_DeleteNameServerByIndex(UI32_T index)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T                   l_index;
    DNS_ResConfigSbelt_T    *sbelt_table_p,*prev_p;
    l_index = 0;
    UI32_T orig_priority;

    /* BODY */
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */

    sbelt_table_p=gdns_config.DnsResConfig.dns_res_config_sbelt_table_p;
    prev_p=sbelt_table_p;

    if(NULL==sbelt_table_p)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_OK;
    }
    else
  	{
        while(sbelt_table_p)
        {
            l_index = l_index + 1;
            if (index == 1)
            {
                gdns_config.DnsResConfig.dns_res_config_sbelt_table_p = sbelt_table_p->next_p;
                DNS_OM_SBELT_TABLE_ENTRY_FREE(sbelt_table_p);
                sbelt_table_p = NULL;
                number_of_name_server_entry -= 1;

                SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    	        return DNS_OK;

            }
            if (l_index == index)
            {
                prev_p->next_p = sbelt_table_p->next_p;
                DNS_OM_SBELT_TABLE_ENTRY_FREE(sbelt_table_p);
                number_of_name_server_entry -= 1;
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    	        return DNS_OK;

            }
            prev_p = sbelt_table_p;
            sbelt_table_p = sbelt_table_p->next_p;
        }
  	}

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}

/* River@May 7, 2008, add nslookup mib */

/* FUNCTION NAME : DNS_OM_GetNextLookupCtlTableAndIndex
 * PURPOSE:
 *  This function get next Nslookup control table .
 *
 *
 * INPUT:
 *     CTRL_table   --  Nslookup control table
 *     index_p      --  pointer to get 0-based OM index
 *
 * OUTPUT:
 *     *index_p     -- 0-based OM index
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetNextLookupCtlTableAndIndex(DNS_Nslookup_CTRL_T *CTRL_table, UI32_T *index_p)
{
    UI32_T i, least;
    DNS_Nslookup_CTRL_Head_T temp;
    BOOL_T first = TRUE;
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);
    least = DNS_DEF_NSLOOKUP_REQUEST;

    for (i = 0; i < DNS_DEF_NSLOOKUP_REQUEST; i++)
    {
        if(memcmp(CTRL_table, &nslookup_ctrl_table[i], sizeof(DNS_Nslookup_CTRL_Head_T)) < 0) /* bigger than header */
        {
            if (first)
            {
                memcpy(&temp, &nslookup_ctrl_table[i], sizeof(DNS_Nslookup_CTRL_Head_T));
                first = FALSE;
                least = i;
            }
            else
            {
                if (memcmp(&temp, &nslookup_ctrl_table[i], sizeof(DNS_Nslookup_CTRL_Head_T)) > 0) /* smaller than temp */
                {
                    memcpy(&temp, &nslookup_ctrl_table[i], sizeof(DNS_Nslookup_CTRL_Head_T));
                    least = i;
                }
            }
        } /* end of if bigger than header */
    }

    if (least == DNS_DEF_NSLOOKUP_REQUEST)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_ERROR;
    }

    memcpy(CTRL_table, &nslookup_ctrl_table[least], sizeof(DNS_Nslookup_CTRL_T));
    *index_p = least;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_GetNextLookupCtlTable
 * PURPOSE:
 *  This function get next Nslookup control table .
 *
 *
 * INPUT:
 *     CTRL_table   --  Nslookup control table
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetNextLookupCtlTable(DNS_Nslookup_CTRL_T *CTRL_table)
{
    int rc;
    UI32_T index;

    rc = DNS_OM_GetNextLookupCtlTableAndIndex(CTRL_table, &index);
    return rc;
}

/* FUNCTION NAME : DNS_OM_GetLookupCtlTableAndIndex
 * PURPOSE:
 *  This function get next Nslookup control table.
 *
 *
 * INPUT:
 *     CTRL_table   --  Nslookup control table
 *     index_p      --  pointer to get 0-based OM index
 *
 * OUTPUT:
 *     *index_p     -- 0-based OM index
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetLookupCtlTableAndIndex(DNS_Nslookup_CTRL_T *CTRL_table, UI32_T *index_p)
{
    int i;
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);
    /* find matching entry
     */
    for (i = 0; i < DNS_DEF_NSLOOKUP_REQUEST; i++)
    {
        if (memcmp(CTRL_table, &nslookup_ctrl_table[i], sizeof(DNS_Nslookup_CTRL_Head_T)) == 0)
        {
            break;
        }
    }

    /* if entry does not exist
     */
    if (i == DNS_DEF_NSLOOKUP_REQUEST)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_ERROR;
    }

    /* copy content and index
     */
    memcpy(CTRL_table, &nslookup_ctrl_table[i], sizeof(DNS_Nslookup_CTRL_T));
    *index_p = i;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_GetLookupCtlTable
 * PURPOSE:
 *  This function get next Nslookup control table.
 *
 *
 * INPUT:
 *     CTRL_table   --  Nslookup control table
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetLookupCtlTable(DNS_Nslookup_CTRL_T *CTRL_table)
{
    int rc;
    UI32_T index;

    rc = DNS_OM_GetLookupCtlTableAndIndex(CTRL_table, &index);
    return rc;
}

/* FUNCTION NAME : DNS_OM_SetDNSCtlTable_TargetAddressType
 * PURPOSE:
 *  This function set DNS control table TagetAddress Type.
 *
 *
 * INPUT:
 *     CTRL_table   --  Nslookup control table
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_SetDNSCtlTable_TargetAddressType(DNS_Nslookup_CTRL_T *CTRL_table)
{
    int i;
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);

    for (i = 0; i < DNS_DEF_NSLOOKUP_REQUEST; i++)
    {
        if (0 == memcmp(CTRL_table, &nslookup_ctrl_table[i], sizeof(DNS_Nslookup_CTRL_Head_T)))
            break;
    }

    /* if entry does not exist
     */
    if (i == DNS_DEF_NSLOOKUP_REQUEST)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_ERROR;
    }

    nslookup_ctrl_table[i].TargetAddressType = CTRL_table->TargetAddressType;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_SetDNSCtlTable_TargetAddress
 * PURPOSE:
 *  This function set DNS control table TagetAddress Type.
 *
 *
 * INPUT:
 *     CTRL_table   --  Nslookup control table
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_SetDNSCtlTable_TargetAddress(DNS_Nslookup_CTRL_T *CTRL_table)
{
    int i;
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);

    for (i = 0; i < DNS_DEF_NSLOOKUP_REQUEST; i++)
    {
        if (0 == memcmp(CTRL_table, &nslookup_ctrl_table[i], sizeof(DNS_Nslookup_CTRL_Head_T)))
            break;
    }

    /* if entry does not exist
     */
    if (i == DNS_DEF_NSLOOKUP_REQUEST)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_ERROR;
    }

    memcpy(nslookup_ctrl_table[i].TargetAddress, CTRL_table->TargetAddress, CTRL_table->TargetAddressLen);
    nslookup_ctrl_table[i].TargetAddressLen = CTRL_table->TargetAddressLen;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_SetLookupCtlOperStatusByIndex
 * PURPOSE:
 *  This function sets lookupCtlOperStatus by index.
 *
 * INPUT:
 *      index       -- 0-based OM index of the entry
 *      oper_status -- new value of lookupCtlOperStatus
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 *
 * NOTES:
 *
 */
int DNS_OM_SetLookupCtlOperStatusByIndex(UI32_T index, UI32_T oper_status)
{
    UI32_T orig_priority;

    if (index >= DNS_DEF_NSLOOKUP_REQUEST)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);
    nslookup_ctrl_table[index].OperStatus = oper_status;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_SetLookupCtlRcByIndex
 * PURPOSE:
 *  This function sets lookupCtlRc by index.
 *
 * INPUT:
 *      index       -- 0-based OM index of the entry
 *      oper_status -- new value of lookupCtlRc
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 *
 * NOTES:
 *
 */
int DNS_OM_SetLookupCtlRcByIndex(UI32_T index, UI32_T rc)
{
    UI32_T orig_priority;

    if (index >= DNS_DEF_NSLOOKUP_REQUEST)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);
    nslookup_ctrl_table[index].Rc = rc;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_SetLookupCtlTimeByIndex
 * PURPOSE:
 *  This function set DNS control table's Time by index.
 *
 * INPUT:
 *      index       -- 0-based OM index of the entry
 *      oper_status -- new value of lookupCtlTime
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 *
 * NOTES:
 *
 */
int DNS_OM_SetLookupCtlTimeByIndex(UI32_T index, UI32_T time)
{
    UI32_T orig_priority;

    if (index >= DNS_DEF_NSLOOKUP_REQUEST)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);
    nslookup_ctrl_table[index].Time = time;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_SetLookupCtlRowStatusByIndex
 * PURPOSE:
 *  This function sets lookupCtlRowStatus by index.
 *
 * INPUT:
 *      index       -- 0-based OM index of the entry
 *      oper_status -- new value of RowStatus
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 *
 * NOTES:
 *
 */
int DNS_OM_SetLookupCtlRowStatusByIndex(UI32_T index, UI32_T row_status)
{
    UI32_T orig_priority;

    if (index >= DNS_DEF_NSLOOKUP_REQUEST)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);
    nslookup_ctrl_table[index].RowStatus = row_status;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_SetLookupResultsIndexByIndex
 * PURPOSE:
 *  This function sets lookupResultsIndex by index.
 *
 *
 * INPUT:
 *      ctl_index       -- 0-based OM index of the control table
 *      result_index    -- 0-based OM index of the results table
 *      user_index      -- 1-based user index of the results table
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 *
 * NOTES:
 *
 */
int DNS_OM_SetLookupResultsIndexByIndex(UI32_T ctl_index, UI32_T result_index,
    UI32_T user_index)
{
    UI32_T orig_priority;

    if ((ctl_index >= DNS_DEF_NSLOOKUP_REQUEST)
        || (result_index >= DNS_MAXNSLOOKUPHOSTIPNUM))
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);
    nslookup_result_table[ctl_index][result_index].ResultsIndex = user_index;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_SetLookupResultsAddressByIndex
 * PURPOSE:
 *  This function sets lookupResultsAddress by index.
 *
 *
 * INPUT:
 *      ctl_index       -- 0-based OM index of the control table
 *      result_index    -- 0-based OM index of the results table
 *      addr_str_p      -- pointer to L_INET_AddrIp_T structure
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 *
 * NOTES:
 *
 */
int DNS_OM_SetLookupResultsAddressByIndex(UI32_T ctl_index, UI32_T result_index,
    L_INET_AddrIp_T *addr_str_p)
{
    UI32_T orig_priority;

    if ((ctl_index >= DNS_DEF_NSLOOKUP_REQUEST)
        || (result_index >= DNS_MAXNSLOOKUPHOSTIPNUM))
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);
    memcpy(& nslookup_result_table[ctl_index][result_index].ResultsAddress_str,
        addr_str_p, sizeof(L_INET_AddrIp_T));
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_GetNextLookupResultTable
 * PURPOSE:
 *  This function get next nslookup result table.
 *
 *
 * INPUT:
 *     Result_table   --  nslookup result table.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetNextLookupResultTable(DNS_Nslookup_Result_T *Result_table)
{
    UI32_T i, j;
    UI32_T matchi, matchj;
    DNS_Nslookup_Result_T temp;
    BOOL_T first = TRUE;
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);

    matchi = DNS_DEF_NSLOOKUP_REQUEST;
    matchj = MAXHOSTIPNUM;

    memset(&temp, 0, sizeof(DNS_Nslookup_Result_T));
    for (i = 0; i < DNS_DEF_NSLOOKUP_REQUEST; i++)
    {
        for (j = 0; j < DNS_MAXNSLOOKUPHOSTIPNUM; j++)  /*maggie liu, NSLOOKUP*/
        {
            if (nslookup_result_table[i][j].CtlOwnerIndexLen != 0 &&
                nslookup_result_table[i][j].OperationNameLen != 0 &&
                nslookup_result_table[i][j].ResultsIndex != 0) /* only compare exsit entry */
            {
                if (gdns_config.DnsDebugStatus == DNS_ENABLE)
                {
                    printf("\nenter DNS_OM_GetNextLookupResultTable\n");
                    printf("compare entrys\n");
                }

                if (memcmp(Result_table, &nslookup_result_table[i][j], sizeof(DNS_Nslookup_Result_Head_T)) < 0) /* bigger than Result_table */
                {
                    if (first)
                    {
                        memcpy(&temp, &nslookup_result_table[i][j], sizeof(DNS_Nslookup_Result_T));
                        first = FALSE;
                        matchi = i;
                        matchj = j;
                    }
                    else
                    {
                        if (memcmp(&temp, &nslookup_result_table[i][j], sizeof(DNS_Nslookup_Result_Head_T)) > 0) /* smaller than temp */
                        {
                            memcpy(&temp, &nslookup_result_table[i][j], sizeof(DNS_Nslookup_Result_T));
                            matchi = i;
                            matchj = j;
                        }
                    }
                } /* end of if bigger than header */
            }
        }
    }

    if (matchi == DNS_DEF_NSLOOKUP_REQUEST && matchj == MAXHOSTIPNUM)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_ERROR;
    }

    memcpy(Result_table, &temp, sizeof(DNS_Nslookup_Result_T));
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}



/* FUNCTION NAME : DNS_OM_GetLookupResultTable
 * PURPOSE:
 *  This function This function get nslookup result table.
 *
 *
 * INPUT:
 *     intable   --  nslookup result table.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetLookupResultTable(DNS_Nslookup_Result_T *Result_table)
{
    int i, j;
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);

    for (i = 0; i < DNS_DEF_NSLOOKUP_REQUEST; i++)
    {
        for (j = 0; j < DNS_MAXNSLOOKUPHOSTIPNUM; j++) /*maggie liu, NSLOOKUP*/
        {
            if (nslookup_result_table[i][j].CtlOwnerIndexLen != 0 &&
                nslookup_result_table[i][j].OperationNameLen != 0 &&
                nslookup_result_table[i][j].ResultsIndex != 0)
            {
                if (memcmp(Result_table, &nslookup_result_table[i][j], sizeof(DNS_Nslookup_Result_Head_T)) == 0)
                {
                    memcpy(Result_table, &nslookup_result_table[i][j], sizeof(DNS_Nslookup_Result_T));
                    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
                    return DNS_OK;
                }
            }
        }
    }
#if 0
    if (i == DNS_DEF_NSLOOKUP_REQUEST && j == MAXHOSTIPNUM)
        return DNS_ERROR;
#endif

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_ERROR;

}

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
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
BOOL_T DNS_Nslookup_TargetAddrCheck(void *addr_len)
{
    if ((*(UI32_T *)addr_len) == 0) /*Grace.zheng,24-10-2006*/
        return FALSE;

    return TRUE;
}

/* FUNCTION NAME : DNS_OM_SetNslookupTimeOut
 * PURPOSE:
 *  This function delete nslookup control and result tables.
 *
 *
 * INPUT:
 *     index        --  index of nslookup table that is been deleted.
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_SetNslookupTimeOut(UI32_T index, UI32_T timeout)
{
    UI32_T orig_priority;

    if (index >= DNS_DEF_NSLOOKUP_REQUEST)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);
    nslookup_table_timeout[index] = timeout;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}


/* FUNCTION NAME : DNS_OM_GetNslookupTimeOut
 * PURPOSE:
 *  This function delete nslookup control and result tables.
 *
 *
 * INPUT:
 *     index        --  index of nslookup table that is been deleted.
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
void DNS_OM_GetNslookupTimeOut(UI32_T *timeout, UI32_T index)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);
    *timeout = nslookup_table_timeout[index];
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return;
}


/* FUNCTION NAME : DNS_OM_Nslookup_DeleteEntry
 * PURPOSE:
 *  This function delete nslookup control and result tables.
 *
 *
 * INPUT:
 *     index        --  index of nslookup table that is been deleted.
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_Nslookup_DeleteEntry(UI32_T index)
{
    DNS_Nslookup_DeleteEntry(index);
    return DNS_OK;
}


/* FUNCTION NAME : DNS_Nslookup_DeleteEntry
 * PURPOSE:
 *  This function delete nslookup control and result tables.
 *
 *
 * INPUT:
 *     index        --  index of nslookup table that is been deleted.
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_Nslookup_DeleteEntry(UI32_T index)
{
    int i;
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);
    /* delete control table */
    memset(&nslookup_ctrl_table[index], 0, sizeof(DNS_Nslookup_CTRL_T));

    /* delete result table list */
    for(i = 0; i < DNS_MAXNSLOOKUPHOSTIPNUM; i++) /*maggie liu, NSLOOKUP*/
        memset(&nslookup_result_table[index][i], 0, sizeof(DNS_Nslookup_Result_T));

    nslookup_table_timeout[index] = 0;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}

/* FUNCTION NAME : DNS_OM_GetNslookupPurgeTime
 * PURPOSE:
 *  This function get nslookup PurgeTime.
 *
 *
 * INPUT:
 *     purge_time   --  nslookup PurgeTime.
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  TRUE :failure,
 *  FALSE:success.
 * NOTES:
 *
 */
void DNS_OM_GetNslookupPurgeTime(UI32_T *purge_time)
{
    *purge_time = DNS_Nslookup_PurgeTime();
    return;
}

/* FUNCTION NAME : DNS_OM_SetNslookupPurgeTime
 * PURPOSE:
 *  This function set nslookup PurgeTime.
 *
 *
 * INPUT:
 *     purge_time   --  nslookup PurgeTime.
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  TRUE :failure,
 *  FALSE:success.
 * NOTES:
 *
 */
void DNS_OM_SetNslookupPurgeTime(UI32_T purge_time)
{
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);
    nslookup_purgetime = purge_time;
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return;
}


/* FUNCTION NAME : DNS_Nslookup_PurgeTime
 * PURPOSE:
 *  This function return nslookup PurgeTime.
 *
 *
 * INPUT: none
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  TRUE :failure,
 *  FALSE:success.
 * NOTES:
 *
 */
UI32_T DNS_Nslookup_PurgeTime(void)
{
    return nslookup_purgetime;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_OM_IsNslookupCtlTableFull
 *-------------------------------------------------------------------------
 * PURPOSE  : Check nslookup control table is full or not.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T DNS_OM_IsNslookupCtlTableFull(void)
{
    UI32_T orig_priority;
    UI32_T ctl_index;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);

    for (ctl_index = 0; ctl_index < _countof(nslookup_ctrl_table); ++ctl_index)
    {
        if (0 == nslookup_ctrl_table[ctl_index].CtlOwnerIndexLen)
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
            return FALSE;
        }
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_OM_GetNslookupCtlEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get nslookup control entry by index.
 * INPUT    : ctl_index  -- 0-based nslookup control entry index
 * OUTPUT   : None
 * RETUEN   : DNS_OK/DNS_ERROR
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
int
DNS_OM_GetNslookupCtlEntryByIndex(
    UI32_T ctl_index,
    DNS_Nslookup_CTRL_T *ctl_entry_p)
{
    DNS_Nslookup_CTRL_T *local_ctl_entry_p;
    UI32_T orig_priority;

    if (NULL == ctl_entry_p)
    {
        return DNS_ERROR;
    }

    local_ctl_entry_p = DNS_OM_LocalGetNslookupCtlEntryByIndex(ctl_index);

    if (NULL == local_ctl_entry_p)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);
    memcpy(ctl_entry_p, local_ctl_entry_p, sizeof(DNS_Nslookup_CTRL_T));
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_OM_CreateNslookupCtlEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Create nslookup control entry.
 * INPUT    : ctl_entry_p  -- nslookup control entry
 * OUTPUT   : ctl_index_p  -- 0-based nslookup control entry index
 * RETUEN   : DNS_OK/DNS_ERROR
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
int
DNS_OM_CreateNslookupCtlEntry(
    DNS_Nslookup_CTRL_T *ctl_entry_p,
    UI32_T *ctl_index_p)
{
    UI32_T orig_priority;
    UI32_T ctl_index;

    if ((NULL == ctl_entry_p) ||
        (NULL == ctl_index_p))
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);

    for (ctl_index = 0; ctl_index < _countof(nslookup_ctrl_table); ++ctl_index)
    {
        if (0 == nslookup_ctrl_table[ctl_index].CtlOwnerIndexLen)
        {
            break;
        }
    }

    /* if table is full
     */
    if (ctl_index == _countof(nslookup_ctrl_table))
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
        return DNS_ERROR;
    }

    memcpy(&nslookup_ctrl_table[ctl_index], ctl_entry_p, sizeof(DNS_Nslookup_CTRL_T));
    *ctl_index_p = ctl_index;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return DNS_OK;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_OM_GetNslookupResultEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get nslookup result entry by index.
 * INPUT    : ctl_index       -- 0-based nslookup control entry index
 *            result_index    -- 0-based nslookup result entry index
 *            result_entry_p  -- nslookup result entry
 * OUTPUT   : None
 * RETUEN   : DNS_OK/DNS_ERROR
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
int
DNS_OM_GetNslookupResultEntryByIndex(
    UI32_T ctl_index,
    UI32_T result_index,
    DNS_Nslookup_Result_T *result_entry_p)
{
    DNS_Nslookup_Result_T *local_result_entry_p;
    UI32_T orig_priority;

    if (NULL == result_entry_p)
    {
        return DNS_ERROR;
    }

    local_result_entry_p = DNS_OM_LocalGetNslookupResultEntryByIndex(ctl_index, result_index);

    if (NULL == local_result_entry_p)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);
    memcpy(result_entry_p, local_result_entry_p, sizeof(DNS_Nslookup_Result_T));
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_OM_CreateNslookupResultEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Create nslookup result entry by index.
 * INPUT    : ctl_index       -- 0-based nslookup control entry index
 *            result_index    -- 0-based nslookup result entry index
 *            result_entry_p  -- nslookup result entry
 * OUTPUT   : None
 * RETUEN   : DNS_OK/DNS_ERROR
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
int
DNS_OM_CreateNslookupResultEntryByIndex(
    UI32_T ctl_index,
    UI32_T result_index,
    DNS_Nslookup_Result_T *result_entry_p)
{
    DNS_Nslookup_Result_T *local_result_entry_p;
    UI32_T orig_priority;

    if (NULL == result_entry_p)
    {
        return DNS_ERROR;
    }

    local_result_entry_p = DNS_OM_LocalGetNslookupResultEntryByIndex(ctl_index, result_index);

    if (NULL == local_result_entry_p)
    {
        return DNS_ERROR;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);
    memcpy(local_result_entry_p, result_entry_p, sizeof(DNS_Nslookup_Result_T));
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return DNS_OK;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_OM_HandleIPCReqMsg
 *-------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for DNS om.
 * INPUT   : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT  : ipcmsg_p  --  output response ipc message buffer
 * RETUEN  : TRUE  - need to send response.
 *           FALSE - not need to send response.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T DNS_OM_HandleIPCReqMsg(SYSFUN_Msg_T *ipcmsg_p)
{
    if(ipcmsg_p == NULL)
        return FALSE;

    switch(DNS_OM_MSG_CMD(ipcmsg_p))
    {
        case DNS_OM_IPC_CMD_GET_SERVER_COUNTER_REQ_UNPARSES:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServCounterReqUnparses((int *)&data_p->dnsServCounterReqUnparses);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERVER_OPT_COUNTER_FRIENDS_AUTH_ANS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterFriendsAuthAns((int *)&data_p->dnsServOptCounterFriendsAuthAns);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDS_AUTH_NO_DATE_RESPS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterFriendsAuthNoDataResps(
            &data_p->dnsServOptCounterFriendsAuthNoDataResps);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SEROPT_COUNTER_FRIENDS_AUTH_NO_NAMES:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterFriendsAuthNoNames(
            (int *)&data_p->dnsServOptCounterFriendsAuthNoNames);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_DNS_SERVOPT_COUNTER_FRIENDS_ERRORS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterFriendsErrors(
            &data_p->dnsServOptCounterFriendsErrors);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_DNS_SERVOPT_COUNTER_FRIENDS_NON_AUTH_DATES:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterFriendsNonAuthDatas(
            &data_p->dnsServOptCounterFriendsNonAuthDatas);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDS_NON_AUTH_NO_DATAS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterFriendsNonAuthNoDatas(
            &data_p->dnsServOptCounterFriendsNonAuthNoDatas);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_GETSERVOPT_COUNTER_FRIENDS_OTHER_ERRORS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterFriendsOtherErrors(
            &data_p->dnsServOptCounterFriendsOtherErrors);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

/*
		case DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDS_NON_AUTH_NO_DATAS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterFriendsNonAuthNoDatas(
            &data_p->dnsServOptCounterFriendsNonAuthNoDatas);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }
 */

		case DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDSREFERRALS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterFriendsReferrals(
            &data_p->dnsServOptCounterFriendsReferrals);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

/*
		case DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDS_RELNAMES:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterFriendsRelNames(
            &data_p->dnsServOptCounterFriendsRelNames);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }
    */
        case DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDS_REQREFUSALS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterFriendsReqRefusals(
            &data_p->dnsServOptCounterFriendsReqRefusals);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDS_REQUNPARSES:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterFriendsReqUnparses(
            &data_p->dnsServOptCounterFriendsReqUnparses);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_AUTHANS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterSelfAuthAns(
            (int *)&data_p->dnsServOptCounterSelfAuthAns);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDS_RELNAMES:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterFriendsRelNames(
            &data_p->dnsServOptCounterFriendsRelNames);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_AUTH_NO_DATA_RESPS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterSelfAuthNoDataResps(
            (int *)&data_p->dnsServOptCounterSelfAuthNoDataResps);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_AUTH_NO_NAMES:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterSelfAuthNoNames(
            (int *)&data_p->dnsServOptCounterSelfAuthNoNames);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_ERRORS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterSelfErrors(
            (int *)&data_p->dnsServOptCounterSelfErrors);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_NON_AUTH_DATAS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterSelfNonAuthDatas(
            (int *)&data_p->dnsServOptCounterSelfNonAuthDatas);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_NON_AUTH_NO_DATAS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterSelfNonAuthNoDatas(
            (int *)&data_p->dnsServOptCounterSelfNonAuthNoDatas);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }
/*
        case DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_OTHER_ERRORS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterSelfOtherErrors(
            &data_p->dnsServOptCounterSelfOtherErrors);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }
    */
        case DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_REFERRALS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterSelfReferrals(
            (int *)&data_p->dnsServOptCounterSelfReferrals);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_RELNAMES:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterSelfRelNames(
            (int *)&data_p->dnsServOptCounterSelfRelNames);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_REQ_REFUSALS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterSelfReqRefusals(
            (int *)&data_p->dnsServOptCounterSelfReqRefusals);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_REQ_UNPARSES:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterSelfReqUnparses(
            (int *)&data_p->dnsServOptCounterSelfReqUnparses);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_OTHER_ERRORS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServOptCounterSelfOtherErrors(
            (int *)&data_p->dnsServOptCounterSelfOtherErrors);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_NAME_SERVER_BY_INDEX:
        {
            DNS_OM_IPCMsg_Name_Server_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetNameServerByIndex(
            data_p->index, &data_p->name_server_ip);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_Name_Server_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_DOMAIN_NAME_LIST:
        {
            DNS_OM_IPCMsg_IpDomain_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDomainNameList(
            data_p->DnsIpDomainName);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_IpDomain_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_DNS_RES_COUNTER_BY_OPCODE_ENTRY:
        {
            DNS_OM_IPCMsg_ResCounterByOpcodeEntry_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsResCounterByOpcodeEntry(
            (DNS_ResCounterByOpcodeEntry_T *)data_p);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounterByOpcodeEntry_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_RES_COUNTER_BY_RCODE_ENTRY:
        {
            DNS_OM_IPCMsg_ResCounterByRcodeEntry_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsResCounterByRcodeEntry(
            (DNS_ResCounterByRcodeEntry_T *)data_p);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounterByRcodeEntry_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_RES_COUNTER_FALLBACKS:
        {
            DNS_OM_IPCMsg_ResCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsResCounterFallbacks(
            (int *)&data_p->dnsResCounterFallbacks);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_RES_COUNTER_MARTIANS:
        {
            DNS_OM_IPCMsg_ResCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsResCounterMartians(
            &data_p->dnsResCounterMartians);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_RES_COUNTER_NON_AUTH_DATA_RESPS:
        {
            DNS_OM_IPCMsg_ResCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsResCounterNonAuthDataResps(
            (int *)&data_p->dnsResCounterNonAuthDataResps);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_RES_COUNTER_NON_AUTH_NO_DATA_RESPS:
        {
            DNS_OM_IPCMsg_ResCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsResCounterNonAuthNoDataResps(
            (int *)&data_p->dnsResCounterNonAuthNoDataResps);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T);
            break;
        }

		case DNS_OM_IPC_CMD_GET_RES_COUNTER_RECD_RESPONSES:
        {
            DNS_OM_IPCMsg_ResCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsResCounterRecdResponses(
            &data_p->dnsResCounterRecdResponses);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_RES_COUNTER_UNPARSE_RESPS:
        {
            DNS_OM_IPCMsg_ResCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsResCounterUnparseResps(
            &data_p->dnsResCounterUnparseResps);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_RES_OPT_COUNTER_INTERNALS:
        {
            DNS_OM_IPCMsg_ResCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsResOptCounterInternals(
            (int *)&data_p->dnsResOptCounterInternals);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_RES_OPT_COUNTER_INTERNAL_TIMEOUTS:
        {
            DNS_OM_IPCMsg_ResCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsResOptCounterInternalTimeOuts(
            (int *)&data_p->dnsResOptCounterInternalTimeOuts);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_RES_OPT_COUNTER_NO_RESPONSES:
        {
            DNS_OM_IPCMsg_ResCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsResOptCounterNoResponses(
            &data_p->dnsResOptCounterNoResponses);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_RES_OPT_COUNTER_REFERALS:
        {
            DNS_OM_IPCMsg_ResCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsResOptCounterReferals(
            (int *)&data_p->dnsResOptCounterReferals);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_RES_OPT_COUNTER_RETRANS:
        {
            DNS_OM_IPCMsg_ResCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsResOptCounterRetrans(
            (int *)&data_p->dnsResOptCounterRetrans);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_RES_OPT_COUNTER_ROOT_RETRANS:
        {
            DNS_OM_IPCMsg_ResCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsResOptCounterRootRetrans(
            (int *)&data_p->dnsResOptCounterRootRetrans);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ResCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERV_CONFIG_OMPLEMENT_IDENT:
        {
            DNS_OM_IPCMsg_ProxyConfig_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServConfigImplementIdent((I8_T *)
            &data_p->dnsServConfigImplementIdent);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyConfig_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERV_CONFIG_RECURS:
        {
            DNS_OM_IPCMsg_ProxyConfig_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServConfigRecurs(
            &data_p->dnsServConfigRecurs);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyConfig_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERV_CONFIG_RESET_TIME:
        {
            DNS_OM_IPCMsg_TIME_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServConfigResetTime(
            &data_p->time);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_TIME_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERV_CONFIG_UP_TIME:
        {
            DNS_OM_IPCMsg_TIME_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServConfigUpTime(
            &data_p->time);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_TIME_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERV_COUNTER_AUTH_ANS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServCounterAuthAns(
            (int *)&data_p->dnsServCounterAuthAns);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERV_COUNTER_AUTH_NO_DATA_RESPS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServCounterAuthNoDataResps(
            (int *)&data_p->dnsServCounterAuthNoDataResps);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERV_COUNTER_AUTH_NO_NAMES:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServCounterAuthNoNames(
            &data_p->dnsServCounterAuthNoNames);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERV_COUNTER_ENTRY:
        {
            DNS_OM_IPCMsg_ServCounterEntry_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServCounterEntry(
            (DNS_ServCounterEntry_T *)data_p);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ServCounterEntry_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERV_COUNTER_ERRORS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServCounterErrors(
            (int *)&data_p->dnsServCounterErrors);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERV_COUNTER_NON_AUTH_DATAS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServCounterNonAuthDatas(
            (int *)&data_p->dnsServCounterNonAuthDatas);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERV_COUNTER_NON_AUTH_NO_DATAS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServCounterNonAuthNoDatas(
            &data_p->dnsServCounterNonAuthNoDatas);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERV_COUNTER_OTHER_ERRORS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServCounterOtherErrors(
            &data_p->dnsServCounterOtherErrors);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERV_COUNTER_REFERRALS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServCounterReferrals(
            (int *)&data_p->dnsServCounterReferrals);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_SERV_COUNTER_RELNAMES:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServCounterRelNames(
            (int *)&data_p->dnsServCounterRelNames);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_NEXT_SERVER_LIST:
        {
            DNS_OM_IPCMsg_Name_Server_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetNextNameServerList(
            &data_p->name_server_ip);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_Name_Server_T);

            break;
        }

        case DNS_OM_IPC_CMD_GET_SERVER_COUNTER_REQ_REFUSALS:
        {
            DNS_OM_IPCMsg_ProxyCounter_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDnsServCounterReqRefusals(
            (int *)&data_p->dnsServCounterReqRefusals);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_ProxyCounter_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_DOMAIN_NAME_LIST_ENTRY:
        {
            DNS_OM_IPCMsg_IdxNameStr_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetDomainNameListEntry(
            data_p->index, data_p->name_str);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_IdxNameStr_T);
            break;
        }

        case DNS_OM_IPC_CMD_GET_NEXT_DOMAIN_NAME_LIST_ENTRY:
        {
            DNS_OM_IPCMsg_IdxNameStr_T *data_p = DNS_OM_MSG_DATA(ipcmsg_p);
            DNS_OM_MSG_RETVAL(ipcmsg_p) = DNS_OM_GetNextDomainNameListEntry(
            &data_p->index, data_p->name_str);
            ipcmsg_p->msg_size = DNS_OM_GET_MSGBUFSIZE(DNS_OM_IPCMsg_IdxNameStr_T);
            break;
        }

       default:
            SYSFUN_Debug_Printf("\r\n%s(): Invalid cmd.", __FUNCTION__);
            return FALSE;
    } /* switch ipcmsg_p->cmd */

    return TRUE;
} /* DNS_OM_HandleIPCReqMsg */

/* FUNCTION NAME : DNS_OM_ClearDatabase
 * PURPOSE: To clear database in DNS_OM.
 * INPUT  : none.
 * OUTPUT : none.
 * RETURN : none.
 * NOTES  : 1. for entering transition mode, etc...
 */
void DNS_OM_ClearDatabase(void)
{
    UI32_T          orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */

    DNS_OM_LocalFreeDomainNameList();

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
}

/* FUNCTION NAME : DNS_OM_CreateDomainNameListEntry
 * PURPOSE: To create a new dnsDomainListEntry in dnsDomainListEntry table.
 * INPUT  : idx   -- index of dnsDomainListEntry
 *                   (1-based, key to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 *          2. default name is 'domainname1', 'domainname2', ...
 */
BOOL_T DNS_OM_CreateDomainNameListEntry(UI32_T idx)
{
    UI32_T          orig_priority;
    char            name_buf[20] = ""; /* 10 (DomainName) + 9 (No.) + 1 ('\0') */
    int             ret = FALSE;

    if ((idx == 0) || (idx > MAX_NBR_OF_DOMAIN_NAME_LIST))
    {
        return ret;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */

    if (NULL == DNS_OM_LocalFindDomainNameListEntryByIndex(idx))
    {
        DNS_OM_LocalGetDefDomainName(name_buf);

        ret = DNS_OM_LocalAddDomainNameToListWithIndex(idx, name_buf);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);

    return ret;
}

/* FUNCTION NAME : DNS_OM_DestroyDomainNameListEntry
 * PURPOSE: To destroy adnsDomainListEntry in dnsDomainListEntry table.
 * INPUT  : idx   -- index of dnsDomainListEntry
 *                   (1-based, key to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_OM_DestroyDomainNameListEntry(UI32_T idx)
{
    UI32_T          orig_priority;
    DNS_IpDomain_T  *cur_p, *prv_p;
    BOOL_T          ret = FALSE;

    if ((1 <= idx) && (idx <= MAX_NBR_OF_DOMAIN_NAME_LIST))
    {
        orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */

        prv_p = NULL;
        cur_p = gdns_config.DnsIpDomainList_p;
        while (NULL != cur_p)
        {
            if (cur_p->idx == idx)
            {
                break;
            }

            prv_p = cur_p;
            cur_p = cur_p->next_p;
        }

        if (cur_p != NULL)
        {
            if (prv_p == NULL)
            {
                gdns_config.DnsIpDomainList_p = cur_p->next_p;
            }
            else
            {
                prv_p->next_p = cur_p->next_p;
            }
            L_MM_Free(cur_p);
            ret = TRUE;
        }
        else
        {
            /* old entry does not exist, return true for delete
             * bcz row_status is allowed to set destroy on unexist entry
             */
            ret = TRUE;
        }

        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    }

    return ret;
}

/* FUNCTION NAME : DNS_OM_SetDomainNameListEntry
 * PURPOSE: To modify a domain name in the dnsDomainListEntry table.
 * INPUT  : idx           -- index of entry.
 *                           (1-based, key to search the entry)
 *          domain_name_p -- domain name content.
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 *          2. domain_name_p[0] == '\0' is not valid
 */
BOOL_T DNS_OM_SetDomainNameListEntry(UI32_T idx, char *domain_name_p)
{
    DNS_IpDomain_T  *cur_p;
    UI32_T          orig_priority;
    int             ret = FALSE;

    if ((idx == 0) ||
        (idx > MAX_NBR_OF_DOMAIN_NAME_LIST) ||
        (NULL == domain_name_p) ||
        (domain_name_p[0] == '\0') ||
        (strlen(domain_name_p) > DNS_MAX_NAME_LENGTH))
    {
        return ret;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */

    cur_p = gdns_config.DnsIpDomainList_p;
    while (NULL != cur_p)
    {
        if (cur_p->idx == idx)
        {
            break;
        }

        cur_p = cur_p->next_p;
    }

    if (cur_p != NULL)
    {
        if (TRUE == DNS_OM_LocalIsDomainNameInList(domain_name_p))
        {
            /* same name can not be added twice,
             * only setting the same index with the same name is ok...
             */
            if (strcmp(cur_p->DnsIpDomainName, domain_name_p) == 0)
            {
                ret = TRUE;
            }
        }
        else
        {
            /* rename to new name
             */
            strcpy(cur_p->DnsIpDomainName, domain_name_p);
            ret = TRUE;
        }
    }
    else
    {
        /* old entry does not exist, return false
         */
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    return ret;
}

/* FUNCTION NAME : DNS_OM_GetDomainNameListEntry
 * PURPOSE: To get entry from the dnsDomainListEntry table.
 * INPUT  : idx           -- index of dnsDomainListEntry
 *                           (1-based, key to search the entry)
 * OUTPUT : domain_name_p -- pointer to domain name content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_OM_GetDomainNameListEntry(UI32_T idx, char *domain_name_p)
{
    DNS_IpDomain_T  *cur_p;
    UI32_T          orig_priority;
    BOOL_T          ret = FALSE;

    if (NULL == domain_name_p)
    {
        return FALSE;
    }

    if ((1 <= idx) && (idx <= MAX_NBR_OF_DOMAIN_NAME_LIST))
    {
        orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */

        /* 1st step, find the entry with exactly index
         */
        cur_p = gdns_config.DnsIpDomainList_p;
        while (NULL != cur_p)
        {
            if (cur_p->idx == idx)
            {
                break;
            }
            cur_p = cur_p->next_p;
        }

        /* 2nd step, return the data of the entry found
         */
        if (NULL != cur_p)
        {
            strcpy (domain_name_p, cur_p->DnsIpDomainName);
            ret = TRUE;
        }

        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    }

    return ret;
}

/* FUNCTION NAME : DNS_OM_GetNextDomainNameListEntry
 * PURPOSE: To get next entry from the dnsDomainListEntry table.
 * INPUT  : idx_p         -- index of dnsDomainListEntry
 *                           (1-based, 0 to get the first,
 *                            key to search the entry)
 * OUTPUT : idx_p         -- next index of dnsDomainListEntry
 *          domain_name_p -- pointer to domain name content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_OM_GetNextDomainNameListEntry(UI32_T *idx_p, char *domain_name_p)
{
    DNS_IpDomain_T  *cur_p;
    UI32_T          orig_priority, nxt_idx, used_bmp=0;
    BOOL_T          ret = FALSE;

    if ((NULL == domain_name_p) || (NULL == idx_p))
    {
        return FALSE;
    }

    nxt_idx = *idx_p +1;

    if (nxt_idx <= MAX_NBR_OF_DOMAIN_NAME_LIST)
    {
        orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_om_semaphore_id);  /* pgr0695, return value of statement block in macro */

        /* 1st step, find the entry with exactly next index
         */
        cur_p = gdns_config.DnsIpDomainList_p;
        while (NULL != cur_p)
        {
            if (cur_p->idx == nxt_idx)
            {
                break;
            }

            used_bmp |= 1 <<( cur_p->idx -1);
            cur_p = cur_p->next_p;
        }

        /* 2nd step, find the entry with the next available index
         *   if entry with (*idx +1) is not found
         */
        if (NULL == cur_p)
        {
            for ( ; nxt_idx <= MAX_NBR_OF_DOMAIN_NAME_LIST; nxt_idx++)
            {
                if (used_bmp & (1 << (nxt_idx -1)))
                {
                    break;
                }
            }

            if (nxt_idx <= MAX_NBR_OF_DOMAIN_NAME_LIST)
            {
                cur_p = gdns_config.DnsIpDomainList_p;
                while (NULL != cur_p)
                {
                    if (cur_p->idx == nxt_idx)
                    {
                        break;
                    }

                    cur_p = cur_p->next_p;
                }
            }
        }

        /* 3rd step, return the data of the entry found
         */
        if (NULL != cur_p)
        {
            *idx_p = cur_p->idx;
            strcpy (domain_name_p, cur_p->DnsIpDomainName);
            ret = TRUE;
        }

        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_om_semaphore_id, orig_priority);
    }

    return ret;
}

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME : DNS_OM_Name2Lower
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
static void DNS_OM_Name2Lower(const char* name,char* rname)
{
    const char* ptr = name;

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

/* FUNCTION NAME : DNS_OM_LocalGetDomainNameListEntrySmallestIndex
 * PURPOSE: To add a domain name with index to the dnsDomainListEntry table.
 * INPUT  : none.
 * OUTPUT : none.
 * RETURN :  0 -- failed
 *          >0 -- the smallest available index
 * NOTES  : none.
 */
static int DNS_OM_LocalGetDomainNameListEntrySmallestIndex(void)
{
    DNS_IpDomain_T  *cur_p;
    UI8_T           idx, used_bmp =0;
    int             ret = 0;

    cur_p = gdns_config.DnsIpDomainList_p;
    while (NULL != cur_p)
    {
        if (cur_p->idx <= MAX_NBR_OF_DOMAIN_NAME_LIST)
        {
            used_bmp |= 1 << (cur_p->idx -1);
        }

        cur_p = cur_p->next_p;
    }

    for (idx =0; idx <MAX_NBR_OF_DOMAIN_NAME_LIST; idx++)
    {
        if (!(used_bmp & (1 << idx)))
        {
            ret = idx +1;
            break;
        }
    }

    return ret;
}

/* FUNCTION NAME : DNS_OM_LocalIsDomainNameInList
 * PURPOSE: To check if the domain name is already in the list.
 * INPUT  : domain_name_p -- pointer to domain name content
 * OUTPUT : none.
 * RETURN : FALSE -- No,
 *          TRUE  -- Yes.
 * NOTES  : none.
 */
static BOOL_T DNS_OM_LocalIsDomainNameInList(char *domain_name_p)
{
    DNS_IpDomain_T  *cur_p;
    char            str_lower_case[DNS_MAX_NAME_LENGTH+1],
                    cur_lower_case[DNS_MAX_NAME_LENGTH+1];
    BOOL_T          ret = FALSE;

    DNS_OM_Name2Lower(domain_name_p, str_lower_case);

    /* 1st step, return error if this name is added before;
     *           try to move prv_p to the tail.
     */
    cur_p = gdns_config.DnsIpDomainList_p;
    while (NULL != cur_p)
    {
        DNS_OM_Name2Lower(cur_p->DnsIpDomainName, cur_lower_case);
        if (0 == strcmp(str_lower_case, cur_lower_case))
        {
            ret = TRUE;
            break;
        }
        cur_p = cur_p->next_p;
    }

    return ret;
}

/* FUNCTION NAME : DNS_OM_LocalAddDomainNameToListWithIndex
 * PURPOSE: To add a domain name with index to the dnsDomainListEntry table.
 * INPUT  : idx           -- index of dnsDomainListEntry
 *          domain_name_p -- pointer to domain name content
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. will use the smallest available index in dnsDomainListEntry
 *             if idx == 0
 */
static BOOL_T DNS_OM_LocalAddDomainNameToListWithIndex(UI32_T idx, char *domain_name_p)
{
    DNS_IpDomain_T *cur_p, *prv_p;
    BOOL_T          ret = FALSE;

    if ((NULL == domain_name_p) ||(strlen(domain_name_p)>DNS_MAX_NAME_LENGTH) ||
        (TRUE == DNS_OM_LocalIsDomainNameInList(domain_name_p)))
    {
        return FALSE;
    }

    if (idx == 0)
    {
        idx = DNS_OM_LocalGetDomainNameListEntrySmallestIndex();
    }

    if ((idx != 0) && (idx <= MAX_NBR_OF_DOMAIN_NAME_LIST))
    {
        cur_p = DNS_OM_IP_DOMAIN_LIST_ENTRY_MALLOC();
        if (NULL != cur_p)
        {
            strcpy(cur_p->DnsIpDomainName, domain_name_p);

            cur_p->idx = idx;
            cur_p->next_p = NULL;

            /* add new entry to tail
             */
            prv_p = gdns_config.DnsIpDomainList_p;
            while (NULL != prv_p)
            {
                if (prv_p->next_p == NULL)
                {
                    break;
                }

                prv_p = prv_p->next_p;
            }

            if (NULL == prv_p)
            {
                gdns_config.DnsIpDomainList_p = cur_p;
            }
            else
            {
                prv_p->next_p = cur_p;
            }

            ret = TRUE;
        }
    }

    return ret;;
}

/* FUNCTION NAME : DNS_OM_LocalFindDomainNameListEntryByIndex
 * PURPOSE: To find entries with specified index in the dnsDomainListEntry table.
 * INPUT  : none.
 * OUTPUT : none.
 * RETURN : NULL       -- failed.
 *          Non-NULL   -- pointer to the entry with specified index
 * NOTES  : none.
 */
static DNS_IpDomain_T *DNS_OM_LocalFindDomainNameListEntryByIndex(UI32_T idx)
{
    DNS_IpDomain_T  *cur_p;

    cur_p = gdns_config.DnsIpDomainList_p;
    while (NULL != cur_p)
    {
        if (cur_p->idx == idx)
        {
            break;
        }
        cur_p = cur_p->next_p;
    }

    return cur_p;
}

/* FUNCTION NAME : DNS_OM_LocalFreeDomainNameList
 * PURPOSE: To free entries in the dnsDomainListEntry table.
 * INPUT  : none.
 * OUTPUT : none.
 * RETURN : none.
 * NOTES  : 1. for entering transition mode, etc...
 */
static void DNS_OM_LocalFreeDomainNameList(void)
{
    DNS_IpDomain_T  *cur_p, *tmp_p;

    cur_p = gdns_config.DnsIpDomainList_p;
    while (NULL != cur_p)
    {
        tmp_p = cur_p;
        cur_p = cur_p->next_p;

        L_MM_Free(tmp_p);
    }
    gdns_config.DnsIpDomainList_p = cur_p;
}

/* FUNCTION NAME : DNS_OM_LocalGetDefDomainName
 * PURPOSE: To get an available default domain name for the new entry.
 * INPUT  : none.
 * OUTPUT : domain_name_p -- pointer to the domain name content.
 * RETURN : none.
 * NOTES  : none.
 */
static void DNS_OM_LocalGetDefDomainName(char *domain_name_p)
{
    UI32_T  used_idx =1;

    if (NULL != domain_name_p)
    {
        /* Use default name: 'DomainName1' ,'DomainName2', 'DomainName3', ...
         */
        do
        {
            sprintf(domain_name_p, DNS_TYPE_DEFAULT_DOMAIN_NAME_FORMAT, (long)used_idx++);
        } while (TRUE == DNS_OM_LocalIsDomainNameInList(domain_name_p));
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_OM_LocalGetNslookupCtlEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get nslookup control entry by index.
 * INPUT    : ctl_index  -- 0-based nslookup control entry index
 * OUTPUT   : None
 * RETUEN   : nslookup control entry
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static DNS_Nslookup_CTRL_T *
DNS_OM_LocalGetNslookupCtlEntryByIndex(
    UI32_T ctl_index)
{
    if (ctl_index < _countof(nslookup_ctrl_table))
    {
        return &nslookup_ctrl_table[ctl_index];
    }

    return NULL;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_OM_LocalGetNslookupResultEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get nslookup result entry by index.
 * INPUT    : ctl_index       -- 0-based nslookup control entry index
 *            result_index    -- 0-based nslookup result entry index
 *            result_entry_p  -- nslookup result entry
 * OUTPUT   : None
 * RETUEN   : nslookup result entry
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static DNS_Nslookup_Result_T *
DNS_OM_LocalGetNslookupResultEntryByIndex(
    UI32_T ctl_index,
    UI32_T result_index)
{
    if ((ctl_index < _countof(nslookup_ctrl_table)) &&
        (result_index < _countof(nslookup_result_table[0])))
    {
        return &nslookup_result_table[ctl_index][result_index];
    }

    return NULL;
}

#endif /* #if (SYS_CPNT_DNS == TRUE) */

