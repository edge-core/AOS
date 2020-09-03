/* -------------------------------------------------------------------------+
 * FILE NAME - telnet_init.h												    +
 * -------------------------------------------------------------------------+
 * ABSTRACT: define the root task for TELNET									+
 * -------------------------------------------------------------------------+
 *		 Project :		 Poppadom                                           +
 *       Written by:     spchen                                             +
 *       Date:           03/02/99                                           +
 *                                                                          +
 * Note: None.																+
 *		1. this module is modified from tn_root.h, MC2 version. For Mercury,
 *		   we use xxx_Init to init CSC.
 *																			+
 * Modification History:                                                    +
 *   By              Date     Ver.   Modification Description               +
 *   --------------- -------- -----  ---------------------------------------+
 *		Jason Chen	 09-28-2000       Add two action APIs to enable/disable
 *									  telnet function.
 *		William Chiang, 2001.11.18	  Change module name, Telnet_Init.
 * -------------------------------------------------------------------------+
 * Copyright(C)                              ACCTON Technology Corp., 1998  +
 * ------------------------------------------------------------------------*/
#ifndef _TELNET_INIT_H
#define _TELNET_INIT_H

/* ---------------------------------------------------|
 *                  EXTERN VARIABLES                  |
 * ---------------------------------------------------*/

/* ---------------------------------------------------|
 *                  EXTERN ROUTINES			          |
 * ---------------------------------------------------*/
/*	ROUTINE NAME : TELNET_INIT_Initiate_System_Resource
 *	FUNCTION	 : Allocate memory, message Q, semaphore used in Telnet related task.
 *	INPUT		 : None.
 *	OUTPUT		 : None.
 *	RETURN		 : None.
 *	NOTES		 : 1. Currently, this function is a null function.
 */ 
void	TELNET_INIT_Initiate_System_Resources (void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - TELNET_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void TELNET_INIT_Create_InterCSC_Relation(void);

/*	ROUTINE NAME : TELNET_INIT_Create_Tasks
 *	FUNCTION	 : Create Telnet Daemon and Telnet Shell two tasks.
 *	INPUT		 : None.
 *	OUTPUT		 : None.
 *	RETURN		 : None.
 *	NOTES		 : None.
 */ 
void	TELNET_INIT_Create_Tasks (void);


/* FUNCTION	NAME : TELNET_INIT_Set_Transition_Mode
 * PURPOSE:
 *		Set Transition Mode flag to prevent future request.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *
 * NOTES:
 *		1. Reset all modules, release all resource and reallocate.
 */
void	TELNET_INIT_SetTransitionMode (void);


/* FUNCTION	NAME : TELNET_INIT_Enter_Transition_Mode
 * PURPOSE:
 *		Enter Transition Mode.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *
 * NOTES:
 *		1. Reset all modules, release all resource and reallocate.
 */
void    TELNET_INIT_EnterTransitionMode (void);

void    TELNET_INIT_ProvisionComplete(void);

/* FUNCTION	NAME : TELNET_INIT_Enter_Master_Mode
 * PURPOSE:
 *		Enter Master Mode.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		1. Do nothing.
 */
void	TELNET_INIT_EnterMasterMode (void);


/* FUNCTION	NAME : TELNET_INIT_Enter_Slave_Mode
 * PURPOSE:
 *		Enter Slave	Mode.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *
 * NOTES:
 *		1. Do nothing.
 */
void	TELNET_INIT_EnterSlaveMode (void);



/* FUNCTION NAME - TELNET_INIT_HandleHotInsertion
 *
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is inserted at a time.
 */
void TELNET_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);



/* FUNCTION NAME - TELNET_INIT_HandleHotRemoval
 *
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is removed at a time.
 */
void TELNET_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

BOOL_T TELNET_INIT_InitiateProcessResource(void);




#endif	/*	end of _TELNET_INIT_H	*/
