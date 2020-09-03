/* Module Name: SWCTRL_INIT.C
 * Purpose: 
 *        ( 1. Whole module function and scope.                 )
 *         This file provides the initialization function of switch control
 *         module.
 *        ( 2.  The domain MUST be handled by this module.      )
 *        ( 3.  The domain would not be handled by this module. )
 * Notes: 
 *        ( Something must be known or noticed by developer     )
 * History:                                                               
 *       Date        Modifier        Reason
 *       2001/6/1    Jimmy Lin       Create this file
 *       2001/11/1   Arthur Wu       Take over
 *       2002/9/23   Charles Cheng   Refine for stacking
 *
 * Copyright(C)      Accton Corporation, 1999, 2000   				
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h> 
#include <stdlib.h>

#include "sys_bld.h"
#include "sys_adpt.h"
#include "leaf_sys.h"
#include "sysfun.h"
#include "leaf_2863.h"
#include "leaf_2674p.h"
#include "leaf_2674q.h"
#include "leaf_2933.h"
#include "leaf_es3626a.h"
// #include "stktplg_type.h"
// #include "stktplg_mgr.h"
#include "swctrl.h"
#include "swctrl_om.h"
#include "swctrl_init.h"
#include "swctrl_task.h"
#include "sysrsc_mgr.h"
/* LOCAL VARIABLES
 */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_INIT_Initiate_System_Resources
 * -------------------------------------------------------------------------
 * FUNCTION: This function will init all the compoents which is located at
 *           directory of swctrl,
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_INIT_Initiate_System_Resources(void)
{
    /*SWCTRL*/
    SWCTRL_Init();
    SWCTRL_Task_Init();
    
}    

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_INIT_Create_InterCSC_Relation
 * -------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_INIT_Create_InterCSC_Relation(void)
{
    /*SWCTRL*/
    SWCTRL_Create_InterCSC_Relation();
    SWCTRL_Task_Create_InterCSC_Relation();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_INIT_Create_Tasks
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create all the compoent tasks which are located
 *           at directory of swctrl,
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_INIT_Create_Tasks(void)
{
    /*SWCTRL*/
    SWCTRL_Task_CreateTask();    
   
}    

/*-------------------------------------------------------------------------
 * FUNCTION NAME: SWCTRL_INIT_SetTransitionMode
 * PURPOSE: set transition state flag
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 * -------------------------------------------------------------------------*/
void SWCTRL_INIT_SetTransitionMode(void)
{
    /*SWCTRL*/
    SWCTRL_SetTransitionMode();
    SWCTRL_TASK_SetTransitionMode();
        
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: SWCTRL_INIT_EnterTransitionMode
 * PURPOSE: enter transition state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 * -------------------------------------------------------------------------*/
void SWCTRL_INIT_EnterTransitionMode(void)
{
    /*SWCTRL*/
    SWCTRL_EnterTransitionMode();
    SWCTRL_TASK_EnterTransitionMode();
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: SWCTRL_INIT_EnterMasterMode
 * PURPOSE: enter master state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 * -------------------------------------------------------------------------*/
void SWCTRL_INIT_EnterMasterMode(void)
{   
    /*SWCTRL*/
    SWCTRL_EnterMasterMode();
    SWCTRL_TASK_EnterMasterMode();
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: SWCTRL_INIT_EnterSlaveMode
 * PURPOSE: enter slave state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 * -------------------------------------------------------------------------*/
void SWCTRL_INIT_EnterSlaveMode(void)
{
    /*SWCTRL*/
    SWCTRL_EnterSlaveMode();
    SWCTRL_TASK_EnterSlaveMode();
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: SWCTRL_INIT_HandleHotInsertion
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
void SWCTRL_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /*SWCTRL*/
    SWCTRL_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME: SWCTRL_INIT_HandleHotRemoval
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
void SWCTRL_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /*SWCTRL*/
    SWCTRL_HandleHotRemoval(starting_port_ifindex, number_of_port);
    
}

