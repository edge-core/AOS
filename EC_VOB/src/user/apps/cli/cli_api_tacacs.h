#ifndef CLI_API_TACACS_H
#define CLI_API_TACACS_H

UI32_T CLI_API_Tacacs_Server_Port(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Tacacs_Server_Retransmit(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Tacacs_Server_Timeout(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Tacacs_Server_Key(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#if 0 /*maggie liu, ES3628BT-FLF-ZZ-00052/ES3628BT-FLF-ZZ-00158/ES4827G-FLF-ZZ-00404*/
UI32_T CLI_API_Tacacs_Server_Host(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#endif
UI32_T CLI_API_Show_Tacacs_Server(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Tacacs_Server(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#endif

