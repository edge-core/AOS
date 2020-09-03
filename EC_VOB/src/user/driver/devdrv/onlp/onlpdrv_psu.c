/* MODULE NAME:  onlpdrv_psu.c
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
 
/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_hwcfg_common.h"

#include "onlpdrv_psu.h"
#if (SYS_CPNT_SYSDRV_USE_ONLP != TRUE)
#error "onlpdrv_psu.c can only be used when SYS_CPNT_SYSDRV_USE_ONLP is TRUE."
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

/* EXPORTED VARIABLE DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static BOOL_T is_init=FALSE;
static BOOL_T debug_flag=FALSE;
static UI8_T  total_nbr_of_psu=0;

/* EXPORTED SUBPROGRAM BODIES
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
BOOL_T ONLPDRV_PSU_Init(void)
{
    const char* shell_cmd_base_p ="/usr/bin/onlp_query -p";
    char *generated_shell_cmd_p=NULL;
    char output_buf[32];
    UI32_T output_buf_size=sizeof(output_buf);

    /* only need to do init for once
     */
    if (is_init==FALSE)
    {
        is_init=TRUE;

        if (getenv("onlpdrv_psu_debug"))
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
        total_nbr_of_psu=atoi(output_buf);

        DEBUG_MSG("Total number of psu=%hu\r\n", total_nbr_of_psu);
        free(generated_shell_cmd_p);
    } /* if (is_init==FALSE) */

    return TRUE;

}

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
BOOL_T ONLPDRV_PSU_GetPowerStatus(UI32_T power_index, UI8_T *power_status_p)
{
    char shell_cmd[40];
    char *generated_shell_cmd_p=NULL;
    char output_buf[32];
    UI32_T output_buf_size=sizeof(output_buf);
    int rc, present_status, failed_status, unplugged_status;
    BOOL_T ret_val=TRUE;

    if (power_index>total_nbr_of_psu)
    {
        DEBUG_MSG("Illegal psu id %lu(max psu id=%hu)\r\n", power_index, total_nbr_of_psu);
        return FALSE;
    }

    if (power_status_p==NULL)
    {
        DEBUG_MSG("Illegal argument 'power_status_p'(psu id=%lu)\r\n", power_index);
        return FALSE;
    }

    snprintf(shell_cmd, sizeof(shell_cmd), "/usr/bin/onlp_psuutil -p%lu", power_index);
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
        DEBUG_MSG("%s(%d):Execute shell command error.\r\n", __FUNCTION__, __LINE__);
        ret_val=FALSE;
        goto error_exit;
    }

    DEBUG_MSG("Command output='%s'", output_buf);
    sscanf(output_buf, "%d %d %d %d", &rc, &present_status, &failed_status, &unplugged_status);
    if (rc>=0)
    {
        *power_status_p=0;
        if (present_status)
        {
            DEBUG_MSG("Present.\r\n");
            *power_status_p |= ONLPDRV_PSU_STATUS_PRESENT;
        }
        else
        {
            DEBUG_MSG("Not Present.\r\n");
        }

        if (failed_status)
        {
            DEBUG_MSG("Power failure detected.\r\n");
            *power_status_p |= ONLPDRV_PSU_STATUS_FAILED;
        }
        else
        {
            DEBUG_MSG("Power status OK.\r\n");
        }

        if (unplugged_status)
        {
            DEBUG_MSG("Power down.\r\n");
            *power_status_p |= ONLPDRV_PSU_STATUS_UNPLUGGED;
        }
        else
        {
            DEBUG_MSG("Power up.\r\n");
        }
    }

error_exit:
    if (generated_shell_cmd_p)
        free(generated_shell_cmd_p);

    return ret_val;
}

/* LOCAL SUBPROGRAM BODIES
 */

