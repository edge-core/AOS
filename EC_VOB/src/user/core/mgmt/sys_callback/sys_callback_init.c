/* MODULE NAME:  sys_callback_init.c
 * PURPOSE:
 *     Initiate SYS_CALLBACK.
 *
 * NOTES:
 *
 * HISTORY
 *    6/13/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include "sys_callback_init.h"
#include "sysrsc_mgr.h"
#include "sys_callback_mgr.h"
#include "sys_callback_om_private.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_INIT_InitiateSystemResources
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Initiate system resource for SYS_CALLBACK
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
void SYS_CALLBACK_INIT_InitiateSystemResources(void)
{
    SYS_CALLBACK_OM_InitiateSystemResources();
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)     
    SYS_CALLBACK_REFINED_OM_InitResource();
#endif
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_INIT_AttachSystemResources
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Attach system resource for SYS_CALLBACK in the context of the calling
 *      process.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
void SYS_CALLBACK_INIT_AttachSystemResources(void)
{
    SYS_CALLBACK_OM_AttachSystemResources();
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_INIT_GetShMemInfo
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Provide shared memory information for SYSRSC.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      segid_p  --  shared memory segment id
 *      seglen_p --  length of the shared memroy segment
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 *-------------------------------------------------------------------------
 */
void SYS_CALLBACK_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    SYS_CALLBACK_OM_GetShMemInfo(segid_p, seglen_p);
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_INIT_EnterMasterMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Let SYS_CALLBACK_INIT enter master mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_INIT_EnterMasterMode(void)
{
    SYS_CALLBACK_MGR_EnterMasterMode();
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_INIT_EnterSlaveMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Let SYS_CALLBACK_INIT enter slave mode.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_INIT_EnterSlaveMode(void)
{
    SYS_CALLBACK_MGR_EnterSlaveMode();
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_INIT_EnterTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Let SYS_CALLBACK_INIT enter transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_INIT_EnterTransitionMode(void)
{
    SYS_CALLBACK_MGR_EnterTransitionMode();
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_INIT_SetTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Set SYS_CALLBACK_INIT to temporary transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_INIT_SetTransitionMode(void)
{
    SYS_CALLBACK_MGR_SetTransitionMode();
}

/* LOCAL SUBPROGRAM BODIES
 */

