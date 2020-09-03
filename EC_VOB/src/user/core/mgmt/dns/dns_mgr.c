/* MODULE NAME: dns_mgr.c
 * PURPOSE:
 *   Initialize the resource and provide some functions for the dns module.
 *   Functions provide service for snmp module and CLI is included in this file.
 *
 * NOTES:
 *
 * History:
 *       Date          -- Modifier,     Reason
 *       2002-09-06   -- Simon zhou  Created
 *       2002-10-23    -- Wiseway ,   modified for convention.
 *       2002-11-14    -- isiah,  porting to ACP@2.0
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

#include "l_stdlib.h"
#include "l_inet.h"
#include "backdoor_mgr.h"
#include "l_hoststring2ip.h"
#include "ip_lib.h"

#include "sys_module.h"
#include "l_mm.h"
#include "l_rstatus.h"
#include "sys_time.h"

#include "dns_cmm.h"
#include "dns.h"
#include "dns_type.h"
#include "dns_om.h"
#include "dns_mgr.h"
#include "dns_cache.h"
#include "dns_hostlib.h"
#include "dns_resolver.h"
#include "sysfun.h"

struct hostent* gethostbyname(const I8_T* name, UI32_T *rc);
#if 0
static BOOL_T DNS_RESOLVER_ExtractIp(UI32_T* ip, I8_T** ip_addr_list);
#endif

extern I32_T dns_inet_addr(register const char *cp);

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define  SYSFUN_USE_CSC(ret)
#define SYSFUN_RELEASE_CSC()

#define DNS_MGR_ISALNUM(c)  ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))

#define DNS_MGR_IS_SYSTEM_OWNER_INDEX(CtlOwnerIndex)    \
    ( (0 == strcmp((char *)CtlOwnerIndex, DNS_TYPE_NSLOOKUP_CTL_OWNER_INDEX_SYSTEM_CLI)) ||  \
      (0 == strcmp((char *)CtlOwnerIndex, DNS_TYPE_NSLOOKUP_CTL_OWNER_INDEX_SYSTEM_WEB)) )

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T DNS_MGR_CheckHostName(char *host);

static BOOL_T
DNS_MGR_IsNslookupCtlEntryAgeOut(
    DNS_Nslookup_CTRL_T *ctl_entry_p
);

static BOOL_T
DNS_MGR_IsNslookupResultEntryAgeOut(
    DNS_Nslookup_Result_T *result_entry_p
);

static BOOL_T
DNS_MGR_IsNslookupCtlEntryExist(
    DNS_Nslookup_CTRL_T *ctl_entry_p,
    UI32_T *ctl_index_p
);

static int
DNS_MGR_DeleteAllSystemAgeOutNslookupCtlEntry(void);

static int
DNS_MGR_CreateNslookupCtlEntry(
    DNS_Nslookup_CTRL_T *ctl_entry_p,
    UI32_T *ctl_index_p
);

static int
DNS_MGR_AsyncStartNslookup(
    UI32_T ctl_index
);

static void
DNS_MGR_AsnycNslookupTaskMain(
    UI32_T *ctl_index_p
);

/* STATIC VARIABLE DECLARATIONS
 */
SYSFUN_DECLARE_CSC


/* EXPORTED SUBPROGRAM BODIES
 */

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
BOOL_T DNS_MGR_Init(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    if ( DNS_OM_Init() == FALSE )
    {
        return FALSE;
    }
    return TRUE;
}

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
void DNS_MGR_ResetConfig()
{
    DNS_OM_ResetConfig();
}


/*--------------------------------------------------------------------------
 * FUNCTION NAME - DNS_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void DNS_MGR_Create_InterCSC_Relation(void)
{
    return;
} /* end of DNS_MGR_Create_InterCSC_Relation */

/*the following is from liuheming */


/* FUNCTION NAME : DNS_MGR_GetDnsServConfigImplementIdent
 * PURPOSE:
 *  This function gets the implementation identification string for the DNS server software in use on the system
 *
 *
 * INPUT:
 *  I8_T * -- a pointer to a string to storing  the implementation
 *      identification string for the DNS server software
 *      in use on the system.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *
 */
int DNS_MGR_GetDnsServConfigImplementIdent(I8_T *dns_serv_config_implement_ident_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret=DNS_OM_GetDnsServConfigImplementIdent(dns_serv_config_implement_ident_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_GR_SetDnsServConfigRecurs
 * PURPOSE:
 *  This function sets the recursion services offered by this name server.
 *
 *
 * INPUT:
 *  none.
 * OUTPUT:
 *  int * -- A pointer to a variable to store the value to be set.
 *    This represents the recursion services offered by this
 *    name server.  The values that can be read or written are:
 *    available(1) - performs recursion on requests from clients.
 *    restricted(2) - recursion is performed on requests only
 *    from certain clients, for example; clients on an access
 *    control list.  It is not supported currently.
 *    unavailable(3) - recursion is not available.
 *
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *
 */
int DNS_MGR_SetDnsServConfigRecurs(int *config_recurs_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_SetDnsServConfigRecurs(config_recurs_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_GetDnsServConfigRecurs
 * PURPOSE:
 *  This function gets the recursion services offered by this name server.
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *  int * -- A pointer to a variable to store the returned value.
 *    This represents the recursion services offered by this
 *    name server.  The values that can be read or written are:
 *    available(1) - performs recursion on requests from clients.
 *    restricted(2) - recursion is performed on requests only
 *    from certain clients, for example; clients on an access
 *    control list.  It is not supported currently.
 *    unavailable(3) - recursion is not available.
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *      This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServConfigRecurs(int *config_recurs_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret=DNS_OM_GetDnsServConfigRecurs(config_recurs_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_GetDnsServConfigUpTime
 * PURPOSE:
 *  This function get the up time since the server started.
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *      UI32_T* -- a pointer to a variable to store the returned
 *        value about the up time since the server started
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *      This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServConfigUpTime(UI32_T *dns_serv_config_up_time_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret=DNS_OM_GetDnsServConfigUpTime(dns_serv_config_up_time_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetDnsServConfigResetTime
 * PURPOSE:
 *  This function gets the time elapsed since the last time the name server was `reset.'
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  UI32_T* -- a pointer to a variable to stored the returned
 *       value about the time elapsed since the last time
 *       the name server was reset.
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *      This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServConfigResetTime(UI32_T    *dns_serv_config_reset_time)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsServConfigResetTime(dns_serv_config_reset_time);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_SetDnsServConfigReset
 * PURPOSE:
 *  This function reinitialize any persistant name server state.
 *
 *
 * INPUT:
 *  int * -- When set to reset(2), any persistant name server state
 *    (such as a process) is reinitialized as if the name
 *    server had just been started
 *
 * OUTPUT:
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *      This function will be called by snmp module.
 */
int DNS_MGR_SetDnsServConfigReset(int *dns_serv_config_reset_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_SetDnsServConfigReset(dns_serv_config_reset_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_OM_GetDnsServConfigReset
 * PURPOSE:
 *  This funtion gets any persistant name server state.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to store the result
 *    other(1) - server in some unknown state;
 *    initializing(3) - server (re)initializing;
 *    running(4) - server currently running
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *      This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServConfigReset(int *dns_serv_config_reset_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsServConfigReset(dns_serv_config_reset_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_OM_SetDnsServConfigMaxRequests
 * PURPOSE:
 *  This fcuntion set the max number of requests in proxy.
 *
 *
 * INPUT:
 *  int * -- a pointer to a variable for storing the number of
 *    max requests in proxy. range:1-20. default:10
 *
 * OUTPUT:
 *
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *
 */
int DNS_MGR_SetDnsServConfigMaxRequests(int *dns_serv_config_max_requests_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_SetDnsServConfigMaxRequests(dns_serv_config_max_requests_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME :DNS_MGR_GetDnsServConfigMaxRequests
 * PURPOSE:
 *  This fcuntion gets the max number of requests in proxy.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *  int * -- a pointer to a variable for storing the returned value
 *
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *
 */
int DNS_MGR_GetDnsServConfigMaxRequests(int *dns_serv_config_max_requests_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsServConfigMaxRequests(dns_serv_config_max_requests_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_SetDnsResCacheStatus
 * PURPOSE:
 *  This function is used for initializing dns cache
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *      int * -- 1 enable cache,
 *              2 enable cache,
 *              3 clear the contents in the cache
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *      This function will be called by DNS_CACHE_Init and snmp module.
 *
 */
int DNS_MGR_SetDnsResCacheStatus (int *dns_res_cache_status_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        if(dns_res_cache_status_p == NULL)
        {
            SYSFUN_RELEASE_CSC();
            return ret;
        }
        if ((*dns_res_cache_status_p<VAL_dnsResCacheStatus_enabled)||(*dns_res_cache_status_p>VAL_dnsResCacheStatus_clear))
        {
            SYSFUN_RELEASE_CSC();
            return ret;     /*DNS_ERROR*/
        }

/*isiah.2003-10-30*/
        if (*dns_res_cache_status_p==VAL_dnsResCacheStatus_clear)
        {
            DNS_CACHE_Reset();
            ret = DNS_OK;
        }
        else
        {
        ret = DNS_OM_SetDnsResCacheStatus(dns_res_cache_status_p);
        }

/*isiah.2003-10-30*/
#if 0
        if (*dns_res_cache_status_p==VAL_dnsResCacheStatus_clear)
        {
            DNS_CACHE_Reset();
        }
#endif
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_OM_GetDnsResCacheStatus
 * PURPOSE:
 *  This function is used for getting the cache status.
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to store the returned cache status
 *    enabled(1),
 *    disabled(2)
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResCacheStatus(int *dns_res_cache_status_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResCacheStatus(dns_res_cache_status_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_OM_SetDnsResCacheMaxTTL
 * PURPOSE:
 *  This function is used for setting Maximum Time-To-Live for RRs in this cache.
 *  If the resolver does not implement a TTL ceiling, the value of this field should be zero.
 *
 * INPUT:
 *  int * -- a pointer to a variable for storing Maximum Time-To-Live for RRs in the cache
 *
 * OUTPUT:
 *   none.
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_SetDnsResCacheMaxTTL(int *dns_res_cache_max_ttl_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_SetDnsResCacheMaxTTL(dns_res_cache_max_ttl_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_OM_GetDnsResCacheMaxTTL
 * PURPOSE:
 *  This fuction will be called by snmp module.
 *  If the resolver does not implement a TTL ceiling, the value of this field should be zero.
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  UI32_T * -- a pointer to a variable for storing the returned Maximum Time-To-Live for RRs in the cache
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResCacheMaxTTL(UI32_T *dns_res_cache_max_ttl_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResCacheMaxTTL(dns_res_cache_max_ttl_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_OM_GetDnsResCacheGoodCaches
 * PURPOSE:
 *  This function is used for getting the number of rrs the resolver has cached successfully.
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  int * -- a pointer to a variable for storing the number of rrs the resolver has cache successfully
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResCacheGoodCaches(int *dns_res_good_caches_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResCacheGoodCaches(dns_res_good_caches_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_OM_GetDnsResCacheBadCaches
 * PURPOSE:
 *  This function is used for getting the number of RRs the resolver has refused to cache.
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to storing the returned number of rrs the resolver has refused to cache
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResCacheBadCaches(int *dns_res_cache_bad_caches_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResCacheBadCaches(dns_res_cache_bad_caches_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_OM_SetDnsResCacheMaxEntries
 * PURPOSE:
 *  This fcuntion set the max number of entries in cache.
 *
 *
 * INPUT:
 *  int * -- a pointer to a variable for storing the number of
 *    max entries in cache. range:1280-6400. default:2560
 *
 * OUTPUT:
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_SetDnsResCacheMaxEntries(int *dns_res_cache_max_entries_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_SetDnsResCacheMaxEntries(dns_res_cache_max_entries_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetDnsResCacheMaxEntries
 * PURPOSE:
 *  This fcuntion get the max number of entries in cache.
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *  int * -- a pointer to a variable for storing the returned value
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResCacheMaxEntries(int *dns_res_cache_max_entries_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResCacheMaxEntries(dns_res_cache_max_entries_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/*the above is from liuheming */

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
BOOL_T DNS_MGR_EnableDomainLookup(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
/*isiah.2003-08-15*/
#if 0
        if ( NULL==DNS_MGR_GetDnsSbelt() )
        {
            printf("You must assign Domain Name Server first\r\n");
            SYSFUN_RELEASE_CSC();
            return ret;
        }
#endif
        DNS_OM_EnableDomainLookup();
        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

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
BOOL_T DNS_MGR_DisableDomainLookup(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        DNS_OM_DisableDomainLookup();
        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_AddNameServer
 * PURPOSE:
 *  This function adds a name server IP address to the name server list.
 *
 *
 * INPUT:
 *  L_INET_AddrIp_T *addr_p -- a pointer to a ip addr will be added as a name server;
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *      This function will be called by CLI command "ip name-servert".
 */
int DNS_MGR_AddNameServer(L_INET_AddrIp_T *addr_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return ret;
    }
    else
    {
        if(DNS_MGR_CheckNameServerIp(addr_p) == TRUE)
        {
            ret = DNS_OM_AddNameServer(addr_p);

            if (ret==DNS_OK)             /*modified by wiseway for semaphore bug 2002-10-27*/
            {
                DNS_RESOLVER_SbeltInit();
            }
        }

    }

    return ret;
}

/* FUNCTION NAME : DNS_MGR_DeleteNameServer
 * PURPOSE:
 *  This function deletes a name server entry from
 *  the name server list accordint to the IP address of the name server.
 *
 * INPUT:
 *  I8_T * -- a pointer to a ip addr , whose related name server will be deleted;
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *      This function will be called by CLI command "no ip name-servert".
 */
int DNS_MGR_DeleteNameServer(L_INET_AddrIp_T *addr_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_DeleteNameServer(addr_p);
        if (ret==DNS_OK)             /*modified by wiseway for semaphore bug 2002-10-27*/
        {
                DNS_RESOLVER_SbeltInit();
        }
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_ShowNameServerList
 * PURPOSE:
 *  This fcuntion displays the nam server informaion.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  none.
 *
 * NOTES:
 *
 */
BOOL_T DNS_MGR_ShowNameServerList(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        DNS_OM_ShowNameServerList();
        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_DebugOpen
 * PURPOSE:
 *  This fcuntion enables the displaying of debugging information.
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
 *
 */
BOOL_T DNS_MGR_DebugOpen(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        DNS_OM_DebugOpen();
        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_OM_DebugClose
 * PURPOSE:
 *  This fcuntion disables the displaying of debugging information.
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
 *
 */
BOOL_T DNS_MGR_DebugClose(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        DNS_OM_DebugClose();
        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

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
 *  DNS_OK :success
 * NOTES:
 *      This function will be called by configuration sub moudle.
 *      This function will be called by CLI command "clear dns cache"
 */
int DNS_MGR_ClearDnsCache(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        /*isiah*/
        ret = DNS_CACHE_Reset();
        /*DNS_OM_ClearDnsCache();*/
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



/*
 * FUNCTION NAME : DNS_MGR_ShowDnsCache
 *
 * PURPOSE:
 *      This function is used for showing  cache database and status.
 *       Every field except link should be displayed, and the index of this cache
 *      entry should also be displayed.
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
BOOL_T  DNS_MGR_ShowDnsCache(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;
    I32_T   index = -1;
    DNS_CacheRecord_T   cache_entry;
    char    ip[L_INET_MAX_IPADDR_STR_LEN+1];

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
#if 0
        DNS_CACHE_ShowDatabase();
#else

        while ( DNS_MGR_GetNextCacheEntry(&index, &cache_entry) == TRUE )
        {
            if(DNS_RRT_A == cache_entry.type || DNS_RRT_AAAA == cache_entry.type)
            {
                L_INET_InaddrToString((L_INET_Addr_T*)&cache_entry.ip, ip, sizeof(ip));
            }
            else
        {
                UI32_T idx;
                memcpy(&idx, cache_entry.ip.addr, 4);
                printf("POINTER TO:%-4ld\t", (long)idx);
            }

            printf("%-4lu\t", (unsigned long)cache_entry.ttl);
            printf("%-s\n", cache_entry.name);
        }
#endif
        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



/* FUNCTION NAME : DNS_MGR_ShowDnsDatabase
 * PURPOSE:
 *  This funciton is used for showing  cache database and status.
 *  Every field except link should be displayed, and the index of this cache
 *  entry should also be displayed.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *  DNS_OK :success
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
BOOL_T  DNS_MGR_ShowDnsDatabase(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        DNS_CACHE_ShowDatabase();
        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_SetDnsLocalMaxRequests
 * PURPOSE:
 *  This funciton set the max number of local requests that resolver
 *  can deal with.
 *
 *
 * INPUT:
 *  int * -- a pointer to a variable for storing the number of
 *    local max requests. range:1..10. default:5
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *  DNS_OK :success
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
int DNS_MGR_SetDnsLocalMaxRequests(int *dns_config_local_max_requests_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_SetDnsLocalMaxRequests(dns_config_local_max_requests_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_GetDnsLocalMaxRequests
 * PURPOSE:
 *  This funciton set the max number of local requests that resolver
 *  can deal with.
 *
 *
 * INPUT:
 *      int * -- a pointer to a variable for storing the returned value
 *              range 1:10; default5:
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *  DNS_OK :success
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
int  DNS_MGR_GetDnsLocalMaxRequests(int *dns_config_local_max_requests_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsLocalMaxRequests(dns_config_local_max_requests_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/*the following APIs is used for Resolver Mibs                         */

/* FUNCTION NAME : DNS_MGR_GetDnsResConfigImplementIdent
 * PURPOSE:
 *  The implementation identification string for the
 *  resolver software in use on the system.
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  I8_T * -- a pointer to the variable to store the returned value
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResConfigImplementIdent(I8_T *dns_res_config_implement_ident_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResConfigImplementIdent(dns_res_config_implement_ident_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_OM_GetDnsResConfigService
 * PURPOSE:
 *  Get kind of DNS resolution service provided .
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to store the result
 *    recursiveOnly(1) indicates a stub resolver.
 *    iterativeOnly(2) indicates a normal full service resolver.
 *    recursiveAndIterative(3) indicates a full-service
 *    resolver which performs a mix of recursive and iterative queries.
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResConfigService(int *dns_res_config_service_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResConfigService(dns_res_config_service_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



/* FUNCTION NAME : DNS_MGR_GetDnsResConfigMaxCnames
 * PURPOSE:
 *  Limit on how many CNAMEs the resolver should allow
 *  before deciding that there's a CNAME loop.  Zero means
 *  that resolver has no explicit CNAME limit.
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResConfigMaxCnames(int *dns_resconfig_max_cnames_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResConfigMaxCnames(dns_resconfig_max_cnames_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_SetDnsResConfigSbeltEntry
 * PURPOSE:
 *  This funciton get the specified DnsResConfigSbeltEntry according to the index.
 *
 *
 *
 * INPUT:
 *  DNS_ResConfigSbeltEntry_T * -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the value to be set
 *          INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *  DNS_ResConfigSbeltEntry_T * --
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK :success
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
int DNS_MGR_SetDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;
    L_INET_AddrIp_T sbelt_ipaddr;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return ret;
    }
    else
    {
        memcpy(&sbelt_ipaddr, &dns_res_config_sbelt_entry_t_p->dnsResConfigSbeltAddr, sizeof(L_INET_AddrIp_T));
        if(DNS_MGR_CheckNameServerIp(&sbelt_ipaddr) == TRUE)
        {
            ret = DNS_OM_SetDnsResConfigSbeltEntry(dns_res_config_sbelt_entry_t_p);
            if (ret==DNS_OK)             /*modified by wiseway for semaphore bug 2002-10-27*/
            {
                DNS_RESOLVER_SbeltInit();
            }
     }
    }

    return ret;
}


/* FUNCTION NAME : DNS_MGR_GetDnsResConfigSbeltEntry
 * PURPOSE:
 *  This funciton get the specified DnsResConfigSbeltEntry according to the index.
 *
 *
 * INPUT:
 *  DNS_ResConfigSbeltEntry_T* -- INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
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
int DNS_MGR_GetDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResConfigSbeltEntry(dns_res_config_sbelt_entry_t_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetNextDnsResConfigSbeltEntry
 * PURPOSE:
 *  This function gets the DNS_ResConfigSbeltEntry_T  next to the specified index.
 *
 *
 * INPUT:
 *  DNS_ResConfigSbeltEntry_T * -- INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *  DNS_ResConfigSbeltEntry_T * -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the returned value
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetNextDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetNextDnsResConfigSbeltEntry(dns_res_config_sbelt_entry_t_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_GetDnsResConfigUpTime
 * PURPOSE:
 *  If the resolver has a persistent state (e.g., a
 *  process), this value will be the time elapsed since it
 *  started.  For software without persistant state, this
 *  value will be 0.
 * INPUT:
 *
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to storing config up time
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResConfigUpTime(int *config_up_time_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResConfigUpTime(config_up_time_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetDnsResConfigResetTime
 * PURPOSE:
 *  This function gets the time elapsed since it started.
 *
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to storing the returned config reset time
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResConfigResetTime(int *config_reset_time_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResConfigResetTime(config_reset_time_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_SetDnsResConfigReset
 * PURPOSE:
 *  This function reinitialize any persistant resolver state.
 *
 *
 *
 * INPUT:
 *  int * -- a pointer to a variable stored with the reset value,2 means reset
 *
 * OUTPUT:
 *   none.
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_SetDnsResConfigReset(int *config_reset_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_SetDnsResConfigReset(config_reset_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_SetDnsResConfigQueryOrder
 * PURPOSE:
 *  This function  set the query order.
 *
 *
 *
 * INPUT:
 *  int * -- a pointer to a variable storing the value to be set
 *    DNS_QUERY_LOCAL_FIRST:Query static host table first;
 *    DNS_QUERY_DNS_FIRST: Query DNS server first
 *    default: DNS_QUERY_LOCAL_FIRST
 *    0 means uses default configuration
 * OUTPUT:
 *  none.
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_SetDnsResConfigQueryOrder(int *dns_res_config_query_order_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_SetDnsResConfigQueryOrder(dns_res_config_query_order_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_GetDnsResConfigQueryOrder
 * PURPOSE:
 *  This function  get the query order.
 *
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to store the returned value about query order
 *    DNS_QUERY_LOCAL_FIRST:Query static host table first;
 *    DNS_QUERY_DNS_FIRST: Query DNS server first
 *    DNS_QUERY_ DNS_ONLY: Query DNS server only
 *    default: DNS_QUERY_LOCAL_FIRST
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResConfigQueryOrder(int *dns_res_config_query_order_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResConfigQueryOrder(dns_res_config_query_order_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetDnsResConfigReset
 * PURPOSE:
 *  This function gets any persistant resolver state.
 *
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  int * -- a pointer to a variable for storing the returned resolver state
 *    other(1) - resolver in some unknown state;
 *    initializing(3) - resolver (re)initializing;
 *    running(4) - resolver currently running.
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResConfigReset(int *config_reset_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResConfigReset(config_reset_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



/* FUNCTION NAME : DNS_OM_GetDnsResCounterNonAuthDataResps
 * PURPOSE:
 *  This functin gets the number of requests made by the resolver for which a
 *  non-authoritative answer (cached data) was received.
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResCounterNonAuthDataResps(int *dns_res_counter_non_auth_data_resps_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResCounterNonAuthDataResps(dns_res_counter_non_auth_data_resps_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetDnsResCounterNonAuthNoDataResps
 * PURPOSE:
 *  This fucniton gets Number of requests made by the resolver for which a
 *  non-authoritative answer - no such data response (empty answer) was received
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResCounterNonAuthNoDataResps(int *dns_res_counter_non_auth_no_data_resps_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResCounterNonAuthNoDataResps(dns_res_counter_non_auth_no_data_resps_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_GetDnsResCounterMartians
 * PURPOSE:
 *  This function gets the number of responses received which were received from
 *  servers that the resolver does not think it asked.
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResCounterMartians(int *dns_res_counter_martians_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResCounterMartians((UI32_T *)dns_res_counter_martians_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_GetDnsResCounterRecdResponses
 * PURPOSE:
 *  This function gets Number of responses received to all queries.
 *
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResCounterRecdResponses(int *dns_res_counter_recd_responses_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResCounterRecdResponses((UI32_T *)dns_res_counter_recd_responses_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_GetDnsResCounterUnparseResps
 * PURPOSE:
 *  This function gets Number of responses received which were unparseable.
 *
 *
 *
 * INPUT:
 *  int * -- a pointer to a variable to store the result
 *
 * OUTPUT:
 *  none.
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResCounterUnparseResps(int *dns_res_counter_unparse_resps_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResCounterUnparseResps((UI32_T *)dns_res_counter_unparse_resps_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_GetDnsResCounterFallbacks
 * PURPOSE:
 *  This function gets the number of times the resolver had to fall back to its seat belt information.
 *
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_MGR_GetDnsResCounterFallbacks(int *dns_res_counter_fallbacks_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResCounterFallbacks(dns_res_counter_fallbacks_p);
        }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/*
 * FUNCTION NAME : DNS_MGR_GetDnsResOptCounterReferals
 *
 * PURPOSE:
 *  This function gets the number of responses which were
 *  eived from servers redirecting query to another server.
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsResOptCounterReferals(int *dns_res_opt_counter_referals_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResOptCounterReferals(dns_res_opt_counter_referals_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsResOptCounterRetrans
 *
 * PURPOSE:
 *  This function gets the number requests retransmitted for all reasons
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsResOptCounterRetrans(int *dns_res_opt_counter_retrans_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsResOptCounterRetrans(dns_res_opt_counter_retrans_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsResOptCounterNoResponses
 *
 * PURPOSE:
 *  This function gets the number of queries that were retransmitted because of no response
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsResOptCounterNoResponses(int *dns_res_opt_counter_no_responses_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResOptCounterNoResponses((UI32_T *)dns_res_opt_counter_no_responses_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsResOptCounterRootRetrans
 *
 * PURPOSE:
 *  This function gets the number of queries that were retransmitted that were to root servers.
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsResOptCounterRootRetrans(int *dns_res_opt_counter_root_retrans_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsResOptCounterRootRetrans(dns_res_opt_counter_root_retrans_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsResOptCounterInternals
 *
 * PURPOSE:
 *  This function gets the number of requests internally generated by the resolver.
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsResOptCounterInternals(int *dns_res_opt_counter_internals)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResOptCounterInternals(dns_res_opt_counter_internals);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsResOptCounterInternalTimeOuts
 *
 * PURPOSE:
 *  This function gets the number of requests internally generated which timed out.
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsResOptCounterInternalTimeOuts(int *dns_res_opt_counter_internal_time_outs_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResOptCounterInternalTimeOuts(dns_res_opt_counter_internal_time_outs_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

#if 0
/* FUNCTION NAME : DNS_MGR_HostTblInit
 *
 * PURPOSE:
 *      Initialize th local host table. Set hostList with two default host entries.
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      none
 */
void DNS_MGR_HostTblInit(void)
{

    DNS_HOSTLIB_HostTblInit();

}
#endif

/* FUNCTION NAME : DNS_MGR_HostGetByName
 *
 * PURPOSE:
 *      This routine returns a list of the Internet address of a host that has
 *      been added to the host table by DNS_MGR_HostAdd(), and store it in the
 *      addr[].
 *
 * INPUT:
 *      const I8_T *   -- host name or alias.
 *
 * OUTPUT:
 *      struct in_addr --Ip addr array where the searched IP addrs will be put.
 *
 * RETURN:
 *      int
 *
 * NOTES:
 *      none
 */
int DNS_MGR_HostGetByName(const char * name, L_INET_AddrIp_T addr_ar[])
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_HOSTLIB_HostGetByName(name,addr_ar);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_HostGetByAddr
 *
 * PURPOSE:
 *      This routine finds the host name by its Internet address and copies it to
 *      <name>.  The buffer <name> should be preallocated with (MAXHOSTNAMELEN + 1)
 *      bytes of memory and is NULL-terminated unless insufficient space is
 *      provided.
 *      This routine does not look for aliases.  Host names are limited to
 *      MAXHOSTNAMELEN (from hostLib.h) characters.
 *
 * INPUT:
 *      const I8_T * addr -- inet address of host.
 *
 * OUTPUT:
 *      I8_T * name -- buffer to hold name..
 *
 * RETURN:
 *      DNS_OK, or DNS_ERROR if buffer is invalid or the host is unknown
 *
 * NOTES:
 *       none
 */
int DNS_MGR_HostGetByAddr(const I8_T * addr,I8_T * name)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_HOSTLIB_HostGetByAddr(addr,name);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_HostAdd
 * PURPOSE:
 *  add a host to the host table
 *
 *
 *
 * INPUT:
 *  I8_T * -- host name
 *  L_INET_AddrIp_T hostAddr* --  host addr in standard Internet format
 * OUTPUT:
 *  none.
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *      This function will be called by CLI command "ip host"
 */
int DNS_MGR_HostAdd(char *hostName, L_INET_AddrIp_T *hostaddr_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        if (DNS_MGR_CheckHostName(hostName) != TRUE
            || DNS_MGR_CheckNameServerIp(hostaddr_p) != TRUE)
        {
            SYSFUN_RELEASE_CSC();
            return DNS_ERROR;
        }

            ret = DNS_HOSTLIB_HostAdd(hostName, hostaddr_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_HostShow
 * PURPOSE:
 *  This routine prints a list of remote hosts, along with their Internet addresses and aliases
 *
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  none.
 *
 * RETURN:
 *  none.
 *
 * NOTES:
 *      .
 */
BOOL_T DNS_MGR_HostShow(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;
    int host_index;
    HostEntry_PTR hostentry_p = NULL;
    int i, j;
    char ip[L_INET_MAX_IPADDR_STR_LEN+1];


    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
    host_index = -1;
    hostentry_p = (HostEntry_T *)L_MM_Malloc(sizeof(HostEntry_T), L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_MGR_HOSTSHOW));

    if (hostentry_p==NULL)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }

    printf("hostname\t\tinet address   \t\talias   \n");
    printf("--------\t\t------------   \t\t-----   \n\n");

    while( DNS_OK == DNS_MGR_GetNextDnsHostEntry(&host_index, hostentry_p) )
    {
        j = 1;
        printf("%-8s\t\t", hostentry_p->hostName[0].name);
        for(i = 0; i < MAXHOSTIPNUM; i++)
        {
            if(hostentry_p->netAddr[i].addrlen != 0)
            {
                L_INET_InaddrToString((L_INET_Addr_T*)&hostentry_p->netAddr[i], ip, sizeof(ip));
                printf("%-54s\t\t", ip );

                for( ; j < MAXHOSTNAMENUM; j++)
                {
                    if(hostentry_p->hostName[j].name[0]!='\0')
                    {
                        printf("%s\n        \t\t", hostentry_p->hostName[j].name);
                        j++;
                        break;
                    }
                }
                if(j == MAXHOSTNAMENUM)     /*MAXHOSTNAMENUM - 1 to MAXHOSTNAMENUM ,wiseway 2002-11-01*/
                {
                    printf("\n        \t\t");
                }
            }
            if(i == MAXHOSTIPNUM - 1)
            {
                for( ; j < MAXHOSTNAMENUM; j++)
                {
                    if(hostentry_p->hostName[j].name[0]!='\0')
                    {
                        printf("               \t\t%s\n        \t\t", hostentry_p->hostName[j].name);
                    }
                }
                printf("\n");
            }
        }   /* for i < MAXHOSTIPNUM end */
    }
    L_MM_Free(hostentry_p);

        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_ClearHosts
 * PURPOSE:
 *  This routine prints a list of remote hosts, along with their Internet addresses and aliases
 *
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  none.
 *
 * RETURN:
 *  none.
 *
 * NOTES:
 *      This function will be called by CLI command " clear host [*] ".
 */
BOOL_T DNS_MGR_ClearHosts(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_HOSTLIB_HostsClear();

        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_HostDelete
 * PURPOSE:
 *  This routine deletes a host name from the local host table.  If <name> is
 *  a host name, the host entry is deleted.  If <name> is a host name alias,
 *  the alias is deleted.
 *
 * INPUT:
 *  I8_T * -- host name or alias
 *  I8_T * -- host addr in standard Internet format
 * OUTPUT:
 *  none.
 *
 * RETURN:
 *  DNS_OK, of DNS_ERROR if not find the entry..
 *
 * NOTES:
 *      This function will be called by CLI command "no ip host".
 */
int DNS_MGR_HostDelete(char *name, L_INET_AddrIp_T *addr_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_HOSTLIB_HostDelete(name,addr_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_HostNameDelete
 *
 * PURPOSE:
 *      This routine deletes a host name from the local host table.  If <name> is
 *      a host name, the host entry is deleted.  If <name> is a host name alias,
 *      only the alias is deleted.
 *
 * INPUT:
 *      I8_T * -- host name or alias
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_OK, of DNS_ERROR if not find the entry.
 *
 * NOTES:
 *      This function will be called by CLI command "clear host [name]"
 */
int DNS_MGR_HostNameDelete(char * name)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_HOSTLIB_HostNameDelete(name);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



/* FUNCTION NAME : DNS_MGR_SetHostName
 *
 * PURPOSE:
 *      This routine sets the target machine's symbolic name, which can be used
 *      for identification.
 *
 * INPUT:
 *      const I8_T * -- machine name
 *      int          -- length of name
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_OK, or DNS_ERROR if nameLen is larger then MAXHOSTNAMELEN
 *
 * NOTES:
 *      none
 */
int DNS_MGR_SetHostName(const I8_T * name,  int nameLen)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_HOSTLIB_SetHostName(name,nameLen);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME :  DNS_MGR_GetHostName
 *
 * PURPOSE:
 *      This routine gets the target machine's symbolic name, which can be used
 *      for identification.
 *
 * INPUT:
 *      int    -- length of name
 *
 * OUTPUT:
 *      I8_T * -- buffer to hold machine name .
 *
 * RETURN:
 *      DNS_OK, or DNS_ERROR if nameLen is smaller then the lenth of targetName.
 *
 * NOTES:
 *       none
 */
int DNS_MGR_GetHostName(I8_T *name, int nameLen)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_HOSTLIB_GetHostName(name,nameLen);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_GetNextDnsHostEntry
 * PURPOSE:
 *      This funciton get the specified  according to the index.
 *
 *
 * INPUT:
 *      int *host_index_p -- previous active index of HostEntry_T struct.
 *
 * OUTPUT:
 *      int *host_index_p -- current active index of HostEntry_T struct.
 *      HostEntry_PTR *  -- a pointer to a HostEntry_T variable to store the returned value.
 *
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK    :success.
 * NOTES:
 *      This function will be called by Config module.
 *      The initial value is -1.
 *      This function will be called by CLI command "show hosts"
 */
int DNS_MGR_GetNextDnsHostEntry(int *host_index_p,HostEntry_PTR dns_host_entry_t_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_HOSTLIB_GetNextHostEntry(host_index_p,dns_host_entry_t_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_GetDnsHostEntry
 * PURPOSE:
 *      This funciton get the specified  according to the index.
 *
 *
 * INPUT:
 *      int host_index    -- current active index of HostEntry_T struct.
 *
 * OUTPUT:
 *      int *host_index_p -- current active index of HostEntry_T struct.
 *      HostEntry_PTR *  -- a pointer to a HostEntry_T variable to store the returned value.
 *
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK    :success.
 * NOTES:
 *      This function will be called by Config module.
 *      This function will be called by web and snmp
 */
int DNS_MGR_GetDnsHostEntry(int host_index_p,HostEntry_PTR dns_host_entry_t_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;
    int index;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        index = host_index_p-1;

        ret = DNS_HOSTLIB_GetNextHostEntry(&index,dns_host_entry_t_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_AddDnsHostEntry
 * PURPOSE:
 *      This funciton add the specified struct to the local host table.
 *
 *
 * INPUT:
 *      HostEntry_PTR -- a pointer to a struct to be added to the local host table.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK    :success.
 * NOTES:
 *       This function will be called by Config module.
 */
int DNS_MGR_AddDnsHostEntry(HostEntry_PTR dns_host_entry_t_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_HOSTLIB_AddHostEntry(dns_host_entry_t_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_AddDomainName
 * PURPOSE:
 *  To define a default domain name that the ACCTON Switch software uses to
 *  complete unqualified host names (names without a dotted-decimal domain name)
 *
 *
 * INPUT:
 *  I8_T * -- a domain name to be added
 *
 * OUTPUT:
 *  none.
 *
 * RETURN:
 *  DNS_ERROR:failure;
 *  DNS_OK:succsess;
 * NOTES:
 *   This function will be called by CLI command "ip domain-name".
 */
int DNS_MGR_AddDomainName(char *domain_name_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        if (DNS_MGR_CheckHostName(domain_name_p) != TRUE)
        {
            SYSFUN_RELEASE_CSC();
            return DNS_ERROR;
        }

        ret = DNS_OM_AddDomainName(domain_name_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_DeleteDomainName
 * PURPOSE:
 *  Delete the default domain name.
 *
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  none.
 *
 * RETURN:
 *  DNS_OK:success;
 *
 * NOTES:
 *   This function will be called by CLI command "no ip domain-name".
 */
int DNS_MGR_DeleteDomainName(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_DeleteDomainName();

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_AddDomainNameToList
 * PURPOSE:
 *  If there is no domain list, the domain name that you specified with the ip
 *  domain-name global configuration command is used. If there is a domain list,
 *  the default domain name is not used. The ip domain-list command is similar
 *  to the ip domain-name command, except that with the ip domain-list command
 *      you can define a list of domains, each to be tried in turn.
 * INPUT:
 *  I8_T * -- a domain name to be added to the domain name lsit.
 *
 * OUTPUT:
 *  none.
 *
 * RETURN:
 *  DNS_ERROR:failure;
 *  DNS_OK:succsess;
 *
 * NOTES:
 *   This function will be called by CLI command "ip domain-list"
 */
int DNS_MGR_AddDomainNameToList(char *domain_name_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        if (DNS_MGR_CheckHostName(domain_name_p) != TRUE)
        {
            SYSFUN_RELEASE_CSC();
            return DNS_ERROR;
        }

        ret = DNS_OM_AddDomainNameToList(domain_name_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}




/* FUNCTION NAME : DNS_MGR_DeleteDomainNameToList
 * PURPOSE:
 *  If there is no domain list, the domain name that you specified with the ip
 *  domain-name global configuration command is used. If there is a domain list,
 *  the default domain name is not used. The ip domain-list command is similar
 *  to the ip domain-name command, except that with the ip domain-list command
 *  you can define a list of domains, each to be tried in turn.
 * INPUT:
 *  I8_T * -- a domain name to be deleted to the domain name lsit
 *
 * OUTPUT:
 *  none.
 *
 * RETURN:
 *  DNS_ERROR:failure;
 *  DNS_OK:succsess;
 *
 * NOTES:
 *   This function will be called by CLI comamnd "no ip domain-list"
 */
int DNS_MGR_DeleteDomainNameFromList(char *domain_name_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_DeleteDomainNameFromList(domain_name_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_ShowDomainNameList
 * PURPOSE:
 *  This function shows the dns domain name list
 *
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  none.
 *
 * RETURN:
 *  none.
 *
 * NOTES:
 *   .
 */
BOOL_T DNS_MGR_ShowDomainNameList(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_ShowDomainNameList();

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_ShowDnsConfig
 * PURPOSE:
 *  This function shows the dns configuration.
 *
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  none.
 *
 * RETURN:
 *  none.
 *
 * NOTES:
 *   This function will be called by CLI module.
 */
BOOL_T DNS_MGR_ShowDnsConfig(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;
    char    dns_ip_domain_name[DNS_MAX_NAME_LENGTH+1]={0};
    L_INET_AddrIp_T ip;
    char    ipaddr[L_INET_MAX_IPADDR_STR_LEN+1];

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
#if 0

        DNS_OM_ShowDnsConfig();

        ret = TRUE;
#else
        printf("\ndomain lookup status:\n");
        if ( DNS_MGR_GetDnsStatus() == DNS_ENABLE )
        {
            printf("DNS enabled\n");
        }
        else
        {
            printf("DNS disabled\n");
        }
        printf("Domain Name:\n");
        DNS_MGR_GetDnsIpDomain((char *)dns_ip_domain_name);
        printf("%s\n",dns_ip_domain_name);
        strcpy((char *)dns_ip_domain_name, "");
        printf("Domain Name List:\n");
        while( DNS_MGR_GetNextDomainNameList(dns_ip_domain_name) == TRUE )
        {
            printf("%s\n",dns_ip_domain_name);
        }
        printf("Name Server List:\n");
        memset(&ip, 0, sizeof(ip));
        while ( DNS_MGR_GetNextNameServerList(&ip) == TRUE )
        {
            L_INET_InaddrToString((L_INET_Addr_T*)&ip, ipaddr, sizeof(ipaddr));
            printf("%s\n",ipaddr);
        }

        printf("------------------------------\n");
        ret = TRUE;
#endif
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_SetDnsTimeOut
 * PURPOSE:
 *  This fcuntion sets time out value (seconds) for requests.
 *
 *
 *
 * INPUT:
 *  UI32_T* --  a pointer to a variable for storing timeout value
 *     for requests. range:1..20 . default:5 seconds
 * OUTPUT:
 *  none.
 *
 * RETURN:
 *  DNS_ERROR:failure;
 *  DNS_OK:succsess;
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
int  DNS_MGR_SetDnsTimeOut(UI32_T *dns_time_out_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_SetDnsTimeOut(dns_time_out_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetDnsTimeOut
 * PURPOSE:
 *  This fcuntion gets the time out value (seconds) for requests.
 *
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  UI32_T * -- a pointer to a variable for storing the returned value
 *     range 1:20; default: 5 seconds
 * RETURN:
 *  DNS_ERROR:failure;
 *  DNS_OK:succsess;
 * NOTES:
 *   This function will be called by snmp module.
 */
int DNS_MGR_GetDnsTimeOut(UI32_T *dns_time_out_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsTimeOut(dns_time_out_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsErrors
 * PURPOSE:
 *  Number of requests the server has processed which
 *  originated from friends and were answered with errors
 *  (RCODE values other than 0 and 3).  The definition of
 *  friends is a locally defined matter.
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  UI32_T* -- a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_ERROR:failure;
 *  DNS_OK:succsess;
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterFriendsErrors(UI32_T *dns_serv_opt_counter_friends_errors_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterFriendsErrors(dns_serv_opt_counter_friends_errors_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsReferrals
 * PURPOSE:
 *  Number of requests which originated from friends that
 *  were referred to other servers.  The definition of
 *  friends is a locally defined matter.
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  UI32_T * -- a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_ERROR:failure;
 *  DNS_OK:succsess;
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterFriendsReferrals(UI32_T *dns_serv_opt_counter_friends_referrals_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterFriendsReferrals(dns_serv_opt_counter_friends_referrals_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsOtherErrors
 * PURPOSE:
 *  Number of requests which were aborted for other (local)
 *  server errors and which originated from `friends'.
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  UI32_T * -- a pointer to the variable to store the returned value
 *
 * RETURN:
 *  DNS_ERROR:failure;
 *  DNS_OK:succsess;
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterFriendsOtherErrors(UI32_T *dns_serv_opt_counter_friends_other_errors_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterFriendsOtherErrors(dns_serv_opt_counter_friends_other_errors_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsReqUnparses
 * PURPOSE:
 *  Number of requests received which were unparseable and
 *  which originated from `friends'
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  UI32_T * -- a pointer to the variable to store the returned value.
 *
 * RETURN:
 *  DNS_ERROR:failure;
 *  DNS_OK:succsess;
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterFriendsReqUnparses(UI32_T *dns_serv_opt_counter_friends_req_unparses_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterFriendsReqUnparses(dns_serv_opt_counter_friends_req_unparses_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsReqRefusals
 * PURPOSE:
 *  This function gets the number of DNS requests refused by the server which were
 *  received from `friends'.
 *
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  UI32_T * -- a pointer to the variable to store the returned value
 *
 * RETURN:
 *  DNS_ERROR:failure;
 *  DNS_OK:succsess;
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterFriendsReqRefusals(UI32_T *dns_serv_opt_counter_friends_req_refusals_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterFriendsReqRefusals(dns_serv_opt_counter_friends_req_refusals_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsRelNames
 * PURPOSE:
 *  Number of requests received for names from friends that
 *  are only 1 label long (text form - no internal dots) the
 *  server has processed
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  UI32_T * -- a pointer to a variable to store the result.
 *
 * RETURN:
 *  DNS_ERROR:failure;
 *  DNS_OK:succsess;
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterFriendsRelNames(UI32_T *dns_serv_opt_counter_friends_rel_names_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterFriendsRelNames(dns_serv_opt_counter_friends_rel_names_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsNonAuthNoDatas
 * PURPOSE:
 *  Number of queries originating from friends which were
 *  non-authoritatively answered with no such data (empty
 *  answer)
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *  UI32_T * -- a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_ERROR:failure;
 *  DNS_OK:succsess;
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServOptCounterFriendsNonAuthNoDatas(UI32_T *dns_serv_opt_counter_friends_non_auth_no_datas_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterFriendsNonAuthNoDatas(dns_serv_opt_counter_friends_non_auth_no_datas_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetDnsResCounterByRcodeEntry
 * PURPOSE:
 *  This function gets the number of responses for the specified index.
 *
 *
 *
 * INPUT:
 *  DNS_ResCounterByRcodeEntry_T * -- Given as a index for the entry.
 *            INDEX { dnsResCounterByRcodeCode }
 * OUTPUT:
 *  DNS_ResCounterByRcodeEntry_T * --a pointer to a DnsResCounterByRcodeEntry variable to store the returned entry
 *
 * RETURN:
 *  DNS_ERROR:failure;
 *  DNS_OK:succsess;
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
int DNS_MGR_GetDnsResCounterByRcodeEntry(DNS_ResCounterByRcodeEntry_T *dns_res_counter_by_rcode_entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResCounterByRcodeEntry(dns_res_counter_by_rcode_entry_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetNextDnsResCounterByRcodeEntry
 * PURPOSE:
 *  This function gets the number of responses for the index next to the specified index.
 *
 *
 *
 * INPUT:
 *  DNS_ResCounterByRcodeEntry_T * -- Given as the index of the entry.
 *           INDEX { dnsResCounterByRcodeCode }
 * OUTPUT:
 *  DNS_ResCounterByRcodeEntry_T * -- a pointer to a DnsResCounterByRcodeEntry variable to store the returned entry
 *
 * RETURN:
 *  DNS_ERROR:failure;
 *  DNS_OK:succsess;
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
int DNS_MGR_GetNextDnsResCounterByRcodeEntry(DNS_ResCounterByRcodeEntry_T *dns_res_counter_by_rcode_entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetNextDnsResCounterByRcodeEntry(dns_res_counter_by_rcode_entry_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetDnsResCounterByOpcodeEntry
 * PURPOSE:
 *  This function gets dnsResCounterByOpcodeEntry according to the index.
 *
 *
 *
 * INPUT:
 *  DNS_ResCounterByOpcodeEntry_T * -- Given as the index of the entry.
 *
 * OUTPUT:
 *  DNS_ResCounterByOpcodeEntry_T * -- a pointer to a variable t store the returned entry
 *
 * RETURN:
 *  DNS_ERROR:failure;
 *  DNS_OK   :succsess;
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
int DNS_MGR_GetDnsResCounterByOpcodeEntry(DNS_ResCounterByOpcodeEntry_T *dns_res_counter_by_opcode_entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDnsResCounterByOpcodeEntry(dns_res_counter_by_opcode_entry_p);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_GetNextDnsResCounterByOpcodeEntry
 * PURPOSE:
 *  This function gets dnsResCounterByOpcodeEntry next to the specified index.
 *
 *
 *
 * INPUT:
 *  DNS_ResCounterByOpcodeEntry_T * -- Given to provide the index of the entry.
 *
 * OUTPUT:
 *  DNS_ResCounterByOpcodeEntry_T * -- a pointer to a variable to store the returned entry
 *
 * RETURN:
 *  DNS_ERROR:failure;
 *  DNS_OK   :succsess;
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
int DNS_MGR_GetNextDnsResCounterByOpcodeEntry(DNS_ResCounterByOpcodeEntry_T *dns_res_counter_by_opcode_entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetNextDnsResCounterByOpcodeEntry(dns_res_counter_by_opcode_entry_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterAuthAns
 *
 * PURPOSE:
 *  This function gets the Number of queries which were authoritatively answered.
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int *
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServCounterAuthAns(int *dns_serv_counter_auth_ans_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServCounterAuthAns(dns_serv_counter_auth_ans_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterAuthNoNames
 *
 * PURPOSE:
 *  This function gets the Number of queries for which `authoritative no such name'
 *      responses were made.
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int *
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServCounterAuthNoNames(int *dns_serv_counter_auth_ans_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServCounterAuthNoNames((UI32_T *)dns_serv_counter_auth_ans_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterAuthNoDataResps
 *
 * PURPOSE:
 *  This function gets the Number of queries for which `authoritative no such data'
 *       (empty answer) responses were made
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServCounterAuthNoDataResps(int *dns_serv_counter_auth_no_data_resps_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServCounterAuthNoDataResps(dns_serv_counter_auth_no_data_resps_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterNonAuthDatas
 *
 * PURPOSE:
 *  This function gets the Number of queries which were non-authoritatively
 *      answered (cached data)
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServCounterNonAuthDatas(int *dns_serv_counter_non_auth_datas_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServCounterNonAuthDatas(dns_serv_counter_non_auth_datas_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterNonAuthNoDatas
 *
 * PURPOSE:
 *  This funciton gets the number of Number of queries which were non-authoritatively
         answered with no data (empty answer)
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServCounterNonAuthNoDatas(UI32_T *dns_serv_counter_non_auth_no_datas_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServCounterNonAuthNoDatas(dns_serv_counter_non_auth_no_datas_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterReferrals
 *
 * PURPOSE:
 *  This function gets the Number of requests that were referred to other servers
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *   int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServCounterReferrals(int *dns_serv_counter_referrals_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServCounterReferrals(dns_serv_counter_referrals_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterErrors
 *
 * PURPOSE:
 *  This function gets the Number of requests the server has processed that were
 *      answered with errors (RCODE values other than 0 and 3).
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServCounterErrors(int *dns_serv_counter_errors_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServCounterErrors(dns_serv_counter_errors_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterRelNames
 *
 * PURPOSE:
 *  This function gets the Number of requests received by the server for names that
 *       are only 1 label long (text form - no internal dots)
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServCounterRelNames(int *dns_serv_counter_rel_names_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServCounterRelNames(dns_serv_counter_rel_names_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterReqRefusals
 *
 * PURPOSE:
 *  This function gets the Number of DNS requests refused by the server.
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *   int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServCounterReqRefusals(int *dns_serv_counter_req_refusals_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServCounterReqRefusals(dns_serv_counter_req_refusals_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterReqUnparses
 *
 * PURPOSE:
 *  This function gets the Number of requests received which were unparseable
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to store the result
 *

 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServCounterReqUnparses(int *dns_serv_counter_req_unparses_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServCounterReqUnparses(dns_serv_counter_req_unparses_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServCounterOtherErrors
 *
 * PURPOSE:
 *  This function gets the Number of requests which were aborted for other (local)
 *      server errors
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *   int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServCounterOtherErrors(int *dns_serv_counter_other_errors_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServCounterOtherErrors((UI32_T *)dns_serv_counter_other_errors_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetDnsServCounterEntry
 * PURPOSE:
 *  This function gets the dnsServCounterEntry according the specified index.
 *
 *
 *
 * INPUT:
 *  DNS_ServCounterEntry_T * --  Given to provide the index of the entry.
 *                              INDEX { dnsServCounterOpCode, dnsServCounterQClass, dnsServCounterQType, dnsServCounterTransport }
 * OUTPUT:
 *  DNS_ServCounterEntry_T * -- a pointer to a DNS_ServCounterEntry_T variable to store the returned entry
 *
 * RETURN:
 *  DNS_ERROR:failure;
 *  DNS_OK   :succsess;
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
int DNS_MGR_GetDnsServCounterEntry(DNS_ServCounterEntry_T *dns_serv_counter_entry_t_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServCounterEntry(dns_serv_counter_entry_t_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



/* FUNCTION NAME : DNS_MGR_GetNextDnsServCounterEntry
 * PURPOSE:
 *  This function gets the dnsServCounterEntry next to the specified index.
 *
 *
 *
 * INPUT:
 *  DNS_ServCounterEntry_T * --  Given to provide the index of the entry.
 *                              INDEX { dnsServCounterOpCode, dnsServCounterQClass, dnsServCounterQType, dnsServCounterTransport }
 * OUTPUT:
 *  DNS_ServCounterEntry_T * -- a pointer to a DNS_ServCounterEntry_T variable to store the returned entry
 *
 * RETURN:
 *  DNS_ERROR:failure;
 *  DNS_OK   :succsess;
 *
 * NOTES:
 *      This function will be called by snmp module.
 *      The initial input is {0,0,0,0,0,0}
 */
int DNS_MGR_GetNextDnsServCounterEntry(DNS_ServCounterEntry_T *dns_serv_counter_entry_t_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetNextDnsServCounterEntry(dns_serv_counter_entry_t_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfAuthAns
 *
 * PURPOSE:
 *  This function gets the number of requests the server has processed which
 *      originated from a resolver on the same host for which  there has been an
 *      authoritative answer.
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServOptCounterSelfAuthAns(int *dns_serv_opt_counter_self_auth_ans_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterSelfAuthAns(dns_serv_opt_counter_self_auth_ans_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfAuthNoNames
 *
 * PURPOSE:
 *  This function gets the number of requests the server has processed which
        originated from a resolver on the same host for which
        there has been an authoritative no such name answer given.
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServOptCounterSelfAuthNoNames(int *dns_serv_opt_counter_self_auth_no_names_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterSelfAuthNoNames(dns_serv_opt_counter_self_auth_no_names_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfAuthNoDataResps
 *
 * PURPOSE:
 *  This function gets the number of requests the server has processed which
 *       originated from a resolver on the same host for which
 *       there has been an authoritative no such data answer (empty answer) made.
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServOptCounterSelfAuthNoDataResps(int *dns_serv_opt_counter_self_auth_no_data_resps)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterSelfAuthNoDataResps(dns_serv_opt_counter_self_auth_no_data_resps);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfNonAuthDatas
 *
 * PURPOSE:
 *   Number of requests the server has processed which
          originated from a resolver on the same host for which a
          non-authoritative answer (cached data) was made
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServOptCounterSelfNonAuthDatas(int *dns_serv_opt_counter_self_non_auth_datas_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterSelfNonAuthDatas(dns_serv_opt_counter_self_non_auth_datas_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfNonAuthNoDatas
 *
 * PURPOSE:
 *  Number of requests the server has processed which
 *      originated from a resolver on the same host for which a
 *      non-authoritative, no such data' response was made
 *      (empty answer).
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:

 *  none
 */
int DNS_MGR_GetDnsServOptCounterSelfNonAuthNoDatas(int *dns_serv_opt_counter_self_non_auth_no_datas_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterSelfNonAuthNoDatas(dns_serv_opt_counter_self_non_auth_no_datas_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfReferrals
 *
 * PURPOSE:
 *  Number of queries the server has processed which
 *       originated from a resolver on the same host and were
 *       referred to other servers
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServOptCounterSelfReferrals(int *dns_serv_opt_counter_self_referrals)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterSelfReferrals(dns_serv_opt_counter_self_referrals);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfErrors
 *
 * PURPOSE:
 *  Number of requests the server has processed which
 *       originated from a resolver on the same host which have
 *      been answered with errors (RCODEs other than 0 and 3).
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServOptCounterSelfErrors(int *dns_serv_opt_counter_self_errors_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterSelfErrors(dns_serv_opt_counter_self_errors_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfRelNames
 *
 * PURPOSE:
 *  Number of requests received for names that are only 1
 *      label long (text form - no internal dots) the server has
 *       processed which originated from a resolver on the same
 *       host.
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServOptCounterSelfRelNames( int *dns_serv_opt_counter_self_rel_names)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterSelfRelNames(dns_serv_opt_counter_self_rel_names);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfReqRefusals
 *
 * PURPOSE:
 *  Number of DNS requests refused by the server which
 *      originated from a resolver on the same host
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServOptCounterSelfReqRefusals(int *dns_serv_opt_counter_self_req_refusals_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterSelfReqRefusals(dns_serv_opt_counter_self_req_refusals_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfReqUnparses
 *
 * PURPOSE:
 *  Number of requests received which were unparseable and
 *      which originated from a resolver on the same host.
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServOptCounterSelfReqUnparses(int *dns_serv_opt_counter_self_req_unparses_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterSelfReqUnparses(dns_serv_opt_counter_self_req_unparses_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterSelfOtherErrors
 *
 * PURPOSE:
 *  Number of requests which were aborted for other (local)
 *      server errors and which originated on the same host.
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * --   a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServOptCounterSelfOtherErrors(int *dns_serv_opt_counter_self_other_errors_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterSelfOtherErrors(dns_serv_opt_counter_self_other_errors_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsAuthAns
 *
 * PURPOSE:
 *  Number of queries originating from friends which were
 *      authoritatively answered.  The definition of friends is
 *      a locally defined matter
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  int * --   a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServOptCounterFriendsAuthAns(int *dns_serv_opt_counter_friends_auth_ans_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterFriendsAuthAns(dns_serv_opt_counter_friends_auth_ans_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

 /*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsAuthNoNames
 *
 * PURPOSE:
 *  Number of queries originating from friends, for which
 *      authoritative `no such name' responses were made.  The
 *      definition of friends is a locally defined matter.
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  none
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServOptCounterFriendsAuthNoNames(int *dns_serv_opt_counter_friends_auth_no_names)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterFriendsAuthNoNames(dns_serv_opt_counter_friends_auth_no_names);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsAuthNoDataResps
 *
 * PURPOSE:
 *  Number of queries originating from friends for which
 *      authoritative no such data (empty answer) responses were
 *      made.  The definition of friends is a locally defined
 *      matter
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  UI32_T * --  a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServOptCounterFriendsAuthNoDataResps(UI32_T *dns_serv_opt_counter_friends_auth_no_data_resps)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterFriendsAuthNoDataResps(dns_serv_opt_counter_friends_auth_no_data_resps);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/*
 * FUNCTION NAME : DNS_MGR_GetDnsServOptCounterFriendsNonAuthDatas
 *
 * PURPOSE:
 *  Number of queries originating from friends which were
 *      non-authoritatively answered (cached data). The
 *      definition of friends is a locally defined matter.
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  UI32_T * --  a pointer to a variable to store the result
 *
 * RETURN:
 *  DNS_OK/DNS_ERROR
 *
 * NOTES:
 *  none
 */
int DNS_MGR_GetDnsServOptCounterFriendsNonAuthDatas(UI32_T *dns_serv_opt_counter_friends_non_auth_datas_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsServOptCounterFriendsNonAuthDatas(dns_serv_opt_counter_friends_non_auth_datas_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_ResCounterByRcodeInc
 *
 * PURPOSE:
 *  resolver counter add 1 (index by rcode)
 *
 * INPUT:
 *  int -- index
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
BOOL_T DNS_MGR_ResCounterByRcodeInc(int rcode)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_OM_ResCounterByRcodeInc(rcode);

        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}


/* FUNCTION NAME : DNS_MGR_ResCounterByOpcodeInc
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
BOOL_T DNS_MGR_ResCounterByOpcodeInc(int qr,int opcode)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_OM_ResCounterByOpcodeInc(qr,opcode);

        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_ResCounterInc
 *
 * PURPOSE:
 *  Resolver counter add 1 (index by leaf)
 *
 * INPUT:
 *  int -- leaf.
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
BOOL_T DNS_MGR_ResCounterInc(int leaf)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_OM_ResCounterInc(leaf);

        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_ResOptCounterInc
 *
 * PURPOSE:
 *   Resolver's opt counter add 1 (index by leaf)
 *
 * INPUT:
 *  int -- leaf.
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
BOOL_T DNS_MGR_ResOptCounterInc(int leaf)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_OM_ResOptCounterInc(leaf);

        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_ServOptCounterInc
 *
 * PURPOSE:
 *   Server counter add 1 (index by leaf)
 *
 * INPUT:
 *  int -- .
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
BOOL_T DNS_MGR_ServOptCounterInc(int leaf)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_OM_ServOptCounterInc(leaf);

        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_ServCounterInc
 *
 * PURPOSE:
 *  Server's counter add 1(index with leaf)
 *
 * INPUT:
 *  int.
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
BOOL_T DNS_MGR_ServCounterInc(int leaf)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_OM_ServCounterInc(leaf);

        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetDnsDebugStatus
 *
 * PURPOSE:
 *   Get whether server debug status is open.
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
int DNS_MGR_GetDnsDebugStatus(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_DISABLE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        DNS_OM_GetDnsDebugStatus(&ret);   /*ret= is ommitted 2002-10-27 wiseway*/
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_ShowCounter
 *
 * PURPOSE:
 *  show dns counter.
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
BOOL_T DNS_MGR_ShowCounter(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_OM_ShowCounter();

        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetDnsStatus
 *
 * PURPOSE:
 *   Get dns status
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ENABLE or DNS_DISABLE.
 *
 * NOTES:
 *      This function will be called by CLI command "show dns"
 */
int DNS_MGR_GetDnsStatus(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_DISABLE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_OM_GetDnsStatus(&ret);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_SetDnsStatus
 *
 * PURPOSE:
 *   set dns status
 *
 * INPUT:
 *  int -- status.
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
BOOL_T DNS_MGR_SetDnsStatus(int status)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_OM_SetDnsStatus(status);

        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetDnsIpDomain
 *
 * PURPOSE:
 *  Get dns ip domain name.
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *      char* -- ipdomain name .
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK :success
 *
 * NOTES:
 *      This function will be called by CLI command "show dns"
 */
int DNS_MGR_GetDnsIpDomain(char* ipdomain)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDnsIpDomain(ipdomain);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_DnsUptimeInit
 *
 * PURPOSE:
 *  Initiate dns uptime
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
BOOL_T DNS_MGR_DnsUptimeInit(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_OM_DnsUptimeInit();

        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_DnsResetTimeInit
 *
 * PURPOSE:
 *  initiate dns reset time
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
BOOL_T DNS_MGR_DnsResetTimeInit(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_OM_DnsResetTimeInit();

        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetDnsSbelt
 *
 * PURPOSE:
 *  Get dns sbelt.
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *      DNS_ResConfigSbelt_T* -- dns sbelt
 *
 * RETURN:
 *  none.
 *
 * NOTES:
 *  none.
 */
DNS_ResConfigSbelt_T* DNS_MGR_GetDnsSbelt(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    DNS_ResConfigSbelt_T *sbelt_p = NULL;

    /* BODY */
    SYSFUN_USE_CSC(sbelt_p);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return sbelt_p;
    }
    else
    {

        sbelt_p = DNS_OM_GetDnsSbelt();

    }
    SYSFUN_RELEASE_CSC();
    return sbelt_p;
}

/* FUNCTION NAME : DNS_MGR_ResCounterInit
 *
 * PURPOSE:
 *   initiate dns resolver counter
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
BOOL_T DNS_MGR_ResCounterInit(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_OM_ResCounterInit();

        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_ServCounterInit
 *
 * PURPOSE:
 *  initiate dns server counter.
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
BOOL_T DNS_MGR_ServCounterInit(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_OM_ServCounterInit();

        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetServStatus
 *
 * PURPOSE:
 *  get dns server status
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  int -- dns server status
 *
 * NOTES:
 *  none.
 */
int DNS_MGR_GetServStatus(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_SERV_STATUS_STOPPED;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetServStatus();

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_SetServStatus
 *
 * PURPOSE:
 *  ser dns server status
 *
 * INPUT:
 *  int -- dns server status
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
BOOL_T DNS_MGR_SetServStatus(int status)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_OM_SetServStatus(status);

        ret = FALSE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetServCurrentRequestNumber
 *
 * PURPOSE:
 *  get server current request number.
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  int -- current server request number
 *
 * NOTES:
 *  none.
 */
int DNS_MGR_GetServCurrentRequestNumber(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = 0;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetServCurrentRequestNumber();

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_ServCurrentRequestNumberInc
 *
 * PURPOSE:
 *  current server request number add 1
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
BOOL_T DNS_MGR_ServCurrentRequestNumberInc(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_OM_ServCurrentRequestNumberInc();

        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_ServCurrentRequestNumberDec
 *
 * PURPOSE:
 *  Dec current request number
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
BOOL_T DNS_MGR_ServCurrentRequestNumberDec(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_OM_ServCurrentRequestNumberDec();

        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_SetServResetStatus
 *
 * PURPOSE:
 *  Ser server reset status.
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
BOOL_T DNS_MGR_SetServResetStatus(int status)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_OM_SetServResetStatus(status);

        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_SetServServiceEnable
 *
 * PURPOSE:
 *  set dns server service whether is enable
 *
 * INPUT:
 *  none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *  none.
 */
BOOL_T DNS_MGR_SetServServiceEnable(int enable)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_OM_SetServServiceEnable(enable);

        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetServServiceEnable
 *
 * PURPOSE:
 *  Get whether server is enable
 *
 * INPUT:
 *  none.
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
int DNS_MGR_GetServServiceEnable(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_SERV_DISABLED;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetServServiceEnable();

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_ServUpTimeInit
 *
 * PURPOSE:
 *  initiate server uptime
 *
 * INPUT:
 *  none.
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
BOOL_T DNS_MGR_ServUpTimeInit(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_OM_ServUpTimeInit();

        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_ServResetTimeInit
*
 * PURPOSE:
 *  initiate server reset time
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
 *  none
 */
BOOL_T DNS_MGR_ServResetTimeInit(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_OM_ServResetTimeInit();

        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



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
 *              switch will be initiated to the factory default value.
 *          2. DNS will handle network requests only when this subsystem
 *              is in the Master Operation mode
 *          3. This function is invoked in DNS_INIT_EnterMasterMode.
 */
BOOL_T DNS_MGR_EnterMasterMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */

    DNS_OM_SetDnsStatus(DNS_DISABLE);

    DNS_OM_SetServStatus(DNS_SERV_STARTED);
    DNS_OM_ServResetTimeInit();
    DNS_OM_SetServServiceEnable(DNS_SERV_ENABLED);
    DNS_OM_SetServResetStatus(VAL_dnsServConfigReset_running);  /* 4 running*/
    DNS_OM_ServCounterInit();


/* set mgr in master mode */
    SYSFUN_ENTER_MASTER_MODE();

    return TRUE;
}



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
 *          .
 */
BOOL_T DNS_MGR_EnterTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    SYSFUN_ENTER_TRANSITION_MODE();

    DNS_OM_ClearDatabase();

    return TRUE;
}



/* FUNCTION NAME : DNS_MGR_SetTransitionMode
 * PURPOSE:
 *      Set transition mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void DNS_MGR_SetTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */

    /* set transition flag to prevent calling request */
    SYSFUN_SET_TRANSITION_MODE();
}



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
void DNS_MGR_EnterSlaveMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    SYSFUN_ENTER_SLAVE_MODE();

    return;

}



/* FUNCTION NAME : DNS_MGR_GetOperationMode
 * PURPOSE:
 *      Get current sshd operation mode (master / slave / transition).
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *          SYS_TYPE_Stacking_Mode_T - opmode.
 *
 * NOTES:
 *      None.
 */
SYS_TYPE_Stacking_Mode_T DNS_MGR_GetOperationMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    return ( SYSFUN_GET_CSC_OPERATING_MODE() );
}



/* FUNCTION NAME : DNS_MGR_GetResolverTaskId
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
BOOL_T DNS_MGR_GetResolverTaskId(UI32_T *tid)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetResolverTaskId(tid);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



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
void DNS_MGR_BackdoorFunction()
{
    char keyin[256];
    UI8_T    hostname[256];
    char hostaddr[L_INET_MAX_IPADDR_STR_LEN+1];
    L_INET_AddrIp_T ipadd;
    I32_T   rc;

    while(1)
    {
        BACKDOOR_MGR_Printf("\r\n1. ip host.");
        BACKDOOR_MGR_Printf("\r\n2. clear host.");
        BACKDOOR_MGR_Printf("\r\n3. ip domain-list.");
        BACKDOOR_MGR_Printf("\r\n4. ip domain-lookup.");
        BACKDOOR_MGR_Printf("\r\n5. ip domain-name.");
        BACKDOOR_MGR_Printf("\r\n6. ip name-server.");
        BACKDOOR_MGR_Printf("\r\n7. show hosts.");
        BACKDOOR_MGR_Printf("\r\n8. clear dns cache.");
        BACKDOOR_MGR_Printf("\r\n9. show dns config.");
        BACKDOOR_MGR_Printf("\r\na. show dns database.");
        BACKDOOR_MGR_Printf("\r\nb. show dns cache.");
        BACKDOOR_MGR_Printf("\r\nc. debug ip dns.");
        BACKDOOR_MGR_Printf("\r\nd. gethostbyname.");
        BACKDOOR_MGR_Printf("\r\ne. l_stoip.");
        BACKDOOR_MGR_Printf("\r\nf. set domain list by index.");
        BACKDOOR_MGR_Printf("\r\ng. get next domain list by index.");
        BACKDOOR_MGR_Printf("\r\nh. set host by index.");
        BACKDOOR_MGR_Printf("\r\ni. get next host by index.");
        BACKDOOR_MGR_Printf("\r\nj. set host ip entry.");
        BACKDOOR_MGR_Printf("\r\nk. get next host ip entry.");
        BACKDOOR_MGR_Printf("\r\nu. unit test.");
        BACKDOOR_MGR_Printf("\r\n0. Exit.\r\n");
        BACKDOOR_MGR_Printf("\r\nEnter your choice: ");

        *keyin = 0;
        BACKDOOR_MGR_RequestKeyIn(keyin, 1);
        BACKDOOR_MGR_Printf("\r\n");

        switch(*keyin)
        {
            case '0':
                return;

            case '1':
                BACKDOOR_MGR_Printf("\r\n1. ip host.");
                BACKDOOR_MGR_Printf("\r\n2. no ip hostt.");
                BACKDOOR_MGR_Printf("\r\nEnter your choice: ");
                *keyin = 0;
                BACKDOOR_MGR_RequestKeyIn(keyin, 1);
                BACKDOOR_MGR_Printf("\r\n");
                switch(*keyin)
                {
                    case '1':
                        BACKDOOR_MGR_Printf("\r\nInput hostname : ");
                        BACKDOOR_MGR_RequestKeyIn(keyin, 256);
                        BACKDOOR_MGR_Printf("\r\n");
                        strcpy((char *)hostname,keyin);
                        BACKDOOR_MGR_Printf("Input hostaddr : ");
                        BACKDOOR_MGR_RequestKeyIn(keyin, L_INET_MAX_IPADDR_STR_LEN+1);
                        BACKDOOR_MGR_Printf("\r\n");
                        strcpy((char *)hostaddr,keyin);

                        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC,
                                                                           hostaddr,
                                                                           (L_INET_Addr_T *)&ipadd,
                                                                           sizeof(ipadd)))
                        {
                            BACKDOOR_MGR_Printf("Convert addr String to Inaddr fail\n");
                        }

                        DNS_MGR_HostAdd((char*)hostname,&ipadd);
                        break;
                    case '2':
                        BACKDOOR_MGR_Printf("\r\nInput hostname : ");
                        BACKDOOR_MGR_RequestKeyIn(keyin, 256);
                        BACKDOOR_MGR_Printf("\r\n");
                        strcpy((char *)hostname,keyin);
                        BACKDOOR_MGR_Printf("Input hostaddr : ");
                        BACKDOOR_MGR_RequestKeyIn(keyin, L_INET_MAX_IPADDR_STR_LEN+1);
                        BACKDOOR_MGR_Printf("\r\n");
                        strcpy((char *)hostaddr,keyin);

                        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC,
                                                                           hostaddr,
                                                                           (L_INET_Addr_T *)&ipadd,
                                                                           sizeof(ipadd)))
                        {
                            BACKDOOR_MGR_Printf("Convert addr String to Inaddr fail\n");
                        }

                        DNS_MGR_HostDelete((char*)hostname,&ipadd);
                        break;
                    default:
                        BACKDOOR_MGR_Printf("\r\nChoice error!!");
                        break;
                }
                break;

            case '2':
                BACKDOOR_MGR_Printf("\r\nInput hostname : ");
                BACKDOOR_MGR_RequestKeyIn(keyin, 256);
                BACKDOOR_MGR_Printf("\r\n");
                strcpy((char *)hostname,keyin);
                if ( strncmp((char *)hostname,"*",1) == 0 )
                {
                    DNS_MGR_ClearHosts();
                }
                else
                {
                    DNS_MGR_HostNameDelete((char*)hostname);
                }
                break;

            case '3':
                BACKDOOR_MGR_Printf("\r\n1. ip domain-list.");
                BACKDOOR_MGR_Printf("\r\n2. no ip domain-list.");
                BACKDOOR_MGR_Printf("\r\nEnter your choice: ");
                *keyin = 0;
                BACKDOOR_MGR_RequestKeyIn(keyin, 1);
                BACKDOOR_MGR_Printf("\r\n");
                switch(*keyin)
                {
                    case '1':
                        BACKDOOR_MGR_Printf("\r\nInput domain name : ");
                        BACKDOOR_MGR_RequestKeyIn(keyin, 256);
                        BACKDOOR_MGR_Printf("\r\n");
                        strcpy((char *)hostname,keyin);
                        DNS_MGR_AddDomainNameToList((char*)hostname);
                        break;
                    case '2':
                        BACKDOOR_MGR_Printf("\r\nInput domain name : ");
                        BACKDOOR_MGR_RequestKeyIn(keyin, 256);
                        BACKDOOR_MGR_Printf("\r\n");
                        strcpy((char *)hostname,keyin);
                        DNS_MGR_DeleteDomainNameFromList((char*)hostname);
                        break;
                    default:
                        BACKDOOR_MGR_Printf("\r\nChoice error!!");
                        break;
                }
                break;

            case '4':
                BACKDOOR_MGR_Printf("\r\n1. ip domain-lookup.");
                BACKDOOR_MGR_Printf("\r\n2. no ip domain-lookup.");
                BACKDOOR_MGR_Printf("\r\nEnter your choice: ");
                *keyin = 0;
                BACKDOOR_MGR_RequestKeyIn(keyin, 1);
                BACKDOOR_MGR_Printf("\r\n");
                switch(*keyin)
                {
                    case '1':
                        DNS_MGR_EnableDomainLookup();
                        break;
                    case '2':
                        DNS_MGR_DisableDomainLookup();
                        break;
                    default:
                        BACKDOOR_MGR_Printf("\r\nChoice error!!");
                        break;
                }
                break;

            case '5':
                BACKDOOR_MGR_Printf("\r\n1. ip domain-name.");
                BACKDOOR_MGR_Printf("\r\n2. no ip domain-name.");
                BACKDOOR_MGR_Printf("\r\nEnter your choice: ");
                *keyin = 0;
                BACKDOOR_MGR_RequestKeyIn(keyin, 1);
                BACKDOOR_MGR_Printf("\r\n");
                switch(*keyin)
                {
                    case '1':
                        BACKDOOR_MGR_Printf("\r\nInput domain name : ");
                        BACKDOOR_MGR_RequestKeyIn(keyin, 256);
                        BACKDOOR_MGR_Printf("\r\n");
                        strcpy((char *)hostname,keyin);
                        DNS_MGR_AddDomainName((char*)hostname);
                        break;
                    case '2':
                        DNS_MGR_DeleteDomainName();
                        break;
                    default:
                        BACKDOOR_MGR_Printf("\r\nChoice error!!");
                        break;
                }
                break;

            case '6':
                BACKDOOR_MGR_Printf("\r\n1. ip name-servert.");
                BACKDOOR_MGR_Printf("\r\n2. no ip name-server.");
                BACKDOOR_MGR_Printf("\r\nEnter your choice: ");
                *keyin = 0;
                BACKDOOR_MGR_RequestKeyIn(keyin, 1);
                BACKDOOR_MGR_Printf("\r\n");
                switch(*keyin)
                {
                    case '1':
                        BACKDOOR_MGR_Printf("\r\nInput name server IP : ");
                        BACKDOOR_MGR_RequestKeyIn(keyin, L_INET_MAX_IPADDR_STR_LEN+1);
                        BACKDOOR_MGR_Printf("\r\n");
                        strcpy((char *)hostaddr,keyin);

                        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC,
                                                                           hostaddr,
                                                                           (L_INET_Addr_T *)&ipadd,
                                                                           sizeof(ipadd)))
                        {
                            BACKDOOR_MGR_Printf("Convert addr String to Inaddr fail\n");
                        }

                        DNS_MGR_AddNameServer(&ipadd);
                        break;
                    case '2':
                        BACKDOOR_MGR_Printf("\r\nInput name server IP : ");
                        BACKDOOR_MGR_RequestKeyIn(keyin, L_INET_MAX_IPADDR_STR_LEN+1);
                        BACKDOOR_MGR_Printf("\r\n");
                        strcpy((char *)hostaddr,keyin);

                        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC,
                                                                           hostaddr,
                                                                           (L_INET_Addr_T *)&ipadd,
                                                                           sizeof(ipadd)))
                        {
                            BACKDOOR_MGR_Printf("Convert addr String to Inaddr fail\n");
                        }

                        DNS_MGR_DeleteNameServer(&ipadd);
                        break;
                    default:
                        BACKDOOR_MGR_Printf("\r\nChoice error!!");
                        break;
                }
                break;

            case '7':
                DNS_MGR_HostShow();
                break;

            case '8':
                DNS_MGR_ClearDnsCache();
                break;

            case '9':
                DNS_MGR_ShowDnsConfig();
                break;

            case 'a':
                DNS_MGR_ShowDnsDatabase();
                break;

            case 'b':
                DNS_MGR_ShowDnsCache();
                break;

            case 'c':
                BACKDOOR_MGR_Printf("\r\n1. debug ip dns.");
                BACKDOOR_MGR_Printf("\r\n2. no debug ip dns.");
                BACKDOOR_MGR_Printf("\r\nEnter your choice: ");
                *keyin = 0;
                BACKDOOR_MGR_RequestKeyIn(keyin, 1);
                BACKDOOR_MGR_Printf("\r\n");
                switch(*keyin)
                {
                    case '1':
                        DNS_MGR_DebugOpen();
                        break;
                    case '2':
                        DNS_MGR_DebugClose();
                        break;
                    default:
                        BACKDOOR_MGR_Printf("\r\nChoice error!!");
                        break;
                }
                break;

            case 'd':
            {
                UI32_T family = 0;
                int i =0;
                L_INET_AddrIp_T host_ip[MAXHOSTIPNUM];

                BACKDOOR_MGR_Printf("\r\nInput hostname : ");
                BACKDOOR_MGR_RequestKeyIn(keyin, 256);
                BACKDOOR_MGR_Printf("\r\n");
                strcpy((char *)hostname,keyin);

                BACKDOOR_MGR_Printf("\r\n1. AF_INET");
                BACKDOOR_MGR_Printf("\r\n2. AF_INET6.");
                BACKDOOR_MGR_Printf("\r\n3. AF_UNSPEC.");
                BACKDOOR_MGR_Printf("\r\nEnter your choice: ");
                *keyin = 0;
                BACKDOOR_MGR_RequestKeyIn(keyin, 1);
                BACKDOOR_MGR_Printf("\r\n");

                switch(*keyin)
                {
                    case '1':
                        family = AF_INET;
                        break;
                    case '2':
                        family = AF_INET6;
                        break;
                    case '3':
                        family = AF_UNSPEC;
                        break;
                    default:
                        BACKDOOR_MGR_Printf("\r\nChoice error!!");
                        break;
                }
                memset(host_ip, 0, sizeof(host_ip));

                if ( (rc = DNS_MGR_HostNameToIp(hostname, family, host_ip)) == 0 )
                {
                    BACKDOOR_MGR_Printf("DNS_MGR_HostNameToIp ok\r\n");

                    for(i=0;i<MAXHOSTIPNUM;i++)
                    {
                        memset(hostaddr, 0, sizeof(hostaddr));
                        if(host_ip[i].addrlen != 0)
                        {
                            if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&host_ip[i],
                                                                               hostaddr,
                                                                               sizeof(hostaddr)))
                            {
                                BACKDOOR_MGR_Printf("L_INET_InaddrToString Fail\n");
                            }
                            BACKDOOR_MGR_Printf("\tinterent address: %s\r\n",hostaddr);
                        }
                    }
                }
                else
                {
                    BACKDOOR_MGR_Printf("DNS_MGR_HostNameToIp fail, rc=%ld\r\n", (long)rc);
                }

                }

                break;

            case 'e':
#if 0
                BACKDOOR_MGR_Printf("\r\nInput hostname or ip : ");
                BACKDOOR_MGR_RequestKeyIn(keyin, 256);
                BACKDOOR_MGR_Printf("\r\n");
                strcpy(hostname,keyin);
                if ( (rc = DNS_MGR_HostNameToIp(hostname,&ipadd)) == 0 )
                {
                    BACKDOOR_MGR_Printf("DNS_MGR_HostNameToIp ok\r\n");
                }
                else
                {
                    BACKDOOR_MGR_Printf("DNS_MGR_HostNameToIp fail, rc = %ld\r\n",rc);
                }
                L_INET_Ntoa(ipadd,hostaddr);
                BACKDOOR_MGR_Printf("\tinterent address: %s\r\n",hostaddr);
#endif
                break;

            case 'f':
#if 0
                BACKDOOR_MGR_Printf("\r\nInput index : ");
                BACKDOOR_MGR_RequestKeyIn(keyin, 20);
                BACKDOOR_MGR_Printf("\r\n");
                strcpy(hostaddr,keyin);
                BACKDOOR_MGR_Printf("Input domainname : ");
                BACKDOOR_MGR_RequestKeyIn(keyin, 256);
                BACKDOOR_MGR_Printf("\r\n");
                strcpy(hostname,keyin);

                if (TRUE != DNS_OM_SetDomainNameListEntry(atoi(hostaddr), hostname))
                    {
                    BACKDOOR_MGR_Printf(" failed\r\n");
                }
#endif
                break;
            case 'g':
#if 0
                {
                    UI32_T  idx =0;

                    BACKDOOR_MGR_Printf("\r\n DNS_OM_GetNextDomainNameListEntry");
                    while (TRUE == DNS_OM_GetNextDomainNameListEntry
                                    (&idx, hostname))
                    {
                        BACKDOOR_MGR_Printf("\r\n %d - %s", idx, hostname);
                    }
                }
#endif
                break;
            case 'h':
#if 0
                BACKDOOR_MGR_Printf("\r\nInput index : ");
                BACKDOOR_MGR_RequestKeyIn(keyin, 20);
                BACKDOOR_MGR_Printf("\r\n");
                strcpy(hostaddr,keyin);
                BACKDOOR_MGR_Printf("Input hostname : ");
                BACKDOOR_MGR_RequestKeyIn(keyin, 256);
                BACKDOOR_MGR_Printf("\r\n");
                strcpy(hostname,keyin);

                if (TRUE != DNS_MGR_SetDnsHostEntry(atoi(hostaddr), hostname))
                {
                    BACKDOOR_MGR_Printf(" failed\r\n");
                }
#endif
                break;
            case 'i':
#if 0
                {
                    UI32_T  idx =0;
                    HostEntry_T host;
                    memset(&host, 0, sizeof(host));

                    BACKDOOR_MGR_Printf("\r\n DNS_HOSTLIB_GetNextDnsHostEntry");
                    while (TRUE == DNS_MGR_GetNextDnsHostEntry
                                    (&idx, &host))
                {
                    BACKDOOR_MGR_Printf("\r\n %d - %s", idx, hostname);
                }
                }
#endif
                break;

            case 'j':
#if 0
                {

                    UI32_T  idx =0, ip, is_add;

                    BACKDOOR_MGR_Printf("\r\nInput host index : ");
                    BACKDOOR_MGR_RequestKeyIn(keyin, 20);
                    BACKDOOR_MGR_Printf("\r\n");
                    idx = atoi(keyin);

                    BACKDOOR_MGR_Printf("Input addr : ");
                    BACKDOOR_MGR_RequestKeyIn(keyin, 20);
                    BACKDOOR_MGR_Printf("\r\n");
                    L_INET_Aton(keyin, &ip);

                    BACKDOOR_MGR_Printf("Add(1)/Del(0) : ");
                    BACKDOOR_MGR_RequestKeyIn(keyin, 20);
                    BACKDOOR_MGR_Printf("\r\n");
                    is_add = atoi(keyin);

                    BACKDOOR_MGR_Printf("\r\n DNS_HOSTLIB_SetDnsHostAddrEntry");
                    if (TRUE != DNS_MGR_SetDnsHostAddrEntry(idx, 1, (UI8_T *) &ip, (is_add>0)))
                {
                        BACKDOOR_MGR_Printf(" failed\r\n");
                    }
                }
#endif
                break;

            case 'k':
#if 0
                {

                    UI32_T  hidx =0, addr_type, addr;

                    BACKDOOR_MGR_Printf("\r\n DNS_HOSTLIB_GetNextDnsHostEntry");
                    while (TRUE == DNS_MGR_GetNextDnsHostAddrEntry
                                    (&hidx, &addr_type, &addr))
                    {
                        if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&addr_str,
                                                                           hostaddr,
                                                                           sizeof(hostaddr)))
                        {
                            BACKDOOR_MGR_Printf("L_INET_InaddrToString Fail\n");  /* pgr0695, return value of statement block in macro */
                        }
                        BACKDOOR_PMGR_Printf("\r\n i/ip %d/%s", hidx, hostaddr);  /* pgr0695, return value of statement block in macro */
                    }
                }
#endif
                break;

            case 'u':
#if (DNS_MGR_UNIT_TEST == TRUE)
            {
                DNS_MGR_UT_UnitTestMain();
}
#else
            {
                BACKDOOR_MGR_Printf("DNS_MGR_UNIT_TEST == FALSE");
}
#endif
                break;

            default:
                continue;
      }
      BACKDOOR_MGR_Printf("\r\n\r\n\r\n\r\n");
      BACKDOOR_MGR_Printf("---------------------------\r\n");
   }

}

/* FUNCTION NAME:  DNS_MGR_HostNameToIp
 * PURPOSE:
 *          This function is get host ip from host name.
 *
 * INPUT:
 *          UI8_T   *hostname   --  string of hostname.
 *          int family -- spec AF_INET(V4), AF_INET6(V6), AF_UNSPEC(V4/V6)
 *
 * OUTPUT:
 *          L_INET_AddrIp_T hostip[] --  host address.
 *
 * RETURN:
 *          DNS_OK/DNS_ERROR
 *
 * NOTES:
 *          .
 */
int DNS_MGR_HostNameToIp(UI8_T *hostname, UI32_T af_family, L_INET_AddrIp_T hostip_ar[])
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  rc = DNS_ERROR;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return DNS_ERROR;
    }
    else
    {
        if ( DNS_MGR_GetDnsStatus() == DNS_DISABLE )
        {
            return DNS_ERROR;
        }

        /*First, search HostTable*/
        rc = DNS_RESOLVER_SearchHostTable(hostname, af_family, hostip_ar);
        if(rc == DNS_OK)
        {
            return rc;
        }

        /*Second, search cache*/
        rc = DNS_RESOLVER_GetHostByName(hostname, af_family, DNS_SEARCH_CACHE, hostip_ar);
        if(rc == DNS_OK)
        {
            return rc;
        }

        /*Third, send query*/
        rc = DNS_RESOLVER_GetHostByName(hostname, af_family, DNS_SEND_QUERY, hostip_ar);
        if(rc == DNS_OK)
        {
            return rc;
        }

        return rc;
    }
}

/* FUNCTION NAME : DNS_RESOLVER_ExtractIp()
 * PURPOSE:
 *      This function extract ip from ip_addr list of hostent.
 *
 * INPUT:
 *      I8_T**   ip_addr_list   --  ip address list.
 *           UI32_T* hostip           -- 0       : means get first ip in list
 *                                                others: get next ip which found
 *
 * OUTPUT:
 *      UI32_T* hostip       --  ip address.
 *
 * RETURN:
 *      TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *
 *
 */
#if 0
static BOOL_T DNS_RESOLVER_ExtractIp(UI32_T* hostip,I8_T** ip_addr_list)
{
    I8_T ipaddr[20];
    //UI32_T**  ip;
    UI32_T *ip;
    int k = 0;

    if((hostip == NULL) || (ip_addr_list == NULL))
        return DNS_ERROR;

    /*Search part*/
    while( (ip=(UI32_T *)ip_addr_list[k]) != NULL )
    {
         k++;
         L_INET_Ntoa(*ip,(UI8_T *)ipaddr);
         if ( strcmp((char *)ipaddr, "255.255.255.255") == 0 )
         {
             continue;
         }
         if(*hostip == 0)
         {
           *hostip = *ip;
           return DNS_OK;
         }
         if(*hostip == *ip)
         {

            if((ip=(UI32_T *)ip_addr_list[k]) != NULL)
            {
                *hostip = *ip;
                return DNS_OK;
            }
            else
            {
                 ip=(UI32_T *)ip_addr_list[k-1];
                 *hostip = *ip;
                 return DNS_OK;
            }
         }
    }
    /*
    while( (ip=(UI32_T *)ip_addr_list[i]) != NULL )
    {

         i++;
         L_INET_Ntoa(*ip,ipaddr);
         if ( strcmp(ipaddr, "255.255.255.255") == 0 )
         {
             continue;
         }
         if(*hostip == *ip)
         {
            printf("Get next ip case\n");
            continue;
         }
         *hostip = *ip;
         return DNS_OK;
    }
    */
    return DNS_ERROR;

}
#endif

/* FUNCTION NAME : DNS_MGR_GetNextDomainNameList
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
 *      This function will be called by CLI aommand "show dns"
 */
BOOL_T DNS_MGR_GetNextDomainNameList(char *dns_ip_domain_name)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;
    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetNextDomainNameList(dns_ip_domain_name);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



/* FUNCTION NAME : DNS_MGR_GetDomainNameList
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
 *      This function will be called by snmp module.
 */
BOOL_T DNS_MGR_GetDomainNameList(I8_T *dns_ip_domain_name)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;
    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetDomainNameList(dns_ip_domain_name);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



/* FUNCTION NAME : DNS_MGR_GetNextNameServerList
 * PURPOSE:
 *      This function get next the domain name server from list.
 *
 *
 * INPUT:
 *      L_INET_AddrIp_T *ip_p  --  current doamin name server ip.
 *
 * OUTPUT:
 *      L_INET_AddrIp_T *ip_p  --  next doamin name server ip.
 *
 * RETURN:
 *      TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *      The initial ip value is zero.
 *      This function will be called by CLI command "show dns"
 */
BOOL_T DNS_MGR_GetNextNameServerList(L_INET_AddrIp_T *ip_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;
    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret= DNS_OM_GetNextNameServerList(ip_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



/* FUNCTION NAME : DNS_MGR_GetNameServerList
 * PURPOSE:
 *      This function get the domain name server from list.
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
 *      This function will be called by snmp module.
 */
BOOL_T DNS_MGR_GetNameServerList(L_INET_AddrIp_T *ip)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;
    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_GetNameServerList(ip);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



/* FUNCTION NAME : DNS_MGR_GetNextCacheEntry
 * PURPOSE:
 *      This function get next entry from dns cache.
 *
 *
 * INPUT:
 *      UI32_T *index   --  current index of cache entry.
 *
 * OUTPUT:
 *      UI32_T *index                  --  next index of cache entry.
 *      DNS_CacheRecord_T *cache_entry   --  next cache entry.
 *
 * RETURN:
 *      TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *      The initial index value is -1.
 *      This function will be called by CLI command "show dns cache"
 */
BOOL_T DNS_MGR_GetNextCacheEntry(I32_T *index, DNS_CacheRecord_T *cache_entry)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;
    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_CACHE_GetNextCacheEntry(index, cache_entry);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



/* FUNCTION NAME : DNS_MGR_GetNextCacheEntryForSNMP
 * PURPOSE:
 *      This function get next entry from dns cache.
 *
 *
 * INPUT:
 *      UI32_T *index   --  current index of cache entry.
 *
 * OUTPUT:
 *      UI32_T *index                  --  next index of cache entry.
 *      DNS_CacheRecord_T *cache_entry   --  next cache entry.
 *
 * RETURN:
 *      TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *      The initial index value is 0.
 *      This function will be used by SNMP.
 */
BOOL_T DNS_MGR_GetNextCacheEntryForSNMP(I32_T *index, DNS_CacheRecord_T *cache_entry)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;
    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        (*index)--;
        ret = DNS_CACHE_GetNextCacheEntry(index, cache_entry);
        (*index)++;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetCacheEntryForSNMP
 * PURPOSE:
 *      This function get entry from dns cache.
 *
 *
 * INPUT:
 *      UI32_T index   --  current index of cache entry.
 *
 * OUTPUT:
 *      DNS_CacheRecord_T *cache_entry   --  current cache entry.
 *
 * RETURN:
 *      TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *      This function will be called by snmp module, initial index=0.
 */
BOOL_T DNS_MGR_GetCacheEntryForSNMP(I32_T index, DNS_CacheRecord_T *cache_entry)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        if( index < 0 )
        {
            SYSFUN_RELEASE_CSC();
            return ret;
        }
        index--;
        ret = DNS_CACHE_GetCacheEntryForSNMP(index, cache_entry);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



/* FUNCTION NAME:  DNS_MGR_GetRunningDnsStatus
 * PURPOSE:
 *          This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
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
SYS_TYPE_Get_Running_Cfg_T  DNS_MGR_GetRunningDnsStatus(UI32_T *state)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    SYSFUN_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        *state = DNS_MGR_GetDnsStatus();
        if ( *state != DNS_DEFAULT_STATUS )
        {
            SYSFUN_RELEASE_CSC();
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS ;
        }
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}



/* FUNCTION NAME:  DNS_MGR_GetNextRunningDnsHostEntry
 * PURPOSE:
 *          This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific Sshd Status with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          int *host_index_p -- previous active index of HostEntry_T struct.
 *
 * OUTPUT:
 *          int *host_index_p -- current active index of HostEntry_T struct.
 *          HostEntry_PTR *  -- a pointer to a HostEntry_T variable to store the returned value.
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 *          The initial value is -1.
 */
SYS_TYPE_Get_Running_Cfg_T  DNS_MGR_GetNextRunningDnsHostEntry(int *host_index_p,HostEntry_PTR dns_host_entry_t_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    SYSFUN_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        if ( DNS_OK == DNS_MGR_GetNextDnsHostEntry(host_index_p, dns_host_entry_t_p) )
        {
            SYSFUN_RELEASE_CSC();
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS ;
        }
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}



/* FUNCTION NAME:  DNS_MGR_GetNextRunningDomainNameList
 * PURPOSE:
 *          This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific Sshd Status with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          I8_T *dns_ip_domain_name    --  current dommain name of list.
 *
 * OUTPUT:
 *          I8_T *dns_ip_domain_name    --  next dommain name of list.
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 *          the initial name is empty string.
 */
SYS_TYPE_Get_Running_Cfg_T  DNS_MGR_GetNextRunningDomainNameList(char *dns_ip_domain_name)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    SYSFUN_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        if ( TRUE == DNS_MGR_GetNextDomainNameList(dns_ip_domain_name) )
        {
            SYSFUN_RELEASE_CSC();
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS ;
        }
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}



/* FUNCTION NAME:  DNS_MGR_GetNextRunningNameServerList
 * PURPOSE:
 *          This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific Sshd Status with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          L_INET_AddrIp_T *ip  --  current doamin name server ip.
 *
 * OUTPUT:
 *          L_INET_AddrIp_T *ip  --  next doamin name server ip.
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 *          The initial ip value is zero.
 */
SYS_TYPE_Get_Running_Cfg_T  DNS_MGR_GetNextRunningNameServerList(L_INET_AddrIp_T *ip_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    SYSFUN_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        if ( TRUE == DNS_MGR_GetNextNameServerList(ip_p) )
        {
            SYSFUN_RELEASE_CSC();
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS ;
        }
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}



/* FUNCTION NAME:  DNS_MGR_GetRunningDnsIpDomain
 * PURPOSE:
 *          This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
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
SYS_TYPE_Get_Running_Cfg_T  DNS_MGR_GetRunningDnsIpDomain(UI8_T *ipdomain)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    SYSFUN_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        if ( DNS_OK == DNS_MGR_GetDnsIpDomain((char *)ipdomain) )
        {
            SYSFUN_RELEASE_CSC();
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS ;
        }
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}



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
int DNS_MGR_SetDnsResConfigMaxCnames(UI32_T dns_resconfig_max_cnames_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_SetDnsResConfigMaxCnames(dns_resconfig_max_cnames_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



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
 *      DNS_OK :success
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
int DNS_MGR_SetDnsResConfigSbeltName(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_SetDnsResConfigSbeltName(dns_res_config_sbelt_entry_t_p);

        if (ret==DNS_OK)             /*modified by wiseway for semaphore bug 2002-10-27*/
        {
            DNS_RESOLVER_SbeltInit();
        }
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



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
 *      DNS_OK :success
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
int DNS_MGR_SetDnsResConfigSbeltRecursion(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_SetDnsResConfigSbeltRecursion(dns_res_config_sbelt_entry_t_p);

        if (ret==DNS_OK)             /*modified by wiseway for semaphore bug 2002-10-27*/
        {
            DNS_RESOLVER_SbeltInit();
        }
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



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
 *      DNS_OK :success
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
int DNS_MGR_SetDnsResConfigSbeltPref(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_SetDnsResConfigSbeltPref(dns_res_config_sbelt_entry_t_p);

        if (ret==DNS_OK)             /*modified by wiseway for semaphore bug 2002-10-27*/
        {
            DNS_RESOLVER_SbeltInit();
        }
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



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
 *      DNS_OK :success
 *
 * NOTES:
 *   This function will be called by snmp module.
 */
int DNS_MGR_SetDnsResConfigSbeltStatus(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_OM_SetDnsResConfigSbeltStatus(dns_res_config_sbelt_entry_t_p);

        if (ret==DNS_OK)             /*modified by wiseway for semaphore bug 2002-10-27*/
        {
            DNS_RESOLVER_SbeltInit();
        }
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



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
int DNS_MGR_GetDefaultDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        ret = DNS_OM_GetDefaultDnsResConfigSbeltEntry(dns_res_config_sbelt_entry_t_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



/* FUNCTION NAME : DNS_MGR_GetDnsHostEntryBySnmp
 * PURPOSE:
 *      This funciton get the specified  according to the index.
 *
 *
 * INPUT:
 *      I8_T   *hostname_p   --  current active host name.
 *      I8_T   *hostaddr_p   --  current active host addr in standard Internet format
 *
 * OUTPUT:
 *      none.
 *
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK    :success.
 * NOTES:
 *      This function will be called by snmp
 */
int DNS_MGR_GetDnsHostEntryBySnmp(char *hostname_p, char *hostaddr_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_HOSTLIB_GetHostEntryBySnmp(hostname_p, hostaddr_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



/* FUNCTION NAME : DNS_MGR_GetNextDnsHostEntryBySnmp
 * PURPOSE:
 *      This funciton get next the specified  according to the index.
 *
 *
 * INPUT:
 *      I8_T   *hostname_p   --  current active host name.
 *      I8_T   *hostaddr_p   --  current active host addr in standard Internet format.
 *
 * OUTPUT:
 *      I8_T   *hostname_p   --  next active host name.
 *      I8_T   *hostaddr_p   --  next active host addr in standard Internet format.
 *
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK    :success.
 * NOTES:
 *      This function will be called by snmp
 *          the initial name is empty string.
 *          The initial ip value is 0.0.0.0.
 */
int DNS_MGR_GetNextDnsHostEntryBySnmp(UI8_T *hostname_p, UI8_T *hostaddr_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_HOSTLIB_GetNextHostEntryBySnmp(hostname_p, hostaddr_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



/* FUNCTION NAME : DNS_MGR_GetDnsAliasNameBySnmp
 * PURPOSE:
 *      This funciton get the specified  according to the index.
 *
 *
 * INPUT:
 *      I8_T   *hostname_p   --  current active host name.
 *      I8_T   *aliasname_p  --  current active alias name.
 *
 * OUTPUT:
 *      none.
 *
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK    :success.
 * NOTES:
 *      This function will be called by snmp
 */
int DNS_MGR_GetDnsAliasNameBySnmp(I8_T *hostname_p, I8_T *aliasname_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_HOSTLIB_GetAliasNameBySnmp(hostname_p, aliasname_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



/* FUNCTION NAME : DNS_MGR_GetNextDnsAliasNameBySnmp
 * PURPOSE:
 *      This funciton get next the specified  according to the index.
 *
 *
 * INPUT:
 *      I8_T   *hostname_p   --  current active host name.
 *      I8_T   *aliasname_p  --  current active alias name.
 *
 * OUTPUT:
 *      I8_T   *hostname_p   --  next active host name.
 *      I8_T   *aliasname_p  --  next active alias name.
 *
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK    :success.
 * NOTES:
 *      This function will be called by snmp
 *          the initial name is empty string.
 */
int DNS_MGR_GetNextDnsAliasNameBySnmp(I8_T *hostname_p, I8_T *aliasname_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_HOSTLIB_GetNextAliasNameBySnmp(hostname_p, aliasname_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



/* FUNCTION NAME : DNS_MGR_SetNameServerByIndex
 * PURPOSE:
 *  This function adds a name server IP address to the name server list by index.
 *
 *
 * INPUT:
 *      index   --  index of name server.
 *      ip_addr --  ip addr will be added as a name server;
 *      is_add  --  add or delete name server
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
int DNS_MGR_SetNameServerByIndex(UI32_T index, L_INET_AddrIp_T *ip_addr, BOOL_T is_add)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return ret;
    }
    else
    {
        if( index < 1 || index > SYS_ADPT_DNS_MAX_NBR_OF_NAME_SERVER_TABLE_SIZE )
        {
            return ret;
        }

        if(is_add == TRUE)
        {
          /*maggie liu for ES4827G-FLF-00346, 2008-11-26 */
            if(DNS_MGR_CheckNameServerIp(ip_addr) == TRUE)
            {
                ret = DNS_OM_SetNameServerByIndex(index, ip_addr);
                if (ret==DNS_OK)
                {
                    DNS_RESOLVER_SbeltInit();
                }
            }
        }
        else
        {
            /*maggie liu for es4827g-FLF-00244*/
            ret = DNS_OM_DeleteNameServerByIndex(index);
            if (ret==DNS_OK)
            {
                DNS_RESOLVER_SbeltInit();
            }
        }
    }
    return ret;
}



/* FUNCTION NAME : DNS_MGR_GetNameServerByIndex
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
BOOL_T DNS_MGR_GetNameServerByIndex(UI32_T index, L_INET_AddrIp_T *ip)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;
    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        if( index < 1 || index > SYS_ADPT_DNS_MAX_NBR_OF_NAME_SERVER_TABLE_SIZE )
        {
            SYSFUN_RELEASE_CSC();
            return ret;
        }

        ret = DNS_OM_GetNameServerByIndex(index, ip);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



/* FUNCTION NAME : DNS_MGR_GetNextNameServerByIndex
 * PURPOSE:
 *      This function get next the domain name server by indx from list.
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
 *      TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *      The initial ip value is zero.
 *      This function will be called by SNMP.
 */
BOOL_T DNS_MGR_GetNextNameServerByIndex(UI32_T *index, L_INET_AddrIp_T *ip)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;
    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        *index = *index + 1 ;
        ret = DNS_MGR_GetNameServerByIndex(*index, ip);
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



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
int DNS_MGR_DeleteAllNameServer(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        /*maggie liu, ES4827G-FLF-ZZ-00244*/
        ret = DNS_OM_DeleteNameServerAll();
        if (ret==DNS_OK)
        {
            DNS_RESOLVER_SbeltInit();
        }
    }
    SYSFUN_RELEASE_CSC();
    return ret;
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
#if 0  /*!*/ /* PENDING: DNS_MGR_*DnsHostEntryByNameAndIndex; suspected unused code */
/* FUNCTION NAME : DNS_MGR_GetNextDnsHostEntryByNameAndIndex
 * PURPOSE:
 *      This funciton get next the specified  according to the index.
 *
 *
 * INPUT:
 *      I8_T    *hostname_p --  current active host name.
 *      I32_T   *index_p    --  current active index of host addr for host name.
 *      I8_T    *hostaddr_p --  current active host addr in standard Internet format.
 *
 * OUTPUT:
 *      I8_T    *hostname_p --  next active host name.
 *      I32_T   *index_p    --  next active index of host addr for host name.
 *      I8_T    *hostaddr_p --  next active host addr in standard Internet format.
 *
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK    :success.
 * NOTES:
 *      This function will be called by snmp
 *          the initial name is empty string.
 *          The initial index is 0.
 *          The initial ip value is 0.0.0.0.
 */
int DNS_MGR_GetNextDnsHostEntryByNameAndIndex(char *hostname_p, I32_T *index_p, char *hostaddr_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_HOSTLIB_GetNextDnsHostEntryByNameAndIndex(hostname_p, index_p, hostaddr_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



/* FUNCTION NAME : DNS_MGR_SetDnsHostEntryByNameAndIndex
 * PURPOSE:
 *  This function set a IP address to the host table list by host name & index.
 *
 *
 * INPUT:
 *      *hostname_p --  host name.
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
 *      This function will be called by SNMP.
 */
int DNS_MGR_SetDnsHostEntryByNameAndIndex(char *hostname_p, UI32_T index, L_INET_AddrIp_T *ip_addr)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        if( index < 1 || index > SYS_ADPT_MAX_NBR_OF_DNS_HOST_IP )
        {
            SYSFUN_RELEASE_CSC();
            return ret;
        }
        if( ip_addr->addrlen != 0 )
        {

            ret = DNS_HOSTLIB_SetDnsHostEntryByNameAndIndex(hostname_p, index, ip_addr);

        }
        else
        {
             /*maggie liu, ES4827G-FLF-ZZ-00243*/
            ret = DNS_HOSTLIB_DeleteDnsHostEntryByNameAndIndex(hostname_p, index);

        }
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}



/* FUNCTION NAME : DNS_MGR_GetDnsHostEntryByNameAndIndex
 * PURPOSE:
 *  This function get a IP address to the host table list by host name & index.
 *
 *
 * INPUT:
 *      I8_T    *hostname_p --  host name.
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
int DNS_MGR_GetDnsHostEntryByNameAndIndex(char *hostname_p, UI32_T index, char *hostaddr_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {
        if( index < 1 || index > SYS_ADPT_MAX_NBR_OF_DNS_HOST_IP )
        {
            SYSFUN_RELEASE_CSC();
            return ret;
        }

        ret = DNS_HOSTLIB_GetDnsHostEntryByNameAndIndex(hostname_p, index, hostaddr_p);

    }
    SYSFUN_RELEASE_CSC();
    return ret;
}
#endif  /*!*/ /* 0; PENDING: DNS_MGR_*DnsHostEntryByNameAndIndex; suspected unused code */



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
BOOL_T DNS_MGR_SetResResetStatus(int status)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        DNS_OM_SetResResetStatus(status);

        ret = TRUE;
    }
    SYSFUN_RELEASE_CSC();
    return ret;
}

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

int DNS_MGR_GetNextLookupCtlTable(DNS_Nslookup_CTRL_T *CTRL_table)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return DNS_ERROR;
    }

    if (NULL == CTRL_table)
    {
        return DNS_ERROR;
    }

    while (DNS_OK == DNS_OM_GetNextLookupCtlTable(CTRL_table))
    {
        if (TRUE == DNS_MGR_IS_SYSTEM_OWNER_INDEX(CTRL_table->CtlOwnerIndex))
        {
            if (TRUE == DNS_MGR_IsNslookupCtlEntryAgeOut(CTRL_table))
            {
                continue;
            }
        }

        return DNS_OK;
    }

    return DNS_ERROR;
}

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

int DNS_MGR_GetLookupCtlTable(DNS_Nslookup_CTRL_T *CTRL_table)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return DNS_ERROR;
    }

    if (NULL == CTRL_table)
    {
        return DNS_ERROR;
    }

    if (DNS_OK == DNS_OM_GetLookupCtlTable(CTRL_table))
    {
        if (TRUE == DNS_MGR_IS_SYSTEM_OWNER_INDEX(CTRL_table->CtlOwnerIndex))
        {
            if (TRUE == DNS_MGR_IsNslookupCtlEntryAgeOut(CTRL_table))
            {
                return DNS_ERROR;
            }
        }

        return DNS_OK;
    }

    return DNS_ERROR;

}

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

int DNS_MGR_SetDNSCtlTable_TargetAddressType(DNS_Nslookup_CTRL_T *CTRL_table)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return DNS_ERROR;
    }

    if (NULL == CTRL_table)
    {
        return DNS_ERROR;
    }

    if (TRUE == DNS_MGR_IS_SYSTEM_OWNER_INDEX(CTRL_table->CtlOwnerIndex))
    {
        return DNS_ERROR;
    }

    if (VAL_dnsCtlTargetAddressType_dns != CTRL_table->TargetAddressType)
    {
        return DNS_ERROR;
    }

    if (DNS_OK != DNS_OM_SetDNSCtlTable_TargetAddressType(CTRL_table))
    {
        return DNS_ERROR;
    }

    return DNS_OK;
}

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

int DNS_MGR_SetDNSCtlTable_TargetAddress(DNS_Nslookup_CTRL_T *CTRL_table)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return DNS_ERROR;
    }

    if (NULL == CTRL_table)
    {
        return DNS_ERROR;
    }

    if (TRUE == DNS_MGR_IS_SYSTEM_OWNER_INDEX(CTRL_table->CtlOwnerIndex))
    {
        return DNS_ERROR;
    }

    if (('\0' == CTRL_table->TargetAddress[0]) ||
        (DNS_MAX_NAME_LENGTH < CTRL_table->TargetAddressLen))
    {
        return DNS_ERROR;
    }

    if (DNS_OK != DNS_OM_SetDNSCtlTable_TargetAddress(CTRL_table))
    {
        return DNS_ERROR;
    }

    return DNS_OK;
}

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
int DNS_MGR_SetDNSCtlTable_RowStatus(DNS_Nslookup_CTRL_T *CTRL_table)
{
    int  ret = DNS_ERROR;
    UI32_T  rowstatus_result, fsm_rowstatus, user_rowstatus;
    UI32_T  index = DNS_DEF_NSLOOKUP_REQUEST;
    DNS_Nslookup_CTRL_T om_str;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return ret;
    }
    else
    {
        if (CTRL_table == NULL)
        {
            return ret;
        }

        /* user can't use same CtlOwnerIndex string as system
         */
        if (TRUE == DNS_MGR_IS_SYSTEM_OWNER_INDEX(CTRL_table->CtlOwnerIndex))
        {
            return ret;
        }

        /* try to get entry
         */
        memset(&om_str, 0, sizeof(om_str));
        memcpy(&om_str, CTRL_table, sizeof(DNS_Nslookup_CTRL_Head_T));

        if (DNS_OM_GetLookupCtlTableAndIndex(&om_str, &index) == DNS_OK)
        {
            fsm_rowstatus = om_str.RowStatus;
        }
        else
        {
            fsm_rowstatus = L_RSTATUS_NOT_EXIST;
        }

        /* get user status
         */
        user_rowstatus = CTRL_table->RowStatus;

        /* get FSM result
         */
        rowstatus_result = L_RSTATUS_Fsm(user_rowstatus, &fsm_rowstatus,
                                DNS_Nslookup_TargetAddrCheck, (void *) & om_str.TargetAddressLen);

        /* follow FSM result
         */
        switch (rowstatus_result)
        {
            /* does not exist to does not exist
             */
            case L_RSTATUS_NOTEXIST_2_NOTEXIST:
                ret = DNS_OK;
                break;

            /* does not exist to not ready
             */
            case L_RSTATUS_NOTEXIST_2_NOTREADY:
                /* create new entry and remember the index
                 */
                CTRL_table->af_family = AF_UNSPEC;
                CTRL_table->RowStatus = fsm_rowstatus;

                if (DNS_OK == DNS_MGR_CreateNslookupCtlEntry(CTRL_table, &index))
                {
                    ret = DNS_OK;
                }
                else
                {
                    ret = DNS_ERROR;
                }
                break;

            /* does not exist to active
             */
            case L_RSTATUS_NOTEXIST_2_ACTIVE:
                ret = DNS_ERROR;
                break;

            /* active or not ready to does not exist
             */
            case L_RSTATUS_ACTIVE_2_NOTEXIST:
            case L_RSTATUS_NOTREADY_2_NOTEXIST:
                ret = DNS_OM_Nslookup_DeleteEntry(index);
                break;

            /* not ready or active to not ready
             */
            case L_RSTATUS_NOTREADY_2_NOTREADY:
            case L_RSTATUS_ACTIVE_2_NOTREADY:
                DNS_OM_SetLookupCtlRowStatusByIndex(index, fsm_rowstatus);
                ret = DNS_OK;
                break;

            /* not ready to active
             */
            case L_RSTATUS_NOTREADY_2_ACTIVE:
                DNS_OM_SetLookupCtlRowStatusByIndex(index, fsm_rowstatus);
                ret = DNS_MGR_AsyncStartNslookup(index);
                break;

            /* active to active
             */
            case L_RSTATUS_ACTIVE_2_ACTIVE:
                DNS_OM_SetLookupCtlRowStatusByIndex(index, fsm_rowstatus);
                ret = DNS_OK;
                break;

            /* not allowed or unrecognised transition
             */
            default:
                ret = DNS_ERROR;
                break;
        }
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_MGR_CreateSystemNslookupCtlEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Create nslookup control entry which control owner is system.
 * INPUT    : ctl_entry_p  -- nslookup control entry
 * OUTPUT   : ctl_index_p  -- 0-based nslookup control entry index
 * RETUEN   : DNS_OK/DNS_ERROR
 * NOTES    : If create entry success, it will start nslookup.
 * ------------------------------------------------------------------------
 */
int
DNS_MGR_CreateSystemNslookupCtlEntry(
    DNS_Nslookup_CTRL_T *ctl_entry_p,
    UI32_T *ctl_index_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return DNS_ERROR;
    }

    if ((NULL == ctl_entry_p) || (NULL == ctl_index_p))
    {
        return DNS_ERROR;
    }

    if (TRUE != DNS_MGR_IS_SYSTEM_OWNER_INDEX(ctl_entry_p->CtlOwnerIndex))
    {
        return DNS_ERROR;
    }

    if (('\0' == ctl_entry_p->TargetAddress[0]) ||
        (DNS_MAX_NAME_LENGTH < ctl_entry_p->TargetAddressLen))
    {
        return DNS_ERROR;
    }

    ctl_entry_p->RowStatus = VAL_dnsCtlRowStatus_active;

    if (DNS_OK != DNS_MGR_CreateNslookupCtlEntry(ctl_entry_p, ctl_index_p))
    {
        return DNS_ERROR;
    }

    if (DNS_OK != DNS_MGR_AsyncStartNslookup(*ctl_index_p))
    {
        DNS_OM_Nslookup_DeleteEntry(*ctl_index_p);
        return DNS_ERROR;
    }

    return DNS_OK;
}

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
    DNS_Nslookup_CTRL_T *ctl_entry_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return DNS_ERROR;
    }

    if (NULL == ctl_entry_p)
    {
        return DNS_ERROR;
    }

    if (DNS_OK == DNS_OM_GetNslookupCtlEntryByIndex(ctl_index, ctl_entry_p))
    {
        if (TRUE == DNS_MGR_IS_SYSTEM_OWNER_INDEX(ctl_entry_p->CtlOwnerIndex))
        {
            if (TRUE == DNS_MGR_IsNslookupCtlEntryAgeOut(ctl_entry_p))
            {
                return DNS_ERROR;
            }
        }

        return DNS_OK;
    }

    return DNS_ERROR;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_MGR_GetNslookupResultEntryByIndex
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
DNS_MGR_GetNslookupResultEntryByIndex(
    UI32_T ctl_index,
    UI32_T result_index,
    DNS_Nslookup_Result_T *results_entry_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return DNS_ERROR;
    }

    if (NULL == results_entry_p)
    {
        return DNS_ERROR;
    }

    if (DNS_OK == DNS_OM_GetNslookupResultEntryByIndex(ctl_index, result_index, results_entry_p))
    {
        if (TRUE == DNS_MGR_IS_SYSTEM_OWNER_INDEX(results_entry_p->CtlOwnerIndex))
        {
            if (TRUE == DNS_MGR_IsNslookupResultEntryAgeOut(results_entry_p))
            {
                return DNS_ERROR;
            }
        }

        return DNS_OK;
    }

    return DNS_ERROR;
}

/* FUNCTION NAME : DNS_MGR_GetNextLookupResultTable
 *
 * PURPOSE:
 *  This function get next Result table .
 *
 * INPUT:
 *  Result_table    --  Nslookup result table
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
int DNS_MGR_GetNextLookupResultTable(DNS_Nslookup_Result_T *Result_table)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return DNS_ERROR;
    }

    if (NULL == Result_table)
    {
        return DNS_ERROR;
    }

    while (DNS_OK == DNS_OM_GetNextLookupResultTable(Result_table))
    {
        if (TRUE == DNS_MGR_IS_SYSTEM_OWNER_INDEX(Result_table->CtlOwnerIndex))
        {
            if (TRUE == DNS_MGR_IsNslookupResultEntryAgeOut(Result_table))
            {
                continue;
            }
        }

        return DNS_OK;
    }

    return DNS_ERROR;
}

/* FUNCTION NAME : DNS_MGR_GetLookupResultTable
 *
 * PURPOSE:
 *  This function get next Result table .
 *
 * INPUT:
 *  Result_table    --  Nslookup result table
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

int DNS_MGR_GetLookupResultTable(DNS_Nslookup_Result_T *Result_table)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return DNS_ERROR;
    }

    if (NULL == Result_table)
    {
        return DNS_ERROR;
    }

    if (DNS_OK == DNS_OM_GetLookupResultTable(Result_table))
    {
        if (TRUE == DNS_MGR_IS_SYSTEM_OWNER_INDEX(Result_table->CtlOwnerIndex))
        {
            if (TRUE == DNS_MGR_IsNslookupResultEntryAgeOut(Result_table))
            {
                return DNS_ERROR;
            }
        }

        return DNS_OK;
    }

    return DNS_ERROR;
}

/* FUNCTION NAME : DNS_MGR_Nslookup_DeleteEntry
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
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *
 */
int DNS_MGR_Nslookup_DeleteEntry(UI32_T index)
{

    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return ret;
    }
    else
    {
        ret = DNS_OM_Nslookup_DeleteEntry(index);
    }

    return ret;
}


/* FUNCTION NAME : DNS_MGR_GetNslookupTimeOut
 * PURPOSE:
 *  This function get nslookup control and result tables time out.
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
 *  TRUE :failure,
 *  FALSE:success.
 * NOTES:
 *
 */
BOOL_T DNS_MGR_GetNslookupTimeOut(UI32_T *timeout, UI32_T index)
{

    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return ret;
    }
    else
    {
        DNS_OM_GetNslookupTimeOut(timeout, index);
        ret = TRUE;
    }

    return ret;
}

/* FUNCTION NAME : NS_MGR_GetNslookupPurgeTime
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
BOOL_T NS_MGR_GetNslookupPurgeTime(UI32_T *purge_time)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return ret;
    }
    else
    {
        DNS_OM_GetNslookupPurgeTime(purge_time);
        ret = TRUE;
    }

    return ret;
}

/* FUNCTION NAME : DNS_MGR_SetNslookupPurgeTime
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
BOOL_T DNS_MGR_SetNslookupPurgeTime(UI32_T purge_time)
{

    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return ret;
    }
    else
    {
        DNS_OM_SetNslookupPurgeTime(purge_time);
        ret = TRUE;
    }

    return ret;
}

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
void DNS_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    return;
}



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
void DNS_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    return;
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - DNS_MGR_CheckNameServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : check input address is in suitable range.
 * INPUT    : ipaddress
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------
 */
BOOL_T
DNS_MGR_CheckNameServerIp(
    L_INET_AddrIp_T *ipaddress_p)
{
    switch(ipaddress_p->type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
        {
            if (IP_LIB_OK != IP_LIB_IsValidForRemoteIp(ipaddress_p->addr))
            {
                return FALSE;
            }
        }
            break;

        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
        {
            /* IPv6 address should not be
             * IP_LIB_INVALID_IPV6_UNSPECIFIED
             * IP_LIB_INVALID_IPV6_LOOPBACK
             * IP_LIB_INVALID_IPV6_MULTICAST
             */
            if (IP_LIB_OK != IP_LIB_CheckIPv6PrefixForInterface(ipaddress_p->addr, SYS_ADPT_IPV6_ADDR_LEN))
            {
                return FALSE;
            }
        }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME : DNS_MGR_DeleteDnsCacheRR
 *
 * PURPOSE:
 *      This routine deletes a cache from the local cache.
 *
 * INPUT:
 *      I8_T * -- name
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_OK, of DNS_ERROR if not find the entry.
 *
 * NOTES:
 *      This function will be called by CLI command "clear host [name]"
 */
int DNS_MGR_DeleteDnsCacheRR(char * name)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int  ret = DNS_ERROR;

    /* BODY */
    SYSFUN_USE_CSC(ret);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return ret;
    }
    else
    {

        ret = DNS_CACHE_DelEntry(name);

    }
    SYSFUN_RELEASE_CSC();
    return ret;

}

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
BOOL_T DNS_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *ipcmsg_p)
{
    UI32_T cmd = DNS_MGR_MSG_CMD(ipcmsg_p);
    if(ipcmsg_p == NULL)
        return FALSE;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_IPC_RESULT_FAIL;
        ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        return TRUE;
    }

    switch(cmd)
    {
        case DNS_MGR_IPC_CMD_ADD_DOMAIN_NAME:
        {
            DNS_MGR_IPCMsg_Config_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_AddDomainName(data_p->DnsIpDomainName);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_ADD_DOMAIN_NAME_TO_LIST:
        {
            DNS_MGR_IPCMsg_IpDomain_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_AddDomainNameToList(data_p->DnsIpDomainName);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_ADD_NAME_SERVER:
        {
            DNS_MGR_IPCMsg_NAME_SERVER_Entry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_AddNameServer(&data_p->serveraddr);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_CLEAR_DNS_CACHE:
        {
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_ClearDnsCache();
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_CLEAR_HOSTS:
        {
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_ClearHosts();
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_DELETE_ALL_NAME_SERVER:
        {
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_DeleteAllNameServer();
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_DELETE_DOMAIN_NAME:
        {
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_DeleteDomainName();
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_DELETE_DOMAIN_NAME_FROM_LIST:
        {
            DNS_MGR_IPCMsg_IpDomain_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_DeleteDomainNameFromList(
              data_p->DnsIpDomainName);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case  DNS_MGR_IPC_CMD_DELETE_NAME_SERVER:
        {
            DNS_MGR_IPCMsg_NAME_SERVER_Entry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_DeleteNameServer(&data_p->serveraddr);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case  DNS_MGR_IPC_CMD_DISABLE_DOMAIN_LOOKUP:
        {
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_DisableDomainLookup();
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case  DNS_MGR_IPC_CMD_ENABLE_DOMAIN_LOOKUP:
        {
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_EnableDomainLookup();
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_GET_CACHE_ENTRY_FOR_SNMP:
        {
            DNS_MGR_IPCMsg_Cache_Record_T*data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetCacheEntryForSNMP(
               data_p->index,
               &data_p->cache);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Cache_Record_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_DEFAULT_DNS_RES_CONFIG_SBEL_ENTRY:
        {
            DNS_MGR_IPCMsg_Cache_Record_T*data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetCacheEntryForSNMP(data_p->index, &data_p->cache);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Cache_Record_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_ALIAS_NAME_BY_SNMP:
        {
           DNS_MGR_IPCMsg_AliasName_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsAliasNameBySnmp(
               data_p->hostname,
               data_p->aliasname);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_AliasName_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_HOST_ENTRY:
        {
           DNS_MGR_IPCMsg_HostEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsHostEntry(
               data_p->index,
               &data_p->host_entry);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HostEntry_T);
            break;
        }

#if 0  /*!*/ /* PENDING: DNS_MGR_*DnsHostEntryByNameAndIndex; suspected unused code */
        case DNS_MGR_IPC_CMD_GET_HOST_ENTRY_BY_NAME_AND_INDEX:
        {
           DNS_MGR_IPCMsg_HostEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsHostEntryByNameAndIndex(
               data_p->hostname,
               data_p->index,
               data_p->hostaddr);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HostEntry_T);
            break;
        }
#endif  /*!*/ /* 0; PENDING: DNS_MGR_*DnsHostEntryByNameAndIndex; suspected unused code */

        case  DNS_MGR_IPC_CMD_GET_HOST_ENTRY_BY_SNMP:
        {
            DNS_MGR_IPCMsg_HostEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsHostEntryBySnmp(
               data_p->hostname,
               data_p->hostaddr);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_GET_IP_DOMAIN:
        {
            DNS_MGR_IPCMsg_Config_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsIpDomain((char *)data_p->DnsIpDomainName);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T);
            break;
        }

        case  DNS_MGR_IPC_CMD_GET_LOCAL_MAX_REQUESTS:
        {
            DNS_MGR_IPCMsg_Config_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsLocalMaxRequests(&data_p->DnsMaxLocalRequests);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T);
            break;
        }

        case  DNS_MGR_IPC_CMD_GET_RES_BAD_CACHES:
        {
            DNS_MGR_IPCMsg_CacheConfig_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsResCacheBadCaches(&data_p->cache_bad_caches);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CacheConfig_T);
            break;
        }

        case  DNS_MGR_IPC_CMD_GET_RES_GOOD_CACHES:
        {
            DNS_MGR_IPCMsg_CacheConfig_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsResCacheGoodCaches(&data_p->cache_good_caches);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CacheConfig_T);
            break;
        }

        case  DNS_MGR_IPC_CMD_GET_DNS_RES_CACHE_MAX_TTL:
        {
            DNS_MGR_IPCMsg_CacheConfig_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsResCacheMaxTTL(&data_p->cache_max_ttl_32);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CacheConfig_T);
            break;
        }

        case  DNS_MGR_IPC_CMD_GET_DNS_RES_CACHE_STATUS:
        {
            DNS_MGR_IPCMsg_CacheConfig_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsResCacheStatus(&data_p->cache_status);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CacheConfig_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_DNS_RES_MAX_CNAMES:
        {
            DNS_MGR_IPCMsg_ResConfig_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsResConfigMaxCnames(&data_p->dnsResConfigMaxCnames);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfig_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_DNS_RES_CONFIG_RESET:
        {
            DNS_MGR_IPCMsg_CONFIG_RESET_TIME_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsResConfigReset(&data_p->config_reset_time);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CONFIG_RESET_TIME_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_DNS_RES_IMPLEMENTIDENT:
        {
            DNS_MGR_IPCMsg_ResConfig_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsResConfigImplementIdent(data_p->dnsResConfigImplementIdent);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfig_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_SERV_CONFIG_RESET:
        {
            // DNS_MGR_IPCMsg_CONFIG_RESET_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            // DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsServConfigReset(&data_p->config_reset);
            DNS_MGR_IPCMsg_ProxyConfig_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsServConfigReset(&data_p->dnsServConfigReset);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ProxyConfig_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_DNS_RES_RESET_TIME:
        {
            DNS_MGR_IPCMsg_CONFIG_RESET_TIME_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsResConfigResetTime(&data_p->config_reset_time);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CONFIG_RESET_TIME_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_DNS_RES_SERVICE:
        {
            DNS_MGR_IPCMsg_ResConfig_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsResConfigService(&data_p->dnsResConfigService);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfig_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_DNS_RES_SBEL_ENTRY:
        {
            DNS_MGR_IPCMsg_ResConfigSbeltEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsResConfigSbeltEntry(&data_p->data);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_CONFIG_UP_TIME:
        {
            DNS_MGR_IPCMsg_CONFIG_RESET_TIME_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsResConfigUpTime(&data_p->config_reset_time);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CONFIG_RESET_TIME_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_STATUS:
        {
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsStatus();
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_SET_STATUS:
        {
            DNS_MGR_IPCMsg_Config_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_SetDnsStatus(data_p->DnsStatus);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_SET_RES_CACHE_MAX_TTL:
        {
            DNS_MGR_IPCMsg_CacheConfig_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_SetDnsResCacheMaxTTL(&data_p->cache_max_ttl);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_SET_RES_CACHE_STATUS:
        {
            DNS_MGR_IPCMsg_CacheConfig_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_SetDnsResCacheStatus(&data_p->cache_status);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_SET_RES_MAX_CNAMES:
        {
            DNS_MGR_IPCMsg_ResConfig_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_SetDnsResConfigMaxCnames(data_p->dnsResConfigMaxCnames);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_SET_RES_RESET:
        {
            DNS_MGR_IPCMsg_ResConfig_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_SetDnsResConfigReset(&data_p->dnsResConfigReset);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_SET_RES_SBELT_ENTRY:
        {
            DNS_MGR_IPCMsg_ResConfigSbeltEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_SetDnsResConfigSbeltEntry(&data_p->data);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_SET_RES_SBELT_NAME:
        {
            DNS_MGR_IPCMsg_ResConfigSbeltEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_SetDnsResConfigSbeltName(&data_p->data);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_SET_RES_SBELT_PREF:
        {
            DNS_MGR_IPCMsg_ResConfigSbeltEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_SetDnsResConfigSbeltPref(&data_p->data);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_SET_RES_SBELT_RECURSION:
        {
            DNS_MGR_IPCMsg_ResConfigSbeltEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_SetDnsResConfigSbeltRecursion(&data_p->data);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_SET_RES_SBELT_STATUS:
        {
            DNS_MGR_IPCMsg_ResConfigSbeltEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_SetDnsResConfigSbeltStatus(&data_p->data);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_SET_SERV_CONFIG_RECURS:
        {
            DNS_MGR_IPCMsg_ProxyConfig_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_SetDnsServConfigRecurs(&data_p->dnsServConfigRecurs);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_SET_SERV_CONFIG_RESET:
        {
            DNS_MGR_IPCMsg_ProxyConfig_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_SetDnsServConfigReset(&data_p->dnsServConfigReset);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_GET_NEXT_CACHE_ENTRY:
        {
            DNS_MGR_IPCMsg_Cache_Record_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNextCacheEntry(
                 &data_p->index,
                 &data_p->cache);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Cache_Record_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_NEXT_CACHE_ENTRY_FOR_SNMP:
        {
            DNS_MGR_IPCMsg_Cache_Record_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNextCacheEntryForSNMP(
                 &data_p->index,
                 &data_p->cache);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Cache_Record_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_NEXT_ALIAS_NAME_BY_SNMP:
        {
            DNS_MGR_IPCMsg_AliasName_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNextDnsAliasNameBySnmp(
                 data_p->hostname,
                 data_p->aliasname);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_AliasName_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_NEXT_HOST_ENTRY:
        {
            DNS_MGR_IPCMsg_GetNextRunningHostEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNextDnsHostEntry(
                 &data_p->index,
                 &data_p->host_entry);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_GetNextRunningHostEntry_T);
            break;
        }

#if 0  /*!*/ /* PENDING: DNS_MGR_*DnsHostEntryByNameAndIndex; suspected unused code */
        case DNS_MGR_IPC_CMD_GET_NEXT_HOST_ENTRY_BY_NAME_INDEX:
        {
            DNS_MGR_IPCMsg_HostEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNextDnsHostEntryByNameAndIndex(
                 data_p->hostname,
                 &data_p->index,
                 data_p->hostaddr);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HostEntry_T);
            break;
        }
#endif  /*!*/ /* 0; PENDING: DNS_MGR_*DnsHostEntryByNameAndIndex; suspected unused code */

        case DNS_MGR_IPC_CMD_GET_NEXT_RUNNING_DOMAIN_NAME_LIST:
        {
            DNS_MGR_IPCMsg_Config_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNextRunningDomainNameList(data_p->DnsIpDomainName);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_NEXT_RUNNING_NAME_SERVER_LIST:
        {
            DNS_MGR_IPCMsg_ResConfigSbeltEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNextRunningNameServerList(&data_p->data.dnsResConfigSbeltAddr);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T);

            break;
        }

        case DNS_MGR_IPC_CMD_GET_RUNNING_DNS_IP_DOMAIN:
        {
            DNS_MGR_IPCMsg_Config_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetRunningDnsIpDomain((UI8_T *)data_p->DnsIpDomainName);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_RUNNING_DNS_STATUS:
        {
            DNS_MGR_IPCMsg_Config_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetRunningDnsStatus(&data_p->DnsStatus);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_NEXT_DOMAIN_NAME_LIST:
        {
            DNS_MGR_IPCMsg_Config_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNextDomainNameList(data_p->DnsIpDomainName);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Config_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_NEXT_RES_CONFIG_SBELT_ENTRY:
        {
            DNS_MGR_IPCMsg_ResConfigSbeltEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNextDnsResConfigSbeltEntry(&data_p->data);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResConfigSbeltEntry_T);
            break;
        }

        case DNS_MGR_IPC_CMD_HOST_ADD:
        {
            DNS_MGR_IPCMsg_HOST_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_HostAdd(data_p->hostname, &data_p->hostaddr);
            ipcmsg_p->msg_size =DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

#if 0  /*!*/ /* PENDING: DNS_MGR_*DnsHostEntryByNameAndIndex; suspected unused code */
        case DNS_MGR_IPC_CMD_SET_HOST_ENTRY_BY_NAME_AND_INDEX:
        {
            DNS_MGR_IPCMsg_Set_Host_Entry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_SetDnsHostEntryByNameAndIndex(
            data_p->hostname,
            data_p->index,
            &data_p->ip_addr);
            ipcmsg_p->msg_size =DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
#endif  /*!*/ /* 0; PENDING: DNS_MGR_*DnsHostEntryByNameAndIndex; suspected unused code */

        case DNS_MGR_IPC_CMD_GET_RES_COUNTER_BY_OPCODE_ENTRY:
        {
            DNS_MGR_IPCMsg_ResCounterByOpcodeEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsResCounterByOpcodeEntry(&data_p->data);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResCounterByOpcodeEntry_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_NEXT_RES_COUNTER_BY_OPCODE_ENTRY:
        {
            DNS_MGR_IPCMsg_ResCounterByOpcodeEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNextDnsResCounterByOpcodeEntry(&data_p->data);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResCounterByOpcodeEntry_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_NEXT_RES_COUNTER_BY_RCODE_ENTRY:
        {
            DNS_MGR_IPCMsg_ResCounterByRcodeEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNextDnsResCounterByRcodeEntry(&data_p->data);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ResCounterByRcodeEntry_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_NEXT_DNS_SERVER_COUNTER_ENTRY:
        {
            DNS_MGR_IPCMsg_ServCounterEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNextDnsServCounterEntry(&data_p->serverdata);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_ServCounterEntry_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_NEXT_NAME_SERVER_BY_INDEX:
        {
            DNS_MGR_IPCMsg_Next_Name_Server_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNextNameServerByIndex(
            &data_p->index,
            &data_p->ip);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Next_Name_Server_T);
            break;
        }

        case DNS_MGR_IPC_CMD_HOST_DELETE:
        {
            DNS_MGR_IPCMsg_HOST_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_HostDelete(
            data_p->hostname,
            &data_p->hostaddr);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_HOST_NAME_DELETE:
        {
            DNS_MGR_IPCMsg_HOST_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_HostNameDelete(
            data_p->hostname);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_SET_NAME_SERVER_BY_INDEX:
        {
            DNS_MGR_IPCMsg_Next_Name_Server_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_SetNameServerByIndex(
            data_p->index,
            &data_p->ip,
            data_p->is_add);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_GET_NEXT_RUNNING_HOST_ENTRY:
        {
            DNS_MGR_IPCMsg_GetNextRunningHostEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNextRunningDnsHostEntry(
            &data_p->index,
            &data_p->host_entry);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_GetNextRunningHostEntry_T);
            break;
        }

        case DNS_MGR_IPC_CMD_HOSTNAMETOIP:
        {
            DNS_MGR_IPCMsg_Host2Ip_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_HostNameToIp(
            (UI8_T *)data_p->hostname,
            data_p->family,
            data_p->ipaddr);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Host2Ip_T);
            break;
        }

        case DNS_MGR_IPC_CMD_CHECK_NAME_SERVER_IP:
        {
            DNS_MGR_IPCMsg_CheckNameServerIP_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_CheckNameServerIp(&data_p->serverip);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_CREATE_DOMAIN_NAME_LIST_ENTRY:
        {
            DNS_MGR_IPCMsg_IdxNameStr_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_CreateDomainNameListEntry(data_p->index);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_DESTROY_DOMAIN_NAME_LIST_ENTRY:
        {
            DNS_MGR_IPCMsg_IdxNameStr_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_DestroyDomainNameListEntry(data_p->index);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_SET_DOMAIN_NAME_LIST_ENTRY:
        {
            DNS_MGR_IPCMsg_IdxNameStr_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_SetDomainNameListEntry(data_p->index, data_p->name_str);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_CREATE_DNS_HOST_ENTRY:
        {
            DNS_MGR_IPCMsg_IdxNameStr_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_CreateDnsHostEntry(data_p->index);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_DESTROY_DNS_HOST_ENTRY:
        {
            DNS_MGR_IPCMsg_IdxNameStr_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_DestroyDnsHostEntry(data_p->index);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_SET_DNS_HOST_ENTRY:
        {
            DNS_MGR_IPCMsg_IdxNameStr_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_SetDnsHostEntry(data_p->index, data_p->name_str);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_GET_DNS_HOST_ENTRY_FOR_SNMP:
        {
            DNS_MGR_IPCMsg_IdxNameStr_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsHostEntryForSnmp(data_p->index, data_p->name_str);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T);
            break;
        }

        case DNS_MGR_IPC_CMD_GET_NEXT_DNS_HOST_ENTRY_FOR_SNMP:
        {
            DNS_MGR_IPCMsg_IdxNameStr_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNextDnsHostEntryForSnmp(&data_p->index, data_p->name_str);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_IdxNameStr_T);
            break;
        }

        case DNS_MGR_IPC_CMD_SET_DNS_HOST_IP_ENTRY:
        {
            DNS_MGR_IPCMsg_HostAddrEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_SetDnsHostAddrEntry(
                data_p->index, &data_p->addr, data_p->is_add);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_GET_DNS_HOST_IP_ENTRY:
        {
            DNS_MGR_IPCMsg_HostAddrEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetDnsHostAddrEntry(
                data_p->index, &data_p->addr);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_GET_NEXT_DNS_HOST_IP_ENTRY:
        {
            DNS_MGR_IPCMsg_HostAddrEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNextDnsHostAddrEntry(
                &data_p->index, &data_p->addr);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_HostAddrEntry_T);
            break;
        }

        case DNS_MGR_IPC_CMD_HOST_NAME_CACHE_DELETE:
        {
            DNS_MGR_IPCMsg_HOST_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_DeleteDnsCacheRR(
            data_p->hostname);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_GET_NEXT_LOOKUPCTL_TABLE:
        {
            DNS_MGR_IPCMsg_Nslookup_CTRL_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNextLookupCtlTable(&data_p->data);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_CTRL_T);
            break;
        }
        case DNS_MGR_IPC_CMD_GET_LOOKUPCTL_TABLE:
        {
            DNS_MGR_IPCMsg_Nslookup_CTRL_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetLookupCtlTable(&data_p->data);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_CTRL_T);
            break;
        }
        case DNS_MGR_IPC_CMD_SET_CTLTABLE_TARGETADDRESSTYPE:
        {
            DNS_MGR_IPCMsg_Nslookup_CTRL_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_SetDNSCtlTable_TargetAddressType(&data_p->data);
            ipcmsg_p->msg_size =  DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case DNS_MGR_IPC_CMD_SET_CTLTABLE_TARGETADDRESS:
        {
            DNS_MGR_IPCMsg_Nslookup_CTRL_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_SetDNSCtlTable_TargetAddress(&data_p->data);
            ipcmsg_p->msg_size =  DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case DNS_MGR_IPC_CMD_SET_CTLTABLE_ROWSTATUS:
        {
            DNS_MGR_IPCMsg_Nslookup_CTRL_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_SetDNSCtlTable_RowStatus(&data_p->data);
            ipcmsg_p->msg_size =  DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case DNS_MGR_IPC_CMD_CREATE_SYSTEM_NSLOOKUP_CTL_ENTRY:
        {
            DNS_MGR_IPCMsg_CreateNslookupCtlEntry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_CreateSystemNslookupCtlEntry(&data_p->ctl_entry, &data_p->ctl_index);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_CreateNslookupCtlEntry_T);
            break;
        }
        case DNS_MGR_IPC_CMD_GET_NSLOOKUP_CTL_ENTRY_BY_INDEX:
        {
            DNS_MGR_IPCMsg_GetNslookupCtlEntryByIndex_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNslookupCtlEntryByIndex(data_p->ctl_index, &data_p->ctl_entry);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_GetNslookupCtlEntryByIndex_T);
            break;
        }
        case DNS_MGR_IPC_CMD_GET_NSLOOKUP_RESULT_ENTRY_BY_INDEX:
        {
            DNS_MGR_IPCMsg_GetNslookupResultEntryByIndex_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNslookupResultEntryByIndex(data_p->ctl_index, data_p->result_index, &data_p->result_entry);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_GetNslookupResultEntryByIndex_T);
            break;
        }
        case DNS_MGR_IPC_CMD_GET_NEXT_LOOKUPRESULT_TABLE:
        {
            DNS_MGR_IPCMsg_Nslookup_RESULT_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNextLookupResultTable(&data_p->data);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_RESULT_T);
            break;
        }
        case DNS_MGR_IPC_CMD_GET_LOOKUPRESULT_TABLE:
        {
            DNS_MGR_IPCMsg_Nslookup_RESULT_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetLookupResultTable(&data_p->data);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_Nslookup_RESULT_T);
            break;
        }
        case DNS_MGR_IPC_CMD_DELETE_ENTRY:
        {
            DNS_MGR_IPCMsg_DELETE_NSLOOKUP_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_Nslookup_DeleteEntry(data_p->index);
            ipcmsg_p->msg_size =  DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case DNS_MGR_IPC_CMD_GET_TIMEOUT:
        {
            DNS_MGR_IPCMsg_GET_TIMEOUT_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNslookupTimeOut(&data_p->timeout,data_p->index);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_GET_TIMEOUT_T);
            break;
        }
        case DNS_MGR_IPC_CMD_GET_PURGE_TIME:
        {
            DNS_MGR_IPCMsg_PURGE_TIME_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = NS_MGR_GetNslookupPurgeTime(&data_p->purge_time);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_PURGE_TIME_T);
            break;
        }
        case DNS_MGR_IPC_CMD_SET_PURGE_TIME:
        {
            DNS_MGR_IPCMsg_PURGE_TIME_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_SetNslookupPurgeTime(data_p->purge_time);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case DNS_MGR_IPC_CMD_GET_NAME_SERVER_LIST:
        {
            DNS_MGR_IPCMsg_NAME_SERVER_Entry_T *data_p = DNS_MGR_MSG_DATA(ipcmsg_p);
            DNS_MGR_MSG_RETVAL(ipcmsg_p) = DNS_MGR_GetNameServerList(&data_p->serveraddr);
            ipcmsg_p->msg_size = DNS_MGR_GET_MSGBUFSIZE(DNS_MGR_IPCMsg_NAME_SERVER_Entry_T);
            break;
        }

        default:
        {
            SYSFUN_Debug_Printf("\r\n%s(): Invalid cmd.", __FUNCTION__);
            return FALSE;
        }
    } /* switch ipcmsg_p->cmd */

    return TRUE;
} /* DNS_MGR_HandleIPCReqMsg */

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
BOOL_T DNS_MGR_CreateDomainNameListEntry(UI32_T idx)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        ret = DNS_OM_CreateDomainNameListEntry(idx);
    }

    return ret;
}

/* FUNCTION NAME : DNS_MGR_DestroyDomainNameListEntry
 * PURPOSE: To destroy a dnsDomainListEntry in dnsDomainListTable.
 * INPUT  : idx   -- index of dnsDomainListEntry
 *                   (1-based, key to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_MGR_DestroyDomainNameListEntry(UI32_T idx)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        ret = DNS_OM_DestroyDomainNameListEntry(idx);
    }

    return ret;
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
BOOL_T DNS_MGR_SetDomainNameListEntry(UI32_T idx, char *domain_name_p)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        ret = DNS_OM_SetDomainNameListEntry(idx, domain_name_p);
    }

    return ret;
}

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
BOOL_T DNS_MGR_CreateDnsHostEntry(UI32_T host_idx)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        ret = DNS_HOSTLIB_CreateDnsHostEntry(host_idx);
    }

    return ret;
}

/* FUNCTION NAME : DNS_MGR_DestroyDnsHostEntry
 * PURPOSE: To destroy a host entry in dnsHostEntry table.
 * INPUT  : host_idx   -- index of dnsHostEntry
 *                        (1-based, key to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_MGR_DestroyDnsHostEntry(UI32_T host_idx)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        ret = DNS_HOSTLIB_DestroyDnsHostEntry(host_idx);
    }

    return ret;
}

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
BOOL_T DNS_MGR_SetDnsHostEntry(UI32_T host_idx, char *hostname_p)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        ret = DNS_HOSTLIB_SetDnsHostEntry(host_idx, hostname_p);
    }

    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetDnsHostEntryForSnmp
 * PURPOSE: To get entry from the dnsHostEntry table.
 * INPUT  : host_idx   -- index of dnsHostEntry
 *                        (1-based, key to search the entry)
 * OUTPUT : hostname_p -- pointer to hostname content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_MGR_GetDnsHostEntryForSnmp(UI32_T host_idx, char *hostname_p)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        ret = DNS_HOSTLIB_GetDnsHostEntry(host_idx, hostname_p);
    }

    return ret;
}

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
BOOL_T DNS_MGR_GetNextDnsHostEntryForSnmp(UI32_T *host_idx_p, char *hostname_p)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        ret = DNS_HOSTLIB_GetNextDnsHostEntry(host_idx_p, hostname_p);
    }

    return ret;
}

/* FUNCTION NAME : DNS_MGR_SetDnsHostAddrEntry
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
BOOL_T DNS_MGR_SetDnsHostAddrEntry(
    UI32_T host_idx, L_INET_AddrIp_T *addr_p, BOOL_T is_add)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        ret = DNS_HOSTLIB_SetDnsHostAddrEntry(host_idx, addr_p, is_add);
    }

    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetDnsHostAddrEntry
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
BOOL_T DNS_MGR_GetDnsHostAddrEntry(
    UI32_T host_idx, L_INET_AddrIp_T *addr_p)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        ret = DNS_HOSTLIB_GetDnsHostAddrEntry(host_idx, addr_p);
    }

    return ret;
}

/* FUNCTION NAME : DNS_MGR_GetNextDnsHostAddrEntry
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
BOOL_T DNS_MGR_GetNextDnsHostAddrEntry(
    UI32_T *host_idx_p, L_INET_AddrIp_T *addr_p)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        ret = DNS_HOSTLIB_GetNextDnsHostAddrEntry(host_idx_p, addr_p);
    }

    return ret;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DNS_MGR_CheckHostName
 *------------------------------------------------------------------------------
 * PURPOSE  : check input HostName's first character is letter.
 * INPUT    : host
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : Use the Accton spec to check the naming for host name not RFC document.
 *------------------------------------------------------------------------------*/
static BOOL_T DNS_MGR_CheckHostName(char *host)
{
    int     i;
    int     pos; /* according the index for subhost */
    char    subhost[MAXHOSTNAMELEN+1];

    if (host == NULL)
    {
        return FALSE;
    }

    memset(subhost, 0, sizeof(subhost));
    pos = 0;

    for (i=0; host[i]!='\0'; i++)
    {
        if (host[i] != '.')
        {
            /* check charset
             */
            if ((DNS_MGR_ISALNUM(host[i]) != TRUE) && (host[i] != '*') && (host[i] != '-') && (host[i] != '_'))  /* pgr0014, "(...) != TRUE" equivalent to "! (...)" */
            {
                return FALSE;
            }

            /* too long host STRING
             */
            if (sizeof(subhost)-2 < pos)
            {
                return FALSE;
            }

            subhost[pos++] = host[i];
        }

        /* check naming rule
         * meet to char '.' or last char
         */
        if (host[i] == '.' || host[i+1] == '\0')
        {
            /* It is not acceptable to the dot-postfix host
             * e.g., abc. or abc.def.
             */
            if (host[i] == '.' && host[i+1] == '\0')
                return FALSE;

            /* It is not acceptable to the zero host name length
             */
            if (pos == 0)
                return FALSE;

            /* It is not acceptable to the star-prefix host name
             * e.g., *abc
             */
            if (subhost[0] == '*' && subhost[1] != '\0')
                return FALSE;

            memset(subhost, 0, sizeof(subhost));
            pos = 0;
        }
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_MGR_IsNslookupCtlEntryAgeOut
 *-------------------------------------------------------------------------
 * PURPOSE  : Check nslookup control entry is age out or not.
 * INPUT    : ctl_entry_p  -- nslookup control entry
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T
DNS_MGR_IsNslookupCtlEntryAgeOut(
    DNS_Nslookup_CTRL_T *ctl_entry_p)
{
    UI32_T  base_time;
    UI32_T  expired_time;
    UI32_T  current_time;

    base_time = ctl_entry_p->created_time;
    expired_time = base_time + DNS_TYPE_NSLOOKUP_TTL_TICKS;
    SYS_TIME_GetSystemUpTimeByTick(&current_time);

    if (TRUE == DNS_CMM_IsAgeOut32(base_time, expired_time, current_time))
    {
        return TRUE;
    }

    return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_MGR_IsNslookupResultEntryAgeOut
 *-------------------------------------------------------------------------
 * PURPOSE  : Check nslookup result entry is age out or not.
 * INPUT    : result_entry_p  -- nslookup result entry
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T
DNS_MGR_IsNslookupResultEntryAgeOut(
    DNS_Nslookup_Result_T *result_entry_p)
{
    DNS_Nslookup_CTRL_T ctl_entry;
    UI32_T  base_time;
    UI32_T  expired_time;
    UI32_T  current_time;

    memset(&ctl_entry, 0, sizeof(ctl_entry));

    strncpy((char *)ctl_entry.CtlOwnerIndex,
        (char *)result_entry_p->CtlOwnerIndex,
        sizeof(ctl_entry.CtlOwnerIndex) - 1);
    ctl_entry.CtlOwnerIndex[sizeof(ctl_entry.CtlOwnerIndex) - 1] = '\0';
    ctl_entry.CtlOwnerIndexLen = strlen((char *)ctl_entry.CtlOwnerIndex);

    strncpy((char *)ctl_entry.OperationName,
        (char *)result_entry_p->OperationName,
        sizeof(ctl_entry.OperationName) - 1);
    ctl_entry.OperationName[sizeof(ctl_entry.OperationName) - 1] = '\0';
    ctl_entry.OperationNameLen = strlen((char *)ctl_entry.OperationName);

    if (DNS_OK != DNS_OM_GetLookupCtlTable(&ctl_entry))
    {
        return TRUE;
    }

    base_time = ctl_entry.created_time;
    expired_time = base_time + DNS_TYPE_NSLOOKUP_TTL_TICKS;
    SYS_TIME_GetSystemUpTimeByTick(&current_time);

    if (TRUE == DNS_CMM_IsAgeOut32(base_time, expired_time, current_time))
    {
        return TRUE;
    }

    return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_MGR_IsNslookupCtlEntryExist
 *-------------------------------------------------------------------------
 * PURPOSE  : Check nslookup control entry exist or not.
 * INPUT    : ctl_entry_p  -- nslookup control entry
 * OUTPUT   : ctl_index_p  -- 0-based nslookup control entry index
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T
DNS_MGR_IsNslookupCtlEntryExist(
    DNS_Nslookup_CTRL_T *ctl_entry_p,
    UI32_T *ctl_index_p)
{
    DNS_Nslookup_CTRL_T ctl_entry;

    memset(&ctl_entry, 0, sizeof(ctl_entry));

    strncpy((char *)ctl_entry.CtlOwnerIndex,
        (char *)ctl_entry_p->CtlOwnerIndex,
        sizeof(ctl_entry.CtlOwnerIndex) - 1);
    ctl_entry.CtlOwnerIndex[sizeof(ctl_entry.CtlOwnerIndex) - 1] = '\0';
    ctl_entry.CtlOwnerIndexLen = strlen((char *)ctl_entry.CtlOwnerIndex);

    strncpy((char *)ctl_entry.OperationName,
        (char *)ctl_entry_p->OperationName,
        sizeof(ctl_entry.OperationName) - 1);
    ctl_entry.OperationName[sizeof(ctl_entry.OperationName) - 1] = '\0';
    ctl_entry.OperationNameLen = strlen((char *)ctl_entry.OperationName);

    if (DNS_OK == DNS_OM_GetLookupCtlTableAndIndex(&ctl_entry, ctl_index_p))
    {
        return TRUE;
    }

    return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_MGR_DeleteAllSystemAgeOutNslookupCtlEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Delete all nslookup control entrys which control owner is system
 *            and is age out.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : DNS_OK/DNS_ERROR
 * NOTES    : If no any one entry be deleted, the return value is false.
 * ------------------------------------------------------------------------
 */
static int
DNS_MGR_DeleteAllSystemAgeOutNslookupCtlEntry(void)
{
    DNS_Nslookup_CTRL_T ctl_entry;
    UI32_T ctl_index;
    UI32_T ret = DNS_ERROR;

    memset(&ctl_entry, 0, sizeof(ctl_entry));

    while (DNS_OK == DNS_OM_GetNextLookupCtlTableAndIndex(&ctl_entry, &ctl_index))
    {
        if (TRUE == DNS_MGR_IS_SYSTEM_OWNER_INDEX(ctl_entry.CtlOwnerIndex))
        {
            if (TRUE == DNS_MGR_IsNslookupCtlEntryAgeOut(&ctl_entry))
            {
                DNS_OM_Nslookup_DeleteEntry(ctl_index);
                ret = DNS_OK;
            }
        }
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_MGR_CreateNslookupCtlEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Create nslookup control entry.
 * INPUT    : ctl_entry_p  -- nslookup control entry
 * OUTPUT   : ctl_index_p  -- 0-based nslookup control entry index
 * RETUEN   : DNS_OK/DNS_ERROR
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static int
DNS_MGR_CreateNslookupCtlEntry(
    DNS_Nslookup_CTRL_T *ctl_entry_p,
    UI32_T *ctl_index_p)
{
    DNS_Nslookup_CTRL_T ctl_entry;
    DNS_Nslookup_Result_T result_entry;
    UI32_T ctl_index;
    UI32_T result_index;

    if ((NULL == ctl_entry_p) || (NULL == ctl_index_p))
    {
        return DNS_ERROR;
    }

    if ((0 == ctl_entry_p->CtlOwnerIndexLen) ||
        (MAXSIZE_dnsCtlOwnerIndex < ctl_entry_p->CtlOwnerIndexLen) ||
        (0 == ctl_entry_p->OperationNameLen) ||
        (MAXSIZE_dnsCtlOperationName < ctl_entry_p->OperationNameLen))
    {
        return DNS_ERROR;
    }

    if ((AF_INET != ctl_entry_p->af_family) &&
        (AF_INET6 != ctl_entry_p->af_family) &&
        (AF_UNSPEC != ctl_entry_p->af_family))
    {
        return DNS_ERROR;
    }

    if (TRUE == DNS_MGR_IsNslookupCtlEntryExist(ctl_entry_p, &ctl_index))
    {
        DNS_OM_Nslookup_DeleteEntry(ctl_index);
    }

    if (TRUE == DNS_OM_IsNslookupCtlTableFull())
    {
        if (DNS_OK != DNS_MGR_DeleteAllSystemAgeOutNslookupCtlEntry())
        {
            return DNS_ERROR;
        }
    }

    memset(&ctl_entry, 0, sizeof(ctl_entry));

    strncpy((char *)ctl_entry.CtlOwnerIndex,
        (char *)ctl_entry_p->CtlOwnerIndex,
        sizeof(ctl_entry.CtlOwnerIndex) - 1);
    ctl_entry.CtlOwnerIndex[sizeof(ctl_entry.CtlOwnerIndex) - 1] = '\0';
    ctl_entry.CtlOwnerIndexLen = strlen((char *)ctl_entry.CtlOwnerIndex);

    strncpy((char *)ctl_entry.OperationName,
        (char *)ctl_entry_p->OperationName,
        sizeof(ctl_entry.OperationName) - 1);
    ctl_entry.OperationName[sizeof(ctl_entry.OperationName) - 1] = '\0';
    ctl_entry.OperationNameLen = strlen((char *)ctl_entry.OperationName);

    strncpy((char *)ctl_entry.TargetAddress,
        (char *)ctl_entry_p->TargetAddress,
        sizeof(ctl_entry.TargetAddress) - 1);
    ctl_entry.TargetAddress[sizeof(ctl_entry.TargetAddress) - 1] = '\0';

    ctl_entry.TargetAddressLen = strlen((char *)ctl_entry.TargetAddress);
    ctl_entry.TargetAddressType = VAL_dnsCtlTargetAddressType_dns;
    ctl_entry.af_family = ctl_entry_p->af_family;
    ctl_entry.RowStatus = ctl_entry_p->RowStatus;
    ctl_entry.OperStatus = NSLOOKUP_OPSTATUS_NOTSTARTED;
    SYS_TIME_GetSystemUpTimeByTick(&ctl_entry.created_time);

    if (DNS_OK != DNS_OM_CreateNslookupCtlEntry(&ctl_entry, &ctl_index))
    {
        if (DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
        {
            BACKDOOR_MGR_Printf("%s:%d Failed to create control entry\r\n", __FUNCTION__, __LINE__); fflush(stdout);
        }

        return DNS_ERROR;
    }

    for (result_index = 0; result_index < MAXHOSTIPNUM; result_index++)
    {
        memset(&result_entry, 0, sizeof(result_entry));

        strncpy((char *)result_entry.CtlOwnerIndex,
            (char *)ctl_entry_p->CtlOwnerIndex,
            sizeof(result_entry.CtlOwnerIndex) - 1);
        result_entry.CtlOwnerIndex[sizeof(result_entry.CtlOwnerIndex) - 1] = '\0';
        result_entry.CtlOwnerIndexLen = strlen((char *)result_entry.CtlOwnerIndex);

        strncpy((char *)result_entry.OperationName,
            (char *)ctl_entry_p->OperationName,
            sizeof(result_entry.OperationName) - 1);
        result_entry.OperationName[sizeof(result_entry.OperationName) - 1] = '\0';
        result_entry.OperationNameLen = strlen((char *)result_entry.OperationName);

        if (DNS_OK != DNS_OM_CreateNslookupResultEntryByIndex(ctl_index, result_index, &result_entry))
        {
            if (DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
            {
                BACKDOOR_MGR_Printf("%s:%d Failed to create result entry. ctl_index = %lu, result_index = %lu\r\n",
                    __FUNCTION__, __LINE__, (unsigned long)ctl_index, (unsigned long)result_index); fflush(stdout);
            }

            DNS_OM_Nslookup_DeleteEntry(ctl_index);
            return DNS_ERROR;
        }
    }

    *ctl_index_p = ctl_index;

    if (DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
    {
        BACKDOOR_MGR_Printf("\n %s:%d \r\n", __FUNCTION__, __LINE__); fflush(stdout);
        BACKDOOR_MGR_Printf("==================================================\r\n"); fflush(stdout);
        BACKDOOR_MGR_Printf("ctl_index = %lu \r\n", (unsigned long)ctl_index);
        BACKDOOR_MGR_Printf("CtlOwnerIndex = %s, CtlOwnerIndexLen = %lu\r\n",
            ctl_entry.CtlOwnerIndex, (unsigned long)ctl_entry.CtlOwnerIndexLen); fflush(stdout);
        BACKDOOR_MGR_Printf("OperationName = %s, OperationNameLen = %lu\r\n",
            ctl_entry.OperationName, (unsigned long)ctl_entry.OperationNameLen); fflush(stdout);
        BACKDOOR_MGR_Printf("TargetAddress = %s, TargetAddressLen = %lu\r\n",
            ctl_entry.TargetAddress, (unsigned long)ctl_entry.TargetAddressLen); fflush(stdout);
        BACKDOOR_MGR_Printf("TargetAddressType = %lu \r\n", (unsigned long)ctl_entry.TargetAddressType); fflush(stdout);
        BACKDOOR_MGR_Printf("af_family = %lu \r\n", (unsigned long)ctl_entry.af_family); fflush(stdout);
        BACKDOOR_MGR_Printf("RowStatus = %lu \r\n", (unsigned long)ctl_entry.RowStatus); fflush(stdout);
        BACKDOOR_MGR_Printf("OperStatus = %lu \r\n", (unsigned long)ctl_entry.OperStatus); fflush(stdout);
        BACKDOOR_MGR_Printf("created_time = %lu \r\n", (unsigned long)ctl_entry.created_time); fflush(stdout);
        BACKDOOR_MGR_Printf("==================================================\r\n"); fflush(stdout);
    }

    return DNS_OK;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_MGR_AsyncStartNslookup
 *-------------------------------------------------------------------------
 * PURPOSE  : Asynchronous start nslookup.
 * INPUT    : ctl_index  -- 0-based nslookup control entry index
 * OUTPUT   : None
 * RETUEN   : DNS_OK/DNS_ERROR
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static int
DNS_MGR_AsyncStartNslookup(
    UI32_T ctl_index)
{
    UI32_T *ctl_index_p;
    UI32_T thread_id;

    if (DNS_DISABLE == DNS_MGR_GetDnsStatus())
    {
        if (DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
        {
            BACKDOOR_MGR_Printf("\r\n%s:%d start lookup fail. DNS disabled.\r\n", __FUNCTION__, __LINE__); fflush(stdout);
        }

        return DNS_ERROR;
    }

    ctl_index_p = (UI32_T *)L_MM_Malloc(sizeof(UI32_T),
        L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_MGR_ASYNCSTARTNSLOOKUP));

    if (NULL == ctl_index_p)
    {
        return DNS_ERROR;
    }

    *ctl_index_p = ctl_index;

    if (SYSFUN_OK != SYSFUN_SpawnThread(SYS_BLD_DNS_PROXY_CSC_THREAD_PRIORITY,
                                        SYS_BLD_DNS_PROXY_CSC_THREAD_SCHED_POLICY,
                                        SYS_BLD_DNS_RESOLVER_CSC_THREAD_NAME,
                                        SYS_BLD_TASK_COMMON_STACK_SIZE,
                                        SYSFUN_TASK_NO_FP,
                                        DNS_MGR_AsnycNslookupTaskMain,
                                        ctl_index_p,
                                        &thread_id))
    {
        if (DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
        {
            BACKDOOR_MGR_Printf("\r\n%s:%d Failed to spawn thread\r\n", __FUNCTION__, __LINE__); fflush(stdout);
        }

        L_MM_Free(ctl_index_p);
        return DNS_ERROR;
    }

    return DNS_OK;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_MGR_AsnycNslookupTaskMain
 *-------------------------------------------------------------------------
 * PURPOSE  : This function start nslookup and write result to
 *            nslookup control entry and result entry.
 * INPUT    : ctl_index_p  -- 0-based nslookup control entry index
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static void
DNS_MGR_AsnycNslookupTaskMain(
    UI32_T *ctl_index_p)
{
    DNS_Nslookup_CTRL_T  ctl_entry;
    DNS_Nslookup_Result_T result_entry;
    L_INET_AddrIp_T  hostip_ar[MAXHOSTIPNUM];
    struct timespec start_time, end_time;
    long ctl_time;
    int rc;
    UI32_T  ctl_index;
    UI32_T  result_index;
    char host_ip_str[L_INET_MAX_IPADDR_STR_LEN + 1] = {0};

    if (NULL == ctl_index_p)
    {
        return;
    }

    ctl_index = *ctl_index_p;
    L_MM_Free(ctl_index_p);
    memset(&ctl_entry, 0, sizeof(ctl_entry));
    memset(&hostip_ar, 0, sizeof(hostip_ar));

    if (DNS_OK != DNS_OM_GetNslookupCtlEntryByIndex(ctl_index, &ctl_entry))
    {
        if (DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
        {
            BACKDOOR_MGR_Printf("\r\n%s:%d DNS_OM_GetNslookupCtrlEntryByIndex fail. ctl_index = %lu\r\n",
                __FUNCTION__, __LINE__, (unsigned long)ctl_index); fflush(stdout);
        }

        return;
    }

    if ((VAL_dnsCtlRowStatus_active != ctl_entry.RowStatus) ||
        (0 == ctl_entry.TargetAddressLen))
    {
        if (DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
        {
            BACKDOOR_MGR_Printf("\r\n%s:%d ctl_entry invalid. RowStatus = %lu TargetAddressLen = %lu\r\n",
                __FUNCTION__, __LINE__, (unsigned long)ctl_entry.RowStatus, (unsigned long)ctl_entry.TargetAddressLen); fflush(stdout);
        }

        return;
    }

    DNS_OM_SetLookupCtlOperStatusByIndex(ctl_index, NSLOOKUP_OPSTATUS_ENABLE);

    if (DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
    {
        BACKDOOR_MGR_Printf("\r\n%s:%d host name to ip. \r\n ctl_index = %lu, hostname = %s, af_family = %lu\r\n",
            __FUNCTION__, __LINE__, (unsigned long)ctl_index, ctl_entry.TargetAddress, (unsigned long)ctl_entry.af_family); fflush(stdout);
    }

    clock_gettime(CLOCK_REALTIME, &start_time);
    rc = DNS_MGR_HostNameToIp(ctl_entry.TargetAddress, ctl_entry.af_family, hostip_ar);
    clock_gettime(CLOCK_REALTIME, &end_time);
    ctl_time = (end_time.tv_sec - start_time.tv_sec)  /* seconds (time_t), zero or positive */
                * 1000
                + (end_time.tv_nsec - start_time.tv_nsec) /* nanoseconds (long), zero or positive or negative */
                / 1000000;
    DNS_OM_SetLookupCtlTimeByIndex(ctl_index, ctl_time);
    DNS_OM_SetLookupCtlRcByIndex(ctl_index, rc);
    DNS_OM_SetLookupCtlOperStatusByIndex(ctl_index, NSLOOKUP_OPSTATUS_COMPLETED);

    if (DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
    {
        BACKDOOR_MGR_Printf("\r\n %s:%d lookup completed.\r\n", __FUNCTION__, __LINE__); fflush(stdout);
        BACKDOOR_MGR_Printf("==================================================\r\n"); fflush(stdout);
        BACKDOOR_MGR_Printf("ctl_index = %lu, hostname = %s, af_family = %lu\r\n",
            (unsigned long)ctl_index, ctl_entry.TargetAddress, (unsigned long)ctl_entry.af_family); fflush(stdout);
        BACKDOOR_MGR_Printf("rc = %d\r\n", rc); fflush(stdout);
        BACKDOOR_MGR_Printf("==================================================\r\n"); fflush(stdout);
    }

    if (DNS_OK == rc)
    {
        for (result_index = 0; result_index < MAXHOSTIPNUM; result_index++)
        {
            /* if found an empty entry, there is no more valid entry
             */
            if (0 == hostip_ar[result_index].addrlen)
            {
                break;
            }

            DNS_OM_SetLookupResultsIndexByIndex(ctl_index, result_index, result_index + 1);
            DNS_OM_SetLookupResultsAddressByIndex(ctl_index, result_index, &hostip_ar[result_index]);

            if (DNS_ENABLE == DNS_MGR_GetDnsDebugStatus())
            {
                DNS_Nslookup_Result_T result_entry;

                memset(&result_entry, 0, sizeof(result_entry));

                if (DNS_OK == DNS_OM_GetNslookupResultEntryByIndex(ctl_index, result_index, &result_entry))
                {
                    BACKDOOR_MGR_Printf("\r\n %s:%d \r\n", __FUNCTION__, __LINE__); fflush(stdout);
                    BACKDOOR_MGR_Printf("==================================================\r\n"); fflush(stdout);
                    BACKDOOR_MGR_Printf("ctl_index = %lu, result_index = %d\r\n", (unsigned long)ctl_index, result_index); fflush(stdout);
                    BACKDOOR_MGR_Printf("CtlOwnerIndex = %s, CtlOwnerIndexLen = %lu\r\n",
                        result_entry.CtlOwnerIndex, (unsigned long)result_entry.CtlOwnerIndexLen); fflush(stdout);
                    BACKDOOR_MGR_Printf("OperationName = %s, OperationNameLen = %lu\r\n",
                        result_entry.OperationName, (unsigned long)result_entry.OperationNameLen); fflush(stdout);
                    BACKDOOR_MGR_Printf("ResultsIndex = %lu\r\n", (unsigned long)result_entry.ResultsIndex); fflush(stdout);
                    BACKDOOR_MGR_Printf("ResultsAddressType = %u\r\n", result_entry.ResultsAddress_str.type); fflush(stdout);

                    if (L_INET_RETURN_SUCCESS == L_INET_InaddrToString((L_INET_Addr_T *)&result_entry.ResultsAddress_str,
                        host_ip_str, sizeof(host_ip_str)))
                    {
                        BACKDOOR_MGR_Printf("ResultsAddress = %s\r\n", host_ip_str); fflush(stdout);
                    }

                    BACKDOOR_MGR_Printf("==================================================\r\n"); fflush(stdout);
                }
            }
        }
    }
}

#endif /* #if (SYS_CPNT_DNS == TRUE) */

