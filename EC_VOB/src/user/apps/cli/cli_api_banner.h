#ifndef  CLI_API_BANNER_H
#define  CLI_API_BANNER_H

UI32_T CLI_API_Banner_Configure(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Banner_Configure_Company(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Banner_Configure_Department(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Banner_Configure_Manager_Info(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Banner_Configure_Equipment_Location(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Banner_Configure_Equipment_Info(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Banner_Configure_Dc_Power_Info(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Banner_Configure_Ip_Lan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Banner_Configure_Lp_Number(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Banner_Configure_Mux(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Banner_Configure_Note(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_Show_Banner(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#endif
