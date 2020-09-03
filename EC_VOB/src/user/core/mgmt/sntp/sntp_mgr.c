/* static char SccsId[] = "+-<>?!SNTP_MGR.C   22.1  22/04/02  15:00:00";
 * ------------------------------------------------------------------------
 *  FILE NAME  -  SNTP_MGR.C
 * ------------------------------------------------------------------------
 *  ABSTRACT:
 *
 *  Modification History:
 *  Modifier           Date        Description
 *  -----------------------------------------------------------------------
 *  S.K.Yang              04-22-2002  Created
 * ------------------------------------------------------------------------
 *  Copyright(C)                                Accton Corporation, 1999
 * ------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>     /* for memset */
#include "sys_type.h"
#include "sysfun.h"
#include "sntp_om.h"
#include "sntp_mgr.h"
#include "sntp_type.h"
#include "sntp_dbg.h"   /* for debug use */
#include "sntp_txrx.h"
#include "l_stdlib.h"
#include "l_inet.h"
#include "ip_lib.h"
#include "sys_time.h"
#include "sys_bld.h"
#include "sys_dflt.h"

/* For Exceptional Handler */
#include "sys_module.h"
#include "syslog_type.h"
#include "eh_type.h"
#include "eh_mgr.h"
/* end For Exceptional Handler */

#include "dev_nicdrv_pmgr.h"

#if (SYS_CPNT_NTP == TRUE)
#include "ntp_mgr.h"
#endif

/*
 * NAMING CONSTANT DECLARATIONS
 */
#define SNTP_MGR_FIRE_IMMEDIATLY    0xffffffff

/* MACRO FUNCTION DECLARATIONS
 */
#define SNTP_MGR_USE_CSC_CHECK_OPER_MODE(RET_VAL)                         \
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE) \
    {                                                                     \
        return (RET_VAL);                                                 \
    }

#define SNTP_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE()           \
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE) \
    {                                                                     \
        return;                                                           \
    }

#define SNTP_MGR_RETURN_AND_RELEASE_CSC(RET_VAL) \
    {                                            \
        return (RET_VAL);                        \
    }

#define SNTP_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE() \
    {                                                          \
        return;                                                \
    }

/*
 * DATA TYPE DECLARATIONS ;
 */

/*
 * STATIC VARIABLE DECLARATION
 */
static UI32_T   hz;
static UI32_T   local_timer;
static UI32_T   Fire;
static UI32_T   exponential_backoff_timer;
static BOOL_T   is_exponential_backoff_timer_start;

/*--------------------------------------------------------------------------------------------
 * Begin of Data base type definition
 *--------------------------------------------------------------------------------------------
 */

/*
 * TYPE DECLARATIONS
 */
enum SNTP_MGR_FUN_NO_E
{
    SNTP_MGR_STATUS_FUNC_NO         = 1,
    SNTP_MGR_SERVICE_MODE_FUNC_NO,
    SNTP_MGR_POLL_TIME_FUNC_NO,
    SNTP_MGR_INVALID_IP_FUNC_NO,
    SNTP_MGR_SERVER_IP_FUNC_NO,
    SNTP_MGR_TIME_FUNC_NO,
    SNTP_MGR_SERVER_FUNC_NO
};

/*
 * STATIC VARIABLE DECLARATIONS ;
 */

/*
 * STATIC LOCAL PROGRAM
 */
static BOOL_T   SNTP_MGR_PerformOperationMode(L_INET_AddrIp_T *ipaddress);
static BOOL_T   SNTP_MGR_UpdateSntpStatus(const SNTP_UPDATE_STATUS_T *STATUS);
static BOOL_T   SNTP_MGR_CheckServerIp(L_INET_AddrIp_T *ipaddress);

SYSFUN_DECLARE_CSC

/*
 * EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * FUNCTION NAME -  SNTP_MGR_Init
 *------------------------------------------------------------------------------
 * PURPOSE  : Initialize SNTP_MGR used system resource, eg. protection semaphore.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void SNTP_MGR_Init(void)
{
    local_timer = SYSFUN_GetSysTick();
    Fire = SNTP_MGR_FIRE_IMMEDIATLY;
    exponential_backoff_timer = 1;
    is_exponential_backoff_timer_start = FALSE;
    
    SNTP_OM_CreatSem();
    SNTP_OM_Initiate_System_Resources();
    DEV_NICDRV_PMGR_InitiateProcessResource();
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  SNTP_MGR_EnterMasterMode
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will make the SNTP_MGR enter the master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void SNTP_MGR_EnterMasterMode(void)
{
    /* Record the system state and send signal to SNTP_TASK */
    Fire = SNTP_MGR_FIRE_IMMEDIATLY;
    exponential_backoff_timer = 1;
    is_exponential_backoff_timer_start = FALSE;
    hz = SYS_BLD_TICKS_PER_SECOND;

    SNTP_OM_InitSntpDatabaseToDefaultValue();

    SYSFUN_ENTER_MASTER_MODE();
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_EnterTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will make the SNTP_MGR enter the transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void SNTP_MGR_EnterTransitionMode(void)
{
    /* Record the system state and send signal to SNTP_TASK */
    SYSFUN_ENTER_TRANSITION_MODE();

    local_timer = SYSFUN_GetSysTick();
    Fire = SNTP_MGR_FIRE_IMMEDIATLY;
    exponential_backoff_timer = 1;

    SNTP_OM_ClearSntpDatabase();
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  SNTP_MGR_EnterSlaveMode
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will make the SNTP_MGR enter the slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void SNTP_MGR_EnterSlaveMode(void)
{
    /* Record the system state and send signal to SNTP_TASK */
    SYSFUN_ENTER_SLAVE_MODE();
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  SNTP_MGR_SetTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void SNTP_MGR_SetTransitionMode(void)
{
    /* set transition flag to prevent calling request */
    SYSFUN_SET_TRANSITION_MODE();
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  SNTP_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE  : Handle the ipc request message for sntp mgr.
 * INPUT    : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT   : ipcmsg_p  --  input request ipc message buffer
 * RETURN   : TRUE  - There is a response need to send.
 *            FALSE - There is no response to send.
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    UI32_T  cmd;

    if (ipcmsg_p == NULL)
    {
        return FALSE;
    }

    /* Every ipc request will fail when operating mode is transition mode */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        SNTP_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;
        ipcmsg_p->msg_size = SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        return TRUE;
    }

    switch ((cmd = SNTP_MGR_MSG_CMD(ipcmsg_p)))
    {
        case SNTP_MGR_IPC_CMD_ADD_SVR_IP:
            {
                SNTP_MGR_IPCMsg_IpAddrWithIndex_T   *data_p = SNTP_MGR_MSG_DATA(ipcmsg_p);
                SNTP_MGR_MSG_RETVAL(ipcmsg_p) = SNTP_MGR_AddServerIp(data_p->index,
                                                                     &data_p->ip_addr);
                ipcmsg_p->msg_size = SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
                break;
            }

        case SNTP_MGR_IPC_CMD_ADD_SVR_IP_FOR_CLI:
            {
                SNTP_MGR_IPCMsg_IpAddr_T    *data_p = SNTP_MGR_MSG_DATA(ipcmsg_p);
                SNTP_MGR_MSG_RETVAL(ipcmsg_p) = SNTP_MGR_AddServerIpForCLI(&data_p->ip_addr);
                ipcmsg_p->msg_size = SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
                break;
            }

        case SNTP_MGR_IPC_CMD_DEL_ALL_SVR_IP:
            {
                SNTP_MGR_MSG_RETVAL(ipcmsg_p) = SNTP_MGR_DeleteAllServerIp();
                ipcmsg_p->msg_size = SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
                break;
            }

        case SNTP_MGR_IPC_CMD_DEL_SVR_IP:
            {
                SNTP_MGR_IPCMsg_SvrIndex_T  *data_p = SNTP_MGR_MSG_DATA(ipcmsg_p);
                SNTP_MGR_MSG_RETVAL(ipcmsg_p) = SNTP_MGR_DeleteServerIp(data_p->index);
                ipcmsg_p->msg_size = SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
                break;
            }

        case SNTP_MGR_IPC_CMD_DEL_SVR_IP_FOR_CLI:
            {
                SNTP_MGR_IPCMsg_IpAddr_T    *data_p = SNTP_MGR_MSG_DATA(ipcmsg_p);
                SNTP_MGR_MSG_RETVAL(ipcmsg_p) = SNTP_MGR_DeleteServerIpForCLI(&data_p->ip_addr);
                ipcmsg_p->msg_size = SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
                break;
            }

        case SNTP_MGR_IPC_CMD_GET_POLL_TIME:
            {
                SNTP_MGR_IPCMsg_PollTime_T  *data_p = SNTP_MGR_MSG_DATA(ipcmsg_p);
                SNTP_MGR_MSG_RETVAL(ipcmsg_p) = SNTP_MGR_GetPollTime(&data_p->poll_time);
                ipcmsg_p->msg_size = SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_PollTime_T);
                break;
            }

        case SNTP_MGR_IPC_CMD_GET_CUR_SVR_IP:
            {
                SNTP_MGR_IPCMsg_IpAddr_T    *data_p = SNTP_MGR_MSG_DATA(ipcmsg_p);
                SNTP_MGR_MSG_RETVAL(ipcmsg_p) = SNTP_MGR_GetCurrentServer(&data_p->ip_addr);
                ipcmsg_p->msg_size = SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_IpAddr_T);
                break;
            }

        case SNTP_MGR_IPC_CMD_GET_RUNN_POLL_TIME:
            {
                SNTP_MGR_IPCMsg_PollTime_T  *data_p = SNTP_MGR_MSG_DATA(ipcmsg_p);
                SNTP_MGR_MSG_RETVAL(ipcmsg_p) = SNTP_MGR_GetRunningPollTime(&data_p->poll_time);
                ipcmsg_p->msg_size = SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_PollTime_T);
                break;
            }

        case SNTP_MGR_IPC_CMD_GET_RUNN_SV_OPMODE:
            {
                SNTP_MGR_IPCMsg_ServMode_T  *data_p = SNTP_MGR_MSG_DATA(ipcmsg_p);
                SNTP_MGR_MSG_RETVAL(ipcmsg_p) = SNTP_MGR_GetRunningServiceMode(&data_p->serv_mode);
                ipcmsg_p->msg_size = SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_ServMode_T);
                break;
            }

        case SNTP_MGR_IPC_CMD_GET_RUNN_STATUS:
            {
                SNTP_MGR_IPCMsg_Status_T    *data_p = SNTP_MGR_MSG_DATA(ipcmsg_p);
                SNTP_MGR_MSG_RETVAL(ipcmsg_p) = SNTP_MGR_GetRunningStatus(&data_p->status);
                ipcmsg_p->msg_size = SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_Status_T);
                break;
            }

        case SNTP_MGR_IPC_CMD_GET_SVR_IP_BY_IDX:
            {
                SNTP_MGR_IPCMsg_IpAddrWithIndex_T   *data_p = SNTP_MGR_MSG_DATA(ipcmsg_p);
                SNTP_MGR_MSG_RETVAL(ipcmsg_p) = SNTP_MGR_GetServerIp(data_p->index,
                                                                     &data_p->ip_addr);
                ipcmsg_p->msg_size = SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_IpAddrWithIndex_T);
                break;
            }

        case SNTP_MGR_IPC_CMD_GET_SV_OPMODE:
            {
                SNTP_MGR_IPCMsg_ServMode_T  *data_p = SNTP_MGR_MSG_DATA(ipcmsg_p);
                SNTP_MGR_MSG_RETVAL(ipcmsg_p) = SNTP_MGR_GetServiceOperationMode(&data_p->serv_mode);
                ipcmsg_p->msg_size = SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_ServMode_T);
                break;
            }

        case SNTP_MGR_IPC_CMD_GET_STATUS:
            {
                SNTP_MGR_IPCMsg_Status_T    *data_p = SNTP_MGR_MSG_DATA(ipcmsg_p);
                SNTP_MGR_MSG_RETVAL(ipcmsg_p) = SNTP_MGR_GetStatus(&data_p->status);
                ipcmsg_p->msg_size = SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_Status_T);
                break;
            }

        case SNTP_MGR_IPC_CMD_SET_POLL_TIME:
            {
                SNTP_MGR_IPCMsg_PollTime_T  *data_p = SNTP_MGR_MSG_DATA(ipcmsg_p);
                SNTP_MGR_MSG_RETVAL(ipcmsg_p) = SNTP_MGR_SetPollTime(data_p->poll_time);
                ipcmsg_p->msg_size = SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
                break;
            }

        case SNTP_MGR_IPC_CMD_SET_SV_OPMODE:
            {
                SNTP_MGR_IPCMsg_ServMode_T  *data_p = SNTP_MGR_MSG_DATA(ipcmsg_p);
                SNTP_MGR_MSG_RETVAL(ipcmsg_p) = SNTP_MGR_SetServiceOperationMode(data_p->serv_mode);
                ipcmsg_p->msg_size = SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
                break;
            }

        case SNTP_MGR_IPC_CMD_SET_STATUS:
            {
                SNTP_MGR_IPCMsg_Status_T    *data_p = SNTP_MGR_MSG_DATA(ipcmsg_p);
                SNTP_MGR_MSG_RETVAL(ipcmsg_p) = SNTP_MGR_SetStatus(data_p->status);
                ipcmsg_p->msg_size = SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
                break;
            }

        case SNTP_MGR_IPC_CMD_GET_NEXT_SVR_IP_BY_IDX:
            {
                SNTP_MGR_IPCMsg_IpAddrWithIndex_T   *data_p = SNTP_MGR_MSG_DATA(ipcmsg_p);
                SNTP_MGR_MSG_RETVAL(ipcmsg_p) = SNTP_MGR_GetNextServerIp(&data_p->index,
                                                                         &data_p->ip_addr);
                ipcmsg_p->msg_size = SNTP_MGR_GET_MSGBUFSIZE(SNTP_MGR_IPCMsg_IpAddrWithIndex_T);
                break;
            }

        case SNTP_MGR_IPC_CMD_DEL_SVR_IP_FOR_SNMP:
            {
                SNTP_MGR_IPCMsg_SvrIndex_T    *data_p = SNTP_MGR_MSG_DATA(ipcmsg_p);
                SNTP_MGR_MSG_RETVAL(ipcmsg_p) = SNTP_MGR_DeleteServerIpForSNMP(data_p->index);
                ipcmsg_p->msg_size = SNTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
                break;
            }

        default:
            {
                SNTP_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;

                if (DBG_SNTP_TURN_MESSAGE_ON_OFF)
                {
                    printf("*** %s(): Invalid cmd.\n", __FUNCTION__);
                }
                break;
            }
    }                                   /* switch ipcmsg_p->cmd */

    if (SNTP_MGR_MSG_RETVAL(ipcmsg_p) == FALSE)
    {
        if (DBG_SNTP_TURN_MESSAGE_ON_OFF)
        {
            printf("*** %s(): [cmd: %ld] failed\n", __FUNCTION__, (long)cmd);
        }
    }

    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  SNTP_MGR_GetOperationMode
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will return the SNTP_MGR  mode. (slave/master/transition)
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T SNTP_MGR_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_SetStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Set SNTP status : enable/disable
 * INPUT    : status you want to set. 1 :VAL_sntpStatus_enabled, 2 :VAL_sntpStatus_disabled
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_MGR_SetStatus(UI32_T status)
{
    BOOL_T  ret;
    UI32_T  current_status;
#if (SYS_CPNT_NTP == TRUE)
    UI32_T  ntp_status;
#endif

    SNTP_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

#if (SYS_CPNT_NTP == TRUE)
    if (NTP_MGR_GetStatus(&ntp_status) != FALSE)
    {
        if ((ntp_status == VAL_ntpStatus_enabled) && (status == VAL_sntpStatus_enabled))
        {
            return FALSE;
        }
    }
#endif

    if (status == VAL_sntpStatus_enabled || status == VAL_sntpStatus_disabled)
    {
        ret = SNTP_MGR_GetStatus(&current_status);
        if (TRUE == ret)
        {
            if (status != current_status)
            {
                Fire = SNTP_MGR_FIRE_IMMEDIATLY;
                exponential_backoff_timer = 1;

                ret = SNTP_OM_SetStatus(status);
            }
        }
    }
    else
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_SNTP, SNTP_MGR_STATUS_FUNC_NO,
                                 EH_TYPE_MSG_FAILED_TO_SET, SYSLOG_LEVEL_ERR,
                                 "SNTP status");
        ret = FALSE;
    }

    SNTP_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_GetStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get SNTP status  : enable/disable
 * INPUT    : none
 * OUTPUT   : current SNTP status.1 :VAL_sntpStatus_enabled, 2: VAL_sntpStatus_disabled
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_MGR_GetStatus(UI32_T *status)
{
    BOOL_T  ret;

    if (status == NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_SNTP, SNTP_MGR_STATUS_FUNC_NO,
                                 EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_ERR,
                                 "SNTP status");
        return FALSE;
    }

    SNTP_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = SNTP_OM_GetStatus(status);

    SNTP_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_MGR_GetRunningStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the status of disable/enable mapping of system
 * INPUT:    None
 * OUTPUT:   status -- VAL_sntpStatus_enabled/VAL_sntpStatus_disabled
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:     default value is disable  1:enable, 2:disable
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SNTP_MGR_GetRunningStatus(UI32_T *status)
{
    if (FALSE == SNTP_MGR_GetStatus(status))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (*status == SNTP_DEFAULT_STATUS) /* if status is default status */
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_SetServiceOperationMode
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
BOOL_T SNTP_MGR_SetServiceOperationMode(UI32_T mode)
{
    BOOL_T  ret = TRUE;

    switch (mode)
    {
        case VAL_sntpServiceMode_unicast:
            break;

        case VAL_sntpServiceMode_broadcast:
        case VAL_sntpServiceMode_anycast:           /* Not support now */
        default:
            ret = FALSE;
            EH_MGR_Handle_Exception1(SYS_MODULE_SNTP, SNTP_MGR_SERVICE_MODE_FUNC_NO,
                                     EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_ERR,
                                     "Server mode (1-2)");
            break;
    }

    SNTP_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (TRUE == ret)
    {
        ret = SNTP_OM_SetServiceOperationMode(mode);
    }

    SNTP_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_GetServiceOperationMode
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
BOOL_T SNTP_MGR_GetServiceOperationMode(UI32_T *mode)
{
    BOOL_T  ret;

    if (NULL == mode)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_SNTP, SNTP_MGR_SERVICE_MODE_FUNC_NO,
                                 EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_ERR,
                                 "SNTP service mode");
        return FALSE;
    }

    SNTP_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = SNTP_OM_GetServiceOperationMode(mode);

    SNTP_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_MGR_GetRunningServiceMode
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
SYS_TYPE_Get_Running_Cfg_T SNTP_MGR_GetRunningServiceMode(UI32_T *servicemode)
{
    if (FALSE == SNTP_MGR_GetServiceOperationMode(servicemode))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (*servicemode == SNTP_DEFAULT_OPERATIONMODE) /* if service mode is default mode*/
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_SetPollTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Set the polling interval
 * INPUT    : polling interval used in unicast mode
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_MGR_SetPollTime(UI32_T polltime)
{
    BOOL_T  ret = FALSE;

    SNTP_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /*
     * If polltime fall in leagal inteval, then write to database else return
     * error message
     */
#if (SYS_CPNT_A3COM515_SNTP_MIB == TRUE)
    if (polltime <= MAX_sntpPollInterval3com && polltime >= MIN_sntpPollInterval3com)
#else
    if (polltime <= MAX_sntpPollInterval && polltime >= MIN_sntpPollInterval)
#endif
        {
            ret = SNTP_OM_SetPollTime(polltime);
        }
        else
        {
            {
                char    arg_buffer[32];
#if (SYS_CPNT_A3COM515_SNTP_MIB == TRUE)
            sprintf(arg_buffer,"Poll time (%lu-%lu)",(unsigned long)MIN_sntpPollInterval3com,(unsigned long)MAX_sntpPollInterval3com);
#else
            sprintf(arg_buffer,"Poll time (%lu-%lu)",(unsigned long)MIN_sntpPollInterval,(unsigned long)MAX_sntpPollInterval);
#endif
                EH_MGR_Handle_Exception1(SYS_MODULE_SNTP,
                                         SNTP_MGR_POLL_TIME_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                         SYSLOG_LEVEL_ERR, arg_buffer);
            }
        }

    SNTP_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_GetPollTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get the polling interval
 * INPUT    : A buffer is used to be put data
 * OUTPUT   : polling interval used in unicast mode
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_MGR_GetPollTime(UI32_T *polltime)
{
    BOOL_T  ret;

    if (NULL == polltime)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_SNTP, SNTP_MGR_POLL_TIME_FUNC_NO,
                                 EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_ERR,
                                 "SNTP poll time");
        return FALSE;
    }

    SNTP_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = SNTP_OM_GetPollTime(polltime);

    SNTP_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_MGR_GetRunningPollTime
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the status of disable/enable mapping of system
 * INPUT:    None
 * OUTPUT:   polling time
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:    default value is 16 secconds
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SNTP_MGR_GetRunningPollTime(UI32_T *polltime)
{
    if (FALSE == SNTP_OM_GetPollTime(polltime))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (*polltime == SNTP_DEFAULT_POLLINGTIME) /* if polling time is default value*/
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}

 /*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_AddServerIp
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
BOOL_T SNTP_MGR_AddServerIp(UI32_T index, L_INET_AddrIp_T *ipaddress)
{
    BOOL_T  ret;

    /*
     * only allow ip address of calss A, B, and C IP address >= 224.x.x.x is
     * invalid ,x.x.x.0 is also invali
     */
    if (SNTP_MGR_CheckServerIp(ipaddress) != TRUE)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_SNTP, SNTP_MGR_INVALID_IP_FUNC_NO,
                                EH_TYPE_MSG_INVALID_IP, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    /* Protection for the range of index */
    if (index < MIN_sntpServerIndex || index > MAX_sntpServerIndex)
    {
        return FALSE;
    }

    if (ipaddress->addrlen == 0)
    {
        return TRUE;
    }

    SNTP_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = SNTP_OM_AddServerIp(index, ipaddress);

    SNTP_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_DeleteServerIp
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
BOOL_T SNTP_MGR_DeleteServerIp(UI32_T index)
{
    BOOL_T  ret;

    if (index < MIN_sntpServerIndex || index > MAX_sntpServerIndex)
    {
        return FALSE;
    }

    SNTP_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = SNTP_OM_DeleteServerIp(index);

    SNTP_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_GetServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Get a server entry  from OM using ip address as index
 * INPUT    : 1.index (MIN_sntpServerIndex <= index <= MAX_sntpServerIndex)
 *            2.point to buffer that can contain information of a server
 * OUTPUT   : buffer contain information
 * RETURN   : TRUE : If success
 *            FALSE: If get failed.
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_MGR_GetServerIp(UI32_T index, L_INET_AddrIp_T *ipaddress)
{
    BOOL_T  ret;

    if (NULL == ipaddress)
    {
        return FALSE;
    }

    /* check index range */
    if (index < MIN_sntpServerIndex || index > MAX_sntpServerIndex)
    {
        return FALSE;
    }

    SNTP_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = SNTP_OM_GetServerIp(index, ipaddress);

    SNTP_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_GetNextServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Get next server entry using ip address as index
 * INPUT    : 1.index (start from 0)
 *            2.point to buffer that can contain server ip address
 * OUTPUT   : buffer contain information
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : if input index is 0, then it will find first entry in table.
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_MGR_GetNextServerIp(UI32_T *index, L_INET_AddrIp_T *ipaddress)
{
    /* NULL buffer */
    if ((NULL == ipaddress) || (NULL == index))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_SNTP, SNTP_MGR_SERVER_IP_FUNC_NO,
                                 EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_ERR,
                                 "next time server IP");
        return FALSE;
    }
    (*index)++;

    return SNTP_MGR_GetServerIp(*index, ipaddress);
}

 /*------------------------------------------------------------------------------
  * FUNCTION NAME - SNTP_MGR_GetLastUpdateUTCTime
  *------------------------------------------------------------------------------
  * PURPOSE  : Get time information of last update-time
  * INPUT    : buffer pointer stored time information
  * OUTPUT   : time in seconds from 2001/01/01 00:00:00
  * RETURN   : TRUE : If success
  *            FALSE: SNTP never get time from server.
  * NOTES    :
  *------------------------------------------------------------------------------*/
BOOL_T SNTP_MGR_GetLastUpdateUTCTime(UI32_T *time)
{
    BOOL_T  ret;

    if (time == NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_SNTP, SNTP_MGR_TIME_FUNC_NO,
                                 EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_ERR,
                                 "last update time");
        return FALSE;
    }

    SNTP_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = SNTP_OM_GetLastUpdateUTCTime(time);

    SNTP_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_GetCurrentUTCTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get time information of GMT
 * INPUT    : Buffer of  UTC time
 * OUTPUT   : 1.time in seconds from 2001/01/01 00:00:00
 * RETURN   : TRUE : If success
 *            FALSE: If failed
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_MGR_GetCurrentUTCTime(UI32_T *time)
{
    BOOL_T  ret;

    if (time == NULL)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_SNTP, SNTP_MGR_TIME_FUNC_NO,
                                 EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_ERR,
                                 "current time");
        return FALSE;
    }

    SNTP_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = SNTP_OM_GetCurrentUTCTime(time);

    SNTP_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_GetCurrentServer
 *------------------------------------------------------------------------------
 * PURPOSE  : Get current server of getting current time
 * INPUT    : buffer to get server
 * OUTPUT   : ip address
 * RETURN   : TRUE : If success
 *            FALSE: If failed
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_MGR_GetCurrentServer(L_INET_AddrIp_T *ipaddress)
{
    BOOL_T  ret;

    if (ipaddress == NULL)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_SNTP, SNTP_MGR_SERVER_FUNC_NO,
                                EH_TYPE_MSG_NO_TIME_SRV, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    SNTP_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = SNTP_OM_GetCurrentServer(ipaddress);

    SNTP_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_InTimeServiceMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Perform time serice mode,e.g, unicast, broadcast mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : Called by SNTP_TASK
 *------------------------------------------------------------------------------*/
void SNTP_MGR_InTimeServiceMode(void)
{
    UI32_T  service_mode;

    UI32_T  service;
    UI32_T  poll_time;
    UI32_T  operation_time;
    UI32_T  poll_time_in_ticks;
    L_INET_AddrIp_T ipaddress;
    UI32_T  index;
    UI32_T  exponential_backoff_timer_in_ticks;
    BOOL_T  ret = TRUE;

    SNTP_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    if ((SNTP_OM_GetStatus(&service) != TRUE) || (service != ENABLE))
    {
        SNTP_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }

    /*
     * If there is no sntp server ip, current ip must is zero. Don't
     * continue.
     */
    if (SNTP_OM_GetServerIpCount() == 0)
    {
        memset(&ipaddress, 0, sizeof(ipaddress));
        SNTP_OM_SetCurrentServer(&ipaddress);
        SNTP_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }

    if ((SNTP_OM_GetServiceOperationMode(&service_mode) != TRUE)
    ||  (SNTP_OM_GetPollTime(&poll_time) != TRUE))
    {
        SNTP_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }

    poll_time_in_ticks = poll_time * hz;
    exponential_backoff_timer_in_ticks = exponential_backoff_timer * hz;

    switch (service_mode)
    {
        case VAL_sntpServiceMode_unicast:
            /*
             * To measue how much time is passed ;
             * If polling time is not zero, then do nothing
             */
        if (Fire == SNTP_MGR_FIRE_IMMEDIATLY) /* first comming, so don't need to wait polling timer */
            {
                ;
            }
            else if (((Fire = SYSFUN_GetSysTick() - local_timer) < exponential_backoff_timer_in_ticks)
                 &&  ((Fire = SYSFUN_GetSysTick() - local_timer) < poll_time_in_ticks))
            {
                break;
            }

            /* Start to get time, and measure how much time is spent */
            operation_time = SYSFUN_GetSysTick();
            for (index = 1; index <= MAX_sntpServerIndex; index++)
            {
                memset(&ipaddress, 0, sizeof(L_INET_AddrIp_T));
                SNTP_MGR_GetServerIp(index, &ipaddress);
                
                if (0 != ipaddress.addrlen) /*ipaddress = 0 means empty entry. if found a ip then do it */
                {
                    ret = SNTP_MGR_PerformOperationMode(&ipaddress);
                }
            else /* if first server shown in CLI is zero, do broadcast mode*/
                {
                    if (1 == index)
                    {
                        memset(&ipaddress, 0, sizeof(L_INET_AddrIp_T));
                        ret = SNTP_MGR_PerformOperationMode(&ipaddress);
                    }
                    break;
                }

                /* If operation is success, then go out, otherwise continue to do */
                if (ret == TRUE)
                {
                    break;
                }
            }

        if (TRUE == ret) /* get the current timer, so expoential timer set to polling timer */
            {
                is_exponential_backoff_timer_start = FALSE;
                exponential_backoff_timer = poll_time;
            }
        else /* so exponential_backoff_timer *= 2, and its max value <= polling timer */
            {
                /*
                 * if there is a SNTP server available before, the
                 * exponential_backoff_timer will equal poll_time (see above
                 * source code "if (TRUE == msg)"). as a result, re-init
                 * exponential_backoff_timer like SNTP_MGR_SetStatus() did.
                 */
                if (FALSE == is_exponential_backoff_timer_start)
                {
                    is_exponential_backoff_timer_start = TRUE;
                    exponential_backoff_timer = 1;
                }

                exponential_backoff_timer = exponential_backoff_timer * 2;
                if (exponential_backoff_timer > poll_time)
                {
                    exponential_backoff_timer = poll_time;
                }
            }

            operation_time = SYSFUN_GetSysTick() - operation_time;

            /* Reset the timer and substract the operation time */
            local_timer = SYSFUN_GetSysTick();
            Fire = 0;
            break;

        case VAL_sntpServiceMode_broadcast:
            memset(&ipaddress, 0, sizeof(L_INET_AddrIp_T));
            ret = SNTP_MGR_PerformOperationMode(&ipaddress);

            /* Reset the timer */
            local_timer = SYSFUN_GetSysTick();
            Fire = 0;
            break;

        case VAL_sntpServiceMode_anycast:
            /* Reset the timer */
            local_timer = SYSFUN_GetSysTick();
            Fire = 0;
            break;

        default:
            break;
    }

    SNTP_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_AddServerIpForCLI
 *------------------------------------------------------------------------------
 * PURPOSE  : Add a sntp server ip to OM
 * INPUT    : ipaddress
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_MGR_AddServerIpForCLI(L_INET_AddrIp_T *ipaddress)
{
    UI32_T  i;
    L_INET_AddrIp_T ip_tmp;

    if (SNTP_MGR_CheckServerIp(ipaddress) != TRUE)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_SNTP, SNTP_MGR_INVALID_IP_FUNC_NO,
                                EH_TYPE_MSG_INVALID_IP, SYSLOG_LEVEL_INFO);
        return FALSE;
    }

    SNTP_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    for (i = 1; i <= MAX_sntpServerIndex; i++)
    {
        memset(&ip_tmp, 0, sizeof(ip_tmp));
        SNTP_MGR_GetServerIp(i, &ip_tmp);
        if (0 == ip_tmp.addrlen)
        {
            if (SNTP_MGR_AddServerIp(i, ipaddress) != TRUE)
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_SNTP,
                                         SNTP_MGR_SERVER_IP_FUNC_NO, EH_TYPE_MSG_FAILED_TO_ADD,
                                         SYSLOG_LEVEL_ERR, "SNTP server IP");
                break;
            }
            else
            {
                SNTP_MGR_RETURN_AND_RELEASE_CSC(TRUE);
            }
        }
    }

    SNTP_MGR_RETURN_AND_RELEASE_CSC(FALSE);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_DeleteServerIpForCLI
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete a server ip from OM
 * INPUT    : ipaddress
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : Delete one server ip will cause the SNTP server address entry be relocated,
 *            such as: server entry  : 10.1.1.1, 10.1.1.2, 10.1.1.3 remove 10.1.1.2
 *                                 --> 10.1.1.1, 10.1.1.3,
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_MGR_DeleteServerIpForCLI(L_INET_AddrIp_T *ipaddress)
{
    UI32_T  i;

    L_INET_AddrIp_T ip_tmp;

    SNTP_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    for (i = 1; i <= MAX_sntpServerIndex; i++)
    {
        SNTP_MGR_GetServerIp(i, &ip_tmp);
        
        if ((ip_tmp.type == ipaddress->type)&&
            (memcmp(ip_tmp.addr, ipaddress->addr, ipaddress->addrlen)==0))
        {
            if (SNTP_MGR_DeleteServerIp(i) != TRUE)
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_SNTP,
                                         SNTP_MGR_SERVER_IP_FUNC_NO, EH_TYPE_MSG_FAILED_TO_DELETE,
                                         SYSLOG_LEVEL_ERR, "SNTP server IP");
                SNTP_MGR_RETURN_AND_RELEASE_CSC(FALSE);
            }
            else
            {
                Fire = SNTP_MGR_FIRE_IMMEDIATLY;
                SNTP_MGR_RETURN_AND_RELEASE_CSC(TRUE);
            }
        }
    }

    EH_MGR_Handle_Exception1(SYS_MODULE_SNTP, SNTP_MGR_SERVER_IP_FUNC_NO,
                             EH_TYPE_MSG_FAILED_TO_DELETE, SYSLOG_LEVEL_ERR,
                             "SNTP server IP");
    SNTP_MGR_RETURN_AND_RELEASE_CSC(FALSE);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_DeleteAllServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete all servers ip from OM
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_MGR_DeleteAllServerIp(void)
{
    UI32_T  i;

    SNTP_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    for (i = MAX_sntpServerIndex; i >= 1; i--)
    {
        if (SNTP_MGR_DeleteServerIp(i) != TRUE)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_SNTP,
                                     SNTP_MGR_SERVER_IP_FUNC_NO, EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_ERR, "SNTP server IP");
            SNTP_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }
    }

    SNTP_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

/* FUNCTION NAME - SNTP_MGR_HandleHotInsertion
 *
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is inserted at a time.
 */
void SNTP_MGR_HandleHotInsertion(UI32_T  starting_port_ifindex,
                                 UI32_T  number_of_port,
                                 BOOL_T  use_default)
{
    return;
}

/* FUNCTION NAME - SNTP_MGR_HandleHotRemoval
 *
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is removed at a time.
 */
void SNTP_MGR_HandleHotRemoval(UI32_T  starting_port_ifindex,
                               UI32_T  number_of_port)
{
    return;
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
 *                                 --> 10.1.1.1, 10.1.1.3, 0.0.0.0
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_MGR_DeleteServerIpForSNMP(UI32_T index)
{
    if (SNTP_MGR_DeleteServerIp(index) != TRUE)
    {
        return FALSE;
    }

    return TRUE;
}


/* LOCAL SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * FUNCTION NAME -  SNTP_MGR_PerformOperationMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Perform unicast or broadcast mode
 * INPUT    : ipaddress:if is zero then broadcast, or unicast
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : Called by SNTP_MGR_InTimeService
 *------------------------------------------------------------------------------*/
static BOOL_T SNTP_MGR_PerformOperationMode(L_INET_AddrIp_T *ipaddress)
{
    UI32_T                  totaltime;

    UI32_T                  tick;                   /* total seconds from
                                                     * 1900/01/01 00:00 */
    SNTP_TXRX_STATUS_E      Operation_status;       /* operation status of
                                                     * unicast mode */
    SNTP_UPDATE_STATUS_T    status;
    BOOL_T                  ret = FALSE;

    Operation_status = SNTP_TXRX_Client(ipaddress, SNTP_WAIT_TIMEOUT,
                                        &totaltime, &tick);

    if (Operation_status == SNTP_TXRX_MSG_SUCCESS)
    {
        memcpy(&status.Current_server, ipaddress, sizeof(L_INET_AddrIp_T));
        status.Current_time = totaltime;
        status.Server_Status = ACTIVE;

        /* It must return SNTP_MSG_SUCCESS,else assert */
        if (SNTP_MGR_UpdateSntpStatus(&status) != TRUE)
        {
            if (DBG_SNTP_TURN_MESSAGE_ON_OFF)
            {
                printf(" *** %s:%d failed.\n", __FUNCTION__, __LINE__);
            }
        }

        /*
         * Update time to SYS_TIME ;
         * debug use
         */
        if (DBG_SNTP_TURN_MESSAGE_ON_OFF)
        {
            printf("%s:Exponential backoff Timer %lu\n", __FUNCTION__, (unsigned long)exponential_backoff_timer);
            printf("%d:Get time success from %lx\n", __LINE__, (unsigned long)(*(UI32_T *)ipaddress->addr));
        }

        SYS_TIME_SetRealTimeClockBySeconds(totaltime - SNTP_1900_TO_1970_SECS, tick);
        ret = TRUE;
    }
    else if (Operation_status == SNTP_TXRX_MSG_TIMEOUT)
    {
        /*Syslog;*/
        memcpy(&status.Current_server, ipaddress, sizeof(L_INET_AddrIp_T));
        status.Current_time = 0;
        status.Server_Status = NO_RESPONSE;

        /* It must return SNTP_MSG_SUCCESS,else assert */
        if (SNTP_MGR_UpdateSntpStatus(&status) != TRUE)
        {
            if (DBG_SNTP_TURN_MESSAGE_ON_OFF)
            {
                printf(" *** %s:%d failed.\n", __FUNCTION__, __LINE__);
            }
        }

        /* debug use */
        if (DBG_SNTP_TURN_MESSAGE_ON_OFF)
        {
            printf("%s:Exponential backoff Timer %lu\n", __FUNCTION__, (unsigned long)exponential_backoff_timer);
            printf("%d:Get time error from %lx\n", __LINE__, (unsigned long)(*(UI32_T *)ipaddress->addr));
            printf("%d:The error message is %d\n", __LINE__, Operation_status);
        }
    }
    else  /* SNTP_TXRX_MSG_INVALID_PARAMETER,  SNTP_TXRX_MSG_FAIL*/
    {
        /* debug use */
        if (DBG_SNTP_TURN_MESSAGE_ON_OFF)
        {
            printf("%s:Exponential backoff Timer %lu\n", __FUNCTION__, (unsigned long)exponential_backoff_timer);
            printf("%d:Get time error from %lx\n", __LINE__, (unsigned long)(*(UI32_T *)ipaddress->addr));
            printf("%d:The error message is %d\n", __LINE__, Operation_status);
        }

        /* End of debug use */
    }

    return ret;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  SNTP_MGR_UpdateSntpStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Update Sntp current Status
 * INPUT    : a pointer pointed to a constant STATUS
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE: If failed
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static BOOL_T SNTP_MGR_UpdateSntpStatus(const SNTP_UPDATE_STATUS_T *STATUS)
{
    UI32_T              ipindex = 0;
    SNTP_OM_VAR_STATE_T temp_state;

    /* If there is no buffer to be accessed ,then return message */
    if (NULL == STATUS)
    {
        return FALSE;
    }

    SNTP_OM_GetVarStateTable(&temp_state);

    /* If there is an active sntp server serving, then update the database */
    if (STATUS->Current_server.addrlen != 0)        /* It has one time server */
    {
        /* Set current server and current time */
        /*Move currnt time and server to old state */
        if (temp_state.current_time != 0) /* If current is not zero, then update the last updat time*/
        {
            temp_state.last_update_time = temp_state.current_time;
        }

        temp_state.last_update_server = temp_state.current_server;
        memcpy(&temp_state.last_update_server, &temp_state.current_server, sizeof(L_INET_AddrIp_T));

        /* Copy new time and server to current state */
        temp_state.current_time = STATUS->Current_time;
        memcpy(&temp_state.current_server, &STATUS->Current_server, sizeof(L_INET_AddrIp_T));

        if (SNTP_OM_SearchIpEntry(&temp_state.current_server, &ipindex) == TRUE)
        {
            SNTP_OM_SetServerStatusByIndex(ipindex, STATUS->Server_Status);
        }
        else
        {
            temp_state.current_server_status = STATUS->Server_Status;
        }
    }

    /* otherwise set current time and server to zero,and server status to WAS_CLEARED */
    else
    {
        temp_state.current_time = 0;
        memset(&temp_state.current_server, 0, sizeof(temp_state.current_server));
        temp_state.current_server_status = STATUS->Server_Status;
    }

    SNTP_OM_SetVarStateTable(&temp_state);

    return TRUE;
}

 /*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_MGR_CheckServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : check input address is in suitable range.
 * INPUT    : ipaddress
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static BOOL_T 
SNTP_MGR_CheckServerIp(
    L_INET_AddrIp_T *ipaddress)
{
    switch(ipaddress->type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
        {
            if (IP_LIB_OK != IP_LIB_IsValidForRemoteIp(ipaddress->addr))
            {
                return FALSE;
            }
        }
            break;

        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
        {
            /* IPv6 address should not be
             * IP_LIB_INVALID_IPV6_UNSPECIFIED
             * IP_LIB_INVALID_IPV6_LOOPBACK
             * IP_LIB_INVALID_IPV6_MULTICAST
             */
            if (IP_LIB_OK != IP_LIB_CheckIPv6PrefixForInterface(ipaddress->addr, SYS_ADPT_IPV6_ADDR_LEN))
            {
                return FALSE;
            }
        }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

