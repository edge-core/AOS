#include "cgi_auth.h"
#include "stktplg_pmgr.h"
#include "stktplg_pom.h"

//static BOOL_T CGI_MODULE_DEVICE_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request);

/**----------------------------------------------------------------------
 * This API is used to get device info.
 *
 * @param NA.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_DEVICE_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    BOOL_T  ret_info = TRUE;
    UI32_T  unit = 0;
    char    manufacturer_ar[MAXSIZE_sysDescr+1] = {"Accton"};
    unsigned char cpu_mac_ar[SYS_TYPE_MAC_ADDR_LEN] = {0};
    char    chassis_id_ar[18] = {0};
    STKTPLG_MGR_Switch_Info_T sw_info = {0};

    ret_info = ret_info && STKTPLG_POM_GetNextUnit(&unit);

    /* CPU mac-address.
     */
    ret_info = ret_info && STKTPLG_POM_GetUnitBaseMac(unit, cpu_mac_ar);

    /* switch info.
     */
    sw_info.sw_unit_index = unit;
    ret_info = ret_info && STKTPLG_PMGR_GetSwitchInfo(&sw_info);

    if (TRUE == ret_info)
    {
        json_t *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);

        ASSERT(result_p != NULL);
        if (result_p != NULL)
        {
            sprintf(chassis_id_ar, "%02X-%02X-%02X-%02X-%02X-%02X",
                cpu_mac_ar[0], cpu_mac_ar[1], cpu_mac_ar[2], cpu_mac_ar[3], cpu_mac_ar[4], cpu_mac_ar[5]);

            json_object_set_new(result_p, "type", json_string("SWITCH"));
            json_object_set_new(result_p, "manufacturer", json_string(manufacturer_ar));
            json_object_set_new(result_p, "hwVersion", json_string(sw_info.sw_hardware_ver));
            json_object_set_new(result_p, "swVersion", json_string(sw_info.sw_opcode_ver));
            json_object_set_new(result_p, "serialNumber", json_string(sw_info.sw_serial_number));
            json_object_set_new(result_p, "chassisId", json_string(chassis_id_ar));
        }

        return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
    }
    else
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError",
            "An unexpected problem occurred, please try again. If the problem continues, contact us about your condition in details.");
    }
}

void CGI_MODULE_DEVICE_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler   = CGI_MODULE_DEVICE_Read;

    CGI_MAIN_Register("/api/v1/device", &handlers, 0);
}

static void CGI_MODULE_DEVICE_Init()
{
    CGI_MODULE_DEVICE_RegisterHandlers();
}

/*
static BOOL_T CGI_MODULE_DEVICE_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request)
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
