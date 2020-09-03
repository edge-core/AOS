/* =============================================================================
 * MODULE NAME : MLAG_POM.C
 * PURPOSE     : Provide definitions for MLAG IPC data management.
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
#include "mlag_om.h"
#include "mlag_pom.h"
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

/* FUNCTION NAME - MLAG_POM_InitiateProcessResources
 * PURPOSE : Initiate process resources for POM in context of the calling
 *           process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 */
BOOL_T MLAG_POM_InitiateProcessResources()
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    /* get the handle of ipc message queues for MLAG OM */
    if (SYSFUN_GetMsgQ(SYS_BLD_L2_L4_PROC_OM_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __func__);
        return FALSE;
    }

    return TRUE;
} /* End of MLAG_POM_InitiateProcessResources */

/* FUNCTION NAME - MLAG_POM_GetGlobalStatus
 * PURPOSE : Get global status of the CSC.
 * INPUT   : None
 * OUTPUT  : *status_p -- MLAG_TYPE_GLOBAL_STATUS_ENABLED /
 *                        MLAG_TYPE_GLOBAL_STATUS_DISABLED
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_POM_GetGlobalStatus(UI32_T *status_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T    msg_size = MLAG_OM_GET_MSG_SIZE(arg_ui32);

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    MLAG_OM_IpcMsg_T    *msg_p;

    /* BODY
     */

    msgbuf_p->cmd = SYS_MODULE_MLAG;
    msgbuf_p->msg_size = MLAG_OM_IPCMSG_TYPE_SIZE;

    msg_p = (MLAG_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = MLAG_OM_IPC_GETGLOBALSTATUS;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    *status_p = msg_p->data.arg_ui32;

    return msg_p->type.ret_ui32;
} /* End of MLAG_POM_GetGlobalStatus */

/* FUNCTION NAME - MLAG_POM_GetRunningGlobalStatus
 * PURPOSE : Get running global status of the CSC.
 * INPUT   : None
 * OUTPUT  : *status_p -- MLAG_TYPE_GLOBAL_STATUS_ENABLED /
 *                        MLAG_TYPE_GLOBAL_STATUS_DISABLED
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE    : None
 */
SYS_TYPE_Get_Running_Cfg_T MLAG_POM_GetRunningGlobalStatus(UI32_T *status_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T    msg_size = MLAG_OM_GET_MSG_SIZE(arg_ui32);

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    MLAG_OM_IpcMsg_T    *msg_p;

    /* BODY
     */

    msgbuf_p->cmd = SYS_MODULE_MLAG;
    msgbuf_p->msg_size = MLAG_OM_IPCMSG_TYPE_SIZE;

    msg_p = (MLAG_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = MLAG_OM_IPC_GETRUNNINGGLOBALSTATUS;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *status_p = msg_p->data.arg_ui32;

    return msg_p->type.ret_ui32;
} /* End of MLAG_POM_GetRunningGlobalStatus */

/* FUNCTION NAME - MLAG_POM_GetDomainEntry
 * PURPOSE : Get a MLAG domain entry.
 * INPUT   : entry_p->domain_id - MLAG domain ID
 * OUTPUT  : entry_p - buffer containing information for a domain
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_POM_GetDomainEntry(MLAG_TYPE_DomainEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T    msg_size = MLAG_OM_GET_MSG_SIZE(arg_domain);

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    MLAG_OM_IpcMsg_T    *msg_p;

    /* BODY
     */

    msgbuf_p->cmd = SYS_MODULE_MLAG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (MLAG_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = MLAG_OM_IPC_GETDOMAINENTRY;

    strncpy(msg_p->data.arg_domain.domain_id, entry_p->domain_id,
        MLAG_TYPE_MAX_DOMAIN_ID_LEN);

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    *entry_p = msg_p->data.arg_domain;

    return msg_p->type.ret_ui32;
} /* End of MLAG_POM_GetDomainEntry */

/* FUNCTION NAME - MLAG_POM_GetNextDomainEntry
 * PURPOSE : Get next MLAG domain entry.
 * INPUT   : entry_p->domain_id - MLAG domain ID
 * OUTPUT  : entry_p - buffer containing information for next domain
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : Empty string for domain ID to get the first entry
 */
UI32_T MLAG_POM_GetNextDomainEntry(MLAG_TYPE_DomainEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T    msg_size = MLAG_OM_GET_MSG_SIZE(arg_domain);

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    MLAG_OM_IpcMsg_T    *msg_p;

    /* BODY
     */

    msgbuf_p->cmd = SYS_MODULE_MLAG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (MLAG_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = MLAG_OM_IPC_GETNEXTDOMAINENTRY;

    strncpy(msg_p->data.arg_domain.domain_id, entry_p->domain_id,
        MLAG_TYPE_MAX_DOMAIN_ID_LEN);

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    *entry_p = msg_p->data.arg_domain;

    return msg_p->type.ret_ui32;
} /* End of MLAG_POM_GetNextDomainEntry */

/* FUNCTION NAME - MLAG_POM_GetMlagEntry
 * PURPOSE : Get a MLAG entry.
 * INPUT   : entry_p->mlag_id - MLAG ID
 * OUTPUT  : entry_p - buffer containing information for a MLAG
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_POM_GetMlagEntry(MLAG_TYPE_MlagEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T    msg_size = MLAG_OM_GET_MSG_SIZE(arg_mlag);

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    MLAG_OM_IpcMsg_T    *msg_p;

    /* BODY
     */

    msgbuf_p->cmd = SYS_MODULE_MLAG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (MLAG_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = MLAG_OM_IPC_GETMLAGENTRY;

    msg_p->data.arg_mlag.mlag_id = entry_p->mlag_id;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    *entry_p = msg_p->data.arg_mlag;

    return msg_p->type.ret_ui32;
} /* End of MLAG_POM_GetMlagEntry */

/* FUNCTION NAME - MLAG_POM_GetNextMlagEntry
 * PURPOSE : Get next MLAG entry.
 * INPUT   : entry_p->mlag_id - MLAG ID
 * OUTPUT  : entry_p - buffer containing information for next MLAG
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : 0 for MLAG ID to get the first entry
 */
UI32_T MLAG_POM_GetNextMlagEntry(MLAG_TYPE_MlagEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T    msg_size = MLAG_OM_GET_MSG_SIZE(arg_mlag);

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    MLAG_OM_IpcMsg_T    *msg_p;

    /* BODY
     */

    msgbuf_p->cmd = SYS_MODULE_MLAG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (MLAG_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = MLAG_OM_IPC_GETNEXTMLAGENTRY;

    msg_p->data.arg_mlag.mlag_id = entry_p->mlag_id;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    *entry_p = msg_p->data.arg_mlag;

    return msg_p->type.ret_ui32;
} /* End of MLAG_POM_GetNextMlagEntry */

/* FUNCTION NAME - MLAG_POM_GetNextMlagEntryByDomain
 * PURPOSE : Get next MLAG entry in the given domain.
 * INPUT   : entry_p->mlag_id   -- MLAG ID
 *           entry_p->domain_id -- MLAG domain ID
 * OUTPUT  : entry_p -- buffer containing information for next MLAG
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : 0 for MLAG ID to get the first entry
 */
UI32_T MLAG_POM_GetNextMlagEntryByDomain(MLAG_TYPE_MlagEntry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T    msg_size = MLAG_OM_GET_MSG_SIZE(arg_mlag);

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    MLAG_OM_IpcMsg_T    *msg_p;

    /* BODY
     */

    msgbuf_p->cmd = SYS_MODULE_MLAG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (MLAG_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = MLAG_OM_IPC_GETNEXTMLAGENTRYBYDOMAIN;

    msg_p->data.arg_mlag.mlag_id = entry_p->mlag_id;
    strncpy(msg_p->data.arg_mlag.domain_id, entry_p->domain_id,
        MLAG_TYPE_MAX_DOMAIN_ID_LEN);

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    *entry_p = msg_p->data.arg_mlag;

    return msg_p->type.ret_ui32;
} /* End of MLAG_POM_GetNextMlagEntryByDomain */

/* FUNCTION NAME - MLAG_POM_IsMlagPort
 * PURPOSE : Check whether a logical port is peer link or MLAG member.
 * INPUT   : lport -- logical port to be checked
 * OUTPUT  : None
 * RETURN  : TRUE  -- peer link or MLAG member
 *           FALSE -- otherwise
 * NOTE    : None
 */
BOOL_T MLAG_POM_IsMlagPort(UI32_T lport)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T    msg_size = MLAG_OM_GET_MSG_SIZE(arg_ui32);

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    MLAG_OM_IpcMsg_T    *msg_p;

    /* BODY
     */

    msgbuf_p->cmd = SYS_MODULE_MLAG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (MLAG_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = MLAG_OM_IPC_ISMLAGPORT;

    msg_p->data.arg_ui32 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            MLAG_OM_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return MLAG_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_bool;
} /* End of MLAG_POM_IsMlagPort */

/* -----------------------------------------------------------------------------
 * LOCAL SUBPROGRAM BODIES
 * -----------------------------------------------------------------------------
 */
