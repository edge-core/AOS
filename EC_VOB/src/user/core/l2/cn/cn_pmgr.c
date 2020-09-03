/* MODULE NAME - CN_PMGR.C
 * PURPOSE : Provides the definitions for CN IPC functional management.
 * NOTES   : None.
 * HISTORY : 2012/09/12 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2012
 */

/* INCLUDE FILE DECLARATIONS
 */

#include <stdio.h>
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sysfun.h"
#include "cn_mgr.h"
#include "cn_pmgr.h"
#include "cn_type.h"

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

/* FUNCTION NAME - CN_PMGR_InitiateProcessResources
 * PURPOSE : Initiate resources for CN_PMGR in the calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTES   : None
 */
BOOL_T CN_PMGR_InitiateProcessResources(void)
{
    /* get the handle of ipc message queues for LBD MGR
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_DCB_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\n%s(): SYSFUN_GetMsgQ failed.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
} /* End of CN_PMGR_InitiateProcessResources */

/* FUNCTION NAME - CN_PMGR_SetGlobalAdminStatus
 * PURPOSE : Set CN global admin status.
 * INPUT   : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_PMGR_SetGlobalAdminStatus(UI32_T status)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T        msg_size = CN_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    CN_MGR_IpcMsg_T     *msg_p;

    /* BODY
     */

    msgbuf_p->cmd = SYS_MODULE_CN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (CN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = CN_MGR_IPC_SETGLOBALADMINSTATUS;
    msg_p->data.arg_ui32 = status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            CN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return CN_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of CN_PMGR_SetGlobalAdminStatus */

/* FUNCTION NAME - CN_PMGR_SetCnmTxPriority
 * PURPOSE : Set the priority used for transmitting CNMs.
 * INPUT   : priority - priority in the range between 0 and 7
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_PMGR_SetCnmTxPriority(UI32_T priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T        msg_size = CN_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    CN_MGR_IpcMsg_T     *msg_p;

    /* BODY
     */

    msgbuf_p->cmd = SYS_MODULE_CN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (CN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = CN_MGR_IPC_SETCNMTXPRIORITY;
    msg_p->data.arg_ui32 = priority;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            CN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return CN_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of CN_PMGR_SetCnmTxPriority */

/* FUNCTION NAME - CN_PMGR_SetCnpv
 * PURPOSE : Set a priority to be CNPV or not.
 * INPUT   : priority - the specify priority
 *           active   - TRUE/FALSE
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_PMGR_SetCnpv(UI32_T priority, BOOL_T active)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T        msg_size = CN_MGR_GET_MSG_SIZE(arg_grp_ui32_bool);
    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    CN_MGR_IpcMsg_T     *msg_p;

    /* BODY
     */

    msgbuf_p->cmd = SYS_MODULE_CN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (CN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = CN_MGR_IPC_SETCNPV;
    msg_p->data.arg_grp_ui32_bool.arg_ui32 = priority;
    msg_p->data.arg_grp_ui32_bool.arg_bool = active;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            CN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return CN_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of CN_PMGR_SetCnpv */

/* FUNCTION NAME - CN_PMGR_SetCnpvDefenseMode
 * PURPOSE : Set the defense mode for a CNPV.
 * INPUT   : priority - the specified CNPV
 *           mode     - CN_TYPE_DEFENSE_MODE_E
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_PMGR_SetCnpvDefenseMode(UI32_T priority, UI32_T mode)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T        msg_size = CN_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    CN_MGR_IpcMsg_T     *msg_p;

    /* BODY
     */

    msgbuf_p->cmd = SYS_MODULE_CN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (CN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = CN_MGR_IPC_SETCNPVDEFENSEMODE;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = priority;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2 = mode;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            CN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return CN_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of CN_PMGR_SetCnpvDefenseMode */

/* FUNCTION NAME - CN_PMGR_SetCnpvAlternatePriority
 * PURPOSE : Set the alternate priority used for a CNPV.
 * INPUT   : priority     - the specified CNPV
 *           alt_priority - the specified alternate priority
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_PMGR_SetCnpvAlternatePriority(UI32_T priority, UI32_T alt_priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T        msg_size = CN_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    CN_MGR_IpcMsg_T     *msg_p;

    /* BODY
     */

    msgbuf_p->cmd = SYS_MODULE_CN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (CN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = CN_MGR_IPC_SETCNPVALTERNATEPRIORITY;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = priority;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2 = alt_priority;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            CN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return CN_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of CN_PMGR_SetCnpvAlternatePriority */

/* FUNCTION NAME - CN_PMGR_SetPortCnpvDefenseMode
 * PURPOSE : Set the defense mode for a CNPV on a port.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 *           mode     - CN_TYPE_DEFENSE_MODE_E
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_PMGR_SetPortCnpvDefenseMode(UI32_T priority, UI32_T lport, UI32_T mode)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T        msg_size = CN_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    CN_MGR_IpcMsg_T     *msg_p;

    /* BODY
     */

    msgbuf_p->cmd = SYS_MODULE_CN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (CN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = CN_MGR_IPC_SETPORTCNPVDEFENSEMODE;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = priority;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = lport;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3 = mode;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            CN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return CN_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of CN_PMGR_SetPortCnpvDefenseMode */

/* FUNCTION NAME - CN_PMGR_SetPortCnpvAlternatePriority
 * PURPOSE : Set the alternate priority used for a CNPV on a port.
 * INPUT   : priority     - the specified CNPV
 *           lport        - the specified logical port
 *           alt_priority - the specified alternate priority
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_PMGR_SetPortCnpvAlternatePriority(UI32_T priority, UI32_T lport, UI32_T alt_priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T        msg_size = CN_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32_ui32);
    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    CN_MGR_IpcMsg_T     *msg_p;

    /* BODY
     */

    msgbuf_p->cmd = SYS_MODULE_CN;
    msgbuf_p->msg_size = msg_size;

    msg_p = (CN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = CN_MGR_IPC_SETPORTCNPVALTERNATEPRIORITY;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1 = priority;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2 = lport;
    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3 = alt_priority;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            CN_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return CN_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of CN_PMGR_SetPortCnpvAlternatePriority */

/* LOCAL SUBPROGRAM BODIES
 */
