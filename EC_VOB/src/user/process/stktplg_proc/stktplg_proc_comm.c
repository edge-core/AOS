/* MODULE NAME:  stktplg_proc_comm.c
 * PURPOSE:
 *    This file defines APIs for getting common resource of this process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    7/11/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>

#include "stktplg_proc_comm.h"
#include "l_threadgrp.h"

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
static L_THREADGRP_Handle_T stktplg_group_tg_handle;   /* the handle of thread group for stktplg group*/

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : STKTPLG_PROC_COMM_InitiateProcessResources
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the resource which is common for all CSCs in stktplg process.
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
BOOL_T STKTPLG_PROC_COMM_InitiateProcessResources(void)
{
    BOOL_T ret=TRUE;

    stktplg_group_tg_handle = L_THREADGRP_Create();
    if(stktplg_group_tg_handle==NULL)
    {
        printf("%s(): L_THREADGRP_Create fail.\r\n", __FUNCTION__);
        ret=FALSE;
    }
    return ret;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : STKTPLG_PROC_COMM_GetStktplgGroupTGHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for STKTPLG Group.
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
L_THREADGRP_Handle_T STKTPLG_PROC_COMM_GetStktplgGroupTGHandle(void)
{
    return stktplg_group_tg_handle;
}

/* LOCAL SUBPROGRAM BODIES
 */

