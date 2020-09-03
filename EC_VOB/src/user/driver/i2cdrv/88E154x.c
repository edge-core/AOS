/* MODULE NAME:  88E154x.c
 * PURPOSE:
 *     Device driver for MARVELL thermal registers of PHY chipset.
 *
 * NOTES:
 *     Current support MARVELL PHY chips is listed below:
 *         88E1540 Family:
 *           - 88E1543
 *           - 88E1540
 *
 * HISTORY
 *    12/18/2013 - Weihsiang Huang, Copy and Modify source from lm.c.
 *
 * Copyright(C)    Accton Corporation, 2013
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_hwcfg.h"
#include "i2cdrv.h"
#include "sysdrv.h"
#include "dev_swdrv.h"
#include "dev_swdrv_pmgr.h"
#include "stktplg_om.h"
#include "sysfun.h"
#include "uc_mgr.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
//static I8_T LM_TranslateRawDataToTemperature(UI8_T raw_data[]);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - __SYSDRV_THERMAL_CHIP_Init
 *---------------------------------------------------------------------------
 * PURPOSE:  Do initialization on chip.
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   TRUE - The chip is initialized sucessfully.
 * NOTE:     Use compiler control to define seperate __SYSDRV_THERMAL_CHIP_Init
 *           if required.
 *---------------------------------------------------------------------------
 */
BOOL_T __SYSDRV_THERMAL_CHIP_Init(void)
{
    /* Not implement yet.
     */
    return TRUE;
}

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_THERMAL_CHIP_Init
 *---------------------------------------------------------------------------
 * PURPOSE:  Do initialization on chip.
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   TRUE - The chip is initialized sucessfully.
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_THERMAL_CHIP_Init(void)
{
    return __SYSDRV_THERMAL_CHIP_Init();
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_THERMAL_GetTemperature
 *---------------------------------------------------------------------------
 * PURPOSE:  get cuurent temperature from Thermal
 * INPUT:    index: Thermal index (starts by 1)
 * OUTPUT:   temperature: Thermal sensor temperature (Celsius)
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_THERMAL_GetTemperature(UI8_T index, I8_T *temperature)
{
    UI32_T unit = 0xFF, board_id = 0xFF;
    UI32_T temp_temperature = 0;

#if !defined(INCLUDE_DIAG)
    UC_MGR_Sys_Info_T uc_sys_info;
#endif


    *temperature = 0;
    if ((index < 1) || (index > SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT))
    {
        printf("%s(%d): Invalid index(%hu).\r\n", __FUNCTION__, __LINE__, index);
        return FALSE;
    }

    if(temperature == NULL)
    {
        printf("%s(%d): Invalid temperature.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (STKTPLG_OM_GetMyUnitID(&unit) == FALSE)
    {
        printf("%s(%d): Get System Unit ID Fail.\r\n", __FUNCTION__, __LINE__);

        /* Severe problem, while loop here.
         */
        while (TRUE);
    }

#if !defined(INCLUDE_DIAG)
    if (UC_MGR_GetSysInfo(&uc_sys_info) == FALSE)
    {
        printf("%s(%d): Get UC System Information Fail.\r\n", __FUNCTION__, __LINE__);

        /* Severe problem, while loop here.
         */
        while (TRUE);
    }
    board_id = uc_sys_info.board_id;
#else
    if (STKTPLG_OM_GetUnitBoardID(unit, &board_id) == FALSE)
    {
        printf("%s(%d): Get System Board ID Fail.\r\n", __FUNCTION__, __LINE__);

        /* Severe problem, while loop here.
         */
        while (TRUE);
    }
#endif

#if (SYS_HWCFG_THERMAL_TYPE == SYS_HWCFG_THERMAL_MARVELL_PHY_88E154X)

    /* Read average temperature from thermal register of PHY chipset.
     */
    if (DEV_SWDRV_PMGR_GetAllPortAverageTemperature(&temp_temperature) == FALSE) 
    {
        printf("%s(%d): Get average temperature Fail.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
#else
    return FALSE;
#endif

   *temperature = (UI8_T) temp_temperature;
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_THERMAL_SetThreshold
 *---------------------------------------------------------------------------
 * PURPOSE:  set Thermal trap value
 * INPUT:    index: Thermal index.             
 * OUTPUT:   temperature:set Thermal sensor temperature
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_THERMAL_SetThreshold(UI8_T index, I8_T  temperature) 
{
    /* Not implement yet.
     */
    return TRUE;
}

/* LOCAL SUBPROGRAM BODIES
 */

