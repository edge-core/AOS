#ifndef CLI_API_SYSTEM_H
#define CLI_API_SYSTEM_H

UI32_T CLI_API_Ping(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_TraceRoute(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_FanSpeed(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Reload(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Test_Snmp_Trap(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Enable_Password(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
/*
  EPR_ID:ES3628BT-FLF-ZZ-00057
  Problem: CLI: The behavior of hostname command is NOT correct.
  Root Cause: use error command line.
  Solution: 1. use "hostname" command is modification of "system name".
            2. Add "prompt" CLI command.
*/
UI32_T CLI_API_Prompt(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Username(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Hostname(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Jumbo_Frame(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Authentication_Login(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Authentication_Enable(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_PHYMAP(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Light_Unit(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Disconnect(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_IP_Telnet_Server(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_System_Mtu(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Reload(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_System_Mtu(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Ip_Telnet(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Banner_Motd(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#if (SYS_CPNT_STACKING == TRUE)
UI32_T CLI_API_Switch_Renumber(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_SwitchMasterButton(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_SwitchStackingButton(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#endif

UI32_T CLI_API_Show_Cable_Diag_Dsp_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Test_Cable_Diag_Dsp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Test_Cable_Diag(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Test_Cable_Diag_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Test_Loop_Internal_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Loop_Internal_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T HostString2IP(UI8_T *hoststr_p, UI32_T *ip_p);
UI32_T CLI_API_Show_Ip_Traffic(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_Process_Cpu(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Cpu_Guard(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Memory(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_Control_Plane(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#if (SYS_CPNT_CRAFT_PORT == TRUE)
UI32_T CLI_API_Interface_Craft(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Craft_Interface_Ip_Address(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#endif
UI32_T CLI_API_Show_SyncE_Status(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_SyncE_SSM(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_SyncE_ClockSource(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_SyncE(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Set_SyncE(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_SyncE_SSM(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_SyncE_ClockSourceSSM(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_Show_PSECheckStatus(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Set_PSECheckStatus(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_PDPortStatus(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_Set_Alarm_Input_Name(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_SetLocationLed(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_ShowLocationLed(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#if (SYS_CPNT_CHANGE_LOADER_BACKDOOR_PASSWORD == TRUE)
UI32_T CLI_API_Loader_Password(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
#endif
UI32_T CLI_API_ONIE(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_Hardware_Profile_Portmode(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Show_Hardware_Profile_Portmode(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#endif
