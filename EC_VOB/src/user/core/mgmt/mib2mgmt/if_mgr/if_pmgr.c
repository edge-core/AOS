/* -------------------------------------------------------------------------
 * FILE NAME - IF_PMGR.C
 * -------------------------------------------------------------------------
 * Purpose: For other CSCS to ipc MGR
 *          Written by:     Eli
 *          Date:           06/13/2007
 *
 * Modification History:
 * -------------------------------------------------------------------------
 * Copyright(C)                              ACCTON Technology Corp., 1998
 * -------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include "sysfun.h"
#include "sys_module.h"
#include "sysfun.h"
#include "l_mm.h"
#include "sys_bld.h"
#include "if_mgr.h"

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : TACACS_PMGR_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for TACACS_PMGR.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void IF_PMGR_Init(void)
{
    if(SYSFUN_GetMsgQ(SYS_BLD_SYSMGMT_GROUP_IPCMSGQ_KEY,
        SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
    }
}

/* FUNCTION NAME: IF_PMGR_GetIfNumber
 * PURPOSE: This funtion returns the number of network interfaces
 *          (regardless of their current state) present on this system.
 * INPUT:  None.
 * OUTPUT: if_number        - the total number of network interfaces presented on the system
 * RETURN: TRUE/FALSE
 * NOTES: For those interfaces which are not installed shall not be count into this number.
 */
BOOL_T IF_PMGR_GetIfNumber(UI32_T *if_number)
{
    const UI32_T msg_buf_size=(sizeof(((IF_MGR_IPCMsg_T *)0)->data.ui32_v)
        +sizeof(((IF_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    IF_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_IFMGR;
    /*request size
     */
    msg_p->msg_size = IF_MGR_MSGBUF_TYPE_SIZE;

    msg_data_p=(IF_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = IF_MGR_IPCCMD_GETIFNUMBER;

    /*respond size
     */
    resp_data_size=msg_buf_size ;

    /*assign input parameter
     */

    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output parameter
     */
    *if_number=msg_data_p->data.ui32_v;

    return msg_data_p->type.result_bool;
} /* end of IF_PMGR_GetIfNumber() */

/* FUNCTION NAME: IF_PMGR_GetIfNumber
 * PURPOSE: This funtion returns the value of sysUpTime at the time of the
 *          last creation or deletion of an entry in the ifTable. If the number of
 *          entries has been unchanged since the last re-initialization
 *          of the local network management subsystem, then this object
 *          contains a zero value.
 * INPUT:  None.
 * OUTPUT: if_table_last_change_time
 * RETURN: TRUE/FALSE
 * NOTES: None.
 */
BOOL_T IF_PMGR_GetIfTableLastChange (UI32_T *if_table_last_change_time)
{
    const UI32_T msg_buf_size=(sizeof(((IF_MGR_IPCMsg_T *)0)->data.ui32_v)
        +sizeof(((IF_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    IF_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_IFMGR;
    /*request size
     */
    msg_p->msg_size = IF_MGR_MSGBUF_TYPE_SIZE;

    msg_data_p=(IF_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = IF_MGR_IPCCMD_GETIFTABLELASTCHANGE;

    /*respond size
     */
    resp_data_size=msg_buf_size ;

    /*assign input parameter
     */

    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output parameter
     */
    *if_table_last_change_time=msg_data_p->data.ui32_v;

    return msg_data_p->type.result_bool;
} /* end of IF_PMGR_GetIfTableLastChange() */

/* FUNCTION NAME: IF_PMGR_GetIfEntry
 * PURPOSE: This funtion returns true if the specified interface entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:   if_entry->if_index      - key to specify a unique interface entry
 * OUTPUT:  if_entry                - interface entry info of specified if_index
 * RETURN:  TRUE/FALSE
 * NOTES:   None.
 */
BOOL_T IF_PMGR_GetIfEntry (IF_MGR_IfEntry_T  *if_entry)
{
    const UI32_T msg_buf_size=(sizeof(((IF_MGR_IPCMsg_T *)0)->data.if_entry)
        +sizeof(((IF_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    IF_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_IFMGR;
    /*request size
     */
    msg_p->msg_size = (sizeof(msg_data_p->data.if_entry.if_index)
        +sizeof(((IF_MGR_IPCMsg_T *)0)->type)) ;

    msg_data_p=(IF_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = IF_MGR_IPCCMD_GETIFENTRY;

    /*respond size
     */
    resp_data_size=msg_buf_size;

    /*assign input parameter
     */
     msg_data_p->data.if_entry.if_index=if_entry->if_index;

    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output parameter
     */
    *if_entry=msg_data_p->data.if_entry;

    return msg_data_p->type.result_bool;
} /* end of IF_PMGR_GetIfEntry() */

/* FUNCTION NAME: IF_PMGR_GetNextIfEntry
 * PURPOSE: This funtion returns true if the next available interface entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:   if_entry->if_index      - key to specify a unique interface entry
 * OUTPUT:  if_entry                - interface entry info of specified if_index
 * RETURN:  TRUE/FALSE
 * NOTES:   If next available interface entry is available, the if_entry->if_index
 *          will be updated and the entry info will be retrieved from the table.
 */
BOOL_T IF_PMGR_GetNextIfEntry (IF_MGR_IfEntry_T  *if_entry)
{
    const UI32_T msg_buf_size=(sizeof(((IF_MGR_IPCMsg_T *)0)->data.if_entry)
        +sizeof(((IF_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    IF_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_IFMGR;
    /*request size
     */
    msg_p->msg_size = (sizeof(msg_data_p->data.if_entry.if_index)
        +sizeof(((IF_MGR_IPCMsg_T *)0)->type)) ;

    msg_data_p=(IF_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = IF_MGR_IPCCMD_GETNEXTIFENTRY;

    /*respond size
     */
    resp_data_size=msg_buf_size ;

    /*assign input parameter
     */
    msg_data_p->data.if_entry.if_index=if_entry->if_index;

    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output parameter
     */
    *if_entry=msg_data_p->data.if_entry;

    return msg_data_p->type.result_bool;
} /* end of IF_PMGR_GetNextIfEntry() */

/* FUNCTION NAME: IF_PMGR_SetIfAdminStatus
 * PURPOSE: This funtion returns true if desired/new admin status is successfully set to
 *          the specified interface. Otherwise, false is returned.
 * INPUT:  if_index         - key to specify a unique interface
 *         if_admin_status  - VAL_ifAdminStatus_up / VAL_ifAdminStatus_down /
 *                            VAL_ifAdminStatus_testing
 * OUTPUT: None.
 * RETURN: TRUE/FALSE
 * NOTES: 1. The testing(3) state indicates that no operational packets can be passed.
 *        2. When a managed system initializes, all interfaces start with ifAdminStatus
 *           in the down(2) state.
 *        3. As a result of either explicit management action or per configuration
 *           information retained by the managed system, ifAdminStatus is then changed
 *           to either the up(1) or  testing(3) states (or remains in the down(2) state).
 */
BOOL_T IF_PMGR_SetIfAdminStatus (UI32_T if_index, UI32_T if_admin_status)
{
    const UI32_T msg_buf_size=(sizeof(((IF_MGR_IPCMsg_T *)0)->data.index_adminstatus)
        +sizeof(((IF_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    IF_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_IFMGR;
    /*request size
     */
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(IF_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = IF_MGR_IPCCMD_SETIFADMINSTATUS;

    /*respond size
     */
    resp_data_size=IF_MGR_MSGBUF_TYPE_SIZE ;

    /*assign input parameter
     */
    msg_data_p->data.index_adminstatus.if_index=if_index;
    msg_data_p->data.index_adminstatus.if_admin_status=if_admin_status;

    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output parameter
     */

    return msg_data_p->type.result_bool;
} /* end of IF_PMGR_SetIfAdminStatus() */

/* FUNCTION NAME: IF_PMGR_GetIfXEntry
 * PURPOSE: This funtion returns true if the specified extension interface entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:   if_x_entry->if_index    - key to specify a unique extension interface entry
 * OUTPUT:  if_entry                - extension interface entry info of specified if_index
 * RETURN:  TRUE/FALSE
 * NOTES:   None.
 */
BOOL_T IF_PMGR_GetIfXEntry (IF_MGR_IfXEntry_T  *if_x_entry)
{
    const UI32_T msg_buf_size=(sizeof(((IF_MGR_IPCMsg_T *)0)->data.if_x_entry)
        +sizeof(((IF_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    IF_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_IFMGR;
    /*request size
     */
    msg_p->msg_size = (sizeof(msg_data_p->data.if_x_entry.if_index)
        +sizeof(((IF_MGR_IPCMsg_T *)0)->type)) ;

    msg_data_p=(IF_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = IF_MGR_IPCCMD_GETIFXENTRY;

    /*respond size
     */
    resp_data_size=msg_buf_size ;

    /*assign input parameter
     */
    msg_data_p->data.if_x_entry.if_index=if_x_entry->if_index;

    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output parameter
     */
    *if_x_entry=msg_data_p->data.if_x_entry;

    return msg_data_p->type.result_bool;
} /* end of IF_PMGR_GetIfXEntry() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME: IF_PMGR_GetNextIfXEntry
 * ------------------------------------------------------------------------
 * PURPOSE: This funtion returns true if the next available extension interface entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:   if_x_entry->if_index    - key to specify a unique extension interface entry
 * OUTPUT:  if_x_entry              - extension interface entry info of specified if_index
 * RETURN:  TRUE/FALSE
 * NOTES:   If next available extension interface entry is available, the if_x_entry->if_index
 *          will be updated and the entry info will be retrieved from the table.
 * ------------------------------------------------------------------------
 */
BOOL_T IF_PMGR_GetNextIfXEntry (IF_MGR_IfXEntry_T  *if_x_entry)
{
    const UI32_T msg_buf_size=(sizeof(((IF_MGR_IPCMsg_T *)0)->data.if_x_entry)
        +sizeof(((IF_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    IF_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_IFMGR;
    /*request size
     */
    msg_p->msg_size = (sizeof(msg_data_p->data.if_x_entry.if_index)
        +sizeof(((IF_MGR_IPCMsg_T *)0)->type)) ;

    msg_data_p=(IF_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = IF_MGR_IPCCMD_GETNEXTIFXENTRY;

    /*respond size
     */
    resp_data_size=msg_buf_size ;

    /*assign input parameter
     */
    msg_data_p->data.if_x_entry.if_index=if_x_entry->if_index;

    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output parameter
     */
    *if_x_entry=msg_data_p->data.if_x_entry;

    return msg_data_p->type.result_bool;
} /* end of IF_PMGR_GetNextIfXEntry() */

/* ------------------------------------------------------------------------
 * FUNCTION NAME: IF_PMGR_SetIfLinkUpDownTrapEnableGlobal
 * ------------------------------------------------------------------------
 * PURPOSE: Set trap status to all interfaces.
 * INPUT:   trap_status -- trap status
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTES:   None
 * ------------------------------------------------------------------------
 */
BOOL_T 
IF_PMGR_SetIfLinkUpDownTrapEnableGlobal(
    UI32_T trap_status)
{
    const UI32_T msg_buf_size = (sizeof(((IF_MGR_IPCMsg_T *)0)->data.ui32_v)
        + sizeof(((IF_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    IF_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p = (SYSFUN_Msg_T *)&space_msg;
    msg_p->cmd = SYS_MODULE_IFMGR;
    msg_p->msg_size = msg_buf_size;
    resp_data_size = IF_MGR_MSGBUF_TYPE_SIZE;

    msg_data_p = (IF_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = IF_MGR_IPCCMD_SETIFLINKUPDOWNTRAPENABLEGLOBAL;
    msg_data_p->data.ui32_v = trap_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!= SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
} /* End of IF_PMGR_SetIfLinkUpDownTrapEnableGlobal */

/* ------------------------------------------------------------------------
 * FUNCTION NAME: IF_PMGR_SetIfLinkUpDownTrapEnable
 * ------------------------------------------------------------------------
 * PURPOSE: This funtion returns true if the trap status of the specific interface
 *          can be set successfully.  Otherwise, return FALSE.
 * INPUT:   if_x_index  -- key to specify which index to configured.
 *          trap_status -- trap status
 *                        VAL_ifLinkUpDownTrapEnable_enabled  (1)
 *                        VAL_ifLinkUpDownTrapEnable_disabled (2)
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE
 * NOTES:   none
 * ------------------------------------------------------------------------
 */
BOOL_T IF_PMGR_SetIfLinkUpDownTrapEnable(UI32_T if_x_index, UI32_T trap_status)
{
    const UI32_T msg_buf_size=(sizeof(((IF_MGR_IPCMsg_T *)0)->data.index_trapstatus)
        +sizeof(((IF_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    IF_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_IFMGR;
    /*request size
     */
    msg_p->msg_size =msg_buf_size ;

    msg_data_p=(IF_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = IF_MGR_IPCCMD_SETIFLINKUPDOWNTRAPENABLE;

    /*respond size
     */
    resp_data_size=IF_MGR_MSGBUF_TYPE_SIZE ;

    /*assign input parameter
     */
    msg_data_p->data.index_trapstatus.if_x_index=if_x_index;
    msg_data_p->data.index_trapstatus.trap_status=trap_status;
    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output parameter
     */

    return msg_data_p->type.result_bool;
} /* End of IF_PMGR_SetIfLinkUpDownTrapEnable */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - IF_PMGR_GetRunningIfLinkUpDownTrapEnable
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the global LinkUpDownTrapEnable status.
 * INPUT    :   if_x_index              -- the specified IfIndex
 * OUTPUT   :   UI32_T *trap_status     -- pointer of the status value
 *                      VAL_ifLinkUpDownTrapEnable_enabled  (1)
 *                      VAL_ifLinkUpDownTrapEnable_disabled (2)
 *
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  IF_PMGR_GetRunningIfLinkUpDownTrapEnable(UI32_T if_x_index, UI32_T *trap_status)
{
    const UI32_T msg_buf_size=(sizeof(((IF_MGR_IPCMsg_T *)0)->data.index_trapstatus)
        +sizeof(((IF_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    IF_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_IFMGR;
    /*request size
     */
    msg_p->msg_size =(sizeof(((IF_MGR_IPCMsg_T *)0)->data.index_trapstatus.if_x_index)
        +sizeof(((IF_MGR_IPCMsg_T *)0)->type)) ;

    msg_data_p=(IF_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = IF_MGR_IPCCMD_GETRUNNINGIFLINKUPDOWNTRAPENABLE;

    /*respond size
     */
    resp_data_size=msg_buf_size ;

    /*assign input parameter
     */
    msg_data_p->data.index_trapstatus.if_x_index=if_x_index;

    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output parameter
     */
    if_x_index=msg_data_p->data.index_trapstatus.if_x_index;
    *trap_status=msg_data_p->data.index_trapstatus.trap_status;
    return msg_data_p->type.result_ui32;
} /* End of IF_PMGR_GetRunningIfLinkUpDownTrapEnable */

/* FUNCTION NAME: IF_PMGR_SetIfAlias
 * PURPOSE: This funtion returns true if the if_alias of the specific interface
 *          can be set successfully.  Otherwise, return FALSE.
 * INPUT:   if_x_index - key to specify which index to configured.
 *			if_alias - the read/write name of the specific interface
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE
 * NOTES:   none
 */
BOOL_T IF_PMGR_SetIfAlias(UI32_T if_x_index, UI8_T *if_alias)
{
    const UI32_T msg_buf_size=(sizeof(((IF_MGR_IPCMsg_T *)0)->data.index_alias)
        +sizeof(((IF_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    IF_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_IFMGR;
    /*request size
     */
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(IF_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = IF_MGR_IPCCMD_SETIFALIAS;

    /*respond size
     */
    resp_data_size=IF_MGR_MSGBUF_TYPE_SIZE ;

    /*assign input parameter
     */
    msg_data_p->data.index_alias.if_x_index=if_x_index;
    memcpy(msg_data_p->data.index_alias.if_alias,if_alias,
        sizeof(msg_data_p->data.index_alias.if_alias));

    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output parameter
     */

    return msg_data_p->type.result_bool;
} /* end of IF_PMGR_SetIfAlias() */

/* FUNCTION NAME: IF_PMGR_GetIfStackEntry
 * PURPOSE: This funtion returns true if next available interface stack entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:  if_stack_entry->if_stack_higher_layer_if_index   - primary key to specify
 *                                                          - a unique interface stack entry
 *         if_stack_entry->if_stack_lower_layer_if_index    - secondary key
 * OUTPUT: if_stack_entry                   - next available interface stack entry info
 * RETURN: TRUE/FALSE
 * NOTES: 1. The write-access to this object will not be supported by this device.
 *        2. Changing the value of this object from 'active' to 'notInService' or 'destroy'
 *           will likely have consequences up and down the interface stack.
 *           Thus, write access to this object is likely to be inappropriate for some types of
 *           interfaces, and many implementations will choose not to support write-access for
 *           any type of interface.
 */
BOOL_T IF_PMGR_GetIfStackEntry(IF_MGR_IfStackEntry_T *if_stack_entry)
{
    const UI32_T msg_buf_size=(sizeof(((IF_MGR_IPCMsg_T *)0)->data.if_stack_entry)
        +sizeof(((IF_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    IF_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_IFMGR;
    /*request size
     */
    msg_p->msg_size =msg_buf_size ;

    msg_data_p=(IF_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = IF_MGR_IPCCMD_GETIFSTACKENTRY;

    /*respond size
     */
    resp_data_size=msg_buf_size ;

    /*assign input parameter
     */
    msg_data_p->data.if_stack_entry=*if_stack_entry;

    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output parameter
     */
    *if_stack_entry=msg_data_p->data.if_stack_entry;

    return msg_data_p->type.result_bool;
} /* end of IF_PMGR_GetIfStackEntry() */


/* FUNCTION NAME: IF_PMGR_GetNextIfStackEntry
 * PURPOSE: This funtion returns true if next available interface stack entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:  if_stack_entry->if_stack_higher_layer_if_index   - primary key to specify
 *                                                          - a unique interface stack entry
 *         if_stack_entry->if_stack_lower_layer_if_index    - secondary key
 * OUTPUT: if_stack_entry                   - next available interface stack entry info
 * RETURN: TRUE/FALSE
 * NOTES: 1. The write-access to this object will not be supported by this device.
 *        2. Changing the value of this object from 'active' to 'notInService' or 'destroy'
 *           will likely have consequences up and down the interface stack.
 *           Thus, write access to this object is likely to be inappropriate for some types of
 *           interfaces, and many implementations will choose not to support write-access for
 *           any type of interface.
 */
BOOL_T IF_PMGR_GetNextIfStackEntry(IF_MGR_IfStackEntry_T *if_stack_entry)
{
    const UI32_T msg_buf_size=(sizeof(((IF_MGR_IPCMsg_T *)0)->data.if_stack_entry)
        +sizeof(((IF_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    IF_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_IFMGR;
    /*request size
     */
    msg_p->msg_size =msg_buf_size ;

    msg_data_p=(IF_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = IF_MGR_IPCCMD_GETNEXTIFSTACKENTRY;

    /*respond size
     */
    resp_data_size=msg_buf_size ;

    /*assign input parameter
     */
    msg_data_p->data.if_stack_entry=*if_stack_entry;

    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output parameter
     */
    *if_stack_entry=msg_data_p->data.if_stack_entry;

    return msg_data_p->type.result_bool;
} /* end of IF_PMGR_GetNextIfStackEntry() */

/* FUNCTION NAME: IF_PMGR_GetIfStackLastChange
 * PURPOSE: This funtion returns true if the last update time of specified interface stack
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:   none
 * OUTPUT:  if_stack_last_change_time - The value of sysUpTime at the time of the last change of
 *                                      the (whole) interface stack.
 * RETURN: TRUE/FALSE
 * NOTES:  A change of the interface stack is defined to be any creation, deletion, or change in
 *         value of any instance of ifStackStatus.  If the interface stack has been unchanged
 *         since the last re-initialization of the local network management subsystem, then
 *         this object contains a zero value.
 */
BOOL_T IF_PMGR_GetIfStackLastChange (UI32_T *if_stack_last_change_time)
{
    const UI32_T msg_buf_size=(sizeof(((IF_MGR_IPCMsg_T *)0)->data.ui32_v)
        +sizeof(((IF_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    IF_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_IFMGR;
    /*request size
     */
    msg_p->msg_size = IF_MGR_MSGBUF_TYPE_SIZE;

    msg_data_p=(IF_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = IF_MGR_IPCCMD_GETIFSTACKLASTCHANGE;

    /*respond size
     */
    resp_data_size=msg_buf_size ;

    /*assign input parameter
     */

    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output parameter
     */
    *if_stack_last_change_time=msg_data_p->data.ui32_v;

    return msg_data_p->type.result_bool;
} /* end of IF_PMGR_GetIfStackLastChange() */

/* FUNCTION NAME: IF_PMGR_IfnameToIfindex
 * PURPOSE: This function returns true if the given ifname has a corresponding ifindex existed
 *          in the system.  Otherwise, returns false.
 * INPUT:   ifname - a read-only name for each interface defined during intialization
 * OUTPUT:  ifindex - corresponding interface index for the specific name.
 * RETURN:  TRUE/FALSE
 * NOTES:   None
 */
BOOL_T IF_PMGR_IfnameToIfindex (UI8_T *ifname, UI32_T *ifindex)
{
    const UI32_T msg_buf_size=(sizeof(((IF_MGR_IPCMsg_T *)0)->data.index_name)
        +sizeof(((IF_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    IF_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_IFMGR;
    /*request size
     */
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(IF_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = IF_MGR_IPCCMD_IFNAMETOIFINDEX;

    /*respond size
     */
    resp_data_size=msg_buf_size ;

    /*assign input parameter
     */
     memcpy(msg_data_p->data.index_name.ifname,ifname,
        sizeof(msg_data_p->data.index_name.ifname));

    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output parameter
     */
    *ifindex=msg_data_p->data.index_name.ifindex;

    return msg_data_p->type.result_bool;
} /* end of IF_PMGR_IfnameToIfindex() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - IF_PMGR_GetIfType
 *------------------------------------------------------------------------------
 * PURPOSE  : This function determines the type of interface based on ifindex
 *            value.
 * INPUT    : ifindex -- interface index
 * OUTPUT   : iftype -- type of interface based on ifindex
 *                      (IF_MGR_NORMAL_IFINDEX,
 *                       IF_MGR_TRUNK_IFINDEX,
 *                       IF_MGR_RS232_IFINDEX,
 *                       IF_MGR_VLAN_IFINDEX,
 *                       IF_MGR_ERROR_IFINDEX)
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *------------------------------------------------------------------------------
 */
BOOL_T  IF_PMGR_GetIfType(UI32_T ifindex, UI32_T *iftype)
{
    const UI32_T msg_buf_size=(sizeof(((IF_MGR_IPCMsg_T *)0)->data.index_type)
        +sizeof(((IF_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    IF_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T resp_data_size;

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_IFMGR;
    /*request size
     */
    msg_p->msg_size =(sizeof(((IF_MGR_IPCMsg_T *)0)->data.index_type.ifindex)
        +sizeof(((IF_MGR_IPCMsg_T *)0)->type)) ;

    msg_data_p=(IF_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = IF_MGR_IPCCMD_GETIFTYPE;

    /*respond size
     */
    resp_data_size=msg_buf_size ;

    /*assign input parameter
     */
    msg_data_p->data.index_type.ifindex=ifindex;

    /* send ipc message
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, resp_data_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output parameter
     */
    *iftype=msg_data_p->data.index_type.iftype;
    return msg_data_p->type.result_bool;
} /* end of IF_PMGR_GetIfType */
