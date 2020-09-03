#ifndef CLI_API_PVLAN_MS_H
#define CLI_API_PVLAN_MS_H


/* 2007-07-16 Eugene add for Private VLAN traffic segmentation feature */
UI32_T CLI_API_Pvlan_MS(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Pvlan_MS(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Pvlan_MS_U2U(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


#endif
