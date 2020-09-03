#include "cgi_auth.h"
#include "netcfg_pmgr_nd.h"
#include "netcfg_pom_nd.h"
#include "sys_adpt.h"

#define CGI_MODULE_ARP_DFLT_COUNT 50

//static BOOL_T CGI_MODULE_ARP_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request);
static void CGI_MODULE_ARP_GetOne(json_t *arps_p, NETCFG_TYPE_IpNetToMediaEntry_T  *entry);
static BOOL_T CGI_MODULE_OSPF_ParseId(const char *str, UI32_T *vid, UI32_T *ip);

/**----------------------------------------------------------------------
 * This API is used to get ARP info.
 *
 * @param id    (optional, string) Unique ARP ID (vid_ip)
 * @param count (optional, number) Total number of result to be returned.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ARP_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *arps_p;
    json_t  *id_p, *count_p;
    UI32_T  time_out_value = 0;
    UI32_T  id_vid = 0, id_ip = 0, count = 0;
    const char    *id_str_p;
    NETCFG_TYPE_IpNetToMediaEntry_T  entry;

    id_p = CGI_REQUEST_GetQueryValue(http_request, "id");
    count_p = CGI_REQUEST_GetQueryValue(http_request, "count");

    memset(&entry, 0, sizeof(NETCFG_TYPE_IpNetToMediaEntry_T));
    if(NETCFG_POM_ND_GetIpNetToMediaTimeout(&time_out_value) == FALSE )
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netconf.nd.getIpNetToMediaTimeoutFail", "Failed to get timeout value.");
    }

    arps_p = json_array();
    json_object_set_new(result_p, "timeout", json_integer(time_out_value));
    json_object_set_new(result_p, "arps", arps_p);

    if (NULL != id_p)
    {
        id_str_p = json_string_value(id_p);
        if (TRUE == CGI_MODULE_OSPF_ParseId(id_str_p, &id_vid, &id_ip))
        {
            if (NULL != count_p)
            {
                count = json_integer_value(count_p);
                if (count == 0)
                {
                    return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The count need more than 0.");
                }
            }
            else
            {
                count = CGI_MODULE_ARP_DFLT_COUNT;
            }
        }
        else
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get id because format is wrong.");
        }

        if (0 != count)
        {
            VLAN_OM_ConvertToIfindex(id_vid, &entry.ip_net_to_media_if_index);
            entry.ip_net_to_media_net_address = id_ip;
            while (NETCFG_PMGR_ND_GetNextIpNetToMediaEntry(&entry) == NETCFG_TYPE_OK)
            {
                CGI_MODULE_ARP_GetOne(arps_p, &entry);
                count--;
                if (count == 0)
                {
                    break;
                }
            }
        }
    }
    else
    {
        while (NETCFG_PMGR_ND_GetNextIpNetToMediaEntry(&entry) == NETCFG_TYPE_OK)
        {
            CGI_MODULE_ARP_GetOne(arps_p, &entry);
        }
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to add ARP.
 *
 * @param destIp  (required, string) Destination IP address
 * @param destMac (required, string) Destination MAC address
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ARP_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *mac_p;
    json_t  *ip_p;
    const char*   mac_str_p;
    const char*   ip_str_p;
    L_INET_AddrIp_T  inet_address;
    NETCFG_TYPE_PhysAddress_T   phys_address;
    UI32_T  return_type = 0;

    memset(&inet_address, 0, sizeof(inet_address));
    memset(&phys_address, 0, sizeof(phys_address));
    mac_p = CGI_REQUEST_GetBodyValue(http_request, "destMac");

    if (mac_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get destMac.");
    }
    mac_str_p = json_string_value(mac_p);

    if (CGI_UTIL_MacStrToHex(mac_str_p, phys_address.phy_address_cctet_string) == FALSE)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to parse MAC.");
    }

    ip_p = CGI_REQUEST_GetBodyValue(http_request, "destIp");

    if (ip_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get destIp.");
    }
    ip_str_p = json_string_value(ip_p);

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(
            L_INET_FORMAT_IP_UNSPEC, ip_str_p, (L_INET_Addr_T *)&inet_address, sizeof(inet_address)))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid IP.");
    }

    phys_address.phy_address_type = 1;
    phys_address.phy_address_len =  SYS_ADPT_MAC_ADDR_LEN;
    return_type = NETCFG_PMGR_ND_AddStaticIpNetToPhysicalEntry(0, &inet_address, &phys_address);

    if (NETCFG_TYPE_OK != return_type) {
        switch(return_type) {
            case NETCFG_TYPE_CAN_NOT_ADD_LOCAL_IP:
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netcfg.AddStaticIpNetToPhysicalEntry",
                        "The added IP address is the same as local IP address; the add action is aborted.");

            case NETCFG_TYPE_TABLE_FULL:
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netcfg.AddStaticIpNetToPhysicalEntry",
                        "The static ARP Cache is full; the add action is aborted.");

            case NETCFG_TYPE_ENTRY_EXIST:
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netcfg.AddStaticIpNetToPhysicalEntry",
                        "The added IP address exists; the add action is aborted.");

             case NETCFG_TYPE_INVALID_ARG:
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netcfg.AddStaticIpNetToPhysicalEntry",
                        "The added IP address or physical address is invalid; the add action is aborted.");

            default:
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netcfg.AddStaticIpNetToPhysicalEntry",
                        "Failed to set the static ARP cache entry.");
        }
    }

    json_object_set_new(result_p, "id", json_string(ip_str_p));
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete ARP.
 *
 * @param id            (required, string) Unique ARP ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ARP_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    const char *id_str_p;
    L_INET_AddrIp_T  inet_address;
    UI32_T  return_type = 0;

    memset(&inet_address, 0, sizeof(inet_address));
    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ID.");
    }
    id_str_p = json_string_value(id_p);

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(
            L_INET_FORMAT_IP_UNSPEC, id_str_p, (L_INET_Addr_T *)&inet_address, sizeof(inet_address)))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid IP.");
    }

    return_type = NETCFG_PMGR_ND_DeleteStaticIpNetToPhysicalEntry(0, &inet_address);

    if (NETCFG_TYPE_OK != return_type) {
        switch(return_type) {
            case NETCFG_TYPE_ENTRY_NOT_EXIST:
                return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);

             case NETCFG_TYPE_INVALID_ARG:
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netcfg.DeleteStaticIpNetToPhysicalEntry",
                        "The deleted static IP address is invalid; the delete action is aborted.");

            default:
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netcfg.DeleteStaticIpNetToPhysicalEntry",
                        "Failed to delete this entry.");
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

void CGI_MODULE_ARP_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler   = CGI_MODULE_ARP_Read;
    handlers.create_handler = CGI_MODULE_ARP_Create;

    CGI_MAIN_Register("/api/v1/arps", &handlers, 0);

    {   /* for "/api/v1/arps/{id}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.delete_handler = CGI_MODULE_ARP_ID_Delete;
        CGI_MAIN_Register("/api/v1/arps/{id}", &handlers, 0);
    }

}

static void CGI_MODULE_ARP_Init()
{
    CGI_MODULE_ARP_RegisterHandlers();
}

/*
static BOOL_T CGI_MODULE_ARP_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request)
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

static void CGI_MODULE_ARP_GetOne(json_t *arps_p, NETCFG_TYPE_IpNetToMediaEntry_T  *entry)
{
    json_t *arp_obj_p = json_object();
    UI32_T vid = 0;
    char  id_ar[128] = {0};
    char  mac_str_ar[18] = {0};
    char  ip_str_ar[18] = {0};

    if ((VAL_ipNetToMediaType_dynamic != entry->ip_net_to_media_type) &&
            (VAL_ipNetToMediaType_static != entry->ip_net_to_media_type))
    {
        return; //ignore switch IP self
    }

    L_INET_Ntoa(entry->ip_net_to_media_net_address,ip_str_ar);
    VLAN_OM_ConvertFromIfindex(entry->ip_net_to_media_if_index, &vid);

    sprintf(mac_str_ar, "%02X-%02X-%02X-%02X-%02X-%02X",
        entry->ip_net_to_media_phys_address.phy_address_cctet_string[0],
        entry->ip_net_to_media_phys_address.phy_address_cctet_string[1],
        entry->ip_net_to_media_phys_address.phy_address_cctet_string[2],
        entry->ip_net_to_media_phys_address.phy_address_cctet_string[3],
        entry->ip_net_to_media_phys_address.phy_address_cctet_string[4],
        entry->ip_net_to_media_phys_address.phy_address_cctet_string[5]);

    sprintf(id_ar, "%lu_%s",(unsigned long)vid, ip_str_ar);

    json_object_set_new(arp_obj_p, "id", json_string(id_ar));
    json_object_set_new(arp_obj_p, "ip", json_string(ip_str_ar));
    json_object_set_new(arp_obj_p, "mac", json_string(mac_str_ar));
    json_object_set_new(arp_obj_p, "vid", json_integer(vid));
    json_array_append_new(arps_p, arp_obj_p);
    return;
}

static BOOL_T CGI_MODULE_OSPF_ParseId(const char *str, UI32_T *vid, UI32_T *ip)
{
    char buf_ar[128] = {0};
    char *strtok_state_p = NULL;
    char *vid_str_p, *ip_str_p;
    char *end_p;

    strncpy(buf_ar, str, strlen(str));
    strtok_state_p = buf_ar;

    vid_str_p = strtok_r(NULL, "_", &strtok_state_p);
    if (vid_str_p == NULL)
    {
        return FALSE;
    }
    else
    {
        *vid  = strtol(vid_str_p, &end_p, 0);
    }

    ip_str_p = strtok_r(NULL, "_", &strtok_state_p);
    if (ip_str_p == NULL)
    {
        return FALSE;
    }
    if (L_INET_Aton(ip_str_p, ip) == FALSE)
    {
        return FALSE;
    }
    return TRUE;
}