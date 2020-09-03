#include "cgi_request.h"
#include "cgi_auth.h"
#include "cgi_util.h"

#if (SYS_CPNT_ADD == TRUE)
#include "add_pmgr.h"
#endif  /* #if (SYS_CPNT_ADD == TRUE) */

#if (SYS_CPNT_ADD == TRUE)
static BOOL_T CGI_MODULE_VOICE_VLAN_CreateOui(json_t *mac_p, json_t *mask_p, json_t *desc_p);
static CGI_STATUS_CODE_T CGI_MODULE_VOICE_VLAN_UpdatePortMode(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *mode_p);
static CGI_STATUS_CODE_T CGI_MODULE_VOICE_VLAN_UpdatePortSecurity(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *security_p);

#if (SYS_CPNT_ADD_CONFIGURABLE_PRIORTIY == TRUE)
static CGI_STATUS_CODE_T CGI_MODULE_VOICE_VLAN_UpdatePortPriority(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *priority_p);
#endif  //#if (SYS_CPNT_ADD_CONFIGURABLE_PRIORTIY == TRUE) */

#if (SYS_CPNT_ADD_SUPPORT_LLDP == TRUE)
static CGI_STATUS_CODE_T CGI_MODULE_VOICE_VLAN_UpdatePortRule(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *rule_oui_p, json_t *rule_lldp_p);
#endif  /* #if (SYS_CPNT_ADD_SUPPORT_LLDP == TRUE) */
#endif  /* #if (SYS_CPNT_ADD == TRUE) */

/**----------------------------------------------------------------------
 * This API is used to update voice VLAN status.
 *
 * @param status    (required, boolean) Set voice VLAN status
 * @param vlanId    (optional, number) Voice VLAN ID; Used when status is true
 * @param agingTime (optional, number) Aging time in minutes <5-43200>; Used when status is true
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VOICE_VLAN_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

#if (SYS_CPNT_ADD == TRUE)
    json_t  *status_p;
    json_t  *vid_p;
    json_t  *aging_p;
    UI32_T vid = VAL_voiceVlanEnabledId_disabled;

    status_p = CGI_REQUEST_GetBodyValue(http_request, "status");

    if (NULL == status_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Status is NULL.");
    }

    if (TRUE != json_is_boolean(status_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
    }

    if (TRUE == json_boolean_value(status_p))
    {
        vid_p = CGI_REQUEST_GetBodyValue(http_request, "vlanId");

        if (NULL == vid_p)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "VID is NULL for enabled.");
        }

        vid = json_integer_value(vid_p);
    }

    if (TRUE != ADD_PMGR_SetVoiceVlanEnabledId(vid))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                "add.setVoiceVlanEnabledIdError", "Failed to set status.");
    }

    aging_p = CGI_REQUEST_GetBodyValue(http_request, "agingTime");

    if (NULL != aging_p)
    {
        UI32_T timeout = json_integer_value(aging_p); //SYS_DFLT_ADD_VOICE_VLAN_TIMEOUT_MINUTE

        if (TRUE != ADD_PMGR_SetVoiceVlanAgingTime(timeout))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "add.setVoiceVlanAgingTimeError", "Failed to set agint timeout.");
        }
    }
#endif  /* #if (SYS_CPNT_ADD == TRUE) */

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_VOICE_VLAN_Update

/**----------------------------------------------------------------------
 * This API is used to create voice VLAN OUI.
 *
 * @param ouis   (required, array) Voice VLAN MAC address
 * @param oui.macAddr (required, string) Voice VLAN MAC address
 * @param oui.mask   (required, string) Voice VLAN MAC mask address
 * @param oui.description   (optional, string) MAC address description
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VOICE_VLAN_OUI_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

#if (SYS_CPNT_ADD == TRUE)
    json_t  *ouis_p;
    json_t  *oui_p;
    json_t  *mac_p;
    json_t  *mask_p;
    json_t  *desc_p;
    int oui_num = 0, idx = 0;

    ouis_p = CGI_REQUEST_GetBodyValue(http_request, "ouis");

    if (NULL != ouis_p)
    {
        oui_num = json_array_size(ouis_p);

        if (0 >= oui_num)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "oui number is 0.");
        }

        for (idx = 0; idx < oui_num; idx ++)
        {
            oui_p = json_array_get(ouis_p, idx);

            if (NULL == oui_p)
            {
                continue;
            }

            mac_p = json_object_get(oui_p, "macAddr");
            mask_p = json_object_get(oui_p, "mask");
            desc_p = json_object_get(oui_p, "description");

            if ((NULL == mac_p) || (NULL == mask_p))
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Must specify macAddr and mask.");
            }

            if (TRUE != CGI_MODULE_VOICE_VLAN_CreateOui(mac_p, mask_p, desc_p))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "add.addOuiEntryError", "Failed to add OUI entry.");
            }
        } //for (idx = 0; idx < oui_num; idx ++)
    }
    else //only create one oui
    {
        mac_p = CGI_REQUEST_GetBodyValue(http_request, "macAddr");
        mask_p = CGI_REQUEST_GetBodyValue(http_request, "mask");
        desc_p = CGI_REQUEST_GetBodyValue(http_request, "description");

        if ((NULL == mac_p) || (NULL == mask_p))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Must specify macAddr and mask.");
        }

        if (TRUE != CGI_MODULE_VOICE_VLAN_CreateOui(mac_p, mask_p, desc_p))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "add.addOuiEntryError", "Failed to add OUI entry.");
        }
    }
#endif  /* #if (SYS_CPNT_ADD == TRUE) */

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_VOICE_VLAN_OUI_Create

/**----------------------------------------------------------------------
 * This API is used to delete a OUI.
 *
 * @param id (required, string) OUI ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VOICE_VLAN_OUI_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

#if (SYS_CPNT_ADD == TRUE)
    json_t  *id_p;
    char    *id_str_p;
    char    *save_str_addr = NULL;
    char    *mac_str_p;
    ADD_MGR_VoiceVlanOui_T entry;

    memset(&entry, 0, sizeof(entry));
    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get OUI ID.");
    }

    id_str_p = (char *) json_string_value(id_p);
    mac_str_p = strtok_r(id_str_p, "_", &save_str_addr);

    if (TRUE != CGI_UTIL_MacStrToHex(mac_str_p, entry.oui))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid MAC address.");
    }

    if (TRUE != CGI_UTIL_MacStrToHex(save_str_addr, entry.mask))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid MAC mask address.");
    }

    if (TRUE != ADD_PMGR_RemoveOuiEntry(entry.oui, entry.mask))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "add.removeOuiEntryError", "Failed to remove OUI entry.");
    }
#endif  /* #if (SYS_CPNT_ADD == TRUE) */

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_VOICE_VLAN_OUI_ID_Delete


/**----------------------------------------------------------------------
 * This API is used to update attribute of specified ports.
 *
 * @param ports    (required, array) Port array
 * @param mode     (optional, string) Mode auto, manual, or none
 * @param security (optional, boolean) Security mode
 * @param priority (optional, number) Voice VLAN priority. <0-6>
 * @param ruleOui  (optional, boolean) Rule oui
 * @param ruleLldp (optional, boolean) Rule LLDP
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VOICE_VLAN_INTF_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

#if (SYS_CPNT_ADD == TRUE)
    json_t  *ports_p;
    json_t  *port_p;
    json_t  *mode_p;
    json_t  *security_p;
#if (SYS_CPNT_ADD_CONFIGURABLE_PRIORTIY == TRUE)
    json_t  *priority_p;
#endif  /* #if (SYS_CPNT_ADD_CONFIGURABLE_PRIORTIY == TRUE) */
#if (SYS_CPNT_ADD_SUPPORT_LLDP == TRUE)
    json_t  *rule_oui_p;
    json_t  *rule_lldp_p;
#endif  /* #if (SYS_CPNT_ADD_SUPPORT_LLDP == TRUE) */
    const char *port_str_p;
    const char *rule_str_p;
    int port_num = 0, idx = 0;
    UI32_T lport = 0;

    ports_p = CGI_REQUEST_GetBodyValue(http_request, "ports");

    if (NULL == ports_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "ports is NULL.");
    }
    port_num = json_array_size(ports_p);

    if (0 >= port_num)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "ports number is 0.");
    }

    for (idx = 0; idx < port_num; idx ++)
    {
        port_p = json_array_get(ports_p, idx);

        if (NULL != port_p)
        {
            port_str_p = json_string_value(port_p);

            if (TRUE != CGI_UTIL_InterfaceIdToLport(port_str_p, &lport))
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid ifId.");
            }
        }
    }

    mode_p = CGI_REQUEST_GetBodyValue(http_request, "mode");

    if (NULL != mode_p)
    {
        return CGI_MODULE_VOICE_VLAN_UpdatePortMode(http_response, ports_p, port_num, mode_p);
    }

    security_p = CGI_REQUEST_GetBodyValue(http_request, "security");

    if (NULL != security_p)
    {
        return CGI_MODULE_VOICE_VLAN_UpdatePortSecurity(http_response, ports_p, port_num, security_p);
    }

#if (SYS_CPNT_ADD_CONFIGURABLE_PRIORTIY == TRUE)
    priority_p = CGI_REQUEST_GetBodyValue(http_request, "priority");

    if (NULL != priority_p)
    {
        return CGI_MODULE_VOICE_VLAN_UpdatePortPriority(http_response, ports_p, port_num, priority_p);
    }
#endif  /* #if (SYS_CPNT_ADD_CONFIGURABLE_PRIORTIY == TRUE) */


#if (SYS_CPNT_ADD_SUPPORT_LLDP == TRUE)
    rule_oui_p = CGI_REQUEST_GetBodyValue(http_request, "ruleOui");

    if (NULL != rule_oui_p)
    {
        rule_lldp_p = CGI_REQUEST_GetBodyValue(http_request, "ruleLldp");

        if (NULL == rule_lldp_p)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Miss ruleLldp.");
        }

        return CGI_MODULE_VOICE_VLAN_UpdatePortRule(http_response, ports_p, port_num, rule_oui_p, rule_lldp_p);
    }
#endif  /* #if (SYS_CPNT_ADD_SUPPORT_LLDP == TRUE) */

#endif  /* #if (SYS_CPNT_ADD == TRUE) */

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_VOICE_VLAN_INTF_Update

#if (SYS_CPNT_ADD == TRUE)
static BOOL_T CGI_MODULE_VOICE_VLAN_CreateOui(json_t *mac_p, json_t *mask_p, json_t *desc_p)
{
    ADD_MGR_VoiceVlanOui_T entry;
    const char *mac_str_p = json_string_value(mac_p);
    const char *mask_str_p = json_string_value(mask_p);

    memset(&entry, 0, sizeof(entry));

    if (NULL != desc_p)
    {
        const char *desc_str_p = json_string_value(desc_p);
        strncpy(entry.description, desc_str_p, sizeof(entry.description)-1);
    }

    if (TRUE != CGI_UTIL_MacStrToHex(mac_str_p, entry.oui))
    {
        return FALSE;
    }

    if (TRUE != CGI_UTIL_MacStrToHex(mask_str_p, entry.mask))
    {
        return FALSE;
    }

    if (TRUE != ADD_PMGR_AddOuiEntry(&entry))
    {
        return FALSE;
    }

    return TRUE;
} //CGI_MODULE_VOICE_VLAN_CreateOui

static CGI_STATUS_CODE_T CGI_MODULE_VOICE_VLAN_UpdatePortMode(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *mode_p)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *port_p;
    const char *mode_str_p = json_string_value(mode_p);
    const char *port_str_p;
    UI32_T port_mode = VAL_voiceVlanPortMode_none;
    UI32_T lport = 0;
    int idx = 0;

    if (0 == strncmp(mode_str_p, "auto", strlen("auto")))
    {
        port_mode = VAL_voiceVlanPortMode_auto;
    }
    else if (0 == strncmp(mode_str_p, "none", strlen("none")))
    {
        port_mode = VAL_voiceVlanPortMode_none;
    }
    else if (0 == strncmp(mode_str_p, "manual", strlen("manual")))
    {
        port_mode = VAL_voiceVlanPortMode_manual;
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid port mode.");
    }

    for (idx = 0; idx < port_num; idx ++)
    {
        port_p = json_array_get(ports_p, idx);
        port_str_p = json_string_value(port_p);
        CGI_UTIL_InterfaceIdToLport(port_str_p, &lport);

        if (TRUE != ADD_PMGR_SetVoiceVlanPortMode(lport, port_mode))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "add.setVoiceVlanPortModeError", "Failed to set port mode.");
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_VOICE_VLAN_UpdatePortMode

static CGI_STATUS_CODE_T CGI_MODULE_VOICE_VLAN_UpdatePortSecurity(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *security_p)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *port_p;
    const char *port_str_p;
    UI32_T voice_vlan_state = VAL_voiceVlanPortSecurity_disabled;
    UI32_T lport = 0;
    int idx = 0;

    if (TRUE != json_is_boolean(security_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The security status is not boolean type.");
    }

    if (TRUE == json_boolean_value(security_p))
    {
        voice_vlan_state = VAL_voiceVlanPortSecurity_enabled;
    }

    for (idx = 0; idx < port_num; idx ++)
    {
        port_p = json_array_get(ports_p, idx);
        port_str_p = json_string_value(port_p);
        CGI_UTIL_InterfaceIdToLport(port_str_p, &lport);

        if (TRUE != ADD_PMGR_SetVoiceVlanPortSecurityState(lport, voice_vlan_state))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "add.setVoiceVlanPortSecurityStateError", "Failed to set port security status.");
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_VOICE_VLAN_UpdatePortSecurity

#if (SYS_CPNT_ADD_CONFIGURABLE_PRIORTIY == TRUE)
static CGI_STATUS_CODE_T CGI_MODULE_VOICE_VLAN_UpdatePortPriority(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *priority_p)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *port_p;
    const char *port_str_p;
    UI32_T priority = json_integer_value(priority_p);
    UI32_T lport = 0;
    int idx = 0;

    for (idx = 0; idx < port_num; idx ++)
    {
        port_p = json_array_get(ports_p, idx);
        port_str_p = json_string_value(port_p);
        CGI_UTIL_InterfaceIdToLport(port_str_p, &lport);

        if (TRUE != ADD_PMGR_SetVoiceVlanPortPriority(lport, priority))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "add.setVoiceVlanPortPriorityError", "Failed to set port priority.");
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_VOICE_VLAN_UpdatePortPriority
#endif  /* #if (SYS_CPNT_ADD_CONFIGURABLE_PRIORTIY == TRUE) */

#if (SYS_CPNT_ADD_SUPPORT_LLDP == TRUE)
static CGI_STATUS_CODE_T CGI_MODULE_VOICE_VLAN_UpdatePortRule(
        HTTP_Response_T *http_response, json_t *ports_p, int port_num, json_t *rule_oui_p, json_t *rule_lldp_p)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *port_p;
    const char *port_str_p;
    UI32_T oui_state = VAL_voiceVlanPortRuleOui_disabled;
    UI32_T lldp_state = VAL_voiceVlanPortRuleLldp_disabled;
    UI32_T lport = 0;
    int idx = 0;

    if (TRUE != json_is_boolean(rule_oui_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The oui status is not boolean type.");
    }

    if (TRUE == json_boolean_value(rule_oui_p))
    {
        oui_state = VAL_voiceVlanPortRuleOui_enabled;
    }

    if (TRUE != json_is_boolean(rule_lldp_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The lldp status is not boolean type.");
    }

    if (TRUE == json_boolean_value(rule_lldp_p))
    {
        lldp_state = VAL_voiceVlanPortRuleLldp_enabled;
    }

    if ((VAL_voiceVlanPortRuleOui_disabled == oui_state) && (VAL_voiceVlanPortRuleLldp_disabled == lldp_state))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "At least one rule state should be enabled.");
    }

    for (idx = 0; idx < port_num; idx ++)
    {
        port_p = json_array_get(ports_p, idx);
        port_str_p = json_string_value(port_p);
        CGI_UTIL_InterfaceIdToLport(port_str_p, &lport);

        /* cause the rule can not be empty, when oui rule is disabled, set lldp rule enabled first
         */
        if (VAL_voiceVlanPortRuleOui_disabled == oui_state)
        {
            if (TRUE != ADD_PMGR_SetVoiceVlanPortLldpRuleState(lport, lldp_state))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "add.setVoiceVlanPortPriorityError", "Failed to set port LLDP rule.");
            }
        }

        if (TRUE != ADD_PMGR_SetVoiceVlanPortOuiRuleState(lport, oui_state))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "add.setVoiceVlanPortOuiRuleStateError", "Failed to set port OUI rule.");
        }

        if (VAL_voiceVlanPortRuleOui_disabled != oui_state)
        {
            if (TRUE != ADD_PMGR_SetVoiceVlanPortLldpRuleState(lport, lldp_state))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "add.setVoiceVlanPortPriorityError", "Failed to set port LLDP rule.");
            }
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_VOICE_VLAN_UpdatePortRule
#endif  /* #if (SYS_CPNT_ADD_SUPPORT_LLDP == TRUE) */
#endif  /* #if (SYS_CPNT_ADD == TRUE) */

void CGI_MODULE_VOICE_VLAN_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.update_handler = CGI_MODULE_VOICE_VLAN_Update;
    CGI_MAIN_Register("/api/v1/voice-vlan", &handlers, 0);

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.create_handler = CGI_MODULE_VOICE_VLAN_OUI_Create;
        //handlers.delete_handler = CGI_MODULE_VOICE_VLAN_OUI_Delete;
        CGI_MAIN_Register("/api/v1/voice-vlan/oui", &handlers, 0);
    }

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.delete_handler = CGI_MODULE_VOICE_VLAN_OUI_ID_Delete;
        CGI_MAIN_Register("/api/v1/voice-vlan/oui/{id}", &handlers, 0);
    }

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.update_handler = CGI_MODULE_VOICE_VLAN_INTF_Update;
        CGI_MAIN_Register("/api/v1/voice-vlan/interfaces", &handlers, 0);
    }
}

static void CGI_MODULE_VOICE_VLAN_Init()
{
    CGI_MODULE_VOICE_VLAN_RegisterHandlers();
}
