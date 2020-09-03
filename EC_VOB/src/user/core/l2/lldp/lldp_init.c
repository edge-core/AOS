/*-----------------------------------------------------------------------------
 * Module Name: lldp_init.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate the system resources and create the task.
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 */


#include "sys_type.h"
#include "lldp_init.h"
#include "lldp_mgr.h"


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_INIT_InitiateSystemResources
 * ------------------------------------------------------------------------
 * FUNCTION : Initiate resources.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void LLDP_INIT_InitiateSystemResources(void)
{
    LLDP_MGR_Init();
    return;
}/* End of LLDP_INIT_InitiateSystemResources */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_INIT_Create_InterCSC_Relation()
 *-------------------------------------------------------------------------
 * FUNCTION: Register other CSC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void LLDP_INIT_Create_InterCSC_Relation(void)
{
    LLDP_MGR_Create_InterCSC_Relation();
    return;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_INIT_CreateTask
 * ------------------------------------------------------------------------
 * FUNCTION : Create LLDP tasks.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void LLDP_INIT_CreateTasks(void)
{
//    LLDP_TASK_CreateTasks();
    return;
} /* End of LLDP_INIT_CreateTask */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_INIT_SetTransitionMode
 * ------------------------------------------------------------------------
 * FUNCTION : Set transition mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void LLDP_INIT_SetTransitionMode(void)
{
    LLDP_MGR_SetTransitionMode();
    return;
}/* End of LLDP_INIT_SetTransitionMode */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_INIT_EnterTransitionMode
 * ------------------------------------------------------------------------
 * FUNCTION : Enter transition mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void LLDP_INIT_EnterTransitionMode(void)
{
    LLDP_MGR_EnterTransitionMode();
    return;
}/* End of LLDP_INIT_EnterTransitionMode */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_INIT_EnterSlaveMode
 * ------------------------------------------------------------------------
 * FUNCTION : Enter master mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void LLDP_INIT_EnterSlaveMode(void)
{
    LLDP_MGR_EnterSlaveMode();
    return;
}/* End of LLDP_INIT_EnterSlaveMode */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_INIT_EnterMasterMode
 * ------------------------------------------------------------------------
 * FUNCTION : Enter master mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void LLDP_INIT_EnterMasterMode(void)
{
    LLDP_MGR_EnterMasterMode();
    return;
}/* End of LLDP_INIT_EnterMasterMode */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_TASK_ProvisionComplete
 * ------------------------------------------------------------------------
 * FUNCTION : provision complete.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void LLDP_INIT_ProvisionComplete(void)
{
    LLDP_MGR_ProvisionComplete();
    return ;
}/* End of LLDP_INIT_ProvisionComplete */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_INIT_HandleHotInsertion
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the module hot insert.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_INIT_HandleHotInsertion(UI32_T beg_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    LLDP_MGR_HandleHotInsertion(beg_ifindex, number_of_port, use_default);
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_INIT_HandleHotRemoval
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the module hot insert.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_INIT_HandleHotRemoval(UI32_T beg_ifindex, UI32_T number_of_port)
{
    LLDP_MGR_HandleHotRemoval(beg_ifindex, number_of_port);
    return ;
}

