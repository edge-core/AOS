/*
 *   File Name: ipal_neigh.h
 *   Purpose:   TCP/IP shim layer(ipal) Neighbor management implementation API
 *   Note:
 *   Create:    Basen LV     2008.04.06
 *
 *   Histrory:
 *              Modify		   Date      Reason
 *              kh_shi      2008/10/23  ipal_arp_mgr.h --> ipal_neigh.h
 *
 *
 *   Copyright(C)  Accton Corporation 2007~2009
 */

#ifndef __IPAL_NEIGH_H
#define __IPAL_NEIGH_H

#include "sys_type.h"
#include "leaf_es3626a.h"
#include <linux/neighbour.h>

/*
 * INCLUDE FILE DECLARATIONS
 */


/*
 * NAMING CONST DECLARATIONS
 */
/* NUD_XXX defined in <linux/neighbour.h> */
#define IPAL_NEIGH_NUD_PERMANENT NUD_PERMANENT
#define IPAL_NEIGH_NUD_NOARP     NUD_NOARP
#define IPAL_NEIGH_NUD_REACHABLE NUD_REACHABLE
#define IPAL_NEIGH_NUD_PROBE     NUD_PROBE
#define IPAL_NEIGH_NUD_STALE     NUD_STALE
#define IPAL_NEIGH_NUD_DELAY     NUD_DELAY

/*
 * MACRO FUNCTION DECLARATIONS
 */


/*
 * DATA TYPE DECLARATIONS
 */
typedef enum IPAL_NeighborType_E
{
    IPAL_NEIGHBOR_TYPE_STATIC = VAL_ipNetToPhysicalExtType_static,
    IPAL_NEIGHBOR_TYPE_DYNAMIC = VAL_ipNetToPhysicalExtType_dynamic,
    IPAL_NEIGHBOR_TYPE_INVALID = VAL_ipNetToPhysicalExtType_invalid,
    IPAL_NEIGHBOR_TYPE_MAX
} IPAL_NeighborType_T;


typedef enum IPAL_NeighborStatisticType_E
{
    IPAL_NEIGHBOR_STATISTIC_TYPE_STATIC_ENTRIES_NUM,
    IPAL_NEIGHBOR_STATISTIC_TYPE_DYNAMIC_ENTRIES_NUM,
    IPAL_NEIGHBOR_STATISTIC_TYPE_MAX
} IPAL_NeighborStatisticType_T;

typedef enum IPAL_NeighborAddressType_E
{
    IPAL_NEIGHBOR_ADDR_TYPE_IPV4 = 1,
    IPAL_NEIGHBOR_ADDR_TYPE_IPV6,
    IPAL_NEIGHBOR_ADDR_TYPE_MAX
} IPAL_NeighborAddressType_T;
/*
 * STATIC VARIABLE DECLARATIONS
 */


/*
 * LOCAL SUBPROGRAM DECLARATIONS
 */


/* FUNCTION NAME : IPAL_NB_AddNeighbor
 * PURPOSE:
 *      Add a new entry in neighbor table
 * INPUT:
 *      ifindex     -- L3 interface index
 *      ip_addr_p   -- interface ip address in L_INET_AddrIp_T format
 *      phy_len     -- physical address (MAC) length
 *      phy_addr_p  -- physical address (MAC)
 *      neigh_type  -- neighbor entry type (static, dynamic)
 *                         IPAL_ARP_TYPE_STATIC   : static neighbor
 *                         IPAL_ARP_TYPE_DYNAMIC  : dynamic neighbor
 *      replace_if_exist -- whether replace if entry already exist
 * OUTPUT:
 *      None
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail to add  neighbor entry
 * NOTES:
 */
UI32_T IPAL_NEIGH_AddNeighbor(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p, UI32_T phy_len,
                    UI8_T *phy_addr_p, IPAL_NeighborType_T neigh_type, BOOL_T replace_if_exist);


/* FUNCTION NAME : IPAL_NB_DeleteNeighbor
 * PURPOSE:
 *      Delete an entry in neighbor table
 * INPUT:
 *      ifindex    -- L3 interface index
 *      ip_addr_p  -- L3 interface ip address in L_INET_AddrIp_T format
 * OUTPUT:
 *      None.
 * RETURN:
 *      IPAL_RESULT_OK   -- success
 *      IPAL_RESULT_FAIL -- fail to delete neighbor entry
 * NOTES:
 */
UI32_T IPAL_NEIGH_DeleteNeighbor(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p);

#if (SYS_CPNT_PROXY_ARP == TRUE)
/* FUNCTION NAME : IPAL_NB_EnableIpv4ProxyArp
 * PURPOSE:
 *      Enable IPv4 Proxy ARP for a L3 interface
 * INPUT:
 *      ifindex    -- L3 interface index
 * OUTPUT:
 *      None.
 * RETURN:
 *      IPAL_RESULT_OK   -- success
 *      IPAL_RESULT_FAIL --  fail to enable Proxy ARP
 * NOTES:
 */
UI32_T IPAL_NEIGH_EnableIpv4ProxyArp(UI32_T ifindex);

/* FUNCTION NAME : IPAL_NB_DisableIpv4ProxyArp
 * PURPOSE:
 *     disable IPv4 Proxy ARP for a L3 interface
 * INPUT:
 *      ifindex    -- L3 interface index
 * OUTPUT:
 *      None
 * RETURN:
 *      IPAL_RESULT_OK   -- success
 *      IPAL_RESULT_FAIL -- fail to disable Proxy ARP
 * NOTES:
 */
UI32_T IPAL_NEIGH_DisableIpv4ProxyArp(UI32_T ifindex);
#endif /* #if (SYS_CPNT_PROXY_ARP == TRUE) */

/* FUNCTION NAME : IPAL_NB_GetIpv4ProxyArp
 * PURPOSE:
 *      Get IPv4 Proxy ARP status from a L3 interface
 * INPUT:
 *      ifindex    -- the L3 interface index
 * OUTPUT:
 *      status_p   -- the interface Proxy ARP status
 * RETURN:
 *      IPAL_RESULT_OK   -- success
 *      IPAL_RESULT_FAIL --  fail to get the interface Proxy ARP status
 * NOTES:
 */
UI32_T IPAL_NEIGH_GetIpv4ProxyArp(UI32_T ifindex, BOOL_T *status_p);

#if (SYS_CPNT_IPV6 == TRUE)
/* FUNCTION NAME : IPAL_NB_EnableIPv6ProxyNdp
 * PURPOSE:
 *      Enable IPv6 Proxy NDP for a L3 interface
 * INPUT:
 *      ifindex    -- the L3 interface index
 * OUTPUT:
 *      None.
 * RETURN:
 *      IPAL_RESULT_OK   -- success
 *      IPAL_RESULT_FAIL --  fail to enable Proxy NDP
 * NOTES:
 */
UI32_T IPAL_NEIGH_EnableIpv6ProxyNdp(UI32_T ifindex);


/* FUNCTION NAME : IPAL_NEIGH_DisableIPv6ProxyNdp
 * PURPOSE:
 *     disable IPv6 Proxy ARP for a L3 interface
 * INPUT:
 *      ifindex    -- the L3 interface index
 * OUTPUT:
 *      None
 * RETURN:
 *      IPAL_RESULT_OK   -- success
 *      IPAL_RESULT_FAIL -- fail to disable Proxy NDP
 * NOTES:
 */
UI32_T IPAL_NEIGH_DisableIpv6ProxyNdp(UI32_T ifindex);


/* FUNCTION NAME : IPAL_NEIGH_GetIPv6ProxyNdp
 * PURPOSE:
 *      Get IPv6 Proxy NDP status from a L3 interface
 * INPUT:
 *      ifindex    -- the L3 interface index
 * OUTPUT:
 *      status_p   -- the interface Proxy NDP status
 * RETURN:
 *      IPAL_RESULT_OK   -- success
 *      IPAL_RESULT_FAIL --  fail to get the interface Proxy NDP status
 * NOTES:
 */
UI32_T IPAL_NEIGH_GetIpv6ProxyNdp(UI32_T ifindex, BOOL_T *status_p);
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */


/* FUNCTION NAME : IPAL_NEIGH_GetNeighbor
 * PURPOSE:
 *      Get an neighbor entry from TCP/IP stack neighbor table
 * INPUT:
 *      ifindex     -- L3 interface index
 *      ip_addr_p   -- ip address in L_INET_AddrIp_T format
 * OUTPUT:
 *      neighbor_p  --   neighbor entry in IPAL_NeighborEntry_T format
 * RETURN:
 *      IPAL_RESULT_OK   -- success
 *      IPAL_RESULT_FAIL --  fail to get neighbor entry
 * NOTES:
 */
UI32_T IPAL_NEIGH_GetNeighbor(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p, IPAL_NeighborEntry_T *neighbor_p);


/* FUNCTION NAME : IPAL_NEIGH_GetNextNNeighbor
 * PURPOSE:
 *      Get next N neighbor entries from TCP/IP stack neighbor table
 * INPUT:
 *      ifindex    -- L3 interface index
 *      ip_addr_p  -- ip address in L_INET_AddrIp_T format
 *      num        -- the number of request neighbor entries
 * OUTPUT:
 *      neighbor_p ---- N neighbor entries in IPAL_NeighborEntry_T format
 * RETURN:
 *      IPAL_RESULT_OK  -- success
 *      IPAL_RESULT_FAIL  -- fail to get next N neighbor entries in neighbor table
 * NOTES:
 *      1. The output neightbor entry is the next N entries in neighbor table if the input
 *           ifindex and ip_addr can exactly match an entry in the table
 */
UI32_T IPAL_NEIGH_GetNextNNeighbor(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p, UI32_T *num_p, IPAL_NeighborEntry_T *neighbor_p);


/* FUNCTION NAME : IPAL_NEIGH_GetNeighborTableStatistic
 * PURPOSE:
 *     Get statistic of the neighbor table
 * INPUT:
 *     stat_type  -- neighbor table statistic type (IPAL_NeighborStatisticType_T)
 * OUTPUT:
 *     value_p    -- retrieved neighbor table statistic value
 * RETURN:
 *     IPAL_RESULT_OK  -- success
 *     IPAL_RESULT_FAIL  -- Fail to get statistic of the neighbor table
 * NOTES:
 *     1. Currently unavailable
 */
UI32_T IPAL_NEIGH_GetNeighborTableStatistic(IPAL_NeighborStatisticType_T stat_type, UI32_T *value_p);


/* FUNCTION NAME : IPAL_NEIGH_FlushAllDynamicNeighbor
 * PURPOSE:
 *     Flush all dynamic neighbor entries in neighbor table
 * INPUT:
 *     addr_type  -- The address family to clear
 *                   IPAL_NEIGHBOR_ADDR_TYPE_IPV4 -- IPV4
 *   	             IPAL_NEIGHBOR_ADDR_TYPE_IPV6 -- IPV6
 * OUTPUT:
 *     None
 * RETURN:
 *     IPAL_RESULT_OK  -- success
 *     IPAL_RESULT_FAIL  -- Flush all dynamic neighbor entries fail
 * NOTES:
 */
UI32_T IPAL_NEIGH_FlushAllDynamicNeighbor(IPAL_NeighborAddressType_T addr_type);


/* FUNCTION NAME : IPAL_NEIGH_FlushAllNeighbor
 * PURPOSE:
 *     Flush all neighbor entries in neighbor table
 * INPUT:
 *     None
 * OUTPUT:
 *     None
 * RETURN:
 *     IPAL_RESULT_OK -- success
 *     IPAL_RESULT_FAIL -- Flush all neighbor entries fail
 * NOTES:
 */
UI32_T IPAL_NEIGH_FlushAllNeighbor();


/* FUNCTION NAME : IPAL_NEIGH_SetNeighborAgingTime
 * PURPOSE:
 *      Set aging time of neighbor table in TCP/IP stack
 * INPUT:
 *      aging_time   --- aging time of neighbor table in millisecond
 * OUTPUT:
 *      None
 * RETURN:
 *      IPAL_RESULT_OK -- success
 *      IPAL_RESULT_FAIL ¡V set aging time fail
 * NOTES:
 */
UI32_T IPAL_NEIGH_SetNeighborAgingTime(UI32_T aging_time);


/* FUNCTION NAME : IPAL_NEIGH_GetNeighborAgingTime
 * PURPOSE:
 *      Get neighbor table aging time from TCP/IP stack
 * INPUT:
 *      None
 * OUTPUT:
 *      aging_time_p   --- aging time of neighbor table in millisecond
 * RETURN:
 *      IPAL_RESULT_OK -- success
 *      IPAL_RESULT_FAIL -- get aging time fail
 * NOTES:
 */
UI32_T IPAL_NEIGH_GetNeighborAgingTime(UI32_T *aging_time_p);

/*
 * Get ARP statistics
 */
UI32_T IPAL_NEIGH_GetIpv4ArpStatistics(IPAL_Ipv4ArpStatistics_T *arp_stat_p);

/* FUNCTION NAME : IPAL_NEIGH_SendNeighborRequest
 * PURPOSE:
 *      Send a neighbour request (ARP request/Neighbor solicitation)
 * INPUT:
 *      ifindex     --  the L3 interface index
 *      dst_addr_p  --  requested destination address
 * OUTPUT:
 *      None
 * RETURN:
 *      IPAL_RESULT_OK -- success
 *      IPAL_RESULT_FAIL -- get aging time fail
 * NOTES:
 */
UI32_T IPAL_NEIGH_SendNeighborRequest(UI32_T ifindex, L_INET_AddrIp_T *dst_addr_p);

UI32_T IPAL_NEIGH_ShowIpv4NeighTable();

UI32_T IPAL_NEIGH_SendArpRequest(UI32_T ifindex, L_INET_AddrIp_T *dst_addr_p);

#if (SYS_CPNT_IPV6 == TRUE)
UI32_T IPAL_NEIGH_ShowIpv6NeighTable();
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

#endif /* end of __IPAL_NEIGH_H */
