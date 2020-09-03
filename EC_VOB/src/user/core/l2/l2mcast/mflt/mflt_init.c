/*-----------------------------------------------------------------------------
 * Module   : mflt_init.c
 *-----------------------------------------------------------------------------
 * PURPOSE  : Initiate the system resources.
 *-----------------------------------------------------------------------------
 * NOTES    :
 *
 *-----------------------------------------------------------------------------
 * HISTORY  : 10/22/2001 - Aaron Chuang, Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "mflt_init.h"
#include "mflt_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* MACRO FUNCTIONS DECLARACTION
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_INIT_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to initialize the multicast filtering module.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MFLT_INIT_InitiateProcessResources(void)
{
    /* initialize mflt module */
    if (!MFLT_MGR_InitiateProcessResource())
    {
        SYSFUN_Debug_Printf("\r\nMFLT_InitiateProcessResource 56 Failed ");
        while (1);
    }
}   /* End of MFLT_INIT_InitiateProcessResources() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_INIT_Create_InterCSC_Relation
 *-------------------------------------------------------------------------
 * PURPOSE : This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MFLT_INIT_Create_InterCSC_Relation(void)
{
    MFLT_MGR_Create_InterCSC_Relation();
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_INIT_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE : The function enters transition mode and free all MFLT
 *           resources and resets database to factory default.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MFLT_INIT_EnterTransitionMode(void)
{
    MFLT_MGR_EnterTransitionMode();
    return;
}   /* End of MFLT_INIT_EnterTransitionMode() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_INIT_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE : This function enters master mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MFLT_INIT_EnterMasterMode(void)
{
    MFLT_MGR_EnterMasterMode();
    return;
}   /* End of MFLT_INIT_EnterMasterMode() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_INIT_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE : The function enters slave mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MFLT_INIT_EnterSlaveMode(void)
{
    MFLT_MGR_EnterSlaveMode();
    return;
}   /* End of MFLT_INIT_EnterSlaveMode() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_INIT_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE : This function sets transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MFLT_INIT_SetTransitionMode(void)
{
    MFLT_MGR_SetTransitionMode();
    return;
}   /* End of MFLT_INIT_SetTransitionMode() */
