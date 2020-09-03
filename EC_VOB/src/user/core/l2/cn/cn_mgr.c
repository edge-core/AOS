/* MODULE NAME - CN_MGR.C
 * PURPOSE : Provides the definitions for CN functional management.
 * NOTES   : None.
 * HISTORY : 2012/09/12 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2012
 */

/* INCLUDE FILE DECLARATIONS
 */

#include "sys_cpnt.h"
#if (SYS_CPNT_CN == TRUE)
#include <stdio.h>
#include <string.h>
#include "sys_type.h"
#include "l_cvrt.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#include "cn_backdoor.h"
#include "cn_engine.h"
#include "cn_mgr.h"
#include "cn_om.h"
#include "cn_om_private.h"
#include "cn_type.h"
#include "l4_pmgr.h"
#include "cos_vm.h"
#include "swctrl_pmgr.h"
#include "swctrl.h"
#if (SYS_CPNT_PFC == TRUE)
#include "pfc_om.h"
#include "pfc_type.h"
#endif
#if (SYS_CPNT_ETS == TRUE)
#include "ets_om.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

enum TRUNK_CALLBACK_ACTION_E
{
    CN_MGR_TRUNK_MEMBER_ADD,
    CN_MGR_TRUNK_MEMBER_DELETE,
};

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

static void CN_MGR_ProcessTrunkMemberCallback(UI32_T ifindex, UI32_T action);
static UI32_T CN_MGR_SetGlobalStatus(UI32_T status);
static UI32_T CN_MGR_SetPortCnpvOperAlternatePriority(UI32_T priority,
                UI32_T lport, UI32_T alt_priority);

/* STATIC VARIABLE DEFINITIONS
 */

SYSFUN_DECLARE_CSC

static BOOL_T is_provision_complete;

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME - CN_MGR_InitiateProcessResources
 * PURPOSE : Initiate process resources.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_InitiateProcessResources(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    return;
} /* End of CN_MGR_InitiateProcessResources */

/* FUNCTION NAME - CN_MGR_CreateInterCscRelation
 * PURPOSE : Create inter-CSC relations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_CreateInterCscRelation(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("cn",
        SYS_BLD_DCB_GROUP_IPCMSGQ_KEY, CN_BACKDOOR_Main);
    return;
} /* End of CN_MGR_CreateInterCscRelation */

/* FUNCTION NAME - CN_MGR_SetTransitionMode
 * PURPOSE : Process when setting CN to transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_SetTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    SYSFUN_SET_TRANSITION_MODE();
    return;
} /* End of CN_MGR_SetTransitionMode */

/* FUNCTION NAME - CN_MGR_EnterTransitionMode
 * PURPOSE : Process when CN enters transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_EnterTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    SYSFUN_ENTER_TRANSITION_MODE();
    is_provision_complete = FALSE;
    CN_OM_ResetAll();
    return;
} /* End of CN_MGR_EnterTransitionMode */

/* FUNCTION NAME - CN_MGR_EnterMasterMode
 * PURPOSE : Process when CN enters master mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_EnterMasterMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_GlobalData_T  *global_data_p;

    /* BODY
     */

    CN_OM_DefaultAll();

    global_data_p = CN_OM_GetGlobalDataPtr();
    if (global_data_p->admin_status == CN_TYPE_GLOBAL_STATUS_ENABLE)
    {
        if (CN_MGR_SetGlobalStatus(CN_TYPE_GLOBAL_STATUS_ENABLE) !=
                CN_TYPE_RETURN_OK)
        {
            return;
        }
    }

    SYSFUN_ENTER_MASTER_MODE();
    return;
} /* End of CN_MGR_EnterMasterMode */

/* FUNCTION NAME - CN_MGR_EnterSlaveMode
 * PURPOSE : Process EnterSlaveMode for LBD MGR.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_EnterSlaveMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    SYSFUN_ENTER_SLAVE_MODE();
    return;
} /* End of CN_MGR_EnterSlaveMode */

/* FUNCTION NAME - CN_MGR_ProvisionComplete
 * PURPOSE : Process when CN is informed of provision complete.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_ProvisionComplete(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    is_provision_complete = TRUE;
    return;
} /* End of CN_MGR_ProvisionComplete */

/* FUNCTION NAME - CN_MGR_TrunkMemberAdd1st_CallBack
 * PURPOSE : Process callback when the first member is added to a trunk.
 * INPUT   : trunk_ifindex  - the ifindex of the trunk
 *           member_ifindex - the ifindex of the member
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_TrunkMemberAdd1st_CallBack(UI32_T trunk_ifindex,
        UI32_T member_ifindex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (CN_OM_ThreadDebug() == TRUE)
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu, %lu \r\n", __FUNCTION__,
            trunk_ifindex, member_ifindex);
    }

    CN_MGR_ProcessTrunkMemberCallback(member_ifindex, CN_MGR_TRUNK_MEMBER_ADD);
    return;
} /* End of CN_MGR_TrunkMemberAdd1st_CallBack */

/* FUNCTION NAME - CN_MGR_TrunkMemberAdd_CallBack
 * PURPOSE : Process callback when a member except the first is added to a
 *           trunk.
 * INPUT   : trunk_ifindex  - the ifindex of the trunk
 *           member_ifindex - the ifindex of the member
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_TrunkMemberAdd_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (CN_OM_ThreadDebug() == TRUE)
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu, %lu \r\n", __FUNCTION__,
            trunk_ifindex, member_ifindex);
    }

    CN_MGR_ProcessTrunkMemberCallback(member_ifindex, CN_MGR_TRUNK_MEMBER_ADD);
    return;
} /* End of CN_MGR_TrunkMemberAdd_CallBack */

/* FUNCTION NAME - CN_MGR_TrunkMemberDelete_CallBack
 * PURPOSE : Process callback when a member except the last is deleted from a
 *           trunk.
 * INPUT   : trunk_ifindex  - the ifindex of the trunk
 *           member_ifindex - the ifindex of the member
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_TrunkMemberDelete_CallBack(UI32_T trunk_ifindex,
        UI32_T member_ifindex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (CN_OM_ThreadDebug() == TRUE)
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu, %lu \r\n", __FUNCTION__,
            trunk_ifindex, member_ifindex);
    }

    CN_MGR_ProcessTrunkMemberCallback(member_ifindex, CN_MGR_TRUNK_MEMBER_DELETE);
    return;
} /* End of CN_MGR_TrunkMemberDelete_CallBack */

/* FUNCTION NAME - CN_MGR_TrunkMemberDeleteLst_CallBack
 * PURPOSE : Process callback when the last member is deleted from a trunk.
 * INPUT   : trunk_ifindex  - the ifindex of the trunk
 *           member_ifindex - the ifindex of the member
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_TrunkMemberDeleteLst_CallBack(UI32_T trunk_ifindex,
        UI32_T member_ifindex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (CN_OM_ThreadDebug() == TRUE)
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu, %lu \r\n", __FUNCTION__,
            trunk_ifindex, member_ifindex);
    }

    CN_MGR_ProcessTrunkMemberCallback(member_ifindex, CN_MGR_TRUNK_MEMBER_DELETE);
    return;
} /* End of CN_MGR_TrunkMemberDeleteLst_CallBack */

/* FUNCTION NAME - CN_MGR_RemoteChange_CallBack
 * PURPOSE : Process callback when the remote CN data is changed.
 * INPUT   : lport            - the logical port which receives the CN TLV
 *           neighbor_num     - the number of neighbors
 *           cnpv_indicators  - the CNPV indicators in the received CN TLV
 *           ready_indicators - the Ready indicators in the received CN TLV
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_MGR_RemoteChange_CallBack(UI32_T lport, UI32_T neighbor_num,
    UI32_T cnpv_indicators, UI32_T ready_indicators)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    SWCTRL_Lport_Type_T     port_type;
    UI32_T                  unit, port, trunk_id;
    UI32_T                  i, result;

    /* BODY
     */

    if (CN_OM_ThreadDebug() == TRUE)
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu, %lu, %lu, %lu \r\n", __FUNCTION__,
            lport, neighbor_num, cnpv_indicators, ready_indicators);
    }

    port_type = SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);
    if (port_type != SWCTRL_LPORT_NORMAL_PORT)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: invalid logical port \r\n",
                __FUNCTION__);
        }
        return;
    }

    for (i = 0; i < 8; i++)
    {
        if (cnpv_indicators & (1 << i))
        {
            if (neighbor_num > 1)
            {
                result = CN_ENGINE_ProgressStateMachine(i, lport,
                            CN_ENGINE_EVENT_NOT_RCV_CNPV_EV);
                if ((result != CN_TYPE_RETURN_OK) && (CN_OM_ErrDebug() == TRUE))
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                        __LINE__, CN_OM_ErrStr(result));
                }
            }
            else
            {
                result = CN_ENGINE_ProgressStateMachine(i, lport,
                            CN_ENGINE_EVENT_RCV_CNPV_EV);
                if ((result != CN_TYPE_RETURN_OK) && (CN_OM_ErrDebug() == TRUE))
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                        __LINE__, CN_OM_ErrStr(result));
                }

                if (ready_indicators & (1 << i))
                {
                    result = CN_ENGINE_ProgressStateMachine(i, lport,
                                CN_ENGINE_EVENT_RCV_READY_EV);
                    if (    (result != CN_TYPE_RETURN_OK)
                         && (CN_OM_ErrDebug() == TRUE)
                       )
                    {
                        BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n",
                            __FUNCTION__, __LINE__, CN_OM_ErrStr(result));
                    }
                }
                else
                {
                    result = CN_ENGINE_ProgressStateMachine(i, lport,
                                CN_ENGINE_EVENT_NOT_RCV_READY_EV);
                    if (    (result != CN_TYPE_RETURN_OK)
                         && (CN_OM_ErrDebug() == TRUE)
                       )
                    {
                        BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n",
                            __FUNCTION__, __LINE__, CN_OM_ErrStr(result));
                    }
                }
            }
        }
        else
        {
            CN_ENGINE_ProgressStateMachine(i, lport,
                CN_ENGINE_EVENT_NOT_RCV_CNPV_EV);
        }
    }

    return;
} /* End of CN_MGR_RemoteChange_CallBack */

/* FUNCTION NAME - CN_MGR_SetGlobalAdminStatus
 * PURPOSE : Set CN global admin status.
 * INPUT   : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_MGR_SetGlobalAdminStatus(UI32_T status)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_GlobalData_T  *global_data_p;
    UI32_T              result;

    /* BODY
     */

    if (CN_OM_ThreadDebug() == TRUE)
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu \r\n", __FUNCTION__, status);
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: not master mode \r\n", __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_MASTER_MODE;
    }

    if ((status != CN_TYPE_GLOBAL_STATUS_ENABLE) &&
        (status != CN_TYPE_GLOBAL_STATUS_DISABLE))
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: invalid global status \r\n",
                __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_GLOBAL_STATUS_RANGE;
    }

    global_data_p = CN_OM_GetGlobalDataPtr();
    if (global_data_p->admin_status == status)
    {
        return CN_TYPE_RETURN_OK;
    }

    result = CN_OM_SetGlobalAdminStatus(status);
    if (result != CN_TYPE_RETURN_OK)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__, __LINE__,
                CN_OM_ErrStr(result));
        }
        return result;
    }

    result = CN_MGR_SetGlobalStatus(status);
    if (result == CN_TYPE_RETURN_ERROR_PFC_CONFLICT)
    {
        printf("Warning: operational status is not enabled because of PFC "
            "conflict.");
    }
    else if (result == CN_TYPE_RETURN_ERROR_ETS_CONFLICT)
    {
        printf("Warning: operational status is not enabled because of ETS "
            "conflict.");
    }
    else if (result != CN_TYPE_RETURN_OK)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__, __LINE__,
                CN_OM_ErrStr(result));
        }
        return CN_TYPE_RETURN_OK;
    }

    return CN_TYPE_RETURN_OK;
} /* End of CN_MGR_SetGlobalAdminStatus */

/* FUNCTION NAME - CN_MGR_SetGlobalOperStatus
 * PURPOSE : Set CN global oper status.
 * INPUT   : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : For other core layer CSCs
 */
UI32_T CN_MGR_SetGlobalOperStatus(UI32_T status)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_GlobalData_T  *global_data_p;
    UI32_T              result;

    /* BODY
     */

    if (CN_OM_ThreadDebug() == TRUE)
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu \r\n", __FUNCTION__, status);
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: not master mode \r\n", __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_MASTER_MODE;
    }

    if ((status != CN_TYPE_GLOBAL_STATUS_ENABLE) &&
        (status != CN_TYPE_GLOBAL_STATUS_DISABLE))
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: invalid global status \r\n",
                __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_GLOBAL_STATUS_RANGE;
    }

    global_data_p = CN_OM_GetGlobalDataPtr();
    if (global_data_p->oper_status == status)
    {
        return CN_TYPE_RETURN_OK;
    }
    if (global_data_p->admin_status == CN_TYPE_GLOBAL_STATUS_DISABLE)
    {
        return CN_TYPE_RETURN_OK;
    }

    result = CN_MGR_SetGlobalStatus(status);
    if (result != CN_TYPE_RETURN_OK)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__, __LINE__,
                CN_OM_ErrStr(result));
        }
        return result;
    }

    return CN_TYPE_RETURN_OK;
} /* End of CN_MGR_SetGlobalOperStatus */

/* FUNCTION NAME - CN_MGR_SetCnmTxPriority
 * PURPOSE : Set the priority used for transmitting CNMs.
 * INPUT   : priority - the specified priority
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_MGR_SetCnmTxPriority(UI32_T priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_GlobalData_T  *global_data_p;
    UI32_T              result;

    /* BODY
     */

    if (CN_OM_ThreadDebug() == TRUE)
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu \r\n", __FUNCTION__, priority);
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: not master mode \r\n", __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_MASTER_MODE;
    }

    if ((priority < CN_TYPE_MIN_PRIORITY) || (priority > CN_TYPE_MAX_PRIORITY))
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: invalid priority \r\n", __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_PRIORITY_RANGE;
    }

    global_data_p = CN_OM_GetGlobalDataPtr();
    if (global_data_p->cnm_tx_priority == priority)
    {
        return CN_TYPE_RETURN_OK;
    }

    result = CN_OM_SetCnmTxPriority(priority);
    if (result != CN_TYPE_RETURN_OK)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__, __LINE__,
                CN_OM_ErrStr(result));
        }
        return result;
    }

    if (SWCTRL_PMGR_SetQcnCnmPriority(priority) == FALSE)
    {
        return CN_TYPE_RETURN_ERROR;
    }

    return CN_TYPE_RETURN_OK;
} /* End of CN_MGR_SetCnmTxPriority */

/* FUNCTION NAME - CN_MGR_SetCnpv
 * PURPOSE : Set a priority to be CNPV or not.
 * INPUT   : priority - the specified priority
 *           active   - TRUE/FALSE
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : The failures of per-port processes are not considered to be failed
 *           for the function; these failures can be found by checking per-port
 *           oper defense mode
 */
UI32_T CN_MGR_SetCnpv(UI32_T priority, BOOL_T active)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_PriData_T         *pri_data_p;
    CN_OM_PortPriData_T     *port_pri_data_p;
    CN_OM_CpData_T          *cp_data_p;
    UI32_T                  result, auto_priority, lport;
    I32_T                   i;
    BOOL_T                  alt_priority_changed, warning;
    UI8_T   cp_freed[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

    /* BODY
     */

    if (CN_OM_ThreadDebug() == TRUE)
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu, %lu \r\n", __FUNCTION__, priority,
            active);
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: not master mode \r\n", __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_MASTER_MODE;
    }

    if ((priority < CN_TYPE_MIN_PRIORITY) || (priority > CN_TYPE_MAX_PRIORITY))
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: invalid priority \r\n", __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_PRIORITY_RANGE;
    }

    if ((active != TRUE) && (active != FALSE))
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: invalid active \r\n", __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_ACTIVE_RANGE;
    }

    pri_data_p = CN_OM_GetPriDataPtr(priority);
    if (pri_data_p->active == active)
    {
        return CN_TYPE_RETURN_OK;
    }

    /* search a non-CNPV to be auto alternate priority for a new CNPV:
     * 1. search the next smaller non-CNPV than the CNPV
     * 2. if not found, search the next larger non-CNPV
     */
    auto_priority = CN_TYPE_MAX_PRIORITY + 1;
    if (active == TRUE)
    {
        if (priority == CN_TYPE_MIN_PRIORITY)
        {
            for (i = priority+1; i <= CN_TYPE_MAX_PRIORITY; i++)
            {
                pri_data_p = CN_OM_GetPriDataPtr(i);
                if (pri_data_p->active == FALSE)
                {
                    auto_priority = i;
                    break;
                }
            }
        }
        else
        {
            for (i = priority-1; i >= CN_TYPE_MIN_PRIORITY; i--)
            {
                pri_data_p = CN_OM_GetPriDataPtr(i);
                if (pri_data_p->active == FALSE)
                {
                    auto_priority = i;
                    break;
                }
            }
            if (auto_priority > CN_TYPE_MAX_PRIORITY)
            {
                for (i = priority+1; i <= CN_TYPE_MAX_PRIORITY; i++)
                {
                    pri_data_p = CN_OM_GetPriDataPtr(i);
                    if (pri_data_p->active == FALSE)
                    {
                        auto_priority = i;
                        break;
                    }
                }
            }
        }

        /* update auto alternate priority for this new CNPV */
        result = CN_OM_SetPriAlternatePriority(priority, auto_priority, FALSE);
        if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return result;
        }
    } /* if active == TRUE */

    result = CN_OM_SetPriStatus(priority, active);
    if (result != CN_TYPE_RETURN_OK)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__, __LINE__,
                CN_OM_ErrStr(result));
        }
        return result;
    }

    warning = FALSE;

    /* update port cnpv entries related to this priority */
    lport = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) == SWCTRL_LPORT_NORMAL_PORT)
    {
        result = CN_OM_SetPortPriStatus(priority, lport, active);
        if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return CN_TYPE_RETURN_OK;
        }

        pri_data_p = CN_OM_GetPriDataPtr(priority);
        port_pri_data_p = CN_OM_GetPortPriDataPtr(priority, lport);
        if (    (port_pri_data_p->admin_alt_priority ==
                    CN_TYPE_ALTERNATE_PRIORITY_BY_GLOBAL)
             && (    (port_pri_data_p->admin_defense_mode ==
                        CN_TYPE_DEFENSE_MODE_AUTO)
                  || (    (port_pri_data_p->admin_defense_mode ==
                            CN_TYPE_DEFENSE_MODE_BY_GLOBAL)
                       && (pri_data_p->defense_mode == CN_TYPE_DEFENSE_MODE_AUTO)
                     )
                )
           )
        {
            result = CN_OM_SetPortPriAlternatePriority(priority, lport,
                        auto_priority, FALSE);
            if (result != CN_TYPE_RETURN_OK)
            {
                if (CN_OM_ErrDebug() == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                        __LINE__, CN_OM_ErrStr(result));
                }
                return CN_TYPE_RETURN_OK;
            }
        }

        if (active == TRUE)
        {
            result = CN_ENGINE_ProgressStateMachine(priority, lport,
                        CN_ENGINE_EVENT_BEGIN);
            if (result == CN_TYPE_RETURN_ERROR_CP_CAPACITY)
            {
                warning = TRUE;
                continue;
            }
            else if (result != CN_TYPE_RETURN_OK)
            {
                if (CN_OM_ErrDebug() == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                        __LINE__, CN_OM_ErrStr(result));
                }
                return CN_TYPE_RETURN_OK;
            }
        }
        else
        {
            result = CN_ENGINE_ResetStateMachine(priority, lport);
            if (result != CN_TYPE_RETURN_OK)
            {
                if (CN_OM_ErrDebug() == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                        __LINE__, CN_OM_ErrStr(result));
                }
                return CN_TYPE_RETURN_OK;
            }

            /* check if there is cp freed */
            for (i = 0; i < SYS_ADPT_CN_MAX_NBR_OF_CP_PER_PORT; i++)
            {
                cp_data_p = CN_OM_GetCpDataPtr(lport, i);
                if (cp_data_p->active == FALSE)
                {
                    L_CVRT_ADD_MEMBER_TO_PORTLIST(cp_freed, lport);
                }
            }
        }
    }

    /* update other active cnpv entries and related port cnpv entries */
    for (i = CN_TYPE_MIN_PRIORITY; i <= CN_TYPE_MAX_PRIORITY; i++)
    {
        if (i == priority)
        {
            continue;
        }

        pri_data_p = CN_OM_GetPriDataPtr(i);
        if (pri_data_p->active == FALSE)
        {
            continue;
        }

        if (active == TRUE)
        {
            if (pri_data_p->auto_alt_priority == priority)
            {
                result = CN_OM_SetPriAlternatePriority(i, auto_priority, FALSE);
                if (result != CN_TYPE_RETURN_OK)
                {
                    if (CN_OM_ErrDebug() == TRUE)
                    {
                        BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n",
                            __FUNCTION__, __LINE__, CN_OM_ErrStr(result));
                    }
                    return CN_TYPE_RETURN_OK;
                }

                lport = 0;
                while (SWCTRL_GetNextLogicalPort(&lport) ==
                        SWCTRL_LPORT_NORMAL_PORT)
                {
                    port_pri_data_p = CN_OM_GetPortPriDataPtr(i, lport);

                    if (    (    (port_pri_data_p->admin_defense_mode ==
                                    CN_TYPE_DEFENSE_MODE_BY_GLOBAL)
                              && (pri_data_p->defense_mode ==
                                    CN_TYPE_DEFENSE_MODE_AUTO)
                            )
                         || (port_pri_data_p->admin_defense_mode ==
                                CN_TYPE_DEFENSE_MODE_AUTO)
                       )
                    {
                        if (port_pri_data_p->oper_defense_mode ==
                                CN_TYPE_DEFENSE_MODE_DISABLED)
                        {
                            result = CN_OM_SetPortPriAlternatePriority(i, lport,
                                        auto_priority, FALSE);
                            if (result != CN_TYPE_RETURN_OK)
                            {
                                if (CN_OM_ErrDebug() == TRUE)
                                {
                                    BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n",
                                        __FUNCTION__, __LINE__,
                                        CN_OM_ErrStr(result));
                                }
                                return CN_TYPE_RETURN_OK;
                            }
                        }
                        else
                        {
                            result = CN_MGR_SetPortCnpvOperAlternatePriority(i,
                                        lport, auto_priority);
                            if (result != CN_TYPE_RETURN_OK)
                            {
                                if (CN_OM_ErrDebug() == TRUE)
                                {
                                    BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n",
                                        __FUNCTION__, __LINE__,
                                        CN_OM_ErrStr(result));
                                }
                                return CN_TYPE_RETURN_OK;
                            }
                        }
                    }
                } /* while */
            } /* if pri_data_p->auto_alt_priority == priority */
        } /* if active == TRUE */
        else
        {
            if (    (    (i < priority)
                      && (pri_data_p->auto_alt_priority > priority)
                    )
                 || (    (i > priority)
                      && (    (pri_data_p->auto_alt_priority < priority)
                           || (i < pri_data_p->auto_alt_priority)
                         )
                    )
               )
            {
                result = CN_OM_SetPriAlternatePriority(i, priority, FALSE);
                if (result != CN_TYPE_RETURN_OK)
                {
                    if (CN_OM_ErrDebug() == TRUE)
                    {
                        BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n",
                            __FUNCTION__, __LINE__, CN_OM_ErrStr(result));
                    }
                    return CN_TYPE_RETURN_OK;
                }
                alt_priority_changed = TRUE;
            }
            else
            {
                alt_priority_changed = FALSE;
            }

            lport = 0;
            while (SWCTRL_GetNextLogicalPort(&lport) == SWCTRL_LPORT_NORMAL_PORT)
            {
                port_pri_data_p = CN_OM_GetPortPriDataPtr(i, lport);
                if (    (alt_priority_changed == TRUE)
                     && (    (port_pri_data_p->admin_defense_mode ==
                                CN_TYPE_DEFENSE_MODE_AUTO)
                          || (    (port_pri_data_p->admin_defense_mode ==
                                    CN_TYPE_DEFENSE_MODE_BY_GLOBAL)
                               && (pri_data_p->defense_mode ==
                                    CN_TYPE_DEFENSE_MODE_AUTO)
                             )
                        )
                   )
                {
                    if (port_pri_data_p->oper_defense_mode ==
                            CN_TYPE_DEFENSE_MODE_DISABLED)
                    {
                        result = CN_OM_SetPortPriAlternatePriority(i, lport,
                                    priority, FALSE);
                        if (result != CN_TYPE_RETURN_OK)
                        {
                            if (CN_OM_ErrDebug() == TRUE)
                            {
                                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n",
                                    __FUNCTION__, __LINE__, CN_OM_ErrStr(result));
                            }
                            return CN_TYPE_RETURN_OK;
                        }
                    }
                    else
                    {
                        result = CN_MGR_SetPortCnpvOperAlternatePriority(i,
                                    lport, priority);
                        if (result != CN_TYPE_RETURN_OK)
                        {
                            if (CN_OM_ErrDebug() == TRUE)
                            {
                                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n",
                                    __FUNCTION__, __LINE__, CN_OM_ErrStr(result));
                            }
                            return CN_TYPE_RETURN_OK;
                        }
                    }
                }

                if (    (L_CVRT_IS_MEMBER_OF_PORTLIST(cp_freed, lport))
                     && (port_pri_data_p->oper_defense_mode ==
                            CN_TYPE_DEFENSE_MODE_DISABLED)
                     && !(    (    (port_pri_data_p->admin_defense_mode ==
                                        CN_TYPE_DEFENSE_MODE_BY_GLOBAL)
                                && (pri_data_p->defense_mode ==
                                        CN_TYPE_DEFENSE_MODE_DISABLED)
                              )
                           || (port_pri_data_p->admin_defense_mode ==
                                CN_TYPE_DEFENSE_MODE_DISABLED)
                         )
                   )
                {
                    result = CN_ENGINE_ProgressStateMachine(i, lport,
                                CN_ENGINE_EVENT_BEGIN);
                    if (result == CN_TYPE_RETURN_ERROR_CP_CAPACITY)
                    {
                        warning = TRUE;
                        continue;
                    }
                    else if (result != CN_TYPE_RETURN_OK)
                    {
                        if (CN_OM_ErrDebug() == TRUE)
                        {
                            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n",
                                __FUNCTION__, __LINE__, CN_OM_ErrStr(result));
                        }
                        return CN_TYPE_RETURN_OK;
                    }
                }
            } /* while SWCTRL_GetNextLogicalPort == SWCTRL_LPORT_NORMAL_PORT */
        } /* else (active == FALSE) */
    } /* for i <= CN_TYPE_MAX_PRIORITY */

    if (warning == TRUE)
    {
        return CN_TYPE_RETURN_ERROR_CP_CAPACITY;
    }
    else
    {
        return CN_TYPE_RETURN_OK;
    }
} /* End of CN_MGR_SetCnpv */

/* FUNCTION NAME - CN_MGR_SetCnpvDefenseMode
 * PURPOSE : Set the defense mode for a CNPV.
 * INPUT   : priority - the specified CNPV
 *           mode     - CN_TYPE_DEFENSE_MODE_E
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : The failures of per-port processes are not considered to be failed
 *           for the function; these failures can be found by checking per-port
 *           oper defense mode
 */
UI32_T CN_MGR_SetCnpvDefenseMode(UI32_T priority, UI32_T mode)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_PriData_T         *pri_data_p;
    CN_OM_PortPriData_T     *port_pri_data_p;
    CN_OM_CpData_T          *cp_data_p;
    UI32_T                  old_mode, result, lport, i;
    BOOL_T                  warning, cp_freed;

    /* BODY
     */

    if (CN_OM_ThreadDebug() == TRUE)
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu, %lu \r\n", __FUNCTION__, priority,
            mode);
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: not master mode \r\n", __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_MASTER_MODE;
    }

    if ((priority < CN_TYPE_MIN_PRIORITY) || (priority > CN_TYPE_MAX_PRIORITY))
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: invalid priority \r\n", __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_PRIORITY_RANGE;
    }

    if (    (mode < CN_TYPE_DEFENSE_MODE_DISABLED)
         || (mode > CN_TYPE_DEFENSE_MODE_AUTO)
       )
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: invalid defense mdoe \r\n",
                __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_MODE_RANGE;
    }

    pri_data_p = CN_OM_GetPriDataPtr(priority);
    if (pri_data_p->active == FALSE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: cnpv entry is inactive \r\n",
                __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_CNPV_EXISTENCE;
    }

    old_mode = pri_data_p->defense_mode;
    if (old_mode == mode)
    {
        return CN_TYPE_RETURN_OK;
    }

    result = CN_OM_SetPriDefenseMode(priority, mode);
    if (result != CN_TYPE_RETURN_OK)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__, __LINE__,
                CN_OM_ErrStr(result));
        }
        return result;
    }

    warning = FALSE;

    /* progress state machines for port cnpv entries with admin defense mode
     * equal to CN_TYPE_DEFENSE_MODE_BY_GLOBAL
     */
    lport = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) == SWCTRL_LPORT_NORMAL_PORT)
    {
        pri_data_p = CN_OM_GetPriDataPtr(priority);
        port_pri_data_p = CN_OM_GetPortPriDataPtr(priority, lport);

        if (port_pri_data_p->admin_defense_mode !=
                CN_TYPE_DEFENSE_MODE_BY_GLOBAL)
        {
            continue;
        }

        if (mode == CN_TYPE_DEFENSE_MODE_AUTO)
        {
            result = CN_OM_SetPortPriAlternatePriority(priority, lport,
                        pri_data_p->auto_alt_priority, FALSE);
            if (result != CN_TYPE_RETURN_OK)
            {
                if (CN_OM_ErrDebug() == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                        __LINE__, CN_OM_ErrStr(result));
                }
                return CN_TYPE_RETURN_OK;
            }

            result = CN_ENGINE_ProgressStateMachine(priority, lport,
                        CN_ENGINE_EVENT_AUTO_EV);
            if (result == CN_TYPE_RETURN_ERROR_CP_CAPACITY)
            {
                warning = TRUE;
                continue;
            }
            else if (result != CN_TYPE_RETURN_OK)
            {
                if (CN_OM_ErrDebug() == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                        __LINE__, CN_OM_ErrStr(result));
                }
                return CN_TYPE_RETURN_OK;
            }
        } /* if mode == CN_TYPE_DEFENSE_MODE_AUTO */
        else
        {
            if (old_mode == CN_TYPE_DEFENSE_MODE_AUTO)
            {
                if (port_pri_data_p->admin_alt_priority ==
                        CN_TYPE_ALTERNATE_PRIORITY_BY_GLOBAL)
                {
                    result = CN_OM_SetPortPriAlternatePriority(priority, lport,
                                pri_data_p->admin_alt_priority, FALSE);
                    if (result != CN_TYPE_RETURN_OK)
                    {
                        if (CN_OM_ErrDebug() == TRUE)
                        {
                            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n",
                                __FUNCTION__, __LINE__, CN_OM_ErrStr(result));
                        }
                        return CN_TYPE_RETURN_OK;
                    }
                }
                else
                {
                    result = CN_OM_SetPortPriAlternatePriority(priority, lport,
                                port_pri_data_p->admin_alt_priority, FALSE);
                    if (result != CN_TYPE_RETURN_OK)
                    {
                        if (CN_OM_ErrDebug() == TRUE)
                        {
                            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n",
                                __FUNCTION__, __LINE__, CN_OM_ErrStr(result));
                        }
                        return CN_TYPE_RETURN_OK;
                    }
                }
            }

            result = CN_ENGINE_ProgressStateMachine(priority, lport,
                        CN_ENGINE_EVENT_ADMIN_EV);
            if (result == CN_TYPE_RETURN_ERROR_CP_CAPACITY)
            {
                warning = TRUE;
                continue;
            }
            else if (result != CN_TYPE_RETURN_OK)
            {
                if (CN_OM_ErrDebug() == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                        __LINE__, CN_OM_ErrStr(result));
                }
                return CN_TYPE_RETURN_OK;
            }

            cp_freed = FALSE;
            for (i = 0; i < SYS_ADPT_CN_MAX_NBR_OF_CP_PER_PORT; i++)
            {
                cp_data_p = CN_OM_GetCpDataPtr(lport, i);
                if (cp_data_p->active == FALSE)
                {
                    cp_freed = TRUE;
                    break;
                }
            }
            if (cp_freed == TRUE)
            {
                for (i = CN_TYPE_MIN_PRIORITY; i <= CN_TYPE_MAX_PRIORITY; i++)
                {
                    if (i == priority)
                    {
                        continue;
                    }

                    pri_data_p = CN_OM_GetPriDataPtr(i);
                    if (pri_data_p->active == FALSE)
                    {
                        continue;
                    }

                    port_pri_data_p = CN_OM_GetPortPriDataPtr(i, lport);
                    if (    (port_pri_data_p->oper_defense_mode ==
                                CN_TYPE_DEFENSE_MODE_DISABLED)
                         && !(    (port_pri_data_p->admin_defense_mode ==
                                    CN_TYPE_DEFENSE_MODE_DISABLED)
                               || (    (port_pri_data_p->admin_defense_mode ==
                                            CN_TYPE_DEFENSE_MODE_BY_GLOBAL)
                                    && (pri_data_p->defense_mode ==
                                            CN_TYPE_DEFENSE_MODE_DISABLED)
                                  )
                             )
                       )
                    {
                        result = CN_ENGINE_ProgressStateMachine(i, lport,
                                    CN_ENGINE_EVENT_BEGIN);
                        if (result == CN_TYPE_RETURN_ERROR_CP_CAPACITY)
                        {
                            warning = TRUE;
                            continue;
                        }
                        else if (result != CN_TYPE_RETURN_OK)
                        {
                            if (CN_OM_ErrDebug() == TRUE)
                            {
                                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n",
                                    __FUNCTION__, __LINE__, CN_OM_ErrStr(result));
                            }
                            return CN_TYPE_RETURN_OK;
                        }
                    }
                }
            } /* if cp_freed == TRUE */
        } /* else (mode != CN_TYPE_DEFENSE_MODE_AUTO) */
    } /* while SWCTRL_GetNextLogicalPort() == SWCTRL_LPORT_NORMAL_PORT */

    if (warning == TRUE)
    {
        return CN_TYPE_RETURN_ERROR_CP_CAPACITY;
    }
    else
    {
        return CN_TYPE_RETURN_OK;
    }
} /* End of CN_MGR_SetCnpvDefenseMode */

/* FUNCTION NAME - CN_MGR_SetCnpvAlternatePriority
 * PURPOSE : Set the alternate priority used for a CNPV.
 * INPUT   : priority     - the specified CNPV
 *           alt_priority - the specified alternate priority
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : The failures of per-port processes are not considered to be failed
 *           for the function; these failures can be found by checking per-port
 *           oper alternate priority
 */
UI32_T CN_MGR_SetCnpvAlternatePriority(UI32_T priority, UI32_T alt_priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_PriData_T         *pri_data_p;
    CN_OM_PortPriData_T     *port_pri_data_p;
    UI32_T                  result, lport;

    /* BODY
     */

    if (CN_OM_ThreadDebug() == TRUE)
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu, %lu \r\n", __FUNCTION__, priority,
            alt_priority);
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: not master mode \r\n", __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_MASTER_MODE;
    }

    if ((priority < CN_TYPE_MIN_PRIORITY) || (priority > CN_TYPE_MAX_PRIORITY))
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: invalid priority \r\n", __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_PRIORITY_RANGE;
    }

    if (    (alt_priority < CN_TYPE_MIN_PRIORITY)
         || (alt_priority > CN_TYPE_MAX_PRIORITY)
       )
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: invalid alternate priority \r\n",
                __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_ALT_PRIORITY_RANGE;
    }

    pri_data_p = CN_OM_GetPriDataPtr(priority);
    if (pri_data_p->active == FALSE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: cnpv entry is inactive \r\n",
                __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_CNPV_EXISTENCE;
    }

    if (pri_data_p->admin_alt_priority == alt_priority)
    {
        return CN_TYPE_RETURN_OK;
    }

    result = CN_OM_SetPriAlternatePriority(priority, alt_priority, TRUE);
    if (result != CN_TYPE_RETURN_OK)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__, __LINE__,
                CN_OM_ErrStr(result));
        }
        return result;
    }

    /* apply to all ports whose admin alternate priority is global and admin
     * defense mode is not equivalent to auto
     */
    lport = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) == SWCTRL_LPORT_NORMAL_PORT)
    {
        port_pri_data_p = CN_OM_GetPortPriDataPtr(priority, lport);

        if (port_pri_data_p->admin_alt_priority !=
                CN_TYPE_ALTERNATE_PRIORITY_BY_GLOBAL)
        {
            continue;
        }

        if (    (    (port_pri_data_p->admin_defense_mode ==
                        CN_TYPE_DEFENSE_MODE_BY_GLOBAL)
                  && (pri_data_p->defense_mode == CN_TYPE_DEFENSE_MODE_AUTO)
                )
             || (port_pri_data_p->admin_defense_mode == CN_TYPE_DEFENSE_MODE_AUTO)
           )
        {
            continue;
        }

        if (port_pri_data_p->oper_defense_mode == CN_TYPE_DEFENSE_MODE_DISABLED)
        {
            result = CN_OM_SetPortPriAlternatePriority(priority, lport,
                        alt_priority, FALSE);
            if (result != CN_TYPE_RETURN_OK)
            {
                if (CN_OM_ErrDebug() == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                        __LINE__, CN_OM_ErrStr(result));
                }
                return CN_TYPE_RETURN_OK;
            }
        }
        else
        {
            result = CN_MGR_SetPortCnpvOperAlternatePriority(priority, lport,
                        alt_priority);
            if (result != CN_TYPE_RETURN_OK)
            {
                if (CN_OM_ErrDebug() == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                        __LINE__, CN_OM_ErrStr(result));
                }
                return CN_TYPE_RETURN_OK;
            }
        }
    } /* while SWCTRL_GetNextLogicalPort == SWCTRL_LPORT_NORMAL_PORT */

    return CN_TYPE_RETURN_OK;
} /* End of CN_MGR_SetCnpvAlternatePriority */

/* FUNCTION NAME - CN_MGR_SetPortCnpvDefenseMode
 * PURPOSE : Set the defense mode for a CNPV on a port.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 *           mode     - CN_TYPE_DEFENSE_MODE_E
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_MGR_SetPortCnpvDefenseMode(UI32_T priority, UI32_T lport, UI32_T mode)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_PriData_T         *pri_data_p;
    CN_OM_PortPriData_T     *port_pri_data_p;
    CN_OM_CpData_T          *cp_data_p;
    SWCTRL_Lport_Type_T     port_type;
    UI32_T                  unit, port, trunk_id;
    UI32_T                  result, old_mode, i;
    BOOL_T                  cp_freed;

    /* BODY
     */

    if (CN_OM_ThreadDebug() == TRUE)
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu, %lu, %lu \r\n", __FUNCTION__,
            priority, lport, mode);
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: not master mode \r\n", __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_MASTER_MODE;
    }

    if ((priority < CN_TYPE_MIN_PRIORITY) || (priority > CN_TYPE_MAX_PRIORITY))
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: invalid priority \r\n", __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_PRIORITY_RANGE;
    }

    port_type = SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);
    if (port_type != SWCTRL_LPORT_NORMAL_PORT)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: invalid logical port \r\n",
                __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_LPORT_RANGE;
    }

    if (    (mode < CN_TYPE_DEFENSE_MODE_DISABLED)
         || (mode > CN_TYPE_DEFENSE_MODE_BY_GLOBAL)
       )
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: invalid defense mode \r\n",
                __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_MODE_RANGE;
    }

    port_pri_data_p = CN_OM_GetPortPriDataPtr(priority, lport);
    if (port_pri_data_p->active == FALSE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: port cnpv entry is inactive \r\n",
                __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_PORT_CNPV_EXISTENCE;
    }

    old_mode = port_pri_data_p->admin_defense_mode;
    if (old_mode == mode)
    {
        return CN_TYPE_RETURN_OK;
    }

    result = CN_OM_SetPortPriDefenseMode(priority, lport, mode, TRUE);
    if (result != CN_TYPE_RETURN_OK)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__, __LINE__,
                CN_OM_ErrStr(result));
        }
        return result;
    }

    pri_data_p = CN_OM_GetPriDataPtr(priority);
    if (    (mode == CN_TYPE_DEFENSE_MODE_AUTO)
         || (    (mode == CN_TYPE_DEFENSE_MODE_BY_GLOBAL)
              && (pri_data_p->defense_mode == CN_TYPE_DEFENSE_MODE_AUTO)
            )
       )
    {
        result = CN_OM_SetPortPriAlternatePriority(priority, lport,
                    pri_data_p->auto_alt_priority, FALSE);
        if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return CN_TYPE_RETURN_OK;
        }

        result = CN_ENGINE_ProgressStateMachine(priority, lport,
                    CN_ENGINE_EVENT_AUTO_EV);
        if (result == CN_TYPE_RETURN_ERROR_CP_CAPACITY)
        {
            return result;
        }
        else if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return CN_TYPE_RETURN_OK;
        }
    } /* if mode is equivalent to auto */
    else
    {
        if (    (old_mode == CN_TYPE_DEFENSE_MODE_AUTO)
             || (    (old_mode == CN_TYPE_DEFENSE_MODE_BY_GLOBAL)
                  && (pri_data_p->defense_mode == CN_TYPE_DEFENSE_MODE_AUTO)
                )
           )
        {
            if (port_pri_data_p->admin_alt_priority ==
                    CN_TYPE_ALTERNATE_PRIORITY_BY_GLOBAL)
            {
                result = CN_OM_SetPortPriAlternatePriority(priority, lport,
                            pri_data_p->admin_alt_priority, FALSE);
                if (result != CN_TYPE_RETURN_OK)
                {
                    if (CN_OM_ErrDebug() == TRUE)
                    {
                        BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n",
                            __FUNCTION__, __LINE__, CN_OM_ErrStr(result));
                    }
                    return CN_TYPE_RETURN_OK;
                }
            }
            else
            {
                result = CN_OM_SetPortPriAlternatePriority(priority, lport,
                            port_pri_data_p->admin_alt_priority, FALSE);
                if (result != CN_TYPE_RETURN_OK)
                {
                    if (CN_OM_ErrDebug() == TRUE)
                    {
                        BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n",
                            __FUNCTION__, __LINE__, CN_OM_ErrStr(result));
                    }
                    return CN_TYPE_RETURN_OK;
                }
            }
        }

        result = CN_ENGINE_ProgressStateMachine(priority, lport,
                    CN_ENGINE_EVENT_ADMIN_EV);
        if (result == CN_TYPE_RETURN_ERROR_CP_CAPACITY)
        {
            return result;
        }
        else if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return CN_TYPE_RETURN_OK;
        }

        cp_freed = FALSE;
        for (i = 0; i < SYS_ADPT_CN_MAX_NBR_OF_CP_PER_PORT; i++)
        {
            cp_data_p = CN_OM_GetCpDataPtr(lport, i);
            if (cp_data_p->active == FALSE)
            {
                cp_freed = TRUE;
                break;
            }
        }
        if (cp_freed == TRUE)
        {
            for (i = CN_TYPE_MIN_PRIORITY; i <= CN_TYPE_MAX_PRIORITY; i++)
            {
                if (i == priority)
                {
                    continue;
                }

                pri_data_p = CN_OM_GetPriDataPtr(i);
                if (pri_data_p->active == FALSE)
                {
                    continue;
                }

                port_pri_data_p = CN_OM_GetPortPriDataPtr(i, lport);
                if (    (port_pri_data_p->oper_defense_mode ==
                            CN_TYPE_DEFENSE_MODE_DISABLED)
                     && !(    (port_pri_data_p->admin_defense_mode ==
                                CN_TYPE_DEFENSE_MODE_DISABLED)
                           || (    (port_pri_data_p->admin_defense_mode ==
                                        CN_TYPE_DEFENSE_MODE_BY_GLOBAL)
                                && (pri_data_p->defense_mode ==
                                        CN_TYPE_DEFENSE_MODE_DISABLED)
                              )
                         )
                   )
                {
                    result = CN_ENGINE_ProgressStateMachine(i, lport,
                                CN_ENGINE_EVENT_BEGIN);
                    if (result == CN_TYPE_RETURN_ERROR_CP_CAPACITY)
                    {
                        continue;
                    }
                    else if (result != CN_TYPE_RETURN_OK)
                    {
                        if (CN_OM_ErrDebug() == TRUE)
                        {
                            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n",
                                __FUNCTION__, __LINE__, CN_OM_ErrStr(result));
                        }
                        return CN_TYPE_RETURN_OK;
                    }
                }
            }
        } /* if cp_freed == TRUE */
    } /* else (mode is not equivalent to auto) */

    return CN_TYPE_RETURN_OK;
} /* End of CN_MGR_SetPortCnpvDefenseMode */

/* FUNCTION NAME - CN_MGR_SetPortCnpvAlternatePriority
 * PURPOSE : Set the alternate priority used for a CNPV on a port.
 * INPUT   : priority     - the specified CNPV
 *           lport        - the specified logical port
 *           alt_priority - the specified alternate priority
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_MGR_SetPortCnpvAlternatePriority(UI32_T priority, UI32_T lport,
        UI32_T alt_priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_PriData_T         *pri_data_p;
    CN_OM_PortPriData_T     *port_pri_data_p;
    SWCTRL_Lport_Type_T     port_type;
    UI32_T                  unit, port, trunk_id;
    UI32_T                  result;

    /* BODY
     */

    if (CN_OM_ThreadDebug() == TRUE)
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu, %lu, %lu \r\n", __FUNCTION__,
            priority, lport, alt_priority);
    }

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return CN_TYPE_RETURN_ERROR_MASTER_MODE;
    }

    if ((priority < CN_TYPE_MIN_PRIORITY) || (priority > CN_TYPE_MAX_PRIORITY))
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: invalid priority \r\n", __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_PRIORITY_RANGE;
    }

    port_type = SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);
    if (port_type != SWCTRL_LPORT_NORMAL_PORT)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: invalid logical port \r\n",
                __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_LPORT_RANGE;
    }

    if (   (alt_priority != CN_TYPE_ALTERNATE_PRIORITY_BY_GLOBAL)
        && (    (alt_priority < CN_TYPE_MIN_PRIORITY)
             || (alt_priority > CN_TYPE_MAX_PRIORITY)
           )
       )
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: invalid alternate priority \r\n",
                __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_ALT_PRIORITY_RANGE;
    }

    port_pri_data_p = CN_OM_GetPortPriDataPtr(priority, lport);
    if (port_pri_data_p->active == FALSE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: port cnpv entry is inactive \r\n",
                __FUNCTION__);
        }
        return CN_TYPE_RETURN_ERROR_PORT_CNPV_EXISTENCE;
    }

    if (port_pri_data_p->admin_alt_priority == alt_priority)
    {
        return CN_TYPE_RETURN_OK;
    }

    result = CN_OM_SetPortPriAlternatePriority(priority, lport, alt_priority,
                TRUE);
    if (result != CN_TYPE_RETURN_OK)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__, __LINE__,
                CN_OM_ErrStr(result));
        }
        return result;
    }

    pri_data_p = CN_OM_GetPriDataPtr(priority);
    if (    (    (port_pri_data_p->admin_defense_mode !=
                    CN_TYPE_DEFENSE_MODE_BY_GLOBAL)
              && (port_pri_data_p->admin_defense_mode !=
                    CN_TYPE_DEFENSE_MODE_AUTO)
            )
         || (    (port_pri_data_p->admin_defense_mode ==
                    CN_TYPE_DEFENSE_MODE_BY_GLOBAL)
              && (pri_data_p->defense_mode != CN_TYPE_DEFENSE_MODE_AUTO)
            )
       )
    {
        if (alt_priority == CN_TYPE_ALTERNATE_PRIORITY_BY_GLOBAL)
        {
            alt_priority = pri_data_p->admin_alt_priority;
        }

        if (port_pri_data_p->oper_defense_mode == CN_TYPE_DEFENSE_MODE_DISABLED)
        {
            result = CN_OM_SetPortPriAlternatePriority(priority, lport,
                        alt_priority, FALSE);
            if (result != CN_TYPE_RETURN_OK)
            {
                if (CN_OM_ErrDebug() == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                        __LINE__, CN_OM_ErrStr(result));
                }
                return CN_TYPE_RETURN_OK;
            }
        }
        else
        {
            result = CN_MGR_SetPortCnpvOperAlternatePriority(priority, lport,
                        alt_priority);
            if (result != CN_TYPE_RETURN_OK)
            {
                if (CN_OM_ErrDebug() == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                        __LINE__, CN_OM_ErrStr(result));
                }
                return CN_TYPE_RETURN_OK;
            }
        }
    }

    return CN_TYPE_RETURN_OK;
} /* End of CN_MGR_SetPortCnpvAlternatePriority */

/* FUNCTION NAME - CN_MGR_HandleIPCReqMsg
 * PURPOSE : Handle the IPC request message for CN MGR.
 * INPUT   : msgbuf_p - input request ipc message buffer
 * OUTPUT  : msgbuf_p - output response ipc message buffer
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 * NOTES   : None
 */
BOOL_T CN_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_MGR_IpcMsg_T *msg_p;

    /* BODY
     */

    if (CN_OM_ThreadDebug() == TRUE)
    {
        BACKDOOR_MGR_Printf("\r\n %s: %p \r\n", __FUNCTION__, msgbuf_p);
    }

    if (msgbuf_p == NULL)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: null pointer input \r\n", __FUNCTION__);
        }
        return FALSE;
    }

    msgbuf_p->msg_size = CN_MGR_IPCMSG_TYPE_SIZE;
    msg_p = (CN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;

    /* every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s: not master mode \r\n", __FUNCTION__);
        }
        msg_p->type.ret_ui32 = CN_TYPE_RETURN_ERROR;
        return TRUE;
    }

    /* dispatch IPC message and call the corresponding VLAN_MGR function
     */
    switch (msg_p->type.cmd)
    {
        case CN_MGR_IPC_SETGLOBALADMINSTATUS:
        	msg_p->type.ret_ui32 =
                CN_MGR_SetGlobalAdminStatus(msg_p->data.arg_ui32);
            break;

        case CN_MGR_IPC_SETCNMTXPRIORITY:
        	msg_p->type.ret_ui32 = CN_MGR_SetCnmTxPriority(msg_p->data.arg_ui32);
            break;

        case CN_MGR_IPC_SETCNPV:
        	msg_p->type.ret_ui32 = CN_MGR_SetCnpv(
        	    msg_p->data.arg_grp_ui32_bool.arg_ui32,
        	    msg_p->data.arg_grp_ui32_bool.arg_bool);
            break;

        case CN_MGR_IPC_SETCNPVDEFENSEMODE:
        	msg_p->type.ret_ui32 = CN_MGR_SetCnpvDefenseMode(
        	    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1,
        	    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            break;

        case CN_MGR_IPC_SETCNPVALTERNATEPRIORITY:
        	msg_p->type.ret_ui32 = CN_MGR_SetCnpvAlternatePriority(
        	    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1,
        	    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            break;

        case CN_MGR_IPC_SETPORTCNPVDEFENSEMODE:
        	msg_p->type.ret_ui32 = CN_MGR_SetPortCnpvDefenseMode(
        	    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1,
        	    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2,
        	    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            break;

        case CN_MGR_IPC_SETPORTCNPVALTERNATEPRIORITY:
        	msg_p->type.ret_ui32 = CN_MGR_SetPortCnpvAlternatePriority(
        	    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_1,
        	    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_2,
        	    msg_p->data.arg_grp_ui32_ui32_ui32.arg_ui32_3);
            break;

        default:
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s: invalid IPC command \r\n",
                    __FUNCTION__);
            }
            msg_p->type.ret_ui32 = CN_TYPE_RETURN_ERROR;
    }

    return TRUE;
} /* End of CN_MGR_HandleIPCReqMsg */

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME - CN_MGR_ProcessTrunkMemberCallback
 * PURPOSE : Process callback for trunk member addition or deletion.
 * INPUT   : ifindex - ifindex of member port
 *           action  - TRUNK_CALLBACK_ACTION_E
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void CN_MGR_ProcessTrunkMemberCallback(UI32_T ifindex, UI32_T action)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_PriData_T     *pri_data_p;
    CN_OM_PortPriData_T *port_pri_data_p;
    UI32_T              i, result;

    /* BODY
     */

    if (CN_OM_ThreadDebug() == TRUE)
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu, %lu \r\n", __FUNCTION__, ifindex,
            action);
    }

    switch (action)
    {
    case CN_MGR_TRUNK_MEMBER_ADD:
        for (i = CN_TYPE_MIN_PRIORITY; i <= CN_TYPE_MAX_PRIORITY; i++)
        {
            port_pri_data_p = CN_OM_GetPortPriDataPtr(i, ifindex);
            if (port_pri_data_p->oper_defense_mode ==
                    CN_TYPE_DEFENSE_MODE_DISABLED)
            {
                continue;
            }

            result = CN_ENGINE_ResetStateMachine(i, ifindex);
            if (result != CN_TYPE_RETURN_OK)
            {
                if (CN_OM_ErrDebug() == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                        __LINE__, CN_OM_ErrStr(result));
                }
                return;
            }
        }
        CN_OM_DefaultPort(ifindex);
        break;

    case CN_MGR_TRUNK_MEMBER_DELETE:
        for (i = CN_TYPE_MIN_PRIORITY; i <= CN_TYPE_MAX_PRIORITY; i++)
        {
            pri_data_p = CN_OM_GetPriDataPtr(i);
            if (pri_data_p->active == FALSE)
            {
                continue;
            }

            result = CN_OM_SetPortPriStatus(i, ifindex, TRUE);
            if (result != CN_TYPE_RETURN_OK)
            {
                if (CN_OM_ErrDebug() == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                        __LINE__, CN_OM_ErrStr(result));
                }
                return;
            }

            if (pri_data_p->defense_mode == CN_TYPE_DEFENSE_MODE_AUTO)
            {
                result = CN_OM_SetPortPriAlternatePriority(i, ifindex,
                            pri_data_p->auto_alt_priority, FALSE);
                if (result != CN_TYPE_RETURN_OK)
                {
                    if (CN_OM_ErrDebug() == TRUE)
                    {
                        BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n",
                            __FUNCTION__, __LINE__, CN_OM_ErrStr(result));
                    }
                    return;
                }
            } /* if pri_data_p->defense_mode == CN_TYPE_DEFENSE_MODE_AUTO */
            else
            {
                result = CN_OM_SetPortPriAlternatePriority(i, ifindex,
                            pri_data_p->admin_alt_priority, FALSE);
                if (result != CN_TYPE_RETURN_OK)
                {
                    if (CN_OM_ErrDebug() == TRUE)
                    {
                        BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n",
                            __FUNCTION__, __LINE__, CN_OM_ErrStr(result));
                    }
                    return;
                }
            } /* else (pri_data_p->defense_mode != CN_TYPE_DEFENSE_MODE_AUTO) */

            if (pri_data_p->defense_mode == CN_TYPE_DEFENSE_MODE_DISABLED)
            {
                continue;
            }

            result = CN_ENGINE_ProgressStateMachine(i, ifindex,
                        CN_ENGINE_EVENT_BEGIN);
            if (result != CN_TYPE_RETURN_OK)
            {
                if (CN_OM_ErrDebug() == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                        __LINE__, CN_OM_ErrStr(result));
                }
                return;
            }
        } /* for i <= CN_TYPE_MAX_PRIORITY */
        break;

    default:
        return;
    }

    return;
} /* End of CN_MGR_ProcessTrunkMemberCallback */

/* FUNCTION NAME - CN_MGR_SetGlobalStatus
 * PURPOSE : Set CN global status.
 * INPUT   : status - CN_TYPE_GLOBAL_STATUS_ENABLE /
 *                    CN_TYPE_GLOBAL_STATUS_DISABLE
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : Only PFC/ETS enabled operation prevent global oper status from
 *           being enabled; per-port failures do not affect the success of
 *           oper status change
 */
static UI32_T CN_MGR_SetGlobalStatus(UI32_T status)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

#if (SYS_CPNT_PFC == TRUE)
    BOOL_T                  pfc_enable;
#endif
    CN_OM_PriData_T         *pri_data_p;
    CN_OM_PortPriData_T     *port_pri_data_p;
    UI32_T                  result, i, lport;
    BOOL_T                  warning;

    /* BODY
     */

    if (CN_OM_ThreadDebug() == TRUE)
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu \r\n", __FUNCTION__, status);
    }

#if (SYS_CPNT_PFC == TRUE)
    if (PFC_OM_GetDataByField(0, PFC_TYPE_FLDE_GLOBAL_IS_ANY_PORT_OP_EN,
            &pfc_enable) == FALSE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: PFC_OM_GetDataByField failed \r\n",
                __FUNCTION__, __LINE__);
        }
    }
    if ((pfc_enable == TRUE) && (status == CN_TYPE_GLOBAL_STATUS_ENABLE))
    {
        return CN_TYPE_RETURN_ERROR_PFC_CONFLICT;
    }
#endif /* #if (SYS_CPNT_PFC == TRUE) */

#if (SYS_CPNT_ETS == TRUE)
    if (    (ETS_OM_IsAnyPortOperEnabled() == TRUE)
         && (status == CN_TYPE_GLOBAL_STATUS_ENABLE)
       )
    {
        return CN_TYPE_RETURN_ERROR_ETS_CONFLICT;
    }
#endif /* #if (SYS_CPNT_ETS == TRUE) */

    result = CN_OM_SetGlobalOperStatus(status);
    if (result != CN_TYPE_RETURN_OK)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__, __LINE__,
                CN_OM_ErrStr(result));
        }
        return result;
    }

    warning = FALSE;
    for (i = CN_TYPE_MIN_PRIORITY; i <= CN_TYPE_MAX_PRIORITY; i++)
    {
        pri_data_p = CN_OM_GetPriDataPtr(i);
        if (pri_data_p->active == FALSE)
        {
            continue;
        }

        lport = 0;
        while (SWCTRL_GetNextLogicalPort(&lport) == SWCTRL_LPORT_NORMAL_PORT)
        {
            port_pri_data_p = CN_OM_GetPortPriDataPtr(i, lport);
            if (port_pri_data_p->admin_defense_mode ==
                    CN_TYPE_DEFENSE_MODE_DISABLED)
            {
                continue;
            }
            if ((port_pri_data_p->admin_defense_mode ==
                    CN_TYPE_DEFENSE_MODE_BY_GLOBAL) &&
                (pri_data_p->defense_mode == CN_TYPE_DEFENSE_MODE_DISABLED))
            {
                continue;
            }

            if (status == CN_TYPE_GLOBAL_STATUS_ENABLE)
            {
                result = CN_ENGINE_ProgressStateMachine(i, lport,
                            CN_ENGINE_EVENT_BEGIN);
                if (result == CN_TYPE_RETURN_ERROR_CP_CAPACITY)
                {
                    warning = TRUE;
                    continue;
                }
                else if (result != CN_TYPE_RETURN_OK)
                {
                    if (CN_OM_ErrDebug() == TRUE)
                    {
                        BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n",
                            __FUNCTION__, __LINE__, CN_OM_ErrStr(result));
                    }
                    return result;
                }
            }
            else
            {
                result = CN_ENGINE_ResetStateMachine(i, lport);
                if (result != CN_TYPE_RETURN_OK)
                {
                    if (CN_OM_ErrDebug() == TRUE)
                    {
                        BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n",
                            __FUNCTION__, __LINE__, CN_OM_ErrStr(result));
                    }
                    return result;
                }
            }
        }
    }

    if (warning == TRUE)
    {
        return CN_TYPE_RETURN_ERROR_CP_CAPACITY;
    }
    else
    {
        return CN_TYPE_RETURN_OK;
    }
} /* End of CN_MGR_SetGlobalStatus */

/* FUNCTION NAME - CN_MGR_SetPortCnpvOperAlternatePriority
 * PURPOSE : Set oper alternate priority for a port cnpv entry.
 * INPUT   : priority     - CNPV
 *           lport        - logical port
 *           alt_priority - alternate priority
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : This function sets both om and chip
 */
static UI32_T CN_MGR_SetPortCnpvOperAlternatePriority(UI32_T priority,
                UI32_T lport, UI32_T alt_priority)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T phb, color;

    /* BODY
     */

    if ((CN_OM_ThreadDebug() == TRUE) && (lport == 1))
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu, %lu, %lu \r\n", __FUNCTION__,
            priority, lport, alt_priority);
    }

    if (L4_PMGR_QOS_GetPortPriorityIngressCos2Dscp(lport, COS_VM_PRIORITY_USER,
            priority, 0, &phb, &color) == FALSE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: "
                "L4_PMGR_QOS_GetPortPriorityIngressCos2Dscp"
                "(%lu, COS_VM_PRIORITY_USER, %lu, ...) failed \r\n",
                __FUNCTION__, __LINE__, lport, priority);
        }
        return CN_TYPE_RETURN_ERROR;
    }

    if (L4_PMGR_QOS_SetPortPriorityIngressCos2Dscp(lport, COS_VM_PRIORITY_CN,
            priority, 0, alt_priority, color) == FALSE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: "
                "L4_PMGR_QOS_SetPortPriorityIngressCos2Dscp"
                "(%lu, COS_VM_PRIORITY_CN, %lu, 0, %lu, %lu) failed \r\n",
                __FUNCTION__, __LINE__, lport, priority, alt_priority, color);
        }
        return CN_TYPE_RETURN_ERROR;
    }

    return CN_OM_SetPortPriAlternatePriority(priority, lport, alt_priority, FALSE);
} /* End of CN_MGR_SetPortCnpvOperAlternatePriority */

#endif /* (SYS_CPNT_CN == TRUE) */
