/* =====================================================================================*
 * FILE	NAME: MIB2_MGR_INIT.c                                                           *
 *                                                                                      *
 * ABSTRACT:  The two primary functions	of this	file is	to Initialize MIB2_MGR resouce  *
 *	          information and to create Task.									        *
 *                                                                                      *
 * MODIFICATION	HISOTRY:	                                                            *
 *                                                                                      *
 * MODIFIER		   DATE		   DESCRIPTION                                              *
 * -------------------------------------------------------------------------------------*
 * amytu		10-22-2001	   First Create     							            *
 *                                                                                      *
 * -------------------------------------------------------------------------------------*
 * Copyright(C)		   Accton Techonology Corporation 2001                              *
 * =====================================================================================*/
 
#ifndef MIB2_MGR_INIT_H
#define MIB2_MGR_INIT_H
#include "sys_type.h"

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2_MGR_INIT_Initiate_System_Resources
 *--------------------------------------------------------------------------
 * PURPOSE  : This function allocates and initiates the system resource for
 *            MIB2_MGR module.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void MIB2MGMT_INIT_Initiate_System_Resources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2MGMT_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void MIB2MGMT_INIT_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2_MGR_INIT_Create_Tasks
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will create task. 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void MIB2MGMT_INIT_Create_Tasks(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2MGMT_INIT_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set MIB2MGMT_into master mode.  
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void MIB2MGMT_INIT_EnterMasterMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2MGMT_INIT_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set MIB2MGMT into slave mode. 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void MIB2MGMT_INIT_EnterSlaveMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2MGMT_INIT_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set MIB2MGMT into transition mode. 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void MIB2MGMT_INIT_EnterTransitionMode(void);


/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2MGMT_INIT_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the component to temporary transition mode           
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void  MIB2MGMT_INIT_SetTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2MGMT_INIT_ProvisionComplete
 *--------------------------------------------------------------------------
 * PURPOSE  : stkctrl will call this function when provision completed 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void MIB2MGMT_INIT_ProvisionComplete(void);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - MIB2MGMT_INIT_HandleHotInsertion									
 *-------------------------------------------------------------------------
 * Purpose: This function is called when the module is plug in.								
 * INPUT   : None															
 * OUTPUT  : None															
 * RETURN  : None														
 * NOTE: This function do nothing here.															
 *-------------------------------------------------------------------------
 */
void MIB2MGMT_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - MIB2MGMT_INIT_HandleHotRemoval									
 *-------------------------------------------------------------------------
 * Purpose: This function is called when module is plug off.									
 * INPUT   : None															
 * OUTPUT  : None															
 * RETURN  : None														
 * NOTE:     SNMP do nothing here														
 *-------------------------------------------------------------------------
 */
void MIB2MGMT_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

#endif
