#include "cgi_request.h"
#include "cgi_auth.h"
#include "cgi_util.h"
#include "pfc_pmgr.h"

/**----------------------------------------------------------------------
 * This API is used to create PFC entry.
 *
 * @param port (required, number) port ID
 * @param queues (required, array) set PFC queues
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PFC_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *port_p;
    json_t  *queues_p;
    json_t  *queue_p;
    UI32_T pfc_port_mode = PFC_TYPE_PMODE_ON;
    UI32_T pfc_pri_bmp = 0;
    UI32_T lport = 0;
    UI32_T ret = 0;
    int queue_num = 0, idx = 0, queue = 0;

    port_p = CGI_REQUEST_GetBodyValue(http_request, "port");

    if (port_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get port.");
    }
    lport = json_integer_value(port_p);

    queues_p = CGI_REQUEST_GetBodyValue(http_request, "queues");

    if (queues_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get queues.");
    }
    queue_num = json_array_size(queues_p);

    if (0 >= queue_num)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get queues.");
    }

    //clear previous setting
    PFC_PMGR_GetDataByField(lport, PFC_TYPE_FLDE_PORT_PRI_EN_ADM, &pfc_pri_bmp);
    PFC_PMGR_SetDataByField(lport, PFC_TYPE_FLDE_PORT_PRI_EN_ADM_REM, &pfc_pri_bmp);
    pfc_pri_bmp = 0;

    for (idx = 0; idx < queue_num; idx ++)
    {
        queue_p = json_array_get(queues_p, idx);

        if (NULL != queue_p)
        {
            queue = json_integer_value(queue_p);

            if (queue >= 8)
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid queue ID.");
            }

            pfc_pri_bmp |= (0x1 << queue);
        }
    }

    if (PFC_TYPE_RCE_OK != PFC_PMGR_SetDataByField(lport, PFC_TYPE_FLDE_PORT_MODE_ADM, &pfc_port_mode))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "pfc.PfcOnError", "Failed.");
    }

    if (PFC_TYPE_RCE_OK != PFC_PMGR_SetDataByField(lport, PFC_TYPE_FLDE_PORT_PRI_EN_ADM_ADD, &pfc_pri_bmp))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "pfc.PfcPriError", "Failed.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to read PFC settings.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PFC_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *pfcs_p = json_array();
    UI32_T pfc_port_mode = PFC_TYPE_PMODE_ON;
    UI32_T pfc_pri_bmp = 0;
    UI32_T ifindex = 0;
    int pri = 0;

    while (SWCTRL_POM_GetNextLogicalPort(&ifindex) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        json_t *pfc_obj_p = json_object();
        json_t *queues_p = json_array();

        if (TRUE != PFC_PMGR_GetDataByField(ifindex, PFC_TYPE_FLDE_PORT_MODE_ADM, &pfc_port_mode))
        {
            continue;
        }

        if (PFC_TYPE_PMODE_ON != pfc_port_mode)
        {
            continue;
        }

        if (TRUE == PFC_PMGR_GetDataByField(ifindex, PFC_TYPE_FLDE_PORT_PRI_EN_ADM, &pfc_pri_bmp))
        {
            for (pri = 0; pri <=7; pri++)
            {
                if (pfc_pri_bmp & (1 << pri))
                {
                    json_array_append_new(queues_p, json_integer(ifindex));
                }
            }
        }

        json_object_set_new(pfc_obj_p, "port", json_integer(ifindex));
        json_object_set_new(pfc_obj_p, "queues", queues_p);
    }
    json_object_set_new(result_p, "pfcs", pfcs_p);
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete PFC.
 *
 * @param id (required, string) port ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PFC_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    const char* id_str_p;
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    UI32_T  pfc_port_mode = PFC_TYPE_PMODE_OFF;
    UI32_T  pfc_pri_bmp = 0;
    UI32_T  lport = 0;
    UI32_T  ret = 0;

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ID.");
    }
    lport = json_integer_value(id_p);

    if (PFC_TYPE_RCE_OK != PFC_PMGR_SetDataByField(lport, PFC_TYPE_FLDE_PORT_MODE_ADM, &pfc_port_mode))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "pfc.PfcOffError", "Failed.");
    }

    if (TRUE == PFC_PMGR_GetDataByField(lport, PFC_TYPE_FLDE_PORT_PRI_EN_ADM, &pfc_pri_bmp))
    {
        if (TRUE != PFC_PMGR_SetDataByField(lport, PFC_TYPE_FLDE_PORT_PRI_EN_ADM_REM, &pfc_pri_bmp))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "pfc.PfcPriError", "Failed.");
        }
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

void CGI_MODULE_PFC_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler = CGI_MODULE_PFC_Read;
    handlers.create_handler = CGI_MODULE_PFC_Create;
    CGI_MAIN_Register("/api/v1/pfc", &handlers, 0);

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.delete_handler = CGI_MODULE_PFC_ID_Delete;
        CGI_MAIN_Register("/api/v1/pfc/{id}", &handlers, 0);
    }
}

static void CGI_MODULE_PFC_Init()
{
    CGI_MODULE_PFC_RegisterHandlers();
}
