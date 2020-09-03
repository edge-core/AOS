/*-----------------------------------------------------------------------------
 * Module Name: xstp_init.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate the system resources and create the task.
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    05/30/2001 - Allen Cheng, Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */
#ifndef XSTP_INIT_H
#define XSTP_INIT_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_INIT_Initiate_System_Resources
 *-------------------------------------------------------------------------
 * PURPOSE  : Init XSTP data
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void XSTP_INIT_Initiate_System_Resources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_INIT_Create_InterCSC_Relation
 *-------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void XSTP_INIT_Create_InterCSC_Relation(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_INIT_Create_Tasks
 *-------------------------------------------------------------------------
 * PURPOSE  : Create XSTP task
 * INPUT    : tg_handle - the handle of thread group
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void XSTP_INIT_Create_Tasks(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_INIT_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE : Enter master mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void XSTP_INIT_EnterMasterMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_INIT_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE : Enter slave mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void XSTP_INIT_EnterSlaveMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_INIT_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE : Enter transition mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void XSTP_INIT_EnterTransitionMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_INIT_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE : Set transition mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void XSTP_INIT_SetTransitionMode(void);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_INIT_HandleHotInsertion
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
void XSTP_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_INIT_HandleHotRemoval
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
void XSTP_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

#endif
