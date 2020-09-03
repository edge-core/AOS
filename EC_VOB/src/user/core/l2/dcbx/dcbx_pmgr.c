/*-----------------------------------------------------------------------------
 * FILE NAME: DCBX_PMGR.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file implements the APIs for DCBX MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    9/20/2012 - Ricky Lin, Created
 *
 * Copyright(C)      Accton Corporation, 2012
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>   /*to remove warning, because the memcpy function can't be recognized,so here we add string.h and stdio.h head files*/
#include <string.h>  /*to remove warning, because the memcpy function can't be recognized,so here we add string.h and stdio.h head files*/
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sysfun.h"
#include "dcbx_mgr.h"
#include "dcbx_pmgr.h"
#include "dcbx_type.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DEFINITIONS
 */

static SYSFUN_MsgQ_T ipcmsgq_handle;


/* EXPORTED SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : DCBX_PMGR_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resources for DCBX_PMGR in the calling process.
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
BOOL_T DCBX_PMGR_InitiateProcessResources(void)
{
    /* get the handle of ipc message queues for DCBX MGR
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_DCB_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\n%s(): SYSFUN_GetMsgQ failed.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
} /* End of DCBX_PMGR_InitiateProcessResources */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_PMGR_SetPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  :  Set port_status to control DCBX is enabled
 *             or is disabled.
 * INPUT    : UI32_T lport            -- lport number
 *            BOOL_T port_status      -- Enable(TRUE)
 *                                       Disable(FALSE)
 * OUTPUT   : None
 * RETURN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 38.4, IEEE Std P802.1Qaz/D2.5.
 *            Default value: Enable(TRUE).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_PMGR_SetPortStatus(UI32_T lport, BOOL_T port_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = DCBX_MGR_GET_MSG_SIZE(arg_grp_ui32_bool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DCBX_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_DCBX;
    msgbuf_p->msg_size = msg_size;

    msg_p = (DCBX_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DCBX_MGR_IPC_SETPORTSTATUS;
    msg_p->data.arg_grp_ui32_bool.arg_ui32 = lport;
    msg_p->data.arg_grp_ui32_bool.arg_bool = port_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,/* pgr0695 */
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DCBX_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_PMGR_GetPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  :  Get port_status
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : port_status_p
 * RETURN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 38.4, IEEE Std P802.1Qaz/D2.5.
 *            Default value: Enable(TRUE).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_PMGR_GetPortStatus(UI32_T lport, BOOL_T *port_status_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = DCBX_MGR_GET_MSG_SIZE(arg_grp_ui32_bool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DCBX_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_DCBX;
    msgbuf_p->msg_size = msg_size;

    msg_p = (DCBX_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DCBX_MGR_IPC_GETPORTSTATUS;
    msg_p->data.arg_grp_ui32_bool.arg_ui32 = lport;
    msg_p->data.arg_grp_ui32_bool.arg_bool = *port_status_p;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,/* pgr0695 */
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DCBX_TYPE_RETURN_ERROR;
    }

    *port_status_p = msg_p->data.arg_grp_ui32_bool.arg_bool;
    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_PMGR_GetRunningPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  :  Get port_status
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : port_status_p
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : Ref to the description in 38.4, IEEE Std P802.1Qaz/D2.5.
 *            Default value: Enable(TRUE).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_PMGR_GetRunningPortStatus(UI32_T lport, BOOL_T *port_status_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = DCBX_MGR_GET_MSG_SIZE(arg_grp_ui32_bool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DCBX_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_DCBX;
    msgbuf_p->msg_size = msg_size;

    msg_p = (DCBX_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DCBX_MGR_IPC_GETRUNNINGPORTSTATUS;
    msg_p->data.arg_grp_ui32_bool.arg_ui32 = lport;
    msg_p->data.arg_grp_ui32_bool.arg_bool = *port_status_p;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,/* pgr0695 */
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *port_status_p = msg_p->data.arg_grp_ui32_bool.arg_bool;
    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_PMGR_SetPortMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set port mode
 * INPUT    : UI32_T lport            -- lport number
 *            UI32_T port_mode        -- manual(1),
 *                                       configuration_source(2),
 *                                       auto-upstream(3),
 *                                       auto-downstream(4)
 * OUTPUT   : None
 * RETURN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTE     : Default value: manual(1).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_PMGR_SetPortMode(UI32_T lport, UI32_T port_mode)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = DCBX_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DCBX_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_DCBX;
    msgbuf_p->msg_size = msg_size;

    msg_p = (DCBX_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DCBX_MGR_IPC_SETPORTMODE;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = lport;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2 = port_mode;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,/* pgr0695 */
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DCBX_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_PMGR_GetPortMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port mode
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : port_mode_p -- manual(1),
 *                                                  configuration_source(2),
 *                                                  auto-upstream(3),
 *                                                  auto-downstream(4)
 * RETURN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTE     : Default value: manual(1).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_PMGR_GetPortMode(UI32_T lport, UI32_T *port_mode_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = DCBX_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DCBX_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_DCBX;
    msgbuf_p->msg_size = msg_size;

    msg_p = (DCBX_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DCBX_MGR_IPC_GETPORTMODE;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = lport;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2 = *port_mode_p;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,/* pgr0695 */
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return DCBX_TYPE_RETURN_ERROR;
    }

    *port_mode_p = msg_p->data.arg_grp_ui32_ui32.arg_ui32_2;
    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_PMGR_GetRunningPortMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port mode
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : port_mode_p -- manual(1),
 *                                                  configuration_source(2),
 *                                                  auto-upstream(3),
 *                                                  auto-downstream(4)
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : Default value: manual(1).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_PMGR_GetRunningPortMode(UI32_T lport, UI32_T *port_mode_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = DCBX_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    DCBX_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_DCBX;
    msgbuf_p->msg_size = msg_size;

    msg_p = (DCBX_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = DCBX_MGR_IPC_GETRUNNINGPORTMODE;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = lport;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2 = *port_mode_p;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,/* pgr0695 */
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *port_mode_p = msg_p->data.arg_grp_ui32_ui32.arg_ui32_2;
    return msg_p->type.ret_ui32;
}
