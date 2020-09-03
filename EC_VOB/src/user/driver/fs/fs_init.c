/* Module Name: FLASHDRV_INIT.C
 * Purpose:
 *  This module is in first task of FLASH Driver, bring up whole flash driver.
 *
 * Notes:
 *
 * History:
 *          Date      -- Modifier,        Reason
 * ------------------------------------------------------------------------
 *  V1.0  2001.10.22  -- Jason Hsue,      Creation
 *        04/06/2004  -- Allen Cheng,
 *                       Create the FS task for the background routine jobs on each
 *                       unit.
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sysrsc_mgr.h"
#include "fs_om.h"
#include "fs.h"

#ifdef INCLUDE_DIAG
#undef SYS_CPNT_STACKING
#endif

//#include "sysdrv.h"

/* FUNCTION NAME: FLASHDRV_INIT_Inititate_System_Resources
 *----------------------------------------------------------------------------------
 * PURPOSE: init NIC driver and LAN driver
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
void FLASHDRV_INIT_InitiateSystemResources(void)
{
#ifndef INCLUDE_DIAG
    FS_InitiateSystemResources();
#endif
}

/* For Diag only */
void FLASHDRV_INIT_Inititate_System_Resources(void)
{
    FS_Init();
#if (SYS_CPNT_STACKING == TRUE)
    FS_TASK_Init();
#endif
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: FLASHDRV_INIT_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for FLASHDRV in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------
 */

void FLASHDRV_INIT_AttachSystemResources(void)
{
#ifndef INCLUDE_DIAG
    FS_AttachSystemResources();
#endif
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: FLASHDRV_INIT_GetShMemInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Provide shared memory information for FLASHDRV.
 * INPUT:   None
 * OUTPUT:  segid_p  --  shared memory segment id
 *          seglen_p --  length of the shared memroy segment
 * RETUEN:  None
 * NOTES:
 *    This function is called in SYSRSC_MGR_CreateSharedMemory().
 *---------------------------------------------------------------------------------
 */

void FLASHDRV_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
#ifndef INCLUDE_DIAG
    FS_INIT_GetShMemInfo(segid_p, seglen_p);
#endif
}


/* FUNCTION NAME: FLASHDRV_INIT_Create_InterCSC_Relation
 *----------------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
void FLASHDRV_INIT_Create_InterCSC_Relation(void)
{
#ifndef INCLUDE_DIAG
    FS_Create_InterCSC_Relation();
#endif
}

/* FUNCTION NAME: FLASHDRV_INIT_Create_Tasks
 *----------------------------------------------------------------------------------
 * PURPOSE: create NIC task
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
void FLASHDRV_INIT_Create_Tasks(void)
{
#if (SYS_CPNT_STACKING == TRUE)
    FS_TASK_CreateTask();
#endif
    return;
}

/* FUNCTION NAME: FLASHDRV_INIT_SetTransitionMode
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will set FLASHDRV to transition mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void FLASHDRV_INIT_SetTransitionMode(void)
{
    FS_SetTransitionMode();
}


/* FUNCTION NAME: FLASHDRV_INIT_EnterTransitionMode
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will force FLASHDRV to enter transition mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void FLASHDRV_INIT_EnterTransitionMode(void)
{
    FS_EnterTransitionMode();
}


/* FUNCTION NAME: FLASHDRV_INIT_EnterMasterMode
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will force FLASHDRV to enter master mode	
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void FLASHDRV_INIT_EnterMasterMode(void)
{
    FS_EnterMasterMode();
}


/* FUNCTION NAME: FLASHDRV_INIT_EnterSlaveMode
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will force FLASHDRV to enter slave mode	
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void FLASHDRV_INIT_EnterSlaveMode(void)
{
    FS_EnterSlaveMode();   
}


/* FUNCTION NAME: FLASHDRV_INIT_Create_Tasks
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will create a task for FLASHDRV
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 */

void FLASHDRV_INIT_ProvisionComplete(void)
{
    FS_ProvisionComplete();   
}




