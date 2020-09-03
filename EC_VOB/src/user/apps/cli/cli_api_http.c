#include <string.h>
#include <stdlib.h>
#include <sys_type.h>
#include <time.h>
#include "sysfun.h"
#include "sys_dflt.h"
#include "sys_cpnt.h"

/*cli internal*/
#include "cli_type.h"
#include "clis.h"
#include "cli_def.h"
#include "cli_mgr.h"
#include "cli_msg.h"
#include "cli_io.h"
#include "cli_task.h"
#include "cli_lib.h"
#include "cli_pars.h"
#include "cli_cmd.h"
#include "http_pmgr.h"
#include "http_type.h"

#include "cli_api.h"
#include "cli_api_http.h"

UI32_T CLI_API_Ip_Http_Port(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_HTTP_UI == TRUE)

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_IP_HTTP_PORT:
            if (!HTTP_PMGR_Set_Http_Port (atoi(arg[0])))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set HTTP port\r\n");
#endif
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_IP_HTTP_PORT:
            if (!HTTP_PMGR_Set_Http_Port (HTTP_MGR_DEFPORT))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set HTTP default port\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif  /* #if (SYS_CPNT_HTTP_UI == TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Ip_Http_Server(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_HTTP_UI == TRUE)

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_IP_HTTP_SERVER:
            if (!HTTP_PMGR_Set_Http_Status(HTTP_STATE_ENABLED))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to enable HTTP server\r\n");
#endif
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_IP_HTTP_SERVER:
            if (!HTTP_PMGR_Set_Http_Status(HTTP_STATE_DISABLED))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to disable HTTP server\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif  /* #if (SYS_CPNT_HTTP_UI == TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Ip_Http_SecurePort(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_HTTP_UI == TRUE)
#if (SYS_CPNT_HTTPS == TRUE)

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_IP_HTTP_SECUREPORT:
            if (!HTTP_PMGR_Set_Secure_Port(atoi(arg[0])))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set HTTPS port\r\n");
#endif
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_IP_HTTP_SECUREPORT:
            if (!HTTP_PMGR_Set_Secure_Port(SYS_DFLT_IP_HTTP_SECURE_PORT/*SYS_DFLT_IP_HTTP_SECURE_PORT*/))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set HTTPS default port\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif  /* #if (SYS_CPNT_HTTPS == TRUE) */
#endif  /* #if (SYS_CPNT_HTTP_UI == TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Ip_Http_SecureServer(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_HTTP_UI == TRUE)
#if (SYS_CPNT_HTTPS == TRUE)

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_IP_HTTP_SECURESERVER:
            if (!HTTP_PMGR_Set_Secure_Http_Status(SECURE_HTTP_STATE_ENABLED))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to enable HTTPS server.\r\n");
#endif
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_IP_HTTP_SECURESERVER:
            if (!HTTP_PMGR_Set_Secure_Http_Status(SECURE_HTTP_STATE_DISABLED))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to disable HTTPS server.\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif  /* #if (SYS_CPNT_HTTPS == TRUE) */
#endif  /* #if (SYS_CPNT_HTTP_UI == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Debug_Ip_Http_SecureAll(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_HTTP_UI == TRUE)
#if (CLI_SUPPORT_DEBUG_HTTPS == 1)

    switch(cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W4_DEBUG_IP_HTTP_SECUREALL:
            if (!HTTP_PMGR_Enable_Debug_Information(HTTP_MGR_DEBUG_ALL))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to enable HTTP secure all function\r\n");
#endif
            }
            break;

        case PRIVILEGE_EXEC_CMD_W5_NO_DEBUG_IP_HTTP_SECUREALL:
            if (!HTTP_PMGR_Disable_Debug_Information(HTTP_MGR_DEBUG_ALL))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to disable HTTP secure all function\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif  /* #if (CLI_SUPPORT_DEBUG_HTTPS == 1) */
#endif  /* #if (SYS_CPNT_HTTP_UI == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Debug_Ip_Http_SecureSession(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_HTTP_UI == TRUE)
#if (CLI_SUPPORT_DEBUG_HTTPS == 1)

    HTTP_MGR_DebugType_T debug_type;

    HTTP_MGR_Get_Running_Debug_Information_Status(&debug_type);
    switch(cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W4_DEBUG_IP_HTTP_SECURESESSION:
            if (!HTTP_PMGR_Enable_Debug_Information(HTTP_MGR_DEBUG_SESSION))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to enable HTTP secure session function\r\n");
#endif
            }
            break;

        case PRIVILEGE_EXEC_CMD_W5_NO_DEBUG_IP_HTTP_SECURESESSION:
            if (!HTTP_PMGR_Disable_Debug_Information(HTTP_MGR_DEBUG_SESSION))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to disable HTTP secure session function\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif  /* #if (CLI_SUPPORT_DEBUG_HTTPS == 1) */
#endif  /* #if (SYS_CPNT_HTTP_UI == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Debug_Ip_Http_SecureState(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_HTTP_UI == TRUE)
#if (CLI_SUPPORT_DEBUG_HTTPS == 1)

    HTTP_MGR_DebugType_T debug_type;

    HTTP_MGR_Get_Running_Debug_Information_Status(&debug_type);
    switch(cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W4_DEBUG_IP_HTTP_SECURESTATE:
            if (!HTTP_PMGR_Enable_Debug_Information(HTTP_MGR_DEBUG_STATE))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to enable HTTP secure state function\r\n");
#endif
            }
            break;

        case PRIVILEGE_EXEC_CMD_W5_NO_DEBUG_IP_HTTP_SECURESTATE:
            if (!HTTP_PMGR_Disable_Debug_Information(HTTP_MGR_DEBUG_STATE))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to disable HTTP secure state function\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif  /* #if (CLI_SUPPORT_DEBUG_HTTPS == 1) */
#endif  /* #if (SYS_CPNT_HTTP_UI == TRUE) */

    return CLI_NO_ERROR;
}

