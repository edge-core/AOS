/*-----------------------------------------------------------------------------
 * Module Name: amtr_init.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate the system resources and create the task.
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    1/8/2004 - Water Huang, Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2004
 *-----------------------------------------------------------------------------
 */

#ifndef AMTR_INIT_H
#define AMTR_INIT_H

/* INCLUDE FILE DECLARATIONS
 */


/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_INIT_InitiateSystemResources
 * PURPOSE: init AMTR data
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *-----------------------------------------------------------------------------*/
void AMTR_INIT_InitiateSystemResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_INIT_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *-----------------------------------------------------------------------------*/
void AMTR_INIT_Create_InterCSC_Relation(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_INIT_EnterTransitionMode
 * PURPOSE: enter transition state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *-----------------------------------------------------------------------------*/
void AMTR_INIT_EnterTransitionMode(void);

/*------------------------------------------------------------------------------
 * Function : AMTR_INIT_SetTransitionMode()
 *------------------------------------------------------------------------------
 * Purpose  : This function will set the operation mode to transition mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-----------------------------------------------------------------------------*/
void AMTR_INIT_SetTransitionMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_INIT_EnterMasterMode
 * PURPOSE: enter master state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *-----------------------------------------------------------------------------*/
void AMTR_INIT_EnterMasterMode(void);


/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_INIT_EnterSlaveMode
 * PURPOSE: enter slave state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *-----------------------------------------------------------------------------*/
void AMTR_INIT_EnterSlaveMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_INIT_HandleHotInsertion
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

 * -------------------------------------------------------------------------*/
void AMTR_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);


/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTR_INIT_HandleHotRemoval
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * -------------------------------------------------------------------------*/
void AMTR_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);


#endif /* End of AMTR_INIT_H */
