/* MODULE NAME: http_om.h
 * PURPOSE:
 *   {1. What is covered in this file - function and scope.}
 *   {2. Related documents or hardware information}
 * NOTES:
 *     {Something must be known or noticed}
 *   {1. How to use these functions - Give an example.}
 *   {2. Sequence of messages if applicable.}
 *   {3. Any design limitation}
 *   {4. Any performance limitation}
 *   {5. Is it a reusable component}
 *
 * CREATOR:  Isiah           Date 2002-02
 *
 * Copyright(C)      Accton Corporation, 2002
 */



#ifndef HTTP_OM_H

#define HTTP_OM_H



/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "sys_type.h"
#include "http_def.h"
#include "http_type.h"
#include "l_inet.h"

#if (SYS_CPNT_HTTPS == TRUE)
#include "openssl/ssl.h"
#endif /* if (SYS_CPNT_HTTPS == TRUE) */

#if __cplusplus
extern "C" {
#endif




/* NAMING CONSTANT DECLARATIONS
 */





/* MACRO FUNCTION DECLARATIONS
 */
#define HTTP_OM_EnterCriticalSection(sem_id)    SYSFUN_OM_ENTER_CRITICAL_SECTION(sem_id)
#define HTTP_OM_LeaveCriticalSection(sem_id, orig_priority)    SYSFUN_OM_LEAVE_CRITICAL_SECTION(sem_id, orig_priority)


/* DATA TYPE DECLARATIONS
 */



/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:  HTTP_OM_InitateSystemResource
 * PURPOSE:
 *          Initiate system resource.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate changed and FALSE to indicate not changed.
 * NOTES:
 *
 */
void HTTP_OM_InitateSystemResource(void);

/* FUNCTION NAME:  HTTP_OM_ClearConfig
 * PURPOSE:
 *          Clear the config.
 *
 * INPUT:
 *          None
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          None
 *
 * NOTES:
 *          None
 */
void HTTP_OM_ClearConfig();

/* FUNCTION NAME:  HTTP_OM_ConfigResetToDefault
 * PURPOSE:
 *          Reset config to default.
 *
 * INPUT:
 *          None
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          None
 *
 * NOTES:
 *          None
 */
void HTTP_OM_ConfigResetToDefault();

/* FUNCTION NAME:  HTTP_OM_UpdateConfigFilePointer
 * PURPOSE:
 *          Update the config file pointer. The pointer shall available for
 *          HTTP_OM. The caller will not need to free this pointer.
 *
 * INPUT:
 *          None
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          None
 *
 * NOTES:
 *          None
 */
BOOL_T HTTP_OM_UpdateConfigFilePointer(const void *file);

/* FUNCTION NAME:  HTTP_OM_GetConfigValue
 * PURPOSE:
 *          Get config by key.
 *
 * INPUT:
 *          key
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          config value
 *
 * NOTES:
 *          None
 */
void *HTTP_OM_GetConfigValue(const char *key);

/* FUNCTION NAME:  HTTP_OM_Set_Root_Dir
 * PURPOSE:
 *          Set web root directory.
 *
 * INPUT:
 *          dir - the root directory
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          None
 *
 * NOTES:
 *          None
 */
BOOL_T HTTP_OM_Set_Root_Dir(const char *dir);

/* FUNCTION NAME:  HTTP_OM_Set_Root_Dir
 * PURPOSE:
 *          Get web root directory.
 *
 * INPUT:
 *          buf - buffer pointer
 *          bufsz - buffer size
 *
 * OUTPUT:
 *          the root directory
 *
 * RETURN:
 *          None
 *
 * NOTES:
 *          None
 */
BOOL_T HTTP_OM_Get_Root_Dir(char *buf, size_t bufsz);

/* FUNCTION NAME:  HTTP_OM_Set_Https_Certificate_Path
 * PURPOSE:
 *          Set HTTP certificate path
 *
 * INPUT:
 *          path
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          TRUE/FALSE
 *
 * NOTES:
 *          None
 */
HTTP_OM_ConfigState_T HTTP_OM_Set_Https_Certificate_Path(const char* path);

/* FUNCTION NAME:  HTTP_OM_Get_Https_Certificate_Path
 * PURPOSE:
 *          Get HTTPS certificate path.
 *
 * INPUT:
 *          buf, bufsz
 *
 * OUTPUT:
 *          buf
 *
 * RETURN:
 *          TRUE/FALSE
 *
 * NOTES:
 *          None
 */
BOOL_T HTTP_OM_Get_Https_Certificate_Path(char *buf, size_t bufsz);

/* FUNCTION NAME:  HTTP_OM_Set_Https_Private_Key_Path
 * PURPOSE:
 *          Set HTTP certificate private key path
 *
 * INPUT:
 *          path
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          TRUE/FALSE
 *
 * NOTES:
 *          None
 */
HTTP_OM_ConfigState_T HTTP_OM_Set_Https_Private_Key_Path(const char* path);

/* FUNCTION NAME:  HTTP_OM_Get_Https_Private_Key_Path
 * PURPOSE:
 *          Get HTTPS certificate private key path.
 *
 * INPUT:
 *          buf, bufsz
 *
 * OUTPUT:
 *          buf
 *
 * RETURN:
 *          TRUE/FALSE
 *
 * NOTES:
 *          None
 */
BOOL_T HTTP_OM_Get_Https_Private_Key_Path(char *buf, size_t bufsz);

/* FUNCTION NAME:  HTTP_OM_Set_Https_Passphrase_Path
 * PURPOSE:
 *          Set HTTP certificate pass phrase path
 *
 * INPUT:
 *          path
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          TRUE/FALSE
 *
 * NOTES:
 *          None
 */
HTTP_OM_ConfigState_T HTTP_OM_Set_Https_Passphrase_Path(const char* path);

/* FUNCTION NAME:  HTTP_OM_Get_Https_Passphrase_Path
 * PURPOSE:
 *          Get HTTPS certificate pass phrase path.
 *
 * INPUT:
 *          buf, bufsz
 *
 * OUTPUT:
 *          buf
 *
 * RETURN:
 *          TRUE/FALSE
 *
 * NOTES:
 *          None
 */
BOOL_T HTTP_OM_Get_Https_Passphrase_Path(char *buf, size_t bufsz);

/* FUNCTION NAME:  HTTP_OM_Set_Http_Status
 * PURPOSE:
 *          This function set http state.
 *
 * INPUT:
 *          HTTP_STATE_T - HTTP status.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void HTTP_OM_Set_Http_Status (HTTP_STATE_T );



/* FUNCTION NAME:  HTTP_OM_Get_Http_Status
 * PURPOSE:
 *          This function get http state.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          HTTP_STATE_T - HTTP status.
 * NOTES:
 *          .
 */
HTTP_STATE_T HTTP_OM_Get_Http_Status();



/* FUNCTION NAME:  HTTP_OM_Set_Http_Port
 * PURPOSE:
 *          This function set http port number.
 *
 * INPUT:
 *          UI32_T - HTTP port number.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Set_Http_Port (UI32_T );



/* FUNCTION NAME:  HTTP_OM_Get_Http_Port
 * PURPOSE:
 *			This function get http port number.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T - HTTP port value.
 * NOTES:
 *          default is tcp/80.
 */
UI32_T HTTP_OM_Get_Http_Port(void);



/* FUNCTION NAME:  HTTP_OM_Set_OpMode
 * PURPOSE:
 *          This function set http operation mode.
 *
 * INPUT:
 *          SYS_TYPE_Stacking_Mode_T - opmode.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void HTTP_OM_Set_OpMode (SYS_TYPE_Stacking_Mode_T );



/* FUNCTION NAME:  HTTP_OM_Get_OpMode
 * PURPOSE:
 *          This function get http operation mode.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          SYS_TYPE_Stacking_Mode_T - opmode.
 * NOTES:
 *          .
 */
SYS_TYPE_Stacking_Mode_T HTTP_OM_Get_OpMode ();



/* FUNCTION NAME:  HTTP_OM_CleanAllConnectionObjects
 * PURPOSE:
 *          Clean all connection
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void HTTP_OM_CleanAllConnectionObjects();



/* FUNCTION NAME:  HTTP_OM_AllocateConnectionObject
 * PURPOSE:
 *          Allocate a connection object
 *
 * INPUT:
 *          tid     -- task ID
 *          sockfd  -- socket ID
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          Connection object
 * NOTES:
 *          .
 */
HTTP_Connection_T * HTTP_OM_AllocateConnectionObject(UI32_T tid, int sockfd);



/* FUNCTION NAME:  HTTP_OM_FreeConnectionObject
 * PURPOSE:
 *          Free a connection object
 *
 * INPUT:
 *          conn_obj_p  -- A connection object
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE    -- successed;
 *          FALSE   -- connection object is invalid
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_FreeConnectionObject(HTTP_Connection_T * conn_obj_p);



/* FUNCTION NAME:  HTTP_OM_GetNextConnectionObjectAtIndex
 * PURPOSE:
 *          Enumerates all connection objects
 *
 * INPUT:
 *          tid    -- task ID
 *          index  -- index. Use 0xffffffff to get first object.
 *
 * OUTPUT:
 *          http_connection -- connection object
 *
 * RETURN:
 *          TRUE/FALSE
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_GetNextConnectionObjectAtIndex(UI32_T tid, UI32_T *index, HTTP_Connection_T *http_connection);

/* FUNCTION NAME:  HTTP_OM_HasConnectionObject
 * PURPOSE:
 *          Test has any connection connected
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE - has connection connected; FALSE - no any connection
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_HasConnectionObject();

HTTP_Worker_T * HTTP_OM_GetWorkerByIndex(UI32_T idx);

HTTP_Worker_T * HTTP_OM_GetMasterWorker();

void HTTP_OM_InitLog();

void HTTP_OM_AddLog(struct tm *occurred_time, HTTP_LOG_LEVEL_T level, HTTP_LOG_MSG_TYPE_T message_type, const char *function, int line, const char *message);

/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
#if (SYS_CPNT_HTTPS == TRUE)

/* FUNCTION NAME:  HTTP_OM_Set_Secure_Port
 * PURPOSE:
 *          This function set security port number.
 *
 * INPUT:
 *          UI32_T - HTTPS port number.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate changed an FALSE to indicate not changed.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Set_Secure_Port (UI32_T );



/* FUNCTION NAME:  HTTP_OM_Get_Secure_Port
 * PURPOSE:
 *			This function get security port number.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T * - HTTPS port value.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          default is tcp/443.
 */
BOOL_T HTTP_OM_Get_Secure_Port(UI32_T *);



/* FUNCTION NAME:  HTTP_OM_Set_Secure_Http_Status
 * PURPOSE:
 *          This function set https state.
 *
 * INPUT:
 *          SECURE_HTTP_STATE_T - HTTPS status.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void HTTP_OM_Set_Secure_Http_Status (SECURE_HTTP_STATE_T state);



/* FUNCTION NAME:  HTTP_OM_Get_Secure_Http_Status
 * PURPOSE:
 *          This function get https state.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          SECURE_HTTP_STATE_T - HTTPS status.
 * NOTES:
 *          .
 */
BOOL_T  HTTP_OM_Get_Secure_Http_Status(SECURE_HTTP_STATE_T *state_p);



/* FUNCTION NAME:  HTTP_OM_Set_SSL_CTX
 * PURPOSE:
 *          Store SSL Context object pointer.
 *
 * INPUT:
 *          SSL_CTX * - SSL_CTX object.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          (Something must be known to use this function.)
 */
BOOL_T HTTP_OM_Set_SSL_CTX(SSL_CTX *);



/* FUNCTION NAME:  HTTP_OM_Get_SSL_CTX
 * PURPOSE:
 *          Get SSL Context object pointer.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          SSL_CTX * - SSL_CTX object.
 * NOTES:
 *          (Something must be known to use this function.)
 */
SSL_CTX *HTTP_OM_Get_SSL_CTX();



/* FUNCTION NAME:  HTTP_OM_Set_SSL_Object
 * PURPOSE:
 *          Store SSL structure pointer.
 *
 * INPUT:
 *          SSL * - SSL structure.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Set_SSL_Object(SSL *);



/* FUNCTION NAME:  HTTP_OM_Get_SSL_Object
 * PURPOSE:
 *          Get SSL structure pointer.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          SSL * - SSL structure.
 * NOTES:
 *          .
 */
SSL *HTTP_OM_Get_SSL_Object();



/* FUNCTION NAME:  HTTP_OM_Init_Session_Cache
 * PURPOSE:
 *          Initialize session cache
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Init_Session_Cache();



/* FUNCTION NAME:  HTTP_OM_Get_SSL_Session_Cache_Timeout
 * PURPOSE:
 *			Get Session cache timeout .
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T - Session Cache Timeout.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Get_SSL_Session_Cache_Timeout(UI32_T *);



/* FUNCTION NAME:  HTTP_OM_Set_SSL_Session_Cache_Timeout
 * PURPOSE:
 *			Set Session cache timeout .
 *
 * INPUT:
 *          UI32_T  -- Timeout value.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Set_SSL_Session_Cache_Timeout(UI32_T );



/* FUNCTION NAME:  HTTP_OM_Set_SSL_Session_Cache
 * PURPOSE:
 *          Store the SSL_SESSION in the cache.
 *
 * INPUT:
 *          UI8_T *        -- session_id.
 *
 *          UI32_T         -- session_id_length.
 *
 *          SSL_SESSION *  -- SSL_SESSION structure.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in HTTP_MGR_Set_SSL_Session_Cache_Entry.
 */
BOOL_T HTTP_OM_Set_SSL_Session_Cache(UI8_T *, UI32_T , SSL_SESSION *);



/* FUNCTION NAME:  HTTP_OM_Get_SSL_Session_Cache
 * PURPOSE:
 *          Try to retrieve the SSL_SESSION from cache.
 *
 * INPUT:
 *          UI8_T *  -- session_id.
 *
 *          UI32_T   -- session_id_length.
 *
 * OUTPUT:
 *          SSL_SESSION ** -- SSL_SESSION structure.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in HTTP_MGR_Get_SSL_Session_Cache_Entry.
 */
BOOL_T HTTP_OM_Get_SSL_Session_Cache(UI8_T *, UI32_T , SSL_SESSION **);



/* FUNCTION NAME:  HTTP_OM_Delete_SSL_Session_Cache
 * PURPOSE:
 *          Remove the SSL_SESSION from cache.
 *
 * INPUT:
 *          UI8_T *  -- session_id.
 *
 *          UI32_T   -- session_id_length.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in HTTP_MGR_Delete_SSL_Session_Cache_Entry.
 */
BOOL_T HTTP_OM_Delete_SSL_Session_Cache(UI8_T *, UI32_T );



/* FUNCTION NAME:  HTTP_OM_Set_Keeper_Task_Id
 * PURPOSE:
 *			Set task id which keep certificate buffer.
 *
 * INPUT:
 *          UI32_T  tid --  task id.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *			.
 */
BOOL_T HTTP_OM_Set_Keeper_Task_Id(UI32_T tid);



/* FUNCTION NAME:  HTTP_OM_Get_Keeper_Task_Id
 * PURPOSE:
 *          Get task id which keep certificate buffer.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T  *tid    --  task id.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Get_Keeper_Task_Id(UI32_T *tid);



/* FUNCTION NAME:  HTTP_OM_Set_Connection_Status
 * PURPOSE:
 *          This function set current connection status to http or https.
 *
 * INPUT:
 *          CONNECTION_STATE_T -- current connection status.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Set_Connection_Status(CONNECTION_STATE_T connection_status);



/* FUNCTION NAME:  HTTP_OM_Get_Connection_Status
 * PURPOSE:
 *          This function get current connection status is http or https.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          CONNECTION_STATE_T -- current connection status.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Get_Connection_Status(CONNECTION_STATE_T *state_p);



/* FUNCTION NAME:  HTTP_OM_Get_Debug_Status
 * PURPOSE:
 *          This function to get debug status -- Is_Debug_Session and Is_Debug_State.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          HTTPS_DEBUG_SESSION_STATE_T * -- debug_session flag
 *			HTTPS_DEBUG_STATE_STATE_T  * -- debug_state flag
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Get_Debug_Status(HTTPS_DEBUG_SESSION_STATE_T *, HTTPS_DEBUG_STATE_STATE_T *);



/* FUNCTION NAME:  HTTP_OM_Set_Debug_Status
 * PURPOSE:
 *          This function to set debug status -- Is_Debug_Session and Is_Debug_State.
 *
 * INPUT:
 *          HTTPS_DEBUG_SESSION_STATE_T  -- debug_session flag
 *			HTTPS_DEBUG_STATE_STATE_T   -- debug_state flag
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Set_Debug_Status(HTTPS_DEBUG_SESSION_STATE_T , HTTPS_DEBUG_STATE_STATE_T );



/* FUNCTION NAME:  HTTP_OM_Set_Auto_Redirect_To_Https_Status
 * PURPOSE:
 *          This function set state of http redirect to https.
 *
 * INPUT:
 *          HTTP_Redirect_Status_T status    --  http redirect to https status.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Set_Auto_Redirect_To_Https_Status(HTTP_Redirect_Status_T status);



/* FUNCTION NAME:  HTTP_OM_Get_Auto_Redirect_To_Https_Status
 * PURPOSE:
 *          This function get state of http redirect to https.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          HTTP_Redirect_Status_T *status   --  http redirect to https status.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Get_Auto_Redirect_To_Https_Status(HTTP_Redirect_Status_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - HTTP_OM_Init_Download_Certificate_Entry
 *-------------------------------------------------------------------------
 * PURPOSE  : Initialize download certificate entry.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void HTTP_OM_Init_Download_Certificate_Entry();

/* FUNCTION NAME:  HTTP_OM_Set_Tftp_Ip
 * PURPOSE:
 *          This function set tftp server to download certificate.
 *
 * INPUT:
 *          L_INET_AddrIp_T  *tftp_server --  tftp server.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Set_Tftp_Ip(L_INET_AddrIp_T *tftp_server);



/* FUNCTION NAME:  HTTP_OM_Get_Tftp_Ip
 * PURPOSE:
 *          This function get tftp server will be download certificate.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          L_INET_AddrIp_T  *tftp_server    --  tftp server.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Get_Tftp_Ip(L_INET_AddrIp_T *tftp_server);



/* FUNCTION NAME:  HTTP_OM_Set_Tftp_Certificate_Filename
 * PURPOSE:
 *          This function set certificate flename to be download from tftp server.
 *
 * INPUT:
 *          UI8_T   *tftp_cert_file --  certificate filename.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Set_Tftp_Certificate_Filename(const char *tftp_cert_file);



/* FUNCTION NAME:  HTTP_OM_Get_Tftp_Certificate_Filename
 * PURPOSE:
 *          This function get certificate flename to be download from tftp server.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI8_T   *tftp_cert_file --  certificate filename.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Get_Tftp_Certificate_Filename(char *tftp_cert_file);



/* FUNCTION NAME:  HTTP_OM_Set_Tftp_Privatekey_Filename
 * PURPOSE:
 *          This function set privatekey flename to be download from tftp server.
 *
 * INPUT:
 *          UI8_T   *tftp_private_file  --  privatekey filename.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Set_Tftp_Privatekey_Filename(const char *tftp_private_file);



/* FUNCTION NAME:  HTTP_OM_Get_Tftp_Privatekey_Filename
 * PURPOSE:
 *          This function get privatekey flename to be download from tftp server.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI8_T   *tftp_private_file  --  privatekey filename.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Get_Tftp_Privatekey_Filename(char *tftp_private_file);



/* FUNCTION NAME:  HTTP_OM_Set_Tftp_Privatekey_Password
 * PURPOSE:
 *          This function set privatekey password to be download from tftp server.
 *
 * INPUT:
 *          UI8_T   *tftp_private_password  --  privatekey password.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Set_Tftp_Privatekey_Password(const char *tftp_private_password);

/* FUNCTION NAME:  HTTP_OM_Set_Tftp_Cookie
 * PURPOSE:
 *          This function is used for saving CLI working area ID to HTTP OM cookie.
 * INPUT:
 *          UI32_T cookie  --  CLI working area ID.
 * OUTPUT:
 *          None.
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 */
BOOL_T HTTP_OM_Set_Tftp_Cookie(void *cookie);

/* FUNCTION NAME:  HTTP_OM_Get_Tftp_Privatekey_Password
 * PURPOSE:
 *          This function get privatekey password to be download from tftp server.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI8_T   *tftp_private_password  --  privatekey password.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Get_Tftp_Privatekey_Password(char *tftp_private_password);

/* FUNCTION NAME:  HTTP_OM_Get_Tftp_Cookie
 * PURPOSE:
 *          This function is used for reading CLI working area ID from HTTP OM cookie.
 * INPUT:
 *          None.
 * OUTPUT:
 *          UI32_T *cookie  --  CLI working area ID.
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 */
BOOL_T HTTP_OM_Get_Tftp_Cookie(void **cookie);


/* FUNCTION NAME:  HTTP_OM_Set_Tftp_Download_Certificate_Flag
 * PURPOSE:
 *          This function set a flag to protect only one web user seting parameter
 *          for download certificate file.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE    --  Indicate user can go on to set parameters.
 *          FALSE   --  Indicate have another user keep those parameters.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Set_Tftp_Download_Certificate_Flag();



/* FUNCTION NAME:  HTTP_OM_Reset_Tftp_Download_Certificate_Flag
 * PURPOSE:
 *          This function reset a flag to allow other web user seting parameter
 *          for download certificate file.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Reset_Tftp_Download_Certificate_Flag();



/* FUNCTION NAME:  HTTP_OM_Set_Certificate_Download_Status
 * PURPOSE:
 *          This function set certificate download status.
 *
 * INPUT:
 *          HTTP_GetCertificateStatus_T status  --  certificate download status.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Set_Certificate_Download_Status(HTTP_GetCertificateStatus_T status);



/* FUNCTION NAME:  HTTP_OM_Get_Certificate_Download_Status
 * PURPOSE:
 *          This function get certificate download status.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          HTTP_GetCertificateStatus_T *status --  certificate download status.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Get_Certificate_Download_Status(HTTP_GetCertificateStatus_T *status);



/* FUNCTION NAME:  HTTP_OM_Set_XferProgressStatus
 * PURPOSE:
 *          This function set status of xfer in progress.
 *
 * INPUT:
 *          BOOL_T  status  --  status of xfer in progress.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void HTTP_OM_Set_XferProgressStatus(BOOL_T status);



/* FUNCTION NAME:  HTTP_OM_Get_XferProgressStatus
 * PURPOSE:
 *          This function get status of xfer in progress.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          BOOL_T  *status --  status of xfer in progress.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void HTTP_OM_Get_XferProgressStatus(BOOL_T *status);



/* FUNCTION NAME:  HTTP_OM_Set_XferProgressResult
 * PURPOSE:
 *          This function set xfer process result.
 *
 * INPUT:
 *          BOOL_T  result  --  xfer process result.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void HTTP_OM_Set_XferProgressResult(BOOL_T result);



/* FUNCTION NAME:  HTTP_OM_Get_XferProgressResult
 * PURPOSE:
 *          This function get xfer process result.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          BOOL_T  *result --  status of xfer in progress.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void HTTP_OM_Get_XferProgressResult(BOOL_T *result);



/* FUNCTION NAME:  HTTP_OM_Get_Certificate_Status
 * PURPOSE:
 *          This function to get flag of certificate status.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          BOOL_T  *is_change  --  certificate is change or not.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Get_Certificate_Status(BOOL_T *is_change);



/* FUNCTION NAME:  HTTP_OM_Set_Certificate_Status
 * PURPOSE:
 *          This function to set flag of certificate status.
 *
 * INPUT:
 *          BOOL_T  is_change   --  certificate is change or not.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Set_Certificate_Status(BOOL_T is_change);




#endif /* if (SYS_CPNT_HTTPS == TRUE) */
/*-----------------------------------------------------------------------*/



/*isiah.2003-06-12. add for show web connection.*/
/* FUNCTION NAME:  HTTP_OM_Clear_User_Connection_Info
 * PURPOSE:
 *			Clear user conncection information.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *			This function is invoked in HTTP_MGR_Enter_Master_Mode.
 */
BOOL_T HTTP_OM_Clear_User_Connection_Info(void);



/* FUNCTION NAME:  HTTP_OM_Set_User_Connection_Info
 * PURPOSE:
 *          This function set user connection info.
 *
 * INPUT:
 *          remote_ip_p -- remote ip address.
 *          local_ip_p  -- local ip address.
 *          username    -- remote user.
 *          access_time -- access time.
 *          protocol    -- ptotocol of connection.
 *          auth_level  -- authentication level
 *          session_id  -- session ID
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Set_User_Connection_Info(L_INET_AddrIp_T *remote_ip_p, L_INET_AddrIp_T *local_ip_p, const char *username, UI32_T access_time, UI32_T protocol, int auth_level, const char *session_id);


/* FUNCTION NAME:  HTTP_OM_Get_User_Connection_Info
 * PURPOSE:
 *          This function get user connection info.
 *
 * INPUT:
 *          session_id  -- session ID
 *
 * OUTPUT:
 *          user_conn_info_p    -- user connection information
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Get_User_Connection_Info(const char *session_id, HTTP_Session_T *session);


/* FUNCTION NAME:  HTTP_OM_Check_User_Connection_Info
 * PURPOSE:
 *          This function check current connection info.
 *
 * INPUT:
 *          L_INET_AddrIp_T  ip_addr     --  remote ip address.
 *          UI8_T   *username   --  remote user.
 *          UI32_T  access_time --  access time.
 *          UI32_T  protocol    --  ptotocol of connection.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate have data and not timeout.
            FALSE to indicate no data or timeout.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Check_User_Connection_Info(UI32_T access_time, const char *session_id);


/* FUNCTION NAME:  HTTP_OM_Delete_User_Connection_Info_If_Timeout
 * PURPOSE:
 *          This function check current connection info.
 *
 * INPUT:
 *          UI32_T  access_time --  access time.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate have data and not timeout.
 *          FALSE to indicate no data or timeout.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Delete_User_Connection_Info_If_Timeout(UI32_T access_time, const char *session_id);


/* FUNCTION NAME:  HTTP_OM_Delete_User_Connection_Info
 * PURPOSE:
 *          This function delete current connection info.
 *
 * INPUT:
 *          L_INET_AddrIp_T  ip_addr     --  remote ip address.
 *          char    *username   --  remote user.
 *          UI32_T  access_time --  access time.
 *          UI32_T  protocol    --  ptotocol of connection.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Delete_User_Connection_Info(const char *session_id);

/* FUNCTION NAME:  HTTP_OM_Set_User_Connection_Send_Log
 * PURPOSE:
 *          This function set current connection send log flag.
 *
 * INPUT:
 *          L_INET_AddrIp_T  ip_addr     --  remote ip address.
 *          UI8_T   *username   --  remote user.
 *          BOOL_T is_send_log  -- send log flag.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Set_User_Connection_Send_Log(L_INET_AddrIp_T ip_addr, const char *username, BOOL_T is_send_log);


/* FUNCTION NAME:  HTTP_OM_Get_Next_User_Connection_Info
 * PURPOSE:
 *          This function get next connection info.
 *
 * INPUT:
 *          HTTP_Session_T   *connection_info    --  previous active connection information.
 *          UI32_T                      current_time        --  current time.
 *
 * OUTPUT:
 *          HTTP_Session_T   *connection_info    --  current active connection information.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *      Initial input value is zero.
 */
BOOL_T HTTP_OM_Get_Next_User_Connection_Info(HTTP_Session_T *session, UI32_T current_time);


#if (SYS_CPNT_CLUSTER == TRUE)
/* FUNCTION NAME:  HTTP_OM_Set_Cluster_Port
 * PURPOSE:
 *          This function set cluster port number.
 *
 * INPUT:
 *          UI32_T - Cluster port number.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Set_Cluster_Port(UI32_T port);

/* FUNCTION NAME:  HTTP_OM_Get_Cluster_Port
 * PURPOSE:
 *			This function get cluster port number.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          *port_p -- Cluster port
 *
 * RETURN:
 *          TRUE
 * NOTES:
 *          default is tcp/80.
 */
UI32_T HTTP_OM_Get_Cluster_Port(void);
#endif /* SYS_CPNT_CLUSTER */

/* FUNCTION NAME:  HTTP_OM_ConfigAddAlias
 * PURPOSE:
 *          Add alias that used to map URI to local files beginning with path.
 *
 * INPUT:
 *          const char *uri     -- URI (key), cannot be blank string
 *          const char *path    -- optional, file path or directory path
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          TRUE / FALSE
 *
 * NOTES:
 *          Replace path if uri exist.
 */
BOOL_T HTTP_OM_ConfigAddAlias(const char *uri, const char *path);

/* FUNCTION NAME:  HTTP_OM_ConfigGetNextAlias
 * PURPOSE:
 *          Get next alias.
 *
 * INPUT:
 *          int *index              -- index, use -1 to get first alias.
 *
 * OUTPUT:
 *          int *index              -- current index
 *          HTTP_Alias_T *alias_p   -- alias
 *
 * RETURN:
 *          Return the alias or return FALSE if no more alias.
 *
 * NOTES:
 *          None
 */
BOOL_T HTTP_OM_ConfigGetNextAlias(int *index, HTTP_Alias_T *alias_p);

/* FUNCTION NAME:  HTTP_OM_ConfigDeleteAlias
 * PURPOSE:
 *          Delete alias by uri.
 *
 * INPUT:
 *          const char *uri -- URI
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          TRUE / FALSE
 *
 * NOTES:
 *          Pack up the list after delete entry (speed up the search).
 */
BOOL_T HTTP_OM_ConfigDeleteAlias(const char *uri);

/* FUNCTION NAME:  HTTP_OM_ConfigMapUriToPath
 * PURPOSE:
 *          Map uri to file path.
 *
 * INPUT:
 *          const char *uri -- URI
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          Return file path that allocated by malloc() or
 *          return NULL if operation failed.
 *
 * NOTES:
 *          1. The path for REST API should not be change by alias.
 *          2. All alias path will be replaced.
 *          3. Nonmatch URI will be replaced by root.
 */
char *HTTP_OM_ConfigMapUriToPath(const char *uri);

/* FUNCTION NAME:  HTTP_OM_SetConfigFilePath
 * PURPOSE:
 *          Set config file path.
 *
 * INPUT:
 *          file_path -- the config file path
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          TRUE / FALSE
 *
 * NOTES:
 *          None
 */
BOOL_T HTTP_OM_SetConfigFilePath(const char *file_path);

/* FUNCTION NAME:  HTTP_OM_GetConfigFilePath
 * PURPOSE:
 *          Get config file path.
 *
 * INPUT:
 *          buf     -- string buffer
 *          buf_len -- buffer length
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          TRUE / FALSE
 *
 * NOTES:
 *          None
 */
BOOL_T HTTP_OM_GetConfigFilePath(char *buf, UI32_T buf_len);

#if __cplusplus
}
#endif

#endif /* #ifndef HTTP_OM_H */
