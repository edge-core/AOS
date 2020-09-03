/*-----------------------------------------------------------------------------
 * Module Name: LACP_init.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate the system resources and create the task.
 *-----------------------------------------------------------------------------
 * NOTES:
 *   1. This header file provides the declarations for LACP initialization.
 *   2. Called by swctrl_init.c
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    12/05/2001 - Lewis Kang, Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2001
 *-----------------------------------------------------------------------------
 */
#ifndef _LACP_INIT_H
#define _LACP_INIT_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"


/*-------------------------------------------------------------------------
 * FUNCTION NAME: LACP_INIT_InitiateSystemResources
 * PURPOSE: init LACP needed resources
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */

void LACP_INIT_InitiateSystemResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_INIT_Create_InterCSC_Relation
 *-------------------------------------------------------------------------
 * PURPOSE : This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void LACP_INIT_Create_InterCSC_Relation(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: LACP_Create_Task
 * PURPOSE: create LACP task
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
void LACP_INIT_Create_Tasks(void);

void LACP_INIT_EnterMasterMode(void);
void LACP_INIT_EnterSlaveMode(void);
void LACP_INIT_EnterTransitionMode(void);
void LACP_INIT_SetTransitionMode(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_INIT_HandleHotInsertion
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
void LACP_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_INIT_HandleHotRemoval
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
void LACP_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

#endif
