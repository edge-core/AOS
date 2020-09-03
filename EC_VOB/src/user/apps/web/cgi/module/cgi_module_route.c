#include "cgi_request.h"
#include "cgi_auth.h"
#include "cgi_util.h"
#include "ip_lib.h"
#include "l_inet.h"
#include "netcfg_pmgr_route.h"
#include "netcfg_pmgr_ip.h"
#if (SYS_CPNT_ARP == TRUE)
#include "netcfg_pom_route.h"
#include "netcfg_pmgr_nd.h"
#endif /* #if (SYS_CPNT_ARP == TRUE) */
#include "amtr_pmgr.h"

#define CGI_MODULE_ROUTE_LINUX_CMD_SIZE  128
#define CGI_MODULE_ROUTE_LINUX_RETURN_BUFFER_SIZE  256

static void CGI_MODULE_ROUTE_GetMacEntry(UI8_T *mac_p, int *vid_p, char *port_p);
static BOOL_T CGI_MODULE_ROUTE_ExecLinuxCommand(char *cmd_p, BOOL_T isSet);

/**----------------------------------------------------------------------
 * This API is used to create static route.
 *
 * @param destIp (required, string) Destination IP address
 * @param prefix (required, number) Prefix of destination IP address
 * @param vrf (optional, string) VRF name
 * @param nexthop (optional, array) Nexthop array
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ROUTE_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *dest_p;
    json_t  *prefix_p;
    json_t  *vrf_p;
    json_t  *nexthops_p;
    json_t  *nexthop_p;
    json_t  *ip_p;
    json_t  *mac_p;
    json_t  *vid_p;
    json_t  *port_p;
    const char*  dest_str_p;
    const char*  vrf_str_p = NULL;
    const char*  ip_str_p;
    const char*  mac_str_p;
    const char*  port_str_p;
    L_INET_AddrIp_T dest;
    L_INET_AddrIp_T next_hop;
    AMTR_TYPE_AddrEntry_T addr_entry;
    UI8_T  dip_ar[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    UI8_T  mask_ar[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    UI32_T idx = 0, nexthop_num = 0, lport = 0;
    UI32_T ret = 0;

    memset(&dest, 0, sizeof(dest));
    memset(&next_hop, 0, sizeof(next_hop));
    dest_p = CGI_REQUEST_GetBodyValue(http_request, "destIp");

    if (dest_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get dest IP.");
    }
    dest_str_p = json_string_value(dest_p);

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(
            L_INET_FORMAT_IP_UNSPEC, dest_str_p, (L_INET_Addr_T *)&dest, sizeof(dest)))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid dest IP format of route.");
    }

    prefix_p = CGI_REQUEST_GetBodyValue(http_request, "prefix");

    if (prefix_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get prefix.");
    }
    dest.preflen = json_integer_value(prefix_p);

    if ((dest.preflen > 0) && (dest.preflen < 8))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "prefix nubmber range is 8-32 and 0 (For default route)");
    }

    if ( ((dest.preflen > 0) && (IP_LIB_IsZeroNetwork(dest.addr) == TRUE))
            || (IP_LIB_IsIpInClassD(dest.addr) == TRUE)
            || (IP_LIB_IsIpInClassE(dest.addr) == TRUE)
            || (IP_LIB_IsLoopBackIp(dest.addr) == TRUE)
            || (IP_LIB_IsBroadcastIp(dest.addr) == TRUE)
            || (IP_LIB_IsMulticastIp(dest.addr) == TRUE)
            || (IP_LIB_IsTestingIp(dest.addr) == TRUE) )
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid dest IP of route.");
    }

    IP_LIB_CidrToMask(dest.preflen, mask_ar);

    if (IP_LIB_IsValidNetworkMask(mask_ar) == FALSE)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid dest IP prefix of route.");
    }

    vrf_p = CGI_REQUEST_GetBodyValue(http_request, "vrf");

    if (vrf_p != NULL)
    {
        vrf_str_p = json_string_value(vrf_p);
    }

    nexthops_p = CGI_REQUEST_GetBodyValue(http_request, "nexthop");

    if (nexthops_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get nexthop.");
    }

    nexthop_num = json_array_size(nexthops_p);

    for (idx = 0; idx < nexthop_num; idx ++)
    {
        nexthop_p = json_array_get(nexthops_p, idx);

        if (NULL == nexthop_p)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get a nexthop.");
        }

        ip_p = json_object_get(nexthop_p, "ip");

        if (NULL == ip_p)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get a nexthop IP.");
        }

        ip_str_p = json_string_value(ip_p);

        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(
                L_INET_FORMAT_IP_UNSPEC, ip_str_p, (L_INET_Addr_T *)&next_hop, sizeof(next_hop)))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid gateway format of route.");
        }

        //if (0 != memcmp(ip_str_p, dest_str_p, strlen(ip_str_p)))
        {
            char cmd_ar[CGI_MODULE_ROUTE_LINUX_CMD_SIZE] = {0};

            if (NULL == vrf_str_p)
            {
                sprintf(cmd_ar, "vtysh -c \"config terminal\" -c \"ip route %s/%d %s\"",
                        dest_str_p, dest.preflen, ip_str_p);
            }
            else
            {
                sprintf(cmd_ar, "vtysh -c \"config terminal\" -c \"ip route %s/%d %s vrf %s\"",
                        dest_str_p, dest.preflen, ip_str_p, vrf_str_p);
            }

            if (TRUE != CGI_MODULE_ROUTE_ExecLinuxCommand(cmd_ar, TRUE))
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "Failed", "Failed to set ip route.");
            }
        }

        mac_p = json_object_get(nexthop_p, "destMac");

        if (NULL != mac_p)
        {
#if (SYS_CPNT_ARP == TRUE)
            NETCFG_TYPE_PhysAddress_T  phys;

            memset(&phys, 0, sizeof(phys));
            phys.phy_address_type = 1/*SN_LAN*/;
            phys.phy_address_len = SYS_ADPT_MAC_ADDR_LEN;
            mac_str_p = json_string_value(mac_p);

            if (TRUE != CGI_UTIL_MacStrToHex(mac_str_p, phys.phy_address_cctet_string))
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to parse MAC address.");
            }

            ret = NETCFG_PMGR_ND_AddStaticIpNetToPhysicalEntry(0, &next_hop, &phys);
            switch(ret)
            {
                case NETCFG_TYPE_OK: /* do nothing */
                    break;
                case NETCFG_TYPE_CAN_NOT_ADD_LOCAL_IP:
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "arp.AddStaticIpNetToPhysicalEntry",
                            "The added IP address is the same as local IP address; the add action is aborted.");
                case NETCFG_TYPE_TABLE_FULL:
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "arp.AddStaticIpNetToPhysicalEntry",
                            "The static ARP Cache is full; the add action is aborted.");
                case NETCFG_TYPE_ENTRY_EXIST:
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "arp.AddStaticIpNetToPhysicalEntry",
                            "The added IP address exists; the add action is aborted.");
                case NETCFG_TYPE_INVALID_ARG:
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "arp.AddStaticIpNetToPhysicalEntry",
                            "The added IP address or physical address is invalid; the add action is aborted.");
                default:
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "arp.AddStaticIpNetToPhysicalEntry",
                            "Failed to set the static ARP cache entry.");
            }
#endif  /*#if (SYS_CPNT_ARP == TRUE)*/

            vid_p = json_object_get(nexthop_p, "vid");

            if (NULL != vid_p)
            {
                memset(&addr_entry, 0, sizeof(addr_entry));
                addr_entry.vid = json_integer_value(vid_p);
                port_p = json_object_get(nexthop_p, "port");

                if (NULL == port_p)
                {
                    return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get port.");
                }

                port_str_p = json_string_value(port_p);

                if (TRUE != CGI_UTIL_InterfaceIdToLport(port_str_p, &lport))
                {
                    return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid port.");
                }

                addr_entry.life_time= AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET;
                addr_entry.source= AMTR_TYPE_ADDRESS_SOURCE_CONFIG;
                addr_entry.action= AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
                memcpy(addr_entry.mac, phys.phy_address_cctet_string, SYS_ADPT_MAC_ADDR_LEN);
                addr_entry.ifindex = lport;

                if (!AMTR_PMGR_SetAddrEntry(&addr_entry))
                {
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "mac.SetAddrEntry", "Failed.");
                }
            } //vid_p to set static MAC entry
        } //ip_p to set "arp" & "mac-addr static"
    } //for nexthop

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to read all IP routes.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ROUTE_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *routes_p = json_array();
    char ip_ar[20] = {0};
    char gateway_ar[16] = {0};
    char mac_ar[18] = {0};
    char port_ar[8] = {0};
    int vid = 0;
    NETCFG_TYPE_IpCidrRouteEntry_T  entry;
    NETCFG_TYPE_StaticIpNetToPhysicalEntry_T phys_entry;

    memset(&entry, 0, sizeof(entry));
    entry.route_dest.type = L_INET_ADDR_TYPE_IPV4;

    while (NETCFG_POM_ROUTE_GetNextRunningStaticIpCidrRouteEntry(&entry) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS)
    {
        json_t *route_obj_p = json_object();
        json_t *nexthops_p = json_array();
        json_t *nexthop_obj_p = json_object();

        memset(&ip_ar, 0, sizeof(ip_ar));
        memset(&gateway_ar, 0, sizeof(gateway_ar));
        memset(&phys_entry.ip_net_to_physical_entry, 0, sizeof(phys_entry.ip_net_to_physical_entry));

        sprintf(ip_ar, "%d.%d.%d.%d", ((UI8_T *)(&(entry.route_dest.addr)))[0],
                ((UI8_T *)(&(entry.route_dest.addr)))[1], ((UI8_T *)(&(entry.route_dest.addr)))[2],
                ((UI8_T *)(&(entry.route_dest.addr)))[3]);
        sprintf(gateway_ar, "%d.%d.%d.%d", ((UI8_T *)(&(entry.route_next_hop.addr)))[0],
                ((UI8_T *)(&(entry.route_next_hop.addr)))[1], ((UI8_T *)(&(entry.route_next_hop.addr)))[2],
                ((UI8_T *)(&(entry.route_next_hop.addr)))[3]);

        json_object_set_new(route_obj_p, "destIp", json_string(ip_ar));
        json_object_set_new(route_obj_p, "prefix", json_integer(entry.route_dest.preflen));
        json_object_set_new(nexthop_obj_p, "ip", json_string(gateway_ar));

        while (NETCFG_POM_ND_GetNextStaticEntry(L_INET_ADDR_TYPE_IPV4, &phys_entry))
        {
            if (0 != memcmp(phys_entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.addr,
                    entry.route_next_hop.addr, sizeof(entry.route_next_hop.addr)))
            {
                continue;
            }

            sprintf(mac_ar, "%02X-%02X-%02X-%02X-%02X-%02X", L_INET_EXPAND_MAC(phys_entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_cctet_string));
            json_object_set_new(nexthop_obj_p, "destMac", json_string(mac_ar));
            CGI_MODULE_ROUTE_GetMacEntry(
                    phys_entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_cctet_string,
                    &vid, port_ar);
            if (0 != vid)
            {
                json_object_set_new(nexthop_obj_p, "vid", json_integer(vid));
                json_object_set_new(nexthop_obj_p, "port", json_string(port_ar));
            }
            break;
        } //NETCFG_POM_ND_GetNextStaticEntry

        json_array_append_new(nexthops_p, nexthop_obj_p);
        json_object_set_new(route_obj_p, "nexthop", nexthops_p);
        json_array_append_new(routes_p, route_obj_p);
    } //NETCFG_POM_ROUTE_GetNextRunningStaticIpCidrRouteEntry

    //maybe host route without net route setting
    memset(&phys_entry.ip_net_to_physical_entry, 0, sizeof(phys_entry.ip_net_to_physical_entry));

    while (NETCFG_POM_ND_GetNextStaticEntry(L_INET_ADDR_TYPE_IPV4, &phys_entry))
    {
        json_t *route_obj_p = json_object();
        json_t *nexthops_p = json_array();
        json_t *nexthop_obj_p = json_object();
        BOOL_T already_read = FALSE;

        memset(&entry, 0, sizeof(entry));
        entry.route_dest.type = L_INET_ADDR_TYPE_IPV4;

        while (NETCFG_POM_ROUTE_GetNextRunningStaticIpCidrRouteEntry(&entry) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS)
        {
            if (0 == memcmp(phys_entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.addr,
                    entry.route_next_hop.addr, sizeof(entry.route_next_hop.addr)))
            {
                already_read = TRUE;
                break;
            }
        }

        if (TRUE == already_read)
        {
            continue;
        }

        sprintf(ip_ar, "%d.%d.%d.%d",
                ((UI8_T *)(&(phys_entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.addr)))[0],
                ((UI8_T *)(&(phys_entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.addr)))[1],
                ((UI8_T *)(&(phys_entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.addr)))[2],
                ((UI8_T *)(&(phys_entry.ip_net_to_physical_entry.ip_net_to_physical_net_address.addr)))[3]);
        json_object_set_new(route_obj_p, "destIp", json_string(ip_ar));
        json_object_set_new(route_obj_p, "prefix", json_integer(32));
        json_object_set_new(nexthop_obj_p, "ip", json_string(ip_ar));
        sprintf(mac_ar, "%02X-%02X-%02X-%02X-%02X-%02X", L_INET_EXPAND_MAC(
                phys_entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_cctet_string));
        json_object_set_new(nexthop_obj_p, "destMac", json_string(mac_ar));
        CGI_MODULE_ROUTE_GetMacEntry(
                phys_entry.ip_net_to_physical_entry.ip_net_to_physical_phys_address.phy_address_cctet_string,
                &vid, port_ar);
        if (0 != vid)
        {
            json_object_set_new(nexthop_obj_p, "vid", json_integer(vid));
            json_object_set_new(nexthop_obj_p, "port", json_string(port_ar));
        }
        json_array_append_new(nexthops_p, nexthop_obj_p);
        json_object_set_new(route_obj_p, "nexthop", nexthops_p);
        json_array_append_new(routes_p, route_obj_p);
    } //NETCFG_POM_ND_GetNextStaticEntry

    json_object_set_new(result_p, "ipRoutes", routes_p);
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete route.
 *
 * @param id (required, string) route ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ROUTE_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    char* id_str_p;
    char *save_str_addr = NULL;
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    char *dip_str_p;
    char *prefix_str_p;
    char *gw_str_p;
    char *mac_p;
    char *vid_p;
    char *vrf_p;
    char *end_p;
    UI8_T  dip_ar[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    UI8_T  mask_ar[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    L_INET_AddrIp_T dest;
    L_INET_AddrIp_T next_hop;
    AMTR_TYPE_AddrEntry_T addr_entry;
    UI32_T prefix = 0;
    UI32_T ret = 0;

    memset(&dest, 0, sizeof(dest));
    memset(&next_hop, 0, sizeof(next_hop));
    memset(&addr_entry, 0, sizeof(addr_entry));
    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get route ID.");
    }

    id_str_p = (char *) json_string_value(id_p);
    dip_str_p = strtok_r(id_str_p, "_", &save_str_addr);
    prefix_str_p = strtok_r(NULL, "_", &save_str_addr);
    prefix = strtol(prefix_str_p, &end_p, 0);
    gw_str_p = strtok_r(NULL, "_", &save_str_addr);
    mac_p = strtok_r(NULL, "_", &save_str_addr);
    vid_p = strtok_r(NULL, "_", &save_str_addr);
    addr_entry.vid = strtol(vid_p, &end_p, 0);
    vrf_p = strtok_r(NULL, "_", &save_str_addr);

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(
            L_INET_FORMAT_IP_UNSPEC, dip_str_p, (L_INET_Addr_T *) &dest, sizeof(dest)))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid dest IP of route ID.");
    }

    dest.preflen = prefix;

    if (0 != memcmp(gw_str_p, "0", 1))
    {
        char cmd_ar[CGI_MODULE_ROUTE_LINUX_CMD_SIZE] = {0};

        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(
                L_INET_FORMAT_IP_UNSPEC, gw_str_p, (L_INET_Addr_T *) &next_hop, sizeof(next_hop)))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid nexthop IP of route ID.");
        }

        if (NULL == vrf_p)
        {
            sprintf(cmd_ar, "vtysh -c \"config terminal\" -c \"no ip route %s/%d %s\"",
                    dip_str_p, prefix, gw_str_p);
        }
        else
        {
            sprintf(cmd_ar, "vtysh -c \"config terminal\" -c \"no ip route %s/%d %s vrf %s\"",
                    dip_str_p, prefix, gw_str_p, vrf_p);
        }

        if (TRUE != CGI_MODULE_ROUTE_ExecLinuxCommand(cmd_ar, TRUE))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "Failed", "Failed to delete ip route.");
        }
    }

    if ((1 != strlen(mac_p)) && (0 != memcmp(mac_p, "0", 1)))
    {
        if (TRUE != CGI_UTIL_MacStrToHex(mac_p, addr_entry.mac))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to parse MAC address.");
        }

        if (!AMTR_PMGR_DeleteAddr(addr_entry.vid, addr_entry.mac))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "mac.DeleteAddr", "Failed to delete MAC.");
        }
    }

    dest.preflen = 32;
    ret = NETCFG_PMGR_ND_DeleteStaticIpNetToPhysicalEntry(0, &dest);
    switch(ret)
    {
        case NETCFG_TYPE_OK:
            break;
        case NETCFG_TYPE_ENTRY_NOT_EXIST:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "route.DeleteStaticIpNetToPhysicalEntry",
                    "The deleted static IP address does not exist; the delete action is aborted.");
        case NETCFG_TYPE_INVALID_ARG:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "route.DeleteStaticIpNetToPhysicalEntry",
                    "The deleted static IP address is invalid; the delete action is aborted.");
        default:
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR,
                    "route.DeleteStaticIpNetToPhysicalEntry", "Failed to delete this entry.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to create VRF interface.
 *
 * @param vrf (required, string) VRF name
 * @param tableId (required, number) table ID
 * @param vlanIds (required, array) binding VLAN array
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ROUTE_VRF_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vrf_p;
    json_t  *table_p;
    json_t  *vids_p;
    json_t  *vid_p;
    const char*  vrf_str_p = NULL;
    int table_id = 0;
    UI32_T idx = 0, vid_num = 0;

    vrf_p = CGI_REQUEST_GetBodyValue(http_request, "vrf");

    if (vrf_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vrf.");
    }
    vrf_str_p = json_string_value(vrf_p);

    table_p = CGI_REQUEST_GetBodyValue(http_request, "tableId");

    if (table_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get tableId.");
    }
    table_id = json_integer_value(table_p);

    {
        char cmd_ar[CGI_MODULE_ROUTE_LINUX_CMD_SIZE] = {0};

        sprintf(cmd_ar, "ip link add %s type vrf table %d", vrf_str_p, table_id);

        if (TRUE != CGI_MODULE_ROUTE_ExecLinuxCommand(cmd_ar, TRUE))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "Failed", "Failed to create vrf.");
        }
    }

    {
        char cmd_ar[CGI_MODULE_ROUTE_LINUX_CMD_SIZE] = {0};

        sprintf(cmd_ar, "ip link set dev %s up", vrf_str_p);

        if (TRUE != CGI_MODULE_ROUTE_ExecLinuxCommand(cmd_ar, TRUE))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "Failed", "Failed to set vrf up.");
        }
    }

    vids_p = CGI_REQUEST_GetBodyValue(http_request, "vlanIds");

    if (vids_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vlanIds.");
    }

    vid_num = json_array_size(vids_p);

    for (idx = 0; idx < vid_num; idx ++)
    {
        char cmd_ar[CGI_MODULE_ROUTE_LINUX_CMD_SIZE] = {0};
        int vid = 0;

        vid_p = json_array_get(vids_p, idx);

        if (NULL == vid_p)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get a vid.");
        }

        vid = json_integer_value(vid_p);
        sprintf(cmd_ar, "ip link set VLAN%d vrf %s", vid, vrf_str_p);

        if (TRUE != CGI_MODULE_ROUTE_ExecLinuxCommand(cmd_ar, TRUE))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "Failed", "Failed to bind vrf to vlan.");
        }
    }

    //root@test:/# ip link show
    //root@test:/# ip link show vrf v1
    //check cmd: test# show vrf
    //check cmd: test# show ip route vrf all
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to add VRF binding VLAN.
 *
 * @param vrf (required, string) VRF name
 * @param vlanIds (required, array) binding VLAN array
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ROUTE_VRF_ID_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vrf_p;
    json_t  *vids_p;
    json_t  *vid_p;
    const char*  vrf_str_p = NULL;
    UI32_T idx = 0, vid_num = 0;

    vrf_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (vrf_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vrf.");
    }
    vrf_str_p = json_string_value(vrf_p);
    vids_p = CGI_REQUEST_GetBodyValue(http_request, "vlanIds");

    if (vids_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vlanIds.");
    }

    vid_num = json_array_size(vids_p);

    for (idx = 0; idx < vid_num; idx ++)
    {
        char cmd_ar[CGI_MODULE_ROUTE_LINUX_CMD_SIZE] = {0};
        int vid = 0;

        vid_p = json_array_get(vids_p, idx);

        if (NULL == vid_p)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get a vid.");
        }

        vid = json_integer_value(vid_p);
        sprintf(cmd_ar, "ip link set VLAN%d vrf %s", vid, vrf_str_p);

        if (TRUE != CGI_MODULE_ROUTE_ExecLinuxCommand(cmd_ar, TRUE))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "Failed", "Failed to unbind vrf from vlan.");
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to remove VRF interface.
 *
 * @param vrf (required, string) VRF name
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ROUTE_VRF_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vrf_p;
    const char*  vrf_str_p = NULL;
    char cmd_ar[CGI_MODULE_ROUTE_LINUX_CMD_SIZE] = {0};

    vrf_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (vrf_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vrf.");
    }
    vrf_str_p = json_string_value(vrf_p);

    sprintf(cmd_ar, "ip link delete %s", vrf_str_p);

    if (TRUE != CGI_MODULE_ROUTE_ExecLinuxCommand(cmd_ar, TRUE))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "Failed", "Failed to delete vrf.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to unbind vlan from vrf.
 *
 * @param id (required, string) VRF name
 * @param vid (required, string) vlan id
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ROUTE_VRF_VLAN_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vrf_p;
    json_t  *vid_p;
    const char*  vrf_str_p = NULL;
    char cmd_ar[CGI_MODULE_ROUTE_LINUX_CMD_SIZE] = {0};
    int vid = 0;

    vrf_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (vrf_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vrf.");
    }
    vrf_str_p = json_string_value(vrf_p);

    vid_p = CGI_REQUEST_GetParamsValue(http_request, "vid");

    if (vid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vid.");
    }
    vid = json_integer_value(vid_p);

    sprintf(cmd_ar, "ip link set VLAN%d nomaster", vid);

    if (TRUE != CGI_MODULE_ROUTE_ExecLinuxCommand(cmd_ar, TRUE))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "Failed", "Failed to unbind vlan vrf.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

void CGI_MODULE_ROUTE_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler = CGI_MODULE_ROUTE_Read;
    handlers.create_handler = CGI_MODULE_ROUTE_Create;
    CGI_MAIN_Register("/api/v1/routes", &handlers, 0);

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.delete_handler = CGI_MODULE_ROUTE_ID_Delete;
        CGI_MAIN_Register("/api/v1/routes/{id}", &handlers, 0);
    }

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.create_handler = CGI_MODULE_ROUTE_VRF_Create;
        CGI_MAIN_Register("/api/v1/vrfs", &handlers, 0);
    }

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.update_handler = CGI_MODULE_ROUTE_VRF_ID_Update;
        handlers.delete_handler = CGI_MODULE_ROUTE_VRF_ID_Delete;
        CGI_MAIN_Register("/api/v1/vrfs/{id}", &handlers, 0);
    }

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.delete_handler = CGI_MODULE_ROUTE_VRF_VLAN_ID_Delete;
        CGI_MAIN_Register("/api/v1/vrfs/{id}/{vid}", &handlers, 0);
    }
}

static void CGI_MODULE_ROUTE_Init()
{
    CGI_MODULE_ROUTE_RegisterHandlers();
}

static void CGI_MODULE_ROUTE_GetMacEntry(UI8_T *mac_p, int *vid_p, char *port_p)
{
#if (SYS_CPNT_AMTR == TRUE)
    AMTR_TYPE_AddrEntry_T addr_entry;
    char port_ar[8] = {0};
    UI32_T unit = 0, port = 0, trunk = 0;

    memset(&addr_entry, 0, sizeof(addr_entry));
    memcpy(addr_entry.mac, mac_p, SYS_ADPT_MAC_ADDR_LEN);
//printf("get for %02X-%02X-%02X-%02X-%02X-%02X.\n", L_INET_EXPAND_MAC(mac_p));
    while(TRUE == AMTR_PMGR_GetNextMVAddrEntry(&addr_entry, AMTR_MGR_GET_ALL_ADDRESS))
    {//printf("loop %02X-%02X-%02X-%02X-%02X-%02X.\n", L_INET_EXPAND_MAC(addr_entry.mac));
        if (0 != memcmp(addr_entry.mac, mac_p, SYS_ADPT_MAC_ADDR_LEN))
        {
            *vid_p = 0;
            return;
        }

        if (SWCTRL_LPORT_TRUNK_PORT == SWCTRL_POM_LogicalPortToUserPort(addr_entry.ifindex, &unit, &port, &trunk))
        {
            sprintf(port_ar, "trunk%lu", (unsigned long)trunk);
        }
        else
        {
            sprintf(port_ar, "eth%lu/%lu", (unsigned long)unit, (unsigned long)port);
        }
//printf("get for %02X-%02X-%02X-%02X-%02X-%02X. result vid %d.\n", L_INET_EXPAND_MAC(mac_p), addr_entry.vid);
        *vid_p = addr_entry.vid;
        memcpy(port_p, port_ar, strlen(port_ar));
        return;
    }
#endif /* #if (SYS_CPNT_AMTR == TRUE) */
}

static BOOL_T CGI_MODULE_ROUTE_ExecLinuxCommand(char *cmd_p, BOOL_T isSet)
{
    FILE* fp_p;
    char buff_ar[CGI_MODULE_ROUTE_LINUX_RETURN_BUFFER_SIZE] = {0};
//printf("cmd: %s.\n", cmd_p);
    if (TRUE == isSet)
    {
        fp_p = popen(cmd_p, "w");
    }
    else
    {
        fp_p = popen(cmd_p, "r");
    }

    if (fp_p == NULL)
    {
        printf("ERROR, can execute command %s\r\n", cmd_p);
        return FALSE;
    }

    while (fgets(buff_ar, CGI_MODULE_ROUTE_LINUX_RETURN_BUFFER_SIZE, fp_p) != NULL)
    {
        printf("%s",buff_ar);
    }

    pclose(fp_p);
    return TRUE;
} //CGI_MODULE_ROUTE_SetLinuxCommand
