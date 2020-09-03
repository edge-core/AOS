/*-----------------------------------------------------------------------------
 * FILE NAME: poe_pmgr.h
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file declares the APIs for POE MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    12/03/2008 - Eugene Yu, porting POE to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */

#ifndef POE_PMGR_H
#define POE_PMGR_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "poe_type.h"
#include "poe_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DEFINITIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : POE_PMGR_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resources for POE_PMGR in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  --  Sucess
 *           FALSE --  Error
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T POE_PMGR_InitiateProcessResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_SetMainpowerMaximumAllocation
 *-------------------------------------------------------------------------
 * PURPOSE  : Set POE mainpower maxmum allocation
 * INPUT    : group_index, value
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_SetMainpowerMaximumAllocation(UI32_T group_index, UI32_T value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_SetPsePortAdmin
 *-------------------------------------------------------------------------
 * PURPOSE  : Set POE PSE port admin status
 * INPUT    : group_index, port_index, value
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_SetPsePortAdmin(UI32_T group_index, UI32_T port_index, UI32_T value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_SetPortPowerMaximumAllocation
 *-------------------------------------------------------------------------
 * PURPOSE  : Set POE PSE port admin status
 * INPUT    : group_index, port_index, value
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_SetPortPowerMaximumAllocation(UI32_T group_index, UI32_T port_index, UI32_T value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_SetPsePortPowerPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : Set POE PSE port admin status
 * INPUT    : group_index, port_index, value
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_SetPsePortPowerPriority(UI32_T group_index, UI32_T port_index, UI32_T value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_SetLegacyDetection
 *-------------------------------------------------------------------------
 * PURPOSE  : Set POE PSE port admin status
 * INPUT    : group_index, value
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_SetLegacyDetection(UI32_T group_index, UI32_T value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_SetPortManualHighPowerMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set POE PSE port admin status
 * INPUT    : group_index, port_index, value
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_SetPortManualHighPowerMode(UI32_T group_index, UI32_T port_index, UI32_T value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_SetPsePortPowerPairs
 *-------------------------------------------------------------------------
 * PURPOSE  : Set POE PSE port admin status
 * INPUT    : group_index, port_index, value
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_SetPsePortPowerPairs(UI32_T group_index, UI32_T port_index, UI32_T value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_SetPsePortType
 *-------------------------------------------------------------------------
 * PURPOSE  : Set POE PSE port admin status
 * INPUT    : group_index, port_index, value1, value2
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_SetPsePortType(UI32_T group_index, UI32_T port_index, UI8_T* value1, UI32_T value2);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_SetMainPseUsageThreshold
 *-------------------------------------------------------------------------
 * PURPOSE  : Set POE PSE port admin status
 * INPUT    : group_index, value
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_SetMainPseUsageThreshold(UI32_T group_index, UI32_T value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_SetNotificationCtrl
 *-------------------------------------------------------------------------
 * PURPOSE  : Set POE PSE port admin status
 * INPUT    : group_index, value
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_SetNotificationCtrl(UI32_T group_index, UI32_T value);

#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_BindTimeRangeToPsePort
 *-------------------------------------------------------------------------
 * PURPOSE  : Bind POE PSE port to a specified time range with name
 * INPUT    : group_index, port_index, time_range
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_BindTimeRangeToPsePort(UI32_T group_index, UI32_T port_index, UI8_T* time_range);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_PMGR_UnbindTimeRangeToPsePort
 *-------------------------------------------------------------------------
 * PURPOSE  : Unbind POE PSE port to a specified time range with name
 * INPUT    : group_index, port_index
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_PMGR_UnbindTimeRangeToPsePort(UI32_T group_index, UI32_T port_index);
#endif

#endif /* #ifndef POE_PMGR_H */


