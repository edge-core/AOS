#include "cgi_auth.h"

#if (SYS_CPNT_PPPOE_IA == TRUE)
#include "pppoe_ia_pmgr.h"
#endif

#if (SYS_CPNT_PPPOE_IA == TRUE)
static CGI_STATUS_CODE_T CGI_MODULE_PPPOE_IA_UpdatePortStatus(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *status_p);
static CGI_STATUS_CODE_T CGI_MODULE_PPPOE_IA_UpdatePortVendorStrip(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *strip_p);
static CGI_STATUS_CODE_T CGI_MODULE_PPPOE_IA_UpdatePortTrustStatus(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *trust_p);
static BOOL_T CGI_MODULE_PPPOE_IA_GetPortStats(UI32_T lport, json_t *stats_obj_p);
#endif  /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

/**----------------------------------------------------------------------
 * This API is used to update PPPoE IA status.
 *
 * @param status   (optional, boolean) PPPoE IA status.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PPPOE_IA_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

#if (SYS_CPNT_PPPOE_IA == TRUE)
    json_t  *status_p;
    BOOL_T  is_enable = FALSE;

    status_p = CGI_REQUEST_GetBodyValue(http_request, "status");

    if (NULL == status_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is NULL.");
    }

    if (TRUE != json_is_boolean(status_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
    }

    if (TRUE == json_boolean_value(status_p))
    {
        is_enable = TRUE;
    }

    if (TRUE != PPPOE_IA_PMGR_SetGlobalEnable(is_enable))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "pppoeia.setGlobalEnableError", "Failed to set PPPoE IA status.");
    }
#endif  /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_PPPOE_IA_Update

/**----------------------------------------------------------------------
 * This API is used to update attribute of specified ports.
 *
 * @param ports       (required, array) Port array
 * @param status      (optional, boolean) PPPoE intermediate-agent port status
 * @param vendorStrip (optional, boolean) Strip vendor tag status
 * @param trust       (optional, boolean) PPPoE intermediate-agent trust status
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PPPOE_IA_INTF_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

#if (SYS_CPNT_PPPOE_IA == TRUE)
    json_t  *ports_p;
    json_t  *port_p;
    json_t  *status_p;
    json_t  *strip_p;
    json_t  *trust_p;
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

    status_p = CGI_REQUEST_GetBodyValue(http_request, "status");

    if (NULL != status_p)
    {
        return CGI_MODULE_PPPOE_IA_UpdatePortStatus(http_response, ports_p, port_num, status_p);
    }

    strip_p = CGI_REQUEST_GetBodyValue(http_request, "vendorStrip");

    if (NULL != strip_p)
    {
        return CGI_MODULE_PPPOE_IA_UpdatePortVendorStrip(http_response, ports_p, port_num, strip_p);
    }

    trust_p = CGI_REQUEST_GetBodyValue(http_request, "trust");

    if (NULL != trust_p)
    {
        return CGI_MODULE_PPPOE_IA_UpdatePortTrustStatus(http_response, ports_p, port_num, trust_p);
    }
#endif  /* #if (SYS_CPNT_PPPOE_IA == TRUE) */
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_PPPOE_IA_INTF_Update

/**----------------------------------------------------------------------
 * This API is used to put an interface info by ID.
 *
 * @param id        (required, string) Interface ID
 * @param circuitId (optional, string) null to set default
 * @param remoteId  (optional, string) null to set default
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PPPOE_IA_INTF_ID_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    json_t  *circuit_p;
    json_t  *remote_p;
    const char  *id_str_p;
    const char  *circuit_str_p;
    const char  *remote_str_p;
    UI32_T  lport = 0;
    char    buffer_ar[20] = {0};
    BOOL_T  ret = FALSE;

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (NULL == id_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Interface ID is NULL.");
    }

    id_str_p = json_string_value(id_p);
    circuit_p = CGI_REQUEST_GetBodyValue(http_request, "circuitId");
    remote_p = CGI_REQUEST_GetBodyValue(http_request, "remoteId");

    if ((NULL == circuit_p) && (NULL == remote_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Please specify circuitId, or remoteId.");
    }

    CGI_UTIL_UrlDecode(buffer_ar, id_str_p);

    if (TRUE != CGI_UTIL_InterfaceIdToLport((const char *) buffer_ar, &lport))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid ID.");
    }

    if (NULL != circuit_p)
    {
        circuit_str_p = json_string_value(circuit_p);

        if (0 == strlen(circuit_str_p))
        {
            ret = PPPOE_IA_PMGR_SetPortAdmStrDataByField(lport, PPPOE_IA_TYPE_FLDID_PORT_CIRCUIT_ID, NULL, 0);
        }
        else
        {
            ret = PPPOE_IA_PMGR_SetPortAdmStrDataByField(lport,
                    PPPOE_IA_TYPE_FLDID_PORT_CIRCUIT_ID, (UI8_T *) circuit_str_p, strlen(circuit_str_p));
        }

        if (TRUE != ret)
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "pppoeia.setPortCircuitIdError", "Failed to port cirtuit ID.");
        }
    }

    if (NULL != remote_p)
    {
        remote_str_p = json_string_value(remote_p);

        if (0 == strlen(remote_str_p))
        {
            ret = PPPOE_IA_PMGR_SetPortAdmStrDataByField(lport, PPPOE_IA_TYPE_FLDID_PORT_REMOTE_ID, NULL, 0);
        }
        else
        {
            ret = PPPOE_IA_PMGR_SetPortAdmStrDataByField(lport,
                    PPPOE_IA_TYPE_FLDID_PORT_REMOTE_ID, (UI8_T *) remote_str_p, strlen(remote_str_p));
        }

        if (TRUE != ret)
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "pppoeia.setPortRemoteIdError", "Failed to port remote ID.");
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_PPPOE_IA_INTF_ID_Update

/**----------------------------------------------------------------------
 * This API is used to get interface info.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PPPOE_IA_INTF_STATS_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    UI32_T  lport = 0;
    json_t  *statistics_p;

    /* Get all interfaces.
     */
    statistics_p = json_array();
    json_object_set_new(result_p, "statistics", statistics_p);

    while (SWCTRL_POM_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        json_t *statistics_obj_p = json_object();

        if (TRUE == CGI_MODULE_PPPOE_IA_GetPortStats(lport, statistics_obj_p))
        {
            json_array_append_new(statistics_p, statistics_obj_p);
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to get an interface info by ID.
 *
 * @param id (required, string) Interface ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PPPOE_IA_INTF_STATS_ID_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    const char *id_str_p;
    UI32_T  lport = 0;
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

    if (TRUE != CGI_MODULE_PPPOE_IA_GetPortStats(lport, result_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Get port statistics failed.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to update attribute of all ports to default.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PPPOE_IA_INTF_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

#if (SYS_CPNT_PPPOE_IA == TRUE)
    UI32_T  lport = 0;

    while (SWCTRL_POM_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        PPPOE_IA_PMGR_SetPortBoolDataByField(lport, PPPOE_IA_TYPE_FLDID_PORT_ENABLE, SYS_DFLT_PPPOE_IA_PORT_STATUS);
        PPPOE_IA_PMGR_SetPortBoolDataByField(lport, PPPOE_IA_TYPE_FLDID_PORT_TRUST, SYS_DFLT_PPPOE_IA_PORT_TRUST);
        PPPOE_IA_PMGR_SetPortBoolDataByField(lport, PPPOE_IA_TYPE_FLDID_PORT_STRIP_VENDOR, SYS_DFLT_PPPOE_IA_PORT_VENDOR_STRIP);
        PPPOE_IA_PMGR_SetPortAdmStrDataByField(lport, PPPOE_IA_TYPE_FLDID_PORT_CIRCUIT_ID, NULL, 0);
        PPPOE_IA_PMGR_SetPortAdmStrDataByField(lport, PPPOE_IA_TYPE_FLDID_PORT_REMOTE_ID, NULL, 0);
    }
#endif  /* #if (SYS_CPNT_PPPOE_IA == TRUE) */
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_PPPOE_IA_INTF_Delete

#if (SYS_CPNT_PPPOE_IA == TRUE)
static CGI_STATUS_CODE_T CGI_MODULE_PPPOE_IA_UpdatePortStatus(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *status_p)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *port_p;
    const char *port_str_p;
    BOOL_T status = FALSE;
    UI32_T lport = 0;
    int idx = 0;

    if (TRUE != json_is_boolean(status_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
    }

    if (TRUE == json_boolean_value(status_p))
    {
        status = TRUE;
    }

    for (idx = 0; idx < port_num; idx ++)
    {
        port_p = json_array_get(ports_p, idx);
        port_str_p = json_string_value(port_p);
        CGI_UTIL_InterfaceIdToLport(port_str_p, &lport);

        if (TRUE != PPPOE_IA_PMGR_SetPortBoolDataByField(lport, PPPOE_IA_TYPE_FLDID_PORT_ENABLE, status))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "pppoeia.setPortStatusError", "Failed to set port status.");
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_PPPOE_IA_UpdatePortStatus

static CGI_STATUS_CODE_T CGI_MODULE_PPPOE_IA_UpdatePortVendorStrip(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *strip_p)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *port_p;
    const char *port_str_p;
    BOOL_T status = FALSE;
    UI32_T lport = 0;
    int idx = 0;

    if (TRUE != json_is_boolean(strip_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
    }

    if (TRUE == json_boolean_value(strip_p))
    {
        status = TRUE;
    }

    for (idx = 0; idx < port_num; idx ++)
    {
        port_p = json_array_get(ports_p, idx);
        port_str_p = json_string_value(port_p);
        CGI_UTIL_InterfaceIdToLport(port_str_p, &lport);

        if (TRUE != PPPOE_IA_PMGR_SetPortBoolDataByField(lport, PPPOE_IA_TYPE_FLDID_PORT_STRIP_VENDOR, status))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "pppoeia.setPortStatusError", "Failed to set port vendor strip status.");
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_PPPOE_IA_UpdatePortVendorStrip

static CGI_STATUS_CODE_T CGI_MODULE_PPPOE_IA_UpdatePortTrustStatus(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *trust_p)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *port_p;
    const char *port_str_p;
    BOOL_T status = FALSE;
    UI32_T lport = 0;
    int idx = 0;

    if (TRUE != json_is_boolean(trust_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
    }

    if (TRUE == json_boolean_value(trust_p))
    {
        status = TRUE;
    }

    for (idx = 0; idx < port_num; idx ++)
    {
        port_p = json_array_get(ports_p, idx);
        port_str_p = json_string_value(port_p);
        CGI_UTIL_InterfaceIdToLport(port_str_p, &lport);

        if (TRUE != PPPOE_IA_PMGR_SetPortBoolDataByField(lport, PPPOE_IA_TYPE_FLDID_PORT_TRUST, status))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "pppoeia.setPortStatusError", "Failed to set port trust status.");
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_PPPOE_IA_UpdatePortTrustStatus

static BOOL_T CGI_MODULE_PPPOE_IA_GetPortStats(UI32_T lport, json_t *stats_obj_p)
{
    PPPOE_IA_OM_PortStsEntry_T  psts;
    char id_ar[CGI_MODULE_INTERFACE_ID_LEN] = {0};

    memset(&psts, 0, sizeof(psts));

    if (TRUE != PPPOE_IA_PMGR_GetPortStatisticsEntry(lport, &psts))
    {
        return FALSE;
    }

    CGI_UTIL_IfindexToRestPortStr(lport, id_ar);
    json_object_set_new(stats_obj_p, "id", json_string(id_ar));
    json_object_set_new(stats_obj_p, "rxAll", json_integer(psts.all));
    json_object_set_new(stats_obj_p, "rxPadi", json_integer(psts.padi));
    json_object_set_new(stats_obj_p, "rxPado", json_integer(psts.pado));
    json_object_set_new(stats_obj_p, "rxPadr", json_integer(psts.padr));
    json_object_set_new(stats_obj_p, "rxPads", json_integer(psts.pads));
    json_object_set_new(stats_obj_p, "rxPadt", json_integer(psts.padt));
    json_object_set_new(stats_obj_p, "dropRespUntrust", json_integer(psts.rep_untrust));
    json_object_set_new(stats_obj_p, "dropReqUntrust", json_integer(psts.req_untrust));
    json_object_set_new(stats_obj_p, "dropMalformed", json_integer(psts.malform));

    return TRUE;
}

#endif  /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

void CGI_MODULE_PPPOE_IA_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.update_handler = CGI_MODULE_PPPOE_IA_Update;
    CGI_MAIN_Register("/api/v1/pppoeia", &handlers, 0);

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.update_handler = CGI_MODULE_PPPOE_IA_INTF_Update;
        handlers.delete_handler = CGI_MODULE_PPPOE_IA_INTF_Delete;
        CGI_MAIN_Register("/api/v1/pppoeia/interfaces", &handlers, 0);
    }

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.update_handler = CGI_MODULE_PPPOE_IA_INTF_ID_Update;
        CGI_MAIN_Register("/api/v1/pppoeia/interfaces/{id}", &handlers, 0);
    }

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_PPPOE_IA_INTF_STATS_Read;
        CGI_MAIN_Register("/api/v1/pppoeia/interfaces/statistics", &handlers, 0);
    }

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_PPPOE_IA_INTF_STATS_ID_Read;
        CGI_MAIN_Register("/api/v1/pppoeia/interfaces/statistics/{id}", &handlers, 0);
    }
}


static void CGI_MODULE_PPPOE_IA_Init()
{
    CGI_MODULE_PPPOE_IA_RegisterHandlers();
}

