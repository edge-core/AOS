/* MODULE NAME - CN_POM.H
 * PURPOSE : Provides the declarations for integrated CN database management.
 * NOTES   : None.
 * HISTORY : 2012/09/12 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2012
 */

#ifndef CN_POM_H
#define CN_POM_H

/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "cn_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME - CN_POM_GetGlobalAdminStatus
 * PURPOSE : Get CN global admin status.
 * INPUT   : None
 * OUTPUT  : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_POM_GetGlobalAdminStatus(UI32_T *status);

/* FUNCTION NAME - CN_POM_GetGlobalOperStatus
 * PURPOSE : Get CN global oper status.
 * INPUT   : None
 * OUTPUT  : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_POM_GetGlobalOperStatus(UI32_T *status);

/* FUNCTION NAME - CN_POM_GetRunningGlobalAdminStatus
 * PURPOSE : Get running CN global admin status.
 * INPUT   : None
 * OUTPUT  : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None
 */
SYS_TYPE_Get_Running_Cfg_T CN_POM_GetRunningGlobalAdminStatus(UI32_T *status);

/* FUNCTION NAME - CN_POM_GetCnmTxPriority
 * PURPOSE : Get the priority used for transmitting CNMs.
 * INPUT   : None
 * OUTPUT  : priority - the priority used for CNM transmission
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_POM_GetCnmTxPriority(UI32_T *priority);

/* FUNCTION NAME - CN_POM_GetRunningCnmTxPriority
 * PURPOSE : Get the running priority used for transmitting CNMs.
 * INPUT   : None
 * OUTPUT  : priority - the priority used for CNM transmission
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None
 */
SYS_TYPE_Get_Running_Cfg_T CN_POM_GetRunningCnmTxPriority(UI32_T *priority);

/* FUNCTION NAME - CN_POM_GetNextCnpv
 * PURPOSE : Set a priority to be CNPV or not.
 * INPUT   : priority - the starting priority for searching
 * OUTPUT  : priority - the CNPV next to the starting priority
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_POM_GetNextCnpv(UI32_T *priority);

/* FUNCTION NAME - CN_POM_GetCnpvDefenseMode
 * PURPOSE : Get the defense mode for a CNPV.
 * INPUT   : priority - the specified CNPV
 * OUTPUT  : mode - CN_TYPE_DEFENSE_MODE_E
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_POM_GetCnpvDefenseMode(UI32_T priority, UI32_T *mode);

/* FUNCTION NAME - CN_POM_GetRunningCnpvDefenseMode
 * PURPOSE : Get the running defense mode for a CNPV.
 * INPUT   : priority - the specified CNPV
 * OUTPUT  : mode - CN_TYPE_DEFENSE_MODE_E
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None
 */
SYS_TYPE_Get_Running_Cfg_T CN_POM_GetRunningCnpvDefenseMode(UI32_T priority, UI32_T *mode);

/* FUNCTION NAME - CN_POM_GetCnpvAlternatePriority
 * PURPOSE : Get the admin alternate priority used for a CNPV.
 * INPUT   : priority - the specified CNPV
 * OUTPUT  : alt_priority - the alternate priority used for the CNPV
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_POM_GetCnpvAlternatePriority(UI32_T priority, UI32_T *alt_priority);

/* FUNCTION NAME - CN_POM_GetRunningCnpvAlternatePriority
 * PURPOSE : Get the running admin alternate priority used for a CNPV.
 * INPUT   : priority - the specified CNPV
 * OUTPUT  : alt_priority - the alternate priority used for the CNPV
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None
 */
SYS_TYPE_Get_Running_Cfg_T CN_POM_GetRunningCnpvAlternatePriority(UI32_T priority, UI32_T *alt_priority);

/* FUNCTION NAME - CN_POM_GetPortCnpvDefenseMode
 * PURPOSE : Get the admin defense mode for a CNPV on a port.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : mode - CN_TYPE_DEFENSE_MODE_E
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_POM_GetPortCnpvDefenseMode(UI32_T priority, UI32_T lport, UI32_T *mode);

/* FUNCTION NAME - CN_POM_GetRunningPortCnpvDefenseMode
 * PURPOSE : Get the running admin defense mode for a CNPV on a port.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : mode - CN_TYPE_DEFENSE_MODE_E
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None
 */
SYS_TYPE_Get_Running_Cfg_T CN_POM_GetRunningPortCnpvDefenseMode(UI32_T priority, UI32_T lport, UI32_T *mode);

/* FUNCTION NAME - CN_POM_GetPortCnpvAlternatePriority
 * PURPOSE : Get the admin alternate priority used for a CNPV on a port.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : alt_priority - the alternate priority for the CNPV on the logical port
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_POM_GetPortCnpvAlternatePriority(UI32_T priority, UI32_T lport, UI32_T *alt_priority);

/* FUNCTION NAME - CN_POM_GetRunningPortCnpvAlternatePriority
 * PURPOSE : Get the running admin alternate priority used for a CNPV on a port.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : alt_priority - the alternate priority for the CNPV on the logical port
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None
 */
SYS_TYPE_Get_Running_Cfg_T CN_POM_GetRunningPortCnpvAlternatePriority(UI32_T priority, UI32_T lport, UI32_T *alt_priority);

/* FUNCTION NAME - CN_POM_GetTxReady
 * PURPOSE : Get the ready value for CN TLV Ready Indicators.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : ready - whether priority remap defense has been disabled
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : IEEE 802.1Qau-2010 32.4.8
 */
UI32_T CN_POM_GetTxReady(UI32_T priority, UI32_T lport, BOOL_T *ready);

/* FUNCTION NAME - CN_POM_GetGlobalEntry
 * PURPOSE : Get the global entry.
 * INPUT   : None
 * OUTPUT  : entry - buffer to contain the global entry
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_POM_GetGlobalEntry(CN_TYPE_GlobalEntry_T *entry);

/* FUNCTION NAME - CN_POM_GetCnpvEntry
 * PURPOSE : Get an entry for a CNPV.
 * INPUT   : entry.cnpv - the specified CNPV
 * OUTPUT  : entry - buffer to contain the cnpv entry
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : Only active entry is returned
 */
UI32_T CN_POM_GetCnpvEntry(CN_TYPE_CnpvEntry_T *entry);

/* FUNCTION NAME - CN_POM_GetNextCnpvEntry
 * PURPOSE : Get the next entry for a CNPV.
 * INPUT   : entry.cnpv - the specified CNPV
 * OUTPUT  : entry - buffer to contain the next cnpv entry
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_POM_GetNextCnpvEntry(CN_TYPE_CnpvEntry_T *entry);

/* FUNCTION NAME - CN_OM_GetPortCnpvEntry
 * PURPOSE : Get an entry for a CNPV on a port.
 * INPUT   : entry.cnpv  - the specified CNPV
 *           entry.lport - the specified logical port
 * OUTPUT  : entry - buffer to contain the port cnpv entry
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : Only active entry is returned
 */
UI32_T CN_POM_GetPortCnpvEntry(CN_TYPE_PortCnpvEntry_T *entry);

/* FUNCTION NAME - CN_OM_GetNextPortCnpvEntry
 * PURPOSE : Get the next entry for a CNPV on a port.
 * INPUT   : entry.cnpv  - the specified CNPV
 *           entry.lport - the specified logical port
 * OUTPUT  : entry - buffer to contain the next port cnpv entry
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : Next lport first, then next CNPV; but if inputed entry.cnpv
 *           is not a CNPV, next CNPV first
 */
UI32_T CN_POM_GetNextPortCnpvEntry(CN_TYPE_PortCnpvEntry_T *entry);

/* FUNCTION NAME - CN_POM_GetCpEntry
 * PURPOSE : Get an entry for a CP.
 * INPUT   : entry.lport    - the specified logical port number
 *           entry.cp_index - the specified CP index
 * OUTPUT  : entry - buffer to contain the cp entry
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : Only active entry is returned
 */
UI32_T CN_POM_GetCpEntry(CN_TYPE_CpEntry_T *entry);

/* FUNCTION NAME - CN_POM_GetNextCpEntry
 * PURPOSE : Get the next entry for a CP.
 * INPUT   : entry.lport    - the specified logical port number
 *           entry.cp_index - the specified CP index
 * OUTPUT  : entry - buffer to contain the next cp entry
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : Next cp index first, then next lport
 */
UI32_T CN_POM_GetNextCpEntry(CN_TYPE_CpEntry_T *entry);

/* FUNCTION NAME - CN_POM_IsCnpv
 * PURPOSE : Check whether a priority is a CNPV.
 * INPUT   : priroity - the priority to be checked
 * OUTPUT  : None
 * RETURN  : TRUE  - the priority is a CNPV
 *           FALSE - the priority is not a CNPV
 * NOTES   : None
 */
BOOL_T CN_POM_IsCnpv(UI32_T priority);

/* FUNCTION NAME - CN_POM_IsCp
 * PURPOSE : Check whether it is a CP given port and index.
 * INPUT   : lport - the lport where the CP is located
 *           index - the index of the CP
 * OUTPUT  : None
 * RETURN  : TRUE  - it is a CP
 *           FALSE - it is not a CP
 * NOTES   : None
 */
BOOL_T CN_POM_IsCp(UI32_T lport, UI32_T index);

#endif /* End of CN_POM_H */
