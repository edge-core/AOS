#include "cgi_request.h"
#include "cgi_auth.h"
#include "cgi_util.h"
#include "dhcp_pom.h"
#include "dhcp_pmgr.h"
#include "swctrl.h"

/**----------------------------------------------------------------------
 * This API is used to set DHCP relay servers to specified VLAN.
 *
 * @param vlanId (required, number) VLAN ID
 * @param servers (required, array) set vlan name
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_DHCPRELAY_VLAN_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

#if (SYS_CPNT_DHCP_RELAY == TRUE)
    json_t  *vid_p;
    json_t  *servers_p;
    json_t  *server_p;
    const char*   server_str_p;
    UI32_T  ip_addr_ar[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER] = {0};
    UI32_T  vid = 0, vid_ifindex = 0;
    UI32_T  idx = 0, server_num = 0, ret = 0;

    vid_p = CGI_REQUEST_GetBodyValue(http_request, "vlanId");

    if (vid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vid.");
    }
    vid = json_integer_value(vid_p);

    if (VLAN_OM_ConvertToIfindex(vid, &vid_ifindex) == FALSE)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                "vlan.convertToIfindexError", "Failed to get VLAN interface value.");
    }

    servers_p = CGI_REQUEST_GetBodyValue(http_request, "servers");

    if (servers_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Must specify servers.");
    }

    /* get port from members array
     */
    server_num = json_array_size(servers_p);

    if (0 == server_num)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "No server is specified.");
    }

    for (idx = 0; idx < server_num; idx++)
    {
        server_p = json_array_get(servers_p, idx);

        if (NULL == server_p)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get a server.");
        }

        server_str_p = json_string_value(server_p);

        if (TRUE != CGI_UTIL_IpStrToInt(server_str_p, &ip_addr_ar[idx]))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid server.");
        }
    } //for (idx=0; idx <server_num; idx++)

    //remove all before setting
    DHCP_PMGR_DeleteAllRelayServerAddress(vid_ifindex);
    ret = DHCP_PMGR_AddInterfaceRelayServerAddress(vid_ifindex, server_num, ip_addr_ar);

    switch(ret)
    {
        case DHCP_MGR_OK:
            break;
        case DHCP_MGR_NO_IP:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "dhcp.addInterfaceRelayServerAddressError", "The interface has not yet configured IP.");
        case DHCP_MGR_DYNAMIC_IP:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "dhcp.addInterfaceRelayServerAddressError", "The interface has not yet configured IP.");
        case DHCP_MGR_SERVER_ON:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "dhcp.addInterfaceRelayServerAddressError", "DHCP Server is running.");
        default:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "dhcp.addInterfaceRelayServerAddressError", "Failed to set up IP operation for relay server.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
#else
    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "dhcp.error", "Not support DHCP relay.");
#endif
}

/**----------------------------------------------------------------------
 * This API is used to read all DHCP relay servers.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_DHCPRELAY_VLAN_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vlans_p = json_array();
    UI32_T  ifindex = 0, index = 0, ip_address = 0, prev_ifindex = 0;

    json_object_set_new(result_p, "vlans", vlans_p);

#if (SYS_CPNT_DHCP_RELAY == TRUE)
    while (DHCP_MGR_OK == DHCP_PMGR_GetNextDhcpRelayServerAddrTable(&ifindex, &index, &ip_address))
    {
        json_t *vlan_obj_p = json_object();
        json_t *servers_p = json_array();
        UI8_T  addr_ar[SYS_ADPT_IPV4_ADDR_LEN] = {0};
        UI8_T  ip_str_ar[CGI_IPSTRLEN] = {0};
        UI32_T vid = 0;

        index = 0;
        prev_ifindex = ifindex;

        while (DHCP_MGR_OK == DHCP_PMGR_GetNextDhcpRelayServerAddrTable(&ifindex, &index, &ip_address))
        {
            if (ifindex != prev_ifindex)
            {
                break;
            }

            if (0 == ip_address)
            {
                continue;
            }

            IP_LIB_UI32toArray(ip_address, addr_ar);
            CGI_UTIL_IntToIpStr(addr_ar, ip_str_ar);
            json_array_append_new(servers_p, json_string(ip_str_ar));
        }

        if (0 != json_array_size(servers_p))
        {
            VLAN_OM_ConvertFromIfindex(prev_ifindex, &vid);

            json_object_set_new(vlan_obj_p, "vlanId", json_integer(vid));
            json_object_set_new(vlan_obj_p, "servers", servers_p);
            json_array_append_new(vlans_p, vlan_obj_p);
        }

        index = 0;
        ifindex = (prev_ifindex + 1);
    } //DHCP_PMGR_GetNextDhcpRelayServerAddrTable
#endif  /* #if (SYS_CPNT_DHCP_RELAY == TRUE) */

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_DHCPRELAY_VLAN_Read

/**----------------------------------------------------------------------
 * This API is used to add a DHCP relay server.
 *
 * @param vlanId (required, number) VLAN ID
 * @param server (required, number) server IP
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_DHCPRELAY_VLAN_SERVER_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

#if (SYS_CPNT_DHCP_RELAY == TRUE)
    json_t  *vid_p;
    json_t  *server_p;
    const char*  server_str_p;
    UI32_T  vid = 0, vid_ifindex = 0, ret = 0;
    UI32_T  ip_addr_ar[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER] = {0};

    vid_p = CGI_REQUEST_GetParamsValue(http_request, "vlanId");

    if (vid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get VLAN ID.");
    }
    vid = json_integer_value(vid_p);

    if (VLAN_OM_ConvertToIfindex(vid, &vid_ifindex) == FALSE)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                "vlan.convertToIfindexError", "Failed to get VLAN interface value.");
    }

    server_p = CGI_REQUEST_GetParamsValue(http_request, "server");

    if (server_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get server IP.");
    }

    server_str_p = json_string_value(server_p);

    if (TRUE != CGI_UTIL_IpStrToInt(server_str_p, &ip_addr_ar[0]))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid server IP.");
    }

    ret = DHCP_PMGR_AddInterfaceRelayServerAddress(vid_ifindex, 1, ip_addr_ar);

    switch(ret)
    {
        case DHCP_MGR_OK:
            break;
        case DHCP_MGR_NO_IP:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "dhcp.addInterfaceRelayServerAddressError", "The interface has not yet configured IP.");
        case DHCP_MGR_DYNAMIC_IP:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "dhcp.addInterfaceRelayServerAddressError", "The interface has not yet configured IP.");
        case DHCP_MGR_SERVER_ON:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "dhcp.addInterfaceRelayServerAddressError", "DHCP Server is running.");
        default:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "dhcp.addInterfaceRelayServerAddressError", "Failed to set up IP operation for relay server.");
    }
#endif

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete a DHCP relay server.
 *
 * @param vlanId (required, number) VLAN ID
 * @param server (required, number) server IP
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_DHCPRELAY_VLAN_SERVER_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

#if (SYS_CPNT_DHCP_RELAY == TRUE)
    json_t  *vid_p;
    json_t  *server_p;
    const char*  server_str_p;
    UI32_T  vid = 0, vid_ifindex = 0;
    UI32_T  ip_addr_ar[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER] = {0};

    vid_p = CGI_REQUEST_GetParamsValue(http_request, "vlanId");

    if (vid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get VLAN ID.");
    }
    vid = json_integer_value(vid_p);

    if (VLAN_OM_ConvertToIfindex(vid, &vid_ifindex) == FALSE)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                "vlan.convertToIfindexError", "Failed to get VLAN interface value.");
    }

    server_p = CGI_REQUEST_GetParamsValue(http_request, "server");

    if (server_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get server IP.");
    }

    server_str_p = json_string_value(server_p);

    if (TRUE != CGI_UTIL_IpStrToInt(server_str_p, &ip_addr_ar[0]))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid server IP.");
    }

    if (DHCP_MGR_OK != DHCP_PMGR_DeleteInterfaceRelayServerAddress(vid_ifindex, 1, ip_addr_ar))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                "dhcp.deleteInterfaceRelayServerAddressError", "Failed to delete configured IP.");
    }
#endif

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to restart DHCP relay.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_DHCPRELAY_RESTART_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

#if (SYS_CPNT_DHCP_RELAY == TRUE)
    DHCP_PMGR_Restart3(DHCP_MGR_RESTART_RELAY);
#endif

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}


void CGI_MODULE_DHCPRELAY_RegisterHandlers()
{
    {
        CGI_API_HANDLER_SET_T handlers = {0};

        handlers.read_handler = CGI_MODULE_DHCPRELAY_VLAN_Read;
        handlers.create_handler = CGI_MODULE_DHCPRELAY_VLAN_Create;
        CGI_MAIN_Register("/api/v1/dhcp-relay/vlans", &handlers, 0);
    }

    {   /* for "/api/v1/dhcp-relay/vlans/{vlanId}/servers/{server}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.update_handler = CGI_MODULE_DHCPRELAY_VLAN_SERVER_Update;
        handlers.delete_handler = CGI_MODULE_DHCPRELAY_VLAN_SERVER_Delete;
        CGI_MAIN_Register("/api/v1/dhcp-relay/vlans/{vlanId}/servers/{server}", &handlers, 0);
    }

    {   /* for "/api/v1/dhcp-relay/restart" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.update_handler = CGI_MODULE_DHCPRELAY_RESTART_Update;
        CGI_MAIN_Register("/api/v1/dhcp-relay/restart", &handlers, 0);
    }
}

static void CGI_MODULE_DHCPRELAY_Init()
{
    CGI_MODULE_DHCPRELAY_RegisterHandlers();
}

