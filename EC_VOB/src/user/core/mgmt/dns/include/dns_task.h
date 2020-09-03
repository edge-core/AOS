/* MODULE NAME: dns_task.h
 * PURPOSE:
 *   DNS initiation and task creation
 *
 * NOTES:
 *
 * History:
 *       Date          -- Modifier,  Reason
 *     2002-11-14      -- Isiah , created.
 *
 * Copyright(C)      Accton Corporation, 2002
 */

#ifndef DNS_TASK_H

#define DNS_TASK_H



/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */
#include "sys_type.h"

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:  DNS_TASK_Init
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
 *          This function is invoked in DNS_INIT_Initiate_System_Resources.
 */
BOOL_T DNS_TASK_Init(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DNS_TASK_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void DNS_TASK_Create_InterCSC_Relation(void);

/* FUNCTION NAME:  DNS_TASK_CreateResolverTask
 * PURPOSE:
 *			This function create dns resolver task.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          return TRUE to indicate success and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in DNS_INIT_Create_Tasks().
 */
BOOL_T DNS_TASK_CreateResolverTask(void);



/* FUNCTION NAME:  DNS_TASK_CreateProxyDaemon
 * PURPOSE:
 *			This function create dns proxy daemon.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          return TRUE to indicate success and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in DNS_INIT_Create_Tasks().
 */
BOOL_T DNS_TASK_CreateProxyDaemon(void);



/* FUNCTION NAME : DNS_TASK_SetTransitionMode
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
void DNS_TASK_SetTransitionMode(void);



/* FUNCTION NAME : DNS_TASK_EnterTransitionMode
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
void DNS_TASK_EnterTransitionMode(void);



/* FUNCTION NAME:  DNS_TASK_ProvisionComplete
 * PURPOSE:
 *          This function will tell the DNS module to start.
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
 *          This function is invoked in DNS_INIT_ProvisionComplete().
 */
void DNS_TASK_ProvisionComplete(void);

#endif  /* #ifndef DNS_TASK_H */
