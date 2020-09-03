/* Project Name: New Feature
 * Module Name : Radius_init.C
 * Abstract    : to be included in root.c to Initialize Radius agent.
 * Purpose     : Radius initiation and Radius task creation
 *
 * 2001/11/22    : Kevin Cheng     Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (Mercury)
 */



/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "radius_task.h"
#include "radius_mgr.h"
#include "radiusclient.h"
/* GLOBAL VARIABLES DECLARATION
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_INIT_InitiateProcessResources
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
void RADIUS_INIT_InitiateProcessResources(void)
{
    RADIUS_MGR_InitiateProcessResources();
    RADIUS_TASK_InitiateProcessResources();
} /* end of RADIUS_INIT_InitiateProcessResources() */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_INIT_Create_InterCSC_Relation
 *---------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTE:    None.
 *---------------------------------------------------------------------------
 */
void RADIUS_INIT_Create_InterCSC_Relation(void)
{
    RADIUS_MGR_Create_InterCSC_Relation();
    RADIUS_TASK_Create_InterCSC_Relation();
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_INIT_Create_Task()
 *---------------------------------------------------------------------------
 * PURPOSE: This function creates all the task of this subsystem.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE: This function shall not be invoked before RADIUS_INIT_Initiate_System_
 *       Resources() is performed.
 *---------------------------------------------------------------------------
 */
void RADIUS_INIT_Create_Tasks(void)
{

    /* Create Radius task since this fuction can only be called by Root */
     RADIUS_TASK_CreateRadiusTask();

} /* End of RADIUS_INIT_Create_Task() */



/*-------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE: This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the RADIUS subsystem will enter the
 *          Master Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: 1. All the other subsystems must enter the master mode before this function
 *          can be invoked.
 *       2. If "System Boot Configruation File" does not exist, the system database and
 *          switch will be initiated to the factory default value.
 *-------------------------------------------------------------------------
 */
void RADIUS_INIT_EnterMasterMode(void)
{
     RADIUS_MGR_EnterMasterMode();
     RADIUS_TASK_EnterMasterMode();

} /* End of RADIUS_INIT_EnterMasterMode() */


/*-------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_INIT_EnterTransitionMode
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Transition Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: In Slave Operation mode, any requests issued from applications will be ignored.
 *-------------------------------------------------------------------------
 */
void RADIUS_INIT_EnterTransitionMode(void)
{
     RADIUS_MGR_EnterTransitionMode();
     RADIUS_TASK_EnterTransitionMode();

} /* End of RADIUS_INIT_EnterTransitionMode() */

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_INIT_EnterSlaveMode
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Slave Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: In Slave Operation mode, any requests issued from applications will be ignored.
 *-------------------------------------------------------------------------
 */

void RADIUS_INIT_EnterSlaveMode(void)
{

    RADIUS_MGR_EnterSlaveMode();
    RADIUS_TASK_EnterSlaveMode();

} /* end RADIUS_INIT_Enter_Slave_Mode() */

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_INIT_SetTransitionMode
 *-------------------------------------------------------------------------
 * Purpose: This call will set dhcp_mgr into transition mode to prevent
 *          calling request.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE:   : None
 *-------------------------------------------------------------------------
 */
void RADIUS_INIT_SetTransitionMode(void)
{
    RADIUS_MGR_SetTransitionMode();
    RADIUS_TASK_SetTransitionMode();
}

/* FUNCTION NAME - RADIUS_INIT_HandleHotInsertion
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
void RADIUS_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    RADIUS_MGR_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);
}



/* FUNCTION NAME - RADIUS_INIT_HandleHotRemoval
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
void RADIUS_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    RADIUS_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);
}

/* End of RADIUS_INIT.C */
