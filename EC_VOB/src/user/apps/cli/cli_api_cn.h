/* MODULE NAME - CLI_API_CN.H
 * PURPOSE : Provides the declarations for CN CLI APIs.
 * NOTES   : None.
 * HISTORY : 2012/09/12 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2012
 */

#ifndef CLI_API_CN_H
#define CLI_API_CN_H

/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "cli_def.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME - CLI_API_CN
 * PURPOSE : Set CN global status in global configuration mode.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_CN(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


/* FUNCTION NAME - CLI_API_CN_CnmTransmitPriority
 * PURPOSE : Set the priority used for CNM transmission in global configuration
 *           mode.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_CN_CnmTransmitPriority(UI16_T cmd_idx, char *arg[],
                                      CLI_TASK_WorkingArea_T *ctrl_P);

/* FUNCTION NAME - CLI_API_CN_Cnpv
 * PURPOSE : Set per-CNPV parameters in global configuration mode.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_CN_Cnpv(UI16_T cmd_idx, char *arg[],
                       CLI_TASK_WorkingArea_T *ctrl_P);

/* FUNCTION NAME - CLI_API_CN_Cnpv_Eth
 * PURPOSE : Set per-CNPV parameters in ethernet interface configuration mode.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_CN_Cnpv_Eth(UI16_T cmd_idx, char *arg[],
                           CLI_TASK_WorkingArea_T *ctrl_P);

/* FUNCTION NAME - CLI_API_CN_Cnpv_Pch
 * PURPOSE : Set per-CNPV parameters in port channel interface configuration
 *           mode.
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E
 * NOTES   : None
 */
UI32_T CLI_API_CN_Cnpv_Pch(UI16_T cmd_idx, char *arg[],
                           CLI_TASK_WorkingArea_T *ctrl_P);

/* FUNCTION NAME - CLI_API_CN_ShowCn
 * PURPOSE : Show CN global information in privilege execution mode
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES   : None
 */
UI32_T CLI_API_CN_ShowCn(UI16_T cmd_idx, char *arg[],
                         CLI_TASK_WorkingArea_T *ctrl_P);

/* FUNCTION NAME - CLI_API_CN_ShowCnpv
 * PURPOSE : Show per-CNPV information in privilege execution mode
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES   : None
 */
UI32_T CLI_API_CN_ShowCnpv(UI16_T cmd_idx, char *arg[],
                           CLI_TASK_WorkingArea_T *ctrl_P);

/* FUNCTION NAME - CLI_API_CN_ShowCp
 * PURPOSE : Show per-CP information in privilege execution mode
 * INPUT   : cmd_idx
 *           arg
 *           ctrl_P
 * OUTPUT  : None
 * RETURN  : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES   : None
 */
UI32_T CLI_API_CN_ShowCp(UI16_T cmd_idx, char *arg[],
                         CLI_TASK_WorkingArea_T *ctrl_P);

#endif /* End of CLI_API_CN_H */
