/* MODULE NAME:  xfer_proc_comm.c
 * PURPOSE:
 *    This file defines APIs for getting common resource of this process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    7/07/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include "xfer_proc_comm.h"

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
static L_THREADGRP_Handle_T xfer_group_tg_handle;   /* the handle of thread group */

/* EXPORTED SUBPROGRAM BODIES
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
BOOL_T XFER_PROC_COMM_InitiateProcessResource(void)
{
    BOOL_T ret=TRUE;

    xfer_group_tg_handle = L_THREADGRP_Create();
    if(xfer_group_tg_handle==NULL)
    {
        printf("%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        ret=FALSE;
    }
    return ret;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : XFER_PROC_COMM_GetXFER_GROUPTGHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for xfer Group.
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
L_THREADGRP_Handle_T XFER_PROC_COMM_GetXFER_GROUPTGHandle(void)
{
    return xfer_group_tg_handle;
}

/* LOCAL SUBPROGRAM BODIES
 */

