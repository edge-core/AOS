/* MODULE NAME:  sysdrv_private.h
 * PURPOSE:
 *     header file for declarations of functions that are private to sysdrv
 *
 * NOTES:
 *
 * REASON:
 *    9/13/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef SYSDRV_PRIVATE_H
#define SYSDRV_PRIVATE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "sys_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
/* FUNCTION NAME: SYSDRV_GetFanSpeedAddr
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the array of the fan speed addresses.
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : The array of the fan speed addresses.
 *-----------------------------------------------------------------------------
 * NOTES:
 */
const UI8_T* SYSDRV_GetFanSpeedAddr(void);
#endif

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
/* FUNCTION NAME: SYSDRV_GetFanFullSpeedSetting
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the array of the full fan speed setting.
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : The array of the fan full speed setting.
 *-----------------------------------------------------------------------------
 * NOTES:
 */
const UI8_T* SYSDRV_GetFanFullSpeedSetting(void);
#endif

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
/* FUNCTION NAME: SYSDRV_GetFanSpeedRegs
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the array of the fan speed regs.
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : The array of the fan speed regs.
 *-----------------------------------------------------------------------------
 * NOTES:
 */
const UI16_T* SYSDRV_GetFanSpeedRegs(void);
#endif

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
/* FUNCTION NAME: SYSDRV_GetFanMidSpeedSetting
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the array of the fan mid speed setting.
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : The array of the fan mid speed setting.
 *-----------------------------------------------------------------------------
 * NOTES:
 */
const UI8_T* SYSDRV_GetFanMidSpeedSetting(void);
#endif

/* FUNCTION NAME: SYSDRV_Notify_AlarmInputStatusChanged
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will callback when alarm input status is changed
 *-----------------------------------------------------------------------------
 * INPUT    : unit         --- Which unit.
 *            status       ---
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void SYSDRV_Notify_AlarmInputStatusChanged(UI32_T unit, UI32_T status);

/* FUNCTION NAME: SYSDRV_Notify_MajorAlarmOutputStatusChanged
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will callback when major alarm output status is changed
 *-----------------------------------------------------------------------------
 * INPUT    : unit         --- Which unit.
 *            status       --- SYSDRV_ALARMMAJORSTATUS_XXX
 *                             (e.g. SYSDRV_ALARMMAJORSTATUS_POWER_STATUS_CHANGED_MASK)
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void SYSDRV_Notify_MajorAlarmOutputStatusChanged(UI32_T unit, UI32_T status);

/* FUNCTION NAME: SYSDRV_Notify_MinorAlarmOutputStatusChanged
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will callback when minor alarm output status is changed
 *-----------------------------------------------------------------------------
 * INPUT    : unit         --- Which unit.
 *            status       --- SYSDRV_ALARMMINORSTATUS_XXX
 *                             (e.g. SYSDRV_ALARMMINORSTATUS_OVERHEAT_MASK)
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void SYSDRV_Notify_MinorAlarmOutputStatusChanged(UI32_T unit, UI32_T status);

#endif    /* End of SYSDRV_PRIVATE_H */

