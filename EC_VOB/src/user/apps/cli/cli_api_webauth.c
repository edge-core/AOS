#include "sys_type.h"
#include "sys_cpnt.h"
#include "cli_cmd.h"
#include "cli_def.h"
#include "cli_api.h"

#if (SYS_CPNT_WEBAUTH == TRUE)
#include "webauth_type.h"
#include "webauth_pmgr.h"
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

/* command: 1.1	web-auth system-auth-control */
UI32_T CLI_API_WebAuth_SYSTEMTAUTHCTRL(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_WEBAUTH == TRUE)
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_WEBAUTH_SYSTEMAUTHCONTROL:
        {
            if(WEBAUTH_PMGR_SetSystemStatus(VAL_webauthEnable_enabled) != WEBAUTH_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to enable WebAuth.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_WEBAUTH_SYSTEMAUTHCONTROL:
        {
            if(WEBAUTH_PMGR_SetSystemStatus(VAL_webauthEnable_disabled) != WEBAUTH_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to disable WebAuth.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }
    }
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

    return CLI_NO_ERROR;
}

/* command: 1.2	web-auth login-page-url */
UI32_T CLI_API_WebAuth_Login_URL(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_WEBAUTH == TRUE)
#if 0 /* 2007/11/09, chengrui_yeh, The following web-auth login url feature aren't supported yet. */
    WEBAUTH_TYPE_EXTERNAL_URL_T url_type;


    url_type = WEBAUTH_TYPE_EXTERNAL_URL_LOGIN;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_WEBAUTH_LOGINPAGEURL:
        {

            if(WEBAUTH_PMGR_SetExternalURL(arg[0],url_type )!=WEBAUTH_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set external login URL.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_WEBAUTH_LOGINPAGEURL:
        {
            if(WEBAUTH_PMGR_SetExternalURL("",url_type )!=WEBAUTH_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set external login fail URL.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }
    }
#endif /* #if 0 */
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

    return CLI_NO_ERROR;
}

/* command: 1.3	web-auth login-fail-page-url */
UI32_T CLI_API_WebAuth_Login_Fail_URL(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_WEBAUTH == TRUE)
#if 0 /* 2007/11/09, chengrui_yeh, The following web-auth login url feature aren't supported yet. */
    WEBAUTH_TYPE_EXTERNAL_URL_T url_type;


    url_type = WEBAUTH_TYPE_EXTERNAL_URL_LOGIN_FAIL;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_WEBAUTH_LOGINFAILPAGEURL:
        {

            if(WEBAUTH_PMGR_SetExternalURL(arg[0],url_type )!=WEBAUTH_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set external login URL.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_WEBAUTH_LOGINFAILPAGEURL:
        {
            if(WEBAUTH_PMGR_SetExternalURL("",url_type )!=WEBAUTH_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set external login fail URL.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }
    }
#endif /* #if 0 */
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

    return CLI_NO_ERROR;
}

/* command: 1.4	web-auth login-success-page-url */
UI32_T CLI_API_WebAuth_Login_Success_URL(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_WEBAUTH == TRUE)
#if 0 /* 2007/11/09, chengrui_yeh, The following web-auth login url feature aren't supported yet. */
    WEBAUTH_TYPE_EXTERNAL_URL_T url_type;


    url_type = WEBAUTH_TYPE_EXTERNAL_URL_LOGIN_SUCCESS;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_WEBAUTH_LOGINSUCCESSPAGEURL:
        {

            if(WEBAUTH_PMGR_SetExternalURL(arg[0],url_type )!=WEBAUTH_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set external login success URL.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_WEBAUTH_LOGINSUCCESSPAGEURL:
        {
            if(WEBAUTH_PMGR_SetExternalURL("",url_type )!=WEBAUTH_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set external login success URL.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }
    }
#endif /* #if 0 */
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

    return CLI_NO_ERROR;
}

/* command: 1.5	web-auth session-timeout */
UI32_T CLI_API_WebAuth_Session_Timeout(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_WEBAUTH == TRUE)
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_WEBAUTH_SESSIONTIMEOUT:
        {

            if(WEBAUTH_PMGR_SetSystemSessionTimeout(atoi(arg[0]))!=WEBAUTH_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set session timeout.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_WEBAUTH_SESSIONTIMEOUT:
        {

            if(WEBAUTH_PMGR_SetSystemSessionTimeout(SYS_DFLT_WEBAUTH_SESSION_TIMEOUT)!=WEBAUTH_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set session timeout.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }
   }
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

   return CLI_NO_ERROR;
}

/* command: 1.6	web-auth quiet-period */
UI32_T CLI_API_WebAuth_Quiet_Period(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_WEBAUTH == TRUE)
    UI16_T quiet_period;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_WEBAUTH_QUIETPERIOD:
        {
            quiet_period = atoi(arg[0]);

            if(WEBAUTH_PMGR_SetQuietPeriod(quiet_period)!=WEBAUTH_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set quiet period.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_WEBAUTH_QUIETPERIOD:
        {
            if(WEBAUTH_PMGR_SetQuietPeriod(SYS_DFLT_WEBAUTH_QUIET_PERIOD)!=WEBAUTH_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set quiet period.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }
   }
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

   return CLI_NO_ERROR;
}

/* command: 1.7	web-auth login-attempts */
UI32_T CLI_API_WebAuth_Login_Attempt(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_WEBAUTH == TRUE)
    UI8_T  login_attempt;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_WEBAUTH_LOGINATTEMPTS:
        {
            login_attempt = (UI8_T)atoi(arg[0]);


            if(WEBAUTH_PMGR_SetMaxLoginAttempts(login_attempt)!=WEBAUTH_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set max login attempts.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_WEBAUTH_LOGINATTEMPTS:
        {
            if(WEBAUTH_PMGR_SetMaxLoginAttempts(SYS_DFLT_WEBAUTH_MAX_LOGIN_ATTEMPTS)!=WEBAUTH_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set max login attempts.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }
   }
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

   return CLI_NO_ERROR;
}

/* command: 1.8	web-auth  */
UI32_T CLI_API_WebAuth_Port_Setting(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_WEBAUTH == TRUE)
    UI32_T i;
    UI32_T max_port_num = 0;
    CLI_API_EthStatus_T eth_status;
    UI32_T lport;

    max_port_num = SWCTRL_POM_UIGetUnitPortNumber(ctrl_P->CMenu.unit_id);

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_WEBAUTH:

        for (i = 1; i <= max_port_num; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                lport = 0;
                eth_status = verify_ethernet(ctrl_P->CMenu.unit_id, i, &lport);
                if (eth_status != CLI_API_ETH_OK)
                {
                   display_ethernet_msg(eth_status, ctrl_P->CMenu.unit_id, i);
                   continue;
                }

                if(WEBAUTH_PMGR_SetStatusByLPort(lport, VAL_webAuthPortConfigStatus_enabled) != WEBAUTH_TYPE_RETURN_OK)
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr("Failed to enable WebAuth.\r\n");
    #endif
                    return CLI_NO_ERROR;
                }
            }
        }
        break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_WEBAUTH:

        for (i = 1; i <= max_port_num; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                lport = 0;
                eth_status = verify_ethernet(ctrl_P->CMenu.unit_id, i, &lport);
                if (eth_status != CLI_API_ETH_OK)
                {
                   display_ethernet_msg(eth_status, ctrl_P->CMenu.unit_id, i);
                   continue;
                }

                if(WEBAUTH_PMGR_SetStatusByLPort(lport, VAL_webAuthPortConfigStatus_disabled) != WEBAUTH_TYPE_RETURN_OK)
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr("Failed to disable WebAuth.\r\n");
    #endif
                    return CLI_NO_ERROR;
                }
            }
        }
        break;
    }
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

    return CLI_NO_ERROR;
}


/* command: 1.9 web-auth re-authenticate interface interface [host]  */
UI32_T CLI_API_WebAuth_ReAuth_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_WEBAUTH == TRUE)
    CLI_API_EthStatus_T eth_status;
    UI32_T unit = 0;
    UI32_T port = 0;
    UI32_T lport= 0;
    UI32_T ip_addr;
    /*PRIVILEGE_EXEC_CMD_W3_WEBAUTH_REAUTHENTICATE_INTERFACE    */

    CLI_LIB_GetUnitPortFromString(arg[1], &unit, &port);
    eth_status = verify_ethernet(unit, port, &lport);
    if (eth_status != CLI_API_ETH_OK)
    {
       display_ethernet_msg(eth_status, unit, port);
       return CLI_NO_ERROR;
    }

    if(arg[2]==NULL)/* one port all hosts*/
    {
        if(WEBAUTH_PMGR_ReAuthByLPort(lport) != WEBAUTH_TYPE_RETURN_OK)
        {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to reauth.\r\n");
#endif
                    return CLI_NO_ERROR;
        }


    }
    else/* special host */
    {
        L_INET_Aton((UI8_T *)arg[2], &(ip_addr));

        if(WEBAUTH_PMGR_ReAuthHostByLPort(lport, ip_addr) != WEBAUTH_TYPE_RETURN_OK)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to reauth port.\r\n");
#endif
            return CLI_NO_ERROR;
        }
    }
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

    return CLI_NO_ERROR;
}

/* command: 2.1 show web-auth [summary | interface ethernet port] */
UI32_T CLI_API_Show_WebAuth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_WEBAUTH == TRUE)
    UI32_T line_num = 0;
    UI16_T quiet_period;
    UI16_T session_timeout;
    UI8_T global_status;
    UI8_T max_login_attempt;
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};

    PROCESS_MORE("Global Web-Auth Parameters:\r\n");

    WEBAUTH_PMGR_GetSystemStatus(&global_status);
    if (VAL_webauthEnable_enabled == global_status)
    {
    	PROCESS_MORE("  System Auth Control      : Enabled\r\n");
    }
    else
    {
    	PROCESS_MORE("  System Auth Control      : Disabled\r\n");
    }

#if 0 /* 2007/11/07, chengrui_yeh, The following web-auth login url feature aren't supported yet. */
    /* login url */
    WEBAUTH_PMGR_GetExternalURL(url,WEBAUTH_TYPE_EXTERNAL_URL_LOGIN);
    PROCESS_MORE_1("  Login Page URL           : %s\r\n", url);
    /* login fail */
    WEBAUTH_PMGR_GetExternalURL(url,WEBAUTH_TYPE_EXTERNAL_URL_LOGIN_FAIL);
    PROCESS_MORE_1("  Login Fail Page URL      : %s\r\n", url);
    /* success */
    WEBAUTH_PMGR_GetExternalURL(url,WEBAUTH_TYPE_EXTERNAL_URL_LOGIN_SUCCESS);
    PROCESS_MORE_1("  Login Success Page URL   : %s\r\n", url);
#endif /* #if 0 */

    WEBAUTH_PMGR_GetSystemSessionTimeout(&session_timeout);
    PROCESS_MORE_1("  Session Timeout          : %d seconds\r\n", session_timeout);

    WEBAUTH_PMGR_GetQuietPeriod(&quiet_period);
    PROCESS_MORE_1("  Quiet Period             : %d seconds\r\n", quiet_period);

    WEBAUTH_PMGR_GetMaxLoginAttempts(&max_login_attempt);
    PROCESS_MORE_1("  Max Login Attempts       : %d\r\n", max_login_attempt);
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_WebAuth_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_WEBAUTH == TRUE)
    UI32_T line_num = 0;
    CLI_API_EthStatus_T eth_status;
    UI32_T unit = 0;
    UI32_T port = 0;
    UI32_T l_port;
    UI16_T session_timeout;
    UI16_T current_time;
    UI8_T index;
    UI8_T port_status;
    UI8_T temp[30] = {0};
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};
    WEBAUTH_TYPE_Host_Info_T host_info;
    BOOL_T first_host = TRUE;

    CLI_LIB_GetUnitPortFromString(arg[1], &unit, &port);
    eth_status = verify_ethernet(unit, port, &l_port);
    if (eth_status != CLI_API_ETH_OK)
    {
       display_ethernet_msg(eth_status, unit, port);
       return CLI_NO_ERROR;
    }

    WEBAUTH_PMGR_GetStatusByLPort(l_port, &port_status);
    if (VAL_webAuthPortConfigStatus_enabled == port_status)
    {
        PROCESS_MORE("Web Auth Status: Enabled\r\n");
    }
    else
    {
        PROCESS_MORE("Web Auth Status: Disabled\r\n");
    }

    WEBAUTH_PMGR_GetSystemSessionTimeout(&session_timeout);

    current_time = (SYSFUN_GetSysTick() / 100);
    index = 0;

    while (WEBAUTH_PMGR_GetNextSuccessHostByLPort(&host_info, l_port, &index) == WEBAUTH_TYPE_RETURN_OK)
    {
        if (TRUE == first_host)
        {
            PROCESS_MORE("\r\n");
            PROCESS_MORE("Host Summary\r\n");
            PROCESS_MORE("\r\n");

            PROCESS_MORE("IP address      Web-Auth-State Remaining-Session-Time\r\n");
            PROCESS_MORE("--------------- -------------- ----------------------\r\n");

            first_host = FALSE;
        }

        L_INET_Ntoa(host_info.ip, temp);
        sprintf(buff, "%-15s %-14s %-18d  \r\n", temp, "Authenticated", host_info.remaining_time);
        PROCESS_MORE(buff);
    }
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_WebAuth_Summary(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_WEBAUTH == TRUE)
    BOOL_T is_inherit = TRUE;
    UI32_T i;
    UI32_T j;
    UI32_T line_num = 0;
    UI32_T l_port = 0;
    UI8_T global_status;
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};

    PROCESS_MORE("Global Web-Auth Parameters:\r\n");

    WEBAUTH_PMGR_GetSystemStatus(&global_status);
    if (VAL_webauthEnable_enabled == global_status)
    {
    	PROCESS_MORE(" System Auth Control          :  Enabled\r\n");
    }
    else
    {
    	PROCESS_MORE(" System Auth Control          :  Disabled\r\n");
    }

    PROCESS_MORE("Port       Status         Authenticated Host Count\r\n");
    PROCESS_MORE("----       ------         ------------------------\r\n");

    for (i = 0; STKTPLG_POM_GetNextUnit(&i); )
    {
        UI32_T max_port_num;

        max_port_num = SWCTRL_POM_UIGetUnitPortNumber(i);
        for (j = 1; j <= max_port_num; j++)
        {
            UI16_T success_count;
            UI8_T port_status;
            char port_status_ar[10] = {0};
            char eth_name[10] = {0};

            if (FALSE == SWCTRL_POM_UIUserPortExisting(i, j))
            {
                continue;
            }

            if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(i, j, &l_port, &is_inherit))
            {
                continue;
            }

            if (WEBAUTH_TYPE_RETURN_OK == WEBAUTH_PMGR_GetStatusByLPort(l_port, &port_status))
            {
                if(port_status == VAL_webAuthPortConfigStatus_enabled)
                {
                    strcpy(port_status_ar, "Enabled");
                }
                else
                {
                    strcpy(port_status_ar, "Disabled");
                }
            }

            WEBAUTH_PMGR_GetSuccessCountByLPort(l_port, &success_count);
            sprintf(eth_name, "%1lu/%2lu", (unsigned long)i, (unsigned long)j);
            sprintf(buff, "%-10s %-10s     %-18d  \r\n", eth_name, port_status_ar, success_count);
            PROCESS_MORE(buff);
        }
    }
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

    return CLI_NO_ERROR;
}
