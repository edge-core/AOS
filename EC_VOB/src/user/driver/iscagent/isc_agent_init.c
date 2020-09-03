/* MODULE NAME:  ISC_AGENT_INIT.C
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

/* INCLUDE FILE DECLARATIONS
 */
#include "isc_agent_init.h"
#include "isc_agent.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
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
void ISC_AGENT_INIT_Initiate_System_Resources(void)
{
    ISC_AGENT_Init();
}

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
void ISC_AGENT_INIT_Create_InterCSC_Relation(void)
{
    ISC_AGENT_Create_InterCSC_Relation();
}

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
void ISC_AGENT_INIT_CreateTasks(void)
{
    ISC_AGENT_CreateTasks();
}

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
void ISC_AGENT_INIT_SetTransitionMode(void)
{
    ISC_AGENT_SetTransitionMode();
}

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
void ISC_AGENT_INIT_EnterTransitionMode(void)
{
    ISC_AGENT_EnterTransitionMode();
}

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
void ISC_AGENT_INIT_EnterSlaveMode(void)
{
    ISC_AGENT_EnterSlaveMode();
}

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
void ISC_AGENT_INIT_EnterMasterMode(void)
{
    ISC_AGENT_EnterMasterMode();
}

/* LOCAL SUBPROGRAM BODIES
 */

