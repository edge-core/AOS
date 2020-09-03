/* MODULE NAME - CN_OM.H
 * PURPOSE : Provides the declarations for CN database management.
 * NOTES   : None.
 * HISTORY : 2012/09/12 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2012
 */

#ifndef CN_OM_H
#define CN_OM_H

/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "sysrsc_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    UI32_T  admin_status;
    UI32_T  oper_status;
    UI32_T  cnm_tx_priority;
} CN_OM_GlobalData_T;

typedef struct
{
    BOOL_T  active;
    UI32_T  defense_mode;
    UI32_T  admin_alt_priority;
    UI32_T  auto_alt_priority;
} CN_OM_PriData_T;

typedef struct
{
    BOOL_T  active;
    UI32_T  admin_defense_mode;
    UI32_T  oper_defense_mode;
    UI32_T  admin_alt_priority;
    UI32_T  oper_alt_priority;
    BOOL_T  tx_ready;
} CN_OM_PortPriData_T;

typedef struct
{
    BOOL_T  active;
    UI8_T   queue;
    UI8_T   managed_cnpvs;
    UI32_T  set_point;
    UI8_T   feedback_weight;
    UI32_T  min_sample_base;
} CN_OM_CpData_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME - CN_OM_InitiateSystemResources
 * PURPOSE : Initiate system resources.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_OM_InitiateSystemResources(void);

/* FUNCTION NAME - CN_OM_AttachSystemResources
 * PURPOSE : Attach system resources for CN in the context of the calling
 *           process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_OM_AttachSystemResources(void);

/* FUNCTION NAME - CN_OM_GetShMemInfo
 * PURPOSE : Provide shared memory information of CN for SYSRSC_MGR.
 * INPUT   : None
 * OUTPUT  : segid_p  - shared memory segment id
 *           seglen_p - length of the shared memroy segment
 * RETURN  : None
 * NOTES   : This function is called in SYSRSC_MGR_CreateSharedMemory().
 */
void CN_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

/* FUNCTION NAME - CN_OM_GetGlobalAdminStatus
 * PURPOSE : Get CN global admin status.
 * INPUT   : None
 * OUTPUT  : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_GetGlobalAdminStatus(UI32_T *status);

/* FUNCTION NAME - CN_OM_GetGlobalOperStatus
 * PURPOSE : Get CN global oper status.
 * INPUT   : None
 * OUTPUT  : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_GetGlobalOperStatus(UI32_T *status);

/* FUNCTION NAME - CN_OM_GetRunningGlobalAdminStatus
 * PURPOSE : Get running CN global admin status.
 * INPUT   : None
 * OUTPUT  : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None
 */
SYS_TYPE_Get_Running_Cfg_T CN_OM_GetRunningGlobalAdminStatus(UI32_T *status);

/* FUNCTION NAME - CN_OM_GetCnmTxPriority
 * PURPOSE : Get the priority used for transmitting CNMs.
 * INPUT   : None
 * OUTPUT  : priority - the priority used for CNM transmission
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_GetCnmTxPriority(UI32_T *priority);

/* FUNCTION NAME - CN_OM_GetRunningCnmTxPriority
 * PURPOSE : Get the running priority used for transmitting CNMs.
 * INPUT   : None
 * OUTPUT  : priority - the priority used for CNM transmission
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None
 */
SYS_TYPE_Get_Running_Cfg_T CN_OM_GetRunningCnmTxPriority(UI32_T *priority);

/* FUNCTION NAME - CN_OM_GetNextCnpv
 * PURPOSE : Get the next CNPV to a given priority.
 * INPUT   : priority - the starting priority for searching
 * OUTPUT  : priority - the CNPV next to the given priority
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_GetNextCnpv(UI32_T *priority);

/* FUNCTION NAME - CN_OM_GetPriDefenseMode
 * PURPOSE : Get the defense mode for a priority.
 * INPUT   : priority - the specified priority
 * OUTPUT  : mode - CN_TYPE_DEFENSE_MODE_E
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_GetPriDefenseMode(UI32_T priority, UI32_T *mode);

/* FUNCTION NAME - CN_OM_GetRunningPriDefenseMode
 * PURPOSE : Get the running defense mode for a priority.
 * INPUT   : priority - the specified CNPV
 * OUTPUT  : mode - CN_TYPE_DEFENSE_MODE_E
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None
 */
SYS_TYPE_Get_Running_Cfg_T CN_OM_GetRunningPriDefenseMode(UI32_T priority,
                                UI32_T *mode);

/* FUNCTION NAME - CN_OM_GetPriAlternatePriority
 * PURPOSE : Get the admin alternate priority used for a priority.
 * INPUT   : priority - the specified priority
 * OUTPUT  : alt_priority - the alternate priority for a priority
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_GetPriAlternatePriority(UI32_T priority, UI32_T *alt_priority);

/* FUNCTION NAME - CN_OM_GetRunningPriAlternatePriority
 * PURPOSE : Get the running admin alternate priority used for a priority.
 * INPUT   : priority - the specified priority
 * OUTPUT  : alt_priority - the alternate priority for a priority
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None
 */
SYS_TYPE_Get_Running_Cfg_T CN_OM_GetRunningPriAlternatePriority(UI32_T priority,
                                UI32_T *alt_priority);

/* FUNCTION NAME - CN_OM_GetPortPriDefenseMode
 * PURPOSE : Get the admin defense mode for a priority on a port.
 * INPUT   : priority - the specified priority
 *           lport    - the specified logical port
 * OUTPUT  : mode - CN_TYPE_DEFENSE_MODE_E
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_GetPortPriDefenseMode(UI32_T priority, UI32_T lport, UI32_T *mode);

/* FUNCTION NAME - CN_OM_GetRunningPortPriDefenseMode
 * PURPOSE : Get the running admin defense mode for a priority on a port.
 * INPUT   : priority - the specified priority
 *           lport    - the specified logical port
 * OUTPUT  : mode - CN_TYPE_DEFENSE_MODE_E
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None
 */
SYS_TYPE_Get_Running_Cfg_T CN_OM_GetRunningPortPriDefenseMode(UI32_T priority,
                            UI32_T lport, UI32_T *mode);

/* FUNCTION NAME - CN_OM_GetPortPriAlternatePriority
 * PURPOSE : Get the admin alternate priority used for a priority on a port.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : alt_priority - the alternate priority for a priority on a port
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_OM_GetPortPriAlternatePriority(UI32_T priority, UI32_T lport,
        UI32_T *alt_priority);

/* FUNCTION NAME - CN_OM_GetRunningPortPriAlternatePriority
 * PURPOSE : Get the running admin alternate priority used for a priority on a
 *           port.
 * INPUT   : priority - the specified priority
 *           lport    - the specified logical port
 * OUTPUT  : alt_priority - the alternate priority for a priority on a port
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None
 */
SYS_TYPE_Get_Running_Cfg_T CN_OM_GetRunningPortPriAlternatePriority(
                            UI32_T priority, UI32_T lport, UI32_T *alt_priority);

/* FUNCTION NAME - CN_OM_GetTxReady
 * PURPOSE : Get the ready value for CN TLV Ready Indicators.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : ready - whether priority remap defense has been disabled
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : IEEE 802.1Qau-2010 32.4.8
 */
UI32_T CN_OM_GetTxReady(UI32_T priority, UI32_T lport, BOOL_T *ready);

/* FUNCTION NAME - CN_OM_IsCnpv
 * PURPOSE : Check whether a priority is a CNPV.
 * INPUT   : priority - the priority to be checked
 * OUTPUT  : None
 * RETURN  : TRUE  - the priority is a CNPV
 *           FALSE - the priority is not a CNPV
 * NOTES   : None
 */
BOOL_T CN_OM_IsCnpv(UI32_T priority);

/* FUNCTION NAME - CN_OM_IsCp
 * PURPOSE : Check whether it is a CP given port and index.
 * INPUT   : lport - the lport where the CP is located
 *           index - the index of the CP
 * OUTPUT  : None
 * RETURN  : TRUE  - it is a CP
 *           FALSE - it is not a CP
 * NOTES   : None
 */
BOOL_T CN_OM_IsCp(UI32_T lport, UI32_T index);

/* FUNCTION NAME - CN_OM_GetGlobalData
 * PURPOSE : Get the global data.
 * INPUT   : None
 * OUTPUT  : buffer - buffer to contain the global data
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : Used by CN backdoor
 */
UI32_T CN_OM_GetGlobalData(CN_OM_GlobalData_T *buffer);

/* FUNCTION NAME - CN_OM_GetPriData
 * PURPOSE : Get data for a priority.
 * INPUT   : priority - the specified priority
 * OUTPUT  : buffer - buffer to contain the data for a priority
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : Used by CN backdoor
 */
UI32_T CN_OM_GetPriData(UI32_T priority, CN_OM_PriData_T *buffer);

/* FUNCTION NAME - CN_OM_GetPortPriData
 * PURPOSE : Get the data for a priority on a port.
 * INPUT   : priority - the specified priority
 *           lport    - the specified logical port
 * OUTPUT  : buffer - buffer to contain the data for a priority on a port
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : Used by CN backdoor
 */
UI32_T CN_OM_GetPortPriData(UI32_T priority, UI32_T lport,
        CN_OM_PortPriData_T *buffer);

/* FUNCTION NAME - CN_OM_GetCpData
 * PURPOSE : Get the data for a CP.
 * INPUT   : lport    - the specified logical port number
 *           cp_index - the specified CP index
 * OUTPUT  : buffer - buffer to contain the data for a CP
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : Used by CN backdoor
 */
UI32_T CN_OM_GetCpData(UI32_T lport, UI32_T cp_index, CN_OM_CpData_T *buffer);

#endif /* End of CN_OM_H */
