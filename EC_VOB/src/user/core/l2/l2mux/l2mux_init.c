/*-----------------------------------------------------------------------------
 * Module Name: L2MUX_INIT.C   										 		   
 *-----------------------------------------------------------------------------
 * PURPOSE: This module is used to initialize the component of L2MUX.		                                     	   
 *-----------------------------------------------------------------------------
 * NOTES: 
 *
 * HISTORY:																	   
 *    10/21/2002 - Benson Hsu, Created	
 * 						   
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002 		   
 *-----------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "l2mux_mgr.h"

/* FUNCTION NAME: L2MUX_INIT_InitiateProcessResources
 *----------------------------------------------------------------------------------
 * PURPOSE: Initialization of L2MUX
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void L2MUX_INIT_InitiateProcessResources(void)
{
    L2MUX_MGR_InitiateProcessResources();
}

/* FUNCTION NAME: L2MUX_INIT_InitiateSystemResources
 *----------------------------------------------------------------------------------
 * PURPOSE: Initialization of L2MUX
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void L2MUX_INIT_InitiateSystemResources(void)
{
    L2MUX_MGR_InitiateSystemResources();
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: L2MUX_INIT_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for L2MUX in the context of the calling process.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void L2MUX_INIT_AttachSystemResources(void)
{
    L2MUX_MGR_AttachSystemResources();
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MUX_INIT_Create_InterCSC_Relation
 *-------------------------------------------------------------------------
 * PURPOSE : This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MUX_INIT_Create_InterCSC_Relation(void)
{
    L2MUX_MGR_Create_InterCSC_Relation();
    return;
}

/* FUNCTION NAME: L2MUX_INIT_SetTransitionMode
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will set L2MUX to transition mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void L2MUX_INIT_SetTransitionMode(void)
{
    L2MUX_MGR_SetTransitionMode();
}


/* FUNCTION NAME: L2MUX_INIT_EnterTransitionMode
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will force L2MUX to enter transition mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void L2MUX_INIT_EnterTransitionMode(void)
{
    L2MUX_MGR_EnterTransitionMode();
}


/* FUNCTION NAME: L2MUX_INIT_EnterMasterMode
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will force L2MUX to enter master mode	
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void L2MUX_INIT_EnterMasterMode(void)
{
    L2MUX_MGR_EnterMasterMode();
}


/* FUNCTION NAME: L2MUX_INIT_EnterSlaveMode
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will force L2MUX to enter slave mode	
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void L2MUX_INIT_EnterSlaveMode(void)
{
    L2MUX_MGR_EnterSlaveMode();   
}


/* FUNCTION NAME: L2MUX_INIT_Create_Tasks
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will create a task for L2MUX
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   There is no task on L2MUX.
 */
void L2MUX_INIT_Create_Tasks(void)
{
    return;
}