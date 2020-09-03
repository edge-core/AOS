/* FUNCTION NAME: add_init.h
 * PURPOSE:
 *	1. Initialize the add task
 * NOTES:
 *
 * REASON:
 *    DESCRIPTION:
 *    CREATOR:	Junying
 *    Date       2006/04/26
 *
 * Copyright(C)      Accton Corporation, 2006
 */

/*------------------------------------------------------------------------
 * INCLUDE STRUCTURES
 *------------------------------------------------------------------------*/
#include "sys_type.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_ADD == TRUE)

#include <stdio.h>
#include "sysfun.h"
#include "add_init.h"
#include "add_task.h"
#include "add_mgr.h"

/* EXPORTED SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - ADD_INIT_Init
 *---------------------------------------------------------------------------
 * PURPOSE: This function initaites the system resources, such as queue, semaphore,
 *          and events used by this subsystem. All the call back functions shall be
 *          registered during subsystem initiation.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:
 *---------------------------------------------------------------------------
 */
void ADD_INIT_Initiate_System_Resources(void)
{
    if (!ADD_MGR_Initiate_System_Resources())
    {
      SYSFUN_Debug_Printf("\r\nInitiate ADD_MGR_Initiate_System_Resources() Error!!");
      return ;
    }

    if (!ADD_TASK_Initiate_System_Resources())
    {
        SYSFUN_Debug_Printf("\r\nInitiate ADD_TASK_Initiate_System_Resources() Error!!");
        return;
    }
} /* end of ADD_INIT_Initiate_System_Resources() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ADD_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void ADD_INIT_Create_InterCSC_Relation()
{
    ADD_MGR_Create_InterCSC_Relation();
}


/*--------------------------------------------------------------------------
 * ROUTINE NAME - ADD_INIT_Create_Task()
 *---------------------------------------------------------------------------
 * PURPOSE: This function creates all the task of this subsystem.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:
 *---------------------------------------------------------------------------
 */
void ADD_INIT_CreateTask(void)
{
    /* Create console task since this fuction can only be called by Root
     */
    ADD_TASK_CreateTask();

} /* End of ADD_INIT_CreateTask() */



/*-------------------------------------------------------------------------
 * ROUTINE NAME - ADD_INIT_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE: This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the CLI subsystem will enter the
 *          Master Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE:
 *-------------------------------------------------------------------------
 */
void ADD_INIT_EnterMasterMode(void)
{
    ADD_MGR_EnterMasterMode();
    ADD_TASK_EnterMasterMode();
} /* End of ADD_INIT_EnterMasterMode() */

void ADD_INIT_ProvisionComplete()
{
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ADD_INIT_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : It's the temporary transition mode between system into master
 *            mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *--------------------------------------------------------------------------*/
void ADD_INIT_EnterTransitionMode(void)
{
    ADD_MGR_EnterTransitionMode();
    ADD_TASK_EnterTransitionMode();
}/* end ADD_INIT_EnterTransitionMode() */

/*-------------------------------------------------------------------------
 * ROUTINE NAME - ADD_INIT_EnterSlaveMode
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Slave Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE:
 *-------------------------------------------------------------------------
 */
void ADD_INIT_EnterSlaveMode(void)
{
    ADD_MGR_EnterSlaveMode();
    ADD_TASK_EnterSlaveMode();
} /* end ADD_INIT_EnterSlaveMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ADD_INIT_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This call will set dhcp_mgr into transition mode to prevent
 *            calling request.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void ADD_INIT_SetTransitionMode(void)
{
    ADD_MGR_SetTransitionMode();
    ADD_TASK_SetTransitionMode();
}/* end ADD_INIT_SetTransitionMode() */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - ADD_INIT_HandleHotInsertion
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
void ADD_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - ADD_INIT_HandleHotRemoval
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
void ADD_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
}

/* End of ADD_INIT.C */#endif  /* #if (SYS_CPNT_ADD == TRUE) */
