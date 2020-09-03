#ifndef CLI_API_HASH_SELECTION_H
#define CLI_API_HASH_SELECTION_H

UI32_T CLI_API_HashSelection(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_HashSelection_MAC(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_HashSelection_IPv4(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_HashSelection_IPv6(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_ShowHashSelection(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#endif /*#ifndef CLI_API_HASH_SELECTION_H*/
