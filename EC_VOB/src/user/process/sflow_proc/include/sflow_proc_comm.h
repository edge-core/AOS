/* MODULE NAME:  sflow_proc_comm.h
 * PURPOSE:
 *    This file defines APIs for getting common resource of SFLOW process.
 *
 * NOTES:
 *
 * REASON:
 * Descpimtion:
 * HISTORY
 *    05/06/2008 - Hongliang , Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */
#ifndef SFLOW_PROC_COMM_H
#define SFLOW_PROC_COMM_H

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
 * ROUTINE NAME : SFLOW_PROC_COMM_InitiateProcessResources
 *------------------------------------------------------------------------------
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
 *------------------------------------------------------------------------------
 */
    BOOL_T SFLOW_PROC_COMM_InitiateProcessResources(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SFLOW_PROC_COMM_GetSflowTGHandle
 *------------------------------------------------------------------------------
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
 *------------------------------------------------------------------------------
 */
    L_THREADGRP_Handle_T SFLOW_PROC_COMM_GetSflowTGHandle(void);

#endif    /* End of SFLOW_PROC_COMM_H */

