/* Module Name: TRACEROUTE_PMGR.C
 * Purpose:
 *    Implements the APIs for IPCs with TRACEROUTE MGR.
 *
 * Notes:
 *    None.
 * History:
 *      Date        --  Modifier,   Reason
 *      2007/12/19  --  Peter Yu,   Create
 *
 * Copyright(C)      Accton Corporation, 2007
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sysfun.h"
#include "traceroute_mgr.h"
#include "traceroute_pmgr.h"
#include "traceroute_type.h"

/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DEFINITIONS
 */
static SYSFUN_MsgQ_T traceroute_ipcmsgq_handle;


/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: TRACEROUTE_PMGR_InitiateProcessResources
 * PURPOSE:
 *          Initiate resources for TRACEROUTE_PMGR in the calling process.
 * INPUT:
 *          None.
 * OUTPUT:
 *          None.
 * RETURN:
 *          TRUE    -- Success
 *          FALSE   -- Fail
 * NOTES:
 *          None.
 */
BOOL_T TRACEROUTE_PMGR_InitiateProcessResources(void)
{
    /* get the ipc message queues for TRACEROUTE MGR
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_APP_PROTOCOL_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &traceroute_ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\n%s(): L_IPCMSGQ_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
} /* End of TRACEROUTE_PMGR_InitiateProcessResources */

/* FUNCTION NAME: TRACEROUTE_PMGR_SetCtlEntryByField
 * PURPOSE:
 *          Set only the field of the entry.
 * INPUT:
 *          ctl_entry_p -- the pointer of the specified ctl entry.
 *          field       -- field.
 * OUTPUT:
 *          None.
 * RETURN:
 *          TRACEROUTE_TYPE_OK
 *          TRACEROUTE_TYPE_FAIL
 *          TRACEROUTE_TYPE_INVALID_ARG
 * NOTES:
 *          1. set only the field of the entry.
 */
UI32_T TRACEROUTE_PMGR_SetCtlEntryByField(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p, TRACEROUTE_TYPE_CtlEntryField_T field)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRACEROUTE_MGR_GET_MSG_SIZE(ctl_entry_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRACEROUTE_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRACEROUTE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRACEROUTE_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRACEROUTE_MGR_IPCCMD_SETCTLENTRBYFIELD;

    memcpy(&msg_p->data.ctl_entry_ui32.ctl_entry, ctl_entry_p, sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));
    msg_p->data.ctl_entry_ui32.ui32 = field;

    if (SYSFUN_SendRequestMsg(traceroute_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return TRACEROUTE_TYPE_FAIL;
    }
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME: TRACEROUTE_PMGR_SetTraceRouteCtlEntry
 * PURPOSE:
 *          To create / destroy contrl entry
 * INPUT:
 *          ctl_entry_p->trace_route_ctl_owner_index       -- The owner index of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_owner_index_len   -- The length of the owner index.
 *          ctl_entry_p->trace_route_ctl_test_name         -- The test name of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_test_name_len     -- The length of the test name.
 *          ctl_entry_p->trace_route_ctl_rowstatus         -- The row status of the control entry.
 *          ctl_entry_p->trace_route_ctl_target_address    -- The target address of the traceroute operation.
 *          ctl_entry_p->trace_route_ctl_admin_status      -- The admin status of the traceroute entry.
 * OUTPUT:
 *          None
 * RETURN:
 *          TRACEROUTE_TYPE_OK
 *          TRACEROUTE_TYPE_INVALID_ARG
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          1. Currently, only CAG and Destroy is supported.
 *             Operation for CAW and NotInService is not allowed.
 *          2. Currentlt, we only permit the setting of row_status, target_address and admin_status
 */
UI32_T TRACEROUTE_PMGR_SetTraceRouteCtlEntry(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = TRACEROUTE_MGR_GET_MSG_SIZE(ctl_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TRACEROUTE_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_TRACEROUTE;
    msgbuf_p->msg_size = msg_size;

    msg_p = (TRACEROUTE_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = TRACEROUTE_MGR_IPCCMD_SETTRACEROUTECTLENTRY;

    memcpy(&msg_p->data.ctl_entry, ctl_entry_p, sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));

    if (SYSFUN_SendRequestMsg(traceroute_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return TRACEROUTE_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME: TRACEROUTE_PMGR_SetTraceRouteCtlAdminStatus
 * PURPOSE: 
 *          To enable or disable traceroute control entry
 * INPUT:  
 *          ctl_entry_p->trace_route_ctl_owner_index       -- The owner index of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_owner_index_len   -- The length of the owner index.
 *          ctl_entry_p->trace_route_ctl_test_name         -- The test name of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_test_name_len     -- The length of the test name.
 *          ctl_entry_p->trace_route_ctl_admin_status      -- The admin status of the traceroute entry.
 * OUTPUT:  
 *          None
 * RETURN: 
 *          TRACEROUTE_TYPE_OK
 *          TRACEROUTE_TYPE_INVALID_ARG
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          1.This API is only used by "set by filed", not "set by record".
 */ 
UI32_T TRACEROUTE_PMGR_SetTraceRouteCtlAdminStatus(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p)
{
    return TRACEROUTE_PMGR_SetCtlEntryByField(ctl_entry_p, TRACEROUTE_TYPE_CTLENTRYFIELD_ADMIN_STATUS);
}

/* FUNCTION NAME: TRACEROUTE_PMGR_SetTraceRouteCtlRowStatus
 * PURPOSE: 
 *          To set row status field for traceroute control entry
 * INPUT:  
 *          ctl_entry_p->trace_route_ctl_owner_index       -- The owner index of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_owner_index_len   -- The length of the owner index.
 *          ctl_entry_p->trace_route_ctl_test_name         -- The test name of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_test_name_len     -- The length of the test name.
 *          ctl_entry_p->trace_route_ctl_rowstatus         -- The row status of the control entry.
 * OUTPUT:  
 *          None
 * RETURN: 
 *          TRACEROUTE_TYPE_OK
 *          TRACEROUTE_TYPE_INVALID_ARG
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 */ 
UI32_T  TRACEROUTE_PMGR_SetTraceRouteCtlRowStatus(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p)
{
    return TRACEROUTE_PMGR_SetCtlEntryByField(ctl_entry_p, TRACEROUTE_TYPE_CTLENTRYFIELD_ROWSTATUS);
}

/* FUNCTION NAME: TRACEROUTE_PMGR_SetTraceRouteCtlTargetAddress
 * PURPOSE: 
 *          To set the target address field for traceroute control entry
 * INPUT:  
 *          ctl_entry_p->trace_route_ctl_owner_index       -- The owner index of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_owner_index_len   -- The length of the owner index.
 *          ctl_entry_p->trace_route_ctl_test_name         -- The test name of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_test_name_len     -- The length of the test name.
 *          ctl_entry_p->trace_route_ctl_target_address    -- The target address of the traceroute operation.
 * OUTPUT:  
 *          None
 * RETURN: 
 *          TRACEROUTE_TYPE_OK
 *          TRACEROUTE_TYPE_INVALID_ARG
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          1. We can not change the target address for the specified index when admin_status is enabled.
 *          2. Currently we do not support the domain name query of the target address.
 */
UI32_T  TRACEROUTE_PMGR_SetTraceRouteCtlTargetAddress(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p)
{
    return TRACEROUTE_PMGR_SetCtlEntryByField(ctl_entry_p, TRACEROUTE_TYPE_CTLENTRYFIELD_TARGET_ADDRESS);
}
