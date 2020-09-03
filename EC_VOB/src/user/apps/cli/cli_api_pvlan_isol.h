#ifndef CLI_API_PVLAN_ISOL_H
#define CLI_API_PVLAN_ISOL_H
#ifndef isspace
#define isspace(c)	((c) == ' ')
#endif
UI32_T CLI_API_Pvlan_Isol(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Pvlan_Isol(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Switchport_Protected_Isol(UI16_T cmd_idx, UI8_T *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


#endif

