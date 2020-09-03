/*-----------------------------------------------------------------------------
 * Module Name: LACP_init.c
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    12/05/2001 - Lewis Kang, Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2001
 *-----------------------------------------------------------------------------
 */

#include "sys_type.h"
#include "lacp_init.h"
/*#include "lacp_task.h"*/
#include "lacp_mgr.h"

/* ---------------------------------------------------
 *                  EXTERN ROUTINES
 * ---------------------------------------------------
 */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_INIT_InitiateSystemResources
 *-------------------------------------------------------------------------
 * PURPOSE : Create queue for LACP and set LACP opstate to "FALSE"
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void LACP_INIT_InitiateSystemResources(void)
{
    LACP_MGR_Init();
/*    LACP_TASK_Init();*/
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_INIT_Create_InterCSC_Relation
 *-------------------------------------------------------------------------
 * PURPOSE : This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void LACP_INIT_Create_InterCSC_Relation(void)
{
    LACP_MGR_Create_InterCSC_Relation();
/*    LACP_TASK_Create_InterCSC_Relation();*/
}

void LACP_INIT_Create_Tasks(void)
{
/*    LACP_TASK_CreateTask();*/
}

void LACP_INIT_EnterMasterMode(void)
{
    LACP_MGR_EnterMasterMode();
/*    LACP_TASK_EnterMasterMode();*/
}

void LACP_INIT_EnterSlaveMode(void)
{
    LACP_MGR_EnterSlaveMode();
/*    LACP_TASK_EnterSlaveMode();*/
}

void LACP_INIT_EnterTransitionMode(void)
{
    LACP_MGR_EnterTransitionMode();
/*    LACP_TASK_EnterTransitionMode();*/
}

void LACP_INIT_SetTransitionMode(void)
{
    LACP_MGR_SetTransitionMode();
/*    LACP_TASK_SetTransitionMode();*/
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_INIT_HandleHotInsertion
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
void LACP_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    LACP_MGR_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);
    return;
} /* End of LACP_INIT_HandleHotInsertion */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_INIT_HandleHotRemoval
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
void LACP_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    LACP_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);
    return;
} /* End of LACP_INIT_HandleHotRemoval */
