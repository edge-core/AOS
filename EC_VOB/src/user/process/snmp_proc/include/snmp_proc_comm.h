/* MODULE NAME:  snmp_proc_comm.h
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
#ifndef SNMP_PROC_COMM_H
#define SNMP_PROC_COMM_H

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
 * ROUTINE NAME : SNMP_PROC_COMM_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the system resource which is common for all CSCs in SNMP process.
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
BOOL_T SNMP_PROC_COMM_InitiateProcessResource(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SNMP_PROC_COMM_GetSNMPGroupTGHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for SNMP_PROC.
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
L_THREADGRP_Handle_T SNMP_PROC_COMM_GetSnmpGroupTGHandle(void);
#endif    /* End of SNMP_PROC_COMM_H */


