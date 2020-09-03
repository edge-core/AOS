/*
 *   File Name: ipal_rt_netlink.c
 *   Purpose:   TCP/IP shim layer(ipal) NETLINK ROUTE Implementation
 *   Note:
 *   Create:    Basen LV     2008.04.06
 *
 *   Histrory:
 *              Modify         Date      Reason
 *
 *
 *   Copyright(C)  Accton Corporation 2007~2009
 */

#ifndef __IPAL_RT_NETLINK_H
#define __IPAL_RT_NETLINK_H

/*
 * INCLUDE FILE DECLARATIONS
 */
#include "ipal_reflect.h"
#include "l_sort_lst.h"

#include <linux/rtnetlink.h>

/*
 * NAMING CONST DECLARATIONS
 */


/*
 * MACRO FUNCTION DECLARATIONS
 */



/*
 * DATA TYPE DECLARATIONS
 */

#if 0
/*
 *  IPAL ROUTE NETLINK Add IPv4 Arp
 */
UI32_T IPAL_Rt_Netlink_AddIPv4Arp (const IPAL_IPv4_ARP_T *arp);

/*
 *  IPAL ROUTE NETLINK Delete IPv4 Arp
 */
UI32_T IPAL_Rt_Netlink_DeleteIPv4Arp (const IPAL_IPv4_ARP_T *arp);
#endif

/*
 *  IPAL ROUTE NETLINK Add IPv4/IPv6 neighbor
 */
UI32_T IPAL_Rt_Netlink_AddNeighbor(const IPAL_NeighborEntry_T *neigh, BOOL_T replace_if_exist);

/*
 *  IPAL ROUTE NETLINK Delete IPv4/IPv6 neighbor
 */
UI32_T IPAL_Rt_Netlink_DeleteNeighbor(const IPAL_NeighborEntry_T *neigh);

/*
 *  IPAL ROUTE NETLINK Add IPv4 Route
 */
UI32_T IPAL_Rt_Netlink_AddIpv4Route (UI32_T table_id, const L_INET_AddrIp_T *p, const IPAL_Ipv4UcNextHop_T *nhs);

/*
 *  IPAL ROUTE NETLINK Delete Ipv4 Route
 */
UI32_T IPAL_Rt_Netlink_DeleteIpv4Route (UI32_T table_id, const L_INET_AddrIp_T *p, const IPAL_Ipv4UcNextHop_T *nhs);

#if (SYS_CPNT_IPV6 == TRUE)
/*
 *  IPAL ROUTE NETLINK Add IPv6 Route
 */
UI32_T IPAL_Rt_Netlink_AddIpv6Route (UI32_T table_id, const L_INET_AddrIp_T *p, const IPAL_Ipv6UcNextHop_T *nhs);

/*
 *  IPAL ROUTE NETLINK Delete Ipv6 Route
 */
UI32_T IPAL_Rt_Netlink_DeleteIpv6Route (UI32_T table_id, const L_INET_AddrIp_T *p, const IPAL_Ipv6UcNextHop_T *nhs);

#endif

/*
 *  IPAL ROUTE NETLINK Add Network Interface IPv4 Address
 */
UI32_T IPAL_Rt_Netlink_AddIfIpAddress (UI32_T ifindex, const L_INET_AddrIp_T *p);

/*
 *  IPAL ROUTE NETLINK Add Network Interface alias IP Address
 */
UI32_T IPAL_Rt_Netlink_AddIfIpAddressAlias (UI32_T ifindex, const L_INET_AddrIp_T *p,UI32_T alias_id);

/*
 *  IPAL ROUTE NETLINK Delete Network Interface IPv4 Address
 */
UI32_T IPAL_Rt_Netlink_DeleteIfIpAddress (UI32_T ifindex, const L_INET_AddrIp_T *p);

/*
 *  IPAL ROUTE NETLINK Delete Network Interface alias IP Address
 */
UI32_T IPAL_Rt_Netlink_DeleteIfIpAddressAlias (UI32_T ifindex, const L_INET_AddrIp_T *p, UI32_T alias_id);

/*
 *  IPAL ROUTE NETLINK Fetch All IPv4 Arp
 */
#if 0
UI32_T IPAL_Rt_Netlink_FetchAllIPv4Arp (IPAL_IPv4_ARP_LIST_T *arp_list);
#endif

/*
 *  IPAL NETLINK Fetch All IPv4 Neighbor
 */
UI32_T IPAL_Rt_Netlink_FetchAllIpv4Neighbor (L_SORT_LST_List_T *neigh_list_p);

/*
 *Fetch all ipv4 address.
 */
UI32_T IPAL_Rt_Netlink_FetchAllIpv4Addr (IPAL_Ipv4AddressList_T **addr_list_pp);


UI32_T IPAL_Rt_Netlink_FetchAllIpv4Route (IPAL_Ipv4UcRouteList_T **route_list_pp);


UI32_T IPAL_Rt_Netlink_FetchAllIpv4Interface (IPAL_IfInfoList_T **if_list_pp);

UI32_T IPAL_Rt_Netlink_RouteLookup(L_INET_AddrIp_T *dest_ip_p, L_INET_AddrIp_T *src_ip_p,
                                   L_INET_AddrIp_T *nexthop_ip_p, UI32_T *nexthop_if_p);


#if (SYS_CPNT_IPV6 == TRUE)
/*
 *  IPAL NETLINK Fetch All IPv6 Neighbor
 */
UI32_T IPAL_Rt_Netlink_FetchAllIpv6Neighbor (L_SORT_LST_List_T *neigh_list_p);

/*
 *Fetch all ipv6 address.
 */
UI32_T IPAL_Rt_Netlink_FetchAllIpv6Addr (IPAL_Ipv6AddressList_T **addr_list_pp);

UI32_T IPAL_Rt_Netlink_FetchAllIpv6Route (IPAL_Ipv6UcRouteList_T **route_list_pp);

UI32_T IPAL_Rt_Netlink_FetchAllIpv6RouteCache (IPAL_Ipv6UcRouteList_T **route_list_pp);

UI32_T IPAL_Rt_Netlink_FetchAllIpv6Interface (IPAL_IfInfoList_T **if_list_pp);

UI32_T IPAL_Rt_Netlink_DeleteIpv6Route (UI32_T table_id, const L_INET_AddrIp_T *p, const IPAL_Ipv6UcNextHop_T *nhs);

#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

UI32_T IPAL_Rt_Netlink_IF_Info (struct sockaddr_nl *snl, struct nlmsghdr *h, IPAL_IfInfo_T *if_info_entry);


/*
 *  IPAL ROUTE NETLINK Initialization
 */
void IPAL_Rt_Netlink_Init ();

UI32_T IPAL_Rt_RecvMsg(IPAL_ReflectMesg_T *reflect_msg_p);

#endif  /*end of __IPAL_RT_NETLINK_H*/
