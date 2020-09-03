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

#ifndef _ADD_INIT_H_
#define _ADD_INIT_H_



/*
 *  INCLUDE STRUCTURES
 */
#include "sys_type.h"



/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
 /*--------------------------------------------------------------------------
 * ROUTINE NAME - ADD_INIT_Initiate_System_Resources
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
void ADD_INIT_Initiate_System_Resources(void);


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
void ADD_INIT_CreateTask(void);

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
void ADD_INIT_EnterMasterMode(void);


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
void ADD_INIT_EnterSlaveMode(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_INIT_EnterTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Sending enter transition event to task calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void ADD_INIT_EnterTransitionMode(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_INIT_SetTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Sending enter transition event to task calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void ADD_INIT_SetTransitionMode(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_INIT_ProvisionComplete
 * ---------------------------------------------------------------------
 * PURPOSE: Sending provision complete event to task calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void ADD_INIT_ProvisionComplete();

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
void ADD_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

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
void ADD_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

#endif /* end _ADD_INIT_H_ */
