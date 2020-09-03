/*-----------------------------------------------------------------------------
 * MODULE NAME: Driver_PROC_COMM.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    The module provides APIs for all CSCs in Driver process to get common
 *    resources of the process.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/07/13     --- Echo, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef DRIVER_PROC_COMM_H
#define DRIVER_PROC_COMM_H


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

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : Driver_PROC_COMM_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initialize the system resource which is common for all CSCs in
 *           Driver process.
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
BOOL_T DRIVER_PROC_COMM_InitiateProcessResource(void);

#endif /* #ifndef DRIVER_PROC_COMM_H */
