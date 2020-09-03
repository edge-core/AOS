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
 *              9-25-2002      Add SetTransitionMode for stacking                       *
 * -------------------------------------------------------------------------------------*
 * Copyright(C)		   Accton Techonology Corporation 2001                              *
 * =====================================================================================*/
 
/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "mib2mgmt_init.h"
#include "mib2_mgr.h"
#include "if_mgr.h"


/* EXPORTED SUBPROGRAM BODIES
 */
 
/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2MGMT_INIT_Initiate_System_Resources
 *--------------------------------------------------------------------------
 * PURPOSE  : This function allocates and initiates the system resource for
 *            MIB2_MGR module.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void MIB2MGMT_INIT_Initiate_System_Resources(void)
{
    IF_MGR_InitiateSystemResources();
    MIB2_MGR_InitiateSystemResources();
    return;
    
} /* end of MIB2MGMT_INIT_Initiate_System_Resources() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2MGMT_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void MIB2MGMT_INIT_Create_InterCSC_Relation(void)
{
    IF_MGR_Create_InterCSC_Relation();
    MIB2_MGR_Create_InterCSC_Relation();
} /* end of MIB2MGMT_INIT_Create_InterCSC_Relation */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2MGMT_INIT_Create_Tasks
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will create task. 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void MIB2MGMT_INIT_Create_Tasks(void)
{  
    return;
} /* end of MIB2MGMT_INIT_Create_Tasks() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2MGMT_INIT_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set MIB2MGMT_into master mode.  
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void MIB2MGMT_INIT_EnterMasterMode(void)
{
    IF_MGR_EnterMasterMode();
    MIB2_MGR_EnterMasterMode();
    
    return;
} /* end of MIB2MGMT_INIT_EnterTransitionMode() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2MGMT_INIT_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set MIB2MGMT into slave mode. 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void MIB2MGMT_INIT_EnterSlaveMode(void)
{
    IF_MGR_EnterSlaveMode();
    MIB2_MGR_EnterSlaveMode();
    return;
    
} /* end of MIB2MGMT_INIT_EnterTransitionMode() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - MIB2MGMT_INIT_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set MIB2MGMT into transition mode. 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void MIB2MGMT_INIT_EnterTransitionMode(void)
{
    IF_MGR_EnterTransitionMode();
    MIB2_MGR_EnterTransitionMode();
    return;
    
} /* end of MIB2MGMT_INIT_EnterTransitionMode() */


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
void  MIB2MGMT_INIT_SetTransitionMode(void)
{
    IF_MGR_SetTransitionMode();
    MIB2_MGR_SetTransitionMode();
    return;    
    
} /* end of MIB2MGMT_INIT_SetTransitionMode() */

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
void MIB2MGMT_INIT_ProvisionComplete(void)
{
	IF_MGR_ProvisionComplete();
    MIB2_MGR_ProvisionComplete(); 
	return;
}

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
void MIB2MGMT_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* do nothing here*/
    return;
}

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
void MIB2MGMT_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /* do nothing here*/
    return;
}
