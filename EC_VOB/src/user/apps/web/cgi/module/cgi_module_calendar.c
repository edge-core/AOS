#include <time.h>
#include "cgi_auth.h"
#include "sys_time.h"

//static BOOL_T CGI_MODULE_CALENDAR_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request);

/**----------------------------------------------------------------------
 * This API is used to get date and time.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_CALENDAR_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    struct timespec cur_time;
    time_t now;
    char date_time_ar[40] = {0};
    char offset_ar[8] = {0};

    ASSERT(result_p != NULL);
    time(&now);
    clock_gettime(CLOCK_REALTIME, &cur_time);

    /* format:
     * "2017-02-17T14:03:36,833711595+0800"
     */
    strftime(date_time_ar, sizeof(date_time_ar), "%FT%T", gmtime(&now));
    strftime(offset_ar, sizeof(offset_ar), "%z", gmtime(&now));
    sprintf(date_time_ar, "%s,%ld", date_time_ar, cur_time.tv_nsec);
    sprintf(date_time_ar, "%s%s", date_time_ar, offset_ar);

    if (result_p != NULL)
    {
        json_object_set_new(result_p, "dateTime", json_string(date_time_ar));
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to update calendar.
 *
 * @param dateTime (required, string) date and time.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_CALENDAR_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *time_p;
    int  year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
    char nsec_offset_ar[16] = {0};

    time_p = CGI_REQUEST_GetBodyValue(http_request, "dateTime");

    if (NULL == time_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get time_p.");
    }

    if (TRUE != json_is_string(time_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The time_p is not string type.");
    }

    /* Blank string is not an valid string
     */
    if ('\0' == json_string_value(time_p)[0])
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The time_p is null.");
    }

    if (sscanf(json_string_value(time_p), "%d-%d-%dT%d:%d:%d,%s",
            &year, &month, &day, &hour, &minute, &second, nsec_offset_ar) != 7)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The time_p is wrong format.");
    }

    if (TRUE != SYS_TIME_SetRealTimeClock(year, month, day, hour, minute, second))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "calendar.setSystemCalendarError", "Failed to set Calendar.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

void CGI_MODULE_CALENDAR_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler   = CGI_MODULE_CALENDAR_Read;
    handlers.update_handler = CGI_MODULE_CALENDAR_Update;

    CGI_MAIN_Register("/api/v1/calendar", &handlers, 0);
}


static void CGI_MODULE_CALENDAR_Init()
{
    CGI_MODULE_CALENDAR_RegisterHandlers();
}

/*
static BOOL_T CGI_MODULE_CALENDAR_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request)
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
