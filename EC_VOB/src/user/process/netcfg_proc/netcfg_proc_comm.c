/* MODULE NAME:  netcfg_proc_comm.c
 * PURPOSE:
 *    This file defines APIs for getting common resource of NETCFG process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    05/16/2007 - Charlie Chen , Created
 *    07/10/2007 - Max Chen     , Modified for netcfg
 *
 * Copyright(C)      Accton Corporation, 2007
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include "netcfg_proc_comm.h"
#include <stdio.h>

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
static L_THREADGRP_Handle_T netcfg_tg_handle;   /* the handle of thread group */

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  NETCFG_PROC_COMM_InitiateProcessResources
 * PURPOSE:
 *    Initialize the resource which is common for all CSCs in NETCFG process.
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
 *
 */
BOOL_T NETCFG_PROC_COMM_InitiateProcessResources(void)
{
    BOOL_T ret=TRUE;

    netcfg_tg_handle = L_THREADGRP_Create();
    if(netcfg_tg_handle==NULL)
    {
        printf("%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        ret=FALSE;
    }
    return ret;
}

/* FUNCTION NAME:  NETCFG_PROC_COMM_GetNetcfgTGHandle
 * PURPOSE:
 *    Get the thread group handler for NETCFG CSC Group.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    The NETCFG thread group handle.
 * NOTES:
 *    None
 *
 */
L_THREADGRP_Handle_T NETCFG_PROC_COMM_GetNetcfgTGHandle(void)
{
    return netcfg_tg_handle;
}

/* LOCAL SUBPROGRAM BODIES
 */
