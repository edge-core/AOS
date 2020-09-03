/* Module Name: SMTP_TASK.H
 * Purpose: This file contains the information of smtp module:
 *          1. Initialize resource.
 *          2. Create task.
 *
 * Notes:   None.
 * History:
 *    01/22/03       -- Ricky Lin, Create
 *
 * Copyright(C)      Accton Corporation, 1999 - 2001
 *
 */


#ifndef SMTP_TASK_H
#define SMTP_TASK_H


/* INCLUDE FILE DECLARATIONS
 */

#define SMTP_TASK_EVENT_PERIODIC_TIMER 0x0001L
#define SMTP_TASK_EVENT_ENTER_TRANSITION  0x0002L
#define SMTP_TASK_EVENT_SEND_MAIL  0x0004L

/* NAME CONSTANT DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */


/* FUNCTION NAME: SMTP_TASK_Initiate_System_Resources
 * PURPOSE: This function is used to initialize the smtp task.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void SMTP_TASK_Initiate_System_Resources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SMTP_TASK_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SMTP_TASK_Create_InterCSC_Relation(void);

/* FUNCTION NAME: SMTP_TASK_Create_Task
 * PURPOSE: This function will create and start SMTP task.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void SMTP_TASK_Create_Task(void);


/* FUNCTION NAME: SMTP_TASK_EnterTransitionMode
 * PURPOSE: The function will enter transition mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   1. The function MUST be called before calling the
 *             SMTP_TASK_EnterMasterMode function.
 */
void SMTP_TASK_EnterTransitionMode(void);

/* FUNCTION NAME: SMTP_TASK_SetTransitionMode
 * PURPOSE: The function will set transition mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void SMTP_TASK_SetTransitionMode(void);

#endif /* SMTP_TASK_H */
