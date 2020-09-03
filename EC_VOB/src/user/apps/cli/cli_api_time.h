#ifndef CLI_API_TIME_H
#define CLI_API_TIME_H

UI32_T CLI_API_Calendar_Set(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Calendar(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Clock_SummerTime(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#if (SYS_CPNT_SYS_TIME == TRUE)
UI32_T 
CLI_API_Clock_Timezone(
    UI16_T cmd_idx, 
    char *arg[], 
    CLI_TASK_WorkingArea_T *ctrl_P
    );

UI32_T
CLI_API_Clock_Timezone_Predefined(
    UI16_T cmd_idx,
    char *arg[],
    CLI_TASK_WorkingArea_T *ctrl_P
    );
#endif /* #if (SYS_CPNT_SYS_TIME == TRUE) */
#endif /* #ifndef CLI_API_TIME_H */

