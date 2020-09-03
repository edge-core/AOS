/*-----------------------------------------------------------------------------
 * Module Name: cfm_init.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate the system resources and create the task.
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    12/5/2006 - macauley_cheng, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2006
 *-----------------------------------------------------------------------------
 */
#include "cfm_init.h"
#include "cfm_mgr.h"
#include "cfm_om.h"
#if 0
#include "cfm_task.h"
#endif

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_INIT_Initiate_System_Resources
 * ------------------------------------------------------------------------
 * FUNCTION : Initiate resources.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CFM_INIT_Initiate_System_Resources()
{
#if 0
    CFM_TASK_Init();
#endif
    CFM_MGR_Init();
    CFM_OM_Init();

    return;
}/* End of CFM_INIT_Initiate_System_Resources */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_INIT_Create_InterCSC_Relation()
 *-------------------------------------------------------------------------
 * FUNCTION: Register other CSC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void CFM_INIT_Create_InterCSC_Relation()
{

#if 0
    CFM_TASK_Create_InterCSC_Relation();
#endif
    return;
}
#if 0
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_INIT_CreateTask
 * ------------------------------------------------------------------------
 * FUNCTION : Create CFM tasks.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CFM_INIT_CreateTasks()
{
    CFM_TASK_CreateTask();
    return;
} /* End of CFM_INIT_CreateTask */
#endif
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_INIT_SetTransitionMode
 * ------------------------------------------------------------------------
 * FUNCTION : Set transition mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CFM_INIT_SetTransitionMode()
{
#if 0
    CFM_TASK_SetTransitionMode();
#endif
    CFM_OM_SetTransitionMode();
    CFM_MGR_SetTransitionMode();

    return;
}/* End of CFM_INIT_SetTransitionMode */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_INIT_EnterTransitionMode
 * ------------------------------------------------------------------------
 * FUNCTION : Enter transition mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CFM_INIT_EnterTransitionMode()
{
#if 0
    CFM_TASK_EnterTransitionMode();
#endif
    CFM_OM_EnterTransitionMode();
    CFM_MGR_EnterTransitionMode();
    return;
}/* End of CFM_INIT_EnterTransitionMode */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_INIT_EnterSlaveMode
 * ------------------------------------------------------------------------
 * FUNCTION : Enter master mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CFM_INIT_EnterSlaveMode()
{
#if 0
    CFM_TASK_EnterSlaveMode();
#endif
    CFM_OM_EnterSlaveMode();
    CFM_MGR_EnterSlaveMode();
    return;
}/* End of CFM_INIT_EnterSlaveMode */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_INIT_EnterMasterMode
 * ------------------------------------------------------------------------
 * FUNCTION : Enter master mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CFM_INIT_EnterMasterMode()
{
#if 0
    CFM_TASK_EnterMasterMode();
#endif
    CFM_OM_EnterMasterMode();
    CFM_MGR_EnterMasterMode();
    return;
}/* End of CFM_INIT_EnterMasterMode */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_ProvisionComplete
 * ------------------------------------------------------------------------
 * FUNCTION : provision complete.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CFM_INIT_ProvisionComplete()
{
#if 0
    //Linux plateform this function needn't be used.
    CFM_TASK_ProvisionComplete();
#endif

    return ;
}/* End of CFM_INIT_ProvisionComplete */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_INIT_HandleHotInsertion
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the module hot insert.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void CFM_INIT_HandleHotInsertion(UI32_T beg_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    CFM_MGR_HandleHotInsertion(beg_ifindex, number_of_port, use_default);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_INIT_HandleHotRemoval
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the module hot insert.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void CFM_INIT_HandleHotRemoval(UI32_T beg_ifindex, UI32_T number_of_port)
{
    CFM_MGR_HandleHotRemoval(beg_ifindex, number_of_port);
}

