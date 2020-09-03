/* MODULE NAME:  dbsync_txt_pmgr.c
 * PURPOSE:
 *    PMGR implement for dbsync_txt.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    07/30/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
 
/* INCLUDE FILE DECLARATIONS
 */
 
#if (SYS_CPNT_DBSYNC_TXT == TRUE)

#include <stdio.h>

#include "sys_bld.h"
#include "dbsync_txt_pmgr.h"
#include "dbsync_txt_mgr.h"
#include "sys_module.h"
#include "l_mm.h"

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
static SYSFUN_MsgQ_T ipcmsgq_handle;

enum
{
    SYS_MODULE_DBSYNC_TXT,
};
/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : DBSYNC_TXT_PMGR_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initiate resource used in the calling process, means the process that use
 *    this pmgr functions should call this init.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DBSYNC_TXT_PMGR_InitiateProcessResource(void)
{
    if(SYSFUN_GetMsgQ(SYS_BLD_XFER_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DBSYNC_TXT_PMGR_GetSwitchConfigAutosaveBusyStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call DBSYNC_TXT_MGR_GetSwitchConfigAutosaveBusyStatus through the IPC msgq.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DBSYNC_TXT_PMGR_GetSwitchConfigAutosaveBusyStatus ( UI32_T * status )
{
    const UI32_T msg_size = DBSYNC_TXT_MGR_GET_MSGBUFSIZE(dbsync_txt_data);    
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;    
    DBSYNC_TXT_MGR_IPCMsg_T  *data_p;
    BOOL_T             result;

    /* msgbuf_p will be used on both request and response
     * msg_size shall be max(req_buf_size, resp_buf_size)
     */
    

    msgbuf_p->cmd = SYS_MODULE_DBSYNC_TXT;

    data_p = (DBSYNC_TXT_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = DBSYNC_TXT_MGR_IPC_GETSWITCHCONFIGAUTOSAVEBUSYSTATUS;
    msgbuf_p->msg_size = msg_size;
    
    
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, DBSYNC_TXT_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return FALSE;
    }    
    
    *status = data_p->data.dbsync_txt_data.status;
    result = data_p->type.ret_bool;
    L_MM_Free(msgbuf_p);
    return result;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DBSYNC_TXT_PMGR_GetSwitchConfigAutosaveEnableStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call DBSYNC_TXT_MGR_GetSwitchConfigAutosaveEnableStatus through the IPC msgq.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DBSYNC_TXT_PMGR_GetSwitchConfigAutosaveEnableStatus ( UI32_T * status )
{
    const UI32_T msg_size = DBSYNC_TXT_MGR_GET_MSGBUFSIZE(dbsync_txt_data);    
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;    
    DBSYNC_TXT_MGR_IPCMsg_T  *data_p;
    BOOL_T             result;

    /* msgbuf_p will be used on both request and response
     * msg_size shall be max(req_buf_size, resp_buf_size)
     */
    

    msgbuf_p->cmd = SYS_MODULE_DBSYNC_TXT;

    data_p = (DBSYNC_TXT_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = DBSYNC_TXT_MGR_IPC_GETSWITCHCONFIGAUTOSAVEENABLESTATUS;
    msgbuf_p->msg_size = msg_size;
    
    
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, DBSYNC_TXT_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return FALSE;
    }    
    
    *status = data_p->data.dbsync_txt_data.status;
    result = data_p->type.ret_bool;
    L_MM_Free(msgbuf_p);
    return result;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DBSYNC_TXT_PMGR_Get_IsAllDirtyClear
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call DBSYNC_TXT_MGR_Get_IsAllDirtyClear through the IPC msgq.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DBSYNC_TXT_PMGR_Get_IsAllDirtyClear( void )
{
    const UI32_T msg_size = DBSYNC_TXT_MGR_GET_MSGBUFSIZE(dbsync_txt_data);    
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;    
    DBSYNC_TXT_MGR_IPCMsg_T  *data_p;
    BOOL_T             result;

    /* msgbuf_p will be used on both request and response
     * msg_size shall be max(req_buf_size, resp_buf_size)
     */
    

    msgbuf_p->cmd = SYS_MODULE_DBSYNC_TXT;

    data_p = (DBSYNC_TXT_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = DBSYNC_TXT_MGR_IPC_GET_ISALLDIRTYCLEAR;
    msgbuf_p->msg_size = msg_size;
    
    
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, DBSYNC_TXT_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return FALSE;
    }    
    
    result = data_p->type.ret_bool;
    L_MM_Free(msgbuf_p);
    return result;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DBSYNC_TXT_PMGR_Get_IsDoingAutosave
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call DBSYNC_TXT_MGR_Get_IsDoingAutosave through the IPC msgq.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DBSYNC_TXT_PMGR_Get_IsDoingAutosave(void)
{
    const UI32_T msg_size = DBSYNC_TXT_MGR_GET_MSGBUFSIZE(dbsync_txt_data);    
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf; 
    DBSYNC_TXT_MGR_IPCMsg_T  *data_p;
    BOOL_T             result;

    /* msgbuf_p will be used on both request and response
     * msg_size shall be max(req_buf_size, resp_buf_size)
     */
    
    msgbuf_p->cmd = SYS_MODULE_DBSYNC_TXT;

    data_p = (DBSYNC_TXT_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = DBSYNC_TXT_MGR_IPC_GET_ISALLDIRTYCLEAR;
    msgbuf_p->msg_size = msg_size;
    
    
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, DBSYNC_TXT_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return FALSE;
    }    
    
    result = data_p->type.ret_bool;
    L_MM_Free(msgbuf_p);
    return result;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DBSYNC_TXT_PMGR_SetDirty
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call DBSYNC_TXT_MGR_SetDirty through the IPC msgq.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void DBSYNC_TXT_PMGR_SetDirty(UI32_T status)
{
    const UI32_T msg_size = DBSYNC_TXT_MGR_GET_MSGBUFSIZE(dbsync_txt_data);    
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;    
    DBSYNC_TXT_MGR_IPCMsg_T  *data_p;
    /*BOOL_T             result; */ /* comment out for compiler warning:variable 'result' set but not used */

    /* msgbuf_p will be used on both request and response
     * msg_size shall be max(req_buf_size, resp_buf_size)
     */
   

    msgbuf_p->cmd = SYS_MODULE_DBSYNC_TXT;

    data_p = (DBSYNC_TXT_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = DBSYNC_TXT_MGR_IPC_SETDIRTY;
    msgbuf_p->msg_size = msg_size;
    data_p->data.dbsync_txt_data.status = status;
    
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, DBSYNC_TXT_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return ;
    }    
    
    /* result = data_p->type.ret_bool; */ /* comment out for compiler warning:variable 'result' set but not used */
    L_MM_Free(msgbuf_p);
    return ;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DBSYNC_TXT_PMGR_SetSwitchConfigAutosaveEnableStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call DBSYNC_TXT_MGR_SetSwitchConfigAutosaveEnableStatus through the IPC msgq.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DBSYNC_TXT_PMGR_SetSwitchConfigAutosaveEnableStatus ( UI32_T status )
{
    const UI32_T msg_size = DBSYNC_TXT_MGR_GET_MSGBUFSIZE(dbsync_txt_data);    
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;    
    DBSYNC_TXT_MGR_IPCMsg_T  *data_p;
    BOOL_T             result;

    /* msgbuf_p will be used on both request and response
     * msg_size shall be max(req_buf_size, resp_buf_size)
     */
    

    msgbuf_p->cmd = SYS_MODULE_DBSYNC_TXT;

    data_p = (DBSYNC_TXT_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = DBSYNC_TXT_MGR_IPC_SETSWITCHCONFIGAUTOSAVEENABLESTATUS;
    msgbuf_p->msg_size = msg_size;
    data_p->data.dbsync_txt_data.status = status;
    
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, DBSYNC_TXT_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return FALSE;
    }    
    
    result = data_p->type.ret_bool;
    L_MM_Free(msgbuf_p);
    return result;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DBSYNC_TXT_PMGR_Get_IsDoingAutosave
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call DBSYNC_TXT_MGR_Get_IsDoingAutosave through the IPC msgq.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DBSYNC_TXT_PMGR_Set_FlushAndDisable(BOOL_T status)
{
    const UI32_T msg_size = DBSYNC_TXT_MGR_GET_MSGBUFSIZE(dbsync_txt_data);    
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;    
    DBSYNC_TXT_MGR_IPCMsg_T  *data_p;
    BOOL_T             result;

    /* msgbuf_p will be used on both request and response
     * msg_size shall be max(req_buf_size, resp_buf_size)
     */
    

    msgbuf_p->cmd = SYS_MODULE_DBSYNC_TXT;

    data_p = (DBSYNC_TXT_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = DBSYNC_TXT_MGR_IPC_SET_FLUSHANDDISABLE;
    msgbuf_p->msg_size = msg_size;
    data_p->data.dbsync_txt_data.status = status;
    
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, DBSYNC_TXT_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return FALSE;
    }    
    
    result = data_p->type.ret_bool;
    L_MM_Free(msgbuf_p);
    return result;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DBSYNC_TXT_PMGR_Set_IsDoingAutosave
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call DBSYNC_TXT_MGR_Set_IsDoingAutosave through the IPC msgq.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DBSYNC_TXT_PMGR_Set_IsDoingAutosave(BOOL_T status)
{
    const UI32_T msg_size = DBSYNC_TXT_MGR_GET_MSGBUFSIZE(dbsync_txt_data);    
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;    
    DBSYNC_TXT_MGR_IPCMsg_T  *data_p;
    BOOL_T             result;

    /* msgbuf_p will be used on both request and response
     * msg_size shall be max(req_buf_size, resp_buf_size)
     */    

    msgbuf_p->cmd = SYS_MODULE_DBSYNC_TXT;

    data_p = (DBSYNC_TXT_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = DBSYNC_TXT_MGR_IPC_SET_ISDOINGAUTOSAVE;
    msgbuf_p->msg_size = msg_size;
    data_p->data.dbsync_txt_data.status = status;
    
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, DBSYNC_TXT_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return FALSE;
    }    
    
    result = data_p->type.ret_bool;
    L_MM_Free(msgbuf_p);
    return result;
}

#endif /* End of #if (SYS_CPNT_DBSYNC_TXT == TRUE) */
