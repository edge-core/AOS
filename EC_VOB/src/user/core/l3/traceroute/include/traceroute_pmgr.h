/* Module Name: TRACEROUTE_PMGR.H
 * Purpose:
 *    Declares the APIs for IPCs with TRACEROUTE MGR.
 *
 * Notes:
 *    None.
 * History:
 *      Date        --  Modifier,   Reason
 *      2007/12/19  --  Peter Yu,   Create
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef TRACEROUTE_PMGR_H
#define TRACEROUTE_PMGR_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "traceroute_type.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: TRACEROUTE_PMGR_InitiateProcessResources
 * PURPOSE:
 *          Initiate resources for TRACEROUTE_PMGR in the calling process.
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
BOOL_T TRACEROUTE_PMGR_InitiateProcessResources(void);


/* FUNCTION NAME: TRACEROUTE_PMGR_SetTraceRouteCtlEntry
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
UI32_T  TRACEROUTE_PMGR_SetTraceRouteCtlEntry(TRACEROUTE_TYPE_TraceRouteCtlEntry_T  *ctl_entry_p);


/* FUNCTION NAME: TRACEROUTE_PMGR_SetTraceRouteCtlAdminStatus
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
UI32_T  TRACEROUTE_PMGR_SetTraceRouteCtlAdminStatus(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p);



/* FUNCTION NAME: TRACEROUTE_PMGR_SetTraceRouteCtlRowStatus
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
UI32_T  TRACEROUTE_PMGR_SetTraceRouteCtlRowStatus(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p);


/* FUNCTION NAME: TRACEROUTE_PMGR_SetTraceRouteCtlTargetAddress
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
UI32_T  TRACEROUTE_PMGR_SetTraceRouteCtlTargetAddress(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p);

/* FUNCTION NAME: TRACEROUTE_PMGR_SetCtlEntryByField
 * PURPOSE:
 *          Set only the field of the entry.
 * INPUT:
 *          ctl_entry_p -- the pointer of the specified ctl entry.
 *          field       -- field.
 * OUTPUT:
 *          None.
 * RETURN:
 *          TRACEROUTE_TYPE_OK
 *          TRACEROUTE_TYPE_FAIL
 *          TRACEROUTE_TYPE_INVALID_ARG
 * NOTES:
 *          1. set only the field of the entry.
 */
UI32_T TRACEROUTE_PMGR_SetCtlEntryByField(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p, TRACEROUTE_TYPE_CtlEntryField_T field);

 
#endif /* #ifndef TRACEROUTE_PMGR_H */
