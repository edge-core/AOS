#ifndef CLI_API_PFC_H
#define CLI_API_PFC_H

/* Global mode */

/* Interface mode */
UI32_T CLI_API_PFC_Mode_Eth(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_PFC_Mode_Pch(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_PFC_PriorityEnable_Eth(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_PFC_PriorityEnable_Pch(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_PFC_LinkDelayAllowance_Eth(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_PFC_LinkDelayAllowance_Pch(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/* Exec mode */
UI32_T CLI_API_PFC_ClearStatistics(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_PFC_Show(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#endif /* #ifndef CLI_API_PFC_H */

