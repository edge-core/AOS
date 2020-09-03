/*-----------------------------------------------------------------------------
 * Module Name: dcbx_mgr.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Implementatinos for the DCBX API
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    9/20/2012 - Ricky Lin, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2012
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "leaf_1213.h"

#include "sys_adpt.h"
#include "sys_cpnt.h"

#include "l_cvrt.h"
#include "l_mm.h"
#include "l_stdlib.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#include "dcb_group.h"
#include "if_pmgr.h"
#include "dcbx_backdoor.h"
#include "dcbx_mgr.h"
#include "dcbx_om_private.h"
#include "dcbx_type.h"
#include "mib2_pom.h"
#include "swctrl.h"
#include "trk_lib.h"
#include "trk_pmgr.h"
#include "lldp_type.h"
#include "lldp_pmgr.h"

#if(SYS_CPNT_PFC == TRUE)
#include "pfc_mgr.h"
#include "pfc_om.h"
#endif
#if(SYS_CPNT_ETS == TRUE)
#include "ets_mgr.h"
#endif



static BOOL_T provision_completed = FALSE;

static BOOL_T DCBX_MGR_LogicalPortExisting(UI32_T lport);
static UI32_T DCBX_MGR_AsymmetricStateMachine(UI8_T *sm_state, DCBX_TYPE_EVENT_ASYM_E asym_event);
static UI32_T DCBX_MGR_SymmetricStateMachine(UI8_T *sm_state, DCBX_TYPE_EVENT_SYM_E sym_event);
static UI32_T DCBX_MGR_EnableEtsStateMachine(UI32_T lport);
static UI32_T DCBX_MGR_DisableEtsStateMachine(UI32_T lport);
static UI32_T DCBX_MGR_RunEtsStateMachine(UI32_T lport, DCBX_OM_RemEtsEntry_T *remote_data);
static UI32_T DCBX_MGR_EnablePfcStateMachine(UI32_T lport);
static UI32_T DCBX_MGR_DisablePfcStateMachine(UI32_T lport);
static UI32_T DCBX_MGR_RunPfcStateMachine(UI32_T lport, DCBX_OM_RemPfcEntry_T *remote_data);
static UI32_T DCBX_MGR_ElectConfigSourcePort(UI32_T *lport_p);
static UI32_T DCBX_MGR_RunPortStateMachine(UI32_T lport);
static UI32_T DCBX_MGR_LocalReRunPortStateMachine(UI32_T lport, BOOL_T is_ets_chgd);

#if(SYS_CPNT_PFC == TRUE)
static BOOL_T DCBX_MGR_SetPfcOperEntry(UI32_T lport, UI32_T oper_mode, UI32_T oper_priority);
static void DCBX_MGR_ApplyAllPfcOperEntry(UI32_T lport, UI32_T oper_priority);
#endif
#if(SYS_CPNT_ETS == TRUE)
static void DCBX_MGR_ApplyAllEtsOperEntry(UI32_T lport, ETS_TYPE_PortEntry_T  oper_entry);
#endif

SYSFUN_DECLARE_CSC



/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_SetPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  :  Set port_status to control DCBX is enabled
 *             or is disabled.
 * INPUT    : UI32_T lport            -- lport number
 *            BOOL_T port_status      -- Enable(TRUE)
 *                                       Disable(FALSE)
 * OUTPUT   : None
 * RETURN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 38.4, IEEE Std P802.1Qaz/D2.5.
 *            Default value: Enable(TRUE).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_MGR_SetPortStatus(UI32_T lport, BOOL_T port_status)
{
    BOOL_T current_admin_status;
    UI32_T ret = DCBX_TYPE_RETURN_ERROR;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return ret;
    }

    /* +++ Enter critical region +++*/
    DCBX_OM_EnterCriticalSection();

    /* get port config entry pointer from om*/
    if(DCBX_MGR_LogicalPortExisting(lport))
    {
        if(DCBX_OM_GetPortStatus(lport, &current_admin_status) == DCBX_TYPE_RETURN_ERROR)
        {
            DCBX_OM_LeaveCriticalSection();
            return ret;
        }

        /* if admin_status is not change, return directly */
        if(port_status == current_admin_status)
        {
            ret = DCBX_TYPE_RETURN_OK;
            DCBX_OM_LeaveCriticalSection();

            return ret;
        }
        /* if value are different */
        if(DCBX_OM_SetPortStatus(lport, port_status) == DCBX_TYPE_RETURN_ERROR)
        {
            DCBX_OM_LeaveCriticalSection();
            return ret;
        }

        DCBX_MGR_RunPortStateMachine(lport);

        if (TRUE == port_status)
        {
            /* notify LLDP to pass ETS/PFC setting to DCBX again.
             */
            LLDP_PMGR_NotifyEtsPfcCfgChanged(lport, TRUE, TRUE);
        }

        ret = DCBX_TYPE_RETURN_OK;
    }
    /* +++ Leave critical region +++*/
    DCBX_OM_LeaveCriticalSection();

    return ret;
}/* End of DCBX_MGR_SetPortStatus */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_GetPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  :  Get port_status
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : port_status_p      -- Enable(TRUE)
 *                                                       Disable(FALSE)
 * RETURN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 38.4, IEEE Std P802.1Qaz/D2.5.
 *            Default value: Enable(TRUE).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_MGR_GetPortStatus(UI32_T lport, BOOL_T *port_status_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return DCBX_TYPE_RETURN_ERROR;
    }

    if (port_status_p == NULL)
    {
        return DCBX_TYPE_RETURN_ERROR;
    }

    if(DCBX_MGR_LogicalPortExisting(lport) == FALSE)
    {
        return DCBX_TYPE_RETURN_ERROR;
    }

    /* +++ Enter critical region +++*/
    DCBX_OM_EnterCriticalSection();

    /* get port status from om */
    if(DCBX_OM_GetPortStatus(lport, port_status_p) == DCBX_TYPE_RETURN_ERROR)
    {
        DCBX_OM_LeaveCriticalSection();
        return DCBX_TYPE_RETURN_ERROR;
    }
    /* +++ Leave critical region +++*/
    DCBX_OM_LeaveCriticalSection();

    return DCBX_TYPE_RETURN_OK;
}/* End of DCBX_MGR_GetPortStatus */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_GetRunningPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  :  Get port_status
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : port_status_p      -- Enable(TRUE)
 *                                                       Disable(FALSE)
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : Ref to the description in 38.4, IEEE Std P802.1Qaz/D2.5.
 *            Default value: Enable(TRUE).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_MGR_GetRunningPortStatus(UI32_T lport, BOOL_T *port_status_p)
{
    UI32_T result = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    BOOL_T admin_status = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    if (port_status_p == NULL)
    {
        return result;
    }

    if(DCBX_MGR_LogicalPortExisting(lport) == FALSE)
    {
        return result;
    }

    /* +++ Enter critical region +++*/
    DCBX_OM_EnterCriticalSection();

    /* get port status from om */
    if(DCBX_OM_GetPortStatus(lport, &admin_status) == DCBX_TYPE_RETURN_ERROR)
    {
        DCBX_OM_LeaveCriticalSection();
        return result;
    }

    if(admin_status == DCBX_TYPE_DEFAULT_PORT_STATUS)
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    else
    {
        *port_status_p = admin_status;
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    /* +++ Leave critical region +++*/
    DCBX_OM_LeaveCriticalSection();

    return result;
}/* End of DCBX_MGR_GetRunningPortStatus */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_SetPortMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set port mode
 * INPUT    : UI32_T lport            -- lport number
 *            UI32_T port_mode        -- manual(1),
 *                                       configuration_source(2),
 *                                       auto-upstream(3),
 *                                       auto-downstream(4)
 * OUTPUT   : None
 * RETURN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTE     : Default value: manual(1).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_MGR_SetPortMode(UI32_T lport, UI32_T port_mode)
{
    UI32_T          current_admin_mode;
    DCBX_OM_PortConfigEntry_T * port_entry_p = NULL;
    DCBX_OM_SystemOperEntry_T* sys_oper_p = NULL;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return DCBX_TYPE_RETURN_ERROR;
    }

    /* +++ Enter critical region +++*/
    DCBX_OM_EnterCriticalSection();

    sys_oper_p = DCBX_OM_GetSysOper();
    if(sys_oper_p == NULL)
    {
        DCBX_OM_LeaveCriticalSection();
        return DCBX_TYPE_RETURN_ERROR;
    }

    if((port_mode == DCBX_TYPE_PORT_MODE_CFGSRC) &&
        (sys_oper_p->is_manual_cfg_src == TRUE) &&
        (sys_oper_p->cfg_src_ifindex != lport))
    {
        /* not allow two manual configuration source ports*/
        DCBX_OM_LeaveCriticalSection();
        return DCBX_TYPE_RETURN_ERROR;
    }

    /* get port config entry pointer from om*/
    if(DCBX_MGR_LogicalPortExisting(lport))
    {
        port_entry_p = DCBX_OM_GetPortConfigEntryPtr(lport);

        if(port_entry_p == NULL)
        {
            DCBX_OM_LeaveCriticalSection();
            return DCBX_TYPE_RETURN_ERROR;
        }

        current_admin_mode = port_entry_p->port_mode;

        /* if admin_status is not change, return directly*/
        if(port_mode == current_admin_mode)
        {
            DCBX_OM_LeaveCriticalSection();
            return DCBX_TYPE_RETURN_OK;
        }

        /* if value are different */
        if(DCBX_OM_SetPortMode(lport, port_mode) == DCBX_TYPE_RETURN_ERROR)
        {
            DCBX_OM_LeaveCriticalSection();
            return DCBX_TYPE_RETURN_ERROR;
        }

        if(port_mode == DCBX_TYPE_PORT_MODE_CFGSRC)
        {
            sys_oper_p->is_manual_cfg_src = TRUE;
            sys_oper_p->is_cfg_src_selected = TRUE;
            sys_oper_p->cfg_src_ifindex = lport;
        }
        else if(current_admin_mode == DCBX_TYPE_PORT_MODE_CFGSRC)
        {
            sys_oper_p->is_manual_cfg_src = FALSE;
            sys_oper_p->is_cfg_src_selected = FALSE;
            sys_oper_p->cfg_src_ifindex = 0;
        }
        else if(current_admin_mode == DCBX_TYPE_PORT_MODE_AUTOUP)
        {
            if (  (TRUE == sys_oper_p->is_cfg_src_selected)
                &&(lport == sys_oper_p->cfg_src_ifindex)
               )
            {
                sys_oper_p->is_cfg_src_selected = FALSE;
                sys_oper_p->cfg_src_ifindex = 0;
            }
        }

        DCBX_MGR_RunPortStateMachine(lport);

        switch (port_mode)
        {
        case DCBX_TYPE_PORT_MODE_CFGSRC:
        case DCBX_TYPE_PORT_MODE_AUTOUP:
            /* notify LLDP to pass ETS/PFC setting to DCBX again.
             */
            LLDP_PMGR_NotifyEtsPfcCfgChanged(lport, TRUE, TRUE);
            break;
        default:
            break;
        }
    }
    /* +++ Leave critical region +++*/
    DCBX_OM_LeaveCriticalSection();

    return DCBX_TYPE_RETURN_OK;
}/* End of DCBX_MGR_SetPortMode */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_GetPortMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port mode
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : port_mode_p   --   manual(1),
 *                                                      configuration_source(2),
 *                                                      auto-upstream(3),
 *                                                      auto-downstream(4)
 * RETURN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTE     : Default value: manual(1).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_MGR_GetPortMode(UI32_T lport, UI32_T *port_mode_p)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return DCBX_TYPE_RETURN_ERROR;
    }

    if (port_mode_p == NULL)
    {
        return DCBX_TYPE_RETURN_ERROR;
    }

    if(DCBX_MGR_LogicalPortExisting(lport) == FALSE)
    {
        DCBX_OM_LeaveCriticalSection();
        return DCBX_TYPE_RETURN_ERROR;
    }

    /* +++ Enter critical region +++*/
    DCBX_OM_EnterCriticalSection();

    /* get port mode from om*/
    if(DCBX_OM_GetPortMode(lport, port_mode_p) == DCBX_TYPE_RETURN_ERROR)
    {
        DCBX_OM_LeaveCriticalSection();
        return DCBX_TYPE_RETURN_ERROR;
    }

    /* +++ Leave critical region +++*/
    DCBX_OM_LeaveCriticalSection();

    return DCBX_TYPE_RETURN_OK;
}/* End of DCBX_MGR_GetPortMode */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_GetRunningPortMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port mode
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : port_mode_p   --   manual(1),
 *                                                      configuration_source(2),
 *                                                      auto-upstream(3),
 *                                                      auto-downstream(4)
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : Default value: manual(1).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_MGR_GetRunningPortMode(UI32_T lport, UI32_T *port_mode_p)
{
    UI32_T result = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    UI32_T admin_mode = DCBX_TYPE_PORT_MODE_MANUAL;
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    if (port_mode_p == NULL)
    {
        return result;
    }

    if(DCBX_MGR_LogicalPortExisting(lport) == FALSE)
    {
        DCBX_OM_LeaveCriticalSection();
        return result;
    }

    /* +++ Enter critical region +++*/
    DCBX_OM_EnterCriticalSection();

    /* get port mode from om*/
    if(DCBX_OM_GetPortMode(lport, &admin_mode) == DCBX_TYPE_RETURN_ERROR)
    {
        DCBX_OM_LeaveCriticalSection();
        return result;
    }

    if(admin_mode == DCBX_TYPE_DEFAULT_PORT_MODE)
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    else
    {
        *port_mode_p = admin_mode;
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    /* +++ Leave critical region +++*/
    DCBX_OM_LeaveCriticalSection();

    return result;
}/* End of DCBX_MGR_GetRunningPortMode */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter Master mode calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_EnterMasterMode(void)
{
    UI32_T  lport = 0;
    DCBX_OM_PortConfigEntry_T   *port_config;
    DCBX_OM_SystemOperEntry_T    *sys_config;

    /* set the system and ports default configuration */

    /* +++ Enter critical region +++ */
    DCBX_OM_EnterCriticalSection();

    sys_config = DCBX_OM_GetSysOper();
    sys_config->is_cfg_src_selected = FALSE;
    sys_config->is_manual_cfg_src = FALSE;
    sys_config->cfg_src_ifindex = 0;

    /* reset each port's admin status */
    for(lport = 1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT; lport ++)
    {
        port_config = DCBX_OM_GetPortConfigEntryPtr(lport);
        port_config->lport_num = lport;
        port_config->port_status = DCBX_TYPE_DEFAULT_PORT_STATUS;
        port_config->port_mode = DCBX_TYPE_DEFAULT_PORT_MODE;
        port_config->ets_sm_state = DCBX_TYPE_ASYM_DISABLE_STATE;
        port_config->pfc_sm_state = DCBX_TYPE_SYM_DISABLE_STATE;
        port_config->is_peer_detected = FALSE;
    }
    /* Leave critical region*/
    DCBX_OM_LeaveCriticalSection();

    SYSFUN_ENTER_MASTER_MODE();

}/* End of DCBX_MGR_EnterMasterMode*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter Slave mode calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
    return;
}/* End of DCBX_MGR_EnterSlaveMode */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_SetTransitionMode()
{
    SYSFUN_SET_TRANSITION_MODE();
}/* End of DCBX_MGR_SetTransitionMode */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_MGR_EnterTransitionMode()
{
    UI32_T                  result;

    SYSFUN_ENTER_TRANSITION_MODE();

    result              = DCBX_TYPE_RETURN_ERROR;
    /* +++ Enter critical region +++ */
    DCBX_OM_EnterCriticalSection();

    /* Delete all databases and all link information */
    DCBX_OM_ResetAll();

    /* Leave critical region*/
    DCBX_OM_LeaveCriticalSection();

    return result;
}/* End of DCBX_MGR_EnterTransitionMode */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_TrunkMemberAdd1st_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port joins the trunk
 *            as the 2nd or the following member
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_TrunkMemberAdd1st_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
#if (SYS_CPNT_TRUNK_MEMBER_AUTO_ATTRIBUTE == TRUE)
    DCBX_OM_PortConfigEntry_T *om_port_config, *trunk_port_config;
#endif

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return ;
    }

    /* +++ Enter critical region +++ */
    DCBX_OM_EnterCriticalSection();

#if (SYS_CPNT_TRUNK_MEMBER_AUTO_ATTRIBUTE == TRUE)
    /* member admin_status is inherited to trunk*/
    om_port_config = DCBX_OM_GetPortConfigEntryPtr(member_ifindex);
    trunk_port_config = DCBX_OM_GetPortConfigEntryPtr(trunk_ifindex);
    trunk_port_config->lport_num = trunk_ifindex;

    trunk_port_config->port_status = om_port_config->port_status;
    trunk_port_config->port_mode = om_port_config->port_mode;
#endif

    DCBX_OM_ResetPort(trunk_ifindex);

    DCBX_MGR_RunPortStateMachine(trunk_ifindex);

    /* Leave critical region*/
    DCBX_OM_LeaveCriticalSection();

    return ;
}/* End of DCBX_MGR_TrunkMemberAdd1st_CallBack*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_TrunkMemberAdd_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port joins the trunk
 *            as the 2nd or the following member
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_TrunkMemberAdd_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
#if (SYS_CPNT_TRUNK_MEMBER_AUTO_ATTRIBUTE == TRUE)
    DCBX_OM_PortConfigEntry_T *om_port_config = NULL, *trunk_port_config = NULL;
#endif

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return ;
    }
    /* +++ Enter critical region +++ */
    DCBX_OM_EnterCriticalSection();

#if (SYS_CPNT_TRUNK_MEMBER_AUTO_ATTRIBUTE == TRUE)
    /* trunk configuration sync to member*/
    om_port_config = DCBX_OM_GetPortConfigEntryPtr(member_ifindex);
    trunk_port_config = DCBX_OM_GetPortConfigEntryPtr(trunk_ifindex);

    om_port_config->port_status = trunk_port_config->port_status;
    om_port_config->port_mode = trunk_port_config->port_mode;
#endif

    DCBX_OM_ResetPort(member_ifindex);

    DCBX_MGR_RunPortStateMachine(member_ifindex);

    /* Leave critical region*/
    DCBX_OM_LeaveCriticalSection();

    return ;

}/* End of DCBX_MGR_TrunkMemberAdd_CallBack*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_TrunkMemberDelete_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port delete from the trunk
 *            as the 2nd or the following member
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_TrunkMemberDelete_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
#if (SYS_CPNT_TRUNK_MEMBER_AUTO_ATTRIBUTE == TRUE)
    DCBX_OM_PortConfigEntry_T *om_port_config = NULL, *trunk_port_config = NULL;
#endif

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return ;
    }
    /*+++ Enter Critical Section +++*/
    DCBX_OM_EnterCriticalSection();

#if (SYS_CPNT_TRUNK_MEMBER_AUTO_ATTRIBUTE == TRUE)
    /* trunk configuration sync to member*/
    om_port_config = DCBX_OM_GetPortConfigEntryPtr(member_ifindex);
    trunk_port_config = DCBX_OM_GetPortConfigEntryPtr(trunk_ifindex);

    om_port_config->port_status = trunk_port_config->port_status;
    om_port_config->port_mode = trunk_port_config->port_mode;
#endif

    DCBX_OM_ResetPort(member_ifindex);

    DCBX_MGR_RunPortStateMachine(member_ifindex);

    /*+++ Leave Critical Section +++*/
    DCBX_OM_LeaveCriticalSection();

    return ;
}/* End of DCBX_MGR_TrunkMemberDelete_CallBack*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_TrunkMemberDeleteLst_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port delete from the trunk
 *            as the 2nd or the following member
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_TrunkMemberDeleteLst_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
#if (SYS_CPNT_TRUNK_MEMBER_AUTO_ATTRIBUTE == TRUE)
    DCBX_OM_PortConfigEntry_T *om_port_config = NULL, *trunk_port_config = NULL;
#endif

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return ;
    }

    /* +++Enter critical section+++ */
    DCBX_OM_EnterCriticalSection();

#if (SYS_CPNT_TRUNK_MEMBER_AUTO_ATTRIBUTE == TRUE)
    /* trunk configuration sync to member*/
    om_port_config = DCBX_OM_GetPortConfigEntryPtr(member_ifindex);
    trunk_port_config = DCBX_OM_GetPortConfigEntryPtr(trunk_ifindex);

    om_port_config->port_status = trunk_port_config->port_status;
    om_port_config->port_mode = trunk_port_config->port_mode;
#endif

    DCBX_OM_ResetPort(member_ifindex);

    DCBX_MGR_RunPortStateMachine(member_ifindex);
    trunk_port_config->port_status = FALSE;
    DCBX_MGR_RunPortStateMachine(trunk_ifindex);

    /* +++Leaver critical section+++ */
    DCBX_OM_LeaveCriticalSection();

    return ;
}/* End of DCBX_MGR_TrunkMemberDeleteLst_CallBack*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_EtsRcvd_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Service the callback from LLDP when ETS TLV changed for the port
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_EtsRcvd_CallBack(UI32_T   lport,
                               BOOL_T   is_delete,
                               BOOL_T   rem_recommend_rcvd,
                               BOOL_T   rem_willing,
                               BOOL_T   rem_cbs,
                               UI8_T    rem_max_tc,
                               UI8_T    *rem_config_pri_assign_table,
                               UI8_T    *rem_config_tc_bandwidth_table,
                               UI8_T    *rem_config_tsa_assign_table,
                               UI8_T    *rem_recommend_pri_assign_table,
                               UI8_T    *rem_recommend_tc_bandwidth_table,
                               UI8_T    *rem_recommend_tsa_assign_table)
{
    DCBX_OM_RemEtsEntry_T remote_entry;

    if (DCBX_MGR_LogicalPortExisting(lport) == FALSE)
    {
        return;
    }

    if ((rem_config_pri_assign_table == NULL) ||
        (rem_config_tc_bandwidth_table == NULL) ||
        (rem_config_tsa_assign_table == NULL) ||
        (rem_recommend_pri_assign_table == NULL) ||
        (rem_recommend_tc_bandwidth_table == NULL) ||
        (rem_recommend_tsa_assign_table == NULL))
    {
        return;
    }

    remote_entry.lport = lport;
    remote_entry.is_delete = is_delete;
    remote_entry.rem_recommend_rcvd = rem_recommend_rcvd;
    remote_entry.rem_willing = rem_willing;
    remote_entry.rem_cbs = rem_cbs;
    remote_entry.rem_max_tc = rem_max_tc;
    memcpy(remote_entry.rem_con_pri_assign_table, rem_config_pri_assign_table, LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN);
    memcpy(remote_entry.rem_con_tc_bandwidth_table, rem_config_tc_bandwidth_table, LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN);
    memcpy(remote_entry.rem_con_tsa_assign_table, rem_config_tsa_assign_table, LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN);
    memcpy(remote_entry.rem_recom_pri_assign_table, rem_recommend_pri_assign_table, LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN);
    memcpy(remote_entry.rem_recom_tc_bandwidth_table, rem_recommend_tc_bandwidth_table, LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN);
    memcpy(remote_entry.rem_recom_tsa_assign_table, rem_recommend_tsa_assign_table, LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN);

    DCBX_MGR_RunEtsStateMachine(lport, &remote_entry);
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_PfcRcvd_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Service the callback from LLDP when PFC TLV changed for the port
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_PfcRcvd_CallBack(UI32_T   lport,
                               BOOL_T   is_delete,
                               UI8_T    *rem_mac,
                               BOOL_T   rem_willing,
                               BOOL_T   rem_mbc,
                               UI8_T    rem_pfc_cap,
                               UI8_T    rem_pfc_enable)
{
    DCBX_OM_RemPfcEntry_T remote_entry;

    if(rem_mac == NULL)
    {
        return;
    }

    if(DCBX_MGR_LogicalPortExisting(lport) == FALSE)
    {
        return;
    }

    remote_entry.lport = lport;
    remote_entry.is_delete = is_delete;
    memcpy(&remote_entry.rem_mac[0], rem_mac, 6);
    remote_entry.rem_willing = rem_willing;
    remote_entry.rem_mbc = rem_mbc;
    remote_entry.rem_pfc_cap = rem_pfc_cap;
    remote_entry.rem_pfc_enable = rem_pfc_enable;
    DCBX_MGR_RunPfcStateMachine(lport, &remote_entry);
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_GetOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE  : get the bridge operation mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_MGR_GetOperationMode()
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
}/* End of DCBX_MGR_GetOperationMode*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_HandleHotInsertion
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the module hot insert.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_HandleHotInsertion(UI32_T beg_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    DCBX_OM_PortConfigEntry_T   *port_config;
    UI32_T                      lport;

    /* +++Enter critical section+++ */
    DCBX_OM_EnterCriticalSection();

    for(lport = beg_ifindex; lport < (beg_ifindex + number_of_port); lport++)
    {
        port_config = DCBX_OM_GetPortConfigEntryPtr(lport);
        port_config->lport_num = lport;
        port_config->port_status = DCBX_TYPE_DEFAULT_PORT_STATUS;
        port_config->port_mode = DCBX_TYPE_DEFAULT_PORT_MODE;
        port_config->ets_sm_state = DCBX_TYPE_ASYM_DISABLE_STATE;
        port_config->pfc_sm_state = DCBX_TYPE_SYM_DISABLE_STATE;
        port_config->is_peer_detected = FALSE;
    }

    /* Leave critical section */
    DCBX_OM_LeaveCriticalSection();

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_HandleHotRemoval
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the module hot remove.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_HandleHotRemoval(UI32_T beg_ifindex, UI32_T number_of_port)
{
    UI32_T lport;

    /* +++Enter critical section+++ */
    DCBX_OM_EnterCriticalSection();

    for(lport = beg_ifindex; lport < (beg_ifindex + number_of_port); lport++)
    {
        DCBX_OM_ResetPort(lport);
    }

    /* Leave critical section */
    DCBX_OM_LeaveCriticalSection();
}


/*-----------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for DCBX MGR.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T DCBX_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    DCBX_MGR_IpcMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (DCBX_MGR_IpcMsg_T*)msgbuf_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msg_p->type.ret_ui32 = DCBX_TYPE_RETURN_ERROR;
        msgbuf_p->msg_size = DCBX_MGR_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    /* dispatch IPC message and call the corresponding DCBX_MGR function
     */
    switch (msg_p->type.cmd)
    {
        case DCBX_MGR_IPC_SETPORTSTATUS:
            msg_p->type.ret_ui32 = DCBX_MGR_SetPortStatus(
                msg_p->data.arg_grp_ui32_bool.arg_ui32,
                msg_p->data.arg_grp_ui32_bool.arg_bool);
            msgbuf_p->msg_size = DCBX_MGR_GET_MSG_SIZE(arg_grp_ui32_bool);
            break;

        case DCBX_MGR_IPC_GETPORTSTATUS:
            msg_p->type.ret_ui32 = DCBX_MGR_GetPortStatus(
                msg_p->data.arg_grp_ui32_bool.arg_ui32,
                &msg_p->data.arg_grp_ui32_bool.arg_bool);
            msgbuf_p->msg_size = DCBX_MGR_GET_MSG_SIZE(arg_grp_ui32_bool);
            break;

        case DCBX_MGR_IPC_GETRUNNINGPORTSTATUS:
            msg_p->type.ret_ui32 = DCBX_MGR_GetRunningPortStatus(
                msg_p->data.arg_grp_ui32_bool.arg_ui32,
                &msg_p->data.arg_grp_ui32_bool.arg_bool);
            msgbuf_p->msg_size = DCBX_MGR_GET_MSG_SIZE(arg_grp_ui32_bool);
            break;

        case DCBX_MGR_IPC_SETPORTMODE:
            msg_p->type.ret_ui32 = DCBX_MGR_SetPortMode(
                msg_p->data.arg_grp_ui32_ui32.arg_ui32_1,
                msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = DCBX_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
            break;

        case DCBX_MGR_IPC_GETPORTMODE:
            msg_p->type.ret_ui32 = DCBX_MGR_GetPortMode(
                msg_p->data.arg_grp_ui32_ui32.arg_ui32_1,
                &msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = DCBX_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
            break;

        case DCBX_MGR_IPC_GETRUNNINGPORTMODE:
            msg_p->type.ret_ui32 = DCBX_MGR_GetRunningPortMode(
                msg_p->data.arg_grp_ui32_ui32.arg_ui32_1,
                &msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = DCBX_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
            break;

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_ui32 = DCBX_TYPE_RETURN_ERROR;
            msgbuf_p->msg_size = DCBX_MGR_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of DCBX_MGR_HandleIPCReqMsg */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_Init
 *-------------------------------------------------------------------------
 * FUNCTION: Init the DCBX CSC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_Init(void)
{
    /* Init OM : default state is disabled */
    DCBX_OM_InitSemaphore();
    DCBX_OM_Init();
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_Create_InterCSC_Relation()
 *-------------------------------------------------------------------------
 * FUNCTION: Register other CSC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_Create_InterCSC_Relation(void)
{
    /* Register Backdoor */
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("DCBX",
        SYS_BLD_DCB_GROUP_IPCMSGQ_KEY, DCBX_BACKDOOR_Main);
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_ProvisionComplete
 *-------------------------------------------------------------------------
 * PURPOSE  : This function calling by stkctrl will tell DCBX that provision is completed.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void DCBX_MGR_ProvisionComplete(void)
{
    provision_completed = TRUE;
    return;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_IsProvisionComplete
 *-------------------------------------------------------------------------
 * PURPOSE  : This function calling by DCBX mgr will tell if DCBX provision is completed.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
BOOL_T DCBX_MGR_IsProvisionComplete(void)
{
    return (provision_completed);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_ReRunPortStateMachine
 *-------------------------------------------------------------------------
 * PURPOSE  : To re-run port sm when ETS/PFC config is changed.
 * INPUT    : lport       -- lport number
 *            is_ets_chgd -- TRUE if ETS changed/FALSE if PFC changed
 * OUTPUT   : None
 * RETURN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_MGR_ReRunPortStateMachine(
    UI32_T  lport,
    BOOL_T  is_ets_chgd)
{
    UI32_T ret = DCBX_TYPE_RETURN_ERROR;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return ret;
    }

    /* +++ Enter critical region +++*/
    DCBX_OM_EnterCriticalSection();

    ret = DCBX_MGR_LocalReRunPortStateMachine(lport, is_ets_chgd);

    /* +++ Leave critical region +++*/
    DCBX_OM_LeaveCriticalSection();

    return ret;
}


/* Local APIs
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_LogicalPortExisting
 *-------------------------------------------------------------------------
 * PURPOSE  : Check logical port
 * INPUT    : lport
 * OUTPUT   : lport
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T DCBX_MGR_LogicalPortExisting(UI32_T lport)
{
    UI32_T  trunk_id = 0;

    if(SWCTRL_LogicalPortExisting(lport))
    {
        if(SWCTRL_LogicalPortIsTrunkPort(lport))
        {
            TRK_OM_IfindexToTrunkId(lport, &trunk_id);
            if(TRK_PMGR_GetTrunkMemberCounts(trunk_id) == 0)
                return FALSE;
        }

        return TRUE;
    }
    return FALSE;
}

static UI32_T DCBX_MGR_AsymmetricStateMachine(UI8_T *sm_state, DCBX_TYPE_EVENT_ASYM_E asym_event)
{
    UI32_T ret = DCBX_TYPE_ASYM_ACTION_NONE;
    /* check input */
    if(sm_state == NULL)
        return ret;

    switch(asym_event)
    {
    case DCBX_TYPE_EVENT_ASYM_ENABLE:
        /* Init state machine, enter Init state and do action */
        if(*sm_state == DCBX_TYPE_ASYM_DISABLE_STATE)
        {
            *sm_state = DCBX_TYPE_ASYM_INIT_STATE;
            ret = DCBX_TYPE_ASYM_ACTION_INIT;
        }
        break;
    case DCBX_TYPE_EVENT_ASYM_ADD_REMOTE_PARAM:
        /* if original state is Init, enter RxRecommend state and do action */
        if(*sm_state == DCBX_TYPE_ASYM_INIT_STATE)
        {
            *sm_state = DCBX_TYPE_ASYM_RXRECOMMEND_STATE;
            ret = DCBX_TYPE_ASYM_ACTION_APPLY_REMOTE;
        }
        break;
    case DCBX_TYPE_EVENT_ASYM_REMOVE_REMOTE_PARAM:
        /* Init state machine, enter Init state and do action */
        if(*sm_state == DCBX_TYPE_ASYM_RXRECOMMEND_STATE)
        {
            *sm_state = DCBX_TYPE_ASYM_INIT_STATE;
            ret = DCBX_TYPE_ASYM_ACTION_INIT;
        }
        break;
    case DCBX_TYPE_EVENT_ASYM_OPER_NOT_EQUAL_REMOTE_PARAM:
        /* if original state is RxRecommend, enter RxRecommend state and do action */
        if(*sm_state == DCBX_TYPE_ASYM_RXRECOMMEND_STATE)
        {
            *sm_state = DCBX_TYPE_ASYM_RXRECOMMEND_STATE;
            ret = DCBX_TYPE_ASYM_ACTION_APPLY_REMOTE;
        }
        break;
    case DCBX_TYPE_EVENT_ASYM_OPER_NOT_EQUAL_LOCAL_ADMIN_PARAM:
        /* Init state machine, enter Init state and do action */
        if(*sm_state == DCBX_TYPE_ASYM_INIT_STATE)
        {
            *sm_state = DCBX_TYPE_ASYM_INIT_STATE;
            ret = DCBX_TYPE_ASYM_ACTION_INIT;
        }
        break;
    case DCBX_TYPE_EVENT_ASYM_DISABLE:
        *sm_state = DCBX_TYPE_ASYM_DISABLE_STATE;
        ret = DCBX_TYPE_ASYM_ACTION_INIT;
        break;
    case DCBX_TYPE_EVENT_ASYM_NONE:
    default:
        break;
    }
    return ret;
}

static UI32_T DCBX_MGR_SymmetricStateMachine(UI8_T *sm_state, DCBX_TYPE_EVENT_SYM_E sym_event)
{
    UI32_T ret = DCBX_TYPE_SYM_ACTION_NONE;
    /* check input */
    if(sm_state == NULL)
        return ret;

    switch(sym_event)
    {
    case DCBX_TYPE_EVENT_SYM_ENABLE:
        /* Init state machine, enter Init state and do action */
        if(*sm_state == DCBX_TYPE_SYM_DISABLE_STATE)
        {
            *sm_state = DCBX_TYPE_SYM_INIT_STATE;
            ret = DCBX_TYPE_SYM_ACTION_INIT;
        }
        break;
    case DCBX_TYPE_EVENT_SYM_ADD_REMOTE_PARAM:
        /* if original state is Init, enter RxRecommend state and do action */
        if(*sm_state == DCBX_TYPE_SYM_INIT_STATE)
        {
            *sm_state = DCBX_TYPE_SYM_RXRECOMMEND_STATE;
            ret = DCBX_TYPE_SYM_ACTION_APPLY_REMOTE;
        }
        break;
    case DCBX_TYPE_EVENT_SYM_REMOVE_REMOTE_PARAM:
        /* Init state machine, enter Init state and do action */
        if(*sm_state == DCBX_TYPE_SYM_RXRECOMMEND_STATE)
        {
            *sm_state = DCBX_TYPE_SYM_INIT_STATE;
            ret = DCBX_TYPE_SYM_ACTION_INIT;
        }
        break;
    case DCBX_TYPE_EVENT_SYM_OPER_NOT_EQUAL_REMOTE_PARAM:
        /* if original state is Init or RxRecommend, enter RxRecommend state and do action */
        if(*sm_state == DCBX_TYPE_SYM_RXRECOMMEND_STATE)
        {
            *sm_state = DCBX_TYPE_SYM_RXRECOMMEND_STATE;
            ret = DCBX_TYPE_SYM_ACTION_APPLY_REMOTE;
        }
        break;
    case DCBX_TYPE_EVENT_SYM_OPER_NOT_EQUAL_LOCAL_ADMIN_PARAM:
        /* Init state machine, enter Init state and do action */
        if(*sm_state == DCBX_TYPE_SYM_INIT_STATE)
        {
            *sm_state = DCBX_TYPE_SYM_INIT_STATE;
            ret = DCBX_TYPE_SYM_ACTION_INIT;
        }
        break;
    case DCBX_TYPE_EVENT_SYM_DISABLE:
        *sm_state = DCBX_TYPE_SYM_DISABLE_STATE;
        ret = DCBX_TYPE_SYM_ACTION_INIT;
        break;
    case DCBX_TYPE_EVENT_SYM_NONE:
    default:
        break;
    }

    return ret;
}

static UI32_T DCBX_MGR_EnableEtsStateMachine(UI32_T lport)
{
    UI32_T ret = DCBX_TYPE_RETURN_ERROR;
    UI32_T sm_action = DCBX_TYPE_ASYM_ACTION_NONE;
    DCBX_OM_PortConfigEntry_T * port_entry_p = NULL;
    DCBX_TYPE_EVENT_ASYM_E ets_event = DCBX_TYPE_EVENT_ASYM_ENABLE;

    port_entry_p = DCBX_OM_GetPortConfigEntryPtr(lport);

    if(port_entry_p == NULL)
    {
        return ret;
    }

    /* run state machine */
    sm_action = DCBX_MGR_AsymmetricStateMachine(&port_entry_p->ets_sm_state, ets_event);

    /* call ETS to set new config */
    if(sm_action == DCBX_TYPE_ASYM_ACTION_INIT)
    {
        /* set local admin config */
#if(SYS_CPNT_ETS == TRUE)
        ETS_MGR_RestoreToConfigByDCBx(lport);
#endif
    }
    else
    {
        /* do nothing */
    }
    ret = DCBX_TYPE_RETURN_OK;
    return ret;
}

static UI32_T DCBX_MGR_DisableEtsStateMachine(UI32_T lport)
{
    DCBX_OM_PortConfigEntry_T   *port_entry_p = NULL;
    UI32_T                      sm_action;

    port_entry_p = DCBX_OM_GetPortConfigEntryPtr(lport);
    if (port_entry_p == NULL)
    {
        return DCBX_TYPE_RETURN_ERROR;
    }

    /* run state machine */
    sm_action = DCBX_MGR_AsymmetricStateMachine(&port_entry_p->ets_sm_state,
                    DCBX_TYPE_EVENT_ASYM_DISABLE);
    if (sm_action == DCBX_TYPE_ASYM_ACTION_NONE)
    {
        return DCBX_TYPE_RETURN_ERROR;
    }

    /* set ETS oper mode to off and oper config as admin value */
#if(SYS_CPNT_ETS == TRUE)
    ETS_MGR_RestoreToConfigByDCBx(lport);
#endif

    return DCBX_TYPE_RETURN_OK;
}

static UI32_T DCBX_MGR_RunEtsStateMachine(UI32_T lport, DCBX_OM_RemEtsEntry_T *remote_data)
{
    UI32_T ret = DCBX_TYPE_RETURN_ERROR;
    UI32_T sm_action = DCBX_TYPE_ASYM_ACTION_NONE;
    DCBX_OM_PortConfigEntry_T * port_entry_p = NULL;
    DCBX_OM_SystemOperEntry_T* sys_oper_p = NULL;
    DCBX_TYPE_EVENT_ASYM_E ets_event = DCBX_TYPE_EVENT_ASYM_NONE;
    BOOL_T local_willing = FALSE;
    BOOL_T is_oper_equal_admin = FALSE;
    BOOL_T is_oper_equal_remote = FALSE;
#if(SYS_CPNT_ETS == TRUE)
    UI32_T index = 0;
    BOOL_T is_admin_from_config_src = FALSE;
    ETS_TYPE_PortEntry_T  oper_entry;
    ETS_TYPE_PortEntry_T  admin_entry;
    ETS_TYPE_PortEntry_T  remote_entry;
    ETS_TYPE_MODE_T ets_mode;
    ETS_TYPE_PortEntry_T  src_oper_entry;
#endif

    port_entry_p = DCBX_OM_GetPortConfigEntryPtr(lport);

    if((remote_data == NULL) || (port_entry_p == NULL))
    {
        return ret;
    }

    if(port_entry_p->port_status == FALSE)
    {
        /* nothing to do */
        return DCBX_TYPE_RETURN_OK;
    }

    sys_oper_p = DCBX_OM_GetSysOper();
    if(sys_oper_p == NULL)
    {
        return DCBX_TYPE_RETURN_ERROR;
    }

    DCBX_OM_EnterCriticalSection();
    port_entry_p->is_peer_detected = !remote_data->is_delete;
    DCBX_MGR_RunPortStateMachine(lport);
    DCBX_OM_LeaveCriticalSection();

    /* check sm event */
#if(SYS_CPNT_ETS == TRUE)
    memset(&oper_entry, 0, sizeof(ETS_TYPE_PortEntry_T));
    ETS_MGR_GetPortEntry(lport, &oper_entry, ETS_TYPE_DB_OPER);
    memset(&admin_entry, 0, sizeof(ETS_TYPE_PortEntry_T));
    ETS_MGR_GetPortEntry(lport, &admin_entry, ETS_TYPE_DB_CONFIG);
    ETS_MGR_GetMode(lport, &ets_mode);
    if((port_entry_p->port_mode == DCBX_TYPE_PORT_MODE_AUTODOWN) && (sys_oper_p->is_cfg_src_selected))
    {
        is_admin_from_config_src = TRUE;
        memset(&src_oper_entry, 0, sizeof(ETS_TYPE_PortEntry_T));
        ETS_MGR_GetPortEntry(sys_oper_p->cfg_src_ifindex, &src_oper_entry, ETS_TYPE_DB_OPER);
        if(memcmp(&src_oper_entry, &oper_entry, sizeof(ETS_TYPE_PortEntry_T)) == 0)
        {
            is_oper_equal_admin = TRUE;
        }
        else
        {
            is_oper_equal_admin = FALSE;
        }
    }
    else
    {
        is_admin_from_config_src = FALSE;
        is_oper_equal_admin = ETS_MGR_Is_Config_Match_Operation(lport);
    }
    memset(&remote_entry, 0, sizeof(ETS_TYPE_PortEntry_T));
    for(index = 0; index < SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; index++)
    {
        remote_entry.tsa[index] = remote_data->rem_recom_tsa_assign_table[index];
    }
    for(index = 0; index < SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE; index++)
    {
        remote_entry.priority_assign[index] = (0xf & (remote_data->rem_recom_pri_assign_table[index/2]>>(((index+1)%2)*4)));
    }
    for(index = 0; index < SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS; index++)
    {
        remote_entry.tc_weight[index] = remote_data->rem_recom_tc_bandwidth_table[index];
    }
    if(memcmp(&oper_entry, &remote_entry, sizeof(ETS_TYPE_PortEntry_T)) == 0)
    {
        is_oper_equal_remote = TRUE;
    }
#endif
    if(((port_entry_p->port_mode == DCBX_TYPE_PORT_MODE_CFGSRC) || (port_entry_p->port_mode == DCBX_TYPE_PORT_MODE_AUTOUP))
#if(SYS_CPNT_ETS == TRUE)
         && (ets_mode == ETS_TYPE_MODE_AUTO)
#endif
      )
    {
        local_willing = TRUE;
    }
    else
    {
        local_willing = FALSE;
    }


    if(port_entry_p->ets_sm_state == DCBX_TYPE_ASYM_INIT_STATE)
    {
        if((local_willing == TRUE) && (remote_data->is_delete == FALSE))
        {
            ets_event = DCBX_TYPE_EVENT_ASYM_ADD_REMOTE_PARAM;
        }
        else
        {
            if(is_oper_equal_admin == FALSE)
            {
                ets_event = DCBX_TYPE_EVENT_ASYM_OPER_NOT_EQUAL_LOCAL_ADMIN_PARAM;
            }
        }
    }

    if(port_entry_p->ets_sm_state == DCBX_TYPE_ASYM_RXRECOMMEND_STATE)
    {
        if((local_willing == FALSE) || (remote_data->is_delete == TRUE))
        {
            ets_event = DCBX_TYPE_EVENT_ASYM_REMOVE_REMOTE_PARAM;
        }
        else
        {
            if(is_oper_equal_remote == FALSE)
            {
                ets_event = DCBX_TYPE_EVENT_ASYM_OPER_NOT_EQUAL_REMOTE_PARAM;
            }
        }
    }

    /* run state machine */
    sm_action = DCBX_MGR_AsymmetricStateMachine(&port_entry_p->ets_sm_state, ets_event);

    /* call ETS to set new config */
    switch(sm_action)
    {
    case DCBX_TYPE_ASYM_ACTION_INIT:
        /* set local admin config */
#if(SYS_CPNT_ETS == TRUE)
        if(is_admin_from_config_src)
        {
            ETS_MGR_SetOperConfigEntryByDCBx(lport, src_oper_entry);
            DCBX_MGR_ApplyAllEtsOperEntry(lport, src_oper_entry);
        }
        else
        {
            ETS_MGR_RestoreToConfigByDCBx(lport);
            DCBX_MGR_ApplyAllEtsOperEntry(lport, admin_entry);
        }
#endif
        break;
    case DCBX_TYPE_ASYM_ACTION_APPLY_REMOTE:
        /* set remote config */
#if(SYS_CPNT_ETS == TRUE)
        ETS_MGR_SetOperConfigEntryByDCBx(lport, remote_entry);
        DCBX_MGR_ApplyAllEtsOperEntry(lport, remote_entry);
#endif
        break;
    case DCBX_TYPE_ASYM_ACTION_NONE:
    default:
        /* do nothing */
        break;
    }
    ret = DCBX_TYPE_RETURN_OK;
    return ret;
}

static UI32_T DCBX_MGR_EnablePfcStateMachine(UI32_T lport)
{
    UI32_T sm_action = DCBX_TYPE_SYM_ACTION_NONE;
    DCBX_OM_PortConfigEntry_T * port_entry_p = NULL;
    DCBX_TYPE_EVENT_SYM_E pfc_event = DCBX_TYPE_EVENT_SYM_ENABLE;

    port_entry_p = DCBX_OM_GetPortConfigEntryPtr(lport);
    if(port_entry_p == NULL)
    {
        return DCBX_TYPE_RETURN_ERROR;
    }

    /* run state machine */
    sm_action = DCBX_MGR_SymmetricStateMachine(&port_entry_p->pfc_sm_state, pfc_event);

    /* call PFC to set new config */
    if(sm_action == DCBX_TYPE_SYM_ACTION_INIT)
    {
        /* set local admin config to oper */
#if(SYS_CPNT_PFC == TRUE)
        PFC_MGR_RestoreToConfigByDCBX(lport);
#endif
    }
    else
    {
        /* do nothing */
    }

    return DCBX_TYPE_RETURN_OK;
}

static UI32_T DCBX_MGR_DisablePfcStateMachine(UI32_T lport)
{
    DCBX_OM_PortConfigEntry_T   *port_entry_p = NULL;
    UI32_T                      sm_action;

    port_entry_p = DCBX_OM_GetPortConfigEntryPtr(lport);
    if (port_entry_p == NULL)
    {
        return DCBX_TYPE_RETURN_ERROR;
    }

    /* run state machine */
    sm_action = DCBX_MGR_SymmetricStateMachine(&port_entry_p->pfc_sm_state,
                    DCBX_TYPE_EVENT_SYM_DISABLE);
    if (sm_action == DCBX_TYPE_ASYM_ACTION_NONE)
    {
        return DCBX_TYPE_RETURN_ERROR;
    }

    /* set PFC oper mode to off and oper config as admin value */
#if(SYS_CPNT_PFC == TRUE)
    PFC_MGR_RestoreToConfigByDCBX(lport);
#endif

    return DCBX_TYPE_RETURN_OK;
}
static UI32_T DCBX_MGR_RunPfcStateMachine(UI32_T lport, DCBX_OM_RemPfcEntry_T *remote_data)
{
    UI32_T ret = DCBX_TYPE_RETURN_ERROR;
    UI32_T sm_action = DCBX_TYPE_SYM_ACTION_NONE;
    DCBX_OM_PortConfigEntry_T * port_entry_p = NULL;
    DCBX_OM_SystemOperEntry_T* sys_oper_p = NULL;
    DCBX_TYPE_EVENT_SYM_E pfc_event = DCBX_TYPE_EVENT_NONE;
    UI8_T local_port_mac[SYS_ADPT_MAC_ADDR_LEN] = {0};
    BOOL_T local_willing = FALSE;
    UI32_T oper_priority = 0;
    UI32_T remote_priority = 0;
    BOOL_T is_oper_equal_admin = FALSE;
#if(SYS_CPNT_PFC == TRUE)
    UI32_T pfc_mode = 0;
    UI32_T admin_priority = 0;
    UI32_T src_oper_priority = 0;
    BOOL_T is_admin_from_config_src = FALSE;
#endif

    port_entry_p = DCBX_OM_GetPortConfigEntryPtr(lport);

    if((remote_data == NULL) || (port_entry_p == NULL))
    {
        return ret;
    }

    if(port_entry_p->port_status == FALSE)
    {
        /* nothing to do */
        return DCBX_TYPE_RETURN_OK;
    }

    sys_oper_p = DCBX_OM_GetSysOper();
    if(sys_oper_p == NULL)
    {
        return DCBX_TYPE_RETURN_ERROR;
    }

    if(SWCTRL_GetPortMac(lport, local_port_mac) == FALSE)
    {
        return ret;
    }

    DCBX_OM_EnterCriticalSection();
    port_entry_p->is_peer_detected = !remote_data->is_delete;
    DCBX_MGR_RunPortStateMachine(lport);
    DCBX_OM_LeaveCriticalSection();

    /* check sm event */
#if(SYS_CPNT_PFC == TRUE)
    PFC_OM_GetDataByField(lport, PFC_TYPE_FLDE_PORT_MODE_ADM, &pfc_mode);
    PFC_OM_GetDataByField(lport, PFC_TYPE_FLDE_PORT_PRI_EN_OPR, &oper_priority);
    PFC_OM_GetDataByField(lport, PFC_TYPE_FLDE_PORT_PRI_EN_ADM, &admin_priority);
    if((port_entry_p->port_mode == DCBX_TYPE_PORT_MODE_AUTODOWN) && (sys_oper_p->is_cfg_src_selected))
    {
        is_admin_from_config_src = TRUE;
        PFC_OM_GetDataByField(sys_oper_p->cfg_src_ifindex, PFC_TYPE_FLDE_PORT_PRI_EN_OPR, &src_oper_priority);
        if(src_oper_priority == oper_priority)
        {
            is_oper_equal_admin = TRUE;
        }
        else
        {
            is_oper_equal_admin = FALSE;
        }
    }
    else
    {
        is_admin_from_config_src = FALSE;
        if(admin_priority == oper_priority)
        {
            is_oper_equal_admin = TRUE;
        }
        else
        {
            is_oper_equal_admin = FALSE;
        }
    }
#endif
    remote_priority = remote_data->rem_pfc_enable;


    if(((port_entry_p->port_mode == DCBX_TYPE_PORT_MODE_CFGSRC) || (port_entry_p->port_mode == DCBX_TYPE_PORT_MODE_AUTOUP))
#if(SYS_CPNT_PFC == TRUE)
       && (pfc_mode == PFC_TYPE_PMODE_AUTO)
#endif
      )
    {
        local_willing = TRUE;
    }
    else
    {
        local_willing = FALSE;
    }

    if(port_entry_p->pfc_sm_state == DCBX_TYPE_SYM_INIT_STATE)
    {
        if(((local_willing == TRUE) && (remote_data->rem_willing == FALSE)) ||
            ((local_willing == TRUE) && (remote_data->rem_willing == TRUE) && (memcmp(local_port_mac, remote_data->rem_mac, SYS_ADPT_MAC_ADDR_LEN) > 0)))
        {
            pfc_event = DCBX_TYPE_EVENT_SYM_ADD_REMOTE_PARAM;
        }
        else
        {
            if(is_oper_equal_admin == FALSE)
            {
                pfc_event = DCBX_TYPE_EVENT_SYM_OPER_NOT_EQUAL_LOCAL_ADMIN_PARAM;
            }
        }
    }

    if(port_entry_p->pfc_sm_state == DCBX_TYPE_SYM_RXRECOMMEND_STATE)
    {
        if((local_willing == FALSE) ||
           (remote_data->is_delete == TRUE) ||
           ((local_willing == TRUE) && (remote_data->rem_willing == TRUE) && (memcmp(local_port_mac, remote_data->rem_mac, SYS_ADPT_MAC_ADDR_LEN) < 0)))
        {
            pfc_event = DCBX_TYPE_EVENT_SYM_REMOVE_REMOTE_PARAM;
        }
        else
        {
            if(oper_priority != remote_priority)
            {
                pfc_event = DCBX_TYPE_EVENT_SYM_OPER_NOT_EQUAL_REMOTE_PARAM;
            }
        }
    }

    /* run state machine */
    sm_action = DCBX_MGR_SymmetricStateMachine(&port_entry_p->pfc_sm_state, pfc_event);

    /* call PFC to set new oper config, and oper mode to on */
    switch(sm_action)
    {
    case DCBX_TYPE_SYM_ACTION_INIT:
        /* set local admin config to oper */
#if(SYS_CPNT_PFC == TRUE)
        if(is_admin_from_config_src)
        {
            DCBX_MGR_SetPfcOperEntry(lport, PFC_TYPE_PMODE_ON, src_oper_priority);
            DCBX_MGR_ApplyAllPfcOperEntry(lport, src_oper_priority);
        }
        else
        {
            DCBX_MGR_SetPfcOperEntry(lport, PFC_TYPE_PMODE_ON, admin_priority);
            DCBX_MGR_ApplyAllPfcOperEntry(lport, admin_priority);
        }
#endif
        break;
    case DCBX_TYPE_SYM_ACTION_APPLY_REMOTE:
        /* set remote config to oper */
#if(SYS_CPNT_PFC == TRUE)
        DCBX_MGR_SetPfcOperEntry(lport, PFC_TYPE_PMODE_ON, remote_priority);
        DCBX_MGR_ApplyAllPfcOperEntry(lport, remote_priority);
#endif
        break;
    case DCBX_TYPE_SYM_ACTION_NONE:
    default:
        /* do nothing */
        break;
    }
    ret = DCBX_TYPE_RETURN_OK;
    return ret;
}

static UI32_T DCBX_MGR_ElectConfigSourcePort(UI32_T *lport_p)
{
    UI32_T ret = DCBX_TYPE_RETURN_ERROR;
    DCBX_OM_PortConfigEntry_T * port_entry_p = NULL;
    DCBX_OM_SystemOperEntry_T* sys_oper_p = NULL;

    if(lport_p == NULL)
    {
        return ret;
    }

    sys_oper_p = DCBX_OM_GetSysOper();
    if(sys_oper_p == NULL)
    {
        return ret;
    }

    /*check if there is CfgSrc port */
    if((sys_oper_p->is_cfg_src_selected == TRUE) && (sys_oper_p->is_manual_cfg_src == TRUE))
    {
        *lport_p = sys_oper_p->cfg_src_ifindex;
        ret = DCBX_TYPE_RETURN_OK;
    }
    else
    {
        UI32_T autoup_port = 0;

        /* ther is no port in DCBX_TYPE_PORT_MODE_CFGSRC, check all ports in DCBX_TYPE_PORT_MODE_AUTOUP  */
        for(autoup_port=1; autoup_port <= SYS_ADPT_TOTAL_NBR_OF_LPORT; autoup_port++)
        {
            port_entry_p = DCBX_OM_GetPortConfigEntryPtr(autoup_port);
            if(port_entry_p == NULL)
            {
                continue;
            }

            if((port_entry_p->port_mode == DCBX_TYPE_PORT_MODE_AUTOUP) && (port_entry_p->is_peer_detected == TRUE))
            {
                *lport_p = autoup_port;
                sys_oper_p->is_cfg_src_selected = TRUE;
                sys_oper_p->is_manual_cfg_src = FALSE;
                sys_oper_p->cfg_src_ifindex = *lport_p;
                ret = DCBX_TYPE_RETURN_OK;
                break;
            }
        }
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_MGR_LocalReRunPortStateMachine
 *-------------------------------------------------------------------------
 * PURPOSE  : To re-run port sm when ETS/PFC config is changed.
 * INPUT    : lport       -- lport number
 *            is_ets_chgd -- TRUE if ETS changed/FALSE if PFC changed
 * OUTPUT   : None
 * RETURN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static UI32_T DCBX_MGR_LocalReRunPortStateMachine(
    UI32_T  lport,
    BOOL_T  is_ets_chgd)
{
    DCBX_OM_PortConfigEntry_T   *port_entry_p = NULL;
    DCBX_OM_SystemOperEntry_T   *sys_oper_p = NULL;
#if(SYS_CPNT_PFC == TRUE)
    UI32_T                      pfc_oper_priority = 0;
#endif
#if(SYS_CPNT_ETS == TRUE)
    ETS_TYPE_PortEntry_T        ets_oper_entry;
#endif

    /* 1. get config
     */
    port_entry_p = DCBX_OM_GetPortConfigEntryPtr(lport);
    if (NULL == port_entry_p)
    {
        return DCBX_TYPE_RETURN_ERROR;
    }
    sys_oper_p = DCBX_OM_GetSysOper();
    if (NULL == sys_oper_p)
    {
        return DCBX_TYPE_RETURN_ERROR;
    }

    /* 2. when port mode is manual or port status is disable,
     *    do nothing
     */
    if (  (DCBX_TYPE_PORT_MODE_MANUAL == port_entry_p->port_mode)
        ||(FALSE == port_entry_p->port_status))
    {
        return DCBX_TYPE_RETURN_OK;
    }

    /* 3. when port mode is auto-up/config-source or this port is selected config-source,
     *    elect new configuration source and apply setting
     */
    switch (port_entry_p->port_mode)
    {
    case DCBX_TYPE_PORT_MODE_AUTOUP:
    case DCBX_TYPE_PORT_MODE_CFGSRC:
        /* if peer     exists, notfiy LLDP to send peer data again
         * if peer not exists, apply to all auto down if it's cfg source
         */
        if (TRUE == port_entry_p->is_peer_detected)
        {
            /* notify LLDP to pass ETS/PFC setting to DCBX again.
             */
            LLDP_PMGR_NotifyEtsPfcCfgChanged(lport, is_ets_chgd, !is_ets_chgd);
        }
        else
        {
            if (sys_oper_p->cfg_src_ifindex == lport)
            {
                if (TRUE == is_ets_chgd)
                {
#if(SYS_CPNT_ETS == TRUE)
                    ETS_MGR_GetPortEntry(sys_oper_p->cfg_src_ifindex, &ets_oper_entry, ETS_TYPE_DB_OPER);
                    DCBX_MGR_ApplyAllEtsOperEntry(lport, ets_oper_entry);
#endif
                }
                else
                {
#if(SYS_CPNT_PFC == TRUE)
                    PFC_OM_GetDataByField(sys_oper_p->cfg_src_ifindex, PFC_TYPE_FLDE_PORT_PRI_EN_OPR, &pfc_oper_priority);
                    DCBX_MGR_ApplyAllPfcOperEntry(lport, pfc_oper_priority);
#endif
                }
            }
        }
        break;

    case DCBX_TYPE_PORT_MODE_AUTODOWN:
        /* apply cfg src port setting to this port
         */
        if (TRUE == sys_oper_p->is_cfg_src_selected)
        {
            if (TRUE == is_ets_chgd)
            {
#if(SYS_CPNT_ETS == TRUE)
                ETS_MGR_GetPortEntry(sys_oper_p->cfg_src_ifindex, &ets_oper_entry, ETS_TYPE_DB_OPER);
                ETS_MGR_SetOperConfigEntryByDCBx(lport, ets_oper_entry);
#endif
            }
            else
            {
#if(SYS_CPNT_PFC == TRUE)
                PFC_OM_GetDataByField(sys_oper_p->cfg_src_ifindex, PFC_TYPE_FLDE_PORT_PRI_EN_OPR, &pfc_oper_priority);
                DCBX_MGR_SetPfcOperEntry(lport, PFC_TYPE_PMODE_ON, pfc_oper_priority);
#endif
            }
        }
        break;

    default:
        break;
    }

    return DCBX_TYPE_RETURN_OK;
}

static UI32_T DCBX_MGR_RunPortStateMachine(UI32_T lport)
{
    DCBX_OM_PortConfigEntry_T * port_entry_p = NULL;
    DCBX_OM_SystemOperEntry_T* sys_oper_p = NULL;
#if(SYS_CPNT_PFC == TRUE)
    UI32_T pfc_oper_priority = 0;
#endif
#if(SYS_CPNT_ETS == TRUE)
    ETS_TYPE_PortEntry_T  ets_oper_entry;
#endif

    /* 1. get config
     */
    port_entry_p = DCBX_OM_GetPortConfigEntryPtr(lport);
    if(port_entry_p == NULL)
    {
        return DCBX_TYPE_RETURN_ERROR;
    }
    sys_oper_p = DCBX_OM_GetSysOper();
    if(sys_oper_p == NULL)
    {
        return DCBX_TYPE_RETURN_ERROR;
    }

    /* 2. when port mode is manual or port status is disable,disable state machine
     *    or enable state machine
     */
    if((port_entry_p->port_mode == DCBX_TYPE_PORT_MODE_MANUAL) ||
        (port_entry_p->port_status == FALSE))
    {
        DCBX_MGR_DisableEtsStateMachine(lport);
        DCBX_MGR_DisablePfcStateMachine(lport);
        return DCBX_TYPE_RETURN_OK;
    }
    else
    {
        DCBX_MGR_EnableEtsStateMachine(lport);
        DCBX_MGR_EnablePfcStateMachine(lport);
    }

    /* 3. when port mode is auto-up/config-source or this port is selected config-source,
     *    elect new configuration source and apply setting
     */
    if((port_entry_p->port_mode == DCBX_TYPE_PORT_MODE_AUTOUP) ||
        (port_entry_p->port_mode == DCBX_TYPE_PORT_MODE_CFGSRC) ||
        (lport == sys_oper_p->cfg_src_ifindex))
    {
        UI32_T old_cfg_src_port = 0;
        UI32_T new_cfg_src_port = 0;

        old_cfg_src_port = sys_oper_p->cfg_src_ifindex;
        DCBX_MGR_ElectConfigSourcePort(&new_cfg_src_port);
#if(SYS_CPNT_ETS == TRUE)
        ETS_MGR_GetPortEntry(new_cfg_src_port, &ets_oper_entry, ETS_TYPE_DB_OPER);
#endif
#if(SYS_CPNT_PFC == TRUE)
        PFC_OM_GetDataByField(new_cfg_src_port, PFC_TYPE_FLDE_PORT_PRI_EN_OPR, &pfc_oper_priority);
#endif

        if(old_cfg_src_port == new_cfg_src_port)
        {
            if((port_entry_p->port_mode == DCBX_TYPE_PORT_MODE_AUTODOWN) ||
                (port_entry_p->port_mode == DCBX_TYPE_PORT_MODE_CFGSRC))
            {
                /* apply new setting to this port */
#if(SYS_CPNT_ETS == TRUE)
                ETS_MGR_SetOperConfigEntryByDCBx(lport, ets_oper_entry);
#endif
#if(SYS_CPNT_PFC == TRUE)
                DCBX_MGR_SetPfcOperEntry(lport, PFC_TYPE_PMODE_ON, pfc_oper_priority);
#endif
            }
        }
        else
        {
#if(SYS_CPNT_ETS == TRUE)
            DCBX_MGR_ApplyAllEtsOperEntry(lport, ets_oper_entry);
#endif
#if(SYS_CPNT_PFC == TRUE)
            DCBX_MGR_ApplyAllPfcOperEntry(lport, pfc_oper_priority);
#endif
        }
    }
    else if(port_entry_p->port_mode == DCBX_TYPE_PORT_MODE_AUTODOWN)
    {
        /* when port mode is auto-dwon, apply config-source setting to this port */
        if (sys_oper_p->is_cfg_src_selected == TRUE)
        {
#if(SYS_CPNT_ETS == TRUE)
            ETS_MGR_GetPortEntry(sys_oper_p->cfg_src_ifindex, &ets_oper_entry, ETS_TYPE_DB_OPER);
            ETS_MGR_SetOperConfigEntryByDCBx(lport, ets_oper_entry);
#endif
#if(SYS_CPNT_PFC == TRUE)
            PFC_OM_GetDataByField(sys_oper_p->cfg_src_ifindex, PFC_TYPE_FLDE_PORT_PRI_EN_OPR, &pfc_oper_priority);
            DCBX_MGR_SetPfcOperEntry(lport, PFC_TYPE_PMODE_ON, pfc_oper_priority);
#endif
        }
    }
    return DCBX_TYPE_RETURN_OK;
}

#if(SYS_CPNT_PFC == TRUE)
static BOOL_T DCBX_MGR_SetPfcOperEntry(UI32_T lport, UI32_T oper_mode, UI32_T oper_priority)
{
    PFC_MGR_SetDataByFieldByDCBX(lport, PFC_TYPE_FLDE_PORT_MODE_OPR,   &oper_mode);
    PFC_MGR_SetDataByFieldByDCBX(lport, PFC_TYPE_FLDE_PORT_PRI_EN_OPR, &oper_priority);
    return TRUE;
}

static void DCBX_MGR_ApplyAllPfcOperEntry(UI32_T lport, UI32_T oper_priority)
{
    UI32_T i;
    DCBX_OM_PortConfigEntry_T * port_entry_tmp_p = NULL;
    DCBX_OM_SystemOperEntry_T* sys_oper_p = NULL;

    sys_oper_p = DCBX_OM_GetSysOper();
    if(sys_oper_p == NULL)
    {
        return;
    }

    if(sys_oper_p->cfg_src_ifindex != lport)
    {
        return;
    }

    /* apply new setting to all auto-down ports */
    for(i = 1; i<=SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
    {
        port_entry_tmp_p = DCBX_OM_GetPortConfigEntryPtr(i);
        if(port_entry_tmp_p == NULL)
        {
            continue;
        }

        if((port_entry_tmp_p->port_mode != DCBX_TYPE_PORT_MODE_AUTODOWN) && (port_entry_tmp_p->port_mode != DCBX_TYPE_PORT_MODE_CFGSRC))
        {
                    continue;
        }

        DCBX_MGR_SetPfcOperEntry(i, PFC_TYPE_PMODE_ON, oper_priority);
    }

    return;
}
#endif /* #if(SYS_CPNT_PFC == TRUE) */

#if(SYS_CPNT_ETS == TRUE)
static void DCBX_MGR_ApplyAllEtsOperEntry(
    UI32_T                  lport,
    ETS_TYPE_PortEntry_T    oper_entry)
{
    UI32_T                      i;
    DCBX_OM_PortConfigEntry_T   *port_entry_tmp_p = NULL;
    DCBX_OM_SystemOperEntry_T   *sys_oper_p = NULL;

    sys_oper_p = DCBX_OM_GetSysOper();
    if(sys_oper_p == NULL)
    {
        return;
    }

    if(sys_oper_p->cfg_src_ifindex != lport)
    {
        return;
    }

    /* apply new setting to all auto-down ports */
    for(i = 1; i<=SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
    {
        port_entry_tmp_p = DCBX_OM_GetPortConfigEntryPtr(i);
        if(port_entry_tmp_p == NULL)
        {
            continue;
        }

        if(  (port_entry_tmp_p->port_mode != DCBX_TYPE_PORT_MODE_AUTODOWN)
           &&(port_entry_tmp_p->port_mode != DCBX_TYPE_PORT_MODE_CFGSRC))
        {
            continue;
        }

        ETS_MGR_SetOperConfigEntryByDCBx(i, oper_entry);
    }
}

#endif /* #if(SYS_CPNT_ETS == TRUE) */
