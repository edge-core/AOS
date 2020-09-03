/* MODULE NAME:  onlpdrv_fan.h
 * PURPOSE:
 *   This module implements the fan related wrapper functions
 *   which call ONLP functions.
 *
 * NOTES:
 *
 * HISTORY
 *    10/23/2015 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation , 2015
 */
#ifndef ONLPDRV_FAN_H
#define ONLPDRV_FAN_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: ONLPDRV_FAN_GetAllFanStatus
 * PURPOSE: This routine is used to get fan status(OK or failed)
 * INPUT:   fan_status_ar_sz_p - the number of element in fan_status_ar
 * OUTPUT:  fan_status_ar_sz_p - the actual number of element output by this function
 *          fan_status_ar[]    - The status of the fans will be put in elements of the
 *                               array. The possible values of the status is shown below:
 *                                 VAL_switchFanStatus_ok
 *                                 VAL_switchFanStatus_failure
 * RETURN:  None.
 * NOTES:   None.
 */
BOOL_T ONLPDRV_FAN_GetAllFanStatus(UI32_T *fan_status_ar_sz_p, UI32_T fan_status_ar[]);

#endif    /* End of ONLPDRV_FAN_H */

