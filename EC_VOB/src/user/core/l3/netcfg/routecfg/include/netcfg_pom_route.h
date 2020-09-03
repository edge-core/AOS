/* MODULE NAME:  netcfg_pom_route.h
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to 
 *    NETCFG_OM_ROUTE service.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    02/21/2008 - Vai Wang, Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */

/* INCLUDE FILE DECLARATIONS
 */
 
#ifndef NETCFG_POM_ROUTE_H
#define NETCFG_POM_ROUTE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "netcfg_type.h"
#include "route_mgr.h"
#include "netcfg_om_route.h"

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
/* FUNCTION NAME : NETCFG_POM_ROUTE_InitiateProcessResource
 * PURPOSE:
 *    Initiate resource for CSCA_POM in the calling process.
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
 *    Before other CSC use NETCFG_POM_ROUTE, it should initiate the resource (get the message queue handler internally)
 
 *
 */
BOOL_T NETCFG_POM_ROUTE_InitiateProcessResource(void);

/* ROUTINE NAME: NETCFG_POM_ROUTE_GetNextStaticIpCidrRoute
 *
 * FUNCTION:
 *      Get next route using 4 keys.
 *
 * INPUT:
 *      entry_p.
 *
 * OUTPUT:
 *      entry_p.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *
 * NOTES:
 *      key(entry->ip_cidr_route_dest,
 *          entry->ip_cidr_route_mask,
 *          entry->ip_cidr_route_next_hop,
 *          entry->ip_cidr_route_distance)
 */
UI32_T NETCFG_POM_ROUTE_GetNextStaticIpCidrRoute(ROUTE_MGR_IpCidrRouteEntry_T *entry_p);

/* FUNCTION NAME : NETCFG_POM_ROUTE_GetIpForwardingStatus
 * PURPOSE:
 *      Retrieve IPv4/IPv6 forwarding status
 *
 * INPUT:
 *      ifs->vr_id          -- vrid
 *
 * OUTPUT:
 *      ifs->status_bitmap  -- forwarding status bitmap
 *
 * RETURN:
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_OK
 *
 * NOTES:
 */
UI32_T NETCFG_POM_ROUTE_GetIpForwardingStatus(NETCFG_OM_ROUTE_IpForwardingStatus_T *ifs);


/* FUNCTION NAME : NETCFG_POM_ROUTE_GetNextRunningStaticIpCidrRouteEntry
 * PURPOSE:
 *      Get next running static record after the specified key.
 *
 * INPUT:
 *      ip_cidr_route_dest      : IP address of the route entry. (network order)
 *      ip_cidr_route_mask      : IP mask of the route entry. (network order)
 *      ip_cidr_route_distance  : Administrative distance.
 *      ip_cidr_route_next_hop  : IP address of Next-Hop. (network order)
 *
 * OUTPUT:
 *      ip_cidr_route_entry -- record of forwarding table.
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- got static route setting
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL       -- no more interface information.
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *      1. Key is (ipCidrRouteDest, ipCidrRouteMask, ip_cidr_route_distance, ipCidrRouteNextHop)
 *         in NETCFG_MGR_IpCidrRouteEntry_T.
 *      2. if key is (0,0,0,0), means get first one.
 *      3. Related definition, please ref. RFC 2096 (IP CIDR Route Table).
 *      4. Static route entry always set by user.
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_ROUTE_GetNextRunningStaticIpCidrRouteEntry(NETCFG_TYPE_IpCidrRouteEntry_T *ip_cidr_route_entry);

/* FUNCTION NAME: NETCFG_POM_ROUTE_GetRunningDefaultGateway
 * PURPOSE:
 *      Get running system default gateway.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      default_gateway_ip -- no default value. (network order)
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- got new default gateway IP.
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL       -- no more defautl gateway specified.
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- do not set default gateway.
 *
 * NOTES:
 *      1. Default gateway is set in L2, in L3 is using "ip route" to configure.
 *      2. Support IPv4/IPv6.
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_ROUTE_GetRunningDefaultGateway(L_INET_AddrIp_T *default_gateway_ip);

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_POM_ROUTE_GetRunningEcmpBalanceMode
 *-----------------------------------------------------------------------------
 * PURPOSE : To get running ecmp load balance mode.
 * INPUT   : None
 * OUTPUT  : mode_p - pointer to output mode
 *           idx_p  - pointer to output hash selection list index
 *                    if *mode_p is NETCFG_TYPE_ECMP_HASH_SELECTION
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES   : None
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_ROUTE_GetRunningEcmpBalanceMode(UI32_T *mode_p, UI32_T *idx_p);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_POM_ROUTE_GetEcmpBalanceMode
 *-----------------------------------------------------------------------------
 * PURPOSE : To get current ecmp load balance mode.
 * INPUT   : None
 * OUTPUT  : mode_p - pointer to output mode
 *           idx_p  - pointer to output hash selection list index
 *                    if *mode_p is NETCFG_TYPE_ECMP_HASH_SELECTION
 * RETURN  : NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
 * NOTES   : None
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_POM_ROUTE_GetEcmpBalanceMode(UI32_T *mode_p, UI32_T *idx_p);

#endif /* #if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE) */

#endif /* NETCFG_POM_ROUTE_H */

