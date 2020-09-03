/* MODULE NAME:  http_om.c
 * PURPOSE:
 *   Initialize the database resource and provide some get/set functions for accessing the
 *   http database.
 *
 * NOTES:
 *
 * History:
 *       Date          -- Modifier,  Reason
 *     2002-02         -- Isiah , created.
 *
 * Copyright(C)      Accton Corporation, 2002
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "http_loc.h"
#include <pthread.h>
#include <jansson.h>

// FIXME: move out
#ifndef _countof
#define _countof(_Array) (sizeof(_Array)/sizeof(_Array[0]))
#endif

/* NAMING CONSTANT DECLARATIONS
 */

#if (SYS_CPNT_HTTPS == TRUE)
/*
 *  Maximum length of a DER encoded session.
 *  FIXME: There is no define in OpenSSL, but OpenSSL uses 1024*10,
 *         so this value should be ok. Although we have no warm feeling.
 */
/*#define SSL_SESSION_MAX_DER 1024*10*/

#endif /* if SYS_CPNT_HTTPS == TRUE */

#define HTTP_WEB_USER_CONNECTION_TIMEOUT    300


/* MACRO FUNCTION DECLARATIONS
 */

/* the following MACRO definitions work for gap between last_access_time and timeout is less than half of 2**32
 */
#define HTTP_OM_TIMEOUT32(last_access_time, timeout) ((I32_T)((UI32_T)(last_access_time) - (UI32_T)(timeout)) < 0 )

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI32_T              begin_index;
    UI32_T              matched_len;
} HTTP_OM_StringTestRes_T;

typedef struct
{
    char                root_dir[HTTP_CONFIG_ROOT_DIR_MAX_LEN + 1];
    char                file_path[HTTP_CONFIG_FILE_PATH_MAX_LEN + 1];   // FIXME: Is xx's file_path. It need to be clean.
    char                index_page[HTTP_CONFIG_INDEX_PAGE_MAX_LEN + 1];

    char                https_certificate_path[HTTP_CONFIG_HTTPS_CERTIFICATE_PATH_MAX_LEN + 1];
    char                https_private_key_path[HTTP_CONFIG_HTTPS_PARIVATE_KEY_PATH_MAX_LEN + 1];
    char                https_pass_phrase_path[HTTP_CONFIG_HTTPS_PASS_PHRASE_PATH_MAX_LEN + 1];

    struct
    {
        HTTP_Alias_T    list[HTTP_CONFIG_TOTAL_ALIAS];
        UI32_T          length;
    } alias;

    json_t             *raw_data;

} HTTP_OM_Config_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* FUNCTION NAME:  HTTP_OM_Local_ConfigUpdateAlias
 * PURPOSE:
 *          Update exist alias.
 *
 * INPUT:
 *          HTTP_Alias_T    *alias_p    -- the alias that you want to update
 *          const char      *uri        -- URI
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
static BOOL_T HTTP_OM_Local_ConfigUpdateAlias(HTTP_Alias_T *alias_p, const char *path);

/* FUNCTION NAME:  HTTP_OM_Local_ConfigLookupAlias
 * PURPOSE:
 *          Lookup alias by uri.
 *
 * INPUT:
 *          const char *uri -- URI
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          Return the alias or return NULL if not found.
 *
 * NOTES:
 *          None
 */
static const HTTP_Alias_T *HTTP_OM_Local_ConfigLookupAlias(const char *uri);

/* FUNCTION NAME:  HTTP_OM_Local_ConfigGetNextAlias
 * PURPOSE:
 *          Get next alias.
 *
 * INPUT:
 *          int *index  -- index, use -1 to get first alias.
 *
 * OUTPUT:
 *          int *index  -- current index
 *
 * RETURN:
 *          Return the alias or return NULL if no more alias.
 *
 * NOTES:
 *          None
 */
static const HTTP_Alias_T *HTTP_OM_Local_ConfigGetNextAlias(int *index);

/* FUNCTION NAME:  HTTP_OM_Local_ConfigFreeAlias
 * PURPOSE:
 *          Free alias.
 *
 * INPUT:
 *          HTTP_Alias_T *alias_p -- the alias that you want to free
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
static void HTTP_OM_Local_ConfigFreeAlias(HTTP_Alias_T *alias_p);

/* FUNCTION NAME:  HTTP_OM_Local_ConfigPackAliasList
 * PURPOSE:
 *          Pack up the list (speed up the search).
 *
 * INPUT:
 *          HTTP_Alias_T *alias_p -- the alias address that need to pack up
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
static void HTTP_OM_Local_ConfigPackAliasList(const HTTP_Alias_T *alias_p);

/* FUNCTION NAME:  HTTP_OM_Local_StringTest
 * PURPOSE:
 *          Check if uri is matches the pattern.
 *
 * INPUT:
 *          const char *uri
 *          const char *pattern
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          test result
 *
 * NOTES:
 *          None
 */
static HTTP_OM_StringTestRes_T HTTP_OM_Local_StringTest(const char *uri, const char *pattern);

/* FUNCTION NAME:  HTTP_OM_Local_ReplaceString
 * PURPOSE:
 *          Replace the matched pattern of original string with replacement.
 *
 * INPUT:
 *          const char *original    -- original string
 *          UI32_T      from        -- from which one char beginning to replace
 *          UI32_T      len         -- how much char to replace
 *          const char *replacement -- replace string
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          Return new buffer that allocated by calloc() or
 *          return NULL if operation failed.
 *
 * NOTES:
 *          None
 */
static char *HTTP_OM_Local_ReplaceString(const char *original, UI32_T from, UI32_T len, const char *replacement);

/* STATIC VARIABLE DECLARATIONS
 */


static UI32_T                       Http_Om_Port = HTTP_DEFAULT_PORT_NUMBER;
static HTTP_STATE_T                 Http_Om_Http_State = (HTTP_STATE_T) HTTP_DEFAULT_STATE;

#if (SYS_CPNT_HTTPS == TRUE)
static UI32_T                       Http_Default_Secure_Port = HTTP_DEFAULT_SECURE_PORT_NUMBER;
static UI32_T                       Session_Cache_Timeout = HTTP_DEFAULT_SESSION_CACHE_TIMEOUT;
static SECURE_HTTP_STATE_T          Http_Om_Secure_Http_State = (SECURE_HTTP_STATE_T) HTTP_DEFAULT_SECURE_HTTP_STATE;

static SSL_CTX                      *ssl_ctx = NULL;

static HTTPS_DEBUG_SESSION_STATE_T  Is_Debug_Session = HTTPS_DEBUG_SESSION_STATE_DISABLED;
static HTTPS_DEBUG_STATE_STATE_T    Is_Debug_State	= HTTPS_DEBUG_STATE_STATE_DISABLED;

static CONNECTION_STATE_T           Current_connection_status = HTTPS_CONNECTION;

static HTTP_Redirect_Status_T       Http_Om_Redirect_Http_State = (HTTP_Redirect_Status_T) HTTP_DEFAULT_REDIRECT_HTTP_STATE;
static HTTP_DownloadCertificateEntry_T  http_om_tftp_certificate_from_web;
static BOOL_T                       http_om_is_xfer_in_progress = FALSE;
static BOOL_T                       http_om_xfer_process_result = FALSE;
#endif /* if SYS_CPNT_HTTPS == TRUE */

#if (SYS_CPNT_CLUSTER == TRUE)
static UI32_T                       Http_Om_Cluster_Port = HTTP_DEFAULT_PORT_NUMBER;
#endif /* SYS_CPNT_CLUSTER */

/*isiah.2003-06-12. add for show web connection.*/
static UI32_T                       http_om_number_of_web_connection = 0;
static HTTP_Session_T               user_connection_info[HTTP_CFG_MAXWAIT];

#if (SYS_CPNT_HTTPS == TRUE)
static BOOL_T                       http_om_is_cert_change = TRUE;
#endif

static UI32_T                       http_om_semid = 0;

static pthread_mutex_t              http_om_mutex = PTHREAD_MUTEX_INITIALIZER;

static HTTP_LIST_T                  http_om_connection_free_list;
static HTTP_Connection_T            http_om_connection[HTTP_CFG_MAXWAIT];

static HTTP_Worker_T                http_om_workers[HTTP_CFG_TOTAL_WORKER];

static HTTP_LOG_DB_T                http_om_log_db;

static HTTP_OM_Config_T             http_om_config;


/* EXPORTED SUBPROGRAM BODIES
 */
#include "http_om_exp.c"

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:  HTTP_OM_InitateSystemResource
 * PURPOSE:
 *          Initiate system resource.
 *
 * INPUT:
 *          None
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
void HTTP_OM_InitateSystemResource(void)
{
    memset(http_om_workers, 0, sizeof(http_om_workers));

    HTTP_OM_InitLog();

    memset(&http_om_config, 0, sizeof(http_om_config));

    // TODO: shall be able to get from config file
    strncpy(http_om_config.index_page, HTTP_CFG_DFLT_INDEX_PAGE, sizeof(http_om_config.index_page) - 1);
    http_om_config.index_page[sizeof(http_om_config.index_page) - 1] = '\0';

    // FIXME: just memset to zero only ?? no need to set to default value.
    strncpy(http_om_config.file_path, HTTP_CFG_DFLT_CONFIG_FILE_PATH, sizeof(http_om_config.file_path) - 1);
    http_om_config.file_path[sizeof(http_om_config.file_path) - 1] = '\0';

    if (SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &http_om_semid) != SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateSem fail\r\n", __FUNCTION__, __LINE__);
        assert(0);
    }
}

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
void HTTP_OM_ClearConfig()
{
    UI32_T orig_priority;
    UI32_T i;

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    for (i = 0; i < _countof(http_om_config.alias.list); ++i)
    {
        HTTP_OM_Local_ConfigFreeAlias(&http_om_config.alias.list[i]);
    }

    http_om_config.alias.length = 0;

    json_decref(http_om_config.raw_data);
    http_om_config.raw_data = NULL;

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
}

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
void HTTP_OM_ConfigResetToDefault()
{
    UI32_T orig_priority;
    UI32_T i;

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    for (i = 0; i < _countof(http_om_config.alias.list); ++i)
    {
        HTTP_OM_Local_ConfigFreeAlias(&http_om_config.alias.list[i]);
    }

    http_om_config.alias.length = 0;

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
}

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
BOOL_T HTTP_OM_UpdateConfigFilePointer(const void *file)
{
    UI32_T orig_priority;

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    if (http_om_config.raw_data)
    {
        json_decref(http_om_config.raw_data);
    }

    http_om_config.raw_data = (json_t *)file;

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);

    return TRUE;
}

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
void *HTTP_OM_GetConfigValue(const char *key)
{
    UI32_T orig_priority;
    json_t *value = NULL;

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    if (http_om_config.raw_data)
    {
        json_t *found = json_object_get(http_om_config.raw_data, key);
        if (found)
        {
            value = json_deep_copy(found);
        }
    }

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);

    return value;
}

/* FUNCTION NAME:  HTTP_OM_Set_Root_Dir
 * PURPOSE:
 *          Set web root directory.
 *
 * INPUT:
 *          dir
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
BOOL_T HTTP_OM_Set_Root_Dir(const char *dir)
{
    UI32_T orig_priority;

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    strncpy(http_om_config.root_dir, dir, sizeof(http_om_config.root_dir) - 1);

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
    return TRUE;
}

/* FUNCTION NAME:  HTTP_OM_Set_Root_Dir
 * PURPOSE:
 *          Get web root directory.
 *
 * INPUT:
 *          dir
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
BOOL_T HTTP_OM_Get_Root_Dir(char *buf, size_t bufsz)
{
    UI32_T orig_priority;

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    memset(buf, 0, bufsz);
    strncpy(buf, http_om_config.root_dir, bufsz - 1);

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
    return TRUE;
}

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
HTTP_OM_ConfigState_T HTTP_OM_Set_Https_Certificate_Path(const char* path)
{
    UI32_T orig_priority;
    HTTP_OM_ConfigState_T ret = HTTP_OM_CONFIG_FAIL;

    ASSERT(path);
    if (!path)
    {
        return HTTP_OM_CONFIG_FAIL;
    }

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    if (strcmp(http_om_config.https_certificate_path, path) == 0)
    {
        ret = HTTP_OM_CONFIG_NO_CHANGE;
    }
    else
    {
        // FIXME: check length before copy
        strncpy(http_om_config.https_certificate_path, path, sizeof(http_om_config.https_certificate_path) - 1);
        http_om_config.https_certificate_path[sizeof(http_om_config.https_certificate_path) - 1] = '\0';

        ret = HTTP_OM_CONFIG_SUCCESS;
    }

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
    return ret;
}

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
BOOL_T HTTP_OM_Get_Https_Certificate_Path(char *buf, size_t bufsz)
{
    UI32_T orig_priority;

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    memset(buf, 0, bufsz);
    strncpy(buf, http_om_config.https_certificate_path, bufsz - 1);

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
    return TRUE;
}

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
HTTP_OM_ConfigState_T HTTP_OM_Set_Https_Private_Key_Path(const char* path)
{
    UI32_T orig_priority;
    HTTP_OM_ConfigState_T ret = HTTP_OM_CONFIG_FAIL;

    ASSERT(path);
    if (!path)
    {
        return HTTP_OM_CONFIG_FAIL;
    }

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    if (strcmp(http_om_config.https_private_key_path, path) == 0)
    {
        ret = HTTP_OM_CONFIG_NO_CHANGE;
    }
    else
    {
        // FIXME: check length before copy
        strncpy(http_om_config.https_private_key_path, path, sizeof(http_om_config.https_private_key_path) - 1);
        http_om_config.https_private_key_path[sizeof(http_om_config.https_private_key_path) - 1] = '\0';

        ret = HTTP_OM_CONFIG_SUCCESS;
    }

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
    return ret;
}

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
BOOL_T HTTP_OM_Get_Https_Private_Key_Path(char *buf, size_t bufsz)
{
    UI32_T orig_priority;

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    memset(buf, 0, bufsz);
    strncpy(buf, http_om_config.https_private_key_path, bufsz - 1);

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
    return TRUE;
}

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
HTTP_OM_ConfigState_T HTTP_OM_Set_Https_Passphrase_Path(const char* path)
{
    UI32_T orig_priority;
    HTTP_OM_ConfigState_T ret = HTTP_OM_CONFIG_FAIL;

    ASSERT(path);
    if (!path)
    {
        return HTTP_OM_CONFIG_FAIL;
    }

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    if (strcmp(http_om_config.https_pass_phrase_path, path) == 0)
    {
        ret = HTTP_OM_CONFIG_NO_CHANGE;
    }
    else
    {
        // FIXME: check length before copy
        strncpy(http_om_config.https_pass_phrase_path, path, sizeof(http_om_config.https_pass_phrase_path) - 1);
        http_om_config.https_pass_phrase_path[sizeof(http_om_config.https_pass_phrase_path) - 1] = '\0';

        ret = HTTP_OM_CONFIG_SUCCESS;
    }

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
    return ret;
}

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
BOOL_T HTTP_OM_Get_Https_Passphrase_Path(char *buf, size_t bufsz)
{
    UI32_T orig_priority;

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    memset(buf, 0, bufsz);
    strncpy(buf, http_om_config.https_pass_phrase_path, bufsz - 1);

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
    return TRUE;
}

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
void HTTP_OM_Set_Http_Status (HTTP_STATE_T state)
{
    UI32_T orig_priority;

	/* if same state, ignore */
	if (state == Http_Om_Http_State)
	{
		return;
	}

	/* set state */
	orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
	Http_Om_Http_State = state;
       HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
	return;
}

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
HTTP_STATE_T HTTP_OM_Get_Http_Status()
{
    UI32_T         orig_priority;
    HTTP_STATE_T   status;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    status = Http_Om_Http_State;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return status;
}

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
BOOL_T HTTP_OM_Set_Http_Port (UI32_T port)
{
    UI32_T orig_priority;

    /* set port */
    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    Http_Om_Port = port;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

/* FUNCTION NAME:  HTTP_OM_Get_Http_Port
 * PURPOSE:
 *			This function get http port number.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          *port_p -- HTTP port
 *
 * RETURN:
 *          TRUE
 * NOTES:
 *          default is tcp/80.
 */
UI32_T HTTP_OM_Get_Http_Port(void)
{
    UI32_T port;
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    port = Http_Om_Port;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return port;
}

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
void HTTP_OM_CleanAllConnectionObjects()
{
    UI32_T i;
    UI32_T orig_priority;

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    HTTP_LIST_Init(&http_om_connection_free_list, NULL);
    memset(http_om_connection, 0, sizeof(http_om_connection));

    for (i = 0; i < sizeof(http_om_connection)/sizeof(*http_om_connection); ++i)
    {
        HTTP_Connection_T *conn = &http_om_connection[i];

        conn->super.type = HTTP_INST_CONNECTION;

        conn->fds[HTTP_CONN_FD_NET] = -1;

        HTTP_LIST_InsertEnd(&http_om_connection_free_list, &conn->super);
    }

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
}

void HTTP_OM_ValidateConnectionPtr(HTTP_Connection_T *http_connection)
{
    int i;

    for (i = 0; i < _countof(http_om_connection); ++ i)
    {
        if (http_connection == &http_om_connection[i])
        {
            return;
        }
    }

    ASSERT(0);
}

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
HTTP_Connection_T * HTTP_OM_AllocateConnectionObject(UI32_T tid, int sockfd)
{
    HTTP_Connection_T   *conn = NULL;
    UI32_T              orig_priority;

    ASSERT(0 <= sockfd);

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    {
        HTTP_INSTANCE_T *list = &http_om_connection_free_list.list_ptr;

        if (list->links.last_node.in)
        {
            conn = (HTTP_Connection_T *)list->links.last_node.in;

            HTTP_LIST_Remove(&http_om_connection_free_list, (HTTP_INSTANCE_T *)conn);

            HTTP_OM_ValidateConnectionPtr(conn);
        }
    }

    if (conn != NULL)
    {
        HTTP_INSTANCE_T backup_super = conn->super;
        size_t size = offsetof(HTTP_Connection_T, stat);

        ASSERT(size == sizeof(HTTP_Connection_T) - sizeof(HTTP_CONNECTION_STAT_T));
        memset(conn, 0, size);

        conn->super = backup_super;

        conn->tid = tid;
        conn->fds[HTTP_CONN_FD_NET] = sockfd;

#if (SYS_CPNT_CLUSTER == TRUE)
        conn->is_relaying = FALSE;
        conn->socket_relay_id = -1;
#endif /* SYS_CPNT_CLUSTER */
    }

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
    return conn;
}

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
BOOL_T HTTP_OM_FreeConnectionObject(HTTP_Connection_T * http_connection)
{
    UI32_T orig_priority;

    HTTP_OM_ValidateConnectionPtr(http_connection);

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    HTTP_LIST_InsertEnd(&http_om_connection_free_list, &http_connection->super);

    {
        HTTP_INSTANCE_T backup_super = http_connection->super;
        size_t size = offsetof(HTTP_Connection_T, stat);

        ASSERT(size == sizeof(HTTP_Connection_T) - sizeof(HTTP_CONNECTION_STAT_T));
        memset(http_connection, 0xcc, size);

        http_connection->super = backup_super;
    }

    http_connection->fds[HTTP_CONN_FD_NET] = -1;
    http_connection->tid = 0;

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
    return TRUE;
}

/* FUNCTION NAME:  HTTP_OM_GetNextConnectionObjectAtIndex
 * PURPOSE:
 *          Enumerates all connection objects
 *
 * INPUT:
 *          tid    -- task ID
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
 BOOL_T HTTP_OM_GetNextConnectionObjectAtIndex(UI32_T tid, UI32_T *index, HTTP_Connection_T *http_connection)
{
    HTTP_Connection_T * obj_p;
    UI32_T i;
    UI32_T orig_priority;

    if ((NULL == index) || (NULL == http_connection))
    {
        return FALSE;
    }

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    if (0xffffffff == *index)
    {
        i = 0;
    }
    else
    {
        i = *index + 1;
    }

    for (; i < sizeof(http_om_connection)/sizeof(*http_om_connection); ++i)
    {
        obj_p = &http_om_connection[i];

        if (0 <= obj_p->fds[HTTP_CONN_FD_NET])
        {
            *index = i;
            memcpy(http_connection, obj_p, sizeof(*http_connection));
            HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
            return TRUE;
        }
    }

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
    return FALSE;
}

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
BOOL_T HTTP_OM_HasConnectionObject()
{
    UI32_T orig_priority;
    HTTP_Worker_T *worker = NULL;
    int i;

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    for (i = 0; i < _countof(http_om_workers); ++i)
    {
        worker = &http_om_workers[i];

        if (worker->connections.list_ptr.links.first_node.in)
        {
            HTTP_Connection_T *http_connection = (HTTP_Connection_T *) worker->connections.list_ptr.links.first_node.in;
            HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
            return TRUE;
        }
    }

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
    return FALSE;
}

HTTP_Worker_T * HTTP_OM_GetWorkerByIndex(UI32_T idx)
{
    UI32_T orig_priority;
    HTTP_Worker_T *worker = NULL;

    if (_countof(http_om_workers) <= idx)
    {
        return NULL;
    }

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    worker = &http_om_workers[idx];

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
    return worker;
}

HTTP_Worker_T * HTTP_OM_GetMasterWorker()
{
    UI32_T orig_priority;
    HTTP_Worker_T *worker = NULL;
    int i;

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    for (i = 0; i < _countof(http_om_workers); ++i)
    {
        if (http_om_workers[i].kind == HTTP_WORKER_MASTER)
        {
            worker = &http_om_workers[i];
            break;
        }
    }

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
    return worker;
}

void HTTP_OM_InitLog()
{
    memset(http_om_log_db.records, 0, sizeof(http_om_log_db.records));

    http_ring_buffer_init(&http_om_log_db.rb,
                 _countof(http_om_log_db.records),
                 sizeof(http_om_log_db.records[0]),
                 http_om_log_db.records);
}

void HTTP_OM_AddLog(struct tm *occurred_time, HTTP_LOG_LEVEL_T level, HTTP_LOG_MSG_TYPE_T message_type, const char *function, int line, const char *message)
{
    HTTP_LOG_RECORD_T   *rec = NULL;
    UI32_T              orig_priority;

    ASSERT(occurred_time != NULL);
    ASSERT(function != NULL);
    ASSERT(message != NULL);

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    rec = (HTTP_LOG_RECORD_T *) http_ring_buffer_new_element(&http_om_log_db.rb);
    ASSERT(rec != NULL &&
           rec == &http_om_log_db.records[
                            (http_om_log_db.rb.last_idx + _countof(http_om_log_db.records) - 1) % _countof(http_om_log_db.records)
                                          ]);

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);

    rec->occurred_time  = *occurred_time;
    rec->level          = level;
    rec->message_type   = message_type;

    strncpy(rec->function, function, sizeof(rec->function) - 1);
    rec->function[sizeof(rec->function) - 1] = '\0';

    rec->line           = line;

    strncpy(rec->message, message, sizeof(rec->message) - 1);
    rec->message[sizeof(rec->message) - 1] = '\0';
}

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
BOOL_T HTTP_OM_Set_Secure_Port (UI32_T port)
{
	UI32_T orig_priority;

	/* set port */
    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
	Http_Default_Secure_Port = port;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
	return TRUE;
}

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
BOOL_T HTTP_OM_Get_Secure_Port(UI32_T *port)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    *port = Http_Default_Secure_Port;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
void HTTP_OM_Set_Secure_Http_Status (SECURE_HTTP_STATE_T state)
{
    UI32_T orig_priority;

    /* if same state, ignore */
    if (state == Http_Om_Secure_Http_State)
    {
        return;
    }

    /* set state */
    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    Http_Om_Secure_Http_State = state;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return;
}

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
BOOL_T  HTTP_OM_Get_Secure_Http_Status(SECURE_HTTP_STATE_T *state_p)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    *state_p = Http_Om_Secure_Http_State;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
BOOL_T HTTP_OM_Set_SSL_CTX(SSL_CTX *set_ssl_ctx)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    ssl_ctx = set_ssl_ctx;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
SSL_CTX *HTTP_OM_Get_SSL_CTX()
{
    return ssl_ctx;
}

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
BOOL_T HTTP_OM_Get_SSL_Session_Cache_Timeout(UI32_T *timeout)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    *timeout = Session_Cache_Timeout;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
BOOL_T HTTP_OM_Set_SSL_Session_Cache_Timeout(UI32_T t)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    Session_Cache_Timeout = t;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
BOOL_T HTTP_OM_Set_Connection_Status(CONNECTION_STATE_T connection_status)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    Current_connection_status = connection_status;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
BOOL_T HTTP_OM_Get_Connection_Status(CONNECTION_STATE_T *state_p)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    *state_p = Current_connection_status;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
 	return TRUE;
}

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
BOOL_T HTTP_OM_Get_Debug_Status(HTTPS_DEBUG_SESSION_STATE_T *debug_session, HTTPS_DEBUG_STATE_STATE_T *debug_state)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
	*debug_session = Is_Debug_Session;
	*debug_state = Is_Debug_State;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
	return TRUE;
}

/* FUNCTION NAME:  HTTP_OM_Set_Debug_Status
 * PURPOSE:
 *          This function to set debug status -- Is_Debug_Session and Is_Debug_State.
 *
 * INPUT:
 *          HTTPS_DEBUG_SESSION_STATE_T  -- debug_session flag
 *			HTTPS_DEBUG_STATE_STATE_T    -- debug_state flag
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Set_Debug_Status(HTTPS_DEBUG_SESSION_STATE_T debug_session, HTTPS_DEBUG_STATE_STATE_T debug_state)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
	Is_Debug_Session = debug_session;
	Is_Debug_State = debug_state;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
	return TRUE;
}

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
BOOL_T HTTP_OM_Set_Auto_Redirect_To_Https_Status(HTTP_Redirect_Status_T status)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    Http_Om_Redirect_Http_State = status;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
	return TRUE;
}

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
BOOL_T HTTP_OM_Get_Auto_Redirect_To_Https_Status(HTTP_Redirect_Status_T *status)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    *status = Http_Om_Redirect_Http_State;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
void HTTP_OM_Init_Download_Certificate_Entry()
{
    memset(&http_om_tftp_certificate_from_web, 0, sizeof(http_om_tftp_certificate_from_web));
    http_om_tftp_certificate_from_web.setting = FALSE;
}

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
BOOL_T HTTP_OM_Set_Tftp_Ip(L_INET_AddrIp_T *tftp_server)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    memcpy(&http_om_tftp_certificate_from_web.tftp_server, tftp_server, sizeof(L_INET_AddrIp_T));
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

/* FUNCTION NAME:  HTTP_OM_Get_Tftp_Ip
 * PURPOSE:
 *          This function get tftp server will be download certificate.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T  *tftp_server    --  tftp server.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Get_Tftp_Ip(L_INET_AddrIp_T *tftp_server)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    memcpy(tftp_server, &http_om_tftp_certificate_from_web.tftp_server, sizeof(L_INET_AddrIp_T));
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
BOOL_T HTTP_OM_Set_Tftp_Certificate_Filename(const char *tftp_cert_file)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    strcpy(http_om_tftp_certificate_from_web.tftp_cert_file, tftp_cert_file);
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
BOOL_T HTTP_OM_Get_Tftp_Certificate_Filename(char *tftp_cert_file)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    strcpy(tftp_cert_file, http_om_tftp_certificate_from_web.tftp_cert_file);
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
BOOL_T HTTP_OM_Set_Tftp_Privatekey_Filename(const char *tftp_private_file)
{
     UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    strcpy(http_om_tftp_certificate_from_web.tftp_private_file, tftp_private_file);
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
BOOL_T HTTP_OM_Get_Tftp_Privatekey_Filename(char *tftp_private_file)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    strcpy(tftp_private_file, http_om_tftp_certificate_from_web.tftp_private_file);
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
BOOL_T HTTP_OM_Set_Tftp_Privatekey_Password(const char *tftp_private_password)
{
     UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    strcpy(http_om_tftp_certificate_from_web.tftp_private_password, tftp_private_password);
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
BOOL_T HTTP_OM_Set_Tftp_Cookie(void *cookie)
{
    UI32_T orig_priority;

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);
    http_om_tftp_certificate_from_web.cookie = cookie;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
BOOL_T HTTP_OM_Get_Tftp_Privatekey_Password(char *tftp_private_password)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    strcpy(tftp_private_password, http_om_tftp_certificate_from_web.tftp_private_password);
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
BOOL_T HTTP_OM_Get_Tftp_Cookie(void **cookie)
{
    UI32_T orig_priority;

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);
    *cookie = http_om_tftp_certificate_from_web.cookie;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}


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
BOOL_T HTTP_OM_Set_Tftp_Download_Certificate_Flag()
{
    BOOL_T ret;
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);

    if( http_om_tftp_certificate_from_web.setting == TRUE )
    {
        ret = FALSE;
    }
    else
    {
        http_om_tftp_certificate_from_web.setting = TRUE;
        ret = TRUE;
    }
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return ret;
}

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
BOOL_T HTTP_OM_Reset_Tftp_Download_Certificate_Flag()
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    http_om_tftp_certificate_from_web.setting = FALSE;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
BOOL_T HTTP_OM_Set_Certificate_Download_Status(HTTP_GetCertificateStatus_T status)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    http_om_tftp_certificate_from_web.status = status;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
BOOL_T HTTP_OM_Get_Certificate_Download_Status(HTTP_GetCertificateStatus_T *status)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    *status = http_om_tftp_certificate_from_web.status;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
void HTTP_OM_Set_XferProgressStatus(BOOL_T status)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    http_om_is_xfer_in_progress = status;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return;
}

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
void HTTP_OM_Get_XferProgressStatus(BOOL_T *status)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    *status = http_om_is_xfer_in_progress;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
}

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
void HTTP_OM_Set_XferProgressResult(BOOL_T result)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    http_om_xfer_process_result = result;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return;
}

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
void HTTP_OM_Get_XferProgressResult(BOOL_T *result)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    *result = http_om_xfer_process_result;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return;
}

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
BOOL_T HTTP_OM_Get_Certificate_Status(BOOL_T *is_change)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    *is_change = http_om_is_cert_change;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
	return TRUE;
}

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
BOOL_T HTTP_OM_Set_Certificate_Status(BOOL_T is_change)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    http_om_is_cert_change = is_change;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
	return TRUE;
}
#endif /* if SYS_CPNT_HTTPS == TRUE */
/*-----------------------------------------------------------------------*/

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
BOOL_T HTTP_OM_Clear_User_Connection_Info(void)
{
    UI32_T orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);
    http_om_number_of_web_connection = 0;
    memset(user_connection_info, 0, sizeof(user_connection_info));
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
BOOL_T HTTP_OM_Set_User_Connection_Info(L_INET_AddrIp_T *remote_ip_p, L_INET_AddrIp_T *local_ip_p, const char *username, UI32_T access_time, UI32_T protocol, int auth_level, const char *session_id)
{
    UI32_T  i,j;
    UI32_T  min_time, replace_index;
    UI32_T  orig_priority;

    if (0 == remote_ip_p->addrlen || 0 == local_ip_p->addrlen)
    {
        return FALSE;
    }

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);

    if ( http_om_number_of_web_connection >= HTTP_CFG_MAXWAIT )
    {
        replace_index = 0;
        min_time = user_connection_info[0].last_access_time;

        for( i=1 ; i<HTTP_CFG_MAXWAIT ; i++ )
        {
            if( user_connection_info[i].last_access_time < min_time )
            {
                replace_index = i;
                min_time = user_connection_info[i].last_access_time;
            }

            if (0 == strcmp(user_connection_info[i].session_id, session_id))
            {
                user_connection_info[i].last_access_time = access_time;
                HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
                return TRUE;
            }
        }

        for ( i=replace_index ; i<(HTTP_CFG_MAXWAIT-2) ; i++ )
        {
            memcpy(&user_connection_info[i], &user_connection_info[i+1], sizeof(user_connection_info[i]));
        }

        memset(&user_connection_info[(HTTP_CFG_MAXWAIT-1)], 0, sizeof(user_connection_info[(HTTP_CFG_MAXWAIT-1)]));

        user_connection_info[(HTTP_CFG_MAXWAIT-1)].protocol = UNKNOW_CONNECTION;
        user_connection_info[(HTTP_CFG_MAXWAIT-1)].is_send_log = FALSE;
        http_om_number_of_web_connection -= 1;
    }

    for ( i=0 ; i<http_om_number_of_web_connection ; i++ )
    {
        if (L_INET_CompareInetAddr((L_INET_Addr_T *) remote_ip_p,
            (L_INET_Addr_T *) &user_connection_info[i].remote_ip, 0) < 0)
        {
            for( j=http_om_number_of_web_connection ; j>i ; j-- )
            {
                memcpy(&user_connection_info[j], &user_connection_info[j-1], sizeof(user_connection_info[j]));
            }

            memcpy(&user_connection_info[i].remote_ip, remote_ip_p, sizeof(L_INET_AddrIp_T));
            memcpy(&user_connection_info[i].local_ip, local_ip_p, sizeof(L_INET_AddrIp_T));
            strcpy(user_connection_info[i].username, username);
            strncpy(user_connection_info[i].session_id, session_id,
                sizeof(user_connection_info[i].session_id)-1);
            user_connection_info[i].session_id[sizeof(user_connection_info[i].session_id)-1] = '\0';
            user_connection_info[i].auth_level = auth_level;
            user_connection_info[i].last_access_time = access_time;
            user_connection_info[i].protocol = protocol;
            user_connection_info[i].is_send_log = FALSE;
            http_om_number_of_web_connection += 1;
            HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
            return TRUE;
        }

        if (0 == strcmp(user_connection_info[i].session_id, session_id))
        {
            user_connection_info[i].last_access_time = access_time;
            HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
            return TRUE;
        }
    }

    memcpy(&user_connection_info[http_om_number_of_web_connection].remote_ip,
        remote_ip_p, sizeof(L_INET_AddrIp_T));
    memcpy(&user_connection_info[http_om_number_of_web_connection].local_ip,
        local_ip_p, sizeof(L_INET_AddrIp_T));

    strncpy(user_connection_info[http_om_number_of_web_connection].username, username,
            sizeof(user_connection_info[http_om_number_of_web_connection].username)-1);
    user_connection_info[http_om_number_of_web_connection].username[sizeof(user_connection_info[http_om_number_of_web_connection].username)-1] = '\0';

    strncpy(user_connection_info[http_om_number_of_web_connection].session_id, session_id,
        sizeof(user_connection_info[http_om_number_of_web_connection].session_id)-1);
    user_connection_info[http_om_number_of_web_connection].session_id[sizeof(user_connection_info[http_om_number_of_web_connection].session_id)-1] = '\0';

    user_connection_info[http_om_number_of_web_connection].auth_level = auth_level;
    user_connection_info[http_om_number_of_web_connection].last_access_time = access_time;
    user_connection_info[http_om_number_of_web_connection].protocol = protocol;
    user_connection_info[http_om_number_of_web_connection].is_send_log = FALSE;
    http_om_number_of_web_connection += 1;
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
BOOL_T HTTP_OM_Get_User_Connection_Info(const char *session_id, HTTP_Session_T *session)
{
    UI32_T i;
    UI32_T orig_priority;

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    for (i=0 ; i<http_om_number_of_web_connection; i++)
    {
        if (0 == strcmp(user_connection_info[i].session_id, session_id))
        {
            memcpy(session, &user_connection_info[i], sizeof(*session));

            HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
            return TRUE;
        }
    }

    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return FALSE;
}

static void HTTP_OM_Local_Set_User_As_Default_Value(HTTP_Session_T *session)
{
    memset(session, 0, sizeof(*session));
    session->protocol = UNKNOW_CONNECTION;
}

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
 *          FALSE to indicate no data or timeout.
 * NOTES:
 *          .
 */
BOOL_T HTTP_OM_Check_User_Connection_Info(UI32_T access_time, const char *session_id)
{
    UI32_T  i;
    UI32_T  timeout; /* can allow less than 0 as UI32_T */
    UI32_T  orig_priority;

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

#ifdef SYS_ADPT_HTTP_WEB_USER_CONNECTION_TIMEOUT
    timeout = access_time - SYS_ADPT_HTTP_WEB_USER_CONNECTION_TIMEOUT;
#else
    timeout = access_time - (HTTP_WEB_USER_CONNECTION_TIMEOUT*100);
#endif

    for ( i=0 ; i < http_om_number_of_web_connection ; i++ )
    {
        if (0 == strcmp(user_connection_info[i].session_id, session_id))
        {
            if (TRUE == HTTP_OM_TIMEOUT32(user_connection_info[i].last_access_time, timeout))
            {
                memmove(&(user_connection_info)[i],
                    &(user_connection_info)[(i + 1)],
                    (_countof(user_connection_info) - (i + 1)) * sizeof(user_connection_info[0]));

                HTTP_OM_Local_Set_User_As_Default_Value(&user_connection_info[_countof(user_connection_info) - 1]);
                http_om_number_of_web_connection -= 1;

                HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
                return FALSE;
            }
            else
            {
                /* not timeout, update last_access_time
                 */
                user_connection_info[i].last_access_time = access_time;
                HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
                return TRUE;
            }
        }
    }

    /* no data
     */
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return FALSE;
}

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
BOOL_T HTTP_OM_Delete_User_Connection_Info_If_Timeout(UI32_T access_time, const char *session_id)
{
    UI32_T  i;
    UI32_T  timeout; /* can allow less than 0 as UI32_T */
    UI32_T  orig_priority;

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

#ifdef SYS_ADPT_HTTP_WEB_USER_CONNECTION_TIMEOUT
    timeout = access_time - SYS_ADPT_HTTP_WEB_USER_CONNECTION_TIMEOUT;
#else
    timeout = access_time - (HTTP_WEB_USER_CONNECTION_TIMEOUT*100);
#endif

    for ( i=0 ; i < http_om_number_of_web_connection ; i++ )
    {
        if (0 == strcmp(user_connection_info[i].session_id, session_id))
        {
            if (TRUE == HTTP_OM_TIMEOUT32(user_connection_info[i].last_access_time, timeout))
            {
                memmove(&(user_connection_info)[i],
                        &(user_connection_info)[(i + 1)],
                        (_countof(user_connection_info) - (i + 1)) * sizeof(user_connection_info[0]));

                HTTP_OM_Local_Set_User_As_Default_Value(&user_connection_info[_countof(user_connection_info) - 1]);
                http_om_number_of_web_connection -= 1;

                HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
                return FALSE;
            }

            /* do nothing
             */
            break;
        }
    }

    /* no data
     */
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
BOOL_T HTTP_OM_Delete_User_Connection_Info(const char *session_id)
{
    UI32_T  i;
    UI32_T  orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);

    for ( i=0 ; i<http_om_number_of_web_connection ; i++ )
    {
        if (0 == strcmp(user_connection_info[i].session_id, session_id))
        {
            memmove(&(user_connection_info)[i],
                    &(user_connection_info)[(i + 1)],
                    (_countof(user_connection_info) - (i + 1)) * sizeof(user_connection_info[0]));

            HTTP_OM_Local_Set_User_As_Default_Value(&user_connection_info[_countof(user_connection_info) - 1]);
            http_om_number_of_web_connection -= 1;

            HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
            return TRUE;
        }
    }

    /* no data
     */
    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return TRUE;
}

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
BOOL_T HTTP_OM_Set_User_Connection_Send_Log(L_INET_AddrIp_T ip_addr, const char *username, BOOL_T is_send_log)
{
    UI32_T  i;
    UI32_T  orig_priority;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);

    for ( i=0 ; i<http_om_number_of_web_connection ; i++ )
    {
        if ( (L_INET_CompareInetAddr((L_INET_Addr_T *) &user_connection_info[i].remote_ip,
              (L_INET_Addr_T *) &ip_addr, 0) == 0)
             && (strcmp(user_connection_info[i].username, username) == 0) )
        {
            user_connection_info[i].is_send_log = is_send_log;
            HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
            return TRUE;
        }
    }

    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return FALSE;
}

/* FUNCTION NAME:  HTTP_OM_Get_Next_User_Connection_Info
 * PURPOSE:
 *          This function get next connection info.
 *
 * INPUT:
 *          HTTP_Session_T   *connection_info    --  previous active connection information.
 *          UI32_T           current_time        --  current time.
 *
 * OUTPUT:
 *          HTTP_Session_T   *connection_info    --  current active connection information.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *      Initial input value is zero.
 */
BOOL_T HTTP_OM_Get_Next_User_Connection_Info(HTTP_Session_T *session, UI32_T current_time)
{
    UI32_T  i;
    UI32_T  timeout; /* can allow less than 0 as UI32_T */
    UI32_T  orig_priority;
    BOOL_T  ret = FALSE;

    orig_priority=HTTP_OM_EnterCriticalSection(http_om_semid);

    if (session == NULL || http_om_number_of_web_connection == 0)
    {
        memset(session, 0, sizeof(HTTP_Session_T));
        HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
        return ret;
    }

#ifdef SYS_ADPT_HTTP_WEB_USER_CONNECTION_TIMEOUT
    timeout = current_time - SYS_ADPT_HTTP_WEB_USER_CONNECTION_TIMEOUT;
#else
    timeout = current_time - (HTTP_WEB_USER_CONNECTION_TIMEOUT*100);
#endif

    /* delete timeout sessions
     */
    for (i = 0; i < http_om_number_of_web_connection; ++ i)
    {
        if (TRUE == HTTP_OM_TIMEOUT32(user_connection_info[i].last_access_time, timeout))
        {
            memmove(&(user_connection_info)[i],
                &(user_connection_info)[(i + 1)],
                (_countof(user_connection_info) - (i + 1)) * sizeof(user_connection_info[0]));

            HTTP_OM_Local_Set_User_As_Default_Value(&user_connection_info[_countof(user_connection_info) - 1]);
            http_om_number_of_web_connection -= 1;
        }
    }

    if (session->session_id[0] == '\0')
    {
        if (user_connection_info[0].session_id[0] == '\0')
        {
            memset(session, 0, sizeof(HTTP_Session_T));
            ret = FALSE;
        }
        else
        {
            memcpy(session, &user_connection_info[0], sizeof(HTTP_Session_T));
            ret = TRUE;
        }
    }
    else
    {
        for (i = 0; i < http_om_number_of_web_connection; ++ i)
        {
            if (0 == strcmp(user_connection_info[i].session_id, session->session_id))
            {
                if ((i + 1) == http_om_number_of_web_connection)
                {
                    memset(session, 0, sizeof(HTTP_Session_T));
                    ret = FALSE;
                    break;
                }

                memcpy(session, &user_connection_info[i + 1], sizeof(HTTP_Session_T));
                ret = TRUE;
                break;
            }
        }
    }

    HTTP_OM_LeaveCriticalSection(http_om_semid,orig_priority);
    return ret;
}

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
BOOL_T HTTP_OM_Set_Cluster_Port(UI32_T port)
{
    UI32_T orig_priority;

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);
    Http_Om_Cluster_Port = port;
    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
    return TRUE;
}

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
UI32_T HTTP_OM_Get_Cluster_Port(void)
{
    UI32_T port;
    UI32_T orig_priority;

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);
    port = Http_Om_Cluster_Port;
    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
    return port;
}
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
BOOL_T HTTP_OM_ConfigAddAlias(const char *uri, const char *path)
{
    UI32_T  orig_priority;
    BOOL_T  ret;
    HTTP_Alias_T *alias_p = NULL;

    if (uri == NULL || uri[0] == '\0')
    {
        return FALSE;
    }

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    alias_p = (HTTP_Alias_T *) HTTP_OM_Local_ConfigLookupAlias(uri);

    if (alias_p)
    {
        ret = HTTP_OM_Local_ConfigUpdateAlias(alias_p, path);
        HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
        return ret;
    }

    if (http_om_config.alias.length == _countof(http_om_config.alias.list))
    {
        HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
        return FALSE;
    }

    ASSERT(http_om_config.alias.length < _countof(http_om_config.alias.list));
    alias_p = &http_om_config.alias.list[http_om_config.alias.length];

    alias_p->uri = strdup(uri);

    if (alias_p->uri == NULL)
    {
        HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
        return FALSE;
    }

    if (path)
    {
        alias_p->path = strdup(path);

        if (alias_p->path == NULL)
        {
            HTTP_OM_Local_ConfigFreeAlias(alias_p);

            HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
            return FALSE;
        }
    }

    http_om_config.alias.length ++;
    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
    return TRUE;
}

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
BOOL_T HTTP_OM_ConfigDeleteAlias(const char *uri)
{
    UI32_T orig_priority;
    const HTTP_Alias_T *alias_p;

    if (uri == NULL)
    {
        return FALSE;
    }

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    alias_p = HTTP_OM_Local_ConfigLookupAlias(uri);

    if (alias_p)
    {
        HTTP_OM_Local_ConfigFreeAlias((HTTP_Alias_T *)alias_p);

        ASSERT(0 < http_om_config.alias.length && http_om_config.alias.length <= _countof(http_om_config.alias.list));

        http_om_config.alias.length --;

        ASSERT(0 <= http_om_config.alias.length && http_om_config.alias.length <= _countof(http_om_config.alias.list));

        HTTP_OM_Local_ConfigPackAliasList(alias_p);
    }

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
    return TRUE;
}

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
BOOL_T HTTP_OM_ConfigGetNextAlias(int *index, HTTP_Alias_T *alias_p)
{
    UI32_T orig_priority;
    const HTTP_Alias_T *local_alias_p = NULL;

    if (index == NULL || alias_p == NULL)
    {
        return FALSE;
    }

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    while ((local_alias_p = HTTP_OM_Local_ConfigGetNextAlias(index)) != NULL)
    {
        memcpy(alias_p, local_alias_p, sizeof(*alias_p));

        HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
        return TRUE;
    }

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
    return FALSE;
}

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
char *HTTP_OM_ConfigMapUriToPath(const char *uri)
{
    int     i = -1;
    UI32_T  orig_priority;
    const HTTP_Alias_T *alias_p;
    char *file_path_p = NULL;

    if (uri == NULL)
    {
        return NULL;
    }

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    while ((alias_p = HTTP_OM_Local_ConfigGetNextAlias(&i)) != NULL)
    {
        HTTP_OM_StringTestRes_T tr;

        ASSERT(alias_p->uri);

        tr = HTTP_OM_Local_StringTest(uri, alias_p->uri);

        if (0 < tr.matched_len)
        {
            if (alias_p->path == NULL)
            {
                file_path_p = HTTP_OM_Local_ReplaceString(uri, 0, 0, "");

                HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
                return file_path_p;
            }

            file_path_p = HTTP_OM_Local_ReplaceString(uri, tr.begin_index, tr.matched_len, alias_p->path);

            HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
            return file_path_p;
        }
    }

    if (strcmp(uri, "/") == 0)
    {
        char index_path[sizeof(http_om_config.root_dir) + sizeof(http_om_config.index_page) + 1];

        index_path[sizeof(index_path) - 1] = '\0';

        snprintf(index_path, sizeof(index_path), "%s%s", http_om_config.root_dir, http_om_config.index_page);

        if (index_path[sizeof(index_path) - 1] == '\0')
        {
            file_path_p = HTTP_OM_Local_ReplaceString(uri, 0, 1, index_path);
        }
    }
    else
    {
        file_path_p = HTTP_OM_Local_ReplaceString(uri, 0, 0, http_om_config.root_dir);
    }

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
    return file_path_p;
}

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
BOOL_T HTTP_OM_SetConfigFilePath(const char *file_path)
{
    UI32_T orig_priority;
    char dup_file_path[sizeof(http_om_config.file_path)];

    if (file_path == NULL)
    {
        return FALSE;
    }

    memset(dup_file_path, 0, sizeof(dup_file_path));

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    strncpy(dup_file_path, file_path, sizeof(dup_file_path));
    if (dup_file_path[sizeof(dup_file_path) - 1] != '\0')
    {
        HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);
        return FALSE;
    }

    strncpy(http_om_config.file_path, file_path, sizeof(http_om_config.file_path) - 1);
    http_om_config.file_path[sizeof(http_om_config.file_path) - 1] = '\0';

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);

    return TRUE;
}

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
BOOL_T HTTP_OM_GetConfigFilePath(char *buf, UI32_T buf_len)
{
    UI32_T orig_priority;

    if (buf == NULL || buf_len <= 1)
    {
        return FALSE;
    }

    orig_priority = HTTP_OM_EnterCriticalSection(http_om_semid);

    buf[buf_len - 1] = '\0';
    strncpy(buf, http_om_config.file_path, buf_len);

    HTTP_OM_LeaveCriticalSection(http_om_semid, orig_priority);

    if (buf[buf_len - 1] != '\0')
    {
        buf[0] = '\0';
        return FALSE;
    }

    return TRUE;
}

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  HTTP_OM_Local_ConfigUpdateAlias
 * PURPOSE:
 *          Update exist alias.
 *
 * INPUT:
 *          HTTP_Alias_T    *alias_p    -- the alias that you want to update
 *          const char      *uri        -- URI
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
static BOOL_T HTTP_OM_Local_ConfigUpdateAlias(HTTP_Alias_T *alias_p, const char *path)
{
    char *dup_path = NULL;

    ASSERT(alias_p);

    if (path)
    {
        dup_path = strdup(path);
        if (dup_path == NULL)
        {
            return FALSE;
        }
    }

    if (alias_p->path)
    {
        free(alias_p->path);
    }

    alias_p->path = dup_path;

    return TRUE;
}

/* FUNCTION NAME:  HTTP_OM_Local_ConfigLookupAlias
 * PURPOSE:
 *          Lookup alias by uri.
 *
 * INPUT:
 *          const char *uri -- URI
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          Return the alias or return NULL if not found.
 *
 * NOTES:
 *          None
 */
static const HTTP_Alias_T *HTTP_OM_Local_ConfigLookupAlias(const char *uri)
{
    int i = -1;
    const HTTP_Alias_T *alias_p;

    ASSERT(uri);

    while ((alias_p = HTTP_OM_Local_ConfigGetNextAlias(&i)) != NULL)
    {
        ASSERT(alias_p->uri);

        if (strcmp(alias_p->uri, uri) == 0)
        {
            return alias_p;
        }
    }

    return NULL;
}

/* FUNCTION NAME:  HTTP_OM_Local_ConfigGetNextAlias
 * PURPOSE:
 *          Get next alias.
 *
 * INPUT:
 *          int *index  -- index, use -1 to get first alias.
 *
 * OUTPUT:
 *          int *index  -- current index
 *
 * RETURN:
 *          Return the alias or return NULL if no more alias.
 *
 * NOTES:
 *          None
 */
static const HTTP_Alias_T *HTTP_OM_Local_ConfigGetNextAlias(int *index)
{
    ASSERT(index);

    if (*index < 0)
    {
        *index = 0;
    }
    else
    {
        *index += 1;
    }

    for (; *index < _countof(http_om_config.alias.list); ++(*index))
    {
        if (http_om_config.alias.list[*index].uri)
        {
            return &http_om_config.alias.list[*index];
        }
    }

    return NULL;
}

/* FUNCTION NAME:  HTTP_OM_Local_ConfigFreeAlias
 * PURPOSE:
 *          Free alias.
 *
 * INPUT:
 *          HTTP_Alias_T *alias_p -- the alias that you want to free
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
static void HTTP_OM_Local_ConfigFreeAlias(HTTP_Alias_T *alias_p)
{
    if (alias_p)
    {
        free(alias_p->uri);
        free(alias_p->path);
        memset(alias_p, 0, sizeof(*alias_p));
    }
}

/* FUNCTION NAME:  HTTP_OM_Local_ConfigPackAliasList
 * PURPOSE:
 *          Pack up the list (speed up the search).
 *
 * INPUT:
 *          HTTP_Alias_T *alias_p -- the alias address that need to pack up
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
static void HTTP_OM_Local_ConfigPackAliasList(const HTTP_Alias_T *alias_p)
{
    UI32_T i;
    UI32_T last;

    ASSERT(alias_p);

    last = _countof(http_om_config.alias.list) - 1;

    ASSERT(http_om_config.alias.list <= alias_p);
    ASSERT(alias_p <= &http_om_config.alias.list[last]);

    i = ((HTTP_Alias_T *)alias_p - http_om_config.alias.list);
    ASSERT(i < _countof(http_om_config.alias.list));

    memmove(&(http_om_config.alias.list)[i],
            &(http_om_config.alias.list)[(i + 1)],
            (_countof(http_om_config.alias.list)- (i + 1)) * sizeof(http_om_config.alias.list[0]));

    memset(&http_om_config.alias.list[last], 0, sizeof(http_om_config.alias.list[last]));
}

/* FUNCTION NAME:  HTTP_OM_Local_StringTest
 * PURPOSE:
 *          Check if uri is matches the pattern.
 *
 * INPUT:
 *          const char *uri
 *          const char *pattern
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          test result
 *
 * NOTES:
 *          None
 */
static HTTP_OM_StringTestRes_T HTTP_OM_Local_StringTest(const char *uri, const char *pattern)
{
    HTTP_OM_StringTestRes_T tr = {0};
    int     r;
    size_t  pattern_len;

    ASSERT(uri);
    ASSERT(pattern);

    pattern_len = strlen(pattern);
    r = strncmp(uri, pattern, pattern_len);

    if (r == 0)
    {
        tr.begin_index = 0;
        tr.matched_len = pattern_len;
    }

    return tr;
}

/* FUNCTION NAME:  HTTP_OM_Local_ReplaceString
 * PURPOSE:
 *          Replace a part of original string with replacement.
 *
 * INPUT:
 *          const char *original    -- original string
 *          UI32_T      from        -- from which one char beginning to replace
 *          UI32_T      len         -- how much char to replace
 *          const char *replacement -- replace string
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          Return new buffer that allocated by calloc() or
 *          return NULL if operation failed.
 *
 * NOTES:
 *          None
 */
static char *HTTP_OM_Local_ReplaceString(const char *original, UI32_T from, UI32_T len, const char *replacement)
{
    size_t  new_strlen;
    size_t  orig_len;
    size_t  replace_len;
    char   *new_str;
    char   *p;

    ASSERT(original);
    ASSERT(replacement);

    orig_len = strlen(original);
    if (orig_len < from)
    {
        return NULL;
    }

    if (orig_len < from + len)
    {
        len = orig_len - from;
    }

    ASSERT(from + len <= orig_len);

    replace_len = strlen(replacement);

    new_strlen = orig_len - len + replace_len;
    new_str = (char *)calloc(new_strlen + 1 + 2, 1);

    if (new_str == NULL) {
        return NULL;
    }

    new_str[new_strlen + 1] = (unsigned char)0xaa;  // boundary check
    new_str[new_strlen + 2] = (unsigned char)0xbb;  // boundary check

    p = new_str;

    strncat(p, original, from);
    p += from;
    strcat(p, replacement);
    p += replace_len;
    strcat(p, original + from + len);

    ASSERT(new_str[new_strlen + 0] == '\0');
    ASSERT((unsigned char)new_str[new_strlen + 1] == 0xaa);
    ASSERT((unsigned char)new_str[new_strlen + 2] == 0xbb);

    return new_str;
}
