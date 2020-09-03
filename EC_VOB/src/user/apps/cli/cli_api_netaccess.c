#include "cli_def.h"
#include "cli_api.h"
#include "cli_lib.h"
#include "cli_tbl.h"
#include "swctrl_pom.h"

#if(SYS_CPNT_NETACCESS==TRUE)
#include "netaccess_pmgr.h"
#endif
#if(SYS_CPNT_PORT_SECURITY==TRUE)
#include "psec_pmgr.h"
#include "amtr_om.h"
#endif

#include "sys_time.h"

#if(SYS_CPNT_NETACCESS==TRUE)
static UI32_T show_one_network_access_entry(UI32_T line_num, UI32_T lport);
static UI32_T show_mac_auth(UI32_T line_num, UI32_T lport);
static UI32_T show_dynamic_vlan(UI32_T line_num, UI32_T lport);
static UI32_T show_dynamic_qos(UI32_T line_num, UI32_T lport);
static UI32_T show_mac_filter_id(UI32_T line_num, UI32_T lport);
static UI32_T show_guest_vlan(UI32_T line_num, UI32_T lport);
static UI32_T show_link_detection(UI32_T line_num, UI32_T lport);
static BOOL_T is_mac_filter(UI8_T got_mac[6], UI8_T keyin_mac[6], UI8_T ignore_mask[6]);

static void unit_port_to_string(UI32_T unit, UI32_T port, UI32_T out_str_size, char *out_str_p);
static void mac_addr_to_string(const UI8_T *mac_p, UI32_T out_str_size, char *out_str_p);
static void ipv4_addr_to_string(UI32_T ipv4_addr, UI32_T out_str_size, char *out_str_p);
static void tick_to_real_time_string(unsigned long tick, unsigned long out_str_size, char *out_str_p);
static void learnt_attr_to_string(BOOL_T learnt, unsigned long out_str_size, char *out_str_p);
#endif

#if(SYS_CPNT_PORT_SECURITY==TRUE)

static UI32_T
input_interface_str_to_ifindex(
    char *type_str_p,
    char *interface_str_p,
    UI32_T *ifindex_p);

static UI32_T show_one_port_security_summary(UI32_T line_num, UI32_T lport);
static UI32_T show_one_port_security_entry(UI32_T line_num, UI32_T lport);
static UI32_T show_psec_enable_status(UI32_T line_num, UI32_T lport);
static UI32_T show_psec_port_state(UI32_T line_num, UI32_T lport);
static UI32_T show_psec_intrusion_action(UI32_T line_num, UI32_T lport);
static UI32_T show_psec_max_mac_count(UI32_T line_num, UI32_T lport);
static UI32_T show_psec_learnt_mac_count(UI32_T line_num, UI32_T lport);
static UI32_T show_psec_mac_filter_id(UI32_T line_num, UI32_T lport);
static UI32_T show_psec_last_intrusion_mac(UI32_T line_num, UI32_T lport);
static UI32_T show_psec_last_intrusion_time(UI32_T line_num, UI32_T lport);
#endif

#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE==TRUE)

#define MAC_FTR_TBL_FILTER_ID           0
#define MAC_FTR_TBL_MAC_ADDRESS         1
#define MAC_FTR_TBL_MAC_MASK            2

CLI_TBL_Temp_T mac_ftr_tbl[] =
{
        {MAC_FTR_TBL_FILTER_ID,     9,  CLI_TBL_ALIGN_RIGHT},
        {MAC_FTR_TBL_MAC_ADDRESS,   17, CLI_TBL_ALIGN_LEFT},
        {MAC_FTR_TBL_MAC_MASK,      17, CLI_TBL_ALIGN_LEFT}
};

static int show_one_mac_filter(
    CLI_TBL_Object_T *tb,
    UI32_T filter_id,
    const UI8_T mac_address[SYS_ADPT_MAC_ADDR_LEN],
    const UI8_T mask_address[SYS_ADPT_MAC_ADDR_LEN]
    );
#endif /* #if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE==TRUE) */

/*
#define PRIVILEGE_CFG_GLOBAL_CMD_W2_NETWORKACCESS_MACFILTER 1
#define PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_NETWORKACCESS_MACFILTER 2
#define PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NETWORKACCESS_PORTMACFILTER 3
#define PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_NETWORKACCESS_PORTMACFILTER 4
#define PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NETWORKACCESS_LINKDETECTION 5
#define PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_NETWORKACCESS_LINKDETECTION 6
*/

UI32_T CLI_API_Mac_Authentication_Reauth_Time(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NETACCESS==TRUE) && (SYS_CPNT_NETACCESS_MACAUTH==TRUE)
    UI32_T reauth_time;

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_MACAUTHENTICATION_REAUTHTIME:
        reauth_time=atoi(arg[0]);
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_MACAUTHENTICATION_REAUTHTIME:
        reauth_time=SYS_DFLT_NETACCESS_SECURE_REAUTH_TIME;
        break;

    default:
        return CLI_ERR_INTERNAL;
    }

    if(FALSE == NETACCESS_PMGR_SetSecureReauthTime(reauth_time))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set MAC_Authentication reauth time\r\n");
#endif
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Mac_Authentication_Intrusion_Action(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NETACCESS==TRUE) && (SYS_CPNT_NETACCESS_MACAUTH==TRUE)
    UI32_T i, action;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        UI32_T lport = 0;
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
               display_ethernet_msg(verify_ret, verify_unit, verify_port);
               continue;
            }
            else
            {
                switch(cmd_idx)
                {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_MACAUTHENTICATION_INTRUSIONACTION:
                    if (arg[0][0] == 'b' || arg[0][0] == 'B')
                    {
                        action = VAL_macAuthPortIntrusionAction_block_traffic;
                    }
                    else
                    {
                        action = VAL_macAuthPortIntrusionAction_pass_traffic;
                    }
                    break;

                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_MACAUTHENTICATION_INTRUSIONACTION:
                    action = SYS_DFLT_NETACCESS_MACAUTH_INTRUSIONACTION_ACTION;
                    break;

                default:
                    return CLI_ERR_INTERNAL;
                }

                if(!NETACCESS_PMGR_SetMacAuthPortIntrusionAction(lport, action))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set the intrusion action for MAC authentication.\r\n");
#endif
                }
            }
        }
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Mac_Authentication_Max_Mac_Count(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NETACCESS==TRUE) && (SYS_CPNT_NETACCESS_MACAUTH==TRUE)
    UI32_T i, count;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        UI32_T lport = 0;
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
               display_ethernet_msg(verify_ret, verify_unit, verify_port);
               continue;
            }
            else
            {
                switch(cmd_idx)
                {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_MACAUTHENTICATION_MAXMACCOUNT:
                    count = atoi(arg[0]);
                    break;

                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_MACAUTHENTICATION_MAXMACCOUNT:
                    count = SYS_DFLT_NETACCESS_MACAUTH_SECURE_ADDRESSES_PER_PORT;
                    break;

                default:
                    return CLI_ERR_INTERNAL;
                }

                if(!NETACCESS_PMGR_SetMacAuthPortMaxMacCount(lport, count))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set the maximum count of the authenticated MAC address.\r\n");
#endif
                }
            }
        }
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Network_Access_Aging(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_NETACCESS==TRUE)
#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
    UI32_T aging_mode;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_NETWORKACCESS_AGING:
            aging_mode = VAL_networkAccessAging_enabled;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_NETWORKACCESS_AGING:
            aging_mode = VAL_networkAccessAging_disabled;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (FALSE == NETACCESS_PMGR_SetMacAddressAgingMode(aging_mode))
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr("Failed to enable MAC address aging.\r\n");
    #endif
    }
#endif /* #if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE) */
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Network_Access_Dynamic_Vlan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN==TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T dynamic_vlan_status;

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        UI32_T lport = 0;
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
        verify_port = i;
            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
               display_ethernet_msg(verify_ret, verify_unit, verify_port);
               continue;
            }
            else
            {
                switch(cmd_idx)
                {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NETWORKACCESS_DYNAMICVLAN:
                    dynamic_vlan_status = VAL_networkAccessPortDynamicVlan_enabled;
                    break;

                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_NETWORKACCESS_DYNAMICVLAN:
                    dynamic_vlan_status = VAL_networkAccessPortDynamicVlan_disabled;
                    break;

                default:
                    return CLI_ERR_INTERNAL;
                }

                if(!NETACCESS_PMGR_SetSecureDynamicVlanStatus( lport, dynamic_vlan_status))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set Network Access Enable Dynamic Vlan\r\n");
#endif
                }
            }
        }
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Network_Access_Dynamic_Qos(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        UI32_T lport = 0;
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
               display_ethernet_msg(verify_ret, verify_unit, verify_port);
               continue;
            }
            else
            {
                switch(cmd_idx)
                {
                    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NETWORKACCESS_DYNAMICQOS:
                    {
                        if(!NETACCESS_PMGR_SetDynamicQosStatus( lport, VAL_networkAccessPortLinkDynamicQos_enabled))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr("Failed to set dynamic QoS assignment\r\n");
#endif
                        }
                    }
                    break;

                    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_NETWORKACCESS_DYNAMICQOS:
                    {
                        if(!NETACCESS_PMGR_SetDynamicQosStatus( lport, VAL_networkAccessPortLinkDynamicQos_disabled))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr("Failed to set dynamic QoS assignment\r\n");
#endif
                        }
                     }
                     break;

                     default:
                         return CLI_ERR_INTERNAL;
                }
            }
        }
    }
#endif /* #if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Network_Access_Guest_Vlan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
    UI32_T i;
    UI32_T max_port_num = 0;
    UI32_T guest_vlan_id;

    max_port_num = SWCTRL_POM_UIGetUnitPortNumber(ctrl_P->CMenu.unit_id);

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NETWORKACCESS_GUESTVLAN:
            guest_vlan_id = atoi(arg[0]);
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_NETWORKACCESS_GUESTVLAN:
            guest_vlan_id = NETACCESS_TYPE_DFLT_GUEST_VLAN_ID;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    for (i = 1; i <= max_port_num; i++)
    {
       if(!SWCTRL_POM_UIUserPortExisting(ctrl_P->CMenu.unit_id, i))
          continue;

       if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
       {
          if(NETACCESS_PMGR_SetSecureGuestVlanId(i, guest_vlan_id)!=TRUE)
          {
#if (SYS_CPNT_EH == TRUE)
             CLI_API_Show_Exception_Handeler_Msg();
#else
             CLI_LIB_PrintStr_1("Fail to set the guest VLAN on port %lu.\r\n",(unsigned long)i);
#endif
          }
       }
    }
#endif /* SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE */
    return CLI_NO_ERROR;
}


UI32_T CLI_API_Network_Access_Link_Detection(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;

    UI32_T link_detection_mode = SYS_DFLT_NETACCESS_LINK_DETECTION_MODE;
    UI32_T link_detection_action = SYS_DFLT_NETACCESS_LINK_DETECTION_ACTION;

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        UI32_T lport = 0;
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
               display_ethernet_msg(verify_ret, verify_unit, verify_port);
               continue;
            }

            /*
             network-access link-detection [{$mode} action {$action}]
             no network-access link-detection
             */
            switch(cmd_idx)
            {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NETWORKACCESS_LINKDETECTION:
                {
                    if (arg[0] != NULL)
                    {
                        /* network-access link-detection link-xxx action yyy */
                        /*if (arg[1][0] == 'a' || arg[1][0] == 'A')
                        {*/
                            /* link-up or link-up-down */
                            if ((arg[0][5] == 'u' || arg[0][5] == 'U'))
                            {
                                if (arg[0][7] == '\0')
                                {
                                    link_detection_mode = VAL_networkAccessPortLinkDetectionMode_linkUp;
                                }
                                else
                                {
                                    link_detection_mode = VAL_networkAccessPortLinkDetectionMode_linkUpDown;
                                }
                            }
                            /* link-down */
                            else if (arg[0][5] == 'd' || arg[0][5] == 'D')
                            {
                                link_detection_mode = VAL_networkAccessPortLinkDetectionMode_linkDown;
                            }
                            else
                                return CLI_ERR_INTERNAL;

                            /* shutdown */
                            if (arg[2][0] == 's' || arg[2][0] == 'S')
                            {
                                link_detection_action = VAL_networkAccessPortLinkDetectionAciton_shutdown;
                            }
                            /* trap or trap-and-shutdown */
                            else if ((arg[2][0] == 't') || (arg[2][0] == 'T'))
                            {
                                if (arg[2][4] == '\0')
                                {
                                    link_detection_action = VAL_networkAccessPortLinkDetectionAciton_trap;
                                }
                                else
                                {
                                    link_detection_action = VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown;
                                }
                            }
                            else
                                return CLI_ERR_INTERNAL;

                            if((NETACCESS_PMGR_SetLinkDetectionMode( lport, link_detection_mode) != TRUE)||
                               (NETACCESS_PMGR_SetLinkDetectionAction( lport, link_detection_action) != TRUE))
                            {
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr("Failed to set link detection\r\n");
#endif
                            }

                        /*}
                        else
                            return CLI_ERR_INTERNAL;*/
                    }
                    else    /* network-access link-detection */
                    {
                        if(!NETACCESS_PMGR_SetLinkDetectionStatus( lport, VAL_networkAccessPortLinkDetectionStatus_enabled))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr("Failed to set link detection\r\n");
#endif
                        }
                    }
                }
                break;

                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_NETWORKACCESS_LINKDETECTION:
                {
                    if(!NETACCESS_PMGR_SetLinkDetectionStatus( lport, VAL_networkAccessPortLinkDetectionStatus_disabled))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to set link detection\r\n");
#endif
                    }
                }
                break;

                default:
                    return CLI_ERR_INTERNAL;
            }

        }
    }
#endif /* #if(SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Network_Access_Max_Mac_Count_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_NETACCESS_MACAUTH==TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T number;

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        UI32_T lport = 0;
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
        verify_port = i;
            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
               display_ethernet_msg(verify_ret, verify_unit, verify_port);
               continue;
            }
            else
            {
                switch(cmd_idx)
                {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NETWORKACCESS_MAXMACCOUNT:
                    number=atoi(arg[0]);
                    break;

                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_NETWORKACCESS_MAXMACCOUNT:
                    number=SYS_DFLT_NETACCESS_SECURE_ADDRESSES_PER_PORT;
                    break;

                default:
                    return CLI_ERR_INTERNAL;
                }

                if(!NETACCESS_PMGR_SetSecureNumberAddresses(lport, number))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set Network access Max MAC count\r\n");
#endif
                }
            }
        }
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Network_Access_Mode_Mac_Authentication(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NETACCESS==TRUE) && (SYS_CPNT_NETACCESS_MACAUTH==TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T mac_auth_status;

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        UI32_T lport = 0;
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
               display_ethernet_msg(verify_ret, verify_unit, verify_port);
               continue;
            }
            else
            {
                switch(cmd_idx)
                {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NETWORKACCESS_MODE:
                    if (arg[0][0] == 'm' || arg[0][0] == 'M')
                        mac_auth_status = NETACCESS_TYPE_MACAUTH_ENABLED;
                    else
                        return CLI_ERR_INTERNAL;
                break;

                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_NETWORKACCESS_MODE:
                    if (arg[0][0] == 'm' || arg[0][0] == 'M')
                        mac_auth_status = NETACCESS_TYPE_MACAUTH_DISABLED;
                    else
                        return CLI_ERR_INTERNAL;
                break;

                default:
                    return CLI_ERR_INTERNAL;
                }

                if(FALSE == NETACCESS_PMGR_SetMacAuthPortStatus(lport, mac_auth_status))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to specify secure port mode.\r\n");
#endif
                }
            }
        }
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Network_Access_Mac_Filter(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE==TRUE)
    UI32_T filter_id;
    UI8_T mac_address[SYS_ADPT_MAC_ADDR_LEN] = {0};
    UI8_T mask[SYS_ADPT_MAC_ADDR_LEN] = {0};
    UI8_T default_mask[SYS_ADPT_MAC_ADDR_LEN] = {0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF};
    BOOL_T is_add;

    filter_id = atoi(arg[0]);
    CLI_LIB_ValsInMac(arg[2], mac_address);

    if (arg[3] != NULL)
    {
        CLI_LIB_ValsInMac(arg[4], mask);
    }
    else
    {
        memcpy(mask, default_mask, SYS_ADPT_MAC_ADDR_LEN);
    }


    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_NETWORKACCESS_MACFILTER:
        is_add=TRUE;
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_NETWORKACCESS_MACFILTER:
        is_add=FALSE;
        break;

    default:
        return CLI_ERR_INTERNAL;
    }

    if(!NETACCESS_PMGR_SetFilterMac( filter_id, mac_address, mask, is_add))
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        if (is_add == TRUE)
        {
            CLI_LIB_PrintStr("Failed to add MAC address into MAC filter table\r\n");
        }
        else
        {
            CLI_LIB_PrintStr("Failed to remove MAC address from MAC filter table\r\n");
        }
    #endif
    }
#endif /* #if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE==TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Network_Access_Port_Mac_Filter(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE==TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T filter_id;

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        UI32_T lport = 0;
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
               display_ethernet_msg(verify_ret, verify_unit, verify_port);
               continue;
            }
            else
            {
                switch(cmd_idx)
                {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NETWORKACCESS_PORTMACFILTER:
                    filter_id=atoi(arg[0]);
                    break;

                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_NETWORKACCESS_PORTMACFILTER:
                    filter_id=0;
                    break;

                default:
                    return CLI_ERR_INTERNAL;
                }

                if(!NETACCESS_PMGR_SetFilterIdToPort(lport,filter_id))
                {
                #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
                #else
                    CLI_LIB_PrintStr("Failed to enable MAC filter\r\n");
                #endif
                }
            }
        }
    }
#endif /* #if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE==TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Clear_Network_Access_Mac_Address_Table(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_NETACCESS==TRUE)
    UI32_T to_detect_position   = 0;
    UI32_T lport       =  0;
    UI32_T interface_position = 0;
    NETACCESS_MGR_SecureAddressFilter_T in_filter;
    UI8_T  mac_addr[6] = {0};

    memset(&in_filter,0,sizeof(NETACCESS_MGR_SecureAddressFilter_T));

    in_filter.sort=NETACCESS_ADDRESS_ENTRY_SORT_ADDRESS;
    in_filter.type=NETACCESS_ADDRESS_ENTRY_TYPE_ALL;

    /*static or dynamic*/
    if ((arg[to_detect_position] != NULL) &&
        ((arg[to_detect_position][0] == 's') || (arg[to_detect_position][0] == 'S')))
    {
        to_detect_position += 1;
        in_filter.type= NETACCESS_ADDRESS_ENTRY_TYPE_STATIC;
    }
    else if((arg[to_detect_position] != NULL) &&
        ((arg[to_detect_position][0] == 'd') || (arg[to_detect_position][0] == 'D')))
    {
        to_detect_position += 1;
        in_filter.type= NETACCESS_ADDRESS_ENTRY_TYPE_DYNAMIC;
    }

    /*address*/
    if ((arg[to_detect_position] != NULL) &&
        ((arg[to_detect_position][0]=='a') || (arg[to_detect_position][0]=='A' )))
    {
        to_detect_position    += 1;
        CLI_LIB_ValsInMac(arg[to_detect_position], mac_addr);
        memcpy(in_filter.mac,mac_addr,sizeof(mac_addr));
    }


    /*interface*/
    if ((arg[to_detect_position] != NULL) &&
        ((arg[to_detect_position][0]=='i') || (arg[to_detect_position][0]=='I')))
    {
        to_detect_position++;
        interface_position  = to_detect_position;

        /*ethernet*/
        if ((arg[to_detect_position] != NULL) &&
            ((arg[to_detect_position][0] == 'e') || (arg[to_detect_position][0] == 'E')))
        {
            UI32_T verify_unit = atoi(arg[interface_position+1]);
            UI32_T verify_port = atoi(strchr(arg[interface_position+1],'/')+1);
            CLI_API_EthStatus_T verify_ret;

            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                return CLI_NO_ERROR;
            }

            in_filter.sort=NETACCESS_ADDRESS_ENTRY_SORT_INTERFACE;
            in_filter.lport=lport;
        }
    }


    if(!NETACCESS_PMGR_ClearSecureAddressEntryByFilter(&in_filter))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to Clear Network Access Mac Address Table\r\n");
#endif
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Port_Security_MacAddressAsPermanent(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)

    UI32_T ifindex;

    /* specified interface
     */
    if (NULL != arg[0])
    {
        if (CLI_NO_ERROR !=
            input_interface_str_to_ifindex(arg[1], arg[2], &ifindex))
        {
            return CLI_ERR_INTERNAL;
        }
    }
    /*  all interface
     */
    else
    {
        ifindex = PSEC_MGR_INTERFACE_INDEX_FOR_ALL;
    }

#if (SYS_CPNT_NETACCESS == TRUE)
    if (TRUE != NETACCESS_PMGR_ConvertPortSecuritySecuredAddressIntoManual(ifindex))
#else
    if (TRUE != PSEC_PMGR_ConvertSecuredAddressIntoManual(ifindex))
#endif
    {
        CLI_LIB_PrintStr("Failed to convert the learnt secured MAC address into manual configured.\r\n");
    }

 #endif /* #if (SYS_CPNT_PORT_SECURITY == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Port_Security(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_PORT_SECURITY==TRUE)
    UI32_T Max_port;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;/* wrong unit id*/
    UI32_T verify_port;
    UI32_T lport = 0;
    UI32_T line_num=0;

#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
    UI32_T aging_mode;
#endif

    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};

    PROCESS_MORE("Global Port Security Parameters\r\n");


    memset(buff,0,sizeof(buff));
#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
    if (FALSE == NETACCESS_PMGR_GetMacAddressAgingMode(&aging_mode))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to get MAC address aging mode\r\n");
#endif
        return CLI_NO_ERROR;
    }
    sprintf(buff," Secure MAC Aging Mode : %s\r\n", (aging_mode == VAL_networkAccessAging_enabled) ? "Enabled" : "Disabled");
    PROCESS_MORE(buff);
    memset(buff,0,sizeof(buff));
#endif

    if(arg[0]==NULL)
    {
         PROCESS_MORE("\r\nPort Security Port Summary\r\n");
         PROCESS_MORE(" Port     Port Security Port Status Intrusion Action  MaxMacCnt CurrMacCnt\r\n");
         PROCESS_MORE(" -------- ------------- ----------- ----------------  --------- ----------\r\n");

         /*for(j = 1; j <= current_max_unit; j++)*/
         for (verify_unit=0; STKTPLG_POM_GetNextUnit(&verify_unit); )
         {
            Max_port = SWCTRL_POM_UIGetUnitPortNumber(verify_unit);
            /*eth*/
            for (verify_port = 1 ; verify_port <= Max_port;verify_port++)
            {
               if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                {
                   display_ethernet_msg(verify_ret, verify_unit, verify_port);
                   continue;
                }
                else
                {

                    if((line_num = show_one_port_security_summary(line_num,lport)) == JUMP_OUT_MORE)
                    {
                        return CLI_NO_ERROR;
                    }
                    else if (line_num == EXIT_SESSION_MORE)
                    {
                        return CLI_EXIT_SESSION;
                    }
                }
            }
        }

    }
    else if(arg[1][0]=='e' || arg[1][0]=='E')
    {
        verify_unit = atoi(arg[2]);
        verify_port = atoi( strchr(arg[2], '/')+1 );

        if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
        {
           display_ethernet_msg(verify_ret, verify_unit, verify_port);
        }
        else
        {
            PROCESS_MORE("\r\nPort Security Details\r\n");

            if((line_num = show_one_port_security_entry(line_num,lport)) == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
            else if (line_num == EXIT_SESSION_MORE)
            {
                return CLI_EXIT_SESSION;
            }
        }
    }
#endif
    return CLI_NO_ERROR;
}

#if(SYS_CPNT_PORT_SECURITY==TRUE)
static UI32_T
input_interface_str_to_ifindex(
    char *type_str_p,
    char *interface_str_p,
    UI32_T *ifindex_p)
{
    if (NULL == type_str_p ||
        NULL == interface_str_p)
    {
        return CLI_ERR_INTERNAL;
    }

    switch (type_str_p[0])
    {
        /* ethernet
         */
        case 'e':
        case 'E':
        {
            UI32_T verify_unit = 0;
            UI32_T verify_port = 0;
            CLI_API_EthStatus_T verify_ret;

            if (TRUE != CLI_LIB_GetUnitPortFromString(interface_str_p, &verify_unit, &verify_port))
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
                UI32_T trunk_id = 0;

                if (TRUE != IF_PMGR_IfnameToIfindex(interface_str_p, &ifindex))
                {
                    CLI_LIB_PrintStr_1("%s does not exist\r\n",interface_str_p);
                    return CLI_NO_ERROR;
                }

                SWCTRL_POM_LogicalPortToUserPort(ifindex, &verify_unit, &verify_port, &trunk_id);
            }
#else
            {
                return CLI_ERR_INTERNAL;
            }
#endif

            if ((verify_ret = verify_ethernet(verify_unit, verify_port, ifindex_p)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                return CLI_NO_ERROR;
            }
        }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    return CLI_NO_ERROR;
}

static UI32_T show_one_port_security_summary(UI32_T line_num, UI32_T lport)
{
    enum SHOW_STATUS
    {
        SHOW_STATUS_UNKNOWN,
        SHOW_STATUS_SECUREUP,
        SHOW_STATUS_SECUREDOWN,
        SHOW_STATUS_SHUTDOWN
    };
	
    UI32_T unit;
    UI32_T port;
    UI32_T trunk_id;
    UI32_T psec_enable_status=0,psec_port_state=0,psec_action=0,mac_count=0,learnt_count=0,action_active=0;
    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};
    char *str_ary_enable_status[] =
          {
              "Unknown",
              "Enabled",
              "Disabled"
          };
    char *str_ary_port_state[] =
          {
              "Unknown",
              "Secure/Up",
              "Secure/Down",
              "Shutdown"
          };
    char *str_ary_action[] =
          {
              "Unknown",
              "None",
              "Trap",
              "Shutdown",
              "Trap-and-Shutdown"
          };
	
    /* Port : 1/1
     */
    SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);
    PSEC_PMGR_GetPortSecurityStatus(lport,&psec_enable_status);
    PSEC_PMGR_GetPortSecurityActionStatus(lport,&psec_action);
    PSEC_PMGR_GetPortSecurityMacCount(lport,&mac_count);
    learnt_count = AMTR_OM_GetSecurityCounterByport(lport);
    PSEC_PMGR_GetPortSecurityActionActive(lport,&action_active);
    if (psec_enable_status==VAL_portSecPortStatus_enabled)
    {
        psec_port_state = SHOW_STATUS_SECUREUP;
        if( action_active==TRUE &&
		(psec_action==VAL_portSecAction_shutdown||psec_action==VAL_portSecAction_trapAndShutdown))
            psec_port_state = SHOW_STATUS_SHUTDOWN;
    }
    else
    {
        psec_port_state = SHOW_STATUS_SECUREDOWN;
    }


	
    sprintf(buff, " Eth %lu/%2lu %-13s %-11s %-17s %9lu %10lu\r\n",
		                (unsigned long)unit,(unsigned long)port,
		                str_ary_enable_status[psec_enable_status], 
		                str_ary_port_state[psec_port_state], str_ary_action[psec_action],
		                (unsigned long)mac_count,(unsigned long)learnt_count);
    PROCESS_MORE_FUNC(buff);

    return line_num;
}

static UI32_T show_one_port_security_entry(UI32_T line_num, UI32_T lport)
{
    UI32_T unit;
    UI32_T port;
    UI32_T trunk_id;

    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};

    /* Port : 1/1
     */
    SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);
    sprintf(buff, " Port                                  : %lu/%lu\r\n", (unsigned long)unit, (unsigned long)port);
    PROCESS_MORE_FUNC(buff);

    line_num = show_psec_enable_status(line_num, lport);
    if (line_num == EXIT_SESSION_MORE || line_num == JUMP_OUT_MORE)
    {
        return line_num;
    }

    line_num = show_psec_port_state(line_num, lport);
    if (line_num == EXIT_SESSION_MORE || line_num == JUMP_OUT_MORE)
    {
        return line_num;
    }

    line_num = show_psec_intrusion_action(line_num, lport);
    if (line_num == EXIT_SESSION_MORE || line_num == JUMP_OUT_MORE)
    {
        return line_num;
    }

    line_num = show_psec_max_mac_count(line_num, lport);
    if (line_num == EXIT_SESSION_MORE || line_num == JUMP_OUT_MORE)
    {
        return line_num;
    }

    line_num = show_psec_learnt_mac_count(line_num, lport);
    if (line_num == EXIT_SESSION_MORE || line_num == JUMP_OUT_MORE)
    {
        return line_num;
    }

    line_num = show_psec_mac_filter_id(line_num, lport);
    if (line_num == EXIT_SESSION_MORE || line_num == JUMP_OUT_MORE)
    {
        return line_num;
    }

    line_num = show_psec_last_intrusion_mac(line_num, lport);
    if (line_num == EXIT_SESSION_MORE || line_num == JUMP_OUT_MORE)
    {
        return line_num;
    }

    line_num = show_psec_last_intrusion_time(line_num, lport);
    if (line_num == EXIT_SESSION_MORE || line_num == JUMP_OUT_MORE)
    {
        return line_num;
    }
	
    return line_num;
}

static UI32_T show_psec_enable_status(UI32_T line_num, UI32_T lport)
{

    UI32_T psec_enable_status=VAL_portSecPortStatus_disabled;
    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};
    char *str_ary_enable_status[] =
          {
              "Unknown",
              "Enabled",
              "Disabled"
          };

    if (!PSEC_PMGR_GetPortSecurityStatus(lport,&psec_enable_status))
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr("Failed to get Port Security Enable Status\r\n");
    #endif
        return line_num;
    }
    
    sprintf(buff," Port Security                         : %s\r\n", str_ary_enable_status[psec_enable_status]);

    PROCESS_MORE_FUNC(buff);
    memset(buff,0,sizeof(buff));
		
    return line_num;
}

static UI32_T show_psec_port_state(UI32_T line_num, UI32_T lport)
{
    enum SHOW_STATUS
    {
        SHOW_STATUS_UNKNOWN,
        SHOW_STATUS_SECUREUP,
        SHOW_STATUS_SECUREDOWN,
        SHOW_STATUS_SHUTDOWN
    };
	
    UI32_T psec_port_state,psec_enable_status,psec_action,action_active;
    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};
    char *str_ary_port_state[] =
          {
              "Unknown",
              "Secure/Up",
              "Secure/Down",
              "Shutdown"
          };


    if (!PSEC_PMGR_GetPortSecurityStatus(lport,&psec_enable_status) ||
		!PSEC_PMGR_GetPortSecurityActionStatus(lport,&psec_action)||
		!PSEC_PMGR_GetPortSecurityActionActive(lport,&action_active))
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr("Failed to get Port Security Port Status\r\n");
    #endif
        return line_num;
    }

    if (psec_enable_status==VAL_portSecPortStatus_enabled)
    {
        psec_port_state = SHOW_STATUS_SECUREUP;
        if( action_active==TRUE &&
		(psec_action==VAL_portSecAction_shutdown||psec_action==VAL_portSecAction_trapAndShutdown))
            psec_port_state = SHOW_STATUS_SHUTDOWN;
    }
    else
    {
        psec_port_state = SHOW_STATUS_SECUREDOWN;
    }
    
    sprintf(buff," Port Status                           : %s\r\n", str_ary_port_state[psec_port_state]);

    PROCESS_MORE_FUNC(buff);
    memset(buff,0,sizeof(buff));
		
    return line_num;
}

static UI32_T show_psec_intrusion_action(UI32_T line_num, UI32_T lport)
{

    UI32_T psec_action;
    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};
    char *str_ary_action[] =
          {
              "Unknown",
              "None",
              "Trap",
              "Shutdown",
              "Trap-and-Shutdown"
          };

    if (!PSEC_PMGR_GetPortSecurityActionStatus(lport,&psec_action))
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr("Failed to get Port Security Intrusion Action\r\n");
    #endif
        return line_num;
    }
    
    sprintf(buff," Intrusion Action                      : %s\r\n", str_ary_action[psec_action]);

    PROCESS_MORE_FUNC(buff);
    memset(buff,0,sizeof(buff));
		
    return line_num;
}

static UI32_T show_psec_max_mac_count(UI32_T line_num, UI32_T lport)
{

    UI32_T mac_count;
    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};

    if (!PSEC_PMGR_GetPortSecurityMacCount(lport,&mac_count))
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr("Failed to get Port Security Max MAC Count\r\n");
    #endif
        return line_num;
    }
    
    sprintf(buff," Max MAC Count                         : %lu\r\n", (unsigned long)mac_count);

    PROCESS_MORE_FUNC(buff);
    memset(buff,0,sizeof(buff));
		
    return line_num;
}

static UI32_T show_psec_learnt_mac_count(UI32_T line_num, UI32_T lport)
{

    UI32_T mac_count=0;
    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};

    mac_count  = AMTR_OM_GetSecurityCounterByport(lport);
    
    sprintf(buff," Current MAC Count                     : %lu\r\n", (unsigned long)mac_count);

    PROCESS_MORE_FUNC(buff);
    memset(buff,0,sizeof(buff));
		
    return line_num;
}

static UI32_T show_psec_mac_filter_id(UI32_T line_num, UI32_T lport)
{
#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE)
    UI32_T filter_id;
    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};

    /*MAC Filter applied & MAC filter id*/
    /*if (NETACCESS_PMGR_IsPortBindFilterId(lport))
    {*/
        if (!NETACCESS_PMGR_GetFilterIdOnPort(lport,&filter_id))
        {
        #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
        #else
            CLI_LIB_PrintStr("Failed to get Filter Id\r\n");
        #endif
            return line_num;
        }
        if(filter_id!=0)
        {
            sprintf(buff," MAC Filter                            : Enabled\r\n");
            PROCESS_MORE_FUNC(buff);
            memset(buff,0,sizeof(buff));
			
            sprintf(buff," MAC Filter ID                         : %ld\r\n",(long)filter_id);
            PROCESS_MORE_FUNC(buff);
            memset(buff,0,sizeof(buff));
        }
        else
        {
            PROCESS_MORE_FUNC(" MAC Filter                            : Disabled\r\n");
        }
    /*}
    else
    {
        PROCESS_MORE_FUNC("MAC Filter ID                         : Disabled\r\n");
    }*/
#endif /* SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE */

    return line_num;
}

static UI32_T show_psec_last_intrusion_mac(UI32_T line_num, UI32_T lport)
{

    UI8_T mac[SYS_ADPT_MAC_ADDR_LEN];
    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};

    if (!PSEC_PMGR_GetPortSecurityLastIntrusionMac(lport,mac))
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr("Failed to get Last Intrusion MAC\r\n");
    #endif
        return line_num;
    }

    if (mac[0]==0&&mac[1]==0&&mac[2]==0&&mac[3]==0&&mac[4]==0&&mac[5]==0)
        sprintf(buff," Last Intrusion MAC                    : NA\r\n");
    else
        sprintf(buff," Last Intrusion MAC                    : %02X-%02X-%02X-%02X-%02X-%02X\r\n",
                                                                                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    PROCESS_MORE_FUNC(buff);
    memset(buff,0,sizeof(buff));
		
    return line_num;
}

static UI32_T show_psec_last_intrusion_time(UI32_T line_num, UI32_T lport)
{

    UI32_T seconds=0;
    int   year, month, day, hour, minute, second;
    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};

    if (!PSEC_PMGR_GetPortSecurityLastIntrusionTime(lport,&seconds))
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr("Failed to get Last Intrusion Time\r\n");
    #endif
        return line_num;
    }

    SYS_TIME_ConvertSecondsToDateTime(seconds, &year, &month, &day, &hour, &minute, &second);
    if (seconds==0)
        sprintf(buff," Last Time Detected Intrusion MAC      : NA\r\n");
    else
        sprintf(buff," Last Time Detected Intrusion MAC      : %d/%d/%d %d:%d:%d\r\n",
                                                                                year, month, day, hour, minute, second);

    PROCESS_MORE_FUNC(buff);
    memset(buff,0,sizeof(buff));
		
    return line_num;
}

#endif

UI32_T CLI_API_Show_Network_Access(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_NETACCESS==TRUE)
    UI32_T Max_port;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;/* wrong unit id*/
    UI32_T verify_port;
    UI32_T lport = 0;
    UI32_T line_num=0;

#if (SYS_CPNT_NETACCESS_MACAUTH == TRUE)
    UI32_T reauth_time;
#endif  /* #if (SYS_CPNT_NETACCESS_MACAUTH == TRUE) */

#if (SYS_CPNT_NETACCESS_MACAUTH_AUTH_AGE == TRUE)
    UI32_T aging;
#endif

#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
    UI32_T aging_mode;
#endif

    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};

    PROCESS_MORE("Global secure port information\r\n");

#if (SYS_CPNT_NETACCESS_MACAUTH == TRUE)
    /*Reauth Time*/
    if (!NETACCESS_PMGR_GetSecureReauthTime(&reauth_time))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to get Reauth Time\r\n");
#endif
        return CLI_NO_ERROR;
    }

    sprintf(buff,"Reauthentication Time                 : %ld\r\n",(long)reauth_time);
    PROCESS_MORE(buff);
#endif  /* #if (SYS_CPNT_NETACCESS_MACAUTH == TRUE) */

    memset(buff,0,sizeof(buff));

#if (SYS_CPNT_NETACCESS_MACAUTH_AUTH_AGE == TRUE)
    /*Aging Time*/
    if (!NETACCESS_PMGR_GetSecureAuthAge(&aging))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to get address table aging time\r\n");
#endif
        return CLI_NO_ERROR;
    }

    sprintf(buff,"Authenticated Age                     : %ld\r\n",(long)aging);
    PROCESS_MORE(buff);
    memset(buff,0,sizeof(buff));
#endif /* SYS_CPNT_NETACCESS_MACAUTH_AUTH_AGE == TRUE */

#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
    if (FALSE == NETACCESS_PMGR_GetMacAddressAgingMode(&aging_mode))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to get MAC address aging mode\r\n");
#endif
        return CLI_NO_ERROR;
    }
    sprintf(buff,"MAC Address Aging                     : %s\r\n", (aging_mode == VAL_networkAccessAging_enabled) ? "Enabled" : "Disabled");
    PROCESS_MORE(buff);
    memset(buff,0,sizeof(buff));
#endif

    if(arg[0]==NULL)
    {
         /*for(j = 1; j <= current_max_unit; j++)*/
         for (verify_unit=0; STKTPLG_POM_GetNextUnit(&verify_unit); )
         {
            Max_port = SWCTRL_POM_UIGetUnitPortNumber(verify_unit);
            /*eth*/
            for (verify_port = 1 ; verify_port <= Max_port;verify_port++)
            {
               if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                {
                   display_ethernet_msg(verify_ret, verify_unit, verify_port);
                   continue;
                }
                else
                {
                    PROCESS_MORE("\r\n");

                    if((line_num = show_one_network_access_entry(line_num,lport)) == JUMP_OUT_MORE)
                    {
                        return CLI_NO_ERROR;
                    }
                    else if (line_num == EXIT_SESSION_MORE)
                    {
                        return CLI_EXIT_SESSION;
                    }
                }
            }
        }

    }
    else if(arg[1][0]=='e' || arg[1][0]=='E')
    {
        verify_unit = atoi(arg[2]);
        verify_port = atoi( strchr(arg[2], '/')+1 );

        if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
        {
           display_ethernet_msg(verify_ret, verify_unit, verify_port);
        }
        else
        {
            PROCESS_MORE("\r\n");

            if((line_num = show_one_network_access_entry(line_num,lport)) == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
            else if (line_num == EXIT_SESSION_MORE)
            {
                return CLI_EXIT_SESSION;
            }
        }
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Network_Access_Mac_Address_Table(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_NETACCESS==TRUE)

    enum
    {
        ETH_PORT_STR_SIZE = 9,
        MAC_STR_SIZE = 17,
        IPV4_STR_SIZE = 15,
        TIME_STR_SIZE = 25,
        ATTR_STR_SIZE = 9
    };

    UI32_T to_detect_position   = 0;
    UI32_T interface_position   = 0;
    UI32_T mac_addr_position    = 0;
    UI32_T mac_netmask_position = 0;
    UI32_T sort_method_position = 0;
    UI32_T lport       =  0;
    UI32_T line_num=0;
    UI32_T used_buffer;
    NETACCESS_MGR_SecureAddressEntryKeyFilter_T entry_key;
    NETACCESS_MGR_SecureAddressEntry_T entry;
    UI8_T  mac_addr[6] = {0};
    UI8_T  mac_mask[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; /*didn't specify mac addr, mac mask => display all*/
    char eth_ach[ETH_PORT_STR_SIZE + 1];
    char mac_ach[MAC_STR_SIZE + 1];
    char ipv4_ach[IPV4_STR_SIZE + 1];
    char time_ach[TIME_STR_SIZE + 1];
    char attr_ach[ATTR_STR_SIZE + 1];
    BOOL_T is_specify_mac_addr = FALSE;
    char   buff[CLI_DEF_MAX_BUFSIZE]       = {0};

    memset(&entry_key,0,sizeof(NETACCESS_MGR_SecureAddressEntryKeyFilter_T));
    memset(&entry,0,sizeof(NETACCESS_MGR_SecureAddressEntry_T));

    entry_key.filter.type=NETACCESS_ADDRESS_ENTRY_TYPE_ALL;
    entry_key.filter.sort=NETACCESS_ADDRESS_ENTRY_SORT_ADDRESS;

    /* format: show network-access mac-address-table
     *              [static | dynamic]    [address hw-addr [mask-addr]]
     *              [interface interface] [sort {address | interface}]
     */

    /* static or dynamic
     */
    if (NULL != arg[to_detect_position])
    {
        if((arg[to_detect_position][1] == 't') || (arg[to_detect_position][1] == 'T'))
        {
            entry_key.filter.type= NETACCESS_ADDRESS_ENTRY_TYPE_STATIC;
            /*pttch 92.01.08 use the first specify with primary key*/
            to_detect_position += 1;
        }
        else if((arg[to_detect_position][0] == 'd') || (arg[to_detect_position][0] == 'D'))
        {
            entry_key.filter.type= NETACCESS_ADDRESS_ENTRY_TYPE_DYNAMIC;
            /*pttch 92.01.08 use the first specify with primary key*/
            to_detect_position += 1;
        }
    }

    /* specify address
     */
    if (NULL != arg[to_detect_position])
    {
        if ((arg[to_detect_position][0] == 'a') || (arg[to_detect_position][0] == 'A'))
        {
            /*pttch 92.01.08 use the first specify with primary key*/
            to_detect_position += 1;
            /*MAC address and mask*/
            if(arg[to_detect_position])
            {
               if(CLI_LIB_ValsInMac(arg[to_detect_position], mac_addr)) /*check MAC format*/
               {
               	  mac_addr_position      =  to_detect_position;
                  to_detect_position    += 1;
                  is_specify_mac_addr = TRUE; /*just get mac_addr*/
                  if(arg[to_detect_position] != NULL && CLI_LIB_ValsInMac(arg[to_detect_position], mac_mask)) /*check MAC format*/
                  {
                     mac_netmask_position      = to_detect_position;
                     to_detect_position       += 1;
                     is_specify_mac_addr = FALSE; /*must check union of mac_addr and mac_mask*/
                  }
                  else
                     memset(mac_mask, 0, sizeof(mac_mask));/*specify MAC address, check all => don't by pass any one*/
               }
               else
                  memset(mac_mask, 0xff, sizeof(mac_mask)); /*no specified MAC address, ignore all => by pass all*/
            }
        }
    }

    /* interface
     */
    if (NULL != arg[to_detect_position])
    {
        if(arg[to_detect_position][0]=='i' || arg[to_detect_position][0]=='I')
        {
            if((arg[to_detect_position+1][0] == 'e') || (arg[to_detect_position+1][0] == 'E'))
            {
               to_detect_position++;
               interface_position  = to_detect_position;
               to_detect_position += 2;

               if((arg[interface_position][0] == 'e') || (arg[interface_position][0] == 'E')) /*interface*/
               {
                  UI32_T verify_unit = atoi(arg[interface_position+1]);
                  UI32_T verify_port = atoi(strchr(arg[interface_position+1],'/')+1);
                  CLI_API_EthStatus_T verify_ret;

                  if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                  {
                      display_ethernet_msg(verify_ret, verify_unit, verify_port);
                      return CLI_NO_ERROR;
                  }

                  entry_key.filter.lport = lport;
               }
            }
        }
    }

    /* sort method
     */
    if (NULL != arg[to_detect_position])
    {
        if(arg[to_detect_position][1]=='o' || arg[to_detect_position][1]=='O')
        {
           sort_method_position = to_detect_position + 1;

           switch(arg[sort_method_position][0])
           {
           case 'a':
           case 'A':
              entry_key.filter.sort = NETACCESS_ADDRESS_ENTRY_SORT_ADDRESS;
              break;

           case 'i':
           case 'I':
              entry_key.filter.sort = NETACCESS_ADDRESS_ENTRY_SORT_INTERFACE;
              break;

           }
        }
    }

    PROCESS_MORE("Interface MAC Address       RADIUS Server   Time                      Attribute\r\n");
    PROCESS_MORE("--------- ----------------- --------------- ------------------------- ---------\r\n");

    if (is_specify_mac_addr == TRUE)/*get specify_mac_addr*/
    {
        memcpy(entry_key.mac_address, mac_addr, sizeof(mac_addr));
        memcpy(entry_key.filter.mac, mac_addr, sizeof(mac_addr));
    }

    while(0 == NETACCESS_PMGR_GetNextSecureAddressEntryByFilter(SYS_TYPE_FID_ALL, &entry_key, &entry, sizeof(NETACCESS_MGR_SecureAddressEntry_T), &used_buffer))
    {

        UI32_T unit;
        UI32_T port;
        UI32_T trunk_id;

        if(SWCTRL_LPORT_NORMAL_PORT!=SWCTRL_POM_LogicalPortToUserPort(entry.addr_lport, &unit, &port, &trunk_id))
        {
            continue;
        }

        if (is_specify_mac_addr == FALSE)/*pttch 92.01.08 specify mac address to check*/
        {
            if(!is_mac_filter(entry.addr_MAC, mac_addr, mac_mask))
            {
                continue;
            }
        }

        unit_port_to_string(unit, port, sizeof(eth_ach)-1, eth_ach);
        mac_addr_to_string(entry.addr_MAC, sizeof(mac_ach)-1, mac_ach);
        ipv4_addr_to_string(entry.server_ip, sizeof(ipv4_ach)-1, ipv4_ach);
        tick_to_real_time_string(entry.record_time, sizeof(time_ach)-1, time_ach);
        learnt_attr_to_string(entry.is_learnt, sizeof(attr_ach)-1, attr_ach);

        snprintf(buff, sizeof(buff), "%-9s %-17s %15s %-25s %-9s\r\n", eth_ach, mac_ach, ipv4_ach, time_ach, attr_ach);
        buff[sizeof(buff)-1] = '\0';

        PROCESS_MORE(buff);
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Network_Access_Mac_Filter(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE==TRUE)

    UI32_T filter_id;
    int    rc;
    UI8_T  mac_address[SYS_ADPT_MAC_ADDR_LEN] = {0};
    UI8_T  mask[SYS_ADPT_MAC_ADDR_LEN]        = {0};
    char   buf[CLI_DEF_MAX_BUFSIZE] = {0};

    CLI_TBL_Object_T tb;

    CLI_TBL_InitWithBuf(&tb, buf, sizeof(buf));
    CLI_TBL_SetColIndirect(&tb, mac_ftr_tbl, sizeof(mac_ftr_tbl)/sizeof(mac_ftr_tbl[0]));

    CLI_TBL_SetColTitle(&tb, MAC_FTR_TBL_FILTER_ID,    "Filter ID");
    CLI_TBL_SetColTitle(&tb, MAC_FTR_TBL_MAC_ADDRESS,  "MAC Address");
    CLI_TBL_SetColTitle(&tb, MAC_FTR_TBL_MAC_MASK,     "MAC Mask");
    CLI_TBL_Print(&tb);

    CLI_TBL_SetLine(&tb);
    CLI_TBL_Print(&tb);

    if(arg[0]==NULL)
    {
        filter_id=0;
        while(NETACCESS_PMGR_GetNextFilterMac(&filter_id, mac_address, mask))
        {
            rc = show_one_mac_filter(&tb, filter_id, mac_address, mask);
            if (rc != CLI_TBL_PRINT_RC_SUCCESS)
            {
                return CLI_TBL_PRINT_FAIL_RC_TO_CLI(rc);
            }
        }
    }
    else
    {
        filter_id=atoi(arg[0]);
        while(TRUE==NETACCESS_PMGR_GetNextFilterMacByFilterId(filter_id, mac_address, mask))
        {
            rc = show_one_mac_filter(&tb, filter_id, mac_address, mask);
            if (rc != CLI_TBL_PRINT_RC_SUCCESS)
            {
                return CLI_TBL_PRINT_FAIL_RC_TO_CLI(rc);
            }
        }
    }

#endif
    return CLI_NO_ERROR;
}

#if(SYS_CPNT_NETACCESS==TRUE)
static UI32_T show_one_network_access_entry(UI32_T line_num, UI32_T lport)
{
    UI32_T unit;
    UI32_T port;
    UI32_T trunk_id;

#if (SYS_CPNT_NETACCESS_MACAUTH == TRUE)
    UI32_T number;
#endif  /* #if (SYS_CPNT_NETACCESS_MACAUTH == TRUE) */

    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};


    /* Port : 1/1
     */
    SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);
    sprintf(buff, "Port : %lu/%lu\r\n", (unsigned long)unit, (unsigned long)port);
    PROCESS_MORE_FUNC(buff);


    line_num = show_mac_auth(line_num, lport);
    if (line_num == EXIT_SESSION_MORE || line_num == JUMP_OUT_MORE)
    {
        return line_num;
    }

#if (SYS_CPNT_NETACCESS_MACAUTH == TRUE)
    /* show max MAC count
     */
    if (!NETACCESS_PMGR_GetSecureNumberAddresses(lport, &number))
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr("Failed to get Max-MAC-count\r\n");
    #endif
        return line_num;
    }

    sprintf(buff,"Maximum MAC Counts                    : %ld\r\n",(long)number);
    PROCESS_MORE_FUNC(buff);
#endif /* #if (SYS_CPNT_NETACCESS_MACAUTH == TRUE) */

    memset(buff,0,sizeof(buff));


    line_num = show_dynamic_vlan(line_num, lport);
    if (line_num == EXIT_SESSION_MORE || line_num == JUMP_OUT_MORE)
    {
        return line_num;
    }

    line_num = show_dynamic_qos(line_num, lport);
    if (line_num == EXIT_SESSION_MORE || line_num == JUMP_OUT_MORE)
    {
        return line_num;
    }

    line_num = show_mac_filter_id(line_num, lport);
    if (line_num == EXIT_SESSION_MORE || line_num == JUMP_OUT_MORE)
    {
        return line_num;
    }

    line_num = show_guest_vlan(line_num, lport);
    if (line_num == EXIT_SESSION_MORE || line_num == JUMP_OUT_MORE)
    {
        return line_num;
    }

    line_num = show_link_detection(line_num, lport);
    if (line_num == EXIT_SESSION_MORE || line_num == JUMP_OUT_MORE)
    {
        return line_num;
    }

    return line_num;
}

static UI32_T show_mac_auth(UI32_T line_num, UI32_T lport)
{
#if (SYS_CPNT_NETACCESS_MACAUTH == TRUE)
    UI32_T number;
    UI32_T mac_auth_status, mac_auth_action;
    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};

    /*MAC-Authentication*/
    if (!NETACCESS_PMGR_GetMacAuthPortStatus(lport, &mac_auth_status))
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr("Failed to get MAC-Authentication status\r\n");
    #endif
        return line_num;
    }

    if(mac_auth_status == NETACCESS_TYPE_MACAUTH_ENABLED)
    {
        PROCESS_MORE_FUNC("MAC Authentication                    : Enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC("MAC Authentication                    : Disabled\r\n");
    }

    /* MAC-Authentication Intrusion Action */
    if (!NETACCESS_PMGR_GetMacAuthPortIntrusionAction(lport, &mac_auth_action))
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr("Failed to get MAC-Authentication Intrusion Action\r\n");
    #endif
        return line_num;
    }

    if (VAL_macAuthPortIntrusionAction_block_traffic == mac_auth_action)
    {
        PROCESS_MORE_FUNC("MAC Authentication Intrusion Action   : Block traffic\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC("MAC Authentication Intrusion Action   : Pass traffic\r\n");
    }

    /* MAC-Authentication MAC-MAC-Count */
    if (!NETACCESS_PMGR_GetMacAuthPortMaxMacCount(lport, &number))
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr("Failed to get MAC-Authentication MAX-MAC-Count\r\n");
    #endif
        return line_num;
    }

    sprintf(buff,"MAC Authentication Maximum MAC Counts : %ld\r\n",(long)number);
    PROCESS_MORE_FUNC(buff);
    memset(buff,0,sizeof(buff));
#endif /* SYS_CPNT_NETACCESS_MACAUTH == TRUE */

    return line_num;
}

static UI32_T show_dynamic_vlan(UI32_T line_num, UI32_T lport)
{
#if (SYS_CPNT_NETACCESS_DYNAMIC_VLAN==TRUE)
    UI32_T  dynamic_vlan_status;
    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};

    /*dynamic_vlan status*/
    if (!NETACCESS_PMGR_GetSecureDynamicVlanStatus(lport, &dynamic_vlan_status))
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr("Failed to get dynamic_vlan status\r\n");
    #endif
        return line_num;
    }

    if(dynamic_vlan_status == VAL_networkAccessPortDynamicVlan_enabled)
    {
        PROCESS_MORE_FUNC("Dynamic VLAN Assignment               : Enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC("Dynamic VLAN Assignment               : Disabled\r\n");
    }
#endif /* SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE */

    return line_num;
}

static UI32_T show_dynamic_qos(UI32_T line_num, UI32_T lport)
{
#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS==TRUE)
    UI32_T dynamic_qos_status;
    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};

    /*dynamic_vlan status*/
    if (!NETACCESS_PMGR_GetDynamicQosStatus(lport, &dynamic_qos_status))
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr("Failed to get Dynamic QoS status\r\n");
    #endif
        return line_num;
    }

    if(dynamic_qos_status == VAL_networkAccessPortLinkDynamicQos_enabled)
    {
        PROCESS_MORE_FUNC("Dynamic QoS Assignment                : Enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC("Dynamic QoS Assignment                : Disabled\r\n");
    }
#endif /* SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE */

    return line_num;
}


static UI32_T show_mac_filter_id(UI32_T line_num, UI32_T lport)
{
#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE)
    UI32_T filter_id;
    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};

    /*MAC Filter applied & MAC filter id*/
    /*if (NETACCESS_PMGR_IsPortBindFilterId(lport))
    {*/
        if (!NETACCESS_PMGR_GetFilterIdOnPort(lport,&filter_id))
        {
        #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
        #else
            CLI_LIB_PrintStr("Failed to get Filter Id\r\n");
        #endif
            return line_num;
        }
        if(filter_id!=0)
        {
            sprintf(buff,"MAC Filter ID                         : %ld\r\n",(long)filter_id);
            PROCESS_MORE_FUNC(buff);
            memset(buff,0,sizeof(buff));
        }
        else
        {
            PROCESS_MORE_FUNC("MAC Filter ID                         : Disabled\r\n");
        }
    /*}
    else
    {
        PROCESS_MORE_FUNC("MAC Filter ID                         : Disabled\r\n");
    }*/
#endif /* SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE */

    return line_num;
}

static UI32_T show_guest_vlan(UI32_T line_num, UI32_T lport)
{
#if (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)

    UI32_T guest_vlan_id;

    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};


    if (!NETACCESS_PMGR_GetSecureGuestVlanId(lport, &guest_vlan_id))
    {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr("Failed to guest VLAN\r\n");
    #endif
        return line_num;
    }

    if (0 != guest_vlan_id)
    {
        sprintf(buff,"Guest VLAN                            : %ld\r\n", (long)guest_vlan_id);
        PROCESS_MORE_FUNC(buff);
        memset(buff,0,sizeof(buff));
    }
    else
    {
        PROCESS_MORE_FUNC("Guest VLAN                            : Disabled\r\n");
    }

#endif /* SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE */

    return line_num;
}

static UI32_T show_link_detection(UI32_T line_num, UI32_T lport)
{
#if (SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
    UI32_T link_detection_status;
    UI32_T link_detection_mode;
    UI32_T link_detection_action;

    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};


    if (!NETACCESS_PMGR_GetLinkDetectionStatus(lport, &link_detection_status))
    {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr("Failed to link detection status\r\n");
    #endif
        return line_num;
    }

    if (VAL_networkAccessPortLinkDetectionStatus_enabled == link_detection_status)
    {
        PROCESS_MORE_FUNC("Link Detection                        : Enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC("Link Detection                        : Disabled\r\n");
    }

    if (!NETACCESS_PMGR_GetLinkDetectionMode(lport, &link_detection_mode))
    {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr("Failed to link detection mode\r\n");
    #endif
        return line_num;
    }

    switch (link_detection_mode)
    {
        case VAL_networkAccessPortLinkDetectionMode_linkUp:
            PROCESS_MORE_FUNC("Detection Mode                        : Link-up\r\n");
            break;

        case VAL_networkAccessPortLinkDetectionMode_linkDown:
            PROCESS_MORE_FUNC("Detection Mode                        : Link-down\r\n");
            break;

        case VAL_networkAccessPortLinkDetectionMode_linkUpDown:
            PROCESS_MORE_FUNC("Detection Mode                        : Link-up-down\r\n");
            break;

        default:
            PROCESS_MORE_FUNC("Detection Mode                        : None\r\n");
            break;
    }

    if (!NETACCESS_PMGR_GetLinkDetectionAction(lport, &link_detection_action))
    {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr("Failed to link detection action\r\n");
    #endif
        return line_num;
    }

    switch(link_detection_action)
    {
        case VAL_networkAccessPortLinkDetectionAciton_trap:
            PROCESS_MORE_FUNC("Detection Action                      : Trap\r\n");
            break;

        case VAL_networkAccessPortLinkDetectionAciton_shutdown:
            PROCESS_MORE_FUNC("Detection Action                      : Shutdown\r\n");
            break;

        case VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown:
            PROCESS_MORE_FUNC("Detection Action                      : Trap and shutdown\r\n");
            break;

        default:
            PROCESS_MORE_FUNC("Detection Action                      : None\r\n");
            break;
    }


#endif /* #if (SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE) */
    return line_num;
}

static BOOL_T is_mac_filter(UI8_T got_mac[6], UI8_T keyin_mac[6], UI8_T ignore_mask[6])
{
   UI32_T n;

   for(n=0; n<6; n++)
   {
      if( (got_mac[n] & (~ignore_mask[n])) != (keyin_mac[n] & (~ignore_mask[n])))
         return FALSE;
   }
   return TRUE;
}

/* Convert unit/port number to string
 */
static void unit_port_to_string(UI32_T unit, UI32_T port, UI32_T out_str_size, char *out_str_p)
{
    snprintf(out_str_p, out_str_size+1, "Eth %1lu/%2lu", (unsigned long)unit, (unsigned long)port);
    out_str_p[out_str_size] = '\0';
}

/* Convert MAC address to string
 */
static void mac_addr_to_string(const UI8_T *mac_p, UI32_T out_str_size, char *out_str_p)
{
    snprintf(out_str_p, out_str_size+1, "%02X-%02X-%02X-%02X-%02X-%02X",
        mac_p[0], mac_p[1], mac_p[2], mac_p[3], mac_p[4], mac_p[5]);

    out_str_p[out_str_size] = '\0';
}

/* Convert IPv4 address to string
 */
static void ipv4_addr_to_string(UI32_T ipv4_addr, UI32_T out_str_size, char *out_str_p)
{
    snprintf(out_str_p, out_str_size+1, "%d.%d.%d.%d",
        ((unsigned char *)(&ipv4_addr))[0], ((unsigned char *)(&ipv4_addr))[1],
        ((unsigned char *)(&ipv4_addr))[2], ((unsigned char *)(&ipv4_addr))[3]);

    out_str_p[out_str_size] = '\0';
}

/* Convert tick to real time string
 */
static void tick_to_real_time_string(unsigned long tick, unsigned long out_str_size, char *out_str_p)
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;

    SYS_TIME_ConvertSecondsToDateTime(tick, &year, &month, &day, &hour, &minute, &second);
    snprintf(out_str_p, out_str_size+1, "%4dy %02dm %02dd %02dh %02dm %02ds", year, month, day, hour, minute, second);
    out_str_p[out_str_size] = '\0';
}

/* Convert attribute to string
 */
static void learnt_attr_to_string(BOOL_T learnt, unsigned long out_str_size, char *out_str_p)
{
    if (learnt == TRUE)
    {
        strncpy(out_str_p, "Dynamic", out_str_size);
    }
    else
    {
        strncpy(out_str_p, "Static", out_str_size);
    }

    out_str_p[out_str_size] = '\0';
}
#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */

#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE==TRUE)
/* Show one MAC filter entry to CLI
 */
static int show_one_mac_filter(
    CLI_TBL_Object_T *tb,
    UI32_T filter_id,
    const UI8_T mac_address[SYS_ADPT_MAC_ADDR_LEN],
    const UI8_T mask_address[SYS_ADPT_MAC_ADDR_LEN]
    )
{
    char  buff[CLI_DEF_MAX_BUFSIZE];

    CLI_TBL_SetColInt(tb, MAC_FTR_TBL_FILTER_ID, filter_id);

    mac_addr_to_string(mac_address, CLI_DEF_MAX_BUFSIZE-1, buff);
    CLI_TBL_SetColText(tb, MAC_FTR_TBL_MAC_ADDRESS, buff);

    mac_addr_to_string(mask_address, CLI_DEF_MAX_BUFSIZE-1, buff);
    CLI_TBL_SetColText(tb, MAC_FTR_TBL_MAC_MASK, buff);

    return CLI_TBL_Print(tb);
}

#endif /* #if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE==TRUE) */
