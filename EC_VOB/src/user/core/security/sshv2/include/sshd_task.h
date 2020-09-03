/* MODULE NAME:  sshd_task.h
* PURPOSE:
*   SSHD initiation and SSHD task creation
*
* NOTES:
*
* History:
*       Date          -- Modifier,  Reason
*     2003-03-27      -- Isiah , created.
*
* Copyright(C)      Accton Corporation, 2003
*/

#ifndef SSHD_TASK_H

#define SSHD_TASK_H



/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    int accepted_socket;
} SSHD_TASK_SessionThreadArgs;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:  SSHD_TASK_Init
 * PURPOSE:
 *          This function init the message queue.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in SSHD_INIT_InitiateSystemResources.
 */
BOOL_T SSHD_TASK_Init(void);



/* FUNCTION NAME:  SSHD_TASK_CreateTask
 * PURPOSE:
 *			This function create sshd main task.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          return TRUE to indicate success and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in SSHD_INIT_CreateTasks().
 */
BOOL_T SSHD_TASK_CreateTask(void);



/* FUNCTION NAME : SSHD_TASK_SetTransitionMode
 * PURPOSE:
 *		Sending enter transition event to task calling by stkctrl.
 * INPUT:
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *     None.
 *
 */
void SSHD_TASK_SetTransitionMode(void);



/* FUNCTION NAME : SSHD_TASK_EnterTransitionMode
 * PURPOSE:
 *		Leave CSC Task while transition done.
 * INPUT:
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *     None.
 *
 */
void SSHD_TASK_EnterTransitionMode(void);



/* FUNCTION NAME:  SSHD_TASK_ProvisionComplete
 * PURPOSE:
 *          This function will tell the SSH module to start.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 *
 * NOTES:
 *          This function is invoked in SSHD_INIT_ProvisionComplete().
 */
void SSHD_TASK_ProvisionComplete(void);



/* FUNCTION NAME:  SSHD_TASK_RegisterCallBackFunction
 * PURPOSE:
 *			Register Callback Function.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function is invoked in SSHD_INIT_RegisterCallBackFunction().
 */
void SSHD_TASK_RegisterCallBackFunction(void);




#endif /* #ifndef SSHD_TASK_H */



