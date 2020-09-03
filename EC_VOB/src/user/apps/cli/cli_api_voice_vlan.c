#include <stdio.h>
#include <ctype.h>
#include "sys_cpnt.h"
#include "l_cvrt.h"
#include "cli_api.h"
#include "add_pmgr.h"
#include "cli_api_voice_vlan.h"

/**************************************<<VOICE VLAN>>*****************************************/

#if (SYS_CPNT_ADD == TRUE)
static UI32_T CLI_API_ShowOui(UI32_T line_num);
static UI32_T CLI_API_ShowVoiceVlanStatus(UI32_T line_num);
#endif  /*#if (SYS_CPNT_ADD == TRUE) */

/* command: voice vlan <vlan-id> */
UI32_T CLI_API_Voice_Vlan_Id(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ADD == TRUE)
    UI32_T vid;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_VOICE_VLAN:
            vid = atoi(arg[0]);
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_VOICE_VLAN:
            vid = VAL_voiceVlanEnabledId_disabled;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if(ADD_PMGR_SetVoiceVlanEnabledId(vid) != TRUE)
    {
        CLI_LIB_PrintStr("Fail to set the voice VLAN.\r\n");
    }

#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_VoiceVlan_AgingTime(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ADD == TRUE)
    UI32_T timeout;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_VOICE_VLAN_AGING:
            timeout=atoi(arg[0]);
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_VOICE_VLAN_AGING:
            timeout = SYS_DFLT_ADD_VOICE_VLAN_TIMEOUT_MINUTE;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (ADD_PMGR_SetVoiceVlanAgingTime(timeout) == FALSE)
    {
        CLI_LIB_PrintStr("Fail to set voice VLAN timeout.\r\n");
    }

#endif /* #if (SYS_CPNT_ADD == TRUE) */
    return CLI_NO_ERROR;
}

/*command: voice vlan mac-address mac-address mask mask-address [description description] */
UI32_T CLI_API_Voice_Vlan_MacAddress(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ADD == TRUE)
    ADD_MGR_VoiceVlanOui_T entry;

    memset(&entry, 0, sizeof(entry));

    if (CLI_LIB_ValsInMac(arg[0], entry.oui) == FALSE)
    {
        CLI_LIB_PrintStr("Invalid oui address.\r\n");
        return CLI_ERR_INTERNAL;
    }

    if (CLI_LIB_ValsInMac(arg[2], entry.mask) == FALSE)
    {
        CLI_LIB_PrintStr("Invalid mask address.\r\n");
        return CLI_ERR_INTERNAL;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_VOICE_VLAN_MACADDRESS:
            /* description <LINE>
             */
            if (NULL != arg[3])
            {
                strncpy(entry.description, arg[4], sizeof(entry.description)-1);
            }

            if(ADD_PMGR_AddOuiEntry(&entry) == FALSE)
                CLI_LIB_PrintStr("Fail to set the MAC address.\r\n");
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_VOICE_VLAN_MACADDRESS:
            if(ADD_PMGR_RemoveOuiEntry(entry.oui, entry.mask) == FALSE)
                CLI_LIB_PrintStr("Fail to remove the MAC address.\r\n");
            break;

        default:
            return CLI_ERR_INTERNAL;
   }
#endif
      return CLI_NO_ERROR;
}

/* command: switchport voice-vlan mode */
UI32_T CLI_API_Switchport_Voice_Vlan_Mode_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ADD == TRUE)
    UI32_T lport;
    UI32_T port_mode;
    UI32_T i;

    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_SWITCHPORT_VOICE_VLAN:
            if (arg[0][0]== 'm' || arg[0][0] == 'M')
            {
                port_mode = VAL_voiceVlanPortMode_manual;
            }
            else if (arg[0][0]== 'a' || arg[0][0] == 'A')
            {
                port_mode = VAL_voiceVlanPortMode_auto;
            }
            else
            {
                return CLI_ERR_INTERNAL;
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_SWITCHPORT_VOICE_VLAN:
            port_mode = VAL_voiceVlanPortMode_none;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;

            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
               display_ethernet_msg(verify_ret, verify_unit, verify_port);
               continue;
            }

            if (FALSE == ADD_PMGR_SetVoiceVlanPortMode(lport, port_mode))
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
                {
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Fail to set voice VLAN port mode on Ethernet %s\r\n", name);
#endif
                }
#else /* CLI_SUPPORT_PORT_NAME */
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Fail to set voice VLAN port mode on Ethernet %lu/%lu\r\n", verify_unit, verify_port);
#endif
#endif
            }
        }
    }
#endif
   return CLI_NO_ERROR;

}

UI32_T CLI_API_Switchport_Voice_Vlan_Priority_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ADD_CONFIGURABLE_PRIORTIY == TRUE)
    UI32_T lport;
    UI32_T i;
    UI8_T priority;

    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_SWITCHPORT_VOICE_VLAN_PRIORITY:
            priority=atoi(arg[0]);
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W5_NO_SWITCHPORT_VOICE_VLAN_PRIORITY:
            priority=SYS_DFLT_ADD_VOICE_VLAN_PORT_PRIORITY;
            break;

        default:
            return CLI_ERR_INTERNAL;

    }

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
      if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
      {
         verify_port = i;

         if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
         {
            display_ethernet_msg(verify_ret, verify_unit, verify_port);
            continue;
         }
         if (ADD_PMGR_SetVoiceVlanPortPriority(lport, priority) == FALSE)
             CLI_LIB_PrintStr("Fail to set voice vlan priority.\r\n");

        }
     }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Switchport_Voice_Vlan_Rule_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ADD_SUPPORT_LLDP == TRUE)
     UI32_T lport;
     UI32_T i;

     UI32_T verify_unit = ctrl_P->CMenu.unit_id;
     UI32_T verify_port;
     CLI_API_EthStatus_T verify_ret;

     for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
     {

         if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
         {
             verify_port = i;

             if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
             {
                 display_ethernet_msg(verify_ret, verify_unit, verify_port);
                 continue;
             }

            switch(cmd_idx)
            {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_SWITCHPORT_VOICE_VLAN_RULE:
                    if (arg[0][0]== 'o' || arg[0][0] == 'O')
                    {
                        if(FALSE == ADD_PMGR_SetVoiceVlanPortOuiRuleState(lport, VAL_voiceVlanPortRuleOui_enabled))
                        {
                            CLI_LIB_PrintStr("Fail to set voice vlan rule.\r\n");
                        }
                    }

                    if (arg[0][0]== 'l' || arg[0][0] == 'L')
                    {
                        if(FALSE == ADD_PMGR_SetVoiceVlanPortLldpRuleState(lport, VAL_voiceVlanPortRuleLldp_enabled))
                        {
                            CLI_LIB_PrintStr("Fail to set voice vlan rule.\r\n");
                        }
                    }
                    break;

                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W5_NO_SWITCHPORT_VOICE_VLAN_RULE:
                    if (arg[0][0]== 'o' || arg[0][0] == 'O')
                    {
                        if(FALSE == ADD_PMGR_SetVoiceVlanPortOuiRuleState(lport, VAL_voiceVlanPortRuleOui_disabled))
                        {
                            CLI_LIB_PrintStr("Fail to set voice vlan rule.\r\n");
                        }
                    }

                    if (arg[0][0]== 'l' || arg[0][0] == 'L')
                    {
                        if(FALSE == ADD_PMGR_SetVoiceVlanPortLldpRuleState(lport, VAL_voiceVlanPortRuleLldp_disabled))
                        {
                            CLI_LIB_PrintStr("Fail to set voice vlan rule.\r\n");
                        }
                    }
                    break;

                default:
				    return CLI_ERR_INTERNAL;
             }
         }
     }
#endif
    return CLI_NO_ERROR;
}

/* command: switchport voice vlan security */
UI32_T CLI_API_Switchport_VoiceVlan_Security_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ADD == TRUE)
    UI32_T lport;
    UI32_T voice_vlan_state;
    UI32_T i;

    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_SWITCHPORT_VOICE_VLAN_SECURITY:
            voice_vlan_state = VAL_voiceVlanPortSecurity_enabled;
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W5_NO_SWITCHPORT_VOICE_VLAN_SECURITY:
            voice_vlan_state = VAL_voiceVlanPortSecurity_disabled;
            break;

        default:
            return CLI_ERR_INTERNAL;

    }

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;

            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }

            if (FALSE == ADD_PMGR_SetVoiceVlanPortSecurityState(lport, voice_vlan_state))
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
                {
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Fail to set voice VLAN port mode on Ethernet %s\r\n", name);
#endif
                }
#else /* CLI_SUPPORT_PORT_NAME */

#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Fail to set voice VLAN security mode on Ethernet %lu/%lu\r\n", verify_unit, verify_port);
#endif

#endif /* CLI_SUPPORT_PORT_NAME */
            }
        }
    }
#endif
  return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_VoiceVlan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ADD == TRUE)
    UI32_T line_num=0;

    if (arg[0][0]== 'o' || arg[0][0] == 'O')
    {
        line_num=CLI_API_ShowOui(line_num);
        if(line_num == JUMP_OUT_MORE)
        {
           return CLI_NO_ERROR;
        }
        else if (line_num == EXIT_SESSION_MORE)
        {
           return CLI_EXIT_SESSION;
        }
    }
    else if (arg[0][0]== 's' || arg[0][0] == 'S')
    {
        line_num=CLI_API_ShowVoiceVlanStatus(line_num);
        if(line_num == JUMP_OUT_MORE)
        {
           return CLI_NO_ERROR;
        }
        else if (line_num == EXIT_SESSION_MORE)
        {
           return CLI_EXIT_SESSION;
        }
    }

#endif /* #if (SYS_CPNT_ADD == TRUE) */
    return CLI_NO_ERROR;
}

#if (SYS_CPNT_ADD == TRUE)
static UI32_T CLI_API_ShowOui(UI32_T line_num)
{
    ADD_MGR_VoiceVlanOui_T entry;
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};

    sprintf(buff, "OUI Address       Mask              Description\r\n");
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, "----------------- ----------------- ------------------------------\r\n");
    PROCESS_MORE_FUNC(buff);

    memset(entry.oui, 255, 6);
    while(TRUE == ADD_PMGR_GetNextOuiEntry(&entry))
    {
        sprintf(buff, "%02X-%02X-%02X-%02X-%02X-%02X %02X-%02X-%02X-%02X-%02X-%02X %s\r\n",
            entry.oui[0],entry.oui[1],entry.oui[2],entry.oui[3],entry.oui[4],entry.oui[5],
            entry.mask[0],entry.mask[1],entry.mask[2],entry.mask[3],entry.mask[4],entry.mask[5],
            entry.description);
        PROCESS_MORE_FUNC(buff);
    }
    CLI_LIB_PrintNullStr(1);
    return line_num;
}

static UI32_T CLI_API_ShowVoiceVlanStatus(UI32_T line_num)
{
    I32_T voice_vlan_id;
    UI32_T timeout;
    UI32_T per_mode;
    UI32_T lport;
    UI32_T per_security_state;
    UI32_T state_lldp, state_oui;
    UI32_T first_line_len;
    I32_T  remain_age;

    UI32_T verify_unit;
    UI32_T verify_port;
    UI32_T i;
    UI32_T max_port_num;
    CLI_API_EthStatus_T verify_ret;
    char  buff[CLI_DEF_MAX_BUFSIZE];
    char  temp_buff[CLI_DEF_MAX_BUFSIZE];

#if (SYS_CPNT_ADD_SUPPORT_LLDP == TRUE)
    UI8_T per_protocol;
#endif

#if (SYS_CPNT_ADD_CONFIGURABLE_PRIORTIY == TRUE)
    UI8_T per_priority;
#endif

    ADD_PMGR_GetVoiceVlanId(&voice_vlan_id);
    ADD_PMGR_GetVoiceVlanAgingTime(&timeout);

    sprintf(buff, "Global Voice VLAN Status\r\n");
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, "Voice VLAN Status     : %s\r\n", (ADD_PMGR_IsVoiceVlanEnabled()==TRUE)?"Enabled":"Disabled");
    PROCESS_MORE_FUNC(buff);

    /* when voice vlan is disabled, maybe it's better
     *  to hide the row of voice vlan id info.
     */
    if (VAL_voiceVlanEnabledId_disabled != voice_vlan_id)
    {
        sprintf(buff, "Voice VLAN ID         : %ld\r\n", voice_vlan_id);
        PROCESS_MORE_FUNC(buff);
    }

    sprintf(buff, "Voice VLAN Aging time : %ld minutes\r\n", timeout);
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, "\r\n");
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, "Voice VLAN Port Summary\r\n");
    PROCESS_MORE_FUNC(buff);

    memset(buff, 0, sizeof(buff));
    memset(temp_buff, 0, sizeof(temp_buff));
    strcat(buff,      "Port     Mode     Security");
    strcat(temp_buff, "-------- ------ --------");

#if (SYS_CPNT_ADD_SUPPORT_LLDP == TRUE)
    strcat(buff,      " Rule     ");
    strcat(temp_buff, " ---------");
#endif

#if (SYS_CPNT_ADD_CONFIGURABLE_PRIORTIY == TRUE)
    strcat(buff,      " Priority");
    strcat(temp_buff, " --------");
#endif

    first_line_len = strlen(buff);

    strcat(buff,      " Remaining Age");
    strcat(temp_buff, " -------------");

    strcat(buff,      "\r\n");
    strcat(temp_buff, "\r\n");

    /* dupm first line
     */
    PROCESS_MORE_FUNC(buff);

    /* second line
     */
    memset(buff, ' ', first_line_len);
    buff[first_line_len] = 0;
    strcat(buff,      "   (minutes)\r\n");

    PROCESS_MORE_FUNC(buff);

    /* dump --- line
     */
    PROCESS_MORE_FUNC(temp_buff);

    for (i=0; STKTPLG_POM_GetNextUnit(&i); )
    {
        max_port_num = SWCTRL_POM_UIGetUnitPortNumber(i);

        verify_unit = i;
        //max_port_num = SWCTRL_UIGetUnitPortNumber(verify_unit);

        for(verify_port = 1; verify_port <= max_port_num; verify_port++)
        {
            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
            if(verify_ret == CLI_API_ETH_NOT_PRESENT)
            {
                continue;
            }

            memset(buff, 0, sizeof(buff));

#if (CLI_SUPPORT_PORT_NAME == 1)
            {
                UI8_T name[MAXSIZE_ifName+1] = {0};
                CLI_LIB_Ifindex_To_Name(lport,name);
                sprintf(temp_buff, "%-12s", name);
             }
#else
            sprintf(temp_buff, "Eth %lu/%2lu", verify_unit, verify_port);
#endif
            strcat(buff, temp_buff);

            ADD_PMGR_GetVoiceVlanPortMode(lport, &per_mode);
            sprintf(temp_buff, " %-6s", (per_mode==VAL_voiceVlanPortMode_auto)?"Auto":(per_mode==VAL_voiceVlanPortMode_manual)?"Manual":"None");
            strcat(buff, temp_buff);

            ADD_PMGR_GetVoiceVlanPortSecurityState(lport, &per_security_state);
            sprintf(temp_buff, " %-8s", (per_security_state==VAL_voiceVlanPortSecurity_enabled)?"Enabled":"Disabled");
            strcat(buff, temp_buff);

#if (SYS_CPNT_ADD_SUPPORT_LLDP == TRUE)
            ADD_PMGR_GetVoiceVlanPortLldpRuleState(lport, &state_lldp);
            ADD_PMGR_GetVoiceVlanPortOuiRuleState(lport, &state_oui);

            if ((state_lldp == VAL_voiceVlanPortRuleLldp_enabled) && (state_oui == VAL_voiceVlanPortRuleOui_enabled))
            {
                per_protocol = ADD_TYPE_DISCOVERY_PROTOCOL_ALL;
            }
            else if ((state_lldp == VAL_voiceVlanPortRuleLldp_enabled) && (state_oui == VAL_voiceVlanPortRuleOui_disabled))
            {
                per_protocol = ADD_TYPE_DISCOVERY_PROTOCOL_LLDP;
            }
            else if ((state_lldp == VAL_voiceVlanPortRuleLldp_disabled) && (state_oui == VAL_voiceVlanPortRuleOui_enabled))
            {
                per_protocol = ADD_TYPE_DISCOVERY_PROTOCOL_OUI;
            }
            else
            {
                per_protocol = ADD_TYPE_DISCOVERY_PROTOCOL_NONE;
            }

            if(per_protocol == ADD_TYPE_DISCOVERY_PROTOCOL_NONE)
                sprintf(temp_buff, " %-9s", " ");
            if(per_protocol == ADD_TYPE_DISCOVERY_PROTOCOL_OUI)
                sprintf(temp_buff, " %-9s", "OUI");
            else if(per_protocol == ADD_TYPE_DISCOVERY_PROTOCOL_LLDP)
                sprintf(temp_buff, " %-9s", "LLDP");
            else
                sprintf(temp_buff, " %-9s", "OUI, LLDP");
            strcat(buff, temp_buff);

#endif

#if (SYS_CPNT_ADD_CONFIGURABLE_PRIORTIY == TRUE)
            ADD_PMGR_GetVoiceVlanPortPriority(lport, &per_priority);
            sprintf(temp_buff, " %8d", per_priority);
            strcat(buff, temp_buff);
#endif

            ADD_PMGR_GetVoiceVlanPortRemainAge(lport, &remain_age);
            if (remain_age < 0)
            {
                sprintf(temp_buff, " %-13s",
                    (remain_age == ADD_TYPE_VOICE_VLAN_ERROR_NA)   ? "NA":
                    (remain_age == ADD_TYPE_VOICE_VLAN_ERROR_NO_START) ? "Not Start"
                                                                        : "??");
                strcat(buff, temp_buff);
            }
            else if (remain_age == 0)
            {
                sprintf(temp_buff, " %13s", "<1");
                strcat(buff, temp_buff);
            }
            else
            {
                sprintf(temp_buff, " %13ld", remain_age);
                strcat(buff, temp_buff);
            }

            strcat(buff, "\r\n");
            PROCESS_MORE_FUNC(buff);
        }
    }

    CLI_LIB_PrintNullStr(1);
    return line_num;
}
#endif  /* #if (SYS_CPNT_ADD == TRUE) */


