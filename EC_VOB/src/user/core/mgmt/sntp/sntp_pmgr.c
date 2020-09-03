/*-----------------------------------------------------------------------------
 * Module   : sntp_pmgr.c
 *-----------------------------------------------------------------------------
 * PURPOSE  : Provide APIs to access SNTP control functions.
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
#include "sntp_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void SNTP_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val);

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T   ipcmsgq_handle;

/* MACRO FUNCTIONS DECLARACTION
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE : Initiate resource used in the calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T SNTP_PMGR_InitiateProcessResources(void)
{
    if (SYSFUN_GetMsgQ(SNTP_MGR_IPCMSG_KEY, SYSFUN_MSGQ_BIDIRECTIONAL,
                       &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ fail.", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_AddServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Add a sntp server ip to OM
 * INPUT    : 1.index 2. ip address
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : Ex: index1=10.1.1.1 index2=NULL index3=NULL
 *            if we want to add an IP to index3, we can not add it
 *            becasue index2=NULL
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_PMGR_AddServerIp(UI32_T index, L_INET_AddrIp_T *ipaddress)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_IpAddrWithIndex_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SNTP_MGR_IPCMsg_IpAddrWithIndex_T   *data_p;

    data_p = SNTP_MGR_MSG_DATA(msg_p);
    data_p->index = index;
    memcpy(&data_p->ip_addr, ipaddress, sizeof(L_INET_AddrIp_T));

    SNTP_PMGR_SendMsg(SNTP_MGR_IPC_CMD_ADD_SVR_IP,
                      msg_p,
                      SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_IpAddrWithIndex_T),
                      SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T) FALSE);

    return (BOOL_T) SNTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_AddServerIpForCLI
 *------------------------------------------------------------------------------
 * PURPOSE  : Add a sntp server ip to OM
 * INPUT    : ipaddress
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_PMGR_AddServerIpForCLI(L_INET_AddrIp_T *ipaddress)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_IpAddr_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SNTP_MGR_IPCMsg_IpAddr_T    *data_p;

    data_p = SNTP_MGR_MSG_DATA(msg_p);
    memcpy(&data_p->ip_addr, ipaddress, sizeof(L_INET_AddrIp_T));

    SNTP_PMGR_SendMsg(SNTP_MGR_IPC_CMD_ADD_SVR_IP_FOR_CLI,
                      msg_p,
                      SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_IpAddr_T),
                      SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T) FALSE);

    return (BOOL_T) SNTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_DeleteAllServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete all servers ip from OM
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_PMGR_DeleteAllServerIp(void)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;

    SNTP_PMGR_SendMsg(SNTP_MGR_IPC_CMD_DEL_ALL_SVR_IP,
                      msg_p,
                      SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T) FALSE);

    return (BOOL_T) SNTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_DeleteServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete a server ip from OM
 * INPUT    : index (MIN_sntpServerIndex <= index <= MAX_sntpServerIndex)
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : 1.Delete one server ip will cause the ip behind deleted-ip was deleted.
 *            2.The ip is zero if it is deleted
 *            3. If sntp srv ip are 192.168.1.1, 192.168.1.2, 192.168.1.3.
 *               If delete 192.168.1.2, sort om to 192.168.1.1, 192.168.1.3, 0.0.0.0.
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_PMGR_DeleteServerIp(UI32_T index)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_SvrIndex_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SNTP_MGR_IPCMsg_SvrIndex_T  *data_p;

    data_p = SNTP_MGR_MSG_DATA(msg_p);
    data_p->index = index;

    SNTP_PMGR_SendMsg(SNTP_MGR_IPC_CMD_DEL_SVR_IP,
                      msg_p,
                      SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_SvrIndex_T),
                      SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T) FALSE);

    return (BOOL_T) SNTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_DeleteServerIpForCLI
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete a server ip from OM
 * INPUT    : ipaddress
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : Delete one server ip will cause the SNTP server address entry be relocated,
 *            such as: server entry  : 10.1.1.1, 10.1.1.2, 10.1.1.3 remove 10.1.1.2
 *                                 --> 10.1.1.1, 10.1.1.3
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_PMGR_DeleteServerIpForCLI(L_INET_AddrIp_T *ipaddress)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_IpAddr_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SNTP_MGR_IPCMsg_IpAddr_T    *data_p;

    data_p = SNTP_MGR_MSG_DATA(msg_p);
    memcpy(&data_p->ip_addr, ipaddress, sizeof(L_INET_AddrIp_T));

    SNTP_PMGR_SendMsg(SNTP_MGR_IPC_CMD_DEL_SVR_IP_FOR_CLI,
                      msg_p,
                      SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_IpAddr_T),
                      SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T) FALSE);

    return (BOOL_T) SNTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_GetCurrentServer
 *------------------------------------------------------------------------------
 * PURPOSE  : Get current server of getting current time
 * INPUT    : buffer to get server
 * OUTPUT   : ip address
 * RETURN   : TRUE : If success
 *            FALSE: If failed
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_PMGR_GetCurrentServer(L_INET_AddrIp_T *ipaddress)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_IpAddr_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SNTP_MGR_IPCMsg_IpAddr_T    *data_p;

    data_p = SNTP_MGR_MSG_DATA(msg_p);

    SNTP_PMGR_SendMsg(SNTP_MGR_IPC_CMD_GET_CUR_SVR_IP,
                      msg_p,
                      SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_IpAddr_T),
                      (UI32_T) FALSE);

    if (TRUE == (BOOL_T) SNTP_MGR_MSG_RETVAL(msg_p))
    {
        memcpy(ipaddress, &data_p->ip_addr, sizeof(L_INET_AddrIp_T));
    }

    return (BOOL_T) SNTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_GetPollTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get the polling interval
 * INPUT    : A buffer is used to be put data
 * OUTPUT   : polling interval used in unicast mode
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_PMGR_GetPollTime(UI32_T *polltime)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_PollTime_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SNTP_MGR_IPCMsg_PollTime_T  *data_p;

    data_p = SNTP_MGR_MSG_DATA(msg_p);

    SNTP_PMGR_SendMsg(SNTP_MGR_IPC_CMD_GET_POLL_TIME,
                      msg_p,
                      SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_PollTime_T),
                      (UI32_T) FALSE);

    if (TRUE == (BOOL_T) SNTP_MGR_MSG_RETVAL(msg_p))
    {
        *polltime = data_p->poll_time;
    }

    return (BOOL_T) SNTP_MGR_MSG_RETVAL(msg_p);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_PMGR_GetRunningPollTime
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the status of disable/enable mapping of system
 * INPUT:    None
 * OUTPUT:   polling time
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:    default value is 16 secconds
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SNTP_PMGR_GetRunningPollTime(UI32_T *polltime)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_PollTime_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SNTP_MGR_IPCMsg_PollTime_T  *data_p;

    data_p = SNTP_MGR_MSG_DATA(msg_p);

    SNTP_PMGR_SendMsg(SNTP_MGR_IPC_CMD_GET_RUNN_POLL_TIME,
                      msg_p,
                      SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_PollTime_T),
                      SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == SNTP_MGR_MSG_RETVAL(msg_p))
    {
        *polltime = data_p->poll_time;
    }

    return SNTP_MGR_MSG_RETVAL(msg_p);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_PMGR_GetRunningServiceMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the operation mode mapping of system
 * INPUT:    None
 * OUTPUT:   VAL_sntpServiceMode_unicast = 1
 *           VAL_sntpServiceMode_broadcast = 2
 *           VAL_sntpServiceMode_anycast = 3
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE: default value is unicast
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SNTP_PMGR_GetRunningServiceMode(UI32_T *servicemode)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_ServMode_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SNTP_MGR_IPCMsg_ServMode_T  *data_p;

    data_p = SNTP_MGR_MSG_DATA(msg_p);

    SNTP_PMGR_SendMsg(SNTP_MGR_IPC_CMD_GET_RUNN_SV_OPMODE,
                      msg_p,
                      SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_ServMode_T),
                      SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == SNTP_MGR_MSG_RETVAL(msg_p))
    {
        *servicemode = data_p->serv_mode;
    }

    return SNTP_MGR_MSG_RETVAL(msg_p);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_PMGR_GetRunningStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the status of disable/enable mapping of system
 * INPUT:    None
 * OUTPUT:   status -- VAL_sntpStatus_enabled/VAL_sntpStatus_disabled
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:     default value is disable  1:enable, 2:disable
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SNTP_PMGR_GetRunningStatus(UI32_T *status)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_Status_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SNTP_MGR_IPCMsg_Status_T    *data_p;

    data_p = SNTP_MGR_MSG_DATA(msg_p);

    SNTP_PMGR_SendMsg(SNTP_MGR_IPC_CMD_GET_RUNN_STATUS,
                      msg_p,
                      SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_Status_T),
                      SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == SNTP_MGR_MSG_RETVAL(msg_p))
    {
        *status = data_p->status;
    }

    return SNTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_GetServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Get a server entry  from OM using ip address as index
 * INPUT    : 1.index (MIN_sntpServerIndex <= index <= MAX_sntpServerIndex)
 *            2.point to buffer that can contain information of a server
 * OUTPUT   : buffer contain information
 * RETURN   : TRUE : If success
 *            FALSE: If get failed.
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_PMGR_GetServerIp(UI32_T index, L_INET_AddrIp_T *ipaddress)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_IpAddrWithIndex_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SNTP_MGR_IPCMsg_IpAddrWithIndex_T   *data_p;

    data_p = SNTP_MGR_MSG_DATA(msg_p);
    data_p->index = index;

    SNTP_PMGR_SendMsg(SNTP_MGR_IPC_CMD_GET_SVR_IP_BY_IDX,
                      msg_p,
                      SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_IpAddrWithIndex_T),
                      SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_IpAddrWithIndex_T),
                      (UI32_T) FALSE);

    if (TRUE == (BOOL_T) SNTP_MGR_MSG_RETVAL(msg_p))
    {
        memcpy(ipaddress, &data_p->ip_addr, sizeof(L_INET_AddrIp_T));
    }

    return (BOOL_T) SNTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_GetServiceOperationMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set sntp service operation mode , Unicast/brocast/anycast
 * INPUT    : VAL_sntpServiceMode_unicast = 1
 *            VAL_sntpServiceMode_broadcast = 2
 *            VAL_sntpServiceMode_anycast = 3
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_PMGR_GetServiceOperationMode(UI32_T *mode)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_ServMode_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SNTP_MGR_IPCMsg_ServMode_T  *data_p;

    data_p = SNTP_MGR_MSG_DATA(msg_p);

    SNTP_PMGR_SendMsg(SNTP_MGR_IPC_CMD_GET_SV_OPMODE,
                      msg_p,
                      SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_ServMode_T),
                      (UI32_T) FALSE);

    if (TRUE == (BOOL_T) SNTP_MGR_MSG_RETVAL(msg_p))
    {
        *mode = data_p->serv_mode;
    }

    return (BOOL_T) SNTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_GetStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get SNTP status  : enable/disable
 * INPUT    : none
 * OUTPUT   : current SNTP status. 1: VAL_sntpStatus_enabled,
 *                                 2: VAL_sntpStatus_disabled
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_PMGR_GetStatus(UI32_T *status)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_Status_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SNTP_MGR_IPCMsg_Status_T    *data_p;

    data_p = SNTP_MGR_MSG_DATA(msg_p);

    SNTP_PMGR_SendMsg(SNTP_MGR_IPC_CMD_GET_STATUS,
                      msg_p,
                      SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_Status_T),
                      (UI32_T) FALSE);

    if (TRUE == (BOOL_T) SNTP_MGR_MSG_RETVAL(msg_p))
    {
        *status = data_p->status;
    }

    return (BOOL_T) SNTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_SetPollTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Set the polling interval
 * INPUT    : polling interval used in unicast mode
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_PMGR_SetPollTime(UI32_T polltime)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_PollTime_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SNTP_MGR_IPCMsg_PollTime_T  *data_p;

    data_p = SNTP_MGR_MSG_DATA(msg_p);
    data_p->poll_time = polltime;

    SNTP_PMGR_SendMsg(SNTP_MGR_IPC_CMD_SET_POLL_TIME,
                      msg_p,
                      SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_PollTime_T),
                      SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T) FALSE);

    return (BOOL_T) SNTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_SetServiceOperationMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set sntp service operation mode , unicast/brocast/anycast/disable
 * INPUT    : VAL_sntpServiceMode_unicast = 1
 *            VAL_sntpServiceMode_broadcast = 2
 *            VAL_sntpServiceMode_anycast = 3
 *
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_PMGR_SetServiceOperationMode(UI32_T mode)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_ServMode_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SNTP_MGR_IPCMsg_ServMode_T  *data_p;

    data_p = SNTP_MGR_MSG_DATA(msg_p);
    data_p->serv_mode = mode;

    SNTP_PMGR_SendMsg(SNTP_MGR_IPC_CMD_SET_SV_OPMODE,
                      msg_p,
                      SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_ServMode_T),
                      SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T) FALSE);

    return (BOOL_T) SNTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_SetStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Set SNTP status : enable/disable
 * INPUT    : status you want to set. 1: VAL_sntpStatus_enabled,
 *                                    2: VAL_sntpStatus_disabled
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_PMGR_SetStatus(UI32_T status)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_Status_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SNTP_MGR_IPCMsg_Status_T    *data_p;

    data_p = SNTP_MGR_MSG_DATA(msg_p);
    data_p->status = status;

    SNTP_PMGR_SendMsg(SNTP_MGR_IPC_CMD_SET_STATUS,
                      msg_p,
                      SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_Status_T),
                      SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T) FALSE);

    return (BOOL_T) SNTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_GetNextServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Get next server entry using ip address as index
 * INPUT    : 1.index (start from 0)
 *            2.point to buffer that can contain server ip address
 * OUTPUT   : buffer contain information
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : if input index is 0, then it will find first entry in table.
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_PMGR_GetNextServerIp(UI32_T *index, L_INET_AddrIp_T *ipaddress)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_IpAddrWithIndex_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SNTP_MGR_IPCMsg_IpAddrWithIndex_T   *data_p;

    data_p = SNTP_MGR_MSG_DATA(msg_p);
    data_p->index = *index;

    SNTP_PMGR_SendMsg(SNTP_MGR_IPC_CMD_GET_NEXT_SVR_IP_BY_IDX,
                      msg_p,
                      SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_IpAddrWithIndex_T),
                      SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_IpAddrWithIndex_T),
                      (UI32_T) FALSE);

    if (TRUE == (BOOL_T) SNTP_MGR_MSG_RETVAL(msg_p))
    {
        *index = data_p->index;
        memcpy(ipaddress, &data_p->ip_addr, sizeof(L_INET_AddrIp_T));
    }

    return (BOOL_T) SNTP_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_DeleteServerIpForSNMP
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete a server ip from OM
 * INPUT      : index
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : Delete one server ip will cause the SNTP server address entry be relocated,
 *            such as: server entry  : 10.1.1.1, 10.1.1.2, 10.1.1.3 remove 10.1.1.2
 *                                 --> 10.1.1.1, 10.1.1.3
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_PMGR_DeleteServerIpForSNMP(UI32_T index)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_SvrIndex_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SNTP_MGR_IPCMsg_SvrIndex_T  *data_p;

    data_p = SNTP_MGR_MSG_DATA(msg_p);
    data_p->index = index;

    SNTP_PMGR_SendMsg(SNTP_MGR_IPC_CMD_DEL_SVR_IP_FOR_SNMP,
                      msg_p,
                      SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_SvrIndex_T),
                      SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T) FALSE);

    return (BOOL_T) SNTP_MGR_MSG_RETVAL(msg_p);
}

/* LOCAL SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_SendMsg
 *-------------------------------------------------------------------------
 * PURPOSE : To send an IPC message.
 * INPUT   : cmd       - the SNTP message command.
 *           msg_p     - the buffer of the IPC message.
 *           req_size  - the size of SNTP request message.
 *           res_size  - the size of SNTP response message.
 *           ret_val   - the return value to set when IPC is failed.
 * OUTPUT  : msg_p     - the response message.
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
static void SNTP_PMGR_SendMsg(UI32_T          cmd,
                              SYSFUN_Msg_T    *msg_p,
                              UI32_T          req_size,
                              UI32_T          res_size,
                              UI32_T          ret_val)
{
    UI32_T  ret;

    msg_p->cmd = SYS_MODULE_SNTP;
    msg_p->msg_size = req_size;

    SNTP_MGR_MSG_CMD(msg_p) = cmd;

    ret = SYSFUN_SendRequestMsg(ipcmsgq_handle,
                                msg_p,
                                SYSFUN_TIMEOUT_WAIT_FOREVER,
                                SYSFUN_SYSTEM_EVENT_IPCMSG,
                                res_size,
                                msg_p);

    /* If IPC is failed, set return value as ret_val. */
    if (ret != SYSFUN_OK)
    {
        SNTP_MGR_MSG_RETVAL(msg_p) = ret_val;
    }
}

/* End of this file */

