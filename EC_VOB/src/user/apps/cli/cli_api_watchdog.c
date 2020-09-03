#include <stdlib.h>
#include <string.h>
#include "sw_watchdog_mgr.h"
#include "cli_api.h"

UI32_T CLI_API_Watchdog_Software(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if ( SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    BOOL_T status = FALSE;

    switch(cmd_idx)
    {
        /* Enable global software watchdog */
        case PRIVILEGE_EXEC_CMD_W3_WATCHDOG_SOFTWARE_ENABLE:
            status = SW_WATCHDOG_STATUS_ENABLE;
            break;
        /* Disable global software watchdog */
        case PRIVILEGE_EXEC_CMD_W3_WATCHDOG_SOFTWARE_DISABLE:
            status = SW_WATCHDOG_STATUS_DISABLE;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (SW_WATCHDOG_MGR_SetMonitorStatus(status) == FALSE)
    {
         CLI_LIB_PrintStr("Failed to set software watchdog status.");
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Watchdog_Software(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if ( SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    BOOL_T status=FALSE;

    if (SW_WATCHDOG_MGR_GetMonitorStatus(&status) == FALSE)
    {
        CLI_LIB_PrintStr("Failed to get software watchdog status");
    }
    else
    {
        CLI_LIB_PrintStr("\nSoftware Watchdog Information\r\n");
        CLI_LIB_PrintStr_1(" Status :    %s\r\n", (status== SW_WATCHDOG_STATUS_ENABLE ? "Enabled" : "Disabled"));
    }
#endif
    return CLI_NO_ERROR;
}
