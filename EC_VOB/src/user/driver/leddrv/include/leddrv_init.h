/* Module Name: LEDDRV_INIT.H
 * Purpose: 
 *  This module is in first task of LED Driver, bring up whole led driver.
 *
 * Notes: 
 *
 * History:                                                               
 *          Date      -- Modifier,        Reason
 * ------------------------------------------------------------------------
 *  V1.0  2001.10.22  -- Jason Hsue,      Creation
 *          2007.08.01  -- Echo Chen        Modified for linux platform
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001
 */
#ifndef LEDDRV_INIT_H
#define LEDDRV_INIT_H


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
void LEDDRV_INIT_InitiateSystemResources(void);

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
void LEDDRV_INIT_AttachSystemResources(void);

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
void LEDDRV_INIT_Create_InterCSC_Relation(void);

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
void LEDDRV_INIT_EnterTransitionMode(void);

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
void LEDDRV_INIT_EnterMasterMode(void);

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
void LEDDRV_INIT_EnterSlaveMode(void);

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
void LEDDRV_INIT_SetTransitionMode(void);

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
void LEDDRV_INIT_Create_Tasks(void);

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
void LEDDRV_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);
#endif

