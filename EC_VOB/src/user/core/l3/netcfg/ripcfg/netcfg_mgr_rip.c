/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_MGR_RIP.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2008/05/18     --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */
#include <string.h>
#include "sysfun.h"
#include "sys_type.h"
#include "ip_lib.h"
#include "netcfg_type.h"
#include "netcfg_mgr_rip.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "netcfg_om_rip.h"
#include "rip_pmgr.h"
#include "rip_type.h"
#include "vlan_lib.h"
#include "rip_type.h"
#include "l_radix.h"
#include "l_string.h"
#include "netcfg_om_ip.h"
#include "l4_pmgr.h"

SYSFUN_DECLARE_CSC

static BOOL_T is_provision_complete = FALSE;

static UI32_T NETCFG_MGR_RIP_RedistributeTypeCheck(char* protocol);
static BOOL_T NETCFG_MGR_RIP_GetIfindexFromIfname(char *ifname, UI32_T *ifindex);
static BOOL_T NETCFG_MGR_RIP_CheckDistributeTable(NETCFG_TYPE_RIP_Distribute_T *distribute_entry,
                                                                enum NETCFG_TYPE_RIP_Distribute_Type_E type,
                                                                enum NETCFG_TYPE_RIP_Distribute_List_Type_E list_type,
                                                                char *list_name);
static BOOL_T NETCFG_MGR_RIP_PacketDebugStatusCheck(NETCFG_TYPE_RIP_Packet_Debug_Type_T type,
                                                                     NETCFG_TYPE_RIP_Debug_Status_T *status);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetIfindexFromIfname
 * PURPOSE:
 *      Get ifindex from ifname
 *
 * INPUT:
 *      ifname.
 *
 * OUTPUT:
 *      ifindex.
 *
 * RETURN:
 *      TRUE/FALSE
 */
static BOOL_T NETCFG_MGR_RIP_GetIfindexFromIfname(char *ifname, UI32_T *ifindex)
{
    char    *ptr;
    int  vlan_id = 0;
    
    if(strncasecmp("vlan",ifname,4)!= 0)
      return FALSE;
    for(ptr = ifname + 4; *ptr != '\0'; ++ptr)
      if ((*ptr -'9' > 0) || (*ptr -'0'< 0))
        return FALSE;
    sscanf(ifname+4, "%d", &vlan_id);
    if(VLAN_OM_ConvertToIfindex(vlan_id, ifindex) == TRUE)
        return TRUE;
    else
        return FALSE;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_RedistributeTypeCheck
 * PURPOSE:
 *      Check the protocol type
 *
 * INPUT:
 *      protocol.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_RIP_Redistribute_Connected/
 *      NETCFG_TYPE_RIP_Redistribute_Static/
 *      NETCFG_TYPE_RIP_Redistribute_Ospf/
 *      NETCFG_TYPE_RIP_Redistribute_Bgp/
 *      NETCFG_TYPE_RIP_Redistribute_Max
 */
static UI32_T NETCFG_MGR_RIP_RedistributeTypeCheck(char* protocol)
{
    if (! strncmp ("connected", protocol, strlen(protocol)))
        return NETCFG_TYPE_RIP_Redistribute_Connected;
    else if (! strncmp ("static", protocol, strlen(protocol)))
        return NETCFG_TYPE_RIP_Redistribute_Static;
    else if (! strncmp ("ospf", protocol, strlen(protocol)))
        return NETCFG_TYPE_RIP_Redistribute_Ospf;
    else if (! strncmp ("bgp", protocol, strlen(protocol)))
        return NETCFG_TYPE_RIP_Redistribute_Bgp;
    else
        return NETCFG_TYPE_RIP_Redistribute_Max;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_PacketDebugStatusCheck
 * PURPOSE:
 *      Check if the packet debug status is configed.
 *
 * INPUT:
 *      type,
 *      status.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE/FALSE
 */
static BOOL_T NETCFG_MGR_RIP_PacketDebugStatusCheck(NETCFG_TYPE_RIP_Packet_Debug_Type_T type,
                                                                     NETCFG_TYPE_RIP_Debug_Status_T *status)
{
    BOOL_T result = FALSE;
    
    if(status == NULL)
        return FALSE;
    
    switch(type)
    {
        case NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE:
            if(status->packet_send_status && status->packet_recv_status)
                result = TRUE;
        break;

        case NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SEND: 
            if(status->packet_send_status)
                result = TRUE;
        break;
        
        case NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVE: 
            if(status->packet_recv_status)
                result = TRUE;
        break;

        case NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_DETAIL: 
            if(status->packet_send_status && status->packet_recv_status && status->packet_detail_status)
                result = TRUE;
        break;

        case NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SENDANDDETAIL: 
            if(status->packet_send_status && status->packet_detail_status)
                result = TRUE;
        break;

        case NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVEANDDETAIL: 
            if(status->packet_send_status && status->packet_recv_status && status->packet_detail_status)
                result = TRUE;
        break;

        default:
        break;    
    }
    return result;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_CheckDistributeTable
 * PURPOSE:
 *      Check if the distribute list exist
 *
 * INPUT:
 *      distribute_entry,
 *      type:in/out,
 *      list_type:acl/prefix
 *      list_name.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE/FALSE
 */
static BOOL_T NETCFG_MGR_RIP_CheckDistributeTable(NETCFG_TYPE_RIP_Distribute_T *distribute_entry,
                                                                enum NETCFG_TYPE_RIP_Distribute_Type_E type,
                                                                enum NETCFG_TYPE_RIP_Distribute_List_Type_E list_type,
                                                                char *list_name)
{
    if(list_name == NULL)
        return FALSE;
    
    if(((type != NETCFG_TYPE_RIP_DISTRIBUTE_IN) && (type != NETCFG_TYPE_RIP_DISTRIBUTE_OUT)) ||
        ((list_type != NETCFG_TYPE_RIP_DISTRIBUTE_ACCESS_LIST) && (list_type != NETCFG_TYPE_RIP_DISTRIBUTE_PREFIX_LIST)))
        return FALSE;
    
    if(list_type == NETCFG_TYPE_RIP_DISTRIBUTE_ACCESS_LIST)
    {
        if(strcmp(distribute_entry->acl_list[type], list_name) != 0)
            return FALSE;
        else
            return TRUE;
    }
    else
    {
        if(strcmp(distribute_entry->pre_list[type], list_name) != 0)
            return FALSE;
        else
            return TRUE;
    }
}


/* FUNCTION NAME : NETCFG_MGR_RIP_InitiateProcessResources
 * PURPOSE:
 *      Initialize NETCFG_MGR_RIP used system resource, eg. protection semaphore.
 *      Clear all working space.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  - Success
 *      FALSE - Fail
 */
BOOL_T NETCFG_MGR_RIP_InitiateProcessResources(void)
{
    NETCFG_OM_RIP_Init();
    return TRUE;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_EnterMasterMode
 * PURPOSE:
 *      Make Routing Engine enter master mode, handling all TCP/IP configuring requests,
 *      and receiving packets.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *
 */
void NETCFG_MGR_RIP_EnterMasterMode (void)
{
    NETCFG_OM_RIP_Init();
    SYSFUN_ENTER_MASTER_MODE();
} 

/* FUNCTION NAME : NETCFG_MGR_RIP_ProvisionComplete
 * PURPOSE:
 *      1. Let default gateway CFGDB into route when provision complete.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 *
 * NOTES:
 *
 */
void NETCFG_MGR_RIP_ProvisionComplete(void)
{
    is_provision_complete = TRUE;
} 

/* FUNCTION NAME : NETCFG_MGR_RIP_EnterSlaveMode
 * PURPOSE:
 *      Make Routing Engine enter slave mode, discarding all TCP/IP configuring requests,
 *      and receiving packets.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. In slave mode, just rejects function request and discard incoming message.
 */
void NETCFG_MGR_RIP_EnterSlaveMode (void)
{
    SYSFUN_ENTER_SLAVE_MODE();
}

/* FUNCTION NAME : NETCFG_MGR_RIP_SetTransitionMode
 * PURPOSE:
 *      Make Routing Engine enter transition mode, releasing all allocateing resource in master mode,
 *      discarding TCP/IP configuring requests, and receiving packets.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. In Transition Mode, must make sure all messages in queue are read
 *         and dynamic allocated space is free, resource set to INIT state.
 *      2. All function requests and incoming messages should be dropped.
 */
void NETCFG_MGR_RIP_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
}

/* FUNCTION NAME : NETCFG_MGR_RIP_EnterTransitionMode
 * PURPOSE:
 *      Make Routing Engine enter transition mode, releasing all allocateing resource in master mode,
 *      discarding TCP/IP configuring requests, and receiving packets.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. In Transition Mode, must make sure all messages in queue are read
 *         and dynamic allocated space is free, resource set to INIT state.
 *      2. All function requests and incoming messages should be dropped.
 */
void NETCFG_MGR_RIP_EnterTransitionMode (void)
{
    SYSFUN_ENTER_TRANSITION_MODE();
    NETCFG_OM_RIP_DeleteAllRipMasterEntry();
    is_provision_complete = FALSE;
}   

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_RIP_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for RIP MGR.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T NETCFG_MGR_RIP_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msg_p->type.result_bool = FALSE;
        msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    switch (msg_p->type.cmd)
    {
        case NETCFG_MGR_RIP_IPC_DEBUG:
            msg_p->type.result_bool = NETCFG_MGR_RIP_Debug();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_RIP_IPC_CONFIGDEBUG:
            msg_p->type.result_bool = NETCFG_MGR_RIP_ConfigDebug();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
            
        case NETCFG_MGR_RIP_IPC_UNDEBUG:
            msg_p->type.result_bool = NETCFG_MGR_RIP_UnDebug();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_RIP_IPC_CONFIGUNDEBUG:
            msg_p->type.result_bool = NETCFG_MGR_RIP_ConfigUnDebug();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
            
        case NETCFG_MGR_RIP_IPC_DEBUGEVENT:
            msg_p->type.result_bool = NETCFG_MGR_RIP_DebugEvent();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
            
        case NETCFG_MGR_RIP_IPC_CONFIGDEBUGEVENT:
            msg_p->type.result_bool = NETCFG_MGR_RIP_ConfigDebugEvent();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
            
        case NETCFG_MGR_RIP_IPC_UNDEBUGEVENT:
            msg_p->type.result_bool = NETCFG_MGR_RIP_UnDebugEvent();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
            
        case NETCFG_MGR_RIP_IPC_CONFIGUNDEBUGEVENT:
            msg_p->type.result_bool = NETCFG_MGR_RIP_ConfigUnDebugEvent();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_RIP_IPC_DEBUGPACKET:
            msg_p->type.result_bool = NETCFG_MGR_RIP_DebugPacket(msg_p->data.pdebug_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_RIP_IPC_CONFIGDEBUGPACKET:
            msg_p->type.result_bool = NETCFG_MGR_RIP_ConfigDebugPacket(msg_p->data.pdebug_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
            
        case NETCFG_MGR_RIP_IPC_UNDEBUGPACKET:
            msg_p->type.result_bool = NETCFG_MGR_RIP_UnDebugPacket(msg_p->data.pdebug_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_RIP_IPC_CONFIGUNDEBUGPACKET:
            msg_p->type.result_bool = NETCFG_MGR_RIP_ConfigUnDebugPacket(msg_p->data.pdebug_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_RIP_IPC_DEBUGNSM:
            msg_p->type.result_bool = NETCFG_MGR_RIP_DebugNsm();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
            
        case NETCFG_MGR_RIP_IPC_CONFIGDEBUGNSM:
            msg_p->type.result_bool = NETCFG_MGR_RIP_ConfigDebugNsm();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
            
        case NETCFG_MGR_RIP_IPC_UNDEBUGNSM:
            msg_p->type.result_bool = NETCFG_MGR_RIP_UnDebugNsm();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
            
        case NETCFG_MGR_RIP_IPC_CONFIGUNDEBUGNSM:
            msg_p->type.result_bool = NETCFG_MGR_RIP_ConfigUnDebugNsm();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_RIP_IPC_GETDEBUGSTATUS:
            msg_p->type.result_bool = NETCFG_MGR_RIP_GetDebugStatus(&(msg_p->data.status_v));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(status_v);
            break;
            
        case NETCFG_MGR_RIP_IPC_RECVPACKETSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_RecvPacketSet(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
            
        case NETCFG_MGR_RIP_IPC_RECVPACKETUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_RecvPacketUnset(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_RIP_IPC_SENDPACKETSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_SendPacketSet(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
            
        case NETCFG_MGR_RIP_IPC_SENDPACKETUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_SendPacketUnset(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_RIP_IPC_RECVVERSIONTYPESET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_RecvVersionTypeSet(msg_p->data.arg_grp6.arg1, msg_p->data.arg_grp6.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_RIP_IPC_RECVVERSIONUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_RecvVersionUnset(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_SENDVERSIONTYPESET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_SendVersionTypeSet(msg_p->data.arg_grp6.arg1, msg_p->data.arg_grp6.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_SENDVERSIONUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_SendVersionUnset(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_AUTHMODESET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_AuthModeSet(msg_p->data.arg_grp7.arg1, msg_p->data.arg_grp7.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_AUTHMODEUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_AuthModeUnset(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_AUTHSTRINGSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_AuthStringSet(msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_AUTHSTRINGUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_AuthStringUnset(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_AUTHKEYCHAINSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_AuthKeyChainSet(msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_AUTHKEYCHAINUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_AuthKeyChainUnset(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_SPLITHORIZONSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_SplitHorizonSet(msg_p->data.arg_grp7.arg1, msg_p->data.arg_grp7.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_SPLITHORIZONUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_SplitHorizonUnset(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_ROUTERRIPSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_RouterRipSet();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_ROUTERRIPUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_RouterRipUnset();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_VERSIONSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_VersionSet(msg_p->data.version_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_VERSIONUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_VersionUnset();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_NETWORKSETBYVID:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_NetworkSetByVid(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_NETWORKSETBYADDRESS:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_NetworkSetByAddress(msg_p->data.arg_grp9.arg1);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_NETWORKUNSETBYVID:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_NetworkUnsetByVid(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_RIP_IPC_NETWORKUNSETBYADDRESS:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_NetworkUnsetByAddress(msg_p->data.arg_grp9.arg1);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_NEIGHBORSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_NeighborSet(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_NEIGHBORUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_NeighborUnset(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_PASSIVEIFADD:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_PassiveIfAdd(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_PASSIVEIFDELETE:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_PassiveIfDelete(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_DEFAULTADD:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_DefaultAdd();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_RIP_IPC_DEFAULTDELETE:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_DefaultDelete();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_RIP_IPC_DEFAULTMETRICSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_DefaultMetricSet(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_RIP_IPC_DEFAULTMETRICUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_DefaultMetricUnset();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_RIP_IPC_DISTRIBUTELISTADD:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_DistributeListAdd(msg_p->data.arg_grp3.arg1,msg_p->data.arg_grp3.arg2,msg_p->data.arg_grp3.arg3,msg_p->data.arg_grp3.arg4);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_RIP_IPC_DISTRIBUTELISTDELETE:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_DistributeListDelete(msg_p->data.arg_grp3.arg1,msg_p->data.arg_grp3.arg2,msg_p->data.arg_grp3.arg3,msg_p->data.arg_grp3.arg4);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_RIP_IPC_TIMERSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_TimerSet(&(msg_p->data.timer_v));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_RIP_IPC_TIMERUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_TimerUnset();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_RIP_IPC_DISTANCEDEFAULTSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_DistanceDefaultSet(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_RIP_IPC_DISTANCEDEFAULTUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_DistanceDefaultUnset();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_RIP_IPC_DISTANCESET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_DistanceSet(msg_p->data.arg_grp4.arg1,msg_p->data.arg_grp4.arg2,msg_p->data.arg_grp4.arg3,msg_p->data.arg_grp4.arg4);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_RIP_IPC_DISTANCEUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_DistanceUnset(msg_p->data.arg_grp4.arg2,msg_p->data.arg_grp4.arg3);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_RIP_IPC_MAXPREFIXSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_MaxPrefixSet(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_RIP_IPC_MAXPREFIXUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_MaxPrefixUnset();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_RIP_IPC_RECVBUFFSIZESET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_RecvBuffSizeSet(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_RIP_IPC_RECVBUFFSIZEUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_RecvBuffSizeUnset();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_RIP_IPC_REDISTRIBUTESET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_RedistributeSet(msg_p->data.arg_grp5.arg1);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_RIP_IPC_REDISTRIBUTEMETRICSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_RedistributeMetricSet(msg_p->data.arg_grp5.arg1,msg_p->data.arg_grp5.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_RIP_IPC_REDISTRIBUTERMAPSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_RedistributeRmapSet(msg_p->data.arg_grp5.arg1, msg_p->data.arg_grp5.arg3);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_RIP_IPC_REDISTRIBUTEALLSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_RedistributeAllSet(msg_p->data.arg_grp5.arg1,msg_p->data.arg_grp5.arg2,msg_p->data.arg_grp5.arg3);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_RIP_IPC_REDISTRIBUTEUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_RedistributeUnset(msg_p->data.arg_grp5.arg1);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_RIP_IPC_CLEARROUTE:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_ClearRoute(msg_p->data.char_v);
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;
            
        case NETCFG_MGR_RIP_IPC_CLEARSTATISTICS:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_ClearStatistics();
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_RIP_IPC_GETINSTANCE:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_GetInstanceEntry(&(msg_p->data.instance_v));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(instance_v);
            break;

        case NETCFG_MGR_RIP_IPC_GETINTERFACE:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_GetInterfaceEntry(&(msg_p->data.if_v));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(if_v);
            break;

        case NETCFG_MGR_RIP_IPC_GETNEXTINTERFACE:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_GetNextInterfaceEntry(&(msg_p->data.if_v));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(if_v);
            break;

        case NETCFG_MGR_RIP_IPC_GETNETWORKTABLE:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_GetNetworkTable(&(msg_p->data.prefix_v));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(prefix_v);
            break;

        case NETCFG_MGR_RIP_IPC_GETNEXTNETWORKTABLE:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_GetNextNetworkTable(&(msg_p->data.prefix_v));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(prefix_v);
            break;

        case NETCFG_MGR_RIP_IPC_GETNEIGHBOR:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_GetNeighborTable(&(msg_p->data.prefix_v));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(prefix_v);
            break;

        case NETCFG_MGR_RIP_IPC_GETNEXTNEIGHBOR:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_GetNextNeighborTable(&(msg_p->data.prefix_v));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(prefix_v);
            break;

        case NETCFG_MGR_RIP_IPC_GETDISTANCETABLE:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_GetDistanceTable(&(msg_p->data.distance_v));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(distance_v);
            break;

        case NETCFG_MGR_RIP_IPC_GETNEXTDISTANCETABLE:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_GetNextDistanceTable(&(msg_p->data.distance_v));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(distance_v);
            break;

        case NETCFG_MGR_RIP_IPC_GETNEXTRIPROUTE:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_GetNextRouteEntry(&(msg_p->data.route_v));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(route_v);
            break;

        case NETCFG_MGR_RIP_IPC_GETNEXTTHREADTIMER:
            msg_p->type.result_bool = NETCFG_MGR_RIP_GetNextThreadTimer(&(msg_p->data.ui32_v));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
            break;

        case NETCFG_MGR_RIP_IPC_GETNEXTPEERENTRY:
            msg_p->type.result_bool = NETCFG_MGR_RIP_GetNextPeerEntry(&(msg_p->data.peer_v));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(peer_v);
            break;

        case NETCFG_MGR_RIP_IPC_GETPEERENTRY:
            msg_p->type.result_bool = NETCFG_MGR_RIP_GetPeerEntry(&(msg_p->data.peer_v));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(peer_v);
            break;
            
        case NETCFG_MGR_RIP_IPC_GETGLOBALSTATISTICS:
            msg_p->type.result_bool = NETCFG_MGR_RIP_GetGlobalStatistics(&(msg_p->data.global_stat_v));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(global_stat_v);
            break;
            
        case NETCFG_MGR_RIP_IPC_GETIFADDRESS:
            msg_p->type.result_bool = NETCFG_MGR_RIP_GetIfAddress(msg_p->data.arg_grp8.arg1, msg_p->data.arg_grp8.arg2, &(msg_p->data.arg_grp8.arg3));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp8);
            break;

        case NETCFG_MGR_RIP_IPC_GETIFRECVBADPACKET:
            msg_p->type.result_bool = NETCFG_MGR_RIP_GetIfRecvBadPacket(msg_p->data.arg_grp8.arg1, msg_p->data.arg_grp8.arg2, &(msg_p->data.arg_grp8.arg3));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp8);
            break;

        case NETCFG_MGR_RIP_IPC_GETIFRECVBADROUTE:
            msg_p->type.result_bool = NETCFG_MGR_RIP_GetIfRecvBadRoute(msg_p->data.arg_grp8.arg1, msg_p->data.arg_grp8.arg2, &(msg_p->data.arg_grp8.arg3));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp8);
            break;

        case NETCFG_MGR_RIP_IPC_GETIFSENDUPDATE:
            msg_p->type.result_bool = NETCFG_MGR_RIP_GetIfSendUpdate(msg_p->data.arg_grp8.arg1, msg_p->data.arg_grp8.arg2, &(msg_p->data.arg_grp8.arg3));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp8);
            break;
            
        case NETCFG_MGR_RIP_IPC_GETREDISTRIBUTETABLE:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_GetRedistributeTable(&(msg_p->data.redistribute_v));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(redistribute_v);
            break;

        case NETCFG_MGR_RIP_IPC_GETNEXTREDISTRIBUTETABLE:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_GetNextRedistributeTable(&(msg_p->data.redistribute_v));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(redistribute_v);
            break;

        case NETCFG_MGR_RIP_IPC_GETNEXTACTIVERIFBYVLANIFINDEX:
            msg_p->type.result_ui32 = NETCFG_MGR_RIP_GetNextActiveRifByVlanIfIndex(msg_p->data.arg_grp10.ui32, &(msg_p->data.arg_grp10.addr));
            msgbuf_p->msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp10);
            break;

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.result_bool = FALSE;
            msgbuf_p->msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} 

/* FUNCTION NAME : NETCFG_MGR_RIP_ConfigDebug
* PURPOSE:
*     RIP debug on config mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_ConfigDebug(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_Debug_Status_T status;

    memset(&status, 0, sizeof(NETCFG_TYPE_RIP_Debug_Status_T));

    if(NETCFG_OM_RIP_GetDebugStatus(vr_id, &status) != TRUE)
        return FALSE;
    
    if(status.event_all_status && status.packet_all_status && status.nsm_all_status)
        return TRUE;
    else
    {
        if(RIP_PMGR_ConfigDebug(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
            return FALSE;
        else 
        {
            NETCFG_OM_RIP_DebugSet(vr_id);
            return TRUE;
        }
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_Debug
* PURPOSE:
*     RIP debug on EXEC mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_Debug(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;

    if(RIP_PMGR_Debug(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
    {
        return FALSE;
    }
    else 
    {
        return TRUE;
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_ConfigUnDebug
* PURPOSE:
*     RIP undebug on CONFIG mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_ConfigUnDebug(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_Debug_Status_T status;

    memset(&status, 0, sizeof(NETCFG_TYPE_RIP_Debug_Status_T));

    if(NETCFG_OM_RIP_GetDebugStatus(vr_id, &status) != TRUE)
        return FALSE;
    
    if(!status.event_all_status && !status.packet_all_status && !status.nsm_all_status)
        return TRUE;
    else
    {
        if(RIP_PMGR_ConfigUnDebug(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
            return FALSE;
        else 
        {
            NETCFG_OM_RIP_DebugUnset(vr_id);
            return TRUE;
        }
    }
}


/* FUNCTION NAME : NETCFG_MGR_RIP_UnDebug
* PURPOSE:
*     RIP undebug on EXEC mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_UnDebug(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    if(RIP_PMGR_UnDebug(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
        return FALSE;
    else 
        return TRUE;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_ConfigDebugEvent
* PURPOSE:
*     RIP debug event on config mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_ConfigDebugEvent(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_Debug_Status_T status;

    memset(&status, 0, sizeof(NETCFG_TYPE_RIP_Debug_Status_T));

    if(NETCFG_OM_RIP_GetDebugStatus(vr_id, &status) != TRUE)
        return FALSE;
    
    if(status.event_all_status)
        return TRUE;
    else
    {
        if(RIP_PMGR_ConfigDebugEvent(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
            return FALSE;
        else 
        {
            NETCFG_OM_RIP_EventDebugSet(vr_id);
            return TRUE;
        }
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_ConfigUnDebugEvent
* PURPOSE:
*     RIP undebug event on config mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_ConfigUnDebugEvent(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_Debug_Status_T status;

    memset(&status, 0, sizeof(NETCFG_TYPE_RIP_Debug_Status_T));

    if(NETCFG_OM_RIP_GetDebugStatus(vr_id, &status) != TRUE)
        return FALSE;
    
    if(!status.event_all_status)
        return TRUE;
    else
    {
        if(RIP_PMGR_ConfigUnDebugEvent(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
            return FALSE;
        else 
        {
            NETCFG_OM_RIP_EventDebugUnset(vr_id);
            return TRUE;
        }
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_DebugEvent
* PURPOSE:
*     RIP debug event on EXEC mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_DebugEvent(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    if(RIP_PMGR_DebugEvent(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
        return FALSE;
    else 
        return TRUE;
}


/* FUNCTION NAME : NETCFG_MGR_RIP_UnDebugEvent
* PURPOSE:
*     RIP undebug event on EXEC mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_UnDebugEvent(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    if(RIP_PMGR_UnDebugEvent(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
        return FALSE;
    else 
        return TRUE;
}


/* FUNCTION NAME : NETCFG_MGR_RIP_DebugPacket
* PURPOSE:
*     RIP undebug packet on EXEC mode.
*
* INPUT:
*       type:
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE = 1,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SEND,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SENDANDDETAIL,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVE,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVEANDDETAIL,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_DETAIL
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_DebugPacket(NETCFG_TYPE_RIP_Packet_Debug_Type_T type)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    if(RIP_PMGR_DebugPacket(vr_id, vrf_id, type) != RIP_TYPE_RESULT_OK)
        return FALSE;
    else 
        return TRUE;
}


/* FUNCTION NAME : NETCFG_MGR_RIP_ConfigDebugPacket
* PURPOSE:
*     RIP debug packet on config mode.
*
* INPUT:
*       type:
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE = 1,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SEND,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SENDANDDETAIL,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVE,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVEANDDETAIL,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_DETAIL
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_ConfigDebugPacket(NETCFG_TYPE_RIP_Packet_Debug_Type_T type)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_Debug_Status_T status;

    if ((type < NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE) ||
        (type > NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_DETAIL))
    {
        return FALSE;
    }

    memset(&status, 0, sizeof(NETCFG_TYPE_RIP_Debug_Status_T));

    if(NETCFG_OM_RIP_GetDebugStatus(vr_id, &status) != TRUE)
        return FALSE;
    
    if(NETCFG_MGR_RIP_PacketDebugStatusCheck(type, &status) == TRUE)
        return TRUE;
    else
    {
        if(RIP_PMGR_ConfigDebugPacket(vr_id, vrf_id, type) != RIP_TYPE_RESULT_OK)
            return FALSE;
        else 
        {
            NETCFG_OM_RIP_PacketDebugSet(vr_id, type);
            return TRUE;
        }
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_UnDebugPacket
* PURPOSE:
*     RIP undebug packet on EXEC mode.
*
* INPUT:
*       type:
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE = 1,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SEND,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SENDANDDETAIL,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVE,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVEANDDETAIL,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_DETAIL
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_UnDebugPacket(NETCFG_TYPE_RIP_Packet_Debug_Type_T type)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    if(RIP_PMGR_UnDebugPacket(vr_id, vrf_id,type) != RIP_TYPE_RESULT_OK)
        return FALSE;
    else 
        return TRUE;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_ConfigUnDebugPacket
* PURPOSE:
*     RIP undebug packet on config mode.
*
* INPUT:
*       type:
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE = 1,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SEND,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SENDANDDETAIL,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVE,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVEANDDETAIL,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_DETAIL
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_ConfigUnDebugPacket(NETCFG_TYPE_RIP_Packet_Debug_Type_T type)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_Debug_Status_T status;

    memset(&status, 0, sizeof(NETCFG_TYPE_RIP_Debug_Status_T));

    if(NETCFG_OM_RIP_GetDebugStatus(vr_id, &status) != TRUE)
        return FALSE;
    
    if(NETCFG_MGR_RIP_PacketDebugStatusCheck(type, &status) == TRUE)
        return TRUE;
    else
    {
        if(RIP_PMGR_ConfigUnDebugPacket(vr_id, vrf_id, type) != RIP_TYPE_RESULT_OK)
            return FALSE;
        else 
        {
            NETCFG_OM_RIP_PacketDebugUnset(vr_id, type);
            return TRUE;
        }
    }
}


/* FUNCTION NAME : NETCFG_MGR_RIP_ConfigDebugNsm
* PURPOSE:
*     RIP debug nsm on config mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_ConfigDebugNsm(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_Debug_Status_T status;

    memset(&status, 0, sizeof(NETCFG_TYPE_RIP_Debug_Status_T));

    if(NETCFG_OM_RIP_GetDebugStatus(vr_id, &status) != TRUE)
        return FALSE;
    
    if(status.nsm_all_status)
        return TRUE;
    else
    {
        if(RIP_PMGR_ConfigDebugNsm(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
            return FALSE;
        else 
        {
            NETCFG_OM_RIP_NsmDebugSet(vr_id);
            return TRUE;
        }
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_ConfigUnDebugNsm
* PURPOSE:
*     RIP undebug nsm on config mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_ConfigUnDebugNsm(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_Debug_Status_T status;

    memset(&status, 0, sizeof(NETCFG_TYPE_RIP_Debug_Status_T));

    if(NETCFG_OM_RIP_GetDebugStatus(vr_id, &status) != TRUE)
        return FALSE;
    
    if(!status.nsm_all_status)
        return TRUE;
    else
    {
        if(RIP_PMGR_ConfigUnDebugNsm(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
            return FALSE;
        else 
        {
            NETCFG_OM_RIP_NsmDebugUnset(vr_id);
            return TRUE;
        }
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_DebugNsm
* PURPOSE:
*     RIP debug nsm on EXEC mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_DebugNsm(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    if(RIP_PMGR_DebugNsm(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
        return FALSE;
    else 
        return TRUE;
}


/* FUNCTION NAME : NETCFG_MGR_RIP_UnDebugNsm
* PURPOSE:
*     RIP undebug nsm on EXEC mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_UnDebugNsm(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    if(RIP_PMGR_UnDebugNsm(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
        return FALSE;
    else 
        return TRUE;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_GetDebugStatus
* PURPOSE:
*     Get RIP debug status on EXEC mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      status.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_GetDebugStatus(NETCFG_TYPE_RIP_Debug_Status_T *status)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;

    if(status == NULL)
        return FALSE;
    
    if(NETCFG_OM_RIP_GetDebugStatus(vr_id, status) != RIP_TYPE_RESULT_OK)
        return FALSE;
    else
        return TRUE;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_RecvPacketSet
* PURPOSE:
*     Set receive packet on an interface.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RecvPacketSet(UI32_T ifindex)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_If_T ifentry;
    
    memset(&ifentry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    ifentry.ifindex = ifindex;
    
    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &ifentry)!= TRUE)
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;

    if(ifentry.recv_packet != TRUE)
    {
        if(RIP_PMGR_RecvPacketSet(vr_id, vrf_id, ifindex) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_RecvPacketSet(vr_id,vrf_id, ifindex) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_RecvPacketUnset(vr_id, vrf_id, ifindex);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_RecvPacketUnset
* PURPOSE:
*     Unset receive packet on an interface.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RecvPacketUnset(UI32_T ifindex)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_If_T ifentry;
    
    memset(&ifentry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    ifentry.ifindex = ifindex;
    
    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &ifentry)!= TRUE)
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;

    if(ifentry.recv_packet != FALSE)
    {
        if(RIP_PMGR_RecvPacketUnset(vr_id, vrf_id, ifindex) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_RecvPacketUnset(vr_id,vrf_id, ifindex) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_RecvPacketSet(vr_id, vrf_id, ifindex);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_SendPacketSet
* PURPOSE:
*     set send packet on an interface.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_SendPacketSet(UI32_T ifindex)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_If_T ifentry;
    
    memset(&ifentry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    ifentry.ifindex = ifindex;
    
    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &ifentry)!= TRUE)
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;

    if(ifentry.send_packet != TRUE)
    {
        if(RIP_PMGR_SendPacketSet(vr_id, vrf_id, ifindex) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_SendPacketSet(vr_id, vrf_id, ifindex) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_SendPacketUnset(vr_id, vrf_id, ifindex);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_RIP_SendPacketUnset
* PURPOSE:
*     Unset send packet on an interface.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_SendPacketUnset(UI32_T ifindex)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_If_T ifentry;
    
    memset(&ifentry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    ifentry.ifindex = ifindex;
    
    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &ifentry)!= TRUE)
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;

    if(ifentry.send_packet != FALSE)  
    {
        if(RIP_PMGR_SendPacketUnset(vr_id, vrf_id, ifindex) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_SendPacketUnset(vr_id, vrf_id, ifindex) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_SendPacketSet(vr_id, vrf_id, ifindex);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_OK;
}



/* FUNCTION NAME : NETCFG_MGR_RIP_RecvVersionTypeSet
* PURPOSE:
*     Set RIP receive version.
*
* INPUT:
*      ifindex,
*      type:
*        rip version 1----- 1
*        rip version 2------2
*        rip version 1 and 2----3
*        rip version 1 compatible-----4
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RecvVersionTypeSet(UI32_T ifindex, enum NETCFG_TYPE_RIP_Version_E type)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_If_T ifentry;

    if ((type <= NETCFG_TYPE_RIP_VERSION_UNSPEC) ||
            (type >= NETCFG_TYPE_RIP_VERSION_VERSION_MAX))
    {
        return NETCFG_TYPE_INVALID_ARG;
    }

    memset(&ifentry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    ifentry.ifindex = ifindex;

    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &ifentry)!= TRUE)
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;

    if(RIP_PMGR_RecvVersionTypeSet(vr_id, vrf_id, ifindex, type) != RIP_TYPE_RESULT_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    else 
    {
        if(NETCFG_OM_RIP_RecvVersionTypeSet(vr_id, vrf_id, ifindex, type) != NETCFG_TYPE_OK)
        {
            RIP_PMGR_RecvVersionTypeSet(vr_id, vrf_id, ifindex, ifentry.recv_version_type);
            return NETCFG_TYPE_FAIL;
        }
    }
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_RIP_RecvVersionUnset
* PURPOSE:
*     unset RIP receive version.
*
* INPUT:
*      ifindex,
*
* OUTPUT:
*      None.
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RecvVersionUnset(UI32_T ifindex)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_If_T ifentry;

    memset(&ifentry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    ifentry.ifindex = ifindex;

    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &ifentry)!= TRUE)
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;

    if(RIP_PMGR_RecvVersionUnset(vr_id, vrf_id, ifindex) != RIP_TYPE_RESULT_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    else 
    {
        if(NETCFG_OM_RIP_RecvVersionUnset(vr_id, vrf_id, ifindex) != NETCFG_TYPE_OK)
        {
            RIP_PMGR_RecvVersionTypeSet(vr_id, vrf_id, ifindex, ifentry.recv_version_type);
            return NETCFG_TYPE_FAIL;
        }
    }
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_RIP_SendVersionTypeSet
* PURPOSE:
*     Set RIP send version.
*
* INPUT:
*      ifindex,
*      type:
*        rip version 1----- 1
*        rip version 2------2
*        rip version 1 and 2----3
*        rip version 1 compatible-----4
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_SendVersionTypeSet(UI32_T ifindex, enum NETCFG_TYPE_RIP_Version_E type)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_If_T ifentry;

    if ((type <= NETCFG_TYPE_RIP_VERSION_UNSPEC) ||
            (type >= NETCFG_TYPE_RIP_VERSION_VERSION_MAX))
    {
        return NETCFG_TYPE_INVALID_ARG;
    }

    memset(&ifentry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    ifentry.ifindex = ifindex;

    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &ifentry)!= TRUE)
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;

    if(RIP_PMGR_SendVersionTypeSet(vr_id, vrf_id, ifindex, type) != RIP_TYPE_RESULT_OK)
        return NETCFG_TYPE_FAIL;
    else 
    {
        if(NETCFG_OM_RIP_SendVersionTypeSet(vr_id, vrf_id, ifindex, type) != NETCFG_TYPE_OK)
        {
            RIP_PMGR_SendVersionTypeSet(vr_id, vrf_id, ifindex, ifentry.send_version_type);
            return NETCFG_TYPE_FAIL;
        }
    }
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_RIP_SendVersionUnset
* PURPOSE:
*     unset RIP send version.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_SendVersionUnset(UI32_T ifindex)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_If_T ifentry;

    memset(&ifentry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    ifentry.ifindex = ifindex;

    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &ifentry)!= TRUE)
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;

    if(RIP_PMGR_SendVersionUnset(vr_id, vrf_id, ifindex) != RIP_TYPE_RESULT_OK)
        return NETCFG_TYPE_FAIL;
    else 
    {
        if(NETCFG_OM_RIP_SendVersionUnset(vr_id, vrf_id, ifindex) != NETCFG_TYPE_OK)
        {
            RIP_PMGR_SendVersionTypeSet(vr_id, vrf_id, ifindex, ifentry.send_version_type);
            return NETCFG_TYPE_FAIL;
        }
    }
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_RIP_AuthModeSet
* PURPOSE:
*     Set RIP authentication mode.
*
* INPUT:
*      ifindex,
*      mode.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_AuthModeSet(UI32_T ifindex, enum NETCFG_TYPE_RIP_Auth_Mode_E mode)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_If_T ifentry;

    if ((mode < NETCFG_TYPE_RIP_NO_AUTH) ||
        (mode > NETCFG_TYPE_RIP_AUTH_MD5))
    {
        return NETCFG_TYPE_INVALID_ARG;
    }

    memset(&ifentry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    ifentry.ifindex = ifindex;
    
    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &ifentry)!= TRUE)
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;

    if(ifentry.auth_mode != mode)
    {
        if(RIP_PMGR_AuthModeSet(vr_id, vrf_id, ifindex, mode) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_AuthModeSet(vr_id, vrf_id, ifindex, mode) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_SendVersionTypeSet(vr_id, vrf_id, ifindex, ifentry.auth_mode);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_RIP_AuthModeUnset
* PURPOSE:
*     Unset RIP authentication mode.
*
* INPUT:
*      ifindex,
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_AuthModeUnset(UI32_T ifindex)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_If_T ifentry;
    
    memset(&ifentry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    ifentry.ifindex = ifindex;
    
    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &ifentry)!= TRUE)
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;

    if(ifentry.auth_mode != NETCFG_TYPE_RIP_NO_AUTH)   
    {
        if(RIP_PMGR_AuthModeUnset(vr_id, vrf_id, ifindex) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_AuthModeUnset(vr_id, vrf_id, ifindex) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_SendVersionTypeSet(vr_id, vrf_id, ifindex, ifentry.auth_mode);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_AuthStringSet
* PURPOSE:
*     set RIP authentication string.
*
* INPUT:
*      ifindex,
*      str
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_AuthStringSet(UI32_T ifindex, char *str)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_If_T ifentry;

    if (NULL == str)
        return NETCFG_TYPE_INVALID_ARG;

    if(strlen(str) > NETCFG_TYPE_RIP_AUTH_STRING_LENGTH)
        return NETCFG_TYPE_INVALID_ARG;
    
    memset(&ifentry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    ifentry.ifindex = ifindex;
    
    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &ifentry)!= TRUE)
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;

    if(RIP_PMGR_AuthStringSet(vr_id, vrf_id, ifindex, str) != RIP_TYPE_RESULT_OK)
        return NETCFG_TYPE_FAIL;
    else 
    {
        if(NETCFG_OM_RIP_AuthStringSet(vr_id, vrf_id, ifindex,str) != NETCFG_TYPE_OK)
        {
            RIP_PMGR_AuthStringSet(vr_id, vrf_id, ifindex, ifentry.auth_str);
            return NETCFG_TYPE_FAIL;
        }
        else
            return  NETCFG_TYPE_OK;
    }
}


/* FUNCTION NAME : NETCFG_MGR_RIP_AuthStringUnset
* PURPOSE:
*     Unset RIP authentication string.
*
* INPUT:
*      ifindex,
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_AuthStringUnset(UI32_T ifindex)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_If_T ifentry;
    
    memset(&ifentry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    ifentry.ifindex = ifindex;
    
    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &ifentry)!= TRUE)
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;

    if(RIP_PMGR_AuthStringUnset(vr_id, vrf_id, ifindex) != RIP_TYPE_RESULT_OK)
        return NETCFG_TYPE_FAIL;
    else 
    {
        if(NETCFG_OM_RIP_AuthStringUnset(vr_id, vrf_id, ifindex) != NETCFG_TYPE_OK)
        {
            RIP_PMGR_AuthStringSet(vr_id, vrf_id, ifindex, ifentry.auth_str);
            return NETCFG_TYPE_FAIL;
        }
        else
            return  NETCFG_TYPE_OK;
    }
}


/* FUNCTION NAME : NETCFG_MGR_RIP_AuthKeyChainSet
* PURPOSE:
*     set RIP authentication key-chain.
*
* INPUT:
*      ifindex,
*      str
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_AuthKeyChainSet(UI32_T ifindex, char *str)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_If_T ifentry;

    if (NULL == str)
        return NETCFG_TYPE_INVALID_ARG;

    if(strlen(str) > NETCFG_TYPE_RIP_AUTH_STRING_LENGTH)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&ifentry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    ifentry.ifindex = ifindex;
    
    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &ifentry)!= TRUE)
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;

    if(RIP_PMGR_AuthKeyChainSet(vr_id, vrf_id, ifindex, str) != RIP_TYPE_RESULT_OK)
        return NETCFG_TYPE_FAIL;
    else 
    {
        if(NETCFG_OM_RIP_AuthKeyChainSet(vr_id, vrf_id, ifindex,str) != NETCFG_TYPE_OK)
        {
            RIP_PMGR_AuthKeyChainSet(vr_id, vrf_id, ifindex, ifentry.key_chain);
            return NETCFG_TYPE_FAIL;
        }
        else
            return  NETCFG_TYPE_OK;
    }
}


/* FUNCTION NAME : NETCFG_MGR_RIP_AuthKeyChainUnset
* PURPOSE:
*     Unset RIP authentication key-chain.
*
* INPUT:
*      ifindex,
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_AuthKeyChainUnset(UI32_T ifindex)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_If_T ifentry;
    
    memset(&ifentry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    ifentry.ifindex = ifindex;
    
    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &ifentry)!= TRUE)
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;

    if(RIP_PMGR_AuthKeyChainUnset(vr_id, vrf_id, ifindex) != RIP_TYPE_RESULT_OK)
        return NETCFG_TYPE_FAIL;
    else 
    {
        if(NETCFG_OM_RIP_AuthKeyChainUnset(vr_id, vrf_id, ifindex) != NETCFG_TYPE_OK)
        {
            RIP_PMGR_AuthKeyChainSet(vr_id, vrf_id, ifindex, ifentry.key_chain);
            return NETCFG_TYPE_FAIL;
        }
        else
            return  NETCFG_TYPE_OK;
    }
}


/* FUNCTION NAME : NETCFG_MGR_RIP_SplitHorizonSet
* PURPOSE:
*     Set RIP split horizon.
*
* INPUT:
*      ifindex,
*      mode:
*        split horizon----- 0
*        split horizon poisoned------1
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_SplitHorizonSet(UI32_T ifindex, enum NETCFG_TYPE_RIP_Split_Horizon_E type)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_If_T ifentry;

    if ((type != NETCFG_TYPE_RIP_SPLIT_HORIZON_POISONED) &&
        (type != NETCFG_TYPE_RIP_SPLIT_HORIZON))
    {
        return NETCFG_TYPE_INVALID_ARG;
    }
    
    memset(&ifentry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    ifentry.ifindex = ifindex;
    
    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &ifentry)!= TRUE)
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;

    if(ifentry.split_horizon != type)   
    {
        if(RIP_PMGR_SplitHorizonSet(vr_id, vrf_id, ifindex, type) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_SplitHorizonSet(vr_id, vrf_id, ifindex, type) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_SplitHorizonSet(vr_id, vrf_id, ifindex, ifentry.split_horizon);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_RIP_SplitHorizonUnset
* PURPOSE:
*     unset rip spliet horizon.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_SplitHorizonUnset(UI32_T ifindex)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_If_T ifentry;
    
    memset(&ifentry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    ifentry.ifindex = ifindex;
    
    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &ifentry)!= TRUE)
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;

    if(ifentry.split_horizon != NETCFG_TYPE_RIP_SPLIT_HORIZON_NONE)  
    {
        if(RIP_PMGR_SplitHorizonUnset(vr_id, vrf_id, ifindex) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_SplitHorizonUnset(vr_id, vrf_id, ifindex) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_SplitHorizonSet(vr_id, vrf_id, ifindex, ifentry.split_horizon);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_RIP_RouterRipSet
* PURPOSE:
*     Set router rip.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RouterRipSet(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Instance_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)== TRUE)
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(RIP_PMGR_RouterRipSet(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else 
        {
            if(NETCFG_OM_RIP_AddInstance(vr_id, vrf_id) != TRUE)
            {
                RIP_PMGR_RouterRipUnset(vr_id, vrf_id);
                return NETCFG_TYPE_FAIL;
            }
            else
            {
                L4_PMGR_TrapPacket2Cpu(TRUE, RULE_TYPE_PacketType_RIP);
                return  NETCFG_TYPE_OK;
            }
        }
    }
}


/* FUNCTION NAME : NETCFG_MGR_RIP_RouterRipUnset
* PURPOSE:
*     unset router rip.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None,
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RouterRipUnset(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Instance_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    entry.instance_value.instance = vrf_id;
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(RIP_PMGR_RouterRipUnset(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else 
        {
            NETCFG_OM_RIP_DeleteInstanceEntry(vr_id, vrf_id);
            L4_PMGR_TrapPacket2Cpu(FALSE, RULE_TYPE_PacketType_RIP);
            return  NETCFG_TYPE_OK;
        }
    }
}


/* FUNCTION NAME : NETCFG_MGR_RIP_VersionSet
* PURPOSE:
*     set rip version.
*
* INPUT:
*      version.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_VersionSet(enum NETCFG_TYPE_RIP_Global_Version_E version)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Instance_T entry;

    if ((version < NETCFG_TYPE_RIP_GLOBAL_VERSION_1) ||
        (version > NETCFG_TYPE_RIP_GLOBAL_VERSION_BY_INTERFACE))
    {
        return NETCFG_TYPE_INVALID_ARG;
    }
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;

    if(entry.instance_value.version != version)
    {
        if(RIP_PMGR_VersionSet(vr_id, vrf_id, version) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_VersionSet(vr_id, vrf_id, version) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_VersionSet(vr_id, vrf_id, entry.instance_value.version);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_RIP_VersionUnset
* PURPOSE:
*     unset rip version.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None,
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_VersionUnset(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Instance_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;

    if(entry.instance_value.version != SYS_DFLT_RIP_GLOBAL_VERSION)
    {
        if(RIP_PMGR_VersionUnset(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_VersionUnset(vr_id, vrf_id) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_VersionSet(vr_id, vrf_id, entry.instance_value.version);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_RIP_NetworkSetByVid
* PURPOSE:
*     set rip network.
*
* INPUT:
*      vid: vlan index
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_NetworkSetByVid(UI32_T vid)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    UI32_T   ifindex;
    NETCFG_OM_RIP_Instance_T entry;
    NETCFG_TYPE_RIP_If_T       if_entry;

    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    memset(&if_entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
    {
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }

    
    VLAN_VID_CONVERTTO_IFINDEX(vid, ifindex);
    
        if_entry.ifindex = ifindex;
        
        if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &if_entry)!= TRUE)
            return NETCFG_TYPE_INTERFACE_NOT_EXISTED;
        
        if(if_entry.network_if == FALSE)
        {
            if(RIP_PMGR_NetworkSetByVid(vr_id, vrf_id, vid) != RIP_TYPE_RESULT_OK)
            {
                return NETCFG_TYPE_FAIL;
            }
            else 
            {
                if(NETCFG_OM_RIP_NetworkIfSet(vr_id, vrf_id, ifindex) != NETCFG_TYPE_OK)
                {
                    RIP_PMGR_NetworkUnsetByVid(vr_id, vrf_id, vid);
                    return NETCFG_TYPE_FAIL;
                }
                else
                    return  NETCFG_TYPE_OK;
            }
        }
        else
            return NETCFG_TYPE_OK;
    
   
    }

/* FUNCTION NAME : NETCFG_MGR_RIP_NetworkSetByAddress
* PURPOSE:
*     set rip network.
*
* INPUT:
*      network address -- network address
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_NetworkSetByAddress(L_PREFIX_T network_address)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    /*struct prefix p;*/
    NETCFG_OM_RIP_Instance_T entry;
    NETCFG_TYPE_RIP_If_T       if_entry;
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    memset(&if_entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
    {
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }

    
    
    
     L_PREFIX_ApplyMask(&network_address);
     if(NETCFG_OM_RIP_CheckNetworkTable(vr_id, vrf_id, &network_address) != TRUE)
     {
        if(RIP_PMGR_NetworkSetByAddress(vr_id, vrf_id, network_address) != RIP_TYPE_RESULT_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else 
        {
        	if(NETCFG_OM_RIP_NetworkTableSet(vr_id, vrf_id, &network_address) != NETCFG_TYPE_OK)
            {
            	RIP_PMGR_NetworkUnsetByAddress(vr_id, vrf_id, network_address);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
      }
      else
        return NETCFG_TYPE_OK;
   
}


/* FUNCTION NAME : NETCFG_MGR_RIP_NetworkUnsetByVid
* PURPOSE:
*     unset rip network.
*
* INPUT:
*      vid  --  vlan index
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_NetworkUnsetByVid(UI32_T vid)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    UI32_T   ifindex;
    NETCFG_OM_RIP_Instance_T entry;
    NETCFG_TYPE_RIP_If_T       if_entry;

    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    memset(&if_entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    
    VLAN_VID_CONVERTTO_IFINDEX(vid, ifindex);
    if_entry.ifindex = ifindex;
        
    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &if_entry)!= TRUE)
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;
    
    if(if_entry.network_if == TRUE)
    {
        if(RIP_PMGR_NetworkUnsetByVid(vr_id, vrf_id, vid) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_NetworkIfUnset(vr_id, vrf_id, ifindex) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_NetworkSetByVid(vr_id, vrf_id, vid);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_ENTRY_NOT_EXIST;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_NetworkUnsetByAddress
* PURPOSE:
*     unset rip network.
*
* INPUT:
*      network address   --  network address
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_NetworkUnsetByAddress(L_PREFIX_T network_address)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
  
    NETCFG_OM_RIP_Instance_T entry;
    NETCFG_TYPE_RIP_If_T       if_entry;

    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    memset(&if_entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    
    if(NETCFG_OM_RIP_CheckNetworkTable(vr_id, vrf_id, &network_address) == TRUE)
    {
        if(RIP_PMGR_NetworkUnsetByAddress(vr_id, vrf_id, network_address) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_NetworkTableUnset(vr_id, vrf_id, &network_address) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_NetworkSetByAddress(vr_id, vrf_id, network_address);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
     }
     else
        return NETCFG_TYPE_ENTRY_NOT_EXIST;
    
}

/* FUNCTION NAME : NETCFG_MGR_RIP_NeighborSet
* PURPOSE:
*     set rip neighbor
*
* INPUT:
*      ip_addr.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_NeighborSet(UI32_T ip_addr)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Instance_T entry;
    struct pal_in4_addr addr;
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    memset(&addr, 0, sizeof(struct pal_in4_addr));
    entry.instance_value.instance = vrf_id;
    addr.s_addr = ip_addr;
        
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
    {
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
       
    if(NETCFG_OM_RIP_CheckNeighbor(vr_id, vrf_id, &addr) != TRUE)
    {
        if(RIP_PMGR_NeighborSet(vr_id, vrf_id, &addr) != RIP_TYPE_RESULT_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else 
        {
            if(NETCFG_OM_RIP_NeighborSet(vr_id, vrf_id, &addr) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_NeighborUnset(vr_id, vrf_id, &addr);
                return NETCFG_TYPE_FAIL;
            }
            else
            {
                return  NETCFG_TYPE_OK;
            }
        }
    }
    else
    {
        return NETCFG_TYPE_OK;
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_NeighborUnset
* PURPOSE:
*     unset rip neighbor
*
* INPUT:
*      addr.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_NeighborUnset(UI32_T ip_addr)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Instance_T entry;
    struct pal_in4_addr addr;
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    memset(&addr, 0, sizeof(struct pal_in4_addr));
    entry.instance_value.instance = vrf_id;
    addr.s_addr = ip_addr;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
    {
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
       
    if(NETCFG_OM_RIP_CheckNeighbor(vr_id, vrf_id, &addr) == TRUE)
    {
        if(RIP_PMGR_NeighborUnset(vr_id, vrf_id, &addr) != RIP_TYPE_RESULT_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else 
        {
            if(NETCFG_OM_RIP_NeighborUnset(vr_id, vrf_id, &addr) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_NeighborSet(vr_id, vrf_id, &addr);
                return NETCFG_TYPE_FAIL;
            }
            else
            {
                return  NETCFG_TYPE_OK;
            }
        }
    }
    else
    {
        return NETCFG_TYPE_OK;
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_PassiveIfAdd
* PURPOSE:
*     add passive interface.
*
* INPUT:
*      vid
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_PassiveIfAdd(UI32_T vid)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    UI32_T   ifindex;
    NETCFG_OM_RIP_Instance_T instance_entry;
    NETCFG_TYPE_RIP_If_T entry;

    

    memset(&instance_entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));

    instance_entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&instance_entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;

    VLAN_VID_CONVERTTO_IFINDEX(vid, ifindex);
    
    entry.ifindex = ifindex;

    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &entry) == FALSE)
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;
    
    if(entry.pass_if == FALSE)
    {
        if(RIP_PMGR_PassiveIfAdd(vr_id, vrf_id, vid) != RIP_TYPE_RESULT_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else 
        {
            if(NETCFG_OM_RIP_PassiveIfSet(vr_id, vrf_id, ifindex) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_PassiveIfDelete(vr_id, vrf_id, vid);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_PassiveIfDelete
* PURPOSE:
*     delete passive interface.
*
* INPUT:
*      vid
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_PassiveIfDelete(UI32_T vid)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    UI32_T   ifindex;
    NETCFG_OM_RIP_Instance_T instance_entry;
    NETCFG_TYPE_RIP_If_T entry;


    memset(&instance_entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));

    instance_entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&instance_entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    
    VLAN_VID_CONVERTTO_IFINDEX(vid, ifindex);
    
    entry.ifindex = ifindex;

    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &entry) == FALSE)
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;
    
    if(entry.pass_if == TRUE)
    {
        if(RIP_PMGR_PassiveIfDelete(vr_id, vrf_id, vid) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_PassiveIfUnset(vr_id, vrf_id, ifindex) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_PassiveIfAdd(vr_id, vrf_id, vid);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_OK;

}

/* FUNCTION NAME : NETCFG_MGR_RIP_DefaultAdd
* PURPOSE:
*     originate rip default information.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_DefaultAdd(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Instance_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
    {
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    if(entry.instance_value.default_information != TRUE)
    {
        if(RIP_PMGR_DefaultAdd(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else 
        {
            if(NETCFG_OM_RIP_DefaultAdd(vr_id, vrf_id) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_DefaultDelete(vr_id, vrf_id);
                return NETCFG_TYPE_FAIL;
            }
            else
            {
                return  NETCFG_TYPE_OK;
            }
        }
    }
    else
    {
        return NETCFG_TYPE_OK;
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_DefaultDelete
* PURPOSE:
*     not originate rip default information.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_DefaultDelete(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Instance_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;

    if(entry.instance_value.default_information != SYS_DFLT_RIP_DEFAULT_ROUTE_ORIGINATE)
    {
        if(RIP_PMGR_DefaultDelete(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_DefaultDelete(vr_id, vrf_id) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_DefaultAdd(vr_id, vrf_id);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_DefaultMetricSet
* PURPOSE:
*     set rip default metric.
*
* INPUT:
*      metric.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_DefaultMetricSet(UI32_T metric)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Instance_T entry;

    if ((metric < NETCFG_TYPE_RIP_MIN_METRIC) ||
        (metric > NETCFG_TYPE_RIP_INFINITY_METRIC))
    {
        return NETCFG_TYPE_INVALID_ARG;
    }
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;

    if(entry.instance_value.default_metric != metric)
    {
        if(RIP_PMGR_DefaultMetricSet(vr_id, vrf_id, metric) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_DefaultMetricSet(vr_id, vrf_id, metric) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_DefaultMetricSet(vr_id, vrf_id, entry.instance_value.default_metric);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_DefaultMetricUnset
* PURPOSE:
*     unset rip default metric.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_DefaultMetricUnset(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Instance_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;

    if(entry.instance_value.default_metric != SYS_DFLT_RIP_DEFAULT_METRIC)
    {
        if(RIP_PMGR_DefaultMetricUnset(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_DefaultMetricUnset(vr_id, vrf_id) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_DefaultMetricSet(vr_id, vrf_id, entry.instance_value.default_metric);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_DistributeListAdd
* PURPOSE:
*     add distribute list.
*
* INPUT:
*      ifname.
*      list_name,
*      distribute type: in/out,
*      list type: access/prefix
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_DistributeListAdd(char *ifname, char *list_name,
                                                    enum NETCFG_TYPE_RIP_Distribute_Type_E type,
                                                    enum NETCFG_TYPE_RIP_Distribute_List_Type_E list_type)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    UI32_T   ifindex;
    NETCFG_OM_RIP_Instance_T entry;
    NETCFG_TYPE_RIP_If_T       if_entry;

    if((list_name == NULL) || (NULL == ifname) ||
       ((type != NETCFG_TYPE_RIP_DISTRIBUTE_IN) && (type != NETCFG_TYPE_RIP_DISTRIBUTE_OUT)) ||
       ((list_type != NETCFG_TYPE_RIP_DISTRIBUTE_ACCESS_LIST) && (list_type != NETCFG_TYPE_RIP_DISTRIBUTE_PREFIX_LIST)))
        return NETCFG_TYPE_INVALID_ARG;
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    memset(&if_entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;

    if(*ifname != 0)
    {
        NETCFG_MGR_RIP_GetIfindexFromIfname(ifname,&ifindex);
        if_entry.ifindex = ifindex;
        
        if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &if_entry)!= TRUE)
            return NETCFG_TYPE_INTERFACE_NOT_EXISTED;
        
        if(NETCFG_MGR_RIP_CheckDistributeTable(&if_entry.distribute_table, type, list_type, list_name) == TRUE)
            return NETCFG_TYPE_OK;
        else
        {
            if(RIP_PMGR_DistributeListAdd(vr_id, vrf_id, ifname, list_name, type, list_type) != RIP_TYPE_RESULT_OK)
                return NETCFG_TYPE_FAIL;
            else
            {
                if(NETCFG_OM_RIP_DistributeSet(vr_id, vrf_id, ifindex, list_name, type, list_type) != TRUE)
                {
                    if(list_type == NETCFG_TYPE_RIP_DISTRIBUTE_ACCESS_LIST)
                    {
                        if(*if_entry.distribute_table.acl_list[type] != 0)
                            RIP_PMGR_DistributeListAdd(vr_id, vrf_id, ifname, if_entry.distribute_table.acl_list[type], type, list_type);
                        else
                            RIP_PMGR_DistributeListDelete(vr_id, vrf_id, ifname, list_name, type, list_type);
                    }
                    else
                    {
                        if(*if_entry.distribute_table.pre_list[type] != 0)
                            RIP_PMGR_DistributeListAdd(vr_id, vrf_id, ifname, if_entry.distribute_table.pre_list[type], type, list_type);
                        else
                            RIP_PMGR_DistributeListDelete(vr_id, vrf_id, ifname, list_name, type, list_type);
                    }   
                    return NETCFG_TYPE_FAIL;
                }
                else
                    return NETCFG_TYPE_OK;
            }
        }            
    }
    else/*if not have ifname, modify all of interface distribute-list*/
    {
        if(RIP_PMGR_DistributeListDelete(vr_id, vrf_id, ifname, list_name, type, list_type) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else
        {
            NETCFG_OM_RIP_DistributeSetAllInterface(vr_id, vrf_id, list_name, type, list_type);
            return NETCFG_TYPE_OK;
        }
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_DistributeListDelete
* PURPOSE:
*     delete distribute list.
*
* INPUT:
*      ifname.
*      list_name,
*      distribute type: in/out,
*      list type: access/prefix
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_DistributeListDelete(char *ifname, char *list_name,
                                                       enum NETCFG_TYPE_RIP_Distribute_Type_E type,
                                                       enum NETCFG_TYPE_RIP_Distribute_List_Type_E list_type)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    UI32_T   ifindex;
    NETCFG_OM_RIP_Instance_T entry;
    NETCFG_TYPE_RIP_If_T       if_entry;

    if((list_name == NULL) || (NULL == ifname) ||
       ((type != NETCFG_TYPE_RIP_DISTRIBUTE_IN) && (type != NETCFG_TYPE_RIP_DISTRIBUTE_OUT)) ||
       ((list_type != NETCFG_TYPE_RIP_DISTRIBUTE_ACCESS_LIST) && (list_type != NETCFG_TYPE_RIP_DISTRIBUTE_PREFIX_LIST)))
        return NETCFG_TYPE_INVALID_ARG;
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    memset(&if_entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;

    if(*ifname != 0)
    {
        NETCFG_MGR_RIP_GetIfindexFromIfname(ifname,&ifindex);
        if_entry.ifindex = ifindex;

        if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &if_entry)!= TRUE)
            return NETCFG_TYPE_INTERFACE_NOT_EXISTED;
                
        if(NETCFG_MGR_RIP_CheckDistributeTable(&if_entry.distribute_table, type, list_type, list_name) != TRUE)/*if the list_name not match, can't delete*/
            return NETCFG_TYPE_INVALID_ARG;
        else
        {
            if(RIP_PMGR_DistributeListDelete(vr_id, vrf_id, ifname, list_name, type, list_type) != RIP_TYPE_RESULT_OK)
                return NETCFG_TYPE_FAIL;
            else
            {
                if(NETCFG_OM_RIP_DistributeUnset(vr_id, vrf_id, ifindex, list_name, type, list_type) != TRUE)
                {
                    if(list_type == NETCFG_TYPE_RIP_DISTRIBUTE_ACCESS_LIST)
                    {
                        if(*if_entry.distribute_table.acl_list[type] != 0)
                            RIP_PMGR_DistributeListAdd(vr_id, vrf_id, ifname, if_entry.distribute_table.acl_list[type], type, list_type);
                    }
                    else
                    {
                        if(*if_entry.distribute_table.pre_list[type] != 0)
                            RIP_PMGR_DistributeListAdd(vr_id, vrf_id, ifname, if_entry.distribute_table.pre_list[type], type, list_type);
                    }   
                    return NETCFG_TYPE_FAIL;
                }
                else
                    return NETCFG_TYPE_OK;
            }
        }
    }
    else/*if not have ifname, modify all of interface distribute-list*/
    {
        if(RIP_PMGR_DistributeListDelete(vr_id, vrf_id, ifname, list_name, type, list_type) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else
        {
            NETCFG_OM_RIP_DistributeUnsetAllInterface(vr_id, vrf_id, list_name, type, list_type);
            return NETCFG_TYPE_OK;
        }
    }
}


/* FUNCTION NAME : NETCFG_MGR_RIP_TimerSet
* PURPOSE:
*     set timer value.
*
* INPUT:
*      timer: update, timeout,carbage.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_TimerSet(NETCFG_TYPE_RIP_Timer_T *timer)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Instance_T entry;
    RIP_TYPE_Timer_T    local_timer;
    
    if (NULL == timer)
        return NETCFG_TYPE_INVALID_ARG;
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    memset(&local_timer, 0, sizeof(RIP_TYPE_Timer_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
    {
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    
    if(memcmp(&entry.instance_value.timer, timer, sizeof(entry.instance_value.timer)) != 0)
    {
        memcpy(&local_timer, timer, sizeof(RIP_TYPE_Timer_T));
        if(RIP_PMGR_TimerSet(vr_id, vrf_id, &local_timer) != RIP_TYPE_RESULT_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else 
        {
            if(NETCFG_OM_RIP_TimerSet(vr_id, vrf_id, timer) != NETCFG_TYPE_OK)
            {
                memcpy(&local_timer, &entry.instance_value.timer, sizeof(RIP_TYPE_Timer_T));
                RIP_PMGR_TimerSet(vr_id, vrf_id, &local_timer);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_TimerUnset
* PURPOSE:
*     unset timer.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_TimerUnset(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Instance_T entry;
    RIP_TYPE_Timer_T    local_timer;

    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    memset(&local_timer, 0, sizeof(RIP_TYPE_Timer_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;

    if(entry.instance_value.timer.update != SYS_DFLT_RIP_UPDATE_TIME || 
       entry.instance_value.timer.timeout!= SYS_DFLT_RIP_TIMEOUT_TIME||
       entry.instance_value.timer.garbage!= SYS_DFLT_RIP_GARBAGE_TIME)
    {
        if(RIP_PMGR_TimerUnset(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_TimerUnset(vr_id, vrf_id) != NETCFG_TYPE_OK)
            {
                memcpy(&local_timer, &entry.instance_value.timer, sizeof(RIP_TYPE_Timer_T));
                RIP_PMGR_TimerSet(vr_id, vrf_id, &local_timer);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_DistanceDefaultSet
* PURPOSE:
*     set distance value.
*
* INPUT:
*      distance.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_DistanceDefaultSet(UI32_T distance)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Instance_T entry;

    if ((distance < SYS_ADPT_MIN_ROUTE_DISTANCE) ||
        (distance > SYS_ADPT_MAX_ROUTE_DISTANCE))
    {
        return NETCFG_TYPE_INVALID_ARG;
    }
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;

    if(entry.instance_value.distance != distance)
    {
        if(RIP_PMGR_DistanceDefaultSet(vr_id, vrf_id, distance) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_DistanceDefaultSet(vr_id, vrf_id, distance) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_DistanceDefaultSet(vr_id, vrf_id, entry.instance_value.distance);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_DistanceDefaultUnset
* PURPOSE:
*     unset distance value.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_DistanceDefaultUnset(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Instance_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;

    if(entry.instance_value.distance != SYS_DFLT_RIP_DISTANCE)
    {
        if(RIP_PMGR_DistanceDefaultUnset(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_DistanceDefaultUnset(vr_id, vrf_id) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_DistanceDefaultSet(vr_id, vrf_id, entry.instance_value.distance);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_DistanceSet
* PURPOSE:
*     set distance .
*
* INPUT:
*      distance,
*      ip_addr,
*      plen: prefix length
*      alist.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_DistanceSet(UI32_T distance, UI32_T ip_addr, UI32_T plen, char *alist)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    BOOL_T check_distance_flag = FALSE;
    NETCFG_OM_RIP_Instance_T   entry;
    NETCFG_OM_RIP_Distance_T   distance_entry;
    struct pal_in4_addr addr;

    if ((distance < SYS_ADPT_MIN_ROUTE_DISTANCE) ||
        (distance > SYS_ADPT_MAX_ROUTE_DISTANCE))
    {
        return NETCFG_TYPE_INVALID_ARG;
    }

    if (plen < 0 || plen > IPV4_MAX_BITLEN)
        return NETCFG_TYPE_INVALID_ARG;

    if (NULL == alist)
        return NETCFG_TYPE_INVALID_ARG;
    
    memset(&addr, 0, sizeof(struct pal_in4_addr));
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    memset(&distance_entry, 0, sizeof(NETCFG_OM_RIP_Distance_T));
    entry.instance_value.instance = vrf_id;
    addr.s_addr = ip_addr;
    
    distance_entry.p.family = AF_INET;
    distance_entry.p.prefixlen = plen;
    memcpy(&distance_entry.p.u.prefix4, &addr, sizeof(struct pal_in4_addr));
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
    {
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }

    if(NETCFG_OM_RIP_GetDistanceTableEntry(vr_id, vrf_id, &distance_entry)== NETCFG_TYPE_OK)
    {
        check_distance_flag = TRUE;
    }
    
    if((distance_entry.distance != distance) || (*alist == 0) ||
        ((*alist != 0) && strcmp(distance_entry.alist_name,alist)))
    {
        if(RIP_PMGR_DistanceSet(vr_id, vrf_id, distance, &addr, plen, alist) != RIP_TYPE_RESULT_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else 
        {
            if(NETCFG_OM_RIP_DistanceSet(vr_id, vrf_id, &distance_entry.p, distance, alist) != NETCFG_TYPE_OK)
            {
                if(check_distance_flag)
                {
                    RIP_PMGR_DistanceSet(vr_id, vrf_id, distance_entry.distance, &addr, plen, distance_entry.alist_name);
                }
                else
                {
                    RIP_PMGR_DistanceUnset(vr_id, vrf_id, &addr, plen);
                }
                return NETCFG_TYPE_FAIL;
            }
            else
            {
                return  NETCFG_TYPE_OK;
            }
        }
    }
    else
    {
        return NETCFG_TYPE_OK;
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_DistanceUnset
* PURPOSE:
*     unset distance .
*
* INPUT:
*      ip_add,
*      plen: prefix length
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_DistanceUnset(UI32_T ip_addr, UI32_T plen)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Instance_T   entry;
    NETCFG_OM_RIP_Distance_T   distance_entry;
    struct pal_in4_addr addr;

    if (plen < 0 || plen > IPV4_MAX_BITLEN)
        return NETCFG_TYPE_INVALID_ARG;
    
    memset(&addr, 0, sizeof(struct pal_in4_addr));
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    memset(&distance_entry, 0, sizeof(NETCFG_OM_RIP_Distance_T));
    entry.instance_value.instance = vrf_id;
    addr.s_addr = ip_addr;
    
    distance_entry.p.family = AF_INET;
    distance_entry.p.prefixlen = plen;
    memcpy(&distance_entry.p.u.prefix4, &addr, sizeof(struct pal_in4_addr));
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
    {
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }

    if(NETCFG_OM_RIP_GetDistanceTableEntry(vr_id, vrf_id, &distance_entry)!= NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(RIP_PMGR_DistanceUnset(vr_id, vrf_id, &addr, plen) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_DistanceUnset(vr_id, vrf_id, &distance_entry.p) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_DistanceSet(vr_id, vrf_id, distance_entry.distance, &addr, plen, distance_entry.alist_name);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_MaxPrefixSet
* PURPOSE:
*     set max prefix value.
*
* INPUT:
*      pmax.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_MaxPrefixSet(UI32_T pmax)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Instance_T entry;

    if ((pmax < 1) ||
        (pmax > NETCFG_TYPE_RIP_MAX_NBR_OF_IPV4_ROUTE))
    {
        return NETCFG_TYPE_INVALID_ARG;
    }
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;

    if(entry.instance_value.pmax != pmax)
    {
        if(RIP_PMGR_MaxPrefixSet(vr_id, vrf_id, pmax) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_MaxPrefixSet(vr_id, vrf_id, pmax) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_MaxPrefixSet(vr_id, vrf_id, entry.instance_value.pmax);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_MaxPrefixUnset
* PURPOSE:
*     unset max prefix.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_MaxPrefixUnset(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Instance_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;

    if(entry.instance_value.pmax != SYS_DFLT_RIP_PREFIX_MAX)
    {
        if(RIP_PMGR_MaxPrefixUnset(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_MaxPrefixUnset(vr_id, vrf_id) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_MaxPrefixSet(vr_id, vrf_id, entry.instance_value.pmax);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_RecvBuffSizeSet
* PURPOSE:
*     set reveiver buffer size.
*
* INPUT:
*      buff_size.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RecvBuffSizeSet(UI32_T buff_size)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Instance_T entry;

    if ((buff_size < NETCFG_TYPE_RIP_RECVBUF_DEFAULT) ||
        (buff_size > NETCFG_TYPE_RIP_RECVBUF_MAXSIZE))
    {
        return NETCFG_TYPE_INVALID_ARG;
    }
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;

    if(entry.instance_value.recv_buffer_size != buff_size)
    {
        if(RIP_PMGR_RecvBuffSizeSet(vr_id, vrf_id, buff_size) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_RecvBuffSizeSet(vr_id, vrf_id, buff_size) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_RecvBuffSizeSet(vr_id, vrf_id, entry.instance_value.recv_buffer_size);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_RecvBuffSizeUnset
* PURPOSE:
*     unset receive buffer size.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RecvBuffSizeUnset(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Instance_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;

    if(entry.instance_value.recv_buffer_size != NETCFG_TYPE_RIP_RECVBUF_DEFAULT)
    {
        if(RIP_PMGR_RecvBuffSizeUnset(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
            return NETCFG_TYPE_FAIL;
        else 
        {
            if(NETCFG_OM_RIP_RecvBuffSizeUnset(vr_id, vrf_id) != NETCFG_TYPE_OK)
            {
                RIP_PMGR_RecvBuffSizeSet(vr_id, vrf_id, entry.instance_value.recv_buffer_size);
                return NETCFG_TYPE_FAIL;
            }
            else
                return  NETCFG_TYPE_OK;
        }
    }
    else
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_RedistributeSet
* PURPOSE:
*     set redistribute.
*
* INPUT:
*      protocol: protocol string.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RedistributeSet(char *protocol)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    UI32_T   type = NETCFG_TYPE_RIP_Redistribute_Max;
    NETCFG_OM_RIP_Instance_T entry;

    if (NULL == protocol)
        return NETCFG_TYPE_INVALID_ARG;
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;

    type = NETCFG_MGR_RIP_RedistributeTypeCheck(protocol);
    
    if(type == NETCFG_TYPE_RIP_Redistribute_Max)
        return NETCFG_TYPE_INVALID_ARG;
    
    if(RIP_PMGR_RedistributeSet(vr_id, vrf_id, protocol) != RIP_TYPE_RESULT_OK)
        return NETCFG_TYPE_FAIL;
    else 
    {
        if(NETCFG_OM_RIP_RedistributeSet(vr_id, vrf_id, type, 0, NULL) != NETCFG_TYPE_OK)
        {
            if(entry.instance_value.redistribute[type] != NULL)
            {
                if((entry.instance_value.redistribute[type]->metric) && (*entry.instance_value.redistribute[type]->rmap_name))
                    RIP_PMGR_RedistributeAllSet(vr_id, vrf_id, protocol,
                                                entry.instance_value.redistribute[type]->metric,
                                                entry.instance_value.redistribute[type]->rmap_name);
                else if(entry.instance_value.redistribute[type]->metric)
                    RIP_PMGR_RedistributeMetricSet(vr_id, vrf_id, protocol,
                                                   entry.instance_value.redistribute[type]->metric);
                else if(*entry.instance_value.redistribute[type]->rmap_name)
                    RIP_PMGR_RedistributeRmapSet(vr_id, vrf_id, protocol,
                                                 entry.instance_value.redistribute[type]->rmap_name);
            }
            else
            {
                RIP_PMGR_RedistributeUnset(vr_id, vrf_id, protocol);
            }
            return NETCFG_TYPE_FAIL;
        }
        else
            return  NETCFG_TYPE_OK;
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_RedistributeMetricSet
* PURPOSE:
*     set redistribute with metric.
*
* INPUT:
*      protocol: protocol string,
*      metric.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RedistributeMetricSet(char *protocol, UI32_T metric)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    UI32_T   type = NETCFG_TYPE_RIP_Redistribute_Max;
    NETCFG_OM_RIP_Instance_T entry;

    if (NULL == protocol)
        return NETCFG_TYPE_INVALID_ARG;

    if ((metric < 0) ||
        (metric > NETCFG_TYPE_RIP_INFINITY_METRIC))
    {
        return NETCFG_TYPE_INVALID_ARG;
    }
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;

    type = NETCFG_MGR_RIP_RedistributeTypeCheck(protocol);
    
    if(type == NETCFG_TYPE_RIP_Redistribute_Max)
        return NETCFG_TYPE_INVALID_ARG;
    
    if(RIP_PMGR_RedistributeMetricSet(vr_id, vrf_id, protocol, metric) != RIP_TYPE_RESULT_OK)
        return NETCFG_TYPE_FAIL;
    else 
    {
        if(NETCFG_OM_RIP_RedistributeSet(vr_id, vrf_id, type, metric, NULL) != NETCFG_TYPE_OK)
        {
            if(entry.instance_value.redistribute[type] != NULL)
            {
                if((entry.instance_value.redistribute[type]->metric) && (*entry.instance_value.redistribute[type]->rmap_name))
                    RIP_PMGR_RedistributeAllSet(vr_id, vrf_id, protocol,
                                                entry.instance_value.redistribute[type]->metric,
                                                entry.instance_value.redistribute[type]->rmap_name);
                else if(entry.instance_value.redistribute[type]->metric)
                    RIP_PMGR_RedistributeMetricSet(vr_id, vrf_id, protocol,
                                                   entry.instance_value.redistribute[type]->metric);
                else if(*entry.instance_value.redistribute[type]->rmap_name)
                    RIP_PMGR_RedistributeRmapSet(vr_id, vrf_id, protocol,
                                                 entry.instance_value.redistribute[type]->rmap_name);
                else 
                    RIP_PMGR_RedistributeSet(vr_id, vrf_id, protocol);
            }
            else
            {
                RIP_PMGR_RedistributeUnset(vr_id, vrf_id, protocol);
            }
            return NETCFG_TYPE_FAIL;
        }
        else
            return  NETCFG_TYPE_OK;
    }
}


/* FUNCTION NAME : NETCFG_MGR_RIP_RedistributeRmapSet
* PURPOSE:
*     set redistribute with route map.
*
* INPUT:
*      protocol: protocol string,
*      rmap.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RedistributeRmapSet(char *protocol, char *rmap)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    UI32_T   type = NETCFG_TYPE_RIP_Redistribute_Max;
    NETCFG_OM_RIP_Instance_T entry;

    if ((NULL == protocol) || (NULL == rmap))
        return NETCFG_TYPE_INVALID_ARG;
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;

    type = NETCFG_MGR_RIP_RedistributeTypeCheck(protocol);
    
    if(type == NETCFG_TYPE_RIP_Redistribute_Max)
        return NETCFG_TYPE_INVALID_ARG;
    
    if(RIP_PMGR_RedistributeRmapSet(vr_id, vrf_id, protocol, rmap) != RIP_TYPE_RESULT_OK)
        return NETCFG_TYPE_FAIL;
    else 
    {
        if(NETCFG_OM_RIP_RedistributeSet(vr_id, vrf_id, type, 0, rmap) != NETCFG_TYPE_OK)
        {
            if(entry.instance_value.redistribute[type] != NULL)
            {
                if((entry.instance_value.redistribute[type]->metric) && (*entry.instance_value.redistribute[type]->rmap_name))
                    RIP_PMGR_RedistributeAllSet(vr_id, vrf_id, protocol,
                                                entry.instance_value.redistribute[type]->metric,
                                                entry.instance_value.redistribute[type]->rmap_name);
                else if(entry.instance_value.redistribute[type]->metric)
                    RIP_PMGR_RedistributeMetricSet(vr_id, vrf_id, protocol,
                                                   entry.instance_value.redistribute[type]->metric);
                else if(*entry.instance_value.redistribute[type]->rmap_name)
                    RIP_PMGR_RedistributeRmapSet(vr_id, vrf_id, protocol,
                                                 entry.instance_value.redistribute[type]->rmap_name);
                else 
                    RIP_PMGR_RedistributeSet(vr_id, vrf_id, protocol);
            }
            else
            {
                RIP_PMGR_RedistributeUnset(vr_id, vrf_id, protocol);
            }
            return NETCFG_TYPE_FAIL;
        }
        else
            return  NETCFG_TYPE_OK;
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_RedistributeAllSet
* PURPOSE:
*     set redistribute with metric and route map.
*
* INPUT:
*      protocol: protocol string,
*      metric
*      rmap.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RedistributeAllSet(char *protocol, UI32_T metric, char *rmap)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    UI32_T   type = NETCFG_TYPE_RIP_Redistribute_Max;
    NETCFG_OM_RIP_Instance_T entry;

    if ((NULL == protocol) || (NULL == rmap))
        return NETCFG_TYPE_INVALID_ARG;

    if ((metric < 0) ||
        (metric > NETCFG_TYPE_RIP_INFINITY_METRIC))
    {
        return NETCFG_TYPE_INVALID_ARG;
    }
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;

    type = NETCFG_MGR_RIP_RedistributeTypeCheck(protocol);
    
    if(type == NETCFG_TYPE_RIP_Redistribute_Max)
        return NETCFG_TYPE_INVALID_ARG;
    
    if(RIP_PMGR_RedistributeAllSet(vr_id, vrf_id, protocol, metric, rmap) != RIP_TYPE_RESULT_OK)
        return NETCFG_TYPE_FAIL;
    else 
    {
        if(NETCFG_OM_RIP_RedistributeSet(vr_id, vrf_id, type, metric, rmap) != NETCFG_TYPE_OK)
        {
            if(entry.instance_value.redistribute[type] != NULL)
            {
                if((entry.instance_value.redistribute[type]->metric) && (*entry.instance_value.redistribute[type]->rmap_name))
                    RIP_PMGR_RedistributeAllSet(vr_id, vrf_id, protocol,
                                                entry.instance_value.redistribute[type]->metric,
                                                entry.instance_value.redistribute[type]->rmap_name);
                else if(entry.instance_value.redistribute[type]->metric)
                    RIP_PMGR_RedistributeMetricSet(vr_id, vrf_id, protocol,
                                                   entry.instance_value.redistribute[type]->metric);
                else if(*entry.instance_value.redistribute[type]->rmap_name)
                    RIP_PMGR_RedistributeRmapSet(vr_id, vrf_id, protocol,
                                                 entry.instance_value.redistribute[type]->rmap_name);
                else 
                    RIP_PMGR_RedistributeSet(vr_id, vrf_id, protocol);
            }
            else
            {
                RIP_PMGR_RedistributeUnset(vr_id, vrf_id, protocol);
            }
            return NETCFG_TYPE_FAIL;
        }
        else
            return  NETCFG_TYPE_OK;
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_RedistributeUnset
* PURPOSE:
*     unset redistribute.
*
* INPUT:
*      protocol: protocol string,
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RedistributeUnset(char *protocol)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    UI32_T   type = NETCFG_TYPE_RIP_Redistribute_Max;
    NETCFG_OM_RIP_Instance_T entry;

    if (NULL == protocol)
        return NETCFG_TYPE_INVALID_ARG;
    
    memset(&entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));
    entry.instance_value.instance = vrf_id;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id,vrf_id,&entry)!= TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;

    type = NETCFG_MGR_RIP_RedistributeTypeCheck(protocol);
    
    if(type == NETCFG_TYPE_RIP_Redistribute_Max)
        return NETCFG_TYPE_INVALID_ARG;
    
    if(RIP_PMGR_RedistributeUnset(vr_id, vrf_id, protocol) != RIP_TYPE_RESULT_OK)
        return NETCFG_TYPE_FAIL;
    else 
    {
        NETCFG_OM_RIP_RedistributeUnset(vr_id, vrf_id, type);
        return  NETCFG_TYPE_OK;
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_ClearRoute
* PURPOSE:
*     Clear rip router by type or by network address.
*
* INPUT:
*      arg
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      Input parameter 'arg', please input string "connected","static","ospf","rip","static","all" or "A.B.C.D/M".
*/
UI32_T NETCFG_MGR_RIP_ClearRoute(char *arg)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;

    if (NULL == arg)
        return NETCFG_TYPE_INVALID_ARG;
    
    if(RIP_PMGR_ClearRoute(vr_id, vrf_id, arg) != RIP_TYPE_RESULT_OK)
        return NETCFG_TYPE_FAIL;
    else 
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_ClearStatistics
* PURPOSE:
*     Clear rip statistics.
*
* INPUT:
*      None
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      
*/
UI32_T NETCFG_MGR_RIP_ClearStatistics(void)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T         vrf_id = SYS_DFLT_VRF_ID;
    if(RIP_PMGR_ClearStatistics(vr_id, vrf_id) != RIP_TYPE_RESULT_OK)
        return NETCFG_TYPE_FAIL;
    else 
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_SignalInterfaceAdd
* PURPOSE:
*     When add an l3 interface signal RIP.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      
*/
UI32_T NETCFG_MGR_RIP_SignalInterfaceAdd(UI32_T ifindex)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_If_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;

    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &entry) == TRUE)
    {   
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(RIP_PMGR_InterfaceAdd(vr_id, vrf_id, ifindex) != RIP_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            if(NETCFG_OM_RIP_AddInterface(vr_id, ifindex) != TRUE)
            {
                RIP_PMGR_InterfaceDelete(vr_id, vrf_id, ifindex);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }

    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_RIP_SignalInterfaceDelete
* PURPOSE:
*     When delete an l3 interface signal RIP.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      
*/
UI32_T NETCFG_MGR_RIP_SignalInterfaceDelete(UI32_T ifindex)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_TYPE_RIP_If_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    entry.ifindex = ifindex;

    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &entry) != TRUE)
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(RIP_PMGR_InterfaceDelete(vr_id, vrf_id, ifindex) != RIP_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            NETCFG_OM_RIP_DeleteInterface(vr_id, ifindex);
            return NETCFG_TYPE_OK;
        }
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_SignalLoopbackInterfaceAdd
* PURPOSE:
*     When add a loopback interface signal RIP.
*
* INPUT:
*      ifindex.
*      if_flags
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      
*/
UI32_T NETCFG_MGR_RIP_SignalLoopbackInterfaceAdd(UI32_T ifindex, UI16_T if_flags)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    if(RIP_PMGR_LoopbackInterfaceAdd(vr_id, vrf_id, ifindex, if_flags) != RIP_TYPE_RESULT_OK)
        return  NETCFG_TYPE_FAIL;

    /* There's no need to store the loopback interface in RIP_OM,
     * as nothing can be configured to the loopback interface for RIP.
     */
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_SignalInterfaceUp
 * PURPOSE:
 *     When a l3 interface up, signal RIP.
 *
 * INPUT:
 *      ifindex  -- Interface ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T NETCFG_MGR_RIP_SignalInterfaceUp(UI32_T ifindex)
{
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T vrf_id = SYS_DFLT_VRF_ID;
    
    if (RIP_PMGR_InterfaceUp(vr_id, vrf_id, ifindex) != RIP_TYPE_RESULT_OK)
    {
        return  NETCFG_TYPE_FAIL;
    }

    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_SignalInterfaceDown
 * PURPOSE:
 *     When a l3 interface down, signal RIP.
 *
 * INPUT:
 *      ifindex -- Interface ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T NETCFG_MGR_RIP_SignalInterfaceDown(UI32_T ifindex)
{
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T vrf_id = SYS_DFLT_VRF_ID;
    
    if (RIP_PMGR_InterfaceDown(vr_id, vrf_id, ifindex) != RIP_TYPE_RESULT_OK)
        return  NETCFG_TYPE_FAIL;

    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_RIP_SignalRifUp
 * PURPOSE:
 *     When a l3 interface rif up signal RIP.
 *
 * INPUT:
 *      ifindex  -- Interface ifindex
 *      ipaddr   -- RIF address
 *      ipmask   -- RIF address mask
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T NETCFG_MGR_RIP_SignalRifUp(UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask)
{
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T vrf_id = SYS_DFLT_VRF_ID;
    
    if (RIP_PMGR_RifUp(vr_id, vrf_id, ifindex, ip_addr, ip_mask) != RIP_TYPE_RESULT_OK)
        return  NETCFG_TYPE_FAIL;

    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_RIP_SignalRifDown
 * PURPOSE:
 *     When a l3 interface rif down signal RIP.
 *
 * INPUT:
 *      ifindex,
 *      ipaddr,
 *      ipmask,
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T NETCFG_MGR_RIP_SignalRifDown(UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask)
{
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T vrf_id = SYS_DFLT_VRF_ID;
    
    if (RIP_PMGR_RifDown(vr_id, vrf_id, ifindex, ip_addr, ip_mask) != RIP_TYPE_RESULT_OK)
        return  NETCFG_TYPE_FAIL;

    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_GetInstanceEntry
* PURPOSE:
*     Get rip instance entry.
*
* INPUT:
*      None
*
* OUTPUT:
*      entry.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      
*/
UI32_T NETCFG_MGR_RIP_GetInstanceEntry(NETCFG_TYPE_RIP_Instance_T *entry)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Instance_T instance_entry;
    memset(&instance_entry, 0, sizeof(NETCFG_OM_RIP_Instance_T));

    if (NULL == entry)
        return NETCFG_TYPE_INVALID_ARG;
    
    if(NETCFG_OM_RIP_GetInstanceEntry(vr_id, vrf_id, &instance_entry) != TRUE)
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    else
    {
        memcpy(entry, &instance_entry.instance_value, sizeof(NETCFG_TYPE_RIP_Instance_T));
        return NETCFG_TYPE_OK;
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_GetInterfaceEntry
* PURPOSE:
*     Get rip interface entry.
*
* INPUT:
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      
*/
UI32_T NETCFG_MGR_RIP_GetInterfaceEntry(NETCFG_TYPE_RIP_If_T *entry)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;

    if (NULL == entry)
        return NETCFG_TYPE_INVALID_ARG;
    
    if(NETCFG_OM_RIP_GetInterfaceEntry(vr_id, entry) != TRUE)
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;
    else
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_GetNextInterfaceEntry
* PURPOSE:
*     Getnext rip interface entry.
*
* INPUT:
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      if the entry.ifindex == 0, get first
*/
UI32_T NETCFG_MGR_RIP_GetNextInterfaceEntry(NETCFG_TYPE_RIP_If_T *entry)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;

    if (NULL == entry)
        return NETCFG_TYPE_INVALID_ARG;
    
    if(NETCFG_OM_RIP_GetNextInterfaceEntry(vr_id, entry) != TRUE)
        return NETCFG_TYPE_FAIL;
    else
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_GetNetworkTable
* PURPOSE:
*     Get rip network table.
*
* INPUT:
*      p
*
* OUTPUT:
*      p.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*     
*/
UI32_T NETCFG_MGR_RIP_GetNetworkTable(NETCFG_TYPE_RIP_Network_T *entry)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    struct prefix   p;

    if (NULL == entry)
        return NETCFG_TYPE_INVALID_ARG;
    
    memset(&p, 0 ,sizeof(struct prefix));
    p.family = AF_INET;
    p.prefixlen = entry->pfxlen;
    p.u.prefix4.s_addr = entry->ip_addr;
    
    if(NETCFG_OM_RIP_CheckNetworkTable(vr_id, vrf_id, &p) != TRUE)
        return NETCFG_TYPE_FAIL;
    else
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_GetNextNetworkTable
* PURPOSE:
*     Getnext rip network table.
*
* INPUT:
*      p
*
* OUTPUT:
*      p.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      if the p.prefix and p.u.prefix4.s_addr are 0, get first
*/
UI32_T NETCFG_MGR_RIP_GetNextNetworkTable(NETCFG_TYPE_RIP_Network_T *entry)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    struct prefix   p;

    if (NULL == entry)
        return NETCFG_TYPE_INVALID_ARG;
    
    memset(&p, 0 ,sizeof(struct prefix));
    p.family = AF_INET;
    p.prefixlen = entry->pfxlen;
    p.u.prefix4.s_addr = entry->ip_addr;
    
    if(NETCFG_OM_RIP_GetNextNetworkTableEntry(vr_id, vrf_id, &p) != TRUE)
        return NETCFG_TYPE_FAIL;
    else
    {
        entry->pfxlen = p.prefixlen;
        entry->ip_addr = p.u.prefix4.s_addr;
        return NETCFG_TYPE_OK;
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_GetNeighborTable
* PURPOSE:
*     Get rip Neighbor table.
*
* INPUT:
*      p
*
* OUTPUT:
*      p.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*     
*/
UI32_T NETCFG_MGR_RIP_GetNeighborTable(NETCFG_TYPE_RIP_Network_T *entry)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    struct prefix   p;

    if (NULL == entry)
        return NETCFG_TYPE_INVALID_ARG;
    
    memset(&p, 0 ,sizeof(struct prefix));
    p.family = AF_INET;
    p.prefixlen = IPV4_MAX_PREFIXLEN;
    p.u.prefix4.s_addr = entry->ip_addr;
    
    if(NETCFG_OM_RIP_CheckNeighbor(vr_id, vrf_id, &p.u.prefix4) != TRUE)
        return NETCFG_TYPE_FAIL;
    else
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_GetNextNeighborTable
* PURPOSE:
*     Getnext rip Neighbor table.
*
* INPUT:
*      p
*
* OUTPUT:
*      p.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      if the p.u.prefix4.s_addr is 0, get first
*/
UI32_T NETCFG_MGR_RIP_GetNextNeighborTable(NETCFG_TYPE_RIP_Network_T *entry)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    struct prefix   p;

    if (NULL == entry)
        return NETCFG_TYPE_INVALID_ARG;
    
    memset(&p, 0 ,sizeof(struct prefix));
    p.family = AF_INET;
    p.prefixlen = IPV4_MAX_PREFIXLEN;
    p.u.prefix4.s_addr = entry->ip_addr;
    
    if(NETCFG_OM_RIP_GetNextNeighborTable(vr_id, vrf_id, &p) != TRUE)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        entry->ip_addr = p.u.prefix4.s_addr;
        return NETCFG_TYPE_OK;
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_GetDistanceTable
* PURPOSE:
*     Get rip distance table.
*
* INPUT:
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*     
*/
UI32_T NETCFG_MGR_RIP_GetDistanceTable(NETCFG_TYPE_RIP_Distance_T *entry)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Distance_T distance_entry;

    if (NULL == entry)
        return NETCFG_TYPE_INVALID_ARG;
    
    memset(&distance_entry, 0, sizeof(NETCFG_OM_RIP_Distance_T));

    distance_entry.p.family = AF_INET;
    distance_entry.p.prefixlen = entry->pfxlen;
    distance_entry.p.u.prefix4.s_addr = entry->ip_addr;
    
    if(NETCFG_OM_RIP_GetDistanceTableEntry(vr_id, vrf_id, &distance_entry) != NETCFG_TYPE_OK)
        return NETCFG_TYPE_FAIL;
    else
    {
        memset(entry, 0, sizeof(NETCFG_TYPE_RIP_Distance_T));
        entry->pfxlen = distance_entry.p.prefixlen;
        entry->ip_addr = distance_entry.p.u.prefix4.s_addr;
        entry->distance = distance_entry.distance;
        strcpy(entry->alist_name, distance_entry.alist_name);
        return NETCFG_TYPE_OK;
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_GetNextDistanceTable
* PURPOSE:
*     Getnext rip distance table.
*
* INPUT:
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*       if entry.p.prefixlen and entry.p.u.prefix4.s_addr are 0 ,get first
*/
UI32_T NETCFG_MGR_RIP_GetNextDistanceTable(NETCFG_TYPE_RIP_Distance_T *entry)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    NETCFG_OM_RIP_Distance_T distance_entry;

    if (NULL == entry)
        return NETCFG_TYPE_INVALID_ARG;
    
    memset(&distance_entry, 0, sizeof(NETCFG_OM_RIP_Distance_T));

    distance_entry.p.prefixlen = entry->pfxlen;
    distance_entry.p.u.prefix4.s_addr = entry->ip_addr;
    
    if(NETCFG_OM_RIP_GetNextDistanceTable(vr_id, vrf_id, &distance_entry) != TRUE)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        memset(entry, 0, sizeof(NETCFG_TYPE_RIP_Distance_T));
        entry->pfxlen = distance_entry.p.prefixlen;
        entry->ip_addr = distance_entry.p.u.prefix4.s_addr;
        entry->distance = distance_entry.distance;
        strcpy(entry->alist_name, distance_entry.alist_name);
        return NETCFG_TYPE_OK;
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_GetNextRouteEntry
* PURPOSE:
*     Getnext rip route table.
*
* INPUT:
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*       
*/
UI32_T NETCFG_MGR_RIP_GetNextRouteEntry(NETCFG_TYPE_RIP_Route_T *entry)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    RIP_TYPE_Route_Entry_T tem_entry;

    if (NULL == entry)
        return NETCFG_TYPE_INVALID_ARG;

    memset(&tem_entry, 0, sizeof(RIP_TYPE_Route_Entry_T));
    tem_entry.dest_addr = entry->dest_addr;
    tem_entry.dest_pfxlen = entry->dest_pfxlen;
    
    if(RIP_PMGR_GetNextRouteEntry(vr_id, vrf_id, &tem_entry) != RIP_TYPE_RESULT_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        entry->dest_addr = tem_entry.dest_addr;
        entry->dest_pfxlen = tem_entry.dest_pfxlen;
        entry->nexthop_addr = tem_entry.nexthop_addr;
        entry->from_addr = tem_entry.from_addr;
        entry->ifindex = tem_entry.ifindex;
        entry->metric = tem_entry.metric;
        strcpy(entry->type_str, tem_entry.type_str);
        memcpy(entry->timebuf, tem_entry.timebuf, sizeof(tem_entry.timebuf));
        return NETCFG_TYPE_OK;
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_GetNextThreadTimer
* PURPOSE:
*     Get next thread timer.
*
* INPUT:
*      None
*
* OUTPUT:
*      nexttime.
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T NETCFG_MGR_RIP_GetNextThreadTimer(UI32_T *nexttime)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;

    if (NULL == nexttime)
        return FALSE;
   
    if(RIP_PMGR_GetNextThreadTimer(vr_id, vrf_id, nexttime))
        return TRUE;
    else
        return FALSE;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_GetNextPeerEntry
* PURPOSE:
*     Get next peer entry.
*
* INPUT:
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T NETCFG_MGR_RIP_GetNextPeerEntry(NETCFG_TYPE_RIP_Peer_Entry_T *entry)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    RIP_TYPE_Peer_Entry_T tem_entry;

    if (NULL == entry)
        return FALSE;
   
    memset(&tem_entry, 0, sizeof(RIP_TYPE_Peer_Entry_T));
    memcpy(&tem_entry, entry, sizeof(RIP_TYPE_Peer_Entry_T));
    
    if(RIP_PMGR_GetNextPeerEntry(vr_id, vrf_id, &tem_entry) != TRUE)
        return FALSE;
    else
    {
        memcpy(entry, &tem_entry, sizeof(NETCFG_TYPE_RIP_Peer_Entry_T));
        return TRUE;
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_GetPeerEntry
* PURPOSE:
*     Get peer entry.
*
* INPUT:
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T NETCFG_MGR_RIP_GetPeerEntry(NETCFG_TYPE_RIP_Peer_Entry_T *entry)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    RIP_TYPE_Peer_Entry_T tem_entry;

    if (NULL == entry)
        return FALSE;
   
    memset(&tem_entry, 0, sizeof(RIP_TYPE_Peer_Entry_T));
    memcpy(&tem_entry, entry, sizeof(RIP_TYPE_Peer_Entry_T));
    
    if(RIP_PMGR_GetPeerEntry(vr_id, vrf_id, &tem_entry) != TRUE)
        return FALSE;
    else
    {
        memcpy(entry, &tem_entry, sizeof(NETCFG_TYPE_RIP_Peer_Entry_T));
        return TRUE;
    }
}


/* FUNCTION NAME : NETCFG_MGR_RIP_GetGlobalStatistics
* PURPOSE:
*     Get rip global statistics, include global route changes and blobal queries.
*
* INPUT:
*      None
*
* OUTPUT:
*      stat.
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T NETCFG_MGR_RIP_GetGlobalStatistics(NETCFG_TYPE_RIP_Global_Statistics_T *stat)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    RIP_TYPE_Global_Statistics_T tem_entry;

    if (NULL == stat)
        return FALSE;
   
    memset(&tem_entry, 0, sizeof(RIP_TYPE_Global_Statistics_T));
    
    if(RIP_PMGR_GetGlobalStatistics(vr_id, vrf_id, &tem_entry) != TRUE)
        return FALSE;
    else
    {
        memcpy(stat, &tem_entry, sizeof(NETCFG_TYPE_RIP_Global_Statistics_T));
        return TRUE;
    }
}

/* FUNCTION NAME : NETCFG_MGR_RIP_GetIfAddress
* PURPOSE:
*     Get or getnext interface IP address.
*
* INPUT:
*      exact: 0- getnext/1 - get,
*      in_addr
*
* OUTPUT:
*      out_addr.
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T NETCFG_MGR_RIP_GetIfAddress(UI32_T exact, UI32_T in_addr, UI32_T *out_addr)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;

    if (NULL == out_addr)
        return FALSE;

    if(RIP_PMGR_GetIfAddress(vr_id, vrf_id, exact, in_addr, out_addr) != TRUE)
        return FALSE;
    else
        return TRUE;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_GetIfRecvBadPacket
* PURPOSE:
*     Get or getnext interface receive bad packet.
*
* INPUT:
*      exact: 0- getnext/1 - get,
*      in_addr
*
* OUTPUT:
*      value.
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T NETCFG_MGR_RIP_GetIfRecvBadPacket(UI32_T exact, UI32_T in_addr, UI32_T *value)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;

    if (NULL == value)
        return FALSE;
    
    if(RIP_PMGR_GetIfRecvBadPacket(vr_id, vrf_id, exact, in_addr, value) != TRUE)
        return FALSE;
    else
        return TRUE;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_GetIfRecvBadRoute
* PURPOSE:
*     Get or getnext interface receive bad route.
*
* INPUT:
*      exact: 0- getnext/1 - get,
*      in_addr
*
* OUTPUT:
*      value.
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T NETCFG_MGR_RIP_GetIfRecvBadRoute(UI32_T exact, UI32_T in_addr, UI32_T *value)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;

    if (NULL == value)
        return FALSE;
    
    if(RIP_PMGR_GetIfRecvBadRoute(vr_id, vrf_id, exact, in_addr, value) != TRUE)
        return FALSE;
    else
        return TRUE;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_GetIfSendUpdate
* PURPOSE:
*     Get or getnext interface send update.
*
* INPUT:
*      exact: 0- getnext/1 - get,
*      in_addr
*
* OUTPUT:
*      value.
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T NETCFG_MGR_RIP_GetIfSendUpdate(UI32_T exact, UI32_T in_addr, UI32_T *value)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;

    if (NULL == value)
        return FALSE;
    
    if(RIP_PMGR_GetIfSendUpdate(vr_id, vrf_id, exact, in_addr, value) != TRUE)
        return FALSE;
    else
        return TRUE;
}

/* FUNCTION NAME : NETCFG_MGR_RIP_GetRedistributeTable
* PURPOSE:
*     Get rip redistribute table.
*
* INPUT:
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      key is entry->protocol
*/
UI32_T NETCFG_MGR_RIP_GetRedistributeTable(NETCFG_TYPE_RIP_Redistribute_Table_T *entry)
{
    NETCFG_TYPE_RIP_Instance_T instance_entry;

    if (NULL == entry)
        return NETCFG_TYPE_INVALID_ARG;
    
    memset(&instance_entry, 0, sizeof(NETCFG_TYPE_RIP_Instance_T));
    if(NETCFG_MGR_RIP_GetInstanceEntry(&instance_entry) != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    else
    {
        if(instance_entry.redistribute[entry->protocol] != NULL)
        {
            memcpy(&entry->table, instance_entry.redistribute[entry->protocol], sizeof(NETCFG_TYPE_RIP_Redistribute_T));

            return NETCFG_TYPE_OK;
        }
    }
    return NETCFG_TYPE_FAIL;
}


/* FUNCTION NAME : NETCFG_MGR_RIP_GetNextRedistributeTable
* PURPOSE:
*     Get next rip redistribute table.
*
* INPUT:
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      key is entry->protocol, if the protocol is NETCFG_TYPE_RIP_Redistribute_Max , get first
*/
UI32_T NETCFG_MGR_RIP_GetNextRedistributeTable(NETCFG_TYPE_RIP_Redistribute_Table_T *entry)
{
    enum NETCFG_TYPE_RIP_Redistribute_Type_E   type;
    enum NETCFG_TYPE_RIP_Redistribute_Type_E   pre_type;
    NETCFG_TYPE_RIP_Instance_T instance_entry;

    if (NULL == entry)
        return NETCFG_TYPE_INVALID_ARG;
    
    memset(&instance_entry, 0, sizeof(NETCFG_TYPE_RIP_Instance_T));
    if(NETCFG_MGR_RIP_GetInstanceEntry(&instance_entry) != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_INSTANCE_NOT_EXIST;
    }
    else
    {
        pre_type = entry->protocol;
        if(entry->protocol == NETCFG_TYPE_RIP_Redistribute_Max)
        {
            type = NETCFG_TYPE_RIP_Redistribute_Connected;
        }
        else
        {
            type = entry->protocol;
            type++;
        }

        for( ; type < NETCFG_TYPE_RIP_Redistribute_Max; type++)
        {
            if(instance_entry.redistribute[type] != NULL)
            {
                entry->protocol = type;
                memcpy(&entry->table, instance_entry.redistribute[type], sizeof(NETCFG_TYPE_RIP_Redistribute_T));
                break;
            }
        }
        
        if(entry->protocol == pre_type)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }
}


/* FUNCTION NAME : NETCFG_MGR_RIP_GetNextActiveRifByVlanIfIndex
 * PURPOSE:
 *     Get next rif which is rip enabled.
 *
 * INPUT:
 *      ifindex -- ifindex of vlan
 *
 * OUTPUT:
 *      addr_p  -- ip address of rif.
 *
 * RETURN:
 *       NETCFG_TYPE_OK / NETCFG_TYPE_NO_MORE_ENTRY
 *
 * NOTES:
 *
 */
UI32_T NETCFG_MGR_RIP_GetNextActiveRifByVlanIfIndex(UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{

    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T vrf_id = SYS_DFLT_VRF_ID;

    NETCFG_TYPE_RIP_If_T entry;
    UI32_T ip_addr, node_mask;
    struct prefix p;
    BOOL_T network_vlan = FALSE;

    memset(&entry, 0, sizeof(entry));
    entry.ifindex = ifindex;
    if(TRUE == NETCFG_OM_RIP_GetInterfaceEntry(vr_id, &entry))
    {
        if(entry.network_if)
            network_vlan = TRUE;
    }

    
    NETCFG_TYPE_InetRifConfig_T rif_config;
    memset(&rif_config, 0, sizeof(rif_config));
    rif_config.ifindex = ifindex;
    rif_config.addr = *addr_p;
    
    while(NETCFG_TYPE_OK == NETCFG_OM_IP_GetNextRifFromInterface(&rif_config))
    {

        if(rif_config.ifindex != ifindex)
            return NETCFG_TYPE_NO_MORE_ENTRY;
        if(network_vlan)
        {
            *addr_p = rif_config.addr;
            return NETCFG_TYPE_OK;
        }           

        memset(&p, 0 ,sizeof(struct prefix));
        while(NETCFG_OM_RIP_GetNextNetworkTable(vr_id, vrf_id, &p)== TRUE)
        {    
            UI8_T arr_mask[4] = {};

            IP_LIB_CidrToMask(p.prefixlen, arr_mask);
            IP_LIB_ArraytoUI32(arr_mask, &node_mask);
            
            IP_LIB_ArraytoUI32(rif_config.addr.addr, &ip_addr);
            if((ip_addr & node_mask) == (p.u.prefix4.s_addr & node_mask))
            {    

                *addr_p = rif_config.addr;
                return NETCFG_TYPE_OK;
            }
        } /* while */
    } /* while */
    return NETCFG_TYPE_NO_MORE_ENTRY;
}
