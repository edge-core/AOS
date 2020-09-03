/* MODULE NAME - CN_PMGR.H
 * PURPOSE : Provides the declarations for CN IPC functional management.
 * NOTES   : None.
 * HISTORY : 2012/09/12 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2012
 */

#ifndef CN_PMGR_H
#define CN_PMGR_H

/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME - CN_PMGR_InitiateProcessResources
 * PURPOSE : Initiate resources for CN_PMGR in the calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTES   : None
 */
BOOL_T CN_PMGR_InitiateProcessResources(void);

/* FUNCTION NAME - CN_PMGR_SetGlobalAdminStatus
 * PURPOSE : Set CN global admin status.
 * INPUT   : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_PMGR_SetGlobalAdminStatus(UI32_T status);

/* FUNCTION NAME - CN_PMGR_SetCnmTxPriority
 * PURPOSE : Set the priority used for transmitting CNMs.
 * INPUT   : priority - priority in the range between 0 and 7
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_PMGR_SetCnmTxPriority(UI32_T priority);

/* FUNCTION NAME - CN_PMGR_SetCnpv
 * PURPOSE : Set a priority to be CNPV or not.
 * INPUT   : priority - the specify priority
 *           active   - TRUE/FALSE
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_PMGR_SetCnpv(UI32_T priority, BOOL_T active);

/* FUNCTION NAME - CN_PMGR_SetCnpvDefenseMode
 * PURPOSE : Set the defense mode for a CNPV.
 * INPUT   : priority - the specified CNPV
 *           mode     - CN_TYPE_DEFENSE_MODE_E
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_PMGR_SetCnpvDefenseMode(UI32_T priority, UI32_T mode);

/* FUNCTION NAME - CN_PMGR_SetCnpvAlternatePriority
 * PURPOSE : Set the alternate priority used for a CNPV.
 * INPUT   : priority     - the specified CNPV
 *           alt_priority - the specified alternate priority
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_PMGR_SetCnpvAlternatePriority(UI32_T priority, UI32_T alt_priority);

/* FUNCTION NAME - CN_PMGR_SetPortCnpvDefenseMode
 * PURPOSE : Set the defense mode for a CNPV on a port.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 *           mode     - CN_TYPE_DEFENSE_MODE_E
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_PMGR_SetPortCnpvDefenseMode(UI32_T priority, UI32_T lport, UI32_T mode);

/* FUNCTION NAME - CN_PMGR_SetPortCnpvAlternatePriority
 * PURPOSE : Set the alternate priority used for a CNPV on a port.
 * INPUT   : priority     - the specified CNPV
 *           lport        - the specified logical port
 *           alt_priority - the specified alternate priority
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_PMGR_SetPortCnpvAlternatePriority(UI32_T priority, UI32_T lport, UI32_T alt_priority);

#endif /* End of CN_PMGR_H */
