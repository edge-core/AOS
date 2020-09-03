#ifndef CLI_API_CWMP_H
#define CLI_API_CWMP_H

UI32_T CLI_API_Cwmp_Url(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_Cwmp_Username(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_Cwmp_Password(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_Cwmp_PeriodicInform(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_Cwmp_PeriodicInformInterval(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_Cwmp_ConnReqUsername(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_Cwmp_ConnReqPassword(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_Show_Cwmp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#endif
