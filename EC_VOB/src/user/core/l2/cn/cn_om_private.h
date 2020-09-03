/* MODULE NAME - CN_OM_PRIVATE.H
 * PURPOSE : Provides the declarations for CN private database management.
 * NOTES   : None.
 * HISTORY : 2012/09/12 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2012
 */

#ifndef CN_OM_PRIVATE_H
#define CN_OM_PRIVATE_H

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

/* FUNCTION NAME - CN_OM_ResetAll
 * PURPOSE : Reset all CN data.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_OM_ResetAll(void);

/* FUNCTION NAME - CN_OM_DefaultAll
 * PURPOSE : Default all CN data.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_OM_DefaultAll(void);

/* FUNCTION NAME - CN_OM_DefaultPort
 * PURPOSE : Default all port-pri data and cp data for a logical port.
 * INPUT   : lport - the specified logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_OM_DefaultPort(UI32_T lport);

/* FUNCTION NAME - CN_OM_SetGlobalAdminStatus
 * PURPOSE : Set CN global admin status.
 * INPUT   : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetGlobalAdminStatus(UI32_T status);

/* FUNCTION NAME - CN_OM_SetGlobalOperStatus
 * PURPOSE : Set CN global oper status.
 * INPUT   : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetGlobalOperStatus(UI32_T status);

/* FUNCTION NAME - CN_OM_SetCnmTxPriority
 * PURPOSE : Set the priority used for transmitting CNMs.
 * INPUT   : priority - priority in the range between 0 and 7
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetCnmTxPriority(UI32_T priority);

/* FUNCTION NAME - CN_OM_SetPriStatus
 * PURPOSE : Set a priority to be CNPV or not.
 * INPUT   : priority - the specify priority
 *           active   - TRUE/FALSE
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetPriStatus(UI32_T priority, BOOL_T active);

/* FUNCTION NAME - CN_OM_SetPriDefenseMode
 * PURPOSE : Set the defense mode for a priority.
 * INPUT   : priority - the specify priority
 *           mode     - CN_TYPE_DEFENSE_MODE_E
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetPriDefenseMode(UI32_T priority, UI32_T mode);

/* FUNCTION NAME - CN_OM_SetPriAlternatePriority
 * PURPOSE : Set the alternate priority used for a priority..
 * INPUT   : priority     - the specify priority
 *           alt_priority - the alternate priority for a priority
 *           is_admin - whether it is admin or auto
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetPriAlternatePriority(UI32_T priority, UI32_T alt_priority,
            BOOL_T is_admin);

/* FUNCTION NAME - CN_OM_SetPortPriStatus
 * PURPOSE : Set a priority to be CNPV or not on a port.
 * INPUT   : priority - the specify priority
 *           lport    - the specified logical port
 *           active   - TRUE/FALSE
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetPortPriStatus(UI32_T priority, UI32_T lport, BOOL_T active);

/* FUNCTION NAME - CN_OM_SetPortPriDefenseMode
 * PURPOSE : Set the defense mode for a priority on a port.
 * INPUT   : priority - the specify priority
 *           lport    - the specified logical port
 *           mode     - CN_TYPE_DEFENSE_MODE_E
 *           is_admin - whether it is admin or oper
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetPortPriDefenseMode(UI32_T priority, UI32_T lport, UI32_T mode,
        BOOL_T is_admin);

/* FUNCTION NAME - CN_OM_SetPortPriAlternatePriority
 * PURPOSE : Set the alternate priority used for a priority on a port.
 * INPUT   : priority     - the specify priority
 *           lport        - the specified logical port
 *           alt_priority - the specified alternate priority
 *           is_admin     - whether it is admin or oper
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetPortPriAlternatePriority(UI32_T priority, UI32_T lport,
        UI32_T alt_priority, BOOL_T is_admin);

/* FUNCTION NAME - CN_OM_SetTxReady
 * PURPOSE : Set the ready value for CN TLV Ready Indicators.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 *           ready    - whether priority remap defense has been disabled
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : IEEE 802.1Qau-2010 32.4.8
 */
UI32_T CN_OM_SetTxReady(UI32_T priority, UI32_T lport, BOOL_T ready);

/* FUNCTION NAME - CN_OM_SetCpStatus
 * PURPOSE : Set the status of a CP.
 * INPUT   : lport  - the specified logical port
 *           index  - the index of the CP
 *           active - to be active or not
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetCpStatus(UI32_T lport, UI32_T index, BOOL_T active);

/* FUNCTION NAME - CN_OM_SetCpMappedQueue
 * PURPOSE : Set the mapped queue for a CP.
 * INPUT   : lport - the specified logical port
 *           index - the index of the CP
 *           queue - queue value to be mapped with the CP
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetCpMappedQueue(UI32_T lport, UI32_T index, UI8_T queue);

/* FUNCTION NAME - CN_OM_SetCpManagedCnpvs
 * PURPOSE : Set the bitmap of CNPVs managed by a CP.
 * INPUT   : lport - the specified logical port
 *           index - the index of the CP
 *           cnpvs - the bitmap of CNPVs managed by a CP
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_SetCpManagedCnpvs(UI32_T lport, UI32_T index, UI8_T cnpvs);

/* FUNCTION NAME - CN_OM_GetGlobalDataPtr
 * PURPOSE : Get a pointer to the global data.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : the pointer to the global data
 * NOTES   : Used by CN backdoor
 */
CN_OM_GlobalData_T* CN_OM_GetGlobalDataPtr(void);

/* FUNCTION NAME - CN_OM_GetPriDataPtr
 * PURPOSE : Get a pointer to the data for a priority.
 * INPUT   : priority - the specified priority
 * OUTPUT  : None
 * RETURN  : the pointer to the data for a priority
 * NOTES   : None
 */
CN_OM_PriData_T* CN_OM_GetPriDataPtr(UI32_T priority);

/* FUNCTION NAME - CN_OM_GetPortPriDataPtr
 * PURPOSE : Get a pointer to the data for a priority on a port.
 * INPUT   : priority - the specified priority
 *           lport    - the specified logical port
 * OUTPUT  : None
 * RETURN  : the pointer to the data for a priority on a port
 * NOTES   : None
 */
CN_OM_PortPriData_T* CN_OM_GetPortPriDataPtr(UI32_T priority, UI32_T lport);

/* FUNCTION NAME - CN_OM_GetCpDataPtr
 * PURPOSE : Get a pointer to an entry for the specified CP on a port.
 * INPUT   : lport    - the specified logical port
 *           cp_index - the specified CP index
 * OUTPUT  : None
 * RETURN  : the pointer to the data for a CP
 * NOTES   : None
 */
CN_OM_CpData_T* CN_OM_GetCpDataPtr(UI32_T lport, UI32_T cp_index);

void CN_OM_SetErrDebug(BOOL_T flag);
BOOL_T CN_OM_ErrDebug(void);
char* CN_OM_ErrStr(UI32_T ret_code);
void CN_OM_SetThreadDebug(BOOL_T flag);
BOOL_T CN_OM_ThreadDebug(void);

#endif /* End of CN_OM_PRIVATE_H */
