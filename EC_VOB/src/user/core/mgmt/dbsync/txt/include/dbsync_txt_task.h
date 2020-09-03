/* MODULE NAME:  dbsync_txt_task.h
* PURPOSE: 
*   DBSYNC_TXT initiation and DBSYNC_TXT task creation
*   
* NOTES:
*
* History:                                                               
*       Date          -- Modifier,  Reason
*     2003-2-10       -- poli , created.
*   
* Copyright(C)      Accton Corporation, 2002
*/

#ifndef DBSYNC_TXT_TASK_H

#define DBSYNC_TXT_TASK_H



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

/* FUNCTION NAME:  DBSYNC_TXT_TASK_Init
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
 *          This function is invoked in DBSYNC_TXT_INIT_Initiate_System_Resources.
 */
BOOL_T DBSYNC_TXT_TASK_Init(void);



/* FUNCTION NAME:  DBSYNC_TXT_TASKCreateTask
 * PURPOSE: 
 *			This function create DBSYNC_TXT main task.
 * INPUT:   
 *          none. 
 *                                   
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          return TRUE to indicate success and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in DBSYNC_TXT_INIT_Create_Tasks().
 */ 
BOOL_T DBSYNC_TXT_TASK_CreateTask();



/* FUNCTION NAME : DBSYNC_TXT_TASK_SetTransitionMode
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
void DBSYNC_TXT_TASK_SetTransitionMode();



/* FUNCTION NAME : DBSYNC_TXT_TASK_EnterTransitionMode
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
void DBSYNC_TXT_TASK_EnterTransitionMode();


#endif /* #ifndef DBSYNC_TXT_TASK_H */



