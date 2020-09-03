#include "cgi_auth.h"
#include "mlag_pmgr.h"
#include "mlag_pom.h"
#include "mlag_type.h"
#include "swctrl_pom.h"
#include "sys_dflt.h"

//static BOOL_T CGI_MODULE_MLAG_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request);

/**----------------------------------------------------------------------
 * This API is used to read all MLAG domain and group.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_MLAG_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *domains_p;
    json_t  *groups_p;
    MLAG_TYPE_MlagEntry_T   mlag_entry;
    MLAG_TYPE_DomainEntry_T domain_entry;
    UI32_T                  status;

    if (MLAG_POM_GetGlobalStatus(&status) != MLAG_TYPE_RETURN_OK)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "mlag:getStatusError", "Failed to get MLAG status");
    }

    json_object_set_new(result_p, "status", json_boolean((status == MLAG_TYPE_GLOBAL_STATUS_ENABLED)));
    domains_p = json_array();
    groups_p = json_array();
    memset(&mlag_entry, 0, sizeof(MLAG_TYPE_MlagEntry_T));
    memset(&domain_entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));
    json_object_set_new(result_p, "domains", domains_p);
    json_object_set_new(result_p, "groups", groups_p);
    while (MLAG_POM_GetNextDomainEntry(&domain_entry) == MLAG_TYPE_RETURN_OK)
    {
        json_array_append_new(domains_p, json_string(domain_entry.domain_id));
    }
    while (MLAG_POM_GetNextMlagEntry(&mlag_entry) == MLAG_TYPE_RETURN_OK)
    {
        json_array_append_new(groups_p, json_integer(mlag_entry.mlag_id));
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to set MLAG status.
 *
 * @param status (required, boolean) MLAG status
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_MLAG_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *status_p;
    UI32_T  status_value;

    status_p = CGI_REQUEST_GetBodyValue(http_request, "status");
    if (NULL == status_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get status.");
    }

    if (TRUE != json_is_boolean(status_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
    }

    if (TRUE == json_boolean_value(status_p))
        status_value = MLAG_TYPE_GLOBAL_STATUS_ENABLED;
    else
        status_value = MLAG_TYPE_GLOBAL_STATUS_DISABLED;

    if (MLAG_PMGR_SetGlobalStatus(status_value) != XSTP_TYPE_RETURN_OK)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "mlag.setGlobalStatusError", "Failed to set MLAG status.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to create MLAG domain.
 *
 * @param name (required, string) MLAG name
 * @param peerLink (required, string) Interface ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_MLAG_Domain_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p;
    json_t  *peer_link_p;
    const char    *name_str_p;
    const char    *peer_link_str_p;
    UI32_T  unit = 0, port = 0, lport = 0;
    MLAG_TYPE_DomainEntry_T domain_entry;

    memset(&domain_entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));

    name_p = CGI_REQUEST_GetBodyValue(http_request, "name");
    if (name_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get domain name.");
    }
    name_str_p = json_string_value(name_p);

    peer_link_p = CGI_REQUEST_GetBodyValue(http_request, "peerLink");
    if (peer_link_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get domain peerLink.");
    }

    peer_link_str_p = json_string_value(peer_link_p);
    if (TRUE == CGI_UTIL_InterfaceIdToEth(peer_link_str_p, &unit, &port))
    {
        SWCTRL_POM_UserPortToIfindex(unit, port, &lport);

        if (MLAG_PMGR_SetDomain((char *)name_str_p, lport) != MLAG_TYPE_RETURN_OK)
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "mlag.setDomainError", "Failed to set create domain.");
        }
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The peerLink format is wrong.");
    }

    json_object_set_new(result_p, "id", name_p);
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete a MLAG domain.
 *
 * @param id (required, string) Unique MLAG domain ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_MLAG_Domain_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    const char    *id_str_p;

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get domian ID.");
    }
    id_str_p = json_string_value(id_p);

    if (MLAG_PMGR_RemoveDomain((char *)id_str_p) != MLAG_TYPE_RETURN_OK)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "mlag.removeDomainError", "Failed to set delete domain.");
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to read a MLAG domain ID.
 *
 * @param id (required, string) Unique MLAG domain ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_MLAG_Domain_ID_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    json_t  *groups_p;
    const char    *id_str_p;
    UI32_T  unit = 0, port = 0, trunk_id = 0;
    char    peer_link_ar[20] = {0};
    SWCTRL_Lport_Type_T     lport_type;
    MLAG_TYPE_MlagEntry_T   mlag_entry;
    MLAG_TYPE_DomainEntry_T domain_entry;

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get domian ID.");
    }

    memset(&mlag_entry, 0, sizeof(MLAG_TYPE_MlagEntry_T));
    memset(&domain_entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));

    id_str_p = json_string_value(id_p);
    strncpy(domain_entry.domain_id, id_str_p, MLAG_TYPE_MAX_DOMAIN_ID_LEN);
    if (MLAG_POM_GetDomainEntry(&domain_entry) != MLAG_TYPE_RETURN_OK)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "mlag.getDomainEntryError", "Failed to set get domain entry.");
    }

    lport_type = SWCTRL_POM_LogicalPortToUserPort(domain_entry.peer_link,
                        &unit, &port, &trunk_id);
    json_object_set_new(result_p, "id", id_p);
    json_object_set_new(result_p, "name", id_p);

    switch (lport_type)
    {
        case SWCTRL_LPORT_NORMAL_PORT:
            sprintf(peer_link_ar, "eth%lu/%lu",(unsigned long)unit, (unsigned long)port);
            json_object_set_new(result_p, "peerLink", json_string(peer_link_ar));
            break;

        case SWCTRL_LPORT_TRUNK_PORT:
            sprintf(peer_link_ar, "trunk%lu",(unsigned long)trunk_id);
            json_object_set_new(result_p, "peerLink", json_string(peer_link_ar));
            break;

        default:
            break;
    }

    groups_p = json_array();
    strncpy(mlag_entry.domain_id, domain_entry.domain_id,  MLAG_TYPE_MAX_DOMAIN_ID_LEN);
    json_object_set_new(result_p, "groups", groups_p);
    while (MLAG_POM_GetNextMlagEntryByDomain(&mlag_entry) == MLAG_TYPE_RETURN_OK)
    {
        json_array_append_new(groups_p, json_integer(mlag_entry.mlag_id));
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to create MLAG group ID.
 *
 * @param mlagId (required, string) MLAG ID
 * @param domainId (required, string) MLAG domain ID
 * @param member (required, string) Interface ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_MLAG_Group_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *mlag_id_p;
    json_t  *domain_id_p;
    json_t  *member_p;
    UI32_T  mlag_id = 0;
    const char    *domain_id_str_p;
    const char    *member_str_p;
    UI32_T  unit = 0, port = 0, lport = 0;
    MLAG_TYPE_DomainEntry_T domain_entry;

    memset(&domain_entry, 0, sizeof(MLAG_TYPE_DomainEntry_T));

    mlag_id_p = CGI_REQUEST_GetBodyValue(http_request, "mlagId");
    if (mlag_id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get mlagId.");
    }
    mlag_id = json_integer_value(mlag_id_p);

    domain_id_p = CGI_REQUEST_GetBodyValue(http_request, "domainId");
    if (domain_id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get domainId.");
    }

    domain_id_str_p = json_string_value(domain_id_p);

    member_p = CGI_REQUEST_GetBodyValue(http_request, "member");
    if (member_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get member.");
    }

    member_str_p = json_string_value(member_p);

    if (TRUE == CGI_UTIL_InterfaceIdToEth(member_str_p, &unit, &port))
    {
        SWCTRL_POM_UserPortToIfindex(unit, port, &lport);

        if (MLAG_PMGR_SetMlag(mlag_id, lport, (char *)domain_id_str_p) != MLAG_TYPE_RETURN_OK)
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "mlag.getDomainEntryError", "Failed to set create group.");
        }
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "peerLink Error.");
    }

    json_object_set_new(result_p, "id", mlag_id_p);
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}


/**----------------------------------------------------------------------
 * This API is used to delete a MLAG group ID.
 *
 * @param id (required, number) Unique MLAG group ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_MLAG_Group_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    UI32_T  id = 0;

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get domian ID.");
    }
    id = json_integer_value(id_p);

    if (MLAG_PMGR_RemoveMlag(id) != MLAG_TYPE_RETURN_OK)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "mlag.removeMlagError", "Failed to set delete group.");
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to get a group ID.
 *
 * @param id (required, number) Unique MLAG group ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_MLAG_Group_ID_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    json_t  *groups_p = json_array();
    UI32_T  id = 0;
    UI32_T  unit = 0, port = 0, trunk_id = 0;
    char    member_ar[20] = {0};
    char    local_state_ar[20] = {0};
    char    remote_state_ar[20] = {0};
    SWCTRL_Lport_Type_T     lport_type;
    MLAG_TYPE_MlagEntry_T   mlag_entry;

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get group ID.");
    }
    id = json_integer_value(id_p);

    memset(&mlag_entry, 0, sizeof(MLAG_TYPE_MlagEntry_T));
    mlag_entry.mlag_id = id;
    if (MLAG_POM_GetMlagEntry(&mlag_entry) != MLAG_TYPE_RETURN_OK)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "mlag.getMlagEntryError", "Failed to set get group entry.");
    }

    json_object_set_new(result_p, "id", id_p);
    json_object_set_new(result_p, "mlagId", id_p);
    json_object_set_new(result_p, "domainId", json_string(mlag_entry.domain_id));

    lport_type = SWCTRL_POM_LogicalPortToUserPort(mlag_entry.local_member,
                        &unit, &port, &trunk_id);
    switch (lport_type)
    {
        case SWCTRL_LPORT_NORMAL_PORT:
            sprintf(member_ar, "eth%lu/%lu", (unsigned long)unit, (unsigned long)port);
            json_object_set_new(result_p, "member", json_string(member_ar));
            break;

        case SWCTRL_LPORT_TRUNK_PORT:
            sprintf(member_ar, "trunk%lu", (unsigned long)trunk_id);
            json_object_set_new(result_p, "member", json_string(member_ar));
            break;

        default:
            break;
    }

    switch (mlag_entry.local_state)
    {
        case MLAG_TYPE_STATE_INEXISTENT:
            sprintf(local_state_ar, "Inactive");
            break;

        case MLAG_TYPE_STATE_DOWN:
            sprintf(local_state_ar, "Down");
            break;

        case MLAG_TYPE_STATE_DORMANT:
            sprintf(local_state_ar, "Dormant");
            break;

        case MLAG_TYPE_STATE_UP:
            sprintf(local_state_ar, "Up");
            break;
    }

    switch (mlag_entry.remote_state)
    {
        case MLAG_TYPE_STATE_INEXISTENT:
            sprintf(remote_state_ar, "Inactive");
            break;

        case MLAG_TYPE_STATE_DOWN:
            sprintf(remote_state_ar, "Down");
            break;

        case MLAG_TYPE_STATE_DORMANT:
            sprintf(remote_state_ar, "Dormant");
            break;

        case MLAG_TYPE_STATE_UP:
            sprintf(remote_state_ar, "Up");
            break;
    }
    json_object_set_new(result_p, "localState", json_string(local_state_ar));
    json_object_set_new(result_p, "remoteState", json_string(remote_state_ar));
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

void CGI_MODULE_MLAG_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler = CGI_MODULE_MLAG_Read;
    handlers.update_handler = CGI_MODULE_MLAG_Update;
    CGI_MAIN_Register("/api/v1/mlag", &handlers, 0);

    {   /* for "/api/v1/mlag/domains" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.create_handler = CGI_MODULE_MLAG_Domain_Create;
        CGI_MAIN_Register("/api/v1/mlag/domains", &handlers, 0);
    }

    {   /* for "/api/v1/mlag/domains/{id}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_MLAG_Domain_ID_Read;
        handlers.delete_handler = CGI_MODULE_MLAG_Domain_Delete;
        CGI_MAIN_Register("/api/v1/mlag/domains/{id}", &handlers, 0);
    }

    {   /* for "/api/v1/mlag/groups" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.create_handler = CGI_MODULE_MLAG_Group_Create;
        CGI_MAIN_Register("/api/v1/mlag/groups", &handlers, 0);
    }

    {   /* for "/api/v1/mlag/groups/{id}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_MLAG_Group_ID_Read;
        handlers.delete_handler = CGI_MODULE_MLAG_Group_Delete;
        CGI_MAIN_Register("/api/v1/mlag/groups/{id}", &handlers, 0);
    }
}

static void CGI_MODULE_MLAG_Init()
{
    CGI_MODULE_MLAG_RegisterHandlers();
}

/*
static BOOL_T CGI_MODULE_MLAG_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request)
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
