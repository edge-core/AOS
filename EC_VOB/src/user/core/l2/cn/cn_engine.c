/* MODULE NAME - CN_ENGINE.C
 * PURPOSE : Provides the definitions for CN stat machine.
 * NOTES   : None.
 * HISTORY : 2012/09/12 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2012
 */

/* INCLUDE FILE DECLARATIONS
 */

#include "sys_cpnt.h"
#if (SYS_CPNT_CN == TRUE)
#include <string.h>
#include "sys_type.h"
#include "backdoor_mgr.h"
#include "cn_engine.h"
#include "cn_om.h"
#include "cn_om_private.h"
#include "cn_type.h"
#include "l4_pmgr.h"
#include "cos_vm.h"
#include "swctrl_pmgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

enum
{
    CN_ENGINE_STATE_EDGE,
    CN_ENGINE_STATE_INTERIOR,
    CN_ENGINE_STATE_INTERIOR_READY,
    CN_ENGINE_STATE_ADMIN,
    CN_ENGINE_STATE_RESET,
    CN_ENGINE_STATE_UNKNOWN,
};

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    UI32_T  current_state;
    UI32_T  next_state;
} CN_ENGINE_StateMachineEntry_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

static UI32_T CN_ENGINE_TurnOnCnDefenses(UI32_T priority, UI32_T lport);
static UI32_T CN_ENGINE_TurnOffCnDefenses(UI32_T priority, UI32_T lport);
static UI32_T CN_ENGINE_ProcessAdminActions(UI32_T priority, UI32_T lport);
static UI32_T CN_ENGINE_ProcessDisabledActions(UI32_T priority, UI32_T lport);
static UI32_T CN_ENGINE_ProcessEdgeActions(UI32_T priority, UI32_T lport);
static UI32_T CN_ENGINE_ProcessInteriorActions(UI32_T priority, UI32_T lport);
static UI32_T CN_ENGINE_ProcessInteriorReadyActions(UI32_T priority,
                UI32_T lport);
static UI32_T CN_ENGINE_AssociateCp(UI32_T priority, UI32_T lport);
static UI32_T CN_ENGINE_DissociateCp(UI32_T priority, UI32_T lport);
static UI32_T CN_ENGINE_RegisterToL4(UI32_T lport);
static UI32_T CN_ENGINE_DeregisterFromL4(UI32_T lport);

/* STATIC VARIABLE DEFINITIONS
 */

static CN_ENGINE_StateMachineEntry_T cn_state_machines[CN_TYPE_MAX_PRIORITY+1][SYS_ADPT_TOTAL_NBR_OF_LPORT];

/*---------------------------------------------------------------------------------------
 * CURRENT \ EVENT | RCV_CNPV | NOT_RCV_CNPV | RCV_READY | NOT_RCV_READY | ADMIN | AUTO  |
 *---------------------------------------------------------------------------------------
 * EDGE            | INT      | EDGE         | EDGE      | EDGE          | ADMIN | EDGE  |
 * INTERIOR        | INT      | EDGE         | INT_R     | INT           | ADMIN | INT   |
 * INTERIOR_READY  | INT_R    | EDGE         | INT_R     | INT           | ADMIN | INT_R |
 * ADMIN           | ADMIN    | ADMIN        | ADMIN     | ADMIN         | ADMIN | RESET |
 *---------------------------------------------------------------------------------------
 */
static UI32_T state_transition_table[CN_ENGINE_STATE_ADMIN+1][CN_ENGINE_EVENT_AUTO_EV+1] =
{
    {CN_ENGINE_STATE_INTERIOR,       CN_ENGINE_STATE_EDGE,  CN_ENGINE_STATE_EDGE,           CN_ENGINE_STATE_EDGE,     CN_ENGINE_STATE_ADMIN, CN_ENGINE_STATE_EDGE},
    {CN_ENGINE_STATE_INTERIOR,       CN_ENGINE_STATE_EDGE,  CN_ENGINE_STATE_INTERIOR_READY, CN_ENGINE_STATE_INTERIOR, CN_ENGINE_STATE_ADMIN, CN_ENGINE_STATE_INTERIOR},
    {CN_ENGINE_STATE_INTERIOR_READY, CN_ENGINE_STATE_EDGE,  CN_ENGINE_STATE_INTERIOR_READY, CN_ENGINE_STATE_INTERIOR, CN_ENGINE_STATE_ADMIN, CN_ENGINE_STATE_INTERIOR_READY},
    {CN_ENGINE_STATE_ADMIN,          CN_ENGINE_STATE_ADMIN, CN_ENGINE_STATE_ADMIN,          CN_ENGINE_STATE_ADMIN,    CN_ENGINE_STATE_ADMIN, CN_ENGINE_STATE_RESET},
};

static UI8_T cn_tag_removal_bitmap[SYS_ADPT_TOTAL_NBR_OF_LPORT];
static UI32_T l4_register_count[SYS_ADPT_TOTAL_NBR_OF_LPORT];

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME - CN_ENGINE_InitiateProcessResources
 * PURPOSE : Initiate Process Resources.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void CN_ENGINE_InitiateProcessResources(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  i, j;

    /* BODY
     */

    memset(cn_state_machines, 0, sizeof(cn_state_machines));
    for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
    {
        for (j = 0; j <= CN_TYPE_MAX_PRIORITY; j++)
        {
            cn_state_machines[j][i].current_state =
                cn_state_machines[j][i].next_state = CN_ENGINE_STATE_UNKNOWN;
        }

        cn_tag_removal_bitmap[i] = 0;
        l4_register_count[i] = 0;
    }

    return;
} /* End of CN_ENGINE_InitiateProcessResources */

/* FUNCTION NAME - CN_ENGINE_ProgressStateMachine
 * PURPOSE : Progress one state machine given an event.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 *           event    - CN_ENGINE_EVENT_E
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_ENGINE_ProgressStateMachine(UI32_T priority, UI32_T lport, UI32_T event)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_GlobalData_T      *global_data_p;
    CN_OM_PriData_T         *pri_data_p;
    CN_OM_PortPriData_T     *port_pri_data_p;
    UI32_T                  result;

    /* BODY
     */

    if ((CN_OM_ThreadDebug() == TRUE) && (lport == 1))
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu, %lu, %lu \r\n", __FUNCTION__,
            priority, lport, event);
    }

    global_data_p = CN_OM_GetGlobalDataPtr();
    if (global_data_p->admin_status == CN_TYPE_GLOBAL_STATUS_DISABLE)
    {
        return CN_TYPE_RETURN_OK;
    }

    pri_data_p = CN_OM_GetPriDataPtr(priority);
    port_pri_data_p = CN_OM_GetPortPriDataPtr(priority, lport);

    if (event == CN_ENGINE_EVENT_BEGIN)
    {
        if (port_pri_data_p->admin_defense_mode ==
                CN_TYPE_DEFENSE_MODE_BY_GLOBAL)
        {
            if (pri_data_p->defense_mode == CN_TYPE_DEFENSE_MODE_AUTO)
            {
                cn_state_machines[priority][lport-1].next_state =
                    CN_ENGINE_STATE_RESET;
            }
            else
            {
                cn_state_machines[priority][lport-1].next_state =
                    CN_ENGINE_STATE_ADMIN;
            }
        }
        else
        {
            if (port_pri_data_p->admin_defense_mode ==
                    CN_TYPE_DEFENSE_MODE_AUTO)
            {
                cn_state_machines[priority][lport-1].next_state =
                    CN_ENGINE_STATE_RESET;
            }
            else
            {
                cn_state_machines[priority][lport-1].next_state =
                    CN_ENGINE_STATE_ADMIN;
            }
        }
    }
    else
    {
        cn_state_machines[priority][lport-1].next_state = state_transition_table
            [cn_state_machines[priority][lport-1].current_state][event];

        if ((cn_state_machines[priority][lport-1].current_state !=
                CN_ENGINE_STATE_ADMIN) &&
            (cn_state_machines[priority][lport-1].next_state ==
                cn_state_machines[priority][lport-1].current_state))
        {
            return CN_TYPE_RETURN_OK;
        }
    }

    switch (cn_state_machines[priority][lport-1].next_state)
    {
    case CN_ENGINE_STATE_RESET:
        /* reset actions */
        if (port_pri_data_p->oper_defense_mode != CN_TYPE_DEFENSE_MODE_DISABLED)
        {
            result = CN_ENGINE_ProcessDisabledActions(priority, lport);
            if (result != CN_TYPE_RETURN_OK)
            {
                if (CN_OM_ErrDebug() == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                        __LINE__, CN_OM_ErrStr(result));
                }
                return result;
            }
        }
        cn_state_machines[priority][lport-1].current_state =
            CN_ENGINE_STATE_RESET;
        /* UCT to edge actions */
        result = CN_ENGINE_AssociateCp(priority, lport);
        if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return result;
        }
        result = CN_ENGINE_RegisterToL4(lport);
        if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return result;
        }
        result = CN_ENGINE_ProcessEdgeActions(priority, lport);
        if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return result;
        }
        cn_state_machines[priority][lport-1].current_state =
            CN_ENGINE_STATE_EDGE;
        break;

    case CN_ENGINE_STATE_EDGE:
        result = CN_ENGINE_ProcessEdgeActions(priority, lport);
        if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return result;
        }
        cn_state_machines[priority][lport-1].current_state =
            CN_ENGINE_STATE_EDGE;
        break;

    case CN_ENGINE_STATE_INTERIOR:
        result = CN_ENGINE_ProcessInteriorActions(priority, lport);
        if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return result;
        }
        cn_state_machines[priority][lport-1].current_state =
            CN_ENGINE_STATE_INTERIOR;
        break;

    case CN_ENGINE_STATE_INTERIOR_READY:
        result = CN_ENGINE_ProcessInteriorReadyActions(priority, lport);
        if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return result;
        }
        cn_state_machines[priority][lport-1].current_state =
            CN_ENGINE_STATE_INTERIOR_READY;
        break;

    case CN_ENGINE_STATE_ADMIN:
        result = CN_ENGINE_ProcessAdminActions(priority, lport);
        if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return result;
        }
        cn_state_machines[priority][lport-1].current_state =
            CN_ENGINE_STATE_ADMIN;
        break;
    } /* end of switch */

    return CN_TYPE_RETURN_OK;
} /* End of CN_ENGINE_ProgressStateMachine */

/* FUNCTION NAME - CN_ENGINE_ResetStateMachine
 * PURPOSE : Reset one state machine.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
UI32_T CN_ENGINE_ResetStateMachine(UI32_T priority, UI32_T lport)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_PortPriData_T     *port_pri_data_p;
    UI32_T                  result;

    /* BODY
     */

    if ((CN_OM_ThreadDebug() == TRUE) && (lport == 1))
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu, %lu \r\n", __FUNCTION__, priority,
            lport);
    }

    port_pri_data_p = CN_OM_GetPortPriDataPtr(priority, lport);
    switch (port_pri_data_p->oper_defense_mode)
    {
    case CN_TYPE_DEFENSE_MODE_INTERIOR:
    case CN_TYPE_DEFENSE_MODE_INTERIOR_READY:
        result = CN_OM_SetTxReady(priority, lport, FALSE);
        if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return result;
        }
    case CN_TYPE_DEFENSE_MODE_EDGE:
        result = CN_ENGINE_DissociateCp(priority, lport);
        if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return result;
        }
        /* the standard action here is DisableCnpvRemapping() */
        result = CN_ENGINE_DeregisterFromL4(lport);
        if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return result;
        }
        result = CN_OM_SetPortPriDefenseMode(priority, lport,
                    CN_TYPE_DEFENSE_MODE_DISABLED, FALSE);
        if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return result;
        }
        break;
    }

    if (SWCTRL_PMGR_SetPortQcnEgrCnTagRemoval(lport, 0) == FALSE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: "
                "SWCTRL_PMGR_SetPortQcnEgrCnTagRemoval(%lu, 0) failed \r\n",
                __FUNCTION__, __LINE__, lport);
        }
        return CN_TYPE_RETURN_ERROR;
    }

    cn_state_machines[priority][lport-1].current_state =
        cn_state_machines[priority][lport-1].next_state =
            CN_ENGINE_STATE_UNKNOWN;

    return CN_TYPE_RETURN_OK;
} /* End of CN_ENGINE_ResetStateMachine */

/* FUNCTION NAME - CN_ENGINE_CnTagRemovalBitmap
 * PURPOSE : Get the CN_TAG removal bitmap for a lport.
 * INPUT   : lport - the specified logical port
 * OUTPUT  : None
 * RETURN  : CN_TAG removal bitmap (each bit for a priority)
 * NOTES   : Only used by CN backdoor
 */
UI8_T CN_ENGINE_CnTagRemovalBitmap(UI32_T lport)
{
    return cn_tag_removal_bitmap[lport-1];
} /* End of CN_ENGINE_CnTagRemovalBitmap */

/* FUNCTION NAME - CN_ENGINE_L4RegisterCount
 * PURPOSE : Get the L4 register count for a lport.
 * INPUT   : lport - the specified logical port
 * OUTPUT  : None
 * RETURN  : L4 register count
 * NOTES   : Only used by CN backdoor
 */
UI32_T CN_ENGINE_L4RegisterCount(UI32_T lport)
{
    return l4_register_count[lport-1];
} /* End of CN_ENGINE_L4RegisterCount */

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME - CN_ENGINE_TurnOnCnDefenses
 * PURPOSE : Override priority regeneration table to use alternate priority for
 *           a CNPV.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
static UI32_T CN_ENGINE_TurnOnCnDefenses(UI32_T priority, UI32_T lport)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_PortPriData_T     *port_pri_data_p;
    UI32_T                  phb, color;

    /* BODY
     */

    if ((CN_OM_ThreadDebug() == TRUE) && (lport == 1))
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu, %lu \r\n", __FUNCTION__, priority,
            lport);
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

    port_pri_data_p = CN_OM_GetPortPriDataPtr(priority, lport);
    if (L4_PMGR_QOS_SetPortPriorityIngressCos2Dscp(lport, COS_VM_PRIORITY_CN,
            priority, 0, port_pri_data_p->oper_alt_priority, color) == FALSE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: "
                "L4_PMGR_QOS_SetPortPriorityIngressCos2Dscp(%lu, ...) failed "
                "\r\n", __FUNCTION__, __LINE__, lport);
        }
        return CN_TYPE_RETURN_ERROR;
    }

    return CN_TYPE_RETURN_OK;
} /* End of CN_ENGINE_TurnOnCnDefenses */

/* FUNCTION NAME - CN_ENGINE_TurnOffCnDefenses
 * PURPOSE : Override priority regeneration table to not alter priority for a
 *           CNPV.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
static UI32_T CN_ENGINE_TurnOffCnDefenses(UI32_T priority, UI32_T lport)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  phb, color;

    /* BODY
     */

    if ((CN_OM_ThreadDebug() == TRUE) && (lport == 1))
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu, %lu \r\n", __FUNCTION__, priority,
            lport);
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
        priority, 0, priority, color) == FALSE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: "
                "L4_PMGR_QOS_SetPortPriorityIngressCos2Dscp(%lu, ...) failed "
                "\r\n", __FUNCTION__, __LINE__, lport);
        }
        return CN_TYPE_RETURN_ERROR;
    }

    return CN_TYPE_RETURN_OK;
} /* End of CN_ENGINE_TurnOffCnDefenses */

/* FUNCTION NAME - CN_ENGINE_ProcessAdminActions
 * PURPOSE : Process actions when transition to ADMIN state.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
static UI32_T CN_ENGINE_ProcessAdminActions(UI32_T priority, UI32_T lport)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_PriData_T         *pri_data_p;
    CN_OM_PortPriData_T     *port_pri_data_p;
    UI32_T                  admin_mode;
    UI32_T                  result;

    /* BODY
     */

    if ((CN_OM_ThreadDebug() == TRUE) && (lport == 1))
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu, %lu \r\n", __FUNCTION__, priority,
            lport);
    }

    pri_data_p = CN_OM_GetPriDataPtr(priority);
    port_pri_data_p = CN_OM_GetPortPriDataPtr(priority, lport);

    if (port_pri_data_p->admin_defense_mode == CN_TYPE_DEFENSE_MODE_BY_GLOBAL)
    {
        admin_mode = pri_data_p->defense_mode;
    }
    else
    {
        admin_mode = port_pri_data_p->admin_defense_mode;
    }

    if (    (admin_mode != CN_TYPE_DEFENSE_MODE_DISABLED)
         && (port_pri_data_p->oper_defense_mode == CN_TYPE_DEFENSE_MODE_DISABLED)
       )
    {
        result = CN_ENGINE_AssociateCp(priority, lport);
        if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return result;
        }

        result = CN_ENGINE_RegisterToL4(lport);
        if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return result;
        }
    }
    else if (    (admin_mode == CN_TYPE_DEFENSE_MODE_DISABLED)
              && (port_pri_data_p->oper_defense_mode !=
                    CN_TYPE_DEFENSE_MODE_DISABLED)
            )
    {
        result = CN_ENGINE_DissociateCp(priority, lport);
        if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return result;
        }
    }

    switch (admin_mode)
    {
    case CN_TYPE_DEFENSE_MODE_DISABLED:
        if (port_pri_data_p->oper_defense_mode != CN_TYPE_DEFENSE_MODE_DISABLED)
        {
            result = CN_ENGINE_ProcessDisabledActions(priority, lport);
            if (result != CN_TYPE_RETURN_OK)
            {
                if (CN_OM_ErrDebug() == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                        __LINE__, CN_OM_ErrStr(result));
                }
                return result;
            }
        }
        break;

    case CN_TYPE_DEFENSE_MODE_EDGE:
        result = CN_ENGINE_ProcessEdgeActions(priority, lport);
        if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return result;
        }
        break;

    case CN_TYPE_DEFENSE_MODE_INTERIOR:
        result = CN_ENGINE_ProcessInteriorActions(priority, lport);
        if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return result;
        }
        break;

    case CN_TYPE_DEFENSE_MODE_INTERIOR_READY:
        result = CN_ENGINE_ProcessInteriorReadyActions(priority, lport);
        if (result != CN_TYPE_RETURN_OK)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__,
                    __LINE__, CN_OM_ErrStr(result));
            }
            return result;
        }
        break;

    default:
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: invalid mode case \r\n",
                __FUNCTION__, __LINE__);
        }
        return CN_TYPE_RETURN_ERROR;
    }

    return CN_TYPE_RETURN_OK;
} /* End of CN_ENGINE_ProcessAdminActions */

/* FUNCTION NAME - CN_ENGINE_ProcessDisabledActions
 * PURPOSE : Process actions when transition to DISABLED state.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
static UI32_T CN_ENGINE_ProcessDisabledActions(UI32_T priority, UI32_T lport)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  result;

    /* BODY
     */

    /* the standard action here is DisableCnpvRemapping() */
    result = CN_ENGINE_DeregisterFromL4(lport);
    if (result != CN_TYPE_RETURN_OK)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__, __LINE__,
                CN_OM_ErrStr(result));
        }
        return result;
    }

    result = CN_OM_SetTxReady(priority, lport, FALSE);
    if (result != CN_TYPE_RETURN_OK)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__, __LINE__,
                CN_OM_ErrStr(result));
        }
        return result;
    }

    result = CN_OM_SetPortPriDefenseMode(priority, lport,
                CN_TYPE_DEFENSE_MODE_DISABLED, FALSE);
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
} /* End of CN_ENGINE_ProcessDisabledActions */

/* FUNCTION NAME - CN_ENGINE_ProcessEdgeActions
 * PURPOSE : Process actions when transition to EDGE state.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
static UI32_T CN_ENGINE_ProcessEdgeActions(UI32_T priority, UI32_T lport)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  result;

    /* BODY
     */

    result = CN_ENGINE_TurnOnCnDefenses(priority, lport);
    if (result != CN_TYPE_RETURN_OK)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__, __LINE__,
                CN_OM_ErrStr(result));
        }
        return result;
    }

    result = CN_OM_SetTxReady(priority, lport, FALSE);
    if (result != CN_TYPE_RETURN_OK)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__, __LINE__,
                CN_OM_ErrStr(result));
        }
        return result;
    }

    cn_tag_removal_bitmap[lport-1] |= (1 << priority);
    if (SWCTRL_PMGR_SetPortQcnEgrCnTagRemoval(lport,
            cn_tag_removal_bitmap[lport-1]) == FALSE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: "
                "SWCTRL_PMGR_SetPortQcnEgrCnTagRemoval(%lu, %02X) failed \r\n",
                __FUNCTION__, __LINE__, lport, cn_tag_removal_bitmap[lport-1]);
        }
        cn_tag_removal_bitmap[lport-1] &= ~(1 << priority);
        return CN_TYPE_RETURN_ERROR;
    }

    result = CN_OM_SetPortPriDefenseMode(priority, lport,
                CN_TYPE_DEFENSE_MODE_EDGE, FALSE);
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
} /* End of CN_ENGINE_ProcessEdgeActions */

/* FUNCTION NAME - CN_ENGINE_ProcessInteriorActions
 * PURPOSE : Process actions when transition to INTERIOR state.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
static UI32_T CN_ENGINE_ProcessInteriorActions(UI32_T priority, UI32_T lport)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  result;

    /* BODY
     */

    result = CN_ENGINE_TurnOffCnDefenses(priority, lport);
    if (result != CN_TYPE_RETURN_OK)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__, __LINE__,
                CN_OM_ErrStr(result));
        }
        return result;
    }

    result = CN_OM_SetTxReady(priority, lport, TRUE);
    if (result != CN_TYPE_RETURN_OK)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__, __LINE__,
                CN_OM_ErrStr(result));
        }
        return result;
    }

    cn_tag_removal_bitmap[lport-1] |= (1 << priority);
    if (SWCTRL_PMGR_SetPortQcnEgrCnTagRemoval(lport,
            cn_tag_removal_bitmap[lport-1]) == FALSE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: "
                "SWCTRL_PMGR_SetPortQcnEgrCnTagRemoval(%lu, %02X) failed \r\n",
                __FUNCTION__, __LINE__, lport, cn_tag_removal_bitmap[lport-1]);
        }
        cn_tag_removal_bitmap[lport-1] &= ~(1 << priority);
        return CN_TYPE_RETURN_ERROR;
    }

    result = CN_OM_SetPortPriDefenseMode(priority, lport,
                CN_TYPE_DEFENSE_MODE_INTERIOR, FALSE);
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
} /* End of CN_ENGINE_ProcessInteriorActions */

/* FUNCTION NAME - CN_ENGINE_ProcessInteriorReadyActions
 * PURPOSE : Process actions when transition to INTERIOR-READY state.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
static UI32_T CN_ENGINE_ProcessInteriorReadyActions(UI32_T priority,
                UI32_T lport)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  result;

    /* BODY
     */

    result = CN_ENGINE_TurnOffCnDefenses(priority, lport);
    if (result != CN_TYPE_RETURN_OK)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__, __LINE__,
                CN_OM_ErrStr(result));
        }
        return result;
    }

    result = CN_OM_SetTxReady(priority, lport, TRUE);
    if (result != CN_TYPE_RETURN_OK)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: [%s] \r\n", __FUNCTION__, __LINE__,
                CN_OM_ErrStr(result));
        }
        return result;
    }

    cn_tag_removal_bitmap[lport-1] &= ~(1 << priority);
    if (SWCTRL_PMGR_SetPortQcnEgrCnTagRemoval(lport,
            cn_tag_removal_bitmap[lport-1]) == FALSE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: "
                "SWCTRL_PMGR_SetPortQcnEgrCnTagRemoval(%lu, %02X) failed \r\n",
                __FUNCTION__, __LINE__, lport, cn_tag_removal_bitmap[lport-1]);
        }
        cn_tag_removal_bitmap[lport-1] |= (1 << priority);
        return CN_TYPE_RETURN_ERROR;
    }

    result = CN_OM_SetPortPriDefenseMode(priority, lport,
                CN_TYPE_DEFENSE_MODE_INTERIOR_READY, FALSE);
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
} /* End of CN_ENGINE_ProcessInteriorReadyActions */

/* FUNCTION NAME - CN_ENGINE_AssociateCp
 * PURPOSE : Associate a CNPV with a CP on a logical port.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
static UI32_T CN_ENGINE_AssociateCp(UI32_T priority, UI32_T lport)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_CpData_T      *cp_data_p;
    UI32_T              i;
    UI8_T               tmp_cnpvs, queue_map[MAX_PHB_VAL+1];

    /* BODY
     */

    if ((CN_OM_ThreadDebug() == TRUE) && (lport == 1))
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu, %lu \r\n", __FUNCTION__, priority,
            lport);
    }

#if (SYS_CPNT_COS_INTER_DSCP_TO_QUEUE_PER_PORT == TRUE)
    if (L4_PMGR_QOS_GetPortIngressDscp2Queue(lport, queue_map) == FALSE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: L4_PMGR_QOS_GetIngressDscp2Queue"
                "(%lu, ...) failed \r\n", __FUNCTION__, __LINE__, lport);
        }
        return CN_TYPE_RETURN_ERROR;
    }
#else
    if (L4_PMGR_QOS_GetIngressDscp2Queue(queue_map) == FALSE)
    {
        if (CN_OM_ErrDebug() == TRUE)
        {
            BACKDOOR_MGR_Printf("\r\n %s,%d: L4_PMGR_QOS_GetIngressDscp2Queue"
                "(%lu, ...) failed \r\n", __FUNCTION__, __LINE__, lport);
        }
        return CN_TYPE_RETURN_ERROR;
    }
#endif /* #if (SYS_CPNT_COS_INTER_DSCP_TO_QUEUE_PER_PORT == TRUE) */

    /* check existant CPQ */
    for (i = 0; i < SYS_ADPT_CN_MAX_NBR_OF_CP_PER_PORT; i++)
    {
        cp_data_p = CN_OM_GetCpDataPtr(lport, i);
        if ((cp_data_p->active == TRUE) &&
            (cp_data_p->queue == queue_map[priority]))
        {
            tmp_cnpvs = cp_data_p->managed_cnpvs | (1 << (7 - priority));
            if (CN_OM_SetCpManagedCnpvs(lport, i, tmp_cnpvs) != CN_TYPE_RETURN_OK)
            {
                if (CN_OM_ErrDebug() == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: CN_OM_SetCpManagedCnpvs"
                        "(%lu, %lu, %02X) failed \r\n", __FUNCTION__, __LINE__,
                        lport, i, tmp_cnpvs);
                }
                return CN_TYPE_RETURN_ERROR;
            }

            return CN_TYPE_RETURN_OK;
        }
    }

    /* find/create new CPQ */
    for (i = 0; i < SYS_ADPT_CN_MAX_NBR_OF_CP_PER_PORT; i++)
    {
        cp_data_p = CN_OM_GetCpDataPtr(lport, i);
        if (cp_data_p->active == FALSE)
        {
            if (SWCTRL_PMGR_SetPortQcnCpq(lport, queue_map[priority], i) == FALSE)
            {
                if (CN_OM_ErrDebug() == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: SWCTRL_PMGR_SetPortQcnCpq"
                        "(%lu, %hu, %lu) failed \r\n", __FUNCTION__, __LINE__,
                        lport, queue_map[priority], i);
                }
                return CN_TYPE_RETURN_ERROR;
            }

            if (CN_OM_SetCpStatus(lport, i, TRUE) != CN_TYPE_RETURN_OK)
            {
                if (CN_OM_ErrDebug() == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: CN_OM_SetCpStatus"
                        "(%lu, %lu, TRUE) failed \r\n", __FUNCTION__, __LINE__,
                        lport, i);
                }
                return CN_TYPE_RETURN_ERROR;
            }

            if (CN_OM_SetCpMappedQueue(lport, i, queue_map[priority]) !=
                    CN_TYPE_RETURN_OK)
            {
                if (CN_OM_ErrDebug() == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: CN_OM_SetCpMappedQueue"
                        "(%lu, %lu, %hu) failed \r\n", __FUNCTION__, __LINE__,
                        lport, i, queue_map[priority]);
                }
                return CN_TYPE_RETURN_ERROR;
            }

            tmp_cnpvs = cp_data_p->managed_cnpvs | (1 << (7 - priority));
            if (CN_OM_SetCpManagedCnpvs(lport, i, tmp_cnpvs) != CN_TYPE_RETURN_OK)
            {
                if (CN_OM_ErrDebug() == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: CN_OM_SetCpManagedCnpvs"
                        "(%lu, %lu, %02X) failed \r\n", __FUNCTION__, __LINE__,
                        lport, i, tmp_cnpvs);
                }
                return CN_TYPE_RETURN_ERROR;
            }

            return CN_TYPE_RETURN_OK;
        }
    }

    return CN_TYPE_RETURN_ERROR_CP_CAPACITY;
} /* End of CN_ENGINE_AssociateCp */

/* FUNCTION NAME - CN_ENGINE_DissociateCp
 * PURPOSE : Dissociate a CNPV with a CP on a logical port.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
static UI32_T CN_ENGINE_DissociateCp(UI32_T priority, UI32_T lport)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    CN_OM_CpData_T      *cp_data_p;
    UI8_T               tmp_cnpvs, i;

    /* BODY
     */

    if ((CN_OM_ThreadDebug() == TRUE) && (lport == 1))
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu, %lu \r\n", __FUNCTION__, priority,
            lport);
    }

    for (i = 0; i < SYS_ADPT_CN_MAX_NBR_OF_CP_PER_PORT; i++)
    {
        cp_data_p = CN_OM_GetCpDataPtr(lport, i);
        if (cp_data_p->managed_cnpvs & (1 << (7 - priority)))
        {
            tmp_cnpvs = cp_data_p->managed_cnpvs & ~(1 << (7 - priority));
            if (CN_OM_SetCpManagedCnpvs(lport, i, tmp_cnpvs) != CN_TYPE_RETURN_OK)
            {
                if (CN_OM_ErrDebug() == TRUE)
                {
                    BACKDOOR_MGR_Printf("\r\n %s,%d: CN_OM_SetCpManagedCnpvs"
                        "(%lu, %lu, %02X) failed \r\n", __FUNCTION__, __LINE__,
                        lport, i,  tmp_cnpvs);
                }
                return CN_TYPE_RETURN_ERROR;
            }

            if (tmp_cnpvs == 0)
            {
                if (SWCTRL_PMGR_SetPortQcnCpq(lport, cp_data_p->queue,
                        SWCTRL_QCN_CPQ_INVALID) == FALSE)
                {
                    if (CN_OM_ErrDebug() == TRUE)
                    {
                        BACKDOOR_MGR_Printf("\r\n %s,%d: "
                            "SWCTRL_PMGR_SetPortQcnCpq"
                            "(%lu, %lu, SWCTRL_QCN_CPQ_INVALID) failed \r\n",
                            __FUNCTION__, __LINE__, lport, cp_data_p->queue);
                    }
                    return CN_TYPE_RETURN_ERROR;
                }

                if (CN_OM_SetCpStatus(lport, i, FALSE) != CN_TYPE_RETURN_OK)
                {
                    if (CN_OM_ErrDebug() == TRUE)
                    {
                        BACKDOOR_MGR_Printf("\r\n %s,%d: CN_OM_SetCpStatus"
                            "(%lu, %lu, FALSE) failed \r\n", __FUNCTION__,
                            __LINE__, lport, i);
                    }
                    return CN_TYPE_RETURN_ERROR;
                }
            }

            return CN_TYPE_RETURN_OK;
        }
    }

    return CN_TYPE_RETURN_OK;
} /* End of CN_ENGINE_DissociateCp */

/* FUNCTION NAME - CN_ENGINE_RegisterToL4
 * PURPOSE : Make a declaration to use CoS config by CN for a logical port.
 * INPUT   : lport - the specified logical port
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : None
 */
static UI32_T CN_ENGINE_RegisterToL4(UI32_T lport)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if ((CN_OM_ThreadDebug() == TRUE) && (lport == 1))
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu \r\n", __FUNCTION__, lport);
    }

    if (l4_register_count[lport-1] == 0)
    {
        if (L4_PMGR_QOS_EnablePortPriorityMode(lport, COS_VM_PRIORITY_CN)
                == FALSE)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: "
                    "L4_PMGR_QOS_EnablePortPriorityMode"
                    "(%lu, COS_VM_PRIORITY_CN) failed \r\n", __FUNCTION__,
                    __LINE__, lport);
            }

            return CN_TYPE_RETURN_ERROR;
        }

        if (L4_PMGR_QOS_SetPortPriorityTrustMode(lport, COS_VM_PRIORITY_CN,
                COS_MAPPING_MODE) == FALSE)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: "
                    "L4_PMGR_QOS_SetPortPriorityTrustMode"
                    "(%lu, COS_VM_PRIORITY_CN, COS_MAPPING_MODE) failed \r\n",
                    __FUNCTION__, __LINE__, lport);
            }

            return CN_TYPE_RETURN_ERROR;
        }
    }

    l4_register_count[lport-1]++;
    return CN_TYPE_RETURN_OK;
} /* End of CN_ENGINE_RegisterToL4 */

/* FUNCTION NAME - CN_ENGINE_DeregisterFromL4
 * PURPOSE : Withdraw a declaration to use CoS config by CN for a logical port.
 * INPUT   : priority - the specified CNPV
 *           lport    - the specified logical port
 * OUTPUT  : None
 * RETURN  : CN_TYPE_RETURN_CODE_E
 * NOTES   : It provides the function as DisableCnpvRemapping() in
 *           IEEE 802.1Qau-2010 32.5.1
 */
static UI32_T CN_ENGINE_DeregisterFromL4(UI32_T lport)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if ((CN_OM_ThreadDebug() == TRUE) && (lport == 1))
    {
        BACKDOOR_MGR_Printf("\r\n %s: %lu \r\n", __FUNCTION__, lport);
    }

    if (l4_register_count[lport-1] == 0)
    {
        return CN_TYPE_RETURN_OK;
    }

    l4_register_count[lport-1]--;

    if (l4_register_count[lport-1] == 0)
    {
        if (L4_PMGR_QOS_DisablePortPriorityMode(lport, COS_VM_PRIORITY_CN)
                == FALSE)
        {
            if (CN_OM_ErrDebug() == TRUE)
            {
                BACKDOOR_MGR_Printf("\r\n %s,%d: "
                    "L4_PMGR_QOS_DisablePortPriorityMode"
                    "(%lu, COS_VM_PRIORITY_CN) failed \r\n", __FUNCTION__,
                    __LINE__, lport);
            }

            return CN_TYPE_RETURN_ERROR;
        }
    }

    return CN_TYPE_RETURN_OK;
} /* End of CN_ENGINE_DeregisterFromL4 */

#endif /* (SYS_CPNT_CN == TRUE) */
