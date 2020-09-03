/*-----------------------------------------------------------------------------
 * FILE NAME: EXTBRG_PMGR.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file implements the APIs for EXTBRG MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/07/24     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <stdio.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sysfun.h"
#include "extbrg_mgr.h"
#include "extbrg_pmgr.h"


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
 * ROUTINE NAME : EXTBRG_PMGR_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for EXTBRG_PMGR in the calling process.
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
BOOL_T EXTBRG_PMGR_InitiateProcessResource(void)
{
    /* get the ipc message queues for XSTP MGR
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
} /* End of EXTBRG_PMGR_InitiateProcessResource */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_PMGR_GetDot1dDeviceCapabilities
 *--------------------------------------------------------------------------
 * Purpose: This procedure returns the extended capability of this bridge device.
 * Input:   None.
 * Output:  bridge_device_capability - capabilities defined as a bit map
 * Return:  TRUE/FALSE.
 * Note:    1. Please refer to the naming constant declarations for detailed bit map
 *             definition of bridge_device_capability.
 *          2. When bit is set to '1', the corresponding capability is provided.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_PMGR_GetDot1dDeviceCapabilities(UI32_T *bridge_device_capability)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = EXTBRG_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    EXTBRG_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_EXTBRG;
    msgbuf_p->msg_size = EXTBRG_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (EXTBRG_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = EXTBRG_MGR_IPC_GETDOT1DDEVICECAPABILITIES;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *bridge_device_capability = msg_p->data.arg_ui32;

    return msg_p->type.ret_bool;
} /* End of EXTBRG_PMGR_GetDot1dDeviceCapabilities */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_PMGR_GetDot1dPortCapabilities
 *--------------------------------------------------------------------------
 * Purpose: This function returns the extended capability of a given bridge port.
 * Input:   lport_ifindex.
 * Output:  bridge_port_capability - capabilities defined as a bit map
 * Return:  TRUE/FALSE.
 * Note:    1. Please refer to the naming constant declarations for detailed bit map
 *             definition of bridge_port_capability.
 *          2. When bit is set to '1', the corresponding capability is provided.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_PMGR_GetDot1dPortCapabilities(UI32_T lport_ifindex, UI32_T *bridge_port_capability)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = EXTBRG_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    EXTBRG_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_EXTBRG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (EXTBRG_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = EXTBRG_MGR_IPC_GETDOT1DPORTCAPABILITIES;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = lport_ifindex;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *bridge_port_capability = msg_p->data.arg_grp_ui32_ui32.arg_ui32_2;

    return msg_p->type.ret_bool;
} /* End of EXTBRG_PMGR_GetDot1dPortCapabilities */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_PMGR_GetNextDot1dPortCapabilities
 *--------------------------------------------------------------------------
 * Purpose: This function returns the extended capability of next available bridge port.
 * Input:   lport_ifindex.
 * Output:  lport_ifindex.
 *          bridge_port_capability      -- capabilities defined as a bit map
 * Return:  TRUE/FALSE.
 * Note:    1. Please refer to the naming constant declarations for detailed bit map
 *             definition of bridge_port_capability.
 *          2. When bit is set to '1', the corresponding capability is provided.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_PMGR_GetNextDot1dPortCapabilities(UI32_T *linear_port_id, UI32_T *bridge_port_capability)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = EXTBRG_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    EXTBRG_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_EXTBRG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (EXTBRG_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = EXTBRG_MGR_IPC_GETNEXTDOT1DPORTCAPABILITIES;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = *linear_port_id;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *linear_port_id = msg_p->data.arg_grp_ui32_ui32.arg_ui32_1;
    *bridge_port_capability = msg_p->data.arg_grp_ui32_ui32.arg_ui32_2;

    return msg_p->type.ret_bool;
} /* End of EXTBRG_PMGR_GetNextDot1dPortCapabilities */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_PMGR_GetDot1dGmrpStatus
 *--------------------------------------------------------------------------
 * Purpose: This procedure returns the GMRP operation status of bridge device.
 * Input:   None.
 * Output:  gmrp_status - VAL_dot1dGmrpStatus_enabled \
 *                        VAL_dot1dGmrpStatus_disabled
 * Return:  TRUE/FALSE.
 * Note:    Please refer to the naming constant declarations for definition of gmrp_status.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_PMGR_GetDot1dGmrpStatus(UI32_T *gmrp_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = EXTBRG_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    EXTBRG_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_EXTBRG;
    msgbuf_p->msg_size = EXTBRG_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (EXTBRG_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = EXTBRG_MGR_IPC_GETDOT1DGMRPSTATUS;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *gmrp_status = msg_p->data.arg_ui32;

    return msg_p->type.ret_bool;
} /* End of EXTBRG_PMGR_GetDot1dGmrpStatus */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_PMGR_SetDot1dGmrpStatus
 *--------------------------------------------------------------------------
 * Purpose: This procedure set the GMRP operation status to a given value.
 * Input:   gmrp_status - VAL_dot1dGmrpStatus_enabled \
 *                        VAL_dot1dGmrpStatus_disabled
 * Output:  None.
 * Return:  TRUE/FALSE.
 * Note:    Please refer to the naming constant declarations for definition of gmrp_status.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_PMGR_SetDot1dGmrpStatus(UI32_T gmrp_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = EXTBRG_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    EXTBRG_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_EXTBRG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (EXTBRG_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = EXTBRG_MGR_IPC_SETDOT1DGMRPSTATUS;
    msg_p->data.arg_ui32 = gmrp_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            EXTBRG_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of EXTBRG_PMGR_SetDot1dGmrpStatus */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_PMGR_GetDot1dTrafficClassesEnabled
 *--------------------------------------------------------------------------
 * Purpose: This procedure returns the traffic classes operation status of bridge device.
 * Input: None.
 * Output: traffic_class_status  - VAL_dot1dTrafficClassesEnabled_true \
 *                                 VAL_dot1dTrafficClassesEnabled_false
 * Return: TRUE/FALSE.
 * Note: Please refer to the naming constant declarations for definition of traffic_class_status.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_PMGR_GetDot1dTrafficClassesEnabled(UI32_T *traffic_class_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = EXTBRG_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    EXTBRG_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_EXTBRG;
    msgbuf_p->msg_size = EXTBRG_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (EXTBRG_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = EXTBRG_MGR_IPC_GETDOT1DTRAFFICCLASSESENABLED;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *traffic_class_status = msg_p->data.arg_ui32;

    return msg_p->type.ret_bool;
} /* End of EXTBRG_PMGR_GetDot1dTrafficClassesEnabled */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_PMGR_SetDot1dTrafficClassesEnabled
 *--------------------------------------------------------------------------
 * Purpose: This procedure enables the traffic class operation on the bridge.
 * Input:   traffic_class_status - VAL_dot1dTrafficClassesEnabled_true \
 *                                 VAL_dot1dTrafficClassesEnabled_false
 * Output:  None.
 * Return:  TRUE/FALSE.
 * Note:    Please refer to the naming constant declarations for definition of traffic_class_status.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_PMGR_SetDot1dTrafficClassesEnabled(UI32_T traffic_class_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = EXTBRG_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    EXTBRG_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_EXTBRG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (EXTBRG_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = EXTBRG_MGR_IPC_SETDOT1DTRAFFICCLASSESENABLED;
    msg_p->data.arg_ui32 = traffic_class_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            EXTBRG_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of EXTBRG_PMGR_SetDot1dTrafficClassesEnabled */


/* LOCAL SUBPROGRAM BODIES
 */
