/*-----------------------------------------------------------------------------
 * Module Name: amtrl3_init.c
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
 *   Ted             03/16/2002         Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "amtrl3_task.h"
#include "amtrl3_mgr.h"
//#include "amtrl3mc_mgr.h"
#include "amtrl3_backdoor.h"

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_INIT_Initiate_System_Resources
 * PURPOSE: init AMTR data
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void AMTRL3_INIT_Initiate_System_Resources(void)
{
    AMTRL3_MGR_Initiate_System_Resources();
    AMTRL3_TASK_Init();
    //AMTRL3MC_MGR_Init();
    
    return;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_INIT_ProvisionComplete
 * -------------------------------------------------------------------------
 * FUNCTION: This function will tell the AMTRL3 to start action.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void AMTRL3_INIT_ProvisionComplete(void)
{
    
     //printf("Enter provision complete.\n");
     return;
    
}    


/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_INIT_Create_Tasks
 * PURPOSE: create AMTR task
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void AMTRL3_INIT_Create_Tasks(void)
{
    AMTRL3_TASK_CreateTask();
    
    return;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_INIT_SetTransitionMode
 * PURPOSE: set transition state
 * INPUT    : None
 * OUTPUT   : None                                                           
 * RETUEN:  None
 * NOTES:   
 */
void AMTRL3_INIT_SetTransitionMode(void)
{  
    AMTRL3_MGR_SetTransitionMode();
    AMTRL3_TASK_SetTransitionMode();
    //AMTRL3MC_MGR_SetTransitionMode();
    
    return;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_INIT_EnterTransitionMode
 * PURPOSE: enter transition state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void AMTRL3_INIT_EnterTransitionMode(void)
{
    AMTRL3_MGR_EnterTransitionMode();
    AMTRL3_TASK_EnterTransitionMode();
    //AMTRL3MC_MGR_EnterTransitionMode();
    
    return;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_INIT_EnterMasterMode
 * PURPOSE: enter master state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void AMTRL3_INIT_EnterMasterMode(void)
{
    AMTRL3_MGR_EnterMasterMode();
    AMTRL3_TASK_EnterMasterMode();
    //AMTRL3MC_MGR_EnterMasterMode();
    
    return;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_INIT_EnterSlaveMode
 * PURPOSE: enter slave state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void AMTRL3_INIT_EnterSlaveMode(void)
{
    AMTRL3_MGR_EnterSlaveMode();
    AMTRL3_TASK_EnterSlaveMode();
    //AMTRL3MC_MGR_EnterSlaveMode();
    
    return;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_INIT_HandleHotInsertion
 * PURPOSE: Hot swap init function
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void AMTRL3_INIT_HandleHotInsertion(UI32_T starting_port_ifindex,
                                    UI32_T number_of_port,
                                    BOOL_T use_default)
{
    AMTRL3_MGR_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);
    return;
} /* end of AMTRL3_INIT_HandleHotInsertion ()*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_INIT_HandleHotRemoval
 * PURPOSE: Hot swap init function for removal
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void AMTRL3_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    AMTRL3_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);
    return;
} /* end of AMTRL3_INIT_HandleHotRemoval() */

