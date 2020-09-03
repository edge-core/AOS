/* FUNCTION NAME: add_task.h
 * PURPOSE:
 *	1. ADD initiation and task creation
 * NOTES:
 *
 * REASON:
 *    DESCRIPTION:
 *    CREATOR:	Junying
 *    Date       2006/04/26
 *
 * Copyright(C)      Accton Corporation, 2006
 */

#ifndef	ADD_TASK_H
#define	ADD_TASK_H

#include "sys_type.h"

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_TASK_Initiate_System_Resources
 * ---------------------------------------------------------------------
 * PURPOSE: Init system resource.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_TASK_Initiate_System_Resources(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_TASK_CreateTask
 * ---------------------------------------------------------------------
 * PURPOSE: Create and start ADD task.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
void ADD_TASK_CreateTask(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_TASK_SetTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Sending enter transition event to task calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void ADD_TASK_SetTransitionMode();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_TASK_EnterTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Sending enter transition event to task calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void ADD_TASK_EnterTransitionMode();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_TASK_EnterMasterMode
 * ---------------------------------------------------------------------
 * PURPOSE: Enter Master mode calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void ADD_TASK_EnterMasterMode();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_TASK_EnterSlaveMode
 * ---------------------------------------------------------------------
 * PURPOSE: Enter Slave mode calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void ADD_TASK_EnterSlaveMode();

#endif /* ADD_TASK_H */
