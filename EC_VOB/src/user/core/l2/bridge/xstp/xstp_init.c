/*-----------------------------------------------------------------------------
 * Module Name: xstp_init.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate the system resources and create the task.
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    05/30/2001 - Allen Cheng, Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */
/*#include "xstp_task.h"*/
#include "xstp_mgr.h"


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_INIT_Initiate_System_Resources
 *-------------------------------------------------------------------------
 * PURPOSE  : Init XSTP data
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void XSTP_INIT_Initiate_System_Resources(void)
{
    XSTP_TASK_Init();

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_INIT_Create_InterCSC_Relation
 *-------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void XSTP_INIT_Create_InterCSC_Relation(void)
{
    XSTP_TASK_Create_InterCSC_Relation();
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_INIT_Create_Tasks
 *-------------------------------------------------------------------------
 * PURPOSE  : Create XSTP task
 * INPUT    : tg_handle - the handle of thread group
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void XSTP_INIT_Create_Tasks(void)
{
    /*XSTP_TASK_CreateTask();*/

    return;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_INIT_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE : Enter master mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void XSTP_INIT_EnterMasterMode(void)
{
    XSTP_MGR_EnterMasterMode();
    /*XSTP_TASK_EnterMasterMode();*/
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_INIT_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE : Enter slave mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void XSTP_INIT_EnterSlaveMode(void)
{
    XSTP_MGR_EnterSlaveMode();
    /*XSTP_TASK_EnterSlaveMode();*/
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_INIT_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE : Enter transition mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void XSTP_INIT_EnterTransitionMode(void)
{
    XSTP_MGR_EnterTransitionMode();
    /*XSTP_TASK_EnterTransitionMode();*/

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_INIT_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE : Set transition mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void XSTP_INIT_SetTransitionMode(void)
{
    XSTP_MGR_SetTransitionMode();
    /*XSTP_TASK_SetTransitionMode();*/

    return;
} /* End of XSTP_INIT_SetTransitionMode */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_INIT_HandleHotInsertion
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 * ------------------------------------------------------------------------
 */
void XSTP_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{

   XSTP_MGR_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);

} /* end of XSTP_INIT_HandleHotInsertion() */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_INIT_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void XSTP_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{

   XSTP_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);

}/* end of XSTP_INIT_HandleHotRemoval() */
