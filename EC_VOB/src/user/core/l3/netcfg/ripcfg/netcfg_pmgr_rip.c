/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_PMGR_RIP.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to access NETCFG_MGR_RIP and NETCFG_OM_RIP service.
 *    In Linux platform, the communication between CSC group are done via IPC.
 *    Other CSC can call NETCFG_PMGR_XXX for APIs NETCFG_MGR_XXX provided by NETCFG
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

/* INCLUDE FILE DECLARATIONS
 */

#include <string.h>
#include <sys/types.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "l_mm.h"
#include "sysfun.h"
#include "netcfg_type.h"
#include "netcfg_mgr_rip.h"
#include "netcfg_pmgr_rip.h"

static SYSFUN_MsgQ_T ipcmsgq_handle;

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_RIP_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for NETCFG_PMGR_RIP in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  --  Sucess
 *           FALSE --  Error
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T NETCFG_PMGR_RIP_InitiateProcessResource(void)/*init in netcfg_main*/
{
    if (SYSFUN_GetMsgQ(SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_ConfigDebug
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
BOOL_T NETCFG_PMGR_RIP_ConfigDebug(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_CONFIGDEBUG;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_Debug
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
BOOL_T NETCFG_PMGR_RIP_Debug(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_DEBUG;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_ConfigUnDebug
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
BOOL_T NETCFG_PMGR_RIP_ConfigUnDebug(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_CONFIGUNDEBUG;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_UnDebug
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
BOOL_T NETCFG_PMGR_RIP_UnDebug(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_UNDEBUG;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_ConfigDebugEvent
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
BOOL_T NETCFG_PMGR_RIP_ConfigDebugEvent(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_CONFIGDEBUGEVENT;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_ConfigUnDebugEvent
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
BOOL_T NETCFG_PMGR_RIP_ConfigUnDebugEvent(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_CONFIGUNDEBUGEVENT;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_DebugEvent
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
BOOL_T NETCFG_PMGR_RIP_DebugEvent(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_DEBUGEVENT;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_UnDebugEvent
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
BOOL_T NETCFG_PMGR_RIP_UnDebugEvent(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_UNDEBUGEVENT;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_DebugPacket
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
BOOL_T NETCFG_PMGR_RIP_DebugPacket(NETCFG_TYPE_RIP_Packet_Debug_Type_T type)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(pdebug_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_DEBUGPACKET;
    msg_p->data.pdebug_v= type;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_ConfigDebugPacket
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
BOOL_T NETCFG_PMGR_RIP_ConfigDebugPacket(NETCFG_TYPE_RIP_Packet_Debug_Type_T type)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(pdebug_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_CONFIGDEBUGPACKET;
    msg_p->data.pdebug_v= type;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_UnDebugPacket
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
BOOL_T NETCFG_PMGR_RIP_UnDebugPacket(NETCFG_TYPE_RIP_Packet_Debug_Type_T type)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(pdebug_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_UNDEBUGPACKET;
    msg_p->data.pdebug_v= type;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_ConfigUnDebugPacket
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
BOOL_T NETCFG_PMGR_RIP_ConfigUnDebugPacket(NETCFG_TYPE_RIP_Packet_Debug_Type_T type)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(pdebug_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_CONFIGUNDEBUGPACKET;
    msg_p->data.pdebug_v= type;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_ConfigDebugNsm
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
BOOL_T NETCFG_PMGR_RIP_ConfigDebugNsm(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_CONFIGDEBUGNSM;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_ConfigUnDebugNsm
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
BOOL_T NETCFG_PMGR_RIP_ConfigUnDebugNsm(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_CONFIGUNDEBUGNSM;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_DebugNsm
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
BOOL_T NETCFG_PMGR_RIP_DebugNsm(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_DEBUGNSM;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_UnDebugNsm
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
BOOL_T NETCFG_PMGR_RIP_UnDebugNsm(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_UNDEBUGNSM;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetDebugStatus
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
BOOL_T NETCFG_PMGR_RIP_GetDebugStatus(NETCFG_TYPE_RIP_Debug_Status_T *status)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(status_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETDEBUGSTATUS;
    memset(&(msg_p->data.status_v),0,sizeof(msg_p->data.status_v));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(status, &(msg_p->data.status_v), sizeof(msg_p->data.status_v));
    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_RecvPacketSet
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
UI32_T NETCFG_PMGR_RIP_RecvPacketSet(UI32_T ifindex)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_RECVPACKETSET;
    msg_p->data.ui32_v= ifindex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_RecvPacketUnset
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
UI32_T NETCFG_PMGR_RIP_RecvPacketUnset(UI32_T ifindex)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_RECVPACKETUNSET;
    msg_p->data.ui32_v= ifindex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_SendPacketSet
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
UI32_T NETCFG_PMGR_RIP_SendPacketSet(UI32_T ifindex)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_SENDPACKETSET;
    msg_p->data.ui32_v= ifindex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_SendPacketUnset
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
UI32_T NETCFG_PMGR_RIP_SendPacketUnset(UI32_T ifindex)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_SENDPACKETUNSET;
    msg_p->data.ui32_v= ifindex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_RecvVersionTypeSet
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
UI32_T NETCFG_PMGR_RIP_RecvVersionTypeSet(UI32_T ifindex, enum NETCFG_TYPE_RIP_Version_E type)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_RECVVERSIONTYPESET;
    msg_p->data.arg_grp6.arg1= ifindex;
    msg_p->data.arg_grp6.arg2= type;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_RecvVersionUnset
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
UI32_T NETCFG_PMGR_RIP_RecvVersionUnset(UI32_T ifindex)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_RECVVERSIONUNSET;
    msg_p->data.ui32_v= ifindex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_SendVersionTypeSet
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
UI32_T NETCFG_PMGR_RIP_SendVersionTypeSet(UI32_T ifindex, enum NETCFG_TYPE_RIP_Version_E type)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_SENDVERSIONTYPESET;
    msg_p->data.arg_grp6.arg1= ifindex;
    msg_p->data.arg_grp6.arg2= type;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_SendVersionUnset
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
UI32_T NETCFG_PMGR_RIP_SendVersionUnset(UI32_T ifindex)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_SENDVERSIONUNSET;
    msg_p->data.ui32_v= ifindex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_AuthModeSet
* PURPOSE:
*     Set RIP authentication mode.
*
* INPUT:
*      ifindex,
*      mode:
*        text----- 1
*        md5------2
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
UI32_T NETCFG_PMGR_RIP_AuthModeSet(UI32_T ifindex, enum NETCFG_TYPE_RIP_Auth_Mode_E mode)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp7);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_AUTHMODESET;
    msg_p->data.arg_grp7.arg1= ifindex;
    msg_p->data.arg_grp7.arg2= mode;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_AuthModeUnset
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
UI32_T NETCFG_PMGR_RIP_AuthModeUnset(UI32_T ifindex)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_AUTHMODEUNSET;
    msg_p->data.ui32_v= ifindex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_AuthStringSet
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
*     If the string is null string, please input "".
*/
UI32_T NETCFG_PMGR_RIP_AuthStringSet(UI32_T ifindex, char *str)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    if(str == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_AUTHSTRINGSET;
    msg_p->data.arg_grp2.arg1= ifindex;
    memset(&(msg_p->data.arg_grp2.arg2), 0, sizeof(msg_p->data.arg_grp2.arg2));
    strncpy((char *)&(msg_p->data.arg_grp2.arg2), str, strlen(str));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_AuthStringUnset
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
UI32_T NETCFG_PMGR_RIP_AuthStringUnset(UI32_T ifindex)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_AUTHSTRINGUNSET;
    msg_p->data.ui32_v= ifindex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_AuthKeyChainSet
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
*     If the string is null string, please input "".
*/
UI32_T NETCFG_PMGR_RIP_AuthKeyChainSet(UI32_T ifindex, char *str)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    if(str == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_AUTHKEYCHAINSET;
    msg_p->data.arg_grp2.arg1= ifindex;
    memset(&(msg_p->data.arg_grp2.arg2), 0, sizeof(msg_p->data.arg_grp2.arg2));
    memcpy(&(msg_p->data.arg_grp2.arg2), str, strlen(str));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_AuthKeyChainUnset
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
UI32_T NETCFG_PMGR_RIP_AuthKeyChainUnset(UI32_T ifindex)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_AUTHKEYCHAINUNSET;
    msg_p->data.ui32_v= ifindex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_SplitHorizonSet
* PURPOSE:
*     Set RIP split horizon.
*
* INPUT:
*      ifindex,
*      mode:
*        split horizon----- 1
*        split horizon poisoned------2
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
UI32_T NETCFG_PMGR_RIP_SplitHorizonSet(UI32_T ifindex, enum NETCFG_TYPE_RIP_Split_Horizon_E type)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp7);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_SPLITHORIZONSET;
    msg_p->data.arg_grp7.arg1= ifindex;
    msg_p->data.arg_grp7.arg2= type;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_SplitHorizonUnset
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
UI32_T NETCFG_PMGR_RIP_SplitHorizonUnset(UI32_T ifindex)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_SPLITHORIZONUNSET;
    msg_p->data.ui32_v= ifindex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_RouterRipSet
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
UI32_T NETCFG_PMGR_RIP_RouterRipSet(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_ROUTERRIPSET;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_RouterRipUnset
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
UI32_T NETCFG_PMGR_RIP_RouterRipUnset(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_ROUTERRIPUNSET;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_VersionSet
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
UI32_T NETCFG_PMGR_RIP_VersionSet(enum NETCFG_TYPE_RIP_Global_Version_E version)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(version_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_VERSIONSET;
    msg_p->data.version_v= version;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_VersionUnset
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
UI32_T NETCFG_PMGR_RIP_VersionUnset(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_VERSIONUNSET;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_NetworkSetByVid
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
UI32_T NETCFG_PMGR_RIP_NetworkSetByVid(UI32_T vid)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
   

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_NETWORKSETBYVID;
    msg_p->data.ui32_v = vid;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_NetworkSetByAddress
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
UI32_T NETCFG_PMGR_RIP_NetworkSetByAddress(L_PREFIX_T network_address)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp9);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
   

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_NETWORKSETBYADDRESS;
    memcpy(&(msg_p->data.arg_grp9.arg1), &network_address, sizeof(L_PREFIX_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

    }

/* FUNCTION NAME : NETCFG_PMGR_RIP_NetworkUnsetByVid
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
UI32_T NETCFG_PMGR_RIP_NetworkUnsetByVid(UI32_T vid)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;


    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_NETWORKUNSETBYVID;
    msg_p->data.ui32_v = vid;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}


/* FUNCTION NAME : NETCFG_PMGR_RIP_NetworkUnsetByAddress
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
UI32_T NETCFG_PMGR_RIP_NetworkUnsetByAddress(L_PREFIX_T network_address)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp9);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;


    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_NETWORKUNSETBYADDRESS;
    memcpy(&(msg_p->data.arg_grp9.arg1), &network_address, sizeof(L_PREFIX_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_NeighborSet
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
UI32_T NETCFG_PMGR_RIP_NeighborSet(UI32_T ip_addr)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_NEIGHBORSET;
    msg_p->data.ui32_v = ip_addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_NeighborUnset
* PURPOSE:
*     unset rip neighbor
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
UI32_T NETCFG_PMGR_RIP_NeighborUnset(UI32_T ip_addr)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_NEIGHBORUNSET;
    msg_p->data.ui32_v = ip_addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_PassiveIfAdd
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
UI32_T NETCFG_PMGR_RIP_PassiveIfAdd(UI32_T vid)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
  

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_PASSIVEIFADD;
    msg_p->data.ui32_v = vid;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_PassiveIfDelete
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
UI32_T NETCFG_PMGR_RIP_PassiveIfDelete(UI32_T vid)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_PASSIVEIFDELETE;
    msg_p->data.ui32_v = vid;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_DefaultAdd
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
UI32_T NETCFG_PMGR_RIP_DefaultAdd(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_DEFAULTADD;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_DefaultDelete
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
UI32_T NETCFG_PMGR_RIP_DefaultDelete(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_DEFAULTDELETE;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_DefaultMetricSet
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
UI32_T NETCFG_PMGR_RIP_DefaultMetricSet(UI32_T metric)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_DEFAULTMETRICSET;
    msg_p->data.ui32_v= metric;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_DefaultMetricUnset
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
UI32_T NETCFG_PMGR_RIP_DefaultMetricUnset(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_DEFAULTMETRICUNSET;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_DistributeListAdd
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
UI32_T NETCFG_PMGR_RIP_DistributeListAdd(char *ifname, char *list_name,
                                                      enum NETCFG_TYPE_RIP_Distribute_Type_E type,
                                                      enum NETCFG_TYPE_RIP_Distribute_List_Type_E list_type)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    
    if(list_name == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    if((type < NETCFG_TYPE_RIP_DISTRIBUTE_IN) || (type > NETCFG_TYPE_RIP_DISTRIBUTE_MAX)||
        (list_type < NETCFG_TYPE_RIP_DISTRIBUTE_ACCESS_LIST) || (list_type > NETCFG_TYPE_RIP_DISTRIBUTE_PREFIX_LIST))
        return NETCFG_TYPE_INVALID_ARG;
    
    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_DISTRIBUTELISTADD;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    if(ifname != NULL)
        memcpy(&(msg_p->data.arg_grp3.arg1), ifname, strlen(ifname));
    memcpy(&(msg_p->data.arg_grp3.arg2), list_name, strlen(list_name));
    msg_p->data.arg_grp3.arg3 = type;
    msg_p->data.arg_grp3.arg4 = list_type;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_DistributeListDelete
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
UI32_T NETCFG_PMGR_RIP_DistributeListDelete(char *ifname, char *list_name,
                                                         enum NETCFG_TYPE_RIP_Distribute_Type_E type,
                                                         enum NETCFG_TYPE_RIP_Distribute_List_Type_E list_type)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    
    if(list_name == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    if((type < NETCFG_TYPE_RIP_DISTRIBUTE_IN) || (type > NETCFG_TYPE_RIP_DISTRIBUTE_MAX)||
        (list_type < NETCFG_TYPE_RIP_DISTRIBUTE_ACCESS_LIST) || (list_type > NETCFG_TYPE_RIP_DISTRIBUTE_PREFIX_LIST))
        return NETCFG_TYPE_INVALID_ARG;
    
    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_DISTRIBUTELISTDELETE;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    if(ifname != NULL)
        memcpy(&(msg_p->data.arg_grp3.arg1), ifname, strlen(ifname));
    memcpy(&(msg_p->data.arg_grp3.arg2), list_name, strlen(list_name));
    msg_p->data.arg_grp3.arg3 = type;
    msg_p->data.arg_grp3.arg4 = list_type;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_TimerSet
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
UI32_T NETCFG_PMGR_RIP_TimerSet(NETCFG_TYPE_RIP_Timer_T *timer)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(timer_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_TIMERSET;

    memset(&(msg_p->data.timer_v), 0, sizeof(msg_p->data.timer_v));
    memcpy(&(msg_p->data.timer_v), timer, sizeof(msg_p->data.timer_v));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_TimerUnset
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
UI32_T NETCFG_PMGR_RIP_TimerUnset(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_TIMERUNSET;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_DistanceDefaultSet
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
UI32_T NETCFG_PMGR_RIP_DistanceDefaultSet(UI32_T distance)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_DISTANCEDEFAULTSET;

    msg_p->data.ui32_v = distance;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_DistanceDefaultUnset
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
UI32_T NETCFG_PMGR_RIP_DistanceDefaultUnset(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_DISTANCEDEFAULTUNSET;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_DistanceSet
* PURPOSE:
*     set distance .
*
* INPUT:
*      distance,
*      add,
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
UI32_T NETCFG_PMGR_RIP_DistanceSet(UI32_T distance, UI32_T addr, UI32_T plen, char *alist)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp4);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_DISTANCESET;
    
    memset(&(msg_p->data.arg_grp4), 0, sizeof(msg_p->data.arg_grp4));
    msg_p->data.arg_grp4.arg1 = distance;
    msg_p->data.arg_grp4.arg2 = addr;
    msg_p->data.arg_grp4.arg3 = plen;
    if(alist != NULL)
    {
        strcpy(msg_p->data.arg_grp4.arg4, alist);
    }
    else
    {
        strcpy(msg_p->data.arg_grp4.arg4, "");
    }

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_DistanceUnset
* PURPOSE:
*     unset distance .
*
* INPUT:
*      add,
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
UI32_T NETCFG_PMGR_RIP_DistanceUnset(UI32_T addr, UI32_T plen)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp4);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_DISTANCEUNSET;
    
    memset(&(msg_p->data.arg_grp4), 0, sizeof(msg_p->data.arg_grp4));
    msg_p->data.arg_grp4.arg2 = addr;
    msg_p->data.arg_grp4.arg3 = plen;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_MaxPrefixSet
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
UI32_T NETCFG_PMGR_RIP_MaxPrefixSet(UI32_T pmax)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_MAXPREFIXSET;

    msg_p->data.ui32_v = pmax;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_MaxPrefixUnset
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
UI32_T NETCFG_PMGR_RIP_MaxPrefixUnset(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_MAXPREFIXUNSET;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_RecvBuffSizeSet
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
UI32_T NETCFG_PMGR_RIP_RecvBuffSizeSet(UI32_T buff_size)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_RECVBUFFSIZESET;

    msg_p->data.ui32_v = buff_size;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_RecvBuffSizeUnset
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
UI32_T NETCFG_PMGR_RIP_RecvBuffSizeUnset(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_RECVBUFFSIZEUNSET;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_RedistributeSet
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
UI32_T NETCFG_PMGR_RIP_RedistributeSet(char *protocol)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp5);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    
    if(protocol == NULL)
        return NETCFG_TYPE_INVALID_ARG;
    
    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_REDISTRIBUTESET;

    memset(&(msg_p->data.arg_grp5), 0, sizeof(msg_p->data.arg_grp5));
    memcpy(&(msg_p->data.arg_grp5.arg1), protocol, strlen(protocol));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_RedistributeMetricSet
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
UI32_T NETCFG_PMGR_RIP_RedistributeMetricSet(char *protocol, UI32_T metric)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp5);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    
    if(protocol == NULL)
        return NETCFG_TYPE_INVALID_ARG;
    
    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_REDISTRIBUTEMETRICSET;

    memset(&(msg_p->data.arg_grp5), 0, sizeof(msg_p->data.arg_grp5));
    memcpy(&(msg_p->data.arg_grp5.arg1), protocol, strlen(protocol));
    msg_p->data.arg_grp5.arg2 = metric;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_RedistributeRmapSet
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
UI32_T NETCFG_PMGR_RIP_RedistributeRmapSet(char *protocol, char *rmap)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp5);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    
    if((protocol == NULL) || (rmap == NULL))
        return NETCFG_TYPE_INVALID_ARG;
    
    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_REDISTRIBUTERMAPSET;

    memset(&(msg_p->data.arg_grp5), 0, sizeof(msg_p->data.arg_grp5));
    memcpy(&(msg_p->data.arg_grp5.arg1), protocol, strlen(protocol));
    memcpy(&(msg_p->data.arg_grp5.arg3), rmap, strlen(rmap));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_RedistributeAllSet
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
UI32_T NETCFG_PMGR_RIP_RedistributeAllSet(char *protocol, UI32_T metric, char *rmap)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp5);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    
    if((protocol == NULL) || (rmap == NULL))
        return NETCFG_TYPE_INVALID_ARG;
    
    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_REDISTRIBUTEALLSET;

    memset(&(msg_p->data.arg_grp5), 0, sizeof(msg_p->data.arg_grp5));
    memcpy(&(msg_p->data.arg_grp5.arg1), protocol, strlen(protocol));
    msg_p->data.arg_grp5.arg2 = metric;
    memcpy(&(msg_p->data.arg_grp5.arg3), rmap, strlen(rmap));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_RedistributeUnset
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
UI32_T NETCFG_PMGR_RIP_RedistributeUnset(char *protocol)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp5);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    
    if(protocol == NULL)
        return NETCFG_TYPE_INVALID_ARG;
    
    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_REDISTRIBUTEUNSET;

    memset(&(msg_p->data.arg_grp5), 0, sizeof(msg_p->data.arg_grp5));
    memcpy(&(msg_p->data.arg_grp5.arg1), protocol, strlen(protocol));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}






/* FUNCTION NAME : NETCFG_PMGR_RIP_ClearRoute
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
UI32_T NETCFG_PMGR_RIP_ClearRoute(char *arg)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(char_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    
    if(arg == NULL)
        return NETCFG_TYPE_INVALID_ARG;
    
    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_CLEARROUTE;

    memset(&(msg_p->data.char_v), 0, sizeof(msg_p->data.char_v));
    memcpy(&(msg_p->data.char_v), arg, strlen(arg));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_ClearStatistics
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
UI32_T NETCFG_PMGR_RIP_ClearStatistics(void)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_CLEARSTATISTICS;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetInstanceEntry
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
UI32_T NETCFG_PMGR_RIP_GetInstanceEntry(NETCFG_TYPE_RIP_Instance_T *entry)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(instance_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETINSTANCE;
    memset(&(msg_p->data.instance_v), 0, sizeof(NETCFG_TYPE_RIP_Instance_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(entry, &(msg_p->data.instance_v), sizeof(NETCFG_TYPE_RIP_Instance_T));
    
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetInterfaceEntry
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
UI32_T NETCFG_PMGR_RIP_GetInterfaceEntry(NETCFG_TYPE_RIP_If_T *entry)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(if_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETINTERFACE;
    memset(&(msg_p->data.if_v), 0, sizeof(NETCFG_TYPE_RIP_If_T));
    memcpy(&(msg_p->data.if_v), entry, sizeof(NETCFG_TYPE_RIP_If_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(entry, &(msg_p->data.if_v), sizeof(NETCFG_TYPE_RIP_If_T));
    
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNextInterfaceEntry
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
UI32_T NETCFG_PMGR_RIP_GetNextInterfaceEntry(NETCFG_TYPE_RIP_If_T *entry)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(if_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    if(entry == NULL)
    {
        return NETCFG_TYPE_INVALID_ARG;
    }

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETNEXTINTERFACE;
    memset(&(msg_p->data.if_v), 0, sizeof(NETCFG_TYPE_RIP_If_T));
    memcpy(&(msg_p->data.if_v), entry, sizeof(NETCFG_TYPE_RIP_If_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(entry, &(msg_p->data.if_v), sizeof(NETCFG_TYPE_RIP_If_T));
    
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNetworkTable
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
UI32_T NETCFG_PMGR_RIP_GetNetworkTable(NETCFG_TYPE_RIP_Network_T *entry)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(prefix_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETNETWORKTABLE;
    memset(&(msg_p->data.prefix_v), 0, sizeof(NETCFG_TYPE_RIP_Network_T));
    memcpy(&(msg_p->data.prefix_v), entry, sizeof(NETCFG_TYPE_RIP_Network_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNextNetworkTable
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
UI32_T NETCFG_PMGR_RIP_GetNextNetworkTable(NETCFG_TYPE_RIP_Network_T *entry)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(prefix_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETNEXTNETWORKTABLE;
    memset(&(msg_p->data.prefix_v), 0, sizeof(NETCFG_TYPE_RIP_Network_T));
    memcpy(&(msg_p->data.prefix_v), entry, sizeof(NETCFG_TYPE_RIP_Network_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(entry, &(msg_p->data.prefix_v), sizeof(NETCFG_TYPE_RIP_Network_T));
    
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNeighborTable
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
UI32_T NETCFG_PMGR_RIP_GetNeighborTable(NETCFG_TYPE_RIP_Network_T *entry)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(prefix_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETNEIGHBOR;
    memset(&(msg_p->data.prefix_v), 0, sizeof(NETCFG_TYPE_RIP_Network_T));
    memcpy(&(msg_p->data.prefix_v), entry, sizeof(NETCFG_TYPE_RIP_Network_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNextNeighborTable
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
UI32_T NETCFG_PMGR_RIP_GetNextNeighborTable(NETCFG_TYPE_RIP_Network_T *entry)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(prefix_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    
    if(entry == NULL)
    {
        return NETCFG_TYPE_INVALID_ARG;
    }

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETNEXTNEIGHBOR;
    memset(&(msg_p->data.prefix_v), 0, sizeof(NETCFG_TYPE_RIP_Network_T));
    memcpy(&(msg_p->data.prefix_v), entry, sizeof(NETCFG_TYPE_RIP_Network_T));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    memcpy(entry, &(msg_p->data.prefix_v), sizeof(NETCFG_TYPE_RIP_Network_T));
    
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetDistanceTable
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
UI32_T NETCFG_PMGR_RIP_GetDistanceTable(NETCFG_TYPE_RIP_Distance_T *entry)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(distance_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETDISTANCETABLE;
    memset(&(msg_p->data.distance_v), 0, sizeof(NETCFG_TYPE_RIP_Distance_T));
    memcpy(&(msg_p->data.distance_v), entry, sizeof(NETCFG_TYPE_RIP_Distance_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    memcpy(entry, &(msg_p->data.distance_v), sizeof(NETCFG_TYPE_RIP_Distance_T));
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNextDistanceTable
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
UI32_T NETCFG_PMGR_RIP_GetNextDistanceTable(NETCFG_TYPE_RIP_Distance_T *entry)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(distance_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETNEXTDISTANCETABLE;
    memset(&(msg_p->data.distance_v), 0, sizeof(NETCFG_TYPE_RIP_Distance_T));
    memcpy(&(msg_p->data.distance_v), entry, sizeof(NETCFG_TYPE_RIP_Distance_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    memcpy(entry, &(msg_p->data.distance_v), sizeof(NETCFG_TYPE_RIP_Distance_T));
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNextRouteEntry
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
UI32_T NETCFG_PMGR_RIP_GetNextRouteEntry(NETCFG_TYPE_RIP_Route_T *entry)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(route_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    if(entry == NULL)
    {
        return NETCFG_TYPE_INVALID_ARG;
    }

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETNEXTRIPROUTE;
    memset(&(msg_p->data.route_v), 0, sizeof(NETCFG_TYPE_RIP_Route_T));
    memcpy(&(msg_p->data.route_v), entry, sizeof(NETCFG_TYPE_RIP_Route_T));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    memcpy(entry, &(msg_p->data.route_v), sizeof(NETCFG_TYPE_RIP_Route_T));
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNextThreadTimer
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
BOOL_T NETCFG_PMGR_RIP_GetNextThreadTimer(UI32_T *nexttime)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETNEXTTHREADTIMER;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *nexttime = msg_p->data.ui32_v;
    return msg_p->type.result_bool;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNextPeerEntry
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
BOOL_T NETCFG_PMGR_RIP_GetNextPeerEntry(NETCFG_TYPE_RIP_Peer_Entry_T *entry)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(peer_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    if(entry == NULL)
        return FALSE;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETNEXTPEERENTRY;
    memset(&(msg_p->data.peer_v), 0, sizeof(NETCFG_TYPE_RIP_Peer_Entry_T));
    memcpy(&(msg_p->data.peer_v), entry, sizeof(NETCFG_TYPE_RIP_Peer_Entry_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(entry, &(msg_p->data.peer_v), sizeof(NETCFG_TYPE_RIP_Peer_Entry_T));
    return msg_p->type.result_bool;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetPeerEntry
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
BOOL_T NETCFG_PMGR_RIP_GetPeerEntry(NETCFG_TYPE_RIP_Peer_Entry_T *entry)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(peer_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    if(entry == NULL)
        return FALSE;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETPEERENTRY;
    memset(&(msg_p->data.peer_v), 0, sizeof(NETCFG_TYPE_RIP_Peer_Entry_T));
    memcpy(&(msg_p->data.peer_v), entry, sizeof(NETCFG_TYPE_RIP_Peer_Entry_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(entry, &(msg_p->data.peer_v), sizeof(NETCFG_TYPE_RIP_Peer_Entry_T));
    return msg_p->type.result_bool;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetGlobalStatistics
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
BOOL_T NETCFG_PMGR_RIP_GetGlobalStatistics(NETCFG_TYPE_RIP_Global_Statistics_T *stat)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(global_stat_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    if(stat == NULL)
        return FALSE;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETGLOBALSTATISTICS;
    memset(&(msg_p->data.global_stat_v), 0, sizeof(NETCFG_TYPE_RIP_Global_Statistics_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(stat, &(msg_p->data.global_stat_v), sizeof(NETCFG_TYPE_RIP_Global_Statistics_T));
    return msg_p->type.result_bool;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetIfAddress
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
BOOL_T NETCFG_PMGR_RIP_GetIfAddress(UI32_T exact, UI32_T in_addr, UI32_T *out_addr)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    if(out_addr == NULL)
        return FALSE;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETIFADDRESS;
    msg_p->data.arg_grp8.arg1= exact;
    msg_p->data.arg_grp8.arg2 = in_addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *out_addr = msg_p->data.arg_grp8.arg3;
    return msg_p->type.result_bool;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetIfRecvBadPacket
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
BOOL_T NETCFG_PMGR_RIP_GetIfRecvBadPacket(UI32_T exact, UI32_T in_addr, UI32_T *value)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    if(value == NULL)
        return FALSE;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETIFRECVBADPACKET;
    msg_p->data.arg_grp8.arg1= exact;
    msg_p->data.arg_grp8.arg2 = in_addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *value = msg_p->data.arg_grp8.arg3;
    return msg_p->type.result_bool;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetIfRecvBadRoute
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
BOOL_T NETCFG_PMGR_RIP_GetIfRecvBadRoute(UI32_T exact, UI32_T in_addr, UI32_T *value)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    if(value == NULL)
        return FALSE;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETIFRECVBADROUTE;
    msg_p->data.arg_grp8.arg1= exact;
    msg_p->data.arg_grp8.arg2 = in_addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *value = msg_p->data.arg_grp8.arg3;
    return msg_p->type.result_bool;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetIfSendUpdate
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
BOOL_T NETCFG_PMGR_RIP_GetIfSendUpdate(UI32_T exact, UI32_T in_addr, UI32_T *value)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    if(value == NULL)
        return FALSE;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETIFSENDUPDATE;
    msg_p->data.arg_grp8.arg1= exact;
    msg_p->data.arg_grp8.arg2 = in_addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *value = msg_p->data.arg_grp8.arg3;
    return msg_p->type.result_bool;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetRedistributeTable
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
UI32_T NETCFG_PMGR_RIP_GetRedistributeTable(NETCFG_TYPE_RIP_Redistribute_Table_T *entry)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(redistribute_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    if(entry == NULL)
    {
        return NETCFG_TYPE_INVALID_ARG;
    }

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETREDISTRIBUTETABLE;
    memset(&(msg_p->data.redistribute_v), 0, sizeof(NETCFG_TYPE_RIP_Redistribute_Table_T));
    memcpy(&(msg_p->data.redistribute_v), entry, sizeof(NETCFG_TYPE_RIP_Redistribute_Table_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    memcpy(entry, &(msg_p->data.redistribute_v), sizeof(NETCFG_TYPE_RIP_Redistribute_Table_T));
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNextRedistributeTable
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
UI32_T NETCFG_PMGR_RIP_GetNextRedistributeTable(NETCFG_TYPE_RIP_Redistribute_Table_T *entry)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(redistribute_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;
    if(entry == NULL)
    {
        return NETCFG_TYPE_INVALID_ARG;
    }

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETNEXTREDISTRIBUTETABLE;
    memset(&(msg_p->data.redistribute_v), 0, sizeof(NETCFG_TYPE_RIP_Redistribute_Table_T));
    memcpy(&(msg_p->data.redistribute_v), entry, sizeof(NETCFG_TYPE_RIP_Redistribute_Table_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    memcpy(entry, &(msg_p->data.redistribute_v), sizeof(NETCFG_TYPE_RIP_Redistribute_Table_T));
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNextActiveRifByVlanIfIndex
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
UI32_T NETCFG_PMGR_RIP_GetNextActiveRifByVlanIfIndex(UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
    const UI32_T msg_size = NETCFG_MGR_RIP_GET_MSG_SIZE(arg_grp10);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_RIP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_RIPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_RIP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_RIP_IPC_GETNEXTACTIVERIFBYVLANIFINDEX;
    msg_p->data.arg_grp10.ui32 = ifindex;
    msg_p->data.arg_grp10.addr = *addr_p;
        
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    *addr_p = msg_p->data.arg_grp10.addr;

    return msg_p->type.result_ui32;
}
