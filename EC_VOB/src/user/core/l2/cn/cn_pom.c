/* MODULE NAME - CN_POM.C
 * PURPOSE : Provides the definitions for integrated CN database management.
 * NOTES   : None.
 * HISTORY : 2012/09/12 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2012
 */

/* INCLUDE FILE DECLARATIONS
 */

#include <string.h>
#include "sys_adpt.h"
#include "sys_type.h"
#include "cn_om.h"
#include "cn_pom.h"
#include "cn_type.h"
#include "swctrl_pom.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DEFINITIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME - CN_POM_GetGlobalAdminStatus
 * PURPOSE : Get CN global admin status.
 * INPUT   : None
 * OUTPUT  : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_POM_GetGlobalAdminStatus(UI32_T *status)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return CN_OM_GetGlobalAdminStatus(status);
} /* End of CN_POM_GetGlobalAdminStatus */

/* FUNCTION NAME - CN_POM_GetGlobalOperStatus
 * PURPOSE : Get CN global oper status.
 * INPUT   : None
 * OUTPUT  : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_POM_GetGlobalOperStatus(UI32_T *status)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return CN_OM_GetGlobalOperStatus(status);
} /* End of CN_POM_GetGlobalOperStatus */

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
SYS_TYPE_Get_Running_Cfg_T CN_POM_GetRunningGlobalAdminStatus(UI32_T *status)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return CN_OM_GetRunningGlobalAdminStatus(status);
} /* End of CN_POM_GetRunningGlobalAdminStatus */

/* FUNCTION NAME - CN_POM_GetCnmTxPriority
 * PURPOSE : Get the priority used for transmitting CNMs.
 * INPUT   : None
 * OUTPUT  : priority - the priority used for CNM transmission
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_POM_GetCnmTxPriority(UI32_T *priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return CN_OM_GetCnmTxPriority(priority);
} /* End of CN_POM_GetCnmTxPriority */

/* FUNCTION NAME - CN_POM_GetRunningCnmTxPriority
 * PURPOSE : Get the running priority used for transmitting CNMs.
 * INPUT   : None
 * OUTPUT  : priority - the priority used for CNM transmission
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None
 */
SYS_TYPE_Get_Running_Cfg_T CN_POM_GetRunningCnmTxPriority(UI32_T *priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return CN_OM_GetRunningCnmTxPriority(priority);
} /* End of CN_POM_GetRunningCnmTxPriority */

/* FUNCTION NAME - CN_POM_GetNextCnpv
 * PURPOSE : Set a priority to be CNPV or not.
 * INPUT   : priority - the starting priority for searching
 * OUTPUT  : priority - the CNPV next to the starting priority
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_POM_GetNextCnpv(UI32_T *priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return CN_OM_GetNextCnpv(priority);
} /* End of CN_POM_GetNextCnpv */

/* FUNCTION NAME - CN_POM_GetCnpvDefenseMode
 * PURPOSE : Get the defense mode for a CNPV.
 * INPUT   : priority - the specified CNPV
 * OUTPUT  : mode - CN_TYPE_DEFENSE_MODE_E
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_POM_GetCnpvDefenseMode(UI32_T priority, UI32_T *mode)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return CN_OM_GetPriDefenseMode(priority, mode);
} /* End of CN_POM_GetCnpvDefenseMode */

/* FUNCTION NAME - CN_POM_GetRunningCnpvDefenseMode
 * PURPOSE : Get the running defense mode for a CNPV.
 * INPUT   : priority - the specified CNPV
 * OUTPUT  : mode - CN_TYPE_DEFENSE_MODE_E
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None
 */
SYS_TYPE_Get_Running_Cfg_T CN_POM_GetRunningCnpvDefenseMode(UI32_T priority, UI32_T *mode)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return CN_OM_GetRunningPriDefenseMode(priority, mode);
} /* End of CN_POM_GetRunningCnpvDefenseMode */

/* FUNCTION NAME - CN_POM_GetCnpvAlternatePriority
 * PURPOSE : Get the admin alternate priority used for a CNPV.
 * INPUT   : priority - the specified CNPV
 * OUTPUT  : alt_priority - the alternate priority used for the CNPV
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_POM_GetCnpvAlternatePriority(UI32_T priority, UI32_T *alt_priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return CN_OM_GetPriAlternatePriority(priority, alt_priority);
} /* End of CN_POM_IsCnpv */

/* FUNCTION NAME - CN_POM_GetRunningCnpvAlternatePriority
 * PURPOSE : Get the running admin alternate priority used for a CNPV.
 * INPUT   : priority - the specified CNPV
 * OUTPUT  : alt_priority - the alternate priority used for the CNPV
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None
 */
SYS_TYPE_Get_Running_Cfg_T CN_POM_GetRunningCnpvAlternatePriority(UI32_T priority, UI32_T *alt_priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return CN_OM_GetRunningPriAlternatePriority(priority, alt_priority);
} /* End of CN_POM_GetRunningCnpvAlternatePriority */

/* FUNCTION NAME - CN_POM_GetPortCnpvDefenseMode
 * PURPOSE : Get the admin defense mode for a CNPV on a port.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : mode - CN_TYPE_DEFENSE_MODE_E
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_POM_GetPortCnpvDefenseMode(UI32_T priority, UI32_T lport, UI32_T *mode)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return CN_OM_GetPortPriDefenseMode(priority, lport, mode);
} /* End of CN_POM_GetPortCnpvDefenseMode */

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
SYS_TYPE_Get_Running_Cfg_T CN_POM_GetRunningPortCnpvDefenseMode(UI32_T priority, UI32_T lport, UI32_T *mode)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return CN_OM_GetRunningPortPriDefenseMode(priority, lport, mode);
} /* End of CN_POM_GetRunningPortCnpvDefenseMode */

/* FUNCTION NAME - CN_POM_GetPortCnpvAlternatePriority
 * PURPOSE : Get the admin alternate priority used for a CNPV on a port.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : alt_priority - the alternate priority for the CNPV on the logical port
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_POM_GetPortCnpvAlternatePriority(UI32_T priority, UI32_T lport, UI32_T *alt_priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return CN_OM_GetPortPriAlternatePriority(priority, lport, alt_priority);
} /* End of CN_POM_GetPortCnpvAlternatePriority */

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
SYS_TYPE_Get_Running_Cfg_T CN_POM_GetRunningPortCnpvAlternatePriority(UI32_T priority, UI32_T lport, UI32_T *alt_priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return CN_OM_GetRunningPortPriAlternatePriority(priority, lport, alt_priority);
} /* End of CN_POM_GetRunningPortCnpvAlternatePriority */

/* FUNCTION NAME - CN_POM_GetTxReady
 * PURPOSE : Get the ready value for CN TLV Ready Indicators.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : ready - whether priority remap defense has been disabled
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : IEEE 802.1Qau-2010 32.4.8
 */
UI32_T CN_POM_GetTxReady(UI32_T priority, UI32_T lport, BOOL_T *ready)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return CN_OM_GetTxReady(priority, lport, ready);
} /* End of CN_POM_GetTxReady */

/* FUNCTION NAME - CN_POM_GetGlobalEntry
 * PURPOSE : Get the global entry.
 * INPUT   : None
 * OUTPUT  : entry - buffer to contain the global entry
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_POM_GetGlobalEntry(CN_TYPE_GlobalEntry_T *entry)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_GlobalData_T      global_data;
    UI32_T                  result;

    /* BODY
     */

    if (entry == NULL)
    {
        return CN_TYPE_RETURN_ERROR_NULL_POINTER;
    }

    result = CN_OM_GetGlobalData(&global_data);
    if (result != CN_TYPE_RETURN_OK)
    {
        return result;
    }

    entry->admin_status     = global_data.admin_status;
    entry->oper_status      = global_data.oper_status;
    entry->cnm_tx_priority  = global_data.cnm_tx_priority;

    return CN_TYPE_RETURN_OK;
} /* End of CN_POM_GetGlobalEntry */

/* FUNCTION NAME - CN_POM_GetCnpvEntry
 * PURPOSE : Get an entry for a CNPV.
 * INPUT   : entry.cnpv - the specified CNPV
 * OUTPUT  : entry - buffer to contain the cnpv entry
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : Only active entry is returned
 */
UI32_T CN_POM_GetCnpvEntry(CN_TYPE_CnpvEntry_T *entry)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_PriData_T     pri_data;
    UI32_T              result;

    /* BODY
     */

    if (entry == NULL)
    {
        return CN_TYPE_RETURN_ERROR_NULL_POINTER;
    }

    result = CN_OM_GetPriData(entry->cnpv, &pri_data);
    if (result != CN_TYPE_RETURN_OK)
    {
        return result;
    }

    if (pri_data.active == FALSE)
    {
        return CN_TYPE_RETURN_ERROR_CNPV_EXISTENCE;
    }

    entry->defense_mode         = pri_data.defense_mode;
    entry->admin_alt_priority   = pri_data.admin_alt_priority;
    entry->auto_alt_priority    = pri_data.auto_alt_priority;

    return CN_TYPE_RETURN_OK;
} /* End of CN_POM_GetCnpvEntry */

/* FUNCTION NAME - CN_POM_GetNextCnpvEntry
 * PURPOSE : Get the next entry for a CNPV.
 * INPUT   : entry.cnpv - the specified CNPV
 * OUTPUT  : entry - buffer to contain the next cnpv entry
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : Only active entry is returned
 */
UI32_T CN_POM_GetNextCnpvEntry(CN_TYPE_CnpvEntry_T *entry)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_PriData_T     pri_data;
    UI32_T              cnpv, result;

    /* BODY
     */

    if (entry == NULL)
    {
        return CN_TYPE_RETURN_ERROR_NULL_POINTER;
    }

    cnpv = entry->cnpv;
    result = CN_OM_GetNextCnpv(&cnpv);
    if (result != CN_TYPE_RETURN_OK)
    {
        return result;
    }

    result = CN_OM_GetPriData(cnpv, &pri_data);
    if (result != CN_TYPE_RETURN_OK)
    {
        return result;
    }

    entry->cnpv                 = cnpv;
    entry->defense_mode         = pri_data.defense_mode;
    entry->admin_alt_priority   = pri_data.admin_alt_priority;
    entry->auto_alt_priority    = pri_data.auto_alt_priority;

    return CN_TYPE_RETURN_OK;
} /* End of CN_POM_GetNextCnpvEntry */

/* FUNCTION NAME - CN_OM_GetPortCnpvEntry
 * PURPOSE : Get an entry for a CNPV on a port.
 * INPUT   : entry.cnpv  - the specified CNPV
 *           entry.lport - the specified logical port
 * OUTPUT  : entry - buffer to contain the port cnpv entry
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : Only active entry is returned
 */
UI32_T CN_POM_GetPortCnpvEntry(CN_TYPE_PortCnpvEntry_T *entry)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_PortPriData_T     port_pri_data;
    UI32_T                  result;

    /* BODY
     */

    if (entry == NULL)
    {
        return CN_TYPE_RETURN_ERROR_NULL_POINTER;
    }

    result = CN_OM_GetPortPriData(entry->cnpv, entry->lport, &port_pri_data);
    if (result != CN_TYPE_RETURN_OK)
    {
        return result;
    }

    if (port_pri_data.active == FALSE)
    {
        return CN_TYPE_RETURN_ERROR_PORT_CNPV_EXISTENCE;
    }

    entry->admin_defense_mode   = port_pri_data.admin_defense_mode;
    entry->oper_defense_mode    = port_pri_data.oper_defense_mode;
    entry->admin_alt_priority   = port_pri_data.admin_alt_priority;
    entry->oper_alt_priority    = port_pri_data.oper_alt_priority;

    return CN_TYPE_RETURN_OK;
} /* End of CN_POM_GetPortCnpvEntry */

/* FUNCTION NAME - CN_OM_GetNextPortCnpvEntry
 * PURPOSE : Get the next entry for a CNPV on a port.
 * INPUT   : entry.cnpv  - the specified CNPV
 *           entry.lport - the specified logical port
 * OUTPUT  : entry - buffer to contain the next port cnpv entry
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : Next lport first, then next CNPV; but if inputed entry.cnpv
 *           is not a CNPV, next CNPV first
 */
UI32_T CN_POM_GetNextPortCnpvEntry(CN_TYPE_PortCnpvEntry_T *entry)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_PortPriData_T     port_pri_data;
    UI32_T                  cnpv, lport, result;

    /* BODY
     */

    if (entry == NULL)
    {
        return CN_TYPE_RETURN_ERROR_NULL_POINTER;
    }

    cnpv = entry->cnpv;
    if (CN_OM_IsCnpv(cnpv) == FALSE)
    {
        result = CN_OM_GetNextCnpv(&cnpv);
        if (result != CN_TYPE_RETURN_OK)
        {
            return result;
        }

        lport = 0;
        if (SWCTRL_POM_GetNextLogicalPort(&lport) != SWCTRL_LPORT_NORMAL_PORT)
        {
            return CN_TYPE_RETURN_ERROR;
        }
    }
    else
    {
        lport = entry->lport;
        if (SWCTRL_POM_GetNextLogicalPort(&lport) != SWCTRL_LPORT_NORMAL_PORT)
        {
            result = CN_OM_GetNextCnpv(&cnpv);
            if (result != CN_TYPE_RETURN_OK)
            {
                return result;
            }

            lport = 0;
            if (SWCTRL_POM_GetNextLogicalPort(&lport)
                    != SWCTRL_LPORT_NORMAL_PORT)
            {
                return CN_TYPE_RETURN_ERROR;
            }
        }
    }

    result = CN_OM_GetPortPriData(cnpv, lport, &port_pri_data);
    if (result != CN_TYPE_RETURN_OK)
    {
        return result;
    }

    if (port_pri_data.active == FALSE)
    {
        return CN_TYPE_RETURN_ERROR_PORT_CNPV_EXISTENCE;
    }

    entry->cnpv                 = cnpv;
    entry->lport                = lport;
    entry->admin_defense_mode   = port_pri_data.admin_defense_mode;
    entry->oper_defense_mode    = port_pri_data.oper_defense_mode;
    entry->admin_alt_priority   = port_pri_data.admin_alt_priority;
    entry->oper_alt_priority    = port_pri_data.oper_alt_priority;

    return CN_TYPE_RETURN_OK;
} /* End of CN_POM_GetNextPortCnpvEntry */

/* FUNCTION NAME - CN_POM_GetCpEntry
 * PURPOSE : Get an entry for a CP.
 * INPUT   : entry.lport    - the specified logical port number
 *           entry.cp_index - the specified CP index
 * OUTPUT  : entry - buffer to contain the cp entry
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : Only active cp entry
 */
UI32_T CN_POM_GetCpEntry(CN_TYPE_CpEntry_T *entry)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_CpData_T          cp_data;
    UI8_T                   mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T                  result;

    /* BODY
     */

    if (entry == NULL)
    {
        return CN_TYPE_RETURN_ERROR_NULL_POINTER;
    }

    result = CN_OM_GetCpData(entry->lport, entry->cp_index, &cp_data);
    if (result != CN_TYPE_RETURN_OK)
    {
        return result;
    }

    if (cp_data.active == FALSE)
    {
        return CN_TYPE_RETURN_ERROR_CP_EXISTENCE;
    }

    if (SWCTRL_POM_GetCpuMac(mac) == FALSE)
    {
        printf("\r\n %s: failed for calling SWCTRL_POM_GetCpuMac(%p) \r\n",
            __FUNCTION__, mac);
        return CN_TYPE_RETURN_ERROR;
    }

    entry->queue                = cp_data.queue;
    entry->managed_cnpvs        = cp_data.managed_cnpvs;
    memcpy(entry->mac_address, mac, SYS_ADPT_MAC_ADDR_LEN);
    entry->set_point            = cp_data.set_point;
    entry->feedback_weight      = cp_data.feedback_weight;
    entry->min_sample_base      = cp_data.min_sample_base;

    return CN_TYPE_RETURN_OK;
} /* End of CN_POM_GetCpEntry */

/* FUNCTION NAME - CN_POM_GetNextCpEntry
 * PURPOSE : Get the next entry for a CP.
 * INPUT   : entry.lport    - the specified logical port number
 *           entry.cp_index - the specified CP index
 * OUTPUT  : entry - buffer to contain the next cp entry
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : Next cp index first, then next lport
 */
UI32_T CN_POM_GetNextCpEntry(CN_TYPE_CpEntry_T *entry)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_CpData_T          cp_data;
    UI32_T                  lport, cp_index;
    UI8_T                   mac[SYS_ADPT_MAC_ADDR_LEN];

    /* BODY
     */

    if (entry == NULL)
    {
        return CN_TYPE_RETURN_ERROR_NULL_POINTER;
    }

    lport = entry->lport;
    cp_index = entry->cp_index;
    while (1)
    {
        cp_index++;
        if (cp_index >= SYS_ADPT_CN_MAX_NBR_OF_CP_PER_PORT)
        {
            if (SWCTRL_POM_GetNextLogicalPort(&lport) !=
                    SWCTRL_LPORT_NORMAL_PORT)
            {
                return CN_TYPE_RETURN_ERROR;
            }
            else
            {
                cp_index = 0;
            }
        }

        if (    (CN_OM_GetCpData(lport, cp_index, &cp_data) == CN_TYPE_RETURN_OK)
             && (cp_data.active == TRUE)
           )
        {
            break;
        }
    }

    if (SWCTRL_POM_GetCpuMac(mac) == FALSE)
    {
        printf("\r\n %s: failed for calling SWCTRL_POM_GetCpuMac(%p) \r\n",
            __FUNCTION__, mac);
        return CN_TYPE_RETURN_ERROR;
    }

    entry->lport                = lport;
    entry->cp_index             = cp_index;
    entry->queue                = cp_data.queue;
    entry->managed_cnpvs        = cp_data.managed_cnpvs;
    memcpy(entry->mac_address, mac, SYS_ADPT_MAC_ADDR_LEN);
    entry->set_point            = cp_data.set_point;
    entry->feedback_weight      = cp_data.feedback_weight;
    entry->min_sample_base      = cp_data.min_sample_base;

    return CN_TYPE_RETURN_OK;
} /* End of CN_POM_GetNextCpEntry */

/* FUNCTION NAME - CN_POM_IsCnpv
 * PURPOSE : Check whether a priority is a CNPV.
 * INPUT   : priroity - the priority to be checked
 * OUTPUT  : None
 * RETURN  : TRUE  - the priority is a CNPV
 *           FALSE - the priority is not a CNPV
 * NOTES   : None
 */
BOOL_T CN_POM_IsCnpv(UI32_T priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return CN_OM_IsCnpv(priority);
} /* End of CN_POM_IsCnpv */

/* FUNCTION NAME - CN_POM_IsCp
 * PURPOSE : Check whether it is a CP given port and index.
 * INPUT   : lport - the lport where the CP is located
 *           index - the index of the CP
 * OUTPUT  : None
 * RETURN  : TRUE  - it is a CP
 *           FALSE - it is not a CP
 * NOTES   : None
 */
BOOL_T CN_POM_IsCp(UI32_T lport, UI32_T index)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return CN_OM_IsCp(lport, index);
} /* End of CN_POM_IsCp */

/* LOCAL SUBPROGRAM BODIES
 */
