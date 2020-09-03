#include "cgi_auth.h"
#include "xstp_type.h"
#include "xstp_pmgr.h"
#include "xstp_pom.h"

//static BOOL_T CGI_MODULE_STP_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request);
static CGI_STATUS_CODE_T CGI_MODULE_STP_UpdatePortBpduGuard(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *bpdu_guard_p);
static CGI_STATUS_CODE_T CGI_MODULE_STP_UpdatePortBpduFilter(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *bpdu_filter_p);
static CGI_STATUS_CODE_T CGI_MODULE_STP_UpdatePortRootGuard(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *root_guard_p);
static CGI_STATUS_CODE_T CGI_MODULE_STP_UpdatePortPriority(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *priority_p);
static BOOL_T CGI_MODULE_STP_GetPortInfo(UI32_T mstid, UI32_T lport, json_t *info_obj_p);

/**----------------------------------------------------------------------
 * This API is used to get STP info.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_STP_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    UI32_T stp_status = 0, mode = 0, mstid = 0;
    XSTP_MGR_BridgeIdComponent_T designated_root;
    XSTP_MGR_Dot1dStpEntry_T entry;
    char designated_root_ar[32] = {0};
    char port_ar[CGI_MODULE_INTERFACE_ID_LEN] = {0};

    memset(&entry, 0, sizeof(entry));

    if (SYS_TYPE_GET_RUNNING_CFG_FAIL != XSTP_POM_GetRunningSystemSpanningTreeStatus(&stp_status))
    {
        json_object_set_new(result_p, "status", json_boolean((stp_status == VAL_ifAdminStatus_up)));
    }
    else
    {
        json_object_set_new(result_p, "status", json_boolean(FALSE));
    }

    XSTP_POM_GetRunningSystemSpanningTreeVersion(&mode);
    XSTP_POM_GetDesignatedRoot(mstid, &designated_root);
    XSTP_PMGR_GetDot1dMstEntry(mstid, &entry);

    switch(mode)
    {
        case XSTP_TYPE_STP_PROTOCOL_VERSION_ID:
            json_object_set_new(result_p, "mode", json_string("STP"));
            break;

        case XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID:
            json_object_set_new(result_p, "mode", json_string("RSTP"));
            break;

        case XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID:
            json_object_set_new(result_p, "mode", json_string("MSTP"));
            break;

        default:
            json_object_set_new(result_p, "mode", json_string("None"));
            break;
    }

#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    if (XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID == mode)
    {
        sprintf(designated_root_ar,"%d.%d.%02X%02X%02X%02X%02X%02X",
                designated_root.priority, designated_root.system_id_ext,
                designated_root.addr[0], designated_root.addr[1], designated_root.addr[2],
                designated_root.addr[3], designated_root.addr[4], designated_root.addr[5]);
    }
    else
#endif
    {
        sprintf(designated_root_ar,"%d.%02X%02X%02X%02X%02X%02X",
                designated_root.priority,
                designated_root.addr[0], designated_root.addr[1], designated_root.addr[2],
                designated_root.addr[3], designated_root.addr[4], designated_root.addr[5]);
    }

    json_object_set_new(result_p, "designatedRoot", json_string(designated_root_ar));

    if (TRUE == CGI_UTIL_IfindexToRestPortStr(entry.dot1d_stp_root_port, port_ar))
    {
        json_object_set_new(result_p, "currentRootPort", json_string(port_ar));
    }
    else
    {
        json_object_set_new(result_p, "currentRootPort", json_string("0"));
    }

    json_object_set_new(result_p, "currentRootCost", json_integer(entry.dot1d_stp_root_cost));
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to update STP status.
 *
 * @param status   (optional, boolean) STP status.
 * @param priority (optional, number) STP priority.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_STP_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *status_p;
    json_t  *priority_p;
    UI32_T  status_value;
    UI32_T  port_status;
    UI32_T  priority = XSTP_TYPE_DEFAULT_BRIDGE_PRIORITY;
    UI32_T  lport = 0;

    status_p = CGI_REQUEST_GetBodyValue(http_request, "status");

    if (NULL != status_p)
    {
        if (TRUE != json_is_boolean(status_p))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
        }

        if (TRUE == json_boolean_value(status_p))
        {
            status_value = VAL_staSystemStatus_enabled;
            port_status = VAL_staPortSystemStatus_enabled;
        }
        else
        {
            status_value = VAL_staSystemStatus_disabled;
            port_status = VAL_staPortSystemStatus_disabled;
        }

        if (XSTP_PMGR_SetSystemSpanningTreeStatus(status_value) != XSTP_TYPE_RETURN_OK)
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "xstp.setSystemSpanningTreeStatusError", "Failed to set spanning tree status.");
        }

        for (lport = 1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT ; lport++)
        {
            XSTP_PMGR_SetPortSpanningTreeStatus(lport, port_status);
        }
    } //if (NULL != status_p)

    priority_p = CGI_REQUEST_GetBodyValue(http_request, "priority");

    if (NULL != priority_p)
    {
        priority = json_integer_value(priority_p);

        if (XSTP_PMGR_SetSystemBridgePriority(priority) != XSTP_TYPE_RETURN_OK)
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "xstp.setSystemBridgePriorityError", "Failed to set spanning tree priority.");
        }
    } //if (NULL != priority_p)

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to update attribute of specified ports.
 *
 * @param ports      (required, array) Port array
 * @param bpduGuard  (optional, boolean) STP BPDU guard status
 * @param bpduFilter (optional, boolean) STP BPDU filter status
 * @param rootGuard  (optional, boolean) STP root guard status
 * @param priority   (optional, number) Spanning-tree port priority value in steps of 16
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_STP_INTF_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *ports_p;
    json_t  *port_p;
    json_t  *bpdu_guard_p;
    json_t  *bpdu_filter_p;
    json_t  *root_guard_p;
    json_t  *priority_p;
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

            if (TRUE != CGI_UTIL_InterfaceIdToLport(port_str_p, &lport))
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid ifId.");
            }
        }
    }

    bpdu_guard_p = CGI_REQUEST_GetBodyValue(http_request, "bpduGuard");

    if (NULL != bpdu_guard_p)
    {
        return CGI_MODULE_STP_UpdatePortBpduGuard(http_response, ports_p, port_num, bpdu_guard_p);
    }

    bpdu_filter_p = CGI_REQUEST_GetBodyValue(http_request, "bpduFilter");

    if (NULL != bpdu_filter_p)
    {
        return CGI_MODULE_STP_UpdatePortBpduFilter(http_response, ports_p, port_num, bpdu_filter_p);
    }

    root_guard_p = CGI_REQUEST_GetBodyValue(http_request, "rootGuard");

    if (NULL != root_guard_p)
    {
        return CGI_MODULE_STP_UpdatePortRootGuard(http_response, ports_p, port_num, root_guard_p);
    }

    priority_p = CGI_REQUEST_GetBodyValue(http_request, "priority");

    if (NULL != priority_p)
    {
        return CGI_MODULE_STP_UpdatePortPriority(http_response, ports_p, port_num, priority_p);
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_VOICE_VLAN_INTF_Update

/**----------------------------------------------------------------------
 * This API is used to get interface info.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_STP_INTF_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    UI32_T  pmax = 0, unit = 0, pidx = 0, lport = 0;
    UI32_T  mstid = 0;
    json_t  *ports_p;

    STKTPLG_POM_GetNextUnit(&unit);
    pmax = SWCTRL_POM_UIGetUnitPortNumber(unit);

    if (pmax == 0)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "No port exists.");
    }

    /* Get all interfaces.
     */
    ports_p = json_array();
    json_object_set_new(result_p, "ports", ports_p);

    for (pidx =1; pidx <= pmax; pidx++)
    {
        json_t *port_obj_p = json_object();
        SWCTRL_POM_UserPortToIfindex(unit, pidx, &lport);

        if (TRUE == CGI_MODULE_STP_GetPortInfo(mstid, lport, port_obj_p))
        {
            json_array_append_new(ports_p, port_obj_p);
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to get an interface info by ID.
 *
 * @param id (required, string) Interface ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_STP_INTF_ID_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    const char *id_str_p;
    UI32_T  lport = 0;
    UI32_T  mstid = 0;
    char    buffer_ar[20] = {0};

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (NULL == id_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Interface ID is NULL.");
    }

    id_str_p = json_string_value(id_p);

    CGI_UTIL_UrlDecode(buffer_ar, id_str_p);

    if (TRUE != CGI_UTIL_InterfaceIdToLport((const char *) buffer_ar, &lport))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Error ID.");
    }

    if (TRUE != CGI_MODULE_STP_GetPortInfo(mstid, lport, result_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Get port info failed.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

static CGI_STATUS_CODE_T CGI_MODULE_STP_UpdatePortBpduGuard(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *bpdu_guard_p)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *port_p;
    const char *port_str_p;
    UI32_T status = XSTP_TYPE_PORT_BPDU_GUARD_DISABLED;
    UI32_T lport = 0;
    int idx = 0;

    if (TRUE != json_is_boolean(bpdu_guard_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
    }

    if (TRUE == json_boolean_value(bpdu_guard_p))
    {
        status = XSTP_TYPE_PORT_BPDU_GUARD_ENABLED;
    }

    for (idx = 0; idx < port_num; idx ++)
    {
        port_p = json_array_get(ports_p, idx);
        port_str_p = json_string_value(port_p);
        CGI_UTIL_InterfaceIdToLport(port_str_p, &lport);

        if (XSTP_TYPE_RETURN_OK != XSTP_PMGR_SetPortBpduGuardStatus(lport, status))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "stp.setPortBpduGuardStatusError", "Failed to set port BPDU guard status.");
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_STP_UpdatePortBpduGuard

static CGI_STATUS_CODE_T CGI_MODULE_STP_UpdatePortBpduFilter(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *bpdu_filter_p)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *port_p;
    const char *port_str_p;
    UI32_T status = XSTP_TYPE_PORT_BPDU_FILTER_DISABLED;
    UI32_T lport = 0;
    int idx = 0;

    if (TRUE != json_is_boolean(bpdu_filter_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
    }

    if (TRUE == json_boolean_value(bpdu_filter_p))
    {
        status = XSTP_TYPE_PORT_BPDU_FILTER_ENABLED;
    }

    for (idx = 0; idx < port_num; idx ++)
    {
        port_p = json_array_get(ports_p, idx);
        port_str_p = json_string_value(port_p);
        CGI_UTIL_InterfaceIdToLport(port_str_p, &lport);

        if (XSTP_TYPE_RETURN_OK != XSTP_PMGR_SetPortBpduFilterStatus(lport, status))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "stp.setPortBpduFilterStatusError", "Failed to set port BPDU filter status.");
        }
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_STP_UpdatePortBpduFilter

static CGI_STATUS_CODE_T CGI_MODULE_STP_UpdatePortRootGuard(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *root_guard_p)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *port_p;
    const char *port_str_p;
    UI32_T status = VAL_staPortRootGuardAdminStatus_disabled;
    UI32_T lport = 0;
    int idx = 0;

    if (TRUE != json_is_boolean(root_guard_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
    }

    if (TRUE == json_boolean_value(root_guard_p))
    {
        status = VAL_staPortRootGuardAdminStatus_enabled;
    }

    for (idx = 0; idx < port_num; idx ++)
    {
        port_p = json_array_get(ports_p, idx);
        port_str_p = json_string_value(port_p);
        CGI_UTIL_InterfaceIdToLport(port_str_p, &lport);

        if (XSTP_TYPE_RETURN_OK != XSTP_PMGR_SetPortRootGuardStatus(lport, status))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "stp.setPortRootGuardStatusError", "Failed to set port root guard status.");
        }
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_STP_UpdatePortBpduGuard

static CGI_STATUS_CODE_T CGI_MODULE_STP_UpdatePortPriority(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *priority_p)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *port_p;
    const char *port_str_p;
    UI32_T priority = json_integer_value(priority_p);
    UI32_T lport = 0;
    int idx = 0;

    for (idx = 0; idx < port_num; idx ++)
    {
        port_p = json_array_get(ports_p, idx);
        port_str_p = json_string_value(port_p);
        CGI_UTIL_InterfaceIdToLport(port_str_p, &lport);

        if (XSTP_TYPE_RETURN_OK != XSTP_PMGR_SetPortPriority(lport, priority))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "stp.setPortPriorityError", "Failed to set port priority.");
        }
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_STP_UpdatePortBpduGuard

static BOOL_T CGI_MODULE_STP_GetPortInfo(UI32_T mstid, UI32_T lport, json_t *info_obj_p)
{
    enum SHOW_STATUS
    {
        SHOW_STATUS_DISABLE = 0,
        SHOW_STATUS_ENABLE,
        SHOW_STATUS_BLOCKING,
        SHOW_STATUS_LISTENING,
        SHOW_STATUS_LEARNING,
        SHOW_STATUS_FORWARDING,
        SHOW_STATUS_BROKEN,
        SHOW_MST_PORT_POINT,
        SHOW_MST_PORT_SHARED,
        SHOW_MST_PORT_AUTO,
        SHOW_MST_ROLE_ROOT,
        SHOW_MST_ROLE_DESIGNATE,
        SHOW_MST_ROLE_ALTERNATE,
        SHOW_MST_ROLE_BACKUP,
        SHOW_MST_ROLE_MASTER,
        SHOW_STATUS_DISCARDING
    };

    XSTP_MGR_BridgeIdComponent_T designated_bridge;
    XSTP_MGR_Dot1dStpPortEntry_T stp_port_entry;
    char id_ar[CGI_MODULE_INTERFACE_ID_LEN] = {0};
    char designated_bridge_ar[32] = {0};
    char *str[] = {"Disabled",/*0*/  //to remove warning, change type from UI8_T* to char *
                   "Enabled",
                   "Blocking",
                   "Listening",
                   "Learning",
                   "Forwarding",
                   "Broken",
                   "Point-to-point",
                   "Shared",
                   "Auto",
                   "Root",
                   "Designate",
                   "Alternate",
                   "Backup",
                   "Master",
                   "Discarding"
                  };
    UI32_T role = 0, state = 0, mode = 0;

    memset(&designated_bridge, 0, sizeof(designated_bridge));
    memset(&stp_port_entry, 0, sizeof(stp_port_entry));

    if (XSTP_POM_GetMstPortRole(lport, mstid, &role) != XSTP_TYPE_RETURN_OK)
    {
        return FALSE;
    }

    if (XSTP_POM_GetMstPortState(lport, mstid, &state) != XSTP_TYPE_RETURN_OK)
    {
        return FALSE;
    }

    if (!XSTP_POM_GetPortDesignatedBridge(lport, mstid, &designated_bridge))
    {
        return FALSE;
    }

    stp_port_entry.dot1d_stp_port = (UI16_T)lport;

    if (!XSTP_POM_GetDot1dMstPortEntry(mstid, &stp_port_entry))
    {
        return FALSE;
    }

    switch(role)
    {
        case XSTP_TYPE_PORT_ROLE_DISABLED:
            role = SHOW_STATUS_DISABLE;
            break;

        case XSTP_TYPE_PORT_ROLE_ROOT:
            role = SHOW_MST_ROLE_ROOT;
            break;

        case XSTP_TYPE_PORT_ROLE_DESIGNATED:
            role = SHOW_MST_ROLE_DESIGNATE;
            break;

        case XSTP_TYPE_PORT_ROLE_ALTERNATE:
            role = SHOW_MST_ROLE_ALTERNATE;
            break;

        case XSTP_TYPE_PORT_ROLE_BACKUP:
            role = SHOW_MST_ROLE_BACKUP;
            break;

#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
        case XSTP_TYPE_PORT_ROLE_MASTER:
            role = SHOW_MST_ROLE_MASTER;
            break;
#endif
        default:
            role = SHOW_STATUS_DISABLE;
            break;
    }

#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP || SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    switch(state)
    {
        case XSTP_TYPE_PORT_STATE_LISTENING:
            state = SHOW_STATUS_LISTENING;
            break;

        case XSTP_TYPE_PORT_STATE_LEARNING:
            state = SHOW_STATUS_LEARNING;
            break;

        case XSTP_TYPE_PORT_STATE_DISCARDING:
            state = SHOW_STATUS_DISCARDING;
            break;

        default:
            break;
    }

#else

    switch(state)
    {
        case VAL_dot1dStpPortState_disabled:
            state = SHOW_STATUS_DISABLE;
            break;

        case VAL_dot1dStpPortState_blocking:
            state = SHOW_STATUS_BLOCKING;
            break;

        case VAL_dot1dStpPortState_listening:
            state = SHOW_STATUS_LISTENING;
            break;

        case VAL_dot1dStpPortState_learning:
            state = SHOW_STATUS_LEARNING;
            break;

        case VAL_dot1dStpPortState_forwarding:
            state = SHOW_STATUS_FORWARDING;
            break;

        case VAL_dot1dStpPortState_broken:
            state = SHOW_STATUS_BROKEN;
            break;

        default:
            break;
    }
#endif

#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_POM_GetRunningSystemSpanningTreeVersion(&mode);

    if (XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID == mode)
    {
        sprintf(designated_bridge_ar,"%d.%d.%02X%02X%02X%02X%02X%02X",
                designated_bridge.priority, designated_bridge.system_id_ext,
                designated_bridge.addr[0], designated_bridge.addr[1], designated_bridge.addr[2],
                designated_bridge.addr[3], designated_bridge.addr[4], designated_bridge.addr[5]);
    }
    else
#endif
    {
        sprintf(designated_bridge_ar,"%d.%02X%02X%02X%02X%02X%02X",
                designated_bridge.priority,
                designated_bridge.addr[0], designated_bridge.addr[1], designated_bridge.addr[2],
                designated_bridge.addr[3], designated_bridge.addr[4], designated_bridge.addr[5]);
    }

    CGI_UTIL_IfindexToRestPortStr(lport, id_ar);
    json_object_set_new(info_obj_p, "id", json_string(id_ar));
    json_object_set_new(info_obj_p, "role", json_string(str[role]));
    json_object_set_new(info_obj_p, "state", json_string(str[state]));
    json_object_set_new(info_obj_p, "designatedBridge", json_string(designated_bridge_ar));
    json_object_set_new(info_obj_p, "forwardTrans", json_integer(stp_port_entry.dot1d_stp_port_forward_transitions));


#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
    {
        UI32_T status = 0;
        XSTP_POM_GetPortRootGuardStatus(lport, &status);
        json_object_set_new(info_obj_p, "rootGuard", json_boolean((status == VAL_staPortRootGuardAdminStatus_enabled)));
    }
#endif

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    {
        UI32_T status = 0;
        XSTP_POM_GetPortBpduGuardStatus(lport, &status);
        json_object_set_new(info_obj_p, "bpduGuard", json_boolean((status == XSTP_TYPE_PORT_BPDU_GUARD_ENABLED)));
    }
#endif

#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
    {
        UI32_T status = 0;
        XSTP_POM_GetPortBpduFilterStatus(lport, &status);
        json_object_set_new(info_obj_p, "bpduFilter", json_boolean((status == XSTP_TYPE_PORT_BPDU_FILTER_ENABLED)));
    }
#endif

    return TRUE;
}

void CGI_MODULE_STP_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler   = CGI_MODULE_STP_Read;
    handlers.update_handler = CGI_MODULE_STP_Update;

    CGI_MAIN_Register("/api/v1/stp", &handlers, 0);

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler   = CGI_MODULE_STP_INTF_Read;
        handlers.update_handler = CGI_MODULE_STP_INTF_Update;
        CGI_MAIN_Register("/api/v1/stp/interfaces", &handlers, 0);
    }

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_STP_INTF_ID_Read;
        CGI_MAIN_Register("/api/v1/stp/interfaces/{id}", &handlers, 0);
    }
}


static void CGI_MODULE_STP_Init()
{
    CGI_MODULE_STP_RegisterHandlers();
}

/*
static BOOL_T CGI_MODULE_STP_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request)
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
