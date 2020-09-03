/* Module Name: traceroute_pom.c
 * Purpose:
 *    Implements the APIs for IPCs with TRACEROUTE OM.
 *
 * Notes:
 *    None.
 * History:
 *      Date        --  Modifier,   Reason
 *      2007/12/19  --  Peter Yu,   Create
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
#include "traceroute_om.h"
#include "traceroute_pom.h"
#include "traceroute_type.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* trace id definition when using L_MM
 */
enum
{
    TRACEROUTE_TYPE_TRACEID_POM_GETCTLENTRY
};


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DEFINITIONS
 */

static SYSFUN_MsgQ_T traceroute_om_ipcmsgq_handle;


/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: TRACEROUTE_POM_InitiateProcessResources
 * PURPOSE:
 *          Initiate resources for TRACEROUTE_POM in the calling process.
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
BOOL_T TRACEROUTE_POM_InitiateProcessResources(void)
{
    /* get the ipc message queues for TRACEROUTE OM
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_APP_PROTOCOL_PROC_OM_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &traceroute_om_ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\n%s(): L_IPCMSGQ_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
} /* End of TRACEROUTE_POM_InitiateProcessResources */

/* FUNCTION NAME:TRACEROUTE_POM_GetTraceRouteCtlEntry
 * PURPOSE:
 *          Get the specific traceroute control entry.
 * INPUT:
 *          owner_index_p - The owner index of the trace route operation.
 *          owner_index_len - The length of the owner index.
 *          test_name_p - The test name of the trace route operation.
 *          test_name_len - The length of the test name.
 * OUTPUT:
 *          ctrl_entry_p - the specific control entry if it exists.
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE \
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T  TRACEROUTE_POM_GetTraceRouteCtlEntry(char  *owner_index_p,
                                             UI32_T owner_index_len,
                                             char  *test_name_p,
                                             UI32_T test_name_len,
                                             TRACEROUTE_TYPE_TraceRouteCtlEntry_T  *ctrl_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRACEROUTE_OM_GET_MSG_SIZE(ctrl_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRACEROUTE_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRACEROUTE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRACEROUTE_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRACEROUTE_OM_IPCCMD_GETTRACEROUTECTLENTRY;
    memcpy(&msg_p->data.ctrl_entry.trace_route_ctl_owner_index, owner_index_p, owner_index_len);
    msg_p->data.ctrl_entry.trace_route_ctl_owner_index_len = owner_index_len;
    memcpy(&msg_p->data.ctrl_entry.trace_route_ctl_test_name, test_name_p, test_name_len);
    msg_p->data.ctrl_entry.trace_route_ctl_test_name_len = test_name_len;

    if (SYSFUN_SendRequestMsg(traceroute_om_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return TRACEROUTE_TYPE_FAIL;
    }
    memcpy(ctrl_entry_p, &msg_p->data.ctrl_entry, sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));
    return msg_p->type.result_ui32;
}


/* FUNCTION NAME:TRACEROUTE_POM_GetNextTraceRouteCtlEntry
 * PURPOSE:
 *          Get the next traceroute control entry.
 * INPUT:
 *          owner_index_p - The owner index of the trace route operation.
 *          owner_index_len - The length of the owner index.
 *          test_name_p - The test name of the trace route operation.
 *          test_name_len - The length of the test name.
 * OUTPUT:
 *          ctrl_entry_p - the specific control entry if it exists.
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T  TRACEROUTE_POM_GetNextTraceRouteCtlEntry(char  *owner_index_p,
                                                 UI32_T owner_index_len,
                                                 char  *test_name_p,
                                                 UI32_T test_name_len,
                                                 TRACEROUTE_TYPE_TraceRouteCtlEntry_T  *ctrl_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRACEROUTE_OM_GET_MSG_SIZE(ctrl_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRACEROUTE_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRACEROUTE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRACEROUTE_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRACEROUTE_OM_IPCCMD_GETNEXTTRACEROUTECTLENTRY;
    memcpy(&msg_p->data.ctrl_entry.trace_route_ctl_owner_index, owner_index_p, owner_index_len);
    msg_p->data.ctrl_entry.trace_route_ctl_owner_index_len = owner_index_len;
    memcpy(&msg_p->data.ctrl_entry.trace_route_ctl_test_name, test_name_p, test_name_len);
    msg_p->data.ctrl_entry.trace_route_ctl_test_name_len = test_name_len;

    if (SYSFUN_SendRequestMsg(traceroute_om_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return TRACEROUTE_TYPE_FAIL;
    }
    memcpy(ctrl_entry_p, &msg_p->data.ctrl_entry, sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));
    return msg_p->type.result_ui32;
}


/* FUNCTION NAME: TRACEROUTE_POM_GetTraceRouteResultsEntry
 * PURPOSE:
 *          To the specific result entry base on the given index
 * INPUT:
 *          owner_index_p - The owner index of the trace route operation.
 *          owner_index_len - The length of the owner index.
 *          test_name_p - The test name of the trace route operation.
 *          test_name_len - The length of the test name.
 * OUTPUT:
 *          result_entry_p - the result entry.
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T  TRACEROUTE_POM_GetTraceRouteResultsEntry(char  *owner_index_p,
                                                 UI32_T owner_index_len,
                                                 char  *test_name_p,
                                                 UI32_T test_name_len,
                                                 TRACEROUTE_TYPE_TraceRouteResultsEntry_T *result_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRACEROUTE_OM_GET_MSG_SIZE(results_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRACEROUTE_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRACEROUTE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRACEROUTE_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRACEROUTE_OM_IPCCMD_GETTRACEROUTERESULTSENTRY;
    memcpy(&msg_p->data.results_entry.trace_route_ctl_owner_index, owner_index_p, owner_index_len);
    msg_p->data.results_entry.trace_route_ctl_owner_index_len = owner_index_len;
    memcpy(&msg_p->data.results_entry.trace_route_ctl_test_name, test_name_p, test_name_len);
    msg_p->data.results_entry.trace_route_ctl_test_name_len = test_name_len;

    if (SYSFUN_SendRequestMsg(traceroute_om_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return TRACEROUTE_TYPE_FAIL;
    }
    memcpy(result_entry_p, &msg_p->data.results_entry, sizeof(TRACEROUTE_TYPE_TraceRouteResultsEntry_T));
    return msg_p->type.result_ui32;
}


/* FUNCTION NAME: TRACEROUTE_POM_GetNextTraceRouteResultsEntry
 * PURPOSE:
 *          To get the next result entry of the specified control entry.
 * INPUT:
 *          owner_index_p - The owner index of the trace route operation.
 *          owner_index_len - The length of the owner index.
 *          test_name_p - The test name of the trace route operation.
 *          test_name_len - The length of the test name.
 * OUTPUT:
 *          result_entry_p - the next result entry.
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T  TRACEROUTE_POM_GetNextTraceRouteResultsEntry(char  *owner_index_p,
                                                     UI32_T owner_index_len,
                                                     char  *test_name_p,
                                                     UI32_T test_name_len,
                                                     TRACEROUTE_TYPE_TraceRouteResultsEntry_T *result_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRACEROUTE_OM_GET_MSG_SIZE(results_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRACEROUTE_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRACEROUTE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRACEROUTE_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRACEROUTE_OM_IPCCMD_GETNEXTTRACEROUTERESULTSENTRY;
    memcpy(&msg_p->data.results_entry.trace_route_ctl_owner_index, owner_index_p, owner_index_len);
    msg_p->data.results_entry.trace_route_ctl_owner_index_len = owner_index_len;
    memcpy(&msg_p->data.results_entry.trace_route_ctl_test_name, test_name_p, test_name_len);
    msg_p->data.results_entry.trace_route_ctl_test_name_len = test_name_len;

    if (SYSFUN_SendRequestMsg(traceroute_om_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return TRACEROUTE_TYPE_FAIL;
    }
    memcpy(result_entry_p, &msg_p->data.results_entry, sizeof(TRACEROUTE_TYPE_TraceRouteResultsEntry_T));
    return msg_p->type.result_ui32;
}


/* FUNCTION NAME: TRACEROUTE_POM_GetTraceRouteProbeHistoryEntry
 * PURPOSE:
 *          To get the specific probe history entry base on the given index
 * INPUT:
 *          owner_index_p - The owner index of the trace route operation.
 *          owner_index_len - The length of the owner index.
 *          test_name_p - The test name of the trace route operation.
 *          test_name_len - The length of the test name.
 *          history_index - The history index of the TraceRouteProbeHistoryEntry.
 *          history_hop_index - The hop index of the TraceRouteProbeHistoryEntry.
 *          history_probe_index - The probe index of the TraceRouteProbeHistoryEntry.
 * OUTPUT:
 *          prob_history_entry_p - the probe history entry.
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_PROCESS_NOT_COMPLETE \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T  TRACEROUTE_POM_GetTraceRouteProbeHistoryEntry(char  *owner_index_p,
                                                      UI32_T owner_index_len,
                                                      char  *test_name_p,
                                                      UI32_T test_name_len,
                                                      UI32_T history_index,
                                                      UI32_T history_hop_index,
                                                      UI32_T history_probe_index,
                                                      TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T  *prob_history_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRACEROUTE_OM_GET_MSG_SIZE(probe_history_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRACEROUTE_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRACEROUTE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRACEROUTE_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRACEROUTE_OM_IPCCMD_GETTRACEROUTEPROBEHISTORYENTRY;
    memcpy(&msg_p->data.probe_history_entry.trace_route_ctl_owner_index, owner_index_p, owner_index_len);
    msg_p->data.probe_history_entry.trace_route_ctl_owner_index_len = owner_index_len;
    memcpy(&msg_p->data.probe_history_entry.trace_route_ctl_test_name, test_name_p, test_name_len);
    msg_p->data.probe_history_entry.trace_route_ctl_test_name_len = test_name_len;
    msg_p->data.probe_history_entry.trace_route_probe_history_index = history_index;
    msg_p->data.probe_history_entry.trace_route_probe_history_hop_index = history_hop_index;
    msg_p->data.probe_history_entry.trace_route_probe_history_probe_index = history_probe_index;

    if (SYSFUN_SendRequestMsg(traceroute_om_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return TRACEROUTE_TYPE_FAIL;
    }
    memcpy(prob_history_entry_p, &msg_p->data.probe_history_entry, sizeof(TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T));
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME: TRACEROUTE_POM_GetNextTraceRouteProbeHistoryEntry
 * PURPOSE:
 *          To Get next available prob history entry
 * INPUT:
 *          owner_index_p - The owner index of the trace route operation.
 *          owner_index_len - The length of the owner index.
 *          test_name_p - The test name of the trace route operation.
 *          test_name_len - The length of the test name.
 *          history_index - The history index of the TraceRouteProbeHistoryEntry.
 *          history_hop_index - The hop index of the TraceRouteProbeHistoryEntry.
 *          history_probe_index - The probe index of the TraceRouteProbeHistoryEntry.
 * OUTPUT:
 *          prob_history_entry_p - the next probe history entry.
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \
 *          TRACEROUTE_TYPE_PROCESS_NOT_COMPLETE \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 */
UI32_T  TRACEROUTE_POM_GetNextTraceRouteProbeHistoryEntry(char  *owner_index_p,
                                                          UI32_T owner_index_len,
                                                          char  *test_name_p,
                                                          UI32_T test_name_len,
                                                          UI32_T history_index,
                                                          UI32_T history_hop_index,
                                                          UI32_T history_probe_index,
                                                          TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T  *prob_history_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRACEROUTE_OM_GET_MSG_SIZE(probe_history_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRACEROUTE_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRACEROUTE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRACEROUTE_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRACEROUTE_OM_IPCCMD_GETNEXTTRACEROUTEPROBEHISTORYENTRY;
    memcpy(&msg_p->data.probe_history_entry.trace_route_ctl_owner_index, owner_index_p, owner_index_len);
    msg_p->data.probe_history_entry.trace_route_ctl_owner_index_len = owner_index_len;
    memcpy(&msg_p->data.probe_history_entry.trace_route_ctl_test_name, test_name_p, test_name_len);
    msg_p->data.probe_history_entry.trace_route_ctl_test_name_len = test_name_len;
    msg_p->data.probe_history_entry.trace_route_probe_history_index = history_index;
    msg_p->data.probe_history_entry.trace_route_probe_history_hop_index = history_hop_index;
    msg_p->data.probe_history_entry.trace_route_probe_history_probe_index = history_probe_index;

    if (SYSFUN_SendRequestMsg(traceroute_om_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return TRACEROUTE_TYPE_FAIL;
    }
    memcpy(prob_history_entry_p, &msg_p->data.probe_history_entry, sizeof(TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T));
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME: TRACEROUTE_POM_GetNextTraceRouteProbeHistoryEntryForCLIWEB
 * PURPOSE:
 *          To Get next available prob history entry
 * INPUT:
 *          owner_index_p - The owner index of the trace route operation.
 *          owner_index_len - The length of the owner index.
 *          test_name_p - The test name of the trace route operation.
 *          test_name_len - The length of the test name.
 *          history_index - The history index of the TraceRouteProbeHistoryEntry.
 *          history_hop_index - The hop index of the TraceRouteProbeHistoryEntry.
 *          history_probe_index - The probe index of the TraceRouteProbeHistoryEntry.
 *
 * OUTPUT:
 *          prob_history_entry_p- next available entry that contains prob packet information
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \
 *          TRACEROUTE_TYPE_PROCESS_NOT_COMPLETE \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T  TRACEROUTE_POM_GetNextTraceRouteProbeHistoryEntryForCLIWEB(char  *owner_index_p,
                                                                  UI32_T owner_index_len,
                                                                  char  *test_name_p,
                                                                  UI32_T test_name_len,
                                                                  UI32_T history_index,
                                                                  UI32_T history_hop_index,
                                                                  UI32_T history_probe_index,
                                                                  TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T  *prob_history_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRACEROUTE_OM_GET_MSG_SIZE(probe_history_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRACEROUTE_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRACEROUTE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRACEROUTE_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRACEROUTE_OM_IPCCMD_GETNEXTTRACEROUTEPROBEHISTORYENTRYFORCLIWEB;
    memcpy(&msg_p->data.probe_history_entry.trace_route_ctl_owner_index, owner_index_p, owner_index_len);
    msg_p->data.probe_history_entry.trace_route_ctl_owner_index_len = owner_index_len;
    memcpy(&msg_p->data.probe_history_entry.trace_route_ctl_test_name, test_name_p, test_name_len);
    msg_p->data.probe_history_entry.trace_route_ctl_test_name_len = test_name_len;
    msg_p->data.probe_history_entry.trace_route_probe_history_index = history_index;
    msg_p->data.probe_history_entry.trace_route_probe_history_hop_index = history_hop_index;
    msg_p->data.probe_history_entry.trace_route_probe_history_probe_index = history_probe_index;

    if (SYSFUN_SendRequestMsg(traceroute_om_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return TRACEROUTE_TYPE_FAIL;
    }
    memcpy(prob_history_entry_p, &msg_p->data.probe_history_entry, sizeof(TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T));
    return msg_p->type.result_ui32;
}

