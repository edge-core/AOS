/*-----------------------------------------------------------------------------
 * FILE NAME: poe_pom.h
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file declares the APIs for POE OM IPC.
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

#ifndef POE_POM_H
#define POE_POM_H

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
 * ROUTINE NAME : POE_POM_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resources for POE_POM in the calling process.
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
BOOL_T POE_POM_InitiateProcessResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPsePortAdmin
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE global admin status
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPsePortAdmin(UI32_T group_index, UI32_T port_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPsePortPowerPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE global admin status
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPsePortPowerPriority(UI32_T group_index, UI32_T port_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPsePortDetectionStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE global admin status
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPsePortDetectionStatus(UI32_T group_index, UI32_T port_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPortPowerMaximumAllocation
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE global admin status
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPortPowerMaximumAllocation(UI32_T group_index, UI32_T port_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPortPowerConsumption
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE global admin status
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPortPowerConsumption(UI32_T group_index, UI32_T port_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPortManualHighPowerMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE global admin status
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPortManualHighPowerMode(UI32_T group_index, UI32_T port_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetMainpowerMaximumAllocation
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE global admin status
 * INPUT    : group_index
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetMainpowerMaximumAllocation(UI32_T group_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPoeSoftwareVersion
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE global admin status
 * INPUT    : group_index
 * OUTPUT   : version1, version2, build
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPoeSoftwareVersion(UI32_T group_index, UI8_T *version1, UI8_T *version2, UI8_T *build);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetNextLegacyDetection
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE global admin status
 * INPUT    : group_index
 * OUTPUT   : group_index, value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetNextLegacyDetection(UI32_T *group_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetNextPsePortAdmin
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE global admin status
 * INPUT    : group_index, port_idex
 * OUTPUT   : group_index, port_idex, value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetNextPsePortAdmin(UI32_T *group_index, UI32_T *port_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetNextMainPseEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE global admin status
 * INPUT    : group_index
 * OUTPUT   : group_index, entry
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetNextMainPseEntry(UI32_T *group_index, POE_OM_MainPse_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPethMainPseEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE global admin status
 * INPUT    : group_index
 * OUTPUT   : entry
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPethMainPseEntry(UI32_T group_index, POE_OM_MainPse_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPsePortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE PSE port entry
 * INPUT    : group_index, port_idex
 * OUTPUT   : entry
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPsePortEntry(UI32_T group_index, UI32_T port_index, POE_OM_PsePort_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetRunningPsePortAdmin
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE running PSE port admin
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetRunningPsePortAdmin(UI32_T group_index, UI32_T port_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetRunningPsePortPowerPriority
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE running PSE port power priority
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetRunningPsePortPowerPriority(UI32_T group_index, UI32_T port_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetRunningPortPowerMaximumAllocation
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE running port power maximum allocation
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetRunningPortPowerMaximumAllocation(UI32_T group_index, UI32_T port_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetRunningMainPowerMaximumAllocation
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE running main power maximum allocation
 * INPUT    : group_index
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetRunningMainPowerMaximumAllocation(UI32_T group_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetRunningLegacyDetection
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE running legacy detection
 * INPUT    : group_index
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetRunningLegacyDetection(UI32_T group_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetMainPseOperStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE main PSE operation status
 * INPUT    : group_index
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetMainPseOperStatus(UI32_T group_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPsePortPowerPairsCtrlAbility
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE PSE port power pairs control ability
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPsePortPowerPairsCtrlAbility(UI32_T group_index, UI32_T port_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPsePortPowerPairs
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE PSE port power pairs
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPsePortPowerPairs(UI32_T group_index, UI32_T port_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPsePortPowerClassifications
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE PSE port power pairs
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPsePortPowerClassifications(UI32_T group_index, UI32_T port_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPortPowerCurrent
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE port power current
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPortPowerCurrent(UI32_T group_index, UI32_T port_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPortPowerVoltage
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE port power voltage
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPortPowerVoltage(UI32_T group_index, UI32_T port_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetNextPsePortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE PSE port entry
 * INPUT    : group_index, port_idex
 * OUTPUT   : group_index, port_idex, entry
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetNextPsePortEntry(UI32_T *group_index, UI32_T *port_index, POE_OM_PsePort_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPseNotificationCtrl
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE main PSE operation status
 * INPUT    : group_index
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPseNotificationCtrl(UI32_T group_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetNextNotificationCtrl
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE main PSE operation status
 * INPUT    : group_index
 * OUTPUT   : group_index, value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetNextNotificationCtrl(UI32_T *group_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetPortDot3atPowerInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the poe infomation for LLDP to transmition frame
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetPortDot3atPowerInfo(UI32_T group_index, UI32_T port_index, POE_TYPE_Dot3atPowerInfo_T *info);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_POM_GetRunningPortManualHighPowerMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get POE running port power maximum allocation
 * INPUT    : group_index, port_idex
 * OUTPUT   : value
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_POM_GetRunningPortManualHighPowerMode(UI32_T group_index, UI32_T port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_POM_GetUseLocalPower
 * -------------------------------------------------------------------------
 * FUNCTION: Set Legacy Detection
 * INPUT   : unit
 *           value (TRUE => Local power, FALSE => RPS)
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T POE_POM_GetUseLocalPower(UI32_T group_index, BOOL_T *value);
#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_POM_GetPsePortTimeRangeName
 * -------------------------------------------------------------------------
 * FUNCTION: Get POE port binding time range name on this port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : time_range - time range name
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T POE_POM_GetPsePortTimeRangeName(UI32_T group_index, UI32_T port_index, UI8_T *time_range);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_POM_GetNextPsePortTimeRangeName
 * -------------------------------------------------------------------------
 * FUNCTION: Get POE port binding time range name on next port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           time_range - time range name
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T POE_POM_GetNextPsePortTimeRangeName(UI32_T *group_index, UI32_T *port_index, UI8_T *time_range);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_POM_GetRunningPsePortTimeRangeName
 * -------------------------------------------------------------------------
 * FUNCTION: Get running POE port binding time range name on this port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : time_range
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T POE_POM_GetRunningPsePortTimeRangeName(UI32_T group_index, UI32_T port_index, UI8_T *time_range);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_POM_GetNextRunningPsePortTimeRangeName
 * -------------------------------------------------------------------------
 * FUNCTION: Get running POE port binding time range name on next port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           time_range
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T POE_POM_GetNextRunningPsePortTimeRangeName(UI32_T *group_index, UI32_T *port_index, UI8_T *time_range);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_POM_GetPsePortTimeRangeStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get POE port binding time range status on this port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : status - time range status
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T POE_POM_GetPsePortTimeRangeStatus(UI32_T group_index, UI32_T port_index, UI32_T *status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_POM_GetNextPsePortTimeRangeStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get POE port binding time range status on next port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           status - time range status
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T POE_POM_GetNextPsePortTimeRangeStatus(UI32_T *group_index, UI32_T *port_index, UI32_T *status);
#endif

#endif /* #ifndef POE_POM_H */

