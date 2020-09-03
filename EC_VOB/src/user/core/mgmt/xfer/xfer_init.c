/* Project Name: Mercury 
 * Module Name : XFER_INIT.C
 * Abstract    : to be included in root.c and tn_main.c to access cli
 * Purpose     : XFER initiation and XFER task creation
 *
 * History :                                                               
 *          Date        Modifier        Reason
 *          2001/10/11    beck chen     Create this file
 *
 * Copyright(C)      Accton Corporation, 1999, 2000 
 *
 * Note    : 
 */

/*------------------------------------------------------------------------
 * INCLUDE STRUCTURES                             
 *------------------------------------------------------------------------*/
#include "sys_type.h"
#include "sysfun.h"
#include "xfer_init.h"
#include "xfer_mgr.h"
#include "fs.h"
#include "stktplg_mgr.h"

/* EXPORTED SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - XFER_INIT_Init
 *---------------------------------------------------------------------------
 * PURPOSE: This function initaites the system resources, such as queue, semaphore, 
 *          and events used by this subsystem. All the call back functions shall be
 *          registered during subsystem initiation.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:    
 *---------------------------------------------------------------------------
 */
void XFER_INIT_InitiateProcessResources(void)
{
    XFER_MGR_Init();
    
} /* end of XFER_INIT_Init() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - XFER_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void XFER_INIT_Create_InterCSC_Relation(void)
{
    XFER_MGR_Create_InterCSC_Relation();
} /* end of XFER_INIT_Create_InterCSC_Relation */


/*--------------------------------------------------------------------------
 * ROUTINE NAME - XFER_INIT_Create_Task()
 *---------------------------------------------------------------------------
 * PURPOSE: This function creates all the task of this subsystem.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE: 
 *---------------------------------------------------------------------------
 */
void XFER_INIT_CreateTask(void)
{
    /* Create console task since this fuction can only be called by Root
     */
    XFER_MGR_CreateTask();
     
} /* End of XFER_INIT_Create_Task() */



/*-------------------------------------------------------------------------
 * ROUTINE NAME - XFER_INIT_EnterMasterMode                         
 *-------------------------------------------------------------------------
 * PURPOSE: This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the CLI subsystem will enter the
 *          Master Operation mode.                                                            
 * INPUT   : None														   
 * OUTPUT  : None														   
 * RETURN  : None														   
 * NOTE: 
 *-------------------------------------------------------------------------
 */
void XFER_INIT_EnterMasterMode(void)
{
    XFER_MGR_EnterMasterMode();
} /* End of XFER_INIT_EnterMasterMode() */

void XFER_INIT_ProvisionComplete()
{
    XFER_MGR_ProvisionComplete();
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME - XFER_INIT_EnterSlaveMode									
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Slave Operation mode.										
 * INPUT   : None															
 * OUTPUT  : None															
 * RETURN  : None														
 * NOTE: 														
 *-------------------------------------------------------------------------
 */
 
void XFER_INIT_EnterSlaveMode(void)
{
    XFER_MGR_EnterSlaveMode();
} /* end XFER_INIT_EnterSlaveMode() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - XFER_INIT_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : It's the temporary transition mode between system into master 
 *            mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : 
 *--------------------------------------------------------------------------*/
void XFER_INIT_EnterTransitionMode(void)
{
    XFER_MGR_EnterTransitionMode();
}/* end XFER_INIT_EnterTransitionMode() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - XFER_INIT_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This call will set dhcp_mgr into transition mode to prevent 
 *            calling request.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void XFER_INIT_SetTransitionMode(void)
{
	XFER_MGR_SetTransitionMode();
}/* end XFER_INIT_SetTransitionMode() */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - XFER_INIT_HandleHotInsertion
 * ------------------------------------------------------------------------
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
 * ------------------------------------------------------------------------
 */
void XFER_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    XFER_MGR_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);

    return;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XFER_INIT_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void XFER_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    XFER_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);

    return;
}

/* End of XFER_INIT.C */
