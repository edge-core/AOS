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

#define CGI_MODULE_ROUTER_ASN_MIN 64512
#define CGI_MODULE_ROUTER_ASN_MAX 65535
#define CGI_MODULE_ROUTER_BGP_DETAIL_SIZE 20000


static void CGI_MODULE_ROUTER_ConstructAsnNetworkCmd(
        UI32_T asn, char *network_str_p, char *type_str_p, BOOL_T add, char *cmd_p);
static void CGI_MODULE_ROUTER_ConstructAsnNeighborCmd(UI32_T asn, char *intf_str_p, BOOL_T add, char *cmd_p);
static BOOL_T CGI_MODULE_ROUTER_GetRoutesNetwork(char *network_p);

/**----------------------------------------------------------------------
 * This API is used to create ASN.
 *
 * @param asn (required, number) Autonomous system number <64512-65535>
 * @param deviceType (required, string) Spine, leaf or core
 * @param network (required, string) Loopback interface IP
 * @param linkInterfaces (required, array) Link interfaces. ex, VLAN1, Ethernet1
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ROUTER_ASN_Create(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *asn_p;
    json_t  *type_p;
    json_t  *network_p;
    json_t  *intfs_p;
    json_t  *intf_p;
    const char*  type_str_p;
    const char*  network_str_p;
    const char*  intf_str_p;
    L_INET_AddrIp_T inet_network;
    char cmd_ar[CGI_UTIL_LINUX_CMD_SIZE] = {0};
    UI32_T idx = 0, intf_num = 0, asn = 0;

    memset(&inet_network, 0, sizeof(inet_network));
    asn_p = CGI_REQUEST_GetBodyValue(http_request, "asn");
    type_p = CGI_REQUEST_GetBodyValue(http_request, "deviceType");
    network_p = CGI_REQUEST_GetBodyValue(http_request, "network");
    intfs_p = CGI_REQUEST_GetBodyValue(http_request, "linkInterfaces");

    if (asn_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ASN.");
    }

    if (type_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get deviceType.");
    }

    if (network_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get network.");
    }

    if (intfs_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get linkInterfaces.");
    }

    asn = json_integer_value(asn_p);
    type_str_p = json_string_value(type_p);
    network_str_p = json_string_value(network_p);

    if ((CGI_MODULE_ROUTER_ASN_MIN > asn) || (CGI_MODULE_ROUTER_ASN_MAX < asn))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "ASN is out of range.");
    }

    if ((0 != strcmp(type_str_p, "core")) && (0 != strcmp(type_str_p, "leaf")) && (0 != strcmp(type_str_p, "spine")))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid deviceType.");
    }

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_PREFIX,
                                                       network_str_p,
                                                       (L_INET_Addr_T *) &inet_network,
                                                       sizeof(inet_network)))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid network IP format.");
    }

    CGI_MODULE_ROUTER_ConstructAsnNetworkCmd(asn, network_str_p, type_str_p, TRUE, cmd_ar);

    if (TRUE != CGI_UTIL_ExecLinuxSetCommand(cmd_ar))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "redistribute", "Failed to set ASN.");
    }

    intf_num = json_array_size(intfs_p);

    for (idx = 0; idx < intf_num; idx ++)
    {
        intf_p = json_array_get(intfs_p, idx);

        if (NULL == intf_p)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get a interface.");
        }

        intf_str_p = json_string_value(intf_p);

        //need to check intf string format "VLANx" ???
        memset(cmd_ar, 0, CGI_UTIL_LINUX_CMD_SIZE);
        CGI_MODULE_ROUTER_ConstructAsnNeighborCmd(asn, intf_str_p, TRUE, cmd_ar);

        if (TRUE != CGI_UTIL_ExecLinuxSetCommand(cmd_ar))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "neighbor", "Failed to set neighbor interface.");
        }
    } //for intfs_p

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to delete ASN.
 *
 * @param id (required, number) ASN ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ROUTER_ASN_ID_Delete(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    char cmd_ar[CGI_UTIL_LINUX_CMD_SIZE] = {0};
    UI32_T asn = 0;

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");

    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get route ID.");
    }

    asn = json_integer_value(id_p);

    sprintf(cmd_ar, "vtysh -c \"config terminal\" -c \"no router bgp %d\"", asn);

    if (TRUE != CGI_UTIL_ExecLinuxSetCommand(cmd_ar))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "router bgp", "Failed to delete router bgp.");
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to update ASN features.
 *
 * @param id (required, number) ASN ID
 * @param deviceType (required, string) Spine, leaf or core
 * @param network (optional, string) Loopback interface IP
 * @param status (optional, boolean) Add or remove interfaces
 * @param linkInterfaces (optional, array) Link interfaces. ex, VLAN1, Ethernet1
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_ROUTER_ASN_ID_Update(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    json_t  *type_p;
    json_t  *network_p;
    json_t  *intfs_p;
    json_t  *intf_p;
    json_t  *status_p;
    const char*  type_str_p = NULL;
    const char*  network_str_p = NULL;
    const char*  intf_str_p;
    L_INET_AddrIp_T inet_network;
    char cmd_ar[CGI_UTIL_LINUX_CMD_SIZE] = {0};
    UI32_T idx = 0, intf_num = 0, asn = 0;

    memset(&inet_network, 0, sizeof(inet_network));
    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    type_p = CGI_REQUEST_GetBodyValue(http_request, "deviceType");
    network_p = CGI_REQUEST_GetBodyValue(http_request, "network");
    intfs_p = CGI_REQUEST_GetBodyValue(http_request, "linkInterfaces");
    status_p = CGI_REQUEST_GetBodyValue(http_request, "status");

    if (id_p == NULL)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get ASN.");
    }

    asn = json_integer_value(id_p);

    if ((CGI_MODULE_ROUTER_ASN_MIN > asn) || (CGI_MODULE_ROUTER_ASN_MAX < asn))
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "ASN is out of range.");
    }

    if (NULL != network_p)
    {
        char network_ar[24] = {0};

        network_str_p = json_string_value(network_p);

        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_PREFIX,
                                                           network_str_p,
                                                           (L_INET_Addr_T *) &inet_network,
                                                           sizeof(inet_network)))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid network IP format.");
        }

        //remove preivous network
        if (TRUE == CGI_MODULE_ROUTER_GetRoutesNetwork(network_ar))
        {
            CGI_MODULE_ROUTER_ConstructAsnNetworkCmd(asn, network_ar, NULL, FALSE, cmd_ar);

            if (TRUE != CGI_UTIL_ExecLinuxSetCommand(cmd_ar))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "network", "Failed to remove old network.");
            }
        }

        memset(cmd_ar, 0, CGI_UTIL_LINUX_CMD_SIZE);

        if (NULL != type_p)
        {
            type_str_p = json_string_value(type_p);
            CGI_MODULE_ROUTER_ConstructAsnNetworkCmd(asn, network_str_p, type_str_p, TRUE, cmd_ar);
        }
        else
        {
            CGI_MODULE_ROUTER_ConstructAsnNetworkCmd(asn, network_str_p, NULL, TRUE, cmd_ar);
        }

        if (TRUE != CGI_UTIL_ExecLinuxSetCommand(cmd_ar))
        {
            return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "network", "Failed to set network.");
        }
    } //if (NULL != network_p)

    if (NULL != intfs_p)
    {
        if (NULL == status_p)
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get status.");
        }

        if (TRUE != json_is_boolean(status_p))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "The status is not boolean type.");
        }

        intf_num = json_array_size(intfs_p);

        for (idx = 0; idx < intf_num; idx ++)
        {
            intf_p = json_array_get(intfs_p, idx);

            if (NULL == intf_p)
            {
                return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Failed to get a interface.");
            }

            intf_str_p = json_string_value(intf_p);

            //need to check intf string format "VLANx" ???

            memset(cmd_ar, 0, CGI_UTIL_LINUX_CMD_SIZE);

            if (TRUE == json_boolean_value(status_p))
            {
                CGI_MODULE_ROUTER_ConstructAsnNeighborCmd(asn, intf_str_p, TRUE, cmd_ar);
            }
            else
            {
                CGI_MODULE_ROUTER_ConstructAsnNeighborCmd(asn, intf_str_p, FALSE, cmd_ar);
            }

            if (TRUE != CGI_UTIL_ExecLinuxSetCommand(cmd_ar))
            {
                return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "neighbor", "Failed to set neighbor interface.");
            }
        } //for intfs_p
    } //if (NULL != intfs_p)

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

void CGI_MODULE_ROUTER_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.create_handler = CGI_MODULE_ROUTER_ASN_Create;
    CGI_MAIN_Register("/api/v1/router/bgp/asn", &handlers, 0);

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.update_handler = CGI_MODULE_ROUTER_ASN_ID_Update;
        handlers.delete_handler = CGI_MODULE_ROUTER_ASN_ID_Delete;
        CGI_MAIN_Register("/api/v1/router/bgp/asn/{id}", &handlers, 0);
    }
}

static void CGI_MODULE_ROUTER_Init()
{
    CGI_MODULE_ROUTER_RegisterHandlers();
}

static void CGI_MODULE_ROUTER_ConstructAsnNetworkCmd(
        UI32_T asn, char *network_str_p, char *type_str_p, BOOL_T add, char *cmd_p)
{
    if (TRUE != add)
    {
        sprintf(cmd_p, "vtysh -c \"config terminal\" -c \"router bgp %d\" -c \"address-family ipv4 unicast\" -c \"no network %s\" -c \"exit-address-family\"", asn, network_str_p);
        return;
    }

    //add
    if ((NULL != type_str_p) && (0 == strcmp(type_str_p, "leaf")))
    {
        sprintf(cmd_p, "vtysh -c \"config terminal\" -c \"router bgp %d\" -c \"address-family ipv4 unicast\" -c \"network %s\" -c \"redistribute connected\" -c \"exit-address-family\"", asn, network_str_p);
        return;
    }

    sprintf(cmd_p, "vtysh -c \"config terminal\" -c \"router bgp %d\" -c \"address-family ipv4 unicast\" -c \"network %s\" -c \"exit-address-family\"", asn, network_str_p);
} //CGI_MODULE_ROUTER_ConstructAsnNetworkCommand

static void CGI_MODULE_ROUTER_ConstructAsnNeighborCmd(UI32_T asn, char *intf_str_p, BOOL_T add, char *cmd_p)
{
    if (TRUE == add)
    {
        sprintf(cmd_p, "vtysh -c \"config terminal\" -c \"router bgp %d\" -c \"neighbor %s interface remote-as external\" -c \"neighbor %s capability extended-nexthop\"", asn, intf_str_p, intf_str_p);
    }
    else
    {
        sprintf(cmd_p, "vtysh -c \"config terminal\" -c \"router bgp %d\" -c \"no neighbor %s capability extended-nexthop\" -c \"no neighbor %s interface remote-as external\"", asn, intf_str_p, intf_str_p);
    }
} //CGI_MODULE_ROUTER_ConstructAsnNeighborCmd

static BOOL_T CGI_MODULE_ROUTER_GetRoutesNetwork(char *network_p)
{
    FILE* fp_p;
    json_t *json_file;
    void *iter_p;
    char buff_ar[CGI_UTIL_LINUX_RETURN_BUFFER_SIZE] = {0};
    char json_ar[CGI_MODULE_ROUTER_BGP_DETAIL_SIZE] = {0};
    json_error_t error;
    int size = 0;
    BOOL_T find = FALSE;

    fp_p = popen("vtysh -c \"show bgp detail json\"", "r");

    if (fp_p == NULL)
    {
        printf("ERROR, can execute command %s\r\n", "vtysh -c \"show bgp detail json\"");
        return FALSE;
    }

    while (fgets(buff_ar, CGI_UTIL_LINUX_RETURN_BUFFER_SIZE, fp_p) != NULL)
    {
        size += strlen(buff_ar);

        if (CGI_MODULE_ROUTER_BGP_DETAIL_SIZE < size)
        {
            break;
        }

        strcat(json_ar, buff_ar);
    }

    json_file = json_loads(json_ar, 0, &error);

    if (NULL != json_file)
    {
        json_t *routes_p = json_object_get(json_file, "routes");

        if (NULL != routes_p)
        {
            iter_p = json_object_iter(routes_p);

            while(iter_p)
            {
                const char *key_p = json_object_iter_key(iter_p);

                if (NULL != strstr(key_p, "/32"))
                {
                    sprintf(network_p, "%s", (char *) key_p);
                    find = TRUE;
                    break;
                }

                iter_p = json_object_iter_next(routes_p, iter_p);
            }
        }
        json_decref(json_file);
    }

    pclose(fp_p);
    return find;
} //CGI_UTIL_ExecLinuxSetCommand
