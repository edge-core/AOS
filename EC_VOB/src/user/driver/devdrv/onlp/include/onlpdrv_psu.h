/* MODULE NAME:  onlpdrv_psu.h
 * PURPOSE:
 *   This module implements the psu(Power Supply Unit) related wrapper functions
 *   which call ONLP functions.
 *
 * NOTES:
 *
 * HISTORY
 *    10/27/2015 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation , 2015
 */
#ifndef ONLPDRV_PSU_H
#define ONLPDRV_PSU_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define ONLPDRV_PSU_STATUS_PRESENT    BIT_0
#define ONLPDRV_PSU_STATUS_FAILED     BIT_1
#define ONLPDRV_PSU_STATUS_UNPLUGGED  BIT_2

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: ONLPDRV_PSU_Init
 *-----------------------------------------------------------------------------
 * PURPOSE: Do initialization for this module.
 *-----------------------------------------------------------------------------
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:
 */
BOOL_T ONLPDRV_PSU_Init(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - ONLPDRV_PSU_GetPowerStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  Get power status through ONLP API.
 * INPUT  :  power_index          -- Which power(1 based)
 * OUTPUT :  power_status_p       -- Power status value got from ONLP. Bitmap
 *                                   definitions:
 *                                     ONLPDRV_PSU_STATUS_PRESENT
 *                                     ONLPDRV_PSU_STATUS_FAILED
 *                                     ONLPDRV_PSU_STATUS_UNPLUGGED
 * RETURN :  TRUE - success FALSE - failed
 * NOTE   :  None.
 *---------------------------------------------------------------------------
 */
BOOL_T ONLPDRV_PSU_GetPowerStatus(UI32_T power_index, UI8_T *power_status_p);

#endif    /* End of ONLPDRV_PSU_H */

