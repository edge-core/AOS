/*-----------------------------------------------------------------------------
 * Module Name: systime_init.h   											 		   
 *-----------------------------------------------------------------------------
 * PURPOSE: A header file for the definition of Data Structure of sysdrv_init.c                               	   
 *-----------------------------------------------------------------------------
 * NOTES: 
 *-----------------------------------------------------------------------------
 * HISTORY:																	   
 *    07/16/2007 - Echo Chen , Created for Linux Platform
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007 		   
 *-----------------------------------------------------------------------------
 */

#ifndef	SYSTIME_INIT_H
#define	SYSTIME_INIT_H

/* INCLUDE FILE DECLARATIONS
 */
 #include "sys_type.h"
 #include "sysfun.h"

/* FUNCTION NAME: SYSTIME_INIT_Initiate_System_Resources
 *----------------------------------------------------------------------------------
 * PURPOSE: Initialization of SYSDRV
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SYS_TIME_INIT_InitiateSystemResources(void);

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
void SYS_TIME_INIT_Create_InterCSC_Relation(void);

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
void SYS_TIME_INIT_SetTransitionMode(void);


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
void SYS_TIME_INIT_EnterTransitionMode(void);


/* FUNCTION NAME: SYSTIME_INIT_EnterMasterMode
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will force SYSDRV to enter master mode	
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SYS_TIME_INIT_EnterMasterMode(void);


/* FUNCTION NAME: SYSTIME_INIT_EnterSlaveMode
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will force SYSDRV to enter slave mode	
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SYS_TIME_INIT_EnterSlaveMode(void);


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
void SYS_TIME_INIT_Create_Tasks(void);

void SYS_TIME_INIT_ProvisionComplete(UI32_T unit);

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
void SYS_TIME_INIT_AttachSystemResources(void);

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
void SYS_TIME_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

#endif /* SYSTIME_INIT_H */

