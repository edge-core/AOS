/* =====================================================================================*
 * FILE	NAME: VRRP_task.h                                                               *
 *                                                                                      *
 * PURPOSE: Register callback functions and wait for message send.  			        *
 *
 * NOTES:
 *
 * HISTORY
 *    3/28/2009 - Donny.Li     , Created
 *
 * Copyright(C)      Accton Corporation, 2009    

 * =====================================================================================*/
 
#ifndef VRRP_TASK_H
#define VRRP_TASK_H

#include <sys_bld.h>
//#include "L_mref.h"

/* DATA TYPE DECLARATIONS
 */
 
void VRRP_TASK_Master_Routine(UI32_T task_event);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_TASK_Initiate_System_Resources                                  
 *------------------------------------------------------------------------------
 * PURPOSE  : This function allocates and initiates the system resource for
 *            VRRP database.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure                                                              
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
BOOL_T VRRP_TASK_Initiate_System_Resources(void);

/*------------------------------------------------------------------------------
 * FUNCTION : VRRP_TASK_RegisterCallbackFunction
 * PURPOSE  : Register Callback Function.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETUEN   : None.
 * NOTES    : None.
 *------------------------------------------------------------------------------*/
BOOL_T VRRP_TASK_RegisterCallbackFunction(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_TASK_EnterTransitionMode                                  
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will configured VRRP to enter transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none                                                              
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/
void VRRP_TASK_EnterTransitionMode(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_TASK_SetTransitionMode                                  
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will configured VRRP to set transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none                                                              
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/
void VRRP_TASK_SetTransitionMode(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_TASK_CreateTask                                  
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will create VRRP task.  This will be call by root.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *          : FALSE -- failure
 * NOTES    : none
 *------------------------------------------------------------------------------*/ 
BOOL_T VRRP_TASK_CreateTask(void *arg);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_TASK_ProvisionComplete
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will process necessary procedures when provision completes
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void VRRP_TASK_ProvisionComplete(void);

#endif

