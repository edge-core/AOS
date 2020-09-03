/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_PMGR_ND.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file declares the APIs for NIEGHBOR MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2008/01/18     --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */

#ifndef NETCFG_PMGR_ND_H
#define NETCFG_PMGR_ND_H
#include "netcfg_type.h"


/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for NETCFG_PMGR_ND in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  --  Sucess
 *           FALSE --  Error
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T NETCFG_PMGR_ND_InitiateProcessResource(void);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetNextIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Look up next NIEGHBOR entry.
 *
 * INPUT   : entry.
 *
 * OUTPUT  : entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_GetNextIpNetToMediaEntry(NETCFG_TYPE_IpNetToMediaEntry_T *entry);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_DeleteAllDynamicIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete all dynamic NIEGHBOR entries.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */

UI32_T NETCFG_PMGR_ND_DeleteAllDynamicIpNetToMediaEntry(void);
UI32_T NETCFG_PMGR_ND_DeleteAllDynamicIpv4NetToMediaEntry(void);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetIpNetToMediaTimeout
 *-----------------------------------------------------------------------------
 * PURPOSE : Set NIEGHBOR timeout.
 *
 * INPUT   : age_time.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetIpNetToMediaTimeout(UI32_T age_time);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_AddStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Add a static NIEGHBOR entry.
 *
 * INPUT   : vid_ifindex, ip_addr, phy_addr_len and phy_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_AddStaticIpNetToMediaEntry(UI32_T vid_ifIndex,
                                                                         UI32_T ip_addr,
                                                                         UI32_T phy_addr_len,
                                                                         UI8_T *phy_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_DeleteStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete a static NIEGHBOR entry.
 *
 * INPUT   : vid_ifindex, ip_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_DeleteStaticIpNetToMediaEntry(UI32_T vid_ifIndex, UI32_T ip_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_AddRouterRedundancyProtocolIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Add a VRRP/HSRP ARP entry.
 *
 * INPUT   : vid_ifindex, ip_addr, phy_addr_len and phy_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */

UI32_T NETCFG_PMGR_ND_AddRouterRedundancyProtocolIpNetToMediaEntry(UI32_T vid_ifindex,
                                                                         UI32_T ip_addr,
                                                                         UI32_T phy_addr_len,
                                                                         UI8_T *phy_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_DeleteRouterRedundancyProtocolIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Remove a VRRP/HSRP ARP entry.
 *
 * INPUT   : vid_ifindex, ip_addr, phy_addr_len and phy_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_DeleteRouterRedundancyProtocolIpNetToMediaEntry(UI32_T vid_ifindex,
                                                                         UI32_T ip_addr,
                                                                         UI32_T phy_addr_len,
                                                                         UI8_T *phy_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetStatistics
 *-----------------------------------------------------------------------------
 * PURPOSE : Get NIEGHBOR packet statistics.
 *
 * INPUT   : None.
 *
 * OUTPUT  : stat.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_GetStatistics(NETCFG_TYPE_IpNetToMedia_Statistics_T *stat);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Get an NIEGHBOR entry information.
 *
 * INPUT   : entry.
 *
 * OUTPUT  : entry.
 *
 * RETURN  :TRUE/
 *                 FALSE
 *
 * NOTES   :key are ifindex and ip address.
 *-----------------------------------------------------------------------------
 */
BOOL_T NETCFG_PMGR_ND_GetIpNetToMediaEntry(NETCFG_TYPE_IpNetToMediaEntry_T *entry);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Set a static NIEGHBOR entry invalid or valid.
 *
 * INPUT   : entry, type.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetStaticIpNetToMediaEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry, int type);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningIpNetToMediaTimeout
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running NIEGHBOR timeout.
 *
 * INPUT   : None.
 *
 * OUTPUT  : age_time.
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningIpNetToMediaTimeout(UI32_T *age_time);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetNextStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE :Lookup next static NIEGHBOR entry.
 *
 * INPUT   : entry.
 *
 * OUTPUT  : entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_GetNextStaticIpNetToMediaEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry);



/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_AddStaticIpNetToPhysicalEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Add a static NIEGHBOR entry.
 *
 * INPUT   : vid_ifindex, ip_addr, phy_addr_len and phy_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_AddStaticIpNetToPhysicalEntry(UI32_T vid_ifIndex,
                                                                         L_INET_AddrIp_T *ip_addr,
                                                                         NETCFG_TYPE_PhysAddress_T *phy_addr);
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_DeleteStaticIpNetToPhysicalEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete a static NIEGHBOR entry.
 *
 * INPUT   : vid_ifindex, L_INET_AddrIp_T.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_DeleteStaticIpNetToPhysicalEntry(UI32_T vid_ifIndex, L_INET_AddrIp_T* ip_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_AddRouterRedundancyProtocolIpNetToPhysicalEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Add a VRRP/HSRP ARP entry.
 *
 * INPUT   : vid_ifindex, ip_addr, phy_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_AddRouterRedundancyProtocolIpNetToPhysicalEntry(UI32_T vid_ifindex,
                                                                         L_INET_AddrIp_T* ip_addr,
                                                                         NETCFG_TYPE_PhysAddress_T *phy_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_DeleteRouterRedundancyProtocolIpNetToPhysicalEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Remove a VRRP/HSRP ARP entry.
 *
 * INPUT   : vid_ifindex, ip_addr, phy_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_DeleteRouterRedundancyProtocolIpNetToPhysicalEntry(UI32_T vid_ifindex,
                                                                         L_INET_AddrIp_T* ip_addr,
                                                                         NETCFG_TYPE_PhysAddress_T *phy_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetIpNetToPhysicalEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Get an NIEGHBOR entry information.
 *
 * INPUT   : entry. key is  ip_net_to_media_phys_address
 *
 * OUTPUT  : entry.
 *
 * RETURN  :TRUE/
 *                 FALSE
 *
 * NOTES   :key are ifindex and ip address.
 *-----------------------------------------------------------------------------
 */
BOOL_T NETCFG_PMGR_ND_GetIpNetToPhysicalEntry(NETCFG_TYPE_IpNetToPhysicalEntry_T *entry);
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetNextStaticIpNetToPhysicalEntry
 *-----------------------------------------------------------------------------
 * PURPOSE :Lookup next static NIEGHBOR entry.
 *
 * INPUT   : entry. key is ip_net_to_physical_if_index + ip_net_to_physical_net_address
 *               when key set to 0, get 1st entry
 *
 * OUTPUT  : entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *               this API gat all IpNetToPhysical entries, including IPV4 and IPV6. 
 *                If you  need  only v4, use NETCFG_PMGR_ND_GetNextIpNetToMediaEntry;
 *  `            if you need only v6, use NETCFG_PMGR_ND_GetNextStaticIpv6NetToMediaEntry
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_GetNextStaticIpNetToPhysicalEntry(NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry);
UI32_T NETCFG_PMGR_ND_GetNextStaticIpv4NetToPhysicalEntry(NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry);
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetNextIpv6NetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Look up next NIEGHBOR entry.
 *
 * INPUT   : entry.  entry.ip_net_to_physical_net_address.type must givein(L_INET_ADDR_TYPE_IPV4 or L_INET_ADDR_TYPE_IPV6)
 *              when entry.ip_net_to_physical_if_index and entry.
 *  
 *
 * OUTPUT  : entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_GetNextIpNetToPhysicalEntry(NETCFG_TYPE_IpNetToPhysicalEntry_T *entry);
UI32_T NETCFG_PMGR_ND_GetNextIpv4NetToPhysicalEntry(NETCFG_TYPE_IpNetToPhysicalEntry_T *entry);
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetStaticIpv6NetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Set a static NIEGHBOR entry invalid or valid.
 *
 * INPUT   : entry,
 *                  type should be one of follow type:.
 *                            VAL_ipNetToMediaType_invalid
 *                            VAL_ipNetToMediaType_static
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetStaticIpNetToPhysicalEntry(NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry, int type);

#if (SYS_CPNT_IPV6 == TRUE)
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetNextStaticIpv6NetToPhysicalEntry
 *-----------------------------------------------------------------------------
 * PURPOSE :Lookup next static NIEGHBOR entry.
 *
 * INPUT   : entry. key is ip_net_to_physical_if_index + ip_net_to_physical_net_address
 *               when key set to 0, get 1st entry
 *
 * OUTPUT  : entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *               this API gat all IpNetToPhysical entries, including IPV4 and IPV6. 
 *                If you  need  only v4, use NETCFG_PMGR_ND_GetNextIpNetToMediaEntry;
 *  `            if you need only v6, use NETCFG_PMGR_ND_GetNextStaticIpv6NetToMediaEntry
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_GetNextStaticIpv6NetToPhysicalEntry(NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry);



/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetNextIpv6NetToPhysicalEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Look up next NIEGHBOR entry.
 *
 * INPUT   : entry.  entry.ip_net_to_physical_net_address.type must givein(L_INET_ADDR_TYPE_IPV4 or L_INET_ADDR_TYPE_IPV6)
 *              when entry.ip_net_to_physical_if_index and entry.
 *  
 *
 * OUTPUT  : entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T  NETCFG_PMGR_ND_GetNextIpv6NetToPhysicalEntry(NETCFG_TYPE_IpNetToPhysicalEntry_T *entry);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_DeleteAllDynamicIpv6NetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete all dynamic NIEGHBOR entries.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_DeleteAllDynamicIpv6NetToMediaEntry(void);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdDADAttempts
 *-----------------------------------------------------------------------------
 * PURPOSE :configure the number of consecutive neighbor solicitation messages 
 *                      that are sent on an interface while duplicate address detection is performed 
 *                      on the unicast IPv6 addresses of the interface
 *
 * INPUT   : UI32_T attempts :  The number of neighbor solicitation messages
 *
 * OUTPUT  : None.
 *
 * RETURN  :      NETCFG_TYPE_OK /
 *                      NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdDADAttempts(UI32_T vid_ifIndex,UI32_T attempts);
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningDADAttempts
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND DAD attempts .
 *
 * INPUT   : none .
 *
 * OUTPUT  : attempts.
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningDADAttempts(UI32_T vid_ifIndex,UI32_T* attempts);


/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdNsInterval
 *-----------------------------------------------------------------------------
 * PURPOSE :configure the interval between IPv6 neighbor solicitation retransmissions on an interface
 *
 * INPUT   :    UI32_T vid_ifIndex: interface id
 *                  UI32_T msec :  The interval between IPv6 neighbor solicit transmissions in milliseconds
 *
 * OUTPUT  : None.
 *
 * RETURN  :      NETCFG_TYPE_OK /
 *                      NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdNsInterval(UI32_T vid_ifIndex, UI32_T msec);
UI32_T NETCFG_PMGR_ND_UnsetNdNsInterval(UI32_T vid_ifIndex);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningNdNsInterval
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND NS interval .
 *
 * INPUT   : vid_ifIndex .
 *
 * OUTPUT  : msec.
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningNdNsInterval(UI32_T vid_ifIndex, UI32_T* msec);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdReachableTime
 *-----------------------------------------------------------------------------
 * PURPOSE :configure the amount of time that a remote IPv6 node is considered reachable 
 *                  after some reachability confirmation event has occurred
 *
 * INPUT   :UI32_T vid_ifIndex: interface id 
 *              UI32_T msec:  amount of time to considered reachable in milliseconds
 *
 * OUTPUT  : None.
 *
 * RETURN  :      NETCFG_TYPE_OK /
 *                      NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdReachableTime(UI32_T vid_ifIndex,UI32_T msec);
UI32_T NETCFG_PMGR_ND_UnsetNdReachableTime(UI32_T vid_ifIndex);
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningNdReachableTime
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND reachalbe time by vlan .
 *
 * INPUT   : vid_ifIndex .
 *
 * OUTPUT  : msec.
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningNdReachableTime(UI32_T vid_ifIndex, UI32_T* msec);

#if (SYS_CPNT_IPV6_ROUTING == TRUE)
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdHoplimit
 *-----------------------------------------------------------------------------
 * PURPOSE :configure the maximum number of hops used in router advertisements 
 *
 * INPUT   : hoplimit :  
 *
 * OUTPUT  : None.
 *
 * RETURN  :      NETCFG_TYPE_OK /
 *                      NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdHoplimit(UI32_T hoplimit);
UI32_T NETCFG_PMGR_ND_UnsetNdHoplimit();
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningNdHoplimit
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND hoplimit .
 *
 * INPUT   : none .
 *
 * OUTPUT  : hoplimit.
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningNdHoplimit(UI32_T* hoplimit);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdPrefix
 *-----------------------------------------------------------------------------
 * PURPOSE :configure which IPv6 prefixes are included in IPv6 router advertisements
 *
 * INPUT   :UI32_T vid_ifIndex : interface id
 *              L_INET_AddrIp_T prefix :   Configure IPv6 prefixes in router advertisements
 *              UI32_T validLifetime : The amount of time (in seconds) 
 *                                                  that the specified IPv6 prefix is advertised as being valid
 *              
 *              UI32_T preferredLifetime: The amount of time (in seconds) 
 *                                                      that the specified IPv6 prefix is advertised as being preferred.
 *              BOOL_T enable_on_link : When FALSE, means do not use this prefix for onlink determination
 *              BOOL_T enable_autoconf:When FALSE, means do not use this prefix for autoconfiguration
 
 * OUTPUT  : None.
 *
 * RETURN  :      NETCFG_TYPE_OK /
 *                      NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdPrefix(UI32_T vid_ifIndex, L_INET_AddrIp_T* prefix, UI32_T validLifetime, UI32_T preferredLifetime,BOOL_T enable_on_link,BOOL_T enable_autoconf);
UI32_T NETCFG_PMGR_ND_UnsetNdPrefix(UI32_T vid_ifIndex, L_INET_AddrIp_T* prefix );

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningNdPrefix
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND prefix by vlan
 *
 * INPUT   : vid_ifIndex .
 *
 * OUTPUT  : prefix + validLifetime +preferredLifetime + offLink +noAutoconfig
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetNextRunningNdPrefix(UI32_T vid_ifIndex, L_INET_AddrIp_T* prefix, UI32_T *validLifetime, UI32_T *preferredLifetime,BOOL_T*enable_on_link,BOOL_T* enable_autoconf);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdManagedConfigFlag
 *-----------------------------------------------------------------------------
 * PURPOSE :To set the "managed address configuration" flag in IPv6 router advertisements
 *
 * INPUT   :UI32_T vid_ifIndex: interface id 
 *              BOOL_T enableFlag:  when TRUE means  flag on, when FLASE means flag off.
 *
 * OUTPUT  : None.
 *
 * RETURN  :      NETCFG_TYPE_OK /
 *                      NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdManagedConfigFlag(UI32_T vid_ifIndex, BOOL_T enableFlag);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningNdManagedConfigFlag
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND managed flag by vlan .
 *
 * INPUT   : vid_ifIndex .
 *
 * OUTPUT  : enableFlag.
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningNdManagedConfigFlag(UI32_T vid_ifIndex, BOOL_T* enableFlag);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdOtherConfigFlag
 *-----------------------------------------------------------------------------
 * PURPOSE :set the "other stateful configuration" flag in IPv6 router advertisements
 *
 * INPUT   :    UI32_T vid_ifIndex: interface id
 *                  BOOL_T enableFlag:  when TRUE means  flag on, when FLASE means flag off.
 *
 * OUTPUT  : None.
 *
 * RETURN  :      NETCFG_TYPE_OK /
 *                      NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdOtherConfigFlag(UI32_T vid_ifIndex, BOOL_T enableFlag);
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningNdOtherConfigFlag
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND other flag by vlan .
 *
 * INPUT   : vid_ifIndex .
 *
 * OUTPUT  : enableFlag.
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningNdOtherConfigFlag(UI32_T vid_ifIndex, BOOL_T* enableFlag);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdRaSuppress
 *-----------------------------------------------------------------------------
 * PURPOSE : suppress IPv6 router advertisement transmissions  
 *
 * INPUT   :UI32_T vid_ifIndex: interface 
 *              BOOL_T enableSuppress:  when TRUE means  supress , when FLASE means not suppress.
 *
 * OUTPUT  : None.
 *
 * RETURN  :      NETCFG_TYPE_OK /
 *                      NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdRaSuppress(UI32_T vid_ifIndex, BOOL_T enableSuppress);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningNdRaSuppress
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND RA supress by vlan .
 *
 * INPUT   : vid_ifIndex .
 *
 * OUTPUT  : msec.
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningNdRaSuppress(UI32_T vid_ifIndex, BOOL_T* enableSuppress);
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdRaLifetime
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the router lifetime value in IPv6 router advertisements 
 *
 * INPUT   :UI32_T vid_ifIndex: interface 
 *              UI32_T seconds:  router lifetime in seconds
 *
 * OUTPUT  : None.
 *
 * RETURN  :      NETCFG_TYPE_OK /
 *                      NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdRaLifetime(UI32_T vid_ifIndex, UI32_T seconds);
UI32_T NETCFG_PMGR_ND_UnsetNdRaLifetime(UI32_T vid_ifIndex);
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningNdRaLifetime
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND RA lifetime
 *
 * INPUT   : vid_ifIndex .
 *
 * OUTPUT  : seconds.
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningNdRaLifetime(UI32_T vid_ifIndex, UI32_T* seconds);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdRaInterval
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the interval between IPv6 router advertisement  transmissions
 *
 * INPUT   :vid_ifindex: interface ifindex
 *          max:  max router advertisement interval in seconds
 *          min:  min router advertisement interval in seconds
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK
 *           NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdRaInterval(UI32_T vid_ifindex, UI32_T max, UI32_T min);
UI32_T NETCFG_PMGR_ND_UnsetNdRaInterval(UI32_T vid_ifIndex);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningNdRaInterval
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND RA interval
 *
 * INPUT   : vid_ifindex -- vlan ifindex
 *
 * OUTPUT  : max_p -- max ra interval
 *           min_p -- min ra interval
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningNdRaInterval(UI32_T vid_ifindex, UI32_T *max_p, UI32_T* min_p);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_SetNdRouterPreference
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the Preference flag in IPv6 router advertisements
 *
 * INPUT   :UI32_T vid_ifIndex: interface 
 *              UI8_T perference: default route preference, possible value are one of:
 *                  {NETCFG_TYPE_ND_ROUTER_PERFERENCE_HIGH,
 *                      NETCFG_TYPE_ND_ROUTER_PERFERENCE_MEDIUM,
 *                       NETCFG_TYPE_ND_ROUTER_PERFERENCE_LOW}
 *
 * OUTPUT  : None.
 *
 * RETURN  :      NETCFG_TYPE_OK /
 *                      NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ND_SetNdRouterPreference(UI32_T vid_ifIndex, UI32_T  prefer);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ND_GetRunningNdRaLifetime
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ND route preference
 *
 * INPUT   : vid_ifIndex .
 *
 * OUTPUT  : UI8_T prefer
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ND_GetRunningNdRouterPreference(UI32_T vid_ifIndex, UI32_T*  prefer);
#endif /* #if (SYS_CPNT_IPV6_ROUTING == TRUE) */
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_PMGR_ND_RAGUARD_GetPortStatus
 * ------------------------------------------------------------------------
 * PURPOSE: To get RA Guard port status for specified lport.
 * INPUT  : lport       - which lport to get
 * OUTPUT : is_enable_p - pointer to output status
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETCFG_PMGR_ND_RAGUARD_GetPortStatus(
    UI32_T  lport,
    BOOL_T  *is_enable_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_PMGR_ND_RAGUARD_GetNextPortStatus
 * ------------------------------------------------------------------------
 * PURPOSE: To get RA Guard next port status for specified lport.
 * INPUT  : lport_p     - which lport to get next
 * OUTPUT : lport_p     - next lport
 *          is_enable_p - pointer to output status
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETCFG_PMGR_ND_RAGUARD_GetNextPortStatus(
    UI32_T  *lport_p,
    BOOL_T  *is_enable_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_PMGR_ND_RAGUARD_GetRunningPortStatus
 * ------------------------------------------------------------------------
 * PURPOSE: To get RA Guard running port status for specified lport.
 * INPUT  : lport       - which lport to get
 * OUTPUT : is_enable_p - pointer to output status
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_PMGR_ND_RAGUARD_GetRunningPortStatus(
    UI32_T  lport,
    BOOL_T  *is_enable_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_PMGR_ND_RAGUARD_SetPortStatus
 * ------------------------------------------------------------------------
 * PURPOSE: To set RA Guard port status for specified lport.
 * INPUT  : lport     - which lport to set
 *          is_enable - TRUE to enable
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETCFG_PMGR_ND_RAGUARD_SetPortStatus(
    UI32_T  lport,
    BOOL_T  is_enable);

#endif /* #if (SYS_CPNT_IPV6_RA_GUARD == TRUE) */

#endif /* #ifndef NETCFG_PMGR_ND_H */

