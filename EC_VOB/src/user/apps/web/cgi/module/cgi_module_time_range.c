#include "cgi_auth.h"
#include "time_range_pmgr.h"

static void CGI_MODULE_TIME_RANGE_GetOne(TIME_RANGE_TYPE_ENTRY_T *entry_p, json_t *time_obj_p);
static BOOL_T CGI_MODULE_TIME_RANGE_ConvertWeekDay(char *weekday_p, UI8_T *val_p);


/**----------------------------------------------------------------------
 * This API is used to get time range info.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_TIME_RANGE_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *times_p = json_array();
    TIME_RANGE_TYPE_ENTRY_T entry;
    UI32_T index = TIME_RANGE_TYPE_UNDEF_TIME_RANGE;

    memset(&entry, 0, sizeof(entry));
    json_object_set_new(result_p, "times", times_p);

    while (TIME_RANGE_ERROR_TYPE_NONE == TIME_RANGE_PMGR_GetNextTimeRangeEntry(&index , &entry))
    {
        json_t *time_obj_p = json_object();

        CGI_MODULE_TIME_RANGE_GetOne(&entry, time_obj_p);
        json_array_append_new(times_p, time_obj_p);
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to create time range.
 *
 * @param name     (Mandatory, string) Time range name
 * @param absolute (optional, object) Absolute time and date
 * @param periodic (optional, array) Periodic time and date
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_TIME_RANGE_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p;
    json_t  *abs_p;
    json_t  *periods_p;
    const char*  name_str_p;
    TIME_RANGE_TYPE_ENTRY_T entry;
    TIME_RANGE_TYPE_ABSOLUTE_TIME_T abs_start, abs_end;
    TIME_RANGE_TYPE_PERIODIC_TIME_T period_ar[SYS_ADPT_TIME_RANGE_MAX_NBR_OF_PERIODIC_ENTRY];
    int idx = 0;
    int period_num = 0;
    BOOL_T set_abs = FALSE;
    BOOL_T set_abs_start = FALSE;

    memset(&entry, 0, sizeof(entry));
    memset(&abs_start, 0, sizeof(abs_start));
    memset(&abs_end, 0, sizeof(abs_end));
    memset(period_ar, 0, sizeof(period_ar));
    name_p = CGI_REQUEST_GetBodyValue(http_request, "name");
    abs_p = CGI_REQUEST_GetBodyValue(http_request, "absolute");
    periods_p = CGI_REQUEST_GetBodyValue(http_request, "periodic");

    if (NULL == name_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Must specify time range name.");
    }

    name_str_p = json_string_value(name_p);

    if (NULL != abs_p)
    {
        json_t  *abs_start_p;
        json_t  *abs_start_year_p;
        json_t  *abs_start_month_p;
        json_t  *abs_start_day_p;
        json_t  *abs_start_hour_p;
        json_t  *abs_start_minute_p;
        json_t  *abs_end_p;
        json_t  *abs_end_year_p;
        json_t  *abs_end_month_p;
        json_t  *abs_end_day_p;
        json_t  *abs_end_hour_p;
        json_t  *abs_end_minute_p;

        abs_start_p = json_object_get(abs_p, "start");
        abs_end_p = json_object_get(abs_p, "end");

        if (NULL == abs_end_p)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Must specify absolute end time");
        }

        if (NULL != abs_start_p)
        {
            abs_start_year_p = json_object_get(abs_start_p, "year");
            abs_start_month_p = json_object_get(abs_start_p, "month");
            abs_start_day_p = json_object_get(abs_start_p, "day");
            abs_start_minute_p = json_object_get(abs_start_p, "minute");
            abs_start_hour_p = json_object_get(abs_start_p, "hour");

            if ((NULL == abs_start_year_p) || (NULL == abs_start_month_p) || (NULL == abs_start_day_p)
                    || (NULL == abs_start_minute_p) || (NULL == abs_start_hour_p))
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Absolute start time miss some fields");
            }

            abs_start.hour = json_integer_value(abs_start_hour_p);
            abs_start.minute = json_integer_value(abs_start_minute_p);
            abs_start.day = json_integer_value(abs_start_day_p);
            abs_start.month = json_integer_value(abs_start_month_p);
            abs_start.year = json_integer_value(abs_start_year_p);
            set_abs_start = TRUE;
        }

        abs_end_year_p = json_object_get(abs_end_p, "year");
        abs_end_month_p = json_object_get(abs_end_p, "month");
        abs_end_day_p = json_object_get(abs_end_p, "day");
        abs_end_minute_p = json_object_get(abs_end_p, "minute");
        abs_end_hour_p = json_object_get(abs_end_p, "hour");

        if ((NULL == abs_end_year_p) || (NULL == abs_end_month_p) || (NULL == abs_end_day_p)
                || (NULL == abs_end_minute_p) || (NULL == abs_end_hour_p))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Absolute end time miss some fields");
        }

        abs_end.hour = json_integer_value(abs_end_hour_p);
        abs_end.minute = json_integer_value(abs_end_minute_p);
        abs_end.day = json_integer_value(abs_end_day_p);
        abs_end.month = json_integer_value(abs_end_month_p);
        abs_end.year = json_integer_value(abs_end_year_p);
        set_abs = TRUE;
    } //if (NULL != abs_p)

    if (NULL != periods_p)
    {
        json_t  *period_p;
        json_t  *period_start_p;
        json_t  *start_day_p;
        json_t  *start_hour_p;
        json_t  *start_minute_p;
        json_t  *period_end_p;
        json_t  *end_day_p;
        json_t  *end_hour_p;
        json_t  *end_minute_p;

        period_num = json_array_size(periods_p);

        if (SYS_ADPT_TIME_RANGE_MAX_NBR_OF_PERIODIC_ENTRY < period_num)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Periodic max is 7 entries.");
        }

        for (idx = 0; idx < period_num; idx++)
        {
            period_p = json_array_get(periods_p, idx);

            if (NULL == period_p)
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get a periodic.");
            }

            period_start_p = json_object_get(period_p, "start");
            period_end_p = json_object_get(period_p, "end");

            if (NULL == period_start_p)
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Must specify periodic start time");
            }

            if (NULL == period_end_p)
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Must specify periodic end time");
            }

            start_day_p = json_object_get(period_start_p, "day");
            start_hour_p = json_object_get(period_start_p, "hour");
            start_minute_p = json_object_get(period_start_p, "minute");
            end_day_p = json_object_get(period_end_p, "day");
            end_hour_p = json_object_get(period_end_p, "hour");
            end_minute_p = json_object_get(period_end_p, "minute");

            if ((NULL == start_day_p) || (NULL == start_hour_p) || (NULL == start_minute_p)
                    || (NULL == end_day_p) || (NULL == end_hour_p) || (NULL == end_minute_p))
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Periodic start/end time miss fileds");
            }

            if (TRUE != CGI_MODULE_TIME_RANGE_ConvertWeekDay(
                    (char*)json_string_value(start_day_p), &period_ar[idx].start_type_of_wk))
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid Periodic start day");
            }

            if (TRUE != CGI_MODULE_TIME_RANGE_ConvertWeekDay(
                    (char*)json_string_value(end_day_p), &period_ar[idx].end_type_of_wk))
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid Periodic end day");
            }

            period_ar[idx].start_hour = json_integer_value(start_hour_p);
            period_ar[idx].start_minute = json_integer_value(start_minute_p);
            period_ar[idx].start_second = 0;

            period_ar[idx].end_hour = json_integer_value(end_hour_p);
            period_ar[idx].end_minute = json_integer_value(end_minute_p);
            period_ar[idx].end_second = 0;
        } //for (idx = 0; idx < period_num; idx++)
    } //if (NULL != periods_p)

    if (TIME_RANGE_ERROR_TYPE_NONE != TIME_RANGE_PMGR_GetTimeRangeEntryByName((UI8_T *)name_str_p, &entry))
    {
        if (TIME_RANGE_ERROR_TYPE_NONE != TIME_RANGE_PMGR_CreateTimeRangeEntry((UI8_T *)name_str_p))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "time-range.CreateTimeRangeError", "Failed to create time range.");
        }
    }

    if (TRUE == set_abs)
    {
        if (TRUE == set_abs_start)
        {
            if (TIME_RANGE_ERROR_TYPE_NONE != TIME_RANGE_PMGR_SetTimeRangeAbsolute((UI8_T *)name_str_p, &abs_start, &abs_end))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "time-range.SetTimeRangeAbsoluteError", "Failed to set time range absolute.");
            }
        }
        else
        {
            if (TIME_RANGE_ERROR_TYPE_NONE != TIME_RANGE_PMGR_SetTimeRangeAbsolute((UI8_T *)name_str_p, NULL, &abs_end))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "time-range.SetTimeRangeAbsoluteError", "Failed to set time range absolute.");
            }
        }
    }

    for (idx = 0; idx < period_num; idx++)
    {
        if (TIME_RANGE_ERROR_TYPE_NONE != TIME_RANGE_PMGR_SetTimeRangePeriodic((UI8_T *)name_str_p, &period_ar[idx]))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "time-range.SetTimeRangePeriodicError", "Failed to set time range periodic.");
        }
    } //for (idx = 0; idx < period_num; idx++)

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_TIME_RANGE_Create

/**----------------------------------------------------------------------
 * This API is used to get an time range info by name.
 *
 * @param id (required, string) time range name
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_TIME_RANGE_ID_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p;
    const char* name_str_p;
    TIME_RANGE_TYPE_ENTRY_T entry;

    memset(&entry, 0, sizeof(entry));
    name_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (NULL == name_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Time range name is NULL.");
    }

    name_str_p = json_string_value(name_p);

    if (TIME_RANGE_ERROR_TYPE_NONE != TIME_RANGE_PMGR_GetTimeRangeEntryByName((UI8_T *)name_str_p, &entry))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                "time-range.GetTimeRangeEntryByNameError", "No such time range.");
    }

    CGI_MODULE_TIME_RANGE_GetOne(&entry, result_p);
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete a time range.
 *
 * @param id (required, string) time range name
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_TIME_RANGE_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p;
    const char* name_str_p;
    TIME_RANGE_TYPE_ENTRY_T entry;

    memset(&entry, 0, sizeof(entry));
    name_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (NULL == name_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Time range name is NULL.");
    }

    name_str_p = json_string_value(name_p);

    if (TIME_RANGE_ERROR_TYPE_NONE == TIME_RANGE_PMGR_GetTimeRangeEntryByName((UI8_T *)name_str_p, &entry))
    {
        if (TIME_RANGE_ERROR_TYPE_NONE != TIME_RANGE_PMGR_DeleteTimeRangeEntry((UI8_T *)name_str_p))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "time-range.DeleteTimeRangeEntryError", "Failed to delete time range.");
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_TIME_RANGE_ID_Delete

/**----------------------------------------------------------------------
 * This API is used to delete a time range absolute.
 *
 * @param id (required, string) time range name
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_TIME_RANGE_ABS_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p;
    const char* name_str_p;
    TIME_RANGE_TYPE_ENTRY_T entry;

    memset(&entry, 0, sizeof(entry));
    name_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (NULL == name_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Time range name is NULL.");
    }

    name_str_p = json_string_value(name_p);

    if (TIME_RANGE_ERROR_TYPE_NONE == TIME_RANGE_PMGR_GetTimeRangeEntryByName((UI8_T *)name_str_p, &entry))
    {
        if (TIME_RANGE_ERROR_TYPE_NONE != TIME_RANGE_PMGR_DestroyTimeRangeAbsolute((UI8_T *)name_str_p))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "time-range.DestroyTimeRangeAbsoluteError", "Failed to delete time range absolute.");
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_TIME_RANGE_ABS_Delete

/**----------------------------------------------------------------------
 * This API is used to delete a time range periodic.
 *
 * @param id (required, string) time range name
 * @param time (required, string) periodic time
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_TIME_RANGE_PERIODIC_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p;
    json_t  *id_p;
    const char* name_str_p;
    char* id_str_p;
    char *start_day_p, *start_hour_p, *start_minute_p;
    char *to_p;
    char *end_day_p, *end_hour_p, *end_minute_p;
    char *save_str_addr = NULL;
    TIME_RANGE_TYPE_ENTRY_T entry;
    TIME_RANGE_TYPE_PERIODIC_TIME_T period;

    memset(&entry, 0, sizeof(entry));
    memset(&period, 0, sizeof(period));
    name_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    id_p = CGI_REQUEST_GetParamsValue(http_request, "time");

    if (NULL == name_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Time range name is NULL.");
    }

    if (NULL == id_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Time range ID is NULL.");
    }

    name_str_p = json_string_value(name_p);
    id_str_p = (char *) json_string_value(id_p);

    if (TIME_RANGE_ERROR_TYPE_NONE != TIME_RANGE_PMGR_GetTimeRangeEntryByName((UI8_T *)name_str_p, &entry))
    {
        return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
    }

    start_day_p = strtok_r(id_str_p, "_", &save_str_addr);
    start_hour_p = strtok_r(NULL, "_", &save_str_addr);
    start_minute_p = strtok_r(NULL, "_", &save_str_addr);
    to_p = strtok_r(NULL, "_", &save_str_addr);
    end_day_p = strtok_r(NULL, "_", &save_str_addr);
    end_hour_p = strtok_r(NULL, "_", &save_str_addr);
    end_minute_p = strtok_r(NULL, "_", &save_str_addr);


    if (TRUE != CGI_MODULE_TIME_RANGE_ConvertWeekDay((char *)start_day_p, &period.start_type_of_wk))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid Periodic start day");
    }

    if (TRUE != CGI_MODULE_TIME_RANGE_ConvertWeekDay((char *)end_day_p, &period.end_type_of_wk))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid Periodic end day");
    }

    period.start_hour = atoi(start_hour_p);
    period.start_minute = atoi(start_minute_p);
    period.start_second = 0;

    period.end_hour = atoi(end_hour_p);
    period.end_minute = atoi(end_minute_p);
    period.end_second = 0;

    if (TIME_RANGE_ERROR_TYPE_NONE != TIME_RANGE_PMGR_DestroyTimeRangePeriodic((UI8_T *)name_str_p, &period))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                "time-range.DestroyTimeRangePeriodicError", "Failed to delete time range periodic.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_TIME_RANGE_PERIODIC_ID_Delete


static void CGI_MODULE_TIME_RANGE_GetOne(TIME_RANGE_TYPE_ENTRY_T *entry_p, json_t *time_obj_p)
{
    json_t  *periodic_p = json_array();
    char day[10][10]   = {
        "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday",
        "Daily", "Weekdays", "Weekend"};
    int idx = 0;

    json_object_set_new(time_obj_p, "name", json_string(entry_p->name));
    json_object_set_new(time_obj_p, "active", json_boolean(entry_p->active));

    if (TRUE == entry_p->check_end_absolute)
    {
        json_t *absolute_obj_p = json_object();
        json_t *absolute_end_obj_p = json_object();

        if (TRUE == entry_p->check_start_absolute)
        {
            json_t *absolute_start_obj_p = json_object();

            json_object_set_new(absolute_start_obj_p, "year", json_integer(entry_p->start_time.year));
            json_object_set_new(absolute_start_obj_p, "month", json_integer(entry_p->start_time.month));
            json_object_set_new(absolute_start_obj_p, "day", json_integer(entry_p->start_time.day));
            json_object_set_new(absolute_start_obj_p, "hour", json_integer(entry_p->start_time.hour));
            json_object_set_new(absolute_start_obj_p, "minute", json_integer(entry_p->start_time.minute));

            json_object_set_new(absolute_obj_p, "start", absolute_start_obj_p);
        }

        json_object_set_new(absolute_end_obj_p, "year", json_integer(entry_p->end_time.year));
        json_object_set_new(absolute_end_obj_p, "month", json_integer(entry_p->end_time.month));
        json_object_set_new(absolute_end_obj_p, "day", json_integer(entry_p->end_time.day));
        json_object_set_new(absolute_end_obj_p, "hour", json_integer(entry_p->end_time.hour));
        json_object_set_new(absolute_end_obj_p, "minute", json_integer(entry_p->end_time.minute));

        json_object_set_new(absolute_obj_p, "end", absolute_end_obj_p);
        json_object_set_new(time_obj_p, "absolute", absolute_obj_p);
    } //absolute

    json_object_set_new(time_obj_p, "periodic", periodic_p);

    for (idx = 0; idx < SYS_ADPT_TIME_RANGE_MAX_NBR_OF_PERIODIC_ENTRY; idx++)
    {
        if (TRUE == entry_p->check_periodic[idx])
        {
            json_t *periodic_obj_p = json_object();
            json_t *periodic_start_obj_p = json_object();
            json_t *periodic_end_obj_p = json_object();

            json_object_set_new(periodic_start_obj_p, "day", json_string(day[entry_p->periodic_daily_time[idx].start_type_of_wk]));
            json_object_set_new(periodic_start_obj_p, "hour", json_integer(entry_p->periodic_daily_time[idx].start_hour));
            json_object_set_new(periodic_start_obj_p, "minute", json_integer(entry_p->periodic_daily_time[idx].start_minute));

            json_object_set_new(periodic_end_obj_p, "day", json_string(day[entry_p->periodic_daily_time[idx].end_type_of_wk]));
            json_object_set_new(periodic_end_obj_p, "hour", json_integer(entry_p->periodic_daily_time[idx].end_hour));
            json_object_set_new(periodic_end_obj_p, "minute", json_integer(entry_p->periodic_daily_time[idx].end_minute));

            json_object_set_new(periodic_obj_p, "start", periodic_start_obj_p);
            json_object_set_new(periodic_obj_p, "end", periodic_end_obj_p);
            json_array_append_new(periodic_p, periodic_obj_p);
        }
    }
}

static BOOL_T CGI_MODULE_TIME_RANGE_ConvertWeekDay(char *weekday_p, UI8_T *val_p)
{
    if (0 == strncmp(weekday_p, "daily", strlen("daily")))
    {
        *val_p = TIME_RANGE_TYPE_WEEK_DAILY;
    }
    else if (0 == strncmp(weekday_p, "sunday", strlen("sunday")))
    {
        *val_p = TIME_RANGE_TYPE_WEEK_SUNDAY;
    }
    else if (0 == strncmp(weekday_p, "monday", strlen("monday")))
    {
        *val_p = TIME_RANGE_TYPE_WEEK_MONDAY;
    }
    else if (0 == strncmp(weekday_p, "tuesday", strlen("tuesday")))
    {
        *val_p = TIME_RANGE_TYPE_WEEK_TUESDAY;
    }
    else if (0 == strncmp(weekday_p, "wednesday", strlen("wednesday")))
    {
        *val_p = TIME_RANGE_TYPE_WEEK_WEDNESDAY;
    }
    else if (0 == strncmp(weekday_p, "thursday", strlen("thursday")))
    {
        *val_p = TIME_RANGE_TYPE_WEEK_THURSDAY;
    }
    else if (0 == strncmp(weekday_p, "friday", strlen("friday")))
    {
        *val_p = TIME_RANGE_TYPE_WEEK_FRIDAY;
    }
    else if (0 == strncmp(weekday_p, "saturday", strlen("saturday")))
    {
        *val_p = TIME_RANGE_TYPE_WEEK_SATURDAY;
    }
    else if (0 == strncmp(weekday_p, "weekdays", strlen("weekdays")))
    {
        *val_p = TIME_RANGE_TYPE_WEEK_WEEKDAYS;
    }
    else if (0 == strncmp(weekday_p, "weekend", strlen("weekend")))
    {
        *val_p = TIME_RANGE_TYPE_WEEK_WEEKEND;
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

void CGI_MODULE_TIME_RANGE_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler = CGI_MODULE_TIME_RANGE_Read;
    handlers.create_handler = CGI_MODULE_TIME_RANGE_Create;
    CGI_MAIN_Register("/api/v1/time-range", &handlers, 0);

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_TIME_RANGE_ID_Read;
        handlers.delete_handler = CGI_MODULE_TIME_RANGE_ID_Delete;
        CGI_MAIN_Register("/api/v1/time-range/{id}", &handlers, 0);
    }

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.delete_handler = CGI_MODULE_TIME_RANGE_ABS_Delete;
        CGI_MAIN_Register("/api/v1/time-range/{id}/absolute", &handlers, 0);
    }

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.delete_handler = CGI_MODULE_TIME_RANGE_PERIODIC_ID_Delete;
        CGI_MAIN_Register("/api/v1/time-range/{id}/periodic/{time}", &handlers, 0);
    }
}

static void CGI_MODULE_TIME_RANGE_Init()
{
    CGI_MODULE_TIME_RANGE_RegisterHandlers();
}

