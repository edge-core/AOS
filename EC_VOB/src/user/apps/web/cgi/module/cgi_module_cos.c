#include "cgi_request.h"
#include "cgi_auth.h"
#include "cgi_util.h"
#include "cos_type.h"
#include "cos_vm.h"
#include "l4_pmgr.h"

static UI32_T CGI_MODULE_COS_GetPhbByQueue(UI32_T queue_id);

/**----------------------------------------------------------------------
 * This API is used to set DSCP to queue.
 *
 * @param queue (required, number) Queue ID
 * @param dscp (required, array)  DSCP value of the queue
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_COS_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *queue_p;
    json_t  *dscp_p;
    json_t  *one_dscp_p;
    UI32_T  queue = 0;
    UI32_T  dscp = 0, dscp_num = 0, idx = 0;
    UI32_T  lport = 0, phb = 0, phb_tmp = 0, color = 0;

    queue_p = CGI_REQUEST_GetBodyValue(http_request, "queue");

    if (queue_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get queue.");
    }
    queue = json_integer_value(queue_p);

    dscp_p = CGI_REQUEST_GetBodyValue(http_request, "dscp");

    if (dscp_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get dscp.");
    }

    dscp_num = json_array_size(dscp_p);

    if (0 >= dscp_num)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "dscp number is 0.");
    }

    for (idx = 0; idx < dscp_num; idx ++)
    {
        one_dscp_p = json_array_get(dscp_p, idx);

        if (NULL != one_dscp_p)
        {
            dscp = json_integer_value(one_dscp_p);

            if (MAX_DSCP_VAL < dscp)
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid dscp.");
            }
        }
    }

    phb = CGI_MODULE_COS_GetPhbByQueue(queue);

    for (idx = 0; idx < dscp_num; idx ++)
    {
        one_dscp_p = json_array_get(dscp_p, idx);

        if (NULL != one_dscp_p)
        {
            lport = 0;
            dscp = json_integer_value(one_dscp_p);

            while (SWCTRL_POM_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
#if (SYS_CPNT_COS_ING_COS_TO_INTER_DSCP_PER_PORT == TRUE)
                color = 0;
                L4_PMGR_QOS_GetIngressDscp2Dscp(lport, dscp, &phb_tmp, &color);

                if (COS_TYPE_E_NONE != L4_PMGR_QOS_SetPortDscp2InternalDscp(lport, COS_TYPE_PRIORITY_USER, dscp, phb, color))
                {
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "qos.PortDscp2InternalDscpError", "Failed.");
                }
#endif  /* #if (SYS_CPNT_COS_ING_COS_TO_INTER_DSCP_PER_PORT == TRUE) */
            } //SWCTRL_POM_GetNextLogicalPort
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to read CoS setting.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_COS_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *queues_p = json_array();
    UI32_T phb = 0, queue = 0, color = 0;
    UI32_T lport = 0;

    json_object_set_new(result_p, "queues", queues_p);

    if (SWCTRL_POM_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        for (phb = 0; phb <= MAX_PHB_VAL; ++phb)
        {
            json_t *queue_obj_p = json_object();
            json_t *dscp_p = json_array();

            if (COS_TYPE_E_NONE == L4_PMGR_QOS_GetIngressPhb2Queue(COS_TYPE_PRIORITY_USER, phb, &queue))
            {
                json_t *dscp_obj_p = json_object();
                UI32_T dscp = 0, phb_tmp = 0;

                for(dscp = 0; dscp <= MAX_DSCP_VAL; dscp++)
                {
                    if(L4_PMGR_QOS_GetIngressDscp2Dscp(lport, dscp, &phb_tmp, &color))
                    {
                        if (phb_tmp == phb)
                        {
                            json_array_append_new(dscp_p, json_integer(dscp));
                        }
                    }
                }
                json_object_set_new(queue_obj_p, "queue", json_integer(queue));
                json_object_set_new(queue_obj_p, "phb", json_integer(phb));
                json_object_set_new(queue_obj_p, "dscp", dscp_p);
                json_array_append_new(queues_p, queue_obj_p);
            }
        } //for (phb = 0; phb <= MAX_PHB_VAL; ++phb)
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to read CoS setting.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_COS_ID_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *queue_p;
    json_t  *dscp_p = json_array();
    UI32_T queue = 0, queue_tmp = 0;
    UI32_T phb = 0, phb_tmp = 0, color = 0, dscp = 0;
    UI32_T lport = 0;

    queue_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (queue_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get Queue ID.");
    }
    queue = json_integer_value(queue_p);

    if (SWCTRL_POM_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        for (phb = 0; phb <= MAX_PHB_VAL; ++phb)
        {
            if (COS_TYPE_E_NONE == L4_PMGR_QOS_GetIngressPhb2Queue(COS_TYPE_PRIORITY_USER, phb, &queue_tmp))
            {
                if (queue_tmp == queue)
                {
                    break;
                }
            }
        } //for (phb = 0; phb <= MAX_PHB_VAL; ++phb)

        for(dscp = 0; dscp <= MAX_DSCP_VAL; dscp++)
        {
            if(L4_PMGR_QOS_GetIngressDscp2Dscp(lport, dscp, &phb_tmp, &color))
            {
                if (phb_tmp == phb)
                {
                    json_array_append_new(dscp_p, json_integer(dscp));
                }
            }
        }
        json_object_set_new(result_p, "queue", json_integer(queue));
        json_object_set_new(result_p, "phb", json_integer(phb));
        json_object_set_new(result_p, "dscp", dscp_p);
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

void CGI_MODULE_COS_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler = CGI_MODULE_COS_Read;
    handlers.update_handler = CGI_MODULE_COS_Update;
    CGI_MAIN_Register("/api/v1/cos", &handlers, 0);

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_COS_ID_Read;
        CGI_MAIN_Register("/api/v1/cos/{id}", &handlers, 0);
    }
}

static void CGI_MODULE_COS_Init()
{
    CGI_MODULE_COS_RegisterHandlers();
}

static UI32_T CGI_MODULE_COS_GetPhbByQueue(UI32_T queue_id)
{
    UI32_T phb = 0, queue_tmp = 0;

    for (phb = 0; phb <= MAX_PHB_VAL; ++phb)
    {
        if (COS_TYPE_E_NONE == L4_PMGR_QOS_GetIngressPhb2Queue(COS_TYPE_PRIORITY_USER, phb, &queue_tmp))
        {//printf("%s, %d. get %lu, queue %lu, phb %lu.\n", __FUNCTION__, __LINE__, queue_id, queue_tmp, phb);
            if (queue_id == queue_tmp)
            {
                return phb;
            }
        }
    }
    return 0;
}
