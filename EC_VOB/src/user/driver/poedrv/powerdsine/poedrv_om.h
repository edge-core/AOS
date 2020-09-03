/*-----------------------------------------------------------------------------
 * FILE NAME: poedrv_om.h
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 *    This file declares the APIs for POEDRV OM to read/write the database.
 * 
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    07/01/2009 - Eugene Yu, Porting to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2009
 *-----------------------------------------------------------------------------
 */

#ifndef POEDRV_OM_H
#define POEDRV_OM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "poedrv_type.h"

/* NAME CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */ 

/* FUNCTION NAME : POEDRV_OM_Logical2PhyDevicePortID
 * PURPOSE: This function initializes all releated variables and restarts
 *          the PoE driver.
 * INPUT:   port->logical port num;phy_port->address of physical port num
 * OUTPUT:  None.
 * RETURN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 */
BOOL_T	POEDRV_OM_Logical2PhyDevicePortID(UI32_T port,UI32_T *phy_port);

/* FUNCTION NAME : POEDRV_OM_Physical2LogicalPort
 * PURPOSE: This function initializes all releated variables and restarts
 *          the PoE driver.
 * INPUT:   port->phisical port num; lport->address of logical port num
 * OUTPUT:  None.
 * RETURN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 */
BOOL_T POEDRV_OM_Physical2LogicalPort(UI32_T port, UI32_T *lport);


void POEDRV_OM_Reset(void);
void POEDRV_OM_ResetByBoardID(void);


void POEDRV_OM_Init(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: POEDRV_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for BUFFERMGMT in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void POEDRV_OM_AttachSystemResources(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: POEDRV_OM_EnterMasterMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter master mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void POEDRV_OM_EnterMasterMode (void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: POEDRV_OM_EnterSlaveMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter slave mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void POEDRV_OM_EnterSlaveMode (void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: POEDRV_OM_SetTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Sets to temporary transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void POEDRV_OM_SetTransitionMode (void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: POEDRV_OM_EnterTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void POEDRV_OM_EnterTransitionMode (void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: POEDRV_OM_GetOperatingMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Get operating mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
UI32_T POEDRV_OM_GetOperatingMode();

void POEDRV_OM_SetProvisionComplete(BOOL_T status);
void POEDRV_OM_GetProvisionComplete(BOOL_T *status);
void POEDRV_OM_SetHwEnable(BOOL_T status);
void POEDRV_OM_GetHwEnable(BOOL_T *status);

void POEDRV_OM_SetMyUnitID(UI32_T value);
void POEDRV_OM_GetMyUnitID(UI32_T *value);
void POEDRV_OM_SetNumOfUnits(UI32_T value);
void POEDRV_OM_GetNumOfUnits(UI32_T *value);
void POEDRV_OM_GetMainPowerMaxAllocation(UI32_T *value);
void POEDRV_OM_GetPOEPortNumber(UI32_T *min, UI32_T *max);
void POEDRV_OM_SetImageVersion(UI8_T value_1, UI8_T value_2);
void POEDRV_OM_GetImageVersion(UI8_T *value_1, UI8_T *value_2);
void POEDRV_OM_SetImageBuild(UI8_T value);
void POEDRV_OM_GetImageBuild(UI8_T *value);

void POEDRV_OM_SetMainPowerInfoUnitID(UI8_T value);
void POEDRV_OM_GetMainPowerInfoUnitID(UI8_T *value);
void POEDRV_OM_SetMainPowerInfoBoardID(UI32_T value);
void POEDRV_OM_GetMainPowerInfoBoardID(UI32_T *value);
void POEDRV_OM_SetMainPowerInfoMainOperStatus(UI8_T value);
void POEDRV_OM_GetMainPowerInfoMainOperStatus(UI8_T *value);
void POEDRV_OM_SetMainPowerInfoMainPower(UI32_T value);
void POEDRV_OM_GetMainPowerInfoMainPower(UI32_T *value);
void POEDRV_OM_SetMainPowerInfoMainConsumption(UI32_T value);
void POEDRV_OM_GetMainPowerInfoMainConsumption(UI32_T *value);
void POEDRV_OM_SetMainPowerInfoLegacyDectionEnable(UI8_T value);
void POEDRV_OM_GetMainPowerInfoLegacyDectionEnable(UI8_T *value);

void POEDRV_OM_SetPortInfoPowerConsumption(UI32_T port, UI32_T value);
void POEDRV_OM_GetPortInfoPowerConsumption(UI32_T port, UI32_T *value);
void POEDRV_OM_SetPortInfoDetectionStatus(UI32_T port, UI8_T value);
void POEDRV_OM_GetPortInfoDetectionStatus(UI32_T port, UI8_T *value);
void POEDRV_OM_SetPortInfoPowerClass(UI32_T port, UI32_T value);
void POEDRV_OM_GetPortInfoPowerClass(UI32_T port, UI32_T *value);
void POEDRV_OM_SetPortInfoIsOverload(UI32_T port, BOOL_T value);
void POEDRV_OM_GetPortInfoIsOverload(UI32_T port, BOOL_T *value);
void POEDRV_OM_SetPortInfoActualStatus(UI32_T port, UI8_T value);
void POEDRV_OM_GetPortInfoActualStatus(UI32_T port, UI8_T *value);
void POEDRV_OM_SetPortInfoLinkUp(UI32_T port, UI8_T value);
void POEDRV_OM_GetPortInfoLinkUp(UI32_T port, UI8_T *value);
void POEDRV_OM_SetPortInfoActive(UI32_T port, UI8_T value);
void POEDRV_OM_GetPortInfoActive(UI32_T port, UI8_T *value);
void POEDRV_OM_SetPortInfoIsPortFailure(UI32_T port, BOOL_T value);
void POEDRV_OM_GetPortInfoIsPortFailure(UI32_T port, BOOL_T *value);
void POEDRV_OM_SetPortInfoAdminStatus(UI32_T port, UI8_T value);
void POEDRV_OM_GetPortInfoAdminStatus(UI32_T port, UI8_T *value);


void POEDRV_OM_SetPortState(UI32_T port, UI8_T value);
void POEDRV_OM_GetPortState(UI32_T port, UI8_T *value);

/* FUNCTION NAME: POEDRV_OM_IsStopMonitorFlagOn
 * PURPOSE:       This function is used to get the status of query function in PoE.
 * INPUT:         None.
 * OUTPUT:        None.
 * RETURN:        TRUE  -- Stop to poll.
 *                FALSE -- In polling.
 * NOTES:
 */
BOOL_T POEDRV_OM_IsStopMonitorFlagOn(void);

/* FUNCTION NAME: POEDRV_OM_SetStopMonitorFlag
 * PURPOSE:       This function is used to set the flag of poe monitor function.
 * INPUT:         state: TRUE  -- Stop to poll.
 *                       FALSE -- In polling.
 * OUTPUT:        None.
 * RETURN:        None.
 *
 * NOTES:
 */
void POEDRV_OM_SetStopMonitorFlag(BOOL_T state);

#endif  /* POEDRV_OM_H */

