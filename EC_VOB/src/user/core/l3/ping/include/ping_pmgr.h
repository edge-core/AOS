/* Module Name: PING_PMGR.H
 * Purpose:
 *    Declares the APIs for IPCs with PING MGR.
 *
 * Notes:
 *    None.
 * History:
 *      Date        --  Modifier,   Reason
 *      2007/11/01  --  Timon,      Create
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef PING_PMGR_H
#define PING_PMGR_H


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

/* FUNCTION NAME: PING_PMGR_InitiateProcessResources
 * PURPOSE:
 *          Initiate resources for PING_PMGR in the calling process.
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
BOOL_T PING_PMGR_InitiateProcessResources(void);

/* FUNCTION NAME : PING_PMGR_SetCtlEntry
 * PURPOSE:
 *          To create / destroy contrl entry
 * INPUT:
 *          ctrl_entry_p    -- the specific control entry.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_NO_MORE_WORKSPACE
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. Currently, only CAG and Destroy is supported.
 *             Operation for CAW and NotInService is not allowed.
 *          2. Currentlt, we only permit the setting of row_status, target_address and admin_status
 *          3. key: ping_ctl_owner_index, ping_ctl_test_name.
 */
UI32_T PING_PMGR_SetCtlEntry(PING_TYPE_PingCtlEntry_T *ctrl_entry_p);

/* FUNCTION NAME: PING_PMGR_SetCtlAdminStatus
 * PURPOSE:
 *          To enable or disable ping control entry
 * INPUT:
 *          ctl_entry_p         -- the specific control entry.
 *          ctrl_admin_status   -- the admin status of the to enable or disable the ping.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_NO_MORE_WORKSPACE
 *          PING_TYPE_FAIL
 * NOTES:
 *          1.This API is only used by "set by filed", not "set by record".
 */
UI32_T PING_PMGR_SetCtlAdminStatus(PING_TYPE_PingCtlEntry_T *ctl_entry_p , UI32_T ctrl_admin_status);

/* FUNCTION NAME: PING_PMGR_SetCtlTargetAddress
 * PURPOSE:
 *          To set the target address field for ping control entry
 * INPUT:
 *          ctl_entry_p     -- the pointer of specific control entry.
 *          taget_addr      -- the target address of the remote host.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. We can not change the target address for the specified index when admin_status is enabled.
 *          2. Currently we do not support the domain name query of the target address.
 */
//UI32_T PING_PMGR_SetCtlTargetAddress(PING_TYPE_PingCtlEntry_T *ctl_entry_p, UI32_T taget_addr);
UI32_T PING_PMGR_SetCtlTargetAddress(PING_TYPE_PingCtlEntry_T *ctl_entry_p, L_INET_AddrIp_T* target_addr_p);

/* FUNCTION NAME: PING_PMGR_SetCtlDataSize
 * PURPOSE:
 *          To set the data size field for ping control entry
 * INPUT:
 *          ctl_entry_p     -- the pointer of specific control entry.
 *          data_size       -- the size of data portion in ICMP pkt.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. We can not change the data size for the specified index when admin_status is enabled.
 *          2. SNMP range: 0..65507, CLI range: 32-512.
 */
UI32_T PING_PMGR_SetCtlDataSize(PING_TYPE_PingCtlEntry_T *ctl_entry_p, UI32_T data_size);

/* FUNCTION NAME: PING_PMGR_SetCtlProbeCount
 * PURPOSE:
 *          To set the probe count field for ping control entry
 * INPUT:
 *          ctl_entry_p     -- the pointer of specific control entry.
 *          probe_count     -- the number of ping packet
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. We can not change the probe count for the specified index when admin_status is enabled.
 *          2. SNMP range: 1-15, CLI range: 1-16. So 0 is not allowed.
 */
UI32_T PING_PMGR_SetCtlProbeCount(PING_TYPE_PingCtlEntry_T *ctl_entry_p, UI32_T probe_count);


/* FUNCTION NAME: PING_PMGR_SetCtlRowStatus
 * PURPOSE:
 *          To set row status field for ping control entry
 * INPUT:
 *          ctl_entry_p     -- the  specific control entry.
 *          ctrl_row_status -- the row status of the specified control entry.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_NO_MORE_WORKSPACE
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. The PingCtlEntry should not be modified. So we create a local entry for local use.
 */
UI32_T  PING_PMGR_SetCtlRowStatus(PING_TYPE_PingCtlEntry_T *ctl_entry_p, UI32_T ctrl_row_status);

/* FUNCTION NAME: PING_PMGR_SetCtlEntryByField
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
UI32_T PING_PMGR_SetCtlEntryByField(PING_TYPE_PingCtlEntry_T *ctl_entry_p, PING_TYPE_CtlEntryField_T field);

#endif /* #ifndef PING_PMGR_H */
