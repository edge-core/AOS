#include "cgi_util.h"
#include "amtr_pmgr.h"
#include "swctrl_pom.h"
#if(SYS_CPNT_VXLAN == TRUE)
#include "vxlan_type.h"
#include "vxlan_pom.h"
#include "amtrl3_pom.h"
#endif

#define CGI_MODULE_MAC_ADDRESS_DFLT_COUNT 50

static void CGI_MODULE_MAC_ADDRESS_GetOne(json_t *macs_p, AMTR_TYPE_AddrEntry_T *addr_entry_p);
static BOOL_T CGI_MODULE_MAC_ADDRESS_ParseId(const char *str, UI16_T *vid, UI8_T *mac);

/**----------------------------------------------------------------------
 * This API is used to get MAC info.
 *
 * @param id       (optional, string) Unique MAC ID (vid_mac)
 * @param type     (optional, string) MAC type
 * @param lifeTime (optional, string) MAC life-time
 * @param ifId     (optional, string) Interface ID
 * @param count    (optional, number) Total number of result to be returned.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_MACs_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *macs_p = json_array();
    json_t  *id_p, *count_p, *life_time_p, *type_p, *if_id_p;
    const char    *id_str_p;
    const char    *life_time_str_p;
    const char    *type_str_p;
    const char    *if_id_str_p;
    UI32_T  count = 0;
    UI32_T  get_mode = AMTR_MGR_GET_ALL_ADDRESS;
    UI32_T  unit = 0, port = 0, trunk_id = 0, lport = 0;
    BOOL_T  query = FALSE, filer_if_id = FALSE;
    BOOL_T  filer_life_time = FALSE, filer_type = FALSE;
    UI8_T   life_time = 0, src_type = 0;
    AMTR_TYPE_AddrEntry_T addr_entry;

    id_p = CGI_REQUEST_GetQueryValue(http_request, "id");
    count_p = CGI_REQUEST_GetQueryValue(http_request, "count");
    life_time_p = CGI_REQUEST_GetQueryValue(http_request, "lifeTime");
    type_p = CGI_REQUEST_GetQueryValue(http_request, "type");
    if_id_p = CGI_REQUEST_GetQueryValue(http_request, "ifId");

    memset(&addr_entry, 0, sizeof(AMTR_TYPE_AddrEntry_T));
    json_object_set_new(result_p, "macs", macs_p);

    if (    (NULL != id_p)
        ||  (NULL != life_time_p)
        ||  (NULL != type_p)
        ||  (NULL != if_id_p)
       )
    {
        query = TRUE;
    }
    if (NULL != id_p)
    {
        id_str_p = json_string_value(id_p);
        if (FALSE == CGI_MODULE_MAC_ADDRESS_ParseId(id_str_p, &(addr_entry.vid), addr_entry.mac))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get id because format is wrong.");
        }
    }

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
        count = CGI_MODULE_MAC_ADDRESS_DFLT_COUNT;
    }

    if (NULL != life_time_p)
    {
        life_time_str_p = json_string_value(life_time_p);
        filer_life_time = TRUE;
        if (0 == strncmp(life_time_str_p, "Permanent", sizeof("Permanent")))
        {
            life_time = AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT;
            get_mode = AMTR_MGR_GET_STATIC_ADDRESS;
        }
        else if (0 == strncmp(life_time_str_p, "Delete on Reset", sizeof("Delete on Reset")))
        {
            life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET;
            get_mode = AMTR_MGR_GET_ALL_ADDRESS;
        }
        else if (0 == strncmp(life_time_str_p, "Delete on Timeout", sizeof("Delete on Timeout")))
        {
            life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;
            get_mode = AMTR_MGR_GET_ALL_ADDRESS;
        }
        else if (0 == strncmp(life_time_str_p, "Other", sizeof("Other")))
        {
            life_time = AMTR_TYPE_ADDRESS_LIFETIME_OTHER;
            get_mode = AMTR_MGR_GET_ALL_ADDRESS;
        }
        else
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get lifeTime because format is wrong.");
        }

    }

    if (NULL != type_p)
    {
        type_str_p = json_string_value(type_p);
        filer_type = TRUE;
        if (0 == strncmp(type_str_p, "Config", sizeof("Config")))
        {
            src_type = AMTR_TYPE_ADDRESS_SOURCE_CONFIG;
        }
        else if (0 == strncmp(type_str_p, "Internal", sizeof("Internal")))
        {
            src_type = AMTR_TYPE_ADDRESS_SOURCE_INTERNAL;
        }
        else if (0 == strncmp(type_str_p, "CPU", sizeof("CPU")))
        {
            src_type = AMTR_TYPE_ADDRESS_SOURCE_SELF;
        }
        else if (0 == strncmp(type_str_p, "Security", sizeof("Security")))
        {
            src_type = AMTR_TYPE_ADDRESS_SOURCE_SECURITY;
        }
        else if (0 == strncmp(type_str_p, "Learn", sizeof("Learn")))
        {
            src_type = AMTR_TYPE_ADDRESS_SOURCE_LEARN;
        }
        else
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get type because format is wrong.");
        }
    }

    if (NULL != if_id_p)
    {
        if_id_str_p = json_string_value(if_id_p);
        filer_if_id = TRUE;
        if (TRUE == CGI_UTIL_InterfaceIdToEth(if_id_str_p, &unit, &port))
        {
            SWCTRL_POM_UserPortToIfindex(unit, port, &lport);
        }
        else if (strncmp(if_id_str_p, "trunk", 5) == 0)
        {
            trunk_id = atoi(if_id_str_p + 5);
            SWCTRL_POM_TrunkIDToLogicalPort(trunk_id, &lport);
        }
        else
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Error ifId.");
        }
    }

    /* To get next address table entry (mac+vid).
     */
    memset(&addr_entry, 0, sizeof(AMTR_TYPE_AddrEntry_T));
    while(AMTR_PMGR_GetNextMVAddrEntry(&addr_entry, get_mode))
    {
        /* filer lifeTime.
         */
        if (    (TRUE == filer_life_time)
            &&  (life_time != addr_entry.life_time)
           )
        {
            continue;
        }

        /* filer type.
         */
        if (    (TRUE == filer_type)
            &&  (src_type != addr_entry.source)
           )
        {
            continue;
        }

        /* filer ifId.
         */
        if (    (TRUE == filer_if_id)
            &&  (lport != addr_entry.ifindex)
           )
        {
            continue;
        }

        CGI_MODULE_MAC_ADDRESS_GetOne(macs_p, &addr_entry);
        if (query == TRUE)
        {
            count--;
            if (count == 0)
            {
                break;
            }
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to add MAC.
 *
 * @param mac         (required, string) MAC address
 * @param vid         (required, number) VLAN ID
 * @param ifId        (required, string) interface ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_MACs_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *mac_p;
    json_t  *vid_p;
    json_t  *if_id_p;
    const char*   mac_str_p;
    const char*   if_id_str_p;
    AMTR_TYPE_AddrEntry_T addr_entry;
    char    mac_id_ar[24] = {0}; //xxxx_xx-xx-xx-xx-xx-xx
    UI32_T  ifindex = 0, vid = 0;

    memset(&addr_entry, 0, sizeof(addr_entry));
    mac_p = CGI_REQUEST_GetBodyValue(http_request, "mac");

    if (mac_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get mac.");
    }
    mac_str_p = json_string_value(mac_p);

    if (CGI_UTIL_MacStrToHex(mac_str_p, addr_entry.mac) == FALSE)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to parse MAC.");
    }

    vid_p = CGI_REQUEST_GetBodyValue(http_request, "vid");

    if (vid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vid.");
    }
    vid = json_integer_value(vid_p);
    if_id_p = CGI_REQUEST_GetBodyValue(http_request, "ifId");

    if (if_id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ifId.");
    }
    if_id_str_p = json_string_value(if_id_p);

    if (TRUE != CGI_UTIL_InterfaceIdToLport(if_id_str_p, &ifindex))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid ifId.");
    }

    addr_entry.vid = vid;
    addr_entry.ifindex = ifindex;
    addr_entry.life_time= AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT;
    addr_entry.source= AMTR_TYPE_ADDRESS_SOURCE_CONFIG;
    addr_entry.action= AMTR_TYPE_ADDRESS_ACTION_FORWARDING;

    if (!AMTR_PMGR_SetAddrEntry(&addr_entry))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "amtr.SetAddrEntry", "Failed to set MAC.");
    }

    sprintf(mac_id_ar, "%u_%02X-%02X-%02X-%02X-%02X-%02X", addr_entry.vid,
            addr_entry.mac[0], addr_entry.mac[1], addr_entry.mac[2],
            addr_entry.mac[3], addr_entry.mac[4], addr_entry.mac[5]);
    json_object_set_new(result_p, "id", json_string(mac_id_ar));
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete MAC.
 *
 * @param id            (required, string) Unique MAC ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_MACs_MAC_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    char    *id_str_p;
    char    *save_str_addr = NULL;
    char    *vid_str_p;
    UI8_T   mac_ar[SYS_ADPT_MAC_ADDR_LEN] = {0};
    UI32_T  vid = 0;

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ID.");
    }
    id_str_p = (char *)json_string_value(id_p);
    vid_str_p = strtok_r(id_str_p, "_", &save_str_addr);

    if (sscanf (vid_str_p, "%u", &vid) != 1)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to parse VID.");
    }

    if (CGI_UTIL_MacStrToHex(save_str_addr, mac_ar) == FALSE)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to parse MAC.");
    }

    if (!AMTR_PMGR_DeleteAddr(vid, mac_ar))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "amtr.DeleteAddr", "Failed to remove MAC.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete MAC by specified ifindex.
 *
 * @param id            (required, string) Unique MAC ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_MACs_INTF_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    UI32_T  ifindex = 0;

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ID.");
    }
    ifindex = json_integer_value(id_p);

    AMTR_PMGR_DeleteAddrByLifeTimeAndLPort(ifindex, AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT);
    AMTR_PMGR_DeleteAddrByLifeTimeAndLPort(ifindex, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET);
    AMTR_PMGR_DeleteAddrByLifeTimeAndLPort(ifindex, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to create a mac entry for network-ports.
 *
 * @param vni           (required, number) VXLAN ID.
 *        mac           (required, string) mac address of specify route.
 *        rVtep	        (required, string) ip address of remote vtep.
 *        permanent	    (required, boolean) the specify route is permanent or not.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_MACs_Vxlan_VNI_NetworkPort_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t *vni_p = NULL;
    json_t *mac_p = NULL;
    json_t *remote_vtep_p = NULL;
    json_t *permanent_p = NULL;
    UI32_T  vni = 0;
    char *mac_str_p = NULL;
    char *remote_vtep_str_p = NULL;
    UI32_T permanent_status = 0;

    /* get parameter */
    vni_p = CGI_REQUEST_GetParamsValue(http_request, "vni");
    if (NULL == vni_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vni.");
    }
    vni = json_integer_value(vni_p);

    mac_p = CGI_REQUEST_GetBodyValue(http_request, "mac");
    if (NULL == mac_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get mac.");
    }
    mac_str_p = (char *)json_string_value(mac_p);

    remote_vtep_p = CGI_REQUEST_GetBodyValue(http_request, "rVtep");
    if (NULL == remote_vtep_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get rVtep.");
    }
    remote_vtep_str_p = (char *)json_string_value(remote_vtep_p);

    permanent_p = CGI_REQUEST_GetBodyValue(http_request, "permanent");
    if (NULL == permanent_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get permanent.");
    }

    if (TRUE != json_is_boolean(permanent_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The permanent is not boolean type.");
    }

    if (TRUE == json_boolean_value(permanent_p))
    {
        permanent_status = AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT;
    }
    else
    {
        permanent_status = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET;
    }

    /* check parameter */
    AMTR_TYPE_AddrEntry_T addr_entry = {0};
    VXLAN_OM_VNI_T vni_entry = {0};
    vni_entry.vni = vni;

    if (VXLAN_TYPE_RETURN_OK != VXLAN_POM_GetVniEntry(&vni_entry))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "vni not exist.");
    }
    addr_entry.vid = vni_entry.vfi;

    if (!CGI_UTIL_MacStrToHex(mac_str_p, addr_entry.mac))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "mac format is invalid.");
    }

    AMTRL3_OM_VxlanTunnelEntry_T vxlan_tunnel = {0};
    VXLAN_OM_RVtep_T rVtep_entry = {0};
    L_INET_AddrIp_T ip_p;
    UI16_T l_vxlan_port;

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                       remote_vtep_str_p,
                                                       (L_INET_Addr_T *) &ip_p,
                                                       sizeof(ip_p)))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "rVtep ip format is invalid.");
    }

    memcpy(addr_entry.r_vtep_ip, ip_p.addr, SYS_ADPT_IPV4_ADDR_LEN);
    if (VXLAN_POM_GetNextFloodRVtep(&rVtep_entry) == VXLAN_TYPE_RETURN_OK)
    {
        /* just need get "s_ip" information, even use VXLAN_POM_GetNextFloodRVtep(),
         * but only need get one entry, the "s_ip" of each entrys are the same.
         */
        vxlan_tunnel.local_vtep = rVtep_entry.s_ip;
        vxlan_tunnel.vfi_id = vni_entry.vfi;
        vxlan_tunnel.remote_vtep = ip_p;

        if (FALSE == AMTRL3_POM_GetVxlanTunnelEntry(SYS_ADPT_DEFAULT_FIB, &vxlan_tunnel))
        {
            /* If vxlan tunnel entry is not established, network vxlan port cannot be obtained,
             * so just fill the field with any valid value. Until host route with the r-vtep is ready,
             * the network vxlan port will be fixed with correct value.
             */
            addr_entry.ifindex = SYS_ADPT_VXLAN_MAX_LOGICAL_PORT_ID - 1;
        }
        else
        {
            VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(vxlan_tunnel.uc_vxlan_port, l_vxlan_port);
            addr_entry.ifindex = l_vxlan_port;
        }
    }

    addr_entry.life_time = permanent_status;
    addr_entry.source= AMTR_TYPE_ADDRESS_SOURCE_CONFIG;
    addr_entry.action= AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
    if (!AMTR_PMGR_SetAddrEntry(&addr_entry)) {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "amtr.addMacOfNetworkError", "Failed to add mac of vxlan network port.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete a mac entry of vni.
 *
 * @param vni           (required, number) VXLAN ID.
 *        mac           (required, string) mac address of specify route.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_MACs_Vxlan_VNI_MAC_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t *vni_p = NULL;
    json_t *mac_p = NULL;
    UI32_T  vni = 0;
    char *mac_str_p = NULL;
    AMTR_TYPE_AddrEntry_T addr_entry = {0};

    /* get parameter */
    vni_p = CGI_REQUEST_GetParamsValue(http_request, "vni");
    if (NULL == vni_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vni.");
    }
    vni = json_integer_value(vni_p);

    mac_p = CGI_REQUEST_GetParamsValue(http_request, "mac");
    if (NULL == mac_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get mac.");
    }
    mac_str_p = (char *)json_string_value(mac_p);

    /* check parameter */
    VXLAN_OM_VNI_T vni_entry = {0};
    vni_entry.vni = vni;
    if (VXLAN_TYPE_RETURN_OK != VXLAN_POM_GetVniEntry(&vni_entry))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "vni not exist.");
    }
    addr_entry.vid = vni_entry.vfi;

    if (!CGI_UTIL_MacStrToHex(mac_str_p, addr_entry.mac))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "mac format is invalid.");
    }

    if (!AMTR_PMGR_DeleteAddr(addr_entry.vid, addr_entry.mac))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "amtr.deleteMacOfVniError", "Failed to delete mac of vni.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

void CGI_MODULE_MAC_ADDRESS_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler   = CGI_MODULE_MACs_Read;
    handlers.create_handler = CGI_MODULE_MACs_Create;

    CGI_MAIN_Register("/api/v1/macs", &handlers, 0);

    {   /* for "/api/v1/macs/mac-address-table/{id}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.delete_handler = CGI_MODULE_MACs_MAC_ID_Delete;
        CGI_MAIN_Register("/api/v1/macs/mac-address-table/{id}", &handlers, 0);
    }

    {   /* for "/api/v1/macs/interface/{id}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.delete_handler = CGI_MODULE_MACs_INTF_ID_Delete;
        CGI_MAIN_Register("/api/v1/macs/interface/{id}", &handlers, 0);
    }

    {   /* for "/api/v1/macs/vxlan/{vni}/network-ports" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.create_handler = CGI_MODULE_MACs_Vxlan_VNI_NetworkPort_Create;
        CGI_MAIN_Register("/api/v1/macs/vxlan/{vni}/network-ports", &handlers, 0);
    }

    {   /* for "/api/v1/macs/vxlan/{vni}/{mac}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.delete_handler = CGI_MODULE_MACs_Vxlan_VNI_MAC_Delete;
        CGI_MAIN_Register("/api/v1/macs/vxlan/{vni}/{mac}", &handlers, 0);
    }

}

static void CGI_MODULE_MAC_ADDRESS_Init()
{
    CGI_MODULE_MAC_ADDRESS_RegisterHandlers();
}

static void CGI_MODULE_MAC_ADDRESS_GetOne(json_t *macs_p, AMTR_TYPE_AddrEntry_T *addr_entry_p)
{
    json_t *mac_obj_p = json_object();
    UI32_T  unit = 0;
    UI32_T  port = 0;
    UI32_T  trunk_id = 0;
    UI32_T  vid = 0;
    UI8_T   id_ar[128] = {0};
    UI8_T   mac_str_ar[18] = {0};
    char    if_id_str_ar[30] = {0};
    char    type_str_ar[18] = {0};
    char    life_time_str[20] = {0};
    SWCTRL_Lport_Type_T lport_type;
#if(SYS_CPNT_VXLAN == TRUE)
    AMTRL3_TYPE_VxlanTunnelEntry_T tunnel_entry;
    UI32_T r_vxlan_port = 0;
    SWCTRL_Lport_Type_T access_type;
    UI32_T access_lport = 0;
    UI32_T access_unit = 0;
    UI32_T access_port = 0;
    UI32_T access_trunk = 0;
    UI32_T vxlan_vni =0;
    UI16_T vxlan_vid = 0;
#endif

    if(addr_entry_p->source == AMTR_TYPE_ADDRESS_SOURCE_SELF) //ignore cpu
    {
        return;
    }

    /* type
     */
    switch(addr_entry_p->source)
    {
        case AMTR_TYPE_ADDRESS_SOURCE_CONFIG:

            sprintf(type_str_ar, "Config");
            break;

        case AMTR_TYPE_ADDRESS_SOURCE_INTERNAL:
            sprintf(type_str_ar, "Internal");
            break;

        case AMTR_TYPE_ADDRESS_SOURCE_SELF:
            sprintf(type_str_ar, "CPU");
            break;

        case AMTR_TYPE_ADDRESS_SOURCE_SECURITY:
            sprintf(type_str_ar, "Security");
            break;

        default:
            sprintf(type_str_ar, "Learn");
            break;
    }

    /* lifeTime
     */
    switch(addr_entry_p->life_time)
    {
        case AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT:
            sprintf(life_time_str, "Permanent");
            break;

        case AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET:
            sprintf(life_time_str, "Delete on Reset");
            break;

        case AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT:
            sprintf(life_time_str, "Delete on Timeout");
            break;

        default:
            sprintf(life_time_str, " Other");
            break;
    }

    /* mac
     */
    sprintf(mac_str_ar, "%02X-%02X-%02X-%02X-%02X-%02X",
        addr_entry_p->mac[0],
        addr_entry_p->mac[1],
        addr_entry_p->mac[2],
        addr_entry_p->mac[3],
        addr_entry_p->mac[4],
        addr_entry_p->mac[5]);

    /* vid
     */
#if(SYS_CPNT_VXLAN == TRUE)
    if(addr_entry_p->vid <= SYS_ADPT_MAX_VLAN_ID)
    {
        sprintf(id_ar, "%u_%s",addr_entry_p->vid, mac_str_ar);
        json_object_set_new(mac_obj_p, "vid", json_integer(addr_entry_p->vid));
        json_object_set_new(mac_obj_p, "vni", json_integer(0));
    }
    else
    {
        I32_T  vni;
        vni = VXLAN_POM_GetVniByVfi(addr_entry_p->vid);

        if (vni > 0)
        {
            sprintf(id_ar, "*%u_%s", vni, mac_str_ar);
            json_object_set_new(mac_obj_p, "vni", json_integer(vni));
        }
        else
        {
            sprintf(id_ar, "%u_%s",addr_entry_p->vid, mac_str_ar);
            json_object_set_new(mac_obj_p, "vid", json_integer(addr_entry_p->vid));
            json_object_set_new(mac_obj_p, "vni", json_integer(0));
        }
    }
#else
    sprintf(id_ar, "%u_%s",addr_entry_p->vid, mac_str_ar);
    json_object_set_new(mac_obj_p, "vid", json_integer(addr_entry_p->vid));
#endif

    /* ifId
     */
    if(addr_entry_p->source==AMTR_TYPE_ADDRESS_SOURCE_SELF)
    {
        sprintf(if_id_str_ar, "CPU");
    }
    else if(addr_entry_p->ifindex==0)
    {
        sprintf(if_id_str_ar, "NA");
    }
    else
    {
        lport_type = SWCTRL_POM_LogicalPortToUserPort(addr_entry_p->ifindex, &unit, &port, &trunk_id);

        if(lport_type==SWCTRL_LPORT_TRUNK_PORT)
        {
          sprintf(if_id_str_ar, "trunk%lu", (unsigned long)trunk_id);
        }
#if(SYS_CPNT_VXLAN == TRUE)
        else if (lport_type == SWCTRL_LPORT_VXLAN_PORT)
        {
            VXLAN_TYPE_L_PORT_CONVERTTO_R_PORT(addr_entry_p->ifindex, r_vxlan_port);
            if (r_vxlan_port != 0)
            {
                memset(&tunnel_entry, 0, sizeof(AMTRL3_TYPE_VxlanTunnelEntry_T));
                tunnel_entry.vxlan_port = r_vxlan_port;
                if (0 != addr_entry_p->r_vtep_ip[0])
                {   /* entry for static MAC of network port */
                    sprintf(if_id_str_ar, "Vx(%d.%d.%d.%d)",
                        addr_entry_p->r_vtep_ip[0], addr_entry_p->r_vtep_ip[1],
                        addr_entry_p->r_vtep_ip[2], addr_entry_p->r_vtep_ip[3]);
                    json_object_set_new(mac_obj_p, "vid", json_integer(0));
                }
                else if (AMTRL3_POM_GetVxlanTunnelEntryByVxlanPort(SYS_ADPT_DEFAULT_FIB, &tunnel_entry) == TRUE)
                {
                    sprintf(if_id_str_ar, "Vx(%d.%d.%d.%d)",
                        tunnel_entry.remote_vtep.addr[0], tunnel_entry.remote_vtep.addr[1],
                        tunnel_entry.remote_vtep.addr[2], tunnel_entry.remote_vtep.addr[3]);
                    json_object_set_new(mac_obj_p, "vid", json_integer(0));
                }
                else if (VXLAN_POM_GetVlanNlportOfAccessPort(r_vxlan_port, &vxlan_vid, &access_lport))
                {
                    access_type = SWCTRL_POM_LogicalPortToUserPort(addr_entry_p->ifindex, &access_unit, &access_port, &access_trunk);
                    if(access_type==SWCTRL_LPORT_TRUNK_PORT)
                    {
                        sprintf(if_id_str_ar, "trunk%lu", (unsigned long)access_trunk);
                    }
                    else
                    {
                        sprintf(if_id_str_ar, "eth%lu/%lu", (unsigned long)access_unit, (unsigned long)access_lport);
                    }
                    json_object_set_new(mac_obj_p, "vid", json_integer(vxlan_vid));
                }
                else
                {
                    sprintf(if_id_str_ar, "%s", "");
                }
            }
            else
            {
                sprintf(if_id_str_ar, "%s", "");
            }
        }
#endif
        else
        {
            sprintf(if_id_str_ar, "eth%lu/%lu", (unsigned long)unit, (unsigned long)port);
        }
    }
    json_object_set_new(mac_obj_p, "id", json_string(id_ar));
    json_object_set_new(mac_obj_p, "ifId", json_string(if_id_str_ar));
    json_object_set_new(mac_obj_p, "mac", json_string(mac_str_ar));
    json_object_set_new(mac_obj_p, "type", json_string(type_str_ar));
    json_object_set_new(mac_obj_p, "lifeTime", json_string( life_time_str));
    json_array_append_new(macs_p, mac_obj_p);
    return;
}

static BOOL_T CGI_MODULE_MAC_ADDRESS_ParseId(const char *str, UI16_T *vid, UI8_T *mac)
{
    char buf_ar[128] = {0};
    char *strtok_state_p = NULL;
    char *vid_str_p, *mac_str_p, *tmp_str_p;
    char *end_p;
    UI8_T  i = 0, count = 0;

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

    for (i=0;i<6;i++)
    {
        mac_str_p = strtok_r(NULL, "-", &strtok_state_p);
        if (mac_str_p != NULL)
        {
            mac[i] =  strtol(mac_str_p, &end_p, 16);
            count++;
        }
    }
    if (count != 6)
    {
        /* Format is wrong.
         */
        return FALSE;
    }
    return TRUE;
}