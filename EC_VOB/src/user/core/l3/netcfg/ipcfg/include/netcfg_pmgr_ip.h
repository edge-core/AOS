/* MODULE NAME:  netcfg_pmgr_ip.h
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to 
 *    access NETCFG_MGR_IP and NETCFG_OM_IP service.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    01/22/2008 - Vai Wang, Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */
 
#ifndef NETCFG_PMGR_IP_H
#define NETCFG_PMGR_IP_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "netcfg_type.h"
#include "ipal_types.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME : NETCFG_PMGR_IP_InitiateProcessResource
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
 *    Before other CSC use NETCFG_PMGR_IP, it should initiate 
 *    the resource (get the message queue handler internally)
 *
 */
BOOL_T NETCFG_PMGR_IP_InitiateProcessResource(void);


/* FUNCTION NAME : NETCFG_PMGR_IP_CreateL3If
 * PURPOSE:
 *    Create a L3 interface.
 * INPUT:
 *    vid -- vlan id
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_MAC_COLLISION
 *
 * NOTES:
 *    None
 *
 */
UI32_T NETCFG_PMGR_IP_CreateL3If(UI32_T vid);

/* FUNCTION NAME : NETCFG_PMGR_IP_CreateLoopbackInterface
 * PURPOSE:
 *    Create a loopback interface.
 * INPUT:
 *    lo_id -- loopback id
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_MAC_COLLISION
 *
 * NOTES:
 *    None
 *
 */
UI32_T NETCFG_PMGR_IP_CreateLoopbackInterface(UI32_T lo_id);

/* FUNCTION NAME : NETCFG_PMGR_IP_DeleteL3If
 * PURPOSE:
 *    Delete a L3 interface.
 * INPUT:
 *    vid -- vlan id
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *    None
 *
 */
UI32_T NETCFG_PMGR_IP_DeleteL3If(UI32_T vid);

/* FUNCTION NAME : NETCFG_PMGR_IP_DeleteLoopbackInterface
 * PURPOSE:
 *    Delete a Loopback interface.
 * INPUT:
  *    lo_id -- loopback id
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *    None
 *
 */
UI32_T NETCFG_PMGR_IP_DeleteLoopbackInterface(UI32_T lo_id);

/* FUNCTION NAME : NETCFG_PMGR_IP_SetInetRif
 * PURPOSE:
 *      To add or delete rif. also remember the incoming type
 * INPUT:
 *      rif_p               -- pointer to rif
 *      incoming_type       -- CLI/Web      1
 *                             SNMP         2
 *                             Dynamic      3
 *      rif_p->rowStatus    -- the action on this entry, valid actions :
 *          VAL_netConfigStatus_2_active          1L
 *              make a RIF to be active.
 *          VAL_netConfigStatus_2_notInService    2L
 *          VAL_netConfigStatus_2_notReady        3L
 *              disable the circuit, to change some configurations.
 *          VAL_netConfigStatus_2_createAndGo     4L
 *              not allowed in IPCFG, by field config, could not active immediately.
 *          VAL_netConfigStatus_2_createAndWait   5L
 *              create entry and wait other fields config.
 *          VAL_netConfigStatus_2_destroy         6L
 *              disable and destroy an entry.
 *
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_REJECT_SETTING_ENTRY
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 * NOTES:
 *      None.
 */
UI32_T NETCFG_PMGR_IP_SetInetRif(NETCFG_TYPE_InetRifConfig_T *rif_p, UI32_T incoming_type);


/* FUNCTION NAME : NETCFG_PMGR_IP_SetIpAddressMode
 * PURPOSE:
 *      Set interface IP address mode.
 *
 * INPUT:
 *      ifindex -- the interface be assoicated.
 *      ip_addr_mode-- one of {DHCP | BOOTP | USER_DEFINE}
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK -- keep this information in IPCFG_OM.
 *      NETCFG_TYPE_INTERFACE_NOT_EXISTED -- interface (ifindex do not exist)
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      1. Interface Ip Address Mode (or Access Method) kept in VLAN.
 */
UI32_T  NETCFG_PMGR_IP_SetIpAddressMode(UI32_T ifindex, NETCFG_TYPE_IP_ADDRESS_MODE_T ip_addr_mode);

#if (SYS_CPNT_PROXY_ARP == TRUE)
/* FUNCTION NAME : NETCFG_PMGR_IP_SetIpNetToMediaProxyStatus
 * PURPOSE:
 *      Set ARP proxy enable/disable.
 *
 * INPUT:
 *      ifindex -- the interface.
 *      status -- one of {TRUE(stand for enable) |FALSE(stand for disable)}
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK/
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T NETCFG_PMGR_IP_SetIpNetToMediaProxyStatus(UI32_T ifindex, BOOL_T status);

/* FUNCTION NAME : NETCFG_PMGR_IP_GetRunningIpNetToMediaProxyStatus
 * PURPOSE:
 *      Get running proxy ARP status.
 *
 * INPUT:
 *      ifindex -- the interface.
 *
 * OUTPUT:
 *      status -- one of {TRUE(stand for enable) |FALSE(stand for disable)}
 *
 * RETURN:
 *     SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *     SYS_TYPE_GET_RUNNING_CFG_FAIL/
 *     SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_IP_GetRunningIpNetToMediaProxyStatus(UI32_T ifindex, BOOL_T *status);
#endif /* #if (SYS_CPNT_PROXY_ARP == TRUE) */

#if (SYS_CPNT_IPV6 == TRUE)

/* FUNCTION NAME : NETCFG_PMGR_IP_IPv6Enable
 * PURPOSE:
 *      To enable IPv6 processing on an interface that has not been configured
 *      with an explicit IPv6 address.
 *
 * INPUT:
 *      ifindex     -- interface index
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. It will automatically configures an IPv6 link-local unicast address 
 *         on the interface while also enabling the interface for IPv6 processing.
 */
UI32_T NETCFG_PMGR_IP_IPv6Enable(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_PMGR_IP_IPv6Disable
 * PURPOSE:
 *      To disable IPv6 processing on an interface that has not been configured
 *      with an explicit IPv6 address.
 *
 * INPUT:
 *      ifindex     -- interface index
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. It does not disable IPv6 processing on an interface that is configured 
 *         with an explicit IPv6 address.
 */
UI32_T NETCFG_PMGR_IP_IPv6Disable(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_PMGR_IP_IPv6AddrAutoconfigEnable
 * PURPOSE:
 *      To enable automatic configuration of IPv6 addresses using stateless 
 *      autoconfiguration on an interface and enable IPv6 processing on the interface. 
 *
 * INPUT:
 *      ifindex     -- interface index
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      1. The autoconfiguration process specified here applies only to hosts, not routers.
 *      2. The configuration is passed into TCP/IP protocol stack, and after the process, 
 *         the stack will call NETCFG_MGR_IP_SetInetRif to add into OM.
 */
UI32_T NETCFG_PMGR_IP_IPv6AddrAutoconfigEnable(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_PMGR_IP_DisableIPv6AddrAutoconfig
 * PURPOSE:
 *      To disable automatic configuration of IPv6 addresses.
 *
 * INPUT:
 *      ifindex     -- interface index
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      1. None.
 */
UI32_T NETCFG_PMGR_IP_IPv6AddrAutoconfigDisable(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_PMGR_IP_SetIPv6InterfaceMTU
 * PURPOSE:
 *      Set IPv6 interface MTU.
 *
 * INPUT:
 *      ifindex -- the interface be assoicated.
 *      mtu     -- MTU
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK -- keep this information in IPCFG_OM.
 *      NETCFG_TYPE_INTERFACE_NOT_EXISTED -- interface (ifindex do not exist)
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T  NETCFG_PMGR_IP_SetIPv6InterfaceMTU(UI32_T ifindex, UI32_T mtu);

UI32_T  NETCFG_PMGR_IP_UnsetIPv6InterfaceMTU(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_PMGR_IP_GetIPv6InterfaceMTU
 * PURPOSE:
 *      Get IPv6 interface MTU from kernel.
 *
 * INPUT:
 *      ifindex -- the interface be assoicated.
 *
 * OUTPUT:
 *      mtu_p   -- MTU
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_PMGR_IP_GetIPv6InterfaceMTU(UI32_T ifindex, UI32_T *mtu_p);

/* FUNCTION NAME : NETCFG_PMGR_IP_GetNextIfJoinIpv6McastAddr
 * PURPOSE:
 *      Get next joined multicast group for the interface.
 *
 * INPUT:
 *      ifindex     -- interface 
 *
 * OUTPUT:
 *      mcaddr_p    -- pointer to multicast address
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_PMGR_IP_GetNextIfJoinIpv6McastAddr(UI32_T ifindex, L_INET_AddrIp_T *mcaddr_p);

/* FUNCTION NAME : NETCFG_PMGR_IP_GetNextPMTUEntry
 * PURPOSE:
 *      Get next path mtu entry.
 *
 * INPUT:
 *      entry_p     -- pointer to entry
 *
 * OUTPUT:
 *      entry_p     -- pointer to entry
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_PMGR_IP_GetNextPMTUEntry(NETCFG_TYPE_PMTU_Entry_T *entry_p);

/* FUNCTION NAME : NETCFG_PMGR_IP_GetIfIpv6AddrInfo
 * PURPOSE:
 *      Get ipv6 address info 
 *
 * INPUT:
 *      ifindex     -- interface 
 *
 * OUTPUT:
 *      addr_info_p -- pointer to address info
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_PMGR_IP_GetIfIpv6AddrInfo(NETCFG_TYPE_IpAddressInfoEntry_T *addr_info_p);

/* FUNCTION NAME : NETCFG_PMGR_IP_DhcpReleaseComplete
 * PURPOSE:
 *      If DHCP or DHCPv6 has release the last address, it will use this
 *      to notify NETCFG.
 *
 * INPUT:
 *      ifindex     -- interface 
 *      protocols   --
 *                  DHCP    0x01
 *                  DHCPv6  0x02
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      1.For IPv6, NETCFG may delet the link-local address after this 
 *        notification.
 */
UI32_T NETCFG_PMGR_IP_DhcpReleaseComplete(UI32_T ifindex, UI32_T protocols);

/* FUNCTION NAME : NETCFG_PMGR_IP_GetRunningIPv6EnableStatus
 * PURPOSE:
 *      Get "ipv6 enable" configuration of the interface.
 *
 * INPUT:
 *      ifindex     -- the interface.
 *
 * OUTPUT:
 *      status_p    -- status
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *      1.This function is for CLI running config.
 */
SYS_TYPE_Get_Running_Cfg_T 
NETCFG_PMGR_IP_GetRunningIPv6EnableStatus(UI32_T ifindex, BOOL_T *status_p);

/* FUNCTION NAME : NETCFG_PMGR_IP_GetRunningIPv6AddrAutoconfigEnableStatus
 * PURPOSE:
 *      To get the IPv6 address autoconfig enable status of the interface.
 *
 * INPUT:
 *      ifindex     -- interface index
 *
 * OUTPUT:
 *      status      -- status
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *      1.This function is for CLI running config.
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_IP_GetRunningIPv6AddrAutoconfigEnableStatus(UI32_T ifindex, BOOL_T *status_p);

/* FUNCTION NAME : NETCFG_PMGR_IP_GetRunningIPv6InterfaceMTU
 * PURPOSE:
 *      Get IPv6 interface MTU.
 *
 * INPUT:
 *      ifindex -- the interface be assoicated.
 *
 * OUTPUT:
 *      mtu_p   -- the pointer to the mtu value.
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *      1.This function is for CLI running config.
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_IP_GetRunningIPv6InterfaceMTU(UI32_T ifindex, UI32_T *mtu_p);

/* FUNCTION NAME : NETCFG_PMGR_IP_GetIPv6Statistics
 * PURPOSE:
 *      Get IPv6 statistics.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      ip6stat_p -- IPv6 statistics.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_PMGR_IP_GetIPv6Statistics(IPAL_Ipv6Statistics_T *ip6stat_p);

/* FUNCTION NAME : NETCFG_PMGR_IP_GetICMPv6Statistics
 * PURPOSE:
 *      Get ICMPv6 statistics.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp6stat_p -- ICMPv6 statistics.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_PMGR_IP_GetICMPv6Statistics(IPAL_Icmpv6Statistics_T *icmp6stat_p);

/* FUNCTION NAME : NETCFG_PMGR_IP_GetUDPv6Statistics
 * PURPOSE:
 *      Get UDPv6 statistics.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      udp6stat_p -- UDPv6 statistics.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_PMGR_IP_GetUDPv6Statistics(IPAL_Udpv6Statistics_T *udp6stat_p);

/* FUNCTION NAME : NETCFG_PMGR_IP_ClearIPv6StatisticsByType
 * PURPOSE:
 *      Clear IPv6 statistics by specified type.
 *
 * INPUT:
 *      clear_type -- which type to clear.
 *                    refer to NETCFG_MGR_IPV6_STAT_TYPE_E
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
UI32_T NETCFG_PMGR_IP_ClearIPv6StatisticsByType(UI32_T clear_type);

#endif /* SYS_CPNT_IPV6 */
#if (SYS_CPNT_IP_TUNNEL == TRUE)
/* FUNCTION NAME : NETCFG_PMGR_IP_CreateTunnelInterface
 * PURPOSE:
 *      Create a new tunnel interface
 *
 * INPUT:
 *      tunnel_id -- the tunnel id.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK 
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_EXIST
 *
 * NOTES:
 */
UI32_T NETCFG_PMGR_IP_CreateTunnelInterface(UI32_T tunnel_id);
/* FUNCTION NAME : NETCFG_PMGR_IP_DeleteTunnelInterface
 * PURPOSE:
 *      delete existing tunnel interface
 *
 * INPUT:
 *      tunnel_id -- the tunnel id.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK 
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_EXIST
 *
 * NOTES:
 */
UI32_T NETCFG_PMGR_IP_DeleteTunnelInterface(UI32_T tunnel_id);
/* FUNCTION NAME : NETCFG_PMGR_IP_SetTunnelIPv6Rif
 * PURPOSE:
 *      To add or delete rif.
 * INPUT:
 *      rif_p               -- pointer to rif
 *      incoming_type       -- CLI/Web      1
 *                             SNMP         2
 *                             Dynamic      3
 *
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_REJECT_SETTING_ENTRY
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 * NOTES:
 *      1. This function is used for both IPv4/IPv6. 
 */
UI32_T NETCFG_PMGR_IP_SetTunnelIPv6Rif(NETCFG_TYPE_InetRifConfig_T *rif_p,UI32_T incoming_type);
/* FUNCTION NAME : NETCFG_PMGR_IP_UnsetTunnelIPv6Rif
 * PURPOSE:
 *      To  delete rif.
 * INPUT:
 *      rif_p               -- pointer to rif
 *      incoming_type       -- CLI/Web      1
 *                             SNMP         2
 *                             Dynamic      3
 *
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_REJECT_SETTING_ENTRY
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 * NOTES:
 *      1. This function is used for both IPv4/IPv6. 
 */
UI32_T NETCFG_PMGR_IP_UnsetTunnelIPv6Rif(NETCFG_TYPE_InetRifConfig_T *rif_p,UI32_T incoming_type);

/*
* FUNCTION NAME : NETCFG_MGR_IP_SetTunnelSourceVLAN
 * PURPOSE:
 *      set tunnel source vlan 
 * INPUT:
 *      tunnel_ifindex: tunnel to be set
 *      source_vid: the source vid (NOT vid ifindex)
 *
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_TUNNEL_SOURCE_VLAN_HAS_NO_IPV4_ADDRESS
 *     
 * NOTES:
 *     source vlan must exist and have IPv4 address, otherwise return fail
 */
UI32_T NETCFG_PMGR_IP_SetTunnelSourceVLAN(UI32_T tid_ifindex, UI32_T src_vid);
/*
* FUNCTION NAME : NETCFG_PMGR_IP_UnsetTunnelSourceVLAN
 * PURPOSE:
 *      unset  tunnel source vlan 
 * INPUT:
 *      tunnel_ifindex: tunnel to be set
 *
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_TUNNEL_SOURCE_VLAN_HAS_NO_IPV4_ADDRESS
 *     
 * NOTES:
 *     the default source vlan is 0, i.e. not configured
 */
UI32_T NETCFG_PMGR_IP_UnsetTunnelSourceVLAN(UI32_T tid_ifindex);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_PMGR_IP_GetRunningTunnelSourceVlan
 * ------------------------------------------------------------------------
 * PURPOSE  :  Get tunnel source vlan configuration
 * INPUT    :   ifindex : tunnel ifindex
 * OUTPUT   :   src_vid: 
  * RETURN   :  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   According to current design, a tunnel interface can only have one rif; 
                        if design changed, this function have to modified.
 * ------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_PMGR_IP_GetRunningTunnelSourceVlan(UI32_T ifindex, UI32_T *src_vid);

UI32_T NETCFG_PMGR_IP_SetTunnelDestination(UI32_T tid_ifindex, L_INET_AddrIp_T* dip);
UI32_T NETCFG_PMGR_IP_UnsetTunnelDestination(UI32_T tid_ifindex);
SYS_TYPE_Get_Running_Cfg_T NETCFG_PMGR_IP_GetRunningTunnelDestination(UI32_T ifindex, L_INET_AddrIp_T *dip);

UI32_T NETCFG_PMGR_IP_SetTunnelMode(UI32_T tid_ifindex, NETCFG_TYPE_TUNNEL_MODE_T tunnel_mode);
UI32_T NETCFG_PMGR_IP_UnsetTunnelMode(UI32_T tid_ifindex);
SYS_TYPE_Get_Running_Cfg_T NETCFG_PMGR_IP_GetRunningTunnelMode(UI32_T ifindex, UI32_T *tunnel_mode);


UI32_T NETCFG_PMGR_IP_SetTunnelTtl(UI32_T tid_ifindex, UI32_T ttl);
UI32_T NETCFG_PMGR_IP_UnsetTunnelTtl(UI32_T tid_ifindex);
SYS_TYPE_Get_Running_Cfg_T NETCFG_PMGR_IP_GetRunningTunnelTtl(UI32_T ifindex, UI32_T *ttl);

/* FUNCTION NAME: NETCFG_PMGR_IP_GetTunnelIfindexBySrcVidIfindex
 * PURPOSE  : Get tunnel interface ifindex by its configuration source vlan ifindex
 * INPUT    : src_vid_ifindex -- Tunnel source vid ifindex
 *            
 * OUTPUT   : tunnel_l3_intf      -- Tunnel l3 interface
 * RETURN   : successful (NETCFG_TYPE_OK), failed (NETCFG_TYPE_FAIL)
 * NOTES    :
 *     
 */
UI32_T  NETCFG_PMGR_IP_GetTunnelIfindexBySrcVidIfindex(UI32_T src_vid_ifindex, NETCFG_TYPE_L3_Interface_T *tunnel_l3_intf);

/* FUNCTION NAME: NETCFG_PMGR_IP_SetTunnelRowStatus
 * PURPOSE  : set tunnel interface's row status
 * INPUT    : tunnel_id -- Tunnel interface index
 *            rowstatus -- action rowstatus(createAndGo/
                                            createAndWait/
                                            Active/
                                            NotReady/
                                            Destroy)
 *            
 * OUTPUT   : none
 * RETURN   : successful (NETCFG_TYPE_OK), failed (NETCFG_TYPE_FAIL)
 * NOTES    : This api is for snmp to use.
 *     
 */
UI32_T NETCFG_PMGR_IP_SetTunnelRowStatus(UI32_T tunnel_id,UI16_T rowstatus);

/* FUNCTION NAME: NETCFG_PMGR_IP_SetTunnelInterfaceFromSnmp
 * PURPOSE  : configure tunnel interface
 * INPUT    : tunnel_if  --  tunnel interface information 
 *            
 * OUTPUT   : none
 * RETURN   : successful (NETCFG_TYPE_OK), failed (NETCFG_TYPE_FAIL)
 * NOTES    : This api is for snmp to use.
 *            we use this api to set (destination/source vlan/mode) 
 *     
 */
UI32_T NETCFG_PMGR_IP_SetTunnelInterfaceFromSnmp(NETCFG_TYPE_L3_Interface_T *tunnel_if);
#endif /* SYS_CPNT_IP_TUNNEL */

#if (SYS_CPNT_CLUSTER == TRUE)
/* FUNCTION NAME: NETCFG_MGR_IP_ClusterVlanSetRifRowStatus
 * PURPOSE  : Set and delete a IP address on specific Cluster VLAN
 * INPUT    : ipAddress -- for cluster VLAN internal IP
 *            rowStatus -- only allow VAL_netConfigStatus_2_createAndGo and VAL_netConfigStatus_2_destroy
 *            incoming_type -- from which UI (CLI/WEB/dynamic)
 * OUTPUT   : none
 * RETURN   : NETCFG_TYPE_OK
 *            NETCFG_TYPE_FAIL
 * NOTES    :
 *      1. Because we can set only 1 IP, if there is existing one, delete it first. Then we process rowStatus request by each
 *         case, and only validate IP in createAndGo case. If no IP existed, destroy case will return OK.
 *      1. This function is derived from NETCFG_SetRifRowStatus() and for cluster vlan only.
 *      2. Allow only 1 IP address on this VLAN as far.
 *      3. rowStatus only provides VAL_netConfigStatus_2_createAndGo and VAL_netConfigStatus_2_destroy
 *      4. Call NETCFG_MGR_SetIpAddressMode() here.
 *      hawk, 2006.3.1
 */
UI32_T NETCFG_PMGR_IP_ClusterVlanSetRifRowStatus(UI8_T *ipAddress, UI8_T *ipMask, UI32_T rowStatus, UI32_T incoming_type);
#endif /* SYS_CPNT_CLUSTER */

#if (SYS_CPNT_CRAFT_PORT == TRUE)
/* FUNCTION NAME : NETCFG_PMGR_IP_SetCraftInterfaceInetAddress
 * PURPOSE:
 *      To add or delete ipv4/v6 address on craft interface
 * INPUT:
 *      rif_p               -- pointer to rif
 *
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 * NOTES:
 *      None
 */
UI32_T NETCFG_PMGR_IP_SetCraftInterfaceInetAddress(NETCFG_TYPE_CraftInetAddress_T *craft_addr_p);

/* FUNCTION NAME : NETCFG_PMGR_IP_IPv6Enable_Craft
 * PURPOSE:
 *      To enable/disable ipv6 address on craft interface
 * INPUT:
 *      ifindex            
 *      do_enable           -- enable/disable
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 * NOTES:
 *      None
 */
UI32_T NETCFG_PMGR_IP_IPv6Enable_Craft(UI32_T ifindex, BOOL_T do_enable);

#endif /* SYS_CPNT_CRAFT_PORT */


#if (SYS_CPNT_DHCP_INFORM == TRUE)
/* FUNCTION NAME : NETCFG_PMGR_IP_SetDhcpInform
 * PURPOSE:
 *      To enable/disable dhcp inform on L3 interface
 * INPUT:
 *      ifindex            
 *      do_enable           -- enable/disable
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 * NOTES:
 *      None
 */
UI32_T NETCFG_PMGR_IP_SetDhcpInform(UI32_T ifindex, BOOL_T do_enable);
#endif /* SYS_CPNT_DHCP_INFORM */

#endif /* NETCFG_PMGR_IP_H */

