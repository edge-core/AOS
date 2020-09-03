/*-----------------------------------------------------------------------------
 * Module Name: bridge_init.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate the system resources and create the task.
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    10/22/2001 - Allen Cheng, Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2001
 *-----------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "extbrg_mgr.h"
//#include "pri_mgr.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
#include "xstp_init.h"
#else
#include "sta_init.h"
#endif


/*-------------------------------------------------------------------------
 * FUNCTION NAME: BRIDGE_INIT_InitiateSystemResources
 * PURPOSE: This function is used to initialize the bridge module.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
void BRIDGE_INIT_InitiateSystemResources(void)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_INIT_Initiate_System_Resources();
#else
    STA_INIT_Initiate_System_Resources();
#endif
//    PRI_MGR_Initiate_System_Resources();
    EXTBRG_MGR_Initiate_System_Resources();
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: BRIDGE_INIT_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
void BRIDGE_INIT_Create_InterCSC_Relation(void)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_INIT_Create_InterCSC_Relation();
#else
    STA_INIT_Create_InterCSC_Relation();
#endif
//    PRI_MGR_Create_InterCSC_Relation();
    EXTBRG_MGR_Create_InterCSC_Relation();
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: BRIDGE_INIT_Create_Tasks
 * PURPOSE: This function is used to create the main task of the bridge module.
 * INPUT:   tg_handle - the handle of thread group
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
void BRIDGE_INIT_Create_Tasks(void)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_INIT_Create_Tasks();
#else
    STA_INIT_Create_Tasks();
#endif
    return;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - BRIDGE_INIT_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set BRIDGE_into master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void BRIDGE_INIT_EnterMasterMode(void)
{
//    PRI_MGR_EnterMasterMode();
    EXTBRG_MGR_EnterMasterMode();
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_INIT_EnterMasterMode();
#else
    STA_INIT_EnterMasterMode();
#endif

    return;
} /* end of BRIDGE_INIT_EnterMasterMode() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - BRIDGE_INIT_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set BRIDGE into slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void BRIDGE_INIT_EnterSlaveMode(void)
{

//    PRI_MGR_EnterSlaveMode();
    EXTBRG_MGR_EnterSlaveMode();
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_INIT_EnterSlaveMode();
#else
    STA_INIT_EnterSlaveMode();
#endif

    return;
} /* end of BRIDGE_INIT_EnterSlaveMode() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - BRIDGE_INIT_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set BRIDGE into transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void BRIDGE_INIT_EnterTransitionMode(void)
{
//    PRI_MGR_EnterTransitionMode();
    EXTBRG_MGR_EnterTransitionMode();
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_INIT_EnterTransitionMode();
#else
    STA_INIT_EnterTransitionMode();
#endif

    return;
} /* end of BRIDGE_INIT_EnterTransitionMode() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - BRIDGE_INIT_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void  BRIDGE_INIT_SetTransitionMode(void)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_INIT_SetTransitionMode();
#else
    STA_INIT_SetTransitionMode();
#endif
    EXTBRG_MGR_SetTransitionMode();
//    PRI_MGR_SetTransitionMode();
} /* end of BRIDGE_INIT_SetTransitionMode() */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - BRIDGE_INIT_HandleHotInsertion
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
void BRIDGE_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
//    PRI_MGR_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);

    /* XSTP */
    XSTP_INIT_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);
} /* end of BRIDGE_INIT_HandleHotInsertion() */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - BRIDGE_INIT_HandleHotRemoval
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
void BRIDGE_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
//    PRI_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);

    /* XSTP */
    XSTP_INIT_HandleHotRemoval(starting_port_ifindex, number_of_port);
}/* end of BRIDGE_INIT_HandleHotRemoval() */
