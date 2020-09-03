/* Module Name: FLASHDRV_INIT.H
 * Purpose:
 *  This module is in first task of FLASH Driver, bring up whole flash driver.
 *
 * Notes:
 *
 * History:
 *          Date      -- Modifier,        Reason
 * ------------------------------------------------------------------------
 *  V1.0  2001.10.22  -- Jason Hsue,      Creation
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001
 */
#ifndef FLASHDRV_INIT_H
#define FLASHDRV_INIT_H

/* INCLUDE FILE DECLARATIONS
 */
//#include <fs.h>


/* FUNCTION NAME: FLASHDRV_INIT_InitiateSystemResources
 *----------------------------------------------------------------------------------
 * PURPOSE: init NIC driver and LAN driver
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
void FLASHDRV_INIT_InitiateSystemResources(void);

/* for Diag only */
void FLASHDRV_INIT_Inititate_System_Resources(void);

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

void FLASHDRV_INIT_AttachSystemResources(void);

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

void FLASHDRV_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);


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
void FLASHDRV_INIT_Create_InterCSC_Relation(void);

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
void FLASHDRV_INIT_Create_Tasks(void);

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
void FLASHDRV_INIT_SetTransitionMode(void);


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
void FLASHDRV_INIT_EnterTransitionMode(void);


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
void FLASHDRV_INIT_EnterMasterMode(void);


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
void FLASHDRV_INIT_EnterSlaveMode(void);


/* FUNCTION NAME: FLASHDRV_INIT_Create_Tasks
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will create a task for FLASHDRV
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 */

void FLASHDRV_INIT_ProvisionComplete(void);


#endif

