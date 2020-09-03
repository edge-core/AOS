/*-----------------------------------------------------------------------------
 * FILE NAME: poe_init.c
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 *    This file provides the APIs to init the PoE database.
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

#include "sys_type.h"
#include "poe_mgr.h"
#include "sys_cpnt.h"
#include "leaf_sys.h"
#include "poedrv.h"

#include "poe_task.h"
#ifdef SYS_CPNT_POE_PSE_DOT3AT
#include "poe_engine.h"
#endif

//#include "stktplg_board.h"
#if 0
#define DBG_PRINT(format,...) printf("%s(%d): "format"\r\n",__FUNCTION__,__LINE__,##__VA_ARGS__); fflush(stdout);
#else
#define DBG_PRINT(format,...)
#endif

//static BOOL_T   poe_init_support_poe;

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_INIT_InitiateSystemResources                               
 * -------------------------------------------------------------------------
 * FUNCTION: init POE system              
 * INPUT   : None                                                   
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_INIT_InitiateSystemResources(void)
{
DBG_PRINT();
    POE_MGR_InitiateSystemResources();

#ifdef SYS_CPNT_POE_PSE_DOT3AT
    POE_ENGINE_InitialSystemResource();
#endif

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_INIT_EnterMasterMode                               
 * -------------------------------------------------------------------------
 * FUNCTION: enter master state
 * INPUT   : None                                                   
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_INIT_EnterMasterMode(void)
{
DBG_PRINT();
    POE_MGR_EnterMasterMode();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_INIT_EnterSlaveMode                               
 * -------------------------------------------------------------------------
 * FUNCTION: enter slave state
 * INPUT   : None                                                   
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_INIT_EnterSlaveMode(void)
{
DBG_PRINT();
    POE_MGR_EnterSlaveMode();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_INIT_SetTransitionMode                               
 * -------------------------------------------------------------------------
 * FUNCTION: set transition state flag         
 * INPUT   : None                                                   
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_INIT_SetTransitionMode(void)
{
DBG_PRINT();

    POE_MGR_SetTransitionMode();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_INIT_EnterTransitionMode                               
 * -------------------------------------------------------------------------
 * FUNCTION: enter transition state     
 * INPUT   : None                                                   
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_INIT_EnterTransitionMode(void)
{
DBG_PRINT();

    POE_MGR_EnterTransitionMode();
}

/* -------------------------------------------------------------------------
 * ROUTINE POE_INIT_HandleHotInsertion                          
 * -------------------------------------------------------------------------
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
void POE_INIT_HandleHotInsertion(UI32_T starting_port_ifindex,
											UI32_T number_of_port,
											BOOL_T use_default)
{
    POE_MGR_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);
    return;
}
/* -------------------------------------------------------------------------
 * ROUTINE POE_INIT_HandleHotRemoval                          
 * -------------------------------------------------------------------------
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
void POE_INIT_HandleHotRemoval(UI32_T starting_port_ifindex,
										  UI32_T number_of_port)
{
    return;
}
/*--------------------------------------------------------------------------
 * FUNCTION NAME - POE_INIT_Create_Tasks
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will create task. 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void POE_INIT_CreateTasks(void)
{
#if 0 /* Eugene marked for not using universal image */
    if (SUPPORT_POE_NONE == poe_init_support_poe)
    {
        return;
    }
#endif
DBG_PRINT();

    POE_TASK_CreateTasks();

} 

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
void POE_INIT_ProvisionComplete(void)
{
#if 0 /* Eugene marked for not using universal image */
    if (SUPPORT_POE_NONE == poe_init_support_poe)
    {
        return;
    }
#endif
DBG_PRINT();

}/* End of POE_INIT_ProvisionComplete */

