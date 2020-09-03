/* Module Name: PING_INIT.C
 * Purpose:
 *     This component implements the standard Ping-MIB functionality (rfc2925).
 *
 * Notes:
 *      1. This design references the TRACEROUTE module.
 *
 * History:
 *      Date        --  Modifier,   Reason
 *      2007/3      --  peter_yu    Create to support partial standard ping-MIB.
 *
 * Copyright(C)      Accton Corporation, 2007
 */

 /* INCLUDE FILE DECLARATIONS
 */
#include "ping_init.h"
#include "ping_mgr.h"
#include "ping_task.h"


/* FUNCTION NAME - PING_INIT_Initiate_System_Resources
 * PURPOSE  : This function allocates and initiates the system resource for
 *            PING module.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *
 */
void PING_INIT_Initiate_System_Resources(void)
{
    PING_MGR_Initiate_System_Resources();
    PING_TASK_Initiate_System_Resources();
    return;
} /* end of PING_INIT_Initiate_System_Resources() */


/* FUNCTION NAME - PING_INIT_Create_Tasks
 * PURPOSE  : This function will create task.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_INIT_Create_Tasks(void)
{
    PING_TASK_CreateTask();

} /* end of PING_INIT_Create_Tasks() */



/* FUNCTION NAME - PING_INIT_EnterMasterMode
 * PURPOSE  : This function will call set PING into master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_INIT_EnterMasterMode(void)
{
    PING_MGR_EnterMasterMode();
    PING_TASK_EnterMasterMode();
    return;
} /* end of PING_INIT_EnterMasterMode() */


/* FUNCTION NAME - PING_INIT_EnterSlaveMode
 * PURPOSE  : This function will call set PING into slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_INIT_EnterSlaveMode(void)
{
    PING_MGR_EnterSlaveMode();
    PING_TASK_EnterMasterMode();
    return;
} /* end of PING_INIT_EnterSlaveMode() */


/* FUNCTION NAME - PING_INIT_EnterTransitionMode
 * PURPOSE  : This function will call set PING into transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_INIT_EnterTransitionMode(void)
{
    PING_TASK_EnterTransitionMode();
    PING_MGR_EnterTransitionMode();

    return;
} /* end of PING_INIT_EnterTransitionMode() */


/* FUNCTION NAME - PING_INIT_SetTransitionMode
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void  PING_INIT_SetTransitionMode(void)
{
    PING_MGR_SetTransitionMode();
    PING_TASK_SetTransitionMode();
    return;
} /* end of PING_INIT_SetTransitionMode() */

/* FUNCTION NAME - PING_INIT_ProvisionComplete
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void  PING_INIT_ProvisionComplete(void)
{
    PING_TASK_ProvisionComplete();
} /* end of PING_INIT_ProvisionComplete() */



