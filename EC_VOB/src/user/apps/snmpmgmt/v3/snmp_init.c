/* Project Name: Mercury 
 * Module Name : Snmp_init.C
 * Abstract    : to be included in root.c and tn_main.c to access snmp agent.
 * Purpose     : Snmp initiation and Snmp task creation
 *
 * 2001/10/16    : James Chien     Create this file
 *
 * Copyright(C)      Accton Corporation, 1999, 2000 
 *
 * Note    : Inherit from Foxfire Switch product familiy designed by Orlando
 */


 
/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "snmp_task.h"
//#include "snmpstat.h"
#include "snmp_mgr.h"
#include "snmp_init.h"
// SNMP_STATS_T snmp_stats;
 
/* LOCAL SUBPROGRAM DECLARATIONS
 */

 
/* EXPORTED SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_INIT_Init
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
void SNMP_INIT_Initiate_System_Resources(void)
{
   
    
     //SNMP_INIT_VarInit(); /* initialize snmp global variables.*/
   
    SNMP_MGR_Init();
    SNMP_TASK_Init();
} /* end of SNMP_INIT_Initiate_System_Resources() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SNMP_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void SNMP_INIT_Create_InterCSC_Relation(void)
{
    SNMP_MGR_Create_InterCSC_Relation();
} /* end of SNMP_INIT_Create_InterCSC_Relation */


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_INIT_Create_Task()
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
void SNMP_INIT_Create_Tasks(void)
{

    /* Create snmp task since this fuction can only be called by Root */
     SNMP_TASK_CreateSnmpTask();
    

} /* End of SNMP_INIT_Create_Task() */

/*-------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_EnterMasterMode                         
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
void SNMP_INIT_EnterMasterMode(void)
{
     SNMP_MGR_EnterMasterMode();
     SNMP_TASK_EnterMasterMode();
     

} /* End of SNMP_INIT_EnterMasterMode() */


/*-------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_INIT_EnterTransitionMode									
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Transition Operation mode.										
 * INPUT   : None															
 * OUTPUT  : None															
 * RETURN  : None														
 * NOTE: In Slave Operation mode, any Cisco Commands issued from local console or 
 *       Telnet will be ignored.															
 *-------------------------------------------------------------------------
 */
void SNMP_INIT_EnterTransitionMode(void)
{ 
     SNMP_MGR_EnterTransitionMode();     
     SNMP_TASK_EnterTransitionMode();
} /* End of SNMP_INIT_EnterTransitionMode() */



/*-------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_INIT_EnterSlaveMode									
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Slave Operation mode.										
 * INPUT   : None															
 * OUTPUT  : None															
 * RETURN  : None														
 * NOTE: In Slave Operation mode, any Cisco Commands issued from local console or 
 *       Telnet will be ignored.															
 *-------------------------------------------------------------------------
 */
 
void SNMP_INIT_EnterSlaveMode(void)
{
	SNMP_MGR_EnterSlaveMode();
	SNMP_TASK_EnterSlaveMode();
	
	/*
	 * release resource resources
	 */
} /* end SNMP_INIT_Enter_Slave_Mode() */


/* FUNCTION	NAME : SNMP_INIT_SetTransitionMode
 * PURPOSE:
 *		This call will set SNMP_mgr into transition mode to prevent
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
void SNMP_INIT_SetTransitionMode(void)
{
	SNMP_MGR_SetTransitionMode();
	SNMP_TASK_SetTransitionMode();
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_INIT_HandleHotInsertion									
 *-------------------------------------------------------------------------
 * Purpose: This function is called when the module is plug in.								
 * INPUT   : None															
 * OUTPUT  : None															
 * RETURN  : None														
 * NOTE: This function do nothing here.															
 *-------------------------------------------------------------------------
 */
void SNMP_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* do nothing here*/
    return;
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_INIT_HandleHotRemoval									
 *-------------------------------------------------------------------------
 * Purpose: This function is called when module is plug off.									
 * INPUT   : None															
 * OUTPUT  : None															
 * RETURN  : None														
 * NOTE:     SNMP do nothing here														
 *-------------------------------------------------------------------------
 */
void SNMP_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /* do nothing here*/
    return;
}


/*-------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_INIT_ProvisionComplete									
 *-------------------------------------------------------------------------
 * Purpose: This function is call when provision complete.										
 * INPUT   : None															
 * OUTPUT  : None															
 * RETURN  : None														
 * NOTE:     None															
 *-------------------------------------------------------------------------
 */
void SNMP_INIT_ProvisionComplete()
{
    SNMP_MGR_ProvisionComplete();
    SNMP_TASK_ProvisionComplete();
}
