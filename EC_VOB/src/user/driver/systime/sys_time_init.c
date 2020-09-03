/*-----------------------------------------------------------------------------
 * Module Name: systime_init.c   											 		   
 *-----------------------------------------------------------------------------
 * PURPOSE: This module is used to initialize the component of SYSDRV.                               	   
 *-----------------------------------------------------------------------------
 * NOTES: 
 *-----------------------------------------------------------------------------
 * HISTORY:																	   
 *    07/16/2007 - Echo Chen , Create for Linux Platform
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002 		   
 *-----------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sysdrv.h"
#include "sysrsc_mgr.h"
#include "sys_time.h"

/* FUNCTION NAME: SYSTIME_INIT_InitiateSystemResources
 *----------------------------------------------------------------------------------
 * PURPOSE: Initialization of SYSDRV
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SYS_TIME_INIT_InitiateSystemResources(void)
{
    SYS_TIME_InitiateSystemResources();
}

/* FUNCTION NAME: SYSTIME_INIT_Create_InterCSC_Relation
 *----------------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
void SYS_TIME_INIT_Create_InterCSC_Relation(void)
{
    SYS_TIME_Create_InterCSC_Relation();
}

/* FUNCTION NAME: SYSTIME_INIT_SetTransitionMode
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will set SYSDRV to transition mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SYS_TIME_INIT_SetTransitionMode(void)
{
    SYS_TIME_SetTransitionMode();
}


/* FUNCTION NAME: SYSTIME_INIT_EnterTransitionMode
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will force SYSDRV to enter transition mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SYS_TIME_INIT_EnterTransitionMode(void)
{
    SYS_TIME_EnterTransitionMode();
}


/* FUNCTION NAME: SYSDRV_INIT_EnterMasterMode
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will force SYSDRV to enter master mode	
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SYS_TIME_INIT_EnterMasterMode(void)
{
    SYS_TIME_EnterMasterMode();
}


/* FUNCTION NAME: SYSDRV_INIT_EnterSlaveMode
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will force SYSDRV to enter slave mode	
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SYS_TIME_INIT_EnterSlaveMode(void)
{
    SYS_TIME_EnterSlaveMode();   
}


/* FUNCTION NAME: SYSTIME_INIT_Create_Tasks
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will create a task for SYSTIME
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   There is no task on SYSDRV.
 */
void SYS_TIME_INIT_Create_Tasks(void)
{
    
    return;
}
void SYS_TIME_INIT_ProvisionComplete(UI32_T unit)
{
    SYS_TIME_ProvisionComplete(unit);   
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYSTIME_INIT_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for SYSDRV in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void SYS_TIME_INIT_AttachSystemResources(void)
{
    SYS_TIME_AttachSystemResources();
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYSTIME_INIT_GetShMemInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Provide shared memory information for SYSRSC.
 * INPUT:   None
 * OUTPUT:  segid_p  --  shared memory segment id
 *          seglen_p --  length of the shared memroy segment
 * RETUEN:  None
 * NOTES:
 *    This function is called in SYSRSC_MGR_CreateSharedMemory().
 *---------------------------------------------------------------------------------*/
void SYS_TIME_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_SYS_TIME_SHMEM_SEGID;
    *seglen_p = sizeof(SYS_TIME_Shmem_Data_T);
}

