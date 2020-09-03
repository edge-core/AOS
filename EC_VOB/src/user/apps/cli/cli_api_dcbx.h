#ifndef CLI_API_DCBX_H
#define CLI_API_DCBX_H

UI32_T CLI_API_Dcbx_eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Dcbx_pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_DcbxMode_eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_DcbxMode_pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Dcbx(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


#endif
