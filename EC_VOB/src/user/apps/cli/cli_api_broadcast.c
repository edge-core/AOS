#include <ctype.h>
#include <stdio.h>
#include "sys_dflt.h"
#include "cli_api.h"
#include "cli_api_broadcast.h"
#include "swctrl_pmgr.h"

#if (SYS_CPNT_ATC_STORM == TRUE)
UI32_T show_one_port(UI32_T unit, UI32_T port,SWCTRL_ATCBroadcastStormEntry_T bcast_entry, SWCTRL_ATCMulticastStormEntry_T mcast_entry,UI32_T line_num);
#endif

/************************************<<BROADCAST STORM CONTROL>>*****************************/
/*change mode*/

/*execution*/
UI32_T CLI_API_Broadcast_global(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if defined(UNICORN)/*special for UNICORN*/
    UI32_T i = 0;
    UI32_T lport = 0;
    BOOL_T is_global_set_success = TRUE;     
    UI32_T verify_unit = ctrl_P->sys_info.my_unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;
    UI32_T packet_rate;
   

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W1_BROADCAST:
            if(arg[0] == NULL) /*enable by default*/
            {
#ifdef ALLAYER_SWITCH /*temp*/
                packet_rate = 256;
#else
                packet_rate = 500;
#endif
            }
            else
            {
                if(arg[1]!=NULL)
                {
#ifdef ALLAYER_SWITCH /*16*/ /*128*/ /*256*/ /*64*/

                    switch(arg[1][0])
                    {
                        case '1':
                            switch(arg[1][1])
                            {
                                case '2':
                                    packet_rate = 128;
                                    break;
               
                                case '6':
                                    packet_rate = 16;
                                    break;
               
                                default:
                                    return CLI_ERR_INTERNAL;
                            }
                            break;
            
                        case '2':
                            packet_rate = 256;
                            break;
            
                        case '6':
                            packet_rate = 64;
                            break;
            
                        default:
                            return CLI_ERR_INTERNAL;
                    }
#else    /*<500-262143>*/
                    packet_rate = atoi(arg[1]);
#endif
                }
            }
            for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
            {
                verify_port = i;
          
                verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
           
                if(verify_ret ==  CLI_API_ETH_NOT_PRESENT || verify_ret ==  CLI_API_ETH_UNKNOWN_PORT)
                {
                    //display_ethernet_msg(verify_ret, verify_unit, verify_port);
                    continue;  
                }            
                else
                {
                    if (!SWCTRL_PMGR_SetBroadcastStormStatus(lport, VAL_bcastStormStatus_enabled))
                    {
                    //CLI_LIB_PrintStr_1("Failed to enable broadcast storm on port %lu\r\n",(unsigned long)lport);            
                        continue; 
                    }
                    else if (!SWCTRL_PMGR_SetBStormControlRateLimit(lport, SYS_DFLT_BSTORM_TYPE, packet_rate))
                    {
                        is_global_set_success = FALSE;
                        /*restore*/
                        SWCTRL_PMGR_SetBroadcastStormStatus(lport, VAL_bcastStormStatus_disabled);
                    }
                }
            }

            if (!is_global_set_success)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set broadcast storm\r\n");   
#endif
            }
            break;
    
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_BROADCAST:
            for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
            {
                verify_port = i;
             
                verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
            
                if(verify_ret ==  CLI_API_ETH_NOT_PRESENT || verify_ret ==  CLI_API_ETH_UNKNOWN_PORT)
                {
                    //display_ethernet_msg(verify_ret, verify_unit, verify_port);
                    continue;  
                }                        
                else if (!SWCTRL_PMGR_SetBroadcastStormStatus(lport, VAL_bcastStormStatus_disabled))
                {
                    is_global_set_success = FALSE;
                    break;
                }
            }

            if (!is_global_set_success)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to disable broadcast storm\r\n");              
#endif
            }
            break;
      
        default:
            return CLI_ERR_INTERNAL;   
    }

#endif
    return CLI_NO_ERROR; 
}

/*configuration*/
UI32_T CLI_API_Switchport_Broadcast(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_BSTORM)
#if !defined(UNICORN)/*special for UNICORN*/
    UI32_T i = 0;
    UI32_T lport = 0;
     
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;
    UI32_T packet_rate = 0;
    SWCTRL_BcastStormEntry_T bcast_storm_entry;
   
    memset(&bcast_storm_entry, 0x0, sizeof(SWCTRL_BcastStormEntry_T)); 
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_SWITCHPORT_BROADCAST:
            if(arg[0] == NULL) /*enable by default*/
            
#if defined(STRAWMAN)/*special for STRAWMAN*/
            {
                packet_rate = 256;
            }
            else                
            {
                if(arg[2]!=NULL)
                {
                    switch(arg[2][0])
                    {
                        case '1':
                            switch(arg[2][1])
                            {
                                case '2':
                                    packet_rate = 128;
                                    break;
               
                                case '6':
                                    packet_rate = 16;
                                    break;
               
                                default:
                                    return CLI_ERR_INTERNAL;
                            }
                            break;
            
                        case '2':
                            packet_rate = 256;
                            break;
            
                        case '6':
                            packet_rate = 64;
                            break;
            
                        default:
                            return CLI_ERR_INTERNAL;
                    }
                }
            }

      
#else   /*exclude STRAWMAN*/

            {
    #ifdef ALLAYER_SWITCH /*temp*/
                packet_rate = 256;
    #else
                packet_rate = 500;
    #endif
            }
            else
            {
                if(arg[1]!=NULL)
                {
    #ifdef ALLAYER_SWITCH /*16*/ /*128*/ /*256*/ /*64*/
                    switch(arg[1][0])
                    {
                        case '1':
                            switch(arg[1][1])
                            {
                                case '2':
                                    packet_rate = 128;
                                    break;
                   
                                case '6':
                                    packet_rate = 16;
                                    break;
                   
                                default:
                                    return CLI_ERR_INTERNAL;
                            }
                            break;
                
                        case '2':
                            packet_rate = 256;
                            break;
                
                        case '6':
                            packet_rate = 64;
                            break;
                
                        default:
                            return CLI_ERR_INTERNAL;
                }
    #else    /*<500-262143>*/
                    packet_rate = atoi((char*)arg[1]);
    #endif
                }
            }
#endif
            


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
                        bcast_storm_entry.bcast_storm_ifindex=lport;
                        if(!SWCTRL_POM_GetBcastStormEntry(&bcast_storm_entry))
                        {
                            CLI_LIB_PrintStr_1("Failed to set broadcast storm on port %lu\r\n",(unsigned long)lport);
                            continue;                            
                        }
                        if(bcast_storm_entry.bcast_storm_status!=VAL_bcastStormStatus_enabled)
                        {
                            if (!SWCTRL_PMGR_SetBroadcastStormStatus(lport, VAL_bcastStormStatus_enabled))
                            {
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr_1("Failed to enable broadcast storm on port %lu\r\n",(unsigned long)lport);            
#endif
                                continue; 
                            }              
                        }
                        if(!SWCTRL_PMGR_SetBStormControlRateLimit(lport, SYS_DFLT_BSTORM_TYPE, packet_rate))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                           
#if (SYS_CPNT_SWCTRL_STORM_GRANULARITY_VALIDATION==TRUE)
                            UI32_T granularity;
                            
                            if(SWCTRL_POM_GetPortStormGranularity(lport, &granularity))                      
                            {
                                CLI_LIB_PrintStr_3("Failed to set broadcast storm on port %lu/%lu. Granularity is %ld.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port, (long)granularity);
                                if(bcast_storm_entry.bcast_storm_status==VAL_bcastStormStatus_enabled)
                                {
                                    continue;
                                }
                            }
                            else
                            {
                                CLI_LIB_PrintStr_2("Failed to set broadcast storm on port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
                            }
#else
                            CLI_LIB_PrintStr_2("Failed to set broadcast storm on port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif /* End of #if (SYS_CPNT_SWCTRL_STORM_GRANULARITY_VALIDATION==TRUE) */                           
                            
#endif
                            /*restore*/
                            SWCTRL_PMGR_SetBroadcastStormStatus(lport, VAL_bcastStormStatus_disabled);
                        }          
                    }
                }
            }
            break;
    
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_SWITCHPORT_BROADCAST:
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
                    else if (!SWCTRL_PMGR_SetBroadcastStormStatus(lport, VAL_bcastStormStatus_disabled))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("Failed to disable broadcast storm on port %lu\r\n",(unsigned long)lport);            
#endif
                    }
                }
            }
            break;
      
        default:
            return CLI_ERR_INTERNAL;   
    }
#endif
#endif /* (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_BSTORM) */

    return CLI_NO_ERROR; 
}

UI32_T CLI_API_Switchport_Broadcast_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRUNK_UI==TRUE)
#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_BSTORM)
#if (SYS_CPNT_BSTORM_SUPPORT_LPORT == TRUE)
#if !defined(UNICORN)/*special for UNICORN*/
    UI32_T lport = 0;
    CLI_API_TrunkStatus_T verify_ret;
    UI32_T packet_rate = 0;
    SWCTRL_BcastStormEntry_T bcast_storm_entry;
   
    memset(&bcast_storm_entry, 0x0, sizeof(SWCTRL_BcastStormEntry_T)); 

    //SWCTRL_TrunkIDToLogicalPort(ctrl_P->CMenu.pchannel_id,&lport);
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_SWITCHPORT_BROADCAST:
            if(arg[0] == NULL) /*enable by default*/
#if defined(STRAWMAN)/*special for STRAWMAN*/
            {
                packet_rate = 256;
            }
            else
            {
                if(arg[2]!=NULL)
                {
                    switch(arg[2][0])
                    {
                        case '1':
                            switch(arg[2][1])
                            {
                                case '2':
                                    packet_rate = 128;
                                    break;
                   
                                case '6':
                                    packet_rate = 16;
                                    break;
                   
                                default:
                                    return CLI_ERR_INTERNAL;
                            }
                            break;
                
                        case '2':
                            packet_rate = 256;
                            break;
                
                        case '6':
                            packet_rate = 64;
                            break;
                
                        default:
                            return CLI_ERR_INTERNAL;
                    }
                }
#else   /*exclude STRAWMAN*/

            {
#ifdef ALLAYER_SWITCH /*temp*/
                packet_rate = 256;
#else
                packet_rate = 500;
#endif
            }
            else
            {
                if(arg[1]!=NULL)
                {
#ifdef ALLAYER_SWITCH /*16*/ /*128*/ /*256*/ /*64*/
                    switch(arg[1][0])
                    {
                        case '1':
                            switch(arg[1][1])
                            {
                                case '2':
                                    packet_rate = 128;
                                     break;
                   
                                case '6':
                                    packet_rate = 16;
                                    break;
                   
                                default:
                                    return CLI_ERR_INTERNAL;
                            }
                            break;
                
                        case '2':
                            packet_rate = 256;
                            break;
                
                        case '6':
                            packet_rate = 64;
                            break;
                
                        default:
                            return CLI_ERR_INTERNAL;
                }
#else    /*<500-262143>*/
                    packet_rate = atoi((char*)arg[1]);
#endif
                }
#endif
            }


            if( (verify_ret = verify_trunk(ctrl_P->CMenu.pchannel_id, &lport)) != CLI_API_TRUNK_OK)
            {
                display_trunk_msg(verify_ret, ctrl_P->CMenu.pchannel_id);
                return CLI_NO_ERROR;
            }
            else
            {
                bcast_storm_entry.bcast_storm_ifindex=lport;
                if(!SWCTRL_POM_GetBcastStormEntry(&bcast_storm_entry))
                {
                    CLI_LIB_PrintStr_1("Failed to set broadcast storm on port %lu\r\n",(unsigned long)lport);
                    return CLI_NO_ERROR;
                }
                if(bcast_storm_entry.bcast_storm_status!=VAL_bcastStormStatus_enabled)
                {
                    if (!SWCTRL_PMGR_SetBroadcastStormStatus(lport, VAL_bcastStormStatus_enabled))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("Failed to enable broadcast storm on port %lu\r\n",(unsigned long)lport);            
#endif
                    }          
                }          
                if (!SWCTRL_PMGR_SetBStormControlRateLimit(lport, SYS_DFLT_BSTORM_TYPE, packet_rate))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    
#if (SYS_CPNT_SWCTRL_STORM_GRANULARITY_VALIDATION==TRUE) 
                    UI32_T granularity;
                        
                    if(SWCTRL_POM_GetPortStormGranularity(lport, &granularity))
                    {
                        CLI_LIB_PrintStr_2("Failed to set broadcast storm on trunk %lu. Granularity is %ld.\r\n",(unsigned long)ctrl_P->CMenu.pchannel_id, (long)granularity);
                        if(bcast_storm_entry.bcast_storm_status==VAL_bcastStormStatus_enabled)
                        {
                            break;
                        }
                    }
                    else
                    {
                        CLI_LIB_PrintStr_1("Failed to set broadcast storm on trunk %lu\r\n",(unsigned long)ctrl_P->CMenu.pchannel_id);
                    }
#else
                    CLI_LIB_PrintStr_1("Failed to set broadcast storm on trunk %lu\r\n",(unsigned long)ctrl_P->CMenu.pchannel_id);
#endif /* End of #if (SYS_CPNT_SWCTRL_STORM_GRANULARITY_VALIDATION==TRUE) */                     
                       
#endif
                    /*restore*/
                    SWCTRL_PMGR_SetBroadcastStormStatus(lport, VAL_bcastStormStatus_disabled);
                }       
            }
            break;
      
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_SWITCHPORT_BROADCAST:
            verify_ret = verify_trunk(ctrl_P->CMenu.pchannel_id, &lport);
            if(verify_ret != CLI_API_TRUNK_OK)
            {
                display_trunk_msg(verify_ret, ctrl_P->CMenu.pchannel_id);
                return CLI_NO_ERROR;
            }                        
            else if (!SWCTRL_PMGR_SetBroadcastStormStatus(lport, VAL_bcastStormStatus_disabled))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("Failed to disable broadcast storm on trunk %lu\r\n",(unsigned long)ctrl_P->CMenu.pchannel_id);            
#endif
            }
            break;
      
        default:
            return CLI_ERR_INTERNAL;	
   }
#endif
#endif /* (SYS_CPNT_BSTORM_SUPPORT_LPORT == TRUE) */
#endif /* (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_BSTORM) */
#endif  /* #if (SYS_CPNT_TRUNK_UI==TRUE) */
    return CLI_NO_ERROR;	
}

#define CLI_ATC_BROADCAST_STORM_TC_RELEASE_ENABLE 1L
#define CLI_ATC_BROADCAST_STORM_TC_RELEASE_DISABLE 2L
#define CLI_ATC_MULTICAST_STORM_TC_RELEASE_ENABLE 1L
#define CLI_ATC_MULTICAST_STORM_TC_RELEASE_DISABLE 2L

/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_ATC_Broadcast
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for configure auto traffic control broadcast mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_ATC_Broadcast_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T lport;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_AUTOTRAFFICCONTROL_BROADCAST:
        {
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
                    else if (!SWCTRL_PMGR_SetATCBroadcastStormAutoTrafficControlOnStatus(lport, VAL_atcBcastStormEnable_enabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to enable auto traffic control broadcast on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to enable auto traffic control broadcast on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        }

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_AUTOTRAFFICCONTROL_BROADCAST:
        {
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
                    else if (!SWCTRL_PMGR_SetATCBroadcastStormAutoTrafficControlOnStatus(lport, VAL_atcBcastStormEnable_disabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to disable auto traffic control broadcast on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to disable auto traffic control broadcast on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
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
#endif
    return CLI_NO_ERROR;
}

/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_ATC_Multicast
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for configure auto traffic control multicast mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_ATC_Multicast_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)

    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T lport;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_AUTOTRAFFICCONTROL_MULTICAST:
        {
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
                    else if (!SWCTRL_PMGR_SetATCMulticastStormAutoTrafficControlOnStatus(lport, VAL_atcMcastStormEnable_enabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to enable auto traffic control multicast on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to enable auto traffic control multicast on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        }

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_AUTOTRAFFICCONTROL_MULTICAST:
        {
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
                    else if (!SWCTRL_PMGR_SetATCMulticastStormAutoTrafficControlOnStatus(lport, VAL_atcMcastStormEnable_disabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to disable auto traffic control multicast on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to disable auto traffic control multicast on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
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
#endif
    return CLI_NO_ERROR;
}


/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_ATC_Broadcast_Action_Eth
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for configure auto traffic control broadcast action mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_ATC_Broadcast_Action_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T action_mode = 0;
    UI32_T lport;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_AUTOTRAFFICCONTROL_BROADCAST_ACTION:
        {
            switch(arg[0][0])
            {
                case 'r':
                    action_mode = VAL_atcBcastStormTcAction_rate_control;
                    break;
                case 's':
                    action_mode = VAL_atcBcastStormTcAction_shutdown;
                    break;
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
                    else if (!SWCTRL_PMGR_SetATCBroadcastStormAction(lport, action_mode))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to enable auto traffic control broadcast action on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set enable auto traffic control broadcast action on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        }

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_AUTOTRAFFICCONTROL_BROADCAST_ACTION:
        {
            action_mode = VAL_atcBcastStormTcAction_rate_control;

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
                    else if (!SWCTRL_PMGR_SetATCBroadcastStormAction(lport, action_mode))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to disable auto traffic control broadcast action on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to disable auto traffic control broadcast action on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
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
#endif
    return CLI_NO_ERROR;
}

/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_ATC_Multicast_Action_Eth
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for configure auto traffic control multicast action mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_ATC_Multicast_Action_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T action_mode = 0;
    UI32_T lport;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_AUTOTRAFFICCONTROL_MULTICAST_ACTION:
        {
            switch(arg[0][0])
            {
                case 'r':
                    action_mode = VAL_atcMcastStormTcAction_rate_control;
                    break;
                case 's':
                    action_mode = VAL_atcMcastStormTcAction_shutdown;
                    break;
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
                    else if (!SWCTRL_PMGR_SetATCMulticastStormAction(lport, action_mode))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to enable auto traffic control multicast action on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to enable auto traffic control multicast action on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        }

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_AUTOTRAFFICCONTROL_MULTICAST_ACTION:
        {
            action_mode = VAL_atcMcastStormTcAction_rate_control;

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
                    else if (!SWCTRL_PMGR_SetATCMulticastStormAction(lport, action_mode))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to disable auto traffic control multicast action on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set disable auto traffic control multicast on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
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
#endif
    return CLI_NO_ERROR;
}


/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_ATC_Broadcast_AutoControlRelease_Eth
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for configure ATC broadcast control releaase
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_ATC_Broadcast_AutoControlRelease_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T lport;

    switch(cmd_idx)
    {             
        /* broadcast strom part */
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_AUTOTRAFFICCONTROL_BROADCAST_AUTOCONTROLRELEASE:
        {
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
                    else if (!SWCTRL_PMGR_SetATCBroadcastStormAutoTrafficControlReleaseStatus(lport, VAL_atcBcastStormAutoRelease_enabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to enable control release on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to enable control release on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        }

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_AUTOTRAFFICCONTROL_BROADCAST_AUTOCONTROLRELEASE:
        {
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
                    else if (!SWCTRL_PMGR_SetATCBroadcastStormAutoTrafficControlReleaseStatus(lport, VAL_atcBcastStormAutoRelease_disabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to disable control release on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to disable control release on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        } /* the end of broadcast part */

        default:
            return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}


/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_ATC_Multicast_AutoControlRelease_Eth
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for configure ATC multicast control releaase
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_ATC_Multicast_AutoControlRelease_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T lport;

    switch(cmd_idx)
    {
        /* multicast strom part */
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_AUTOTRAFFICCONTROL_MULTICAST_AUTOCONTROLRELEASE:
        {
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
                    else if (!SWCTRL_PMGR_SetATCMulticastStormAutoTrafficControlReleaseStatus(lport, VAL_atcMcastStormAutoRelease_enabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to enable control release on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to enable control release on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        }

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_AUTOTRAFFICCONTROL_MULTICAST_AUTOCONTROLRELEASE:
        {
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
                    else if (!SWCTRL_PMGR_SetATCMulticastStormAutoTrafficControlReleaseStatus(lport, VAL_atcMcastStormAutoRelease_disabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to disable control release on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to disable control release on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        } /* the end of multicast part */

        default:
            return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}



UI32_T CLI_API_ATC_Broadcast_ControlRelease_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T lport;

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
            else if (!SWCTRL_PMGR_SetATCBroadcastStormTrafficControlReleaseStatus(lport, CLI_ATC_BROADCAST_STORM_TC_RELEASE_ENABLE))
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
                {
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to enable control release on ethernet port %s.\r\n", name);
#endif
                }
#else
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to enable control release on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
            }
        }
    }
#endif

    return CLI_NO_ERROR;
}

UI32_T CLI_API_ATC_Multicast_ControlRelease_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T lport;
		
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
            else if (!SWCTRL_PMGR_SetATCMulticastStormTrafficControlReleaseStatus(lport, CLI_ATC_MULTICAST_STORM_TC_RELEASE_ENABLE))
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
                {
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to enable control release on ethernet port %s.\r\n", name);
#endif
                }
#else
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to enable control release on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
            }
        }
    }

#endif
    return CLI_NO_ERROR;
  
}


/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_ATC_Broadcast_AlarmFireThreshold_Eth
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for configure the theshold of ATC broadcast alarm fire
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_ATC_Broadcast_AlarmFireThreshold_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T lport;

    switch(cmd_idx)
    {
        /* broadcast strom part */
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_AUTOTRAFFICCONTROL_BROADCAST_ALARMFIRETHRESHOLD:
        {
            UI32_T threshold_value = 0;
            threshold_value = atoi((char*)arg[0]);

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
                    else if (!SWCTRL_PMGR_SetATCBroadcastStormStormAlarmThreshold(lport, threshold_value))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to set alarm fire threshold on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set alarm fire threshold on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        }

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_AUTOTRAFFICCONTROL_BROADCAST_ALARMFIRETHRESHOLD:
        {
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
                    else if (!SWCTRL_PMGR_SetATCBroadcastStormStormAlarmThreshold(lport, (SYS_DFLT_ATC_BSTORM_STORM_ALARM_THRESHOLD/SYS_ADPT_ATC_STORM_CONTROL_UNIT)))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to set alarm fire threshold on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set alarm fire threshold on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        } /* the end of broadcast part */

        default:
            return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}

/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_ATC_Multicast_AlarmFireThreshold_Eth
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for configure the theshold of ATC multicast alarm fire
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_ATC_Multicast_AlarmFireThreshold_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T lport;

    switch(cmd_idx)
    {
        /* broadcast strom part */
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_AUTOTRAFFICCONTROL_MULTICAST_ALARMFIRETHRESHOLD:
        {
            UI32_T threshold_value = 0;
            threshold_value = atoi((char*)arg[0]);

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
                    else if (!SWCTRL_PMGR_SetATCMulticastStormStormAlarmThreshold(lport, threshold_value))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to set alarm fire threshold on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set alarm fire threshold on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        }

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_AUTOTRAFFICCONTROL_MULTICAST_ALARMFIRETHRESHOLD:
        {
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
                    else if (!SWCTRL_PMGR_SetATCMulticastStormStormAlarmThreshold(lport, (SYS_DFLT_ATC_MSTORM_STORM_ALARM_THRESHOLD/SYS_ADPT_ATC_STORM_CONTROL_UNIT)))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to set alarm fire threshold on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set alarm fire threshold on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        } /* the end of broadcast part */

        default:
            return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}


/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_ATC_Broadcast_AlarmClearThreshold_Eth
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for configure the theshold of ATC broadcast alarm clear
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_ATC_Broadcast_AlarmClearThreshold_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T lport;

    switch(cmd_idx)
    {
        /* broadcast strom part */
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_AUTOTRAFFICCONTROL_BROADCAST_ALARMCLEARTHRESHOLD:
        {
            UI32_T threshold_value = 0;
            threshold_value = atoi((char*)arg[0]);

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
                    else if (!SWCTRL_PMGR_SetATCBroadcastStormStormClearThreshold(lport, threshold_value))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to set alarm clear threshold on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set alarm clear threshold on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
										else/*when ATC on. disable broadcast storm after set clear thresholed*/
										{
										   SWCTRL_PMGR_DisableBStormAfterClearThreshold(lport);
										}
                }
            }
            break;
        }

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_AUTOTRAFFICCONTROL_BROADCAST_ALARMCLEARTHRESHOLD:
        {
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
                    else if (!SWCTRL_PMGR_SetATCBroadcastStormStormClearThreshold(lport, (SYS_DFLT_ATC_BSTORM_STORM_CLEAR_THRESHOLD/SYS_ADPT_ATC_STORM_CONTROL_UNIT)))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to set alarm clear threshold on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set alarm clear threshold on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
										else/*when ATC on. disable broadcast storm after set clear thresholed*/
										{
										   SWCTRL_PMGR_DisableBStormAfterClearThreshold(lport);
										}
										
                }
            }
            break;
        } /* the end of broadcast part */

        default:
            return CLI_ERR_INTERNAL;
    }
#endif

    return CLI_NO_ERROR;
}

/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_ATC_Multicast_AlarmClearThreshold_Eth
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for configure the theshold of ATC multicast alarm clear
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_ATC_Multicast_AlarmClearThreshold_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T lport;

    switch(cmd_idx)
    {
        /* broadcast strom part */
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_AUTOTRAFFICCONTROL_MULTICAST_ALARMCLEARTHRESHOLD:
        {
            UI32_T threshold_value = 0;
            threshold_value = atoi((char*)arg[0]);

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
                    else if (!SWCTRL_PMGR_SetATCMulticastStormStormClearThreshold(lport, threshold_value))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to set alarm clear threshold on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set alarm clear threshold on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
										else/*when ATC on. disable multicast storm after set clear thresholed*/
										{
										   SWCTRL_PMGR_DisableMStormAfterClearThreshold(lport);
										}
                }
            }
            break;
        }

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_AUTOTRAFFICCONTROL_MULTICAST_ALARMCLEARTHRESHOLD:
        {
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
                    else if (!SWCTRL_PMGR_SetATCMulticastStormStormClearThreshold(lport, (SYS_DFLT_ATC_MSTORM_STORM_CLEAR_THRESHOLD/SYS_ADPT_ATC_STORM_CONTROL_UNIT)))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to set alarm clear threshold on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set alarm clear threshold on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
										else/*when ATC on. disable multicast storm after set clear thresholed*/
										{
										   SWCTRL_PMGR_DisableMStormAfterClearThreshold(lport);
										}
										
                }
            }
            break;
        } /* the end of broadcast part */

        default:
            return CLI_ERR_INTERNAL;
    }
#endif

    return CLI_NO_ERROR;
}


/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_ATC_Bcast_ApplyTimer
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for configures broadcast traffic control apply timer
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_ATC_Bcast_ApplyTimer(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T apply_time = 0;

    switch(cmd_idx)
    {
        /* broadcast strom part */
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_AUTOTRAFFICCONTROL_BROADCAST_APPLYTIMER:
        {
            apply_time = atoi((char*)arg[0]);
            break;
        }
        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_AUTOTRAFFICCONTROL_BROADCAST_APPLYTIMER:
        {
            apply_time = SYS_DFLT_ATC_BSTORM_TC_ON_TIMER;
            break;
        }
        default:
            return CLI_ERR_INTERNAL;
    }
    if (!SWCTRL_PMGR_SetATCBroadcastStormTrafficControlOnTimer(apply_time))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set ATC broadcast apply-timer.\r\n");
#endif
    }
#endif

    return CLI_NO_ERROR;
}


/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_ATC_Mcast_ApplyTimer
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for configures multicast traffic control apply timer
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_ATC_Mcast_ApplyTimer(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T apply_time = 0;

    switch(cmd_idx)
    {
        /* broadcast strom part */
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_AUTOTRAFFICCONTROL_MULTICAST_APPLYTIMER:
        {
            apply_time = atoi((char*)arg[0]);
            break;
        }
        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_AUTOTRAFFICCONTROL_MULTICAST_APPLYTIMER:
        {
            apply_time = SYS_DFLT_ATC_MSTORM_TC_ON_TIMER;
            break;
        }
        default:
            return CLI_ERR_INTERNAL;
    }

    if (!SWCTRL_PMGR_SetATCMulticastStormTrafficControlOnTimer(apply_time))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set ATC multicast apply-timer.\r\n");
#endif
    }
#endif

    return CLI_NO_ERROR;
}


/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_ATC_Bcast_ReleaseTimer
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for configures broadcast traffic control release timer
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_ATC_Bcast_ReleaseTimer(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T release_time = 0;

    switch(cmd_idx)
    {
        /* broadcast strom part */
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_AUTOTRAFFICCONTROL_BROADCAST_RELEASETIMER:
        {
            release_time = atoi((char*)arg[0]);
            break;
        }
        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_AUTOTRAFFICCONTROL_BROADCAST_RELEASETIMER:
        {
            release_time = SYS_DFLT_ATC_BSTORM_TC_RELEASE_TIMER;
            break;
        }
        default:
            return CLI_ERR_INTERNAL;
    }

    if (!SWCTRL_PMGR_SetATCBroadcastStormTrafficControlReleaseTimer(release_time))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set ATC broadcast release-timer.\r\n");
#endif
    }
#endif

    return CLI_NO_ERROR;
}


/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_ATC_Mcast_ReleaseTimer
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for configures multicast traffic control release timer
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_ATC_Mcast_ReleaseTimer(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T release_time = 0;

    switch(cmd_idx)
    {
        /* broadcast strom part */
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_AUTOTRAFFICCONTROL_MULTICAST_RELEASETIMER:
        {
            release_time = atoi((char*)arg[0]);
            break;
        }
        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_AUTOTRAFFICCONTROL_MULTICAST_RELEASETIMER:
        {
            release_time = SYS_DFLT_ATC_MSTORM_TC_RELEASE_TIMER;
            break;
        }
        default:
            return CLI_ERR_INTERNAL;
    }

    if (!SWCTRL_PMGR_SetATCMulticastStormTrafficControlReleaseTimer(release_time))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set ATC multicast release-timer.\r\n");
#endif
    }
#endif

    return CLI_NO_ERROR;
}


/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_ATC_Bcast_ControlRelease
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for the broadcast port to leave the state of shutdown.
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_ATC_Bcast_ControlRelease(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ATC_STORM  == TRUE)
    char   delemiters[2] = {0};
    char   Token[CLI_DEF_MAX_BUFSIZE] = {0};
    char   *s;
    UI8_T  unit;
    UI32_T lower_val = 0;
    UI32_T upper_val = 0;
    UI32_T err_idx;
    UI8_T  i;
    UI32_T lport = 0;
    CLI_API_EthStatus_T verify_ret;

    delemiters[0] = ',';

    if (isdigit(arg[0][0]))
    {
        s = arg[0];

        /*get the unit*/
        unit = atoi(s);/*pttch stacking*/

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
                    if( (verify_ret = verify_ethernet(unit, i, &lport)) != CLI_API_ETH_OK)
                    {
                        display_ethernet_msg(verify_ret, unit, i);
                        continue;
                    }
                    else if(!SWCTRL_PMGR_SetATCBroadcastStormTrafficControlReleaseStatus(lport, CLI_ATC_BROADCAST_STORM_TC_RELEASE_ENABLE))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set control release on ethernet port %d/%d\r\n", unit, i);
#endif
                    }
                }
            }

            if(s == 0)
                break;
            else
                memset(Token, 0, sizeof(Token));
        }
    }
#if (CLI_SUPPORT_PORT_NAME == 1)
    else
    {
        UI32_T ifindex = 0;
        UI32_T unit,port,trunk_id;

        if (!IF_MGR_IfnameToIfindex(arg[0], &ifindex))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[0]);
#endif
            return CLI_NO_ERROR;
        }
        //SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);
        if (!SWCTRL_PMGR_SetATCBroadcastStormTrafficControlReleaseStatus(ifindex, CLI_ATC_BROADCAST_STORM_TC_RELEASE_ENABLE))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1("Failed to set control release on ethernet port %s\r\n", arg[0]);
#endif
            return CLI_NO_ERROR;
        }
    }
#endif
#endif
    return CLI_NO_ERROR;
}


/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_ATC_Mcast_ControlRelease
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for the multicast port to leave the state of shutdown.
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_ATC_Mcast_ControlRelease(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ATC_STORM  == TRUE)
    char   delemiters[2] = {0};
    char   Token[CLI_DEF_MAX_BUFSIZE] = {0};
    char   *s;
    UI8_T  unit;
    UI32_T lower_val = 0;
    UI32_T upper_val = 0;
    UI32_T err_idx;
    UI8_T  i;
    UI32_T lport = 0;
    CLI_API_EthStatus_T verify_ret;

    delemiters[0] = ',';

    if (isdigit(arg[0][0]))
    {
        s = arg[0];

        /*get the unit*/
        unit = atoi(s);/*pttch stacking*/

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
                    if( (verify_ret = verify_ethernet(unit, i, &lport)) != CLI_API_ETH_OK)
                    {
                        display_ethernet_msg(verify_ret, unit, i);
                        continue;
                    }
                    else if(!SWCTRL_PMGR_SetATCMulticastStormTrafficControlReleaseStatus(lport, CLI_ATC_MULTICAST_STORM_TC_RELEASE_ENABLE))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set control release on ethernet port %d/%d\r\n", unit, i);
#endif
                    }
                }
            }

            if(s == 0)
                break;
            else
                memset(Token, 0, sizeof(Token));
        }
    }
#if (CLI_SUPPORT_PORT_NAME == 1)
    else
    {
        UI32_T ifindex = 0;
        UI32_T unit,port,trunk_id;

        if (!IF_MGR_IfnameToIfindex(arg[0], &ifindex))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[0]);
#endif
            return CLI_NO_ERROR;
        }
        //SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);
        if (!SWCTRL_PMGR_SetATCMulticastStormTrafficControlReleaseStatus(ifindex, CLI_ATC_MULTICAST_STORM_TC_RELEASE_ENABLE))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1("Failed to set control release on ethernet port %s\r\n", arg[0]);
#endif
            return CLI_NO_ERROR;
        }
    }
#endif
#endif
    return CLI_NO_ERROR;
}


/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_SnmpSvr_Enable_PortTraps_Bcast_ATC_StormAlarmFire_Eth
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for the traps control about broadcast alarm fire
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_SnmpSvr_Enable_PortTraps_Bcast_ATC_StormAlarmFire_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

#if(SYS_CPNT_ATC_STORM == TRUE)

    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T lport;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W5_SNMPSERVER_ENABLE_PORTTRAPS_ATC_BROADCASTALARMFIRE:
        {
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
                    else if (!SWCTRL_PMGR_SetATCBroadcastStormTrapStormAlarmStatus(lport, VAL_atcBcastStormAlarmFireTrapStatus_enabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to enable alarm fire trap on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to enable alarm fire trap on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        }

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W6_NO_SNMPSERVER_ENABLE_PORTTRAPS_ATC_BROADCASTALARMFIRE:
        {
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
                    else if (!SWCTRL_PMGR_SetATCBroadcastStormTrapStormAlarmStatus(lport, VAL_atcBcastStormAlarmFireTrapStatus_disabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to disable alarm fire trap on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set disable fire trap on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
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
#endif
    return CLI_NO_ERROR;
}


/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_SnmpSvr_Enable_PortTraps_Mcast_ATC_StormAlarmFire_Eth
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for the traps control about broadcast alarm fire
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_SnmpSvr_Enable_PortTraps_Mcast_ATC_StormAlarmFire_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T lport;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W5_SNMPSERVER_ENABLE_PORTTRAPS_ATC_MULTICASTALARMFIRE:
        {
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
                    else if (!SWCTRL_PMGR_SetATCMulticastStormTrapStormAlarmStatus(lport, VAL_atcMcastStormAlarmFireTrapStatus_enabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to enable alarm fire trap on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to enable alarm fire trap on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        }

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W6_NO_SNMPSERVER_ENABLE_PORTTRAPS_ATC_MULTICASTALARMFIRE:
        {
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
                    else if (!SWCTRL_PMGR_SetATCMulticastStormTrapStormAlarmStatus(lport, VAL_atcMcastStormAlarmFireTrapStatus_disabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to disable alarm fire trap on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set disable fire trap on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
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
#endif
    return CLI_NO_ERROR;
}


/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_SnmpSvr_Enable_PortTraps_Bcast_ATC_StormAlarmClear_Eth
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for the traps control about broadcast alarm clear
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_SnmpSvr_Enable_PortTraps_Bcast_ATC_StormAlarmClear_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T lport;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W5_SNMPSERVER_ENABLE_PORTTRAPS_ATC_BROADCASTALARMCLEAR:
        {
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
                    else if (!SWCTRL_PMGR_SetATCBroadcastStormTrapStormClearStatus(lport, VAL_atcBcastStormAlarmClearTrapStatus_enabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to enable alarm clear trap on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to enable alarm clear trap on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        }

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W6_NO_SNMPSERVER_ENABLE_PORTTRAPS_ATC_BROADCASTALARMCLEAR:
        {
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
                    else if (!SWCTRL_PMGR_SetATCBroadcastStormTrapStormClearStatus(lport, VAL_atcBcastStormAlarmClearTrapStatus_disabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to disable alarm fire trap on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set disable fire trap on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
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
#endif
    return CLI_NO_ERROR;
}


/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_SnmpSvr_Enable_PortTraps_Mcast_ATC_StormAlarmClear_Eth
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for the traps control about broadcast alarm clear
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_SnmpSvr_Enable_PortTraps_Mcast_ATC_StormAlarmClear_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T lport;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W5_SNMPSERVER_ENABLE_PORTTRAPS_ATC_MULTICASTALARMCLEAR:
        {
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
                    else if (!SWCTRL_PMGR_SetATCMulticastStormTrapStormClearStatus(lport, VAL_atcMcastStormAlarmClearTrapStatus_enabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to enable alarm clear trap on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to enable alarm fire clear on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        }

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W6_NO_SNMPSERVER_ENABLE_PORTTRAPS_ATC_MULTICASTALARMCLEAR:
        {
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
                    else if (!SWCTRL_PMGR_SetATCMulticastStormTrapStormClearStatus(lport, VAL_atcMcastStormAlarmClearTrapStatus_disabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to disable alarm clear trap on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to disable alarm fire clear on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
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
#endif
    return CLI_NO_ERROR;
}


/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_SnmpSvr_Enable_PortTraps_Bcast_ATC_TrafficControlApply_Eth
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for the traps control about broadcast traffic control apply
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_SnmpSvr_Enable_PortTraps_Bcast_ATC_TrafficControlApply_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T lport;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W5_SNMPSERVER_ENABLE_PORTTRAPS_ATC_BROADCASTCONTROLAPPLY:
        {
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
                    else if (!SWCTRL_PMGR_SetATCBroadcastStormTrapTrafficControlOnStatus(lport, VAL_atcBcastStormTcApplyTrapStatus_enabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to enable traffic control apply trap on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to enable traffic control apply trap on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        }

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W6_NO_SNMPSERVER_ENABLE_PORTTRAPS_ATC_BROADCASTCONTROLAPPLY:
        {
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
                    else if (!SWCTRL_PMGR_SetATCBroadcastStormTrapTrafficControlOnStatus(lport, VAL_atcBcastStormTcApplyTrapStatus_disabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to disable traffic control apply trap on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to disable traffic control apply trap on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
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
#endif
    return CLI_NO_ERROR;
}


/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_SnmpSvr_Enable_PortTraps_Mcast_ATC_TrafficControlApply_Eth
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for the traps control about multicast traffic control apply
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_SnmpSvr_Enable_PortTraps_Mcast_ATC_TrafficControlApply_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T lport;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W5_SNMPSERVER_ENABLE_PORTTRAPS_ATC_MULTICASTCONTROLAPPLY:
        {
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
                    else if (!SWCTRL_PMGR_SetATCMulticastStormTrapTrafficControlOnStatus(lport, VAL_atcMcastStormTcApplyTrapStatus_enabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to enable traffic control apply trap on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to enable traffic control apply trap on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        }

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W6_NO_SNMPSERVER_ENABLE_PORTTRAPS_ATC_MULTICASTCONTROLAPPLY:
        {
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
                    else if (!SWCTRL_PMGR_SetATCMulticastStormTrapTrafficControlOnStatus(lport, VAL_atcMcastStormTcApplyTrapStatus_disabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to disable traffic control apply trap on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to disable traffic control apply trap on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
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
#endif
    return CLI_NO_ERROR;
}


/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_SnmpSvr_Enable_PortTraps_Bcast_ATC_TrafficControlRelease_Eth
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for the traps control about broadcast traffic control release
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_SnmpSvr_Enable_PortTraps_Bcast_ATC_TrafficControlRelease_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T lport;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W5_SNMPSERVER_ENABLE_PORTTRAPS_ATC_BROADCASTCONTROLRELEASE:
        {
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
                    else if (!SWCTRL_PMGR_SetATCBroadcastStormTrapTrafficControlReleaseStatus(lport, VAL_atcBcastStormTcReleaseTrapStatus_enabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to enable traffic control release trap on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to enable traffic control apply release on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        }

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W6_NO_SNMPSERVER_ENABLE_PORTTRAPS_ATC_BROADCASTCONTROLRELEASE:
        {
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
                    else if (!SWCTRL_PMGR_SetATCBroadcastStormTrapTrafficControlReleaseStatus(lport, VAL_atcBcastStormTcReleaseTrapStatus_disabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to disable traffic control release trap on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to disable traffic control release trap on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
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
#endif
    return CLI_NO_ERROR;
}


/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_SnmpSvr_Enable_PortTraps_Mcast_ATC_TrafficControlRelease_Eth
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for the traps control about multicast traffic control apply
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_SnmpSvr_Enable_PortTraps_Mcast_ATC_TrafficControlRelease_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T lport;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W5_SNMPSERVER_ENABLE_PORTTRAPS_ATC_MULTICASTCONTROLRELEASE:
        {
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
                    else if (!SWCTRL_PMGR_SetATCMulticastStormTrapTrafficControlReleaseStatus(lport, VAL_atcMcastStormTcReleaseTrapStatus_enabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to set traffic control release trap on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set traffic control release trap on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                    }
                }
            }
            break;
        }

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W6_NO_SNMPSERVER_ENABLE_PORTTRAPS_ATC_MULTICASTCONTROLRELEASE:
        {
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
                    else if (!SWCTRL_PMGR_SetATCMulticastStormTrapTrafficControlReleaseStatus(lport, VAL_atcMcastStormTcReleaseTrapStatus_disabled))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to set traffic control release trap on ethernet port %s.\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set traffic control apply release on ethernet port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
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
#endif
    return CLI_NO_ERROR;
}

#if(SYS_CPNT_ATC_STORM == TRUE)

UI32_T show_one_port(UI32_T unit, UI32_T port,SWCTRL_ATCBroadcastStormEntry_T bcast_entry, SWCTRL_ATCMulticastStormEntry_T mcast_entry,UI32_T line_num)
{
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char    *status[] = {"Disabled", "Enabled"};
    char    *threshold_unit_str = "(kpps)";

#if SYS_ADPT_ATC_STORM_CONTROL_UNIT == 1
    threshold_unit_str = "(pps)";
#elif SYS_ADPT_ATC_STORM_CONTROL_UNIT == 500
    threshold_unit_str = "(500 pps)";
#endif

    sprintf(buff, "Eth %lu/%lu Information\r\n", (unsigned long)unit, (unsigned long)port);
    PROCESS_MORE_FUNC(buff);
    PROCESS_MORE_FUNC("------------------------------------------------------------------------\r\n");
    PROCESS_MORE_FUNC("Storm Control                   : Broadcast              Multicast\r\n");

    /* state */
    sprintf(buff, "State                           : %-22s %-22s\r\n",
        bcast_entry.atc_broadcast_storm_status == VAL_bcastStormStatus_enabled ? status[1] : status[0],
        mcast_entry.atc_multicast_storm_status == VAL_mcastStormStatus_enabled ? status[1] : status[0]);
    PROCESS_MORE_FUNC(buff);

    /* action */
    sprintf(buff, "Action                          : %-22s %-22s\r\n",
        bcast_entry.atc_broadcast_storm_action == VAL_atcBcastStormTcAction_rate_control ? "Rate Control" : "Shutdown",
        mcast_entry.atc_multicast_storm_action == VAL_atcMcastStormTcAction_rate_control ? "Rate Control" : "Shutdown");
    PROCESS_MORE_FUNC(buff);

    /* release control */
    sprintf(buff, "Auto Release Control            : %-22s %-22s\r\n",
        bcast_entry.atc_broadcast_storm_auto_traffic_control_release == VAL_atcBcastStormAutoRelease_enabled ? status[1] : status[0],
        mcast_entry.atc_multicast_storm_auto_traffic_control_release == VAL_atcMcastStormAutoRelease_enabled ? status[1] : status[0]);
    PROCESS_MORE_FUNC(buff);

    /* alarm fire threshold */
    sprintf(buff, "Alarm Fire Threshold %-9s  : %-22lu %-22lu\r\n",
        threshold_unit_str,
        bcast_entry.atc_broadcast_storm_storm_alarm_threshold,
        mcast_entry.atc_multicast_storm_storm_alarm_threshold);
    PROCESS_MORE_FUNC(buff);

    /* alarm clear threshold */
    sprintf(buff, "Alarm Clear Threshold %-9s : %-22lu %-22lu\r\n",
        threshold_unit_str,
        bcast_entry.atc_broadcast_storm_storm_clear_threshold,
        mcast_entry.atc_multicast_storm_storm_clear_threshold);
    PROCESS_MORE_FUNC(buff);

    /* Trap Storm Fire */
    sprintf(buff, "Trap Storm Fire                 : %-22s %-22s\r\n",
        bcast_entry.atc_broadcast_storm_trap_storm_alarm == VAL_atcBcastStormAlarmFireTrapStatus_enabled ? status[1] : status[0],
        mcast_entry.atc_multicast_storm_trap_storm_alarm == VAL_atcMcastStormAlarmFireTrapStatus_enabled ? status[1] : status[0]);
    PROCESS_MORE_FUNC(buff);

    /* Trap Storm CLear */
    sprintf(buff, "Trap Storm Clear                : %-22s %-22s\r\n",
        bcast_entry.atc_broadcast_storm_trap_storm_clear == VAL_atcBcastStormAlarmClearTrapStatus_enabled ? status[1] : status[0],
        mcast_entry.atc_multicast_storm_trap_storm_clear == VAL_atcMcastStormAlarmClearTrapStatus_enabled ? status[1] : status[0]);
    PROCESS_MORE_FUNC(buff);

    /* Trap Traffic Apply */
    sprintf(buff, "Trap Traffic Apply              : %-22s %-22s\r\n",
        bcast_entry.atc_broadcast_storm_trap_traffic_control_on == VAL_atcBcastStormTcApplyTrapStatus_enabled ? status[1] : status[0],
        mcast_entry.atc_multicast_storm_trap_traffic_control_on == VAL_atcMcastStormTcApplyTrapStatus_enabled ? status[1] : status[0]);
    PROCESS_MORE_FUNC(buff);

    /* Trap Traffic Release */
    sprintf(buff, "Trap Traffic Release            : %-22s %-22s\r\n",
        bcast_entry.atc_broadcast_storm_trap_traffic_control_release == VAL_atcBcastStormTcReleaseTrapStatus_enabled ? status[1] : status[0],
        mcast_entry.atc_multicast_storm_trap_traffic_control_release == VAL_atcMcastStormTcReleaseTrapStatus_enabled ? status[1] : status[0]);
    PROCESS_MORE_FUNC(buff);

    PROCESS_MORE_FUNC("------------------------------------------------------------------------\r\n");
    PROCESS_MORE_FUNC("\r\n");

    return line_num;
}
#endif

/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_AutoTrafficControl
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for show global information about ATC
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_Show_AutoTrafficControl(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    SWCTRL_ATCBroadcastStormTimer_T     bcast_timer;
    SWCTRL_ATCMulticastStormTimer_T     mcast_timer;

    memset(&bcast_timer, 0, sizeof(bcast_timer));
    memset(&mcast_timer, 0, sizeof(mcast_timer));
    
    if (!SWCTRL_PMGR_GetATCBroadcastStormTimer(&bcast_timer))
    {
        CLI_LIB_PrintStr("Failed to get broadcast storm control timer information\r\n");
    }
    else
    {
        CLI_LIB_PrintStr("\nStorm Control Broadcast\r\n");
        CLI_LIB_PrintStr_1(" Apply Timer (sec)   : %lu\r\n", (unsigned long)bcast_timer.atc_broadcast_storm_traffic_control_on_timer);
        CLI_LIB_PrintStr_1(" Release Timer (sec) : %lu\r\n", (unsigned long)bcast_timer.atc_broadcast_storm_traffic_control_release_timer);
    }

    if (!SWCTRL_PMGR_GetATCMulticastStormTimer(&mcast_timer))
    {
        CLI_LIB_PrintStr("Failed to get multicast storm control timer information\r\n");
    }
    else
    {
        CLI_LIB_PrintStr("\nStorm Control Multicast\r\n");
        CLI_LIB_PrintStr_1(" Apply Timer (sec)   : %lu\r\n", (unsigned long)mcast_timer.atc_multicast_storm_traffic_control_on_timer);
        CLI_LIB_PrintStr_1(" Release Timer (sec) : %lu\r\n", (unsigned long)mcast_timer.atc_multicast_storm_traffic_control_release_timer);
    }
#endif
    return CLI_NO_ERROR;
}


/*----------------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_AutoTrafficControl_Interface
 *----------------------------------------------------------------------------------------
 * PURPOSE  : This is action for show interface information about ATC
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *-----------------------------------------------------------------------------------------*/
UI32_T CLI_API_Show_AutoTrafficControl_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_ATC_STORM == TRUE)
    UI32_T lport    = 0;
    UI32_T line_num = 0;
    /*UI32_T ret = 0;
    UI32_T trunk_id = 0;*/
    SWCTRL_ATCBroadcastStormEntry_T bcast_entry;
    SWCTRL_ATCMulticastStormEntry_T mcast_entry;
    UI32_T max_port_num;
    UI32_T verify_unit = ctrl_P->sys_info.my_unit_id;
    UI32_T verify_port;
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    
    memset(&bcast_entry, 0, sizeof(bcast_entry));
    memset(&mcast_entry, 0, sizeof(mcast_entry));

    if (arg[0] == NULL)
    {
	  for (i=0; STKTPLG_POM_GetNextUnit(&i); )
	  {
		  verify_unit = i;
		  max_port_num = SWCTRL_POM_UIGetUnitPortNumber(verify_unit);
		  for(verify_port = 1; verify_port <= max_port_num; verify_port++)
		  {
			verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
			if(verify_ret==CLI_API_ETH_TRUNK_MEMBER)
			{
			    continue;
			}
			bcast_entry.atc_broadcast_storm_ifindex = lport;
			mcast_entry.atc_multicast_storm_ifindex = lport;						
			if (!SWCTRL_PMGR_GetATCBroadcastStormEntry(&bcast_entry))
			{
			   CLI_LIB_PrintStr_2("Failed to get broadcast storm entry information on port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)lport);
			   continue;
			}

			if (!SWCTRL_PMGR_GetATCMulticastStormEntry(&mcast_entry))
			{
				CLI_LIB_PrintStr_2("Failed to get multicast storm entry information on port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)lport);
				continue;
			}

			if((line_num = show_one_port(verify_unit, verify_port, bcast_entry, mcast_entry, line_num)) == JUMP_OUT_MORE)
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
    else if (arg[0][0] == 'e' || arg[0][0] == 'E')
    {
        UI32_T lport = 0;
        UI32_T verify_unit = atoi((char*)arg[1]);
        UI32_T verify_port = atoi(strchr((char*)arg[1], '/')+1);
        CLI_API_EthStatus_T verify_ret;
#if (CLI_SUPPORT_PORT_NAME == 1)
        if (isdigit(arg[1][0]))
        {
            verify_unit = atoi(arg[1]);
            verify_port = atoi(strchr(arg[1],'/')+1);
        }
        else/*port name*/
        {
            UI32_T trunk_id = 0;
            if (!IF_MGR_IfnameToIfindex(arg[1], &lport))
            {
                CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[1]);
                return CLI_NO_ERROR;
            }
            SWCTRL_PMGR_LogicalPortToUserPort(lport, &verify_unit, &verify_port, &trunk_id);
        }
#endif

        verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
        if(verify_ret==CLI_API_ETH_TRUNK_MEMBER)
	{
	    display_ethernet_msg(verify_ret, verify_unit, verify_port);
	    return CLI_NO_ERROR;
	}
        
#if 0
        if (SWCTRL_PMGR_LogicalPortToUserPort(lport, &verify_unit, &verify_port, &trunk_id) != SWCTRL_LPORT_NORMAL_PORT)
        {
            CLI_LIB_PrintStr_2("Failed to get auto-traffic-control information on port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
            return CLI_NO_ERROR;
        }
           

       if (SWCTRL_PMGR_IsManagementPort(lport) == TRUE)
        {
            CLI_LIB_PrintStr_2("Failed to get auto-traffic-control information on port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
            return CLI_NO_ERROR;
        }
#endif 
        bcast_entry.atc_broadcast_storm_ifindex = lport;
        mcast_entry.atc_multicast_storm_ifindex = lport;
        
				if (!SWCTRL_PMGR_GetATCBroadcastStormEntry(&bcast_entry))
        {
            CLI_LIB_PrintStr_2("Failed to get broadcast storm entry information on port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
        }
        if (!SWCTRL_PMGR_GetATCMulticastStormEntry(&mcast_entry))
        {
            CLI_LIB_PrintStr_2("Failed to get multicast storm entry information on port %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
        }
        if((line_num = show_one_port(verify_unit, verify_port,bcast_entry, mcast_entry, line_num)) == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }
        else if (line_num == EXIT_SESSION_MORE)
        {
            return CLI_EXIT_SESSION;
        }
    }

#endif

    return CLI_NO_ERROR;
}

UI32_T CLI_API_StormSampleType(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SWCTRL_GLOBAL_STORM_SAMPLE_TYPE == TRUE)
    UI32_T global_storm_sample_type;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W1_STORMSAMPLETYPE:
            if (arg[0] != NULL)
            {
                if (arg[0][0] == 'o') /* octet */
                    global_storm_sample_type = VAL_stormSampleType_octet_rate;
                else if (arg[0][1] == 'e') /* percent */
                    global_storm_sample_type = VAL_stormSampleType_percent;
                else /* packet */
                    global_storm_sample_type = VAL_stormSampleType_pkt_rate;
            }
            else
            {
                return CLI_ERR_INTERNAL;
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_STORMSAMPLETYPE:
            global_storm_sample_type = 0;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (!SWCTRL_PMGR_SetGlobalStormSampleType(global_storm_sample_type))
    {
        CLI_LIB_PrintStr("Failed to set storm sample type.\r\n");
    }
#endif

    return CLI_NO_ERROR;
}

