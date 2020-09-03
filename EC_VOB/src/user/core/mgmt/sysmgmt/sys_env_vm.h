#ifndef SYS_ENV_VM_H
#define SYS_ENV_VM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "l_sort_lst.h"

/* ------------------------------------------------------------------------
 *  FILE NAME  -  SYS_ENV_VM.H
 * ------------------------------------------------------------------------
 * PURPOSE: 1. This package provide a set of database and machine to store the
 *             environment status and send trap when necessary.
 *          2. The environment status means fan status, power status, thermal,
 *             and etc.
 *          3. The naming constant defined in this package shall be reused by
 *             all the BNBU L2/L3 switch projects.
 *          4. This package shall be reusable for all all the BNBU L2/L3 switch
 *             projects.
 *
 *  History:
 *       Charles Cheng   3/12/2003      new created
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2003
 * ------------------------------------------------------------------------
 */

typedef struct SYS_ENV_VM_SwitchThermalActionEntry_S
{
    UI32_T      unit_index;         /* key */
    UI32_T      thermal_index;      /* key */
    UI32_T      action_index;       /* key */
    I32_T       rising_threshold;
    I32_T       falling_threshold;
    UI32_T      action;            /* bits: bit 0:none; bit 1:trap */
    UI32_T      status;
    BOOL_T      is_default_entry;
} SYS_ENV_VM_SwitchThermalActionEntry_T;

typedef struct
{
    L_SORT_LST_List_T sorted_list;
    BOOL_T            is_init;
} SYS_ENV_VM_SwitchThermalActionTable_T;



/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetDatabaseToDefault
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to init the database without sending trap.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetDatabaseToDefault(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetPowerStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get power status.
 * INPUT   : unit   --- Which unit.
 *           power  --- Which power.
 * OUTPUT  : status --- VAL_swIndivPowerStatus_notPresent
 *                      VAL_swIndivPowerStatus_green
 *                      VAL_swIndivPowerStatus_red
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_GetPowerStatus(UI32_T unit, UI32_T power, UI32_T *status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetPowerStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set power status.
 * INPUT   : unit   --- Which unit.
 *           power  --- Which power.
 *           status --- VAL_swIndivPowerStatus_notPresent
 *                      VAL_swIndivPowerStatus_green
 *                      VAL_swIndivPowerStatus_red
 * OUTPUT  :
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetPowerStatus(UI32_T unit, UI32_T power, UI32_T status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetPowerType
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get power type.
 * INPUT   : unit   --- Which unit.
 *           power  --- Which power index(1 based).
 * OUTPUT  : type   --- VAL_swIndivPowerType_DC_N48
 *                      VAL_swIndivPowerType_DC_P24
 *                      VAL_swIndivPowerType_AC
 *                      VAL_swIndivPowerType_DC_N48_Wrong
 *                      VAL_swIndivPowerType_DC_P24_Wrong
 *                      VAL_swIndivPowerType_none
 *                      VAL_swIndivPowerType_AC_Wrong
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_GetPowerType(UI32_T unit, UI32_T power, UI32_T *type);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetPowerType
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set power type.
 * INPUT   : unit   --- Which unit.
 *           power  --- Which power index(1 based).
 *           type   --- VAL_swIndivPowerType_DC_N48
 *                      VAL_swIndivPowerType_DC_P24
 *                      VAL_swIndivPowerType_AC
 *                      VAL_swIndivPowerType_none
 * OUTPUT  :
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetPowerType(UI32_T unit, UI32_T power, UI32_T type);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetAlarmInputStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get status of alarm input.
 * INPUT   : unit             --- Which unit.
 * OUTPUT  : status_p         ---  status of four alarm input(bit mapped)
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SYS_ENV_VM_GetAlarmInputStatus(UI32_T unit, UI32_T *status_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetAlarmInputStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set status of alarm input.
 * INPUT   : unit             --- Which unit.
 *           status           ---  status of four alarm input(bit mapped)
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SYS_ENV_VM_SetAlarmInputStatus(UI32_T unit, UI32_T status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetAlarmInputName
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get name of alarm input.
 * INPUT   : unit             --- Which unit.
 *           index            --- Which index.
 * OUTPUT  : name_p           --- description of alarm input
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SYS_ENV_VM_GetAlarmInputName(UI32_T unit, UI32_T index, char *name_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetAlarmInputName
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set name of alarm input.
 * INPUT   : unit             --- Which unit.
 *           index            --- Which index.
 *           name_p           --- description of alarm input
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetAlarmInputName(UI32_T unit, UI32_T index, char *name_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetMajorAlarmOutputStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get major alarm output status.
 * INPUT   : unit             --- Which unit.
 * OUTPUT  : status           --- SYSDRV_ALARMMAJORSTATUS_XXX_MASK
 * RETURN  : TRUE if get successfully
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_GetMajorAlarmOutputStatus(UI32_T unit, UI32_T *status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetMajorAlarmOutputStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set major alarm output status.
 * INPUT   : unit             --- Which unit.
 *           status           --- SYSDRV_ALARMMAJORSTATUS_XXX_MASK
 * OUTPUT  : None.
 * RETURN  : TRUE if set sucessfully.
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetMajorAlarmOutputStatus(UI32_T unit, UI32_T status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetMinorAlarmOutputStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get minor alarm output status.
 * INPUT   : unit             --- Which unit.
 * OUTPUT  : status           --- SYSDRV_ALARMMINORSTATUS_XXX_MASK
 * RETURN  : TRUE if get successfully
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_GetMinorAlarmOutputStatus(UI32_T unit, UI32_T *status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetMinorAlarmOutputStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set minor alarm output status.
 * INPUT   : unit             --- Which unit.
 *           status           --- SYSDRV_ALARMMINORSTATUS_XXX_MASK
 * OUTPUT  : None.
 * RETURN  : TRUE if set sucessfully.
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetMinorAlarmOutputStatus(UI32_T unit, UI32_T status);

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetFanStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get fan status.
 * INPUT   : unit   --- Which unit.
 *           fan    --- Which fan.
 * OUTPUT  : status --- VAL_switchFanStatus_ok/VAL_switchFanStatus_failure
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_GetFanStatus(UI32_T unit, UI32_T fan, UI32_T *status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetFanStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set fan status and send trap if iy is
 *           necessary.
 * INPUT   : unit   --- Which unit.
 *           fan   ---  Which fan.
 *           status --- VAL_switchFanStatus_ok/VAL_switchFanStatus_failure
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetFanStatus(UI32_T unit, UI32_T fan, UI32_T status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetFanFailCounter
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get fan fail counter.
 * INPUT   : unit   --- Which unit.
 *           fan   --- Which fan.
 *           *counter   --- buffer for fan fail counter
 * OUTPUT  : *counter -- fan fail counter
 * RETURN  : TRUE/FALSE
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_GetFanFailCounter(UI32_T unit, UI32_T fan, UI32_T *fan_fail_counter);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetFanSpeed
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get fan speed.
 * INPUT   : unit      --- Which unit.
 *           fan       --- Which fan.
 * OUTPUT  : fan_speed_p --- Fan speed.
 * RETURN  : TRUE   -  Success
 *           FALSE  -  Fail
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_GetFanSpeed(UI32_T unit, UI32_T fan, UI32_T *fan_speed_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetFanSpeed
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set fan speed
 * INPUT   : unit      --- Which unit.
 *           fan       --- Which fan.
 *           fan_speed --- Fan speed.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetFanSpeed(UI32_T unit, UI32_T fan, UI32_T fan_speed);

#if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE)
/*------------------------------------------------------------------------ 
 * FUNCTION NAME: SYS_ENV_VM_SetFanSpeedForceFull
 *------------------------------------------------------------------------ 
 * PURPOSE: This routine is used to Set fan speed full database
 * INPUT:   mode:  TRUE  -- enabled.
 *                 FALSE -- disabled.
 * OUTPUT:  None.
 * RETURN:  TRUE  : success
 *          FALSE : fail
 * NOTES:   None.
 *------------------------------------------------------------------------ 
 */
BOOL_T SYS_ENV_VM_SetFanSpeedForceFull(BOOL_T mode);

/*------------------------------------------------------------------------ 
 * FUNCTION NAME: SYS_ENV_VM_GetFanSpeedForceFull
 *------------------------------------------------------------------------ 
 * PURPOSE: This routine is used to Get fan speed full database
 * INPUT:   None.
 * OUTPUT:  mode:  TRUE  -- enabled.
 *                 FALSE -- disabled.
 * RETURN:  TRUE  : success
 *          FALSE : fail
 * NOTES:   None.
 *------------------------------------------------------------------------ 
 */
BOOL_T SYS_ENV_VM_GetFanSpeedForceFull(BOOL_T *mode);
#endif
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetThermalStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get thermal status of the specified
 *           thermal sensor.
 * INPUT   : unit           -- Which unit.
 *           thermal_idx    -- Which thermal sensor. (starts from 1)
 * OUTPUT  : temperature_p  -- Thermal temperature
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. No critical section protection, because semaphore exists in SYS_MGR.
 *           2. Thermal status is a generic name for all of the information
 *              which might have in a thermal sensor. For now, thermal status
 *              only contains temperature data.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_GetThermalStatus(UI32_T unit, UI32_T thermal_idx, I32_T *temperature_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetThermalStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set thermal status and send trap if it is
 *           necessary.
 * INPUT   : unit        -- Which unit.
 *           thermal_idx -- Which thermal sensor. (starts from 1)
 *           temperature -- Thermal temperature
 * OUTPUT  : abnormal_status_changed_p
 *                       -- TRUE if the abnormal status is changed
 *                          FALSE if the abnormal status is not changed
 *           is_abnormal_p
 *                       -- TRUE if the temperature got from the specified
 *                          thermal sensor falls in the abnormal region
 *                          (overheating or undercooling)
 *                          FALSE if the temperature got from the specified
 *                          thermal sensor falls in the normal region
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. No critical section protection, because semaphore exists in SYS_MGR.
 *           2. Thermal status is a generic name for all of the information
 *              which might have in a thermal sensor. For now, thermal status
 *              only contains temperature data.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetThermalStatus(UI32_T unit, UI32_T thermal_idx, I32_T temperature,
    BOOL_T *abnormal_status_changed_p, BOOL_T *is_abnormal_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_InitThermalActionTable
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to initial thermal action table.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 * -------------------------------------------------------------------------*/
void SYS_ENV_VM_InitThermalActionTable(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetSwitchThermalActionEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get switch thermal action entry.
 * INPUT   : *action_entry -- output buffer of action entry
 * OUTPUT  : *action_entry -- action entry
 * RETURN  : TRUE/FALSE
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 *           Entry key are as follow:
 *           1.action_entry->unit_index
 *           2.action_entry->thermal_index
 *           3.action_entry->action_index
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_GetSwitchThermalActionEntry(SYS_ENV_VM_SwitchThermalActionEntry_T *action_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetNextSwitchThermalActionEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get next switch thermal action entry.
 * INPUT   : *action_entry -- output buffer of next action entry with key
 * OUTPUT  : *action_entry -- next action entry
 * RETURN  : TRUE/FALSE
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 *           Entry key are as follow:
 *           1.action_entry->unit_index
 *           2.action_entry->thermal_index
 *           3.action_entry->action_index
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_GetNextSwitchThermalActionEntry(SYS_ENV_VM_SwitchThermalActionEntry_T *action_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetSwitchThermalActionEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set switch thermal action entry.
 * INPUT   : *action_entry -- pointer of action entry
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetSwitchThermalActionEntry(SYS_ENV_VM_SwitchThermalActionEntry_T *action_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetSwitchThermalActionRisingThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set switch thermal action rising threshold.
 * INPUT   : unit_index -- unit index
 *           thermal_index -- thermal index
 *           action_index -- action index
 *           rising_threshold -- rising threshold
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetSwitchThermalActionRisingThreshold(UI32_T unit_index, UI32_T thermal_index,
                                                      UI32_T action_index, UI32_T rising_threshold);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetSwitchThermalActionFallingThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set switch thermal action falling threshold.
 * INPUT   : unit_index -- unit index
 *           thermal_index -- thermal index
 *           action_index -- action index
 *           falling_threshold -- falling threshold
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetSwitchThermalActionFallingThreshold(UI32_T unit_index, UI32_T thermal_index,
                                                       UI32_T action_index, UI32_T falling_threshold);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetSwitchThermalActionAction
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set switch thermal action action.
 * INPUT   : unit_index -- unit index
 *           thermal_index -- thermal index
 *           action_index -- action index
 *           action -- action
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetSwitchThermalActionAction(UI32_T unit_index, UI32_T thermal_index,
                                             UI32_T action_index, UI32_T action);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetSwitchThermalActionStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set switch thermal action status.
 * INPUT   : unit_index -- unit index
 *           thermal_index -- thermal index
 *           action_index -- action index
 *           status -- status
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetSwitchThermalActionStatus(UI32_T unit_index, UI32_T thermal_index,
                                             UI32_T action_index, UI32_T status);

#endif


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetDebugMode
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get debug mode for sys_env_vm
 * INPUT   : *mode -- output buffer of debug mode
 * OUTPUT  : *mode -- debug mode
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SYS_ENV_VM_GetDebugMode(BOOL_T *mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetDebugMode
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set debug mode for sys_env_vm
 * INPUT   : mode -- debug mode
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SYS_ENV_VM_SetDebugMode(BOOL_T mode);

#endif
