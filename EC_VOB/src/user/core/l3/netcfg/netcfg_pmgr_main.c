/* MODULE NAME:  netcfg_pmgr_main.c
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to
 *    access NETCFG_MGR_MAIN and NETCFG_OM_MAIN service.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    01/29/2008 - Vai Wang, Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "sys_bld.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "netcfg_mgr_main.h"
#include "netcfg_pmgr_route.h"
#include "netcfg_pmgr_ip.h"
#include "netcfg_pmgr_nd.h"
#if (SYS_CPNT_RIP == TRUE)
#include "netcfg_pmgr_rip.h"/*Lin.Li, for RIP porting*/
#endif
#include "netcfg_type.h"
#include "sys_module.h"
#include "l_mm.h"
#if (SYS_CPNT_OSPF == TRUE)
#include "netcfg_mgr_ospf.h"
#include "netcfg_pmgr_ospf.h"/*Lin.Li, for OSPF porting*/
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T netcfg_main_ipcmsgq_handle;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME : NETCFG_PMGR_MAIN_InitiateProcessResource
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
 *    Before other CSC use NETCFG_PMGR_XXX, it should initiate
 *    the resource (get the message queue handler internally)
 *
 */
BOOL_T NETCFG_PMGR_MAIN_InitiateProcessResource(void)
{
    if(SYSFUN_GetMsgQ(SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &netcfg_main_ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    NETCFG_PMGR_IP_InitiateProcessResource();
    NETCFG_PMGR_ROUTE_InitiateProcessResource();
    NETCFG_PMGR_ND_InitiateProcessResource();
#if (SYS_CPNT_RIP == TRUE)
    NETCFG_PMGR_RIP_InitiateProcessResource();/*Lin.Li, for RIP porting*/
#endif

#if (SYS_CPNT_OSPF == TRUE)
    NETCFG_PMGR_OSPF_InitiateProcessResource();/*Lin.Li, for OSPF porting*/
#endif

    return TRUE;
}


/* FUNCTION NAME - NETCFG_PMGR_MAIN_HandleHotInsertion
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 *
 */
void NETCFG_PMGR_MAIN_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_MAIN_GET_MSG_SIZE(hot_inerstion_handle);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_MAIN_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_NETCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_MAIN_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_MAIN_IPCCMD_HANDLEHOTINSERTION;
    msg_p->data.hot_inerstion_handle.starting_port_ifindex = starting_port_ifindex;
    msg_p->data.hot_inerstion_handle.number_of_port = number_of_port;
    msg_p->data.hot_inerstion_handle.use_default = use_default;

    if (SYSFUN_SendRequestMsg(netcfg_main_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            0, NULL) != SYSFUN_OK)
    {
        return;
    }

    return;
}

/* FUNCTION NAME - NETCFG_PMGR_MAIN_HandleHotRemoval
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 *
 */
void NETCFG_PMGR_MAIN_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
/* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_MAIN_GET_MSG_SIZE(u32a1_u32a2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_MAIN_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_NETCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_MAIN_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_MAIN_IPCCMD_HANDLEHOTREMOVAL;
    msg_p->data.u32a1_u32a2.u32_a1 = starting_port_ifindex;
    msg_p->data.u32a1_u32a2.u32_a2 = number_of_port;


    if (SYSFUN_SendRequestMsg(netcfg_main_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            0, NULL) != SYSFUN_OK)
    {
        return;
    }

    return;
}


/* FUNCTION NAME : NETCFG_PMGR_MAIN_IsEmbeddedUdpPort
 * PURPOSE:
 *      Check the udp-port is used in protocol engine or not.
 *
 * INPUT:
 *      udp_port -- the udp-port to be checked.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- this port is used in protocl engine, eg. 520.
 *      FALSE   -- this port is available for sokcet used,
 *                 but possibly already used in socket engine
 *
 * NOTES:
 *      None.
 */
BOOL_T NETCFG_PMGR_MAIN_IsEmbeddedUdpPort(UI32_T udp_port)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_MAIN_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_MAIN_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_NETCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_MAIN_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_MAIN_IPCCMD_ISEMBEDDEDUDPPORT;
    msg_p->data.ui32_v = udp_port;
    if (SYSFUN_SendRequestMsg(netcfg_main_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;
}


/* FUNCTION NAME : NETCFG_PMGR_MAIN_IsEmbeddedTcpPort
 * PURPOSE:
 *      Check the tcp-port is used in protocol engine or not.
 *
 * INPUT:
 *      tcp_port -- the tcp-port to be checked.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- this port is used in protocl engine, eg. 520.
 *      FALSE   -- this port is available for sokcet used,
 *                 but possibly already used in socket engine
 *
 * NOTES:
 *      None.
 */
BOOL_T NETCFG_PMGR_MAIN_IsEmbeddedTcpPort(UI32_T tcp_port)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = NETCFG_MGR_MAIN_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_MAIN_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_NETCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_MAIN_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_MAIN_IPCCMD_ISEMBEDDEDTCPPORT;
    msg_p->data.ui32_v = tcp_port;
    if (SYSFUN_SendRequestMsg(netcfg_main_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;

}

//TODO_K4
#include "rip_pmgr.h"
UI32_T RIP_PMGR_RifUp(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_InterfaceDown(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_InterfaceUp(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_InterfaceDelete(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_LoopbackInterfaceAdd(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, UI16_T if_flags)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_InterfaceAdd(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_RedistributeUnset(UI32_T vr_id, UI32_T instance,char *protocol)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_PassiveIfAdd(UI32_T vr_id, UI32_T instance,UI32_T vid)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_DefaultAdd(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_NeighborSet(UI32_T vr_id, UI32_T instance,struct pal_in4_addr *addr)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_NeighborUnset(UI32_T vr_id, UI32_T instance,struct pal_in4_addr *addr)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_DefaultDelete(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_RecvBuffSizeSet(UI32_T vr_id, UI32_T instance,UI32_T buff_size)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_RecvBuffSizeUnset(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_MaxPrefixSet(UI32_T vr_id, UI32_T instance,UI32_T pmax)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_MaxPrefixUnset(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_DistanceDefaultSet(UI32_T vr_id, UI32_T instance,UI32_T distance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_DistanceDefaultUnset(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_TimerSet(UI32_T vr_id, UI32_T instance, RIP_TYPE_Timer_T *timer)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_TimerUnset(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_NetworkSetByVid(UI32_T vr_id, UI32_T instance,UI32_T vid)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_NetworkUnsetByVid(UI32_T vr_id, UI32_T instance,UI32_T vid)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_AuthKeyChainSet(UI32_T vr_id, UI32_T instance,UI32_T ifindex, char *str)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_AuthKeyChainUnset(UI32_T vr_id, UI32_T instance,UI32_T ifindex)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_RouterRipSet(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_RouterRipUnset(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_PassiveIfDelete(UI32_T vr_id, UI32_T instance,UI32_T vid)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_AuthStringSet(UI32_T vr_id, UI32_T instance,UI32_T ifindex, char *str)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_AuthStringUnset(UI32_T vr_id, UI32_T instance,UI32_T ifindex)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_SendPacketSet(UI32_T vr_id, UI32_T instance,UI32_T ifindex)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_SendPacketUnset(UI32_T vr_id, UI32_T instance,UI32_T ifindex)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_VersionSet(UI32_T vr_id, UI32_T instance,enum RIP_TYPE_Global_Version_E version)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_VersionUnset(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_NetworkSetByAddress(UI32_T vr_id, UI32_T instance,L_PREFIX_T network_address)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_NetworkUnsetByAddress(UI32_T vr_id, UI32_T instance,L_PREFIX_T network_address)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_RecvPacketSet(UI32_T vr_id, UI32_T instance,UI32_T ifindex)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_RecvPacketUnset(UI32_T vr_id, UI32_T instance,UI32_T ifindex)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_DefaultMetricSet(UI32_T vr_id, UI32_T instance,UI32_T metric)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_DefaultMetricUnset(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_SplitHorizonSet(UI32_T vr_id, UI32_T instance,UI32_T ifindex, enum RIP_TYPE_Split_Horizon_E type)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_SplitHorizonUnset(UI32_T vr_id, UI32_T instance,UI32_T ifindex)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_SendVersionTypeSet(UI32_T vr_id, UI32_T instance,
                                              UI32_T ifindex, enum RIP_TYPE_Version_E type)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_AuthModeUnset(UI32_T vr_id, UI32_T instance,UI32_T ifindex)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_SendVersionUnset(UI32_T vr_id, UI32_T instance,UI32_T ifindex)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_RecvVersionTypeSet(UI32_T vr_id, UI32_T instance,
                                              UI32_T ifindex, enum RIP_TYPE_Version_E type)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_RecvVersionUnset(UI32_T vr_id, UI32_T instance,UI32_T ifindex)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_ConfigUnDebugNsm(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_ConfigDebugNsm(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_ConfigUnDebugPacket(UI32_T vr_id, UI32_T instance,RIP_TYPE_Packet_Debug_Type_T type)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_ConfigDebugPacket(UI32_T vr_id, UI32_T instance,RIP_TYPE_Packet_Debug_Type_T type)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_ConfigUnDebugEvent(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_ConfigDebugEvent(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_ConfigUnDebug(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_ConfigDebug(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_GetNextRouteEntry(UI32_T vr_id, UI32_T vrf_id, RIP_TYPE_Route_Entry_T *entry)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_RedistributeSet(UI32_T vr_id, UI32_T instance,char *protocol)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_DistanceSet(UI32_T vr_id, UI32_T instance,UI32_T distance,
                                    struct pal_in4_addr *addr, UI32_T plen, char *alist)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_DistanceUnset(UI32_T vr_id, UI32_T instance,
                                       struct pal_in4_addr *addr, UI32_T plen)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_DistributeListAdd(UI32_T vr_id, UI32_T instance,char *ifname, char *list_name,
                                          enum RIP_TYPE_Distribute_Type_E type,
                                          enum RIP_TYPE_Distribute_List_Type_E list_type)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_DistributeListDelete(UI32_T vr_id, UI32_T instance,char *ifname, char *list_name,
                                             enum RIP_TYPE_Distribute_Type_E type,
                                             enum RIP_TYPE_Distribute_List_Type_E list_type)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_RedistributeRmapSet(UI32_T vr_id, UI32_T instance,char *protocol, char *rmap)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_RedistributeMetricSet(UI32_T vr_id, UI32_T instance,char *protocol, UI32_T metric)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_RedistributeAllSet(UI32_T vr_id, UI32_T instance,char *protocol, UI32_T metric, char *rmap)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_AuthModeSet(UI32_T vr_id, UI32_T instance,UI32_T ifindex, enum RIP_TYPE_Auth_Mode_E mode)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_Debug(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_UnDebug(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_DebugEvent(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_UnDebugEvent(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_DebugPacket(UI32_T vr_id, UI32_T instance,RIP_TYPE_Packet_Debug_Type_T type)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_UnDebugPacket(UI32_T vr_id, UI32_T instance,RIP_TYPE_Packet_Debug_Type_T type)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_DebugNsm(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_UnDebugNsm(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_ClearRoute(UI32_T vr_id, UI32_T instance,char *arg)
{
    return RIP_TYPE_RESULT_FAIL;
}

UI32_T RIP_PMGR_ClearStatistics(UI32_T vr_id, UI32_T instance)
{
    return RIP_TYPE_RESULT_FAIL;
}

BOOL_T RIP_PMGR_GetNextThreadTimer(UI32_T vr_id, UI32_T vrf_id, UI32_T *nexttime)
{
    return FALSE;
}

BOOL_T RIP_PMGR_GetIfAddress(UI32_T vr_id, UI32_T vrf_id, UI32_T exact, UI32_T in_addr, UI32_T *out_addr)
{
    return FALSE;
}

BOOL_T RIP_PMGR_GetIfRecvBadPacket(UI32_T vr_id, UI32_T vrf_id, UI32_T exact, UI32_T in_addr, UI32_T *value)
{
    return FALSE;
}

BOOL_T RIP_PMGR_GetIfRecvBadRoute(UI32_T vr_id, UI32_T vrf_id, UI32_T exact, UI32_T in_addr, UI32_T *value)
{
    return FALSE;
}

BOOL_T RIP_PMGR_GetIfSendUpdate(UI32_T vr_id, UI32_T vrf_id,UI32_T exact, UI32_T in_addr, UI32_T *value)
{
    return FALSE;
}

BOOL_T RIP_PMGR_GetGlobalStatistics(UI32_T vr_id, UI32_T vrf_id, RIP_TYPE_Global_Statistics_T *stat)
{
    return FALSE;
}

BOOL_T RIP_PMGR_GetPeerEntry(UI32_T vr_id, UI32_T vrf_id, RIP_TYPE_Peer_Entry_T *entry)
{
    return FALSE;
}

BOOL_T RIP_PMGR_GetNextPeerEntry(UI32_T vr_id, UI32_T vrf_id, RIP_TYPE_Peer_Entry_T *entry)
{
    return FALSE;
}

UI32_T RIP_PMGR_RifDown(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask)
{
    return RIP_TYPE_RESULT_FAIL;
}

BOOL_T RIP_PMGR_InitiateProcessResource(void)
{
    return TRUE;
}

BOOL_T RIP_PMGR_GetRip2IfConfTable(UI32_T vr_id, UI32_T vrf_id, Rip2IfConfEntry_T *data)
{
    return FALSE;
}

BOOL_T RIP_PMGR_GetNextRip2IfConfTable(UI32_T vr_id, UI32_T vrf_id, Rip2IfConfEntry_T *data)
{
    return FALSE;
}

BOOL_T RIP_PMGR_GetRip2IfStatTable(UI32_T vr_id, UI32_T vrf_id, Rip2IfStatEntry_T *data)
{
    return FALSE;
}

BOOL_T RIP_PMGR_GetNextRip2IfStatTable(UI32_T vr_id, UI32_T vrf_id, Rip2IfStatEntry_T *data)
{
    return FALSE;
}

//=======================================================
BOOL_T BGP_POM_InitiateProcessResource(void)
{
    return TRUE;
}

#include "bgp_pmgr.h"
UI32_T BGP_PMGR_SignalL3IfCreate(UI32_T ifindex)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_SignalL3IfRifDown(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_SignalL3IfRifDestroy(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_SignalL3IfDestroy(UI32_T ifindex)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_SignalL3IfRifUp(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_SignalL3IfRifCreate(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_SignalL3IfDown(UI32_T ifindex)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_SignalL3IfUp(UI32_T ifindex)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

BOOL_T BGP_PMGR_InitiateProcessResource(void)
{
    return TRUE;
}

UI32_T BGP_PMGR_MIB_GetBgp4PathAttrEntry(BGP_TYPE_MIB_Bgp4PathAttrEntry_T *path_attr_p)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_MIB_GetNextBgp4PathAttrEntry(BGP_TYPE_MIB_Bgp4PathAttrEntry_T *path_attr_p)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_MIB_GetBgpVersion(char version_str[MAXSIZE_bgpVersion+1])
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_MIB_GetBgpLocalAs(UI32_T *local_as_p)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_MIB_GetBgpIdentifier(UI32_T *bgp_identifier_p)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_MIB_GetBgpPeerEntry(BGP_TYPE_MIB_BgpPeerEntry_T *peer_entry_p)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_MIB_GetNextBgpPeerEntry(BGP_TYPE_MIB_BgpPeerEntry_T *peer_entry_p)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_MIB_SetBgpPeerAdminStatus(L_INET_AddrIp_T *neighbor_p, I32_T status)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_MIB_SetBgpPeerConnectRetryInterval(L_INET_AddrIp_T *neighbor_p, I32_T interval)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_MIB_SetBgpPeerHoldTimeConfigured(L_INET_AddrIp_T *neighbor_p, I32_T hold_time)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_MIB_SetBgpPeerKeepAliveConfigured(L_INET_AddrIp_T *neighbor_p, I32_T keep_alive)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_MIB_SetBgpPeerMinASOriginationInterval(L_INET_AddrIp_T *neighbor_p, I32_T as_orig)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_MIB_SetBgpPeerMinRouteAdvertisementInterval(L_INET_AddrIp_T *neighbor_p, I32_T route_adv)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_GetNextRouteMapSet(NSM_POLICY_TYPE_RouteMapRule_T *rmap_rule_p)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_GetNextRouteMapMatch(NSM_POLICY_TYPE_RouteMapRule_T *rmap_rule_p)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_GetRouteMapIndex(NSM_POLICY_TYPE_RouteMapIndex_T *rmap_index_p)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_GetRouteMap(NSM_POLICY_TYPE_RouteMap_T *rmap_info_p)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_GetNextRouteMapIndex(NSM_POLICY_TYPE_RouteMapIndex_T *rmap_index_p)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}
//=======================================================
#include "ospf6_pmgr.h"
UI32_T OSPF6_PMGR_SignalL3IfUp(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    return OSPF6_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T OSPF6_PMGR_SignalL3IfDown(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    return OSPF6_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T OSPF6_PMGR_SignalL3IfCreate(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, UI32_T mtu, UI32_T bandwidth, UI32_T if_flags)
{
    return OSPF6_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T OSPF6_PMGR_SignalL3IfDestroy(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    return OSPF6_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T OSPF6_PMGR_SignalL3IfRifCreate(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, L_INET_AddrIp_T *addr, UI32_T primary)
{
    return OSPF6_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T OSPF6_PMGR_SignalL3IfRifDestroy(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, L_INET_AddrIp_T *addr)
{
    return OSPF6_TYPE_RESULT_SEND_MSG_FAIL;
}

BOOL_T OSPF6_PMGR_InitiateProcessResource(void)
{
    return TRUE;
}

//==n====================================================
#include "ospf_pmgr.h"
UI32_T OSPF_PMGR_GetOspfInterfaceEntry(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

void OSPF_PMGR_InterfaceDown(UI32_T vr_id, UI32_T ifindex)
{
}

void OSPF_PMGR_InterfaceUp(UI32_T vr_id, UI32_T ifindex)
{
}

UI32_T OSPF_PMGR_IpAddressAdd(UI32_T vr_id, UI32_T ifindex,UI32_T ip_addr, UI32_T ip_mask, UI32_T primary)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_IpAddressDelete(UI32_T vr_id, UI32_T ifindex,UI32_T ip_addr, UI32_T ip_mask)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_InterfaceAdd(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, UI32_T mtu, UI32_T bandwidth, UI32_T if_flags)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_InterfaceDelete(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_DefaultInfoRoutemapUnset(UI32_T vr_id, UI32_T proc_id)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_DefaultInfoRoutemapSet(UI32_T vr_id, UI32_T proc_id, char *route_map)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_RedistributeRoutemapUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_RedistributeRoutemapSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, char *route_map)
{
    return OSPF_TYPE_RESULT_FAIL;
}


BOOL_T OSPF_PMGR_InitiateProcessResource(void)
{
    return TRUE;
}

UI32_T OSPF_PMGR_SetIfMetricEntry(OSPF_TYPE_IfMetricEntry_T *entry_p)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_SetIfAuthKey(UI32_T addr, char *auth_key)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_SetIfPollInterval(UI32_T addr, UI32_T value)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_SetIfAdminStat(UI32_T addr, UI32_T value)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_SetIfType(UI32_T addr, UI32_T value)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_SetHostStatus(UI32_T vr_id, UI32_T proc_id,
                      UI32_T ip_addr, UI32_T tos, UI32_T status)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_SetHostMetric(UI32_T vr_id, UI32_T proc_id,
                      UI32_T ip_addr, UI32_T tos, UI32_T metric)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_StubAreaMetricTypeSet(UI32_T vr_id,UI32_T proc_id,UI32_T area_id, UI32_T tos, UI32_T metric_type)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_SetAreaStatus(OSPF_TYPE_Area_T *area)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetExternLsaCksumSum(UI32_T vr_id, UI32_T proc_id, UI32_T *ret)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetOriginateNewLsas(UI32_T vr_id, UI32_T proc_id, UI32_T *ret)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetRxNewLsas(UI32_T vr_id, UI32_T proc_id, UI32_T *ret)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetAreaBdrRtrStatus(UI32_T vr_id, UI32_T proc_id, UI32_T *ret)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetVersionNumber(UI32_T vr_id, UI32_T proc_id, UI32_T *ret)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetExternLsaCount(UI32_T vr_id, UI32_T proc_id, UI32_T *ret)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextIfMetricEntry(OSPF_TYPE_IfMetricEntry_T *entry_p)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetIfMetricEntry(OSPF_TYPE_IfMetricEntry_T *entry_p)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextHostEntry(OSPF_TYPE_HostEntry_T *host_entry_p)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetHostEntry(OSPF_TYPE_HostEntry_T *host_entry_p)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_SetDemandExtensions(UI32_T vr_id, UI32_T proc_id, UI32_T status)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetDemandExtensions(UI32_T vr_id, UI32_T proc_id, UI32_T *ret)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_SetMulticastExtensions(UI32_T vr_id, UI32_T proc_id, UI32_T status)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetMulticastExtensions(UI32_T vr_id, UI32_T proc_id, UI32_T *ret)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_SetExitOverflowInterval(UI32_T vr_id, UI32_T proc_id, UI32_T interval)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_SetAdminStat(UI32_T vr_id, UI32_T proc_id, UI32_T status)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetAdminStat(UI32_T vr_id, UI32_T proc_id, UI32_T *ret)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetExitOverflowInterval(UI32_T vr_id, UI32_T proc_id, UI32_T *ret)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_SetTOSSupport(UI32_T vr_id, UI32_T proc_id, UI32_T status)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetTOSSupport(UI32_T vr_id, UI32_T proc_id, UI32_T *ret)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_SetASBdrRtrStatus(UI32_T vr_id, UI32_T proc_id, UI32_T status)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetASBdrRtrStatus (UI32_T vr_id, UI32_T proc_id, UI32_T *ret)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextOspfEntry(OSPF_MGR_OSPF_ENTRY_T *ospf_entry_p)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextOspfLsaEntry(OSPF_MGR_LSA_ENTRY_T *lsa_entry_p)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextOspfNeighborEntry(OSPF_MGR_NBR_ENTRY_T *nbr_entry_p)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextProcessStatus(UI32_T vr_id, UI32_T *proc_id)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextOspfIfEntryByIfindex(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextVirtualLinkEntry(OSPF_TYPE_Vlink_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextRoute(OSPF_TYPE_Route_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetAbrBorderRouter(UI32_T vr_id, UI32_T proc_id)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetAsbrBorderRouter(UI32_T vr_id, UI32_T proc_id)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_ClearOspfProcessAll(UI32_T vr_id)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_ClearOspfProcess(UI32_T vr_id, UI32_T proc_id)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_DefaultInfoMetricTypeUnset( UI32_T vr_id, UI32_T proc_id )
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_DefaultInfoMetricUnset(UI32_T vr_id, UI32_T proc_id )
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_DefaultInfoSet(UI32_T vr_id, UI32_T proc_id)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_DefaultInfoUnset(UI32_T vr_id, UI32_T proc_id)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextMultiProcSummaryAddrEntry(UI32_T vr_id,UI32_T vrf_id, OSPF_TYPE_Multi_Proc_Summary_Addr_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextNetworkAreaTable(OSPF_TYPE_Network_Area_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextAreaRangeTable(OSPF_TYPE_Area_Range_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextAreaPara(OSPF_TYPE_Area_Para_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextPassIfTable(OSPF_TYPE_Passive_If_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetMultiProcessSystemEntry(UI32_T vr_id, UI32_T vrf_id, OSPF_TYPE_MultiProcessSystem_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AreaStubNoSummarySet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AreaRangeNoAdvertiseSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T range_addr, UI32_T range_masklen)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_DefaultInfoMetricTypeSet(UI32_T vr_id, UI32_T proc_id, UI32_T metric_type)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_DefaultInfoMetricSet(UI32_T vr_id, UI32_T proc_id,UI32_T metric)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetMultiProcRedistEntry(UI32_T vr_id,UI32_T vrf_id, OSPF_TYPE_Multi_Proc_Redist_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AreaStubSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AreaStubUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetAutoCost(UI32_T vr_id, UI32_T proc_id, UI32_T *ref_bandwidth)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_PassiveIfSet(OSPF_TYPE_Passive_If_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_PassiveIfUnset(OSPF_TYPE_Passive_If_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AreaNssaSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, OSPF_TYPE_Area_Nssa_Para_T *nssa_para)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AreaNssaUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T flag)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AreaRangeSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T range_addr, UI32_T range_masklen)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AreaRangeUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T range_addr, UI32_T range_masklen)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AreaVirtualLinkSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, OSPF_TYPE_Area_Virtual_Link_Para_T *vlink_para)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AreaVirtualLinkUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, OSPF_TYPE_Area_Virtual_Link_Para_T *vlink_para)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AreaSnubNoSummarySet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AreaStubNoSummaryUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AreaDefaultCostSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T cost)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AreaDefaultCostUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_CompatibleRfc1853Set(UI32_T vr_id, UI32_T proc_id)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_CompatibleRfc1853Unset(UI32_T vr_id, UI32_T proc_id)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_DefaultMetricSet(UI32_T vr_id, UI32_T proc_id, UI32_T metric)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_DefaultMetricUnset(UI32_T vr_id, UI32_T proc_id)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AreaAuthenticationTypeSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T type)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AreaAuthenticationTypeUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_TimerSet(UI32_T vr_id, UI32_T proc_id, UI32_T delay, UI32_T hold)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_TimerUnset(UI32_T vr_id, UI32_T proc_id)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_RouterIdSet(UI32_T vr_id, UI32_T proc_id, UI32_T router_id)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_RouterIdUnset(UI32_T vr_id, UI32_T proc_id)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_NetworkSet(UI32_T vr_id, UI32_T proc_id, UI32_T network_addr, UI32_T masklen, UI32_T area_id, UI32_T format)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_NetworkUnset(UI32_T vr_id, UI32_T proc_id, UI32_T network_addr, UI32_T masklen, UI32_T area_id, UI32_T format)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_IfAuthenticationTypeSet(UI32_T vr_id, UI32_T ifindex, UI8_T type, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_IfAuthenticationTypeUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_IfTransmitDelaySet(UI32_T vr_id, UI32_T ifindex, UI32_T delay, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_IfTransmitDelayUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextIfParamEntry(UI32_T vr_id, UI32_T vrf_id, int indexlen, OSPF_TYPE_IfParam_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_IfCostSet(UI32_T vr_id, UI32_T ifindex, UI32_T cost, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_IfCostUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AutoCostSet(UI32_T vr_id, UI32_T proc_id, UI32_T ref_bandwidth)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AutoCostUnset(UI32_T vr_id, UI32_T proc_id)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_SummaryAddressSet(UI32_T vr_id, UI32_T proc_id, UI32_T address, UI32_T mask)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_SummaryAddressUnset(UI32_T vr_id, UI32_T proc_id, UI32_T address, UI32_T mask)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_IfAuthenticationKeySet(UI32_T vr_id, UI32_T ifindex, char *auth_key, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_IfAuthenticationKeyUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_DefaultInfoAlwaysSet(UI32_T vr_id, UI32_T proc_id)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_DefaultInfoAlwaysUnset(UI32_T vr_id, UI32_T proc_id)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_IfPrioritySet(UI32_T vr_id, UI32_T ifindex, UI8_T priority, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_IfPriorityUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_RedistributeProtoSet(UI32_T vr_id, UI32_T proc_id, char *type)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_RedistributeProtoUnset(UI32_T vr_id, UI32_T proc_id, char *type)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_IfDeadIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_IfDeadIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_RedistributeTagSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, UI32_T tag)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_RedistributeTagUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_RedistributeMetricSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, UI32_T metric)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_RedistributeMetricUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_IfMessageDigestKeySet(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, char *auth_key, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_IfMessageDigestKeyUnset(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_IfHelloIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_IfHelloIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_IfRetransmitIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_IfRetransmitIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_RedistributeMetricTypeSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, UI32_T metric_type)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_RedistributeMetricTypeUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetExtLsdbLimit(UI32_T vr_id, UI32_T proc_id, UI32_T *ret)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetOspfMultiProcessRouteNexthopEntry(OspfMultiProcessRouteNexthopEntry_T *entry_p)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextOspfMultiProcessRouteNexthopEntry(OspfMultiProcessRouteNexthopEntry_T *entry_p)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetIfAuthMd5Key(UI32_T vr_id, UI32_T vrf_id, OSPF_TYPE_MultiProcessIfAuthMd5_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextIfAuthMd5Key(UI32_T vr_id, UI32_T vrf_id, UI32_T indexlen, OSPF_TYPE_MultiProcessIfAuthMd5_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetMultiProcIfEntry(UI32_T vr_id,UI32_T vrf_id, OSPF_TYPE_Msg_OspfInterfac_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetIfParamEntry(UI32_T vr_id, UI32_T vrf_id , OSPF_TYPE_IfParam_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_SetIfParamEntry(UI32_T vr_id, UI32_T vrf_id, OSPF_TYPE_IfParam_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetAreaTable(OSPF_TYPE_Area_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextAreaTable(OSPF_TYPE_Area_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetMultiProcessExtLsdbEntry(UI32_T vr_id, OSPF_TYPE_MultiProcessExtLsdb_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextMultiProcessExtLsdbEntry(UI32_T vr_id, UI32_T indexlen,  OSPF_TYPE_MultiProcessExtLsdb_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetMultiProcessLsdbEntry(UI32_T vr_id, OSPF_TYPE_MultiProcessLsdb_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextMultiProcessLsdbEntry(UI32_T vr_id, UI32_T indexlen,  OSPF_TYPE_MultiProcessLsdb_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetMultiProcessNbrEntry(UI32_T vr_id, OSPF_TYPE_MultiProcessNbr_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextMultiProcessNbrEntry(UI32_T vr_id, UI32_T indexlen,  OSPF_TYPE_MultiProcessNbr_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextMultiProcessSystemEntry(UI32_T vr_id, UI32_T vrf_id, UI32_T indexlen, OSPF_TYPE_MultiProcessSystem_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextNssaTable(OSPF_TYPE_Nssa_Area_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNssaTable(OSPF_TYPE_Nssa_Area_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetAreaRangeTable(OSPF_TYPE_Area_Range_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetStubAreaTable(OSPF_TYPE_Stub_Area_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextStubAreaTable(OSPF_TYPE_Stub_Area_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetRouterId(UI32_T vr_id, UI32_T proc_id, UI32_T *ret)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetMultiProcVirtualLinkEntry(OSPF_TYPE_Vlink_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextMultiProcVirtualLinkEntry(OSPF_TYPE_Vlink_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNetworkAreaTable(OSPF_TYPE_Network_Area_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetMultiProcVirtNbrEntry(OSPF_TYPE_MultiProcessVirtNbr_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextMultiProcVirtNbrEntry(OSPF_TYPE_MultiProcessVirtNbr_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetMultiProcVirtIfAuthMd5Entry(OSPF_TYPE_Vlink_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextMultiProcVirtIfAuthMd5Entry(OSPF_TYPE_Vlink_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetMultiProcSummaryAddrEntry(UI32_T vr_id,UI32_T vrf_id, OSPF_TYPE_Multi_Proc_Summary_Addr_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_IfMtuIgnoreSet(UI32_T vr_id, UI32_T ifindex, UI32_T mtu_ignore, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_RouterOspfSet(UI32_T vr_id, UI32_T vrf_id, UI32_T proc_id)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_RouterOspfUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T proc_id)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextMultiProcIfEntry(UI32_T vr_id,UI32_T vrf_id, OSPF_TYPE_Msg_OspfInterfac_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetNextMultiProcRedistEntry(UI32_T vr_id,UI32_T vrf_id, OSPF_TYPE_Multi_Proc_Redist_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AreaLimitSet(UI32_T vr_id, UI32_T proc_id, UI32_T limit)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_SetIfAuthMd5Key(UI32_T vr_id, UI32_T vrf_id, OSPF_TYPE_MultiProcessIfAuthMd5_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AreaAggregateEffectSet(UI32_T vr_id,UI32_T proc_id,UI32_T area_id,UI32_T type,UI32_T range_addr,UI32_T range_mask,UI32_T effect)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AreaAggregateStatusSet(UI32_T vr_id,UI32_T proc_id,UI32_T area_id,UI32_T type,UI32_T range_addr,UI32_T range_mask,UI32_T status)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_StubAreaStatusSet(UI32_T vr_id,UI32_T proc_id,UI32_T area_id, UI32_T tos, UI32_T status)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_StubAreaMetricSet(UI32_T vr_id,UI32_T proc_id,UI32_T area_id, UI32_T tos, UI32_T metric)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_AreaSummarySet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T val)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_IfMtuSet(UI32_T vr_id, UI32_T ifindex, UI32_T mtu, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_SetExtLsdbLimit(UI32_T vr_id, UI32_T proc_id, UI32_T limit)
{
    return OSPF_TYPE_RESULT_FAIL;
}

//TODO_K4
//==========================================

UI32_T OSPF6_PMGR_Interface_Get(OSPF6_TYPE_Interface_T *entry)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_IfParam_Get(OSPF6_TYPE_IfParam_T *entry)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_PassiveIfGetNext(UI32_T vr_id, char * tag, UI32_T * ifindex)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_AreaRange_GetNext(OSPF6_TYPE_Area_Range_T *entry)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_RedistributeEntry_GetNext(OSPF6_TYPE_Multi_Proc_Redist_T *data)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_LinkScopeLSA_GetNext(OSPF6_MGR_LSA_ENTRY_T *lsa_entry_p)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_Process_GetNext(OSPF6_MGR_OSPF_ENTRY_T *ospf_entry_p)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_ClearOspf6Process(UI32_T vr_id, char * tag)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_ASScopeLSA_GetNext(OSPF6_MGR_LSA_ENTRY_T *lsa_entry_p)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_AreaScopeLSA_GetNext(OSPF6_MGR_LSA_ENTRY_T *lsa_entry_p)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_TimerUnset(UI32_T vr_id, char * tag)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_RedistributeMetricTypeSet(UI32_T vr_id, char * tag, UI32_T proto, UI32_T metric_type)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_RedistributeMetricTypeUnset(UI32_T vr_id, char * tag, UI32_T proto)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_RouterIdSet(UI32_T vr_id, char * tag, UI32_T router_id)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_RouterIdUnset(UI32_T vr_id, char * tag)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_HoldTimerSet(UI32_T vr_id, char * tag, UI32_T hold)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_DelayTimerSet(UI32_T vr_id, char * tag, UI32_T delay)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_AreaVirtualLinkUnset(UI32_T vr_id, char * tag, UI32_T area_id, UI32_T peer, UI32_T type)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_Process_Get(OSPF6_MGR_OSPF_ENTRY_T *ospf_entry_p)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_AreaStubSet(UI32_T vr_id, char * tag, UI32_T area_id, UI32_T format)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_AreaStubUnset(UI32_T vr_id, char * tag, UI32_T area_id, UI32_T format)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_RedistributeProtoSet(UI32_T vr_id, char * tag, UI32_T proto)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_RedistributeProtoUnset(UI32_T vr_id, char * tag, UI32_T proto)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_RedistributeMetricSet(UI32_T vr_id, char * tag, UI32_T proto, UI32_T metric)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_RedistributeMetricUnset(UI32_T vr_id, char * tag, UI32_T proto)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_PassiveIfSet(UI32_T vr_id, char * tag, UI32_T ifindex)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_PassiveIfUnset(UI32_T vr_id, char * tag, UI32_T ifindex)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_ConcurrentDDSet(UI32_T vr_id, char * tag, UI32_T number)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_ConcurrentDDUnset(UI32_T vr_id, char * tag)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_DefaultMetricSet(UI32_T vr_id, char * tag, UI32_T metric)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_DefaultMetricUnset(UI32_T vr_id, char * tag)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_RouterOspfSet(UI32_T vr_id , char * tag)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_RouterOspfUnset(UI32_T vr_id, char * tag)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_IfRouterSet(UI32_T vr_id, char * tag, UI32_T ifindex, UI32_T aid, UI32_T format, UI32_T id)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_IfRouterUnset(UI32_T vr_id, char * tag, UI32_T ifindex, UI32_T aid, UI32_T id, UI32_T check_flag)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_AreaDefaultCostSet(UI32_T vr_id, char * tag, UI32_T area_id, UI32_T format, UI32_T cost)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_AreaDefaultCostUnset(UI32_T vr_id, char * tag, UI32_T area_id)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_ABRTypeSet(UI32_T vr_id, char * tag, UI32_T type)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_ABRTypeUnset(UI32_T vr_id, char * tag)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_IfCostSet(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T cost)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_IfCostUnset(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T check_flag)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_IfTransmitDelaySet(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T delay)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_IfTransmitDelayUnset(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T check_flag)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_IfRetransmitIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T interval)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_IfRetransmitIntervalUnset(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T check_flag)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_AreaRangeSet(OSPF6_TYPE_Area_Range_T * range)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_AreaRangeUnset(OSPF6_TYPE_Area_Range_T * range)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_AreaStubNoSummarySet(UI32_T vr_id, char * tag, UI32_T area_id, UI32_T format)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_AreaStubNoSummaryUnset(UI32_T vr_id, char * tag, UI32_T area_id, UI32_T format)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_AreaVirtualLinkSet(UI32_T vr_id, char * tag, UI32_T area_id, UI32_T format, UI32_T peer, UI32_T type, UI32_T value)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_IfPrioritySet(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T priority)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_IfPriorityUnset(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T check_flag)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_IfDeadIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T interval)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_IfDeadIntervalUnset(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T check_flag)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_IfHelloIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T interval)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_IfHelloIntervalUnset(UI32_T vr_id, UI32_T ifindex, UI32_T instance_id, UI32_T check_flag)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_Area_GetNext(OSPF6_TYPE_Area_T *entry)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_Interface_GetNext(OSPF6_TYPE_Interface_T *entry)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_Neighbor_GetNext(OSPF6_MGR_NBR_ENTRY_T *nbr_entry_p)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_VirtNeighbor_GetNext(OSPF6_MGR_VNBR_ENTRY_T *nbr_entry_p)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_Route_GetNext(OSPF6_TYPE_Route_T *entry)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

UI32_T OSPF6_PMGR_VirtualLink_GetNext(OSPF6_TYPE_Vlink_T *entry)
{
    return OSPF6_TYPE_RESULT_FAIL;
}

//=======================================================
UI32_T BGP_PMGR_DeleteRouteMap(char rmap_name[BGP_TYPE_ROUTE_MAP_NAME_LEN+1])
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_AddRouteMapSet(char rmap_name[BGP_TYPE_ROUTE_MAP_NAME_LEN+1], BOOL_T is_permit, UI32_T pref_index, char command[BGP_TYPE_ROUTE_MAP_COMMAND_LEN+1], char arg[BGP_TYPE_ROUTE_MAP_ARG_LEN+1])
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_AddRouteMap(char rmap_name[BGP_TYPE_ROUTE_MAP_NAME_LEN+1])
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_AddRouteMapPref(char rmap_name[BGP_TYPE_ROUTE_MAP_NAME_LEN+1], BOOL_T is_permit, UI32_T pref_index)
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

UI32_T BGP_PMGR_AddRouteMapMatch(char rmap_name[BGP_TYPE_ROUTE_MAP_NAME_LEN+1], BOOL_T is_permit, UI32_T pref_index, char command[BGP_TYPE_ROUTE_MAP_COMMAND_LEN+1], char arg[BGP_TYPE_ROUTE_MAP_ARG_LEN+1])
{
    return BGP_TYPE_RESULT_SEND_MSG_FAIL;
}

//=======================================================
UI32_T OSPF_PMGR_GetOspfEntry(OSPF_MGR_OSPF_ENTRY_T *ospf_entry_p)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetAreaPara(OSPF_TYPE_Area_Para_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

UI32_T OSPF_PMGR_GetOperatingIfParamEntry(UI32_T vr_id, UI32_T vrf_id, OSPF_TYPE_IfParam_T *entry)
{
    return OSPF_TYPE_RESULT_FAIL;
}

//TODO_K4

