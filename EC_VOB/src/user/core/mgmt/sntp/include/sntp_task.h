/* static char SccsId[] = "+-<>?!SNTP_TASK.H   22.1  24/04/02  14:00:00";  
 * ------------------------------------------------------------------------
 *  FILE NAME  -  _SNTP_TASK.H				           					            
 * ------------------------------------------------------------------------
 *  ABSTRACT:   
 *		This module handles all events and signal handling functions, one callback
 *		handling packet received, and timeout of protocol.
 *
 *  Notes:
 *		None.
 *  Modification History:
 *  Modifier           Date        Description
 *  -----------------------------------------------------------------------
 *  S.K.Yang		  04-24-2002  Created										   
 * ------------------------------------------------------------------------
 *  Copyright(C)				Accton Corporation, 1999				   
 * ------------------------------------------------------------------------
 */

#ifndef _SNTP_TASK_H
#define _SNTP_TASK_H


/* INCLUDE FILE	DECLARATIONS
 */


/* NAME	CONSTANT DECLARATIONS
 */



/* MACRO FUNCTION DECLARATIONS
 */


/* DATA	TYPE DECLARATIONS
 */


/* EXPORTED	SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_TASK_Init                                
 *------------------------------------------------------------------------------
 * PURPOSE  : SInit system resource required by SNTP_TASK; including message queue,
 *			  memory.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none                                                              
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
void	SNTP_TASK_Init (void);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_TASK_CreateTask                                
 *------------------------------------------------------------------------------
 * PURPOSE  : Create all tasks in SNTP, client, server, and relay.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none                                                              
 * NOTES    : 1. All tasks created, then call MGR, perform proper function.                                                             
 *------------------------------------------------------------------------------*/ 
void	SNTP_TASK_CreateTask (void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_TASK_ProvisionComplete                                
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will tell the SSHD module to start.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none                                                              
 * NOTES    : This function is invoked in SNTP_INIT_ProvisionComplete().
 *------------------------------------------------------------------------------*/ 
void SNTP_TASK_ProvisionComplete(void);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_TASK_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE: This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the COS subsystem will enter the
 *          Master Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: 1. All the other subsystems must enter the master mode before this function
 *          can be invoked.
 *       2. If "System Boot Configruation File" does not exist, the database and
 *          switch will be initiated to the factory default value.
 *-------------------------------------------------------------------------
 */
void SNTP_TASK_EnterMasterMode(void);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_TASK_EnterTransitionMode
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Transition Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: 1. Reallocate all resource for this subsystem
 *       2. Will be called when operation mode change between Master and Slave mode
 *-------------------------------------------------------------------------
 */
void SNTP_TASK_EnterTransitionMode(void);
/*-------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_TASK_Enter_Slave_Mode
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Slave Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: In Slave Operation mode, any requests issued from applications will be ignored.
 *-------------------------------------------------------------------------
 */
void SNTP_TASK_EnterSlaveMode(void);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_TASK_SetTransitionMode                             
 *------------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by stkctrl.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none                                                              
 * NOTES    :                                                              
 *------------------------------------------------------------------------------*/ 
void SNTP_TASK_SetTransitionMode(void);

#endif	 /*	_SNTP_TASK_H */
