/* MODULE NAME:  cli_proc_comm.h
 * PURPOSE:
 *    This file defines APIs for getting common resource of this process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    5/16/2007 -  Charlie Chen, Created
 *    06/02/2007 - Rich Lee    , modified for CLI process
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef cli_PROC_COMM_H
#define cli_PROC_COMM_H

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
 * ROUTINE NAME : CLI_PROC_COMM_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the resource which is common for all CSCs in cli process.
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
BOOL_T CLI_PROC_COMM_InitiateProcessResource(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CLI_PROC_COMM_GetCLIGroupHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for cli group.
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
L_THREADGRP_Handle_T CLI_PROC_COMM_GetCLIGroupHandle(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CLI_PROC_COMM_GetTELNET_GROUPTGHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for telnet daemon group.
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
L_THREADGRP_Handle_T CLI_PROC_COMM_GetTELNET_GROUPTGHandle(void);
                     
/*------------------------------------------------------------------------------
 * ROUTINE NAME : CLI_PROC_COMM_GetSSH_GROUPTGHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for ssh shell group.
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
L_THREADGRP_Handle_T CLI_PROC_COMM_GetSSH_GROUPTGHandle(void);

#endif    /* End of XXX_PROC_COMM_H */

