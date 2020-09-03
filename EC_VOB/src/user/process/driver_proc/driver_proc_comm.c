/*-----------------------------------------------------------------------------
 * MODULE NAME: Driver_PROC_COMM.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    The module provides APIs for CSCs within the Driver process to get common
 *    resources of the Driver process.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/07/13     --- Echo, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <stdio.h>
#include "sys_type.h"
#include "sys_cpnt.h"
#include "l_threadgrp.h"
#include "driver_proc_comm.h"
#include "dev_nicdrv_pmgr.h"
#include "dev_swdrv_pmgr.h"
#include "swctrl_pom.h"
#include "dev_rm_pmgr.h"
#include "stktplg_pom.h"
#include "dev_nmtrdrv_pmgr.h"
#include "dev_amtrdrv_pmgr.h"
#include "dev_nicdrv_pmgr.h"
#include "stktplg_pom.h"
#include "uc_mgr.h"
#include "xstp_pom.h"
#include "amtr_pmgr.h"
#include "dev_swdrvl3_pmgr.h"
#include "amtr_pmgr.h"
#include "amtr_pom.h"

/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DEFINITIONS
 */

/* the handle of thread group
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : DRIVER_COMM_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initialize the system resource which is common for all CSCs in
 *           driver process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T DRIVER_PROC_COMM_InitiateProcessResource(void)
{
    if(FALSE==UC_MGR_InitiateProcessResources())
    {
        printf("\r\n%s: UC_MGR_InitiateProcessResources fails", __FUNCTION__);
        return FALSE;
    }
    /* PMGR init here */
    DEV_SWDRV_PMGR_InitiateProcessResource();
    DEV_AMTRDRV_PMGR_InitiateProcessResource();
    DEV_NMTRDRV_PMGR_InitiateProcessResource();
    DEV_NICDRV_PMGR_InitiateProcessResource();
    DEV_SWDRVL3_PMGR_InitiateProcessResource();
    DEVRM_PMGR_InitiateProcessResource();

    STKTPLG_POM_InitiateProcessResources();
    XSTP_POM_InitiateProcessResource();
    SWCTRL_POM_Init();
#if (SYS_CPNT_AMTR == TRUE)
    if(AMTR_PMGR_InitiateProcessResource()==FALSE)
        return FALSE;

    if(AMTR_POM_InitiateProcessResource()==FALSE)
        return FALSE;
#endif
    //SWCTRL_PMGR_Init();
    return TRUE;
}

/* LOCAL SUBPROGRAM BODIES
 */
