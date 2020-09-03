#include "cgi_request.h"
#include "cgi_auth.h"
#include "cgi_util.h"
#include "swctrl_pmgr.h"

/**----------------------------------------------------------------------
 * This API is used to set port monitor.
 *
 * @param src (required, string) Source port interface (monitored port)
 * @param dst (required, string) Destination port interface (listen port)
 * @param mode (required, string) Monitor mode {both | rx | tx}
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_MONITOR_Port_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *src_p;
    json_t  *dst_p;
    json_t  *mode_p;
    const char*  src_str_p;
    const char*  dst_str_p;
    const char*  mode_str_p;
    UI32_T src_lport = 0;
    UI32_T dst_lport = 0;
    UI32_T mode = 0;

    src_p = CGI_REQUEST_GetBodyValue(http_request, "src");

    if (src_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get src.");
    }
    src_str_p = json_string_value(src_p);

    if (TRUE != CGI_UTIL_InterfaceIdToLport((char *)src_str_p, &src_lport))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid source port.");
    }

    dst_p = CGI_REQUEST_GetBodyValue(http_request, "dst");

    if (dst_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get dst.");
    }
    dst_str_p = json_string_value(dst_p);

    if (TRUE != CGI_UTIL_InterfaceIdToLport((char *)dst_str_p, &dst_lport))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid destination port.");
    }

    mode_p = CGI_REQUEST_GetBodyValue(http_request, "mode");

    if (mode_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get mode.");
    }
    mode_str_p = json_string_value(mode_p);

    if (0 == strncmp(mode_str_p, "tx", strlen("tx")))
    {
        mode = VAL_mirrorType_tx;
    }
    else if (0 == strncmp(mode_str_p, "rx", strlen("rx")))
    {
        mode = VAL_mirrorType_rx;
    }
    else if (0 == strncmp(mode_str_p, "both", strlen("both")))
    {
        mode = VAL_mirrorType_both;
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid mode.");
    }

    if (TRUE != SWCTRL_PMGR_SetMirrorStatus(src_lport, dst_lport, VAL_mirrorStatus_valid))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "portMonitor.SetMirrorStatusError", "Failed.");
    }

    if (TRUE != SWCTRL_PMGR_SetMirrorType(src_lport, dst_lport, mode))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "portMonitor.SetMirrorTypeError", "Failed.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to read sFlow setting.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_MONITOR_Port_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *ports_p = json_array();
    SWCTRL_MirrorEntry_T mirroring_entry;
    SWCTRL_Lport_Type_T lport_type;
    UI32_T unit = 0, port = 0, trunk_id = 0;
    char id_ar[20] = {0};

    memset(&mirroring_entry, 0, sizeof(mirroring_entry));

    while (TRUE == SWCTRL_POM_GetNextMirrorEntry(&mirroring_entry))
    {
        json_t *port_obj_p = json_object();

        memset(id_ar, 0, sizeof(id_ar));
        lport_type = SWCTRL_POM_LogicalPortToUserPort(mirroring_entry.mirror_source_port, &unit, &port, &trunk_id);

        if (lport_type == SWCTRL_LPORT_NORMAL_PORT)
        {
            sprintf(id_ar, "eth%lu/%lu", (unsigned long)unit, (unsigned long)port);
        }
        else if (lport_type == SWCTRL_LPORT_TRUNK_PORT)
        {
            sprintf(id_ar, "trunk%lu", (unsigned long)trunk_id);
        }
        json_object_set_new(port_obj_p, "src", json_string(id_ar));

        lport_type = SWCTRL_POM_LogicalPortToUserPort(mirroring_entry.mirror_destination_port, &unit, &port, &trunk_id);

        if (lport_type == SWCTRL_LPORT_NORMAL_PORT)
        {
            sprintf(id_ar, "eth%lu/%lu", (unsigned long)unit, (unsigned long)port);
        }
        else if (lport_type == SWCTRL_LPORT_TRUNK_PORT)
        {
            sprintf(id_ar, "trunk%lu", (unsigned long)trunk_id);
        }
        json_object_set_new(port_obj_p, "dst", json_string(id_ar));

        switch(mirroring_entry.mirror_type)
        {
            case VAL_mirrorType_rx:
                json_object_set_new(port_obj_p, "mode", json_string("rx"));
                break;

            case VAL_mirrorType_tx:
                json_object_set_new(port_obj_p, "mode", json_string("tx"));
                break;

            case VAL_mirrorType_both:
                json_object_set_new(port_obj_p, "mode", json_string("both"));
                break;

            default:
                break;
        }
        json_array_append_new(ports_p, port_obj_p);
    } //while (TRUE == SWCTRL_POM_GetNextMirrorEntry(&mirroring_entry))

    json_object_set_new(result_p, "ports", ports_p);
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete port monitor.
 *
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_MONITOR_Port_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    const char*  id_str_p;
    char  buffer_ar[40] = {0};
    char  *save_str_addr = NULL;
    char  *src_str_p;
    char  *dst_str_p;
    UI32_T src_lport = 0;
    UI32_T dst_lport = 0;

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ID.");
    }
    id_str_p = json_string_value(id_p);
    CGI_UTIL_UrlDecode(buffer_ar, id_str_p);

    src_str_p = strtok_r(buffer_ar, "_", &save_str_addr);
    dst_str_p = strtok_r(NULL, "_", &save_str_addr);

    if (TRUE != CGI_UTIL_InterfaceIdToLport(src_str_p, &src_lport))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid source port.");
    }

    if (TRUE != CGI_UTIL_InterfaceIdToLport(dst_str_p, &dst_lport))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid destination port.");
    }

    if (TRUE != SWCTRL_PMGR_SetMirrorStatus(src_lport, dst_lport, VAL_mirrorStatus_invalid))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "portMonitor.SetMirrorStatusError", "Failed.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

void CGI_MODULE_MONITOR_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler = CGI_MODULE_MONITOR_Port_Read;
    handlers.create_handler = CGI_MODULE_MONITOR_Port_Create;
    CGI_MAIN_Register("/api/v1/monitor/port", &handlers, 0);

    {
        CGI_API_HANDLER_SET_T handlers = {0};

        handlers.delete_handler = CGI_MODULE_MONITOR_Port_Delete;
        CGI_MAIN_Register("/api/v1/monitor/port/{id}", &handlers, 0);
    }
}

static void CGI_MODULE_MONITOR_Init()
{
    CGI_MODULE_MONITOR_RegisterHandlers();
}
