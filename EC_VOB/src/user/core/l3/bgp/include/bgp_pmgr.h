/* MODULE NAME:  bgp_pmgr.h
 * PURPOSE:
 *     BGP PMGR APIs.
 *
 * NOTES:
 *
 * HISTORY
 *    02/14/2011 - Peter Yu, Created
 *    03/11/2011 - KH Shi, Add and Modify APIs
 *
 * Copyright(C)      Edge-Core Corporation, 2011
 */
#ifndef BGP_PMGR_H
#define BGP_PMGR_H

#include "sys_type.h"
#include "bgp_mgr.h"
#include "bgp_type.h"

/* FUNCTION NAME : BGP_PMGR_InitiateProcessResource
* PURPOSE:
*      Initiate process resources for BGP_PMGR.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE   -- Success
*      FALSE  -- Fail
*
* NOTES:
*      None.
*/
BOOL_T BGP_PMGR_InitiateProcessResource(void);

/* policy */
UI32_T BGP_PMGR_AddIpPrefixList(char plist_name[BGP_TYPE_PREFIX_LIST_NAME_LEN+1], UI32_T afi);
UI32_T BGP_PMGR_DeleteIpPrefixList(char plist_name[BGP_TYPE_PREFIX_LIST_NAME_LEN+1], UI32_T afi);
UI32_T BGP_PMGR_GetNextIpPrefixList(NSM_POLICY_TYPE_PrefixList_T *prefix_list_info_p);
UI32_T BGP_PMGR_AddIpPrefixListEntry(char plist_name[BGP_TYPE_PREFIX_LIST_NAME_LEN+1], UI32_T afi, UI32_T seq_num, BOOL_T is_permit, L_INET_AddrIp_T *prefix_p, UI32_T ge_num, UI32_T le_num);
UI32_T BGP_PMGR_DeleteIpPrefixListEntry(char plist_name[BGP_TYPE_PREFIX_LIST_NAME_LEN+1], UI32_T afi, UI32_T seq_num, BOOL_T is_permit, L_INET_AddrIp_T *prefix_p, UI32_T ge_num, UI32_T le_num);
UI32_T BGP_PMGR_GetNextIpPrefixListEntry(NSM_POLICY_TYPE_PrefixListEntry_T *prefix_list_entry_info_p);
UI32_T BGP_PMGR_AddIpAsPathList(char as_list[BGP_TYPE_AS_LIST_NAME_LEN+1], BOOL_T is_permit, char reg_exp[BGP_TYPE_REGULAR_EXP_LEN+1]);
UI32_T BGP_PMGR_DeleteIpAsPathList(char as_list[BGP_TYPE_AS_LIST_NAME_LEN+1], BOOL_T is_permit, char reg_exp[BGP_TYPE_REGULAR_EXP_LEN+1]);
UI32_T BGP_PMGR_GetNextIpAsPathList(BGP_TYPE_AsList_T *as_list_p);
UI32_T BGP_PMGR_GetNextIpAsPathListEntry(BGP_TYPE_AsFilter_T *as_filter_p);
UI32_T BGP_PMGR_AddIpCommunityList(char commu_name[BGP_TYPE_COMMUNITY_NAME_LEN+1], BOOL_T is_permit, UI32_T style, char commu_and_regexp[BGP_TYPE_REGULAR_EXP_LEN+1], BOOL_T reject_all_digit_name);
UI32_T BGP_PMGR_DeleteIpCommunityList(char commu_name[BGP_TYPE_COMMUNITY_NAME_LEN+1], BOOL_T is_permit, UI32_T style, char commu_and_regexp[BGP_TYPE_REGULAR_EXP_LEN+1]);
UI32_T BGP_PMGR_AddIpExtCommunityList(char ext_commu_name[BGP_TYPE_COMMUNITY_NAME_LEN+1], BOOL_T is_permit, UI32_T style, char ext_commu_and_regexp[BGP_TYPE_REGULAR_EXP_LEN+1], BOOL_T reject_all_digit_name);
UI32_T BGP_PMGR_GetNextIpCommunityList(BGP_TYPE_CommunityList_T *list_p);
UI32_T BGP_PMGR_GetNextIpCommunityListEntry(BGP_TYPE_CommunityEntry_T *entry_p);
UI32_T BGP_PMGR_DeleteIpExtCommunityList(char ext_commu_name[BGP_TYPE_COMMUNITY_NAME_LEN+1], BOOL_T is_permit, UI32_T style, char ext_commu_and_regexp[BGP_TYPE_REGULAR_EXP_LEN+1]);
UI32_T BGP_PMGR_GetNextIpExtCommunityList(BGP_TYPE_CommunityList_T *list_p);
UI32_T BGP_PMGR_GetNextIpExtCommunityListEntry(BGP_TYPE_CommunityEntry_T *entry_p);
UI32_T BGP_PMGR_AddRouteMap(char rmap_name[BGP_TYPE_ROUTE_MAP_NAME_LEN+1]);
UI32_T BGP_PMGR_DeleteRouteMap(char rmap_name[BGP_TYPE_ROUTE_MAP_NAME_LEN+1]);
UI32_T BGP_PMGR_GetRouteMap(NSM_POLICY_TYPE_RouteMap_T *rmap_info_p);
UI32_T BGP_PMGR_GetNextRouteMap(NSM_POLICY_TYPE_RouteMap_T *rmap_info_p);
UI32_T BGP_PMGR_AddRouteMapPref(char rmap_name[BGP_TYPE_ROUTE_MAP_NAME_LEN+1], BOOL_T is_permit, UI32_T pref_index);
UI32_T BGP_PMGR_DeleteRouteMapPref(char rmap_name[BGP_TYPE_ROUTE_MAP_NAME_LEN+1], BOOL_T is_permit, UI32_T pref_index);
UI32_T BGP_PMGR_GetRouteMapIndex(NSM_POLICY_TYPE_RouteMapIndex_T *rmap_index_p);
UI32_T BGP_PMGR_GetNextRouteMapIndex(NSM_POLICY_TYPE_RouteMapIndex_T *rmap_index_p);
UI32_T BGP_PMGR_AddRouteMapMatch(char rmap_name[BGP_TYPE_ROUTE_MAP_NAME_LEN+1], BOOL_T is_permit, UI32_T pref_index, char command[BGP_TYPE_ROUTE_MAP_COMMAND_LEN+1], char arg[BGP_TYPE_ROUTE_MAP_ARG_LEN+1]);
UI32_T BGP_PMGR_DeleteRouteMapMatch(char rmap_name[BGP_TYPE_ROUTE_MAP_NAME_LEN+1], BOOL_T is_permit, UI32_T pref_index, char command[BGP_TYPE_ROUTE_MAP_COMMAND_LEN+1], char arg[BGP_TYPE_ROUTE_MAP_ARG_LEN+1]);
UI32_T BGP_PMGR_GetNextRouteMapMatch(NSM_POLICY_TYPE_RouteMapRule_T *rmap_rule_p);
UI32_T BGP_PMGR_AddRouteMapSet(char rmap_name[BGP_TYPE_ROUTE_MAP_NAME_LEN+1], BOOL_T is_permit, UI32_T pref_index, char command[BGP_TYPE_ROUTE_MAP_COMMAND_LEN+1], char arg[BGP_TYPE_ROUTE_MAP_ARG_LEN+1]);
UI32_T BGP_PMGR_DeleteRouteMapSet(char rmap_name[BGP_TYPE_ROUTE_MAP_NAME_LEN+1], BOOL_T is_permit, UI32_T pref_index, char command[BGP_TYPE_ROUTE_MAP_COMMAND_LEN+1], char arg[BGP_TYPE_ROUTE_MAP_ARG_LEN+1]);
UI32_T BGP_PMGR_GetNextRouteMapSet(NSM_POLICY_TYPE_RouteMapRule_T *rmap_rule_p);

UI32_T BGP_PMGR_SetRouteMapCall(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index, char call_rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1]);
UI32_T BGP_PMGR_UnsetRouteMapCall(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index);
UI32_T BGP_PMGR_SetRouteMapDescription(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index, char desc[NSM_POLICY_TYPE_ROUTE_MAP_DESCRIPTION_LEN +1]);
UI32_T BGP_PMGR_UnsetRouteMapDescription(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index);
UI32_T BGP_PMGR_SetRouteMapOnMatchNext(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index);
UI32_T BGP_PMGR_UnsetRouteMapOnMatchNext(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index);
UI32_T BGP_PMGR_SetRouteMapOnMatchGoto(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index, UI32_T clause_number);
UI32_T BGP_PMGR_UnsetRouteMapOnMatchGoto(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index);

/* interface and rif update*/
UI32_T BGP_PMGR_SignalL3IfCreate(UI32_T ifindex);
UI32_T BGP_PMGR_SignalL3IfDestroy(UI32_T ifindex);
UI32_T BGP_PMGR_SignalL3IfUp(UI32_T ifindex);
UI32_T BGP_PMGR_SignalL3IfDown(UI32_T ifindex);
UI32_T BGP_PMGR_SignalL3IfRifCreate(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p);
UI32_T BGP_PMGR_SignalL3IfRifDestroy(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p);
UI32_T BGP_PMGR_SignalL3IfRifUp(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p);
UI32_T BGP_PMGR_SignalL3IfRifDown(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p);

/* FUNCTION NAME : BGP_PMGR_AddAggregateAddress
* PURPOSE:
*     Set receive packet on an interface.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*      BGP_TYPE_RESULT_SUCCESS/BGP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T BGP_PMGR_AddAggregateAddress(UI32_T as_number, UI32_T afi, UI32_T safi, L_INET_AddrIp_T *ipaddr_p, BOOL_T is_summary_only, BOOL_T is_as_set);
UI32_T BGP_PMGR_DeleteAggregateAddress(UI32_T as_number, UI32_T afi, UI32_T safi, L_INET_AddrIp_T *ipaddr_p);
UI32_T BGP_PMGR_UnsetAggregateAddress(UI32_T as_number, UI32_T afi, UI32_T safi, L_INET_AddrIp_T *ipaddr_p, BOOL_T is_summary_only, BOOL_T is_as_set);

UI32_T BGP_PMGR_SetFlag(UI32_T as_number, UI32_T flag);
UI32_T BGP_PMGR_UnsetFlag(UI32_T as_number, UI32_T flag);


//use bgp flag to set, UI32_T BGP_PMGR_SetBestPathMED(UI32_T as_number, BOOL_T confed, BOOL_T missing_as_worst);
//UI32_T BGP_PMGR_UnsetBestPathMED(UI32_T as_number, BOOL_T confed, BOOL_T missing_as_worst);


UI32_T BGP_PMGR_SetClusterId(UI32_T as_number, UI32_T cluster_id, UI32_T format);
UI32_T BGP_PMGR_UnsetClusterId(UI32_T as_number);

UI32_T BGP_PMGR_SetConfederationId(UI32_T as_number, UI32_T confederation_id);
UI32_T BGP_PMGR_UnsetConfederationId(UI32_T as_number);

UI32_T BGP_PMGR_AddConfederationPeer(UI32_T as_number, UI32_T peer_as_id);
UI32_T BGP_PMGR_DeleteConfederationPeer(UI32_T as_number, UI32_T peer_as_id);

UI32_T BGP_PMGR_EnableDampening(UI32_T as_number, UI32_T afi, UI32_T safi, UI32_T half_life, UI32_T reuse_limit, UI32_T suppress_limit, UI32_T max_suppress_time);
UI32_T BGP_PMGR_DisableDampening(UI32_T as_number, UI32_T afi, UI32_T safi);

UI32_T BGP_PMGR_SetDefaultLocalPreference(UI32_T as_number, UI32_T preference);
UI32_T BGP_PMGR_UnsetDefaultLocalPreference(UI32_T as_number);

UI32_T BGP_PMGR_SetRouterId(UI32_T as_number, UI32_T router_id);
UI32_T BGP_PMGR_UnsetRouterId(UI32_T as_number);

UI32_T BGP_PMGR_SetScanTime(UI32_T as_number, UI32_T scan_time);
UI32_T BGP_PMGR_UnsetScanTime(UI32_T as_number);

/* clear_sort: enum BGP_TYPE_CLEAR_SORT, clear_type: enum BGP_TYPE_CLEAR_TYPE */
UI32_T BGP_PMGR_ClearBgp(UI32_T as_number, UI32_T afi, UI32_T safi, UI32_T clear_sort, UI32_T clear_type, char arg[BGP_TYPE_CLEAR_BGP_ARG_STR_LEN+1]);

UI32_T BGP_PMGR_ClearDampening(UI32_T as_number, UI32_T afi, UI32_T safi, L_INET_AddrIp_T *ipaddr_p);

UI32_T BGP_PMGR_SetDistance(UI32_T as_number, UI32_T distance, L_INET_AddrIp_T *ipaddr_p, char access_list_str[BGP_TYPE_ACCESS_LIST_NAME_LEN+1]);
UI32_T BGP_PMGR_UnsetDistance(UI32_T as_number, L_INET_AddrIp_T *ipaddr_p);

UI32_T BGP_PMGR_SetDistanceBgp(UI32_T as_number, UI32_T distance_ebgp, UI32_T distance_ibgp, UI32_T distance_local);
UI32_T BGP_PMGR_UnsetDistanceBgp(UI32_T as_number);

UI32_T BGP_PMGR_AddNetwork(UI32_T as_number, UI32_T afi, UI32_T safi, L_INET_AddrIp_T *network_addr_p, char route_map[BGP_TYPE_ROUTE_MAP_NAME_LEN+1], UI32_T backdoor, UI8_T ttl);
UI32_T BGP_PMGR_DeleteNetwork(UI32_T as_number, UI32_T afi, UI32_T safi, L_INET_AddrIp_T *network_addr_p);

UI32_T BGP_PMGR_SetRedistribute(UI32_T as_number, UI32_T afi, UI32_T type);
UI32_T BGP_PMGR_UnsetRedistribute(UI32_T as_number, UI32_T afi, UI32_T type);
UI32_T BGP_PMGR_SetRedistributeRouteMap(UI32_T as_number, UI32_T afi, UI32_T type, char route_map[BGP_TYPE_ROUTE_MAP_NAME_LEN+1]);
UI32_T BGP_PMGR_UnsetRedistributeRouteMap(UI32_T as_number, UI32_T afi, UI32_T type);
UI32_T BGP_PMGR_SetRedistributeMetric(UI32_T as_number, UI32_T afi, UI32_T type, UI32_T metric);
UI32_T BGP_PMGR_UnsetRedistributeMetric(UI32_T as_number, UI32_T afi, UI32_T type);

UI32_T BGP_PMGR_EnableBgpInstance(UI32_T as_number);
UI32_T BGP_PMGR_DisableBgpInstance(UI32_T as_number);

UI32_T BGP_PMGR_SetKeepAliveAndHoldTime(UI32_T as_number, UI32_T keepalive, UI32_T holdtime);
UI32_T BGP_PMGR_UnsetKeepAliveAndHoldTime(UI32_T as_number);

/*---------- neighbor ------------*/
UI32_T BGP_PMGR_ActivateNeighbor(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1]);
UI32_T BGP_PMGR_DeactivateNeighbor(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1]);

UI32_T BGP_PMGR_SetNeighborAdvertisementInterval(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T interval);
UI32_T BGP_PMGR_UnsetNeighborAdvertisementInterval(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1]);

UI32_T BGP_PMGR_SetNeighborAllowasIn(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T allow_num);
UI32_T BGP_PMGR_UnsetNeighborAllowasIn(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1]);

/*  BGP_TYPE_NEIGHBOR_AF_FLAG_XXXXX_UNCHANGED */
UI32_T BGP_PMGR_SetNeighborAttributeUnchanged(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T flag);
UI32_T BGP_PMGR_UnsetNeighborAttributeUnchanged(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T flag);

/* BGP_TYPE_NEIGHBOR_FLAG_DYNAMIC_CAPABILITY, route refresh always enabled in quagga */
UI32_T BGP_PMGR_SetNeighborCapability(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T flag);
UI32_T BGP_PMGR_UnsetNeighborCapability(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T flag); 

/* BGP_TYPE_NEIGHBOR_AF_FLAG_ORF_PREFIX_SM, BGP_TYPE_NEIGHBOR_AF_FLAG_ORF_PREFIX_RM */
UI32_T BGP_PMGR_SetNeighborCapabilityOrfPrefixList(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T flag);
UI32_T BGP_PMGR_UnsetNeighborCapabilityOrfPrefixList(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T flag);

UI32_T BGP_PMGR_SetNeighborDefaultOriginate(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], char route_map[BGP_TYPE_ROUTE_MAP_NAME_LEN+1]);
UI32_T BGP_PMGR_UnsetNeighborDefaultOriginate(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1]);

UI32_T BGP_PMGR_SetNeighborDescription(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], char description[BGP_TYPE_NEIGHBOR_DESC_LEN+1]);
UI32_T BGP_PMGR_UnsetNeighborDescription(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1]);

UI32_T BGP_PMGR_SetNeighborDistributeList(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], char access_list[BGP_TYPE_ACCESS_LIST_NAME_LEN+1], UI32_T direct);
UI32_T BGP_PMGR_UnsetNeighborDistributeList(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T direct);

UI32_T BGP_PMGR_SetNeighborFlag(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T flag);
UI32_T BGP_PMGR_UnsetNeighborFlag(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T flag);

UI32_T BGP_PMGR_SetNeighborAfFlag(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T flag);
UI32_T BGP_PMGR_UnsetNeighborAfFlag(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T flag);

UI32_T BGP_PMGR_SetNeighborEbgpMultihop(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T hop_count);
UI32_T BGP_PMGR_UnsetNeighborEbgpMultihop(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1]);

UI32_T BGP_PMGR_SetNeighborFilterList(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], char as_list[BGP_TYPE_AS_LIST_NAME_LEN+1], UI32_T direct);
UI32_T BGP_PMGR_UnsetNeighborFilterList(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T direct);

UI32_T BGP_PMGR_SetNeighborInterface(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T ifindex);
UI32_T BGP_PMGR_UnsetNeighborInterface(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1]);

UI32_T BGP_PMGR_SetNeighborMaximumPrefix(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T max_prefix, UI8_T threshold, UI32_T warn_only, UI16_T restart_interval);
UI32_T BGP_PMGR_UnsetNeighborMaximumPrefix(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1]);

UI32_T BGP_PMGR_AddNeighborPeerGroup(UI32_T as_number, char group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1]);
UI32_T BGP_PMGR_DeleteNeighborPeerGroup(UI32_T as_number, char group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1]);

UI32_T BGP_PMGR_AddNeighborPeerGroupMember(UI32_T as_number, UI32_T afi, UI32_T safi, char group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1], L_INET_AddrIp_T *neighbor_ip_p);
UI32_T BGP_PMGR_DeleteNeighborPeerGroupMember(UI32_T as_number, UI32_T afi, UI32_T safi, char group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1], L_INET_AddrIp_T *neighbor_ip_p);

UI32_T BGP_PMGR_SetNeighborPort(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T port_number);
UI32_T BGP_PMGR_UnsetNeighborPort(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1]);

UI32_T BGP_PMGR_SetNeighborPrefixList(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], char prefix_list[BGP_TYPE_PREFIX_LIST_NAME_LEN+1], UI32_T direct);
UI32_T BGP_PMGR_UnsetNeighborPrefixList(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T direct);

UI32_T BGP_PMGR_SetNeighborRemoteAs(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T neighbor_as_number);
UI32_T BGP_PMGR_UnsetNeighborRemoteAs(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1]);

UI32_T BGP_PMGR_SetNeighborRouteMap(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], char route_map[BGP_TYPE_ROUTE_MAP_NAME_LEN+1], UI32_T direct);
UI32_T BGP_PMGR_UnsetNeighborRouteMap(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T direct);

UI32_T BGP_PMGR_SetNeighborRouteServerClient(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1]);
UI32_T BGP_PMGR_UnsetNeighborRouteServerClient(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1]);

UI32_T BGP_PMGR_SetNeighborKeepAliveAndHoldTime(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T keepalive, UI32_T holdtime);
UI32_T BGP_PMGR_UnsetNeighborKeepAliveAndHoldTime(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1]);

UI32_T BGP_PMGR_SetNeighborConnectRetryInterval(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T interval);
UI32_T BGP_PMGR_UnsetNeighborConnectRetryInterval(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1]);

UI32_T BGP_PMGR_SetNeighborUnsuppressMap(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], char route_map[BGP_TYPE_ROUTE_MAP_NAME_LEN+1]);
UI32_T BGP_PMGR_UnsetNeighborUnsuppressMap(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1]);

UI32_T BGP_PMGR_SetNeighborUpdateSource(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T ifindex);
UI32_T BGP_PMGR_UnsetNeighborUpdateSource(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1]);

UI32_T BGP_PMGR_SetNeighborWeight(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T weight);
UI32_T BGP_PMGR_UnsetNeighborWeight(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1]);

UI32_T BGP_PMGR_SetNeighborPassword(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], char password[BGP_TYPE_PEER_PASSWORD_LEN+1]);
UI32_T BGP_PMGR_UnsetNeighborPassowrd(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1]);


/* BGP route print out function. */
UI32_T BGP_PMGR_ShowBGP(UI32_T as_number, UI32_T afi, UI32_T safi);
UI32_T BGP_PMGR_ShowBGPSummary(UI32_T as_number, UI32_T afi, UI32_T safi);
UI32_T BGP_PMGR_ShowNeighbor(UI32_T as_number, UI32_T is_all, char *peer_str);
UI32_T BGP_PMGR_ShowAttributeInfo(UI32_T as_number);

/* get function */
UI32_T BGP_PMGR_GetBgpInstance(BGP_TYPE_Instance_T *instance_p);
UI32_T BGP_PMGR_GetNextBgpInstance(BGP_TYPE_Instance_T *instance_p);

UI32_T BGP_PMGR_GetNextNeighborByAsNumber(BGP_TYPE_Neighbor_T *neighbor_p);
UI32_T BGP_PMGR_GetNextFilterRoute(UI32_T as_number, UI32_T afi, UI32_T safi, UI32_T router_id,
	  enum BGP_TYPE_SHOW_TYPE type, char *arg_str,
	  L_INET_AddrIp_T *out_p,
      BGP_TYPE_BgpInfo_T *binfo_p);

UI32_T BGP_PMGR_GetNextDataByType(BGP_TYPE_DataUnion_T *data_union_p);
UI32_T BGP_PMGR_GetNextMatchRouteInfo
(
    UI32_T as_number, 
    UI32_T afi, 
    UI32_T safi,
	L_INET_AddrIp_T *prefix_p,
    BGP_TYPE_BgpInfo_T *binfo_p, 
    BOOL_T prefix_check
);

UI32_T BGP_PMGR_GetNextAdvertiseNeighborAddrByPrefix(UI32_T as_number, UI32_T afi, UI32_T safi, L_INET_AddrIp_T *neighbor_addr_p, L_INET_AddrIp_T *prefix_p);
UI32_T BGP_PMGR_GetNextClusterId(UI32_T as_number, UI32_T afi, UI32_T safi, L_INET_AddrIp_T *prefix_p, BGP_TYPE_BgpInfo_T *binfo_p, UI32_T *cluster_id_p);
UI32_T BGP_PMGR_GetScanInfo(BOOL_T *is_running, UI32_T *scan_interval);
UI32_T BGP_PMGR_GetNextNexthopCache(UI32_T as_number, UI32_T afi, L_INET_AddrIp_T *prefix_p, BGP_TYPE_BgpNexthopCache_T *bnc_p);
UI32_T BGP_PMGR_GetNextConnectedRoute(UI32_T as_number, UI32_T afi, L_INET_AddrIp_T *prefix_p);
UI32_T BGP_PMGR_GetDampeningParameters(BGP_TYPE_DampeningParameters_T *param_p);
/* FUNCTION NAME : BGP_PMGR_GetNextPrefix
 * PURPOSE:
 *      Get BGP rib route by AFI/SAFI.
 *
 * INPUT:
 *      as_number       -- as_number
 *      afi             -- AFI
 *      safi            -- SAFI
 * OUTPUT:
 *      prefix_p        -- prefix
 *
 * RETURN:
 *      BGP_TYPE_RESULT_OK/BGP_TYPE_RESULT_FAIL
 *
 * NOTES:
 *      For command: "show ip bgp neighbor A.B.C.D advertised-routes/received-routes"
 */
UI32_T BGP_PMGR_GetNextPrefix(UI32_T as_number, UI32_T afi, UI32_T safi, L_INET_AddrIp_T *prefix_p);

/* FUNCTION NAME : BGP_PMGR_GetNextAdjAttrByPrefix
 * PURPOSE:
 *      Get neighbor and attribute info. of the (received/advertised) prefix.
 *
 * INPUT:
 *      as_number       -- as_number
 *      afi             -- AFI
 *      safi            -- SAFI
 *      in              -- in/out (received/advertised)
 *      prefix_p        -- prefix
 *      index_p         -- index of ain/adj
 * OUTPUT:
 *      index_p         -- index of ain/adj
 *      neighbor_addr_p -- neighbor address
 *      attr_p          -- attribute
 *
 * RETURN:
 *      BGP_TYPE_RESULT_OK/BGP_TYPE_RESULT_FAIL
 *
 * NOTES:
 *      For command: "show ip bgp neighbor A.B.C.D advertised-routes/received-routes"
 */
UI32_T BGP_PMGR_GetNextAdjAttrByPrefix(UI32_T as_number, UI32_T afi, UI32_T safi, L_INET_AddrIp_T *prefix_p, BOOL_T in, UI32_T *index_p, L_INET_AddrIp_T *neighbor_addr_p, BGP_TYPE_Attr_T *attr_p);

UI32_T BGP_PMGR_GetOrfPrefixList(NSM_POLICY_TYPE_PrefixList_T *prefix_list_info_p);

UI32_T BGP_PMGR_GetNextOrfPrefixListEntry(NSM_POLICY_TYPE_PrefixListEntry_T *prefix_list_entry_info_p);
UI32_T BGP_PMGR_GetNeighborPeerSort(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T *peer_sort_p);

/* MIB
 */
UI32_T BGP_PMGR_MIB_GetBgpVersion(char version_str[MAXSIZE_bgpVersion+1]);
UI32_T BGP_PMGR_MIB_GetBgpLocalAs(UI32_T *local_as_p);
UI32_T BGP_PMGR_MIB_GetBgpPeerEntry(BGP_TYPE_MIB_BgpPeerEntry_T *peer_entry_p);
UI32_T BGP_PMGR_MIB_GetNextBgpPeerEntry(BGP_TYPE_MIB_BgpPeerEntry_T *peer_entry_p);
UI32_T BGP_PMGR_MIB_SetBgpPeerAdminStatus(L_INET_AddrIp_T *neighbor_p, I32_T status);
UI32_T BGP_PMGR_MIB_SetBgpPeerConnectRetryInterval(L_INET_AddrIp_T *neighbor_p, I32_T interval);
UI32_T BGP_PMGR_MIB_SetBgpPeerHoldTimeConfigured(L_INET_AddrIp_T *neighbor_p, I32_T hold_time);
UI32_T BGP_PMGR_MIB_SetBgpPeerKeepAliveConfigured(L_INET_AddrIp_T *neighbor_p, I32_T keep_alive);
UI32_T BGP_PMGR_MIB_SetBgpPeerMinASOriginationInterval(L_INET_AddrIp_T *neighbor_p, I32_T as_orig);
UI32_T BGP_PMGR_MIB_SetBgpPeerMinRouteAdvertisementInterval(L_INET_AddrIp_T *neighbor_p, I32_T route_adv);
UI32_T BGP_PMGR_MIB_GetBgpIdentifier(UI32_T *bgp_identifier_p);
UI32_T BGP_PMGR_MIB_GetBgp4PathAttrEntry(BGP_TYPE_MIB_Bgp4PathAttrEntry_T *path_attr_p);
UI32_T BGP_PMGR_MIB_GetNextBgp4PathAttrEntry(BGP_TYPE_MIB_Bgp4PathAttrEntry_T *path_attr_p);

#endif    /* End of BGP_PMGR_H */

