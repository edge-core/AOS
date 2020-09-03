/* MODULE NAME:  bgp_pmgr.c
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

#include "sys_type.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "bgp_mgr.h"
#include "bgp_type.h"
#include "string.h"

static SYSFUN_MsgQ_T msgq_handle= 0;

#define BGP_PMGR_DECLARE_MSG_P(data_type) \
    const UI32_T msg_size = BGP_MGR_GET_MSG_SIZE(data_type);\
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];\
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;\
    BGP_MGR_IPCMsg_T *msg_p = (BGP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;\
    msgbuf_p->cmd = SYS_MODULE_BGP;\
    msgbuf_p->msg_size = msg_size;

#define BGP_PMGR_SEND_WAIT_MSG_P() \
do{\
    if(SYSFUN_OK!=SYSFUN_SendRequestMsg(msgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,\
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size , msgbuf_p))\
    {\
        SYSFUN_Debug_Printf("%s():SYSFUN_SendRequestMsg fail\n", __FUNCTION__);\
        return BGP_TYPE_RESULT_SEND_MSG_FAIL;\
    }\
}while(0)


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
BOOL_T BGP_PMGR_InitiateProcessResource(void)
{
    if(SYSFUN_OK!=SYSFUN_GetMsgQ(SYS_BLD_BGP_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL,
     &msgq_handle))
    {
         printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
         return FALSE;
    }
    return TRUE;
}

/* policy */
UI32_T BGP_PMGR_AddIpPrefixList(char plist_name[BGP_TYPE_PREFIX_LIST_NAME_LEN+1], UI32_T afi)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_plist);

    msg_p->type.cmd = BGP_MGR_IPCCMD_ADD_IP_PREFIX_LIST;

    msg_p->data.arg_grp_ui32_plist.ui32 = afi;
    memcpy(msg_p->data.arg_grp_ui32_plist.plist, plist_name, sizeof(msg_p->data.arg_grp_ui32_plist.plist));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_DeleteIpPrefixList(char plist_name[BGP_TYPE_PREFIX_LIST_NAME_LEN+1], UI32_T afi)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_plist);

    msg_p->type.cmd = BGP_MGR_IPCCMD_DELETE_IP_PREFIX_LIST;

    msg_p->data.arg_grp_ui32_plist.ui32 = afi;
    memcpy(msg_p->data.arg_grp_ui32_plist.plist, plist_name, sizeof(msg_p->data.arg_grp_ui32_plist.plist));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetNextIpPrefixList(NSM_POLICY_TYPE_PrefixList_T *prefix_list_info_p)
{
    BGP_PMGR_DECLARE_MSG_P(prefix_list_info);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_IP_PREFIX_LIST;

    memcpy(&msg_p->data.prefix_list_info, prefix_list_info_p, sizeof(msg_p->data.prefix_list_info));

    BGP_PMGR_SEND_WAIT_MSG_P();
    memcpy(prefix_list_info_p, &msg_p->data.prefix_list_info, sizeof(msg_p->data.prefix_list_info));
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_AddIpPrefixListEntry(char plist_name[BGP_TYPE_PREFIX_LIST_NAME_LEN+1], UI32_T afi, UI32_T seq_num, BOOL_T is_permit, L_INET_AddrIp_T *prefix_p, UI32_T ge_num, UI32_T le_num)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4_ipaddr_plist_bool);

    msg_p->type.cmd = BGP_MGR_IPCCMD_ADD_IP_PREFIX_LIST_ENTRY;

    msg_p->data.arg_grp_ui32x4_ipaddr_plist_bool.ui32_1 = afi;
    msg_p->data.arg_grp_ui32x4_ipaddr_plist_bool.ui32_2 = seq_num;
    msg_p->data.arg_grp_ui32x4_ipaddr_plist_bool.ui32_3 = ge_num;
    msg_p->data.arg_grp_ui32x4_ipaddr_plist_bool.ui32_4 = le_num;
    msg_p->data.arg_grp_ui32x4_ipaddr_plist_bool.bool   = is_permit;
    msg_p->data.arg_grp_ui32x4_ipaddr_plist_bool.ipaddr = *prefix_p;
    memcpy(msg_p->data.arg_grp_ui32x4_ipaddr_plist_bool.plist, plist_name, sizeof(msg_p->data.arg_grp_ui32x4_ipaddr_plist_bool.plist));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_DeleteIpPrefixListEntry(char plist_name[BGP_TYPE_PREFIX_LIST_NAME_LEN+1], UI32_T afi, UI32_T seq_num, BOOL_T is_permit, L_INET_AddrIp_T *prefix_p, UI32_T ge_num, UI32_T le_num)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4_ipaddr_plist_bool);

    msg_p->type.cmd = BGP_MGR_IPCCMD_DELETE_IP_PREFIX_LIST_ENTRY;

    msg_p->data.arg_grp_ui32x4_ipaddr_plist_bool.ui32_1 = afi;
    msg_p->data.arg_grp_ui32x4_ipaddr_plist_bool.ui32_2 = seq_num;
    msg_p->data.arg_grp_ui32x4_ipaddr_plist_bool.ui32_3 = ge_num;
    msg_p->data.arg_grp_ui32x4_ipaddr_plist_bool.ui32_4 = le_num;
    msg_p->data.arg_grp_ui32x4_ipaddr_plist_bool.bool   = is_permit;
    msg_p->data.arg_grp_ui32x4_ipaddr_plist_bool.ipaddr = *prefix_p;
    memcpy(msg_p->data.arg_grp_ui32x4_ipaddr_plist_bool.plist, plist_name, sizeof(msg_p->data.arg_grp_ui32x4_ipaddr_plist_bool.plist));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetNextIpPrefixListEntry(NSM_POLICY_TYPE_PrefixListEntry_T *prefix_list_entry_info_p)
{
    BGP_PMGR_DECLARE_MSG_P(prefix_list_entry_info);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_IP_PREFIX_LIST_ENTRY;

    memcpy(&msg_p->data.prefix_list_entry_info, prefix_list_entry_info_p, sizeof(msg_p->data.prefix_list_entry_info));

    BGP_PMGR_SEND_WAIT_MSG_P();

    memcpy(prefix_list_entry_info_p, &msg_p->data.prefix_list_entry_info, sizeof(msg_p->data.prefix_list_entry_info));
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_AddIpAsPathList(char as_list[BGP_TYPE_AS_LIST_NAME_LEN+1], BOOL_T is_permit, char reg_exp[BGP_TYPE_REGULAR_EXP_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_aslist_regexp_bool);

    msg_p->type.cmd = BGP_MGR_IPCCMD_ADD_IP_AS_PATH_LIST;

    msg_p->data.arg_grp_aslist_regexp_bool.bool = is_permit;
    memcpy(msg_p->data.arg_grp_aslist_regexp_bool.aslist, as_list, sizeof(msg_p->data.arg_grp_aslist_regexp_bool.aslist));
    memcpy(msg_p->data.arg_grp_aslist_regexp_bool.regexp, reg_exp, sizeof(msg_p->data.arg_grp_aslist_regexp_bool.regexp));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_DeleteIpAsPathList(char as_list[BGP_TYPE_AS_LIST_NAME_LEN+1], BOOL_T is_permit, char reg_exp[BGP_TYPE_REGULAR_EXP_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_aslist_regexp_bool);

    msg_p->type.cmd = BGP_MGR_IPCCMD_DELETE_IP_AS_PATH_LIST;

    msg_p->data.arg_grp_aslist_regexp_bool.bool = is_permit;
    memcpy(msg_p->data.arg_grp_aslist_regexp_bool.aslist, as_list, sizeof(msg_p->data.arg_grp_aslist_regexp_bool.aslist));
    memcpy(msg_p->data.arg_grp_aslist_regexp_bool.regexp, reg_exp, sizeof(msg_p->data.arg_grp_aslist_regexp_bool.regexp));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetNextIpAsPathList(BGP_TYPE_AsList_T *as_list_p)
{
    BGP_PMGR_DECLARE_MSG_P(as_list_info);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_IP_AS_PATH_LIST;

    memcpy(&msg_p->data.as_list_info, as_list_p, sizeof(msg_p->data.as_list_info));

    BGP_PMGR_SEND_WAIT_MSG_P();
    memcpy(as_list_p, &msg_p->data.as_list_info, sizeof(*as_list_p));
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetNextIpAsPathListEntry(BGP_TYPE_AsFilter_T *as_filter_p)
{
    BGP_PMGR_DECLARE_MSG_P(as_filter_info);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_IP_AS_PATH_LIST_ENTRY;

    memcpy(&msg_p->data.as_filter_info, as_filter_p, sizeof(msg_p->data.as_filter_info));

    BGP_PMGR_SEND_WAIT_MSG_P();
    memcpy(as_filter_p, &msg_p->data.as_filter_info, sizeof(*as_filter_p));
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_AddIpCommunityList(char commu_name[BGP_TYPE_COMMUNITY_NAME_LEN+1], BOOL_T is_permit, UI32_T style, char commu_and_regexp[BGP_TYPE_REGULAR_EXP_LEN+1], BOOL_T reject_all_digit_name)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_commu_regexp_boolx2);

    msg_p->type.cmd = BGP_MGR_IPCCMD_ADD_IP_COMMUNITY_LIST;

    msg_p->data.arg_grp_ui32_commu_regexp_boolx2.bool_1 = is_permit;
    msg_p->data.arg_grp_ui32_commu_regexp_boolx2.bool_2 = reject_all_digit_name;
    msg_p->data.arg_grp_ui32_commu_regexp_boolx2.ui32 = style;
    memcpy(msg_p->data.arg_grp_ui32_commu_regexp_boolx2.commu, commu_name, sizeof(msg_p->data.arg_grp_ui32_commu_regexp_boolx2.commu));
    memcpy(msg_p->data.arg_grp_ui32_commu_regexp_boolx2.regexp, commu_and_regexp, sizeof(msg_p->data.arg_grp_ui32_commu_regexp_boolx2.regexp));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_DeleteIpCommunityList(char commu_name[BGP_TYPE_COMMUNITY_NAME_LEN+1], BOOL_T is_permit, UI32_T style, char commu_and_regexp[BGP_TYPE_REGULAR_EXP_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_commu_regexp_bool);

    msg_p->type.cmd = BGP_MGR_IPCCMD_DELETE_IP_COMMUNITY_LIST;

    msg_p->data.arg_grp_ui32_commu_regexp_bool.bool = is_permit;
    msg_p->data.arg_grp_ui32_commu_regexp_bool.ui32 = style;
    memcpy(msg_p->data.arg_grp_ui32_commu_regexp_bool.commu, commu_name, sizeof(msg_p->data.arg_grp_ui32_commu_regexp_bool.commu));
    memcpy(msg_p->data.arg_grp_ui32_commu_regexp_bool.regexp, commu_and_regexp, sizeof(msg_p->data.arg_grp_ui32_commu_regexp_bool.regexp));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetNextIpCommunityList(BGP_TYPE_CommunityList_T *list_p)
{
    BGP_PMGR_DECLARE_MSG_P(commu_list_info);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_IP_COMMUNITY_LIST;

    memcpy(&msg_p->data.commu_list_info, list_p, sizeof(msg_p->data.commu_list_info));

    BGP_PMGR_SEND_WAIT_MSG_P();

    memcpy(list_p, &msg_p->data.commu_list_info, sizeof(msg_p->data.commu_list_info));
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetNextIpCommunityListEntry(BGP_TYPE_CommunityEntry_T *entry_p)
{
    BGP_PMGR_DECLARE_MSG_P(commu_entry_info);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_IP_COMMUNITY_LIST_ENTRY;

    memcpy(&msg_p->data.commu_entry_info, entry_p, sizeof(msg_p->data.commu_entry_info));

    BGP_PMGR_SEND_WAIT_MSG_P();
    memcpy(entry_p, &msg_p->data.commu_entry_info, sizeof(msg_p->data.commu_entry_info));

    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_AddIpExtCommunityList(char ext_commu_name[BGP_TYPE_COMMUNITY_NAME_LEN+1], BOOL_T is_permit, UI32_T style, char ext_commu_and_regexp[BGP_TYPE_REGULAR_EXP_LEN+1], BOOL_T reject_all_digit_name)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_commu_regexp_boolx2);

    msg_p->type.cmd = BGP_MGR_IPCCMD_ADD_IP_EXT_COMMUNITY_LIST;

    msg_p->data.arg_grp_ui32_commu_regexp_boolx2.bool_1 = is_permit;
    msg_p->data.arg_grp_ui32_commu_regexp_boolx2.bool_2 = reject_all_digit_name;
    msg_p->data.arg_grp_ui32_commu_regexp_boolx2.ui32 = style;
    memcpy(msg_p->data.arg_grp_ui32_commu_regexp_boolx2.commu, ext_commu_name, sizeof(msg_p->data.arg_grp_ui32_commu_regexp_boolx2.commu));
    memcpy(msg_p->data.arg_grp_ui32_commu_regexp_boolx2.regexp, ext_commu_and_regexp, sizeof(msg_p->data.arg_grp_ui32_commu_regexp_boolx2.regexp));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_DeleteIpExtCommunityList(char ext_commu_name[BGP_TYPE_COMMUNITY_NAME_LEN+1], BOOL_T is_permit, UI32_T style, char ext_commu_and_regexp[BGP_TYPE_REGULAR_EXP_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_commu_regexp_bool);

    msg_p->type.cmd = BGP_MGR_IPCCMD_DELETE_IP_EXT_COMMUNITY_LIST;

    msg_p->data.arg_grp_ui32_commu_regexp_bool.bool = is_permit;
    msg_p->data.arg_grp_ui32_commu_regexp_bool.ui32 = style;
    memcpy(msg_p->data.arg_grp_ui32_commu_regexp_bool.commu, ext_commu_name, sizeof(msg_p->data.arg_grp_ui32_commu_regexp_bool.commu));
    memcpy(msg_p->data.arg_grp_ui32_commu_regexp_bool.regexp, ext_commu_and_regexp, sizeof(msg_p->data.arg_grp_ui32_commu_regexp_bool.regexp));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetNextIpExtCommunityList(BGP_TYPE_CommunityList_T *list_p)
{
    BGP_PMGR_DECLARE_MSG_P(commu_list_info);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_IP_EXT_COMMUNITY_LIST;

    memcpy(&msg_p->data.commu_list_info, list_p, sizeof(msg_p->data.commu_list_info));

    BGP_PMGR_SEND_WAIT_MSG_P();

    memcpy(list_p, &msg_p->data.commu_list_info, sizeof(msg_p->data.commu_list_info));
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetNextIpExtCommunityListEntry(BGP_TYPE_CommunityEntry_T *entry_p)
{
    BGP_PMGR_DECLARE_MSG_P(commu_entry_info);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_IP_EXT_COMMUNITY_LIST_ENTRY;

    memcpy(&msg_p->data.commu_entry_info, entry_p, sizeof(msg_p->data.commu_entry_info));

    BGP_PMGR_SEND_WAIT_MSG_P();
    memcpy(entry_p, &msg_p->data.commu_entry_info, sizeof(msg_p->data.commu_entry_info));

    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_AddRouteMap(char rmap_name[BGP_TYPE_ROUTE_MAP_NAME_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_rmap);

    msg_p->type.cmd = BGP_MGR_IPCCMD_ADD_ROUTE_MAP;

    memcpy(msg_p->data.arg_grp_rmap.rmap, rmap_name, sizeof(msg_p->data.arg_grp_rmap.rmap));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_DeleteRouteMap(char rmap_name[BGP_TYPE_ROUTE_MAP_NAME_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_rmap);

    msg_p->type.cmd = BGP_MGR_IPCCMD_DELETE_ROUTE_MAP;

    memcpy(msg_p->data.arg_grp_rmap.rmap, rmap_name, sizeof(msg_p->data.arg_grp_rmap.rmap));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetRouteMap(NSM_POLICY_TYPE_RouteMap_T *rmap_info_p)
{
    BGP_PMGR_DECLARE_MSG_P(rmap_info);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_ROUTE_MAP;

    memcpy(&msg_p->data.rmap_info, rmap_info_p, sizeof(msg_p->data.rmap_info));

    BGP_PMGR_SEND_WAIT_MSG_P();
    memcpy(rmap_info_p, &msg_p->data.rmap_info, sizeof(msg_p->data.rmap_info));
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetNextRouteMap(NSM_POLICY_TYPE_RouteMap_T *rmap_info_p)
{
    BGP_PMGR_DECLARE_MSG_P(rmap_info);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_ROUTE_MAP;

    memcpy(&msg_p->data.rmap_info, rmap_info_p, sizeof(msg_p->data.rmap_info));

    BGP_PMGR_SEND_WAIT_MSG_P();
    memcpy(rmap_info_p, &msg_p->data.rmap_info, sizeof(msg_p->data.rmap_info));
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_AddRouteMapPref(char rmap_name[BGP_TYPE_ROUTE_MAP_NAME_LEN+1], BOOL_T is_permit, UI32_T pref_index)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_rmap_bool);

    msg_p->type.cmd = BGP_MGR_IPCCMD_ADD_ROUTE_MAP_PREF;

    msg_p->data.arg_grp_ui32_rmap_bool.ui32 = pref_index;
    msg_p->data.arg_grp_ui32_rmap_bool.bool = is_permit;
    memcpy(msg_p->data.arg_grp_ui32_rmap_bool.rmap, rmap_name, sizeof(msg_p->data.arg_grp_ui32_rmap_bool.rmap));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_DeleteRouteMapPref(char rmap_name[BGP_TYPE_ROUTE_MAP_NAME_LEN+1], BOOL_T is_permit, UI32_T pref_index)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_rmap_bool);

    msg_p->type.cmd = BGP_MGR_IPCCMD_DELETE_ROUTE_MAP_PREF;

    msg_p->data.arg_grp_ui32_rmap_bool.ui32 = pref_index;
    msg_p->data.arg_grp_ui32_rmap_bool.bool = is_permit;
    memcpy(msg_p->data.arg_grp_ui32_rmap_bool.rmap, rmap_name, sizeof(msg_p->data.arg_grp_ui32_rmap_bool.rmap));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetRouteMapIndex(NSM_POLICY_TYPE_RouteMapIndex_T *rmap_index_p)
{
    BGP_PMGR_DECLARE_MSG_P(rmap_index);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_ROUTE_MAP_INDEX;

    memcpy(&msg_p->data.rmap_index, rmap_index_p, sizeof(msg_p->data.rmap_index));

    BGP_PMGR_SEND_WAIT_MSG_P();

    memcpy(rmap_index_p, &msg_p->data.rmap_index, sizeof(msg_p->data.rmap_index));
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetNextRouteMapIndex(NSM_POLICY_TYPE_RouteMapIndex_T *rmap_index_p)
{
    BGP_PMGR_DECLARE_MSG_P(rmap_index);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_ROUTE_MAP_INDEX;

    memcpy(&msg_p->data.rmap_index, rmap_index_p, sizeof(msg_p->data.rmap_index));

    BGP_PMGR_SEND_WAIT_MSG_P();

    memcpy(rmap_index_p, &msg_p->data.rmap_index, sizeof(msg_p->data.rmap_index));
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_AddRouteMapMatch(char rmap_name[BGP_TYPE_ROUTE_MAP_NAME_LEN+1], BOOL_T is_permit, UI32_T pref_index, char command[BGP_TYPE_ROUTE_MAP_COMMAND_LEN+1], char arg[BGP_TYPE_ROUTE_MAP_ARG_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_rmap_cmd_arg_bool);

    msg_p->type.cmd = BGP_MGR_IPCCMD_ADD_ROUTE_MAP_MATCH;

    msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.ui32 = pref_index;
    msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.bool = is_permit;
    memcpy(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.rmap, rmap_name, sizeof(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.rmap));
    memcpy(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.cmd, command, sizeof(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.cmd));
    memcpy(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.arg, arg, sizeof(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.arg));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}
    
UI32_T BGP_PMGR_DeleteRouteMapMatch(char rmap_name[BGP_TYPE_ROUTE_MAP_NAME_LEN+1], BOOL_T is_permit, UI32_T pref_index, char command[BGP_TYPE_ROUTE_MAP_COMMAND_LEN+1], char arg[BGP_TYPE_ROUTE_MAP_ARG_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_rmap_cmd_arg_bool);

    msg_p->type.cmd = BGP_MGR_IPCCMD_DELETE_ROUTE_MAP_MATCH;

    msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.ui32 = pref_index;
    msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.bool = is_permit;
    memcpy(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.rmap, rmap_name, sizeof(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.rmap));
    memcpy(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.cmd, command, sizeof(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.cmd));
    memcpy(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.arg, arg, sizeof(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.arg));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;    
}

UI32_T BGP_PMGR_GetNextRouteMapMatch(NSM_POLICY_TYPE_RouteMapRule_T *rmap_rule_p)
{
    BGP_PMGR_DECLARE_MSG_P(rmap_rule);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_ROUTE_MAP_MATCH;

    memcpy(&msg_p->data.rmap_rule, rmap_rule_p, sizeof(msg_p->data.rmap_rule));
    BGP_PMGR_SEND_WAIT_MSG_P();
    memcpy(rmap_rule_p, &msg_p->data.rmap_rule, sizeof(msg_p->data.rmap_rule));

    return msg_p->type.result_ui32;    
}

UI32_T BGP_PMGR_AddRouteMapSet(char rmap_name[BGP_TYPE_ROUTE_MAP_NAME_LEN+1], BOOL_T is_permit, UI32_T pref_index, char command[BGP_TYPE_ROUTE_MAP_COMMAND_LEN+1], char arg[BGP_TYPE_ROUTE_MAP_ARG_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_rmap_cmd_arg_bool);

    msg_p->type.cmd = BGP_MGR_IPCCMD_ADD_ROUTE_MAP_SET;

    msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.ui32 = pref_index;
    msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.bool = is_permit;
    memcpy(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.rmap, rmap_name, sizeof(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.rmap));
    memcpy(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.cmd, command, sizeof(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.cmd));
    memcpy(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.arg, arg, sizeof(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.arg));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;    
}
    
UI32_T BGP_PMGR_DeleteRouteMapSet(char rmap_name[BGP_TYPE_ROUTE_MAP_NAME_LEN+1], BOOL_T is_permit, UI32_T pref_index, char command[BGP_TYPE_ROUTE_MAP_COMMAND_LEN+1], char arg[BGP_TYPE_ROUTE_MAP_ARG_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_rmap_cmd_arg_bool);

    msg_p->type.cmd = BGP_MGR_IPCCMD_DELETE_ROUTE_MAP_SET;

    msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.ui32 = pref_index;
    msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.bool = is_permit;
    memcpy(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.rmap, rmap_name, sizeof(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.rmap));
    memcpy(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.cmd, command, sizeof(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.cmd));
    memcpy(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.arg, arg, sizeof(msg_p->data.arg_grp_ui32_rmap_cmd_arg_bool.arg));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;    
}

UI32_T BGP_PMGR_GetNextRouteMapSet(NSM_POLICY_TYPE_RouteMapRule_T *rmap_rule_p)
{
    BGP_PMGR_DECLARE_MSG_P(rmap_rule);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_ROUTE_MAP_SET;

    memcpy(&msg_p->data.rmap_rule, rmap_rule_p, sizeof(msg_p->data.rmap_rule));

    BGP_PMGR_SEND_WAIT_MSG_P();
    memcpy(rmap_rule_p, &msg_p->data.rmap_rule, sizeof(msg_p->data.rmap_rule));
    
    return msg_p->type.result_ui32;    
}

UI32_T BGP_PMGR_SetRouteMapCall(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index, char call_rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_bool_rmapx2);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_ROUTE_MAP_CALL;

    memcpy(msg_p->data.arg_grp_ui32_bool_rmapx2.rmap_1, rmap_name, sizeof(msg_p->data.arg_grp_ui32_bool_rmapx2.rmap_1));
    msg_p->data.arg_grp_ui32_bool_rmapx2.bool = is_permit;
    msg_p->data.arg_grp_ui32_bool_rmapx2.ui32 = pref_index;
    memcpy(msg_p->data.arg_grp_ui32_bool_rmapx2.rmap_2, call_rmap_name, sizeof(msg_p->data.arg_grp_ui32_bool_rmapx2.rmap_2));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;    
}
UI32_T BGP_PMGR_UnsetRouteMapCall(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_bool_rmap);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_ROUTE_MAP_CALL;

    memcpy(msg_p->data.arg_grp_ui32_bool_rmap.rmap, rmap_name, sizeof(msg_p->data.arg_grp_ui32_bool_rmap.rmap));
    msg_p->data.arg_grp_ui32_bool_rmap.bool = is_permit;
    msg_p->data.arg_grp_ui32_bool_rmap.ui32 = pref_index;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;    
}
UI32_T BGP_PMGR_SetRouteMapDescription(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index, char desc[NSM_POLICY_TYPE_ROUTE_MAP_DESCRIPTION_LEN +1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_bool_rmap_desc);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_ROUTE_MAP_DESCRIPTION;

    memcpy(msg_p->data.arg_grp_ui32_bool_rmap_desc.rmap, rmap_name, sizeof(msg_p->data.arg_grp_ui32_bool_rmap_desc.rmap));
    msg_p->data.arg_grp_ui32_bool_rmap_desc.bool = is_permit;
    msg_p->data.arg_grp_ui32_bool_rmap_desc.ui32 = pref_index;
    memcpy(msg_p->data.arg_grp_ui32_bool_rmap_desc.desc, desc, sizeof(msg_p->data.arg_grp_ui32_bool_rmap_desc.desc));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;    
}
UI32_T BGP_PMGR_UnsetRouteMapDescription(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_bool_rmap);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_ROUTE_MAP_DESCRIPTION;

    memcpy(msg_p->data.arg_grp_ui32_bool_rmap.rmap, rmap_name, sizeof(msg_p->data.arg_grp_ui32_bool_rmap.rmap));
    msg_p->data.arg_grp_ui32_bool_rmap.bool = is_permit;
    msg_p->data.arg_grp_ui32_bool_rmap.ui32 = pref_index;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;    
}
UI32_T BGP_PMGR_SetRouteMapOnMatchNext(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_bool_rmap);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_ROUTE_MAP_ONMATCH_NEXT;

    memcpy(msg_p->data.arg_grp_ui32_bool_rmap.rmap, rmap_name, sizeof(msg_p->data.arg_grp_ui32_bool_rmap.rmap));
    msg_p->data.arg_grp_ui32_bool_rmap.bool = is_permit;
    msg_p->data.arg_grp_ui32_bool_rmap.ui32 = pref_index;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;    
}
UI32_T BGP_PMGR_UnsetRouteMapOnMatchNext(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_bool_rmap);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_ROUTE_MAP_ONMATCH_NEXT;

    memcpy(msg_p->data.arg_grp_ui32_bool_rmap.rmap, rmap_name, sizeof(msg_p->data.arg_grp_ui32_bool_rmap.rmap));
    msg_p->data.arg_grp_ui32_bool_rmap.bool = is_permit;
    msg_p->data.arg_grp_ui32_bool_rmap.ui32 = pref_index;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;    
}
UI32_T BGP_PMGR_SetRouteMapOnMatchGoto(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index, UI32_T clause_number)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2_bool_rmap);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_ROUTE_MAP_ONMATCH_GOTO;

    memcpy(msg_p->data.arg_grp_ui32x2_bool_rmap.rmap, rmap_name, sizeof(msg_p->data.arg_grp_ui32x2_bool_rmap.rmap));
    msg_p->data.arg_grp_ui32x2_bool_rmap.bool = is_permit;
    msg_p->data.arg_grp_ui32x2_bool_rmap.ui32_1 = pref_index;
    msg_p->data.arg_grp_ui32x2_bool_rmap.ui32_2 = clause_number;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;    
}
UI32_T BGP_PMGR_UnsetRouteMapOnMatchGoto(char rmap_name[NSM_POLICY_TYPE_ROUTE_MAP_NAME_LEN +1], BOOL_T is_permit, UI32_T pref_index)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_bool_rmap);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_ROUTE_MAP_ONMATCH_GOTO;

    memcpy(msg_p->data.arg_grp_ui32_bool_rmap.rmap, rmap_name, sizeof(msg_p->data.arg_grp_ui32_bool_rmap.rmap));
    msg_p->data.arg_grp_ui32_bool_rmap.bool = is_permit;
    msg_p->data.arg_grp_ui32_bool_rmap.ui32 = pref_index;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;    
}

/* interface and rif update*/
UI32_T BGP_PMGR_SignalL3IfCreate(UI32_T ifindex)
{
    BGP_PMGR_DECLARE_MSG_P(arg_ui32);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SIGNAL_L3IF_CREATE;
    msg_p->data.arg_ui32 = ifindex;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SignalL3IfDestroy(UI32_T ifindex)
{
    BGP_PMGR_DECLARE_MSG_P(arg_ui32);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SIGNAL_L3IF_DESTROY;
    msg_p->data.arg_ui32 = ifindex;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SignalL3IfUp(UI32_T ifindex)
{
    BGP_PMGR_DECLARE_MSG_P(arg_ui32);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SIGNAL_L3IF_UP;
    msg_p->data.arg_ui32 = ifindex;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SignalL3IfDown(UI32_T ifindex)
{
    BGP_PMGR_DECLARE_MSG_P(arg_ui32);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SIGNAL_L3IF_DOWN;
    msg_p->data.arg_ui32 = ifindex;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SignalL3IfRifCreate(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_ipaddr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SIGNAL_L3IF_RIF_CREATE;
    msg_p->data.arg_grp_ui32_ipaddr.ui32 = ifindex;
    memcpy(&(msg_p->data.arg_grp_ui32_ipaddr.ipaddr), ip_addr_p, sizeof(L_INET_AddrIp_T));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SignalL3IfRifDestroy(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_ipaddr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SIGNAL_L3IF_RIF_DESTROY;
    msg_p->data.arg_grp_ui32_ipaddr.ui32 = ifindex;
    memcpy(&(msg_p->data.arg_grp_ui32_ipaddr.ipaddr), ip_addr_p, sizeof(L_INET_AddrIp_T));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SignalL3IfRifUp(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_ipaddr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SIGNAL_L3IF_RIF_UP;
    msg_p->data.arg_grp_ui32_ipaddr.ui32 = ifindex;
    memcpy(&(msg_p->data.arg_grp_ui32_ipaddr.ipaddr), ip_addr_p, sizeof(L_INET_AddrIp_T));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SignalL3IfRifDown(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_ipaddr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SIGNAL_L3IF_RIF_DOWN;
    msg_p->data.arg_grp_ui32_ipaddr.ui32 = ifindex;
    memcpy(&(msg_p->data.arg_grp_ui32_ipaddr.ipaddr), ip_addr_p, sizeof(L_INET_AddrIp_T));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}


/* BGP instance functions
 */

UI32_T BGP_PMGR_EnableBgpInstance(UI32_T as_number)
{
    BGP_PMGR_DECLARE_MSG_P(arg_ui32);

    msg_p->type.cmd = BGP_MGR_IPCCMD_ENABLE_BGP_INSTANCE;

    msg_p->data.arg_ui32 = as_number;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_DisableBgpInstance(UI32_T as_number)
{
    BGP_PMGR_DECLARE_MSG_P(arg_ui32);

    msg_p->type.cmd = BGP_MGR_IPCCMD_DISABLE_BGP_INSTANCE;

    msg_p->data.arg_ui32 = as_number;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}
    
/* FUNCTION NAME : BGP_PMGR_AddAggregateAddress
 * PURPOSE:
 *     
 *
 * INPUT:
 *     
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
UI32_T BGP_PMGR_AddAggregateAddress(UI32_T as_number, UI32_T afi, UI32_T safi, L_INET_AddrIp_T *ipaddr_p, BOOL_T is_summary_only, BOOL_T is_as_set)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_ipaddr_boolx2);

    msg_p->type.cmd = BGP_MGR_IPCCMD_ADD_AGGREGATE_ADDRESS;

    msg_p->data.arg_grp_ui32x3_ipaddr_boolx2.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_ipaddr_boolx2.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_ipaddr_boolx2.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x3_ipaddr_boolx2.bool_1 = is_summary_only;
    msg_p->data.arg_grp_ui32x3_ipaddr_boolx2.bool_2 = is_as_set;
    msg_p->data.arg_grp_ui32x3_ipaddr_boolx2.ipaddr = *ipaddr_p;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

//afi_t afi, safi_t safi,
UI32_T BGP_PMGR_DeleteAggregateAddress(UI32_T as_number, UI32_T afi, UI32_T safi, L_INET_AddrIp_T *ipaddr_p)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_ipaddr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_DELETE_AGGREGATE_ADDRESS;

    msg_p->data.arg_grp_ui32x3_ipaddr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_ipaddr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_ipaddr.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x3_ipaddr.ipaddr = *ipaddr_p;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetAggregateAddress(UI32_T as_number, UI32_T afi, UI32_T safi, L_INET_AddrIp_T *ipaddr_p, BOOL_T is_summary_only, BOOL_T is_as_set)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_ipaddr_boolx2);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_AGGREGATE_ADDRESS;

    msg_p->data.arg_grp_ui32x3_ipaddr_boolx2.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_ipaddr_boolx2.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_ipaddr_boolx2.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x3_ipaddr_boolx2.bool_1 = is_summary_only;
    msg_p->data.arg_grp_ui32x3_ipaddr_boolx2.bool_2 = is_as_set;
    msg_p->data.arg_grp_ui32x3_ipaddr_boolx2.ipaddr = *ipaddr_p;

    BGP_PMGR_SEND_WAIT_MSG_P();

    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetFlag(UI32_T as_number, UI32_T flag)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_FLAG;

    msg_p->data.arg_grp_ui32x2.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2.ui32_2 = flag;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetFlag(UI32_T as_number, UI32_T flag)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_FLAG;

    msg_p->data.arg_grp_ui32x2.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2.ui32_2 = flag;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

/*
UI32_T BGP_PMGR_SetBestPathMED(UI32_T as_number, BOOL_T confed, BOOL_T missing_as_worst)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_3);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_BEST_PATH_MED;

    msg_p->data.arg_grp3.arg1 = as_number;
    msg_p->data.arg_grp3.arg2 = confed;
    msg_p->data.arg_grp3.arg3 = missing_as_worst;

    BGP_PMGR_SEND_WAIT_MSG_P
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetBestPathMED(UI32_T as_number, BOOL_T confed, BOOL_T missing_as_worst)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_3);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_BEST_PATH_MED;

    msg_p->data.arg_grp3.arg1 = as_number;
    msg_p->data.arg_grp3.arg2 = confed;
    msg_p->data.arg_grp3.arg3 = missing_as_worst;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}
*/

UI32_T BGP_PMGR_SetClusterId(UI32_T as_number, UI32_T cluster_id, UI32_T format)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_CLUSTER_ID;

    msg_p->data.arg_grp_ui32x3.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3.ui32_2 = cluster_id;
    msg_p->data.arg_grp_ui32x3.ui32_3 = format;
    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetClusterId(UI32_T as_number)
{
    BGP_PMGR_DECLARE_MSG_P(arg_ui32);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_CLUSTER_ID;

    msg_p->data.arg_ui32 = as_number;
    //msg_p->data.arg_grp4.arg2 = cluster_id;
    //msg_p->data.arg_grp3.arg3 = missing_as_worst;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetConfederationId(UI32_T as_number, UI32_T confederation_id)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_CONFEDERATION_ID;

    msg_p->data.arg_grp_ui32x2.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2.ui32_2 = confederation_id;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetConfederationId(UI32_T as_number)
{
    BGP_PMGR_DECLARE_MSG_P(arg_ui32);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_CONFEDERATION_ID;

    msg_p->data.arg_ui32 = as_number;
    //msg_p->data.arg_grp4.arg2 = confederation_id;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_AddConfederationPeer(UI32_T as_number, UI32_T peer_as_id)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2);

    msg_p->type.cmd = BGP_MGR_IPCCMD_ADD_CONFEDERATION_PEERS;

    msg_p->data.arg_grp_ui32x2.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2.ui32_2 = peer_as_id;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_DeleteConfederationPeer(UI32_T as_number, UI32_T peer_as_id)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2);

    msg_p->type.cmd = BGP_MGR_IPCCMD_DELETE_CONFEDERATION_PEERS;

    msg_p->data.arg_grp_ui32x2.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2.ui32_2 = peer_as_id;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_EnableDampening(UI32_T as_number, UI32_T afi, UI32_T safi, UI32_T half_life, UI32_T reuse_limit, UI32_T suppress_limit, UI32_T max_suppress_time)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x7);

    msg_p->type.cmd = BGP_MGR_IPCCMD_ENABLE_DAMPENING;

    msg_p->data.arg_grp_ui32x7.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x7.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x7.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x7.ui32_4 = half_life;
    msg_p->data.arg_grp_ui32x7.ui32_5 = reuse_limit;
    msg_p->data.arg_grp_ui32x7.ui32_6 = suppress_limit;
    msg_p->data.arg_grp_ui32x7.ui32_7 = max_suppress_time;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_DisableDampening(UI32_T as_number, UI32_T afi, UI32_T safi)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3);

    msg_p->type.cmd = BGP_MGR_IPCCMD_DISABLE_DAMPENING;

    msg_p->data.arg_grp_ui32x3.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3.ui32_3 = safi;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetDefaultLocalPreference(UI32_T as_number, UI32_T preference)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_DEFAULT_LOCAL_PREFERENCE;

    msg_p->data.arg_grp_ui32x2.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2.ui32_2 = preference;    

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetDefaultLocalPreference(UI32_T as_number)
{
    BGP_PMGR_DECLARE_MSG_P(arg_ui32);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_DEFAULT_LOCAL_PREFERENCE;

    msg_p->data.arg_ui32 = as_number;
    //msg_p->data.arg_grp4.arg2 = preference;    

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetRouterId(UI32_T as_number, UI32_T router_id)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_ROUTER_ID;

    msg_p->data.arg_grp_ui32x2.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2.ui32_2 = router_id;    

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetRouterId(UI32_T as_number)
{
    BGP_PMGR_DECLARE_MSG_P(arg_ui32);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_ROUTER_ID;

    msg_p->data.arg_ui32 = as_number;
    //msg_p->data.arg_grp4.arg2 = router_id;    

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetScanTime(UI32_T as_number, UI32_T scan_time)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_SCAN_TIME;

    msg_p->data.arg_grp_ui32x2.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2.ui32_2 = scan_time;    

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetScanTime(UI32_T as_number)
{
    BGP_PMGR_DECLARE_MSG_P(arg_ui32);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_SCAN_TIME;

    msg_p->data.arg_ui32 = as_number;
    //msg_p->data.arg_grp4.arg2 = scan_time;    

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_ClearBgp(UI32_T as_number, UI32_T afi, UI32_T safi, UI32_T clear_sort, UI32_T clear_type, char arg[BGP_TYPE_CLEAR_BGP_ARG_STR_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x5_argstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_CLEAR_BGP;

    msg_p->data.arg_grp_ui32x5_argstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x5_argstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x5_argstr.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x5_argstr.ui32_4 = clear_sort;
    msg_p->data.arg_grp_ui32x5_argstr.ui32_5 = clear_type;
    
    if (arg) /* arg may be NULL */
        memcpy(msg_p->data.arg_grp_ui32x5_argstr.argstr, arg, sizeof(msg_p->data.arg_grp_ui32x5_argstr.argstr));
    else
        memset(msg_p->data.arg_grp_ui32x5_argstr.argstr, 0, sizeof(msg_p->data.arg_grp_ui32x5_argstr.argstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_ClearDampening(UI32_T as_number, UI32_T afi, UI32_T safi, L_INET_AddrIp_T *ipaddr_p)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_ipaddr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_CLEAR_DAMPENING;

    msg_p->data.arg_grp_ui32x3_ipaddr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_ipaddr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_ipaddr.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x3_ipaddr.ipaddr = *ipaddr_p;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetDistance(UI32_T as_number, UI32_T distance, L_INET_AddrIp_T *ipaddr_p, char access_list_str[BGP_TYPE_ACCESS_LIST_NAME_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2_ipaddr_accesslist);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_DISTANCE;

    msg_p->data.arg_grp_ui32x2_ipaddr_accesslist.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2_ipaddr_accesslist.ui32_2 = distance;
    msg_p->data.arg_grp_ui32x2_ipaddr_accesslist.ipaddr = *ipaddr_p;
    if (access_list_str)
        memcpy(msg_p->data.arg_grp_ui32x2_ipaddr_accesslist.accesslist, access_list_str, sizeof(msg_p->data.arg_grp_ui32x2_ipaddr_accesslist.accesslist));
    else
        memset(msg_p->data.arg_grp_ui32x2_ipaddr_accesslist.accesslist, 0, sizeof(msg_p->data.arg_grp_ui32x2_ipaddr_accesslist.accesslist));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetDistance(UI32_T as_number, L_INET_AddrIp_T *ipaddr_p)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_ipaddr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_DISTANCE;

    msg_p->data.arg_grp_ui32_ipaddr.ui32 = as_number;
    msg_p->data.arg_grp_ui32_ipaddr.ipaddr = *ipaddr_p;
    
    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetDistanceBgp(UI32_T as_number, UI32_T distance_ebgp, UI32_T distance_ibgp, UI32_T distance_local)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_DISTANCE_BGP;

    msg_p->data.arg_grp_ui32x4.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x4.ui32_2 = distance_ebgp;
    msg_p->data.arg_grp_ui32x4.ui32_3 = distance_ibgp;
    msg_p->data.arg_grp_ui32x4.ui32_4 = distance_local;

    BGP_PMGR_SEND_WAIT_MSG_P();

    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetDistanceBgp(UI32_T as_number)
{
    BGP_PMGR_DECLARE_MSG_P(arg_ui32);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_DISTANCE_BGP;

    msg_p->data.arg_ui32 = as_number;
    //msg_p->data.arg_grp5.arg2 = distance_ebgp;
    //msg_p->data.arg_grp5.arg3 = distance_ibgp;
    //msg_p->data.arg_grp5.arg4 = distance_local;
    
    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_AddNetwork(UI32_T as_number, UI32_T afi, UI32_T safi, L_INET_AddrIp_T *network_addr_p, char route_map[BGP_TYPE_ROUTE_MAP_NAME_LEN+1], UI32_T backdoor, UI8_T ttl)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4_ipaddr_routemap_ui8);

    msg_p->type.cmd = BGP_MGR_IPCCMD_ADD_NETWORK;

    msg_p->data.arg_grp_ui32x4_ipaddr_routemap_ui8.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x4_ipaddr_routemap_ui8.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x4_ipaddr_routemap_ui8.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x4_ipaddr_routemap_ui8.ui32_4 = backdoor;
    msg_p->data.arg_grp_ui32x4_ipaddr_routemap_ui8.ui8    = ttl;
    msg_p->data.arg_grp_ui32x4_ipaddr_routemap_ui8.ipaddr = *network_addr_p;
    memcpy(msg_p->data.arg_grp_ui32x4_ipaddr_routemap_ui8.routemap, route_map, sizeof(msg_p->data.arg_grp_ui32x4_ipaddr_routemap_ui8.routemap));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_DeleteNetwork(UI32_T as_number, UI32_T afi, UI32_T safi, L_INET_AddrIp_T *network_addr_p)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_ipaddr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_DELETE_NETWORK;

    msg_p->data.arg_grp_ui32x3_ipaddr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_ipaddr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_ipaddr.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x3_ipaddr.ipaddr = *network_addr_p;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetRedistribute(UI32_T as_number, UI32_T afi, UI32_T type)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_REDISTRIBUTE;

    msg_p->data.arg_grp_ui32x3.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3.ui32_3 = type;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetRedistribute(UI32_T as_number, UI32_T afi, UI32_T type)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_REDISTRIBUTE;

    msg_p->data.arg_grp_ui32x3.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3.ui32_3 = type;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetRedistributeRouteMap(UI32_T as_number, UI32_T afi, UI32_T type, char route_map[BGP_TYPE_ROUTE_MAP_NAME_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_routemap);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_REDISTRIBUTE_ROUTEMAP;

    msg_p->data.arg_grp_ui32x3_routemap.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_routemap.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_routemap.ui32_3 = type;
    memcpy(msg_p->data.arg_grp_ui32x3_routemap.routemap, route_map, sizeof(msg_p->data.arg_grp_ui32x3_routemap.routemap));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetRedistributeRouteMap(UI32_T as_number, UI32_T afi, UI32_T type)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_REDISTRIBUTE_ROUTEMAP;

    msg_p->data.arg_grp_ui32x3.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3.ui32_3 = type;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetRedistributeMetric(UI32_T as_number, UI32_T afi, UI32_T type, UI32_T metric)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_REDISTRIBUTE_METRIC;

    msg_p->data.arg_grp_ui32x4.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x4.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x4.ui32_3 = type;
    msg_p->data.arg_grp_ui32x4.ui32_4 = metric;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetRedistributeMetric(UI32_T as_number, UI32_T afi, UI32_T type)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_REDISTRIBUTE_METRIC;

    msg_p->data.arg_grp_ui32x3.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3.ui32_3 = type;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetKeepAliveAndHoldTime(UI32_T as_number, UI32_T keepalive, UI32_T holdtime)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_KEEP_ALIVE_AND_HOLD_TIME;

    msg_p->data.arg_grp_ui32x3.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3.ui32_2 = keepalive;
    msg_p->data.arg_grp_ui32x3.ui32_3 = holdtime;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetKeepAliveAndHoldTime(UI32_T as_number)
{
    BGP_PMGR_DECLARE_MSG_P(arg_ui32);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_KEEP_ALIVE_AND_HOLD_TIME;

    msg_p->data.arg_ui32 = as_number;

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

/*---------- neighbor ------------*/

UI32_T BGP_PMGR_ActivateNeighbor(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1]) /* ??? */
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_ACTIVATE_NEIGHBOR;

    msg_p->data.arg_grp_ui32x3_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_peerstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_peerstr.ui32_3 = safi;
    memcpy(msg_p->data.arg_grp_ui32x3_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x3_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_DeactivateNeighbor(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_DEACTIVATE_NEIGHBOR;

    msg_p->data.arg_grp_ui32x3_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_peerstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_peerstr.ui32_3 = safi;
    memcpy(msg_p->data.arg_grp_ui32x3_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x3_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetNeighborAdvertisementInterval(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T interval)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_ADVERTISEMENT_INTERVAL;

    msg_p->data.arg_grp_ui32x3_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_peerstr.ui32_2 = interval;
    memcpy(msg_p->data.arg_grp_ui32x2_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x2_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetNeighborAdvertisementInterval(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_ADVERTISEMENT_INTERVAL;

    msg_p->data.arg_grp_ui32_peerstr.ui32 = as_number;
    memcpy(msg_p->data.arg_grp_ui32_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetNeighborAllowasIn(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T allow_num)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_ALLOW_AS_IN;

    msg_p->data.arg_grp_ui32x4_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_4 = allow_num;
    memcpy(msg_p->data.arg_grp_ui32x4_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x4_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}
UI32_T BGP_PMGR_UnsetNeighborAllowasIn(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_ALLOW_AS_IN;

    msg_p->data.arg_grp_ui32x3_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_peerstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_peerstr.ui32_3 = safi;    
    memcpy(msg_p->data.arg_grp_ui32x3_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x3_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

/*  BGP_TYPE_NEIGHBOR_AF_FLAG_XXXXX_UNCHANGED */
UI32_T BGP_PMGR_SetNeighborAttributeUnchanged(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T flag) /* bit0-2: [as-path][med][next-hop] */
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_ATTRIBUTE_UNCHANGED;

    msg_p->data.arg_grp_ui32x4_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_4 = flag;
    memcpy(msg_p->data.arg_grp_ui32x4_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x4_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetNeighborAttributeUnchanged(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T flag)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_ATTRIBUTE_UNCHANGED;

    msg_p->data.arg_grp_ui32x4_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_4 = flag;
    memcpy(msg_p->data.arg_grp_ui32x4_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x4_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

/* BGP_TYPE_NEIGHBOR_FLAG_DYNAMIC_CAPABILITY, route refresh always enabled in quagga */
UI32_T BGP_PMGR_SetNeighborCapability(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T flag) /* bit0-1: dynamic | route-refresh */
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_CAPABILITY;

    msg_p->data.arg_grp_ui32x2_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2_peerstr.ui32_2 = flag;
    memcpy(msg_p->data.arg_grp_ui32x2_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x2_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetNeighborCapability(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T flag)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_CAPABILITY;

    msg_p->data.arg_grp_ui32x2_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2_peerstr.ui32_2 = flag;
    memcpy(msg_p->data.arg_grp_ui32x2_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x2_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

/* BGP_TYPE_NEIGHBOR_AF_FLAG_ORF_PREFIX_SM, BGP_TYPE_NEIGHBOR_AF_FLAG_ORF_PREFIX_RM */
UI32_T BGP_PMGR_SetNeighborCapabilityOrfPrefixList(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T flag) /* bit0-1: send | receive */
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_CAPABILITY_ORF_PREFIX_LIST;

    msg_p->data.arg_grp_ui32x4_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_4 = flag;
    memcpy(msg_p->data.arg_grp_ui32x4_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x4_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetNeighborCapabilityOrfPrefixList(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T flag)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_CAPABILITY_ORF_PREFIX_LIST;

    msg_p->data.arg_grp_ui32x4_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_4 = flag;
    memcpy(msg_p->data.arg_grp_ui32x4_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x4_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetNeighborDefaultOriginate(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], char route_map[BGP_TYPE_ROUTE_MAP_NAME_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_peerstr_routemap);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_DEFAULT_ORIGINATE;

    msg_p->data.arg_grp_ui32x3_peerstr_routemap.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_peerstr_routemap.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_peerstr_routemap.ui32_3 = safi;
    memcpy(msg_p->data.arg_grp_ui32x3_peerstr_routemap.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x3_peerstr_routemap.peerstr));

    if (route_map) /* route_map may be NULL */
        memcpy(msg_p->data.arg_grp_ui32x3_peerstr_routemap.routemap, route_map, sizeof(msg_p->data.arg_grp_ui32x3_peerstr_routemap.routemap));
    else
        memset(msg_p->data.arg_grp_ui32x3_peerstr_routemap.routemap, 0, sizeof(msg_p->data.arg_grp_ui32x3_peerstr_routemap.routemap));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetNeighborDefaultOriginate(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_DEFAULT_ORIGINATE;

    msg_p->data.arg_grp_ui32x3_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_peerstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_peerstr.ui32_3 = safi;
    memcpy(msg_p->data.arg_grp_ui32x3_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x3_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetNeighborDescription(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], char description[BGP_TYPE_NEIGHBOR_DESC_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_peerstr_neighdesc);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_DESCRIPTION;

    msg_p->data.arg_grp_ui32_peerstr_neighdesc.ui32 = as_number;
    memcpy(msg_p->data.arg_grp_ui32_peerstr_neighdesc.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32_peerstr_neighdesc.peerstr));

    if (description) /* description may be NULL */
        memcpy(msg_p->data.arg_grp_ui32_peerstr_neighdesc.neighdesc, description, sizeof(msg_p->data.arg_grp_ui32_peerstr_neighdesc.neighdesc));
    else
        memset(msg_p->data.arg_grp_ui32_peerstr_neighdesc.neighdesc, 0, sizeof(msg_p->data.arg_grp_ui32_peerstr_neighdesc.neighdesc));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetNeighborDescription(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_DESCRIPTION;

    msg_p->data.arg_grp_ui32_peerstr.ui32 = as_number;
    memcpy(msg_p->data.arg_grp_ui32_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetNeighborDistributeList(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], char access_list[BGP_TYPE_ACCESS_LIST_NAME_LEN+1], UI32_T direct) /* direct: in/out */
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4_peerstr_accesslist);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_DISTRIBUTE_LIST;

    msg_p->data.arg_grp_ui32x4_peerstr_accesslist.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x4_peerstr_accesslist.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x4_peerstr_accesslist.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x4_peerstr_accesslist.ui32_4  = direct;
    memcpy(msg_p->data.arg_grp_ui32x4_peerstr_accesslist.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x4_peerstr_accesslist.peerstr));
    memcpy(msg_p->data.arg_grp_ui32x4_peerstr_accesslist.accesslist, access_list, sizeof(msg_p->data.arg_grp_ui32x4_peerstr_accesslist.accesslist));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetNeighborDistributeList(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T direct)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_DISTRIBUTE_LIST;

    msg_p->data.arg_grp_ui32x4_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_4 = direct;
    memcpy(msg_p->data.arg_grp_ui32x4_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x4_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetNeighborFlag(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T flag)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_FLAG;

    msg_p->data.arg_grp_ui32x2_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2_peerstr.ui32_2 = flag;
    memcpy(msg_p->data.arg_grp_ui32x2_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x2_peerstr.peerstr));
    
    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetNeighborFlag(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T flag)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_FLAG;

    msg_p->data.arg_grp_ui32x2_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2_peerstr.ui32_2 = flag;
    memcpy(msg_p->data.arg_grp_ui32x2_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x2_peerstr.peerstr));
    
    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetNeighborAfFlag(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T flag)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_AF_FLAG;

    msg_p->data.arg_grp_ui32x4_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_4 = flag;
    memcpy(msg_p->data.arg_grp_ui32x4_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x4_peerstr.peerstr));
    
    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetNeighborAfFlag(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T flag)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_AF_FLAG;

    msg_p->data.arg_grp_ui32x4_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_4 = flag;
    memcpy(msg_p->data.arg_grp_ui32x4_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x4_peerstr.peerstr));
    
    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetNeighborEbgpMultihop(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T hop_count)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_EBGP_MULTIHOP;

    msg_p->data.arg_grp_ui32x2_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2_peerstr.ui32_2 = hop_count;
    memcpy(msg_p->data.arg_grp_ui32x2_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x2_peerstr.peerstr));
    
    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetNeighborEbgpMultihop(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_EBGP_MULTIHOP;

    msg_p->data.arg_grp_ui32_peerstr.ui32 = as_number;
    memcpy(msg_p->data.arg_grp_ui32_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32_peerstr.peerstr));
    
    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetNeighborFilterList(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], char as_list[BGP_TYPE_ACCESS_LIST_NAME_LEN+1], UI32_T direct)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4_peerstr_aslist);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_FILTER_LIST;

    msg_p->data.arg_grp_ui32x4_peerstr_aslist.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x4_peerstr_aslist.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x4_peerstr_aslist.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x4_peerstr_aslist.ui32_4 = direct;
    memcpy(msg_p->data.arg_grp_ui32x4_peerstr_aslist.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x4_peerstr_aslist.peerstr));
    memcpy(msg_p->data.arg_grp_ui32x4_peerstr_aslist.aslist, as_list, sizeof(msg_p->data.arg_grp_ui32x4_peerstr_aslist.aslist));
    
    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}
UI32_T BGP_PMGR_UnsetNeighborFilterList(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T direct)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_FILTER_LIST;

    msg_p->data.arg_grp_ui32x4_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_4 = direct;
    memcpy(msg_p->data.arg_grp_ui32x4_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x4_peerstr.peerstr));
    
    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetNeighborInterface(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T ifindex)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_INTERFACE;

    msg_p->data.arg_grp_ui32x2_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2_peerstr.ui32_2 = ifindex;
    memcpy(msg_p->data.arg_grp_ui32x2_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x2_peerstr.peerstr));
    
    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetNeighborInterface(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_INTERFACE;

    msg_p->data.arg_grp_ui32_peerstr.ui32 = as_number;
    memcpy(msg_p->data.arg_grp_ui32_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetNeighborMaximumPrefix(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T max_prefix, UI8_T threshold, UI32_T warn_only, UI16_T restart_interval)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x5_ui16_peerstr_ui8);
    
    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_MAXIMUM_PREFIX;

    msg_p->data.arg_grp_ui32x5_ui16_peerstr_ui8.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x5_ui16_peerstr_ui8.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x5_ui16_peerstr_ui8.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x5_ui16_peerstr_ui8.ui32_4 = max_prefix;
    msg_p->data.arg_grp_ui32x5_ui16_peerstr_ui8.ui32_5 = warn_only;
    msg_p->data.arg_grp_ui32x5_ui16_peerstr_ui8.ui16   = restart_interval;
    msg_p->data.arg_grp_ui32x5_ui16_peerstr_ui8.ui8    = threshold;
    memcpy(msg_p->data.arg_grp_ui32x5_ui16_peerstr_ui8.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x5_ui16_peerstr_ui8.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetNeighborMaximumPrefix(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_peerstr);
    
    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_MAXIMUM_PREFIX;

    msg_p->data.arg_grp_ui32x3_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_peerstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_peerstr.ui32_3 = safi;
    memcpy(msg_p->data.arg_grp_ui32x3_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x3_peerstr.peerstr));
    
    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_AddNeighborPeerGroup(UI32_T as_number, char group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_peergroup);

    msg_p->type.cmd = BGP_MGR_IPCCMD_ADD_NEIGHBOR_PEER_GROUP;

    msg_p->data.arg_grp_ui32_peergroup.ui32 = as_number;
    memcpy(msg_p->data.arg_grp_ui32_peergroup.peergroup, group_name, sizeof(msg_p->data.arg_grp_ui32_peergroup.peergroup));
    
    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_DeleteNeighborPeerGroup(UI32_T as_number, char group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_peergroup);

    msg_p->type.cmd = BGP_MGR_IPCCMD_DELETE_NEIGHBOR_PEER_GROUP;

    msg_p->data.arg_grp_ui32_peergroup.ui32 = as_number;
    memcpy(msg_p->data.arg_grp_ui32_peergroup.peergroup, group_name, sizeof(msg_p->data.arg_grp_ui32_peergroup.peergroup));
    
    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_AddNeighborPeerGroupMember(UI32_T as_number, UI32_T afi, UI32_T safi, char group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1], L_INET_AddrIp_T *neighbor_ip_p)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_ipaddr_peergroup);

    msg_p->type.cmd = BGP_MGR_IPCCMD_ADD_NEIGHBOR_PEER_GROUP_MEMBER;

    msg_p->data.arg_grp_ui32x3_ipaddr_peergroup.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_ipaddr_peergroup.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_ipaddr_peergroup.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x3_ipaddr_peergroup.ipaddr = *neighbor_ip_p;
    memcpy(msg_p->data.arg_grp_ui32x3_ipaddr_peergroup.peergroup, group_name, sizeof(msg_p->data.arg_grp_ui32x3_ipaddr_peergroup.peergroup));
    
    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;    
}

UI32_T BGP_PMGR_DeleteNeighborPeerGroupMember(UI32_T as_number, UI32_T afi, UI32_T safi, char group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1], L_INET_AddrIp_T *neighbor_ip_p)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_ipaddr_peergroup);

    msg_p->type.cmd = BGP_MGR_IPCCMD_DELETE_NEIGHBOR_PEER_GROUP_MEMBER;

    msg_p->data.arg_grp_ui32x3_ipaddr_peergroup.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_ipaddr_peergroup.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_ipaddr_peergroup.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x3_ipaddr_peergroup.ipaddr = *neighbor_ip_p;
    memcpy(msg_p->data.arg_grp_ui32x3_ipaddr_peergroup.peergroup, group_name, sizeof(msg_p->data.arg_grp_ui32x3_ipaddr_peergroup.peergroup));
    
    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetNeighborPort(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T port_number)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_PORT;

    msg_p->data.arg_grp_ui32x2_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2_peerstr.ui32_2 = port_number;
    memcpy(msg_p->data.arg_grp_ui32x2_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x2_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;    
}

UI32_T BGP_PMGR_UnsetNeighborPort(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_PORT;

    msg_p->data.arg_grp_ui32_peerstr.ui32 = as_number;
    memcpy(msg_p->data.arg_grp_ui32_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32; 
}

UI32_T BGP_PMGR_SetNeighborPrefixList(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], char prefix_list[BGP_TYPE_PREFIX_LIST_NAME_LEN+1], UI32_T direct)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4_peerstr_prefixlist);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_PREFIX_LIST;

    msg_p->data.arg_grp_ui32x4_peerstr_prefixlist.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x4_peerstr_prefixlist.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x4_peerstr_prefixlist.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x4_peerstr_prefixlist.ui32_4 = direct;
    memcpy(msg_p->data.arg_grp_ui32x4_peerstr_prefixlist.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x4_peerstr_prefixlist.peerstr));
    memcpy(msg_p->data.arg_grp_ui32x4_peerstr_prefixlist.prefixlist, prefix_list, sizeof(msg_p->data.arg_grp_ui32x4_peerstr_prefixlist.prefixlist));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetNeighborPrefixList(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T direct)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_PREFIX_LIST;

    msg_p->data.arg_grp_ui32x4_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_4 = direct;
    memcpy(msg_p->data.arg_grp_ui32x4_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x4_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetNeighborRemoteAs(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T neighbor_as_number)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_REMOTE_AS;

    msg_p->data.arg_grp_ui32x4_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_4 = neighbor_as_number;
    memcpy(msg_p->data.arg_grp_ui32x4_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x4_peerstr.peerstr));
    
    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetNeighborRemoteAs(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_REMOTE_AS;

    msg_p->data.arg_grp_ui32x3_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_peerstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_peerstr.ui32_3 = safi;
    memcpy(msg_p->data.arg_grp_ui32x3_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x3_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetNeighborRouteMap(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], char route_map[BGP_TYPE_ROUTE_MAP_NAME_LEN+1], UI32_T direct)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4_peerstr_routemap);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_ROUTE_MAP;

    msg_p->data.arg_grp_ui32x4_peerstr_routemap.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x4_peerstr_routemap.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x4_peerstr_routemap.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x4_peerstr_routemap.ui32_4 = direct;
    memcpy(msg_p->data.arg_grp_ui32x4_peerstr_routemap.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x4_peerstr_routemap.peerstr));
    memcpy(msg_p->data.arg_grp_ui32x4_peerstr_routemap.routemap, route_map, sizeof(msg_p->data.arg_grp_ui32x4_peerstr_routemap.routemap));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;    
}
    
UI32_T BGP_PMGR_UnsetNeighborRouteMap(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T direct)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_ROUTE_MAP;

    msg_p->data.arg_grp_ui32x4_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x4_peerstr.ui32_4 = direct;
    memcpy(msg_p->data.arg_grp_ui32x4_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x4_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
} 

UI32_T BGP_PMGR_SetNeighborRouteServerClient(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_ROUTE_SERVER_CLIENT;

    msg_p->data.arg_grp_ui32x3_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_peerstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_peerstr.ui32_3 = safi;
    memcpy(msg_p->data.arg_grp_ui32x3_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x3_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
} 

UI32_T BGP_PMGR_UnsetNeighborRouteServerClient(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_ROUTE_SERVER_CLIENT;

    msg_p->data.arg_grp_ui32x3_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_peerstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_peerstr.ui32_3 = safi;
    memcpy(msg_p->data.arg_grp_ui32x3_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x3_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
} 

UI32_T BGP_PMGR_SetNeighborKeepAliveAndHoldTime(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T keepalive, UI32_T holdtime)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_KEEP_ALIVE_AND_HOLD_TIME;

    msg_p->data.arg_grp_ui32x3_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_peerstr.ui32_2 = keepalive;
    msg_p->data.arg_grp_ui32x3_peerstr.ui32_3 = holdtime;
    memcpy(msg_p->data.arg_grp_ui32x3_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x3_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
} 
    
UI32_T BGP_PMGR_UnsetNeighborKeepAliveAndHoldTime(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_KEEP_ALIVE_AND_HOLD_TIME;

    msg_p->data.arg_grp_ui32_peerstr.ui32 = as_number;
    memcpy(msg_p->data.arg_grp_ui32_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetNeighborConnectRetryInterval(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T interval)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_CONNECT_RETRY_INTERVAL;

    msg_p->data.arg_grp_ui32x2_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2_peerstr.ui32_2 = interval;
    memcpy(msg_p->data.arg_grp_ui32x2_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x2_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}
    
UI32_T BGP_PMGR_UnsetNeighborConnectRetryInterval(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_CONNECT_RETRY_INTERVAL;

    msg_p->data.arg_grp_ui32_peerstr.ui32 = as_number;
    memcpy(msg_p->data.arg_grp_ui32_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetNeighborUnsuppressMap(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], char route_map[BGP_TYPE_ROUTE_MAP_NAME_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_peerstr_routemap);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_UNSUPPRESS_MAP;

    msg_p->data.arg_grp_ui32x3_peerstr_routemap.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_peerstr_routemap.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_peerstr_routemap.ui32_3 = safi;
    memcpy(msg_p->data.arg_grp_ui32x3_peerstr_routemap.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x3_peerstr_routemap.peerstr));
    memcpy(msg_p->data.arg_grp_ui32x3_peerstr_routemap.routemap, route_map, sizeof(msg_p->data.arg_grp_ui32x3_peerstr_routemap.routemap));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}
    
UI32_T BGP_PMGR_UnsetNeighborUnsuppressMap(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_UNSUPPRESS_MAP;

    msg_p->data.arg_grp_ui32x3_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_peerstr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_peerstr.ui32_3 = safi;
    memcpy(msg_p->data.arg_grp_ui32x3_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x3_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetNeighborUpdateSource(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T ifindex)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_UPDATE_SOURCE;

    msg_p->data.arg_grp_ui32x2_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2_peerstr.ui32_2 = ifindex;
    memcpy(msg_p->data.arg_grp_ui32x2_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x2_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
} 
    
UI32_T BGP_PMGR_UnsetNeighborUpdateSource(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_UPDATE_SOURCE;

    msg_p->data.arg_grp_ui32_peerstr.ui32 = as_number;
    memcpy(msg_p->data.arg_grp_ui32_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetNeighborWeight(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T weight)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_WEIGHT;

    msg_p->data.arg_grp_ui32x2_peerstr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2_peerstr.ui32_2 = weight;
    memcpy(msg_p->data.arg_grp_ui32x2_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x2_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
} 

UI32_T BGP_PMGR_UnsetNeighborWeight(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_WEIGHT;

    msg_p->data.arg_grp_ui32_peerstr.ui32 = as_number;
    memcpy(msg_p->data.arg_grp_ui32_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_SetNeighborPassword(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], char password[BGP_TYPE_PEER_PASSWORD_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_peerstr_password);

    msg_p->type.cmd = BGP_MGR_IPCCMD_SET_NEIGHBOR_PASSWORD;

    msg_p->data.arg_grp_ui32_peerstr_password.ui32 = as_number;
    memcpy(msg_p->data.arg_grp_ui32_peerstr_password.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32_peerstr_password.peerstr));
    memcpy(msg_p->data.arg_grp_ui32_peerstr_password.password, password, sizeof(msg_p->data.arg_grp_ui32_peerstr_password.password));
    BGP_PMGR_SEND_WAIT_MSG_P();
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_UnsetNeighborPassowrd(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1])
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_UNSET_NEIGHBOR_PASSWORD;

    msg_p->data.arg_grp_ui32_peerstr.ui32 = as_number;
    memcpy(msg_p->data.arg_grp_ui32_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32_peerstr.peerstr));
    BGP_PMGR_SEND_WAIT_MSG_P();
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetBgpInstance(BGP_TYPE_Instance_T *instance_p)
{
    BGP_PMGR_DECLARE_MSG_P(instance);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_BGP_INSTANCE;

    memcpy(&(msg_p->data.instance), instance_p, sizeof(msg_p->data.instance));
    
    BGP_PMGR_SEND_WAIT_MSG_P();

    memcpy(instance_p, &(msg_p->data.instance), sizeof(msg_p->data.instance));
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetNextBgpInstance(BGP_TYPE_Instance_T *instance_p)
{
    BGP_PMGR_DECLARE_MSG_P(instance);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_BGP_INSTANCE;

    memcpy(&(msg_p->data.instance), instance_p, sizeof(msg_p->data.instance));
    
    BGP_PMGR_SEND_WAIT_MSG_P();
    
    memcpy(instance_p, &(msg_p->data.instance), sizeof(msg_p->data.instance));
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetNextNeighborByAsNumber(BGP_TYPE_Neighbor_T *neighbor_p)
{
    const UI32_T msg_size = BGP_MGR_GET_MSG_SIZE(neighbor);
    UI8_T *ipc_buf;
    SYSFUN_Msg_T *msgbuf_p;
    BGP_MGR_IPCMsg_T *msg_p;

    if ((ipc_buf = (UI8_T*)malloc(SYSFUN_SIZE_OF_MSG(msg_size))) == NULL)
        return BGP_TYPE_RESULT_FAIL;

    msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    msg_p = (BGP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msgbuf_p->cmd = SYS_MODULE_BGP;
    msgbuf_p->msg_size = msg_size;
    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_NEIGHBOR_BY_AS_NUMBER;

    memcpy(&(msg_p->data.neighbor), neighbor_p, sizeof(msg_p->data.neighbor));
    
    if(SYSFUN_OK != SYSFUN_SendRequestMsg(msgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size , msgbuf_p))
    {
        SYSFUN_Debug_Printf("%s():SYSFUN_SendRequestMsg fail\n", __FUNCTION__);
        free(ipc_buf);
        return BGP_TYPE_RESULT_SEND_MSG_FAIL;
    }

    memcpy(neighbor_p, &(msg_p->data.neighbor), sizeof(msg_p->data.neighbor));
 
    free(ipc_buf);

    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetNextFilterRoute(UI32_T as_number, UI32_T afi, UI32_T safi, UI32_T router_id,
	  enum BGP_TYPE_SHOW_TYPE type, char *arg_str, 
	  L_INET_AddrIp_T *prefix_p,
      BGP_TYPE_BgpInfo_T *binfo_p)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x5_argstr_ipaddr_binfo);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_FILTER_ROUTE;
    msg_p->data.arg_grp_ui32x5_argstr_ipaddr_binfo.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x5_argstr_ipaddr_binfo.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x5_argstr_ipaddr_binfo.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x5_argstr_ipaddr_binfo.ui32_4 = router_id;
    msg_p->data.arg_grp_ui32x5_argstr_ipaddr_binfo.ui32_5 = (UI32_T) type;

    if(arg_str)
        memcpy(msg_p->data.arg_grp_ui32x5_argstr_ipaddr_binfo.argstr, arg_str, BGP_TYPE_REGULAR_EXP_LEN+1);
    else
        memset(&(msg_p->data.arg_grp_ui32x5_argstr_ipaddr_binfo.argstr), 0, BGP_TYPE_REGULAR_EXP_LEN+1);

    memcpy(&(msg_p->data.arg_grp_ui32x5_argstr_ipaddr_binfo.ipaddr), prefix_p, sizeof(L_INET_AddrIp_T));
    memcpy(&(msg_p->data.arg_grp_ui32x5_argstr_ipaddr_binfo.binfo), binfo_p, sizeof(BGP_TYPE_BgpInfo_T));
    
    BGP_PMGR_SEND_WAIT_MSG_P();

    memcpy(prefix_p, &(msg_p->data.arg_grp_ui32x5_argstr_ipaddr_binfo.ipaddr), sizeof(L_INET_AddrIp_T));
    memcpy(binfo_p, &(msg_p->data.arg_grp_ui32x5_argstr_ipaddr_binfo.binfo), sizeof(BGP_TYPE_BgpInfo_T));
 
    return msg_p->type.result_ui32;
}
#if 0
/*--------------------------------------------------------------------------
 * FUNCTION NAME - BGP_PMGR_GetRunningDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get running data by specified field id and domain id.
 * INPUT  : domain_id - domain id to set
 *                       0 - global data to get
 *                      >0 - domain data to get
 *          field_id  - field id to get (ERPS_TYPE_FieldId_E)
 * OUTPUT : data_p    - pointer to output data
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE   : 1. all output data are represented in UI32_T
 *               except DOMAIN_NODE_ID/DOMAIN_NAME
 *          2. DOMAIN_NODE_ID/DOMAIN_NAME is respresented in UI8_T[]
 *          3. all timer is in ms
 *--------------------------------------------------------------------------
 */
UI32_T BGP_PMGR_GetRunningDataByField(
    //UI32_T  domain_id,
    UI32_T  field_id,
    UI8_T   *data_p)
{

    UI32_T  ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;

    BGP_PMGR_DECLARE_MSG_P(arg_ui32);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_RUNNING_DATA_BY_FLD;

    BGP_PMGR_SEND_WAIT_MSG_P();

 
//    return msg_p->type.result_running_cfg;
    
    if (NULL == data_p)
        return ret;

    BGP_PMGR_SEND_WAIT_MSG_P();

    ret = msg_p->type.result_running_cfg;

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == ret)
    {
        switch (field_id)
        {
            default:
                *((UI32_T *) data_p) = msg_p->data.arg_ui32;
                break;
        }
    }

    return ret;
}
#endif

UI32_T BGP_PMGR_GetNextDataByType(BGP_TYPE_DataUnion_T *data_union_p)
{
    
    BGP_PMGR_DECLARE_MSG_P(data_union);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_DATA_BY_TYPE;
    
    memcpy(&(msg_p->data.data_union), data_union_p, sizeof(BGP_TYPE_DataUnion_T));
    
    BGP_PMGR_SEND_WAIT_MSG_P();

    memcpy(data_union_p, &(msg_p->data.data_union), sizeof(BGP_TYPE_DataUnion_T));
 
    return msg_p->type.result_ui32;
}
UI32_T BGP_PMGR_GetNextMatchRouteInfo
(
    UI32_T as_number, 
    UI32_T afi, 
    UI32_T safi,
	L_INET_AddrIp_T *prefix_p,
    BGP_TYPE_BgpInfo_T *binfo_p, 
    BOOL_T prefix_check
)
{
    
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_bool_ipaddr_binfo);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_MATCH_ROUTE_INFO;
    
    msg_p->data.arg_grp_ui32x3_bool_ipaddr_binfo.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_bool_ipaddr_binfo.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_bool_ipaddr_binfo.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x3_bool_ipaddr_binfo.bool = prefix_check;

    memcpy(&(msg_p->data.arg_grp_ui32x3_bool_ipaddr_binfo.ipaddr), prefix_p, sizeof(L_INET_AddrIp_T));
    memcpy(&(msg_p->data.arg_grp_ui32x3_bool_ipaddr_binfo.binfo), binfo_p, sizeof(BGP_TYPE_BgpInfo_T));
    
    BGP_PMGR_SEND_WAIT_MSG_P();

    memcpy(prefix_p, &(msg_p->data.arg_grp_ui32x3_bool_ipaddr_binfo.ipaddr), sizeof(L_INET_AddrIp_T));
    memcpy(binfo_p, &(msg_p->data.arg_grp_ui32x3_bool_ipaddr_binfo.binfo), sizeof(BGP_TYPE_BgpInfo_T));
   
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetNextAdvertiseNeighborAddrByPrefix(UI32_T as_number, UI32_T afi, UI32_T safi, L_INET_AddrIp_T *neighbor_addr_p, L_INET_AddrIp_T *prefix_p)
{
    
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_ipaddrx2);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_ADVERTISE_NEIGHBOR_BY_PREFIX;
    
    msg_p->data.arg_grp_ui32x3_ipaddrx2.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_ipaddrx2.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_ipaddrx2.ui32_3 = safi;

    memcpy(&(msg_p->data.arg_grp_ui32x3_ipaddrx2.ipaddr_1), neighbor_addr_p, sizeof(L_INET_AddrIp_T));
    memcpy(&(msg_p->data.arg_grp_ui32x3_ipaddrx2.ipaddr_2), prefix_p, sizeof(L_INET_AddrIp_T));
    
    BGP_PMGR_SEND_WAIT_MSG_P();

    memcpy(neighbor_addr_p, &(msg_p->data.arg_grp_ui32x3_ipaddrx2.ipaddr_1), sizeof(L_INET_AddrIp_T));
   
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetNextClusterId(UI32_T as_number, UI32_T afi, UI32_T safi, L_INET_AddrIp_T *prefix_p, BGP_TYPE_BgpInfo_T *binfo_p, UI32_T *cluster_id_p)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4_prefix_binfo);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_CLUSTER_ID;
    
    msg_p->data.arg_grp_ui32x4_prefix_binfo.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x4_prefix_binfo.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x4_prefix_binfo.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x4_prefix_binfo.ui32_4 = *cluster_id_p;

    memcpy(&(msg_p->data.arg_grp_ui32x4_prefix_binfo.prefix), prefix_p, sizeof(L_INET_AddrIp_T));
    memcpy(&(msg_p->data.arg_grp_ui32x4_prefix_binfo.binfo), binfo_p, sizeof(BGP_TYPE_BgpInfo_T));
    
    BGP_PMGR_SEND_WAIT_MSG_P();

    *cluster_id_p = msg_p->data.arg_grp_ui32x4_prefix_binfo.ui32_4;
   
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetScanInfo(BOOL_T *is_running, UI32_T *scan_interval)
{
   
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_bool);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_SCAN_INFO;
    
    msg_p->data.arg_grp_ui32_bool.ui32 = *scan_interval;
    msg_p->data.arg_grp_ui32_bool.bool = *is_running;
    
    BGP_PMGR_SEND_WAIT_MSG_P();

    *scan_interval = msg_p->data.arg_grp_ui32_bool.ui32;
    *is_running = msg_p->data.arg_grp_ui32_bool.bool;
   
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetNextNexthopCache(UI32_T as_number, UI32_T afi, L_INET_AddrIp_T *prefix_p, BGP_TYPE_BgpNexthopCache_T *bnc_p)
{
    
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2_prefix_bnc);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_NEXTHOP_CACHE;
    
    msg_p->data.arg_grp_ui32x2_prefix_bnc.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2_prefix_bnc.ui32_2 = afi;

    memcpy(&(msg_p->data.arg_grp_ui32x2_prefix_bnc.prefix), prefix_p, sizeof(L_INET_AddrIp_T));
    memcpy(&(msg_p->data.arg_grp_ui32x2_prefix_bnc.bnc), bnc_p, sizeof(BGP_TYPE_BgpNexthopCache_T));
    
    BGP_PMGR_SEND_WAIT_MSG_P();

    memcpy(prefix_p, &(msg_p->data.arg_grp_ui32x2_prefix_bnc.prefix), sizeof(L_INET_AddrIp_T));
    memcpy(bnc_p, &(msg_p->data.arg_grp_ui32x2_prefix_bnc.bnc), sizeof(BGP_TYPE_BgpNexthopCache_T));
   
    return msg_p->type.result_ui32;
}
UI32_T BGP_PMGR_GetNextConnectedRoute(UI32_T as_number, UI32_T afi, L_INET_AddrIp_T *prefix_p)
{
    
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2_ipaddr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_CONNECTED_ROUTE;
    
    msg_p->data.arg_grp_ui32x2_ipaddr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x2_ipaddr.ui32_2 = afi;

    memcpy(&(msg_p->data.arg_grp_ui32x2_ipaddr.ipaddr), prefix_p, sizeof(L_INET_AddrIp_T));
    
    BGP_PMGR_SEND_WAIT_MSG_P();

    memcpy(prefix_p, &(msg_p->data.arg_grp_ui32x2_ipaddr.ipaddr), sizeof(L_INET_AddrIp_T));
   
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetDampeningParameters(BGP_TYPE_DampeningParameters_T *param_p)
{
    
    BGP_PMGR_DECLARE_MSG_P(dampening_param);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_DAMPENING_PARAMETERS;
    
    memcpy(&(msg_p->data.dampening_param), param_p, sizeof(*param_p));
    
    BGP_PMGR_SEND_WAIT_MSG_P();

    memcpy(param_p, &(msg_p->data.dampening_param), sizeof(*param_p));
   
    return msg_p->type.result_ui32;
}

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
UI32_T BGP_PMGR_GetNextPrefix(UI32_T as_number, UI32_T afi, UI32_T safi, L_INET_AddrIp_T *prefix_p)
{
    
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x3_ipaddr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_PREFIX;
    
    msg_p->data.arg_grp_ui32x3_ipaddr.ui32_1 = as_number;
    msg_p->data.arg_grp_ui32x3_ipaddr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x3_ipaddr.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x3_ipaddr.ipaddr = *prefix_p;

    BGP_PMGR_SEND_WAIT_MSG_P();

    *prefix_p = msg_p->data.arg_grp_ui32x3_ipaddr.ipaddr;

    return msg_p->type.result_ui32;
}

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
UI32_T BGP_PMGR_GetNextAdjAttrByPrefix(UI32_T as_number, UI32_T afi, UI32_T safi, L_INET_AddrIp_T *prefix_p, BOOL_T in, UI32_T *index_p, L_INET_AddrIp_T *neighbor_addr_p, BGP_TYPE_Attr_T *attr_p)
{
    
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x4_bool_ipaddrx2_attr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_ADJ_ATTR_BY_PREFIX;
    
    msg_p->data.arg_grp_ui32x4_bool_ipaddrx2_attr.ui32_1 = as_number; 
    msg_p->data.arg_grp_ui32x4_bool_ipaddrx2_attr.ui32_2 = afi;
    msg_p->data.arg_grp_ui32x4_bool_ipaddrx2_attr.ui32_3 = safi;
    msg_p->data.arg_grp_ui32x4_bool_ipaddrx2_attr.ipaddr_1 = *prefix_p;
    msg_p->data.arg_grp_ui32x4_bool_ipaddrx2_attr.bool = in;
    msg_p->data.arg_grp_ui32x4_bool_ipaddrx2_attr.ui32_4 = *index_p;
    msg_p->data.arg_grp_ui32x4_bool_ipaddrx2_attr.ipaddr_2 = *neighbor_addr_p;
    memcpy(&msg_p->data.arg_grp_ui32x4_bool_ipaddrx2_attr.attr, attr_p, sizeof(*attr_p));
        
    BGP_PMGR_SEND_WAIT_MSG_P();

    *index_p = msg_p->data.arg_grp_ui32x4_bool_ipaddrx2_attr.ui32_4;
    *neighbor_addr_p = msg_p->data.arg_grp_ui32x4_bool_ipaddrx2_attr.ipaddr_2;
    memcpy(attr_p, &msg_p->data.arg_grp_ui32x4_bool_ipaddrx2_attr.attr, sizeof(*attr_p));
   
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetOrfPrefixList(NSM_POLICY_TYPE_PrefixList_T *prefix_list_info_p)
{
    BGP_PMGR_DECLARE_MSG_P(prefix_list_info);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_ORF_PREFIX_LIST;

    memcpy(&msg_p->data.prefix_list_info, prefix_list_info_p, sizeof(msg_p->data.prefix_list_info));

    BGP_PMGR_SEND_WAIT_MSG_P();
    memcpy(prefix_list_info_p, &msg_p->data.prefix_list_info, sizeof(msg_p->data.prefix_list_info));
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetNextOrfPrefixListEntry(NSM_POLICY_TYPE_PrefixListEntry_T *prefix_list_entry_info_p)
{
    BGP_PMGR_DECLARE_MSG_P(prefix_list_entry_info);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEXT_ORF_PREFIX_LIST_ENTRY;

    memcpy(&msg_p->data.prefix_list_entry_info, prefix_list_entry_info_p, sizeof(msg_p->data.prefix_list_entry_info));

    BGP_PMGR_SEND_WAIT_MSG_P();

    memcpy(prefix_list_entry_info_p, &msg_p->data.prefix_list_entry_info, sizeof(msg_p->data.prefix_list_entry_info));
    
    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_GetNeighborPeerSort(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T *peer_sort_p)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32x2_peerstr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_GET_NEIGHBOR_PEER_SORT;

    msg_p->data.arg_grp_ui32x2_peerstr.ui32_1 = as_number;
    memcpy(msg_p->data.arg_grp_ui32x2_peerstr.peerstr, peer_str, sizeof(msg_p->data.arg_grp_ui32x2_peerstr.peerstr));

    BGP_PMGR_SEND_WAIT_MSG_P();

    *peer_sort_p = msg_p->data.arg_grp_ui32x2_peerstr.ui32_2;

    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_MIB_GetBgpVersion(char version_str[MAXSIZE_bgpVersion+1])
{
    BGP_PMGR_DECLARE_MSG_P(bgp_version);

    msg_p->type.cmd = BGP_MGR_IPCCMD_MIB_GET_BGP_VERSION;

    BGP_PMGR_SEND_WAIT_MSG_P();

    memcpy(version_str, &msg_p->data.bgp_version.version, sizeof(msg_p->data.bgp_version.version));

    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_MIB_GetBgpLocalAs(UI32_T *local_as_p)
{
    BGP_PMGR_DECLARE_MSG_P(arg_ui32);

    msg_p->type.cmd = BGP_MGR_IPCCMD_MIB_GET_BGP_LOCAL_AS;

    BGP_PMGR_SEND_WAIT_MSG_P();

    *local_as_p = msg_p->data.arg_ui32;

    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_MIB_GetBgpPeerEntry(BGP_TYPE_MIB_BgpPeerEntry_T *peer_entry_p)
{
    BGP_PMGR_DECLARE_MSG_P(bgp_peer_entry);

    msg_p->type.cmd = BGP_MGR_IPCCMD_MIB_GET_BGP_PEER_ENTRY;
    msg_p->data.bgp_peer_entry = *peer_entry_p;

    BGP_PMGR_SEND_WAIT_MSG_P();

    *peer_entry_p = msg_p->data.bgp_peer_entry;

    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_MIB_GetNextBgpPeerEntry(BGP_TYPE_MIB_BgpPeerEntry_T *peer_entry_p)
{
    BGP_PMGR_DECLARE_MSG_P(bgp_peer_entry);

    msg_p->type.cmd = BGP_MGR_IPCCMD_MIB_GET_NEXT_BGP_PEER_ENTRY;
    msg_p->data.bgp_peer_entry = *peer_entry_p;

    BGP_PMGR_SEND_WAIT_MSG_P();

    *peer_entry_p = msg_p->data.bgp_peer_entry;

    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_MIB_SetBgpPeerAdminStatus(L_INET_AddrIp_T *neighbor_p, I32_T status)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_ipaddr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_MIB_SET_BGP_PEER_ADMIN_STATUS;
    msg_p->data.arg_grp_ui32_ipaddr.ipaddr = *neighbor_p;
    msg_p->data.arg_grp_ui32_ipaddr.ui32   = (UI32_T)status;

    BGP_PMGR_SEND_WAIT_MSG_P();

    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_MIB_SetBgpPeerConnectRetryInterval(L_INET_AddrIp_T *neighbor_p, I32_T interval)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_ipaddr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_MIB_SET_BGP_PEER_CONNECT_RETRY_INTERVAL;
    msg_p->data.arg_grp_ui32_ipaddr.ipaddr = *neighbor_p;
    msg_p->data.arg_grp_ui32_ipaddr.ui32   = (UI32_T)interval;

    BGP_PMGR_SEND_WAIT_MSG_P();

    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_MIB_SetBgpPeerHoldTimeConfigured(L_INET_AddrIp_T *neighbor_p, I32_T hold_time)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_ipaddr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_MIB_SET_BGP_PEER_HOLD_TIME_CONFIGURED;
    msg_p->data.arg_grp_ui32_ipaddr.ipaddr = *neighbor_p;
    msg_p->data.arg_grp_ui32_ipaddr.ui32   = (UI32_T)hold_time;

    BGP_PMGR_SEND_WAIT_MSG_P();

    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_MIB_SetBgpPeerKeepAliveConfigured(L_INET_AddrIp_T *neighbor_p, I32_T keep_alive)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_ipaddr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_MIB_SET_BGP_PEER_KEEP_ALIVE_CONFIGURED;
    msg_p->data.arg_grp_ui32_ipaddr.ipaddr = *neighbor_p;
    msg_p->data.arg_grp_ui32_ipaddr.ui32   = (UI32_T)keep_alive;

    BGP_PMGR_SEND_WAIT_MSG_P();

    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_MIB_SetBgpPeerMinASOriginationInterval(L_INET_AddrIp_T *neighbor_p, I32_T as_orig)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_ipaddr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_MIB_SET_BGP_PEER_MIN_AS_ORIGINATION_INTERVAL;
    msg_p->data.arg_grp_ui32_ipaddr.ipaddr = *neighbor_p;
    msg_p->data.arg_grp_ui32_ipaddr.ui32   = (UI32_T)as_orig;

    BGP_PMGR_SEND_WAIT_MSG_P();

    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_MIB_SetBgpPeerMinRouteAdvertisementInterval(L_INET_AddrIp_T *neighbor_p, I32_T route_adv)
{
    BGP_PMGR_DECLARE_MSG_P(arg_grp_ui32_ipaddr);

    msg_p->type.cmd = BGP_MGR_IPCCMD_MIB_SET_BGP_PEER_MIN_ROUTE_ADVERTISEMENT_INTERVAL;
    msg_p->data.arg_grp_ui32_ipaddr.ipaddr = *neighbor_p;
    msg_p->data.arg_grp_ui32_ipaddr.ui32   = (UI32_T)route_adv;

    BGP_PMGR_SEND_WAIT_MSG_P();

    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_MIB_GetBgpIdentifier(UI32_T *bgp_identifier_p)
{
    BGP_PMGR_DECLARE_MSG_P(arg_ui32);

    msg_p->type.cmd = BGP_MGR_IPCCMD_MIB_GET_BGP_IDENTIFIER;

    BGP_PMGR_SEND_WAIT_MSG_P();

    *bgp_identifier_p = msg_p->data.arg_ui32;

    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_MIB_GetBgp4PathAttrEntry(BGP_TYPE_MIB_Bgp4PathAttrEntry_T *path_attr_p)
{
    BGP_PMGR_DECLARE_MSG_P(bgp4_path_attr_entry);

    msg_p->type.cmd = BGP_MGR_IPCCMD_MIB_GET_BGP4_PATH_ATTR_ENTRY;
    msg_p->data.bgp4_path_attr_entry = *path_attr_p;

    BGP_PMGR_SEND_WAIT_MSG_P();

    *path_attr_p = msg_p->data.bgp4_path_attr_entry;

    return msg_p->type.result_ui32;
}

UI32_T BGP_PMGR_MIB_GetNextBgp4PathAttrEntry(BGP_TYPE_MIB_Bgp4PathAttrEntry_T *path_attr_p)
{
    BGP_PMGR_DECLARE_MSG_P(bgp4_path_attr_entry);

    msg_p->type.cmd = BGP_MGR_IPCCMD_MIB_GET_NEXT_BGP4_PATH_ATTR_ENTRY;
    msg_p->data.bgp4_path_attr_entry = *path_attr_p;

    BGP_PMGR_SEND_WAIT_MSG_P();

    *path_attr_p = msg_p->data.bgp4_path_attr_entry;

    return msg_p->type.result_ui32;
}


