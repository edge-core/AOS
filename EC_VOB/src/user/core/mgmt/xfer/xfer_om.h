/* Project Name: ES3526V-60
 * Module Name : XFER_OM.h
 * Abstract    :
 * Purpose     :
 *
 * History :
 *          Date        Modifier        Reason
 *          2003/07/24  Erica Li        Create this file
 *          2002/08/15  Erica Li        Add XFER_OM_GetTftpRetryTimes()
 *                                      Add XFER_OM_SetTftpRetryTimes()
 *
 * Copyright(C)      Accton Corporation, 1999, 2000
 *
 * Note    :
 */

#ifndef XFER_OM_H
#define XFER_OM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "xfer_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
void XFER_OM_Init(void);

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
void XFER_OM_EnterMasterMode(void);

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
UI32_T XFER_OM_GetTftpRetryTimes();

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
void XFER_OM_SetTftpRetryTimes(UI32_T retry_times);

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
UI32_T XFER_OM_GetTftpTimeout();

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
void XFER_OM_SetTftpTimeout(UI32_T timeout);

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
void XFER_OM_SetAutoOpCodeUpgradeStatus(UI32_T status);

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
UI32_T XFER_OM_GetAutoOpCodeUpgradeStatus();

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
void XFER_OM_SetAutoOpCodeUpgradeReloadStatus(UI32_T status);

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
UI32_T XFER_OM_GetAutoOpCodeUpgradeReloadStatus();

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
BOOL_T XFER_OM_SetAutoOpCodeUpgradePath(const char *path_p);

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
BOOL_T XFER_OM_GetAutoOpCodeUpgradePath(char *path_p);
#endif /* #if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE) */

#endif /* end XFER_OM_H */


