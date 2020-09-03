/* MODULE NAME:  cli_proc_comm.c
 * PURPOSE:
 *    This file defines APIs for getting common resource of this process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    5/16/2007 - Charlie Chen, Created
 *    06/01/2007 - Rich Lee,    modified for CLI process
 * Copyright(C)      Accton Corporation, 2007
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>

#include "cli_proc_comm.h"

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
static L_THREADGRP_Handle_T cli_group_tg_handle;   /* the handle of cli thread group */
static L_THREADGRP_Handle_T telnet_group_tg_handle;   /* the handle of telnet daemon thread group */
static L_THREADGRP_Handle_T ssh_group_tg_handle;   /* the handle of telnet shell thread group */

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : CLI_PROC_COMM_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the resource which is common for all CSCs in cli process.
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
BOOL_T CLI_PROC_COMM_InitiateProcessResource(void)
{
    BOOL_T ret=TRUE;

    cli_group_tg_handle = L_THREADGRP_Create();
    if(cli_group_tg_handle==NULL)
    {
        printf("%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        ret=FALSE;
    }
    
    telnet_group_tg_handle = L_THREADGRP_Create();
    if(telnet_group_tg_handle==NULL)
    {
        printf("%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        ret=FALSE;
    }
    
    ssh_group_tg_handle = L_THREADGRP_Create();
    if(ssh_group_tg_handle==NULL)
    {
        printf("%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        ret=FALSE;
    }
    return ret;    
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CLI_PROC_COMM_GetCLIGroupHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for cli group.
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
L_THREADGRP_Handle_T CLI_PROC_COMM_GetCLIGroupHandle(void)
{
    return cli_group_tg_handle;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CLI_PROC_COMM_GetTELNET_GROUPTGHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for telnet daemon group.
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
L_THREADGRP_Handle_T CLI_PROC_COMM_GetTELNET_GROUPTGHandle(void)
{                    
    return telnet_group_tg_handle;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CLI_PROC_COMM_GetSSH_GROUPTGHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for telnet shell group.
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
L_THREADGRP_Handle_T CLI_PROC_COMM_GetSSH_GROUPTGHandle(void)
{
    return ssh_group_tg_handle;
}
/* LOCAL SUBPROGRAM BODIES
 */

