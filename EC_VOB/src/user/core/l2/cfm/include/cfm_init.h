/*-----------------------------------------------------------------------------
 * Module Name: cfm_init.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate the system resources and create the task.
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *      15/12/2006 -- Mmacauley Cheng, created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2006
 *-----------------------------------------------------------------------------
 */
#ifndef _CFM_INIT_H
#define _CFM_INIT_H

#include "sys_type.h"
#if (SYS_CPNT_CFM == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_INIT_Initiate_System_Resources
 * ------------------------------------------------------------------------
 * FUNCTION : Initiate resources.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CFM_INIT_Initiate_System_Resources() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_INIT_Create_InterCSC_Relation
 *-------------------------------------------------------------------------
 * FUNCTION: Register other CSC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void CFM_INIT_Create_InterCSC_Relation() ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_INIT_CreateTask
 * ------------------------------------------------------------------------
 * FUNCTION : Create CFM tasks.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CFM_INIT_CreateTasks() ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_INIT_SetTransitionMode
 * ------------------------------------------------------------------------
 * FUNCTION : Set transition mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CFM_INIT_SetTransitionMode() ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_INIT_EnterTransitionMode
 * ------------------------------------------------------------------------
 * FUNCTION : Enter transition mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CFM_INIT_EnterTransitionMode() ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_INIT_EnterSlaveMode
 * ------------------------------------------------------------------------
 * FUNCTION : Enter slave mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CFM_INIT_EnterSlaveMode() ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_INIT_EnterMasterMode
 * ------------------------------------------------------------------------
 * FUNCTION : Enter master mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CFM_INIT_EnterMasterMode() ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_INIT_ProvisionComplete
 * ------------------------------------------------------------------------
 * FUNCTION : provision complete.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CFM_INIT_ProvisionComplete() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_INIT_HandleHotInsert
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the module hot insert.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void CFM_INIT_HandleHotInsertion(UI32_T beg_ifindex, UI32_T end_of_index, BOOL_T use_default) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_INIT_HandleHotRemoval
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the module hot insert.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void CFM_INIT_HandleHotRemoval(UI32_T beg_ifindex, UI32_T number_of_port) ;
#endif /*#if (SYS_CPNT_CFM == TRUE)*/

#endif /* End of _CFM_INIT_H */

