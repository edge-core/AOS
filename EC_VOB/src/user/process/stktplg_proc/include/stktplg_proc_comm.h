/* MODULE NAME:  stktplg_proc_comm.h
 * PURPOSE:
 *    This file defines APIs for getting common resource of this process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    7/11/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef STKTPLG_PROC_COMM_H
#define STKTPLG_PROC_COMM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "l_threadgrp.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : STKTPLG_PROC_COMM_InitiateProcessResources
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the resource which is common for all CSCs in stktplg process.
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
BOOL_T STKTPLG_PROC_COMM_InitiateProcessResources(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : STKTPLG_PROC_COMM_GetStktplgGroupTGHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for STKTPLG Group.
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
L_THREADGRP_Handle_T STKTPLG_PROC_COMM_GetStktplgGroupTGHandle(void);

#endif    /* End of STKTPLG_PROC_COMM_H */

