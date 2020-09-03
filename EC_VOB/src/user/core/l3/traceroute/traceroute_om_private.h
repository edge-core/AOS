/* -------------------------------------------------------------------------
 * FILE NAME - traceroute_om_private.h
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

#ifndef _TRACEROUTE_OM_PRIAVTE_H
#define _TRACEROUTE_OM_PRIAVTE_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "traceroute_type.h"


/* TYPE DECLARATIONS
 */
/* FUNCTION NAME : TRACEROUTE_OM_Init
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
void TRACEROUTE_OM_Init (void);

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
/* FUNCTION NAME:TRACEROUTE_OM_SetTraceRouteCtlEntry
 * PURPOSE:
 *          To create / destroy contrl entry
 * INPUT:
 *          ctrl_entry.trace_route_ctl_owner_index - The owner index of the trace route operation.
 *          ctrl_entry.trace_route_ctl_owner_index_len - The length of the owner index.
 *          ctrl_entry.trace_route_ctl_test_name - The test name of the trace route operation.
 *          ctrl_entry.trace_route_ctl_test_name_len - The length of the test name.
 *          ctrl_entry.trace_route_ctl_rowstatus - The row status of the control entry.
 *          ctrl_entry.trace_route_ctl_target_address - The target address of the traceroute operation.
 *          ctrl_entry.trace_route_ctl_admin_status - The admin status of the traceroute entry.
 * OUTPUT:
 *          None
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          1. Currently, only CAG and Destroy is supported.
 *             Operation for CAW and NotInService is not allowed.
 *          2. Currentlt, we only permit the setting of row_status, target_address and admin_status
 */
UI32_T  TRACEROUTE_OM_SetTraceRouteCtlEntry(TRACEROUTE_TYPE_TraceRouteCtlEntry_T  ctrl_entry);


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


/* FUNCTION NAME: TRACEROUTE_OM_AppendProbePacketResult
 * PURPOSE:
 *          To append specific prob histroy entry to the end of the last histroy entry
 * INPUT:
 *          keys - A key index to identify prob history entry for different owner_index and test name
 *          prob_history_entry - contains values to set to prob history entry.
 *
 * OUTPUT:
 *          None.
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T  TRACEROUTE_OM_AppendProbePacketResult(UI32_T keys, TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T  prob_packet_entry);

/* FUNCTION NAME: TRACEROUTE_OM_SetTraceRouteResultsEntry
 * PURPOSE:
 *          To set specific traceroute result entry by the specific entry index
 * INPUT:
 *          key_index - the index of the result table.
 *          result_entry - contains values to set to prob history entry.
 *
 * OUTPUT:
 *          None.
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T  TRACEROUTE_OM_SetTraceRouteResultsEntry(UI32_T  key_index,
                                                TRACEROUTE_TYPE_TraceRouteResultsEntry_T   result_entry);


/* FUNCTION NAME: TRACEROUTE_OM_GetFirstActiveResultsEntry
 * PURPOSE:
 *          To get the first active result entry which oper status is enabled.
 * INPUT:
 *          index_p - Index to the result entry.
 * OUTPUT:
 *          result_entry_p - contains values to set to prob history entry.
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T  TRACEROUTE_OM_GetFirstActiveResultsEntry(UI32_T *index_p, TRACEROUTE_TYPE_TraceRouteResultsEntry_T *result_entry_p);


/* FUNCTION NAME: TRACEROUTE_OM_GetNextActiveResultsEntry
 * PURPOSE:
 *          To get next active result entry which oper status is enabled.
 * INPUT:
 *          index_p - Index to the result entry.
 * OUTPUT:
 *          index_p - Index to the result entry.
 *          result_entry_p - next available active result entry
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T  TRACEROUTE_OM_GetNextActiveResultsEntry(UI32_T *index_p, TRACEROUTE_TYPE_TraceRouteResultsEntry_T *result_entry_p);

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

/* UTILITY FUNCTIONS
 */
/* FUNCTION NAME: TRACEROUTE_OM_IsTraceRouteControlEntryExist
 * PURPOSE:
 *          To check if the specific control entry information exist
 *          in OM or not.
 * INPUT:
 *          owner_index_p - The owner index of the trace route operation.
 *          owner_index_len - The length of the owner index.
 *          test_name_p - The test name of the trace route operation.
 *          test_name_len - The length of the test name.
 * OUTPUT:
 *          None.
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T TRACEROUTE_OM_IsTraceRouteControlEntryExist(char *owner_index_p, UI32_T owner_index_len,
                                                   char *test_name_p, UI32_T test_name_len);

/* FUNCTION NAME: TRACEROUTE_OM_TraceRouteKeyToTableIndex
 * PURPOSE:
 *          Find the table index from the key of the table.
 * INPUT:
 *          elm_p->owner_index - owner index is the task name
 *          elm_p->owner_index_len - the length of the owner index.
 *          elm_p->test_name - the specific test session name
 *          elm_p->test_name_len - the length of the test name index.
 * OUTPUT:
 *          table_index_p - table indeex
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T TRACEROUTE_OM_TraceRouteKeyToTableIndex(TRACEROUTE_SORTLST_ELM_T *elm_p, UI32_T *table_index_p);

/* FUNCTION NAME: TRACEROUTE_OM_TraceRouteKeyToTableIndex
 * PURPOSE:
 *          Find the key of the table from the table index.
 * INPUT:
 *          table_index - table index
 * OUTPUT:
 *          elm_p - includes owner_index, owner_index_len, test_name, test_name_len, table_index;
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T TRACEROUTE_OM_TraceRouteTableIndexToKey(UI32_T table_index, TRACEROUTE_SORTLST_ELM_T *elm_p);

/* FUNCTION NAME:TRACEROUTE_OM_GetFirstTraceRouteCtlEntry
 * PURPOSE:
 *          Get the first traceroute control entry.
 * INPUT:
 *          None.
 * OUTPUT:
 *          ctrl_entry_p - the first control entry if it exists.
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T  TRACEROUTE_OM_GetFirstTraceRouteCtlEntry(TRACEROUTE_TYPE_TraceRouteCtlEntry_T  *ctrl_entry_p);

/* FUNCTION NAME:TRACEROUTE_OM_SetTraceRouteCtlTargetAddress
 * PURPOSE:
 *          To set the target address field for traceroute control entry
 * INPUT:
 *          owner_index_p - The owner index of the trace route operation.
 *          owner_index_len - The length of the owner index.
 *          test_name_p - The test name of the trace route operation.
 *          test_name_len - The length of the test name.
 *          target_addr_p    -   The target address of the remote host.
 *          target_addr_len -   The length of target_addr_p
 * OUTPUT:
 *          None
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T TRACEROUTE_OM_SetTraceRouteCtlTargetAddress(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p);

#endif
