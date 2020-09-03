/* MODULE NAME:  ospf_proc_comm.h
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
#ifndef OSPF_PROC_COMM_H
#define OSPF_PROC_COMM_H

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
 * ROUTINE NAME : OSPF_PROC_COMM_InitiateProcessResources
 *------------------------------------------------------------------------------
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
 *------------------------------------------------------------------------------
 */
    BOOL_T OSPF_PROC_COMM_InitiateProcessResources(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : OSPF_PROC_COMM_GetOspfTGHandle
 *------------------------------------------------------------------------------
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
 *------------------------------------------------------------------------------
 */
    L_THREADGRP_Handle_T OSPF_PROC_COMM_GetOspfTGHandle(void);

#endif    /* End of OSPF_PROC_COMM_H */


