/* Module Name: _ROUTE_MGR.C
 * Purpose: To provide some APIs for other components to access Phase2 engine.
 *
 * Notes:
 *      1.
 *
 *
 * History:
 *       Date       --  Modifier,   Reason
 *  0.1 2002.02.27  --  Czliao,     Created
 *  2004.06.07  --  wuli        mark out SYS_CPNT_HARDWARE_SUPPORT_L3 since we always support hw routing
 *  2004.06.10  --  amytu       Replace SYS_CPNT_HARDWARE_SUPPORT_L3 by SYS_CPNT_AMTRL3 to seperate layer 3
 *                              host route and net route feature
 *  2007.06.29  --  charlie_chen        port to linux platform
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001, 2002.
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "sys_cpnt.h"
#include "netcfg_om_ip.h"
#include "route_mgr.h"
#if 0
#if (SYS_CPNT_RIP == TRUE)
#include "rip_mgr.h"
#endif
#endif
#include "netcfg_type.h"
#include "sysfun.h"
#include "l_cvrt.h"
#include "l_inet.h"
#include "l_stdlib.h"
#include "backdoor_mgr.h"
#include "l_threadgrp.h"
#if (SYS_CPNT_NSM == TRUE)
#include "nsm_pmgr.h"
#include "nsm_type.h"
#endif
#include "l_prefix.h"
// #include "Pal_types.h"

#if (SYS_CPNT_AMTRL3 == TRUE)
#include "amtrl3_pmgr.h"
#endif

#include "ipal_types.h"
#include "ipal_if.h"
#include "ipal_route.h"

#include "ip_lib.h"
#include "vlan_lib.h"

/* NAME CONSTANT DECLARATIONS
 */
#define NEXT_HOP_AS     0

/* MACRO FUNCTION DECLARATIONS
 */
/*Simon's debug function*/
#define DEBUG_FLAG_BIT_DEBUG 0x01
#define DEBUG_FLAG_BIT_INFO  0x02
#define DEBUG_FLAG_BIT_NOTE  0x04
/***************************************************************/
static unsigned long DEBUG_FLAG = 0;//DEBUG_FLAG_BIT_DEBUG|DEBUG_FLAG_BIT_INFO|DEBUG_FLAG_BIT_NOTE;
/***************************************************************/
#define DBGprintf(format,args...) ((DEBUG_FLAG_BIT_DEBUG & DEBUG_FLAG)==0)?0:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)
#define INFOprintf(format,args...) ((DEBUG_FLAG_BIT_INFO & DEBUG_FLAG)==0)?0:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)
#define NOTEprintf(format,args...) ((DEBUG_FLAG_BIT_NOTE & DEBUG_FLAG)==0)?0:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)

/*Simon's dump memory  function*/
#define DUMP_MEMORY_BYTES_PER_LINE 32
#define DUMP_MEMORY(mptr,size) do{\
    int i;\
    unsigned char * ptr=(unsigned char *)mptr;\
    for(i=0;i<size;i++){\
        if(i%DUMP_MEMORY_BYTES_PER_LINE ==0){\
            if(i>0)printf("\r\n");\
            printf("%0.4xH\t",i);\
        }\
        printf("%0.2x", *ptr++);\
    }\
    printf("\r\n");\
}while(0)
/*END Simon's debug function*/
#if _CHIP_TEST_

#undef  IP_CMN_GetMode
#define IP_CMN_GetMode()  IP_CMN_MODE_MASTER

#endif

/* STATIC VARIABLE DECLARATIONS
 */
#if (SYS_CPNT_OSPF == TRUE)
static UI32_T trigger_ospf_route = 0;
#endif

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME : ROUTE_MGR_InitiateProcessResources
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
void ROUTE_MGR_InitiateProcessResources(void)
{
    return;
}

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
void ROUTE_MGR_Create_InterCSC_Relation(void)
{
}

/* FUNCTION NAME : ROUTE_MGR_Enter_Master_Mode
 * PURPOSE:
 *      Enter Master Mode; could handle ROUTE management operation.
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
 *
 */
void ROUTE_MGR_Enter_Master_Mode(void)
{
//    ROUTE_MGR_Reset();
    return;
}

/* FUNCTION NAME : ROUTE_MGR_Enter_Slave_Mode
 * PURPOSE:
 *      Enter Slave Mode; ignore any request.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *
 * NOTES:
 *
 */
void ROUTE_MGR_Enter_Slave_Mode(void)
{
//    ROUTE_MGR_Reset();
    return;
}

/* FUNCTION NAME : ROUTE_MGR_Enter_Transition_Mode
 * PURPOSE:
 *      Enter Transition Mode; release all resource of ROUTE.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *
 * NOTES:
 *      1. Reset all modules, release all resources.
 */
void ROUTE_MGR_Enter_Transition_Mode(void)
{
//    ROUTE_MGR_Reset();

    return;
}

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
 *      NETCFG_TYPE_FAIL -- Unknown condition
 *
 * NOTES: None.
 */
UI32_T ROUTE_MGR_EnableIpForwarding(UI32_T vr_id, UI8_T addr_type)
{
    UI32_T result;

    /* Because disable ip forwarding in NSM will call disable ip forwarding
     * in AMTRL3, then it will set chip's ip forwarding status to false
     * If ip forwarding is false with L3 bit on for CPU MAC, local host
     * management packet with CPU can't trap to CPU. On the other hand,
     * we can't turn off L3 bit in this case because there might be L2 routing
     * product with ipv6 management request. The chip don't have separate
     * L3 bit for ipv4 and ipv6 in chip. So, currently we let ip forwarding
     * in chip always on. Here, we just enable/disable ip forwarding in kernel.
     */
#if 0
    result = NSM_PMGR_EnableIpForwarding(vr_id, addr_type);

    if(result==NSM_TYPE_RESULT_OK)
        return NETCFG_TYPE_OK;
    return NETCFG_TYPE_FAIL;
#endif

    result = IPAL_RESULT_FAIL;
    if (addr_type == L_INET_ADDR_TYPE_IPV4)
        result = IPAL_ROUTE_EnableIpv4Forwarding();

#if (SYS_CPNT_IPV6 == TRUE)
    else if (addr_type == L_INET_ADDR_TYPE_IPV6)
        result = IPAL_ROUTE_EnableIpv6Forwarding();
#endif

#if (SYS_CPNT_CRAFT_PORT == TRUE)
    if (result == IPAL_RESULT_OK)
        result = IPAL_ROUTE_DisableIfIpv4Forwarding(SYS_ADPT_CRAFT_INTERFACE_IFINDEX);
#endif

    if (result == IPAL_RESULT_OK)
        return NETCFG_TYPE_OK;
    else
        return NETCFG_TYPE_FAIL;
}


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
UI32_T ROUTE_MGR_DisableIpForwarding(UI32_T vr_id, UI8_T addr_type)
{
    UI32_T result;

    /* Because disable ip forwarding in NSM will call disable ip forwarding
     * in AMTRL3, then it will set chip's ip forwarding status to false
     * If ip forwarding is false with L3 bit on for CPU MAC, local host
     * management packet with CPU can't trap to CPU. On the other hand,
     * we can't turn off L3 bit in this case because there might be L2 routing
     * product with ipv6 management request. The chip don't have separate
     * L3 bit for ipv4 and ipv6 in chip. So, currently we let ip forwarding
     * in chip always on. Here, we just enable/disable ip forwarding in kernel.
     */
#if 0
    result = NSM_PMGR_DisableIpForwarding(vr_id, addr_type);

    if(result==NSM_TYPE_RESULT_OK)
        return NETCFG_TYPE_OK;
    return NETCFG_TYPE_FAIL;
#endif

    result = IPAL_RESULT_FAIL;
    if (addr_type == L_INET_ADDR_TYPE_IPV4)
        result = IPAL_ROUTE_DisableIpv4Forwarding();
#if (SYS_CPNT_IPV6 == TRUE)
    else if (addr_type == L_INET_ADDR_TYPE_IPV6)
    {
        INFOprintf("IPAL_ROUTE_DisableIpv6Forwarding");
        result = IPAL_ROUTE_DisableIpv6Forwarding();
    }
#endif
    if (result == IPAL_RESULT_OK)
        return NETCFG_TYPE_OK;
    return NETCFG_TYPE_FAIL;
}



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
UI32_T ROUTE_MGR_AddStaticIpCidrRoute(UI32_T fib_id, L_INET_AddrIp_T *dest_p, L_INET_AddrIp_T *next_hop_p, UI32_T if_index, UI32_T distance)
{
#if (SYS_CPNT_NSM == TRUE)
    if (NSM_TYPE_RESULT_OK != NSM_PMGR_AddStaticIpCidrRoute(dest_p, next_hop_p, if_index, distance))
    {
        return NETCFG_TYPE_CAN_NOT_ADD;
    }
#else
    if (dest_p->type == L_INET_ADDR_TYPE_IPV4 ||
        dest_p->type == L_INET_ADDR_TYPE_IPV4Z)
    {
        IPAL_ROUTE_AddIpv4Route(dest_p, next_hop_p, if_index);
    }
#if (SYS_CPNT_IPV6 == TRUE)
    else if (dest_p->type == L_INET_ADDR_TYPE_IPV6 ||
             dest_p->type == L_INET_ADDR_TYPE_IPV6Z)
    {
        IPAL_IfInfo_T if_info;

        if (next_hop_p->type == L_INET_ADDR_TYPE_IPV6Z)
        {
            if (!VLAN_OM_ConvertToIfindex(next_hop_p->zoneid, &if_index))
                return NETCFG_TYPE_INVALID_ARG;

            if (IPAL_RESULT_OK != IPAL_IF_GetIpv6IfInfo(if_index, &if_info))
                return NETCFG_TYPE_INVALID_ARG;
        }
        IPAL_ROUTE_AddIpv6Route(dest_p, next_hop_p, if_index);
    }
#endif
    else
        return NETCFG_TYPE_INVALID_ARG;
#endif

    return NETCFG_TYPE_OK;
}

/*
 * ROUTINE NAME: ROUTE_MGR_ModifyStaticIpCidrRouteEntry
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
BOOL_T ROUTE_MGR_ModifyStaticIpCidrRouteEntry(UI32_T fib_id, L_INET_AddrIp_T *ip_cidr_route_dest, L_INET_AddrIp_T *ip_cidr_route_next_hop, UI32_T ip_cidr_route_distance)
{
    return TRUE;
}

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
UI32_T ROUTE_MGR_DeleteStaticIpCidrRoute(UI32_T fib_id, L_INET_AddrIp_T *dest_p, L_INET_AddrIp_T *next_hop_p, UI32_T if_index, UI32_T distance)
{
#if (SYS_CPNT_NSM == TRUE)
    if (NSM_TYPE_RESULT_OK != NSM_PMGR_DeleteStaticIpCidrRoute(dest_p, next_hop_p, if_index, distance))
    {
        return NETCFG_TYPE_FAIL;
    }
#else
    if (dest_p->type == L_INET_ADDR_TYPE_IPV4 ||
        dest_p->type == L_INET_ADDR_TYPE_IPV4Z)
    {
        IPAL_ROUTE_DeleteIpv4Route(dest_p, next_hop_p, 0);
    }
#if (SYS_CPNT_IPV6 == TRUE)
    else if (dest_p->type == L_INET_ADDR_TYPE_IPV6 ||
             dest_p->type == L_INET_ADDR_TYPE_IPV6Z)
    {
        IPAL_ROUTE_DeleteIpv6Route(dest_p, next_hop_p, 0);
    }
#endif
    else
        return NETCFG_TYPE_INVALID_ARG;
#endif

    return NETCFG_TYPE_OK;
}

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
 *      NETCFG_TYPE_OK           -- Successfully add the entry to routing table.
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_DELETE -- Unknown condition
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 *      If the route is not dynamic, reject this function.
 */
UI32_T ROUTE_MGR_DeleteDynamicRoute(UI32_T fib_id, L_INET_AddrIp_T *ip_cidr_route_dest)
{
/* charlie_chen:
 *     Zebos nsm cli does not support delete dynamic routes.
 *     The porting of this function will be postponed.
 */
#if 0
    IPNA    ipna;
    iproute_ent_pt iprt;
    UI32_T  ret = NETCFG_TYPE_CAN_NOT_DELETE;

    if(phase2_ip_lock() != SYSFUN_OK)
    {
        return NETCFG_TYPE_CAN_NOT_DELETE;
    }

    ipna.ip_add  = ip_addr;
    ipna.ip_mask = ip_mask;

    /* Bcz phase2 didn't provide API to get certain route.
     * So we need to check whether this route in all dyanmic type
     * In future, should check whether we can use ipFindRt
     */
    if ((iprt = ipFindRoute(&ipna, IPRT_RIP)) != 0)
    {
        /* Bcz RIP will use timer entry of iprt.
         * So we must remove this timer entry from ripTimers
         */
        if(RT_TTL(iprt))
        {
            p2TimerDelete(RT_TTL(iprt), &ripTimers);
        }
        ipDelRoute(iprt);

        ret = NETCFG_TYPE_OK;
    }
    else if ((iprt = ipFindRoute(&ipna, IPRT_OSPF)) != 0)
    {
        ipDelRoute(iprt);
        ret = NETCFG_TYPE_OK;
    }
    else
    {/* static routes */
        ret = NETCFG_TYPE_ENTRY_NOT_EXIST;
    }

    phase2_ip_unlock();

    return ret;
#else
    return NETCFG_TYPE_CAN_NOT_DELETE;
#endif
}

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
UI32_T ROUTE_MGR_SetDefaultRoute(UI32_T fib_id, L_INET_AddrIp_T *default_route_p, UI32_T metrics)
{
    L_INET_AddrIp_T dest;

    memset(&dest, 0, sizeof(L_INET_AddrIp_T));
    switch (default_route_p->type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
            dest.type = L_INET_ADDR_TYPE_IPV4;
            break;
        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
            dest.type = L_INET_ADDR_TYPE_IPV6;
            break;
        default:
            return NETCFG_TYPE_INVLAID_NEXT_HOP;
    }

#if (SYS_CPNT_NSM == TRUE)
    if (NSM_TYPE_RESULT_OK != NSM_PMGR_AddStaticIpCidrRoute(&dest, default_route_p, 0, metrics))
        return NETCFG_TYPE_FAIL;
#else
    if (dest.type == L_INET_ADDR_TYPE_IPV4 ||
        dest.type == L_INET_ADDR_TYPE_IPV4Z)
    {
        IPAL_ROUTE_AddIpv4Route(&dest, default_route_p, 0);
    }
#if (SYS_CPNT_IPV6 == TRUE)
    else
    {
        IPAL_ROUTE_AddIpv6Route(&dest, default_route_p, 0);
    }
#endif
#endif

    return NETCFG_TYPE_OK;

}   /* end of ROUTE_MGR_SetDefaultRoute */

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
UI32_T ROUTE_MGR_DeleteDefaultRoute(L_INET_AddrIp_T *default_route_p)
{
    L_INET_AddrIp_T dest;

    memset(&dest, 0, sizeof(L_INET_AddrIp_T));
    switch (default_route_p->type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
            dest.type = L_INET_ADDR_TYPE_IPV4;
            break;
        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
            dest.type = L_INET_ADDR_TYPE_IPV6;
            break;
        default:
            return NETCFG_TYPE_INVLAID_NEXT_HOP;
    }

#if (SYS_CPNT_NSM == TRUE)
    if (NSM_TYPE_RESULT_OK != NSM_PMGR_DeleteStaticIpCidrRoute(&dest, default_route_p, 0, SYS_ADPT_MIN_ROUTE_DISTANCE))
        return NETCFG_TYPE_FAIL;
#else
    if (dest.type == L_INET_ADDR_TYPE_IPV4 ||
        dest.type == L_INET_ADDR_TYPE_IPV4Z)
    {
        IPAL_ROUTE_DeleteIpv4Route(&dest, default_route_p, 0);
    }
    else if (dest.type == L_INET_ADDR_TYPE_IPV6 ||
             dest.type == L_INET_ADDR_TYPE_IPV6Z)
    {
        IPAL_ROUTE_DeleteIpv6Route(&dest, default_route_p, 0);
    }
#endif

    return NETCFG_TYPE_OK;
}

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
                                UI32_T *nexthop_if_p, UI32_T *owner)
{
    UI8_T zero_addr[SYS_ADPT_IPV6_ADDR_LEN] = {0};
    L_INET_AddrIp_T src_ip;
    UI32_T count = 1;

#if (SYS_CPNT_NSM == TRUE)
    if (NSM_TYPE_RESULT_OK != NSM_PMGR_FindBestRoute(dest_ip_p, nexthop_ip_p, nexthop_if_p, &count, owner))
        return NETCFG_TYPE_FAIL;

    if (count != 1)
        return NETCFG_TYPE_FAIL;

    /* when finding best route is a local host route, it's nexthop will be zero,
     * here we replaced it with destination address
     */
    if(memcmp(nexthop_ip_p->addr, zero_addr, dest_ip_p->addrlen) == 0)
    {
        *nexthop_ip_p = *dest_ip_p;
    }

#else
    if (IPAL_RESULT_OK != IPAL_ROUTE_RouteLookup(dest_ip_p, &src_ip, nexthop_ip_p, nexthop_if_p))
        return NETCFG_TYPE_FAIL;
#endif

    return NETCFG_TYPE_OK;
}

/*
 * ROUTINE NAME: ROUTE_MGR_FlushDynamicRoute
 *
 * FUNCTION: Flush the routing table (remove all the DYNAMIC route).
 *
 * INPUT: None.
 *
 * OUTPUT: None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_CAN_NOT_DELETE
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES: None.
 */
UI32_T ROUTE_MGR_FlushDynamicRoute()
{
/* charlie_chen:
 *     Zebos nsm cli does not support delete dynamic routes.
 *     The porting of this function will be postponed.
 */
#if 0
    int             i, max_riptime;
    UI32_T          update_time;
#if (SYS_CPNT_OSPF == TRUE)
    iproute_ent_pt  iprt = 0;
#endif

#if (SYS_CPNT_RIP == TRUE)
    if(RIP_MGR_GetRip2UpdateTimer(&update_time) == FALSE)
    {
        /* 180 seconds (inactivity) + 120 seconds (hold time) */
        max_riptime = 300;
    }
    else
#endif
    {
        /* (UpdateTimer*6) seconds (inactivity) + (UpdateTimer*4) seconds (hold time) */
        max_riptime = (update_time) * 6 + (update_time * 4);
    }

    if(phase2_ip_lock() == SYSFUN_OK)
    {
        for(i = 0; i < max_riptime; i++)
            p2ProcTimers(&ripTimers);

#if (SYS_CPNT_OSPF == TRUE)
RT_TABLEWALK

        if (RT_UNREACHABLE(iprt))
            continue;

        if (RT_OWNER(iprt) == IPRT_OSPF)
        {
           // DBG_PrintText(" IP Address: %X, Mask: %X, Owner: %d\r\n",
            //          iprt->iprt_ipa.ip_add, iprt->iprt_ipa.ip_mask, iprt->iprt_owner);
            /* ospf route */
            ipDelRoute(iprt);
        }
RT_TABLEWALKEND
       ROUTE_MGR_TriggerOspfRoute(1);

#endif

        phase2_ip_unlock();

        return NETCFG_TYPE_OK;
    }

    return NETCFG_TYPE_CAN_NOT_DELETE;
#else
    return NETCFG_TYPE_OK;
#endif
}

#if 0    // 2002.11.11, William, do not use this function,
         // because, dynamic route should be hanlded by dynamic protocol.
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
 *      NETCFG_TYPE_CAN_NOT_DELETE
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 *      1. Move rip/route entry to proper location, RIPCFG_SignalRifNotInService()
 *         otherwise, there is a coupling btw ROUTE and RIP.
 *      2. ROUTE handling static route entry, dynamic route entry is handled by
 *         protocol, eg. RIP, OSPF.
 */
UI32_T ROUTE_MGR_FlushDynamicRouteByRif(UI32_T ip_addr, UI32_T ip_mask)
{
    UI32_T rifno;
    UI32_T ref=NETCFG_TYPE_OK;
    /* BODY
     */

#if (SYS_CPNT_RIP == TRUE)
    if(phase2_ip_lock() == SYSFUN_OK)
    {
        if(NETIF_MGR_GetRifFromIp(ip_addr, &rifno) == NETIF_MGR_OK)
        {
            rcirc_pt rc = ripCircFromId((int)rifno);
            if(rc)
            {
                RipPurgeRouteByCirc(rc);
            }
            else
            {
                //printf("RIP Circuit ID is invalid\n");
                ret = NETCFG_TYPE_CAN_NOT_DELETE;
            }
        }
        phase2_ip_unlock();
    }
    //printf("Can't get RIP circuit ID\n");
#endif
    return (ret);
}
#endif   /* end of #if 0 */

/*
 * ROUTINE NAME: ROUTE_MGR_FlushStaticRoute
 *
 * FUNCTION: Flush the routing table (remove all the STATIC route).
 *
 * INPUT: None.
 *
 * OUTPUT: None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_CAN_NOT_DELETE
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES: None.
 */
UI32_T ROUTE_MGR_FlushStaticRoute(UI32_T action_flags)
{
    UI32_T ret;

#if (SYS_CPNT_NSM == TRUE)
    if(L_INET_ADDR_TYPE_IPV4 == action_flags)
        ret = NSM_PMGR_DelAllStaticIpv4CidrRoute();
    else if (L_INET_ADDR_TYPE_IPV6 == action_flags)
        ret = NSM_PMGR_DelAllStaticIpv6CidrRoute();
    else
        return NETCFG_TYPE_FAIL;

    if (ret != NSM_TYPE_RESULT_OK)
        return NETCFG_TYPE_FAIL;
#else
    if(L_INET_ADDR_TYPE_IPV4 == action_flags)
        ret = IPAL_ROUTE_DeleteAllIpv4Route();
    else if (L_INET_ADDR_TYPE_IPV6 == action_flags)
        ret = IPAL_ROUTE_DeleteAllIpv6Route();
    else
        return NETCFG_TYPE_FAIL;

    if (ret != IPAL_RESULT_OK)
        return NETCFG_TYPE_FAIL;
#endif

    return NETCFG_TYPE_OK;
}

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
UI32_T ROUTE_MGR_GetIpCidrRouteEntry(ROUTE_MGR_IpCidrRouteEntry_T *route_entry)
{
#if 0//(SYS_CPNT_AMTRL3 == TRUE)
    BOOL_T                          rc;
    AMTRL3_MGR_IpCidrRouteEntry_T  entry;

    /* charlie: TBD
     *          need to make sure that type of
     *          AMTRL3_MGR_IpCidrRouteEntry_T.ip_cidr_route_dest
     *          AMTRL3_MGR_IpCidrRouteEntry_T.ip_cidr_route_mask
     *          AMTRL3_MGR_IpCidrRouteEntry_T.ip_cidr_route_next_hop
     *          has been changed to UI8_T [SYS_ADPT_IPV4_ADDR_LEN]
     */
    memcpy(entry.ip_cidr_route_dest, route_entry->ip_cidr_route_dest, sizeof(entry.ip_cidr_route_dest));
    memcpy(entry.ip_cidr_route_mask, route_entry->ip_cidr_route_mask, sizeof(entry.ip_cidr_route_mask));
    entry.ip_cidr_route_tos     = NEXT_HOP_AS;
    memcpy(entry.ip_cidr_route_next_hop, route_entry->ip_cidr_route_next_hop);

    rc = AMTRL3_PMGR_GetIpCidrRouteEntry(&entry);

    if(rc == TRUE)
    {
        route_entry->ip_cidr_route_if_index = entry.ip_cidr_route_if_index;
        route_entry->ip_cidr_route_metric   = entry.ip_cidr_route_metric1;
        route_entry->ip_cidr_route_status   = entry.ip_cidr_route_status;

        route_entry->ip_cidr_route_type     = entry.ip_cidr_route_type;
        route_entry->ip_cidr_route_proto    = entry.ip_cidr_route_proto;
        route_entry->ip_cidr_route_age      = entry.ip_cidr_route_age;
        route_entry->ip_cidr_route_next_hop_as= entry.ip_cidr_route_next_hop_as;
        route_entry->ip_cidr_route_metric2  = entry.ip_cidr_route_metric2;
        route_entry->ip_cidr_route_metric3  = entry.ip_cidr_route_metric3;
        route_entry->ip_cidr_route_metric4  = entry.ip_cidr_route_metric4;
        route_entry->ip_cidr_route_metric5  = entry.ip_cidr_route_metric5;

        return NETCFG_TYPE_OK;
    }

#endif
    return NETCFG_TYPE_CAN_NOT_GET;
}

/* ROUTINE NAME: ROUTE_MGR_GetNextIpCidrRouteEntry
 *
 * FUNCTION: Get next route (static or dynamic).
 *
 * INPUT:
 *      route_entry->ip_cidr_route_dest
 *      route_entry->ip_cidr_route_mask
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
UI32_T ROUTE_MGR_GetNextIpCidrRouteEntry(ROUTE_MGR_IpCidrRouteEntry_T *route_entry)
{
#if 0//(SYS_CPNT_AMTRL3 == TRUE)
    UI32_T                         rc;
    AMTRL3_MGR_IpCidrRouteEntry_T entry;

    /* charlie: TBD
     *          need to make sure that type of
     *          AMTRL3_MGR_IpCidrRouteEntry_T.ip_cidr_route_dest
     *          AMTRL3_MGR_IpCidrRouteEntry_T.ip_cidr_route_mask
     *          AMTRL3_MGR_IpCidrRouteEntry_T.ip_cidr_route_next_hop
     *          has been changed to UI8_T [SYS_ADPT_IPV4_ADDR_LEN]
     */
    memcpy(entry.ip_cidr_route_dest, route_entry->ip_cidr_route_dest, sizeof(entry.ip_cidr_route_dest));
    memcpy(entry.ip_cidr_route_mask, route_entry->ip_cidr_route_mask, sizeof(entry.ip_cidr_route_dest));
    entry.ip_cidr_route_tos     = NEXT_HOP_AS;
    memcpy(entry.ip_cidr_route_next_hop, route_entry->ip_cidr_route_next_hop, sizeof(entry.ip_cidr_route_next_hop));

    rc = AMTRL3_PMGR_GetNextIpCidrRouteEntry(&entry);

    if(rc == NETCFG_TYPE_NO_MORE_ENTRY)
        return rc;

    if(rc == TRUE) /* wait for IPLRN change spec, 91/06/27 */
    {
        memcpy(route_entry->ip_cidr_route_dest, entry.ip_cidr_route_dest, sizeof(route_entry->ip_cidr_route_dest));
        memcpy(route_entry->ip_cidr_route_mask, entry.ip_cidr_route_mask, sizeof(route_entry->ip_cidr_route_mask));
        route_entry->ip_cidr_route_if_index = entry.ip_cidr_route_if_index;
        memcpy(route_entry->ip_cidr_route_next_hop, entry.ip_cidr_route_next_hop, sizeof(route_entry->ip_cidr_route_next_hop));
        route_entry->ip_cidr_route_metric   = entry.ip_cidr_route_metric1;
        route_entry->ip_cidr_route_status   = entry.ip_cidr_route_status;

        route_entry->ip_cidr_route_type     = entry.ip_cidr_route_type;
        route_entry->ip_cidr_route_proto    = entry.ip_cidr_route_proto;
        route_entry->ip_cidr_route_age      = (SYSFUN_GetSysTick()/100) - entry.ip_cidr_route_age;
        route_entry->ip_cidr_route_next_hop_as= entry.ip_cidr_route_next_hop_as;
        route_entry->ip_cidr_route_metric2  = entry.ip_cidr_route_metric2;
        route_entry->ip_cidr_route_metric3  = entry.ip_cidr_route_metric3;
        route_entry->ip_cidr_route_metric4  = entry.ip_cidr_route_metric4;
        route_entry->ip_cidr_route_metric5  = entry.ip_cidr_route_metric5;

        return NETCFG_TYPE_OK;
    }

#endif
    return NETCFG_TYPE_CAN_NOT_GET;
}

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
UI32_T ROUTE_MGR_Rt6MtuChange(UI32_T ifindex, UI32_T mtu)
{
    return SYSFUN_Syscall(SYSFUN_SYSCALL_ROUTE_MGR,
                          NETCFG_TYPE_SYSCALL_CMD_ROUTE_MGR_RT6_MTU_CHANGE,
                          L_CVRT_UINT_TO_PTR(ifindex), L_CVRT_UINT_TO_PTR(mtu), NULL, NULL);
}

/*
 * ROUTINE NAME: ROUTE_MGR_SetIPForwarding
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
BOOL_T ROUTE_MGR_SetIPForwarding(BOOL_T is_forward)
{

#if 0//(SYS_CPNT_AMTRL3 == TRUE)

    /* biker,
     In original design, ipcfg won't add Vlan Mac when routing is disabled.
     But it will encounter some issue.
     Bcz L3INTF of BCM's chip won't be programmed when routing is disabled.
     It mean any host entries(ARP) and net route(static route) can't be programmed
     into ASIC.
     It should be ok. But we won't reprogrammed those entries when routing is enable again.
     It will cause a lot of information lost in ASIC.
     Due to schedule issue. We do a patch here.
     IPCFG alwasy "AddVlanMac" even routing is disable.
     IPCFG won't change "VlanMac" status when routing status change
     But ROUTE_MGR will turn off "L3ENABLE" flag in "PORT" table of ASIC.
     It will let routing behavior disable.
     We should refine it in future.
   */
    BOOL_T rc;


    if(is_forward == TRUE)
        rc = AMTRL3_PMGR_SetIpForwarding();
    else
        rc = AMTRL3_PMGR_SetIpNotForwarding();

    return rc;

#endif
    return FALSE;
}

void ROUTE_MGR_TriggerOspfRoute(UI32_T trigger_ospf_route_flag)
{
#if (SYS_CPNT_OSPF == TRUE)
    if(trigger_ospf_route_flag == 0)
       trigger_ospf_route = 0;
    else
        trigger_ospf_route = SYSFUN_GetSysTick();
#endif
}

void ROUTE_MGR_GetOspfRouteTriggerTime(UI32_T *ospf_route_trigger_time)
{
#if (SYS_CPNT_OSPF == TRUE)
    *ospf_route_trigger_time = trigger_ospf_route;
#else
    *ospf_route_trigger_time = 0;
#endif
}


/* backdoor function
 */
void ROUTE_MGR_DumpIplrnRoute(void)
{
    ROUTE_MGR_IpCidrRouteEntry_T  entry;
    UI32_T          route_num = 0;
    char           cp1[28], cp2[18], cp3[18];

    BACKDOOR_MGR_Printf("        IP Address\t    IP   Mask\t    Next  Hop\t  Metric\n");

    memset(&entry.ip_cidr_route_dest, 0, sizeof(entry.ip_cidr_route_dest));
//    memset(entry.ip_cidr_route_mask, 0, sizeof(entry.ip_cidr_route_mask));
    entry.ip_cidr_route_tos         = 0;
    memset(&entry.ip_cidr_route_next_hop, 0, sizeof(entry.ip_cidr_route_next_hop));
    while(ROUTE_MGR_GetNextIpCidrRouteEntry(&entry) == NETCFG_TYPE_OK)
    {
        sprintf(cp1, "%d.%d.%d.%d/%d", entry.ip_cidr_route_dest.addr[0], entry.ip_cidr_route_dest.addr[1], entry.ip_cidr_route_dest.addr[2], entry.ip_cidr_route_dest.addr[3], entry.ip_cidr_route_dest.preflen);
        sprintf(cp3, "%d.%d.%d.%d", entry.ip_cidr_route_next_hop.addr[0], entry.ip_cidr_route_next_hop.addr[1], entry.ip_cidr_route_next_hop.addr[2], entry.ip_cidr_route_next_hop.addr[3]);

        BACKDOOR_MGR_Printf("%28s %18s     %2ld\n", cp1, cp3, (long)entry.ip_cidr_route_metric);

        route_num++;
    }

    BACKDOOR_MGR_Printf("\nThere are %ld routes in IPLRN\n", (long)route_num);
}

/* backdoor function
 */

// Convert Hexidecimal ASCII digits to integer
static int ROUTE_MGR_BACKDOOR_AHtoI(char *token)
{
  int result=0, value_added=0, i=0;

   do {
   if((*(token+i) >= '0') && (*(token+i) <= '9'))
    value_added = (int) (*(token+i) - 48);
   else if((*(token+i) >= 'a') && (*(token+i) <= 'f'))
    value_added = (int) (*(token+i) - 87);
   else if((*(token+i) >= 'A') && (*(token+i) <= 'F'))
    value_added = (int) (*(token+i) - 55);
   else
    return -1;
   result = result * 16 + value_added;
   i++;
  } while(*(token+i) != '\0');

   if(result < 0 || result > 255)
    return -1;
   return result;
}


static int ROUTE_MGR_BACKDOOR_AtoIPV6(UI8_T *s, UI8_T *ip)
{
        UI8_T token[50];
        int   i,j;  /* i for s[]; j for token[] */
        int   k,l;  /* k for ip[]; l for copying coutner */

        UI8_T temp[20];

        i = 0;
        j = 0;
        k = 0;
    	l = 0;

        while (s[i] != '\0')
        {
            if ((s[i] == ':') || (j == 2))
            {
		 token[j] = '\0';

                if (strlen((char *)token) < 1 || strlen((char *)token) > 2 ||
                    ROUTE_MGR_BACKDOOR_AHtoI((char *)token) < 0 || ROUTE_MGR_BACKDOOR_AHtoI((char *)token) > 255) // Invalid Token
                    return 0;
                else if (k >= 16)  // Too many digits
                    return 0;
                else // token is ready
                {
                    temp[k++] =(UI8_T)ROUTE_MGR_BACKDOOR_AHtoI((char *)token);
		    if(s[i] == ':')
                     i++;
		    j = 0;
                }
            }
            else if ((s[i] < '0' || s[i] > '9') && (s[i] < 'a' || s[i] > 'f') && (s[i] < 'A' || s[i] > 'F'))
                return 0;
            else
                token[j++] = s[i++];
        } /* while */

        token[j] = '\0';

        if (strlen((char *)token) < 1 || strlen((char *)token) > 2 ||
            ROUTE_MGR_BACKDOOR_AHtoI((char *)token) < 0 || ROUTE_MGR_BACKDOOR_AHtoI((char *)token) > 255) // Invalid Token
            return 0;
        else if (k >= 16)  // Too many digits
            return 0;


        temp[k]=(UI8_T)ROUTE_MGR_BACKDOOR_AHtoI((char *)token);

        for(l=0;l<16;l++)
         ip[l] = temp[l];

        return 1;

}

static int ROUTE_MGR_BACKDOOR_GetIPV6(UI32_T * IPaddr)
{
    int ret = 0;
    UI8_T   *temp_ip = (UI8_T *)IPaddr;
    UI8_T   buffer[50] = {0};

    //if(AMTRL3_BACKDOOR_GetLine((char*)buffer, 20) > 0)
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 50);

    if(strlen((char *)buffer) != 39)
     ret = 0;
    else
     ret = ROUTE_MGR_BACKDOOR_AtoIPV6(buffer, temp_ip);

    if(ret == 0)
    {
        BACKDOOR_MGR_Printf("\nYou entered an invalid IPv6 address\n");
        return  ret;
    }

    //BACKDOOR_MGR_Printf("\n");

    return  1;
}


void ROUTE_MGR_TestAddStaticIPv6Route(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    UI8_T   buffer[20] = {0};
 //   UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV6;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    char    *terminal;
    UI32_T distance;
    L_INET_AddrIp_T dest, next_hop;
    BACKDOOR_MGR_Printf("\n\r Input dest host IPv6 address(x:x:x:x:x:x:x:x): ");
    if(!ROUTE_MGR_BACKDOOR_GetIPV6((UI32_T*)&(dest.addr)))
        return;

    BACKDOOR_MGR_Printf("\n\r Input prefix length(0~64): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    dest.preflen = (UI16_T)strtoul((char *)buffer, &terminal, 10);
    dest.type = L_INET_ADDR_TYPE_IPV6;
    BACKDOOR_MGR_Printf("\n\r Input next hop IPv6 address(x:x:x:x:x:x:x:x): ");
    if(!ROUTE_MGR_BACKDOOR_GetIPV6((UI32_T*)&(next_hop.addr)))
        return;
    next_hop.type = L_INET_ADDR_TYPE_IPV6;

    BACKDOOR_MGR_Printf("\n\r Input Distance: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    distance = (UI32_T)strtoul((char *)buffer, &terminal, 10);
    BACKDOOR_MGR_Printf("\n");

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, member_id);

    ROUTE_MGR_AddStaticIpCidrRoute(fib_id, &dest, &next_hop,0, distance);
    /* Release execution permission from the thread group handler if necessary
     */

    L_THREADGRP_Execution_Release(tg_handle, member_id);

    return;
}

void ROUTE_MGR_TestDeleteStaticIPv6Route(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    UI8_T   buffer[20] = {0};
//    UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV6;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    char    *terminal;
    UI32_T distance;
    L_INET_AddrIp_T dest, next_hop;
    BACKDOOR_MGR_Printf("\n\r Input dest host IPv6 address(x:x:x:x:x:x:x:x): ");
    if(!ROUTE_MGR_BACKDOOR_GetIPV6((UI32_T*)&(dest.addr)))
        return;

    BACKDOOR_MGR_Printf("\n\r Input prefix length(0~64): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    dest.preflen = (UI16_T)strtoul((char *)buffer, &terminal, 10);
    dest.type = L_INET_ADDR_TYPE_IPV6;
    BACKDOOR_MGR_Printf("\n\r Input next hop IPv6 address(x:x:x:x:x:x:x:x): ");
    if(!ROUTE_MGR_BACKDOOR_GetIPV6((UI32_T*)&(next_hop.addr)))
        return;
    next_hop.type = L_INET_ADDR_TYPE_IPV6;

    BACKDOOR_MGR_Printf("\n\r Input Distance: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    distance = (UI32_T)strtoul((char *)buffer, &terminal, 10);
    BACKDOOR_MGR_Printf("\n");

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, member_id);

    ROUTE_MGR_DeleteStaticIpCidrRoute(fib_id,&dest, &next_hop,0, distance);
    /* Release execution permission from the thread group handler if necessary
     */

    L_THREADGRP_Execution_Release(tg_handle, member_id);

    return;
}

#if (SYS_CPNT_NSM == TRUE)
void ROUTE_MGR_DumpZebOSRoute(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    UI32_T ret;
    NSM_MGR_GetNextRouteEntry_T entry;
    UI16_T route_num;

//    entry.action_flags = GET_IPV4_ROUTE_FLAG;
    entry.data.next_route_node=NULL;
    entry.data.next_rib_node=NULL;
    entry.data.next_hop_node=NULL;

    route_num=0;

    L_THREADGRP_Execution_Request(tg_handle, member_id);

    BACKDOOR_MGR_Printf("        IP Address  Prefix Len         Next  Hop  Metric Distance\n");

    for(ret=NSM_PMGR_GetNextIpv4Route(&entry);TRUE;ret=NSM_PMGR_GetNextIpv4Route(&entry))
    {
        UI8_T idx;
        char cp1[18], cp2[18];

        if( (ret==NSM_TYPE_RESULT_OK) || (ret==NSM_TYPE_RESULT_EOF) )
        {
            sprintf(cp1, "%d.%d.%d.%d", entry.data.ip_route_dest.addr[0], entry.data.ip_route_dest.addr[1], entry.data.ip_route_dest.addr[2], entry.data.ip_route_dest.addr[3]);

            for(idx=0; idx<entry.data.num_of_next_hop; idx++)
            {
                sprintf(cp2, "%d.%d.%d.%d", entry.data.ip_next_hop[idx].addr[0], entry.data.ip_next_hop[idx].addr[1], entry.data.ip_next_hop[idx].addr[2], entry.data.ip_next_hop[idx].addr[3]);

                BACKDOOR_MGR_Printf("%18s%12d%18s%8d%9d\n",
                    cp1, entry.data.ip_route_dest_prefix_len, cp2, entry.data.metric,
                    entry.data.distance);
                route_num++;
            }
        }

        if(ret!=NSM_TYPE_RESULT_OK)
            break;
    }

    BACKDOOR_MGR_Printf("\nThere are %ld routes in ZebOS\n", (long)route_num);
    if(ret!=NSM_TYPE_RESULT_EOF)
    {
        BACKDOOR_MGR_Printf("Abnormal return value: %d\n", ret);
    }

    L_THREADGRP_Execution_Release(tg_handle, member_id);
}
#endif /* #if (SYS_CPNT_NSM == TRUE) */

/* Purpose: Convert IP address's netmask into integer
 * Input:  netmask
 * Output: None
 * Return: mask length
 * Notes:  We assume netmask is sequential one.
 *         Argument netmask should be network byte order.
 */
UI8_T ROUTE_MGR_IPv4MaskLen (UI8_T netmask[SYS_ADPT_IPV4_ADDR_LEN])
{
    UI8_T len;
    UI8_T *pnt;
    UI8_T *end;
    UI8_T val;
    UI8_T byte = 0;

    len = 0;
    pnt = (UI8_T *) &netmask;
    end = pnt + IPV4_MAX_BYTELEN;

    while((netmask[byte] == 0xff) && (byte < IPV4_MAX_BYTELEN))
    {
	byte++;
        len+= 8;
    }

    if(byte < IPV4_MAX_BYTELEN)
    {
	val = netmask[byte];
        while (val)
        {
            len++;
            val <<= 1;
        }
    }

    return len;

}

#if 0
/* backdoor function
 */
void ROUTE_MGR_Add2kRoutes(void)
{
#define TEST_INTERFACE_ADR_BYTE0 0x0a
#define TEST_INTERFACE_ADR_BYTE1 0xa0
#define TEST_INTERFACE_ADR_BYTE2 0x00
#define TEST_INTERFACE_ADR_BYTE3 0x00

#define TEST_INTERFACE_ADR_IN_HOST_ORDER ((TEST_INTERFACE_ADR_BYTE0<<24) || \
                                          (TEST_INTERFACE_ADR_BYTE1<<16) || \
                                          (TEST_INTERFACE_ADR_BYTE2<<8)  || \
                                          (TEST_INTERFACE_ADR_BYTE3))

    UI32_T      i, j, ip_addr32, temp32, nexthop32;
    UI8_T       ip_addr[SYS_ADPT_IPV4_ADDR_LEN], ip_mask[SYS_ADPT_IPV4_ADDR_LEN], temp[SYS_ADPT_IPV4_ADDR_LEN];
    IPCFG_TYPE_IPv4RifConfig_T rif_config;
    UI8_T        buf[18];

    BACKDOOR_MGR_Printf("\nInput Start IP : ");
    BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
    /* charlie: TBD
     *          make sure that data type of L_INET_Aton has been changed to UI8_T[SYS_ADPT_IPV4_ADDR_LEN]
     */

    if(L_INET_Aton(buf, ((UI32_T *)(&temp))) == FALSE)
    {
        BACKDOOR_MGR_Printf("Can't convert string to IP\n");
        return ;
    }

    ip_mask[0] = TEST_INTERFACE_ADR_BYTE0;
    ip_mask[1] = TEST_INTERFACE_ADR_BYTE1;
    ip_mask[2] = TEST_INTERFACE_ADR_BYTE2;
    ip_mask[3] = TEST_INTERFACE_ADR_BYTE3;

    temp32 = L_STDLIB_Ntoh32( *((UI32_T*)(&(temp[0]))) );
    for(i = 0; i < 8; i++)
    {
        ip_addr32 = TEST_INTERFACE_ADR_IN_HOST_ORDER +(0x00010000*i);

        nexthop32 = temp32+1;

        for(j = 1;j < 251;j++)
        {
            UI32_T ret;

            ip_addr32 += 0x00000100;

            /* charlie: TBD.
             *          Need to check the use of IPCFG_OM_GetRifFromIp when
             *          the detailed api usage is available
             */
            *((UI32_T*)&(rif_config.ip_addr)) = L_STDLIB_Hton32(nexthop32);
            ret=IPCFG_OM_GetRifFromIp(&rif_config);
            if(ret == NETCFG_TYPE_OK)
            {
                *((UI32_T*)(&(ip_addr[0]))) = L_STDLIB_Hton32(ip_addr32);
                NSM_PMGR_AddStaticRoute(ip_addr, ip_mask, rif_config.ip_addr, 1);
            }
            nexthop32++;
        }
    }
    printf("\n");
}

/* backdoor function
 */
void ROUTE_MGR_DumpRouteNum(void)
{
    UI32_T route_num=0;

    BACKDOOR_MGR_Printf("\n");

    if(NSM_TYPE_RESULT_OK==NSM_PMGR_GetRouteNumber(NSM_MGR_ROUTE_TYPE_LOCAL, &route_num))
    {
        BACKDOOR_MGR_Printf("# of local routes: %lu\n", route_num);
    }

    if(NSM_TYPE_RESULT_OK==NSM_PMGR_GetRouteNumber(NSM_MGR_ROUTE_TYPE_STATIC, &route_num))
    {
        BACKDOOR_MGR_Printf("# of static routes: %lu\n", route_num);
    }

    if(NSM_TYPE_RESULT_OK==NSM_PMGR_GetRouteNumber(NSM_MGR_ROUTE_TYPE_DYNAMIC, &route_num))
    {
        BACKDOOR_MGR_Printf("# of static routes: %lu\n", route_num);
    }

    if(NSM_TYPE_RESULT_OK==NSM_PMGR_GetRouteNumber(NSM_MGR_ROUTE_TYPE_ALL, &route_num))
    {
        BACKDOOR_MGR_Printf("# of all routes: %lu\n", route_num);
    }

}

/* FUNCTION NAME : ROUTE_MGR_Reset
 * PURPOSE:
 *      Reset all resources to Transition State, free dynamic memory allocation,
 *      semaphore to original state.
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
static void ROUTE_MGR_Reset(void)
{
    return;
}


/* Delete all off-static route in Phase2.        */
/* Since we will maintain this off/on mechanism. */
static void ROUTE_MGR_DeleteOffStaticRoute()
{
    IP_TMPSTAT  *iprt,*fwd;

    for(iprt = i_node->rn_stat_fwd; iprt;iprt = fwd)
    {
        fwd = (IP_TMPSTAT *)(iprt->s_dll.dll_fwd);

        ipULink(&iprt->s_dll,(dll_pt)&i_node->rn_stat_fwd);
        ipStaticRtFree(iprt);
    }
}
#endif

