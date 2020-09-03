#ifndef _ND_MGR_H
#define _ND_MGR_H

#include <sys_type.h>
#include "sysfun.h"
#include "sys_type.h"
#include "netcfg_type.h"


#define ND_MGR_GET_FLAGS_IPV4  (1 << L_INET_ADDR_TYPE_IPV4) /* L_INET_ADDR_TYPE_IPV4*/ 
#define ND_MGR_GET_FLAGS_IPV6 (1 << L_INET_ADDR_TYPE_IPV6)

/* FUNCTION NAME : ND_MGR_InitiateProcessResources
 * PURPOSE: Initialize process resources for ARP_MGR
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
void ND_MGR_InitiateProcessResources(void);

/* FUNCTION NAME : ND_MGR_GetIpNetToMediaEntry
 * PURPOSE:
 *      Get an arp entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T     ND_MGR_GetIpNetToPhysicalEntry(NETCFG_TYPE_IpNetToPhysicalEntry_T *entry);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_AddStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Add a static ARP entry.
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
UI32_T ND_MGR_AddStaticIpNetToMediaEntry(UI32_T vid_ifIndex,
                                                            UI32_T ip_addr,
                                                            UI32_T phy_addr_len,
                                                            UI8_T *phy_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_DeleteStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete a static ARP entry.
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
UI32_T ND_MGR_DeleteStaticIpNetToMediaEntry(UI32_T vid_ifIndex, UI32_T ip_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetIpNetToMediaEntryTimeout
 *-----------------------------------------------------------------------------
 * PURPOSE : Set ARP timeout.
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
UI32_T ND_MGR_SetIpNetToMediaEntryTimeout(UI32_T age_time, UI32_T pre_timeout);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_DeleteAllDynamicIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete all dynamic ARP entries.
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
UI32_T ND_MGR_DeleteAllDynamicIpNetToMediaEntry(void);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_DeleteAllDynamicIpNetToPhysicalEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete all dynamic ARP entries.
 *
 * INPUT   : action flag: ND_MGR_GET_FLAGS_IPV4,  ND_MGR_GET_FLAGS_IPV6, or both
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ND_MGR_DeleteAllDynamicIpNetToPhysicalEntry(UI32_T actionflags);
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_GetNextIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Look up next ARP entry.
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
UI32_T ND_MGR_GetNextIpNetToMediaEntry(NETCFG_TYPE_IpNetToMediaEntry_T *entry);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_GetStatistic
 *-----------------------------------------------------------------------------
 * PURPOSE : Get ARP packet statistics.
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
UI32_T ND_MGR_GetStatistic(NETCFG_TYPE_IpNetToMedia_Statistics_T *stat);


/*Creat by Simon: wrap around NSM Router Advertisement CLI-APIs*/



/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_AddStaticIpNetToPhysicalEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : add a static IPV4/V6 neighbor entry.
 *
 * INPUT   : 
 *           vid_ifIndex
 *           ip_addr:  
 *           phy_addr_len
 *           phy_addr
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ND_MGR_AddStaticIpNetToPhysicalEntry(UI32_T vid_ifIndex,
                                                            L_INET_AddrIp_T*  ip_addr,
                                                            UI32_T phy_addr_len,
                                                            UI8_T *phy_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_DeleteStaticIpNetToPhysicalEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete a static IPV4/V6 neighbor entry.
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
UI32_T ND_MGR_DeleteStaticIpNetToPhysicalEntry(UI32_T vid_ifIndex, L_INET_AddrIp_T* ip_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_DeleteAllDynamicIpv6NetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete all dynamic ARP entries.
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
UI32_T ND_MGR_DeleteAllDynamicIpv6NetToMediaEntry(void);


/* FUNCTION NAME : ND_MGR_GetIpNetToMediaEntry
 * PURPOSE:
 *      Get an IPV6 neighbor entry from net_address
 *
 * INPUT:
 *      NETCFG_TYPE_Ipv6NetToMediaEntry_T entry:key is _net_address
 *
 * OUTPUT:
 *      NETCFG_TYPE_Ipv6NetToMediaEntry_T entry
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
//UI32_T     ND_MGR_GetIpv6NetToMediaEntry(NETCFG_TYPE_Ipv6NetToMediaEntry_T *entry);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_GetNextIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Look up next IPV6 neighbor entry.
 *
 * INPUT   : NETCFG_TYPE_IpNetToMediaEntry_T *entry. key is _net_address
 *              possible action_flags are ND_MGR_GET_FLAGS_IPV4 and ND_MGR_GET_FLAGS_IPV6
 * OUTPUT  : NETCFG_TYPE_IpNetToMediaEntry_T *entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ND_MGR_GetNextIpNetToPhysicalEntry(UI32_T action_flags,NETCFG_TYPE_IpNetToPhysicalEntry_T *entry);



#if (SYS_CPNT_IPV6 == TRUE)
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGRSetRaHopLimit
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the maximum number of hops used in router advertisements
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  UI32_T value: hop limit
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ND_MGR_SetHopLimit(  UI32_T  value);
UI32_T ND_MGR_UnsetHopLimit( );

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetDadAttempts
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the number of consecutive neighbor solicitation messages 
 *                  that are sent on an interface while duplicate address detected
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  UI32_T  value: retransmission value
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ND_MGR_SetNsDadAttempts( UI32_T vid_ifindex, UI32_T  value);
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetNsInterval
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the interval between IPv6 neighbor solicitation retransmissions
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  UI32_T  milliseconds: ns retransmission interval in milliseconds
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ND_MGR_SetNsInterval(UI32_T vid_ifindex, UI32_T  milliseconds);
UI32_T ND_MGR_UnsetNsInterval(UI32_T vid_ifindex);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetNdReachableTime
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the reachable-time
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  UI32_T  milliseconds: reachable time of neighboring nodesl in milliseconds
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ND_MGR_SetNdReachableTime(UI32_T vid_ifindex, UI32_T  milliseconds);
UI32_T ND_MGR_UnsetNdReachableTime(UI32_T vid_ifindex);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetRaLifetime
 *-----------------------------------------------------------------------------
 * PURPOSE : To configure the router lifetime value in IPv6 router advertisements on an interface,
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  UI32_T  seconds: The validity of this router as a default router on this interface (in seconds).
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ND_MGR_SetRaLifetime(UI32_T vid_ifindex, UI32_T  seconds);
UI32_T ND_MGR_UnsetRaLifetime(UI32_T vid_ifindex);
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetRaRouterPreference
 *-----------------------------------------------------------------------------
 * PURPOSE : configure a default router preference (DRP)
 *
 * INPUT   :        UI32_T vid_ifindex: vlan ifindex
 *                      UI32_T preference: on of
                            NETCFG_TYPE_ND_ROUTER_PERFERENCE_HIGH = 1,
                            NETCFG_TYPE_ND_ROUTER_PERFERENCE_MEDIUM=0,
                            NETCFG_TYPE_ND_ROUTER_PERFERENCE_LOW=3
 *                  
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ND_MGR_SetRaRouterPreference(UI32_T vid_ifindex, UI32_T  preference);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetRa
 *-----------------------------------------------------------------------------
 * PURPOSE : enable/disable  the sending of IPv6 router advertisement transmissions
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  BOOL_T enable_RA: when TRUE, enable RA transmission; when FALSE, suppress RA transmission
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
//UI32_T ND_MGR_SetRa(UI32_T vid_ifindex, BOOL_T enable_RA);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGRSetRaInterval
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the interval between router advertisement (RA) transmissions
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  UI32_T seconds: interval in second
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
//UI32_T ND_MGR_SetRaInterval(UI32_T vid_ifindex, UI32_T seconds);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGRUnsetRaInterval
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the interval between router advertisement (RA) transmissions as default
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
//UI32_T ND_MGR_UnsetRaInterval(UI32_T vid_ifindex);


/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGRSetRaLifetime
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the router lifetime value in IPv6 router advertisements on an interface
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  UI32_T seconds: lifetime in second
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
//UI32_T ND_MGR_SetRaLifetime(UI32_T vid_ifindex, UI32_T seconds);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGRUnsetRaLifetime
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the router lifetime value in IPv6 router advertisements  as default
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
//UI32_T ND_MGR_UnsetRaLifetime(UI32_T vid_ifindex);


/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGRSetRaReachableTime
 *-----------------------------------------------------------------------------
 * PURPOSE : configure the amount of time that a remote IPv6 node is considered reachable
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  UI32_T milliseconds: reachable time in millisecond
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
//UI32_T ND_MGR_SetRaReachableTime(UI32_T vid_ifindex, UI32_T milliseconds);
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGRUnsetRaReachableTime
 *-----------------------------------------------------------------------------
 * PURPOSE : configure RA reachable time as default
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
//UI32_T ND_MGR_UnsetRaReachableTime(UI32_T vid_ifindex);


/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetRaManagedConfigFlag
 *-----------------------------------------------------------------------------
 * PURPOSE :  set the "managed address configuration" flag in IPv6 router advertisements
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  BOOL_T enable_managed: when TRUE enable M flag, when FALSE disable M flag
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
//UI32_T ND_MGR_SetRaManagedConfigFlag(UI32_T vid_ifindex, BOOL_T enable_managed);


/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetRaOtherConfigFlag
 *-----------------------------------------------------------------------------
 * PURPOSE :  set the "other stateful configuration" flag in IPv6 router advertisements
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  BOOL_T enable_other: when TRUE enable O flag, when FALSE disable O flag
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
//UI32_T ND_MGR_SetRaOtherConfigFlag(UI32_T vid_ifindex, BOOL_T enable_other);



/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetRaPrefix
 *-----------------------------------------------------------------------------
 * PURPOSE :  configure which IPv6 prefixes are included in IPv6 router advertisements
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  L_INET_AddrIp_T* addr: IPV6 prefix that are included in RA. addr.addr and addr.prefix_length must not empty
 *                  UI32_T vlifetime : valid lifetime in second
 *                  UI32_T plifetime: preferred lifetime in second
 *                  BOOL_T enable_on_link: when TRUE set to on-link, when FALSE set to off-link
 *                  BOOL_T enable_autoconf: when TRUE set to autoconfig, when FALSE set to no-autoconfig
 *  
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
//UI32_T ND_MGR_SetRaPrefix(UI32_T vid_ifindex, L_INET_AddrIp_T* addr, UI32_T vlifetime, UI32_T plifetime, BOOL_T enable_on_link, BOOL_T enable_autoconf);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGRUnsetRaPrefix
 *-----------------------------------------------------------------------------
 * PURPOSE :  configure which IPv6 prefixes are included in IPv6 router advertisements as default
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                  L_INET_AddrIp_T* addr: IPV6 prefix that are included in RA. addr.addr and addr.prefix_length must not empty
 *  
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
//UI32_T ND_MGR_UnsetRaPrefix(UI32_T vid_ifindex,L_INET_AddrIp_T* addr);
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ND_MGR_SetRaRouterPreference
 *-----------------------------------------------------------------------------
 * PURPOSE :  set the "managed address configuration" flag in IPv6 router advertisements
 *
 * INPUT   :    UI32_T vid_ifindex: vlan ifindex
 *                 preference: high = 0x01, medium=0x00, low=0x03
 *
 * OUTPUT  : None.
 *
 * RETURN  : NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
//UI32_T ND_MGR_SetRaRouterPreference(UI32_T vid_ifindex, UI8_T preference);

void ND_MGR_BackdoorSetDebugFlag(UI32_T flag);


#endif
