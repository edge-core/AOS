/* MODULE NAME:  bgp_mgr.h
 * PURPOSE:
 *     BGP MGR APIs.
 *
 * NOTES:
 *
 * HISTORY
 *    02/14/2011 - KH Shi, Create
 *    03/01/2011 - Peter Yu, Add Show APIs
 *
 * Copyright(C)      Edge-Core Corporation, 2011
 */

#ifndef BGP_MGR_H
#define BGP_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "bgp_type.h"
//#include "l_radix.h"
#include <stdio.h>

#include <nsm_policy_type.h>

int bgp_main (void *arg);

#define BGP_MGR_DEBUG_FLAG_FUNCTION_CALL_IN  BIT_0
#define BGP_MGR_DEBUG_FLAG_SHOW_ERROR_MSG    BIT_1

#define BGP_MGR_IPCMSG_TYPE_SIZE sizeof(union BGP_MGR_IPCMsg_Type_U)

#define BGP_MGR_MSG_HEADER_SIZE ((UI32_T)(&(((BGP_MGR_IPCMsg_T*)0)->data)))


/*#define BGP_MGR_GET_MSGBUFSIZE(msg_data_type) \
    (BGP_MGR_IPCMSG_TYPE_SIZE + sizeof(msg_data_type))
*/
#define BGP_MGR_GET_MSG_SIZE(field_name)                       \
            (BGP_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((BGP_MGR_IPCMsg_T *)0)->data.field_name))

/***************************************************
 **    bgp_mgr ipc request command definitions    **
 ***************************************************
 */
enum
{
    /* policy */
    BGP_MGR_IPCCMD_ADD_IP_PREFIX_LIST = 0,
    BGP_MGR_IPCCMD_DELETE_IP_PREFIX_LIST,
    BGP_MGR_IPCCMD_GET_NEXT_IP_PREFIX_LIST,
    BGP_MGR_IPCCMD_ADD_IP_PREFIX_LIST_ENTRY,
    BGP_MGR_IPCCMD_DELETE_IP_PREFIX_LIST_ENTRY,
    BGP_MGR_IPCCMD_GET_NEXT_IP_PREFIX_LIST_ENTRY,
    BGP_MGR_IPCCMD_ADD_IP_AS_PATH_LIST,
    BGP_MGR_IPCCMD_DELETE_IP_AS_PATH_LIST,
    BGP_MGR_IPCCMD_GET_NEXT_IP_AS_PATH_LIST,
    BGP_MGR_IPCCMD_GET_NEXT_IP_AS_PATH_LIST_ENTRY,
    BGP_MGR_IPCCMD_ADD_IP_COMMUNITY_LIST,
    BGP_MGR_IPCCMD_DELETE_IP_COMMUNITY_LIST,
    BGP_MGR_IPCCMD_GET_NEXT_IP_COMMUNITY_LIST,
    BGP_MGR_IPCCMD_GET_NEXT_IP_COMMUNITY_LIST_ENTRY,
    BGP_MGR_IPCCMD_ADD_IP_EXT_COMMUNITY_LIST,
    BGP_MGR_IPCCMD_DELETE_IP_EXT_COMMUNITY_LIST,
    BGP_MGR_IPCCMD_GET_NEXT_IP_EXT_COMMUNITY_LIST,
    BGP_MGR_IPCCMD_GET_NEXT_IP_EXT_COMMUNITY_LIST_ENTRY,
    BGP_MGR_IPCCMD_ADD_ROUTE_MAP,
    BGP_MGR_IPCCMD_DELETE_ROUTE_MAP,
    BGP_MGR_IPCCMD_GET_ROUTE_MAP,
    BGP_MGR_IPCCMD_GET_NEXT_ROUTE_MAP,
    BGP_MGR_IPCCMD_ADD_ROUTE_MAP_PREF,
    BGP_MGR_IPCCMD_DELETE_ROUTE_MAP_PREF,
    BGP_MGR_IPCCMD_GET_ROUTE_MAP_INDEX,
    BGP_MGR_IPCCMD_GET_NEXT_ROUTE_MAP_INDEX,
    BGP_MGR_IPCCMD_ADD_ROUTE_MAP_MATCH,
    BGP_MGR_IPCCMD_DELETE_ROUTE_MAP_MATCH,
    BGP_MGR_IPCCMD_GET_NEXT_ROUTE_MAP_MATCH,
    BGP_MGR_IPCCMD_ADD_ROUTE_MAP_SET,
    BGP_MGR_IPCCMD_DELETE_ROUTE_MAP_SET,
    BGP_MGR_IPCCMD_GET_NEXT_ROUTE_MAP_SET,
    BGP_MGR_IPCCMD_SET_ROUTE_MAP_CALL,
    BGP_MGR_IPCCMD_UNSET_ROUTE_MAP_CALL,
    BGP_MGR_IPCCMD_SET_ROUTE_MAP_DESCRIPTION,
    BGP_MGR_IPCCMD_UNSET_ROUTE_MAP_DESCRIPTION,
    BGP_MGR_IPCCMD_SET_ROUTE_MAP_ONMATCH_NEXT,
    BGP_MGR_IPCCMD_UNSET_ROUTE_MAP_ONMATCH_NEXT,
    BGP_MGR_IPCCMD_SET_ROUTE_MAP_ONMATCH_GOTO,
    BGP_MGR_IPCCMD_UNSET_ROUTE_MAP_ONMATCH_GOTO,

    /* interface and address */
    BGP_MGR_IPCCMD_SIGNAL_L3IF_CREATE,
    BGP_MGR_IPCCMD_SIGNAL_L3IF_DESTROY,
    BGP_MGR_IPCCMD_SIGNAL_L3IF_UP,
    BGP_MGR_IPCCMD_SIGNAL_L3IF_DOWN,
    BGP_MGR_IPCCMD_SIGNAL_L3IF_RIF_CREATE,
    BGP_MGR_IPCCMD_SIGNAL_L3IF_RIF_DESTROY,
    BGP_MGR_IPCCMD_SIGNAL_L3IF_RIF_UP,
    BGP_MGR_IPCCMD_SIGNAL_L3IF_RIF_DOWN,

    /* bgp instance */
    BGP_MGR_IPCCMD_ENABLE_BGP_INSTANCE,
    BGP_MGR_IPCCMD_DISABLE_BGP_INSTANCE,
    BGP_MGR_IPCCMD_ADD_AGGREGATE_ADDRESS,
    BGP_MGR_IPCCMD_DELETE_AGGREGATE_ADDRESS,
    BGP_MGR_IPCCMD_UNSET_AGGREGATE_ADDRESS,
    BGP_MGR_IPCCMD_SET_FLAG,
    BGP_MGR_IPCCMD_UNSET_FLAG,
//    BGP_MGR_IPCCMD_SET_BEST_PATH_MED,
//    BGP_MGR_IPCCMD_UNSET_BEST_PATH_MED,
    BGP_MGR_IPCCMD_SET_CLUSTER_ID,
    BGP_MGR_IPCCMD_UNSET_CLUSTER_ID,
    BGP_MGR_IPCCMD_SET_CONFEDERATION_ID,
    BGP_MGR_IPCCMD_UNSET_CONFEDERATION_ID,
    BGP_MGR_IPCCMD_ADD_CONFEDERATION_PEERS,
    BGP_MGR_IPCCMD_DELETE_CONFEDERATION_PEERS,
    BGP_MGR_IPCCMD_ENABLE_DAMPENING,
    BGP_MGR_IPCCMD_DISABLE_DAMPENING,
    BGP_MGR_IPCCMD_SET_DEFAULT_LOCAL_PREFERENCE,
    BGP_MGR_IPCCMD_UNSET_DEFAULT_LOCAL_PREFERENCE,
    BGP_MGR_IPCCMD_SET_ROUTER_ID,
    BGP_MGR_IPCCMD_UNSET_ROUTER_ID,
    BGP_MGR_IPCCMD_SET_SCAN_TIME,
    BGP_MGR_IPCCMD_UNSET_SCAN_TIME,
    BGP_MGR_IPCCMD_CLEAR_BGP,
    BGP_MGR_IPCCMD_CLEAR_DAMPENING,
    BGP_MGR_IPCCMD_SET_DISTANCE,
    BGP_MGR_IPCCMD_UNSET_DISTANCE,    
    BGP_MGR_IPCCMD_SET_DISTANCE_BGP,
    BGP_MGR_IPCCMD_UNSET_DISTANCE_BGP,    
    BGP_MGR_IPCCMD_ADD_NETWORK,
    BGP_MGR_IPCCMD_DELETE_NETWORK,
    BGP_MGR_IPCCMD_SET_REDISTRIBUTE,
    BGP_MGR_IPCCMD_UNSET_REDISTRIBUTE,
    BGP_MGR_IPCCMD_SET_REDISTRIBUTE_ROUTEMAP,
    BGP_MGR_IPCCMD_UNSET_REDISTRIBUTE_ROUTEMAP,
    BGP_MGR_IPCCMD_SET_REDISTRIBUTE_METRIC,
    BGP_MGR_IPCCMD_UNSET_REDISTRIBUTE_METRIC,
    BGP_MGR_IPCCMD_SET_KEEP_ALIVE_AND_HOLD_TIME,
    BGP_MGR_IPCCMD_UNSET_KEEP_ALIVE_AND_HOLD_TIME,
    
    /* bgp neighbor */
    BGP_MGR_IPCCMD_ACTIVATE_NEIGHBOR,
    BGP_MGR_IPCCMD_DEACTIVATE_NEIGHBOR,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_ADVERTISEMENT_INTERVAL,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_ADVERTISEMENT_INTERVAL,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_ALLOW_AS_IN,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_ALLOW_AS_IN,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_ATTRIBUTE_UNCHANGED,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_ATTRIBUTE_UNCHANGED,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_CAPABILITY,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_CAPABILITY,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_CAPABILITY_ORF_PREFIX_LIST,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_CAPABILITY_ORF_PREFIX_LIST,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_DEFAULT_ORIGINATE,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_DEFAULT_ORIGINATE,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_DESCRIPTION,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_DESCRIPTION,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_DISTRIBUTE_LIST,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_DISTRIBUTE_LIST,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_FLAG,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_FLAG,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_AF_FLAG,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_AF_FLAG,    
    BGP_MGR_IPCCMD_SET_NEIGHBOR_EBGP_MULTIHOP,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_EBGP_MULTIHOP,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_FILTER_LIST,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_FILTER_LIST,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_INTERFACE,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_INTERFACE,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_MAXIMUM_PREFIX,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_MAXIMUM_PREFIX,
    BGP_MGR_IPCCMD_ADD_NEIGHBOR_PEER_GROUP,
    BGP_MGR_IPCCMD_DELETE_NEIGHBOR_PEER_GROUP,
    BGP_MGR_IPCCMD_ADD_NEIGHBOR_PEER_GROUP_MEMBER,
    BGP_MGR_IPCCMD_DELETE_NEIGHBOR_PEER_GROUP_MEMBER,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_PORT,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_PORT,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_PREFIX_LIST,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_PREFIX_LIST,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_REMOTE_AS,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_REMOTE_AS,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_ROUTE_MAP,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_ROUTE_MAP,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_ROUTE_SERVER_CLIENT,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_ROUTE_SERVER_CLIENT,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_KEEP_ALIVE_AND_HOLD_TIME,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_KEEP_ALIVE_AND_HOLD_TIME,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_CONNECT_RETRY_INTERVAL,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_CONNECT_RETRY_INTERVAL,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_UNSUPPRESS_MAP,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_UNSUPPRESS_MAP,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_UPDATE_SOURCE,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_UPDATE_SOURCE,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_WEIGHT,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_WEIGHT,
    BGP_MGR_IPCCMD_SET_NEIGHBOR_PASSWORD,
    BGP_MGR_IPCCMD_UNSET_NEIGHBOR_PASSWORD,
    // show
    BGP_MGR_IPCCMD_SHOW_BGP,
    BGP_MGR_IPCCMD_SHOW_BGP_SUMMARY,
    BGP_MGR_IPCCMD_SHOW_NEIGHBOR,
    // get
    BGP_MGR_IPCCMD_GET_BGP_INSTANCE,
    BGP_MGR_IPCCMD_GET_NEXT_BGP_INSTANCE,
    BGP_MGR_IPCCMD_GET_NEXT_NEIGHBOR_BY_AS_NUMBER,
    BGP_MGR_IPCCMD_GET_RUNNING_DATA_BY_FLD,
    BGP_MGR_IPCCMD_GET_NEXT_FILTER_ROUTE,
    BGP_MGR_IPCCMD_GET_NEXT_DATA_BY_TYPE,
    BGP_MGR_IPCCMD_GET_NEXT_MATCH_ROUTE_INFO,
    BGP_MGR_IPCCMD_GET_NEXT_ADVERTISE_NEIGHBOR_BY_PREFIX,
    BGP_MGR_IPCCMD_GET_NEXT_CLUSTER_ID,
//    BGP_MGR_IPCCMD_SHOW_SCAN, //temp
    BGP_MGR_IPCCMD_GET_SCAN_INFO,
    BGP_MGR_IPCCMD_GET_NEXT_NEXTHOP_CACHE,
    BGP_MGR_IPCCMD_GET_NEXT_CONNECTED_ROUTE,
    BGP_MGR_IPCCMD_GET_DAMPENING_PARAMETERS,
    BGP_MGR_IPCCMD_GET_NEXT_PREFIX,
    BGP_MGR_IPCCMD_GET_NEXT_ADJ_ATTR_BY_PREFIX,
    BGP_MGR_IPCCMD_GET_ORF_PREFIX_LIST,
    BGP_MGR_IPCCMD_GET_NEXT_ORF_PREFIX_LIST_ENTRY,
    BGP_MGR_IPCCMD_GET_NEIGHBOR_PEER_SORT,

    /* MIB */
    BGP_MGR_IPCCMD_MIB_GET_BGP_VERSION,
    BGP_MGR_IPCCMD_MIB_GET_BGP_LOCAL_AS,
    BGP_MGR_IPCCMD_MIB_GET_BGP_PEER_ENTRY,
    BGP_MGR_IPCCMD_MIB_GET_NEXT_BGP_PEER_ENTRY,
    BGP_MGR_IPCCMD_MIB_SET_BGP_PEER_ADMIN_STATUS,
    BGP_MGR_IPCCMD_MIB_SET_BGP_PEER_CONNECT_RETRY_INTERVAL,
    BGP_MGR_IPCCMD_MIB_SET_BGP_PEER_HOLD_TIME_CONFIGURED,
    BGP_MGR_IPCCMD_MIB_SET_BGP_PEER_KEEP_ALIVE_CONFIGURED,
    BGP_MGR_IPCCMD_MIB_SET_BGP_PEER_MIN_AS_ORIGINATION_INTERVAL,
    BGP_MGR_IPCCMD_MIB_SET_BGP_PEER_MIN_ROUTE_ADVERTISEMENT_INTERVAL,
    BGP_MGR_IPCCMD_MIB_GET_BGP_IDENTIFIER,
    BGP_MGR_IPCCMD_MIB_GET_BGP4_PATH_ATTR_ENTRY,
    BGP_MGR_IPCCMD_MIB_GET_NEXT_BGP4_PATH_ATTR_ENTRY,

};


/*****************************************
 **      bgp_mgr ipc msg structure      **
 *****************************************
 */
typedef struct BGP_MGR_IPCMsg_S
{
    union BGP_MGR_IPCMsg_Type_U
    {
        UI32_T cmd;    /* for sending IPC request command */
        BOOL_T result_bool;      /*respond bool return*/
        UI32_T result_ui32;      /*respond ui32 return*/
        SYS_TYPE_Get_Running_Cfg_T         result_running_cfg; /* For running config API */
    } type;

    union
    {
        UI32_T arg_ui32;
        BGP_TYPE_Instance_T instance;
        BGP_TYPE_Neighbor_T neighbor;
        BGP_TYPE_DataUnion_T data_union;
        BGP_TYPE_DampeningParameters_T dampening_param;

        // get info
        BGP_TYPE_AsList_T                       as_list_info;
        BGP_TYPE_AsFilter_T                     as_filter_info;
        BGP_TYPE_CommunityList_T                commu_list_info;
        BGP_TYPE_CommunityEntry_T               commu_entry_info;
        NSM_POLICY_TYPE_PrefixList_T            prefix_list_info;
        NSM_POLICY_TYPE_PrefixListEntry_T       prefix_list_entry_info;
        NSM_POLICY_TYPE_RouteMap_T              rmap_info;
        NSM_POLICY_TYPE_RouteMapIndex_T         rmap_index; //aka route_map pref
        NSM_POLICY_TYPE_RouteMapRule_T          rmap_rule; // match or set

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
        } arg_grp_ui32x2;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
        } arg_grp_ui32x3;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            UI32_T              ui32_4;
        } arg_grp_ui32x4;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            UI32_T              ui32_4;
            UI32_T              ui32_5;
            char                argstr[BGP_TYPE_CLEAR_BGP_ARG_STR_LEN+1];
        } arg_grp_ui32x5_argstr;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            UI32_T              ui32_4;
            UI32_T              ui32_5;
            UI32_T              ui32_6;
            UI32_T              ui32_7;
        } arg_grp_ui32x7;

        struct
        {
            UI32_T              ui32;
            L_INET_AddrIp_T     ipaddr;
        } arg_grp_ui32_ipaddr;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            L_INET_AddrIp_T     ipaddr;
        } arg_grp_ui32x2_ipaddr;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            L_INET_AddrIp_T     ipaddr;
            char                accesslist[BGP_TYPE_ACCESS_LIST_NAME_LEN+1];
        } arg_grp_ui32x2_ipaddr_accesslist;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            L_INET_AddrIp_T     ipaddr;
        } arg_grp_ui32x3_ipaddr;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            L_INET_AddrIp_T     ipaddr;
            char                peergroup[BGP_TYPE_PEER_GROUP_NAME_LEN+1];
        } arg_grp_ui32x3_ipaddr_peergroup;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            L_INET_AddrIp_T     ipaddr;
            BOOL_T              bool_1;
            BOOL_T              bool_2;    
        } arg_grp_ui32x3_ipaddr_boolx2;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            UI32_T              ui32_4;
            L_INET_AddrIp_T     ipaddr;
            char                routemap[BGP_TYPE_ROUTE_MAP_NAME_LEN+1];
            UI8_T               ui8;
        } arg_grp_ui32x4_ipaddr_routemap_ui8;

        struct
        {
            UI32_T              ui32;
            char                peerstr[BGP_TYPE_PEER_STR_LEN+1];
        } arg_grp_ui32_peerstr;

        struct
        {
            UI32_T              ui32;
            char                peergroup[BGP_TYPE_PEER_GROUP_NAME_LEN+1];
        } arg_grp_ui32_peergroup;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            char                peerstr[BGP_TYPE_PEER_STR_LEN+1];
        } arg_grp_ui32x2_peerstr;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            char                peerstr[BGP_TYPE_PEER_STR_LEN+1];
        } arg_grp_ui32x3_peerstr;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            char                routemap[BGP_TYPE_ROUTE_MAP_NAME_LEN+1];
        } arg_grp_ui32x3_routemap;        

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            UI32_T              ui32_4;
            char                peerstr[BGP_TYPE_PEER_STR_LEN+1];
        } arg_grp_ui32x4_peerstr;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            UI32_T              ui32_4;
            char                routemap[BGP_TYPE_ROUTE_MAP_NAME_LEN+1];
        } arg_grp_ui32x4_routemap;

        struct
        {
            UI32_T              ui32;
            char                peerstr[BGP_TYPE_PEER_STR_LEN+1];
            char                routemap[BGP_TYPE_ROUTE_MAP_NAME_LEN+1];
        } arg_grp_ui32_peerstr_routemap;

        struct
        {
            UI32_T              ui32;
            char                peerstr[BGP_TYPE_PEER_STR_LEN+1];
            char                neighdesc[BGP_TYPE_NEIGHBOR_DESC_LEN+1];
        } arg_grp_ui32_peerstr_neighdesc;

        struct
        {
            UI32_T              ui32;
            char                peerstr[BGP_TYPE_PEER_STR_LEN+1];
            char                accesslist[BGP_TYPE_ACCESS_LIST_NAME_LEN+1];
        } arg_grp_ui32_peerstr_accesslist;

        struct
        {
            UI32_T              ui32;
            char                peerstr[BGP_TYPE_PEER_STR_LEN+1];
            char                accesslist[BGP_TYPE_ACCESS_LIST_NAME_LEN+1];
            UI8_T               ui8;
        } arg_grp_ui32_peerstr_accesslist_ui8;

        struct
        {
            UI32_T              ui32;
            char                commu[BGP_TYPE_COMMUNITY_NAME_LEN+1];
            char                regexp[BGP_TYPE_REGULAR_EXP_LEN+1];
            BOOL_T              bool;
        } arg_grp_ui32_commu_regexp_bool;

        struct
        {
            UI32_T              ui32;
            char                commu[BGP_TYPE_COMMUNITY_NAME_LEN+1];
            char                regexp[BGP_TYPE_REGULAR_EXP_LEN+1];
            BOOL_T              bool_1;
            BOOL_T              bool_2;
        } arg_grp_ui32_commu_regexp_boolx2;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            char                peerstr[BGP_TYPE_PEER_STR_LEN+1];
            char                accesslist[BGP_TYPE_ACCESS_LIST_NAME_LEN+1];
        } arg_grp_ui32x3_peerstr_accesslist;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            char                peerstr[BGP_TYPE_PEER_STR_LEN+1];
            char                aslist[BGP_TYPE_AS_LIST_NAME_LEN+1];
        } arg_grp_ui32x3_peerstr_aslist;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            char                peerstr[BGP_TYPE_PEER_STR_LEN+1];
            char                routemap[BGP_TYPE_ROUTE_MAP_NAME_LEN+1];
        } arg_grp_ui32x3_peerstr_routemap;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            char                peerstr[BGP_TYPE_PEER_STR_LEN+1];
            char                prefixlist[BGP_TYPE_AS_LIST_NAME_LEN+1];
        } arg_grp_ui32x3_peerstr_prefixlist;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            UI32_T              ui32_4;            
            char                peerstr[BGP_TYPE_PEER_STR_LEN+1];
            char                accesslist[BGP_TYPE_ACCESS_LIST_NAME_LEN+1];
        } arg_grp_ui32x4_peerstr_accesslist;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            UI32_T              ui32_4;
            char                peerstr[BGP_TYPE_PEER_STR_LEN+1];
            char                aslist[BGP_TYPE_AS_LIST_NAME_LEN+1];
        } arg_grp_ui32x4_peerstr_aslist;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            UI32_T              ui32_4;
            char                peerstr[BGP_TYPE_PEER_STR_LEN+1];
            char                routemap[BGP_TYPE_ROUTE_MAP_NAME_LEN+1];
        } arg_grp_ui32x4_peerstr_routemap;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            UI32_T              ui32_4;
            char                peerstr[BGP_TYPE_PEER_STR_LEN+1];
            char                prefixlist[BGP_TYPE_AS_LIST_NAME_LEN+1];
        } arg_grp_ui32x4_peerstr_prefixlist;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2; 
            UI32_T              ui32_3;
            UI32_T              ui32_4; 
            UI32_T              ui32_5;
            UI16_T              ui16;
            char                peerstr[BGP_TYPE_PEER_STR_LEN+1];
            UI8_T               ui8;
        } arg_grp_ui32x5_ui16_peerstr_ui8;

        struct
        {
            char                aslist[BGP_TYPE_AS_LIST_NAME_LEN+1];
            char                regexp[BGP_TYPE_REGULAR_EXP_LEN+1];
            BOOL_T              bool;
        } arg_grp_aslist_regexp_bool;

        struct
        {
            UI32_T              ui32;
            char                plist[BGP_TYPE_PREFIX_LIST_NAME_LEN+1];
        } arg_grp_ui32_plist;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            UI32_T              ui32_4;
            L_INET_AddrIp_T     ipaddr;
            char                plist[BGP_TYPE_PREFIX_LIST_NAME_LEN+1];
            BOOL_T              bool;
        } arg_grp_ui32x4_ipaddr_plist_bool;

        struct
        {
            char                rmap[BGP_TYPE_ROUTE_MAP_NAME_LEN+1];
        } arg_grp_rmap;

        struct
        {
            UI32_T              ui32;
            char                rmap[BGP_TYPE_ROUTE_MAP_NAME_LEN+1];
            BOOL_T              bool;
        } arg_grp_ui32_rmap_bool;        

        struct
        {
            UI32_T              ui32;
            char                rmap[BGP_TYPE_ROUTE_MAP_NAME_LEN+1];
            char                cmd[BGP_TYPE_ROUTE_MAP_COMMAND_LEN+1];
            char                arg[BGP_TYPE_ROUTE_MAP_ARG_LEN+1];
            BOOL_T              bool;
        } arg_grp_ui32_rmap_cmd_arg_bool;        

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            UI32_T              ui32_4;
            UI32_T              ui32_5;
            char                argstr[BGP_TYPE_REGULAR_EXP_LEN+1];
            L_INET_AddrIp_T     ipaddr;
            BGP_TYPE_BgpInfo_T  binfo;
        } arg_grp_ui32x5_argstr_ipaddr_binfo;

        struct
        {
            UI32_T              ui32_1;
            UI32_T              ui32_2;
            UI32_T              ui32_3;
            BOOL_T              bool;
            L_INET_AddrIp_T     ipaddr;
            BGP_TYPE_BgpInfo_T  binfo;
        } arg_grp_ui32x3_bool_ipaddr_binfo;

        /* route-map */
        struct
        {
            UI32_T ui32;
            BOOL_T bool;
            char rmap[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1];
        } arg_grp_ui32_bool_rmap;

        struct
        {
            UI32_T ui32;
            BOOL_T bool;
            char rmap_1[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1];
            char rmap_2[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1];
        } arg_grp_ui32_bool_rmapx2;

        struct
        {
            UI32_T ui32;
            BOOL_T bool;
            char rmap[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1];
            char desc[NSM_POLICY_TYPE_ROUTE_MAP_DESCRIPTION_LEN +1];
        } arg_grp_ui32_bool_rmap_desc;

        struct
        {
            UI32_T ui32_1;
            UI32_T ui32_2;
            BOOL_T bool;
            char rmap[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1];
        } arg_grp_ui32x2_bool_rmap;
        
        struct
        {
            UI32_T ui32_1;
            UI32_T ui32_2;
            UI32_T ui32_3;
            L_INET_AddrIp_T ipaddr_1;
            L_INET_AddrIp_T ipaddr_2;
        } arg_grp_ui32x3_ipaddrx2;

        struct
        {
            UI32_T ui32;
            BOOL_T bool;
        } arg_grp_ui32_bool;
        
        struct
        {
            UI32_T ui32_1;
            UI32_T ui32_2;
            UI32_T ui32_3;
            UI32_T ui32_4;
            L_INET_AddrIp_T prefix;
            BGP_TYPE_BgpInfo_T binfo;
        } arg_grp_ui32x4_prefix_binfo;
        struct
        {
            UI32_T ui32_1;
            UI32_T ui32_2;
            L_INET_AddrIp_T prefix;        
            BGP_TYPE_BgpNexthopCache_T bnc;
        } arg_grp_ui32x2_prefix_bnc;
        struct
        {
            UI32_T ui32_1;
            UI32_T ui32_2;
            UI32_T ui32_3;
            UI32_T ui32_4;
            UI32_T bool;
            L_INET_AddrIp_T     ipaddr_1;
            L_INET_AddrIp_T     ipaddr_2;
            BGP_TYPE_Attr_T     attr;
        } arg_grp_ui32x4_bool_ipaddrx2_attr;
        struct
        {
            UI32_T              ui32;
            char                peerstr[BGP_TYPE_PEER_STR_LEN+1];
            char                password[BGP_TYPE_PEER_PASSWORD_LEN+1];
        } arg_grp_ui32_peerstr_password;
        /* MIB */
        BGP_TYPE_MIB_BgpVersion_T        bgp_version;
        BGP_TYPE_MIB_BgpPeerEntry_T      bgp_peer_entry;
        BGP_TYPE_MIB_Bgp4PathAttrEntry_T bgp4_path_attr_entry;

    } data;
} BGP_MGR_IPCMsg_T;

void BGP_MGR_Init(void);

/* FUNCTION NAME:  BGP_MGR_SetTransitionMode
* PURPOSE:
*    This function will set transition state flag.
*
* INPUT:
*    None.
*
* OUTPUT:
*    None.
*
* RETURN:
*    None.
*
* NOTES:
*    None.
*/
void BGP_MGR_SetTransitionMode(void);

/* FUNCTION NAME:  BGP_MGR_EnterTransitionMode
* PURPOSE:
*    This function will force BGP to enter transition state.
*
* INPUT:
*    None.
*
* OUTPUT:
*    None.
*
* RETURN:
*    None.
*
* NOTES:
*    None.
*
*/
void BGP_MGR_EnterTransitionMode(void);

/* FUNCTION NAME:  BGP_MGR_EnterMasterMode
* PURPOSE:
*    This function will force BGP to enter master state.
*
* INPUT:
*    None.
*
* OUTPUT:
*    None.
*
* RETURN:
*    None.
*
* NOTES:
*    None.
*
*/
void BGP_MGR_EnterMasterMode(void);

/* FUNCTION NAME:  BGP_MGR_EnterSlaveMode
* PURPOSE:
*    This function will force BGP to enter slave state.
*
* INPUT:
*    None.
*
* OUTPUT:
*    None.
*
* RETURN:
*    None.
*
* NOTES:
*    None.
*
*/
void BGP_MGR_EnterSlaveMode(void);

/* FUNCTION NAME : BGP_MGR_HandleIpcReqMsg
* PURPOSE:
*      Handle the ipc request received from mgr queue.
*
* INPUT:
*      sysfun_msg_p  --  The ipc request for BGP_MGR.
*
* OUTPUT:
*      sysfun_msg_p  --  The ipc response to send when return value is TRUE
*
* RETURN:
*      TRUE   --  A response is required to send
*      FALSE  --  Need not to send response.
*
* NOTES:
*      1. The buffer length in sysfun_msg_p must be large enough for sending
*         all possible response messages.
*/
BOOL_T BGP_MGR_HandleIpcReqMsg(SYSFUN_Msg_T *sysfun_msg_p);

BOOL_T BGP_MGR_GetDebugFlag(UI32_T *flag_p);
BOOL_T BGP_MGR_SetDebugFlag(UI32_T flag);
BOOL_T BGP_MGR_UnsetDebugFlag(UI32_T flag);

#endif

