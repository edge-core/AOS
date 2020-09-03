/* MODULE NAME: pppoe_ia_init.c
 * PURPOSE:
 *   Definitions of initialization APIs for PPPOE Intermediate Agent.
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   03/17/09    -- Squid Ro, Create
 *   11/26/09    -- Squid Ro, Modify for Linux platform
 *
 * Copyright(C)      Accton Corporation, 2009
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include "pppoe_ia_mgr.h"
#include "pppoe_ia_init.h"
#include "pppoe_ia_type.h"

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
 * ROUTINE NAME - PPPOE_IA_INIT_Create_Tasks
 *---------------------------------------------------------------------------
 * PURPOSE: This function creates all the task of this subsystem.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 *---------------------------------------------------------------------------
 */
void PPPOE_IA_INIT_Create_Tasks(void)
{
}/* End of PPPOE_IA_INIT_Create_Tasks */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - PPPOE_IA_INIT_EnterMasterMode
 * -------------------------------------------------------------------------
 * PURPOSE: This function will configurate the Control module to
 *          enter master mode after stacking
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * -------------------------------------------------------------------------
 */
void PPPOE_IA_INIT_EnterMasterMode(void)
{
    PPPOE_IA_MGR_EnterMasterMode();
}/* End of PPPOE_IA_INIT_EnterMasterMode */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - PPPOE_IA_INIT_EnterSlaveMode
 * -------------------------------------------------------------------------
 * PURPOSE: This function will disable the Control services and
 *          enter slave mode after stacking
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * -------------------------------------------------------------------------
 */
void PPPOE_IA_INIT_EnterSlaveMode(void)
{
    PPPOE_IA_MGR_EnterSlaveMode();
}/* End of PPPOE_IA_INIT_EnterSlaveMode */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - PPPOE_IA_INIT_EnterTransitionMode
 * -------------------------------------------------------------------------
 * PURPOSE: This function will initialize the Control module and
 *          free all resource to enter transition mode while stacking
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * -------------------------------------------------------------------------
 */
void PPPOE_IA_INIT_EnterTransitionMode(void)
{
    PPPOE_IA_MGR_EnterTransitionMode();
}/* End of PPPOE_IA_INIT_EnterTransitionMode */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_INIT_HandleHotInsertion
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
void PPPOE_IA_INIT_HandleHotInsertion(
    UI32_T  starting_port_ifindex,
    UI32_T  number_of_port,
    BOOL_T  use_default)
{
    PPPOE_IA_MGR_HandleHotInsertion(starting_port_ifindex,
            number_of_port, use_default);
} /* End of PPPOE_IA_INIT_HandleHotInsertion */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_INIT_HandleHotRemoval
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
void PPPOE_IA_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    PPPOE_IA_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);
} /* End of PPPOE_IA_INIT_HandleHotRemoval */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_INIT_InitiateSystemResources
 *-------------------------------------------------------------------------
 * PURPOSE: Initiate system resource for PPPOE IA.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * Note   : None
 *-------------------------------------------------------------------------
 */
void PPPOE_IA_INIT_InitiateSystemResources(void)
{
    PPPOE_IA_MGR_InitiateSystemResources();
}/* End of PPPOE_IA_INIT_Initiate_System_Resources */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_INIT_Create_InterCSC_Relation
 *-------------------------------------------------------------------------
 * PURPOSE : This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void PPPOE_IA_INIT_Create_InterCSC_Relation(void)
{
    PPPOE_IA_MGR_Create_InterCSC_Relation();
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_INIT_ProvisionComplete
 * ------------------------------------------------------------------------
 * PURPOSE: To tell the task provision is completed.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * ------------------------------------------------------------------------
 */
void PPPOE_IA_INIT_ProvisionComplete(void)
{
}/* End of PPPOE_IA_INIT_ProvisionComplete */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - PPPOE_IA_INIT_SetTransitionMode
 * -------------------------------------------------------------------------
 * PURPOSE: This function set transition mode flag
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 * -------------------------------------------------------------------------
 */
void PPPOE_IA_INIT_SetTransitionMode(void)
{
    PPPOE_IA_MGR_SetTransitionMode();
}/* End of PPPOE_IA_INIT_SetTransitionMode */


/* LOCAL SUBPROGRAM BODIES
 */

