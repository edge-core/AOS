/* Project Name: New Feature
 * File_Name : Radius_task.h
 * Purpose     : Radius initiation and Radius task creation
 *
 * 2001/11/22    : JJ Young     Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (Mercury)
 */
#include "sys_type.h"

/*---------------------------------------------------------------------------
 * Routine Name : RADIUS_TASK_CreateRadiusTask()
 *---------------------------------------------------------------------------
 * Function : Create and start RADIUS task
 * Input    : None
 * Output   :
 * Return   : never returns
 * Note     :
 *---------------------------------------------------------------------------*/
void RADIUS_TASK_CreateRadiusTask(void);
/*---------------------------------------------------------------------------+
 * Routine Name : RADIUS_TASK_Init()                                         +
 *---------------------------------------------------------------------------+
 * Function : Initialize RADIUS 's Task .                                    +
 * Input    : None                                                           +
 * Output   :                                                                +
 * Return   : never returns                                                  +
 * Note     :                                                                +
 *---------------------------------------------------------------------------*/
void RADIUS_TASK_Init(void);

/* FUNCTION NAME: RADIUS_TASK_InitiateProcessResources
 * PURPOSE: This function is used to initialize the stack control module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T RADIUS_TASK_InitiateProcessResources(void);

/* FUNCTION NAME: RADIUS_TASK_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:
 *
 */
void RADIUS_TASK_Create_InterCSC_Relation(void);

/* FUNCTION NAME : RADIUS_TASK_SetTransitionMode
 * PURPOSE:
 *      Sending enter transition event to task calling by stkctrl.
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
void RADIUS_TASK_SetTransitionMode();

/* FUNCTION NAME : RADIUS_TASK_EnterTransitionMode
 * PURPOSE:
 *      Sending enter transition event to task calling by stkctrl.
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
void    RADIUS_TASK_EnterTransitionMode();

/* FUNCTION NAME : RADIUS_TASK_EnterMasterMode
 * PURPOSE:
 *      Enter Master mode calling by stkctrl.
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
void RADIUS_TASK_EnterMasterMode();

/* FUNCTION NAME : RADIUS_TASK_EnterSlaveMode
 * PURPOSE:
 *      Enter Slave mode calling by stkctrl.
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
void RADIUS_TASK_EnterSlaveMode();
