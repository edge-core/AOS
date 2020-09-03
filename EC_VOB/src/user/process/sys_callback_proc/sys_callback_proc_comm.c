/*-----------------------------------------------------------------------------
 * MODULE NAME: SYS_CALLBACK_PROC_COMM.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    The module provides APIs for CSCs within the SYS_CALLBACK process to get common
 *    resources of the SYS_CALLBACK process.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2013/05/21     --- Jimi, Create
 *
 * Copyright(C)      EdgeCore Corporation, 2013
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <stdio.h>
#include "sys_type.h"
#include "l_threadgrp.h"
#include "sys_cpnt.h"
//#include "l2_l4_proc_comm.h"


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

static L_THREADGRP_Handle_T sys_callback_group_tg_handle;

/* EXPORTED SUBPROGRAM BODIES
 */

/* ------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_PROC_COMM_InitiateProcessResources
 * ------------------------------------------------------------------------ 
 * PURPOSE: Initialize the system resource which is common for all CSCs in
 *          SYS_CALLBACK process.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : TRUE  -- Success
 *          FALSE -- Fail
 * NOTES  : none
 * ------------------------------------------------------------------------ 
 */ 
BOOL_T SYS_CALLBACK_PROC_COMM_InitiateProcessResources(void)
{
	sys_callback_group_tg_handle = L_THREADGRP_Create();
	if (sys_callback_group_tg_handle == NULL)
	{
		printf("\n%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
		return FALSE;
	}

    return TRUE;
}


/* ------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_PROC_COMM_GetSyscallbackGroupTGHandle
 * ------------------------------------------------------------------------ 
 * PURPOSE: Get the thread group handler for SYS_CALLBACK group.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : TRUE  -- Success
 *          FALSE -- Fail
 * NOTES  : none
 * ------------------------------------------------------------------------ 
 */  
L_THREADGRP_Handle_T SYS_CALLBACK_PROC_COMM_GetSyscallbackGroupTGHandle(void)
{
    return sys_callback_group_tg_handle;
}


/* LOCAL SUBPROGRAM BODIES
 */

