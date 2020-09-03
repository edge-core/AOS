/* MODULE NAME:  WEBAUTH_INIT.H
 * PURPOSE:  This package provides the webauth init functions .
 *
 * NOTE:
 *
 * HISTORY:
 * 02/05/2007     --  Rich Lee , Create
 *
 *
 * Copyright(C)       Accton Corporation, 2007
 */

#ifndef _WEBAUTH_INIT_H
#define _WEBAUTH_INIT_H


/* INCLUDE FILE DECLARATTIONS
 */
#include <sys_type.h>
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "webauth_type.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */


/* FUNCTION NAME: WEBAUTH_INIT_EnterMasterMode
 * PURPOSE: This function will enable address monitor services
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTE:    None
 */
void WEBAUTH_INIT_EnterMasterMode(void);

/* FUNCTION NAME : WEBAUTH_INIT_EnterTransitionMode
 * PURPOSE: Set transition mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTE:    None
 */
void WEBAUTH_INIT_EnterTransitionMode(void);

/* FUNCTION NAME: WEBAUTH_INIT_EnterSlaveMode
 * PURPOSE: Disable the WEBAUTH operation while in slave mode.
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:   none
 */
void WEBAUTH_INIT_EnterSlaveMode(void);

/* FUNCTION NAME : WEBAUTH_INIT_SetTransitionMode
 * PURPOSE: Set transition mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 */
void WEBAUTH_INIT_SetTransitionMode(void);


/* FUNCTION NAME: WEBAUTH_INIT_Initiate_System_Resources
 * PURPOSE: This function will initialize kernel resources
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTE:    Invoked by root.c()
 */
void WEBAUTH_INIT_Initiate_System_Resources (void);


/* FUNCTION NAME: WEBAUTH_INIT_CreateTasks
 * PURPOSE: This function creates all the task of this subsystem.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTE: 1. This function shall not be invoked before WEBAUTH_INIT_Init()
 *          is performed.
 */
void WEBAUTH_INIT_CreateTasks(void);

/* FUNCTION NAME:  WEBAUTH_INIT_ProvisionComplete
 * PURPOSE: This function will tell the WEBAUTH module to start.
 * INPUT:   none.
 * OUTPUT:  none.
 * RETURN:  none.
 * NOTES:   This function is invoked in STKCTRL_TASK_ProvisionComplete().
 *          This function shall call WEBAUTH_TASK_ProvisionComplete().
 *          If it is necessary this function will call WEBAUTH_MGR_ProvisionComplete().
 */
void WEBAUTH_INIT_ProvisionComplete(void);

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
void WEBAUTH_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

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
void WEBAUTH_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);


#endif  /* End of webauth_init_h */

