/*-----------------------------------------------------------------------------
 * FILE NAME: LLDP_POM.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file implements the APIs for LLDP OM IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/07/07     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "l_mm.h"
#include "sysfun.h"
#include "lldp_om.h"
#include "lldp_pom.h"
#include "lldp_type.h"


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
 * ROUTINE NAME : LLDP_POM_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resources for LLDP_POM in the calling process.
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
BOOL_T LLDP_POM_InitiateProcessResources(void)
{
    /* get the handle of ipc message queues for LLDP OM
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_L2_L4_PROC_OM_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
} /* End of LLDP_POM_InitiateProcessResources */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetSysAdminStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP global admin status
 * INPUT    : None
 * OUTPUT   : admin_status
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetSysAdminStatus(UI32_T *admin_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = LLDP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETSYSADMINSTATUS;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *admin_status = msg_p->data.arg_ui32;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetSysAdminStatus */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningSysAdminStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP global admin status
 * INPUT    : None
 * OUTPUT   : admin_status
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningSysAdminStatus(UI32_T *admin_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = LLDP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETRUNNINGSYSADMINSTATUS;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *admin_status = msg_p->data.arg_ui32;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetRunningSysAdminStatus */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetSysConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get configuration entry info. for system.
 * INPUT    : LLDP_MGR_SysConfigEntry_T  *config_entry
 * OUTPUT   : LLDP_MGR_SysConfigEntry_T  *config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetSysConfigEntry(LLDP_MGR_SysConfigEntry_T *config_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_sys_config_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = LLDP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETSYSCONFIGENTRY;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *config_entry = msg_p->data.arg_sys_config_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetSysConfigEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRemoteSystemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get related TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteSystemData_T *system_entry
 * OUTPUT   : LLDP_MGR_RemoteSystemData_T *system_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.2.2/9.5.2.3/9.5.3.3/9.5.5.2/
 *            9.5.6.2/9.5.7.2/9.5.8.1/9.5.8.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetRemoteSystemData(LLDP_MGR_RemoteSystemData_T *system_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_remote_system_data);
    SYSFUN_Msg_T *msgbuf_p = NULL;
    LLDP_OM_IpcMsg_T *msg_p;
    BOOL_T result;

    msgbuf_p = (SYSFUN_Msg_T*)L_MM_Malloc(SYSFUN_SIZE_OF_MSG(msg_size),
                   L_MM_USER_ID2(SYS_MODULE_LLDP,
                       LLDP_TYPE_TRACE_ID_POM_GETREMOTESYSTEMDATA));
    if (msgbuf_p == NULL)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETREMOTESYSTEMDATA;
    msg_p->data.arg_remote_system_data = *system_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return LLDP_TYPE_RETURN_ERROR;
    }

    *system_entry = msg_p->data.arg_remote_system_data;
    result = msg_p->type.ret_ui32;
    L_MM_Free(msgbuf_p);

    return result;
} /* End of LLDP_POM_GetRemoteSystemData */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetNextRemManAddrByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * OUTPUT   : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : This function is used by CLI/WEB
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetNextRemManAddrByIndex(LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_remote_mgmt_addr_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETNEXTREMMANADDRBYINDEX;
    msg_p->data.arg_remote_mgmt_addr_entry = *man_addr_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *man_addr_entry = msg_p->data.arg_remote_mgmt_addr_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetNextRemManAddrByIndex */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningMsgTxHoldMul
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : None
 * OUTPUT   : UI32_T  *msg_tx_hold
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningMsgTxHoldMul(UI32_T  *msg_tx_hold)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = LLDP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETRUNNINGMSGTXHOLDMUL;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *msg_tx_hold = msg_p->data.arg_ui32;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetRunningMsgTxHoldMul */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningMsgTxInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : None
 * OUTPUT   : UI32_T  *msg_tx_interval
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningMsgTxInterval(UI32_T  *msg_tx_interval)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = LLDP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETRUNNINGMSGTXINTERVAL;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *msg_tx_interval = msg_p->data.arg_ui32;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetRunningMsgTxInterval */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningNotifyInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : None
 * OUTPUT   : UI32_T *notify_interval
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningNotifyInterval(UI32_T *notify_interval)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = LLDP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETRUNNINGNOTIFYINTERVAL;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *notify_interval = msg_p->data.arg_ui32;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetRunningNotifyInterval */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningReinitDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : None
 * OUTPUT   : UI32_T *reinit_delay
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningReinitDelay(UI32_T *reinit_delay)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = LLDP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETRUNNINGREINITDELAY;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *reinit_delay = msg_p->data.arg_ui32;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetRunningReinitDelay */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningTxDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : None
 * OUTPUT   : UI32_T  *tx_delay_time
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningTxDelay(UI32_T  *tx_delay_time)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = LLDP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETRUNNINGTXDELAY;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *tx_delay_time = msg_p->data.arg_ui32;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetRunningTxDelay */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningPortAdminStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : lport
 * OUTPUT   : basic_tlv_enable
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningPortAdminStatus(UI32_T lport, UI8_T *admin_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_grp_ui32_ui8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETRUNNINGPORTADMINSTATUS;
    msg_p->data.arg_grp_ui32_ui8.arg_ui32 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *admin_status = msg_p->data.arg_grp_ui32_ui8.arg_ui8;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetRunningPortAdminStatus */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningPortBasicTlvTransfer
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : lport
 * OUTPUT   : basic_tlvs_tx_flag, basic_tlvs_change_flag
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningPortBasicTlvTransfer(UI32_T lport, UI8_T *basic_tlvs_tx_flag, UI8_T *basic_tlvs_change_flag)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_grp_ui32_ui8_ui8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETRUNNINGPORTBASICTLVTRANSFER;
    msg_p->data.arg_grp_ui32_ui8_ui8.arg_ui32 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *basic_tlvs_tx_flag = msg_p->data.arg_grp_ui32_ui8_ui8.arg_ui8_1;
    *basic_tlvs_change_flag = msg_p->data.arg_grp_ui32_ui8_ui8.arg_ui8_2;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetRunningPortBasicTlvTransfer */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningPortManAddrTlvTransfer
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : lport
 * OUTPUT   : man_addr_tlv_enable
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningPortManAddrTlvTransfer(UI32_T lport, UI8_T *man_addr_tlv_enable)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_grp_ui32_ui8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETRUNNINGPORTMANADDRTLVTRANSFER;
    msg_p->data.arg_grp_ui32_ui8.arg_ui32 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *man_addr_tlv_enable = msg_p->data.arg_grp_ui32_ui8.arg_ui8;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetRunningPortManAddrTlvTransfer */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningPortNotificationEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : lport
 * OUTPUT   : notify_enable
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningPortNotificationEnable(UI32_T lport, BOOL_T *notify_enable)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_grp_ui32_ui8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETRUNNINGPORTNOTIFICATIONENABLE;
    msg_p->data.arg_grp_ui32_ui8.arg_ui32 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *notify_enable = msg_p->data.arg_grp_ui32_ui8.arg_ui8;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetRunningPortNotificationEnable */

#if (LLDP_TYPE_EXT_802DOT1 == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningXdot1ConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get current 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : xdot1_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetRunningXdot1ConfigEntry(LLDP_MGR_Xdot1ConfigEntry_T *xdot1_config_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot1_config_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETRUNNINGXDOT1CONFIGENTRY;
    msg_p->data.arg_xdot1_config_entry = *xdot1_config_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_config_entry = msg_p->data.arg_xdot1_config_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetRunningXdot1ConfigEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXdot1RemEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote entry
 * INPUT    : xdot1_rem_entry
 * OUTPUT   : xdot1_rem_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetXdot1RemEntry(LLDP_MGR_Xdot1RemEntry_T *xdot1_rem_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot1_rem_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETXDOT1REMENTRY;
    msg_p->data.arg_xdot1_rem_entry = *xdot1_rem_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_rem_entry = msg_p->data.arg_xdot1_rem_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetXdot1RemEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXdot1RemProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol vlan entry
 * INPUT    : xdot1_rem_proto_vlan_entry
 * OUTPUT   : xdot1_rem_proto_vlan_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetXdot1RemProtoVlanEntry(LLDP_MGR_Xdot1RemProtoVlanEntry_T *xdot1_rem_proto_vlan_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot1_rem_proto_vlan_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETXDOT1REMPROTOVLANENTRY;
    msg_p->data.arg_xdot1_rem_proto_vlan_entry = *xdot1_rem_proto_vlan_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_rem_proto_vlan_entry = msg_p->data.arg_xdot1_rem_proto_vlan_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetNextXdot1RemProtoVlanEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol vlan entry
 * INPUT    : xdot1_rem_proto_vlan_entry
 * OUTPUT   : xdot1_rem_proto_vlan_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetNextXdot1RemProtoVlanEntryByIndex(LLDP_MGR_Xdot1RemProtoVlanEntry_T *xdot1_rem_proto_vlan_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot1_rem_proto_vlan_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETNEXTXDOT1REMPROTOVLANENTRYBYINDEX;
    msg_p->data.arg_xdot1_rem_proto_vlan_entry = *xdot1_rem_proto_vlan_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_rem_proto_vlan_entry = msg_p->data.arg_xdot1_rem_proto_vlan_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetNextXdot1RemProtoVlanEntryByIndex */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXdot1RemProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote vlan name entry
 * INPUT    : xdot1_rem_vlan_name_entry
 * OUTPUT   : xdot1_rem_vlan_name_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetXdot1RemVlanNameEntry(LLDP_MGR_Xdot1RemVlanNameEntry_T *xdot1_rem_vlan_name_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot1_rem_vlan_name_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETXDOT1REMVLANNAMEENTRY;
    msg_p->data.arg_xdot1_rem_vlan_name_entry = *xdot1_rem_vlan_name_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_rem_vlan_name_entry = msg_p->data.arg_xdot1_rem_vlan_name_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetNextXdot1RemVlanNameEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote vlan name entry
 * INPUT    : xdot1_rem_vlan_name_entry
 * OUTPUT   : xdot1_rem_vlan_name_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetNextXdot1RemVlanNameEntryByIndex(LLDP_MGR_Xdot1RemVlanNameEntry_T *xdot1_rem_vlan_name_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot1_rem_vlan_name_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETNEXTXDOT1REMVLANNAMEENTRYBYINDEX;
    msg_p->data.arg_xdot1_rem_vlan_name_entry = *xdot1_rem_vlan_name_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_rem_vlan_name_entry = msg_p->data.arg_xdot1_rem_vlan_name_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetNextXdot1RemVlanNameEntryByIndex */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXdot1RemProtocolEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol entry
 * INPUT    : xdot1_rem_protocol_entry
 * OUTPUT   : xdot1_rem_protocol_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetXdot1RemProtocolEntry(LLDP_MGR_Xdot1RemProtocolEntry_T *xdot1_rem_protocol_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot1_rem_protocol_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETXDOT1REMPROTOCOLENTRY;
    msg_p->data.arg_xdot1_rem_protocol_entry = *xdot1_rem_protocol_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_rem_protocol_entry = msg_p->data.arg_xdot1_rem_protocol_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetNextXdot1RemProtocolEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol entry
 * INPUT    : xdot1_rem_protocol_entry
 * OUTPUT   : xdot1_rem_protocol_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetNextXdot1RemProtocolEntryByIndex(LLDP_MGR_Xdot1RemProtocolEntry_T * xdot1_rem_protocol_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot1_rem_protocol_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETNEXTXDOT1REMPROTOCOLENTRYBYINDEX;
    msg_p->data.arg_xdot1_rem_protocol_entry = *xdot1_rem_protocol_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_rem_protocol_entry = msg_p->data.arg_xdot1_rem_protocol_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetNextXdot1RemProtocolEntryByIndex */

#if (SYS_CPNT_CN == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXdot1RemCnEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote congestion notification entry
 * INPUT    : xdot1_rem_cn_entry
 * OUTPUT   : xdot1_rem_cn_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetXdot1RemCnEntry(LLDP_MGR_Xdot1RemCnEntry_T *xdot1_rem_cn_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot1_rem_cn_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETXDOT1REMCNENTRY;
    msg_p->data.arg_xdot1_rem_cn_entry = *xdot1_rem_cn_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_rem_cn_entry = msg_p->data.arg_xdot1_rem_cn_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetXdot1RemCnEntry */
#endif /* #if (SYS_CPNT_CN == TRUE) */
#endif /* #if (LLDP_TYPE_EXT_802DOT1 == TRUE) */

#if (LLDP_TYPE_EXT_802DOT3 == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningXdot3PortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get current 802.3 extension port config entry
 * INPUT    : xdot3_port_config_entry
 * OUTPUT   : xdot3_port_config_entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetRunningXdot3PortConfigEntry(LLDP_MGR_Xdot3PortConfigEntry_T * xdot3_port_config_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot3_port_config_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETRUNNINGXDOT3PORTCONFIGENTRY;
    msg_p->data.arg_xdot3_port_config_entry = *xdot3_port_config_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_port_config_entry = msg_p->data.arg_xdot3_port_config_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetRunningXdot3PortConfigEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXdot3RemPortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote port entry
 * INPUT    : xdot3_rem_port_entry
 * OUTPUT   : xdot3_rem_port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetXdot3RemPortEntry(LLDP_MGR_Xdot3RemPortEntry_T *xdot3_rem_port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot3_rem_port_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETXDOT3REMPORTENTRY;
    msg_p->data.arg_xdot3_rem_port_entry = *xdot3_rem_port_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_rem_port_entry = msg_p->data.arg_xdot3_rem_port_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetXdot3RemPortEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXdot3RemPowerEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote power entry
 * INPUT    : xdot3_rem_power_entry
 * OUTPUT   : xdot3_rem_power_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetXdot3RemPowerEntry(LLDP_MGR_Xdot3RemPowerEntry_T *xdot3_rem_power_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot3_rem_power_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETXDOT3REMPOWERENTRY;
    msg_p->data.arg_xdot3_rem_power_entry = *xdot3_rem_power_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_rem_power_entry = msg_p->data.arg_xdot3_rem_power_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetXdot3RemPowerEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXdot3RemLinkAggEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote link aggregation entry
 * INPUT    : xdot3_rem_link_agg_entry
 * OUTPUT   : xdot3_rem_link_agg_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetXdot3RemLinkAggEntry(LLDP_MGR_Xdot3RemLinkAggEntry_T *xdot3_rem_link_agg_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot3_rem_link_agg_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETXDOT3REMLINKAGGENTRY;
    msg_p->data.arg_xdot3_rem_link_agg_entry = *xdot3_rem_link_agg_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_rem_link_agg_entry = msg_p->data.arg_xdot3_rem_link_agg_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetXdot3RemLinkAggEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXdot3RemMaxFrameSizeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote maximum frame size entry
 * INPUT    : xdot3_rem_max_frame_size_entry
 * OUTPUT   : xdot3_rem_max_frame_size_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetXdot3RemMaxFrameSizeEntry(LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T *xdot3_rem_max_frame_size_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xdot3_rem_max_frame_size_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETXDOT3REMMAXFRAMESIZEENTRY;
    msg_p->data.arg_xdot3_rem_max_frame_size_entry = *xdot3_rem_max_frame_size_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_rem_max_frame_size_entry = msg_p->data.arg_xdot3_rem_max_frame_size_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_POM_GetXdot3RemMaxFrameSizeEntry */
#endif /* #if (LLDP_TYPE_EXT_802DOT3 == TRUE) */

#if (LLDP_TYPE_MED == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXMedConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED configuration entry
 * INPUT    : None
 * OUTPUT   : xmed_config_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_POM_GetXMedConfigEntry(LLDP_MGR_XMedConfig_T *xmed_config_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xmed_config_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = LLDP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETXMEDCONFIGENTRY;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *xmed_config_entry = msg_p->data.arg_xmed_config_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningXMedFastStartRepeatCount
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED running cfg fast start repeat count
 * INPUT    : repeat_count
 * OUTPUT   : repeat_count
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningXMedFastStartRepeatCount(UI32_T  *repeat_count)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = LLDP_OM_IPCMSG_TYPE_SIZE;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETRUNNINGXMEDFASTSTARTREPEATCOUNT;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *repeat_count = msg_p->data.arg_ui32;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXMedRemCapEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device capability entry
 * INPUT    : None
 * OUTPUT   : rem_cap_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_POM_GetXMedRemCapEntry(LLDP_MGR_XMedRemCapEntry_T *rem_cap_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xmed_rem_cap_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETXMEDREMCAPENTRY;
    msg_p->data.arg_xmed_rem_cap_entry = *rem_cap_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *rem_cap_entry = msg_p->data.arg_xmed_rem_cap_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXMedRemMediaPolicyEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device media policy entry
 * INPUT    : None
 * OUTPUT   : rem_med_policy_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_POM_GetXMedRemMediaPolicyEntry(LLDP_MGR_XMedRemMediaPolicyEntry_T *rem_med_policy_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xmed_rem_med_policy_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETXMEDREMMEDIAPOLICYENTRY;
    msg_p->data.arg_xmed_rem_med_policy_entry = *rem_med_policy_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *rem_med_policy_entry = msg_p->data.arg_xmed_rem_med_policy_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXMedRemInventoryEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device inventory entry
 * INPUT    : None
 * OUTPUT   : rem_inventory_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_POM_GetXMedRemInventoryEntry(LLDP_MGR_XMedRemInventoryEntry_T *rem_inventory_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xmed_rem_inventory_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETXMEDREMINVENTORYENTRY;
    msg_p->data.arg_xmed_rem_inventory_entry = *rem_inventory_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *rem_inventory_entry = msg_p->data.arg_xmed_rem_inventory_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXMedRemLocationEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location entry
 * INPUT    : None
 * OUTPUT   : rem_location_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_POM_GetXMedRemLocationEntry(LLDP_MGR_XMedRemLocationEntry_T *rem_location_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xmed_rem_location_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETXMEDREMLOCATIONENTRY;
    msg_p->data.arg_xmed_rem_location_entry = *rem_location_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *rem_location_entry = msg_p->data.arg_xmed_rem_location_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXMedRemPoeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe entry
 * INPUT    : None
 * OUTPUT   : rem_poe_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_POM_GetXMedRemPoeEntry(LLDP_MGR_XMedRemPoeEntry_T *rem_poe_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xmed_rem_poe_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETXMEDREMPOEENTRY;
    msg_p->data.arg_xmed_rem_poe_entry = *rem_poe_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *rem_poe_entry = msg_p->data.arg_xmed_rem_poe_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXMedRemPoePseEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe pse entry
 * INPUT    : None
 * OUTPUT   : rem_pse_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_POM_GetXMedRemPoePseEntry(LLDP_MGR_XMedRemPoePseEntry_T *rem_pse_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xmed_rem_pse_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETXMEDREMPOEPSEENTRY;
    msg_p->data.arg_xmed_rem_pse_entry = *rem_pse_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *rem_pse_entry = msg_p->data.arg_xmed_rem_pse_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXMedRemPoePdEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe pse entry
 * INPUT    : None
 * OUTPUT   : rem_pd_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_POM_GetXMedRemPoePdEntry(LLDP_MGR_XMedRemPoePdEntry_T *rem_pd_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_xmed_rem_pd_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETXMEDREMPOEPDENTRY;
    msg_p->data.arg_xmed_rem_pd_entry = *rem_pd_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *rem_pd_entry = msg_p->data.arg_xmed_rem_pd_entry;

    return msg_p->type.ret_bool;
}
#endif /* #if (LLDP_TYPE_MED == TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetAllRemIndexByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : Get all remote index for a port
 * INPUT    : lport
 * OUTPUT   : remote_index
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_POM_GetAllRemIndexByPort(UI32_T lport, UI32_T remote_index[LLDP_TYPE_MAX_NBR_OF_REM_DATA_ENTRY_PER_PORT])
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_OM_GET_MSG_SIZE(arg_grp_ui32_index);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_OM_IPC_GETALLREMINDEXBYPORT;
    msg_p->data.arg_grp_ui32_index.arg_ui32 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    memcpy(remote_index, msg_p->data.arg_grp_ui32_index.arg_index,
        sizeof(UI32_T)*LLDP_TYPE_MAX_NBR_OF_REM_DATA_ENTRY_PER_PORT);

    return msg_p->type.ret_bool;
} /* End of LLDP_POM_GetAllRemIndexByPort */

/* LOCAL SUBPROGRAM BODIES
 */
