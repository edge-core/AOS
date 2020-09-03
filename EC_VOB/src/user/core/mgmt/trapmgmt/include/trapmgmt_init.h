/* Project Name: Mercury 
 * File_Name : TRAPMGMT_INIT.c
 * Purpose     : TRAP initiation and Trap task creation
 *
 * 2001/10/16    : James Chien     Create this file
 *
 * Copyright(C)      Accton Corporation, 1999, 2000 
 *
 * Note    : Inherit from Foxfire Switch product familiy designed by Orlando
 */

#ifndef TRAPMGMT_INIT_H
#define TRAPMGMT_INIT_H 
#include "sys_type.h"

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TRAPMGMT_INIT_Initiate_System_Resourves
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
void TRAPMGMT_INIT_Initiate_System_Resources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - TRAPMGMT_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void TRAPMGMT_INIT_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TRAPMGMT_INIT_Create_Task()
 *---------------------------------------------------------------------------
 * PURPOSE: This function creates all the task of this subsystem.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE: 1. This function shall not be invoked before SNMP_INIT_Init() is performed
 *---------------------------------------------------------------------------
 */
void TRAPMGMT_INIT_Create_Tasks(void);


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
void TRAPMGMT_INIT_EnterMasterMode(void);


/*-------------------------------------------------------------------------
 * ROUTINE NAME - TRAPMGMT_INIT_EnterTransitionMode									
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Transition Operation mode.										
 * INPUT   : None															
 * OUTPUT  : None															
 * RETURN  : None														
 * NOTE: In Slave Operation mode, any Cisco Commands issued from local console or 
 *       Telnet will be ignored.															
 *-------------------------------------------------------------------------
 */
void TRAPMGMT_INIT_EnterTransitionMode(void);

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
void TRAPMGMT_INIT_SetTransitionMode(void);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - TRAPMGMT_INIT_EnterSlaveMode									
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Slave Operation mode.										
 * INPUT   : None															
 * OUTPUT  : None															
 * RETURN  : None														
 * NOTE: In Slave Operation mode, any Cisco Commands issued from local console or 
 *       Telnet will be ignored.															
 *-------------------------------------------------------------------------
 */
  
void TRAPMGMT_INIT_EnterSlaveMode(void);


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
void TRAPMGMT_INIT_ProvisionComplete(void);

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
void TRAPMGMT_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

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
void TRAPMGMT_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

#endif 

/* end of TRAPMGMT_INIT.H */

