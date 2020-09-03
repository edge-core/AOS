#include "sysfun.h"
#include "sys_cpnt.h"
#include <string.h>
#include <stdlib.h>
#include <sys_type.h>
#include <time.h>
#include <ctype.h>
#include "sys_dflt.h"

/*cli internal*/
#include "clis.h"
#include "cli_def.h"
#include "cli_mgr.h"
#include "cli_io.h"
//#include "cli_type.h"
#include "cli_task.h"
#include "cli_func.h"
#include "cli_main.h"
#include "cli_lib.h"
#include "cli_api.h"
#include "cli_pars.h"
#include "cli_msg.h"
#include "cli_cmd.h"
#include "cli_runcfg.h"
#include "cli_api.h"

UI32_T CLI_API_Storm_Bcast_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RATE_BASED_STORM_CONTROL == TRUE)
    UI32_T i;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;
    
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_SWITCHPORT_BROADCAST:
            for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
            {
                UI32_T lport = 0;
                verify_port = i;

                if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                {
                   display_ethernet_msg(verify_ret, verify_unit, verify_port);
                   continue;
                }
                else
                {
                    UI32_T mode = 0;
                    UI32_T rate = 0;

                    if(!SWCTRL_POM_GetRateBasedStormControl(lport, &rate, &mode))
                    {
                        CLI_LIB_PrintStr("Fatal error before setting the broadcast storm control!");
                    }
                    else
                    {
                        mode |= (1 << VAL_rateBasedStormMode_bcastStorm);
                        if(!SWCTRL_PMGR_SetBroadcastStormMode(lport, rate, mode))
                        {
                            CLI_LIB_PrintStr("Fail to enable broadcast storm control!");
                        }
                    }
                }
            }
        
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_SWITCHPORT_BROADCAST:
            for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
            {
                UI32_T lport = 0;
                verify_port = i;

                if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                {
                   display_ethernet_msg(verify_ret, verify_unit, verify_port);
                   continue;
                }
                else
                {
                    UI32_T mode = 0;
                    UI32_T rate = 0;

                    if(!SWCTRL_POM_GetRateBasedStormControl(lport, &rate, &mode))
                    {
                        CLI_LIB_PrintStr("Fatal error before setting the broadcast storm control!");
                    }
                    else
                    {
                        mode &= ~(1 << VAL_rateBasedStormMode_bcastStorm);
                        if(!SWCTRL_PMGR_SetBroadcastStormMode(lport, rate, mode))
                        {
                            CLI_LIB_PrintStr("Fail to disable broadcast storm control!");
                        }
                    }
                }
            }
            break;
    
        default:
            break;
    }
#endif    
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Storm_Mcast_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RATE_BASED_STORM_CONTROL == TRUE)
    UI32_T i;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;
    
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_SWITCHPORT_MULTICAST:
            for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
            {
                UI32_T lport = 0;
                verify_port = i;

                if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                {
                   display_ethernet_msg(verify_ret, verify_unit, verify_port);
                   continue;
                }
                else
                {
                    UI32_T mode = 0;
                    UI32_T rate = 0;

                    if(!SWCTRL_POM_GetRateBasedStormControl(lport, &rate, &mode))
                    {
                        CLI_LIB_PrintStr("Fatal error before setting the broadcast storm control!");
                    }
                    else
                    {
                        mode |= (1 << VAL_rateBasedStormMode_mcastStorm);
                        if(!SWCTRL_PMGR_SetRateBasedStormControl(lport, rate, mode))
                        {
                            CLI_LIB_PrintStr("Fail to enable broadcast storm control!");
                        }
                    }
                }
            }
        
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_SWITCHPORT_MULTICAST:
            for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
            {
                UI32_T lport = 0;
                verify_port = i;

                if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                {
                   display_ethernet_msg(verify_ret, verify_unit, verify_port);
                   continue;
                }
                else
                {
                    UI32_T mode = 0;
                    UI32_T rate = 0;

                    if(!SWCTRL_POM_GetRateBasedStormControl(lport, &rate, &mode))
                    {
                        CLI_LIB_PrintStr("Fatal error before setting the broadcast storm control!");
                    }
                    else
                    {
                        mode &= ~(1 << VAL_rateBasedStormMode_mcastStorm);
                        if(!SWCTRL_PMGR_SetRateBasedStormControl(lport, rate, mode))
                        {
                            CLI_LIB_PrintStr("Fail to disable broadcast storm control!");
                        }
                    }
                }
            }
            break;
    
        default:
            break;
    }
#endif    
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Storm_UnknownUnicast_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RATE_BASED_STORM_CONTROL == TRUE)
    UI32_T i;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;
    
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_SWITCHPORT_UNKNOWNUNICAST:
            for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
            {
                UI32_T lport = 0;
                verify_port = i;

                if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                {
                   display_ethernet_msg(verify_ret, verify_unit, verify_port);
                   continue;
                }
                else
                {
                    UI32_T mode = 0;
                    UI32_T rate = 0;

                    if(!SWCTRL_POM_GetRateBasedStormControl(lport, &rate, &mode))
                    {
                        CLI_LIB_PrintStr("Fatal error before setting the broadcast storm control!");
                    }
                    else
                    {
                        mode |= (1 << VAL_rateBasedStormMode_unknownUcastStorm);
                        if(!SWCTRL_PMGR_SetRateBasedStormControl(lport, rate, mode))
                        {
                            CLI_LIB_PrintStr("Fail to enable broadcast storm control!");
                        }
                    }
                }
            }
        
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_SWITCHPORT_UNKNOWNUNICAST:
            for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
            {
                UI32_T lport = 0;
                verify_port = i;

                if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                {
                   display_ethernet_msg(verify_ret, verify_unit, verify_port);
                   continue;
                }
                else
                {
                    UI32_T mode = 0;
                    UI32_T rate = 0;

                    if(!SWCTRL_POM_GetRateBasedStormControl(lport, &rate, &mode))
                    {
                        CLI_LIB_PrintStr("Fatal error before setting the broadcast storm control!");
                    }
                    else
                    {
                        mode &= ~(1 << VAL_rateBasedStormMode_unknownUcastStorm);
                        if(!SWCTRL_PMGR_SetRateBasedStormControl(lport, rate, mode))
                        {
                            CLI_LIB_PrintStr("Fail to disable broadcast storm control!");
                        }
                    }
                }
            }
            break;
    
        default:
            break;
    }
#endif    
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Storm_Bcast_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RATE_BASED_STORM_CONTROL == TRUE)
    UI32_T lport = 0;
    UI32_T mode = 0;
    UI32_T rate = 0;
    CLI_API_TrunkStatus_T verify_ret;
    
    if( (verify_ret = verify_trunk(ctrl_P->CMenu.pchannel_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, ctrl_P->CMenu.pchannel_id);
        return CLI_NO_ERROR;
    }
    if(!SWCTRL_POM_GetRateBasedStormControl(lport, &rate, &mode))
    {
        CLI_LIB_PrintStr("Fatal error before setting the broadcast storm control!");
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_SWITCHPORT_BROADCAST:
            mode |= (1 << VAL_rateBasedStormMode_bcastStorm);
            if(!SWCTRL_PMGR_SetBroadcastStormMode(lport, rate, mode))
            {
                CLI_LIB_PrintStr("Fail to enable broadcast storm control!");
            }
      
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_SWITCHPORT_BROADCAST:
            mode &= ~(1 << VAL_rateBasedStormMode_bcastStorm);
            if(!SWCTRL_PMGR_SetBroadcastStormMode(lport, rate, mode))
            {
                CLI_LIB_PrintStr("Fail to disable broadcast storm control!");
            }

            break;
    
        default:
            break;
    }
#endif
    return CLI_NO_ERROR;
}
UI32_T CLI_API_Storm_Mcast_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RATE_BASED_STORM_CONTROL == TRUE)
    UI32_T lport = 0;
    UI32_T mode = 0;
    UI32_T rate = 0;
    CLI_API_TrunkStatus_T verify_ret;

    if( (verify_ret = verify_trunk(ctrl_P->CMenu.pchannel_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, ctrl_P->CMenu.pchannel_id);
        return CLI_NO_ERROR;
    }
    if(!SWCTRL_POM_GetRateBasedStormControl(lport, &rate, &mode))
    {
        CLI_LIB_PrintStr("Fatal error before setting the broadcast storm control!");
        return CLI_NO_ERROR;
    }
    
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_SWITCHPORT_MULTICAST:

            mode |= (1 << VAL_rateBasedStormMode_bcastStorm);
            if(!SWCTRL_PMGR_SetBroadcastStormMode(lport, rate, mode))
            {
                CLI_LIB_PrintStr("Fail to enable broadcast storm control!");
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_SWITCHPORT_MULTICAST:
            mode &= ~(1 << VAL_rateBasedStormMode_bcastStorm);
            if(!SWCTRL_PMGR_SetBroadcastStormMode(lport, rate, mode))
            {
                CLI_LIB_PrintStr("Fail to disable broadcast storm control!");
            }
            break;
    
        default:
            break;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Storm_UnknownUnicastPch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RATE_BASED_STORM_CONTROL == TRUE)
    UI32_T lport = 0;
    UI32_T mode = 0;
    UI32_T rate = 0;
    CLI_API_TrunkStatus_T verify_ret;

    if( (verify_ret = verify_trunk(ctrl_P->CMenu.pchannel_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, ctrl_P->CMenu.pchannel_id);
        return CLI_NO_ERROR;
    }
    if(!SWCTRL_POM_GetRateBasedStormControl(lport, &rate, &mode))
    {
        CLI_LIB_PrintStr("Fatal error before setting the broadcast storm control!");
        return CLI_NO_ERROR;
    }
    
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_SWITCHPORT_UNKNOWNUNICAST:
            mode |= (1 << VAL_rateBasedStormMode_bcastStorm);
            if(!SWCTRL_PMGR_SetBroadcastStormMode(lport, rate, mode))
            {
                CLI_LIB_PrintStr("Fail to enable broadcast storm control!");
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_SWITCHPORT_UNKNOWNUNICAST:
            mode &= ~(1 << VAL_rateBasedStormMode_bcastStorm);
            if(!SWCTRL_PMGR_SetBroadcastStormMode(lport, rate, mode))
            {
                CLI_LIB_PrintStr("Fail to disable broadcast storm control!");
            }
            break;
    
        default:
            break;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Storm_Rate_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RATE_BASED_STORM_CONTROL == TRUE)
    UI32_T rate = 0, mode = 0, tmp_rate = 0;
    CLI_API_EthStatus_T verify_ret;
    UI32_T i;
    UI32_T verify_port;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    
    rate = atoi(arg[0]);
    if (rate < 64 || rate > 1000000)
        CLI_LIB_PrintStr("Invalid input value");

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_SWITCHPORT_STORMRATE_BITRATEINKILO:
            for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
            {
                UI32_T lport = 0;
                verify_port = i;
        
                if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                {
                   display_ethernet_msg(verify_ret, verify_unit, verify_port);
                   continue;
                }
                else
                {
                    if(!SWCTRL_POM_GetRateBasedStormControl(lport, &tmp_rate, &mode))
                    {
                        CLI_LIB_PrintStr("Fatal error before setting the broadcast storm control!");
                    }
                    else
                    {
                        if(!SWCTRL_PMGR_SetRateBasedStormControl(lport, rate, mode))
                        {
                            CLI_LIB_PrintStr("Fail to enable broadcast storm control!");
                        }
                    }
                }
            }
            break;
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_SWITCHPORT_STORMRATE_BITRATEINKILO:
            for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
            {
                UI32_T lport = 0;
                verify_port = i;
        
                if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                {
                   display_ethernet_msg(verify_ret, verify_unit, verify_port);
                   continue;
                }
                else
                {
                    if(!SWCTRL_POM_GetRateBasedStormControl(lport, &tmp_rate, &mode))
                    {
                        CLI_LIB_PrintStr("Fatal error before setting the broadcast storm control!");
                    }
                    else
                    {
                        if(!SWCTRL_PMGR_SetRateBasedStormControl(lport, SYS_DFLT_RATE_BASED_STORM_CONTROL_RATE, mode))
                        {
                            CLI_LIB_PrintStr("Fail to enable broadcast storm control!");
                        }
                    }
                }
            }
    }
    
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Storm_Rate_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RATE_BASED_STORM_CONTROL == TRUE)
    UI32_T rate = 0, mode = 0, tmp_rate = 0;
    UI32_T lport = 0;
    CLI_API_TrunkStatus_T verify_ret;
    
    rate = atoi(arg[0]);
    if (rate < 64 || rate > 1000000)
        CLI_LIB_PrintStr("Invalid input value");
    
    if( (verify_ret = verify_trunk(ctrl_P->CMenu.pchannel_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, ctrl_P->CMenu.pchannel_id);
        return CLI_NO_ERROR;
    }
    
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_SWITCHPORT_STORMRATE_BITRATEINKILO: 
            if(!SWCTRL_POM_GetRateBasedStormControl(lport, &tmp_rate, &mode))
            {
                CLI_LIB_PrintStr("Fatal error before setting the broadcast storm control!");
                return CLI_NO_ERROR;
            }
            else
            {
                if(!SWCTRL_PMGR_SetRateBasedStormControl(lport, rate, mode))
                {
                    CLI_LIB_PrintStr("Fail to enable broadcast storm control!");
                }
            }
            break;
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_NO_SWITCHPORT_STORMRATE_BITRATEINKILO:
            if(!SWCTRL_POM_GetRateBasedStormControl(lport, &tmp_rate, &mode))
            {
                CLI_LIB_PrintStr("Fatal error before setting the broadcast storm control!");
                return CLI_NO_ERROR;
            }
            else
            {
                if(!SWCTRL_PMGR_SetRateBasedStormControl(lport, SYS_DFLT_RATE_BASED_STORM_CONTROL_RATE, mode))
                {
                    CLI_LIB_PrintStr("Fail to enable broadcast storm control!");
                }
            }
            break;
    }
#endif
    return CLI_NO_ERROR;
}
