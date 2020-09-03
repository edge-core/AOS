#include "cli_api.h"
#include "cli_api_rate_limit.h"
#include <stdio.h>

UI32_T CLI_API_Ratelimit_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_INGRESS_RATE_LIMIT == TRUE || SYS_CPNT_EGRESS_RATE_LIMIT == TRUE)   
   UI32_T i;
   UI32_T rate_limit = 0;
   UI32_T lport;
   BOOL_T is_set_last = FALSE;
   BOOL_T is_set_disable = FALSE;
   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;
   CLI_API_EthStatus_T verify_ret;
   
   switch(cmd_idx)
   {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_RATELIMIT:
            if(arg[1]==0)
            {
                is_set_last = TRUE;
            }
            else
            {
#ifdef SYS_ADPT_UI_RATE_LIMIT_FACTOR
                rate_limit = atoi(arg[1]) * SYS_ADPT_UI_RATE_LIMIT_FACTOR;
#else
                /* for backwawrd-compatible, use Mbps as default unit.
                 * new project should specify SYS_ADPT_UI_RATE_LIMIT_FACTOR.
                 */
                rate_limit = atoi(arg[1]) * 1000;
#endif
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_RATELIMIT:
            is_set_disable = TRUE;
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

            if(arg[0][0]=='i' || arg[0][0]=='I')
            {
#if (SYS_CPNT_INGRESS_RATE_LIMIT == TRUE)
               if(is_set_disable==TRUE)
               {	
                  if(!SWCTRL_PMGR_DisablePortIngressRateLimit(lport))
                  {
#if (SYS_CPNT_EH == TRUE)
                     CLI_API_Show_Exception_Handeler_Msg();
#else
                     CLI_LIB_PrintStr_2("Failed to disable ingress rate-limit on ethernet %lu/%lu\r\n",(unsigned long)verify_unit,(unsigned long)verify_port);
#endif
                  }     
               }   		
               else
               {
				  if(TRUE == is_set_last)
                  {
                  	if(!SWCTRL_PMGR_EnablePortIngressRateLimit(lport))
#if (SYS_CPNT_EH == TRUE)
                     CLI_API_Show_Exception_Handeler_Msg();
#else
                     CLI_LIB_PrintStr_2("Failed to enable ingress rate-limit on ethernet %lu/%lu\r\n",(unsigned long)verify_unit,(unsigned long)verify_port);
#endif
                  } 	
                  else
                  {              
                     if(!SWCTRL_PMGR_SetPortIngressRateLimit(lport, rate_limit))
                     {
#if (CLI_SUPPORT_PORT_NAME == 1)  
                        {             
                           UI8_T name[MAXSIZE_ifName+1] = {0};
                           CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                           CLI_API_Show_Exception_Handeler_Msg();
#else
                           CLI_LIB_PrintStr_2("Failed to %s ingress rate-limit on ethernet %s\r\n", status == PORT_ACTIVE? "enable" : "disable", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set ingress rate-limit on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);               
#endif
#endif
                     }
                  }   
               }
#endif/*ingress symbol define*/
            }        
            else if(arg[0][0]=='o' || arg[0][0]=='O')
            {
#if (SYS_CPNT_EGRESS_RATE_LIMIT == TRUE)
               if(is_set_disable==TRUE)
               {	
                  if(!SWCTRL_PMGR_DisablePortEgressRateLimit(lport))
                  {
#if (SYS_CPNT_EH == TRUE)
                     CLI_API_Show_Exception_Handeler_Msg();
#else
                     CLI_LIB_PrintStr_2("Failed to disable egress rate-limit on ethernet %lu/%lu\r\n",(unsigned long)verify_unit,(unsigned long)verify_port);
#endif
                  }
               }   	
               else
               {
				  if(TRUE == is_set_last)
                  {
                  	if(!SWCTRL_PMGR_EnablePortEgressRateLimit(lport))
#if (SYS_CPNT_EH == TRUE)
                     CLI_API_Show_Exception_Handeler_Msg();
#else
                     CLI_LIB_PrintStr_2("Failed to enable egress rate-limit on ethernet %lu/%lu\r\n",(unsigned long)verify_unit,(unsigned long)verify_port);
#endif
                  }
                  else
                  { 	
                     if(!SWCTRL_PMGR_SetPortEgressRateLimit(lport, rate_limit))
                     {
#if (CLI_SUPPORT_PORT_NAME == 1)  
                        {             
                           UI8_T name[MAXSIZE_ifName+1] = {0};
                           CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                           CLI_API_Show_Exception_Handeler_Msg();
#else
                           CLI_LIB_PrintStr_2("Failed to %s rate-limit on ethernet %s\r\n", status == PORT_ACTIVE? "enable" : "disable", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set rate-limit on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);               
#endif
#endif
                     }
                  }
               }  
#endif
            }          	
      }
   }
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Ratelimit_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_INGRESS_RATE_LIMIT == TRUE || SYS_CPNT_EGRESS_RATE_LIMIT == TRUE)   
    UI32_T rate_limit = 0;
    UI32_T lport;
    BOOL_T is_set_last = FALSE;
    BOOL_T is_set_disable = FALSE;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    CLI_API_TrunkStatus_T verify_ret;

    if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W1_RATELIMIT:
            if(arg[1]==0)
            {
                is_set_last = TRUE;	
            }
            else
            {	
#ifdef SYS_ADPT_UI_RATE_LIMIT_FACTOR
                rate_limit = atoi(arg[1]) * SYS_ADPT_UI_RATE_LIMIT_FACTOR;
#else
                /* for backwawrd-compatible, use Mbps as default unit.
                * new project should specify SYS_ADPT_UI_RATE_LIMIT_FACTOR.
                */
                rate_limit = atoi(arg[1]) * 1000;
#endif
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_NO_RATELIMIT:
            is_set_disable = TRUE;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if(arg[0][0]=='i' || arg[0][0]=='I')
    {
#if (SYS_CPNT_INGRESS_RATE_LIMIT == TRUE)
        if(is_set_disable==TRUE)
        {	
            if(!SWCTRL_PMGR_DisablePortIngressRateLimit(lport))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("Failed to disable input rate-limit on port-channel %lu\r\n",(unsigned long)verify_trunk_id);
#endif
            }
        }   		
        else
        {
            if (is_set_last)
            {
                if(!SWCTRL_PMGR_EnablePortIngressRateLimit(lport))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to enable ingress rate-limit on port-channel %lu\r\n",(unsigned long)verify_trunk_id);
#endif
                }
            } 	
            else
            {
                if(!SWCTRL_PMGR_SetPortIngressRateLimit(lport, rate_limit))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to set rate-limit on port-channel %lu\r\n", (unsigned long)verify_trunk_id);               
#endif
                }
            }
        }
#endif
    }
    else if(arg[0][0]=='o' || arg[0][0]=='O')
    {
#if (SYS_CPNT_EGRESS_RATE_LIMIT == TRUE)
        if(is_set_disable==TRUE)
        {	
            if(!SWCTRL_PMGR_DisablePortEgressRateLimit(lport))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("Failed to disable output rate-limit on port-channel %lu\r\n",(unsigned long)verify_trunk_id);
#endif
            }
        }   		
        else
        {
            if (is_set_last)
            {
                if(!SWCTRL_PMGR_EnablePortEgressRateLimit(lport))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to enable ingress rate-limit on port-channel %lu\r\n",(unsigned long)verify_trunk_id);
#endif
                }
            } 
            else
            {		
                if(!SWCTRL_PMGR_SetPortEgressRateLimit(lport, rate_limit))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to set rate-limit on port-channel %lu\r\n", (unsigned long)verify_trunk_id);               
#endif
                }	
            }
        }
#endif
    }	   
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Ratelimit(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W1_RATELIMIT:
      if(!SWCTRL_PMGR_EnableIngressRateLimit())
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to enable rate-limit\r\n");
#endif
      }
      break;
      
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_RATELIMIT:
      if(!SWCTRL_PMGR_DisableIngressRateLimit())
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to disable rate-limit\r\n");
#endif
      }
      break;
      
   default:
      return CLI_ERR_INTERNAL;
   }
#endif
   return CLI_NO_ERROR;
}
