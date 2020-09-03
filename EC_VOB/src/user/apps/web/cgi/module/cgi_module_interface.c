#include "cgi_auth.h"
#include "stktplg_pom.h"
#include "swdrv_type.h"
#include "swctrl.h"
#include "swctrl_pom.h"
#include "nmtr_mgr.h"
#include "vlan_type.h"
#include "vlan_lib.h"
#include "vlan_pom.h"
#include "cmgr.h"
#include "netcfg_pmgr_ip.h"
#include "netcfg_pom_ip.h"
#include "ip_lib.h"
#include "l_inet.h"
#include "sys_adpt.h"
#include "inttypes.h"

#define CGI_MODULE_INTERFACE_ID_LEN   20
#define CGI_MODULE_INTERFACE_MAC_STR_LEN   18
#define CGI_MODULE_INTERFACE_TYPE_STR_LEN   18
static BOOL_T CGI_MODULE_INTERFACE_GetOne(UI32_T unit, UI32_T port, UI32_T tid, json_t *interface_obj_p);
static CGI_STATUS_CODE_T CGI_MODULE_INTERFACE_SetOneIp(HTTP_Request_T *http_request, HTTP_Response_T *http_response, BOOL_T create_interface);
static BOOL_T CGI_MODULE_INTERFACE_VlanGetOne(UI32_T vid, json_t *interface_obj_p);
//static BOOL_T CGI_MODULE_INTERFACE_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request);
static BOOL_T CGI_MODULE_INTERFACE_GetVlan(const char *str, UI32_T *vid);
static BOOL_T CGI_MODULE_INTERFACE_GetTrunk(const char *str, UI32_T *tid);

/**----------------------------------------------------------------------
 * This API is used to get interface info.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_INTERFACEs_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    UI32_T  pmax = 0, unit = 0, pidx = 0;
    json_t  *interfaces_p;

    STKTPLG_POM_GetNextUnit(&unit);
    pmax = SWCTRL_POM_UIGetUnitPortNumber(unit);
    if (pmax == 0)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "No port exists.");
    }

    /* Get all interfaces.
     */
    interfaces_p = json_array();
    json_object_set_new(result_p, "interfaces", interfaces_p);
    for (pidx =1; pidx <= pmax; pidx++)
    {
        json_t *interface_obj_p = json_object();
        if (TRUE == CGI_MODULE_INTERFACE_GetOne(unit, pidx, 0, interface_obj_p))
        {
            json_array_append_new(interfaces_p, interface_obj_p);
        }
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}


/**----------------------------------------------------------------------
 * This API is used to get an interface info by ID.
 *
 * @param id (required, string) Interface ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_INTERFACEs_ID_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    const char    *id_str_p;
    UI32_T  unit = 0, port = 0, vid = 0, tid = 0;
    BOOL_T  ret = FALSE;
    char    buffer_ar[20] = {0};

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (NULL == id_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Interface ID is NULL.");
    }
    id_str_p = json_string_value(id_p);

    CGI_UTIL_UrlDecode(buffer_ar, id_str_p);
    if (TRUE == CGI_UTIL_InterfaceIdToEth((const char *) buffer_ar, &unit, &port))
    {
        ret = CGI_MODULE_INTERFACE_GetOne(unit, port, 0, result_p);
    }
    else if (TRUE == CGI_MODULE_INTERFACE_GetVlan(buffer_ar, &vid))
    {
        ret = CGI_MODULE_INTERFACE_VlanGetOne(vid, result_p);
    }
    else if (TRUE == CGI_MODULE_INTERFACE_GetTrunk(buffer_ar, &tid))
    {
        ret = CGI_MODULE_INTERFACE_GetOne(0, 0, tid, result_p);
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Error ID.");
    }

    if (FALSE == ret)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "Failled to get interface entry.");
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to put an interface info by ID.
 *
 * @param id        (required, string) Interface ID
 * @param isEnabled (Optional, boolean) Shuts down the selected port or not
 * @param pvid      (Optional, number) Port VLAN ID
 * @param vlanMode  (Optional, string) Port mode: access, hybrid, trunk
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_INTERFACEs_ID_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    json_t  *is_enabled_p;
    json_t  *pvid_p;
    json_t  *mode_p;
    //json_t  *l2_proto_p;
    const char    *id_str_p;
    const char    *mode_str_p;
    //const char    *l2_proto_str_p;
    UI32_T  pvid = 0;
    UI32_T  is_enabled_value;
    UI32_T  lport = 0, vid = 0;
    UI32_T  port_mode = 0;
    char    buffer_ar[20] = {0};

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (NULL == id_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Interface ID is NULL.");
    }
    id_str_p = json_string_value(id_p);

    is_enabled_p = CGI_REQUEST_GetBodyValue(http_request, "isEnabled");
    pvid_p = CGI_REQUEST_GetBodyValue(http_request, "pvid");
    mode_p = CGI_REQUEST_GetBodyValue(http_request, "vlanMode");
    //l2_proto_p = CGI_REQUEST_GetBodyValue(http_request, "l2protocolTunnel");

    if ((NULL == is_enabled_p) && (NULL == pvid_p) && (NULL == mode_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Please specify isEnabled, mode, or pvid value.");
    }

    CGI_UTIL_UrlDecode(buffer_ar, id_str_p);
    if (TRUE == CGI_UTIL_InterfaceIdToLport((const char *) buffer_ar, &lport))
    {
        if (NULL != is_enabled_p)
        {
            if (TRUE != json_is_boolean(is_enabled_p))
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The isEnabled is not boolean type.");
            }

            if (TRUE == json_boolean_value(is_enabled_p))
                is_enabled_value = VAL_ifAdminStatus_up;
            else
                is_enabled_value = VAL_ifAdminStatus_down;

            if (FALSE == CMGR_SetPortAdminStatus(lport, is_enabled_value))
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "cmgr.setPortAdminStatusError", "Failed to  port admin status.");
        }

        if (NULL != pvid_p)
        {
            pvid = json_integer_value(pvid_p);
            if (TRUE != VLAN_PMGR_SetDot1qPvid(lport, pvid))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vlan.setDot1qPvidError", "Failed to set port PVID.");
            }
        }

        if (NULL != mode_p)
        {
            mode_str_p = json_string_value(mode_p);

            if (0 == strncmp(mode_str_p, "trunk", strlen("trunk")))
            {
                port_mode = VAL_vlanPortMode_dot1qTrunk;
            }
            else if (0 == strncmp(mode_str_p, "access", strlen("access")))
            {
                port_mode = VAL_vlanPortMode_access;
            }
            else if (0 == strncmp(mode_str_p, "hybrid", strlen("hybrid")))
            {
                port_mode = VAL_vlanPortMode_hybrid;
            }
            else
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid mode.");
            }

            if (!VLAN_PMGR_SetVlanPortMode(lport, port_mode))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vlan.setVlanPortModeError", "Failed to set port mode.");
            }
        } //if (NULL != mode_p)
        /*
        {
            if (NULL != l2_proto_p)
            {
                l2_proto_str_p = json_string_value(l2_proto_p);

#if (SYS_CPNT_LACP == TRUE)
                if (0 == strncmp(mode_str_p, "lacp", strlen("lacp")))
                {
                    UI32_T state = VAL_lacpPortStatus_enabled;

                    if (LACP_RETURN_SUCCESS!= LACP_PMGR_SetDot3adLacpPortEnabled(lport, state))
                    {
                        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                                "lacp.setDot3adLacpPortEnabledError", "Failed to set port LACP.");
                    }
                }
#endif
            }
        }
        */
        return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
    }
    else if (TRUE == CGI_MODULE_INTERFACE_GetVlan(buffer_ar, &vid))
    {
        return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
    }
    else if (memcmp(buffer_ar, "trunk", 5) == 0)
    {
        return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Error ID.");
    }
}

/**----------------------------------------------------------------------
 * This API is used to create a L2/L3 interface info by ID.
 *
 * @param id   (required, string) Interface ID
 * @param ip (optional, string) IPv4 address/prefix
 * @param role (optional, string) Pv4 address role
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_INTERFACEs_ID_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    return CGI_MODULE_INTERFACE_SetOneIp(http_request, http_response, TRUE);
}

/**----------------------------------------------------------------------
 * This API is used to delete a L2/L3 interface info by ID.
 *
 * @param id (required, string) Interface ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_INTERFACEs_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    const char    *id_str_p;
    UI32_T  vid = 0, loopback_index = 0;
    NETCFG_TYPE_L3_Interface_T intf;

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (NULL == id_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Interface ID is NULL.");
    }
    id_str_p = json_string_value(id_p);

    if ('l' == id_str_p[0]) //loopback
    {
        if (sscanf (id_str_p, "lo%" SCNu32, &loopback_index) != 1) {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Error loopback ID.");
        }

        memset(&intf, 0, sizeof(intf));
        IP_LIB_ConvertLoopbackIdToIfindex(loopback_index, &intf.ifindex);

        if (NETCFG_TYPE_OK == NETCFG_POM_IP_GetL3Interface(&intf))
        {
            if (NETCFG_TYPE_OK != NETCFG_PMGR_IP_DeleteLoopbackInterface(loopback_index))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "netconf.deleteLoopbackIfFailError", "Failed to delete loopback interface.");
            }
        }

        return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
    }
    else if (TRUE == CGI_MODULE_INTERFACE_GetVlan(id_str_p, &vid))
    {
        if (VLAN_POM_IsVlanExisted(vid))
        {
            char cmd_ar[CGI_UTIL_LINUX_CMD_SIZE] = {0};

            sprintf(cmd_ar, "ip link set VLAN%d nomaster", vid);

            if (TRUE != CGI_UTIL_ExecLinuxSetCommand(cmd_ar))
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "Failed", "Failed to unbind vlan vrf.");
            }

            if (NETCFG_PMGR_IP_DeleteL3If(vid) != NETCFG_TYPE_OK)
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netconf.deleteL3IfFailError", "Failed to delete L3 interface on VLAN.");
            }
        }
        return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Error ID.");
    }
}

/**----------------------------------------------------------------------
 * This API is used to add IP address.
 *
 * @param id   (required, string) Interface ID
 * @param ip (required, string) IPv4 address/prefix
 * @param role (optional, string) Pv4 address role
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_INTERFACEs_ID_IP_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    return CGI_MODULE_INTERFACE_SetOneIp(http_request, http_response, FALSE);
}

/**----------------------------------------------------------------------
 * This API is used to remove IP address.
 *
 * @param id   (required, string) Interface ID
 * @param ipId (required, string) Unique IP ID (ip_preflen)
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_INTERFACEs_ID_IP_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    json_t  *ip_id_p;
    const char    *id_str_p;
    const char    *ip_id_str_p;
    char    *tmp_p;
    UI32_T  vid = 0, vid_ifindex = 0, loopback_index = 0;
    UI32_T  ret;
    BOOL_T is_loopback = FALSE;
    NETCFG_TYPE_InetRifConfig_T rif_config;

    memset(&rif_config, 0, sizeof(rif_config));

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (NULL == id_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The interface ID is NULL.");
    }
    id_str_p = json_string_value(id_p);

    ip_id_p = CGI_REQUEST_GetParamsValue(http_request, "ipId");
    if (NULL == ip_id_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The ipId is NULL.");
    }
    ip_id_str_p = json_string_value(ip_id_p);
    if (NULL != (tmp_p = strchr(ip_id_str_p, '_')))
    {
        *tmp_p = '/';
    }

    /* format: 192.168.1.1/24
     */
    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_PREFIX,
                                                       ip_id_str_p,
                                                       (L_INET_Addr_T *) &rif_config.addr,
                                                       sizeof(rif_config.addr)))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid IP address.");
    }

    if (('l' != id_str_p[0]) && (TRUE != CGI_MODULE_INTERFACE_GetVlan(id_str_p, &vid)))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Error ID.");
    }

    if ('l' == id_str_p[0]) //loopback interface
    {
        if (sscanf (id_str_p, "lo%" SCNu32, &loopback_index) != 1) {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Error loopback ID.");
        }

        IP_LIB_ConvertLoopbackIdToIfindex(loopback_index, &rif_config.ifindex);
        is_loopback = TRUE;
    }
    else if (TRUE == CGI_MODULE_INTERFACE_GetVlan(id_str_p, &vid))
    {
        if(VLAN_POM_IsVlanExisted(vid))
        {
            if (VLAN_OM_ConvertToIfindex(vid, &vid_ifindex) == FALSE)
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vlan.convertToIfindexError", "Failed to get VLAN interface value.");
            rif_config.ifindex = vid_ifindex;
        }
    }

    if(NETCFG_TYPE_OK != NETCFG_POM_IP_GetRifFromInterface(&rif_config))
    {
         return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netconf.getRif", "Can't find the address on VLAN.");
    }
    rif_config.row_status = VAL_netConfigStatus_2_destroy;

    if (FALSE == is_loopback)
    {
        if( NETCFG_PMGR_IP_SetIpAddressMode (vid_ifindex, NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE) != NETCFG_TYPE_OK)
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netcfg.ip.setIpAddressModeError", "Failed to set the IP address mode as user define mode on VLAN.");
        }
    }

    ret = NETCFG_PMGR_IP_SetInetRif(&rif_config, NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB);
    if(ret != NETCFG_TYPE_OK)
    {
        if (TRUE == is_loopback)
        {
            char cmd_ar[CGI_UTIL_LINUX_CMD_SIZE] = {0};

            sprintf(cmd_ar, "ifconfig %s %s", id_str_p, "0");

            if (TRUE != CGI_UTIL_ExecLinuxSetCommand(cmd_ar))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netcfg.ip.setInetRif", "Failed to set the IP address to loopback.");
            }
        }
        else
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netcfg.ip.setInetRifError", "Failed to set IP address on VLAN.");
        }
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

void CGI_MODULE_INTERFACEs_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler   = CGI_MODULE_INTERFACEs_Read;

    CGI_MAIN_Register("/api/v1/interfaces", &handlers, 0);
}

void CGI_MODULE_INTERFACEs_ID_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler   = CGI_MODULE_INTERFACEs_ID_Read;
    handlers.update_handler = CGI_MODULE_INTERFACEs_ID_Update;
    handlers.create_handler = CGI_MODULE_INTERFACEs_ID_Create;
    handlers.delete_handler = CGI_MODULE_INTERFACEs_ID_Delete;
    CGI_MAIN_Register("/api/v1/interfaces/{id}", &handlers, 0);
}

void CGI_MODULE_INTERFACEs_ID_IP_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.create_handler = CGI_MODULE_INTERFACEs_ID_IP_Create;
    CGI_MAIN_Register("/api/v1/interfaces/{id}/ipaddrs", &handlers, 0);

    {   /* for "/api/v1/interfaces/{id}/ipaddrs/{ipId}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.delete_handler = CGI_MODULE_INTERFACEs_ID_IP_Delete;
        CGI_MAIN_Register("/api/v1/interfaces/{id}/ipaddrs/{ipId}", &handlers, 0);
    }
}

static void CGI_MODULE_INTERFACE_Init()
{
    CGI_MODULE_INTERFACEs_RegisterHandlers();
    CGI_MODULE_INTERFACEs_ID_RegisterHandlers();
    CGI_MODULE_INTERFACEs_ID_IP_RegisterHandlers();
}

/*
static BOOL_T CGI_MODULE_INTERFACE_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request)
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

static BOOL_T CGI_MODULE_INTERFACE_GetOne(UI32_T unit, UI32_T port, UI32_T tid, json_t *interface_obj_p)
{
    UI32_T  lport = 0, speed = 0;
    UI32_T  ret = FALSE;
    UI8_T   port_mac_ar[SYS_TYPE_MAC_ADDR_LEN] = {0};
    char    mac_str_ar[CGI_MODULE_INTERFACE_MAC_STR_LEN] = {0};
    char    id_ar[CGI_MODULE_INTERFACE_ID_LEN] = {0};
    char    type_ar[CGI_MODULE_INTERFACE_TYPE_STR_LEN] = {0};
    SWCTRL_Lport_Type_T         lport_type = SWCTRL_LPORT_UNKNOWN_PORT;
    Port_Info_T                 port_info;
    VLAN_OM_Vlan_Port_Info_T    vlan_port_info;

    if (0 == tid)
    {
        lport_type = SWCTRL_POM_UserPortToIfindex(unit, port, &lport);
    }
    else
    {
        if(FALSE == SWCTRL_POM_TrunkIDToLogicalPort(tid, &lport))
        {
            return FALSE;
        }
    }

    memset(mac_str_ar, 0, CGI_MODULE_INTERFACE_MAC_STR_LEN);
    memset(id_ar, 0, CGI_MODULE_INTERFACE_ID_LEN);
    memset(&port_info, 0, sizeof(Port_Info_T));
    memset(&vlan_port_info, 0, sizeof(VLAN_OM_Vlan_Port_Info_T));
    if (    (TRUE == SWCTRL_POM_GetPortInfo(lport, &port_info))
        &&  (TRUE == VLAN_PMGR_GetPortEntry(lport, &vlan_port_info))
       )
    {
        if (0 == tid)
        {
            sprintf(id_ar, "eth%lu/%lu",(unsigned long)unit, (unsigned long)port);
        }
        else
        {
            sprintf(id_ar, "trunk%lu",(unsigned long)tid);
        }

        SWCTRL_POM_GetPortMac(lport, port_mac_ar);
        sprintf(mac_str_ar, "%02X-%02X-%02X-%02X-%02X-%02X",
            port_mac_ar[0], port_mac_ar[1], port_mac_ar[2], port_mac_ar[3], port_mac_ar[4], port_mac_ar[5]);

        switch(port_info.port_type)
        {
            case VAL_portType_hundredBaseTX:
            case VAL_portType_thousandBaseT:
            case VAL_portType_tenG:
            case VAL_portType_tenGBaseT:
                strncpy(type_ar, "COPPER", sizeof(type_ar));
                break;
            default:
                strncpy(type_ar, "FIBER", sizeof(type_ar));
                break;
        }

        switch(port_info.speed_duplex_oper)
        {
            case VAL_portSpeedDpxCfg_halfDuplex10:
            case VAL_portSpeedDpxCfg_fullDuplex10:
                speed = 10;
                break;

            case VAL_portSpeedDpxCfg_halfDuplex100:
            case VAL_portSpeedDpxCfg_fullDuplex100:
                speed = 100;
                break;

            case VAL_portSpeedDpxCfg_halfDuplex1000:
            case VAL_portSpeedDpxCfg_fullDuplex1000:
                speed = 1000;
                break;

            case VAL_portSpeedDpxCfg_halfDuplex10g:
            case VAL_portSpeedDpxCfg_fullDuplex10g:
                speed = 10000;
                break;

            case VAL_portSpeedDpxCfg_halfDuplex40g:
            case VAL_portSpeedDpxCfg_fullDuplex40g:
                speed = 40000;
            break;

            default:
                speed=0;
                break;
        }

        json_object_set_new(interface_obj_p, "id", json_string(id_ar));
        json_object_set_new(interface_obj_p, "ifName", json_string(port_info.port_name));
        json_object_set_new(interface_obj_p, "portType", json_string(type_ar));
        json_object_set_new(interface_obj_p, "isEnabled", json_boolean((port_info.admin_state == VAL_ifAdminStatus_up)));
        json_object_set_new(interface_obj_p, "speed", json_integer(speed));
        json_object_set_new(interface_obj_p, "portNumber", json_integer(lport));
        json_object_set_new(interface_obj_p, "mac", json_string(mac_str_ar));
        json_object_set_new(interface_obj_p, "linkStatus", json_string(((port_info.link_status == SWCTRL_LINK_UP)?"UP":"Down")));
        json_object_set_new(interface_obj_p, "pvid", json_integer(vlan_port_info.port_item.dot1q_pvid_index));
        {
            json_t    *vlan_untagged = json_array();
            json_t    *vlan_tagged = json_array();
            UI32_T    vid = 0, vid_ifindex=0, trunk_id=0, trunk_ifindex=0;
            VLAN_OM_Dot1qVlanCurrentEntry_T  vlan_info;

            json_object_set_new(interface_obj_p, "allowedVlansUntagged", vlan_untagged);
            json_object_set_new(interface_obj_p, "allowedVlansTagged", vlan_tagged);

            memset(&vlan_info, 0, sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
            while(VLAN_POM_GetNextDot1qVlanCurrentEntry(0, &vid, &vlan_info))
            {
                VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
                if (    (lport_type == SWCTRL_LPORT_TRUNK_PORT_MEMBER)
                    &&  (SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_POM_UIUserPortToTrunkPort(unit, port, &trunk_id))
                    &&  (TRUE == SWCTRL_POM_TrunkIDToLogicalPort(trunk_id, &trunk_ifindex))
                   )
                {
                     lport = trunk_ifindex;
                }

                if (VLAN_POM_IsPortVlanMember(vid_ifindex, lport))
                {
                    if (vlan_info.dot1q_vlan_current_untagged_ports[(UI32_T)( (lport - 1)/8 )] & (1 << ( 7 - ( (lport - 1) % 8) ) ) )
                    {
                        json_array_append_new(vlan_untagged, json_integer(vid));
                    }
                    else
                    {
                        json_array_append_new(vlan_tagged, json_integer(vid));
                    }
                }
            }
        }
        ret = TRUE;
    }
    return ret;
}

static CGI_STATUS_CODE_T CGI_MODULE_INTERFACE_SetOneIp(HTTP_Request_T *http_request, HTTP_Response_T *http_response, BOOL_T create_interface)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    json_t  *ip_p;
    json_t  *role_p;
    const char    *id_str_p;
    const char    *ip_str_p;
    const char    *role_str_p;
    UI32_T  vid = 0, vid_ifindex = 0, loopback_index = 0;
    UI32_T  ret;
    NETCFG_TYPE_InetRifConfig_T rif_config;
    UI8_T byte_mask_ar[SYS_ADPT_IPV4_ADDR_LEN] = {};
    BOOL_T is_loopback = FALSE;

    memset(&rif_config, 0, sizeof(rif_config));

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (NULL == id_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Interface ID is NULL.");
    }
    id_str_p = json_string_value(id_p);

    if (('l' != id_str_p[0]) && (TRUE != CGI_MODULE_INTERFACE_GetVlan(id_str_p, &vid)))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Error ID.");
    }

    if ('l' == id_str_p[0]) //loopback interface
    {
        if (sscanf (id_str_p, "lo%" SCNu32, &loopback_index) != 1) {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Error loopback ID.");
        }

        is_loopback = TRUE;
    }
    else if(!VLAN_POM_IsVlanExisted(vid))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "No such VLAN. Please create it first");
    }

    ip_p = CGI_REQUEST_GetBodyValue(http_request, "ip");
    role_p = CGI_REQUEST_GetBodyValue(http_request, "role");

    if ((TRUE == create_interface) && (NULL == ip_p) && (NULL == role_p) )
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Shall specify ip.");
    }

    if (TRUE == create_interface)
    {
        if (TRUE == is_loopback)
        {
            if (NETCFG_PMGR_IP_CreateLoopbackInterface(loopback_index) != NETCFG_TYPE_OK)
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Cannot create the loopback ID");
            }

            //to create loopback interface: use loopback index
            //to set loopback interface address, use (loopback index + SYS_ADPT_LOOPBACK_IF_INDEX_NUMBER)
            IP_LIB_ConvertLoopbackIdToIfindex(loopback_index, &rif_config.ifindex);
        }
        else
        {
            /* L3 interface.
             */
            if ((ret = NETCFG_PMGR_IP_CreateL3If(vid)) != NETCFG_TYPE_OK)
            {
                if (ret == NETCFG_TYPE_MAC_COLLISION)
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netcfg.ip.createL3IfError", "Failed, MAC collision. Please clear MAC address table and try again.");
                else if(ret == NETCFG_TYPE_TABLE_FULL)
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netcfg.ip.createL3IfError", "Failed, interface table is full.");
                else
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netcfg.ip.createL3IfError", "Failed to change interface type.");
            }
        }
    }

    /* format: 192.168.1.1/24
     */
    ip_str_p = json_string_value(ip_p);
    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_PREFIX,
                                                       ip_str_p,
                                                       (L_INET_Addr_T *) &rif_config.addr,
                                                       sizeof(rif_config.addr)))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid IP address.");
    }

    IP_LIB_CidrToMask(rif_config.addr.preflen, byte_mask_ar);

    if (NULL == role_p)
    {
        rif_config.ipv4_role = NETCFG_TYPE_MODE_PRIMARY; /* default */
    }
    else
    {
        role_str_p = json_string_value(role_p);
        if (0 == strncmp(role_str_p, "primary", 7))
        {
            rif_config.ipv4_role = NETCFG_TYPE_MODE_PRIMARY;
        }
        else if (0 == strncmp(role_str_p, "secondary", 9))
        {
            rif_config.ipv4_role = NETCFG_TYPE_MODE_SECONDARY;
        }
        else if (0 == strncmp(role_str_p, "virtual", 7))
        {
            rif_config.ipv4_role = NETCFG_TYPE_MODE_VIRTUAL;
        }
        else
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The role is wrong, please confugure primary, secondary, or virtual.");
        }
    }
    rif_config.row_status = VAL_netConfigStatus_2_createAndGo;

    if (TRUE != is_loopback)
    {
        ret = IP_LIB_IsValidForIpConfig(rif_config.addr.addr, byte_mask_ar);
        switch(ret)
        {
            case IP_LIB_INVALID_IP:
            case IP_LIB_INVALID_IP_LOOPBACK_IP:
            case IP_LIB_INVALID_IP_ZERO_NETWORK:
            case IP_LIB_INVALID_IP_BROADCAST_IP:
            case IP_LIB_INVALID_IP_IN_CLASS_D:
            case IP_LIB_INVALID_IP_IN_CLASS_E:
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid IP address.");
            case IP_LIB_INVALID_IP_SUBNET_NETWORK_ID:
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid IP address. Can't be network ID.");
            case IP_LIB_INVALID_IP_SUBNET_BROADCAST_ADDR:
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid IP address. Can't be network b'cast IP..");
            default:
                break;
        }
    }

    if (TRUE == is_loopback)
    {
        if (NETCFG_TYPE_OK != NETCFG_PMGR_IP_SetInetRif(&rif_config, NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB))
        {
            char cmd_ar[CGI_UTIL_LINUX_CMD_SIZE] = {0};

            sprintf(cmd_ar, "ifconfig %s %s", id_str_p, ip_str_p);

            if (TRUE != CGI_UTIL_ExecLinuxSetCommand(cmd_ar))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netcfg.ip.setInetRif", "Failed to set the IP address to loopback.");
            }
        }
    }
    else
    {
        if (VLAN_OM_ConvertToIfindex(vid, &vid_ifindex) == FALSE)
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vlan.convertToIfindexError", "Failed to get VLAN interface value.");

        rif_config.ifindex = vid_ifindex;
        if( NETCFG_PMGR_IP_SetIpAddressMode(vid_ifindex, NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE) != NETCFG_TYPE_OK)
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netcfg.ip.setIpAddressModeError", "Failed to set the IP address mode as user define mode on VLAN.");
        }
        else
        {
            ret = NETCFG_PMGR_IP_SetInetRif(&rif_config, NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB);
            if(ret != NETCFG_TYPE_OK)
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netcfg.ip.setInetRifError", "Failed to set IP address on VLAN.");
            }
        }
    }


    if ((TRUE == create_interface) && (TRUE != is_loopback))
    {
        json_t  *vrf_p = CGI_REQUEST_GetBodyValue(http_request, "vrf");

        if (NULL != vrf_p)
        {
            UI32_T vrf = json_integer_value(vrf_p);
            char cmd_ar[CGI_UTIL_LINUX_CMD_SIZE] = {0};

            if (0 < vrf)
            {
                sprintf(cmd_ar, "ip link add VRF%d type vrf table %d", vrf, vrf);

                if (TRUE != CGI_UTIL_ExecLinuxSetCommand(cmd_ar))
                {
                    return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "Failed", "Failed to create vrf.");
                }

                sprintf(cmd_ar, "ip link set VLAN%d vrf VRF%d", vid, vrf);

                if (TRUE != CGI_UTIL_ExecLinuxSetCommand(cmd_ar))
                {
                    return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "Failed", "Failed to bind vrf to vlan.");
                }
            }
        }
    } //vrf

    if (FALSE == create_interface)
    {
        char   id_ar[128]={0};
        /* Only add a IP, need to return id.
         */
        sprintf(id_ar, "%d.%d.%d.%d_%lu",
            rif_config.addr.addr[0], rif_config.addr.addr[1],
            rif_config.addr.addr[2], rif_config.addr.addr[3],
            (unsigned long)rif_config.addr.preflen);
        json_object_set_new(result_p, "id", json_string(id_ar));
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

static BOOL_T CGI_MODULE_INTERFACE_VlanGetOne(UI32_T vid, json_t *interface_obj_p)
{
    json_t  *ipaddrs_p = json_array();
    UI32_T  vid_ifindex = 0;
    UI32_T  access_mode;
    BOOL_T  ret = FALSE;
    char    id_ar[20] = {0};
    char    ip_ar[20] = {0};
    NETCFG_TYPE_L3_Interface_T intf;
    NETCFG_TYPE_InetRifConfig_T rif_config;

    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
    memset(&intf, 0, sizeof(intf));
    intf.ifindex = vid_ifindex;

    if (NETCFG_TYPE_OK == NETCFG_POM_IP_GetL3Interface(&intf))
    {
        json_object_set_new(interface_obj_p, "ipAddrs", ipaddrs_p);

        if (    (NETCFG_POM_IP_GetIpAddressMode(vid_ifindex, &access_mode) == NETCFG_TYPE_OK)
            &&  (access_mode == NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE)
           )
        {
            /* get primary rif first
             */
            memset(&rif_config, 0, sizeof(rif_config));
            rif_config.ifindex = vid_ifindex;
            if(NETCFG_TYPE_OK == NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config))
            {
                if(rif_config.addr.type == L_INET_ADDR_TYPE_IPV4)
                {
                    json_t *ip_obj_p = json_object();

                    memset(id_ar, 0, sizeof(id_ar));
                    memset(ip_ar, 0, sizeof(ip_ar));
                    sprintf(id_ar, "%d.%d.%d.%d_%lu",
                        rif_config.addr.addr[0], rif_config.addr.addr[1],
                        rif_config.addr.addr[2], rif_config.addr.addr[3],
                        (unsigned long)rif_config.addr.preflen);
                    sprintf(ip_ar, "%d.%d.%d.%d/%lu",
                        rif_config.addr.addr[0], rif_config.addr.addr[1],
                        rif_config.addr.addr[2], rif_config.addr.addr[3],
                        (unsigned long)rif_config.addr.preflen);

                    json_object_set_new(ip_obj_p, "id", json_string(id_ar));
                    json_object_set_new(ip_obj_p, "ip", json_string(ip_ar));
                    json_object_set_new(ip_obj_p, "role", json_string("primary"));
                    json_array_append_new(ipaddrs_p, ip_obj_p);
                }
            }

            /* get secondary rif
             */
            memset(&rif_config, 0, sizeof(rif_config));
            rif_config.ifindex = vid_ifindex;
            while(NETCFG_TYPE_OK == NETCFG_POM_IP_GetNextSecondaryRifFromInterface(&rif_config))
            {
                if(rif_config.addr.type == L_INET_ADDR_TYPE_IPV4)
                {
                    json_t *ip_obj_p = json_object();

                    memset(id_ar, 0, sizeof(id_ar));
                    memset(ip_ar, 0, sizeof(ip_ar));
                    sprintf(id_ar, "%d.%d.%d.%d_%lu",
                        rif_config.addr.addr[0], rif_config.addr.addr[1],
                        rif_config.addr.addr[2], rif_config.addr.addr[3],
                        (unsigned long)rif_config.addr.preflen);
                    sprintf(ip_ar, "%d.%d.%d.%d/%lu",
                        rif_config.addr.addr[0], rif_config.addr.addr[1],
                        rif_config.addr.addr[2], rif_config.addr.addr[3],
                        (unsigned long)rif_config.addr.preflen);

                    json_object_set_new(ip_obj_p, "id", json_string(id_ar));
                    json_object_set_new(ip_obj_p, "ip", json_string(ip_ar));
                    json_object_set_new(ip_obj_p, "role", json_string("secondary"));
                    json_array_append_new(ipaddrs_p, ip_obj_p);
                }
            }

#if (SYS_CPNT_VIRTUAL_IP == TRUE)
            /* get virtual rif */
            memset(&rif_config, 0, sizeof(rif_config));
            rif_config.ifindex = vid_ifindex;

            while(NETCFG_TYPE_OK == NETCFG_POM_IP_GetNextVirtualRifByIfindex(&rif_config))
            {
                json_t *ip_obj_p = json_object();

                memset(id_ar, 0, sizeof(id_ar));
                memset(ip_ar, 0, sizeof(ip_ar));
                sprintf(id_ar, "%d.%d.%d.%d_%lu",
                    rif_config.addr.addr[0], rif_config.addr.addr[1],
                    rif_config.addr.addr[2], rif_config.addr.addr[3],
                    (unsigned long)rif_config.addr.preflen);
                sprintf(ip_ar, "%d.%d.%d.%d/%lu",
                    rif_config.addr.addr[0], rif_config.addr.addr[1],
                    rif_config.addr.addr[2], rif_config.addr.addr[3],
                    (unsigned long)rif_config.addr.preflen);

                json_object_set_new(ip_obj_p, "id", json_string(id_ar));
                json_object_set_new(ip_obj_p, "ip", json_string(ip_ar));
                json_object_set_new(ip_obj_p, "role", json_string("virtual"));
                json_array_append_new(ipaddrs_p, ip_obj_p);
            }
#endif
        }

        ret = TRUE;
    }

    if(TRUE == VLAN_POM_IsVlanExisted(vid))
    {
        ret = TRUE;
    }
    return ret;
}

static BOOL_T CGI_MODULE_INTERFACE_GetVlan(const char *str, UI32_T *vid)
{
    if (strlen(str) < 5)
        return FALSE;
    if (memcmp(str, "vlan", 4) != 0)
        return FALSE;
    *vid = atoi(str + 4);
    if (*vid == 0)
        return FALSE;
    return TRUE;
}

static BOOL_T CGI_MODULE_INTERFACE_GetTrunk(const char *str, UI32_T *tid)
{
    if (strlen(str) < 6)
        return FALSE;
    if (memcmp(str, "trunk", 5) != 0)
        return FALSE;
    *tid = atoi(str + 5);
    if (*tid == 0)
        return FALSE;
    return TRUE;
}
