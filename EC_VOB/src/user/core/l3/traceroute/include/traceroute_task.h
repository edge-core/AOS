/* FILE NAME  -  TraceRoute_Task.h
 *
 *  ABSTRACT :  This packet provides basic TraceRoute functionality.
 *                                                                         
 *  NOTES: This package shall be a reusable component of BNBU L2/L3/L4 switch product lines. 
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
#ifndef     _TRACEROUTE_TASK_H
#define     _TRACEROUTE_TASK_H


/* INCLUDE FILE DECLARATIONS
 */
 
/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME - TRACEROUTE_TASK_Initiate_System_Resources                                  
 * PURPOSE  : This function allocates and initiates the system resource for
 *            TRACEROUTE database.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none                                                              
 * NOTES    : none                                                              
 */
void TRACEROUTE_TASK_Initiate_System_Resources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - TRACEROUTE_TASK_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void TRACEROUTE_TASK_Create_InterCSC_Relation(void);

/* FUNCTION NAME - TRACEROUTE_TASK_EnterMasterMode                                  
 * PURPOSE  : This function will configured TRACEROUTE to enter master mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none                                                              
 * NOTES    : none                                                              
 */
void TRACEROUTE_TASK_EnterMasterMode(void);


/* FUNCTION NAME - TRACEROUTE_TASK_EnterTransitionMode                                  
 * PURPOSE  : This function will configured TRACEROUTE to enter transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none                                                              
 * NOTES    : none                                                              
 */
void TRACEROUTE_TASK_EnterTransitionMode(void);


/* FUNCTION NAME - TRACEROUTE_TASK_EnterSlaveMode                                  
 * PURPOSE  : This function will configured TRACEROUTE to enter master mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none                                                              
 * NOTES    : none                                                              
 */
void TRACEROUTE_TASK_EnterSlaveMode(void);


/* FUNCTION NAME - TRACEROUTE_TASK_SetTransitionMode
 * PURPOSE  : This function sets the component to temporary transition mode           
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void  TRACEROUTE_TASK_SetTransitionMode(void);


/* FUNCTION NAME - TRACEROUTE_TASK_CreateTask                                  
 * PURPOSE  : This function will create TRACEROUTE task
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none                                                              
 * NOTES    : none                                                              
 */
void TRACEROUTE_TASK_CreateTask(void);


/* FUNCTION NAME - TRACEROUTE_TASK_ProvisionComplete                                  
 * PURPOSE  : This function will create socket after provision complete
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none                                                              
 * NOTES    : none                                                              
 */
void TRACEROUTE_TASK_ProvisionComplete(void);

/* FUNCTION NAME - TRACEROUTE_TASK_PeriodicTimerStart_Callback
 * PURPOSE  : This function will send event to tracert task in order to 
 *            start periodic timer
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void TRACEROUTE_TASK_PeriodicTimerStart_Callback(void);

#endif
