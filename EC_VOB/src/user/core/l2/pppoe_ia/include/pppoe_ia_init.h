/* MODULE NAME: pppoe_ia_init.h
 * PURPOSE:
 *   Declarations of initialization APIs for PPPOE Intermediate Agent.
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   03/17/09    -- Squid Ro, Create
 *
 * Copyright(C)      Accton Corporation, 2009
 */
#ifndef _PPPOE_IA_INIT_H_
#define _PPPOE_IA_INIT_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
void PPPOE_IA_INIT_Create_Tasks(void);

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
void PPPOE_IA_INIT_InitiateSystemResources(void);

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
void PPPOE_IA_INIT_Create_InterCSC_Relation(void);

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
void PPPOE_IA_INIT_EnterMasterMode(void);

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
void PPPOE_IA_INIT_EnterSlaveMode(void);

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
void PPPOE_IA_INIT_EnterTransitionMode(void);

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
    UI32_T starting_port_ifindex,
    UI32_T number_of_port,
    BOOL_T use_default);

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
void PPPOE_IA_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_INIT_Initiate_System_Resources
 *-------------------------------------------------------------------------
 * PURPOSE: Initiate system resource for PPPOE IA.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * Note   : None
 *-------------------------------------------------------------------------
 */
void PPPOE_IA_INIT_Initiate_System_Resources(void);

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
void PPPOE_IA_INIT_ProvisionComplete(void);

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
void PPPOE_IA_INIT_SetTransitionMode(void);

#endif /* End of _PPPOE_IA_INIT_H */

