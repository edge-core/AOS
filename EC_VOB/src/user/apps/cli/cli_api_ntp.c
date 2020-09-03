#include "sys_cpnt.h"

#if (SYS_CPNT_NTP == TRUE)
#include <string.h>
#include "sys_type.h"
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
#include "cli_api.h"
#include "cli_def.h"
#include "sys_adpt.h"
#include "ntp_pmgr.h"
#include "ntp_mgr.h"
#include "cli_api_sntp.h"
#include "cli_api_ntp.h"
#include "sys_time.h"
#include "l_stdlib.h"

/* command:Enable or Disable ntp client  */
UI32_T CLI_API_Ntp_Client(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_NTP_CLIENT:

      if(NTP_PMGR_SetStatus(VAL_ntpStatus_enabled)!=TRUE)
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Fail to enable NTP client.\r\n");
#endif
         return CLI_NO_ERROR;
      }
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_NTP_CLIENT:

      if(NTP_PMGR_SetStatus(VAL_ntpStatus_disabled)!=TRUE)
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Fail to disable NTP client.\r\n");
#endif
         return CLI_NO_ERROR;
      }
      break;
   }
   return CLI_NO_ERROR;
}

/* command: configure ntp server ipaddress [version][key] */
UI32_T CLI_API_Ntp_Server(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   UI32_T ipAddress=0, key=0;

   switch(cmd_idx)
   {
       case PRIVILEGE_CFG_GLOBAL_CMD_W2_NTP_SERVER:
       {
           //server
           CLI_LIB_AtoIp(arg[0], (UI8_T*)&ipAddress);

           //key
           if (arg[2] !=0)
               key = atoi(arg[2]);
           else
               key = VAL_ntpServerKey_no;

           if(NTP_PMGR_AddServerIp(ipAddress, NTP_VERSION, key)!=TRUE)
           {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("Fail to set NTP server IP %s.\r\n",arg[0]);
#endif
                break;
           }


       } // end case PRIVILEGE_CFG_GLOBAL_CMD_W2_SNTP_SERVER
       break;

       case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_NTP_SERVER:
       {
           /* remove all server IP */
           if (arg[0] ==0)
           {
               if(NTP_PMGR_DeleteAllServerIp()!=TRUE)
               {
#if (SYS_CPNT_EH == TRUE)
                   CLI_API_Show_Exception_Handeler_Msg();
#else
                   CLI_LIB_PrintStr("Fail to remove all NTP servers .\r\n");
#endif
                   break;
               }
           }
           else /* remove assigned server IP */
           {

      	        CLI_LIB_AtoIp(arg[0], (UI8_T*)&ipAddress);
                if(NTP_PMGR_DeleteServerIp(ipAddress)!=TRUE)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Fail to remove NTP server IP %s.\r\n",arg[0]);
#endif
                    break;
                }

           }

       } // end case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SNTP_SERVER
       break;

       default:
       break;
   } //end switch
   return CLI_NO_ERROR;
}

/* command: show sntp */
UI32_T CLI_API_Show_Ntp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    NTP_MGR_SERVER_T lastServer;
    NTP_MGR_AUTHKEY_T *sk;
    I32_T   updatetime=0;
    UI32_T  time=0, polltime=0, mode=0;
    UI32_T  ntp_status;
    UI32_T  ntp_authstatus;
    UI32_T  ip = 0;
    UI32_T  version = 0;
    UI32_T  keyid = 0;
    char    date_time[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN + 1];
    char    Update_time[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN + 1];
    UI8_T   ip_str[16] = {0};

#if (SYS_CPNT_CLI_FILTERING == TRUE)
    if(arg[0]!=NULL)
    {
        if(arg[0][0]=='|')
        {
            switch(arg[1][0])
            {
                case 'b':
                case 'B':
                    ctrl_P->option_flag=CLI_API_OPTION_BEGIN;
                    break;
                case 'e':
                case 'E':
                    ctrl_P->option_flag=CLI_API_OPTION_EXCLUDE;
                    break;

                case 'i':
                case 'I':
                    ctrl_P->option_flag=CLI_API_OPTION_INCLUDE;
                    break;
                default:
                    return CLI_ERR_INTERNAL;
            }
            strcpy(ctrl_P->option_buf,arg[2]);
        }
    }
#endif /*#if (SYS_CPNT_CLI_FILTERING == TRUE)*/

   SYS_TIME_GetRealTimeBySec(&time);
   SYS_TIME_ConvertTime(time, date_time);
   CLI_LIB_PrintStr_1("Current Time             : %s\r\n",date_time);

   if(NTP_PMGR_GetPollTime(&polltime)==TRUE)
   {
      CLI_LIB_PrintStr_1("Polling                  : %lu seconds\r\n",(unsigned long)polltime);
   }

   if(NTP_PMGR_GetServiceOperationMode(&mode)==TRUE)
   {
      if(mode==VAL_ntpServiceMode_unicast)
      {
         CLI_LIB_PrintStr("Current Mode             : unicast\r\n");
      }
      else if(mode==VAL_ntpServiceMode_broadcast)
      {
         CLI_LIB_PrintStr("Current Mode             : broadcast\r\n");
      }
      else if(mode==VAL_ntpServiceMode_anycast)
      {
         CLI_LIB_PrintStr("Current Mode             : anycast\r\n");
      }
   }

   /*NTP status*/
    NTP_PMGR_GetStatus(&ntp_status);
    if (ntp_status==VAL_ntpStatus_enabled)
    {
    	CLI_LIB_PrintStr("NTP Status               : Enabled       \r\n");
    }
    else if (ntp_status==VAL_sntpStatus_disabled)
    {
    	CLI_LIB_PrintStr("NTP Status               : Disabled      \r\n");
    }
    if(NTP_PMGR_GetAuthStatus(&ntp_authstatus) == TRUE)
    {
        if (ntp_authstatus==VAL_ntpAuthenticateStatus_enabled)
        {
        	CLI_LIB_PrintStr("NTP Authenticate Status  : Enabled\r\n");
        }
        else if (ntp_authstatus==VAL_ntpAuthenticateStatus_disabled)
        {
        	CLI_LIB_PrintStr("NTP Authenticate Status  : Disabled\r\n");
        }
    }
    if(NTP_PMGR_GetLastUpdateServer(&lastServer)== TRUE)
   {
        /*Server and port*/
        UI8_T  ip_str[16] = {0};

        L_INET_Ntoa(lastServer.srcadr.sin_addr.s_addr, ip_str);

        CLI_LIB_PrintStr_2("Last Update NTP Server   : %s        Port: %d\r\n",ip_str,lastServer.srcadr.sin_port);
        if(NTP_PMGR_GetLastUpdateTime(&updatetime) == TRUE)
        {
            SYS_TIME_ConvertTime(updatetime, Update_time);
            CLI_LIB_PrintStr_1("Last Update Time         : %s UTC\r\n",Update_time);
        }
        /* Del  - QingfengZhang, 14 April, 2005 6:09:48 */
        /*stratum and precision */
        //CLI_LIB_PrintStr_2("Stratum  %d          Precision  %d          ",lastServer.stratum,lastServer.precision);

        /*transmitted number and in filter number */
        //CLI_LIB_PrintStr_2("Transmitted  %d          In filter  %d\r\n",lastServer.xmtcnt,lastServer.filter_nextpt);

        /*Offset */
        //CLI_LIB_PrintStr_1("Adjust time offset  %d seconds \r\n",lastServer.offset.Ul_i.Xl_ui);
        /* End - QingfengZhang, 14 April, 2005 6:10:07 */
    }

    /*show server*/
    while (NTP_PMGR_GetNextServer(&ip, &version, &keyid) == TRUE)
    {
        L_INET_Ntoa(ip, ip_str);

        if (keyid != 0)
        {
            CLI_LIB_PrintStr_3("NTP Server %s version %lu key %lu\r\n", ip_str, (unsigned long)version, (unsigned long)keyid);
        }
        else
        {
            CLI_LIB_PrintStr_2("NTP Server %s version %lu\r\n",ip_str, (unsigned long)version);
        }
    }

    /*show hash key*/
    sk = (NTP_MGR_AUTHKEY_T *)malloc(sizeof(NTP_MGR_AUTHKEY_T));

    if(NULL!=sk)
    {
        memset(sk,0,sizeof(NTP_MGR_AUTHKEY_T));

        while(NTP_PMGR_GetNextKey(sk) == TRUE)
        {
            CLI_LIB_PrintStr_2("NTP Authentication Key %ld md5 %s\r\n", (long)sk->keyid, sk->password);
        }
        free(sk);
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Ntp_Authenticate(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

    switch(cmd_idx)
       {
       case PRIVILEGE_CFG_GLOBAL_CMD_W2_NTP_AUTHENTICATE:
          if(NTP_PMGR_SetAuthStatus(VAL_ntpAuthenticateStatus_enabled)!=TRUE)
          {
#if (SYS_CPNT_EH == TRUE)
             CLI_API_Show_Exception_Handeler_Msg();
#else
             CLI_LIB_PrintStr("Fail to enable NTP authenticate.\r\n");
#endif
             return CLI_NO_ERROR;
          }
          break;

       case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_NTP_AUTHENTICATE:
          if(NTP_PMGR_SetAuthStatus(VAL_ntpAuthenticateStatus_disabled)!=TRUE)
          {
#if (SYS_CPNT_EH == TRUE)
             CLI_API_Show_Exception_Handeler_Msg();
#else
             CLI_LIB_PrintStr("Fail to disable NTP authenticate.\r\n");
#endif
             return CLI_NO_ERROR;
          }
          break;
       }
       return CLI_NO_ERROR;

}

UI32_T CLI_API_Ntp_Authenticationkey(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   UI32_T keynumber=0;


   switch(cmd_idx)
   {
       case PRIVILEGE_CFG_GLOBAL_CMD_W2_NTP_AUTHENTICATIONKEY:
       {
           //keynumber
           keynumber=atoi(arg[0]);

           if(ctrl_P->sess_type == CLI_TYPE_PROVISION)
           {
               if(NTP_PMGR_AddAuthKey_Encrypted(keynumber, arg[2])!=TRUE)
               {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Fail to add Authentication key %ld.\r\n",(long)keynumber);
#endif
               }
           }
           else
           {
                if(NTP_PMGR_AddAuthKey(keynumber, arg[2])!=TRUE)
               {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Fail to add Authentication key %ld.\r\n",(long)keynumber);
#endif
               }
           }
       } // end case PRIVILEGE_CFG_GLOBAL_CMD_W2_SNTP_SERVER
       break;

       case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_NTP_AUTHENTICATIONKEY:
       {
           /* remove all authentication key */
           if (arg[0] ==0)
           {
               if(NTP_PMGR_DeleteAllAuthKey()!=TRUE)
               {
#if (SYS_CPNT_EH == TRUE)
                   CLI_API_Show_Exception_Handeler_Msg();
#else
                   CLI_LIB_PrintStr("Fail to remove authenticaion keys.\r\n");
#endif
               }
           }
           else /* remove assigned authentication key */
           {

      	        keynumber=atoi(arg[0]);
                if(NTP_PMGR_DeleteAuthKey(keynumber)!=TRUE)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Fail to remove NTP Authticaion key %ld.\r\n",(long)keynumber);
#endif
                    break;
                }

           }

       }  // end case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SNTP_SERVER
       break;

       default:
       break;
   } //end switch
   return CLI_NO_ERROR;
}

#endif

