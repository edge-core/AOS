/* MODULE NAME: WEBAUTH_TASK.H 
 * PURPOSE: 
 *      Declarations for WEBAUTH task 
 * NOTES:
 *
 *
 * HISTORY:
 *    01/29/07 --  Rich Lee, Create
 * 
 * Copyright(C)      Accton Corporation, 2007 
 */

#ifndef _WEBAUTH_TASK_H
#define _WEBAUTH_TASK_H


/* INCLUDE FILE DECLARATTIONS
 */
#include <sys_type.h>


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */
 
/* DATA TYPE DECLARATIONS
 */
 

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
 
 
/* FUNCTION NAME: WEBAUTH_TASK_EnterMasterMode                      
 * PURPOSE: would be called from webauth_init.c    
 * INPUT:   None                                                         
 * OUTPUT:  None                                                         
 * RETURN:  None                                                        
 * NOTE:    None
 */
void WEBAUTH_TASK_EnterMasterMode(void);

/* FUNCTION NAME:  WEBAUTH_TASK_EnterTransitionMode
 * PURPOSE: This function forces this subsystem enter the Transition 
 *          Operation mode.
 * INPUT:   none.
 * OUTPUT:  none.
 * RETURN:  none.
 * NOTES:   none.
 */
void WEBAUTH_TASK_EnterTransitionMode(void);

/* FUNCTION NAME:  WEBAUTH_TASK_EnterSlaveMode
 * PURPOSE: This function forces this subsystem enter the Slave Operation mode.
 * INPUT:   none.
 * OUTPUT:  none.
 * RETURN:  none.
 * NOTES:
 */
void WEBAUTH_TASK_EnterSlaveMode(void);

/* FUNCTION NAME : WEBAUTH_TASK_SetTransitionMode
 * PURPOSE: Set transition mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 */
void WEBAUTH_TASK_SetTransitionMode(void);

/* FUNCTION NAME: WEBAUTH_TASK_Initialize_System_Resources                                               
 * PURPOSE: This function will initialize kernel resources               
 * INPUT:   None                                                         
 * OUTPUT:  None                                                         
 * RETURN:  None                                                         
 * NOTE:    Invoked by root.c()                                        
 */
void WEBAUTH_TASK_Initiate_System_Resources (void);

/* FUNCTION NAME:  WEBAUTH_TASK_Create_Task
 * PURPOSE: This function creates the WEBAUTH task for network
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:   none
 */
void WEBAUTH_TASK_Create_Task (void);

/* FUNCTION NAME:  WEBAUTH_TASK_ProvisionComplete
 * PURPOSE: This function will tell webauth that provision is completed
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTE:    None
 */
void WEBAUTH_TASK_ProvisionComplete(void);
#endif  /* End of webauth_TASK_h */

