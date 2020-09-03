/* MODULE NAME: ts_init.c
 * PURPOSE:
 *   Definitions of initialization APIs for ETS
 *   (IEEE Std 802.1Qaz - Enhanced Transmission Selection).
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   11/23/2012 - Roy Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2012
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include "ets_type.h"

#if (SYS_CPNT_ETS == TRUE)

#include "ets_init.h"
#include "ets_mgr.h"

#if (ETS_TYPE_BUILD_LINUX == FALSE)
#include "ets_task.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTIONS DECLARACTION
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
 * ROUTINE NAME - ETS_INIT_Create_Tasks
 *---------------------------------------------------------------------------
 * PURPOSE: This function creates all the task of this subsystem.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 *---------------------------------------------------------------------------
 */
void ETS_INIT_Create_Tasks(void)
{
#if (ETS_TYPE_BUILD_LINUX == FALSE)
    ETS_TASK_CreateTask();
#endif
}/* End of ETS_INIT_Create_Tasks */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_INIT_EnterMasterMode
 * -------------------------------------------------------------------------
 * PURPOSE: This function will configurate the Control module to
 *          enter master mode after stacking.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * -------------------------------------------------------------------------
 */
void ETS_INIT_EnterMasterMode(void)
{
    ETS_MGR_EnterMasterMode();

#if (ETS_TYPE_BUILD_LINUX == FALSE)
    ETS_TASK_EnterMasterMode();
#endif
}/* End of ETS_INIT_EnterMasterMode */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_INIT_EnterSlaveMode
 * -------------------------------------------------------------------------
 * PURPOSE: This function will disable the Control services and
 *          enter slave mode after stacking.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * -------------------------------------------------------------------------
 */
void ETS_INIT_EnterSlaveMode(void)
{
    ETS_MGR_EnterSlaveMode();

#if (ETS_TYPE_BUILD_LINUX == FALSE)
    ETS_TASK_EnterSlaveMode();
#endif
}/* End of ETS_INIT_EnterSlaveMode */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_INIT_EnterTransitionMode
 * -------------------------------------------------------------------------
 * PURPOSE: This function will initialize the Control module and
 *          free all resource to enter transition mode while stacking.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * -------------------------------------------------------------------------
 */
void ETS_INIT_EnterTransitionMode(void)
{
    ETS_MGR_EnterTransitionMode();

#if (ETS_TYPE_BUILD_LINUX == FALSE)
    ETS_TASK_EnterTransitionMode();
#endif
}/* End of ETS_INIT_EnterTransitionMode */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - ETS_INIT_HandleHotInsertion
 * ------------------------------------------------------------------------
 * PURPOSE: This function will initialize the port_p OM of the module ports
 *          when the option module is inserted.
 * INPUT  : starting_port_ifindex -- the ifindex of the first module port_p
 *                                   inserted
 *          number_of_port        -- the number of ports on the inserted
 *                                   module
 *          use_default           -- the flag indicating the default
 *                                   configuration is used without further
 *                                   provision applied; TRUE if a new module
 *                                   different from the original one is
 *                                   inserted
 * OUTPUT : None
 * RETURN : None
 * NOTE   : Only one module is inserted at a time.
 * ------------------------------------------------------------------------
 */
void ETS_INIT_HandleHotInsertion(
    UI32_T  starting_port_ifindex,
    UI32_T  number_of_port,
    BOOL_T  use_default)
{
    ETS_MGR_HandleHotInsertion(starting_port_ifindex,
            number_of_port, use_default);
} /* End of ETS_INIT_HandleHotInsertion */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - ETS_INIT_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE: This function will clear the port_p OM of the module ports when
 *          the option module is removed.
 * INPUT  : starting_port_ifindex -- the ifindex of the first module port_p
 *                                   removed
 *          number_of_port        -- the number of ports on the removed
 *                                   module
 * OUTPUT : None
 * RETURN : None
 * NOTE   : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void ETS_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    ETS_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);
} /* End of ETS_INIT_HandleHotRemoval */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - ETS_INIT_InitiateSystemResources
 *-------------------------------------------------------------------------
 * PURPOSE: Initiate system resource for ETS.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * Note   : None
 *-------------------------------------------------------------------------
 */
void ETS_INIT_InitiateSystemResources(void)
{
    ETS_MGR_InitiateSystemResources();

#if (ETS_TYPE_BUILD_LINUX == FALSE)
    ETS_TASK_Initiate_System_Resources();
#endif
}/* End of ETS_INIT_Initiate_System_Resources */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - ETS_INIT_Create_InterCSC_Relation
 *-------------------------------------------------------------------------
 * PURPOSE : This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void ETS_INIT_Create_InterCSC_Relation(void)
{
    ETS_MGR_Create_InterCSC_Relation();
}/* End of ETS_INIT_Create_InterCSC_Relation */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - ETS_INIT_ProvisionComplete
 * ------------------------------------------------------------------------
 * PURPOSE: To tell the task provision is completed.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * ------------------------------------------------------------------------
 */
void ETS_INIT_ProvisionComplete(void)
{
    ETS_MGR_ProvisionComplete();

#if (ETS_TYPE_BUILD_LINUX == FALSE)
    ETS_TASK_ProvisionComplete();
#endif
}/* End of ETS_INIT_ProvisionComplete */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - ETS_INIT_SetTransitionMode
 * -------------------------------------------------------------------------
 * PURPOSE: This function set transition mode flag.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * -------------------------------------------------------------------------
 */
void ETS_INIT_SetTransitionMode(void)
{
    ETS_MGR_SetTransitionMode();

#if (ETS_TYPE_BUILD_LINUX == FALSE)
    ETS_TASK_SetTransitionMode();
#endif

}/* End of ETS_INIT_SetTransitionMode */


/* LOCAL SUBPROGRAM BODIES
 */

#endif /* #if (SYS_CPNT_ETS == TRUE) */


