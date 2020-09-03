/* MODULE NAME:  sys_mgmt_proc_comm.h
 * PURPOSE:
 *    This file defines APIs for getting common resource of this process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    7/08/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef SYS_MGMT_PROC_COMM_H
#define SYS_MGMT_PROC_COMM_H

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
 * ROUTINE NAME : SYS_MGMT_PROC_COMM_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the resource which is common for all CSCs in sys mgmt process.
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
BOOL_T SYS_MGMT_PROC_COMM_InitiateProcessResource(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYS_MGMT_PROC_COMM_GetUI_MGR_GROUPTGHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for ui mgr group.
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
L_THREADGRP_Handle_T SYS_MGMT_PROC_COMM_GetUI_MGR_GROUPTGHandle(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYS_MGMT_PROC_COMM_Get_SYSMGMT_MGR_GROUPTGHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for SYSMGMT Group.
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
L_THREADGRP_Handle_T SYS_MGMT_PROC_COMM_Get_SYSMGMT_MGR_GROUPTGHandle(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYS_MGMT_PROC_COMM_GetClusterGroupTGHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for SYSMGMT Group.
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
L_THREADGRP_Handle_T SYS_MGMT_PROC_COMM_GetClusterGroupTGHandle(void);

#endif    /* End of XFER_PROC_COMM_H */

