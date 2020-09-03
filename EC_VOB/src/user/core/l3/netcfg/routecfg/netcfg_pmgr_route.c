/* MODULE NAME:  netcfg_pmgr_route.c
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to
 *    access NETCFG_MGR_ROUTE and NETCFG_OM_ROUTE service.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    12/18/2007 - Vai Wang, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>

#include "sys_bld.h"
#include "netcfg_mgr_route.h"
#include "netcfg_pmgr_route.h"
#include "netcfg_type.h"
#include "sys_module.h"
#include "l_mm.h"
#include "sys_dflt.h"
#include "sysfun.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define NETCFG_PMGR_ROUTE_FUNC_BEGIN(req_sz, rep_sz, cmd_id)        \
    const UI32_T        req_size = req_sz;                  \
    const UI32_T        rep_size = rep_sz;                  \
    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG((req_sz>rep_size)?req_sz:rep_size)]; \
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf; \
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;                       \
                                                            \
    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;                    \
    msgbuf_p->msg_size = req_size;                          \
    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T *)msgbuf_p->msg_buf; \
    msg_p->type.cmd = cmd_id;


/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T netcfg_route_ipcmsgq_handle;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME : NETCFG_PMGR_ROUTE_InitiateProcessResource
 * PURPOSE:
 *    Initiate resource used in the calling process.
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
 *    Before other CSC use NETCFG_PMGR_ROUTE, it should initiate
 *    the resource (get the message queue handler internally)
 *
 */
BOOL_T NETCFG_PMGR_ROUTE_InitiateProcessResource(void)
{
    if(SYSFUN_GetMsgQ(SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &netcfg_route_ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}


/* FUNCTION NAME: NETCFG_PMGR_ROUTE_SetDhcpDefaultGateway
 * PURPOSE:
 *      Set system default gateway which claimed from DHCP server.
 * INPUT:
 *      default_gateway -- ip address of the default gateway (in network order)
 * OUTPUT:
 *      none
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL -- failure to set.
 *      NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_ADD
 *      NETCFG_TYPE_TABLE_FULL
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 * NOTES:
 *      1. this default gateway will keep until binding subnet destroyed.
 *      2. DHCP claimed default gateway takes as static configured route,
 *         but do not show in running configuration.
 */
UI32_T NETCFG_PMGR_ROUTE_SetDhcpDefaultGateway(L_INET_AddrIp_T* default_gateway)
{
/* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(addr_ip_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_SETDHCPDEFAULTGATEWAY;
    memcpy(&msg_p->data.addr_ip_v, default_gateway, sizeof(msg_p->data.addr_ip_v));

    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}


/* FUNCTION NAME: NETCFG_PMGR_ROUTE_AddDefaultGateway
 * PURPOSE:
 *      Add system default gateway, for multiple default gateway,
 *  distance determine the order be used.
 *
 * INPUT:
 *      default_gateway -- ip address of the default gateway (network order)
 *      distance        -- smallest value means higher priority be used.
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG          : input parameters are invalid.
 *      NETCFG_TYPE_CAN_NOT_ADD
 *
 * NOTES:
 *
 */
UI32_T NETCFG_PMGR_ROUTE_AddDefaultGateway(L_INET_AddrIp_T *default_gateway, UI32_T distance)
{
/* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_cidr_route_entry_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_ADDDEFAULTGATEWAY;
    memcpy(&msg_p->data.ip_cidr_route_entry_v.route_next_hop, default_gateway, sizeof(msg_p->data.ip_cidr_route_entry_v.route_next_hop));
    msg_p->data.ip_cidr_route_entry_v.ip_cidr_route_distance = distance;

    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_CAN_NOT_ADD;
    }

    return msg_p->type.result_ui32;

}


/* FUNCTION NAME: NETCFG_PMGR_ROUTE_DeleteDefaultGateway
 * PURPOSE:
 *      Delete system default gateway which specified in argument.
 *
 * INPUT:
 *      default_gateway_ip -- to be deleted. (network order)
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 *      NETCFG_TYPE_CAN_NOT_DELETE
 *
 * NOTES:
 *  1. If default_gateway_ip == 0, delete all default gateway.
 *  2. Currently, only one gateway, so base on argument 'default_gateway_ip'
 *     to verify consistent or not.
 */
UI32_T NETCFG_PMGR_ROUTE_DeleteDefaultGateway(L_INET_AddrIp_T *default_gateway_ip)
{
/* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(addr_ip_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_DELETEDEFAULTGATEWAY;
    memcpy(&msg_p->data.addr_ip_v, default_gateway_ip, sizeof(msg_p->data.addr_ip_v));

    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_CAN_NOT_DELETE;
    }

    return msg_p->type.result_ui32;

}


UI32_T NETCFG_PMGR_ROUTE_DeleteDhcpDefaultGateway(L_INET_AddrIp_T *default_gateway_ip)
{
/* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(addr_ip_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_DELETEDHCPDEFAULTGATEWAY;
    memcpy(&msg_p->data.addr_ip_v, default_gateway_ip, sizeof(msg_p->data.addr_ip_v));

    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_CAN_NOT_DELETE;
    }

    return msg_p->type.result_ui32;

}
/* FUNCTION NAME: NETCFG_PMGR_ROUTE_GetDefaultGateway
 * PURPOSE:
 *      Get system default gateway.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      default_gateway_ip. (network order)
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
 *
 */
UI32_T NETCFG_PMGR_ROUTE_GetDefaultGateway(L_INET_AddrIp_T *default_gateway_ip)
{
/* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(addr_ip_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_GETDEFAULTGATEWAY;
    memcpy(&msg_p->data.addr_ip_v, default_gateway_ip, sizeof(msg_p->data.addr_ip_v));

    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_CAN_NOT_GET;
    }
    memcpy(default_gateway_ip, &msg_p->data.addr_ip_v, sizeof(msg_p->data.addr_ip_v));
    return msg_p->type.result_ui32;

}


UI32_T NETCFG_PMGR_ROUTE_GetDhcpDefaultGateway(L_INET_AddrIp_T *default_gateway_ip)
{
/* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(addr_ip_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_GETDEFAULTGATEWAY;
    memcpy(&msg_p->data.addr_ip_v, default_gateway_ip, sizeof(msg_p->data.addr_ip_v));

    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_CAN_NOT_GET;
    }
    memcpy(default_gateway_ip, &msg_p->data.addr_ip_v, sizeof(msg_p->data.addr_ip_v));
    return msg_p->type.result_ui32;

}
/* FUNCTION NAME : NETCFG_PMGR_ROUTE_GetNextRunningStaticIpCidrRouteEntry
 * PURPOSE:
 *      Get next static record after the specified key.
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
SYS_TYPE_Get_Running_Cfg_T NETCFG_PMGR_ROUTE_GetNextRunningStaticIpCidrRouteEntry(NETCFG_TYPE_IpCidrRouteEntry_T *ip_cidr_route_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_cidr_route_entry_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_GETNEXTRUNNINGSTATICIPCIDRROUTEENTRY;
    memcpy(&(msg_p->data.ip_cidr_route_entry_v), ip_cidr_route_entry, sizeof(msg_p->data.ip_cidr_route_entry_v));

    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    memcpy(ip_cidr_route_entry, &(msg_p->data.ip_cidr_route_entry_v), sizeof(msg_p->data.ip_cidr_route_entry_v));
    return msg_p->type.result_running_cfg;

}



/* FUNCTION NAME: NETCFG_PMGR_ROUTE_GetRunningDefaultGateway
 * PURPOSE:
 *      Get system default gateway.
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
SYS_TYPE_Get_Running_Cfg_T NETCFG_PMGR_ROUTE_GetRunningDefaultGateway(L_INET_AddrIp_T *default_gateway_ip)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(addr_ip_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_GETRUNNINGDEFAULTGATEWAY;
    memcpy(&(msg_p->data.addr_ip_v), default_gateway_ip, sizeof(msg_p->data.addr_ip_v));


    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    memcpy(default_gateway_ip, &(msg_p->data.addr_ip_v), sizeof(msg_p->data.addr_ip_v));
    return msg_p->type.result_running_cfg;
}


/* FUNCTION NAME : NETCFG_PMGR_ROUTE_EnableIpForwarding
 * PURPOSE:
 *      Enable IPv4/IPv6 forwarding function.
 *
 * INPUT:
 *      vr_id	    : virtual router id
 *      address_type: IPv4/IPv6 to enable
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK   -- Successfully enable the IPv4/IPv6 forwarding function.
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 *      NETCFG_TYPE_DESTINATION_IS_LOCAL
 *      NETCFG_TYPE_TABLE_FULL
 *      NETCFG_TYPE_CAN_NOT_ADD  -- Unknown condition
 *      NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 */
UI32_T NETCFG_PMGR_ROUTE_EnableIpForwarding(UI32_T vr_id,
                                 UI8_T addr_type)
{
/* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_forwarding_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_ENABLEIPFORWARDING;
    msg_p->data.ip_forwarding_v.vr_id = vr_id;
    msg_p->data.ip_forwarding_v.addr_type = addr_type;


//printf("%s(%d): IP:%d.%d.%d.%d\n", __FUNCTION__, __LINE__, msg_p->data.ip_cidr_route_entry_v.route_dest.addr[0], msg_p->data.ip_cidr_route_entry_v.route_dest.addr[1], msg_p->data.ip_cidr_route_entry_v.route_dest.addr[2], msg_p->data.ip_cidr_route_entry_v.route_dest.addr[3]); fflush(stdout);

//    printf("%s(%d): IP:%d.%d.%d.%d\n", __FUNCTION__, __LINE__, msg_p->data.ip_cidr_route_entry_v.route_next_hop.addr[0], msg_p->data.ip_cidr_route_entry_v.route_next_hop.addr[1], msg_p->data.ip_cidr_route_entry_v.route_next_hop.addr[2], msg_p->data.ip_cidr_route_entry_v.route_next_hop.addr[3]); fflush(stdout);

//



    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_ROUTE_DisableIpForwarding
 * PURPOSE:
 *      Disable IPv4/IPv6 forwarding function.
 *
 * INPUT:
 *      vr_id	    : virtual router id
 *      address_type: IPv4/IPv6 to enable
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK   -- Successfully disable the IPv4/IPv6 forwarding function.
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 *      NETCFG_TYPE_DESTINATION_IS_LOCAL
 *      NETCFG_TYPE_TABLE_FULL
 *      NETCFG_TYPE_CAN_NOT_ADD  -- Unknown condition
 *      NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 */
UI32_T NETCFG_PMGR_ROUTE_DisableIpForwarding(UI32_T vr_id,
                                 UI8_T addr_type)
{
/* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_forwarding_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_DISABLEIPFORWARDING;
    msg_p->data.ip_forwarding_v.vr_id = vr_id;
    msg_p->data.ip_forwarding_v.addr_type = addr_type;

//printf("%s(%d): IP:%d.%d.%d.%d\n", __FUNCTION__, __LINE__, msg_p->data.ip_cidr_route_entry_v.route_dest.addr[0], msg_p->data.ip_cidr_route_entry_v.route_dest.addr[1], msg_p->data.ip_cidr_route_entry_v.route_dest.addr[2], msg_p->data.ip_cidr_route_entry_v.route_dest.addr[3]); fflush(stdout);

//    printf("%s(%d): IP:%d.%d.%d.%d\n", __FUNCTION__, __LINE__, msg_p->data.ip_cidr_route_entry_v.route_next_hop.addr[0], msg_p->data.ip_cidr_route_entry_v.route_next_hop.addr[1], msg_p->data.ip_cidr_route_entry_v.route_next_hop.addr[2], msg_p->data.ip_cidr_route_entry_v.route_next_hop.addr[3]); fflush(stdout);

//



    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_ROUTE_GetIpForwardingStatus
 * PURPOSE:
 *      Retrieve IPv4/IPv6 forwarding function status.
 *
 * INPUT:
 *      vr_id	    : virtual router id
 *      address_type: IPv4/IPv6 to query
 *
 * OUTPUT:
 *      *forward_status_p   -- 1 (IP_FORWARDING_ENABLED)
 *                             0 (IP_FORWARDING_DISABLED)
 *
 * RETURN:
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 *      NETCFG_TYPE_DESTINATION_IS_LOCAL
 *      NETCFG_TYPE_TABLE_FULL
 *      NETCFG_TYPE_CAN_NOT_ADD  -- Unknown condition
 *      NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 */
UI32_T NETCFG_PMGR_ROUTE_GetIpForwardingStatus(UI32_T vr_id,
                                 UI8_T addr_type, UI32_T *forward_status_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_forwarding_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_GETIPFORWARDINGSTATUS;
    msg_p->data.ip_forwarding_v.vr_id = vr_id;
    msg_p->data.ip_forwarding_v.addr_type = addr_type;
    *forward_status_p = 0; /* init to 0 */

//printf("%s(%d): IP:%d.%d.%d.%d\n", __FUNCTION__, __LINE__, msg_p->data.ip_cidr_route_entry_v.route_dest.addr[0], msg_p->data.ip_cidr_route_entry_v.route_dest.addr[1], msg_p->data.ip_cidr_route_entry_v.route_dest.addr[2], msg_p->data.ip_cidr_route_entry_v.route_dest.addr[3]); fflush(stdout);

//    printf("%s(%d): IP:%d.%d.%d.%d\n", __FUNCTION__, __LINE__, msg_p->data.ip_cidr_route_entry_v.route_next_hop.addr[0], msg_p->data.ip_cidr_route_entry_v.route_next_hop.addr[1], msg_p->data.ip_cidr_route_entry_v.route_next_hop.addr[2], msg_p->data.ip_cidr_route_entry_v.route_next_hop.addr[3]); fflush(stdout);

//



    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *forward_status_p = msg_p->data.ip_forwarding_v.forward_status;

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_ROUTE_GetRunningIpForwarding
 * PURPOSE:
 *      Retrieve IPv4/IPv6 forwarding running configuration.
 *
 * INPUT:
 *      vr_id	    : virtual router id
 *      address_type: IPv4/IPv6 to query
 *      status      : status of ip forwarding
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 *      NETCFG_TYPE_DESTINATION_IS_LOCAL
 *      NETCFG_TYPE_TABLE_FULL
 *      NETCFG_TYPE_CAN_NOT_ADD  -- Unknown condition
 *      NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ROUTE_GetRunningIpForwarding(UI32_T vr_id,
                                 UI8_T addr_type,
				 BOOL_T *status)
{

 UI32_T result;
 UI32_T forward_status;


 if (NETCFG_PMGR_ROUTE_GetIpForwardingStatus(vr_id, addr_type, &forward_status) == NETCFG_TYPE_OK)
    result = forward_status;


 if(result == IP_FORWARDING_ENABLED)
  *status = TRUE;
 else if(result == IP_FORWARDING_DISABLED)
  *status = FALSE;
 else
  return SYS_TYPE_GET_RUNNING_CFG_FAIL;

 if(addr_type == L_INET_ADDR_TYPE_IPV4)
 {
	if (SYS_DFLT_IP_FORWARDING != *status)
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

 }
#if (SYS_CPNT_IPV6 == TRUE)
 else if(addr_type == L_INET_ADDR_TYPE_IPV6)
 {
	if (SYS_DFLT_IPV6_FORWARDING != *status)
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

 }
#endif
 else
  return SYS_TYPE_GET_RUNNING_CFG_FAIL;

}

/* FUNCTION NAME : NETCFG_PMGR_ROUTE_AddStaticRoute
 * PURPOSE:
 *      Add a static route entry to routing table.
 *
 * INPUT:
 *      ip          : IP address of the route entry. (network order)
 *      mask        : IP mask of the route entry.(network order)
 *      next_hop    : the ip for this subnet.(network order)
 *      distance    : Administrative distance
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK   -- Successfully add the entry to routing table.
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_INVLAID_NEXT_HOP
 *      NETCFG_TYPE_DESTINATION_IS_LOCAL
 *      NETCFG_TYPE_TABLE_FULL
 *      NETCFG_TYPE_CAN_NOT_ADD  -- Unknown condition
 *      NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *
 * NOTES:
 *      1. Key is (ip, mask)
 *      2. vid_ifIndex need or not ?
 *         if no need vid_ifIndex, need to change CLI.
 *         or define new function to support CLI.
 *      3. So, define new function for Cisco command line.
 */
UI32_T NETCFG_PMGR_ROUTE_AddStaticRoute(L_INET_AddrIp_T *ip,
                                 L_INET_AddrIp_T *next_hop,
                                 UI32_T distance)
{
/* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_cidr_route_entry_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_ADDSTATICROUTE;
    memcpy(&msg_p->data.ip_cidr_route_entry_v.route_dest, ip, sizeof(msg_p->data.ip_cidr_route_entry_v.route_dest));
    memcpy(&msg_p->data.ip_cidr_route_entry_v.route_next_hop, next_hop, sizeof(msg_p->data.ip_cidr_route_entry_v.route_next_hop));
    msg_p->data.ip_cidr_route_entry_v.ip_cidr_route_if_index =0;
    msg_p->data.ip_cidr_route_entry_v.ip_cidr_route_distance = distance;

//printf("%s(%d): IP:%d.%d.%d.%d\n", __FUNCTION__, __LINE__, msg_p->data.ip_cidr_route_entry_v.route_dest.addr[0], msg_p->data.ip_cidr_route_entry_v.route_dest.addr[1], msg_p->data.ip_cidr_route_entry_v.route_dest.addr[2], msg_p->data.ip_cidr_route_entry_v.route_dest.addr[3]); fflush(stdout);

//    printf("%s(%d): IP:%d.%d.%d.%d\n", __FUNCTION__, __LINE__, msg_p->data.ip_cidr_route_entry_v.route_next_hop.addr[0], msg_p->data.ip_cidr_route_entry_v.route_next_hop.addr[1], msg_p->data.ip_cidr_route_entry_v.route_next_hop.addr[2], msg_p->data.ip_cidr_route_entry_v.route_next_hop.addr[3]); fflush(stdout);

//



    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}
#if (SYS_CPNT_IP_TUNNEL == TRUE)
UI32_T NETCFG_PMGR_ROUTE_AddStaticRouteToTunnel(L_INET_AddrIp_T *ip, UI32_T tid_ifindex)
{
/* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_cidr_route_entry_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_ADDSTATICROUTE;
    memcpy(&msg_p->data.ip_cidr_route_entry_v.route_dest, ip, sizeof(msg_p->data.ip_cidr_route_entry_v.route_dest));
    memset(&msg_p->data.ip_cidr_route_entry_v.route_next_hop, 0, sizeof(msg_p->data.ip_cidr_route_entry_v.route_next_hop));
    msg_p->data.ip_cidr_route_entry_v.route_next_hop.type = L_INET_ADDR_TYPE_IPV6;
    msg_p->data.ip_cidr_route_entry_v.ip_cidr_route_if_index = tid_ifindex;
    msg_p->data.ip_cidr_route_entry_v.ip_cidr_route_distance =   SYS_ADPT_MIN_ROUTE_DISTANCE;
    //msg_p->data.ip_cidr_route_entry_v.ip_cidr_route_type = 3/*VAL_ipCidrRouteType_local*/;

    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}
UI32_T NETCFG_PMGR_ROUTE_DeleteStaticRouteToTunnel(L_INET_AddrIp_T *ip, UI32_T tid_ifindex)
{
/* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_cidr_route_entry_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_DELETESTATICROUTE;
    memcpy(&msg_p->data.ip_cidr_route_entry_v.route_dest, ip, sizeof(msg_p->data.ip_cidr_route_entry_v.route_dest));
    memset(&msg_p->data.ip_cidr_route_entry_v.route_next_hop, 0, sizeof(msg_p->data.ip_cidr_route_entry_v.route_next_hop));
    msg_p->data.ip_cidr_route_entry_v.route_next_hop.type = L_INET_ADDR_TYPE_IPV6;
    msg_p->data.ip_cidr_route_entry_v.ip_cidr_route_if_index = tid_ifindex;
    msg_p->data.ip_cidr_route_entry_v.ip_cidr_route_distance =   SYS_ADPT_MIN_ROUTE_DISTANCE;
    //msg_p->data.ip_cidr_route_entry_v.ip_cidr_route_type = 3/*VAL_ipCidrRouteType_local*/;

    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}
#endif /*SYS_CPNT_IP_TUNNEL*/

/* FUNCTION NAME : NETCFG_PMGR_ROUTE_DeleteStaticRoute
 * PURPOSE:
 *      Delete a static route entry from routing table.
 *
 * INPUT:
 *      ip          : IP address of the route entry. (network order)
 *      mask        : IP mask of the route entry.(network order)
 *      next_hop    : the ip for this subnet.(network order)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK           -- Successfully delete the entry from routing table.
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_CAN_NOT_DELETE -- Unknown condition
 *
 * NOTES:
 *      1. Key is (ip, mask, nexthop, distance)
 */
UI32_T NETCFG_PMGR_ROUTE_DeleteStaticRoute(L_INET_AddrIp_T *ip,
                                 L_INET_AddrIp_T *next_hop)
{
/* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_cidr_route_entry_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_DELETESTATICROUTE;
    memcpy(&msg_p->data.ip_cidr_route_entry_v.route_dest, ip, sizeof(msg_p->data.ip_cidr_route_entry_v.route_dest));
    memcpy(&msg_p->data.ip_cidr_route_entry_v.route_next_hop, next_hop, sizeof(msg_p->data.ip_cidr_route_entry_v.route_next_hop));
    msg_p->data.ip_cidr_route_entry_v.ip_cidr_route_if_index =0;

    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}


/* FUNCTION NAME : NETCFG_PMGR_ROUTE_DeleteStaticRouteByNetwork
 * PURPOSE:
 *      Delete a static route entry from routing table.
 *
 * INPUT:
 *      ip   : IP address of the route entry.(network order)
 *      mask : IP mask of the route entry.(network order)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK            -- Successfully add the entry to routing table.
 *      NETCFG_TYPE_ENTRY_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_DELETE -- Unknown condition
 *
 * NOTES:
 *      1. Key is (ip, mask)
 */
UI32_T NETCFG_PMGR_ROUTE_DeleteStaticRouteByNetwork(UI8_T ip[SYS_ADPT_IPV4_ADDR_LEN], UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN])
{
/* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_mask);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_DELETESTATICROUTEBYNETWORK;
    memcpy(msg_p->data.ip_mask.ip, ip, sizeof(msg_p->data.ip_mask.ip));
    memcpy(msg_p->data.ip_mask.mask, mask, sizeof(msg_p->data.ip_mask.mask));

    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_CAN_NOT_DELETE;
    }

    return msg_p->type.result_ui32;
}


/* FUNCTION NAME : NETCFG_PMGR_ROUTE_DeleteAllStaticIpCidrRoutes
 * PURPOSE:
 *      Delete all static routes.
 *
 * INPUT:
 *       None.
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_PMGR_ROUTE_DeleteAllStaticIpCidrRoutes(UI32_T action_flags)
{
/* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_DELETEALLSTATICROUTES;

    memcpy(&msg_p->data.ui32_v, &action_flags, sizeof(msg_p->data.ui32_v));

    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}



/* FUNCTION NAME: NETCFG_PMGR_ROUTE_SetIpCidrRouteRowStatus
 *
 * PURPOSE:
 *      Change the row status field of the route entry
 *
 * INPUT:
 *      ip_addr     : IP address of rif. (network order)
 *      ip_mask     : Mask of rif.  (network order)
 *      tos         : type of service. (must be 0).
 *      next_hop    : NextHop of route entry. (network order)
 *      row_status  :
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_SET
 *
 * NOTES:
 *      1.row_status must be one of the following values:
 *            VAL_ipCidrRouteStatus_active
 *            VAL_ipCidrRouteStatus_notInService
 *            VAL_ipCidrRouteStatus_createAndGo
 *            VAL_ipCidrRouteStatus_createAndWait
 *            VAL_ipCidrRouteStatus_destroy
 *
 *      2.Currently, only createAndGo and destroy are supported.
 */
UI32_T NETCFG_PMGR_ROUTE_SetIpCidrRouteRowStatus(UI8_T ip_addr[SYS_ADPT_IPV4_ADDR_LEN],
                                          UI8_T ip_mask[SYS_ADPT_IPV4_ADDR_LEN],
                                          UI32_T tos,
                                          UI8_T next_hop[SYS_ADPT_IPV4_ADDR_LEN],
                                          UI32_T row_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_mask_tos_next_hop_u32a1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_SETIPCIDRROUTEROWSTATUS;
    memcpy(msg_p->data.ip_mask_tos_next_hop_u32a1.ip, ip_addr, sizeof(msg_p->data.ip_mask_tos_next_hop_u32a1.ip));
    memcpy(msg_p->data.ip_mask_tos_next_hop_u32a1.mask, ip_mask, sizeof(msg_p->data.ip_mask_tos_next_hop_u32a1.mask));
    msg_p->data.ip_mask_tos_next_hop_u32a1.tos = tos;
    memcpy(msg_p->data.ip_mask_tos_next_hop_u32a1.next_hop, next_hop, sizeof(msg_p->data.ip_mask_tos_next_hop_u32a1.next_hop));
    msg_p->data.ip_mask_tos_next_hop_u32a1.u32_a1 = row_status;

    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_CAN_NOT_SET;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_ROUTE_GetReversePathIpMac
 * PURPOSE:
 *      If a user is managing the DUT, we will log the src IP (target IP) and mac-address.
 *      If the target IP is in local subnet, then return target IP and target Mac
 *      If the target IP is not in local subnet, return reverse path nexthop IP and Mac.
 * INPUT:
 *      target_addr_p   -- target address to be checked.
 *
 * OUTPUT:
 *      nexthop_addr_p      -- nexthop address
 *      nexthop_mac_addr_p  -- mac-address of nexthop address
 *
 * RETURN:
 *      NETCFG_TYPE_OK      -- if success
 *      NETCFG_TYPE_FAIL    -- if fail
 *
 * NOTES:
 */
UI32_T NETCFG_PMGR_ROUTE_GetReversePathIpMac(L_INET_AddrIp_T *target_addr_p, L_INET_AddrIp_T *nexthop_addr_p, UI8_T *nexthop_mac_addr_p)
{
     /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(arg_grp_addrx2_mac);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_GETREVERSEPATHIPMAC;
    memcpy(&(msg_p->data.arg_grp_addrx2_mac.addr_1), target_addr_p, sizeof(L_INET_AddrIp_T));

    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    memcpy(nexthop_addr_p, &(msg_p->data.arg_grp_addrx2_mac.addr_2), sizeof(L_INET_AddrIp_T));
    memcpy(nexthop_mac_addr_p, msg_p->data.arg_grp_addrx2_mac.mac,SYS_ADPT_MAC_ADDR_LEN );

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME: NETCFG_PMGR_ROUTE_SetMRouteStatus
 * PURPOSE:
 *      Enable/disable muticast routing.
 *
 * INPUT:
 *      status  -- true: enable; false: disable
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 *
 * NOTES:
 *
 */
UI32_T NETCFG_PMGR_ROUTE_SetMRouteStatus(UI32_T status)
{
/* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_SETMROUTESTATUS;
    msg_p->data.ui32_v = status;

    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_CAN_NOT_DELETE;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME: NETCFG_PMGR_ROUTE_GetRunningMRouteEnableStatus
 * PURPOSE:
 *      Get multicast routing running status
 * INPUT:
 * OUTPUT:
 *      status_p  -- true: enable; false: disable
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 * NOTES:
 */
UI32_T NETCFG_PMGR_ROUTE_GetRunningMRouteEnableStatus(UI32_T *status_p )
{
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_GETRUNNINGMROUTESTATUS;

    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_CAN_NOT_DELETE;
    }

    *status_p = msg_p->data.ui32_v;

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME: NETCFG_PMGR_ROUTE_SetM6RouteStatus
 * PURPOSE:
 *      Enable/disable muticast routing.
 *
 * INPUT:
 *      status  -- true: enable; false: disable
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 *
 * NOTES:
 *
 */
UI32_T NETCFG_PMGR_ROUTE_SetM6RouteStatus(UI32_T status)
{
/* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_SETM6ROUTETATUS;
    msg_p->data.ui32_v = status;

    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_CAN_NOT_DELETE;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME: NETCFG_PMGR_ROUTE_GetM6RouteStatus
 * PURPOSE:
 *      Get multicast routing running status
 * INPUT:
 * OUTPUT:
 *      status_p  -- true: enable; false: disable
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 * NOTES:
 */
UI32_T NETCFG_PMGR_ROUTE_GetM6RouteStatus(UI32_T *status_p )
{
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_GETM6ROUTESTATUS;

    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_CAN_NOT_DELETE;
    }

    *status_p = msg_p->data.ui32_v;

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME: NETCFG_PMGR_ROUTE_GetRunningM6RouteEnableStatus
 * PURPOSE:
 *      Get multicast routing running status
 * INPUT:
 * OUTPUT:
 *      status_p  -- true: enable; false: disable
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 * NOTES:
 */
UI32_T NETCFG_PMGR_ROUTE_GetRunningM6RouteEnableStatus(UI32_T *status_p )
{
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_GETRUNNINGM6ROUTESTATUS;

    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_CAN_NOT_DELETE;
    }

    *status_p = msg_p->data.ui32_v;

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_ROUTE_FindBestRoute
 * PURPOSE:
 *      Find the best route for the specified destionation IP address.
 *
 * INPUT:
 *      dest_ip_p   -- destionation IP address to be checked.
 *
 * OUTPUT:
 *      nexthop_ip_p -- next hop of forwarding.
 *      nexthop_if_p -- the routing interface which next hope belong to.
 *      owner        -- who generates the routing entry. ie. static, RIP or OSPF.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T NETCFG_PMGR_ROUTE_FindBestRoute(
    L_INET_AddrIp_T *dest_ip_p, L_INET_AddrIp_T *nexthop_ip_p, UI32_T *nexthop_if_p, UI32_T *owner)
{
    NETCFG_PMGR_ROUTE_FUNC_BEGIN(
        NETCFG_MGR_ROUTE_GET_MSG_SIZE(ipa1_ipa2_u32a3_u32a4),
        NETCFG_MGR_ROUTE_GET_MSG_SIZE(ipa1_ipa2_u32a3_u32a4),
        NETCFG_MGR_ROUTE_IPCCMD_FINDBESTROUTE);

    {
        UI32_T  ret = NETCFG_TYPE_FAIL;

        msg_p->data.ipa1_ipa2_u32a3_u32a4.ip_a1 = *dest_ip_p;

        if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_ui32;
        }

        if (ret == NETCFG_TYPE_OK)
        {
            *nexthop_ip_p = msg_p->data.ipa1_ipa2_u32a3_u32a4.ip_a2;
            *nexthop_if_p = msg_p->data.ipa1_ipa2_u32a3_u32a4.u32_a3;
            *owner        = msg_p->data.ipa1_ipa2_u32a3_u32a4.u32_a4;
        }

        return ret;
    }
}

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ROUTE_SetEcmpBalanceMode
 *-----------------------------------------------------------------------------
 * PURPOSE : To set ecmp load balance mode.
 * INPUT   : mode - which mode to set
 *           idx  - which idx to set if mode is NETCFG_TYPE_ECMP_HASH_SELECTION
 * OUTPUT  : None
 * RETURN  : NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 * NOTES   : None
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ROUTE_SetEcmpBalanceMode(UI32_T mode, UI32_T idx)
{
    NETCFG_PMGR_ROUTE_FUNC_BEGIN(
        NETCFG_MGR_ROUTE_GET_MSG_SIZE(u32a1_u32a2),
        NETCFG_MGR_ROUTE_MSGBUF_TYPE_SIZE,
        NETCFG_MGR_ROUTE_IPCCMD_SETECMPBALANCEMODE);

    {
        UI32_T  ret = NETCFG_TYPE_FAIL;

        msg_p->data.u32a1_u32a2.u32_a1 = mode;
        msg_p->data.u32a1_u32a2.u32_a2 = idx;

        if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.result_ui32;
        }

        return ret;
    }
}
#endif /* #if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE) */

#if (SYS_CPNT_STATIC_ROUTE_AND_METER_WORKAROUND == TRUE)
/* FUNCTION NAME : NETCFG_PMGR_ROUTE_EnableSWRoute
 * PURPOSE:
 *      Enable: Meter and SW route work. HW route non-work
 *      Disable: HW route work. Meter and SW route non-work
 * INPUT:
 *      status: True - Enable
 *                 False - Disable
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *
 * NOTES:
 */
UI32_T NETCFG_PMGR_ROUTE_EnableSWRoute(UI32_T status)
{
/* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_forwarding_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_ENABLEIPSWROUTE;
    msg_p->data.ui32_v = status;

    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_ROUTE_GetSWRoute
 * PURPOSE:
 *      Enable: Meter and SW route work. HW route non-work
 *      Disable: HW route work. Meter and SW route non-work
 * INPUT:
 *      status: True - Enable
 *                 False - Disable
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *
 * NOTES:
 */
UI32_T NETCFG_PMGR_ROUTE_GetSWRoute(UI32_T *status)
{
/* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_ROUTE_GET_MSG_SIZE(ip_forwarding_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_ROUTE_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ROUTECFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_ROUTE_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_ROUTE_IPCCMD_GETIPSWROUTE;

    if (SYSFUN_SendRequestMsg(netcfg_route_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *status = msg_p->data.ui32_v;

    return msg_p->type.result_ui32;

}
#endif /*#if (SYS_CPNT_STATIC_ROUTE_AND_METER_WORKAROUND == TRUE)*/


