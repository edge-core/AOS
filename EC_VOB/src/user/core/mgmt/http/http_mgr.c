/* MODULE NAME:  http_mgr.c
 * PURPOSE:
 *   Initialize the resource and provide some functions for the http module.
 *
 * NOTES:
 *
 * History:
 *       Date          -- Modifier,  Reason
 *     2002-02         -- Isiah , created.
 *     2007-07         -- Rich Lee, porting to Linux
 * Copyright(C)      Accton Corporation, 2002
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <pthread.h>
#include "http_loc.h"
#include "http_backdoor.h"
#include "http_om_exp.h"

#include "l_md5.h"

#include "backdoor_mgr.h"
#include "buffer_mgr.h"

#if (SYS_CPNT_HTTPS == TRUE)
#include "leaf_es3626a.h"

#include "xfer_mgr.h"
#include "keygen_type.h"
#include "keygen_mgr.h"
#include "keygen_pmgr.h"
#include "ssl_crypto_lock.h"
#include "rand.h"
#endif /* SYS_CPNT_HTTPS */

#include "syslog_type.h"
#include "eh_type.h"
#include "eh_mgr.h"
#include "ip_lib.h"

#include "sys_time.h"
#include "xfer_pmgr.h"
#include "netcfg_pmgr_main.h"
#include "netcfg_pmgr_route.h"
#include "snmp_pmgr.h"
#include "trap_event.h"
#include "sys_callback_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */

/*-----------------------------------------------------------------------*/
#if (SYS_CPNT_HTTPS == TRUE)

/* FUNCTION NAME:  HTTP_MGR_NotifyCliXferResult
 * PURPOSE:
 *          callback to CLI to announce Xfer status
 *
 * INPUT:
 *          void * cookie .
 *          XFER_MGR_FileCopyStatus_T  status
 *
 * OUTPUT:
 *
 * RETURN:
 *
 * NOTES:
 */
static void HTTP_MGR_NotifyCliXferResult (void *cookie, XFER_MGR_FileCopyStatus_T  status);

/* FUNCTION NAME:  HTTP_SSL_Rand_Choosenum
 * PURPOSE:
 *          Choose number between input parameter 1 and parameter 2.
 *
 * INPUT:
 *          UI32_T  -- Lowest number .
 *
 *          UI32_T  -- Highest number.
 *
 * OUTPUT:
 *          UI32_T * - The number between lowest and highest.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
static BOOL_T HTTP_SSL_Rand_Choosenum(UI32_T ,UI32_T ,UI32_T *);

/* FUNCTION NAME:  HTTP_Get_Temp_PublicKey_Pair
 * PURPOSE:
 *          Get 512 and 1024 bit temporary keys
 *
 * INPUT:
 *          SSL *   -- SSL structure.
 *
 *          int     -- nExport,1 to indicate an export cipher is used and 0 to indicate not.
 *
 *          int     -- temporary key length,valid value are 512 and 1024 .
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          RSA * - RSA structure.
 * NOTES:
 *          This API will be registed into SSL library and called from there
 */
RSA *HTTP_Get_Temp_PublicKey_Pair(SSL *, int , int );

/* FUNCTION NAME:  HTTP_Set_SSL_Session_Cache_Entry
 * PURPOSE:
 *          This function is executed by OpenSSL and a new SSL_SESSION is
 *          added to the session cache.
 *
 * INPUT:
 *          SSL *           -- ssl structure.
 *
 *          SSL_SESSION *   -- ssl session.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          0.
 * NOTES:
 *          This API will be registed into SSL library and called from there
 */
int HTTP_Set_SSL_Session_Cache_Entry(SSL *, SSL_SESSION * );

/* FUNCTION NAME:  HTTP_Get_SSL_Session_Cache_Entry
 * PURPOSE:
 *          This function is executed by OpenSSL. We use this
 *          to lookup the SSL_SESSION in the cache where it was perhaps
 *          stored by HTTP_OM_Set_SSL_Session_Cache_Entry function.
 * INPUT:
 *          SSL *   -- SSL structure,
 *
 *          char *  -- Session ID,
 *
 *          int     -- ID length,
 *
 *          int *   -- copy
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          SSL_SESSION * - SSL_Session.
 * NOTES:
 *          This API will be registed into SSL library and called from there
 */
SSL_SESSION *HTTP_Get_SSL_Session_Cache_Entry(SSL *, unsigned char *, int , int * );

/* FUNCTION NAME:  HTTP_Delete_SSL_Session_Cache_Entry
 * PURPOSE:
 *          This function is executed by OpenSSL.
 *          We use this to remove the SSL_SESSION in the cache.
 *
 * INPUT:
 *          SSL_CTX *       -- SSL CTX object.
 *
 *          SSL_SESSION *   -- SSL SESSION.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This API will be registed into SSL library and called from there
 */
void HTTP_Delete_SSL_Session_Cache_Entry(SSL_CTX *, SSL_SESSION *);

/* FUNCTION NAME:  HTTP_Log_Tracing_State
 * PURPOSE:
 *          This function is executed while OpenSSL processes the
 *          SSL handshake and does SSL record layer stuff. We use it to
 *          trace OpenSSL's processing in out SSL logfile.
 * INPUT:
 *          parameter-1  -- argument description included possible value,
 *                                   valid range, notice.
 *          parameter-2  -- argument description included possible value,
 *                                   valid range, notice.
 *          parameter-3  -- argument description included possible value,
 *                                   valid range, notice.
 * OUTPUT:
 *          parameter-n  -- calculation result be returned, describes valid /
 *                                   invalid value, semantic meaning.
 * RETURN:
 *          return-value-1 - describes the meaning.
 * NOTES:
 *          This API will be registed into SSL library and called from there
 */
void HTTP_Log_Tracing_State(const SSL *, int , int );

/* FUNCTION NAME:  HTTP_Get_SSL_Cipher_Bits
 * PURPOSE:
 *          This function to get cipher's key size.
 *
 * INPUT:
 *          SSL *  -- SSL object.
 *
 * OUTPUT:
 *          int * -- usekeysize.
 *          int * -- algkeysize.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
static void HTTP_Get_SSL_Cipher_Bits(const SSL *, int *, int *);

/* FUNCTION NAME:  HTTP_Hook_Function_Into_SSL_CTX
 * PURPOSE:
 *          Register hook function into OpenSSL library.
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
 *          HTTP_Init_SSL_Resources() will call this API to hook MGR APIs into SSL_CTX
 *          .
 */
BOOL_T HTTP_Hook_Function_Into_SSL_CTX();

/* FUNCTION NAME:  HTTP_Seed_SSL_Rand
 * PURPOSE:
 *          Support for better seeding of SSL library's RNG
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T - Number of bytes for seeding PRNG.
 * NOTES:
 *          .
 */
UI32_T HTTP_Seed_SSL_Rand();

static BOOL_T HTTP_MGR_ReadTextFile(const char* path, char* buf, size_t buf_size);

static BOOL_T HTTP_MGR_WriteTextFile(const char* path, const char *buf);

static BOOL_T HTTP_MGR_Get_Https_Certificate(char *cert, UI32_T cert_size);

static BOOL_T HTTP_MGR_Get_Https_Private_Key(char *key, UI32_T key_size);

static BOOL_T HTTP_MGR_Get_Https_Passphrase(char *pass, UI32_T pass_size);

static BOOL_T HTTP_MGR_Set_Server_Certificate(const char *server_certificate, const char *server_private_key, const char *server_pass_phrase);

#endif /* if SYS_CPNT_HTTPS == TRUE */
/*-----------------------------------------------------------------------*/

/* FUNCTION NAME:  HTTP_fingerprint_hex
 * PURPOSE:
 *          Convert digest message to hex format.
 *
 * INPUT:
 *          UI8_T   *dgst_raw       --  original digest message using MD5 or SHA1.
 *          UI32_T  dgst_raw_len    --  length of digest message.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T - Number of bytes for seeding PRNG.
 * NOTES:
 *          .
 */
static UI8_T *HTTP_fingerprint_hex(UI8_T *dgst_raw, UI32_T dgst_raw_len);

static int HTTP_MGR_GetSockPort(struct sockaddr *sa);

/* FUNCTION NAME:  HTTP_MGR_Async_Get_Certificate
 * PURPOSE:
 *          A server certificate authenticates the server to the client.
 *          This function to save server certificate ,
 *          when you got a server certificate from a certification authority(CA).
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
 *          User can use TFTP or XModem to upload server certificate
 *
 *          This function is invoked in HTTP_MGR_Get_Certificate_From_Tftp() or (SNMP).
 */
static BOOL_T HTTP_MGR_Async_Get_Certificate();

/* FUNCTION NAME:  HTTP_MGR_SetConfigFromJson
 * PURPOSE:
 *          Set config from json.
 *
 * INPUT:
 *          config -- the config in json format
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
static void HTTP_MGR_SetConfigFromJson(const json_t *config);

/* FUNCTION NAME:  HTTP_MGR_ResetConfigToDefault
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
static void HTTP_MGR_ResetConfigToDefault();

/* FUNCTION NAME:  HTTP_MGR_SetConfig
 * PURPOSE:
 *          Set config.
 *
 * INPUT:
 *          config  -- the config in json format
 *          key     -- the key in config
 *          fn      -- the function which is used to call for set related config
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
static void HTTP_MGR_SetConfig(const json_t *config, const char *key, void (*fn)(const json_t *));

/* FUNCTION NAME:  HTTP_MGR_SetConfig_RootDir
 * PURPOSE:
 *          Set web root directory.
 *
 * INPUT:
 *          root_dir -- the config for root directory
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          None
 *
 * NOTES:
 *          Only allow string format.
 */
static void HTTP_MGR_SetConfig_RootDir(const json_t *root_dir);

#if (SYS_CPNT_HTTPS == TRUE)
static void HTTP_MGR_SetConfig_Https(const json_t *https);
#endif // SYS_CPNT_HTTPS

/* FUNCTION NAME:  HTTP_MGR_SetConfig_Alias
 * PURPOSE:
 *          Set alias config.
 *
 * INPUT:
 *          alias -- the config for alias
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          None
 *
 * NOTES:
 *          1. Only allow array and each element has uri and path.
 *          2. Path is optional.
 *
 *          e.g. "alias": [ { "uri": "/xxx", "path": "/tmp" } ]
 */
static void HTTP_MGR_SetConfig_Alias(const json_t *config);

/* FUNCTION NAME:  HTTP_MGR_SetConfig_Logger
 * PURPOSE:
 *          Config logger.
 *
 * INPUT:
 *          logger -- the config for logger
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          None
 *
 * NOTES:
 *          Only allow string format.
 */
static void HTTP_MGR_SetConfig_Logger(const json_t *config);

/* FUNCTION NAME:  HTTP_MGR_SetConfig_LogLevel
 * PURPOSE:
 *          Config log level.
 *
 * INPUT:
 *          level -- the config for log level
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          None
 *
 * NOTES:
 *          Allow integer and string format.
 */
static void HTTP_MGR_SetConfig_LogLevel(const json_t *config);

/* FUNCTION NAME:  HTTP_MGR_SetConfig_LogType
 * PURPOSE:
 *          Config log types which you want to log.
 *
 * INPUT:
 *          type -- the config for log type
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          None
 *
 * NOTES:
 *          Allow array and string format.
 */
static void HTTP_MGR_SetConfig_LogType(const json_t *config);


/* STATIC VARIABLE DECLARATIONS
 */
#if (SYS_CPNT_HTTPS == TRUE)
static  UI8_T   server_certificate[SERVER_CERTIFICATE_FILE_LENGTH];
static  UI8_T   server_private_key[SERVER_PRIVATE_KEY_FILE_LENGTH];
static  UI8_T   server_pass_phrase[SERVER_PASS_PHRASE_FILE_LENGTH];
#endif /* #if (SYS_CPNT_HTTPS == TRUE) */
SYSFUN_DECLARE_CSC

#include "mm.h"

static pthread_rwlock_t       rwlock;

static void HTTP_MGR_mm_lock_cb(int flags)
{
    int rc;

    if (flags & MM_LOCK)
    {
        if (flags & MM_READ)
        {
            rc = pthread_rwlock_rdlock(&rwlock);
        }
        else if (flags & MM_WRITE)
        {
            rc = pthread_rwlock_wrlock(&rwlock);
        }
    }
    else if (flags & MM_UNLOCK)
    {
        rc = pthread_rwlock_unlock(&rwlock);
    }
}

static void HTTP_MGR_ssl_dbg_malloc_cb(void *addr, int num, const char *file, int line, int before_p)
{
    if (before_p == 1)
    {
        void *caller_addr = NULL;
        MM_alloc(addr, num, CRYPTO_thread_id(), file, line, caller_addr);
    }
}

static void HTTP_MGR_ssl_dbg_realloc_cb(void *old_addr, void *addr, int num, const char *file, int line, int before_p)
{
    if (before_p == 1)
    {
        void *caller_addr = NULL;

        MM_free(old_addr, CRYPTO_thread_id());
        MM_alloc(addr, num, CRYPTO_thread_id(), file, line, caller_addr);
    }
}

static void HTTP_MGR_ssl_dbg_free_cb(void *addr, int before_p)
{
    if (before_p == 0)
    {
        MM_free(addr, CRYPTO_thread_id());
    }
}

static unsigned long HTTP_MGR_id_callback(void)
{
    return SYSFUN_TaskIdSelf();
}

/* EXPORTED SUBPROGRAM BODIES
 */
void HTTP_MGR_InitiateSystemResources(void)
{
    HTTP_OM_InitateSystemResource();

    HTTP_OM_InitateProcessResource();
    HTTP_OM_CleanAllConnectionObjects();

    http_log_init();

    {
#if (SYS_CPNT_HTTPS == TRUE)
    int rc;
    rc = pthread_rwlock_init(&rwlock, NULL);
    MM_set_lock_function(HTTP_MGR_mm_lock_cb);

    SSL_CRYPTO_LOCK_Create();
    CRYPTO_set_locking_callback(SSL_CRYPTO_LOCK_Lock);

    CRYPTO_set_mem_debug_functions(HTTP_MGR_ssl_dbg_malloc_cb,
        HTTP_MGR_ssl_dbg_realloc_cb,
        HTTP_MGR_ssl_dbg_free_cb,
        NULL,
        NULL);

    CRYPTO_set_id_callback(HTTP_MGR_id_callback);

    /*
     * Registers the available ciphers and digests.
     * Read server certificate,server private key,temporary private key
     * initialize session cache,and hook functions into SSL_CTX.
     */
     SSL_library_init();
     HTTP_Init_SSL_Resources();
#endif /* if SYS_CPNT_HTTPS == TRUE */
    }
}

/* FUNCTION NAME:  HTTP_MGR_Create_InterCSC_Relation
 * PURPOSE:
 *          This function initializes all function pointer registration operations.
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
 *          none.
 */
void HTTP_MGR_Create_InterCSC_Relation(void)
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("HTTP", SYS_BLD_WEB_GROUP_IPCMSGQ_KEY, HTTP_BACKDOOR_Main);
}

/* FUNCTION NAME:  HTTP_MGR_Enter_Master_Mode
 * PURPOSE:
 *          This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the HTTP subsystem will enter the
 *          Master Operation mode.
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
 *          1. If "System Boot Configruation File" does not exist, the system database and
 *              switch will be initiated to the factory default value.
 *          2. HTTP will handle network requests only when this subsystem
 *              is in the Master Operation mode
 *          3. This function is invoked in HTTP_INIT_EnterMasterMode.
 */
BOOL_T HTTP_MGR_Enter_Master_Mode(void)
{
    /* init database */
    HTTP_OM_Set_Http_Status (HTTP_STATE_ENABLED);
    HTTP_OM_Set_Http_Port (HTTP_DEFAULT_PORT_NUMBER);
/*isiah.2003-06-12. add for show web connection.*/
    HTTP_OM_Clear_User_Connection_Info();

/*Isiah.2002-03-05*/
#if (SYS_CPNT_HTTPS == TRUE)

    HTTP_OM_Set_Secure_Port(HTTP_DEFAULT_SECURE_PORT_NUMBER);
    HTTP_OM_Set_Secure_Http_Status(SECURE_HTTP_STATE_ENABLED);
    HTTP_OM_Set_Certificate_Status(TRUE);
#endif /* if SYS_CPNT_HTTPS == TRUE */

    {
        char config_file_path[HTTP_CONFIG_FILE_PATH_MAX_LEN + 1];

        if (FALSE == HTTP_OM_GetConfigFilePath(config_file_path, sizeof(config_file_path)))
        {
            printf("error: failed to get config file path\r\n");
        }
        else
        {
            HTTP_MGR_LoadConfig(config_file_path);
        }
    }

    /* set mgr in master mode */
    SYSFUN_ENTER_MASTER_MODE();

    return TRUE;
}

/* FUNCTION NAME:  HTTP_MGR_Enter_Transition_Mode
 * PURPOSE:
 *          This function forces this subsystem enter the Transition Operation mode.
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
BOOL_T HTTP_MGR_Enter_Transition_Mode(void)
{
    /* set mgr in transition mode */
    SYSFUN_ENTER_TRANSITION_MODE();

    HTTP_OM_Set_Http_Status(HTTP_STATE_DISABLED);

#if (SYS_CPNT_HTTPS == TRUE)
    HTTP_OM_Set_Secure_Http_Status(SECURE_HTTP_STATE_DISABLED);
#endif /* if SYS_CPNT_HTTPS == TRUE */
    return TRUE;
}

/* FUNCTION NAME:  HTTP_MGR_Enter_Slave_Mode
 * PURPOSE:
 *          This function forces this subsystem enter the Slave Operation mode.
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
 *          In Slave Operation mode, any network requests
 *          will be ignored.
 */
void HTTP_MGR_Enter_Slave_Mode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();

    return;
}

/* FUNCTION NAME : HTTP_MGR_SetTransitionMode
 * PURPOSE:
 *      Set transition mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void HTTP_MGR_SetTransitionMode(void)
{
    /* set transition flag to prevent calling request */
    SYSFUN_SET_TRANSITION_MODE();
}

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
/* FUNCTION NAME : HTTP_MGR_MgmtIPFltChanged_Callback
 * PURPOSE:
 *      Process when the database of management IP filter was changed
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void HTTP_MGR_MgmtIPFltChanged_Callback()
{
    HTTP_Session_T   sess;

    memset(&sess, 0, sizeof(sess));

    /* If the connection is not valid for mgmt ip filter, then remove it
     */
    while (TRUE == HTTP_OM_Get_Next_User_Connection_Info(&sess,
                                                         SYSFUN_GetSysTick()))
    {
        if (FALSE == MGMT_IP_FLT_IsValidIpFilterAddress(MGMT_IP_FLT_WEB,
                                                        &sess.remote_ip))
        {
            HTTP_OM_Delete_User_Connection_Info(sess.session_id);
            memset(&sess, 0, sizeof(sess));
        }
    }
}
#endif /* #if (SYS_CPNT_MGMT_IP_FLT == TRUE) */

/* FUNCTION NAME:  HTTP_MGR_Set_Root_Dir
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
BOOL_T HTTP_MGR_Set_Root_Dir(const char *dir)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    return HTTP_OM_Set_Root_Dir(dir);
}

/* FUNCTION NAME:  HTTP_MGR_Get_Root_Dir
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
BOOL_T HTTP_MGR_Get_Root_Dir(char *buf, size_t bufsz)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    return HTTP_OM_Get_Root_Dir(buf, bufsz);
}

/* FUNCTION NAME:  HTTP_MGR_Set_Http_Status
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
BOOL_T HTTP_MGR_Set_Http_Status (HTTP_STATE_T state)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {

        HTTP_OM_Set_Http_Status(state);
    }

    return TRUE;
}

/* FUNCTION NAME:  HTTP_MGR_Get_Http_Status
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
HTTP_STATE_T HTTP_MGR_Get_Http_Status()
{
    HTTP_STATE_T    state = HTTP_STATE_DISABLED;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        state = HTTP_OM_Get_Http_Status();
    }

    return ( state );
}

/* FUNCTION NAME:  HTTP_MGR_Set_Http_Port
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
 *          none.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Set_Http_Port (UI32_T port)
{
    UI32_T  old_port;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if( port<1 || port>65535 )
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_HTTP, HTTP_MGR_Set_Http_Port_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "HTTP port number(1-65535)");
            return FALSE;
        }

        /* if same port, ignore
         */
        old_port = HTTP_OM_Get_Http_Port();

        if ( old_port == port )
        {
            return TRUE;
        }

        if (TRUE != IP_LIB_IsValidSocketPortForServerListen((UI16_T)port, IP_LIB_SKTTYPE_TCP, IP_LIB_CHKAPP_HTTP))
        {
            return FALSE;
        }

       /*  ES4549-08-00505 - to avoid user setting a port
           which is used by other components.
        */
        if(NETCFG_PMGR_MAIN_IsEmbeddedTcpPort(port))
        {
          return FALSE;
        }

        HTTP_OM_Set_Http_Port(port);

        /* Here is a patch code to set cluster port directly.
         * The cluster port shall be configured from UI not invoke self MGR.
         */
#if (SYS_CPNT_CLUSTER == TRUE)
        HTTP_OM_Set_Cluster_Port(port);
#endif /* SYS_CPNT_CLUSTER */

        /* close socket
         */
        HTTP_TASK_Port_Changed ();
        return TRUE;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_Http_Port
 * PURPOSE:
 *          This function get http port number.
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
UI32_T HTTP_MGR_Get_Http_Port()
{
    UI32_T  port = 0;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
       port = HTTP_OM_Get_Http_Port();
    }

    return port;
}

/* FUNCTION NAME : HTTP_MGR_GetOperationMode
 * PURPOSE:
 *      Get current http operation mode (master / slave / transition).
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *          SYS_TYPE_Stacking_Mode_T - opmode.
 *
 * NOTES:
 *      None.
 */
SYS_TYPE_Stacking_Mode_T HTTP_MGR_GetOperationMode(void)
{
    return ( SYSFUN_GET_CSC_OPERATING_MODE() );
}

/* FUNCTION NAME:  HTTP_MGR_GetRunningHttpParameters
 * PURPOSE:
 *          This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
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
UI32_T HTTP_MGR_GetRunningHttpParameters(HTTP_MGR_RunningCfg_T *http_cfg)
{

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {

        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        http_cfg->port  =   HTTP_MGR_Get_Http_Port();
        http_cfg->state =   HTTP_MGR_Get_Http_Status();
        http_cfg->port_changed  =   FALSE;
        http_cfg->state_changed =   FALSE;

        if (http_cfg->state != HTTP_DEFAULT_STATE)
        {
            http_cfg->state_changed =   TRUE;
        }
        if (http_cfg->port != HTTP_DEFAULT_PORT_NUMBER)
        {
            http_cfg->port_changed  =   TRUE;
        }

        if (http_cfg->port_changed || http_cfg->state_changed)
        {

            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
        else
        {

            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        }
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_Running_Http_Port
 * PURPOSE:
 *          This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific http port with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T * - HTTP port number.
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
SYS_TYPE_Get_Running_Cfg_T  HTTP_MGR_Get_Running_Http_Port(UI32_T *port)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        *port = HTTP_MGR_Get_Http_Port();
        if ( *port != HTTP_DEFAULT_PORT_NUMBER )
        {

            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS ;
        }

        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_Running_Http_Status
 * PURPOSE:
 *          This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific http state with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T * - HTTP state.
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
SYS_TYPE_Get_Running_Cfg_T  HTTP_MGR_Get_Running_Http_Status(UI32_T *status)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {

        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        *status = HTTP_MGR_Get_Http_Status();
        if ( *status != HTTP_DEFAULT_STATE )
        {

            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS ;
        }

        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}

/* FUNCTION NAME:  HTTP_MGR_AllocateConnectionObject
 * PURPOSE:
 *          Allocate a connection object
 *
 * INPUT:
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
void * HTTP_MGR_AllocateConnectionObject(int sockfd)
{
    UI32_T tid = SYSFUN_TaskIdSelf();

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return NULL;
    }

    return HTTP_OM_AllocateConnectionObject(tid, sockfd);
}

/* FUNCTION NAME:  HTTP_MGR_FreeConnectionObject
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
BOOL_T HTTP_MGR_FreeConnectionObject(HTTP_Connection_T *conn_obj_p)
{
/* don't check operating mode here, because it may called in transition mode
 */

/*
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
*/
    return HTTP_OM_FreeConnectionObject(conn_obj_p);
}

/* FUNCTION NAME:  HTTP_MGR_GetNextConnectionObjectAtIndex
 * PURPOSE:
 *          Enumerates all connection objects
 *
 * INPUT:
 *          index  -- index. Use 0xffffffff to get first object.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          Connection object
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_GetNextConnectionObjectAtIndex(UI32_T *index, HTTP_Connection_T *http_connection)
{
    UI32_T tid = SYSFUN_TaskIdSelf();

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    return HTTP_OM_GetNextConnectionObjectAtIndex(tid, index, http_connection);
}

#if (SYS_CPNT_HTTPS == TRUE)
/* FUNCTION NAME:  HTTP_MGR_Set_Secure_Port
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
BOOL_T HTTP_MGR_Set_Secure_Port (UI32_T port)
{
    UI32_T  secure_port;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if( port<1 || port>65535 )
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_HTTP, HTTP_MGR_Set_Secure_Port_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "HTTPS port number(1-65535)");

            return FALSE;
        }

        HTTP_OM_Get_Secure_Port(&secure_port);

        /* if same port, ignore
         */
        if (port == secure_port)
        {
            return TRUE;
        }

        if (TRUE != IP_LIB_IsValidSocketPortForServerListen((UI16_T)port, IP_LIB_SKTTYPE_TCP, IP_LIB_CHKAPP_HTTPS))
        {
            return FALSE;
        }

       /*  ES4549-08-00505 - to avoid user setting a port
           which is used by other components.
        */
        if(NETCFG_PMGR_MAIN_IsEmbeddedTcpPort(port))
        {
          return FALSE;
        }

        HTTP_OM_Set_Secure_Port(port);

        /* close socket
         */
        HTTP_TASK_Secure_Port_Changed();

        return TRUE;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_Secure_Port
 * PURPOSE:
 *          This function get security port number.
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
BOOL_T HTTP_MGR_Get_Secure_Port(UI32_T *port)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        HTTP_OM_Get_Secure_Port(port);
    }

    return TRUE;
}

/* FUNCTION NAME:  HTTP_MGR_Set_Secure_Http_Status
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
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Set_Secure_Http_Status (SECURE_HTTP_STATE_T state)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        HTTP_OM_Set_Secure_Http_Status(state);
    }

    return TRUE;
}

/* FUNCTION NAME:  HTTP_MGR_Get_Secure_Http_Status
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
SECURE_HTTP_STATE_T HTTP_MGR_Get_Secure_Http_Status()
{
    SECURE_HTTP_STATE_T state = SECURE_HTTP_STATE_DISABLED;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        HTTP_OM_Get_Secure_Http_Status(&state);
    }

    return ( state );
}

/* FUNCTION NAME:  HTTP_MGR_Get_SSL_CTX
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
 *          void * - SSL_CTX object.
 * NOTES:
 *          .
 */
void *HTTP_MGR_Get_SSL_CTX()
{
    SSL_CTX *get_ssl_ctx = NULL;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        get_ssl_ctx = HTTP_OM_Get_SSL_CTX();
    }

    return (void *)get_ssl_ctx;
}

/* FUNCTION NAME:  HTTP_MGR_XferCopy_Callback
 * PURPOSE:
 *          when Xfer callback, write back to buffer
 * INPUT:
 *          cookie BIT_0: certificate 0x8000
 *                 BIT_1: private key 0x4000
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *
 * NOTES:
 *
 */
void HTTP_MGR_XferCopy_Callback(void *cookie, UI32_T status)
{
    switch(status)
    {
        case XFER_MGR_FILE_COPY_SUCCESS:
            HTTP_MGR_Set_XferProgressResult(TRUE);
            break;

        case XFER_MGR_FILE_COPY_COMPLETED:
            HTTP_MGR_Set_XferProgressStatus(FALSE);
            break;

        case XFER_MGR_FILE_COPY_ERROR:
            HTTP_MGR_Set_XferProgressResult(FALSE);
            break;

        default:
            break;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_Running_Http_Secure_Port
 * PURPOSE:
 *          This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific http secure port with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T * - HTTPS port number.
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
SYS_TYPE_Get_Running_Cfg_T  HTTP_MGR_Get_Running_Http_Secure_Port(UI32_T *port)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        HTTP_MGR_Get_Secure_Port(port);

        if ( *port != HTTP_DEFAULT_SECURE_PORT_NUMBER )
        {
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS ;
        }

        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_Running_Secure_Http_Status
 * PURPOSE:
 *          This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific https state with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T * - HTTPS state.
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
SYS_TYPE_Get_Running_Cfg_T  HTTP_MGR_Get_Running_Secure_Http_Status(UI32_T *status)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        *status = HTTP_MGR_Get_Secure_Http_Status();

        if ( *status != HTTP_DEFAULT_SECURE_HTTP_STATE )
        {
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS ;
        }

        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_Running_SSL_Session_Cache_Timeout
 * PURPOSE:
 *          This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific SSL session cache timeout with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T * - SSL session cache timeout.
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
SYS_TYPE_Get_Running_Cfg_T  HTTP_MGR_Get_Running_SSL_Session_Cache_Timeout(UI32_T *timeout)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        HTTP_OM_Get_SSL_Session_Cache_Timeout(timeout);

        if ( *timeout != HTTP_DEFAULT_SESSION_CACHE_TIMEOUT )
        {
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS ;
        }

        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_Certificate_Status
 * PURPOSE:
 *          This function to get flag of certificate status.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate certificate have changed;
 *          FALSE to indicate not changed.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Get_Certificate_Status()
{
    BOOL_T is_changed;

    if (FALSE == HTTP_OM_Get_Certificate_Status(&is_changed))
    {
        return FALSE;
    }

    return is_changed;
}

/* FUNCTION NAME:  HTTP_MGR_Get_Certificate
 * PURPOSE:
 *          This function to read server certificate and private key to ssl_ctx.
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
BOOL_T HTTP_MGR_Get_Certificate()
{
    SSL_CTX *ssl_ctx;
    UI8_T   *cert_buf;
    UI8_T   *key_buf;
    UI8_T   *passwd_buf;
    BOOL_T  change_cert;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        HTTP_OM_Get_Certificate_Status(&change_cert);
        HTTP_OM_Set_Certificate_Status(FALSE);

        if( FALSE == change_cert )
        {
            return TRUE;
        }
        cert_buf = (UI8_T *) L_MM_Malloc(SERVER_CERTIFICATE_FILE_LENGTH, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_HTTP_MGR_GET_CERTIFICATE));
        if ( cert_buf == NULL )
        {

            return FALSE;
        }

        ssl_ctx = HTTP_OM_Get_SSL_CTX();

        /*
         * Read server certificate
         */
        HTTP_MGR_Get_Https_Certificate((char*)cert_buf, SERVER_CERTIFICATE_FILE_LENGTH);

        {
            BIO *in;
            int ret=0;
            X509 *x=NULL;

            in=BIO_new_mem_buf(cert_buf, -1);

            x=PEM_read_bio_X509(in,NULL,ssl_ctx->default_passwd_callback,ssl_ctx->default_passwd_callback_userdata);
            ret=SSL_CTX_use_certificate(ssl_ctx,x);
            if (x != NULL) X509_free(x);
            if (in != NULL) BIO_free(in);

            if (ret <= 0)
            {
                L_MM_Free(cert_buf);
                EH_MGR_Handle_Exception1(SYS_MODULE_SSL, HTTP_MGR_Get_Certificate_FUNC_NO, EH_TYPE_MSG_FAILED_TO, SYSLOG_LEVEL_INFO, "decrypt certificate file");

                return FALSE;
            }
        }

        L_MM_Free(cert_buf);

        /*
         * Read server private key
         */
        key_buf = (UI8_T *) L_MM_Malloc(SERVER_PRIVATE_KEY_FILE_LENGTH, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_HTTP_MGR_GET_CERTIFICATE));
        if ( key_buf == NULL )
        {

            return FALSE;
        }
        passwd_buf = (UI8_T *) L_MM_Malloc(SERVER_PASS_PHRASE_FILE_LENGTH, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_HTTP_MGR_GET_CERTIFICATE));
        if ( passwd_buf == NULL )
        {
            L_MM_Free(key_buf);

            return FALSE;
        }

        HTTP_MGR_Get_Https_Private_Key((char*)key_buf, SERVER_PRIVATE_KEY_FILE_LENGTH);
        HTTP_MGR_Get_Https_Passphrase((char*)passwd_buf, SERVER_PASS_PHRASE_FILE_LENGTH);

        ssl_ctx->default_passwd_callback_userdata = passwd_buf;

        {
            int ret=0;
            BIO *in;
            EVP_PKEY *pkey=NULL;

            in=BIO_new_mem_buf(key_buf, -1);
            if (in == NULL)
            {
            }

            {
                pkey=PEM_read_bio_PrivateKey(in,NULL,
                    ssl_ctx->default_passwd_callback,ssl_ctx->default_passwd_callback_userdata);
            }

            if (pkey != NULL)
            {
                ret=SSL_CTX_use_PrivateKey(ssl_ctx,pkey);
            }

            if (pkey) EVP_PKEY_free(pkey);
            if (in != NULL) BIO_free(in);

            if (ret <= 0)
            {
                L_MM_Free(key_buf);
                L_MM_Free(passwd_buf);
                EH_MGR_Handle_Exception1(SYS_MODULE_SSL, HTTP_MGR_Get_Certificate_FUNC_NO, EH_TYPE_MSG_FAILED_TO, SYSLOG_LEVEL_INFO, "decrypt  private key file");

                return FALSE;
            }
        }

        if ( !SSL_CTX_check_private_key(ssl_ctx) )
        {
            L_MM_Free(key_buf);
            L_MM_Free(passwd_buf);
            EH_MGR_Handle_Exception1(SYS_MODULE_SSL, HTTP_MGR_Get_Certificate_FUNC_NO, EH_TYPE_MSG_FAILED_TO, SYSLOG_LEVEL_INFO, "verify private key");

            return FALSE;
        }

        L_MM_Free(key_buf);
        L_MM_Free(passwd_buf);

        return TRUE;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Enable_Debug_Information
 * PURPOSE:
 *          This function to enable generate debug information .
 *
 * INPUT:
 *          HTTP_MGR_DebugType_T - Debug type.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Enable_Debug_Information(HTTP_MGR_DebugType_T debug_type)
{
    HTTPS_DEBUG_SESSION_STATE_T is_debug_session;
    HTTPS_DEBUG_STATE_STATE_T   is_debug_state;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if ( debug_type & HTTP_MGR_DEBUG_SESSION )
        {
            is_debug_session = HTTPS_DEBUG_SESSION_STATE_ENABLED;
        }
        if ( debug_type & HTTP_MGR_DEBUG_STATE )
        {
            is_debug_state = HTTPS_DEBUG_STATE_STATE_ENABLED;
        }

        HTTP_OM_Set_Debug_Status(is_debug_session,is_debug_state);

        return TRUE;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Disable_Debug_Information
 * PURPOSE:
 *          This function to disable generate debug information .
 *
 * INPUT:
 *          HTTP_MGR_DebugType_T - Debug type.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Disable_Debug_Information(HTTP_MGR_DebugType_T debug_type)
{
    HTTPS_DEBUG_SESSION_STATE_T is_debug_session;
    HTTPS_DEBUG_STATE_STATE_T   is_debug_state;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if ( debug_type & HTTP_MGR_DEBUG_SESSION )
        {
            is_debug_session = HTTPS_DEBUG_SESSION_STATE_DISABLED;
        }
        if ( debug_type & HTTP_MGR_DEBUG_STATE )
        {
            is_debug_state = HTTPS_DEBUG_STATE_STATE_DISABLED;
        }

        HTTP_OM_Set_Debug_Status(is_debug_session,is_debug_state);
        return TRUE;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_Running_Debug_Information_Status
 * PURPOSE:
 *          This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific Debug type with non-disable values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          HTTP_MGR_DebugType_T * - debug type.
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
// TODO: Is this function really used ??
SYS_TYPE_Get_Running_Cfg_T  HTTP_MGR_Get_Running_Debug_Information_Status(HTTP_MGR_DebugType_T *debug_type)
{
    HTTPS_DEBUG_SESSION_STATE_T is_debug_session;
    HTTPS_DEBUG_STATE_STATE_T   is_debug_state;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        *debug_type = (HTTP_MGR_DebugType_T) 0;

        HTTP_OM_Get_Debug_Status(&is_debug_session,&is_debug_state);

        if ( is_debug_session == HTTPS_DEBUG_SESSION_STATE_ENABLED )
        {
            *debug_type = (HTTP_MGR_DebugType_T) (*debug_type | HTTP_MGR_DEBUG_SESSION);
        }

        if ( is_debug_state == HTTPS_DEBUG_STATE_STATE_ENABLED )
        {
            *debug_type = (HTTP_MGR_DebugType_T) (*debug_type | HTTP_MGR_DEBUG_STATE);
        }

        if ( *debug_type == 0 )
        {
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        }

        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_Debug_Session_Status
 * PURPOSE:
 *          This function to get debug session status .
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          HTTPS_DEBUG_SESSION_STATE_T - Debug Session status.
 * NOTES:
 *          .
 */
HTTPS_DEBUG_SESSION_STATE_T HTTP_MGR_Get_Debug_Session_Status()
{
    HTTPS_DEBUG_SESSION_STATE_T is_debug_session = HTTPS_DEBUG_SESSION_STATE_DISABLED;
    HTTPS_DEBUG_STATE_STATE_T  is_debug_state;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        HTTP_OM_Get_Debug_Status(&is_debug_session,&is_debug_state);
    }

    return is_debug_session;
}

/* FUNCTION NAME:  HTTP_MGR_Get_Debug_State_Status
 * PURPOSE:
 *          This function to get debug state status .
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          HTTPS_DEBUG_STATE_STATE_T - Debug State status.
 * NOTES:
 *          .
 */
HTTPS_DEBUG_STATE_STATE_T HTTP_MGR_Get_Debug_State_Status()
{
    HTTPS_DEBUG_SESSION_STATE_T is_debug_session;
    HTTPS_DEBUG_STATE_STATE_T   is_debug_state = HTTPS_DEBUG_STATE_STATE_DISABLED;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        HTTP_OM_Get_Debug_Status(&is_debug_session,&is_debug_state);
    }

    return is_debug_state;
}

/* FUNCTION NAME : HTTP_MGR_Get_Certificate_Info
 * PURPOSE:
 *      Get HTTPS Certificate Info.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      HTTP_CertificateInfo_T  * -- HTTPS certificate info.
 *
 * RETURN:
 *      TRUE  - The output value is https certifiacte info.
 *      FALSE - The output value is invalid.
 *
 * NOTES:
 *      This function invoked in UI.
 */
BOOL_T HTTP_MGR_Get_Certificate_Info(HTTP_CertificateInfo_T *info)
{
    UI8_T       *cert_buf;
    BIO         *in;
    SSL_CTX     *ssl_ctx;
    X509        *server_cert=NULL;
    UI8_T       *str = NULL;
    UI8_T       *temp, *temp1;
    ASN1_TIME   *valid_start;
    ASN1_TIME   *valid_end;
    UI32_T      str_len;
    UI32_T      i=0,j=0;
    UI8_T       *start,*end;
    const       EVP_MD *md = NULL;
    UI8_T       *dgst_raw = NULL;
    unsigned int dgst_raw_length = 0;
    UI8_T       *retval;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if ( info == NULL )
        {
            return FALSE;
        }

        cert_buf = (UI8_T *)L_MM_Malloc(SERVER_CERTIFICATE_FILE_LENGTH, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_HTTP_MGR_GET_CERTIFICATE_INFO));
        if( cert_buf == NULL )
        {
            return FALSE;
        }

        ssl_ctx = HTTP_OM_Get_SSL_CTX();

        KEYGEN_PMGR_GetServerCertificate(cert_buf);

        in=BIO_new_mem_buf(cert_buf, -1);
        if (in == NULL)
        {
    	    L_MM_Free(cert_buf);

            return FALSE;
        }

        server_cert=PEM_read_bio_X509(in,NULL,ssl_ctx->default_passwd_callback,ssl_ctx->default_passwd_callback_userdata);
        if (server_cert == NULL)
        {
            L_MM_Free(cert_buf);
            BIO_free(in);

            return FALSE;
        }

        str = (UI8_T *)X509_NAME_oneline(X509_get_subject_name(server_cert), 0, 0);
        if( str == NULL )
        {
            L_MM_Free(cert_buf);
            X509_free (server_cert);
            BIO_free(in);

            return FALSE;
        }

        temp = str;
        str_len = strlen((const char *)str);
        i=0;
        j=0;
        for( i=0 ; i<str_len ; i++,temp++ )
        {
            if( strncmp((const char *)temp, "/CN=", 4) == 0 )
            {
                i = i+4;
                temp = temp+4;
                temp1 = temp;
                for( j=i ; j<str_len ; j++,temp1++ )
                {
                    if( strncmp((const char *)temp1, "/Email=", 7) == 0 )
                    {
                        break;
                    }
                }
                break;
            }
        }

        if( j < i )
        {
            temp = str;
            str_len = strlen((const char *)str);
            i=0;
            j=0;
            for( i=0 ; i<str_len ; i++,temp++ )
            {
                if( strncmp((const char *)temp, "/OU=", 4) == 0 )
                {
                    i = i+4;
                    temp = temp+4;
                    temp1 = temp;
                    for( j=i ; j<str_len ; j++,temp1++ )
                    {
                        if( strncmp((const char *)temp1, "/Email=", 7) == 0 )
                        {
                            break;
                        }
                    }
                    break;
                }
            }
        }

        if( j < i )
        {
            temp = str;
            str_len = strlen((const char *)str);
            i=0;
            j=0;
            for( i=0 ; i<str_len ; i++,temp++ )
            {
                if( strncmp((const char *)temp, "/O=", 3) == 0 )
                {
                    i = i+3;
                    temp = temp+3;
                    temp1 = temp;
                    for( j=i ; j<str_len ; j++,temp1++ )
                    {
                        if( strncmp((const char *)temp1, "/Email=", 7) == 0 )
                        {
                            break;
                        }
                    }
                    break;
                }
            }
        }

        memset(info->subject, 0, HTTP_DEFAULT_SUBJECT_NAME_LEN+1);

        if( j > i )
        {
            strncpy( (char *)info->subject, (const char *)temp, j-i );
        }

        OPENSSL_free(str);
        str = NULL;

        str = (UI8_T *)X509_NAME_oneline(X509_get_issuer_name(server_cert), 0, 0);

        if( str == NULL )
        {
            L_MM_Free(cert_buf);
            X509_free (server_cert);
            BIO_free(in);

            return FALSE;
        }

        temp = str;
        str_len = strlen((const char *)str);
        i=0;
        j=0;

        for( i=0 ; i<str_len ; i++,temp++ )
        {
            if( strncmp((const char *)temp, "/CN=", 4) == 0 )
            {
                i = i+4;
                temp = temp+4;
                temp1 = temp;
                for( j=i ; j<str_len ; j++,temp1++ )
                {
                    if( strncmp((const char *)temp1, "/Email=", 7) == 0 )
                    {
                        break;
                    }
                }
                break;
            }
        }

        if( j < i )
        {
            temp = str;
            str_len = strlen((const char *)str);
            i=0;
            j=0;
            for( i=0 ; i<str_len ; i++,temp++ )
            {
                if( strncmp((const char *)temp, "/OU=", 4) == 0 )
                {
                    i = i+4;
                    temp = temp+4;
                    temp1 = temp;
                    for( j=i ; j<str_len ; j++,temp1++ )
                    {
                        if( strncmp((const char *)temp1, "/Email=", 7) == 0 )
                        {
                            break;
                        }
                    }
                    break;
                }
            }
        }

        if( j < i )
        {
            temp = str;
            str_len = strlen((const char *)str);
            i=0;
            j=0;
            for( i=0 ; i<str_len ; i++,temp++ )
            {
                if( strncmp((const char *)temp, "/O=", 3) == 0 )
                {
                    i = i+3;
                    temp = temp+3;
                    temp1 = temp;
                    for( j=i ; j<str_len ; j++,temp1++ )
                    {
                        if( strncmp((const char *)temp1, "/Email=", 7) == 0 )
                        {
                            break;
                        }
                    }
                    break;
                }
            }
        }

        memset(info->issuer, 0, HTTP_DEFAULT_ISSUER_NAME_LEN+1);

        if( j > i )
        {
            strncpy( (char *)info->issuer, (const char *)temp, j-i );
        }

        OPENSSL_free(str);
        str = NULL;

        valid_start = X509_get_notBefore(server_cert);
        start = valid_start->data;

        if (valid_start->type == V_ASN1_UTCTIME)
        {
            sprintf((char *)info->valid_begin, "%s%c%c/%c%c/%c%c",
                (start[0] == '9' ? "19" : "20"),
                start[0], start[1], start[2], start[3], start[4], start[5]);
        }
        else /* V_ASN1_GENERALIZEDTIME */
        {
            sprintf((char *)info->valid_begin, "%c%c%c%c/%c%c/%c%c",
                start[0], start[1], start[2], start[3], start[4], start[5], start[6], start[7]);
        }

        valid_end = X509_get_notAfter(server_cert);
        end = valid_end->data;

        if (valid_start->type == V_ASN1_UTCTIME)
        {
            sprintf((char *)info->valid_end, "%s%c%c/%c%c/%c%c",
                (end[0] == '9' ? "19" : "20"),
                end[0], end[1], end[2], end[3], end[4], end[5]);
        }
        else /* V_ASN1_GENERALIZEDTIME */
        {
            sprintf((char *)info->valid_end, "%c%c%c%c/%c%c/%c%c",
                end[0], end[1], end[2], end[3], end[4], end[5], end[6], end[7]);
        }

    	dgst_raw = (UI8_T *)L_MM_Malloc(EVP_MAX_MD_SIZE, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_HTTP_MGR_GET_CERTIFICATE_INFO));
        if( dgst_raw == NULL )
        {
            L_MM_Free(cert_buf);
            X509_free (server_cert);
            BIO_free(in);

            return FALSE;
        }

        md = EVP_sha1();
        X509_digest(server_cert, md, dgst_raw, &dgst_raw_length);
        retval = (UI8_T *)HTTP_fingerprint_hex(dgst_raw, dgst_raw_length);
        strcpy((char *)info->sha1_fingerprint, (const char *)retval);
    	L_MM_Free(retval);

        memset(dgst_raw, 0, EVP_MAX_MD_SIZE);
        md = EVP_md5();
        X509_digest(server_cert, md, dgst_raw, &dgst_raw_length);
        retval = (UI8_T *)HTTP_fingerprint_hex(dgst_raw, dgst_raw_length);
        strcpy((char *)info->md5_fingerprint, (const char *)retval);
    	L_MM_Free(retval);
    	L_MM_Free(dgst_raw);
        L_MM_Free(cert_buf);
        X509_free (server_cert);
        BIO_free(in);
    }

    return TRUE;
}

/* FUNCTION NAME:  HTTP_MGR_Set_Auto_Redirect_To_Https_Status
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
BOOL_T HTTP_MGR_Set_Auto_Redirect_To_Https_Status(HTTP_Redirect_Status_T status)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        HTTP_OM_Set_Auto_Redirect_To_Https_Status(status);
    }

    return TRUE;
}

/* FUNCTION NAME:  HTTP_MGR_Get_Auto_Redirect_To_Https_Status
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
BOOL_T HTTP_MGR_Get_Auto_Redirect_To_Https_Status(HTTP_Redirect_Status_T *status)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        HTTP_OM_Get_Auto_Redirect_To_Https_Status(status);

        return TRUE;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_Running_Auto_Redirect_To_Https_Status
 * PURPOSE:
 *          This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific https state with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          HTTP_Redirect_Status_T *status   --  http redirect to https status.
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
SYS_TYPE_Get_Running_Cfg_T  HTTP_MGR_Get_Running_Auto_Redirect_To_Https_Status(HTTP_Redirect_Status_T *status)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        HTTP_MGR_Get_Auto_Redirect_To_Https_Status(status);

        if ( *status != HTTP_DEFAULT_REDIRECT_HTTP_STATE )
        {
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS ;
        }

        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Set_Tftp_Ip
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
BOOL_T HTTP_MGR_Set_Tftp_Ip(L_INET_AddrIp_T *tftp_server)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if( tftp_server == NULL )
        {
            return FALSE;
        }

        ret = HTTP_OM_Set_Tftp_Ip(tftp_server);

        return ret;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_Tftp_Ip
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
BOOL_T HTTP_MGR_Get_Tftp_Ip(L_INET_AddrIp_T *tftp_server)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = HTTP_OM_Get_Tftp_Ip(tftp_server);

        return ret;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Set_Tftp_Certificate_Filename
 * PURPOSE:
 *          This function set certificate flename to be download from tftp server.
 *
 * INPUT:
 *          char   *tftp_cert_file --  certificate filename.
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
BOOL_T HTTP_MGR_Set_Tftp_Certificate_Filename(const char *tftp_cert_file)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if (tftp_cert_file == NULL)
        {
            return FALSE;
        }

        if (strlen(tftp_cert_file) > MAXSIZE_tftpSrcFile)
        {
            return FALSE;
        }

        ret = HTTP_OM_Set_Tftp_Certificate_Filename(tftp_cert_file);

        return ret;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_Tftp_Certificate_Filename
 * PURPOSE:
 *          This function get certificate flename to be download from tftp server.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          char   *tftp_cert_file --  certificate filename.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Get_Tftp_Certificate_Filename(char *tftp_cert_file)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if (tftp_cert_file == NULL)
        {
            return FALSE;
        }

        ret = HTTP_OM_Get_Tftp_Certificate_Filename(tftp_cert_file);

        return ret;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Set_Tftp_Privatekey_Filename
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
BOOL_T HTTP_MGR_Set_Tftp_Privatekey_Filename(const char *tftp_private_file)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if (tftp_private_file == NULL)
        {
            return FALSE;
        }
        if (strlen(tftp_private_file) > MAXSIZE_tftpSrcFile)
        {
            return FALSE;
        }

        ret = HTTP_OM_Set_Tftp_Privatekey_Filename(tftp_private_file);

        return ret;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_Tftp_Privatekey_Filename
 * PURPOSE:
 *          This function get privatekey flename to be download from tftp server.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          char   *tftp_private_file  --  privatekey filename.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Get_Tftp_Privatekey_Filename(char *tftp_private_file)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if (tftp_private_file == NULL)
        {
            return FALSE;
        }

        ret = HTTP_OM_Get_Tftp_Privatekey_Filename(tftp_private_file);

        return ret;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Set_Tftp_Privatekey_Password
 * PURPOSE:
 *          This function set privatekey password to be download from tftp server.
 *
 * INPUT:
 *          const char *tftp_private_password  --  privatekey password.
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
BOOL_T HTTP_MGR_Set_Tftp_Privatekey_Password(const char *tftp_private_password)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if (tftp_private_password == NULL)
        {
            return FALSE;
        }

        if (strlen(tftp_private_password) > 20)
        {
            return FALSE;
        }

        ret = HTTP_OM_Set_Tftp_Privatekey_Password(tftp_private_password);

        return ret;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_Tftp_Privatekey_Password
 * PURPOSE:
 *          This function get privatekey password to be download from tftp server.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          char   *tftp_private_password  --  privatekey password.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Get_Tftp_Privatekey_Password(char *tftp_private_password)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if (tftp_private_password == NULL)
        {
            return FALSE;
        }

        ret = HTTP_OM_Get_Tftp_Privatekey_Password(tftp_private_password);

        return ret;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Set_Tftp_Download_Certificate_Flag
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
BOOL_T HTTP_MGR_Set_Tftp_Download_Certificate_Flag()
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = HTTP_OM_Set_Tftp_Download_Certificate_Flag();

        return ret;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Reset_Tftp_Download_Certificate_Flag
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
BOOL_T HTTP_MGR_Reset_Tftp_Download_Certificate_Flag()
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = HTTP_OM_Reset_Tftp_Download_Certificate_Flag();

        return ret;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Set_Certificate_Download_Status
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
BOOL_T HTTP_MGR_Set_Certificate_Download_Status(HTTP_GetCertificateStatus_T status)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = HTTP_OM_Set_Certificate_Download_Status(status);

        return ret;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_Certificate_Download_Status
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
BOOL_T HTTP_MGR_Get_Certificate_Download_Status(HTTP_GetCertificateStatus_T *status)
{
    BOOL_T  ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = HTTP_OM_Get_Certificate_Download_Status(status);

        return ret;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_Certificate_From_Tftp
 * PURPOSE:
 *          A server certificate authenticates the server to the client.
 *          This function to save server certificate ,
 *          when you got a server certificate from a certification authority(CA).
 *
 * INPUT:
 *          certificate_entry_p  -- certificate entry
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          User can use TFTP or XModem to upload server certificate
 *
 *          This function is invoked in WEB or SNMP.
 */
BOOL_T
HTTP_MGR_Get_Certificate_From_Tftp(
    HTTP_DownloadCertificateEntry_T *certificate_entry_p)
{
    UI32_T  tid;

    if (SYS_TYPE_STACKING_MASTER_MODE != SYSFUN_GET_CSC_OPERATING_MODE())
    {
        return FALSE;
    }

    if (TRUE != HTTP_OM_Set_Tftp_Download_Certificate_Flag())
    {
        return FALSE;
    }

    if (TRUE != HTTP_MGR_Set_Tftp_Ip(&certificate_entry_p->tftp_server))
    {
        HTTP_OM_Init_Download_Certificate_Entry();
        return FALSE;
    }

    if (TRUE != HTTP_MGR_Set_Tftp_Certificate_Filename(certificate_entry_p->tftp_cert_file))
    {
        HTTP_OM_Init_Download_Certificate_Entry();
        return FALSE;
    }

    if (TRUE != HTTP_MGR_Set_Tftp_Privatekey_Filename(certificate_entry_p->tftp_private_file))
    {
        HTTP_OM_Init_Download_Certificate_Entry();
        return FALSE;
    }

    if (TRUE != HTTP_MGR_Set_Tftp_Privatekey_Password(certificate_entry_p->tftp_private_password))
    {
        HTTP_OM_Init_Download_Certificate_Entry();
        return FALSE;
    }

    if (TRUE != HTTP_OM_Set_Tftp_Cookie(certificate_entry_p->cookie))
    {
        HTTP_OM_Init_Download_Certificate_Entry();
        return FALSE;
    }

    if (SYSFUN_SpawnThread(SYS_BLD_HTTP_ASYNC_GET_CERT_THREAD_PRIORITY,
                           SYS_BLD_HTTP_ASYNC_GET_CERT_THREAD_SCHED_POLICY,
                           SYS_BLD_HTTP_ASYNC_GET_CERT_TASK,
                           SYS_BLD_HTTP_ASYNC_GET_CERT_TASK_STACK_SIZE,
                           SYSFUN_TASK_NO_FP,
                           HTTP_MGR_Async_Get_Certificate,
                           0,
                           &tid) != SYSFUN_OK)
    {
        HTTP_OM_Init_Download_Certificate_Entry();
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME:  HTTP_MGR_Async_Get_Certificate
 * PURPOSE:
 *          A server certificate authenticates the server to the client.
 *          This function to save server certificate ,
 *          when you got a server certificate from a certification authority(CA).
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
 *          User can use TFTP or XModem to upload server certificate
 *
 *          This function is invoked in HTTP_MGR_Get_Certificate_From_Tftp() or (SNMP).
 */
static BOOL_T HTTP_MGR_Async_Get_Certificate()
{
    HTTP_GetCertificateStatus_T cert_status;
    XFER_MGR_UserInfo_T  user_info;
    UI8_T   *xfer_buf;
    char    *certificate_filename;
    BOOL_T  xfer_ret;
    BOOL_T  is_xfer_in_progress;
    char    *key_filename;
    char    *passwd;
    SSL_CTX *ssl_ctx;
    UI8_T username[MAXSIZE_fileCopyServerUserName + 1] = {0};
    UI8_T password[MAXSIZE_fileCopyServerPassword + 1] = {0};
    L_INET_AddrIp_T  server_ip;
    void *cookie = NULL;

    HTTP_OM_Get_Tftp_Cookie(&cookie);

    cert_status = HTTP_GET_CERT_DOING;
    HTTP_OM_Set_Certificate_Download_Status(cert_status);

    memset(&server_ip, 0, sizeof(L_INET_AddrIp_T));

    /* http & xfer are in different process, should use shared memory
     */
    xfer_buf = BUFFER_MGR_Allocate();

    if (NULL == xfer_buf)
    {
        HTTP_OM_Set_Certificate_Download_Status(HTTP_GET_CERT_NOMEMORY);
        HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_ERROR);
        HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_COMPLETED);
        HTTP_OM_Reset_Tftp_Download_Certificate_Flag();
        return FALSE;
    }

    HTTP_MGR_Get_Tftp_Ip(&server_ip);
    certificate_filename = L_MM_Malloc(MAXSIZE_tftpSrcFile + 1, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_HTTP_MGR_ASYNC_GET_CERTIFICATE));

    if( certificate_filename == NULL )
    {
        HTTP_OM_Set_Certificate_Download_Status(HTTP_GET_CERT_NOMEMORY);
        HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_ERROR);
        HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_COMPLETED);
        BUFFER_MGR_Free(xfer_buf);
        HTTP_OM_Reset_Tftp_Download_Certificate_Flag();
        return FALSE;
    }

    HTTP_MGR_Get_Tftp_Certificate_Filename(certificate_filename);

    memset(&user_info, 0, sizeof(user_info));

    HTTP_MGR_Set_XferProgressStatus(TRUE);

    if (!XFER_PMGR_RemoteToStream(&user_info,
                                  XFER_MGR_REMOTE_SERVER_TFTP,
                                  &server_ip,
                                  username,
                                  password,
                                  (UI8_T *)certificate_filename,
                                  FS_FILE_TYPE_CERTIFICATE,
                                  xfer_buf,
                                  (UI32_T)0,
                                  SYS_BLD_WEB_GROUP_IPCMSGQ_KEY,
                                  0,
                                  SERVER_CERTIFICATE_FILE_LENGTH))
  {
        HTTP_OM_Set_Certificate_Download_Status(HTTP_GET_CERT_ERROR);
        HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_ERROR);
        HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_COMPLETED);
        /*has been freed in xfer */
        /*BUFFER_MGR_Free(xfer_buf);*/
        L_MM_Free(certificate_filename);
        HTTP_OM_Reset_Tftp_Download_Certificate_Flag();
        return FALSE;
    }

    while(1)
    {
        SYSFUN_Sleep(100);
        HTTP_MGR_Get_XferProgressStatus(&is_xfer_in_progress);

        if( is_xfer_in_progress == FALSE )
        {
            HTTP_MGR_Get_XferProgressResult(&xfer_ret);

            if( xfer_ret == FALSE )
            {
                HTTP_OM_Set_Certificate_Download_Status(HTTP_GET_CERT_ERROR);
                HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_ERROR);
                HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_COMPLETED);
                /*has been freed in xfer */
                /*BUFFER_MGR_Free(xfer_buf);*/
                L_MM_Free(certificate_filename);
                HTTP_OM_Reset_Tftp_Download_Certificate_Flag();
                return FALSE;
            }
            else
            {
                break;
            }
        }
    }

    memset(server_certificate,0,SERVER_CERTIFICATE_FILE_LENGTH);
    memcpy(server_certificate, xfer_buf, SERVER_CERTIFICATE_FILE_LENGTH);
    L_MM_Free(certificate_filename);

    memset(server_pass_phrase,0,SERVER_PASS_PHRASE_FILE_LENGTH);
    key_filename = L_MM_Malloc(MAXSIZE_tftpSrcFile + 1, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_HTTP_MGR_ASYNC_GET_CERTIFICATE));

    if( key_filename == NULL )
    {
        HTTP_OM_Set_Certificate_Download_Status(HTTP_GET_CERT_NOMEMORY);
        HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_ERROR);
        HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_COMPLETED);
        BUFFER_MGR_Free(xfer_buf);
        HTTP_OM_Reset_Tftp_Download_Certificate_Flag();
        return FALSE;
    }

    HTTP_MGR_Get_Tftp_Privatekey_Filename(key_filename);
    passwd = L_MM_Malloc(HTTP_TYPE_KEY_PASSWD_MAX_LEN, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_HTTP_MGR_ASYNC_GET_CERTIFICATE));

    if( passwd == NULL )
    {
        HTTP_OM_Set_Certificate_Download_Status(HTTP_GET_CERT_NOMEMORY);
        HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_ERROR);
        HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_COMPLETED);
        BUFFER_MGR_Free(xfer_buf);
        L_MM_Free(key_filename);
        HTTP_OM_Reset_Tftp_Download_Certificate_Flag();
        return FALSE;
    }

    HTTP_MGR_Get_Tftp_Privatekey_Password(passwd);
    strcpy((char *)server_pass_phrase,passwd);
    HTTP_MGR_Set_XferProgressStatus(TRUE);

    if (!XFER_PMGR_RemoteToStream(&user_info,
                                  XFER_MGR_REMOTE_SERVER_TFTP,
                                  &server_ip,
                                  username,
                                  password,
                                  (UI8_T *)key_filename,
                                  FS_FILE_TYPE_CERTIFICATE,
                                  xfer_buf,
                                  (UI32_T)0,
                                  SYS_BLD_WEB_GROUP_IPCMSGQ_KEY,
                                  0,
                                  SERVER_PRIVATE_KEY_FILE_LENGTH))
    {
        HTTP_OM_Set_Certificate_Download_Status(HTTP_GET_CERT_ERROR);
        HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_ERROR);
        HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_COMPLETED);
        /*has been freed in xfer */
        /*BUFFER_MGR_Free(xfer_buf);*/
        L_MM_Free(key_filename);
        L_MM_Free(passwd);
        HTTP_OM_Reset_Tftp_Download_Certificate_Flag();
        return FALSE;
    }

    while(1)
    {
        SYSFUN_Sleep(100);
        HTTP_MGR_Get_XferProgressStatus(&is_xfer_in_progress);
        if( is_xfer_in_progress == FALSE )
        {
            HTTP_MGR_Get_XferProgressResult(&xfer_ret);
            if( xfer_ret == FALSE )
            {
                HTTP_OM_Set_Certificate_Download_Status(HTTP_GET_CERT_ERROR);
                HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_ERROR);
                HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_COMPLETED);
                /*has been freed in xfer */
                /*BUFFER_MGR_Free(xfer_buf);*/
        	       L_MM_Free(key_filename);
        	       L_MM_Free(passwd);
                HTTP_OM_Reset_Tftp_Download_Certificate_Flag();
                return FALSE;
            }
            else
            {
                break;
            }
        }
    }

    memset(server_private_key, 0, SERVER_PRIVATE_KEY_FILE_LENGTH);
    memcpy(server_private_key, xfer_buf, SERVER_PRIVATE_KEY_FILE_LENGTH);
    L_MM_Free(key_filename);
    L_MM_Free(passwd);

    ssl_ctx = SSL_CTX_new(SSLv23_server_method());

    if ( NULL == ssl_ctx )
    {
        HTTP_OM_Set_Certificate_Download_Status(HTTP_GET_CERT_NOMEMORY);
        HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_ERROR);
        HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_COMPLETED);
        BUFFER_MGR_Free(xfer_buf);
        HTTP_OM_Reset_Tftp_Download_Certificate_Flag();
        return FALSE;
    }

    {
        BIO *in;
        int ret=0;
        X509 *x=NULL;

        in=BIO_new_mem_buf(server_certificate, -1);

        x=PEM_read_bio_X509(in,NULL,ssl_ctx->default_passwd_callback,ssl_ctx->default_passwd_callback_userdata);
        ret=SSL_CTX_use_certificate(ssl_ctx,x);
        if (x != NULL) X509_free(x);
        if (in != NULL) BIO_free(in);

        if (ret <= 0)
        {
            SSL_CTX_free(ssl_ctx);
            HTTP_OM_Set_Certificate_Download_Status(HTTP_GET_CERT_FILE_ERROR);
            HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_ERROR);
            HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_COMPLETED);
            BUFFER_MGR_Free(xfer_buf);
            HTTP_OM_Reset_Tftp_Download_Certificate_Flag();
            return FALSE;
        }
    }

    ssl_ctx->default_passwd_callback_userdata = server_pass_phrase;

    {
        int ret=0;
        BIO *in;
        EVP_PKEY *pkey=NULL;

        in=BIO_new_mem_buf(server_private_key, -1);
        if (in == NULL)
        {
        }

        {
            pkey=PEM_read_bio_PrivateKey(in,NULL,
                ssl_ctx->default_passwd_callback,ssl_ctx->default_passwd_callback_userdata);
        }

        if (pkey != NULL)
        {
            ret=SSL_CTX_use_PrivateKey(ssl_ctx,pkey);
        }

        if (pkey) EVP_PKEY_free(pkey);
        if (in != NULL) BIO_free(in);

        if (ret <= 0)
        {
            SSL_CTX_free(ssl_ctx);
            HTTP_OM_Set_Certificate_Download_Status(HTTP_GET_CERT_FILE_PHRAS_ERROR);
            HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_ERROR);
            HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_COMPLETED);
            BUFFER_MGR_Free(xfer_buf);
            HTTP_OM_Reset_Tftp_Download_Certificate_Flag();
            return FALSE;
        }
    }

    if ( !SSL_CTX_check_private_key(ssl_ctx) )
    {
        SSL_CTX_free(ssl_ctx);
        HTTP_OM_Set_Certificate_Download_Status(HTTP_GET_CERT_CERT_PRIVATE_NOMATCH);
        HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_ERROR);
        HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_COMPLETED);
        BUFFER_MGR_Free(xfer_buf);
        HTTP_OM_Reset_Tftp_Download_Certificate_Flag();
        return FALSE;
    }

    SSL_CTX_free(ssl_ctx);
    HTTP_MGR_Set_Server_Certificate((char*)server_certificate, (char*)server_private_key, (char*)server_pass_phrase);
    HTTP_OM_Set_Certificate_Download_Status(HTTP_GET_CERT_DONE);
    HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_SUCCESS);
    HTTP_MGR_NotifyCliXferResult(cookie, XFER_MGR_FILE_COPY_COMPLETED);
    BUFFER_MGR_Free(xfer_buf);

    HTTP_OM_Set_Certificate_Status(TRUE);
    HTTP_OM_Reset_Tftp_Download_Certificate_Flag();
    return TRUE;
}
/* FUNCTION NAME:  HTTP_MGR_Set_XferProgressStatus
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
void HTTP_MGR_Set_XferProgressStatus(BOOL_T status)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return;
    }
    else
    {
        HTTP_OM_Set_XferProgressStatus(status);

        return;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_XferProgressStatus
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
void HTTP_MGR_Get_XferProgressStatus(BOOL_T *status)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return;
    }
    else
    {
        HTTP_OM_Get_XferProgressStatus(status);

        return;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Set_XferProgressResult
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
void HTTP_MGR_Set_XferProgressResult(BOOL_T result)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return;
    }
    else
    {
        HTTP_OM_Set_XferProgressResult(result);

        return;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_XferProgressResult
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
void HTTP_MGR_Get_XferProgressResult(BOOL_T *result)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return;
    }
    else
    {
        HTTP_OM_Get_XferProgressResult(result);

        return;
    }
}

static BOOL_T HTTP_MGR_ReadTextFile(const char* path, char* buf, size_t buf_size)
{
    FILE *file;
    size_t fsize;
    size_t rdlen;

    file = fopen(path, "rb");
    if (!file)
    {printf("no such file %s\r\n", path);
        return FALSE;
    }

    // obtain file size:
    fseek(file, 0, SEEK_END);
    fsize = ftell(file);
    rewind(file);

    if ((buf_size - 1) < fsize)
    {
        fclose(file);
        return FALSE;
    }

    rdlen = fread(buf, 1, buf_size - 1, file);
    buf[rdlen] = '\0';

    fclose(file);

    return TRUE;
}

static BOOL_T HTTP_MGR_WriteTextFile(const char* path, const char *buf)
{
    FILE *file;

    file = fopen(path, "wb");
    if (!file)
    {
        return FALSE;
    }

    fwrite(buf, 1, strlen(buf), file);
    fclose(file);

    return TRUE;
}

static BOOL_T HTTP_MGR_Get_Https_Certificate(char *cert, UI32_T cert_size)
{
    char path[HTTP_CONFIG_HTTPS_CERTIFICATE_PATH_MAX_LEN + 1];

    HTTP_OM_Get_Https_Certificate_Path(path, sizeof(path));
    if (path[0] != '\0')
    {
        if (TRUE == HTTP_MGR_ReadTextFile(path, cert, cert_size))
        {printf("get file from %s OK\r\n", path);
            return TRUE;
        }
    }

    return KEYGEN_PMGR_GetServerCertificate((UI8_T*)cert);
}

static BOOL_T HTTP_MGR_Get_Https_Private_Key(char *key, UI32_T key_size)
{
    char path[HTTP_CONFIG_HTTPS_PARIVATE_KEY_PATH_MAX_LEN + 1];

    HTTP_OM_Get_Https_Private_Key_Path(path, sizeof(path));
    if (path[0] != '\0')
    {
        if (TRUE == HTTP_MGR_ReadTextFile(path, key, key_size))
        {
            return TRUE;
        }
    }

    return KEYGEN_PMGR_GetServerPrivateKey((UI8_T*)key);
}

static BOOL_T HTTP_MGR_Get_Https_Passphrase(char *pass, UI32_T pass_size)
{
    char path[HTTP_CONFIG_HTTPS_PASS_PHRASE_PATH_MAX_LEN + 1];

    HTTP_OM_Get_Https_Passphrase_Path(path, sizeof(path));
    if (path[0] != '\0')
    {
        if (TRUE == HTTP_MGR_ReadTextFile(path, pass, pass_size))
        {
            return TRUE;
        }
    }

    return KEYGEN_PMGR_GetServerPassPhrase((UI8_T*)pass);
}

static BOOL_T HTTP_MGR_Set_Server_Certificate(const char *server_certificate, const char *server_private_key, const char *server_pass_phrase)
{
    char cert_url[HTTP_CONFIG_HTTPS_CERTIFICATE_PATH_MAX_LEN + 1];
    char key_url[HTTP_CONFIG_HTTPS_PARIVATE_KEY_PATH_MAX_LEN + 1];
    char pass_url[HTTP_CONFIG_HTTPS_PASS_PHRASE_PATH_MAX_LEN + 1];

    HTTP_OM_Get_Https_Certificate_Path(cert_url, sizeof(cert_url));
    if (cert_url[0] != '\0')
    {
        HTTP_MGR_WriteTextFile(cert_url, server_certificate);
    }

    HTTP_OM_Get_Https_Private_Key_Path(key_url, sizeof(key_url));
    if (key_url[0] != '\0')
    {
        HTTP_MGR_WriteTextFile(key_url, server_private_key);
    }

    HTTP_OM_Get_Https_Passphrase_Path(pass_url, sizeof(pass_url));
    if (pass_url[0] != '\0')
    {
        HTTP_MGR_WriteTextFile(pass_url, server_pass_phrase);
    }

    return KEYGEN_PMGR_SetNewCertificate((UI8_T*)server_certificate, (UI8_T*)server_private_key, (UI8_T*)server_pass_phrase);
}

#endif /* if SYS_CPNT_HTTPS == TRUE */

/* FUNCTION NAME:  HTTP_MGR_Set_User_Connection_Info
 * PURPOSE:
 *          This function set user connection info.
 *
 * INPUT:
 *          void *req   --  request struct of http connection.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Set_User_Connection_Info(void *req)
{
    HTTP_Connection_T      *http_connection;
    HTTP_Request_T         *req_p;
    struct  sockaddr_in6    peer_sock_addr;
    socklen_t               peer_sock_addr_size=sizeof(peer_sock_addr);
    UI32_T                  access_time;
    char                   *username;
    char                   *sz_auth_level;
    BOOL_T                  ret;
    int                     http_port, sock_port;
#if (SYS_CPNT_HTTPS == TRUE)
    UI32_T                  secure_port;
#endif
    UI32_T                  protocol;
    struct  sockaddr_in6    sock_addr;
    socklen_t               sock_addr_size = sizeof(sock_addr);
    L_INET_AddrIp_T         remote_addr, local_addr;
    char                   *session_id_p;
    char                    session_id[HTTP_MAX_SESSION_ID_STR_LEN + 1];
    int                     auth_level;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        req_p = (HTTP_Request_T *)req;
        memset(&remote_addr, 0, sizeof(remote_addr));
        memset(&local_addr, 0, sizeof(local_addr));

        if( getsockname(req_p->fd, (struct sockaddr *) &sock_addr, &sock_addr_size) != 0 )
        {
            return FALSE;
        }

        sock_port = HTTP_MGR_GetSockPort((struct sockaddr*)&sock_addr);
        http_port = HTTP_MGR_Get_Http_Port();

#if (SYS_CPNT_HTTPS == TRUE)
        HTTP_MGR_Get_Secure_Port(&secure_port);
#endif

        if( http_port == sock_port )
        {
            protocol = HTTP_CONNECTION;
        }
#if (SYS_CPNT_HTTPS == TRUE)
        else if( secure_port == sock_port )
        {
            protocol = HTTPS_CONNECTION;
        }
#endif
        else
        {
            return FALSE;
        }

        /* get socket's remote ip
         */
        if( getpeername(req_p->fd, (struct sockaddr*)&peer_sock_addr, &peer_sock_addr_size) != 0 )
        {
            return FALSE;
        }
        if(L_INET_SockaddrToInaddr((struct sockaddr *)&peer_sock_addr, &remote_addr) == FALSE)
        {
            return FALSE;
        }

        /* get socket's local ip (sock_addr is already get at above)
         */
        if(L_INET_SockaddrToInaddr((struct sockaddr *)&sock_addr, &local_addr) == FALSE)
        {
            return FALSE;
        }

        username = get_env(req_p->envcfg, "REMOTE_USER");

        if( (username == NULL) || (strcmp(username, "") == 0) )
        {
            return FALSE;
        }

        sz_auth_level = get_env(req_p->envcfg, "AUTH_LEVEL");
        if (sz_auth_level == NULL)
        {
            return FALSE;
        }

        auth_level = atoi(sz_auth_level);

        access_time = SYSFUN_GetSysTick();

        /* for cluster operate, use commander's seesion id
         */
        if ((get_env(req_p->envcfg, "REMOTE_CLUSTER_ID") != NULL))
        {
            session_id_p = get_env(req_p->envcfg, "SESSION_ID");

            if (session_id_p == NULL)
            {
                return FALSE;
            }

            SYSFUN_Snprintf(session_id, sizeof(session_id), "%s", session_id_p);
            session_id[ sizeof(session_id) -1 ] = '\0';
        }
        else
        {
            UI32_T secure_rand;
            UI32_T seed;
            char identity[100+SYS_ADPT_MAX_USER_NAME_LEN+L_INET_MAX_IPADDR_STR_LEN+1];
            char sz_rem_ip_addr[L_INET_MAX_IPADDR_STR_LEN+1];
            UI8_T digest[16];

            assert(sizeof(session_id) == ((sizeof(digest)*2)+1));

            srand(access_time);
            seed = rand();

            srand(seed);
            secure_rand = rand();

            if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&remote_addr,
                                                               sz_rem_ip_addr,
                                                               sizeof(sz_rem_ip_addr)))
            {
                return FALSE;
            }

            SYSFUN_Snprintf(identity, sizeof(identity), "%lu%lu%lu%s%s%lu",
                access_time,
                seed, secure_rand,
                username, sz_rem_ip_addr,
                protocol);
            identity[ sizeof(identity)-1 ] = '\0';
// FIXME: fix this
            L_MD5_MDString(digest, (UI8_T *)identity, strlen(identity));

            memset(identity, 0, sizeof(identity));

            SYSFUN_Snprintf(session_id,
                sizeof(session_id),
                "%02X%02X%02X%02X%02X%02X%02X%02X"
                "%02X%02X%02X%02X%02X%02X%02X%02X",
                digest[0],  digest[1],  digest[2],  digest[3],
                digest[4],  digest[5],  digest[6],  digest[7],
                digest[8],  digest[9],  digest[10], digest[11],
                digest[12], digest[13], digest[14], digest[15]);
            session_id[ sizeof(session_id) -1 ] = '\0';

            set_env(req_p->envcfg, "SESSION_ID", session_id);
        }

        ret = HTTP_OM_Set_User_Connection_Info(&remote_addr, &local_addr, username, access_time, protocol, auth_level, session_id);
        return ret;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_User_Connection_Info
 * PURPOSE:
 *          This function get user connection info.
 *
 * INPUT:
 *          void *req   --  request struct of http connection.
 *
 * OUTPUT:
 *          user_conn_info_p   -- user connection information
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Get_User_Connection_Info(void *req, HTTP_Session_T *user_conn_info_p)
{
    HTTP_Request_T         *req_p;
    const char             *session_id;
    struct  sockaddr_in6    peer_sock_addr;
    socklen_t               peer_sock_addr_size=sizeof(peer_sock_addr);
    L_INET_AddrIp_T         remote_addr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }

    req_p = (HTTP_Request_T *)req;

    session_id = get_env(req_p->envcfg, "SESSION_ID");
    if (NULL == session_id)
    {
        return FALSE;
    }

    if (FALSE == HTTP_OM_Get_User_Connection_Info(session_id, user_conn_info_p))
    {
        return FALSE;
    }

    /* Check peer IP address for security issue
     */
    memset(&remote_addr, 0, sizeof(remote_addr));

    if (getpeername(req_p->fd, (struct sockaddr*)&peer_sock_addr, &peer_sock_addr_size) != 0 )
    {
        return FALSE;
    }

    if (L_INET_SockaddrToInaddr((struct sockaddr *)&peer_sock_addr, &remote_addr) == FALSE)
    {
        return FALSE;
    }

    if (L_INET_CompareInetAddr((L_INET_Addr_T *) &remote_addr,
        (L_INET_Addr_T *) &user_conn_info_p->remote_ip, 0) != 0)
    {
        /* TODO: Detected an intrusion. Write syslog or debug message here
         */
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME:  HTTP_MGR_Check_User_Connection_Info
 * PURPOSE:
 *          This function check current connection info.
 *
 * INPUT:
 *          void *req   --  request struct of http connection.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Check_User_Connection_Info(void *req)
{
    BOOL_T                  ret;
    HTTP_Request_T         *req_p;
    UI32_T                  access_time;
    BOOL_T                  is_pooling_url = FALSE;
    const char             *session_id;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        req_p = (HTTP_Request_T *)req;

        access_time = SYSFUN_GetSysTick();

        {
            const char *pooling_urls[] = HTTP_CFG_POOLING_URLS;
            int         i;

            for (i = 0; i < _countof(pooling_urls); ++ i) {
                if (strstr(req_p->request_uri, pooling_urls[i])) {
                    is_pooling_url = TRUE;
                    break;
                }
            }
        }

        session_id = get_env(req_p->envcfg, "SESSION_ID");
        if (session_id == NULL)
        {
            return FALSE;
        }

        if (is_pooling_url)
        {
            ret = HTTP_OM_Delete_User_Connection_Info_If_Timeout(access_time, session_id);
        }
        else
        {
            ret = HTTP_OM_Check_User_Connection_Info(access_time, session_id);
        }
        return ret;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Delete_User_Connection_Info
 * PURPOSE:
 *          This function delete current connection info.
 *
 * INPUT:
 *          void *req   --  request struct of http connection.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Delete_User_Connection_Info(void *req)
{
    BOOL_T                  ret;
    HTTP_Request_T         *req_p;
    const char             *session_id;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        req_p = (HTTP_Request_T *)req;

        session_id = get_env(req_p->envcfg, "SESSION_ID");
        if (session_id == NULL)
        {
            return FALSE;
        }

        ret = HTTP_OM_Delete_User_Connection_Info(session_id);
        return ret;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Set_User_Connection_Send_Log
 * PURPOSE:
 *          This function set current connection info.
 *
 * INPUT:
 *          L_INET_AddrIp_T  ip_addr     --  remote ip address.
 *          UI8_T   *username   --  remote user.
 *          BOOL_T is_send_log  -- send log flag.
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Set_User_Connection_Send_Log(L_INET_AddrIp_T ip_addr, const char *username, BOOL_T is_send_log)
{
    BOOL_T  ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = HTTP_OM_Set_User_Connection_Send_Log(ip_addr, username, is_send_log);
        return ret;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_Next_User_Connection_Info
 * PURPOSE:
 *          This function get next connection info.
 *
 * INPUT:
 *          HTTP_UserConnectionInfo_T   *connection_info    --  previous active connection information.
 *
 * OUTPUT:
 *          HTTP_UserConnectionInfo_T   *connection_info    --  current active connection information.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *      This function invoked in CLI command "show user".
 *      Initial input value is zero.
 */
BOOL_T HTTP_MGR_Get_Next_User_Connection_Info(HTTP_Session_T *connection_info)
{
    BOOL_T  ret;
    UI32_T  current_time;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        current_time = SYSFUN_GetSysTick();
        ret = HTTP_OM_Get_Next_User_Connection_Info(connection_info, current_time);
        return ret;
    }
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - HTTP_MGR_SendLoginLogoutTrap
 *------------------------------------------------------------------------
 * FUNCTION: Send login/logout trap
 * INPUT   : trap_type  -- trap type is login or logout
 *           http_connection -- http connection object
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T
HTTP_MGR_SendLoginLogoutTrap(
    UI32_T trap_type,
    HTTP_Connection_T *http_connection)
{
    TRAP_EVENT_TrapData_T    data;
    L_INET_AddrIp_T          rem_ip_addr;
    L_INET_AddrIp_T          nexthop_addr;
    struct sockaddr_storage  sock_addr;
    socklen_t                sock_addr_size = sizeof(sock_addr);
    char                    *user_name_p;

    memset(&data, 0, sizeof(data));
    data.trap_type = trap_type;
    data.community_specified = FALSE;

    http_connection_dbg_check(http_connection);

    /* get login method
     */
    switch (http_connection->conn_state)
    {
        case HTTP_CONNECTION:
            data.u.user_info.session_type = VAL_trapVarSessionType_http;
            break;

        case HTTPS_CONNECTION:
            data.u.user_info.session_type = VAL_trapVarSessionType_https;
            break;

        case UNKNOW_CONNECTION:
        default:
            return FALSE;
    }

    /* get user name
     */
    user_name_p = get_env(http_connection->req->envcfg, "REMOTE_USER");

    if (NULL == user_name_p)
    {
        return FALSE;
    }

    strncpy(data.u.user_info.user_name, user_name_p,
        sizeof(data.u.user_info.user_name)-1);
    data.u.user_info.user_name[sizeof(data.u.user_info.user_name)-1] = '\0';

    /* get user ip
     */
    memset(&rem_ip_addr, 0, sizeof(rem_ip_addr));

    if (0 != getpeername(http_connection->req->fd, (struct sockaddr *)&sock_addr, &sock_addr_size))
    {
        return FALSE;
    }

    if (FALSE == L_INET_SockaddrToInaddr((struct sockaddr *)&sock_addr, &rem_ip_addr))
    {
        return FALSE;
    }

    memcpy(&data.u.user_info.user_ip, &rem_ip_addr, sizeof(data.u.user_info.user_ip));

    /* get user mac
     */
    if (NETCFG_TYPE_FAIL == NETCFG_PMGR_ROUTE_GetReversePathIpMac(&rem_ip_addr,
        &nexthop_addr, data.u.user_info.user_mac))
    {
        return FALSE;
    }

    SNMP_PMGR_ReqSendTrapOptional(&data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
    return TRUE;
}

/* FUNCTION NAME - HTTP_MGR_HandleHotInsertion
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
void HTTP_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    return;
}

/* FUNCTION NAME - HTTP_MGR_HandleHotRemoval
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
void HTTP_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    return;
}

#if (SYS_CPNT_CLUSTER == TRUE)
/* FUNCTION NAME:  HTTP_MGR_Set_Http_Cluster_Port
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
*          none.
* NOTES:
*          .
*/
BOOL_T HTTP_MGR_Set_Cluster_Port(UI32_T port)
{
    UI32_T  old_port;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if (port<1 || port>65535)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_HTTP, HTTP_MGR_Set_Http_Port_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "HTTP port number(1-65535)");
            return FALSE;
        }

        old_port = HTTP_OM_Get_Cluster_Port();

        if (old_port == port)
        {
            return TRUE;
        }

        if (TRUE != IP_LIB_IsValidSocketPortForServerListen((UI16_T)port, IP_LIB_SKTTYPE_TCP, IP_LIB_CHKAPP_HTTP))
        {
            return FALSE;
        }

        HTTP_OM_Set_Cluster_Port(port);

        return TRUE;
    }
}

/* FUNCTION NAME:  HTTP_MGR_Get_Cluster_Port
 * PURPOSE:
 *          This function get cluster port number.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T - Cluster port value.
 * NOTES:
 *          default is tcp/80.
 */
UI32_T HTTP_MGR_Get_Cluster_Port()
{
    UI32_T  port = 0;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        port = HTTP_OM_Get_Cluster_Port();
    }

    return port;
}
#endif /* SYS_CPNT_CLUSTER */

/* FUNCTION NAME : HTTP_MGR_RifDestroyedCallbackHandler
 * PURPOSE:
 *      Process when the rif destroy (IP address was changed)
 *
 * INPUT:
 *      ip_addr_p  -- the ip address which is down
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void HTTP_MGR_RifDestroyedCallbackHandler(L_INET_AddrIp_T *ip_addr_p)
{
    HTTP_Session_T   user_info;

    memset(&user_info, 0, sizeof(user_info));

    while (TRUE == HTTP_OM_Get_Next_User_Connection_Info(&user_info,
        SYSFUN_GetSysTick()))
    {
        if ((user_info.local_ip.addrlen == ip_addr_p->addrlen) &&
            (memcmp(user_info.local_ip.addr, ip_addr_p->addr, ip_addr_p->addrlen)==0))
        {
            HTTP_OM_Delete_User_Connection_Info(user_info.session_id);
            memset(&user_info, 0, sizeof(user_info));
        }
    }

    HTTP_TASK_RifDestroyed(ip_addr_p);
}

/* LOCAL SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------*/
#if (SYS_CPNT_HTTPS == TRUE)

/* FUNCTION NAME:  HTTP_MGR_NotifyCliXferResult
 * PURPOSE:
 *          callback to CLI to announce Xfer status
 *
 * INPUT:
 *          void * cookie .
 *          XFER_MGR_FileCopyStatus_T  status
 *
 * OUTPUT:
 *
 * RETURN:
 *
 * NOTES:
 */
static void HTTP_MGR_NotifyCliXferResult (void *cookie, XFER_MGR_FileCopyStatus_T  status)
{
   SYS_CALLBACK_MGR_AnnounceCliXferResultCallback( SYS_MODULE_HTTP,
       SYS_BLD_CLI_GROUP_IPCMSGQ_KEY, 0, cookie, status);
}


/* FUNCTION NAME:  HTTP_SSL_Rand_Choosenum
 * PURPOSE:
 *          Choose number between input parameter 1 and parameter 2.
 *
 * INPUT:
 *          UI32_T  -- Lowest number .
 *
 *          UI32_T  -- Highest number.
 *
 * OUTPUT:
 *          UI32_T * - The number between lowest and highest.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
static BOOL_T HTTP_SSL_Rand_Choosenum(UI32_T l, UI32_T h, UI32_T *choose)
{
    UI32_T i,l_time;
    double d;

/*isiah.2004-01-08*/
    SYS_TIME_GetRealTimeBySec(&l_time);
    srand(l_time);
    d = ((double)(rand()%RAND_MAX)/RAND_MAX)*(h-l);
    i = (UI32_T)d+1;

    if (i < l)
    {
        i = l;
    }

    if (i > h)
    {
        i = h;
    }

    *choose = i;
    return TRUE;
}

/* FUNCTION NAME:  HTTP_Get_Temp_PublicKey_Pair
 * PURPOSE:
 *          Get 512 and 1024 bits temporary keys
 *
 * INPUT:
 *          SSL *   -- SSL structure.
 *
 *          int     -- nExport,1 to indicate an export cipher is used and 0 to indicate not.
 *
 *          int     -- temporary key length,valid value are 512 and 1024 .
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          RSA * - RSA structure.
 * NOTES:
 *          This API will be registed into SSL library and called from there
 */
RSA *HTTP_Get_Temp_PublicKey_Pair(SSL *pSSL, int nExport, int nKeyLen)
{
    RSA *rsa;

    rsa = (RSA *) KEYGEN_PMGR_GetHttpsTempPublicKeyPair(nExport, nKeyLen);
    return ( rsa );
}

/* FUNCTION NAME:  HTTP_Log_Tracing_State
 * PURPOSE:
 *          This function is executed while OpenSSL processes the
 *          SSL handshake and does SSL record layer stuff. We use it to
 *          trace OpenSSL's processing in out SSL logfile.
 * INPUT:
 *          parameter-1  -- argument description included possible value,
 *                                   valid range, notice.
 *          parameter-2  -- argument description included possible value,
 *                                   valid range, notice.
 *          parameter-3  -- argument description included possible value,
 *                                   valid range, notice.
 * OUTPUT:
 *          parameter-n  -- calculation result be returned, describes valid /
 *                                   invalid value, semantic meaning.
 * RETURN:
 *          return-value-1 - describes the meaning.
 * NOTES:
 *          This API will be registed into SSL library and called from there
 */
void HTTP_Log_Tracing_State(const SSL *ssl, int where, int rc)
{
    const char *str;
    int usekeysize,algkeysize;
    HTTPS_DEBUG_STATE_STATE_T   is_debug_state;
    HTTPS_DEBUG_SESSION_STATE_T is_debug_session;

    /*
     * create the various trace messages
     */
    HTTP_OM_Get_Debug_Status(&is_debug_session,&is_debug_state);


    if ( is_debug_state == HTTPS_DEBUG_STATE_STATE_ENABLED )
    {
        if (where & SSL_CB_HANDSHAKE_START)
        {
            printf("State = SSL Handshake start\n");
        }
        else if (where & SSL_CB_HANDSHAKE_DONE)
        {
            printf("State = SSL Handshake done\n");
        }
        else if (where & SSL_CB_LOOP)
        {
            printf("State = %s\n", SSL_state_string_long(ssl));
        }
        else if (where & SSL_CB_READ)
        {
            printf("State = %s\n", SSL_state_string_long(ssl));
        }
        else if (where & SSL_CB_WRITE)
        {
            printf("State = %s\n", SSL_state_string_long(ssl));
        }
        else if (where & SSL_CB_ALERT)
        {
            str = (where & SSL_CB_READ) ? "read" : "write";
            printf("State =  Alert: %s:%s:%s\n", str, SSL_alert_type_string_long(rc), SSL_alert_desc_string_long(rc));
        }
        else if (where & SSL_CB_EXIT)
        {
            if (rc == 0)
            {
                printf("State = Exit: failed in %s\n", SSL_state_string_long(ssl));
            }
            else if (rc < 0)
            {
                printf("State = Exit: error in %s\n", SSL_state_string_long(ssl));
            }
        }
    }

    if ( is_debug_session == HTTPS_DEBUG_SESSION_STATE_ENABLED )
    {
        if ( where & SSL_CB_HANDSHAKE_DONE )
        {
            HTTP_Get_SSL_Cipher_Bits(ssl, &usekeysize, &algkeysize);
            printf("SessionInfo : SSLVer=%s, Cipher=%s(%d/%d bits)\n",SSL_get_version(ssl),SSL_get_cipher_name(ssl),usekeysize,algkeysize);
        }
    }

    return ;
}

/* FUNCTION NAME:  HTTP_Get_SSL_Cipher_Bits
 * PURPOSE:
 *          This function to get cipher's key size.
 *
 * INPUT:
 *          SSL *  -- SSL object.
 *
 * OUTPUT:
 *          int * -- usekeysize.
 *          int * -- algkeysize.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
static void HTTP_Get_SSL_Cipher_Bits(const SSL *ssl, int *usekeysize, int *algkeysize)
{
    SSL_CIPHER *cipher;

    *usekeysize = 0;
    *algkeysize = 0;
    if (ssl != NULL)
        if ((cipher = SSL_get_current_cipher(ssl)) != NULL)
            *usekeysize = SSL_CIPHER_get_bits(cipher, algkeysize);
    return;
}

/* FUNCTION NAME:  HTTP_Init_SSL_Resources
 * PURPOSE:
 *          Initiate SSL Resources -- Registers the available ciphers and digests,
 *          read certificate files from file system, create a new SSL_CTX object
 *          read server certificate and private key into SSL_CTX, initialize session cache,
 *          and hook function into SSL_CTX.
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
BOOL_T HTTP_Init_SSL_Resources()
{
    SSL_CTX *ssl_ctx;

    ssl_ctx = SSL_CTX_new (SSLv23_server_method());
    if ( ssl_ctx == NULL )
    {
        return FALSE;
    }

    ssl_ctx->session_cache_size = 5;
    HTTP_OM_Set_SSL_CTX(ssl_ctx);

    HTTP_Hook_Function_Into_SSL_CTX();

    /*
     * Seed Pseudo Random Number Generator
     */
    HTTP_Seed_SSL_Rand();

    return TRUE;
}

/* FUNCTION NAME:  HTTP_Hook_Function_Into_SSL_CTX
 * PURPOSE:
 *          Register hook function into OpenSSL library.
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
 *          HTTP_Init_SSL_Resources() will call this API to hook MGR APIs into SSL_CTX
 *          .
 */
BOOL_T HTTP_Hook_Function_Into_SSL_CTX()
{
    SSL_CTX *ssl_ctx;

    /*
     * Register callback function
     */

    ssl_ctx = HTTP_OM_Get_SSL_CTX();

    if ( NULL == ssl_ctx )
    {
        return FALSE;
    }

    SSL_CTX_set_tmp_rsa_callback(ssl_ctx,HTTP_Get_Temp_PublicKey_Pair);
    SSL_CTX_set_info_callback(ssl_ctx,HTTP_Log_Tracing_State);

    return TRUE;
}

/* FUNCTION NAME:  HTTP_Seed_SSL_Rand
 * PURPOSE:
 *          Support for better seeding of SSL library's RNG
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T - Number of bytes for seeding PRNG.
 * NOTES:
 *          .
 */
UI32_T HTTP_Seed_SSL_Rand()
{
    UI32_T l,nDone;
    UI32_T t;
    UI32_T  task_id;
    UI8_T stackdata[256];
    UI32_T n;

    nDone = 0;

    SYS_TIME_GetRealTimeBySec(&t);
    l = sizeof(t);
    RAND_seed((UI8_T *)&t, l);
    nDone += l;

    /*
    * seed in the current process id (usually just 4 bytes)
    */
    task_id = SYSFUN_TaskIdSelf();
    l = sizeof(UI32_T);
    RAND_seed((UI8_T *)&task_id, l);
    nDone += l;

    /*
    * seed in some current state of the run-time stack (128 bytes)
    */
    HTTP_SSL_Rand_Choosenum(0, sizeof(stackdata)-128-1, &n);
    RAND_seed(stackdata+n, 128);
    nDone += 128;

    return nDone;
}
#endif /* HTTP_CPNT_HTTPS */

/* FUNCTION NAME:  HTTP_MGR_MapUriToPath
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
char *HTTP_MGR_MapUriToPath(const char *uri)
{
    if (uri == NULL)
    {
        return NULL;
    }

    return HTTP_OM_ConfigMapUriToPath(uri);
}

/* FUNCTION NAME:  HTTP_MGR_LoadConfig
 * PURPOSE:
 *          Load config file from file_path.
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
BOOL_T HTTP_MGR_LoadConfig(const char *file_path)
{
    json_error_t error;
    json_t *config = NULL;

    config = json_load_file(file_path, 0, &error);

    if (config == NULL)
    {
        printf("error: line = %d, column = %d, msg = %s\r\n", error.line, error.column, error.text[0] != '\0' ? error.text : "(none)");
        return FALSE;
    }

    HTTP_MGR_SetConfigFromJson(config);

    HTTP_OM_UpdateConfigFilePointer(config);

    return TRUE;
}

/* FUNCTION NAME:  HTTP_MGR_GetConfigValue
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
void * HTTP_MGR_GetConfigValue(const char *key)
{
    return HTTP_OM_GetConfigValue(key);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : HTTP_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for http mgr.
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
BOOL_T HTTP_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    UI32_T  cmd;

    if(ipcmsg_p == NULL)
        return FALSE;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        HTTP_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;
        ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        return TRUE;
    }

    switch((cmd = HTTP_MGR_MSG_CMD(ipcmsg_p)))
    {
        case HTTP_MGR_IPC_CMD_GET_SECURE_HTTP_STATUS:
        {
            HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T *data_p = HTTP_MGR_MSG_DATA(ipcmsg_p);
            data_p->status = HTTP_MGR_Get_Secure_Http_Status();
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T);
            break;
        }

        case HTTP_MGR_IPC_CMD_GET_SECURE_HTTP_PORT:
        {
            HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T *data_p = HTTP_MGR_MSG_DATA(ipcmsg_p);
            HTTP_MGR_MSG_RETVAL(ipcmsg_p) = HTTP_MGR_Get_Secure_Port(&data_p->port);
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T);
            break;
        }

         case HTTP_MGR_IPC_CMD_SET_SECURE_HTTP_STATUS:
        {
            HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T *data_p = HTTP_MGR_MSG_DATA(ipcmsg_p);
            HTTP_MGR_MSG_RETVAL(ipcmsg_p) = HTTP_MGR_Set_Secure_Http_Status(data_p->status);
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case HTTP_MGR_IPC_CMD_SET_SECURE_HTTP_PORT:
        {
            HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T *data_p = HTTP_MGR_MSG_DATA(ipcmsg_p);
            HTTP_MGR_MSG_RETVAL(ipcmsg_p) = HTTP_MGR_Set_Secure_Port(data_p->port);
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case HTTP_MGR_IPC_CMD_GET_RUNNING_HTTP_SECURE_PORT:
        {
            HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T *data_p = HTTP_MGR_MSG_DATA(ipcmsg_p);
            HTTP_MGR_MSG_RETVAL(ipcmsg_p) = HTTP_MGR_Get_Running_Http_Secure_Port(&data_p->port);
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T);
            break;
        }

        case HTTP_MGR_IPC_CMD_GET_RUNNING_HTTP_SECURE_STATUS:
        {
            HTTP_MGR_IPCMsg_RUNNING_STATUS_Type_T *data_p = HTTP_MGR_MSG_DATA(ipcmsg_p);
            HTTP_MGR_MSG_RETVAL(ipcmsg_p) = HTTP_MGR_Get_Running_Secure_Http_Status(&data_p->running_status);
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_RUNNING_STATUS_Type_T);
            break;
        }

        case HTTP_MGR_IPC_CMD_GET_RUNNING_HTTP_PORT:
        {
            HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T *data_p = HTTP_MGR_MSG_DATA(ipcmsg_p);
            HTTP_MGR_MSG_RETVAL(ipcmsg_p) = HTTP_MGR_Get_Running_Http_Port(&data_p->port);
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T);
            break;
        }

        case HTTP_MGR_IPC_CMD_GET_RUNNING_HTTP_STATUS:
        {
            HTTP_MGR_IPCMsg_RUNNING_STATUS_Type_T *data_p = HTTP_MGR_MSG_DATA(ipcmsg_p);
            HTTP_MGR_MSG_RETVAL(ipcmsg_p) = HTTP_MGR_Get_Running_Http_Status(&data_p->running_status);
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_RUNNING_STATUS_Type_T);
            break;
        }

        case HTTP_MGR_IPC_CMD_GET_HTTP_PORT:
        {
            HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T *data_p = HTTP_MGR_MSG_DATA(ipcmsg_p);
            data_p->port = HTTP_MGR_Get_Http_Port();
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T);
            break;
        }

        case HTTP_MGR_IPC_CMD_GET_HTTP_STATUS:
        {
            HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T *data_p = HTTP_MGR_MSG_DATA(ipcmsg_p);
            data_p->status = HTTP_MGR_Get_Http_Status();
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T);
            break;
        }

        case HTTP_MGR_IPC_CMD_SET_HTTP_PORT:
        {
            HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T *data_p = HTTP_MGR_MSG_DATA(ipcmsg_p);
            HTTP_MGR_MSG_RETVAL(ipcmsg_p) = HTTP_MGR_Set_Http_Port(data_p->port);
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case HTTP_MGR_IPC_CMD_SET_HTTP_STATUS:
        {
            HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T *data_p = HTTP_MGR_MSG_DATA(ipcmsg_p);
            HTTP_MGR_MSG_RETVAL(ipcmsg_p) = HTTP_MGR_Set_Http_Status(data_p->status);
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case HTTP_MGR_IPC_CMD_GET_RUNNING_HTTP_DEBUG_INFO:
        {
            HTTP_MGR_IPCMsg_HTTP_DEBUG_Type_T *data_p = HTTP_MGR_MSG_DATA(ipcmsg_p);
            HTTP_MGR_MSG_RETVAL(ipcmsg_p) = HTTP_MGR_Get_Running_Debug_Information_Status(&data_p->http_debug);
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_DEBUG_Type_T);
            break;
        }

        case HTTP_MGR_IPC_CMD_GET_RUNNING_HTTP_PARAMETERS:
        {
            HTTP_MGR_IPCMsg_HTTP_RUNNING_CFG_Type_T *data_p = HTTP_MGR_MSG_DATA(ipcmsg_p);
            HTTP_MGR_MSG_RETVAL(ipcmsg_p) = HTTP_MGR_GetRunningHttpParameters(&data_p->http_runcfg_data);
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE(HTTP_MGR_IPCMsg_HTTP_RUNNING_CFG_Type_T);
            break;
        }

#if (SYS_CPNT_HTTPS == TRUE)

        case HTTP_MGR_IPC_CMD_SET_TFTP_DOWNLOAD_CERTIFICATE_FLAG:
        {
            HTTP_MGR_MSG_RETVAL(ipcmsg_p) = HTTP_MGR_Set_Tftp_Download_Certificate_Flag();
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case HTTP_MGR_IPC_CMD_SET_TFTP_IP:
        {
            HTTP_MGR_IPCMsg_TFTP_ADDR_Type_T *data_p = HTTP_MGR_MSG_DATA(ipcmsg_p);
            HTTP_MGR_MSG_RETVAL(ipcmsg_p) = HTTP_MGR_Set_Tftp_Ip(&data_p->server_ip);
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case HTTP_MGR_IPC_CMD_SET_TFTP_CERTIFICATE_FILENAME:
        {
            HTTP_MGR_IPCMsg_TFTP_SRC_FILENAME_Type_T *data_p = HTTP_MGR_MSG_DATA(ipcmsg_p);
            HTTP_MGR_MSG_RETVAL(ipcmsg_p) = HTTP_MGR_Set_Tftp_Certificate_Filename((char *)data_p->tftp_src_filename);
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case HTTP_MGR_IPC_CMD_SET_PRIVATEKEY_FILENAME:
        {
            HTTP_MGR_IPCMsg_TFTP_SRC_FILENAME_Type_T *data_p = HTTP_MGR_MSG_DATA(ipcmsg_p);
            HTTP_MGR_MSG_RETVAL(ipcmsg_p) = HTTP_MGR_Set_Tftp_Privatekey_Filename((char *)data_p->tftp_src_filename);
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case HTTP_MGR_IPC_CMD_SET_PRIVATEKEY_PASSWORD:
        {
            HTTP_MGR_IPCMsg_SERVER_PRIVATE_PWD_Type_T *data_p = HTTP_MGR_MSG_DATA(ipcmsg_p);
            HTTP_MGR_MSG_RETVAL(ipcmsg_p) = HTTP_MGR_Set_Tftp_Privatekey_Password((char *)data_p->passwd);
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case HTTP_MGR_IPC_CMD_GET_CERTIFICATE_FROM_TFTP:
        {
            HTTP_MGR_IPCMsg_DOWNLOAD_CERTIFICATE_T *data_p = HTTP_MGR_MSG_DATA(ipcmsg_p);
            HTTP_MGR_MSG_RETVAL(ipcmsg_p) = HTTP_MGR_Get_Certificate_From_Tftp(&data_p->certificate_entry);
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

#endif /* #if (SYS_CPNT_HTTPS == TRUE) */

        case HTTP_MGR_IPC_CMD_GETNEXT_USER_CONNECT_INFO:
        {
            HTTP_Session_T *data_p = HTTP_MGR_MSG_DATA(ipcmsg_p);
            HTTP_MGR_MSG_RETVAL(ipcmsg_p) = HTTP_MGR_Get_Next_User_Connection_Info(data_p);
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE(HTTP_Session_T);
            break;
        }

        case HTTP_MGR_IPC_CMD_SET_USER_CONNECT_SENDLOGFLAG:
        {
            HTTP_Session_T *data_p = HTTP_MGR_MSG_DATA(ipcmsg_p);
            HTTP_MGR_MSG_RETVAL(ipcmsg_p) = HTTP_MGR_Set_User_Connection_Send_Log(data_p->remote_ip,data_p->username,data_p->is_send_log);
            ipcmsg_p->msg_size = HTTP_MGR_GET_MSGBUFSIZE(HTTP_Session_T);
            break;
        }
        default:
        {
            HTTP_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;
            SYSFUN_Debug_Printf("*** %s(): Invalid cmd.\n", __FUNCTION__);
            return FALSE;
        }
    } /* switch ipcmsg_p->cmd */

    if (HTTP_MGR_MSG_RETVAL(ipcmsg_p) == FALSE)
    {
        SYSFUN_Debug_Printf("*** %s(): [cmd: %ld] failed\n", __FUNCTION__, cmd);
    }

    return TRUE;
}

/* FUNCTION NAME:  HTTP_fingerprint_hex
 * PURPOSE:
 *          Convert digest message to hex format.
 *
 * INPUT:
 *          UI8_T   *dgst_raw       --  original digest message using MD5 or SHA1.
 *          UI32_T  dgst_raw_len    --  length of digest message.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T - Number of bytes for seeding PRNG.
 * NOTES:
 *          .
 */
static UI8_T *HTTP_fingerprint_hex(UI8_T *dgst_raw, UI32_T dgst_raw_len)
{
    UI8_T   *retval;
    UI32_T  i;

    retval = (UI8_T*)L_MM_Malloc(dgst_raw_len * 3 + 1, L_MM_USER_ID2(SYS_MODULE_HTTP, HTTP_TYPE_TRACE_ID_HTTP_FINGERPRINT_HEX));
    retval[0] = '\0';
    for( i=0 ; i<dgst_raw_len ; i++ )
    {
        UI8_T   hex[4];
        sprintf((char *)hex, "%02x:", dgst_raw[i]);
        // FIXME: low preformance
        strncat((char *)retval, (const char *)hex, dgst_raw_len * 3);
    }
    retval[(dgst_raw_len * 3) - 1] = '\0';
    return retval;
}

/* get port number from sockaddr
 * returns -1 if failed
 */
static int HTTP_MGR_GetSockPort(struct sockaddr *sa)
{
    switch (sa->sa_family)
    {
    case AF_INET:
        {
            struct sockaddr_in  *sin = (struct sockaddr_in *) sa;

            return ntohs(sin->sin_port);
        }

    case AF_INET6:
        {
            struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;

            return ntohs(sin6->sin6_port);
        }
    }

    return(-1);
}

/* FUNCTION NAME:  HTTP_MGR_SetConfigFromJson
 * PURPOSE:
 *          Set config from json.
 *
 * INPUT:
 *          config -- the config in json format
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
static void HTTP_MGR_SetConfigFromJson(const json_t *config)
{
    if (config == NULL)
    {
        return;
    }

    HTTP_MGR_ResetConfigToDefault();

    HTTP_MGR_SetConfig(config, "rootDir",   HTTP_MGR_SetConfig_RootDir);

#if (SYS_CPNT_HTTPS == TRUE)
    HTTP_MGR_SetConfig(config, "https", HTTP_MGR_SetConfig_Https);
#endif // SYS_CPNT_HTTPS

    HTTP_MGR_SetConfig(config, "alias",     HTTP_MGR_SetConfig_Alias);
    HTTP_MGR_SetConfig(config, "logger",    HTTP_MGR_SetConfig_Logger);
    HTTP_MGR_SetConfig(config, "log.level", HTTP_MGR_SetConfig_LogLevel);
    HTTP_MGR_SetConfig(config, "log.type",  HTTP_MGR_SetConfig_LogType);
}

/* FUNCTION NAME:  HTTP_MGR_ResetConfigToDefault
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
static void HTTP_MGR_ResetConfigToDefault()
{
    HTTP_OM_ConfigResetToDefault();

    http_log_config_reset_to_default();
}

/* FUNCTION NAME:  HTTP_MGR_SetConfig
 * PURPOSE:
 *          Set config.
 *
 * INPUT:
 *          config  -- the config in json format
 *          key     -- the key in config
 *          fn      -- the function which is used to call for set related config
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
static void HTTP_MGR_SetConfig(const json_t *config, const char *key, void (*fn)(const json_t *))
{
    const json_t *param;

    ASSERT(config);
    ASSERT(key);
    ASSERT(fn);

    param = json_object_get(config, key);

    if (param)
    {
        fn(param);
    }
}

/* FUNCTION NAME:  HTTP_MGR_SetConfig_RootDir
 * PURPOSE:
 *          Set web root directory.
 *
 * INPUT:
 *          root_dir -- the config for root directory
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          None
 *
 * NOTES:
 *          Only allow string format.
 */
static void HTTP_MGR_SetConfig_RootDir(const json_t *root_dir)
{
    ASSERT(root_dir);

    if (json_is_string(root_dir))
    {
        HTTP_MGR_Set_Root_Dir(json_string_value(root_dir));
    }
}

#if (SYS_CPNT_HTTPS == TRUE)
static void HTTP_MGR_SetConfig_Https(const json_t *https)
{
    ASSERT(https);

    if (json_is_object(https))
    {
        json_t *cert_path = json_object_get(https, "certificatePath"); // required
        json_t *priv_path = json_object_get(https, "privateKeyPath");  // required
        json_t *pass_path = json_object_get(https, "passphrasePath");  // optional

        if (cert_path && priv_path && json_is_string(cert_path) && json_is_string(priv_path))
        {
            BOOL_T restart_https_server = FALSE;

            if (HTTP_OM_CONFIG_NO_CHANGE != HTTP_OM_Set_Https_Certificate_Path(json_string_value(cert_path)))
            {printf("%s: %d\r\n", __FUNCTION__, __LINE__);
                restart_https_server = TRUE;
            }

            if (HTTP_OM_CONFIG_NO_CHANGE != HTTP_OM_Set_Https_Private_Key_Path(json_string_value(priv_path)))
            {printf("%s: %d\r\n", __FUNCTION__, __LINE__);
                restart_https_server = TRUE;
            }

            if (pass_path && json_is_string(pass_path))
            {printf("%s: %d\r\n", __FUNCTION__, __LINE__);
                if (HTTP_OM_CONFIG_NO_CHANGE != HTTP_OM_Set_Https_Passphrase_Path(json_string_value(pass_path)))
                {printf("%s: %d\r\n", __FUNCTION__, __LINE__);
                    restart_https_server = TRUE;
                }
            }
        }
    }
}
#endif // SYS_CPNT_HTTPS

/* FUNCTION NAME:  HTTP_MGR_SetConfig_Alias
 * PURPOSE:
 *          Config alias.
 *
 * INPUT:
 *          alias -- the config for alias
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          None
 *
 * NOTES:
 *          1. Only allow array and each element has uri and path.
 *          2. Path is optional.
 *
 *          e.g. "alias": [ { "uri": "/xxx", "path": "/tmp" } ]
 */
static void HTTP_MGR_SetConfig_Alias(const json_t *alias)
{
    ASSERT(alias);

    if (json_is_array(alias))
    {
        int index;
        json_t *value;

        json_array_foreach(alias, index, value)
        {
            const json_t *element = json_array_get(alias, index);

            if (element && json_is_object(element))
            {
                const json_t *uri     = json_object_get(element, "uri");
                const json_t *path    = json_object_get(element, "path");

                if (!uri || !json_is_string(uri))
                {
                    continue;
                }

                // path is optional
                if (path && !json_is_string(path))
                {
                    continue;
                }

                HTTP_OM_ConfigAddAlias(json_string_value(uri), path ? json_string_value(path) : NULL);
            }
        }
    }
}

/* FUNCTION NAME:  HTTP_MGR_SetConfig_Logger
 * PURPOSE:
 *          Config logger.
 *
 * INPUT:
 *          logger -- the config for logger
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          None
 *
 * NOTES:
 *          Only allow string format.
 */
static void HTTP_MGR_SetConfig_Logger(const json_t *logger)
{
    const char *http_logger[] = {
        "SYSLOG",
        "CONSOLE",
        "FILE"
    };
    ASSERT(logger);

    if (json_is_string(logger))
    {
        int i;

        for (i = 0; i < _countof(http_logger); ++i)
        {
            if (strcasecmp(json_string_value(logger), http_logger[i]) == 0)
            {
                http_log_set_logger((HTTP_LOG_LOGGER_T) i);
                break;
            }
        }
    }
}

/* FUNCTION NAME:  HTTP_MGR_SetConfig_LogLevel
 * PURPOSE:
 *          Config log level.
 *
 * INPUT:
 *          level -- the config for log level
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          None
 *
 * NOTES:
 *          Allow integer and string format.
 */
static void HTTP_MGR_SetConfig_LogLevel(const json_t *level)
{
    ASSERT(level);

    if (json_is_integer(level))
    {
        http_log_set_level(json_integer_value(level));
    }
    else if (json_is_string(level))
    {
        http_log_set_level_by_string(json_string_value(level));
    }
}

/* FUNCTION NAME:  HTTP_MGR_SetConfig_LogType
 * PURPOSE:
 *          Config log types which you want to log.
 *
 * INPUT:
 *          type -- the config for log type
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          None
 *
 * NOTES:
 *          Allow array and string format.
 */
static void HTTP_MGR_SetConfig_LogType(const json_t *type)
{
    const char *http_log_err_msgtypes[] = {
        #define MSGTYPE(no, str)    str,
        #include "http_msgtype.h"
    };

    ASSERT(type);

    if (json_is_array(type))
    {
        int index;
        const json_t *value;
        HTTP_LOG_TYPE_T types;
        memset(types.enabled, 0, sizeof(types.enabled));

        json_array_foreach(type, index, value)
        {
            int i;

            if (!json_is_string(value))
            {
                continue;
            }

            for (i = 0; i < _countof(http_log_err_msgtypes); ++i)
            {
                if (strcasecmp(json_string_value(value), http_log_err_msgtypes[i]) == 0)
                {
                    HTTP_SET_BIT_ON(types.enabled, i);
                    break;
                }
            }
        }

        http_log_enable_debug(&types);
    }
    else if (json_is_string(type))
    {
        int i;
        HTTP_LOG_TYPE_T types;
        memset(types.enabled, 0, sizeof(types.enabled));

        for (i = 0; i < _countof(http_log_err_msgtypes); ++i)
        {
            if (strcasecmp(json_string_value(type), http_log_err_msgtypes[i]) == 0)
            {
                HTTP_SET_BIT_ON(types.enabled, i);
                break;
            }
        }

        http_log_enable_debug(&types);
    }
}
