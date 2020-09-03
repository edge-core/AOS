/*-----------------------------------------------------------------------------
 * MODULE NAME: SYS_CALLBACK_PROC_COMM.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    The module provides APIs for all CSCs in SYS_CALLBACK process to get common
 *    resources of the process.
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

#ifndef SYS_CALLBACK_PROC_COMM_H
#define SYS_CALLBACK_PROC_COMM_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "l_threadgrp.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
BOOL_T SYS_CALLBACK_PROC_COMM_InitiateProcessResources(void);


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
L_THREADGRP_Handle_T SYS_CALLBACK_PROC_COMM_GetSyscallbackGroupTGHandle(void);

#endif /* #ifndef SYS_CALLBACK_PROC_COMM_H */


