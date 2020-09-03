/* static char SccsId[] = "+-<>?!SNTP_INIT.C   22.1  05/08/02  11:00:00";  
 * ------------------------------------------------------------------------
 *  FILE NAME  -  _SNTP_INIT.C				           					            
 * ------------------------------------------------------------------------
 *  ABSTRACT:   
 *	
 *  Modification History:
 *  Modifier           Date        Description
 *  -----------------------------------------------------------------------
 *  S.K.Yang		  05-08-2002  Created										   
 * ------------------------------------------------------------------------
 *  Copyright(C)				Accton Corporation, 1999				   
 * ------------------------------------------------------------------------
 */


/* INCLUDE FILE	DECLARATIONS
 */
#include "sys_type.h"

#include "sntp_mgr.h"
#include "sntp_task.h"
#include "sntp_txrx.h"
#include "sntp_dbg.h"

/* NAME	CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA	TYPE DECLARATIONS
 */

/* EXPORTED	SUBPROGRAM SPECIFICATIONS
 */

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_INIT_Init
 *---------------------------------------------------------------------------
 * PURPOSE: This function initaites the system resources, such as queue, semaphore,
 *          and events used by this subsystem. All the call back functions shall be
 *          registered during subsystem initiation.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE: 1. This function must be invoked before any tasks in this subsystem can be created.
 *       2. This function must be invoked before any services in this subsystem can be executed.
 *       3. This function initialize mapping tables to their default values
 *---------------------------------------------------------------------------
 */
void SNTP_INIT_Initiate_System_Resources(void)
{
    SNTP_TXRX_Init();
    SNTP_MGR_Init();
    SNTP_TASK_Init();
    SNTP_DBG_Init();

    return;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void SNTP_INIT_Create_InterCSC_Relation(void)
{
    SNTP_DBG_Create_InterCSC_Relation();
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_INIT_Create_Task()
 *---------------------------------------------------------------------------
 * PURPOSE: This function creates all the task of this subsystem.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE: 1. This function shall not be invoked before SNTP_INIT_Init() is performed
 *       2. SNTP is a totally passive module, that is there is no task will be created
 *---------------------------------------------------------------------------
 */
void SNTP_INIT_Create_Tasks(void)
{
    SNTP_TASK_CreateTask();
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_INIT_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE: This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the COS subsystem will enter the
 *          Master Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: 1. All the other subsystems must enter the master mode before this function
 *          can be invoked.
 *       2. If "System Boot Configruation File" does not exist, the database and
 *          switch will be initiated to the factory default value.
 *-------------------------------------------------------------------------
 */
void SNTP_INIT_EnterMasterMode(void)
{
    SNTP_MGR_EnterMasterMode();
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_INIT_EnterTransitionMode
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Transition Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: 1. Reallocate all resource for this subsystem
 *       2. Will be called when operation mode change between Master and Slave mode
 *-------------------------------------------------------------------------
 */
void SNTP_INIT_EnterTransitionMode(void)
{
    SNTP_MGR_EnterTransitionMode();
    SNTP_TASK_EnterTransitionMode();
}

 /*-------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_INIT_SetTransitionMode
 *-------------------------------------------------------------------------
 * Purpose: TThis call will set sntp_mgr into transition mode to prevent
 *			calling request.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: 
 *-------------------------------------------------------------------------
 */
void SNTP_INIT_SetTransitionMode(void)
{
    SNTP_MGR_SetTransitionMode();
    SNTP_TASK_SetTransitionMode();
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_INIT_Enter_Slave_Mode
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Slave Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: In Slave Operation mode, any requests issued from applications will be ignored.
 *-------------------------------------------------------------------------
 */
void SNTP_INIT_EnterSlaveMode(void)
{
    SNTP_MGR_EnterSlaveMode();
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_INIT_ProvisionComplete
 *-------------------------------------------------------------------------
 * Purpose: This function will tell the SSHD module to start.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE:    This function is invoked in STKCTRL_TASK_ProvisionComplete().
 *          This function shall call SNTP_TASK_ProvisionComplete().
 *          If it is necessary this function will call SNTP_MGR_ProvisionComplete().
 *-------------------------------------------------------------------------
 */
void SNTP_INIT_ProvisionComplete(void)
{
    SNTP_TASK_ProvisionComplete();
}

/* FUNCTION NAME - SNTP_INIT_HandleHotInsertion
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
void SNTP_INIT_HandleHotInsertion(UI32_T  starting_port_ifindex,
                                  UI32_T  number_of_port,
                                  BOOL_T  use_default)
{
    SNTP_MGR_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);
    return;
}



/* FUNCTION NAME - SNTP_INIT_HandleHotRemoval
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
void SNTP_INIT_HandleHotRemoval(UI32_T  starting_port_ifindex,
                                UI32_T  number_of_port)
{
    SNTP_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);
    return;
}

