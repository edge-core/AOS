#include "cgi_auth.h"
#include "stktplg_pom.h"
#include "swdrv_type.h"
#include "swctrl.h"
#include "swctrl_pom.h"
#include "nmtr_mgr.h"
#include "nmtr_pmgr.h"
#include "sys_adpt.h"

#define CGI_MODULE_STATISTICS_ID_LEN   20

static BOOL_T CGI_MODULE_STATISTICS_GetOne(UI32_T unit, UI32_T port, json_t *interface_obj_p);
//static BOOL_T CGI_MODULE_STATISTICS_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request);

/**----------------------------------------------------------------------
 * This API is used to get statistics.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_STATISTICS_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    UI32_T  pmax = 0, unit = 0, pidx = 0;
    json_t  *interfaces_p;

    STKTPLG_POM_GetNextUnit(&unit);
    pmax = SWCTRL_POM_UIGetUnitPortNumber(unit);
    if (pmax == 0)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "No port exists.");
    }

    /* Get all interfaces.
     */
    interfaces_p = json_array();
    json_object_set_new(result_p, "interfaces", interfaces_p);
    for (pidx =1; pidx <= pmax; pidx++)
    {
        json_t *interface_obj_p = json_object();
        if (TRUE == CGI_MODULE_STATISTICS_GetOne(unit, pidx, interface_obj_p))
        {
            json_array_append_new(interfaces_p, interface_obj_p);
        }
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to get an interface info by ID.
 *
 * @param id (required, string) Interface ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_STATISTICS_ID_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    const char    *id_str_p;
    UI32_T  unit = 0, port = 0;
    BOOL_T  ret = FALSE;
    char    buffer_ar[20] = {0};

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (NULL == id_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Interface ID is NULL.");
    }
    id_str_p = json_string_value(id_p);

    CGI_UTIL_UrlDecode(buffer_ar, id_str_p);
    if (TRUE == CGI_UTIL_InterfaceIdToEth((const char *) buffer_ar, &unit, &port))
    {
        ret = CGI_MODULE_STATISTICS_GetOne(unit, port, result_p);
    }
    else if (memcmp(buffer_ar, "vlan", 4) == 0)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Not support VLAN ID.");
    }
    else if (memcmp(buffer_ar, "trunk", 5) == 0)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Not support Trunk ID.");
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Error ID.");
    }

    if (FALSE == ret)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "Failled to get interface entry.");
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to clear statistics for all interfaces.
 *
 * @param NA
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_STATISTICS_Clear(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    UI32_T  pmax = 0, unit = 0, pidx = 0, lport = 0;

    STKTPLG_POM_GetNextUnit(&unit);
    pmax = SWCTRL_POM_UIGetUnitPortNumber(unit);
    if (pmax == 0)
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "No port exists.");
    }

    /* clear all interfaces.
     */
    for (pidx =1; pidx <= pmax; pidx++)
    {
        if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UserPortToIfindex(unit, pidx, &lport))
        {
            continue;
        }

#if(SYS_CPNT_SYSTEMWIDE_COUNTER == TRUE)
        NMTR_PMGR_ClearSystemwideStats(lport);
#else
        /* Do noting.
         */
#endif
    }
    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}


/**----------------------------------------------------------------------
 * This API is used to clear interface statistics.
 *
 * @param id (required, string) Interface ID
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_STATISTICS_Clear_ID(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t  *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *id_p;
    const char    *id_str_p;
    UI32_T  unit = 0, port = 0, lport = 0;
    char    buffer_ar[20] = {0};

    id_p = CGI_REQUEST_GetParamsValue(http_request, "id");
    if (NULL == id_p)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Interface ID is NULL.");
    }
    id_str_p = json_string_value(id_p);

    CGI_UTIL_UrlDecode(buffer_ar, id_str_p);
    if (TRUE == CGI_UTIL_InterfaceIdToEth((const char *) buffer_ar, &unit, &port))
    {
        if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UserPortToIfindex(unit, port, &lport))
        {
            return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Invalid port.");
        }

#if(SYS_CPNT_SYSTEMWIDE_COUNTER == TRUE)
        NMTR_PMGR_ClearSystemwideStats(lport);
#else
        /* Do noting.
         */
#endif
        return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
    }
    else if (memcmp(buffer_ar, "vlan", 4) == 0)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Not support VLAN ID.");
    }
    else if (memcmp(buffer_ar, "trunk", 5) == 0)
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Not support Trunk ID.");
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_BAD_REQUEST, "bad request", "Error ID.");
    }
}

void CGI_MODULE_STATISTICS_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler   = CGI_MODULE_STATISTICS_Read;
    CGI_MAIN_Register("/api/v1/statistics", &handlers, 0);

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.read_handler   = CGI_MODULE_STATISTICS_ID_Read;
        CGI_MAIN_Register("/api/v1/statistics/{id}", &handlers, 0);
    }
}

void CGI_MODULE_STATISTICS_Clear_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.update_handler   = CGI_MODULE_STATISTICS_Clear;
    CGI_MAIN_Register("/api/v1/statistics:clear", &handlers, 0);

    {
        CGI_API_HANDLER_SET_T handlers = {0};
        handlers.update_handler   = CGI_MODULE_STATISTICS_Clear_ID;
        CGI_MAIN_Register("/api/v1/statistics:clear/{id}", &handlers, 0);
    }
}

static void CGI_MODULE_STATISTICS_Init()
{
    CGI_MODULE_STATISTICS_RegisterHandlers();
    CGI_MODULE_STATISTICS_Clear_RegisterHandlers();
}

/*
static BOOL_T CGI_MODULE_STATISTICS_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request)
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

static BOOL_T CGI_MODULE_STATISTICS_GetOne(UI32_T unit, UI32_T port, json_t *interface_obj_p)
{
    UI32_T lport = 0, speed = 0;
    UI8_T  id_ar[CGI_MODULE_STATISTICS_ID_LEN] = {0};
    Port_Info_T             port_info;
    SWDRV_IfTableStats_T    if_table_stats;
    SWDRV_IfXTableStats_T   if_xtable_stats;
    NMTR_MGR_Utilization_T  if_util;

    if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UserPortToIfindex(unit, port, &lport))
    {
        return FALSE;
    }

    memset(&port_info, 0, sizeof(Port_Info_T));
    memset(&if_table_stats, 0, sizeof(SWDRV_IfTableStats_T));
    memset(&if_xtable_stats, 0, sizeof(SWDRV_IfXTableStats_T));
    memset(&if_util, 0, sizeof(NMTR_MGR_Utilization_T));
    sprintf(id_ar, "eth%lu/%lu", (unsigned long)unit, (unsigned long)port);
    json_object_set_new(interface_obj_p, "id", json_string(id_ar));

#if(SYS_CPNT_SYSTEMWIDE_COUNTER == TRUE)
    if(TRUE != NMTR_PMGR_GetSystemwideIfTableStats(lport, &if_table_stats))
#else
    if(TRUE != NMTR_PMGR_GetIfTableStats(lport, &if_table_stats))
#endif
    {
        return FALSE;
    }

    json_object_set_new(interface_obj_p, "dateTime", json_string(if_table_stats.ifDateTime));
    json_object_set_new(interface_obj_p, "inOctets", json_integer(if_table_stats.ifInOctets));
    json_object_set_new(interface_obj_p, "inUcastPkts", json_integer(if_table_stats.ifInUcastPkts));
    json_object_set_new(interface_obj_p, "inDiscards", json_integer(if_table_stats.ifInDiscards));
    json_object_set_new(interface_obj_p, "inErrors", json_integer(if_table_stats.ifInErrors));
    json_object_set_new(interface_obj_p, "inUnknownProtos", json_integer(if_table_stats.ifInUnknownProtos));
    json_object_set_new(interface_obj_p, "outOctets", json_integer(if_table_stats.ifOutOctets));
    json_object_set_new(interface_obj_p, "outUcastPkts", json_integer(if_table_stats.ifOutUcastPkts));
    json_object_set_new(interface_obj_p, "outDiscards", json_integer(if_table_stats.ifOutDiscards));
    json_object_set_new(interface_obj_p, "outErrors", json_integer(if_table_stats.ifOutErrors));

#if(SYS_CPNT_SYSTEMWIDE_COUNTER == TRUE)
    if(TRUE == NMTR_PMGR_GetSystemwideIfXTableStats(lport, &if_xtable_stats))
#else
    if(TRUE == NMTR_PMGR_GetIfXTableStats(lport, &if_xtable_stats))
#endif
    {
        json_object_set_new(interface_obj_p, "inMulticastPkts", json_integer(if_xtable_stats.ifHCInMulticastPkts));
        json_object_set_new(interface_obj_p, "inBroadcastPkts", json_integer(if_xtable_stats.ifHCInBroadcastPkts));
        json_object_set_new(interface_obj_p, "outMulticastPkts", json_integer(if_xtable_stats.ifHCOutMulticastPkts));
        json_object_set_new(interface_obj_p, "outBroadcastPkts", json_integer(if_xtable_stats.ifHCOutBroadcastPkts));
    }
    else
    {
        json_object_set_new(interface_obj_p, "inMulticastPkts", json_integer(0));
        json_object_set_new(interface_obj_p, "inBroadcastPkts", json_integer(0));
        json_object_set_new(interface_obj_p, "outMulticastPkts", json_integer(0));
        json_object_set_new(interface_obj_p, "outBroadcastPkts", json_integer(0));
    }

    if(TRUE == NMTR_PMGR_GetPortUtilization(lport, &if_util))
    {
        json_object_set_new(interface_obj_p, "inUtilization", json_integer(if_util.ifInOctets_utilization));
        json_object_set_new(interface_obj_p, "outUtilization", json_integer(if_util.ifOutOctets_utilization));
    }
    else
    {
        json_object_set_new(interface_obj_p, "inUtilization", json_integer(0));
        json_object_set_new(interface_obj_p, "outUtilization", json_integer(0));
    }

    return TRUE;
}