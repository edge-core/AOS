#if (SYS_CPNT_HTTP_UI == TRUE)
#if (SYS_CPNT_HTTPS == TRUE)

UI32_T CLI_API_Ip_Http_SecurePort(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P) ;
UI32_T CLI_API_Ip_Http_SecureServer(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#if (CLI_SUPPORT_DEBUG_HTTPS == 1)
UI32_T CLI_API_Debug_Ip_Http_SecureAll(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P) ;
UI32_T CLI_API_Debug_Ip_Http_SecureSession(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P) ;
UI32_T CLI_API_Debug_Ip_Http_SecureState(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P) ;
#endif

UI32_T CLI_API_Ip_Http_Port(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_Ip_Http_Server(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#endif  /* #if (SYS_CPNT_HTTPS == TRUE) */
#endif /* #if (SYS_CPNT_HTTP_UI == TRUE) */
