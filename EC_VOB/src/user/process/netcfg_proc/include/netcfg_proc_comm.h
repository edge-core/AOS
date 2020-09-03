/* MODULE NAME:  netcfg_proc_comm.h
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
#ifndef NETCFG_PROC_COMM_H
#define NETCFG_PROC_COMM_H

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
 * ROUTINE NAME : NETCFG_PROC_COMM_InitiateProcessResources
 *------------------------------------------------------------------------------
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
 *------------------------------------------------------------------------------
 */
BOOL_T NETCFG_PROC_COMM_InitiateProcessResources(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PROC_COMM_GetNetcfgTGHandle
 *------------------------------------------------------------------------------
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
 *------------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T NETCFG_PROC_COMM_GetNetcfgTGHandle(void);

#endif    /* End of NETCFGPROC_COMM_H */
