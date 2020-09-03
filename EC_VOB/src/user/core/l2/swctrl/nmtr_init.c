/*-----------------------------------------------------------------------------
 * Module Name: nmtr_init.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate the system resources and create the task.
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    11/27/2001 - Arthur Wu, Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2001
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "nmtr_mgr.h"

/*-------------------------------------------------------------------------
 * FUNCTION NAME: NMTR_INIT_InitiateSystemResources
 * PURPOSE: init NMTR data
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void NMTR_INIT_InitiateSystemResources(void)
{
    NMTR_MGR_Init();
    
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: NMTR_INIT_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void NMTR_INIT_Create_InterCSC_Relation(void)
{
    NMTR_MGR_Create_InterCSC_Relation();
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: NMTR_INIT_Create_Tasks
 * PURPOSE: create NMTR task
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void NMTR_INIT_Create_Tasks(void)
{
    return;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: NMTR_INIT_EnterTransitionMode
 * PURPOSE: enter transition state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void NMTR_INIT_EnterTransitionMode(void)
{
    NMTR_MGR_EnterTransitionMode();
    return;
}

/*------------------------------------------------------------------------------
 * Function : NMTR_INIT_SetTransitionMode()
 *------------------------------------------------------------------------------
 * Purpose  : This function will set the operation mode to transition mode
 * INPUT    : None
 * OUTPUT   : None                                                           
 * RETURN   : None
 * NOTE     : 
 *-----------------------------------------------------------------------------*/
void NMTR_INIT_SetTransitionMode(void)
{  
    NMTR_MGR_SetTransitionMode();
}	/*	end of NMTR_MGR_SetTransitionMode	*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME: NMTR_INIT_EnterMasterMode
 * PURPOSE: enter master state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void NMTR_INIT_EnterMasterMode(void)
{
    NMTR_MGR_EnterMasterMode();
    return;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: NMTR_INIT_EnterSlaveMode
 * PURPOSE: enter slave state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void NMTR_INIT_EnterSlaveMode(void)
{
    NMTR_MGR_EnterSlaveMode();
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: NMTR_INIT_HandleHotInsertion
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.

 * -------------------------------------------------------------------------*/
void NMTR_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    NMTR_MGR_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: NMTR_INIT_HandleHotRemoval
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * -------------------------------------------------------------------------*/
void NMTR_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    NMTR_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);
}

