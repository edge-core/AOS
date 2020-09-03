/*
 *   File Name: ipal_route.c
 *   Purpose:   TCP/IP shim layer(ipal) IP management implementation
 *   Note:
 *   Create:    Basen LV     2008.04.06
 *
 *   Histrory:
 *              Modify         Date      Reason
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
#include <unistd.h>
#include <netinet/in.h>

#include "sys_type.h"
#include "sysfun.h"

#include "l_prefix.h"
#include "l_mm.h"

#include "ipal_types.h"
#include "ipal_debug.h"
#include "ipal_sysctl.h"
#include "ipal_if.h"

#include <linux/rtnetlink.h>
#include "ipal_rt_netlink.h"

#include "backdoor_mgr.h"

/*
 * NAMING CONST DECLARATIONS
*/



/*
 * MACRO FUNCTION DECLARATIONS
*/


/*
 * DATA TYPE DECLARATIONS
*/


/*
 * STATIC VARIABLE DECLARATIONS
 */



/*
 * LOCAL SUBPROGRAM DECLARATIONS
*/
static UI32_T IPAL_ROUTE_FetchAllIpv4Route(IPAL_Ipv4UcRouteList_T **route_list_pp);
static UI32_T IPAL_ROUTE_FetchAllIpv6Route(IPAL_Ipv6UcRouteList_T **route_list_pp);
static UI32_T IPAL_ROUTE_FetchAllIpv6RouteCache(IPAL_Ipv6UcRouteList_T **route_list_pp);

/*
 *  Name    IPAL_Enable_Ipv4Forwarding
 *  Purpose Enable IPv4 Forwarding in TCP/IP stack
 *  Input   None
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_EnableIpv4Forwarding(void)
{
    return IPAL_Sysctl_EnableIpv4Forwarding();
}

/*
 *  Name    IPAL_Disable_Ipv4Forwarding
 *  Purpose Disable IPv4 Forwarding in TCP/IP stack
 *  Input   None
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_DisableIpv4Forwarding(void)
{
    return IPAL_Sysctl_DisableIpv4Forwarding();
}

/*
 *  Name    IPAL_Get_Ipv4Forwarding
 *  Purpose Disable IPv4 Forwarding in TCP/IP stack
 *  Input   int *ip_forward
 *  Output  IPv4 forwarding or not
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_GetIpv4Forwarding (I32_T *ip_forward)
{
    if(ip_forward == NULL)
        return IPAL_RESULT_FAIL;
    return IPAL_Sysctl_GetIpv4Forwarding((int*)ip_forward);
}

/*
 *  Name    IPAL_ROUTE_EnableIfIpv4Forwarding
 *  Purpose Enable IPv4 Forwarding on an interface in TCP/IP stack
 *  Input   ifindex  -- interface ifindex
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_EnableIfIpv4Forwarding(UI32_T ifindex)
{
    return IPAL_Sysctl_EnableIfIpv4Forwarding(ifindex);
}

/*
 *  Name    IPAL_ROUTE_DisableIfIpv4Forwarding
 *  Purpose Disable IPv4 Forwarding on an interface in TCP/IP stack
 *  Input   ifindex  -- interface ifindex
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_DisableIfIpv4Forwarding(UI32_T ifindex)
{
    return IPAL_Sysctl_DisableIfIpv4Forwarding(ifindex);
}

/*
 *  Name    IPAL_ROUTE_GetIfIpv4Forwarding
 *  Purpose Get IPv4 Forwarding status on an interface in TCP/IP stack
 *  Input   ifindex      -- interface ifindex
 *  Output  *ip_forward  -- IPv4 forwarding or not
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_GetIfIpv4Forwarding (UI32_T ifindex, I32_T *ip_forward)
{
    if(ip_forward == NULL)
        return IPAL_RESULT_FAIL;

    return IPAL_Sysctl_GetIfIpv4Forwarding(ifindex, (int*)ip_forward);
}

#if (SYS_CPNT_IPV6 == TRUE)
/*
 *  Name    IPAL_Enable_Ipv6Forwarding
 *  Purpose Enable IPv4 Forwarding in TCP/IP stack
 *  Input   None
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_EnableIpv6Forwarding(void)
{
    return IPAL_Sysctl_EnableIpv6Forwarding();
}

/*
 *  Name    IPAL_Disable_Ipv6Forwarding
 *  Purpose Disable IPv4 Forwarding in TCP/IP stack
 *  Input   None
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_DisableIpv6Forwarding(void)
{
    return IPAL_Sysctl_DisableIpv6Forwarding();
}

/*
 *  Name    IPAL_Get_Ipv6Forwarding
 *  Purpose Disable IPv4 Forwarding in TCP/IP stack
 *  Input   int *ip_forward
 *  Output  IPv4 forwarding or not
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_GetIpv6Forwarding (I32_T *ip_forward)
{
    if(ip_forward == NULL)
        return IPAL_RESULT_FAIL;
    return IPAL_Sysctl_GetIpv6Forwarding((int*)ip_forward);
}

UI32_T IPAL_ROUTE_GetNextNPmtuEntry(IPAL_PmtuEntry_T *pmtu_entry_p, UI32_T *entry_num_p)
{
    IPAL_Ipv6UcRouteList_T *route_list_p = NULL, *pre_route_list_p;
    UI32_T real_entry_num;
    UI32_T found = FALSE;
    UI32_T mtu_expires = 0;
    static int user_hz = 0;
    static UI8_T all_zero[SYS_ADPT_IPV6_ADDR_LEN] = {0};

    if (!user_hz)
        user_hz = sysconf(_SC_CLK_TCK);

    if (IPAL_RESULT_OK != IPAL_Sysctl_GetIpv6MtuExpires(&mtu_expires))
        return IPAL_RESULT_FAIL;

    IPAL_ROUTE_FetchAllIpv6RouteCache(&route_list_p);

    /* if get first
     */
    if (0 == memcmp(pmtu_entry_p->destip.addr, all_zero, SYS_ADPT_IPV6_ADDR_LEN))
    {
        found = TRUE;
    }

    real_entry_num = 0;
    while (route_list_p)
    {
        if (!found)
        {
            if (memcmp(pmtu_entry_p->destip.addr, route_list_p->route_entry.dst_p.u.prefix6.s6_addr, SYS_ADPT_IPV6_ADDR_LEN)==0)
                found = TRUE;
        }
        else
        {
            if (route_list_p->route_entry.nh.u.ipv6_nh.expires >0 &&
                *entry_num_p > real_entry_num)
            {
                (*(pmtu_entry_p+real_entry_num)).pmtu = route_list_p->route_entry.nh.u.ipv6_nh.mtu;
                (*(pmtu_entry_p+real_entry_num)).since_time = mtu_expires - (route_list_p->route_entry.nh.u.ipv6_nh.expires/user_hz);
                memset(&((*(pmtu_entry_p+real_entry_num)).destip), 0x0, sizeof(L_INET_AddrIp_T));
                memcpy((*(pmtu_entry_p+real_entry_num)).destip.addr, route_list_p->route_entry.dst_p.u.prefix6.s6_addr, SYS_ADPT_IPV6_ADDR_LEN);
                (*(pmtu_entry_p+real_entry_num)).destip.type = L_INET_ADDR_TYPE_IPV6;
                (*(pmtu_entry_p+real_entry_num)).destip.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
                real_entry_num++;
            }
        }

        pre_route_list_p = route_list_p;
        route_list_p = route_list_p->next_p;
        L_MM_Free(pre_route_list_p);
    }

    *entry_num_p = real_entry_num;

    if (real_entry_num>0)
        return IPAL_RESULT_OK;
    else
        return IPAL_RESULT_FAIL;
}
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */


static UI32_T IPAL_ROUTE_FetchAllIpv4Route(IPAL_Ipv4UcRouteList_T **route_list_pp)
{
    return IPAL_Rt_Netlink_FetchAllIpv4Route (route_list_pp);
}

UI32_T IPAL_ROUTE_ShowAllIpv4Route()
{
    IPAL_Ipv4UcRouteList_T *route_list_p = NULL, *pre_route_list_p;
    UI8_T        dst[18]= {0};
    UI8_T        nh[18] = {0};
    UI32_T       ifindex;
    UI32_T       table_id;

    IPAL_ROUTE_FetchAllIpv4Route(&route_list_p);

    BACKDOOR_MGR_Printf("\r\n%-20s %-20s %-10s %s\r\n","Destination","Nexthop","Interface","table id");
    BACKDOOR_MGR_Printf("%-20s %-20s %-10s %s\r\n","-----------","-------","---------","--------");
    while (route_list_p)
    {
        memset(dst, 0, sizeof(dst));
        memset(nh, 0, sizeof(nh));
        L_INET_Ntoa(route_list_p->route_entry.dst_p.u.prefix4.s_addr, dst);
        L_INET_Ntoa(route_list_p->route_entry.nh.u.ipv4_nh.nexthopIP.s_addr, nh);

        ifindex = route_list_p->route_entry.nh.u.ipv4_nh.egressIfindex;
        table_id = route_list_p->route_entry.table_id;
        BACKDOOR_MGR_Printf("%-20s %-20s %-10lu %lu\r\n", dst, nh, (unsigned long)ifindex, (unsigned long)table_id);

        pre_route_list_p = route_list_p;
        route_list_p = route_list_p->next_p;
        L_MM_Free(pre_route_list_p);
    }

    return IPAL_RESULT_OK;
}

/*
 *  Name    IPAL_AddIpv4Route
 *  Purpose Add IPv4 route entry in TCP/IP stack
 *  Input   p           -- destination address
 *          nhs         -- nexthop
 *          ifindex     -- ifindex
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_AddIpv4Route(const L_INET_AddrIp_T *p, const L_INET_AddrIp_T *nexthop, UI32_T ifindex)
{
    IPAL_Ipv4UcNextHop_T nhs;
    UI32_T table_id = RT_TABLE_MAIN;

    /* RT_TABLE_MAIN is 254 in rtnetlink.h */
    if(p == NULL)
        return IPAL_RESULT_FAIL;
    nhs.flags = 0;
    nhs.egressIfindex = ifindex;
    nhs.nexthopCount = 0;
    memcpy(&(nhs.nexthopIP.s_addr), nexthop->addr, SYS_ADPT_IPV4_ADDR_LEN);

    return IPAL_Rt_Netlink_AddIpv4Route(table_id, p, &nhs);
}

/*
 *  Name    IPAL_DeleteIpv4Route
 *  Purpose Delete IPv4 route entry in TCP/IP stack
 *  Input   p           -- destination address
 *          nhs         -- nexthop
 *          ifindex     -- ifindex
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
*/
UI32_T IPAL_ROUTE_DeleteIpv4Route (const L_INET_AddrIp_T *p, const L_INET_AddrIp_T *nexthop, UI32_T ifindex)
{
    IPAL_Ipv4UcNextHop_T nhs;
    UI32_T table_id = RT_TABLE_MAIN;

    if(p == NULL)
        return IPAL_RESULT_FAIL;

    nhs.flags = 0;
    nhs.egressIfindex = ifindex;
    nhs.nexthopCount = 0;
    memcpy(&(nhs.nexthopIP.s_addr), nexthop->addr, SYS_ADPT_IPV4_ADDR_LEN);

    return IPAL_Rt_Netlink_DeleteIpv4Route(table_id, p, &nhs);
}

/*
 *  Name    IPAL_ROUTE_DeleteAllIpv4Route
 *  Purpose Delete All IPv4 route entries in TCP/IP stack
 *  Input   None
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_DeleteAllIpv4Route()
{
    IPAL_Ipv4UcRouteList_T *route_list_p = NULL, *pre_route_list_p;
    IPAL_Ipv4UcNextHop_T nhs;
    L_INET_AddrIp_T      prefix;
    UI32_T               ifindex;
    UI32_T               table_id;

    IPAL_ROUTE_FetchAllIpv4Route(&route_list_p);

    while (route_list_p)
    {
        memset(&prefix, 0, sizeof(L_INET_AddrIp_T));
        prefix.type = L_INET_ADDR_TYPE_IPV4;
        prefix.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
        memcpy(prefix.addr, &route_list_p->route_entry.dst_p.u.prefix4.s_addr, SYS_ADPT_IPV4_ADDR_LEN);

        ifindex = route_list_p->route_entry.nh.u.ipv4_nh.egressIfindex;
        table_id = route_list_p->route_entry.table_id;

        nhs.flags         = 0;
        nhs.egressIfindex = ifindex;
        nhs.nexthopCount  = 0;
        nhs.nexthopIP     = route_list_p->route_entry.nh.u.ipv4_nh.nexthopIP;

        IPAL_Rt_Netlink_DeleteIpv4Route(table_id, &prefix, &nhs);

        pre_route_list_p = route_list_p;
        route_list_p = route_list_p->next_p;
        L_MM_Free(pre_route_list_p);
    }

    return IPAL_RESULT_OK;
}

/*
 *  Name    IPAL_ROUTE_RouteLookup
 *  Purpose Find route for a destination address
 *  Input   dest_ip_p -- destination address
 *  Output  src_ip_p     -- source address
 *          nexthop_ip_p -- nexthop address
 *          nexthop_if_p -- nexthop egress interface
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_RouteLookup(L_INET_AddrIp_T *dest_ip_p, L_INET_AddrIp_T *src_ip_p,
                              L_INET_AddrIp_T *nexthop_ip_p, UI32_T *nexthop_if_p)
{
    return IPAL_Rt_Netlink_RouteLookup(dest_ip_p, src_ip_p, nexthop_ip_p, nexthop_if_p);
}

#if (SYS_CPNT_IPV6 == TRUE)
static UI32_T IPAL_ROUTE_FetchAllIpv6Route(IPAL_Ipv6UcRouteList_T **route_list_pp)
{
    return IPAL_Rt_Netlink_FetchAllIpv6Route (route_list_pp);
}

static UI32_T IPAL_ROUTE_FetchAllIpv6RouteCache(IPAL_Ipv6UcRouteList_T **route_list_pp)
{
    return IPAL_Rt_Netlink_FetchAllIpv6RouteCache (route_list_pp);
}

UI32_T IPAL_ROUTE_ShowAllIpv6Route()
{
    IPAL_Ipv6UcRouteList_T *route_list_p = NULL, *pre_route_list_p;
    char         dst[L_INET_MAX_IP6ADDR_STR_LEN+1]= {0};
    char         nh[L_INET_MAX_IP6ADDR_STR_LEN+1] = {0};
    UI32_T       ifindex;
    UI32_T       table_id;

    IPAL_ROUTE_FetchAllIpv6Route(&route_list_p);

    BACKDOOR_MGR_Printf("\r\n%-20s %-20s %-10s %s\r\n","Destination","Nexthop","Interface","table id");
    BACKDOOR_MGR_Printf("%-20s %-20s %-10s %s\r\n","-----------","-------","---------","--------");
    while (route_list_p)
    {
        memset(dst, 0, sizeof(dst));
        memset(nh, 0, sizeof(nh));
        L_INET_Ntop(L_INET_AF_INET6, route_list_p->route_entry.dst_p.u.prefix6.s6_addr, dst, sizeof(dst));
        L_INET_Ntop(L_INET_AF_INET6, route_list_p->route_entry.nh.u.ipv6_nh.nexthopIP.s6_addr, nh, sizeof(nh));

        ifindex = route_list_p->route_entry.nh.u.ipv6_nh.egressIfindex;
        table_id = route_list_p->route_entry.table_id;
        BACKDOOR_MGR_Printf("%-20s %-20s %-10lu %lu\r\n", dst, nh, (unsigned long)ifindex, (unsigned long)table_id);

        pre_route_list_p = route_list_p;
        route_list_p = route_list_p->next_p;
        L_MM_Free(pre_route_list_p);
    }

    return IPAL_RESULT_OK;
}

/*
 *  Name    IPAL_ROUTE_AddIpv6Route
 *  Purpose Add IPv6 route entry in TCP/IP stack
 *  Input   p           -- destination address
 *          nhs         -- nexthop
 *          ifindex     -- ifindex
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_AddIpv6Route(const L_INET_AddrIp_T *p, const L_INET_AddrIp_T *nexthop, UI32_T ifindex)
{
    IPAL_Ipv6UcNextHop_T nhs;
    UI32_T table_id = RT_TABLE_MAIN;

    /* RT_TABLE_MAIN is 254 in rtnetlink.h */
    if(p == NULL)
        return IPAL_RESULT_FAIL;

    nhs.flags = 0;
    nhs.egressIfindex = ifindex;
    nhs.nexthopCount = 0;
    memcpy(nhs.nexthopIP.s6_addr, nexthop->addr, SYS_ADPT_IPV6_ADDR_LEN);

    return IPAL_Rt_Netlink_AddIpv6Route(table_id, p, &nhs);
}

/*
 *  Name    IPAL_ROUTE_DeleteIpv6Route
 *  Purpose Delete IPv6 route entry in TCP/IP stack
 *  Input   p           -- destination address
 *          nhs         -- nexthop
 *          ifindex     -- ifindex
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_DeleteIpv6Route (const L_INET_AddrIp_T *p, const L_INET_AddrIp_T *nexthop, UI32_T ifindex)
{
    IPAL_Ipv6UcNextHop_T nhs;
    UI32_T table_id = RT_TABLE_MAIN;

    if(p == NULL)
        return IPAL_RESULT_FAIL;

    nhs.flags = 0;
    nhs.egressIfindex = ifindex;
    nhs.nexthopCount = 0;
    memcpy(nhs.nexthopIP.s6_addr, nexthop->addr, SYS_ADPT_IPV6_ADDR_LEN);

    return IPAL_Rt_Netlink_DeleteIpv6Route(table_id, p, &nhs);
}

/*
 *  Name    IPAL_ROUTE_DeleteAllIpv6Route
 *  Purpose Delete All IPv6 route entries in TCP/IP stack
 *  Input   None
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_DeleteAllIpv6Route()
{
    IPAL_Ipv6UcRouteList_T *route_list_p = NULL, *pre_route_list_p;
    IPAL_Ipv6UcNextHop_T nhs;
    L_INET_AddrIp_T      prefix;
    UI32_T               ifindex;
    UI32_T               table_id;

    IPAL_ROUTE_FetchAllIpv6Route(&route_list_p);

    while (route_list_p)
    {
        memset(&prefix, 0, sizeof(L_INET_AddrIp_T));
        prefix.type = L_INET_ADDR_TYPE_IPV6;
        prefix.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
        memcpy(prefix.addr, &route_list_p->route_entry.dst_p.u.prefix4.s_addr, SYS_ADPT_IPV6_ADDR_LEN);

        ifindex = route_list_p->route_entry.nh.u.ipv6_nh.egressIfindex;
        table_id = route_list_p->route_entry.table_id;

        nhs.flags         = 0;
        nhs.egressIfindex = ifindex;
        nhs.nexthopCount  = 0;
        nhs.nexthopIP     = route_list_p->route_entry.nh.u.ipv6_nh.nexthopIP;

        IPAL_Rt_Netlink_DeleteIpv6Route(table_id, &prefix, &nhs);

        pre_route_list_p = route_list_p;
        route_list_p = route_list_p->next_p;
        L_MM_Free(pre_route_list_p);
    }

    return IPAL_RESULT_OK;
}

#endif /* #if (SYS_CPNT_IPV6 == TRUE) */


