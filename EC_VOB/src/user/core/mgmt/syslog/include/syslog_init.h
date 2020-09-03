/* Module Name: SYSLOG_INIT.H
 * Purpose: This file contains the information of system log module:
 *          1. Initialize resource.
 *
 * Notes:   None.
 * History:
 *    10/17/01       -- Aaron Chuang, Create
 *
 * Copyright(C)      Accton Corporation, 1999 - 2001
 *
 */
 
 
#ifndef SYSLOG_INIT_H
#define SYSLOG_INIT_H


/* INCLUDE FILE DECLARATIONS
 */


/* NAME CONSTANT DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */ 


/* FUNCTION NAME: SYSLOG_INIT_InitiateSystemResources
 * PURPOSE: This function is used to initialize the system log module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   Initialize the system log module.
 *
 */
void SYSLOG_INIT_InitiateSystemResources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SYSLOG_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void SYSLOG_INIT_Create_InterCSC_Relation(void);

/* FUNCTION NAME: SYSLOG_INIT_Create_Tasks
 * PURPOSE: This function is used to creat the system log task.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   Create the system log task.
 *
 */
void SYSLOG_INIT_Create_Tasks(void);


/* FUNCTION NAME: SYSLOG_INIT_EnterTransitionMode
 * PURPOSE: The function enter transition mode and free all SYSLOG resources 
 *          and reset database to factory default.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   1. The function MUST be called before calling the 
 *             SYSLOG_INIT_EnterMasterMode.
 */
BOOL_T SYSLOG_INIT_EnterTransitionMode (void);


/* FUNCTION NAME: SYSLOG_INIT_EnterMasterMode
 * PURPOSE: The function enter master mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 */
BOOL_T SYSLOG_INIT_EnterMasterMode(void);


/* FUNCTION NAME: SYSLOG_INIT_EnterSlaveMode
 * PURPOSE: The function enter slave mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 */
BOOL_T SYSLOG_INIT_EnterSlaveMode(void);


/* FUNCTION NAME: SYSLOG_INIT_SetTransitionMode
 * PURPOSE: The function set transition mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 */
BOOL_T SYSLOG_INIT_SetTransitionMode (void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - SYSLOG_INIT_HandleHotInsertion
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
void SYSLOG_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - SYSLOG_INIT_HandleHotRemoval
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
void SYSLOG_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);


/* MACRO FUNCTION DECLARATIONS
 */


#endif /* SYSLOG_INIT_H */
