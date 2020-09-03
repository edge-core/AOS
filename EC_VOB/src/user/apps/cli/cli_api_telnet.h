#ifndef CLI_API_TELNET_H
#define CLI_API_TELNET_H

UI32_T CLI_API_Telnet_Client(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
BOOL_T telnet_client_event_check(int fd);

#endif

