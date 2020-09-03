/* MODULE NAME: CLI_API_POE.C
* PURPOSE:
*   PoE function for CLI.
* NOTES:
*
* HISTORY:
*    mm/dd/yy (A.D.)
*
* Copyright(C)      Accton Corporation, 2013
*/

/* INCLUDE FILE DECLARATIONS
*/
#include "sys_cpnt.h"
#include "sys_hwcfg.h"
#include "cli_api.h"
#if (SYS_CPNT_POE == TRUE)
#include "poe_pmgr.h"
#include "poe_pom.h"
#include "leaf_3621.h"
#include "stktplg_board.h"
#endif
#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
#include "time_range_type.h"
#include "time_range_pmgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
*/
#if 0
#define DBG_PRINT(format,...) printf("%s(%d): "format"\r\n",__FUNCTION__,__LINE__,##__VA_ARGS__); fflush(stdout);
#else
#define DBG_PRINT(format,...)
#endif

/* DATA TYPE DECLARATIONS
*/

/* LOCAL SUBPROGRAM DECLARATIONS
*/

/* STATIC VARIABLE DEFINITIONS
*/

/* EXPORTED SUBPROGRAM BODIES
*/
UI32_T CLI_API_POE_Show_Power_Inline_Status(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_POE==TRUE)
    POE_OM_PsePort_T port_entry;
    UI32_T unit  = 0;
    UI32_T port  = 0;
    UI32_T verify_unit = 0;
    UI32_T ifindex = 0;
    UI32_T verify_port = 0;
#if 0
    UI32_T portadmin = VAL_pethPsePortAdminEnable_false;
    UI32_T detectionstatus = VAL_pethPsePortDetectionStatus_disabled;
    UI32_T powerpriority = VAL_pethPsePortPowerPriority_low;
    UI32_T milliwatts = 0;
    UI32_T consumption =0;
#ifdef SYS_CPNT_POE_PSE_DOT3AT
    UI32_T highpower;
#endif
#endif
    UI32_T power_inline_compatible_status;
    UI32_T line_num = 0;
    char   buff_temp[CLI_DEF_MAX_BUFSIZE] = {0};
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    BOOL_T is_show_header = TRUE;
    CLI_API_EthStatus_T ret;

    if(arg[0] != NULL)
    {
        unit = atoi(arg[0]);
        port = atoi(strchr(arg[0], '/') + 1);

        if (STKTPLG_OM_IsPoeDevice(unit) == FALSE)
        {
            CLI_LIB_PrintStr_1("Unit %lu does not support PoE.\r\n", unit);
            return CLI_NO_ERROR;
        }

        ret = verify_ethernet(unit, port, &ifindex);
        if ((ret==CLI_API_ETH_NOT_PRESENT) || (ret==CLI_API_ETH_UNKNOWN_PORT))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_2("Ethernet %lu/%lu is not present.\r\n", unit, port);
#endif
            return CLI_NO_ERROR;
        }

#if 1
        if(POE_POM_GetPsePortEntry(unit, port, &port_entry))
        {
#else
        if(POE_POM_GetPsePortAdmin(unit, port, &portadmin))
        {
            POE_POM_GetPsePortPowerPriority(unit, port, &powerpriority);
            POE_POM_GetPsePortDetectionStatus(unit, port, &detectionstatus);
            POE_POM_GetPortPowerMaximumAllocation(unit, port, &milliwatts);
            POE_POM_GetPortPowerConsumption(unit, port, &consumption);

            POE_POM_GetPortManualHighPowerMode(unit, port, &highpower);
#endif

#if ((defined SYS_CPNT_POE_PSE_DOT3AT) && (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM))
#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
            PROCESS_MORE("                   Time          Max      Used              High    \r\n");
            PROCESS_MORE("Interface Admin    Range    Oper Power    Power    Priority Power   \r\n");
            PROCESS_MORE("--------- -------- -------- ---- -------- -------- -------- --------\r\n");
#else
            PROCESS_MORE("                        Max      Used              High    \r\n");
            PROCESS_MORE("Interface Admin    Oper Power    Power    Priority Power   \r\n");
            PROCESS_MORE("--------- -------- ---- -------- -------- -------- --------\r\n");
#endif
#else
#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
            PROCESS_MORE("                   Time          Max      Used             \r\n");
            PROCESS_MORE("Interface Admin    Range    Oper Power    Power    Priority\r\n");
            PROCESS_MORE("--------- -------- -------- ---- -------- -------- --------\r\n");
#else
#if (SYS_CPNT_POE_POE_POWER_INLINE_STATUS_SHOW_OPER_ONLY == TRUE)
            PROCESS_MORE("                       \r\n");
            PROCESS_MORE("Interface Admin    Oper\r\n");
            PROCESS_MORE("--------- -------- ----\r\n");
#else
            PROCESS_MORE("                        Max      Used             \r\n");
            PROCESS_MORE("Interface Admin    Oper Power    Power    Priority\r\n");
            PROCESS_MORE("--------- -------- ---- -------- -------- --------\r\n");
#endif
#endif
#endif
            sprintf(buff_temp, "Eth %lu/%2lu  ", unit, port);
            strcat(buff, buff_temp);
            sprintf(buff_temp, "%-8s ", (port_entry.pse_port_admin_enable==VAL_pethPsePortAdminEnable_true)?("Enabled"):("Disabled"));
            strcat(buff, buff_temp);
#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
            sprintf(buff_temp, "%-8s ", (port_entry.pse_port_time_range_status==POE_TYPE_TIMERANGE_STATUS_NONE)?("--"):((port_entry.pse_port_time_range_status==POE_TYPE_TIMERANGE_STATUS_ACTIVE)? ("Active"):("Inactive")));
            strcat(buff, buff_temp);
#endif
            sprintf(buff_temp, "%-4s ", (port_entry.pse_port_detection_status==VAL_pethPsePortDetectionStatus_deliveringPower)?("On"):("Off"));
            strcat(buff, buff_temp);
#if (SYS_CPNT_POE_POE_POWER_INLINE_STATUS_SHOW_OPER_ONLY != TRUE)
            sprintf(buff_temp, "%5lu mW ", port_entry.pse_port_power_max_allocation);
            strcat(buff,buff_temp);
            sprintf(buff_temp, "%5lu mW ", port_entry.pse_port_consumption_power);
            strcat(buff,buff_temp);
            sprintf(buff_temp, "%-8s ", (port_entry.pse_port_power_priority==VAL_pethPsePortPowerPriority_critical)?("Critical"):(port_entry.pse_port_power_priority==VAL_pethPsePortPowerPriority_high)?("High"):("Low"));
            strcat(buff, buff_temp);
#endif
#ifdef SYS_CPNT_POE_PSE_DOT3AT
#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM)
            sprintf(buff_temp, "%-8s ", port_entry.pse_port_force_power_mode? "Enabled" : "Disabled");
            strcat(buff, buff_temp);
#endif
#endif

            strcat(buff, "\r\n");
            PROCESS_MORE(buff);
        }
        else
        {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to get power inline identifier at %lu/%lu.\r\n", unit, port);
#endif
            return CLI_NO_ERROR;
        }

    }
    else
    {
        BOOL_T is_any_unit_support_poe = FALSE;

        while(POE_POM_GetNextLegacyDetection(&unit, &power_inline_compatible_status) == TRUE)
        {
            is_any_unit_support_poe = TRUE;
            switch(power_inline_compatible_status)
            {
                case 0:
                    sprintf(buff, "Unit: %lu\r\n", unit);
                    PROCESS_MORE(buff);
                    sprintf(buff, "Compatible  mode : Disabled\r\n");
                    PROCESS_MORE(buff);
                    break;

                case 1:
                    sprintf(buff, "Unit: %lu\r\n", unit);
                    PROCESS_MORE(buff);
                    sprintf(buff, "Compatible  mode : Enabled\r\n");
                    PROCESS_MORE(buff);
                    break;

                default:
                    break;
            }
            verify_unit = unit;
            verify_port = 0;
            is_show_header = TRUE;
#if 1
            while(POE_POM_GetNextPsePortEntry(&verify_unit, &verify_port, &port_entry))
            {
#else
            while(POE_POM_GetNextPsePortAdmin(&verify_unit, &verify_port, &portadmin))
            {
#endif
                if (verify_unit != unit)
                {
                    break;
                }
                ret = verify_ethernet(verify_unit, verify_port, &ifindex);
                if ((ret==CLI_API_ETH_NOT_PRESENT) || (ret==CLI_API_ETH_UNKNOWN_PORT))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Ethernet %lu/%lu is not present.\r\n", verify_unit, verify_port);
#endif
                    continue;
                }

#if 0
                POE_POM_GetPsePortDetectionStatus(verify_unit,verify_port,&detectionstatus);
                POE_POM_GetPsePortPowerPriority(verify_unit, verify_port,&powerpriority);
                POE_POM_GetPortPowerMaximumAllocation(verify_unit, verify_port,&milliwatts);
                POE_POM_GetPortPowerConsumption(verify_unit, verify_port,&consumption);
                POE_POM_GetPortManualHighPowerMode(verify_unit,verify_port,&highpower);
#endif
                if(is_show_header)
                {
#if ((defined SYS_CPNT_POE_PSE_DOT3AT) && (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM))
#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
                    PROCESS_MORE("                   Time          Max      Used              High    \r\n");
                    PROCESS_MORE("Interface Admin    Range    Oper Power    Power    Priority Power   \r\n");
                    PROCESS_MORE("--------- -------- -------- ---- -------- -------- -------- --------\r\n");
#else
                    PROCESS_MORE("                        Max      Used              High    \r\n");
                    PROCESS_MORE("Interface Admin    Oper Power    Power    Priority Power   \r\n");
                    PROCESS_MORE("--------- -------- ---- -------- -------- -------- --------\r\n");
#endif
#else
#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
                    PROCESS_MORE("                   Time          Max      Used             \r\n");
                    PROCESS_MORE("Interface Admin    Range    Oper Power    Power    Priority\r\n");
                    PROCESS_MORE("--------- -------- -------- ---- -------- -------- --------\r\n");
#else
#if (SYS_CPNT_POE_POE_POWER_INLINE_STATUS_SHOW_OPER_ONLY == TRUE)
                    PROCESS_MORE("                       \r\n");
                    PROCESS_MORE("Interface Admin    Oper\r\n");
                    PROCESS_MORE("--------- -------- ----\r\n");
#else
                    PROCESS_MORE("                        Max      Used             \r\n");
                    PROCESS_MORE("Interface Admin    Oper Power    Power    Priority\r\n");
                    PROCESS_MORE("--------- -------- ---- -------- -------- --------\r\n");
#endif
#endif
#endif
                    is_show_header = FALSE;
                }

                sprintf(buff_temp, "Eth %lu/%2lu  ", verify_unit, verify_port);
                strcat(buff, buff_temp);
                sprintf(buff_temp, "%-8s ", (port_entry.pse_port_admin_enable==VAL_pethPsePortAdminEnable_true)?("Enabled"):("Disabled"));
                strcat(buff, buff_temp);
#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
                sprintf(buff_temp, "%-8s ", (port_entry.pse_port_time_range_status==POE_TYPE_TIMERANGE_STATUS_NONE)?("--"):((port_entry.pse_port_time_range_status==POE_TYPE_TIMERANGE_STATUS_ACTIVE)? ("Active"):("Inactive")));
                strcat(buff, buff_temp);
#endif
                sprintf(buff_temp, "%-4s ", (port_entry.pse_port_detection_status==VAL_pethPsePortDetectionStatus_deliveringPower)?("On"):("Off"));
                strcat(buff, buff_temp);
#if (SYS_CPNT_POE_POE_POWER_INLINE_STATUS_SHOW_OPER_ONLY != TRUE)
                sprintf(buff_temp, "%5lu mW ", port_entry.pse_port_power_max_allocation);
                strcat(buff, buff_temp);
                sprintf(buff_temp, "%5lu mW ", port_entry.pse_port_consumption_power);
                strcat(buff,buff_temp);
                sprintf(buff_temp, "%-8s ", (port_entry.pse_port_power_priority==VAL_pethPsePortPowerPriority_critical)?("Critical"):(port_entry.pse_port_power_priority==VAL_pethPsePortPowerPriority_high)?("High"):("Low"));
                strcat(buff, buff_temp);
#endif
#ifdef SYS_CPNT_POE_PSE_DOT3AT
#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM)
                sprintf(buff_temp, "%-8s ", port_entry.pse_port_force_power_mode?"Enabled":"Disabled");
                strcat(buff, buff_temp);
#endif
#endif
                strcat(buff, "\r\n");
                PROCESS_MORE(buff);
            }
        }
        if (is_any_unit_support_poe == FALSE)
            CLI_LIB_PrintStr("There is no unit supporting PoE.\r\n");
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_POE_Show_Power_Inline_TimeRange(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if ((SYS_CPNT_POE==TRUE) && (SYS_CPNT_POE_TIME_RANGE == TRUE))
    UI32_T unit  = 0;
    UI32_T port  = 0;
    UI32_T j = 0;
    UI32_T i = 0;
    UI32_T ifindex = 0;
    UI32_T verify_port = 0;
    UI32_T line_num = 0;
    UI32_T status;
    UI8_T  time_range[SYS_ADPT_TIME_RANGE_MAX_NAME_LENGTH+1] = {'\0'};
    char   buff_temp[CLI_DEF_MAX_BUFSIZE] = {0};
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    BOOL_T is_show_header = TRUE;
    CLI_API_EthStatus_T ret;

DBG_PRINT();
    if(arg[0]!=NULL)
    {
        unit = atoi(arg[1]);
        port = atoi( strchr(arg[1], '/')+1 );

        if (STKTPLG_OM_IsPoeDevice(unit) == FALSE)
        {
            CLI_LIB_PrintStr_1("Unit %lu does not support PoE.\r\n", unit);

            return CLI_NO_ERROR;
        }

        verify_port = i;
        ret = verify_ethernet(unit,port,&ifindex);
        if ((ret==CLI_API_ETH_NOT_PRESENT) || (ret==CLI_API_ETH_UNKNOWN_PORT))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_2("Ethernet %lu/%lu is not present.\r\n",unit,port);
#endif
            return CLI_NO_ERROR;
        }

        if ((POE_POM_GetPsePortTimeRangeName(unit,port,time_range)) &&
            (POE_POM_GetPsePortTimeRangeStatus(unit,port,&status)))
        {
            PROCESS_MORE("Interface Time Range Name  Status  \r\n");
            PROCESS_MORE("--------- ---------------- --------\r\n");

            sprintf(buff_temp, "Eth %lu/%2lu  ",unit,port);
            strcat(buff,buff_temp);
            sprintf(buff_temp, "%-16s ",(char*)time_range);
            strcat(buff,buff_temp);
            sprintf(buff_temp, "%-8s ",(status==POE_TYPE_TIMERANGE_STATUS_NONE)?("--"):((status==POE_TYPE_TIMERANGE_STATUS_ACTIVE)? ("Active"):("Inactive")));
            strcat(buff,buff_temp);
            strcat(buff,"\r\n");
            PROCESS_MORE(buff);
        }
        else
        {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to get time range of inline power at %lu/%lu.",unit,port);
#endif
            return CLI_NO_ERROR;
        }

    }
    else
    {
        BOOL_T is_any_unit_support_poe = FALSE;

        j = -1;
        while(POE_POM_GetNextPsePortTimeRangeName(&unit,&port,time_range))
        {
            if (STKTPLG_OM_IsPoeDevice(unit) == FALSE)
                continue;

            is_any_unit_support_poe = TRUE;
            if (j != unit)
            {
                if (j != -1)
                    PROCESS_MORE("\r\n");
                sprintf(buff,"Unit: %lu\r\n",unit);
                PROCESS_MORE(buff);
                PROCESS_MORE("Interface Time Range Name  Status  \r\n");
                PROCESS_MORE("--------- ---------------- --------\r\n");

                j = unit;
            }
            verify_port = port;
            is_show_header = TRUE;
            ret = verify_ethernet(j,verify_port,&ifindex);

            if ((ret==CLI_API_ETH_NOT_PRESENT) || (ret==CLI_API_ETH_UNKNOWN_PORT))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Ethernet %lu/%lu is not present.\r\n", j, verify_port);
#endif
                continue;
            }

            if (POE_POM_GetPsePortTimeRangeStatus(unit,port,&status))
            {
                sprintf(buff_temp, "Eth %lu/%2lu  ",unit,port);
                strcat(buff,buff_temp);
                sprintf(buff_temp, "%-16s ",(char*)time_range);
                strcat(buff,buff_temp);
                sprintf(buff_temp, "%-8s ",(status==POE_TYPE_TIMERANGE_STATUS_NONE)?("--"):((status==POE_TYPE_TIMERANGE_STATUS_ACTIVE)? ("Active"):("Inactive")));
                strcat(buff,buff_temp);
                strcat(buff,"\r\n");
                PROCESS_MORE(buff);
            }
            else
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to get time range of inline power at %lu/%lu.\r\n",unit,port);
#endif
                return CLI_NO_ERROR;
            }
        }
        if (is_any_unit_support_poe == FALSE)
            CLI_LIB_PrintStr("There is no unit supporting PoE.\r\n");
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_POE_Show_Power_Mainpower(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_POE==TRUE)
    POE_OM_MainPse_T entry;
    UI32_T unit_id = 0;
    UI32_T main_power_allocate = 0, main_power_available;
#if (SYS_CPNT_THERMAL_DETECTION == TRUE )
    UI32_T temp_value = 0;
#endif
    STKTPLG_BOARD_BoardInfo_T board_info;
    UI32_T board_id;
#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
    BOOL_T local_power = FALSE;
#endif
    UI8_T version1 = 0;
    UI8_T version2 = 0;
    UI8_T build = 0;

DBG_PRINT();
    if (arg[0] == NULL)
    {
        BOOL_T is_any_unit_support_poe = FALSE;
        while(POE_POM_GetNextMainPseEntry(&unit_id, &entry))
        {
            is_any_unit_support_poe = TRUE;
#if (SYS_CPNT_THERMAL_DETECTION == TRUE )
            SYS_MGR_GetTempValue(&temp_value);
#endif
            POE_POM_GetMainpowerMaximumAllocation(unit_id, &main_power_allocate);
            POE_POM_GetPoeSoftwareVersion(unit_id, &version1, &version2, &build);

            STKTPLG_OM_GetUnitBoardID(unit_id, &board_id);
            STKTPLG_BOARD_GetBoardInformation(board_id, &board_info);
    #if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
            POE_POM_GetUseLocalPower(unit_id, &local_power);
            main_power_available = (local_power? board_info.main_pse_power_max_allocation:board_info.main_pse_power_max_allocation_rps);
    #else
            main_power_available = board_info.main_pse_power_max_allocation;
    #endif
            CLI_LIB_PrintStr_1("Unit %lu PoE Status\r\n",unit_id);

    #if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
            CLI_LIB_PrintStr_2(" PoE Maximum Available Power  : %.1lf Watts (%s)\r\n", (double)main_power_available/1000,
            local_power? "using main power" : "using RPS");
    #else
            CLI_LIB_PrintStr_1(" PoE Maximum Available Power  : %.1lf Watts\r\n", (double)main_power_available/1000)
    #endif/* End of #if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE) */
            CLI_LIB_PrintStr_1(" PoE Maximum Allocation Power : %.1lf Watts\r\n", (double)main_power_allocate/1000);

            CLI_LIB_PrintStr_1(" System Operation Status      : %s \r\n",(entry.main_pse_oper_status==VAL_pethMainPseOperStatus_on)?("On"):(entry.main_pse_oper_status==2)?("Off"):("Fault"));

            CLI_LIB_PrintStr_1(" PoE Power Consumption        : %.1lf Watts\r\n", (double)entry.main_pse_consumption_power/1000);

#if (SYS_CPNT_THERMAL_DETECTION == TRUE )
            CLI_LIB_PrintStr_1(" Thermal Temperature          : %lu (in Celsius)\r\n",temp_value);
#endif
#if (SYS_CPNT_POE_ASIC==SYS_CPNT_POE_ASIC_BROADCOM)
            CLI_LIB_PrintStr_2(" Software Version        : Version %d.%d\r\n",(version1>>4)&0xf, version1&0xf);
#else
#if (SYS_CPNT_POE_ASIC_WITHOUT_MCU == TRUE)
            CLI_LIB_PrintStr_3(" Software Version             : Microsemi SDK V%d.%d.%d\r\n",version1,version2,build);
#else
            CLI_LIB_PrintStr_3(" Software Version             : Version %02X%02X (Hex), Build %02X (Hex)\r\n",version1,version2,build);
#endif
#endif
        }
        if (is_any_unit_support_poe == FALSE)
            CLI_LIB_PrintStr("There is no unit supporting PoE.\r\n");
    }
    else
    {
        unit_id = atoi(arg[1]);
        if (!STKTPLG_POM_UnitExist(unit_id))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1("Unit %lu is not exist\r\n",unit_id);
#endif
            return CLI_NO_ERROR;
        }

        if (STKTPLG_OM_IsPoeDevice(unit_id) == FALSE)
        {
            CLI_LIB_PrintStr_1("Unit %lu does not support PoE.\r\n", unit_id);

            return CLI_NO_ERROR;
        }

        if(POE_POM_GetPethMainPseEntry(unit_id,&entry))
        {
#if (SYS_CPNT_THERMAL_DETECTION == TRUE )
            SYS_MGR_GetTempValue(&temp_value);
#endif
            POE_POM_GetMainpowerMaximumAllocation(unit_id, &main_power_allocate);
            POE_POM_GetPoeSoftwareVersion(unit_id, &version1, &version2, &build);

            STKTPLG_OM_GetUnitBoardID(unit_id, &board_id);
            STKTPLG_BOARD_GetBoardInformation(board_id, &board_info);
    #if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
            POE_POM_GetUseLocalPower(unit_id, &local_power);
            main_power_available = (local_power? board_info.main_pse_power_max_allocation:board_info.main_pse_power_max_allocation_rps);
    #else
            main_power_available = board_info.main_pse_power_max_allocation;
    #endif
            CLI_LIB_PrintStr_1("Unit %lu PoE Status\r\n",unit_id);

    #if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
            CLI_LIB_PrintStr_2(" PoE Maximum Available Power  : %.1lf Watts (%s)\r\n", (double)main_power_available/1000,
            local_power? "using main power" : "using RPS");
    #else
            CLI_LIB_PrintStr_1(" PoE Maximum Available Power  : %.1lf Watts\r\n", (double)main_power_available/1000);
    #endif/* End of #if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE) */
            CLI_LIB_PrintStr_1(" PoE Maximum Allocation Power : %.1lf Watts\r\n", (double)main_power_allocate/1000);
            CLI_LIB_PrintStr_1(" System Operation Status      : %s \r\n",(entry.main_pse_oper_status==VAL_pethMainPseOperStatus_on)?("On"):(entry.main_pse_oper_status==2)?("Off"):("Fault"));
            CLI_LIB_PrintStr_1(" PoE Power Consumption        : %.1lf Watts\r\n",(double)entry.main_pse_consumption_power/1000);
#if (SYS_CPNT_THERMAL_DETECTION == TRUE )
            CLI_LIB_PrintStr_1(" Thermal Temperature          : %lu (in Celsius)\r\n",temp_value);
#endif
#if (SYS_CPNT_POE_ASIC==SYS_CPNT_POE_ASIC_BROADCOM)
            CLI_LIB_PrintStr_2(" Software Version        : Version %d.%d\r\n",(version1>>4)&0xf, version1&0xf);
#else
#if (SYS_CPNT_POE_ASIC_WITHOUT_MCU == TRUE)
            CLI_LIB_PrintStr_3(" Software Version             : Microsemi SDK V%d.%d.%d\r\n",version1,version2,build);
#else
            CLI_LIB_PrintStr_3(" Software Version             : Version %02X%02X (Hex), Build %02X (Hex)\r\n",version1,version2,build);
#endif
#endif
        }
        else
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1("Failed to get power main power allocation at unit %lu.\r\n",unit_id);
#endif
            return CLI_NO_ERROR;
        }
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_POE_Power_Mainpower_Maximum_Allocation(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if ((SYS_CPNT_POE==TRUE) && (SYS_CPNT_POE_MAX_ALLOC_FIXED != TRUE))
    UI32_T milliwatts = 0;
    UI32_T unit_id = 0;
    UI32_T default_milliwatts = MAIN_PSE_POWER_MAX_ALLOCATION;
    UI32_T board_id;
    STKTPLG_BOARD_BoardInfo_T board_info;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W4_POWER_MAINPOWER_MAXIMUM_ALLOCATION:
            milliwatts = atoi(arg[0]);

            if (arg[1] == NULL)
            {
                unit_id = 0;
                while(STKTPLG_OM_GetNextUnit(&unit_id))
                {
                    if(STKTPLG_OM_IsPoeDevice(unit_id) == FALSE)
                    {
                        CLI_LIB_PrintStr_1("Unit %lu does not support PoE.\r\n", unit_id);
                        continue;
                    }

                    if(!POE_PMGR_SetMainpowerMaximumAllocation(unit_id, milliwatts))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set power maximum allocation %lu milliwatts at unit %lu.\r\n", milliwatts, unit_id);
#endif
                    }
                }
            }
            else
            {
                unit_id = atoi(arg[2]);
                if (!STKTPLG_POM_UnitExist(unit_id))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Unit %lu does not exist.\r\n", unit_id);
#endif
                    return CLI_NO_ERROR;
                }

                if(STKTPLG_OM_IsPoeDevice(unit_id) == FALSE)
                {
                    CLI_LIB_PrintStr_1("Unit %lu does not support PoE.\r\n", unit_id);
                    return CLI_NO_ERROR;
                }

                if(!POE_PMGR_SetMainpowerMaximumAllocation(unit_id, milliwatts))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to set power maximum allocation %lu milliwatts at unit %lu.\r\n", milliwatts, unit_id);
#endif
                }
            }
            break;
        case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_POWER_MAINPOWER_MAXIMUM_ALLOCATION:
            if (arg[0] == NULL)
            {
                unit_id = 0;
                while(STKTPLG_OM_GetNextUnit(&unit_id))
                {
                    if(STKTPLG_OM_IsPoeDevice(unit_id) == FALSE)
                    {
                        CLI_LIB_PrintStr_1("Unit %lu does not support PoE.\r\n", unit_id);
                        continue;
                    }

                    STKTPLG_OM_GetUnitBoardID(unit_id, &board_id);
                    memset(&board_info, 0, sizeof(board_info));
                    if(STKTPLG_BOARD_GetBoardInformation(board_id, &board_info))
                        default_milliwatts = board_info.main_pse_power_max_allocation;

                    if(!POE_PMGR_SetMainpowerMaximumAllocation(unit_id, default_milliwatts))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("Failed to set power maximum allocation default milliwatts at unit %lu.\r\n", unit_id);
#endif
                    }
                }
            }
            else
            {
                unit_id = atoi(arg[1]);
                if (!STKTPLG_POM_UnitExist(unit_id))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Unit %lu does not exist\r\n", unit_id);
#endif
                    return CLI_NO_ERROR;
                }

                if(STKTPLG_OM_IsPoeDevice(unit_id) == FALSE)
                {
                    CLI_LIB_PrintStr_1("Unit %lu does not support PoE.\r\n", unit_id);
                    return CLI_NO_ERROR;
                }

                STKTPLG_OM_GetUnitBoardID(unit_id, &board_id);
                memset(&board_info, 0, sizeof(board_info));
                if(STKTPLG_BOARD_GetBoardInformation(board_id, &board_info))
                    default_milliwatts = board_info.main_pse_power_max_allocation;

                if(!POE_PMGR_SetMainpowerMaximumAllocation(unit_id, default_milliwatts))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to set power maximum allocation default milliwatts at unit %lu.\r\n", unit_id);
#endif
                }
            }
            break;
        default:
            return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_POE_Power_Inline(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_POE==TRUE)
    UI32_T enable  = VAL_pethPsePortAdminEnable_true;
    UI32_T disable = VAL_pethPsePortAdminEnable_false;
#if 0
    UI8_T  entry[4] = {0};
    UI32_T entry_type = PSE_PORT_POWER_DETECTION_CONTROL_DEF;
#endif

    UI32_T i               = 0;
    UI32_T ifindex         = 0;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;

    DBG_PRINT();
    if(STKTPLG_OM_IsPoeDevice(verify_unit) == FALSE)
    {
        CLI_LIB_PrintStr_1("Unit %lu does not support PoE.\r\n", verify_unit);
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_POWER_INLINE:
#if 0
            if(arg[0]!=NULL)
            {
                if(arg[0][0]=='a')
                {
                    strcpy(entry,"auto");
                    entry_type = PSE_PORT_POWER_DETECTION_CONTROL_DEF;
                }
                else
                {
                    strcpy(entry,"test");
                    entry_type = VAL_pethPsePortPowerDetectionControl_test;
                }
            }
#endif
            for (i = 1;i <= ctrl_P->sys_info.max_port_number ; i++)
            {
                if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
                {
                    verify_port = i;
                    if ((verify_port<SYS_ADPT_POE_PSE_MIN_PORT_NUMBER) || (verify_port>SYS_ADPT_POE_PSE_MAX_PORT_NUMBER))
                    {
                        CLI_LIB_PrintStr_2("Ethernet %lu/%lu could not configure PoE.\r\n",verify_unit,verify_port);
                        continue;
                    }

                    if((verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Ethernet %lu/%lu is not present.\r\n",verify_unit, verify_port);
#endif
                        continue;
                    }
                    else
                    {
                        if(!POE_PMGR_SetPsePortAdmin(verify_unit, verify_port, enable))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr("Failed to enable inline power.\r\n");
#endif
                            return CLI_NO_ERROR;
                        }

#if 0
                        if(arg[0]!=NULL)
                        {
                            if(!POE_MGR_SetPsePortPowerDetectionCtrl(verify_unit, verify_port, entry_type))
                            {
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr("Failed to set power inline identifier.\r\n");
#endif
                                return CLI_NO_ERROR;
                            }
                        }
#endif

                    }
                }
            }

            break;
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_POWER_INLINE:
            for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
            {
                if(ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
                {
                    verify_port = i;
                    if ((verify_port<SYS_ADPT_POE_PSE_MIN_PORT_NUMBER) || (verify_port>SYS_ADPT_POE_PSE_MAX_PORT_NUMBER))
                    {
                        CLI_LIB_PrintStr_2("Ethernet %lu/%lu could not configure PoE.\r\n",verify_unit,verify_port);
                        continue;
                    }
                    if((verify_ret = verify_ethernet(verify_unit,verify_port, &ifindex)) != CLI_API_ETH_OK)
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Ethernet %lu/%lu is not present.\r\n",verify_unit,verify_port);
#endif
                        continue;
                    }
                    else
                    {
                        if(!POE_PMGR_SetPsePortAdmin(verify_unit, verify_port, disable))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr("Failed to disable inline power.\r\n");
#endif
                            return CLI_NO_ERROR;
                        }
                    }
                }
            }
            break;
        default:
            return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_POE_Power_Inline_Maximum_Allocation(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if ((SYS_CPNT_POE==TRUE) && (SYS_CPNT_POE_INLINE_MAX_ALLOC_FIXED != TRUE))
    UI32_T milliwatts  = 0;
    UI32_T default_milliwatts  = PSE_PORT_POWER_MAX_ALLOCATION;
    UI32_T i               = 0;
    UI32_T ifindex         = 0;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;

#if (SYS_CPNT_POE_PORT_MAX_ALLOC_DIFF == TRUE)
    STKTPLG_BOARD_BoardInfo_T board_info;
    UI32_T board_id;

    STKTPLG_OM_GetUnitBoardID(verify_unit, &board_id);
    STKTPLG_BOARD_GetBoardInformation(board_id, &board_info);
#endif

    DBG_PRINT();
    if(STKTPLG_OM_IsPoeDevice(verify_unit) == FALSE)
    {
        CLI_LIB_PrintStr_1("Unit %lu does not support PoE.\r\n", verify_unit);
        return CLI_NO_ERROR;
    }

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            if(verify_port>SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT)
            {
                CLI_LIB_PrintStr_2("Ethernet %lu/%lu could not configure PoE.\r\n",verify_unit,verify_port);
                continue;
            }

            if((verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Ethernet %lu/%lu is not present.\r\n",verify_unit, verify_port);
#endif
                continue;
            }
            else
            {
                switch(cmd_idx)
                {
                    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_POWER_INLINE_MAXIMUM_ALLOCATION:
                        milliwatts = atoi(arg[0]);
#if (SYS_CPNT_POE_PORT_MAX_ALLOC_DIFF == TRUE)
                        if (milliwatts > board_info.per_port_power_max_allocation[verify_port-1])
                        {
                            CLI_LIB_PrintStr("Up to 30000 mW on the first 6 ports, and 15400 mW on the other.\r\n");

                            return CLI_NO_ERROR;
                        }
#endif
                        if(!POE_PMGR_SetPortPowerMaximumAllocation(verify_unit,verify_port,milliwatts))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_3("Failed to set inline power maximum allocation %lu milliwatts at %lu/%lu.\r\n",milliwatts,verify_unit,verify_port);
#endif
                            return CLI_NO_ERROR;
                        }
                        break;
                    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W5_NO_POWER_INLINE_MAXIMUM_ALLOCATION:
#if (SYS_CPNT_POE_PORT_MAX_ALLOC_DIFF == TRUE)
                        default_milliwatts = board_info.per_port_power_max_allocation[verify_port-1];
#endif
                        if(!POE_PMGR_SetPortPowerMaximumAllocation(verify_unit,verify_port,default_milliwatts))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_2("Failed to set inline power maximum allocation default milliwatts at %lu/%lu.\r\n",verify_unit,verify_port);
#endif
                            return CLI_NO_ERROR;
                        }
                        break;
                    default:
                        return CLI_ERR_INTERNAL;
                }
            }
        }
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_POE_Power_Inline_Priority(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if ((SYS_CPNT_POE==TRUE) && (SYS_CPNT_POE_PRIORITY_SUPPORT == TRUE))
    UI32_T entry_priority;
    UI32_T i               = 0;
    UI32_T ifindex         = 0;
    UI32_T verify_port;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    CLI_API_EthStatus_T verify_ret;

    DBG_PRINT();
    if(STKTPLG_OM_IsPoeDevice(verify_unit) == FALSE)
    {
        CLI_LIB_PrintStr_1("Unit %lu does not support PoE.\r\n", verify_unit);
        return CLI_NO_ERROR;
    }

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            if(verify_port>SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT)
            {
                CLI_LIB_PrintStr_2("Ethernet %lu/%lu could not configure PoE.\r\n",verify_unit,verify_port);
                continue;
            }

            if((verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Ethernet %lu/%lu is not present.\r\n",verify_unit, verify_port);
#endif
                continue;
            }
            else
            {
                switch(cmd_idx)
                {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_POWER_INLINE_PRIORITY:
                    entry_priority = atoi(arg[0]);
                    if(!POE_PMGR_SetPsePortPowerPriority(verify_unit, verify_port, entry_priority))
                    {
    #if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
    #else
                        CLI_LIB_PrintStr_3("Failed to set inline power priority %lu identifier at %lu/%lu.\r\n",entry_priority,verify_unit,verify_port);
    #endif
                        return CLI_NO_ERROR;
                    }
                    break;

                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_POWER_INLINE_PRIORITY:
                    entry_priority  = PSE_PORT_POWER_PRIORITY_DEF;
                    if(!POE_PMGR_SetPsePortPowerPriority(verify_unit, verify_port, entry_priority))
                    {
    #if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
    #else
                        CLI_LIB_PrintStr_3("Failed to set inline power default priority %lu identifier at %lu/%lu.\r\n",entry_priority,verify_unit,verify_port);
    #endif
                        return CLI_NO_ERROR;
                    }
                    break;

                default:
                    return CLI_ERR_INTERNAL;
                }
            }
        }
    }
#endif
    return CLI_NO_ERROR;
}

#if 0 /* not support now */
UI32_T CLI_API_POE_Power_Download(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P)
{
#if (SYS_CPNT_POE==TRUE)
    UI8_T filename[16] = {0};
    UI32_T current_max_unit = 0;
    UI32_T  j = 0;
#if (SYS_CPNT_STACKING == TRUE)
        STKTPLG_POM_GetNumberOfUnit(&current_max_unit);
#else
        current_max_unit = 1;
#endif
DBG_PRINT();
    strcpy(filename,arg[0]);
    CLI_LIB_PrintStr("Software downloading in progress, please wait...\r\n");
    for(j = 1 ; j <= current_max_unit ; j++)
    {
        if(!POE_MGR_SoftwareDownload(j, filename))
        {
            CLI_LIB_PrintStr_1("Failed to download file at unit %lu\r\n",j);
        }
        else
        {
            CLI_LIB_PrintStr_1("Unit %lu done\r\n",j);
        }
    }
#endif
    return CLI_NO_ERROR;
}
#endif

UI32_T CLI_API_POE_Power_Inline_Compatible(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T  *ctrl_P)
{
#if (SYS_CPNT_POE == TRUE && SYS_CPNT_POE_COMPATIBLE_SUPPORT == TRUE)
    UI32_T power_inline_compatible_status=0;
    UI32_T unit;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_POWER_INLINE_COMPATIBLE:
            /*1 for Enable, 0 for disable*/
            power_inline_compatible_status = 1;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_POWER_INLINE_COMPATIBLE:
            /*1 for Enable, 0 for disable*/
            power_inline_compatible_status = 0;
            break;

        default:
            break;
    }

    if(arg[0] == NULL)
    {
        unit = 0;
        while(STKTPLG_OM_GetNextUnit(&unit))
        {
            if(STKTPLG_OM_IsPoeDevice(unit) == FALSE)
            {
                CLI_LIB_PrintStr_1("Unit %lu does not support PoE.\r\n", unit);
                continue;
            }
            if(POE_PMGR_SetLegacyDetection(unit, power_inline_compatible_status) != TRUE)
            {
                CLI_LIB_PrintStr_1("Fail to set compatible mode status for Unit %lu.\r\n", unit);
            }
        }
    }
    else
    {
        unit = atoi(arg[0]);

        if (STKTPLG_OM_UnitExist(unit) == FALSE)
        {
            CLI_LIB_PrintStr_1("Unit %lu does not exist.\r\n", unit);
            return CLI_NO_ERROR;
        }
        if(STKTPLG_OM_IsPoeDevice(unit) == FALSE)
        {
            CLI_LIB_PrintStr_1("Unit %lu does not support PoE.\r\n", unit);
            return CLI_NO_ERROR;
        }
        if(POE_PMGR_SetLegacyDetection(unit, power_inline_compatible_status) != TRUE)
        {
            CLI_LIB_PrintStr_1("Fail to set compatible mode status for Unit %lu.\r\n", unit);
        }
    }
#endif

    return CLI_NO_ERROR;
}

#ifdef SYS_CPNT_POE_PSE_DOT3AT
#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_BROADCOM)
UI32_T CLI_API_POE_Power_Inline_HighPower(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_POE==TRUE)
    UI32_T i               = 0;
    UI32_T ifindex         = 0;
    UI32_T verify_port;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    CLI_API_EthStatus_T verify_ret;

    DBG_PRINT();
    if(STKTPLG_OM_IsPoeDevice(verify_unit) == FALSE)
    {
        CLI_LIB_PrintStr_1("Unit %lu does not support PoE.\r\n", verify_unit);
        return CLI_NO_ERROR;
    }

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            if(verify_port>SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT)
            {
                CLI_LIB_PrintStr_2("Ethernet %lu/%lu could not configure PoE.\r\n",verify_unit,verify_port);
                continue;
            }

            if((verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Ethernet %lu/%lu is not present.\r\n",verify_unit, verify_port);
#endif
                continue;
            }
            else
            {
                switch(cmd_idx)
                {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_POWER_INLINE_HIGHPOWER:
                    if(!POE_PMGR_SetPortManualHighPowerMode(verify_unit, verify_port, 1))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set inline high-power at %lu/%lu.\r\n",verify_unit,verify_port);
#endif
                        return CLI_NO_ERROR;
                    }

                    break;
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_POWER_INLINE_HIGHPOWER:
                    if(!POE_PMGR_SetPortManualHighPowerMode(verify_unit, verify_port, 0))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set inline high-power at %lu/%lu.\r\n",verify_unit,verify_port);
#endif
                        return CLI_NO_ERROR;
                    }

                    break;
                default:
                    return CLI_ERR_INTERNAL;
                }
            }
        }
    }
#endif
    return CLI_NO_ERROR;
}
#endif
#endif

UI32_T CLI_API_POE_Power_Inline_TimeRange(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_POE_TIME_RANGE==TRUE)
    UI32_T i               = 0;
    UI32_T ifindex         = 0;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;
    TIME_RANGE_TYPE_ENTRY_T entry;

    DBG_PRINT();
    if(STKTPLG_OM_IsPoeDevice(verify_unit) == FALSE)
    {
        CLI_LIB_PrintStr_1("Unit %lu does not support PoE.\r\n", verify_unit);
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_POWER_INLINE_TIMERANGE:
            for (i = 1 ;i <= ctrl_P->sys_info.max_port_number ; i++)
            {
                if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
                {
                    verify_port = i;
                    if(verify_port>SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT)
                    {
                        CLI_LIB_PrintStr_2("Ethernet %lu/%lu could not configure PoE.\r\n",verify_unit,verify_port);
                        continue;
                    }

                    if((verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Ethernet %lu/%lu is not present.\r\n",verify_unit, verify_port);
#endif
                        continue;
                    }
                    else
                    {
                        if(arg[0]!=NULL)
                        {
                            if (!TIME_RANGE_PMGR_GetTimeRangeEntryByName((UI8_T *)arg[0], &entry))
                            {
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr_1("Time range name \"%s\" does not exist.\r\n", arg[0]);
#endif
                                return CLI_NO_ERROR;
                            }
                            else
                            {
                                if(!POE_PMGR_BindTimeRangeToPsePort(verify_unit, verify_port, (UI8_T *)arg[0]))
                                {
#if (SYS_CPNT_EH == TRUE)
                                    CLI_API_Show_Exception_Handeler_Msg();
#else
                                    CLI_LIB_PrintStr_1("Failed to bind inline power to \"%s\".\r\n", arg[0]);
#endif
                                    return CLI_NO_ERROR;
                                }
                            }
                        }
                        else
                        {
                            return CLI_ERR_INTERNAL;
                        }
                    }
                }
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_POWER_INLINE_TIMERANGE:
            for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
            {
                if(ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
                {
                    verify_port = i;
                    if(verify_port>SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT)
                    {
                        CLI_LIB_PrintStr_2("Ethernet %lu/%lu could not configure PoE.\r\n",verify_unit,verify_port);
                        continue;
                    }
                    if((verify_ret = verify_ethernet(verify_unit,verify_port, &ifindex)) != CLI_API_ETH_OK)
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Ethernet %lu/%lu is not present.\r\n",verify_unit,verify_port);
#endif
                        continue;
                    }
                    else
                    {
                        if(arg[0]==NULL)
                        {
                            if(!POE_PMGR_UnbindTimeRangeToPsePort(verify_unit, verify_port))
                            {
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr("Failed to unbind inline power.\r\n");
#endif
                                return CLI_NO_ERROR;
                            }
                        }
                        else
                        {
                            return CLI_ERR_INTERNAL;
                        }
                    }
                }
            }
            break;
        default:
            return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}

/* LOCAL SUBPROGRAM BODIES
*/


