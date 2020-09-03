#include "cgi_request.h"
#include "cgi_auth.h"
#include "cgi_util.h"
#include "ip_lib.h"
#include "l_inet.h"
#include "bgp_pmgr.h"
#include "l4_pmgr.h"
#include "vlan_pom.h"
#if (SYS_CPNT_PBR == TRUE)
#include "netcfg_pmgr_pbr.h"
#include "netcfg_pom_pbr.h"
#endif

#define CGI_MODULE_ROUTE_LINUX_CMD_SIZE  128
#define CGI_MODULE_ROUTE_LINUX_RETURN_BUFFER_SIZE  256
/**----------------------------------------------------------------------
 * This API is used to create policy route.
 * include ACL, route-map, and bind to ingress vlan
 * cli cmd
 *   access-list IP extended name ...
 *   route-map name ...
 *   (vlan intf mode) ip policy route-map name
 *
 * @param name (required, string) Policy route name and ACL name
 * @param ingressVlans (required, array) Ingress VLAN to bind route-map
 * @param action (required, string) permit or deny
 * @param sequence (required, number) Sequence number <1-65535>
 * @param protocols (required, array) icmp, udp or tcp
 * @param matchDestIp (required, string) Matched destination IP with prefix
 * @param nexthop (required, string) Redirect nexthop
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_POLICY_ROUTE_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *name_p;
    json_t  *vids_p;
    json_t  *action_p;
    json_t  *seq_p;
    json_t  *nexthop_p;
    json_t  *protocols_p;
    json_t  *vrf_p;
    json_t  *dip_p;
    const char*  name_str_p;
    const char*  action_str_p;
    const char*  protocol_str_p;
    const char*  vrf_str_p;
    char*  dip_str_p;
    const char*  nexthop_str_p = NULL;
    char    *save_str_addr = NULL;
    char    *dip_addr_str_p;
    UI32_T vid = 0, pref_index = 0;
    UI32_T dip_addr = 0, mask_prefix = 0;
    UI32_T vid_num = 0, protocol_num = 0;
    UI32_T ret = 0;
    BOOL_T is_permit = FALSE;
    RULE_TYPE_UI_Ace_Entry_T ace;
    RULE_TYPE_RETURN_TYPE_T result;
    int idx = 0;

    memset(&ace, 0, sizeof(ace));
    ace.ace_type = RULE_TYPE_IP_EXT_ACL;
    L4_PMGR_ACL_InitUIAce(&ace);

    name_p = CGI_REQUEST_GetBodyValue(http_request, "name");

    if (name_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get name.");
    }
    name_str_p = json_string_value(name_p);

    vids_p = CGI_REQUEST_GetBodyValue(http_request, "ingressVlans");

    seq_p = CGI_REQUEST_GetBodyValue(http_request, "sequence");

    if (seq_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get sequence.");
    }
    pref_index = json_integer_value(seq_p);

    dip_p = CGI_REQUEST_GetBodyValue(http_request, "matchDestIp");

    if (dip_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get matchDestIp.");
    }
    dip_str_p = json_string_value(dip_p);

    nexthop_p = CGI_REQUEST_GetBodyValue(http_request, "nexthop");

    if (nexthop_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid nexthop.");
    }
    nexthop_str_p = json_string_value(nexthop_p);

    vrf_p = CGI_REQUEST_GetBodyValue(http_request, "vrf");

    if (vrf_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vrf.");
    }
    vrf_str_p = json_integer_value(vrf_p);

    {
        char cmd_ar[CGI_MODULE_ROUTE_LINUX_CMD_SIZE] = {0};
        sprintf(cmd_ar, "vtysh -c \"config terminal\" -c \"pbr-map %s seq %d\"",
                name_str_p, pref_index);
        sprintf(cmd_ar + strlen(cmd_ar), " -c \"match dst-ip %s\"", dip_str_p);
        sprintf(cmd_ar + strlen(cmd_ar), " -c \"set nexthop %s nexthop-vrf %d \"", nexthop_str_p, vrf_str_p);
        printf("cmd_ar = %s", cmd_ar);

        if (TRUE != CGI_MODULE_ROUTE_ExecLinuxCommand(cmd_ar, TRUE))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "Failed", "Failed to set PBR Map.");
        }
    }

    {
        char cmd_ar[CGI_MODULE_ROUTE_LINUX_CMD_SIZE] = {0};
        sprintf(cmd_ar, "vtysh -c \"config terminal\" -c \"interface %d\" -c \"pbr-policy %s\"",
                vrf_str_p, name_str_p);
        printf("cmd_ar = %s", cmd_ar);
        if (TRUE != CGI_MODULE_ROUTE_ExecLinuxCommand(cmd_ar, TRUE))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "Failed", "Failed to apply Policy Map to VRF Interface");
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);

/*    if (vids_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ingressVlans.");
    }

    vid_num = json_array_size(vids_p);

    if (0 == vid_num)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "NULL ingressVlans.");
    }

    for (idx=0; idx <vid_num; idx++)
    {
        json_t  *vid_p = json_array_get(vids_p, idx);

        if (NULL == vid_p)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get a VID.");
        }

        vid = json_integer_value(vid_p);

        if (TRUE != VLAN_POM_IsVlanExisted(vid))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Ingress VLAN does not exist.");
        }
    }

    action_p = CGI_REQUEST_GetBodyValue(http_request, "action");

    if (action_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get action.");
    }
    action_str_p = json_string_value(action_p);

    if (!strncmp(action_str_p, "permit", strlen("permit")))
    {
        is_permit = TRUE;
        ace.access = RULE_TYPE_ACE_PERMIT;
    }
    else if (!strncmp(action_str_p, "deny", strlen("deny")))
    {
        is_permit = FALSE;
        ace.access = RULE_TYPE_ACE_PERMIT; //ACL always be permit
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid action.");
    }

    seq_p = CGI_REQUEST_GetBodyValue(http_request, "sequence");

    if (seq_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get sequence.");
    }
    pref_index = json_integer_value(seq_p);

    dip_p = CGI_REQUEST_GetBodyValue(http_request, "matchDestIp");

    if (dip_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get matchDestIp.");
    }
    dip_str_p = json_string_value(dip_p);
    dip_addr_str_p = strtok_r((char *)dip_str_p, "/", &save_str_addr);

    if (TRUE != CGI_UTIL_IpStrToInt(dip_addr_str_p, &dip_addr))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid match DIP.");
    }

    if (sscanf(save_str_addr, "%u", &mask_prefix) != 1)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to parse match mask.");
    }

    IP_LIB_UI32toArray(dip_addr, ace.ipv4.dip.addr);
    IP_LIB_CidrToMask(mask_prefix, ace.ipv4.dip.mask);

    nexthop_p = CGI_REQUEST_GetBodyValue(http_request, "nexthop");

    if (nexthop_p != NULL)
     {
        L_INET_AddrIp_T addr;

        memset(&addr, 0, sizeof(L_INET_AddrIp_T));
        nexthop_str_p = json_string_value(nexthop_p);

        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                           nexthop_str_p,
                                                           (L_INET_Addr_T *)&addr,
                                                           sizeof(addr)))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid nexthop.");
        }
    }
    nexthop_str_p = json_string_value(nexthop_p);


    protocols_p = CGI_REQUEST_GetBodyValue(http_request, "protocols");

    if (protocols_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get protocols.");
    }

    protocol_num = json_array_size(protocols_p);

    if (0 == protocol_num)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "NULL protocols.");
    }

    for (idx=0; idx <protocol_num; idx++)
    {
        json_t  *proto_p = json_array_get(protocols_p, idx);

        if (NULL == proto_p)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get a protocol.");
        }

        protocol_str_p = json_string_value(proto_p);

        if (strncmp(protocol_str_p, "tcp", strlen("tcp"))
#if (SYS_CPNT_ACL_IP_EXT_ICMP == TRUE)
                && strncmp(protocol_str_p, "icmp", strlen("icmp"))
#endif
                && strncmp(protocol_str_p, "udp", strlen("udp")))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid protocol.");
        }
    } //for (idx=0; idx <protocol_num; idx++)

    result = L4_PMGR_ACL_CreateAcl((char *)name_str_p, RULE_TYPE_IP_EXT_ACL);

    if (RULE_TYPE_ACL_FULLED == result)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "acl.CreateAcl", "ACL table full.");
    }
    else if (RULE_TYPE_OK != result)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "acl.CreateAcl", "Failed to create ACL.");
    }

    for (idx=0; idx <protocol_num; idx++)
    {
        json_t  *proto_p = json_array_get(protocols_p, idx);
        protocol_str_p = json_string_value(proto_p);

        if (!strncmp(protocol_str_p, "tcp", strlen("tcp")))
        {
            ace.ipv4.protocol.u.s.data = RULE_TYPE_ACL_TCP_PROTOCOL;
        } else if (!strncmp(protocol_str_p, "udp", strlen("udp")))
        {
            ace.ipv4.protocol.u.s.data = RULE_TYPE_ACL_UDP_PROTOCOL;
        }
#if (SYS_CPNT_ACL_IP_EXT_ICMP == TRUE)
        else //icmp
        {
            ace.ipv4.protocol.u.s.data = RULE_TYPE_ACL_ICMP_PROTOCOL;
        }
#endif

        ace.ipv4.protocol.op = RULE_TYPE_IPV4_PROTOCOL_OP_EQUAL;
        ace.ipv4.protocol.u.s.mask = 255;

        if (RULE_TYPE_OK != L4_PMGR_ACL_SetUIAce2Acl((char *)name_str_p, RULE_TYPE_IP_EXT_ACL, &ace))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "acl.SetUIAce2Acl", "Failed to create ACE.");
        }
    } //for (idx=0; idx <protocol_num; idx++)

    ret = BGP_PMGR_AddRouteMap((char *)name_str_p);

    if (BGP_TYPE_RESULT_NO_MORE_ENTRY == ret)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "bgp.AddRouteMap", "Route-map table full.");
    }
    else if (BGP_TYPE_RESULT_OK != ret)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "bgp.AddRouteMap", "Failed to add route-map.");
    }

    ret = BGP_PMGR_AddRouteMapPref((char *)name_str_p, is_permit, pref_index);

    if (BGP_TYPE_RESULT_NO_MORE_ENTRY == ret)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "bgp.AddRouteMapPref", "Route-map preference table full.");
    }
    else if (BGP_TYPE_RESULT_OK != ret)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "bgp.AddRouteMapPref", "Failed to add route-map preference.");
    }

    if (BGP_TYPE_RESULT_OK != BGP_PMGR_AddRouteMapMatch((char *)name_str_p, is_permit, pref_index, "ip address", (unsigned char *)name_str_p))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "bgp.AddRouteMapMatch", "Failed to add route-map match.");
    }

    if (nexthop_p != NULL)
    {
        if (BGP_TYPE_RESULT_OK != BGP_PMGR_AddRouteMapSet((char *)name_str_p, is_permit, pref_index, "ip next-hop", (unsigned char *)nexthop_str_p))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "bgp.AddRouteMapSet", "Failed to add route-map set.");
        }
    }

#if (SYS_CPNT_PBR == TRUE)
    for (idx=0; idx <vid_num; idx++)
    {
        json_t  *vid_p = json_array_get(vids_p, idx);
        vid = json_integer_value(vid_p);

        if (NETCFG_TYPE_OK != NETCFG_PMGR_PBR_BindRouteMap(vid, (char *)name_str_p))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netcfg.BindRouteMap", "Failed to bind route-map.");
        }
    }
#endif
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
*/

}

/**----------------------------------------------------------------------
 * This API is used to delete route.
 *
 * @param id (required, string) route ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_POLICY_ROUTE_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    const char* id_str_p;
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get Policy Route Name.");
    }

    id_str_p = json_string_value(id_p);

    
    char cmd_ar[CGI_MODULE_ROUTE_LINUX_CMD_SIZE] = {0};
    sprintf(cmd_ar, "vtysh -c \"config terminal\" -c \"no pbr-map %s\"", id_str_p);

    if (TRUE != CGI_MODULE_ROUTE_ExecLinuxCommand(cmd_ar, TRUE))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "Failed", "Failed to Delete PBR Map.");
    }

/*
#if (SYS_CPNT_PBR == TRUE)
    {
        NETCFG_OM_PBR_BindingEntry_T  pbr_binding;

        memset(&pbr_binding, 0, sizeof(pbr_binding));

        while (NETCFG_POM_PBR_GetNextBindingEntry(&pbr_binding))
        {
            if (!strncmp(pbr_binding.rmap_name, id_str_p, strlen(id_str_p)))
            {
                if(NETCFG_TYPE_OK != NETCFG_PMGR_PBR_UnbindRouteMap(pbr_binding.vid))
                {
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netcfg.UnbindRouteMap", "Failed to unbind route-map.");
                }
            }
        }
    }
#endif

    if (BGP_TYPE_RESULT_OK != BGP_PMGR_DeleteRouteMap((char*)id_str_p))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "bgp.DeleteRouteMap", "Failed to delete this entry.");
    }

    if (RULE_TYPE_FAIL == L4_PMGR_ACL_DelAclByNameAndAclType(id_str_p, RULE_TYPE_IP_EXT_ACL))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "acl.DelAclByNameAndAclType", "Failed to delete ACL.");
    }
*/

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

void CGI_MODULE_POLICY_ROUTE_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.create_handler = CGI_MODULE_POLICY_ROUTE_Create;
    CGI_MAIN_Register("/api/v1/policy-routes", &handlers, 0);

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.delete_handler = CGI_MODULE_POLICY_ROUTE_ID_Delete;
        CGI_MAIN_Register("/api/v1/policy-routes/{id}", &handlers, 0);
    }
}

static void CGI_MODULE_POLICY_ROUTE_Init()
{
    CGI_MODULE_POLICY_ROUTE_RegisterHandlers();
}

