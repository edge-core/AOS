#include "cgi_request.h"
#include "cgi_auth.h"
#include "cgi_util.h"
#include "swctrl_pmgr.h"
#include "swctrl_pom.h"

/**----------------------------------------------------------------------
 * This API is used to set ECN to queue.
 *
 * @param queue (required, number) Queue ID
 * @param ecn (required, number)  ECN value of the queue
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ECN_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *queue_p;
    json_t  *ecn_p;
    UI32_T  ecn = 0;
    UI32_T  lport = 0;
    SWCTRL_RandomDetect_T value;

    memset(&value, 0, sizeof(value));
    value.max = 100;
    value.drop = 0;
    queue_p = CGI_REQUEST_GetBodyValue(http_request, "queue");

    if (queue_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get queue.");
    }
    value.queue_id = json_integer_value(queue_p);

    if (SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE <= value.queue_id)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid Queue ID.");
    }

    ecn_p = CGI_REQUEST_GetBodyValue(http_request, "ecn");

    if (ecn_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ECN threshold.");
    }
    value.min = json_integer_value(ecn_p);
    value.ecn = value.min;
    value.valid = TRUE;

    while (SWCTRL_POM_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        if(TRUE != SWCTRL_PMGR_RandomDetect(lport, &value))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "qos.RandomDetectError", "Failed.");
        }
    } //SWCTRL_POM_GetNextLogicalPort

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to read ECN setting.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ECN_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *queues_p = json_array();
    SWCTRL_OM_RandomDetect_T info;
    UI32_T lport = 0, queue = 0;

    json_object_set_new(result_p, "queues", queues_p);

    if (SWCTRL_POM_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        if (TRUE == SWCTRL_POM_GetRandomDetect(lport, &info))
        {
            for (queue = 0; queue < SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE ; queue++)
            {
                if (TRUE == info.valid[queue])
                {
                    json_t *queue_obj_p = json_object();

                    json_object_set_new(queue_obj_p, "queue", json_integer(queue));
                    json_object_set_new(queue_obj_p, "ecn", json_integer(info.ecn[queue]));
                    json_array_append_new(queues_p, queue_obj_p);
                }
            }
        } //SWCTRL_POM_GetRandomDetect
    } //if (SWCTRL_POM_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to read ECN setting.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ECN_ID_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *queue_p;
    json_t  *dscp_p = json_array();
    UI32_T queue = 0;
    SWCTRL_OM_RandomDetect_T info;
    UI32_T lport = 0;

    queue_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (queue_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get Queue ID.");
    }
    queue = json_integer_value(queue_p);

    if (SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE <= queue)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid Queue ID.");
    }

    if (SWCTRL_POM_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        if (TRUE == SWCTRL_POM_GetRandomDetect(lport, &info))
        {
            json_object_set_new(result_p, "queue", json_integer(queue));
            json_object_set_new(result_p, "ecn", json_integer(info.ecn[queue]));
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

void CGI_MODULE_ECN_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler = CGI_MODULE_ECN_Read;
    handlers.update_handler = CGI_MODULE_ECN_Update;
    CGI_MAIN_Register("/api/v1/ecn", &handlers, 0);

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_ECN_ID_Read;
        CGI_MAIN_Register("/api/v1/ecn/{id}", &handlers, 0);
    }
}

static void CGI_MODULE_ECN_Init()
{
    CGI_MODULE_ECN_RegisterHandlers();
}
