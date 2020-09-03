#include <stdio.h>
#include "cli_api.h"
#include "l_stdlib.h"
#include "cli_api_port_channel.h"
#include "trk_mgr.h"
#include "leaf_sys.h"
#include "netaccess_pmgr.h"
#include "cmgr.h"

#define SYS_CPNT_PORT_SECURITY_TRUNK_DEBUG                       FALSE

/************************************<<PORT TRUNK>>******************************************/
/*change mode*/
UI32_T CLI_API_Interface_Portchannel(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    TRK_MGR_TrunkEntry_T trunk_entry;
    BOOL_T is_trunk_existed;
   
    /*Any interface mode can always change mode to trunk mode, only need init*/
    memset(ctrl_P->CMenu.port_id_list, 0, sizeof(ctrl_P->CMenu.port_id_list));
    ctrl_P->CMenu.vlan_ifindex = 0;
    ctrl_P->CMenu.pchannel_id = 0;

    if(arg[0]!=NULL)
        trunk_entry.trunk_index = atoi((char*)arg[0]);   
    else
        return CLI_ERR_INTERNAL;
   
    if(TRK_PMGR_GetTrunkEntry(&trunk_entry))
        is_trunk_existed = TRUE;
    else
        is_trunk_existed = FALSE;

    if ((ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_GLOBAL_MODE && cmd_idx == PRIVILEGE_CFG_GLOBAL_CMD_W2_INTERFACE_PORTCHANNEL)
#if (CLI_SUPPORT_INTERFACE_TO_INTERFACE == 1)
     || (ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_INTERFACE_VLAN_MODE && cmd_idx == PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W2_INTERFACE_PORTCHANNEL)
     || (ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_INTERFACE_ETH_MODE && cmd_idx == PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_INTERFACE_PORTCHANNEL)
#if (SYS_CPNT_LOOPBACK_IF_VISIBLE == TRUE)
     || (ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_INTERFACE_LOOPBACK_MODE && cmd_idx == PRIVILEGE_CFG_INTERFACE_LOOPBACK_CMD_W2_INTERFACE_PORTCHANNEL)
#endif
#if (SYS_CPNT_CRAFT_PORT == TRUE)
     || (ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_INTERFACE_CRAFT_MODE && cmd_idx == PRIVILEGE_CFG_INTERFACE_CRAFT_CMD_W2_INTERFACE_PORTCHANNEL)
#endif
#if (SYS_CPNT_IP_FOR_ETHERNET0 == TRUE)
     || (ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_INTERFACE_ETH0_MODE && cmd_idx == PRIVILEGE_CFG_INTERFACE_ETH0_CMD_W2_INTERFACE_PORTCHANNEL)
#endif
     || (ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_INTERFACE_PCHANNEL_MODE && cmd_idx == PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_INTERFACE_PORTCHANNEL)
#endif
        )
    {
        if(!is_trunk_existed)
        {
            if(!TRK_PMGR_CreateTrunk(atoi((char*)arg[0])) ) /*not existed and create that*/
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("Failed to create port trunk %d\r\n", atoi((char*)arg[0]));
#endif
                return CLI_NO_ERROR;
            }
            else if(!TRK_PMGR_SetTrunkStatus(atoi((char*)arg[0]), VAL_trunkStatus_valid))
            {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1("Failed to enable port trunk %d\r\n", atoi((char*)arg[0]));
#endif
            return CLI_NO_ERROR;
            }
        }
        ctrl_P->CMenu.pchannel_id = atoi((char*)arg[0]);
        ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_INTERFACE_PCHANNEL_MODE;
    }
    else if (ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_GLOBAL_MODE && cmd_idx == PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_INTERFACE_PORTCHANNEL)
    {
        if(!is_trunk_existed)
        {
            CLI_LIB_PrintStr("no such port trunk\r\n");
        }
        else
        {
            if(!TRK_PMGR_SetTrunkStatus(atoi((char*)arg[0]), VAL_trunkStatus_invalid))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("Failed to disable port trunk %d\r\n", atoi((char*)arg[0]));
#endif
                return CLI_NO_ERROR;
            }
            else if(!TRK_PMGR_DestroyTrunk(atoi((char*)arg[0])))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("Failed to destroy port trunk %d\r\n", atoi((char*)arg[0]));
#endif
            }
        }
    }
    else
    {
        return CLI_ERR_INTERNAL;
    }

    return CLI_NO_ERROR; 
}

/*execution*/

/*configuration*/
UI32_T CLI_API_Shutdown_Portchannel(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T admin_status;
    UI32_T ifindex;
   
    SWCTRL_POM_TrunkIDToLogicalPort(ctrl_P->CMenu.pchannel_id, &ifindex);
      
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W1_SHUTDOWN:
            admin_status = VAL_ifAdminStatus_down;
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_NO_SHUTDOWN:
            admin_status = VAL_ifAdminStatus_up;
            break;
            
        default:
            return CLI_ERR_INTERNAL;      
    }   
    if(!CMGR_SetPortAdminStatus(ifindex, admin_status))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_2("Failed to %s trunk %lu\r\n", admin_status == VAL_ifAdminStatus_down ? "shutdown" : "no shutdown", (unsigned long)ctrl_P->CMenu.pchannel_id);
#endif
    }


    return CLI_NO_ERROR; 
}

UI32_T CLI_API_Description_Portchannel(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI8_T null_str = 0;
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W1_DESCRIPTION:

            if(arg[0]!=NULL)
            {
                if (!TRK_PMGR_SetTrunkName(ctrl_P->CMenu.pchannel_id , (UI8_T *)arg[0]))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set trunk name\r\n");
#endif
                }
            }
            
            else
                return CLI_ERR_INTERNAL;
                
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_NO_DESCRIPTION:
            if (!TRK_PMGR_SetTrunkName(ctrl_P->CMenu.pchannel_id ,&null_str))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to remove trunk name\r\n");
#endif
            }
            break;
            
        default:
            return CLI_ERR_INTERNAL;      
    }   

    return CLI_NO_ERROR; 
}

/*EPR:ES4827G-FLF-ZZ-00232
 *Problem: CLI:size of vlan name different in console and mib
 *Solution: add CLI command "alias" for interface set,the
 *          alias is different from name and port descrition,so
 *          need add new command.
 *modify file: cli_cmd.c,cli_cmd.h,cli_arg.c,cli_arg.h,cli_msg.c,
 *             cli_msg.h,cli_api_vlan.c,cli_api_vlan.h,cli_api_ehternet.c
 *             cli_api_ethernet.h,cli_api_port_channel.c,cli_api_port_channel.h,
 *             cli_running.c,rfc_2863.c,swctrl.h,trk_mgr.h,trk_pmgr.h,swctrl.c
 *             swctrl_pmgr.c,trk_mgr.c,trk_pmgr.c,vlan_mgr.h,vlan_pmgr.h,
 *             vlan_type.h,vlan_mgr.c,vlan_pmgr.c,if_mgr.c
 *Approved by:Hardsun
 *Fixed by:Dan Xie
 */

UI32_T CLI_API_Alias_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SWCTRL_CONTAIN_CLI_ALIAS == TRUE)
    UI8_T null_str = 0;
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W1_ALIAS:

            if((arg[0]!=NULL)&(strlen((char*)arg[0]) <= MAXSIZE_ifAlias))
            {
                if (!TRK_PMGR_SetTrunkAlias(ctrl_P->CMenu.pchannel_id , (UI8_T *)arg[0]))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set trunk alias\r\n");
#endif
                }
            }
            else
                return CLI_ERR_INTERNAL;
                
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_NO_ALIAS:
            if (!TRK_PMGR_SetTrunkAlias(ctrl_P->CMenu.pchannel_id ,&null_str))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to remove trunk alias\r\n");
#endif
            }
            break;
            
        default:
            return CLI_ERR_INTERNAL;      
    }   
#endif /* (SYS_CPNT_SWCTRL_CONTAIN_CLI_ALIAS == TRUE) */

    return CLI_NO_ERROR; 
}

UI32_T CLI_API_Channelgroup(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T i;
    UI32_T lport = 0;
    UI32_T trunk_id, trunk_lport;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T verify_ret;
    BOOL_T ret;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_CHANNELGROUP:
            if (arg[0] == NULL)
                return CLI_ERR_INTERNAL;
            trunk_id = atoi(arg[0]);
            break;
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_CHANNELGROUP:
            trunk_id = 0;
            break;
        default:
           return CLI_ERR_INTERNAL;
    }

    if (trunk_id != 0)
    {
        verify_ret = verify_trunk(trunk_id, &trunk_lport);
        if (verify_ret != CLI_API_TRUNK_OK && verify_ret != CLI_API_TRUNK_NO_MEMBER)
        {
            display_trunk_msg(verify_ret, trunk_id);
            return CLI_NO_ERROR;
        }
    }

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;

            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
            if (verify_ret != CLI_API_ETH_OK && verify_ret != CLI_API_ETH_TRUNK_MEMBER)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }

            if (trunk_id != 0) /* add to trunk */
            {
                ret = TRK_PMGR_AddTrunkMember(trunk_id, lport) == TRK_MGR_SUCCESS;
            }
            else /* remove from trunk */
            {
                UI32_T trunk_id;

                ret = FALSE;

                for (trunk_id = 1; trunk_id <= SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; trunk_id++)
                {
                    if (TRK_PMGR_DeleteTrunkMember(trunk_id, lport))
                    {
                        ret = TRUE;
                        break;
                    }
                }
            }

            if (!ret)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                char name[MAXSIZE_ifName+1] = {0};
#if (CLI_SUPPORT_PORT_NAME == 1)  
                CLI_LIB_Ifindex_To_Name(lport,name);
#else
                sprintf(name, "%lu/%lu", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
                if (trunk_id != 0)
                {
                    CLI_LIB_PrintStr_2("Failed to add ethernet %s to channel-group %lu\r\n", name, (unsigned long)trunk_id);
                }
                else
                {
                    CLI_LIB_PrintStr_1("Failed to remove ethernet %s from channel-group\r\n", name);
                }
#endif
            }
        }
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Speedduplex_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SPEED_DUPLEX == TRUE)
#define SPDPLX_DEFAULT 65535

    UI32_T speed_duplex    = 0;//pttch
    UI32_T ifindex         = 0;
    SWCTRL_POM_TrunkIDToLogicalPort(ctrl_P->CMenu.pchannel_id,&ifindex);
      
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W1_SPEEDDUPLEX:
        
            if(arg[0]!=NULL)
            {
                if (*(arg[0]) == '4')
                    speed_duplex = VAL_portSpeedDpxCfg_fullDuplex40g;
                else if (*(arg[0]+2) == 'F' || *(arg[0]+2) == 'f')
                    speed_duplex = VAL_portSpeedDpxCfg_fullDuplex10;
                else if (*(arg[0]+2) == 'H' || *(arg[0]+2) == 'h')
                    speed_duplex = VAL_portSpeedDpxCfg_halfDuplex10;
                else if (*(arg[0]+2) == 'G' || *(arg[0]+2) == 'g')
                    speed_duplex = VAL_portSpeedDpxCfg_fullDuplex10g;
                else if (*(arg[0]+3) == 'F' || *(arg[0]+3) == 'f')
                    speed_duplex = VAL_portSpeedDpxCfg_fullDuplex100;
                else if (*(arg[0]+3) == 'H' || *(arg[0]+3) == 'h')
                    speed_duplex = VAL_portSpeedDpxCfg_halfDuplex100;      
                else if (*(arg[0]+3) == 'G' || *(arg[0]+3) == 'g')
                    speed_duplex = VAL_portSpeedDpxCfg_fullDuplex100g;
                else if (*(arg[0]+3) == '0')
                    speed_duplex = VAL_portSpeedDpxCfg_fullDuplex1000;
                else
                    return CLI_ERR_INTERNAL;
            }
            else
                return CLI_ERR_INTERNAL;
                
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_NO_SPEEDDUPLEX:
            speed_duplex = SPDPLX_DEFAULT; /*default: temp: SYS_CST_SW_AUTO*/
            break;

        default:
            return CLI_ERR_INTERNAL;
    }  

    if(speed_duplex == SPDPLX_DEFAULT)
    {
        if (!SWCTRL_PMGR_SetPortDefaultSpeedDuplex(ifindex))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set default speed-duplex\r\n"); 
#endif
        }
    }
    else
    {   
        if (!SWCTRL_PMGR_SetPortCfgSpeedDuplex(ifindex, speed_duplex))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set speed-duplex\r\n");       
#endif
        }
    }

#endif   
    return CLI_NO_ERROR;
}



UI32_T CLI_API_Negotiation_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NEGOTIATION == TRUE)
    UI32_T status  = 0;
    UI32_T ifindex = 0;
   
    SWCTRL_POM_TrunkIDToLogicalPort(ctrl_P->CMenu.pchannel_id,&ifindex);     
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W1_NEGOTIATION:
            status = VAL_portAutonegotiation_enabled;
            break;
      
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_NO_NEGOTIATION:
            status = VAL_portAutonegotiation_disabled;
            break;
   
        default:
            return CLI_ERR_INTERNAL;      
    }   
    if (!SWCTRL_PMGR_SetPortAutoNegEnable(ifindex, status))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set PortAutoNeginterface\r\n");     
#endif
    }

#endif
    return CLI_NO_ERROR; 
}

UI32_T CLI_API_Capabilities_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NEGOTIATION == TRUE)
    UI32_T capabilities    = 0;//pttch
    UI32_T ifindex         = 0;
    Port_Info_T port_info;
   
    if (arg[0] == NULL)
        capabilities = DEFAULT_CAPABILITIES; /*actually, just for no form*/
    else if (*arg[0] == 's' || *arg[0] == 'S')
        capabilities = SYS_VAL_portCapabilities_portCapSym;
    else if (*arg[0] == 'f' || *arg[0] == 'F')
        capabilities = SYS_VAL_portCapabilities_portCapFlowCtrl;      
    else if (*(arg[0]) == '4')
        capabilities = SYS_VAL_portCapabilities_portCap40gFull;
    else if (*(arg[0]+2) == 'F' || *(arg[0]+2) == 'f')
        capabilities = SYS_VAL_portCapabilities_portCap10full;
    else if (*(arg[0]+2) == 'H' || *(arg[0]+2) == 'h')
        capabilities = SYS_VAL_portCapabilities_portCap10half;
    else if (*(arg[0]+2) == 'G' || *(arg[0]+2) == 'g')
        capabilities = SYS_VAL_portCapabilities_portCap10gFull;
    else if (*(arg[0]+3) == 'F' || *(arg[0]+3) == 'f')
        capabilities = SYS_VAL_portCapabilities_portCap100full;
    else if (*(arg[0]+3) == 'H' || *(arg[0]+3) == 'h')
        capabilities = SYS_VAL_portCapabilities_portCap100half;      
    else if (*(arg[0]+3) == 'G' || *(arg[0]+3) == 'g')
        capabilities = SYS_VAL_portCapabilities_portCap100gFull;
    else if (*(arg[0]+3) == '0')
        capabilities = SYS_VAL_portCapabilities_portCap1000full;
    else
        return CLI_ERR_INTERNAL;

    SWCTRL_POM_TrunkIDToLogicalPort(ctrl_P->CMenu.pchannel_id,&ifindex);
    memset(&port_info, 0, sizeof(Port_Info_T));
    SWCTRL_POM_GetPortInfo(ifindex, &port_info);
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W1_CAPABILITIES:
            port_info.autoneg_capability |= capabilities;        
            break;
   
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_NO_CAPABILITIES:
            if(capabilities == DEFAULT_CAPABILITIES)
            {
                if (!SWCTRL_PMGR_SetPortDefaultAutoNegCapability(ifindex))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set default capability\r\n");       
#endif
                }
                return CLI_NO_ERROR;
            }
            else
            {
                port_info.autoneg_capability &= ~capabilities;
            }
            break;
   
        default:
            return CLI_ERR_INTERNAL;
    }  
    if (!SWCTRL_PMGR_SetPortAutoNegCapability(ifindex, port_info.autoneg_capability))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set capability\r\n");       
#endif
    }
#endif

    return CLI_NO_ERROR;
}



UI32_T CLI_API_Flowcontrol_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_FLOW_CONTROL==TRUE)
    UI32_T status  = 0;
    UI32_T ifindex = 0;
   
    SWCTRL_POM_TrunkIDToLogicalPort(ctrl_P->CMenu.pchannel_id,&ifindex);     
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W1_FLOWCONTROL:
            status = VAL_portFlowCtrlCfg_enabled;
            break;
      
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_NO_FLOWCONTROL:
            status = VAL_portFlowCtrlCfg_disabled;
            break;
   
        default:
            return CLI_ERR_INTERNAL;      
    }       
    if (!SWCTRL_PMGR_SetPortCfgFlowCtrlEnable(ifindex, status))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set flowcontrol\r\n");       
#endif
    }

#endif   
    return CLI_NO_ERROR; 
}



/************************************<<PORT SECURITY>>***************************************/
/*change mode*/

/*execution*/

/*configuration*/
UI32_T CLI_API_Port_Security_Action_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)
    UI32_T action_trap_status;
   
    UI32_T lport;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_PORT_SECURITY_ACTION:
#ifdef ALLAYER_SWITCH
            action_trap_status = VAL_portSecAction_trapAndShutdown;
#endif
#if defined(STRATA_SWITCH) || defined(XGS_SWITCH)
            if(arg[0]!=NULL)
            {
                switch(arg[0][0])
                {
                    case 's':	
                    case 'S':
                        action_trap_status = VAL_portSecAction_shutdown;
                        break;
             
                    case 't':	
                    case 'T':
                        switch(arg[0][4])
                        {
                            case '-':
                                action_trap_status = VAL_portSecAction_trapAndShutdown;
                                break;
                
                            default:
                                action_trap_status = VAL_portSecAction_trap;
                                break;	
                        }
                        break;
             
                    default:
                        return CLI_ERR_INTERNAL;
                }
            }
            else
                return CLI_ERR_INTERNAL;
#endif
            break;
      
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_NO_PORT_SECURITY_ACTION:
            action_trap_status = VAL_portSecAction_none; 
            break;
      
        default:
            return CLI_ERR_INTERNAL;
    }

    SWCTRL_POM_TrunkIDToLogicalPort(ctrl_P->CMenu.pchannel_id,&lport);     
#if (SYS_CPNT_PORT_SECURITY_TRUNK_DEBUG== TRUE)
    printf("%s--%s--%d lport:%d action_trap_status:%d \n",__FILE__,__FUNCTION__,__LINE__,lport, action_trap_status);
#endif
#if (SYS_CPNT_NETACCESS == TRUE)
    if (!NETACCESS_PMGR_SetPortSecurityActionStatus(lport, action_trap_status))
    #else
    if (!PSEC_PMGR_SetPortSecurityActionStatus(lport, action_trap_status))
#endif
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set PortSecurityAction\r\n");     
#endif
    }

#endif
#endif
    return CLI_NO_ERROR;
}



UI32_T CLI_API_Port_Security_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
#if (CLI_SUPPORT_PSEC_MAC_COUNT == 1)
    UI32_T port_security_status = 0;
   
    UI32_T lport;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port = 0;
    UI32_T number = 0;
	
#if (SYS_CPNT_PORT_SECURITY_TRUNK_DEBUG== TRUE)
    printf("%s--%s--%d cmd_idx:%d  \n",__FILE__,__FUNCTION__,__LINE__,cmd_idx);
#endif
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_PORT_SECURITY:
            if (arg[0] == NULL)
            {
                port_security_status = VAL_portSecPortStatus_enabled;
            }
            else if (arg[0][0] == 'm' || arg[0][0] == 'M')
            {
                number = atoi((char*)arg[1]);	
            }
            break;
      
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_PORT_SECURITY:
            if (arg[0] == NULL)
            {
                port_security_status = VAL_portSecPortStatus_disabled;
            }
            else if (arg[0][0] == 'm' || arg[0][0] == 'M')
            {
                number = SYS_DFLT_PORT_SECURITY_MAX_MAC_COUNT;//SYS_DFLT_PSEC_AUTO_LEARN_MAC_COUNT default	
            }
            break;
      
        default:
            return CLI_NO_ERROR;
    }
	
    SWCTRL_POM_TrunkIDToLogicalPort(ctrl_P->CMenu.pchannel_id,&lport);     

    if (arg[0] == NULL)
    {
#if (SYS_CPNT_PORT_SECURITY_TRUNK_DEBUG== TRUE)
        printf("%s--%s--%d lport:%d port_security_status:%d \n",__FILE__,__FUNCTION__,__LINE__,lport, port_security_status);
#endif
#if (SYS_CPNT_NETACCESS == TRUE)
        if(!NETACCESS_PMGR_SetPortSecurityStatus(lport, port_security_status))
#else
        if(!PSEC_PMGR_SetPortSecurityStatus(lport, port_security_status))
#endif
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set PortSecurity\r\n");     
#endif
        }
    }
    else if (arg[0][0] == 'm' || arg[0][0] == 'M')
    {
#if (SYS_CPNT_PORT_SECURITY_TRUNK_DEBUG== TRUE)
        printf("%s--%s--%d lport:%d number:%d \n",__FILE__,__FUNCTION__,__LINE__,lport, number);
#endif
#if (SYS_CPNT_NETACCESS == TRUE)
        if(!NETACCESS_PMGR_SetPortSecurityMaxMacCount(lport, number))
#else
        if(!PSEC_PMGR_SetPortSecurityMacCount(lport, number))
#endif
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_2("Failed to set port %lu/%lu security MAC address count\r\n",
                                                                 verify_unit, verify_port);       
#endif
        }
    }

#else
    UI32_T port_security_status = 0;
    UI32_T i       = 0;
   
    UI32_T lport;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_PORT_SECURITY:
            port_security_status = VAL_portSecPortStatus_enabled;
            break;
      
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_PORT_SECURITY:
            port_security_status = VAL_portSecPortStatus_disabled;
            break;
      
        default:
            return CLI_ERR_INTERNAL;
    }

#if (SYS_CPNT_NETACCESS == TRUE)
    if (!NETACCESS_PMGR_SetPortSecurityStatus(lport, port_security_status))
#else
    if (!PSEC_PMGR_SetPortSecurityStatus(lport, port_security_status))
#endif
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_3("Failed to %s port %lu/%lu security status\r\n", port_security_status == VAL_portSecPortStatus_enabled ? "enable" : "disable",
                                                                      verify_unit, verify_port);       
#endif
    }

#endif
#endif
    return CLI_NO_ERROR; 
}


#if 0
UI32_T CLI_API_Mdix_Pch(UI16_T cmd_idx, UI8_T *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SWCTRL_MDIX_CONFIG == TRUE)
   UI32_T mdix_mode            = 0;/*pttch*/ /* 1:auto, 2:straight, 3:crossover */
   UI32_T ifindex         = 0;
   Port_Info_T port_info;
   
   if (arg[0] == NULL)
      mdix_mode = SYS_DFLT_MDIX_MODE; /*actually, just for no form*/
   else if (*arg[0] == 'a' || *arg[0] == 'A')
      mdix_mode = VAL_portMdixMode_auto;
   else if (*arg[0] == 'c' || *arg[0] == 'C')
      mdix_mode = VAL_portMdixMode_crossover;  
   else if (*arg[0] == 's' || *arg[0] == 'S')
      mdix_mode = VAL_portMdixMode_straight;
   else
      return CLI_ERR_INTERNAL;

    SWCTRL_TrunkIDToLogicalPort(ctrl_P->CMenu.pchannel_id,&ifindex);

#if (SYS_CPNT_MDIX_SUPPORT_GIGA_ETHERNET_COPPER == TRUE)

                /* when link status change to up and speed is 1000
                 * we modify the MDIX mode to auto
                 *  because other MDIX setting not support 1000T 
                 */
                SWCTRL_GetPortInfo(ifindex, &port_info);

                if (port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_fullDuplex1000 && port_info.link_status == SWCTRL_LINK_UP)
                {
                    CLI_LIB_PrintStr("Failed to set MDIX\r\n");
                    return CLI_NO_ERROR;
                }
#endif /* #if (SYS_CPNT_MDIX_SUPPORT_GIGA_ETHERNET_COPPER == TRUE) */

    memset(&port_info, 0, sizeof(Port_Info_T));
    SWCTRL_GetPortInfo(ifindex, &port_info);
    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W1_MDIX:
        switch ( arg[0][0] )
        {
            case 'a':	/* auto */
            case 'A':
                port_info.port_MDIX_mode = mdix_mode;   
            break;
                    	
            case 'c':   /* crossover */
            case 'C':
                port_info.port_MDIX_mode = mdix_mode;  
            break;
                    	
            case 's':   /* straight */
            case 'S':
                port_info.port_MDIX_mode = mdix_mode;  
            break;                    		
         }
        break;

    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_NO_MDIX:
            port_info.port_MDIX_mode = mdix_mode;  
        break;
    default:
      return CLI_ERR_INTERNAL;
    }



   if (!SWCTRL_PMGR_SetMDIXMode(ifindex, port_info.port_MDIX_mode))
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to set MDIX\r\n");       
#endif
   }
#endif

    return CLI_NO_ERROR;
}

#endif

/* Load Balance */
UI32_T CLI_API_PortChannel_LoadBalance(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRUNK_BALANCE_MODE == TRUE)
    UI32_T mode = SYS_DFLT_TRUNK_BALANCE_MODE;

    switch ( cmd_idx )
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_PORTCHANNEL_LOADBALANCE:
            if ( arg [0][0] == 's' || arg [0][0] == 'S' )
            {
                switch ( arg [0][4] )
                {
                    case 'i': /* src-ip */
                    case 'I':
                        mode = SWCTRL_TRUNK_BALANCE_MODE_IP_SA;
                        break;

                    case 'm': /* src-mac */
                    case 'M':
                        mode = SWCTRL_TRUNK_BALANCE_MODE_MAC_SA;
                        break;

                    case 'd':
                    case 'D':
                        if ( arg [0][8] == 'i' || arg [0][8] == 'I' )
                        { /* src-dst-ip */
                            mode = SWCTRL_TRUNK_BALANCE_MODE_IP_SA_DA;
                        }
                        else
                        { /* src-dst-mac */
                            mode = SWCTRL_TRUNK_BALANCE_MODE_MAC_SA_DA;
                        }
                        break;
                }
            }
            else
            {
                switch ( arg [0][4] )
                {
                    case 'i': /* dst-ip */
                    case 'I':
                        mode = SWCTRL_TRUNK_BALANCE_MODE_IP_DA;
                        break;

                    case 'm': /* dst-mac */
                    case 'M':
                        mode = SWCTRL_TRUNK_BALANCE_MODE_MAC_DA;
                        break;
                }
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_PORTCHANNEL_LOADBALANCE:
            mode = SYS_DFLT_TRUNK_BALANCE_MODE;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    /* improve here */
    if (!SWCTRL_PMGR_SetTrunkBalanceMode(mode))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set trunk load balance mode.\r\n");
#endif
    }
#endif /* (SYS_CPNT_TRUNK_BALANCE_MODE == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_PortChannel_LoadBalance(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRUNK_BALANCE_MODE == TRUE)
    char *balance_mode_str = NULL;
    UI32_T balance_mode;

    if (SWCTRL_POM_GetTrunkBalanceMode(&balance_mode))
    {
    	switch (balance_mode)
    	{
        	case SWCTRL_TRUNK_BALANCE_MODE_MAC_SA:
        	    balance_mode_str = "Source MAC address";
        	    break;

        	case SWCTRL_TRUNK_BALANCE_MODE_MAC_DA:
        	    balance_mode_str = "Destination MAC address";
        	    break;

        	case SWCTRL_TRUNK_BALANCE_MODE_MAC_SA_DA:
        	    balance_mode_str = "Source and destination MAC address";
        	    break;

        	case SWCTRL_TRUNK_BALANCE_MODE_IP_SA:
        	    balance_mode_str = "Source IP address";
        	    break;

        	case SWCTRL_TRUNK_BALANCE_MODE_IP_DA:
        	    balance_mode_str = "Destination IP address";
        	    break;

        	case SWCTRL_TRUNK_BALANCE_MODE_IP_SA_DA:
        	    balance_mode_str = "Source and destination IP address";
        	    break;
    	}

    	CLI_LIB_PrintStr_1("Trunk Load Balance Mode: %s\r\n", balance_mode_str);
        CLI_LIB_PrintStr("\r\n");
    }
#endif /* (SYS_CPNT_TRUNK_BALANCE_MODE == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_PortChannel_MaxActivePorts(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRUNK_MAX_ACTIVE_PORTS_CONFIGURABLE == TRUE)
    UI32_T max_num_of_active_ports;

    switch ( cmd_idx )
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W1_MAXACTIVEPORTS:
            max_num_of_active_ports = atoi(arg[0]);
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_NO_MAXACTIVEPORTS:
            max_num_of_active_ports = SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (!SWCTRL_PMGR_SetTrunkMaxNumOfActivePorts(ctrl_P->CMenu.pchannel_id, max_num_of_active_ports))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set trunk max number of active ports.\r\n");
#endif
    }
#endif /* (SYS_CPNT_TRUNK_MAX_ACTIVE_PORTS_CONFIGURABLE == TRUE) */

    return CLI_NO_ERROR;
}

