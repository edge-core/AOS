/* MODULE NAME:  ospf_proc_comm.c
 * PURPOSE:
 *    This file defines APIs for getting common resource of OSPF process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    11/27/2008 - Lin.Li , Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include "ospf_proc_comm.h"
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
static L_THREADGRP_Handle_T ospf_tg_handle;   /* the handle of thread group */

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  OSPF_PROC_COMM_InitiateProcessResources
 * PURPOSE:
 *    Initialize the resource which is common for all CSCs in OSPF process.
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
BOOL_T OSPF_PROC_COMM_InitiateProcessResources(void)
{
    BOOL_T ret=TRUE;

    ospf_tg_handle = L_THREADGRP_Create();
    if(ospf_tg_handle==NULL)
    {
        printf("%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        ret=FALSE;
    }
    return ret;
}

/* FUNCTION NAME:  OSPF_PROC_COMM_GetOspfTGHandle
 * PURPOSE:
 *    Get the thread group handler for OSPF CSC Group.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    The OSPF thread group handle.
 * NOTES:
 *    None
 *
 */
L_THREADGRP_Handle_T OSPF_PROC_COMM_GetOspfTGHandle(void)
{
    return ospf_tg_handle;
}

/* LOCAL SUBPROGRAM BODIES
 */


