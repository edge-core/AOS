/* MODULE NAME:  sys_mgmt_proc_comm.c
 * PURPOSE:
 *    This file defines APIs for getting common resource of this process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    7/07/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include "sys_mgmt_proc_comm.h"
#include "stdio.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static L_THREADGRP_Handle_T ui_mgr_group_tg_handle;
static L_THREADGRP_Handle_T sys_mgmt_mgr_group_tg_handle;
static L_THREADGRP_Handle_T cluster_mgr_group_tg_handle;

/* EXPORTED SUBPROGRAM BODIES
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
BOOL_T SYS_MGMT_PROC_COMM_InitiateProcessResource(void)
{
    BOOL_T ret=TRUE;

    ui_mgr_group_tg_handle = L_THREADGRP_Create();
    if(ui_mgr_group_tg_handle==NULL)
    {
        printf("%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        ret=FALSE;
    }
    
    sys_mgmt_mgr_group_tg_handle = L_THREADGRP_Create();
    if(sys_mgmt_mgr_group_tg_handle==NULL)
    {
        printf("%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        ret=FALSE;
    }
    
    cluster_mgr_group_tg_handle = L_THREADGRP_Create();
    if(cluster_mgr_group_tg_handle==NULL)
    {
        printf("%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        ret=FALSE;
    }
    
    return ret;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYS_MGMT_PROC_COMM_GetUI_MGR_GROUPTGHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for UI MGR Group.
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
L_THREADGRP_Handle_T SYS_MGMT_PROC_COMM_GetUI_MGR_GROUPTGHandle(void)
{
    return ui_mgr_group_tg_handle;
}

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
L_THREADGRP_Handle_T SYS_MGMT_PROC_COMM_Get_SYSMGMT_MGR_GROUPTGHandle(void)
{
    return sys_mgmt_mgr_group_tg_handle;
}

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
L_THREADGRP_Handle_T SYS_MGMT_PROC_COMM_GetClusterGroupTGHandle(void)
{
    return cluster_mgr_group_tg_handle;
}
/* LOCAL SUBPROGRAM BODIES
 */

