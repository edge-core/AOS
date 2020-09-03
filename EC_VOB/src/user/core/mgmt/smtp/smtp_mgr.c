/* Module Name: SMTP_MGR.C
 * Purpose: Initialize the resources and provide functions
 *          for the smtp module.
 *
 * Notes:
 *
 * History:
 *    01/22/03       -- Ricky Lin, Create
 *
 * Copyright(C)      Accton Corporation, 1999 - 2001
 *
 */

 /* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <unistd.h>     /* for close */
#include <errno.h>

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "sys_bld.h"
#include "sys_time.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "l_mm.h"
#include "syslog_type.h"
#include "syslog_adpt.h"
#include "smtp_mgr.h"
#include "smtp_task.h"
#include "smtp_util.h"

#include "l_stdlib.h"
#include "mib2_pom.h"
#include "netcfg_type.h"

#include "snmp_pmgr.h"
#include "sys_module.h"

#if (SYS_CPNT_EH == TRUE)
#include "eh_type.h"
#include "eh_mgr.h"
#endif

#include "backdoor_mgr.h"
#include "iml_pmgr.h"
#include "l_mname.h"
#include "ip_lib.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define SMTP_ADPT_IPPROTO_TCP         6
#define SMTP_ADPT_SMTP_PORT            25
#define SMTP_ADPT_CONTENT_SIZE        600
#define SMTP_ADPT_BUF_SIZE            256

/* SMTP server reply code */
#define    SMTP_MGR_SERVICE_READY        220
#define    SMTP_MGR_SERVICE_CLOSING    221
#define    SMTP_MGR_MAIL_ACTION_OK        250
#define    SMTP_MGR_START_MAIL_INPUT    354
#define    SMTP_MGR_RCPT_NOT_LOCAL        251

#define SMTP_MGR_MAX_LENGTH_OF_MSG_PATTERN    SMTP_ADPT_CONTENT_SIZE

/* DATA TYPE DECLARATIONS
 */
enum SMTP_MGR_STATE_E
{
    SMTP_MGR_STATE_INIT = 1,
    SMTP_MGR_STATE_HELLO,
    SMTP_MGR_STATE_MAILFROM,
    SMTP_MGR_STATE_RCPTTO,
    SMTP_MGR_STATE_DATA,
    SMTP_MGR_STATE_CONTENT,
    SMTP_MGR_STATE_FINISH,
    SMTP_MGR_STATE_QUIT,
};

/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    SMTP_TYPE_TRACE_ID_SMTP_MGR_SENDMAIL=0
};

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static UI32_T SMTP_MGR_Send(UI8_T *patten);
static UI32_T SMTP_MGR_Valid_IP(UI32_T ip_addr);
static UI32_T SMTP_MGR_ValidateEmailAddress(const char *address);
static UI32_T SMTP_MGR_Connect(UI32_T server_ip,I16_T *socket_id);
static void SMTP_MGR_Close(I16_T socket_id);
static void SMTP_MGR_LogToMsg(SMTP_OM_QueueRecord_T *smtp_data,UI8_T *patten);
static void SMTP_MGR_BackdoorInfo_CallBack(void);

//static void SMTP_MGR_Print_BackdoorHelp(void);
static BOOL_T SMTP_MGR_IsLocalIP(UI32_T ip);

/* STATIC VARIABLE DECLARATIONS
 */
SYSFUN_DECLARE_CSC


extern UI32_T NETCFG_POM_IP_GetNextRifConfig(NETCFG_TYPE_InetRifConfig_T *rif_config_p);    // daniel
extern UI32_T NETCFG_POM_IP_GetNextRifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p);    //daniel


extern int                      errno; /* for socket error code */

static I16_T                    smtp_mgr_socket_id;
static UI32_T                   smtp_mgr_server_ip;/* smtp server ip address which connect successfully */
static BOOL_T                   debug_mode;
static UI32_T                   send_hello_flag;

const static char *MONTH_MAPPING_STR[] = {
    "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};

/* MACRO FUNCTIONS DECLARACTION
 */
#define SMTP_MGR_USE_CSC_CHECK_OPER_MODE(RET_VAL) \
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE) { \
       return (RET_VAL); \
    }

#define SMTP_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE() \
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE) { \
        return; \
    }

#define SMTP_MGR_RETURN_AND_RELEASE_CSC(RET_VAL) { \
        return (RET_VAL); \
    }

#define SMTP_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE() { \
        return; \
    }


/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SMTP_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for smtp mgr.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SMTP_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    UI32_T  cmd;

    if(ipcmsg_p == NULL)
        return FALSE;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        SMTP_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;
        ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        return TRUE;
    }

    switch((cmd = SMTP_MGR_MSG_CMD(ipcmsg_p)))
    {
        case SMTP_MGR_IPC_CMD_ADD_SMTP_DST_EMAIL_ADDR:
        {
            SMTP_MGR_IPCMsg_EmailAddr_T *data_p = SMTP_MGR_MSG_DATA(ipcmsg_p);
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_AddSmtpDestinationEmailAddr(data_p->email_addr);
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SMTP_MGR_IPC_CMD_ADD_SMTP_SERVER_IP:
        {
            SMTP_MGR_IPCMsg_IpAddr_T    *data_p = SMTP_MGR_MSG_DATA(ipcmsg_p);
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_AddSmtpServerIPAddr(data_p->ip_addr);
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SMTP_MGR_IPC_CMD_DEL_SMTP_DST_EMAIL_ADDR:
        {
            SMTP_MGR_IPCMsg_EmailAddr_T *data_p = SMTP_MGR_MSG_DATA(ipcmsg_p);
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_DeleteSmtpDestinationEmailAddr(data_p->email_addr);
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SMTP_MGR_IPC_CMD_DEL_SMTP_SERVER_IP:
        {
            SMTP_MGR_IPCMsg_IpAddr_T    *data_p = SMTP_MGR_MSG_DATA(ipcmsg_p);
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_DeleteSmtpServerIPAddr(data_p->ip_addr);
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SMTP_MGR_IPC_CMD_DISABLE_ADMIN_STATUS:
        {
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_DisableSmtpAdminStatus();
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SMTP_MGR_IPC_CMD_ENABLE_ADMIN_STATUS:
        {
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_EnableSmtpAdminStatus();
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SMTP_MGR_IPC_CMD_GET_EMAIL_SER_LEVEL:
        {
            SMTP_MGR_IPCMsg_SerLevel_T  *data_p = SMTP_MGR_MSG_DATA(ipcmsg_p);
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_GetEmailSeverityLevel(&data_p->ser_level);
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_SerLevel_T);
            break;
        }

        case SMTP_MGR_IPC_CMD_GET_NEXT_RUNN_SMTP_DST_EML_ADR:
        {
            SMTP_MGR_IPCMsg_EmailAddr_T *data_p = SMTP_MGR_MSG_DATA(ipcmsg_p);
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_GetNextRunningSmtpDestinationEmailAddr(data_p->email_addr);
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T);
            break;
        }

        case SMTP_MGR_IPC_CMD_GET_NEXT_RUNN_SMTP_SVR_IP:
        {
            SMTP_MGR_IPCMsg_IpAddr_T    *data_p = SMTP_MGR_MSG_DATA(ipcmsg_p);
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_GetNextRunningSmtpServerIPAddr(&data_p->ip_addr);
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_IpAddr_T);
            break;
        }

        case SMTP_MGR_IPC_CMD_GET_NEXT_SMTP_DES_EMAIL_ADDR:
        {
            SMTP_MGR_IPCMsg_EmailAddr_T *data_p = SMTP_MGR_MSG_DATA(ipcmsg_p);
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_GetNextSmtpDestinationEmailAddr(data_p->email_addr);
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T);
            break;
        }

        case SMTP_MGR_IPC_CMD_GET_NEXT_SMTP_SERVER_IP:
        {
            SMTP_MGR_IPCMsg_IpAddr_T    *data_p = SMTP_MGR_MSG_DATA(ipcmsg_p);
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_GetNextSmtpServerIPAddr(&data_p->ip_addr);
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_IpAddr_T);
            break;
        }

        case SMTP_MGR_IPC_CMD_GET_RUNN_EMAIL_SER_LVL:
        {
            SMTP_MGR_IPCMsg_SerLevel_T  *data_p = SMTP_MGR_MSG_DATA(ipcmsg_p);
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_GetRunningEmailSeverityLevel(&data_p->ser_level);
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_SerLevel_T);
            break;
        }

        case SMTP_MGR_IPC_CMD_GET_RUNN_SMTP_ADMIN_STATUS:
        {
            SMTP_MGR_IPCMsg_Status_T    *data_p = SMTP_MGR_MSG_DATA(ipcmsg_p);
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_GetRunningSmtpAdminStatus(&data_p->status);
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_Status_T);
            break;
        }

        case SMTP_MGR_IPC_CMD_GET_SMTP_ADMIN_STATUS:
        {
            SMTP_MGR_IPCMsg_Status_T    *data_p = SMTP_MGR_MSG_DATA(ipcmsg_p);
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_GetSmtpAdminStatus(&data_p->status);
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_Status_T);
            break;
        }

        case SMTP_MGR_IPC_CMD_GET_RUNN_SMTP_SRC_EML_ADR:
        {
            SMTP_MGR_IPCMsg_EmailAddr_T *data_p = SMTP_MGR_MSG_DATA(ipcmsg_p);
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_GetRunningSmtpSourceEmailAddr(data_p->email_addr);
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T);
            break;
        }

        case SMTP_MGR_IPC_CMD_GET_SMTP_SRC_EMAIL_ADDR:
        {
            SMTP_MGR_IPCMsg_EmailAddr_T *data_p = SMTP_MGR_MSG_DATA(ipcmsg_p);
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_GetSmtpSourceEmailAddr(data_p->email_addr);
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T);
            break;
        }

        case SMTP_MGR_IPC_CMD_SET_EMAIL_SER_LEVEL:
        {
            SMTP_MGR_IPCMsg_SerLevel_T *data_p = SMTP_MGR_MSG_DATA(ipcmsg_p);
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_SetEmailSeverityLevel(data_p->ser_level);
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        //ADD daniel
        case SMTP_MGR_IPC_CMD_DEL_EMAIL_SER_LEVEL:
        {
            SMTP_MGR_IPCMsg_SerLevel_T *data_p = SMTP_MGR_MSG_DATA(ipcmsg_p);
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_DeleteEmailSeverityLevel(data_p->ser_level);
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        //end ADD daniel

        case SMTP_MGR_IPC_CMD_SET_SMTP_SRC_EMAIL_ADDR:
        {
            SMTP_MGR_IPCMsg_EmailAddr_T *data_p = SMTP_MGR_MSG_DATA(ipcmsg_p);
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_SetSmtpSourceEmailAddr(data_p->email_addr);
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        //ADD daniel
        case SMTP_MGR_IPC_CMD_DEL_SMTP_SRC_EMAIL_ADDR:
        {
            SMTP_MGR_IPCMsg_EmailAddr_T *data_p = SMTP_MGR_MSG_DATA(ipcmsg_p);
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_DeleteSmtpSourceEmailAddr();
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        //end ADD daniel

        case SMTP_MGR_IPC_CMD_GET_SMTP_SERVER_IP_ADDR:
        {
            SMTP_MGR_IPCMsg_IpAddr_T    *data_p = SMTP_MGR_MSG_DATA(ipcmsg_p);
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_GetSmtpServerIPAddr(&data_p->ip_addr);
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_IpAddr_T);
            break;
        }

        case SMTP_MGR_IPC_CMD_GET_SMTP_DST_EMAIL_ADDR:
        {
            SMTP_MGR_IPCMsg_EmailAddr_T *data_p = SMTP_MGR_MSG_DATA(ipcmsg_p);
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = SMTP_MGR_GetSmtpDestinationEmailAddr(data_p->email_addr);
            ipcmsg_p->msg_size = SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T);
            break;
        }

        default:
        {
            SMTP_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;
            SYSFUN_Debug_Printf("*** %s(): Invalid cmd.\n", __FUNCTION__);
        }
    } /* switch ipcmsg_p->cmd */

    if (SMTP_MGR_MSG_RETVAL(ipcmsg_p) == FALSE)
    {
        SYSFUN_Debug_Printf("*** %s(): [cmd: %ld] failed\n", __FUNCTION__, cmd);
    }

    return TRUE;
}

/* FUNCTION NAME: SMTP_MGR_GetOperationMode()
 * PURPOSE: This function will get current operation mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  Current operation mode.
 *           1. SYS_TYPE_STACKING_TRANSITION_MODE
 *           2. SYS_TYPE_STACKING_MASTER_MODE
 *           3. SYS_TYPE_STACKING_SLAVE_MODE
 * NOTES:   None.
 *
 */
SYS_TYPE_Stacking_Mode_T SMTP_MGR_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
}/* End of SMTP_MGR_GetOperationMode() */

/* FUNCTION NAME: SMTP_MGR_EnterMasterMode()
 * PURPOSE: This function will enter master mode
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void SMTP_MGR_EnterMasterMode(void)
{
    SMTP_OM_Initiate_System_Resources();

    SYSFUN_ENTER_MASTER_MODE();
    return;
}/* End of SMTP_MGR_EnterMasterMode() */

/* FUNCTION NAME: SMTP_MGR_EntertSlaveMode()
 * PURPOSE: This function will enter slave mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void SMTP_MGR_EnterSlaveMode(void)
{

    SYSFUN_ENTER_SLAVE_MODE();
    return;
}/* End of SMTP_MGR_EnterSlaveMode() */

/* FUNCTION NAME: SMTP_MGR_EntertTransitionMode()
 * PURPOSE: This function will enter transition mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void SMTP_MGR_EnterTransitionMode(void)
{

    SYSFUN_ENTER_TRANSITION_MODE();
    return;
}/* End of SMTP_MGR_EnterTransitionMode() */

/* FUNCTION NAME: SMTP_MGR_SetTransitionMode()
 * PURPOSE: This function will set transition mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void SMTP_MGR_SetTransitionMode(void)
{

    SYSFUN_SET_TRANSITION_MODE();
    return;
}/* End of SMTP_MGR_SetTransitionMode() */

/* FUNCTION NAME: SMTP_MGR_Initiate_System_Resources
 * PURPOSE: This function is used to initialize the smtp module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   1. Create semphore.
 *          2. Initialize OM database of the smtp module.
 *          3. Called by SMTP_INIT_Initiate_System_Resources() only.
 *
 */
void SMTP_MGR_Initiate_System_Resources(void)
{
#if 0
    /* create semaphore
     */
    if (SYSFUN_CreateSem (1, SYSFUN_SEM_FIFO, &smtp_mgr_sem_id) != SYSFUN_OK)
    {
        perror("\r\nSMTP_MGR: Create semaphore id failed!");
        while(1);
    }

    if (SYSFUN_CreateSem (1, SYSFUN_SEM_FIFO, &smtp_mgr_queue_sem_id) != SYSFUN_OK)
    {
        perror("\r\nSMTP_MGR: Create queue semaphore id failed!");
        while(1);
    }
#endif

    SMTP_OM_CreatSem();
    SMTP_OM_Initiate_System_Resources();

    debug_mode=FALSE;

    return;
}/* End of SMTP_MGR_Initiate_System_Resources() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SMTP_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SMTP_MGR_Create_InterCSC_Relation(void)
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("smtp",
        SYS_BLD_UTILITY_GROUP_IPCMSGQ_KEY,SMTP_MGR_BackdoorInfo_CallBack);
} /* end of SMTP_MGR_Create_InterCSC_Relation */

/* FUNCTION NAME: SMTP_MGR_AddSmtpServerIPAddr
 * PURPOSE: This function is used to add the smtp server ip address.
 * INPUT:   ip_addr -- smtp server ip address.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID  -- invalid input value
 *          SMTP_RETURN_INPUT_EXIST -- input exist
 *          SMTP_RETURN_DATABASE_FULL -- database full
 * NOTES:   1. add one server ip address one time.
 *          2. Max number of smtp server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *
 */
UI32_T SMTP_MGR_AddSmtpServerIPAddr(UI32_T ip_addr)
{
    UI32_T  ret = SMTP_RETURN_SUCCESS;

    SMTP_MGR_USE_CSC_CHECK_OPER_MODE(SMTP_RETURN_SUCCESS)

    if(SMTP_MGR_Valid_IP(ip_addr) != SMTP_RETURN_SUCCESS)
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_SMTP, SMTP_MGR_ADD_SERVER_IP_FUNC_NO, EH_TYPE_MSG_INVALID_IP, SYSLOG_LEVEL_INFO);
#endif
        SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_INVALID);
    }

    ret = SMTP_OM_SetSmtpServerIPAddr(ip_addr);

    SMTP_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of SMTP_MGR_AddSmtpServerIPAddr() */

/* FUNCTION NAME: SMTP_MGR_GetSmtpServerIPAddr
 * PURPOSE: This function is used to get smtp server ip address.
 * INPUT:   *ip_addr -- output buffer of smtp server ip address.
 * OUTPUT:  *ip_addr -- smtp server ip address.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID_BUFFER   -- invalid input buffer
 * NOTES:   1. Max number of server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *          2. get one server ip address one time
 *          3. use 0 to get first ip address
 *
 */
UI32_T SMTP_MGR_GetSmtpServerIPAddr(UI32_T *ip_addr)
{
    UI32_T temp_addr, best_addr = 0xffffffff;

    SMTP_MGR_USE_CSC_CHECK_OPER_MODE(SMTP_RETURN_FAIL);

    if(ip_addr == NULL)
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_SMTP, SMTP_MGR_GET_SERVER_IP_FUNC_NO,
            EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif
        SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_INVALID_BUFFER);
    }

    temp_addr = 0;
    while(SMTP_OM_GetNextSmtpServerIPAddr(&temp_addr) == SMTP_RETURN_SUCCESS)
    {
        best_addr = temp_addr;
        if(best_addr == *ip_addr)
        {
            *ip_addr = best_addr;
            SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_SUCCESS);
        }
    }

      SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_FAIL);
}/* End of SMTP_MGR_GetSmtpServerIPAddr() */

/* FUNCTION NAME: SMTP_MGR_GetNextSmtpServerIPAddr
 * PURPOSE: This function is used to get next smtp server ip address.
 * INPUT:   *ip_addr -- output buffer of smtp server ip address.
 * OUTPUT:  *ip_addr -- smtp server ip address.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID  -- invalid input value
 *          SMTP_RETURN_INVALID_BUFFER   -- invalid input buffer
 * NOTES:   1. Max number of server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *          2. get one server ip address one time
 *          3. use 0 to get first ip address
 *
 */
UI32_T SMTP_MGR_GetNextSmtpServerIPAddr(UI32_T *ip_addr)
{
    UI32_T temp_addr, best_addr = 0xffffffff;

    SMTP_MGR_USE_CSC_CHECK_OPER_MODE(SMTP_RETURN_FAIL);

    if(ip_addr == NULL)
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_SMTP, SMTP_MGR_GET_NEXT_SERVER_IP_FUNC_NO,
            EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif
        SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_INVALID_BUFFER);
    }

    temp_addr = *ip_addr;
    if (SMTP_OM_GetNextSmtpServerIPAddr(&temp_addr) == SMTP_RETURN_SUCCESS)
    {
        best_addr = temp_addr;
    }

    if ((best_addr != 0xffffffff) && (best_addr != 0))
    {
        *ip_addr = best_addr;
        SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_SUCCESS);
    }

    SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_FAIL);
}/* End of SMTP_MGR_GetNextSmtpServerIPAddr() */

/* FUNCTION NAME: SMTP_MGR_GetNextRunningSmtpServerIPAddr
 * PURPOSE: This function is used to get next running smtp server ip address.
 * INPUT:   *ip_addr -- output buffer of smtp server ip address.
 * OUTPUT:  *ip_addr -- smtp server ip address.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. Max number of server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *          2. get one server ip address one time
 *          3. use 0 to get first running ip address
 *
 */
SYS_TYPE_Get_Running_Cfg_T SMTP_MGR_GetNextRunningSmtpServerIPAddr(UI32_T *ip_addr)
{
    UI32_T  ret;

    ret = SMTP_MGR_GetNextSmtpServerIPAddr(ip_addr);

    if (ret != SMTP_RETURN_SUCCESS)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (*ip_addr == SYS_DFLT_SMTP_SERVER_IP_ADDR)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/* End of SMTP_MGR_GetNextRunningSmtpServerIPAddr() */

/* FUNCTION NAME: SMTP_MGR_DeleteSmtpServerIPAddr
 * PURPOSE: This function is used to delete the smtp server ip address.
 * INPUT:   ip_addr -- smtp server ip address.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID  -- invalid input value
 *          SMTP_RETURN_INPUT_NOT_EXIST -- input not exist
 * NOTES:   1. Max number of smtp server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *          2. Delete one server ip address one time
 *
 */
UI32_T SMTP_MGR_DeleteSmtpServerIPAddr(UI32_T ip_addr)
{
    UI32_T  ret;

    SMTP_MGR_USE_CSC_CHECK_OPER_MODE(SMTP_RETURN_FAIL);

    if(SMTP_MGR_Valid_IP(ip_addr) != SYSLOG_REMOTE_SUCCESS)
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_SMTP, SMTP_MGR_DELETE_SERVER_IP_FUNC_NO, EH_TYPE_MSG_INVALID_IP, SYSLOG_LEVEL_INFO);
#endif
        SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_INVALID);
    }

    ret = SMTP_OM_DeleteSmtpServerIPAddr(ip_addr);

    SMTP_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of SMTP_MGR_DeleteSmtpServerIPAddr() */

/* FUNCTION NAME: SMTP_MGR_SetEmailSeverityLevel
 * PURPOSE: This function is used to set smtp severity level.
 * INPUT:   level -- smtp seveirity level.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID  -- invalid input value
 * NOTES:   1. smtp severity level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 *
 */
UI32_T SMTP_MGR_SetEmailSeverityLevel(UI32_T level)
{
    UI32_T  om_level, ret;

    SMTP_MGR_USE_CSC_CHECK_OPER_MODE(SMTP_RETURN_FAIL);

    if ((level < SYSLOG_LEVEL_EMERG) || (level > SYSLOG_LEVEL_DEBUG))
    {
#if (SYS_CPNT_EH == TRUE)
        UI8_T   buff1[32] = {0};

        sprintf(buff1, "Severity level (0-7):");
        EH_MGR_Handle_Exception1(SYS_MODULE_SMTP, SMTP_MGR_SET_SEVERITY_LEVEL_FUNC_NO,
                EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (SYSLOG_LEVEL_INFO),
                buff1);
#endif
        SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_INVALID);
    }

    SMTP_OM_GetEmailSeverityLevel(&om_level);
    if (level != om_level)
    {
        ret = SMTP_OM_SetEmailSeverityLevel(level);
        SMTP_MGR_RETURN_AND_RELEASE_CSC(ret);
    }

    SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_SUCCESS);
}/* End of SMTP_MGR_SetEmailSeverityLevel() */

//ADD daniel
/* FUNCTION NAME: SMTP_MGR_DeleteEmailSeverityLevel
 * PURPOSE: This function is used to delete smtp severity level to default value.
 * INPUT:   level -- smtp seveirity level.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID  -- invalid input value
 * NOTES:   1. smtp severity level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 *
 */
UI32_T SMTP_MGR_DeleteEmailSeverityLevel(UI32_T level)
{
    UI32_T  om_level, ret;

    SMTP_MGR_USE_CSC_CHECK_OPER_MODE(SMTP_RETURN_FAIL);

    if ((level < SYSLOG_LEVEL_EMERG) || (level > SYSLOG_LEVEL_DEBUG))
    {
#if (SYS_CPNT_EH == TRUE)
        UI8_T   buff1[32] = {0};

        sprintf(buff1, "Severity level (0-7):");
        EH_MGR_Handle_Exception1(SYS_MODULE_SMTP, SMTP_MGR_SET_SEVERITY_LEVEL_FUNC_NO,
                EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (SYSLOG_LEVEL_INFO),
                buff1);
#endif
        SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_INVALID);
    }

    SMTP_OM_GetEmailSeverityLevel(&om_level);
    if (level == om_level)
    {
        ret = SMTP_OM_DeleteEmailSeverityLevel(level);
        SMTP_MGR_RETURN_AND_RELEASE_CSC(ret);
    }

    SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_INVALID);
}/* End of SMTP_MGR_DeleteEmailSeverityLevel() */
//end ADD daniel

/* FUNCTION NAME: SMTP_MGR_GetEmailSeverityLevel
 * PURPOSE: This function is used to get smtp severity level.
 * INPUT:   *level -- output buffer of smtp severity level.
 * OUTPUT:  *level -- smtp severity level.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID_BUFFER   -- invalid input buffer
 * NOTES:   1. smtp severity level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 *
 */
UI32_T SMTP_MGR_GetEmailSeverityLevel(UI32_T *level)
{
    UI32_T  ret;

    SMTP_MGR_USE_CSC_CHECK_OPER_MODE(SMTP_RETURN_FAIL);

    if (level == NULL)
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_SMTP, SMTP_MGR_GET_SEVERITY_LEVEL_FUNC_NO,
            EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif
        SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_INVALID_BUFFER);
    }

    ret = SMTP_OM_GetEmailSeverityLevel(level);

    SMTP_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of SMTP_MGR_GetEmailSeverityLevel() */

/* FUNCTION NAME: SMTP_MGR_GetRunningEmailSeverityLevel
 * PURPOSE: This function is used to get running smtp severity level.
 * INPUT:   *level -- output buffer of smtp severity level.
 * OUTPUT:  *level -- smtp severity level.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. smtp severity level symbol is defined as following:
 *          --  SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *          --  SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *          --  SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *          --  SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *          --  SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *          --  SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *          --  SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *          --  SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *          2. SYSLOG_LEVEL_EMERG is highest priority.
 *             SYSLOG_LEVEL_DEBUG is lowest priority.
 *
 */
SYS_TYPE_Get_Running_Cfg_T SMTP_MGR_GetRunningEmailSeverityLevel(UI32_T *level)
{
    UI32_T  ret;

    SMTP_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (level == NULL)
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_SMTP, SMTP_MGR_GET_RUNNING_SEVERITY_LEVEL_FUNC_NO,
            EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif
        SMTP_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    ret = SMTP_OM_GetEmailSeverityLevel(level);

    if (ret != SMTP_RETURN_SUCCESS)
        SMTP_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (*level == SYS_DFLT_SMTP_LEVEL)
        SMTP_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);

    SMTP_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
}/* End of SMTP_MGR_GetRunningEmailSeverityLevel() */

/* FUNCTION NAME: SMTP_MGR_SetSmtpSourceEmailAddr
 * PURPOSE: This function is used to set the smtp source email address.
 * INPUT:   *email_addr -- smtp source email address.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID  -- invalid input value
 * NOTES:   1. There is only one source email address
 *          2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
UI32_T SMTP_MGR_SetSmtpSourceEmailAddr(UI8_T *email_addr)
{
    UI8_T  om_addr[SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1] = {0};
    UI32_T  ret;

    SMTP_MGR_USE_CSC_CHECK_OPER_MODE(SMTP_RETURN_FAIL);

    if (SMTP_RETURN_SUCCESS != SMTP_MGR_ValidateEmailAddress((const char *)email_addr))
    {
#if (SYS_CPNT_EH == TRUE)
        UI8_T   buff1[48] = {0};

        sprintf(buff1, "smtp source email address");
        EH_MGR_Handle_Exception1(SYS_MODULE_SMTP, SMTP_MGR_SET_SOURCE_EMAIL_ADDR_FUNC_NO,
                EH_TYPE_MSG_INVALID, (SYSLOG_LEVEL_INFO), buff1);
#endif
        SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_INVALID);
    }

    SMTP_OM_GetSmtpSourceEmailAddr(om_addr);
    if (strcmp((char *)email_addr,(char *)om_addr) != 0)
    {
        ret = SMTP_OM_SetSmtpSourceEmailAddr(email_addr);
        SMTP_MGR_RETURN_AND_RELEASE_CSC(ret);
    }

    SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_SUCCESS);
}/* End of SMTP_MGR_SetSmtpSourceEmailAddr() */

//ADD daniel
/* FUNCTION NAME: SMTP_MGR_DeleteSmtpSourceEmailAddr
 * PURPOSE: This function is used to delete the smtp source email address.
 * INPUT:   *email_addr -- smtp source email address.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID  -- invalid input value
 * NOTES:   1. There is only one source email address
 *          2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
UI32_T SMTP_MGR_DeleteSmtpSourceEmailAddr(void)
{
    UI8_T  om_addr[SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1] = {0};
    UI32_T  ret;

    SMTP_MGR_USE_CSC_CHECK_OPER_MODE(SMTP_RETURN_FAIL);

    SMTP_OM_GetSmtpSourceEmailAddr(om_addr);
    if (om_addr != NULL)
    {
        ret = SMTP_OM_DeleteSmtpSourceEmailAddr(om_addr);
        SMTP_MGR_RETURN_AND_RELEASE_CSC(ret);
    }

    SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_INVALID);
}/* End of SMTP_MGR_SetSmtpSourceEmailAddr() */
//end ADD daniel


/* FUNCTION NAME: SMTP_MGR_GetSmtpSourceEmailAddr
 * PURPOSE: This function is used to get the smtp source email address.
 * INPUT:   *email_addr -- output buffer of smtp source email address.
 * OUTPUT:  *email_addr -- smtp source email address.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID_BUFFER   -- invalid input buffer
 * NOTES:   1. There is only one source email address
 *          2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
UI32_T SMTP_MGR_GetSmtpSourceEmailAddr(UI8_T *email_addr)
{
    UI32_T  ret;

    SMTP_MGR_USE_CSC_CHECK_OPER_MODE(SMTP_RETURN_FAIL);

    if (email_addr == NULL)
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_SMTP, SMTP_MGR_GET_SOURCE_EMAIL_ADDR_FUNC_NO,
            EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif
        SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_INVALID_BUFFER);
    }

    ret = SMTP_OM_GetSmtpSourceEmailAddr(email_addr);

    SMTP_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of SMTP_MGR_GetSmtpSourceEmailAddr() */

/* FUNCTION NAME: SMTP_MGR_GetRunningSmtpSourceEmailAddr
 * PURPOSE: This function is used to get the running smtp source email address.
 * INPUT:   *email_addr -- output buffer of smtp source email address.
 * OUTPUT:  *email_addr -- smtp source email address.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. There is only one source email address.
 *          2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
SYS_TYPE_Get_Running_Cfg_T SMTP_MGR_GetRunningSmtpSourceEmailAddr(UI8_T *email_addr)
{
    UI32_T  ret;

    SMTP_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (email_addr == NULL)
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_SMTP, SMTP_MGR_GET_RUNNING_SOURCE_EMAIL_ADDR_FUNC_NO,
            EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif
        SMTP_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    ret = SMTP_OM_GetSmtpSourceEmailAddr(email_addr);

    if (ret != SMTP_RETURN_SUCCESS)
        SMTP_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (strcmp((char *)email_addr,SYS_DFLT_SMTP_SOURCE_EMAIL_ADDR) == 0)
        SMTP_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);

    SMTP_MGR_RETURN_AND_RELEASE_CSC(SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
}/* End of SMTP_MGR_GetRunningSmtpSourceEmailAddr() */

/* FUNCTION NAME: SMTP_MGR_AddSmtpDestinationEmailAddr
 * PURPOSE: This function is used to add the smtp destination email address.
 * INPUT:   *email_addr -- smtp destination email address.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_INVALID  -- invalid input value
 *          SMTP_RETURN_INPUT_EXIST  -- input value already exist
 *          SMTP_RETURN_DATABASE_FULL -- database full
 * NOTES:   1. Add one destination email address one time
 *          2. Max number of destination email address is SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS.
 *          3. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
UI32_T SMTP_MGR_AddSmtpDestinationEmailAddr(UI8_T *email_addr)
{
    UI32_T  ret = SMTP_RETURN_SUCCESS;

    SMTP_MGR_USE_CSC_CHECK_OPER_MODE(SMTP_RETURN_SUCCESS);

    if (SMTP_RETURN_SUCCESS != SMTP_MGR_ValidateEmailAddress((const char *)email_addr))
    {
#if (SYS_CPNT_EH == TRUE)
        UI8_T   buff1[48] = {0};

        sprintf(buff1, "smtp destination email address");
        EH_MGR_Handle_Exception1(SYS_MODULE_SMTP, SMTP_MGR_SET_DES_EMAIL_ADDR_FUNC_NO,
                EH_TYPE_MSG_INVALID, (SYSLOG_LEVEL_INFO), buff1);
#endif

        SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_INVALID);
    }

    ret = SMTP_OM_SetSmtpDestinationEmailAddr(email_addr);

    SMTP_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of SMTP_MGR_AddSmtpDestinationEmailAddr() */

/* FUNCTION NAME: SMTP_MGR_GetSmtpDestinationEmailAddr
 * PURPOSE: This function is used to get smtp destination email address.
 * INPUT:   *email_addr -- output buffer of smtp destination email address.
 * OUTPUT:  *email_addr -- smtp destination email address.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID  -- invalid input value
 *          SMTP_RETURN_INVALID_BUFFER   -- invalid input buffer
 * NOTES:   1. Max number of destination email address is SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS.
 *          2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
UI32_T SMTP_MGR_GetSmtpDestinationEmailAddr(UI8_T *email_addr)
{
    UI8_T temp_addr[SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1] = {0};
    UI8_T best_addr[SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1] = {0};

    SMTP_MGR_USE_CSC_CHECK_OPER_MODE(SMTP_RETURN_FAIL);

    if(email_addr == NULL)
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_SMTP, SMTP_MGR_GET_DES_EMAIL_ADDR_FUNC_NO,
            EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif
        SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_INVALID_BUFFER);
    }

    memset(temp_addr,'\0',SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1);

    while(SMTP_OM_GetNextSmtpDestinationEmailAddr(temp_addr) == SMTP_RETURN_SUCCESS)
    {
        strcpy((char *)best_addr,(char *)temp_addr);

        if (strcmp((char *)best_addr,(char *)email_addr) == 0)
        {
            strcpy((char *)email_addr,(char *)best_addr);
            SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_SUCCESS);
        }
    }

    SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_FAIL);
}/* End of SMTP_MGR_GetSmtpDestinationEmailAddr() */

/* FUNCTION NAME: SMTP_MGR_GetNextSmtpDestinationEmailAddr
 * PURPOSE: This function is used to get next smtp destination email address.
 * INPUT:   *email_addr -- output buffer of smtp destination email address.
 * OUTPUT:  *email_addr -- smtp destination email address.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID  -- invalid input value
 *          SMTP_RETURN_INVALID_BUFFER   -- invalid input buffer
 * NOTES:   1. Max number of destination email address is SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS.
 *          2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
UI32_T SMTP_MGR_GetNextSmtpDestinationEmailAddr(UI8_T *email_addr)
{
    UI32_T ret;
    UI8_T temp_addr[SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1] = {0};
    UI8_T best_addr[SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1] = {0};

    SMTP_MGR_USE_CSC_CHECK_OPER_MODE(SMTP_RETURN_FAIL);

    if(email_addr == NULL)
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_SMTP, SMTP_MGR_GET_NEXT_DES_EMAIL_ADDR_FUNC_NO,
            EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif
        SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_INVALID_BUFFER);
    }

    memset(temp_addr,'\0',SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1);
    strcpy((char *)temp_addr,(char *)email_addr);
    ret = SMTP_OM_GetNextSmtpDestinationEmailAddr(temp_addr);
    if (ret == SMTP_RETURN_SUCCESS)
    {
        strcpy((char *)best_addr,(char *)temp_addr);

        if (strcmp((char *)best_addr,"") != 0)
        {
            strcpy((char *)email_addr,(char *)best_addr);
            SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_SUCCESS);
        }
    }

    SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_FAIL);
}/* End of SMTP_MGR_GetNextSmtpDestinationEmailAddr() */

/* FUNCTION NAME: SMTP_MGR_GetNextRunningSmtpDestinationEmailAddr
 * PURPOSE: This function is used to get next running smtp destination email address.
 * INPUT:   *email_addr -- output buffer of smtp destination email address.
 * OUTPUT:  *email_addr -- smtp destination email address.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. Max number of destination email address is SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS.
 *          2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
SYS_TYPE_Get_Running_Cfg_T SMTP_MGR_GetNextRunningSmtpDestinationEmailAddr(UI8_T *email_addr)
{
    UI32_T  ret;

    ret = SMTP_MGR_GetNextSmtpDestinationEmailAddr(email_addr);

    if (ret != SMTP_RETURN_SUCCESS)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (strcmp((char *)email_addr,SYS_DFLT_SMTP_DESTINATION_EMAIL_ADDR) == 0)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/* End of SMTP_MGR_GetNextRunningSmtpDestinationEmailAddr() */

/* FUNCTION NAME: SMTP_MGR_DeleteSmtpDestinationEmailAddr
 * PURPOSE: This function is used to delete the smtp destination email address.
 * INPUT:   *email_addr -- smtp destination email address.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID  -- invalid input value
 * NOTES:   1. delete one destination email address one time.
 *          2. Max number of destination email address is SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS.
 *          3. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *
 */
UI32_T SMTP_MGR_DeleteSmtpDestinationEmailAddr(UI8_T *email_addr)
{
    UI32_T  ret;

    SMTP_MGR_USE_CSC_CHECK_OPER_MODE(SMTP_RETURN_FAIL);

    /* not in slave mode */
    if(strlen((char *)email_addr) > SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS)
    {
        SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_INVALID);
    }

    ret = SMTP_OM_DeleteSmtpDestinationEmailAddr(email_addr);

    SMTP_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of SMTP_MGR_DeleteSmtpDestinationEmailAddr() */

/* FUNCTION NAME: SMTP_MGR_EnableSmtpAdminStatus
 * PURPOSE: This function is used to enable smtp admin status.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 * NOTES:   1. status is defined as following:
 *             SMTP_STATUS_ENABLE
 *             SMTP_STATUS_DISABLE
 *
 */
UI32_T SMTP_MGR_EnableSmtpAdminStatus(void)
{
    UI32_T  om_status, ret, set_status = SMTP_STATUS_ENABLE;

    SMTP_MGR_USE_CSC_CHECK_OPER_MODE(SMTP_RETURN_FAIL);

    SMTP_OM_GetSmtpAdminStatus(&om_status);
    if (set_status != om_status)
    {
        ret = SMTP_OM_SetSmtpAdminStatus(set_status);
        SMTP_MGR_RETURN_AND_RELEASE_CSC(ret);
    }

    SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_SUCCESS);
}/* End of SMTP_MGR_EnableSmtpAdminStatus() */

/* FUNCTION NAME: SMTP_MGR_DisableSmtpAdminStatus
 * PURPOSE: This function is used to disable smtp admin status.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 * NOTES:   1. status is defined as following:
 *             SMTP_STATUS_ENABLE
 *             SMTP_STATUS_DISABLE
 *
 */
UI32_T SMTP_MGR_DisableSmtpAdminStatus(void)
{
    UI32_T  om_status, ret, set_status = SMTP_STATUS_DISABLE;

    SMTP_MGR_USE_CSC_CHECK_OPER_MODE(SMTP_RETURN_FAIL);

    SMTP_OM_GetSmtpAdminStatus(&om_status);
    if (set_status != om_status)
    {
        ret = SMTP_OM_SetSmtpAdminStatus(set_status);
        SMTP_MGR_RETURN_AND_RELEASE_CSC(ret);
    }

    SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_SUCCESS);
}/* End of SMTP_MGR_DisableSmtpAdminStatus() */

/* FUNCTION NAME: SMTP_MGR_GetSmtpAdminStatus
 * PURPOSE: This function is used to get smtp admin status.
 * INPUT:   *status -- output buffer of smtp status.
 * OUTPUT:  *status -- smtp status.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 *          SMTP_RETURN_INVALID_BUFFER   -- invalid input buffer
 * NOTES:   1. status is defined as following:
 *             SMTP_STATUS_ENABLE
 *             SMTP_STATUS_DISABLE
 *
 */
UI32_T SMTP_MGR_GetSmtpAdminStatus(UI32_T *status)
{
    UI32_T  ret;

    SMTP_MGR_USE_CSC_CHECK_OPER_MODE(SMTP_RETURN_FAIL);

    if (status == NULL)
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_SMTP, SMTP_MGR_GET_ADMIN_STATUS_FUNC_NO,
            EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif
        SMTP_MGR_RETURN_AND_RELEASE_CSC(SMTP_RETURN_INVALID_BUFFER);
    }

    ret = SMTP_OM_GetSmtpAdminStatus(status);

    SMTP_MGR_RETURN_AND_RELEASE_CSC(ret);
}/* End of SMTP_MGR_GetSmtpAdminStatus() */

/* FUNCTION NAME: SMTP_MGR_GetRunningSmtpAdminStatus
 * PURPOSE: This function is used to get running smtp status.
 * INPUT:   *status -- output buffer of smtp status.
 * OUTPUT:  *status -- smtp status.
 * RETUEN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:   1. status is defined as following:
 *             SMTP_STATUS_ENABLE
 *             SMTP_STATUS_DISABLE
 *          2. default status is SYS_DFLT_SMTP_ADMIN_STATUS
 *
 */
SYS_TYPE_Get_Running_Cfg_T SMTP_MGR_GetRunningSmtpAdminStatus(UI32_T *status)
{
    UI32_T  ret;

    ret = SMTP_MGR_GetSmtpAdminStatus(status);

    if (ret != SMTP_RETURN_SUCCESS)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (*status == SYS_DFLT_SMTP_ADMIN_STATUS)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}/* End of SMTP_MGR_GetRunningSmtpAdminStatus() */

/* FUNCTION NAME: SMTP_MGR_SendMail
 * PURPOSE: This function is used to send mail.
 * INPUT:   smtp_entry -- smtp entry.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   1. check smtp entey level
 *          2. if smtp entry level is higher than configured level,keep the entry into queue.
 *
 */
void
SMTP_MGR_SendMail(
    SMTP_OM_Record_T *smtp_entry)
{
    UI32_T                  smtp_level;
    SMTP_OM_QueueRecord_T   *smtp_data;

    SMTP_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    if (SMTP_MGR_GetEmailSeverityLevel(&smtp_level) != SMTP_RETURN_SUCCESS)
    {
        if (debug_mode)
        {
            printf("FUNCTION:SMTP_MGR_SendMail; DESC:Reject, get level fail\n");
        }

        SMTP_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }

    if (smtp_level < smtp_entry->owner_info.level)
    {
        if (debug_mode)
        {
            printf("FUNCTION:SMTP_MGR_SendMail; DESC:Reject, log level is larger than configure level\n");
        }
        SMTP_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
    }

    smtp_data = (SMTP_OM_QueueRecord_T *)L_MM_Malloc(sizeof(SMTP_OM_QueueRecord_T), L_MM_USER_ID2(SYS_MODULE_SMTP, SMTP_TYPE_TRACE_ID_SMTP_MGR_SENDMAIL));
    memcpy(&(smtp_data->smtp_entry),smtp_entry,sizeof(SMTP_OM_Record_T));

    if (debug_mode)
    {
        printf("FUNCTION:SMTP_MGR_SendMail; DESC:Success, log will be enqueue\n");
    }

    SMTP_OM_QueueEnqueue(smtp_data);
    SYSFUN_SendEvent(SMTP_OM_GetTaskId(), SMTP_TASK_EVENT_SEND_MAIL);

    if (debug_mode)
    {
        printf("FUNCTION:SMTP_MGR_SendMail; DESC:Success, log have been enqueue\n");
    }

    SMTP_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
}/* End of SMTP_MGR_SendMail() */

/* FUNCTION NAME: SMTP_MGR_HandleTrapQueue
 * PURPOSE: This function is used to send smtp mail when task recieve 
 *          SMTP_TASK_EVENT_SEND_MAIL or SMTP_TASK_EVENT_PERIODIC_TIMER event.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   1. if there is any entry in queue,
 *             then create connection,dequeue the entry and send out mail
 *          2. if send mail fail,enqueue the entry again.
 *          3. Before end of this function,close the connection.
 *          4. if send mail successfully,free the entry.
 *
 */
SMTP_MGR_RETURN_T
SMTP_MGR_HandleTrapQueue(
    void)
{
#if (SYS_CPNT_SMTP == TRUE)
#define SMTP_MGR_MAX_NBR_OF_DEQUEUE_PER_EVENT     1
    UI32_T                  status, ret, SmtpSrvrIpAddr = 0;
    UI32_T                  dequeue_counter = 0;
    SMTP_OM_QueueRecord_T   *smtp_data = (SMTP_OM_QueueRecord_T *)NULL;
    UI16_T                  i;
    UI8_T                   patten[SMTP_MGR_MAX_LENGTH_OF_MSG_PATTERN+1] = {0};
    UI8_T                   temp[SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1] = {0};
    UI8_T                   flag = 0;

    if(SMTP_OM_QueueGetElementNbr()!= 0)
    {
        if (debug_mode)
            printf("FUNCTION:SMTP_MGR_HandleTrapQueue; DESC:queue is not empty\n");

        ret = SMTP_OM_GetSmtpAdminStatus(&status);
        if((ret == SMTP_RETURN_SUCCESS) && (status == SMTP_STATUS_DISABLE))
        {
            if (debug_mode)
                  printf("FUNCTION:SMTP_MGR_HandleTrapQueue; DESC:clear queue\n");

            /* clear queue member */
            SMTP_OM_ClearQueue();
            return SMTP_MGR_RETURN_FAIL;
        }

        strcpy((char *)temp,"");
        ret = SMTP_OM_GetSmtpSourceEmailAddr(temp);

        if (strcmp((char *)temp,SYS_DFLT_SMTP_SOURCE_EMAIL_ADDR) == 0)
        {
            return SMTP_MGR_RETURN_FAIL;
        }

        i = 0;

        if(smtp_mgr_server_ip == 0)
        {
            while(SMTP_OM_GetNextSmtpServerIPAddr(&SmtpSrvrIpAddr) == SMTP_RETURN_SUCCESS)
            {
                if(SMTP_MGR_IsLocalIP(SmtpSrvrIpAddr) == TRUE)
                {
                    continue;
                }
                flag = 1;
                smtp_mgr_server_ip = SmtpSrvrIpAddr;
                break;
            }
        }
        else
        {
            if(SMTP_OM_IsSmtpServerIPAddrExist(smtp_mgr_server_ip) != SMTP_RETURN_SUCCESS)
            {
                smtp_mgr_server_ip = 0;
                SmtpSrvrIpAddr = 0;
                while(SMTP_OM_GetNextSmtpServerIPAddr(&SmtpSrvrIpAddr) == SMTP_RETURN_SUCCESS)
                {
                    if(SMTP_MGR_IsLocalIP(SmtpSrvrIpAddr) == TRUE)
                {
                        continue;
                    }
                    flag = 1;
                    smtp_mgr_server_ip = SmtpSrvrIpAddr;
                    break;
                }
            }
            else if(SMTP_MGR_IsLocalIP(smtp_mgr_server_ip) == TRUE)
            {
                  UI32_T temp_ip;
                  temp_ip = smtp_mgr_server_ip;
                  while(SMTP_OM_GetNextSmtpServerIPAddr(&temp_ip) == SMTP_RETURN_SUCCESS)
                {
                    if(SMTP_MGR_IsLocalIP(temp_ip) == TRUE)
                    {
                        continue;
                    }
                    flag = 1;
                    smtp_mgr_server_ip = temp_ip;
                    break;
                }
                if(flag == 0)
                {
                    temp_ip = 0;
                    while(SMTP_OM_GetNextSmtpServerIPAddr(&temp_ip) == SMTP_RETURN_SUCCESS)
                    {
                        if(SMTP_MGR_IsLocalIP(temp_ip) == TRUE)
                        {
                            continue;
                        }
                        flag = 1;
                        smtp_mgr_server_ip = temp_ip;
                        break;
                    }
                }
            }
            else
            {
                flag = 1;
            }

            SmtpSrvrIpAddr = smtp_mgr_server_ip;
        }

        /*no server ip,return*/
        if ((SmtpSrvrIpAddr == 0) || (flag == 0))
        {
            if (debug_mode)
                printf("FUNCTION:SMTP_MGR_HandleTrapQueue; DESC:not valid server ip\n");
            return SMTP_MGR_RETURN_FAIL;
        }

        strcpy((char *)temp,"");

        while(SMTP_OM_GetNextSmtpDestinationEmailAddr(temp) == SMTP_RETURN_SUCCESS)
        {
            i++;
        }

        /* no destination email address,return */
        if(i == 0)
        {
            return SMTP_MGR_RETURN_FAIL;
        }

        /* create smtp connection*/
        if(SMTP_MGR_Connect(SmtpSrvrIpAddr,&smtp_mgr_socket_id) != SMTP_RETURN_SUCCESS)
        {
            {
                if (debug_mode)
                  printf("FUNCTION:SMTP_MGR_HandleTrapQueue; DESC:connect server ip %d.%d.%d.%d fail\n",
                          (UI8_T)((SmtpSrvrIpAddr>>24)&255),(UI8_T)((SmtpSrvrIpAddr>>16)&255),
                          (UI8_T)((SmtpSrvrIpAddr>>8)&255),(UI8_T)(SmtpSrvrIpAddr&255));
                if(SMTP_OM_GetNextSmtpServerIPAddr(&SmtpSrvrIpAddr) != SMTP_RETURN_SUCCESS)
                {
                    SmtpSrvrIpAddr = 0;
                    if(SMTP_OM_GetNextSmtpServerIPAddr(&SmtpSrvrIpAddr) != SMTP_RETURN_SUCCESS)
                    {
                        return SMTP_MGR_RETURN_FAIL;
                    }
                    else
                    {
                        if(SmtpSrvrIpAddr == smtp_mgr_server_ip)
                        {
                            return SMTP_MGR_RETURN_FAIL;
                        }
                        if(SMTP_MGR_IsLocalIP(SmtpSrvrIpAddr) == TRUE)
                        {
                            smtp_mgr_server_ip = SmtpSrvrIpAddr;
                            return SMTP_MGR_RETURN_FAIL;
                        }

                        /* before reconnection,close first */
                        //SMTP_MGR_Close(smtp_mgr_socket_id);
                        /*reconnection*/
                        if(SMTP_MGR_Connect(SmtpSrvrIpAddr,&smtp_mgr_socket_id) != SMTP_RETURN_SUCCESS)
                        {
                            if (debug_mode)
                               printf("FUNCTION:SMTP_MGR_HandleTrapQueue; DESC:connect server ip %d.%d.%d.%d fail\n",
                                       (UI8_T)((SmtpSrvrIpAddr>>24)&255),(UI8_T)((SmtpSrvrIpAddr>>16)&255),
                                       (UI8_T)((SmtpSrvrIpAddr>>8)&255),(UI8_T)(SmtpSrvrIpAddr&255));
                            smtp_mgr_server_ip = SmtpSrvrIpAddr;

                            return SMTP_MGR_RETURN_FAIL;
                        }
                        smtp_mgr_server_ip = SmtpSrvrIpAddr;
                    }
                }
                else
                {
                    if(SmtpSrvrIpAddr == smtp_mgr_server_ip)
                    {
                        return SMTP_MGR_RETURN_FAIL;
                    }
                    if(SMTP_MGR_IsLocalIP(SmtpSrvrIpAddr) == TRUE)
                    {
                          smtp_mgr_server_ip = SmtpSrvrIpAddr;
                           return SMTP_MGR_RETURN_FAIL;
                    }
                    /* before reconnection,close first */
                    //SMTP_MGR_Close(smtp_mgr_socket_id);
                    /*reconnection*/
                    if(SMTP_MGR_Connect(SmtpSrvrIpAddr,&smtp_mgr_socket_id) != SMTP_RETURN_SUCCESS)
                    {
                        if (debug_mode)
                            printf("FUNCTION:SMTP_MGR_HandleTrapQueue; DESC:connect server ip %d.%d.%d.%d fail\n",
                                    (UI8_T)((SmtpSrvrIpAddr>>24)&255),(UI8_T)((SmtpSrvrIpAddr>>16)&255),
                                    (UI8_T)((SmtpSrvrIpAddr>>8)&255),(UI8_T)(SmtpSrvrIpAddr&255));

                        if(SMTP_OM_GetNextSmtpServerIPAddr(&SmtpSrvrIpAddr) != SMTP_RETURN_SUCCESS)
                        {
                            smtp_mgr_server_ip = SmtpSrvrIpAddr;
                            return SMTP_MGR_RETURN_FAIL;
                        }
                        else
                        {
                            if(SmtpSrvrIpAddr == smtp_mgr_server_ip)
                            {
                                return SMTP_MGR_RETURN_FAIL;
                            }
                            if(SMTP_MGR_IsLocalIP(SmtpSrvrIpAddr) == TRUE)
                            {
                                smtp_mgr_server_ip = SmtpSrvrIpAddr;
                                return SMTP_MGR_RETURN_FAIL;
                            }
                            /* before reconnection,close first */
                            //SMTP_MGR_Close(smtp_mgr_socket_id);
                            /*reconnection*/
                            if(SMTP_MGR_Connect(SmtpSrvrIpAddr,&smtp_mgr_socket_id) != SMTP_RETURN_SUCCESS)
                            {
                                if (debug_mode)
                                    printf("FUNCTION:SMTP_MGR_HandleTrapQueue; DESC:connect server ip %d.%d.%d.%d fail\n",
                                            (UI8_T)((SmtpSrvrIpAddr>>24)&255),(UI8_T)((SmtpSrvrIpAddr>>16)&255),
                                            (UI8_T)((SmtpSrvrIpAddr>>8)&255),(UI8_T)(SmtpSrvrIpAddr&255));
                                smtp_mgr_server_ip = SmtpSrvrIpAddr;

                                return SMTP_MGR_RETURN_FAIL;
                            }
                            smtp_mgr_server_ip = SmtpSrvrIpAddr;
                        }
                    }
                    smtp_mgr_server_ip = SmtpSrvrIpAddr;
                }
            }
        }

        send_hello_flag = 1;
        while ((smtp_data = SMTP_OM_QueueDequeue()) != (SMTP_OM_QueueRecord_T *)NULL)
        {
            SMTP_MGR_LogToMsg(smtp_data,patten);

            while (SMTP_MGR_Send(patten) != SMTP_RETURN_SUCCESS)
            {
                if (debug_mode)
                  printf("FUNCTION:SMTP_MGR_HandleTrapQueue; DESC:SMTP_MGR_Send return false\n");

                if(SMTP_OM_GetNextSmtpServerIPAddr(&SmtpSrvrIpAddr) != SMTP_RETURN_SUCCESS)
                {
                    SmtpSrvrIpAddr = 0;
                    if(SMTP_OM_GetNextSmtpServerIPAddr(&SmtpSrvrIpAddr) != SMTP_RETURN_SUCCESS)
                    {
                        SMTP_OM_QueueEnqueue(smtp_data);
                        SMTP_MGR_Close(smtp_mgr_socket_id);
                        if (debug_mode)
                            printf("FUNCTION:SMTP_MGR_HandleTrapQueue; DESC:Send fail and return\n");
                        return SMTP_MGR_RETURN_FAIL;
                    }
                    else
                    {
                        if(SmtpSrvrIpAddr == smtp_mgr_server_ip)
                        {
                            SMTP_OM_QueueEnqueue(smtp_data);
                            SMTP_MGR_Close(smtp_mgr_socket_id);
                            if (debug_mode)
                                printf("FUNCTION:SMTP_MGR_HandleTrapQueue; DESC:Send fail and return\n");
                            return SMTP_MGR_RETURN_FAIL;
                        }
                        if(SMTP_MGR_IsLocalIP(SmtpSrvrIpAddr) == TRUE)
                        {
                            SMTP_OM_QueueEnqueue(smtp_data);
                            SMTP_MGR_Close(smtp_mgr_socket_id);
                            if (debug_mode)
                                printf("FUNCTION:SMTP_MGR_HandleTrapQueue; DESC:Send fail and return\n");
                            smtp_mgr_server_ip = SmtpSrvrIpAddr;
                            return SMTP_MGR_RETURN_FAIL;
                        }
                        /* before reconnection,close first */
                        SMTP_MGR_Close(smtp_mgr_socket_id);
                        /*reconnection*/
                        if(SMTP_MGR_Connect(SmtpSrvrIpAddr,&smtp_mgr_socket_id) != SMTP_RETURN_SUCCESS)
                        {
                            if (debug_mode)
                                printf("FUNCTION:SMTP_MGR_HandleTrapQueue; DESC:connect server ip %d.%d.%d.%d fail\n",
                                        (UI8_T)((SmtpSrvrIpAddr>>24)&255),(UI8_T)((SmtpSrvrIpAddr>>16)&255),
                                        (UI8_T)((SmtpSrvrIpAddr>>8)&255),(UI8_T)(SmtpSrvrIpAddr&255));
                            smtp_mgr_server_ip = SmtpSrvrIpAddr;
                            SMTP_OM_QueueEnqueue(smtp_data);

                            if (debug_mode)
                               printf("FUNCTION:SMTP_MGR_HandleTrapQueue; DESC:Send fail and return\n");
                            return SMTP_MGR_RETURN_FAIL;
                        }
                        smtp_mgr_server_ip = SmtpSrvrIpAddr;
                        send_hello_flag = 1;
                    }
                }
                else
                {
                    if(SmtpSrvrIpAddr == smtp_mgr_server_ip)
                    {
                        SMTP_OM_QueueEnqueue(smtp_data);
                        SMTP_MGR_Close(smtp_mgr_socket_id);
                        if (debug_mode)
                            printf("FUNCTION:SMTP_MGR_HandleTrapQueue; DESC:Send fail and return\n");
                        return SMTP_MGR_RETURN_FAIL;
                    }
                    if(SMTP_MGR_IsLocalIP(SmtpSrvrIpAddr) == TRUE)
                    {
                        SMTP_OM_QueueEnqueue(smtp_data);
                        SMTP_MGR_Close(smtp_mgr_socket_id);
                        if (debug_mode)
                            printf("FUNCTION:SMTP_MGR_HandleTrapQueue; DESC:Send fail and return\n");
                        smtp_mgr_server_ip = SmtpSrvrIpAddr;
                        return SMTP_MGR_RETURN_FAIL;
                    }
                    /* before reconnection,close first */
                    SMTP_MGR_Close(smtp_mgr_socket_id);
                    /*reconnection*/
                    if(SMTP_MGR_Connect(SmtpSrvrIpAddr,&smtp_mgr_socket_id) != SMTP_RETURN_SUCCESS)
                    {
                        if (debug_mode)
                              printf("FUNCTION:SMTP_MGR_HandleTrapQueue; DESC:connect server ip %d.%d.%d.%d fail\n",
                                      (UI8_T)((SmtpSrvrIpAddr>>24)&255),(UI8_T)((SmtpSrvrIpAddr>>16)&255),
                                      (UI8_T)((SmtpSrvrIpAddr>>8)&255),(UI8_T)(SmtpSrvrIpAddr&255));
                        smtp_mgr_server_ip = SmtpSrvrIpAddr;
                        SMTP_OM_QueueEnqueue(smtp_data);

                        if (debug_mode)
                           printf("FUNCTION:SMTP_MGR_HandleTrapQueue; DESC:Send fail and return\n");
                        return SMTP_MGR_RETURN_FAIL;
                    }
                    smtp_mgr_server_ip = SmtpSrvrIpAddr;
                    send_hello_flag = 1;
                }
            }

            /* Release memory when trap is successfully send.
             */
             send_hello_flag = 0;
            if(smtp_data != (SMTP_OM_QueueRecord_T *)NULL)
            {
                L_MM_Free((void *)smtp_data);
            }

            dequeue_counter ++;
            if (SMTP_MGR_MAX_NBR_OF_DEQUEUE_PER_EVENT <= dequeue_counter)
            {
                break;
            }
        } /* End of while */

        if(send_hello_flag == 0)
        {
            send(smtp_mgr_socket_id, "QUIT\r\n", strlen("QUIT\r\n"), 0);
            if (debug_mode)
                printf("FUNCTION:SMTP_MGR_HandleTrapQueue; DESC:send QUIT to close connection.\n");
        }

        smtp_mgr_server_ip = SmtpSrvrIpAddr;
        /*close connection*/
        SMTP_MGR_Close(smtp_mgr_socket_id);
        if (debug_mode)
            printf("FUNCTION:SMTP_MGR_HandleTrapQueue; DESC:finish sending mail in queue\n");
    }
    else
    {
        if (debug_mode)
            printf("FUNCTION:SMTP_MGR_HandleTrapQueue; DESC:there is no queue member\n");

        return SMTP_MGR_RETURN_QUEUE_EMPTY;
    }

#endif /* #if(SYS_CPNT_SMTP == TRUE) */
    return SMTP_MGR_RETURN_SUCCESS;
}/* End of SMTP_MGR_HandleTrapQueue() */


/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME: SMTP_MGR_GetResponseCode
 * PURPOSE: This function is to get the response code from the response string.
 * INPUT:   now_state
 * OUTPUT:  res_code
 * RETUEN:  TRUE    -- success
 *          FALSE   -- fail
 * NOTES:   None.
 */
static BOOL_T SMTP_MGR_GetResponseCode (UI8_T *recv_buf_p, UI16_T *res_code_p)
{
    UI8_T   *tmp_recv_p;
    BOOL_T  ret = FALSE;

    /* according to rfc-2821, 4.2
     *   Reply-line = Reply-code [ SP text ] CRLF
     */
    tmp_recv_p = (UI8_T *)strtok ((char *)recv_buf_p, "\x0d\x0a");

    while ((tmp_recv_p != NULL) && (strlen ((char *)tmp_recv_p) >= 3))
    {
        switch (tmp_recv_p[3])
        {
        case '-':
            /* multiple reply, 4.2.1
             */
             tmp_recv_p = (UI8_T *)strtok (NULL, "\x0d\x0a");

             if (NULL != tmp_recv_p)
             {
                if (debug_mode)
                {
                    printf("recv multiple reply in one response\n");
                }

                continue;
             }
            break;

        case ' ':
            /* final reply
             */
            tmp_recv_p[3] = '\0';

        case '\0':
            *res_code_p = atoi((char *)tmp_recv_p);
            ret = TRUE;

        default:
            tmp_recv_p = NULL;
            break;
        } /* switch (tmp_recv_p[3]) */
    } /* while ((tmp_recv_p != NULL) && (strlen (tmp_recv_p) >= 3)) */

    return ret;
}

/* FUNCTION NAME: SMTP_MGR_ProcessResponse
 * PURPOSE: This function is to process the response data from SMTP server.
 * INPUT:   now_state
 * OUTPUT:  res_code
 * RETUEN:  TRUE    -- success
 *          FALSE   -- fail
 * NOTES:   None.
 */
static BOOL_T SMTP_MGR_ProcessResponse (UI16_T now_state, UI16_T *res_code_p)
{
    int     rc, retry =0;
    //UI8_T   *recv_p;
     UI8_T   recvbuf[SMTP_ADPT_BUF_SIZE];
    BOOL_T  ret = TRUE;

    if (debug_mode)
    {
        printf("recv response for sta-%d\n", now_state);
    }

    while (1)
    {
        memset (recvbuf, 0, sizeof (recvbuf));

        rc = recv(smtp_mgr_socket_id, recvbuf, SMTP_ADPT_BUF_SIZE, 0);

        if (debug_mode)
        {
            printf("rc-%d\n", rc);

            if (rc > 0)
                printf("recvbuf-%s\n", recvbuf);
        }

        switch (rc)
        {
        case -1:
            /* error    */
            if ((errno == EINTR) || (errno == EWOULDBLOCK))
            {
                if (debug_mode)
                {
                    printf (" continue errno-%d\n", errno);
                }

                if (++ retry <=3)
                    continue;
            }

        case  0:
            /* shutdown */
            ret = FALSE;
            break;

        default:
            break;
        } /* switch (rc) */

        if (TRUE == ret)
        {
            if (FALSE == SMTP_MGR_GetResponseCode(recvbuf, res_code_p))
            {
                if (++retry <= 3)
                    continue;

                ret = FALSE;
            }
        }

        break;
    } /* while (1) */

    return ret;
}

/* FUNCTION NAME: SMTP_MGR_Send
 * PURPOSE: This function is used to send out smtp mail to mail server.
 * INPUT:   *patten -- smtp mail patten.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 * NOTES:   1.state machine of connection is as follow:
 *             1) SMTP_MGR_STATE_INIT
 *             2) SMTP_MGR_STATE_HELLO
 *             3) SMTP_MGR_STATE_MAILFROM
 *             4) SMTP_MGR_STATE_RCPTTO
 *             5) SMTP_MGR_STATE_DATA
 *             6) SMTP_MGR_STATE_CONTENT
 *             7) SMTP_MGR_STATE_FINISH
 *             8) SMTP_MGR_STATE_QUIT
 *          2.send one mail to all configured server in this function
 *          3.smtp mail server reply code is as follow:
 *             1) SMTP_MGR_SERVICE_READY
 *             2) SMTP_MGR_SERVICE_CLOSING
 *             3) SMTP_MGR_MAIL_ACTION_OK
 *             4) SMTP_MGR_START_MAIL_INPUT
 *
 */
static UI32_T SMTP_MGR_Send(UI8_T *patten)
{
    int year,month,day,hour,minute,second;
      UI16_T i,j, nowstate, rescode,temp_rescode;
    UI16_T server_num, flag = 0;
    UI8_T  sourceaddr[SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1] = {0};
    UI8_T  temp[SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1] = {0};
    UI8_T  destinationaddr[SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS][SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1];
      UI8_T  Mailcmd[SMTP_ADPT_CONTENT_SIZE] = {0};
    UI8_T  Name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN + 1]     = {0};
    NETCFG_TYPE_InetRifConfig_T  rifConfig;

    if(SMTP_OM_GetSmtpSourceEmailAddr(sourceaddr) != SMTP_RETURN_SUCCESS)
    {
        return SMTP_RETURN_FAIL;
    }

    for(i=0;i<SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS;i++)
    {
        memset(destinationaddr[i],'\0',SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1);
    }

    i = 0;
    memset(temp,'\0',SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS+1);

    while(SMTP_OM_GetNextSmtpDestinationEmailAddr(temp) == SMTP_RETURN_SUCCESS)
    {
        strcpy((char *)destinationaddr[i],(char *)temp);
        i++;
    }

    server_num = i;
    /*no destination email address,return*/
    if (i == 0)
    {
        return    SMTP_RETURN_FAIL;
    }

    if (debug_mode)
        printf("FUNCTION:SMTP_MGR_Send; DESC:send hello flag is %ld\n", (long)send_hello_flag);

    if(send_hello_flag == 1)
    {
        /* Send Hello */

        MIB2_POM_GetSysName(Name);
        if(strlen((char *)Name) != 0)
        {
            SMTP_UTIL_Print_SysName(Mailcmd, Name);
        }
        else
        {
            memset(&rifConfig, 0, sizeof(rifConfig));

            if(NETCFG_POM_IP_GetNextRifConfig(&rifConfig) != NETCFG_TYPE_OK)
            {
                if (debug_mode)
                    printf("FUNCTION:SMTP_MGR_Send; DESC:NETCFG_POM_IP_GetNextRifConfig < 0\n");
                return SMTP_RETURN_FAIL;
            }
            else
            {
                UI32_T  temp_ip;
                memcpy (&temp_ip, rifConfig.addr.addr, sizeof(temp_ip));
                SMTP_UTIL_Print_IPAddr(Mailcmd, temp_ip);
            }
        }

        //SYSFUN_Sleep(5);
        if(send(smtp_mgr_socket_id, Mailcmd, strlen((char *)Mailcmd), 0) <= 0)
        {
            return SMTP_RETURN_FAIL;
        }
        //SYSFUN_Sleep(5);
        if (debug_mode)
            printf("FUNCTION:SMTP_MGR_Send; DESC:%s\n",Mailcmd);

        if (FALSE == SMTP_MGR_ProcessResponse(SMTP_MGR_STATE_INIT, &rescode))
        {
            return SMTP_RETURN_FAIL;
        }
    }
    else
    {
        rescode = SMTP_MGR_MAIL_ACTION_OK;
    }

    temp_rescode = rescode;

    for(j=0;j<server_num;j++)
    {
        nowstate = SMTP_MGR_STATE_HELLO;

        while(nowstate != SMTP_MGR_STATE_QUIT)
        {
            if (debug_mode)
                printf("FUNCTION:SMTP_MGR_Send; DESC:state is %d\n",nowstate);
            switch (nowstate)
            {
            case SMTP_MGR_STATE_HELLO:
                /* check the response from server */
                //rescode = temp_rescode;
                if(rescode == SMTP_MGR_SERVICE_READY)
                {
                    break;
                }

                //if (rescode == SMTP_MGR_MAIL_ACTION_OK || rescode == SMTP_MGR_SERVICE_READY)
                if (rescode == SMTP_MGR_MAIL_ACTION_OK)
                {
                    /* send information to server */
                    SMTP_UTIL_Print_MailFrom(Mailcmd,sourceaddr);
                    if(send(smtp_mgr_socket_id, Mailcmd, strlen((char *)Mailcmd), 0) <= 0)
                    {
                        strcpy((char *)Mailcmd,"RSET\r\n");
                        send(smtp_mgr_socket_id, Mailcmd, strlen((char *)Mailcmd), 0);
                        nowstate = SMTP_MGR_STATE_QUIT;
                        flag++;
                        break;
                    }

                    if (debug_mode)
                        printf("FUNCTION:SMTP_MGR_Send; DESC:%s\n",Mailcmd);

                    nowstate = SMTP_MGR_STATE_MAILFROM;
                    rescode = 0;
                }
                else
                {
                    if (debug_mode)
                        printf("FUNCTION:SMTP_MGR_Send; DESC:return code fail sta-%d\n", nowstate);
                    strcpy((char *)Mailcmd,"RSET\r\n");
                    send(smtp_mgr_socket_id, Mailcmd, strlen((char *)Mailcmd), 0);
                    nowstate = SMTP_MGR_STATE_QUIT;
                    flag++;
                    break;
                }
                break;

            case SMTP_MGR_STATE_MAILFROM:
                /* check the response from server */
                if (rescode == SMTP_MGR_MAIL_ACTION_OK)
                {
                    /* send information to server */
                    SMTP_UTIL_Print_RcptTo(Mailcmd,destinationaddr[j]);

                    if(send(smtp_mgr_socket_id, Mailcmd, strlen((char *)Mailcmd), 0) <= 0)
                    {
                        strcpy((char *)Mailcmd,"RSET\r\n");
                        send(smtp_mgr_socket_id, Mailcmd, strlen((char *)Mailcmd), 0);
                        nowstate = SMTP_MGR_STATE_QUIT;
                        flag++;
                        break;
                    }

                    if (debug_mode)
                        printf("FUNCTION:SMTP_MGR_Send; DESC:%s\n",Mailcmd);
                    nowstate = SMTP_MGR_STATE_RCPTTO;
                    rescode = 0;
                }
                else
                {
                    if (debug_mode)
                        printf("FUNCTION:SMTP_MGR_Send; DESC:return code fail sta-%d\n", nowstate);
                    strcpy((char *)Mailcmd,"RSET\r\n");
                    send(smtp_mgr_socket_id, Mailcmd, strlen((char *)Mailcmd), 0);
                    nowstate = SMTP_MGR_STATE_QUIT;
                    flag++;
                    break;
                }

                break;

            case SMTP_MGR_STATE_RCPTTO:
                /* check the response from server */
                if (rescode == SMTP_MGR_MAIL_ACTION_OK || rescode == SMTP_MGR_RCPT_NOT_LOCAL)
                {
                    /* send information to server */
                    strcpy((char *)Mailcmd,"DATA\r\n");

                    if(send(smtp_mgr_socket_id, Mailcmd, strlen((char *)Mailcmd), 0) <= 0)
                    {
                        strcpy((char *)Mailcmd,"RSET\r\n");
                        send(smtp_mgr_socket_id, Mailcmd, strlen((char *)Mailcmd), 0);
                        nowstate = SMTP_MGR_STATE_QUIT;
                        flag++;
                        break;
                    }

                    if (debug_mode)
                        printf("FUNCTION:SMTP_MGR_Send; DESC:%s\n",Mailcmd);
                    nowstate = SMTP_MGR_STATE_DATA;
                    rescode = 0;
                }
                else
                {
                    if (debug_mode)
                        printf("FUNCTION:SMTP_MGR_Send; DESC:return code fail sta-%d\n", nowstate);
                    strcpy((char *)Mailcmd,"RSET\r\n");
                    send(smtp_mgr_socket_id, Mailcmd, strlen((char *)Mailcmd), 0);
                    nowstate = SMTP_MGR_STATE_QUIT;
                    flag++;
                    break;
                }
                break;

            case SMTP_MGR_STATE_DATA:
                /* check the response from server */
                if (rescode == SMTP_MGR_START_MAIL_INPUT)
                {
                    if (debug_mode)
                        printf("FUNCTION:SMTP_MGR_Send; DESC:start to send mail content\n");
                    /* send information to server */
                    SMTP_UTIL_Print_MailSubject(Mailcmd, sourceaddr, destinationaddr[j]);

                       if(send(smtp_mgr_socket_id, Mailcmd, strlen((char *)Mailcmd), 0) <= 0)
                    {
                        strcpy((char *)Mailcmd,"RSET\r\n");
                        send(smtp_mgr_socket_id, Mailcmd, strlen((char *)Mailcmd), 0);
                        nowstate = SMTP_MGR_STATE_QUIT;
                        flag++;
                        break;
                    }
                    //SYSFUN_Sleep(5);
                    if (debug_mode)
                        printf("FUNCTION:SMTP_MGR_Send; DESC:%s\n",Mailcmd);
                    nowstate = SMTP_MGR_STATE_CONTENT;

                    SYS_TIME_GetRealTimeClock(&year, &month, &day, &hour, &minute, &second);
                    SMTP_UTIL_Print_Date(Mailcmd,(const UI8_T *)MONTH_MAPPING_STR[month-1],day,hour,minute,second);

                    strcat((char *)Mailcmd,"Message: ");
                    strcat((char *)Mailcmd, (char *)patten);

                    if(send(smtp_mgr_socket_id, Mailcmd, strlen((char *)Mailcmd), 0) <= 0)
                    {
                        strcpy((char *)Mailcmd,"RSET\r\n");
                        send(smtp_mgr_socket_id, Mailcmd, strlen((char *)Mailcmd), 0);
                        nowstate = SMTP_MGR_STATE_QUIT;
                        flag++;
                        break;
                    }
                    //SYSFUN_Sleep(5);
                    if (debug_mode)
                        printf("FUNCTION:SMTP_MGR_Send; DESC:%s\n",Mailcmd);
                    nowstate = SMTP_MGR_STATE_FINISH;

                    /* send information to server */
                    strcpy((char *)Mailcmd ,"\r\n.\r\n");

                    if(send(smtp_mgr_socket_id, Mailcmd, strlen((char *)Mailcmd), 0) <= 0)
                    {
                        strcpy((char *)Mailcmd,"RSET\r\n");
                        send(smtp_mgr_socket_id, Mailcmd, strlen((char *)Mailcmd), 0);
                        nowstate = SMTP_MGR_STATE_QUIT;
                        flag++;
                        break;
                    }

                    if (debug_mode)
                        printf("FUNCTION:SMTP_MGR_Send; DESC:%s\n",Mailcmd);

                    nowstate = SMTP_MGR_STATE_QUIT;
                    break;
                }
                else
                {
                    if (debug_mode)
                        printf("FUNCTION:SMTP_MGR_Send; DESC:return code fail sta-%d\n", nowstate);
                    strcpy((char *)Mailcmd,"RSET\r\n");
                    send(smtp_mgr_socket_id, Mailcmd, strlen((char *)Mailcmd), 0);
                    nowstate = SMTP_MGR_STATE_QUIT;
                    flag++;
                    break;
                }
                break;

            case SMTP_MGR_STATE_QUIT:
            default:
                break;
            }

            //SYSFUN_Sleep(5);
            /* receive the message replied from server */

            if (FALSE == SMTP_MGR_ProcessResponse(nowstate, &rescode))
            {
                strcpy((char *)Mailcmd,"RSET\r\n");
                send(smtp_mgr_socket_id, Mailcmd, strlen((char *)Mailcmd), 0);
                nowstate = SMTP_MGR_STATE_QUIT;
                flag++;
                rescode = SMTP_MGR_MAIL_ACTION_OK;
            }
        }
    }

    if(flag >= server_num)
    {
        return SMTP_RETURN_FAIL;
    }

    return  SMTP_RETURN_SUCCESS;
}/* End of SMTP_MGR_Send() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME: SMTP_MGR_Valid_IP
 *------------------------------------------------------------------------------
 * PURPOSE: This function is used to check if ip address is valid.
 * INPUT:   ip_addr -- ip address.
 * OUTPUT:  None.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL    -- fail
 * NOTE:    None.
 *------------------------------------------------------------------------------
 */
static UI32_T 
SMTP_MGR_Valid_IP(
    UI32_T ip_addr)
{
    if (IP_LIB_OK != IP_LIB_IsValidForRemoteIp((UI8_T *)&ip_addr))
    {
        return SMTP_RETURN_FAIL;
    }

    return SMTP_RETURN_SUCCESS;
}/* End of SMTP_MGR_Valid_IP() */

/* spc_email_isvalid is from "Recipe 3.9: Validating Email Addresses" of
 * "Secure Programming Cookbook for C and C++"
 *
 * RFC 822 defines the syntax for email addresses. Unfortunately, the syntax is
 * complex, and it supports several address formats that are no longer relevant.
 * The fortunate thing is that if anyone attempts to use one of these
 * no-longer-relevant address formats, you can be reasonably certain they are
 * attempting to do something they are not supposed to do.
 *
 * You can use the following spc_email_isvalid( ) function to check the format
 * of an email address. It will perform only a syntactical check and will not
 * actually attempt to verify the authenticity of the address by attempting to
 * deliver mail to it or by performing any DNS lookups on the domain name
 * portion of the address.
 *
 * The function only validates the actual email address and will not accept any
 * associated data. For example, it will fail to validate
 * "Bob Bobson <bob@bobson.com>", but it will successfully validate "bob@bobson.com".
 * If the supplied email address is syntactically valid, spc_email_isvalid( )
 * will return 1; otherwise, it will return 0.
 */
static int spc_email_isvalid(const char *address)
{
    int        count = 0;
    const char *c, *domain;
    static char *rfc822_specials = "()<>@,;:\\\"[]";

    /* first we validate the name portion (name@domain) */
    for (c = address;  *c;  c++) {
        if (*c == '\"' && (c == address || *(c - 1) == '.' || *(c - 1) ==
                           '\"')) {
            while (*++c) {
                if (*c == '\"') break;
                if (*c == '\\' && (*++c == ' ')) continue;
                if (*c <= ' ' || *c >= 127) return 0;
            }
            if (!*c++) return 0;
            if (*c == '@') break;
            if (*c != '.') return 0;
            continue;
        }
        if (*c == '@') break;
        if (*c <= ' ' || *c >= 127) return 0;
        if (strchr(rfc822_specials, *c)) return 0;
    }
    if (c == address || *(c - 1) == '.') return 0;

    /* next we validate the domain portion (name@domain) */
    if (!*(domain = ++c)) return 0;
    do {
        if (*c == '.') {
            if (c == domain || *(c - 1) == '.') return 0;
            count++;
        }
        if (*c <= ' ' || *c >= 127) return 0;
        if (strchr(rfc822_specials, *c)) return 0;
    } while (*++c);

    /* deny "bob@gmail.com."
     */
    if (*(c - 1) == '.') return 0;

    return (count >= 1);
}

/* FUNCTION NAME: SMTP_MGR_ValidateEmailAddress
 * PURPOSE: Validates E-mail address
 * INPUT  : address -- E-mail address
 * OUTPUT : None.
 * RETUEN : SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_FAIL       -- fail
 * NOTE   :
 */
static UI32_T SMTP_MGR_ValidateEmailAddress(const char *address)
{
    if((strlen(address) > SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS) ||
       (strlen(address) <= 0))
    {
        return SMTP_RETURN_FAIL;
    }

    return spc_email_isvalid(address) == 1 ? SMTP_RETURN_SUCCESS : SMTP_RETURN_FAIL;
}


/* FUNCTION NAME: SMTP_MGR_Connect
 * PURPOSE: This function is used to create socket and connect.
 * INPUT:   server_ip -- server ip address
 *          *socket_id -- socket id.
 * OUTPUT:  *socket_id -- socket id.
 * RETUEN:  SMTP_RETURN_SUCCESS    -- success
 *          SMTP_RETURN_CREATE_SOCKET_FAIL   -- create socket fail
 *          SMTP_RETURN_BIND_SOCKET_FAIL -- bind socket fail
 * NOTES:   1. smtp TCP protocol is SMTP_ADPT_IPPROTO_TCP
 *          2. smtp TCP port is SMTP_ADPT_SMTP_PORT
 *
 */
static UI32_T SMTP_MGR_Connect(UI32_T server_ip,I16_T *socket_id)
{
    struct sockaddr_in PeerAddress;

    *socket_id = socket(AF_INET, SOCK_STREAM, 0);

    if(*socket_id < 0)
    {
        return SMTP_RETURN_CREATE_SOCKET_FAIL;
    }

    /* connect to server */
    PeerAddress.sin_family      = AF_INET;
    PeerAddress.sin_addr.s_addr = server_ip;
    PeerAddress.sin_port        = L_STDLIB_Hton16(SMTP_ADPT_SMTP_PORT);

    if(connect(*socket_id, (struct sockaddr* )(&PeerAddress),
        sizeof(PeerAddress)) != 0)
    {
        SMTP_MGR_Close(*socket_id);
        return SMTP_RETURN_BIND_SOCKET_FAIL;
    }

    if (debug_mode)
            printf("FUNCTION:SMTP_MGR_Connect; DESC:connect success\n");
    return SMTP_RETURN_SUCCESS;
}/* End of SMTP_MGR_Connect() */

/* FUNCTION NAME: SMTP_MGR_Close
 * PURPOSE: This function is used to close socket.
 * INPUT:   socket_id -- socket id.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
static void SMTP_MGR_Close(I16_T socket_id)
{
    close(socket_id);
    return;
}/* End of SMTP_MGR_Close() */

/* FUNCTION NAME: SMTP_MGR_LogToMsg
 * PURPOSE: This function is used to change smtp entry to message patten.
 * INPUT:   *smtp_data -- smtp event data.
 *          *patten -- message patten.
 * OUTPUT:  *patten -- message patten.
 * RETUEN:  None.
 * NOTES:   1. Following message should be displayed in smtp mail
 *             1) Level
 *             2) Module
 *             3) Function
 *             4) Error
 *             5) Time
 *             6) Host
 *             7) Content
 *
 */
static void SMTP_MGR_LogToMsg(SMTP_OM_QueueRecord_T *smtp_data,UI8_T *patten)
{
    int     year, month, day, hour, minute, second;
    UI8_T   temp[SMTP_MGR_MAX_LENGTH_OF_MSG_PATTERN+1] = {0};
    UI8_T   Name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN + 1] = {0};
    UI8_T   mod_name [L_MNAME_MAX_NAME_LEN+1]          = {0};

    strcpy((char *)patten,"There is a log.\r\n");

    SMTP_UTIL_Print_MsgLevel(temp,smtp_data->smtp_entry.owner_info.level);
    strcat((char *)patten,(char *)temp);

    L_MNAME_GetModuleName(smtp_data->smtp_entry.owner_info.module_no, mod_name);
    SMTP_UTIL_Print_MsgModule(temp, mod_name);
    strcat((char *)patten,(char *)temp);

    SMTP_UTIL_Print_MsgFunction(temp,smtp_data->smtp_entry.owner_info.function_no);
    strcat((char *)patten,(char *)temp);

    SMTP_UTIL_Print_MsgError(temp,smtp_data->smtp_entry.owner_info.error_no);
    strcat((char *)patten,(char *)temp);

    SYS_TIME_ConvertSecondsToDateTime(smtp_data->smtp_entry.log_time, &year,
            &month, &day, &hour,
            &minute, &second);

    SMTP_UTIL_Print_MsgTime(temp,(const UI8_T *)MONTH_MAPPING_STR[month-1],day,hour,minute,second);
    strcat((char *)patten,(char *)temp);

    MIB2_POM_GetSysName(Name);
    if(strlen((char *)Name) != 0)
    {
        SMTP_UTIL_Print_MsgSysName(temp,Name);
        strcat((char *)patten,(char *)temp);
    }
    else
    {
        NETCFG_TYPE_InetRifConfig_T rif_config;
        UI32_T mgmt_vid_ifIndex = 0, temp_ip;

        IML_PMGR_GetManagementVid(&mgmt_vid_ifIndex);

        memset(&rif_config, 0, sizeof(rif_config));
        rif_config.ifindex = mgmt_vid_ifIndex;
        rif_config.addr.type = 1;// L_INET_ADDR_TYPE_TPV4;

        if (NETCFG_POM_IP_GetNextRifFromInterface(&rif_config) == NETCFG_TYPE_OK)
        {
            memcpy (&temp_ip, rif_config.addr.addr, sizeof(temp_ip));
            SMTP_UTIL_Print_MsgIPAddr(temp, temp_ip);
            strcat((char *)patten,(char *)temp);
        }

#if 0
        /* get IP address */
        memset(&rifNode_p, 0, sizeof(NETIF_OM_RifNode_T));
        rifNode_p.vid_ifIndex = 0;
        rifNode_p.ipAddress = 0;
        rifNode_p.ipMask = 0;
        if(NETCFG_GetNextRifInfo(&rifNode_p) < 0)
        {
            if (debug_mode)
                printf("FUNCTION:SMTP_MGR_LogToMsg; DESC:NETCFG_GetNextRifInfo < 0\n");
            return;
        }
        else
        {
            SMTP_UTIL_Print_MsgIPAddr(temp,rifNode_p.ipAddress);
            strcat((char *)patten,(char *)temp);
        }
#endif
    }
    SMTP_UTIL_Print_MsgContent(temp,smtp_data->smtp_entry.message);
    strcat((char *)patten,(char *)temp);

    if (debug_mode)
        printf("FUNCTION:SMTP_MGR_LogToMsg; DESC:log message success\n");

    return;
}/* End of SMTP_MGR_LogToMsg() */

/* FUNCTION NAME: SMTP_MGR_BackdoorInfo_CallBack
 * PURPOSE: This function is used to do the smtp backdoor function.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
static  void   SMTP_MGR_BackdoorInfo_CallBack(void)
{
    debug_mode = ! debug_mode;

    if (TRUE == debug_mode)
    {
        printf("\n SMTP Debug mode is enabled\n");
    }
    else
    {
        printf("\n SMTP Debug mode is disabled\n");
    }


#if 0
    int                     ch;
    BOOL_T                  backdoor_continue;

    printf("\n SMTP Backdoor Selection");

    backdoor_continue = TRUE;

    /* backdoor_mgr & smtp are under the same thread,
     *   so can not call backdoor_pmgr ...
     */
    while(backdoor_continue)
    {
        SMTP_MGR_Print_BackdoorHelp();


        ch = BACKDOOR_MGR_GetChar();

        switch(ch)
        {
            case 'Y':
            case 'y':
                debug_mode=TRUE;
                printf("\n Debug mode is enabled\n");
                break;
            case 'N':
            case 'n':
                debug_mode=FALSE;
                printf("\n Debug mode is disable\n");
                break;
            case 'Q':
            case 'q':
                backdoor_continue = FALSE;
                break;
            default:
                printf("\n Invalid command\n");
                break;
        }
    }
#endif

}/* End of SMTP_MGR_BackdoorInfo_CallBack() */

/* FUNCTION NAME: SMTP_MGR_Print_BackdoorHelp
 * PURPOSE: This function is used to print the smtp backdoor select item.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */

#if 0
static void SMTP_MGR_Print_BackdoorHelp(void)
{
    printf("\n\tEnable Debug Mode:Y");
    printf("\n\tDisable Debug Mode:N");
    printf("\n\tQuit:Q");
    printf("\n");
}/* End of SMTP_MGR_Print_BackdoorHelp() */
#endif

/* FUNCTION NAME: SMTP_MGR_IsLocalIP
 * PURPOSE: This function is used to check if server IP is local.
 * INPUT:   ip -- server IP.
 * OUTPUT:  None.
 * RETUEN:  TRUE/FALSE.
 * NOTES:   None.
 *
 */
static BOOL_T SMTP_MGR_IsLocalIP(UI32_T ip)
{
    NETCFG_TYPE_InetRifConfig_T rif_config;
    UI32_T                      mgmt_vid_ifIndex = 0;

    IML_PMGR_GetManagementVid(&mgmt_vid_ifIndex);

    memset(&rif_config, 0, sizeof(rif_config));
    rif_config.ifindex = mgmt_vid_ifIndex;
    rif_config.addr.type = 1;// L_INET_ADDR_TYPE_IPV4;

    if (NETCFG_POM_IP_GetNextRifFromInterface(&rif_config) == NETCFG_TYPE_OK)
    {
        if (memcmp (&ip, rif_config.addr.addr, sizeof(ip)) == 0)
            return TRUE;
    }

    return FALSE;
}/* End of SMTP_MGR_IsLocalIP() */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - SMTP_MGR_HandleHotInsertion
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 * ------------------------------------------------------------------------
 */
void SMTP_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* no port database, do nothing */
    return;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - SMTP_MGR_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void SMTP_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /* no port database, do nothing */
    return;
}
