/* Module Name: PSEC_INIT.H
 * Purpose:
 *        ( 1. Whole module function and scope.                 )
 *         This file provides the hardware independent interface of port security
 *         control functions to applications.
 *        ( 2.  The domain MUST be handled by this module.      )
 *         This module includes port security manipulation.
 *        ( 3.  The domain would not be handled by this module. )
 *         None.
 * Notes:
 *        ( Something must be known or noticed by developer     )
 * History:
 *       Date        Modifier    Reason
 *      2002/5/30    Arthur Wu   Create this file
 *
 *
 * Copyright(C)      Accton Corporation, 2002
 */
#ifndef PSEC_INIT_H
#define PSEC_INIT_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT
 */

/* MACRO DEFINITIONS
 */

/* TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_INIT_InitiateProcessResources
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize kernel resources
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Invoked by root.c()
 *------------------------------------------------------------------------*/
void PSEC_INIT_InitiateProcessResources (void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_INIT_Create_InterCSC_Relation
 *------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void PSEC_INIT_Create_InterCSC_Relation(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_INIT_Create_Tasks
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize kernel resources
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Invoked by root.c()
 *------------------------------------------------------------------------*/
void PSEC_INIT_Create_Tasks (void);


/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_INIT_EnterTransitionMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize all system resource
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void PSEC_INIT_EnterTransitionMode(void);


/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_INIT_EnterMasterMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will enable address monitor services
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void PSEC_INIT_EnterMasterMode(void);


/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_INIT_EnterSlaveMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will disable address monitor services
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void PSEC_INIT_EnterSlaveMode(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_INIT_SetTransitionMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will set the transition mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void PSEC_INIT_SetTransitionMode(void);



/* FUNCTION NAME - PSEC_INIT_HandleHotInsertion
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
void PSEC_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);



/* FUNCTION NAME - PSEC_INIT_HandleHotRemoval
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
void PSEC_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

#endif /* End of PSEC_INIT_H */
