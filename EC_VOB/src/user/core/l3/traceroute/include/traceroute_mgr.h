/* FILE NAME  -  traceroute_mgr.h
 *
 *  ABSTRACT :  This packet provides basic TraceRoute functionality.
 *
 *  NOTES: This package shall be a reusable component of BNBU L2/L3/L4 switch product lines.
 *
 *
 * Modification History:
 *   By            Date      Ver.    Modification Description
 * ------------ ----------   -----   ---------------------------------------
 *   Amytu       2003-07-01          Modify
 * ------------------------------------------------------------------------
 * Copyright(C)                   ACCTON Technology Corp. 2003
 * ------------------------------------------------------------------------
 */


#ifndef _TRACEROUTE_H
#define _TRACEROUTE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "traceroute_type.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define TRACEROUTE_MGR_MSGBUF_TYPE_SIZE sizeof(union TRACEROUTE_MGR_IPCMsg_Type_U)

/* command used in IPC message
 */
enum
{
    TRACEROUTE_MGR_IPCCMD_SETTRACEROUTECTLENTRY = 1,
    TRACEROUTE_MGR_IPCCMD_SETTRACEROUTECTLADMINSTATUS,
    TRACEROUTE_MGR_IPCCMD_SETTRACEROUTECTLROWSTATUS,
    TRACEROUTE_MGR_IPCCMD_SETTRACEROUTECTLTARGETADDRESS,

    TRACEROUTE_MGR_IPCCMD_SETCTLENTRBYFIELD

};

/* MACRO FUNCTION DECLARATIONS
 */
/* Macro function for computation of IPC msg_buf size based on field name
 * used in PING_MGR_IpcMsg_T.data
 */
#define TRACEROUTE_MGR_GET_MSG_SIZE(field_name)                       \
            (TRACEROUTE_MGR_MSGBUF_TYPE_SIZE +                        \
             sizeof(((TRACEROUTE_MGR_IPCMsg_T*)0)->data.field_name))

/* DATA TYPE DECLARATIONS
 */
/* IPC message structure
 */
typedef struct
{
    union TRACEROUTE_MGR_IPCMsg_Type_U
    {
        UI32_T cmd;
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/
        UI32_T result_i32;   /*respond i32 return*/
        SYS_TYPE_Get_Running_Cfg_T result_running_cfg; /* For running config API */
    } type; /* the intended action or return value */

    union
    {
        TRACEROUTE_TYPE_TraceRouteCtlEntry_T          ctl_entry;
//      TRACEROUTE_TYPE_TraceRouteResultsEntry_T      results_entry;
//      TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T probe_history_entry;

        struct {
            char    owner_index[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1];
            UI32_T  owner_index_len;
            char    test_name[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1];
            UI32_T  test_name_len;
            UI32_T  ui32;
        } ctrl_ui32;

        struct {
            char    owner_index[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1];
            UI32_T  owner_index_len;
            char    test_name[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1];
            UI32_T  test_name_len;
            char    target_addr[SYS_ADPT_TRACEROUTE_MAX_IP_ADDRESS_STRING_SIZE + 1];
            UI32_T  target_addr_len;
        } ctrl_target_address;

        struct
        {
            TRACEROUTE_TYPE_TraceRouteCtlEntry_T    ctl_entry;
            UI32_T                                  ui32;
        } ctl_entry_ui32;

    } data; /* the argument(s) for the function corresponding to cmd */
} TRACEROUTE_MGR_IPCMsg_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME : TRACEROUTE_MGR_Initiate_System_Resources
 * PURPOSE:
 *      Initialize working space of trace route utility.
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
 *      1. working space is used to limit amount of ping service could be supported.
 *      2. The max simutanious trace route requests is defined in SYS_BLD.h.
 */
void    TRACEROUTE_MGR_Initiate_System_Resources (void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - TRACEROUTE_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void TRACEROUTE_MGR_Create_InterCSC_Relation(void);

/* FUNCTION NAME : TRACEROUTE_MGR_EnterMasterMode
 * PURPOSE:
 *      TraceRoute enters master mode.
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
 *
 */
void    TRACEROUTE_MGR_EnterMasterMode (void);


/* FUNCTION NAME : TRACEROUTE_MGR_EnterSlaveMode
 * PURPOSE:
 *      TraceRoute enters Slave mode mode.
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
 *      1. In slave mode, just rejects function request and discard incoming message.
 */
void    TRACEROUTE_MGR_EnterSlaveMode (void);


/* FUNCTION NAME : TRACEROUTE_MGR_EnterTransitionMode
 * PURPOSE:
 *      TraceRoute enters Transition mode mode.
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
 *      1. In Transition Mode, must make sure all messages in queue are read
 *         and dynamic allocated space is free, resource set to INIT state.
 */
void    TRACEROUTE_MGR_EnterTransitionMode (void);


/* FUNCTION NAME : TRACEROUTE_MGR_SetTransitionMode
 * PURPOSE:
 *      Make Routing Engine enter transition mode, releasing all allocateing resource in master mode,
 *      discarding TCP/IP configuring requests, and receiving packets.
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
 *      1. In Transition Mode, must make sure all messages in queue are read
 *         and dynamic allocated space is free, resource set to INIT state.
 *      2. All function requests and incoming messages should be dropped.
 */
void TRACEROUTE_MGR_SetTransitionMode(void);


/* FUNCTION NAME - TRACEROUTE_MGR_GetOperationMode
 * PURPOSE  :
 *      This functions returns the current operation mode of this component
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
 *      None. */
SYS_TYPE_Stacking_Mode_T  TRACEROUTE_MGR_GetOperationMode(void);


/* FUNCTION NAME : TRACEROUTE_MGR_CreateSocket
 * PURPOSE:
 *      Create socket for all workspace.
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
 *      None.
 */
UI32_T TRACEROUTE_MGR_CreateSocket(void);


/* FUNCTION NAME : TRACEROUTE_MGR_CloseSocket
 * PURPOSE:
 *      Close socket for all workspace.
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
 *      None.
 */
UI32_T TRACEROUTE_MGR_CloseSocket(void);


/* FUNCTION NAME: TRACEROUTE_MGR_TriggerTraceRoute
 * PURPOSE:
 *          To start trace route periodically
 * INPUT:
 *          None
 * OUTPUT:
 *          None
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 */
UI32_T  TRACEROUTE_MGR_TriggerTraceRoute(void);


/* FUNCTION NAME: TRACEROUTE_MGR_SetTraceRouteCtlEntry
 * PURPOSE:
 *          To create / destroy contrl entry
 * INPUT:
 *          ctl_entry_p->trace_route_ctl_owner_index       -- The owner index of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_owner_index_len   -- The length of the owner index.
 *          ctl_entry_p->trace_route_ctl_test_name         -- The test name of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_test_name_len     -- The length of the test name.
 *          ctl_entry_p->trace_route_ctl_rowstatus         -- The row status of the control entry.
 *          ctl_entry_p->trace_route_ctl_target_address    -- The target address of the traceroute operation.
 *          ctl_entry_p->trace_route_ctl_admin_status      -- The admin status of the traceroute entry.
 * OUTPUT:
 *          None
 * RETURN:
 *          TRACEROUTE_TYPE_OK
 *          TRACEROUTE_TYPE_INVALID_ARG
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          1. Currently, only CAG and Destroy is supported.
 *             Operation for CAW and NotInService is not allowed.
 *          2. Currentlt, we only permit the setting of row_status, target_address and admin_status
 */
UI32_T  TRACEROUTE_MGR_SetTraceRouteCtlEntry(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p);

/* FUNCTION NAME:TRACEROUTE_MGR_GetTraceRouteCtlEntry
 * PURPOSE:
 *          Get the specific traceroute control entry.
 * INPUT:
 *          ctl_entry_p->trace_route_ctl_owner_index       -- The owner index of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_owner_index_len   -- The length of the owner index.
 *          ctl_entry_p->trace_route_ctl_test_name         -- The test name of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_test_name_len     -- The length of the test name.
 * OUTPUT:
 *          ctl_entry_p - the specific control entry if it exists.
 * RETURN:
 *          TRACEROUTE_TYPE_OK
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE
 *          TRACEROUTE_TYPE_INVALID_ARG
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T TRACEROUTE_MGR_GetTraceRouteCtlEntry(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p);

/* FUNCTION NAME:TRACEROUTE_MGR_GetNextTraceRouteCtlEntry
 * PURPOSE:
 *          Get the next traceroute control entry.
 * INPUT:
 *          ctl_entry_p->trace_route_ctl_owner_index       -- The owner index of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_owner_index_len   -- The length of the owner index.
 *          ctl_entry_p->trace_route_ctl_test_name         -- The test name of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_test_name_len     -- The length of the test name.
 * OUTPUT:
 *          ctl_entry_p - the specific control entry if it exists.
 * RETURN:
 *          TRACEROUTE_TYPE_OK
 *          TRACEROUTE_TYPE_INVALID_ARG
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T TRACEROUTE_MGR_GetNextTraceRouteCtlEntry(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p);


/* FUNCTION NAME: TRACEROUTE_MGR_SetTraceRouteCtlAdminStatus
 * PURPOSE:
 *          To enable or disable traceroute control entry
 * INPUT:
 *          ctl_entry_p->trace_route_ctl_owner_index       -- The owner index of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_owner_index_len   -- The length of the owner index.
 *          ctl_entry_p->trace_route_ctl_test_name         -- The test name of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_test_name_len     -- The length of the test name.
 *          ctl_entry_p->trace_route_ctl_admin_status      -- The admin status of the traceroute entry.
 * OUTPUT:
 *          None
 * RETURN:
 *          TRACEROUTE_TYPE_OK
 *          TRACEROUTE_TYPE_INVALID_ARG
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          1.This API is only used by "set by filed", not "set by record".
 */
UI32_T TRACEROUTE_MGR_SetTraceRouteCtlAdminStatus(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p);

/* FUNCTION NAME: TRACEROUTE_MGR_SetTraceRouteCtlRowStatus
 * PURPOSE:
 *          To set row status field for traceroute control entry
 * INPUT:
 *          ctl_entry_p->trace_route_ctl_owner_index       -- The owner index of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_owner_index_len   -- The length of the owner index.
 *          ctl_entry_p->trace_route_ctl_test_name         -- The test name of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_test_name_len     -- The length of the test name.
 *          ctl_entry_p->trace_route_ctl_rowstatus         -- The row status of the control entry.
 * OUTPUT:
 *          None
 * RETURN:
 *          TRACEROUTE_TYPE_OK
 *          TRACEROUTE_TYPE_INVALID_ARG
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 */
UI32_T TRACEROUTE_MGR_SetTraceRouteCtlRowStatus(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p);


/* FUNCTION NAME: TRACEROUTE_MGR_SetTraceRouteCtlTargetAddress
 * PURPOSE:
 *          To set the target address field for traceroute control entry
 * INPUT:
 *          ctl_entry_p->trace_route_ctl_owner_index       -- The owner index of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_owner_index_len   -- The length of the owner index.
 *          ctl_entry_p->trace_route_ctl_test_name         -- The test name of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_test_name_len     -- The length of the test name.
 *          ctl_entry_p->trace_route_ctl_target_address    -- The target address of the traceroute operation.
 * OUTPUT:
 *          None
 * RETURN:
 *          TRACEROUTE_TYPE_OK
 *          TRACEROUTE_TYPE_INVALID_ARG
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          1. We can not change the target address for the specified index when admin_status is enabled.
 *          2. Currently we do not support the domain name query of the target address.
 */
UI32_T TRACEROUTE_MGR_SetTraceRouteCtlTargetAddress(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p);

#if 0
/* FUNCTION NAME: TRACEROUTE_MGR_GetNextTraceRouteProbeHistoryEntry
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
UI32_T  TRACEROUTE_MGR_GetNextTraceRouteProbeHistoryEntry(UI8_T  *owner_index_p,
                                                          UI32_T owner_index_len,
                                                          UI8_T  *test_name_p,
                                                          UI32_T test_name_len,
                                                          UI32_T history_index,
                                                          UI32_T history_hop_index,
                                                          UI32_T history_probe_index,
                                                          TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T  *prob_history_entry_p);
#endif
/* FUNCTION NAME: TRACEROUTE_MGR_GetNextTraceRouteProbeHistoryEntryForCLIWEB
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
UI32_T  TRACEROUTE_MGR_GetNextTraceRouteProbeHistoryEntryForCLIWEB(UI8_T  *owner_index_p,
                                                          UI32_T owner_index_len,
                                                          UI8_T  *test_name_p,
                                                          UI32_T test_name_len,
                                                          UI32_T history_index,
                                                          UI32_T history_hop_index,
                                                          UI32_T history_probe_index,
                                                          TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T  *prob_history_entry_p);

/* FUNCTION NAME : TRACEROUTE_MGR_CreateWorkSpace
 * PURPOSE:
 *      Create workspace for routing path from src to dst.
 *
 * INPUT:
 *      dst_ip : probe target IP address.
 *      src_ip : interface IP address which send out probe packet.
 *      workspace_index: Location of this entry
 * OUTPUT:
 *      None.
 * RETURN:
 *      TRACEROUTE_TYPE_OK  -- successfully create the workspace.
 *      TRACEROUTE_TYPE_INVALID_ARG -- src_ip or dst_ip is invalid value.
 *      TRACEROUTE_TYPE_NO_MORE_WORKSPACE
 *      TRACEROUTE_TYPE_NO_MORE_SOCKET
 *      TRACEROUTE_TYPE_NO_MORE_ENTRY
 *      TRACEROUTE_TYPE_FAIL
 * NOTES:
 *      1. If same task create workspace twice, cause previous one auto-free.
 */
UI32_T TRACEROUTE_MGR_CreateWorkSpace(UI32_T workspace_index, TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p);

/* FUNCTION NAME : TRACEROUTE_MGR_FreeWorkSpace
 * PURPOSE:
 *      Release working space to trace route utility.
 *
 * INPUT:
 *      workspace_index - the starting address of workspace handler.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRACEROUTE_TYPE_OK - the space is return to utility.
 *      TRACEROUTE_TYPE_INVALID_WORK_SPACE - the pointer is no valid pointer of working space,
 *                                     maybe not owned by this task.
 *
 * NOTES:
 *      1. After free workspace, handler will set to NULL.
 */
UI32_T  TRACEROUTE_MGR_FreeWorkSpace (UI32_T  workspace_index);

/* FUNCTION NAME: TRACEROUTE_MGR_SetWorkSpaceAdminStatus
 * PURPOSE:
 *          Sync admin status in the work space.
 * INPUT:
 *          table_index - table index to be synchronize.
 *          status - {VAL_traceRouteCtlAdminStatus_disabled|VAL_traceRouteCtlAdminStatus_enabled}.
 * OUTPUT:
 *          None
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *
 */
UI32_T TRACEROUTE_MGR_SetWorkSpaceAdminStatus(UI32_T table_index, UI32_T status);

/* FUNCTION NAME: TRACEROUTE_MGR_GetTraceRouteResultsEntry
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
UI32_T  TRACEROUTE_MGR_GetTraceRouteResultsEntry(UI8_T  *owner_index_p,
                                                 UI32_T owner_index_len,
                                                 UI8_T  *test_name_p,
                                                 UI32_T test_name_len,
                                                 TRACEROUTE_TYPE_TraceRouteResultsEntry_T *result_entry_p);

/* FUNCTION NAME: TRACEROUTE_MGR_GetNextTraceRouteResultsEntry
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
UI32_T  TRACEROUTE_MGR_GetNextTraceRouteResultsEntry(UI8_T  *owner_index_p,
                                                     UI32_T owner_index_len,
                                                     UI8_T  *test_name_p,
                                                     UI32_T test_name_len,
                                                     TRACEROUTE_TYPE_TraceRouteResultsEntry_T *result_entry_p);

/* FUNCTION NAME: TRACEROUTE_MGR_GetTraceRouteProbeHistoryEntry
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
UI32_T  TRACEROUTE_MGR_GetTraceRouteProbeHistoryEntry(UI8_T  *owner_index_p,
                                                      UI32_T owner_index_len,
                                                      UI8_T  *test_name_p,
                                                      UI32_T test_name_len,
                                                      UI32_T history_index,
                                                      UI32_T history_hop_index,
                                                      UI32_T history_probe_index,
                                                      TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T  *prob_history_entry_p);



/* FUNCTION NAME : TRACEROUTE_MGR_HandleIPCReqMsg
 * PURPOSE:
 *    Handle the ipc request message for TRACEROUTE_MGR.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *
 */
BOOL_T TRACEROUTE_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

#endif

