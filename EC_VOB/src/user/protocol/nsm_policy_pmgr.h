/* MODULE NAME:  nsm_policy_pmgr.h
 * PURPOSE:
 *     NSM POLICY PMGR APIs.
 *
 * NOTES:
 *
 * HISTORY
 *    27/04/2011 - KH Shi, Create
 *
 * Copyright(C)      Edge-Core Corporation, 2011
 */
#ifndef NSM_POLICY_PMGR_H
#define NSM_POLICY_PMGR_H

#include "sys_type.h"
#include "l_inet.h"
#include "nsm_policy_type.h"

/* Current implementation of NSM_POLICY is just pass through to BGP,
 * but NSM shouldn't call BGP, would cause dead-lock in some circumstances.
 * Marked off NSM_POLICY_PMGR APIs temporary, will be re-opened after the implementation moved back to NSM
 */
#if 0
/* FUNCTION NAME : NSM_POLICY_PMGR_InitiateProcessResource
 * PURPOSE:
 *      Initiate process resources for NSM_POLICY_PMGR.
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
BOOL_T NSM_POLICY_PMGR_InitiateProcessResource(void);

UI32_T NSM_POLICY_PMGR_AddIpPrefixList(char plist_name[NSM_POLICY_TYPE_PREFIX_LIST_NAME_LEN +1], UI32_T afi);
UI32_T NSM_POLICY_PMGR_DeleteIpPrefixList(char plist_name[NSM_POLICY_TYPE_PREFIX_LIST_NAME_LEN +1], UI32_T afi);
UI32_T NSM_POLICY_PMGR_GetNextIpPrefixList(NSM_POLICY_TYPE_PrefixList_T *prefix_list_info_p);
UI32_T NSM_POLICY_PMGR_AddIpPrefixListEntry(char plist_name[NSM_POLICY_TYPE_PREFIX_LIST_NAME_LEN +1], UI32_T afi, UI32_T seq_num, BOOL_T is_permit, L_INET_AddrIp_T *prefix_p, UI32_T ge_num, UI32_T le_num);
UI32_T NSM_POLICY_PMGR_DeleteIpPrefixListEntry(char plist_name[NSM_POLICY_TYPE_PREFIX_LIST_NAME_LEN +1], UI32_T afi, UI32_T seq_num, BOOL_T is_permit, L_INET_AddrIp_T *prefix_p, UI32_T ge_num, UI32_T le_num);
UI32_T NSM_POLICY_PMGR_GetNextIpPrefixListEntry(NSM_POLICY_TYPE_PrefixListEntry_T *prefix_list_entry_info_p);
UI32_T NSM_POLICY_PMGR_AddIpAsPathList(char as_list[NSM_POLICY_TYPE_AS_LIST_NAME_LEN +1], BOOL_T is_permit, char reg_exp[NSM_POLICY_TYPE_REGULAR_EXP_LEN +1]);
UI32_T NSM_POLICY_PMGR_DeleteIpAsPathList(char as_list[NSM_POLICY_TYPE_AS_LIST_NAME_LEN +1], BOOL_T is_permit, char reg_exp[NSM_POLICY_TYPE_REGULAR_EXP_LEN +1]);
UI32_T NSM_POLICY_PMGR_GetNextIpAsPathList(BGP_TYPE_AsList_T *as_list_p);
UI32_T NSM_POLICY_PMGR_GetNextIpAsPathListEntry(BGP_TYPE_AsFilter_T *as_filter_p);
UI32_T NSM_POLICY_PMGR_AddIpCommunityList(char commu_name[NSM_POLICY_TYPE_COMMUNITY_NAME_LEN +1], BOOL_T is_permit, UI32_T style, char commu_and_regexp[NSM_POLICY_TYPE_REGULAR_EXP_LEN +1], BOOL_T reject_all_digit_name);
UI32_T NSM_POLICY_PMGR_DeleteIpCommunityList(char commu_name[NSM_POLICY_TYPE_COMMUNITY_NAME_LEN +1], BOOL_T is_permit, UI32_T style, char commu_and_regexp[NSM_POLICY_TYPE_REGULAR_EXP_LEN +1]);
UI32_T NSM_POLICY_PMGR_GetNextIpCommunityList(BGP_TYPE_CommunityList_T *list_p);
UI32_T NSM_POLICY_PMGR_GetNextIpCommunityListEntry(BGP_TYPE_CommunityEntry_T *entry_p);
UI32_T NSM_POLICY_PMGR_AddIpExtCommunityList(char ext_commu_name[NSM_POLICY_TYPE_COMMUNITY_NAME_LEN +1], BOOL_T is_permit, UI32_T style, char ext_commu_and_regexp[NSM_POLICY_TYPE_REGULAR_EXP_LEN +1], BOOL_T reject_all_digit_name);
UI32_T NSM_POLICY_PMGR_DeleteIpExtCommunityList(char ext_commu_name[NSM_POLICY_TYPE_COMMUNITY_NAME_LEN +1], BOOL_T is_permit, UI32_T style, char ext_commu_and_regexp[NSM_POLICY_TYPE_REGULAR_EXP_LEN +1]);
UI32_T NSM_POLICY_PMGR_GetNextIpExtCommunityList(BGP_TYPE_CommunityList_T *list_p);
UI32_T NSM_POLICY_PMGR_GetNextIpExtCommunityListEntry(BGP_TYPE_CommunityEntry_T *entry_p);
UI32_T NSM_POLICY_PMGR_AddRouteMap(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1]);
UI32_T NSM_POLICY_PMGR_DeleteRouteMap(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1]);
UI32_T NSM_POLICY_PMGR_GetNextRouteMap(NSM_POLICY_TYPE_RouteMap_T *rmap_info_p);
UI32_T NSM_POLICY_PMGR_AddRouteMapPref(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index);
UI32_T NSM_POLICY_PMGR_DeleteRouteMapPref(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index);
UI32_T NSM_POLICY_PMGR_GetNextRouteMapIndex(NSM_POLICY_TYPE_RouteMapIndex_T *rmap_index_p);
UI32_T NSM_POLICY_PMGR_AddRouteMapMatch(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index, char command[NSM_POLICY_TYPE_ROUTE_MAP_COMMAND_LEN +1], char arg[NSM_POLICY_TYPE_ROUTE_MAP_ARG_LEN +1]);
UI32_T NSM_POLICY_PMGR_DeleteRouteMapMatch(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index, char command[NSM_POLICY_TYPE_ROUTE_MAP_COMMAND_LEN +1], char arg[NSM_POLICY_TYPE_ROUTE_MAP_ARG_LEN +1]);
UI32_T NSM_POLICY_PMGR_GetNextRouteMapMatch(NSM_POLICY_TYPE_RouteMapRule_T *rmap_rule_p);
UI32_T NSM_POLICY_PMGR_AddRouteMapSet(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index, char command[NSM_POLICY_TYPE_ROUTE_MAP_COMMAND_LEN +1], char arg[NSM_POLICY_TYPE_ROUTE_MAP_ARG_LEN +1]);
UI32_T NSM_POLICY_PMGR_DeleteRouteMapSet(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index, char command[NSM_POLICY_TYPE_ROUTE_MAP_COMMAND_LEN +1], char arg[NSM_POLICY_TYPE_ROUTE_MAP_ARG_LEN +1]);
UI32_T NSM_POLICY_PMGR_GetNextRouteMapSet(NSM_POLICY_TYPE_RouteMapRule_T *rmap_rule_p);

UI32_T NSM_POLICY_PMGR_SetRouteMapCall(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index, char call_rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1]);
UI32_T NSM_POLICY_PMGR_UnsetRouteMapCall(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index);
UI32_T NSM_POLICY_PMGR_SetRouteMapDescription(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index, char desc[NSM_POLICY_TYPE_ROUTE_MAP_DESCRIPTION_LEN +1]);
UI32_T NSM_POLICY_PMGR_UnsetRouteMapDescription(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index);
UI32_T NSM_POLICY_PMGR_SetRouteMapOnMatchNext(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index);
UI32_T NSM_POLICY_PMGR_UnsetRouteMapOnMatchNext(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index);
UI32_T NSM_POLICY_PMGR_SetRouteMapOnMatchGoto(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index, UI32_T clause_number);
UI32_T NSM_POLICY_PMGR_UnsetRouteMapOnMatchGoto(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index);
#endif

#endif

