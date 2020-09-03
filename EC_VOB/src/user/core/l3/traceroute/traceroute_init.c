/* FILE NAME  -  TraceRoute_mgr.c
 *
 *  ABSTRACT :  This packet provides basic TraceRoute functionality.
 *                                                                         
 *  NOTES: This package shall be a reusable component of BNBU L2/L3/L4 switch 
 *         product lines. 
 *
 *
 * Modification History:                                        
 *   By            Date      Ver.    Modification Description                
 * ------------ ----------   -----   --------------------------------------- 
 *   Amytu       2003-07-01          Modify
 * ------------------------------------------------------------------------
 * Copyright(C)                   ACCTON Technology Corp. 2003      
 * ------------------------------------------------------------------------ 
 */
 /* INCLUDE FILE DECLARATIONS
 */
#include "traceroute_init.h"
#include "traceroute_mgr.h"
#include "traceroute_task.h"
 

/* FUNCTION NAME - TRACEROUTE_INIT_Initiate_System_Resources
 * PURPOSE  : This function allocates and initiates the system resource for
 *            TRACEROUTE module.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *
 */
void TRACEROUTE_INIT_Initiate_System_Resources(void)
{
    TRACEROUTE_MGR_Initiate_System_Resources();
    TRACEROUTE_TASK_Initiate_System_Resources();
    return;
} /* end of TRACEROUTE_INIT_Initiate_System_Resources() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - TRACEROUTE_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void TRACEROUTE_INIT_Create_InterCSC_Relation(void)
{
    TRACEROUTE_MGR_Create_InterCSC_Relation();
    TRACEROUTE_TASK_Create_InterCSC_Relation();
} /* end of TRACEROUTE_INIT_Create_InterCSC_Relation */

/* FUNCTION NAME - TRACEROUTE_INIT_Create_Tasks
 * PURPOSE  : This function will create task. 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void TRACEROUTE_INIT_Create_Tasks(void)
{
    TRACEROUTE_TASK_CreateTask();
    
} /* end of TRACEROUTE_INIT_Create_Tasks() */



/* FUNCTION NAME - TRACEROUTE_INIT_EnterMasterMode
 * PURPOSE  : This function will call set TRACEROUTE into master mode.  
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void TRACEROUTE_INIT_EnterMasterMode(void)
{
    TRACEROUTE_MGR_EnterMasterMode();
    TRACEROUTE_TASK_EnterMasterMode();
    return;
} /* end of TRACEROUTE_INIT_EnterMasterMode() */


/* FUNCTION NAME - TRACEROUTE_INIT_EnterSlaveMode
 * PURPOSE  : This function will call set TRACEROUTE into slave mode. 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void TRACEROUTE_INIT_EnterSlaveMode(void)
{
    TRACEROUTE_MGR_EnterSlaveMode();
    TRACEROUTE_TASK_EnterMasterMode();
    return;
} /* end of TRACEROUTE_INIT_EnterSlaveMode() */


/* FUNCTION NAME - TRACEROUTE_INIT_EnterTransitionMode
 * PURPOSE  : This function will call set TRACEROUTE into transition mode. 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void TRACEROUTE_INIT_EnterTransitionMode(void)
{
    TRACEROUTE_TASK_EnterTransitionMode();
    TRACEROUTE_MGR_EnterTransitionMode();
    
    return;
} /* end of TRACEROUTE_INIT_EnterTransitionMode() */


/* FUNCTION NAME - TRACEROUTE_INIT_SetTransitionMode
 * PURPOSE  : This function sets the component to temporary transition mode           
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void  TRACEROUTE_INIT_SetTransitionMode(void)
{
    TRACEROUTE_MGR_SetTransitionMode();
    TRACEROUTE_TASK_SetTransitionMode();
    return;
} /* end of TRACEROUTE_INIT_SetTransitionMode() */

/* FUNCTION NAME - TRACEROUTE_INIT_ProvisionComplete
 * PURPOSE  : This function sets the component to temporary transition mode           
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void  TRACEROUTE_INIT_ProvisionComplete(void)
{
    TRACEROUTE_TASK_ProvisionComplete();
} /* end of TRACEROUTE_INIT_ProvisionComplete() */



