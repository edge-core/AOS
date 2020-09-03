/*-----------------------------------------------------------------------------
 * Module Name: SYSDRV_INIT.H   											 		   
 *-----------------------------------------------------------------------------
 * PURPOSE: A header file for the definition of Data Structure of sysdrv_init.c                               	   
 *-----------------------------------------------------------------------------
 * NOTES: 
 *-----------------------------------------------------------------------------
 * HISTORY:																	   
 *    11/21/2002 - Benson Hsu, Created	
 *    07/16/2007 - Echo Chen , Modified for Linux Platform
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007 		   
 *-----------------------------------------------------------------------------
 */

#ifndef	SYSDRV_INIT_H
#define	SYSDRV_INIT_H

/* INCLUDE FILE DECLARATIONS
 */
 #include "sys_type.h"
 #include "sysfun.h"
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
void SYSDRV_INIT_InitiateSystemResources(void);

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
void SYSDRV_INIT_Create_InterCSC_Relation(void);

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
void SYSDRV_INIT_SetTransitionMode(void);


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
void SYSDRV_INIT_EnterTransitionMode(void);


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
void SYSDRV_INIT_EnterMasterMode(void);


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
void SYSDRV_INIT_EnterSlaveMode(void);


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
void SYSDRV_INIT_Create_Tasks(void);

void SYSDRV_INIT_ProvisionComplete(void);

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
void SYSDRV_INIT_AttachSystemResources(void);

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
void SYSDRV_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);




#endif /* SYSDRV_INIT_H */
