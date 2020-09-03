#include "cgi_auth.h"
#include "sysfun.h"
#include "sys_mgr.h"
#include "sys_pmgr.h"
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE ) || (SYS_CPNT_THERMAL_DETECT_SHOW_TEMPERATURE == TRUE)
#include "stktplg_board.h"
#endif

#if (SYS_CPNT_SYSMGMT_SHOW_POWER==TRUE) || (SYS_CPNT_SYSMGMT_SHOW_BACKUP_POWER == TRUE)
#include "leaf_es3626a.h"
#endif

#if (SYS_CPNT_SYSMGMT_SHOW_POWER==TRUE) || (SYS_CPNT_SYSMGMT_SHOW_BACKUP_POWER == TRUE)
static void CGI_MODULE_SYSTEM_GetPsu(SYS_MGR_IndivPower_T *indiv_power_p, json_t *psu_obj_p);
#endif

//static BOOL_T CGI_MODULE_SYSTEM_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request);

/**----------------------------------------------------------------------
 * This API is used to get cpu util info.
 *
 * @param NA.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_SYSTEM_CPU_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE)
    json_t *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    UI32_T total_cpu_percent = 0, float_cpu_percent = 0;
    UI32_T cpu_util_max = 0, cpu_util_avg = 0;

    ASSERT(result_p != NULL);

    if (TRUE != SYS_PMGR_GetCpuUsagePercentage(&total_cpu_percent, &float_cpu_percent))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "system.getCpuError", "Failed to get CpuUsagePercentage.");
    }

    if (TRUE != SYS_PMGR_GetCpuUsageAverage(&cpu_util_avg))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "system.getCpuError", "Failed to get CpuUsageAverage.");
    }

    if (TRUE != SYS_PMGR_GetCpuUsageMaximum(&cpu_util_max))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "system.getCpuError", "Failed to get CpuUsageMaximum.");
    }

    if (NULL != result_p)
    {
        json_object_set_new(result_p, "last5SecsUtilization", json_integer(total_cpu_percent));
        json_object_set_new(result_p, "last1MinUtilizationAvg", json_integer(cpu_util_avg));
        json_object_set_new(result_p, "last1MinUtilizationMax", json_integer(cpu_util_max));
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
#else

    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "Unsupport CPU read.");
#endif  /* #if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE) */
}

/**----------------------------------------------------------------------
 * This API is used to get memory info.
 *
 * @param NA.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_SYSTEM_Memory_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
    json_t *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    UI32_T total_bytes = 0, free_bytes = 0, used_bytes = 0;
    UI32_T free_percent = 0, used_percent = 0;

#if (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE)
    SYS_MGR_MemoryUtilBrief_T sys_mem_brief;

    memset(&sys_mem_brief, 0, sizeof(sys_mem_brief));

    if (TRUE != SYS_PMGR_GetMemoryUtilizationBrief(&sys_mem_brief))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "system.getMemoryError", "Failed to get MemoryUtilization.");
    }

    if (TRUE != SYS_PMGR_GetMemoryUtilFreePercentage(&free_percent))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "system.getMemoryError", "Failed to get MemoryUtilFreePercent.");
    }

    total_bytes = (sys_mem_brief.free_bytes + sys_mem_brief.used_bytes);
    free_bytes = sys_mem_brief.free_bytes;
    used_bytes = sys_mem_brief.used_bytes;
    used_percent = (100 - free_percent);

#else
    /* the total_bytes returned by SYSFUN_GetMemoryUsage() means system heap size
     * but the spec of total size here is DRAM size.
     */
    if (SYSFUN_OK != SYSFUN_GetMemoryUsage(&total_bytes, &free_bytes))
    {
        return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "system.getMemoryError", "Failed to get MemoryUsage.");
    }

    total_bytes = (unsigned long)SYS_HWCFG_DRAM_SIZE;
    used_bytes = (total_bytes - free_bytes);
    free_percent = ((free_bytes*100)/total_bytes);
    used_percent = ((used_bytes*100)/total_bytes);
#endif  /* #if (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE) */

    ASSERT(result_p != NULL);

    if (NULL != result_p)
    {
        json_object_set_new(result_p, "totalBytes", json_integer(total_bytes));
        json_object_set_new(result_p, "freeBytes", json_integer(free_bytes));
        json_object_set_new(result_p, "freePercentage", json_integer(free_percent));
        json_object_set_new(result_p, "usedBytes", json_integer(used_bytes));
        json_object_set_new(result_p, "usedPercentage", json_integer(used_percent));
    }

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
}

/**----------------------------------------------------------------------
 * This API is used to get FAN info.
 *
 * @param NA.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_SYSTEM_Fan_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
#if (SYS_CPNT_SYSMGMT_SHOW_FAN_STATUS == TRUE)
    json_t *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *fans_p = json_array();
    SYS_MGR_SwitchFanEntry_T entry;
    STKTPLG_BOARD_BoardInfo_T board_info;
    char name_ar[8] = {0}; //Fan x
    UI32_T board_id = 0;
    UI8_T nbr_of_fan = SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT;

    memset(&entry, 0, sizeof(entry));
    nbr_of_fan = STKTPLG_BOARD_GetFanNumber();
    json_object_set_new(result_p, "fans", fans_p);

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
    for (entry.switch_unit_index = 0; STKTPLG_OM_GetNextUnit(&entry.switch_unit_index); )
    {
        memset(&board_info, 0, sizeof(board_info));
        STKTPLG_OM_GetUnitBoardID(entry.switch_unit_index, &board_id);
        STKTPLG_BOARD_GetBoardInformation(board_id, &board_info);
        nbr_of_fan = board_info.fan_number;

        if(nbr_of_fan == 0)
        {
            continue;
        }

        for (entry.switch_fan_index = 1; entry.switch_fan_index <= nbr_of_fan; entry.switch_fan_index++)
        {
            json_t *fan_obj_p = json_object();

            SYSFUN_Snprintf(name_ar, sizeof(name_ar), "Fan %u", entry.switch_fan_index);
            json_object_set_new(fan_obj_p, "index", json_integer(entry.switch_fan_index));
            json_object_set_new(fan_obj_p, "name", json_string(name_ar));

            if (SYS_PMGR_GetFanStatus(&entry) && (entry.switch_fan_status == VAL_switchFanStatus_ok))
            {
                json_object_set_new(fan_obj_p, "status", json_string("good"));
            }
            else
            {
                json_object_set_new(fan_obj_p, "status", json_string("failed"));
            }

            json_object_set_new(fan_obj_p, "flowType", json_string("f2b"));
            json_object_set_new(fan_obj_p, "rpm", json_integer(entry.switch_fan_oper_speed));

#if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE)
            {
                BOOL_T mode = FALSE;

                if ((SYS_PMGR_GetFanSpeedForceFull(&mode) == TRUE) && (TRUE == mode)
                        && (0 != entry.switch_fan_oper_speed))
                {
                    json_object_set_new(fan_obj_p, "pct", json_integer(100));
                }
                else
                {
                    json_object_set_new(fan_obj_p, "pct", json_integer(0));
                }
            }
#else
            json_object_set_new(fan_obj_p, "pct", json_integer(0));
#endif
            json_object_set_new(fan_obj_p, "model", json_string("unknown"));
            json_object_set_new(fan_obj_p, "serial", json_string("unknown"));
            json_object_set_new(fan_obj_p, "location", json_string("rear"));
            json_array_append_new(fans_p, fan_obj_p);
        } // for (entry.switch_fan_index=1;entry.switch_fan_index<=nbr_of_fan;entry.switch_fan_index++)
    } //for (entry.switch_unit_index=0; STKTPLG_OM_GetNextUnit(&entry.switch_unit_index); )
#endif  /* #if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) */

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
#else
    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "Unsupport FAN read.");
#endif  /* #if (SYS_CPNT_SYSMGMT_SHOW_FAN_STATUS == TRUE) */
}

/**----------------------------------------------------------------------
 * This API is used to get Temperature info.
 *
 * @param NA.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_SYSTEM_Temp_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
#if (SYS_CPNT_THERMAL_DETECT_SHOW_TEMPERATURE == TRUE)
    json_t *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *temps_p = json_array();
    SYS_MGR_SwitchThermalEntry_T entry;
    STKTPLG_BOARD_BoardInfo_T board_info;
    char name_ar[16] = {0}; //Temperature x
    UI32_T board_id = 0;
    UI8_T nbr_of_thermal = SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT;

    memset(&entry, 0, sizeof(entry));
    json_object_set_new(result_p, "temps", temps_p);

#if (SYS_HWCFG_SUPPORT_DIFFERENT_THERMAL_NUM == FALSE)
    nbr_of_thermal = SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT;
#else
    nbr_of_thermal = STKTPLG_BOARD_GetThermalNumber();
#endif

    for (entry.switch_unit_index = 0; STKTPLG_OM_GetNextUnit(&entry.switch_unit_index); )
    {
#if (SYS_HWCFG_SUPPORT_DIFFERENT_THERMAL_NUM == TRUE)
        memset(&board_info, 0, sizeof(board_info));
        STKTPLG_OM_GetUnitBoardID(entry.switch_unit_index, &board_id);
        STKTPLG_BOARD_GetBoardInformation(board_id, &board_info);
        nbr_of_thermal = board_info.thermal_number;

        if(nbr_of_thermal == 0)
        {
            continue;
        }
#endif

        for (entry.switch_thermal_index = 1; entry.switch_thermal_index <= nbr_of_thermal; entry.switch_thermal_index++)
        {
            json_t *temp_obj_p = json_object();

            SYSFUN_Snprintf(name_ar, sizeof(name_ar), "Temperature %u", entry.switch_thermal_index);
            json_object_set_new(temp_obj_p, "index", json_integer(entry.switch_thermal_index));
            json_object_set_new(temp_obj_p, "name", json_string(name_ar));

            if (SYS_PMGR_GetThermalStatus(&entry) == TRUE)
            {
                json_object_set_new(temp_obj_p, "status", json_string("good"));
                json_object_set_new(temp_obj_p, "value", json_integer(entry.switch_thermal_temp_value));
            }
            else
            {
                json_object_set_new(temp_obj_p, "status", json_string("failed"));
                json_object_set_new(temp_obj_p, "value", json_integer(0));
            }

            json_array_append_new(temps_p, temp_obj_p);
        } //for (entry.switch_thermal_index = 1; entry.switch_thermal_index <= nbr_of_thermal; entry.switch_thermal_index++)
    } //for (entry.switch_unit_index = 0; STKTPLG_OM_GetNextUnit(&entry.switch_unit_index); )

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
#else
    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "Unsupport Temperature read.");
#endif  /* #if (SYS_CPNT_THERMAL_DETECT_SHOW_TEMPERATURE == TRUE) */
}

/**----------------------------------------------------------------------
 * This API is used to get power supply info.
 *
 * @param NA.
 * ---------------------------------------------------------------------- */
static CGI_STATUS_CODE_T CGI_MODULE_SYSTEM_Psu_Read(HTTP_Request_T *http_request, HTTP_Response_T *http_response)
{
#if (SYS_CPNT_SYSMGMT_SHOW_POWER == TRUE)
    json_t *result_p = (json_t *) CGI_RESPONSE_GetResult(http_response);
    json_t  *psus_p = json_array();
    SYS_MGR_IndivPower_T indiv_power;
    UI32_T unit = 0;

    memset(&indiv_power, 0, sizeof(indiv_power));
    json_object_set_new(result_p, "psus", psus_p);

    while (STKTPLG_POM_GetNextUnit(&unit))
    {
        memset(&indiv_power, 0, sizeof(SYS_MGR_IndivPower_T));
        indiv_power.sw_indiv_power_unit_index = unit;
        indiv_power.sw_indiv_power_index = 1;

        if (SYS_PMGR_GetSwitchIndivPower(&indiv_power) == TRUE)
        {
            json_t *psu_obj_p = json_object();

            CGI_MODULE_SYSTEM_GetPsu(&indiv_power, psu_obj_p);
            json_array_append_new(psus_p, psu_obj_p);
        } //if (SYS_PMGR_GetSwitchIndivPower(&indiv_power) == TRUE)

#if (SYS_CPNT_SYSMGMT_SHOW_BACKUP_POWER == TRUE)
        indiv_power.sw_indiv_power_unit_index = unit;
        indiv_power.sw_indiv_power_index = 2;

        if(SYS_PMGR_GetSwitchIndivPower(&indiv_power) == TRUE)
        {
            json_t *psu_obj_p = json_object();

            CGI_MODULE_SYSTEM_GetPsu(&indiv_power, psu_obj_p);
            json_array_append_new(psus_p, psu_obj_p);
        }
#endif  /* #if (SYS_CPNT_SYSMGMT_SHOW_BACKUP_POWER == TRUE) */
    } //while (STKTPLG_POM_GetNextUnit(&unit))

    return CGI_RESPONSE_SUCCESS(CGI_SUCCESS);
#else
    return CGI_RESPONSE_ERROR(CGI_INTERNAL_SERVER_ERROR, "internalServerError", "Unsupport PSU read.");
#endif  /* #if (SYS_CPNT_SYSMGMT_SHOW_POWER == TRUE) */
}

void CGI_MODULE_SYSTEM_RegisterHandlers()
{
    CGI_API_HANDLER_SET_T handlers = {0};

    handlers.read_handler   = CGI_MODULE_SYSTEM_CPU_Read;
    CGI_MAIN_Register("/api/v1/system/cpu", &handlers, 0);

    handlers.read_handler   = CGI_MODULE_SYSTEM_Memory_Read;
    CGI_MAIN_Register("/api/v1/system/memory", &handlers, 0);

    handlers.read_handler   = CGI_MODULE_SYSTEM_Fan_Read;
    CGI_MAIN_Register("/api/v1/system/fan", &handlers, 0);

    handlers.read_handler   = CGI_MODULE_SYSTEM_Temp_Read;
    CGI_MAIN_Register("/api/v1/system/temperature", &handlers, 0);

    handlers.read_handler   = CGI_MODULE_SYSTEM_Psu_Read;
    CGI_MAIN_Register("/api/v1/system/psu", &handlers, 0);
}

static void CGI_MODULE_SYSTEM_Init()
{
    CGI_MODULE_SYSTEM_RegisterHandlers();
}

#if (SYS_CPNT_SYSMGMT_SHOW_POWER==TRUE) || (SYS_CPNT_SYSMGMT_SHOW_BACKUP_POWER == TRUE)
static void CGI_MODULE_SYSTEM_GetPsu(SYS_MGR_IndivPower_T *indiv_power_p, json_t *psu_obj_p)
{
    char name_ar[8] = {0}; //PSU-x

    SYSFUN_Snprintf(name_ar, sizeof(name_ar), "PSU-%u", indiv_power_p->sw_indiv_power_index);
    json_object_set_new(psu_obj_p, "index", json_integer(indiv_power_p->sw_indiv_power_index));
    json_object_set_new(psu_obj_p, "name", json_string(name_ar));

    switch(indiv_power_p->sw_indiv_power_status)
    {
        case VAL_swIndivPowerStatus_green:
            json_object_set_new(psu_obj_p, "status", json_string("good"));
            break;

        case VAL_swIndivPowerStatus_red:
            json_object_set_new(psu_obj_p, "status", json_string("failed"));
            break;

        case VAL_swIndivPowerStatus_notPresent:
            json_object_set_new(psu_obj_p, "status", json_string("Not present"));
            break;

        default:
            json_object_set_new(psu_obj_p, "status", json_string("unknown"));
            break;
    } //switch(indiv_power.sw_indiv_power_status)

    if (VAL_swIndivPowerType_DC_N48 == indiv_power_p->sw_indiv_power_type)
    {
        json_object_set_new(psu_obj_p, "powerType", json_string("dc"));
    }
    else if (VAL_swIndivPowerType_DC_P24 == indiv_power_p->sw_indiv_power_type)
    {
        json_object_set_new(psu_obj_p, "powerType", json_string("dc"));
    }
    else if (VAL_swIndivPowerType_AC == indiv_power_p->sw_indiv_power_type)
    {
        json_object_set_new(psu_obj_p, "powerType", json_string("ac"));
    }
    else //VAL_swIndivPowerType_DC_N48_Wrong,VAL_swIndivPowerType_DC_P24_Wrong,VAL_swIndivPowerType_none,VAL_swIndivPowerType_AC_Wrong
    {
        json_object_set_new(psu_obj_p, "powerType", json_string("unknown"));
    }

    json_object_set_new(psu_obj_p, "model", json_string("unknown"));
    json_object_set_new(psu_obj_p, "vin", json_integer(0));
    json_object_set_new(psu_obj_p, "vout", json_integer(0));
    json_object_set_new(psu_obj_p, "iin", json_integer(0));
    json_object_set_new(psu_obj_p, "iout", json_integer(0));
    json_object_set_new(psu_obj_p, "pin", json_integer(0));
    json_object_set_new(psu_obj_p, "pout", json_integer(0));
    json_object_set_new(psu_obj_p, "serial", json_string("unknown"));
}
#endif

/*
static BOOL_T CGI_MODULE_SYSTEM_CheckAuth(const char *username, const char *password, HTTP_Request_T *http_request)
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
