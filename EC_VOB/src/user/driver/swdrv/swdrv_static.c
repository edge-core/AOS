/* ------------------------------------------------------------------------
 * FILE NAME - swdrv_static.c
 * ------------------------------------------------------------------------
 * Purpose:
 * Note:
 *   Not support stacking now.
 * ------------------------------------------------------------------------
 *  History
 *
 *   Wakka             30/10/2015      new created
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2015
 * ------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "dev_swdrv.h"
#include "dev_swdrv_pmgr.h"
#include "swdrv_om.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* TYPE DECLARATIONS
 */


/* MACRO DEFINITIONS
 */


/* LOCAL FUNCTIONS DECLARATIONS
 */


/* LOCAL VARIABLES DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetPortStatusForLicense
 * -------------------------------------------------------------------------
 * FUNCTION: To set set port administration status
 * INPUT   : unit
 *           port
 *           status  - TRUE/FALSE
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_SetPortStatusForLicense(UI32_T unit, UI32_T port, BOOL_T status)
{
    if (SWDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
         /* UIMSG_MGR_SetErrorCode() */
        return FALSE;
    }

    if(!DEV_SWDRV_PMGR_SetPortStatusForLicense(unit, port, status))
    {
        return FALSE;
    }

    return TRUE;
}

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

