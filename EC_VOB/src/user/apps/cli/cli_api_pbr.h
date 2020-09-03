/* MODULE NAME: cli_api_pbr.h
 * PURPOSE:
 *   Definitions of CLI APIs for PBR.
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   2015/07/10     KH Shi, Create
 *
 * Copyright(C)      Accton Corporation, 2015
 */

#ifndef _CLI_API_PBR_H
#define _CLI_API_PBR_H

 /* INCLUDE FILE DECLARATIONS
  */
#include "sys_type.h"

UI32_T CLI_API_PBR_IpPolicyRouteMap(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
UI32_T CLI_API_PBR_ShowIpPolicy(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);

#endif



