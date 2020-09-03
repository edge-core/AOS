/* Project Name: Mercury
 * Module Name : HTTP_INIT.C
 * Abstract    : to be included in root.c and to access HTTP
 * Purpose     : HTTP initiation and HTTP task creation
 *
 * History :
 *          Date        Modifier        Reason
 *
 * Copyright(C)      Accton Corporation, 1999, 2000
 *
 * Note    : Inherit from Foxfire Switch product familiy designed by Orlando
 */


/*------------------------------------------------------------------------
 * INCLUDE STRUCTURES
 *------------------------------------------------------------------------*/
#include "sys_cpnt.h"
#include "sys_type.h"

#include "http_loc.h"
#include "http_task.h"
#include "http_mgr.h"
#include "http_init.h"
#include "cgi_auth.h"


/* EXPORTED SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - HTTP_INIT_InitiateProcessResource
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will create om sema id from call om init function
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : This init function will init from cli_proc.c (for http in cli process)
 *--------------------------------------------------------------------------*/
BOOL_T HTTP_INIT_InitiateProcessResource(void)
{
    /* create OM sema id
     */
    HTTP_MGR_InitiateSystemResources();
    HTTP_TASK_Init();
    return TRUE;
}

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
void HTTP_INIT_InitiateSystemResources(void)
{
    HTTP_MGR_InitiateSystemResources();
    HTTP_TASK_Init();
    return;

}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - HTTP_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void HTTP_INIT_Create_InterCSC_Relation(void)
{
    HTTP_MGR_Create_InterCSC_Relation();
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - HTTP_INIT_Create_Tasks
 *---------------------------------------------------------------------------
 * PURPOSE: This function creates all the task of this subsystem.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE: 1. This function shall not be invoked before HTTP_INIT_Init() is performed.
 *       2. All the telnet session task will be dynamically created by the incoming
 *          telent request.
 *---------------------------------------------------------------------------
 */
void HTTP_INIT_Create_Tasks(void)
{
    /* Create console task since this fuction can only be called by Root */
    HTTP_TASK_Create_Task ();

}

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
 * NOTE: 1. If "System Boot Configruation File" does not exist, the system database and
 *          switch will be initiated to the factory default value.
 *       2. HTTP will handle network requests only when this subsystem
 *          is in the Master Operation mode
 *-------------------------------------------------------------------------
 */
void HTTP_INIT_EnterMasterMode(void)
{
    /* init database */
    /* Isiah. 2002-04-11 */
    HTTP_MGR_Enter_Master_Mode();

    //spk 2002.01.26
	CGI_AUTH_InitTempRadiusUserBase();
}


/*-------------------------------------------------------------------------
 * ROUTINE NAME - HTTP_INIT_EnterSlaveMode
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Slave Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: In Slave Operation mode, any network requests
 *       will be ignored.
 *-------------------------------------------------------------------------
 */

void HTTP_INIT_EnterSlaveMode(void)
{
    HTTP_MGR_Enter_Slave_Mode();
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME - HTTP_INIT_EnterTransitionMode
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Slave Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: In Slave Operation mode, any network requests
 *       will be ignored.
 *-------------------------------------------------------------------------
 */
void HTTP_INIT_EnterTransitionMode(void)
{
    /* Isiah 2002-04-15 */
    HTTP_MGR_Enter_Transition_Mode();
    HTTP_TASK_EnterTransitionMode();
}

/* FUNCTION NAME : HTTP_INIT_SetTransitionMode
 * PURPOSE:
 *      This call will set http_mgr into transition mode to prevent
 *      calling request.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void HTTP_INIT_SetTransitionMode(void)
{
    HTTP_MGR_SetTransitionMode();
    HTTP_TASK_SetTransitionMode();
}

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
void HTTP_INIT_ProvisionComplete(void)
{
    HTTP_TASK_ProvisionComplete();
}

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
void HTTP_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    HTTP_MGR_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);
    return;
}

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
void HTTP_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    HTTP_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);
    return;
}
