/* MODULE NAME:  ISC_AGENT_INIT.H
 * PURPOSE:
 *
 * NOTES:
 *
 * CREATOR:      Charlie Chen
 * HISTORY
 *    2/7/2006 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2006
 */
#ifndef ISC_AGENT_INIT_H
#define ISC_AGENT_INIT_H

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
/*--------------------------------------------------------------------------
 * FUNCTION NAME - ISC_AGENT_INIT_Initiate_System_Resources
 *--------------------------------------------------------------------------
 * PURPOSE  : This function is called by root to do system resources initialization
 *            for ISC_AGENT.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void ISC_AGENT_INIT_Initiate_System_Resources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ISC_AGENT_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void ISC_AGENT_INIT_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ISC_AGENT_INIT_CreateTasks
 *--------------------------------------------------------------------------
 * PURPOSE  : This function is called to create tasks for ISC_AGENT.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void ISC_AGENT_INIT_CreateTasks(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ISC_AGENT_INIT_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function is called to Enter Transition Mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void ISC_AGENT_INIT_SetTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ISC_AGENT_INIT_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function is called to Enter Transition Mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void ISC_AGENT_INIT_EnterTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ISC_AGENT_INIT_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function is called to Enter Slave Mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void ISC_AGENT_INIT_EnterSlaveMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ISC_AGENT_INIT_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function is called to Enter Master Mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void ISC_AGENT_INIT_EnterMasterMode(void);
# endif    /* End of ISC_AGENT_INIT_H */
