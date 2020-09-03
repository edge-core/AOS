#include "cgi_request.h"
#include "cgi_auth.h"
#include "cgi_util.h"
#include "rspan_pmgr.h"
#include "rspan_om.h"

static void CGI_MODULE_RSPAN_ConvertLport2Str(UI32_T lport, char *str_p);

/**----------------------------------------------------------------------
 * This API is used to create RSPAN session.
 *
 * @param session (required, number) Session ID <1-2>
 * @param src (required, string) Source port interface (monitored port)
 * @param dst (required, string) Destination port interface (listen port)
 * @param vid (required, number) VLAN ID <4001-4002>
 * @param mode (required, string) Monitor mode {both | rx | tx}
 * @param type (required, string) Monitor type {source | destination | intermediate}
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_RSPAN_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *session_p;
    json_t  *src_p;
    json_t  *dst_p;
    json_t  *vid_p;
    json_t  *mode_p;
    json_t  *type_p;
    const char*  src_str_p;
    const char*  dst_str_p;
    const char*  mode_str_p;
    const char*  type_str_p;
    UI32_T session = 0;
    UI32_T src_lport = 0;
    UI32_T dst_lport = 0;
    UI32_T vid = 0;
    UI8_T mode = 0;
    UI8_T role = 0;

    session_p = CGI_REQUEST_GetBodyValue(http_request, "session");

    if (session_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get session.");
    }
    session = json_integer_value(session_p);

    src_p = CGI_REQUEST_GetBodyValue(http_request, "src");

    if (src_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get src.");
    }
    src_str_p = json_string_value(src_p);

    if (TRUE != CGI_UTIL_InterfaceIdToLport(src_str_p, &src_lport))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid source port.");
    }

    dst_p = CGI_REQUEST_GetBodyValue(http_request, "dst");

    if (dst_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get dst.");
    }
    dst_str_p = json_string_value(dst_p);

    if (TRUE != CGI_UTIL_InterfaceIdToLport(dst_str_p, &dst_lport))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid destination port.");
    }

    vid_p = CGI_REQUEST_GetBodyValue(http_request, "vid");

    if (vid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get VLAN ID.");
    }
    vid = json_integer_value(vid_p);

    if ((4001 > vid) || (4006 < vid))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid VLAN ID for RSPAN.");
    }

    mode_p = CGI_REQUEST_GetBodyValue(http_request, "mode");

    if (mode_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get mode.");
    }
    mode_str_p = json_string_value(mode_p);

    if (0 == strncmp(mode_str_p, "tx", strlen("tx")))
    {
        mode = VAL_mirrorType_tx;
    }
    else if (0 == strncmp(mode_str_p, "rx", strlen("rx")))
    {
        mode = VAL_mirrorType_rx;
    }
    else if (0 == strncmp(mode_str_p, "both", strlen("both")))
    {
        mode = VAL_mirrorType_both;
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid mode.");
    }

    type_p = CGI_REQUEST_GetBodyValue(http_request, "type");

    if (type_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get mode.");
    }
    type_str_p = json_string_value(type_p);

    if (0 == strncmp(type_str_p, "source", strlen("source")))
    {
        role = VAL_rspanSwitchRole_source;
    }
    else if (0 == strncmp(type_str_p, "destination", strlen("destination")))
    {
        role = VAL_rspanSwitchRole_destination;
    }
    else if (0 == strncmp(type_str_p, "intermediate", strlen("intermediate")))
    {
        role = VAL_rspanSwitchRole_intermediate;
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid type.");
    }

#if (SYS_CPNT_RSPAN == TRUE)
    if (TRUE != RSPAN_PMGR_CreateRspanVlan(vid))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "rspan.CreateRspanVlanError", "Failed.");
    }

    if (TRUE != VLAN_PMGR_SetDot1qVlanStaticRowStatus(vid, VAL_dot1qVlanStaticRowStatus_active))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "rspan.SetDot1qVlanStaticRowStatusError", "Failed.");
    }

    if (VAL_rspanSwitchRole_source == role)
    {
        if (TRUE != RSPAN_PMGR_SetSessionSourceInterface(session, src_lport, mode))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "rspan.SetSessionSourceInterfaceError", "Failed.");
        }
    } //VAL_rspanSwitchRole_source
    else if (VAL_rspanSwitchRole_destination == role)
    {
        if (TRUE != RSPAN_PMGR_SetSessionDestinationInterface(session, src_lport, VAL_rspanDstPortTag_untagged))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "rspan.SetSessionDestinationInterfaceError", "Failed.");
        }
    }

    if (TRUE != RSPAN_PMGR_SetSessionRemoteVlan(session, role, vid, dst_lport))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "rspan.SetSessionRemoteVlanError", "Failed.");
    }

    if (VAL_rspanSwitchRole_intermediate == role)
    {
        if (TRUE != RSPAN_PMGR_SetSessionRemoteVlan(session, role, vid, src_lport))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "rspan.SetSessionRemoteVlanError", "Failed.");
        }

        if (TRUE != RSPAN_PMGR_SetSessionRemoteVlanDst(session, dst_lport))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "rspan.SetSessionRemoteVlanDstError", "Failed.");
        }
    }
#endif  /* #if (SYS_CPNT_RSPAN == TRUE) */

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to read RSPAN setting.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_RSPAN_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *sessions_p = json_array();
    RSPAN_OM_SessionEntry_T rspan_session_entry;
    char src_ar[20] = {0};
    char dst_ar[20] = {0};
    UI32_T lport = 0;
    UI32_T rx_on = 0;
    UI32_T tx_on = 0;
    UI8_T session_id = 0;

    memset(&rspan_session_entry, 0, sizeof(rspan_session_entry));

    while (TRUE == RSPAN_PMGR_GetNextRspanSessionEntry(&session_id, &rspan_session_entry))
    {
        json_t *session_obj_p = json_object();
        json_t *src_p = json_array();

        memset(src_ar, 0, sizeof(src_ar));
        memset(dst_ar, 0, sizeof(dst_ar));
        json_object_set_new(session_obj_p, "id", json_integer(session_id));
        json_object_set_new(session_obj_p, "vid", json_integer(rspan_session_entry.remote_vid));
        json_object_set_new(session_obj_p, "src", src_p);

        switch(rspan_session_entry.switch_role)
        {
            case VAL_rspanSwitchRole_source:
                json_object_set_new(session_obj_p, "type", json_string("source"));

                for (lport = 1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT ; lport++)
                {
                    rx_on = rspan_session_entry.src_rx[(UI32_T)((lport-1)/8)] & (1 << (7 - ((lport-1)%8)));
                    tx_on = rspan_session_entry.src_tx[(UI32_T)((lport-1)/8)] & (1 << (7 - ((lport-1)%8)));

                    if ((FALSE == rx_on) && (FALSE == tx_on))
                    {
                        continue;
                    }

                    CGI_MODULE_RSPAN_ConvertLport2Str(lport, src_ar);
                    json_array_append_new(src_p, json_string(src_ar));

                    if (rx_on && tx_on)
                    {
                        json_object_set_new(session_obj_p, "mode", json_string("both"));
                    }
                    else if (rx_on)
                    {
                        json_object_set_new(session_obj_p, "mode", json_string("rx"));
                    }
                    else if (tx_on)
                    {
                        json_object_set_new(session_obj_p, "mode", json_string("tx"));
                    }
                }

                for (lport = 1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT ; lport++)
                {
                    if (rspan_session_entry.uplink[(UI32_T)((lport-1)/8)] & (1 << (7-((lport-1)%8))) )
                    {
                        CGI_MODULE_RSPAN_ConvertLport2Str(lport, dst_ar);
                        json_object_set_new(session_obj_p, "dst", json_string(dst_ar));
                        break;
                    }
                }
                break;

            case VAL_rspanSwitchRole_destination:
                json_object_set_new(session_obj_p, "type", json_string("destination"));

                for (lport = 1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT ; lport++)
                {
                    if (rspan_session_entry.uplink[(UI32_T)((lport-1)/8)] & (1 << (7-((lport-1)%8))) )
                    {
                        CGI_MODULE_RSPAN_ConvertLport2Str(lport, dst_ar);
                        json_object_set_new(session_obj_p, "dst", json_string(dst_ar));
                        break;
                    }
                }

                CGI_MODULE_RSPAN_ConvertLport2Str(rspan_session_entry.dst, src_ar);
                json_array_append_new(src_p, json_string(src_ar));
                break;

            case VAL_rspanSwitchRole_intermediate:
                json_object_set_new(session_obj_p, "type", json_string("intermediate"));

                CGI_MODULE_RSPAN_ConvertLport2Str(rspan_session_entry.dst_in, dst_ar);
                json_object_set_new(session_obj_p, "dst", json_string(dst_ar));

                for (lport = 1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT ; lport++)
                {
                    if (rspan_session_entry.uplink[(UI32_T)((lport-1)/8)] & (1 << (7-((lport-1)%8))) )
                    {
                        if (rspan_session_entry.dst_in != lport)
                        {
                            CGI_MODULE_RSPAN_ConvertLport2Str(lport, src_ar);
                            json_array_append_new(src_p, json_string(src_ar));
                        }
                    }
                }
                break;

            default:
                break;
        }
        json_array_append_new(sessions_p, session_obj_p);
    } //while (TRUE == SWCTRL_POM_GetNextMirrorEntry(&mirroring_entry))

    json_object_set_new(result_p, "sessions", sessions_p);
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete port monitor.
 *
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_RSPAN_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    RSPAN_OM_SessionEntry_T rspan_session_entry;
    UI32_T session_id = 0;

    memset(&rspan_session_entry, 0, sizeof(rspan_session_entry));
    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ID.");
    }
    session_id = json_integer_value(id_p);

    if (TRUE == RSPAN_PMGR_GetRspanSessionEntry(session_id, &rspan_session_entry))
    {
        RSPAN_PMGR_RemoveCliWebSessionId(session_id);

        if (TRUE == RSPAN_OM_IsRspanVlan(rspan_session_entry.remote_vid))
        {
            if (TRUE != RSPAN_PMGR_DeleteRspanVlan(rspan_session_entry.remote_vid))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "rspan.DeleteRspanVlanError", "Failed.");
            }
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

void CGI_MODULE_RSPAN_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler = CGI_MODULE_RSPAN_Read;
    handlers.create_handler = CGI_MODULE_RSPAN_Create;
    CGI_MAIN_Register("/api/v1/rspan", &handlers, 0);

    {
        CGI_API_HANDLER_SET_T handlers = {0};

        handlers.delete_handler = CGI_MODULE_RSPAN_Delete;
        CGI_MAIN_Register("/api/v1/rspan/{id}", &handlers, 0);
    }
}

static void CGI_MODULE_RSPAN_Init()
{
    CGI_MODULE_RSPAN_RegisterHandlers();
}

static void CGI_MODULE_RSPAN_ConvertLport2Str(UI32_T lport, char *str_p)
{
    UI32_T unit = 0, port = 0, trunk_id = 0;
    UI32_T lport_type = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

    if (lport_type == SWCTRL_LPORT_NORMAL_PORT)
    {
        sprintf(str_p, "eth%lu/%lu", (unsigned long)unit, (unsigned long)port);
    }
    else if (lport_type == SWCTRL_LPORT_TRUNK_PORT)
    {
        sprintf(str_p, "trunk%lu", (unsigned long)trunk_id);
    }
}
