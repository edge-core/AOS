#ifndef CLI_API_PPPOE_IA_H
#define CLI_API_PPPOE_IA_H

/* Global mode */
UI32_T CLI_API_PPPoE_IntermediateAgent(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_PPPoE_IntermediateAgent_FmtType(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/* Interface mode */
UI32_T CLI_API_PPPoE_IntermediateAgent_PortEn_Eth(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_PPPoE_IntermediateAgent_PortEn_Pch(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_PPPoE_IntermediateAgent_Trust_Eth(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_PPPoE_IntermediateAgent_Trust_Pch(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_PPPoE_IntermediateAgent_VTag_Strip_Eth(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_PPPoE_IntermediateAgent_VTag_Strip_Pch(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_PPPoE_IntermediateAgent_PortFmtType_Eth(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_PPPoE_IntermediateAgent_PortFmtType_Pch(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/* Exec mode */
UI32_T CLI_API_PPPoE_Clear_IntermediateAgent_Statistics(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_PPPoE_Show_IntermediateAgent_Info(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_PPPoE_Show_IntermediateAgent_Statistics(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#endif /* #ifndef CLI_API_PPPOE_IA_H */

