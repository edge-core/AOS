/* MODULE NAME:  web_proc_comm.h
 * PURPOSE:
 *    This file defines APIs for getting common resource of this process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    7/08/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef WEB_PROC_COMM_H
#define WEB_PROC_COMM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "l_threadgrp.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : WEB_PROC_COMM_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the resource which is common for all CSCs in web process.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  -- Success
 *    FALSE -- Fail
 * NOTES:
 *    None
 *------------------------------------------------------------------------------
 */
BOOL_T WEB_PROC_COMM_InitiateProcessResource(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : WEB_PROC_COMM_GetWEBGROUPTGHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for web group.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  -- Success
 *    FALSE -- Fail
 * NOTES:
 *    None
 *------------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T WEB_PROC_COMM_GetWEB_GROUPTGHandle(void);

#endif    /* End of XFER_PROC_COMM_H */

