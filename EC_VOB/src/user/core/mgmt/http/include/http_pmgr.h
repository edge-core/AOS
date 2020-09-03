/* MODULE NAME: http_Pmgr.h
 * PURPOSE:
 *   Initialize the resource and provide some functions for the http module.
 *
 * NOTES:
 *
 * History:
 *       Date          -- Modifier,  Reason
 *     2007-10         --Rich ,       created.
 *
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef HTTP_PMGR_H
#define HTTP_PMGR_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "http_type.h"
#include "http_mgr.h"

/* NAMING CONSTANT DECLARATIONS
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
BOOL_T HTTP_PMGR_InitiateProcessResource(void);

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
SYS_TYPE_Get_Running_Cfg_T  HTTP_PMGR_Get_Running_Debug_Information_Status(HTTP_MGR_DebugType_T *debug_type);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Get_Running_Http_Port
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get running http port
 * INPUT    : None
 * OUTPUT	: port
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS / SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE		: none
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  HTTP_PMGR_Get_Running_Http_Port(UI32_T *port);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Get_Running_Http_Status
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get running http port
 * INPUT    : None
 * OUTPUT	: status
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS / SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE		: none
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  HTTP_PMGR_Get_Running_Http_Status(UI32_T *status);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Get_Http_Port
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get http port
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : port
 * NOTE		: none
 * ------------------------------------------------------------------------*/
UI32_T HTTP_PMGR_Get_Http_Port();

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Get_Http_Status
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get http status
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : port
 * NOTE		: none
 * ------------------------------------------------------------------------*/
HTTP_STATE_T HTTP_PMGR_Get_Http_Status();

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Set_Http_Port
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set http port
 * INPUT    : port
 * OUTPUT	: None
 * RETURN   : port
 * NOTE		: none
 * ------------------------------------------------------------------------*/
BOOL_T HTTP_PMGR_Set_Http_Port (UI32_T port);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Set_Http_Status
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set http status
 * INPUT    : state
 * OUTPUT	: None
 * RETURN   : port
 * NOTE		: none
 * ------------------------------------------------------------------------*/
BOOL_T HTTP_PMGR_Set_Http_Status (HTTP_STATE_T state);
#endif /* #if (SYS_CPNT_HTTP == TRUE) */

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
SYS_TYPE_Get_Running_Cfg_T  HTTP_PMGR_Get_Running_Http_Secure_Port(UI32_T *port);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Get_Running_Secure_Http_Status
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get running https status
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS / SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE		: none
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  HTTP_PMGR_Get_Running_Secure_Http_Status(UI32_T *status);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Set_Secure_Http_Status
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set https status
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : SECURE_HTTP_STATE_ENABLED / SECURE_HTTP_STATE_DISABLED
 * NOTE		: none
 * ------------------------------------------------------------------------*/
BOOL_T HTTP_PMGR_Set_Secure_Http_Status (SECURE_HTTP_STATE_T state);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Get_Secure_Port
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get https port
 * INPUT    : None
 * OUTPUT	: port
 * RETURN   : TRUE/FALSE
 * NOTE		: none
 * ------------------------------------------------------------------------*/
BOOL_T HTTP_PMGR_Get_Secure_Port(UI32_T *port);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Get_Secure_Http_Status
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get https status
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : SECURE_HTTP_STATE_ENABLED / SECURE_HTTP_STATE_DISABLED
 * NOTE		: none
 * ------------------------------------------------------------------------*/
SECURE_HTTP_STATE_T HTTP_PMGR_Get_Secure_Http_Status();

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - HTTP_PMGR_Set_Secure_Http_Port
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set https status
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : SECURE_HTTP_STATE_ENABLED / SECURE_HTTP_STATE_DISABLED
 * NOTE		: none
 * ------------------------------------------------------------------------*/
BOOL_T HTTP_PMGR_Set_Secure_Port (UI32_T port);

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
BOOL_T HTTP_PMGR_Set_Tftp_Download_Certificate_Flag();

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
BOOL_T HTTP_PMGR_Set_Tftp_Ip(L_INET_AddrIp_T *tftp_server);

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
BOOL_T HTTP_PMGR_Set_Tftp_Certificate_Filename(UI8_T *tftp_cert_file);

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
BOOL_T HTTP_PMGR_Set_Tftp_Privatekey_Filename(UI8_T *tftp_private_file);

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
BOOL_T HTTP_PMGR_Set_Tftp_Privatekey_Password(UI8_T *tftp_private_password);

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
    HTTP_DownloadCertificateEntry_T *certificate_entry_p
);

#endif /* #if (SYS_CPNT_HTTPS == TRUE) */

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
UI32_T HTTP_PMGR_GetRunningHttpParameters(HTTP_MGR_RunningCfg_T *http_cfg);
/* LOCAL SUBPROGRAM BODIES
 */
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
BOOL_T  HTTP_PMGR_Get_Next_User_Connection_Info(HTTP_Session_T *session);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - HTTP_PMGR_Set_User_Connection_Send_Log
 *-------------------------------------------------------------------------
 * PURPOSE : To set user connection send log.
 * INPUT   : ip_addr username is_send_log
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T  HTTP_PMGR_Set_User_Connection_Send_Log(L_INET_AddrIp_T ip_addr, UI8_T *username, BOOL_T is_send_log);

#endif /* end HTTP_PMGR_H */
