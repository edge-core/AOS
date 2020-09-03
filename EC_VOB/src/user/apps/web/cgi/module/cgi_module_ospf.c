#include "cgi_auth.h"
#include "netcfg_type.h"
#include "netcfg_pmgr_ospf.h"
#include "ospf_pmgr.h"
#include "ospf_type.h"
#include "ospf_mgr.h"
#include "ip_lib.h"
#include "sys_dflt.h"

//static BOOL_T CGI_MODULE_OSPF_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request);
static BOOL_T CGI_MODULE_OSPF_ParseNetworkId(char *str, UI32_T *ip, UI32_T *preflen, UI32_T *area);

/**----------------------------------------------------------------------
 * This API is used to read all OSPF processes and networks.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_OSPF_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *processes_p = json_array();
    UI32_T  vr_id = SYS_DFLT_VR_ID;
    UI32_T  proc_id = 0;
    char    id_ar[30] = {0};
    char    ip_ar[20] = {0};
    char    addr_ar[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    OSPF_TYPE_Network_Area_T    network_entry;

    json_object_set_new(result_p, "processes", processes_p);
    while(OSPF_PMGR_GetNextProcessStatus(vr_id, &proc_id) == OSPF_TYPE_RESULT_SUCCESS)
    {
        json_t *process_obj_p = json_object();
        json_t *networks_p = json_array();

        json_object_set_new(process_obj_p, "id", json_integer(proc_id));
        json_object_set_new(process_obj_p, "pid", json_integer(proc_id));
        json_object_set_new(process_obj_p, "networks", networks_p);
        memset(&network_entry, 0, sizeof(OSPF_TYPE_Network_Area_T));
        network_entry.vr_id = vr_id;
        network_entry.proc_id = proc_id;
        while(OSPF_PMGR_GetNextNetworkAreaTable(&network_entry) == OSPF_TYPE_RESULT_SUCCESS)
        {
            if((network_entry.proc_id == proc_id) && (network_entry.vr_id == vr_id))
            {
                if(network_entry.format == NETCFG_TYPE_OSPF_AREA_ID_FORMAT_DECIMAL)
                {
                    json_t *network_obj_p = json_object();

                    memset(id_ar, 0, sizeof(id_ar));
                    memset(ip_ar, 0, sizeof(ip_ar));
                    memset(addr_ar, 0, sizeof(addr_ar));
                    memcpy(addr_ar, &(network_entry.network_addr), sizeof(UI32_T));
                    sprintf(id_ar, "%d.%d.%d.%d_%lu_%lu",addr_ar[0], addr_ar[1], addr_ar[2], addr_ar[3],
                        (unsigned long)network_entry.network_pfx, (unsigned long)network_entry.area_id);
                    sprintf(ip_ar, "%d.%d.%d.%d/%lu",addr_ar[0], addr_ar[1], addr_ar[2], addr_ar[3],
                        (unsigned long)network_entry.network_pfx);
                    json_object_set_new(network_obj_p, "id", json_string(id_ar));
                    json_object_set_new(network_obj_p, "ip", json_string(ip_ar));
                    json_object_set_new(network_obj_p, "area", json_integer(network_entry.area_id));
                    json_array_append_new(networks_p, network_obj_p);
                }
            }
        }
        json_array_append_new(processes_p, process_obj_p);
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to create OSPF process ID.
 *
 * @param pid (required, number) OSPF process ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_OSPF_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *pid_p;
    UI32_T  pid = 0;
    UI32_T  vr_id = SYS_DFLT_VR_ID;
    UI32_T  vrf_id = SYS_DFLT_VRF_ID;
    OSPF_MGR_OSPF_ENTRY_T ospf_entry;

    memset(&ospf_entry, 0, sizeof(OSPF_MGR_OSPF_ENTRY_T));

    pid_p = CGI_REQUEST_GetBodyValue(http_request, "pid");
    if (pid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get pid.");
    }
    pid = json_integer_value(pid_p);

    if(NETCFG_TYPE_OK != NETCFG_PMGR_OSPF_RouterOspfSet(vr_id, vrf_id, pid))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netcfg.ospf.routerOspfSetError", "Failed to create OSPF process ID.");
    }

    json_object_set_new(result_p, "id", pid_p);
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to read OSPF process.
 *
 * @param id (required, number) Unique OSPF ID, same OSPF process ID (pid)
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_OSPF_PID_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t *networks_p = json_array();
    json_t  *pid_p;
    UI32_T  pid = 0;
    UI32_T  vr_id = SYS_DFLT_VR_ID;
    char    id_ar[30] = {0};
    char    ip_ar[20] = {0};
    char    addr_ar[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    OSPF_MGR_OSPF_ENTRY_T       ospf_entry;
    OSPF_TYPE_Network_Area_T    network_entry;

    memset(&ospf_entry, 0, sizeof(OSPF_MGR_OSPF_ENTRY_T));
    memset(&network_entry, 0, sizeof(OSPF_TYPE_Network_Area_T));
    pid_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (pid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get id.");
    }
    pid = json_integer_value(pid_p);

    ospf_entry.vr_id = vr_id;
    ospf_entry.proc_id = pid;
    if (OSPF_PMGR_GetOspfEntry(&ospf_entry) == OSPF_TYPE_RESULT_SUCCESS )
    {
        json_object_set_new(result_p, "id", json_integer(pid));
        json_object_set_new(result_p, "pid", json_integer(pid));
        json_object_set_new(result_p, "networks", networks_p);
        network_entry.vr_id = vr_id;
        network_entry.proc_id = pid;
        while(OSPF_PMGR_GetNextNetworkAreaTable(&network_entry) == OSPF_TYPE_RESULT_SUCCESS)
        {
            if((network_entry.proc_id == pid) && (network_entry.vr_id == vr_id))
            {
                if(network_entry.format == NETCFG_TYPE_OSPF_AREA_ID_FORMAT_DECIMAL)
                {
                    json_t *network_obj_p = json_object();

                    memset(id_ar, 0, sizeof(id_ar));
                    memset(ip_ar, 0, sizeof(ip_ar));
                    memset(addr_ar, 0, sizeof(addr_ar));
                    memcpy(addr_ar, &(network_entry.network_addr), sizeof(UI32_T));
                    sprintf(id_ar, "%d.%d.%d.%d_%lu_%lu",addr_ar[0], addr_ar[1], addr_ar[2], addr_ar[3],
                        (unsigned long)network_entry.network_pfx, (unsigned long)network_entry.area_id);
                    sprintf(ip_ar, "%d.%d.%d.%d/%lu",addr_ar[0], addr_ar[1], addr_ar[2], addr_ar[3],
                        (unsigned long)network_entry.network_pfx);
                    json_object_set_new(network_obj_p, "id", json_string(id_ar));
                    json_object_set_new(network_obj_p, "ip", json_string(ip_ar));
                    json_object_set_new(network_obj_p, "area", json_integer(network_entry.area_id));
                    json_array_append_new(networks_p, network_obj_p);
                }
            }
        }
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "ospf.getOspfEntryError", "No such process ID.");
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete OSPF process.
 *
 * @param id (required, number) Unique OSPF ID, same OSPF process ID (pid)
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_OSPF_PID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *pid_p;
    UI32_T  pid = 0;
    UI32_T  vr_id = SYS_DFLT_VR_ID;
    UI32_T  vrf_id = SYS_DFLT_VRF_ID;
    OSPF_MGR_OSPF_ENTRY_T ospf_entry;

    memset(&ospf_entry, 0, sizeof(OSPF_MGR_OSPF_ENTRY_T));

    pid_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (pid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get id.");
    }
    pid = json_integer_value(pid_p);

    if(NETCFG_TYPE_OK != NETCFG_PMGR_OSPF_RouterOspfUnset(vr_id, vrf_id, pid))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "netcfg.ospf.routerOspfUnsetError", "Failed to delete OSPF process ID.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to get OSPF networks.
 *
 * @param id (required, number) Unique OSPF ID, same OSPF process ID (pid)
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_OSPF_Networks_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t *networks_p = json_array();
    json_t  *pid_p;
    UI32_T  pid = 0;
    UI32_T  vr_id = SYS_DFLT_VR_ID;
    char    id_ar[30] = {0};
    char    ip_ar[20] = {0};
    char    addr_ar[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    OSPF_TYPE_Network_Area_T    network_entry;

    memset(&network_entry, 0, sizeof(OSPF_TYPE_Network_Area_T));
    pid_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (pid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get id.");
    }
    pid = json_integer_value(pid_p);

    network_entry.vr_id = vr_id;
    network_entry.proc_id = pid;
    json_object_set_new(result_p, "networks", networks_p);
    while(OSPF_PMGR_GetNextNetworkAreaTable(&network_entry) == OSPF_TYPE_RESULT_SUCCESS)
    {
        if((network_entry.proc_id == pid) && (network_entry.vr_id == vr_id))
        {
            if(network_entry.format == NETCFG_TYPE_OSPF_AREA_ID_FORMAT_DECIMAL)
            {
                json_t *network_obj_p = json_object();

                memset(id_ar, 0, sizeof(id_ar));
                memset(ip_ar, 0, sizeof(ip_ar));
                memset(addr_ar, 0, sizeof(addr_ar));
                memcpy(addr_ar, &(network_entry.network_addr), sizeof(UI32_T));
                sprintf(id_ar, "%d.%d.%d.%d_%lu_%lu",addr_ar[0], addr_ar[1], addr_ar[2], addr_ar[3],
                    (unsigned long)network_entry.network_pfx, (unsigned long)network_entry.area_id);
                sprintf(ip_ar, "%d.%d.%d.%d/%lu",addr_ar[0], addr_ar[1], addr_ar[2], addr_ar[3],
                    (unsigned long)network_entry.network_pfx);
                json_object_set_new(network_obj_p, "id", json_string(id_ar));
                json_object_set_new(network_obj_p, "ip", json_string(ip_ar));
                json_object_set_new(network_obj_p, "area", json_integer(network_entry.area_id));
                json_array_append_new(networks_p, network_obj_p);
            }
        }
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to create OSPF network.
 *
 * @param id   (required, number) Unique OSPF ID, same OSPF process ID (pid)
 * @param ip   (required, string) IP network (IP address/prefix)
 * @param area (required, number) Area ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_OSPF_Networks_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *pid_p;
    json_t  *ip_p;
    json_t  *area_p;
    UI32_T  pid = 0, area = 0;
    const char    *ip_str_p;
    UI32_T  vr_id = SYS_DFLT_VR_ID;
    UI32_T  vrf_id = SYS_DFLT_VRF_ID;
    UI32_T  format = NETCFG_TYPE_OSPF_AREA_ID_FORMAT_DECIMAL;
    UI32_T  ip_value;
    UI32_T  ret;
    char    id_ar[128] = {0};
    L_INET_AddrIp_T addr_ip;

    memset(&addr_ip,0,sizeof(addr_ip));

    pid_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (pid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get pid.");
    }
    pid = json_integer_value(pid_p);

    ip_p = CGI_REQUEST_GetBodyValue(http_request, "ip");
    if (ip_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ip.");
    }
    ip_str_p = json_string_value(ip_p);

    area_p = CGI_REQUEST_GetBodyValue(http_request, "area");
    if (area_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get area.");
    }
    area = json_integer_value(area_p);

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_PREFIX,
                                                       ip_str_p,
                                                       (L_INET_Addr_T *) &addr_ip,
                                                       sizeof(addr_ip)))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid IP address.");
    }

    IP_LIB_ArraytoUI32(addr_ip.addr, &ip_value);
    if((ret = OSPF_PMGR_NetworkSet(vr_id, pid, ip_value, addr_ip.preflen, area, format))!= OSPF_TYPE_RESULT_SUCCESS)
    {
        if (ret == OSPF_TYPE_RESULT_NETWORK_EXIST)
        {
           return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "ospf.networkSetError", "There is already the same network statement.");
        }
        else
        {
           return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "ospf.networkSetError", "The OSPF network setting is failed.");
        }
    }

    sprintf(id_ar, "%d.%d.%d.%d_%lu_%lu",addr_ip.addr[0], addr_ip.addr[1],
        addr_ip.addr[2], addr_ip.addr[3], (unsigned long)addr_ip.preflen, (unsigned long)area);
    json_object_set_new(result_p, "id", json_string(id_ar));

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete OSPF network ID.
 *
 * @param id        (required, number) Unique OSPF ID, same OSPF process ID (pid)
 * @param networkId (required, string) Unique OSPF Network ID, ip_area
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_OSPF_Networks_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *pid_p;
    json_t  *network_id_p;
    UI32_T  pid = 0;
    const char    *network_id_str_p;
    UI32_T  vr_id = SYS_DFLT_VR_ID;
    UI32_T  vrf_id = SYS_DFLT_VRF_ID;
    UI32_T  format = NETCFG_TYPE_OSPF_AREA_ID_FORMAT_DECIMAL;
    UI32_T  ip = 0, preflen = 0, area = 0;

    pid_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (pid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get id.");
    }
    pid = json_integer_value(pid_p);

    network_id_p = CGI_REQUEST_GetParamsValue(http_request, "networkId");
    if (network_id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get network id.");
    }
    network_id_str_p = json_string_value(network_id_p);

    if (TRUE == CGI_MODULE_OSPF_ParseNetworkId((char *)network_id_str_p, &ip, &preflen, &area))
    {
        if(OSPF_TYPE_RESULT_SUCCESS != OSPF_PMGR_NetworkUnset(vr_id, pid, ip, preflen, area, format))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "ospf.networkUnsetError", "The OSPF delete network setting is failed.");
        }
        return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The networkId format is wrong.");
    }
}

/**----------------------------------------------------------------------
 * This API is used to get network info. for specifed OSPF process ID.
 *
 * @param id        (required, number) Unique OSPF ID, same OSPF process ID (pid)
 * @param networkId (required, string) Unique OSPF Network ID, ip_area
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_OSPF_Networks_ID_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *pid_p;
    json_t  *network_id_p;
    UI32_T  pid = 0;
    const char    *network_id_str_p;
    UI32_T  vr_id = SYS_DFLT_VR_ID;
    UI32_T  ip = 0, preflen = 0, area = 0;
    BOOL_T  found = FALSE;
    char    ip_ar[20] = {0};
    char    addr_ar[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    OSPF_TYPE_Network_Area_T    network_entry;

    memset(&network_entry, 0, sizeof(OSPF_TYPE_Network_Area_T));
    pid_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (pid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get id.");
    }
    pid = json_integer_value(pid_p);

    network_id_p = CGI_REQUEST_GetParamsValue(http_request, "networkId");
    if (network_id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get network id.");
    }
    network_id_str_p = json_string_value(network_id_p);
    if (TRUE == CGI_MODULE_OSPF_ParseNetworkId((char *)network_id_str_p, &ip, &preflen, &area))
    {
        network_entry.vr_id = vr_id;
        network_entry.proc_id = pid;
        while(OSPF_PMGR_GetNextNetworkAreaTable(&network_entry) == OSPF_TYPE_RESULT_SUCCESS)
        {
            if(     (network_entry.proc_id == pid)
               &&   (network_entry.vr_id == vr_id)
               &&   (network_entry.network_addr == ip)
               &&   (network_entry.network_pfx == preflen)
               &&   (network_entry.format == NETCFG_TYPE_OSPF_AREA_ID_FORMAT_DECIMAL)
               &&   (network_entry.area_id == area)
              )
            {
                memset(ip_ar, 0, sizeof(ip_ar));
                memset(addr_ar, 0, sizeof(addr_ar));
                memcpy(addr_ar, &(network_entry.network_addr), sizeof(UI32_T));
                sprintf(ip_ar, "%d.%d.%d.%d/%lu",addr_ar[0], addr_ar[1], addr_ar[2], addr_ar[3],
                    (unsigned long)network_entry.network_pfx);
                json_object_set_new(result_p, "id", network_id_p);
                json_object_set_new(result_p, "ip", json_string(ip_ar));
                json_object_set_new(result_p, "area", json_integer(network_entry.area_id));
                found = TRUE;
            }
        }

        if (found == TRUE)
        {
            return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
        }
        else
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "No such network.");
        }
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The networkId format is wrong.");
    }
}

/**----------------------------------------------------------------------
 * This API is used to set/unset OSPF interface passive .
 *
 * @param id        (required, number) Unique OSPF ID, same OSPF process ID (pid)
 * @param networkId (required, string) Unique OSPF Network ID, ip_area
 * @param isPassive (required, boolean) Passive interface or not
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_OSPF_Interfaces_ID_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *pid_p;
    json_t  *if_id_p;
    json_t  *is_passive_p;
    UI32_T  pid = 0;
    const char    *if_str_p;
    UI32_T  vid = 0;
    UI32_T  ret;
    OSPF_TYPE_Passive_If_T passive_entry;

    memset(&passive_entry, 0, sizeof(OSPF_TYPE_Passive_If_T));
    pid_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (pid_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get id.");
    }
    pid = json_integer_value(pid_p);

    if_id_p = CGI_REQUEST_GetParamsValue(http_request, "ifId");
    if (if_id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ifId.");
    }
    if_str_p = json_string_value(if_id_p);

    /* Parse vlan interface ID.
     */
    if (    (strlen(if_str_p) < 5)
        ||  (memcmp(if_str_p, "vlan", 4) != 0)
       )
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The ifId format is error..");
    }
    vid = atoi(if_str_p + 4);
    if (vid == 0)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "vlan0 is wrong.");
    }

    is_passive_p = CGI_REQUEST_GetBodyValue(http_request, "isPassive");
    if (NULL == is_passive_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get isPassive.");
    }

    if (TRUE != json_is_boolean(is_passive_p))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "isPassive not boolean.");
    }

    passive_entry.vr_id = SYS_DFLT_VR_ID;
    passive_entry.proc_id  = pid ;
    VLAN_VID_CONVERTTO_IFINDEX(vid, passive_entry.ifindex);
    if (TRUE == json_boolean_value(is_passive_p))
    {
        ret = OSPF_PMGR_PassiveIfSet(&passive_entry);
    }
    else
    {
        ret = OSPF_PMGR_PassiveIfUnset(&passive_entry);
    }

    if(ret != OSPF_TYPE_RESULT_SUCCESS)
    {
        if(ret == OSPF_TYPE_RESULT_IF_NOT_EXIST)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Please specify an existing interface.");
        }
        else
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "ospf.passiveIfSetError", "The OSPF passive interface setting is failed.");
        }
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

void CGI_MODULE_OSPF_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler = CGI_MODULE_OSPF_Read;
    handlers.create_handler = CGI_MODULE_OSPF_Create;
    CGI_MAIN_Register("/api/v1/ospf", &handlers, 0);

    {   /* for "/api/v1/ospf/{id}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_OSPF_PID_Read;
        handlers.delete_handler = CGI_MODULE_OSPF_PID_Delete;
        CGI_MAIN_Register("/api/v1/ospf/{id}", &handlers, 0);
    }

    {   /* for "/api/v1/ospf/{id}/networks" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_OSPF_Networks_Read;
        handlers.create_handler = CGI_MODULE_OSPF_Networks_Create;
        CGI_MAIN_Register("/api/v1/ospf/{id}/networks", &handlers, 0);
    }

    {   /* for "/api/v1/ospf/{id}/networks/{networkId}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler = CGI_MODULE_OSPF_Networks_ID_Read;
        handlers.delete_handler = CGI_MODULE_OSPF_Networks_ID_Delete;
        CGI_MAIN_Register("/api/v1/ospf/{id}/networks/{networkId}", &handlers, 0);
    }

    {   /* for "/api/v1/ospf/{id}/interfaces/{ifId}" */
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.update_handler = CGI_MODULE_OSPF_Interfaces_ID_Update;
        CGI_MAIN_Register("/api/v1/ospf/{id}/interfaces/{ifId}", &handlers, 0);
    }
}

static void CGI_MODULE_OSPF_Init()
{
    CGI_MODULE_OSPF_RegisterHandlers();
}

/*
static BOOL_T CGI_MODULE_OSPF_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request)
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

static BOOL_T CGI_MODULE_OSPF_ParseNetworkId(char *str, UI32_T *ip, UI32_T *preflen, UI32_T *area)
{
    char buf_ar[128] = {0};
    char *strtok_state_p = NULL;
    char *ip_str_p = NULL, *preflen_str_p = NULL, *area_str_p = NULL;
    char *end_p = NULL;

    strncpy(buf_ar, str, strlen(str));
    strtok_state_p = buf_ar;

    ip_str_p = strtok_r(NULL, "_", &strtok_state_p);
    if (ip_str_p == NULL)
    {
        return FALSE;
    }
    if (L_INET_Aton(ip_str_p, ip) == FALSE)
    {
        return FALSE;
    }

    preflen_str_p = strtok_r(NULL, "_", &strtok_state_p);
    if (preflen_str_p == NULL)
    {
        return FALSE;
    }
    else
    {
        *preflen  = strtol(preflen_str_p, &end_p, 0);
    }

    area_str_p = strtok_r(NULL, "_", &strtok_state_p);
    if (area_str_p == NULL)
    {
        return FALSE;
    }
    else
    {
        *area  = strtol(area_str_p, &end_p, 0);
    }

    return TRUE;
}
