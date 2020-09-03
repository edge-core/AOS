/* Module Name: PING_TASK.H
 * Purpose:
 *      This component implements the standard Ping-MIB functionality (rfc2925).
 *
 * Notes:
 *      1. This design references the TRACEROUTE module.
 *
 * History:
 *      Date        --  Modifier,   Reason
 *      2007/3      --  peter_yu    Create to support partial standard Ping-MIB.
 *
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef     _PING_TASK_H
#define     _PING_TASK_H


/* INCLUDE FILE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME - PING_TASK_Initiate_System_Resources
 * PURPOSE  : This function allocates and initiates the system resource for
 *            PING database.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_TASK_Initiate_System_Resources(void);


/* FUNCTION NAME - PING_TASK_EnterMasterMode
 * PURPOSE  : This function will configured PING to enter master mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_TASK_EnterMasterMode(void);


/* FUNCTION NAME - PING_TASK_EnterTransitionMode
 * PURPOSE  : This function will configured PING to enter transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_TASK_EnterTransitionMode(void);


/* FUNCTION NAME - PING_TASK_EnterSlaveMode
 * PURPOSE  : This function will configured PING to enter master mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_TASK_EnterSlaveMode(void);


/* FUNCTION NAME - PING_TASK_SetTransitionMode
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void  PING_TASK_SetTransitionMode(void);


/* FUNCTION NAME - PING_TASK_CreateTask
 * PURPOSE  : This function will create PING task.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_TASK_CreateTask(void);


/* FUNCTION NAME - PING_TASK_ProvisionComplete
 * PURPOSE  : This function will create socket after provision complete
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_TASK_ProvisionComplete(void);

/* FUNCTION NAME - PING_TASK_PeriodicTimerStart_Callback
 * PURPOSE  : This function will send event to ping task in order to 
 *            start periodic timer
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_TASK_PeriodicTimerStart_Callback(void);


#endif
