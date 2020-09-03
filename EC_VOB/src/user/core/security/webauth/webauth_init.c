/* MODULE NAME: WEBAUTH_INIT.C
 * PURPOSE: Definitions for the WEBAUTH INIT
 * NOTES:
 *
 *
 * HISTORY:
 *    02/05/2007 --  Rich Lee, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 */


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_adpt.h"
#include "webauth_type.h"
#include "webauth_mgr.h"
#include "webauth_task.h"
#include "webauth_om.h"
#include "webauth_init.h"
#include "webauth_backdoor.h"

#if (WEBAUTH_SUPPORT_ACCTON_BACKDOOR == TRUE)
#include "backdoor_mgr.h"
#endif /* WEBAUTH_SUPPORT_ACCTON_BACKDOOR == TRUE */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DEFINITIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: WEBAUTH_INIT_Initiate_System_Resources
 * PURPOSE: This function will initialize kernel resources
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTE:    Invoked by root.c()
 */
void WEBAUTH_INIT_Initiate_System_Resources (void)
{
    /*WEBAUTH MGR - Initiate System Resources*/
    WEBAUTH_MGR_Initiate_System_Resources();

    /*WEBAUTH TASK - Initiate System Resources;*/
    WEBAUTH_TASK_Initiate_System_Resources();
#if (WEBAUTH_SUPPORT_ACCTON_BACKDOOR == TRUE)
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("webauth", SYS_BLD_WEB_GROUP_IPCMSGQ_KEY, WEBAUTH_BACKDOOR_Main);
#endif
} /* End of WEBAUTH_INIT_Initiate_System_Resources */

/* FUNCTION NAME: WEBAUTH_INIT_CreateTasks
 * PURPOSE: This function creates all the task of this subsystem.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTE: 1. This function shall not be invoked before WEBAUTH_INIT_Init()
 *          is performed.
 */
void WEBAUTH_INIT_CreateTasks(void)
{
    /* Create console task since this fuction can only be called by Root */
    WEBAUTH_TASK_Create_Task ();

} /* End of WEBAUTH_INIT_CreateTask() */

/* FUNCTION NAME: WEBAUTH_INIT_EnterMasterMode
 * PURPOSE: This function will enable address monitor services
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTE:    None
 */
void WEBAUTH_INIT_EnterMasterMode(void)
{
    WEBAUTH_MGR_EnterMasterMode();
    WEBAUTH_TASK_EnterMasterMode();
} /*End of WEBAUTH_INIT_EnterMasterMode */

/* FUNCTION NAME: WEBAUTH_INIT_EnterSlaveMode
 * PURPOSE: Disable the WEBAUTH operation while in slave mode.
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:   none
 */
void WEBAUTH_INIT_EnterSlaveMode(void)
{
    WEBAUTH_MGR_EnterSlaveMode ();
    WEBAUTH_TASK_EnterSlaveMode();
} /*End of WEBAUTH_INIT_EnterSlaveMode */

/* FUNCTION NAME : WEBAUTH_INIT_EnterTransitionMode
 * PURPOSE: Set transition mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTE:    None
 */
void WEBAUTH_INIT_EnterTransitionMode(void)
{
    WEBAUTH_MGR_EnterTransitionMode ();
    WEBAUTH_TASK_EnterTransitionMode();
} /* End of  WEBAUTH_INIT_EnterTransitionMode */

/* FUNCTION NAME : WEBAUTH_INIT_SetTransitionMode
 * PURPOSE: Set transition mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 */
void WEBAUTH_INIT_SetTransitionMode(void)
{
    WEBAUTH_MGR_SetTransitionMode ();
    WEBAUTH_TASK_SetTransitionMode ();
} /* End of WEBAUTH_INIT_SetTransitionMode */

/* FUNCTION NAME:  WEBAUTH_INIT_ProvisionComplete
 * PURPOSE: This function will tell the WEBAUTH module to start.
 * INPUT:   none.
 * OUTPUT:  none.
 * RETURN:  none.
 * NOTES:   This function is invoked in STKCTRL_TASK_ProvisionComplete().
 *          This function shall call WEBAUTH_TASK_ProvisionComplete().
 *          If it is necessary this function will call WEBAUTH_MGR_ProvisionComplete().
 */
void WEBAUTH_INIT_ProvisionComplete(void)
{
    WEBAUTH_MGR_ProvisionComplete(TRUE);
    WEBAUTH_TASK_ProvisionComplete();

} /* End of WEBAUTH_INIT_ProvisionComplete */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - WEBAUTH_INIT_HandleHotInsertion
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
void WEBAUTH_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    WEBAUTH_MGR_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - WEBAUTH_INIT_HandleHotRemoval
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
void WEBAUTH_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    WEBAUTH_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);
}


