/* -------------------------------------------------------------------------------------
 * FILE NAME:  CLI_API_MNGMNT_IP_FILTER.C
 * -------------------------------------------------------------------------------------
 * PURPOSE:This file is the action function of  command
 * NOTE:
 *
 *
 *
 * HISTORY:
 * Modifier         Date                Description
 * -------------------------------------------------------------------------------------
 * peggy            2-20-2003           First Created
 *
 * -------------------------------------------------------------------------------------
 * Copyright(C)                 Accton Technology Corp. 2002
 * -------------------------------------------------------------------------------------*/
/* INCLUDE FILE DECLARATIONS
 */

/*cli internal*/
#include "cli_api.h"
#include "mgmt_ip_flt.h"
#include "l_inet.h"
#include <stdio.h>

static UI32_T PRINT_Management_Ip_Filter(UI32_T mode, UI32_T line_num);
static UI32_T Show_Management_Ip_Filter_Entry(UI8_T *buff, MGMT_IP_FLT_IpFilter_T ip_filter_entry, UI32_T line_num);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_MANAGEMENT_AllClient
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "management all-client"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_MANAGEMENT_AllClient(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T  mode = 0;
    MGMT_IP_FLT_IpFilter_T ip_filter_entry;
    memset(&ip_filter_entry, 0, sizeof(MGMT_IP_FLT_IpFilter_T));

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                       arg[0],
                                                       (L_INET_Addr_T *)&ip_filter_entry.start_ipaddress,
                                                       sizeof(ip_filter_entry.start_ipaddress)))
    {
        CLI_LIB_PrintStr("Invalid IP address.\r\n");
        return CLI_ERR_INTERNAL;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_MANAGEMENT_ALLCLIENT:

           if (arg[1] == NULL)
           {
              ip_filter_entry.end_ipaddress = ip_filter_entry.start_ipaddress;
           }
           else
           {
                if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                                   arg[1],
                                                                   (L_INET_Addr_T *)&ip_filter_entry.end_ipaddress,
                                                                   sizeof(ip_filter_entry.end_ipaddress)))
                {
                    CLI_LIB_PrintStr("Invalid IP address.\r\n");
                    return CLI_ERR_INTERNAL;
                }
           }

            mode = MGMT_IP_FLT_WEB;
            if (MGMT_IP_FLT_SetIpFilter(mode, &ip_filter_entry) != MGMT_IP_FLT_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set HTTP-client\r\n");
#endif
            }

            mode = MGMT_IP_FLT_SNMP;
            if (MGMT_IP_FLT_SetIpFilter(mode, &ip_filter_entry) != MGMT_IP_FLT_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set SNMP-client\r\n");
#endif
            }

            mode = MGMT_IP_FLT_TELNET;
            if (MGMT_IP_FLT_SetIpFilter(mode, &ip_filter_entry) != MGMT_IP_FLT_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set TELNET-client\r\n");
#endif
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_MANAGEMENT_ALLCLIENT:

            mode = MGMT_IP_FLT_WEB;
            if (MGMT_IP_FLT_DeleteIpFilter(mode, &ip_filter_entry) != MGMT_IP_FLT_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to delete HTTP-client\r\n");
#endif
            }

            mode = MGMT_IP_FLT_SNMP;
            if (MGMT_IP_FLT_DeleteIpFilter(mode, &ip_filter_entry) != MGMT_IP_FLT_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to delete SNMP-client\r\n");
#endif
            }

            mode = MGMT_IP_FLT_TELNET;
            if (MGMT_IP_FLT_DeleteIpFilter(mode, &ip_filter_entry) != MGMT_IP_FLT_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to delete TELNET-client\r\n");
#endif
            }
            break;

       default:
          return CLI_ERR_INTERNAL;
   }
   return CLI_NO_ERROR;
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_MANAGEMENT_HttpClient
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "management http-client"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_MANAGEMENT_HttpClient(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T  mode = 0;
    MGMT_IP_FLT_IpFilter_T ip_filter_entry;
    memset(&ip_filter_entry, 0, sizeof(MGMT_IP_FLT_IpFilter_T));
    mode = MGMT_IP_FLT_WEB;

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                       arg[0],
                                                       (L_INET_Addr_T *)&ip_filter_entry.start_ipaddress,
                                                       sizeof(ip_filter_entry.start_ipaddress)))
    {
        CLI_LIB_PrintStr("Invalid IP address.\r\n");
        return CLI_ERR_INTERNAL;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_MANAGEMENT_HTTPCLIENT:

           if (arg[1] == NULL)
           {
              ip_filter_entry.end_ipaddress = ip_filter_entry.start_ipaddress;
           }
           else
           {
                if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                                   arg[1],
                                                                   (L_INET_Addr_T *)&ip_filter_entry.end_ipaddress,
                                                                   sizeof(ip_filter_entry.end_ipaddress)))
                {
                    CLI_LIB_PrintStr("Invalid IP address.\r\n");
                    return CLI_ERR_INTERNAL;
                }
           }

            if (MGMT_IP_FLT_SetIpFilter(mode, &ip_filter_entry) != MGMT_IP_FLT_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set HTTP-client\r\n");
#endif
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_MANAGEMENT_HTTPCLIENT:
            if (MGMT_IP_FLT_DeleteIpFilter(mode, &ip_filter_entry) != MGMT_IP_FLT_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to delete HTTP-client\r\n");
#endif
            }
            break;
        default:
            return CLI_ERR_INTERNAL;
    }
    return CLI_NO_ERROR;
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_MANAGEMENT_SnmpClient
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "management snmpclient"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_MANAGEMENT_SnmpClient(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T  mode= 0;
    MGMT_IP_FLT_IpFilter_T ip_filter_entry;
    memset(&ip_filter_entry, 0, sizeof(MGMT_IP_FLT_IpFilter_T));
    mode = MGMT_IP_FLT_SNMP;

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                       arg[0],
                                                       (L_INET_Addr_T *)&ip_filter_entry.start_ipaddress,
                                                       sizeof(ip_filter_entry.start_ipaddress)))
    {
        CLI_LIB_PrintStr("Invalid IP address.\r\n");
        return CLI_ERR_INTERNAL;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_MANAGEMENT_SNMPCLIENT:

            if(arg[1]!= NULL)
            {
                if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                                   arg[1],
                                                                   (L_INET_Addr_T *)&ip_filter_entry.end_ipaddress,
                                                                   sizeof(ip_filter_entry.end_ipaddress)))
                {
                    CLI_LIB_PrintStr("Invalid IP address.\r\n");
                    return CLI_ERR_INTERNAL;
                }
            }
            else
            {
                ip_filter_entry.end_ipaddress = ip_filter_entry.start_ipaddress;
            }

            if(MGMT_IP_FLT_SetIpFilter(mode, &ip_filter_entry) != MGMT_IP_FLT_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set SNMP-client\r\n");
#endif
            }
            break;
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_MANAGEMENT_SNMPCLIENT:
            if(MGMT_IP_FLT_DeleteIpFilter(mode, &ip_filter_entry) != MGMT_IP_FLT_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to delete SNMP-client\r\n");
#endif
            }
            break;
        default:
            return CLI_ERR_INTERNAL;
    }
    return CLI_NO_ERROR;
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_MANAGEMENT_TelnetClient
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "management telnet-client"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_MANAGEMENT_TelnetClient(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TELNET == TRUE)
    UI32_T  mode= 0;
    MGMT_IP_FLT_IpFilter_T ip_filter_entry;
    memset(&ip_filter_entry, 0, sizeof(MGMT_IP_FLT_IpFilter_T));
    mode = MGMT_IP_FLT_TELNET;

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                       arg[0],
                                                       (L_INET_Addr_T *)&ip_filter_entry.start_ipaddress,
                                                       sizeof(ip_filter_entry.start_ipaddress)))
    {
        CLI_LIB_PrintStr("Invalid IP address.\r\n");
        return CLI_ERR_INTERNAL;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_MANAGEMENT_TELNETCLIENT:

            if(arg[1]!=NULL)
            {
                if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                                   arg[1],
                                                                   (L_INET_Addr_T *)&ip_filter_entry.end_ipaddress,
                                                                   sizeof(ip_filter_entry.end_ipaddress)))
                {
                    CLI_LIB_PrintStr("Invalid IP address.\r\n");
                    return CLI_ERR_INTERNAL;
                }
            }
            else
            {
                ip_filter_entry.end_ipaddress = ip_filter_entry.start_ipaddress;
            }

            if(MGMT_IP_FLT_SetIpFilter(mode, &ip_filter_entry) != MGMT_IP_FLT_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set TELNET-client\r\n");
#endif
            }
            break;
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_MANAGEMENT_TELNETCLIENT:
            if(MGMT_IP_FLT_DeleteIpFilter(mode, &ip_filter_entry) != MGMT_IP_FLT_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to delete TELNET-client\r\n");
#endif
            }
            break;
    }
#endif  /*#if (SYS_CPNT_TELNET == TRUE)*/
    return CLI_NO_ERROR;
}

/*=====================SHOW=MANAGEMENT========================*/

static UI32_T PRINT_Management_Ip_Filter(UI32_T mode, UI32_T line_num)
{
    UI8_T   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    MGMT_IP_FLT_IpFilter_T ip_filter_entry;
    memset(&ip_filter_entry, 0, sizeof(MGMT_IP_FLT_IpFilter_T));
    switch(mode)
    {    case MGMT_IP_FLT_WEB:
            PROCESS_MORE_FUNC(" HTTP Client:\r\n");
            break;
        case MGMT_IP_FLT_SNMP:
            PROCESS_MORE_FUNC(" SNMP Client:\r\n");
            break;
        case MGMT_IP_FLT_TELNET:
#if (SYS_CPNT_TELNET == TRUE)
            PROCESS_MORE_FUNC(" Telnet Client:\r\n");
            break;
#else
            return line_num;
#endif  /*#if (SYS_CPNT_TELNET == TRUE)*/

        default:
            break;
    }
    sprintf((char *)buff, "%-39s %s\r\n", "Start IP Address", "End IP Address");
    PROCESS_MORE_FUNC((char *)buff);
    PROCESS_MORE_FUNC("--------------------------------------- ---------------------------------------\r\n");

    switch(mode)
    {
        case MGMT_IP_FLT_WEB:
            while(MGMT_IP_FLT_WEB_GetNextIpFilter(&ip_filter_entry) == MGMT_IP_FLT_OK)
            {
                if ((line_num = Show_Management_Ip_Filter_Entry(buff, ip_filter_entry, line_num)) == JUMP_OUT_MORE)
                  return JUMP_OUT_MORE;
            }
            break;
        case MGMT_IP_FLT_SNMP:
            while(MGMT_IP_FLT_Snmp_GetNextIpFilter(&ip_filter_entry) == MGMT_IP_FLT_OK)
            {
                if ((line_num = Show_Management_Ip_Filter_Entry(buff, ip_filter_entry, line_num)) == JUMP_OUT_MORE)
                  return JUMP_OUT_MORE;
            }
            break;
        case MGMT_IP_FLT_TELNET:
            while(MGMT_IP_FLT_TELNET_GetNextIpFilter(&ip_filter_entry) == MGMT_IP_FLT_OK)
            {
                if ((line_num = Show_Management_Ip_Filter_Entry(buff, ip_filter_entry, line_num)) == JUMP_OUT_MORE)
                    return JUMP_OUT_MORE;
            }
            break;
        default:
            break;
    }
    return line_num;
}


static UI32_T Show_Management_Ip_Filter_Entry(UI8_T *buff, MGMT_IP_FLT_IpFilter_T ip_filter_entry, UI32_T line_num)
{
    char  ipadd_start_str[L_INET_MAX_IPADDR_STR_LEN+1] = {0};
    char  ipadd_end_str[L_INET_MAX_IPADDR_STR_LEN+1] = {0};

    if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&(ip_filter_entry.start_ipaddress),
                                                       ipadd_start_str,
                                                       sizeof(ipadd_start_str)))
    {
        return CLI_ERR_INTERNAL;
    }

    if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&(ip_filter_entry.end_ipaddress),
                                                       ipadd_end_str,
                                                       sizeof(ipadd_end_str)))
    {
        return CLI_ERR_INTERNAL;
    }

    sprintf((char *)buff, "%-39s %s\r\n", ipadd_start_str, ipadd_end_str);
    PROCESS_MORE_FUNC((char *)buff);

    return line_num;
}



/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Management_AllClient
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "show management all-client"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Show_Management_AllClient(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T  mode= 0;
    UI32_T  line_num = 0;
    UI8_T   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    PROCESS_MORE("Management IP Filter\r\n");

    for (mode = MGMT_IP_FLT_WEB; mode <= MGMT_IP_FLT_TELNET; mode++)
    {
       if((line_num = PRINT_Management_Ip_Filter(mode, line_num)) == JUMP_OUT_MORE)
       {
          return CLI_NO_ERROR;
       }
       else if (line_num == EXIT_SESSION_MORE)
       {
          return CLI_EXIT_SESSION;
       }
       PROCESS_MORE("\r\n");
    }
   return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Management_HttpClient
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "show management http-client"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Show_Management_HttpClient(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T  mode= 0;
    UI32_T  line_num = 0;
    UI8_T   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    mode = MGMT_IP_FLT_WEB;
    PROCESS_MORE("Management IP Filter\r\n");
    if((line_num = PRINT_Management_Ip_Filter(mode, line_num)) == JUMP_OUT_MORE)
    {
       return CLI_NO_ERROR;
    }
    else if (line_num == EXIT_SESSION_MORE)
    {
       return CLI_EXIT_SESSION;
    }
    return CLI_NO_ERROR;
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Management_SnmpClient
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "show management snmp-client"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Show_Management_SnmpClient(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T  mode= 0;
    UI32_T  line_num = 0;
    UI8_T   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    mode = MGMT_IP_FLT_SNMP;
    PROCESS_MORE("Management IP Filter\r\n");
    if((line_num = PRINT_Management_Ip_Filter(mode, line_num)) == JUMP_OUT_MORE)
    {
       return CLI_NO_ERROR;
    }
    else if (line_num == EXIT_SESSION_MORE)
    {
       return CLI_EXIT_SESSION;
    }
    return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Management_TelnetClient
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "show management telnet-client"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Show_Management_TelnetClient(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T  mode= 0;
    UI32_T  line_num = 0;
    UI8_T   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    mode = MGMT_IP_FLT_TELNET;
    PROCESS_MORE("Management IP Filter\r\n");
    if((line_num = PRINT_Management_Ip_Filter(mode, line_num)) == JUMP_OUT_MORE)
    {
       return CLI_NO_ERROR;
    }
    else if (line_num == EXIT_SESSION_MORE)
    {
       return CLI_EXIT_SESSION;
    }
    return CLI_NO_ERROR;
}


