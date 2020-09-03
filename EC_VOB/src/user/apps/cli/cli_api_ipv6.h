#ifndef  CLI_API_IPV6_H
#define  CLI_API_IPV6_H

#include "sys_hwcfg.h"
#include "cli_def.h"
#include "sys_cpnt.h"
#include "l_prefix.h"
#include "l_inet.h"
#include "leaf_4001.h"


UI32_T  CLI_API_IPV6_NdHopLimit(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T  CLI_API_IPV6_Neighbor(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T  CLI_API_IPV6_NdRouterPreference(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
                  
UI32_T  CLI_API_IPV6_NdPrefix(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);;
UI32_T  CLI_API_IPV6_NsInterval(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_IPV6_NdReachableTime(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_IPV6_NdRouterLifeTime(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T  CLI_API_IPV6_NdManagedConfig(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T  CLI_API_IPV6_NdOtherConfig(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T  CLI_API_IPV6_NdRaSuppress(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T  CLI_API_IPV6_NdRaInterval(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


UI32_T  CLI_API_Show_Ipv6_Default_Gateway(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T  CLI_API_Show_Ipv6_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T  CLI_API_Show_Ipv6_Mtu(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T  CLI_API_Show_Ipv6_Neighbors(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#if (SYS_CPNT_IPV6_ROUTING == TRUE)
UI32_T  CLI_API_L3_Show_IpV6_Route(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#endif
UI32_T  CLI_API_Show_Ipv6_Traffic(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


UI32_T  CLI_API_Ipv6_Default_Gateway(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T  CLI_API_Ipv6_Neighbor(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T  CLI_API_Ping_Ipv6(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T  CLI_API_Clear_Ipv6_Neighbors(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T  CLI_API_Clear_Ipv6_Traffic(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T  CLI_API_Ipv6_Address(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T  CLI_API_Ipv6_Address_Autoconfig(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T  CLI_API_Ipv6_Enable(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T  CLI_API_Ipv6_Mtu(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T  CLI_API_Ipv6_Nd_Dad_Attempts(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T  CLI_API_Ipv6_Nd_Ns_Interval(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T  CLI_API_L3_IPv6_Route(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
//UI32_T CLI_API_IPv6_UnicastRouting(UI16_T cmd_idx, UI8_T *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_IPv6_TraceRoute(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#if (SYS_CPNT_CRAFT_PORT == TRUE)
UI32_T CLI_API_Craft_Interface_Ipv6_Address(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Craft_Interface_Ipv6_Enable(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#endif

UI32_T  CLI_API_IPV6_Nd_RaGuard_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T  CLI_API_IPV6_Nd_RaGuard_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T  CLI_API_Show_IPv6_ND_RaGuard(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#endif /*CLI_API_IPV6_H*/

