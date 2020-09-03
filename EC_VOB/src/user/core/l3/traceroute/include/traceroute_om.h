/* -------------------------------------------------------------------------
 * FILE NAME - traceroute_om.h
 * -------------------------------------------------------------------------
 * Purpose: This package provides the sevices to manage/support the RFC2925 MIB
 * Notes: 1. This package shall be a reusable package for all the L2/L3 switchs.
 *
 *
 * Modification History:
 *   By            Date      Ver.    Modification Description
 * ------------ ----------   -----   ---------------------------------------
 *   Amytu       2003-07-01          First Created
 * -------------------------------------------------------------------------
 * Copyright(C)         ACCTON Technology Corp., 2003
 * -------------------------------------------------------------------------
 */

#ifndef _TRACEROUTE_OM_H
#define _TRACEROUTE_OM_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "traceroute_type.h"


/* NAMING CONSTANT DECLARATIONS
 */
#define TRACEROUTE_OM_MSGBUF_TYPE_SIZE sizeof(union TRACEROUTE_OM_IPCMsg_Type_U)

/* command used in IPC message
 */
enum
{
    TRACEROUTE_OM_IPCCMD_GETTRACEROUTECTLENTRY = 1,
    TRACEROUTE_OM_IPCCMD_GETNEXTTRACEROUTECTLENTRY,
    TRACEROUTE_OM_IPCCMD_GETTRACEROUTERESULTSENTRY,
    TRACEROUTE_OM_IPCCMD_GETNEXTTRACEROUTERESULTSENTRY,
    TRACEROUTE_OM_IPCCMD_GETTRACEROUTEPROBEHISTORYENTRY,
    TRACEROUTE_OM_IPCCMD_GETNEXTTRACEROUTEPROBEHISTORYENTRY,
    TRACEROUTE_OM_IPCCMD_GETNEXTTRACEROUTEPROBEHISTORYENTRYFORCLIWEB,
};


/* MACRO FUNCTION DECLARATIONS
 */
/* Macro function for computation of IPC msg_buf size based on field name
 * used in PING_MGR_IpcMsg_T.data
 */
#define TRACEROUTE_OM_GET_MSG_SIZE(field_name)                       \
            (TRACEROUTE_OM_MSGBUF_TYPE_SIZE +                        \
             sizeof(((TRACEROUTE_OM_IPCMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */
/* IPC message structure
 */
typedef struct
{
    union TRACEROUTE_OM_IPCMsg_Type_U
    {
        UI32_T cmd;
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/
        UI32_T result_i32;   /*respond i32 return*/
        SYS_TYPE_Get_Running_Cfg_T result_running_cfg; /* For running config API */
    } type; /* the intended action or return value */

    union
    {
        TRACEROUTE_TYPE_TraceRouteCtlEntry_T          ctrl_entry;
        TRACEROUTE_TYPE_TraceRouteResultsEntry_T      results_entry;
        TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T probe_history_entry;
    } data; /* the argument(s) for the function corresponding to cmd */
} TRACEROUTE_OM_IPCMsg_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME : TRACEROUTE_OM_Initiate_System_Resources
 * PURPOSE:
 *      Initialize Traceroute OM
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None
 */
void TRACEROUTE_OM_Initiate_System_Resources(void);

/* FUNCTION NAME : TRACEROUTE_OM_ClearOM
 * PURPOSE:
 *      Clear Traceroute OM
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None
 */
void TRACEROUTE_OM_ClearOM (void);


/* FUNCTION NAME : TRACEROUTE_OM_EnterMasterMode
 * PURPOSE:
 *      Assign default value Traceroute OM
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None
 */
void TRACEROUTE_OM_EnterMasterMode (void);


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:TRACEROUTE_OM_GetTraceRouteCtlEntry
 * PURPOSE:
 *          Get the specific traceroute control entry.
 * INPUT:
 *          ctrl_entry_p->trace_route_ctl_owner_index - The owner index of the trace route operation.
 *          ctrl_entry_p->trace_route_ctl_owner_index_len - The length of the owner index.
 *          ctrl_entry_p->trace_route_ctl_test_name - The test name of the trace route operation.
 *          ctrl_entry_p->trace_route_ctl_test_name_len - The length of the test name.
 * OUTPUT:
 *          ctrl_entry_p - the specific control entry if it exists.
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T  TRACEROUTE_OM_GetTraceRouteCtlEntry(TRACEROUTE_TYPE_TraceRouteCtlEntry_T  *ctrl_entry_p);

/* FUNCTION NAME: TRACEROUTE_OM_GetTraceRouteProbeHistoryEntry
 * PURPOSE:
 *          To get the specified available prob history entry
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
 *          prob_history_entry_p - entry that contains prob packet information
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \
 *          TRACEROUTE_TYPE_PROCESS_NOT_COMPLETE \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 */
UI32_T  TRACEROUTE_OM_GetTraceRouteProbeHistoryEntry(char  *owner_index_p,
                                                     UI32_T owner_index_len,
                                                     char  *test_name_p,
                                                     UI32_T test_name_len,
                                                     UI32_T history_index,
                                                     UI32_T history_hop_index,
                                                     UI32_T history_probe_index,
                                                     TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T  *prob_history_entry_p);


/* FUNCTION NAME: TRACEROUTE_OM_GetNextTraceRouteProbeHistoryEntry
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
UI32_T  TRACEROUTE_OM_GetNextTraceRouteProbeHistoryEntry(char  *owner_index_p,
                                                         UI32_T owner_index_len,
                                                         char  *test_name_p,
                                                         UI32_T test_name_len,
                                                         UI32_T history_index,
                                                         UI32_T history_hop_index,
                                                         UI32_T history_probe_index,
                                                         TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T  *prob_history_entry_p);

/* FUNCTION NAME: TRACEROUTE_OM_GetNextTraceRouteProbeHistoryEntryForCLIWEB
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
UI32_T  TRACEROUTE_OM_GetNextTraceRouteProbeHistoryEntryForCLIWEB(char  *owner_index_p,
                                                                  UI32_T owner_index_len,
                                                                  char  *test_name_p,
                                                                  UI32_T test_name_len,
                                                                  UI32_T history_index,
                                                                  UI32_T history_hop_index,
                                                                  UI32_T history_probe_index,
                                                                  TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T  *prob_history_entry_p);

/* FUNCTION NAME: TRACEROUTE_OM_GetTraceRouteResultsEntry
 * PURPOSE:
 *          To the specific result entry base on the given index
 * INPUT:
 *          result_entry_p->trace_route_ctl_owner_index - The owner index of the trace route operation.
 *          result_entry_p->trace_route_ctl_owner_index_len - The length of the owner index.
 *          result_entry_p->trace_route_ctl_test_name - The test name of the trace route operation.
 *          result_entry_p->trace_route_ctl_test_name_len - The length of the test name
 * OUTPUT:
 *          result_entry_p - contains values to get to result entry.
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T  TRACEROUTE_OM_GetTraceRouteResultsEntry(TRACEROUTE_TYPE_TraceRouteResultsEntry_T *result_entry_p);

#if 0 //peter, no caller
/* FUNCTION NAME: TRACEROUTE_OM_GetFirstResultsEntry
 * PURPOSE:
 *          To get the first active result entry
 * INPUT:
 *          None.
 * OUTPUT:
 *          result_entry_p - contains values to set to prob history entry.
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T  TRACEROUTE_OM_GetFirstResultsEntry(TRACEROUTE_TYPE_TraceRouteResultsEntry_T *result_entry_p);
#endif

/* FUNCTION NAME: TRACEROUTE_OM_GetNextResultsEntry
 * PURPOSE:
 *          To get the next result entry
 * INPUT:
 *          result_entry_p->trace_route_ctl_owner_index - The owner index of the trace route operation.
 *          result_entry_p->trace_route_ctl_owner_index_len - The length of the owner index.
 *          result_entry_p->trace_route_ctl_test_name - The test name of the trace route operation.
 *          result_entry_p->trace_route_ctl_test_name_len - The length of the test name
 * OUTPUT:
 *          result_entry_p - next available result entry
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T  TRACEROUTE_OM_GetNextResultsEntry(TRACEROUTE_TYPE_TraceRouteResultsEntry_T *result_entry_p);

/* FUNCTION NAME:TRACEROUTE_OM_GetNextTraceRouteCtlEntry
 * PURPOSE:
 *          To get the next traceroute control entry.
 * INPUT:
 *          ctrl_entry_p->trace_route_ctl_owner_index - The owner index of the trace route operation.
 *          ctrl_entry_p->trace_route_ctl_owner_index_len - The length of the owner index.
 *          ctrl_entry_p->trace_route_ctl_test_name - The test name of the trace route operation.
 *          ctrl_entry_p->trace_route_ctl_test_name_len - The length of the test name.
 * OUTPUT:
 *          ctrl_entry_p - the next control entry if it exists.
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T  TRACEROUTE_OM_GetNextTraceRouteCtlEntry(TRACEROUTE_TYPE_TraceRouteCtlEntry_T  *ctrl_entry_p);

/* FUNCTION NAME: TRACEROUTE_MGR_OM_DisplayAllProbeHistory
 * PURPOSE:
 *          Print all probe history entry.
 * INPUT:
 *          None.
 * OUTPUT:
 *          None.
 * RETURN:
 *          None.
 * NOTES:
 *          1. For debug purpose.
 */
void TRACEROUTE_MGR_OM_DisplayAllProbeHistory(void);

/* FUNCTION NAME : TRACEROUTE_OM_HandleIPCReqMsg
 * PURPOSE:
 *    Handle the ipc request message for TRACEROUTE om.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  --  There is a response need to send.
 *    FALSE --  No response need to send.
 *
 * NOTES:
 *    1.The size of ipcmsg_p.msg_buf must be large enough to carry any response
 *      messages.
 *
 */
BOOL_T TRACEROUTE_OM_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);


#endif
