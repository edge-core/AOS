/*-----------------------------------------------------------------------------
 * MODULE NAME: IP_SERVICE_PROC_COMM.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    The module provides APIs for CSCs within the IP_SERVICE process to get
 *    common resources of the IP_SERVICE process.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/07/11     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <stdio.h>
#include "sys_type.h"
#include "sys_cpnt.h"
#include "l_threadgrp.h"
#include "ip_service_proc_comm.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DEFINITIONS
 */

/* the handle of thread group
 */
static L_THREADGRP_Handle_T ip_service_group_tg_handle;


/* EXPORTED SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : IP_SERVICE_PROC_COMM_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initialize the system resource which is common for all CSCs in
 *           IP_SERVICE process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T IP_SERVICE_PROC_COMM_InitiateProcessResource(void)
{
    ip_service_group_tg_handle = L_THREADGRP_Create();
    if (ip_service_group_tg_handle == NULL)
    {
        printf("\n%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : IP_SERVICE_PROC_COMM_GetIpServiceGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for IP_SERVICE group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T IP_SERVICE_PROC_COMM_GetIpServiceGroupTGHandle(void)
{
    return ip_service_group_tg_handle;
}


/* LOCAL SUBPROGRAM BODIES
 */
