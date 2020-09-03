/*-----------------------------------------------------------------------------
 * Module Name: cfm_task.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the CFM task
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    15/12/2006 - macauley_cheng, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2006
 *-----------------------------------------------------------------------------
 */

#ifndef _CFM_TASK_H
#define _CFM_TASK_H
#include "sys_type.h"
#if (SYS_CPNT_CFM == TRUE)
#define CFM_TASK_MAX_MSGQ_LEN      1024


/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_CreateTasks
 * ------------------------------------------------------------------------
 * FUNCTION : Create CFM main task.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
 void CFM_TASK_CreateTask() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_Init
 *-------------------------------------------------------------------------
 * FUNCTION: Init the CFM task
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
 void CFM_TASK_Init() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_INIT_Initiate_System_Resources
 *-------------------------------------------------------------------------
 * FUNCTION: Init the CFM System_Resources
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void CFM_TASK_Initiate_System_Resources() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_Create_InterCSC_Relation()
 *-------------------------------------------------------------------------
 * FUNCTION: Register other CSC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void CFM_TASK_Create_InterCSC_Relation() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_IsMasterMode
 *-------------------------------------------------------------------------
 * FUNCTION: Check if the OM initializing is finished or not.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE if CFM op_state is SYS_TYPE_STACKING_MASTER_MODE, else FALSE
 * Note    : None
 *-------------------------------------------------------------------------
 */
BOOL_T CFM_TASK_IsMasterMode() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter Master mode calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void CFM_TASK_EnterMasterMode() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter Slave mode calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void CFM_TASK_EnterSlaveMode() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void CFM_TASK_SetTransitionMode() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void CFM_TASK_EnterTransitionMode() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_ProvisionComplete
 *-------------------------------------------------------------------------
 * PURPOSE  : This function calling by stkctrl will tell CFM that provision is completed.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void CFM_TASK_ProvisionComplete() ;

#endif /*#if (SYS_CPNT_CFM == TRUE)*/
#endif /* _CFM_TASK_H */
