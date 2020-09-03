/*-----------------------------------------------------------------------------
 * Module   : netaccess_pmgr.c
 *-----------------------------------------------------------------------------
 * PURPOSE  : Provide APIs to access NETACCESS.
 *-----------------------------------------------------------------------------
 * NOTES    :
 *
 *-----------------------------------------------------------------------------
 * HISTORY  : 07/23/2007 - Wakka Tu, Create
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_NETACCESS == TRUE)
#include "sys_module.h"
#include "sysfun.h"
#include "netaccess_mgr.h"
#include "netaccess_pmgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void NETACCESS_PMGR_SendMsg(
    UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val);

static BOOL_T NETACCESS_PMGR_GetUI32DataByLport(
    UI32_T ipc_cmd, UI32_T lport, UI32_T *out_p);

static SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningUI32DataByLport(
    UI32_T ipc_cmd, UI32_T lport, UI32_T *out_p);

static BOOL_T NETACCESS_PMGR_SetUI32DataByLport(
    UI32_T ipc_cmd, UI32_T lport, UI32_T data);

static BOOL_T NETACCESS_PMGR_GetUI32Data(UI32_T ipc_cmd, UI32_T *out_p);

static SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningUI32Data(
        UI32_T ipc_cmd,
        UI32_T *out_p
        );

static BOOL_T NETACCESS_PMGR_SetUI32Data(UI32_T ipc_cmd, UI32_T data);

static BOOL_T NETACCESS_PMGR_GetControlPortByLport(
    UI32_T ipc_cmd, UI32_T lport, NETACCESS_MGR_Dot1XAuthControlledPortStatus_T *out_p);


/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

/* MACRO FUNCTIONS DECLARACTION
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE: Initiate resource used in the calling process.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : TRUE  - success
 *          FALSE - failure
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_InitiateProcessResources(void)
{
    if (SYSFUN_GetMsgQ(NETACCESS_MGR_IPCMSG_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetSecurePortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get security modes of the port
 * INPUT  : lport
 * OUTPUT : secure_port_mode.
 * RETURN : TRUE/FALSE.
 * NOTES  : please reference NETACCESS_PortMode_T for secure_port_mode
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetSecurePortMode(UI32_T lport, NETACCESS_PortMode_T *secure_port_mode)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportNapm_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportNapm_T    *data_p;

    if (NULL == secure_port_mode) return FALSE;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport  = lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_SEC_PORT_MODE,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportNapm_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportNapm_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *secure_port_mode = data_p->nacpm;
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xConfigSettingToDefault
 *-------------------------------------------------------------------------
 * PURPOSE: This function will set default value of 1X configuration
 * INPUT  : none
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : [to replace DOT1X_MGR_SetConfigSettingToDefault]
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xConfigSettingToDefault(void)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(0))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_SET_1X_CFG_SETTING_TO_DFLT,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                           NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                           (UI32_T) FALSE);

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_ClearSecureAddressEntryByFilter
 * ---------------------------------------------------------------------
 * PURPOSE: clear secure mac address table entry by filter
 * INPUT  : in_filter.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_ClearSecureAddressEntryByFilter(
    NETACCESS_MGR_SecureAddressFilter_T *in_filter)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_SecureAddressFilter_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_SecureAddressFilter_T *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);

    memcpy (data_p, in_filter, sizeof (NETACCESS_MGR_SecureAddressFilter_T));

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_CLR_SEC_ADR_ENT_BY_FTR,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_SecureAddressFilter_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                           (UI32_T) FALSE);

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetMacAuthPortEntry
 *-------------------------------------------------------------------------
 * PURPOSE: Get the port entry of mac-authentication.
 * INPUT  : field_id     : field id.
 *          *key         : input key
 *          buffer_size  : buffer size
 * OUTPUT : *buffer      : The secure port entry.
 *          *used_buffer : used buffer size for output
 * RETURN : FALSE : error, TRUE : success
 * NOTES  : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *            so used_buffer = 4
 *          2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetMacAuthPortEntry(
    UI32_T field_id, void *key, void *buffer, UI32_T buffer_size, UI32_T *used_buffer)
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_MacAuthPrtEnt_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_MacAuthPrtEnt_T    *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->field_id = field_id;
    data_p->buf_size = buffer_size;
    memcpy(&data_p->entry_key, key, sizeof (data_p->entry_key));

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_MAC_AUTH_PORT_ENT,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_MacAuthPrtEnt_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_MacAuthPrtEnt_T),
                           (UI32_T ) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *used_buffer = data_p->used_buf;
        memcpy(buffer, &data_p->entry, data_p->used_buf);
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_GetNextMacAuthPortEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE: Get the next port entry of mac-authentication.
 * INPUT  : field_id     : field id.
 *          *key         : input key
 *          buffer_size  : buffer size
 * OUTPUT : *buffer      : The secure port entry.
 *          *used_buffer : used buffer size for output
 * RETURN : FALSE : error, TRUE : success
 * NOTES  : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *            so used_buffer = 4
 *          2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextMacAuthPortEntry(
    UI32_T field_id, void *key, void *buffer, UI32_T buffer_size, UI32_T *used_buffer)
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_MacAuthPrtEnt_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_MacAuthPrtEnt_T    *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->field_id = field_id;
    data_p->buf_size = buffer_size;
    memcpy(&data_p->entry_key, key, sizeof (data_p->entry_key));

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_NXT_MAC_AUTH_PORT_ENT,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_MacAuthPrtEnt_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_MacAuthPrtEnt_T),
                           (UI32_T ) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *used_buffer = data_p->used_buf;
        memcpy(buffer, &data_p->entry, data_p->used_buf);
        memcpy(key, &data_p->entry_key, sizeof (data_p->entry_key));
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_GetNextSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE: This function will get next secure address entry by the lport and the mac_address.
 * INPUT  : field_id     : field id.
 *          *key         : input key
 *          buffer_size  : buffer size
 * OUTPUT : *buffer      : The secure port entry.
 *          *used_buffer : used buffer size for output
 * RETURN : negative integers : error, 0 : success
 * NOTES  : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *            so used_buffer = 4
 *          2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *          3.key must be NETACCESS_MGR_SecureAddressEntryKey_T
 *-----------------------------------------------------------------------------------
 */
I32_T NETACCESS_PMGR_GetNextSecureAddressEntry(
        UI32_T field_id,
        NETACCESS_MGR_SecureAddressEntryKey_T *key,
        void *buffer,
        UI32_T buffer_size,
        UI32_T *used_buffer)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_SecAdrEnt_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_SecAdrEnt_T    *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->field_id = field_id;
    data_p->buf_size = buffer_size;
    memcpy(&data_p->key, key, sizeof (data_p->key));

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_NXT_SEC_ADR_ENT,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_SecAdrEnt_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_SecAdrEnt_T),
                           (UI32_T )-1);

    if (0 == NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *used_buffer = data_p->used_buf;
        memcpy(buffer, &data_p->entry, data_p->used_buf);
        memcpy(key, &data_p->key, sizeof (data_p->key));
    }

    return (I32_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_GetNextSecureAddressEntryByFilter
 *-----------------------------------------------------------------------------------
 * PURPOSE: This function will get next secure address entry by filter.
 * INPUT  : field_id     : field id.
 *          *key         : input key
 *          buffer_size  : buffer size
 * OUTPUT : *buffer      : The secure port entry.
 *          *used_buffer : used buffer size for output
 * RETURN : negative integers : error, 0 : success
 * NOTES  : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *            so used_buffer = 4
 *          2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *          3.key must be NETACCESS_MGR_SecureAddressEntryKeyFilter_T
 *-----------------------------------------------------------------------------------
 */
I32_T NETACCESS_PMGR_GetNextSecureAddressEntryByFilter(
        UI32_T field_id,
        NETACCESS_MGR_SecureAddressEntryKeyFilter_T *key,
        void *buffer,
        UI32_T buffer_size,
        UI32_T *used_buffer)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_SecAdrEnt_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_SecAdrEnt_T    *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->field_id = field_id;
    data_p->buf_size = buffer_size;
    memcpy(&data_p->key_ftr, key, sizeof (data_p->key_ftr));

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_NXT_SEC_ADR_ENT_BY_FTR,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_SecAdrEnt_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_SecAdrEnt_T),
                           (UI32_T )-1);

    if (0 == NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *used_buffer = data_p->used_buf;
        memcpy(buffer, &data_p->entry, data_p->used_buf);
        memcpy(key, &data_p->key_ftr, sizeof (data_p->key_ftr));
    }

    return (I32_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_GetNextSecurePortEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE: This function will get secure port entry by the unit and the port.
 * INPUT  : field_id     : field id.
 *          *key         : input key
 *          buffer_size  : buffer size
 * OUTPUT : *buffer      : The secure port entry.
 *          *used_buffer : used buffer size for output
 * RETURN : negative integers : error, 0 : success
 * NOTES  : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *            so used_buffer = 4
 *          2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *-----------------------------------------------------------------------------------
 */
I32_T NETACCESS_PMGR_GetNextSecurePortEntry(
    UI32_T field_id, void *key, void *buffer, UI32_T buffer_size, UI32_T *used_buffer)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_SecPrtEnt_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_SecPrtEnt_T    *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->field_id = field_id;
    data_p->buf_size = buffer_size;
    memcpy(&data_p->entry_key, key, sizeof (data_p->entry_key));

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_NXT_SEC_PORT_ENTRY,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_SecPrtEnt_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_SecPrtEnt_T),
                           (UI32_T )-1);

    if (0 == NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *used_buffer = data_p->used_buf;
        memcpy(buffer, &data_p->prt_entry, data_p->used_buf);
        memcpy(key, &data_p->entry_key, sizeof(data_p->entry_key));
    }

    return (I32_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningSecureReauthTime
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default reauth_time is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT  : None.
 * OUTPUT : reauth_time
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : 1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default system name.
 *          3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningSecureReauthTime(UI32_T *reauth_time)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_U32Data_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_U32Data_T  *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_RUNN_SEC_REAUTH_TIME,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_U32Data_T),
                           SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *reauth_time = data_p->data;
    }

    return NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_GetSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE: This function will get secure address entry entry by the unit,the port and the mac_address.
 * INPUT  : field_id     : field id.
 *          *key         : input key
 *          buffer_size  : buffer size
 * OUTPUT : *buffer      : The secure port entry.
 *          *used_buffer : used buffer size for output
 * RETURN : negative integers : error, 0 : success
 * NOTES  : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *            so used_buffer = 4
 *          2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *-----------------------------------------------------------------------------------
 */
I32_T NETACCESS_PMGR_GetSecureAddressEntry(
        UI32_T field_id,
        NETACCESS_MGR_SecureAddressEntryKey_T *key,
        void *buffer,
        UI32_T buffer_size,
        UI32_T *used_buffer)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_SecAdrEnt_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_SecAdrEnt_T    *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->field_id = field_id;
    data_p->buf_size = buffer_size;
    memcpy(&data_p->key, key, sizeof (data_p->key));

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_SEC_ADR_ENT,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_SecAdrEnt_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_SecAdrEnt_T),
                           (UI32_T )-1);

    if (0 == NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *used_buffer = data_p->used_buf;
        memcpy(buffer, &data_p->entry, data_p->used_buf);
    }

    return (I32_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_GetSecurePortEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE: This function will get secure port entry by the unit and the port.
 * INPUT  : field_id     : field id.
 *          *key         : input key
 *          buffer_size  : buffer size
 * OUTPUT : *buffer      : The secure port entry.
 *          *used_buffer : used buffer size for output
 * RETURN : negative integers : error, 0 : success
 * NOTES  : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *            so used_buffer = 4
 *          2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *-----------------------------------------------------------------------------------
 */
I32_T NETACCESS_PMGR_GetSecurePortEntry(
    UI32_T field_id, void *key, void *buffer, UI32_T buffer_size, UI32_T *used_buffer)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_SecPrtEnt_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_SecPrtEnt_T    *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->field_id = field_id;
    data_p->buf_size = buffer_size;
    memcpy(&data_p->entry_key, key, sizeof (data_p->entry_key));

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_SEC_PORT_ENTRY,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_SecPrtEnt_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_SecPrtEnt_T),
                           (UI32_T )-1);

    if (0 == NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *used_buffer = data_p->used_buf;
        memcpy(buffer, &data_p->prt_entry, data_p->used_buf);
    }

    return (I32_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetSecureReauthTime
 * ---------------------------------------------------------------------
 * PURPOSE: Get the reauth time.
 * INPUT  : None.
 * OUTPUT : reauth_time.
 * RETURN : TRUE/FALSE.
 * NOTES  : Specifies the default session lifetime in seconds before
 *          a forwarding MAC address is re-authenticated.
 *          The default time is 1800 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetSecureReauthTime(UI32_T *reauth_time)
{
    return NETACCESS_PMGR_GetUI32Data (
                NETACCESS_MGR_IPC_CMD_GET_SEC_REAUTH_TIME, reauth_time);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetSecureReauthTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the reauth time.
 * INPUT  : reauth_time.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : Specifies the reauth time in seconds before
 *          a forwarding MAC address is re-authenticated.
 *          The default time is 1800 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetSecureReauthTime(UI32_T reauth_time)
{
    return NETACCESS_PMGR_SetUI32Data (
                NETACCESS_MGR_IPC_CMD_SET_SEC_REAUTH_TIME, reauth_time);
}

#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetMacAddressAgingMode
 *-------------------------------------------------------------------------
 * PURPOSE: Get the MAC address aging mode.
 * INPUT  : None.
 * OUTPUT : mode_p -- MAC address aging mode
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : Aging mode
 *          VAL_networkAccessAging_enabled for aging enabled
 *          VAL_networkAccessAging_disabled for aging disabled
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetMacAddressAgingMode(UI32_T *mode_p)
{
    return NETACCESS_PMGR_GetUI32Data (
                NETACCESS_MGR_IPC_CMD_GET_MAC_ADR_AGING_MODE, mode_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningMacAddressAgingMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the MAC address aging mode.
 * INPUT  : None.
 * OUTPUT : mode_p -- VAL_networkAccessAging_enabled,
 *                    VAL_networkAccessAging_disabled
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningMacAddressAgingMode(UI32_T *mode_p)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_U32Data_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_U32Data_T  *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_RUNN_MAC_ADR_AGING_MODE,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_U32Data_T),
                           SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *mode_p = data_p->data;
    }

    return NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetMacAddressAgingMode
 *-------------------------------------------------------------------------
 * PURPOSE: Set the MAC address aging mode.
 * INPUT  : mode -- MAC address aging mode
 * OUTPUT : None.
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : Aging mode
 *          VAL_networkAccessAging_enabled for aging enabled
 *          VAL_networkAccessAging_disabled for aging disabled
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetMacAddressAgingMode(UI32_T mode)
{
    return NETACCESS_PMGR_SetUI32Data (
                NETACCESS_MGR_IPC_CMD_SET_MAC_ADR_AGING_MODE, mode);
}

#endif /* #if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE) */

#if(SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningSecureDynamicVlanStatus
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default dynamic_vlan_status is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT  : lport.
 * OUTPUT : dynamic_vlan_status
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : 1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default system name.
 *          3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningSecureDynamicVlanStatus(
    UI32_T lport, UI32_T *dynamic_vlan_status)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportData_T    *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport   = lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_RUNN_SEC_DYN_VLAN_STAS,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *dynamic_vlan_status = data_p->data;
    }

    return NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetSecureDynamicVlanStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Get dynamic VLAN configuration control
 * INPUT  : lport.
 * OUTPUT : dynamic_vlan_status
 * RETURN : TRUE/FALSE.
 * NOTES  : The user-based port configuration control. Setting this attribute
 *          TRUE causes the port to be configured with any configuration
 *          parameters supplied by the authentication server. Setting this
 *          attribute to FALSE causes any configuration parameters supplied
 *          by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetSecureDynamicVlanStatus(UI32_T lport, UI32_T *dynamic_vlan_status)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportData_T    *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport   = lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_SEC_DYN_VLAN_STATUS,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *dynamic_vlan_status = data_p->data;
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetSecureDynamicVlanStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Set dynamic VLAN configuration control
 * INPUT  : lport, dynamic_vlan_status
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : The user-based port configuration control. Setting this attribute
 *          TRUE causes the port to be configured with any configuration
 *          parameters supplied by the authentication server. Setting this
 *          attribute to FALSE causes any configuration parameters supplied
 *          by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetSecureDynamicVlanStatus(UI32_T lport, UI32_T dynamic_vlan_status)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportData_T    *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport   = lport;
    data_p->data   = dynamic_vlan_status;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_SET_SEC_DYN_VLAN_STATUS,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                           (UI32_T) FALSE);

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

#endif /* #if(SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE) */

#if(SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningDynamicQosStatus
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default dynamic_vlan_status is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT  : lport.
 * OUTPUT : dynamic_vlan_status
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : 1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default system name.
 *          3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningDynamicQosStatus(
    UI32_T lport, UI32_T *dynamic_qos_status)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportData_T    *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport   = lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_RUNN_DYN_QOS_STATUS,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *dynamic_qos_status = data_p->data;
    }

    return NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetDynamicQosStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Get dynamic QoS configuration control
 * INPUT  : lport.
 * OUTPUT : dynamic_vlan_status
 * RETURN : TRUE/FALSE.
 * NOTES  : The user-based port configuration control. Setting this attribute
 *          TRUE causes the port to be configured with any configuration
 *          parameters supplied by the authentication server. Setting this
 *          attribute to FALSE causes any configuration parameters supplied
 *          by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDynamicQosStatus(UI32_T lport, UI32_T *dynamic_qos_status)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportData_T    *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport   = lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_DYN_QOS_STATUS,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *dynamic_qos_status = data_p->data;
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetDynamicQosStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Set dynamic VLAN configuration control
 * INPUT  : lport, dynamic_vlan_status
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : The user-based port configuration control. Setting this attribute
 *          TRUE causes the port to be configured with any configuration
 *          parameters supplied by the authentication server. Setting this
 *          attribute to FALSE causes any configuration parameters supplied
 *          by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDynamicQosStatus(UI32_T lport, UI32_T dynamic_qos_status)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportData_T    *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport   = lport;
    data_p->data    = dynamic_qos_status;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_SET_DYN_QOS_STATUS,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                           (UI32_T) FALSE);

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}
#endif /* #if(SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE) */

#if(SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningDot1xPortIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Get the intrusion action to determine the action if an unauthorized device
 *          transmits on this port.
 * INPUT  : lport
 * OUTPUT : action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                           VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES  : none.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningDot1xPortIntrusionAction(
    UI32_T lport, UI32_T *action_status)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportData_T    *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport  = lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_RUNN_1X_PORT_INTR_ACTN,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *action_status = data_p->data;
    }

    return NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningSecureGuestVlanId
 * ---------------------------------------------------------------------
 * PURPOSE: Get per-port guest VLAN ID.
 * INPUT  : lport.
 * OUTPUT : vid.
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningSecureGuestVlanId(
    UI32_T lport, UI32_T *vid)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportData_T    *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport = lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_RUNN_SEC_GUEST_VLAN_ID,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *vid = data_p->data;
    }

    return NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetDot1xPortIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Get the intrusion action to determine the action if an unauthorized device
 *          transmits on this port.
 * INPUT  : lport
 * OUTPUT : action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                           VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * RETURN : TRUE/FALSE.
 * NOTES  : none.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortIntrusionAction(UI32_T lport, UI32_T *action_status)
{
    return NETACCESS_PMGR_GetUI32DataByLport (
                NETACCESS_MGR_IPC_CMD_GET_1X_PORT_INTR_ACTN, lport, action_status);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetNextDot1xPortIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Get next port intrusion action that determine the action if an unauthorized device
 *          transmits on this port.
 * INPUT  : lport
 * OUTPUT : action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                           VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * RETURN : TRUE/FALSE.
 * NOTES  : none.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextDot1xPortIntrusionAction(UI32_T *lport, UI32_T *action_status)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportData_T    *data_p;

    if ((NULL == lport) || (NULL == action_status)) return FALSE;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport  = *lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_NXT_1X_PORT_INTR_ACTN,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *lport = data_p->lport;
        *action_status = data_p->data;
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetSecureGuestVlanId
 * ---------------------------------------------------------------------
 * PURPOSE: Get per-port guest VLAN ID.
 * INPUT  : lport.
 * OUTPUT : vid.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetSecureGuestVlanId(UI32_T lport, UI32_T *vid)
{
    return NETACCESS_PMGR_GetUI32DataByLport (
                NETACCESS_MGR_IPC_CMD_GET_SEC_GUEST_VLAN_ID, lport, vid);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetDot1xPortIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Set the intrusion action to determine the action if an unauthorized device
 *          transmits on this port.
 * INPUT  : lport,
 *          action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                           VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * OUTPUT : none.
 * RETURN : TRUE/FALSE.
 * NOTES  : none.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortIntrusionAction(UI32_T lport, UI32_T action_status)
{
    return NETACCESS_PMGR_SetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_SET_1X_PORT_INTR_ACTN, lport, action_status);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetSecureGuestVlanId
 * ---------------------------------------------------------------------
 * PURPOSE: Set per-port guest VLAN ID.
 * INPUT  : lport,vid.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetSecureGuestVlanId(UI32_T lport, UI32_T vid)
{
    return NETACCESS_PMGR_SetUI32DataByLport (
                NETACCESS_MGR_IPC_CMD_SET_SEC_GUEST_VLAN_ID, lport, vid);
}

#endif /* #if(SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE) */


#if (SYS_CPNT_NETACCESS_MACAUTH == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetMacAuthPortIntrusionAction
 *-------------------------------------------------------------------------
 * PURPOSE: Get the intrusion action of mac-authentication for the specified port.
 * INPUT  : lport -- logic port number
 * OUTPUT : action -- intrusion action
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : intrusion action
 *          VAL_macAuthPortIntrusionAction_block_traffic for block action
 *          VAL_macAuthPortIntrusionAction_pass_traffic for pass action
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetMacAuthPortIntrusionAction(UI32_T lport, UI32_T *action)
{
    return NETACCESS_PMGR_GetUI32DataByLport (
                NETACCESS_MGR_IPC_CMD_GET_MAC_AUTH_PORT_INTR_ACTION, lport, action);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetMacAuthPortMaxMacCount
 *-------------------------------------------------------------------------
 * PURPOSE: Get the max allowed MAC number of mac-authentication for the
 *          specified port.
 * INPUT  : lport -- logic port number
 * OUTPUT : count -- max mac count
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetMacAuthPortMaxMacCount(UI32_T lport, UI32_T *count)
{
    return NETACCESS_PMGR_GetUI32DataByLport (
                NETACCESS_MGR_IPC_CMD_GET_MAC_AUTH_PORT_MAX_MAC_CNT, lport, count);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MPGR_GetMacAuthPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE: Get the mac-authentication status for the specified port.
 * INPUT  : lport -- logic port number.
 * OUTPUT : mac_auth_status -- mac-authentication status.
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : mac-authentication status
 *            NETACCESS_TYPE_MACAUTH_ENABLED for Enable mac-authentication
 *            NETACCESS_TYPE_MACAUTH_DISABLED for Disable mac-authentication
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetMacAuthPortStatus(UI32_T lport, UI32_T *mac_auth_status)
{
    return NETACCESS_PMGR_GetUI32DataByLport (
                NETACCESS_MGR_IPC_CMD_GET_MAC_AUTH_PORT_STATUS, lport, mac_auth_status);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetRunningMacAuthPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE: Get the mac-authentication status for the specified port.
 * INPUT  : lport.
 * OUTPUT : mac_auth_status -- mac-authentication status.
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES  : mac-authentication status
 *          NETACCESS_TYPE_MACAUTH_ENABLED for Enable mac-authentication
 *          NETACCESS_TYPE_MACAUTH_DISABLED for Disable mac-authentication
 *-------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningMacAuthPortStatus(
    UI32_T lport, UI32_T *mac_auth_status)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportData_T    *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport  = lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_RUNN_MAC_AUTH_PORT_STAS,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *mac_auth_status = data_p->data;
    }

    return NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetRunningMacAuthPortIntrusionAction
 *-------------------------------------------------------------------------
 * PURPOSE: Get the intrusion action of mac-authentication for the specified port
 * INPUT  : lport -- logic port number
 * OUTPUT : action -- intrusion action
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES  : intrusion action
 *          VAL_macAuthPortIntrusionAction_block_traffic for block action
 *          VAL_macAuthPortIntrusionAction_pass_traffic for pass action
 *-------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningMacAuthPortIntrusionAction(
    UI32_T lport, UI32_T *action)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportData_T    *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport  = lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_RUNN_MAC_AUTH_PORT_INTR_ACTN,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *action = data_p->data;
    }

    return NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetRunningMacAuthPortMaxMacCount
 *-------------------------------------------------------------------------
 * PURPOSE: Get the max allowed MAC number of mac-authentication for the
 *          specified port.
 * INPUT  : lport -- logic port number
 * OUTPUT : count -- max mac count
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES  : none.
 *-------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningMacAuthPortMaxMacCount(
    UI32_T lport, UI32_T *count)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportData_T    *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport = lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_RUNN_MAC_AUTH_PORT_MAX_MAC,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *count = data_p->data;
    }

    return NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningSecureNumberAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default number is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT  : lport.
 * OUTPUT : number:secureNumberAddresses
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : 1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default system name.
 *          3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningSecureNumberAddresses(
    UI32_T lport, UI32_T *number)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportData_T    *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport  = lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_RUNN_SEC_NBR_ADR,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *number = data_p->data;
    }

    return NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetSecureNumberAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get secureNumberAddresses by lport.
 * INPUT  : lport : logical port.
 * OUTPUT : number:secureNumberAddresses.
 * RETURN : TRUE/FALSE
 * NOTES  :
 * The maximum number of addresses that the port can learn or
 * store. Reducing this number may cause some addresses to be deleted.
 * This value is set by the user and cannot be automatically changed by the
 * agent.
 *
 * The following relationship must be preserved.
 *
 *    secureNumberAddressesStored <= secureNumberAddresses <=
 *    secureMaximumAddresses.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetSecureNumberAddresses(UI32_T lport, UI32_T *number)
{
    return NETACCESS_PMGR_GetUI32DataByLport (
                NETACCESS_MGR_IPC_CMD_GET_SEC_NBR_ADR, lport, number);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetMacAuthPortIntrusionAction
 *-------------------------------------------------------------------------
 * PURPOSE: Set the intrustion action of mac-authentication for the specified port.
 * INPUT  : lport  -- logic port number
 *          action -- intrusion action
 * OUTPUT : none.
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : intrusion action
 *          VAL_macAuthPortIntrusionAction_block_traffic for block action
 *          VAL_macAuthPortIntrusionAction_pass_traffic for pass action
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetMacAuthPortIntrusionAction(UI32_T lport, UI32_T action)
{
    return NETACCESS_PMGR_SetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_SET_MAC_AUTH_PORT_INTR_ACTION, lport, action);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetMacAuthPortMaxMacCount
 *-------------------------------------------------------------------------
 * PURPOSE: Set the max allowed MAC number of mac-authentication for the
 *          specified port.
 * INPUT  : lport -- logic port number
 *          count -- max mac count
 * OUTPUT : None.
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetMacAuthPortMaxMacCount(UI32_T lport, UI32_T count)
{
    return NETACCESS_PMGR_SetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_SET_MAC_AUTH_PORT_MAX_MAC_CNT, lport, count);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetMacAuthPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE: Set the mac-authentication status for the specified port.
 * INPUT  : lport -- logic port number.
 *          mac_auth_status -- mac-authentication status.
 * OUTPUT : None.
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : mac-authentication status
 *          NETACCESS_TYPE_MACAUTH_ENABLED for Enable mac-authentication
 *          NETACCESS_TYPE_MACAUTH_DISABLED for Disable mac-authentication
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetMacAuthPortStatus(UI32_T lport, UI32_T mac_auth_status)
{
    return NETACCESS_PMGR_SetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_SET_MAC_AUTH_PORT_STATUS, lport, mac_auth_status);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_SetSecureNumberAddresses
 *-----------------------------------------------------------------------------------
 * PURPOSE: This function will set secureNumberAddresses by lport.
 * INPUT  : lport : logical port.
 *          number:secureNumberAddresses
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  :
 * The maximum number of addresses that the port can learn or
 * store. Reducing this number may cause some addresses to be deleted.
 * This value is set by the user and cannot be automatically changed by the
 * agent.
 *
 * The following relationship must be preserved.
 *
 *    secureNumberAddressesStored <= secureNumberAddresses <=
 *    secureMaximumAddresses.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetSecureNumberAddresses(UI32_T lport, UI32_T number)
{
    return NETACCESS_PMGR_SetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_SET_SEC_NBR_ADR, lport, number);
}

#endif /* #if (SYS_CPNT_NETACCESS_MACAUTH == TRUE) */

#if (SYS_CPNT_NETACCESS_MACAUTH_AUTH_AGE == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetSecureAuthAge
 * ---------------------------------------------------------------------
 * PURPOSE: Get the auth age.
 * INPUT  : None.
 * OUTPUT : auth_age.
 * RETURN : TRUE/FALSE.
 * NOTES  : none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetSecureAuthAge(UI32_T *auth_age)
{
    return NETACCESS_PMGR_GetUI32Data(
        NETACCESS_MGR_IPC_CMD_GET_SEC_AUTH_AGE, auth_age);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetSecureAuthAge
 * ---------------------------------------------------------------------
 * PURPOSE: Set the auth age.
 * INPUT  : None.
 * OUTPUT : auth_age.
 * RETURN : TRUE/FALSE.
 * NOTES  : none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetSecureAuthAge(UI32_T auth_age)
{
    return NETACCESS_PMGR_SetUI32Data(
        NETACCESS_MGR_IPC_CMD_SET_SEC_AUTH_AGE, auth_age);
}

#endif /* #if (SYS_CPNT_NETACCESS_MACAUTH_AUTH_AGE == TRUE) */

#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetNextFilterMac
 * ---------------------------------------------------------------------
 * PURPOSE: Get next secure filter table entry
 * INPUT  : filter_id,mac_address
 * OUTPUT : filter_id,mac_address
 * RETURN : TRUE/FALSE.
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextFilterMac(
    UI32_T  *filter_id, UI8_T   *mac_addr,  UI8_T   *mac_mask)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_FidMac_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_FidMac_T   *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->filter_id   = *filter_id;

    memcpy (data_p->mac_address, mac_addr, SYS_ADPT_MAC_ADDR_LEN);
    memcpy (data_p->mask, mac_mask, SYS_ADPT_MAC_ADDR_LEN);

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_NXT_FTR_MAC,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_FidMac_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_FidMac_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *filter_id = data_p->filter_id;
        memcpy (mac_addr, data_p->mac_address, SYS_ADPT_MAC_ADDR_LEN);
        memcpy (mac_mask, data_p->mask, SYS_ADPT_MAC_ADDR_LEN);
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetNextFilterMacByFilterId
 * ---------------------------------------------------------------------
 * PURPOSE: Get next secure filter table entry by filter id
 * INPUT  : filter_id, mac_addr, mac_mask
 * OUTPUT : mac_addr, mac_mask
 * RETURN : TRUE/FALSE.
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextFilterMacByFilterId(
    UI32_T  filter_id,  UI8_T   *mac_addr, UI8_T   *mac_mask)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_FidMac_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_FidMac_T   *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->filter_id   = filter_id;

    memcpy (data_p->mac_address, mac_addr, SYS_ADPT_MAC_ADDR_LEN);
    memcpy (data_p->mask, mac_mask, SYS_ADPT_MAC_ADDR_LEN);

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_NXT_FTR_MAC_BY_FTR_ID,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_FidMac_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_FidMac_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        memcpy (mac_addr, data_p->mac_address, SYS_ADPT_MAC_ADDR_LEN);
        memcpy (mac_mask, data_p->mask, SYS_ADPT_MAC_ADDR_LEN);
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningFilterMac
 * ---------------------------------------------------------------------
 * PURPOSE: Get next secure filter table entry
 * INPUT  : filter_id,mac_address
 * OUTPUT : filter_id,mac_address
 * RETURN : TRUE/FALSE.
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetRunningFilterMac(
    UI32_T  *filter_id, UI8_T   *mac_addr,  UI8_T   *mac_mask)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_FidMac_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_FidMac_T   *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->filter_id   = *filter_id;

    memcpy (data_p->mac_address, mac_addr, SYS_ADPT_MAC_ADDR_LEN);
    memcpy (data_p->mask, mac_mask, SYS_ADPT_MAC_ADDR_LEN);

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_RUNN_FTR_MAC,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_FidMac_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_FidMac_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *filter_id = data_p->filter_id;
        memcpy (mac_addr, data_p->mac_address, SYS_ADPT_MAC_ADDR_LEN);
        memcpy (mac_mask, data_p->mask, SYS_ADPT_MAC_ADDR_LEN);
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetFilterMac
 * ---------------------------------------------------------------------
 * PURPOSE: Get secure filter table entry
 * INPUT  : filter_id, mac_addr, mac_mask
 * OUTPUT : none
 * RETURN : TRUE/FALSE.
 * NOTES  : TURE means entry exist,FALSE to not exist
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetFilterMac(
    UI32_T  filter_id,  UI8_T   *mac_addr, UI8_T   *mac_mask)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_FidMac_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_FidMac_T   *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->filter_id   = filter_id;

    memcpy (data_p->mac_address, mac_addr, SYS_ADPT_MAC_ADDR_LEN);
    memcpy (data_p->mask, mac_mask, SYS_ADPT_MAC_ADDR_LEN);

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_FTR_MAC,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_FidMac_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_FidMac_T),
                           (UI32_T) FALSE);

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetFilterMac
 * ---------------------------------------------------------------------
 * PURPOSE: Set secure filter table
 * INPUT  : filter_id, mac_addr, mac_mask, is_add
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetFilterMac(
    UI32_T  filter_id,  UI8_T   *mac_addr,  UI8_T   *mac_mask,  BOOL_T  is_add)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_FidMacAct_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_FidMacAct_T    *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->filter_id   = filter_id;
    data_p->is_add      = is_add;

    memcpy (data_p->mac_address, mac_addr, SYS_ADPT_MAC_ADDR_LEN);
    memcpy (data_p->mask, mac_mask, SYS_ADPT_MAC_ADDR_LEN);

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_SET_FTR_MAC,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_FidMacAct_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                           (UI32_T) FALSE);

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_GetFilterIdOnPort
 *-----------------------------------------------------------------------------------
 * PURPOSE: Get secure filter id which bind to port
 * INPUT  : lport
 * OUTPUT : filter_id.
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetFilterIdOnPort(UI32_T lport, UI32_T *filter_id)
{
    return NETACCESS_PMGR_GetUI32DataByLport (
                NETACCESS_MGR_IPC_CMD_GET_FTR_ID_ON_PORT, lport, filter_id);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_GetRunningFilterIdOnPort
 *-----------------------------------------------------------------------------------
 * PURPOSE: Get secure filter id which bind to port
 * INPUT  : lport
 * OUTPUT : filter_id.
 * RETURN : SYS_TYPE_Get_Running_Cfg_T
 * NOTES  : None
 *-----------------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningFilterIdOnPort(UI32_T lport, UI32_T *filter_id)
{
    return NETACCESS_PMGR_GetRunningUI32DataByLport(
                NETACCESS_MGR_IPC_CMD_GET_RUNN_FTR_ID_ON_PORT, lport, filter_id);
}

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_SetFilterIdToPort
 *-----------------------------------------------------------------------------------
 * PURPOSE: bind secure filter id to port
 * INPUT  : lport, filter_id
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetFilterIdToPort(UI32_T lport, UI32_T filter_id)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_FidPortAct_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_FidPortAct_T   *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->filter_id   = filter_id;
    data_p->lport       = lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_SET_FTR_ID_TO_PORT,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_FidPortAct_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                           (UI32_T) FALSE);

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

#endif /* (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE) */

/* begin for ieee_8021x.c
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_DoDot1xReAuthenticate
 *-------------------------------------------------------------------------
 * PURPOSE: use the command to manually initiate a re-authentication of
 *          the specified 1X enabled port
 * INPUT  : lport
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_DoDot1xReAuthenticate(UI32_T lport)
{
    return NETACCESS_PMGR_SetUI32Data(
        NETACCESS_MGR_IPC_CMD_DO_1X_REAUTHEN, lport);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetDot1xAuthConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE: A table that contains the configuration objects for the Authenticator
 *          PAE associated with each port.
 *          An entry appears in this table for each port that may authenticate
 *          access to itself.
 * INPUT  : lport.
 * OUTPUT : AuthConfigEntry.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xAuthConfigEntry(
    UI32_T lport, DOT1X_AuthConfigEntry_T *AuthConfigPortEntry)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXacpe_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportXacpe_T   *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport   = lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_1X_AUTH_CFG_ENTRY,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXacpe_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXacpe_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *AuthConfigPortEntry = data_p->xacpe;
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetDot1xAuthDiagEntry
 *-------------------------------------------------------------------------
 * PURPOSE: A table that contains the diagnostics objects for the Authenticator
 *          PAE associated with each port.
 *          An entry appears in this table for each port that may authenticate
 *          access to itself.
 * INPUT  : lport.
 * OUTPUT : AuthDiagPortEntry.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xAuthDiagEntry(
    UI32_T lport, DOT1X_AuthDiagEntry_T *AuthDiagPortEntry)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXadpe_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportXadpe_T   *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport   = lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_1X_AUTH_DIAG_ENTRY,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXadpe_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXadpe_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *AuthDiagPortEntry = data_p->xadpe;
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetDot1xAuthStatsEntry
 *-------------------------------------------------------------------------
 * PURPOSE: A table that contains the statistics objects for the Authenticator
 *          PAE associated with each port.
 *          An entry appears in this table for each port that may authenticate
 *          access to itself.
 * INPUT  : lort.
 * OUTPUT : AuthStateEntry.
 * RETURN : TRUE  -- The port support 802.1x
 *          FALSE -- The port don't support 802.1x.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xAuthStatsEntry(
    UI32_T lport, DOT1X_AuthStatsEntry_T *AuthStatePortEntry)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXaspe_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportXaspe_T   *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport   = lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_1X_AUTH_STS_ENTRY,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXaspe_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXaspe_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *AuthStatePortEntry = data_p->xaspe;
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetDot1xPaePortEntry
 *-------------------------------------------------------------------------
 * PURPOSE: A table of system level information for each port supported
 *          by the Port Access Entity. An entry appears in this table for
 *          each port of this system.
 * INPUT  : lport.
 * OUTPUT : PaePortEntry.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPaePortEntry(
    UI32_T lport, DOT1X_PaePortEntry_T *PaePortEntry)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXpape_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportXpape_T   *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport   = lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_1X_PAE_PORT_ENTRY,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXpape_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXpape_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *PaePortEntry = data_p->xpape;
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetDot1xSessionStatsEntry
 *-------------------------------------------------------------------------
 * PURPOSE: The session statistics information for an Authenticatior PAE.
 *          This shows the current values being collected for each session
 *          that is still in progress, or the final values for the last valid
 *          session on each port where there is no session currently active.
 * INPUT  : lport.
 * OUTPUT : AuthSessionStatsPortEntry.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xSessionStatsEntry(
    UI32_T lport, DOT1X_AuthSessionStatsEntry_T *AuthSessionStatsPortEntry)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXasse_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportXasse_T   *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport   = lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_1X_SESS_STS_ENTRY,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXasse_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXasse_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *AuthSessionStatsPortEntry = data_p->xasse;
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xSystemAuthControl
 *-------------------------------------------------------------------------
 * PURPOSE: Get the administrative enable/disable state for Port Access
 *          Control in a system.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : SystemAuthControl
 * NOTES  : VAL_dot1xPaeSystemAuthControl_enabled for Enable SystemAuthControl
 *          VAL_dot1xPaeSystemAuthControl_disabled for Disable SystemAuthControl
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xSystemAuthControl(UI32_T *control_status)
{
    return NETACCESS_PMGR_GetUI32Data(
        NETACCESS_MGR_IPC_CMD_GET_1X_SYS_AUTH_CTRL, control_status);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetNextDot1xAuthConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE: Get next table that contains the configuration objects for the Authenticator
 *          PAE associated with each port.
 *          An entry appears in this table for each port that may authenticate
 *          access to itself.
 * INPUT  : lport.
 * OUTPUT : AuthConfigEntry.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextDot1xAuthConfigEntry(
    UI32_T *lport, DOT1X_AuthConfigEntry_T *AuthConfigPortEntry)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXacpe_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportXacpe_T   *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport   = *lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_NXT_1X_AUTH_CFG_ENTRY,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXacpe_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXacpe_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *lport = data_p->lport;
        *AuthConfigPortEntry = data_p->xacpe;
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetNextDot1xAuthDiagEntry
 *-------------------------------------------------------------------------
 * PURPOSE: Get next table that contains the diagnostics objects for the Authenticator
 *          PAE associated with each port.
 *          An entry appears in this table for each port that may authenticate
 *          access to itself.
 * INPUT  : lport.
 * OUTPUT : AuthDiagPortEntry.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextDot1xAuthDiagEntry(
    UI32_T *lport, DOT1X_AuthDiagEntry_T *AuthDiagPortEntry)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXadpe_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportXadpe_T   *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport   = *lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_NXT_1X_AUTH_DIAG_ENTRY,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXadpe_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXadpe_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *lport = data_p->lport;
        *AuthDiagPortEntry = data_p->xadpe;
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetNextDot1xAuthStatsEntry
 *-------------------------------------------------------------------------
 * PURPOSE: Get next table that contains the statistics objects for the Authenticator
 *          PAE associated with each port.
 *          An entry appears in this table for each port that may authenticate
 *          access to itself.
 * INPUT  : lport.
 * OUTPUT : AuthStateEntry.
 * RETURN : TRUE  -- The port support 802.1x
 *          FALSE -- The port don't support 802.1x.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextDot1xAuthStatsEntry(
    UI32_T *lport, DOT1X_AuthStatsEntry_T *AuthStatePortEntry)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXaspe_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportXaspe_T   *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport   = *lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_NXT_1X_AUTH_STS_ENTRY,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXaspe_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXaspe_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *lport = data_p->lport;
        *AuthStatePortEntry = data_p->xaspe;
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetNextDot1xPaePortEntry
 *-------------------------------------------------------------------------
 * PURPOSE: Get next table of system level information for each port supported
 *          by the Port Access Entity. An entry appears in this table for
 *          each port of this system.
 * INPUT  : lport.
 * OUTPUT : PaePortEntry.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextDot1xPaePortEntry(
    UI32_T *lport, DOT1X_PaePortEntry_T *PaePortEntry)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXpape_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportXpape_T   *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport   = *lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_NXT_1X_PAE_PORT_ENTRY,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXpape_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXpape_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *lport          = data_p->lport;
        *PaePortEntry   = data_p->xpape;
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetDot1xPortAuthServerTimeout
 *-------------------------------------------------------------------------
 * PURPOSE: Get the value ,in seconds ,of the serverTimeout constant currently
 *          in use by the Backend Authentication state machine.
 * INPUT  : lport.
 * OUTPUT : None.
 * RETURN : seconds.
 * NOTES  :
 *-------------------------------------------------------------------------
 */
UI32_T NETACCESS_PMGR_GetDot1xPortAuthServerTimeout(UI32_T lport, UI32_T *timeout)
{
    return NETACCESS_PMGR_GetUI32DataByLport (
                NETACCESS_MGR_IPC_CMD_GET_1X_PORT_AUTH_SEVR_TOUT, lport, timeout);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetDot1xPortAuthSuppTimeout
 *-------------------------------------------------------------------------
 * PURPOSE: Get the value ,in seconds ,of the suppTimeout constant currently
 *          in use by the Backend Authentication state machine.
 * INPUT  : lport.
 * OUTPUT : None.
 * RETURN : seconds.
 * NOTES  :
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortAuthSuppTimeout(UI32_T lport, UI32_T *timeout)
{
    return NETACCESS_PMGR_GetUI32DataByLport (
                NETACCESS_MGR_IPC_CMD_GET_1X_PORT_AUTH_SUPP_TOUT, lport, timeout);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xPortAuthorized
 *-------------------------------------------------------------------------
 * PURPOSE:  Get dot1x port authentication status.
 * INPUT  :  lport.
 * OUTPUT :  None.
 * RETURN :  VAL_dot1xAuthAuthControlledPortStatus_unauthorized  -- n/a (Unauthorized)
 *           VAL_dot1xAuthAuthControlledPortStatus_authorized  -- yes (Authorized)
 * NOTE   :  None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortAuthorized(UI32_T lport, NETACCESS_MGR_Dot1XAuthControlledPortStatus_T *port_status)
{
    return NETACCESS_PMGR_GetControlPortByLport (
                NETACCESS_MGR_IPC_CMD_GET_1X_PORT_AUTHORIZED, lport, port_status);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xPortControlMode
 *-------------------------------------------------------------------------
 * PURPOSE: get the port control mode of 1X configuration
 * INPUT  : lport -- logic port number
 * OUTPUT : control_mode -- control mode
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : control_mode (define in leaf_Ieee8021x.h):
 *                VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized
 *                VAL_dot1xAuthAuthControlledPortControl_forceAuthorized
 *                VAL_dot1xAuthAuthControlledPortControl_auto
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortControlMode(UI32_T lport, UI32_T *control_mode)
{
    return NETACCESS_PMGR_GetUI32DataByLport (
                NETACCESS_MGR_IPC_CMD_GET_1X_PORT_CTRL_MODE, lport, control_mode);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xPortMaxReq
 *-------------------------------------------------------------------------
 * PURPOSE: Get port MaxReq of 1X configuration
 * INPUT  : lport
 * OUTPUT : times
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortMaxReq(UI32_T lport, UI32_T *times)
{
    return NETACCESS_PMGR_GetUI32DataByLport (
                NETACCESS_MGR_IPC_CMD_GET_1X_PORT_MAX_REQ, lport, times);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xPortOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE: get the port operation mode of 1X configuration
 * INPUT  : lport -- logic port number
 * OUTPUT : mode -- operation mode
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass   (Single-Host)
 *          DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortOperationMode(UI32_T lport, UI32_T *mode)
{
    return NETACCESS_PMGR_GetUI32DataByLport (
                NETACCESS_MGR_IPC_CMD_GET_1X_PORT_OPER_MODE, lport, mode);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetNextDot1xPortOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE: get the next port operation mode of 1X configuration
 * INPUT  : lport -- logic port number
 * OUTPUT : lport -- logic port number
 *          mode -- operation mode
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass   (Single-Host)
 *          DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextDot1xPortOperationMode(UI32_T *lport, UI32_T *mode)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportData_T    *data_p;

    if ((NULL == lport) || (NULL == mode)) return FALSE;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport  = *lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_NEXT_1X_PORT_OPER_MODE,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *lport = data_p->lport;
        *mode = data_p->data;
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xPortQuietPeriod
 *-------------------------------------------------------------------------
 * PURPOSE: Get port QuietPeriod of 1X configuration
 * INPUT  : lport
 * OUTPUT : seconds
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortQuietPeriod(UI32_T lport, UI32_T *seconds)
{
    return NETACCESS_PMGR_GetUI32DataByLport (
                NETACCESS_MGR_IPC_CMD_GET_1X_PORT_QUIET_PERIOD, lport, seconds);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xPortReAuthEnabled
 *-------------------------------------------------------------------------
 * PURPOSE: Get port re-authentication status
 * INPUT  : lport
 * OUTPUT : control
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : control (define in leaf_Ieee8021x.h):
 *     VAL_dot1xPaePortReauthenticate_true  for Enable re-authentication
 *     VAL_dot1xPaePortReauthenticate_false for Disable re-authentication
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortReAuthEnabled(UI32_T lport ,UI32_T *control)
{
    return NETACCESS_PMGR_GetUI32DataByLport (
                NETACCESS_MGR_IPC_CMD_GET_1X_PORT_REAUTH_ENABLED, lport, control);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetDot1xPortReAuthMax
 *-------------------------------------------------------------------------
 * PURPOSE: Get the port's re-auth max
 * INPUT  : lport.
 * OUTPUT : None.
 * RETURN : PortReAuthMax.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortReAuthMax(UI32_T lport, UI32_T *reauth_max)
{
    return NETACCESS_PMGR_GetUI32DataByLport (
                NETACCESS_MGR_IPC_CMD_GET_1X_PORT_REAUTH_MAX, lport, reauth_max);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xPortReAuthPeriod
 *-------------------------------------------------------------------------
 * PURPOSE: Get port ReAuthPeriod of 1X configuration
 * INPUT  : lport
 * OUTPUT : seconds
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortReAuthPeriod(UI32_T lport,UI32_T *seconds)
{
    return NETACCESS_PMGR_GetUI32DataByLport (
                NETACCESS_MGR_IPC_CMD_GET_1X_PORT_REAUTH_PERIOD, lport, seconds);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xPortTxPeriod
 *-------------------------------------------------------------------------
 * PURPOSE: Get port TxPeriod of 1X configuration
 * INPUT  : lport
 * OUTPUT : seconds
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortTxPeriod(UI32_T lport,UI32_T *seconds)
{
    return NETACCESS_PMGR_GetUI32DataByLport (
                NETACCESS_MGR_IPC_CMD_GET_1X_PORT_TX_PERIOD, lport, seconds);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetNextDot1xSessionStatsEntry
 *-------------------------------------------------------------------------
 * PURPOSE: Get next session statistics information for an Authenticatior PAE.
 *          This shows the current values being collected for each session
 *          that is still in progress, or the final values for the last valid
 *          session on each port where there is no session currently active.
 * INPUT  : lport.
 * OUTPUT : AuthSessionStatsPortEntry.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextDot1xSessionStatsEntry(
    UI32_T *lport, DOT1X_AuthSessionStatsEntry_T *AuthSessionStatsPortEntry)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXasse_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportXasse_T   *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport   = *lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_NXT_1X_SESS_STS_ENTRY,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXasse_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXasse_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *lport = data_p->lport;
        *AuthSessionStatsPortEntry = data_p->xasse;
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xPortAdminCtrlDirections
 *-------------------------------------------------------------------------
 * PURPOSE: Set the value of the administrative controlled directions
 *          parameter for the port.
 * INPUT  : lport,direction.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : direction =   VAL_dot1xAuthAdminControlledDirections_both for Both
 *                        VAL_dot1xAuthAdminControlledDirections_in for In
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortAdminCtrlDirections(UI32_T lport, UI32_T dir)
{
    return NETACCESS_PMGR_SetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_SET_1X_PORT_ADMIN_CTRL_DIR, lport, dir);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_SetDot1xPortAuthServerTimeout
 *-------------------------------------------------------------------------
 * PURPOSE: Set the value ,in seconds ,of the serverTimeout constant currently
            in use by the Backend Authentication state machine.
 * INPUT  : lport,seconds.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  :
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortAuthServerTimeout(UI32_T lport, UI32_T timeout)
{
    return NETACCESS_PMGR_SetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_SET_1X_PORT_AUTH_SEVR_TOUT, lport, timeout);
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_SetDot1xPortAuthSuppTimeout
 *-------------------------------------------------------------------------
 * PURPOSE: Set the value ,in seconds ,of the suppTimeout constant currently
 *          in use by the Backend Authentication state machine.
 * INPUT  : lport,timeout.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  :
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortAuthSuppTimeout(UI32_T lport, UI32_T timeout)
{
    return NETACCESS_PMGR_SetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_SET_1X_PORT_AUTH_SUPP_TOUT, lport, timeout);
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME: NETACCESS_PMGR_SetDot1xPortAuthTxEnabled
 * ------------------------------------------------------------------------
 * PURPOSE: Set the value of the keyTransmissionEnabled constant currently
 *          in use by the Authenticator PAE state machine.
 * INPUT  : lport - port number
 *          value - VAL_dot1xAuthKeyTxEnabled_true or
 *                  VAL_dot1xAuthKeyTxEnabled_false
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortAuthTxEnabled(UI32_T lport, UI32_T value)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportData_T   *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport = lport;
    data_p->data = value;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_SET_1X_PORT_AUTH_TX_ENABLED,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                           (UI32_T) FALSE);

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xPortControlMode
 *-------------------------------------------------------------------------
 * PURPOSE: set the port control mode of 1X configuration
 * INPUT  : lport -- logic port number
 *          control_mode -- control mode
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : control_mode (define in leaf_Ieee8021x.h):
 *                VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized
 *                VAL_dot1xAuthAuthControlledPortControl_forceAuthorized
 *                VAL_dot1xAuthAuthControlledPortControl_auto
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortControlMode(UI32_T lport, UI32_T control_mode)
{
    return NETACCESS_PMGR_SetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_SET_1X_PORT_CTRL_MODE, lport, control_mode);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xPortDetails
 * ------------------------------------------------------------------------
 * PURPOSE: Get dot1x detail information.
 * INPUT  : lport
 * OUTPUT : port_details_p
 * RETURN : TRUE  -- enabled
 *          FALSE -- disabled
 * NOTE   : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortDetails(
    UI32_T lport, DOT1X_PortDetails_T *port_details_p)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXpdtl_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportXpdtl_T   *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport   = lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_1X_PORT_DETAILS,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(UI32_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportXpdtl_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *port_details_p = data_p->xpdtl;
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xPortMaxReAuthReq
 *-------------------------------------------------------------------------
 * PURPOSE: Set port Max-ReAuth Req of 1X configuration
 * INPUT  : lport,times
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortMaxReAuthReq(UI32_T lport, UI32_T times)
{
    return NETACCESS_PMGR_SetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_SET_1X_PORT_REAUTH_MAX, lport, times);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xPortMaxReq
 *-------------------------------------------------------------------------
 * PURPOSE: Set port MaxReq of 1X configuration
 * INPUT  : lport,times
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortMaxReq(UI32_T lport, UI32_T times)
{
    return NETACCESS_PMGR_SetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_SET_1X_PORT_MAX_REQ, lport, times);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xPortMultiHostMacCount
 *-------------------------------------------------------------------------
 * PURPOSE: set the max allowed MAC number in multi-host mode
 * INPUT  : lport -- logic port number
 *          count -- max mac count for multi-host mode
 * OUTPUT : none.
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortMultiHostMacCount(UI32_T lport, UI32_T count)
{
    return NETACCESS_PMGR_SetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_SET_1X_PORT_MULHOST_MAC_CNT, lport, count);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xPortMultiHostMacCount
 *-------------------------------------------------------------------------
 * PURPOSE: Get the max allowed MAC number in multi-host mode
 * INPUT  : lport -- logic port number
 *          count -- max mac count for multi-host mode
 * OUTPUT : none.
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortMultiHostMacCount(UI32_T lport, UI32_T *count)
{
    return NETACCESS_PMGR_GetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_GET_1X_PORT_MULHOST_MAC_CNT, lport, count);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetNextDot1xPortMultiHostMacCount
 *-------------------------------------------------------------------------
 * PURPOSE: Get the next max allowed MAC number in multi-host mode
 * INPUT  : lport -- logic port number
 * OUTPUT : lport -- logic port number
 *          count -- max mac count for multi-host mode
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextDot1xPortMultiHostMacCount(UI32_T *lport, UI32_T *count)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportData_T    *data_p;

    if ((NULL == lport) || (NULL == count)) return FALSE;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport  = *lport;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_GET_NEXT_1X_PORT_MULHOST_MAC_CNT,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *lport = data_p->lport;
        *count = data_p->data;
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xPortOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE: set the port operation mode of 1X configuration
 * INPUT  : lport -- logic port number
 *          mode -- operation mode
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass   (Single-Host)
 *          DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortOperationMode(UI32_T lport,UI32_T mode)
{
    return NETACCESS_PMGR_SetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_SET_1X_PORT_OPER_MODE, lport, mode);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xPortPaePortInitialize
 *-------------------------------------------------------------------------
 * PURPOSE: Set this attribute TRUE causes the Port to be initialized.
 *          the attribute value reverts to FALSE once initialization has completed.
 * INPUT  : lport,control.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : control =   VAL_dot1xPaePortInitialize_true for Enable Initialize
 *                      VAL_dot1xPaePortInitialize_false Disable Initialize
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortPaePortInitialize(UI32_T lport, UI32_T control)
{
    return NETACCESS_PMGR_SetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_SET_1X_PORT_PAE_PORT_INIT, lport, control);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetDot1xPortQuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: set port QuietPeriod of 1X configuration
 * INPUT  : lport,seconds
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortQuietPeriod(UI32_T lport, UI32_T seconds)
{
    return NETACCESS_PMGR_SetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_SET_1X_PORT_QUIET_PERIOD, lport, seconds);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xPortReAuthEnabled
 *-------------------------------------------------------------------------
 * PURPOSE: enable/disable port period re-authentication of the 1X client,
 *          which is disabled by default
 * INPUT  : lport,control
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : control (define in leaf_Ieee8021x.h):
 *     VAL_dot1xPaePortReauthenticate_true  for Enable re-authentication
 *     VAL_dot1xPaePortReauthenticate_false for Disable re-authentication
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortReAuthEnabled(UI32_T lport, UI32_T control)
{
    return NETACCESS_PMGR_SetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_SET_1X_PORT_REAUTH_ENABLED, lport, control);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xPortReAuthPeriod
 *-------------------------------------------------------------------------
 * PURPOSE: set port ReAuthPeriod of 1X configuration
 * INPUT  : lport,seconds
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortReAuthPeriod(UI32_T lport, UI32_T seconds)
{
    return NETACCESS_PMGR_SetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_SET_1X_PORT_REAUTH_PERIOD, lport, seconds);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xPortTxPeriod
 *-------------------------------------------------------------------------
 * PURPOSE: Set port TxPeriod of 1X configuration
 * INPUT  : lport,seconds
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortTxPeriod(UI32_T lport, UI32_T seconds)
{
    return NETACCESS_PMGR_SetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_SET_1X_PORT_TX_PERIOD, lport, seconds);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xSystemAuthControl
 *-------------------------------------------------------------------------
 * PURPOSE: set system auth control of 1X configuration
 * INPUT  : control_status
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : control_status (define in leaf_Ieee8021x.h):
 *         VAL_dot1xPaeSystemAuthControl_enabled for Enable SystemAuthControl
 *         VAL_dot1xPaeSystemAuthControl_disabled for Disable SystemAuthControl
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xSystemAuthControl(UI32_T control_status)
{
    return NETACCESS_PMGR_SetUI32Data(
        NETACCESS_MGR_IPC_CMD_SET_1X_SYS_AUTH_CTRL, control_status);
}

#if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetDot1xEapolPassThrough
 * ---------------------------------------------------------------------
 * PURPOSE  : Set status of EAPOL frames pass-through when the global
 *            status dot1x is disabled
 * INPUT    : status    - VAL_dot1xEapolPassThrough_enabled(1)
 *                        VAL_dot1xEapolPassThrough_disabled(2)
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xEapolPassThrough(UI32_T status)
{
    return NETACCESS_PMGR_SetUI32Data(
        NETACCESS_MGR_IPC_CMD_SET_1X_EAPOL_PASS_TRHOU, status);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetDot1xEapolPassThrough
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the intrusion action to determine the action if an unauthorized device
 *            transmits on this port.
 * INPUT    : lport
 * OUTPUT   : action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                             VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * RETURN   : TRUE/FALSE.
 * NOTES    : none.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xEapolPassThrough(UI32_T *status)
{
    return NETACCESS_PMGR_GetUI32Data(
        NETACCESS_MGR_IPC_CMD_GET_1X_EAPOL_PASS_TRHOU, status);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningDot1xEapolPassThrough
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the intrusion action to determine the action if an unauthorized device
 *            transmits on this port.
 * INPUT    : lport
 * OUTPUT   : action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                             VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : none.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningDot1xEapolPassThrough(UI32_T *status)
{
    return NETACCESS_PMGR_GetRunningUI32Data(
        NETACCESS_MGR_IPC_CMD_GET_RUNN_1X_EAPOL_PASS_TRHOU, status);
}
#endif /* #if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE) */


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetPortSecurityActionStatus
 *-------------------------------------------------------------------------
 * PURPOSE: This function will set port security action status
 * INPUT  : lport          -- which port to
 *          action_status  -- action status
 * OUTPUT : None
 * RETURN : TRUE: Successfully, FALSE: If not available
 * NOTE   : if the port is not in portSecurity port mode,will return FALSE
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetPortSecurityActionStatus (UI32_T lport, UI32_T action_status)
{
    return NETACCESS_PMGR_SetUI32DataByLport (
        NETACCESS_MGR_IPC_CMD_SET_PSEC_ACTN_STAS, lport, action_status);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetPortSecurityMaxMacCount
 *-------------------------------------------------------------------------
 * PURPOSE: This function will set port auto learning mac counts
 * INPUT  : lport          -- which port to
 *          max_mac_count  -- maximum mac learning count
 * OUTPUT : None
 * RETURN : TRUE: Successfully, FALSE: If not available
 * NOTE   : Default : max_mac_count = 1
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetPortSecurityMaxMacCount(UI32_T lport, UI32_T max_mac_count)
{
    return NETACCESS_PMGR_SetUI32DataByLport (
        NETACCESS_MGR_IPC_CMD_SET_PSEC_MAX_MAC_CNT, lport, max_mac_count);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetPortSecurityStatus
 *-------------------------------------------------------------------------
 * PURPOSE: This function will Set the port security status
 * INPUT  : lport                  - interface index
 *          portsec_status         - VAL_portSecPortStatus_enabled
 *                                   VAL_portSecPortStatus_disabled
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : Port security doesn't support
 *          1) unknown port, 2) trunk member, and 3) trunk port
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetPortSecurityStatus(UI32_T lport, UI32_T portsec_status)
{
    return NETACCESS_PMGR_SetUI32DataByLport (
        NETACCESS_MGR_IPC_CMD_SET_PSEC_STAS, lport, portsec_status);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_PMGR_ConvertPortSecuritySecuredAddressIntoManual
 *------------------------------------------------------------------------
 * FUNCTION: To convert port security learnt secured MAC address into manual
 *           configured on the specified interface. If interface is
 *           PSEC_MGR_INTERFACE_INDEX_FOR_ALL, it will apply to all interface.
 * INPUT   : ifindex  -- interface
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T
NETACCESS_PMGR_ConvertPortSecuritySecuredAddressIntoManual(
    UI32_T ifindex)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_Interface_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_Interface_T *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->ifindex = ifindex;

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_CONVERT_PSEC_SECURED_ADDRESS_INTO_MANUAL,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_Interface_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                           (UI32_T)FALSE);

    return (BOOL_T)NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/* begin for dot1x supplicant
 */

#if (SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetLinkDetectionStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Set the link detection status on this port.
 * INPUT:  lport, status
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES: VAL_networkAccessPortLinkDetectionStatus_enabled
 *        VAL_networkAccessPortLinkDetectionStatus_disabled
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetLinkDetectionStatus(UI32_T lport, UI32_T status)
{
    return NETACCESS_PMGR_SetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_SET_LINK_DETECTION_STATUS, lport, status);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetLinkDetectionStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Get the link detection status on this port.
 * INPUT:  lport
 * OUTPUT: status_p
 * RETURN: TRUE/FALSE
 * NOTES: VAL_networkAccessPortLinkDetectionStatus_enabled
 *        VAL_networkAccessPortLinkDetectionStatus_disabled
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetLinkDetectionStatus(UI32_T lport, UI32_T *status_p)
{
    return NETACCESS_PMGR_GetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_GET_LINK_DETECTION_STATUS, lport, status_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningLinkDetectionStatus
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default link detection status is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport
 * OUTPUT:  status_p
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default intrusion action.
 *        3. Caller has to prepare buffer for storing link detection status
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningLinkDetectionStatus(UI32_T lport, UI32_T *status_p)
{
    return NETACCESS_PMGR_GetRunningUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_GET_RUNNING_LINK_DETECTION_STATUS, lport, status_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetLinkDetectionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set the link detection mode on this port.
 * INPUT:  lport, mode
 * OUTPUT: None
 * RETURN: TRUE/FALSE.
 * NOTES: VAL_networkAccessPortLinkDetectionMode_linkUp
 *        VAL_networkAccessPortLinkDetectionMode_linkDown
 *        VAL_networkAccessPortLinkDetectionMode_linkUpDown
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetLinkDetectionMode(UI32_T lport, UI32_T mode)
{
    return NETACCESS_PMGR_SetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_SET_LINK_DETECTION_MODE, lport, mode);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetLinkDetectionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the link detection mode on this port.
 * INPUT:  lport
 * OUTPUT: mode_p
 * RETURN: TRUE/FALSE.
 * NOTES: VAL_networkAccessPortLinkDetectionMode_linkUp
 *        VAL_networkAccessPortLinkDetectionMode_linkDown
 *        VAL_networkAccessPortLinkDetectionMode_linkUpDown
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetLinkDetectionMode(UI32_T lport, UI32_T *mode_p)
{
    return NETACCESS_PMGR_GetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_GET_LINK_DETECTION_MODE, lport, mode_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningLinkDetectionMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default link detection mode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport
 * OUTPUT:  mode_p
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default intrusion action.
 *        3. Caller has to prepare buffer for storing link detection mode
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningLinkDetectionMode(UI32_T lport, UI32_T *mode_p)
{
    return NETACCESS_PMGR_GetRunningUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_GET_RUNNING_LINK_DETECTION_MODE, lport, mode_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetLinkDetectionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Set the link detection action on this port.
 * INPUT:  lport, action
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES: action:
 *        VAL_networkAccessPortLinkDetectionAciton_trap
 *        VAL_networkAccessPortLinkDetectionAciton_shutdown
 *        VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetLinkDetectionAction(UI32_T lport, UI32_T action)
{
    return NETACCESS_PMGR_SetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_SET_LINK_DETECTION_ACTION, lport, action);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetLinkDetectionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Get the link detection action on this port.
 * INPUT:  lport
 * OUTPUT: action_p
 * RETURN: TRUE/FALSE.
 * NOTES: action:
 *        VAL_networkAccessPortLinkDetectionAciton_trap
 *        VAL_networkAccessPortLinkDetectionAciton_shutdown
 *        VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetLinkDetectionAction(UI32_T lport, UI32_T *action_p)
{
    return NETACCESS_PMGR_GetUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_GET_LINK_DETECTION_ACTION, lport, action_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningLinkDetectionAction
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default link detection action is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport
 * OUTPUT:  mode_p
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default intrusion action.
 *        3. Caller has to prepare buffer for storing link detection action
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningLinkDetectionAction(UI32_T lport, UI32_T *action_p)
{
    return NETACCESS_PMGR_GetRunningUI32DataByLport(
        NETACCESS_MGR_IPC_CMD_GET_RUNNING_LINK_DETECTION_ACTION, lport, action_p);
}
#endif /* #if (SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE) */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_AuthenticatePacket
 * ---------------------------------------------------------------------
 * PURPOSE:Asynchronous call to authenticate packet
 * INPUT:  cookie
 * OUTPUT: None.
 * RETURN: TRUE/FALSE
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_AuthenticatePacket(void  *cookie)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_AuthenticatePacket_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_AuthenticatePacket_T   *data_p;

    if(NULL == cookie)
        return FALSE;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    memcpy(data_p, cookie, sizeof(*data_p));

    NETACCESS_PMGR_SendMsg(NETACCESS_MGR_IPC_CMD_AUTHENTICATE_PACKET,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_AuthenticatePacket_T),
                           0,
                           (UI32_T) FALSE);

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/* LOCAL SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_SendMsg
 *-------------------------------------------------------------------------
 * PURPOSE: To send an IPC message.
 * INPUT  : cmd       - the NETACCESS message command.
 *          msg_p     - the buffer of the IPC message.
 *          req_size  - the size of NETACCESS request message.
 *          res_size  - the size of NETACCESS response message.
 *          ret_val   - the return value to set when IPC is failed.
 * OUTPUT : msg_p     - the response message.
 * RETURN : None
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static void NETACCESS_PMGR_SendMsg(
    UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val)
{
    UI32_T ret;

    msg_p->cmd = SYS_MODULE_NETACCESS;
    msg_p->msg_size = req_size;

    NETACCESS_MGR_MSG_CMD(msg_p) = cmd;

    if(cmd < NETACCESS_MGR_IPC_CMD_ASYNC_CALL)
    {
        ret = SYSFUN_SendRequestMsg(ipcmsgq_handle,
                                msg_p,
                                SYSFUN_TIMEOUT_WAIT_FOREVER,
                                SYSFUN_SYSTEM_EVENT_IPCMSG,
                                res_size,
                                msg_p);
    }
    else
    {
        ret = SYSFUN_SendRequestMsg(ipcmsgq_handle,
                                msg_p,
                                SYSFUN_TIMEOUT_NOWAIT,
                                SYSFUN_SYSTEM_EVENT_IPCMSG,
                                0,
                                NULL);
    }

    /* If IPC is failed, set return value as ret_val.
     */
    if (ret != SYSFUN_OK)
        NETACCESS_MGR_MSG_RETVAL(msg_p) = ret_val;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_GetUI32DataByLport
 *-------------------------------------------------------------------------
 * PURPOSE: To get a required data by specified lport and ipc_cmd.
 * INPUT  : ipc_cmd   - the NETACCESS IPC message command
 *          lport     - lport
 * OUTPUT : out_p     - the output buffer of required data
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static BOOL_T NETACCESS_PMGR_GetUI32DataByLport(
    UI32_T ipc_cmd, UI32_T lport, UI32_T *out_p)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportData_T    *data_p;

    if (NULL == out_p) return FALSE;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport  = lport;

    NETACCESS_PMGR_SendMsg(ipc_cmd,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *out_p = data_p->data;
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_GetUI32DataByLport
 *-------------------------------------------------------------------------
 * PURPOSE: To get a required data by specified lport and ipc_cmd.
 * INPUT  : ipc_cmd   - the NETACCESS IPC message command
 *          lport     - lport
 * OUTPUT : out_p     - the output buffer of required data
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningUI32DataByLport(
    UI32_T ipc_cmd, UI32_T lport, UI32_T *out_p)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportData_T    *data_p;

    if (NULL == out_p) return FALSE;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport  = lport;

    NETACCESS_PMGR_SendMsg(ipc_cmd,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           (UI32_T) FALSE);

    *out_p = data_p->data;
    return (SYS_TYPE_Get_Running_Cfg_T)NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_GetControlPortByLport
 *-------------------------------------------------------------------------
 * PURPOSE: To get a required data by specified lport and ipc_cmd.
 * INPUT  : ipc_cmd   - the NETACCESS IPC message command
 *          lport     - lport
 * OUTPUT : out_p     - the output buffer of required data
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static BOOL_T NETACCESS_PMGR_GetControlPortByLport(
    UI32_T ipc_cmd, UI32_T lport, NETACCESS_MGR_Dot1XAuthControlledPortStatus_T *out_p)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportControlPort_T *data_p;

    if (NULL == out_p) return FALSE;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport  = lport;

    NETACCESS_PMGR_SendMsg(ipc_cmd,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportControlPort_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportControlPort_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *out_p = data_p->port_control;
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_SetUI32DataByLport
 *-------------------------------------------------------------------------
 * PURPOSE: To set a required data by specified lport and ipc_cmd.
 * INPUT  : ipc_cmd   - the NETACCESS IPC message command
 *          lport     - lport
 *          data      - the data to set
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static BOOL_T NETACCESS_PMGR_SetUI32DataByLport(
    UI32_T ipc_cmd, UI32_T lport, UI32_T data)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_LportData_T    *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->lport  = lport;
    data_p->data   = data;

    NETACCESS_PMGR_SendMsg(ipc_cmd,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                           (UI32_T) FALSE);

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_GetUI32Data
 *-------------------------------------------------------------------------
 * PURPOSE: To get a required data by specified lport and ipc_cmd.
 * INPUT  : ipc_cmd   - the NETACCESS IPC message command
 *          lport     - lport
 * OUTPUT : out_p     - the output buffer of required data
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static BOOL_T NETACCESS_PMGR_GetUI32Data(UI32_T ipc_cmd, UI32_T *out_p)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_U32Data_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_U32Data_T  *data_p;

    if (NULL == out_p) return FALSE;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);

    NETACCESS_PMGR_SendMsg(ipc_cmd,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p))
    {
        *out_p = data_p->data;
    }

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_GetRunningUI32Data
 *-------------------------------------------------------------------------
 * PURPOSE: To get a required data by specified lport and ipc_cmd.
 * INPUT  : ipc_cmd   - the NETACCESS IPC message command
 *          lport     - lport
 * OUTPUT : out_p     - the output buffer of required data
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningUI32Data(UI32_T ipc_cmd, UI32_T *out_p)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_U32Data_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_U32Data_T  *data_p;

    if (NULL == out_p) return FALSE;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);

    NETACCESS_PMGR_SendMsg(ipc_cmd,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_LportData_T),
                           (UI32_T) FALSE);

    *out_p = data_p->data;

    return (SYS_TYPE_Get_Running_Cfg_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_SetUI32Data
 *-------------------------------------------------------------------------
 * PURPOSE: To set a required data by specified lport and ipc_cmd.
 * INPUT  : ipc_cmd   - the NETACCESS IPC message command
 *          lport     - lport
 *          data      - the data to set
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static BOOL_T NETACCESS_PMGR_SetUI32Data(UI32_T ipc_cmd, UI32_T data)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_U32Data_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MGR_IPCMsg_U32Data_T  *data_p;

    data_p = NETACCESS_MGR_MSG_DATA(msg_p);
    data_p->data   = data;

    NETACCESS_PMGR_SendMsg(ipc_cmd,
                           msg_p,
                           NETACCESS_MGR_GET_MSGBUFSIZE(NETACCESS_MGR_IPCMsg_U32Data_T),
                           NETACCESS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                           (UI32_T) FALSE);

    return (BOOL_T) NETACCESS_MGR_MSG_RETVAL(msg_p);
}

#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
