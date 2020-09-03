/*-----------------------------------------------------------------------------
 * Module Name: SYSDRV_INIT.C   											 		   
 *-----------------------------------------------------------------------------
 * PURPOSE: This module is used to initialize the component of SYSDRV.                               	   
 *-----------------------------------------------------------------------------
 * NOTES: 
 *-----------------------------------------------------------------------------
 * HISTORY:																	   
 *    11/21/2002 - Benson Hsu, Created	
 *    07/16/2007 - Echo Chen , Modified for Linux Platform
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002 		   
 *-----------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sysdrv.h"
#include "sysrsc_mgr.h"
 


/* FUNCTION NAME: SYSDRV_INIT_Initiate_System_Resources
 *----------------------------------------------------------------------------------
 * PURPOSE: Initialization of SYSDRV
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SYSDRV_INIT_InitiateSystemResources(void)
{
    SYSDRV_InitiateSystemResources();
}

/* FUNCTION NAME: SYSDRV_INIT_Create_InterCSC_Relation
 *----------------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
void SYSDRV_INIT_Create_InterCSC_Relation(void)
{

}

/* FUNCTION NAME: SYSDRV_INIT_SetTransitionMode
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will set SYSDRV to transition mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SYSDRV_INIT_SetTransitionMode(void)
{
    SYSDRV_SetTransitionMode();
}


/* FUNCTION NAME: SYSDRV_INIT_EnterTransitionMode
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will force SYSDRV to enter transition mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SYSDRV_INIT_EnterTransitionMode(void)
{
    SYSDRV_EnterTransitionMode();
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
void SYSDRV_INIT_EnterMasterMode(void)
{
    SYSDRV_EnterMasterMode();
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
void SYSDRV_INIT_EnterSlaveMode(void)
{
    SYSDRV_EnterSlaveMode();   
}


/* FUNCTION NAME: SYSDRV_INIT_Create_Tasks
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will create a task for SYSDRV
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   There is no task on SYSDRV.
 */
void SYSDRV_INIT_Create_Tasks(void)
{
    // SYSDRV_TASK_CreateTask();
    return;
}
void SYSDRV_INIT_ProvisionComplete(void)
{
    SYSDRV_ProvisionComplete();   
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYSDRV_INIT_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for SYSDRV in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void SYSDRV_INIT_AttachSystemResources(void)
{
    SYSDRV_AttachSystemResources();
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYSDRV_INIT_GetShMemInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Provide shared memory information for SYSRSC.
 * INPUT:   None
 * OUTPUT:  segid_p  --  shared memory segment id
 *          seglen_p --  length of the shared memroy segment
 * RETUEN:  None
 * NOTES:
 *    This function is called in SYSRSC_MGR_CreateSharedMemory().
 *---------------------------------------------------------------------------------*/
void SYSDRV_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_SYSDRV_SHMEM_SEGID;
    *seglen_p = sizeof(SYSDRV_Shmem_Data_T);
}

