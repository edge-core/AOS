#ifndef CLI_API_NTP_H
#define CLI_API_NTP_H


UI32_T CLI_API_Ntp_Client(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Ntp_Broadcast_Client(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Ntp_Server(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Ntp_Authenticate(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Ntp_Authenticationkey(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#if 0 /* QingfengZhang, 01 April, 2005 2:18:00 */
UI32_T CLI_API_Ntp_Trustedkey(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#endif /* #if 0 */
UI32_T CLI_API_Show_Ntp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


#endif
