/* Project Name: Mercury
 * Module Name : XFER_INIT.h
 * Abstract    : Initialize the resources for the system XFER module.
 * Purpose     : XFER initiation and XFER task creation
 *
 * History :
 *          Date        		Modifier        Reason
 *          2001/10/11    BECK CHEN     Create this file
 *
 * Copyright(C)      Accton Corporation, 1999, 2000 ,2001
 *
 * Note    :
 */

#ifndef _XFER_INIT_H_
#define _XFER_INIT_H_



/*
 *  INCLUDE STRUCTURES
 */
#include "sys_type.h"



/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
 /*--------------------------------------------------------------------------
 * ROUTINE NAME - XFER_INIT_InitiateSystemResources
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
void XFER_INIT_InitiateProcessResources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - XFER_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void XFER_INIT_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - XFER_INIT_Create_Task()
 *---------------------------------------------------------------------------
 * PURPOSE: This function creates all the task of this subsystem.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:
 *---------------------------------------------------------------------------
 */
void XFER_INIT_CreateTask(void);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - XFER_INIT_DB_Init_And_Enter_Master_Mode
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
void XFER_INIT_EnterMasterMode(void);


/*-------------------------------------------------------------------------
 * ROUTINE NAME - XFER_INIT_Enter_Slave_Mode
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Slave Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE:
 *-------------------------------------------------------------------------
 */
void XFER_INIT_EnterSlaveMode(void);

void XFER_INIT_EnterTransitionMode(void);

void XFER_INIT_SetTransitionMode(void);

void XFER_INIT_ProvisionComplete();

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XFER_INIT_HandleHotInsertion
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
void XFER_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XFER_INIT_HandleHotRemoval
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
void XFER_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

#endif /* end _XFER_INIT_H_ */

