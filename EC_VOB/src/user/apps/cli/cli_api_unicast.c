#include <stdio.h>
#include "sys_dflt.h"
#include "cli_api.h"
#include "cli_api_unicast.h"


/************************************<<UNICAST STORM CONTROL>>*****************************/
/*configuration*/


UI32_T CLI_API_Switchport_Unicast(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)
    UI32_T i = 0;
    UI32_T lport = 0;
     
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;
    UI32_T packet_rate;
    SWCTRL_UnknownUcastStormEntry_T unkucast_storm_entry;

    memset(&unkucast_storm_entry, 0x0, sizeof(SWCTRL_UnknownUcastStormEntry_T)); 
   
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_SWITCHPORT_UNKNOWNUNICAST:
            
        packet_rate = atoi((char*)arg[1]);
            for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
            {
                if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
                {
                    verify_port = i;
            
                    verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
            
                    if(verify_ret ==  CLI_API_ETH_NOT_PRESENT || verify_ret ==  CLI_API_ETH_UNKNOWN_PORT)
                    {
                        display_ethernet_msg(verify_ret, verify_unit, verify_port);
                        continue;  
                    }            
                    else
                    {
                        unkucast_storm_entry.unknown_ucast_storm_ifindex=lport;
                        if(!SWCTRL_POM_GetUnkucastStormEntry(&unkucast_storm_entry))
                        {
                            CLI_LIB_PrintStr_1("Failed to set unknown unicast storm on port %lu\r\n",(unsigned long)lport);
                            continue;                            
                        }
                        if(unkucast_storm_entry.unknown_ucast_storm_status!=VAL_unkucastStormStatus_enabled)
                        {
                            if (!SWCTRL_PMGR_SetUnknownUnicastStormStatus(lport, VAL_unkucastStormStatus_enabled))
                            {
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr_1("Failed to enable unknown unicast storm on port %lu\r\n",(unsigned long)lport);            
#endif
                                continue; 
                            }              
                        }              
                        if (!SWCTRL_PMGR_SetUnknownUStormControlRateLimit(lport, SYS_DFLT_UNKUSTORM_TYPE, packet_rate))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
#if (SYS_CPNT_SWCTRL_STORM_GRANULARITY_VALIDATION==TRUE)
                            UI32_T granularity;
                        
                            if(SWCTRL_POM_GetPortStormGranularity(lport, &granularity))
                            {                         
                                CLI_LIB_PrintStr_3("Failed to set unknown unicast storm on port %lu/%lu. Granularity is %ld.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port, (long)granularity);
                                if(unkucast_storm_entry.unknown_ucast_storm_status==VAL_unkucastStormStatus_enabled)
                                {
                                    continue;
                                }
                            }
                            else
                            {
                                CLI_LIB_PrintStr_2("Failed to set unknown unicast storm on port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
                            }
#else
                            CLI_LIB_PrintStr_2("Failed to set unknown unicast storm on port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif  /* #if (SYS_CPNT_SWCTRL_STORM_GRANULARITY_VALIDATION==TRUE) */
                            
#endif
                            /*restore*/
                            SWCTRL_PMGR_SetUnknownUnicastStormStatus(lport, VAL_unkucastStormStatus_disabled);
                        }          
                    }
                }
            }
            break;
    
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_SWITCHPORT_UNKNOWNUNICAST:
            for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
            {
                if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
                {
                    verify_port = i;
             
                    verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
            
                    if(verify_ret ==  CLI_API_ETH_NOT_PRESENT || verify_ret ==  CLI_API_ETH_UNKNOWN_PORT)
                    {
                        display_ethernet_msg(verify_ret, verify_unit, verify_port);
                        continue;  
                    }                        
                    else if (!SWCTRL_PMGR_SetUnknownUnicastStormStatus(lport, VAL_unkucastStormStatus_disabled))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("Failed to disable unknown unicast storm on port %lu\r\n",(unsigned long)lport);            
#endif
                    }
                }
            }
            break;
      
        default:
            return CLI_ERR_INTERNAL;   
    }
#endif /* (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM) */

    return CLI_NO_ERROR;    
}



UI32_T CLI_API_Switchport_Unicast_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRUNK_UI==TRUE)
#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)
#if (SYS_CPNT_UNKNOWN_USTORM_SUPPORT_LPORT == TRUE)
    UI32_T lport = 0;
    CLI_API_TrunkStatus_T verify_ret;
    UI32_T packet_rate;
    SWCTRL_UnknownUcastStormEntry_T unkucast_storm_entry;

    memset(&unkucast_storm_entry, 0x0, sizeof(SWCTRL_UnknownUcastStormEntry_T)); 
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_SWITCHPORT_UNKNOWNUNICAST:

            packet_rate = atoi(arg[1]);

            if( (verify_ret = verify_trunk(ctrl_P->CMenu.pchannel_id, &lport)) != CLI_API_TRUNK_OK)
            {
                display_trunk_msg(verify_ret, ctrl_P->CMenu.pchannel_id);
                return CLI_NO_ERROR;
            }
            else
            {
                unkucast_storm_entry.unknown_ucast_storm_ifindex=lport;
                if(!SWCTRL_POM_GetUnkucastStormEntry(&unkucast_storm_entry))
                {
                    CLI_LIB_PrintStr_1("Failed to set unknown unicast on port %lu\r\n",(unsigned long)lport);
                    return CLI_NO_ERROR;
                                            
                }
                if(unkucast_storm_entry.unknown_ucast_storm_status!=VAL_unkucastStormStatus_enabled)
                {
                    if (SWCTRL_PMGR_SetUnknownUnicastStormStatus(lport, VAL_unkucastStormStatus_enabled) != TRUE)
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("Failed to enable unknown unicast storm on port %lu\r\n",(unsigned long)lport);
#endif
                    }
                }
                if (SWCTRL_PMGR_SetUnknownUStormControlRateLimit(lport, SYS_DFLT_UNKUSTORM_TYPE, packet_rate) != TRUE)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
#if (SYS_CPNT_SWCTRL_STORM_GRANULARITY_VALIDATION==TRUE)
                    UI32_T granularity;
                        
                    if(SWCTRL_POM_GetPortStormGranularity(lport, &granularity))
                    {
                        CLI_LIB_PrintStr_2("Failed to set unknown unicast storm on trunk %lu. Granularity is %ld.\r\n",(unsigned long)ctrl_P->CMenu.pchannel_id, (long)granularity);
                        if(unkucast_storm_entry.unknown_ucast_storm_status==VAL_unkucastStormStatus_enabled)
                        {
                            break;
                        }
                    }
                    else
                    {
                        CLI_LIB_PrintStr_1("Failed to set unknown unicast storm on trunk %lu\r\n",(unsigned long)ctrl_P->CMenu.pchannel_id);
                    }
#else
                    CLI_LIB_PrintStr_1("Failed to set unknown unicast storm on trunk %lu\r\n",(unsigned long)ctrl_P->CMenu.pchannel_id);
#endif  /* #if (SYS_CPNT_SWCTRL_STORM_GRANULARITY_VALIDATION==TRUE) */                         
                    
#endif
                    /*restore*/
                    SWCTRL_PMGR_SetUnknownUnicastStormStatus(lport, VAL_unkucastStormStatus_disabled);
                }
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_SWITCHPORT_UNKNOWNUNICAST:
            verify_ret = verify_trunk(ctrl_P->CMenu.pchannel_id, &lport);
            if(verify_ret != CLI_API_TRUNK_OK)
            {
                display_trunk_msg(verify_ret, ctrl_P->CMenu.pchannel_id);
                return CLI_NO_ERROR;
            }
            else if (SWCTRL_PMGR_SetUnknownUnicastStormStatus(lport, VAL_unkucastStormStatus_disabled) != TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("Failed to disable unknown unicast storm on trunk %lu\r\n",(unsigned long)ctrl_P->CMenu.pchannel_id);
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif /* (SYS_CPNT_UNKNOWN_USTORM_SUPPORT_LPORT == TRUE) */
#endif /* (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM) */
#endif  /* #if (SYS_CPNT_TRUNK_UI==TRUE) */
   return CLI_NO_ERROR;
}
