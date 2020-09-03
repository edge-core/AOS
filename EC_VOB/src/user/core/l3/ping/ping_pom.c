/* Module Name: PING_POM.H
 * Purpose:
 *    Implements the APIs for IPCs with PING OM.
 *
 * Notes:
 *    None.
 * History:
 *      Date        --  Modifier,   Reason
 *      2007/11/01  --  Timon,      Create
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "l_mm.h"
#include "sysfun.h"
#include "ping_om.h"
#include "ping_pom.h"
#include "ping_type.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* trace id definition when using L_MM
 */
enum
{
    PING_TYPE_TRACEID_POM_GETMSTPINSTANCEVLANMAPPED,
};


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DEFINITIONS
 */

static SYSFUN_MsgQ_T ping_om_ipcmsgq_handle;


/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: PING_POM_InitiateProcessResources
 * PURPOSE:
 *          Initiate resources for PING_POM in the calling process.
 * INPUT:
 *          None.
 * OUTPUT:
 *          None.
 * RETURN:
 *          TRUE    -- Success
 *          FALSE   -- Fail
 * NOTES:
 *          None.
 */
BOOL_T PING_POM_InitiateProcessResources(void)
{
    /* get the ipc message queues for PING OM
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_APP_PROTOCOL_PROC_OM_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ping_om_ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\n%s(): L_IPCMSGQ_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
} /* End of PING_POM_InitiateProcessResources */

/* FUNCTION NAME: PING_POM_GetCtlEntry
 * PURPOSE:
 *          Get the specific ping control entry.
 * INPUT:
 *          ctrl_entry_p    -- the specific control entry, including keys:
 *                             ping_ctl_owner_index, ping_ctl_test_name.
 * OUTPUT:
 *          ctrl_entry_p    -- the specific control entry if it exists.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_WORKSPACE
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. key: ping_ctl_owner_index, ping_ctl_test_name.
 */
UI32_T PING_POM_GetCtlEntry(PING_TYPE_PingCtlEntry_T *ctrl_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = PING_OM_GET_MSG_SIZE(ctrl_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PING_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PING;
    msgbuf_p->msg_size = msg_size;

    msg_p = (PING_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = PING_OM_IPCCMD_GETCTLENTRY;
    memcpy(&(msg_p->data.ctrl_entry), ctrl_entry_p, sizeof(PING_TYPE_PingCtlEntry_T));

    if (SYSFUN_SendRequestMsg(ping_om_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return PING_TYPE_FAIL;
    }
    memcpy(ctrl_entry_p, &(msg_p->data.ctrl_entry), sizeof(PING_TYPE_PingCtlEntry_T));
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME:PING_POM_GetNextCtlEntry
 * PURPOSE:
 *          Get the next ping control entry.
 * INPUT:
 *          ctrl_entry_p    -- the specific control entry, including keys:
 *                             ping_ctl_owner_index, ping_ctl_test_name.
 *
 * OUTPUT:
 *          ctrl_entry_p    -- the specific control entry if it exists.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. get entry by the key: ping_ctl_owner_index, ping_ctl_test_name.
 */
UI32_T PING_POM_GetNextCtlEntry(PING_TYPE_PingCtlEntry_T *ctrl_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = PING_OM_GET_MSG_SIZE(ctrl_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PING_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PING;
    msgbuf_p->msg_size = msg_size;

    msg_p = (PING_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = PING_OM_IPCCMD_GETNEXTCTLENTRY;
    memcpy(&(msg_p->data.ctrl_entry), ctrl_entry_p, sizeof(PING_TYPE_PingCtlEntry_T));

    if (SYSFUN_SendRequestMsg(ping_om_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return PING_TYPE_FAIL;
    }
    memcpy(ctrl_entry_p, &(msg_p->data.ctrl_entry), sizeof(PING_TYPE_PingCtlEntry_T));
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME: PING_POM_GetProbeHistoryEntry
 * PURPOSE:
 *          To get the specific probe history entry base on the given index
 * INPUT:
 *          prob_history_entry_p    -- the specific probe history entry, including keys:
 *                                     ping_ctl_owner_index, ping_ctl_test_name, ping_probe_history_index.
 * OUTPUT:
 *          prob_history_entry_p - the probe history entry.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_ENTRY
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_PROCESS_NOT_COMPLETE
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. ping_probe_history_index MUST start at index 1 (rfc2925).
 */
UI32_T PING_POM_GetProbeHistoryEntry(PING_TYPE_PingProbeHistoryEntry_T *prob_history_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = PING_OM_GET_MSG_SIZE(probe_history_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PING_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PING;
    msgbuf_p->msg_size = msg_size;

    msg_p = (PING_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = PING_OM_IPCCMD_GETPROBEHISTORYENTRY;
    memcpy(&(msg_p->data.probe_history_entry), prob_history_entry_p, sizeof(PING_TYPE_PingProbeHistoryEntry_T));

    if (SYSFUN_SendRequestMsg(ping_om_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return PING_TYPE_FAIL;
    }
    
    if(msg_p->type.result_ui32 == PING_TYPE_OK)
    {
        memcpy(prob_history_entry_p, &(msg_p->data.probe_history_entry), sizeof(PING_TYPE_PingProbeHistoryEntry_T));
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME: PING_POM_GetNextProbeHistoryEntry
 * PURPOSE:
 *          To Get next available prob history entry
 * INPUT:
 *          prob_history_entry_p    -- the specific probe history entry, including keys:
 *                                     ping_ctl_owner_index, ping_ctl_test_name, ping_probe_history_index.
 *
 * OUTPUT:
 *          prob_history_entry_p    -- the next probe history entry.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_NO_MORE_ENTRY
 *          PING_TYPE_PROCESS_NOT_COMPLETE
 *          PING_TYPE_FAIL
 * NOTES:
 */
UI32_T PING_POM_GetNextProbeHistoryEntry(PING_TYPE_PingProbeHistoryEntry_T *prob_history_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = PING_OM_GET_MSG_SIZE(probe_history_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PING_OM_IPCMsg_T *msg_p;

   
    msgbuf_p->cmd = SYS_MODULE_PING;
    msgbuf_p->msg_size = msg_size;

    msg_p = (PING_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = PING_OM_IPCCMD_GETNEXTPROBEHISTORYENTRY;
    memcpy(&(msg_p->data.probe_history_entry), prob_history_entry_p, sizeof(PING_TYPE_PingProbeHistoryEntry_T));

    if (SYSFUN_SendRequestMsg(ping_om_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return PING_TYPE_FAIL;
    }
    
    if(msg_p->type.result_ui32 == PING_TYPE_OK)
    {
        memcpy(prob_history_entry_p, &(msg_p->data.probe_history_entry), sizeof(PING_TYPE_PingProbeHistoryEntry_T));
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME: PING_POM_GetNextProbeHistoryEntryForCli
 * PURPOSE:
 *          To Get next available prob history entry
 * INPUT:
 *          prob_history_entry_p    -- the specific probe history entry, including keys:
 *                                     ping_ctl_owner_index, ping_ctl_test_name, ping_probe_history_index.
 *
 * OUTPUT:
 *          prob_history_entry_p    -- the next probe history entry.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_NO_MORE_ENTRY
 *          PING_TYPE_PROCESS_NOT_COMPLETE
 *          PING_TYPE_FAIL
 * NOTES:
 *          Similar to PING_POM_GetNextProbeHistoryEntry() except ping_ctl_owner_index and
 *          ping_ctl_test_name are fixed. Only for CLI use.
 */
UI32_T PING_POM_GetNextProbeHistoryEntryForCli(PING_TYPE_PingProbeHistoryEntry_T *prob_history_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = PING_OM_GET_MSG_SIZE(probe_history_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PING_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PING;
    msgbuf_p->msg_size = msg_size;

    msg_p = (PING_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = PING_OM_IPCCMD_GETNEXTPROBEHISTORYENTRYFORCLI;
    memcpy(&(msg_p->data.probe_history_entry), prob_history_entry_p, sizeof(PING_TYPE_PingProbeHistoryEntry_T));

    if (SYSFUN_SendRequestMsg(ping_om_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return PING_TYPE_FAIL;
    }

    if(msg_p->type.result_ui32 == PING_TYPE_OK)
    {
        memcpy(prob_history_entry_p, &(msg_p->data.probe_history_entry), sizeof(PING_TYPE_PingProbeHistoryEntry_T));
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME: PING_POM_GetResultsEntry
 * PURPOSE:
 *          To the specific result entry base on the given index
 * INPUT:
 *          result_entry_p  -- the specific result entry, including keys:
 *                             ping_ctl_owner_index, ping_ctl_test_name.
 * OUTPUT:
 *          result_entry_p  -- the result entry.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_ENTRY
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T PING_POM_GetResultsEntry(PING_TYPE_PingResultsEntry_T *result_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = PING_OM_GET_MSG_SIZE(results_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PING_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PING;
    msgbuf_p->msg_size = msg_size;

    msg_p = (PING_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = PING_OM_IPCCMD_GETRESULTSENTRY;
    memcpy(&(msg_p->data.results_entry), result_entry_p, sizeof(PING_TYPE_PingResultsEntry_T));

    if (SYSFUN_SendRequestMsg(ping_om_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return PING_TYPE_FAIL;
    }

    if(msg_p->type.result_ui32 == PING_TYPE_OK)
    {
        memcpy(result_entry_p, &(msg_p->data.results_entry), sizeof(PING_TYPE_PingResultsEntry_T));
    }

    return msg_p->type.result_ui32;    
}

/* FUNCTION NAME: PING_POM_GetNextResultsEntry
 * PURPOSE:
 *          To get the next result entry of the specified control entry.
 * INPUT:
 *          result_entry_p  -- the specific result entry, including keys:
 *                             ping_ctl_owner_index, ping_ctl_test_name.
  * OUTPUT:
 *          result_entry_p  -- the next result entry.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_ENTRY
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T PING_POM_GetNextResultsEntry(PING_TYPE_PingResultsEntry_T *result_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = PING_OM_GET_MSG_SIZE(results_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PING_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PING;
    msgbuf_p->msg_size = msg_size;

    msg_p = (PING_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = PING_OM_IPCCMD_GETNEXTRESULTSENTRY;
    memcpy(&(msg_p->data.results_entry), result_entry_p, sizeof(PING_TYPE_PingResultsEntry_T));

    if (SYSFUN_SendRequestMsg(ping_om_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return PING_TYPE_FAIL;
    }

    if(msg_p->type.result_ui32 == PING_TYPE_OK)
    {    
        memcpy(result_entry_p, &(msg_p->data.results_entry), sizeof(PING_TYPE_PingResultsEntry_T));
    }

    return msg_p->type.result_ui32;    
}

