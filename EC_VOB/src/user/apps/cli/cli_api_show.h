/* MODULE NAME: cli_api_show.h
 *
 * PURPOSE: For CLI to show those information, which does not belong to
 * any existing CSC-based "cli_api_....c" files.
 *
 * NOTES:
 *
 * HISTORY (mm/dd/yyyy)
 *    07/27/2011 - Qiyao Zhong, Added this file-head comment
 *
 * Copyright(C)      Accton Corporation, 2011
 */
#ifndef CLI_API_SHOW_H
#define CLI_API_SHOW_H

UI32_T CLI_API_Show_Interface_Runningconfig(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_History(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Startupconfig(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Runningconfig(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_System(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Alarm_Status(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Alarm_Input_Name(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Users(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Version(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Bridge(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Interfaces_Status(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Interfaces_Counters(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_Show_Interfaces(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Memory(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Process_CPU(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Cpu_Guard(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Process_CPU_Task(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_TechSupport(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_Show_Log(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P);

UI32_T CLI_API_Show_SwitchMasterButton(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
UI32_T CLI_API_Show_SwitchStackingButton(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#endif

UI32_T CLI_API_Show_Interfaces_Brief(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_Show_Interfaces_Craft(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_Show_Interfaces_Transceiver_Threshold(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Interfaces_Transceiver(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

/* print interactive mode */
BOOL_T CLI_API_Get_Print_Interactive_Mode();
void CLI_API_Set_Print_Interactive_Mode(BOOL_T mode);

#if (SYS_CPNT_POWER_SAVE == TRUE)
UI32_T CLI_API_Show_PowerSave(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#endif

UI32_T CLI_API_Show_Interfaces_History(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#endif  /* CLI_API_SHOW_H */
