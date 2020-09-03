/*-----------------------------------------------------------------------------
 * Module   : smtp_pmgr.c
 *-----------------------------------------------------------------------------
 * PURPOSE  : Provide APIs to access SMTP control functions.
 *-----------------------------------------------------------------------------
 * NOTES    :
 *
 *-----------------------------------------------------------------------------
 * HISTORY  : 11/07/2007 - Squid Ro, Create
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
#include "smtp_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void SMTP_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val);

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

/* MACRO FUNCTIONS DECLARACTION
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SMTP_PMGR_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE : Initiate resource used in the calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T SMTP_PMGR_InitiateProcessResources(void)
{
    if (SYSFUN_GetMsgQ(SMTP_MGR_IPCMSG_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ fail.", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SMTP_PMGR_AddSmtpDestinationEmailAddr
 *-------------------------------------------------------------------------
 * PURPOSE : This function is used to add the smtp destination email address.
 * INPUT   : *email_addr -- smtp destination email address.
 * OUTPUT  : None
 * RETURN  : SMTP_RETURN_SUCCESS        -- success
 *           SMTP_RETURN_INVALID        -- invalid input value
 *           SMTP_RETURN_INPUT_EXIST    -- input value already exist
 *           SMTP_RETURN_DATABASE_FULL  -- database full
 * NOTES   : 1. Add one destination email address one time
 *           2. Max number of destination email address is SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS.
 *           3. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *-------------------------------------------------------------------------
 */
UI32_T SMTP_PMGR_AddSmtpDestinationEmailAddr(UI8_T *email_addr)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SMTP_MGR_IPCMsg_EmailAddr_T     *data_p;

    data_p = SMTP_MGR_MSG_DATA(msg_p);

    memcpy (data_p->email_addr, email_addr, sizeof (data_p->email_addr));

    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_ADD_SMTP_DST_EMAIL_ADDR,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T),
                      SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SMTP_RETURN_FAIL);

    return SMTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_AddSmtpServerIPAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to add the smtp server ip address.
 * INPUT    : ip_addr -- smtp server ip address.
 * OUTPUT	: None
 * RETURN   : SMTP_RETURN_SUCCESS    -- success
 *            SMTP_RETURN_FAIL    -- fail
 *            SMTP_RETURN_INVALID  -- invalid input value
 *            SMTP_RETURN_INPUT_EXIST -- input exist
 *            SMTP_RETURN_DATABASE_FULL -- database full
 * NOTES	: 1. add one server ip address one time.
 *            2. Max number of smtp server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_AddSmtpServerIPAddr(UI32_T ip_addr)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_IpAddr_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SMTP_MGR_IPCMsg_IpAddr_T    *data_p;

    data_p = SMTP_MGR_MSG_DATA(msg_p);
    data_p->ip_addr = ip_addr;

    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_ADD_SMTP_SERVER_IP,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_IpAddr_T),
                      SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SMTP_RETURN_FAIL);

    return SMTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_DisableSmtpAdminStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to disable smtp admin status.
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : SMTP_RETURN_SUCCESS -- success
 *            SMTP_RETURN_FAIL    -- fail
 * NOTES	: 1. status is defined as following:
 *               SMTP_STATUS_ENABLE/SMTP_STATUS_DISABLE
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_DisableSmtpAdminStatus(void)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;

    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_DISABLE_ADMIN_STATUS,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SMTP_RETURN_FAIL);

    return SMTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_DeleteSmtpDestinationEmailAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to delete the smtp destination email address.
 * INPUT    : *email_addr -- smtp destination email address.
 * OUTPUT	: None
 * RETURN   : SMTP_RETURN_SUCCESS  -- success
 *            SMTP_RETURN_FAIL     -- fail
 *            SMTP_RETURN_INVALID  -- invalid input value
 * NOTES	: 1. delete one destination email address one time.
 *            2. Max number of destination email address is
 *               SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS.
 *            3. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_DeleteSmtpDestinationEmailAddr(UI8_T *email_addr)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SMTP_MGR_IPCMsg_EmailAddr_T *data_p;

    data_p = SMTP_MGR_MSG_DATA(msg_p);
    memcpy (data_p->email_addr, email_addr, sizeof (data_p->email_addr));

    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_DEL_SMTP_DST_EMAIL_ADDR,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T),
                      SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SMTP_RETURN_FAIL);

    return SMTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_DeleteSmtpServerIPAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to delete the smtp server ip address.
 * INPUT    : ip_addr -- smtp server ip address.
 * OUTPUT	: None
 * RETURN   : SMTP_RETURN_SUCCESS    -- success
 *            SMTP_RETURN_FAIL    -- fail
 *            SMTP_RETURN_INVALID  -- invalid input value
 *            SMTP_RETURN_INPUT_NOT_EXIST -- input not exist
 * NOTES	: 1. Max number of smtp server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *            2. Delete one server ip address one time
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_DeleteSmtpServerIPAddr(UI32_T ip_addr)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_IpAddr_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SMTP_MGR_IPCMsg_IpAddr_T    *data_p;

    data_p = SMTP_MGR_MSG_DATA(msg_p);
    data_p->ip_addr = ip_addr;

    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_DEL_SMTP_SERVER_IP,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_IpAddr_T),
                      SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SMTP_RETURN_FAIL);

    return SMTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_EnableSmtpAdminStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to enable smtp admin status.
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : SMTP_RETURN_SUCCESS -- success
 *            SMTP_RETURN_FAIL    -- fail
 * NOTE		: 1. status is defined as following:
 *               SMTP_STATUS_ENABLE/SMTP_STATUS_DISABLE
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_EnableSmtpAdminStatus(void)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;

    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_ENABLE_ADMIN_STATUS,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SMTP_RETURN_FAIL);

    return SMTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetEmailSeverityLevel
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get smtp severity level.
 * INPUT    : None
 * OUTPUT	: *level -- smtp severity level.
 * RETURN   : SMTP_RETURN_SUCCESS        -- success
 *            SMTP_RETURN_FAIL           -- fail
 *            SMTP_RETURN_INVALID_BUFFER -- invalid input buffer
 * NOTES	: 1. smtp severity level symbol is defined as following:
 *              SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *              SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *              SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *              SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *              SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *              SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *              SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *              SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *           2. SYSLOG_LEVEL_EMERG is highest priority.
 *              SYSLOG_LEVEL_DEBUG is lowest priority.
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_GetEmailSeverityLevel(UI32_T *level)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_SerLevel_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SMTP_MGR_IPCMsg_SerLevel_T  *data_p;

    data_p = SMTP_MGR_MSG_DATA(msg_p);

    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_GET_EMAIL_SER_LEVEL,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_SerLevel_T),
                      SMTP_RETURN_FAIL);

    if (SMTP_RETURN_SUCCESS == SMTP_MGR_MSG_RETVAL(msg_p))
    {
        *level = data_p ->ser_level;
    }
    return SMTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetNextRunningSmtpDestinationEmailAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get next smtp destination email address.
 * INPUT    : *email_addr -- smtp destination email address.
 * OUTPUT	: *email_addr -- smtp destination email address.
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES	:  1. Max number of destination email address is
                  SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS.
 *             2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SMTP_PMGR_GetNextRunningSmtpDestinationEmailAddr(UI8_T *email_addr)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SMTP_MGR_IPCMsg_EmailAddr_T *data_p;

    data_p = SMTP_MGR_MSG_DATA(msg_p);
    memcpy (data_p->email_addr, email_addr, sizeof (data_p->email_addr));

    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_GET_NEXT_RUNN_SMTP_DST_EML_ADR,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T),
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T),
                      SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == SMTP_MGR_MSG_RETVAL(msg_p))
    {
        memcpy (email_addr, data_p->email_addr, sizeof (data_p->email_addr));
    }

    return SMTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetNextRunningSmtpServerIPAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get next running smtp server ip address.
 * INPUT    : None
 * OUTPUT	: *ip_addr -- smtp server ip address.
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES	: 1. Max number of server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *            2. get one server ip address one time
 *            3. use 0 to get first running ip address
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SMTP_PMGR_GetNextRunningSmtpServerIPAddr(UI32_T *ip_addr)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_IpAddr_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SMTP_MGR_IPCMsg_IpAddr_T    *data_p;

    data_p = SMTP_MGR_MSG_DATA(msg_p);
    data_p ->ip_addr = *ip_addr;

    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_GET_NEXT_RUNN_SMTP_SVR_IP,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_IpAddr_T),
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_IpAddr_T),
                      SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == SMTP_MGR_MSG_RETVAL(msg_p))
    {
        *ip_addr = data_p ->ip_addr;
    }

    return SMTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetNextSmtpDestinationEmailAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get next smtp destination email address.
 * INPUT    : *email_addr -- smtp destination email address.
 * OUTPUT	: *email_addr -- smtp destination email address.
 * RETURN   : SMTP_RETURN_SUCCESS          -- success
 *            SMTP_RETURN_FAIL             -- fail
 *            SMTP_RETURN_INVALID          -- invalid input value
 *            SMTP_RETURN_INVALID_BUFFER   -- invalid input buffer
 * NOTES	: 1. Max number of destination email address is
 *               SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS.
 *            2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_GetNextSmtpDestinationEmailAddr(UI8_T *email_addr)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SMTP_MGR_IPCMsg_EmailAddr_T *data_p;

    data_p = SMTP_MGR_MSG_DATA(msg_p);
    memcpy (data_p->email_addr, email_addr, sizeof (data_p->email_addr));

    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_GET_NEXT_SMTP_DES_EMAIL_ADDR,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T),
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T),
                      SMTP_RETURN_FAIL);

    if (SMTP_RETURN_SUCCESS == SMTP_MGR_MSG_RETVAL(msg_p))
    {
        memcpy (email_addr, data_p->email_addr, sizeof (data_p->email_addr));
    }

    return SMTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetNextSmtpServerIPAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get next smtp server ip address.
 * INPUT    : *ip_addr -- smtp server ip address.
 * OUTPUT	: *ip_addr -- smtp server ip address.
 * RETURN   : SMTP_RETURN_SUCCESS       -- success
 *            SMTP_RETURN_FAIL          -- fail
 *            SMTP_RETURN_INVALID       -- invalid input value
 *            SMTP_RETURN_INVALID_BUFFER-- invalid input buffer
 * NOTES	: 1. Max number of server ip address is SYS_ADPT_MAX_NUM_OF_SMTP_SERVER.
 *            2. get one server ip address one time
 *            3. use 0 to get first ip address
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_GetNextSmtpServerIPAddr(UI32_T *ip_addr)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_IpAddr_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SMTP_MGR_IPCMsg_IpAddr_T    *data_p;

    data_p = SMTP_MGR_MSG_DATA(msg_p);
    data_p->ip_addr = *ip_addr;

    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_GET_NEXT_SMTP_SERVER_IP,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_IpAddr_T),
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_IpAddr_T),
                      SMTP_RETURN_FAIL);

    if (SMTP_RETURN_SUCCESS == SMTP_MGR_MSG_RETVAL(msg_p))
    {
        *ip_addr = data_p->ip_addr;
    }

    return SMTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetRunningSmtpAdminStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get running smtp status.
 * INPUT    : None
 * OUTPUT	: *status -- smtp status.
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES:     1. status is defined as following:
 *               SMTP_STATUS_ENABLE/SMTP_STATUS_DISABLE
 *            2. default status is SYS_DFLT_SMTP_ADMIN_STATUS
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SMTP_PMGR_GetRunningSmtpAdminStatus(UI32_T *status)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_Status_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SMTP_MGR_IPCMsg_Status_T    *data_p;

    data_p = SMTP_MGR_MSG_DATA(msg_p);

    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_GET_RUNN_SMTP_ADMIN_STATUS,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_Status_T),
                      SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == SMTP_MGR_MSG_RETVAL(msg_p))
    {
        *status = data_p ->status;
    }

    return SMTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetRunningEmailSeverityLevel
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get running smtp severity level.
 * INPUT    : None
 * OUTPUT	: *level -- smtp severity level.
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES	:  1. smtp severity level symbol is defined as following:
 *              SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *              SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *              SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *              SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *              SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *              SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *              SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *              SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *           2. SYSLOG_LEVEL_EMERG is highest priority.
 *              SYSLOG_LEVEL_DEBUG is lowest priority.
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SMTP_PMGR_GetRunningEmailSeverityLevel(UI32_T *level)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_SerLevel_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SMTP_MGR_IPCMsg_SerLevel_T  *data_p;

    data_p = SMTP_MGR_MSG_DATA(msg_p);

    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_GET_RUNN_EMAIL_SER_LVL,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_SerLevel_T),
                      SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == SMTP_MGR_MSG_RETVAL(msg_p))
    {
        *level = data_p ->ser_level;
    }
    return SMTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetRunningSmtpSourceEmailAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get the running smtp source email address.
 * INPUT    : None
 * OUTPUT	: *email_addr -- smtp source email address.
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS    -- not same as default
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL       -- get failure
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  -- same as default
 * NOTES	: 1. There is only one source email address.
 *            2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SMTP_PMGR_GetRunningSmtpSourceEmailAddr(UI8_T *email_addr)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SMTP_MGR_IPCMsg_EmailAddr_T *data_p;

    data_p = SMTP_MGR_MSG_DATA(msg_p);

    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_GET_RUNN_SMTP_SRC_EML_ADR,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T),
                      SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == SMTP_MGR_MSG_RETVAL(msg_p))
    {
        memcpy (email_addr, data_p->email_addr, sizeof (data_p->email_addr));
    }

    return SMTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetSmtpAdminStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get smtp admin status.
 * INPUT    : None
 * OUTPUT	: *status -- smtp status.
 * RETURN   : SMTP_RETURN_SUCCESS           -- success
 *            SMTP_RETURN_FAIL              -- fail
 *            SMTP_RETURN_INVALID_BUFFER    -- invalid input buffer
 * NOTES	: 1. status is defined as following:
 *               SMTP_STATUS_ENABLE/ SMTP_STATUS_DISABLE
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_GetSmtpAdminStatus(UI32_T *status)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_Status_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SMTP_MGR_IPCMsg_Status_T    *data_p;

    data_p = SMTP_MGR_MSG_DATA(msg_p);

    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_GET_SMTP_ADMIN_STATUS,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_Status_T),
                      SMTP_RETURN_FAIL);

    if (SMTP_RETURN_SUCCESS == SMTP_MGR_MSG_RETVAL(msg_p))
    {
        *status = data_p ->status;
    }

    return SMTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetSmtpSourceEmailAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get the smtp source email address.
 * INPUT    : None
 * OUTPUT	: *email_addr -- smtp source email address.
 * RETURN   : SMTP_RETURN_SUCCESS           -- success
 *            SMTP_RETURN_FAIL              -- fail
 *            SMTP_RETURN_INVALID_BUFFER    -- invalid input buffer
 * NOTES	: 1. There is only one source email address
 *            2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_GetSmtpSourceEmailAddr(UI8_T *email_addr)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SMTP_MGR_IPCMsg_EmailAddr_T *data_p;

    data_p = SMTP_MGR_MSG_DATA(msg_p);

    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_GET_SMTP_SRC_EMAIL_ADDR,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T),
                      SMTP_RETURN_FAIL);

    if (SMTP_RETURN_SUCCESS == SMTP_MGR_MSG_RETVAL(msg_p))
    {
        memcpy (email_addr, data_p->email_addr, sizeof (data_p->email_addr));
    }

    return SMTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_SetEmailSeverityLevel
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to set smtp severity level.
 * INPUT    : level -- smtp seveirity level.
 * OUTPUT	: None
 * RETURN   : SMTP_RETURN_SUCCESS  -- success
 *            SMTP_RETURN_FAIL     -- fail
 *            SMTP_RETURN_INVALID  -- invalid input value
 * NOTES	: 1. smtp severity level symbol is defined as following:
 *               SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *               SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *               SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *               SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *               SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *               SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *               SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *               SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *            2. SYSLOG_LEVEL_EMERG is highest priority.
 *               SYSLOG_LEVEL_DEBUG is lowest priority.
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_SetEmailSeverityLevel(UI32_T level)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_SerLevel_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SMTP_MGR_IPCMsg_SerLevel_T  *data_p;

    data_p = SMTP_MGR_MSG_DATA(msg_p);
    data_p->ser_level = level;

    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_SET_EMAIL_SER_LEVEL,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_SerLevel_T),
                      SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SMTP_RETURN_FAIL);

    return SMTP_MGR_MSG_RETVAL(msg_p);
}

//ADD: daniel
/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_DeleteEmailSeverityLevel
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to delete smtp severity level to default value.
 * INPUT    : level -- smtp seveirity level.
 * OUTPUT	: None
 * RETURN   : SMTP_RETURN_SUCCESS  -- success
 *            SMTP_RETURN_FAIL     -- fail
 *            SMTP_RETURN_INVALID  -- invalid input value
 * NOTES	: 1. smtp severity level symbol is defined as following:
 *               SYSLOG_LEVEL_EMERG  = 0,    (System unusable                  )
 *               SYSLOG_LEVEL_ALERT  = 1,    (Immediate action needed          )
 *               SYSLOG_LEVEL_CRIT   = 2,    (Critical conditions              )
 *               SYSLOG_LEVEL_ERR    = 3,    (Error conditions                 )
 *               SYSLOG_LEVEL_WARNING= 4,    (Warning conditions               )
 *               SYSLOG_LEVEL_NOTICE = 5,    (Normal but significant condition )
 *               SYSLOG_LEVEL_INFO   = 6,    (Informational messages only      )
 *               SYSLOG_LEVEL_DEBUG  = 7     (Debugging messages               )
 *            2. SYSLOG_LEVEL_EMERG is highest priority.
 *               SYSLOG_LEVEL_DEBUG is lowest priority.
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_DeleteEmailSeverityLevel(UI32_T level)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_SerLevel_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SMTP_MGR_IPCMsg_SerLevel_T  *data_p;

    data_p = SMTP_MGR_MSG_DATA(msg_p);
    data_p->ser_level = level;

    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_DEL_EMAIL_SER_LEVEL,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_SerLevel_T),
                      SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SMTP_RETURN_FAIL);

    return SMTP_MGR_MSG_RETVAL(msg_p);
}
//end ADD. daniel

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SMTP_PMGR_SetSmtpSourceEmailAddr
 *-------------------------------------------------------------------------
 * PURPOSE : This function is used to set the smtp source email address.
 * INPUT   : *email_addr -- smtp source email address.
 * OUTPUT  : None
 * RETURN  : SMTP_RETURN_SUCCESS  -- success
 *           SMTP_RETURN_FAIL     -- fail
 *           SMTP_RETURN_INVALID  -- invalid input value
 * NOTES   : 1. There is only one source email address
 *           2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *-------------------------------------------------------------------------
 */
UI32_T SMTP_PMGR_SetSmtpSourceEmailAddr(UI8_T *email_addr)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SMTP_MGR_IPCMsg_EmailAddr_T     *data_p;

    data_p = SMTP_MGR_MSG_DATA(msg_p);

    memcpy (data_p->email_addr, email_addr, sizeof (data_p->email_addr));

    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_SET_SMTP_SRC_EMAIL_ADDR,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T),
                      SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SMTP_RETURN_FAIL);

    return SMTP_MGR_MSG_RETVAL(msg_p);
}


//ADD daniel 
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SMTP_PMGR_DeleteSmtpSourceEmailAddr
 *-------------------------------------------------------------------------
 * PURPOSE : This function is used to delete the smtp source email address.
 * INPUT   : *email_addr -- smtp source email address.
 * OUTPUT  : None
 * RETURN  : SMTP_RETURN_SUCCESS  -- success
 *           SMTP_RETURN_FAIL     -- fail
 *           SMTP_RETURN_INVALID  -- invalid input value
 * NOTES   : 1. There is only one source email address
 *           2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 *-------------------------------------------------------------------------
 */
UI32_T SMTP_PMGR_DeleteSmtpSourceEmailAddr(void)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SMTP_MGR_IPCMsg_EmailAddr_T     *data_p;

    data_p = SMTP_MGR_MSG_DATA(msg_p);


    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_DEL_SMTP_SRC_EMAIL_ADDR,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T),
                      SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SMTP_RETURN_FAIL);

    return SMTP_MGR_MSG_RETVAL(msg_p);
}
//end ADD daniel


/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetSmtpServerIPAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get smtp server ip address.
 * INPUT    : None
 * OUTPUT	: *ip_addr -- smtp server ip address.
 * RETURN   : SMTP_RETURN_SUCCESS           -- success
 *            SMTP_RETURN_FAIL              -- fail
 *            SMTP_RETURN_INVALID_BUFFER    -- invalid input buffer
 * NOTES	: 1. status is defined as following:
 *               SMTP_STATUS_ENABLE/ SMTP_STATUS_DISABLE
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_GetSmtpServerIPAddr(UI32_T *ip_addr)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_IpAddr_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SMTP_MGR_IPCMsg_IpAddr_T    *data_p;

    data_p = SMTP_MGR_MSG_DATA(msg_p);
    data_p->ip_addr = *ip_addr;

    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_GET_SMTP_SERVER_IP_ADDR,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_IpAddr_T),
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_IpAddr_T),
                      SMTP_RETURN_FAIL);

    if (SMTP_RETURN_SUCCESS == SMTP_MGR_MSG_RETVAL(msg_p))
    {
        *ip_addr = data_p->ip_addr;
    }

    return SMTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SMTP_PMGR_GetSmtpSourceEmailAddr
 * ------------------------------------------------------------------------|
 * FUNCTION : This function is used to get smtp destination email address.
 * INPUT    : None
 * OUTPUT	: *email_addr -- smtp destination email address.
 * RETURN   : SMTP_RETURN_SUCCESS           -- success
 *            SMTP_RETURN_FAIL              -- fail
 *            SMTP_RETURN_INVALID_BUFFER    -- invalid input buffer
 * NOTES	: 1. There is only one source email address
 *            2. Max length of email address is SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS
 * ------------------------------------------------------------------------*/
UI32_T SMTP_PMGR_GetSmtpDestinationEmailAddr(UI8_T *email_addr)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SMTP_MGR_IPCMsg_EmailAddr_T *data_p;

    data_p = SMTP_MGR_MSG_DATA(msg_p);

    SMTP_PMGR_SendMsg(SMTP_MGR_IPC_CMD_GET_SMTP_DST_EMAIL_ADDR,
                      msg_p,
                      SMTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SMTP_MGR_GET_MSGBUFSIZE(SMTP_MGR_IPCMsg_EmailAddr_T),
                      SMTP_RETURN_FAIL);

    if (SMTP_RETURN_SUCCESS == SMTP_MGR_MSG_RETVAL(msg_p))
    {
        memcpy (email_addr, data_p->email_addr, sizeof (data_p->email_addr));
    }

    return SMTP_MGR_MSG_RETVAL(msg_p);
}

/* LOCAL SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SMTP_PMGR_SendMsg
 *-------------------------------------------------------------------------
 * PURPOSE : To send an IPC message.
 * INPUT   : cmd       - the SMTP message command.
 *           msg_p     - the buffer of the IPC message.
 *           req_size  - the size of SMTP request message.
 *           res_size  - the size of SMTP response message.
 *           ret_val   - the return value to set when IPC is failed.
 * OUTPUT  : msg_p     - the response message.
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
static void SMTP_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val)
{
    UI32_T ret;

    msg_p->cmd = SYS_MODULE_SMTP;
    msg_p->msg_size = req_size;

    SMTP_MGR_MSG_CMD(msg_p) = cmd;

    ret = SYSFUN_SendRequestMsg(ipcmsgq_handle,
                                msg_p,
                                SYSFUN_TIMEOUT_WAIT_FOREVER,
                                SYSFUN_SYSTEM_EVENT_IPCMSG,
                                res_size,
                                msg_p);

    /* If IPC is failed, set return value as ret_val.
     */
    if (ret != SYSFUN_OK)
        SMTP_MGR_MSG_RETVAL(msg_p) = ret_val;
}

/* End of this file */

