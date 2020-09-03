#ifndef CLI_API_SNTP_H
#define CLI_API_SNTP_H


UI32_T CLI_API_Sntp_Client(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Sntp_Broadcast_Client(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Sntp_Poll(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Sntp_Server(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Sntp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


#endif

