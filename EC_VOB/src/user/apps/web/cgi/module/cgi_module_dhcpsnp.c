#include "cgi_request.h"
#include "cgi_auth.h"
#include "cgi_util.h"

#if (SYS_CPNT_DHCPSNP == TRUE)
#include "dhcpsnp_type.h"
#include "dhcpsnp_pmgr.h"
#endif  /* #if (SYS_CPNT_DHCPSNP == TRUE) */


/**----------------------------------------------------------------------
 * This API is used to get DHCP snooping info.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_DHCPSNP_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    UI8_T   status = 0;

    DHCPSNP_PMGR_GetGlobalDhcpSnoopingStatus(&status);
    json_object_set_new(result_p, "status", json_boolean((status == DHCPSNP_TYPE_SNOOPING_GLOBAL_ENABLED)));

    DHCPSNP_PMGR_GetVerifyMacAddressStatus(&status);
    json_object_set_new(result_p, "verifyMacAddressEnable",
            json_boolean((status == DHCPSNP_TYPE_VERIFY_MAC_ADDRESS_ENABLED)));

#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION == TRUE)
    {
        UI8_T   option82_status = 0, option82_policy = 0;

        DHCPSNP_PMGR_GetInformationOptionStatus(&option82_status);
        json_object_set_new(result_p, "informationOptionEnable",
                json_boolean((option82_status == DHCPSNP_TYPE_OPTION82_ENABLED)));

        DHCPSNP_PMGR_GetInformationPolicy(&option82_policy);

        switch(option82_policy)
        {
            case DHCPSNP_TYPE_OPTION82_POLICY_DROP:
                json_object_set_new(result_p, "informationOptionPolicy", json_string("drop"));
                break;

            case DHCPSNP_TYPE_OPTION82_POLICY_REPLACE:
                json_object_set_new(result_p, "informationOptionPolicy", json_string("replace"));
                break;

            case DHCPSNP_TYPE_OPTION82_POLICY_KEEP:
                json_object_set_new(result_p, "informationOptionPolicy", json_string("keep"));
                break;

            default:
                break;
        }
    }
#endif  /* #if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION == TRUE) */

#if (SYS_CPNT_DHCPSNP_SYSTEM_RATELIMIT == TRUE)
    {
        UI32_T limit = 0;

        DHCPSNP_PMGR_GetGlobalRateLimit(&limit);
        json_object_set_new(result_p, "limitRate", json_integer(limit));
    }
#endif  /* #if (SYS_CPNT_DHCPSNP_SYSTEM_RATELIMIT == TRUE) */

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_DHCPSNP_Read

/**----------------------------------------------------------------------
 * This API is used to update dhcpsnp status.
 *
 * @param status    (Optional, boolean) Set DHCP snooping status
 * @param verifyMacAddressEnable  (Optional, boolean) Verifies the client?™s hardware address stored in the DHCP packet against the source MAC address in the Ethernet header
 * @param informationOptionEnable (Optional, boolean) Information option status.
 * @param informationOptionPolicy (Optional, string) Information Policy: ["drop" | "keep" | "replace"].
 * @param subtype   (required, boolean) Status of sub-type and sub-length fields in CID and RID in Option 82 info
 * @param remoteIdType  (required, string) Type of Relay agent information remote ID sub-option ["ip", "mac", "string"]
 * @param remoteIdValue (required, string) When remoteIdType is ip or mac, could specify encode type ["ascii", "hex"]
 *                                         When remoteIdType is string, could specify remote ID
 * @param limitRate (Optional, number) The maximum number of DHCP packets that may be trapped for DHCP snooping.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_DHCPSNP_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

#if (SYS_CPNT_DHCPSNP == TRUE)
    json_t  *status_p;
    json_t  *verify_mac_p;

    status_p = CGI_REQUEST_GetBodyValue(http_request, "status");

    if (NULL != status_p)
    {
        UI8_T  status = DHCPSNP_TYPE_SNOOPING_GLOBAL_DISABLED;

        if (TRUE != json_is_boolean(status_p))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
        }

        if (TRUE == json_boolean_value(status_p))
        {
            status = DHCPSNP_TYPE_SNOOPING_GLOBAL_ENABLED;
        }

        if (DHCPSNP_TYPE_OK != DHCPSNP_PMGR_SetGlobalDhcpSnoopingStatus(status))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "dhcpsnp.setGlobalDhcpSnoopingStatusError", "Failed to set status.");
        }
    } //if (NULL != status_p)

    verify_mac_p = CGI_REQUEST_GetBodyValue(http_request, "verifyMacAddressEnable");

    if (NULL != verify_mac_p)
    {
        UI8_T  status = DHCPSNP_TYPE_VERIFY_MAC_ADDRESS_DISABLED;

        if (TRUE != json_is_boolean(verify_mac_p))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
        }

        if (TRUE == json_boolean_value(verify_mac_p))
        {
            status = DHCPSNP_TYPE_VERIFY_MAC_ADDRESS_ENABLED;
        }

        if (DHCPSNP_TYPE_OK != DHCPSNP_PMGR_SetVerifyMacAddressStatus(status))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "dhcpsnp.setVerifyMacAddressStatusError", "Failed to set status.");
        }
    } //if (NULL != verify_mac_p)

#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION == TRUE)
    {
        json_t  *info_status_p = CGI_REQUEST_GetBodyValue(http_request, "informationOptionEnable");

        if (NULL != info_status_p)
        {
            UI8_T  status = DHCPSNP_TYPE_OPTION82_DISABLED;

            if (TRUE != json_is_boolean(info_status_p))
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
            }

            if (TRUE == json_boolean_value(info_status_p))
            {
                status = DHCPSNP_TYPE_OPTION82_ENABLED;
            }

            if (DHCPSNP_TYPE_OK != DHCPSNP_PMGR_SetInformationOptionStatus(status))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "dhcpsnp.setInformationOptionStatusError", "Failed to set status.");
            }
        } //if (NULL != info_status_p)
    }

    {
        json_t  *info_policy_p = CGI_REQUEST_GetBodyValue(http_request, "informationOptionPolicy");

        if (NULL != info_policy_p)
        {
            const char  *policy_str_p;
            UI8_T  policy = 0;

            policy_str_p = json_string_value(info_policy_p);

            if (0 == strncmp(policy_str_p, "drop", strlen("drop")))
            {
                policy = DHCPSNP_TYPE_OPTION82_POLICY_DROP;
            }
            else if (0 == strncmp(policy_str_p, "keep", strlen("keep")))
            {
                policy = DHCPSNP_TYPE_OPTION82_POLICY_KEEP;
            }
            else if (0 == strncmp(policy_str_p, "replace", strlen("replace")))
            {
                policy = DHCPSNP_TYPE_OPTION82_POLICY_REPLACE;
            }
            else
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid policy.");
            }

            if (DHCPSNP_TYPE_OK != DHCPSNP_PMGR_SetInformationPolicy(policy))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "dhcpsnp.setInformationPolicyError", "Failed to set policy.");
            }
        } //if (NULL != info_policy_p)
    }

    {
        json_t  *subtype_p = CGI_REQUEST_GetBodyValue(http_request, "subtype");
        BOOL_T subtype_format = FALSE;

        if (NULL != subtype_p)
        {
            if (TRUE != json_is_boolean(subtype_p))
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The subtype is not boolean type.");
            }

            if (TRUE == json_boolean_value(subtype_p))
            {
                subtype_format = TRUE;
            }

            if (DHCPSNP_TYPE_OK != DHCPSNP_PMGR_SetInformationFormat(subtype_format))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "dhcpsnp.setInformationFormatError", "Failed to set subtype format.");
            }
        } //if (NULL != subtype_p)
    }
#endif  /* #if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION == TRUE) */

#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION_CONFIGURABLE_RID == TRUE)
    {
        json_t  *remote_type_p = CGI_REQUEST_GetBodyValue(http_request, "remoteIdType");
        json_t  *remote_value_p = CGI_REQUEST_GetBodyValue(http_request, "remoteIdValue");
        const char  *rtype_str_p;
        const char  *rvalue_str_p;
        UI8_T mode = 0;

        if (NULL != remote_type_p)
        {
            if (NULL == remote_value_p)
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Need remoteIdValue.");
            }

            rtype_str_p = json_string_value(remote_type_p);
            rvalue_str_p = json_string_value(remote_value_p);

            if (0 == strncmp(rtype_str_p, "ip", strlen("ip")))
            {
                if (0 == strncmp(rvalue_str_p, "hex", strlen("hex")))
                {
                    mode = DHCPSNP_TYPE_OPTION82_RID_IP_HEX;
                }
                else if (0 == strncmp(rvalue_str_p, "ascii", strlen("ascii")))
                {
                    mode = DHCPSNP_TYPE_OPTION82_RID_IP_ASCII;
                }
                else
                {
                    return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid remoteIdValue of IP type.");
                }
            }
            else if (0 == strncmp(rtype_str_p, "mac", strlen("mac")))
            {
                if (0 == strncmp(rvalue_str_p, "hex", strlen("hex")))
                {
                    mode = DHCPSNP_TYPE_OPTION82_RID_MAC_HEX;
                }
                else if (0 == strncmp(rvalue_str_p, "ascii", strlen("ascii")))
                {
                    mode = DHCPSNP_TYPE_OPTION82_RID_MAC_ASCII;
                }
                else
                {
                    return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid remoteIdValue of MAC type.");
                }
            }
            else if (0 == strncmp(rtype_str_p, "string", strlen("string")))
            {
                mode = DHCPSNP_TYPE_OPTION82_RID_CONFIGURED_STRING;
            }
            else
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid remoteIdType.");
            }

            if (DHCPSNP_TYPE_OK != DHCPSNP_PMGR_SetInformationRidMode(mode))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "dhcpsnp.setInformationRidModeError", "Failed to set information mode.");
            }

            if (DHCPSNP_TYPE_OPTION82_RID_CONFIGURED_STRING == mode)
            {
                if (DHCPSNP_TYPE_OK != DHCPSNP_PMGR_SetInformationRidValue((UI8_T *)rvalue_str_p))
                {
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                            "dhcpsnp.setInformationRidValueError", "Failed to set information value.");
                }
            }
        } //if (NULL != remote_type_p)
    }
#endif  /* #if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION_CONFIGURABLE_RID == TRUE) */

#if (SYS_CPNT_DHCPSNP_SYSTEM_RATELIMIT == TRUE)
    {
        json_t  *limit_p = CGI_REQUEST_GetBodyValue(http_request, "limitRate");

        if (NULL != limit_p)
        {
            UI32_T limit = json_integer_value(limit_p);

            if (DHCPSNP_TYPE_OK != DHCPSNP_PMGR_SetGlobalRateLimit(limit))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "dhcpsnp.setGlobalRateLimitError", "Failed to set policy.");
            }
        }   //if (NULL != limit_p)
    }
#endif  /* #if (SYS_CPNT_DHCPSNP_SYSTEM_RATELIMIT == TRUE) */
#endif  /* #if (SYS_CPNT_DHCPSNP == TRUE) */

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_DHCPSNP_Update

/**----------------------------------------------------------------------
 * This API is used to update dhcpsnp status of specified VLAN.
 *
 * @param status    (required, boolean) VLAN status
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_DHCPSNP_VLANS_ID_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

#if (SYS_CPNT_DHCPSNP == TRUE)
    json_t  *status_p;
    json_t  *vid_p;
    UI32_T vid = 0;

    vid_p = CGI_REQUEST_GetParamsValue(http_request, "vlanId");

    if (NULL == vid_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "vlanId is NULL.");
    }

    vid = json_integer_value(vid_p);
    status_p = CGI_REQUEST_GetBodyValue(http_request, "status");

    if (NULL == status_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "status is NULL.");
    }

    if (TRUE != json_is_boolean(status_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
    }

    if (TRUE == json_boolean_value(status_p))
    {
        if (DHCPSNP_TYPE_OK != DHCPSNP_PMGR_EnableDhcpSnoopingByVlan(vid))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "dhcpsnp.enableDhcpSnoopingByVlanError", "Failed to enable VLAN status.");
        }
    }
    else
    {
        if (DHCPSNP_TYPE_OK != DHCPSNP_PMGR_DisableDhcpSnoopingByVlan(vid))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "dhcpsnp.disableDhcpSnoopingByVlanError", "Failed to disable VLAN status.");
        }
    }
#endif  /* #if (SYS_CPNT_DHCPSNP == TRUE) */

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_DHCPSNP_VLANS_ID_Update

/**----------------------------------------------------------------------
 * This API is used to get DHCP snooping bindings.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_DHCPSNP_BINDING_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

#if (SYS_CPNT_DHCPSNP == TRUE)
    DHCPSNP_TYPE_BindingEntry_T binding_entry;
    char    mac_ar[CGI_MACSTRLEN] = {0};
    char    ip_ar[CGI_IPSTRLEN] = {0};
    char    port_ar[CGI_MODULE_INTERFACE_ID_LEN] = {0};
    UI32_T  vid = 0;
    json_t  *entries_p = json_array();

    memset(&binding_entry, 0, sizeof(binding_entry));
    json_object_set_new(result_p, "entries", entries_p);

    while (DHCPSNP_PMGR_GetNextDhcpSnoopingBindingEntry(&binding_entry) == DHCPSNP_TYPE_OK)
    {
        json_t *entry_obj_p = json_object();

        VLAN_OM_ConvertFromIfindex(binding_entry.vid_ifindex, &vid);
        CGI_UTIL_HexToMacStr(binding_entry.mac_p, mac_ar);
        L_INET_Ntoa(binding_entry.ip_addr, ip_ar);
        CGI_UTIL_IfindexToRestPortStr(binding_entry.lport_ifindex, port_ar);

        json_object_set_new(entry_obj_p, "vlanId", json_integer(vid));
        json_object_set_new(entry_obj_p, "macAddress", json_string(mac_ar));
        json_object_set_new(entry_obj_p, "entryType", json_string("dynamic"));
        json_object_set_new(entry_obj_p, "ipAddress", json_string(ip_ar));
        json_object_set_new(entry_obj_p, "portIfIndex", json_string(port_ar));
        json_object_set_new(entry_obj_p, "leaseTime", json_integer(binding_entry.lease_time));
        json_array_append_new(entries_p, entry_obj_p);
    }
#endif  /* #if (SYS_CPNT_DHCPSNP == TRUE) */

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_DHCPSNP_BINDING_Read

/**----------------------------------------------------------------------
 * This API is used to clear statistics for all bindings.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_DHCPSNP_BINDING_Clear(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

#if (SYS_CPNT_DHCPSNP == TRUE)
    if (DHCPSNP_TYPE_OK != DHCPSNP_PMGR_DeleteAllDynamicDhcpSnoopingBindingEntry())
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                "dhcpsnp.seleteAllDynamicDhcpSnoopingBindingEntry(Error", "Failed to clear bindings.");
    }
#endif

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_DHCPSNP_BINDING_Read

/**----------------------------------------------------------------------
 * This API is used to update dhcpsnp trust status of specified ports.
 *
 * @param ports    (required, array) Port array
 * @param trustStatus (required, boolean) trust status
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_DHCPSNP_INTF_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

#if (SYS_CPNT_DHCPSNP == TRUE)
    json_t  *trust_p;
    json_t  *ports_p;
    json_t  *port_p;
    const char *port_str_p;
    UI32_T lport = 0;
    int port_num = 0, idx = 0;
    UI8_T  status = DHCPSNP_TYPE_PORT_UNTRUSTED;

    trust_p = CGI_REQUEST_GetBodyValue(http_request, "trustStatus");

    if (NULL == trust_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "trust status is NULL.");
    }

    if (TRUE != json_is_boolean(trust_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The trust status is not boolean type.");
    }

    if (TRUE == json_boolean_value(trust_p))
    {
        status = DHCPSNP_TYPE_PORT_TRUSTED;
    }

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

            if (DHCPSNP_TYPE_OK != DHCPSNP_PMGR_SetPortTrustStatus(lport, status))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "dhcpsnp.setPortTrustStatusError", "Failed to set port trust status.");
            }
        }
    }
#endif  /* #if (SYS_CPNT_DHCPSNP == TRUE) */

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_DHCPSNP_INTF_Update

/**----------------------------------------------------------------------
 * This API is used to update dhcpsnp related fields of specified port.
 *
 * @param trustStatus    (optional, boolean) trust status
 * @param circuitId    (optional, string) trust status
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_DHCPSNP_INTF_ID_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

#if (SYS_CPNT_DHCPSNP == TRUE)
    json_t  *trust_p;
    json_t  *ifId_p;
    const char *port_str_p;
    UI8_T  port_ar[CGI_MODULE_INTERFACE_ID_LEN] = {0};
    UI32_T lport = 0;
    UI8_T  status = DHCPSNP_TYPE_PORT_UNTRUSTED;

    ifId_p = CGI_REQUEST_GetParamsValue(http_request, "ifId");

    if (NULL == ifId_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "ifID is NULL.");
    }

    port_str_p = json_string_value(ifId_p);
    CGI_UTIL_UrlDecode(port_ar, port_str_p);

    if (TRUE != CGI_UTIL_InterfaceIdToLport(port_ar, &lport))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid ifId.");
    }

    trust_p = CGI_REQUEST_GetBodyValue(http_request, "trustStatus");

    if (NULL != trust_p)
    {
        if (TRUE != json_is_boolean(trust_p))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The trust status is not boolean type.");
        }

        if (TRUE == json_boolean_value(trust_p))
        {
            status = DHCPSNP_TYPE_PORT_TRUSTED;
        }

        if (DHCPSNP_TYPE_OK != DHCPSNP_PMGR_SetPortTrustStatus(lport, status))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "dhcpsnp.setPortTrustStatusError", "Failed to set port trust status.");
        }
    } //if (NULL != trust_p)

#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION_CONFIGURABLE_CID == TRUE)
    {
        json_t  *cid_p = CGI_REQUEST_GetBodyValue(http_request, "circuitId");
        const char *cid_str_p;
        UI8_T cid_string_ar[SYS_ADPT_MAX_LENGTH_OF_CID + 1] = {0};
        UI8_T mode = SYS_DFLT_DHCPSNP_OPTION82_CID_MODE_DEFAULT;

        if (NULL != cid_p)
        {
            cid_str_p = json_string_value(cid_p);

            if (0 < strlen(cid_str_p))
            {
                strncpy(cid_string_ar, cid_str_p, sizeof(cid_string_ar));
                mode = DHCPSNP_TYPE_OPTION82_CID_CONFIGURED_STRING;
            }

            if (DHCPSNP_TYPE_OK != DHCPSNP_PMGR_SetInformationPortCidMode(lport, mode))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "dhcpsnp.setInformationPortCidModeError", "Failed to set port CID mode.");
            }

            if (DHCPSNP_TYPE_OK != DHCPSNP_PMGR_SetInformationPortCidValue(lport, cid_string_ar))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                        "dhcpsnp.setInformationPortCidValueError", "Failed to set port CID string.");
            }
        } //if (NULL != cid_p)
    }
#endif /* #if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION_CONFIGURABLE_CID == TRUE) */

#endif  /* #if (SYS_CPNT_DHCPSNP == TRUE) */

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
} //CGI_MODULE_DHCPSNP_INTF_ID_Update


void CGI_MODULE_DHCPSNP_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler   = CGI_MODULE_DHCPSNP_Read;
    handlers.update_handler = CGI_MODULE_DHCPSNP_Update;
    CGI_MAIN_Register("/api/v1/dhcp-snooping", &handlers, 0);

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_DHCPSNP_BINDING_Read;
        CGI_MAIN_Register("/api/v1/dhcp-snooping/binding", &handlers, 0);
    }

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.update_handler   = CGI_MODULE_DHCPSNP_BINDING_Clear;
        CGI_MAIN_Register("/api/v1/dhcp-snooping/binding:clear", &handlers, 0);
    }

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.update_handler   = CGI_MODULE_DHCPSNP_VLANS_ID_Update;
        CGI_MAIN_Register("/api/v1/dhcp-snooping/vlans/{vlanId}", &handlers, 0);
    }

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.update_handler = CGI_MODULE_DHCPSNP_INTF_Update;
        CGI_MAIN_Register("/api/v1/dhcp-snooping/interfaces", &handlers, 0);
    }

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.update_handler = CGI_MODULE_DHCPSNP_INTF_ID_Update;
        CGI_MAIN_Register("/api/v1/dhcp-snooping/interfaces/{ifId}", &handlers, 0);
    }
}

static void CGI_MODULE_DHCPSNP_Init()
{
    CGI_MODULE_DHCPSNP_RegisterHandlers();
}
