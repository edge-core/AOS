//rich#include "rs.h"
#include <stdio.h>
#include "cli_api.h"
#include "netaccess_pmgr.h"
#include "sys_cpnt.h"
#include "l_stdlib.h"
#include "cli_api_ethernet.h"
#include "swctrl_pmgr.h"  /*added by Jinhua Wei ,to remove warning ,becaued the relative function's head file not included */
#include "cmgr.h"
#include "psec_pmgr.h"
#ifdef isdigit
#undef isdigit
#endif
#define isdigit(c) ((c) >= '0' && (c) <= '9')  /* remove warning,show undefined isdigit,added by Jinhua Wei*/

/************************************<<INETRAFCE>>************************************//*eth*/
/*change mode*/
UI32_T CLI_API_Interface_Ethernet(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    char   delemiters[2] = {0};
    char   Token[CLI_DEF_MAX_BUFSIZE] = {0};//pttch
    char   *s;
    UI8_T  unit;
    UI32_T lower_val = 0;
    UI32_T upper_val = 0;
    UI32_T err_idx;
    UI8_T  i;
    UI8_T  previous_port_id_list[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST] = {0};
    UI32_T previous_vlan  = 0;
    UI32_T previous_trunk = 0;
    UI32_T previous_loopback = 0;

   
    UI8_T  not_present_port_list[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST] = {0};
    BOOL_T is_any_one_not_present = FALSE;
      
    delemiters[0] = ',';
   
    /*store previous interface information and set working space to 0*/
    memcpy(previous_port_id_list, ctrl_P->CMenu.port_id_list, sizeof(ctrl_P->CMenu.port_id_list));
    previous_vlan = ctrl_P->CMenu.vlan_ifindex;
    previous_trunk = ctrl_P->CMenu.pchannel_id;
    previous_loopback = ctrl_P->CMenu.loopback_id;
   
    memset(ctrl_P->CMenu.port_id_list, 0, sizeof(ctrl_P->CMenu.port_id_list));
    ctrl_P->CMenu.vlan_ifindex = 0;
    ctrl_P->CMenu.pchannel_id = 0;
    ctrl_P->CMenu.loopback_id = 0;

    if(arg[0]!=NULL)
    {
        if (isdigit(arg[0][0])) 
        {
            s = arg[0];
          
            /*get the unit*/
            unit = atoi((char*)s);/*pttch stacking*/
            ctrl_P->CMenu.unit_id = unit;
          
            /*move the ptr to just after the slash */
            s = strchr(s, '/') + 1;
          
            while(1)
            {
                s =  CLI_LIB_Get_Token(s, Token, delemiters);
          
                if(CLI_LIB_CheckNullStr(Token))
                {
                    if(s == 0)
                        break;
                    else
                        continue;
                }
                else
                {
                    CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx);
          
                    /*
                        bit7    ...   bit1 bit0 bit7    ...   bit1 bit0 bit7    ...   bit1 bit0
                       |-----------------------|-----------------------|-----------------------|-- 
                       |        Byte 0         |        Byte 1         |       Byte 2          |   ...
                       |-----------------------|-----------------------|-----------------------|--
                        port1  ...  port7 port8 port9 ... port15 port16 port17 ... port23 port24
                     */
                    for(i=lower_val; i<=upper_val; i++)
                    {
                        if(!SWCTRL_POM_UIUserPortExisting(unit, i))
                        {
                            is_any_one_not_present = TRUE;
                            not_present_port_list[(UI32_T)((i-1)/8)] |= (1 << (7 - ((i-1)%8)));
                        }
                        else
                        {
                            ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)] |=   (1 << (7 - ((i-1)%8))) ;
                        }
                    }
                }
          
                if(s == 0)
                    break;
                else
                    memset(Token, 0, sizeof(Token));
            }
          
          
                    if(is_any_one_not_present)
                    {
                        UI32_T max_port_num = SWCTRL_POM_UIGetUnitPortNumber(unit);
                        UI8_T  not_present_num = 0;
             
                        CLI_LIB_PrintStr("Port");
                        for(i=1; i<=max_port_num; i++)
                        {
                            if (not_present_port_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
                            {
                                not_present_num ++;
                                CLI_LIB_PrintStr_2(" eth %d/%d", unit, i);
                            }
                        }
             
                        if(not_present_num >1)
                            CLI_LIB_PrintStr(" are not present\r\n");
                        else
                            CLI_LIB_PrintStr(" is not present\r\n");
                            /*restore previous port list*/
                            memcpy(ctrl_P->CMenu.port_id_list, previous_port_id_list, sizeof(ctrl_P->CMenu.port_id_list));
                            ctrl_P->CMenu.vlan_ifindex = previous_vlan;
                            ctrl_P->CMenu.pchannel_id = previous_trunk;
                            ctrl_P->CMenu.loopback_id = previous_loopback;
                    }
                    else
                    {
                        ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_INTERFACE_ETH_MODE;
                    }
        }
    
#if (CLI_SUPPORT_PORT_NAME == 1)
        else
        {
            UI32_T ifindex = 0;
            UI32_T unit,port,trunk_id;

            if (!IF_PMGR_IfnameToIfindex(arg[0], &ifindex))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[0]);
#endif
                return CLI_NO_ERROR;
            }
            SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);
            if (!SWCTRL_POM_UIUserPortExisting(unit, port))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("%s is not present\r\n",arg[0]);
#endif
                return CLI_NO_ERROR;
            }
            ctrl_P->CMenu.port_id_list[(UI32_T)((ifindex-1)/8)] |=   (1 << (7 - ((ifindex-1)%8)));
            ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_INTERFACE_ETH_MODE;
        }
#endif
    }
    else
    {
        return CLI_ERR_INTERNAL;
    }
    
    return CLI_NO_ERROR;
}

#if(SYS_CPNT_SYSTEMWIDE_COUNTER == TRUE)

UI32_T CLI_API_Clear_Counters(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0
    CLI_API_ShowIfCounter_T IfCounter;
#endif  /* #if 0 */

    UI32_T                  ifindex;
    UI32_T                  idx;

    if(arg[0]==NULL)
        return CLI_ERR_INTERNAL;
    else
    {
        switch(arg[0][0])
        {
            case 'e':
            case 'E':
            {
                if(arg[1]!=NULL)
                {
                    UI32_T                  verify_unit = atoi(arg[1]);
                    UI32_T                  verify_port = atoi( strchr(arg[1], '/')+1 );
                    CLI_API_EthStatus_T     verify_ret  = 0;
#if (CLI_SUPPORT_PORT_NAME == 1)
                    if (isdigit(arg[1][0]))
                    {
                        verify_unit = atoi(arg[1]);
                        verify_port = atoi(strchr(arg[1],'/')+1);
                    }
                    else/*port name*/
                    {
                        UI32_T trunk_id = 0;
                        UI32_T lport = 0;
                        if (!IF_PMGR_IfnameToIfindex(arg[1], &lport))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("%s is not exist\r\n",arg[1]);
#endif
                            return CLI_NO_ERROR;
                        }
                        SWCTRL_POM_LogicalPortToUserPort(lport, &verify_unit, &verify_port, &trunk_id);
                    }
#endif
                    verify_ethernet(verify_unit, verify_port, &ifindex);
                    idx = (verify_unit - 1) * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT + verify_port - 1;      /*idx is the port number reduce 1*/

                    if(verify_ret == CLI_API_ETH_NOT_PRESENT || verify_ret == CLI_API_ETH_UNKNOWN_PORT )
                    {
                        display_ethernet_msg(verify_ret, verify_unit, verify_port);
                        return CLI_NO_ERROR;
                    } 
                }
                else
                    return CLI_ERR_INTERNAL;
            }
                break;
          
            case 'p':
            case 'P':
            {
                if(arg[1]!=NULL)
                {
                    UI32_T  trunk_id = atoi(arg[1]);
                    CLI_API_TrunkStatus_T verify_ret_t;

                    idx = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT + trunk_id -1;
                    if( (verify_ret_t = verify_trunk(trunk_id, &ifindex)) == CLI_API_TRUNK_NOT_EXIST)
                    {
                        display_trunk_msg(verify_ret_t, trunk_id);
                        return CLI_NO_ERROR;
                    }
                }
                else
                    return CLI_ERR_INTERNAL;
            }
                break;

#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
            case 'v':
            case 'V':
            {
                if(arg[1]!=NULL)
                {
                    UI32_T  vid = atoi(arg[1]);

                    if (!VLAN_POM_IsVlanExisted(vid))
                    {
                        CLI_LIB_PrintStr_1("VLAN %lu does not exist\r\n", vid);
                        return CLI_NO_ERROR;
                    }

                    VLAN_VID_CONVERTTO_IFINDEX(vid, ifindex);
                }
                else
                    return CLI_ERR_INTERNAL;
            }
                break;
#endif /* (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE) */

            default:
                return CLI_ERR_INTERNAL;	
        }
    }
   
    NMTR_PMGR_ClearSystemwideStats(ifindex);
   
#if 0
   if(ctrl_P->CMenu.counters_table[idx] == NULL)
      if((ctrl_P->CMenu.counters_table[idx] = calloc(1, sizeof(CLI_API_ShowIfCounter_T))) == NULL)
         return CLII_ERR_MEMORY_NOT_ENOUGH;

   memset(&IfCounter, 0, sizeof(CLI_API_ShowIfCounter_T));
   /*function GetIfCounter have show error message so here need return*/
   if (GetIfCounter(ifindex, &IfCounter) == TRUE)
   {
      memcpy(ctrl_P->CMenu.counters_table[idx], &IfCounter, sizeof(CLI_API_ShowIfCounter_T));
   }
#endif
   
    return CLI_NO_ERROR; 
}

#else

UI32_T CLI_API_Clear_Counters(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    CLI_API_ShowIfCounter_T IfCounter;
    UI32_T                  ifindex;
    UI32_T                  idx;

    if(arg[0]!=NULL)
    {
        switch(arg[0][0])
        {
            case 'e':
            case 'E':
            {
                if(arg[1]!=NULL)
                {
                    UI32_T                  verify_unit = atoi((char*)arg[1]);
                    UI32_T                  verify_port = atoi(strchr((char*)arg[1], '/')+1 );
                    CLI_API_EthStatus_T     verify_ret  = 0;
#if (CLI_SUPPORT_PORT_NAME == 1)
                    if (isdigit(arg[1][0]))
                    {
                        verify_unit = atoi(arg[1]);
                        verify_port = atoi(strchr(arg[1],'/')+1);
                    }
                    else/*port name*/
                    {
                        UI32_T trunk_id = 0;
                        UI32_T lport = 0;
                        if (!IF_PMGR_IfnameToIfindex(arg[1], &lport))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[1]);
#endif
                            return CLI_NO_ERROR;
                        }
                        SWCTRL_POM_LogicalPortToUserPort(lport, &verify_unit, &verify_port, &trunk_id);
                    }
#endif
                    verify_ethernet(verify_unit, verify_port, &ifindex);
                    idx = (verify_unit - 1) * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT + verify_port - 1;      /*idx is the port number reduce 1*/

                    if(verify_ret == CLI_API_ETH_NOT_PRESENT || verify_ret == CLI_API_ETH_UNKNOWN_PORT )
                    {
                        display_ethernet_msg(verify_ret, verify_unit, verify_port);
                        return CLI_NO_ERROR;
                    } 
                }
                else
                    return CLI_ERR_INTERNAL;
            }
                break;
          
            case 'p':
            case 'P':
            {
                if(arg[1]!=NULL)
                {
                    UI32_T  trunk_id = atoi((char*)arg[1]);
                    CLI_API_TrunkStatus_T verify_ret_t;

                    idx = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT + trunk_id -1;
                    if( (verify_ret_t = verify_trunk(trunk_id, &ifindex)) == CLI_API_TRUNK_NOT_EXIST)
                    {
                        display_trunk_msg(verify_ret_t, trunk_id);
                        return CLI_NO_ERROR;
                    }      
                }
                else
                    return CLI_ERR_INTERNAL;
            }
                break;
          
            default:
                return CLI_ERR_INTERNAL;	
        }
    }/*if(arg[0]!=NULL)*/
    else
        return CLI_ERR_INTERNAL;

    if(ctrl_P->CMenu.counters_table[idx] == NULL)
        if((ctrl_P->CMenu.counters_table[idx] = calloc(1, sizeof(CLI_API_ShowIfCounter_T))) == NULL)
            return CLII_ERR_MEMORY_NOT_ENOUGH;

    memset(&IfCounter, 0, sizeof(CLI_API_ShowIfCounter_T));
    /*function GetIfCounter have show error message so here need return*/
    if (GetIfCounter(ifindex, &IfCounter) == TRUE)
    {
        memcpy(ctrl_P->CMenu.counters_table[idx], &IfCounter, sizeof(CLI_API_ShowIfCounter_T));
    }
   
    return CLI_NO_ERROR; 
}

#endif /* #if(SYS_CPNT_SYSTEMWIDE_COUNTER == TRUE) */

/*configuration*/
UI32_T CLI_API_Shutdown_eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T i       = 0;
    UI32_T state;
   
    UI32_T lport;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;
   
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_SHUTDOWN:
            state = VAL_ifAdminStatus_down;
            break;
      
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_SHUTDOWN:
            state = VAL_ifAdminStatus_up;
            break;
   
        default:
            return CLI_ERR_INTERNAL;      
    }   

    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);

#if (SYS_CPNT_SWCTRL_SHUTDOWN_FOR_TRUNK_MEMBER == TRUE)
            if (verify_ret != CLI_API_ETH_OK && verify_ret != CLI_API_ETH_TRUNK_MEMBER)
#else
            if (verify_ret != CLI_API_ETH_OK)
#endif
            {
                display_ethernet_msg_for_cfg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else if (!CMGR_SetPortAdminStatus(lport, state))
            {
#if (CLI_SUPPORT_PORT_NAME == 1)  
                {             
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to %s interface on ethernet %s\r\n", state == VAL_ifAdminStatus_up ? "admin up" : "admin down", name);
#endif
                }
#else
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_3("Failed to %s interface on ethernet %lu/%lu\r\n", state == VAL_ifAdminStatus_up ? "admin up" : "admin down",
                                                                              (unsigned long)verify_unit, (unsigned long)verify_port);       
#endif
#endif
            }

#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)
            if (state == VAL_ifAdminStatus_up)
            {
                UI32_T psec_status, action_active;
				
                if ((TRUE==PSEC_PMGR_GetPortSecurityStatus(lport, &psec_status)) &&
					(VAL_portSecPortStatus_enabled==psec_status) &&
					(TRUE==PSEC_PMGR_GetPortSecurityActionActive(lport, &action_active)) &&
					(TRUE==action_active))
                    PSEC_PMGR_SetPortSecurityActionActive(lport, FALSE);

            }
#endif
        }
    }   
   


    return CLI_NO_ERROR; 
}

UI32_T CLI_API_Description_eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T lport;
    UI32_T i       = 0;
    UI32_T count   = 0;
    UI8_T  null_str = 0;         
   
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port = 0;
    CLI_API_EthStatus_T verify_ret;
   
    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)/*just allow set a port name*/
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            if(++count > 1)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_SetandShow_Exception_Handeler_Msg(EH_TYPE_MSG_CMD_NOT_FOR_MULT_PORTS, CLI_API_DESCRIPTION_ETH, NULL);        
#else
                 CLI_LIB_PrintStr("This command only for setting name of one port.\r\n");      
#endif
                return CLI_NO_ERROR;            
            }
        }
    }   

    if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
    {
        display_ethernet_msg_for_cfg(verify_ret, verify_unit, verify_port);
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_DESCRIPTION:
            if(arg[0]!=NULL)
            {
                if (!SWCTRL_PMGR_SetPortName(lport , (UI8_T *)arg[0]))
                {
#if (CLI_SUPPORT_PORT_NAME == 1)  
                {             
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to set port name on ethernet %s.\r\n", name);
#endif
                }
#else
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to set port name on ethernet %lu/%lu.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                }
            }
            else
                return CLI_ERR_INTERNAL;
                
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_DESCRIPTION:
            if (!SWCTRL_PMGR_SetPortName(lport , &null_str))
            {
#if (CLI_SUPPORT_PORT_NAME == 1)  
            {             
                UI8_T name[MAXSIZE_ifName+1] = {0};
                CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("Failed to remove port name on ethernet %s.\r\n", name);
#endif
            }
#else
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to remove port name on ethernet %lu/%lu.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
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

UI32_T CLI_API_Alias_eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SWCTRL_CONTAIN_CLI_ALIAS == TRUE)
    UI32_T lport;
    UI32_T i       = 0;
    UI32_T count   = 0;
    UI8_T  null_str = 0;         
   
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port = 0;
    CLI_API_EthStatus_T verify_ret;
   
    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)/*just allow set a port name*/
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            if(++count > 1)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_SetandShow_Exception_Handeler_Msg(EH_TYPE_MSG_CMD_NOT_FOR_MULT_PORTS, CLI_API_DESCRIPTION_ETH, NULL);        
#else
                CLI_LIB_PrintStr("This command only for setting name of one port.\r\n");      
#endif
                return CLI_NO_ERROR;            
            }
        }
    }   

    if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
    {
        display_ethernet_msg_for_cfg(verify_ret, verify_unit, verify_port);
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_ALIAS:
            if((arg[0]!=NULL)&&(strlen((char*)arg[0]) <= MAXSIZE_ifAlias))
            {
                if (!SWCTRL_PMGR_SetPortAlias(lport , (UI8_T *)arg[0]))
                {
#if (CLI_SUPPORT_PORT_NAME == 1)  
                {             
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to set port alias on ethernet %s.\r\n", name);
#endif
                }
#else
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to set port alias on ethernet %lu/%lu.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                }
            }
            else
                return CLI_ERR_INTERNAL;
                
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_ALIAS:
            if (!SWCTRL_PMGR_SetPortAlias(lport , &null_str))
            {
#if (CLI_SUPPORT_PORT_NAME == 1)  
            {             
                UI8_T name[MAXSIZE_ifName+1] = {0};
                CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("Failed to remove port alias on ethernet %s.\r\n", name);
#endif
            }
#else
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to remove port alias on ethernet %lu/%lu.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
            }
            break;
            
        default:
            return CLI_ERR_INTERNAL;      
    }   
#endif /* (SYS_CPNT_SWCTRL_CONTAIN_CLI_ALIAS == TRUE) */

    return CLI_NO_ERROR; 
}

UI32_T CLI_API_Speedduplex(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SPEED_DUPLEX == TRUE)
#define SPDPLX_DEFAULT 65535

    UI32_T speed_duplex    = 0;//pttch
    UI32_T i               = 0;
    UI32_T ifindex         = 0;

    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;
   
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_SPEEDDUPLEX:
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

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_SPEEDDUPLEX:
            speed_duplex = SPDPLX_DEFAULT; /*default: temp: SYS_CST_SW_AUTO*/
            break;

        default:
            return CLI_ERR_INTERNAL;
    }  

    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
         
            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg_for_cfg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else
            {
                if(speed_duplex == SPDPLX_DEFAULT)
                {
                    if (!SWCTRL_PMGR_SetPortDefaultSpeedDuplex(ifindex))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)  
                    {             
                        UI8_T name[MAXSIZE_ifName+1] = {0};
                        CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("Failed to set default speed-duplex on ethernet %s.\r\n", name);
#endif
                    }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set default speed-duplex on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);       
#endif
#endif
                    }
                }
                else 
                {
                    if (!SWCTRL_PMGR_SetPortCfgSpeedDuplex(ifindex, speed_duplex))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)  
                    {             
                        UI8_T name[MAXSIZE_ifName+1] = {0};
                        CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("Failed to set speed-duplex on ethernet %s.\r\n", name);
#endif
                    }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set speed-duplex on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);       
#endif
#endif
                    }
                }
            }
        }
    }
#endif   
   
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Negotiation(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NEGOTIATION == TRUE)
    UI32_T i;
    UI32_T lport;
    UI32_T state;
     
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;
     
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_NEGOTIATION:
            state = VAL_portAutonegotiation_enabled;
            break;
      
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_NEGOTIATION:
            state = VAL_portAutonegotiation_disabled;
            break;
   
        default:
            return CLI_ERR_INTERNAL;      
    }
   
    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
         
            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg_for_cfg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else if (!SWCTRL_PMGR_SetPortAutoNegEnable(lport, state))
            {
#if (CLI_SUPPORT_PORT_NAME == 1)  
            {             
                UI8_T name[MAXSIZE_ifName+1] = {0};
                CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to %s auto negotiation on ethernet %s.\r\n", state == VAL_portAutonegotiation_enabled ? "enable" : "disable", name);
#endif
            }
#else
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_3("Failed to %s auto negotiation on ethernet %lu/%lu\r\n", state == VAL_portAutonegotiation_enabled ? "enable" : "disable",
                                                                                     (unsigned long)verify_unit, (unsigned long)verify_port);       
#endif
#endif
            }
        }
    }               
#endif   
    return CLI_NO_ERROR; 
}

UI32_T CLI_API_Capabilities(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NEGOTIATION == TRUE)
    UI32_T capabilities    = 0;//pttch
    UI32_T i               = 0;
    UI32_T lport           = 0;
    Port_Info_T port_info;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;
   
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

    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            
            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg_for_cfg(verify_ret, verify_unit, verify_port);
                continue;  
            }
         
            memset(&port_info, 0, sizeof(Port_Info_T));
            SWCTRL_POM_GetPortInfo(lport, &port_info);
         
            switch(cmd_idx)
            {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_CAPABILITIES:
                    port_info.autoneg_capability |= capabilities;        
                    break;
        
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_CAPABILITIES:
                    if(capabilities == DEFAULT_CAPABILITIES)
                    {
                        if (!SWCTRL_PMGR_SetPortDefaultAutoNegCapability(lport))
                        {
#if (CLI_SUPPORT_PORT_NAME == 1)  
                            {             
                                UI8_T name[MAXSIZE_ifName+1] = {0};
                                CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr_1("Failed to set default capability on ethernet %s.\r\n", name);
#endif
                            }
#else
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_2("Failed to set default capability on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);       
#endif
#endif
                        }
                        continue;
                    }
                    else
                    {
                        port_info.autoneg_capability &= ~capabilities;
                    }
                    break;
        
                default:
                    return CLI_ERR_INTERNAL;
            }
         
            if (!SWCTRL_PMGR_SetPortAutoNegCapability(lport, port_info.autoneg_capability))
            {
#if (CLI_SUPPORT_PORT_NAME == 1)  
            {             
                UI8_T name[MAXSIZE_ifName+1] = {0};
                CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("Failed to set capability on ethernet %s.\r\n", name);
#endif
            }
#else
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to set capability on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);       
#endif
#endif
            }
        }
    }
#endif   
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Flowcontrol(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_FLOW_CONTROL==TRUE)
    UI32_T i;
    UI32_T lport;
    UI32_T flow_control_cfg;
   
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_FLOWCONTROL:
            flow_control_cfg = VAL_portFlowCtrlCfg_enabled;
            break;
      
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_FLOWCONTROL:
            flow_control_cfg =  VAL_portFlowCtrlCfg_disabled;
            break;
   
        default:
            return CLI_ERR_INTERNAL;      
    }   
   
    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            
            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg_for_cfg(verify_ret, verify_unit, verify_port);
                continue;  
            }
            else if (!SWCTRL_PMGR_SetPortCfgFlowCtrlEnable(lport, flow_control_cfg))
            {
#if (CLI_SUPPORT_PORT_NAME == 1)  
            {             
                UI8_T name[MAXSIZE_ifName+1] = {0};
                CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to %s flowcontrol on ethernet %s.\r\n", flow_control_cfg == VAL_portFlowCtrlCfg_enabled ? "enable" : "disable", name);
#endif
            }
#else
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_3("Failed to %s flowcontrol on ethernet %lu/%lu\r\n", flow_control_cfg == VAL_portFlowCtrlCfg_enabled ? "enable" : "disable",
                                                                                (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
            }
        }
    }          
#endif   
    return CLI_NO_ERROR; 
}

UI32_T CLI_API_Fec(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SWCTRL_FEC == TRUE)
    UI32_T i;
    UI32_T lport;
    UI32_T fec_mode = VAL_portFecMode_invalid;

    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_FEC:
            if(arg[0]!=NULL)
            {
                if (arg[0][0] == 'b' || arg[0][0] == 'B')
                    fec_mode = VAL_portFecMode_baseR;
                else if (arg[0][0] == 'r' || arg[0][0] == 'R')
                    fec_mode = VAL_portFecMode_rs;
                else
                    return CLI_ERR_INTERNAL;
            }
            else
                return CLI_ERR_INTERNAL;
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_FEC:
            fec_mode = VAL_portFecMode_disabled;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;

            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg_for_cfg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else if (!SWCTRL_PMGR_SetPortFec(lport, fec_mode))
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
                UI8_T name[MAXSIZE_ifName+1] = {0};
                CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("Failed to set FEC on ethernet %s.\r\n", name);
#endif
            }
#else
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to set FEC on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
            }
        }
    }
#endif /* (SYS_CPNT_SWCTRL_FEC == TRUE) */
    return CLI_NO_ERROR;
}

/************************************<<LACP>>*****************************/
UI32_T CLI_API_Lacp_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LACP == TRUE)
    UI32_T state   = 0;
    UI32_T i       = 0;
    UI32_T ifindex = 0;
   
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
   
    CLI_API_EthStatus_T verify_ret;
   
    switch(cmd_idx) 
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_LACP:
            state = VAL_lacpPortStatus_enabled;
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_LACP:
            state = VAL_lacpPortStatus_disabled;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
 
    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
         
            verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex);
      
            /*do not check if this port is trunk member or not   */
            /*and semantic check must be completed in LACP itself*/
            if(verify_ret == CLI_API_ETH_NOT_PRESENT || verify_ret == CLI_API_ETH_UNKNOWN_PORT)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;  
            }
            else if (LACP_PMGR_SetDot3adLacpPortEnabled(ifindex, state)!= LACP_RETURN_SUCCESS)
            {
#if (CLI_SUPPORT_PORT_NAME == 1)  
            {             
                UI8_T name[MAXSIZE_ifName+1] = {0};
                CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to %s LACP status on ethernet %s\r\n", state == VAL_lacpPortStatus_enabled ? "enable" : "disable", name);
#endif
            }
#else
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_3("Failed to %s LACP status on ethernet %lu/%lu\r\n", state == VAL_lacpPortStatus_enabled ? "enable" : "disable", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
            }
        }
    }
#endif
    return CLI_NO_ERROR;	
}

/************************************<<PORT SECURITY>>***************************************/
/*change mode*/

/*execution*/

/*configuration*/
UI32_T CLI_API_Port_Security_Action(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)

    UI32_T action_trap_status;
    UI32_T i       = 0;
   
    UI32_T lport;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_PORT_SECURITY_ACTION:
#ifdef ALLAYER_SWITCH
            action_trap_status = VAL_portSecAction_trapAndShutdown;
#endif

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

            break;
      
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_PORT_SECURITY_ACTION:
            action_trap_status = VAL_portSecAction_none; 
            break;
      
        default:
            return CLI_ERR_INTERNAL;
    }
    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;

            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
#if (SYS_CPNT_NETACCESS == TRUE)
            else if (!NETACCESS_PMGR_SetPortSecurityActionStatus(lport, action_trap_status))
#else
            else if (!PSEC_PMGR_SetPortSecurityActionStatus(lport, action_trap_status))
#endif
            {
#if (CLI_SUPPORT_PORT_NAME == 1)  
            {             
                UI8_T name[MAXSIZE_ifName+1] = {0};
                CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("Failed to set port %s port security action status\r\n", name);
#endif
            }
#else
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to set port %lu/%lu port security action status\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);       
#endif
#endif
            }

        }
    }   

#endif
    return CLI_NO_ERROR;
}


UI32_T CLI_API_Port_Security(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)
#if (CLI_SUPPORT_PSEC_MAC_COUNT == 1)
    UI32_T port_security_status = 0;
    UI32_T i       = 0;
   
    UI32_T lport;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T number = 0;
    CLI_API_EthStatus_T verify_ret;
   
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_PORT_SECURITY:
            if (arg[0] == NULL)
            {
                port_security_status = VAL_portSecPortStatus_enabled;
            }
            else if (arg[0][0] == 'm' || arg[0][0] == 'M')
            {
                number = atoi((char*)arg[1]);	
            }
            break;
      
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_PORT_SECURITY:
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
            return CLI_ERR_INTERNAL;
    }
    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;

            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            if (arg[0] == NULL)
            {
#if (SYS_CPNT_NETACCESS == TRUE)
                if(!NETACCESS_PMGR_SetPortSecurityStatus(lport, port_security_status))
#else
                if(!PSEC_PMGR_SetPortSecurityStatus(lport, port_security_status))
#endif
                {
#if (CLI_SUPPORT_PORT_NAME == 1)  
                {             
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to %s port %s security status\r\n", port_security_status == VAL_portSecPortStatus_enabled ? "enable" : "disable", name);
#endif
                }
#else
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_3("Failed to %s port %lu/%lu security status\r\n", port_security_status == VAL_portSecPortStatus_enabled ? "enable" : "disable",
                                                                              (unsigned long)verify_unit, (unsigned long)verify_port);       
#endif
#endif
                }
            }
            else if (arg[0][0] == 'm' || arg[0][0] == 'M')
            {
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
                                                                                 (unsigned long)verify_unit, (unsigned long)verify_port);       
#endif
                }
            }
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
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_PORT_SECURITY:
            port_security_status = VAL_portSecPortStatus_enabled;
            break;
      
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_PORT_SECURITY:
            port_security_status = VAL_portSecPortStatus_disabled;
            break;
      
        default:
            return CLI_ERR_INTERNAL;
    }

    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;

            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }

#if (SYS_CPNT_NETACCESS == TRUE)
            else if (!NETACCESS_PMGR_SetPortSecurityStatus(lport, port_security_status))
#else
            else if (!PSEC_PMGR_SetPortSecurityStatus(lport, port_security_status))
#endif
            {
#if (CLI_SUPPORT_PORT_NAME == 1)  
            {             
                UI8_T name[MAXSIZE_ifName+1] = {0};
                CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to %s port %s security status\r\n", port_security_status == VAL_portSecPortStatus_enabled ? "enable" : "disable", name);
#endif
            }
#else
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_3("Failed to %s port %lu/%lu security status\r\n", port_security_status == VAL_portSecPortStatus_enabled ? "enable" : "disable",
                                                                              (unsigned long)verify_unit, (unsigned long)verify_port);       
#endif
#endif
            }
        }
    }   
#endif
#endif
    return CLI_NO_ERROR; 
}


UI32_T CLI_API_Port_Security_Shutdown(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    return CLI_NO_ERROR; 	
}

UI32_T CLI_API_Port_Security_Trap(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    return CLI_NO_ERROR; 	
}

UI32_T CLI_API_TransceiverMonitor(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
    CLI_API_EthStatus_T verify_ret;
    UI32_T port, lport;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T sfp_index;
    BOOL_T trap_enable = FALSE;

    for (port = 1; port <= ctrl_P->sys_info.max_port_number; port++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((port-1)/8)]  & (1 << ( 7 - ((port-1)%8))) )
        {
            verify_port = port;
            switch(cmd_idx)
            {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_TRANSCEIVERMONITOR:
                    trap_enable = TRUE;
                    break;

                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_TRANSCEIVERMONITOR:
                    trap_enable = FALSE;
                    break;
                default:
                    return CLI_ERR_INTERNAL;
            }/*switch(cmd_idx)*/

            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
            if(verify_ret != CLI_API_ETH_OK && verify_ret != CLI_API_ETH_TRUNK_MEMBER)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }

            if(STKTPLG_POM_UserPortToSfpIndex(verify_unit, verify_port, &sfp_index)==FALSE)
            {
                CLI_LIB_PrintStr_2("Failed to set eth %lu/%lu because it is not a sfp port.\r\n",
                    (unsigned long)verify_unit, (unsigned long)verify_port);
                continue;
            }

            if (!SWCTRL_PMGR_SetPortSfpDdmTrapEnable(lport, trap_enable))
            {
                CLI_LIB_PrintStr_2("Failed to set transceiver monitor on eth %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
            }

        }
    }   
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_TransceiverThresholdAutoMode(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
    CLI_API_EthStatus_T verify_ret;
    UI32_T port, lport;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T sfp_index;
    BOOL_T auto_mode=FALSE;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_TRANSCEIVERTHRESHOLDAUTO:
            auto_mode = TRUE;
            break;
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_TRANSCEIVERTHRESHOLDAUTO:
            auto_mode = FALSE;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }/*switch(cmd_idx)*/

    for (port = 1; port <= ctrl_P->sys_info.max_port_number; port++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((port-1)/8)]  & (1 << ( 7 - ((port-1)%8))) )
        {
            verify_port = port;

            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
            if(verify_ret != CLI_API_ETH_OK && verify_ret != CLI_API_ETH_TRUNK_MEMBER)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }

            if(STKTPLG_POM_UserPortToSfpIndex(verify_unit, verify_port, &sfp_index)==FALSE)
            {
                CLI_LIB_PrintStr_2("Failed to set eth %lu/%lu because it is not a sfp port.\r\n",
                    (unsigned long)verify_unit, (unsigned long)verify_port);
                continue;
            }

            if (!SWCTRL_PMGR_SetPortSfpDdmThresholdAutoMode(lport, auto_mode))
            {
                CLI_LIB_PrintStr_2("Failed to set transceiver threshold auto mode on eth %lu/%lu\r\n",
                    (unsigned long)verify_unit, (unsigned long)verify_port);
            }
        }
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_TransceiverThreshold_Default(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
    CLI_API_EthStatus_T verify_ret;
    UI32_T threshold_type = 0;
    UI32_T port, lport;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T sfp_index;
    BOOL_T auto_mode = FALSE;

    for (port = 1; port <= ctrl_P->sys_info.max_port_number; port++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((port-1)/8)]  & (1 << ( 7 - ((port-1)%8))) )
        {
            verify_port = port;
            switch(cmd_idx)
            {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_TRANSCEIVERTHRESHOLD:
                    if(arg[0]!=NULL)
                    {
                        switch(arg[0][0])
                        {
                            /* rx power */
                            case 'r':
                            case 'R':
                                threshold_type = VAL_trapSfpThresholdAlarmWarnType_rxPowerHighAlarm;
                                break;             
                            /* tx power or temperature */
                            case 't':
                            case 'T':
                                if(arg[0][1]=='x' ||arg[0][1]=='X') /* tx power */
                                {
                                    threshold_type = VAL_trapSfpThresholdAlarmWarnType_txPowerHighAlarm;
                                }
                                else /* temperature */
                                {
                                    threshold_type = VAL_trapSfpThresholdAlarmWarnType_temperatureHighAlarm;
                                }
                                break;
                            /* voltage */
                            case 'v':
                            case 'V':
                                threshold_type = VAL_trapSfpThresholdAlarmWarnType_voltageHighAlarm;
                                break;   
                            /* current */
                            case 'c':
                            case 'C':
                                threshold_type = VAL_trapSfpThresholdAlarmWarnType_currentHighAlarm;
                                break;
                            default:
                                break;
                        }
                    }
                    else
                    {   /* 0 stands for all threshold*/
                        threshold_type = 0;
                    }
                    break;
                default:
                    return CLI_ERR_INTERNAL;
            }/*switch(cmd_idx)*/

            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
            if(verify_ret != CLI_API_ETH_OK && verify_ret != CLI_API_ETH_TRUNK_MEMBER)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }

            if(STKTPLG_POM_UserPortToSfpIndex(verify_unit, verify_port, &sfp_index)==FALSE)
            {
                CLI_LIB_PrintStr_2("Failed to set eth %lu/%lu because it is not a sfp port.\r\n",
                    (unsigned long)verify_unit, (unsigned long)verify_port);
                continue;
            }

            if(SWCTRL_POM_GetPortSfpDdmThresholdAutoMode(lport, &auto_mode)==FALSE)
            {
                CLI_LIB_PrintStr_2("Failed to get transceiver threshold auto mode on eth %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
                continue;
            }
            if(auto_mode == TRUE)
            {
                CLI_LIB_PrintStr_2("Failed to set eth %lu/%lu because it is configured to threshold auto mode.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
                continue;
            }
            if (!SWCTRL_PMGR_SetPortSfpDdmThresholdDefault(lport, threshold_type))
            {
                CLI_LIB_PrintStr_2("Failed to set default transceiver threshold on eth %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
            }

        }
    }   
#endif
    return CLI_NO_ERROR;	
}

UI32_T CLI_API_TransceiverThreshold(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
    CLI_API_EthStatus_T verify_ret;
    UI32_T threshold_type = 0;
    UI32_T port, lport;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    I32_T  val=0;
    UI32_T sfp_index;
    BOOL_T is_provision = FALSE, auto_mode = FALSE, need_to_check_range = TRUE;

    for (port = 1; port <= ctrl_P->sys_info.max_port_number; port++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((port-1)/8)]  & (1 << ( 7 - ((port-1)%8))) )
        {
            verify_port = port;
            switch(cmd_idx)
            {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_TRANSCEIVERTHRESHOLD:
                    if(arg[0]!=NULL)
                    {
                        switch(arg[0][0])
                        {
                            /* rx power */
                            case 'r':
                            case 'R':
                                if(arg[1]!=NULL)
                                {
                                    switch(arg[1][0])
                                    {
                                        case 'h':
                                        case 'H':
                                            if(arg[1][5]=='a'||arg[1][5]=='A')
                                            {
                                                threshold_type = VAL_trapSfpThresholdAlarmWarnType_rxPowerHighAlarm;
                                            }
                                            else if(arg[1][5]=='w'||arg[1][5]=='W')
                                            {
                                                threshold_type = VAL_trapSfpThresholdAlarmWarnType_rxPowerHighWarning;
                                            }
                                            break;
                                        case 'l':
                                        case 'L':
                                            if(arg[1][4]=='a'||arg[1][4]=='A')
                                            {
                                                threshold_type = VAL_trapSfpThresholdAlarmWarnType_rxPowerLowAlarm;
                                            }
                                            else if(arg[1][4]=='w'||arg[1][4]=='W')
                                            {
                                                threshold_type = VAL_trapSfpThresholdAlarmWarnType_rxPowerLowWarning;
                                            }
                                            break;
                                    }

                                    if(arg[2]!=NULL)
                                    {
                                        val = CLI_LIB_Atol(arg[2], 10);
                                    }
                                }
                                break;             
                            /* tx power or temperature */
                            case 't':
                            case 'T':
                                if(arg[0][1]=='x' ||arg[0][1]=='X') /* tx power */
                                {
                                    if(arg[1]!=NULL)
                                    {
                                        switch(arg[1][0])
                                        {
                                            case 'h':
                                            case 'H':
                                                if(arg[1][5]=='a'||arg[1][5]=='A')
                                                {
                                                    threshold_type = VAL_trapSfpThresholdAlarmWarnType_txPowerHighAlarm;
                                                }
                                                else if(arg[1][5]=='w'||arg[1][5]=='W')
                                                {
                                                    threshold_type = VAL_trapSfpThresholdAlarmWarnType_txPowerHighWarning;
                                                }
                                                break;
                                            case 'l':
                                            case 'L':
                                                if(arg[1][4]=='a'||arg[1][4]=='A')
                                                {
                                                    threshold_type = VAL_trapSfpThresholdAlarmWarnType_txPowerLowAlarm;
                                                }
                                                else if(arg[1][4]=='w'||arg[1][4]=='W')
                                                {
                                                    threshold_type = VAL_trapSfpThresholdAlarmWarnType_txPowerLowWarning;
                                                }
                                                break;
                                        }
    
                                        if(arg[2]!=NULL)
                                        {
                                            val = CLI_LIB_Atol(arg[2], 10);
                                        }
                                    }
                                }
                                else /* temperature */
                                {
                                    if(arg[1]!=NULL)
                                    {
                                        switch(arg[1][0])
                                        {
                                            case 'h':
                                            case 'H':
                                                if(arg[1][5]=='a'||arg[1][5]=='A')
                                                {
                                                    threshold_type = VAL_trapSfpThresholdAlarmWarnType_temperatureHighAlarm;
                                                }
                                                else if(arg[1][5]=='w'||arg[1][5]=='W')
                                                {
                                                    threshold_type = VAL_trapSfpThresholdAlarmWarnType_temperatureHighWarning;
                                                }
                                                break;
                                            case 'l':
                                            case 'L':
                                                if(arg[1][4]=='a'||arg[1][4]=='A')
                                                {
                                                    threshold_type = VAL_trapSfpThresholdAlarmWarnType_temperatureLowAlarm;
                                                }
                                                else if(arg[1][4]=='w'||arg[1][4]=='W')
                                                {
                                                    threshold_type = VAL_trapSfpThresholdAlarmWarnType_temperatureLowWarning;
                                                }
                                                break;
                                        }
    
                                        if(arg[2]!=NULL)
                                        {
                                            val = CLI_LIB_Atol(arg[2], 10);
                                        }
                                    }
                                }
                                break;
                            /* voltage */
                            case 'v':
                            case 'V':
                                if(arg[1]!=NULL)
                                {
                                    switch(arg[1][0])
                                    {
                                        case 'h':
                                        case 'H':
                                            if(arg[1][5]=='a'||arg[1][5]=='A')
                                            {
                                                threshold_type = VAL_trapSfpThresholdAlarmWarnType_voltageHighAlarm;
                                            }
                                            else if(arg[1][5]=='w'||arg[1][5]=='W')
                                            {
                                                threshold_type = VAL_trapSfpThresholdAlarmWarnType_voltageHighWarning;
                                            }
                                            break;
                                        case 'l':
                                        case 'L':
                                            if(arg[1][4]=='a'||arg[1][4]=='A')
                                            {
                                                threshold_type = VAL_trapSfpThresholdAlarmWarnType_voltageLowAlarm;
                                            }
                                            else if(arg[1][4]=='w'||arg[1][4]=='W')
                                            {
                                                threshold_type = VAL_trapSfpThresholdAlarmWarnType_voltageLowWarning;
                                            }
                                            break;
                                    }

                                    if(arg[2]!=NULL)
                                    {
                                        val = CLI_LIB_Atol(arg[2], 10);
                                    }
                                }
                                break;   
                            /* current */
                            case 'c':
                            case 'C':
                                if(arg[1]!=NULL)
                                {
                                    switch(arg[1][0])
                                    {
                                        case 'h':
                                        case 'H':
                                            if(arg[1][5]=='a'||arg[1][5]=='A')
                                            {
                                                threshold_type = VAL_trapSfpThresholdAlarmWarnType_currentHighAlarm;
                                            }
                                            else if(arg[1][5]=='w'||arg[1][5]=='W')
                                            {
                                                threshold_type = VAL_trapSfpThresholdAlarmWarnType_currentHighWarning;
                                            }
                                            break;
                                        case 'l':
                                        case 'L':
                                            if(arg[1][4]=='a'||arg[1][4]=='A')
                                            {
                                                threshold_type = VAL_trapSfpThresholdAlarmWarnType_currentLowAlarm;
                                            }
                                            else if(arg[1][4]=='w'||arg[1][4]=='W')
                                            {
                                                threshold_type = VAL_trapSfpThresholdAlarmWarnType_currentLowWarning;
                                            }
                                            break;
                                    }

                                    if(arg[2]!=NULL)
                                    {
                                        val = CLI_LIB_Atol(arg[2], 10);
                                    }
                                }
                                break;
                            default:
                                break;
                        }
                    }
                    else
                        return CLI_ERR_INTERNAL;
      
                    break;

                default:
                    return CLI_ERR_INTERNAL;
            }/*switch(cmd_idx)*/

            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
            if(verify_ret != CLI_API_ETH_OK && verify_ret != CLI_API_ETH_TRUNK_MEMBER)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }

            /* check threshold value if it is reasonable */
            if(STKTPLG_POM_UserPortToSfpIndex(verify_unit, verify_port, &sfp_index)==FALSE)
            {
                CLI_LIB_PrintStr_2("Failed to set eth %lu/%lu because it is not a sfp port.\r\n",
                    (unsigned long)verify_unit, (unsigned long)verify_port);
                continue;
            }

            if(SWCTRL_POM_GetPortSfpDdmThresholdAutoMode(lport, &auto_mode)==FALSE)
            {
                CLI_LIB_PrintStr_2("Failed to get transceiver threshold auto mode on eth %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
                continue;
            }
            if(auto_mode == TRUE)
            {
                CLI_LIB_PrintStr_2("Failed to set eth %lu/%lu because it is configured to threshold auto mode.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
                continue;
            }

            /* Adding provision check:
             *     When provisioning, the setup value may not pass the range
             *     comparison below.
             *     EX: 
             *     SYS_DFLT_SFP_THRESHOLD_RX_POWER_LOW_ALARM          (-21.5)
             *     SYS_DFLT_SFP_THRESHOLD_RX_POWER_LOW_WARNING        (-21)
             *     SYS_DFLT_SFP_THRESHOLD_RX_POWER_HIGH_WARNING       (-3.5)
             *     SYS_DFLT_SFP_THRESHOLD_RX_POWER_HIGH_ALARM         (-3)
             *     Assume the startup config is to setup rx_high_alarm with
             *     -4. Since other 3 thredhold are not set,
             *     they are assigned by default value. Thus, rx_high_alarm can
             *     not be set. 
             */
            CLI_MGR_Get_Provision_Flag(&is_provision);
            if(is_provision)
                need_to_check_range = FALSE;
            if (!SWCTRL_PMGR_SetPortSfpDdmThreshold(lport, threshold_type, val, need_to_check_range))
            {
                CLI_LIB_PrintStr_2("Failed to set transceiver threshold on eth %lu/%lu\r\n",(unsigned long)verify_unit, (unsigned long)verify_port);
            }
        }
    }
#endif/* End of #if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE) */
    return CLI_NO_ERROR;	
}

UI32_T CLI_API_ComboForcedMode(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_COMBO_PORT_FORCE_MODE == TRUE)
    UI32_T forcedmode = 0;
    UI32_T i       = 0;
   
    UI32_T lport;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;

#if (SYS_CPNT_COMBO_PORT_FORCED_MODE_SFP_SPEED == TRUE)
   UI32_T fiber_operator_mode = VAL_portType_thousandBaseT;
#endif

    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            switch(cmd_idx)
            {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_MEDIATYPE:
                    if(arg[0]!=NULL)
                    {
                    switch(arg[0][0])
                        {
                            case 'c':
                            case 'C':
                                /*if (arg[0][7] == 'f' || arg[0][7] == 'F')*/
                                forcedmode = VAL_portComboForcedMode_copperForced;
    /*pttch 2004/8/5 04:20 do not support this mode*/
#if 0
                                else
                                forcedmode = VAL_portComboForcedMode_copperPreferredAuto;
#endif
                                break;	
         
                            case 's':
                            case 'S':
                                 /*sfp-forced*/
                                if (arg[0][4] == 'f' || arg[0][4] == 'F')
                                {
                                    forcedmode = VAL_portComboForcedMode_sfpForced;
                 #if (SYS_CPNT_COMBO_PORT_FORCED_MODE_SFP_SPEED == TRUE)
                                    if(arg[1])
                                    {
                                    if(arg[1][3] == 'f' || arg[1][3] == 'F')
                                    {
                                        fiber_operator_mode = VAL_portType_hundredBaseFX;
                                    }
                                    else if (arg[1][3] == '0' || arg[1][3] == '0')
                                    {
                                        fiber_operator_mode = VAL_portType_thousandBaseSfp;
                                    }
                                    else if (arg[1][3] == 's' || arg[1][3] == 'S')  
                                    {
                                        fiber_operator_mode = VAL_portType_tenGBaseSFP;
                                    }									
                                    else if (arg[1][3] == 'q' || arg[1][3] == 'Q')  
                                    {
                                        fiber_operator_mode = VAL_portType_fortyGBaseQSFP;
                                    }									
                                    else if (arg[1][3] == 'g' || arg[1][3] == 'G')  
                                    {
                                        fiber_operator_mode = VAL_portType_hundredGBaseQSFP;
                                    }									
                                    else if (arg[1][0] == '2' && arg[1][1] == '5' &&
                                             (arg[1][3] == 'g' || arg[1][3] == 'G'))
                                    {
                                        fiber_operator_mode = VAL_portType_twentyFiveGBaseSFP;
                                    }									
                                    }
                                else
                                    {
                                        fiber_operator_mode = VAL_portType_other;
                                    }                     
                 #endif
                                }
                                else/*sfp-preferred-auto*/
                                {
                                    forcedmode = VAL_portComboForcedMode_sfpPreferredAuto;
                                }
                                break;	
             
                            default:
                                break;
                        }
                    }
                    else
                        return CLI_ERR_INTERNAL;
      
                    break;
            
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_MEDIATYPE:
                    /*if cooper port can set to none*/
                    forcedmode = 0;	
                    break;
            
                default:
                    return CLI_ERR_INTERNAL;
            }/*switch(cmd_idx)*/
            
            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);

            if (verify_ret != CLI_API_ETH_OK && verify_ret != CLI_API_ETH_TRUNK_MEMBER)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }

            if (forcedmode == 0) /* use default */
            {
                SWCTRL_POM_GetDefaultPortComboForcedMode(lport, &forcedmode);
#if (SYS_CPNT_COMBO_PORT_FORCED_MODE_SFP_SPEED == TRUE)
                SWCTRL_POM_GetDefaultPortComboForcedModeSfpSpeed(lport, &fiber_operator_mode);
#endif
            }

            if (!SWCTRL_PMGR_SetPortComboForcedMode(lport, forcedmode
#if (SYS_CPNT_COMBO_PORT_FORCED_MODE_SFP_SPEED == TRUE)
                    ,fiber_operator_mode
#endif
            ))
            {
#if (CLI_SUPPORT_PORT_NAME == 1)  
                {             
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to set port %s media type\r\n", name);
#endif
                }
#else
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to set port %lu/%lu media type\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);       
#endif
#endif
            }

        }
    }   
#endif
    return CLI_NO_ERROR;	
}

#if(SYS_CPNT_CHI==TRUE)
UI32_T CLI_API_MAC_MAX_COUNT(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T i       = 0;
   
    UI32_T lport;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T number = 0;
    CLI_API_EthStatus_T verify_ret;
   
    switch(cmd_idx)
    {
    	case /*PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_MACADDRESSTABLE_MAXMACCOUNT:*/ 36:
    	    if(srg[0]!=NULL)
                number = atoi(arg[0]);
            else
                return CLI_ERR_INTERNAL
    	    break;
    	
    	case /* PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_MACADDRESSTABLE_MAXMACCOUNT:*/ 77:
    	    number = 0 ;
    	    break;
    	
    	default: 
    	    return CLI_ERR_INTERNAL;
    } 
    
    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;

            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }

            if(!AMTR_PMGR_SetPortMaxDynamicMacCount(lport, number))
            {

                CLI_LIB_PrintStr("Failed to set max mac address count\r\n");       
            }

        }  
    } 
    return CLI_NO_ERROR;   
}
#endif   // end #if(SYS_CPNT_CHI==TRUE)

UI32_T CLI_API_Giga_Phy_Mode(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE==TRUE)   
    
    UI32_T forced_mode;
    UI32_T i       = 0; 
    UI32_T lport;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;
   
    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            switch(cmd_idx)
            {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_GIGAPHYMODE:
                    if(arg[0]!=NULL)
                    {
                        switch(arg[0][0])
                        {
                            case 'M':
                            case 'm':
                                forced_mode = VAL_portMasterSlaveModeCfg_master;
                                break;
                            
                            case 's':
                            case 'S':
                                forced_mode = VAL_portMasterSlaveModeCfg_slave;
                                break;
                            
                            default:
                                break;
                        }
                    }
                    else
                        return CLI_ERR_INTERNAL;
                        
                    break;
                
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_GIGAPHYMODE:            
                    forced_mode = SYS_DFLT_FORCED_1000BASE_T_MASTER_SLAVE_MODE;
                    break;
                
                default:
                    break;
            }
            
            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else 
            {
            	if (!SWCTRL_SetPort1000BaseTForceMode(lport, forced_mode))
            	{
                    CLI_LIB_PrintStr("Failed to set giga copper port.\n\r");       
                }
            }
        }
    }
        
#endif
    return CLI_NO_ERROR;	
}


UI32_T CLI_API_Switchport_Mtu_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SWCTRL_MTU_CONFIG_MODE == SYS_CPNT_SWCTRL_MTU_PER_PORT )   
   UI32_T i 			  = 0;
   UI32_T ifindex		  = 0;
   UI32_T mtu = 0, jumbo_frame_status,system_mtu,system_jumbo_mtu;
   BOOL_T set;

   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port = 0;
   CLI_API_EthStatus_T verify_ret;


#if 1
   switch( cmd_idx )
   {
   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_SWITCHPORT_MTU:
	  mtu = CLI_LIB_AtoUl(arg[0], 10);
	  set = TRUE;
	  break;

   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_SWITCHPORT_MTU:
	  mtu = SYS_ADPT_MAX_FRAME_SIZE - 4;/* remove tag */
	  set = FALSE;
	  break;

   default:
	  return CLI_ERR_INTERNAL;
   }
#endif


	for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
	{
		if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
		{
			verify_port = i;

			if( (verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
			{
			   display_ethernet_msg(verify_ret, verify_unit, verify_port);
			   continue;
			}

			if (!SWCTRL_PMGR_SetPortMTU(ifindex, mtu))
			{
				goto error_code;
			}
		}
	}


	if (set == FALSE)
	{
		if (!SWCTRL_POM_GetJumboFrameStatus(&jumbo_frame_status))
		{
			goto error_code;
		}
		else
		{
			if (jumbo_frame_status == SWCTRL_JUMBO_FRAME_ENABLE)
			{
				if (!SWCTRL_PMGR_SetJumboFrameStatus(SWCTRL_JUMBO_FRAME_DISABLE))
				{
					goto error_code;
				}

				if (!SWCTRL_PMGR_SetJumboFrameStatus(SWCTRL_JUMBO_FRAME_ENABLE))
				{
					goto error_code;
				}
			}
		}

		if (!SWCTRL_PMGR_GetSystemMTU(&system_jumbo_mtu, &system_mtu))
		{
			goto error_code;
		}
		else
		{
			if (SYS_DFLT_PORT_MTU != system_jumbo_mtu)
			{
				if (!SWCTRL_PMGR_SetSystemMTU(TRUE, system_jumbo_mtu))
				{
					goto error_code;
				}
			}

			if (SYS_DFLT_PORT_MTU != system_mtu)
			{
				if (!SWCTRL_PMGR_SetSystemMTU(FALSE, system_mtu))
				{
					goto error_code;
				}
			}
		}
	}
   	return CLI_NO_ERROR;

error_code:
#if (CLI_SUPPORT_PORT_NAME == 1)
				{
					UI8_T name[MAXSIZE_ifName+1] = {0};
					CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
					CLI_API_Show_Exception_Handeler_Msg();
#else
					CLI_LIB_PrintStr_2("Failed to %s port MTU on ethernet %s\r\n", set ? "set" : "unset", name);
#endif
				}
#else
#if (SYS_CPNT_EH == TRUE)
				CLI_API_Show_Exception_Handeler_Msg();
#else
				CLI_LIB_PrintStr_3("Failed to %s port MTU on ethernet %lu/%lu\r\n", set ? "set" : "unset",
															 (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
	return CLI_ERR_CMD_INCOMPLETE;
#else
   	return CLI_NO_ERROR;
#endif /* end of #if (SYS_CPNT_SWCTRL_MTU_PER_PORT == TRUE) */
}



UI32_T CLI_API_Switchport_Mtu_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRUNK_UI==TRUE)
#if (SYS_CPNT_SWCTRL_MTU_CONFIG_MODE == SYS_CPNT_SWCTRL_MTU_PER_PORT )   
   UI32_T trunk_ifindex		  = 0;
   UI32_T mtu = 0, system_mtu,system_jumbo_mtu,jumbo_frame_status;
   BOOL_T set;
#if 1
   switch( cmd_idx )
   {
   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_SWITCHPORT_MTU:
	  mtu = CLI_LIB_AtoUl(arg[0], 10);
	  set = TRUE;
	  break;

   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_SWITCHPORT_MTU:
	  mtu = SYS_ADPT_MAX_FRAME_SIZE - 4;/* remove tag */
	  set = FALSE;
	  break;

   default:
	  return CLI_ERR_INTERNAL;
   }
#endif
	SWCTRL_POM_TrunkIDToLogicalPort(ctrl_P->CMenu.pchannel_id, &trunk_ifindex);

	if (!SWCTRL_PMGR_SetPortMTU(trunk_ifindex, mtu))
	{
		goto error_code;
	}

	if (set == FALSE)
	{
		if (!SWCTRL_POM_GetJumboFrameStatus(&jumbo_frame_status))
		{
			goto error_code;
		}
		else
		{
			if (jumbo_frame_status == SWCTRL_JUMBO_FRAME_ENABLE)
			{
				if (!SWCTRL_PMGR_SetJumboFrameStatus(SWCTRL_JUMBO_FRAME_DISABLE))
				{
					goto error_code;
				}

				if (!SWCTRL_PMGR_SetJumboFrameStatus(SWCTRL_JUMBO_FRAME_ENABLE))
				{
					goto error_code;
				}

			}
		}

		if (!SWCTRL_PMGR_GetSystemMTU(&system_jumbo_mtu, &system_mtu))
		{
			goto error_code;
		}
		else
		{
			if (SYS_DFLT_PORT_MTU != system_jumbo_mtu)
			{
				if (SWCTRL_PMGR_SetSystemMTU(TRUE, system_jumbo_mtu))
				{
					goto error_code;
				}
			}
			if (SYS_DFLT_PORT_MTU != system_mtu)
			{
				if (SWCTRL_PMGR_SetSystemMTU(FALSE, system_mtu))
				{
					goto error_code;
				}
			}
		}
	}
   return CLI_NO_ERROR;

error_code:

#if (SYS_CPNT_EH == TRUE)
			CLI_API_Show_Exception_Handeler_Msg();
#else
			CLI_LIB_PrintStr_2("Failed to %s port MTU on port channel %lu\r\n",
			set ? "set" : "unset", (unsigned long)ctrl_P->CMenu.pchannel_id);
#endif
	return CLI_ERR_CMD_INCOMPLETE;

#else
#endif /* end of #if (SYS_CPNT_SWCTRL_MTU_PER_PORT == TRUE) */
#endif  /* #if (SYS_CPNT_TRUNK_UI==TRUE) */
   return CLI_NO_ERROR;
}

/*fuzhimin,20090106*/
UI32_T CLI_API_Interface_Ethernet0(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IP_FOR_ETHERNET0 == TRUE)
   ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_INTERFACE_ETH0_MODE;
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Mdix_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SWCTRL_MDIX_CONFIG == TRUE)
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T mdix_mode;
    UI32_T i = 0;
    Port_Info_T port_info;


    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_MDIX:
        for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                switch ( arg[0][0] )
                {
                    case 'a':	/* auto */
                    case 'A':
                        mdix_mode = VAL_portMdixMode_auto;
                        break;
                    	
                    case 'c':   /* crossover */
                    case 'C':
                        mdix_mode = VAL_portMdixMode_crossover;
                        break;
                    	
                    case 's':   /* straight */
                    case 'S':
                        mdix_mode = VAL_portMdixMode_straight;
                        break;                    		

                    default:
                        return CLI_ERR_INTERNAL;
                }
#if (SYS_CPNT_MDIX_SUPPORT_GIGA_ETHERNET_COPPER == TRUE)

                /* when link status change to up and speed is 1000
                 * we modify the MDIX mode to auto
                 * because other MDIX setting not support 1000T 
                 */
                SWCTRL_GetPortInfo(i, &port_info);

                if (port_info.speed_duplex_oper == VAL_portSpeedDpxCfg_fullDuplex1000 && port_info.link_status == SWCTRL_LINK_UP)
                {
                    CLI_LIB_PrintStr("Failed to set MDIX\r\n");
                    continue;
                }
#endif /* #if (SYS_CPNT_MDIX_SUPPORT_GIGA_ETHERNET_COPPER == TRUE) */
                if(!SWCTRL_PMGR_SetMDIXMode(i, mdix_mode))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set MDIX\r\n");       
#endif
                }
            }
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_MDIX:
        for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                /* reset to default value: auto */
                if(!SWCTRL_PMGR_SetMDIXMode(i, VAL_portMdixMode_auto))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set MDIX\r\n");       
#endif
                }
            }
        }
        break;

    default:
        return CLI_ERR_INTERNAL;
    }
#endif /* (SYS_CPNT_SWCTRL_MDIX_CONFIG == TRUE) */

    return CLI_NO_ERROR;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - CLI_API_PowerSave
 *-----------------------------------------------------------
 * FUNCTION: Use the function to set ths status of power saving
 *             on normal port
 * INPUT   : cmd_idx - command index
 *           *arg[]  - the pointer to the arguments array
 *           *ctrl_P - the pointer to the cli task working area
 * OUTPUT  : None.
 * RETURN  : CLI_NO_ERROR/CLI_ERR_INTERNAL.
 * NOTE    : None.
 *----------------------------------------------------------*/
UI32_T CLI_API_PowerSave(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_POWER_SAVE==TRUE)
    UI32_T i;
    UI32_T lport;
    BOOL_T status;
   
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;
   
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_POWERSAVE:
            status = VAL_portPowerSave_enabled;
            break;
      
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_POWERSAVE:
            status =  VAL_portPowerSave_disabled;
            break;
   
        default:
            return CLI_ERR_INTERNAL;      
    }   

    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;          
            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {         
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;  
            }
            else if (!SWCTRL_PMGR_SetPortPowerSave(lport,status))
            {       
#if (CLI_SUPPORT_PORT_NAME == 1)             
                UI8_T name[MAXSIZE_ifName+1] = {0};
                CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to %s power-save on ethernet %s.\r\n", status == VAL_portPowerSave_enabled ? "enable" : "disable", name);
#endif
#else
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_3("Failed to %s power-save on ethernet %lu/%lu\r\n", status == VAL_portPowerSave_enabled ? "enable" : "disable",
                                                                                (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
            }
        }
    }          
#endif   
    return CLI_NO_ERROR; 
}

#if (SYS_CPNT_NMTR_HISTORY == TRUE)
static UI32_T CLI_API_Interface_History(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P, BOOL_T no_form, BOOL_T is_trunk)
{
    UI32_T arg_chk_idx;
    UI32_T i;
    UI32_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port = 0;
    UI32_T verify_trunk_id = 0;
    UI32_T lport;
    UI32_T min_port_id, max_port_id;

    enum {
        ACT_CREATE,
        ACT_DESTROY,
        ACT_SET_DEFAULT,
    } act;
    NMTR_TYPE_HistCtrlInfo_T ctrl_info;
    BOOL_T ret;

    /* extract arguments
     *
     * history <name> <interval> <buckets>
     * no history [<name>]
     */
    arg_chk_idx = 0;

    if (arg[arg_chk_idx])
    {
        strncpy(ctrl_info.name, arg[arg_chk_idx++], sizeof(ctrl_info.name)-1);
        ctrl_info.name[sizeof(ctrl_info.name)-1] = 0;

        act = no_form ? ACT_DESTROY : ACT_CREATE;
    }
    else
    {
        act = ACT_SET_DEFAULT;
    }

    if (act == ACT_CREATE)
    {
        if (!arg[arg_chk_idx] || !arg[arg_chk_idx+1])
        {
            return CLI_ERR_INTERNAL;
        }
        ctrl_info.interval = atoi(arg[arg_chk_idx]);
        ctrl_info.buckets_requested = atoi(arg[arg_chk_idx+1]);
        arg_chk_idx += 2;
    }

    /* perform config
     */
    if (is_trunk)
    {
        min_port_id = max_port_id = ctrl_P->CMenu.pchannel_id;
    }
    else
    {
        min_port_id = 1;
        max_port_id = ctrl_P->sys_info.max_port_number;
    }

    for (i = min_port_id ; i <= max_port_id; i++)
    {
        if (is_trunk)
        {
            verify_trunk_id = i;

            if (CLI_API_TRUNK_OK != (verify_ret = verify_trunk(verify_trunk_id, &lport)))
            {
                display_trunk_msg(verify_ret, verify_trunk_id);
                continue;
            }
        }
        else
        {
            if (!(ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8)))))
            {
                continue;
            }

            verify_port = i;

            if (CLI_API_ETH_OK != (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)))
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
        }

        ctrl_info.data_source = lport;

        switch (act)
        {
            case ACT_CREATE:
                ret = NMTR_PMGR_SetHistoryCtrlEntryByNameAndDataSrc(&ctrl_info);
                break;
            case ACT_DESTROY:
                ret = NMTR_PMGR_DestroyHistoryCtrlEntryByNameAndDataSrc(&ctrl_info);
                break;
            case ACT_SET_DEFAULT:
                ret = NMTR_PMGR_SetDefaultHistoryCtrlEntry(lport);
                break;
            default:
                ret = FALSE;
        }

        if (!ret)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            if (is_trunk)
            {
                CLI_LIB_PrintStr_2("Failed to %s periodic sampling of interface counters on trunk %lu\r\n", no_form?"destroy":"create", (unsigned long)verify_trunk_id);
            }
            else
            {
                CLI_LIB_PrintStr_3("Failed to %s periodic sampling of interface counters on ethernet %lu/%lu\r\n", no_form?"destroy":"create", (unsigned long)verify_unit, (unsigned long)verify_port);
            }
#endif
        }
    } /* end of for (i) */

    return CLI_NO_ERROR; 
}
#endif /* (SYS_CPNT_NMTR_HISTORY == TRUE) */

UI32_T CLI_API_Interface_History_eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NMTR_HISTORY == TRUE)
    BOOL_T is_trunk = FALSE;
    BOOL_T no_form;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_HISTORY:
            no_form = FALSE;
            break;
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_HISTORY:
            no_form = TRUE;
            break;
        default:
            return CLI_ERR_INTERNAL;
    }

    return CLI_API_Interface_History(cmd_idx, arg, ctrl_P, no_form, is_trunk);
#else
    return CLI_NO_ERROR;
#endif /* (SYS_CPNT_NMTR_HISTORY == TRUE) */
}

UI32_T CLI_API_Interface_History_pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRUNK_UI==TRUE)
#if (SYS_CPNT_NMTR_HISTORY == TRUE)
    BOOL_T is_trunk = TRUE;
    BOOL_T no_form;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W1_HISTORY:
            no_form = FALSE;
            break;
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_NO_HISTORY:
            no_form = TRUE;
            break;
        default:
            return CLI_ERR_INTERNAL;
    }

    return CLI_API_Interface_History(cmd_idx, arg, ctrl_P, no_form, is_trunk);
#endif /* (SYS_CPNT_NMTR_HISTORY == TRUE) */
#endif  /* #if (SYS_CPNT_TRUNK_UI==TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Switch_Mode(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)
    UI32_T i       = 0;
    UI32_T mode;
    UI32_T lport;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;
 

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_SWITCHINGMODE:
            if (arg[0][0] == 'c' || arg[0][0] == 'C') /*cut-through*/
            {
                mode = VAL_swctrlSwitchModeCT;
            }
            else if (arg[0][0] == 's' || arg[0][0] == 'S') /*store-and-forward*/
            {
                mode = VAL_swctrlSwitchModeSF;
            }
            break;	
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_SWITCHINGMODE:
            mode = SYS_DFLT_SWCTRL_SWITCH_MODE;
            break;
        default:
            return CLI_ERR_INTERNAL;
    }

    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;

            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else if (!SWCTRL_PMGR_SetSwitchingMode(lport, mode))
            {
#if (CLI_SUPPORT_PORT_NAME == 1)  
            {             
                UI8_T name[MAXSIZE_ifName+1] = {0};
                CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("Failed to set port %s switching mode\r\n", name);
#endif
            }
#else
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to set port %lu/%lu port switching mode\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);       
#endif
#endif
            }

        }
    }   

#endif /*#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)*/
    return CLI_NO_ERROR;
}

