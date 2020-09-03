/* ------------------------------------------------------------------------
 *  FILE NAME  -  led_pmgr.h
 * ------------------------------------------------------------------------
 *  PURPOSE:
 *
 *  History: 
 *          Date         Modifier,        Reason
 * ------------------------------------------------------------------------
 *          2007.08.01   Echo Chen        Created
 *
 * ------------------------------------------------------------------------
 * Copyright(C) ACCTON Technology Corp. , 2007
 * ------------------------------------------------------------------------
 */
#ifndef LED_PMGR_H
#define LED_PMGR_H
/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE)
#include "led_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
 
/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: LED_PMGR_InitiateProcessResource
 * PURPOSE: Initiate resource used in the calling process.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:
 *          TRUE  --  Sucess
 *          FALSE --  Error
 * NOTES:
 */
BOOL_T LED_PMGR_InitiateProcessResource(void);

/* FUNCTION NAME: LED_PMGR_Start_Xfer(UI32_T unit)
 * PURPOSE: Make Diag LED blink green
 * INPUT:   unit:unit num
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
void LED_PMGR_Start_Xfer(UI32_T unit);

/* FUNCTION NAME: LED_PMGR_Stop_Xfer(UI32_T unit)
 * PURPOSE: Make Diag LED back to normal
 * INPUT:   unit:unit num
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
void LED_PMGR_Stop_Xfer(UI32_T unit);

#if (SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - LED_PMGR_SetLocationLED
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will set the location led status
 * INPUT    : unit_id   -- the unit desired to set
 *            led_is_on -- TRUE : Turn on Location LED
 *                         FALSE: Turn off Location LED
 * OUTPUT   : None
 * RETURN   : LEDDRV_TYPE_RET_OK              - Successfully
 *            LEDDRV_TYPE_RET_ERR_HW_FAIL     - Operation failed due to hardware problem
 *            LEDDRV_TYPE_RET_ERR_NOT_SUPPORT - Operation failed because the device does not support
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
LEDDRV_TYPE_RET_T LED_PMGR_SetLocationLED(UI32_T unit_id, BOOL_T led_is_on);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LED_PMGR_GetLocationLEDStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the location led status
 * INPUT    : unit_id     -- the unit desired to set
 * OUTPUT   : led_is_on_p -- TRUE : Location LED is on
 *                           FALSE: Location LED is off
 * RETURN   : LEDDRV_TYPE_RET_OK              - Successfully
 *            LEDDRV_TYPE_RET_ERR_HW_FAIL     - Operation failed due to hardware problem
 *            LEDDRV_TYPE_RET_ERR_NOT_SUPPORT - Operation failed because the device does not support
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
LEDDRV_TYPE_RET_T LED_PMGR_GetLocationLEDStatus(UI32_T unit_id, BOOL_T *led_is_on_p);
#endif

/* FUNCTION NAME - LED_PMGR_SetModuleLED
 * PURPOSE  : This function will set the module led for the speicific unit
 * INPUT    : unit_id -- the unit desired to set
 *            color   --     LEDDRV_COLOR_OFF              off
 *                           LEDDRV_COLOR_GREEN            green
 *                           LEDDRV_COLOR_AMBER            amber
 *                           LEDDRV_COLOR_GREENFLASH       green flash
 *                           LEDDRV_COLOR_AMBERFLASH       amber flash
 *                           LEDDRV_COLOR_GREENAMBER_FLASH green amber flash
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None.
 */
void LED_PMGR_SetModuleLED(UI32_T unit_id, UI8_T color);

/* FUNCTION NAME - LED_PMGR_SetPoeLED
 * PURPOSE  : This function will set the poe led for the speicific unit
 * INPUT    : unit_id -- the unit desired to set
 *            status  -- POEDRV_TYPE_SYSTEM_OVEROAD, system overload
                         POEDRV_TYPE_SYSTEM_NORMAL, system normal
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None.
 */
BOOL_T LED_PMGR_SetPoeLED(UI32_T unit_id, UI8_T status);
void LED_PMGR_SetPOELed_callback(UI32_T unit_id, UI32_T port_id, UI32_T status);

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - LED_PMGR_SetFanFailLED
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the FAN fail led
 * INPUT   : unit   -- in which unit
 *           fan    -- which fan id
 *           status -- fan status(VAL_switchFanStatus_ok/VAL_switchFanStatus_failure)
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void LED_PMGR_SetFanFailLED(UI32_T unit, UI32_T fan, UI32_T status);
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - LED_PMGR_ThermalStatusChanged
 * ---------------------------------------------------------------------
 * PURPOSE  : Light the LED according to the given thermal sensor status
 * INPUT    : unit          - unit number
 *            thermal       - thermal sensor number (Starts from 1)
 *            is_abnormal   - TRUE  - the temperature of the given thermal sensor falls in the
 *                                    abnormal region (overheating or undercooling)
 *                            FALSE - the temperature of the given thermal sensor falls in the
 *                                    normal region
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
void LED_PMGR_ThermalStatusChanged(UI32_T unit, UI32_T thermal, BOOL_T is_abnormal);
#endif

#if (SYS_CPNT_POWER_DETECT == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - LED_PMGR_PowerStatusChanged
 * ---------------------------------------------------------------------
 * PURPOSE  : Light the LED according to the given psu status
 * INPUT    : unit          - unit number
 *            power_id      - psu id (Starts from 1)
 *            status        - VAL_swIndivPowerStatus_notPresent
 *                            VAL_swIndivPowerStatus_green
 *                            VAL_swIndivPowerStatus_red
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
void LED_PMGR_PowerStatusChanged(UI32_T unit, UI32_T power_id, UI32_T status);
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - LED_PMGR_SetMajorAlarmOutputLed
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the Major Alarm output led
 * INPUT   : unit   -- in which unit
 *           status -- SYSDRV_ALARMMAJORSTATUS_XXX_MASK
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void LED_PMGR_SetMajorAlarmOutputLed(UI32_T unit, UI32_T status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - LED_PMGR_SetMinorAlarmOutputLed
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the Minor Alarm output led
 * INPUT   : unit   -- in which unit
 *           status -- SYSDRV_ALARMMINORSTATUS_XXX_MASK
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void LED_PMGR_SetMinorAlarmOutputLed(UI32_T unit, UI32_T status);

#endif /* LED_PMGR_H */

