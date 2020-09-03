/* MODULE NAME:  onlpdrv_sfp.c
 * PURPOSE:
 *   This module implements the sfp(Small Form-factor Pluggable) related wrapper
 *   functions which call ONLP functions.
 *
 * NOTES:
 *
 * HISTORY
 *    10/30/2015 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation , 2015
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_SYSDRV_USE_ONLP != TRUE)
#error "onlpdrv_sfp.c can only be used when SYS_CPNT_SYSDRV_USE_ONLP is TRUE."
#endif

#include "sysfun.h"

#include "stktplg_board_util.h"
#include "uc_mgr.h"

#include "onlpdrv_sfp.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define ONLP_SFPUTIL_INFO_DATA_READ_SIZE 256 /* refer to source code of onlp_sfputil.c */

/* MACRO FUNCTION DECLARATIONS
 */
#define DEBUG_MSG(fmtstr, ...) do { \
    if(debug_flag == TRUE) \
        {printf("%s(%d)"fmtstr, __FUNCTION__, __LINE__, ##__VA_ARGS__);} \
} while (0)

/* DATA TYPE DECLARATIONS
 */
typedef struct onlp_sfputil_BinaryOutputHeader_S
{
    int rc;
    int output_data_sz;
} onlp_sfputil_BinaryOutputHeader_S;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T ONLPDRV_SFP_GetTxDisableValue(UI32_T unit, UI32_T port, int *tx_disable_bmp_p);
static I16_T ONLPDRV_SFP_ConvertAOSUPortToONLPPort(UI32_T unit, UI32_T port);
    
/* STATIC VARIABLE DECLARATIONS
 */
static BOOL_T is_init=FALSE;
static BOOL_T debug_flag=FALSE;
static I16_T aos_uport_to_onlp_port_ar[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT]={(I16_T)ONLP_SFP_INVALID_PORT_ID};
static BOOL_T port_tx_disable[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];

static UI32_T board_id;

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: ONLPDRV_SFP_Init
 *-----------------------------------------------------------------------------
 * PURPOSE: This function is used to do initialization for this module.
 *-----------------------------------------------------------------------------
 * INPUT   : aos_uport_to_onlp_port - array for mapping aos uport id to onlp
 *                                    port id. All unused ports or non-sfp ports
 *                                    should be filled with ONLP_SFP_INVALID_PORT_ID.
 *                                    For example:
 *                                    If aos port id 1 is a sfp port and its
 *                                    corresponding onlp sfp port id is 0, then
 *                                    aos_uport_to_onlp_port[0] should be set as 0.
 *                                    If aos port id 2 is not a sfp port, then
 *                                    aos_uport_to_onlp_port[1] should be set as
 *                                    ONLP_SFP_INVALID_PORT_ID.
 *                                    If the maximum aos port id is 48, then all
 *                                    of the array elements after(including)
 *                                    aos_uport_to_onlp_port[48] should be set
 *                                    as ONLP_SFP_INVALID_PORT_ID when there is
 *                                    any unused elements in the tail of the array.
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES:
 */
BOOL_T ONLPDRV_SFP_Init(I16_T aos_uport_to_onlp_port[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT])
{
    UC_MGR_Sys_Info_T sys_info;

    if (!UC_MGR_GetSysInfo(&sys_info))
    {
        printf("%s(): Get UC System Information Fail.\r\n", __FUNCTION__);
        return;
    }

    board_id = sys_info.board_id;

    if (aos_uport_to_onlp_port==NULL)
    {
        printf("%s(%d)aos_uport_to_onlp_port is NULL.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    /* only need to do init for once
     */
    if (is_init==FALSE)
    {
        is_init=TRUE;

        if (getenv("onlpdrv_sfp_debug"))
        {
            debug_flag=TRUE;
        }

        memcpy(aos_uport_to_onlp_port_ar, aos_uport_to_onlp_port,
            sizeof(I16_T)*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT);
        DEBUG_MSG("Init done.\r\n");
    }
    return TRUE;
}

/* FUNCTION NAME: ONLPDRV_SFP_SetPortSfpTxDisable
 *-----------------------------------------------------------------------------
 * PURPOSE: This function is used set SFP TX_DISABLE state of the specified port
 *-----------------------------------------------------------------------------
 * INPUT   : unit        - unit id
 *           port        - aos port id
 *           tx_disable  - the TX_DISABLE state to be set to the specified port
 *                         TRUE : turn on TX_DISABLE
 *                         FALSE: turn off TX_DISABLE
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES   : The operation is not allowed to perform on non-sfp port.
 */
BOOL_T ONLPDRV_SFP_SetPortSfpTxDisable(UI32_T unit, UI32_T port, BOOL_T tx_disable)
{
    UI32_T rc;
    int shell_exit_status=0;
    char  shell_cmd_buf[40];
    char* generated_shell_cmd_p=NULL;
    I16_T onlp_port_id;
    BOOL_T ret_val=TRUE;
    UI8_T tx_disable_bmp=0;

    onlp_port_id=ONLPDRV_SFP_ConvertAOSUPortToONLPPort(unit, port);

    if (onlp_port_id==ONLP_SFP_INVALID_PORT_ID)
    {
        DEBUG_MSG("Failed to convert to onlp port id.(unit %lu, port %lu)\r\n", (unsigned long)unit, (unsigned long)port);
        return FALSE;
    }

    port_tx_disable[port-1] = tx_disable;

    if (STKTPLG_BOARD_UTIL_IsBreakOutPort(board_id, port))
    {
        UI32_T breakout_port_min, breakout_port_max;

        if (STKTPLG_BOARD_UTIL_GetBreakOutGroupPortIdRange(board_id, port,
                       &breakout_port_min, &breakout_port_max))
        {
            UI32_T tmp_port;
            for(tmp_port = breakout_port_min; tmp_port <= breakout_port_max; tmp_port++)
            {
                tx_disable_bmp |=  port_tx_disable[tmp_port-1] << (tmp_port-breakout_port_min);
            }
        }
    }
    else
    {
        tx_disable_bmp = tx_disable;
    }

    snprintf(shell_cmd_buf, sizeof(shell_cmd_buf), "/usr/bin/onlp_sfputil -p%d -t%#x",
        (int)onlp_port_id, tx_disable_bmp);
    generated_shell_cmd_p=SYSFUN_GetRealAOSShellCmd(shell_cmd_buf);
    if (generated_shell_cmd_p==NULL)
    {
        printf("%s(%d)SYSFUN_GetRealAOSShellCmd returns error.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    rc = SYSFUN_ExecuteSystemShellEx(generated_shell_cmd_p, &shell_exit_status);

    if (rc!=SYSFUN_OK)
    {
        DEBUG_MSG("Execute shell cmd '%s' error.(rc=%lu)\r\n", generated_shell_cmd_p, rc);

        if (shell_exit_status!=0)
        {
            DEBUG_MSG("Shell cmd '%s' returns error.(shell exit status=%d)\r\n", generated_shell_cmd_p, shell_exit_status);
        }
        ret_val=FALSE;
        goto error_exit;
    }

    DEBUG_MSG("Unit %lu port %lu set TX_DISABLE as %d OK.\r\n", (unsigned long)unit, (unsigned long)port,
        (int)tx_disable);

error_exit:
    if (generated_shell_cmd_p)
        free(generated_shell_cmd_p);

    return ret_val;
}

/* FUNCTION NAME: ONLPDRV_SFP_UpdatePortSfpTxDisable
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will configure the sfp tx enable setting kept in the
 *          shadow database to the hardware device.
 *-----------------------------------------------------------------------------
 * INPUT   : unit        - unit id
 *           port        - aos port id
 * OUTPUT  : none
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES   : 1. This function returns FALSE when the specified port is invalid or
 *              is not a sfp port.
 *           2. This function does not support stacking yet.
 */
BOOL_T ONLPDRV_SFP_UpdatePortSfpTxDisable(UI32_T unit, UI32_T port)
{
    if (port < 1 || SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT < port)
    {
        return FALSE;
    }

    return ONLPDRV_SFP_SetPortSfpTxDisable(unit, port, port_tx_disable[port-1]);
}

/* FUNCTION NAME: ONLPDRV_SFP_GetSfpPresentStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Get sfp present status of the specified port
 *-----------------------------------------------------------------------------
 * INPUT   : unit        - unit id
 *           port        - aos port id
 * OUTPUT  : is_present_p- TRUE : The SFP transceiver is present on the specified port.
 *                         FALSE: No SFP transceiver is detected on the specified port.
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES   : 1. This function returns FALSE when the specified port is invalid or
 *              is not a sfp port.
 *           2. This function does not support stacking yet.
 */
BOOL_T ONLPDRV_SFP_GetSfpPresentStatus(UI32_T unit, UI32_T port, UI8_T *is_present_p)
{
    char  shell_cmd_buf[40];
    char *generated_shell_cmd_p=NULL;
    char  cmd_output_buf[4]={'\0'};
    UI32_T cmd_output_buf_sz=sizeof(cmd_output_buf);
    int    cmd_output_val;
    I16_T onlp_port_id;
    BOOL_T rc, ret_val=TRUE;

    if (is_present_p==NULL)
    {
        printf("%s(%d)is_present_p is NULL.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    onlp_port_id=ONLPDRV_SFP_ConvertAOSUPortToONLPPort(unit, port);

    if (onlp_port_id==ONLP_SFP_INVALID_PORT_ID)
    {
        DEBUG_MSG("Failed to convert to onlp port id.(unit %lu, port %lu)\r\n", unit, port);
        return FALSE;
    }

    snprintf(shell_cmd_buf, sizeof(shell_cmd_buf), "/usr/bin/onlp_sfputil -p%d -s",
        (int)onlp_port_id);
    generated_shell_cmd_p=SYSFUN_GetRealAOSShellCmd(shell_cmd_buf);
    if (generated_shell_cmd_p==NULL)
    {
        printf("%s(%d)SYSFUN_GetRealAOSShellCmd returns error.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    rc = SYSFUN_GetExecuteCmdOutput(generated_shell_cmd_p, &cmd_output_buf_sz, cmd_output_buf);
    
    if (rc==FALSE)
    {
        DEBUG_MSG("Execute shell cmd '%s' error.\r\n", generated_shell_cmd_p);
        ret_val=FALSE;
        goto error_exit;
    }

    DEBUG_MSG("Command output='%s'\r\n", cmd_output_buf);
    cmd_output_val=atoi(cmd_output_buf);
    if (cmd_output_val<0)
    {
        DEBUG_MSG("Output of the shell cmd '%s' indicates error.\r\n", generated_shell_cmd_p);
        ret_val=FALSE;
        goto error_exit;
    }

    *is_present_p=cmd_output_val;

error_exit:
    if (generated_shell_cmd_p)
        free(generated_shell_cmd_p);

    return ret_val;

}

/* FUNCTION NAME: ONLPDRV_SFP_AccessGbicInfo
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the raw data of the specified offset and gbic info type from
 *          the specified port.
 *-----------------------------------------------------------------------------
 * INPUT   : unit            - unit id
 *           port            - aos port id
 *           gbic_info_type  - Valid types are listed below:
 *                             ONLPDRV_GBIC_INFO_TYPE_BASIC (Read through I2C address 0x50)
 *                             ONLPDRV_GBIC_INFO_TYPE_DDM   (Read through I2C address 0x51)
 *           offset          - data register offset address
 *           size            - size of read/written data
 *           gbic_access_type- Valid types are listed below:
 *                             ONLPDRV_SFP_GBIC_ACCESS_TYPE_READ
 *                             ONLPDRV_SFP_GBIC_ACCESS_TYPE_WRITE (Not support yet)
 * OUTPUT  : info_p          - read/written data info
 * RETURN  : TRUE: Successfully, FALSE: Failed
 *-----------------------------------------------------------------------------
 * NOTES   : 1. Does not support ONLPDRV_SFP_GBIC_ACCESS_TYPE_WRITE because ONLP
 *              not support yet.
 */
BOOL_T ONLPDRV_SFP_AccessGbicInfo(UI32_T unit, UI32_T port, ONLPDRV_GBIC_INFO_TYPE_T gbic_info_type, UI16_T offset, UI8_T size, ONLPDRV_SFP_GBIC_ACCESS_TYPE_T gbic_access_type, UI8_T *info_p)
{
    char   shell_cmd_buf[44];
    char  *generated_shell_cmd_p=NULL;
    char   *cmd_output_buf_p=NULL;
    char   cmd_opt_info_type;
    UI32_T cmd_output_buf_sz;
    int    cmd_output_val;
    onlp_sfputil_BinaryOutputHeader_S *header_p=NULL;
    I16_T  onlp_port_id;
    BOOL_T rc, ret_val=TRUE;
    int    shell_exit_status;

    if (info_p==NULL)
    {
        printf("%s(%d)info_p is NULL.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    onlp_port_id=ONLPDRV_SFP_ConvertAOSUPortToONLPPort(unit, port);

    if (onlp_port_id==ONLP_SFP_INVALID_PORT_ID)
    {
        DEBUG_MSG("Failed to convert to onlp port id.(unit %lu, port %lu)\r\n", unit, port);
        return FALSE;
    }

    switch (gbic_info_type)
    {
        case ONLPDRV_GBIC_INFO_TYPE_BASIC:
            cmd_opt_info_type='e';
            break;
        case ONLPDRV_GBIC_INFO_TYPE_DDM:
            cmd_opt_info_type='m';
            break;
        default:
            printf("%s(%d)Unknown gbic info type(%d)\r\n", __FUNCTION__, __LINE__, (int)gbic_info_type);
            return FALSE;
    }

    switch (gbic_access_type)
    {
        case ONLPDRV_SFP_GBIC_ACCESS_TYPE_READ:
            break;
        case ONLPDRV_SFP_GBIC_ACCESS_TYPE_WRITE:
            if (gbic_info_type==ONLPDRV_GBIC_INFO_TYPE_DDM)
            {
                printf("%s(%d)Do not support I2C write operation to SFP transceiver addr 0x51.\r\n", __FUNCTION__, __LINE__);
                return FALSE;
            }

            cmd_opt_info_type='w';
            break;
        default:
            break;
            printf("%s(%d)Unknown gbic access type(%d)\r\n", __FUNCTION__, __LINE__, (int)gbic_access_type);
            return FALSE;
    }

    cmd_output_buf_sz=sizeof(onlp_sfputil_BinaryOutputHeader_S)+ONLP_SFPUTIL_INFO_DATA_READ_SIZE;
    cmd_output_buf_p=malloc(cmd_output_buf_sz);
    if (cmd_output_buf_p==NULL)
    {
        printf("%s(%d)malloc %lu bytes failed.\r\n", __FUNCTION__, __LINE__, cmd_output_buf_sz);
        return FALSE;
    }
    else
    {
        memset(cmd_output_buf_p, 0, cmd_output_buf_sz);
    }

    if (gbic_access_type == ONLPDRV_SFP_GBIC_ACCESS_TYPE_WRITE)
    {
        snprintf(shell_cmd_buf, sizeof(shell_cmd_buf), "/usr/bin/onlp_sfputil -p%d -%c -o%d -v%d",
            (int)onlp_port_id, cmd_opt_info_type, offset, (int)*info_p);
        generated_shell_cmd_p=SYSFUN_GetRealAOSShellCmd(shell_cmd_buf);
        if (generated_shell_cmd_p==NULL)
        {
            printf("%s(%d)SYSFUN_GetRealAOSShellCmd returns error\r\n", __FUNCTION__, __LINE__);
            ret_val=FALSE;
            goto error_exit;
        }

        rc = SYSFUN_ExecuteSystemShellEx(generated_shell_cmd_p, &shell_exit_status);
        DEBUG_MSG("shell cmd='%s'.\r\n", generated_shell_cmd_p);
        if (rc!=SYSFUN_OK)
        {
            DEBUG_MSG("Execute shell cmd error.\r\n");
            DEBUG_MSG("exit code = %d\r\n", shell_exit_status);
            ret_val=FALSE;
            goto error_exit;
        }
    }
    else
    {
        snprintf(shell_cmd_buf, sizeof(shell_cmd_buf), "/usr/bin/onlp_sfputil -p%d -%c",
            (int)onlp_port_id, cmd_opt_info_type);
        generated_shell_cmd_p=SYSFUN_GetRealAOSShellCmd(shell_cmd_buf);
        if (generated_shell_cmd_p==NULL)
        {
            printf("%s(%d)SYSFUN_GetRealAOSShellCmd returns error\r\n", __FUNCTION__, __LINE__);
            ret_val=FALSE;
            goto error_exit;
        }

        rc = SYSFUN_GetExecuteCmdOutput(generated_shell_cmd_p, &cmd_output_buf_sz, cmd_output_buf_p);
        DEBUG_MSG("shell cmd='%s'.cmd_output_buf_sz=%lu\r\n", generated_shell_cmd_p, cmd_output_buf_sz);
        if (rc==FALSE)
        {
            DEBUG_MSG("Execute shell cmd error.\r\n");
            DEBUG_MSG("cmd_output='%s'\r\n", cmd_output_buf_p);
            ret_val=FALSE;
            goto error_exit;
        }

        header_p=(onlp_sfputil_BinaryOutputHeader_S*)cmd_output_buf_p;
        if (header_p->rc!=0)
        {
            DEBUG_MSG("onlp_sfputil error(rc=%d, Unit %lu Port %lu)\r\n",
                header_p->rc, unit, port);
            ret_val=FALSE;
            goto error_exit;
        }

        DEBUG_MSG("offset=%hu size=%hu header_p->output_data_sz=%d\n", offset,
            size, header_p->output_data_sz);
        if ( (offset+size) > header_p->output_data_sz)
        {
            DEBUG_MSG("Specified info area cannot be read(Unit %lu Port %lu, offset=%hu, size=%hu)\r\n",
                unit, port, offset, size);
            ret_val=FALSE;
            goto error_exit;
        }
        memcpy(info_p, (char*)(header_p+1) + offset, size);
    }

error_exit:
    if (cmd_output_buf_p)
    {
        free(cmd_output_buf_p);
    }
    if (generated_shell_cmd_p)
    {
        free(generated_shell_cmd_p);
    }

    return ret_val;
}

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME: ONLPDRV_SFP_ConvertAOSUPortToONLPPort
 *-----------------------------------------------------------------------------
 * PURPOSE: Convert AOS user port id to ONLP port id.
 *-----------------------------------------------------------------------------
 * INPUT   : unit        - unit id
 *           port        - aos port id
 * OUTPUT  : None
 * RETURN  : ONLP port id. Returns ONLP_SFP_INVALID_PORT_ID when it fails to
 *           convert.
 *-----------------------------------------------------------------------------
 * NOTES   : 1. This function does not support stacking yet. Unit id is ignored for now.
 */
static I16_T ONLPDRV_SFP_ConvertAOSUPortToONLPPort(UI32_T unit, UI32_T port)
{
    if (port < 1 || SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT < port)
    {
        printf("%s(%d)Invalid port id %lu\r\n", __FUNCTION__, __LINE__, port);
        return ONLP_SFP_INVALID_PORT_ID;
    }

    DEBUG_MSG("Unit %lu port %lu --> ONLP port %d\r\n", unit, port, aos_uport_to_onlp_port_ar[port-1]);

    return aos_uport_to_onlp_port_ar[port-1];
}

