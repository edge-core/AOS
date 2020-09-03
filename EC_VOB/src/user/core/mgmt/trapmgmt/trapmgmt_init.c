/* Project Name: Mercury 
 * Module Name : TRAPMGMT_init.C
 * Abstract    : to be included in root.c and tn_main.c to access snmp agent.
 * Purpose     : Trap initiation and Trap task creation
 *
 *
 * Copyright(C)      Accton Corporation, 1999, 2000 
 *
 * Note    : Inherit from Foxfire Switch product familiy designed by Orlando
 */


 
/* INCLUDE FILE DECLARATIONS
 */
#include "trapmgmt_init.h"
#include "trap_mgr.h"
#include "sys_type.h"

/* GLOBAL VARIABLES DECLARATION
 */

 
 
/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TRAPMGMT_INIT_Init
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
void TRAPMGMT_INIT_Initiate_System_Resources(void)
{
    TRAP_MGR_Init();
 
} /* end of TRAPMGMT_INIT_Initiate_System_Resources() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - TRAPMGMT_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void TRAPMGMT_INIT_Create_InterCSC_Relation(void)
{
    TRAP_MGR_Create_InterCSC_Relation();
} /* end of TRAPMGMT_INIT_Create_InterCSC_Relation */


/*--------------------------------------------------------------------------
 * ROUTINE NAME - TRAPMGMT_INIT_Create_Task()
 *---------------------------------------------------------------------------
 * PURPOSE: This function creates all the task of this subsystem.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE: 1. This function shall not be invoked before SNMP_INIT_Initiate_System_
            Resources() is performed.
 *       2. All the telnet session task will be dynamically created by the incoming
 *          telent request.
 *---------------------------------------------------------------------------
 */
void TRAPMGMT_INIT_Create_Tasks(void)
{

    /* Create trap task since this fuction can only be called by Root */
    TRAP_MGR_CreateTask();

} /* End of TRAPMGMT_INIT_Create_Task() */



/*-------------------------------------------------------------------------
 * ROUTINE NAME - TRAPMGMT_INIT_EnterMasterMode                         
 *-------------------------------------------------------------------------
 * PURPOSE: This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the SNMP subsystem will enter the
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
void TRAPMGMT_INIT_EnterMasterMode(void)
{   
     TRAP_MGR_EnterMasterMode();

} /* End of TRAPMGMT_INIT_EnterMasterMode() */



/*-------------------------------------------------------------------------
 * ROUTINE NAME - TRAPMGMT_INIT_EnterTransitionMode									
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the transition Operation mode.										
 * INPUT   : None															
 * OUTPUT  : None															
 * RETURN  : None														
 * NOTE    : None
 *       															
 *-------------------------------------------------------------------------
 */
void TRAPMGMT_INIT_EnterTransitionMode(void)
{
     TRAP_MGR_EnterTransitionMode();
} /* end of TRAPMGMT_INIT_EnterTransitionMode() */

/*-------------------------------------------------------------------------
 * ROUTINE NAME - TRAPMGMT_INIT_SetTransitionMode									
 *-------------------------------------------------------------------------
 * Purpose: 										
 * INPUT   : None															
 * OUTPUT  : None															
 * RETURN  : None														
 * NOTE    : None
 *       															
 *-------------------------------------------------------------------------
 */
void TRAPMGMT_INIT_SetTransitionMode(void)
{
    TRAP_MGR_SetTransitionMode();
} /* end of TRAPMGMT_INIT_SetTransitionMode() */
 
/*-------------------------------------------------------------------------
 * ROUTINE NAME - TRAPMGMT_INIT_EnterSlaveMode									
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the slave Operation mode.										
 * INPUT   : None															
 * OUTPUT  : None															
 * RETURN  : None														
 * NOTE    : None
 *       															
 *-------------------------------------------------------------------------
 */
void TRAPMGMT_INIT_EnterSlaveMode(void)
{
     TRAP_MGR_EnterSlaveMode();
	
} /* end TRAPMGMT_INIT_Enter_Slave_Mode() */


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAPMGMT_INIT_ProvisionComplete									
 * ---------------------------------------------------------------------
 *  FUNCTION: Cold start trap must wait until provision complete before
 *            send request can be process properly.					
 *  INPUT	 : None.													
 *  OUTPUT	 : None.													
 *  RETURN	 : None.													
 *  NOTE	 : None.													
 * ---------------------------------------------------------------------
 */
void TRAPMGMT_INIT_ProvisionComplete(void)
{
    TRAP_MGR_ProvisionComplete();
} /* end of TRAPMGMT_INIT_ProvisionComplete() */

/*-------------------------------------------------------------------------
 * ROUTINE NAME - TRAPMGMT_INIT_HandleHotInsertion									
 *-------------------------------------------------------------------------
 * Purpose: This function is called when the module is plug in.								
 * INPUT   : None															
 * OUTPUT  : None															
 * RETURN  : None														
 * NOTE: This function do nothing here.															
 *-------------------------------------------------------------------------
 */
void TRAPMGMT_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* do nothing here*/
    return;
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME - TRAPMGMT_INIT_HandleHotRemoval									
 *-------------------------------------------------------------------------
 * Purpose: This function is called when module is plug off.									
 * INPUT   : None															
 * OUTPUT  : None															
 * RETURN  : None														
 * NOTE:     SNMP do nothing here														
 *-------------------------------------------------------------------------
 */
void TRAPMGMT_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /* do nothing here*/
    return;
}

/* End of TRAPMGMT_INIT.C */