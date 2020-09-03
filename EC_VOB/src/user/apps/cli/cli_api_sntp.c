#include <string.h>
#include <stdlib.h>
#include <sys_type.h>
#include <time.h>
#include <stdio.h>
#include "l_inet.h"
#include "sysfun.h"
#include "sys_dflt.h"
#include "cli_type.h"
#include "clis.h"
#include "cli_def.h"
#include "cli_mgr.h"
#include "cli_msg.h"
#include "cli_io.h"
#include "cli_task.h"
#include "cli_lib.h"
#include "cli_pars.h"
#include "cli_cmd.h"
#include "cli_def.h"
#include "cli_api.h"
#include "sys_adpt.h"
#include "sntp_pmgr.h"
#include "cli_api_sntp.h"
#include "sys_time.h"

#if defined(FTTH_OKI)
#include "sys_cpnt.h"
#endif


/* command: sntp client */
UI32_T CLI_API_Sntp_Client(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_SNTP_CLIENT:

        if(SNTP_PMGR_SetStatus(VAL_sntpStatus_enabled)!=TRUE)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to enable SNTP client.\r\n");
#endif
            return CLI_NO_ERROR;
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SNTP_CLIENT:

        if(SNTP_PMGR_SetStatus(VAL_sntpStatus_disabled)!=TRUE)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to disable SNTP client.\r\n");
#endif
             return CLI_NO_ERROR;
        }
        break;
    }

    return CLI_NO_ERROR;
}


/* command: sntp broadcast client */
UI32_T CLI_API_Sntp_Broadcast_Client(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W3_SNTP_BROADCAST_CLIENT:

        if(SNTP_PMGR_SetServiceOperationMode(BROCAST_MODE)!=TRUE)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set SNTP broadcast client.\r\n");
#endif
            return CLI_NO_ERROR;
        }
        break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_SNTP_BROADCAST_CLIENT:

        if(SNTP_PMGR_SetServiceOperationMode(SNTP_DEFAULT_OPERATIONMODE)!=TRUE)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to turn off the SNTP broadcast client and go to default mode.\r\n");
#endif
            return CLI_NO_ERROR;
        }
        break;
    }
#endif
    return CLI_NO_ERROR;
}


/* command: sntp poll poll_interval */
UI32_T CLI_API_Sntp_Poll(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_SNTP_POLL:

        if(SNTP_PMGR_SetPollTime(atoi(arg[0]))!=TRUE)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set polling interval between successive messages.\r\n");
#endif
            return CLI_NO_ERROR;
        }
        break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SNTP_POLL:

        if(SNTP_PMGR_SetPollTime(SNTP_DEFAULT_POLLINGTIME)!=TRUE)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set polling interval to default value.\r\n");
#endif
            return CLI_NO_ERROR;
        }
        break;
    }

    return CLI_NO_ERROR;
}
/* command: sntp server [ip1 [ip2 [ip3]]] */
UI32_T CLI_API_Sntp_Server(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T i=0;
    L_INET_AddrIp_T ipAddress;

    switch(cmd_idx)
    {
       case PRIVILEGE_CFG_GLOBAL_CMD_W2_SNTP_SERVER:
       {
           for(i=0;i<MAX_sntpServerIndex;i++)
           {
               if(arg[i]!=0)
               {
                   memset(&ipAddress, 0, sizeof(ipAddress));

                    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                                       arg[i],
                                                                       (L_INET_Addr_T *)&ipAddress,
                                                                       sizeof(ipAddress)))
                    {
                        CLI_LIB_PrintStr("Invalid IP address.\r\n");
                        return CLI_ERR_INTERNAL;
                    }
                   
                   if(SNTP_PMGR_AddServerIpForCLI(&ipAddress) != TRUE)
                   {
#if (SYS_CPNT_EH == TRUE)
                       CLI_API_Show_Exception_Handeler_Msg();
#else
                       CLI_LIB_PrintStr_1("Failed to set SNTP server IP %s.\r\n",arg[i]);
#endif
                   }
               }
           }
       } // end case PRIVILEGE_CFG_GLOBAL_CMD_W2_SNTP_SERVER
       break;

       case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SNTP_SERVER:
       {
           /* remove all server IP */
           if (arg[0] ==0)
           {
               if(SNTP_PMGR_DeleteAllServerIp()!=TRUE)
               {
#if (SYS_CPNT_EH == TRUE)
                   CLI_API_Show_Exception_Handeler_Msg();
#else
                   CLI_LIB_PrintStr_1("Failed to remove SNTP server IP %s.\r\n",arg[i]);
#endif
               }
           }
           else /* remove assigned server IP */
           {
               for(i=0;i<MAX_sntpServerIndex;i++)
               {
                   if(arg[i]!=0)
                   {
                       memset((UI8_T*)&ipAddress, 0, sizeof(ipAddress));

                        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                                           arg[i],
                                                                           (L_INET_Addr_T *)&ipAddress,
                                                                           sizeof(ipAddress)))
                        {
                            CLI_LIB_PrintStr("Invalid IP address.\r\n");
                            return CLI_ERR_INTERNAL;
                        }
                       
                       if(SNTP_PMGR_DeleteServerIpForCLI(&ipAddress)!=TRUE)
                       {
#if (SYS_CPNT_EH == TRUE)
                           CLI_API_Show_Exception_Handeler_Msg();
#else
                           CLI_LIB_PrintStr_1("Failed to remove SNTP server IP %s.\r\n",arg[i]);
#endif
                           break;
                       }
                   }
               }
           }

       }  // end case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SNTP_SERVER
       break;

       default:
       break;
   } //end switch

   return CLI_NO_ERROR;
}

/* command: show sntp */
UI32_T CLI_API_Show_Sntp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T  time=0, polltime=0, mode=0;
    UI32_T  sntp_status;
    L_INET_AddrIp_T curr_server_ip;
    char    date_time [SIZE_sysCurrentTime + 1];

    SYS_TIME_GetRealTimeBySec(&time);
    SYS_TIME_ConvertTime(time, date_time);
    CLI_LIB_PrintStr_1("Current Time   : %s\r\n",date_time);

    if(SNTP_PMGR_GetPollTime(&polltime)==TRUE)
    {
        CLI_LIB_PrintStr_1("Poll Interval  : %lu seconds\r\n",(unsigned long)polltime);
    }

    if(SNTP_PMGR_GetServiceOperationMode(&mode)==TRUE)
    {
        if(mode==VAL_sntpServiceMode_unicast)
        {
            CLI_LIB_PrintStr("Current Mode   : Unicast\r\n");
        }
        else if(mode==VAL_sntpServiceMode_broadcast)
        {
            CLI_LIB_PrintStr("Current Mode   : Broadcast\r\n");
        }
        else if(mode==VAL_sntpServiceMode_anycast)
        {
            CLI_LIB_PrintStr("Current Mode   : Anycast\r\n");
        }
    }

    /*SNTP status*/
    SNTP_PMGR_GetStatus(&sntp_status);
    if (sntp_status==VAL_sntpStatus_enabled)
    {
    	CLI_LIB_PrintStr("SNTP Status    : Enabled\r\n");
    }
    else if (sntp_status==VAL_sntpStatus_disabled)
    {
    	CLI_LIB_PrintStr("SNTP Status    : Disabled\r\n");
    }

    /*sntp server */
    {
        L_INET_AddrIp_T ip_address;
        UI32_T i =0;
        UI8_T  server_count = 0;
        char   ip_str1[L_INET_MAX_IPADDR_STR_LEN+1] = {0};
        char   ip_str2[L_INET_MAX_IPADDR_STR_LEN+1] = {0};
        char   ip_str3[L_INET_MAX_IPADDR_STR_LEN+1] = {0};

        /*get ip*/
        for (i = 1; i<=MAX_sntpServerIndex; i ++)
        {
            memset(&ip_address, 0, sizeof(L_INET_AddrIp_T));
            
            if (SNTP_PMGR_GetServerIp(i,&ip_address))
            {
                server_count++;
                switch(i)
                {
                case 1:
                    if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&ip_address,
                                                                       ip_str1,
                                                                       sizeof(ip_str1)))
                    {
                        return CLI_ERR_INTERNAL;
                    }
                    break;

                case 2:
                    if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&ip_address,
                                                                       ip_str2,
                                                                       sizeof(ip_str2)))
                    {
                        return CLI_ERR_INTERNAL;
                    }
                    break;

                case 3:
                    if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&ip_address,
                                                                       ip_str3,
                                                                       sizeof(ip_str3)))
                    {
                        return CLI_ERR_INTERNAL;
                    }
                    break;

                default:
                    break;
                }
            }
        }

        /*translate*/
        switch(server_count)
        {
        case 1:
            CLI_LIB_PrintStr_1("SNTP Server    : %s\r\n", ip_str1);
            break;

        case 2:
            CLI_LIB_PrintStr_1("SNTP Server    : %s\r\n", ip_str1);
            CLI_LIB_PrintStr_1("                 %s\r\n", ip_str2);
            break;

        case 3:
            CLI_LIB_PrintStr_1("SNTP Server    : %s\r\n", ip_str1);
            CLI_LIB_PrintStr_1("                 %s\r\n", ip_str2);
            CLI_LIB_PrintStr_1("                 %s\r\n", ip_str3);
            break;

        default:
            break;
        }
    }
    memset(&curr_server_ip, 0, sizeof(curr_server_ip));

    if(SNTP_PMGR_GetCurrentServer(&curr_server_ip)==TRUE)
    {
        char  ip_str[L_INET_MAX_IPADDR_STR_LEN+1] = {0};

        if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&curr_server_ip,
                                                           ip_str,
                                                           sizeof(ip_str)))
        {
            return CLI_ERR_INTERNAL;
        }

        CLI_LIB_PrintStr_1("Current Server : %s\r\n",ip_str);
    }

    return CLI_NO_ERROR;
}
