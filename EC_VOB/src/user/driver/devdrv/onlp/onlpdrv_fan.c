/* MODULE NAME:  onlpdrv_fan.c
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
 
/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>

#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_hwcfg_common.h"
#include "leaf_es3626a.h"

#if (SYS_CPNT_SYSDRV_USE_ONLP == FALSE)
#error "onlpdrv_fan.c can only be used when SYS_CPNT_SYSDRV_USE_ONLP is TRUE."
#endif

#include "sysfun.h"

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

/* fan controller functions
 */
static BOOL_T ONLPDRV_Fan_ChipInit(UI32_T fan_idx);
static BOOL_T ONLPDRV_Fan_ChipGetSpeedInRpm(UI8_T fan_idx, UI32_T* speed);
static BOOL_T ONLPDRV_Fan_ChipGetSpeedInDutyCycle(UI8_T fan_idx, UI32_T* duty_cycle_p);
static BOOL_T ONLPDRV_Fan_ChipSetSpeed(UI8_T fan_idx, UI32_T speed);

/* EXPORTED VARIABLE DECLARATIONS
 */
/* exported variable for fan controller functions
 */
SYS_HWCFG_FanControllerOps_T fan_controller_ops_onlp =
{
    ONLPDRV_Fan_ChipInit,
    ONLPDRV_Fan_ChipGetSpeedInRpm,
    ONLPDRV_Fan_ChipGetSpeedInDutyCycle,
    ONLPDRV_Fan_ChipSetSpeed,
};

/* STATIC VARIABLE DECLARATIONS
 */
static BOOL_T is_init=FALSE;
static BOOL_T debug_flag=FALSE;
static UI8_T  total_nbr_of_fan=0;

/* EXPORTED SUBPROGRAM BODIES
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
BOOL_T ONLPDRV_FAN_GetAllFanStatus(UI32_T *fan_status_ar_sz_p, UI32_T fan_status_ar[])
{
    char shell_cmd[40];
    char *generated_shell_cmd_p=NULL;
    char output_buf[32];
    UI32_T output_buf_size=sizeof(output_buf);
    int rc, present_status, failed_status;
    UI8_T fan_id;

    if (fan_status_ar_sz_p==NULL)
    {
        DEBUG_MSG("fan_status_ar_sz_p is NULL\r\n");
        return FALSE;
    }

    if (fan_status_ar==NULL)
    {
        DEBUG_MSG("fan_status_ar is NULL\r\n");
        return FALSE;
    }

    if (*fan_status_ar_sz_p > total_nbr_of_fan)
    {
        *fan_status_ar_sz_p=total_nbr_of_fan;
    }

    for (fan_id=1; fan_id<=*fan_status_ar_sz_p; fan_id++)
    {
        snprintf(shell_cmd, sizeof(shell_cmd), "/usr/bin/onlp_fanutil -s%hu", fan_id);
        generated_shell_cmd_p=SYSFUN_GetRealAOSShellCmd(shell_cmd);
        if (generated_shell_cmd_p==NULL)
        {
            printf("%s(%d):SYSFUN_GetRealAOSShellCmd returns error.(fan_id=%d)\r\n",
                __FUNCTION__, __LINE__, (int)fan_id);
            continue;
        }

        DEBUG_MSG("Run shell command:'%s'\r\n", generated_shell_cmd_p);
        memset(output_buf, 0, sizeof(output_buf));
        if (SYSFUN_GetExecuteCmdOutput(generated_shell_cmd_p, &output_buf_size, output_buf)==FALSE)
        {
            DEBUG_MSG("%s(%d):Execute shell command error.\r\n", __FUNCTION__, __LINE__);
            free(generated_shell_cmd_p);
            return FALSE;
        }
        free(generated_shell_cmd_p);
        generated_shell_cmd_p=NULL;

        sscanf(output_buf, "%d %d %d", &rc, &present_status, &failed_status);
        if (rc>=0)
        {
                fan_status_ar[fan_id-1]=VAL_switchFanStatus_ok;
                if (present_status)
                {
                    DEBUG_MSG("fan id %hu is present\r\n", fan_id);
                }
                else
                {
                    DEBUG_MSG("fan id %hu is not present\r\n", fan_id);
                    fan_status_ar[fan_id-1]=VAL_switchFanStatus_failure;
                }

                if (failed_status)
                {
                    fan_status_ar[fan_id-1]=VAL_switchFanStatus_failure;
                    DEBUG_MSG("fan id %hu failed status detected\r\n", fan_id);
                }
                else
                {
                    DEBUG_MSG("fan id %hu is OK\r\n", fan_id);
                }
        }
        else
        {
            DEBUG_MSG("onlp_fan_info_get error.(rc=%d, fan id=%hu)\r\n", rc, fan_id);
            return FALSE;
        }
    }

    return TRUE;
}

/* LOCAL SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - ONLPDRV_Fan_ChipInit
 *---------------------------------------------------------------------------
 * PURPOSE: This function is used to do fan init for the given fan index
 * INPUT:   fan_idx        -  system-wised fan index (start from 1)
 * OUTPUT:
 * RETURN:  TRUE  -  Sucess
 *          FALSE -  Failure
 *---------------------------------------------------------------------------
 * NOTE:
 */
static BOOL_T ONLPDRV_Fan_ChipInit(UI32_T fan_idx)
{
    const char* shell_cmd_base_p ="/usr/bin/onlp_query -f";
    char *generated_shell_cmd_p=NULL;
    char output_buf[32];
    UI32_T output_buf_size=sizeof(output_buf);

    /* only need to do init for once
     */
    if (is_init==FALSE)
    {
        is_init=TRUE;

        if (getenv("onlpdrv_fan_debug"))
        {
            debug_flag=TRUE;
        }
        memset(output_buf, 0, sizeof(output_buf));
        generated_shell_cmd_p=SYSFUN_GetRealAOSShellCmd(shell_cmd_base_p);
        if(generated_shell_cmd_p==NULL)
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
        total_nbr_of_fan=atoi(output_buf);

        DEBUG_MSG("Total number of fan=%hu\r\n", total_nbr_of_fan);

        free(generated_shell_cmd_p);
    }

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - ONLPDRV_Fan_ChipGetSpeedInRpm
 *---------------------------------------------------------------------------
 * PURPOSE: This function is used to get fan speed in RPM for the given fan index
 * INPUT:   fan_idx        - system-wised fan index (start from 1)
 * OUTPUT:  speed          - fan speed in RPM.
 * RETURN:  TRUE  -  Sucess
 *          FALSE -  Failure
 *---------------------------------------------------------------------------
 * NOTE:
 */   
static BOOL_T ONLPDRV_Fan_ChipGetSpeedInRpm(UI8_T fan_idx, UI32_T* speed)
{
    int rpm;
    char shell_cmd[40];
    char *generated_shell_cmd_p=NULL;
    char output_buf[32];
    UI32_T output_buf_size=sizeof(output_buf);
    BOOL_T ret_val=TRUE;

    if (fan_idx>total_nbr_of_fan)
    {
        DEBUG_MSG("Illegal fan id %hu(max fan id=%hu)\r\n", fan_idx, total_nbr_of_fan);
        return FALSE;
    }

    if (speed==NULL)
    {
        DEBUG_MSG("Invalid argument 'speed'(fan id=%hu)\r\n", fan_idx);
        return FALSE;
    }

    snprintf(shell_cmd, sizeof(shell_cmd), "/usr/bin/onlp_fanutil -r%hu", fan_idx);
    generated_shell_cmd_p=SYSFUN_GetRealAOSShellCmd(shell_cmd);
    if (generated_shell_cmd_p==NULL)
    {
        printf("%s(%d):SYSFUN_GetRealAOSShellCmd returns error.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    DEBUG_MSG("Run shell command:'%s'\r\n", generated_shell_cmd_p);

    output_buf[0]=0;
    if (SYSFUN_GetExecuteCmdOutput(generated_shell_cmd_p, &output_buf_size, output_buf)==FALSE)
    {
        DEBUG_MSG("%s(%d):Execute '%s' error.\r\n", __FUNCTION__, __LINE__, generated_shell_cmd_p);
        ret_val=FALSE;
        goto error_exit;
    }

    DEBUG_MSG("Command output='%s'\r\n", output_buf);

    sscanf(output_buf, "%d", &rpm);
    if (rpm<0)
    {
        DEBUG_MSG("Fan %hu does not support.\r\n", fan_idx);
        ret_val=FALSE;
        goto error_exit;
    }
    *speed=rpm;

error_exit:
    if (generated_shell_cmd_p)
        free(generated_shell_cmd_p);

    return ret_val;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - ONLPDRV_Fan_ChipGetSpeedInDutyCycle
 *---------------------------------------------------------------------------
 * PURPOSE: This function is used to get fan speed in duty cycle for the given fan index
 * INPUT:   fan_idx        - system-wised fan index (start from 1)
 * OUTPUT:  duty_cycle_p   - fan speed in duty cycle.
 * RETURN:  TRUE  -  Sucess
 *          FALSE -  Failure
 *---------------------------------------------------------------------------
 * NOTE:
 */   
static BOOL_T ONLPDRV_Fan_ChipGetSpeedInDutyCycle(UI8_T fan_idx, UI32_T* duty_cycle_p)
{
    int duty_cycle_local;
    char shell_cmd[40];
    char *generated_shell_cmd_p=NULL;
    char output_buf[32];
    UI32_T output_buf_size=sizeof(output_buf);
    BOOL_T ret_val=TRUE;

    if (fan_idx>total_nbr_of_fan)
    {
        DEBUG_MSG("Illegal fan id %hu(max fan id=%hu)\r\n", fan_idx, total_nbr_of_fan);
        return FALSE;
    }

    if (duty_cycle_p==NULL)
    {
        DEBUG_MSG("Invalid argument 'duty_cycle_p'(fan id=%hu)\r\n", fan_idx);
        return FALSE;
    }

    snprintf(shell_cmd, sizeof(shell_cmd), "/usr/bin/onlp_fanutil -c%hu", fan_idx);
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
        ret_val=FALSE;
        goto error_exit;
    }

    DEBUG_MSG("Command output='%s'\r\n", output_buf);

    sscanf(output_buf, "%d", &duty_cycle_local);
    if (duty_cycle_local<0)
    {
        DEBUG_MSG("Fan %hu does not support.\r\n", fan_idx);
        ret_val=FALSE;
        goto error_exit;
    }
    *duty_cycle_p=duty_cycle_local;

error_exit:
    if (generated_shell_cmd_p)
        free(generated_shell_cmd_p);

    return ret_val;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - ONLPDRV_Fan_ChipSetSpeed
 *---------------------------------------------------------------------------
 * PURPOSE: This function is used to set fan speed in duty cycle for the given fan index
 * INPUT:   fan_idx        - system-wised fan index (start from 1)
 * OUTPUT:  speed          - fan speed in duty cycle.
 * RETURN:  TRUE  -  Sucess
 *          FALSE -  Failure
 *---------------------------------------------------------------------------
 * NOTE:
 */   
static BOOL_T ONLPDRV_Fan_ChipSetSpeed(UI8_T fan_idx, UI32_T speed)
{
    UI32_T rc;
    char shell_cmd[40];
    char *generated_shell_cmd_p=NULL;
    int shell_exit_status=-1;

    if (fan_idx>total_nbr_of_fan)
    {
        DEBUG_MSG("Illegal fan id %hu(max fan id=%hu)\r\n", fan_idx, total_nbr_of_fan);
        return FALSE;
    }

    if (speed>100)
    {
        DEBUG_MSG("Illegal speed %lu for fan %hu(max value=100)\r\n", speed, fan_idx);
        return FALSE;
    }

    snprintf(shell_cmd, sizeof(shell_cmd), "/usr/bin/onlp_fanutil -f%hu.%d", fan_idx, (int)speed);
    generated_shell_cmd_p=SYSFUN_GetRealAOSShellCmd(shell_cmd);
    if (generated_shell_cmd_p==NULL)
    {
        printf("%s(%d):SYSFUN_GetRealAOSShellCmd returns error.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    DEBUG_MSG("Run shell command:'%s'\r\n", shell_cmd);

    rc = SYSFUN_ExecuteSystemShellEx(shell_cmd, &shell_exit_status);
    if (rc!=SYSFUN_OK)
    {
        DEBUG_MSG("Execute shell cmd error.(rc=%lu)\r\n", rc);
        free(generated_shell_cmd_p);
        return FALSE;
    }

    if (shell_exit_status!=0)
    {
        DEBUG_MSG("Shell cmd returns error.(shell exit status=%d)\r\n", shell_exit_status);
        free(generated_shell_cmd_p);
        return FALSE;
    }

    free(generated_shell_cmd_p);
    return TRUE;
}

