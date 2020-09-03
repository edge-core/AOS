/* MODULE NAME:  syslog_pmgr.c
 * PURPOSE:
 *    PMGR implement for syslog.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_bld.h"
#include "syslog_pmgr.h"
#include "syslog_mgr.h"
#include "syslog_om.h"
#include "sys_module.h"
#include "l_mm.h"
#include "sysfun.h"
#include "sys_module.h"
#include "sys_type.h"

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

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_InitiateProcessResource
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
BOOL_T SYSLOG_PMGR_InitiateProcessResource(void)
{
    if(SYSFUN_GetMsgQ(SYS_BLD_UTILITY_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME: SYSLOG_PMGR_AddEntrySync
 * PURPOSE: Add a log message to system log module synchrously.
 * INPUT:   *syslog_entry   -- add this syslog entry
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *          If the entry will be written to flash, it
 *          will be done after calling this function.
 */
BOOL_T SYSLOG_PMGR_AddEntrySync(SYSLOG_OM_Record_T *syslog_entry)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_entry_data);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_ADD_ENTRY_SYNC;
    memcpy(&data_p->data.syslog_entry_data.syslog_entry, syslog_entry, sizeof(SYSLOG_OM_Record_T));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, SYSLOG_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return data_p->type.ret_bool;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_AddEntry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_AddEntry through the IPC msgq.
 * INPUT:
 *    None
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    port
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SYSLOG_PMGR_AddEntry(SYSLOG_OM_Record_T *syslog_entry)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_entry_data);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_ADD_ENTRY;
    memcpy(&data_p->data.syslog_entry_data.syslog_entry, syslog_entry, sizeof(SYSLOG_OM_Record_T));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, SYSLOG_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return data_p->type.ret_bool;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_AddFormatMsgEntry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_AddFormatMsgEntry through the IPC msgq.
 * INPUT:
 *    None
 *
 * OUTPUT:
 *    *state --  current ssh status.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    The max buffer size of arg_0, arg_1 and arg_2 are SYSLOG_ADPT_MESSAGE_LENGTH.
 *    If pass a string over this length, only SYSLOG_ADPT_MESSAGE_LENGTH will be copied.
 *    And string will be truncated.
 *------------------------------------------------------------------------------
 */
BOOL_T 
SYSLOG_PMGR_AddFormatMsgEntry(
    const SYSLOG_OM_RecordOwnerInfo_T *owner_info,
    UI32_T   message_index,
    const void    *arg_0,
    const void    *arg_1,
    const void    *arg_2)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_format_msg);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_ADD_FORMAT_MSG_ENTRY;

   memset(&data_p->data.syslog_format_msg, 0, sizeof(((SYSLOG_MGR_IPCMsg_T*)0)->data.syslog_format_msg));


    data_p->data.syslog_format_msg.message_index = message_index;
    memcpy(&data_p->data.syslog_format_msg.owner_info, owner_info, sizeof(SYSLOG_OM_RecordOwnerInfo_T));


    if(arg_0 != NULL)
        memcpy(data_p->data.syslog_format_msg.arg_0, arg_0, SYSLOG_ADPT_MESSAGE_LENGTH);

    if(arg_1 != NULL)
        memcpy(data_p->data.syslog_format_msg.arg_1, arg_1, SYSLOG_ADPT_MESSAGE_LENGTH);

    if(arg_2 != NULL)
        memcpy(data_p->data.syslog_format_msg.arg_2, arg_2, SYSLOG_ADPT_MESSAGE_LENGTH);

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return data_p->type.ret_bool;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_ClearAllFlashEntries
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_ClearAllFlashEntries through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SYSLOG_PMGR_ClearAllFlashEntries(void)
{
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(SYSLOG_MGR_MSGBUF_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_CLEAR_ALL_FLASH_ENTRIES;
    msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, SYSLOG_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return data_p->type.ret_bool;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_ClearAllRamEntries
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_ClearAllRamEntries through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SYSLOG_PMGR_ClearAllRamEntries(void)
{
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(SYSLOG_MGR_MSGBUF_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_CLEAR_ALL_RAM_ENTRIES;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, SYSLOG_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return data_p->type.ret_bool;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_GetNextUcFlashEntry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_GetNextUcFlashEntry through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SYSLOG_PMGR_GetNextUcFlashEntry(SYSLOG_MGR_Record_T *mgr_record)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_mgr_record_s);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_GET_NEXT_UC_FLASH_ENTRY;
    memcpy(&data_p->data.syslog_mgr_record_s.mgr_record, mgr_record, sizeof(SYSLOG_MGR_Record_T));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(mgr_record, &data_p->data.syslog_mgr_record, sizeof(SYSLOG_MGR_Record_T));

    return data_p->type.ret_bool;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_GetNextUcNormalEntries
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_GetNextUcNormalEntries through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SYSLOG_PMGR_GetNextUcNormalEntries(SYSLOG_MGR_Record_T *mgr_record)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_mgr_record_s);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_GET_NEXT_UC_NORMAL_ENTRIES;
    memcpy(&data_p->data.syslog_mgr_record_s.mgr_record, mgr_record, sizeof(SYSLOG_MGR_Record_T));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(mgr_record, &data_p->data.syslog_mgr_record_s.mgr_record, sizeof(SYSLOG_MGR_Record_T));

    return data_p->type.ret_bool;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_GetRunningUcLogLevel
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_GetRunningUcLogLevel through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_PMGR_GetRunningUcLogLevel(UI32_T *uc_log_level)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_om_data);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_GET_RUNNING_UC_LOG_LEVEL;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *uc_log_level = data_p->data.syslog_om_data.uc_log_level;

    return data_p->type.ret_ui32;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_GetRunningFlashLogLevel
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_GetRunningFlashLogLevel through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_PMGR_GetRunningFlashLogLevel(UI32_T *flash_log_level)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_om_data);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_GET_RUNNING_FLASH_LOG_LEVEL;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *flash_log_level = data_p->data.syslog_om_data.flash_log_level;

    return data_p->type.ret_ui32;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SetFlashLogLevel
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SetFlashLogLevel through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_PMGR_SetFlashLogLevel(UI32_T flash_log_level)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_om_data);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_SET_FLASH_LOG_LEVEL;
    data_p->data.syslog_om_data.flash_log_level = flash_log_level;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, SYSLOG_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return SYSLOG_RETURN_OK; //don't know what value is suitable
    }

    return data_p->type.ret_ui32;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SetSyslogStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SetSyslogStatus through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SYSLOG_PMGR_SetSyslogStatus(UI32_T syslog_status)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_om_data);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_SET_SYSLOG_STATUS;
    data_p->data.syslog_om_data.syslog_status = syslog_status;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, SYSLOG_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return data_p->type.ret_bool;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SetUcLogLevel
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SetUcLogLevel through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_PMGR_SetUcLogLevel(UI32_T uc_log_level)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_om_data);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_SET_UC_LOG_LEVEL;
    data_p->data.syslog_om_data.uc_log_level = uc_log_level;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, SYSLOG_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return SYSLOG_RETURN_OK; //don't know what value is suitable
    }

    return data_p->type.ret_ui32;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SnmpGetNextUcFlashEntry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SnmpGetNextUcFlashEntry through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SYSLOG_PMGR_SnmpGetNextUcFlashEntry(SYSLOG_MGR_Record_T *mgr_record)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_mgr_record);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_SNMP_GETNEXT_UC_FLASH_ENTRY;
    memcpy(&data_p->data.syslog_mgr_record, mgr_record, sizeof(SYSLOG_MGR_Record_T));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(mgr_record, &data_p->data.syslog_mgr_record, sizeof(SYSLOG_MGR_Record_T));

    return data_p->type.ret_bool;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SnmpGetNextUcNormalEntry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SnmpGetNextUcNormalEntry through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SYSLOG_PMGR_SnmpGetNextUcNormalEntry(SYSLOG_MGR_Record_T *mgr_record)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_mgr_record);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_SNMP_GETNEXT_UC_NORMAL_ENTRY;
    memcpy(&data_p->data.syslog_mgr_record, mgr_record, sizeof(SYSLOG_MGR_Record_T));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(mgr_record, &data_p->data.syslog_mgr_record, sizeof(SYSLOG_MGR_Record_T));

    return data_p->type.ret_bool;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SnmpGetUcFlashEntry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SnmpGetUcFlashEntry through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SYSLOG_PMGR_SnmpGetUcFlashEntry(SYSLOG_MGR_Record_T *mgr_record)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_mgr_record);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_SNMP_GET_UC_FLASH_ENTRY;
    memcpy(&data_p->data.syslog_mgr_record, mgr_record, sizeof(SYSLOG_MGR_Record_T));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(mgr_record, &data_p->data.syslog_mgr_record, sizeof(SYSLOG_MGR_Record_T));

    return data_p->type.ret_bool;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SnmpGetUcNormalEntry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SnmpGetUcNormalEntry through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SYSLOG_PMGR_SnmpGetUcNormalEntry(SYSLOG_MGR_Record_T *mgr_record)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_mgr_record);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_SNMP_GET_UC_NORMAL_ENTRY;
    memcpy(&data_p->data.syslog_mgr_record, mgr_record, sizeof(SYSLOG_MGR_Record_T));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(mgr_record, &data_p->data.syslog_mgr_record, sizeof(SYSLOG_MGR_Record_T));

    return data_p->type.ret_bool;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_NotifyStaTplgChanged
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_NotifyStaTplgChanged through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void SYSLOG_PMGR_NotifyStaTplgChanged (void)
{
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(SYSLOG_MGR_MSGBUF_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_NOTIFYSTATPLGCHANGED;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_NOWAIT,
        SYSFUN_SYSTEM_EVENT_IPCMSG, 0, NULL)!=SYSFUN_OK)
    {
        return;
    }

    return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_NotifyStaTplgStabled
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_NotifyStaTplgStabled through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void SYSLOG_PMGR_NotifyStaTplgStabled (void)
{
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(SYSLOG_MGR_MSGBUF_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_NOTIFYSTATPLGSTABLED;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_NOWAIT,
        SYSFUN_SYSTEM_EVENT_IPCMSG, 0, NULL)!=SYSFUN_OK)
    {
        return;
    }

    return;
}

BOOL_T SYSLOG_PMGR_GetSyslogStatus(UI32_T *syslog_status)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_om_data);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_GETSYSLOGSTATUS;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *syslog_status = data_p->data.syslog_om_data.syslog_status;

    return data_p->type.ret_bool;
}

BOOL_T SYSLOG_PMGR_GetUcLogLevel(UI32_T *uc_log_level)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_om_data);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_GETUCLOGLEVEL;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *uc_log_level = data_p->data.syslog_om_data.uc_log_level;

    return data_p->type.ret_bool;
}

BOOL_T SYSLOG_PMGR_GetFlashLogLevel(UI32_T *flash_log_level)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_om_data);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_GETFLASHLOGLEVEL;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *flash_log_level = data_p->data.syslog_om_data.flash_log_level;

    return data_p->type.ret_bool;
}

/* LOCAL SUBPROGRAM BODIES
 */

/*fuzhimin,20090414*/
#if(SYS_CPNT_REMOTELOG == TRUE)

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_DeleteAllRemoteLogServer
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_DeleteAllHost through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void SYSLOG_PMGR_DeleteAllRemoteLogServer(void)
{
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(SYSLOG_MGR_MSGBUF_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_DELETE_ALL_SERVER;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_NOWAIT,
        SYSFUN_SYSTEM_EVENT_IPCMSG, 0, NULL)!=SYSFUN_OK)
    {
        return;
    }

    return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SetRemoteLogStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SetRemoteLogStatus through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_PMGR_SetRemoteLogStatus(UI32_T status)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_SET_REMOTE_LOG_STATUS;
    data_p->data.syslog_remote_config.status = status;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, SYSLOG_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    return data_p->type.ret_ui32;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_GetRunningRemoteLogStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_GetRunningRemoteLogStatus through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_PMGR_GetRunningRemoteLogStatus(UI32_T *status)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_GET_RUNNING_REMOTE_LOG_STATUS;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *status = data_p->data.syslog_remote_config.status;

    return data_p->type.ret_ui32;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_GetRunningSyslogStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_GetRunningSyslogStatus through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_PMGR_GetRunningSyslogStatus(UI32_T *syslog_status)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_om_data);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_GET_RUNNING_SYSLOG_STATUS;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *syslog_status = data_p->data.syslog_om_data.syslog_status;

    return data_p->type.ret_ui32;
}

UI32_T SYSLOG_PMGR_GetRemoteLogStatus(UI32_T *status)
{
     const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_GETREMOTELOGSTATUS;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    *status = data_p->data.syslog_remote_config.status;

    return data_p->type.ret_ui32;
}
#if 0
UI32_T SYSLOG_PMGR_GetServerIPAddr(UI8_T index, L_INET_AddrIp_T *ip_address )
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_GETSERVERIPADDR;
    data_p->data.syslog_remote_config.server_index = index;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    memcpy(ip_address, &data_p->data.syslog_remote_config.server_config.ipaddr, sizeof(L_INET_AddrIp_T));

    return data_p->type.ret_ui32;
}
#endif
/*fuzhimin,20090414,end*/

UI32_T SYSLOG_PMGR_GetNextRemoteLogServer(SYSLOG_OM_Remote_Server_Config_T *server_config)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;
    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_GET_NEXT_SERVER_CONFIG;
    memcpy(&data_p->data.syslog_remote_config.server_config, server_config
        , sizeof(SYSLOG_OM_Remote_Server_Config_T));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    memcpy( server_config
        , &data_p->data.syslog_remote_config.server_config
        , sizeof(SYSLOG_OM_Remote_Server_Config_T));

    return data_p->type.ret_ui32;
}
#if 0
UI32_T SYSLOG_PMGR_GetServerConfig(SYSLOG_OM_Remote_Server_Config_T *server_config,UI8_T index)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_GETSERVERCONFIG;
    memcpy(&(data_p->data.syslog_remote_config.server_config.ipaddr), &server_config->ipaddr
        , sizeof(SYSLOG_OM_Remote_Server_Config_T));
    data_p->data.syslog_remote_config.server_index = index;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    memcpy(server_config, &(data_p->data.syslog_remote_config.server_config)
        , sizeof(SYSLOG_OM_Remote_Server_Config_T));

    return data_p->type.ret_ui32;
}
#endif
UI32_T SYSLOG_PMGR_GetRemoteLogServer(SYSLOG_OM_Remote_Server_Config_T *server_config)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_GETSERVERCONFIGINDEXBYIPADDR;
    memcpy(&(data_p->data.syslog_remote_config.server_config.ipaddr), &server_config->ipaddr
        , sizeof(SYSLOG_OM_Remote_Server_Config_T));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    memcpy(&server_config->ipaddr, &(data_p->data.syslog_remote_config.server_config.ipaddr)
        , sizeof(SYSLOG_OM_Remote_Server_Config_T));

    return data_p->type.ret_ui32;
}
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_CreateRemoteLogServer
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_CreateHostIpAddr through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_PMGR_CreateRemoteLogServer(L_INET_AddrIp_T *ip_address)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_CREATE_SERVER_IPADDR;

    memcpy(&data_p->data.syslog_remote_config.server_config.ipaddr, ip_address, sizeof(L_INET_AddrIp_T));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, SYSLOG_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    return data_p->type.ret_ui32;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_DeleteRemoteLogServer
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_DeleteHostIPAddr through the IPC msgq.
 * INPUT:
 *    ip_address
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_PMGR_DeleteRemoteLogServer(L_INET_AddrIp_T *ip_address)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_DELETE_SERVER_IPADDR;
    memcpy(&data_p->data.syslog_remote_config.server_config.ipaddr, ip_address, sizeof(L_INET_AddrIp_T));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, SYSLOG_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    return data_p->type.ret_ui32;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SetRemoteLogServerPort
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SetHostPort through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_PMGR_SetRemoteLogServerPort(L_INET_AddrIp_T*ip_address, UI32_T port)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_SET_SERVER_PORT;
    memcpy(&data_p->data.syslog_remote_config.server_config.ipaddr
        , ip_address, sizeof(L_INET_AddrIp_T));
    data_p->data.syslog_remote_config.server_config.udp_port = port;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, SYSLOG_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    return data_p->type.ret_ui32;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_GetRemoteLogServerPort
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_GetHostPort through the IPC msgq.
 * INPUT:
 *    *ip_address -- the host ip to get the level
 *    *port_p -- the host port
 * OUTPUT:
 *    *port_p -- the host port
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_PMGR_GetRemoteLogServerPort(L_INET_AddrIp_T*ip_address, UI32_T* port_p)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;
    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_GET_SERVER_PORT;
    memcpy(&data_p->data.syslog_remote_config.server_config.ipaddr
        , ip_address, sizeof(L_INET_AddrIp_T));
    *port_p = data_p->data.syslog_remote_config.server_config.udp_port;


    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, SYSLOG_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    return data_p->type.ret_ui32;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_GetRunningRemoteLogServer
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_GetRunningRemoteLogServer through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_PMGR_GetRunningRemoteLogServer(SYSLOG_OM_Remote_Server_Config_T *server_config,UI8_T index)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_GET_RUNNING_SERVER_CONFIG;
    data_p->data.syslog_remote_config.server_index = index;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    memcpy(&server_config->ipaddr, &(data_p->data.syslog_remote_config.server_config.ipaddr), sizeof(L_INET_AddrIp_T));
    server_config->udp_port = data_p->data.syslog_remote_config.server_config.udp_port;
#if(SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)
    server_config->facility = data_p->data.syslog_remote_config.server_config.facility;
    server_config->level = data_p->data.syslog_remote_config.server_config.level;
#endif

    return data_p->type.ret_ui32;
}

#if(SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SetRemoteLogServerFacility
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SetHostFacility through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_PMGR_SetRemoteLogServerFacility(L_INET_AddrIp_T*ip_address, UI32_T facility)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;
    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_SET_SERVER_FACILITY;
    memcpy(&data_p->data.syslog_remote_config.server_config.ipaddr,
           ip_address, sizeof(L_INET_AddrIp_T));
    data_p->data.syslog_remote_config.server_config.facility = facility;


    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, SYSLOG_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    return data_p->type.ret_ui32;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SetRemoteLogServerLevel
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SetHostLevel through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_PMGR_SetRemoteLogServerLevel(L_INET_AddrIp_T *ip_address, UI32_T level)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;
    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_SET_SERVER_LEVEL;
    memcpy(&data_p->data.syslog_remote_config.server_config.ipaddr
        , ip_address, sizeof(L_INET_AddrIp_T));
    data_p->data.syslog_remote_config.server_config.level = level;


    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, SYSLOG_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    return data_p->type.ret_ui32;
}

#else
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SetRemoteLogFacility
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SetRemoteLogFacility through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_PMGR_SetRemoteLogFacility(UI32_T facility)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;
    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_SET_FACILITY;
    data_p->data.syslog_remote_config.facility = facility;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, SYSLOG_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    return data_p->type.ret_ui32;
}

UI32_T SYSLOG_PMGR_GetRemoteLogFacility(UI32_T *facility)
{
     const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_GET_FACILITY;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    *facility = data_p->data.syslog_remote_config.facility;

    return data_p->type.ret_ui32;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_SetRemoteLogLevel
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_SetRemoteLogLevel through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_PMGR_SetRemoteLogLevel(UI32_T level)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;
    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_SET_LEVEL;
    data_p->data.syslog_remote_config.level = level;


    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, SYSLOG_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    return data_p->type.ret_ui32;
}

UI32_T SYSLOG_PMGR_GetRemoteLogLevel(UI32_T *level)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_GET_LEVEL;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    *level = data_p->data.syslog_remote_config.level;

    return data_p->type.ret_ui32;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_GetRunningRemotelogLevel
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_GetRunningRemotelogLevel through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_PMGR_GetRunningRemotelogLevel(UI32_T *level)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_GET_RUNNING_REMOTE_LOG_LEVEL;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *level = data_p->data.syslog_remote_config.level;

    return data_p->type.ret_ui32;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_PMGR_GetRunningFacilityType
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_MGR_GetRunningFacilityType through the IPC msgq.
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_PMGR_GetRunningFacilityType(UI32_T *facility)
{
    const UI32_T msg_size = SYSLOG_MGR_GET_MSGBUFSIZE(syslog_remote_config);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = SYSLOG_MGR_MSGBUF_TYPE_SIZE;

    data_p = (SYSLOG_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = SYSLOG_MGR_IPC_GET_RUNNING_FACILITY_TYPE;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size, msgbuf_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *facility = data_p->data.syslog_remote_config.facility;

    return data_p->type.ret_ui32;
}
#endif /*endif (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)*/
#endif /*endif (SYS_CPNT_REMOTELOG == TRUE)*/
