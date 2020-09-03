/* MODULE NAME:  mgmt_proc_comm.h
 * PURPOSE:
 *    This file defines APIs for getting common resource of this process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    5/16/2007 - Eli Lin, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef AUTH_PROTOCOL_PROC_COMM_H
#define AUTH_PROTOCOL_PROC_COMM_H

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
 * ROUTINE NAME : AUTH_PROTOCOL_PROC_COMM_Initiate_System_Resource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the system resource which is common for all CSCs in mgmt process.
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
BOOL_T AUTH_PROTOCOL_PROC_COMM_Initiate_System_Resource(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AUTH_PROTOCOL_PROC_COMM_GetThreadGrpHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for AUTH_PROTOCOL_PROC.
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
L_THREADGRP_Handle_T AUTH_PROTOCOL_PROC_COMM_GetAuthProtocolGroupTGHandle(void);

#endif    /* End of AUTH_PROTOCOL_PROC_COMM_H */

