/* Module Name: ROUTE_MGR.H
 * Purpose: To provide some APIs for other components to access Phase2 engine.
 *
 * Notes:
 *      1.
 *
 *
 * History:
 *       Date       -- 	Modifier,  	Reason
 *  0.1 2002.02.26  --  Czliao, 	Created
 *      2007.06.29  --  charlie_chen        port to linux platform
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001, 2002.
 */

#ifndef     _ROUTE_MGR_H__
#define     _ROUTE_MGR_H__

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_adpt.h"
#include "sys_bld.h"
#include "sys_type.h"
#include "l_threadgrp.h"
#include "l_inet.h"

/* NAME CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define VALID       TRUE
#define INVALID     FALSE

/* DATA TYPE DECLARATIONS
 */
typedef struct ROUTE_MGR_IpCidrRouteEntry_S {
//	UI8_T	ip_cidr_route_dest[SYS_ADPT_IPV4_ADDR_LEN];
//	UI8_T	ip_cidr_route_mask[SYS_ADPT_IPV4_ADDR_LEN];
//	UI8_T	ip_cidr_route_next_hop[SYS_ADPT_IPV4_ADDR_LEN];
	L_INET_AddrIp_T	ip_cidr_route_dest;
	L_INET_AddrIp_T	ip_cidr_route_next_hop;
	UI32_T	ip_cidr_route_if_index;
	UI32_T	ip_cidr_route_metric;
    UI32_T  ip_cidr_route_distance;
    UI16_T  ip_cidr_route_status;   /* valid/invalid */
    I32_T   ip_cidr_route_tos;
    int     ip_cidr_route_type;
    int     ip_cidr_route_proto;
    I32_T   ip_cidr_route_age;
    I32_T   ip_cidr_route_next_hop_as;
    I32_T   ip_cidr_route_metric2;
    I32_T   ip_cidr_route_metric3;
    I32_T   ip_cidr_route_metric4;
    I32_T   ip_cidr_route_metric5;
}   ROUTE_MGR_IpCidrRouteEntry_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME : ROUTE_MGR_InitProcessResources
 * PURPOSE: Initialize process resources for ROUTE_MGR
 *
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
void ROUTE_MGR_InitiateProcessResources(void);

/* FUNCTION NAME : ROUTE_MGR_Create_InterCSC_Relation
 * PURPOSE:
 *      This function initializes all function pointer registration operations.
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
void ROUTE_MGR_Create_InterCSC_Relation(void);

/* FUNCTION	NAME : ROUTE_MGR_Enter_Master_Mode
 * PURPOSE:
 *		Enter Master Mode; could handle ROUTE.
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
 *
 */
void ROUTE_MGR_Enter_Master_Mode(void);

/* FUNCTION	NAME : ROUTE_MGR_Enter_Slave_Mode
 * PURPOSE:
 *		Enter Slave	Mode; ignore any request.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *
 * NOTES:
 *
 */
void ROUTE_MGR_Enter_Slave_Mode(void);

/* FUNCTION	NAME : ROUTE_MGR_Enter_Transition_Mode
 * PURPOSE:
 *		Enter Transition Mode; release all resource of ROUTE.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *
 * NOTES:
 *		1. Reset all modules, release all resources.
 */
void ROUTE_MGR_Enter_Transition_Mode(void);

/*
 * ROUTINE NAME: ROUTE_MGR_EnableIpForwarding
 *
 * FUNCTION: Enable IP Forwarding.
 *
 * INPUT:
 *      vr_id    - ID of Virtal Router
 *      addr_type- IP address Type
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_CAN_NOT_ADD	-- Unknown condition
 *
 * NOTES: None.
 */
UI32_T ROUTE_MGR_EnableIpForwarding(UI32_T vr_id, UI8_T addr_type);

/*
 * ROUTINE NAME: ROUTE_MGR_DisableIpForwarding
 *
 * FUNCTION: Disable IP Forwarding.
 *
 * INPUT:
 *      vr_id    - ID of Virtal Router
 *      addr_type- IP address Type
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_CAN_NOT_ADD	-- Unknown condition
 *
 * NOTES: None.
 */
UI32_T ROUTE_MGR_DisableIpForwarding(UI32_T vr_id, UI8_T addr_type);

/*
 * ROUTINE NAME: ROUTE_MGR_AddStaticIpCidrRoute
 *
 * FUNCTION: Add static route.
 *
 * INPUT:
 *      fib_id   - FIB id
 *      dest     - destination prefix
 *      next_hop - next hop address
 *      if_index - egress ifindex
 *      distance - administrative distance
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_CAN_NOT_ADD -- Unknown condition
 *
 * NOTES: None.
 */
UI32_T ROUTE_MGR_AddStaticIpCidrRoute(UI32_T fib_id, L_INET_AddrIp_T *dest_p, L_INET_AddrIp_T *next_hop_p, UI32_T if_index, UI32_T distance);

#if (SYS_CPNT_IPV6 == TRUE)
/*
 * ROUTINE NAME: ROUTE_MGR_DeleteIPv6StaticIpCidrRoute
 *
 * FUNCTION: Delete static route.
 *
 * INPUT:
 *      ip_cidr_route_dest      - IP address of static route
 *      ip_cidr_route_mask      - IP mask of static route
 *      ip_cidr_route_next_hop  - next hop
 *      ip_cidr_route_distance  - administrative distance
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK           -- Successfully add the entry to routing table.
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *
 * NOTES:
 *      If the route is not static, reject this function.
 *
 */
UI32_T ROUTE_MGR_DeleteIPv6StaticIpCidrRoute(UI32_T action_flags, UI32_T fib_id, L_INET_AddrIp_T *ip_cidr_route_dest, L_INET_AddrIp_T *ip_cidr_route_next_hop, UI32_T ip_cidr_route_distance);

#endif

/*
 * ROUTINE NAME: ROUTE_MGR_ModifyStatic
 *
 * FUNCTION: Modify static route.
 *
 * INPUT: ip_cidr_route_dest    - IP address of static route
 *        ip_cidr_route_mask    - IP mask of static route
 *        ip_cidr_route_next_hop- next hop
 *        ip_cidr_route_metric  - metric
 *
 * OUTPUT: None.
 *
 * RETURN: TRUE  - success
 *         FALSE - fail
 *
 * NOTES: None.
 */
BOOL_T ROUTE_MGR_ModifyStaticIpCidrRouteEntry(UI32_T fib_id, L_INET_AddrIp_T *ip_cidr_route_dest, L_INET_AddrIp_T *ip_cidr_route_next_hop, UI32_T ip_cidr_route_distance);

/*
 * ROUTINE NAME: ROUTE_MGR_DeleteStaticIpCidrRoute
 *
 * FUNCTION: Delete static route.
 *
 * INPUT:
 *      fib_id     - FIB id
 *      dest_p     - destination prefix
 *      next_hop_p - next hop address
 *      ifindex    - egress ifindex
 *      distance   - administrative distance
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      If the route is not static, reject this function.
 *
 */
UI32_T ROUTE_MGR_DeleteStaticIpCidrRoute(UI32_T fib_id, L_INET_AddrIp_T *dest_p, L_INET_AddrIp_T *next_hop_p, UI32_T if_index, UI32_T distance);

/* FUNCTION NAME : ROUTE_MGR_DeleteDynamicIpCidrRouteEntry
 * PURPOSE:
 *      Delete specified entry in routing table.
 *
 * INPUT:
 *      ip_addr : IP address of this route entry.
 *      ip_mask : IP mask of this route mask.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      NETCFG_TYPE_OK	         -- Successfully add the entry to routing table.
 *	    NETCFG_TYPE_ENTRY_NOT_EXIST
 *	    NETCFG_TYPE_INVALID_ARG
 *	    NETCFG_TYPE_CAN_NOT_DELETE -- Unknown condition
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 *      If the route is not dynamic, reject this function.
 */
UI32_T ROUTE_MGR_DeleteDynamicRoute(UI32_T fib_id, L_INET_AddrIp_T *ip_cidr_route_dest);

/*
 * ROUTINE NAME: ROUTE_MGR_SetDefaultRoute
 *
 * FUNCTION: Set default route.
 *
 * INPUT:
 *      fib_id           - FIB id
 *      default_route_p  - default route address
 *      metrics          - metrics
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_CAN_NOT_ADD
 *
 * NOTES:
 *      None.
 */
UI32_T ROUTE_MGR_SetDefaultRoute(UI32_T fib_id, L_INET_AddrIp_T *default_route_p, UI32_T metrics);

/* ROUTINE NAME: ROUTE_MGR_DeleteDefaultRoute
 *
 * FUNCTION:
 *      Delete default route.
 *
 * INPUT:
 *      default_route_p - IP address of default gateway
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T ROUTE_MGR_DeleteDefaultRoute(L_INET_AddrIp_T *default_route_p);

/* FUNCTION NAME : ROUTE_MGR_FindBestIPv4Route
 * PURPOSE:
 *      Find the best IPv4 route for the specified destionation IP address.
 * INPUT:
 *      dest_ip   -- destionation IP address to be checked.
 *
 * OUTPUT:
 *      next_hop_ip -- next hop of forwarding.
 *      next_hop_cid-- the routing interface which next hope belong to.
 *      owner -- who generates the routing entry. ie. static, RIP or OSPF.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T ROUTE_MGR_FindBestIPv4Route(UI8_T *dest_ip, UI8_T *next_hop_ip,
                                UI32_T *next_hop_cid, UI32_T *owner);

/* FUNCTION NAME : ROUTE_MGR_FindBestRoute
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
UI32_T ROUTE_MGR_FindBestRoute(L_INET_AddrIp_T *dest_ip_p, L_INET_AddrIp_T *nexthop_ip_p,
                                UI32_T *nexthop_if_p, UI32_T *owner);


/* ROUTINE NAME: ROUTE_MGR_FlushDynamicRoute
 *
 * FUNCTION:
 *      Flush the routing table (remove all the DYNAMIC route).
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *	    NETCFG_TYPE_CAN_NOT_DELETE
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES: None.
 */
UI32_T ROUTE_MGR_FlushDynamicRoute();

#if 0
/* ROUTINE NAME: ROUTE_MGR_FlushDynamicRouteByRif
 *
 * FUNCTION:
 *      Clear dynamic route by rif.
 *
 * INPUT:
 *      ip_addr
 *      ip_mask
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *	    NETCFG_TYPE_CAN_NOT_DELETE
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * Notes:
 *      1. Move rip/route entry to proper location, RIPCFG_SignalRifNotInService()
 *         otherwise, there is a coupling btw ROUTE and RIP.
 *      2. ROUTE handling static route entry, dynamic route entry is handled by
 *         protocol, eg. RIP, OSPF.
 */
UI32_T ROUTE_MGR_FlushDynamicRouteByRif(UI32_T ip_addr, UI32_T ip_mask);
#endif


/* ROUTINE NAME: ROUTE_MGR_FlushStaticRoute
 *
 * FUNCTION: Flush the routing table (remove all the STATIC route).
 *
 * INPUT: None.
 *
 * OUTPUT: None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *	    NETCFG_TYPE_CAN_NOT_DELETE
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES: None.
 */
UI32_T ROUTE_MGR_FlushStaticRoute(UI32_T action_flags);

/* ROUTINE NAME: ROUTE_MGR_GetIpCidrRouteEntry
 *
 * FUNCTION: Get the route in the routing table.
 *
 * INPUT:
 *      route_entry->ip_cidr_route_dest - IP address
 *      route_entry->ip_cidr_route_mask - IP mask
 *      route_entry->ip_cidr_route_next_hop- Next hop IP
 *
 * OUTPUT:
 *      route_entry->ip_cidr_route_metric  - Route metric
 *      route_entry->ip_cidr_route_status  - VALID/INVALID
 *      route_entry->ip_cidr_route_if_num  - Interface number of the route
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_GET
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 *      key(ip_cidr_route_dest, ip_cidr_route_mask, ip_cidr_route_next_hop).
 */
UI32_T ROUTE_MGR_GetIpCidrRouteEntry(ROUTE_MGR_IpCidrRouteEntry_T *route_entry);

/* ROUTINE NAME: ROUTE_MGR_GetNextIpCidrRouteEntry
 *
 * FUNCTION: Get next route (static or dynamic).
 *
 * INPUT:
 *      route_entry->ip_cidr_route_dest
 *      route_entry->ip_cidr_route_mask
 *      route_entry->ip_cidr_route_next_hop  : gateway
 *
 * OUTPUT:
 *      route_entry->ip_cidr_route_dest
 *      route_entry->ip_cidr_route_mask
 *      route_entry->ip_cidr_route_if_index  : interface number.
 *      route_entry->ip_cidr_route_next_hop  : gateway
 *      route_entry->ip_cidr_route_metric    : cost
 *      route_entry->ip_cidr_route_status    : VALID/INVALID
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_NO_MORE_ENTRY
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_GET
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 *      key(ip_cidr_route_dest, ip_cidr_route_mask, ip_cidr_route_next_hop).
 */
UI32_T ROUTE_MGR_GetNextIpCidrRouteEntry(ROUTE_MGR_IpCidrRouteEntry_T *route_entry);

/* ROUTINE NAME: ROUTE_MGR_Rt6MtuChange
 *
 * FUNCTION: Perform action related to interface IPv6 MTU changed
 *
 * INPUT:
 *      ifindex  -- L3 interface index
 *      mtu      -- IPv6 mtu changed to this value
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_UNKNOWN_SYSCALL_CMD
 *
 * NOTES:
 *      1. This function will call rt6_mtu_change(dev, mtu) in Linux kernel.
 *         It will update the PMTU of each route which destination==dev
 *
 */
UI32_T ROUTE_MGR_Rt6MtuChange(UI32_T ifindex, UI32_T mtu);

/* ROUTINE NAME: ROUTE_MGR_SetIPForwarding
 *
 * FUNCTION:
 *
 * INPUT: isForward
 *              TRUE - enable forwarding.
 *              FALSE- disable forwarding.
 *
 * OUTPUT:
 *
 * RETURN: TRUE  - success
 *         FALSE - fail
 *
 * NOTES:
 */
BOOL_T ROUTE_MGR_SetIPForwarding(BOOL_T is_forward);
void ROUTE_MGR_TriggerOspfRoute(UI32_T trigger_ospf_route_flag);
void ROUTE_MGR_GetOspfRouteTriggerTime(UI32_T *ospf_route_trigger_time);

/* for debug */
void ROUTE_MGR_DumpIplrnRoute(void);
#if (SYS_CPNT_NSM == TRUE)
void ROUTE_MGR_DumpZebOSRoute(L_THREADGRP_Handle_T tg_handle, UI32_T member_id);
#endif
void ROUTE_MGR_Add2kRoutes(void);
void ROUTE_MGR_DumpRouteNum(void);
UI8_T ROUTE_MGR_IPv4MaskLen(UI8_T netmask[SYS_ADPT_IPV4_ADDR_LEN]);

#if (SYS_CPNT_IP_TUNNEL == TRUE)
UI32_T ROUTE_MGR_AddStaticIpTunnelRoute(UI32_T fib_id, L_INET_AddrIp_T *ip_cidr_route_dest, UI32_T tid_ifindex, UI32_T ip_cidr_route_distance);
UI32_T ROUTE_MGR_DeleteStaticIpTunnelRoute(UI32_T fib_id, L_INET_AddrIp_T *ip_cidr_route_dest, UI32_T tid_ifindex, UI32_T ip_cidr_route_distance);
#endif /*SYS_CPNT_IP_TUNNEL*/

#endif  /* end of _ROUTE_MGR_H__ */
