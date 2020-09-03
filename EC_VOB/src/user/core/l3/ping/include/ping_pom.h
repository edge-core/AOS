/* Module Name: PING_POM.H
 * Purpose:
 *    Declares the APIs for IPCs with PING OM.
 *
 * Notes:
 *    None.
 * History:
 *      Date        --  Modifier,   Reason
 *      2007/11/01  --  Timon,      Create
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef PING_POM_H
#define PING_POM_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "ping_type.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
BOOL_T PING_POM_InitiateProcessResources(void);

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
UI32_T PING_POM_GetCtlEntry(PING_TYPE_PingCtlEntry_T *ctrl_entry_p);

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
UI32_T PING_POM_GetNextCtlEntry(PING_TYPE_PingCtlEntry_T *ctrl_entry_p);

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
UI32_T PING_POM_GetProbeHistoryEntry(PING_TYPE_PingProbeHistoryEntry_T *prob_history_entry_p);

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
UI32_T PING_POM_GetNextProbeHistoryEntry(PING_TYPE_PingProbeHistoryEntry_T *prob_history_entry_p);

/* FUNCTION NAME: PING_POM_GetNextProbeHistoryEntryForCli
 * PURPOSE:
 *          To Get next available prob history entry (only for CLI use)
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
 *          ping_ctl_test_name must be provided and are fixed. Only for CLI use.
 */
UI32_T PING_POM_GetNextProbeHistoryEntryForCli(PING_TYPE_PingProbeHistoryEntry_T *prob_history_entry_p);

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
UI32_T PING_POM_GetResultsEntry(PING_TYPE_PingResultsEntry_T *result_entry_p);

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
UI32_T PING_POM_GetNextResultsEntry(PING_TYPE_PingResultsEntry_T *result_entry_p);

#endif /* #ifndef PING_POM_H */
