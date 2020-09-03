/* MODULE NAME:  sys_callback_init.h
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
#ifndef SYS_CALLBACK_INIT_H
#define SYS_CALLBACK_INIT_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sysrsc_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
void SYS_CALLBACK_INIT_InitiateSystemResources(void);

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
void SYS_CALLBACK_INIT_AttachSystemResources(void);

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
void SYS_CALLBACK_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_INIT_EnterMasterMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Let SYS_CALLBACK_INIT enter master mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_INIT_EnterMasterMode(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_INIT_EnterSlaveMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Let SYS_CALLBACK_INIT enter slave mode.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_INIT_EnterSlaveMode(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_INIT_EnterTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Let SYS_CALLBACK_INIT enter transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_INIT_EnterTransitionMode(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_INIT_SetTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Set SYS_CALLBACK_INIT to temporary transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_INIT_SetTransitionMode(void);

#endif    /* End of SYS_CALLBACK_INIT_H */

