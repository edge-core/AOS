#include "cgi_auth.h"
#include "cgi_util.h"
#include "syslog_mgr.h"
#include "syslog_pmgr.h"
#include "l_inet.h"


//static BOOL_T CGI_MODULE_LOGGING_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request);

/**----------------------------------------------------------------------
 * This API is used to read all remote logging servers.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_LOGGING_Servers_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
#if (SYS_CPNT_REMOTELOG == TRUE)
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *servers_p = json_array();
    SYSLOG_MGR_Remote_Server_Config_T server_config;
    char ip_str_ar[L_INET_MAX_IPADDR_STR_LEN+1] = {0};

    json_object_set_new(result_p, "servers", servers_p);
    memset(&server_config, 0, sizeof(server_config));

    while (SYSLOG_REMOTE_SUCCESS == SYSLOG_PMGR_GetNextRemoteLogServer(&server_config))
    {
        json_t *server_obj_p = json_object();

        if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&(server_config.ipaddr),
                                                           ip_str_ar,
                                                           sizeof(ip_str_ar)))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "logging.serverGetError", "Failed to convert server address.");
        }

        json_object_set_new(server_obj_p, "id", json_string(ip_str_ar));
        json_object_set_new(server_obj_p, "ip", json_string(ip_str_ar));

        if (SYS_DFLT_SYSLOG_HOST_PORT != server_config.udp_port)
        {
            json_object_set_new(server_obj_p, "port", json_integer(server_config.udp_port));
        }

        json_array_append_new(servers_p, server_obj_p);
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);

#else
    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "Unsupport remote logging server.");
#endif  /* #if (SYS_CPNT_REMOTELOG == TRUE) */
}

/**----------------------------------------------------------------------
 * This API is used to create remote logging server.
 *
 * @param ip (required, string) remote logging server address
 * @param port (option, number) remote logging server port
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_LOGGING_Servers_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
#if (SYS_CPNT_REMOTELOG == TRUE)
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *ip_p;
    json_t  *port_p;
    char    *ip_str_p;
    L_INET_AddrIp_T ip_address;
    UI32_T  port = 0;

    ip_p = CGI_REQUEST_GetBodyValue(http_request, "ip");

    if (ip_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get IP.");
    }

    ip_str_p = (char *)json_string_value(ip_p);

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(
            L_INET_FORMAT_IP_UNSPEC, ip_str_p, (L_INET_Addr_T *)&ip_address, sizeof(ip_address)))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid IP.");
    }

    if (SYSLOG_REMOTE_SUCCESS != SYSLOG_PMGR_CreateRemoteLogServer(&ip_address))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "logging.serverSetError", "Failed to create server.");
    }

    /* automatically enable remote logging server status
     */
    if (SYSLOG_REMOTE_SUCCESS != SYSLOG_PMGR_SetRemoteLogStatus(SYSLOG_STATUS_ENABLE))
    {
        SYSLOG_PMGR_DeleteRemoteLogServer(&ip_address);
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "logging.serverSetError", "Failed to enable remote status.");
    }

    port_p = CGI_REQUEST_GetBodyValue(http_request, "port");

    if (port_p != NULL)
    {
        port = json_integer_value(port_p);

        if (SYSLOG_REMOTE_SUCCESS != SYSLOG_PMGR_SetRemoteLogServerPort(&ip_address, port))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "logging.serverSetError", "Failed to set server port.");
        }
    }

    json_object_set_new(result_p, "id", ip_p);
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);

#else
    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "Unsupport remote logging server.");
#endif  /* #if (SYS_CPNT_REMOTELOG == TRUE) */
}

/**----------------------------------------------------------------------
 * This API is used to delete remote logging server.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_LOGGING_Servers_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
#if (SYS_CPNT_REMOTELOG == TRUE)
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

    if (SYSLOG_REMOTE_SUCCESS != SYSLOG_PMGR_SetRemoteLogStatus(SYSLOG_STATUS_DISABLE))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "logging.serverSetError", "Failed to disable remote status.");
    }

    SYSLOG_PMGR_DeleteAllRemoteLogServer();
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);

#else
    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "Unsupport remote logging server.");
#endif  /* #if (SYS_CPNT_REMOTELOG == TRUE) */
}

/**----------------------------------------------------------------------
 * This API is used to get remote logging server info. for specifed remote logging server ID.
 *
 * @param id        (required, string) Unique server address ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_LOGGING_Servers_ID_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
#if (SYS_CPNT_REMOTELOG == TRUE)
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    char    *ip_str_p;
    SYSLOG_MGR_Remote_Server_Config_T server_config;
    L_INET_AddrIp_T ip_address;

    memset(&server_config, 0, sizeof(server_config));
    memset(&ip_address, 0, sizeof(ip_address));
    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get id.");
    }

    ip_str_p = (char *)json_string_value(id_p);

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(
            L_INET_FORMAT_IP_UNSPEC, ip_str_p, (L_INET_Addr_T *)&ip_address, sizeof(ip_address)))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid IP.");
    }

    memcpy(&server_config.ipaddr, &ip_address, sizeof(server_config.ipaddr));

    if (SYSLOG_REMOTE_SUCCESS != SYSLOG_PMGR_GetRemoteLogServer(&server_config))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "No such server.");
    }

    json_object_set_new(result_p, "id", json_string(ip_str_p));
    json_object_set_new(result_p, "ip", json_string(ip_str_p));

    if (SYS_DFLT_SYSLOG_HOST_PORT != server_config.udp_port)
    {
        json_object_set_new(result_p, "port", json_integer(server_config.udp_port));
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);

#else
    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "Unsupport remote logging server.");
#endif  /* #if (SYS_CPNT_REMOTELOG == TRUE) */
}

/**----------------------------------------------------------------------
 * This API is used to delete remote logging server.
 *
 * @param id        (required, string) Unique server address ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_LOGGING_Servers_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
#if (SYS_CPNT_REMOTELOG == TRUE)
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    char    *ip_str_p;
    SYSLOG_MGR_Remote_Server_Config_T server_config;
    L_INET_AddrIp_T ip_address;

    memset(&server_config, 0, sizeof(server_config));
    memset(&ip_address, 0, sizeof(ip_address));
    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get id.");
    }

    ip_str_p = (char *)json_string_value(id_p);

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(
            L_INET_FORMAT_IP_UNSPEC, ip_str_p, (L_INET_Addr_T *)&ip_address, sizeof(ip_address)))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid IP.");
    }

    if (SYSLOG_REMOTE_SUCCESS != SYSLOG_PMGR_DeleteRemoteLogServer(&ip_address))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "logging.serverUnsetError", "The deleting server is failed.");
    }

    /* if no remote server exists, disable status
     */
    if (SYSLOG_REMOTE_SUCCESS != SYSLOG_PMGR_GetNextRemoteLogServer(&server_config))
    {
        SYSLOG_PMGR_SetRemoteLogStatus(SYSLOG_STATUS_DISABLE);
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);

#else
    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "Unsupport remote logging server.");
#endif  /* #if (SYS_CPNT_REMOTELOG == TRUE) */
}

void CGI_MODULE_LOGGING_Servers_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler   = CGI_MODULE_LOGGING_Servers_Read;
    handlers.create_handler = CGI_MODULE_LOGGING_Servers_Create;
    handlers.delete_handler = CGI_MODULE_LOGGING_Servers_Delete;
    CGI_MAIN_Register("/api/v1/logging/servers", &handlers, 0);

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler   = CGI_MODULE_LOGGING_Servers_ID_Read;
        handlers.delete_handler = CGI_MODULE_LOGGING_Servers_ID_Delete;
        CGI_MAIN_Register("/api/v1/logging/servers/{id}", &handlers, 0);
    }
}

static void CGI_MODULE_LOGGING_Init()
{
    CGI_MODULE_LOGGING_Servers_RegisterHandlers();
}

#if 0
static BOOL_T CGI_MODULE_LOGGING_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request)
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
#endif
