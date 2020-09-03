/*-----------------------------------------------------------------------------
 * Module   : ntp_pmgr.c
 *-----------------------------------------------------------------------------
 * PURPOSE  : Provide APIs to access NTP control functions.
 *-----------------------------------------------------------------------------
 * NOTES    :
 *
 *-----------------------------------------------------------------------------
 * HISTORY  : 11/13/2007 - Squid Ro, Create
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "sys_type.h"
#include "sys_module.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "ntp_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void NTP_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : NTP_PMGR_CopyStringByLen
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    prevent copy wrong memory length
 * INPUT:
 *    org_str_p-- input string.
 *    str_len  -- input string length
 *
 * OUTPUT:
 *    dest_str_p-- output string
 *
 * RETURN:
 *    TRUE/FALSE
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
static BOOL_T
NTP_PMGR_CopyStringByLen(
    char *dest_str_p,
    const char *org_str_p,
    UI32_T str_len
    );

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T   ipcmsgq_handle;

/* MACRO FUNCTIONS DECLARACTION
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE : Initiate resource used in the calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T NTP_PMGR_InitiateProcessResources(void)
{
    if (SYSFUN_GetMsgQ(NTP_MGR_IPCMSG_KEY, SYSFUN_MSGQ_BIDIRECTIONAL,
                       &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ fail.", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_SetStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Set NTP status : enable/disable
 * INPUT    : status you want to set. 1: VAL_ntpStatus_enabled,
 *                                    2: VAL_ntpStatus_disabled
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_SetStatus(UI32_T status)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Status_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_Status_T     *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);
    data_p->status = status;

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_SET_STATUS,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Status_T),
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T) FALSE);

    return (BOOL_T) NTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_GetStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get NTP status  : enable/disable
 * INPUT    : none
 * OUTPUT   : current NTP status. 1: VAL_ntpStatus_enabled,
 *                                 2: VAL_ntpStatus_disabled
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_GetStatus(UI32_T *status)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Status_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_Status_T    *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_GET_STATUS,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Status_T),
                      (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NTP_MGR_MSG_RETVAL(msg_p))
    {
        *status = data_p->status;
    }

    return (BOOL_T) NTP_MGR_MSG_RETVAL(msg_p);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NTP_PMGR_GetRunningStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the status of disable/enable mapping of system
 * INPUT:    None
 * OUTPUT:   status -- VAL_ntpStatus_enabled/VAL_ntpStatus_disabled
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:     default value is disable  1:enable, 2:disable
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NTP_PMGR_GetRunningStatus(UI32_T *status)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Status_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_Status_T    *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_GET_RUNN_STATUS,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Status_T),
                      SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == NTP_MGR_MSG_RETVAL(msg_p))
    {
        *status = data_p->status;
    }

    return NTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_GetLastUpdateTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Return the last update time for show ntp
 * INPUT    :
 * OUTPUT   :
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_GetLastUpdateTime(I32_T *time)
{
    UI8_T                             msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_LastUpdateTime_T))];
    SYSFUN_Msg_T                      *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_LastUpdateTime_T   *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_GET_LAST_UPDATE_TIME,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_LastUpdateTime_T),
                      (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NTP_MGR_MSG_RETVAL(msg_p))
    {
        *time = data_p->time;
    }

    return (BOOL_T) NTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_GetPollTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get the polling interval
 * INPUT    : A buffer is used to be put data
 * OUTPUT   : polling interval used in unicast mode
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_GetPollTime(UI32_T *polltime)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_PollTime_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_PollTime_T  *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_GET_POLL_TIME,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_PollTime_T),
                      (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NTP_MGR_MSG_RETVAL(msg_p))
    {
        *polltime = data_p->poll_time;
    }

    return (BOOL_T) NTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_SetServiceOperationMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set ntp service operation mode , unicast/brocast/anycast/disable
 * INPUT    : VAL_ntpServiceMode_unicast = 1
 *            VAL_ntpServiceMode_broadcast = 2
 *            VAL_ntpServiceMode_anycast = 3
 *
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_SetServiceOperationMode(UI32_T mode)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_ServMode_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_ServMode_T  *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);
    data_p->serv_mode = mode;

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_SET_SV_OPMODE,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_ServMode_T),
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T) FALSE);

    return (BOOL_T) NTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_GetServiceOperationMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set ntp service operation mode , Unicast/brocast/anycast
 * INPUT    : VAL_ntpServiceMode_unicast = 1
 *            VAL_ntpServiceMode_broadcast = 2
 *            VAL_ntpServiceMode_anycast = 3
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_GetServiceOperationMode(UI32_T *mode)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_ServMode_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_ServMode_T  *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_GET_SV_OPMODE,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_ServMode_T),
                      (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NTP_MGR_MSG_RETVAL(msg_p))
    {
        *mode = data_p->serv_mode;
    }

    return (BOOL_T) NTP_MGR_MSG_RETVAL(msg_p);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NTP_PMGR_GetRunningServiceMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the operation mode mapping of system
 * INPUT:    None
 * OUTPUT:   VAL_ntpServiceMode_unicast = 1
 *           VAL_ntpServiceMode_broadcast = 2
 *           VAL_ntpServiceMode_anycast = 3
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE: default value is unicast
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NTP_PMGR_GetRunningServiceMode(UI32_T *servicemode)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_ServMode_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_ServMode_T  *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_GET_RUNN_SV_OPMODE,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_ServMode_T),
                      SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == NTP_MGR_MSG_RETVAL(msg_p))
    {
        *servicemode = data_p->serv_mode;
    }

    return NTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_AddServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Add a ntp server ip to OM
 * INPUT    : 1.index 2. ip address
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : Ex: index1=10.1.1.1 index2=NULL index3=NULL
 *            if we want to add an IP to index3, we can not add it
 *            becasue index2=NULL
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_AddServerIp(UI32_T ipaddress, UI32_T version, UI32_T keyid)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Server_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_Server_T             *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);
    data_p->ip_addr = ipaddress;
    data_p->version = version;
    data_p->keyid = keyid;

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_ADD_SVR_IP,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Server_T),
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T) FALSE);

    return (BOOL_T) NTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_DeleteServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete a server ip from OM
 * INPUT    : index (MIN_ntpServerIndex <= index <= MAX_ntpServerIndex)
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : 1.Delete one server ip will cause the ip behind deleted-ip was deleted.
 *            2.The ip is zero if it is deleted
 *            3. If ntp srv ip are 192.168.1.1, 192.168.1.2, 192.168.1.3.
 *               If delete 192.168.1.2, sort om to 192.168.1.1, 192.168.1.3, 0.0.0.0.
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_DeleteServerIp(UI32_T ipaddress)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_IpAddr_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_IpAddr_T     *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);
    data_p->ip_addr = ipaddress;

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_DEL_SVR_IP,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_IpAddr_T),
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T) FALSE);

    return (BOOL_T) NTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_DeleteAllServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete all servers ip from OM
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_DeleteAllServerIp(void)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_DEL_ALL_SVR_IP,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T) FALSE);

    return (BOOL_T) NTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_GetNextServer
 *------------------------------------------------------------------------------
 * PURPOSE  : Get a next server entry  from OM using ip address as index
 * INPUT    : 1.point to the server list.
 * OUTPUT   : buffer contain information
 * RETURN   : TRUE : If success
 *            FALSE: If get failed, or the assigned ip was cleared.
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_GetNextServer(UI32_T *ipaddress, UI32_T *version, UI32_T *keyid)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Server_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_Server_T     *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);
    data_p->ip_addr = *ipaddress;

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_GET_NEXT_SVR,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Server_T),
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Server_T),
                      FALSE);

    if (TRUE == NTP_MGR_MSG_RETVAL(msg_p))
    {
        *ipaddress = data_p->ip_addr;
        *version = data_p->version;
        *keyid = data_p->keyid;
    }

    return NTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_GetLastUpdateServer
 *------------------------------------------------------------------------------
 * PURPOSE  : Return the last update server for show ntp
 * INPUT    :
 * OUTPUT   :
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_GetLastUpdateServer(NTP_MGR_SERVER_T *serv)
{
    UI8_T                          msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Server_Entry_T))];
    SYSFUN_Msg_T                   *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_Server_Entry_T  *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_GET_LAST_UPDATE_SVR,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Server_Entry_T),
                      FALSE);

    if (TRUE == NTP_MGR_MSG_RETVAL(msg_p))
    {
        *serv = data_p->server_entry;
    }

    return NTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
  * FUNCTION NAME - NTP_PMGR_FindServer
  *------------------------------------------------------------------------------
  * PURPOSE  : Get a server entry  from OM using ip address as index
  * INPUT    : ip address
  * OUTPUT   : buffer contain information
  * RETURN   : TRUE : If find
  *            FALSE: If not found
  * NOTES    : This is only used in cli
  *------------------------------------------------------------------------------*/
BOOL_T  NTP_PMGR_FindServer(UI32_T ipaddress, NTP_MGR_SERVER_T *server)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Find_Server_Entry_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_Find_Server_Entry_T  *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);
    data_p->ip_addr = ipaddress;

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_FIND_SVR,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Find_Server_Entry_T),
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Find_Server_Entry_T),
                      FALSE);

    if (TRUE == NTP_MGR_MSG_RETVAL(msg_p))
    {
        *server = data_p->server_entry;
    }

    return NTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_FindNextServer
 *------------------------------------------------------------------------------
 * PURPOSE  : Get a next server entry  from OM using ip address as index
 * INPUT    : 1.point to the server list.
 * OUTPUT   : buffer contain information
 * RETURN   : TRUE : If success
 *            FALSE: If get failed, or the assigned ip was cleared.
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_FindNextServer(UI32_T ipadd, NTP_MGR_SERVER_T *serv)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Find_Server_Entry_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_Find_Server_Entry_T  *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);
    data_p->ip_addr = ipadd;

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_FIND_NEXT_SVR,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Find_Server_Entry_T),
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Find_Server_Entry_T),
                      FALSE);

    if (TRUE == NTP_MGR_MSG_RETVAL(msg_p))
    {
        *serv = data_p->server_entry;
    }

    return NTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_SetAuthStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Set NTP authenticate status : enable/disable
 * INPUT    : status you want to set. 1 :VAL_ntpAuthenticateStatus_enabled, 2 :VAL_ntpAuthenticateStatus_disabled
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_SetAuthStatus(UI32_T status)
{
    UI8_T                         msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Auth_Status_T))];
    SYSFUN_Msg_T                  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_Auth_Status_T  *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);
    data_p->status = status;

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_SET_AUTH_STATUS,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Auth_Status_T),
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T) FALSE);

    return (BOOL_T) NTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_GetAuthStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get NTP authenticate status  : enable/disable
 * INPUT    : none
 * OUTPUT   : current NTP status.1 :VAL_ntpAuthenticateStatus_enabled, 2: VAL_ntpAuthenticateStatus_disabled
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_GetAuthStatus(UI32_T *status)
{
    UI8_T                         msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Auth_Status_T))];
    SYSFUN_Msg_T                  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_Auth_Status_T  *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_GET_AUTH_STATUS,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Auth_Status_T),
                      (UI32_T) FALSE);

    if (TRUE == (BOOL_T) NTP_MGR_MSG_RETVAL(msg_p))
    {
        *status = data_p->status;
    }

    return (BOOL_T) NTP_MGR_MSG_RETVAL(msg_p);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NTP_PMGR_GetRunningAuthStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the authenticate status of disable/enable mapping of system
 * INPUT:    None
 * OUTPUT:   status -- VAL_ntpAuthenticateStatus_enabled/VAL_ntpAuthenticateStatus_disabled
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:     default value is disable  1:enable, 2:disable
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  NTP_PMGR_GetRunningAuthStatus(UI32_T *status)
{
    UI8_T                         msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Auth_Status_T))];
    SYSFUN_Msg_T                  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_Auth_Status_T  *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_GET_RUNN_AUTH_STATUS,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Auth_Status_T),
                      SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == NTP_MGR_MSG_RETVAL(msg_p))
    {
        *status = data_p->status;
    }

    return NTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_PMGR_AddAuthKey
*------------------------------------------------------------------------------
* PURPOSE  : Add an authentication key to the list
* INPUT    : 1.index 2. md5_key
* OUTPUT   : none
* RETURN   : TRUE : If success
*            FALSE:
* NOTES    : none
*------------------------------------------------------------------------------*/
BOOL_T  NTP_PMGR_AddAuthKey(UI32_T index, char *encryptedkey)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Auth_Key_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_Auth_Key_T           *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);
    data_p->keyid = index;

    if (FALSE == NTP_PMGR_CopyStringByLen(data_p->auth_key,
                                          encryptedkey,
                                          sizeof(data_p->auth_key)))
    {
        return FALSE;
    }

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_ADD_AUTH_KEY,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Auth_Key_T),
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T) FALSE);

    return (BOOL_T) NTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_PMGR_AddAuthKey_Encrypted
*------------------------------------------------------------------------------
* PURPOSE  : Add an authentication encrypted key to the list ,use in provison
* INPUT    : 1.index 2. encryptedkey
* OUTPUT   : none
* RETURN   : TRUE : If success
*            FALSE:
* NOTES    : none
*------------------------------------------------------------------------------*/
BOOL_T  NTP_PMGR_AddAuthKey_Encrypted(UI32_T index, char *encryptedkey)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Auth_Key_Encrypted_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_Auth_Key_Encrypted_T *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);
    data_p->keyid = index;
    if (FALSE == NTP_PMGR_CopyStringByLen(data_p->auth_key_encrypted,
                                          encryptedkey,
                                          sizeof(data_p->auth_key_encrypted)))
    {
        return FALSE;
    }

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_ADD_AUTH_KEY_ENCRYPTED,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Auth_Key_Encrypted_T),
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T) FALSE);

    return (BOOL_T) NTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_PMGR_SetAuthKeyStatus
*------------------------------------------------------------------------------
* PURPOSE  : Set an authentication key status
* INPUT    : 1.index 2. status : VAL_ntpAuthKeyStatus_valid/VAL_ntpAuthKeyStatus_invalid
* OUTPUT   : none
* RETURN   : TRUE : If success
*            FALSE:
* NOTES    : only used for snmp do set authentication key status
*------------------------------------------------------------------------------*/
BOOL_T  NTP_PMGR_SetAuthKeyStatus(UI32_T index, UI32_T status)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Auth_Key_Status_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_Auth_Key_Status_T    *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);
    data_p->keyid = index;
    data_p->status = status;

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_SET_AUTH_KEY_STATUS,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Auth_Key_Status_T),
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T) FALSE);

    return (BOOL_T) NTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_DeleteAuthKey
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete a designed Authentication key
 * INPUT    : keyid
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T  NTP_PMGR_DeleteAuthKey(UI32_T keyid)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Auth_Key_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_Auth_Key_T   *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);
    data_p->keyid = keyid;

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_DEL_AUTH_KEY,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Auth_Key_T),
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T) FALSE);

    return (BOOL_T) NTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_DeleteAllAuthKey
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete all Authenticaion key
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T  NTP_PMGR_DeleteAllAuthKey(void)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_DEL_ALL_AUTH_KEY,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T) FALSE);

    return (BOOL_T) NTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_PMGR_GetNextKey
*------------------------------------------------------------------------------
* PURPOSE  : Get next key
* INPUT    :
* OUTPUT   :
* RETURN   : TRUE : If find
*            FALSE: If not found
* NOTES    : none
*------------------------------------------------------------------------------*/
BOOL_T  NTP_PMGR_GetNextKey(NTP_MGR_AUTHKEY_T *authkey)
{
    UI8_T                            msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Auth_Key_Entry_T))];
    SYSFUN_Msg_T                     *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_Auth_Key_Entry_T  *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);
    memcpy(&data_p->auth_entry, authkey, sizeof(data_p->auth_entry));

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_GET_NEXT_KEY,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Auth_Key_Entry_T),
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Auth_Key_Entry_T),
                      FALSE);

    if (TRUE == NTP_MGR_MSG_RETVAL(msg_p))
    {
        memcpy(authkey, &data_p->auth_entry, sizeof(*authkey));
    }

    return NTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_PMGR_FindKey
*------------------------------------------------------------------------------
* PURPOSE  : check whether the key exist
* INPUT    : keyid
* OUTPUT   :
* RETURN   : TRUE : If find
*            FALSE: If not found
* NOTES    : none
*------------------------------------------------------------------------------*/
BOOL_T  NTP_PMGR_FindKey(UI32_T keyid, NTP_MGR_AUTHKEY_T *authkey)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Find_Key_Entry_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NTP_MGR_IPCMsg_Find_Key_Entry_T     *data_p;

    data_p = NTP_MGR_MSG_DATA(msg_p);
    data_p->keyid = keyid;

    NTP_PMGR_SendMsg(NTP_MGR_IPC_CMD_FIND_KEY,
                      msg_p,
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Find_Key_Entry_T),
                      NTP_MGR_GET_MSGBUFSIZE(NTP_MGR_IPCMsg_Find_Key_Entry_T),
                      FALSE);

    if (TRUE == NTP_MGR_MSG_RETVAL(msg_p))
    {
        *authkey = data_p->auth_entry;
    }

    return NTP_MGR_MSG_RETVAL(msg_p);
}

/* LOCAL SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_SendMsg
 *-------------------------------------------------------------------------
 * PURPOSE : To send an IPC message.
 * INPUT   : cmd       - the NTP message command.
 *           msg_p     - the buffer of the IPC message.
 *           req_size  - the size of NTP request message.
 *           res_size  - the size of NTP response message.
 *           ret_val   - the return value to set when IPC is failed.
 * OUTPUT  : msg_p     - the response message.
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
static void NTP_PMGR_SendMsg(UI32_T           cmd,
                             SYSFUN_Msg_T    *msg_p,
                             UI32_T          req_size,
                             UI32_T          res_size,
                             UI32_T          ret_val)
{
    UI32_T  ret;

    msg_p->cmd = SYS_MODULE_NTP;
    msg_p->msg_size = req_size;
    NTP_MGR_MSG_CMD(msg_p) = cmd;
    ret = SYSFUN_SendRequestMsg(ipcmsgq_handle,
                                msg_p,
                                SYSFUN_TIMEOUT_WAIT_FOREVER,
                                SYSFUN_SYSTEM_EVENT_IPCMSG,
                                res_size,
                                msg_p);

    /* If IPC is failed, set return value as ret_val. */
    if (ret != SYSFUN_OK)
    {
        NTP_MGR_MSG_RETVAL(msg_p) = ret_val;
    }
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : NTP_PMGR_CopyStringByLen
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    prevent copy wrong memory length
 * INPUT:
 *    org_str_p-- input string.
 *    str_len  -- input string length
 *
 * OUTPUT:
 *    dest_str_p-- output string
 *
 * RETURN:
 *    TRUE/FALSE
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
static BOOL_T
NTP_PMGR_CopyStringByLen(
    char *dest_str_p,
    const char *org_str_p,
    UI32_T str_len)
{
    if ((NULL == org_str_p) ||
        (NULL == dest_str_p))
    {
        return FALSE;
    }

    dest_str_p[str_len - 1] = '\0';
    strncpy(dest_str_p, org_str_p, str_len);

    if (dest_str_p[str_len - 1] != '\0')
    {
        dest_str_p[str_len - 1] = '\0';

        return FALSE;
    }

    return TRUE;
}

/* End of this file */

