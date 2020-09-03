/* MODULE NAME:  netcfg_pom_route.c
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to 
 *    access NETCFG_OM_ROUTE service.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    02/21/2008 - Vai Wang, Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>

#include "sysfun.h"
#include "sys_bld.h"
#include "netcfg_pom_route.h"
#include "netcfg_mgr_route.h"
#include "netcfg_om_route.h"
#include "netcfg_type.h"
#include "sys_module.h"
#include "l_mm.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define NETCFG_POM_ROUTE_FUNC_BEGIN(req_sz, rep_sz, cmd_id)        \
    const UI32_T        req_size = req_sz;                  \
    const UI32_T        rep_size = rep_sz;                  \
    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG((req_sz>rep_size)?req_sz:rep_size)]; \
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf; \
    NETCFG_OM_ROUTE_IPCMsg_T *msg_p;                       \
                                                            \
    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;                    \
    msgbuf_p->msg_size = req_size;                          \
    msg_p = (NETCFG_OM_ROUTE_IPCMsg_T *)msgbuf_p->msg_buf; \
    msg_p->type.cmd = cmd_id;

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T netcfg_om_route_ipcmsgq_handle;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME : NETCFG_POM_ROUTE_InitiateProcessResource
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
 *    Before other CSC use NETCFG_POM_ROUTE, it should initiate the resource (get the message queue handler internally)
 
 *
 */
BOOL_T NETCFG_POM_ROUTE_InitiateProcessResource(void)
{
    if(SYSFUN_GetMsgQ(SYS_BLD_NETCFG_PROC_OM_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &netcfg_om_route_ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* ROUTINE NAME: NETCFG_POM_ROUTE_GetNextStaticIpCidrRoute
 *
 * FUNCTION:
 *      Get next route using 4 keys.
 *
 * INPUT:
 *      entry_p.
 *
 * OUTPUT:
 *      entry_p.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *
 * NOTES:
 *      key(entry->ip_cidr_route_dest,
 *          entry->ip_cidr_route_mask,
 *          entry->ip_cidr_route_next_hop,
 *          entry->ip_cidr_route_distance)
 */
UI32_T NETCFG_POM_ROUTE_GetNextStaticIpCidrRoute(ROUTE_MGR_IpCidrRouteEntry_T *entry_p)
{
    const UI32_T msg_size = NETCFG_OM_ROUTE_GET_MSG_SIZE(arg_route_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_ROUTE_IPC_GETNEXTSTATICIPCIDRROUTE;
    memcpy(&msg_p->data.arg_route_entry, entry_p, sizeof(ROUTE_MGR_IpCidrRouteEntry_T));
    
    if (SYSFUN_SendRequestMsg(netcfg_om_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }  
    
    memcpy(entry_p, &msg_p->data.arg_route_entry, sizeof(ROUTE_MGR_IpCidrRouteEntry_T));
    
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_POM_ROUTE_GetIpForwardingStatus
 * PURPOSE:
 *      Retrieve IPv4/IPv6 forwarding status
 *
 * INPUT:
 *      ifs->vr_id          -- vrid
 *
 * OUTPUT:
 *      ifs->status_bitmap  -- forwarding status bitmap
 *
 * RETURN:
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_OK
 *
 * NOTES:
 */
UI32_T NETCFG_POM_ROUTE_GetIpForwardingStatus(NETCFG_OM_ROUTE_IpForwardingStatus_T *ifs)
{
    const UI32_T msg_size = NETCFG_OM_ROUTE_GET_MSG_SIZE(arg_ip_forwarding);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_ROUTE_IPC_GETIPFORWARDINGSTATUS;
    memcpy(&msg_p->data.arg_ip_forwarding, ifs, sizeof(NETCFG_OM_ROUTE_IpForwardingStatus_T));
    
    if (SYSFUN_SendRequestMsg(netcfg_om_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }  
    
    memcpy(ifs, &msg_p->data.arg_ip_forwarding, sizeof(NETCFG_OM_ROUTE_IpForwardingStatus_T));
    
    return msg_p->type.result_ui32;
}


/* FUNCTION NAME : NETCFG_POM_ROUTE_GetNextRunningStaticIpCidrRouteEntry
 * PURPOSE:
 *      Get next running static record after the specified key.
 *
 * INPUT:
 *      ip_cidr_route_dest      : IP address of the route entry. (network order)
 *      ip_cidr_route_mask      : IP mask of the route entry. (network order)
 *      ip_cidr_route_distance  : Administrative distance.
 *      ip_cidr_route_next_hop  : IP address of Next-Hop. (network order)
 *
 * OUTPUT:
 *      ip_cidr_route_entry -- record of forwarding table.
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- got static route setting
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL       -- no more interface information.
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *      1. Key is (ipCidrRouteDest, ipCidrRouteMask, ip_cidr_route_distance, ipCidrRouteNextHop)
 *         in NETCFG_MGR_IpCidrRouteEntry_T.
 *      2. if key is (0,0,0,0), means get first one.
 *      3. Related definition, please ref. RFC 2096 (IP CIDR Route Table).
 *      4. Static route entry always set by user.
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_ROUTE_GetNextRunningStaticIpCidrRouteEntry(NETCFG_TYPE_IpCidrRouteEntry_T *ip_cidr_route_entry)
{
    ROUTE_MGR_IpCidrRouteEntry_T entry;

    memset(&entry, 0, sizeof(ROUTE_MGR_IpCidrRouteEntry_T));
    memcpy(&entry.ip_cidr_route_dest,&ip_cidr_route_entry->route_dest, sizeof(entry.ip_cidr_route_dest));
    memcpy(&entry.ip_cidr_route_next_hop, &ip_cidr_route_entry->route_next_hop, sizeof(entry.ip_cidr_route_next_hop));
    entry.ip_cidr_route_distance = ip_cidr_route_entry->ip_cidr_route_distance;

    if (NETCFG_POM_ROUTE_GetNextStaticIpCidrRoute(&entry) != NETCFG_TYPE_OK)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    memcpy(&ip_cidr_route_entry->route_dest, &entry.ip_cidr_route_dest, sizeof(entry.ip_cidr_route_dest));
    memcpy(&ip_cidr_route_entry->route_next_hop, &entry.ip_cidr_route_next_hop, sizeof(entry.ip_cidr_route_next_hop));
    ip_cidr_route_entry->ip_cidr_route_distance = entry.ip_cidr_route_distance;
    ip_cidr_route_entry->ip_cidr_route_status   = entry.ip_cidr_route_status;
    ip_cidr_route_entry->ip_cidr_route_tos      = entry.ip_cidr_route_tos;
    ip_cidr_route_entry->ip_cidr_route_if_index = entry.ip_cidr_route_if_index;
    ip_cidr_route_entry->ip_cidr_route_type     = entry.ip_cidr_route_type;
    ip_cidr_route_entry->ip_cidr_route_proto    = entry.ip_cidr_route_proto;
    ip_cidr_route_entry->ip_cidr_route_age      = entry.ip_cidr_route_age;
    ip_cidr_route_entry->ip_cidr_route_nextHopAS    = entry.ip_cidr_route_next_hop_as;
    ip_cidr_route_entry->ip_cidr_route_metric1      = entry.ip_cidr_route_metric;
    ip_cidr_route_entry->ip_cidr_route_metric2  = entry.ip_cidr_route_metric2;
    ip_cidr_route_entry->ip_cidr_route_metric3  = entry.ip_cidr_route_metric3;
    ip_cidr_route_entry->ip_cidr_route_metric4  = entry.ip_cidr_route_metric4;
    ip_cidr_route_entry->ip_cidr_route_metric5  = entry.ip_cidr_route_metric5;
    ip_cidr_route_entry->ip_cidr_route_status   = entry.ip_cidr_route_status;
    
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* FUNCTION NAME: NETCFG_POM_ROUTE_GetRunningDefaultGateway
 * PURPOSE:
 *      Get running system default gateway.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      default_gateway_ip -- no default value. (network order)
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- got new default gateway IP.
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL       -- no more defautl gateway specified.
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- do not set default gateway.
 *
 * NOTES:
 *      1. Default gateway is set in L2, in L3 is using "ip route" to configure.
 *      2. Support IPv4/IPv6.
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_ROUTE_GetRunningDefaultGateway(L_INET_AddrIp_T *default_gateway_ip)
{
    ROUTE_MGR_IpCidrRouteEntry_T entry;
    UI32_T zero_ip[SYS_ADPT_IPV6_ADDR_LEN] = {0};

    memset(&entry, 0, sizeof(ROUTE_MGR_IpCidrRouteEntry_T));
    entry.ip_cidr_route_dest.type = default_gateway_ip->type;
    entry.ip_cidr_route_next_hop.type = default_gateway_ip->type;
    entry.ip_cidr_route_dest.preflen = 0;
    entry.ip_cidr_route_distance = 0;
    
    if (entry.ip_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV4)
    {
        memset(entry.ip_cidr_route_dest.addr, 0 , SYS_ADPT_IPV4_ADDR_LEN);
        memset(entry.ip_cidr_route_next_hop.addr, 0 , SYS_ADPT_IPV4_ADDR_LEN);
    }
    else if(entry.ip_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV6)
    {
        memset(entry.ip_cidr_route_dest.addr, 0 , SYS_ADPT_IPV6_ADDR_LEN);
        memset(entry.ip_cidr_route_next_hop.addr, 0 , SYS_ADPT_IPV6_ADDR_LEN);
    }    

    if (NETCFG_POM_ROUTE_GetNextStaticIpCidrRoute(&entry) != NETCFG_TYPE_OK)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    if (default_gateway_ip->type == L_INET_ADDR_TYPE_IPV4)
    {
        if ((NETCFG_MGR_ROUTE_IP_IS_ALL_ZEROS(entry.ip_cidr_route_dest.addr)) &&
            (!NETCFG_MGR_ROUTE_IP_IS_ALL_ZEROS(entry.ip_cidr_route_next_hop.addr)))
        {
            memcpy(default_gateway_ip, &entry.ip_cidr_route_next_hop, sizeof(L_INET_AddrIp_T));
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
    }
    else if(default_gateway_ip->type == L_INET_ADDR_TYPE_IPV6)
    {
        if (!(memcmp(entry.ip_cidr_route_dest.addr ,zero_ip, SYS_ADPT_IPV6_ADDR_LEN)) &&
            (memcmp(entry.ip_cidr_route_next_hop.addr, zero_ip, SYS_ADPT_IPV6_ADDR_LEN)))
        {
            memcpy(default_gateway_ip, &entry.ip_cidr_route_next_hop, sizeof(L_INET_AddrIp_T));
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
    }
    
    return SYS_TYPE_GET_RUNNING_CFG_FAIL;
}

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_POM_ROUTE_GetRunningEcmpBalanceMode
 *-----------------------------------------------------------------------------
 * PURPOSE : To get running ecmp load balance mode.
 * INPUT   : None
 * OUTPUT  : mode_p - pointer to output mode
 *           idx_p  - pointer to output hash selection list index
 *                    if *mode_p is NETCFG_TYPE_ECMP_HASH_SELECTION
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES   : None
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_ROUTE_GetRunningEcmpBalanceMode(UI32_T *mode_p, UI32_T *idx_p)
{
    NETCFG_POM_ROUTE_FUNC_BEGIN(
        NETCFG_OM_ROUTE_MSGBUF_TYPE_SIZE,
        NETCFG_OM_ROUTE_GET_MSG_SIZE(u32a1_u32a2),
        NETCFG_OM_ROUTE_IPC_GETRUNNINGECMPBALANCEMODE);

    {
        SYS_TYPE_Get_Running_Cfg_T  ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;

        if (SYSFUN_SendRequestMsg(netcfg_om_route_ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_ui32;
        }

        if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == ret)
        {
            *mode_p = msg_p->data.u32a1_u32a2.u32_a1;
            *idx_p  = msg_p->data.u32a1_u32a2.u32_a2;
        }

        return ret;
    }
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_POM_ROUTE_GetEcmpBalanceMode
 *-----------------------------------------------------------------------------
 * PURPOSE : To get current ecmp load balance mode.
 * INPUT   : None
 * OUTPUT  : mode_p - pointer to output mode
 *           idx_p  - pointer to output hash selection list index
 *                    if *mode_p is NETCFG_TYPE_ECMP_HASH_SELECTION
 * RETURN  : NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
 * NOTES   : None
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_POM_ROUTE_GetEcmpBalanceMode(UI32_T *mode_p, UI32_T *idx_p)
{
    NETCFG_POM_ROUTE_FUNC_BEGIN(
        NETCFG_OM_ROUTE_MSGBUF_TYPE_SIZE,
        NETCFG_OM_ROUTE_GET_MSG_SIZE(u32a1_u32a2),
        NETCFG_OM_ROUTE_IPC_GETECMPBALANCEMODE);

    {
        UI32_T  ret = NETCFG_TYPE_FAIL;

        if (SYSFUN_SendRequestMsg(netcfg_om_route_ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_ui32;
        }

        if (NETCFG_TYPE_OK == ret)
        {
            *mode_p = msg_p->data.u32a1_u32a2.u32_a1;
            *idx_p  = msg_p->data.u32a1_u32a2.u32_a2;
        }

        return ret;
    }
}
#endif /* #if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE) */

