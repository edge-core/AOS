/* =====================================================================================*
 * FILE NAME: SFLOW_INIT.c                                                              *
 *                                                                                      *
 * ABSTRACT:  The two primary functions of this file is to Initialize sflow resouce     *
 *            information and to create Task.                                           *
 *                                                                                      *
 * MODIFICATION HISOTRY:                                                                *
 *                                                                                      *
 * MODIFIER        DATE        DESCRIPTION                                              *
 * -------------------------------------------------------------------------------------*
 * Joeanne       10-25-2007    First Create                                             *
 *                                                                                      *
 * -------------------------------------------------------------------------------------*
 * Copyright(C)        Accton Techonology Corporation 2007                              *
 * =====================================================================================*/
 
/* INCLUDE FILE DECLARATIONS
 */
#include "sflow_init.h"
#include "sflow_task.h"
#include "sflow_mgr.h"

#if (SYS_CPNT_SFLOW == TRUE)

/* EXPORTED SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_INIT_Initiate_System_Resources
 *--------------------------------------------------------------------------
 * PURPOSE  : This function allocates and initiates the system resource for
 *            sflow module.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SFLOW_INIT_Initiate_System_Resources(void)
{
    //SFLOW_MGR_InitDefaultSflowEntry();
    SFLOW_TASK_Initiate_System_Resources();
    return;
} /* end of SFLOW_INIT_Initiate_System_Resources() */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SFLOW_INIT_Create_InterCSC_Relation
 *---------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTE:    None.
 *---------------------------------------------------------------------------
 */
void SFLOW_INIT_Create_InterCSC_Relation(void)
{
    SFLOW_MGR_Create_InterCSC_Relation();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_INIT_Create_Tasks
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will create task.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SFLOW_INIT_Create_Tasks(void)
{
    SFLOW_TASK_Create_Tasks();
    return;
} /* end of SFLOW_INIT_Create_Tasks() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_INIT_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set sflow_mgr into master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SFLOW_INIT_EnterMasterMode(void)
{
    SFLOW_MGR_EnterMasterMode();
    SFLOW_TASK_EnterMasterMode();
    return;
} /* end of SFLOW_INIT_EnterMasterMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_INIT_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set sflow_mgr into slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SFLOW_INIT_EnterSlaveMode(void)
{
    SFLOW_MGR_EnterSlaveMode();
    SFLOW_TASK_EnterSlaveMode();
    return;
} /* end of SFLOW_INIT_EnterSlaveMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_INIT_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set sflow_mgr into transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SFLOW_INIT_EnterTransitionMode(void)
{
    SFLOW_MGR_EnterTransitionMode();
    SFLOW_TASK_EnterTransitionMode();
    return;
} /* end of SFLOW_INIT_EnterTransitionMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_INIT_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void  SFLOW_INIT_SetTransitionMode(void)
{
    SFLOW_MGR_SetTransitionMode();
    SFLOW_TASK_SetTransitionMode();
    return;
} /* end of SFLOW_INIT_SetTransitionMode() */

#endif /* #if (SYS_CPNT_SFLOW == TRUE) */
