/* Module Name: SYSMGMT_INIT.C
 * Purpose:
 *  This module is in first task of System Management, bring up whole system
 *  manager components.
 *
 * Notes:
 *
 * History:
 *          Date      -- Modifier,        Reason
 * ------------------------------------------------------------------------
 *  V1.0  2001.11.12  -- Jason Hsue,      Creation
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "sysmgmt_task.h"
#if (SYS_CPNT_SYS_MGR == TRUE)
#include "sys_mgr.h"
#endif

#if (SYS_CPNT_SYS_TIME == TRUE)
#include "sys_time.h"
#endif

#if (SYS_CPNT_MOTD == TRUE)
#include "sys_bnr_mgr.h"
#endif


/* FUNCTION NAME: SYSMGMT_INIT_Initiate_System_Resources
 *----------------------------------------------------------------------------------
 * PURPOSE: Init system management
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
void SYSMGMT_INIT_InitiateProcessResource(void)
{
    SYS_MGR_Init();

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
    SYS_RELOAD_MGR_Init();
#endif
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SYSMGMT_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SYSMGMT_INIT_Create_InterCSC_Relation(void)
{
    SYS_MGR_Create_InterCSC_Relation();

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
    SYS_RELOAD_MGR_Create_InterCSC_Relation();
#endif
} /* end of SYSMGMT_INIT_Create_InterCSC_Relation */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYSMGMT_INIT_SetTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set transition state
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 * ---------------------------------------------------------------------
 */
void SYSMGMT_INIT_SetTransitionMode(void)
{
    SYS_MGR_SetTransitionMode();

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
    SYS_RELOAD_MGR_SetTransitionMode();
#endif
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYSMGMT_INIT_EnterTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will enter transition mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 * ---------------------------------------------------------------------
 */
void SYSMGMT_INIT_EnterTransitionMode(void)
{
    SYS_MGR_EnterTransitionMode();

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
    SYS_RELOAD_MGR_EnterTransitionMode();
#endif
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYSMGMT_INIT_EnterMasterMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will enter master mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 * ---------------------------------------------------------------------
 */
void SYSMGMT_INIT_EnterMasterMode(void)
{
    SYS_MGR_EnterMasterMode();

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
    SYS_RELOAD_MGR_EnterMasterMode();
#endif
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYSMGMT_INIT_EnterSlaveMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will enter slave mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 * ---------------------------------------------------------------------
 */
void SYSMGMT_INIT_EnterSlaveMode(void)
{
    SYS_MGR_EnterSlaveMode();

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
    SYS_RELOAD_MGR_EnterSlaveMode();
#endif
}

/* ---------------------------------------------------------------------
 * FUNCTION NAME: SYSMGMT_INIT_Create_Tasks
 *----------------------------------------------------------------------------------
 * PURPOSE: create SYSMGMT task
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   There is no task for System Management
 * ---------------------------------------------------------------------
 */
void SYSMGMT_INIT_Create_Tasks(void)
{
    SYSMGMT_TASK_CreateTask();
    return;
}

/* ---------------------------------------------------------------------
 * FUNCTION NAME: SYSMGMT_INIT_ProvisionComplete
 *----------------------------------------------------------------------------------
 * PURPOSE: Notify this CSC provision complete.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   None
 * ---------------------------------------------------------------------
 */
void SYSMGMT_INIT_ProvisionComplete(void)
{
    SYS_MGR_ProvisionComplete();

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
    SYS_RELOAD_MGR_ProvisionComplete();
#endif
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - SYSMGMT_INIT_HandleHotInsertion
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
void SYSMGMT_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
#if (SYS_CPNT_SYS_MGR == TRUE)
    SYS_MGR_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);
#endif

    return;
} /* End of SYSMGMT_INIT_HandleHotInsertion */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - SYSMGMT_INIT_HandleHotRemoval
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
void SYSMGMT_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
#if (SYS_CPNT_SYS_MGR == TRUE)
    SYS_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);
#endif

    return;
} /* End of SYSMGMT_INIT_HandleHotRemoval */
