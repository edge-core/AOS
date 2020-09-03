/* =============================================================================
 * MODULE NAME : CLI_API_MLAG.H
 * PURPOSE     : Provide declarations for MLAG command line functions.
 * NOTE        : None.
 * HISTORY     :
 *     2014/04/18 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2014
 * =============================================================================
 */

#ifndef CLI_API_MLAG_H
#define CLI_API_MLAG_H

/* -----------------------------------------------------------------------------
 * INCLUDE FILE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

#include "sys_type.h"
#include "cli_def.h"

/* -----------------------------------------------------------------------------
 * NAMING CONSTANT DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * MACRO FUNCTION DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * DATA TYPE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * EXPORTED SUBPROGRAM SPECIFICATIONS
 * -----------------------------------------------------------------------------
 */

/* FUNCTION NAME - CLI_API_MLAG_Global
 * PURPOSE : Configure MLAG global status in global configuration mode
 * INPUT   : cmd_idx, arg, ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E (cli_msg.h)
 * NOTE    : None
 */
UI32_T CLI_API_MLAG_Global(UI16_T cmd_idx, char *arg[],
        CLI_TASK_WorkingArea_T *ctrl_P);

/* FUNCTION NAME - CLI_API_MLAG_Domain
 * PURPOSE : Configure a domain in global configuration mode
 * INPUT   : cmd_idx, arg, ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E (cli_msg.h)
 * NOTE    : None
 */
UI32_T CLI_API_MLAG_Domain(UI16_T cmd_idx, char *arg[],
        CLI_TASK_WorkingArea_T *ctrl_P);

/* FUNCTION NAME - CLI_API_MLAG_Group
 * PURPOSE : Configure a MLAG in global configuration mode
 * INPUT   : cmd_idx, arg, ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E (cli_msg.h)
 * NOTE    : None
 */
UI32_T CLI_API_MLAG_Group(UI16_T cmd_idx, char *arg[],
        CLI_TASK_WorkingArea_T *ctrl_P);

/* FUNCTION NAME - CLI_API_MLAG_ShowGlobal
 * PURPOSE : Show MLAG global information in privileged execution mode
 * INPUT   : cmd_idx, arg, ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E (cli_msg.h)
 * NOTES   : None
 */
UI32_T CLI_API_MLAG_ShowGlobal(UI16_T cmd_idx, char *arg[],
        CLI_TASK_WorkingArea_T *ctrl_P);

/* FUNCTION NAME - CLI_API_MLAG_ShowDomain
 * PURPOSE : Show MLAG per domain information in privileged execution mode
 * INPUT   : cmd_idx, arg, ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E (cli_msg.h)
 * NOTES   : None
 */
UI32_T CLI_API_MLAG_ShowDomain(UI16_T cmd_idx, char *arg[],
        CLI_TASK_WorkingArea_T *ctrl_P);

/* FUNCTION NAME - CLI_API_MLAG_ShowGroup
 * PURPOSE : Show MLAG per group information in privileged execution mode
 * INPUT   : cmd_idx, arg, ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E (cli_msg.h)
 * NOTES   : None
 */
UI32_T CLI_API_MLAG_ShowGroup(UI16_T cmd_idx, char *arg[],
        CLI_TASK_WorkingArea_T *ctrl_P);

#endif /* End of CLI_API_MLAG_H */
