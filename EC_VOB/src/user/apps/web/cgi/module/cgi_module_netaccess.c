#include "cgi_auth.h"

#if(SYS_CPNT_NETACCESS==TRUE)
#include "netaccess_pmgr.h"
#endif

#if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE)
static CGI_STATUS_CODE_T CGI_MODULE_NETACCESS_UpdatePortDynamicVlan(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *status_p);
#endif  /* #if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE) */

#if (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
static CGI_STATUS_CODE_T CGI_MODULE_NETACCESS_UpdatePortGuestVlan(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *vid_p);
#endif  /* #if (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE) */

/**----------------------------------------------------------------------
 * This API is used to update attribute of specified ports.
 *
 * @param ports       (required, array) Port array
 * @param dynamicVlan (optional, boolean) Dynamic VLAN status
 * @param guestVlan   (optional, number) Guest VLAN value. 0 for disable
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_NETACCESS_INTF_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

#if (SYS_CPNT_NETACCESS == TRUE)
    json_t  *ports_p;
    json_t  *port_p;
    const char *port_str_p;
    int port_num = 0, idx = 0;
    UI32_T lport = 0;

    ports_p = CGI_REQUEST_GetBodyValue(http_request, "ports");

    if (NULL == ports_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "ports is NULL.");
    }
    port_num = json_array_size(ports_p);

    if (0 >= port_num)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "ports number is 0.");
    }

    for (idx = 0; idx < port_num; idx ++)
    {
        port_p = json_array_get(ports_p, idx);

        if (NULL != port_p)
        {
            port_str_p = json_string_value(port_p);

            if (TRUE != CGI_UTIL_InterfaceIdToLport((char *)port_str_p, &lport))
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid ifId.");
            }
        }
    }

#if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE)
    {
        json_t  *dynamic_status_p = CGI_REQUEST_GetBodyValue(http_request, "dynamicVlan");

        if (NULL != dynamic_status_p)
        {
            return CGI_MODULE_NETACCESS_UpdatePortDynamicVlan(http_response, ports_p, port_num, dynamic_status_p);
        }
    }
#endif  /* #if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE) */

#if (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
    {
        json_t  *guest_vid_p = CGI_REQUEST_GetBodyValue(http_request, "guestVlan");

        if (NULL != guest_vid_p)
        {
            return CGI_MODULE_NETACCESS_UpdatePortGuestVlan(http_response, ports_p, port_num, guest_vid_p);
        }
    }
#endif  /* #if (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE) */

#endif  /* #if (SYS_CPNT_NETACCESS == TRUE) */
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_NETACCESS_INTF_Update

#if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE)
static CGI_STATUS_CODE_T CGI_MODULE_NETACCESS_UpdatePortDynamicVlan(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *status_p)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *port_p;
    const char *port_str_p;
    UI32_T status = VAL_networkAccessPortDynamicVlan_disabled;
    UI32_T lport = 0;
    int idx = 0;

    if (TRUE != json_is_boolean(status_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
    }

    if (TRUE == json_boolean_value(status_p))
    {
        status = VAL_networkAccessPortDynamicVlan_enabled;
    }

    for (idx = 0; idx < port_num; idx ++)
    {
        port_p = json_array_get(ports_p, idx);
        port_str_p = json_string_value(port_p);
        CGI_UTIL_InterfaceIdToLport((char *)port_str_p, &lport);

        if (TRUE != NETACCESS_PMGR_SetSecureDynamicVlanStatus(lport, status))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "netaccess.setSecureDynamicVlanStatusError", "Failed to set port dynamic VLAN status.");
        }
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_PPPOE_IA_UpdatePortStatus
#endif  /* #if (CGI_MODULE_NETACCESS_UpdatePortDynamicVlan == TRUE) */

#if (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
static CGI_STATUS_CODE_T CGI_MODULE_NETACCESS_UpdatePortGuestVlan(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *vid_p)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *port_p;
    const char *port_str_p;
    UI32_T guest_vlan_id = NETACCESS_TYPE_DFLT_GUEST_VLAN_ID;
    UI32_T lport = 0;
    int idx = 0;

    guest_vlan_id = json_integer_value(vid_p);

    if (0 == guest_vlan_id)
    {
        guest_vlan_id = NETACCESS_TYPE_DFLT_GUEST_VLAN_ID;
    }

    for (idx = 0; idx < port_num; idx ++)
    {
        port_p = json_array_get(ports_p, idx);
        port_str_p = json_string_value(port_p);
        CGI_UTIL_InterfaceIdToLport((char *)port_str_p, &lport);

        if (TRUE != NETACCESS_PMGR_SetSecureGuestVlanId(lport, guest_vlan_id))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "netaccess.setSecureGuestVlanIdError", "Failed to set port guest VLAN.");
        }
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_PPPOE_IA_UpdatePortTrustStatus
#endif  /* #if (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE) */

void CGI_MODULE_NETACCESS_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.update_handler = CGI_MODULE_NETACCESS_INTF_Update;
    CGI_MAIN_Register("/api/v1/network-access/interfaces", &handlers, 0);
}


static void CGI_MODULE_NETACCESS_Init()
{
    CGI_MODULE_NETACCESS_RegisterHandlers();
}

