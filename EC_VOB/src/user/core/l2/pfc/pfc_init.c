/* MODULE NAME: pfc_init.c
 * PURPOSE:
 *   Definitions of initialization APIs for PFC
 *   (IEEE Std 802.1Qbb, Priority-based Flow Control).
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   10/15/12    -- Squid Ro, Create
 *
 * Copyright(C)      Accton Corporation, 2012
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include "pfc_type.h"

#if (SYS_CPNT_PFC == TRUE)

#include "pfc_init.h"
#include "pfc_mgr.h"

#if (PFC_TYPE_BUILD_LINUX == FALSE)
#include "pfc_task.h"
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
 * ROUTINE NAME - PFC_INIT_Create_Tasks
 *---------------------------------------------------------------------------
 * PURPOSE: This function creates all the task of this subsystem.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 *---------------------------------------------------------------------------
 */
void PFC_INIT_Create_Tasks(void)
{
#if (PFC_TYPE_BUILD_LINUX == FALSE)
    PFC_TASK_CreateTask();
#endif
}/* End of PFC_INIT_Create_Tasks */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - PFC_INIT_EnterMasterMode
 * -------------------------------------------------------------------------
 * PURPOSE: This function will configurate the Control module to
 *          enter master mode after stacking.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * -------------------------------------------------------------------------
 */
void PFC_INIT_EnterMasterMode(void)
{
    PFC_MGR_EnterMasterMode();

#if (PFC_TYPE_BUILD_LINUX == FALSE)
    PFC_TASK_EnterMasterMode();
#endif
}/* End of PFC_INIT_EnterMasterMode */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - PFC_INIT_EnterSlaveMode
 * -------------------------------------------------------------------------
 * PURPOSE: This function will disable the Control services and
 *          enter slave mode after stacking.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * -------------------------------------------------------------------------
 */
void PFC_INIT_EnterSlaveMode(void)
{
    PFC_MGR_EnterSlaveMode();

#if (PFC_TYPE_BUILD_LINUX == FALSE)
    PFC_TASK_EnterSlaveMode();
#endif
}/* End of PFC_INIT_EnterSlaveMode */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - PFC_INIT_EnterTransitionMode
 * -------------------------------------------------------------------------
 * PURPOSE: This function will initialize the Control module and
 *          free all resource to enter transition mode while stacking.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * -------------------------------------------------------------------------
 */
void PFC_INIT_EnterTransitionMode(void)
{
    PFC_MGR_EnterTransitionMode();

#if (PFC_TYPE_BUILD_LINUX == FALSE)
    PFC_TASK_EnterTransitionMode();
#endif
}/* End of PFC_INIT_EnterTransitionMode */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PFC_INIT_HandleHotInsertion
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
void PFC_INIT_HandleHotInsertion(
    UI32_T  starting_port_ifindex,
    UI32_T  number_of_port,
    BOOL_T  use_default)
{
    PFC_MGR_HandleHotInsertion(starting_port_ifindex,
            number_of_port, use_default);
} /* End of PFC_INIT_HandleHotInsertion */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PFC_INIT_HandleHotRemoval
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
void PFC_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    PFC_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);
} /* End of PFC_INIT_HandleHotRemoval */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_INIT_InitiateSystemResources
 *-------------------------------------------------------------------------
 * PURPOSE: Initiate system resource for PFC.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * Note   : None
 *-------------------------------------------------------------------------
 */
void PFC_INIT_InitiateSystemResources(void)
{
    PFC_MGR_InitiateSystemResources();

#if (PFC_TYPE_BUILD_LINUX == FALSE)
    PFC_TASK_Initiate_System_Resources();
#endif
}/* End of PFC_INIT_Initiate_System_Resources */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_INIT_Create_InterCSC_Relation
 *-------------------------------------------------------------------------
 * PURPOSE : This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void PFC_INIT_Create_InterCSC_Relation(void)
{
    PFC_MGR_Create_InterCSC_Relation();
}/* End of PFC_INIT_Create_InterCSC_Relation */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PFC_INIT_ProvisionComplete
 * ------------------------------------------------------------------------
 * PURPOSE: To tell the task provision is completed.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * ------------------------------------------------------------------------
 */
void PFC_INIT_ProvisionComplete(void)
{
    PFC_MGR_ProvisionComplete();

#if (PFC_TYPE_BUILD_LINUX == FALSE)
    PFC_TASK_ProvisionComplete();
#endif
}/* End of PFC_INIT_ProvisionComplete */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - PFC_INIT_SetTransitionMode
 * -------------------------------------------------------------------------
 * PURPOSE: This function set transition mode flag.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * -------------------------------------------------------------------------
 */
void PFC_INIT_SetTransitionMode(void)
{
    PFC_MGR_SetTransitionMode();

#if (PFC_TYPE_BUILD_LINUX == FALSE)
    PFC_TASK_SetTransitionMode();
#endif

}/* End of PFC_INIT_SetTransitionMode */


/* LOCAL SUBPROGRAM BODIES
 */

#endif /* #if (SYS_CPNT_PFC == TRUE) */

