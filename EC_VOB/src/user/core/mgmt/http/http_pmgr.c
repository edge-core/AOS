/* MODULE NAME:  http_pmgr.c
 * PURPOSE:
 *    PMGR implement for http.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    11/10/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "sys_type.h"
#include "sys_module.h"
#include "sysfun.h"
#include "http_mgr.h"
#include "http_pmgr.h"
#include "sys_bld.h"
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
static void HTTP_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val);
/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : HTTP_PMGR_InitiateProcessResource
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
BOOL_T HTTP_PMGR_InitiateProcessResource(void)
{
    if(SYSFUN_GetMsgQ(SYS_BLD_WEB_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

#if (SYS_CPNT_HTTP == TRUE)
/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Get_Running_Debug_Information_Status
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get running http debug info.
 * INPUT    : None
 * OUTPUT	: debug_type
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS / SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE		: none
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  HTTP_PMGR_Get_Running_Debug_Information_Status(HTTP_MGR_DebugType_T *debug_type)
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_DEBUG_Type_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    HTTP_MGR_IPCMsg_HTTP_DEBUG_Type_T    *data_p;

    data_p = HTTP_MGR_MSG_DATA(msg_p);
    HTTP_PMGR_SendMsg(HTTP_MGR_IPC_CMD_GET_RUNNING_HTTP_DEBUG_INFO,
                      msg_p,
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_DEBUG_Type_T),
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_DEBUG_Type_T),
                      (UI32_T)FALSE);

    *debug_type = data_p->http_debug;

    return HTTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Get_Running_Http_Port
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get running http port
 * INPUT    : None
 * OUTPUT	: port
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS / SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE		: none
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  HTTP_PMGR_Get_Running_Http_Port(UI32_T *port)
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T    *data_p;

    data_p = HTTP_MGR_MSG_DATA(msg_p);
    HTTP_PMGR_SendMsg(HTTP_MGR_IPC_CMD_GET_RUNNING_HTTP_PORT,
                      msg_p,
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T),
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T),
                      (UI32_T)SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *port = data_p->port;

    return HTTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Get_Running_Http_Status
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get running http port
 * INPUT    : None
 * OUTPUT	: status
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS / SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE		: none
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  HTTP_PMGR_Get_Running_Http_Status(UI32_T *status)
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_RUNNING_STATUS_Type_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    HTTP_MGR_IPCMsg_RUNNING_STATUS_Type_T    *data_p;

    data_p = HTTP_MGR_MSG_DATA(msg_p);
    HTTP_PMGR_SendMsg(HTTP_MGR_IPC_CMD_GET_RUNNING_HTTP_STATUS,
                      msg_p,
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_RUNNING_STATUS_Type_T),
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_RUNNING_STATUS_Type_T),
                      (UI32_T)SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *status = data_p->running_status;

    return HTTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Get_Http_Port
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get http port
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : port
 * NOTE		: none
 * ------------------------------------------------------------------------*/
UI32_T HTTP_PMGR_Get_Http_Port()
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T    *data_p;

    data_p = HTTP_MGR_MSG_DATA(msg_p);
    HTTP_PMGR_SendMsg(HTTP_MGR_IPC_CMD_GET_HTTP_PORT,
                      msg_p,
                      HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T),
                      (UI32_T)FALSE);

    return data_p->port;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Get_Http_Status
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get http status
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : port
 * NOTE		: none
 * ------------------------------------------------------------------------*/
HTTP_STATE_T HTTP_PMGR_Get_Http_Status()
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T    *data_p;

    data_p = HTTP_MGR_MSG_DATA(msg_p);
    HTTP_PMGR_SendMsg(HTTP_MGR_IPC_CMD_GET_HTTP_STATUS,
                      msg_p,
                      HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T),
                      (UI32_T)FALSE);

    return data_p->status;
}


/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Set_Http_Port
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set http port
 * INPUT    : port
 * OUTPUT	: None
 * RETURN   : port
 * NOTE		: none
 * ------------------------------------------------------------------------*/
BOOL_T HTTP_PMGR_Set_Http_Port (UI32_T port)
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T    *data_p;

    data_p = HTTP_MGR_MSG_DATA(msg_p);
    data_p->port = port;
    HTTP_PMGR_SendMsg(HTTP_MGR_IPC_CMD_SET_HTTP_PORT,
                      msg_p,
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T),
                      HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      FALSE);

    return (BOOL_T) HTTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Set_Http_Status
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set http status
 * INPUT    : state
 * OUTPUT	: None
 * RETURN   : port
 * NOTE		: none
 * ------------------------------------------------------------------------*/
BOOL_T HTTP_PMGR_Set_Http_Status (HTTP_STATE_T state)
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T    *data_p;

    data_p = HTTP_MGR_MSG_DATA(msg_p);
    data_p->status = state;
    HTTP_PMGR_SendMsg(HTTP_MGR_IPC_CMD_SET_HTTP_STATUS,
                      msg_p,
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T),
                      HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T) HTTP_MGR_MSG_RETVAL(msg_p);
}
#endif  /* #if (SYS_CPNT_HTTP == TRUE) */

#if (SYS_CPNT_HTTPS == TRUE)
/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Get_Running_Http_Secure_Port
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get running https port
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS / SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE		: none
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  HTTP_PMGR_Get_Running_Http_Secure_Port(UI32_T *port)
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T    *data_p;

    data_p = HTTP_MGR_MSG_DATA(msg_p);
    HTTP_PMGR_SendMsg(HTTP_MGR_IPC_CMD_GET_RUNNING_HTTP_SECURE_PORT,
                      msg_p,
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T),
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T),
                      (UI32_T)SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *port = data_p->port;

    return (BOOL_T) HTTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Get_Running_Secure_Http_Status
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get running https status
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS / SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE		: none
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  HTTP_PMGR_Get_Running_Secure_Http_Status(UI32_T *status)
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_RUNNING_STATUS_Type_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    HTTP_MGR_IPCMsg_RUNNING_STATUS_Type_T    *data_p;

    data_p = HTTP_MGR_MSG_DATA(msg_p);
    HTTP_PMGR_SendMsg(HTTP_MGR_IPC_CMD_GET_RUNNING_HTTP_SECURE_STATUS,
                      msg_p,
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_RUNNING_STATUS_Type_T),
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_RUNNING_STATUS_Type_T),
                      (UI32_T)SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *status = data_p->running_status;

    return HTTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Set_Secure_Http_Status
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set https status
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : SECURE_HTTP_STATE_ENABLED / SECURE_HTTP_STATE_DISABLED
 * NOTE		: none
 * ------------------------------------------------------------------------*/
BOOL_T HTTP_PMGR_Set_Secure_Http_Status (SECURE_HTTP_STATE_T state)
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T    *data_p;

    data_p = HTTP_MGR_MSG_DATA(msg_p);
    data_p->status = state;
    HTTP_PMGR_SendMsg(HTTP_MGR_IPC_CMD_SET_SECURE_HTTP_STATUS,
                      msg_p,
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T),
                      HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      FALSE);

    return (BOOL_T) HTTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Get_Secure_Http_Status
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get https status
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : SECURE_HTTP_STATE_ENABLED / SECURE_HTTP_STATE_DISABLED
 * NOTE		: none
 * ------------------------------------------------------------------------*/
SECURE_HTTP_STATE_T HTTP_PMGR_Get_Secure_Http_Status()
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T    *data_p;
    UI32_T ret;

    data_p = HTTP_MGR_MSG_DATA(msg_p);
    msg_p->cmd = SYS_MODULE_HTTP;
    msg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
    HTTP_MGR_MSG_CMD(msg_p) = HTTP_MGR_IPC_CMD_GET_SECURE_HTTP_STATUS;

    ret = SYSFUN_SendRequestMsg(ipcmsgq_handle,
                                msg_p,
                                SYSFUN_TIMEOUT_WAIT_FOREVER,
                                SYSFUN_SYSTEM_EVENT_IPCMSG,
                                HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T),
                                msg_p);

    return (data_p->status);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Get_Secure_Port
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get https port
 * INPUT    : None
 * OUTPUT	: port
 * RETURN   : TRUE/FALSE
 * NOTE		: none
 * ------------------------------------------------------------------------*/
BOOL_T HTTP_PMGR_Get_Secure_Port(UI32_T *port)
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T    *data_p;

    data_p = HTTP_MGR_MSG_DATA(msg_p);
    HTTP_PMGR_SendMsg(HTTP_MGR_IPC_CMD_GET_SECURE_HTTP_PORT,
                      msg_p,
                      HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T),
                      FALSE);

    if ( TRUE == (BOOL_T) HTTP_MGR_MSG_RETVAL(msg_p))
    {
        *port = data_p->port;
        return TRUE;
    }
    return FALSE;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Set_Secure_Http_Port
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set https status
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : SECURE_HTTP_STATE_ENABLED / SECURE_HTTP_STATE_DISABLED
 * NOTE		: none
 * ------------------------------------------------------------------------*/
BOOL_T HTTP_PMGR_Set_Secure_Port (UI32_T port)
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T    *data_p;

    data_p = HTTP_MGR_MSG_DATA(msg_p);
    data_p->port = port;
    HTTP_PMGR_SendMsg(HTTP_MGR_IPC_CMD_SET_SECURE_HTTP_PORT,
                      msg_p,
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T),
                      HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T) HTTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Set_Tftp_Download_Certificate_Flag
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set a flag to protect only one web user seting
 *            parameter for download certificate file.
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : TRUE    --  Indicate user can go on to set parameters.
 *            FALSE   --  Indicate have another user keep those parameters.
 * NOTE		: none
 * ------------------------------------------------------------------------*/
BOOL_T HTTP_PMGR_Set_Tftp_Download_Certificate_Flag()
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;

    HTTP_PMGR_SendMsg(HTTP_MGR_IPC_CMD_SET_TFTP_DOWNLOAD_CERTIFICATE_FLAG,
                      msg_p,
                      HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T) HTTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Set_Tftp_Ip
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set tftp server to download certificate.
 * INPUT    : L_INET_AddrIp_T  *tftp_server --  tftp server.
 * OUTPUT	: None
 * RETURN   : TRUE    --  successful.
 *            FALSE   --  failure.
 * NOTE		: none
 * ------------------------------------------------------------------------*/
BOOL_T HTTP_PMGR_Set_Tftp_Ip(L_INET_AddrIp_T *tftp_server)
{
    UI8_T    msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_TFTP_ADDR_Type_T))];
    SYSFUN_Msg_T                   *msg_p = (SYSFUN_Msg_T *)msgbuf;
    HTTP_MGR_IPCMsg_TFTP_ADDR_Type_T    *data_p;

    data_p = HTTP_MGR_MSG_DATA(msg_p);
    data_p->server_ip = *tftp_server;
    HTTP_PMGR_SendMsg(HTTP_MGR_IPC_CMD_SET_TFTP_IP,
                      msg_p,
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_TFTP_ADDR_Type_T),
                      HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      FALSE);

    return (BOOL_T) HTTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Set_Tftp_Certificate_Filename
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set certificate flename to be download from tftp server.
 * INPUT    : UI8_T   *tftp_cert_file --  certificate filename.
 * OUTPUT	: None
 * RETURN   : TRUE    --  successful.
 *            FALSE   --  failure.
 * NOTE		: none
 * ------------------------------------------------------------------------*/
BOOL_T HTTP_PMGR_Set_Tftp_Certificate_Filename(UI8_T *tftp_cert_file)
{
    UI8_T    msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_TFTP_SRC_FILENAME_Type_T))];
    SYSFUN_Msg_T                   *msg_p = (SYSFUN_Msg_T *)msgbuf;
    HTTP_MGR_IPCMsg_TFTP_SRC_FILENAME_Type_T    *data_p;

    data_p = HTTP_MGR_MSG_DATA(msg_p);
    memcpy(data_p->tftp_src_filename, tftp_cert_file, sizeof(data_p->tftp_src_filename));

    HTTP_PMGR_SendMsg(HTTP_MGR_IPC_CMD_SET_TFTP_CERTIFICATE_FILENAME,
                      msg_p,
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_TFTP_SRC_FILENAME_Type_T),
                      HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      FALSE);

    return (BOOL_T) HTTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Set_Tftp_Privatekey_Filename
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set privatekey flename to be download from tftp server.
 * INPUT    : UI8_T   *tftp_private_file  --  privatekey filename.
 * OUTPUT	: None
 * RETURN   : TRUE    --  successful.
 *            FALSE   --  failure.
 * NOTE		: none
 * ------------------------------------------------------------------------*/
BOOL_T HTTP_PMGR_Set_Tftp_Privatekey_Filename(UI8_T *tftp_private_file)
{
    UI8_T    msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_TFTP_SRC_FILENAME_Type_T))];
    SYSFUN_Msg_T                   *msg_p = (SYSFUN_Msg_T *)msgbuf;
    HTTP_MGR_IPCMsg_TFTP_SRC_FILENAME_Type_T    *data_p;

    data_p = HTTP_MGR_MSG_DATA(msg_p);
    memcpy(data_p->tftp_src_filename, tftp_private_file, sizeof(data_p->tftp_src_filename));

    HTTP_PMGR_SendMsg(HTTP_MGR_IPC_CMD_SET_PRIVATEKEY_FILENAME,
                      msg_p,
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_TFTP_SRC_FILENAME_Type_T),
                      HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      FALSE);

    return (BOOL_T) HTTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Set_Tftp_Privatekey_Password
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set privatekey password to be download from tftp server.
 * INPUT    : UI8_T   *tftp_private_password  --  privatekey password.
 * OUTPUT	: None
 * RETURN   : TRUE    --  successful.
 *            FALSE   --  failure.
 * NOTE		: none
 * ------------------------------------------------------------------------*/
BOOL_T HTTP_PMGR_Set_Tftp_Privatekey_Password(UI8_T *tftp_private_password)
{
    UI8_T    msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_SERVER_PRIVATE_PWD_Type_T))];
    SYSFUN_Msg_T                   *msg_p = (SYSFUN_Msg_T *)msgbuf;
    HTTP_MGR_IPCMsg_SERVER_PRIVATE_PWD_Type_T    *data_p;

    data_p = HTTP_MGR_MSG_DATA(msg_p);
    memcpy(data_p->passwd, tftp_private_password, sizeof(data_p->passwd));

    HTTP_PMGR_SendMsg(HTTP_MGR_IPC_CMD_SET_PRIVATEKEY_PASSWORD,
                      msg_p,
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_SERVER_PRIVATE_PWD_Type_T),
                      HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      FALSE);

    return (BOOL_T) HTTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Get_Certificate_From_Tftp
 * ------------------------------------------------------------------------|
 * FUNCTION : A server certificate authenticates the server to the client.
 *            This function to save server certificate ,
 *            when you got a server certificate from a certification authority(CA).
 * INPUT    : certificate_entry_p  -- certificate entry
 * OUTPUT	: None
 * RETURN   : TRUE    --  successful.
 *            FALSE   --  failure.
 * NOTE		: User can use TFTP or XModem to upload server certificate
 * ------------------------------------------------------------------------*/
BOOL_T
HTTP_PMGR_Get_Certificate_From_Tftp(
    HTTP_DownloadCertificateEntry_T *certificate_entry_p)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_DOWNLOAD_CERTIFICATE_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    HTTP_MGR_IPCMsg_DOWNLOAD_CERTIFICATE_T    *data_p;

    data_p = HTTP_MGR_MSG_DATA(msg_p);
    memcpy(&data_p->certificate_entry, certificate_entry_p, sizeof(data_p->certificate_entry));

    HTTP_PMGR_SendMsg(HTTP_MGR_IPC_CMD_GET_CERTIFICATE_FROM_TFTP,
                      msg_p,
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_DOWNLOAD_CERTIFICATE_T),
                      HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T) HTTP_MGR_MSG_RETVAL(msg_p);
}
#endif /* SYS_CPNT_HTTPS */

/* FUNCTION NAME:  HTTP_PMGR_GetRunningHttpParameters
 * PURPOSE:
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific http parameters with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          http_cfg - structure containing changed of status and non-defalut value.
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 */
UI32_T HTTP_PMGR_GetRunningHttpParameters(HTTP_MGR_RunningCfg_T *http_cfg)
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_RUNNING_CFG_Type_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    HTTP_MGR_IPCMsg_HTTP_RUNNING_CFG_Type_T    *data_p;

    data_p = HTTP_MGR_MSG_DATA(msg_p);

    if(http_cfg == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    HTTP_PMGR_SendMsg(HTTP_MGR_IPC_CMD_GET_RUNNING_HTTP_PARAMETERS,
                      msg_p,
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_RUNNING_CFG_Type_T),
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_RUNNING_CFG_Type_T),
                      (UI32_T)SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if(HTTP_MGR_MSG_RETVAL(msg_p) != SYS_TYPE_GET_RUNNING_CFG_FAIL)
        memcpy(http_cfg,&data_p->http_runcfg_data, sizeof(HTTP_MGR_RunningCfg_T));

    return HTTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - HTTP_PMGR_Get_Next_User_Connection_Info
 *-------------------------------------------------------------------------
 * PURPOSE : To get next user connection information.
 * INPUT   : connection_info
 * OUTPUT  : connection_info
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T HTTP_PMGR_Get_Next_User_Connection_Info(HTTP_Session_T *connection_info)
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE(HTTP_Session_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    HTTP_Session_T    *data_p;

    data_p = HTTP_MGR_MSG_DATA(msg_p);

    memcpy(data_p, connection_info, sizeof(HTTP_Session_T));

    HTTP_PMGR_SendMsg(HTTP_MGR_IPC_CMD_GETNEXT_USER_CONNECT_INFO,
                      msg_p,
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_Session_T),
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_Session_T),
                      (UI32_T)FALSE);

      memcpy(connection_info, data_p, sizeof(HTTP_Session_T));
    return  (BOOL_T) HTTP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - HTTP_PMGR_Get_Next_User_Connection_Info
 *-------------------------------------------------------------------------
 * PURPOSE : To get next user connection information.
 * INPUT   : connection_info
 * OUTPUT  : connection_info
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T HTTP_PMGR_Set_User_Connection_Send_Log(L_INET_AddrIp_T ip_addr, UI8_T *username, BOOL_T is_send_log)
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(HTTP_MGR_GET_MSGBUFSIZE(HTTP_Session_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    HTTP_Session_T    *data_p;

     data_p = HTTP_MGR_MSG_DATA(msg_p);
     memcpy(&data_p->remote_ip,&ip_addr, sizeof(L_INET_AddrIp_T));
     data_p->is_send_log=is_send_log;
    memcpy(data_p->username, username, sizeof(data_p->username));
    HTTP_PMGR_SendMsg(HTTP_MGR_IPC_CMD_SET_USER_CONNECT_SENDLOGFLAG,
                      msg_p,
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_Session_T),
                      HTTP_MGR_GET_MSGBUFSIZE(HTTP_Session_T),
                      (UI32_T)FALSE);
    return  (BOOL_T) HTTP_MGR_MSG_RETVAL(msg_p);
}

/* LOCAL SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - HTTP_PMGR_SendMsg
 *-------------------------------------------------------------------------
 * PURPOSE : To send an IPC message.
 * INPUT   : cmd       - the HTTP message command.
 *           msg_p     - the buffer of the IPC message.
 *           req_size  - the size of HTTP request message.
 *           res_size  - the size of HTTP response message.
 *           ret_val   - the return value to set when IPC is failed.
 * OUTPUT  : msg_p     - the response message.
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
static void HTTP_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val)
{
    UI32_T ret;

    msg_p->cmd = SYS_MODULE_HTTP;
    msg_p->msg_size = req_size;

    HTTP_MGR_MSG_CMD(msg_p) = cmd;

    ret = SYSFUN_SendRequestMsg(ipcmsgq_handle,
                                msg_p,
                                SYSFUN_TIMEOUT_WAIT_FOREVER,
                                SYSFUN_SYSTEM_EVENT_IPCMSG,
                                res_size,
                                msg_p);

    /* If IPC is failed, set return value as ret_val.
     */
    if ((ret != SYSFUN_OK) || (HTTP_MGR_MSG_RETVAL(msg_p) == FALSE))
        HTTP_MGR_MSG_RETVAL(msg_p) = ret_val;
}
