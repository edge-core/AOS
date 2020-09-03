/*-----------------------------------------------------------------------------
 * MODULE NAME: L2_L4_PROC_COMM.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    The module provides APIs for all CSCs in IP_SERVICE process to get common
 *    resources of the process.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/05/17     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef IP_SERVICE_PROC_COMM_H
#define IP_SERVICE_PROC_COMM_H


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
 * FUNCTION NAME : IP_SERVICE_PROC_COMM_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initialize the system resource which is common for all CSCs in
 *           IP_SERVICE process.
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
BOOL_T IP_SERVICE_PROC_COMM_InitiateProcessResource(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : IP_SERVICE_PROC_COMM_GetIpServiceGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for IP_SERVICE group.
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
L_THREADGRP_Handle_T IP_SERVICE_PROC_COMM_GetIpServiceGroupTGHandle(void);


#endif /* #ifndef IP_SERVICE_PROC_COMM_H */
