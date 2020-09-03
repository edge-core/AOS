/*******************************************************************
 *
 *    DESCRIPTION:
 *       Layer 3 Driver Layer API Specifications
 *
 *    AUTHOR:
 *
 *    HISTORY:
 *
 *   By              Date     Ver.   Modification Description
 *   --------------- -------- -----  ---------------------------------------
 *   Anderson                        Created
 *   Ted           01/24/2002        1. Clarify the API
 *                                   2. Change the IPMC related APIs
 *   Ted           07/09/2002        Fixed DEV_SWDRVL3_DeleteHostRoute() input parameter "dst_vid" data type
 *   Jason         03/19/2003        1. Add one more argument default_gateway_ip 
 *                                      in DEV_SWDRVL3_SetDefaultRoute() and DEV_SWDRVL3_AddNetRoute()
 *                                   2. Change argument port_member_tagged_list type from UI32_T to UI8_T of
 *                                      DEV_SWDRVL3_SetIpMcastRoute()
 *   Aaron         05/20/2003        1. Remove one more argument default_gateway_ip 
 *                                      in DEV_SWDRVL3_SetDefaultRoute().
 *   Garfield      06/12/2003        1. Remove one more argument default_gateway_ip 
 *                                      in DEV_SWDRVL3_AddNetRoute().
 *   Garfield      05/24/2004        1. Add one new API DEV_SWDRVL3_AddNetRouteWithNextHopIp()
 *   Garfield      06/17/2004        1. Add one new API DEV_SWDRVL3_SetDefaultRouteWithNextHopIp()
 *   Vai           05/21/2008        1. Change the L3Unicast related APIs
 *******************************************************************
 */

#ifndef DEV_SWDRVL3_H
#define DEV_SWDRVL3_H



/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "l_inet.h"
/* NAMING CONSTANT DECLARATIONS
 */
/* 1-30-2002
*/
#define DEV_SWDRVL3_ACTION_TRAP2CPU         SYS_CPNT_DEFAULT_ROUTE_ACTION_TRAP2CPU
#define DEV_SWDRVL3_ACTION_ROUTE            SYS_CPNT_DEFAULT_ROUTE_ACTION_ROUTE
#define DEV_SWDRVL3_ACTION_DROP             SYS_CPNT_DEFAULT_ROUTE_ACTION_DROP

#define DEV_SWDRVL3_L3_NO_ERROR             0
#define DEV_SWDRVL3_L3_BUCKET_FULL          1
#define DEV_SWDRVL3_L3_ECMP_BUCKET_FULL     2
#define DEV_SWDRVL3_L3_INVALID_ARG          3
#define DEV_SWDRVL3_L3_EXISTS               4
#define DEV_SWDRVL3_L3_NOT_EXIST            5
#define DEV_SWDRVL3_L3_FAIL                 6
#define DEV_SWDRVL3_L3_OTHERS               7
#define DEV_SWDRVL3_L3_MAC_COLLISION        8

#define DEV_SWDRVL3_HW_INFO_INVALID         (0xFFFFFFFF)

#define DEV_SWDRVL3_TUNNELTYPE_MANUAL	    1
#define DEV_SWDRVL3_TUNNELTYPE_6TO4	    2
#define DEV_SWDRVL3_TUNNELTYPE_ISATAP	    3


/*
 * Flags definition
 */
#define DEV_SWDRVL3_FLAG_IPV4                     1
#define DEV_SWDRVL3_FLAG_IPV6                     (1 << 1)
#define DEV_SWDRVL3_FLAG_ECMP                     (1 << 2)
#define DEV_SWDRVL3_FLAG_STATIC                   (1 << 3)
#define DEV_SWDRVL3_FLAG_TRUNK_EGRESS_PORT        (1 << 4)
#define DEV_SWDRVL3_FLAG_TAGGED_EGRESS_VLAN       (1 << 5)
#define DEV_SWDRVL3_FLAG_LOCAL_CONNECTED_ROUTE    (1 << 6)
#define DEV_SWDRVL3_FLAG_ECMP_ONE_PATH            (1 << 7)
#define DEV_SWDRVL3_FLAG_ECMP_ONE_PATH_ALONE      (1 << 8)
#define DEV_SWDRVL3_FLAG_PROCESS_L3_EGRESS_ONLY   (1 << 9)
#define DEV_SWDRVL3_FLAG_DYNAMIC_TUNNEL           (1 <<10)
#define DEV_SWDRVL3_FLAG_STATIC_TUNNEL            (1 <<11)
/* DATA TYPE DECLARATIONS
 */
typedef struct DEV_SWDRVL3_Host_S
{
    UI32_T flags;
    UI32_T fib_id;
    union{
        UI32_T ipv4_addr;
        UI8_T  ipv6_addr[SYS_ADPT_IPV6_ADDR_LEN];
    }ip_addr;
    /* The MAC address of the host */
    UI8_T  mac[SYS_ADPT_MAC_ADDR_LEN];
    /* The MAC of the router */
    UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T vid;
    UI32_T unit;
    UI32_T port;
    UI32_T trunk_id;
    void*  hw_info;
}DEV_SWDRVL3_Host_T;



typedef struct DEV_SWDRVL3_Route_S
{
    UI32_T flags;
    UI32_T fib_id;
    union{
        UI32_T ipv4_addr;
        UI8_T  ipv6_addr[SYS_ADPT_IPV6_ADDR_LEN];
    }dst_ip;
    UI32_T prefix_length;
    union{
        UI32_T ipv4_addr;
        UI8_T  ipv6_addr[SYS_ADPT_IPV6_ADDR_LEN];
    }next_hop_ip;
    /* The MAC address of the router */
    UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];
    /* The MAC address of the target next hop */
    UI8_T  nexthop_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T dst_vid;
    UI32_T unit;
    UI32_T port;
    UI32_T trunk_id;
    void*  hw_info;
}DEV_SWDRVL3_Route_T;

typedef struct DEV_SWDRVL3_TunnelInitiator_S
{
    UI32_T l3_intf_id;	    /* L3 Interface Id associated with this tunnel */
    UI32_T vid;		    // Vlan ID of the L3 interface with the tunnel attached
    UI8_T src_mac[SYS_ADPT_MAC_ADDR_LEN]; // MAC address of the L3 interface with the tunnel attached
    UI8_T tunnel_type;	    /* Tunnel Type (Manual, 6to4, ISATAP) */
    UI8_T ttl;		    // Tunnel Header TTL
    UI8_T nexthop_mac[SYS_ADPT_MAC_ADDR_LEN]; // Next Hop's MAC address
    L_INET_AddrIp_T sip;    // Tunnel source IPv4 address
    L_INET_AddrIp_T dip;    // Tunnel destination IPv4 address
}DEV_SWDRVL3_TunnelInitiator_T;

typedef struct DEV_SWDRVL3_TunnelTerminator_S
{
    I8_T fib_id;	    // FIB ID
    UI8_T tunnel_type;	    // Tunnel Type (Manual, 6to4, ISATAP)
    UI8_T lport[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]; //portmap for this tunel
    L_INET_AddrIp_T sip;    // Tunnel source IPv4 address with masklen
    L_INET_AddrIp_T dip;    // Tunnel destination IPv4 address with masklen
}DEV_SWDRVL3_TunnelTerminator_T;

typedef struct DEV_SWDRVL3_HostTunnel_S
{
    UI32_T flags;
    UI32_T fib_id;
    UI32_T vid;
    UI32_T unit;
    UI32_T port;
    UI32_T trunk_id;
    void*  hw_info;
    union{
        UI32_T ipv4_addr;
        UI8_T  ipv6_addr[SYS_ADPT_IPV6_ADDR_LEN];
    }ip_addr;
    /* The MAC address of the host */
    UI8_T  mac[SYS_ADPT_MAC_ADDR_LEN];
    /* The MAC of the router */
    UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];
    DEV_SWDRVL3_TunnelInitiator_T tnl_init;
    DEV_SWDRVL3_TunnelTerminator_T tnl_term;
}DEV_SWDRVL3_HostTunnel_T;

typedef struct DEV_SWDRVL3_TunnelIntfL3_S
{
    UI32_T l3_intf_id;	                    /* L3 Interface Id associated with this tunnel */
    UI16_T vid;		                        // Vlan ID of the L3 interface with the tunnel attached
    UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];  // MAC address of the L3 interface with the tunnel attached
}DEV_SWDRVL3_TunnelIntfL3_T;


#define SWDRV_MC_DESC_MAX  (32)

#define SWDRV_MC_DBG_ERR  (1<<0)
#define SWDRV_MC_DBG_MSG  (1<<1)
#define SWDRV_MC_DBG_DRV  (1<<2)

#define SWDRV_MC_FN_DESC(FN_DESC) (# FN_DESC == "" ? __FUNCTION__ : # FN_DESC)

#define SWDRV_MC_FN_ENTER(...)  

#define SWDRV_MC_EXIT(...)  do{\
  return(__VA_ARGS__);\
}while(0)

#define SWDRV_MC_DEBUG(SUBMOD, FMT, ...)     \
do \
{  \
    if ((swdrv_mc_dbg) & (SWDRV_MC_DBG_ ## SUBMOD))   \
    {  \
        printf ("[SWDRV_MC_" # SUBMOD "] " "%s[%d]%s" FMT"\n",__FUNCTION__, __LINE__, ": " , ##__VA_ARGS__); \
    }  \
}while (0)

#define SWDRV_MC_DEBUG_ADDR(SUBMOD, __group_addr, __src_addr, FMT, ...)     \
{ \
    if ((swdrv_mc_dbg) & (SWDRV_MC_DBG_ ## SUBMOD))   \
    {  \
        if(__group_addr[0]==0xff) \
        { \
            char   ipv6_addr_str[L_INET_MAX_IP6ADDR_STR_LEN+1]={0};        \
            printf("[SWDRV_MC_" # SUBMOD "] " "%s[%d]%s" FMT, __FUNCTION__, __LINE__, ":", ##__VA_ARGS__); \
            L_INET_Ntop(L_INET_AF_INET6, (void *)__group_addr, ipv6_addr_str, sizeof(ipv6_addr_str)); \
            printf("grp %s", ipv6_addr_str); \
            L_INET_Ntop(L_INET_AF_INET6, (void *)__src_addr, ipv6_addr_str, sizeof(ipv6_addr_str)); \
            printf("src %s\r\n", ipv6_addr_str); \
        } \
        else \
        { \
            char   mc_addr_str[L_INET_MAX_IP6ADDR_STR_LEN+1]={0}, src_addr_str[L_INET_MAX_IP6ADDR_STR_LEN+1]={0};        \
            L_INET_Ntop(L_INET_AF_INET, (void *)__group_addr, mc_addr_str, sizeof(mc_addr_str)); \
            L_INET_Ntop(L_INET_AF_INET, (void *)__src_addr, src_addr_str, sizeof(src_addr_str)); \
            printf("[SWDRV_MC_" # SUBMOD "] " "%s[%d]%s, grp %s, src %s," FMT "\r\n", __FUNCTION__, __LINE__, ":", mc_addr_str, src_addr_str, ##__VA_ARGS__); \
        } \
    } \
}

#if (SYS_CPNT_STACKING == TRUE)
#define SWDRV_MC_IFINDEX_2_UNIT(ifindex)    (((UI32_T)(((ifindex)-1)/SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))+1)
#define SWDRV_MC_IFINDEX_2_PORT(unit, ifindex)    ((ifindex) - ((unit)-1)*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
#else
#define SWDRV_MC_IFINDEX_2_UNIT(ifindex)    (swdrv_mc_unit)
#define SWDRV_MC_IFINDEX_2_PORT(unit, ifindex)    (ifindex)
#endif

#define SWDRV_MC_PORT_INCHAR_MAX  SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST
#define SWDRV_MC_PORT_MAX               SYS_ADPT_TOTAL_NBR_OF_LPORT

#define SWDRV_MC_UNIT_PORT_MIN(unit)  ((((unit)-1)*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT) + 1)
#define SWDRV_MC_UNIT_PORT_MAX(unit) ((unit)*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)

#define SWDRV_MC_PBMP_SET(unit, pbmp, port)   do{\
UI32_T mod_id, dev_id, phy_port, uport;\
uport = SWDRV_MC_IFINDEX_2_PORT(unit, port);\
if (!DEV_SWDRV_Logical2PhyDevicePortID((unit), uport, &mod_id, &dev_id, &phy_port))\
{\
  SWDRV_MC_DEBUG(ERR, "fail to get physical port, unit %lu, uport %lu, mod %lu, dev %lu, port %lu", \
                                     (unit), uport, mod_id, dev_id, phy_port);\
  ret = BCM_E_PARAM; \
  goto EXIT;\
} \
BCM_PBMP_PORT_ADD((pbmp), phy_port);\
}while(0)

#define SWDRV_MC_VIF_MAX  (SYS_ADPT_MAX_NBR_OF_IP_MULTICAST_VIFS)/*(32)*/

#define SWDRV_MC_GET_VIFINDEX(vid)  ((vid) +  SYS_ADPT_VLAN_1_IF_INDEX_NUMBER - 1)

/*down interface for ipmc entry*/
typedef struct swdrv_dnintf_s
{
    UI32_T l3vid;
    UI8_T l3port[SWDRV_MC_PORT_INCHAR_MAX];
}swdrv_dnintf_t;

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
/*bitmap*/
typedef enum
{
   DEV_SWDRVL3_ECMP_DST_IP   = 0x0001,
   DEV_SWDRVL3_ECMP_L4_PORT  = 0x0002,
} DEV_SWDRVL3_Ecmp_Mode_T;
#endif

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*******************************************************************************
 * DEV_SWDRVL3_EnableUnicastStormProtect
 *
 * Purpose: to prevent the unicast storm to CPU
 * Inputs: None
 * Outputs: None
 * Return: TRUE/FALSE
 * Note :  (unit=0, port=0) means per system protect
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_EnableUnicastStormProtect(UI32_T unit, UI32_T port);


/*******************************************************************************
 * DEV_SWDRVL3_DisableUnicastStormProtect
 * Purpose: to allow the unicast storm to CPU
 * Inputs: None
 * Outputs: None
 * Return: TRUE/FALSE
 * Note :  (unit=0, port=0) means per system protect
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_DisableUnicastStormProtect(UI32_T unit, UI32_T port);

/*******************************************************************************
 * DEV_SWDRVL3_EnableIPv4Routing
 *
 * Purpose: This function enables the L3 Routing function.
 * Inputs: None.
 * Outputs: None.
 * Return: TRUE/FALSE
 * Notes: None.
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_EnableIPv4Routing(UI32_T flags, UI32_T vir_rt_id);

/*******************************************************************************
 * DEV_SWDRVL3_EnableIPv6Routing
 *
 * Purpose: This function enables the L3 Routing function.
 * Inputs: None.
 * Outputs: None.
 * Return: TRUE/FALSE
 * Notes: None.
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_EnableIPv6Routing(UI32_T flags, UI32_T vir_rt_id);
/*******************************************************************************
 * DEV_SWDRVL3_EnableRouting
 *
 * Purpose: This function enables the L3 Routing function.
 * Inputs: None.
 * Outputs: None.
 * Return: TRUE/FALSE
 * Notes: None.
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_EnableRouting(UI32_T flags, UI32_T vir_rt_id);

/*******************************************************************************
 * DEV_SWDRVL3_DisableIPv4Routing
 *
 * Purpose: Disable GalNet-3 routing function.
 * Inputs: None
 * Outputs: None
 * Return: TRUE/FALSE
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_DisableIPv4Routing(UI32_T flags, UI32_T vir_rt_id);

/*******************************************************************************
 * DEV_SWDRVL3_DisableIPv6Routing
 *
 * Purpose: Disable GalNet-3 routing function.
 * Inputs: None
 * Outputs: None
 * Return: TRUE/FALSE
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_DisableIPv6Routing(UI32_T flags, UI32_T vir_rt_id);

/*******************************************************************************
 * DEV_SWDRVL3_DisableRouting
 *
 * Purpose: Disable GalNet-3 routing function.
 * Inputs: None
 * Outputs: None
 * Return: TRUE/FALSE
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_DisableRouting(UI32_T flags, UI32_T vir_rt_id);


/*------------------------------------------------------------------------------
 * ROUTINE NAME : DEV_SWDRVL3_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for dev_swdrvl4.
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
 *------------------------------------------------------------------------------
 */
BOOL_T DEV_SWDRVL3_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  DEV_SWDRVL3_Backdoor_Main
 *-------------------------------------------------------------------------
 * PURPOSE  : DEV_SWDRVL3 backdoor function
 * INPUT    : none
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
void DEV_SWDRVL3_Backdoor_Main(void);

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

void DEV_SWDRVL3_Create_InterCSC_Relation(void);

I32_T 
DEV_SWDRVL3_Change_BrgMode_SGV(BOOL_T change);

I32_T 
DEV_SWDRVL3_ipmc_enable(UI32_T unit, UI32_T up_port, UI32_T dn_port);

I32_T 
DEV_SWDRVL3_ipmc_disable(void);

I32_T 
DEV_SWDRVL3_ipmc_debug_enable(void);

I32_T 
DEV_SWDRVL3_ipmc_debug_disable(void);

I32_T
DEV_SWDRVL3_l2mc_port_add(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[],
                   UI8_T src_addr_a[], UI32_T port);
I32_T
DEV_SWDRVL3_l2mc_port_del(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[], UI8_T src_addr_a[], UI32_T port);

BOOL_T 
DEV_SWDRVL3_UpdateMulticastAddrTblEntry(UI32_T vid, UI8_T *dip_p, UI8_T *sip_p, UI16_T fwd_priority);

I32_T
DEV_SWDRVL3_l2mc_entry_add(UI32_T vrf_id, 
                   UI32_T vid, 
                   UI8_T grp_addr_a[],
                   UI8_T src_addr_a[], 
                   #if (SYS_CPNT_IGMPSNP_FORWARD_PRIORITY == TRUE)
                   UI16_T fwd_priority,
                   #endif
                   UI8_T *l2port);

I32_T
DEV_SWDRVL3_l2mc_entry_del(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[], UI8_T src_addr_a[]);

I32_T 
DEV_SWDRVL3_l3mc_port_add(UI32_T vrf_id, UI32_T up_vid, UI8_T grp_addr_a[], UI8_T src_addr_a[], UI32_T vid, UI32_T port);

I32_T 
DEV_SWDRVL3_l3mc_port_del(UI32_T vrf_id, UI32_T up_vid, UI8_T grp_addr_a[], UI8_T src_addr_a[], UI32_T vid, UI32_T port);

I32_T 
DEV_SWDRVL3_l3mc_dnintf_add(UI32_T vrf_id, UI32_T up_vid, UI8_T grp_addr_a[], UI8_T src_addr_a[], UI32_T dn_vid, UI8_T *l3port);

I32_T 
DEV_SWDRVL3_l3mc_dnintf_del(UI32_T vrf_id, UI32_T up_vid, UI8_T grp_addr_a[], UI8_T src_addr_a[], UI32_T dn_vid, UI8_T *l3port);
#if 0
I32_T 
DEV_SWDRVL3_l3mc_repl_add(UI32_T vrf_id, UI32_T up_vid, UI8_T grp_addr_a[], UI8_T src_addr_a[], UI32_T dn_vid, UI8_T *l3port);

I32_T 
DEV_SWDRVL3_l3mc_repl_del(UI32_T vrf_id, UI32_T up_vid, UI8_T grp_addr_a[], UI8_T src_addr_a[], UI32_T dn_vid, UI8_T *l3port);
#endif
I32_T
DEV_SWDRVL3_l3mc_entry_add(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[], UI8_T src_addr_a[], UI8_T *l2port, UI8_T count, swdrv_dnintf_t *dn_intf);

I32_T
DEV_SWDRVL3_l3mc_entry_del(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[], UI8_T src_addr_a[], UI8_T count, swdrv_dnintf_t *dn_intf);

UI32_T
DEV_SWDRVL3_l3mc_entry_stat(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[], UI8_T src_addr_a[], UI32_T count);

I32_T
DEV_SWDRVL3_l3mc_cpu_port_del(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[], UI8_T src_addr_a[]);

I32_T
DEV_SWDRVL3_l3mc_cpu_port_add(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[], UI8_T src_addr_a[]);

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRVL3_SetEcmpBalanceMode
 * -------------------------------------------------------------------------
 * FUNCTION: To set ECMP balance mode
 * INPUT   : balance_mode bitmap - DEV_SWDRVL3_ECMP_DST_IP
 *                                 DEV_SWDRVL3_ECMP_L4_PORT
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : Call DEV_SWDRV_SetHashSelectionForECMP for Hash-Selection
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRVL3_SetEcmpBalanceMode(UI32_T balance_mode);
#endif

/*******************************************************************************
 * DEV_SWDRVL3_DeleteInetMyIpHostRoute
 *
 * Purpose: This function will delete a "My IP" host entry.
 * Inputs:
 *          host  : My IP Host entry information
 *              The input KEY fields are:
 *                  host->flags
 *                  host->fib_id
 *                  host->ip_addr
 *                  host->unit
 *
 * Outputs: None
 * Return:
 *          DEV_SWDRVL3_L3_BUCKET_FULL 	HW entry bucket is full
 *  	    DEV_SWDRVL3_L3_NO_ERROR 	Delete host entry successfully
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_DeleteInetMyIpHostRoute(DEV_SWDRVL3_Host_T *host);

/*******************************************************************************
 * DEV_SWDRVL3_AddInetECMPRouteMultiPath
 *
 * Purpose: This function will add an ECMP route with multiple nexthop.
 * Inputs:
 *          route  : Route entry information 
 *              The input KEY fields are:
 *                  route->flags
 *                  route->fib_id
 *                  route->dst_ip
 *                  route->prefix_length
 *                  route->unit
 *
 *          nh_hw_info_array : The array pointer of HW information of the multiple nexthop
 *          count : Indicate number of the multiple nexthop
 *
 * Outputs: None
 * Return:
 *          DEV_SWDRVL3_L3_ECMP_BUCKET_FULL
 *          DEV_SWDRVL3_L3_BUCKET_FULL
 *  	    DEV_SWDRVL3_L3_NO_ERROR
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_AddInetECMPRouteMultiPath(DEV_SWDRVL3_Route_T *route, void *nh_hw_info_array, UI32_T count);

/*******************************************************************************
 * DEV_SWDRVL3_AddInetECMPRouteOnePath
 *
 * Purpose: his function will add a single nexthop of an ECMP route.
 * Inputs:
 *          route  : Route entry information 
 *              The input KEY fields are:
 *                  route->flags
 *                  route->fib_id
 *                  route->dst_ip
 *                  route->prefix_length
 *                  route->unit
 *                  route->hw_info
 *
 *          nh_hw_info 	The HW information of the nexthop
 *          is_first 	Indicate the nexthop is the first one or not
 *
 * Outputs: route->hw_info
 * Return:  
 *          DEV_SWDRVL3_L3_OTHERS
 *          DEV_SWDRVL3_L3_NO_ERROR
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_AddInetECMPRouteOnePath(DEV_SWDRVL3_Route_T *route, void *nh_hw_info, BOOL_T is_first);

/*******************************************************************************
 * DEV_SWDRVL3_AddInetHostTunnelRoute
 *
 * Purpose: This function will add or update a host entry with tunnel initiator and terminator.
 * Inputs:
 *          host  : host entry information
 *              The input KEY fields are:
 *                  host->flags
 *                  host->fib_id
 *                  host->ip_addr
 *                  host->mac
 *                  host->vid
 *                  host->unit
 *                  host->port OR host->trunk_id
 *                  host->hw_info If the action is updating
 *		    host->tnl_init
 *		    host->tnl_term
 *
 * Outputs: 
 *          host->hw_info
 *	    host->tnl_init.l3a_intf_id
 *
 * Return:  DEV_SWDRVL3_L3_BUCKET_FULL
 *          DEV_SWDRVL3_L3_NO_ERROR
 * Note:    1. The hw_info indicate the Egress-Object handler in BCM SDK.
 *          2. If hw_info is invalid, that means create a new host entry and BCM Egress Object.
 *          3. If hw_info not invalid, do updating of BCM Egress Object.
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_AddInetHostTunnelRoute(DEV_SWDRVL3_HostTunnel_T *host);

/*******************************************************************************
 * DEV_SWDRVL3_AddInetMyIpHostRoute
 *
 * Purpose: This function will add a "My IP" host entry.
 * Inputs:
 *          host  : My IP Host entry information
 *              The input KEY fields are:
 *                  host->flags
 *                  host->fib_id
 *                  host->ip_addr
 *                  host->unit
 *
 * Outputs: None
 * Return:
 *          DEV_SWDRVL3_L3_BUCKET_FULL 	HW entry bucket is full
 *  	    DEV_SWDRVL3_L3_NO_ERROR 	Add net entry successfully
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_AddInetMyIpHostRoute(DEV_SWDRVL3_Host_T *host);

/*******************************************************************************
 * DEV_SWDRVL3_AddInetNetRoute
 *
 * Purpose: This function will add a route entry.
 * Inputs:
 *          route  : route entry information
 *              The input KEY fields are:
 *                  route->flags
 *                  route->fib_id
 *                  route->dst_ip
 *                  route->prefix_length
 *                  route->unit
 *
 *          nh_hw_info : The HW information of the nexthop
 *
 * Outputs: 
 *          host->hw_info
 * Return:  
 *          DEV_SWDRVL3_L3_BUCKET_FULL 	HW entry bucket is full
 *  	    DEV_SWDRVL3_L3_NO_ERROR 	Add net entry successfully
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_AddInetNetRoute(DEV_SWDRVL3_Route_T *route, void *nh_hw_info);

/*******************************************************************************
 * DEV_SWDRVL3_AddL3Mac
 *
 * Purpose: Add one MAC address with L3 bit On(For vrrp HSRP)
 * Inputs:
 *			vlan_mac: MAC address of vlan interface
 *			vid		: VLAN ID of this MAC address
 * Outputs: None
 * RETURN:  UI32_T    Success, or cause of fail.
 * Note: 1. This MAC address will be the Router MAC for any subnet/IP interface
 *          deployed on this VLAN
 *       2. For Intel solution, there is only ONE MAC for the whole routing system.
 *       3. If IP Routing is disabled, the router MAC should just delete L3 bit.
 *          And the behavior of CPU mac will be just the same as L2 Intervention.
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_AddL3Mac(UI32_T vid, UI8_T *l3_mac);

/*******************************************************************************
 * DEV_SWDRVL3_AddTunnelInitiator
 *
 * Purpose: This function will add a tunnel initiator.
 * Inputs:
 *          tunnel : tunnel initiator information
 *              The input KEY fields are:
 *                  tunnel->tunnel_type
 *                  tunnel->sip
 *                  tunnel->dip
 *                  tunnel->ttl
 *                  tunnel->nexthop_mac
 *
 * Outputs: 
 *          tunnel->l3_intf_id
 * Return:  
 *          DEV_SWDRVL3_L3_BUCKET_FULL 	HW entry bucket is full
 *  	    DEV_SWDRVL3_L3_NO_ERROR 	Add net entry successfully
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_AddTunnelInitiator(DEV_SWDRVL3_TunnelInitiator_T *tunnel);

/*******************************************************************************
 * DEV_SWDRVL3_AddTunnelIntfL3
 *
 * Purpose: This function will add/del a tunnel l3 intf.
 * Inputs:
 *          tl3_p : tunnel l3 intf information
 *                  l3_intf_id is key for delete
 *
 *          is_add: TRUE if it's to add
 * Outputs:
 *          tl3_p->l3_intf_id for add
 * Return:
 *  	    DEV_SWDRVL3_L3_NO_ERROR/DEV_SWDRVL3_L3_OTHERS
 * Note:    This function is used by VxLAN for now.
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_AddTunnelIntfL3(DEV_SWDRVL3_TunnelIntfL3_T *tl3_p, BOOL_T is_add);

/*******************************************************************************
 * DEV_SWDRVL3_AddTunnelTerminator
 *
 * Purpose: This function will add a tunnel terminator.
 * Inputs:
 *          tunnel : tunnel terminator information
 *              The input KEY fields are:
 *                  tunnel->fib_id
 *                  tunnel->tunnel_type
 *                  tunnel->sip
 *                  tunnel->dip
 *                  tunnel->port_bitmap
 *
 * Outputs: None
 *          
 * Return:  
 *          DEV_SWDRVL3_L3_BUCKET_FULL 	HW entry bucket is full
 *  	    DEV_SWDRVL3_L3_NO_ERROR 	Add net entry successfully
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_AddTunnelTerminator(DEV_SWDRVL3_TunnelTerminator_T *tunnel);

/*******************************************************************************
 * DEV_SWDRVL3_ClearHostRouteHWInfo
 *
 * Purpose: This function will Clear hw_info of a host entry.
 * Inputs:
 *          host  : host entry information
 *              The input KEY fields are:
 *                  host->hw_info
 *
 * Outputs: host->hw_info
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_ClearHostRouteHWInfo(DEV_SWDRVL3_Host_T *host);

/*******************************************************************************
 * DEV_SWDRVL3_ClearNetRouteHWInfo
 *
 * Purpose: This function will Clear hw_info of an ECMP route entry.
 * Inputs:
 *          route  : route entry information
 *              The input KEY fields are:
 *                  route->hw_info
 *
 * Outputs: route->hw_info
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_ClearNetRouteHWInfo(DEV_SWDRVL3_Route_T *route);

/*******************************************************************************
 * DEV_SWDRVL3_CreateL3Interface
 *
 * Purpose: This function will create an L3 interface.
 * Inputs: 
 *          fib_id - FIB Id
 *          vid - VLAN ID of the interface
 *          vlan_mac - MAC address of the interface
 * Outputs:
 *          hw_info  - bcm_l3_intf_t.l3a_intf_id(interface id of L3 table on chip)
 * RETURN:  UI32_T   - Error code
 * Notes:
 *          1. The MAC address will be the Router MAC for any subnet/IP interface
 *             deployed on this VLAN.
 *          2. The MAC address should be added to ARL with BCM solution.
 *          3. hw_info is required to delete the L3 interface on chip.
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_CreateL3Interface(UI32_T fib_id, UI32_T vid, const UI8_T *vlan_mac, void **hw_info);

/*******************************************************************************
 * DEV_SWDRVL3_DeleteInetECMPRoute
 *
 * Purpose: This function will delete an ECMP route.
 * Inputs:
 *          route  : Route entry information 
 *              The input KEY fields are:
 *                  route->flags
 *                  route->fib_id
 *                  route->dst_ip
 *                  route->prefix_length
 *                  route->unit
 *                  route->hw_info
 *
 * Outputs: route
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_DeleteInetECMPRoute(DEV_SWDRVL3_Route_T *route);

/*******************************************************************************
 * DEV_SWDRVL3_DeleteInetECMPRouteOnePath
 *
 * Purpose: This function will delete a single nexthop of an ECMP route.
 * Inputs:
 *          route  : Route entry information 
 *              The input KEY fields are:
 *                  route->flags
 *                  route->fib_id
 *                  route->dst_ip
 *                  route->prefix_length
 *                  route->unit
 *                  route->hw_info
 *
 *          nh_hw_info 	The HW information of the nexthop
 *          is_first 	Indicate the nexthop is the first one or not
 *
 * Outputs: route
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_DeleteInetECMPRouteOnePath(DEV_SWDRVL3_Route_T *route, void *nh_hw_info, BOOL_T is_last);

/*******************************************************************************
 * DEV_SWDRVL3_DeleteInetHostRoute
 *
 * Purpose: This function will delete a host entry.
 * Inputs:
 *          host  : host entry information
 *              The input KEY fields are:
 *                  host->flags
 *                  host->fib_id
 *                  host->ip_addr
 *                  host->mac
 *                  host->vid
 *                  host->unit
 *                  host->port OR host->trunk_id
 *
 * Outputs: 
 *          host->hw_info
 * Return:  TRUE/FALSE
 * Note:    1. The hw_info indicate the Egress-Object handler in BCM SDK.
 *          2. This function should call bcm_l3_egress_destroy to clear the hw_info, 
 *             if this action failed, Core Layer (AMTRL3) should call 
 *             the "DEV_SWDRVL3_ClearHostRouteHWInfo" to clear the hw_info.
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_DeleteInetHostRoute(DEV_SWDRVL3_Host_T *host);

/*******************************************************************************
 * DEV_SWDRVL3_DeleteInetHostTunnelRoute
 *
 * Purpose: This function will delete a host entry with associated tunnel initiator and temrinator.
 * Inputs:
 *          host  : host entry information
 *              The input KEY fields are:
 *                  host->flags
 *                  host->fib_id
 *                  host->ip_addr
 *                  host->mac
 *                  host->vid
 *                  host->unit
 *                  host->port OR host->trunk_id
 *		    host->tnl_init
 *		    host->tnl_term
 *
 * Outputs: 
 *          host->hw_info
 * Return:  TRUE/FALSE
 * Note:    1. The hw_info indicate the Egress-Object handler in BCM SDK.
 *          2. This function should call bcm_l3_egress_destroy to clear the hw_info, 
 *             if this action failed, Core Layer (AMTRL3) should call 
 *             the "DEV_SWDRVL3_ClearHostRouteHWInfo" to clear the hw_info.
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_DeleteInetHostTunnelRoute(DEV_SWDRVL3_HostTunnel_T *host);

/*******************************************************************************
 * DEV_SWDRVL3_DeleteInetMyIpHostRoute
 *
 * Purpose: This function will delete a "My IP" host entry.
 * Inputs:
 *          host  : My IP Host entry information
 *              The input KEY fields are:
 *                  host->flags
 *                  host->fib_id
 *                  host->ip_addr
 *                  host->unit
 *
 * Outputs: None
 * Return:
 *          DEV_SWDRVL3_L3_BUCKET_FULL 	HW entry bucket is full
 *  	    DEV_SWDRVL3_L3_NO_ERROR 	Delete host entry successfully
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_DeleteInetMyIpHostRoute(DEV_SWDRVL3_Host_T *host);

/*******************************************************************************
 * DEV_SWDRVL3_DeleteInetNetRoute
 *
 * Purpose: This function will delete a route entry.
 * Inputs:
 *          route  : route entry information
 *              The input KEY fields are:
 *                  route->flags
 *                  route->fib_id
 *                  route->dst_ip
 *                  route->prefix_length
 *                  route->unit
 *
 *          nh_hw_info : The HW information of the nexthop
 *
 * Outputs: 
 *          host->hw_info
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_DeleteInetNetRoute(DEV_SWDRVL3_Route_T *route, void *nh_hw_info);

/*******************************************************************************
 * DEV_SWDRVL3_DeleteL3Interface
 *
 * Purpose: This function will delete an L3 interface
 * Inputs: 
 *          fib_id - FIB Id
 *          vid - VLAN ID of the interface
 *          vlan_mac - MAC address of the interface
 *          hw_info  - bcm_l3_intf_t.l3a_intf_id which is output from
 *                     DEV_SWDRVL3_CreateL3Interface()
 * Outputs: None
 * Return: TRUE/FALSE
 * Notes:
 *          1. The MAC address will be the Router MAC for any subnet/IP interface
 *             deployed on this VLAN.
 *          2. The MAC address should be delete from ARL with BCM solution.
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_DeleteL3Interface(UI32_T fib_id, UI32_T vid, const UI8_T *vlan_mac, void *hw_info);

/*******************************************************************************
 * DEV_SWDRVL3_DeleteL3Mac
 *
 * Purpose: Remove one L3 MAC address that belonging to one vlan interface.
 * Inputs:
 *			vlan_mac: MAC address of vlan interface
 *			vid		: VLAN ID of this MAC address
 * Outputs: None
 * Return: TRUE/FALSE
 * Note: 1. This MAC address will be the Router MAC for any subnet/IP interface
 *          deployed on this VLAN
 *       2. For Intel solution, there is only ONE MAC for the whole routing system.
 *          This function is non-applicable for Intel solution
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_DeleteL3Mac(UI32_T vid, UI8_T *l3_mac);

/*******************************************************************************
 * DEV_SWDRVL3_DeleteSpecialDefaultRoute
 *
 * Purpose: This function will delete the special default route entry.
 * Inputs:
 *          default_route  : Route entry information 
 *              The input KEY fields are:
 *                  route->flags
 *                  route->fib_id
 *
 *          action : Indicate the special purpose of this Default Route entry
 *              The "action" are:
 *                  SWDRVL3_ACTION_TRAP2CPU Trap packet to CPU if match
 *                  SWDRVL3_ACTION_DROP Drop packet if match
 *
 * Outputs: route
 * Return:  TRUE/FALSE
 * Note:    The implementation DOES NOT accept "SWDRVL3_ACTION_ROUTE".
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_DeleteSpecialDefaultRoute(DEV_SWDRVL3_Route_T *default_route, UI32_T action);

/*******************************************************************************
 * DEV_SWDRVL3_DeleteTunnelInitiator
 *
 * Purpose: This function will delete a tunnel initiator associated with specific
 *          L3 interface ID.
 * Inputs:
 *          l3_intf_id : The L3 Interface ID associated with the tunnel initiator
 *
 * Outputs: None
 *          
 * Return:  
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_DeleteTunnelInitiator(UI32_T l3_intf_id);

/*******************************************************************************
 * DEV_SWDRVL3_DeleteTunnelTerminator
 *
 * Purpose: This function will delete a tunnel terminator.
 * Inputs:
 *          tunnel : tunnel terminator information
 *              The input KEY fields are:
 *                  tunnel->fib_id
 *                  tunnel->tunnel_type
 *                  tunnel->sip
 *                  tunnel->dip
 *                  tunnel->port_bitmap
 *
 * Outputs: None
 *          
 * Return:  
 *          DEV_SWDRVL3_L3_BUCKET_FULL 	HW entry bucket is full
 *  	    DEV_SWDRVL3_L3_NO_ERROR 	Add net entry successfully
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_DeleteTunnelTerminator(DEV_SWDRVL3_TunnelTerminator_T *tunnel);

/*******************************************************************************
 * DEV_SWDRVL3_ReadAndClearHostRouteEntryHitBit
 *
 * Purpose: This function will Read and Clear hit bit of a host entry.
 * Inputs:
 *          host  : host entry information
 *              The input KEY fields are:
 *                  host->flags
 *                  host->fib_id
 *                  host->ip_addr
 *
 *          hit : The read value of Hit bit
 *
 * Outputs: hit
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_ReadAndClearHostRouteEntryHitBit(DEV_SWDRVL3_Host_T *host, UI32_T *hit);

/*******************************************************************************
 * DEV_SWDRVL3_ReadAndClearNetRouteEntryHitBit
 *
 * Purpose: This function will Read and Clear hit bit of a net route entry.
 * Inputs:
 *          route  : route entry information
 *              The input KEY fields are:
 *                  route->flags
 *                  route->fib_id
 *                  route->dst_ip
 *                  route->prefix_length
 *
 *          hit : The read value of Hit bit
 *
 * Outputs: hit
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_ReadAndClearNetRouteEntryHitBit(DEV_SWDRVL3_Route_T *route, UI32_T *hit);

/*******************************************************************************
 * DEV_SWDRVL3_SetInetHostRoute
 *
 * Purpose: This function will add or update a host entry.
 * Inputs:
 *          host  : host entry information
 *              The input KEY fields are:
 *                  host->flags
 *                  host->fib_id
 *                  host->ip_addr
 *                  host->mac
 *                  host->vid
 *                  host->unit
 *                  host->port OR host->trunk_id
 *                  host->hw_info If the action is updating
 *
 * Outputs: 
 *          host->hw_info
 * Return:  DEV_SWDRVL3_L3_BUCKET_FULL
 *          DEV_SWDRVL3_L3_NO_ERROR
 * Note:    1. The hw_info indicate the Egress-Object handler in BCM SDK.
 *          2. If hw_info is invalid, that means create a new host entry and BCM Egress Object.
 *          3. If hw_info not invalid, do updating of BCM Egress Object.
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_SetInetHostRoute(DEV_SWDRVL3_Host_T *host);

/*******************************************************************************
 * DEV_SWDRVL3_SetL3Bit
 *
 * Purpose: Turn on the L3 bit of a MAC on a vlan interface
 * Inputs:
 *			vlan_mac: MAC address of vlan interface
 *			vid		: VLAN ID of this MAC address
 * Outputs: None
 * Return: TRUE/FALSE
 * Note: None
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_SetL3Bit(UI32_T vid, UI8_T *l3_mac);

/*******************************************************************************
 * DEV_SWDRVL3_SetL3InterfaceMtu
 *
 * Purpose: Set L3 MTU on vlan interface
 * Inputs:
 *          vid  : VLAN ID
 *          mtu  : L3 interface MTU 
 * Outputs: None
 * Return: TRUE/FALSE
 * Note: None
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_SetL3InterfaceMtu(UI32_T vid, UI32_T mtu);

/*******************************************************************************
 * DEV_SWDRVL3_SetSpecialDefaultRoute
 *
 * Purpose: This function will add a default route entry for special purpose.
 * Inputs:
 *          default_route  : Route entry information 
 *              The input KEY fields are:
 *                  route->flags
 *                  route->fib_id
 *
 *          action : Indicate the special purpose of this Default Route entry
 *              The "action" are:
 *                  SWDRVL3_ACTION_TRAP2CPU Trap packet to CPU if match
 *                  SWDRVL3_ACTION_DROP Drop packet if match
 *
 * Outputs: route
 * Return:  TRUE/FALSE
 * Note:    The implementation DOES NOT accept "SWDRVL3_ACTION_ROUTE".
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_SetSpecialDefaultRoute(DEV_SWDRVL3_Route_T *default_route, UI32_T action);

/*******************************************************************************
 * DEV_SWDRVL3_UnSetL3Bit
 *
 * Purpose: Turn off the L3 bit of a MAC on a vlan interface
 * Inputs:
 *			vlan_mac: MAC address of vlan interface
 *			vid		: VLAN ID of this MAC address
 * Outputs: None
 * Return: TRUE/FALSE
 * Note: None
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_UnSetL3Bit(UI32_T vid, UI8_T *l3_mac);

/*******************************************************************************
 * DEV_SWDRVL3_UpdateTunnelTtl
 *
 * Purpose: This function will add a tunnel terminator.
 * Inputs:
 *          tunnel : tunnel initiator information
 *              The input KEY fields are:
 *                  tunnel->l3_intf_id
 *                  tunnel->ttl
 *
 * Outputs: N/A
 *          
 * Return:  
 *          DEV_SWDRVL3_L3_BUCKET_FULL 	HW entry bucket is full
 *  	    DEV_SWDRVL3_L3_NO_ERROR 	Add net entry successfully
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_UpdateTunnelTtl(UI32_T l3_intf_id, UI8_T ttl);

I32_T DEV_SWDRVL3_l3mc_repl_add(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[],
                                     UI8_T src_addr_a[], swdrv_dnintf_t *dn_intf);

I32_T DEV_SWDRVL3_l3mc_repl_del(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[],
                                     UI8_T src_addr_a[], swdrv_dnintf_t *dn_intf);

#endif /* End of DEV_SWDRVL3.H */

