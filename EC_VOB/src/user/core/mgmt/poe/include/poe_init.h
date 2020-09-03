/*-----------------------------------------------------------------------------
 * FILE NAME: poe_init.h
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 *    This file declares the APIs to init the PoE database.
 * 
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    04/7/2003 - Kelly Hung, Created
 *    12/03/2008 - Eugene Yu, porting POE to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */

#ifndef POE_INIT_H
#define POE_INIT_H

#include "sys_type.h"

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_INIT_Initiate_System_Resources                               
 * -------------------------------------------------------------------------
 * FUNCTION: init POE system              
 * INPUT   : None                                                   
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_INIT_InitiateSystemResources(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_INIT_SetTransitionMode                               
 * -------------------------------------------------------------------------
 * FUNCTION: set transition state flag         
 * INPUT   : None                                                   
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_INIT_SetTransitionMode(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_INIT_EnterTransitionMode                               
 * -------------------------------------------------------------------------
 * FUNCTION: enter transition state     
 * INPUT   : None                                                   
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_INIT_EnterTransitionMode(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_INIT_EnterMasterMode                               
 * -------------------------------------------------------------------------
 * FUNCTION: enter master state
 * INPUT   : None                                                   
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_INIT_EnterMasterMode(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_INIT_EnterSlaveMode                               
 * -------------------------------------------------------------------------
 * FUNCTION: enter slave state
 * INPUT   : None                                                   
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_INIT_EnterSlaveMode(void);
/* -------------------------------------------------------------------------
 * ROUTINE POE_INIT_HandleHotInsertion                          
 * -------------------------------------------------------------------------
 * FUNCTION: enter slave state
 * INPUT   :starting_port_ifindex,number_of_port,use_default);                                                  
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_INIT_HandleHotInsertion(UI32_T starting_port_ifindex,
											UI32_T number_of_port,
											BOOL_T use_default);
/* -------------------------------------------------------------------------
 * ROUTINE POE_INIT_HandleHotRemoval                          
 * -------------------------------------------------------------------------
 * FUNCTION: enter slave state
 * INPUT   : starting_port_ifindex, number_of_port                                              
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_INIT_HandleHotRemoval(UI32_T starting_port_ifindex,
										  UI32_T number_of_port);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - POE_INIT_Create_Tasks
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will create task. 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void POE_INIT_CreateTasks(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - POE_INIT_ProvisionComplete
 * ------------------------------------------------------------------------
 * FUNCTION : provision complete.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
#ifdef SYS_CPNT_POE_PSE_DOT3AT
void POE_INIT_ProvisionComplete(void);
#endif

#endif /* END OF POE_INIT_H */
