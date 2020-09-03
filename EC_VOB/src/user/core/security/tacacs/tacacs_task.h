/* Project Name: New Feature
 * File_Name : Tacacs_task.h
 * Purpose     : Tacacs initiation and Tacacs task creation
 *
 * 2004/06/08    : Ricky Lin     Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (Mercury)
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)

/*---------------------------------------------------------------------------
 * Routine Name : TACACS_TASK_CreateTACACSTask()
 *---------------------------------------------------------------------------
 * Function : Create and start TACACS task
 * Input    : None								                             +
 * Output   :
 * Return   : never returns
 * Note     :
 *---------------------------------------------------------------------------*/
void TACACS_TASK_CreateTACACSTask(void);
/*---------------------------------------------------------------------------+
 * Routine Name : TACACS_TASK_Init()                                           +
 *---------------------------------------------------------------------------+
 * Function : Initialize TACACS 's Task .	                                     +
 * Input    : None                                                           +
 * Output   :                                                                +
 * Return   : never returns                                                  +
 * Note     :                                                                +
 *---------------------------------------------------------------------------*/
void TACACS_TASK_Init(void);

/* FUNCTION NAME: TACACS_TASK_Initiate_System_Resources
 * PURPOSE: This function is used to initialize the stack control module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T TACACS_TASK_Initiate_System_Resources(void);


/* FUNCTION NAME : TACACS_TASK_SetTransitionMode
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
void TACACS_TASK_SetTransitionMode();

/* FUNCTION NAME : TACACS_TASK_EnterTransitionMode
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
void 	TACACS_TASK_EnterTransitionMode();

/* FUNCTION NAME : TACACS_TASK_EnterMasterMode
 * PURPOSE:
 *		Enter Master mode calling by stkctrl.
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
void TACACS_TASK_EnterMasterMode();

/* FUNCTION NAME : TACACS_TASK_EnterSlaveMode
 * PURPOSE:
 *		Enter Slave mode calling by stkctrl.
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
void TACACS_TASK_EnterSlaveMode();
#endif
