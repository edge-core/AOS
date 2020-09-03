/* Module Name: ISC_INIT.H
 * Purpose: This head file provides the interface for ISC's service initialization.
 * Notes: 
 * History:                                                               
 *    0.0	06/17/02       -- whsu, Create
 *    0.1   08/02/02       -- whsu, Modify   
 *    
 * Copyright(C)      Accton Corporation, 2002   				
 */


#ifndef ISC_INIT_H
#define ISC_INIT_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysrsc_mgr.h"

/* EXPORTED FUNCTION SPECIFICATIONS
 */
  
/*---------------------------------------------------------------------------------
 * FUNCTION : void ISC_INIT_InitiateSystemResources(void)
 *---------------------------------------------------------------------------------
 * PURPOSE  : Initiate system resource for ISC
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void ISC_INIT_InitiateSystemResources(void);


/*---------------------------------------------------------------------------------
 * FUNCTION NAME: ISC_INIT_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for ISC in the context of the calling process.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void ISC_INIT_AttachSystemResources(void);


/*---------------------------------------------------------------------------------
 * FUNCTION NAME: ISC_INIT_GetShMemInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Get shared memory segment id and its length of ISC
 * INPUT:   None
 * OUTPUT:  segid_p  --  shared memory segment id
 *          seglen_p --  length of the shared memroy segment
 * RETURN:  None
 * NOTES:
 *    This function is called in SYSRSC
 *---------------------------------------------------------------------------------*/
void ISC_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);


/*---------------------------------------------------------------------------------
 * FUNCTION : void ISC_INIT_Create_InterCSC_Relation(void)
 *---------------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration
 *            operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void ISC_INIT_Create_InterCSC_Relation(void);


/*---------------------------------------------------------------------------------
 * FUNCTION : void ISC_INIT_CreateTasks(void)
 *---------------------------------------------------------------------------------
 * PURPOSE  : Call ISC_Create_Tasks to create ISC task to handle any coming packet
 *            and timeout event
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void ISC_INIT_CreateTasks(void);


/*---------------------------------------------------------------------------------
 * FUNCTION : void ISC_INIT_EnterMasterMode(void)
 *---------------------------------------------------------------------------------
 * PURPOSE  : Make ISC enter master mode
 * INPUT    : tg_handle   --  thread group handle
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void ISC_INIT_EnterMasterMode(void);


/*---------------------------------------------------------------------------------
 * FUNCTION : void ISC_INIT_EnterSlaveMode(void)
 *---------------------------------------------------------------------------------
 * PURPOSE  : Make ISC enter slave mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void ISC_INIT_EnterSlaveMode(void);


/*---------------------------------------------------------------------------------
 * FUNCTION : void ISC_INIT_EnterTransitionMode(void)
 *---------------------------------------------------------------------------------
 * PURPOSE  : Make ISC enter transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void ISC_INIT_EnterTransitionMode(void);


/*---------------------------------------------------------------------------------
 * FUNCTION : void ISC_INIT_SetTransitionMode(void)
 *---------------------------------------------------------------------------------
 * PURPOSE  : Set ISC to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void ISC_INIT_SetTransitionMode(void);


/*---------------------------------------------------------------------------------
 * FUNCTION : void ISC_INIT_ProvisionComplete(void)
 *---------------------------------------------------------------------------------
 * PURPOSE  : To notify the CSC that provision complete
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void ISC_INIT_ProvisionComplete(void);


/*---------------------------------------------------------------------------------
 * FUNCTION : void ISC_INIT_HandleHotInsertion(void)
 *---------------------------------------------------------------------------------
 * PURPOSE  : To notify the CSC that HOTinsertion
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/

void ISC_INIT_HandleHotInsertion(void);


void ISC_INIT_HandleHotRemoval(void);
#endif /* end of #ifndef ISC_INIT_H */
