/* MODULE NAME:  dns_vm.c
 * PURPOSE:
 *   Provide some functions for the dns module.
 *
 *
 * NOTES:
 *
 * History:
 *       Date          -- Modifier,     Reason
 *       2002-12-03    -- isiah,  created
 *
 * Copyright(C)      Accton Corporation, 2002
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_DNS == TRUE)

#include <sys_type.h>
#include "dns.h"
#include "dns_om.h"
#include "dns_vm.h"
#include "dns_cache.h"
#include "dns_hostlib.h"
#include "dns_resolver.h"
#include "sysfun.h"

struct hostent* gethostbyname(const I8_T* name);

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

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME : DNS_VM_GetDnsSbelt
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
DNS_ResConfigSbelt_T* DNS_VM_GetDnsSbelt(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    DNS_ResConfigSbelt_T* sbelt_p;

    /* BODY */
    sbelt_p = DNS_OM_GetDnsSbelt();


    return sbelt_p;
}



/* FUNCTION NAME : DNS_VM_SetResResetStatus
 * PURPOSE:
 *  This function reinitialize any persistant resolver state.
 *
 *
 *
 * INPUT:
 *  int  -- a variable stored with the reset value,2 means reset
 *
 * OUTPUT:
 *   none.
 *
 * RETURN:
 *  DNS_ERROR :failure,
 *  DNS_OK    :success.
 * NOTES:
 *   .
 */
int DNS_VM_SetResResetStatus(int config_reset_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */

    DNS_OM_SetResResetStatus(config_reset_p);


    return DNS_OK;
}



/* FUNCTION	NAME : DNS_VM_SetResolverTaskId
 * PURPOSE:
 *		Set resolver task id.
 *
 * INPUT:
 *		UI32_T  tid --  Resolver task id.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *          TRUE
 *
 * NOTES:
 *		None.
 */
BOOL_T DNS_VM_SetResolverTaskId(UI32_T tid)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */

    DNS_OM_SetResolverTaskId(tid);

    return TRUE;
}



/* FUNCTION NAME : DNS_VM_SetServStatus
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
BOOL_T DNS_VM_SetServStatus(int status)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */

    DNS_OM_SetServStatus(status);

    return TRUE;
}



/* FUNCTION NAME : DNS_VM_SetServResetStatus
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
BOOL_T DNS_VM_SetServResetStatus(int status)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */

    DNS_OM_SetServResetStatus(status);

    return TRUE;
}



/* FUNCTION NAME : DNS_VM_ServUpTimeInit
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
BOOL_T DNS_VM_ServUpTimeInit(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */

    DNS_OM_ServUpTimeInit();

    return TRUE;
}



/* FUNCTION NAME : DNS_VM_ServResetTimeInit
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
BOOL_T DNS_VM_ServResetTimeInit(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */

    DNS_OM_ServResetTimeInit();

    return TRUE;
}



/* FUNCTION NAME : DNS_VM_GetDnsDebugStatus
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
int DNS_VM_GetDnsDebugStatus(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */
    int ret;

    /* BODY */
    DNS_OM_GetDnsDebugStatus(&ret);

    return ret;
}



/* FUNCTION NAME : DNS_VM_GetDnsResConfigReset
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
 *   .
 */
int DNS_VM_GetDnsResConfigReset(int *config_reset_p)

{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */
    int ret;

    /* BODY */

    ret = DNS_OM_GetDnsResConfigReset(config_reset_p);


    return ret;
}



/* FUNCTION NAME : DNS_VM_SetDnsLocalMaxRequests
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
 *   .
 */
int DNS_VM_GetDnsTimeOut(UI32_T *dns_time_out_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */
    int ret;

    /* BODY */

    ret = DNS_OM_GetDnsTimeOut(dns_time_out_p);


    return ret;
}



/* FUNCTION NAME : DNS_VM_DnsUptimeInit
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
BOOL_T DNS_VM_DnsUptimeInit(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */

    DNS_OM_DnsUptimeInit();

    return TRUE;
}



/* FUNCTION NAME : DNS_VM_DnsResetTimeInit
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
BOOL_T DNS_VM_DnsResetTimeInit(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */

    DNS_OM_DnsResetTimeInit();

    return TRUE;
}



/* FUNCTION NAME : DNS_VM_GetDnsServConfigRecurs
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
 *		.
 */
int DNS_VM_GetDnsServConfigRecurs(int *config_recurs_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */

    ret = DNS_OM_GetDnsServConfigRecurs(config_recurs_p);

    return ret;
}



/* FUNCTION NAME : DNS_VM_SetServServiceEnable
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
BOOL_T DNS_VM_SetServServiceEnable(int enable)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */

    DNS_OM_SetServServiceEnable(enable);

    return TRUE;
}



/* FUNCTION NAME :DNS_VM_GetDnsServConfigMaxRequests
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
int DNS_VM_GetDnsServConfigMaxRequests(int *dns_serv_config_max_requests_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */

    ret = DNS_OM_GetDnsServConfigMaxRequests(dns_serv_config_max_requests_p);

    return ret;
}



/* FUNCTION NAME : DNS_VM_GetDnsStatus
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
 *  none.
 *
 * NOTES:
 *  none.
 */
int DNS_VM_GetDnsStatus(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret;

    /* BODY */
   DNS_OM_GetDnsStatus(&ret);

    return ret;
}



/* FUNCTION NAME : DNS_VM_GetDnsResCacheStatus
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
int DNS_VM_GetDnsResCacheStatus(int *dns_res_cache_status_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */

    ret = DNS_OM_GetDnsResCacheStatus(dns_res_cache_status_p);

    return ret;
}



/* FUNCTION NAME : DNS_VM_GetDnsResCacheMaxTTL
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
int DNS_VM_GetDnsResCacheMaxTTL(UI32_T *dns_res_cache_max_ttl_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */

    ret = DNS_OM_GetDnsResCacheMaxTTL(dns_res_cache_max_ttl_p);

    return ret;
}



/* FUNCTION NAME : DNS_VM_GetDnsResCacheMaxEntries
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
 *   This function will be called by DNS_CACHE_Init.
 */
int DNS_VM_GetDnsResCacheMaxEntries(int *dns_res_cache_max_entries_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    int ret = DNS_ERROR;

    /* BODY */

    ret = DNS_OM_GetDnsResCacheMaxEntries(dns_res_cache_max_entries_p);

    return ret;
}


/* LOCAL SUBPROGRAM BODIES
 */

#endif /* #if (SYS_CPNT_DNS == TRUE) */

