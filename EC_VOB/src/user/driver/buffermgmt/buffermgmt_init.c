/* Module Name: BUFFERMGMT_INIT.C
 * Purpose:
 *  This module is in first thing needed to do of Buffer Management.
 *
 * Notes:
 *
 * History:
 *          Date      -- Modifier,        Reason
 * ------------------------------------------------------------------------
 *  V1.0  2004.08.10  -- Erica Li,        Creation
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "buffermgmt_init.h"
#include "buffer_mgr.h"
#include "sysrsc_mgr.h"

/* EXPORTED SUBPROGRAM BODIES
 */

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFERMGMT_INIT_InitiateSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Initiate system resource for BUFFERMGMT
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void BUFFERMGMT_INIT_InitiateSystemResources(void)
{
    BUFFER_MGR_InitiateSystemResources();
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFERMGMT_INIT_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for BUFFERMGMT in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void BUFFERMGMT_INIT_AttachSystemResources(void)
{
    BUFFER_MGR_AttachSystemResources();
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFERMGMT_INIT_GetShMemInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Provide shared memory information for SYSRSC.
 * INPUT:   None
 * OUTPUT:  segid_p  --  shared memory segment id
 *          seglen_p --  length of the shared memroy segment
 * RETUEN:  None
 * NOTES:
 *    This function is called in SYSRSC_MGR_CreateSharedMemory().
 *---------------------------------------------------------------------------------*/
void BUFFERMGMT_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_BUFFERMGMT_SHMEM_SEGID;
    *seglen_p = BUFFERMGMT_INIT_BUFFER_SIZE;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - BUFFERMGMT_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void BUFFERMGMT_INIT_Create_InterCSC_Relation(void)
{
    return;
} /* end of BUFFERMGMT_INIT_Create_InterCSC_Relation */

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFERMGMT_INIT_Create_Tasks
 *---------------------------------------------------------------------------------
 * PURPOSE: create buffer management task
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void BUFFERMGMT_INIT_Create_Tasks(void)
{
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFERMGMT_INIT_EnterMasterMode
 *---------------------------------------------------------------------------------
 * PURPOSE: make BUFFERMGMT to enter master mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   The function will initialize lower level driver and set local variables
 *          to default state. It should be called *ONCE ONLY* during stacking.
 *---------------------------------------------------------------------------------*/
void BUFFERMGMT_INIT_EnterMasterMode(void)
{
    BUFFER_MGR_EnterMasterMode();
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFERMGMT_INIT_EnterSlaveMode
 *---------------------------------------------------------------------------------
 * PURPOSE: make BUFFERMGMT to enter slave mode.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   The function will initialize lower level driver and set local variables
 *          to default state. It should be called *ONCE ONLY* during stacking.
 *---------------------------------------------------------------------------------*/
void BUFFERMGMT_INIT_EnterSlaveMode(void)
{
    BUFFER_MGR_EnterSlaveMode();
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFERMGMT_INIT_EnterTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: make BUFFERMGMT to enter transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *---------------------------------------------------------------------------------*/
void BUFFERMGMT_INIT_EnterTransitionMode(void)
{
    BUFFER_MGR_EnterTransitionMode();
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFERMGMT_INIT_SetTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: sets BUFFERMGMT to temporary transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *---------------------------------------------------------------------------------*/
void BUFFERMGMT_INIT_SetTransitionMode(void)
{
    BUFFER_MGR_SetTransitionMode();
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME - BUFFERMGMT_INIT_HandleHotInsertion
 *---------------------------------------------------------------------------------
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
 *---------------------------------------------------------------------------------*/
void BUFFERMGMT_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    return;
} /* End of BUFFERMGMT_INIT_HandleHotInsertion */

/*---------------------------------------------------------------------------------
 * FUNCTION NAME - BUFFERMGMT_INIT_HandleHotRemoval
 *---------------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 *---------------------------------------------------------------------------------*/
void BUFFERMGMT_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    return;
} /* End of BUFFERMGMT_INIT_HandleHotRemoval */
