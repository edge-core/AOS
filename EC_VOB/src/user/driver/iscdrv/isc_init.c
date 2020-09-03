/* Module Name: ISC_INIT.C
 * Purpose: 
 *        ( 1. Whole module function and scope.                           )
 *        ( 2.  The domain MUST be handled by this module.      )
 *        ( 3.  The domain would not be handled by this module. )
 * Notes: 
 *        ( Something must be known or noticed by developer     )
 * History:                                                               
 *       Date		-- 	Modifier,Reason
 *	0.0	06/17/02	-- 	whsu	,Create
 *  0.1 08/09/02    --  whsu    ,Modify
 *
 * Copyright(C)      Accton Corporation, 1999, 2000   				
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "isc_init.h"
#include "isc.h"
#include "iuc_init.h"
#include "iuc.h"
#include "isc_om.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS 
 */

/* STATIC VARIABLE DECLARATIONS 
 */

/* EXPORTED SUBPROGRAM BODIES
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
void ISC_INIT_InitiateSystemResources(void)
{        
   /* IUC
    */
   //IUC_INIT_InitiateSystemResources();
    
   /* ISC
    */
   ISC_InitiateSystemResources();
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: ISC_INIT_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for ISC in the context of the calling process.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void ISC_INIT_AttachSystemResources(void)
{
   /* IUC
    */
   //IUC_INIT_AttachSystemResources();
    
   /* ISC
    */
   ISC_AttachSystemResources();
}

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
void ISC_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    ISC_OM_GetShMemInfo(segid_p,seglen_p); 
}

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
void ISC_INIT_Create_InterCSC_Relation(void)
{
    /* IUC
     */
    //IUC_INIT_Create_InterCSC_Relation();
    
    /* ISC
     */
    ISC_Create_InterCSC_Relation();
}

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
void ISC_INIT_CreateTasks(void)
{
    /* IUC
     */
    ;
    
    /* ISC
     */
    ;
}

/*---------------------------------------------------------------------------------
 * FUNCTION : void ISC_INIT_EnterMasterMode(void)
 *---------------------------------------------------------------------------------
 * PURPOSE  : Make ISC enter master mode
 * INPUT    : tg_handle   --  thread group handle
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void ISC_INIT_EnterMasterMode(void)
{
    /* IUC
     */
    ;
    
    /* ISC
     */
    ISC_EnterMasterMode();
}

/*---------------------------------------------------------------------------------
 * FUNCTION : void ISC_INIT_EnterSlaveMode(void)
 *---------------------------------------------------------------------------------
 * PURPOSE  : Make ISC enter slave mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void ISC_INIT_EnterSlaveMode(void)
{
    /* IUC
     */
    ;
    
    /* ISC
     */
    ISC_EnterSlaveMode();
}

/*---------------------------------------------------------------------------------
 * FUNCTION : void ISC_INIT_EnterTransitionMode(void)
 *---------------------------------------------------------------------------------
 * PURPOSE  : Make ISC enter transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void ISC_INIT_EnterTransitionMode(void)
{
    /* IUC
     */
    ;
    
    /* ISC
     */
    ISC_EnterTransitionMode();
}

/*---------------------------------------------------------------------------------
 * FUNCTION : void ISC_INIT_SetTransitionMode(void)
 *---------------------------------------------------------------------------------
 * PURPOSE  : Set ISC to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void ISC_INIT_SetTransitionMode(void)
{
    /* IUC
     */
    ;
    
    /* ISC
     */
    ISC_SetTransitionMode();
}

/*---------------------------------------------------------------------------------
 * FUNCTION : void ISC_INIT_ProvisionComplete(void)
 *---------------------------------------------------------------------------------
 * PURPOSE  : To notify the CSC that provision complete
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void ISC_INIT_ProvisionComplete(void)
{
    /* IUC
     */
    ;
    
    /* ISC
     */
    ISC_ProvisionComplete();
}
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)

void ISC_INIT_HandleHotInsertion(void)
{
	ISC_HandleHotInsertion();

}
void ISC_INIT_HandleHotRemoval(void)
{
   /* EPR:ES3628BT-FLF-ZZ-00742
    Problem: stack:the master unit hung when remve from the stack.
    Rootcause:when the hot removal happend,and other csc is sending pkt to slaves which have been removed 
              and the csc is waiting for the slave reply.so the csc will never receive the slave reply.and it is still in the suspend state
              And in ISC_HandleHotRemoval,if here get the unit info for ISC again ,sometimes it will error.For a unit add ,and removal
              and the add info is not process quickly.
    Solution:when the slave removed update isc_om database ,and wakeup the csc which is waiting for the slave reply which is removed.
    Files:ISC_OM.C,stktplg_engine.c,stktplg_om.c,stktplg_om.h makefile.am,l_stack.c,l_stack.h,isc_init.c*/

	//ISC_HandleHotRemoval();

}
#endif
