#include "cli_api.h"
#include "cli_api_syslog.h"
#include "syslog_pom.h"
#include <stdio.h>

#define LOGGING_TRAP_DEFAULT_LEVEL 7

static UI32_T 
show_logging_flash(
    char **logging_level, 
    UI32_T line_number
);

static UI32_T 
show_logging_ram(
    char **logging_level, 
    UI32_T line_number
);

static UI32_T 
show_logging_trap(
    char **logging_level, 
    UI32_T line_num
);

UI32_T 
CLI_API_Show_Logging(
    UI16_T cmd_idx, char *arg[], 
    CLI_TASK_WorkingArea_T *ctrl_P)
{
   UI32_T  line_num = 0;
   UI32_T syslog_status;
   char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
   char  *logging_level[] = {  "Emergencies",
                                "Alerts",
                                "Critical",
                                "Errors",
                                "Warnings",
                                "Notifications",
                                "Informational",
                                "Debugging"
                            };
   /* show setting
    */

   line_num += 1;
   if(arg[0]!= NULL)
   {
       PROCESS_MORE("Global Configuration: \r\n");

       if(arg[0][0] == 'r' || arg[0][0] == 'R')/*RAM*/
       {
           if(SYSLOG_POM_GetSyslogStatus(&syslog_status))
           {
                SYSFUN_Sprintf(buff, "  Syslog Logging           : %s\r\n", 
                               syslog_status == SYSLOG_STATUS_ENABLE ? "Enabled" : "Disabled");
                PROCESS_MORE(buff);
           }
           else
           {
#if (SYS_CPNT_EH == TRUE)
              CLI_API_Show_Exception_Handeler_Msg();
#else
              CLI_LIB_PrintStr("Failed to get syslog ststus\r\n");
#endif
           }

           show_logging_ram(logging_level, line_num);
       }
       else if(arg[0][0] == 't' || arg[0][0] == 'T')/*TRAP*/
       {
           if(SYSLOG_POM_GetSyslogStatus(&syslog_status))
           {
               SYSFUN_Sprintf(buff, "  Syslog Logging    : %s\r\n", 
                              syslog_status == SYSLOG_STATUS_ENABLE ? "Enabled" : "Disabled");
               PROCESS_MORE(buff);
           }
           else
           {
#if (SYS_CPNT_EH == TRUE)
               CLI_API_Show_Exception_Handeler_Msg();
#else
               CLI_LIB_PrintStr("Failed to get syslog ststus\r\n");
#endif
           }

           show_logging_trap(logging_level, line_num);
       }
       else if(arg[0][0] == 'f' || arg[0][0] == 'F')/*FLASH*/
       {
           if(SYSLOG_POM_GetSyslogStatus(&syslog_status))
           {
                SYSFUN_Sprintf(buff, "  Syslog Logging           : %s\r\n", 
                               syslog_status == SYSLOG_STATUS_ENABLE ? "Enabled" : "Disabled");
                PROCESS_MORE(buff);
           }
           else
           {
#if (SYS_CPNT_EH == TRUE)
              CLI_API_Show_Exception_Handeler_Msg();
#else
              CLI_LIB_PrintStr("Failed to get syslog ststus\r\n");
#endif
           }

           show_logging_flash(logging_level, line_num);
       }

    }

    return CLI_NO_ERROR;
}

static UI32_T 
show_logging_flash(
    char  **logging_level,
    UI32_T line_number)
{
   UI32_T flash_log_level;
   UI32_T line_num = line_number;

   /* setting
    */
   if( SYSLOG_PMGR_GetFlashLogLevel(&flash_log_level))
   {
       CLI_LIB_PrintStr("Flash Logging Configuration: \r\n");
       CLI_LIB_PrintStr_2("History Logging in Flash     : Level %s (%lu)\r\n", 
                          logging_level[flash_log_level], (unsigned long)flash_log_level);
   }
   else
   {
#if (SYS_CPNT_EH == TRUE)
       CLI_API_Show_Exception_Handeler_Msg();
#else
       CLI_LIB_PrintStr("Failed to get syslog history level in FLASH\r\n");
#endif
   }

   line_num += 1;

   return CLI_NO_ERROR;
}

static UI32_T 
show_logging_ram(
    char **logging_level, 
    UI32_T line_number)
{
   UI32_T uc_log_level;
   UI32_T line_num = line_number;

   /* setting
    */
   if( SYSLOG_POM_GetUcLogLevel(&uc_log_level))
   {
       CLI_LIB_PrintStr("Ram Logging Configuration: \r\n");
       CLI_LIB_PrintStr_2("  History Logging in RAM : Level %s (%lu)\r\n", 
                          logging_level[uc_log_level], (unsigned long)uc_log_level);
   }
   else
   {
#if (SYS_CPNT_EH == TRUE)
       CLI_API_Show_Exception_Handeler_Msg();
#else
       CLI_LIB_PrintStr("Failed to get syslog history level in RAM\r\n");
#endif
   }

   line_num += 1;

   return CLI_NO_ERROR;
}

static UI32_T 
show_logging_trap(
    char **logging_level, 
    UI32_T line_num)
{
#if (SYS_CPNT_REMOTELOG == TRUE)
   UI32_T status=SYSLOG_STATUS_DISABLE;
   char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
   SYSLOG_MGR_Remote_Server_Config_T server_config;

   memset(&server_config,0,sizeof(SYSLOG_MGR_Remote_Server_Config_T));

/* fuzhimin,20090417
 */
#if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)

    if(SYSLOG_PMGR_GetRemoteLogStatus(&status) == SYSLOG_REMOTE_SUCCESS)
    {
        PROCESS_MORE_FUNC("Remote Logging Configuration:\n");
        SYSFUN_Sprintf(buff, "  Status            : %s\r\n",
                       (status == SYSLOG_STATUS_ENABLE)?("Enabled"):("Disabled"));
        PROCESS_MORE_FUNC(buff);
    }
    {
        int i;
        char server_facility_str[SYSLOG_TYPE_MAX_FACILITY_STR_LEN];
        char server_level_str[SYSLOG_TYPE_MAX_LEVEL_STR_LEN];

        memset(server_facility_str, 0, sizeof(server_facility_str));
        memset(server_level_str, 0, sizeof(server_level_str));

        for(i = 0; SYSLOG_PMGR_GetNextRemoteLogServer(&server_config) == SYSLOG_REMOTE_SUCCESS; i ++)
        {
            char ip_str[L_INET_MAX_IPADDR_STR_LEN+1] = "";

            if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&(server_config.ipaddr),
                                                               ip_str,
                                                               sizeof(ip_str)))
            {
                continue;
            }

            SYSFUN_Sprintf(buff, "Remote Host %d      :\r\n",i+1);
            PROCESS_MORE_FUNC(buff);

            SYSLOG_MGR_FACILITY_TO_STR(server_config.facility,server_facility_str);
            SYSFUN_Sprintf(buff, "  Facility Type     : %s (%lu)\r\n",
                           server_facility_str, (unsigned long)server_config.facility);
            PROCESS_MORE_FUNC(buff);

            SYSLOG_MGR_LEVEL_TO_STR(server_config.level,server_level_str);
            SYSFUN_Sprintf(buff, "  Level Type        : %s (%lu)\r\n",
                           server_level_str, (unsigned long)server_config.level);
            PROCESS_MORE_FUNC(buff);

            SYSFUN_Sprintf(buff, "  Server IP Address : %s\r\n",(char*)(ip_str));
            PROCESS_MORE_FUNC(buff);

            SYSFUN_Sprintf(buff, "  Port              : %lu\r\n",(unsigned long)server_config.udp_port);
            PROCESS_MORE_FUNC(buff);
        }
   }
#else
/* fuzhimin,20090417,end
 */
   UI32_T facility=0,level=0;
    char facility_str[SYSLOG_TYPE_MAX_FACILITY_STR_LEN];
    char level_str[SYSLOG_TYPE_MAX_LEVEL_STR_LEN];

   if(SYSLOG_PMGR_GetRemoteLogStatus(&status)==SYSLOG_REMOTE_SUCCESS)
   {
      PROCESS_MORE_FUNC("Remote Logging Configuration: \r\n");
      SYSFUN_Sprintf(buff, "  Status              : %s\r\n",
                     (status == SYSLOG_STATUS_ENABLE)?("Enabled"):("Disabled"));
      PROCESS_MORE_FUNC(buff);
   }

   if(SYSLOG_PMGR_GetRemoteLogFacility(&facility)==SYSLOG_REMOTE_SUCCESS)
   {
      memset(facility_str, 0, sizeof(facility_str));
      SYSLOG_MGR_FACILITY_TO_STR(facility,facility_str);
      SYSFUN_Sprintf(buff, "  Facility Type       : %s (%lu)\r\n",
                     facility_str, (unsigned long)facility);
      PROCESS_MORE_FUNC(buff);

   }

   if(SYSLOG_PMGR_GetRemoteLogLevel(&level)==SYSLOG_REMOTE_SUCCESS)
   {
       memset(level_str, 0, sizeof(level_str));
       SYSLOG_MGR_LEVEL_TO_STR(level,level_str);
       SYSFUN_Sprintf(buff, "  Level Type          : %s (%lu)\r\n",
                      level_str, (unsigned long)level);
       PROCESS_MORE_FUNC(buff);
   }

   {
        int i;

        for(i = 0; SYSLOG_PMGR_GetNextRemoteLogServer(&server_config) == SYSLOG_REMOTE_SUCCESS; i ++)
        {
            char ip_str[L_INET_MAX_IPADDR_STR_LEN+1] = "";

            if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&(server_config.ipaddr),
                                                               ip_str,
                                                               sizeof(ip_str)))
            {
                continue;
            }

            SYSFUN_Sprintf(buff, "  Remote Host %d       :\r\n",i+1);
            PROCESS_MORE_FUNC(buff);
            SYSFUN_Sprintf(buff, "    Server IP Address : %s\r\n",ip_str);
            PROCESS_MORE_FUNC(buff);
            SYSFUN_Sprintf(buff, "    Port              : %lu\r\n",
                           (unsigned long)server_config.udp_port);
            PROCESS_MORE_FUNC(buff);
        }
   }
#endif
#endif

   return line_num;
}

UI32_T CLI_API_Clear_Logging(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   if(arg[0] == NULL)
   {
      /*ram*/
        if (!SYSLOG_PMGR_ClearAllRamEntries())
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to clear all log entry in RAM.\r\n");
#endif
      }

      /*flash*/
      if(!SYSLOG_PMGR_ClearAllFlashEntries())
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to clear all log entry in FLASH.\r\n");
#endif
      }
   }
   else
   {
      switch(arg[0][0])
      {
      case 'r':/*ram*/
      case 'R':
            if (!SYSLOG_PMGR_ClearAllRamEntries())
         {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to clear all log entry in RAM.\r\n");
#endif
         }
         break;

      case 'f':/*flash*/
      case 'F':
         if(!SYSLOG_PMGR_ClearAllFlashEntries())
         {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to clear all log entry in FLASH.\r\n");
#endif
         }
         break;

      default:
         return CLI_ERR_INTERNAL;
      }
   }

   return CLI_NO_ERROR;
}



UI32_T CLI_API_Logging_On(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   /*Enable/Disable message logging*/
   switch(cmd_idx)
   {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_LOGGING_ON:
       if(!SYSLOG_PMGR_SetSyslogStatus(SYSLOG_STATUS_ENABLE))
       {
#if (SYS_CPNT_EH == TRUE)
          CLI_API_Show_Exception_Handeler_Msg();
#else
          CLI_LIB_PrintStr("Failed to enable syslog\r\n");
#endif
       }
       break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_LOGGING_ON:
       if(!SYSLOG_PMGR_SetSyslogStatus(SYSLOG_STATUS_DISABLE))
       {
#if (SYS_CPNT_EH == TRUE)
          CLI_API_Show_Exception_Handeler_Msg();
#else
          CLI_LIB_PrintStr("Failed to disable syslog\r\n");
#endif
       }
       break;

    default:
       break;
   }
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Logging(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   /*Log messages to a UNIX syslog server host.*/
   USED_TO_PRT_ARGS;
   CLI_LIB_PrintStr("<TBD>\r\n");
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Logging_Console(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   /*Limit messages logged to the console.*/
   USED_TO_PRT_ARGS;
   CLI_LIB_PrintStr("<TBD>\r\n");
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Logging_Monitor(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   /*Limit messages logged to the terminal lines.*/
   USED_TO_PRT_ARGS;
   CLI_LIB_PrintStr("<TBD>\r\n");
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Logging_History(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#define TYPE_RAM   0
#define TYPE_FLASH 1

   /*Change the default level of syslog messages stored in the history file and sent to the SNMP server.*/
   UI32_T log_level;
   UI8_T  log_type = TYPE_RAM;
   UI32_T ret_val = SYSLOG_UNKNOWN_ERROR;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_LOGGING_HISTORY:
      log_level = atoi( arg[1] );
      if (arg[0][0] == 'f' || arg[0][0] == 'F')
      {
         log_type = TYPE_FLASH;
         ret_val = SYSLOG_PMGR_SetFlashLogLevel(log_level);
      }
      else if (arg[0][0] == 'r' || arg[0][0] == 'R')
      {
         log_type = TYPE_RAM;
         ret_val = SYSLOG_PMGR_SetUcLogLevel(log_level);
      }
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_LOGGING_HISTORY:
      if (arg[0][0] == 'f' || arg[0][0] == 'F')
      {
         log_type = TYPE_FLASH;
         ret_val = SYSLOG_PMGR_SetFlashLogLevel(SYS_DFLT_SYSLOG_FLASH_LOG_LEVEL);
      }
      else if (arg[0][0] == 'r' || arg[0][0] == 'R')
      {
         log_type = TYPE_RAM;
         ret_val = SYSLOG_PMGR_SetUcLogLevel(SYS_DFLT_SYSLOG_UC_LOG_LEVEL);
      }
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

#if (SYS_CPNT_EH == TRUE)
   if (ret_val != SYSLOG_RETURN_OK)
   {
      CLI_API_Show_Exception_Handeler_Msg();
   }
#else
   switch(ret_val)
   {
   case SYSLOG_NORMAL_LEVEL_SMALLER_THAN_FLASH_LEVEL:
      CLI_LIB_PrintStr_1("Failed to set %s logging history level, logging severity level of RAM should not be higher than FLASH\r\n", log_type == TYPE_RAM ? "RAM" : "FLASH");
      break;

   case SYSLOG_FLASH_LEVEL_BIGGER_THAN_NORMAL_LEVEL:
      CLI_LIB_PrintStr_1("Failed to set %s logging history level, logging severity level of FLASH should not be lower than RAM\r\n", log_type == TYPE_RAM ? "RAM" : "FLASH");
      break;

   case SYSLOG_LEVEL_VLAUE_INVALID:
      CLI_LIB_PrintStr_1("Failed to set %s logging history level, invalid level\r\n", log_type == TYPE_RAM ? "RAM" : "FLASH");
      break;

   case SYSLOG_RETURN_OK:
   default:
      break;
   }
#endif
   return CLI_NO_ERROR;
}

UI32_T 
CLI_API_Logging_Trap(
    UI16_T cmd_idx, 
    char *arg[], 
    CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_REMOTELOG == TRUE)
#if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_LOGGING_TRAP:

        if(SYSLOG_PMGR_SetRemoteLogStatus(SYSLOG_STATUS_ENABLE) != SYSLOG_REMOTE_SUCCESS)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to enable remote log.\r\n");
#endif
        }

        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_LOGGING_TRAP:

        if(SYSLOG_PMGR_SetRemoteLogStatus(SYSLOG_STATUS_DISABLE) != SYSLOG_REMOTE_SUCCESS)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to disable remote log.\r\n");
#endif
        }

        break;
    default:
        return CLI_NO_ERROR;
   }

#else

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_LOGGING_TRAP:

        if(SYSLOG_PMGR_SetRemoteLogStatus(SYSLOG_STATUS_ENABLE) != SYSLOG_REMOTE_SUCCESS)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to enable remote log.\r\n");
#endif
        }

        break;
    case PRIVILEGE_CFG_GLOBAL_CMD_W3_LOGGING_TRAP_LEVEL:

        if(SYSLOG_PMGR_SetRemoteLogLevel(atoi(arg[0])) != SYSLOG_REMOTE_SUCCESS)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set remote log level.\r\n");
#endif
        }

        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_LOGGING_TRAP:

        if(SYSLOG_PMGR_SetRemoteLogStatus(SYSLOG_STATUS_DISABLE) != SYSLOG_REMOTE_SUCCESS)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to disable remote log.\r\n");
#endif
        }

        break;
    case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_LOGGING_TRAP_LEVEL:

        if(SYSLOG_PMGR_SetRemoteLogLevel(LOGGING_TRAP_DEFAULT_LEVEL) != SYSLOG_REMOTE_SUCCESS)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set remote log level.\r\n");
#endif
         }

        break;
    default:

        return CLI_NO_ERROR;
   }
#endif /* #if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE) */
#endif /* #if (SYS_CPNT_REMOTELOG == TRUE) */

   return CLI_NO_ERROR;
}

UI32_T 
CLI_API_Logging_Host(
    UI16_T cmd_idx, 
    char *arg[], 
    CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_REMOTELOG == TRUE)
    UI32_T value;
    UI32_T result;
    UI32_T pos = 0;
    L_INET_AddrIp_T ip_address;

    memset(&ip_address, 0, sizeof(ip_address));

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_LOGGING_HOST:
            {
                if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC,
                                                                   arg[pos],
                                                                   (L_INET_Addr_T *)&ip_address,
                                                                   sizeof(ip_address)))
                {
                    CLI_LIB_PrintStr("Invalid IP address.\r\n");
                    return CLI_ERR_INTERNAL;
                }
                
                result = SYSLOG_PMGR_CreateRemoteLogServer(&ip_address);

                if(SYSLOG_REMOTE_SUCCESS != result)
                {
                    CLI_LIB_PrintStr("Failed to set remote log server IP address.\r\n");
                }

                pos = 1;

                for ( ; arg[pos]; pos ++)
                {
                    switch (arg[pos][0])
                    {
#if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)
                        case 'f': case 'F':
                            value = atoi(arg[++pos]);
                            result = SYSLOG_PMGR_SetRemoteLogServerFacility(&ip_address, value);

                            if(SYSLOG_REMOTE_SUCCESS != result)
                            {
                                CLI_LIB_PrintStr("Failed to set remote log server facility type.\r\n");
                            }

                            break;

                        case 'l': case 'L':
                            value = atoi(arg[++pos]);
                            result = SYSLOG_PMGR_SetRemoteLogServerLevel(&ip_address, value);

                            if(SYSLOG_REMOTE_SUCCESS != result)
                            {
                                CLI_LIB_PrintStr("Failed to set remote log server trap level.\r\n");
                            }

                            break;

#endif /*endif (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)*/

                        case 'p': case 'P':
                            value = atoi( arg[++pos] );
                            result = SYSLOG_PMGR_SetRemoteLogServerPort(&ip_address, value);

                            if(SYSLOG_REMOTE_SUCCESS != result)
                            {
                                CLI_LIB_PrintStr("Failed to set remote log server port.\r\n");
                            }

                            break;

                        default:

                            return CLI_ERR_INTERNAL;
                    }
                }

            }
            break;
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_LOGGING_HOST:
            {
                if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC,
                                                                   arg[0],
                                                                   (L_INET_Addr_T *)&ip_address,
                                                                   sizeof(ip_address)))
                {
                    CLI_LIB_PrintStr("Invalid IP address.\r\n");
                    return CLI_ERR_INTERNAL;
                }

                result = SYSLOG_PMGR_DeleteRemoteLogServer(&ip_address);

                if (arg[1] == NULL)
                {
                    if(SYSLOG_REMOTE_SUCCESS != result)
                    {
                        CLI_LIB_PrintStr("Failed to set remote log server IP address.\r\n");
                    }
                }
                else
                {
                    pos = 1;

                    for (; arg[pos]; pos++)
                    {
                        switch (arg[pos][0])
                        {
#if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)
                            case 'f': case 'F':
                                {
                                    value = SYS_DFLT_REMOTELOG_FACILITY_TYPE;
                                    result = SYSLOG_PMGR_SetRemoteLogServerFacility(&ip_address, value);

                                    if(SYSLOG_REMOTE_SUCCESS != result)
                                    {
                                        CLI_LIB_PrintStr("Failed to set remote log server IP address.\r\n");
                                    }
                                }
                                break;

                            case 'l': case 'L':
                                {
                                    value = SYS_DFLT_REMOTELOG_LEVEL;
                                    result = SYSLOG_PMGR_SetRemoteLogServerLevel(&ip_address, value);

                                    if(SYSLOG_REMOTE_SUCCESS != result)
                                    {
                                        CLI_LIB_PrintStr("Failed to set remote log server IP address.\r\n");
                                    }
                                }
                                break;
#endif /* #if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE) */
                            case 'p': case 'P':
                                {
                                    value = SYS_DFLT_SYSLOG_HOST_PORT;
                                    result = SYSLOG_PMGR_SetRemoteLogServerPort(&ip_address, value);

                                    if(SYSLOG_REMOTE_SUCCESS != result)
                                    {
                                        CLI_LIB_PrintStr("Failed to set remote log server IP address.\r\n");
                                    }
                                }
                                break;

                            default:

                                return CLI_ERR_INTERNAL;
                        }
                    }
                }

            }
            break;

            default:

                return CLI_ERR_INTERNAL;
        }

        return CLI_NO_ERROR;
#endif /* #if (SYS_CPNT_REMOTELOG == TRUE) */
}

UI32_T 
CLI_API_Logging_Facility(
    UI16_T cmd_idx, 
    char *arg[], 
    CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_REMOTELOG == TRUE)
#if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER != TRUE)
   switch(cmd_idx)
   {
       case PRIVILEGE_CFG_GLOBAL_CMD_W2_LOGGING_FACILITY:

          if(SYSLOG_PMGR_SetRemoteLogFacility(atoi(arg[0])) != SYSLOG_REMOTE_SUCCESS)
          {
#if (SYS_CPNT_EH == TRUE)
             CLI_API_Show_Exception_Handeler_Msg();
#else
             CLI_LIB_PrintStr("Failed to set remote log facility type.\r\n");
#endif
          }

          break;

       case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_LOGGING_FACILITY:

          if(SYSLOG_PMGR_SetRemoteLogFacility(SYSLOG_REMOTE_FACILITY_LOCAL7) != SYSLOG_REMOTE_SUCCESS)
          {
#if (SYS_CPNT_EH == TRUE)
             CLI_API_Show_Exception_Handeler_Msg();
#else
             CLI_LIB_PrintStr("Failed to set default remote log facility type.\r\n");
#endif
          }

          break;

       default:

          return CLI_NO_ERROR;
   }
#endif /* #if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER != TRUE) */
#endif /* #if (SYS_CPNT_REMOTELOG == TRUE) */

   return CLI_NO_ERROR;
}

