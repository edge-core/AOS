#ifndef CLI_API_AAA_H
#define CLI_API_AAA_H

UI32_T CLI_API_Aaa_Accounting_Dot1x(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
UI32_T CLI_API_Aaa_Accounting_Exec(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Aaa_Accounting_Commands(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_Aaa_Group_Server(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
UI32_T CLI_API_Aaa_Accounting_Update(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
UI32_T CLI_API_Accounting_Dot1x(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
UI32_T CLI_API_Show_Accounting(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
UI32_T CLI_API_AAA_SG_Server(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
UI32_T CLI_API_Accounting_Exec_Console(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
UI32_T CLI_API_Accounting_Exec_Vty(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
UI32_T CLI_API_Accounting_Commands_Console(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
UI32_T CLI_API_Accounting_Commands_Vty(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);

/*AUTHORIZATION*/
UI32_T CLI_API_Show_Authorization(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
UI32_T CLI_API_Aaa_Authorization_Commands(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
UI32_T CLI_API_Aaa_Authorization_Exec(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
UI32_T CLI_API_Authorization_Exec_Console(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
UI32_T CLI_API_Authorization_Exec_Vty(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
UI32_T CLI_API_Authorization_Commands_Console(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);
UI32_T CLI_API_Authorization_Commands_Vty(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);

UI32_T CLI_API_Ip_Http_Authentication_AAA_Exec_Authorization(
    UI16_T cmd_idx, char *arg[], 
    CLI_TASK_WorkingArea_T  *ctrl_P
    );
#endif

