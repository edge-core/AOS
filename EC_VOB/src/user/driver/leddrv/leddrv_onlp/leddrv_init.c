/* MODULE NAME: leddrv_init.c
 * PURPOSE: This module is in first task of LED Driver, bring up whole led driver.
 *
 * NOTES:
 * REASON:
 * Description:
 * CREATOR:      Chiourung Huang
 * HISTORY
 *    17/5/2016 - Chiourung Huang, Created
 *
 * Copyright(C)      Accton Corporation, 2016
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "leddrv.h"
#include "leddrv_type.h"
 #include "sysrsc_mgr.h"

/* FUNCTION NAME: LEDDRV_INIT_InititateSystemResources
 *----------------------------------------------------------------------------------
 * PURPOSE: init LED driver
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void LEDDRV_INIT_InitiateSystemResources(void)
{
    LEDDRV_InitiateSystemResources();
}

 /* FUNCTION NAME: LEDDRV_INIT_AttachSystemResources
 *----------------------------------------------------------------------------------
 * PURPOSE: init LED driver
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void LEDDRV_INIT_AttachSystemResources(void)
{
    LEDDRV_AttachSystemResources();
}

/* FUNCTION NAME: LEDDRV_INIT_EnterTransitionMode(void)
 *----------------------------------------------------------------------------------
 * PURPOSE: Make LEDDRV enters transition mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void LEDDRV_INIT_EnterTransitionMode(void)
{
    LEDDRV_EnterTransitionMode();
}

/* FUNCTION NAME: LEDDRV_INIT_EnterMasterMode(void)
 *----------------------------------------------------------------------------------
 * PURPOSE: Make LEDDRV enters master mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void LEDDRV_INIT_EnterMasterMode(void)
{
    LEDDRV_EnterMasterMode();
}

/* FUNCTION NAME: LEDDRV_INIT_EnterSlaveMode(void)
 *----------------------------------------------------------------------------------
 * PURPOSE: Make LEDDRV enters slave mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void LEDDRV_INIT_EnterSlaveMode(void)
{
    LEDDRV_EnterSlaveMode();
}

/* FUNCTION NAME: LEDDRV_INIT_SetTransitionMode(void)
 *----------------------------------------------------------------------------------
 * PURPOSE: Tell LEDDRV to enter transition mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void LEDDRV_INIT_SetTransitionMode(void)
{
    LEDDRV_SetTransitionMode();
}

/* FUNCTION NAME: LEDDRV_INIT_Create_Tasks
 *----------------------------------------------------------------------------------
 * PURPOSE: create LEDDRV task, but there is not necessary to have a LED driver task.
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void LEDDRV_INIT_Create_Tasks(void)
{
    return;
}

/* FUNCTION NAME: LEDDRV_INIT_Create_InterCSC_Relation
 *----------------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void LEDDRV_INIT_Create_InterCSC_Relation(void)
{
    LEDDRV_Create_InterCSC_Relation();
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: LEDDRV_INIT_GetShMemInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Provide shared memory information for SYSRSC.
 * INPUT:   None
 * OUTPUT:  segid_p  --  shared memory segment id
 *          seglen_p --  length of the shared memroy segment
 * RETUEN:  None
 * NOTES:
 *    This function is called in SYSRSC_MGR_CreateSharedMemory().
 *---------------------------------------------------------------------------------*/
void LEDDRV_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_LEDDRV_SHMEM_SEGID;
    *seglen_p = sizeof(LEDDRV_Shmem_Data_T);
}
