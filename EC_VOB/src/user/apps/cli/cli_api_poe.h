#ifndef CLI_API_POE_H
#define CLI_API_POE_H

UI32_T CLI_API_POE_Show_Power_Inline_Status(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
UI32_T CLI_API_POE_Show_Power_Inline_TimeRange(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_POE_Show_Power_Mainpower(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
UI32_T CLI_API_POE_Power_Mainpower_Maximum_Allocation(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
UI32_T CLI_API_POE_Power_Inline(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_POE_Power_Inline_Maximum_Allocation(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
UI32_T CLI_API_POE_Power_Inline_Priority(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
UI32_T CLI_API_POE_Power_Download(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
UI32_T CLI_API_POE_Power_Inline_Compatible(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
#ifdef SYS_CPNT_POE_PSE_DOT3AT
UI32_T CLI_API_POE_Power_Inline_HighPower(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#endif
UI32_T CLI_API_POE_Power_Inline_TimeRange(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#endif
