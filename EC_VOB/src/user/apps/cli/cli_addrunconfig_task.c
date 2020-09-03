/* MODULE NAME: cli_addrunconfig_task.c
 * PURPOSE:
 *  This file is a demonstration code for implementations of CLI addrunconfig task.
 *
 * NOTES:
 *  None.
 *
 * HISTORY:
 *    mm/dd/yy (A.D.)
 *    05/18/15    -- Emma, Create
 *
 * Copyright(C)      Accton Corporation, 2015
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "sys_type.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "sys_module.h"
#include "buffer_mgr.h"
#include "buffermgmt_init.h"

#include "cli_addrunconfig_task.h"
#include "cli_type.h"
#include "cli_task.h"
#include "cli_om_exp.h"

/* TYPE DEFINITIONS
 */

/* LOCAL FUNCTIONS DECLARATIONS
 */
static void CLI_ADDRUNCONFIG_RunAllPartialProvision();

/* LOCAL VARIABLES DECLARATIONS
 */

/* MACRO DEFINITIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_ADDRUNCONFIG_CreateTask
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to create CLI addrunconfig task
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CLI_ADDRUNCONFIG_CreateTask(void)
{
    UI32_T tid;

    /* create a thread for take care of related event
     */
    if(SYSFUN_SpawnThread(SYS_BLD_ALU_THREAD_PRIORITY,
                          SYS_BLD_ALU_THREAD_SCHED_POLICY,
                          SYS_BLD_CLI_ADDRUNCONFIG_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          CLI_ADDRUNCONFIG_Task,
                          NULL,
                          &tid)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_SpawnThread fail.\r\n", __FUNCTION__);
    }

    CLI_OM_EXP_ADDRUNCONFIG_SetTaskID(tid);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_ADDRUNCONFIG_Task
 * ------------------------------------------------------------------------
 * PURPOSE  : CLI addrunconfig main task, it will process related event
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CLI_ADDRUNCONFIG_Task(void)
{
   UI32_T  received_events;

    while (TRUE)
    {
        SYSFUN_ReceiveEvent(CLI_OM_EXP_ADDRUNCONFIG_EVENT_EXECUTE_PARTIAL_PROVISION,
                            SYSFUN_EVENT_WAIT_ANY,
                            SYSFUN_TIMEOUT_WAIT_FOREVER,
                            &received_events);

        if (received_events & CLI_OM_EXP_ADDRUNCONFIG_EVENT_EXECUTE_PARTIAL_PROVISION)
        {
            CLI_ADDRUNCONFIG_RunAllPartialProvision();
        }
    }
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_ADDRUNCONFIG_RunAllPartialProvision
 * ------------------------------------------------------------------------
 * PURPOSE  : This API is used to run all partial provision
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void CLI_ADDRUNCONFIG_RunAllPartialProvision()
{
    while (TRUE)
    {
        CLI_OM_EXP_ADDRUNCONFIG_ENTRY_T entry;
        char *buf;

        if (CLI_OM_EXP_ADDRUNCONFIG_SUCCESS != CLI_OM_EXP_ADDRUNCONFIG_GetPendingEntryAndSetToRunning(&entry))
        {
            break;
        }

try_again:
        buf = (char *)BUFFER_MGR_Allocate();

        if (NULL == buf)
        {
            SYSFUN_Sleep(1000);
            goto try_again;
        }

        strncpy(buf, entry.commands, BUFFERMGMT_INIT_BUFFER_SIZE - 1);
        buf[BUFFERMGMT_INIT_BUFFER_SIZE - 1] = '\0';

        if (TRUE != CLI_TASK_CreateTask(CLI_TYPE_PARTIAL_PROV, 0, 0, buf, strlen(buf)))
        {
            BUFFER_MGR_Free(buf);
            SYSFUN_Sleep(1000);
            goto try_again;
        }

        SYSFUN_Sleep(1000);
    }
}
