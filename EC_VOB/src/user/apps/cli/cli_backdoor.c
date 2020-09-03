/* MODULE NAME: cli_backdoor.c
 * PURPOSE:
 *	Implementations for the CLI backdoor

 * NOTES:
 *	None.
 *
 * HISTORY:
 *    mm/dd/yy (A.D.)
 *    05/14/15    -- Emma, Create
 *
 * Copyright(C)      Accton Corporation, 2015
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sys_cpnt.h"
#include "sys_type.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#include "cli_backdoor.h"
#include "cli_om_exp.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define MAXLINE 255

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void CLI_BACKDOOR_Engine();
static void CLI_BACKDOOR_ShowCmd();

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
static void CLI_BACKDOOR_GetAllAddRunconfigEntry();
#endif /* SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG */

/* STATIC VARIABLE DEFINITIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
void CLI_BACKDOOR_Main(void)
{
    BACKDOOR_MGR_Printf("\r\nCLI Backdoor");
    CLI_BACKDOOR_Engine();
    return;
}

/* LOCAL SUBPROGRAM BODIES
 */
static void CLI_BACKDOOR_Engine()
{
    BOOL_T  engine_continue = TRUE;
    UI8_T   ch = 'q';
    UI8_T   cmd_buf[MAXLINE];

    while(engine_continue)
    {
        cmd_buf[0] = 0;

        CLI_BACKDOOR_ShowCmd();
        ch = BACKDOOR_MGR_GetChar();

        switch(ch)
        {
#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
            case '1':
                CLI_BACKDOOR_GetAllAddRunconfigEntry();
                break;
#endif /* SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG */

            case 'q':
                engine_continue = FALSE;
                break;

            default:
                continue;
        }
    }
}

static void CLI_BACKDOOR_ShowCmd()
{
#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
    BACKDOOR_MGR_Printf("\r\n [1] Get all addrunconfig entries");
#endif /* SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG */

    BACKDOOR_MGR_Printf("\r\n [q] Quit");
    BACKDOOR_MGR_Printf("\r\n input: ");
}

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
static void CLI_BACKDOOR_GetAllAddRunconfigEntry()
{
    CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T entry;
    UI32_T index = 0;
    char *entry_status_str[] = {"INVALID", "PENDING", "RUNNING", "DONE"};

    while (CLI_OM_EXP_ADDRUNCONFIG_SUCCESS == CLI_OM_EXP_ADDRUNCONFIG_GetNextEntry(&index, &entry))
    {
        BACKDOOR_MGR_Printf("\r\n\r\n ====================");
        BACKDOOR_MGR_Printf("\r\n index: %d", index);
        BACKDOOR_MGR_Printf("\r\n id: %lu", entry.id);
        BACKDOOR_MGR_Printf("\r\n status: %s", entry_status_str[entry.status]);
        BACKDOOR_MGR_Printf("\r\n finish time by ticks: %d", entry.finish_time_by_ticks);
        BACKDOOR_MGR_Printf("\r\n commands: %s", entry.commands);
    }
}
#endif /* SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG */

