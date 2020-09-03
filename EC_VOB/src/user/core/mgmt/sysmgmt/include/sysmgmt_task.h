/* ------------------------------------------------------------------------
 *  FILE NAME  -  SYSMGMT_TASK.H				           						
 * ------------------------------------------------------------------------
 * PURPOSE:  SYSMGMT TASK
 *
 * Notes: API List
 *
 * HISTORY:
 *      Date        --  Modifier,    Reason
 *      -----------------------------------------------------------------------
 *      12-24-2007  --  Andy_Chang,  Create
 *
 * 
 * ------------------------------------------------------------------------
 * Copyright(C)							  	ACCTON Technology Corp. , 2007      
 * ------------------------------------------------------------------------
 */
#ifndef SYSMGMT_TASK_H
#define SYSMGMT_TASK_H


/* INCLUDE FILE DECLARATIONS
 */

/*------------------------------------------------------------------------
 * FUNCTION NAME - SYSMGMT_TASK_Init                                              
 *------------------------------------------------------------------------
 * PURPOSE : This function will initialize kernel resources               
 * INPUT   : None.                                                         
 * OUTPUT  : None.                                                         
 * RETURN  : None.                                                         
 * NOTES   : None.                                        
 *------------------------------------------------------------------------*/
void SYSMGMT_TASK_Init(void);  


/*------------------------------------------------------------------------
 * FUNCTION NAME - SYSMGMT_TASK_CreateTask                                        
 *------------------------------------------------------------------------
 * PURPOSE : This function will create SYSMGMT task            
 * INPUT   : None.                                                         
 * OUTPUT  : None.                                                         
 * RETURN  : TRUE/FALSE                                                         
 * NOTES   : None.
 *------------------------------------------------------------------------*/
BOOL_T SYSMGMT_TASK_CreateTask(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: SYSMGMT_TASK_EnterMasterMode
 * -------------------------------------------------------------------------
 * PURPOSE : This function will enter SYSMGMT Task master mode.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 * -------------------------------------------------------------------------*/
void SYSMGMT_TASK_EnterMasterMode(void);

/*------------------------------------------------------------------------
 * FUNCTION NAME - SYSMGMT_TASK_EnterTransitionMode
 *------------------------------------------------------------------------
 * PURPOSE : The function enter transition mode 
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 *------------------------------------------------------------------------*/
void SYSMGMT_TASK_EnterTransitionMode(void);

/*------------------------------------------------------------------------
 * FUNCTION NAME - SYSMGMT_TASK_EnterSlaveMode
 *------------------------------------------------------------------------
 * PURPOSE : This function will enter slave mode
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 *------------------------------------------------------------------------*/
void SYSMGMT_TASK_EnterSlaveMode(void);

/*------------------------------------------------------------------------
 * FUNCTION NAME - SYSMGMT_TASK_SetTransitionMode
 *------------------------------------------------------------------------
 * PURPOSE : This function will set transition state flag
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 *------------------------------------------------------------------------*/
void SYSMGMT_TASK_SetTransitionMode(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYSMGMT_TASK_ProvisionComplete
 * ---------------------------------------------------------------------
 * PURPOSE: The function notify us provision is completed
 * INPUT: None 
 * OUTPUT: 
 * RETURN: none
 * NOTES: None
 * ---------------------------------------------------------------------
 */
void SYSMGMT_TASK_ProvisionComplete(void);

#endif /* END OF SYSMGMT_TASK_H */
