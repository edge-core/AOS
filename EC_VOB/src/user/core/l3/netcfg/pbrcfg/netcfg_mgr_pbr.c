/* MODULE NAME:  netcfg_mgr_pbr.c
 * PURPOSE:
 *     NETCFG PBR MGR APIs.
 *
 * NOTES:
 *
 * HISTORY:
 *    2015/07/10     KH Shi, Create
 *
 * Copyright(C)      Accton Corporation, 2015
 */
#include <string.h>
#include "sysfun.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_PBR == TRUE)
#include "sys_type.h"
#include "vlan_lib.h"
#include "rule_type.h"
#include "l4_pmgr.h"
#include "amtrl3_pmgr.h"
#include "amtrl3_type.h"
#include "netcfg_type.h"
#include "netcfg_pmgr_route.h"
#include "netcfg_mgr_route.h"
#include "netcfg_om_ip.h"
#include "netcfg_mgr_pbr.h"
#include "netcfg_om_pbr.h"
#include "nsm_policy_type.h"
#include "bgp_pmgr.h"
#include "amtrl3_om.h"
#include "amtrl3_pom.h"
#include "sys_callback_mgr.h"
#include "ip_lib.h"

#define ROUTE_MAP_MATCH_IP_ADDRESS_STR  "ip address"
#define ROUTE_MAP_SET_DSCP_STR          "ip dscp"
#define ROUTE_MAP_SET_NEXTHOP_STR       "ip next-hop"

SYSFUN_DECLARE_CSC

#define NETCFG_MGR_PBR_CHECK_OPER_MODE(RET_VAL)        \
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)      \
    {  \
        return RET_VAL; \
    }

#define NETCFG_MGR_PBR_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE()        \
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)      \
    {  \
        return; \
    }


/* LOCAL FUNCTIONS DECLARATION
 */
static UI32_T NETCFG_MGR_PBR_BindRouteMap(UI32_T vid, char *rmap_name);
static UI32_T NETCFG_MGR_PBR_UnbindRouteMap(UI32_T vid);
static UI32_T NETCFG_MGR_PBR_GetActiveVlanByIp(L_INET_AddrIp_T *nexthop_p, UI32_T *vid_ifindex);
static UI32_T NETCFG_MGR_PBR_ProcessAddToAmtrl3(UI32_T vid, char *rmap_name, UI32_T seq_num);
static UI32_T NETCFG_MGR_PBR_ProcessAddToChip(UI32_T vid, char *rmap_name, UI32_T seq_num);
static UI32_T NETCFG_MGR_PBR_ProcessDeleteFromAmtrl3(UI32_T vid, char *rmap_name, UI32_T seq_num);
static UI32_T NETCFG_MGR_PBR_ProcessDeleteFromChip(UI32_T vid, char *rmap_name, UI32_T seq_num);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ERPS_MGR_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: This function sets the component to temporary transition mode.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 *--------------------------------------------------------------------------
 */
void NETCFG_MGR_PBR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_PBR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: It's the temporary transition mode between system into master
 *          mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void NETCFG_MGR_PBR_EnterTransitionMode(void)
{
    NETCFG_OM_PBR_ClearAll();
    SYSFUN_ENTER_TRANSITION_MODE();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_PBR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE: Enable PBR operation while in master mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void NETCFG_MGR_PBR_EnterMasterMode(void)
{
    SYSFUN_ENTER_MASTER_MODE();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_PBR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE: Disable the PBR operation while in slave mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void NETCFG_MGR_PBR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_PBR_InitiateSystemResources
 *--------------------------------------------------------------------------
 * PURPOSE: This function will initialize system resouce for PBR.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void NETCFG_MGR_PBR_InitiateSystemResources(void)
{
    NETCFG_OM_PBR_Initiate_System_Resources();
}

/*------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_PBR_Create_InterCSC_Relation
 *------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 *------------------------------------------------------------------------*/
void NETCFG_MGR_PBR_Create_InterCSC_Relation(void)
{
    /* Init backdoor call back functions
     */
}

/* FUNCTION NAME : NETCFG_MGR_PBR_HandleIPCReqMsg
 * PURPOSE:
 *      Handle the ipc request received from mgr queue.
 *
 * INPUT:
 *      sysfun_msg_p  --  The ipc request for PBR_MGR.
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
BOOL_T NETCFG_MGR_PBR_HandleIPCReqMsg(SYSFUN_Msg_T *sysfun_msg_p)
{
    NETCFG_MGR_PBR_IPCMsg_T* msg_p;

    if(sysfun_msg_p==NULL)
    {
        return FALSE;
    }

    msg_p = (NETCFG_MGR_PBR_IPCMsg_T*)sysfun_msg_p->msg_buf;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        msg_p->type.result_bool = FALSE;
        sysfun_msg_p->msg_size= NETCFG_MGR_PBR_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    switch(msg_p->type.cmd)
    {
        case NETCFG_MGR_PBR_IPCCMD_BIND_ROUTE_MAP:
            msg_p->type.result_ui32 = NETCFG_MGR_PBR_BindRouteMap(msg_p->data.arg_grp_bind_routemap.vid,
                                                                  msg_p->data.arg_grp_bind_routemap.rmap_name);
            sysfun_msg_p->msg_size = NETCFG_MGR_PBR_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_PBR_IPCCMD_UNBIND_ROUTE_MAP:
            msg_p->type.result_ui32 = NETCFG_MGR_PBR_UnbindRouteMap(msg_p->data.vid);
            sysfun_msg_p->msg_size = NETCFG_MGR_PBR_IPCMSG_TYPE_SIZE;
            break;

        default:
            msg_p->type.result_ui32 = NETCFG_TYPE_INVALID_ARG;
            sysfun_msg_p->msg_size = sizeof(msg_p->type);
            break;
    }

    return TRUE;
}

void NETCFG_MGR_PBR_SignalL3IfRifUp(UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
    NETCFG_OM_PBR_BindingEntry_T pbr_binding;
    AMTRL3_TYPE_InetHostRouteEntry_T host_entry;

    NETCFG_MGR_PBR_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    if (addr_p->type != L_INET_ADDR_TYPE_IPV4)
        return;

    memset(&pbr_binding, 0, sizeof(pbr_binding));
    IP_LIB_GetPrefixAddr(addr_p->addr, addr_p->addrlen, addr_p->preflen, pbr_binding.nexthop.addr);
    pbr_binding.nexthop.type = L_INET_ADDR_TYPE_IPV4;
    pbr_binding.nexthop.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    pbr_binding.nexthop.preflen = SYS_ADPT_IPV4_ADDR_LEN;

    while (NETCFG_OM_PBR_GetNextBindingEntryByNextHop(&pbr_binding))
    {
        if (!IP_LIB_IsIpBelongToSubnet(addr_p->addr, addr_p->preflen, pbr_binding.nexthop.addr))
            break;

        if (!pbr_binding.in_chip)
        {
            if (pbr_binding.option & NETCFG_OM_PBR_BINDING_OPT_NEXTHOP)
            {
                if (!pbr_binding.in_amtrl3)
                {
                    memset(&host_entry, 0, sizeof(host_entry));
                    host_entry.dst_vid_ifindex = ifindex;
                    host_entry.dst_inet_addr = pbr_binding.nexthop;
                    if (AMTRL3_PMGR_SetHostRouteForPbr(AMTRL3_TYPE_FLAGS_IPV4, SYS_ADPT_DEFAULT_FIB, &host_entry))
                    {
                        pbr_binding.nexthop_ifindex = ifindex;
                        pbr_binding.in_amtrl3 = TRUE;
                        NETCFG_OM_PBR_AddBindingEntry(&pbr_binding);
                    }
                }
            }
        }
    }
}

void NETCFG_MGR_PBR_SignalL3IfRifDown(UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
    NETCFG_OM_PBR_BindingEntry_T pbr_binding;
    AMTRL3_TYPE_InetHostRouteEntry_T host_entry;
    UI32_T acl_index = 0;
    BOOL_T is_modified;

    NETCFG_MGR_PBR_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    if (addr_p->type != L_INET_ADDR_TYPE_IPV4)
        return;

    memset(&pbr_binding, 0, sizeof(pbr_binding));
    IP_LIB_GetPrefixAddr(addr_p->addr, addr_p->addrlen, addr_p->preflen, pbr_binding.nexthop.addr);
    pbr_binding.nexthop.type = L_INET_ADDR_TYPE_IPV4;
    pbr_binding.nexthop.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    pbr_binding.nexthop.preflen = SYS_ADPT_IPV4_ADDR_LEN;

    while (NETCFG_OM_PBR_GetNextBindingEntryByNextHop(&pbr_binding))
    {
        if (!IP_LIB_IsIpBelongToSubnet(addr_p->addr, addr_p->preflen, pbr_binding.nexthop.addr))
            break;

        if (pbr_binding.option & NETCFG_OM_PBR_BINDING_OPT_NEXTHOP)
        {
            is_modified = FALSE;
            if (pbr_binding.in_chip)
            {
                if (L4_PMGR_ACL_GetAclIdByName(pbr_binding.acl_name, &acl_index) == RULE_TYPE_OK)
                {
                    L4_PMGR_PBR_UnbindAcl(pbr_binding.vid, pbr_binding.seq_num, acl_index);
                }

                pbr_binding.in_chip = FALSE;
                is_modified = TRUE;
            }

            if (pbr_binding.in_amtrl3)
            {
                memset(&host_entry, 0, sizeof(host_entry));
                host_entry.dst_vid_ifindex = ifindex;
                host_entry.dst_inet_addr = pbr_binding.nexthop;
                if (AMTRL3_PMGR_DeleteHostRouteForPbr(AMTRL3_TYPE_FLAGS_IPV4, SYS_ADPT_DEFAULT_FIB, &host_entry))
                {
                    pbr_binding.in_amtrl3 = FALSE;
                    is_modified = TRUE;
                }
            }

            if (is_modified)
                NETCFG_OM_PBR_AddBindingEntry(&pbr_binding);
        }
    }
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_MGR_PBR_HostRouteChangedCallbackHandler
 * -------------------------------------------------------------------------
 * PURPOSE:  callback function when host route changed
 * INPUT:    addr_p         -- the host route ip address
 *           is_unresolved  -- is host route unresolved
 * OUTPUT:
 * RETURN:
 * NOTES:
 * -------------------------------------------------------------------------
 */
void NETCFG_MGR_PBR_HostRouteChangedCallbackHandler(L_INET_AddrIp_T *addr_p, BOOL_T is_unresolved)
{
    NETCFG_OM_PBR_BindingEntry_T pbr_binding;
    AMTRL3_TYPE_InetHostRouteEntry_T host_entry;
    UI32_T acl_index = 0;
    BOOL_T is_modified;

    NETCFG_MGR_PBR_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    memset(&pbr_binding, 0, sizeof(pbr_binding));
    pbr_binding.nexthop = *addr_p;
    while (NETCFG_OM_PBR_GetNextBindingEntryByExactNextHop(&pbr_binding))
    {
        if (is_unresolved)
        {
            if (pbr_binding.in_chip &&
                (pbr_binding.option & NETCFG_OM_PBR_BINDING_OPT_NEXTHOP) &&
                pbr_binding.nexthop_hwinfo != AMTRL3_OM_HW_INFO_INVALID)
            {
                NETCFG_MGR_PBR_ProcessDeleteFromChip(pbr_binding.vid, pbr_binding.rmap_name, pbr_binding.seq_num);
                NETCFG_MGR_PBR_ProcessAddToChip(pbr_binding.vid, pbr_binding.rmap_name, pbr_binding.seq_num);
            }
        }
        else
        {
            if (pbr_binding.in_chip)
                NETCFG_MGR_PBR_ProcessDeleteFromChip(pbr_binding.vid, pbr_binding.rmap_name, pbr_binding.seq_num);
            NETCFG_MGR_PBR_ProcessAddToChip(pbr_binding.vid, pbr_binding.rmap_name, pbr_binding.seq_num);
        }
    }
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_MGR_PBR_AclChangedCallbackHandler
 * -------------------------------------------------------------------------
 * PURPOSE:  callback handler function for acl change
 * INPUT:    acl_index -- acl index
 *           acl_name  -- acl name
 *           type      -- change type (ACL_CHANGE_TYPE_ADD/DELETE/MODIFY)
 * OUTPUT:
 * RETURN:
 * NOTES:
 * -------------------------------------------------------------------------
 */
void NETCFG_MGR_PBR_AclChangedCallbackHandler(UI32_T acl_index, char acl_name[SYS_ADPT_MAX_ACCESS_LIST_NAME_LENGTH+1], UI8_T type)
{
    NETCFG_OM_PBR_BindingEntry_T pbr_binding;

    NETCFG_MGR_PBR_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    memset(&pbr_binding, 0, sizeof(pbr_binding));
    strncpy(pbr_binding.acl_name, acl_name, SYS_ADPT_MAX_ACCESS_LIST_NAME_LENGTH+1);
    while (NETCFG_OM_PBR_GetNextBindingEntryByAclName(&pbr_binding))
    {
        if (type == ACL_CHANGE_TYPE_DELETE || type == ACL_CHANGE_TYPE_MODIFY)
            NETCFG_MGR_PBR_ProcessDeleteFromChip(pbr_binding.vid, pbr_binding.rmap_name, pbr_binding.seq_num);
        if (type == ACL_CHANGE_TYPE_ADD || type == ACL_CHANGE_TYPE_MODIFY)
            NETCFG_MGR_PBR_ProcessAddToChip(pbr_binding.vid, pbr_binding.rmap_name, pbr_binding.seq_num);
    }
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_MGR_PBR_RoutemapChangedCallbackHandler
 * -------------------------------------------------------------------------
 * PURPOSE:  callback function when modify acl.
 * INPUT:    rmap_name  -- route-map name
 *           seq_num    -- sequence number
 *           is_deleted -- whether it is deleted
 * OUTPUT:   None
 * RETURN:   None
 * NOTES:
 * -------------------------------------------------------------------------
 */
void NETCFG_MGR_PBR_RoutemapChangedCallbackHandler(char rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1], UI32_T seq_num, BOOL_T is_deleted)
{
    NSM_POLICY_TYPE_RouteMap_T rmap;
    NSM_POLICY_TYPE_RouteMapIndex_T rmap_index;
    NSM_POLICY_TYPE_RouteMapRule_T rmap_rule;
    NETCFG_OM_PBR_BindingEntry_T pbr_binding;
    NETCFG_OM_PBR_BindingEntry_T old_pbr_binding;
    char acl_name[NSM_POLICY_TYPE_ROUTE_MAP_ARG_LEN+1];
    L_INET_AddrIp_T nexthop;
    UI32_T vid;
    UI8_T  dscp = 0;
    UI8_T  option;

    NETCFG_MGR_PBR_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    memset(rmap.name, 0, sizeof(rmap.name));
    strncpy(rmap.name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
    if (BGP_PMGR_GetRouteMap(&rmap) != BGP_TYPE_RESULT_OK) /* route-map was deleted or not used */
    {
        vid = 0;
        while (NETCFG_OM_PBR_GetNextBindingVlan(rmap_name, &vid))
        {
            NETCFG_MGR_PBR_ProcessDeleteFromAmtrl3(vid, rmap_name, 0);
            NETCFG_MGR_PBR_ProcessDeleteFromChip(vid, rmap_name, 0);
            NETCFG_OM_PBR_DeleteEntriesByVidAndRouteMap(vid, rmap_name);
        }
    }
    else
    {
        vid = 0;
        while (NETCFG_OM_PBR_GetNextBindingVlan(rmap_name, &vid))
        {
            if (is_deleted)
            {
                memset(&pbr_binding, 0, sizeof(pbr_binding));
                pbr_binding.vid = vid;
                strncpy(pbr_binding.rmap_name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
                if (seq_num)
                    pbr_binding.seq_num = seq_num - 1; /* in order to get exact seq_num when getnext */
                while (NETCFG_OM_PBR_GetNextBindingEntry(&pbr_binding))
                {
                    if (pbr_binding.vid != vid || 0 != strncmp(pbr_binding.rmap_name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH))
                        break;
                    if (seq_num != 0 && pbr_binding.seq_num != seq_num)
                        break;

                    NETCFG_MGR_PBR_ProcessDeleteFromAmtrl3(vid, rmap_name, pbr_binding.seq_num);
                    NETCFG_MGR_PBR_ProcessDeleteFromChip(vid, rmap_name, pbr_binding.seq_num);
                    NETCFG_OM_PBR_DeleteBindingEntry(&pbr_binding);
                }
            }
            else
            {
                memset(&rmap_index, 0, sizeof(rmap_index));
                strncpy(rmap_index.map_name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
                rmap_index.pref = seq_num;
                if (BGP_PMGR_GetRouteMapIndex(&rmap_index) == BGP_TYPE_RESULT_OK)
                {
                    memset(&rmap_rule, 0, sizeof(rmap_rule));
                    strncpy(rmap_rule.map_name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
                    rmap_rule.index_type = rmap_index.type;
                    rmap_rule.index_pref = rmap_index.pref;
                    memset(acl_name, 0, sizeof(acl_name));
                    while(BGP_PMGR_GetNextRouteMapMatch(&rmap_rule) == BGP_TYPE_RESULT_OK)
                    {
                        if (0 == memcmp(rmap_rule.cmd_str, ROUTE_MAP_MATCH_IP_ADDRESS_STR, sizeof(ROUTE_MAP_MATCH_IP_ADDRESS_STR)))
                        {
                            strncpy(acl_name, rmap_rule.rule_str, NSM_POLICY_TYPE_ROUTE_MAP_ARG_LEN+1);
                            break;
                        }
                    }

                    option = 0;
                    dscp = 0;
                    memset(&nexthop, 0, sizeof(nexthop));
                    memset(&rmap_rule, 0, sizeof(rmap_rule));
                    strncpy(rmap_rule.map_name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
                    rmap_rule.index_type = rmap_index.type;
                    rmap_rule.index_pref = rmap_index.pref;
                    while(BGP_PMGR_GetNextRouteMapSet(&rmap_rule) == BGP_TYPE_RESULT_OK)
                    {
                        if (0 == memcmp(rmap_rule.cmd_str, ROUTE_MAP_SET_DSCP_STR, sizeof(ROUTE_MAP_SET_DSCP_STR)))
                        {
                            dscp = atoi(rmap_rule.rule_str);
                            option |= NETCFG_OM_PBR_BINDING_OPT_DSCP;
                        }

                        if (0 == memcmp(rmap_rule.cmd_str, ROUTE_MAP_SET_NEXTHOP_STR, sizeof(ROUTE_MAP_SET_NEXTHOP_STR)))
                        {
                            if (L_INET_RETURN_SUCCESS == L_INET_StringToInaddr(L_INET_FORMAT_IPV4_ADDR,
                                                                               rmap_rule.rule_str,
                                                                               (L_INET_Addr_T *) &nexthop,
                                                                               sizeof(nexthop)))
                            {
                                option |= NETCFG_OM_PBR_BINDING_OPT_NEXTHOP;
                            }
                        }

                        if ((option & NETCFG_OM_PBR_BINDING_OPT_ALL) == NETCFG_OM_PBR_BINDING_OPT_ALL)
                            break;
                    }

                    memset(&pbr_binding, 0, sizeof(pbr_binding));
                    pbr_binding.vid = vid;
                    strncpy(pbr_binding.rmap_name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
                    pbr_binding.rmap_type = rmap_index.type;
                    pbr_binding.seq_num = rmap_index.pref;
                    strncpy(pbr_binding.acl_name, acl_name, SYS_ADPT_MAX_ACCESS_LIST_NAME_LENGTH+1);
                    pbr_binding.dscp = dscp;
                    pbr_binding.nexthop = nexthop;
                    pbr_binding.option = option;
                    pbr_binding.nexthop_hwinfo = AMTRL3_OM_HW_INFO_INVALID;

                    memset(&old_pbr_binding, 0 , sizeof(old_pbr_binding));
                    old_pbr_binding.vid = vid;
                    strncpy(old_pbr_binding.rmap_name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
                    old_pbr_binding.seq_num = rmap_index.pref;
                    if (NETCFG_OM_PBR_GetBindingEntry(&old_pbr_binding))
                    {
                        pbr_binding.in_amtrl3 = old_pbr_binding.in_amtrl3;
                        pbr_binding.in_chip = old_pbr_binding.in_chip;
                        pbr_binding.nexthop_hwinfo = old_pbr_binding.nexthop_hwinfo;
                        pbr_binding.nexthop_ifindex = old_pbr_binding.nexthop_ifindex;

                        if (old_pbr_binding.in_amtrl3 &&
                            0 != L_INET_CompareInetAddr((L_INET_Addr_T *)&pbr_binding.nexthop, (L_INET_Addr_T *)&old_pbr_binding.nexthop, 0))
                        {
                            NETCFG_MGR_PBR_ProcessDeleteFromAmtrl3(vid, rmap_name, pbr_binding.seq_num);
                            NETCFG_MGR_PBR_ProcessDeleteFromChip(vid, rmap_name, pbr_binding.seq_num);
                            pbr_binding.in_amtrl3 = FALSE;
                            pbr_binding.in_chip = FALSE;
                            pbr_binding.nexthop_hwinfo = AMTRL3_OM_HW_INFO_INVALID;
                            pbr_binding.nexthop_ifindex = 0;
                        }

                        if (old_pbr_binding.in_chip &&
                            (0 != strncmp(pbr_binding.acl_name, old_pbr_binding.acl_name, SYS_ADPT_MAX_ACCESS_LIST_NAME_LENGTH) ||
                             (old_pbr_binding.option & NETCFG_OM_PBR_BINDING_OPT_DSCP) != (pbr_binding.option & NETCFG_OM_PBR_BINDING_OPT_DSCP) ||
                             old_pbr_binding.dscp != pbr_binding.dscp))
                        {
                            NETCFG_MGR_PBR_ProcessDeleteFromChip(vid, rmap_name, old_pbr_binding.seq_num);
                            pbr_binding.in_chip = FALSE;
                        }
                    }

                    if (!NETCFG_OM_PBR_AddBindingEntry(&pbr_binding))
                        return; /*return NETCFG_TYPE_FAIL;*/

                    NETCFG_MGR_PBR_ProcessAddToAmtrl3(vid, rmap_name, pbr_binding.seq_num);
                    NETCFG_MGR_PBR_ProcessAddToChip(vid, rmap_name, pbr_binding.seq_num);
                }
            }
        }
    }

    return; /*return NETCFG_TYPE_OK;*/
}


/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_MGR_PBR_BindRouteMap
 * -------------------------------------------------------------------------
 * PURPOSE:  binding vlan with route-map
 * INPUT:    vid       --  VLAN ID
 *           rmap_name --  route-map name
 * OUTPUT:   none.
 * RETURN:   success - NETCFG_TYPE_OK
 *           fail    - Error code
 * NOTES:
 * -------------------------------------------------------------------------*/
static UI32_T NETCFG_MGR_PBR_BindRouteMap(UI32_T vid, char rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1])
{
    NSM_POLICY_TYPE_RouteMap_T rmap;
    NSM_POLICY_TYPE_RouteMapIndex_T rmap_index;
    NSM_POLICY_TYPE_RouteMapRule_T rmap_rule;
    NETCFG_OM_PBR_BindingEntry_T pbr_binding;
    char old_rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1] = {0};
    char acl_name[NSM_POLICY_TYPE_ROUTE_MAP_ARG_LEN+1];
    L_INET_AddrIp_T nexthop;
    UI8_T  dscp = 0;
    UI8_T  option;
    BOOL_T has_old_rmap = FALSE;

    NETCFG_MGR_PBR_CHECK_OPER_MODE(NETCFG_TYPE_NOT_MASTER_MODE);

    if (NETCFG_OM_PBR_GetBindingRouteMap(vid, old_rmap_name))
    {
        NETCFG_MGR_PBR_ProcessDeleteFromAmtrl3(vid, old_rmap_name, 0);
        NETCFG_MGR_PBR_ProcessDeleteFromChip(vid, old_rmap_name, 0);
        NETCFG_OM_PBR_DeleteEntriesByVid(vid);
    }

    /* Add an entry with seq_num = 0 indicates the vid binds with the rmap_name
     * This entry won't be used to write chip
     */
    memset(&pbr_binding, 0, sizeof(pbr_binding));
    pbr_binding.vid = vid;
    strncpy(pbr_binding.rmap_name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
    pbr_binding.seq_num = 0;
    pbr_binding.nexthop_hwinfo = AMTRL3_OM_HW_INFO_INVALID;

    if (!NETCFG_OM_PBR_AddBindingEntry(&pbr_binding))
        return NETCFG_TYPE_FAIL;

    strncpy(rmap.name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
    if (BGP_PMGR_GetRouteMap(&rmap) == BGP_TYPE_RESULT_OK)
    {
        memset(&rmap_index, 0, sizeof(rmap_index));
        strncpy(rmap_index.map_name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
        while(BGP_PMGR_GetNextRouteMapIndex(&rmap_index) == BGP_TYPE_RESULT_OK)
        {
            memset(&rmap_rule, 0, sizeof(rmap_rule));
            strncpy(rmap_rule.map_name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
            rmap_rule.index_type = rmap_index.type;
            rmap_rule.index_pref = rmap_index.pref;
            memset(acl_name, 0, sizeof(acl_name));
            while(BGP_PMGR_GetNextRouteMapMatch(&rmap_rule) == BGP_TYPE_RESULT_OK)
            {
                if (0 == memcmp(rmap_rule.cmd_str, ROUTE_MAP_MATCH_IP_ADDRESS_STR, sizeof(ROUTE_MAP_MATCH_IP_ADDRESS_STR)))
                {
                    strncpy(acl_name, rmap_rule.rule_str, NSM_POLICY_TYPE_ROUTE_MAP_ARG_LEN+1);
                    break;
                }
            }

            option = 0;
            memset(&rmap_rule, 0, sizeof(rmap_rule));
            strncpy(rmap_rule.map_name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
            rmap_rule.index_type = rmap_index.type;
            rmap_rule.index_pref = rmap_index.pref;
            while(BGP_PMGR_GetNextRouteMapSet(&rmap_rule) == BGP_TYPE_RESULT_OK)
            {
                if (0 == memcmp(rmap_rule.cmd_str, ROUTE_MAP_SET_DSCP_STR, sizeof(ROUTE_MAP_SET_DSCP_STR)))
                {
                    dscp = atoi(rmap_rule.rule_str);
                    option |= NETCFG_OM_PBR_BINDING_OPT_DSCP;
                }

                if (0 == memcmp(rmap_rule.cmd_str, ROUTE_MAP_SET_NEXTHOP_STR, sizeof(ROUTE_MAP_SET_NEXTHOP_STR)))
                {
                    if (L_INET_RETURN_SUCCESS == L_INET_StringToInaddr(L_INET_FORMAT_IPV4_ADDR,
                                                                       rmap_rule.rule_str,
                                                                       (L_INET_Addr_T *) &nexthop,
                                                                       sizeof(nexthop)))
                    {
                        option |= NETCFG_OM_PBR_BINDING_OPT_NEXTHOP;
                    }
                }

                if ((option & NETCFG_OM_PBR_BINDING_OPT_ALL) == NETCFG_OM_PBR_BINDING_OPT_ALL)
                    break;
            }

            memset(&pbr_binding, 0, sizeof(pbr_binding));
            pbr_binding.vid = vid;
            strncpy(pbr_binding.rmap_name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
            pbr_binding.rmap_type = rmap_index.type;
            pbr_binding.seq_num = rmap_index.pref;
            strncpy(pbr_binding.acl_name, acl_name, SYS_ADPT_MAX_ACCESS_LIST_NAME_LENGTH+1);
            pbr_binding.dscp = dscp;
            pbr_binding.nexthop = nexthop;
            pbr_binding.option = option;
            pbr_binding.nexthop_hwinfo = AMTRL3_OM_HW_INFO_INVALID;

            if (!NETCFG_OM_PBR_AddBindingEntry(&pbr_binding))
                return NETCFG_TYPE_FAIL;
        }

        NETCFG_MGR_PBR_ProcessAddToAmtrl3(vid, rmap_name, 0);
        NETCFG_MGR_PBR_ProcessAddToChip(vid, rmap_name, 0);
    }

    return NETCFG_TYPE_OK;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_MGR_PBR_UnbindRouteMap
 * -------------------------------------------------------------------------
 * PURPOSE:  unbinding vlan with route-map
 * INPUT:    vid       --  VLAN ID
 * OUTPUT:   none.
 * RETURN:   success - NETCFG_TYPE_OK
 *           fail    - Error code
 * NOTES:
 * -------------------------------------------------------------------------
 */
static UI32_T NETCFG_MGR_PBR_UnbindRouteMap(UI32_T vid)
{
    char rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1] = {0};

    NETCFG_MGR_PBR_CHECK_OPER_MODE(NETCFG_TYPE_NOT_MASTER_MODE);

    if (!NETCFG_OM_PBR_GetBindingRouteMap(vid, rmap_name))
        return NETCFG_TYPE_OK;

    NETCFG_MGR_PBR_ProcessDeleteFromAmtrl3(vid, rmap_name, 0);
    NETCFG_MGR_PBR_ProcessDeleteFromChip(vid, rmap_name, 0);

    NETCFG_OM_PBR_DeleteEntriesByVid(vid);
    return NETCFG_TYPE_OK;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_MGR_PBR_GetActiveVlanByIp
 * -------------------------------------------------------------------------
 * PURPOSE:  get vlan ifindex of an IP address
 * INPUT:    next_hop_p  -- ip address
 * OUTPUT:   vid_ifindex -- vlan ifindex
 * RETURN:   success - NETCFG_TYPE_OK
 *           fail    - Error code
 * NOTES:
 * -------------------------------------------------------------------------
 */
static UI32_T NETCFG_MGR_PBR_GetActiveVlanByIp(L_INET_AddrIp_T *nexthop_p, UI32_T *vid_ifindex)
{
    NETCFG_TYPE_InetRifConfig_T rif_config;
    NETCFG_TYPE_L3_Interface_T intf;

    rif_config.addr = *nexthop_p;
    if (NETCFG_TYPE_OK == NETCFG_OM_IP_GetRifFromIp(&rif_config))
    {
        intf.ifindex = rif_config.ifindex;
        if (NETCFG_TYPE_OK == NETCFG_OM_IP_GetL3Interface(&intf) &&
            IP_LIB_IsIpInterfaceRunning(intf.u.physical_intf.if_flags))
        {
            *vid_ifindex = intf.ifindex;
            return NETCFG_TYPE_OK;
        }
    }

    return NETCFG_TYPE_FAIL;
}


/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_MGR_PBR_ProcessAddToAmtrl3
 * -------------------------------------------------------------------------
 * PURPOSE:  add PBR host route reference to amtrl3
 * INPUT:    vid       --  VLAN ID
 *           rmap_name --  route-map name
 *           seq_num   --  sequence number
 * OUTPUT:   none.
 * RETURN:   success - NETCFG_TYPE_OK
 *           fail    - Error code
 * NOTES:    1. if rmap_name is blank, means ignore rmap_name
 *           2. if seq_num is 0, means ignore seq_num
 * -------------------------------------------------------------------------
 */
static UI32_T NETCFG_MGR_PBR_ProcessAddToAmtrl3(UI32_T vid, char *rmap_name, UI32_T seq_num)
{
    NETCFG_OM_PBR_BindingEntry_T pbr_binding;
    AMTRL3_TYPE_InetHostRouteEntry_T host_entry;
    UI32_T ifindex = 0;

    memset(&pbr_binding, 0, sizeof(pbr_binding));
    pbr_binding.vid = vid;
    strncpy(pbr_binding.rmap_name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
    if (seq_num)
        pbr_binding.seq_num = seq_num - 1; /* in order to get exact seq_num when getnext */

    while (NETCFG_OM_PBR_GetNextBindingEntry(&pbr_binding))
    {
        if (pbr_binding.seq_num == 0)
            continue;
        if (pbr_binding.vid != vid)
            break;
        if (rmap_name[0] != '\0' && 0 != strncmp(rmap_name, pbr_binding.rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH))
            break;
        if (seq_num != 0 && pbr_binding.seq_num != seq_num)
            break;

        if (pbr_binding.option & NETCFG_OM_PBR_BINDING_OPT_NEXTHOP)
        {
            if (!pbr_binding.in_amtrl3)
            {
                if (NETCFG_TYPE_OK == NETCFG_MGR_PBR_GetActiveVlanByIp(&pbr_binding.nexthop, &ifindex))
                {
                    memset(&host_entry, 0, sizeof(host_entry));
                    host_entry.dst_vid_ifindex = ifindex;
                    host_entry.dst_inet_addr = pbr_binding.nexthop;
                    if (AMTRL3_PMGR_SetHostRouteForPbr(AMTRL3_TYPE_FLAGS_IPV4, SYS_ADPT_DEFAULT_FIB, &host_entry))
                    {
                        pbr_binding.nexthop_ifindex = ifindex;
                        pbr_binding.in_amtrl3 = TRUE;
                        NETCFG_OM_PBR_AddBindingEntry(&pbr_binding);
                    }
                }
            }
        }
    }

    return NETCFG_TYPE_OK;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_MGR_PBR_ProcessAddToChip
 * -------------------------------------------------------------------------
 * PURPOSE:  bind acl to chip
 * INPUT:    vid       --  VLAN ID
 *           rmap_name --  route-map name
 *           seq_num   --  sequence number
 * OUTPUT:   none.
 * RETURN:   success - NETCFG_TYPE_OK
 *           fail    - Error code
 * NOTES:    1. if rmap_name is blank, means ignore rmap_name
 *           2. if seq_num is 0, means ignore seq_num
 * -------------------------------------------------------------------------
 */
static UI32_T NETCFG_MGR_PBR_ProcessAddToChip(UI32_T vid, char *rmap_name, UI32_T seq_num)
{
    NETCFG_OM_PBR_BindingEntry_T pbr_binding;
    UI32_T                       acl_index;
    RULE_TYPE_PBR_ACTION_T       pbr_action;
    AMTRL3_TYPE_InetHostRouteEntry_T host_entry;
    UI32_T ifindex;
    BOOL_T is_modified;

    memset(&pbr_binding, 0, sizeof(pbr_binding));
    pbr_binding.vid = vid;
    strncpy(pbr_binding.rmap_name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
    if (seq_num)
        pbr_binding.seq_num = seq_num - 1; /* in order to get exact seq_num when getnext */
    while (NETCFG_OM_PBR_GetNextBindingEntry(&pbr_binding))
    {
        if (pbr_binding.seq_num == 0)
            continue;
        if (pbr_binding.vid != vid)
            break;
        if (rmap_name[0] != '\0' && 0 != strncmp(rmap_name, pbr_binding.rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH))
            break;
        if (seq_num != 0  && pbr_binding.seq_num != seq_num)
            break;

        if (!pbr_binding.in_chip)
        {
            is_modified = FALSE;
            if (pbr_binding.option & NETCFG_OM_PBR_BINDING_OPT_NEXTHOP)
            {
                if (pbr_binding.nexthop_hwinfo != AMTRL3_OM_HW_INFO_INVALID)
                {
                    pbr_binding.nexthop_hwinfo = AMTRL3_OM_HW_INFO_INVALID;
                    is_modified = TRUE;
                }
                if (pbr_binding.in_amtrl3)
                {
                    host_entry.dst_vid_ifindex = pbr_binding.nexthop_ifindex;
                    host_entry.dst_inet_addr = pbr_binding.nexthop;
                    if (AMTRL3_POM_GetInetHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV4, SYS_ADPT_DEFAULT_FIB, &host_entry))
                    {
                        if (host_entry.hw_info != L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID))
                        {
                            pbr_binding.nexthop_hwinfo = L_CVRT_PTR_TO_UINT(host_entry.hw_info);
                            is_modified = TRUE;
                        }
                    }
                }
            }

            if ((((pbr_binding.option & NETCFG_OM_PBR_BINDING_OPT_DSCP) != 0) ||
                 pbr_binding.nexthop_hwinfo != AMTRL3_OM_HW_INFO_INVALID ||
                 pbr_binding.rmap_type == NSM_POLICY_TYPE_RMAP_DENY) &&
                L4_PMGR_ACL_GetAclIdByName(pbr_binding.acl_name, &acl_index) == RULE_TYPE_OK)
            {
                memset(&pbr_action, 0, sizeof(RULE_TYPE_PBR_ACTION_T));

                if (pbr_binding.rmap_type == NSM_POLICY_TYPE_RMAP_PERMIT)
                    pbr_action.cmd = RULE_TYPE_PBR_PACKET_CMD_PERMIT;
                else if (pbr_binding.rmap_type == NSM_POLICY_TYPE_RMAP_DENY)
                    pbr_action.cmd = RULE_TYPE_PBR_PACKET_CMD_DENY;
                else
                    pbr_action.cmd = RULE_TYPE_PBR_PACKET_CMD_PERMIT; /* for route-map type = any, won't happen */

                if (pbr_binding.option & NETCFG_OM_PBR_BINDING_OPT_DSCP)
                {
                    pbr_action.qos.dscp.is_modified = TRUE;
                    pbr_action.qos.dscp.value = pbr_binding.dscp;
                }

                if ((pbr_binding.option & NETCFG_OM_PBR_BINDING_OPT_NEXTHOP) &&
                    pbr_binding.nexthop_hwinfo != AMTRL3_OM_HW_INFO_INVALID)
                {
                    pbr_action.redirect.action = RULE_TYPE_PBR_REDIRECT_ACTION_ROUTE;
                    pbr_action.redirect.hw_info = pbr_binding.nexthop_hwinfo;
                }

                if (L4_PMGR_PBR_BindAcl(vid, pbr_binding.seq_num, acl_index, &pbr_action) == RULE_TYPE_OK)
                {
                    pbr_binding.in_chip = TRUE;
                    is_modified = TRUE;
                }
            }

            if (is_modified)
                NETCFG_OM_PBR_AddBindingEntry(&pbr_binding);
        }
    }
    return NETCFG_TYPE_OK;
}


/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_MGR_PBR_ProcessDeleteFromAmtrl3
 * -------------------------------------------------------------------------
 * PURPOSE:  delete host route reference in amtrl3
 * INPUT:    vid       --  VLAN ID
 *           rmap_name --  route-map name
 *           seq_num   --  sequence number
 * OUTPUT:   none.
 * RETURN:   success - NETCFG_TYPE_OK
 *           fail    - Error code
 * NOTES:    1. if rmap_name is blank, means ignore rmap_name
 *           2. if seq_num is 0, means ignore seq_num
 * -------------------------------------------------------------------------
 */
static UI32_T NETCFG_MGR_PBR_ProcessDeleteFromAmtrl3(UI32_T vid, char *rmap_name, UI32_T seq_num)
{
    NETCFG_OM_PBR_BindingEntry_T pbr_binding;
    AMTRL3_TYPE_InetHostRouteEntry_T host_entry;

    memset(&pbr_binding, 0, sizeof(pbr_binding));
    pbr_binding.vid = vid;
    strncpy(pbr_binding.rmap_name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
    if (seq_num)
        pbr_binding.seq_num = seq_num - 1; /* in order to get exact seq_num when getnext */

    while (NETCFG_OM_PBR_GetNextBindingEntry(&pbr_binding))
    {
        if (pbr_binding.seq_num == 0)
            continue;
        if (pbr_binding.vid != vid)
            break;
        if (rmap_name[0] != '\0' && 0 != strncmp(rmap_name, pbr_binding.rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH))
            break;
        if (seq_num != 0 && seq_num != pbr_binding.seq_num)
            break;

        if (pbr_binding.in_amtrl3)
        {
            host_entry.dst_vid_ifindex = pbr_binding.nexthop_ifindex;
            host_entry.dst_inet_addr = pbr_binding.nexthop;
            AMTRL3_PMGR_DeleteHostRouteForPbr(AMTRL3_TYPE_FLAGS_IPV4, SYS_ADPT_DEFAULT_FIB, &host_entry);
            pbr_binding.in_amtrl3 = FALSE;
            NETCFG_OM_PBR_AddBindingEntry(&pbr_binding);
        }
    }

    return NETCFG_TYPE_OK;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_MGR_PBR_ProcessDeleteFromChip
 * -------------------------------------------------------------------------
 * PURPOSE:  unbind acl from chip
 * INPUT:    vid       --  VLAN ID
 *           rmap_name --  route-map name
 *           seq_num   --  sequence number
 * OUTPUT:   none.
 * RETURN:   success - NETCFG_TYPE_OK
 *           fail    - Error code
 * NOTES:    1. if rmap_name is blank, means ignore rmap_name
 *           2. if seq_num is zero, means ignore sequence number
 * -------------------------------------------------------------------------
 */
static UI32_T NETCFG_MGR_PBR_ProcessDeleteFromChip(UI32_T vid, char *rmap_name, UI32_T seq_num)
{
    NETCFG_OM_PBR_BindingEntry_T pbr_binding;
    UI32_T                       acl_index;
    AMTRL3_TYPE_InetHostRouteEntry_T host_entry;
    BOOL_T                       is_modified;

    memset(&pbr_binding, 0, sizeof(pbr_binding));
    pbr_binding.vid = vid;
    strncpy(pbr_binding.rmap_name, rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);
    if (seq_num)
        pbr_binding.seq_num = seq_num - 1; /* in order to get exact seq_num when getnext */
    while (NETCFG_OM_PBR_GetNextBindingEntry(&pbr_binding))
    {
        if (pbr_binding.seq_num == 0)
            continue;
        if (pbr_binding.vid != vid)
            break;
        if (rmap_name[0] != '\0' && 0 != strncmp(rmap_name, pbr_binding.rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH))
            break;
        if (seq_num != 0 && pbr_binding.seq_num != seq_num)
            break;

        is_modified = FALSE;
        if (pbr_binding.in_chip)
        {
            if (L4_PMGR_ACL_GetAclIdByName(pbr_binding.acl_name, &acl_index) == RULE_TYPE_OK)
            {
                L4_PMGR_PBR_UnbindAcl(vid, pbr_binding.seq_num, acl_index);
            }

            pbr_binding.nexthop_hwinfo = AMTRL3_OM_HW_INFO_INVALID;
            pbr_binding.in_chip = FALSE;
            is_modified = TRUE;
        }

        if (is_modified)
            NETCFG_OM_PBR_AddBindingEntry(&pbr_binding);
    }
    return NETCFG_TYPE_OK;
}
#endif /* #if (SYS_CPNT_PBR == TRUE) */
