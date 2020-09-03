/* MODULE NAME: cli_api_udld.c
 * PURPOSE:
 *   Definitions of CLI APIs for UDLD.
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   06/09/11    -- Squid Ro, Create
 *
 * Copyright(C)      Accton Corporation, 2011
 */

 /* INCLUDE FILE DECLARATIONS
  */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "cli_api.h"
#include "cli_lib.h"

#include "cli_api_udld.h"
#include "swctrl.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTIONS DECLARACTION
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* command: [no] udld message-interval
 */
UI32_T CLI_API_Udld_MessageInterval(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

    return CLI_NO_ERROR;
}

/* command: [no] udld detection-interval
 */
UI32_T CLI_API_Udld_DetectionInterval(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

    return CLI_NO_ERROR;
}

/* command: [no] udld recovery-interval
 */
UI32_T CLI_API_Udld_RecoveryInterval(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

    return CLI_NO_ERROR;
}

/* command: [no] udld recovery
 */
UI32_T CLI_API_Udld_Recovery(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

    return CLI_NO_ERROR;
}

/* command: [no] udld aggressive
 */
UI32_T CLI_API_Udld_Aggressive_Eth(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

    return CLI_NO_ERROR;
}

/* command: [no] udld port
 */
UI32_T CLI_API_Udld_Port_Eth(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

    return CLI_NO_ERROR;
}


/* command: show udld [interface ethernet port]
 */
UI32_T CLI_API_Udld_Show(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

    return CLI_NO_ERROR;
}

