#include "cgi_auth.h"
#include "cgi_util.h"
#include "vxlan_type.h"
#include "vxlan_pmgr.h"
#include "vxlan_pom.h"
#include "swctrl.h"

static BOOL_T CGI_MODULE_VXLAN_ParseAccessPortId(char *str, UI32_T *lport_p, UI16_T *vid_p);
static BOOL_T CGI_MODULE_VXLAN_GetSrcInterface(char *str, UI32_T *ifindex_p);

/**----------------------------------------------------------------------
 * This API is used to read VXLAN information.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VXLAN_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    UI32_T  source_ifindex;
    UI16_T  udp_dst_port = 0;
    char    id_ar[20] = {0};

    if (VXLAN_POM_GetUdpDstPort(&udp_dst_port) != VXLAN_TYPE_RETURN_OK)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vxlan.getUdpDstPortError", "Failed to get udpDstPort.");
    }

    if (VXLAN_POM_GetSrcIf(&source_ifindex) != VXLAN_TYPE_RETURN_OK)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vxlan.getUdpDstPortError", "Failed to get udpDstPort.");
    }
    else
    {
        memset(id_ar, 0, sizeof(id_ar));
        if (source_ifindex != 0)
        {
            if (IS_VLAN_IFINDEX_VAILD(source_ifindex))
            {
                UI32_T vid;
                VLAN_OM_ConvertFromIfindex(source_ifindex, &vid);
                sprintf(id_ar, "vlan%lu", (unsigned long)vid);
            }
#if (SYS_CPNT_LOOPBACK_IF_VISIBLE == TRUE)
            else if (source_ifindex >= SYS_ADPT_LOOPBACK_IF_INDEX_NUMBER &&
                     source_ifindex < SYS_ADPT_LOOPBACK_IF_INDEX_NUMBER + SYS_ADPT_MAX_NBR_OF_LOOPBACK_IF)
            {
                UI32_T lo_id;
                IP_LIB_ConvertLoopbackIfindexToId(source_ifindex, &lo_id);
                sprintf(id_ar, "lo%lu", (unsigned long)lo_id);
            }
#endif
        }
    }
    json_object_set_new(result_p, "udpDstPort", json_integer(udp_dst_port));
    json_object_set_new(result_p, "srcInterface", json_string(id_ar));
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to set udpDstPort.
 *
 * @param udpDstPort (required, number) UDP destination port number
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VXLAN_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *udp_dst_port_p;
    json_t  *src_interface_p;
    char    *src_interface_str_p;
    UI32_T  ret;
    UI32_T  source_ifindex;
    UI16_T  udp_dst_port = 0;
    char    buffer_ar[20] = {0};

    udp_dst_port_p = CGI_REQUEST_GetBodyValue(http_request, "udpDstPort");
    src_interface_p = CGI_REQUEST_GetBodyValue(http_request, "srcInterface");
    if ((NULL == udp_dst_port_p) && (src_interface_p == NULL))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get udpDstPort and srcInterface.");
    }

    if (NULL != udp_dst_port_p)
    {
        udp_dst_port = json_integer_value(udp_dst_port_p);
        ret = VXLAN_PMGR_SetUdpDstPort(udp_dst_port);
        if (VXLAN_TYPE_ENTRY_EXISTED == ret)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to set udpDstPort because VTEP is created.");
        }
        else if (VXLAN_TYPE_RETURN_OK != ret)
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vxlan.setUdpDstPortError", "Failed to set VXLAN UDP port.");
        }
    }

    if (NULL != src_interface_p)
    {
        src_interface_str_p = (char *)json_string_value(src_interface_p);
        if (TRUE == CGI_MODULE_VXLAN_GetSrcInterface(src_interface_str_p, &source_ifindex))
        {
            if (VXLAN_TYPE_RETURN_OK != VXLAN_PMGR_SetSrcIf(source_ifindex))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vxlan.setSrcIfVlanError", "Failed to set VXLAN source interface VLAN.");
            }
        }
        else
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get srcInterface.");
        }
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to read all VLAN-VNI mapping entris.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VXLAN_VPNs_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vpns_p = json_array();
    char    id_ar[20] = {0};
    VXLAN_OM_VNI_T vni_entry;

    json_object_set_new(result_p, "vpns", vpns_p);
    memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
    while (VXLAN_POM_GetNextVniEntry(&vni_entry) == VXLAN_TYPE_RETURN_OK)
    {
        json_t *vpn_obj_p = json_object();
        sprintf(id_ar, "%lu",(unsigned long)vni_entry.vni);
        json_object_set_new(vpn_obj_p, "id", json_string(id_ar));
        json_object_set_new(vpn_obj_p, "vni", json_integer(vni_entry.vni));
        json_array_append_new(vpns_p, vpn_obj_p);
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to configure VLAN and VNI mapping relationship.
 *
 * @param vid (required, number) VLAN ID
 *        vni (required, number) VXLAN ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VXLAN_VPNs_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vid_p;
    json_t  *vni_p;
    UI32_T  ret;
    UI32_T  vni = 0;
    char    id_ar[20] = {0};

    vni_p = CGI_REQUEST_GetBodyValue(http_request, "vni");
    if (vni_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get VXLAN ID.");
    }
    vni = json_integer_value(vni_p);

    ret = VXLAN_PMGR_AddVpn(vni);

    if (ret == VXLAN_TYPE_ENTRY_EXISTED)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "VXLAN alread exist.");
    }
    else if (ret != VXLAN_TYPE_RETURN_OK)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vxlan.AddVpnError", "Failed to create VPN with this VNI.");
    }

    sprintf(id_ar, "%lu", (unsigned long)vni);
    json_object_set_new(result_p, "id", json_string(id_ar));
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to unset VLAN and VNI mapping relationship.
 *
 * @param id (required, string) vid_vni
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VXLAN_VPNs_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    char    *id_str_p, *end_p;
    UI32_T  ret;
    UI32_T  vni = 0;
    UI16_T  vid = 0;
    UI8_T   id_ar[20] = {0};

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get VPN ID.");
    }

    id_str_p = (char *)json_string_value(id_p);
    vni = strtol(id_str_p, &end_p, 0);

    if (vni < VXLAN_TYPE_VNI_ID_MIN || vni > VXLAN_TYPE_VNI_ID_MAX)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Wrong VPN ID format.");
    }

    ret = VXLAN_PMGR_DeleteVpn(vni);

    if (ret == VXLAN_TYPE_ENTRY_NOT_EXISTED)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "VXLAN with this VNI not exist.");
    }
    else if (ret == VXLAN_TYPE_ENTRY_EXISTED)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Have VTEP port..");
    }
    else if (ret != VXLAN_TYPE_RETURN_OK)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vxlan.deleteVpnError", "Failed to delete VPN with this VNI.");
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to get all flooding remote VTEPs.
 *
 * @param vni (required, number) VXLAN ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VXLAN_FloodRvteps_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vni_p;
    json_t  *rvteps_p = json_array();
    UI32_T  vni = 0;
    UI32_T  unit = 0, port = 0, trunk_id = 0;
    UI32_T  vid = 0;
    UI8_T   id_ar[20] = {0};
    char    s_ip_ar[20] = {0};
    char    port_ar[20] = {0};
    char    n_ip_ar[20] = {0};
    VXLAN_OM_RVtep_T rvtep_entry;
    SWCTRL_Lport_Type_T lport_type;

    memset(&rvtep_entry, 0, sizeof(rvtep_entry));
    vni_p = CGI_REQUEST_GetParamsValue(http_request, "vni");
    if (vni_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vni.");
    }
    vni = json_integer_value(vni_p);
    json_object_set_new(result_p, "rvteps", rvteps_p);

    memset(&rvtep_entry, 0, sizeof(rvtep_entry));
    rvtep_entry.vni = vni;
    while (VXLAN_POM_GetNextFloodRVtepByVni(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
    {
        json_t *rvtep_obj_p = json_object();
        json_t *nexthops_p = json_array();
        UI32_T i;

        memset(id_ar, 0, sizeof(id_ar));
        memset(s_ip_ar, 0, sizeof(s_ip_ar));
        memset(port_ar, 0, sizeof(port_ar));

        sprintf(id_ar, "%d.%d.%d.%d",rvtep_entry.ip.addr[0], rvtep_entry.ip.addr[1],
            rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
        sprintf(s_ip_ar, "%d.%d.%d.%d", rvtep_entry.s_ip.addr[0], rvtep_entry.s_ip.addr[1],
            rvtep_entry.s_ip.addr[2], rvtep_entry.s_ip.addr[3]);

        json_object_set_new(rvtep_obj_p, "nexthops", nexthops_p);
        for (i=0; i<rvtep_entry.nexthop_cnt; i++)
        {
            json_t *nexthop_obj_p = json_object();

            sprintf(n_ip_ar, "%d.%d.%d.%d", rvtep_entry.nexthops_ip_ar[i].addr[0], rvtep_entry.nexthops_ip_ar[i].addr[1],
                rvtep_entry.nexthops_ip_ar[i].addr[2], rvtep_entry.nexthops_ip_ar[i].addr[3]);

            lport_type = SWCTRL_POM_LogicalPortToUserPort(rvtep_entry.nexthops_lport_ar[i], &unit, &port, &trunk_id);
            if(lport_type == SWCTRL_LPORT_NORMAL_PORT)
            {
                sprintf(port_ar, "eth%lu/%lu", (unsigned long)unit, (unsigned long)port);
            }
            else if (lport_type == SWCTRL_LPORT_TRUNK_PORT)
            {
                sprintf(port_ar, "trunk%lu", (unsigned long)trunk_id);
            }
            else
            {
                /* No port.
                 */
                sprintf(port_ar, "%s", "");
            }
            VLAN_IFINDEX_CONVERTTO_VID(rvtep_entry.nexthops_if_ar[i], vid);
            json_object_set_new(nexthop_obj_p, "ip", json_string(n_ip_ar));
            json_object_set_new(nexthop_obj_p, "vid", json_integer(vid));
            json_object_set_new(nexthop_obj_p, "port", json_string(port_ar));
            json_array_append_new(nexthops_p, nexthop_obj_p);
        }
        json_object_set_new(rvtep_obj_p, "id", json_string(id_ar));
        json_object_set_new(rvtep_obj_p, "dstIp", json_string(id_ar));
        json_object_set_new(rvtep_obj_p, "srcIp", json_string(s_ip_ar));
        json_array_append_new(rvteps_p, rvtep_obj_p);
    }

    memset(&rvtep_entry, 0, sizeof(rvtep_entry));
    rvtep_entry.vni = vni;
    if (VXLAN_POM_GetFloodMulticastByVni(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
    {
        json_t *rvtep_obj_p = json_object();

        memset(id_ar, 0, sizeof(id_ar));
        memset(s_ip_ar, 0, sizeof(s_ip_ar));
        memset(port_ar, 0, sizeof(port_ar));
        lport_type = SWCTRL_POM_LogicalPortToUserPort(rvtep_entry.lport, &unit, &port, &trunk_id);
        sprintf(id_ar, "%d.%d.%d.%d",rvtep_entry.ip.addr[0], rvtep_entry.ip.addr[1],
            rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
        sprintf(s_ip_ar, "%d.%d.%d.%d", rvtep_entry.s_ip.addr[0], rvtep_entry.s_ip.addr[1],
            rvtep_entry.s_ip.addr[2], rvtep_entry.s_ip.addr[3]);
        if(lport_type == SWCTRL_LPORT_NORMAL_PORT)
        {
            sprintf(port_ar, "eth%lu/%lu", (unsigned long)unit, (unsigned long)port);
        }
        else if (lport_type == SWCTRL_LPORT_TRUNK_PORT)
        {
            sprintf(port_ar, "trunk%lu", (unsigned long)trunk_id);
        }
        else
        {
            /* No port.
             */
            sprintf(port_ar, "%s", "");
        }
        json_object_set_new(rvtep_obj_p, "id", json_string(id_ar));
        json_object_set_new(rvtep_obj_p, "dstIp", json_string(id_ar));
        json_object_set_new(rvtep_obj_p, "srcIp", json_string(s_ip_ar));
        json_object_set_new(rvtep_obj_p, "vid", json_integer(rvtep_entry.vid));
        json_object_set_new(rvtep_obj_p, "port", json_string(port_ar));
        json_array_append_new(rvteps_p, rvtep_obj_p);
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to create a flooding remote VTEP.
 *
 * @param vni (required, number) VXLAN ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VXLAN_FloodRvteps_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vni_p;
    json_t  *ip_p;
    json_t  *vid_p;
    json_t  *port_p;
    UI32_T  vni;
    char    *ip_str_p;
    UI32_T  vid = 0;
    char    *port_str_p;
    UI32_T  unit = 0, port = 0, lport = 0;
    UI32_T  ret;
    VXLAN_OM_RVtep_T rvtep_entry;
    L_INET_AddrIp_T addr_ip;

    memset(&rvtep_entry, 0, sizeof(rvtep_entry));
    memset(&addr_ip, 0, sizeof(L_INET_AddrIp_T));

    vni_p = CGI_REQUEST_GetParamsValue(http_request, "vni");
    if (vni_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vni.");
    }
    vni = json_integer_value(vni_p);
    ip_p = CGI_REQUEST_GetBodyValue(http_request, "dstIp");
    if (ip_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get dstIp.");
    }
    ip_str_p = (char *)json_string_value(ip_p);
    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                       ip_str_p,
                                                       (L_INET_Addr_T *) &addr_ip,
                                                       sizeof(addr_ip)))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid address format.");
    }

    vid_p = CGI_REQUEST_GetBodyValue(http_request, "vid");
    port_p = CGI_REQUEST_GetBodyValue(http_request, "port");
    if ((vid_p == NULL) && (port_p == NULL))
    {
        /* Unicast.
         */
        ret = VXLAN_PMGR_AddFloodRVtep(vni, &addr_ip);
        if (ret == VXLAN_TYPE_IP_INVALID)
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vxlan.addFloodRVtepError", "Invalid IP address.");
        }
        else if (ret == VXLAN_TYPE_TABLE_FULL)
        {
            CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vxlan.addFloodMulticastError", "VTEP table is full.");
        }
        else if (   (ret != VXLAN_TYPE_RETURN_OK)
                 && (ret != VXLAN_TYPE_HOST_NOT_FIND)
                 && (ret != VXLAN_TYPE_ROUTE_NOT_FIND)
                 &&  (ret != VXLAN_TYPE_SRC_IF_NOT_FIND)
                 &&  (ret != VXLAN_TYPE_SRC_IF_IP_NOT_FIND)
                )
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vxlan.addFloodRVtepError", "Failed to add flooding IP address.");
        }
    }
    else if ((vid_p != NULL) && (port_p != NULL))
    {
        /* Multicast.
         */
        vid = json_integer_value(vid_p);
        port_str_p = (char *)json_string_value(port_p);
        if (TRUE == CGI_UTIL_InterfaceIdToEth(port_str_p, &unit, &port))
        {
            SWCTRL_POM_UserPortToIfindex(unit, port, &lport);
            ret = VXLAN_PMGR_AddFloodMulticast(vni, &addr_ip, vid, lport);
            if (ret == VXLAN_TYPE_IP_INVALID)
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vxlan.addFloodMulticastError", "Invalid IP address.");
            }
            else if (ret == VXLAN_TYPE_VNI_NOT_MATCH)
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vxlan.addFloodMulticastError", "Not find VNI.");
            }
            else if (ret == VXLAN_TYPE_ENTRY_EXISTED)
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vxlan.addFloodMulticastError", "VNI had associated with multicast group.");
            }
            else if (ret == VXLAN_TYPE_TABLE_FULL)
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vxlan.addFloodMulticastError", "VTEP table is full.");
            }
            else if (   (ret != VXLAN_TYPE_RETURN_OK)
                     && (ret != VXLAN_TYPE_SRC_IP_NOT_FIND)
                     &&  (ret != VXLAN_TYPE_SRC_IF_NOT_FIND)
                     &&  (ret != VXLAN_TYPE_SRC_IF_IP_NOT_FIND)
                    )
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vxlan.addFloodMulticastError", "Failed to add multicast group IP address.");
            }
        }
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Please check. If the remote VTEP IP is multicast, shall have vid and port.");
    }

    json_object_set_new(result_p, "id", ip_p);
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete all flooding remote VTEPs
 *
 * @param vni (required, number) VXLAN ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VXLAN_FloodRvteps_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vni_p;
    UI32_T  vni = 0;
    VXLAN_OM_RVtep_T rvtep_entry;

    memset(&rvtep_entry, 0, sizeof(rvtep_entry));
    vni_p = CGI_REQUEST_GetParamsValue(http_request, "vni");
    if (vni_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vni.");
    }
    vni = json_integer_value(vni_p);
    memset(&rvtep_entry, 0, sizeof(rvtep_entry));
    rvtep_entry.vni = vni;
    while (VXLAN_POM_GetNextFloodRVtepByVni(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
    {
        if (VXLAN_PMGR_DelFloodRVtep(vni, &rvtep_entry.ip) != VXLAN_TYPE_RETURN_OK)
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vxlan.delFloodRVtepError", "Failed to delete flooding IP address.");
        }
    }

    memset(&rvtep_entry, 0, sizeof(rvtep_entry));
    rvtep_entry.vni = vni;
    if (VXLAN_POM_GetFloodMulticastByVni(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
    {
        if (VXLAN_PMGR_DelFloodMulticast(vni) != VXLAN_TYPE_RETURN_OK)
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vxlan.delFloodMulticastError", "Failed to delete multicast group IP address.");
        }
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete a flooding remote VTEP
 *
 * @param vni (required, number) VXLAN ID
 *        id ((required, string) Unique RVTEP ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VXLAN_FloodRvteps_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vni_p;
    json_t  *id_p;
    UI32_T  vni = 0;
    char    *id_str_p;
    BOOL_T  found = FALSE;
    VXLAN_OM_RVtep_T rvtep_entry;
    L_INET_AddrIp_T addr_ip;

    memset(&rvtep_entry, 0, sizeof(rvtep_entry));
    memset(&addr_ip, 0, sizeof(L_INET_AddrIp_T));
    vni_p = CGI_REQUEST_GetParamsValue(http_request, "vni");
    if (vni_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vni.");
    }
    vni = json_integer_value(vni_p);

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get remote VTEP ID.");
    }
    id_str_p = (char *)json_string_value(id_p);
    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                       id_str_p,
                                                       (L_INET_Addr_T *) &addr_ip,
                                                       sizeof(addr_ip)))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid id format.");
    }

    if (FALSE == IP_LIB_IsMulticastIp(addr_ip.addr))
    {
        /* Unicast
         */
        if (VXLAN_PMGR_DelFloodRVtep(vni, &addr_ip) != VXLAN_TYPE_RETURN_OK)
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vxlan.delFloodRVtepError", "Failed to delete flooding IP address.");
        }
    }
    else
    {
        /* Multicast, only one VTEP in one VNI.
         */
        rvtep_entry.vni = vni;
        if (VXLAN_POM_GetFloodMulticastByVni(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
        {
            if (0 == memcmp(&rvtep_entry.ip, &addr_ip, sizeof(L_INET_AddrIp_T)))
            {
                if (VXLAN_PMGR_DelFloodMulticast(vni) != VXLAN_TYPE_RETURN_OK)
                {
                    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vxlan.delFloodMulticastError", "Failed to delete multicast group IP address.");
                }
                found = TRUE;
            }
        }
        if (FALSE == found)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "No such remote VTEP ID.");
        }
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to get all access ports in a VNI
 *
 * @param vni (required, number) VXLAN ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VXLAN_AccessPorts_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vni_p;
    json_t  *access_ports_p = json_array();
    UI32_T  vni = 0, lport = 0;
    UI32_T  unit = 0, port = 0, trunk_id = 0;
    UI16_T  vid = 0;
    UI8_T   id_ar[20] = {0};
    char    port_ar[20] = {0};
    SWCTRL_Lport_Type_T lport_type;

    vni_p = CGI_REQUEST_GetParamsValue(http_request, "vni");
    if (vni_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vni.");
    }
    vni = json_integer_value(vni_p);

    json_object_set_new(result_p, "accessPorts", access_ports_p);

    while (VXLAN_POM_GetNextPortVlanVniMapByVni(vni, &lport, &vid) == VXLAN_TYPE_RETURN_OK)
    {
        json_t *access_obj_p = json_object();

        memset(id_ar, 0, sizeof(id_ar));
        memset(port_ar, 0, sizeof(port_ar));

        if (vid != 0)
            sprintf(id_ar, "%lu_%u", (unsigned long)lport, vid);
        else
            sprintf(id_ar, "%lu", (unsigned long)lport);

        lport_type = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);
        if(lport_type == SWCTRL_LPORT_NORMAL_PORT)
        {
            sprintf(port_ar, "eth%lu/%lu", (unsigned long)unit, (unsigned long)port);
        }
        else if (lport_type == SWCTRL_LPORT_TRUNK_PORT)
        {
            sprintf(port_ar, "trunk%lu", (unsigned long)trunk_id);
        }

        json_object_set_new(access_obj_p, "id", json_string(id_ar));
        json_object_set_new(access_obj_p, "port", json_string(port_ar));
        if (vid != 0)
            json_object_set_new(access_obj_p, "vid", json_integer(vid));
        json_array_append_new(access_ports_p, access_obj_p);
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to create a VXLAN access port
 *
 * @param vni (required, number) VXLAN ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VXLAN_AccessPorts_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vni_p;
    json_t  *vid_p;
    json_t  *port_p;
    UI32_T  vni;
    UI16_T  vid = 0;
    char    *port_str_p;
    UI32_T  unit = 0, port = 0, lport = 0;
    UI8_T   id_ar[20] = {0};

    vni_p = CGI_REQUEST_GetParamsValue(http_request, "vni");
    if (vni_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vni.");
    }
    vni = json_integer_value(vni_p);

    vid_p = CGI_REQUEST_GetBodyValue(http_request, "vid");
    port_p = CGI_REQUEST_GetBodyValue(http_request, "port");

    if (port_p == NULL)
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get port.");

    port_str_p = (char *)json_string_value(port_p);
    if (TRUE == CGI_UTIL_InterfaceIdToEth(port_str_p, &unit, &port))
    {
        vid = 0;
        if (vid_p != NULL)
            vid = json_integer_value(vid_p);

        SWCTRL_POM_UserPortToIfindex(unit, port, &lport);

        if (VXLAN_PMGR_SetPortVlanVniMap(lport, vid, vni, TRUE) != VXLAN_TYPE_RETURN_OK)
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vxlan.setPortVlanVniMapError", "Failed to add access port.");
        }
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Please check the port format.");
    }

    if (vid == 0)
        sprintf(id_ar, "%lu", (unsigned long)lport);
    else
        sprintf(id_ar, "%lu_%u", (unsigned long)lport, vid);
    json_object_set_new(result_p, "id", json_string(id_ar));
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete all VXLAN access ports
 *
 * @param vni (required, number) VXLAN ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VXLAN_AccessPorts_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vni_p;
    UI32_T  vni;
    UI32_T lport;
    UI16_T vid;

    vni_p = CGI_REQUEST_GetParamsValue(http_request, "vni");
    if (vni_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vni.");
    }
    vni = json_integer_value(vni_p);

    lport = 0;
    vid = 0;
    while (VXLAN_POM_GetNextPortVlanVniMapByVni(vni, &lport, &vid) == VXLAN_TYPE_RETURN_OK)
    {
        if (VXLAN_PMGR_SetPortVlanVniMap(lport, vid, vni, FALSE) != VXLAN_TYPE_RETURN_OK)
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vxlan.setPortVlanVniMapError", "Failed to delete access port.");
        }
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete an access port
 *
 * @param vni (required, number) VXLAN ID
 *        id ((required, string) Unique Access Port ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_VXLAN_AccessPorts_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *vni_p;
    json_t  *id_p;
    UI32_T  vni = 0;
    char    *id_str_p;
    UI32_T  lport;
    UI16_T  vid;

    vni_p = CGI_REQUEST_GetParamsValue(http_request, "vni");
    if (vni_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get vni.");
    }
    vni = json_integer_value(vni_p);

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get access port ID.");
    }
    id_str_p = (char *)json_string_value(id_p);

    if (!CGI_MODULE_VXLAN_ParseAccessPortId(id_str_p, &lport, &vid))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid id format.");
    }

    if (VXLAN_PMGR_SetPortVlanVniMap(lport, vid, vni, FALSE) != VXLAN_TYPE_RETURN_OK)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "vxlan.setPortVlanVniMapError", "Failed to delete access port.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}


void CGI_MODULE_VXLAN_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler = CGI_MODULE_VXLAN_Read;
    handlers.update_handler = CGI_MODULE_VXLAN_Update;
    CGI_MAIN_Register("/api/v1/vxlan", &handlers, 0);

    {   /* for "/api/v1/vxlan/vpns" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_VXLAN_VPNs_Read;
        handlers.create_handler = CGI_MODULE_VXLAN_VPNs_Create;
        CGI_MAIN_Register("/api/v1/vxlan/vpns", &handlers, 0);
    }

    {   /* for "/api/v1/vxlan/vpns/{id}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.delete_handler = CGI_MODULE_VXLAN_VPNs_ID_Delete;
        CGI_MAIN_Register("/api/v1/vxlan/vpns/{id}", &handlers, 0);
    }

    {   /* for "/api/v1/vxlan/{vni}/flood-rvteps" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_VXLAN_FloodRvteps_Read;
        handlers.create_handler = CGI_MODULE_VXLAN_FloodRvteps_Create;
        handlers.delete_handler = CGI_MODULE_VXLAN_FloodRvteps_Delete;
        CGI_MAIN_Register("/api/v1/vxlan/{vni}/flood-rvteps", &handlers, 0);
    }

    {   /* for "/api/v1/vxlan/{vni}/flood-rvteps/{id}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.delete_handler = CGI_MODULE_VXLAN_FloodRvteps_ID_Delete;
        CGI_MAIN_Register("/api/v1/vxlan/{vni}/flood-rvteps/{id}", &handlers, 0);
    }

    {   /* for "/api/v1/vxlan/{vni}/access-ports" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_VXLAN_AccessPorts_Read;
        handlers.create_handler = CGI_MODULE_VXLAN_AccessPorts_Create;
        handlers.delete_handler = CGI_MODULE_VXLAN_AccessPorts_Delete;
        CGI_MAIN_Register("/api/v1/vxlan/{vni}/access-ports", &handlers, 0);
    }

    {   /* for "/api/v1/vxlan/{vni}/access-ports/{id}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.delete_handler = CGI_MODULE_VXLAN_AccessPorts_ID_Delete;
        CGI_MAIN_Register("/api/v1/vxlan/{vni}/access-ports/{id}", &handlers, 0);
    }
}

static void CGI_MODULE_VXLAN_Init()
{
    CGI_MODULE_VXLAN_RegisterHandlers();
}

static BOOL_T CGI_MODULE_VXLAN_ParseAccessPortId(char *str, UI32_T *lport_p, UI16_T *vid_p)
{
    char buf_ar[128] = {0};
    char *strtok_state_p = NULL;
    char *lport_str_p, *vid_str_p;
    char *end_p;

    strncpy(buf_ar, str, strlen(str));
    strtok_state_p = buf_ar;

    lport_str_p = strtok_r(NULL, "_", &strtok_state_p);
    if (lport_str_p == NULL)
    {
        return FALSE;
    }
    else
    {
        *lport_p  = strtol(lport_str_p, &end_p, 0);
    }

    vid_str_p = strtok_r(NULL, "_", &strtok_state_p);
    if (vid_str_p == NULL)
    {
        return FALSE;
    }
    else
    {
        *vid_p  = strtol(vid_str_p, &end_p, 0);
    }

    return TRUE;
}

static BOOL_T CGI_MODULE_VXLAN_GetSrcInterface(char *str, UI32_T *ifindex_p)
{
    UI32_T vid, lo_id;

    if (strlen(str) == 0)
    {
        *ifindex_p = 0;
        return TRUE;
    }
    else
    {
#if (SYS_CPNT_LOOPBACK_IF_VISIBLE == TRUE)
        if (strlen(str) < 3)
            return FALSE;
        if (memcmp(str, "lo", 2) == 0)
        {
            lo_id = atoi(str + 2);
            if (lo_id >= SYS_ADPT_MAX_NBR_OF_LOOPBACK_IF)
                return FALSE;
            *ifindex_p = SYS_ADPT_LOOPBACK_IF_INDEX_NUMBER + lo_id;
            return TRUE;
        }
#endif

        if (strlen(str) < 5)
            return FALSE;
        if (memcmp(str, "vlan", 4) == 0)
        {
            vid = atoi(str + 4);
            return VLAN_OM_ConvertToIfindex(vid, ifindex_p);
        }

        return FALSE;
    }
}
