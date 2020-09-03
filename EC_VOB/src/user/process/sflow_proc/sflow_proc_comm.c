/* MODULE NAME:  sflow_proc_comm.c
 * PURPOSE:
 *    This file defines APIs for getting common resource of SFLOW process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    09/08/2009 - Nelson Dai, Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include "sflow_proc_comm.h"
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
static L_THREADGRP_Handle_T sflow_tg_handle;   /* the handle of thread group */

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  SFLOW_PROC_COMM_InitiateProcessResources
 * PURPOSE:
 *    Initialize the resource which is common for all CSCs in SFLOW process.
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
BOOL_T SFLOW_PROC_COMM_InitiateProcessResources(void)
{
    BOOL_T ret = TRUE;

    sflow_tg_handle = L_THREADGRP_Create();
    if (sflow_tg_handle == NULL)
    {
        printf("%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        ret = FALSE;
    }
    return ret;
}

/* FUNCTION NAME:  SFLOW_PROC_COMM_GetSflowTGHandle
 * PURPOSE:
 *    Get the thread group handler for SFLOW CSC Group.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    The SFLOW thread group handle.
 * NOTES:
 *    None
 *
 */
L_THREADGRP_Handle_T SFLOW_PROC_COMM_GetSflowTGHandle(void)
{
    return sflow_tg_handle;
}

/* LOCAL SUBPROGRAM BODIES
 */

