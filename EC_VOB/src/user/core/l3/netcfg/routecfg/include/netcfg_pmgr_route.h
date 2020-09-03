/* MODULE NAME:  netcfg_pmgr_route.h
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to
 *    access NETCFG_MGR_ROUTE and NETCFG_OM_ROUTE service.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    12/18/2007 - Vai Wang, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef NETCFG_PMGR_ROUTE_H
#define NETCFG_PMGR_ROUTE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "netcfg_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME : NETCFG_PMGR_ROUTE_InitiateProcessResource
 * PURPOSE:
 *    Initiate resource used in the calling process.
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
 *    Before other CSC use NETCFG_PMGR_ROUTE, it should initiate
 *    the resource (get the message queue handler internally)
 *
 */
BOOL_T NETCFG_PMGR_ROUTE_InitiateProcessResource(void);


/* FUNCTION NAME: NETCFG_PMGR_ROUTE_SetDhcpDefaultGateway
 * PURPOSE:
 *      Set system default gateway which claimed from DHCP server.
 * INPUT:
 *      default_gateway -- ip address of the default gateway (in network order)
 * OUTPUT:
 *      none
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL -- failure to set.
 *      NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_ADD
 *      NETCFG_TYPE_TABLE_FULL
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 * NOTES:
 *      1. this default gateway will keep until binding subnet destroyed.
 *      2. DHCP claimed default gateway takes as static configured route,
 *         but do not show in running configuration.
 *      3. If there exist static configured route, this gateway from DHCP server will not used
 */
UI32_T NETCFG_PMGR_ROUTE_SetDhcpDefaultGateway(L_INET_AddrIp_T* default_gateway);

/* FUNCTION NAME: NETCFG_PMGR_ROUTE_AddDefaultGateway
 * PURPOSE:
 *      Add system default gateway, for multiple default gateway,
 *  distance determine the order be used.
 *
 * INPUT:
 *      default_gateway -- ip address of the default gateway (network order)
 *      distance        -- smallest value means higher priority be used.
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG          : input parameters are invalid.
 *      NETCFG_TYPE_CAN_NOT_ADD
 *
 * NOTES:
 *
 */
UI32_T NETCFG_PMGR_ROUTE_AddDefaultGateway(L_INET_AddrIp_T *default_gateway_ip, UI32_T distance);


/* FUNCTION NAME: NETCFG_PMGR_ROUTE_DeleteDefaultGateway
 * PURPOSE:
 *      Delete system default gateway which specified in argument.
 *
 * INPUT:
 *      default_gateway_ip -- to be deleted. (network order)
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 *      NETCFG_TYPE_CAN_NOT_DELETE
 *
 * NOTES:
 *  1. If default_gateway_ip == 0, delete all default gateway.
 *  2. Currently, only one gateway, so base on argument 'default_gateway_ip'
 *     to verify consistent or not.
 */
UI32_T NETCFG_PMGR_ROUTE_DeleteDefaultGateway(L_INET_AddrIp_T *default_gateway_ip);

/* FUNCTION NAME: NETCFG_PMGR_ROUTE_DeleteDhcpDefaultGateway
 * PURPOSE:
 *      Delete system default gateway which is confgiured by DHCP
 *
 * INPUT:
 *      default_gateway_ip -- to be deleted. (network order)
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 *      NETCFG_TYPE_CAN_NOT_DELETE
 *
 * NOTES:
 */
UI32_T NETCFG_PMGR_ROUTE_DeleteDhcpDefaultGateway(L_INET_AddrIp_T *default_gateway_ip);


/* FUNCTION NAME: NETCFG_PMGR_ROUTE_GetDefaultGateway
 * PURPOSE:
 *      Get system default gateway.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      default_gateway_ip. (network order)
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
 *
 */
UI32_T NETCFG_PMGR_ROUTE_GetDefaultGateway(L_INET_AddrIp_T * default_gateway_ip);

/* FUNCTION NAME: NETCFG_PMGR_ROUTE_GetDhcpDefaultGateway
 * PURPOSE:
 *      Get system default gateway which is configured by DHCP
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      default_gateway_ip
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
 *
 */
UI32_T NETCFG_PMGR_ROUTE_GetDhcpDefaultGateway(L_INET_AddrIp_T * default_gateway_ip);

/* FUNCTION NAME : NETCFG_PMGR_ROUTE_GetNextRunningStaticIpCidrRouteEntry
 * PURPOSE:
 *      Get next static record after the specified key.
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
SYS_TYPE_Get_Running_Cfg_T NETCFG_PMGR_ROUTE_GetNextRunningStaticIpCidrRouteEntry(NETCFG_TYPE_IpCidrRouteEntry_T *ip_cidr_route_entry);


/* FUNCTION NAME: NETCFG_PMGR_ROUTE_GetRunningDefaultGateway
 * PURPOSE:
 *      Get system default gateway.
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
SYS_TYPE_Get_Running_Cfg_T NETCFG_PMGR_ROUTE_GetRunningDefaultGateway(L_INET_AddrIp_T *default_gateway_ip);

/* FUNCTION NAME : NETCFG_PMGR_ROUTE_GetRunningIpForwarding
 * PURPOSE:
 *      Retrieve IPv4/IPv6 forwarding running configuration.
 *
 * INPUT:
 *      vr_id	    : virtual router id
 *      address_type: IPv4/IPv6 to query
 *      status      : status of ip forwarding
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 *      NETCFG_TYPE_DESTINATION_IS_LOCAL
 *      NETCFG_TYPE_TABLE_FULL
 *      NETCFG_TYPE_CAN_NOT_ADD  -- Unknown condition
 *      NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ROUTE_GetRunningIpForwarding(UI32_T vr_id, UI8_T addr_type, BOOL_T
*status);


/* FUNCTION NAME : NETCFG_PMGR_ROUTE_EnableIpForwarding
 * PURPOSE:
 *      Enable IPv4/IPv6 forwarding function.
 *
 * INPUT:
 *      vr_id	    : virtual router id
 *      address_type: IPv4/IPv6 to enable
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK   -- Successfully enable the IPv4/IPv6 forwarding function.
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 *      NETCFG_TYPE_DESTINATION_IS_LOCAL
 *      NETCFG_TYPE_TABLE_FULL
 *      NETCFG_TYPE_CAN_NOT_ADD  -- Unknown condition
 *      NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 */
UI32_T NETCFG_PMGR_ROUTE_EnableIpForwarding(UI32_T vr_id,
                                 UI8_T addr_type);

/* FUNCTION NAME : NETCFG_PMGR_ROUTE_DisableIpForwarding
 * PURPOSE:
 *      Disable IPv4/IPv6 forwarding function.
 *
 * INPUT:
 *      vr_id	    : virtual router id
 *      address_type: IPv4/IPv6 to enable
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK   -- Successfully disable the IPv4/IPv6 forwarding function.
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 *      NETCFG_TYPE_DESTINATION_IS_LOCAL
 *      NETCFG_TYPE_TABLE_FULL
 *      NETCFG_TYPE_CAN_NOT_ADD  -- Unknown condition
 *      NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 */
UI32_T NETCFG_PMGR_ROUTE_DisableIpForwarding(UI32_T vr_id,
                                 UI8_T addr_type);

/* FUNCTION NAME : NETCFG_PMGR_ROUTE_GetIpForwardingStatus
 * PURPOSE:
 *      Retrieve IPv4/IPv6 forwarding function status.
 *
 * INPUT:
 *      vr_id	    : virtual router id
 *      address_type: IPv4/IPv6 to query
 *
 * OUTPUT:
 *      *forward_status_p   -- 1 (IP_FORWARDING_ENABLED)
 *                             0 (IP_FORWARDING_DISABLED)
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T NETCFG_PMGR_ROUTE_GetIpForwardingStatus(UI32_T vr_id,
                                 UI8_T addr_type, UI32_T *forward_status_p);



/* FUNCTION NAME : NETCFG_PMGR_ROUTE_AddStaticRoute
 * PURPOSE:
 *      Add a static route entry to routing table.
 *
 * INPUT:
 *      ip          : IP address of the route entry. (network order)
 *      mask        : IP mask of the route entry.(network order)
 *      next_hop    : the ip for this subnet.(network order)
 *      distance    : Administrative distance
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK   -- Successfully add the entry to routing table.
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 *      NETCFG_TYPE_DESTINATION_IS_LOCAL
 *      NETCFG_TYPE_TABLE_FULL
 *      NETCFG_TYPE_CAN_NOT_ADD  -- Unknown condition
 *      NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 *      1. Key is (ip, mask)
 *      2. vid_ifIndex need or not ?
 *         if no need vid_ifIndex, need to change CLI.
 *         or define new function to support CLI.
 *      3. So, define new function for Cisco command line.
 */
UI32_T NETCFG_PMGR_ROUTE_AddStaticRoute(L_INET_AddrIp_T *ip,
                                 L_INET_AddrIp_T *next_hop,
                                 UI32_T distance);

#if (SYS_CPNT_IP_TUNNEL == TRUE)
UI32_T NETCFG_PMGR_ROUTE_AddStaticRouteToTunnel(L_INET_AddrIp_T *ip, UI32_T tid_ifindex);
UI32_T NETCFG_PMGR_ROUTE_DeleteStaticRouteToTunnel(L_INET_AddrIp_T *ip, UI32_T tid_ifindex);
#endif /*SYS_CPNT_IP_TUNNEL*/
/* FUNCTION NAME : NETCFG_PMGR_ROUTE_DeleteStaticRoute
 * PURPOSE:
 *      Delete a static route entry from routing table.
 *
 * INPUT:
 *      ip          : IP address of the route entry. (network order)
 *      mask        : IP mask of the route entry.(network order)
 *      next_hop    : the ip for this subnet.(network order)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK           -- Successfully delete the entry from routing table.
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_CAN_NOT_DELETE -- Unknown condition
 *
 * NOTES:
 *      1. Key is (ip, mask, nexthop, distance)
 */
UI32_T NETCFG_PMGR_ROUTE_DeleteStaticRoute(L_INET_AddrIp_T *ip,
                                 L_INET_AddrIp_T *next_hop);


/* FUNCTION NAME : NETCFG_PMGR_ROUTE_DeleteStaticRouteByNetwork
 * PURPOSE:
 *      Delete a static route entry from routing table.
 *
 * INPUT:
 *      ip   : IP address of the route entry.(network order)
 *      mask : IP mask of the route entry.(network order)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK            -- Successfully add the entry to routing table.
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_DELETE -- Unknown condition
 *
 * NOTES:
 *      1. Key is (ip, mask)
 */
UI32_T NETCFG_PMGR_ROUTE_DeleteStaticRouteByNetwork(UI8_T ip[SYS_ADPT_IPV4_ADDR_LEN], UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN]);


/* FUNCTION NAME : NETCFG_PMGR_ROUTE_DeleteAllStaticIpCidrRoutes
 * PURPOSE:
 *      Delete all static routes.
 *
 * INPUT:
 *       None.
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_PMGR_ROUTE_DeleteAllStaticIpCidrRoutes(UI32_T action_flags);


/* FUNCTION NAME: NETCFG_PMGR_ROUTE_SetIpCidrRouteRowStatus
 *
 * PURPOSE:
 *      Change the row status field of the route entry
 *
 * INPUT:
 *      ip_addr     : IP address of rif. (network order)
 *      ip_mask     : Mask of rif.  (network order)
 *      tos         : type of service. (must be 0).
 *      next_hop    : NextHop of route entry. (network order)
 *      row_status  :
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_SET
 *
 * NOTES:
 *      1.row_status must be one of the following values:
 *            VAL_ipCidrRouteStatus_active
 *            VAL_ipCidrRouteStatus_notInService
 *            VAL_ipCidrRouteStatus_createAndGo
 *            VAL_ipCidrRouteStatus_createAndWait
 *            VAL_ipCidrRouteStatus_destroy
 *
 *      2.Currently, only createAndGo and destroy are supported.
 */
UI32_T NETCFG_PMGR_ROUTE_SetIpCidrRouteRowStatus(UI8_T ip_addr[SYS_ADPT_IPV4_ADDR_LEN],
                                          UI8_T ip_mask[SYS_ADPT_IPV4_ADDR_LEN],
                                          UI32_T tos,
                                          UI8_T next_hop[SYS_ADPT_IPV4_ADDR_LEN],
                                          UI32_T row_status);


/* FUNCTION NAME : NETCFG_PMGR_ROUTE_GetReversePathIpMac
 * PURPOSE:
 *      If a user is managing the DUT, we will log the src IP (target IP) and mac-address.
 *      If the target IP is in local subnet, then return target IP and target Mac
 *      If the target IP is not in local subnet, return reverse path nexthop IP and Mac.
 * INPUT:
 *      target_addr_p   -- target address to be checked.
 *
 * OUTPUT:
 *      nexthop_addr_p      -- nexthop address
 *      nexthop_mac_addr_p  -- mac-address of nexthop address
 *
 * RETURN:
 *      NETCFG_TYPE_OK      -- if success
 *      NETCFG_TYPE_FAIL    -- if fail
 *
 * NOTES:
 */
UI32_T NETCFG_PMGR_ROUTE_GetReversePathIpMac(L_INET_AddrIp_T *target_addr_p, L_INET_AddrIp_T *nexthop_addr_p, UI8_T *nexthop_mac_addr_p);

/* FUNCTION NAME: NETCFG_PMGR_ROUTE_SetMRouteStatus
 * PURPOSE:
 *      Enable/disable muticast routing.
 *
 * INPUT:
 *      status  -- true: enable; false: disable
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 *
 * NOTES:
 *
 */
UI32_T NETCFG_PMGR_ROUTE_SetMRouteStatus(UI32_T status);

UI32_T NETCFG_PMGR_ROUTE_GetRunningMRouteEnableStatus(UI32_T *status_p );

/* FUNCTION NAME: NETCFG_PMGR_ROUTE_SetM6RouteStatus
 * PURPOSE:
 *      Enable/disable muticast routing.
 *
 * INPUT:
 *      status  -- true: enable; false: disable
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 *
 * NOTES:
 *
 */
UI32_T NETCFG_PMGR_ROUTE_SetM6RouteStatus(UI32_T status);

/* FUNCTION NAME: NETCFG_PMGR_ROUTE_GetM6RouteStatus
 * PURPOSE:
 *      Get multicast routing running status
 * INPUT:
 * OUTPUT:
 *      status_p  -- true: enable; false: disable
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 * NOTES:
 */
UI32_T NETCFG_PMGR_ROUTE_GetM6RouteStatus(UI32_T *status_p );

/* FUNCTION NAME: NETCFG_PMGR_ROUTE_GetRunningM6RouteEnableStatus
 * PURPOSE:
 *      Get multicast routing running status
 * INPUT:
 * OUTPUT:
 *      status_p  -- true: enable; false: disable
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 * NOTES:
 */
UI32_T NETCFG_PMGR_ROUTE_GetRunningM6RouteEnableStatus(UI32_T *status_p );

/* FUNCTION NAME : NETCFG_PMGR_ROUTE_FindBestRoute
 * PURPOSE:
 *      Find the best route for the specified destionation IP address.
 *
 * INPUT:
 *      dest_ip_p   -- destionation IP address to be checked.
 *
 * OUTPUT:
 *      nexthop_ip_p -- next hop of forwarding.
 *      nexthop_if_p -- the routing interface which next hope belong to.
 *      owner        -- who generates the routing entry. ie. static, RIP or OSPF.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T NETCFG_PMGR_ROUTE_FindBestRoute(
    L_INET_AddrIp_T *dest_ip_p, L_INET_AddrIp_T *nexthop_ip_p, UI32_T *nexthop_if_p, UI32_T *owner);

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ROUTE_SetEcmpBalanceMode
 *-----------------------------------------------------------------------------
 * PURPOSE : To set ecmp load balance mode.
 * INPUT   : mode - which mode to set
 *           idx  - which idx to set if mode is NETCFG_TYPE_ECMP_HASH_SELECTION
 * OUTPUT  : None
 * RETURN  : NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 * NOTES   : None
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ROUTE_SetEcmpBalanceMode(UI32_T mode, UI32_T idx);

#endif /* #if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE) */

#if (SYS_CPNT_STATIC_ROUTE_AND_METER_WORKAROUND == TRUE)
UI32_T NETCFG_PMGR_ROUTE_EnableSWRoute(UI32_T status);

UI32_T NETCFG_PMGR_ROUTE_GetSWRoute(UI32_T *status);
#endif

#endif /* NETCFG_PMGR_ROUTE_H */

