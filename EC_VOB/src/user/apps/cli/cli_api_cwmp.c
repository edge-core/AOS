#include "cli_api.h"

#include "cli_api_cwmp.h"
#include "cwmp_pmgr.h"
#include "cwmp_pom.h"
#include "cwmp_type.h"
#include "sys_dflt.h"

UI32_T CLI_API_Cwmp_Url(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CWMP == TRUE)
    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_CWMP_URL:
        if (arg[0] == NULL)
            return CLI_ERR_INTERNAL;

        if (CWMP_PMGR_SetUrl(arg[0]) != TRUE)
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr("Failed to set the URL for connecting to ACS.\r\n");
    #endif
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_CWMP_URL:
        if (CWMP_PMGR_SetUrl(SYS_DFLT_CWMP_ACS_URL) != TRUE)
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr("Failed to reset the URL for connecting to ACS.\r\n");
    #endif
        }
        break;

    default:
        return CLI_ERR_INTERNAL;
    }
#endif

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Cwmp_Username(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CWMP == TRUE)
    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_CWMP_USERNAME:
        if (arg[0] == NULL)
            return CLI_ERR_INTERNAL;

        if (CWMP_PMGR_SetUsername(arg[0]) != TRUE)
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr("Failed to set the username for connecting to ACS.\r\n");
    #endif
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_CWMP_USERNAME:
        if (CWMP_PMGR_SetUsername(SYS_DFLT_CWMP_ACS_USERNAME) != TRUE)
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr("Failed to reset the username for connecting to ACS.\r\n");
    #endif
        }
        break;

    default:
        return CLI_ERR_INTERNAL;
    }
#endif

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Cwmp_Password(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CWMP == TRUE)
    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_CWMP_PASSWORD:
        if (arg[0] == NULL)
            return CLI_ERR_INTERNAL;

        if (CWMP_PMGR_SetPassword(arg[0]) != TRUE)
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr("Failed to set the password for connecting to ACS.\r\n");
    #endif
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_CWMP_PASSWORD:
        if (CWMP_PMGR_SetPassword(SYS_DFLT_CWMP_ACS_PASSWORD) != TRUE)
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr("Failed to reset the password for connecting to ACS.\r\n");
    #endif
        }
        break;

    default:
        return CLI_ERR_INTERNAL;
    }
#endif

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Cwmp_PeriodicInform(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CWMP == TRUE)
    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_CWMP_PERIODICINFORM:
        if (CWMP_PMGR_SetPeriodicInformEnable(TRUE) != TRUE)
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr("Failed to enable sending periodic Inform messages.\r\n");
    #endif
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_CWMP_PERIODICINFORM:
        if (CWMP_PMGR_SetPeriodicInformEnable(FALSE) != TRUE)
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr("Failed to disable sending periodic Inform messages.\r\n");
    #endif
        }
        break;

    default:
        return CLI_ERR_INTERNAL;
    }
#endif

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Cwmp_PeriodicInformInterval(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CWMP == TRUE)
    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W3_CWMP_PERIODICINFORM_INTERVAL:
        if (arg[0] == NULL)
            return CLI_ERR_INTERNAL;

        if (CWMP_PMGR_SetPeriodicInformInterval(atoi(arg[0])) != TRUE)
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr("Failed to set periodic-inform interval.\r\n");
    #endif
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_CWMP_PERIODICINFORM_INTERVAL:
        if (CWMP_PMGR_SetPeriodicInformInterval(SYS_DFLT_CWMP_PERIODIC_INFORM_INTERVAL) != TRUE)
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr("Failed to reset periodic-inform interval.\r\n");
    #endif
        }
        break;

    default:
        return CLI_ERR_INTERNAL;
    }
#endif

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Cwmp_ConnReqUsername(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CWMP == TRUE)
    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W3_CWMP_CONNECTIONREQUEST_PASSWORD:
        if (arg[0] == NULL)
            return CLI_ERR_INTERNAL;

        if (CWMP_PMGR_SetConnectionRequestUsername(arg[0]) != TRUE)
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr("Failed to set username for Connection Request.\r\n");
    #endif
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_CWMP_CONNECTIONREQUEST_USERNAME:
        if (CWMP_PMGR_SetConnectionRequestUsername(SYS_DFLT_CWMP_CONNECTION_REQUEST_USERNAME) != TRUE)
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr("Failed to reset username for Connection Request.\r\n");
    #endif
        }
        break;

    default:
        return CLI_ERR_INTERNAL;
    }
#endif

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Cwmp_ConnReqPassword(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CWMP == TRUE)
    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_CWMP_PASSWORD:
        if (arg[0] == NULL)
            return CLI_ERR_INTERNAL;

        if (CWMP_PMGR_SetConnectionRequestPassword(arg[0]) != TRUE)
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr("Failed to set password for Connection Request.\r\n");
    #endif
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_CWMP_PASSWORD:
        if (CWMP_PMGR_SetConnectionRequestPassword(SYS_DFLT_CWMP_CONNECTION_REQUEST_PASSWORD) != TRUE)
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr("Failed to reset password for Connection Request.\r\n");
    #endif
        }
        break;

    default:
        return CLI_ERR_INTERNAL;
    }
#endif

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Cwmp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CWMP == TRUE)
    UI8_T   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T  line_num = 0;
    CWMP_TYPE_ConfigEntry_T cwmp_config_entry;

        if (CWMP_POM_GetCwmpConfigEntry(&cwmp_config_entry) != TRUE)
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            printf("Failed to get CWMP configuration information.\r\n");
    #endif
        }

        sprintf(buff, "CWMP Configuration Information\r\n");
        PROCESS_MORE(buff);
        sprintf(buff, " URL                         : %s\r\n", cwmp_config_entry.url);
        PROCESS_MORE(buff);
        sprintf(buff, " Username                    : %s\r\n", cwmp_config_entry.username);
        PROCESS_MORE(buff);
        sprintf(buff, " Periodic Inform             : %s\r\n",
            ((cwmp_config_entry.periodic_inform_enable == TRUE) ? "Enabled" : "Disabled"));
        PROCESS_MORE(buff);
        sprintf(buff, " Periodic Inform Interval    : %lu seconds\r\n", cwmp_config_entry.periodic_inform_interval);
        PROCESS_MORE(buff);
        sprintf(buff, " Connection Request Username : %s\r\n", cwmp_config_entry.conn_req_username);
        PROCESS_MORE(buff);

#endif

    return CLI_NO_ERROR;
}
