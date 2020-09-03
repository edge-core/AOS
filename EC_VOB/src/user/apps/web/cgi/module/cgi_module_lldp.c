#include "cgi_auth.h"
#include "stktplg_pmgr.h"
#include "swctrl_pom.h"
#include "lldp_pmgr.h"
#include "lldp_pom.h"
#include "lldp_type.h"

#define CGI_MODULE_LLDP_MAC_FORMAT(out, in, len) do {  \
    UI32_T i;                                          \
    UI8_T  message[10] ={0};                           \
    if (len > 0)                                       \
    {                                                  \
        sprintf((char*)message, "%02X", in[0]);        \
        strcat(out, message);                          \
        for (i = 1; i < len; i++)                      \
        {                                              \
           memset(message, 0, sizeof(message));        \
           sprintf((char*)message, "-%02X", in[i]);    \
           strcat(out, message);                       \
        }                                              \
    }                                                  \
} while (0)

#define CGI_MODULE_LLDP_ID_LEN   20

static BOOL_T CGI_MODULE_LLDP_GetOne(UI32_T unit, UI32_T port, json_t *interface_obj_p);
//static BOOL_T CGI_MODULE_LLDP_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request);

/**----------------------------------------------------------------------
 * This API is used to get LLDP info.
 *
 * @param NA.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_LLDP_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    UI32_T admin_status;
    LLDP_MGR_SysConfigEntry_T   sys_config_entry;

    if (LLDP_POM_GetSysAdminStatus(&admin_status) != LLDP_TYPE_RETURN_OK)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "lldp.getSysAdminStatusError", "Failed to get status.");
    }

    memset(&sys_config_entry, 0, sizeof(sys_config_entry));
    if (LLDP_POM_GetSysConfigEntry(&sys_config_entry) != LLDP_TYPE_RETURN_OK)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "lldp.getSysConfigError", "Failed to get transmitInterval.");
    }
    json_object_set_new(result_p, "status", json_boolean((admin_status == LLDP_TYPE_SYS_ADMIN_ENABLE)));
    json_object_set_new(result_p, "transmitInterval", json_integer(sys_config_entry.message_tx_interval));
    json_object_set_new(result_p, "delayInterval", json_integer(sys_config_entry.tx_delay));
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to update LLDP info.
 *
 * @param status (required, boolean) LLDP status
 * @param transmitInterval (required, number) LLDP transmission interval (in seconds)
  * @param delayInterval (required, number) LLDP tx delay interval (in seconds)
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_LLDP_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *status_p;
    json_t  *transmit_interval_p;
    json_t  *delay_interval_p;
    UI32_T  status_value = 0;
    UI32_T  transmit_interval = 0;
    UI32_T  delay_interval = 0;

    status_p = CGI_REQUEST_GetBodyValue(http_request, "status");
    transmit_interval_p = CGI_REQUEST_GetBodyValue(http_request, "transmitInterval");
    delay_interval_p = CGI_REQUEST_GetBodyValue(http_request, "delayInterval");
    if ((NULL == status_p) && (NULL == transmit_interval_p) && (NULL == delay_interval_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Please specify status, transmitInterval, or delayInterval  value.");
    }

    if (NULL != status_p)
    {
        if (TRUE != json_is_boolean(status_p))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
        }

        if (TRUE == json_boolean_value(status_p))
        {
            status_value = LLDP_TYPE_SYS_ADMIN_ENABLE;
        }
        else
        {
            status_value = LLDP_TYPE_SYS_ADMIN_DISABLE;
        }

        if (LLDP_PMGR_SetSysAdminStatus(status_value) != LLDP_TYPE_RETURN_OK)
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "lldp.setSysAdminStatusError", "Failled to set status.");
        }
    }

    if (NULL != transmit_interval_p)
    {
        transmit_interval = json_integer_value(transmit_interval_p);
        if (LLDP_PMGR_SetMsgTxInterval(transmit_interval) != LLDP_TYPE_RETURN_OK)
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "lldp.setMsgTxInterval",
                "Failled to set transmitInterval. It must be greater than or equal to 4 * delay interval.");
        }
    }

    if (NULL != delay_interval_p)
    {
        delay_interval = json_integer_value(delay_interval_p);
        if (LLDP_PMGR_SetTxDelay(delay_interval) != LLDP_TYPE_RETURN_OK)
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "lldp.setMsgTxDelay",
                "Failled to set delayInterval. (4*delayInterval) must be less than or equal to  transmitInterval.");
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to get LLDP interface info.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_LLDP_Interfaces_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
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
        if (TRUE == CGI_MODULE_LLDP_GetOne(unit, pidx, interface_obj_p))
        {
            json_array_append_new(interfaces_p, interface_obj_p);
        }
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to get LLDP interface info by ID.
 *
 * @param id (required, number) Interface ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_LLDP_Interfaces_ID_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    const char    *id_str_p;
    UI32_T  unit = 0, port = 0;
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
        ret = CGI_MODULE_LLDP_GetOne(unit, port, result_p);
    }
    else if (memcmp(buffer_ar, "vlan", 4) == 0)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Not support VLAN ID.");
    }
    else if (memcmp(buffer_ar, "trunk", 5) == 0)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Not support Trunk ID.");
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Error ID.");
    }

    if (FALSE == ret)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "Failled to get lldp interface entry.");
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

void CGI_MODULE_LLDP_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler   = CGI_MODULE_LLDP_Read;
    handlers.update_handler = CGI_MODULE_LLDP_Update;
    CGI_MAIN_Register("/api/v1/lldp", &handlers, 0);
}

void CGI_MODULE_LLDP_Interfaces_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler   = CGI_MODULE_LLDP_Interfaces_Read;
    CGI_MAIN_Register("/api/v1/lldp/interfaces", &handlers, 0);

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler   = CGI_MODULE_LLDP_Interfaces_ID_Read;
        CGI_MAIN_Register("/api/v1/lldp/interfaces/{id}", &handlers, 0);
    }
}

static void CGI_MODULE_LLDP_Init()
{
    CGI_MODULE_LLDP_RegisterHandlers();
    CGI_MODULE_LLDP_Interfaces_RegisterHandlers();
}

/*
static BOOL_T CGI_MODULE_LLDP_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request)
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

static BOOL_T CGI_MODULE_LLDP_GetOne(UI32_T unit, UI32_T port, json_t *interface_obj_p)
{
    char *ch_type_str[] = {
            [0] = "Reserved",
            [1] = "Chassis Component",
            [2] = "Interface Alias",
            [3] = "Port Component",
            [4] = "MAC Address",
            [5] = "Network Address",
            [6] = "Interface Name",
            [7] = "Locally Assigned"};
    char *port_type_str[] = {
            [0]                                          = "Reserved",
            [LLDP_TYPE_PORT_ID_SUBTYPE_IFALIAS]          = "Interface Alias",
            [LLDP_TYPE_PORT_ID_SUBTYPE_PORT]             = "Port Component",
            [LLDP_TYPE_PORT_ID_SUBTYPE_MAC_ADDR]         = "MAC Address",
            [LLDP_TYPE_PORT_ID_SUBTYPE_NETWORK_ADDR]     = "Network Address",
            [LLDP_TYPE_PORT_ID_SUBTYPE_IFNAME]           = "Interface Name",
            [LLDP_TYPE_PORT_ID_SUBTYPE_AGENT_CIRCUIT_ID] = "Agent Circuit ID",
            [LLDP_TYPE_PORT_ID_SUBTYPE_LOCAL]            = "Locally Assigned"};

    char    *ch_type_str_p = ch_type_str[0];
    char    *port_type_str_p = port_type_str[0];
    UI32_T  lport = 0, count = 0;
    BOOL_T  ret = FALSE;
    char    id_ar[CGI_MODULE_LLDP_ID_LEN] = {0};
    char    rem_chassis_id_ar[LLDP_TYPE_MAX_CHASSIS_ID_LENGTH] = {0};
    char    rem_port_id_ar[LLDP_TYPE_MAX_PORT_ID_LENGTH] = {0};
    LLDP_MGR_RemoteSystemData_T system_entry;
    Port_Info_T port_info;

    if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UserPortToIfindex(unit, port, &lport))
    {
        return FALSE;
    }

    memset(id_ar, 0, CGI_MODULE_INTERFACE_ID_LEN);
    memset(&system_entry, 0, sizeof(LLDP_MGR_RemoteSystemData_T));
    memset(rem_chassis_id_ar, 0, sizeof(rem_chassis_id_ar));
    memset(rem_port_id_ar, 0, sizeof(rem_port_id_ar));
    memset(&port_info, 0, sizeof(Port_Info_T));

    if (    (TRUE == SWCTRL_POM_GetPortInfo(lport, &port_info))
        &&  (port_info.link_status == SWCTRL_LINK_UP)
       )
    {
        system_entry.rem_time_mark = 0;
        system_entry.rem_local_port_num = lport;
        system_entry.rem_index = 0;

        while (LLDP_PMGR_GetNextRemoteSystemDataByPort(&system_entry) == LLDP_TYPE_RETURN_OK)
        {
            memset(id_ar, 0, CGI_MODULE_INTERFACE_ID_LEN);
            memset(rem_chassis_id_ar, 0, sizeof(rem_chassis_id_ar));
            memset(rem_port_id_ar, 0, sizeof(rem_port_id_ar));
            count++;
            sprintf(id_ar, "eth%lu/%lu",(unsigned long)unit, (unsigned long)port);

            /* Chassis ID Type
             */
            if (system_entry.rem_chassis_id_subtype <= 7)
                ch_type_str_p = ch_type_str[system_entry.rem_chassis_id_subtype];

            /* Chassis ID
             */
            if (system_entry.rem_chassis_id_subtype == VAL_lldpRemChassisIdSubtype_interfaceAlias ||
                system_entry.rem_chassis_id_subtype == VAL_lldpRemChassisIdSubtype_interfaceName)
            {
                memcpy(rem_chassis_id_ar, system_entry.rem_chassis_id, system_entry.rem_chassis_id_len);
            }
            else
            {
                CGI_MODULE_LLDP_MAC_FORMAT(rem_chassis_id_ar, system_entry.rem_chassis_id, system_entry.rem_chassis_id_len);
            }

            /* Port ID Type
             */
            if (system_entry.rem_port_id_subtype <= LLDP_TYPE_PORT_ID_SUBTYPE_LOCAL)
                port_type_str_p = port_type_str[system_entry.rem_port_id_subtype];

            /* Port ID
             */
            if (    (system_entry.rem_port_id_subtype == LLDP_TYPE_PORT_ID_SUBTYPE_IFALIAS)
                 || (system_entry.rem_port_id_subtype == LLDP_TYPE_PORT_ID_SUBTYPE_IFNAME)
                 || (system_entry.rem_port_id_subtype == LLDP_TYPE_PORT_ID_SUBTYPE_LOCAL)
               )
            {
                memcpy(rem_port_id_ar, system_entry.rem_port_id, system_entry.rem_port_id_len);
            }
            else
            {
                CGI_MODULE_LLDP_MAC_FORMAT(rem_port_id_ar, system_entry.rem_port_id, system_entry.rem_port_id_len);
            }

            json_object_set_new(interface_obj_p, "id", json_string(id_ar));
            json_object_set_new(interface_obj_p, "localPort", json_integer(lport));
            json_object_set_new(interface_obj_p, "remChassisIdSubtype", json_string(ch_type_str_p));
            json_object_set_new(interface_obj_p, "remChassisId", json_string(rem_chassis_id_ar));
            json_object_set_new(interface_obj_p, "remPortIdSubType", json_string(port_type_str_p));
            json_object_set_new(interface_obj_p, "remPortId", json_string(rem_port_id_ar));
            json_object_set_new(interface_obj_p, "remTtl", json_integer(system_entry.rem_ttl));
            json_object_set_new(interface_obj_p, "remPortDesc", json_string(system_entry.rem_port_desc));
            json_object_set_new(interface_obj_p, "remSysName", json_string(system_entry.rem_sys_name));
            json_object_set_new(interface_obj_p, "remSysDesc", json_string(system_entry.rem_sys_desc));

            {
                json_t  *capability = json_array();
                json_object_set_new(interface_obj_p, "remSysCapabilityName", capability);
                json_array_append_new(capability, json_string("Other"));
                json_array_append_new(capability, json_string("Repeater"));
                json_array_append_new(capability, json_string("Bridge"));
                json_array_append_new(capability, json_string("WLAN Access Point"));
                json_array_append_new(capability, json_string("Router"));
                json_array_append_new(capability, json_string("Telephone"));
                json_array_append_new(capability, json_string("DOCSIS Cable Device"));
                json_array_append_new(capability, json_string("Station Only"));
            }

            json_object_set_new(interface_obj_p, "remSysCapabilitySupport", json_integer(system_entry.rem_sys_cap_supported));
            json_object_set_new(interface_obj_p, "remSysCapabilityEnable", json_integer(system_entry.rem_sys_cap_enabled));
            ret = TRUE;
        }
    }

    return ret;
}