/*-----------------------------------------------------------------------------
 * Module Name: dcbx_init.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate the system resources and create the task.
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 */


#include "sys_type.h"
#include "dcbx_init.h"
#include "dcbx_mgr.h"


/* ------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_INIT_InitiateSystemResources
 * ------------------------------------------------------------------------
 * FUNCTION : Initiate resources.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void DCBX_INIT_InitiateSystemResources(void)
{
    DCBX_MGR_Init();
    return;
}/* End of DCBX_INIT_InitiateSystemResources */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_INIT_Create_InterCSC_Relation()
 *-------------------------------------------------------------------------
 * FUNCTION: Register other CSC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void DCBX_INIT_Create_InterCSC_Relation(void)
{
    DCBX_MGR_Create_InterCSC_Relation();
    return;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_INIT_CreateTask
 * ------------------------------------------------------------------------
 * FUNCTION : Create DCBX tasks.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void DCBX_INIT_CreateTasks(void)
{
//    DCBX_TASK_CreateTasks();
    return;
} /* End of DCBX_INIT_CreateTask */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_INIT_SetTransitionMode
 * ------------------------------------------------------------------------
 * FUNCTION : Set transition mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void DCBX_INIT_SetTransitionMode(void)
{
    DCBX_MGR_SetTransitionMode();
    return;
}/* End of DCBX_INIT_SetTransitionMode */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_INIT_EnterTransitionMode
 * ------------------------------------------------------------------------
 * FUNCTION : Enter transition mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void DCBX_INIT_EnterTransitionMode(void)
{
    DCBX_MGR_EnterTransitionMode();
    return;
}/* End of DCBX_INIT_EnterTransitionMode */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_INIT_EnterSlaveMode
 * ------------------------------------------------------------------------
 * FUNCTION : Enter master mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void DCBX_INIT_EnterSlaveMode(void)
{
    DCBX_MGR_EnterSlaveMode();
    return;
}/* End of DCBX_INIT_EnterSlaveMode */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_INIT_EnterMasterMode
 * ------------------------------------------------------------------------
 * FUNCTION : Enter master mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void DCBX_INIT_EnterMasterMode(void)
{
    DCBX_MGR_EnterMasterMode();
    return;
}/* End of DCBX_INIT_EnterMasterMode */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_TASK_ProvisionComplete
 * ------------------------------------------------------------------------
 * FUNCTION : provision complete.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void DCBX_INIT_ProvisionComplete(void)
{
    DCBX_MGR_ProvisionComplete();
    return ;
}/* End of DCBX_INIT_ProvisionComplete */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_INIT_HandleHotInsertion
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the module hot insert.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_INIT_HandleHotInsertion(UI32_T beg_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    DCBX_MGR_HandleHotInsertion(beg_ifindex, number_of_port, use_default);
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_INIT_HandleHotRemoval
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the module hot insert.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_INIT_HandleHotRemoval(UI32_T beg_ifindex, UI32_T number_of_port)
{
    DCBX_MGR_HandleHotRemoval(beg_ifindex, number_of_port);
    return ;
}

