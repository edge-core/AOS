/* MODULE NAME:  app_protocol_proc_comm.h
 * PURPOSE:
 *    For app protocol process
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    11/13/2007 - Squid Ro, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef APP_PROTOCOL_PROC_COMM_H
#define APP_PROTOCOL_PROC_COMM_H

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
 * ROUTINE NAME : APP_PROTOCOL_PROC_COMM_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the resource which is common for all CSCs in app protocol process.
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
BOOL_T APP_PROTOCOL_PROC_COMM_InitiateProcessResource(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : APP_PROTOCOL_PROC_COMM_GetAppProtocolGroupHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for app protocol group.
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
L_THREADGRP_Handle_T APP_PROTOCOL_PROC_COMM_GetAppProtocolGroupHandle(void);


#endif    /* End of CORE_UTIL_PROC_COMM_H */

