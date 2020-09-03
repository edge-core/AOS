/*
 *   File Name: ipal_neigh.c
 *   Purpose:   TCP/IP shim layer(ipal) neighbor management implementation
 *   Note:
 *   Create:    Basen LV     2008.04.06
 *
 *   Histrory:
 *              Modify         Date      Reason
 *              kh_shi       2008/11/26
 *
 *
 *   Copyright(C)  Accton Corporation 2007~2009
 */

/*
 * INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <linux/neighbour.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/if_packet.h>
#include <errno.h>
#include <unistd.h>

#include "sys_type.h"
#include "sysfun.h"
#include "sys_adpt.h"
#include "sys_hwcfg.h"

#include "l_prefix.h"
#include "l_inet.h"
#include "l_math.h"
#include "l_sort_lst.h"

#include "ipal_types.h"
#include "ipal_debug.h"

#include "ipal_sysctl.h"
#include "ipal_rt_netlink.h"
#include "ipal_if.h"
#include "ipal_neigh.h"

#include "vlan_lib.h"
#include "backdoor_mgr.h"

#include "k_amtrl3_mgr.h"

/*
 * NAMING CONST DECLARATIONS
 */
#define ARP_STATS_LINE          "In_ARP_Request"
#define ARP_STATS_PREFIX_LEN    14

#define IPAL_MAX_NUM_OF_NEIGH_ENTRY  0xFFFFFFFF  /* no limit for neighbor num */

/*
 * MACRO FUNCTION DECLARATIONS
 */


/*
 * DATA TYPE DECLARATIONS
 */
typedef struct IPAL_Icmp6NeighSolHdr_S {
    UI8_T       type;
    UI8_T       code;
    UI16_T      checksum;
    UI32_T      reserved;
    UI8_T       target[16];
    UI8_T       opt_type;
    UI8_T       opt_len;        /* in units of 8 octets */
    UI8_T       opt_data_sll[6];
} __attribute__((packed, aligned(1)))IPAL_Icmp6NeighSolHdr_T;


/*
 * STATIC VARIABLE DECLARATIONS
 */


/*
 * LOCAL SUBPROGRAM DECLARATIONS
 */
static UI32_T IPAL_NEIGH_FetchAllIpv4Neighbor(L_SORT_LST_List_T *neigh_list_p);
static UI32_T IPAL_NEIGH_FetchAllIpv6Neighbor(L_SORT_LST_List_T *neigh_list_p);

static int IPAL_NEIGH_Ipv4SortCompareFunc(void* list_node_p, void* comp_node_p);

#if (SYS_CPNT_IPV6 == TRUE)
static int IPAL_NEIGH_Ipv6SortCompareFunc(void* list_node_p, void* comp_node_p);
#endif

/* FUNCTION NAME : IPAL_NEIGH_AddNeighbor
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
 *      IPAL_RESULT_OK          --  success
 *      IPAL_RESULT_ENTRY_EXIST --  entry already exist
 *      IPAL_RESULT_FAIL        --  fail to add  neighbor entry
 * NOTES:
 */
UI32_T IPAL_NEIGH_AddNeighbor(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p, UI32_T phy_len,
                       UI8_T *phy_addr_p, IPAL_NeighborType_T neigh_type, BOOL_T replace_if_exist)
{
    IPAL_NeighborEntry_T neigh_entry;

    if(ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER || ifindex > SYS_ADPT_MAX_VLAN_ID + SYS_ADPT_VLAN_1_IF_INDEX_NUMBER -1)
        return IPAL_RESULT_FAIL;

    memset (&neigh_entry, 0, sizeof(neigh_entry));
    neigh_entry.ifindex = ifindex;
    memcpy(&(neigh_entry.ip_addr), ip_addr_p, sizeof(L_INET_AddrIp_T));
    neigh_entry.phy_address_len = phy_len;
    memcpy (neigh_entry.phy_address, phy_addr_p, sizeof(neigh_entry.phy_address));
    if (neigh_type == IPAL_NEIGHBOR_TYPE_STATIC)
        neigh_entry.state = NUD_PERMANENT;
    else if (neigh_type == IPAL_NEIGHBOR_TYPE_DYNAMIC)
        neigh_entry.state = NUD_REACHABLE;
    else if (neigh_type == IPAL_NEIGHBOR_TYPE_INVALID)
       neigh_entry.state = NUD_NONE;

    return IPAL_Rt_Netlink_AddNeighbor(&neigh_entry, replace_if_exist);
}

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
UI32_T IPAL_NEIGH_DeleteNeighbor(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p)
{
    IPAL_NeighborEntry_T neigh_entry;

    if(ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER || ifindex > SYS_ADPT_MAX_VLAN_ID + SYS_ADPT_VLAN_1_IF_INDEX_NUMBER -1)
    {
#if (SYS_CPNT_CRAFT_PORT == TRUE)
        if(ifindex != SYS_ADPT_CRAFT_INTERFACE_IFINDEX)
#endif
        return IPAL_RESULT_FAIL;
    }

    memset (&neigh_entry, 0, sizeof(neigh_entry));
    neigh_entry.ifindex = ifindex;
    memcpy(&(neigh_entry.ip_addr), ip_addr_p, sizeof(L_INET_AddrIp_T));

    return IPAL_Rt_Netlink_DeleteNeighbor(&neigh_entry);
}

#if (SYS_CPNT_PROXY_ARP == TRUE)
/* FUNCTION NAME : IPAL_NEIGH_EnableIpv4ProxyArp
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
UI32_T IPAL_NEIGH_EnableIpv4ProxyArp(UI32_T ifindex)
{
    IPAL_DEBUG_PRINT("Enable Interface:%lu Proxy Arp.", (unsigned long)ifindex);

    if(ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER || ifindex > SYS_ADPT_MAX_VLAN_ID + SYS_ADPT_VLAN_1_IF_INDEX_NUMBER -1)
        return IPAL_RESULT_FAIL;

    return IPAL_Sysctl_EnableProxyArp(ifindex);
}

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
UI32_T IPAL_NEIGH_DisableIpv4ProxyArp(UI32_T ifindex)
{
    IPAL_DEBUG_PRINT("Disable Interface:%lu Proxy Arp.", (unsigned long)ifindex);

    if(ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER || ifindex > SYS_ADPT_MAX_VLAN_ID + SYS_ADPT_VLAN_1_IF_INDEX_NUMBER -1)
        return IPAL_RESULT_FAIL;

    return IPAL_Sysctl_DisableProxyArp(ifindex);
}
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
UI32_T IPAL_NEIGH_GetIpv4ProxyArp(UI32_T ifindex, BOOL_T *status_p)
{
    int arp_proxy = 0;
    UI32_T ret;

    if(ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER || ifindex > SYS_ADPT_MAX_VLAN_ID + SYS_ADPT_VLAN_1_IF_INDEX_NUMBER -1)
        return IPAL_RESULT_FAIL;
    if(status_p == NULL)
        return IPAL_RESULT_FAIL;

    ret = IPAL_Sysctl_GetProxyArp(ifindex, &arp_proxy);
    *status_p = !!arp_proxy;
    return ret;
}

#if (SYS_CPNT_IPV6 == TRUE)
/* FUNCTION NAME : IPAL_NB_EnableIpv6ProxyNdp
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
UI32_T IPAL_NEIGH_EnableIpv6ProxyNdp(UI32_T ifindex)
{
    return IPAL_Sysctl_SetIpv6ProxyNdp(ifindex, 1);
}


/* FUNCTION NAME : IPAL_NEIGH_DisableIpv6ProxyNdp
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
UI32_T IPAL_NEIGH_DisableIpv6ProxyNdp(UI32_T ifindex)
{
    return IPAL_Sysctl_SetIpv6ProxyNdp(ifindex, 0);
}


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
UI32_T IPAL_NEIGH_GetIpv6ProxyNdp(UI32_T ifindex, BOOL_T *status_p)
{
    UI32_T proxy_ndp = 0;
    UI32_T ret;

    if(ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER || ifindex > SYS_ADPT_MAX_VLAN_ID + SYS_ADPT_VLAN_1_IF_INDEX_NUMBER -1)
        return IPAL_RESULT_FAIL;
    if(status_p == NULL)
        return IPAL_RESULT_FAIL;

    ret = IPAL_Sysctl_GetIpv6ProxyNdp(ifindex, &proxy_ndp);
    *status_p = !!proxy_ndp;
    return ret;
}
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

static UI32_T IPAL_NEIGH_FetchAllIpv4Neighbor(L_SORT_LST_List_T *neigh_list_p)
{
    if(neigh_list_p == NULL)
        return IPAL_RESULT_FAIL;

    return IPAL_Rt_Netlink_FetchAllIpv4Neighbor(neigh_list_p);
}

#if (SYS_CPNT_IPV6 == TRUE)
static UI32_T IPAL_NEIGH_FetchAllIpv6Neighbor(L_SORT_LST_List_T *neigh_list_p)
{
    if(neigh_list_p == NULL)
        return IPAL_RESULT_FAIL;

    return IPAL_Rt_Netlink_FetchAllIpv6Neighbor(neigh_list_p);
}
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
UI32_T IPAL_NEIGH_GetNeighbor(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p, IPAL_NeighborEntry_T *neighbor_p)
{
#if 0
    UI32_T ret = IPAL_RESULT_FAIL;

    if (ip_addr_p->type == L_INET_ADDR_TYPE_IPV4 ||
        ip_addr_p->type == L_INET_ADDR_TYPE_IPV4Z)
    {
        L_SORT_LST_List_T           neigh_list;
        IPAL_NeighborEntryIpv4_T    neigh_entry;

        L_SORT_LST_Create(&neigh_list, IPAL_MAX_NUM_OF_NEIGH_ENTRY, sizeof(IPAL_NeighborEntryIpv4_T), IPAL_NEIGH_Ipv4SortCompareFunc);

        IPAL_NEIGH_FetchAllIpv4Neighbor(&neigh_list);

        neigh_entry.ifindex = ifindex;
        memcpy(neigh_entry.ip_addr, ip_addr_p->addr, SYS_ADPT_IPV4_ADDR_LEN);
        if (L_SORT_LST_Get(&neigh_list, &neigh_entry) == TRUE)
        {
            memset(neighbor_p, 0, sizeof(IPAL_NeighborEntry_T));
            neighbor_p->ifindex             = neigh_entry.ifindex;
            neighbor_p->phy_address_len     = neigh_entry.phy_address_len;
            neighbor_p->state               = neigh_entry.state;
            memcpy(neighbor_p->phy_address, neigh_entry.phy_address, neigh_entry.phy_address_len);
            memcpy(neighbor_p->ip_addr.addr, neigh_entry.ip_addr, SYS_ADPT_IPV4_ADDR_LEN);
            if (L_INET_ADDR_IS_IPV4_LINK_LOCAL(neigh_entry.ip_addr))
                neighbor_p->ip_addr.type    = L_INET_ADDR_TYPE_IPV4Z;
            else
                neighbor_p->ip_addr.type    = L_INET_ADDR_TYPE_IPV4;
            neighbor_p->ip_addr.addrlen     = SYS_ADPT_IPV4_ADDR_LEN;

            ret = IPAL_RESULT_OK;
        }

        L_SORT_LST_Delete_All(&neigh_list);
    }
#if (SYS_CPNT_IPV6 == TRUE)
    else if (ip_addr_p->type == L_INET_ADDR_TYPE_IPV6 ||
             ip_addr_p->type == L_INET_ADDR_TYPE_IPV6Z)
    {
        L_SORT_LST_List_T           neigh_list;
        IPAL_NeighborEntryIpv6_T    neigh_entry;

        L_SORT_LST_Create(&neigh_list, IPAL_MAX_NUM_OF_NEIGH_ENTRY, sizeof(IPAL_NeighborEntryIpv6_T), IPAL_NEIGH_Ipv6SortCompareFunc);

        IPAL_NEIGH_FetchAllIpv6Neighbor(&neigh_list);

        neigh_entry.ifindex = ifindex;
        memcpy(neigh_entry.ip_addr, ip_addr_p->addr, SYS_ADPT_IPV6_ADDR_LEN);

        if (L_SORT_LST_Get(&neigh_list, &neigh_entry) == TRUE)
        {
            memset(neighbor_p, 0, sizeof(IPAL_NeighborEntry_T));
            neighbor_p->ifindex             = neigh_entry.ifindex;
            neighbor_p->phy_address_len     = neigh_entry.phy_address_len;
            neighbor_p->state               = neigh_entry.state;
            memcpy(neighbor_p->phy_address, neigh_entry.phy_address, neigh_entry.phy_address_len);
            memcpy(neighbor_p->ip_addr.addr, neigh_entry.ip_addr, SYS_ADPT_IPV6_ADDR_LEN);
            if (L_INET_ADDR_IS_IPV6_LINK_LOCAL(neigh_entry.ip_addr))
                neighbor_p->ip_addr.type    = L_INET_ADDR_TYPE_IPV6Z;
            else
                neighbor_p->ip_addr.type    = L_INET_ADDR_TYPE_IPV6;
            neighbor_p->ip_addr.addrlen     = SYS_ADPT_IPV6_ADDR_LEN;

            ret = IPAL_RESULT_OK;
        }

        L_SORT_LST_Delete_All(&neigh_list);
    }
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

    return ret;
#endif

    UI32_T ret = IPAL_RESULT_FAIL;
    UI8_T ha[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T state;

    if (K_AMTRL3_MGR_RESULT_OK == SYSFUN_Syscall(SYSFUN_SYSCALL_AMTRL3_MGR,
                                                 (void *)K_AMTRL3_MGR_SYSCALL_CMD_GET_NEIGHBOR,
                                                 (void *)(uintptr_t)ifindex,
                                                 (void *)ip_addr_p,
                                                 (void *)ha,
                                                 (void *)&state))
    {
            memset(neighbor_p, 0, sizeof(IPAL_NeighborEntry_T));
            neighbor_p->ifindex             = ifindex;
            neighbor_p->phy_address_len     = SYS_ADPT_MAC_ADDR_LEN;
            neighbor_p->state               = state;
            memcpy(neighbor_p->phy_address, ha, SYS_ADPT_MAC_ADDR_LEN);
            memcpy(&neighbor_p->ip_addr, ip_addr_p, sizeof(L_INET_AddrIp_T));

            ret = IPAL_RESULT_OK;        
    }

    return ret;
}


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
UI32_T IPAL_NEIGH_GetNextNNeighbor(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p, UI32_T *num_p, IPAL_NeighborEntry_T *neighbor_p)
{
#if 0
    BOOL_T hit = FALSE;
    IPAL_IPv4_ARP_LIST_T *arp_list, *arp_list_f;

    arp_list = (IPAL_IPv4_ARP_LIST_T *) malloc(sizeof(IPAL_IPv4_ARP_LIST_T));
    memset(arp_list, 0, sizeof(IPAL_IPv4_ARP_LIST_T));

    IPAL_FetchAllIPv4Arp (arp_list);

    while (arp_list && arp_list->arp_entry )
    {
        if (!hit)
        {
            if ( arp_list->arp_entry->ifindex == ifindex )
            {
                if (arp_list->arp_entry->ip_addr > ip_addr)
                {
                    *arp = *(arp_list->arp_entry);
                    hit = TRUE;
                }
            }
            else if (arp_list->arp_entry->ifindex > ifindex)
            {
                *arp = *(arp_list->arp_entry);
                hit = TRUE;
            }
        }
        free (arp_list->arp_entry);
        arp_list_f = arp_list;
        arp_list = arp_list->arp_list_next;
        free (arp_list_f);
    }

    if(hit)
        return RESULT_OK;
    else
        return RESULT_FAIL;
#endif

    UI32_T ret = IPAL_RESULT_FAIL;
    UI32_T copying_num = 0;
    IPAL_NeighborEntry_T *copying_neighbor_p;

    if (ip_addr_p->type == L_INET_ADDR_TYPE_IPV4 ||
        ip_addr_p->type == L_INET_ADDR_TYPE_IPV4Z)
    {
        L_SORT_LST_List_T           neigh_list;
        IPAL_NeighborEntryIpv4_T    neigh_entry;

        L_SORT_LST_Create(&neigh_list, IPAL_MAX_NUM_OF_NEIGH_ENTRY, sizeof(IPAL_NeighborEntryIpv4_T), IPAL_NEIGH_Ipv4SortCompareFunc);

        IPAL_NEIGH_FetchAllIpv4Neighbor(&neigh_list);

        neigh_entry.ifindex = ifindex;
        memcpy(neigh_entry.ip_addr, ip_addr_p->addr, SYS_ADPT_IPV4_ADDR_LEN);

        while (copying_num < *num_p &&
               L_SORT_LST_Get_Next(&neigh_list, &neigh_entry) == TRUE)
        {
            copying_neighbor_p = &(*(neighbor_p+copying_num));
            memset(copying_neighbor_p, 0, sizeof(IPAL_NeighborEntry_T));
            copying_neighbor_p->ifindex          = neigh_entry.ifindex;
            copying_neighbor_p->phy_address_len  = neigh_entry.phy_address_len;
            copying_neighbor_p->state            = neigh_entry.state;
            copying_neighbor_p->last_update      = neigh_entry.last_update;
            memcpy(copying_neighbor_p->phy_address, neigh_entry.phy_address, neigh_entry.phy_address_len);
            memcpy(copying_neighbor_p->ip_addr.addr, neigh_entry.ip_addr, SYS_ADPT_IPV4_ADDR_LEN);
            if (L_INET_ADDR_IS_IPV4_LINK_LOCAL(neigh_entry.ip_addr))
                copying_neighbor_p->ip_addr.type = L_INET_ADDR_TYPE_IPV4Z;
            else
                copying_neighbor_p->ip_addr.type = L_INET_ADDR_TYPE_IPV4;
            copying_neighbor_p->ip_addr.addrlen  = SYS_ADPT_IPV4_ADDR_LEN;

            copying_num++;
            ret = IPAL_RESULT_OK;
        }

        *num_p = copying_num;
        L_SORT_LST_Delete_All(&neigh_list);
    }
#if (SYS_CPNT_IPV6 == TRUE)
    else if (ip_addr_p->type == L_INET_ADDR_TYPE_IPV6 ||
             ip_addr_p->type == L_INET_ADDR_TYPE_IPV6Z)
    {

        L_SORT_LST_List_T           neigh_list;
        IPAL_NeighborEntryIpv6_T    neigh_entry;

        L_SORT_LST_Create(&neigh_list, IPAL_MAX_NUM_OF_NEIGH_ENTRY, sizeof(IPAL_NeighborEntryIpv6_T), IPAL_NEIGH_Ipv6SortCompareFunc);

        IPAL_NEIGH_FetchAllIpv6Neighbor(&neigh_list);
        neigh_entry.ifindex = ifindex;
        memcpy(neigh_entry.ip_addr, ip_addr_p->addr, SYS_ADPT_IPV6_ADDR_LEN);

        while (copying_num < *num_p &&
               L_SORT_LST_Get_Next(&neigh_list, &neigh_entry) == TRUE)
        {
            copying_neighbor_p = &(*(neighbor_p+copying_num));
            memset(copying_neighbor_p, 0, sizeof(IPAL_NeighborEntry_T));
            copying_neighbor_p->ifindex          = neigh_entry.ifindex;
            copying_neighbor_p->phy_address_len  = neigh_entry.phy_address_len;
            copying_neighbor_p->state            = neigh_entry.state;
            copying_neighbor_p->last_update      = neigh_entry.last_update;
            memcpy(copying_neighbor_p->phy_address, neigh_entry.phy_address, neigh_entry.phy_address_len);
            memcpy(copying_neighbor_p->ip_addr.addr, neigh_entry.ip_addr, SYS_ADPT_IPV6_ADDR_LEN);
            if (L_INET_ADDR_IS_IPV6_LINK_LOCAL(neigh_entry.ip_addr))
                copying_neighbor_p->ip_addr.type = L_INET_ADDR_TYPE_IPV6Z;
            else
                copying_neighbor_p->ip_addr.type = L_INET_ADDR_TYPE_IPV6;
            copying_neighbor_p->ip_addr.addrlen  = SYS_ADPT_IPV6_ADDR_LEN;

            copying_num++;
            ret = IPAL_RESULT_OK;
        }

        *num_p = copying_num;
        L_SORT_LST_Delete_All(&neigh_list);
    }
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

    return ret;
}


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
UI32_T IPAL_NEIGH_GetNeighborTableStatistic(IPAL_NeighborStatisticType_T stat_type, UI32_T *value_p)
{
    *value_p = 0;
    return IPAL_RESULT_OK;
}


/* FUNCTION NAME : IPAL_NEIGH_FlushAllDynamicNeighbor
 * PURPOSE:
 *     Flush all dynamic neighbor entries in neighbor table
 * INPUT:
 *     addr_type  -- The address family to clear
 *                   IPAL_NEIGHBOR_ADDR_TYPE_IPV4 -- IPV4
 *                   IPAL_NEIGHBOR_ADDR_TYPE_IPV6 -- IPV6
 * OUTPUT:
 *     None
 * RETURN:
 *     IPAL_RESULT_OK  -- success
 *     IPAL_RESULT_FAIL  -- Flush all dynamic neighbor entries fail
 * NOTES:
 */
UI32_T IPAL_NEIGH_FlushAllDynamicNeighbor(IPAL_NeighborAddressType_T addr_type)
{

    L_SORT_LST_List_T           neigh_list;
    IPAL_NeighborEntryIpv4_T    neigh_entry;
#if (SYS_CPNT_IPV6 == TRUE)
    L_SORT_LST_List_T           neigh_list_v6;
    IPAL_NeighborEntryIpv6_T    neigh_entry_v6;
#endif
    L_INET_AddrIp_T             tmp_ipaddr;

    if (addr_type == IPAL_NEIGHBOR_ADDR_TYPE_IPV4)
    {
        /* delete all dynamic IPv4 Neighbor entries
         */
        memset(&tmp_ipaddr, 0x0, sizeof(L_INET_AddrIp_T));
        tmp_ipaddr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
        tmp_ipaddr.type    = L_INET_ADDR_TYPE_IPV4;

        L_SORT_LST_Create(&neigh_list, IPAL_MAX_NUM_OF_NEIGH_ENTRY, sizeof(IPAL_NeighborEntryIpv4_T), IPAL_NEIGH_Ipv4SortCompareFunc);
        IPAL_NEIGH_FetchAllIpv4Neighbor(&neigh_list);

        memset(&neigh_entry, 0x0, sizeof(IPAL_NeighborEntryIpv4_T));
        while (L_SORT_LST_Get_Next(&neigh_list, &neigh_entry)==TRUE)
        {
            if (!((neigh_entry.state) & (NUD_PERMANENT|NUD_NOARP)) )
            {
                memcpy(tmp_ipaddr.addr, neigh_entry.ip_addr, SYS_ADPT_IPV4_ADDR_LEN);
                IPAL_NEIGH_DeleteNeighbor(neigh_entry.ifindex, &tmp_ipaddr);
            }
        }
        L_SORT_LST_Delete_All(&neigh_list);
    }
#if (SYS_CPNT_IPV6 == TRUE)
    else if (addr_type == IPAL_NEIGHBOR_ADDR_TYPE_IPV6)
    {
        /* delete all dynamic IPv6 Neighbor entries
        */
        tmp_ipaddr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
        tmp_ipaddr.type    = L_INET_ADDR_TYPE_IPV6;

        L_SORT_LST_Create(&neigh_list_v6, IPAL_MAX_NUM_OF_NEIGH_ENTRY, sizeof(IPAL_NeighborEntryIpv6_T), IPAL_NEIGH_Ipv6SortCompareFunc);
        IPAL_NEIGH_FetchAllIpv6Neighbor(&neigh_list_v6);

        memset(&neigh_entry_v6, 0x0, sizeof(IPAL_NeighborEntryIpv6_T));
        while (L_SORT_LST_Get_Next(&neigh_list_v6, &neigh_entry_v6)==TRUE)
        {
          if (!((neigh_entry_v6.state) & (NUD_PERMANENT|NUD_NOARP)) )
          {
              memcpy(tmp_ipaddr.addr, neigh_entry_v6.ip_addr, SYS_ADPT_IPV6_ADDR_LEN);
              IPAL_NEIGH_DeleteNeighbor(neigh_entry_v6.ifindex, &tmp_ipaddr);
          }
        }
        L_SORT_LST_Delete_All(&neigh_list_v6);
    }

#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

#if 0
    IPAL_IPv4_ARP_LIST_T *arp_list, *arp_list_f;

    arp_list = (IPAL_IPv4_ARP_LIST_T *) malloc(sizeof(IPAL_IPv4_ARP_LIST_T));
    memset(arp_list, 0, sizeof(IPAL_IPv4_ARP_LIST_T));

    IPAL_FetchAllIPv4Arp (arp_list);

    while (arp_list && arp_list->arp_entry )
    {
        if (!((arp_list->arp_entry ->state) & (NUD_PERMANENT|NUD_NOARP)) )
        {
            IPAL_DeleteIPv4Arp (arp_list->arp_entry->ifindex, arp_list->arp_entry->ip_addr);
            free (arp_list->arp_entry);
        }

        arp_list_f = arp_list;
        arp_list = arp_list->arp_list_next;
        free (arp_list_f);
    }
#endif

    return IPAL_RESULT_OK;
}


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
UI32_T IPAL_NEIGH_FlushAllNeighbor()
{
    L_SORT_LST_List_T           neigh_list;
    IPAL_NeighborEntryIpv4_T    neigh_entry;
#if (SYS_CPNT_IPV6 == TRUE)
    L_SORT_LST_List_T           neigh_list_v6;
    IPAL_NeighborEntryIpv6_T    neigh_entry_v6;
#endif
    L_INET_AddrIp_T tmp_ipaddr;

    /* delete all IPv4 Neighbor entries
     */
    memset(&tmp_ipaddr, 0x0, sizeof(L_INET_AddrIp_T));
    tmp_ipaddr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    tmp_ipaddr.type    = L_INET_ADDR_TYPE_IPV4;

    L_SORT_LST_Create(&neigh_list, IPAL_MAX_NUM_OF_NEIGH_ENTRY, sizeof(IPAL_NeighborEntryIpv4_T), IPAL_NEIGH_Ipv4SortCompareFunc);
    IPAL_NEIGH_FetchAllIpv4Neighbor(&neigh_list);

    memset(&neigh_entry, 0x0, sizeof(IPAL_NeighborEntryIpv4_T));
    while (L_SORT_LST_Get_Next(&neigh_list, &neigh_entry)==TRUE)
    {
        memcpy(tmp_ipaddr.addr, neigh_entry.ip_addr, SYS_ADPT_IPV4_ADDR_LEN);
        IPAL_NEIGH_DeleteNeighbor(neigh_entry.ifindex, &tmp_ipaddr);
    }
    L_SORT_LST_Delete_All(&neigh_list);

#if (SYS_CPNT_IPV6 == TRUE)
    /* delete all IPv6 Neighbor entries
     */
    tmp_ipaddr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
    tmp_ipaddr.type    = L_INET_ADDR_TYPE_IPV6;

    L_SORT_LST_Create(&neigh_list_v6, IPAL_MAX_NUM_OF_NEIGH_ENTRY, sizeof(IPAL_NeighborEntryIpv6_T), IPAL_NEIGH_Ipv6SortCompareFunc);
    IPAL_NEIGH_FetchAllIpv6Neighbor(&neigh_list_v6);

    memset(&neigh_entry_v6, 0x0, sizeof(IPAL_NeighborEntryIpv6_T));
    while (L_SORT_LST_Get_Next(&neigh_list_v6, &neigh_entry_v6)==TRUE)
    {
        memcpy(tmp_ipaddr.addr, neigh_entry_v6.ip_addr, SYS_ADPT_IPV6_ADDR_LEN);
        IPAL_NEIGH_DeleteNeighbor(neigh_entry_v6.ifindex, &tmp_ipaddr);
    }
    L_SORT_LST_Delete_All(&neigh_list_v6);
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

    return IPAL_RESULT_OK;
}


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
UI32_T IPAL_NEIGH_SetNeighborAgingTime(UI32_T aging_time)
{
    IPAL_DEBUG_PRINT("Set IPv4 Arp Aging Timeout: %lu", (unsigned long)aging_time);

    return IPAL_Sysctl_SetIpv4ArpAgingTime(aging_time);
}

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
UI32_T IPAL_NEIGH_GetNeighborAgingTime(UI32_T *aging_time_p)
{
    return IPAL_Sysctl_GetIpv4ArpAgingTime(aging_time_p);
}

/*
 * Get ARP statistics
 */
UI32_T IPAL_NEIGH_GetIpv4ArpStatistics(IPAL_Ipv4ArpStatistics_T *arp_stat_p)
{
    FILE *in = NULL;
    char line[1024];

    if (NULL == arp_stat_p)
        return IPAL_RESULT_FAIL;

    in = fopen("/proc/net/arpstat", "r");
    if (in == NULL)
        return IPAL_RESULT_FAIL;

    while (fgets(line, sizeof(line), in))
    {
        if (strncmp(line, ARP_STATS_LINE, ARP_STATS_PREFIX_LEN) != 0)
        {
            unsigned long i1, i2, i3, i4;
            sscanf(line, "%lu %lu %lu %lu", &i1, &i2, &i3, &i4);
            arp_stat_p->in_request = i1;
            arp_stat_p->in_reply = i2;
            arp_stat_p->out_request = i3;
            arp_stat_p->out_reply = i4;
        }
    }

    fclose(in);

    return IPAL_RESULT_OK;
}


UI32_T IPAL_NEIGH_SendArpRequest(UI32_T ifindex, L_INET_AddrIp_T *dst_addr_p)
{
    UI32_T fd, ret;
    int status;
    L_INET_AddrIp_T ipaddr;
    UI8_T phy_addr[SYS_ADPT_MAC_ADDR_LEN] = {0};
    IPAL_NeighborType_T neighbor_type = IPAL_NEIGHBOR_TYPE_INVALID;

    struct {
        struct arphdr   arph;
        UI8_T           buf[2*(ETH_ALEN + 4)];
    } arp;
    IPAL_IfInfo_T  if_info;
    struct sockaddr_ll saddr;
    struct iovec iov = { (void*) &arp, sizeof( arp) };
    struct msghdr msg = {(void*) &saddr, sizeof saddr, &iov, 1, NULL, 0, 0};

    memset(&arp, 0, sizeof(arp));
    memset(&if_info, 0, sizeof(if_info));
    memset(&saddr, 0, sizeof(saddr));

    if(ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER || ifindex > SYS_ADPT_MAX_VLAN_ID + SYS_ADPT_VLAN_1_IF_INDEX_NUMBER -1)
        return IPAL_RESULT_FAIL;

    /* if neighbor entry not exist, we must create a new one with state "failed"
     * (let kernel update its state later) otherwise kernel will drop the arp-reply packet
     */
    ret = IPAL_NEIGH_AddNeighbor(ifindex, dst_addr_p, SYS_ADPT_MAC_ADDR_LEN, phy_addr, neighbor_type, FALSE);
    if(IPAL_RESULT_OK != ret && IPAL_RESULT_ENTRY_EXIST != ret)
    {
        IPAL_DEBUG_PRINT("failed to add neighbor to kernel");
        return IPAL_RESULT_FAIL;
    }

    saddr.sll_family = AF_PACKET;
    saddr.sll_ifindex = ifindex;
    saddr.sll_protocol = ntohs(ETH_P_ARP);
    saddr.sll_halen = ETH_ALEN;
    memset(saddr.sll_addr, 0xff,  ETH_ALEN);

    arp.arph.ar_hrd = htons(ARPHRD_ETHER);
    arp.arph.ar_pro = htons(ETH_P_IP);
    arp.arph.ar_hln = ETH_ALEN;
    arp.arph.ar_pln = 4;
    arp.arph.ar_op = htons(ARPOP_REQUEST);

    if(IPAL_IF_GetIpv4IfInfo(ifindex, &if_info) == IPAL_RESULT_FAIL)
    {
        return IPAL_RESULT_FAIL;
    }

    ipaddr.type = L_INET_ADDR_TYPE_IPV4;
    if(IPAL_IF_GetIfIpv4Addr(ifindex, &ipaddr) == IPAL_RESULT_FAIL)
    {
        return IPAL_RESULT_FAIL;
    }

    memcpy(arp.buf, if_info.hw_addr, ETH_ALEN);
    memcpy(arp.buf+ETH_ALEN, (char*)ipaddr.addr, 4);
    memcpy(arp.buf+ETH_ALEN+4+ETH_ALEN, (char*)dst_addr_p->addr, 4);

    fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ARP));
    if (fd < 0)
    {
        IPAL_DEBUG_PRINT ("Can't open socket: %s", strerror (errno));
        return IPAL_RESULT_FAIL;
    }

    status = sendmsg (fd, &msg, 0);
    if (status < 0)
    {
        IPAL_DEBUG_PRINT ("IPAL_SendArpReq error: %s", strerror (errno));
        close(fd);
        return IPAL_RESULT_FAIL;
    }
    close(fd);

    return IPAL_RESULT_OK;
}

#if (SYS_CPNT_IPV6 == TRUE)
UI32_T IPAL_NEIGH_SendNeighSolicitation(UI32_T ifindex, L_INET_AddrIp_T *dst_addr_p)
{
    #define ND_NEIGHBOR_SOLICIT         135
    #define ND_OPT_SOURCE_LINKADDR      1

    UI32_T fd;
    int status;
    IPAL_IfInfo_T  if_info;
    IPAL_Icmp6NeighSolHdr_T nd_sol_data;
    struct  sockaddr_in6 dst_addr;
    L_INET_AddrIp_T ipaddr;
    UI8_T phy_addr[SYS_ADPT_MAC_ADDR_LEN] = {0};
    IPAL_NeighborType_T neighbor_type = IPAL_NEIGHBOR_TYPE_INVALID;
    UI32_T ret;

    if(ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER || ifindex > SYS_ADPT_MAX_VLAN_ID + SYS_ADPT_VLAN_1_IF_INDEX_NUMBER -1)
        return IPAL_RESULT_FAIL;

    /* if neighbor entry not exist, we must create a new one with state "failed"
     * (let kernel update its state later) otherwise kernel will drop the NA packet
     */
    ret = IPAL_NEIGH_AddNeighbor(ifindex, dst_addr_p, SYS_ADPT_MAC_ADDR_LEN, phy_addr, neighbor_type, FALSE);
    if(IPAL_RESULT_OK != ret && IPAL_RESULT_ENTRY_EXIST != ret)
    {
        IPAL_DEBUG_PRINT("failed to add neighbor to kernel");
        return IPAL_RESULT_FAIL;
    }

    fd = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    if (fd < 0)
    {
        IPAL_DEBUG_PRINT ("Can't open socket: %s", strerror (errno));
        return IPAL_RESULT_FAIL;
    }

    memset(&if_info, 0, sizeof(if_info));
    if(IPAL_IF_GetIpv6IfInfo(ifindex, &if_info) == IPAL_RESULT_FAIL)
    {
        close(fd);
        return IPAL_RESULT_FAIL;
    }

    if(IPAL_IF_GetIfIpv6Addr(ifindex, &ipaddr) == IPAL_RESULT_FAIL)
    {
        close(fd);
        return IPAL_RESULT_FAIL;
    }

    memset(&nd_sol_data, 0, sizeof(nd_sol_data));
    nd_sol_data.type        = ND_NEIGHBOR_SOLICIT;
    nd_sol_data.code        = 0;
    nd_sol_data.reserved    = 0;
    nd_sol_data.checksum    = 0;
    nd_sol_data.opt_type    = ND_OPT_SOURCE_LINKADDR;
    nd_sol_data.opt_len     = 1; /* 1 unit = 8 bytes */
    memcpy(nd_sol_data.target, dst_addr_p->addr, SYS_ADPT_IPV6_ADDR_LEN);
    memcpy(nd_sol_data.opt_data_sll, if_info.hw_addr, SYS_ADPT_MAC_ADDR_LEN);

    memset(&dst_addr, 0x0, sizeof(struct  sockaddr_in6));
    dst_addr.sin6_family = AF_INET6;
    memcpy(&(dst_addr.sin6_addr.s6_addr),dst_addr_p->addr,SYS_ADPT_IPV6_ADDR_LEN);

    /* kernel's scope_id uses ifindex */
    VLAN_OM_ConvertToIfindex(dst_addr_p->zoneid, (UI32_T *) &(dst_addr.sin6_scope_id));
    status = sendto (fd, (UI8_T *)(&nd_sol_data), sizeof(IPAL_Icmp6NeighSolHdr_T), 0,
                     (struct sockaddr*)&dst_addr, sizeof(struct  sockaddr_in6));

    if (status < 0)
    {
        IPAL_DEBUG_PRINT ("IPAL_NEIGH_SendNeighSolicitation error: %s",
                            strerror (errno));
        close(fd);
        return IPAL_RESULT_FAIL;
    }
    close(fd);

    return IPAL_RESULT_OK;
}
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

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
UI32_T IPAL_NEIGH_SendNeighborRequest(UI32_T ifindex, L_INET_AddrIp_T *dst_addr_p)
{
    switch (dst_addr_p->type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
            return IPAL_NEIGH_SendArpRequest(ifindex, dst_addr_p);

#if (SYS_CPNT_IPV6 == TRUE)
        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
            return IPAL_NEIGH_SendNeighSolicitation(ifindex, dst_addr_p);
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

        default:
            return IPAL_RESULT_FAIL;
    }
}

/*
 * show arp table entry , include dynamic and static
 */
UI32_T IPAL_NEIGH_ShowIpv4NeighTable()
{
    L_SORT_LST_List_T           neigh_list;
    IPAL_NeighborEntryIpv4_T    neigh_entry;
    char        cp[L_INET_MAX_IP4ADDR_STR_LEN+1]= {0};
    UI8_T        mac[18]= {0};

    L_SORT_LST_Create(&neigh_list, IPAL_MAX_NUM_OF_NEIGH_ENTRY, sizeof(IPAL_NeighborEntryIpv4_T), IPAL_NEIGH_Ipv4SortCompareFunc);
    IPAL_NEIGH_FetchAllIpv4Neighbor(&neigh_list);

    BACKDOOR_MGR_Printf("\r\n%-9s %-15s %-18s %s\r\n","Interface","IPv4Addr","MAC","State");
    BACKDOOR_MGR_Printf(    "%-9s %-15s %-18s %s\r\n","---------","--------","---","-----");

    memset(&neigh_entry, 0x0, sizeof(IPAL_NeighborEntryIpv4_T));
    while (L_SORT_LST_Get_Next(&neigh_list, &neigh_entry)==TRUE)
    {
        L_INET_Ntop(L_INET_AF_INET, neigh_entry.ip_addr, cp, sizeof(cp));
        sprintf((char *)mac, "%02X-%02X-%02X-%02X-%02X-%02X",
                 neigh_entry.phy_address[0], neigh_entry.phy_address[1],
                 neigh_entry.phy_address[2], neigh_entry.phy_address[3],
                 neigh_entry.phy_address[4], neigh_entry.phy_address[5]);
        BACKDOOR_MGR_Printf("%-9lu %-15s %-18s", (unsigned long)neigh_entry.ifindex, cp, mac);

        if (!((neigh_entry.state) & (NUD_PERMANENT|NUD_NOARP)) )
            BACKDOOR_MGR_Printf("%-7s (%lu)\n", "Dynamic", (unsigned long)neigh_entry.state);
        else
            BACKDOOR_MGR_Printf("%-7s (%lu)\n", "Static", (unsigned long)neigh_entry.state);
    }

    L_SORT_LST_Delete_All(&neigh_list);

    return IPAL_RESULT_OK;
}

#if (SYS_CPNT_IPV6 == TRUE)
/*
* show IPv6 neigh table entry , include dynamic and static
*/
UI32_T IPAL_NEIGH_ShowIpv6NeighTable()
{
    L_SORT_LST_List_T           neigh_list;
    IPAL_NeighborEntryIpv6_T    neigh_entry;
    char         cp[L_INET_MAX_IP6ADDR_STR_LEN+1]= {0};
    UI8_T        mac[18]= {0};

    L_SORT_LST_Create(&neigh_list, IPAL_MAX_NUM_OF_NEIGH_ENTRY, sizeof(IPAL_NeighborEntryIpv6_T), IPAL_NEIGH_Ipv6SortCompareFunc);
    IPAL_NEIGH_FetchAllIpv6Neighbor(&neigh_list);

    BACKDOOR_MGR_Printf("\r\n%-9s %-30s %-18s %s\r\n","Interface","IPv6Addr","MAC","State");
    BACKDOOR_MGR_Printf(    "%-9s %-30s %-18s %s\r\n","---------","--------","---","-----");

    memset(&neigh_entry, 0x0, sizeof(IPAL_NeighborEntryIpv6_T));
    while (L_SORT_LST_Get_Next(&neigh_list, &neigh_entry)==TRUE)
    {
        L_INET_Ntop(L_INET_AF_INET6, neigh_entry.ip_addr, cp, sizeof(cp));
        sprintf((char *)mac, "%02X-%02X-%02X-%02X-%02X-%02X",
                 neigh_entry.phy_address[0], neigh_entry.phy_address[1],
                 neigh_entry.phy_address[2], neigh_entry.phy_address[3],
                 neigh_entry.phy_address[4], neigh_entry.phy_address[5]);
        BACKDOOR_MGR_Printf("%-9lu %-30s %-18s", (unsigned long)neigh_entry.ifindex, cp, mac);

        if (neigh_entry.state & NUD_PERMANENT)
            BACKDOOR_MGR_Printf("%-7s (%lu)\n", "Static", (unsigned long)neigh_entry.state);
        else if (neigh_entry.state & NUD_REACHABLE)
            BACKDOOR_MGR_Printf("%-7s (%lu)\n", "Reachable", (unsigned long)neigh_entry.state);
        else if(neigh_entry.state & NUD_STALE)
            BACKDOOR_MGR_Printf("%-7s (%lu)\n", "Stale", (unsigned long)neigh_entry.state);
        else if(neigh_entry.state & NUD_DELAY)
            BACKDOOR_MGR_Printf("%-7s (%lu)\n", "Delay", (unsigned long)neigh_entry.state);
        else if(neigh_entry.state & NUD_PROBE)
            BACKDOOR_MGR_Printf("%-7s (%lu)\n", "Probe", (unsigned long)neigh_entry.state);
        else
            BACKDOOR_MGR_Printf("%-7s (%lu)\n", "Unknown", (unsigned long)neigh_entry.state);
    }

    L_SORT_LST_Delete_All(&neigh_list);

    return IPAL_RESULT_OK;
}
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

/* LOCAL FUNCTIONS BODY
 */
static int IPAL_NEIGH_Ipv4SortCompareFunc(void* list_node_p, void* comp_node_p)
{
    int ret;

    IPAL_NeighborEntryIpv4_T *node_list_p=NULL, *node_comp_p=NULL;

    node_list_p = (IPAL_NeighborEntryIpv4_T *) list_node_p;
    node_comp_p = (IPAL_NeighborEntryIpv4_T *) comp_node_p;

    ret = node_list_p->ifindex - node_comp_p->ifindex;

    if (ret == 0)
        ret = memcmp(node_list_p->ip_addr,node_comp_p->ip_addr,SYS_ADPT_IPV4_ADDR_LEN);

    return ret;
}

#if (SYS_CPNT_IPV6 == TRUE)
static int IPAL_NEIGH_Ipv6SortCompareFunc(void* list_node_p, void* comp_node_p)
{
    int ret;

    IPAL_NeighborEntryIpv6_T *node_list_p=NULL, *node_comp_p=NULL;

    node_list_p = (IPAL_NeighborEntryIpv6_T *) list_node_p;
    node_comp_p = (IPAL_NeighborEntryIpv6_T *) comp_node_p;

    ret = node_list_p->ifindex - node_comp_p->ifindex;

    if (ret == 0)
        ret = memcmp(node_list_p->ip_addr,node_comp_p->ip_addr,SYS_ADPT_IPV6_ADDR_LEN);

    return ret;
}
#endif


