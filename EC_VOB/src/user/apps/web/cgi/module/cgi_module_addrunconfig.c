#include "cli_pmgr.h"
#include "cgi_auth.h"
#include "cgi_request.h"

static BOOL_T CGI_MODULE_ADDRUNCONFIG_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request);
static BOOL_T CGI_MODULE_ADDRUNCONFIG_Join(json_t *commands, char *buf, size_t buf_len, char c);
static void CGI_MODULE_ADDRUNCONFIG_SetResult(json_t *result, CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T entry);

/**----------------------------------------------------------------------
 * This API is used to update addrunconfig entry.
 *
 * @param username (required, string) username for authenticating
 * @param password (required, string) password for authenticating
 * @param commands (required, array)  This string array contains CLI commands
 *                                    to update the running-config on the device
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ADDRUNCONFIG_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    char cmd_str[CLI_OM_EXP_ADDRUNCONFIG_COMMANDS_MAX_LEN + 1];
    json_t *username, *password, *commands;
    const char *username_value, *password_value;
    UI32_T id;
    CLI_OM_EXP_ADDRUNCONFIG_RETURN_T ret;

    username = CGI_REQUEST_GetBodyValue(http_request, "username");
    password = CGI_REQUEST_GetBodyValue(http_request, "password");
    commands = CGI_REQUEST_GetBodyValue(http_request, "commands");

    if (username == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "missingParam", "The parameter username is missing.");
    }

    if (password == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "missingParam", "The parameter password is missing.");
    }

    if (commands == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "missingParam", "The parameter commands is missing.");
    }

    if (FALSE == CGI_MODULE_ADDRUNCONFIG_Join(commands, cmd_str, sizeof(cmd_str) - 1, '\n'))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "invalidParam", "The parameter commands is invalid.");
    }

    username_value = json_string_value(username);
    password_value = json_string_value(password);

    // FIXME: shall check auth in cgi_auth
    if (FALSE == CGI_MODULE_ADDRUNCONFIG_CheckAuth(username_value, password_value, http_request))
    {
        return CGI_RESPONSE_ERROR(CGI_FORBIDDEN, "AuthFail", "Authentication failed.");
    }

    ret = CLI_PMGR_ADDRUNCONFIG_CreateEntry(cmd_str, &id);
    if (ret == CLI_OM_EXP_ADDRUNCONFIG_NO_MORE_EMPTY_ENTRY)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "noMoreEmptyTransactionEntry", "There is no more empty transaction entry on the device.");
    }
    else if (ret == CLI_OM_EXP_ADDRUNCONFIG_SUCCESS)
    {
        CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T entry;

        ret = CLI_PMGR_ADDRUNCONFIG_GetEntryById(id, &entry);
        if (ret == CLI_OM_EXP_ADDRUNCONFIG_SUCCESS)
        {
            json_t *result = (json_t *) CGI_RESPONSE_GetResult(http_response);

            ASSERT(result != NULL);
            if (result != NULL)
            {
                CGI_MODULE_ADDRUNCONFIG_SetResult(result, entry);
            }

            return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
        }
        else
        {
            return CGI_RESPONSE_ERROR(CGI_URI_NOT_FOUND, "notFound", "The transaction entry has already deleted by other request, so cannot represent the information.");
        }
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "An unexpected problem occurred, please try again. If the problem continues, contact us about your condition in details.");
    }
}

/**----------------------------------------------------------------------
 * This API is used to delete addrunconfig entry by id.
 *
 * @param id (required, number) transaction ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ADDRUNCONFIG_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t *entry_id;
    UI32_T entry_id_value;
    CLI_OM_EXP_ADDRUNCONFIG_RETURN_T ret;

    entry_id = CGI_REQUEST_GetQueryValue(http_request, "transactionId");

    if (entry_id == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "missingQueryParam", "The query parameter transactionId is missing.");
    }

    entry_id_value = json_integer_value(entry_id);

    ret = CLI_PMGR_ADDRUNCONFIG_DeleteEntryById(entry_id_value);
    if (ret == CLI_OM_EXP_ADDRUNCONFIG_NOT_FOUND)
    {
        return CGI_RESPONSE_ERROR(CGI_URI_NOT_FOUND, "notFound", "The required transaction entry does not exist.");
    }
    else if (ret == CLI_OM_EXP_ADDRUNCONFIG_NOT_ALLOW)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "notAllow", "The required transaction entry state is running, it is not allow to delete now.");
    }
    else if (ret == CLI_OM_EXP_ADDRUNCONFIG_SUCCESS)
    {
        return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "An unexpected problem occurred, please try again. If the problem continues, contact us about your condition in details.");
    }
}

/**----------------------------------------------------------------------
 * This API is used to get addrunconfig entry by id.
 *
 * @param id (required, number) entry ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ADDRUNCONFIG_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t *entry_id;
    UI32_T entry_id_value;
    CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T entry;
    CLI_OM_EXP_ADDRUNCONFIG_RETURN_T ret;

    entry_id = CGI_REQUEST_GetQueryValue(http_request, "transactionId");

    if (entry_id == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "missingQueryParam", "The query parameter transactionId is missing.");
    }

    entry_id_value = json_integer_value(entry_id);

    ret = CLI_PMGR_ADDRUNCONFIG_GetEntryById(entry_id_value, &entry);
    if (ret == CLI_OM_EXP_ADDRUNCONFIG_NOT_FOUND)
    {
        return CGI_RESPONSE_ERROR(CGI_URI_NOT_FOUND, "notFound", "The required transaction entry does not exist.");
    }
    else if (ret == CLI_OM_EXP_ADDRUNCONFIG_SUCCESS)
    {
        json_t *result = (json_t *) CGI_RESPONSE_GetResult(http_response);

        ASSERT(result != NULL);
        if (result != NULL)
        {
            CGI_MODULE_ADDRUNCONFIG_SetResult(result, entry);
        }

        return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "An unexpected problem occurred, please try again. If the problem continues, contact us about your condition in details.");
    }
}

void CGI_MODULE_ADDRUNCONFIG_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.update_handler = CGI_MODULE_ADDRUNCONFIG_Update;
    handlers.delete_handler = CGI_MODULE_ADDRUNCONFIG_Delete;
    handlers.read_handler   = CGI_MODULE_ADDRUNCONFIG_Read;

    CGI_MAIN_Register("/api/v1/running-config", &handlers, 0);
}

static void CGI_MODULE_ADDRUNCONFIG_Init()
{
    CGI_MODULE_ADDRUNCONFIG_RegisterHandlers();
}

static BOOL_T CGI_MODULE_ADDRUNCONFIG_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request)
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

static BOOL_T CGI_MODULE_ADDRUNCONFIG_Join(json_t *commands, char *buf, size_t buf_len, char c)
{
    size_t ary_size;
    UI32_T index;
    json_t *cmd;
    char   *cmd_str;
    UI32_T buf_pos = 0;
    size_t str_len;
    int buf_remaining;
    size_t copy_len;

    memset(buf, 0, buf_len);

    ary_size = json_array_size(commands);

    if (ary_size <= 0)
    {
        return TRUE;
    }

    json_array_foreach(commands, index, cmd)
    {
        buf_remaining = (buf_len-buf_pos);
        if (buf_remaining <= 0)
        {
            return FALSE;
        }

        if (json_typeof(cmd) != JSON_STRING)
        {
            return FALSE;
        }
        cmd_str = (char *)json_string_value(cmd);

        str_len = strlen(cmd_str);
        copy_len = (str_len < buf_remaining) ? str_len : buf_remaining;

        memcpy(buf+buf_pos, cmd_str, copy_len);
        buf_pos += copy_len;

        if (buf_pos < buf_len)
        {
            buf[buf_pos] = c;
            buf_pos += 1;
        }
    }

    if (buf[buf_pos - 1] != c)
    {
        return FALSE;
    }

    return TRUE;
}

static void CGI_MODULE_ADDRUNCONFIG_SetResult(json_t *result, CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T entry)
{
    char *status;

    if (entry.status == CLI_OM_EXP_ADDRUNCONFIG_ENTRY_PENDING)
    {
        status = (char *)"Pending";
    }
    else if (entry.status == CLI_OM_EXP_ADDRUNCONFIG_ENTRY_RUNNING)
    {
        status = (char *)"Running";
    }
    else if (entry.status == CLI_OM_EXP_ADDRUNCONFIG_ENTRY_DONE)
    {
        status = (char *)"Done";
    }
    else
    {
        status = (char *)"Invalid";
    }

    json_object_set_new(result, "transactionId", json_integer(entry.id));
    json_object_set_new(result, "status", json_string(status));
    json_object_set_new(result, "commands", json_string(entry.commands));
}
