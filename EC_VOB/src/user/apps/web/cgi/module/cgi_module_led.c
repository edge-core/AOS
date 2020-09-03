#include "cgi_request.h"
#include "cgi_auth.h"
#include "cgi_util.h"
#include "led_pmgr.h"

/**----------------------------------------------------------------------
 * This API is used to set location led.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_LOCATION_LED_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    UI32_T  my_unit_id=1;
    UI32_T  status_value = 0;    
     UI32_T ret = 0;
    json_t  *status_p;
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

    status_p = CGI_REQUEST_GetBodyValue(http_request, "status");

    if ((NULL == status_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Please specify status");
    }


    if (TRUE == json_boolean_value(status_p))
    {
        status_value = TRUE;
    }
    else
    {
        status_value = FALSE;
    }

    STKTPLG_POM_GetMyUnitID(&my_unit_id);

    ret = LED_PMGR_SetLocationLED(my_unit_id, status_value);

    switch (ret)
    {
        case LEDDRV_TYPE_RET_ERR_HW_FAIL:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "A hardware error occurs when setting location led status.");
            break;
        case LEDDRV_TYPE_RET_ERR_NOT_SUPPORT:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "This device does not have location led.");
            break;
        case LEDDRV_TYPE_RET_OK:
            break;
        default:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "Unknown error");
            break;
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_DAI_Read


void CGI_MODULE_LOCATION_LED_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.update_handler = CGI_MODULE_LOCATION_LED_Update;
    CGI_MAIN_Register("/api/v1/location-led", &handlers, 0);
}

static void CGI_MODULE_LOCATION_LED_Init()
{
    CGI_MODULE_LOCATION_LED_RegisterHandlers();
}
