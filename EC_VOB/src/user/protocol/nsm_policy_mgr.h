/* MODULE NAME:  nsm_policy_mgr.h
 * PURPOSE:
 *     This module provides APIs for ACCTON CSC to use.
 *
 * NOTES:
 *
 * HISTORY
 *    27/4/2011 - KH Shi, Created
 *
 * Copyright(C)      Edge-Core Corporation, 2011
 */
#ifndef NSM_POLICY_MGR_H
#define NSM_POLICY_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "l_inet.h"
#include "nsm_policy_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/**********************************************************
 **    nsm_policy_mgr ipc request command definitions    **
 **********************************************************
 */
enum
{
    NSM_POLICY_MGR_IPCCMD_ADD_IP_PREFIX_LIST = 0,
    NSM_POLICY_MGR_IPCCMD_DELETE_IP_PREFIX_LIST,
    NSM_POLICY_MGR_IPCCMD_GET_NEXT_IP_PREFIX_LIST,
    NSM_POLICY_MGR_IPCCMD_ADD_IP_PREFIX_LIST_ENTRY,
    NSM_POLICY_MGR_IPCCMD_DELETE_IP_PREFIX_LIST_ENTRY,
    NSM_POLICY_MGR_IPCCMD_GET_NEXT_IP_PREFIX_LIST_ENTRY,
    NSM_POLICY_MGR_IPCCMD_ADD_IP_AS_PATH_LIST,
    NSM_POLICY_MGR_IPCCMD_DELETE_IP_AS_PATH_LIST,
    NSM_POLICY_MGR_IPCCMD_GET_NEXT_IP_AS_PATH_LIST,
    NSM_POLICY_MGR_IPCCMD_GET_NEXT_IP_AS_PATH_LIST_ENTRY,
    NSM_POLICY_MGR_IPCCMD_ADD_IP_COMMUNITY_LIST,
    NSM_POLICY_MGR_IPCCMD_DELETE_IP_COMMUNITY_LIST,
    NSM_POLICY_MGR_IPCCMD_GET_NEXT_IP_COMMUNITY_LIST,
    NSM_POLICY_MGR_IPCCMD_GET_NEXT_IP_COMMUNITY_LIST_ENTRY,
    NSM_POLICY_MGR_IPCCMD_ADD_IP_EXT_COMMUNITY_LIST,
    NSM_POLICY_MGR_IPCCMD_DELETE_IP_EXT_COMMUNITY_LIST,
    NSM_POLICY_MGR_IPCCMD_GET_NEXT_IP_EXT_COMMUNITY_LIST,
    NSM_POLICY_MGR_IPCCMD_GET_NEXT_IP_EXT_COMMUNITY_LIST_ENTRY,
    NSM_POLICY_MGR_IPCCMD_ADD_ROUTE_MAP,
    NSM_POLICY_MGR_IPCCMD_DELETE_ROUTE_MAP,
    NSM_POLICY_MGR_IPCCMD_GET_NEXT_ROUTE_MAP,
    NSM_POLICY_MGR_IPCCMD_ADD_ROUTE_MAP_PREF,
    NSM_POLICY_MGR_IPCCMD_DELETE_ROUTE_MAP_PREF,
    NSM_POLICY_MGR_IPCCMD_GET_NEXT_ROUTE_MAP_INDEX,
    NSM_POLICY_MGR_IPCCMD_ADD_ROUTE_MAP_MATCH,
    NSM_POLICY_MGR_IPCCMD_DELETE_ROUTE_MAP_MATCH,
    NSM_POLICY_MGR_IPCCMD_GET_NEXT_ROUTE_MAP_MATCH,
    NSM_POLICY_MGR_IPCCMD_ADD_ROUTE_MAP_SET,
    NSM_POLICY_MGR_IPCCMD_DELETE_ROUTE_MAP_SET,
    NSM_POLICY_MGR_IPCCMD_GET_NEXT_ROUTE_MAP_SET,
    NSM_POLICY_MGR_IPCCMD_SET_ROUTE_MAP_CALL,
    NSM_POLICY_MGR_IPCCMD_UNSET_ROUTE_MAP_CALL,
    NSM_POLICY_MGR_IPCCMD_SET_ROUTE_MAP_DESCRIPTION,
    NSM_POLICY_MGR_IPCCMD_UNSET_ROUTE_MAP_DESCRIPTION,
    NSM_POLICY_MGR_IPCCMD_SET_ROUTE_MAP_ONMATCH_NEXT,
    NSM_POLICY_MGR_IPCCMD_UNSET_ROUTE_MAP_ONMATCH_NEXT,
    NSM_POLICY_MGR_IPCCMD_SET_ROUTE_MAP_ONMATCH_GOTO,
    NSM_POLICY_MGR_IPCCMD_UNSET_ROUTE_MAP_ONMATCH_GOTO,
};

//#define NSM_POLICY_MGR_MSG_HEADER_SIZE ((UI32_T)(&(((NSM_POLICY_MGR_IPCMsg_T*)0)->data)))
#define NSM_POLICY_MGR_MSG_HEADER_SIZE   (((char *)&((NSM_POLICY_MGR_IPCMsg_T *)0)->data-(char *)0))

/* MACRO FUNCTION DECLARATIONS
 */

/* MACRO FUNCTION NAME : NSM_POLICY_MGR_GET_MSGBUFSIZE
 * PURPOSE:
 *      Get the size of the message buffer which is used by NSM_POLICY ipc message
 *      according to what type is used in NSM_POLICY_MGR_IPCMsg_T.data.
 *
 * INPUT:
 *      msg_data_type  --  The type(NSM_POLICY_MGR_IPCMsg_T.data) used in this message.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      The size of the message buffer which is used by NSM POLICY ipc message.
 *
 * NOTES:
 *      None.
 */
#define NSM_POLICY_MGR_GET_MSGBUFSIZE(msg_data_type) \
    (NSM_POLICY_MGR_MSG_HEADER_SIZE + sizeof(msg_data_type))

#define NSM_POLICY_MGR_GET_MSG_SIZE(field_name)                       \
            (NSM_POLICY_MGR_MSG_HEADER_SIZE +                        \
            sizeof(((NSM_POLICY_MGR_IPCMsg_T *)0)->data.field_name))

/* DATA TYPE DECLARATIONS
 */

/**********************************************************
 **      structures used in nsm_policy_mgr ipc msgs      **
 **********************************************************
 */
typedef struct NSM_POLICY_MGR_IPC_PrefixList_S
{
    UI32_T  afi;
    char    plist_name[NSM_POLICY_TYPE_PREFIX_LIST_NAME_LEN +1];
} NSM_POLICY_MGR_IPC_PrefixList_T;

typedef struct NSM_POLICY_MGR_IPC_PrefixListEntry_S
{
    UI32_T  afi;
    UI32_T  seq_num;
    UI32_T  ge_num;
    UI32_T  le_num;
    L_INET_AddrIp_T prefix;
    char    plist_name[NSM_POLICY_TYPE_PREFIX_LIST_NAME_LEN +1];
    BOOL_T  is_permit;
} NSM_POLICY_MGR_IPC_PrefixListEntry_T;

typedef struct NSM_POLICY_MGR_IPC_AsPathList_S
{
    char    as_list[NSM_POLICY_TYPE_AS_LIST_NAME_LEN +1];
    char    reg_exp[NSM_POLICY_TYPE_REGULAR_EXP_LEN +1];
    BOOL_T  is_permit;
} NSM_POLICY_MGR_IPC_AsPathList_T;

typedef struct NSM_POLICY_MGR_IPC_CommunityList_S
{
    UI32_T style;
    char commu_name[NSM_POLICY_TYPE_COMMUNITY_NAME_LEN +1];
    char commu_and_regexp[NSM_POLICY_TYPE_REGULAR_EXP_LEN +1];
    BOOL_T is_permit;
    BOOL_T reject_all_digit_name;
} NSM_POLICY_MGR_IPC_CommunityList_T;

typedef struct NSM_POLICY_MGR_IPC_ExtCommunityList_S
{
    UI32_T style;
    char ext_commu_name[NSM_POLICY_TYPE_COMMUNITY_NAME_LEN +1];
    char ext_commu_and_regexp[NSM_POLICY_TYPE_REGULAR_EXP_LEN +1];
    BOOL_T is_permit;
    BOOL_T reject_all_digit_name;
} NSM_POLICY_MGR_IPC_ExtCommunityList_T;

typedef struct NSM_POLICY_MGR_IPC_RouteMap_S
{
    char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1];
} NSM_POLICY_MGR_IPC_RouteMap_T;

typedef struct NSM_POLICY_MGR_IPC_RouteMapPref_S
{
    UI32_T pref_index;
    char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1];
    BOOL_T is_permit;
} NSM_POLICY_MGR_IPC_RouteMapPref_T;

typedef struct NSM_POLICY_MGR_IPC_RouteMapMatchSet_S
{
    UI32_T pref_index;
    char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1];
    char command[NSM_POLICY_TYPE_ROUTE_MAP_COMMAND_LEN +1];
    char arg[NSM_POLICY_TYPE_ROUTE_MAP_ARG_LEN +1];
    BOOL_T is_permit;
} NSM_POLICY_MGR_IPC_RouteMapMatchSet_T;


/************************************************
 **      nsm_policy_mgr ipc msg structure      **
 ************************************************
 */
typedef struct NSM_POLICY_MGR_IPCMsg_S
{
    union NSM_POLICY_MGR_IPCMsg_Type_U
    {
        UI32_T cmd;         /* for sending IPC request command */
        BOOL_T result_bool; /* respond bool return */
        UI32_T result;      /* for response */
    } type;

    union
    {
        NSM_POLICY_MGR_IPC_PrefixList_T         prefix_list;
        NSM_POLICY_MGR_IPC_PrefixListEntry_T    prefix_list_entry;
        NSM_POLICY_MGR_IPC_AsPathList_T         as_path_list;
        NSM_POLICY_MGR_IPC_CommunityList_T      commu_list;
        NSM_POLICY_MGR_IPC_ExtCommunityList_T   ext_commu_list;
        NSM_POLICY_MGR_IPC_RouteMap_T           rmap;
        NSM_POLICY_MGR_IPC_RouteMapPref_T       rmap_pref;
        NSM_POLICY_MGR_IPC_RouteMapMatchSet_T   rmap_match_set;
        // get info
        BGP_TYPE_AsList_T                       as_list_info;
        BGP_TYPE_AsFilter_T                     as_filter_info;
        BGP_TYPE_CommunityList_T                commu_list_info;
        BGP_TYPE_CommunityEntry_T               commu_entry_info;
        NSM_POLICY_TYPE_PrefixList_T            prefix_list_info;
        NSM_POLICY_TYPE_PrefixListEntry_T       prefix_list_entry_info;
        NSM_POLICY_TYPE_RouteMap_T              rmap_info;
        NSM_POLICY_TYPE_RouteMapIndex_T         rmap_index; //aka route-map pref
        NSM_POLICY_TYPE_RouteMapRule_T          rmap_rule; // match or set

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

    } data;
} NSM_POLICY_MGR_IPCMsg_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

void NSM_POLICY_MGR_InitiateProcessResources();

/* FUNCTION NAME : NSM_POLICY_MGR_HandleIPCReqMsg
 * PURPOSE:
 *      Handle the ipc request received from mgr queue.
 *
 * INPUT:
 *      sysfun_msg_p  --  the ipc request for NSM_POLICY_MGR.
 *
 * OUTPUT:
 *      sysfun_msg_p  --  the ipc response to send when return value is TRUE
 *
 * RETURN:
 *      TRUE   --  A response is required to send
 *      FALSE  --  Need not to send response.
 *
 * NOTES:
 *      None.
 */
BOOL_T NSM_POLICY_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *sysfun_msg_p);

#endif

