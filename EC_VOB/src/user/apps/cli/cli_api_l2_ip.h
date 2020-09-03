#ifndef CLI_API_L2_IP_H
#define CLI_API_L2_IP_H


#include "cli_def.h"

#if (CLI_SUPPORT_L3_FEATURE != 1)
UI32_T CLI_API_Show_Ip_Interfaces(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Ip_Redirects(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
/* move to cli_api_system.c, UI32_T CLI_API_Show_Ip_Traffic(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P); */
UI32_T CLI_API_Ip_Address(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_IP_Dhcp_Restart(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Ip_Defaultgateway(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
/*UI32_T CLI_API_L3_Ip_Route(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Show_Ip_Route(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);*/
UI32_T CLI_API_Clear_Arp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Arp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Arp_Timeout(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_Ip_Address_Eth0(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/* SYS_CPNT_ECMP_BALANCE_MODE */
UI32_T CLI_API_Ecmp_Load_Balance(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_L3_Show_Ecmp_Load_Balance(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#endif

#endif

