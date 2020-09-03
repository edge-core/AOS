#include "cgi_auth.h"
#include "cgi_util.h"
#include "ntp_pmgr.h"
#include "ntp_mgr.h"
#include "leaf_es3626a.h"


//static BOOL_T CGI_MODULE_NTP_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request);

/**----------------------------------------------------------------------
 * This API is used to get NTP info.
 *
 * @param NA.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_NTP_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    NTP_MGR_SERVER_T last_server;
    char  last_server_ar[24] = {0};
    char  ip_str_ar[16] = {0};
    UI32_T client_status = 0;
    UI32_T auth_status = 0;
    UI32_T poll_time = 0;
    UI32_T real_time = 0;
    char date_time[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN + 1] = {0};

    memset(&last_server, 0, sizeof(last_server));

    if (TRUE != NTP_PMGR_GetStatus(&client_status))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "ntp.getNtpClientStatusError", "Failed to get status.");
    }

    if (TRUE != NTP_PMGR_GetAuthStatus(&auth_status))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "ntp.getNtpAuthStatusError", "Failed to get status.");
    }

    if (TRUE != NTP_PMGR_GetPollTime(&poll_time))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "ntp.getNtpPollingTimeError", "Failed to get polling time.");
    }

    if (TRUE != NTP_PMGR_GetLastUpdateServer(&last_server))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "ntp.getNtpLastUpdateServerError", "Failed to get update server.");
    }

    SYS_TIME_GetRealTimeBySec(&real_time);
    SYS_TIME_ConvertTime(real_time, date_time);
    L_INET_Ntoa(last_server.srcadr.sin_addr.s_addr, ip_str_ar);
    sprintf(last_server_ar, "%s:%d", ip_str_ar, last_server.srcadr.sin_port);

    json_object_set_new(result_p, "clientStatus", json_boolean((VAL_ntpStatus_enabled == client_status)));
    json_object_set_new(result_p, "authStatus", json_boolean((VAL_ntpAuthenticateStatus_enabled == auth_status)));
    json_object_set_new(result_p, "pollingInterval", json_integer(poll_time));
    json_object_set_new(result_p, "lastUpdateServer", json_string(last_server_ar));
    json_object_set_new(result_p, "currentTime", json_string(date_time));
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to update NTP info.
 *
 * @param clientStatus (required, boolean) NTP client status
 * @param authStatus (required, boolean) NTP auth status
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_NTP_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *client_status_p;
    json_t  *auth_status_p;
    UI32_T client_status = 0;
    UI32_T auth_status = 0;

    client_status_p = CGI_REQUEST_GetBodyValue(http_request, "clientStatus");
    auth_status_p = CGI_REQUEST_GetBodyValue(http_request, "authStatus");

    if ((NULL == client_status_p) && (NULL == auth_status_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Please specify clientStatus or authStatus value.");
    }

    if (NULL != client_status_p)
    {
        if (TRUE != json_is_boolean(client_status_p))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
        }

        if (TRUE == json_boolean_value(client_status_p))
        {
            client_status = VAL_ntpStatus_enabled;
        }
        else
        {
            client_status = VAL_ntpStatus_disabled;
        }

        if (TRUE != NTP_PMGR_SetStatus(client_status))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "ntp.setNtpClientStatusError", "Failled to set status.");
        }
    }

    if (NULL != auth_status_p)
    {
        if (TRUE != json_is_boolean(auth_status_p))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
        }

        if (TRUE == json_boolean_value(auth_status_p))
        {
            auth_status = VAL_ntpAuthenticateStatus_enabled;
        }
        else
        {
            auth_status = VAL_ntpAuthenticateStatus_disabled;
        }

        if (TRUE != NTP_PMGR_SetAuthStatus(auth_status))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "ntp.setNtpAuthStatusError", "Failled to set status.");
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to read all NTP servers.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_NTP_Servers_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *servers_p = json_array();
    char    ip_str_ar[16] = {0};
    UI32_T  ip = 0;
    UI32_T  version = 0;
    UI32_T  key_id = 0;

    json_object_set_new(result_p, "servers", servers_p);

    while (TRUE == NTP_PMGR_GetNextServer(&ip, &version, &key_id))
    {
        json_t *server_obj_p = json_object();

        L_INET_Ntoa(ip, ip_str_ar);
        json_object_set_new(server_obj_p, "id", json_string(ip_str_ar));
        json_object_set_new(server_obj_p, "ip", json_string(ip_str_ar));
        json_object_set_new(server_obj_p, "version", json_integer(version));

        if (0 != key_id)
        {
            json_object_set_new(server_obj_p, "key", json_integer(key_id));
        }

        json_array_append_new(servers_p, server_obj_p);
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to create NTP server.
 *
 * @param ip (required, string) NTP server address
 * @param key (option, number) NTP server auth key ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_NTP_Servers_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *ip_p;
    json_t  *key_p;
    char    *ip_str_p;
    UI32_T  server_ip = 0;
    UI32_T  key = VAL_ntpServerKey_no;

    ip_p = CGI_REQUEST_GetBodyValue(http_request, "ip");

    if (ip_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get IP.");
    }

    ip_str_p = (char *)json_string_value(ip_p);

    if (TRUE != CGI_UTIL_IpStrToInt(ip_str_p, &server_ip))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid IP.");
    }

    key_p = CGI_REQUEST_GetBodyValue(http_request, "key");

    if (key_p != NULL)
    {
        key = json_integer_value(key_p);
    }

    if(TRUE != NTP_PMGR_AddServerIp(server_ip, NTP_VERSION, key))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "ntp.serverSetError", "Failed to create NTP server.");
    }

    json_object_set_new(result_p, "id", ip_p);
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete NTP server.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_NTP_Servers_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);


    if (TRUE != NTP_PMGR_DeleteAllServerIp())
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "ntp.serversUnsetError", "The NTP deleting servers is failed.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to get NTP server info. for specifed NTP server ID.
 *
 * @param id        (required, string) Unique server address ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_NTP_Servers_ID_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    char    *ip_str_p;
    NTP_MGR_SERVER_T server_addr;
    UI8_T  ip_str_ar[16] = {0};
    UI32_T  server_ip = 0;

    memset(&server_addr, 0, sizeof(server_addr));
    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get id.");
    }

    ip_str_p = (char *)json_string_value(id_p);

    if (TRUE != CGI_UTIL_IpStrToInt(ip_str_p, &server_ip))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid IP.");
    }

    if (TRUE != NTP_PMGR_FindServer(server_ip, &server_addr))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "No such server.");
    }

    L_INET_Ntoa(server_ip, ip_str_ar);
    json_object_set_new(result_p, "id", json_string(ip_str_ar));
    json_object_set_new(result_p, "ip", json_string(ip_str_ar));
    json_object_set_new(result_p, "version", json_integer(server_addr.version));

    if (0 != server_addr.keyid)
    {
        json_object_set_new(result_p, "key", json_integer(server_addr.keyid));
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete NTP server.
 *
 * @param id        (required, string) Unique server address ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_NTP_Servers_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    char    *ip_str_p;
    UI32_T  server_ip = 0;

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get id.");
    }

    ip_str_p = (char *)json_string_value(id_p);

    if (TRUE != CGI_UTIL_IpStrToInt(ip_str_p, &server_ip))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid IP.");
    }

    if (TRUE != NTP_PMGR_DeleteServerIp(server_ip))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "ntp.serverUnsetError", "The NTP deleting server is failed.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to read all NTP authentications.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_NTP_Auth_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *auths_p = json_array();
    NTP_MGR_AUTHKEY_T *sk_p;

    json_object_set_new(result_p, "authKeys", auths_p);
    sk_p = (NTP_MGR_AUTHKEY_T *)malloc(sizeof(NTP_MGR_AUTHKEY_T));

    if (NULL != sk_p)
    {
        memset(sk_p, 0, sizeof(NTP_MGR_AUTHKEY_T));

        while(TRUE == NTP_PMGR_GetNextKey(sk_p))
        {
            json_t *key_obj_p = json_object();
            json_object_set_new(key_obj_p, "id", json_integer(sk_p->keyid));
            json_object_set_new(key_obj_p, "key", json_integer(sk_p->keyid));
            json_object_set_new(key_obj_p, "md5", json_string(sk_p->password));
            json_array_append_new(auths_p, key_obj_p);
        }

        free(sk_p);
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to create NTP auth key.
 *
 * @param key (required, number) auth key ID
 * @param md5 (required, string) auth key md5 string
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_NTP_Auth_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *key_p;
    json_t  *md5_p;
    char    *md5_str_p;
    UI32_T  key = VAL_ntpServerKey_no;

    key_p = CGI_REQUEST_GetBodyValue(http_request, "key");

    if (key_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get key.");
    }

    key = json_integer_value(key_p);
    md5_p = CGI_REQUEST_GetBodyValue(http_request, "md5");

    if (md5_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get md5.");
    }

    md5_str_p = (char *)json_string_value(md5_p);

    if(TRUE != NTP_PMGR_AddAuthKey(key, md5_str_p))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "ntp.authKeySetError", "Failed to create NTP auth key.");
    }

    json_object_set_new(result_p, "id", key_p);
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete all NTP auth key.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_NTP_Auth_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

    if (TRUE != NTP_PMGR_DeleteAllAuthKey())
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "ntp.authKeyUnsetError", "The NTP deleting keys is failed.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to get NTP auth key. for specifed NTP key ID.
 *
 * @param id        (required, number) Unique server key ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_NTP_Auth_ID_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    NTP_MGR_AUTHKEY_T *sk_p;
    UI32_T  key = VAL_ntpServerKey_no;

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get id.");
    }

    key = json_integer_value(id_p);

    sk_p = (NTP_MGR_AUTHKEY_T *)malloc(sizeof(NTP_MGR_AUTHKEY_T));

    if (NULL != sk_p)
    {
        memset(sk_p, 0, sizeof(NTP_MGR_AUTHKEY_T));

        if (TRUE == NTP_PMGR_FindKey(key, sk_p))
        {
            json_object_set_new(result_p, "id", json_integer(key));
            json_object_set_new(result_p, "key", json_integer(key));
            json_object_set_new(result_p, "md5", json_string(sk_p->password));
        }

        free(sk_p);
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete NTP auth key.
 *
 * @param id        (required, number) Unique server key ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_NTP_Auth_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    UI32_T  key = VAL_ntpServerKey_no;

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get id.");
    }

    key = json_integer_value(id_p);

    if (TRUE != NTP_PMGR_DeleteAuthKey(key))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "ntp.authKeyUnsetError", "The NTP deleting key is failed.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

void CGI_MODULE_NTP_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler   = CGI_MODULE_NTP_Read;
    handlers.update_handler = CGI_MODULE_NTP_Update;
    CGI_MAIN_Register("/api/v1/ntp", &handlers, 0);
}

void CGI_MODULE_NTP_Servers_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler   = CGI_MODULE_NTP_Servers_Read;
    handlers.create_handler = CGI_MODULE_NTP_Servers_Create;
    handlers.delete_handler = CGI_MODULE_NTP_Servers_Delete;
    CGI_MAIN_Register("/api/v1/ntp/servers", &handlers, 0);

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler   = CGI_MODULE_NTP_Servers_ID_Read;
        handlers.delete_handler = CGI_MODULE_NTP_Servers_ID_Delete;
        CGI_MAIN_Register("/api/v1/ntp/servers/{id}", &handlers, 0);
    }
}

void CGI_MODULE_NTP_Auth_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler   = CGI_MODULE_NTP_Auth_Read;
    handlers.create_handler = CGI_MODULE_NTP_Auth_Create;
    handlers.delete_handler = CGI_MODULE_NTP_Auth_Delete;
    CGI_MAIN_Register("/api/v1/ntp/authenticates", &handlers, 0);

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler   = CGI_MODULE_NTP_Auth_ID_Read;
        handlers.delete_handler = CGI_MODULE_NTP_Auth_ID_Delete;
        CGI_MAIN_Register("/api/v1/ntp/authenticates/{id}", &handlers, 0);
    }
}

static void CGI_MODULE_NTP_Init()
{
    CGI_MODULE_NTP_RegisterHandlers();
    CGI_MODULE_NTP_Servers_RegisterHandlers();
    CGI_MODULE_NTP_Auth_RegisterHandlers();
}

/*
static BOOL_T CGI_MODULE_NTP_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request)
{
    int nAuth;
    L_INET_AddrIp_T         rem_ip_addr;
    USERAUTH_AuthResult_T   auth_result;

    nAuth = cgi_check_pass(http_request->connection->conn_state, username, password,
                           &rem_ip_addr, &auth_result);

    if (nAuth < SYS_ADPT_MAX_LOGIN_PRIVILEGE )
    {
        return FALSE;
    }

    return TRUE;
}
*/
