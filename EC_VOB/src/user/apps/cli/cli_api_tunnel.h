#ifndef  CLI_API_TUNNEL_H
#define  CLI_API_TUNNEL_H

#include "sys_hwcfg.h"
#include "cli_def.h"
#include "sys_cpnt.h"
#include "l_prefix.h"
#include "l_inet.h"
#include "leaf_4001.h"

UI32_T CLI_API_Interface_Tunnel(UI16_T cmd_idx, char*arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Tunnel_Ipv6Address(UI16_T cmd_idx, char*arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Tunnel_Source(UI16_T cmd_idx, char*arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Tunnel_Deatination(UI16_T cmd_idx, char*arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Tunnel_Nd(UI16_T cmd_idx, char*arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Tunnel_Ttl(UI16_T cmd_idx, char*arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T  CLI_API_Tunnel_Mode(UI16_T cmd_idx, char*arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T  CLI_API_ShowIpv6Tunnel(UI16_T cmd_idx, char*arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#endif /*CLI_API_TUNNEL_H*/
