/* MODULE NAME:  vrrp_proc_comm.c
 * PURPOSE:
 *    This file defines APIs for getting common resource of VRRP process.
 *
 * NOTES:
 *
 * HISTORY
 *    3/28/2009 - Donny.Li     , Created
 *
 * Copyright(C)      Accton Corporation, 2009     

 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include "vrrp_proc_comm.h"
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
static L_THREADGRP_Handle_T vrrp_tg_handle;   /* the handle of thread group */

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  VRRP_PROC_COMM_InitiateProcessResources
 * PURPOSE:
 *    Initialize the resource which is common for all CSCs in VRRP process.
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
BOOL_T VRRP_PROC_COMM_InitiateProcessResources(void)
{
    BOOL_T ret=TRUE;

    vrrp_tg_handle = L_THREADGRP_Create();
    if(vrrp_tg_handle==NULL)
    {
        printf("%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        ret=FALSE;
    }
    return ret;
}

/* FUNCTION NAME:  VRRP_PROC_COMM_GetVrrpTGHandle
 * PURPOSE:
 *    Get the thread group handler for VRRP CSC Group.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    The VRRP thread group handle.
 * NOTES:
 *    None
 *
 */
L_THREADGRP_Handle_T VRRP_PROC_COMM_GetVrrpTGHandle(void)
{
    return vrrp_tg_handle;
}

/* LOCAL SUBPROGRAM BODIES
 */


