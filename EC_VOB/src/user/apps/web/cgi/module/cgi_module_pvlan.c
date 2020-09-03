#include "cgi_request.h"
#include "cgi_auth.h"
#include "cgi_util.h"
#include "swctrl.h"

#if (SYS_CPNT_EXCLUDED_VLAN == TRUE)
#define CGI_MODULE_PVLAN_EXCLUDE_VLAN_MASK 0xFFF
#endif /* #if (SYS_CPNT_EXCLUDED_VLAN == TRUE) */

static BOOL_T CGI_MODULE_PVLAN_ParseUplinks(json_t *uplinks_p, UI8_T *uplink_port_p);
static BOOL_T CGI_MODULE_PVLAN_ParseDownlinks(json_t *downlinks_p, UI8_T *downlink_port_p);

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)
static BOOL_T CGI_MODULE_PVLAN_GetSession(UI32_T session_id, json_t *entry_obj_p);
static BOOL_T CGI_MODULE_PVLAN_SetUplinkPorts(UI32_T session_id, UI32_T mgmt_port);
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION) */
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */

#if (SYS_CPNT_EXCLUDED_VLAN == TRUE)
static BOOL_T CGI_MODULE_PVLAN_GetExcludeVlanSession(UI32_T session_id, json_t *entry_obj_p);
static BOOL_T CGI_MODULE_PVLAN_ParseExcludeVlan(json_t *vlans_p,
        SWCTRL_VlanInfo_T vlan_ar[SYS_ADPT_EXCLUDED_VLAN_MAX_NBRS_OF_VLAN_TABLE]);
#endif /* #if (SYS_CPNT_EXCLUDED_VLAN == TRUE) */

/**----------------------------------------------------------------------
 * This API is used to read PVLAN.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PVLAN_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)
    UI32_T status = 0, mode = 0;

    SWCTRL_POM_GetPrivateVlanStatus(&status);
    json_object_set_new(result_p, "status", json_boolean((status == VAL_privateVlanStatus_enabled)));

    SWCTRL_POM_GetPrivateVlanUplinkToUplinkStatus(&mode);

    if (SYS_DFLT_TRAFFIC_SEG_UPLINK_TO_UPLINK_MODE_BLOCKING == mode)
    {
        json_object_set_new(result_p, "mode", json_string("blocking"));
    }
    else if (SYS_DFLT_TRAFFIC_SEG_UPLINK_TO_UPLINK_MODE_FORWARDING == mode)
    {
        json_object_set_new(result_p, "mode", json_string("forwarding"));
    }
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION) */
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to PVLAN.
 *
 * @param status (Optional, boolean) Traffic segmentation status.
 * @param mode   (Optional, string) Option: blocking forwarding
 * @param sessionId (Optional, number) Option: session ID
 * @param mgmtPort (Optional, number) Option: port connect to controller
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PVLAN_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)
    json_t  *status_p;
    json_t  *mode_p;
    json_t  *session_p;
    json_t  *mgmt_p;
    const char*   mode_str_p;
    int session_id = 0;
    int mgmt_port = 0;

    status_p = CGI_REQUEST_GetBodyValue(http_request, "status");
    session_p = CGI_REQUEST_GetBodyValue(http_request, "sessionId");
    mgmt_p = CGI_REQUEST_GetBodyValue(http_request, "mgmtPort");

    if (NULL != session_p)
    {
        session_id = json_integer_value(session_p);
    }

    if (NULL != status_p)
    {
        if (TRUE != json_is_boolean(status_p))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
        }

        if (TRUE == json_boolean_value(status_p))
        {
            if (TRUE != SWCTRL_PMGR_EnablePrivateVlan())
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "pvlan.enablePrivateVlanError", "Failed to enable PVLAN status.");
            }

            if (0 != session_id)
            {
                if (NULL != mgmt_p)
                {
                    mgmt_port = json_integer_value(mgmt_p);
                }

                if (TRUE != CGI_MODULE_PVLAN_SetUplinkPorts(session_id, mgmt_port))
                {
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                            "pvlan.setUplinkPortsError", "Failed to set PVLAN uplink ports.");
                }
            }
        }
        else
        {
            if (TRUE != SWCTRL_PMGR_DisablePrivateVlan())
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "pvlan.disablePrivateVlanError", "Failed to disable PVLAN status.");
            }

            if (0 != session_id)
            {
                SWCTRL_PMGR_DestroyPrivateVlanSession(session_id, TRUE, TRUE);
            }
        }
    } //if (NULL != status_p)

    mode_p = CGI_REQUEST_GetBodyValue(http_request, "mode");

    if (NULL != mode_p)
    {
        mode_str_p = json_string_value(mode_p);

        if (0 == strncmp(mode_str_p, "blocking", strlen("blocking")))
        {
            if (TRUE != SWCTRL_PMGR_EnablePrivateVlanUplinkToUplinkBlockingMode())
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "pvlan.enablePrivateVlanUplinkToUplinkBlockingModeError", "Failed to enable blocking mode.");
            }
        }
        else if (0 == strncmp(mode_str_p, "forwarding", strlen("forwarding")))
        {
            if (TRUE != SWCTRL_PMGR_DisablePrivateVlanUplinkToUplinkBlockingMode())
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "pvlan.disablePrivateVlanUplinkToUplinkBlockingModeError", "Failed to disable blocking mode.");
            }
        }
        else
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid mode.");
        }
    } //if (NULL != mode_p)
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION) */
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_PVLAN_Update

/**----------------------------------------------------------------------
 * This API is used to get PVLAN sessions.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PVLAN_Sessions_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *sessions_p = json_array();
    UI32_T session_id = 0;

    json_object_set_new(result_p, "sessions", sessions_p);

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)
    for (session_id = 1; session_id <= SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS; session_id++)
    {
        json_t  *entry_obj_p = json_object();

        if (TRUE == CGI_MODULE_PVLAN_GetSession(session_id, entry_obj_p))
        {
            json_array_append_new(sessions_p, entry_obj_p);
        }
    } //for (session_id = 1; session_id <= SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS; session_id++)
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION) */
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */

#if (SYS_CPNT_EXCLUDED_VLAN == TRUE)
    for (session_id = 1; session_id <= SYS_ADPT_EXCLUDED_VLAN_MAX_NBRS_OF_SESSION; session_id++)
    {
        json_t  *entry_obj_p = json_object();

        if (TRUE == CGI_MODULE_PVLAN_GetExcludeVlanSession(session_id, entry_obj_p))
        {
            json_array_append_new(sessions_p, entry_obj_p);
        }
    }
#endif /* #if (SYS_CPNT_EXCLUDED_VLAN == TRUE) */

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_PVLAN_Sessions_Read

/**----------------------------------------------------------------------
 * This API is used to PVLAN.
 *
 * @param sessionId (required, number) Session id
 * @param uplinks   (Optional, array) Interface ID
 * @param downlinks (Optional, array) Interface ID
 * @param excludeVlans (Optional, array) excluded VLAN
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PVLAN_Sessions_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *session_p;
    json_t  *uplinks_p;
    json_t  *downlinks_p;
    json_t  *vlans_p;
    UI8_T  uplink_port_ar[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    UI8_T  downlink_port_ar[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    UI32_T session_id = 0;
    int downlink_num = 0, vlan_num = 0;
    int max_session_id = SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS;
#if (SYS_CPNT_EXCLUDED_VLAN == TRUE)
    SWCTRL_VlanInfo_T vlan_ar[SYS_ADPT_EXCLUDED_VLAN_MAX_NBRS_OF_VLAN_TABLE];
    int idx = 0;

    max_session_id += SYS_ADPT_EXCLUDED_VLAN_MAX_NBRS_OF_SESSION;
#endif /* #if (SYS_CPNT_EXCLUDED_VLAN == TRUE) */

    session_p = CGI_REQUEST_GetBodyValue(http_request, "sessionId");

    if (NULL == session_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "sessionId is NULL.");
    }

    session_id = json_integer_value(session_p);

    if ((session_id < 1) || (session_id > max_session_id))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "sessionId is out of range.");
    }

    uplinks_p = CGI_REQUEST_GetBodyValue(http_request, "uplinks");
    downlinks_p = CGI_REQUEST_GetBodyValue(http_request, "downlinks");
    vlans_p = CGI_REQUEST_GetBodyValue(http_request, "excludeVlans");

    if (NULL != uplinks_p)
    {
        if (TRUE != CGI_MODULE_PVLAN_ParseUplinks(uplinks_p, uplink_port_ar))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Parsing error of uplinks.");
        }
    } //if (NULL != uplinks_p)

    if (NULL != downlinks_p)
    {
        downlink_num = json_array_size(downlinks_p);

        if ((0 == downlink_num) && (SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS < session_id))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Must specify downlinks for excluded-vlan.");
        }

        if (TRUE != CGI_MODULE_PVLAN_ParseDownlinks(downlinks_p, downlink_port_ar))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Parsing error of downlinks.");
        }
    } //if (NULL != downlinks_p)

    if (NULL != vlans_p)
    {
        if (SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS >= session_id)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Session ID must larger than 4 for excluded VLAN.");
        }

        vlan_num = json_array_size(vlans_p);

#if (SYS_CPNT_EXCLUDED_VLAN == TRUE)
        memset(vlan_ar, 0, sizeof(vlan_ar));

        if (TRUE != CGI_MODULE_PVLAN_ParseExcludeVlan(vlans_p, vlan_ar))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Parsing error of excluded VLAN.");
        }
#endif /* #if (SYS_CPNT_EXCLUDED_VLAN == TRUE) */
    } //if (NULL != vlans_p)

    if (SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS < session_id)
    {
#if (SYS_CPNT_EXCLUDED_VLAN == TRUE)
        session_id -= SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS;

        if ((NULL == vlans_p) || (0 == vlan_num))
        {
            if (TRUE != SWCTRL_PMGR_ExcludedVlanAdd(session_id, uplink_port_ar, downlink_port_ar, 0, 0))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "pvlan.ExcludedVlanAddError", "Failed to set excluded-vlan.");
            }
        }
        else
        {
            for (idx = 0; idx < vlan_num; idx ++)
            {
                if (TRUE != SWCTRL_PMGR_ExcludedVlanAdd(session_id, uplink_port_ar, downlink_port_ar,
                        vlan_ar[idx].vlan, vlan_ar[idx].vlan_mask))
                {
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                            "pvlan.ExcludedVlanAddError", "Failed to set excluded-vlan.");
                }
            } //for (idx = 0; idx < vlan_num; idx ++)
        }
#endif /* #if (SYS_CPNT_EXCLUDED_VLAN == TRUE) */
    }
    else
    {
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)
        if (TRUE != SWCTRL_PMGR_SetPrivateVlanBySessionId(session_id, uplink_port_ar, downlink_port_ar))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "pvlan.setPrivateVlanBySessionIdError", "Failed to set PVLAN uplinks/downlinks.");
        }
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION) */
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_PVLAN_Sessions_Create

/**----------------------------------------------------------------------
 * This API is used to read a session.
 *
 * @param sessionId (required, number) Unique session ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PVLAN_Sessions_ID_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

    json_t  *session_p;
    UI32_T  session_id = 0;

    session_p = CGI_REQUEST_GetParamsValue(http_request, "sessionId");

    if (session_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get sessionId.");
    }

    session_id = json_integer_value(session_p);

    if (SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS < session_id)
    {
#if (SYS_CPNT_EXCLUDED_VLAN == TRUE)
        session_id -= SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS;
        CGI_MODULE_PVLAN_GetExcludeVlanSession(session_id, result_p);
#endif /* #if (SYS_CPNT_EXCLUDED_VLAN == TRUE) */
    }
    else
    {
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)
        CGI_MODULE_PVLAN_GetSession(session_id, result_p);
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION) */
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to PVLAN.
 *
 * @param sessionId (required, number) Session id
 * @param uplinks   (Optional, array) Interface ID
 * @param downlinks (Optional, array) Interface ID
 * @param status    (required, boolean) True to add member; false to delete
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PVLAN_Sessions_ID_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *session_p;
    json_t  *status_p;
    json_t  *uplinks_p;
    json_t  *downlinks_p;
    json_t  *vlans_p;
    UI8_T  uplink_port_ar[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    UI8_T  downlink_port_ar[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    UI32_T  session_id = 0;
    int vlan_num = 0;
    int max_session_id = SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS;
#if (SYS_CPNT_EXCLUDED_VLAN == TRUE)
    SWCTRL_VlanInfo_T vlan_ar[SYS_ADPT_EXCLUDED_VLAN_MAX_NBRS_OF_VLAN_TABLE];
    int idx = 0;

    max_session_id += SYS_ADPT_EXCLUDED_VLAN_MAX_NBRS_OF_SESSION;
#endif /* #if (SYS_CPNT_EXCLUDED_VLAN == TRUE) */

    session_p = CGI_REQUEST_GetParamsValue(http_request, "sessionId");

    if (session_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get sessionId.");
    }

    session_id = json_integer_value(session_p);

    if ((session_id < 1) || (session_id > max_session_id))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "sessionId is out of range.");
    }

    status_p = CGI_REQUEST_GetBodyValue(http_request, "status");

    if (NULL == status_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get status.");
    }

    if (TRUE != json_is_boolean(status_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
    }

    uplinks_p = CGI_REQUEST_GetBodyValue(http_request, "uplinks");
    downlinks_p = CGI_REQUEST_GetBodyValue(http_request, "downlinks");
    vlans_p = CGI_REQUEST_GetBodyValue(http_request, "excludeVlans");

    if (NULL != uplinks_p)
    {
        if (TRUE != CGI_MODULE_PVLAN_ParseUplinks(uplinks_p, uplink_port_ar))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Parsing error of uplinks.");
        }
    } //if (NULL != uplinks_p)

    if (NULL != downlinks_p)
    {
        if (TRUE != CGI_MODULE_PVLAN_ParseDownlinks(downlinks_p, downlink_port_ar))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Parsing error of downlinks.");
        }
    } //if (NULL != downlinks_p)

    if (NULL != vlans_p)
    {
        if (SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS >= session_id)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Session ID must larger than 4 for excluded VLAN.");
        }

        vlan_num = json_array_size(vlans_p);

#if (SYS_CPNT_EXCLUDED_VLAN == TRUE)
        memset(vlan_ar, 0, sizeof(vlan_ar));

        if (TRUE != CGI_MODULE_PVLAN_ParseExcludeVlan(vlans_p, vlan_ar))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Parsing error of excluded VLAN.");
        }
#endif /* #if (SYS_CPNT_EXCLUDED_VLAN == TRUE) */
    } //if (NULL != vlans_p)

    if (SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS < session_id)
    {
#if (SYS_CPNT_EXCLUDED_VLAN == TRUE)
        session_id -= SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS;

        if (TRUE == json_boolean_value(status_p))
        {
            if ((NULL == vlans_p) || (0 == vlan_num))
            {
                if (TRUE != SWCTRL_PMGR_ExcludedVlanAdd(session_id, uplink_port_ar, downlink_port_ar, 0, 0))
                {
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                            "pvlan.ExcludedVlanAddError", "Failed to set excluded-vlan.");
                }
            }
            else
            {
                for (idx = 0; idx < vlan_num; idx ++)
                {
                    if (TRUE != SWCTRL_PMGR_ExcludedVlanAdd(session_id, uplink_port_ar, downlink_port_ar,
                            vlan_ar[idx].vlan, vlan_ar[idx].vlan_mask))
                    {
                        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                                "pvlan.ExcludedVlanAddError", "Failed to set excluded-vlan.");
                    }
                } //for (idx = 0; idx < vlan_num; idx ++)
            }
        }
        else
        {
            if ((NULL == vlans_p) || (0 == vlan_num))
            {
                if (TRUE != SWCTRL_PMGR_ExcludedVlanDelete(session_id, uplink_port_ar, downlink_port_ar, 0, 0))
                {
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                            "pvlan.ExcludedVlanDeleteError", "Failed to delete excluded-vlan.");
                }
            }
            else
            {
                for (idx = 0; idx < vlan_num; idx ++)
                {
                    if (TRUE != SWCTRL_PMGR_ExcludedVlanDelete(session_id, uplink_port_ar, downlink_port_ar,
                            vlan_ar[idx].vlan, vlan_ar[idx].vlan_mask))
                    {
                        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                                "pvlan.ExcludedVlanDeleteError", "Failed to delete excluded-vlan.");
                    }
                } //for (idx = 0; idx < vlan_num; idx ++)
            }
        }
#endif /* #if (SYS_CPNT_EXCLUDED_VLAN == TRUE) */
    }
    else
    {
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)
        if (TRUE == json_boolean_value(status_p))
        {
            //before add downlinks, should remove ports from uplinks
            //SWCTRL_PMGR_DeletePrivateVlanPortlistBySessionId(session_id, downlink_port_ar, uplink_port_ar);

            if (TRUE != SWCTRL_PMGR_SetPrivateVlanBySessionId(session_id, uplink_port_ar, downlink_port_ar))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "pvlan.setPrivateVlanBySessionIdError", "Failed to set PVLAN uplinks/downlinks.");
            }
        }
        else
        {
            if (TRUE != SWCTRL_PMGR_DeletePrivateVlanPortlistBySessionId(session_id, uplink_port_ar, downlink_port_ar))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "pvlan.DeletePrivateVlanPortlistBySessionIdError", "Failed to delete PVLAN uplinks/downlinks.");
            }

            //after remove downlinks, should add ports to uplinks (handle by switch)
            //SWCTRL_PMGR_SetPrivateVlanBySessionId(session_id, downlink_port_ar, uplink_port_ar);
        }
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION) */
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_PVLAN_Sessions_ID_Update

/**----------------------------------------------------------------------
 * This API is used to delete a session.
 *
 * @param sessionId (required, number) session ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_PVLAN_Sessions_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *session_p;
    UI32_T  session_id = 0;
    UI8_T  uplink_port_ar[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    UI8_T  downlink_port_ar[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

    session_p = CGI_REQUEST_GetParamsValue(http_request, "sessionId");

    if (session_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get sessionId.");
    }

    session_id = json_integer_value(session_p);

    if (SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS < session_id)
    {
#if (SYS_CPNT_EXCLUDED_VLAN == TRUE)
        SWCTRL_VlanInfo_T vlan_ar[SYS_ADPT_EXCLUDED_VLAN_MAX_NBRS_OF_VLAN_TABLE];

        memset(vlan_ar, 0, sizeof(vlan_ar));
        session_id -= SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS;

        if (TRUE == SWCTRL_POM_GetExcludedVlanBySessionId(session_id, uplink_port_ar, downlink_port_ar, vlan_ar))
        {
            memset(uplink_port_ar, 0, sizeof(uplink_port_ar));
            memset(downlink_port_ar, 0, sizeof(downlink_port_ar));

            if (TRUE != SWCTRL_PMGR_ExcludedVlanDelete(session_id, uplink_port_ar, downlink_port_ar, 0, 0))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "pvlan.ExcludedVlanDeleteError", "Failed to remove excluded-vlan session.");
            }
        }
#endif /* #if (SYS_CPNT_EXCLUDED_VLAN == TRUE) */
    }
    else
    {
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)
        if (TRUE == SWCTRL_POM_GetPrivateVlanBySessionId(session_id, uplink_port_ar, downlink_port_ar))
        {
            if (TRUE != SWCTRL_PMGR_DestroyPrivateVlanSession(session_id, TRUE, TRUE))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "pvlan.destroyPrivateVlanSessionError", "Failed to remove PVLAN session.");
            }
        }
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION) */
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

static BOOL_T CGI_MODULE_PVLAN_ParseUplinks(json_t *uplinks_p, UI8_T *uplink_port_p)
{
    json_t *uplink_p;
    const char* port_str_p;
    UI32_T lport = 0;
    int uplink_num = json_array_size(uplinks_p);
    int idx = 0;

    if (0 == uplink_num)
    {
        return TRUE;
    }

    for (idx = 0; idx < uplink_num; idx ++)
    {
        uplink_p = json_array_get(uplinks_p, idx);

        if (NULL != uplink_p)
        {
            port_str_p = json_string_value(uplink_p);

            if (TRUE != CGI_UTIL_InterfaceIdToLport((char *)port_str_p, &lport))
            {
                return FALSE;
            }

            uplink_port_p [(UI32_T)((lport-1)/8)] |=   (1 << (7 - ((lport-1)%8)));
            //downlink_port_ar[(UI32_T)((lport-1)/8)] &= (~(1 << (7 - ((lport-1)%8))));
        }
    }

    return TRUE;
} //CGI_MODULE_PVLAN_ParseUplinks

static BOOL_T CGI_MODULE_PVLAN_ParseDownlinks(json_t *downlinks_p, UI8_T *downlink_port_p)
{
    json_t *downlink_p;
    const char* port_str_p;
    UI32_T lport = 0;
    int downlink_num = json_array_size(downlinks_p);
    int idx = 0;

    if (0 == downlink_num)
    {
        return TRUE;
    }

    for (idx = 0; idx < downlink_num; idx ++)
    {
        downlink_p = json_array_get(downlinks_p, idx);

        if (NULL != downlink_p)
        {
            port_str_p = json_string_value(downlink_p);

            if (NULL == port_str_p)
            {
                return FALSE;
            }

            if (TRUE != CGI_UTIL_InterfaceIdToLport((char *)port_str_p, &lport))
            {
                return FALSE;
            }

            //uplink_port_ar  [(UI32_T)((lport-1)/8)] &= (~(1 << (7 - ((lport-1)%8))));
            downlink_port_p[(UI32_T)((lport-1)/8)] |=   (1 << (7 - ((lport-1)%8)));
        }
    }

    return TRUE;
} //CGI_MODULE_PVLAN_ParseDownlinks

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)
static BOOL_T CGI_MODULE_PVLAN_GetSession(UI32_T session_id, json_t *entry_obj_p)
{
    json_t  *uplink_port_p = json_array();
    json_t  *downlink_port_p = json_array();
    UI8_T  uplink_port_ar[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    UI8_T  downlink_port_ar[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    char   port_ar[CGI_MODULE_INTERFACE_ID_LEN] = {0};
    UI32_T lport = 0;

    if (TRUE != SWCTRL_POM_GetPrivateVlanBySessionId(session_id, uplink_port_ar, downlink_port_ar))
    {
        return FALSE;
    }

    for (lport = SYS_ADPT_ETHER_1_IF_INDEX_NUMBER; lport<= SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
    {
        if (uplink_port_ar[(UI32_T)((lport-1)/8)] & (1<<(7-((lport-1)%8))))
        {
            CGI_UTIL_IfindexToRestPortStr(lport, port_ar);
            json_array_append_new(uplink_port_p, json_string(port_ar));
        }
    }

    for (lport = SYS_ADPT_ETHER_1_IF_INDEX_NUMBER; lport<= SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
    {
        if (downlink_port_ar[(UI32_T)((lport-1)/8)] & (1<<(7-((lport-1)%8))))
        {
            CGI_UTIL_IfindexToRestPortStr(lport, port_ar);
            json_array_append_new(downlink_port_p, json_string(port_ar));
        }
    }

    json_object_set_new(entry_obj_p, "id", json_integer(session_id));
    json_object_set_new(entry_obj_p, "uplinks", uplink_port_p);
    json_object_set_new(entry_obj_p, "downlinks", downlink_port_p);
    return TRUE;
}

static BOOL_T CGI_MODULE_PVLAN_SetUplinkPorts(UI32_T session_id, UI32_T mgmt_port)
{
    UI8_T  uplink_port_ar[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    UI8_T  downlink_port_ar[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    UI32_T unit = 0;
    UI32_T max_port = 0;
    int i = 0;

    STKTPLG_POM_GetNextUnit(&unit);
    max_port = SWCTRL_POM_UIGetUnitPortNumber(unit);

    for (i = 1; i <= max_port; i++)
    {
        if(!SWCTRL_POM_UserPortExisting(unit, i))
        {
            continue;
        }

        if (i == mgmt_port)
        {
            continue;
        }

        uplink_port_ar[(UI32_T)((i-1)/8)] |= (1 << (7 - ((i-1)%8)));
    }

    if (TRUE != SWCTRL_PMGR_SetPrivateVlanBySessionId(session_id, uplink_port_ar, downlink_port_ar))
    {
        return FALSE;
    }

    return TRUE;
} //CGI_MODULE_PVLAN_SetUplinkPorts
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION) */
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */

#if (SYS_CPNT_EXCLUDED_VLAN == TRUE)
static BOOL_T CGI_MODULE_PVLAN_GetExcludeVlanSession(UI32_T session_id, json_t *entry_obj_p)
{
    json_t  *uplink_port_p = json_array();
    json_t  *downlink_port_p = json_array();
    json_t  *vlans_p = json_array();
    SWCTRL_VlanInfo_T vlan_ar[SYS_ADPT_EXCLUDED_VLAN_MAX_NBRS_OF_VLAN_TABLE];
    UI8_T  uplink_port_ar[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    UI8_T  downlink_port_ar[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    char   port_ar[CGI_MODULE_INTERFACE_ID_LEN] = {0};
    char   vid_mask_ar[10] = {0}; //max 4094/4095
    UI32_T lport = 0;
    UI32_T display_session_id = (session_id + SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS);
    int idx = 0;

    memset(vlan_ar, 0, sizeof(vlan_ar));

    if (TRUE != SWCTRL_POM_GetExcludedVlanBySessionId(session_id, uplink_port_ar, downlink_port_ar, vlan_ar))
    {
        return FALSE;
    }

    for (lport = SYS_ADPT_ETHER_1_IF_INDEX_NUMBER; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
    {
        if (uplink_port_ar[(UI32_T)((lport-1)/8)] & (1<<(7-((lport-1)%8))))
        {
            CGI_UTIL_IfindexToRestPortStr(lport, port_ar);
            json_array_append_new(uplink_port_p, json_string(port_ar));
        }
    }

    for (lport = SYS_ADPT_ETHER_1_IF_INDEX_NUMBER; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
    {
        if (downlink_port_ar[(UI32_T)((lport-1)/8)] & (1<<(7-((lport-1)%8))))
        {
            CGI_UTIL_IfindexToRestPortStr(lport, port_ar);
            json_array_append_new(downlink_port_p, json_string(port_ar));
        }
    }

    for (idx = 0; idx < SYS_ADPT_EXCLUDED_VLAN_MAX_NBRS_OF_VLAN_TABLE; idx++)
    {
        if (0 != vlan_ar[idx].vlan)
        {
            if (CGI_MODULE_PVLAN_EXCLUDE_VLAN_MASK != vlan_ar[idx].vlan_mask)
            {
                sprintf(vid_mask_ar, "%d/%d", vlan_ar[idx].vlan, vlan_ar[idx].vlan_mask);
            }
            else
            {
                sprintf(vid_mask_ar, "%d", vlan_ar[idx].vlan);
            }

            json_array_append_new(vlans_p, json_string(vid_mask_ar));
        }
    }

    json_object_set_new(entry_obj_p, "id", json_integer(display_session_id));
    json_object_set_new(entry_obj_p, "uplinks", uplink_port_p);
    json_object_set_new(entry_obj_p, "downlinks", downlink_port_p);
    json_object_set_new(entry_obj_p, "excludeVlans", vlans_p);
    return TRUE;
} //CGI_MODULE_PVLAN_GetExcludeVlanSession

static BOOL_T CGI_MODULE_PVLAN_ParseExcludeVlan(json_t *vlans_p,
        SWCTRL_VlanInfo_T vlan_ar[SYS_ADPT_EXCLUDED_VLAN_MAX_NBRS_OF_VLAN_TABLE])
{
    json_t  *vlan_p;
    char *vlan_str_p;
    char *save_str_addr = NULL;
    char *vid_val_str_p;
    int vlan_num = 0, idx = 0;
    int vid = 0, vid_mask = 0;

    vlan_num = json_array_size(vlans_p);

    if (0 == vlan_num)
    {
        return TRUE;
    }

    if (SYS_ADPT_EXCLUDED_VLAN_MAX_NBRS_OF_VLAN_TABLE < vlan_num)
    {
        return FALSE;
    }


    for (idx = 0; idx < vlan_num; idx ++)
    {
        vlan_p = json_array_get(vlans_p, idx);

        if (NULL == vlan_p)
        {
            continue;
        }

        vlan_str_p = (char *)json_string_value(vlan_p);
        vid_val_str_p = strtok_r(vlan_str_p, "/", &save_str_addr);

        if (sscanf(vid_val_str_p, "%d", &vid) != 1)
        {
            return FALSE;
        }

        if ((1 > vid) || (SYS_ADPT_MAX_VLAN_ID < vid))
        {
            return FALSE;
        }

        if (sscanf(save_str_addr, "%d", &vid_mask) != 1)
        {
            vid_mask = CGI_MODULE_PVLAN_EXCLUDE_VLAN_MASK;
        }
        else
        {
            if (CGI_MODULE_PVLAN_EXCLUDE_VLAN_MASK < vid_mask)
            {
                return FALSE;
            }
        }

        vlan_ar[idx].vlan = vid;
        vlan_ar[idx].vlan_mask = vid_mask;
    } //for (idx = 0; idx < vlan_num; idx ++)

    return TRUE;
} //CGI_MODULE_PVLAN_ParseExcludeVlan
#endif /* #if (SYS_CPNT_EXCLUDED_VLAN == TRUE) */

void CGI_MODULE_PVLAN_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler = CGI_MODULE_PVLAN_Read;
    handlers.update_handler = CGI_MODULE_PVLAN_Update;
    CGI_MAIN_Register("/api/v1/traffic-segmentation", &handlers, 0);

    {   /* for "/api/v1/vlans/{vid}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_PVLAN_Sessions_Read;
        handlers.create_handler = CGI_MODULE_PVLAN_Sessions_Create;
        CGI_MAIN_Register("/api/v1/traffic-segmentation/sessions", &handlers, 0);
    }

    {   /* for "/api/v1/vlans/{vid}/members" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_PVLAN_Sessions_ID_Read;
        handlers.update_handler = CGI_MODULE_PVLAN_Sessions_ID_Update;
        handlers.delete_handler = CGI_MODULE_PVLAN_Sessions_ID_Delete;
        CGI_MAIN_Register("/api/v1/traffic-segmentation/sessions/{sessionId}", &handlers, 0);
    }
}

static void CGI_MODULE_PVLAN_Init()
{
    CGI_MODULE_PVLAN_RegisterHandlers();
}

