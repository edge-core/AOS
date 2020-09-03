/* MODULE NAME:  xfer_proc_comm.h
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
#ifndef XFER_PROC_COMM_H
#define XFER_PROC_COMM_H

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
 * ROUTINE NAME : XFER_PROC_COMM_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the resource which is common for all CSCs in xfer process.
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
BOOL_T XFER_PROC_COMM_InitiateProcessResource(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : XFER_PROC_COMM_GetXFER_GROUPTGHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for xfer group.
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
L_THREADGRP_Handle_T XFER_PROC_COMM_GetXFER_GROUPTGHandle(void);

#endif    /* End of XFER_PROC_COMM_H */

