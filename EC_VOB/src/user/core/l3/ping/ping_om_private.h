/*-----------------------------------------------------------------------------
 * FILE NAME: PING_OM_PRIVATE.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    The declarations of PING OM which are only used by PING.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/10/31     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef _PING_OM_PRIVATE_H
#define _PING_OM_PRIVATE_H


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

/* FUNCTION NAME : PING_OM_Init
 * PURPOSE:
 *      Initialize ping OM
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
void PING_OM_Init(void);

/* FUNCTION NAME : PING_OM_ClearOM
 * PURPOSE:
 *      Clear ping OM
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
void PING_OM_ClearOM(void);

/* FUNCTION NAME : PING_OM_EnterMasterMode
 * PURPOSE:
 *      Apply default value to ping OM
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
void PING_OM_EnterMasterMode(void);

/* FUNCTION NAME:PING_OM_SetCtlEntry
 * PURPOSE:
 *          To create / destroy contrl entry.
 * INPUT:
 *          ctrl_entry_p    -- the specific control entry, including keys:
 *                             ping_ctl_owner_index, ping_ctl_test_name.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. Currently, only CAG and Destroy is supported.
 *             Operation for CAW and NotInService is not allowed.
 *          2. Currentlt, we only permit the setting of row_status, target_address and admin_status.
 *          3. The ctrl_entry should not be modified, so we create a local entry for local use.
 */
UI32_T PING_OM_SetCtlEntry(PING_TYPE_PingCtlEntry_T *ctrl_entry_p);

/* FUNCTION NAME: PING_OM_AppendProbePacketResult
 * PURPOSE:
 *          To append specific prob histroy entry to the end of the last histroy entry
 * INPUT:
 *          table_index         -- A key index to identify prob history entry for different
 *                                 owner_index and test name.
 *          prob_history_entry_p  -- contains values to set to prob history entry.
 *
 * OUTPUT:
 *          None.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_ENTRY
 *          PING_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T PING_OM_AppendProbePacketResult(UI32_T table_index, PING_TYPE_PingProbeHistoryEntry_T *prob_packet_entry_p);

/* FUNCTION NAME: PING_OM_SetResultsEntry
 * PURPOSE:
 *          To set specific ping result entry by the specific entry index
 * INPUT:
 *          key_index       -- the index of the result table.
 *          result_entry_p  -- the pointer of the result_entry.
 *
 * OUTPUT:
 *          None.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_ENTRY
 *          PING_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T PING_OM_SetResultsEntry(UI32_T key_index, PING_TYPE_PingResultsEntry_T *result_entry_p);
#if 0
/* FUNCTION NAME: PING_OM_SetCtlEntryByField
 * PURPOSE:
 *          Set only the field of the entry.
 * INPUT:
 *          ctl_entry_p -- the pointer of the specified ctl entry.
 *          field       -- field.
 * OUTPUT:
 *          None.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_FAIL
 *          PING_TYPE_INVALID_ARG
 * NOTES:
 *          1. set only the field of the entry.
 */
UI32_T PING_OM_SetCtlEntryByField(PING_TYPE_PingCtlEntry_T *ctl_entry_p, PING_TYPE_CtlEntryField_T field);
#endif
/* FUNCTION NAME: PING_OM_PrintAllProbeHistory
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
void PING_OM_PrintAllProbeHistory(void);


#endif /* #ifndef _PING_OM_PRIVATE_H */
