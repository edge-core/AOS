/* Module Name: SMTP_INIT.H
 * Purpose: This file contains the information of smtp module:
 *          1. Initialize resource.
 *
 * Notes:   None.
 * History:
 *    01/22/03       -- Ricky Lin, Create
 *
 * Copyright(C)      Accton Corporation, 1999 - 2001
 *
 */


#ifndef SMTP_INIT_H
#define SMTP_INIT_H


/* INCLUDE FILE DECLARATIONS
 */


/* NAME CONSTANT DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */


/* FUNCTION NAME: SMTP_INIT_Initiate_System_Resources
 * PURPOSE: This function is used to initialize the smtp module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   1.Initialize the smtp mgr system resource.
 *          2.Initialize the smtp task system resource.
 *
 */
void SMTP_INIT_Initiate_System_Resources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SMTP_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SMTP_INIT_Create_InterCSC_Relation(void);

/* FUNCTION NAME: SMTP_INIT_Create_Tasks
 * PURPOSE: This function is used to creat the smtp task.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void SMTP_INIT_Create_Tasks(void);


/* FUNCTION NAME: SMTP_INIT_EnterTransitionMode
 * PURPOSE: This function is used to enter transition mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void SMTP_INIT_EnterTransitionMode(void);


/* FUNCTION NAME: SMTP_INIT_EnterMasterMode
 * PURPOSE: The function is used to enter master mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   This function must be invoked first before
 *           SYSLOG_INIT_EnterMasterMode() is called.
 *
 */
void SMTP_INIT_EnterMasterMode(void);


/* FUNCTION NAME: SMTP_INIT_EnterSlaveMode
 * PURPOSE: The function is used to enter slave mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void SMTP_INIT_EnterSlaveMode(void);


/* FUNCTION NAME: SMTP_INIT_SetTransitionMode
 * PURPOSE: The function is used to set transition mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void SMTP_INIT_SetTransitionMode(void);

/* FUNCTION NAME: SMTP_INIT_ProvisionComplete
 * PURPOSE: This function will tell the SMTP module to start
 *           action
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void SMTP_INIT_ProvisionComplete(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - SMTP_INIT_HandleHotInsertion
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
void SMTP_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - SMTP_MGR_HandleHotRemoval
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
void SMTP_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

#endif /* SMTP_INIT_H */
