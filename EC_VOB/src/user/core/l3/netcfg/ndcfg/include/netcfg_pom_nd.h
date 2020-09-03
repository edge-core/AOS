/* MODULE NAME:  NETCFG_POM_ND.h
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to access NETCFG_OM_ARP service.
 *    In Linux platform, the communication between CSC group are done via IPC.
 *    Other CSC can call NETCFG_POM_XXX for APIs NETCFG_OM_XXX provided by NETCFG, and same as NETCFG_PMGR_ARP for
 *    NETCFG_MGR_ARP APIs
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    2008/01/18     --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 */
 
#ifndef NETCFG_POM_ND_H
#define NETCFG_POM_ND_H

#include "sys_type.h"
#include "netcfg_type.h"
#include "netcfg_om_nd.h"

/* FUNCTION NAME : NETCFG_POM_ND_InitiateProcessResource
 * PURPOSE:
 *    Initiate resource for CSCA_POM in the calling process.
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
 *    Before other CSC use NETCFG_POM_ND, it should initiate the resource (get the message queue handler internally)
 
 *
 */
BOOL_T NETCFG_POM_ND_InitiateProcessResource(void);

/* FUNCTION NAME : NETCFG_POM_ND_GetIpNetToMediaTimeout
 * PURPOSE:
 *    Get ARP timeout
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    age_time.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_POM_ND_GetIpNetToMediaTimeout(UI32_T *age_time);

/* FUNCTION NAME : NETCFG_POM_ND_GetRunningIpNetToMediaTimeout
 * PURPOSE:
 *    Get running config of ARP timeout
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    age_time.
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_ND_GetRunningIpNetToMediaTimeout(UI32_T *age_time);

/* FUNCTION NAME : NETCFG_POM_ND_GetRouterRedundancyProtocolIpNetToPhysicalEntry
 * PURPOSE:
 *      Get a VRRP/HSRP ARP entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is entry->ip_addr.
 */
BOOL_T NETCFG_POM_ND_GetRouterRedundancyProtocolIpNetToPhysicalEntry(NETCFG_OM_ND_RouterRedundancyEntry_T *entry);

/* FUNCTION NAME : NETCFG_OM_ND_GetNextStaticEntry
 * PURPOSE:
 *      Get next available static arp entry.
 *
 * INPUT:
 *      entry
 *      addr_type: must either L_INET_ADDR_TYPE_IPV4 or L_INET_ADDR_TYPE_IPV6
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_if_index, entry.ip_net_to_media_net_address).
 *      If key is (0, 0), get first one.
 */
BOOL_T NETCFG_POM_ND_GetNextStaticEntry(UI8_T addr_type, NETCFG_TYPE_StaticIpNetToPhysicalEntry_T *entry);


#ifdef SYS_CPNT_IPV6

/* FUNCTION NAME : NETCFG_POM_ND_GetNdDADAttempts
 * PURPOSE:
 *    Get DAD ( duplicate address detection) attempts 
 * INPUT:
 *    attempts.
 *
 * OUTPUT:
 *    attempts.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_POM_ND_GetNdDADAttempts(UI32_T vid_ifIndex, UI32_T *attempts);

/* FUNCTION NAME : NETCFG_POM_ND_GetRunningDADAttempts
 * PURPOSE:
 *    Get running DAD ( duplicate address detection) attempts 
 * INPUT:
 *    vid_ifindex.
 *
 * OUTPUT:
 *    attempts.
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_ND_GetRunningDADAttempts(UI32_T vid_ifindex, UI32_T* attempts);

/* FUNCTION NAME : NETCFG_POM_ND_GetNdNsInterval
 * PURPOSE:
 *    get  the interval between IPv6 neighbor solicitation retransmissions on an interface 
 * INPUT:
 *    vid_ifIndex
 *    msec.
 *
 * OUTPUT:
 *    msec.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_POM_ND_GetNdNsInterval(UI32_T vid_ifIndex, UI32_T *msec);

/* FUNCTION NAME : NETCFG_POM_ND_GetRunningNdNsInterval
 * PURPOSE:
 *    get running config of the NS interval on an interface
 * INPUT:
 *    vid_ifIndex
 *
 * OUTPUT:
 *    msec.
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_ND_GetRunningNdNsInterval(UI32_T vid_ifindex, UI32_T* msec);


/* FUNCTION NAME : NETCFG_POM_ND_GetNdHoplimit
 * PURPOSE:
 *    get  the maximum number of hops used in router advertisements 
 * INPUT:
 *    vid_ifIndex
 *    hoplimit.
 *
 * OUTPUT:
 *    hoplimit.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_POM_ND_GetNdHoplimit(UI32_T vid_ifindex,UI32_T *hoplimit);

/* FUNCTION NAME : NETCFG_POM_ND_GetNdPrefix
 * PURPOSE:
 *    get  which IPv6 prefixes are included in IPv6 router advertisements
 * INPUT:
 *     L_INET_AddrIp_T: prefix
 *    UI32_T vid_ifIndex ,  validLifetime, preferredLifetime
 *    BOOL_T: offLink. noAutoconfig
 *
 * OUTPUT:
 *     L_INET_AddrIp_T: prefix
 *    UI32_T  validLifetime, preferredLifetime
 *    BOOL_T: offLink. noAutoconfig
 *
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_POM_ND_GetNdPrefix(UI32_T vid_ifIndex, L_INET_AddrIp_T *prefix, UI32_T *validLifetime, UI32_T *preferredLifetime,BOOL_T *offLink,BOOL_T *noAutoconfig);
BOOL_T NETCFG_POM_ND_GetNdDefaultPrefixConfig( UI32_T *validLifetime, UI32_T *preferredLifetime,BOOL_T *enable_onlink,BOOL_T *enable_autoconf);

/* FUNCTION NAME : NETCFG_POM_ND_GetNdManagedConfigFlag
 * PURPOSE:
 *    get    the "managed address configuration" flag in IPv6 router advertisements
 * INPUT:
 *    vid_ifIndex
 *    enableFlag.
 *
 * OUTPUT:
 *    enableFlag.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_POM_ND_GetNdManagedConfigFlag(UI32_T vid_ifIndex, BOOL_T *enableFlag);

/* FUNCTION NAME : NETCFG_POM_ND_GetNdOtherConfigFlag
 * PURPOSE:
 *    get    the "other stateful configuration" flag in IPv6 router advertisements
 * INPUT:
 *    vid_ifIndex
 *    enableFlag.
 *
 * OUTPUT:
 *    enableFlag.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_POM_ND_GetNdOtherConfigFlag(UI32_T vid_ifIndex, BOOL_T *enableFlag);

/* FUNCTION NAME : NETCFG_POM_ND_GetNdReachableTime
 * PURPOSE:
 *    get    the amount of time that a remote IPv6 node is considered reachable  
 *                  after some reachability confirmation event has occurred
 * INPUT:
 *    vid_ifIndex
 *    msec.
 *
 * OUTPUT:
 *    msec.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_POM_ND_GetNdReachableTime(UI32_T vid_ifIndex,UI32_T *msec);


/* FUNCTION NAME : NETCFG_POM_ND_GetRunningNdReachableTime
 * PURPOSE:
 *    get running config of reachable time on an interface
 * INPUT:
 *    vid_ifIndex
 *
 * OUTPUT:
 *    msec.
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_ND_GetRunningNdReachableTime(UI32_T vid_ifindex, UI32_T* msec);


/* FUNCTION NAME : NETCFG_POM_ND_GetNdRaSuppress
 * PURPOSE:
 *    get whether suppress  IPv6 router advertisement transmissions
 * INPUT:
 *    vid_ifIndex
 *    enableSuppress.
 *
 * OUTPUT:
 *    enableSuppress.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_POM_ND_GetNdRaSuppress(UI32_T vid_ifIndex, BOOL_T *enableSuppress);

/* FUNCTION NAME : NETCFG_POM_ND_GetNdRaLifetime
 * PURPOSE:
 *    get the router lifetime value in IPv6 router advertisements 
 * INPUT:
 *    vid_ifIndex
 *    seconds.
 *
 * OUTPUT:
 *    seconds.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_POM_ND_GetNdRaLifetime(UI32_T vid_ifIndex, UI32_T *seconds);

/* FUNCTION NAME : NETCFG_POM_ND_GetNdRaInterval
 * PURPOSE:
 *    get the interval between IPv6 router advertisement  transmissions
 * INPUT:
 *    vid_ifindex -- vlan ifindex
 *
 * OUTPUT:
 *    max_p  -- max ra interval
 *    min_p  -- min ra interval
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_POM_ND_GetNdRaInterval(UI32_T vid_ifindex, UI32_T *max_p, UI32_T *min_p);

/* FUNCTION NAME : NETCFG_POM_ND_GetNdRouterPreference
 * PURPOSE:
 *    get the the Preference flag in IPv6 router advertisements
 * INPUT:
 *    vid_ifIndex
 *    prefer.
 *
 * OUTPUT:
 *    prefer.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_POM_ND_GetNdRouterPreference(UI32_T vid_ifindex, UI32_T  *prefer);
BOOL_T NETCFG_POM_ND_IsConfigFlagSet(UI32_T vid_ifindex, UI32_T flag, BOOL_T *is_set);

#endif /*SYS_CPNT_IPV6*/

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
/* FUNCTION NAME : NETCFG_OM_ND_RAGUARD_IsEnabled
 * PURPOSE:
 *    To check if RA Guard is enabled for specifed lport.
 * INPUT:
 *    lport    - which lport to check (1-based)
 *    pkt_type - which packet type received,
 *               NETCFG_TYPE_RG_PKT_MAX to skip statistics
 *               (NETCFG_TYPE_RG_PKT_RA/NETCFG_TYPE_RG_PKT_RR)
 * OUTPUT:
 *
 * RETURN:
 *    TRUE  --  Success
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_POM_ND_RAGUARD_IsEnabled(
    UI32_T  lport,
    UI32_T  pkt_type);

/* FUNCTION NAME : NETCFG_POM_ND_RAGUARD_IsAnyPortEnabled
 * PURPOSE:
 *    To check if RA Guard is enabled for any port.
 * INPUT:
 *    None.
 * OUTPUT:
 *    None.
 * RETURN:
 *    TRUE  --  Success
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_POM_ND_RAGUARD_IsAnyPortEnabled(void);
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD == TRUE) */

void NETCFG_POM_ND_ConvertIpNetToMediaToIpNetToPhysical (NETCFG_TYPE_IpNetToPhysicalEntry_T* output ,NETCFG_TYPE_IpNetToMediaEntry_T* input);
void NETCFG_POM_ND_ConvertIpNetToPhysicalToIpNetToMedia (NETCFG_TYPE_IpNetToMediaEntry_T* output ,   NETCFG_TYPE_IpNetToPhysicalEntry_T* input);
void NETCFG_POM_ND_ConvertIpv6NetToMediaToIpNetToPhysical (NETCFG_TYPE_IpNetToPhysicalEntry_T* output ,NETCFG_TYPE_Ipv6NetToMediaEntry_T* input);
void NETCFG_POM_ND_ConvertIpNetToPhysicalToIpv6NetToMedia (NETCFG_TYPE_Ipv6NetToMediaEntry_T* output ,   NETCFG_TYPE_IpNetToPhysicalEntry_T* input);


#endif /*NETCFG_POM_ND_H*/

