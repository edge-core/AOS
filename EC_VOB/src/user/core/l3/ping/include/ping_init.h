/* Module Name: PING_INIT.H
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

#ifndef PING_INIT_H
#define PING_INIT_H


/* FUNCTION NAME - PING_INIT_Initiate_System_Resources
 * PURPOSE  : This function allocates and initiates the system resource for
 *            PING module.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *
 */
void PING_INIT_Initiate_System_Resources(void);


/* FUNCTION NAME - PING_INIT_Create_Tasks
 * PURPOSE  : This function will create task.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_INIT_Create_Tasks(void);


/* FUNCTION NAME - PING_INIT_EnterMasterMode
 * PURPOSE  : This function will call set PING into master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_INIT_EnterMasterMode(void);


/* FUNCTION NAME - PING_INIT_EnterSlaveMode
 * PURPOSE  : This function will call set PING into slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_INIT_EnterSlaveMode(void);


/* FUNCTION NAME - PING_INIT_EnterTransitionMode
 * PURPOSE  : This function will call set PING into transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_INIT_EnterTransitionMode(void);


/* FUNCTION NAME - PING_INIT_SetTransitionMode
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void  PING_INIT_SetTransitionMode(void);

/* FUNCTION NAME - PING_INIT_ProvisionComplete
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void  PING_INIT_ProvisionComplete(void);

#endif
