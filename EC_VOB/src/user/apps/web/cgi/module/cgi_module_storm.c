#include "cgi_request.h"
#include "cgi_auth.h"
#include "cgi_util.h"
#include "swctrl_pmgr.h"
#include "swctrl_pom.h"

/**----------------------------------------------------------------------
 * This API is used to set storm control.
 *
 * @param ucastRate (required, number)    unknown unicast rate in pps
 * @param ucastStatus (required, boolean) unknown unicast storm control status
 * @param mcastRate (required, number)    multicast rate in pps
 * @param mcastStatus (required, boolean) multicast storm control status
 * @param bcastRate (required, number)    broadcast rate in pps
 * @param bcastStatus (required, boolean) broadcast storm control status
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_STORM_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *ucast_rate_p;
    json_t  *ucast_status_p;
    json_t  *mcast_rate_p;
    json_t  *mcast_status_p;
    json_t  *bcast_rate_p;
    json_t  *bcast_status_p;
    UI32_T  ucast_rate = 0;
    UI32_T  mcast_rate = 0;
    UI32_T  bcast_rate = 0;
    UI32_T  ifindex = 0;
    BOOL_T  ucast_status = FALSE;
    BOOL_T  mcast_status = FALSE;
    BOOL_T  bcast_status = FALSE;

    ucast_status_p = CGI_REQUEST_GetBodyValue(http_request, "ucastStatus");

    if (ucast_status_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ucastStatus.");
    }
    ucast_status = json_boolean_value(ucast_status_p);

    ucast_rate_p = CGI_REQUEST_GetBodyValue(http_request, "ucastRate");

    if (ucast_rate_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ucastRate.");
    }
    ucast_rate = json_integer_value(ucast_rate_p);

    mcast_status_p = CGI_REQUEST_GetBodyValue(http_request, "mcastStatus");

    if (mcast_status_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get mcastStatus.");
    }
    mcast_status = json_boolean_value(mcast_status_p);

    mcast_rate_p = CGI_REQUEST_GetBodyValue(http_request, "mcastRate");

    if (mcast_rate_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get mcastRate.");
    }
    mcast_rate = json_integer_value(mcast_rate_p);

    bcast_status_p = CGI_REQUEST_GetBodyValue(http_request, "bcastStatus");

    if (bcast_status_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get bcastStatus.");
    }
    bcast_status = json_boolean_value(bcast_status_p);

    bcast_rate_p = CGI_REQUEST_GetBodyValue(http_request, "bcastRate");

    if (bcast_rate_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get bcastRate.");
    }
    bcast_rate = json_integer_value(bcast_rate_p);

    while (SWCTRL_POM_GetNextLogicalPort(&ifindex) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)
        if (TRUE == ucast_status)
        {
            if (TRUE != SWCTRL_PMGR_SetUnknownUnicastStormStatus(ifindex, VAL_unkucastStormStatus_enabled))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "ucast.UnknownUStormControlEnableError", "Failed.");
            }

            if (TRUE != SWCTRL_PMGR_SetUnknownUStormControlRateLimit(ifindex, SYS_DFLT_UNKUSTORM_TYPE, ucast_rate))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "ucast.UnknownUStormControlRateLimitError", "Failed.");
            }
        }
        else
        {
            if (TRUE != SWCTRL_PMGR_SetUnknownUnicastStormStatus(ifindex, VAL_unkucastStormStatus_disabled))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "ucast.UnknownUStormControlDisableError", "Failed.");
            }
        }
#endif  /* #if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM) */

#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_MSTORM)
        if (TRUE == mcast_status)
        {
            if (TRUE != SWCTRL_PMGR_SetMulticastStormStatus(ifindex, VAL_mcastStormStatus_enabled))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "mcast.MulticastStormEnableError", "Failed.");
            }

            if (TRUE != SWCTRL_PMGR_SetMStormControlRateLimit(ifindex, SYS_DFLT_MSTORM_TYPE, mcast_rate))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "mcast.MStormControlRateLimitError", "Failed.");
            }
        }
        else
        {
            if (TRUE != SWCTRL_PMGR_SetMulticastStormStatus(ifindex, VAL_mcastStormStatus_disabled))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "mcast.MulticastStormDisableError", "Failed.");
            }
        }
#endif  /* #if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_MSTORM) */

#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_BSTORM)
        if (TRUE == bcast_status)
        {
            if (TRUE != SWCTRL_PMGR_SetBroadcastStormStatus(ifindex, VAL_bcastStormStatus_enabled))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "bcast.BroadcastStormEnableError", "Failed.");
            }

            if (TRUE != SWCTRL_PMGR_SetBStormControlRateLimit(ifindex, SYS_DFLT_BSTORM_TYPE, bcast_rate))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "bcast.BStormControlRateLimitError", "Failed.");
            }
        }
        else
        {
            if (TRUE != SWCTRL_PMGR_SetBroadcastStormStatus(ifindex, VAL_bcastStormStatus_disabled))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "bcast.BroadcastStormDisableError", "Failed.");
            }
        }
#endif  /* #if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_BSTORM) */
    } //SWCTRL_POM_GetNextLogicalPort

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to read sFlow setting.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_STORM_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    SWCTRL_UnknownUcastStormEntry_T unkucast_storm_entry;
    SWCTRL_McastStormEntry_T mcast_storm_entry;
    SWCTRL_BcastStormEntry_T bcast_storm_entry;
    UI32_T ifindex = 0;

    memset(&unkucast_storm_entry, 0, sizeof(unkucast_storm_entry));
    memset(&mcast_storm_entry, 0, sizeof(mcast_storm_entry));
    memset(&bcast_storm_entry, 0, sizeof(bcast_storm_entry));

    if (SWCTRL_POM_GetNextLogicalPort(&ifindex) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)
        unkucast_storm_entry.unknown_ucast_storm_ifindex = ifindex;

        if(TRUE == SWCTRL_POM_GetUnkucastStormEntry(&unkucast_storm_entry))
        {
            json_object_set_new(result_p, "ucastRate", json_integer(unkucast_storm_entry.unknown_ucast_storm_pkt_rate));
            json_object_set_new(result_p, "ucastStatus",
                    json_boolean(unkucast_storm_entry.unknown_ucast_storm_status == VAL_unkucastStormStatus_enabled));
        }
#endif  /* #if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM) */

#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_MSTORM)
        mcast_storm_entry.mcast_storm_ifindex = ifindex;

        if(TRUE == SWCTRL_POM_GetMcastStormEntry(&mcast_storm_entry))
        {
            json_object_set_new(result_p, "mcastRate", json_integer(mcast_storm_entry.mcast_storm_pkt_rate));
            json_object_set_new(result_p, "mcastStatus",
                    json_boolean(mcast_storm_entry.mcast_storm_status == VAL_mcastStormStatus_enabled));
        }
#endif  /* #if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_MSTORM) */

#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_BSTORM)
        bcast_storm_entry.bcast_storm_ifindex = ifindex;

        if(TRUE == SWCTRL_POM_GetBcastStormEntry(&bcast_storm_entry))
        {
            json_object_set_new(result_p, "bcastRate", json_integer(bcast_storm_entry.bcast_storm_pkt_rate));
            json_object_set_new(result_p, "bcastStatus",
                    json_boolean(bcast_storm_entry.bcast_storm_status == VAL_bcastStormStatus_enabled));
        }
#endif  /* #if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_BSTORM) */
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

void CGI_MODULE_STORM_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler = CGI_MODULE_STORM_Read;
    handlers.create_handler = CGI_MODULE_STORM_Create;
    CGI_MAIN_Register("/api/v1/storm", &handlers, 0);
}

static void CGI_MODULE_STORM_Init()
{
    CGI_MODULE_STORM_RegisterHandlers();
}
