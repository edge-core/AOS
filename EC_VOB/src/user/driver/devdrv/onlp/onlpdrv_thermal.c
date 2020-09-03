/* MODULE NAME:  onlpdrv_thermal.c
 * PURPOSE:
 *   This module implements the thermal related wrapper functions
 *   which call ONLP functions.
 *
 * NOTES:
 *
 * HISTORY
 *    10/20/2015 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation , 2015
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>

#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_hwcfg_common.h"

#include "sysfun.h"
#if (SYS_CPNT_SYSDRV_USE_ONLP != TRUE)
#error "onlpdrv_thermal.c can only be used when SYS_CPNT_SYSDRV_USE_ONLP is TRUE."
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define DEBUG_MSG(fmtstr, ...) do { \
    if(debug_flag == TRUE) \
        {printf("%s(%d)"fmtstr, __FUNCTION__, __LINE__, ##__VA_ARGS__);} \
} while (0)

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T ONLPDRV_THERMAL_ChipInit(UI32_T thermal_idx);
static BOOL_T ONLPDRV_THERMAL_ChipGetTemperature(UI8_T thermal_idx, I8_T* temperature);

/* EXPORTED VARIABLE DECLARATIONS
 */
/* exported variable for thermal functions
 */
SYS_HWCFG_ThermalOps_T thermal_ops_onlp =
{
    ONLPDRV_THERMAL_ChipInit,
    ONLPDRV_THERMAL_ChipGetTemperature
};

/* STATIC VARIABLE DECLARATIONS
 */
static BOOL_T is_init=FALSE;
static BOOL_T debug_flag=FALSE;
static UI8_T  total_nbr_of_thermal=0;

/* EXPORTED SUBPROGRAM BODIES
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME: ONLPDRV_THERMAL_ChipInit
 *-----------------------------------------------------------------------------
 * PURPOSE: This function is used to do thermal init for the given thermal index
 *-----------------------------------------------------------------------------
 * INPUT   : thermal_idx    - system-wised thermal index (start from 1)
 * OUTPUT  : 
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T ONLPDRV_THERMAL_ChipInit(UI32_T thermal_idx)
{
    const char* shell_cmd_base_p ="/usr/bin/onlp_query -t";
    char* generated_shell_cmd_p=NULL;
    char output_buf[32];
    UI32_T output_buf_size=sizeof(output_buf);

    /* only need to do init for once
     */
    if (is_init==FALSE)
    {
        is_init=TRUE;

        if (getenv("onlpdrv_thermal_debug"))
        {
            debug_flag=TRUE;
        }
        memset(output_buf, 0, sizeof(output_buf));
        generated_shell_cmd_p=SYSFUN_GetRealAOSShellCmd(shell_cmd_base_p);
        if (generated_shell_cmd_p==NULL)
        {
            printf("%s(%d):SYSFUN_GetRealAOSShellCmd returns error.\r\n", __FUNCTION__, __LINE__);
            return FALSE;
        }

        if (SYSFUN_GetExecuteCmdOutput(generated_shell_cmd_p, &output_buf_size, output_buf)==FALSE)
        {
            DEBUG_MSG("%s(%d):Execute '%s' error.\r\n", __FUNCTION__, __LINE__, generated_shell_cmd_p);
            free(generated_shell_cmd_p);
            return FALSE;
        }
        total_nbr_of_thermal=atoi(output_buf);
        DEBUG_MSG("Total number of thermal=%d\r\n", (int)total_nbr_of_thermal);
        free(generated_shell_cmd_p);
    }

    return TRUE;
}

/* FUNCTION NAME: ONLPDRV_THERMAL_ChipGetTemperature
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the temperature in Celsius degree from the given thermal index
 *-----------------------------------------------------------------------------
 * INPUT   : thermal_idx    - system-wised thermal index (start from 1)
 * OUTPUT  : temperature    - the temperature in Celsius degree from the given thermal index
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T ONLPDRV_THERMAL_ChipGetTemperature(UI8_T thermal_idx, I8_T* temperature)
{
    int rc, temp_mcelsius;
    char shell_cmd[40];
    char* generated_shell_cmd_p=NULL;
    char output_buf[32];
    UI32_T output_buf_size=sizeof(output_buf);

    if (thermal_idx==0 || thermal_idx>total_nbr_of_thermal)
    {
        DEBUG_MSG("Invalid thermal idx %hu\r\n", thermal_idx);
        return FALSE;
    }

    if (temperature==NULL)
    {
        DEBUG_MSG("temperature is NULL\r\n");
        return FALSE;
    }

    snprintf(shell_cmd, sizeof(shell_cmd), "/usr/bin/onlp_thermalutil -t%hu", thermal_idx);
    generated_shell_cmd_p=SYSFUN_GetRealAOSShellCmd(shell_cmd);
    if (generated_shell_cmd_p==NULL)
    {
        printf("%s(%d):SYSFUN_GetRealAOSShellCmd returns error.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    DEBUG_MSG("Run shell command:'%s'\r\n", generated_shell_cmd_p);

    memset(output_buf, 0, sizeof(output_buf));
    if (SYSFUN_GetExecuteCmdOutput(generated_shell_cmd_p, &output_buf_size, output_buf)==FALSE)
    {
        DEBUG_MSG("%s(%d):Execute '%s' error.\r\n", __FUNCTION__, __LINE__, generated_shell_cmd_p);
        free(generated_shell_cmd_p);
        return FALSE;
    }

    DEBUG_MSG("Command output='%s'\r\n", output_buf);

    sscanf(output_buf, "%d %d", &rc, &temp_mcelsius);
    if (rc>=0)
    {
        *temperature = (I8_T)(temp_mcelsius/1000);
        
        if ( (temp_mcelsius % 1000) >= 500)
        {
            *temperature += 1;
        }
    }

    free(generated_shell_cmd_p);
    return TRUE;
}
