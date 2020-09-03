/* MODULE NAME:  core_util_proc_comm.c
 * PURPOSE:
 *    For core util process
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    7/7/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include "core_util_proc_comm.h"

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
static L_THREADGRP_Handle_T utility_group_tg_handle;   /* the handle of thread group */
static L_THREADGRP_Handle_T cfgdb_group_tg_handle;   /* the handle of thread group */

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : CORE_UTIL_PROC_COMM_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the resource which is common for all CSCs in cfgdb process.
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
BOOL_T CORE_UTIL_PROC_COMM_InitiateProcessResource(void)
{
    BOOL_T ret=TRUE;

    utility_group_tg_handle = L_THREADGRP_Create();
    
    
    if(utility_group_tg_handle==NULL)
    {
        printf("%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        ret=FALSE;
    }
    
    cfgdb_group_tg_handle = L_THREADGRP_Create();
    
    
    if(cfgdb_group_tg_handle==NULL)
    {
        printf("%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        ret=FALSE;
    }
    return ret;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CORE_UTIL_PROC_COMM_Getutility_groupTGHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for EH Group.
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
L_THREADGRP_Handle_T CORE_UTIL_PROC_COMM_GetUTILITY_GROUPTGHandle(void)
{
    return utility_group_tg_handle;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CORE_UTIL_PROC_COMM_GetCFGDB_GROUPTGHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for CFGDB Group.
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
L_THREADGRP_Handle_T CORE_UTIL_PROC_COMM_GetCFGDB_GROUPTGHandle(void)
{
    return cfgdb_group_tg_handle;
}
/* LOCAL SUBPROGRAM BODIES
 */

