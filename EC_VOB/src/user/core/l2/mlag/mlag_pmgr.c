/* =============================================================================
 * MODULE NAME : MLAG_PMGR.C
 * PURPOSE     : Provide definitions for MLAG IPC operational functions.
 * NOTE        : None.
 * HISTORY     :
 *     2014/04/18 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2014
 * =============================================================================
 */

/* -----------------------------------------------------------------------------
 * INCLUDE FILE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sysfun.h"
#include "mlag_mgr.h"
#include "mlag_pmgr.h"
#include "mlag_type.h"

/* -----------------------------------------------------------------------------
 * NAMING CONSTANT DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * MACRO FUNCTION DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * DATA TYPE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * LOCAL SUBPROGRAM DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * STATIC VARIABLE DEFINITIONS
 * -----------------------------------------------------------------------------
 */

static SYSFUN_MsgQ_T ipcmsgq_handle;

/* -----------------------------------------------------------------------------
 * EXPORTED SUBPROGRAM BODIES
 * -----------------------------------------------------------------------------
 */

/* FUNCTION NAME - MLAG_PMGR_InitiateProcessResources
 * PURPOSE : Initiate process resources for PMGR in context of the calling
 *           process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 */
BOOL_T MLAG_PMGR_InitiateProcessResources()
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    /* get the handle of ipc message queues for MLAG MGR */
    if (SYSFUN_GetMsgQ(SYS_BLD_MLAG_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __func__);
        return FALSE;
    }

    return TRUE;
} /* End of MLAG_PMGR_InitiateProcessResources */

/* FUNCTION NAME - MLAG_PMGR_SetGlobalStatus
 * PURPOSE : Set global status for the feature.
 * INPUT   : status -- MLAG_TYPE_GLOBAL_STATUS_ENABLED /
 *                     MLAG_TYPE_GLOBAL_STATUS_DISABLED
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_PMGR_SetGlobalStatus(UI32_T status)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T    msg_size = MLAG_MGR_GET_MSG_SIZE(arg_ui32);

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    MLAG_MGR_IpcMsg_T   *msg_p;

    /* BODY
     */

    msgbuf_p->cmd = SYS_MODULE_MLAG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (MLAG_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = MLAG_MGR_IPC_SETGLOBALSTATUS;
    msg_p->data.arg_ui32 = status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            MLAG_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of MLAG_PMGR_SetGlobalStatus */

/* FUNCTION NAME - MLAG_PMGR_SetDomain
 * PURPOSE : Set a MLAG domain.
 * INPUT   : domain_id_p -- domain ID (MLAG_TYPE_MAX_DOMAIN_ID_LENGTH string)
 *           lport       -- peer link
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : Replace if the domain ID has existed
 */
UI32_T MLAG_PMGR_SetDomain(char *domain_id_p, UI32_T lport)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T    msg_size = MLAG_MGR_GET_MSG_SIZE(arg_str_ui32);

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    MLAG_MGR_IpcMsg_T   *msg_p;

    /* BODY
     */

    if (domain_id_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    msgbuf_p->cmd = SYS_MODULE_MLAG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (MLAG_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = MLAG_MGR_IPC_SETDOMAIN;
    memset(msg_p->data.arg_str_ui32.arg_str, 0, MLAG_TYPE_MAX_DOMAIN_ID_LEN+1);
    strncpy(msg_p->data.arg_str_ui32.arg_str, domain_id_p,
        MLAG_TYPE_MAX_DOMAIN_ID_LEN);
    msg_p->data.arg_str_ui32.arg_ui32 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            MLAG_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of MLAG_PMGR_SetDomain */

/* FUNCTION NAME - MLAG_PMGR_RemoveDomain
 * PURPOSE : Remove a MLAG domain.
 * INPUT   : domain_id_p -- domain ID (MLAG_TYPE_MAX_DOMAIN_ID_LENGTH string)
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : Success if the domain ID does not exist
 */
UI32_T MLAG_PMGR_RemoveDomain(char *domain_id_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T    msg_size = MLAG_MGR_GET_MSG_SIZE(arg_str);

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    MLAG_MGR_IpcMsg_T   *msg_p;

    /* BODY
     */

    if (domain_id_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    msgbuf_p->cmd = SYS_MODULE_MLAG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (MLAG_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = MLAG_MGR_IPC_REMOVEDOMAIN;
    memset(msg_p->data.arg_str, 0, MLAG_TYPE_MAX_DOMAIN_ID_LEN+1);
    strncpy(msg_p->data.arg_str, domain_id_p, MLAG_TYPE_MAX_DOMAIN_ID_LEN);

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            MLAG_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of MLAG_PMGR_RemoveDomain */

/* FUNCTION NAME - MLAG_PMGR_SetMlag
 * PURPOSE : Set a MLAG.
 * INPUT   : mlag_id     -- MLAG ID
 *           lport       -- MLAG member
 *           domain_id_p -- domain ID (MLAG_TYPE_MAX_DOMAIN_ID_LENGTH string)
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : Replace if the MLAG ID has existed
 */
UI32_T MLAG_PMGR_SetMlag(UI32_T mlag_id, UI32_T lport, char *domain_id_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T    msg_size = MLAG_MGR_GET_MSG_SIZE(arg_ui32_ui32_str);

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    MLAG_MGR_IpcMsg_T   *msg_p;

    /* BODY
     */

    if (domain_id_p == NULL)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    msgbuf_p->cmd = SYS_MODULE_MLAG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (MLAG_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = MLAG_MGR_IPC_SETMLAG;
    msg_p->data.arg_ui32_ui32_str.arg_ui32_1 = mlag_id;
    msg_p->data.arg_ui32_ui32_str.arg_ui32_2 = lport;
    memset(msg_p->data.arg_ui32_ui32_str.arg_str, 0,
        MLAG_TYPE_MAX_DOMAIN_ID_LEN+1);
    strncpy(msg_p->data.arg_ui32_ui32_str.arg_str, domain_id_p,
        MLAG_TYPE_MAX_DOMAIN_ID_LEN);

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            MLAG_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of MLAG_PMGR_SetMlag */

/* FUNCTION NAME - MLAG_PMGR_RemoveMlag
 * PURPOSE : Remove a MLAG.
 * INPUT   : mlag_id -- MLAG ID
 * OUTPUT  : None
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_PMGR_RemoveMlag(UI32_T mlag_id)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T    msg_size = MLAG_MGR_GET_MSG_SIZE(arg_ui32);

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    MLAG_MGR_IpcMsg_T   *msg_p;

    /* BODY
     */

    msgbuf_p->cmd = SYS_MODULE_MLAG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (MLAG_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = MLAG_MGR_IPC_REMOVEMLAG;
    msg_p->data.arg_ui32 = mlag_id;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            MLAG_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of MLAG_PMGR_RemoveMlag */

/* -----------------------------------------------------------------------------
 * LOCAL SUBPROGRAM BODIES
 * -----------------------------------------------------------------------------
 */
