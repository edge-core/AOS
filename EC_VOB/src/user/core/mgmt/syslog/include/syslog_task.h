/* Module Name: SYSLOG_TASK.H
 * Purpose: This file contains the information of system log module:
 *          1. Initialize resource.
 *          2. Create task.
 *
 * Notes:   None.
 * History:
 *    11/14/01       -- Aaron Chuang, Create
 *
 * Copyright(C)      Accton Corporation, 1999 - 2001
 *
 */
 
 
#ifndef SYSLOG_TASK_H
#define SYSLOG_TASK_H


/* INCLUDE FILE DECLARATIONS
 */


/* NAME CONSTANT DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */ 


/* FUNCTION NAME: SYSLOG_TASK_Initiate_System_Resources
 * PURPOSE: This function is used to initialize the system log module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   Initialize the system log module.
 *
 */
BOOL_T SYSLOG_TASK_Initiate_System_Resources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SYSLOG_TASK_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void SYSLOG_TASK_Create_InterCSC_Relation(void);

/* FUNCTION NAME: SYSLOG_TASK_Create_Tasks
 * PURPOSE: Create and start SYSLOG task.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T SYSLOG_TASK_Create_Tasks(void);


/* FUNCTION NAME: SYSLOG_TASK_EnterTransitionMode
 * PURPOSE: The function enter transition mode and free all SYSLOG resources 
 *          and reset database to factory default.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   1. The function MUST be called before calling the 
 *             SYSLOG_TASK_EnterMasterMode function.
 */
BOOL_T SYSLOG_TASK_EnterTransitionMode (void);


/* FUNCTION NAME: SYSLOG_TASK_EnterMasterMode
 * PURPOSE: The function enter master mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 */
BOOL_T SYSLOG_TASK_EnterMasterMode(void);


/* FUNCTION NAME: SYSLOG_TASK_EnterSlaveMode
 * PURPOSE: The function enter slave mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 */
BOOL_T SYSLOG_TASK_EnterSlaveMode(void);

/* FUNCTION NAME: SYSLOG_TASK_SetTransitionMode
 * PURPOSE: The function set transition mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 */
BOOL_T SYSLOG_TASK_SetTransitionMode (void);

/* MACRO FUNCTION DECLARATIONS
 */


#endif /* SYSLOG_TASK_H */
