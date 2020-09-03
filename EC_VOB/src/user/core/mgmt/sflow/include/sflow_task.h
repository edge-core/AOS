/*-----------------------------------------------------------------------------
 * Module Name: sflow_task.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the sFlow task
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    10/25/2007 - Joeanne Peng, Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#include "sys_type.h"

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_TASK_Initiate_System_Resources
 *------------------------------------------------------------------------------
 * PURPOSE  : Initialize MsgQ and SFLOW_MGR
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : 
 *------------------------------------------------------------------------------*/
void SFLOW_TASK_Initiate_System_Resources (void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_TASK_Create_Tasks
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will create task.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SFLOW_TASK_Create_Tasks(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_TASK_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call sflow task into master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SFLOW_TASK_EnterMasterMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_TASK_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set sflow task into slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SFLOW_TASK_EnterSlaveMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_TASK_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set sflow task into transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SFLOW_TASK_EnterTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_TASK_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void  SFLOW_TASK_SetTransitionMode(void);

