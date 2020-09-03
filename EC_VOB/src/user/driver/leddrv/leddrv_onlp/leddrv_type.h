/* MODULE NAME:  leddrv_type.h
 * PURPOSE:
 *      Define definitions which are private to LEDDRV.
 * NOTES:
 *      None.
 * HISTORY
 *    2007/08/07 - Echo Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef LEDDRV_TYPE_H
#define LEDDRV_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "sys_adpt.h"
#include "sys_hwcfg.h"

/* NAMING CONSTANT DECLARATIONS
 */

#define LEDDRV_NUM_OF_FRAMES    10     /* frames for a led flashing pattern */


/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
 
typedef struct 
{
    SYSFUN_DECLARE_CSC_ON_SHMEM
    /* not used
    UI8_T    LEDDRV_System1_Display_Pattern[LEDDRV_NUM_OF_FRAMES];
     */
    UI8_T    LEDDRV_System0_Display_Pattern[LEDDRV_NUM_OF_FRAMES];
#ifdef LEDDRV_SUPPORT_7_SEGMENT_LED
    UI8_T    LEDDRV_System7LED_Display_Pattern[LEDDRV_NUM_OF_FRAMES + 2];
#endif
    UI8_T    my_board_id;
    BOOL_T   fan_fault[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT];
    BOOL_T   thermal_fault[SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT];
    BOOL_T   psu_fault[SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT];
    UI32_T   sem_id;
    UI32_T   my_unit_id;
} LEDDRV_Shmem_Data_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

#endif    /* LEDDRV_TYPE_H */

