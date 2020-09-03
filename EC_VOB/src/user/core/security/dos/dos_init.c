/* ------------------------------------------------------------------------
 * FILE NAME - DOS_INIT.C
 * ------------------------------------------------------------------------
 * ABSTRACT :
 * Purpose:
 * Note:
 * ------------------------------------------------------------------------
 *  History
 *
 *   Wakka             24/01/2011      new created
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2011
 * ------------------------------------------------------------------------
 */
/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_DOS == TRUE)
#include "dos_type.h"
#include "dos_init.h"
#include "dos_backdoor.h"
#include "dos_mgr.h"
#include "dos_om.h"

/* NAMING CONSTANT DECLARARTIONS
 */

/* TYPE DEFINITIONS
 */

/* MACRO DEFINITIONS
 */

/* LOCAL FUNCTIONS DECLARATIONS
 */

/* LOCAL VARIABLES DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*--------------------------------------------------------------------------
 * FUNCTION NAME - DOS_INIT_InitiateSystemResources
 *--------------------------------------------------------------------------
 * PURPOSE: This function will initialize system resouce.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void DOS_INIT_InitiateSystemResources(void)
{
    DOS_MGR_InitiateSystemResources();
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - DOS_INIT_Create_InterCSC_Relation
 *------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 *------------------------------------------------------------------------*/
void DOS_INIT_Create_InterCSC_Relation(void)
{
    DOS_MGR_Create_InterCSC_Relation();
    DOS_BACKDOOR_Create_InterCSC_Relation();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DOS_INIT_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: Set component mode to Transition.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 *--------------------------------------------------------------------------
 */
void DOS_INIT_SetTransitionMode(void)
{
    DOS_MGR_SetTransitionMode();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DOS_INIT_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: Enter transition mode.
 *          mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void DOS_INIT_EnterTransitionMode(void)
{
    DOS_MGR_EnterTransitionMode();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DOS_INIT_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE: Enter master mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void DOS_INIT_EnterMasterMode(void)
{
    DOS_MGR_EnterMasterMode();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DOS_INIT_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE: Enter slave mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void DOS_INIT_EnterSlaveMode(void)
{
    DOS_MGR_EnterSlaveMode();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DOS_INIT_ProvisionComplete
 *--------------------------------------------------------------------------
 * PURPOSE: To tell the task provision is completed.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void DOS_INIT_ProvisionComplete(void)
{
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DOS_INIT_HandleHotInsertion
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
void DOS_INIT_HandleHotInsertion(
    UI32_T in_starting_port_ifindex,
    UI32_T in_number_of_port,
    BOOL_T in_use_default)
{
    DOS_MGR_HandleHotInsertion(in_starting_port_ifindex, in_number_of_port, in_use_default);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DOS_INIT_HandleHotRemoval
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
void DOS_INIT_HandleHotRemoval(
    UI32_T in_starting_port_ifindex,
    UI32_T in_number_of_port)
{
    DOS_MGR_HandleHotRemoval(in_starting_port_ifindex, in_number_of_port);
}

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

#endif /* (SYS_CPNT_DOS == TRUE) */

