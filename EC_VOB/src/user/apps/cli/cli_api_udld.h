#ifndef CLI_API_UDLD_H
#define CLI_API_UDLD_H

/* Global mode */
UI32_T CLI_API_Udld_MessageInterval(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Udld_DetectionInterval(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Udld_RecoveryInterval(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Udld_Recovery(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/* Interface mode */
UI32_T CLI_API_Udld_Aggressive_Eth(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Udld_Port_Eth(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/* Exec mode */
UI32_T CLI_API_Udld_Show(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#endif /* #ifndef CLI_API_UDLD_H */

