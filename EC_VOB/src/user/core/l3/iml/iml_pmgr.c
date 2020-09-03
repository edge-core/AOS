/* MODULE NAME:  iml_pmgr.c
 * PURPOSE:
 *    This is a sample code for implementation of PMGR.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    7/10/2007 - kh shi, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include "sys_bld.h"
#include "sysfun.h"
#include "iml_mgr.h"
#include "iml_pmgr.h"
#include "iml_type.h"
#include "sys_module.h"
#include "l_mm.h"
#include "l_ipcmem.h"

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
static SYSFUN_MsgQ_T ipcmsgq_handle;

/* EXPORTED SUBPROGRAM BODIES
 */
/* ----------------------------------------------------------------------------------
 * FUNCTION : IML_PMGR_InitiateProcessResource
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Initial resource for process who want to call IML_PMGR
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE    -- success
 *            FALSE   -- fail
 * NOTE     : This function must be called before calling any other IML_PMGR functions
 * ----------------------------------------------------------------------------------*/
BOOL_T IML_PMGR_InitiateProcessResource(void)
{
    /* Given that IML PMGR requests are handled in l2mux group of L2_L4_PROC
     */
    if(SYSFUN_GetMsgQ(SYS_BLD_L2MUX_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

UI32_T IML_PMGR_SendPkt(L_MM_Mref_Handle_T *mref_handle_p, UI32_T ifindex, UI32_T packet_length, 
                        UI8_T *dst_mac, UI8_T *src_mac, UI16_T packet_type, BOOL_T forward)
{
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(IML_MGR_GET_MSGBUFSIZE(IML_MGR_IPCMsg_SendPacket_Data_S))];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    IML_MGR_IPCMsg_T    *data_p;

    msgbuf_p->cmd = SYS_MODULE_IML;

    data_p = (IML_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = IML_MGR_IPC_SEND_PACKET;
    data_p->data.SendPacket_data.mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    data_p->data.SendPacket_data.ifindex            = ifindex;
    data_p->data.SendPacket_data.packet_length      = packet_length;
    data_p->data.SendPacket_data.packet_type        = packet_type;
    data_p->data.SendPacket_data.forward            = forward;
    memcpy(data_p->data.SendPacket_data.dst_mac,dst_mac,SYS_ADPT_MAC_ADDR_LEN);
    memcpy(data_p->data.SendPacket_data.src_mac,src_mac,SYS_ADPT_MAC_ADDR_LEN);
            
    msgbuf_p->msg_size = IML_MGR_GET_MSGBUFSIZE(IML_MGR_IPCMsg_SendPacket_Data_S);

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, IML_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return -1;
    }
    return data_p->type.result;
}

#if (SYS_CPNT_DHCPV6SNP == TRUE)
/* FUNCTION NAME: IML_PMGR_SendPktByVlanEntry
 * PURPOSE: Send  packet by DHCPV6SNP according to vlan entry
 * INPUT:  mref_handle_p -- L_MREF descriptor
 *             packet_length  -- packet length
 *             vid                  -- vid
 *             dst_mac_p      -- destination mac address
 *             src_mac_p      -- source mac address
 *             vlan_entry_p      -- the main purpose is to get the egress port list.
 * OUTPUT: none
 * RETURN: TRUE/FALSE
 * NOTES:  called by DHCPV6SNP  which is in dhcpsnp_engine.c
 */
BOOL_T IML_PMGR_SendPktByVlanEntry(L_MM_Mref_Handle_T *mref_handle_p, UI32_T packet_length, 
      UI32_T vid,UI8_T *dst_mac_p, UI8_T *src_mac_p, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry_p)
{

    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(IML_MGR_GET_MSGBUFSIZE(IML_MGR_IPCMsg_SendBootPPacketByDhcpSnp_Data_S))];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    IML_MGR_IPCMsg_T    *data_p;

    msgbuf_p->cmd = SYS_MODULE_IML;

    data_p = (IML_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = IML_MGR_IPC_SEND_PACKET_BY_VLAN_ENTRY;
    data_p->data.SendBootPPacketByDhcpSnp_data.mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    data_p->data.SendBootPPacketByDhcpSnp_data.vid                = vid;
    data_p->data.SendBootPPacketByDhcpSnp_data.packet_length      = packet_length;
    memcpy(data_p->data.SendBootPPacketByDhcpSnp_data.dst_mac,dst_mac_p,SYS_ADPT_MAC_ADDR_LEN);
    memcpy(data_p->data.SendBootPPacketByDhcpSnp_data.src_mac,src_mac_p,SYS_ADPT_MAC_ADDR_LEN);
    memcpy(&(data_p->data.SendBootPPacketByDhcpSnp_data.dot1q_entry),vlan_entry_p,sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
            
    msgbuf_p->msg_size = IML_MGR_GET_MSGBUFSIZE(IML_MGR_IPCMsg_SendBootPPacketByDhcpSnp_Data_S);

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, IML_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return data_p->type.result;

}
#endif

void IML_PMGR_GetManagementVid(UI32_T *vid_ifindex_p)
{
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(IML_MGR_GET_MSGBUFSIZE(IML_MGR_IPCMsg_ManagementVid_Data_S))];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    IML_MGR_IPCMsg_T    *data_p;
    UI32_T              msg_size;

    msg_size = IML_MGR_GET_MSGBUFSIZE(IML_MGR_IPCMsg_ManagementVid_Data_S);
    msgbuf_p->cmd = SYS_MODULE_IML;

    data_p = (IML_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = IML_MGR_IPC_GET_MANAGEMENT_VID;
    data_p->data.ManagementVid_data.vid_ifindex = *vid_ifindex_p;
    *vid_ifindex_p = 0; /* clear input vid */

    msgbuf_p->msg_size = msg_size;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return;
    }
    *vid_ifindex_p = data_p->data.ManagementVid_data.vid_ifindex;

    return;
}

UI32_T IML_PMGR_SetManagementVid(UI32_T vid_ifIndex)
{
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(IML_MGR_GET_MSGBUFSIZE(IML_MGR_IPCMsg_ManagementVid_Data_S))];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    IML_MGR_IPCMsg_T    *data_p;

    msgbuf_p->cmd = SYS_MODULE_IML;

    data_p = (IML_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = IML_MGR_IPC_SET_MANAGEMENT_VID;
    data_p->data.ManagementVid_data.vid_ifindex = vid_ifIndex;

    msgbuf_p->msg_size = IML_MGR_GET_MSGBUFSIZE(IML_MGR_IPCMsg_ManagementVid_Data_S);
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, IML_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return -1;
    }

    return data_p->type.result;
}
#if (SYS_CPNT_DAI == TRUE)
/* FUNCTION NAME: IML_PMGR_ARP_Inspection_SendPkt
 * PURPOSE: 
 *      Send packet.
 * INPUT:  
 *         *mem_ref -- L_MREF descriptor
 *         vid -- the vlan id
 *         packet_length -- packet length
 *         *dst_mac --
 *         *src_mac -- 
 *         packet_type --
 * OUTPUT: 
 *      None.
 * RETURN: 
 *      successful (0), failed (-1)
 * NOTES:
 *      None.   
 */
UI32_T IML_PMGR_ARP_Inspection_SendPkt(L_MM_Mref_Handle_T *mref_handle_p, UI32_T vid, UI32_T packet_length, 
                                        UI8_T *dst_mac, UI8_T *src_mac,UI16_T packet_type,UI32_T src_lport_ifindex)
{

    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(IML_MGR_GET_MSGBUFSIZE(IML_MGR_IPCMsg_ArpInspection_Data_S))];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    IML_MGR_IPCMsg_T    *data_p;

    msgbuf_p->cmd = SYS_MODULE_IML;

    data_p = (IML_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = IML_MGR_IPC_SEND_ARP_INSPECTION_PACKET;
    data_p->data.ArpInspection_data.mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    data_p->data.ArpInspection_data.vid                = vid;
    data_p->data.ArpInspection_data.packet_length      = packet_length;
    data_p->data.ArpInspection_data.packet_type        = packet_type;
    data_p->data.ArpInspection_data.src_lport_ifindex  = src_lport_ifindex;
    memcpy(data_p->data.ArpInspection_data.dst_mac,dst_mac,SYS_ADPT_MAC_ADDR_LEN);
    memcpy(data_p->data.ArpInspection_data.src_mac,src_mac,SYS_ADPT_MAC_ADDR_LEN);
            
    msgbuf_p->msg_size = IML_MGR_GET_MSGBUFSIZE(IML_MGR_IPCMsg_ArpInspection_Data_S);

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, IML_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return -1;
    }
    return data_p->type.result;
}
#endif


#if (SYS_CPNT_WEBAUTH == TRUE)
/*--------------------------------------------------------------------------
 * FUNCTION NAME - IML_PMGR_GetOrgDipAndIngLportBySrcIpAndSrcTcpPort
 *--------------------------------------------------------------------------
 * PURPOSE  : To get original dip and ingress lport
 *              by specified src ip/tcp port.
 * INPUT    : src_ip, src_tcpport
 * OUTPUT   : org_dip_p, lport_p
 * RETURN   : TRUE/FALSE
 * NOTES    : 1. all args are in network order
 *            2. for webauth
 *--------------------------------------------------------------------------*/
BOOL_T IML_PMGR_GetOrgDipAndIngLportBySrcIpAndSrcTcpPort(
    UI32_T  src_ip,
    UI16_T  src_tcpport,
    UI32_T  *org_dip_p,
    UI32_T  *lport_p)
{
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(IML_MGR_GET_MSGBUFSIZE(IML_MGR_IPCMsg_WebauthClient_Data_S))];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    IML_MGR_IPCMsg_T    *data_p;
    UI32_T              msg_size;

    msgbuf_p->cmd = SYS_MODULE_IML;
    msg_size = IML_MGR_GET_MSGBUFSIZE(IML_MGR_IPCMsg_WebauthClient_Data_S);

    data_p = (IML_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = IML_MGR_IPC_GETORGDIPNLOPORTBYSIPSTCPPORT;
    data_p->data.WebauthClient_data.src_ip      = src_ip;
    data_p->data.WebauthClient_data.src_tcpport = src_tcpport;

    msgbuf_p->msg_size = msg_size;
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    if (TRUE == data_p->type.result_bool)
    {
        *org_dip_p = data_p->data.WebauthClient_data.org_dip;
        *lport_p = data_p->data.WebauthClient_data.lport;
    }

    return data_p->type.result_bool;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - IML_PMGR_SetWebauthStatus
 *--------------------------------------------------------------------------
 * PURPOSE  : To set webauth status by specified logical port.
 * INPUT    : lport (1-based), status
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : 1. for webauth
 *--------------------------------------------------------------------------*/
BOOL_T IML_PMGR_SetWebauthStatus(
    UI32_T  lport,
    BOOL_T  status)
{
    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(IML_MGR_GET_MSGBUFSIZE(IML_MGR_IPCMsg_WebauthStatus_Data_S))];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    IML_MGR_IPCMsg_T    *data_p;

    msgbuf_p->cmd = SYS_MODULE_IML;

    data_p = (IML_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = IML_MGR_IPC_SETWEBAUTHSTATUS;
    data_p->data.WebauthStatus_data.lport  = lport;
    data_p->data.WebauthStatus_data.status = status;

    msgbuf_p->msg_size = IML_MGR_GET_MSGBUFSIZE(IML_MGR_IPCMsg_WebauthStatus_Data_S);
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, IML_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return data_p->type.result_bool;
}

#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
/* FUNCTION NAME: IML_PMGR_BroadcastBootPPkt_ExceptInport
 * PURPOSE: Send broadcast packet
 * INPUT:  *mem_ref_handle_p -- L_MREF descriptor
 *         vid_ifIndex       -- vlan ifIndex.
 *         packet_length     -- packet length
 *         *dst_mac_p        -- destination mac
 *         *src_mac_p        -- source mac
 *         src_lport_ifindex -- broadcast packet except this port
 * OUTPUT: none
 * RETURN: successful (0), failed (-1)
 * NOTES:  
 */
UI32_T IML_PMGR_BroadcastBootPPkt_ExceptInport(L_MM_Mref_Handle_T *mref_handle_p, UI32_T vid_ifindex,
    UI32_T packet_length, UI8_T *dst_mac_p, UI8_T *src_mac_p,UI32_T src_lport_ifindex)
{
    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(IML_MGR_GET_MSGBUFSIZE(IML_MGR_IPCMsg_Dhcp_Relay_Option82_Broadcast_Except_Inport_Data_S))];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    IML_MGR_IPCMsg_T    *data_p;

    msgbuf_p->cmd = SYS_MODULE_IML;

    data_p = (IML_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = IML_MGR_IPC_BROADCAST_EXCEPT_IN_PORT;
    data_p->data.BroadcastExceptInport_data.mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    data_p->data.BroadcastExceptInport_data.vid_ifindex = vid_ifindex;
    data_p->data.BroadcastExceptInport_data.packet_length = packet_length;
    data_p->data.BroadcastExceptInport_data.src_lport_ifindex = src_lport_ifindex;
    memcpy(data_p->data.BroadcastExceptInport_data.dst_mac, dst_mac_p, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(data_p->data.BroadcastExceptInport_data.src_mac, src_mac_p, SYS_ADPT_MAC_ADDR_LEN);
    
    msgbuf_p->msg_size = IML_MGR_GET_MSGBUFSIZE(IML_MGR_IPCMsg_Dhcp_Relay_Option82_Broadcast_Except_Inport_Data_S);
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, IML_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return (-1);
    }

    return data_p->type.result;
}

/* FUNCTION NAME: IML_PMGR_SendPktToPort
 * PURPOSE: Send packet
 * INPUT:  *mref_handle_p   -- L_MREF descriptor
 *         packet_length    -- packet length
 *         *dst_mac_p       -- destination mac
 *         *src_mac_p       -- source mac
 *         vid              -- vid
 *         out_lport        -- output port
 *         packet_type      --
 * OUTPUT: None
 * RETURN: successful (0), failed (-1)
 * NOTES:  None
 */
UI32_T IML_PMGR_SendPktToPort(L_MM_Mref_Handle_T *mref_handle_p, UI32_T packet_length, UI8_T *dst_mac_p, UI8_T *src_mac_p, 
    UI32_T vid, UI32_T out_lport, UI16_T packet_type)
{
    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(IML_MGR_GET_MSGBUFSIZE(IML_MGR_IPCMsg_Dhcp_Relay_Option82_Send_Pkt_To_Port_Data_S))];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    IML_MGR_IPCMsg_T    *data_p;

    msgbuf_p->cmd = SYS_MODULE_IML;

    data_p = (IML_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = IML_MGR_IPC_SEND_TO_PORT;
    data_p->data.SendPktToPort_data.mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    data_p->data.SendPktToPort_data.vid = vid;
    data_p->data.SendPktToPort_data.packet_length = packet_length;
    data_p->data.SendPktToPort_data.packet_type = packet_type;
    data_p->data.SendPktToPort_data.out_lport = out_lport;
    memcpy(data_p->data.SendPktToPort_data.dst_mac, dst_mac_p, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(data_p->data.SendPktToPort_data.src_mac, src_mac_p, SYS_ADPT_MAC_ADDR_LEN);
    
    msgbuf_p->msg_size = IML_MGR_GET_MSGBUFSIZE(IML_MGR_IPCMsg_Dhcp_Relay_Option82_Send_Pkt_To_Port_Data_S);
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, IML_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return (-1);
    }

    return data_p->type.result;
}
#endif

