#include <stdio.h>
#include "sys_bld.h"
#include "sys_cpnt.h"
#include "cli_api.h"
#include "cli_api_snmp.h"
#include "leaf_2863.h"
#include "leaf_3411.h"
#include "snmp_pmgr.h"
#include "snmp_mgr.h"
#include "l_inet.h"
#include "mib2_pom.h"
#include "swctrl_pom.h"
#include "sys_time.h"

#if (SYS_CPNT_CFM == TRUE)
#include "cfm_pmgr.h"
#endif /* #if (SYS_CPNT_CFM == TRUE) */
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
#include "amtr_om.h"
#endif

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T str_to_nonprintable_Length(char *str, UI8_T *om,UI32_T max_len);
static char *entry_status_str(UI32_T status);
static void convert_timeticks_to_date_string(UI32_T org_seconds, char *output_str_p);
static BOOL_T get_port_info(CLI_TASK_WorkingArea_T *ctrl_p, UI32_T *port_number_p, UI32_T *first_if_index_p);
static BOOL_T is_oid_valid(char *oid_p);

static UI32_T
get_snmpv3_privtype(
    char *str_p,
    UI32_T str_len
    );

static UI32_T
get_snmpv3_privkeylen(
    UI32_T priv_type,
    UI32_T ori_len
    );
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
static UI32_T Show_MacNtfyTrapInfo_Title(
    CLI_TASK_WorkingArea_T *ctrl_P,
    UI32_T line_num);

static UI32_T Show_MacNtfyTrapInfo_One(
    CLI_TASK_WorkingArea_T *ctrl_P,
    UI32_T  lport,
    UI32_T  line_num);
#endif /* #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) */
/* EXPORTED SUBPROGRAM BODIES
 */
UI32_T CLI_API_SNMP_IP_Filter(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IP_FILTER == TRUE)
#if !defined(MERCURY_DC)
   USERAUTH_SnmpIpFilter_T ip_filter_entry;

   L_INET_Aton(arg[0], &(ip_filter_entry.ip_filter_ipaddress));
   L_INET_Aton(arg[1], &(ip_filter_entry.ip_filter_netmask));

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W3_SNMP_IP_FILTER:
      ip_filter_entry.status = USERAUTH_ENTRY_VALID;
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_SNMP_IP_FILTER:
      ip_filter_entry.status = USERAUTH_ENTRY_INVALID;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   if(!USERAUTH_SetSnmpIpFilterStatus(ip_filter_entry))
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to set SNMP IP filter\r\n");
#endif
   }
#endif
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Snmp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   SNMP_MGR_STATS_T snmp_stats;
   BOOL_T  first_snmp_host = TRUE;
   char Contact[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN + 1] = {0};
   char Location[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN + 1] = {0};
   char ip_str[L_INET_MAX_IPADDR_STR_LEN + 1] = {0};
#if (SYS_CPNT_SNMP_VERSION == 3)
   SNMP_MGR_SnmpCommunity_T comm_entry;
   SNMP_MGR_TrapDestEntry_T trap_receiver;
#else
   USERAUTH_SnmpCommunity_T comm_entry;
   TRAP_MGR_TrapDestEntry_T trap_receiver;
#endif

   UI32_T community_num = 0;
   UI8_T  trap_status = 0;
   UI32_T line_num = 0;
   char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};
#if (SYS_CPNT_SNMP_VERSION == 3)
   BOOL_T agent_status = TRUE;
   char  str[CLI_DEF_MAX_BUFSIZE]       = {0};
#endif
   MIB2_POM_GetSysContact((UI8_T *)Contact);
   MIB2_POM_GetSysLocation((UI8_T *)Location);

   if(strlen(Contact) != 0)
   {
      sprintf(buff, "System Contact : %s\r\n", Contact);
      PROCESS_MORE(buff);
   }

   if(strlen(Location) != 0)
   {
      sprintf(buff, "System Location : %s\r\n", Location);
      PROCESS_MORE(buff);
   }
#if (SYS_CPNT_SNMP_VERSION == 3)
   if (SNMP_PMGR_Get_AgentStatus(&agent_status))
   {
       PROCESS_MORE("\r\n");
       sprintf(buff, "SNMP Agent : %s\r\n", (agent_status == TRUE)?("Enabled"):("Disabled"));
       PROCESS_MORE(buff);
   }
#endif
   PROCESS_MORE("\r\n");
   PROCESS_MORE("SNMP Traps : \r\n");
   if (SNMP_PMGR_GetSnmpEnableAuthenTraps(&trap_status))
   {
      sprintf(buff, " Authentication : %s\r\n", (trap_status == VAL_snmpEnableAuthenTraps_enabled)?("Enabled"):("Disabled"));
      PROCESS_MORE(buff);
   }

#if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == FALSE)
   {
      IF_MGR_IfXEntry_T  if_x_entry;
      UI32_T l_port=0;

        if (SWCTRL_POM_GetNextLogicalPort(&l_port)!=SWCTRL_LPORT_UNKNOWN_PORT)
        {
            if_x_entry.if_index=l_port;
            if(IF_PMGR_GetIfXEntry(&if_x_entry)==TRUE)
            {
                sprintf(buff, " Link-up-down   : %s\r\n", (if_x_entry.if_link_up_down_trap_enable == VAL_ifLinkUpDownTrapEnable_enabled)?("Enabled"):("Disabled"));
                PROCESS_MORE(buff);
            }
        }
    }
#endif /* #if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == FALSE) */

#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
    {
        UI32_T  interval;
        BOOL_T  is_enabled = FALSE;

        if (TRUE == AMTR_OM_GetMacNotifyGlobalStatus(&is_enabled))
        {
            sprintf(buff, " MAC-notification : %s\r\n", (is_enabled == TRUE)?("Enabled"):("Disabled"));
            PROCESS_MORE(buff);
        }

        if (TRUE == AMTR_OM_GetMacNotifyInterval(&interval))
        {
            sprintf(buff, " MAC-notification interval : %ld second(s)\r\n", (long)interval/SYS_BLD_TICKS_PER_SECOND);
            PROCESS_MORE(buff);
        }
    }
#endif
   PROCESS_MORE("\r\n");
   PROCESS_MORE("SNMP Communities : \r\n");
   memset(comm_entry.comm_string_name, 0, sizeof(comm_entry.comm_string_name));
#if (SYS_CPNT_SNMP_VERSION == 3)
   while(SNMP_PMGR_GetNextSnmpCommunity(&comm_entry)== SNMP_MGR_ERROR_OK)
#else
   while(USERAUTH_PMGR_GetNextSnmpCommunity(&comm_entry))
#endif
   {
      community_num ++;
#if (SYS_CPNT_SNMP_VERSION == 3)
        switch(comm_entry.access_right)
        {
            case SNMP_MGR_ACCESS_RIGHT_GROUP_SPECIFIC:
                strcpy(str,"group-specific");
                break;
            case SNMP_MGR_ACCESS_RIGHT_READ_ONLY:
                strcpy(str,"read-only");
                break;
            case SNMP_MGR_ACCESS_RIGHT_READ_WRITE:
                strcpy(str,"read/write");
                break;
            default:
                strcpy(str,"");
                break;
      }
          sprintf(buff, "   %lu. %s, and the access level is %s\r\n", (unsigned long)community_num,
                                                               comm_entry.comm_string_name,
                                                               str);
#else
      sprintf(buff, "   %lu. %s, and the access level is %s\r\n", (unsigned long)community_num,
                                                               comm_entry.comm_string_name,
                                                               comm_entry.access_right == USERAUTH_ACCESS_RIGHT_READ_ONLY ? "read-only" : "read/write");
#endif
      PROCESS_MORE(buff);
   }
   PROCESS_MORE("\r\n");
#if (SYS_CPNT_SNMP_VERSION == 3)
   if(SNMP_PMGR_GetSnmpStatus(&snmp_stats)!= SNMP_MGR_ERROR_OK)
#else
   if(!SNMP_PMGR_GetSnmpStatus(&snmp_stats))
#endif
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      PROCESS_MORE("Failed to get SNMP statistics\r\n");
#endif
   }
   else
   {
      sprintf(buff, "%ld SNMP packets input\r\n", (long)snmp_stats.snmpInPkts);
      PROCESS_MORE(buff);

      sprintf(buff, "%5ld Bad SNMP version errors\r\n", (long)snmp_stats.snmpInBadVersions);
      PROCESS_MORE(buff);

      sprintf(buff, "%5ld Unknown community name\r\n", (long)snmp_stats.snmpInBadCommunityNames);
      PROCESS_MORE(buff);

      sprintf(buff, "%5ld Illegal operation for community name supplied\r\n", (long)snmp_stats.snmpInBadCommunityUses);
      PROCESS_MORE(buff);

      sprintf(buff, "%5ld Encoding errors\r\n", (long)snmp_stats.snmpInASNParseErrs);
      PROCESS_MORE(buff);

      sprintf(buff, "%5ld Number of requested variables\r\n", (long)snmp_stats.snmpInTotalReqVars);
      PROCESS_MORE(buff);

      sprintf(buff, "%5ld Number of altered variables\r\n", (long)snmp_stats.snmpInTotalSetVars);
      PROCESS_MORE(buff);

      sprintf(buff, "%5ld Get-request PDUs\r\n", (long)snmp_stats.snmpInGetRequests);
      PROCESS_MORE(buff);

      sprintf(buff, "%5ld Get-next PDUs\r\n", (long)snmp_stats.snmpInGetNexts);
      PROCESS_MORE(buff);

      sprintf(buff, "%5ld Set-request PDUs\r\n", (long)snmp_stats.snmpInSetRequests);
      PROCESS_MORE(buff);

      sprintf(buff, "%ld SNMP packets output\r\n", (long)snmp_stats.snmpOutPkts);
      PROCESS_MORE(buff);

      sprintf(buff, "%5ld Too big errors\r\n", (long)snmp_stats.snmpOutTooBigs);
      PROCESS_MORE(buff);

      sprintf(buff, "%5ld No such name errors\r\n", (long)snmp_stats.snmpOutNoSuchNames);
      PROCESS_MORE(buff);

      sprintf(buff, "%5ld Bad values errors\r\n", (long)snmp_stats.snmpOutBadValues);
      PROCESS_MORE(buff);

      sprintf(buff, "%5ld General errors\r\n", (long)snmp_stats.snmpOutGenErrs);
      PROCESS_MORE(buff);

      sprintf(buff, "%5ld Response PDUs\r\n", (long)snmp_stats.snmpOutGetResponses);
      PROCESS_MORE(buff);

      sprintf(buff, "%5ld Trap PDUs\r\n", (long)snmp_stats.snmpOutTraps);
      PROCESS_MORE(buff);

      PROCESS_MORE("\r\n");
   }

   /*snmp-server host */
   memset(&trap_receiver, 0, sizeof(trap_receiver));
#if (SYS_CPNT_SNMP_VERSION == 3)
   while(SNMP_PMGR_GetNextTrapReceiver(&trap_receiver)  == SNMP_MGR_ERROR_OK)
#else
   while(SNMP_PMGR_GetNextTrapReceiver(&trap_receiver) && trap_receiver.trap_dest_status == TRAP_MGR_ENTRY_VALID)
#endif
   {
      if(first_snmp_host)
      {
         PROCESS_MORE("SNMP Logging : Enabled\r\n");
         first_snmp_host = FALSE;
      }

#if (CLI_SUPPORT_SNMP_V2C == 1)
      {
         char temp_string[3] = {0};
         char temp_security_level[7] = {0};

         switch(trap_receiver.trap_dest_version)
         {
         case SNMP_MGR_SNMPV3_MODEL_V1:
            strcpy(temp_string,"1");
            break;

         case SNMP_MGR_SNMPV3_MODEL_V2C:
            strcpy(temp_string,"2c");
            break;

         case SNMP_MGR_SNMPV3_MODEL_V3:
            strcpy(temp_string,"3");
            break;

         default:
            break;
         }

        if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&trap_receiver.trap_dest_address,
                                                           ip_str,
                                                           sizeof(ip_str)))
        {
            return CLI_ERR_INTERNAL;
        }

#if (SYS_CPNT_SNMP_VERSION != 3)
         sprintf(buff, "    Logging to %s %s version %s\r\n", ip_str, trap_receiver.trap_dest_community,temp_string);
#else
         if(trap_receiver.trap_dest_security_level==VAL_snmpTargetParamsSecurityLevel_noAuthNoPriv)
         {
             strcpy(temp_security_level,"noauth");
         }

         if(trap_receiver.trap_dest_security_level==VAL_snmpTargetParamsSecurityLevel_authNoPriv)
         {
             strcpy(temp_security_level,"auth");
         }

         if(trap_receiver.trap_dest_security_level==VAL_snmpTargetParamsSecurityLevel_authPriv)
         {
             strcpy(temp_security_level,"priv");
         }

         if(trap_receiver.trap_dest_version==SNMP_MGR_SNMPV3_MODEL_V3)
         {
             if(trap_receiver.trap_dest_type == VAL_snmpNotifyType_inform)
             {
                 sprintf(buff, "    Logging to %s %s inform timeout %ld retry %ld version %s %s udp-port %lu\r\n",
                                    ip_str,
                                    trap_receiver.trap_dest_community,
                                    (long)trap_receiver.trap_inform_request_timeout,
                                    (long)trap_receiver.trap_inform_request_retry_cnt,
                                    temp_string,temp_security_level,(unsigned long)trap_receiver.trap_dest_port);
             }
             else
             {
                 sprintf(buff, "    Logging to %s %s version %s %s udp-port %lu\r\n",
                                    ip_str,
                                    trap_receiver.trap_dest_community,
                                    temp_string,temp_security_level,(unsigned long)trap_receiver.trap_dest_port);
             }
         }
         else
         {
             if(trap_receiver.trap_dest_type == VAL_snmpNotifyType_inform)
             {
                 sprintf(buff, "    Logging to %s %s inform timeout %ld retry %ld version %s udp-port %lu\r\n",
                                    ip_str,
                                    trap_receiver.trap_dest_community,
                                    (long)trap_receiver.trap_inform_request_timeout,
                                    (long)trap_receiver.trap_inform_request_retry_cnt,
                                    temp_string,(unsigned long)trap_receiver.trap_dest_port);
            }
            else
            {
                 sprintf(buff, "    Logging to %s %s version %s udp-port %lu\r\n"
                                    , ip_str,
                                    trap_receiver.trap_dest_community,
                                    temp_string,(unsigned long)trap_receiver.trap_dest_port);
             }
         }

#endif
         PROCESS_MORE(buff);
      }
#else
      sprintf(buff, "    Logging to %s\r\n", ip_str);
      PROCESS_MORE(buff);
#endif
   }

   if(first_snmp_host)
   {
      PROCESS_MORE("SNMP Logging: Disabled\r\n");
   }

#if (SYS_CPNT_IP_FILTER == TRUE)
#if !defined(MERCURY_DC)
   {
      USERAUTH_SnmpIpFilter_T ip_filter_entry;
      UI8_T ip[18] = {0};
      UI8_T mask[18] = {0};
      UI8_T count = 1;

      PROCESS_MORE("SNMP IP Filter Group :\r\n");
      while (USERAUTH_GetNextSnmpIpFilter(&ip_filter_entry))
      {
         L_INET_Ntoa(ip_filter_entry.ip_filter_ipaddress, ip);
         L_INET_Ntoa(ip_filter_entry.ip_filter_netmask, mask);
         sprintf(buff, "    %u. IP : %s Mask:%s %s\r\n", count, ip, mask, (ip_filter_entry.status == USERAUTH_ENTRY_VALID)?("Valid"):("Invalid"));
         PROCESS_MORE(buff);
         count ++;
      }

   }
#endif
#endif


   return CLI_NO_ERROR;
}

/*configuration*/
UI32_T CLI_API_Snmpserver_Community(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    //UI8_T  community_str[SYS_ADPT_MAX_COMM_STRING_NAME_LEN + 1] = {0};
    UI32_T access_type = 0;

    if (arg[1] == NULL || arg[1][1] == 'o' || arg[1][1] == 'O')
#if (SYS_CPNT_SNMP_VERSION == 3)
        access_type = SNMP_MGR_ACCESS_RIGHT_READ_ONLY;
#else
        access_type = USERAUTH_ACCESS_RIGHT_READ_ONLY;
#endif
    else
#if (SYS_CPNT_SNMP_VERSION == 3)
        access_type = SNMP_MGR_ACCESS_RIGHT_READ_WRITE;
#else
        access_type = USERAUTH_ACCESS_RIGHT_READ_WRITE;
#endif

    if(arg[0]==NULL)
        return CLI_ERR_INTERNAL;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_SNMPSERVER_COMMUNITY:

#if (SYS_CPNT_SNMP_VERSION == 3)
            if ( SNMP_PMGR_CreateSnmpCommunity((UI8_T *)arg[0], access_type) != SNMP_MGR_ERROR_OK)
#else
            if (!USERAUTH_PMGR_SetSnmpCommunityAccessRight((UI8_T *)arg[0] ,access_type))
#endif
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set SNMP community\r\n");
#endif
            }
#if (SYS_CPNT_SNMP_VERSION != 3)
            else if (!USERAUTH_SetSnmpCommunityStatus((UI8_T *)arg[0] ,USERAUTH_ENTRY_VALID))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to enable SNMP community\r\n");
#endif
            }
#endif

            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SNMPSERVER_COMMUNITY:
#if (SYS_CPNT_SNMP_VERSION == 3)
            if (SNMP_PMGR_RemoveSnmpCommunity ((UI8_T *)arg[0]) != SNMP_MGR_ERROR_OK)
#else
            if (!USERAUTH_SetSnmpCommunityStatus((UI8_T *)arg[0] ,USERAUTH_ENTRY_INVALID))
#endif
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to disable SNMP community\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Snmpserver_Contact(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    char Contact[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN + 1];
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_SNMPSERVER_CONTACT:
            if(arg[0]!=NULL)
                strcpy(Contact, arg[0]);
            else
                return CLI_ERR_INTERNAL;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SNMPSERVER_CONTACT:
            memset(Contact, 0, sizeof(Contact));
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if(!MIB2_PMGR_SetSysContact((UI8_T *)Contact))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set contact\r\n");
#endif
    }
    return CLI_NO_ERROR;
}
/*
  EPR_ID:ES3628BT-FLF-ZZ-00057
  Problem: CLI: The behavior of hostname command is NOT correct.
  Root Cause: use error command line.
  Solution: 1. use "hostname" command is modification of "system name".
            2. Add "prompt" CLI command.
*/
#if 0 /* shumin.wang delete command "snmp-server sysname" */
/* shumin.wang added for ES4827G-FLF-ZZ-00119 */
UI32_T CLI_API_Snmpserver_SysName(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI8_T SysNameBuf[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN + 1];
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_SNMPSERVER_SYSNAME:
            if(arg[0]!=NULL)
                strcpy(SysNameBuf, arg[0]);
            else
                return CLI_ERR_INTERNAL;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SNMPSERVER_SYSNAME:
            memset(SysNameBuf, 0, sizeof(SysNameBuf));
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if(!MIB2_PMGR_SetSysName(SysNameBuf))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set sysname\r\n");
#endif
    }

    return CLI_NO_ERROR;
}
#endif

UI32_T CLI_API_Snmpserver_Enable_Traps(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
    UI32_T mac_ntfy_interval = SYS_DFLT_AMTR_MAC_NOTIFY_TRAP_INTERVAL;
    BOOL_T is_mac_notify = FALSE;
#endif
    BOOL_T  is_set_auth = FALSE;
#if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == FALSE)
    BOOL_T  is_set_link = FALSE;
#endif /* #if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == FALSE) */
    BOOL_T  is_set_cfm  = FALSE;

    if(arg[0] == NULL) /*all*/
    {
        is_set_auth = TRUE;
#if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == FALSE)
        is_set_link = TRUE;
#endif /* #if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == FALSE) */
        is_set_cfm  = TRUE;
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
        is_mac_notify = TRUE;
#endif
    }
    else if (arg[0][0] == 'a' || arg[0][0] == 'A')     /*authentication*/
    {
        is_set_auth = TRUE;
    }
#if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == FALSE)
    else if (arg[0][0] == 'l' || arg[0][0] == 'L')
    {
        is_set_link = TRUE;
    }
#endif /* #if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == FALSE) */
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
    else if (arg[0][0] == 'm' || arg[0][0] == 'M')
    {
        is_mac_notify = TRUE;
    }
#endif

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_SNMPSERVER_ENABLE_TRAPS:

            if(is_set_auth && !SNMP_PMGR_SetSnmpEnableAuthenTraps(VAL_snmpEnableAuthenTraps_enabled))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to enable authentication traps.\r\n");
#endif
            }

#if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == FALSE)
            if (is_set_link && !IF_PMGR_SetIfLinkUpDownTrapEnableGlobal(VAL_ifLinkUpDownTrapEnable_enabled)
                && !SNMP_PMGR_SetSnmpEnableLinkUpDownTraps(VAL_ifLinkUpDownTrapEnable_enabled))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to enable link-up-down traps.\r\n");
#endif
            }
#endif /* #if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == FALSE) */

#if (SYS_CPNT_CFM == TRUE)
            if (TRUE == is_set_cfm)
            {
               if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_SetSNMPCcStatus(CFM_TYPE_SNMP_TRAPS_CC_ALL, TRUE))
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
        #else
                    CLI_LIB_PrintStr("Failed to enable ethernet cfm cc traps.\r\n");
    #endif
                }

                if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_SetSNMPCrosscheckStatus(CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_ALL, TRUE))
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr("Failed to enable ethernet cfm crosscheck traps.\r\n");
    #endif
                }
            }
#endif /* #if (SYS_CPNT_CFM == TRUE) */
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
        if (TRUE  == is_mac_notify)
        {
            /* 1. snmp-server enable traps mac-notification
             *    enable trap
             * 2. snmp-server enable traps mac-notification interval
             *    set new interval
             */
            if((NULL == arg[0]) || ((arg[0][0] == 'm' || arg[0][0] == 'M') && (NULL == arg[1])))
            {
                if (FALSE == AMTR_PMGR_SetMacNotifyGlobalStatus(TRUE))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to enable mac-notification traps.\r\n");
#endif
                }
            }

            if((NULL != arg[0]) && (arg[0][0] == 'm' || arg[0][0] == 'M') && (NULL != arg[1]))
            {
                mac_ntfy_interval = atoi(arg[2]);
                if (FALSE == AMTR_PMGR_SetMacNotifyInterval(mac_ntfy_interval))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to configure mac-notification traps interval.\r\n");
#endif
                }
            }

        }
#endif
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_SNMPSERVER_ENABLE_TRAPS:
            if(is_set_auth && !SNMP_PMGR_SetSnmpEnableAuthenTraps(VAL_snmpEnableAuthenTraps_disabled))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to disable authentication traps.\r\n");
#endif
            }

#if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == FALSE)
            if (is_set_link && !IF_PMGR_SetIfLinkUpDownTrapEnableGlobal(VAL_ifLinkUpDownTrapEnable_disabled))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to disable link-up-down traps.\r\n");
#endif
            }
#endif /* #if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == FALSE) */

#if (SYS_CPNT_CFM == TRUE)
            if (TRUE == is_set_cfm)
            {
               if (CFM_TYPE_CONFIG_SUCCESS != CFM_PMGR_SetSNMPCcStatus(CFM_TYPE_SNMP_TRAPS_CC_ALL, FALSE))
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr("Failed to disable ethernet cfm cc traps.\r\n");
    #endif
                }

                if(CFM_TYPE_CONFIG_SUCCESS!=CFM_PMGR_SetSNMPCrosscheckStatus(CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_ALL, FALSE))
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr("Failed to disable ethernet cfm crosscheck traps.\r\n");
    #endif
                }
            }
#endif /* #if (SYS_CPNT_CFM == TRUE) */
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
        if (TRUE == is_mac_notify)
        {
            /* 1. no snmp-server enable traps mac-notification
             *    disable
             * 2. no snmp-server enable traps mac-notification interval
             *    only reset interval
             */
            if((NULL == arg[0]) || ((arg[0][0] == 'm' || arg[0][0] == 'M') && (NULL == arg[1])))
            {
                if (FALSE == AMTR_PMGR_SetMacNotifyGlobalStatus(FALSE))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to disable mac-notification traps.\r\n");
#endif
                }
            }

            if((NULL != arg[0]) && (arg[0][0] == 'm' || arg[0][0] == 'M') && (NULL != arg[1]))
            {
                if (FALSE == AMTR_PMGR_SetMacNotifyInterval(mac_ntfy_interval))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to configure mac-notification traps interval.\r\n");
#endif
                }
            }
        }
#endif
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Snmpserver_Host(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (CLI_SUPPORT_SNMP_V2C == 1)
    L_INET_AddrIp_T trap_receiver_ip_addr;
    UI32_T version = 0;
    UI32_T udp_port;
    SNMP_MGR_TrapDestEntry_T trap_entry;

    if(arg[0]!=NULL)
    {
        memset(&trap_receiver_ip_addr, 0, sizeof(L_INET_AddrIp_T));

        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                           arg[0],
                                                           (L_INET_Addr_T *) &trap_receiver_ip_addr,
                                                           sizeof(trap_receiver_ip_addr)))
        {
            CLI_LIB_PrintStr("Invalid address format.\r\n");
            return CLI_ERR_INTERNAL;
        }
    }
    else
        return CLI_ERR_INTERNAL;

    if(arg[1]!=NULL)
    {
        if(L_STDLIB_SnmpStrcmp(arg[1],"inform")==0)
        {
            if(arg[2]==NULL)
                return CLI_ERR_INTERNAL;

            if(L_STDLIB_SnmpStrcmp(arg[2],"timeout")==0)
            {  /* timeout */
                if(arg[4]!=NULL)
                {
                    if(L_STDLIB_SnmpStrcmp(arg[4],"retry")==0)
                    {
                        if(arg[7]==NULL)
                        {
                            version = SNMP_MGR_SNMPV3_MODEL_V1;
                        }
                        else
                        {
                            if(arg[8]!=NULL)
                            {
                                switch(arg[8][0])
                                {
                                case '1':
                                    version = SNMP_MGR_SNMPV3_MODEL_V1;
                                    break;

                                case '2':
                                    version = SNMP_MGR_SNMPV3_MODEL_V2C;
                                    break;

                                case '3':
                                    version = SNMP_MGR_SNMPV3_MODEL_V3;
                                    break;

                                default:
                                    return CLI_ERR_INTERNAL;
                                }
                            }
                            else
                                return CLI_ERR_INTERNAL;
                        }
                    }
                    else
                    {
                        if(arg[5]==NULL)
                        {
                            version = SNMP_MGR_SNMPV3_MODEL_V1;
                        }
                        else
                        {
                            if(arg[6]!=NULL)
                            {
                                switch(arg[6][0])
                                {
                                case '1':
                                    version = SNMP_MGR_SNMPV3_MODEL_V1;
                                    break;

                                case '2':
                                    version = SNMP_MGR_SNMPV3_MODEL_V2C;
                                    break;

                                case '3':
                                    version = SNMP_MGR_SNMPV3_MODEL_V3;
                                    break;

                                default:
                                    return CLI_ERR_INTERNAL;
                                }
                            }
                            else
                                return CLI_ERR_INTERNAL;
                        }

                    }
                }
                else
                    return CLI_ERR_INTERNAL;
            }

            else if(L_STDLIB_SnmpStrcmp(arg[2],"retry")==0)
            {  /* timeout */
                if(arg[4]!=NULL)
                {
                    /* shumin.wang fix bug ES3628BT-FLF-ZZ-00254 */
                    if(L_STDLIB_SnmpStrcmp(arg[4],"timeout")==0)
                    {
                        if(arg[7]==NULL)
                        {
                            version = SNMP_MGR_SNMPV3_MODEL_V1;
                        }
                        else
                        {
                            if(arg[8]!=NULL)
                            {
                                switch(arg[8][0])
                                {
                                case '1':
                                    version = SNMP_MGR_SNMPV3_MODEL_V1;
                                    break;

                                case '2':
                                    version = SNMP_MGR_SNMPV3_MODEL_V2C;
                                    break;

                                case '3':
                                    version = SNMP_MGR_SNMPV3_MODEL_V3;
                                    break;

                                default:
                                    return CLI_ERR_INTERNAL;
                                }
                            }
                            else
                                return CLI_ERR_INTERNAL;
                        }
                    }
                    else
                    {
                        if(arg[5]==NULL)
                        {
                            version = SNMP_MGR_SNMPV3_MODEL_V1;
                        }
                        else
                        {
                            if(arg[6]!=NULL)
                            {
                                switch(arg[6][0])
                                {
                                case '1':
                                    version = SNMP_MGR_SNMPV3_MODEL_V1;
                                    break;

                                case '2':
                                    version = SNMP_MGR_SNMPV3_MODEL_V2C;
                                    break;

                                case '3':
                                    version = SNMP_MGR_SNMPV3_MODEL_V3;
                                    break;

                                default:
                                    return CLI_ERR_INTERNAL;
                                }
                            }
                            else
                                return CLI_ERR_INTERNAL;
                        }

                    }
                }
                else
                    return CLI_ERR_INTERNAL;
            }

            else
            {  /* WORD */
                if(arg[3]==NULL)
                {
                    version = SNMP_MGR_SNMPV3_MODEL_V1;
                }
                else
                {
                    if(arg[4]!=NULL)
                    {
                        switch(arg[4][0])
                        {
                        case '1':
                            version = SNMP_MGR_SNMPV3_MODEL_V1;
                            break;

                        case '2':
                            version = SNMP_MGR_SNMPV3_MODEL_V2C;
                            break;

                        case '3':
                            version = SNMP_MGR_SNMPV3_MODEL_V3;
                            break;

                        default:
                            return CLI_ERR_INTERNAL;
                        }
                    }
                    else
                        return CLI_ERR_INTERNAL;
                }
            }
        }/*end of if*/
        else
        {
            if (arg[2] == NULL)
            {
                version = SNMP_MGR_SNMPV3_MODEL_V1;
            }
            else
            {
                if(arg[3]!=NULL)
                {
                    switch(arg[3][0])
                    {
                    case '1':
                        version = SNMP_MGR_SNMPV3_MODEL_V1;
                        break;

                    case '2':
                        version = SNMP_MGR_SNMPV3_MODEL_V2C;
                        break;

                    case '3':
                        version = SNMP_MGR_SNMPV3_MODEL_V3;
                        break;

                    default:
                       return CLI_ERR_INTERNAL;
                    }
                }
                else
                    return CLI_ERR_INTERNAL;
            }
        }
    }/*if(arg[1]!=NULL)*/

#if (SYS_CPNT_SNMP_VERSION == 3)
    memset(&trap_entry, 0, sizeof(SNMP_MGR_TrapDestEntry_T));
#else
    memset(&trap_entry, 0, sizeof(TRAP_MGR_TrapDestEntry_T));
#endif

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_SNMPSERVER_HOST:

#if (SYS_CPNT_SNMP_VERSION == 3)
        memcpy(&trap_entry.trap_dest_address, &trap_receiver_ip_addr, sizeof(trap_receiver_ip_addr));

        if(arg[1]==NULL)
            return CLI_ERR_INTERNAL;

        if(L_STDLIB_SnmpStrcmp(arg[1],"inform")==0)
        {
            if(version==SNMP_MGR_SNMPV3_MODEL_V1)
            {
                CLI_LIB_PrintStr("Version 1 does not support to inform request notification!\r\n");
            }
            else
            {
                if(arg[2]==NULL)
                    return CLI_ERR_INTERNAL;

                /*snmp-server host host_ip inform timeout*/
                if(L_STDLIB_SnmpStrcmp(arg[2],"timeout")==0)
                {
                    if(arg[4]==NULL)
                        return CLI_ERR_INTERNAL;

                    if(L_STDLIB_SnmpStrcmp(arg[4],"retry")==0)
                    {
               	        trap_entry.trap_inform_request_timeout = atoi(arg[3]);

               	        if((arg[5]!=NULL) && (arg[6]!=NULL))
               	        {
                            trap_entry.trap_inform_request_retry_cnt = atoi(arg[5]);
                            strcpy(trap_entry.trap_dest_community,arg[6]);
                        }
                        else
                            return CLI_ERR_INTERNAL;

               	        trap_entry.trap_dest_version = version;
                        trap_entry.trap_dest_type = VAL_snmpNotifyType_inform;

                        if(version!=SNMP_MGR_SNMPV3_MODEL_V3)
                        {
                            if(arg[9]==NULL)
                            {
                                udp_port=162;
                                trap_entry.trap_dest_port = udp_port;
                            }
                            else
                            {
                                if(arg[9][0]=='u'||arg[9][0]=='U')
      	                        {
      	                            if(arg[10]!=NULL)
      	                                udp_port = atoi(arg[10]);
      	                            else
      	                                return  CLI_ERR_INTERNAL;
      	                            trap_entry.trap_dest_port = udp_port;
      	                        }
                            }
                        }
                        else
                        {
                            if(arg[9]!=NULL)
                            {
                                switch(arg[9][0])
                                {
                                case 'n':
                                case 'N':
                                    trap_entry.trap_dest_security_level = VAL_snmpTargetParamsSecurityLevel_noAuthNoPriv;
                                    break;

                                case 'a':
                                case 'A':
                                    trap_entry.trap_dest_security_level = VAL_snmpTargetParamsSecurityLevel_authNoPriv;
                                    break;

                                case 'p':
                                case 'P':
                                    trap_entry.trap_dest_security_level = VAL_snmpTargetParamsSecurityLevel_authPriv;
                                    break;

                                default:
                                    break;
                                }
                            }

                            if(arg[10]==NULL)
                            {
                                udp_port=162;
      	                        trap_entry.trap_dest_port = udp_port;
                            }
                            else
                            {
      	                        if(arg[10][0]=='u'||arg[10][0]=='U')
      	                        {
      	                            if(arg[11]!=NULL)
      	                                udp_port = atoi(arg[11]);
      	                            else
      	                                return CLI_ERR_INTERNAL;

      	                            trap_entry.trap_dest_port = udp_port;
      	                        }
                            }
                        }
                    }
                    else
                    {
                        trap_entry.trap_inform_request_timeout = atoi(arg[3]);
                        trap_entry.trap_inform_request_retry_cnt = SNMP_MGR_INFORM_REQUEST_RETRY_CNT;
                        strcpy(trap_entry.trap_dest_community,arg[4]);
           	            trap_entry.trap_dest_version = version;
                        trap_entry.trap_dest_type = VAL_snmpNotifyType_inform;

                        if(version!=SNMP_MGR_SNMPV3_MODEL_V3)
                        {
                            if(arg[7]==NULL)
                            {
                                udp_port=162;
                                trap_entry.trap_dest_port = udp_port;
                            }
                            else
                            {
                                if(arg[7][0]=='u'||arg[7][0]=='U')
    	                            {
    	                                if(arg[8]!=NULL)
    	                                    udp_port = atoi(arg[8]);
    	                                else
    	                                    return CLI_ERR_INTERNAL;

    	                                trap_entry.trap_dest_port = udp_port;
    	                            }
                            }
                        }
                        else
                        {
                            if(arg[7]!=NULL)
                            {
                                switch(arg[7][0])
                                {
                                case 'n':
                                case 'N':
                                    trap_entry.trap_dest_security_level = VAL_snmpTargetParamsSecurityLevel_noAuthNoPriv;
                                    break;

                                case 'a':
                                case 'A':
                                    trap_entry.trap_dest_security_level = VAL_snmpTargetParamsSecurityLevel_authNoPriv;
                                    break;

                                case 'p':
                                case 'P':
                                    trap_entry.trap_dest_security_level = VAL_snmpTargetParamsSecurityLevel_authPriv;
                                    break;

                                default:
                                    break;
                                }
                            }

                            if(arg[8]==NULL)
                            {
                                udp_port=162;
    	                            trap_entry.trap_dest_port = udp_port;
                            }
                            else
                            {
    	                            if(arg[8][0]=='u'||arg[8][0]=='U')
    	                            {
    	                                if(arg[9]!=NULL)
    	                                    udp_port = atoi(arg[9]);
    	                                else
    	                                    return CLI_ERR_INTERNAL;

    	                                trap_entry.trap_dest_port = udp_port;
    	                            }
                            }
                        }

                    }
                }/*end of timeout*/
                   /*snmp-server host host_ip inform retry*/
                else if(L_STDLIB_SnmpStrcmp(arg[2],"retry")==0)
                {
                    /* shumin.wang fix bug ES3628BT-FLF-ZZ-00254 */
                    if(arg[4]==NULL)
                        return CLI_ERR_INTERNAL;

                    if(L_STDLIB_SnmpStrcmp(arg[4],"timeout")==0)
                    {
               	        trap_entry.trap_inform_request_retry_cnt= atoi(arg[3]);

               	        if((arg[5]!=NULL) && (arg[6]!=NULL))
               	        {
                            trap_entry.trap_inform_request_timeout= atoi(arg[5]);
                            strcpy(trap_entry.trap_dest_community,arg[6]);
                        }
                        else
                            return CLI_ERR_INTERNAL;

               	        trap_entry.trap_dest_version = version;
                        trap_entry.trap_dest_type = VAL_snmpNotifyType_inform;

                        if(version!=SNMP_MGR_SNMPV3_MODEL_V3)
                        {
                            if(arg[9]==NULL)
                            {
                                udp_port=162;
                                trap_entry.trap_dest_port = udp_port;
                            }
                            else
                            {
                                if(arg[9][0]=='u'||arg[9][0]=='U')
      	                        {
      	                            if(arg[10]!=NULL)
      	                                udp_port = atoi(arg[10]);
      	                            else
      	                                return  CLI_ERR_INTERNAL;
      	                            trap_entry.trap_dest_port = udp_port;
      	                        }
                            }
                        }
                        else
                        {
                            if(arg[9]!=NULL)
                            {
                                switch(arg[9][0])
                                {
                                case 'n':
                                case 'N':
                                    trap_entry.trap_dest_security_level = VAL_snmpTargetParamsSecurityLevel_noAuthNoPriv;
                                    break;

                                case 'a':
                                case 'A':
                                    trap_entry.trap_dest_security_level = VAL_snmpTargetParamsSecurityLevel_authNoPriv;
                                    break;

                                case 'p':
                                case 'P':
                                    trap_entry.trap_dest_security_level = VAL_snmpTargetParamsSecurityLevel_authPriv;
                                    break;

                                default:
                                    break;
                                }
                            }

                            if(arg[10]==NULL)
                            {
                                udp_port=162;
      	                        trap_entry.trap_dest_port = udp_port;
                            }
                            else
                            {
      	                        if(arg[10][0]=='u'||arg[10][0]=='U')
      	                        {
      	                            if(arg[11]!=NULL)
      	                                udp_port = atoi(arg[11]);
      	                            else
      	                                return CLI_ERR_INTERNAL;

      	                            trap_entry.trap_dest_port = udp_port;
      	                        }
                            }
                        }
                    }
                    else
                    {
                        trap_entry.trap_inform_request_retry_cnt= atoi(arg[3]);
                        trap_entry.trap_inform_request_timeout = SNMP_MGR_INFORM_REQUEST_TIMEOUT;
                        strcpy(trap_entry.trap_dest_community,arg[4]);
           	            trap_entry.trap_dest_version = version;
                        trap_entry.trap_dest_type = VAL_snmpNotifyType_inform;

                        if(version!=SNMP_MGR_SNMPV3_MODEL_V3)
                        {
                            if(arg[7]==NULL)
                            {
                                udp_port=162;
                                trap_entry.trap_dest_port = udp_port;
                            }
                            else
                            {
                                if(arg[7][0]=='u'||arg[7][0]=='U')
    	                            {
    	                                if(arg[8]!=NULL)
    	                                    udp_port = atoi(arg[8]);
    	                                else
    	                                    return CLI_ERR_INTERNAL;

    	                                trap_entry.trap_dest_port = udp_port;
    	                            }
                            }
                        }
                        else
                        {
                            if(arg[7]!=NULL)
                            {
                                switch(arg[7][0])
                                {
                                case 'n':
                                case 'N':
                                    trap_entry.trap_dest_security_level = VAL_snmpTargetParamsSecurityLevel_noAuthNoPriv;
                                    break;

                                case 'a':
                                case 'A':
                                    trap_entry.trap_dest_security_level = VAL_snmpTargetParamsSecurityLevel_authNoPriv;
                                    break;

                                case 'p':
                                case 'P':
                                    trap_entry.trap_dest_security_level = VAL_snmpTargetParamsSecurityLevel_authPriv;
                                    break;

                                default:
                                    break;
                                }
                            }

                            if(arg[8]==NULL)
                            {
                                udp_port=162;
    	                            trap_entry.trap_dest_port = udp_port;
                            }
                            else
                            {
    	                            if(arg[8][0]=='u'||arg[8][0]=='U')
    	                            {
    	                                if(arg[9]!=NULL)
    	                                    udp_port = atoi(arg[9]);
    	                                else
    	                                    return CLI_ERR_INTERNAL;

    	                                trap_entry.trap_dest_port = udp_port;
    	                            }
                            }
                        }

                    }
                }/*end of retry*/
                /*snmp-server host host_ip inform community_string*/
                else
                {
                    trap_entry.trap_inform_request_timeout = SNMP_MGR_INFORM_REQUEST_TIMEOUT;
                    trap_entry.trap_inform_request_retry_cnt = SNMP_MGR_INFORM_REQUEST_RETRY_CNT;
                    strcpy(trap_entry.trap_dest_community,arg[2]);
                    trap_entry.trap_dest_version = version;
                    trap_entry.trap_dest_type = VAL_snmpNotifyType_inform;

                    if(version!=SNMP_MGR_SNMPV3_MODEL_V3)
                    {
                        if(arg[5]==NULL)
                        {
                            udp_port=162;
                            trap_entry.trap_dest_port = udp_port;
                        }
                        else
                        {
                            if(arg[5][0]=='u'||arg[5][0]=='U')
                            {
                                if(arg[6]!=NULL)
      	                            udp_port = atoi(arg[6]);
      	                        else
      	                            return CLI_ERR_INTERNAL;
      	                        trap_entry.trap_dest_port = udp_port;
      	                    }
                        }
                    }
                    else
                    {
                        if(arg[5]!=NULL)
                        {
                            switch(arg[5][0])
                            {
                            case 'n':
                            case 'N':
                                trap_entry.trap_dest_security_level = VAL_snmpTargetParamsSecurityLevel_noAuthNoPriv;
                                break;

                            case 'a':
                            case 'A':
                                trap_entry.trap_dest_security_level = VAL_snmpTargetParamsSecurityLevel_authNoPriv;
                                break;

                            case 'p':
                            case 'P':
                                trap_entry.trap_dest_security_level = VAL_snmpTargetParamsSecurityLevel_authPriv;
                                break;

                            default:
                                break;
                            }
                        }

                        if(arg[6]==NULL)
                        {
                            udp_port=162;
      	                    trap_entry.trap_dest_port = udp_port;
                        }
                        else
                        {
      	                    if(arg[6][0]=='u'||arg[6][0]=='U')
      	                    {
      	                        if(arg[7]!=NULL)
      	                            udp_port = atoi(arg[7]);
      	                        else
      	                            return CLI_ERR_INTERNAL;
      	                        trap_entry.trap_dest_port = udp_port;
      	                    }
                        }
                    }
                }
            }
        }/*end of inform*/
           /*snmp-server host host_ip community_string*/
        else
        {
            strcpy(trap_entry.trap_dest_community,arg[1]);
            trap_entry.trap_dest_version = version;
            trap_entry.trap_dest_type = VAL_snmpNotifyType_trap;

            if(version!=SNMP_MGR_SNMPV3_MODEL_V3)
            {
                if(arg[4]==NULL)
                {
                    udp_port=162;
      	            trap_entry.trap_dest_port = udp_port;
                }
                else
                {
                    if(arg[4][0]=='u'||arg[4][0]=='U')
      	            {
      	                if(arg[5]!=NULL)
      	                    udp_port = atoi(arg[5]);
      	                else
      	                    return CLI_ERR_INTERNAL;
      	                trap_entry.trap_dest_port = udp_port;
      	            }
                }
            }
            else
            {
                if(arg[4]!=NULL)
                {
                    switch(arg[4][0])
                    {
                    case 'n':
                    case 'N':
                        trap_entry.trap_dest_security_level = VAL_snmpTargetParamsSecurityLevel_noAuthNoPriv;
                        break;

                    case 'a':
                    case 'A':
                        trap_entry.trap_dest_security_level = VAL_snmpTargetParamsSecurityLevel_authNoPriv;
                        break;

                    case 'p':
                    case 'P':
                        trap_entry.trap_dest_security_level = VAL_snmpTargetParamsSecurityLevel_authPriv;
                        break;

                    default:
                        break;
                    }
                }

                if(arg[5]==NULL)
                {
                    udp_port=162;
           	        trap_entry.trap_dest_port = udp_port;
                }
                else
                {
           	        if(arg[5][0]=='u'||arg[5][0]=='U')
           	        {
           	            if(arg[6]!=NULL)
           	                udp_port = atoi(arg[6]);
           	            else
           	                return CLI_ERR_INTERNAL;
           	            trap_entry.trap_dest_port = udp_port;
           	        }
                }
            }
        }/*end of community string*/

        /* ES4649-32-01204, Updated entry is point referenced. Brian 2005-07-25
         */
        if (SNMP_PMGR_SetTrapReceiver( &trap_entry) != SNMP_MGR_ERROR_OK)
#else
        if(!SNMP_PMGR_SetTrapReceiverCommStringName(trap_receiver_ip_addr, arg[1]) ||
            !SNMP_PMGR_SetTrapReceiverStatus(trap_receiver_ip_addr, TRAP_MGR_ENTRY_VALID))
#endif
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set trap receiver\r\n");
#endif
            return CLI_NO_ERROR;
        }
#if (SYS_CPNT_SNMP_VERSION != 3)
        if (!SNMP_PMGR_SetTrapReceiverVersion(trap_receiver_ip_addr, version))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set trap receiver version\r\n");
#endif
        }
#endif
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SNMPSERVER_HOST:
#if (SYS_CPNT_SNMP_VERSION != 3)
        if(!SNMP_PMGR_SetTrapReceiverStatus(trap_receiver_ip_addr, TRAP_MGR_ENTRY_INVALID))
#else
        if (SNMP_PMGR_DeleteTrapReceiver(trap_receiver_ip_addr) != SNMP_MGR_ERROR_OK)

#endif
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to remove trap receiver\r\n");
#endif
        }
        break;

    default:
        return CLI_ERR_INTERNAL;
    }

#else /* #if (CLI_SUPPORT_SNMP_V2C == 1) */

    L_INET_AddrIp_T trap_receiver_ip_addr;

    if(arg[0]!=NULL)
    {
        memset(trap_receiver_ip_addr, 0, sizeof(L_INET_AddrIp_T));

        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                           arg[0],
                                                           (L_INET_Addr_T *)&trap_receiver_ip_addr,
                                                           sizeof(trap_receiver_ip_addr)))
        {
            CLI_LIB_PrintStr("Invalid IP address.\r\n");
            return CLI_ERR_INTERNAL;
        }
    }
    else
        return CLI_ERR_INTERNAL;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_SNMPSERVER_HOST:
            if(arg[1]!=NULL)
            {
                if(!SNMP_PMGR_SetTrapReceiverCommStringName(trap_receiver_ip_addr, arg[1]) ||
                        !SNMP_PMGR_SetTrapReceiverStatus(trap_receiver_ip_addr, TRAP_MGR_ENTRY_VALID))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set trap receiver\r\n");
#endif
                }
            }
            else
                return CLI_ERR_INTERNAL;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SNMPSERVER_HOST:
            if(!SNMP_PMGR_SetTrapReceiverStatus(trap_receiver_ip_addr, TRAP_MGR_ENTRY_INVALID))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to remove trap receiver\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif /* #if (CLI_SUPPORT_SNMP_V2C == 1) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Snmpserver_Location(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    char Location[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN + 1];

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_SNMPSERVER_LOCATION:
            if(arg[0]!=NULL)
                strcpy(Location,arg[0]);
            else
                return CLI_ERR_INTERNAL;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SNMPSERVER_LOCATION:
            memset(Location, 0, sizeof(Location));
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if(!MIB2_PMGR_SetSysLocation((UI8_T *)Location))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set location\r\n");
#endif
    }
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Snmpserver(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SNMP_VERSION == 3)
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W1_SNMPSERVER:
        SNMP_PMGR_Enable_Snmp_Agent();
        break;
	case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_SNMPSERVER:
	    SNMP_PMGR_Disable_Snmp_Agent();
	    break;
	default:
        return CLI_ERR_INTERNAL;
    }
#endif /* #if (SYS_CPNT_SNMP_VERSION == 3) */
	return CLI_NO_ERROR;
}

UI32_T CLI_API_Snmpserver_EngineID(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SNMP_VERSION == 3)
    if(arg[0]==NULL)
        return CLI_ERR_INTERNAL;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_SNMPSERVER_ENGINEID:
        {
            UI8_T engineid[MAXSIZE_snmpEngineID] = {0};
            if(arg[1]==NULL)
                return CLI_ERR_INTERNAL;
            switch(arg[0][0])
            {
                case 'l':
                case 'L':
                {
                    UI32_T i;
                    UI32_T j = 0;
                    UI8_T  temp_str[MAXSIZE_snmpEngineID*2] = {0};
                    char  buff[3] = {0};
                    UI32_T length;

                    if (strlen(arg[1])%2 != 0)
                    {
                        length = strlen(arg[1])/2 + 1;
                        memcpy(temp_str, arg[1], sizeof(UI8_T)*strlen(arg[1]));
                        temp_str[strlen(arg[1])] = '0';

                    }
                    else
                    {
                        length = strlen(arg[1])/2;
                        memcpy(temp_str, arg[1], sizeof(UI8_T)*strlen(arg[1]));
                    }

                    for (i = 0; i <length ; i++)
                    {
                        buff[0] = temp_str[j];
                        buff[1] = temp_str[j+1];
                        buff[2] = 0;
                        engineid[i] = (UI8_T)CLI_LIB_AtoUl(buff,16);
                        j += 2;
                    }

                if(SNMP_PMGR_Set_EngineID(engineid, length) != SNMP_MGR_ERROR_OK)
	            {
		        CLI_LIB_PrintStr("Failed to set engine ID.\r\n");
	            }
                }
                break;

                case 'r':
                case 'R':
                {
                    UI32_T i;
                    UI32_T j = 0;
                    UI8_T  temp_str[MAXSIZE_snmpEngineID*2] = {0};
                    char  buff[3] = {0};
                    UI32_T length;
                    UI32_T trap_receiver_ip_addr;
                    SNMP_MGR_SnmpRemoteEngineID_T entry;

                    memset(&entry,0,sizeof(SNMP_MGR_SnmpRemoteEngineID_T));
                    CLI_LIB_AtoIp(arg[1], (UI8_T*)&trap_receiver_ip_addr);

                    entry.snmp_remote_engineID_host = trap_receiver_ip_addr;

                    if(arg[2]==NULL)
                        return CLI_ERR_INTERNAL;

                    if (strlen(arg[2])%2 != 0)
                    {
                       length = strlen(arg[2])/2 + 1;
                       memcpy(temp_str, arg[2], sizeof(UI8_T)*strlen(arg[2]));
                       temp_str[strlen(arg[2])] = '0';
                       entry.snmp_remote_engineIDLen = length;
                    }
                    else
                    {
                       length = strlen(arg[2])/2;
                       memcpy(temp_str, arg[2], sizeof(UI8_T)*strlen(arg[2]));
                       entry.snmp_remote_engineIDLen = length;
                    }
                    for (i = 0; i < MAXSIZE_snmpEngineID; i++)
                    {
                       buff[0] = temp_str[j];
                       buff[1] = temp_str[j+1];
                       buff[2] = 0;
                       entry.snmp_remote_engineID[i] = (UI8_T)CLI_LIB_AtoUl(buff,16);
                       j += 2;
                    }

                    /* ES4649-32-01204, Updated entry is point referenced. Brian 2005-07-25
                     */
                    if(SNMP_PMGR_CreateRemoteEngineID(&entry) != SNMP_MGR_ERROR_OK)
	            	{
		        		CLI_LIB_PrintStr("Failed to set remote engine ID \r\n");
	            	}
                }
                break;

                default:
                break;
            }
        }
        break;

	case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SNMPSERVER_ENGINEID:
	    switch(arg[0][0])
	    {
	        case 'l':
	        case 'L':
	            if(SNMP_PMGR_SetDefaultSnmpEngineID() != SNMP_MGR_ERROR_OK)
	            {
		            CLI_LIB_PrintStr("Failed to restore engine ID\r\n");
	            }
	            break;

	        case 'r':
	        case 'R':
	        {
	            UI32_T trap_receiver_ip_addr;
	            SNMP_MGR_SnmpRemoteEngineID_T entry;

                    memset(&entry,0,sizeof(SNMP_MGR_SnmpRemoteEngineID_T));
                    if(arg[1]!=NULL)
                        CLI_LIB_AtoIp(arg[1], (UI8_T*)&trap_receiver_ip_addr);
                    else
                        return CLI_ERR_INTERNAL;
                    entry.snmp_remote_engineID_host = trap_receiver_ip_addr;

                    /* ES4649-32-01204, Updated entry is point referenced. Brian 2005-07-25
                     */
                    if(SNMP_PMGR_DeleteRemoteEngineID(&entry)!=SNMP_MGR_ERROR_OK)
                    {
                        CLI_LIB_PrintStr("Failed to restore remote engine ID\r\n");
                    }
	        }
	            break;

	        default:
	            break;
	    }

	break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif /* #if (SYS_CPNT_SNMP_VERSION == 3) */
	return CLI_NO_ERROR;
}

UI32_T CLI_API_Snmpserver_Group(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SNMP_VERSION == 3)
    UI32_T index = 0;
    SNMP_MGR_SnmpV3GroupEntry_T entry;

    memset(&entry, 0, sizeof(entry));

    strcpy(entry.snmpv3_group_name, arg[index]);
    index++;

    switch (arg[index][1])
    {
        case '1':
            entry.snmpv3_group_model = SNMP_MGR_SNMPV3_MODEL_V1;
            entry.snmpv3_group_security_level = VAL_vacmAccessSecurityLevel_noAuthNoPriv;

            index++;
            break;

        case '2':
            entry.snmpv3_group_model = SNMP_MGR_SNMPV3_MODEL_V2C;
            entry.snmpv3_group_security_level = VAL_vacmAccessSecurityLevel_noAuthNoPriv;

            index++;
            break;

        case '3':
            entry.snmpv3_group_model = SNMP_MGR_SNMPV3_MODEL_V3;

            index++;

            switch (arg[index][0])
            {
                case 'a':
                case 'A':
                    entry.snmpv3_group_security_level = VAL_vacmAccessSecurityLevel_authNoPriv;
                    break;

                case 'n':
                case 'N':
                    entry.snmpv3_group_security_level = VAL_vacmAccessSecurityLevel_noAuthNoPriv;
                    break;

                case 'p':
                case 'P':
                    entry.snmpv3_group_security_level = VAL_vacmAccessSecurityLevel_authPriv;
                    break;

                default:
                    return CLI_ERR_INTERNAL;
            }

            index++;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    while (NULL != arg[index])
    {
        switch (arg[index][0])
        {
            /* read view
             */
            case 'r':
            case 'R':
                index++;
                strcpy(entry.snmpv3_group_readview, arg[index]);
                break;

            /* write view
             */
            case 'w':
            case 'W':
                index++;
                strcpy(entry.snmpv3_group_writeview, arg[index]);
                break;

            /* notify view
             */
            case 'n':
            case 'N':
                index++;
                strcpy(entry.snmpv3_group_notifyview, arg[index]);
                break;

            default:
                return CLI_ERR_INTERNAL;
        }

        index++;
    }

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_SNMPSERVER_GROUP:
            /* ES4649-32-01204, Updated entry is point referenced. Brian 2005-07-25
             */
            if (SNMP_PMGR_CreateSnmpV3Group(&entry) != SNMP_MGR_ERROR_OK)
            {
                CLI_LIB_PrintStr("Failed to create snmpv3 group\r\n");
            }

            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SNMPSERVER_GROUP:
            if (SNMP_PMGR_DeleteSnmpV3Group((UI8_T *)entry.snmpv3_group_name,
                                      entry.snmpv3_group_model,
                                      entry.snmpv3_group_security_level) != SNMP_MGR_ERROR_OK)
            {
                CLI_LIB_PrintStr("Failed to delete snmpv3 group\r\n");
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif /* #if (SYS_CPNT_SNMP_VERSION == 3) */
    return CLI_NO_ERROR;
}

UI32_T
CLI_API_Snmpserver_User(
    UI16_T cmd_idx,
    char *arg[],
    CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SNMP_VERSION == 3)
    SNMP_MGR_SnmpV3UserEntry_T entry;
    UI32_T max_len = 0;
    UI32_T max_priv_len= 0;

    memset(&entry, 0, sizeof(SNMP_MGR_SnmpV3UserEntry_T));

    if(arg[0] != NULL)
        strcpy(entry.snmpv3_user_name, arg[0]);
    else
        return CLI_ERR_INTERNAL;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_SNMPSERVER_USER:

            if(arg[1] != NULL)
                strcpy(entry.snmpv3_user_group_name, arg[1]);
            else
                return CLI_ERR_INTERNAL;

            if(arg[2] != NULL)
            {
                if(arg[2][1] == '1')
                {
                    entry.snmpv3_user_security_model = SNMP_MGR_SNMPV3_MODEL_V1;
                    entry.password_from_config = FALSE;

                    /* ES4649-32-01204, Updated entry is point referenced. Brian 2005-07-25
                     */
                    if(SNMP_PMGR_CreateSnmpV3User(&entry) != SNMP_MGR_ERROR_OK)
                    {
                        CLI_LIB_PrintStr("Failed to create snmpv3 user\r\n");
                    }

                }
                else if(arg[2][1] == '2')
                {
                    entry.snmpv3_user_security_model = SNMP_MGR_SNMPV3_MODEL_V2C;
                    entry.password_from_config = FALSE;

                    /* ES4649-32-01204, Updated entry is point referenced. Brian 2005-07-25
                     */
                    if(SNMP_PMGR_CreateSnmpV3User(&entry) != SNMP_MGR_ERROR_OK)
                    {
                        CLI_LIB_PrintStr("Failed to create snmpv3 user\r\n");
                    }

                }
                else if(arg[2][1] == '3')
                {
                    entry.snmpv3_user_security_model = SNMP_MGR_SNMPV3_MODEL_V3;

                    if((arg[3] != NULL) && ((arg[3][0] == 'e') || (arg[3][0] == 'E')))
                    {
                        entry.password_from_config = TRUE;
                        if((arg[4] != NULL) && ((arg[4][0] == 'a') || (arg[4][0] == 'A')))
                        {
                            if(arg[5] != NULL)
                            {
                                if((arg[5][0] == 'm') || (arg[5][0] == 'M'))
                                {
                                    entry.snmpv3_user_auth_type = SNMP_MGR_SNMPV3_AUTHTYPE_MD5;
                                    max_len = SNMP_MGR_SNMPV3_MD5_KEY_LEN;
                                }
                                else if((arg[5][0] == 's') || (arg[5][0] == 'S'))
                                {
                                    entry.snmpv3_user_auth_type = SNMP_MGR_SNMPV3_AUTHTYPE_SHA;
                                    max_len = SNMP_MGR_SNMPV3_SHA_KEY_LEN;
                                }

                            }

                            if((arg[7]!= NULL) && (arg[8]!= NULL) &&
                                (((arg[7][0] == 'p') || (arg[7][0] == 'p')) &&
                                (((arg[8][0] == 'd') || (arg[8][0] == 'D')) ||
                                 ((arg[8][0] == '3') || (arg[8][0] == '3')) ||
                                ((arg[8][0] == 'a') || (arg[8][0] == 'A')))))
                            {
                                entry.snmpv3_user_priv_type = get_snmpv3_privtype(arg[8], SNMP_MGR_SNMPV3_AES_STR_LEN);

                                if(arg[9] != NULL)
                                {
                                    max_priv_len = get_snmpv3_privkeylen(entry.snmpv3_user_priv_type, max_len);

                                    if (!str_to_nonprintable_Length(arg[9], entry.snmpv3_user_priv_key, max_priv_len))
                                    {
                                        CLI_LIB_PrintStr("Failed to set password\r\n");

                                        return CLI_NO_ERROR;
                                    }

                                    //memcpy(entry.snmpv3_user_priv_key,arg[9],entry.snmpv3_user_priv_key_len);
                                    entry.snmpv3_user_priv_key_len = strlen(arg[9])/2;
                                }
                                else
                                    return CLI_ERR_INTERNAL;

                                entry.snmpv3_user_security_level = VAL_vacmAccessSecurityLevel_authPriv;
                            }
                            else
                            {
                                entry.snmpv3_user_security_level = VAL_vacmAccessSecurityLevel_authNoPriv;
                            }

                            if(arg[6]!=NULL)
                            {
                                if (!str_to_nonprintable_Length(arg[6], entry.snmpv3_user_auth_key, max_len))
                                {

                                    CLI_LIB_PrintStr("Failed to set password\r\n");
                                    return CLI_NO_ERROR;
                                }

                                //memcpy(entry.snmpv3_user_auth_key, arg[6], entry.snmpv3_user_auth_key_len);
                                entry.snmpv3_user_auth_key_len = strlen(arg[6])/2;
                            }
                            else
                                return CLI_ERR_INTERNAL;

                        }
                        else
                        {
                            entry.snmpv3_user_security_level = VAL_vacmAccessSecurityLevel_noAuthNoPriv;
                        }

                    }
                    else if((arg[3] != NULL)  && ((arg[3][0] == 'a') || (arg[3][0] == 'A')))
                    {
                        entry.password_from_config = FALSE;

                        if(arg[4] != NULL)
                        {
                            if((arg[4][0] == 'm') || (arg[4][0] == 'M'))
                            {
                                entry.snmpv3_user_auth_type = SNMP_MGR_SNMPV3_AUTHTYPE_MD5;
                            }
                            else if((arg[4][0] == 's') || (arg[4][0] == 'S'))
                            {
                                entry.snmpv3_user_auth_type = SNMP_MGR_SNMPV3_AUTHTYPE_SHA;
                            }

                        }

                        if((arg[6] != NULL) && (arg[7] !=  NULL) &&
                            (((arg[6][0] == 'p') || (arg[6][0] == 'p')) &&
                            (((arg[7][0] == 'd') || (arg[7][0] == 'D')) ||
                             ((arg[7][0] == '3') || (arg[7][0] == '3')) ||
                            ((arg[7][0] == 'a') || (arg[7][0] == 'A')))))
                        {
                            entry.snmpv3_user_priv_type = get_snmpv3_privtype(arg[7], SNMP_MGR_SNMPV3_AES_STR_LEN);

                            if(arg[8]!=NULL)
                                strcpy(entry.snmpv3_user_priv_password,arg[8]);
                            else
                                return CLI_ERR_INTERNAL;

                            entry.snmpv3_user_security_level = VAL_vacmAccessSecurityLevel_authPriv;
                        }
                        else
                        {
                            entry.snmpv3_user_security_level = VAL_vacmAccessSecurityLevel_authNoPriv;
                        }

                        if(arg[5]!=NULL)
                            strcpy(entry.snmpv3_user_auth_password, arg[5]);
                        else
                            return CLI_ERR_INTERNAL;
                    }
                    else
                    {
                        entry.snmpv3_user_security_level = VAL_vacmAccessSecurityLevel_noAuthNoPriv;
                    }

                    /* ES4649-32-01204, Updated entry is point referenced. Brian 2005-07-25
                     */
                    if(SNMP_PMGR_CreateSnmpV3User(&entry) != SNMP_MGR_ERROR_OK)
                    {
    	                CLI_LIB_PrintStr("Failed to create snmpv3 user\r\n");
                    }

                }
                else if ((arg[2][0] == 'r') || (arg[2][0] == 'R'))
                {
                    UI32_T trap_receiver_ip_addr;

                    if(arg[3]!=NULL)
                        CLI_LIB_AtoIp(arg[3], (UI8_T*)&trap_receiver_ip_addr);
                    else
                        return CLI_ERR_INTERNAL;

                    if ((arg[4] == NULL) || (arg[4][1] != '3'))
                    {
                        return CLI_ERR_INTERNAL;
                    }

                    entry.snmpv3_user_security_model = SNMP_MGR_SNMPV3_MODEL_V3;

                    if((arg[5] != NULL) && ((arg[5][0] == 'e') || (arg[5][0] == 'E')))
                    {
                        entry.password_from_config = TRUE;

                        if((arg[6] != NULL) && ((arg[6][0] == 'a') || (arg[6][0] == 'A')))
                        {
                            if(arg[7]!=NULL)
                            {
                                if((arg[7][0] == 'm') || (arg[7][0] == 'M'))
                                {
                                    entry.snmpv3_user_auth_type = SNMP_MGR_SNMPV3_AUTHTYPE_MD5;
                                    max_len = SNMP_MGR_SNMPV3_MD5_KEY_LEN;
                                }
                                else if((arg[7][0] == 's') || (arg[7][0] == 'S'))
                                {
                                    entry.snmpv3_user_auth_type = SNMP_MGR_SNMPV3_AUTHTYPE_SHA;
                                    max_len = SNMP_MGR_SNMPV3_SHA_KEY_LEN;
                                }

                            }

                            if((arg[9] != NULL) && (arg[10]!= NULL) &&
                                (((arg[9][0] == 'p') || (arg[9][0] == 'p')) &&
                                (((arg[10][0] == 'd') || (arg[10][0] == 'D')) ||
                                 ((arg[10][0] == '3') || (arg[10][0] == '3')) ||
                                ((arg[10][0] == 'a') || (arg[10][0] == 'A')))))
                            {
                                entry.snmpv3_user_priv_type = get_snmpv3_privtype(arg[10], SNMP_MGR_SNMPV3_AES_STR_LEN);

                                if(arg[11] != NULL)
                                {
                                    max_priv_len = get_snmpv3_privkeylen(entry.snmpv3_user_priv_type, max_len);

                                    if (!str_to_nonprintable_Length(arg[11], entry.snmpv3_user_priv_key, max_priv_len))
                                    {
                                        CLI_LIB_PrintStr("Failed to set password\r\n");
                                        return CLI_NO_ERROR;
                                    }

                                    //memcpy(entry.snmpv3_user_priv_key, arg[9], entry.snmpv3_user_priv_key_len);
                                    entry.snmpv3_user_priv_key_len = strlen(arg[11])/2;
                                }
                                else
                                    return CLI_ERR_INTERNAL;

                                entry.snmpv3_user_security_level = VAL_vacmAccessSecurityLevel_authPriv;
                            }
                            else
                            {
                                entry.snmpv3_user_security_level = VAL_vacmAccessSecurityLevel_authNoPriv;
                            }

                            if(arg[8] != NULL)
                            {
                                if (!str_to_nonprintable_Length(arg[8], entry.snmpv3_user_auth_key, max_len))
                                {
                                    CLI_LIB_PrintStr("Failed to set password\r\n");
                                    return CLI_NO_ERROR;
                                }

                                //memcpy(entry.snmpv3_user_auth_key, arg[6], entry.snmpv3_user_auth_key_len);
                                entry.snmpv3_user_auth_key_len = strlen(arg[8])/2;
                            }
                            else
                                return CLI_ERR_INTERNAL;

                        }
                        else
                        {
                            entry.snmpv3_user_security_level = VAL_vacmAccessSecurityLevel_noAuthNoPriv;
                        }

                    }
                    else if((arg[5] != NULL) && ((arg[5][0] == 'a') || (arg[5][0] == 'A')))
                    {
                        entry.password_from_config = FALSE;

                        if(arg[6]!= NULL)
                        {
                            if((arg[6][0] == 'm') || (arg[6][0] == 'M'))
                            {
                                entry.snmpv3_user_auth_type = SNMP_MGR_SNMPV3_AUTHTYPE_MD5;
                            }
                            else if((arg[6][0] == 's') || (arg[6][0] == 'S'))
                            {
                                entry.snmpv3_user_auth_type = SNMP_MGR_SNMPV3_AUTHTYPE_SHA;
                            }

                        }
                        else
                            return CLI_ERR_INTERNAL;

                        if((arg[8] != NULL) && (arg[9] !=  NULL) &&
                            (((arg[8][0] == 'p') || (arg[8][0] == 'p')) &&
                            (((arg[9][0] == 'd') || (arg[9][0] == 'D')) ||
                             ((arg[9][0] == '3') || (arg[9][0] == '3')) ||
                            ((arg[9][0] == 'a') || (arg[9][0] == 'A')))))
                        {
                            entry.snmpv3_user_priv_type = get_snmpv3_privtype(arg[9], SNMP_MGR_SNMPV3_AES_STR_LEN);

                            if(arg[10] != NULL)
                                strcpy(entry.snmpv3_user_priv_password, arg[10]);
                            else
                                return CLI_ERR_INTERNAL;

                            entry.snmpv3_user_security_level = VAL_vacmAccessSecurityLevel_authPriv;
                        }
                        else
                        {
                            entry.snmpv3_user_security_level = VAL_vacmAccessSecurityLevel_authNoPriv;
                        }

                        if(arg[7] != NULL)
                            strcpy(entry.snmpv3_user_auth_password,arg[7]);
                        else
                            return CLI_ERR_INTERNAL;

                    }
                    else
                    {
                        entry.snmpv3_user_security_level = VAL_vacmAccessSecurityLevel_noAuthNoPriv;
                    }

                    /* ES4649-32-01204, Updated entry is point referenced. Brian 2005-07-21
                     */
                    if(SNMP_PMGR_CreateSnmpRemoteUser(trap_receiver_ip_addr, &entry) != SNMP_MGR_ERROR_OK)
                    {
                        CLI_LIB_PrintStr("Failed to create snmpv3 user\r\n");
                    }

                }

            }
            else
                return CLI_ERR_INTERNAL;

            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SNMPSERVER_USER:
            if(arg[1]!= NULL)
            {
                if(arg[1][1] == '1')
                {
                    entry.snmpv3_user_security_model = SNMP_MGR_SNMPV3_MODEL_V1;

                    if(SNMP_PMGR_DeleteSnmpV3User(entry.snmpv3_user_security_model,
                                                  (UI8_T *)entry.snmpv3_user_name) != SNMP_MGR_ERROR_OK)
                    {
      	                CLI_LIB_PrintStr("Failed to delete snmpv3 user\r\n");
                    }

                }
                else if(arg[1][1] == '2')
                {
                    entry.snmpv3_user_security_model = SNMP_MGR_SNMPV3_MODEL_V2C;

                    if(SNMP_PMGR_DeleteSnmpV3User(entry.snmpv3_user_security_model,
                                                  (UI8_T *)entry.snmpv3_user_name) != SNMP_MGR_ERROR_OK)
                    {
                        CLI_LIB_PrintStr("Failed to delete snmpv3 user\r\n");
                    }

                }
                else if(arg[1][1] == '3')
                {
                    entry.snmpv3_user_security_model = SNMP_MGR_SNMPV3_MODEL_V3;

                    if(SNMP_PMGR_DeleteSnmpV3User(entry.snmpv3_user_security_model,
                                                  (UI8_T *)entry.snmpv3_user_name) != SNMP_MGR_ERROR_OK)
                    {
                        CLI_LIB_PrintStr("Failed to delete snmpv3 user\r\n");
                    }

                }
                else if ((arg[1][0]=='r') || (arg[1][0]=='R'))
                {
                    UI32_T trap_receiver_ip_addr;
                    SNMP_MGR_SnmpRemoteEngineID_T engine_id_entry;

                    memset(&engine_id_entry, 0, sizeof(SNMP_MGR_SnmpRemoteEngineID_T));

                    if(arg[2] != NULL)
                        CLI_LIB_AtoIp(arg[2], (UI8_T*)&trap_receiver_ip_addr);
                    else
                        return CLI_ERR_INTERNAL;

                    engine_id_entry.snmp_remote_engineID_host = trap_receiver_ip_addr;

                    if ((arg[3] == NULL) || (arg[3][1] != '3'))
                    {
                        return CLI_ERR_INTERNAL;
                    }

                    entry.snmpv3_user_security_model = SNMP_MGR_SNMPV3_MODEL_V3;

                    if(SNMP_PMGR_GetSnmpRemoteEngineIDEntry(&engine_id_entry) == SNMP_MGR_ERROR_OK)
                    {
                        UI32_T i;

                        for (i = 0; i < engine_id_entry.snmp_remote_engineIDLen; i ++)
                        {
                            entry.snmpv3_user_engine_id[i] = engine_id_entry.snmp_remote_engineID[i];
                        }

                        /* ES4649-32-01204, Updated entry is point referenced. Brian 2005-07-21
                         */
                        if(SNMP_PMGR_DeleteSnmpRemoteUser(&entry) != SNMP_MGR_ERROR_OK)
                        {
                            CLI_LIB_PrintStr("Failed to delete remote snmpv3 user\r\n");
                        }

                    }

                }

            }
            else
                return CLI_ERR_INTERNAL;

            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif /* #if (SYS_CPNT_SNMP_VERSION == 3) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Snmpserver_View(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SNMP_VERSION == 3)
	SNMP_MGR_SnmpV3ViewEntry_T entry;
	UI32_T  ret;

	memset(&entry, 0, sizeof(entry));

	if(arg[0]!=NULL)
	    strcpy(entry.snmpv3_view_name,arg[0]);
	else
	    return CLI_ERR_INTERNAL;

	if(arg[1] != NULL)
	{
	    strcpy(entry.snmpv3_wildcard_subtree,arg[1]);
	}

	//strcpy(entry.snmpv3_view_subtree,arg[1]);
	if(arg[2]!=NULL)
	{
    	if((arg[2][0] == 'i') || (arg[2][0] == 'I'))
    	{
    		entry.snmpv3_view_type = VAL_vacmViewTreeFamilyType_included;
    	}
    	else if((arg[2][0] == 'e') || (arg[2][0] == 'E'))
    	{
    		entry.snmpv3_view_type = VAL_vacmViewTreeFamilyType_excluded;
    	}
    }
#if 0
    if((arg[3]!=NULL) && (arg[4]!=NULL))
    {
	    if((arg[3][0] == 'm') || (arg[3][0] == 'M'))
    	{
    		strcpy(entry.snmpv3_view_mask,arg[4]);
    	}
    }
#endif
	switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_SNMPSERVER_VIEW:

            /* ES4649-32-01204, Updated entry is point referenced. Brian 2005-07-25
             */
            if(SNMP_PMGR_CreateSnmpV3View(&entry) != SNMP_MGR_ERROR_OK)
            {
        	    CLI_LIB_PrintStr("Failed to create snmpv3 view\r\n");
            }

            break;
	case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SNMPSERVER_VIEW:
	        if(arg[1] != NULL)
    	    {
    	        ret = SNMP_PMGR_DeleteSnmpV3View((UI8_T *)entry.snmpv3_view_name,
    	                                 (UI8_T *)entry.snmpv3_wildcard_subtree/*entry.snmpv3_view_subtree*/);
            }
            else
            {
                ret = SNMP_PMGR_DeleteSnmpV3ViewByName((UI8_T *)entry.snmpv3_view_name);
            }

            if(ret != SNMP_MGR_ERROR_OK)
            {
            	    CLI_LIB_PrintStr("Failed to delete snmpv3 view\r\n");
            }
	        break;
	default:
            return CLI_ERR_INTERNAL;
    }
#endif /* #if (SYS_CPNT_SNMP_VERSION == 3) */
	return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Snmp_EngineID(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SNMP_VERSION == 3)
    /*show snmp-server engine-id local*/
    {
    	UI8_T  engineid[MAXSIZE_snmpEngineID];
    	UI32_T engineIDLen=0;
    	UI32_T engine_boots=0;

    	memset(engineid,0,sizeof(engineid));

        if(SNMP_PMGR_GetEngineID(engineid,&engineIDLen) == SNMP_MGR_ERROR_OK)
    	{
    		char str[MAXSIZE_snmpEngineID*2+1] = {0};
        	UI32_T i;

        	for(i=0;i<engineIDLen;i++)
        	{
        	    char temp[10] = {0};
        	    sprintf(temp,"%02X",engineid[i]);
        	    strcat(str,temp);
        	}

    		CLI_LIB_PrintStr_1("Local SNMP Engine ID    : %s\r\n",str);
    	}
    	SNMP_PMGR_GetEngineBoots(&engine_boots);
    	CLI_LIB_PrintStr_1("Local SNMP Engine Boots : %lu\r\n",(unsigned long)engine_boots);
    }

    /*show snmp-server engine-id remote*/
    {
        UI8_T remote_engineid[MAXSIZE_snmpEngineID];
    	UI32_T line_num = 0;
    	char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};
    	UI32_T i;
    	char str[MAXSIZE_snmpEngineID*2+1] = {0};
    	SNMP_MGR_SnmpRemoteEngineID_T entry;

    	memset(&entry,0,sizeof(SNMP_MGR_SnmpRemoteEngineID_T));
    	memset(remote_engineid,0,sizeof(remote_engineid));

    	PROCESS_MORE("\r\n");
    	while(SNMP_PMGR_GetNextSnmpRemoteEngineIDEntry(&entry)==SNMP_MGR_ERROR_OK)
    	{
    	    memset(str,0,sizeof(str));

    	    for(i=0;i<entry.snmp_remote_engineIDLen;i++)
    	    {
    	        char temp[10] = {0};
    	        sprintf(temp,"%02X",entry.snmp_remote_engineID[i]);
    	        strcat(str,temp);
    	    }

            PROCESS_MORE("Remote SNMP Engine ID                                             IP address\r\n");
            sprintf(buff,"%-64s  %d.%d.%d.%d\r\n",str,((UI8_T *)(&(entry.snmp_remote_engineID_host)))[0],((UI8_T *)(&(entry.snmp_remote_engineID_host)))[1],
                                                     ((UI8_T *)(&(entry.snmp_remote_engineID_host)))[2],((UI8_T *)(&(entry.snmp_remote_engineID_host)))[3]);
    	    PROCESS_MORE(buff);
    	}
    }
#endif /* #if (SYS_CPNT_SNMP_VERSION == 3) */

	return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Snmp_Group(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SNMP_VERSION == 3)
	UI32_T line_num = 0;
	char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};
	UI32_T first = 1;
	SNMP_MGR_SnmpV3GroupEntry_T entry;
	char  temp[CLI_DEF_MAX_BUFSIZE]       = {0};
	char  *storage_type[] = {  "Other",
                                "Volatile",
                                "Nonvolatile",
                                "Permanent",
                                "Readonly"
                            };
	char *row_status[] = {  "Active",
                             "NotInService",
                             "NotReady",
                             "CreateAndGo",
                             "CreateAndWait",
                             "Destroy"
                            };

	memset(&entry,0,sizeof(entry));
	while(SNMP_PMGR_GetNextSnmpV3Group(&entry) == SNMP_MGR_ERROR_OK)
	{
		first = 0;
	    sprintf(buff, "Group Name     : %s\r\n", entry.snmpv3_group_name);
        PROCESS_MORE(buff);

        if(entry.snmpv3_group_model == SNMP_MGR_SNMPV3_MODEL_V1)
        {
    	    sprintf(temp,"v1");
        }
        else if(entry.snmpv3_group_model == SNMP_MGR_SNMPV3_MODEL_V2C)
        {
    	    sprintf(temp,"v2c");
        }
        else if(entry.snmpv3_group_model == SNMP_MGR_SNMPV3_MODEL_V3)
        {
    	    sprintf(temp,"v3");
        }
        else
        {
        	*temp='\0';
        }

        sprintf(buff, "Security Model : %s\r\n", temp);
        PROCESS_MORE(buff);

        /* security level
         */
        if (entry.snmpv3_group_model == SNMP_MGR_SNMPV3_MODEL_V3)
        {
            switch (entry.snmpv3_group_security_level)
            {
                case VAL_vacmAccessSecurityLevel_noAuthNoPriv:
                    strcpy(temp, "No authentication and no privacy");
                    break;

                case VAL_vacmAccessSecurityLevel_authNoPriv:
                    strcpy(temp, "Authentication and no privacy");
                    break;

                case VAL_vacmAccessSecurityLevel_authPriv:
                    strcpy(temp, "Authentication and privacy");
                    break;

                default:
                    temp[0] = '\0';
                    break;
            }

            sprintf(buff, "Security Level : %s\r\n", temp);
            PROCESS_MORE(buff);
        }

        if(strcmp(entry.snmpv3_group_readview,"") == 0)
        {
        	sprintf(buff, "Read View      : No readview specified\r\n");
        }
        else
        {
        	sprintf(buff, "Read View      : %s\r\n", entry.snmpv3_group_readview);
        }
        PROCESS_MORE(buff);

        if(strcmp(entry.snmpv3_group_writeview,"") == 0)
        {
        	sprintf(buff, "Write View     : No writeview specified\r\n");
        }
        else
        {
        	sprintf(buff, "Write View     : %s\r\n", entry.snmpv3_group_writeview);
        }
        PROCESS_MORE(buff);

        if(strcmp(entry.snmpv3_group_notifyview,"") == 0)
        {
        	sprintf(buff, "Notify View    : No notifyview specified\r\n");
        }
        else
        {
        	sprintf(buff, "Notify View    : %s\r\n", entry.snmpv3_group_notifyview);
        }
        PROCESS_MORE(buff);

        sprintf(buff, "Storage Type   : %s\r\n", storage_type[entry.snmpv3_group_storage_type-1]);
        PROCESS_MORE(buff);

        sprintf(buff, "Row Status     : %s\r\n", row_status[entry.snmpv3_group_status-1]);
        PROCESS_MORE(buff);

        PROCESS_MORE("\r\n");
    }

    if(first == 1)
    {
    	CLI_LIB_PrintStr("No group exist.\r\n");
    }
#endif /* #if (SYS_CPNT_SNMP_VERSION == 3) */

	return CLI_NO_ERROR;
}

UI32_T
CLI_API_Show_Snmp_User(
    UI16_T cmd_idx,
    char *arg[],
    CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SNMP_VERSION == 3)
	UI32_T line_num = 0;
	char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};
	UI32_T first = 1;
	SNMP_MGR_SnmpV3UserEntry_T  entry;
	char  temp[CLI_DEF_MAX_BUFSIZE]       = {0};
	char  *storage_type[] = {  "Other",
                                "Volatile",
                                "Nonvolatile",
                                "Permanent",
                                "Readonly"
                            };
	char *row_status[] = {  "Active",
                             "NotInService",
                             "NotReady",
                             "CreateAndGo",
                             "CreateAndWait",
                             "Destroy"
                            };

    memset(&entry,0,sizeof(SNMP_MGR_SnmpV3UserEntry_T));

    /* show snmp server local
     */
    {
	while(SNMP_PMGR_GetNextSnmpV3User(&entry) == SNMP_MGR_ERROR_OK)
	{
            char str[MAXSIZE_snmpEngineID*2+1] = {0};
    	    UI32_T i;

#if 0       /* Let CLI display the same as WEB page */
    	    if ((entry.snmpv3_user_security_model == SNMP_MGR_SNMPV3_MODEL_V1) ||
                (entry.snmpv3_user_security_model == SNMP_MGR_SNMPV3_MODEL_V2C))
                continue;
#endif
    	    for(i = 0; i < entry.snmpv3_user_engineIDLen; i ++)
            {
                char temp[10] = {0};
                sprintf(temp, "%02x", entry.snmpv3_user_engine_id[i]);
                strcat(str,temp);
    	    }

            sprintf(buff, "Engine ID               : %s\r\n", str);
            PROCESS_MORE(buff);

            sprintf(buff, "User Name               : %s\r\n", entry.snmpv3_user_name);
            PROCESS_MORE(buff);

            /* group name
             */
            sprintf(buff, "Group Name              : %s\r\n", entry.snmpv3_user_group_name);
            PROCESS_MORE(buff);

            /* security model
             */
            switch (entry.snmpv3_user_security_model)
            {
                case SNMP_MGR_SNMPV3_MODEL_V1:
                    strcpy(temp, "v1");
                    break;

                case SNMP_MGR_SNMPV3_MODEL_V2C:
                    strcpy(temp, "v2c");
                    break;

                case SNMP_MGR_SNMPV3_MODEL_V3:
                   strcpy(temp, "v3");
                   break;

                default:
                    temp[0] = '\0';
                    break;
            }

            sprintf(buff, "Security Model          : %s\r\n", temp);
            PROCESS_MORE(buff);

            /* security level
             */
            if (entry.snmpv3_user_security_model == SNMP_MGR_SNMPV3_MODEL_V3)
            {
                switch (entry.snmpv3_user_security_level)
                {
                    case VAL_vacmAccessSecurityLevel_noAuthNoPriv:
                        strcpy(temp, "No authentication and no privacy");
                        break;

                    case VAL_vacmAccessSecurityLevel_authNoPriv:
                        strcpy(temp, "Authentication and no privacy");
                        break;

                    case VAL_vacmAccessSecurityLevel_authPriv:
                        strcpy(temp, "Authentication and privacy");
                        break;

                    default:
                        temp[0] = '\0';
                        break;
                }

                sprintf(buff, "Security Level          : %s\r\n", temp);
                PROCESS_MORE(buff);
            }

            if(entry.snmpv3_user_auth_type == SNMP_MGR_SNMPV3_AUTHTYPE_MD5)
            {
                sprintf(temp, "MD5");
            }
            else if(entry.snmpv3_user_auth_type == SNMP_MGR_SNMPV3_AUTHTYPE_SHA)
            {
                sprintf(temp, "SHA");
            }
            else if(entry.snmpv3_user_auth_type == SNMP_MGR_SNMPV3_AUTHTYPE_NONE)
            {
                sprintf(temp, "None");
            }
            else
            {
       	        *temp='\0';
            }

            sprintf(buff, "Authentication Protocol : %s\r\n", temp);
            PROCESS_MORE(buff);

            if(entry.snmpv3_user_priv_type == SNMP_MGR_SNMPV3_PRIVTYPE_DES)
            {
                sprintf(temp, "DES56");
            }
            else if(entry.snmpv3_user_priv_type == SNMP_MGR_SNMPV3_PRIVTYPE_3DES)
            {
                sprintf(temp,"3DES");
            }
            else if(entry.snmpv3_user_priv_type == SNMP_MGR_SNMPV3_PRIVTYPE_AES128)
            {
                sprintf(temp, "AES128");
            }
            else if(entry.snmpv3_user_priv_type == SNMP_MGR_SNMPV3_PRIVTYPE_AES192)
            {
                sprintf(temp, "AES192");
            }
            else if(entry.snmpv3_user_priv_type == SNMP_MGR_SNMPV3_PRIVTYPE_AES256)
            {
                sprintf(temp, "AES256");
            }
            else if(entry.snmpv3_user_priv_type == SNMP_MGR_SNMPV3_PRIVTYPE_NONE)
            {
                sprintf(temp, "None");
            }
            else
            {
                *temp='\0';
            }

            sprintf(buff, "Privacy Protocol        : %s\r\n", temp);
            PROCESS_MORE(buff);

            sprintf(buff, "Storage Type            : %s\r\n", storage_type[entry.snmpv3_user_storage_type-1]);
            PROCESS_MORE(buff);

            sprintf(buff, "Row Status              : %s\r\n", row_status[entry.snmpv3_user_status-1]);
            PROCESS_MORE(buff);

            PROCESS_MORE("\r\n");

            first = 0;
        }

	    if(first == 1)
        {
    	    PROCESS_MORE("No user exist.\r\n");
        }
    }

    /* show snmp server remote
     */
    {
    	PROCESS_MORE("\r\nSNMP remote user\r\n");
    	memset(&entry, 0, sizeof(SNMP_MGR_SnmpV3UserEntry_T));

    	while(SNMP_PMGR_GetNextSnmpRemoteUserEntry(&entry) == SNMP_MGR_ERROR_OK)
	    {
            char str[MAXSIZE_snmpEngineID*2+1] = {0};
            UI32_T i;

#if 0       /* Let CLI display the same as WEB page */
    	    if ((entry.snmpv3_user_security_model == SNMP_MGR_SNMPV3_MODEL_V1) ||
                (entry.snmpv3_user_security_model == SNMP_MGR_SNMPV3_MODEL_V2C))
                continue;
#endif
    	    for(i = 0; i < entry.snmpv3_user_engineIDLen; i ++)
    	    {
    	        char temp[10] = {0};
    	        sprintf(temp,"%02x",entry.snmpv3_user_engine_id[i]);
    	        strcat(str,temp);
            }

	        sprintf(buff, "Engine ID               : %s\r\n", str);
            PROCESS_MORE(buff);

            sprintf(buff, "User Name               : %s\r\n", entry.snmpv3_user_name);
            PROCESS_MORE(buff);

            /* group name
             */
            sprintf(buff, "Group Name              : %s\r\n", entry.snmpv3_user_group_name);
            PROCESS_MORE(buff);

            /* security model
             */
            switch (entry.snmpv3_user_security_model)
            {
                case SNMP_MGR_SNMPV3_MODEL_V1:
                    strcpy(temp, "v1");
                    break;

                case SNMP_MGR_SNMPV3_MODEL_V2C:
                    strcpy(temp, "v2c");
                    break;

                case SNMP_MGR_SNMPV3_MODEL_V3:
                    strcpy(temp, "v3");
                    break;

                default:
                    temp[0] = '\0';
                    break;
            }

            sprintf(buff, "Security Model          : %s\r\n", temp);
            PROCESS_MORE(buff);

            /* security level
             */
            if (entry.snmpv3_user_security_model == SNMP_MGR_SNMPV3_MODEL_V3)
            {
                switch (entry.snmpv3_user_security_level)
                {
                    case VAL_vacmAccessSecurityLevel_noAuthNoPriv:
                        strcpy(temp, "No authentication and no privacy");
                        break;

                    case VAL_vacmAccessSecurityLevel_authNoPriv:
                        strcpy(temp, "Authentication and no privacy");
                        break;

                    case VAL_vacmAccessSecurityLevel_authPriv:
                        strcpy(temp, "Authentication and privacy");
                        break;

                    default:
                        temp[0] = '\0';
                        break;
                }

                sprintf(buff, "Security Level          : %s\r\n", temp);
                PROCESS_MORE(buff);
            }

            if(entry.snmpv3_user_auth_type == SNMP_MGR_SNMPV3_AUTHTYPE_MD5)
            {
                sprintf(temp,"MD5");
            }
            else if(entry.snmpv3_user_auth_type == SNMP_MGR_SNMPV3_AUTHTYPE_SHA)
            {
        	sprintf(temp,"SHA");
            }
            else if(entry.snmpv3_user_auth_type == SNMP_MGR_SNMPV3_AUTHTYPE_NONE)
            {
        	sprintf(temp,"None");
            }
            else
            {
        	 temp[0] = '\0';
            }

            sprintf(buff, "Authentication Protocol : %s\r\n", temp);
            PROCESS_MORE(buff);

            if(entry.snmpv3_user_priv_type == SNMP_MGR_SNMPV3_PRIVTYPE_DES)
            {
                sprintf(temp, "DES56");
            }
            else if(entry.snmpv3_user_priv_type == SNMP_MGR_SNMPV3_PRIVTYPE_3DES)
            {
                sprintf(temp,"3DES");
            }
            else if(entry.snmpv3_user_priv_type == SNMP_MGR_SNMPV3_PRIVTYPE_AES128)
            {
                sprintf(temp, "AES128");
            }
            else if(entry.snmpv3_user_priv_type == SNMP_MGR_SNMPV3_PRIVTYPE_AES192)
            {
                sprintf(temp, "AES192");
            }
            else if(entry.snmpv3_user_priv_type == SNMP_MGR_SNMPV3_PRIVTYPE_AES256)
            {
                sprintf(temp, "AES256");
            }
            else if(entry.snmpv3_user_priv_type == SNMP_MGR_SNMPV3_PRIVTYPE_NONE)
            {
                sprintf(temp, "None");
            }
            else
            {
                temp[0] = '\0';
            }

            sprintf(buff, "Privacy Protocol        : %s\r\n", temp);
            PROCESS_MORE(buff);

            sprintf(buff, "Storage Type            : %s\r\n", storage_type[entry.snmpv3_user_storage_type-1]);
            PROCESS_MORE(buff);

            sprintf(buff, "Row Status              : %s\r\n", row_status[entry.snmpv3_user_status-1]);
            PROCESS_MORE(buff);

            PROCESS_MORE("\r\n");

            first = 0;
	}

	if(first == 1)
        {
    	    CLI_LIB_PrintStr("No user exist.\r\n");
        }

    }
#endif /* #if (SYS_CPNT_SNMP_VERSION == 3) */

	return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Snmp_View(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SNMP_VERSION == 3)
	UI32_T line_num = 0;
	char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};
	UI32_T first = 1;
	SNMP_MGR_SnmpV3ViewEntry_T  entry;
	char  temp[CLI_DEF_MAX_BUFSIZE]       = {0};
	char  *storage_type[] = {  "other",
                                "volatile",
                                "nonvolatile",
                                "permanent",
                                "readonly"
                            };
	char *row_status[] = {  "active",
                             "notInService",
                             "notReady",
                             "createAndGo",
                             "createAndWait",
                             "destroy"
                            };

	memset(&entry,0,sizeof(entry));
	while(SNMP_PMGR_GetNextSnmpV3View(&entry) == SNMP_MGR_ERROR_OK)
	{
		first = 0;

		sprintf(buff, "View Name    : %s\r\n", entry.snmpv3_view_name);
        PROCESS_MORE(buff);

        sprintf(buff, "Subtree OID  : %s\r\n", entry.snmpv3_wildcard_subtree/*snmpv3_view_subtree*/);
        PROCESS_MORE(buff);
#if 0
        sprintf(buff, "Subtree Mask : %s\r\n", entry.snmpv3_view_mask);
        PROCESS_MORE(buff);
#endif
        if(entry.snmpv3_view_type == VAL_vacmViewTreeFamilyType_excluded)
        {
        	sprintf(temp,"excluded");
        }
        else if(entry.snmpv3_view_type == VAL_vacmViewTreeFamilyType_included)
        {
        	sprintf(temp,"included");
        }
        else
        {
        	*temp='\0';
        }

        sprintf(buff, "View Type    : %s\r\n", temp);
        PROCESS_MORE(buff);

        sprintf(buff, "Storage Type : %s\r\n", storage_type[entry.snmpv3_view_storage_type-1]);
        PROCESS_MORE(buff);

        sprintf(buff, "Row Status   : %s\r\n", row_status[entry.snmpv3_view_status-1]);
        PROCESS_MORE(buff);

        PROCESS_MORE("\r\n");
	}


    if(first == 1)
    {
    	CLI_LIB_PrintStr("No view exist.\r\n");
    }
#endif /* #if (SYS_CPNT_SNMP_VERSION == 3) */

	return CLI_NO_ERROR;
}

UI32_T CLI_API_Snmpserver_NotifyFilter(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NOTIFICATIONLOG_MIB == TRUE)
    SNMP_MGR_SnmpNotifyFilterProfileEntry_T entry;
    SNMP_MGR_TrapDestEntry_T local_entry;

    if (arg[2] == NULL)
    {
        return CLI_ERR_INTERNAL;
    }

    memset(&entry, 0, sizeof(entry));

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                       arg[2],
                                                       (L_INET_Addr_T *)&entry.snmp_notify_filter_profile_ip,
                                                       sizeof(entry.snmp_notify_filter_profile_ip)))
    {
        CLI_LIB_PrintStr("Invalid IP address.\r\n");
        return CLI_ERR_INTERNAL;
    }

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_SNMPSERVER_NOTIFYFILTER:
            if (arg[0] == NULL)
            {
                return CLI_ERR_INTERNAL;
            }

            /* check host ip exist and get params name
             */
            memset(&local_entry, 0, sizeof(local_entry));
            memcpy(&local_entry.trap_dest_address, &entry.snmp_notify_filter_profile_ip,
                sizeof(local_entry.trap_dest_address));

            if (SNMP_PMGR_GetTrapReceiver(&local_entry) != SNMP_MGR_ERROR_OK)
            {
                CLI_LIB_PrintStr("Failed to create notify filter\r\n");
                return CLI_NO_ERROR;
            }

            strncpy(entry.snmp_target_params_name, local_entry.trap_dest_target_params_name, sizeof(entry.snmp_target_params_name));
            entry.snmp_target_params_name[sizeof(entry.snmp_target_params_name)-1] = '\0';
            strncpy(entry.snmp_notify_filter_profile_name, arg[0], sizeof(entry.snmp_notify_filter_profile_name));
            entry.snmp_notify_filter_profile_name[sizeof(entry.snmp_notify_filter_profile_name)-1] = '\0';
            entry.snmp_notify_filter_profile_stor_type = VAL_snmpNotifyFilterProfileStorType_nonVolatile;
            entry.snmp_notify_filter_profile_row_status = VAL_snmpNotifyFilterProfileRowStatus_active ;

            if (SNMP_PMGR_CreateSnmpNotifyFilterProfileTable(&entry) != SNMP_MGR_ERROR_OK)
            {
                CLI_LIB_PrintStr("Failed to create notify filter\r\n");
                return CLI_ERR_INTERNAL;
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SNMPSERVER_NOTIFYFILTER:
            if (SNMP_PMGR_DeleteSnmpNotifyFilterProfileTable(&entry) != SNMP_MGR_ERROR_OK)
            {
                CLI_LIB_PrintStr("Failed to delete notify filter\r\n");
                return CLI_ERR_INTERNAL;
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif /* #if (SYS_CPNT_NOTIFICATIONLOG_MIB == TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Nlm_NotifyFilter(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NOTIFICATIONLOG_MIB == TRUE)
	UI32_T admin_status = 1;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W1_NLM:
            if (arg[0] != NULL)
            {
                admin_status = 1;
            	if (FALSE == NLM_PMGR_SetConfigLogAdminStatus((UI8_T *)"\0", admin_status))
            	{
            		return CLI_ERR_INTERNAL;
            	}
        		if (FALSE == NLM_PMGR_SetConfigLogFilterName((UI8_T *)"\0", (UI8_T *)arg[0]))
        		{
        			CLI_LIB_PrintStr("Failed to config nlm filter-name\r\n");
    			    return CLI_ERR_INTERNAL;
        		}
            }
            else
            {
    			CLI_LIB_PrintStr("nlm filter name is NULL\r\n");
			    return CLI_ERR_INTERNAL;
            }
        	break;

		case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_NLM:
            admin_status = 2;
        	if (FALSE == NLM_PMGR_SetConfigLogAdminStatus((UI8_T *)"\0", admin_status))
        	{
    			CLI_LIB_PrintStr("Failed to config nlm filter-name\r\n");
			    return CLI_ERR_INTERNAL;
        	}
			break;

        default:
        	return CLI_ERR_INTERNAL;
    }
#endif /* #if (SYS_CPNT_NOTIFICATIONLOG_MIB == TRUE) */

	return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Snmp_NotifyFilter(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NOTIFICATIONLOG_MIB == TRUE)
    SNMP_MGR_SnmpNotifyFilterProfileEntry_T entry;
    char ip_str[L_INET_MAX_IPADDR_STR_LEN + 1] = {0};

    CLI_LIB_PrintStr_2("%-30s %-20s\r\n", "Filter profile name", "IP address");
    CLI_LIB_PrintStr("----------------------------  ----------------\r\n");

    memset(&entry, 0, sizeof(entry));

    while (SNMP_PMGR_GetNextSnmpNotifyFilterProfileTable(&entry) == SNMP_MGR_ERROR_OK)
    {
        if (L_INET_RETURN_SUCCESS != L_INET_InaddrToString((L_INET_Addr_T *)&entry.snmp_notify_filter_profile_ip,
                                                           ip_str,
                                                           sizeof(ip_str)))
        {
            continue;
        }

        CLI_LIB_PrintStr_2("%-30s %-20s\r\n", entry.snmp_notify_filter_profile_name, ip_str);
    }
#endif /* #if (SYS_CPNT_NOTIFICATIONLOG_MIB == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Nlm_NotifyFilter(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NOTIFICATIONLOG_MIB == TRUE)
	SNMP_MGR_NlmConfigLog_T entry;

	memset(&entry, 0, sizeof(SNMP_MGR_NlmConfigLog_T));
	if (TRUE == NLM_PMGR_GetConfigLogEntry(&entry))
	{
		CLI_LIB_PrintStr_1("Filter Name: %s\r\n", entry.filter_name);
		if (entry.oper_status == 1)
		{
			CLI_LIB_PrintStr("Oper-Status: Disable\r\n");
		}
		else if (entry.oper_status == 2)
		{
			CLI_LIB_PrintStr("Oper-Status: Operational\r\n");
		}
		else
		{
			CLI_LIB_PrintStr("Oper-Status: noFilter\r\n");
		}
	}
#endif /* #if (SYS_CPNT_NOTIFICATIONLOG_MIB == TRUE) */

	return CLI_NO_ERROR;
}

/* rmon
 */
UI32_T CLI_API_Rmon_Alarm(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T next_arg_no;
    SNMP_MGR_RmonAlarmEntry_T entry;

    memset(&entry, 0, sizeof(entry));

    entry.id = atoi(arg[0]);

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_RMON_ALARM:
            if (FALSE == is_oid_valid(arg[1]))
            {
                CLI_LIB_PrintStr("OID format is incorrect.\r\n");
                return CLI_NO_ERROR;
            }

            strncpy(entry.variable, arg[1], sizeof(entry.variable));
            entry.interval = atoi(arg[2]);

            if (('a' == arg[3][0]) || ('A' == arg[3][0]))
            {
                entry.sample_type = VAL_alarmSampleType_absoluteValue;
            }
            else
            {
                entry.sample_type = VAL_alarmSampleType_deltaValue;
            }

            entry.rising_threshold = atoi(arg[5]);
            if (('f' == arg[6][0]) || ('F' == arg[6][0]))
            {
                next_arg_no = 7;
            }
            else
            {
                entry.rising_event_index = atoi(arg[6]);
                next_arg_no = 8;
            }

            entry.falling_threshold = atoi(arg[next_arg_no]);
            next_arg_no++;

            if (NULL != arg[next_arg_no])
            {
                if (('o' != arg[next_arg_no][0]) || ('O' != arg[next_arg_no][0]))
                {
                    /* falling event index
                     */
                    entry.falling_event_index = atoi(arg[next_arg_no]);
                    next_arg_no++;
                }
            }

            if (NULL != arg[next_arg_no])
            {
                if (('o' == arg[next_arg_no][0]) || ('O' == arg[next_arg_no][0]))
                {
                    next_arg_no++;
                }

                strncpy(entry.owner, arg[next_arg_no], sizeof(entry.owner));
            }

            entry.startup_alarm = VAL_alarmStartupAlarm_risingOrFallingAlarm;
            entry.status = VAL_alarmStatus_valid;

            if (TRUE != SNMP_PMGR_CreateRmonAlarmEntry(&entry))
            {
                CLI_LIB_PrintStr("Failed to create the entry.\r\n");
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_RMON_ALARM:
            if (TRUE != SNMP_PMGR_DeleteRmonAlarmEntry(entry.id))
            {
                CLI_LIB_PrintStr("Failed to delete the entry.\r\n");
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Rmon_Event(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T next_arg_no;
    SNMP_MGR_RmonEventEntry_T entry;
    BOOL_T is_log = FALSE;
    BOOL_T is_trap = FALSE;

    memset(&entry, 0, sizeof(entry));

    entry.id = atoi(arg[0]);

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_RMON_EVENT:
            next_arg_no = 1;
            if (   (NULL != arg[next_arg_no])
                && (('l' == arg[next_arg_no][0]) || ('L' == arg[next_arg_no][0])))
            {
                is_log = TRUE;
                next_arg_no++;
            }

            if (   (NULL != arg[next_arg_no])
                && (('t' == arg[next_arg_no][0]) || ('T' == arg[next_arg_no][0])))
            {
                is_trap = TRUE;
                next_arg_no++;

                strncpy(entry.community, arg[next_arg_no], sizeof(entry.community));
                next_arg_no++;
            }

            if (TRUE == is_log)
            {
                if (TRUE == is_trap)
                {
                    entry.type = VAL_eventType_logandtrap;
                }
                else
                {
                    entry.type = VAL_eventType_log;
                }
            }
            else
            {
                if (TRUE == is_trap)
                {
                    entry.type = VAL_eventType_snmptrap;
                }
                else
                {
                    entry.type = VAL_eventType_none;
                }
            }

            if (   (NULL != arg[next_arg_no])
                && (('d' == arg[next_arg_no][0]) || ('D' == arg[next_arg_no][0])))
            {
                next_arg_no++;

                strncpy(entry.description, arg[next_arg_no], sizeof(entry.description));
                next_arg_no++;
            }

            if (   (NULL != arg[next_arg_no])
                && (('o' == arg[next_arg_no][0]) || ('O' == arg[next_arg_no][0])))
            {
                next_arg_no++;

                strncpy(entry.owner, arg[next_arg_no], sizeof(entry.owner));
                next_arg_no++;
            }

            entry.status = VAL_eventStatus_valid;

            if (TRUE != SNMP_PMGR_CreateRmonEventEntry(&entry))
            {
                CLI_LIB_PrintStr("Failed to create the entry.\r\n");
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_RMON_EVENT:
            if (TRUE != SNMP_PMGR_DeleteRmonEventEntry(entry.id))
            {
                CLI_LIB_PrintStr("Failed to delete the entry.\r\n");
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Rmon_Collection_Rmon1_ControlEntry(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T port_number;
    SNMP_MGR_RmonStatisticsEntry_T entry, existed_entry;

    memset(&entry, 0, sizeof(entry));

    entry.id = atoi(arg[0]);

    if (FALSE == get_port_info(ctrl_P, &port_number, &entry.if_index))
    {
        return CLI_ERR_INTERNAL;
    }

    if (1 < port_number)
    {
        CLI_LIB_PrintStr("This command can only be executed on one port at a time.\r\n");
        return CLI_NO_ERROR;
    }

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_RMON_COLLECTION_RMON1_CONTROLENTRY:
            if (NULL != arg[1])
            {
                strncpy(entry.owner, arg[2], sizeof(entry.owner));
            }

            entry.status = VAL_etherStatsStatus_valid;

            if (TRUE != SNMP_PMGR_CreateRmonStatisticsEntry(&entry))
            {
                CLI_LIB_PrintStr("Failed to create the entry.\r\n");
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W5_NO_RMON_COLLECTION_RMON1_CONTROLENTRY:
            if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
            {
                memset(&existed_entry, 0, sizeof(existed_entry));
                existed_entry.id = entry.id;

                if (SNMP_PMGR_GetRmonStatisticsTable(&existed_entry)
                    && (existed_entry.if_index != entry.if_index))
                {
                    /* default entry change to other port, no need remove
                    */
                    return CLI_NO_ERROR;
                }
            }

            if (TRUE != SNMP_PMGR_DeleteRmonStatisticsEntryByLport(entry.if_index, entry.id))
            {
                CLI_LIB_PrintStr("Failed to delete the entry.\r\n");
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Rmon_Collection_History_ControlEntry(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T next_arg_no;
    UI32_T port_number;
    SNMP_MGR_RmonHistoryControlEntry_T entry, existed_entry;

    memset(&entry, 0, sizeof(entry));

    entry.id = atoi(arg[0]);

    if (FALSE == get_port_info(ctrl_P, &port_number, &entry.if_index))
    {
        return CLI_ERR_INTERNAL;
    }

    if (1 < port_number)
    {
        CLI_LIB_PrintStr("This command can only be executed on one port at a time.\r\n");
        return CLI_NO_ERROR;
    }

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_RMON_COLLECTION_HISTORY_CONTROLENTRY:
            next_arg_no = 1;

            if (   (NULL != arg[next_arg_no])
                && (('o' == arg[next_arg_no][0]) || ('O' == arg[next_arg_no][0])))
            {
                strncpy(entry.owner, arg[next_arg_no + 1], sizeof(entry.owner));
                next_arg_no += 2;
            }

            if (   (NULL != arg[next_arg_no])
                && (('b' == arg[next_arg_no][0]) || ('B' == arg[next_arg_no][0])))
            {
                entry.buckets_requested = atoi(arg[next_arg_no + 1]);
                next_arg_no += 2;
            }

            if (   (NULL != arg[next_arg_no])
                && (('i' == arg[next_arg_no][0]) || ('I' == arg[next_arg_no][0])))
            {
                entry.interval = atoi(arg[next_arg_no + 1]);
                next_arg_no += 2;
            }

            entry.status = VAL_historyControlStatus_valid;

            if (TRUE != SNMP_PMGR_CreateRmonHistoryControlEntry(&entry))
            {
                CLI_LIB_PrintStr("Failed to create the entry.\r\n");
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W5_NO_RMON_COLLECTION_HISTORY_CONTROLENTRY:
            if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
            {
                memset(&existed_entry, 0, sizeof(existed_entry));
                existed_entry.id = entry.id;

                if (SNMP_PMGR_GetRmonHistoryControlTable(&existed_entry)
                    && (existed_entry.if_index != entry.if_index))
                {
                    /* default entry change to other port, no need remove
                    */
                    return CLI_NO_ERROR;
                }
            }

            if (TRUE != SNMP_PMGR_DeleteRmonHistoryControlEntryByLport(entry.if_index, entry.id))
            {
                CLI_LIB_PrintStr("Failed to delete the entry.\r\n");
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Rmon_Alarms(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T line_num = 0;
    SNMP_MGR_RmonAlarmEntry_T entry;
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};
    BOOL_T is_first_entry = TRUE;

    memset(&entry, 0, sizeof(entry));

    while (TRUE == SNMP_PMGR_GetNextRmonAlarmTable(&entry))
    {
        if (TRUE == is_first_entry)
        {
            is_first_entry = FALSE;
        }
        else
        {
            PROCESS_MORE("\r\n");
        }

        sprintf(buff, "Alarm %lu is %s, owned by %s\r\n", (unsigned long)entry.id, entry_status_str(entry.status), entry.owner);
        PROCESS_MORE(buff);

        sprintf(buff, " Monitors %s every %lu seconds\r\n", entry.variable, (unsigned long)entry.interval);
        PROCESS_MORE(buff);

        sprintf(buff, " Taking %s samples, last value was %lu\r\n", (VAL_alarmSampleType_absoluteValue == entry.sample_type) ? "absolute" : "delta", (unsigned long)entry.value);
        PROCESS_MORE(buff);

        sprintf(buff, " Rising threshold is %lu, assigned to event %lu\r\n", (unsigned long)entry.rising_threshold, (unsigned long)entry.rising_event_index);
        PROCESS_MORE(buff);

        sprintf(buff, " Falling threshold is %lu, assigned to event %lu\r\n", (unsigned long)entry.falling_threshold, (unsigned long)entry.falling_event_index);
        PROCESS_MORE(buff);
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Rmon_Events(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T line_num = 0;
    SNMP_MGR_RmonEventEntry_T entry;
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char time_str[8 + 1] = {0}; /* 00:00:00 */
    BOOL_T is_first_entry = TRUE;

    memset(&entry, 0, sizeof(entry));

    while (TRUE == SNMP_PMGR_GetNextRmonEventTable(&entry))
    {
        if (TRUE == is_first_entry)
        {
            is_first_entry = FALSE;
        }
        else
        {
            PROCESS_MORE("\r\n");
        }

        sprintf(buff, "Event %lu is %s, owned by %s\r\n", (unsigned long)entry.id, entry_status_str(entry.status), entry.owner);
        PROCESS_MORE(buff);

        sprintf(buff, " Description is %s\r\n", entry.description);
        PROCESS_MORE(buff);

        convert_timeticks_to_date_string(entry.last_time_sent, time_str);

        switch (entry.type)
        {
            case VAL_eventType_none:
                break;

            case VAL_eventType_log:
                sprintf(buff, " Event firing causes log, last fired %s\r\n", time_str);
                PROCESS_MORE(buff);
                break;

            case VAL_eventType_snmptrap:
                sprintf(buff, " Event firing causes trap to community %s, last fired %s\r\n", entry.community, time_str);
                PROCESS_MORE(buff);
                break;

            case VAL_eventType_logandtrap:
                sprintf(buff, " Event firing causes log and trap to community %s, last fired %s\r\n", entry.community, time_str);
                PROCESS_MORE(buff);
                break;

            default:
                return CLI_ERR_INTERNAL;
        }
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Rmon_Statistics(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T line_num = 0;
    SNMP_MGR_RmonStatisticsEntry_T entry;
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};
    BOOL_T is_first_entry = TRUE;

    memset(&entry, 0, sizeof(entry));

    while (TRUE == SNMP_PMGR_GetNextRmonStatisticsTable(&entry))
    {
        if (TRUE == is_first_entry)
        {
            is_first_entry = FALSE;
        }
        else
        {
            PROCESS_MORE("\r\n");
        }

        sprintf(buff, "Interface %lu is %s, and owned by %s\r\n", (unsigned long)entry.id, entry_status_str(entry.status), entry.owner);
        PROCESS_MORE(buff);

        sprintf(buff, " Monitors %s which has\r\n", entry.data_source);
        PROCESS_MORE(buff);

        sprintf(buff, " Received %lu octets, %lu packets,\r\n", (unsigned long)entry.octets, (unsigned long)entry.packets);
        PROCESS_MORE(buff);

        sprintf(buff, " %lu broadcast and %lu multicast packets,\r\n", (unsigned long)entry.bcast_pkts, (unsigned long)entry.mcast_pkts);
        PROCESS_MORE(buff);

        sprintf(buff, " %lu undersized and %lu oversized packets,\r\n", (unsigned long)entry.undersize, (unsigned long)entry.oversize);
        PROCESS_MORE(buff);

        sprintf(buff, " %lu fragments and %lu jabbers,\r\n", (unsigned long)entry.fragments, (unsigned long)entry.jabbers);
        PROCESS_MORE(buff);

        sprintf(buff, " %lu CRC alignment errors and %lu collisions.\r\n", (unsigned long)entry.crc_align, (unsigned long)entry.collisions);
        PROCESS_MORE(buff);

        sprintf(buff, " # of dropped packet events (due to lack of resources): %lu\r\n", (unsigned long)entry.drop_events);
        PROCESS_MORE(buff);

        sprintf(buff, " # of packets received of length (in octets):\r\n");
        PROCESS_MORE(buff);

        sprintf(buff, "  64: %lu, 65-127: %lu, 128-255: %lu,\r\n", (unsigned long)entry.pkts_64, (unsigned long)entry.pkts_65_127, (unsigned long)entry.pkts_128_255);
        PROCESS_MORE(buff);

        sprintf(buff, "  256-511: %lu, 512-1023: %lu, 1024-1518: %lu\r\n", (unsigned long)entry.pkts_256_511, (unsigned long)entry.pkts_512_1023, (unsigned long)entry.pkts_1024_1518);
        PROCESS_MORE(buff);
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Rmon_History(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T line_num = 0;
    SNMP_MGR_RmonHistoryControlEntry_T control_entry;
    SNMP_MGR_RmonHistoryEntry_T data_entry;
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char time_str[8 + 1] = {0}; /* 00:00:00 */
    BOOL_T is_first_entry = TRUE;

    memset(&control_entry, 0, sizeof(control_entry));

    while (TRUE == SNMP_PMGR_GetNextRmonHistoryControlTable(&control_entry))
    {
        if (TRUE == is_first_entry)
        {
            is_first_entry = FALSE;
        }
        else
        {
            PROCESS_MORE("\r\n");
        }

        sprintf(buff, "Entry %lu is %s, and owned by %s\r\n", (unsigned long)control_entry.id, entry_status_str(control_entry.status), control_entry.owner);
        PROCESS_MORE(buff);

        sprintf(buff, " Monitors %s every %lu seconds\r\n", control_entry.data_source, (unsigned long)control_entry.interval);
        PROCESS_MORE(buff);

        sprintf(buff, " Requested # of time intervals, ie buckets, is %lu\r\n", (unsigned long)control_entry.buckets_requested);
        PROCESS_MORE(buff);

        sprintf(buff, " Granted # of time intervals, ie buckets, is %lu\r\n", (unsigned long)control_entry.buckets_granted);
        PROCESS_MORE(buff);

        memset(&data_entry, 0, sizeof(data_entry));
        data_entry.control_index = control_entry.id;

        while (TRUE == SNMP_PMGR_GetNextRmonHistoryTableByControlIndex(&data_entry))
        {
            convert_timeticks_to_date_string(data_entry.start_interval, time_str);

            sprintf(buff, "  Sample # %lu began measuring at %s\r\n", (unsigned long)data_entry.data_index, time_str);
            PROCESS_MORE(buff);

            sprintf(buff, "  Received %lu octets, %lu packets,\r\n", (unsigned long)data_entry.octets, (unsigned long)data_entry.packets);
            PROCESS_MORE(buff);

            sprintf(buff, "  %lu broadcast and %lu multicast packets,\r\n", (unsigned long)data_entry.bcast_pkts, (unsigned long)data_entry.mcast_pkts);
            PROCESS_MORE(buff);

            sprintf(buff, "  %lu undersized and %lu oversized packets,\r\n", (unsigned long)data_entry.undersize, (unsigned long)data_entry.oversize);
            PROCESS_MORE(buff);

            sprintf(buff, "  %lu fragments and %lu jabbers packets,\r\n", (unsigned long)data_entry.fragments, (unsigned long)data_entry.jabbers);
            PROCESS_MORE(buff);

            sprintf(buff, "  %lu CRC alignment errors and %lu collisions.\r\n", (unsigned long)data_entry.crc_align, (unsigned long)data_entry.collisions);
            PROCESS_MORE(buff);

            sprintf(buff, "  # of dropped packet events is %lu\r\n", (unsigned long)data_entry.drop_events);
            PROCESS_MORE(buff);

            sprintf(buff, "  Network utilization is estimated at %lu\r\n", (unsigned long)data_entry.utilization);
            PROCESS_MORE(buff);
        }
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Snmpserver_Enable_PortTraps_LinkUpDown_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_SNMPSERVER_ENABLE_PORTTRAPS_LINKUPDOWN:
        {
            for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
            {
                UI32_T lport = 0;

                if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
                {
                    verify_port = i;

                    if ((verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                    {
                        display_ethernet_msg(verify_ret, verify_unit, verify_port);
                        continue;
                    }
                    else if (IF_PMGR_SetIfLinkUpDownTrapEnable(lport,VAL_ifLinkUpDownTrapEnable_enabled) != TRUE)
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to set SNMP server traps link-up-down on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set SNMP server traps link-up-down on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        }

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W5_NO_SNMPSERVER_ENABLE_PORTTRAPS_LINKUPDOWN:
        {
            for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
            {
                UI32_T lport = 0;

                if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
                {
                    verify_port = i;

                    if((verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                    {
                        display_ethernet_msg(verify_ret, verify_unit, verify_port);
                        continue;
                    }
                    else if (IF_PMGR_SetIfLinkUpDownTrapEnable(lport,VAL_ifLinkUpDownTrapEnable_disabled) != TRUE)
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to set SNMP server traps link-up-down on ethernet port %s.\r\n", name);
#endif
                       }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set SNMP server traps link-up-down on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        }

        default:
            return CLI_ERR_INTERNAL;
    }
#endif /* #if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Snmpserver_Enable_PortTraps_LinkUpDown_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRUNK_UI == TRUE)
#if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == TRUE)
    UI32_T lport;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    CLI_API_TrunkStatus_T verify_ret;

    if ((verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_SNMPSERVER_ENABLE_PORTTRAPS_LINKUPDOWN:
            if (IF_PMGR_SetIfLinkUpDownTrapEnable(lport,VAL_ifLinkUpDownTrapEnable_enabled) != TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("Failed to set SNMP server traps link-up-down on trunk %lu\r\n", (unsigned long)verify_trunk_id);
#endif
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W5_NO_SNMPSERVER_ENABLE_PORTTRAPS_LINKUPDOWN:
            if (IF_PMGR_SetIfLinkUpDownTrapEnable(lport,VAL_ifLinkUpDownTrapEnable_disabled) != TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("Failed to set SNMP server traps link-up-down on trunk %lu\r\n", (unsigned long)verify_trunk_id);
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;

    }
#endif /* #if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == TRUE) */
#endif /* #if (SYS_CPNT_TRUNK_UI == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Snmpserver_Enable_PortTraps_MacNotification_Eth(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
    UI32_T  lport   = 0;
    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port=0;
    UI32_T  verify_ret=0;
    UI32_T  i = 0;
    BOOL_T  is_enable;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_SNMPSERVER_ENABLE_PORTTRAPS_MACNOTIFICATION:
            is_enable = TRUE;
            break;
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W5_NO_SNMPSERVER_ENABLE_PORTTRAPS_MACNOTIFICATION:
            is_enable = FALSE;
            break;
        default:
            return CLI_NO_ERROR;
    }

    for(i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if(ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)] & (1 << ( 7 - ((i-1)%8))))
        {
            verify_port = i;

            if((verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }

            if(FALSE == AMTR_PMGR_SetMacNotifyPortStatus( lport, is_enable))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2( "Failed to configure mac-notification trap on port %lu/%lu.\r\n",
                    (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
            }
        }
    }
#endif /* #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Snmpserver_Enable_PortTraps_MacNotification_Pch(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRUNK_UI == TRUE)
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
    UI32_T                  verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    UI32_T                  lport;
    CLI_API_TrunkStatus_T   verify_ret;
    BOOL_T                  is_enable;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_SNMPSERVER_ENABLE_PORTTRAPS_MACNOTIFICATION:
            is_enable = TRUE;
            break;
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W5_NO_SNMPSERVER_ENABLE_PORTTRAPS_MACNOTIFICATION:
            is_enable = FALSE;
            break;
        default:
            return CLI_NO_ERROR;
    }

    if((verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
    }
    else
    {
        if(FALSE == AMTR_PMGR_SetMacNotifyPortStatus( lport, is_enable))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1( "Failed to configure mac-notification trap on trunk %lu.\r\n",
                (unsigned long)verify_trunk_id);
#endif
        }
    }
#endif /* #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) */
#endif /* #if (SYS_CPNT_TRUNK_UI == TRUE) */

    return CLI_NO_ERROR;
}

/* command: show snmp-server enable port-traps interface [eth|pch]
 */
UI32_T CLI_API_Show_Snmpserver_Enable_PortTraps(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
    UI32_T  ifindex;
    UI32_T  line_num = 1;
    UI32_T  i        = 0;
    UI32_T  verify_unit;
    UI32_T  verify_port;
    UI32_T  verify_trunk_id = 0;
    UI32_T  max_port_num;
    UI32_T  current_max_unit = 0;
    UI32_T  j;
    CLI_API_EthStatus_T     verify_ret_e;
    CLI_API_TrunkStatus_T   verify_ret_t;

#if (SYS_CPNT_STACKING == TRUE)
    current_max_unit = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
#else
    current_max_unit = 1;
#endif

    line_num = Show_MacNtfyTrapInfo_Title(ctrl_P, line_num);

    if(arg[1] == NULL)
    {
        for(j=0; STKTPLG_POM_GetNextUnit(&j); )
        {
            max_port_num = SWCTRL_POM_UIGetUnitPortNumber(j);
            for(i = 1; i <= max_port_num ; i++)
            {
                verify_unit = j;
                verify_port = i;

                if((verify_ret_e = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
                {
                    continue;
                }

                if((line_num = Show_MacNtfyTrapInfo_One(ctrl_P, ifindex, line_num)) == EXIT_SESSION_MORE)
                {
                    return CLI_EXIT_SESSION;
                }
                if(line_num == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }
            }
        }/*end of unit loop*/

        /*trunk*/
        while(TRK_PMGR_GetNextTrunkId(&verify_trunk_id))
        {
            if((verify_ret_t = verify_trunk(verify_trunk_id, &ifindex)) != CLI_API_TRUNK_OK)
            {
                continue;
            }
            if((line_num = Show_MacNtfyTrapInfo_One(ctrl_P, ifindex, line_num)) == EXIT_SESSION_MORE)
            {
                return CLI_EXIT_SESSION;
            }
            if(line_num == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
        }
    }
    else
    {
        switch(arg[1][0])
        {
            case 'e':
            case 'E':
                verify_unit = atoi(arg[2]);
                verify_port = atoi(strchr(arg[2], '/') + 1);

                if((verify_ret_e = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
                {
                    display_ethernet_msg(verify_ret_e, verify_unit, verify_port);
                }
                else
                {
                    Show_MacNtfyTrapInfo_One(ctrl_P, ifindex, line_num);
                }
                break;

            case 'p':
            case 'P':
                verify_trunk_id = atoi(arg[2]);
                if((verify_ret_t = verify_trunk(verify_trunk_id, &ifindex)) != CLI_API_TRUNK_OK)
                {
                    display_trunk_msg(verify_ret_t, verify_trunk_id);
                }
                else
                {
                    Show_MacNtfyTrapInfo_One(ctrl_P, ifindex, line_num);
                }
                break;

            default:
                return CLI_ERR_INTERNAL;
        }
    }
#endif /* #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) */
    return CLI_NO_ERROR;
}

/* LOCAL SUBPROGRAM BODIES
 */
static BOOL_T str_to_nonprintable_Length(char *str, UI8_T *om,UI32_T max_byte_len)
{
   UI32_T i;
   char  buff[3];

   if (strlen(str) == max_byte_len*2)
   {
      for (i = 0 ; i <max_byte_len; i++, om++)
      {
         buff[0] = *(str);
         buff[1] = *(str+1);
         buff[2] = 0;

         *om = (UI8_T)CLI_LIB_AtoUl(buff,16);
         str += 2;
      }
      *om = 0;
   }
   else
      return FALSE;

   return TRUE;
}
static char *entry_status_str(UI32_T status)
{
    static char *string_array[] =
    {
        "unknown",
        "valid",
        "createRequest",
        "underCreation",
        "invalid"
    };

    if (   (status >= VAL_etherStatsStatus_valid)
        && (status <= VAL_etherStatsStatus_invalid))
    {
        return string_array[status];
    }

    return string_array[0];
}

static void convert_timeticks_to_date_string(UI32_T timeticks, char *output_str_p)
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;

    if (NULL == output_str_p)
    {
        return;
    }

    SYS_TIME_ConvertSecondsToDateTime((timeticks / 100), &year, &month, &day, &hour, &minute, &second);

    sprintf(output_str_p, "%02d:%02d:%02d", hour, minute, second);
}

static BOOL_T get_port_info(CLI_TASK_WorkingArea_T *ctrl_p, UI32_T *port_number_p, UI32_T *first_if_index_p)
{
    UI32_T i;
    UI32_T max_port_num;
    UI32_T port_count;

    if (   (NULL == ctrl_p)
        || (NULL == first_if_index_p))
    {
        return FALSE;
    }

    max_port_num = SWCTRL_POM_UIGetUnitPortNumber(ctrl_p->CMenu.unit_id);
    port_count = 0;
    *first_if_index_p = 0;

    for (i = 1; i <= max_port_num; i++)
    {
        if (FALSE == SWCTRL_POM_UIUserPortExisting(ctrl_p->CMenu.unit_id, i))
        {
            continue;
        }

        if (ctrl_p->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << (7 - ((i-1)%8))))
        {
            port_count++;

            if (1 == port_count)
            {
                *first_if_index_p = i;
            }
        }
    }

    *port_number_p = port_count;
    return TRUE;
}

static BOOL_T is_oid_valid(char *oid_p)
{
    char previous_char = '.';

    if (NULL == oid_p)
    {
        return FALSE;
    }

    while (*oid_p != '\0')
    {
        if (   (('0' > *oid_p) || (*oid_p > '9'))
            && ('\0' != *oid_p)
            && ('.' != *oid_p))
        {
            return FALSE;
        }

        if (('.' == previous_char) && ('.' == *oid_p))
        {
            return FALSE;
        }

        previous_char = *oid_p;
        oid_p++;
    }

    if ('.' == previous_char)
    {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - get_snmpv3_privtype
 *---------------------------------------------------------------------------
 * PURPOSE : The function will extend priv key from short key length
 *           to enough key length for AES 192 and 256
 * INPUT   : str_p    -- the cli input priv type.
 *           str_len  -- the priv type string length
 * OUTPUT  : NONE
 * RETURN  : get priv type
 * NOTE    : If extned failed , it will return false.
 *---------------------------------------------------------------------------
 */
static UI32_T
get_snmpv3_privtype(
    char *str_p,
    UI32_T str_len)
{
    UI32_T get_type = SNMP_MGR_SNMPV3_PRIVTYPE_NONE;
    UI32_T aes_type = 0;
    char   priv_str[SNMP_MGR_SNMPV3_AES_STR_LEN + 1] = {0};

    if ((str_p[0] == 'd') || (str_p[0] == 'D'))
    {
        get_type = SNMP_MGR_SNMPV3_PRIVTYPE_DES;
    }
    else if((str_p[0] == '3') || (str_p[0] == '3'))
    {
        get_type = SNMP_MGR_SNMPV3_PRIVTYPE_3DES;
    }
    else if ((str_p[0] == 'a') || (str_p[0] == 'A'))
    {
        memcpy(priv_str, str_p + SNMP_MGR_SNMPV3_AES_STR_LEN, SNMP_MGR_SNMPV3_AES_STR_LEN);
        aes_type = atoi(priv_str);

        switch(aes_type)
        {
             case SNMP_MGR_SNMPV3_AES128:
                 get_type = SNMP_MGR_SNMPV3_PRIVTYPE_AES128;
                 break;

             case SNMP_MGR_SNMPV3_AES192:
                 get_type = SNMP_MGR_SNMPV3_PRIVTYPE_AES192;
                 break;

             case SNMP_MGR_SNMPV3_AES256:
                 get_type = SNMP_MGR_SNMPV3_PRIVTYPE_AES256;
                 break;

             default:
                 get_type = SNMP_MGR_SNMPV3_PRIVTYPE_NONE;
                 break;
        }

    }

    return get_type;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - get_snmpv3_privkeylen
 *---------------------------------------------------------------------------
 * PURPOSE : The function will extend priv key from short key length
 *           to enough key length for AES 192 and 256
 * INPUT   : priv_type -- the cli input priv type.
 *           ori_len   -- the org priv key string length
 * OUTPUT  : NONE
 * RETURN  : real get priv key length
 * NOTE    : If extned failed , it will return false.
 *---------------------------------------------------------------------------
 */
static UI32_T
get_snmpv3_privkeylen(
    UI32_T priv_type,
    UI32_T ori_len)
{
    UI32_T return_len = 0;

    switch(priv_type)
    {
        case SNMP_MGR_SNMPV3_PRIVTYPE_AES192:
            return_len = SNMP_MGR_SNMPV3_AES192_KEY_LEN;
            break;

        case SNMP_MGR_SNMPV3_PRIVTYPE_3DES:
        case SNMP_MGR_SNMPV3_PRIVTYPE_AES256:
            return_len = SNMP_MGR_SNMPV3_AES256_KEY_LEN;
            break;

        case SNMP_MGR_SNMPV3_PRIVTYPE_DES:
        case SNMP_MGR_SNMPV3_PRIVTYPE_AES128:
            return_len = ori_len;
        default:
            break;
    }

    return return_len;
}
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
static UI32_T Show_MacNtfyTrapInfo_Title(
    CLI_TASK_WorkingArea_T *ctrl_P,
    UI32_T line_num)
{
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};

    sprintf(buff, "%-9s %-21s\r\n", "Interface", "MAC Notification Trap");

    PROCESS_MORE_FUNC(buff);
    sprintf(buff, "%-9s %-21s\r\n", "---------", "---------------------");

    PROCESS_MORE_FUNC(buff);
    return line_num;
}

static UI32_T Show_MacNtfyTrapInfo_One(
    CLI_TASK_WorkingArea_T *ctrl_P,
    UI32_T  lport,
    UI32_T  line_num)
{
    UI32_T                      unit, port, trunk_id;
    SWCTRL_Lport_Type_T         port_type;
    char                        buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char                        port_str[12];
    char                        *en_str[] = {"No", "Yes"};
    char                        *en_p;
    BOOL_T                      is_port_enabled = FALSE;

    port_type = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

    if((port_type == SWCTRL_LPORT_TRUNK_PORT) ||(port_type == SWCTRL_LPORT_NORMAL_PORT))
    {
        if(FALSE == AMTR_OM_GetMacNotifyPortStatus( lport, &is_port_enabled))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            if(port_type == SWCTRL_LPORT_TRUNK_PORT)
            {
                sprintf(buff, "Failed to display SNMP traps information on port %lu/%lu.\r\n",
                    (unsigned long)unit, (unsigned long)port);
            }
            else
            {
                sprintf(buff, "Failed to display SNMP traps information on trunk %lu.\r\n",
                    (unsigned long)trunk_id);
            }
#endif
            PROCESS_MORE_FUNC(buff);
            return line_num;
        }
    }
    else
    {
        return line_num;
    }

    if(port_type == SWCTRL_LPORT_NORMAL_PORT)
    {
        sprintf(port_str, "Eth %lu/%lu", (unsigned long)unit, (unsigned long)port);
    }
    else
    {
        sprintf(port_str, "Trunk %lu", (unsigned long)trunk_id);
    }

    en_p    = (TRUE == is_port_enabled)?en_str[1]:en_str[0];
    sprintf(buff, "%-9s %21s\r\n", port_str, en_p);
    PROCESS_MORE_FUNC(buff);
    return line_num;
}

/* Sample:
 * Interface MAC Notification Trap
 * --------- ---------------------
 *         9                    21
 * Eth 1/1                     Yes
 * Eth 1/2                      No
 * Trunk 1                     Yes
 */
#endif /* #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE) */

