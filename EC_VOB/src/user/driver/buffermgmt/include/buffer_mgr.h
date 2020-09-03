/* Module Name: BUFFER_MGR.h
 * Purpose:
 *  This module is responsible for the control of landing buffer
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

#ifndef BUFFER_MGR_H
#define BUFFER_MGR_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"


/* LOCAL DATATYPE DECLARATION
 */

/* LOCAL NAMEING CONSTANT DECLARATION
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFER_MGR_InitiateSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Initiate system resource for BUFFERMGMT
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void  BUFFER_MGR_InitiateSystemResources(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFER_MGR_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for BUFFERMGMT in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void BUFFER_MGR_AttachSystemResources(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFER_MGR_EnterMasterMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter master mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void BUFFER_MGR_EnterMasterMode (void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFER_MGR_EnterSlaveMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter slave mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void BUFFER_MGR_EnterSlaveMode (void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFER_MGR_SetTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Sets to temporary transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void BUFFER_MGR_SetTransitionMode (void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFER_MGR_EnterTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void BUFFER_MGR_EnterTransitionMode (void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: BUFFER_MGR_ProvisionComplete
 *---------------------------------------------------------------------------------
 * PURPOSE: All provision commands are settle down.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void BUFFER_MGR_ProvisionComplete(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME - BUFFER_MGR_Allocate
 *---------------------------------------------------------------------------------
 * FUNCTION: allocate landing buffer
 * INPUT   : none
 * OUTPUT  : none
 * RETURN  : the start address of the buffer
 * NOTE    : if fail return NULL
 *---------------------------------------------------------------------------------*/
void *BUFFER_MGR_Allocate(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME - BUFFER_MGR_Free
 *---------------------------------------------------------------------------------
 * FUNCTION: free landing buffer
 * INPUT   : the start address of the buffer
 * OUTPUT  : None
 * RETURN  : TRUE(Success) ;FALSE
 * NOTE    :
 *---------------------------------------------------------------------------------*/
BOOL_T BUFFER_MGR_Free(void *buf_p);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME - BUFFER_MGR_GetPtr
 *---------------------------------------------------------------------------------
 * FUNCTION: convert offset to pointer
 * INPUT   : the offset of the buffer
 * OUTPUT  : None
 * RETURN  : the pointer converted from the specified offset.
 * NOTE    :
 *---------------------------------------------------------------------------------*/
void *BUFFER_MGR_GetPtr(I32_T offset);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME - BUFFER_MGR_GetOffset
 *---------------------------------------------------------------------------------
 * FUNCTION: convert buffer pointer to offset
 * INPUT   : the pointer of the buffer
 * OUTPUT  : None
 * RETURN  : the offset converted from the specified buffer.
 * NOTE    :
 *---------------------------------------------------------------------------------*/
I32_T BUFFER_MGR_GetOffset(void *buf_p);

#endif  /* BUFFER_MGR_H */
