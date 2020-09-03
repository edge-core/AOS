/* Module Name: netcfg_mgr_route.c
 * Purpose:
 *      NETCFG_MGR_ROUTE provides Layer-3 routing configuration management access-point for
 *      upper layer.
 *
 * Notes:
 *
 *
 * History:
 *       Date       --  Modifier,   Reason
 *       12/18/2007 --  Vai Wang,   Created
 *
 * Copyright(C)      Accton Corporation, 2007.
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "sys_bld.h"
#include "eh_mgr.h"
#include "ip_lib.h"
#include "sys_type.h"
#include "netcfg_type.h"
#include "netcfg_mgr_route.h"
#include "netcfg_om_route.h"
#include "netcfg_om_ip.h"
#include "netcfg_pom_ip.h"
#include "netcfg_mgr_nd.h" /* NETCFG_MGR_ND_GetIpNetToPhysicalEntry */
#include "sys_callback_mgr.h"

#include "backdoor_mgr.h"
#include "l_threadgrp.h"
#include "l_stdlib.h"
#include "netcfg_proc_comm.h"
#if (SYS_CPNT_NSM == TRUE)
#include "nsm_pmgr.h"
#include "nsm_type.h"
#endif
#include "l_prefix.h"
#include "l_bitmap.h"
#include "vlan_lib.h"
#include "ip_lib.h"
#include "amtr_pmgr.h"
#include "swctrl_pmgr.h"

#if (SYS_CPNT_CRAFT_PORT == TRUE)
#include "ipal_route.h"
#endif

#if (SYS_CPNT_STATIC_ROUTE_AND_METER_WORKAROUND == TRUE)
#include "stktplg_board.h"
#endif

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
    #include "swdrvl3.h"
#endif

#if (SYS_CPNT_MLDSNP == TRUE)
#include "mldsnp_pmgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define  NETCFG_MGR_DHCP_DEFAULT_GATEWAY_METRIC SYS_DFLT_DEFAULT_GATEWAY_METRIC+1

/* MACRO FUNCTION DECLARATIONS
 */
static UI32_T DEBUG_FLAG =  0;
#define DEBUG_FLAG_BIT_DEBUG 0x01
#define DEBUG_FLAG_BIT_INFO  0x02
#define DEBUG_FLAG_BIT_NOTE  0x04
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

/* DATA TYPE DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

SYSFUN_DECLARE_CSC

/* cfgDb */
#if (SYS_CPNT_CFGDB == TRUE)
#if (SYS_CPNT_NETCFG_IP_ADDRESS_IN_CFGDB == TRUE)
static BOOL_T init_config_db_flag;
static UI8_T default_gw_in_cfgDb[SYS_ADPT_IPV4_ADDR_LEN];    /* default gateway in cfgDb */
static UI32_T default_gw_section_handler;
#endif
#endif

#if (SYS_CPNT_IPV4_ROUTING == TRUE)
BOOL_T switch_back_compatible_ipv4 = FALSE;
#else
BOOL_T switch_back_compatible_ipv4 = TRUE;
#endif

#if (SYS_CPNT_IPV6_ROUTING == TRUE)
BOOL_T switch_back_compatible_ipv6 = FALSE;
#else
BOOL_T switch_back_compatible_ipv6 = TRUE;
#endif

#if (SYS_CPNT_STATIC_ROUTE_AND_METER_WORKAROUND == TRUE)
/* Save status for ip sw-route */
static UI32_T ip_sw_route_status = SYS_DFLT_SW_ROUTE_STATUS;
#endif

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static UI32_T NETCFG_MGR_ROUTE_SetDhcpDefaultGateway(L_INET_AddrIp_T* default_gateway);
 #if 0
static UI32_T NETCFG_MGR_ROUTE_AddStaticRoute(UI8_T ip[SYS_ADPT_IPV4_ADDR_LEN],
                                 UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN],
                                 UI8_T next_hop[SYS_ADPT_IPV4_ADDR_LEN],
                                 UI32_T distance);
 #endif
static UI32_T NETCFG_MGR_ROUTE_DeleteStaticRouteByNetwork(UI8_T ip[SYS_ADPT_IPV4_ADDR_LEN],
                                 UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN]);
#if 0
static UI32_T NETCFG_MGR_ROUTE_DeleteStaticRoute(UI8_T ip[SYS_ADPT_IPV4_ADDR_LEN],
                                 UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN],
                                 UI8_T next_hop[SYS_ADPT_IPV4_ADDR_LEN],
                                 UI32_T distance);
#endif
//static UI32_T NETCFG_MGR_ROUTE_AddStaticIpCidrRouteEntry(UI8_T dest[SYS_ADPT_IPV4_ADDR_LEN],
//                                UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN],
//                                UI8_T next_hop[SYS_ADPT_IPV4_ADDR_LEN],
//                                UI32_T distance);
static UI32_T NETCFG_MGR_ROUTE_EnableIpForwarding(UI32_T vr_id, UI8_T addr_type);
#if (SYS_CPNT_STATIC_ROUTE_AND_METER_WORKAROUND == TRUE)
static UI32_T NETCFG_MGR_ROUTE_EnableSWRoute(UI32_T status);
static UI32_T NETCFG_MGR_ROUTE_GetSWRoute(UI32_T *status);
#endif

static UI32_T NETCFG_MGR_ROUTE_DisableIpForwarding(UI32_T vr_id, UI8_T addr_type);


static UI32_T NETCFG_MGR_ROUTE_AddStaticIpCidrRouteEntry(UI32_T fib_id, L_INET_AddrIp_T *ip_cidr_route_dest, L_INET_AddrIp_T *ip_cidr_route_next_hop, UI32_T ip_cidr_route_if_index,  UI32_T ip_cidr_route_distance);
//static UI32_T NETCFG_MGR_ROUTE_DeleteStaticIpCidrRouteEntry(UI8_T dest[SYS_ADPT_IPV4_ADDR_LEN],
//                                UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN],
//                                UI8_T next_hop[SYS_ADPT_IPV4_ADDR_LEN],
//                                UI32_T distance);
static UI32_T NETCFG_MGR_ROUTE_DeleteStaticIpCidrRouteEntry(UI32_T fib_id, L_INET_AddrIp_T *ip_cidr_route_dest, L_INET_AddrIp_T *ip_cidr_route_next_hop, UI32_T ip_cidr_route_if_index);


//static UI32_T NETCFG_MGR_ROUTE_AddDefaultGateway(UI8_T default_gateway[SYS_ADPT_IPV4_ADDR_LEN], UI32_T distance);
static UI32_T NETCFG_MGR_ROUTE_AddDefaultGateway(UI32_T fib_id, L_INET_AddrIp_T *default_gateway, UI32_T distance);
static UI32_T NETCFG_MGR_ROUTE_AddDhcpDefaultGateway(UI32_T fib_id, L_INET_AddrIp_T *default_gateway);
//static UI32_T NETCFG_MGR_ROUTE_DeleteDefaultGateway(UI8_T default_gateway_ip[SYS_ADPT_IPV4_ADDR_LEN], UI32_T distance);
static UI32_T NETCFG_MGR_ROUTE_DeleteDefaultGateway(UI32_T fib_id, L_INET_AddrIp_T *default_gateway);
//static UI32_T NETCFG_MGR_ROUTE_GetDefaultGateway(UI8_T default_gateway_ip[SYS_ADPT_IPV4_ADDR_LEN]);
static UI32_T NETCFG_MGR_ROUTE_GetDefaultGateway(UI32_T fib_id, L_INET_AddrIp_T *default_gateway_ip);

static SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ROUTE_GetNextRunningStaticIpCidrRouteEntry(NETCFG_TYPE_IpCidrRouteEntry_T *ip_cidr_route_entry);
static SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ROUTE_GetRunningDefaultGateway(UI32_T fib_id, L_INET_AddrIp_T *default_gateway_ip);
#if 0
static UI32_T NETCFG_MGR_ROUTE_SetIpCidrRouteRowStatus(UI8_T ip_addr[SYS_ADPT_IPV4_ADDR_LEN],
                                UI8_T ip_mask[SYS_ADPT_IPV4_ADDR_LEN],
                                UI32_T tos,
                                UI8_T next_hop[SYS_ADPT_IPV4_ADDR_LEN],
                                UI32_T row_status);
static UI32_T NETCFG_MGR_ROUTE_SetRowStatus(UI8_T ip_addr[SYS_ADPT_IPV4_ADDR_LEN],
                                UI8_T ip_mask[SYS_ADPT_IPV4_ADDR_LEN],
                                UI32_T tos,
                                UI8_T next_hop[SYS_ADPT_IPV4_ADDR_LEN],
                                UI32_T row_status);
static UI32_T NETCFG_MGR_ROUTE_GetIpCidrRouteRowStatus(UI8_T ip_addr[SYS_ADPT_IPV4_ADDR_LEN],
                                          UI8_T ip_mask[SYS_ADPT_IPV4_ADDR_LEN],
                                          UI32_T tos,
                                          UI8_T next_hop[SYS_ADPT_IPV4_ADDR_LEN],
                                          UI32_T *row_status);
static UI32_T NETCFG_MGR_ROUTE_GetRowStatus(UI8_T ip_addr[SYS_ADPT_IPV4_ADDR_LEN], UI8_T ip_mask[SYS_ADPT_IPV4_ADDR_LEN], UI32_T tos, UI8_T next_hop[SYS_ADPT_IPV4_ADDR_LEN], UI32_T *row_status);
#endif

static UI32_T NETCFG_MGR_ROUTE_GetReversePathIpMac(L_INET_AddrIp_T *target_addr_p, L_INET_AddrIp_T *nexthop_addr_p, UI8_T *nexthop_mac_addr_p);

static UI32_T NETCFG_MGR_ROUTE_ValidateRouteEntry(ROUTE_MGR_IpCidrRouteEntry_T *entry);

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
static UI32_T NETCFG_MGR_ROUTE_LocalSetEcmpBalanceMode(
    UI32_T mode, UI32_T new_idx, UI32_T old_idx);
#endif

#if 0
#if (SYS_CPNT_IP_TUNNEL == TRUE)
static UI32_T NETCFG_MGR_ROUTE_AddStaticTunnelRouteEntry(UI32_T fib_id, L_INET_AddrIp_T *ip_cidr_route_dest, UI32_T next_hop_ifindex , UI32_T ip_cidr_route_distance);
static UI32_T NETCFG_MGR_ROUTE_AddDynamicTunnelRouteEntry(UI32_T fib_id, L_INET_AddrIp_T *ip_cidr_route_dest, UI32_T next_hop_ifindex, L_INET_AddrIp_T *ip_cidr_route_next_hop , UI32_T ip_cidr_route_distance);

#endif /*SYS_CPNT_IP_TUNNEL*/
#endif
/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME : NETCFG_MGR_ROUTE_EnableIpForwarding
 * PURPOSE:
 *      Enable IPv4/IPv6 forwarding function.
 *
 * INPUT:
 *      vr_id       : virtual router id
 *      address_type: IPv4/IPv6 to enable
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
static UI32_T NETCFG_MGR_ROUTE_EnableIpForwarding(UI32_T vr_id, UI8_T addr_type)
{
#if(SYS_CPNT_ROUTING == TRUE)
    NETCFG_TYPE_L3_Interface_T ip_interface;
    UI32_T                     vid;
    UI32_T forward_status;

    if((NETCFG_MGR_ROUTE_GetIpForwardingStatus(vr_id, addr_type, &forward_status) == NETCFG_TYPE_OK)
        && (forward_status == IP_FORWARDING_ENABLED))
        return NETCFG_TYPE_OK;

    if(ROUTE_MGR_EnableIpForwarding(vr_id, addr_type) != NETCFG_TYPE_OK)
        return NETCFG_TYPE_FAIL;

    /* Set IP forwarding status to OM */
    if(NETCFG_OM_ROUTE_EnableIpForwarding(vr_id, addr_type) != NETCFG_TYPE_OK)
        return NETCFG_TYPE_FAIL;

    memset(&ip_interface, 0, sizeof(NETCFG_TYPE_L3_Interface_T));
    while(NETCFG_OM_IP_GetNextL3Interface(&ip_interface) == NETCFG_TYPE_OK)
    {
        if (TRUE == IP_LIB_IsLoopbackInterface(ip_interface.u.physical_intf.if_flags))
            continue;

        /* TODO: here should filter those ip_interface.iftype == TUNNEL
         *       Currently, iftype haven't implemented yet
         */

        VLAN_IFINDEX_CONVERTTO_VID(ip_interface.ifindex, vid);
        if (AMTR_PMGR_SetCpuMac(vid, ip_interface.u.physical_intf.logical_mac, TRUE) != AMTR_TYPE_RET_SUCCESS)
            return NETCFG_TYPE_FAIL;
    }

    return NETCFG_TYPE_OK;
#endif
    return NETCFG_TYPE_FAIL;
}

#if (SYS_CPNT_STATIC_ROUTE_AND_METER_WORKAROUND == TRUE)
/* FUNCTION NAME : NETCFG_MGR_ROUTE_EnableSWRoute
 * PURPOSE:
 *      Enable: Meter and SW route work. HW route non-work
 *      Disable: HW route work. Meter and SW route non-work
 * INPUT:
 *      status: True - Enable
 *                 False - Disable
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 * NOTES:
 */
static UI32_T NETCFG_MGR_ROUTE_EnableSWRoute(UI32_T status)
{
    UI32_T vlan_id = 0;
    NETCFG_TYPE_L3_Interface_T ip_interface;

    if ( STKTPLG_BOARD_GetStaticRouteAndMeterConflict() != TRUE)
        return NETCFG_TYPE_SW_ROUTE_NOT_NEED;

    ip_sw_route_status = status;
    memset(&ip_interface, 0, sizeof(NETCFG_TYPE_L3_Interface_T));

    if (status)
    {
        /*meter work*/
        AMTR_PMGR_SetRouterAdditionalCtrlReg(TRUE);
        while(NETCFG_POM_IP_GetNextL3Interface(&ip_interface) == NETCFG_TYPE_OK)
        {
            VLAN_OM_ConvertFromIfindex(ip_interface.ifindex, &vlan_id);
            if (vlan_id == 0) /*skip loopback*/
                continue;
            AMTR_PMGR_SetCpuMac(vlan_id, ip_interface.u.physical_intf.logical_mac, FALSE);
        }
    }
    else
    {
        /*static route work*/
        AMTR_PMGR_SetRouterAdditionalCtrlReg(FALSE);
        while(NETCFG_POM_IP_GetNextL3Interface(&ip_interface) == NETCFG_TYPE_OK)
        {
            VLAN_OM_ConvertFromIfindex(ip_interface.ifindex, &vlan_id);
            if (vlan_id == 0) /*skip loopback*/
                continue;
            AMTR_PMGR_SetCpuMac(vlan_id, ip_interface.u.physical_intf.logical_mac, TRUE);
        }
    }

    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_ROUTE_GetSWRoute
 * PURPOSE:
 *      get sw-route status
 * INPUT:
 *      status: True - Enable
 *                 False - Disable
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *
 * NOTES:
 */
static UI32_T NETCFG_MGR_ROUTE_GetSWRoute(UI32_T *status)
{
    *status = ip_sw_route_status;
    return NETCFG_TYPE_OK;
}
#endif /*#if (SYS_CPNT_STATIC_ROUTE_AND_METER_WORKAROUND == TRUE)*/

/* FUNCTION NAME : NETCFG_MGR_ROUTE_DisableIpForwarding
 * PURPOSE:
 *      Disable IPv4/IPv6 forwarding function.
 *
 * INPUT:
 *      vr_id       : virtual router id
 *      address_type: IPv4/IPv6 to enable
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
static UI32_T NETCFG_MGR_ROUTE_DisableIpForwarding(UI32_T vr_id, UI8_T addr_type)
{
#if(SYS_CPNT_ROUTING == TRUE)

    NETCFG_TYPE_L3_Interface_T ip_interface;
    NETCFG_OM_ROUTE_IpForwardingStatus_T ifs;
    UI32_T                     vid;
    UI32_T forward_status;
    NOTEprintf("vrid=%lu,addr=%d",(unsigned long)vr_id,addr_type);

    if((NETCFG_MGR_ROUTE_GetIpForwardingStatus(vr_id, addr_type, &forward_status) == NETCFG_TYPE_OK)
        && (forward_status == IP_FORWARDING_DISABLED))
        return NETCFG_TYPE_OK;


    if(ROUTE_MGR_DisableIpForwarding(vr_id, addr_type) != NETCFG_TYPE_OK)
        return NETCFG_TYPE_FAIL;

    /* Set IP forwarding status to OM */
    if(NETCFG_OM_ROUTE_DisableIpForwarding(vr_id, addr_type) != NETCFG_TYPE_OK)
        return NETCFG_TYPE_FAIL;

    if(NETCFG_OM_ROUTE_GetIpForwardingStatus(&ifs) == NETCFG_TYPE_OK)
    {
        if (!CHECK_FLAG(ifs.status_bitmap, NETCFG_MGR_ROUTE_FLAGS_IPV4) &&
            !CHECK_FLAG(ifs.status_bitmap, NETCFG_MGR_ROUTE_FLAGS_IPV6))
        {
            memset(&ip_interface, 0, sizeof(NETCFG_TYPE_L3_Interface_T));
            while(NETCFG_POM_IP_GetNextL3Interface(&ip_interface) == NETCFG_TYPE_OK)
            {
                if (TRUE == IP_LIB_IsLoopbackInterface(ip_interface.u.physical_intf.if_flags))
                    continue;

                /* TODO: here should filter those ip_interface.iftype == TUNNEL
                 *       Currently, iftype haven't implemented yet
                 */

                VLAN_IFINDEX_CONVERTTO_VID(ip_interface.ifindex, vid);
                DBGprintf("AMTR_PMGR_SetCpuMac vid=%lu", (unsigned long)vid);
                if (AMTR_PMGR_SetCpuMac(vid, ip_interface.u.physical_intf.logical_mac, FALSE) != AMTR_TYPE_RET_SUCCESS)
                    return NETCFG_TYPE_FAIL;
            }
        }
    }

    return NETCFG_TYPE_OK;
#else
    /* For L2 device, CPU mac is set by NETCFG_MGR_IP_SetSingleManagementVlan, not here.
     */
    /* kernel's ipv4/v6 forwarding should be disabled. */
    if (ROUTE_MGR_DisableIpForwarding(vr_id, addr_type) != NETCFG_TYPE_OK)
        return NETCFG_TYPE_FAIL;
    /* Set IP forwarding status to OM */
    if(NETCFG_OM_ROUTE_DisableIpForwarding(vr_id, addr_type) != NETCFG_TYPE_OK)
        return NETCFG_TYPE_FAIL;

    return NETCFG_TYPE_OK;
#endif
}

/* FUNCTION NAME : NETCFG_MGR_ROUTE_GetIpForwardingStatus
 * PURPOSE:
 *      Retrieve IPv4/IPv6 forwarding function status.
 *
 * INPUT:
 *      vr_id       : virtual router id
 *      address_type: L_INET_ADDR_TYPE_IPV4 / L_INET_ADDR_TYPE_IPV6
 *
 * OUTPUT:
 *      *forward_status_p   -- 1 (IP_FORWARDING_ENABLED)
 *                             0 (IP_FORWARDING_DISABLED)
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
UI32_T NETCFG_MGR_ROUTE_GetIpForwardingStatus(UI32_T vr_id, UI8_T addr_type, UI32_T *forward_status_p)
{

    NETCFG_OM_ROUTE_IpForwardingStatus_T ifs;
    memset(&ifs, 0, sizeof(NETCFG_OM_ROUTE_IpForwardingStatus_T));
    ifs.vr_id = vr_id;

    if(NETCFG_OM_ROUTE_GetIpForwardingStatus(&ifs) != NETCFG_TYPE_FAIL)
    {
        if(addr_type == L_INET_ADDR_TYPE_IPV4)
        {
            if(CHECK_FLAG(ifs.status_bitmap, NETCFG_MGR_ROUTE_FLAGS_IPV4))
                *forward_status_p = IP_FORWARDING_ENABLED;
            else
                *forward_status_p = IP_FORWARDING_DISABLED;
        }
        else if(addr_type == L_INET_ADDR_TYPE_IPV6)
        {
            if(CHECK_FLAG(ifs.status_bitmap, NETCFG_MGR_ROUTE_FLAGS_IPV6))
                *forward_status_p = IP_FORWARDING_ENABLED;
            else
                *forward_status_p = IP_FORWARDING_DISABLED;
        }
        return NETCFG_TYPE_OK;
    }

    return NETCFG_TYPE_FAIL;
}

/*
 * ROUTINE NAME: NETCFG_MGR_ROUTE_AddStaticIpCidrRouteEntry
 *
 * FUNCTION: Add static route.
 *
 * INPUT:
 *      dest    - IP address of static route
 *      mask    - IP mask of static route
 *      next_hop- next hop
 *      distance- administrative distance (1 ~ 254)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_ADD -- Unknown condition
 *      NETCFG_TYPE_TABLE_FULL
 *
 * NOTES:
 *      Add static route to the engine.
 */
static UI32_T NETCFG_MGR_ROUTE_AddStaticIpCidrRouteEntry(UI32_T fib_id, L_INET_AddrIp_T *ip_cidr_route_dest, L_INET_AddrIp_T *ip_cidr_route_next_hop, UI32_T ip_cidr_route_if_index,  UI32_T ip_cidr_route_distance)
{
    UI32_T rc = NETCFG_TYPE_OK;
    UI32_T routecfg_static_route_number;
    UI32_T total_nexthops;
    ROUTE_MGR_IpCidrRouteEntry_T entry;
//    BOOL_T is_existed = FALSE;

    /* BODY
     */
//    ip_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV4;
//    memcpy(ip_cidr_route_dest.addr ,dest, SYS_ADPT_IPV4_ADDR_LEN);
//    ip_cidr_route_next_hop.type = L_INET_ADDR_TYPE_IPV4;
//    memcpy(ip_cidr_route_next_hop.addr ,next_hop, SYS_ADPT_IPV4_ADDR_LEN);


    NETCFG_OM_ROUTE_GetStaticIpCidrRouteNumber(ip_cidr_route_dest->type, &routecfg_static_route_number);
    if(routecfg_static_route_number >= SYS_ADPT_MAX_NBR_OF_STATIC_ROUTE_ENTRY)
        return NETCFG_TYPE_TABLE_FULL;


#if (SYS_CPNT_IP_TUNNEL == TRUE)
    {
        NETCFG_TYPE_L3_Interface_T tunnel_if;

        /* we should check if this is static route to tunnel interface, check if interface exist */
        if(IS_TUNNEL_IFINDEX(ip_cidr_route_if_index))
        {
            /* it should check if tunnel interface exist */
            memset(&tunnel_if, 0, sizeof(NETCFG_TYPE_L3_Interface_T));
            tunnel_if.ifindex = ip_cidr_route_if_index;
            if(NETCFG_TYPE_OK != NETCFG_POM_IP_GetL3Interface(&tunnel_if))
                return NETCFG_TYPE_INTERFACE_NOT_EXISTED;


            /* We reject user to set static 6to4 tunnel route with prefix length >= 48,
             * because it will cause problem with dynamic 6to4 net route
             */
            if(IP_LIB_IS_6TO4_ADDR(ip_cidr_route_dest->addr))
            {
                if(ip_cidr_route_dest->preflen >= SYS_ADPT_TUNNEL_6to4_PREFIX_LEN)
                {
                    return NETCFG_TYPE_INVALID_ARG;
                }
            }



        }

    }
#endif

    memset(&entry, 0, sizeof(ROUTE_MGR_IpCidrRouteEntry_T));
    memcpy(&entry.ip_cidr_route_dest, ip_cidr_route_dest, sizeof(entry.ip_cidr_route_dest));
    /* apply mask */
    IP_LIB_GetPrefixAddr(ip_cidr_route_dest->addr, ip_cidr_route_dest->addrlen, ip_cidr_route_dest->preflen, entry.ip_cidr_route_dest.addr);

    entry.ip_cidr_route_tos     = 0;
    memcpy(&entry.ip_cidr_route_next_hop, ip_cidr_route_next_hop, sizeof(entry.ip_cidr_route_next_hop));
    entry.ip_cidr_route_if_index = ip_cidr_route_if_index;
    if(ip_cidr_route_if_index !=0)
        entry.ip_cidr_route_type = VAL_ipCidrRouteType_local;
    else
        entry.ip_cidr_route_type = VAL_ipCidrRouteType_remote;
    entry.ip_cidr_route_distance  = ip_cidr_route_distance;

//  printf("%s(%d): IP:%d.%d.%d.%d\n", __FUNCTION__, __LINE__, entry.ip_cidr_route_dest.addr[0],entry.ip_cidr_route_dest.addr[1], entry.ip_cidr_route_dest.addr[2], entry.ip_cidr_route_dest.addr[3]); fflush(stdout);

//  printf("%s(%d): preflen=%d\n", __FUNCTION__, __LINE__, entry.ip_cidr_route_dest.preflen);
//  printf("%s(%d): IP:%d.%d.%d.%d\n", __FUNCTION__, __LINE__, entry.ip_cidr_route_next_hop.addr[0],entry.ip_cidr_route_next_hop.addr[1],entry.ip_cidr_route_next_hop.addr[2],entry.ip_cidr_route_next_hop.addr[3]); fflush(stdout);

//

    rc = NETCFG_MGR_ROUTE_ValidateRouteEntry(&entry);
    if (rc != NETCFG_TYPE_OK)
        return rc;

    /* If a same static route exist with different distance value, following
     * get operation will replace it by set the argument "nh_replace" to TRUE.
     */
    if (NETCFG_OM_ROUTE_GetSameRoute(&entry, TRUE) == NETCFG_TYPE_ENTRY_EXIST)
        return NETCFG_TYPE_OK;

    /* ES3628BT-FLF-ZZ-00603:
     * Add the validator for the total nexthop number of a route.
     */
    if (NETCFG_OM_ROUTE_GetAllNextHopNumberOfRoute(&entry, &total_nexthops) == FALSE)
        return NETCFG_TYPE_CAN_NOT_ADD;
    if (total_nexthops >= SYS_ADPT_MAX_NBR_OF_HOST_ROUTE)
        return NETCFG_TYPE_TABLE_FULL;

    if(DEBUG_FLAG & DEBUG_FLAG_BIT_DEBUG)
    {
        DUMP_INET_ADDR((*ip_cidr_route_dest));
        DUMP_INET_ADDR((*ip_cidr_route_next_hop));
        printf("ifindex=%lu, distance=%lu\n", (unsigned long)ip_cidr_route_if_index, (unsigned long)ip_cidr_route_distance);
    }

#if (SYS_CPNT_IPV6 == TRUE)
    /* chip doesn't support prefix-lenth > 64, thus reject it */
    if (ip_cidr_route_dest->preflen > 64)
        return NETCFG_TYPE_ROUTE_PREFIX_LEN_MORE_THAN_64;
#endif


#if (SYS_CPNT_CRAFT_PORT == TRUE)
    {
        NETCFG_TYPE_CraftInetAddress_T craft_addr;

        memset(&craft_addr, 0, sizeof(craft_addr));
        craft_addr.addr.type = L_INET_ADDR_TYPE_IPV4;
        if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceInetAddress(&craft_addr))
        {
            UI32_T num_nh = 0;
            ROUTE_MGR_IpCidrRouteEntry_T old_entry;
            BOOL_T is_new_nh_on_craft = FALSE;
            BOOL_T is_old_nh_on_craft = FALSE;

            if(IP_LIB_IsIpBelongToSubnet(craft_addr.addr.addr, craft_addr.addr.preflen, entry.ip_cidr_route_next_hop.addr))
                is_new_nh_on_craft = TRUE;

            memcpy(&old_entry, &entry, sizeof(old_entry));
            memset(&old_entry.ip_cidr_route_next_hop, 0, sizeof(old_entry.ip_cidr_route_next_hop));
            while(NETCFG_TYPE_OK == NETCFG_OM_ROUTE_GetNextStaticIpCidrRoute(&old_entry))
            {
                if(memcmp(&old_entry.ip_cidr_route_dest, &entry.ip_cidr_route_dest, sizeof(L_INET_AddrIp_T)))
                    break;
                num_nh++;
                if(IP_LIB_IsIpBelongToSubnet(craft_addr.addr.addr, craft_addr.addr.preflen, old_entry.ip_cidr_route_next_hop.addr))
                {
                    is_old_nh_on_craft = TRUE;
                    break;
                }
            }
            if(is_old_nh_on_craft && is_new_nh_on_craft)
                return NETCFG_TYPE_ERR_ROUTE_TWO_NEXTHOP_ON_CRAFT_INT_IS_NOT_ALLOWED;
            else if ((num_nh && is_new_nh_on_craft) || (num_nh && is_old_nh_on_craft))
                return NETCFG_TYPE_ERR_ROUTE_NEXTHOP_CONFLICT_ON_NORMAL_INT_AND_CRAFT_INT;
            /* else keep going */

        }
    }
#endif

#if (SYS_CPNT_IP_TUNNEL == TRUE)
    if(IS_TUNNEL_IFINDEX(ip_cidr_route_if_index)&&
       (!IP_MGR_IsTunnelInterfaceExist(ip_cidr_route_if_index)))
    {
        /* if NSM doesn't have tunnel interface,
         * it means tunnel interface is not active,
         * we add route to tunnel when tunnel interface is active,
         * and delete when tunnel interface is inactive
         */
    }
    else
#endif

    /* 1. If SYS_CPNT_NSM is TRUE, add route entry to NSM.
     * And if static route is on the subnet of craft port, since ip address
     * on craft interface do not set to NSM, NSM won't add the route
     * into kernel, so we have to set this route to kernel additionally.
     * 2. If SYS_CPNT_NSM is FALSE, ROUTE_MGR will add route into kernel.
     */

    if(ROUTE_MGR_AddStaticIpCidrRoute(fib_id,
                    &entry.ip_cidr_route_dest,
                                        &entry.ip_cidr_route_next_hop,
                                        ip_cidr_route_if_index,
                                           entry.ip_cidr_route_distance) != NETCFG_TYPE_OK)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                 NETCFG_MGR_ROUTE_ADD_STATIC_IP_CIDR_ROUTE_ENTRY_FUN_NO,
                                 EH_TYPE_MSG_FAILED_TO,
                                 SYSLOG_LEVEL_ERR,
                                 "ROUTE_MGR_AddStaticIpCidrRouteEntry failed");
        return NETCFG_TYPE_CAN_NOT_ADD;
    }

#if ((SYS_CPNT_CRAFT_PORT == TRUE) && (SYS_CPNT_NSM == TRUE))
    {
        NETCFG_TYPE_CraftInetAddress_T craft_addr;

        memset(&craft_addr, 0, sizeof(craft_addr));
        craft_addr.addr.type = L_INET_ADDR_TYPE_IPV4;
        if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceInetAddress(&craft_addr))
        {
            if(IP_LIB_IsIpBelongToSubnet(craft_addr.addr.addr, craft_addr.addr.preflen, entry.ip_cidr_route_next_hop.addr))
            {
                NETCFG_TYPE_L3_Interface_T      intf;
                NETCFG_OM_IP_InetRifConfig_T    rif;
                BOOL_T                          is_conflict = FALSE;

                /* Check whether the static route conflict with active rif
                 */
                memset(&intf, 0, sizeof(intf));

                while(NETCFG_OM_IP_GetNextL3Interface(&intf) == NETCFG_TYPE_OK)
                {
                    if (!(intf.u.physical_intf.if_flags & IFF_RUNNING))
                        continue;

                    memset(&rif, 0, sizeof(rif));
                    rif.addr.type = L_INET_ADDR_TYPE_IPV4;

                    while(NETCFG_OM_IP_GetNextInetRifByIfindex(&rif, intf.ifindex) == NETCFG_TYPE_OK)
                    {
                        if (IP_LIB_IsIpPrefixEqual(&rif.addr, &entry.ip_cidr_route_dest))
                        {
                            is_conflict = TRUE;
                            break;
                        }
                    }

                    if (is_conflict)
                        break;
                }

                if (!is_conflict)
                {
                    /* add into kernel */
                    if (IPAL_RESULT_OK != IPAL_ROUTE_AddIpv4Route(&(entry.ip_cidr_route_dest), &(entry.ip_cidr_route_next_hop), 0))
                        return NETCFG_TYPE_CAN_NOT_ADD;
                }
            }
        }
    }
#endif


    /* Vai:
     * Store to OM
     */
    entry.ip_cidr_route_status = NETCFG_TYPE_StaticIpCidrEntryRowStatus_active;

    rc = NETCFG_OM_ROUTE_AddStaticIpCidrRoute(&entry);
    if (rc == NETCFG_TYPE_CAN_NOT_ADD)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                 NETCFG_MGR_ROUTE_ADD_STATIC_IP_CIDR_ROUTE_ENTRY_FUN_NO,
                                 EH_TYPE_MSG_FAILED_TO,
                                 SYSLOG_LEVEL_ERR,
                                 "NETCFG_MGR_ROUTE_AddStaticIpCidrRoute");
        return rc;
    }

    NETCFG_OM_ROUTE_UpdateStaticIpCidrRouteNumber(ip_cidr_route_dest->type, 1);

    return rc;
}


/*
 * ROUTINE NAME: NETCFG_MGR_ROUTE_DeleteStaticIpCidrRouteEntry
 *
 * FUNCTION: Delete static route.
 *
 * INPUT:
 *      ip_cidr_route_dest  - IP address of static route
 *      ip_cidr_route_mask  - IP mask of static route
 *      ip_cidr_route_tos      - type of service
 *      ip_cidr_route_next_hop - gateway address of net route
 *
 * OUTPUT: None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK           -- Successfully delete the entry from routing table.
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_CAN_NOT_DELETE -- Unknown condition
 *
 * NOTES:
 *      If the route is not static, reject this function.
 */
static UI32_T NETCFG_MGR_ROUTE_DeleteStaticIpCidrRouteEntry(UI32_T fib_id, L_INET_AddrIp_T *ip_cidr_route_dest, L_INET_AddrIp_T *ip_cidr_route_next_hop, UI32_T ip_cidr_route_if_index)
{
    ROUTE_MGR_IpCidrRouteEntry_T   entry;

    /* BODY
     */
    memset(&entry, 0, sizeof(ROUTE_MGR_IpCidrRouteEntry_T));
    memcpy(&entry.ip_cidr_route_dest, ip_cidr_route_dest, sizeof(entry.ip_cidr_route_dest));
    entry.ip_cidr_route_tos     = 0;
    memcpy(&entry.ip_cidr_route_next_hop, ip_cidr_route_next_hop, sizeof(entry.ip_cidr_route_next_hop));
    entry.ip_cidr_route_if_index = ip_cidr_route_if_index;

    if(DEBUG_FLAG & DEBUG_FLAG_BIT_DEBUG)
    {
        DUMP_INET_ADDR((*ip_cidr_route_dest));
        DUMP_INET_ADDR((*ip_cidr_route_next_hop));
        printf("ifindex=%lu\n", (unsigned long)ip_cidr_route_if_index);
    }

    /* Find same static route is in the database */
    /* ES3628BT-FLF-ZZ-00014
     * Delete a static route should remain the same procedure as
     * NSM does:
     * Do not use the distance as a searching KEY.
     */
    if (NETCFG_OM_ROUTE_GetSameRoute(&entry, FALSE) != NETCFG_TYPE_ENTRY_EXIST)
        return NETCFG_TYPE_ENTRY_NOT_EXIST;
    NOTEprintf("dest=%lx:%lx:%lx:%lx/%d, next=%lx:%lx:%lx:%lx; ifidx=%ld",
                        L_INET_EXPAND_IPV6(ip_cidr_route_dest->addr),ip_cidr_route_dest->preflen,
                        L_INET_EXPAND_IPV6(ip_cidr_route_next_hop->addr),
                        (long)ip_cidr_route_if_index);

#if (SYS_CPNT_IP_TUNNEL == TRUE)
    if(IS_TUNNEL_IFINDEX(ip_cidr_route_if_index)&&
       (!IP_MGR_IsTunnelInterfaceExist(ip_cidr_route_if_index)))
    {
        /* if NSM doesn't have tunnel interface,
         * it means tunnel interface is not active,
         * we add route to tunnel when tunnel interface is active,
         * and delete when tunnel interface is inactive
         */
    }
    else
#endif
    if (ROUTE_MGR_DeleteStaticIpCidrRoute(fib_id,
                    &entry.ip_cidr_route_dest,
                                    &entry.ip_cidr_route_next_hop,
                                    ip_cidr_route_if_index,
                                    entry.ip_cidr_route_distance) != NETCFG_TYPE_OK)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                 NETCFG_MGR_ROUTE_DELETE_STATIC_IP_CIDR_ROUTE_ENTRY_FUN_NO,
                                 EH_TYPE_MSG_FAILED_TO,
                                 SYSLOG_LEVEL_ERR,
                                 "ROUTE_MGR_DeleteStaticIpCidrRoute");
        DBGprintf("fail to route mgr");
        return NETCFG_TYPE_CAN_NOT_DELETE;
    }

    /* If static route nexthop is on the subnet of craft port, since NSM don't have
     * craft port ip address, it won't add/delete the route to/from kernel (although
     * static route of craft port also added to NSM), so we have to delete it additionally here.
     */
#if (SYS_CPNT_CRAFT_PORT == TRUE)
    {
        NETCFG_TYPE_CraftInetAddress_T craft_addr;
        memset(&craft_addr, 0, sizeof(craft_addr));
        craft_addr.addr.type = L_INET_ADDR_TYPE_IPV4;
        if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceInetAddress(&craft_addr))
        {
            if(IP_LIB_IsIpBelongToSubnet(craft_addr.addr.addr, craft_addr.addr.preflen, entry.ip_cidr_route_next_hop.addr))
            {
                /* remove from kernel */
                IPAL_ROUTE_DeleteIpv4Route(&(entry.ip_cidr_route_dest), &(entry.ip_cidr_route_next_hop), 0);
            }
        }
    }
#endif

    if (NETCFG_OM_ROUTE_DeleteStaticIpCidrRoute(&entry) != NETCFG_TYPE_OK)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                 NETCFG_MGR_ROUTE_DELETE_STATIC_IP_CIDR_ROUTE_ENTRY_FUN_NO,
                                 EH_TYPE_MSG_FAILED_TO,
                                 SYSLOG_LEVEL_ERR,
                                 "NETCFG_OM_ROUTE_DeleteStaticIpCidrRoute");
        DBGprintf("Fail to OM");
        return NETCFG_TYPE_CAN_NOT_DELETE;
    }

    NETCFG_OM_ROUTE_UpdateStaticIpCidrRouteNumber(ip_cidr_route_dest->type, -1);

    return NETCFG_TYPE_OK;
}


/*
 * ROUTINE NAME: NETCFG_MGR_ROUTE_DeleteAllStaticIpCidrRouteEntries
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
static UI32_T NETCFG_MGR_ROUTE_DeleteAllStaticIpCidrRouteEntries(UI32_T action_flags)
{
    UI32_T  ret;

    /* BODY */

    NETCFG_OM_ROUTE_DeleteAllStaticIpCidrRoutes(action_flags);

    NETCFG_OM_ROUTE_SetStaticIpCidrRouteNumber(action_flags, 0);

    ret = ROUTE_MGR_FlushStaticRoute(action_flags);
    if (ret != NETCFG_TYPE_OK)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                 NETCFG_MGR_ROUTE_DELETE_ALL_STATIC_ROUTE_FUN_NO,
                                 EH_TYPE_MSG_FAILED_TO,
                                 SYSLOG_LEVEL_ERR,
                                 "NETCFG_MGR_ROUTE_DeleteAllStaticIpCidrRouteEntries");
    } /* end of if */
    return ret;
}


/* FUNCTION NAME: NETCFG_MGR_ROUTE_AddDefaultGateway
 * PURPOSE:
 *      Set system default gateway, for multiple default gateway,
 *      distance determine the order be used.
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
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_ADD
 *      NETCFG_TYPE_TABLE_FULL
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 *
 */
static UI32_T NETCFG_MGR_ROUTE_AddDefaultGateway(UI32_T fib_id, L_INET_AddrIp_T *default_gateway, UI32_T distance)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    UI32_T ret=NETCFG_TYPE_OK;
    L_INET_AddrIp_T dest;

    INFOprintf("UI Set gw=%d.%d.%d.%d", L_INET_EXPAND_IPV4(default_gateway->addr));
    /* BODY
    */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    /* Arguments validation Checking */
    if (NULL == default_gateway)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&dest, 0, sizeof(L_INET_AddrIp_T));
#if (SYS_CPNT_IPV6== TRUE)
    if (default_gateway->type == L_INET_ADDR_TYPE_IPV6Z)
    {
        dest.type = L_INET_ADDR_TYPE_IPV6;
        dest.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
    }
    else
    {
        dest.type = default_gateway->type;
        dest.addrlen = default_gateway->addrlen;
    }
#else
    dest.type = default_gateway->type;
    dest.addrlen=SYS_ADPT_IPV4_ADDR_LEN;
#endif /*#if (SYS_CPNT_IPV6== TRUE) */

    if(distance < SYS_ADPT_MIN_ROUTE_DISTANCE || distance > SYS_ADPT_MAX_ROUTE_DISTANCE)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                 NETCFG_MGR_ROUTE_SET_DEFAULT_ROUTE_FUN_NO,
                                 EH_TYPE_MSG_INVALID_PARAMETER,
                                 SYSLOG_LEVEL_ERR,
                                 "distance");
        return NETCFG_TYPE_INVALID_ARG;
    }

    if(default_gateway->type == L_INET_ADDR_TYPE_IPV4)
    {
        if(NETCFG_MGR_ROUTE_IP_IS_ALL_ZEROS(default_gateway->addr))
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                     NETCFG_MGR_ROUTE_SET_DEFAULT_ROUTE_FUN_NO,
                                     EH_TYPE_MSG_INVALID_PARAMETER,
                                     SYSLOG_LEVEL_ERR,
                                     "default-gateway is ZERO");
            return NETCFG_TYPE_INVALID_ARG;
        }
        if (IP_LIB_IsIpInClassD(default_gateway->addr) || IP_LIB_IsIpInClassE(default_gateway->addr) ||
            IP_LIB_IsLoopBackIp(default_gateway->addr) || IP_LIB_IsMulticastIp(default_gateway->addr) ||
            IP_LIB_IsBroadcastIp(default_gateway->addr))
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                     NETCFG_MGR_ROUTE_SET_DEFAULT_ROUTE_FUN_NO,
                                     EH_TYPE_MSG_INVALID_PARAMETER,
                                     SYSLOG_LEVEL_ERR,
                                     "invalid default_gateway");
            return NETCFG_TYPE_INVALID_ARG;
        }
#if (SYS_CPNT_IPV6== TRUE)
    }
    else if ((default_gateway->type == L_INET_ADDR_TYPE_IPV6) || (default_gateway->type == L_INET_ADDR_TYPE_IPV6Z))
    {
        if(NETCFG_MGR_ROUTE_IP6_IS_ALL_ZEROS(default_gateway->addr))
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                     NETCFG_MGR_ROUTE_SET_DEFAULT_ROUTE_FUN_NO,
                                     EH_TYPE_MSG_INVALID_PARAMETER,
                                     SYSLOG_LEVEL_ERR,
                                     "default-gateway is ZERO");
            return NETCFG_TYPE_INVALID_ARG;
        }
        if (IP_LIB_IsIPv6Multicast(default_gateway->addr))
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                     NETCFG_MGR_ROUTE_SET_DEFAULT_ROUTE_FUN_NO,
                                     EH_TYPE_MSG_INVALID_PARAMETER,
                                     SYSLOG_LEVEL_ERR,
                                     "invalid default_gateway");
            return NETCFG_TYPE_INVALID_ARG;
        }
        /* check ipv6 link-local address's zone-id */
        if (L_INET_ADDR_IS_IPV6_LINK_LOCAL(default_gateway->addr) && !L_INET_ADDR_IS_VALID_IPV6_LINK_LOCAL_ZONE_ID(default_gateway->zoneid))
        {
            return NETCFG_TYPE_IPV6_LINK_LOCAL_ADDR_INVALID_ZONE_ID;
        }
#endif /*#if (SYS_CPNT_IPV6== TRUE) */
    }
    else
    {
        return NETCFG_TYPE_INVALID_ARG;
    }

    if (((default_gateway->type == L_INET_ADDR_TYPE_IPV4 ||
          default_gateway->type == L_INET_ADDR_TYPE_IPV4Z)&&
         switch_back_compatible_ipv4 == TRUE) ||
        ((default_gateway->type == L_INET_ADDR_TYPE_IPV6 ||
          default_gateway->type == L_INET_ADDR_TYPE_IPV6Z)&&
         switch_back_compatible_ipv6 == TRUE))
    {
        L_INET_AddrIp_T old_gateway;

        memset(&old_gateway, 0, sizeof(L_INET_AddrIp_T));
        old_gateway.type = dest.type;

        if(ROUTECFG_OM_GetDefaultGatewayCompatible(&old_gateway))
        {
            ret =NETCFG_MGR_ROUTE_DeleteStaticIpCidrRouteEntry(fib_id, &dest, &old_gateway,0);
        }
        ROUTECFG_OM_SetDefaultGatewayCompatible(default_gateway);

        ret = NETCFG_MGR_ROUTE_AddStaticIpCidrRouteEntry(fib_id, &dest, default_gateway,0, SYS_DFLT_DEFAULT_GATEWAY_METRIC);

        /* roll back */
        if(ret != NETCFG_TYPE_OK)
        {
            UI32_T rc;
            rc = NETCFG_MGR_ROUTE_AddStaticIpCidrRouteEntry(fib_id, &dest, &old_gateway, 0, SYS_DFLT_DEFAULT_GATEWAY_METRIC);
            if(rc == NETCFG_TYPE_OK)
            {
                ROUTECFG_OM_SetDefaultGatewayCompatible(&old_gateway);
            }
        }
    }
    else
    {
        ret = NETCFG_MGR_ROUTE_AddStaticIpCidrRouteEntry(fib_id, &dest, default_gateway,0, distance);
    }

    return ret;
}


/* FUNCTION NAME: NETCFG_MGR_ROUTE_DeleteDefaultGateway
 * PURPOSE:
 *      Delete system default gateway which specified in argument.
 *
 * INPUT:
 *      default_gateway_ip -- to be deleted. (network order)
 *      distance -- administrative distance
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 *      NETCFG_TYPE_CAN_NOT_DELETE
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 *  1. If default_gateway_ip == 0, delete all default gateway.
 *  2. Currently, only one gateway, so base on argument 'default_gateway_ip'
 *     to verify consistent or not.
 */
static UI32_T NETCFG_MGR_ROUTE_DeleteDefaultGateway(UI32_T fib_id, L_INET_AddrIp_T *default_gateway)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    L_INET_AddrIp_T dest,old_gw;
    UI32_T rc;

    /* BODY */
    memset(&dest, 0, sizeof(L_INET_AddrIp_T));

    //dest.type = default_gateway->type;
    switch(default_gateway->type)
    {
        case L_INET_ADDR_TYPE_IPV4 :
        case L_INET_ADDR_TYPE_IPV4Z :
            dest.type = L_INET_ADDR_TYPE_IPV4;
            break;
        case L_INET_ADDR_TYPE_IPV6 :
        case L_INET_ADDR_TYPE_IPV6Z :
            dest.type = L_INET_ADDR_TYPE_IPV6;
            break;
        default :
            break;
    }

    dest.addrlen = default_gateway->addrlen;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if (((default_gateway->type == L_INET_ADDR_TYPE_IPV4 ||
          default_gateway->type == L_INET_ADDR_TYPE_IPV4Z)&&
         switch_back_compatible_ipv4 == TRUE) ||
        ((default_gateway->type == L_INET_ADDR_TYPE_IPV6 ||
          default_gateway->type == L_INET_ADDR_TYPE_IPV6Z)&&
         switch_back_compatible_ipv6 == TRUE))
    {
        memset(&old_gw, 0, sizeof(L_INET_AddrIp_T));
        old_gw.type = default_gateway->type;
        if(!ROUTECFG_OM_GetDefaultGatewayCompatible(&old_gw))
        {
            DBGprintf("no gateway to delete??");
            return NETCFG_TYPE_CAN_NOT_DELETE;
        }
        if(memcmp(&old_gw.addr, default_gateway->addr, old_gw.addrlen)!=0)
        {
            DBGprintf("your gw=%lx:%lx:%lx:%lx, but to delete is %lx:%lx:%lx:%lx", L_INET_EXPAND_IPV6(old_gw.addr), L_INET_EXPAND_IPV6(default_gateway->addr) );
            return NETCFG_TYPE_CAN_NOT_DELETE;
        }

        rc = NETCFG_MGR_ROUTE_DeleteStaticIpCidrRouteEntry(fib_id, &dest, default_gateway, 0);
        if(NETCFG_TYPE_OK == rc)
        {
            if(default_gateway->type == L_INET_ADDR_TYPE_IPV4 || default_gateway->type == L_INET_ADDR_TYPE_IPV4Z)
                ROUTECFG_OM_DeleteDefaultGatewayCompatible();
            else
                ROUTECFG_OM_DeleteIpv6DefaultGatewayCompatible();
        }
    }
    else
    {
        rc = NETCFG_MGR_ROUTE_DeleteStaticIpCidrRouteEntry(fib_id, &dest, default_gateway, 0);
    }

   return rc;
}

#if (SYS_CPNT_CRAFT_PORT == TRUE)
/* FUNCTION NAME: NETCFG_MGR_ROUTE_UpdateStaticRoute
 * PURPOSE:
 *      update static routes to linux kernel.
 *
 * INPUT:
 *      ifindex     -- ifindex of the rif
 *      addr_p      -- ip address of the rif
 *      is_active   -- whether the rif is active or de-active
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      Normal vlan interface will set interface rif to both NSM and kernel,
 *      but craft interface's rif will only set to kernel. So when craft interface
 *      rif activated, NSM won't help us to write static routes to kernel, we need
 *      to update those static routes whose next-hop is in craft interface's subnet.
 *      Further more, where rif on interface up/down, we need to check whether need
 *      to delete/add static route which conflict with that rif into kernel.
 */
UI32_T NETCFG_MGR_ROUTE_UpdateStaticRoute(UI32_T ifindex, L_INET_AddrIp_T *addr_p, BOOL_T is_active)
{
    if(NULL == addr_p)
        return NETCFG_TYPE_FAIL;

    /* When craft port rif inactive, kernel will delete related static routes automatically
     */
    if(ifindex == SYS_ADPT_CRAFT_INTERFACE_IFINDEX && is_active)
    {
        /* In L3 product, need to check all static routes
         * In L2 product, only need to check default gateway
         */
#if (SYS_CPNT_ROUTING == TRUE)
        ROUTE_MGR_IpCidrRouteEntry_T entry;

        memset(&entry, 0, sizeof(entry));
        entry.ip_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV4;

        while( NETCFG_OM_ROUTE_GetNextStaticIpCidrRoute(&entry) == NETCFG_TYPE_OK )
        {
            if(IP_LIB_IsIpBelongToSubnet(addr_p->addr, addr_p->preflen, entry.ip_cidr_route_next_hop.addr))
            {
                NETCFG_TYPE_L3_Interface_T      intf;
                NETCFG_OM_IP_InetRifConfig_T    rif;
                BOOL_T                          is_conflict = FALSE;

                /* Check whether the static route conflict with active rif
                 */
                memset(&intf, 0, sizeof(intf));

                while(NETCFG_OM_IP_GetNextL3Interface(&intf) == NETCFG_TYPE_OK)
                {
                    if (!(intf.u.physical_intf.if_flags & IFF_RUNNING))
                        continue;

                    memset(&rif, 0, sizeof(rif));
                    rif.addr.type = L_INET_ADDR_TYPE_IPV4;

                    while(NETCFG_OM_IP_GetNextInetRifByIfindex(&rif, intf.ifindex) == NETCFG_TYPE_OK)
                    {
                        if (IP_LIB_IsIpPrefixEqual(&rif.addr, &entry.ip_cidr_route_dest))
                        {
                            is_conflict = TRUE;
                            break;
                        }
                    }

                    if (is_conflict)
                        break;
                }

                if (!is_conflict)
                {
                    if(IPAL_RESULT_OK != IPAL_ROUTE_AddIpv4Route(&entry.ip_cidr_route_dest, &entry.ip_cidr_route_next_hop, 0))
                        DBGprintf("%s:Failed to add static route to kernal.\n",__FUNCTION__);
                }
            }
        }
#else
        L_INET_AddrIp_T gateway;
        L_INET_AddrIp_T dest;

        memset(&gateway, 0, sizeof(gateway));
        gateway.type = L_INET_ADDR_TYPE_IPV4;
        if(NETCFG_TYPE_OK == NETCFG_MGR_ROUTE_GetDefaultGateway(SYS_ADPT_DEFAULT_FIB, &gateway))
        {
            if(IP_LIB_IsIpBelongToSubnet(addr_p->addr, addr_p->preflen, gateway.addr))
            {
                /* add default gateway to kernel
                 */
                memset(&dest, 0, sizeof(L_INET_AddrIp_T));
                dest.type = L_INET_ADDR_TYPE_IPV4;
                if(IPAL_RESULT_OK != IPAL_ROUTE_AddIpv4Route(&dest, &gateway, 0))
                    DBGprintf("%s:Failed to add default gateway to kernal.\n",__FUNCTION__);
            }
        }
#endif
    }
#if (SYS_CPNT_ROUTING == TRUE)
    else if (ifindex != SYS_ADPT_CRAFT_INTERFACE_IFINDEX)
    {
        ROUTE_MGR_IpCidrRouteEntry_T entry;
        NETCFG_TYPE_CraftInetAddress_T craft_addr;

        memset(&craft_addr, 0, sizeof(craft_addr));
        craft_addr.addr.type = L_INET_ADDR_TYPE_IPV4;
        if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceInetAddress(&craft_addr))
        {
            memset(&entry, 0, sizeof(entry));
            entry.ip_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV4;

            while( NETCFG_OM_ROUTE_GetNextStaticIpCidrRoute(&entry) == NETCFG_TYPE_OK )
            {
                if(IP_LIB_IsIpBelongToSubnet(craft_addr.addr.addr, craft_addr.addr.preflen, entry.ip_cidr_route_next_hop.addr))
                {
                    /* Check whether the static route conflict with rif
                     */
                    if (IP_LIB_IsIpPrefixEqual(addr_p, &entry.ip_cidr_route_dest))
                    {
                        if (is_active)
                            IPAL_ROUTE_DeleteIpv4Route(&entry.ip_cidr_route_dest, &entry.ip_cidr_route_next_hop, 0);
                        else
                            IPAL_ROUTE_AddIpv4Route(&entry.ip_cidr_route_dest, &entry.ip_cidr_route_next_hop, 0);
                    }
                }
            }
        }
    }
#endif

    return NETCFG_TYPE_OK;
}

#endif

UI32_T NETCFG_MGR_ROUTE_DeleteDhcpDefaultGateway(UI32_T fib_id, L_INET_AddrIp_T *default_gateway)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    L_INET_AddrIp_T dest, old_gateway, user_gateway;
    UI32_T rc;

    /* BODY */
    memset(&dest, 0, sizeof(L_INET_AddrIp_T));
    dest.type = default_gateway->type;
    dest.addrlen = default_gateway->addrlen;

    memset(&old_gateway, 0, sizeof(L_INET_AddrIp_T));
    old_gateway.type = default_gateway->type;
    old_gateway.addrlen = default_gateway->addrlen;

    memset(&user_gateway, 0, sizeof(L_INET_AddrIp_T));
    user_gateway.type = default_gateway->type;
    user_gateway.addrlen = default_gateway->addrlen;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    //dhcp can only delete default gateway which is created by dhcp
    if(ROUTECFG_OM_GetDhcpDefaultGateway(&old_gateway))
    {
        if(memcmp(default_gateway->addr,old_gateway.addr,default_gateway->addrlen)!=0)
            return NETCFG_TYPE_CAN_NOT_DELETE;
        //if same as user configured gw, do not delete:
        if(ROUTECFG_OM_GetDefaultGatewayCompatible(&user_gateway) &&
                                memcmp(user_gateway.addr, old_gateway.addr, user_gateway.addrlen)==0)
        {
            if(default_gateway->type == L_INET_ADDR_TYPE_IPV4 || default_gateway->type == L_INET_ADDR_TYPE_IPV4Z)
                ROUTECFG_OM_DeleteDhcpDefaultGateway();
            else
                ROUTECFG_OM_DeleteIpv6DhcpDefaultGateway();
            return NETCFG_TYPE_OK;
        }

        rc = NETCFG_MGR_ROUTE_DeleteStaticIpCidrRouteEntry(fib_id, &dest, default_gateway, 0);
        if(NETCFG_TYPE_OK != rc)
        {
            DBGprintf("Delete default gw fail!");
        }
        else
        {
            if(default_gateway->type == L_INET_ADDR_TYPE_IPV4 || default_gateway->type == L_INET_ADDR_TYPE_IPV4Z)
                ROUTECFG_OM_DeleteDhcpDefaultGateway();
            else
                ROUTECFG_OM_DeleteIpv6DefaultGatewayCompatible();

        }
        return rc;
    }
    else
    {
        INFOprintf("Fail to get from OM");
        return NETCFG_TYPE_FAIL;
    }


}
/* FUNCTION NAME: NETCFG_MGR_ROUTE_GetDefaultGateway
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
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 */
static UI32_T NETCFG_MGR_ROUTE_GetDefaultGateway(UI32_T fib_id, L_INET_AddrIp_T *default_gateway_ip)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    UI32_T ret;

    ROUTE_MGR_IpCidrRouteEntry_T entry;
    UI32_T zero_ip[SYS_ADPT_IPV6_ADDR_LEN] = {0};
    /* BODY
    */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    /* Arguments validation Checking */
    if (NULL == default_gateway_ip)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&entry, 0, sizeof(ROUTE_MGR_IpCidrRouteEntry_T));
    entry.ip_cidr_route_dest.type = default_gateway_ip->type;
    entry.ip_cidr_route_next_hop.type = default_gateway_ip->type;
    entry.ip_cidr_route_dest.preflen = 0;
    entry.ip_cidr_route_distance = 0;

    if(entry.ip_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV4)
    {
        memset(entry.ip_cidr_route_dest.addr, 0 , SYS_ADPT_IPV4_ADDR_LEN);
        memset(entry.ip_cidr_route_next_hop.addr, 0 , SYS_ADPT_IPV4_ADDR_LEN);
    }
    else if(entry.ip_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV6)
    {
        memset(entry.ip_cidr_route_dest.addr, 0 , SYS_ADPT_IPV6_ADDR_LEN);
        memset(entry.ip_cidr_route_next_hop.addr, 0 , SYS_ADPT_IPV6_ADDR_LEN);
    }


    ret = NETCFG_OM_ROUTE_GetNextStaticIpCidrRoute(&entry);

    if (ret != NETCFG_TYPE_OK)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                 NETCFG_MGR_ROUTE_GET_DEFAULT_ROUTE_FUN_NO,
                                 EH_TYPE_MSG_FAILED_TO_GET,
                                 SYSLOG_LEVEL_INFO,
                                 "NETCFG_OM_ROUTE_GetNextRoute");
        return ret;
    }


    if(default_gateway_ip->type == L_INET_ADDR_TYPE_IPV4)
    {
        if ((NETCFG_MGR_ROUTE_IP_IS_ALL_ZEROS(entry.ip_cidr_route_dest.addr)) &&
            (!NETCFG_MGR_ROUTE_IP_IS_ALL_ZEROS(entry.ip_cidr_route_next_hop.addr)))
        {
            memcpy(default_gateway_ip, &entry.ip_cidr_route_next_hop, sizeof(L_INET_AddrIp_T));
            return NETCFG_TYPE_OK;
        }

    }
    else if(default_gateway_ip->type == L_INET_ADDR_TYPE_IPV6)
    {
        if (!(memcmp(entry.ip_cidr_route_dest.addr ,zero_ip, SYS_ADPT_IPV6_ADDR_LEN)) &&
            (memcmp(entry.ip_cidr_route_next_hop.addr, zero_ip, SYS_ADPT_IPV6_ADDR_LEN)))
        {
            memcpy(default_gateway_ip, &entry.ip_cidr_route_next_hop, sizeof(L_INET_AddrIp_T));
            return NETCFG_TYPE_OK;
        }

    }

    return NETCFG_TYPE_ENTRY_NOT_EXIST;
}


/* FUNCTION NAME : NETCFG_MGR_ROUTE_GetNextRunningStaticIpCidrRouteEntry
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
static SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ROUTE_GetNextRunningStaticIpCidrRouteEntry(NETCFG_TYPE_IpCidrRouteEntry_T *ip_cidr_route_entry)
{
    UI32_T ret;
    ROUTE_MGR_IpCidrRouteEntry_T entry;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    /* Arguments validation Checking */
    if (NULL == ip_cidr_route_entry)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    memset(&entry, 0, sizeof(ROUTE_MGR_IpCidrRouteEntry_T));

    memcpy(&entry.ip_cidr_route_dest,&ip_cidr_route_entry->route_dest, sizeof(entry.ip_cidr_route_dest));
    memcpy(&entry.ip_cidr_route_next_hop, &ip_cidr_route_entry->route_next_hop, sizeof(entry.ip_cidr_route_next_hop));
    entry.ip_cidr_route_distance = ip_cidr_route_entry->ip_cidr_route_distance;
    ret = NETCFG_OM_ROUTE_GetNextStaticIpCidrRoute(&entry);
    if (ret != NETCFG_TYPE_OK)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    memcpy(&ip_cidr_route_entry->route_dest, &entry.ip_cidr_route_dest, sizeof(entry.ip_cidr_route_dest));
    memcpy(&ip_cidr_route_entry->route_next_hop, &entry.ip_cidr_route_next_hop, sizeof(entry.ip_cidr_route_next_hop));
    ip_cidr_route_entry->ip_cidr_route_distance = entry.ip_cidr_route_distance;
    ip_cidr_route_entry->ip_cidr_route_status = entry.ip_cidr_route_status;
    ip_cidr_route_entry->ip_cidr_route_tos =entry.ip_cidr_route_tos;
    ip_cidr_route_entry->ip_cidr_route_if_index = entry.ip_cidr_route_if_index;
    ip_cidr_route_entry->ip_cidr_route_type = entry.ip_cidr_route_type;
    ip_cidr_route_entry->ip_cidr_route_proto     = entry.ip_cidr_route_proto;
    ip_cidr_route_entry->ip_cidr_route_age    = entry.ip_cidr_route_age;
    ip_cidr_route_entry->ip_cidr_route_nextHopAS      = entry.ip_cidr_route_next_hop_as;
    ip_cidr_route_entry->ip_cidr_route_metric1       = entry.ip_cidr_route_metric;
    ip_cidr_route_entry->ip_cidr_route_metric2 = entry.ip_cidr_route_metric2;
    ip_cidr_route_entry->ip_cidr_route_metric3 = entry.ip_cidr_route_metric3;
    ip_cidr_route_entry->ip_cidr_route_metric4 = entry.ip_cidr_route_metric4;
    ip_cidr_route_entry->ip_cidr_route_metric5 = entry.ip_cidr_route_metric5;
    ip_cidr_route_entry->ip_cidr_route_status =    entry.ip_cidr_route_status;
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}


/* FUNCTION NAME: NETCFG_MGR_ROUTE_GetRunningDefaultGateway
 * PURPOSE:
 *      Get running configuration of system default gateway.
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
 */
static SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_ROUTE_GetRunningDefaultGateway(UI32_T fib_id, L_INET_AddrIp_T *default_gateway_ip)
{
    UI32_T ret;

    ret = NETCFG_MGR_ROUTE_GetDefaultGateway(fib_id, default_gateway_ip);
    if (ret != NETCFG_TYPE_OK)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;


    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}


/* FUNCTION NAME: NETCFG_MGR_ROUTE_SetDhcpDefaultGateway
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
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 *      1. this default gateway will keep until binding subnet destroyed.
 *      2. DHCP claimed default gateway takes as static configured route,
 *         but do not show in running configuration.
 */
static UI32_T NETCFG_MGR_ROUTE_SetDhcpDefaultGateway(L_INET_AddrIp_T* default_gateway)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    UI32_T ret;

    /* BODY
    */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;


    ret = NETCFG_MGR_ROUTE_AddDhcpDefaultGateway(SYS_ADPT_DEFAULT_FIB,default_gateway);

    return ret;

}


/*
 * ROUTINE NAME: ROUTECFG_AddDhcpDefaultGateway
 * FUNCTION:
 *      add DHCP's default gateway to static routing table.
 *
 * INPUT:
 *      dhcp_gateway - next hop
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 *      NETCFG_TYPE_DESTINATION_IS_LOCAL
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_CAN_NOT_ADD -- Unknown condition
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_TABLE_FULL
 *
 * NOTES:
 *      1. Add DHCP client claimed default gateway as
 *         static route to the engine.
 *      2. This default gateway do not show in running configuration.
 *      3. When dhcp client binding interface destroyed, this entry should
 *         be removed.
 */
static UI32_T NETCFG_MGR_ROUTE_AddDhcpDefaultGateway(UI32_T fib_id, L_INET_AddrIp_T *default_gateway)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    UI32_T ret=NETCFG_TYPE_OK;
    L_INET_AddrIp_T dest;


    /* BODY
    */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    /* Arguments validation Checking */
    if (NULL == default_gateway)
        return NETCFG_TYPE_INVALID_ARG;

    if(NETCFG_MGR_ROUTE_IP_IS_ALL_ZEROS(default_gateway->addr))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                 NETCFG_MGR_ROUTE_ADD_DHCP_DEFAULT_GATEWAY_FUN_NO,
                                 EH_TYPE_MSG_INVALID_PARAMETER,
                                 SYSLOG_LEVEL_ERR,
                                 "default-gateway is ZERO");
        return NETCFG_TYPE_INVALID_ARG;
    }

    if (IP_LIB_IsIpInClassD(default_gateway->addr) || IP_LIB_IsIpInClassE(default_gateway->addr) ||
        IP_LIB_IsLoopBackIp(default_gateway->addr) || IP_LIB_IsMulticastIp(default_gateway->addr) ||
        IP_LIB_IsBroadcastIp(default_gateway->addr))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                 NETCFG_MGR_ROUTE_ADD_DHCP_DEFAULT_GATEWAY_FUN_NO,
                                 EH_TYPE_MSG_INVALID_PARAMETER,
                                 SYSLOG_LEVEL_ERR,
                                 "invalid default_gateway");
        return NETCFG_TYPE_INVALID_ARG;
    }

    memset(&dest,0, sizeof(dest));
#if (SYS_CPNT_IPV6 == TRUE)
    if (default_gateway->type == L_INET_ADDR_TYPE_IPV6Z)
    {
        dest.type = L_INET_ADDR_TYPE_IPV6;
        dest.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
    }
    else
    {
        dest.type = default_gateway->type;
        dest.addrlen = default_gateway->addrlen;
    }
#else
    dest.type = default_gateway->type;
    dest.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
#endif /*#if (SYS_CPNT_IPV6== TRUE) */

    if (((default_gateway->type == L_INET_ADDR_TYPE_IPV4 ||
          default_gateway->type == L_INET_ADDR_TYPE_IPV4Z)&&
         switch_back_compatible_ipv4 == TRUE) ||
        ((default_gateway->type == L_INET_ADDR_TYPE_IPV6 ||
          default_gateway->type == L_INET_ADDR_TYPE_IPV6Z)&&
         switch_back_compatible_ipv6 == TRUE))
    {
        L_INET_AddrIp_T old_gateway;
        if(ROUTECFG_OM_GetDefaultGatewayCompatible(&old_gateway))
        {
            if(memcmp(old_gateway.addr, default_gateway->addr, old_gateway.addrlen)==0)
            {//DHCP can not overwrite user configured gateway
                ROUTECFG_OM_SetDhcpDefaultGateway(default_gateway);
                return  NETCFG_TYPE_OK;
            }
        }

        if(ROUTECFG_OM_GetDhcpDefaultGateway(&old_gateway))
        {
            ret =NETCFG_MGR_ROUTE_DeleteStaticIpCidrRouteEntry(SYS_ADPT_DEFAULT_FIB, &dest, &old_gateway,0);
            if(NETCFG_TYPE_OK != ret)
            {
                DBGprintf("Fail to remove old gateway! %lx", (unsigned long)ret);
            }
        }
    }

    ret = NETCFG_MGR_ROUTE_AddStaticIpCidrRouteEntry(SYS_ADPT_DEFAULT_FIB, &dest, default_gateway,0, NETCFG_MGR_DHCP_DEFAULT_GATEWAY_METRIC);

    if(NETCFG_TYPE_OK == ret)
        ROUTECFG_OM_SetDhcpDefaultGateway(default_gateway);
    else
        INFOprintf("Fail to add route!");

    return ret;
}

#if 0
/* FUNCTION NAME : NETCFG_MGR_ROUTE_AddStaticRoute
 * PURPOSE:
 *      Add a static route entry to routing table.
 *
 * INPUT:
 *      ip          : IP address of the route entry. (network order)
 *      mask        : IP mask of the route entry.(network order)
 *      next_hop    : the ip for this subnet.(network order)
 *      distance    : administrative distance
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK   -- Successfully add the entry to routing table.
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_TABLE_FULL
 *      NETCFG_TYPE_CAN_NOT_ADD  -- Unknown condition
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 *      1. Key is (ip, mask)
 *      2. vid_ifIndex need or not ?
 *         if no need vid_ifIndex, need to change CLI.
 *         or define new function to support CLI.
 *      3. So, define new function for Cisco command line.
 */
static UI32_T NETCFG_MGR_ROUTE_AddStaticRoute(UI8_T ip[SYS_ADPT_IPV4_ADDR_LEN],
                                 UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN],
                                 UI8_T next_hop[SYS_ADPT_IPV4_ADDR_LEN],
                                 UI32_T distance)
{

#if (SYS_CPNT_ROUTING == TRUE)
    /* LOCAL VARIABLES DECLARATIONS
     */
    UI32_T rc = NETCFG_TYPE_FAIL;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    /* Arguments validation Checking */
    if ((NULL == ip) || (NULL == mask) || (NULL == next_hop))
        return NETCFG_TYPE_INVALID_ARG;

    if(distance < SYS_ADPT_MIN_ROUTE_DISTANCE || distance > SYS_ADPT_MAX_ROUTE_DISTANCE)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                 NETCFG_MGR_ROUTE_ADD_STATIC_IP_CIDR_ROUTE_ENTRY_FUN_NO,
                                 EH_TYPE_MSG_INVALID_PARAMETER,
                                 SYSLOG_LEVEL_ERR,
                                 "distance");
        return NETCFG_TYPE_INVALID_ARG;
    }

    if(NETCFG_MGR_ROUTE_IP_IS_ALL_ZEROS(next_hop))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                 NETCFG_MGR_ROUTE_ADD_STATIC_IP_CIDR_ROUTE_ENTRY_FUN_NO,
                                 EH_TYPE_MSG_INVALID_PARAMETER,
                                 SYSLOG_LEVEL_ERR,
                                 "dest or mask or next_hop is ZERO");
        return NETCFG_TYPE_INVALID_ARG;
    }

    if (!IP_LIB_IsValidNetworkMask(mask))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                 NETCFG_MGR_ROUTE_ADD_STATIC_IP_CIDR_ROUTE_ENTRY_FUN_NO,
                                 EH_TYPE_MSG_INVALID_PARAMETER,
                                 SYSLOG_LEVEL_ERR,
                                 "Invalid Mask");
        return NETCFG_TYPE_INVALID_ARG;
    }

    if(!NETCFG_MGR_ROUTE_IP_IS_ALL_ZEROS(ip) &&  IP_LIB_IsZeroNetwork (ip))
    {
        return NETCFG_TYPE_INVALID_ARG;
    }

    if (IP_LIB_IsIpInClassD(ip) || IP_LIB_IsIpInClassE(ip) ||
        IP_LIB_IsLoopBackIp(ip) || IP_LIB_IsMulticastIp(ip) ||
        IP_LIB_IsBroadcastIp(ip))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                 NETCFG_MGR_ROUTE_ADD_STATIC_IP_CIDR_ROUTE_ENTRY_FUN_NO,
                                 EH_TYPE_MSG_INVALID_PARAMETER,
                                 SYSLOG_LEVEL_ERR,
                                 "invalid dest");
        return NETCFG_TYPE_INVALID_ARG;
    }

    if (IP_LIB_IsIpInClassD(next_hop) || IP_LIB_IsIpInClassE(next_hop) ||
        IP_LIB_IsLoopBackIp(next_hop) || IP_LIB_IsMulticastIp(next_hop) ||
        IP_LIB_IsBroadcastIp(next_hop))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                 NETCFG_MGR_ROUTE_ADD_STATIC_IP_CIDR_ROUTE_ENTRY_FUN_NO,
                                 EH_TYPE_MSG_INVALID_PARAMETER,
                                 SYSLOG_LEVEL_ERR,
                                 "invalid next_hop");
        return NETCFG_TYPE_INVALID_ARG;
    }

    rc = NETCFG_MGR_ROUTE_AddStaticIpCidrRouteEntry(ip, mask, next_hop,0, distance);

    return rc;
#endif /* SYS_CPNT_ROUTING */

    return NETCFG_TYPE_CAN_NOT_ADD;
}

#endif
/* FUNCTION NAME : NETCFG_MGR_ROUTE_DeleteStaticRouteByNetwork
 * PURPOSE:
 *      Delete static route entries from routing table by given dest & mask.
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
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_DELETE -- Unknown condition
 *
 * NOTES:
 */
static UI32_T NETCFG_MGR_ROUTE_DeleteStaticRouteByNetwork(UI8_T ip[SYS_ADPT_IPV4_ADDR_LEN],
                                 UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN])
{
#if (SYS_CPNT_ROUTING == TRUE)

    /* LOCAL VARIABLES DECLARATIONS
     */
    UI32_T rc = NETCFG_TYPE_CAN_NOT_DELETE;
    ROUTE_MGR_IpCidrRouteEntry_T   entry;
    L_PREFIX_T prefix, tmp_prefix;
    UI32_T dest_addr, mask_addr;

    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    /* Arguments validation Checking */
    if ((NULL == ip) || (NULL == mask))
        return NETCFG_TYPE_INVALID_ARG;

    if(!IP_LIB_IsValidNetworkMask(mask))
        return NETCFG_TYPE_INVALID_ARG;

    memset(&entry, 0, sizeof(ROUTE_MGR_IpCidrRouteEntry_T));
    memcpy(entry.ip_cidr_route_dest.addr, ip, SYS_ADPT_IPV4_ADDR_LEN);
    entry.ip_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV4;
    memcpy(&mask_addr, mask, SYS_ADPT_IPV4_ADDR_LEN);
    entry.ip_cidr_route_dest.preflen = L_PREFIX_Ip2MaskLen(mask_addr);

    memset(&prefix, 0, sizeof(L_PREFIX_T));
    memcpy(&dest_addr, ip, SYS_ADPT_IPV4_ADDR_LEN);

    L_PREFIX_InetAddr2Prefix(dest_addr, mask_addr, &prefix);
    L_PREFIX_ApplyMask(&prefix);

    while( NETCFG_OM_ROUTE_GetNextStaticIpCidrRoute(&entry) == NETCFG_TYPE_OK )
    {
        memset(&tmp_prefix, 0, sizeof(L_PREFIX_T));
        memcpy(&dest_addr, entry.ip_cidr_route_dest.addr, SYS_ADPT_IPV4_ADDR_LEN);
        L_PREFIX_MaskLen2Ip(entry.ip_cidr_route_dest.preflen, &mask_addr);
        L_PREFIX_InetAddr2Prefix(dest_addr, mask_addr, &tmp_prefix);
        L_PREFIX_ApplyMask(&tmp_prefix);

        /* Should check the got entry is the same network as input value */
        if (L_PREFIX_PrefixSame(&prefix, &tmp_prefix) == 0)
            continue;

        rc = NETCFG_MGR_ROUTE_DeleteStaticIpCidrRouteEntry(0,
                                            &entry.ip_cidr_route_dest,
                                            &entry.ip_cidr_route_next_hop,
                                            0);
        if (rc != NETCFG_TYPE_OK)
            return rc;
    }

    return rc;

#endif /* SYS_CPNT_ROUTING */

    return NETCFG_TYPE_CAN_NOT_DELETE;
}


#if 0
/* FUNCTION NAME : NETCFG_MGR_ROUTE_DeleteStaticRoute
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
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_DELETE -- Unknown condition
 *
 * NOTES:
 */
static UI32_T NETCFG_MGR_ROUTE_DeleteStaticRoute(UI8_T ip[SYS_ADPT_IPV4_ADDR_LEN],
                                 UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN],
                                 UI8_T next_hop[SYS_ADPT_IPV4_ADDR_LEN],
                                 UI32_T distance)
{
#if (SYS_CPNT_ROUTING == TRUE)

    /* LOCAL VARIABLES DECLARATIONS
     */
    UI32_T rc;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    /* Arguments validation Checking */
    if ((NULL == ip) || (NULL == mask) || (NULL == next_hop))
        return NETCFG_TYPE_INVALID_ARG;

    if(!IP_LIB_IsValidNetworkMask(mask))
        return NETCFG_TYPE_INVALID_ARG;

    if(distance < SYS_ADPT_MIN_ROUTE_DISTANCE || distance > SYS_ADPT_MAX_ROUTE_DISTANCE)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                 NETCFG_MGR_ROUTE_DELETE_STATIC_IP_CIDR_ROUTE_ENTRY_FUN_NO,
                                 EH_TYPE_MSG_INVALID_PARAMETER,
                                 SYSLOG_LEVEL_ERR,
                                 "distance");
        return NETCFG_TYPE_INVALID_ARG;
    }

    rc = NETCFG_MGR_ROUTE_DeleteStaticIpCidrRouteEntry(ip, mask, next_hop,0, distance);

    return rc;

#endif /* SYS_CPNT_ROUTING */

    return NETCFG_TYPE_CAN_NOT_DELETE;
}


/* FUNCTION NAME : NETCFG_MGR_ROUTE_DeleteAllStaticIpCidrRoutes
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
UI32_T NETCFG_MGR_ROUTE_DeleteAllStaticIpCidrRoutes()
{

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_FAIL;

#if (SYS_CPNT_ROUTING == TRUE)
    if(NETCFG_MGR_ROUTE_DeleteAllStaticIpCidrRouteEntries() != NETCFG_TYPE_OK)
        return NETCFG_TYPE_FAIL;
#endif /* SYS_CPNT_ROUTING */
    return NETCFG_TYPE_OK;
}


/*
 * ROUTINE NAME: NETCFG_MGR_ROUTE_GetIpCidrRouteEntry
 *
 * FUNCTION: Get the route in the routing table.
 *
 * INPUT:
 *      ip_cidr_route_dest - IP address
 *      ip_cidr_route_mask - IP mask
 *
 * OUTPUT:
 *      ip_cidr_route_metric  - Route metric
 *      ip_cidr_route_status  - VALID/INVALID
 *      ip_cidr_route_if_num  - Interface number of the route
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
UI32_T NETCFG_MGR_ROUTE_GetIpCidrRouteEntry(NETCFG_TYPE_IpCidrRouteEntry_T *route_entry)
{
#if (SYS_CPNT_ROUTING == TRUE)
    UI32_T          rc;
    ROUTE_MGR_IpCidrRouteEntry_T  entry;

    if(route_entry == 0)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                 NETCFG_MGR_ROUTE_GET_IP_CIDR_ROUTE_ENTRY_FUN_NO,
                                 EH_TYPE_MSG_INVALID_PARAMETER,
                                 SYSLOG_LEVEL_ERR,
                                 "route_entry");
        return NETCFG_TYPE_INVALID_ARG;
    }

    memcpy(entry.ip_cidr_route_dest, route_entry->ip_cidr_route_dest, sizeof(entry.ip_cidr_route_dest));
    memcpy(entry.ip_cidr_route_mask, route_entry->ip_cidr_route_mask, sizeof(entry.ip_cidr_route_mask));
    entry.ip_cidr_route_tos = route_entry->ip_cidr_route_tos;
    memcpy(entry.ip_cidr_route_next_hop, route_entry->ip_cidr_route_next_hop, sizeof(entry.ip_cidr_route_next_hop));

    rc = ROUTE_MGR_GetIpCidrRouteEntry(&entry);

    if(rc == NETCFG_TYPE_OK)
    {
        route_entry->ip_cidr_route_if_index = entry.ip_cidr_route_if_index;
        route_entry->ip_cidr_route_metric1  = entry.ip_cidr_route_metric;
        route_entry->ip_cidr_route_status   = VAL_ipCidrRouteStatus_active;
        route_entry->ip_cidr_route_type     = entry.ip_cidr_route_type;
        route_entry->ip_cidr_route_proto    = entry.ip_cidr_route_proto;
        route_entry->ip_cidr_route_age      = entry.ip_cidr_route_age;
        route_entry->ip_cidr_route_nextHopAS= entry.ip_cidr_route_next_hop_as;
        route_entry->ip_cidr_route_metric2  = entry.ip_cidr_route_metric2;
        route_entry->ip_cidr_route_metric3  = entry.ip_cidr_route_metric3;
        route_entry->ip_cidr_route_metric4  = entry.ip_cidr_route_metric4;
        route_entry->ip_cidr_route_metric5  = entry.ip_cidr_route_metric5;

        route_entry->ip_cidr_route_ext_subtype = VAL_ipCidrRouteExtOspfSubType_none;
        if(route_entry->ip_cidr_route_proto == VAL_ipCidrRouteProto_ospf)
        {
           route_entry->ip_cidr_route_ext_subtype = (route_entry->ip_cidr_route_type    - 1000 - VAL_ipCidrRouteType_remote);
           route_entry->ip_cidr_route_type = VAL_ipCidrRouteType_remote;

        }
    }
    else
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                 NETCFG_MGR_ROUTE_GET_IP_CIDR_ROUTE_ENTRY_FUN_NO,
                                 EH_TYPE_MSG_FAILED_TO,
                                 SYSLOG_LEVEL_ERR,
                                 "NETCFG_MGR_ROUTE_GetIpCidrRouteEntry");
    }

    return rc;
#endif /* SYS_CPNT_ROUTING */
    return NETCFG_TYPE_CAN_NOT_GET;
}


/* FUNCTION NAME: NETCFG_MGR_ROUTE_SetIpCidrRouteRowStatus
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
static UI32_T NETCFG_MGR_ROUTE_SetIpCidrRouteRowStatus(UI8_T ip_addr[SYS_ADPT_IPV4_ADDR_LEN],
                                          UI8_T ip_mask[SYS_ADPT_IPV4_ADDR_LEN],
                                          UI32_T tos,
                                          UI8_T next_hop[SYS_ADPT_IPV4_ADDR_LEN],
                                          UI32_T row_status)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    UI32_T rc;

    /* BODY  */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_CAN_NOT_SET;

    rc = NETCFG_MGR_ROUTE_SetRowStatus(ip_addr, ip_mask, tos, next_hop, row_status);

    return rc;
}


/* FUNCTION NAME: NETCFG_MGR_ROUTE_GetIpCidrRouteRowStatus
 *
 * PURPOSE:
 *      Get the row status field of the route entry
 *
 * INPUT:
 *      ip_addr     : IP address of rif. (network order)
 *      ip_mask     : Mask of rif. (network order)
 *      tos         : type of service. (must be 0).
 *      next_hop    : NextHop of route entry. (network order)
 *
 * OUTPUT:
 *      row_status  :
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
 *      row_status must be one of the following values:
 *            VAL_ipCidrRouteStatus_active
 *            VAL_ipCidrRouteStatus_notInService
 *            VAL_ipCidrRouteStatus_createAndGo
 *            VAL_ipCidrRouteStatus_createAndWait
 *            VAL_ipCidrRouteStatus_destroy
 */
static UI32_T NETCFG_MGR_ROUTE_GetIpCidrRouteRowStatus(UI8_T ip_addr[SYS_ADPT_IPV4_ADDR_LEN],
                                          UI8_T ip_mask[SYS_ADPT_IPV4_ADDR_LEN],
                                          UI32_T tos,
                                          UI8_T next_hop[SYS_ADPT_IPV4_ADDR_LEN],
                                          UI32_T *row_status)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    UI32_T rc;

    /* BODY */


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_CAN_NOT_GET;

    if(row_status == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    rc = NETCFG_MGR_ROUTE_GetRowStatus(ip_addr, ip_mask, tos, next_hop, row_status);

    return rc;
}

#endif


/* FUNCTION NAME : NETCFG_MGR_ROUTE_GetReversePathIpMac
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
static UI32_T NETCFG_MGR_ROUTE_GetReversePathIpMac(L_INET_AddrIp_T *target_addr_p, L_INET_AddrIp_T *nexthop_addr_p, UI8_T *nexthop_mac_addr_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    L_INET_AddrIp_T dest_addr, src_addr, nexthop_addr;
    UI32_T out_ifindex;
    NETCFG_TYPE_IpNetToPhysicalEntry_T entry;

    /* BODY */
    memset(&dest_addr, 0, sizeof(dest_addr));
    memset(&src_addr, 0, sizeof(src_addr));
    memset(&nexthop_addr, 0, sizeof(nexthop_addr));
    memset(&entry, 0, sizeof(entry));

    dest_addr = *target_addr_p;
    if (IPAL_RESULT_OK != IPAL_ROUTE_RouteLookup(&dest_addr, &src_addr, &nexthop_addr, &out_ifindex))
        return NETCFG_TYPE_FAIL;

    entry.ip_net_to_physical_if_index = out_ifindex;
    entry.ip_net_to_physical_net_address = nexthop_addr;

    if(TRUE != NETCFG_MGR_ND_GetIpNetToPhysicalEntry(&entry))
        return NETCFG_TYPE_FAIL;

    *nexthop_addr_p = nexthop_addr;
    memcpy(nexthop_mac_addr_p, entry.ip_net_to_physical_phys_address.phy_address_cctet_string, SYS_ADPT_MAC_ADDR_LEN);
    return NETCFG_TYPE_OK;

}



#if 0
/* ROUTINE NAME: NETCFG_MGR_ROUTE_SetRowStatus
 *
 * FUNCTION:
 *      Set row status of route entry.
 *
 * INPUT:
 *      ip_addr     : IP address of rif.
 *      ip_mask     : Mask of rif.
 *      tos         : type of service. (must be 0).
 *      next_hop    : NextHop of route entry.
 *      row_status  :
 *
 * OUTPUT:
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_SET
 *
 * NOTES:
 *      Key is (ip_addr, ip_mask, tos, next_hop).
 */
static UI32_T NETCFG_MGR_ROUTE_SetRowStatus(UI8_T ip_addr[SYS_ADPT_IPV4_ADDR_LEN], UI8_T ip_mask[SYS_ADPT_IPV4_ADDR_LEN], UI32_T tos, UI8_T next_hop[SYS_ADPT_IPV4_ADDR_LEN], UI32_T row_status)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
#define     DEFAULT_METRIC      1

    /* LOCAL VARIABLES DEFINITION
     */

    switch(row_status)
    {
        case VAL_ipCidrRouteStatus_active:
        case VAL_ipCidrRouteStatus_notInService:
        case VAL_ipCidrRouteStatus_notReady:
            return NETCFG_TYPE_CAN_NOT_SET;

        case VAL_ipCidrRouteStatus_createAndGo:
            if(NETCFG_MGR_ROUTE_AddStaticIpCidrRouteEntry(ip_addr, ip_mask, next_hop,0, DEFAULT_METRIC) != NETCFG_TYPE_OK)
            {
                return NETCFG_TYPE_CAN_NOT_SET;
            }
            break;

        case VAL_ipCidrRouteStatus_createAndWait:
            return NETCFG_TYPE_CAN_NOT_SET;

        case VAL_ipCidrRouteStatus_destroy:
            if(NETCFG_MGR_ROUTE_DeleteStaticIpCidrRouteEntry(ip_addr, ip_mask, next_hop,0, DEFAULT_METRIC) != NETCFG_TYPE_OK)
            {
                return NETCFG_TYPE_CAN_NOT_SET;
            }
            break;

        default:
            return NETCFG_TYPE_CAN_NOT_SET;
    }

    return NETCFG_TYPE_OK;

}


/* ROUTINE NAME: NETCFG_MGR_ROUTE_GetRowStatus
 *
 * FUNCTION:
 *      Get row status of route entry.
 *
 * INPUT:
 *      ip_addr : IP address of rif.
 *      ip_mask : Mask of rif.
 *      tos     : type of service. (must be 0).
 *      next_hop: NextHop of route entry.
 *
 * OUTPUT:
 *      row_status
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
 *      Key is (ip_addr, ip_mask, tos, next_hop).
 */
static UI32_T NETCFG_MGR_ROUTE_GetRowStatus(UI8_T ip_addr[SYS_ADPT_IPV4_ADDR_LEN], UI8_T ip_mask[SYS_ADPT_IPV4_ADDR_LEN], UI32_T tos, UI8_T next_hop[SYS_ADPT_IPV4_ADDR_LEN], UI32_T *row_status)
{
    NETCFG_TYPE_IpCidrRouteEntry_T   route_entry;
    ROUTE_MGR_IpCidrRouteEntry_T    entry;

    if(row_status == NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                 NETCFG_MGR_ROUTE_GET_ROW_STATUS_FUN_NO,
                                 EH_TYPE_MSG_INVALID_PARAMETER,
                                 SYSLOG_LEVEL_ERR,
                                 "row_status");
        return NETCFG_TYPE_INVALID_ARG;
    } /* end of if */

    if(((ip_addr == NULL) || NETCFG_MGR_ROUTE_IP_IS_ALL_ZEROS(ip_addr))
     ||((ip_mask == NULL) || NETCFG_MGR_ROUTE_IP_IS_ALL_ZEROS(ip_mask))
     ||((next_hop == NULL) || NETCFG_MGR_ROUTE_IP_IS_ALL_ZEROS(next_hop)))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                 NETCFG_MGR_ROUTE_GET_ROW_STATUS_FUN_NO,
                                 EH_TYPE_MSG_INVALID_PARAMETER,
                                 SYSLOG_LEVEL_ERR,
                                 "ip_addr or ip_mask or next hop");
        return NETCFG_TYPE_INVALID_ARG;
    } /* end of if */

    if(NETCFG_MGR_ROUTE_GetIpCidrRouteEntry(&route_entry) == NETCFG_TYPE_OK)
    {
        *row_status = VAL_ipCidrRouteStatus_active;
        return NETCFG_TYPE_OK;
    }

    memcpy(entry.ip_cidr_route_dest, ip_addr, sizeof(entry.ip_cidr_route_dest));
    memcpy(entry.ip_cidr_route_mask, ip_mask, sizeof(entry.ip_cidr_route_mask));
//    if(NETCFG_OM_ROUTE_GetRoute(&entry) == TRUE)
    {
        *row_status = VAL_ipCidrRouteStatus_notInService;
        return NETCFG_TYPE_OK;
    }

    EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                             NETCFG_MGR_ROUTE_GET_ROW_STATUS_FUN_NO,
                             EH_TYPE_MSG_FAILED_TO,
                             SYSLOG_LEVEL_ERR,
                             "NETCFG_OM_ROUTE_GetRoute");
    return NETCFG_TYPE_CAN_NOT_GET;
}
#endif


/* ROUTINE NAME: NETCFG_MGR_ROUTE_ValidateRouteEntry
 *
 * FUNCTION:
 *      Validates route and nexthop of the route entry.
 *
 * INPUT:
 *      entry->ip_cidr_route_next_hop
 *      entry->ip_cidr_route_dest
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Success: NETCFG_TYPE_OK
 *      Fail:    Value representing the fail type
 *
 * NOTES:
 *      None
 */
static UI32_T NETCFG_MGR_ROUTE_ValidateRouteEntry(ROUTE_MGR_IpCidrRouteEntry_T *entry)
{
    UI32_T ret;

    if (NULL == entry)
        return NETCFG_TYPE_INVALID_ARG;

    if(entry->ip_cidr_route_type == VAL_ipCidrRouteType_local)
        return NETCFG_TYPE_OK;

    if (entry->ip_cidr_route_next_hop.type == L_INET_ADDR_TYPE_IPV4 ||
        entry->ip_cidr_route_next_hop.type == L_INET_ADDR_TYPE_IPV4Z)
    {
        /* Validate destination IP address
         */
        if(((entry->ip_cidr_route_dest.preflen > 0)&&(IP_LIB_IsZeroNetwork(entry->ip_cidr_route_dest.addr) == TRUE))
            || (IP_LIB_IsIpInClassD(entry->ip_cidr_route_dest.addr) == TRUE)
            || (IP_LIB_IsIpInClassE(entry->ip_cidr_route_dest.addr) == TRUE)
            || (IP_LIB_IsLoopBackIp(entry->ip_cidr_route_dest.addr) == TRUE)
            || (IP_LIB_IsBroadcastIp(entry->ip_cidr_route_dest.addr) == TRUE)
            || (IP_LIB_IsMulticastIp(entry->ip_cidr_route_dest.addr) == TRUE)
            || (IP_LIB_IsTestingIp(entry->ip_cidr_route_dest.addr) == TRUE))
        {
            return NETCFG_TYPE_INVALID_ARG;
        }

        /* Validate nexthop IP address
         */
        if(IP_LIB_OK != IP_LIB_IsValidForRemoteIp(entry->ip_cidr_route_next_hop.addr))
        {
            return NETCFG_TYPE_INVALID_ARG;
        }
    }
    else if (entry->ip_cidr_route_next_hop.type == L_INET_ADDR_TYPE_IPV6 ||
             entry->ip_cidr_route_next_hop.type == L_INET_ADDR_TYPE_IPV6Z)
    {
        /* Check destination route
         */
        if (L_INET_ADDR_IS_IPV6_LINK_LOCAL(entry->ip_cidr_route_dest.addr))
        {
            return NETCFG_TYPE_INVALID_ROUTE_IPV6_LINKLOCAL;
        }

        if( (entry->ip_cidr_route_dest.preflen != 0) && /* default gateway no need to check the address */
            (IP_LIB_OK != (ret = IP_LIB_CheckIPv6PrefixForInterface(entry->ip_cidr_route_dest.addr, entry->ip_cidr_route_dest.preflen))))
        {
            switch(ret)
            {
                case IP_LIB_INVALID_IPV6_UNSPECIFIED:
                    return NETCFG_TYPE_INVALID_ROUTE_IPV6_UNSPECIFIED;
                case IP_LIB_INVALID_IPV6_LOOPBACK:
                    return NETCFG_TYPE_INVALID_ROUTE_IPV6_LOOPBACK;
                case IP_LIB_INVALID_IPV6_MULTICAST:
                    return NETCFG_TYPE_INVALID_ROUTE_IPV6_MULTICAST;
                default:
                    return NETCFG_TYPE_FAIL;
            }
        }

        /* Check nexthop
         */
        if(IP_LIB_OK != (ret = IP_LIB_CheckIPv6PrefixForInterface(entry->ip_cidr_route_next_hop.addr, entry->ip_cidr_route_next_hop.preflen)))
        {
            switch(ret)
            {
                case IP_LIB_INVALID_IPV6_UNSPECIFIED:
                    return NETCFG_TYPE_INVALID_NEXTHOP_IPV6_UNSPECIFIED;
                case IP_LIB_INVALID_IPV6_LOOPBACK:
                    return NETCFG_TYPE_INVALID_NEXTHOP_IPV6_LOOPBACK;
                case IP_LIB_INVALID_IPV6_MULTICAST:
                    return NETCFG_TYPE_INVALID_NEXTHOP_IPV6_MULTICAST;
                default:
                    return NETCFG_TYPE_FAIL;
            }
        }
    }

    return NETCFG_TYPE_OK;
}


static void NETCFG_MGR_ROUTE_DumpStaticRoute(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    ROUTE_MGR_IpCidrRouteEntry_T   entry;
    char           cp1[28], cp2[18];
    int             route_num = 0;
    BACKDOOR_MGR_Printf("     IP Address       Next  Hop  Metric Distance   Rowstatus Interface\n");

    memset(&entry, 0, sizeof(ROUTE_MGR_IpCidrRouteEntry_T));
     entry.ip_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV4;
     entry.ip_cidr_route_next_hop.type = L_INET_ADDR_TYPE_IPV4;

    L_THREADGRP_Execution_Request(tg_handle, member_id);
    while(NETCFG_OM_ROUTE_GetNextStaticIpCidrRoute(&entry) == NETCFG_TYPE_OK)
    {
        route_num++;

        sprintf(cp1, "%d.%d.%d.%d/%d", entry.ip_cidr_route_dest.addr[0], entry.ip_cidr_route_dest.addr[1], entry.ip_cidr_route_dest.addr[2], entry.ip_cidr_route_dest.addr[3], entry.ip_cidr_route_dest.preflen);
        sprintf(cp2, "%d.%d.%d.%d", entry.ip_cidr_route_next_hop.addr[0], entry.ip_cidr_route_next_hop.addr[1], entry.ip_cidr_route_next_hop.addr[2], entry.ip_cidr_route_next_hop.addr[3]);


        BACKDOOR_MGR_Printf("%15s %15s %7ld %8ld", cp1, cp2, (long)entry.ip_cidr_route_metric, (long)entry.ip_cidr_route_distance);
        switch(entry.ip_cidr_route_status)
        {
            case NETCFG_TYPE_StaticIpCidrEntryRowStatus_active:
                BACKDOOR_MGR_Printf("      Active");
                break;
            case NETCFG_TYPE_StaticIpCidrEntryRowStatus_notInService:
                BACKDOOR_MGR_Printf("NotInService");
                break;
            case NETCFG_TYPE_StaticIpCidrEntryRowStatus_notReady:
                BACKDOOR_MGR_Printf("    NotReady");
                break;
            default:
                break;
        }

        BACKDOOR_MGR_Printf("%10ld\n", (long)entry.ip_cidr_route_if_index);
    }
    L_THREADGRP_Execution_Release(tg_handle, member_id);
    BACKDOOR_MGR_Printf("There are %d Route Entries in Configuration\n", route_num);
}

#if (SYS_CPNT_IPV6 == TRUE)

static void NETCFG_MGR_ROUTE_DumpStaticIPv6Route(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    ROUTE_MGR_IpCidrRouteEntry_T   entry;
//    NETCFG_TYPE_IPv4RifConfig_T rif_config;
    char           cp1[50]={0}, cp2[50]={0};
    int            route_num = 0;

    BACKDOOR_MGR_Printf("     IP Address     Prefix Length  Next  Hop\t  Metric  Status  Interface\n");

    memset(&entry, 0, sizeof(ROUTE_MGR_IpCidrRouteEntry_T));
    entry.ip_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV6;
    entry.ip_cidr_route_next_hop.type = L_INET_ADDR_TYPE_IPV6;

    L_THREADGRP_Execution_Request(tg_handle, member_id);
    while(NETCFG_OM_ROUTE_GetNextStaticIpCidrRoute(&entry) == NETCFG_TYPE_OK)
    {
        route_num++;

        sprintf(cp1, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",entry.ip_cidr_route_dest.addr[0],entry.ip_cidr_route_dest.addr[1],entry.ip_cidr_route_dest.addr[2],entry.ip_cidr_route_dest.addr[3],entry.ip_cidr_route_dest.addr[4],entry.ip_cidr_route_dest.addr[5],entry.ip_cidr_route_dest.addr[6],entry.ip_cidr_route_dest.addr[7],entry.ip_cidr_route_dest.addr[8],entry.ip_cidr_route_dest.addr[9],entry.ip_cidr_route_dest.addr[10],entry.ip_cidr_route_dest.addr[11],entry.ip_cidr_route_dest.addr[12],entry.ip_cidr_route_dest.addr[13],entry.ip_cidr_route_dest.addr[14],entry.ip_cidr_route_dest.addr[15]);


        sprintf(cp2, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",entry.ip_cidr_route_next_hop.addr[0],entry.ip_cidr_route_next_hop.addr[1],entry.ip_cidr_route_next_hop.addr[2],entry.ip_cidr_route_next_hop.addr[3],entry.ip_cidr_route_next_hop.addr[4],entry.ip_cidr_route_next_hop.addr[5],entry.ip_cidr_route_next_hop.addr[6],entry.ip_cidr_route_next_hop.addr[7],entry.ip_cidr_route_next_hop.addr[8],entry.ip_cidr_route_next_hop.addr[9],entry.ip_cidr_route_next_hop.addr[10],entry.ip_cidr_route_next_hop.addr[11],entry.ip_cidr_route_next_hop.addr[12],entry.ip_cidr_route_next_hop.addr[13],entry.ip_cidr_route_next_hop.addr[14],entry.ip_cidr_route_next_hop.addr[15]);


        BACKDOOR_MGR_Printf("%40s ",cp1);
        BACKDOOR_MGR_Printf("%2d ",entry.ip_cidr_route_dest.preflen);
        BACKDOOR_MGR_Printf("%40s    ",cp2);
        BACKDOOR_MGR_Printf("%2ld ", (long)entry.ip_cidr_route_metric);

        if(entry.ip_cidr_route_status == NETCFG_TYPE_StaticIpCidrEntryRowStatus_active)
            BACKDOOR_MGR_Printf("     Valid   ");
        else
            BACKDOOR_MGR_Printf("     Invalid ");

// Terry Liu Begin: Temporarily remove
//        memcpy(rif_config.ip_addr, entry.ip_cidr_route_next_hop, sizeof(rif_config.ip_addr));
//
//        NETCFG_OM_IP_GetRifFromSubnet(&rif_config);
//
//        entry.ip_cidr_route_if_index = rif_config.ifindex;
// Terry Liu End

        BACKDOOR_MGR_Printf("%ld\n", (long)entry.ip_cidr_route_if_index);
    }
    L_THREADGRP_Execution_Release(tg_handle, member_id);
    BACKDOOR_MGR_Printf("There are %d Route Entries in Configuration\n", route_num);
}

#if (SYS_CPNT_NSM == TRUE)
void NETCFG_MGR_ROUTE_DumpZebOSIPv6Route(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{

    UI32_T ret;
    NSM_MGR_GetNextRouteEntry_T entry;
    UI16_T route_num;

    // entry.action_flags = GET_IPV6_ROUTE_FLAG;
    entry.data.next_route_node=NULL;
    entry.data.next_rib_node=NULL;
    entry.data.next_hop_node=NULL;

    route_num=0;

    L_THREADGRP_Execution_Request(tg_handle, member_id);

    BACKDOOR_MGR_Printf("        IP Address  Prefix Len         Next  Hop  Metric Distance\n");
    for(ret=NSM_PMGR_GetNextIpv6Route(&entry);TRUE;ret=NSM_PMGR_GetNextIpv6Route(&entry))
    {
        UI8_T idx;
        char cp1[50]={0}, cp2[50]={0};

        if( (ret==NSM_TYPE_RESULT_OK) || (ret==NSM_TYPE_RESULT_EOF) )
        {
            //sprintf(cp1, "%d.%d.%d.%d", entry.data.ip_route_dest[0], entry.data.ip_route_dest[1], entry.data.ip_route_dest[2], entry.data.ip_route_dest[3]);
        sprintf(cp1, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",entry.data.ip_route_dest.addr[0], entry.data.ip_route_dest.addr[1], entry.data.ip_route_dest.addr[2], entry.data.ip_route_dest.addr[3], entry.data.ip_route_dest.addr[4], entry.data.ip_route_dest.addr[5], entry.data.ip_route_dest.addr[6], entry.data.ip_route_dest.addr[7], entry.data.ip_route_dest.addr[8], entry.data.ip_route_dest.addr[9], entry.data.ip_route_dest.addr[10], entry.data.ip_route_dest.addr[11], entry.data.ip_route_dest.addr[12], entry.data.ip_route_dest.addr[13], entry.data.ip_route_dest.addr[14], entry.data.ip_route_dest.addr[15]);

            for(idx=0; idx<entry.data.num_of_next_hop; idx++)
            {
                //sprintf(cp2, "%d.%d.%d.%d", entry.data.ip_next_hop[idx][1], entry.data.ip_next_hop[idx][1], entry.data.ip_next_hop[idx][2], entry.data.ip_next_hop[idx][3]);
        sprintf(cp2, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",entry.data.ip_next_hop[idx].addr[0], entry.data.ip_next_hop[idx].addr[1], entry.data.ip_next_hop[idx].addr[2], entry.data.ip_next_hop[idx].addr[3], entry.data.ip_next_hop[idx].addr[4], entry.data.ip_next_hop[idx].addr[5], entry.data.ip_next_hop[idx].addr[6], entry.data.ip_next_hop[idx].addr[7], entry.data.ip_next_hop[idx].addr[8], entry.data.ip_next_hop[idx].addr[9], entry.data.ip_next_hop[idx].addr[10], entry.data.ip_next_hop[idx].addr[11], entry.data.ip_next_hop[idx].addr[12], entry.data.ip_next_hop[idx].addr[13], entry.data.ip_next_hop[idx].addr[14], entry.data.ip_next_hop[idx].addr[15]);

                BACKDOOR_MGR_Printf("%40s",cp1);
                BACKDOOR_MGR_Printf("%12d", entry.data.ip_route_dest_prefix_len);
                BACKDOOR_MGR_Printf("%40s", cp2);
                BACKDOOR_MGR_Printf("%8d%9d\n", entry.data.metric, entry.data.distance);

                route_num++;
            }
        }

        if(ret!=NSM_TYPE_RESULT_OK)
            break;
    }

    BACKDOOR_MGR_Printf("\nThere are %ld IPv6 routes in ZebOS\n", (long)route_num);
    if(ret!=NSM_TYPE_RESULT_EOF)
    {
        BACKDOOR_MGR_Printf("Abnormal return value: %d\n", ret);
    }

    L_THREADGRP_Execution_Release(tg_handle, member_id);

}
#endif /* #if (SYS_CPNT_NSM == TRUE) */

#endif /* #if (SYS_CPNT_IPV6 == TRUE) */


static int NETCFG_MGR_ROUTE_BACKDOOR_AHtoI(char *token)
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


static int NETCFG_MGR_ROUTE_BACKDOOR_AtoIPV6(UI8_T *s, UI8_T *ip)
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
                    NETCFG_MGR_ROUTE_BACKDOOR_AHtoI((char *)token) < 0 || NETCFG_MGR_ROUTE_BACKDOOR_AHtoI((char *)token) > 255) // Invalid Token
                    return 0;
                else if (k >= 16)  // Too many digits
                    return 0;
                else // token is ready
                {
                    temp[k++] =(UI8_T)NETCFG_MGR_ROUTE_BACKDOOR_AHtoI((char *)token);
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
            NETCFG_MGR_ROUTE_BACKDOOR_AHtoI((char *)token) < 0 || NETCFG_MGR_ROUTE_BACKDOOR_AHtoI((char *)token) > 255) // Invalid Token
            return 0;
        else if (k >= 16)  // Too many digits
            return 0;


        temp[k]=(UI8_T)NETCFG_MGR_ROUTE_BACKDOOR_AHtoI((char *)token);

        for(l=0;l<16;l++)
         ip[l] = temp[l];

        return 1;

}

static int NETCFG_MGR_ROUTE_BACKDOOR_GetIPV6(UI32_T * IPaddr)
{
    int ret = 0;
    UI8_T   *temp_ip = (UI8_T *)IPaddr;
    UI8_T   buffer[50] = {0};

    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 50);

    if(strlen((char *)buffer) != 39)
     ret = 0;
    else
     ret = NETCFG_MGR_ROUTE_BACKDOOR_AtoIPV6(buffer, temp_ip);

    if(ret == 0)
    {
        BACKDOOR_MGR_Printf("\nYou entered an invalid IPv6 address\n");
        return  ret;
    }

    return  1;
}


int NETCFG_MGR_ROUTE_TestAddStaticIPv6Route(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    UI8_T   buffer[20] = {0};
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    char    *terminal;
    UI32_T distance;
    L_INET_AddrIp_T dest, next_hop;
    BACKDOOR_MGR_Printf("\n\r Input dest host IPv6 address(x:x:x:x:x:x:x:x): ");
    if(!NETCFG_MGR_ROUTE_BACKDOOR_GetIPV6((UI32_T*)&(dest.addr)))
        return 1;

    BACKDOOR_MGR_Printf("\n\r Input prefix length(0~64): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    dest.preflen = (UI16_T)strtoul((char *)buffer, &terminal, 10);
    dest.type = L_INET_ADDR_TYPE_IPV6;
    BACKDOOR_MGR_Printf("\n\r Input next hop IPv6 address(x:x:x:x:x:x:x:x): ");
    if(!NETCFG_MGR_ROUTE_BACKDOOR_GetIPV6((UI32_T*)&(next_hop.addr)))
        return 1;
    next_hop.type = L_INET_ADDR_TYPE_IPV6;

    BACKDOOR_MGR_Printf("\n\r Input Distance: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    distance = (UI32_T)strtoul((char *)buffer, &terminal, 10);
    BACKDOOR_MGR_Printf("\n");

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, member_id);

    NETCFG_MGR_ROUTE_AddStaticIpCidrRouteEntry(fib_id, &dest, &next_hop,0, distance);
    /* Release execution permission from the thread group handler if necessary
     */

    L_THREADGRP_Execution_Release(tg_handle, member_id);

    return 0;
}

int NETCFG_MGR_ROUTE_TestDeleteStaticIPv6Route(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    UI8_T   buffer[20] = {0};
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    char    *terminal;
    L_INET_AddrIp_T dest, next_hop;

    BACKDOOR_MGR_Printf("\n\r Input dest host IPv6 address(x:x:x:x:x:x:x:x): ");
    if(!NETCFG_MGR_ROUTE_BACKDOOR_GetIPV6((UI32_T*)&(dest.addr)))
        return 1;

    BACKDOOR_MGR_Printf("\n\r Input prefix length(0~64): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    dest.preflen = (UI16_T)strtoul((char *)buffer, &terminal, 10);
    dest.type = L_INET_ADDR_TYPE_IPV6;
    BACKDOOR_MGR_Printf("\n\r Input next hop IPv6 address(x:x:x:x:x:x:x:x): ");
    if(!NETCFG_MGR_ROUTE_BACKDOOR_GetIPV6((UI32_T*)&(next_hop.addr)))
        return 1;
    next_hop.type = L_INET_ADDR_TYPE_IPV6;

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, member_id);

    NETCFG_MGR_ROUTE_DeleteStaticIpCidrRouteEntry(fib_id, &dest, &next_hop,0);
    /* Release execution permission from the thread group handler if necessary
     */

    L_THREADGRP_Execution_Release(tg_handle, member_id);

    return 0;
}

int NETCFG_MGR_ROUTE_TestAddDefaultIPv6Route(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    UI8_T   buffer[20] = {0};
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    char    *terminal;
    UI32_T distance;
    L_INET_AddrIp_T next_hop;
//    BACKDOOR_MGR_Printf("\n\r Input Gateway IPv6 address(x:x:x:x:x:x:x:x): ");
//    if(!NETCFG_MGR_ROUTE_BACKDOOR_GetIPV6((UI32_T*)&(dest.addr)))
//        return 1;
//
//    BACKDOOR_MGR_Printf("\n\r Input prefix length(0~64): ");
//    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
//    dest.preflen = (UI16_T)strtoul((char *)buffer, &terminal, 10);
//    dest.type = L_INET_ADDR_TYPE_IPV6;
    BACKDOOR_MGR_Printf("\n\r Input gateway IPv6 address(x:x:x:x:x:x:x:x): ");
    if(!NETCFG_MGR_ROUTE_BACKDOOR_GetIPV6((UI32_T*)&(next_hop.addr)))
        return 1;
    next_hop.type = L_INET_ADDR_TYPE_IPV6;

    BACKDOOR_MGR_Printf("\n\r Input Distance: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    distance = (UI32_T)strtoul((char *)buffer, &terminal, 10);
    BACKDOOR_MGR_Printf("\n");

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, member_id);

//    NETCFG_MGR_ROUTE_AddIPv6StaticIpCidrRouteEntry(action_flags, fib_id, dest.addr, dest.preflen, next_hop.addr, distance);
    NETCFG_MGR_ROUTE_AddDefaultGateway(fib_id, &next_hop, distance);

    /* Release execution permission from the thread group handler if necessary
     */

    L_THREADGRP_Execution_Release(tg_handle, member_id);

    return 0;
}

int NETCFG_MGR_ROUTE_TestDeleteDefaultIPv6Route(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    UI8_T   buffer[20] = {0};
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    char    *terminal;
    UI32_T distance;
    L_INET_AddrIp_T next_hop;
//    BACKDOOR_MGR_Printf("\n\r Input dest host IPv6 address(x:x:x:x:x:x:x:x): ");
//    if(!NETCFG_MGR_ROUTE_BACKDOOR_GetIPV6((UI32_T*)&(dest.addr)))
//        return 1;
//
//    BACKDOOR_MGR_Printf("\n\r Input prefix length(0~64): ");
//    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
//    dest.preflen = (UI16_T)strtoul((char *)buffer, &terminal, 10);
//    dest.type = L_INET_ADDR_TYPE_IPV6;
    BACKDOOR_MGR_Printf("\n\r Input next hop IPv6 address(x:x:x:x:x:x:x:x): ");
    if(!NETCFG_MGR_ROUTE_BACKDOOR_GetIPV6((UI32_T*)&(next_hop.addr)))
        return 1;
    next_hop.type = L_INET_ADDR_TYPE_IPV6;

    BACKDOOR_MGR_Printf("\n\r Input Distance: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    distance = (UI32_T)strtoul((char *)buffer, &terminal, 10);
    BACKDOOR_MGR_Printf("\n");

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, member_id);

//    NETCFG_MGR_ROUTE_DeleteIPv6StaticIpCidrRouteEntry(action_flags, fib_id, dest.addr, dest.preflen, next_hop.addr, distance);
    NETCFG_MGR_ROUTE_DeleteDefaultGateway(fib_id, &next_hop);

    /* Release execution permission from the thread group handler if necessary
     */

    L_THREADGRP_Execution_Release(tg_handle, member_id);

    return 0;
}

int NETCFG_MGR_ROUTE_TestGetDefaultIPv6Route(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    char    cp1[50]={0};
    L_INET_AddrIp_T next_hop;
//    BACKDOOR_MGR_Printf("\n\r Input dest host IPv6 address(x:x:x:x:x:x:x:x): ");
//    if(!NETCFG_MGR_ROUTE_BACKDOOR_GetIPV6((UI32_T*)&(dest.addr)))
//        return 1;
//
//    BACKDOOR_MGR_Printf("\n\r Input prefix length(0~64): ");
//    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
//    dest.preflen = (UI16_T)strtoul((char *)buffer, &terminal, 10);
//    dest.type = L_INET_ADDR_TYPE_IPV6;
//    BACKDOOR_MGR_Printf("\n\r Input next hop IPv6 address(x:x:x:x:x:x:x:x): ");
//    if(!NETCFG_MGR_ROUTE_BACKDOOR_GetIPV6((UI32_T*)&(next_hop.addr)))
//        return 1;
    next_hop.type = L_INET_ADDR_TYPE_IPV6;

//    BACKDOOR_MGR_Printf("\n\r Input Distance: ");
//    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
//    distance = (UI32_T)strtoul((char *)buffer, &terminal, 10);
//    BACKDOOR_MGR_Printf("\n");

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, member_id);
    NETCFG_MGR_ROUTE_GetDefaultGateway(fib_id, &next_hop);

//    NETCFG_MGR_ROUTE_GetBestIPv6RoutingInterface(dest.addr, next_hop.addr);
    /* Release execution permission from the thread group handler if necessary
     */

    L_THREADGRP_Execution_Release(tg_handle, member_id);

    sprintf(cp1, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",next_hop.addr[0],next_hop.addr[1],next_hop.addr[2],next_hop.addr[3],next_hop.addr[4],next_hop.addr[5],next_hop.addr[6],next_hop.addr[7],next_hop.addr[8],next_hop.addr[9],next_hop.addr[10],next_hop.addr[11],next_hop.addr[12],next_hop.addr[13],next_hop.addr[14],next_hop.addr[15]);


    BACKDOOR_MGR_Printf("Next Hop IPV6 address: %39s\n",cp1);


    return 0;

}

int NETCFG_MGR_ROUTE_TestIPv6RouteLookup(L_THREADGRP_Handle_T tg_handle, UI32_T member_id)
{
    char    cp1[50]={0};
    UI32_T  out_ifindex;
    L_INET_AddrIp_T dest, next_hop, src;
    BACKDOOR_MGR_Printf("\n\r Input dest host IPv6 address(x:x:x:x:x:x:x:x): ");
    if(!NETCFG_MGR_ROUTE_BACKDOOR_GetIPV6((UI32_T*)&(dest.addr)))
        return 1;

//    BACKDOOR_MGR_Printf("\n\r Input prefix length(0~64): ");
//    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
//    dest.preflen = (UI16_T)strtoul((char *)buffer, &terminal, 10);
    dest.type = L_INET_ADDR_TYPE_IPV6;
    dest.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
//    BACKDOOR_MGR_Printf("\n\r Input next hop IPv6 address(x:x:x:x:x:x:x:x): ");
//    if(!NETCFG_MGR_ROUTE_BACKDOOR_GetIPV6((UI32_T*)&(next_hop.addr)))
//        return 1;
    next_hop.type = L_INET_ADDR_TYPE_IPV6;
    next_hop.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
//    BACKDOOR_MGR_Printf("\n\r Input Distance: ");
//    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
//    distance = (UI32_T)strtoul((char *)buffer, &terminal, 10);
    BACKDOOR_MGR_Printf("\n");

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, member_id);
    IPAL_ROUTE_RouteLookup(&dest, &src, &next_hop, &out_ifindex);
    /* Release execution permission from the thread group handler if necessary
     */

    L_THREADGRP_Execution_Release(tg_handle, member_id);

    sprintf(cp1, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",next_hop.addr[0],next_hop.addr[1],next_hop.addr[2],next_hop.addr[3],next_hop.addr[4],next_hop.addr[5],next_hop.addr[6],next_hop.addr[7],next_hop.addr[8],next_hop.addr[9],next_hop.addr[10],next_hop.addr[11],next_hop.addr[12],next_hop.addr[13],next_hop.addr[14],next_hop.addr[15]);


    BACKDOOR_MGR_Printf("Next Hop IPV6 address: %39s\n",cp1);


    return 0;
}

static void NETCFG_MGR_ROUTE_BackDoorMain(void)
{
    int ch;
    BOOL_T eof = FALSE;
    char buf[64];
    char *terminal;
    L_THREADGRP_Handle_T tg_handle;
    UI32_T               backdoor_member_id;

    tg_handle      = NETCFG_PROC_COMM_GetNetcfgTGHandle();

    /* Join thread group
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY, &backdoor_member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    while (eof == FALSE)
    {
        BACKDOOR_MGR_Printf("\n");
        BACKDOOR_MGR_Printf("0.  Exit\n");
        BACKDOOR_MGR_Printf("1.  Dump Static Route in Configuration\n");
#if (SYS_CPNT_NSM == TRUE)
        BACKDOOR_MGR_Printf("2.  Dump ZebOS IPV4 Routing Table\n");
#endif
        BACKDOOR_MGR_Printf("3.  Add IPV6 Static Route\n");
        BACKDOOR_MGR_Printf("4.  Delete IPV6 Static Route\n");
        BACKDOOR_MGR_Printf("5.  Dump IPV6 Static Route in Configuration\n");
#if (SYS_CPNT_NSM == TRUE)
        BACKDOOR_MGR_Printf("6.  Dump ZebOS IPV6 Routing Table\n");
#endif
        BACKDOOR_MGR_Printf("7.  IPv6 Route Lookup\n");
        BACKDOOR_MGR_Printf("8.  Add IPV6 Default Route\n");
        BACKDOOR_MGR_Printf("9.  Delete IPV6 Default Route\n");
        BACKDOOR_MGR_Printf("10.  Show IPV6 Default Route\n");
        BACKDOOR_MGR_Printf("11. Debug flag (DEBUG)(%s)\r\n", (DEBUG_FLAG & DEBUG_FLAG_BIT_DEBUG)? "on":"off" );
        BACKDOOR_MGR_Printf("12. Debug flag (INFO)(%s)\r\n", (DEBUG_FLAG & DEBUG_FLAG_BIT_INFO)? "on":"off");
        BACKDOOR_MGR_Printf("13. Debug flag (NOTE)(%s)\r\n", (DEBUG_FLAG & DEBUG_FLAG_BIT_NOTE)? "on":"off");
        BACKDOOR_MGR_Printf("14. Test NETCFG_MGR_ROUTE_GetReversePathIpMac\n");
        BACKDOOR_MGR_Printf("    select =");

        BACKDOOR_MGR_RequestKeyIn(buf, 15);
        ch = (int) strtoul(buf, &terminal, 10);
        BACKDOOR_MGR_Printf ("\n");

        switch(ch)
        {
            case 0:
                eof = TRUE;
                break;
            case 1:
                NETCFG_MGR_ROUTE_DumpStaticRoute(tg_handle, backdoor_member_id);
                break;
#if (SYS_CPNT_NSM == TRUE)
            case 2:
                ROUTE_MGR_DumpZebOSRoute(tg_handle, backdoor_member_id);
                break;
#endif
#if (SYS_CPNT_IPV6 == TRUE)
            case 3:
        NETCFG_MGR_ROUTE_TestAddStaticIPv6Route(tg_handle, backdoor_member_id);
        break;
            case 4:
        NETCFG_MGR_ROUTE_TestDeleteStaticIPv6Route(tg_handle, backdoor_member_id);
        break;
            case 5:
                NETCFG_MGR_ROUTE_DumpStaticIPv6Route(tg_handle, backdoor_member_id);
                break;
#if (SYS_CPNT_NSM == TRUE)
            case 6:
                NETCFG_MGR_ROUTE_DumpZebOSIPv6Route(tg_handle, backdoor_member_id);
                break;
#endif
            case 7:
                NETCFG_MGR_ROUTE_TestIPv6RouteLookup(tg_handle, backdoor_member_id);
                break;
        case 8:
        NETCFG_MGR_ROUTE_TestAddDefaultIPv6Route(tg_handle, backdoor_member_id);
        break;
            case 9:
        NETCFG_MGR_ROUTE_TestDeleteDefaultIPv6Route(tg_handle, backdoor_member_id);
        break;
            case 10:
                NETCFG_MGR_ROUTE_TestGetDefaultIPv6Route(tg_handle, backdoor_member_id);
                break;
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */
        case 11:
                DEBUG_FLAG = DEBUG_FLAG ^ DEBUG_FLAG_BIT_DEBUG;
                break;
            case 12:
                DEBUG_FLAG = DEBUG_FLAG ^ DEBUG_FLAG_BIT_INFO;
                break;
            case 13:
                DEBUG_FLAG = DEBUG_FLAG ^ DEBUG_FLAG_BIT_NOTE;
                break;
            case 14:
            {
                L_INET_AddrIp_T target_addr, nexthop_addr;
                UI8_T nexthop_addr_mac[SYS_ADPT_MAC_ADDR_LEN] = {};
                BACKDOOR_MGR_Printf("Input dest address (v4/v6):");
                BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf) - 1);
                BACKDOOR_MGR_Printf("\r\n");
                memset(&target_addr, 0 , sizeof(L_INET_AddrIp_T));
                memset(&nexthop_addr, 0 , sizeof(L_INET_AddrIp_T));
                if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC,
                                                                   buf,
                                                                   (L_INET_Addr_T *)&target_addr,
                                                                   sizeof(target_addr)))
                {
                    BACKDOOR_MGR_Printf("Failed. Address format error.\r\n");
                    break;
                }
                if(NETCFG_TYPE_OK == NETCFG_MGR_ROUTE_GetReversePathIpMac(&target_addr, &nexthop_addr, nexthop_addr_mac))
                {
                    L_INET_InaddrToString((L_INET_Addr_T *) &nexthop_addr, buf, sizeof(buf));
                    BACKDOOR_MGR_Printf("nexthop_add addr: %s\r\n", buf);

                    BACKDOOR_MGR_Printf("\n\r mac-address: %02X-%02X-%02X-%02X-%02X-%02X", L_INET_EXPAND_MAC(nexthop_addr_mac));
                }
                else
                {
                    BACKDOOR_MGR_Printf("Failed.\r\n");

                }
                break;
            }
            default:
                ch = 0;
                break;
        }
    }

    L_THREADGRP_Leave(tg_handle, backdoor_member_id);
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME : NETCFG_MGR_ROUTE_SetTransitionMode
 * PURPOSE:
 *      Enter transition mode, releasing all allocateing resource in master mode.
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
 *      1. In Transition Mode, must make sure all messages in queue are read
 *         and dynamic allocated space is free, resource set to INIT state.
 *      2. All function requests and incoming messages should be dropped.
 */
void NETCFG_MGR_ROUTE_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
    return;
}


/* FUNCTION NAME : NETCFG_MGR_ROUTE_EnterTransitionMode
 * PURPOSE:
 *      Enter transition mode, releasing all allocateing resource in master mode.
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
 *      1. In Transition Mode, must make sure all messages in queue are read
 *         and dynamic allocated space is free, resource set to INIT state.
 *      2. All function requests and incoming messages should be dropped.
 */
void NETCFG_MGR_ROUTE_EnterTransitionMode (void)
{
    UI16_T action_flags = L_INET_ADDR_TYPE_IPV4;

    SYSFUN_ENTER_TRANSITION_MODE();

    NETCFG_MGR_ROUTE_DeleteAllStaticIpCidrRouteEntries(action_flags);
    action_flags = L_INET_ADDR_TYPE_IPV6;
    NETCFG_MGR_ROUTE_DeleteAllStaticIpCidrRouteEntries(action_flags);

}


/* FUNCTION NAME : NETCFG_MGR_ROUTE_EnterMasterMode
 * PURPOSE:
 *      Enter master mode.
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
void NETCFG_MGR_ROUTE_EnterMasterMode (void)
{
    UI32_T vr_id = 0;

    if (SYS_DFLT_IP_FORWARDING == VAL_ipForwarding_forwarding)
    {
         if(NETCFG_MGR_ROUTE_EnableIpForwarding(vr_id, L_INET_ADDR_TYPE_IPV4) == NETCFG_TYPE_FAIL)
         {
            printf("Enable IPv4 forwarding Failure\n");
            fflush(stdout);
         }
    }
    else if(SYS_DFLT_IP_FORWARDING == VAL_ipForwarding_notForwarding)
    {
         if(NETCFG_MGR_ROUTE_DisableIpForwarding(vr_id, L_INET_ADDR_TYPE_IPV4) == NETCFG_TYPE_FAIL)
         {
            printf("Disable IPv4 forwarding Failure\n");
            fflush(stdout);
         }
    }

#if (SYS_CPNT_IPV6 == TRUE)
    if (SYS_DFLT_IPV6_FORWARDING == VAL_ipv6IpForwarding_forwarding)
    {
         if(NETCFG_MGR_ROUTE_EnableIpForwarding(vr_id, L_INET_ADDR_TYPE_IPV6) == NETCFG_TYPE_FAIL)
         {
            printf("Enable IPv6 forwarding Failure\n");
            fflush(stdout);
         }
    }
    else if(SYS_DFLT_IPV6_FORWARDING == VAL_ipv6IpForwarding_notForwarding)
    {
         if(NETCFG_MGR_ROUTE_DisableIpForwarding(vr_id, L_INET_ADDR_TYPE_IPV6) == NETCFG_TYPE_FAIL)
         {
            printf("Disable IPv6 forwarding Failure\n");
            fflush(stdout);
         }
    }
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
    NETCFG_MGR_ROUTE_LocalSetEcmpBalanceMode(SYS_DFLT_ECMP_BALANCE_MODE, 0, 0);
#endif

    SYSFUN_ENTER_MASTER_MODE();
}


/* FUNCTION NAME : NETCFG_MGR_ROUTE_EnterSlaveMode
 * PURPOSE:
 *      Enter slave mode.
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
 *      1. In slave mode, just rejects function request and discard incoming message.
 */
void NETCFG_MGR_ROUTE_EnterSlaveMode (void)
{
    SYSFUN_ENTER_SLAVE_MODE();
}


/* FUNCTION NAME : NETCFG_MGR_ROUTE_Create_InterCSC_Relation
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
 */
void NETCFG_MGR_ROUTE_Create_InterCSC_Relation(void)
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("routecfg",
                                                      SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,
                                                      NETCFG_MGR_ROUTE_BackDoorMain);
}


 /* FUNCTION NAME : NETCFG_MGR_ROUTE_InitiateProcessResources
 * PURPOSE:
 *      Initialize NETCFG_MGR_ROUTE used system resource, eg. protection semaphore.
 *      Clear all working space.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  - Success
 *      FALSE - Fail
 */
BOOL_T NETCFG_MGR_ROUTE_InitiateProcessResources(void)
{

    NETCFG_OM_ROUTE_InitateProcessResources();
    ROUTE_MGR_InitiateProcessResources();
#if (SYS_CPNT_MLDSNP == TRUE)
    MLDSNP_PMGR_InitiateProcessResources();
#endif
    return TRUE;
}


/* FUNCTION NAME : NETCFG_MGR_ROUTE_HandleIPCReqMsg
 * PURPOSE:
 *    Handle the ipc request message for NETCFG_MGR_ROUTE.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *
 */
BOOL_T NETCFG_MGR_ROUTE_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    NETCFG_MGR_ROUTE_IPCMsg_T *netcfg_mgr_route_msg_p;
    BOOL_T need_respond=TRUE;

    if(ipcmsg_p==NULL)
        return FALSE;

    netcfg_mgr_route_msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)ipcmsg_p->msg_buf;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        netcfg_mgr_route_msg_p->type.result_bool = FALSE;
        ipcmsg_p->msg_size = sizeof(netcfg_mgr_route_msg_p->type);
        return TRUE;
    }

    switch(netcfg_mgr_route_msg_p->type.cmd)
    {
        case NETCFG_MGR_ROUTE_IPCCMD_FINDBESTROUTE:
            netcfg_mgr_route_msg_p->type.result_ui32 = ROUTE_MGR_FindBestRoute(
                &netcfg_mgr_route_msg_p->data.ipa1_ipa2_u32a3_u32a4.ip_a1,
                &netcfg_mgr_route_msg_p->data.ipa1_ipa2_u32a3_u32a4.ip_a2,
                &netcfg_mgr_route_msg_p->data.ipa1_ipa2_u32a3_u32a4.u32_a3,
                &netcfg_mgr_route_msg_p->data.ipa1_ipa2_u32a3_u32a4.u32_a4);
            ipcmsg_p->msg_size=NETCFG_MGR_ROUTE_GET_MSG_SIZE(ipa1_ipa2_u32a3_u32a4);
            need_respond = TRUE;
            break;

        /* Route Configuration */

        case NETCFG_MGR_ROUTE_IPCCMD_SETDHCPDEFAULTGATEWAY:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_SetDhcpDefaultGateway(
                &netcfg_mgr_route_msg_p->data.addr_ip_v);
            ipcmsg_p->msg_size = NETCFG_MGR_ROUTE_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case NETCFG_MGR_ROUTE_IPCCMD_ADDSTATICROUTE:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_AddStaticIpCidrRouteEntry(
        SYS_ADPT_DEFAULT_FIB,
                &netcfg_mgr_route_msg_p->data.ip_cidr_route_entry_v.route_dest,
                &netcfg_mgr_route_msg_p->data.ip_cidr_route_entry_v.route_next_hop,
                netcfg_mgr_route_msg_p->data.ip_cidr_route_entry_v.ip_cidr_route_if_index,
                netcfg_mgr_route_msg_p->data.ip_cidr_route_entry_v.ip_cidr_route_distance);
            ipcmsg_p->msg_size = NETCFG_MGR_ROUTE_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case NETCFG_MGR_ROUTE_IPCCMD_DELETESTATICROUTE:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_DeleteStaticIpCidrRouteEntry(
        SYS_ADPT_DEFAULT_FIB,
                &netcfg_mgr_route_msg_p->data.ip_cidr_route_entry_v.route_dest,
                &netcfg_mgr_route_msg_p->data.ip_cidr_route_entry_v.route_next_hop,
                netcfg_mgr_route_msg_p->data.ip_cidr_route_entry_v.ip_cidr_route_if_index);
            ipcmsg_p->msg_size = NETCFG_MGR_ROUTE_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;

        case NETCFG_MGR_ROUTE_IPCCMD_DELETESTATICROUTEBYNETWORK:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_DeleteStaticRouteByNetwork(
                netcfg_mgr_route_msg_p->data.ip_mask.ip,
                netcfg_mgr_route_msg_p->data.ip_mask.mask);
            ipcmsg_p->msg_size = NETCFG_MGR_ROUTE_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
#if 0
        case NETCFG_MGR_ROUTE_IPCCMD_SETIPCIDRROUTEROWSTATUS:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_SetIpCidrRouteRowStatus(
                netcfg_mgr_route_msg_p->data.ip_mask_tos_next_hop_u32a1.ip,
                netcfg_mgr_route_msg_p->data.ip_mask_tos_next_hop_u32a1.mask,
                netcfg_mgr_route_msg_p->data.ip_mask_tos_next_hop_u32a1.tos,
                netcfg_mgr_route_msg_p->data.ip_mask_tos_next_hop_u32a1.next_hop,
                netcfg_mgr_route_msg_p->data.ip_mask_tos_next_hop_u32a1.u32_a1);
            ipcmsg_p->msg_size = NETCFG_MGR_ROUTE_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case NETCFG_MGR_ROUTE_IPCCMD_GETIPCIDRROUTEROWSTATUS:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_GetIpCidrRouteRowStatus(
                netcfg_mgr_route_msg_p->data.ip_mask_tos_next_hop_u32a1.ip,
                netcfg_mgr_route_msg_p->data.ip_mask_tos_next_hop_u32a1.mask,
                netcfg_mgr_route_msg_p->data.ip_mask_tos_next_hop_u32a1.tos,
                netcfg_mgr_route_msg_p->data.ip_mask_tos_next_hop_u32a1.next_hop,
                &(netcfg_mgr_route_msg_p->data.ip_mask_tos_next_hop_u32a1.u32_a1));
            ipcmsg_p->msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_mask_tos_next_hop_u32a1);
            need_respond = TRUE;
            break;
        case NETCFG_MGR_ROUTE_IPCCMD_GETNEXTIPCIDRROUTEROWSTATUS:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_GetNextIpCidrRouteRowStatus(
                netcfg_mgr_route_msg_p->data.ip_mask_tos_next_hop_u32a1.ip,
                netcfg_mgr_route_msg_p->data.ip_mask_tos_next_hop_u32a1.mask,
                &(netcfg_mgr_route_msg_p->data.ip_mask_tos_next_hop_u32a1.tos),
                netcfg_mgr_route_msg_p->data.ip_mask_tos_next_hop_u32a1.next_hop,
                &(netcfg_mgr_route_msg_p->data.ip_mask_tos_next_hop_u32a1.u32_a1));
            ipcmsg_p->msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_mask_tos_next_hop_u32a1);
            need_respond = TRUE;
            break;
#endif
        case NETCFG_MGR_ROUTE_IPCCMD_ADDDEFAULTGATEWAY:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_AddDefaultGateway(
        SYS_ADPT_DEFAULT_FIB,
                &netcfg_mgr_route_msg_p->data.ip_cidr_route_entry_v.route_next_hop,
                netcfg_mgr_route_msg_p->data.ip_cidr_route_entry_v.ip_cidr_route_distance);
            ipcmsg_p->msg_size = NETCFG_MGR_ROUTE_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case NETCFG_MGR_ROUTE_IPCCMD_DELETEDEFAULTGATEWAY:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_DeleteDefaultGateway(
        SYS_ADPT_DEFAULT_FIB,
                &netcfg_mgr_route_msg_p->data.addr_ip_v);
            ipcmsg_p->msg_size = NETCFG_MGR_ROUTE_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case NETCFG_MGR_ROUTE_IPCCMD_DELETEDHCPDEFAULTGATEWAY:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_DeleteDhcpDefaultGateway(
        SYS_ADPT_DEFAULT_FIB,
                &netcfg_mgr_route_msg_p->data.addr_ip_v);
            ipcmsg_p->msg_size = NETCFG_MGR_ROUTE_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case NETCFG_MGR_ROUTE_IPCCMD_GETDEFAULTGATEWAY:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_GetDefaultGateway(
        SYS_ADPT_DEFAULT_FIB,
                &netcfg_mgr_route_msg_p->data.addr_ip_v);
            ipcmsg_p->msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(addr_ip_v);
            need_respond = TRUE;
            break;
        case NETCFG_MGR_ROUTE_IPCCMD_GETREVERSEPATHIPMAC:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_GetReversePathIpMac(
            &(netcfg_mgr_route_msg_p->data.arg_grp_addrx2_mac.addr_1),
            &(netcfg_mgr_route_msg_p->data.arg_grp_addrx2_mac.addr_2),
            netcfg_mgr_route_msg_p->data.arg_grp_addrx2_mac.mac);
            ipcmsg_p->msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(arg_grp_addrx2_mac);
            need_respond = TRUE;
            break;


#if 0
        case NETCFG_MGR_ROUTE_IPCCMD_DELETEDYNAMICROUTE:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_DeleteDynamicRoute(
                netcfg_mgr_route_msg_p->data.ip_mask.ip,
                netcfg_mgr_route_msg_p->data.ip_mask.mask);
            ipcmsg_p->msg_size = NETCFG_MGR_ROUTE_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case NETCFG_MGR_ROUTE_IPCCMD_DELETEALLROUTES:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_DeleteAllRoutes();
            ipcmsg_p->msg_size=NETCFG_MGR_ROUTE_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
#endif
        case NETCFG_MGR_ROUTE_IPCCMD_DELETEALLSTATICROUTES:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_DeleteAllStaticIpCidrRouteEntries(
        netcfg_mgr_route_msg_p->data.ui32_v);
            ipcmsg_p->msg_size=NETCFG_MGR_ROUTE_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
#if 0
        case NETCFG_MGR_ROUTE_IPCCMD_DELETEALLDYNAMICROUTES:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_DeleteAllDynamicRoutes();
            ipcmsg_p->msg_size=NETCFG_MGR_ROUTE_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case NETCFG_MGR_ROUTE_IPCCMD_GETIPCIDRROUTEENTRY:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_GetIpCidrRouteEntry(
                &(netcfg_mgr_route_msg_p->data.ip_cidr_route_entry_v));
            ipcmsg_p->msg_size=NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_cidr_route_entry_v);
            need_respond = TRUE;
            break;
        case NETCFG_MGR_ROUTE_IPCCMD_GETNEXTIPCIDRROUTEENTRY:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_GetNextIpCidrRouteEntry(
                &(netcfg_mgr_route_msg_p->data.ip_cidr_route_entry_v));
            ipcmsg_p->msg_size=NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_cidr_route_entry_v);
            need_respond = TRUE;
            break;
        case NETCFG_MGR_ROUTE_IPCCMD_GETALLTYPEIPCIDRROUTEENTRY:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_GetAllTypeIpCidrRouteEntry(
                &(netcfg_mgr_route_msg_p->data.ip_cidr_route_entry_v));
            ipcmsg_p->msg_size=NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_cidr_route_entry_v);
            need_respond = TRUE;
            break;
        case NETCFG_MGR_ROUTE_IPCCMD_GETNEXTALLTYPEIPCIDRROUTEENTRY:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_GetNextAllTypeIpCidrRouteEntry(
                &(netcfg_mgr_route_msg_p->data.ip_cidr_route_entry_v));
            ipcmsg_p->msg_size=NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_cidr_route_entry_v);
            need_respond = TRUE;
            break;
        case NETCFG_MGR_ROUTE_IPCCMD_GETNEXTSTATICIPCIDRROUTEENTRY:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_GetNextStaticIpCidrRouteEntry(
                &(netcfg_mgr_route_msg_p->data.ip_cidr_route_entry_v));
            ipcmsg_p->msg_size=NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_cidr_route_entry_v);
            need_respond = TRUE;
            break;
        case NETCFG_MGR_ROUTE_IPCCMD_GETIPCIDRROUTENUMBER:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_GetIpCidrRouteNumber(
                &(netcfg_mgr_route_msg_p->data.ui32_v));
            ipcmsg_p->msg_size=NETCFG_MGR_ROUTE_GET_MSG_SIZE(ui32_v);
            need_respond = TRUE;
            break;
        case NETCFG_MGR_ROUTE_IPCCMD_SETSTATICIPCIDRROUTEMETRICS:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_SetStaticIpCidrRouteMetrics(
                netcfg_mgr_route_msg_p->data.ip_mask_tos_next_hop_u32a1.ip,
                netcfg_mgr_route_msg_p->data.ip_mask_tos_next_hop_u32a1.mask,
                netcfg_mgr_route_msg_p->data.ip_mask_tos_next_hop_u32a1.tos,
                netcfg_mgr_route_msg_p->data.ip_mask_tos_next_hop_u32a1.next_hop,
                netcfg_mgr_route_msg_p->data.ip_mask_tos_next_hop_u32a1.u32_a1);
            ipcmsg_p->msg_size = NETCFG_MGR_ROUTE_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
#endif

        case NETCFG_MGR_ROUTE_IPCCMD_GETNEXTRUNNINGSTATICIPCIDRROUTEENTRY:
            netcfg_mgr_route_msg_p->type.result_running_cfg = NETCFG_MGR_ROUTE_GetNextRunningStaticIpCidrRouteEntry(
                &(netcfg_mgr_route_msg_p->data.ip_cidr_route_entry_v));
            ipcmsg_p->msg_size=NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_cidr_route_entry_v);
            need_respond = TRUE;
            break;

        case NETCFG_MGR_ROUTE_IPCCMD_GETRUNNINGDEFAULTGATEWAY:
            netcfg_mgr_route_msg_p->type.result_running_cfg = NETCFG_MGR_ROUTE_GetRunningDefaultGateway(
        SYS_ADPT_DEFAULT_FIB,
                &(netcfg_mgr_route_msg_p->data.addr_ip_v));
            ipcmsg_p->msg_size=NETCFG_MGR_ROUTE_GET_MSG_SIZE(addr_ip_v);
            need_respond = TRUE;
            break;

        case NETCFG_MGR_ROUTE_IPCCMD_ENABLEIPFORWARDING:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_EnableIpForwarding(
        netcfg_mgr_route_msg_p->data.ip_forwarding_v.vr_id,
                netcfg_mgr_route_msg_p->data.ip_forwarding_v.addr_type);
            ipcmsg_p->msg_size=NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_forwarding_v);
            need_respond = TRUE;
            break;

        case NETCFG_MGR_ROUTE_IPCCMD_DISABLEIPFORWARDING:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_DisableIpForwarding(
        netcfg_mgr_route_msg_p->data.ip_forwarding_v.vr_id,
                netcfg_mgr_route_msg_p->data.ip_forwarding_v.addr_type);
            ipcmsg_p->msg_size=NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_forwarding_v);
            need_respond = TRUE;
            break;

        case NETCFG_MGR_ROUTE_IPCCMD_GETIPFORWARDINGSTATUS:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_GetIpForwardingStatus(
            netcfg_mgr_route_msg_p->data.ip_forwarding_v.vr_id,
            netcfg_mgr_route_msg_p->data.ip_forwarding_v.addr_type,
            &(netcfg_mgr_route_msg_p->data.ip_forwarding_v.forward_status));
            ipcmsg_p->msg_size=NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_forwarding_v);
            need_respond = TRUE;
            break;
#if (SYS_CPNT_STATIC_ROUTE_AND_METER_WORKAROUND == TRUE)
        case NETCFG_MGR_ROUTE_IPCCMD_ENABLEIPSWROUTE:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_EnableSWRoute(
                netcfg_mgr_route_msg_p->data.ui32_v);
            ipcmsg_p->msg_size=NETCFG_MGR_ROUTE_GET_MSG_SIZE(ui32_v);
            need_respond = TRUE;
            break;
        case NETCFG_MGR_ROUTE_IPCCMD_GETIPSWROUTE:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_GetSWRoute(
                &netcfg_mgr_route_msg_p->data.ui32_v);
            ipcmsg_p->msg_size=NETCFG_MGR_ROUTE_GET_MSG_SIZE(ui32_v);
            need_respond = TRUE;
            break;
#endif /*#if (SYS_CPNT_STATIC_ROUTE_AND_METER_WORKAROUND == TRUE)*/
#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
        case NETCFG_MGR_ROUTE_IPCCMD_SETECMPBALANCEMODE:
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_MGR_ROUTE_SetEcmpBalanceMode(
                netcfg_mgr_route_msg_p->data.u32a1_u32a2.u32_a1,
                netcfg_mgr_route_msg_p->data.u32a1_u32a2.u32_a2);
            ipcmsg_p->msg_size=NETCFG_MGR_ROUTE_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
#endif /* #if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE) */
        default:
            SYSFUN_Debug_Printf("%s(): Invalid cmd(%d).\n", __FUNCTION__, (int)(ipcmsg_p->cmd));
            netcfg_mgr_route_msg_p->type.result_ui32 = NETCFG_TYPE_FAIL;
            ipcmsg_p->msg_size = NETCFG_MGR_ROUTE_MSGBUF_TYPE_SIZE;
    }

    return need_respond;
}

#if (SYS_CPNT_IP_TUNNEL == TRUE)

/*
 * ROUTINE NAME: NETCFG_MGR_ROUTE_AddStaticRouteToTunnelInterface
 *
 * FUNCTION: Add static route to tunnel interface when tunnel interface is active .
 *
 * INPUT:
 *      nhop_tunnel_ifindex    -- nexthop tunnel ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_CAN_NOT_ADD
 *
 * NOTES:
 *      Add configured static route to nsm and kernel when tunnel interface is active
 */

UI32_T NETCFG_MGR_ROUTE_AddStaticRouteToTunnelInterface(UI32_T nhop_tunnel_ifindex)
{
    ROUTE_MGR_IpCidrRouteEntry_T entry;

    memset(&entry, 0, sizeof(entry));
    entry.ip_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV6;
    while(NETCFG_TYPE_OK == NETCFG_OM_ROUTE_GetNextStaticIpCidrRoute(&entry))
    {

        if(entry.ip_cidr_route_if_index != nhop_tunnel_ifindex)
            continue;

        /* Add route entry to NSM */
        if(ROUTE_MGR_AddStaticIpCidrRoute(SYS_ADPT_DEFAULT_FIB,
                        &entry.ip_cidr_route_dest,
                        &entry.ip_cidr_route_next_hop,
                        entry.ip_cidr_route_if_index,
                        entry.ip_cidr_route_distance) != NETCFG_TYPE_OK)
        {
             return NETCFG_TYPE_CAN_NOT_ADD;
        }
    }

    return NETCFG_TYPE_OK;
}

/*
 * ROUTINE NAME: NETCFG_MGR_ROUTE_DeleteStaticRouteToTunnelInterface
 *
 * FUNCTION: Delete static route to tunnel interface when tunnel interface is inactive .
 *
 * INPUT:
 *      nhop_tunnel_ifindex    -- nexthop tunnel ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_CAN_NOT_DELETE
 *
 * NOTES:
 *      Delete configured static route to nsm and kernel when tunnel interface is inactive
 */

UI32_T NETCFG_MGR_ROUTE_DeleteStaticRouteToTunnelInterface(UI32_T nhop_tunnel_ifindex)
{
    ROUTE_MGR_IpCidrRouteEntry_T entry;

    memset(&entry, 0, sizeof(entry));
    entry.ip_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV6;
    while(NETCFG_TYPE_OK == NETCFG_OM_ROUTE_GetNextStaticIpCidrRoute(&entry))
    {

        if(entry.ip_cidr_route_if_index != nhop_tunnel_ifindex)
            continue;

        /* Add route entry to NSM */
        if (ROUTE_MGR_DeleteStaticIpCidrRoute(SYS_ADPT_DEFAULT_FIB,
                    &entry.ip_cidr_route_dest,
                    &entry.ip_cidr_route_next_hop,
                    entry.ip_cidr_route_if_index,
                    entry.ip_cidr_route_distance) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_CAN_NOT_DELETE;
        }
    }

    return NETCFG_TYPE_OK;
}
#endif

/* not used */
#if 0
#if (SYS_CPNT_IP_TUNNEL == TRUE)
/*
 * ROUTINE NAME: NETCFG_MGR_ROUTE_AddStaticTunnelRouteEntry
 *
 * FUNCTION: Add tunnel static route.
 *
 * INPUT:
 *      dest    - IP address of static route
 *      mask    - IP mask of static route
 *      next_hop- next hop
 *      distance- administrative distance (1 ~ 254)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_ADD -- Unknown condition
 *      NETCFG_TYPE_TABLE_FULL
 *
 * NOTES:
 *      Add static route to the engine.
 */
static UI32_T NETCFG_MGR_ROUTE_AddStaticTunnelRouteEntry(UI32_T fib_id, L_INET_AddrIp_T *ip_cidr_route_dest, UI32_T next_hop_ifindex , UI32_T ip_cidr_route_distance)
{
    UI32_T rc = NETCFG_TYPE_OK;
    UI32_T routecfg_static_route_number;
    UI32_T total_nexthops;
    ROUTE_MGR_IpCidrRouteEntry_T entry;
    /* BODY
     */
     //a typical route entry contains

    NETCFG_OM_ROUTE_GetStaticIpCidrRouteNumber(ip_cidr_route_dest->type, &routecfg_static_route_number);
    if(routecfg_static_route_number >= SYS_ADPT_MAX_NBR_OF_STATIC_ROUTE_ENTRY)
        return NETCFG_TYPE_TABLE_FULL;

    memset(&entry, 0, sizeof(ROUTE_MGR_IpCidrRouteEntry_T));
    memcpy(&entry.ip_cidr_route_dest, ip_cidr_route_dest, sizeof(entry.ip_cidr_route_dest));
    entry.ip_cidr_route_tos     = 0;
    entry.ip_cidr_route_if_index = next_hop_ifindex;
    entry.ip_cidr_route_distance  = ip_cidr_route_distance;


    /* If a same static route exist with different distance value, following
     * get operation will replace it by set the argument "nh_replace" to TRUE.
     */
    if (NETCFG_OM_ROUTE_GetSameRoute(&entry, TRUE) == NETCFG_TYPE_ENTRY_EXIST)
        return NETCFG_TYPE_OK;

    /* ES3628BT-FLF-ZZ-00603:
     * Add the validator for the total nexthop number of a route.
     */
    if (NETCFG_OM_ROUTE_GetAllNextHopNumberOfRoute(&entry, &total_nexthops) == FALSE)
        return NETCFG_TYPE_CAN_NOT_ADD;
    if (total_nexthops >= SYS_ADPT_MAX_NBR_OF_HOST_ROUTE)
        return NETCFG_TYPE_TABLE_FULL;

    /* Vai:
     * Add route entry to NSM */
    if(ROUTE_MGR_AddStaticIpTunnelRoute(fib_id,
                    &entry.ip_cidr_route_dest,
                                        next_hop_ifindex,
                                           entry.ip_cidr_route_distance) != NETCFG_TYPE_OK)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                 NETCFG_MGR_ROUTE_ADD_STATIC_IP_CIDR_ROUTE_ENTRY_FUN_NO,
                                 EH_TYPE_MSG_FAILED_TO,
                                 SYSLOG_LEVEL_ERR,
                                 "ROUTE_MGR_AddStaticIpCidrRouteEntry failed");
        return NETCFG_TYPE_CAN_NOT_ADD;
    }

    /* Vai:
     * Store to OM
     */
    entry.ip_cidr_route_status = NETCFG_TYPE_StaticIpCidrEntryRowStatus_active;

    rc = NETCFG_OM_ROUTE_AddStaticIpCidrRoute(&entry);
    if (rc == NETCFG_TYPE_CAN_NOT_ADD)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_ROUTECFG,
                                 NETCFG_MGR_ROUTE_ADD_STATIC_IP_CIDR_ROUTE_ENTRY_FUN_NO,
                                 EH_TYPE_MSG_FAILED_TO,
                                 SYSLOG_LEVEL_ERR,
                                 "NETCFG_MGR_ROUTE_AddStaticIpCidrRoute");
        return rc;
    }
}
#endif /*#if (SYS_CPNT_IP_TUNNEL == TRUE)*/
#endif

#if (SYS_CPNT_NSM != TRUE)
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ROUTE_SignalRifUp
 *-----------------------------------------------------------------------------
 * PURPOSE : handler function of RIF activate for NETCFG_MGR_ROUTE
 *
 * INPUT   : ipaddr_p  -- RIF's ip address
 *
 * OUTPUT  : None
 *
 * RETURN  : None
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
void NETCFG_MGR_ROUTE_SignalRifUp(L_INET_AddrIp_T* ipaddr_p)
{
    ROUTE_MGR_IpCidrRouteEntry_T entry;

    /* Check IPv4/IPv6 default gateway
     */
    memset(&entry, 0, sizeof(ROUTE_MGR_IpCidrRouteEntry_T));
    if (ipaddr_p->type == L_INET_ADDR_TYPE_IPV4 ||
        ipaddr_p->type == L_INET_ADDR_TYPE_IPV4Z)
        entry.ip_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV4;
    else if (ipaddr_p->type == L_INET_ADDR_TYPE_IPV6 ||
             ipaddr_p->type == L_INET_ADDR_TYPE_IPV6Z)
        entry.ip_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV6;
    else
        return;

    if (NETCFG_TYPE_OK == NETCFG_OM_ROUTE_GetNextStaticIpCidrRoute(&entry))
    {
        if (IP_LIB_IsIpBelongToSubnet(ipaddr_p->addr, ipaddr_p->preflen, entry.ip_cidr_route_next_hop.addr))
        {
            ROUTE_MGR_AddStaticIpCidrRoute(0,
                            &entry.ip_cidr_route_dest,
                            &entry.ip_cidr_route_next_hop,
                            entry.ip_cidr_route_if_index,
                            entry.ip_cidr_route_distance);
        }
    }
}

/* FUNCTION NAME : NETCFG_MGR_ROUTE_L3IfOperStatusChanged_CallBack
 * PURPOSE:
 *      Handle the callback message for L3 interface operation status change.
 *
 * INPUT:
 *      vid_ifindex -- interface index
 *      status : VAL_ifOperStatus_up, interface up.
 *               VAL_ifOperStatus_down, interface down.
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
void NETCFG_MGR_ROUTE_L3IfOperStatusChanged_CallBack(UI32_T ifindex, UI32_T oper_status)
{
    ROUTE_MGR_IpCidrRouteEntry_T entry;
    NETCFG_TYPE_InetRifConfig_T  rif_config;

    /* Check IPv4 default gateway
     */
    memset(&entry, 0, sizeof(ROUTE_MGR_IpCidrRouteEntry_T));
    entry.ip_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV4;

    if (NETCFG_TYPE_OK == NETCFG_OM_ROUTE_GetNextStaticIpCidrRoute(&entry))
    {
        memset(&rif_config, 0, sizeof(rif_config));
        memcpy(&(rif_config.addr), &entry.ip_cidr_route_next_hop, sizeof(L_INET_AddrIp_T));

        if(NETCFG_OM_IP_GetRifFromIp(&rif_config) == NETCFG_TYPE_OK)
        {
            if (ifindex == rif_config.ifindex)
            {
                if (oper_status == VAL_ifOperStatus_up)
                    ROUTE_MGR_AddStaticIpCidrRoute(0,
                                &entry.ip_cidr_route_dest,
                                &entry.ip_cidr_route_next_hop,
                                entry.ip_cidr_route_if_index,
                                entry.ip_cidr_route_distance);
                else
                    ROUTE_MGR_DeleteStaticIpCidrRoute(0,
                                &entry.ip_cidr_route_dest,
                                &entry.ip_cidr_route_next_hop,
                                entry.ip_cidr_route_if_index,
                                entry.ip_cidr_route_distance);
            }
        }
    }

    /* Check IPv6 default gateway
     */
    memset(&entry, 0, sizeof(ROUTE_MGR_IpCidrRouteEntry_T));
    entry.ip_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV6;

    if (NETCFG_TYPE_OK == NETCFG_OM_ROUTE_GetNextStaticIpCidrRoute(&entry))
    {
        memset(&rif_config, 0, sizeof(rif_config));
        memcpy(&(rif_config.addr), &entry.ip_cidr_route_next_hop, sizeof(L_INET_AddrIp_T));

        if(NETCFG_OM_IP_GetRifFromIp(&rif_config) == NETCFG_TYPE_OK)
        {
            if (ifindex == rif_config.ifindex)
            {
                if (oper_status == VAL_ifOperStatus_up)
                    ROUTE_MGR_AddStaticIpCidrRoute(0,
                                &entry.ip_cidr_route_dest,
                                &entry.ip_cidr_route_next_hop,
                                entry.ip_cidr_route_if_index,
                                entry.ip_cidr_route_distance);
                else
                    ROUTE_MGR_DeleteStaticIpCidrRoute(0,
                                &entry.ip_cidr_route_dest,
                                &entry.ip_cidr_route_next_hop,
                                entry.ip_cidr_route_if_index,
                                entry.ip_cidr_route_distance);
            }
        }
    }
}
#endif

#if (SYS_CPNT_VRRP == TRUE)
/* FUNCTION NAME : NETCFG_MGR_ROUTE_SetStaticRouteWithNexthopVirtualIp
 * PURPOSE:
 *      Set staitc routes with nexthop is virtual ip to NSM
 *
 * INPUT:
 *      virtual_ip      --  VRRP virtual IP address
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
void NETCFG_MGR_ROUTE_SetStaticRouteWithNexthopVirtualIp(L_INET_AddrIp_T *virtual_ip)
{
    ROUTE_MGR_IpCidrRouteEntry_T entry;

    if(!virtual_ip)
        return;

    /* Check IPv4 default gateway
     */
    memset(&entry, 0, sizeof(entry));
    if(virtual_ip->type == L_INET_ADDR_TYPE_IPV4)
        entry.ip_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV4;
    else
        return;

    while(NETCFG_TYPE_OK == NETCFG_OM_ROUTE_GetNextStaticIpCidrRoute(&entry))
    {
        if(!memcmp(virtual_ip->addr, entry.ip_cidr_route_next_hop.addr, SYS_ADPT_IPV4_ADDR_LEN))
        {
            NETCFG_TYPE_InetRifConfig_T  rif_config;

            memset(&rif_config, 0, sizeof(rif_config));
            memcpy(&(rif_config.addr), &entry.ip_cidr_route_next_hop, sizeof(L_INET_AddrIp_T));
            if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetRifFromIp(&rif_config))
            {
                NETCFG_TYPE_L3_Interface_T intf;

                memset(&intf, 0, sizeof(intf));
                intf.ifindex = rif_config.ifindex;
                if((NETCFG_TYPE_OK == NETCFG_OM_IP_GetL3Interface(&intf)) &&
                   (intf.u.physical_intf.if_flags & IFF_UP))
                {
                    ROUTE_MGR_AddStaticIpCidrRoute(0,
                        &entry.ip_cidr_route_dest,
                        &entry.ip_cidr_route_next_hop,
                        entry.ip_cidr_route_if_index,
                        entry.ip_cidr_route_distance);
                }
            }
        }
    }
}

/* FUNCTION NAME : NETCFG_MGR_ROUTE_UnsetStaticRouteWithNexthopVirtualIp
 * PURPOSE:
 *      Unset staitc routes with nexthop is virtual ip from NSM
 *
 * INPUT:
 *      virtual_ip      --  VRRP virtual IP address
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
void NETCFG_MGR_ROUTE_UnsetStaticRouteWithNexthopVirtualIp(L_INET_AddrIp_T *virtual_ip)
{
    ROUTE_MGR_IpCidrRouteEntry_T entry;

    if(!virtual_ip)
        return;

    /* Check IPv4 default gateway
     */
    memset(&entry, 0, sizeof(ROUTE_MGR_IpCidrRouteEntry_T));
    if(virtual_ip->type == L_INET_ADDR_TYPE_IPV4)
        entry.ip_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV4;
    else
        return;

    while(NETCFG_TYPE_OK == NETCFG_OM_ROUTE_GetNextStaticIpCidrRoute(&entry))
    {
        if(!memcmp(virtual_ip->addr, entry.ip_cidr_route_next_hop.addr, SYS_ADPT_IPV4_ADDR_LEN))
        {
            ROUTE_MGR_DeleteStaticIpCidrRoute(0,
                &entry.ip_cidr_route_dest,
                &entry.ip_cidr_route_next_hop,
                entry.ip_cidr_route_if_index,
                entry.ip_cidr_route_distance);
        }
    }
}
#endif

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ROUTE_LocalSetEcmpBalanceMode
 *-----------------------------------------------------------------------------
 * PURPOSE : To set ecmp load balance mode to driver.
 * INPUT   : mode   - which mode to set
 *           new_id - new hsl_id if new mode is NETCFG_TYPE_ECMP_HASH_SELECTION mode
 *           old_id - old hsl_id if old mode is NETCFG_TYPE_ECMP_HASH_SELECTION mode
 * OUTPUT  : None
 * RETURN  : NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 * NOTES   : None
 *-----------------------------------------------------------------------------
 */
static UI32_T NETCFG_MGR_ROUTE_LocalSetEcmpBalanceMode(
    UI32_T mode, UI32_T new_id, UI32_T old_id)
{
    UI32_T  ret = NETCFG_TYPE_OK;
    UI32_T  map2drvl3[] = {
        [NETCFG_TYPE_ECMP_DIP_L4_PORT] = SWDRVL3_ECMP_DST_IP | SWDRVL3_ECMP_L4_PORT,
    };

    return NETCFG_TYPE_OK;

#if (SYS_CPNT_HASH_SELECTION == TRUE)
    /* 1. unbind old hash selection id */
    if (old_id != 0)
    {
        if (FALSE == SWCTRL_PMGR_UnBindHashSelForService(
                        SWCTRL_OM_HASH_SEL_SERVICE_ECMP, old_id))
        {
            DBGprintf("unbind hash selection id(%ld) failed", (long)old_id);
            ret = NETCFG_TYPE_FAIL;
        }
    }
#endif

    if (NETCFG_TYPE_OK == ret)
    {
        if (NETCFG_TYPE_ECMP_HASH_SELECTION == mode)
        {
#if (SYS_CPNT_HASH_SELECTION == TRUE)
            /* 2.a bind new hash selection id */
            if (FALSE == SWCTRL_PMGR_BindHashSelForService(
                            SWCTRL_OM_HASH_SEL_SERVICE_ECMP, new_id))
#endif
            {
                DBGprintf("bind new hash selection id(%ld) failed", (long)new_id);
                ret = NETCFG_TYPE_FAIL;
            }
        }
        else
        {
            /* 2.b set to new mode */
            if (FALSE == SWDRVL3_SetEcmpBalanceMode(map2drvl3[mode]))
            {
                DBGprintf("set balance mode(%ld) failed", (long)mode);
                ret = NETCFG_TYPE_FAIL;
            }
        }

        if (NETCFG_TYPE_FAIL == ret)
        {
#if (SYS_CPNT_HASH_SELECTION == TRUE)
            /* 3. roll back */
            if (old_id != 0)
            {
                SWCTRL_PMGR_BindHashSelForService(
                    SWCTRL_OM_HASH_SEL_SERVICE_ECMP, old_id);
            }
#endif
        }
    }

    return ret;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ROUTE_SetEcmpBalanceMode
 *-----------------------------------------------------------------------------
 * PURPOSE : To set ecmp load balance mode.
 * INPUT   : mode - which mode to set
 *           idx  - which idx to set if mode is NETCFG_TYPE_ECMP_HASH_SELECTION
 * OUTPUT  : None
 * RETURN  : NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 * NOTES   : None
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ROUTE_SetEcmpBalanceMode(UI32_T mode, UI32_T idx)
{
    UI32_T  ret = NETCFG_TYPE_OK, old_mode, old_hsl_id;

    switch (mode)
    {
    case NETCFG_TYPE_ECMP_HASH_SELECTION:
#if (SYS_CPNT_HASH_SELECTION == TRUE)
        if ((SYS_ADPT_MAX_HASH_SELECTION_BLOCK_SIZE < idx) || (idx == 0))
#endif
        {
            return NETCFG_TYPE_FAIL;
        }
        break;

    default:
        if (mode < NETCFG_TYPE_ECMP_MAX)
            break;

        return NETCFG_TYPE_FAIL;
    }

    NETCFG_OM_ROUTE_GetEcmpBalanceMode(&old_mode, &old_hsl_id);

    if (  (old_mode != mode)
        ||((NETCFG_TYPE_ECMP_HASH_SELECTION == mode) && (idx != old_hsl_id))
       )
    {
        ret = NETCFG_MGR_ROUTE_LocalSetEcmpBalanceMode(mode, idx, old_hsl_id);

        if (NETCFG_TYPE_OK == ret)
            NETCFG_OM_ROUTE_SetEcmpBalanceMode(mode, idx);
    }

    return ret;
}
#endif /* #if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE) */

