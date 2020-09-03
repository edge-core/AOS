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
 *    12/03/2008 - Eugene Yu, Porting to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
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

BOOL_T POEDRV_OM_SetProvisionComplete(BOOL_T status);

BOOL_T POEDRV_OM_GetProvisionComplete(BOOL_T *status);

BOOL_T POEDRV_OM_SetThreadId(UI32_T thread_id);

BOOL_T POEDRV_OM_GetThreadId(UI32_T *poedrv_thread_id);

/* FUNCTION NAME : POEDRV_OM_IsStopMonitorFlagOn
 * PURPOSE: This function is used to get the status of query function in PoE
 * INPUT   : 
 * OUTPUT  : none
 * RETURN:  TRUE  -- stop to poll
 *          FALSE -- in polling
 * NOTES:   
 */
BOOL_T POEDRV_OM_IsStopMonitorFlagOn();


/* FUNCTION NAME : POEDRV_OM_SetStopMonitorFlag
 * PURPOSE: This function is used to set the flag of poe monitor function
 * INPUT   : state - TRUE: stop, FALSE - polling
 * OUTPUT  : none
 * RETURN:  
 *          
 * NOTES:   
 */
void POEDRV_OM_SetStopMonitorFlag(BOOL_T state);


void POEDRV_OM_SetMyUnitID(UI32_T value);

void POEDRV_OM_GetMyUnitID(UI32_T *value);

void POEDRV_OM_SetNumOfUnits(UI32_T value);

void POEDRV_OM_GetNumOfUnits(UI32_T *value);

void POEDRV_OM_GetMainPowerMaxAllocation(UI32_T *value);

void POEDRV_OM_GetPOEPortNumber(UI32_T *min, UI32_T *max);

void POEDRV_OM_SetImageVersion(UI8_T value);

void POEDRV_OM_GetImageVersion(UI8_T *value);


void POEDRV_OM_SetMainPowerInfoUnitID(UI8_T value);

void POEDRV_OM_GetMainPowerInfoUnitID(UI8_T *value);

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

void POEDRV_OM_GetPortInfoPowerClass(UI32_T port, UI32_T *value);

void POEDRV_OM_SetPortInfoTemperature(UI32_T port, I32_T value);

void POEDRV_OM_GetPortInfoTemperature(UI32_T port, I32_T *value);

void POEDRV_OM_SetPortInfoVoltage(UI32_T port, UI32_T value);

void POEDRV_OM_GetPortInfoVoltage(UI32_T port, UI32_T *value);

void POEDRV_OM_SetPortInfoCurrent(UI32_T port, UI32_T value);

void POEDRV_OM_GetPortInfoCurrent(UI32_T port, UI32_T *value);

void POEDRV_OM_SetPortInfoLedStatus(UI32_T port, UI8_T value);

void POEDRV_OM_GetPortInfoLedStatus(UI32_T port, UI8_T *value);

void POEDRV_OM_SetPortInfoDetectionStatus(UI32_T port, UI8_T value);

void POEDRV_OM_GetPortInfoDetectionStatus(UI32_T port, UI8_T *value);

void POEDRV_OM_SetPortInfoPowerClass(UI32_T port, UI32_T value);

void POEDRV_OM_GetPortInfoPowerClass(UI32_T port, UI32_T *value);

void POEDRV_OM_SetPortInfoIsOverload(UI32_T port, BOOL_T value);

void POEDRV_OM_GetPortInfoIsOverload(UI32_T port, BOOL_T *value);



void POEDRV_OM_AddPortCounter(UI32_T port, UI32_T value1, UI32_T value2);

void POEDRV_OM_GetPortCounter(UI32_T port, UI32_T value1, UI32_T *value2);



void POEDRV_OM_SetPingPongInfoStartTicks(UI32_T port, UI32_T value);

void POEDRV_OM_GetPingPongInfoStartTicks(UI32_T port, UI32_T *value);

void POEDRV_OM_AddPingPongInfoTimesOfPowerDenied(UI32_T port, UI32_T value);

void POEDRV_OM_GetPingPongInfoTimesOfPowerDenied(UI32_T port, UI32_T *value);

void POEDRV_OM_ResetOnePingPongInfo(UI32_T port);

void POEDRV_OM_ResetAllPingPongInfo(void);

#endif  /* POEDRV_OM_H */

