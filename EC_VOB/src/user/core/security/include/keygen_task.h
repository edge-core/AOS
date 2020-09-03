/* MODULE NAME:  keygen_task.h
* PURPOSE: 
*   KEYGEN initiation and KEYGEN task creation
*   
* NOTES:
*
* History:                                                               
*       Date          -- Modifier,  Reason
*     2002-10-16      -- Isiah , created.
*   
* Copyright(C)      Accton Corporation, 2002
*/

#ifndef KEYGEN_TASK_H

#define KEYGEN_TASK_H



/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:  KEYGEN_TASK_Init
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
 *          This function is invoked in KEYGEN_INIT_Initiate_System_Resources.
 */
BOOL_T KEYGEN_TASK_Init(void);



/* FUNCTION NAME:  KEYGEN_TASKCreateTask
 * PURPOSE: 
 *			This function create keygen main task.
 * INPUT:   
 *          none. 
 *                                   
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          return TRUE to indicate success and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in KEYGEN_INIT_Create_Tasks().
 */ 
BOOL_T KEYGEN_TASK_CreateTask();



/* FUNCTION NAME : KEYGEN_TASK_SetTransitionMode
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
void KEYGEN_TASK_SetTransitionMode();



/* FUNCTION NAME : KEYGEN_TASK_EnterTransitionMode
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
void KEYGEN_TASK_EnterTransitionMode();



/* FUNCTION NAME:  KEYGEN_TASK_ProvisionComplete
 * PURPOSE:
 *          This function will tell the KEYGEN module to start.
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
 *          This function is invoked in KEYGEN_INIT_ProvisionComplete().
 */
void KEYGEN_TASK_ProvisionComplete(void);


#endif /* #ifndef KEYGEN_TASK_H */



