/* MODULE NAME:  app_protocol_proc_comm.c
 * PURPOSE:
 *    For app protocol process
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    11/13/2007 - Squid Ro, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include "app_protocol_proc_comm.h"

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
static L_THREADGRP_Handle_T app_pro_group_tg_handle;   /* the handle of thread group */

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : APP_PROTOCOL_PROC_COMM_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the resource which is common for all CSCs in app protocol process.
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
BOOL_T APP_PROTOCOL_PROC_COMM_InitiateProcessResource(void)
{
    BOOL_T ret=TRUE;

    app_pro_group_tg_handle = L_THREADGRP_Create();


    if(app_pro_group_tg_handle==NULL)
    {
        printf("%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        ret=FALSE;
    }

    return ret;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : APP_PROTOCOL_PROC_COMM_GetAppProtocolGroupHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for app protocol group.
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
L_THREADGRP_Handle_T APP_PROTOCOL_PROC_COMM_GetAppProtocolGroupHandle(void)
{
    return app_pro_group_tg_handle;
}

/* LOCAL SUBPROGRAM BODIES
 */

