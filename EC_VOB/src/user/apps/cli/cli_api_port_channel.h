#ifndef CLI_API_PORT_CHANNEL_H
#define CLI_API_PORT_CHANNEL_H


UI32_T CLI_API_Interface_Portchannel(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Shutdown_Portchannel(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Description_Portchannel(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
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

UI32_T CLI_API_Alias_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Channelgroup(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Speedduplex_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Negotiation_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Capabilities_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Flowcontrol_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Port_Security_Action_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Port_Security_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_Mdix_Pch(UI16_T cmd_idx, UI8_T *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/* Load Balance */
UI32_T CLI_API_PortChannel_LoadBalance(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_PortChannel_LoadBalance(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_PortChannel_MaxActivePorts(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#endif
