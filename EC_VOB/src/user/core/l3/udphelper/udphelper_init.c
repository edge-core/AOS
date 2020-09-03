/*-----------------------------------------------------------------------------
 * Module Name: udphelper_init.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate the system resources and create the task.
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *
 *   By              Date       Ver.   Modification Description
 *   --------------- ---------- -----  ---------------------------------------
 *   LinLi          03/31/2009         Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2009
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "udphelper_mgr.h"
#include "udphelper_backdoor.h"

/*-------------------------------------------------------------------------
 * FUNCTION NAME: UDPHELPER_INIT_Initiate_System_Resources
 * PURPOSE: init UDPHELPER data
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void UDPHELPER_INIT_Initiate_System_Resources(void)
{
    UDPHELPER_MGR_Initiate_System_Resources();
    return;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - UDPHELPER_INIT_ProvisionComplete
 * -------------------------------------------------------------------------
 * FUNCTION: This function will tell the UDPHELPER to start action.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void UDPHELPER_INIT_ProvisionComplete(void)
{
    return;
}    

/*-------------------------------------------------------------------------
 * FUNCTION NAME: UDPHELPER_INIT_SetTransitionMode
 * PURPOSE: set transition state
 * INPUT    : None
 * OUTPUT   : None                                                           
 * RETUEN:  None
 * NOTES:   
 */
void UDPHELPER_INIT_SetTransitionMode(void)
{  
    UDPHELPER_MGR_SetTransitionMode();
    return;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: UDPHELPER_INIT_EnterTransitionMode
 * PURPOSE: enter transition state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void UDPHELPER_INIT_EnterTransitionMode(void)
{
    UDPHELPER_MGR_EnterTransitionMode();
    return;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: UDPHELPER_INIT_EnterMasterMode
 * PURPOSE: enter master state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void UDPHELPER_INIT_EnterMasterMode(void)
{
    UDPHELPER_MGR_EnterMasterMode();
    return;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: UDPHELPER_INIT_EnterSlaveMode
 * PURPOSE: enter slave state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void UDPHELPER_INIT_EnterSlaveMode(void)
{
    UDPHELPER_MGR_EnterSlaveMode();
    return;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: UDPHELPER_INIT_HandleHotRemoval
 * PURPOSE: Hot swap init function for removal
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void UDPHELPER_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    UDPHELPER_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);
    return;
} /* end of UDPHELPER_INIT_HandleHotRemoval() */


