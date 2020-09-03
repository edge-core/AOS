/* Project Name: ES3526V-60 
 * Module Name : XFER_OM.C
 * Purpose     : 
 *
 * History :                                                               
 *          Date        Modifier        Reason
 *          2002/07/24  Erica Li        Create this file
 *          2002/08/15  Erica Li        Add XFER_OM_GetTftpRetryTimes()
 *                                      Add XFER_OM_SetTftpRetryTimes()
 *
 * Copyright(C)      Accton Corporation, 1999, 2000 
 *
 * Note    : 
 */

 /* INCLUDE FILE    DECLARATIONS
 */
#include <string.h>
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_type.h"
#include "sys_dflt.h"
#include "sysfun.h"
#include "xfer_om.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct XFER_OM_TftpCfg_S
{
    UI32_T retry_times;
    UI32_T timeout;     /* timeout in seconds before retry */
}XFER_OM_TftpCfg_T;

#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
typedef struct XFER_OM_AutoUpgradeCfg_S
{
    UI32_T opcode_status;
    UI32_T opcode_reload_status;
    char   opcode_path[MAXSIZE_fileAutoUpgradeOpCodePath + 1];
}XFER_OM_AutoUpgradeCfg_T;
#endif

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* LOCAL VARIABLES
 */
static XFER_OM_TftpCfg_T tftp_cfg;

#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
static XFER_OM_AutoUpgradeCfg_T auto_upg_cfg;
#endif

/* EXPORTED SUBPROGRAM BODIES   
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - XFER_OM_Init
 * -------------------------------------------------------------------------
 * FUNCTION: Initialize XFER OM
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void XFER_OM_Init(void)
{
    tftp_cfg.retry_times = SYS_DFLT_TFTP_NUMBER_OF_RETRIES;
    tftp_cfg.timeout = XFER_TYPE_DFLT_TFTP_TIMEOUT;

#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
    auto_upg_cfg.opcode_status = SYS_DFLT_XFER_AUTO_UPGRADE_OPCODE_STATUS;
    auto_upg_cfg.opcode_reload_status = SYS_DFLT_XFER_AUTO_UPGRADE_OPCODE_RELOAD_STATUS;
    strcpy(auto_upg_cfg.opcode_path, SYS_DFLT_XFER_AUTO_UPGRADE_OPCODE_PATH);
#endif

    return;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - XFER_OM_EnterMasterMode
 *---------------------------------------------------------------------------
 * PURPOSE:  Assign default value for XFER OM
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *--------------------------------------------------------------------------
 */
void XFER_OM_EnterMasterMode(void)
{
    return;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_OM_GetTftpRetryTimes
 *------------------------------------------------------------------------
 * FUNCTION: Retrieves TFTP retry times
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TFTP retry times
 * NOTE    : None
 *------------------------------------------------------------------------
 */
UI32_T XFER_OM_GetTftpRetryTimes()
{
    return tftp_cfg.retry_times;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_OM_SetTftpRetryTimes
 *------------------------------------------------------------------------
 * FUNCTION: Set TFTP retry times
 * INPUT   : retry_times    - TFTP retry times
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------
 */
void XFER_OM_SetTftpRetryTimes(UI32_T retry_times)
{
    tftp_cfg.retry_times = retry_times;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_OM_GetTftpTimeout
 *------------------------------------------------------------------------
 * FUNCTION: Retrieves TFTP timeout value in seconds before retry
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : timeout value in seconds
 * NOTE    : None
 *------------------------------------------------------------------------
 */
UI32_T XFER_OM_GetTftpTimeout()
{
    return tftp_cfg.timeout;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_OM_SetTftpRetryTimes
 *------------------------------------------------------------------------
 * FUNCTION: Set TFTP timeout value in seconds before retry
 * INPUT   : timeout    - timeout value in seconds
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
void XFER_OM_SetTftpTimeout(UI32_T timeout)
{
    tftp_cfg.timeout = timeout;
}

#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_OM_SetAutoOpCodeUpgradeStatus
 *------------------------------------------------------------------------
 * FUNCTION: Set auto image upgrade status
 * INPUT   : UI32_T status
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------
 */
void XFER_OM_SetAutoOpCodeUpgradeStatus(UI32_T status)
{
    auto_upg_cfg.opcode_status = status;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_OM_GetAutoOpCodeUpgradeStatus
 *------------------------------------------------------------------------
 * FUNCTION: Get auto image upgrade status
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : status
 * NOTE    : None
 *------------------------------------------------------------------------
 */
UI32_T XFER_OM_GetAutoOpCodeUpgradeStatus()
{
    return auto_upg_cfg.opcode_status;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_OM_SetAutoOpCodeUpgradeReloadStatus
 *------------------------------------------------------------------------
 * FUNCTION: Set auto image upgrade reload status
 * INPUT   : status
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------
 */
void XFER_OM_SetAutoOpCodeUpgradeReloadStatus(UI32_T status)
{
    auto_upg_cfg.opcode_reload_status = status;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_OM_GetAutoOpCodeUpgradeReloadStatus
 *------------------------------------------------------------------------
 * FUNCTION: Get auto image upgrade reload status
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : status
 * NOTE    : None
 *------------------------------------------------------------------------
 */
UI32_T XFER_OM_GetAutoOpCodeUpgradeReloadStatus()
{
    return auto_upg_cfg.opcode_reload_status;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_OM_SetAutoOpCodeUpgradePath
 *------------------------------------------------------------------------
 * FUNCTION: Set auto image upgrade search path
 * INPUT   : char *path_p
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T XFER_OM_SetAutoOpCodeUpgradePath(const char *path_p)
{
    if (NULL == path_p)
    {
        return FALSE;
    }

    strncpy(auto_upg_cfg.opcode_path, path_p, MAXSIZE_fileAutoUpgradeOpCodePath);
    auto_upg_cfg.opcode_path[MAXSIZE_fileAutoUpgradeOpCodePath] = '\0';

    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_OM_GetAutoOpCodeUpgradePath
 *------------------------------------------------------------------------
 * FUNCTION: Get auto image upgrade search path
 * INPUT   : None
 * OUTPUT  : char *path_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T XFER_OM_GetAutoOpCodeUpgradePath(char *path_p)
{
    if (NULL == path_p)
    {
        return FALSE;
    }

    strncpy(path_p, auto_upg_cfg.opcode_path, MAXSIZE_fileAutoUpgradeOpCodePath);
    path_p[MAXSIZE_fileAutoUpgradeOpCodePath] = '\0';

    return TRUE;
}
#endif /* #if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE) */

/* LOCAL SUBPROGRAM BODIES
 */
