#include "cgi_request.h"
#include "cgi_auth.h"
#include "cgi_util.h"
#include "swctrl_pmgr.h"
#include "swctrl_pom.h"

/**----------------------------------------------------------------------
 * This API is used to set scheduler to queue.
 *
 * @param queue (required, number) Queue ID
 * @param weight (required, number)  scheduler value of the queue
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_SCHEDULER_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *queue_p;
    json_t  *weight_p;
    UI32_T  queue = 0;
    UI32_T  lport = 0;
    UI32_T  weight = 0;

    queue_p = CGI_REQUEST_GetBodyValue(http_request, "queue");

    if (queue_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get queue.");
    }
    queue = json_integer_value(queue_p);

    if (SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE <= queue)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid Queue ID.");
    }

    weight_p = CGI_REQUEST_GetBodyValue(http_request, "weight");

    if (weight_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get weight.");
    }
    weight = json_integer_value(weight_p);

    while (SWCTRL_POM_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        if(TRUE != L4_PMGR_QOS_SetPortEgressWrrQueueWeight(lport, queue, weight))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "qos.SetPortEgressWrrQueueWeight", "Failed.");
        }
    } //SWCTRL_POM_GetNextLogicalPort

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

void CGI_MODULE_SCHEDULER_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.update_handler = CGI_MODULE_SCHEDULER_Update;
    CGI_MAIN_Register("/api/v1/scheduler", &handlers, 0);
}

static void CGI_MODULE_SCHEDULER_Init()
{
    CGI_MODULE_SCHEDULER_RegisterHandlers();
}
