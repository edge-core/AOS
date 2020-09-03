/* MODULE NAME:  l2mux_pmgr.c
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
#include "l2mux_mgr.h"
#include "l2mux_pmgr.h"
#include "l2mux_type.h"
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
/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MUX_PMGR_InitiateProcessResource
 *------------------------------------------------------------------------------
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
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T L2MUX_PMGR_InitiateProcessResource(void)
{
    /* Given that L2MUX PMGR requests are handled in l2mux group of L2_L4_PROC
     */
    if(SYSFUN_GetMsgQ(SYS_BLD_L2MUX_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}


void L2MUX_PMGR_SendPacketPipeline(L_MM_Mref_Handle_T *mref_handle_p,
                          UI32_T      packet_length,
                          UI32_T      in_port)
{
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(L2MUX_MGR_GET_MSGBUFSIZE(L2MUX_MGR_IPCMsg_SendPacket_Data_S))];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    L2MUX_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_L2MUX;

    data_p = (L2MUX_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = L2MUX_MGR_IPC_SEND_PACKET_PIPELINE;
    data_p->data.SendPacket_data.mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    data_p->data.SendPacket_data.packet_length = packet_length;
    data_p->data.SendPacket_data.lport         = in_port;

    msgbuf_p->msg_size = L2MUX_MGR_GET_MSGBUFSIZE(L2MUX_MGR_IPCMsg_SendPacket_Data_S);

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, 0, NULL)!=SYSFUN_OK)
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    return;
}

void L2MUX_PMGR_SendBPDU(L_MM_Mref_Handle_T *mref_handle_p,
                          UI8_T       dst_mac[6],
                          UI8_T       src_mac[6],
                          UI16_T      type,
                          UI16_T      tag_info,
                          UI32_T      packet_length,
                          UI32_T      lport,           
                          BOOL_T      is_tagged, 
                          UI32_T      cos_value,
                          BOOL_T      is_send_to_trunk_members)
{
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(L2MUX_MGR_GET_MSGBUFSIZE(L2MUX_MGR_IPCMsg_SendPacket_Data_S))];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    L2MUX_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_L2MUX;

    data_p = (L2MUX_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = L2MUX_MGR_IPC_SEND_BPDU;
    data_p->data.SendPacket_data.mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    data_p->data.SendPacket_data.packet_length = packet_length;
    data_p->data.SendPacket_data.lport         = lport;
    data_p->data.SendPacket_data.cos_value     = cos_value;
    data_p->data.SendPacket_data.type          = type;
    data_p->data.SendPacket_data.tag_info      = tag_info;
    memcpy(data_p->data.SendPacket_data.dst_mac,dst_mac,6);
    memcpy(data_p->data.SendPacket_data.src_mac,src_mac,6);
    data_p->data.SendPacket_data.is_tagged     = is_tagged;
    data_p->data.SendPacket_data.is_send_to_trunk_members = is_send_to_trunk_members;

    msgbuf_p->msg_size = L2MUX_MGR_GET_MSGBUFSIZE(L2MUX_MGR_IPCMsg_SendPacket_Data_S);

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, 0, NULL)!=SYSFUN_OK)
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    return;
}


void L2MUX_PMGR_SendPacket(L_MM_Mref_Handle_T *mref_handle_p,
                          UI8_T       dst_mac[6],
                          UI8_T       src_mac[6],
                          UI16_T      type,
                          UI16_T      tag_info,
                          UI32_T      packet_length,
                          UI32_T      lport,           
                          BOOL_T      is_tagged, 
                          UI32_T      cos_value,
                          BOOL_T      is_send_to_trunk_members)
{
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(L2MUX_MGR_GET_MSGBUFSIZE(L2MUX_MGR_IPCMsg_SendPacket_Data_S))];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    L2MUX_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_L2MUX;

    data_p = (L2MUX_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = L2MUX_MGR_IPC_SEND_PACKET;
    data_p->data.SendPacket_data.mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    data_p->data.SendPacket_data.packet_length = packet_length;
    data_p->data.SendPacket_data.lport         = lport;
    data_p->data.SendPacket_data.cos_value     = cos_value;
    data_p->data.SendPacket_data.type          = type;
    data_p->data.SendPacket_data.tag_info      = tag_info;
    memcpy(data_p->data.SendPacket_data.dst_mac,dst_mac,6);
    memcpy(data_p->data.SendPacket_data.src_mac,src_mac,6);
    data_p->data.SendPacket_data.is_tagged     = is_tagged;
    data_p->data.SendPacket_data.is_send_to_trunk_members = is_send_to_trunk_members;

    msgbuf_p->msg_size = L2MUX_MGR_GET_MSGBUFSIZE(L2MUX_MGR_IPCMsg_SendPacket_Data_S);

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, 0, NULL)!=SYSFUN_OK)
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    return;
}

void L2MUX_PMGR_SendMultiPacket(L_MM_Mref_Handle_T *mref_handle_p,
                                UI8_T             dst_mac[6], 
                                UI8_T             src_mac[6],
                                UI16_T            type,        
                                UI16_T            tag_info,    
                                UI32_T            packet_length,
                                UI8_T             *lport_list, 
                                UI8_T             *untagged_list,
                                UI32_T            cos_value)
{
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(L2MUX_MGR_GET_MSGBUFSIZE(L2MUX_MGR_IPCMsg_SendMulti_Data_S))];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    L2MUX_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_L2MUX;

    data_p = (L2MUX_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = L2MUX_MGR_IPC_SEND_MULTI_PACKET;
    data_p->data.SendMulti_data.mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    data_p->data.SendMulti_data.packet_length = packet_length;
    data_p->data.SendMulti_data.cos_value     = cos_value;
    data_p->data.SendMulti_data.type          = type;
    data_p->data.SendMulti_data.tag_info      = tag_info;
    memcpy(data_p->data.SendMulti_data.dst_mac,dst_mac,6);
    memcpy(data_p->data.SendMulti_data.src_mac,src_mac,6);
    memcpy(data_p->data.SendMulti_data.lport_list,lport_list,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    memcpy(data_p->data.SendMulti_data.untagged_list,untagged_list,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    msgbuf_p->msg_size = L2MUX_MGR_GET_MSGBUFSIZE(L2MUX_MGR_IPCMsg_SendMulti_Data_S);

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, 0, NULL)!=SYSFUN_OK)
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    return;
}

void L2MUX_PMGR_SendMultiPacketByVlan(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI8_T dst_mac[6],
    UI8_T src_mac[6],
    UI16_T type,
    UI16_T tag_info,
    UI32_T packet_length,
    UI8_T *lport_list,
    UI32_T cos_value)
{
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(L2MUX_MGR_GET_MSGBUFSIZE(L2MUX_MGR_IPCMsg_SendMulti_Data_S))];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    L2MUX_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_L2MUX;

    data_p = (L2MUX_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = L2MUX_MGR_IPC_SEND_MULTI_PACKET_BY_VLAN;
    data_p->data.SendMulti_data.mref_handle_offset = L_IPCMEM_GetOffset(mref_handle_p);
    data_p->data.SendMulti_data.packet_length = packet_length;
    data_p->data.SendMulti_data.cos_value     = cos_value;
    data_p->data.SendMulti_data.type          = type;
    data_p->data.SendMulti_data.tag_info      = tag_info;
    memcpy(data_p->data.SendMulti_data.dst_mac,dst_mac,6);
    memcpy(data_p->data.SendMulti_data.src_mac,src_mac,6);

    if (lport_list)
    {
        memcpy(data_p->data.SendMulti_data.lport_list,lport_list,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
        data_p->data.SendMulti_data.lport_list_is_valid = TRUE;
    }
    else
    {
        data_p->data.SendMulti_data.lport_list_is_valid = FALSE;
    }

    msgbuf_p->msg_size = L2MUX_MGR_GET_MSGBUFSIZE(L2MUX_MGR_IPCMsg_SendMulti_Data_S);

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, 0, NULL)!=SYSFUN_OK)
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    return;
}

/* LOCAL SUBPROGRAM BODIES
 */

