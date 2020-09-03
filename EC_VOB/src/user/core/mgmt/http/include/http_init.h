/* Project Name: Mercury
 * Module Name : HTTP_INIT.C
 * Abstract    : to be included in root.c
 * Purpose     : HTTP initiation and HTTP creation
 *
 * History :
 *          Date        Modifier        Reason
 *
 * Copyright(C)      Accton Corporation, 1999, 2000
 *
 * Note    :
 */

#ifndef _HTTP_INIT_H_
#define _HTTP_INIT_H_

/*--------------------------------------------------------------------------
 * FUNCTION NAME - HTTP_INIT_InitiateProcessResource
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will create om sema id from call om init function
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : This init function will init from cli_proc.c (for http in cli process)
 *--------------------------------------------------------------------------*/
 BOOL_T HTTP_INIT_InitiateProcessResource(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - HTTP_INIT_InitiateSystemResources
 *---------------------------------------------------------------------------
 * PURPOSE: This function initaites the system resources, such as queue, semaphore,
 *          and events used by this subsystem. All the call back functions shall be
 *          registered during subsystem initiation.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE: 1. This function must be invoked before any tasks in this subsystem can be created.
 *       2. This function must be invoked before any services in this subsystem can be executed.
 *---------------------------------------------------------------------------
 */
void HTTP_INIT_InitiateSystemResources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - HTTP_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void HTTP_INIT_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - HTTP_INIT_Create_Tasks()
 *---------------------------------------------------------------------------
 * PURPOSE: This function creates all the task of this subsystem.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE: 1. This function shall not be invoked before HTTP_INIT_Init() is performed.
 *---------------------------------------------------------------------------
 */
void HTTP_INIT_Create_Tasks(void);


/*-------------------------------------------------------------------------
 * ROUTINE NAME - HTTP_INIT_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE: This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the HTTP subsystem will enter the
 *          Master Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: 1. All the other subsystems must enter the master mode before this function
 *          can be invoked.
 *       2. If "System Boot Configruation File" does not exist, the system database and
 *          switch will be initiated to the factory default value.
 *       3. HTTP sessions will operate only when this subsystem
 *          is in the Master Operation mode
 *-------------------------------------------------------------------------
 */
void HTTP_INIT_EnterMasterMode(void);


/*-------------------------------------------------------------------------
 * ROUTINE NAME - HTTP_INIT_EnterSlaveMode
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Slave Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: In Slave Operation mode HTTP server will not work.
 *-------------------------------------------------------------------------
 */
void HTTP_INIT_EnterSlaveMode(void);


/*-------------------------------------------------------------------------
 * ROUTINE NAME - HTTP_INIT_EnterTransitionMode
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Transition Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: In Transition Operation mode HTTP server will not work.
 *-------------------------------------------------------------------------
 */
void HTTP_INIT_EnterTransitionMode(void);


/* FUNCTION	NAME : HTTP_INIT_SetTransitionMode
 * PURPOSE:
 *		This call will set http_mgr into transition mode to prevent
 *		calling request.
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
 *		None.
 */
void HTTP_INIT_SetTransitionMode(void);



/* FUNCTION NAME:  HTTP_INIT_ProvisionComplete
 * PURPOSE:
 *          This function will tell the HTTP module to start.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 *
 * NOTES:
 *          This function is invoked in STKCTRL_TASK_ProvisionComplete().
 *          This function shall call HTTP_TASK_ProvisionComplete().
 *          If it is necessary this function will call HTTP_MGR_ProvisionComplete().
 */
void HTTP_INIT_ProvisionComplete(void);



/* FUNCTION NAME - HTTP_INIT_HandleHotInsertion
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
void HTTP_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);



/* FUNCTION NAME - HTTP_INIT_HandleHotRemoval
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
void HTTP_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);


#endif /* end _HTTP_INIT_H_ */
/* End of HTTP_INIT.C */


