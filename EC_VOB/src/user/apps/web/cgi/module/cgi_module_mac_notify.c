#include "amtr_om.h"

#if 0
static CGI_STATUS_CODE_T CGI_MODULE_MAC_NOTIFY_Read_Global(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t *result = (json_t *) CGI_RESPONSE_GetResult(http_response);

    BOOL_T status;
    UI32_T interval;

    if (TRUE != AMTR_OM_GetMacNotifyGlobalStatus(&status))
    {
        // TODO: Add error code and message
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, NULL, NULL);
    }

    if (TRUE != AMTR_OM_GetMacNotifyInterval(&interval))
    {
        // TODO: Add error code and message
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, NULL, NULL);
    }

    interval = interval / SYS_BLD_TICKS_PER_SECOND;

    {
        json_t *status_value = json_boolean(status);
        json_t *interval_value = json_integer(interval);

        json_object_set_new(result, "status", status_value);
        json_object_set_new(result, "interval", interval_value);
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

static CGI_STATUS_CODE_T CGI_MODULE_MAC_NOTIFY_Update_Global(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t *args = (json_t *) CGI_REQUEST_GetArguments(http_request);
    json_t *status, *interval;

    BOOL_T status_value;
    UI32_T interval_value;

    status = json_object_get(args, "status");
    if (status != NULL)
    {
        // TODO: Add ASSERT check
        if (json_typeof(status) != JSON_TRUE && json_typeof(status) != JSON_FALSE)
        {
            // TODO: Add error code and message
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, NULL, NULL);
        }

        status_value = json_boolean_value(status);

        if (AMTR_PMGR_SetMacNotifyGlobalStatus(status_value) != TRUE)
        {
            // TODO: Add error code and message
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, NULL, NULL);
        }
    }

    interval = json_object_get(args, "interval");
    if (interval != NULL)
    {
        // TODO: Add ASSERT check
        interval_value = json_integer_value(interval);

        if (AMTR_PMGR_SetMacNotifyInterval(interval_value) != TRUE)
        {
            // TODO: Add error code and message
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, NULL, NULL);
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

static CGI_STATUS_CODE_T CGI_MODULE_MAC_NOTIFY_Update_Eth(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t *args = (json_t *) CGI_REQUEST_GetArguments(http_request);

    json_t *uri_id_0 = (json_t *) CGI_REQUEST_GetUriIdAtIndex(http_request, 0);

    json_t *status;

    UI32_T ifindex;
    UI32_T ifindex_start = 1;
    UI32_T ifindex_end = SYS_ADPT_TOTAL_NBR_OF_LPORT;
    BOOL_T status_value;

    if (uri_id_0 != NULL)
    {
        const char *id0_value;

        // TODO: Add ASSERT check
        if (json_typeof(uri_id_0) != JSON_STRING)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, NULL, NULL);
        }

        id0_value = json_string_value(uri_id_0);

        ASSERT(id0_value != NULL);

        ifindex_start = ifindex_end = atol(id0_value);
    }

    status = json_object_get(args, "status");
    if (status == NULL || (json_typeof(status) != JSON_TRUE && json_typeof(status) != JSON_FALSE))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, NULL, NULL);
    }

    status_value = json_boolean_value(status);

    for (ifindex = ifindex_start; ifindex <= ifindex_end; ++ ifindex)
    {
        UI32_T unit, port, trunk_id;

        if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id))
        {
            continue;
        }

        if (AMTR_PMGR_SetMacNotifyPortStatus(ifindex, status_value) != TRUE)
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, NULL, NULL);
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

static CGI_STATUS_CODE_T CGI_MODULE_MAC_NOTIFY_Update_Pch(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t *args = (json_t *) CGI_REQUEST_GetArguments(http_request);

    json_t *uri_id_0 = (json_t *) CGI_REQUEST_GetUriIdAtIndex(http_request, 0);

    json_t *status;

    UI32_T ifindex;
    UI32_T ifindex_start = SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER;
    UI32_T ifindex_end = SYS_ADPT_TOTAL_NBR_OF_LPORT;
    BOOL_T status_value;

    if (uri_id_0 != NULL)
    {
        const char *id0_value;
        // TODO: Add ASSERT check, and the real type shall be integer
        if (json_typeof(uri_id_0) != JSON_STRING)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, NULL, NULL);
        }

        id0_value = json_string_value(uri_id_0);

        ASSERT(id0_value != NULL);

        ifindex_start = ifindex_end = atol(id0_value);
    }

    status = json_object_get(args, "status");
    if (status == NULL || (json_typeof(status) != JSON_TRUE && json_typeof(status) != JSON_FALSE))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, NULL, NULL);
    }

    status_value = json_boolean_value(status);

    for (ifindex = ifindex_start; ifindex <= ifindex_end; ++ ifindex)
    {
        UI32_T unit, port, trunk_id;

        if (SWCTRL_LPORT_TRUNK_PORT != SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id))
        {
            continue;
        }

        if (AMTR_PMGR_SetMacNotifyPortStatus(ifindex, status_value) != TRUE)
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, NULL, NULL);
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/* ----------------------------------------------------------------------
 * CGI_MODULE_MAC_NOTIFY_Read_Eth:
 * Request:
 *
 * Response:
 *   {
 *     "recordset": [
 *       {fields(id, unit, port, status)},
 *       {fields(id, unit, port, status)},
 *       ...
 *     ]
 *   }
 * ---------------------------------------------------------------------- */

static CGI_STATUS_CODE_T CGI_MODULE_MAC_NOTIFY_Read_Eth(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t *uri_id_0 = (json_t *) CGI_REQUEST_GetUriIdAtIndex(http_request, 0);

    json_t *args = (json_t *) CGI_REQUEST_GetArguments(http_request);
    json_t *result = (json_t *) CGI_RESPONSE_GetResult(http_response);

    UI32_T ifindex;

    UI32_T ifindex_start = 1;
    UI32_T ifindex_end = SYS_ADPT_TOTAL_NBR_OF_LPORT;

    UI32_T index = 0;

    json_t *recordset = NULL;

    recordset = json_array();
    json_object_set_new(result, "recordset", recordset);

    //
    // output will be ifindex_start / ifindex_end
    //
    {
        if (uri_id_0 != NULL)
        {
            if (L_STDLIB_StrIsDigit((char *)json_string_value(uri_id_0)) != TRUE)
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, NULL, NULL);
            }

            ifindex_start = ifindex_end = atol(json_string_value(uri_id_0));
        }
        else
        {
            json_t *start_id0 = (json_t *) CGI_REQUEST_GetStartAtIndex(http_request, 0);

            if (start_id0 != NULL)
            {
                if (L_STDLIB_StrIsDigit((char *)json_string_value(start_id0)) != TRUE)
                {
                    return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, NULL, NULL);
                }

                ifindex_start = atol(json_string_value(start_id0)) + 1;
            }
        }
    }

    for (ifindex = ifindex_start; ifindex <= ifindex_end; ++ ifindex)
    {
        UI32_T unit, port, trunk_id;
        BOOL_T status;

        if (SWCTRL_LPORT_NORMAL_PORT != SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id))
        {
            continue;
        }

        if (AMTR_OM_GetMacNotifyPortStatus(ifindex, &status) != TRUE)
        {
            continue;
        }

        {
            json_t *record;

            char id[30];

            record = json_object();
            json_array_append_new(recordset, record);

            SYSFUN_Snprintf(id, sizeof(id), "/%lu", ifindex);
            id[sizeof(id) - 1] = '\0';

            {
                json_t *id_value = json_string(id);
                json_t *unit_value = json_integer(unit);
                json_t *port_value = json_integer(port);
                json_t *status_value = json_boolean(status);

                json_object_set_new(record, "id", id_value);
                json_object_set_new(record, "unit", unit_value);
                json_object_set_new(record, "port", port_value);
                json_object_set_new(record, "status", status_value);
            }
        }

        index ++;
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

static CGI_STATUS_CODE_T CGI_MODULE_MAC_NOTIFY_Read_Pch(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t *uri_id_0 = (json_t *) CGI_REQUEST_GetUriIdAtIndex(http_request, 0);

    json_t *args = (json_t *) CGI_REQUEST_GetArguments(http_request);
    json_t *result = (json_t *) CGI_RESPONSE_GetResult(http_response);

    UI32_T ifindex;

    UI32_T ifindex_start = 1;
    UI32_T ifindex_end = SYS_ADPT_TOTAL_NBR_OF_LPORT;

    UI32_T index = 0;

    json_t *recordset = NULL;

    recordset = json_array();
    json_object_set_new(result, "recordset", recordset);

    {
        if (uri_id_0 != NULL)
        {
            if (L_STDLIB_StrIsDigit((char *)json_string_value(uri_id_0)) != TRUE)
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, NULL, NULL);
            }

            ifindex_start = ifindex_end = atol(json_string_value(uri_id_0));
        }
        else
        {
            json_t *start_id0 = (json_t *) CGI_REQUEST_GetStartAtIndex(http_request, 0);

            if (start_id0 != NULL)
            {
                if (L_STDLIB_StrIsDigit((char *)json_string_value(start_id0)) != TRUE)
                {
                    return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, NULL, NULL);
                }

                ifindex_start = atol(json_string_value(start_id0)) + 1;
            }
        }
    }

    for (ifindex = ifindex_start; ifindex <= ifindex_end; ++ ifindex)
    {
        UI32_T unit, port, trunk_id;
        BOOL_T status;

        if (SWCTRL_LPORT_TRUNK_PORT != SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id))
        {
            continue;
        }

        if (AMTR_OM_GetMacNotifyPortStatus(ifindex, &status) != TRUE)
        {
            continue;
        }

        {
            json_t *record;

            char id[30];

            record = json_object();
            json_array_append_new(recordset, record);

            SYSFUN_Snprintf(id, sizeof(id), "/%lu", ifindex);
            id[sizeof(id) - 1] = '\0';

            {
                json_t *id_value = json_string(id);
                json_t *trunk_id_value = json_integer(trunk_id);
                json_t *status_value = json_boolean(status);

                json_object_set_new(record, "id", id_value);
                json_object_set_new(record, "trunkId", trunk_id_value);
                json_object_set_new(record, "status", status_value);
            }
        }

        index ++;
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}
#endif

void CGI_MODULE_MAC_NOTIFY_RegisterGlobalHandlers()
{
//    CGI_API_HANDLER_SET_T handlers = {0};
//
//    handlers.update_handler = CGI_MODULE_MAC_NOTIFY_Update_Global;
//    handlers.read_handler = CGI_MODULE_MAC_NOTIFY_Read_Global;
//
//    CGI_REST_Register("/mac-notify/global",
//                      NULL,
//                      NULL,
//                      &handlers);
}

void CGI_MODULE_MAC_NOTIFY_RegisterEthHandlers()
{
//    CGI_API_HANDLER_SET_T handlers = {0};
//
//    handlers.update_handler = CGI_MODULE_MAC_NOTIFY_Update_Eth;
//    handlers.read_handler = CGI_MODULE_MAC_NOTIFY_Read_Eth;
//
//    // TODO: Write argument syntax
//    CGI_REST_Register("/mac-notify/eth",
//                      NULL,
//                      "["
//                      "{\"type\":\"string\"}"
//                      "]",
//                      &handlers);
}

void CGI_MODULE_MAC_NOTIFY_RegisterPchHandlers()
{
//    CGI_API_HANDLER_SET_T handlers = {0};
//
//    handlers.update_handler = CGI_MODULE_MAC_NOTIFY_Update_Pch;
//    handlers.read_handler = CGI_MODULE_MAC_NOTIFY_Read_Pch;
//
//    // TODO: Write argument syntax
//    CGI_REST_Register("/mac-notify/pch",
//                      NULL,
//                      "["
//                      "{\"type\":\"string\"}"
//                      "]",
//                      &handlers);
}

static void CGI_MODULE_MAC_NOTIFY_Init()
{
    CGI_MODULE_MAC_NOTIFY_RegisterGlobalHandlers();
    CGI_MODULE_MAC_NOTIFY_RegisterEthHandlers();
    CGI_MODULE_MAC_NOTIFY_RegisterPchHandlers();
}
