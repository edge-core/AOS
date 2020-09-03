/* Project Name: New Feature
 * Module Name : tacacs_init.C
 * Abstract    : to be included in root.c to Initialize TACACS agent.
 * Purpose     : TACACS initiation 
 *
 * 2002/10/04    : Kevin Cheng     Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (Mercury 2.0)
 */
#include "tacacs_init.h"
#include "tacacs_mgr.h"
#include "tacacs_task.h"
/* INCLUDE FILE DECLARATIONS
 */

/* GLOBAL VARIABLES DECLARATION
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_INIT_Init
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
void TACACS_INIT_Initiate_System_Resources(void)
{
     TACACS_MGR_Initiate_System_Resources(); /* initialize TACACS global variables.*/
     return;
} /* end of TACACS_INIT_Initiate_System_Resources() */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_INIT_Create_InterCSC_Relation
 *---------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTE:    None.
 *---------------------------------------------------------------------------
 */
void TACACS_INIT_Create_InterCSC_Relation(void)
{
    TACACS_MGR_Create_InterCSC_Relation();
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_INIT_Create_Task()
 *---------------------------------------------------------------------------
 * PURPOSE: This function creates all the task of this subsystem.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE: This function shall not be invoked before TACACS_INIT_Initiate_System_
 *       Resources() is performed.
 *---------------------------------------------------------------------------
 */
void TACACS_INIT_Create_Tasks(void)
{
#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
    TACACS_TASK_CreateTACACSTask();
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */
} /* End of TACACS_INIT_Create_Task() */

/*-------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE: This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the TACACS subsystem will enter the
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
void TACACS_INIT_EnterMasterMode(void)
{
     TACACS_MGR_EnterMasterMode();
     return;
} /* End of TACACS_INIT_EnterMasterMode() */


/*-------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_INIT_EnterTransitionMode
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Transition Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: In Slave Operation mode, any requests issued from applications will be ignored.
 *-------------------------------------------------------------------------
 */
void TACACS_INIT_EnterTransitionMode(void)
{
     TACACS_MGR_EnterTransitionMode();
     return;
} /* End of TACACS_INIT_EnterTransitionMode() */

/*-------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_INIT_EnterSlaveMode
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Slave Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: In Slave Operation mode, any requests issued from applications will be ignored.
 *-------------------------------------------------------------------------
 */
void TACACS_INIT_EnterSlaveMode(void)
{
	TACACS_MGR_EnterSlaveMode();
        return; 
} /* end TACACS_INIT_Enter_Slave_Mode() */

/*-------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_INIT_SetTransitionMode
 *-------------------------------------------------------------------------
 * Purpose: This call will set dhcp_mgr into transition mode to prevent
 *	    calling request.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE:   : None
 *-------------------------------------------------------------------------
 */
void TACACS_INIT_SetTransitionMode(void)
{
	TACACS_MGR_SetTransitionMode();
        return;
}



/* End of TACACS_INIT.C */
