/* MODULE NAME:  auth_protocol_proc_comm.c
 * PURPOSE:
 *    This file defines APIs for getting common resource of this process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    5/16/2007 - Eli Lin, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include "auth_protocol_proc_comm.h"
#include "sysfun.h"

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
static L_THREADGRP_Handle_T tg_handle;   /* the handle of thread group */

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : AUTH_PROTOCOL_PROC_COMM_Initiate_System_Resource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the system resource which is common for all CSCs in auth_protocol process.
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
BOOL_T AUTH_PROTOCOL_PROC_COMM_Initiate_System_Resource(void)
{
    BOOL_T ret=TRUE;

    tg_handle = L_THREADGRP_Create();
    if(tg_handle==NULL)
    {
        SYSFUN_Debug_Printf("%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        ret=FALSE;
    }
    return ret;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AUTH_PROTOCOL_PROC_COMM_GetThreadGrpHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for AUTH_PROTOCOL_PROC.
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
L_THREADGRP_Handle_T AUTH_PROTOCOL_PROC_COMM_GetAuthProtocolGroupTGHandle(void)
{
    return tg_handle;
}

/* LOCAL SUBPROGRAM BODIES
 */

