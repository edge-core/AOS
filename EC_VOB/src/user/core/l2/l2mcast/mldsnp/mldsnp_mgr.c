/* MODULE NAME: mldsnp_mgr.C
* PURPOSE:
*    1. What is covered in this file - function and scope}
*    2. Related documents or hardware information}
* NOTES:
*    Something must be known or noticed}
*    1. How to use these functions - give an example}
*    2. Sequence of messages if applicable}
*    3. Any design limitations}
*    4. Any performance limitations}
*    5. Is it a reusable component}
*
* HISTORY:
*    mm/dd/yy (A.D.)
*    12/03/2007     Macauley_Cheng Create
*
* Copyright(C)      Accton Corporation, 2007
*/

/* INCLUDE FILE DECLARATIONS
*/
#include "mldsnp_om.h"
#include "mldsnp_pom.h"
#include "mldsnp_mgr.h"
#include "mldsnp_engine.h"
#include "mldsnp_querier.h"
#include "mldsnp_unknown.h"
#include "mldsnp_timer.h"
#include "mldsnp_backdoor.h"
#include "vlan_mgr.h"
#include "vlan_lib.h"
#include "vlan_om.h"
#include "swctrl.h"
#include "backdoor_mgr.h"
#include "msl_pmgr.h"
#include "swctrl_pmgr.h"
/* NAMING CONSTANT DECLARATIONS
*/
SYSFUN_DECLARE_CSC

/* MACRO FUNCTION DECLARATIONS
*/
#define MLDSNP_MGR_LOCK()
#define MLDSNP_MGR_UNLOCK()

/* DATA TYPE DECLARATIONS
*/
/* LOCAL SUBPROGRAM DECLARATIONS
*/
/* STATIC VARIABLE DEFINITIONS
*/
static UI32_T mldsnp_mgr_old_ticks;

extern UI8_T mldsnp_om_null_src_ip_a[SYS_ADPT_IPV6_ADDR_LEN];
extern UI8_T mldsnp_om_null_group_ip_a[MLDSNP_TYPE_IPV6_DST_IP_LEN];

/* LOCAL SUBPROGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
*/





/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_EnterTransitionMode
*------------------------------------------------------------------------------
* Purpose: This function enter the transition mode
* INPUT  : None
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_MGR_EnterTransitionMode()
{
    SYSFUN_ENTER_TRANSITION_MODE();
    MLDSNP_ENGINE_ClearProxyV2AllocatedMemory();
    return;
}/*End of MLDSNP_MGR_EnterTransitionMode*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_EnterMasterMode
*------------------------------------------------------------------------------
* Purpose: This function enter the master mode
* INPUT  : None
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_MGR_EnterMasterMode()
{
    SYSFUN_ENTER_MASTER_MODE();

    MLDSNP_BD(UI, " ");

    /*if default is enable, it need tell the driver to trap the packet*/
#if(MLDSNP_TYPE_MLDSNP_ENABLED == MLDSNP_TYPE_DFLT_MLDSNP_STATUS)
    {
        #if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
        MLDSNP_TYPE_UnknownBehavior_T flood_behavior;
        #endif

        msl_pmgr_mldsnp_set_status(SYS_DFLT_1Q_PORT_VLAN_PVID, VAL_mldSnoopStatus_enabled);
        #if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
        MLDSNP_OM_GetUnknownFloodBehavior(0, &flood_behavior);
        #endif
        SWCTRL_PMGR_EnableMldPacketTrap(SWCTRL_UNKNOWN_MCAST_TRAP_BY_MLDSNP);
        SWCTRL_PMGR_TrapIpv6PIMToCPU(TRUE, SWCTRL_PIM6_TRAP_BY_MLDSNP);

        #if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
        if (MLDSNP_TYPE_UNKNOWN_BEHAVIOR_FLOOD == flood_behavior)
            SWCTRL_PMGR_TrapUnknownIpv6McastToCPU(FALSE, TRUE, SWCTRL_UNKNOWN_MCAST_TRAP_BY_MLDSNP);
        else
            SWCTRL_PMGR_TrapUnknownIpv6McastToCPU(FALSE, FALSE, SWCTRL_UNKNOWN_MCAST_TRAP_BY_MLDSNP);
        #else
        SWCTRL_PMGR_TrapUnknownIpv6McastToCPU(TRUE, FALSE, SWCTRL_UNKNOWN_MCAST_TRAP_BY_MLDSNP);
        #endif
        MLDSNP_QUERIER_SetMrdSolicitationStatus(TRUE);
        if (MLDSNP_TYPE_QUERIER_ENABLED ==  MLDSNP_TYPE_DFLT_QUERIER_STATUS)
            MLDSNP_QUERIER_EnableQuerier();

    }
#else
    {
        msl_pmgr_mldsnp_set_status(SYS_DFLT_1Q_PORT_VLAN_PVID, VAL_mldSnoopStatus_disabled);
        SWCTRL_PMGR_DisableMldPacketTrap(SWCTRL_UNKNOWN_MCAST_TRAP_BY_MLDSNP);
        SWCTRL_PMGR_TrapUnknownIpv6McastToCPU(FALSE, TRUE, SWCTRL_UNKNOWN_MCAST_TRAP_BY_MLDSNP);
        SWCTRL_PMGR_TrapIpv6PIMToCPU(FALSE, SWCTRL_PIM6_TRAP_BY_MLDSNP);
    }
#endif
    return;
}/*End of MLDSNP_MGR_EnterMasterMode*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_EnterSlaveMode
*------------------------------------------------------------------------------
* Purpose: This function enter the slave mode
* INPUT  : None
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_MGR_EnterSlaveMode()
{
    SYSFUN_ENTER_SLAVE_MODE();

    MLDSNP_BD(UI, " ");

    return;
}/*End of MLDSNP_MGR_EnterSlaveMode*/

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void  MLDSNP_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();

    MLDSNP_BD(UI, " ");

    return;
} /* end of MLDSNP_MGR_SetTransitionMode() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_GetOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE : The function gets operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T MLDSNP_MGR_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
}   /* End of MLDSNP_MGR_GetOperationMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_InitiateProcessResources
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void MLDSNP_MGR_InitiateProcessResources()
{
#ifndef UNIT_TEST
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("mldsnp",
            SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY, MLDSNP_BACKDOOR_MainMenu);
#endif
}/*End of MLDSNP_MGR_InitiateProcessResources*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetMldSnpStatus
*------------------------------------------------------------------------------
* Purpose: This function set the Mld status
* INPUT  : mldsnp_status - the mldsnp status
* OUTPUT : None
 * RETURN  : MLDSNP_TYPE_RETURN_SUCCESS  - success
 *           MLDSNP_TYPE_RETURN_FAIL - failure
* NOTES  :
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetMldSnpStatus(
    MLDSNP_TYPE_MLDSNP_STATUS_T mldsnp_status)
{
    MLDSNP_TYPE_MLDSNP_STATUS_T cur_mldsnp_status;
    UI32_T vid = 0;
    BOOL_T ret = FALSE;

    MLDSNP_BD(UI, "mldsnp_status=%d ", mldsnp_status);

    if ((MLDSNP_TYPE_MLDSNP_ENABLED != mldsnp_status)
            && (MLDSNP_TYPE_MLDSNP_DISABLED != mldsnp_status))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    MLDSNP_OM_GetMldStatus(&cur_mldsnp_status);

    if (cur_mldsnp_status == mldsnp_status)
    {
        return MLDSNP_TYPE_RETURN_SUCCESS;
    }

    if (MLDSNP_TYPE_MLDSNP_ENABLED == mldsnp_status)
    {
        #if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
        MLDSNP_TYPE_UnknownBehavior_T flood_behavior;
        #endif

        while (VLAN_OM_GetNextVlanId(0, &vid))
        {
            msl_pmgr_mldsnp_set_status(vid, VAL_mldSnoopStatus_enabled);
        }
        #if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
        MLDSNP_OM_GetUnknownFloodBehavior(0, &flood_behavior);
        #endif
        SWCTRL_PMGR_EnableMldPacketTrap(SWCTRL_UNKNOWN_MCAST_TRAP_BY_MLDSNP);
        SWCTRL_PMGR_TrapIpv6PIMToCPU(TRUE, SWCTRL_PIM6_TRAP_BY_MLDSNP);

        #if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
        if (MLDSNP_TYPE_UNKNOWN_BEHAVIOR_FLOOD == flood_behavior)
            SWCTRL_PMGR_TrapUnknownIpv6McastToCPU(FALSE, TRUE, SWCTRL_UNKNOWN_MCAST_TRAP_BY_MLDSNP);
        else
            SWCTRL_PMGR_TrapUnknownIpv6McastToCPU(TRUE, FALSE, SWCTRL_UNKNOWN_MCAST_TRAP_BY_MLDSNP);
        #else
            SWCTRL_PMGR_TrapUnknownIpv6McastToCPU(TRUE, FALSE, SWCTRL_UNKNOWN_MCAST_TRAP_BY_MLDSNP);
        #endif
    }
    else
    {
        while (VLAN_OM_GetNextVlanId(0, &vid))
        {
            msl_pmgr_mldsnp_set_status(vid, VAL_mldSnoopStatus_disabled);
        }
        SWCTRL_PMGR_DisableMldPacketTrap(SWCTRL_UNKNOWN_MCAST_TRAP_BY_MLDSNP);
        SWCTRL_PMGR_TrapUnknownIpv6McastToCPU(FALSE, TRUE, SWCTRL_UNKNOWN_MCAST_TRAP_BY_MLDSNP);
        SWCTRL_PMGR_TrapIpv6PIMToCPU(FALSE, SWCTRL_PIM6_TRAP_BY_MLDSNP);
    }

    ret = MLDSNP_ENGINE_SetMLdStatus(mldsnp_status);

    return ret ? MLDSNP_TYPE_RETURN_SUCCESS : MLDSNP_TYPE_RETURN_FAIL;
}/*End of MLDSNP_MGR_SetMldSnpStatus*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetMldSnpVer
*------------------------------------------------------------------------------
* Purpose: This function set the mldsno version
* INPUT  : ver
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetMldSnpVer(
    MLDSNP_TYPE_Version_T  ver)
{
    BOOL_T ret = FALSE;

    MLDSNP_BD(UI, "ver=%d", ver);

    if (MLDSNP_TYPE_VERSION_1 != ver
            && MLDSNP_TYPE_VERSION_2 != ver)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    ret = MLDSNP_OM_SetMldSnpVer(ver);

    if (MLDSNP_TYPE_VERSION_1 == ver)
        MLDSNP_ENGINE_ClearProxyV2AllocatedMemory();

    return ret ? MLDSNP_TYPE_RETURN_SUCCESS : MLDSNP_TYPE_RETURN_FAIL;
}/*End of MLDSNP_MGR_SetMldSnpVer*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetQuerierStatus
*------------------------------------------------------------------------------
* Purpose: This function set the querier status
* INPUT  : querier_status - the querier status
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetQuerierStatus(
    MLDSNP_TYPE_QuerierStatus_T querier_status)
{
    BOOL_T ret = FALSE;
    MLDSNP_TYPE_QuerierStatus_T ori_querier_stauts = MLDSNP_TYPE_QUERIER_DISABLED;
    MLDSNP_TYPE_MLDSNP_STATUS_T cur_mldsnp_status;

    MLDSNP_BD(UI, " querier_status=%d", querier_status);

    if ((MLDSNP_TYPE_QUERIER_ENABLED != querier_status)
            && (MLDSNP_TYPE_QUERIER_DISABLED != querier_status))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (FALSE == MLDSNP_OM_GetQuerierStauts(&ori_querier_stauts))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (ori_querier_stauts == querier_status)
    {
        return MLDSNP_TYPE_RETURN_SUCCESS;
    }

    ret = MLDSNP_OM_SetQuerierStatus(querier_status);

    MLDSNP_OM_GetMldStatus(&cur_mldsnp_status);

    if (MLDSNP_TYPE_QUERIER_ENABLED == querier_status)
    {
        if (MLDSNP_TYPE_MLDSNP_ENABLED == cur_mldsnp_status)
        {
            ret = MLDSNP_QUERIER_EnableQuerier();

            /*check shall recovery static group or not*/
            MLDSNP_ENGINE_RecoveryStaticJoin(0);
        }
    }
    else
    {
        UI32_T next_vid = 0;

        ret = MLDSNP_QUERIER_DisableQuerier();

        /*check shall delete static group or not*/
        if (FALSE == MLDSNP_OM_IsMrouteEnabled())
        {
            while (VLAN_OM_GetNextVlanId(0, &next_vid))
            {
                if (MLDSNP_OM_GetVlanRouterPortCount(next_vid) == 0)
                    MLDSNP_ENGINE_DeleteAllGroupInVlan(next_vid);
            }
        }
    }


    return ret ? MLDSNP_TYPE_RETURN_SUCCESS : MLDSNP_TYPE_RETURN_FAIL;
}/*End of MLDSNP_MGR_SetQuerierStatus*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetRobustnessValue
*------------------------------------------------------------------------------
* Purpose: This function set the robust ess value
* INPUT  : value - the robustness value
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :  7.1, 7.9, 9.1, 9.9
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetRobustnessValue(
    UI16_T  value)
{
    BOOL_T ret = FALSE;

    MLDSNP_BD(UI, " robust value=%d", value);

    if (value < MLDSNP_TYPE_MIN_ROBUSTNESS
            || value > MLDSNP_TYPE_MAX_ROBUSTNESS)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    ret = MLDSNP_OM_SetRobustnessValue(value);

    {/*upate oper robust value*/
        MLDSNP_OM_VlanInfo_T vlan_info;
        UI16_T nxt_vid = 0;

        while (TRUE == MLDSNP_OM_GetNextVlanInfo(&nxt_vid , &vlan_info))
        {
            if (MLDSNP_TYPE_QUERIER_ENABLED == vlan_info.querier_runing_status)
            {
                vlan_info.robust_oper_value = value;
                ret = MLDSNP_OM_SetVlanInfo(&vlan_info);
            }
        }
    }
    return ret ? MLDSNP_TYPE_RETURN_SUCCESS : MLDSNP_TYPE_RETURN_FAIL;
}/*End of MLDSNP_MGR_SetRobustnessValue*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetQueryInterval
*------------------------------------------------------------------------------
* Purpose: This funcgtion set the query interval
* INPUT  : interval - the interval in seconds
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :   7.2, 9.2
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetQueryInterval(
    UI16_T  interval)
{
    BOOL_T ret = FALSE;

    MLDSNP_BD(UI, "query_interval=%d", interval);

    if (interval > MLDSNP_TYPE_MAX_QUERY_INTERVAL
            || interval < MLDSNP_TYPE_MIN_QUERY_INTERVAL)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    ret = MLDSNP_QUEIRER_ChangeQueryInterval(interval);

    {/*upate oper query interval*/
        MLDSNP_OM_VlanInfo_T vlan_info;
        UI16_T nxt_vid = 0;

        while (TRUE == MLDSNP_OM_GetNextVlanInfo(&nxt_vid , &vlan_info))
        {
            if (MLDSNP_TYPE_QUERIER_ENABLED == vlan_info.querier_runing_status)
            {
                vlan_info.query_oper_interval = interval;
                MLDSNP_OM_SetVlanInfo(&vlan_info);
            }
        }
    }

    return ret ? MLDSNP_TYPE_RETURN_SUCCESS : MLDSNP_TYPE_RETURN_FAIL;
}/*End of MLDSNP_MGR_SetQueryInterval*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetQueryResponseInterval
*------------------------------------------------------------------------------
* Purpose: This function get the query response interval
* INPUT  : interval - the query repsonse interval in seconds
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :   7.3, 9.3
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetQueryResponseInterval(
    UI16_T  interval)
{
    UI16_T  query_interval = 0;
    BOOL_T  ret            = FALSE;

    MLDSNP_BD(UI, "response interval=%d", interval);

    if (interval > MLDSNP_TYPE_MAX_RESPONSE_INTERVAL
            || interval < MLDSNP_TYPE_MIN_RESPONSE_INTERVAL)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (FALSE == MLDSNP_OM_GetQueryInterval(&query_interval))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (interval >= query_interval)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    ret = MLDSNP_OM_SetQueryRresponseInterval(interval);

    return ret ? MLDSNP_TYPE_RETURN_SUCCESS : MLDSNP_TYPE_RETURN_FAIL;
}/*End of MLDSNP_MGR_SetQueryResponseInterval*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetLastListenerQueryInterval
*------------------------------------------------------------------------------
* Purpose: This function set the last listener query interval
* INPUT  : interval  - the interval in second
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :  7.8, 9.8
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetLastListenerQueryInterval(
    UI16_T  interval)
{
    BOOL_T ret = FALSE;

    MLDSNP_BD(UI, " last listner query interval=%d", interval);

    ret = MLDSNP_OM_SetLastListnerQueryInterval(interval);

    return ret ? MLDSNP_TYPE_RETURN_SUCCESS : MLDSNP_TYPE_RETURN_FAIL;
}/*End of MLDSNP_MGR_SetLastListenerQueryInterval*/

#if (SYS_CPNT_MLDSNP_PROXY == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_SetProxyReporting
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set MLDSNP proxy reporting status.
 * INPUT   :  status - setting value
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *    FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
MLDSNP_TYPE_ReturnValue_T
MLDSNP_MGR_SetProxyReporting(MLDSNP_TYPE_ProxyReporting_T proxy_status)
{
    MLDSNP_TYPE_ProxyReporting_T old_status = MLDSNP_TYPE_PROXY_REPORTING_DISABLE;
    MLDSNP_OM_VlanInfo_T vlan_info;
    UI16_T nxt_vid = 0;

    if (MLDSNP_TYPE_PROXY_REPORTING_DISABLE != proxy_status
            && MLDSNP_TYPE_PROXY_REPORTING_ENABLE != proxy_status)
        return MLDSNP_TYPE_RETURN_FAIL;

    if (MLDSNP_OM_GetProxyReporting(&old_status)
            && old_status == proxy_status)
        return MLDSNP_TYPE_RETURN_SUCCESS;

    MLDSNP_OM_SetProxyReporting(proxy_status);

    if (proxy_status == MLDSNP_TYPE_PROXY_REPORTING_ENABLE)
    {
        MLDSNP_TYPE_MLDSNP_STATUS_T cur_mldsnp_status;

        MLDSNP_ENGINE_StartUnsolicittimer(TRUE);

        MLDSNP_ENGINE_ProxySendUnsolicitReports(0);

        MLDSNP_OM_GetMldStatus(&cur_mldsnp_status);

        if (MLDSNP_TYPE_MLDSNP_ENABLED == cur_mldsnp_status)
        {
            while (TRUE == MLDSNP_OM_GetNextVlanInfo(&nxt_vid , &vlan_info))
            {
                if (vlan_info.querier_uptime == 0)
                {
                    vlan_info.querier_uptime = SYSFUN_GetSysTick();
                    MLDSNP_OM_SetVlanInfo(&vlan_info);
                }
            }
        }
    }
    else
    {
        MLDSNP_ENGINE_StartUnsolicittimer(FALSE);

        while (TRUE == MLDSNP_OM_GetNextVlanInfo(&nxt_vid , &vlan_info))
        {
            if (vlan_info.querier_runing_status == MLDSNP_TYPE_QUERIER_DISABLED)
            {
                vlan_info.querier_uptime = 0;
                MLDSNP_OM_SetVlanInfo(&vlan_info);
            }
        }
    }
    return MLDSNP_TYPE_RETURN_SUCCESS;
}
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetUnsolicitedReportInterval
*------------------------------------------------------------------------------
* Purpose: This function set the unsolicitedReportInterval
* INPUT  : interval  - the inteval in seconds
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :  7.9, 9.11
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetUnsolicitedReportInterval(
    UI32_T  interval)
{
    MLDSNP_TYPE_ProxyReporting_T proxy_status;

    MLDSNP_BD(UI, "unsolicited interval=%d", interval);

    if (interval < SYS_ADPT_MLD_MIN_UNSOLICITED_REPORT_INTERVAL
            || interval > SYS_ADPT_MLD_MAX_UNSOLICITED_REPORT_INTERVAL)
        return MLDSNP_TYPE_RETURN_FAIL;

    MLDSNP_OM_SetUnsolicitedReportInterval(interval);

    if (MLDSNP_OM_GetProxyReporting(&proxy_status)
            && MLDSNP_TYPE_PROXY_REPORTING_DISABLE == proxy_status)
        MLDSNP_ENGINE_StartUnsolicittimer(FALSE);
    else
        MLDSNP_ENGINE_StartUnsolicittimer(TRUE);

    return MLDSNP_TYPE_RETURN_SUCCESS;
}/*End of MLDSNP_MGR_SetUnsolicitedReportInterval*/
#endif

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetRouterExpireTime
*------------------------------------------------------------------------------
* Purpose: This function set the router expire time
* INPUT  : exp_time  - the expire time
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetRouterExpireTime(
    UI16_T  exp_time)
{
    BOOL_T ret = FALSE;

    MLDSNP_BD(UI, "router expire time=%d", exp_time);

    if (exp_time > MLDSNP_TYPE_MAX_ROUTER_EXPIRE_TIME
            || exp_time < MLDSNP_TYPE_MIN_ROUTER_EXPIRE_TIME)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    ret = MLDSNP_OM_SetRouterExpireTime(exp_time);

    return ret ? MLDSNP_TYPE_RETURN_SUCCESS : MLDSNP_TYPE_RETURN_FAIL;
}/*End of MLDSNP_MGR_SetRouterExpireTime*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetUnknownFloodBehavior
*------------------------------------------------------------------------------
* Purpose: This function Set the unknown multicast data flood behavior
* INPUT  : flood_behavior - the returned router port info
*          vlan_id        - vlan id
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetUnknownFloodBehavior(
    UI32_T vlan_id,
    MLDSNP_TYPE_UnknownBehavior_T flood_behavior)
{
    BOOL_T ret = FALSE;

    MLDSNP_BD(UI, "vlan %lu, behavior %d", vlan_id, flood_behavior);

    if (MLDSNP_TYPE_UNKNOWN_BEHAVIOR_FLOOD != flood_behavior
            && MLDSNP_TYPE_UNKNOWN_BEHAVIOR_TO_ROUTER_PORT != flood_behavior)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    #if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == TRUE)
    if(vlan_id > SYS_ADPT_MAX_VLAN_ID)
      return MLDSNP_TYPE_RETURN_FAIL;
    if(vlan_id !=0)
      ret = MLDSNP_UNKNOWN_SetFloodBehavior(vlan_id, flood_behavior);
    else
    {
      vlan_id =0;
      while(TRUE == VLAN_OM_GetNextVlanId(0, &vlan_id))
      {
      ret = MLDSNP_UNKNOWN_SetFloodBehavior(vlan_id, flood_behavior);
        if(FALSE == ret)
          return MLDSNP_TYPE_RETURN_FAIL;
      }
    }
    #else
    vlan_id =0;
      while(TRUE == VLAN_OM_GetNextVlanId(0, &vlan_id))
      {
        ret = MLDSNP_UNKNOWN_SetFloodBehavior(vlan_id, flood_behavior);
        if(FALSE == ret)
          return MLDSNP_TYPE_RETURN_FAIL;
      }
    #endif

    #if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
    MLDSNP_OM_SetUnknownFloodBehavior(0, flood_behavior);
    #endif

    if (ret)
    {
      #if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
        if (MLDSNP_TYPE_UNKNOWN_BEHAVIOR_FLOOD == flood_behavior)
            SWCTRL_PMGR_TrapUnknownIpv6McastToCPU(FALSE, TRUE, SWCTRL_UNKNOWN_MCAST_TRAP_BY_MLDSNP);
        else
            SWCTRL_PMGR_TrapUnknownIpv6McastToCPU(TRUE, FALSE, SWCTRL_UNKNOWN_MCAST_TRAP_BY_MLDSNP);
      #endif
    }
    return ret ? MLDSNP_TYPE_RETURN_SUCCESS : MLDSNP_TYPE_RETURN_FAIL;
}/*End of MLDSNP_MGR_SetUnknownFloodBehavior*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetImmediateLeaveStatus
*------------------------------------------------------------------------------
* Purpose: This function Set the immediate leave status
* INPUT  : vid                       - the vlan id
*          immediate_leave_status - the returned router port info
*
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetImmediateLeaveStatus(
    UI16_T                        vid,
    MLDSNP_TYPE_ImmediateStatus_T immediate_leave_status)
{
    BOOL_T ret = FALSE;

    MLDSNP_BD(UI, "immeidate leave status=%d", immediate_leave_status);

    if (vid < 1
            || vid > SYS_ADPT_MAX_NBR_OF_VLAN)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if ((immediate_leave_status != MLDSNP_TYPE_IMMEDIATE_DISABLED)
            && (immediate_leave_status  != MLDSNP_TYPE_IMMEDIATE_ENABLED))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    ret = MLDSNP_OM_SetImmediateLeaveStatus(vid, immediate_leave_status);

    return ret ? MLDSNP_TYPE_RETURN_SUCCESS : MLDSNP_TYPE_RETURN_FAIL;
}/*End of MLDSNP_MGR_SetImmediateLeaveStatus*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetImmediateLeaveByHostStatus
*------------------------------------------------------------------------------
* Purpose: This function Set the immediate leave by-host-ip status
* INPUT  : vid                           - the vlan id
*          immediate_leave_byhost_status - the immediate leave by-host-ip
*
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetImmediateLeaveByHostStatus(
    UI16_T                        vid,
    MLDSNP_TYPE_ImmediateByHostStatus_T immediate_leave_byhost_status)
{
    BOOL_T ret = FALSE;

    MLDSNP_BD(UI, "immeidate leave status by host=%d", immediate_leave_byhost_status);

    if (vid < 1
            || vid > SYS_ADPT_MAX_NBR_OF_VLAN)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if ((immediate_leave_byhost_status != MLDSNP_TYPE_IMMEDIATE_BYHOST_DISABLED)
            && (immediate_leave_byhost_status  != MLDSNP_TYPE_IMMEDIATE_BYHOST_ENABLED))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    ret = MLDSNP_OM_SetImmediateLeaveByHostStatus(vid, immediate_leave_byhost_status);

    return ret ? MLDSNP_TYPE_RETURN_SUCCESS : MLDSNP_TYPE_RETURN_FAIL;
}/*End of MLDSNP_MGR_SetImmediateLeaveByHostStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_SetPortListStaticJoinGroup
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set static port join group
 * INPUT   : vid            - the vlan id
 *           *gip_ap        - the group ip
 *           *sip_ap        - the source ip
 *           *port_list_ap  - the static port list
 * OUTPUT  : None
 * RETURN  : MLDSNP_TYPE_RETURN_SUCCESS  - success
 *           MLDSNP_TYPE_RETURN_FAIL - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetPortListStaticJoinGroup(
    UI16_T  vid,
    UI8_T   *gip_ap,
    UI8_T   *sip_ap,
    UI8_T   *port_list_ap)
{
    MLDSNP_OM_GroupInfo_T group_info;
    UI32_T vid_ifindex = 0;
    UI16_T idx = 0;
    BOOL_T add_port = FALSE, port_exit = FALSE;

    if ((NULL == gip_ap)
            || (NULL == sip_ap))
    {
        MLDSNP_BD(UI, "gip_ap or sip_ap is NULL");
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    MLDSNP_BD_SHOW_GROUP_SRC(UI, "vid:%d", gip_ap, sip_ap, 1, vid);

    /*Check if the mip is valid multicast ip address */
    if (FALSE == MLDSNP_ENGINE_IsLegalGroupIP(gip_ap))
    {
        MLDSNP_BD(UI, "not multicast address");
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (FALSE == VLAN_OM_IsVlanExisted(vid))
    {
        MLDSNP_BD(UI, "Vid is not exist");
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (FALSE == VLAN_OM_ConvertToIfindex(vid, &vid_ifindex))
    {
        MLDSNP_BD(UI, "conver vid ot ifindex is failed");
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    MLDSNP_OM_GetGroupPortlist(vid, gip_ap, sip_ap, &group_info);

    for (idx = 1; idx <= SYS_ADPT_TOTAL_NBR_OF_LPORT; idx ++)
    {
        port_exit = MLDSNP_TYPE_IsPortInPortBitMap(idx, group_info.static_port_bitmap);
        add_port  = MLDSNP_TYPE_IsPortInPortBitMap(idx, port_list_ap);

        if (add_port)
        {
            if (SWCTRL_LogicalPortExisting(idx) == FALSE)
            {
                MLDSNP_BD(UI, "port %d not logical port", idx);
                return MLDSNP_TYPE_RETURN_FAIL;

            }

            if (FALSE == VLAN_OM_IsPortVlanMember(vid_ifindex, idx))
            {
                MLDSNP_BD(UI, "port %d not vlan member failed", idx);
                return MLDSNP_TYPE_RETURN_FAIL;
            }
        }

        if ((FALSE == port_exit)
                && (TRUE == add_port))
        {
            if (TRUE == MLDSNP_OM_IsExistStaticPortJoinGroup(vid, gip_ap, sip_ap, idx))
            {
                MLDSNP_BD(UI, "already exist");
                continue;
            }

            if (FALSE == MLDSNP_OM_AddStaticPortJoinGroup(vid, gip_ap, sip_ap, idx, MLDSNP_TYPE_IS_EXCLUDE_MODE))
            {
                MLDSNP_BD(UI, "can't save to om");
                return MLDSNP_TYPE_RETURN_FAIL;
            }

            /* add to hisam entry only when vlan is active, otherwise only save to om */
            if (FALSE == MLDSNP_ENGINE_IsVlanActive(vid))
                continue;

            if (0 == memcmp(sip_ap, mldsnp_om_null_src_ip_a, MLDSNP_TYPE_IPV6_SRC_IP_LEN))
            {
                if (FALSE == MLDSNP_ENGINE_AddPortStaticJoinGroup(vid, gip_ap, sip_ap, idx, MLDSNP_TYPE_IS_EXCLUDE_MODE))
                {
                    MLDSNP_BD(UI, " add port %d is failed", idx);

                    return MLDSNP_TYPE_RETURN_FAIL;
                }
            }
            else
            {
                if (FALSE == MLDSNP_ENGINE_AddPortStaticJoinGroup(vid, gip_ap, sip_ap, idx, MLDSNP_TYPE_IS_INCLUDE_MODE))
                {
                    MLDSNP_BD(UI, " add port %d is failed", idx);

                    return MLDSNP_TYPE_RETURN_FAIL;
                }
            }
        }
        else if ((TRUE == port_exit)
                 && (FALSE == add_port))
        {
            if (FALSE ==  MLDSNP_OM_DeleteStaticPortJoinGroup(vid, gip_ap, sip_ap, idx))
            {
                MLDSNP_BD(UI, "can't delete om");
                return MLDSNP_TYPE_RETURN_FAIL;
            }
            
            /* delete hisam entry only when vlan is active, otherwise only delete om */
            if (FALSE == MLDSNP_ENGINE_IsVlanActive(vid))
                continue;
        
            if (FALSE == MLDSNP_ENGINE_DeletePortStaticJoinGroup(vid, gip_ap, sip_ap, idx))
            {
                MLDSNP_BD(UI, "delete port %d is failed", idx);

                return MLDSNP_TYPE_RETURN_FAIL;
            }
        }
    }

    return MLDSNP_TYPE_RETURN_SUCCESS;
}/*End of MLDSNP_MGR_SetPortListStaticJoinGroup*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_SetPortListStaticLeaveGroup
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set static port join group
 * INPUT   : vid          - the vlan id
 *           *gip_ap      - the group ip
 *           *sip_ap      - the source ip
 *           *port_list_ap- the static port list
 * OUTPUT  : None
 * RETURN  : MLDSNP_TYPE_RETURN_SUCCESS  - success
 *           MLDSNP_TYPE_RETURN_FAIL     - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetPortListStaticLeaveGroup(
    UI16_T  vid,
    UI8_T   *gip_ap,
    UI8_T   *sip_ap,
    UI8_T   *port_list_ap)
{
    UI16_T   idx = 0;
    BOOL_T delete_port = FALSE;

    if ((NULL == gip_ap)
            || (NULL == sip_ap))
    {
        MLDSNP_BD(UI, "gip_ap or sip_ap is NULL");
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    MLDSNP_BD_SHOW_GROUP_SRC(UI, "vid:%d", gip_ap, sip_ap, 1, vid);

    for (idx = 1; idx <= SYS_ADPT_TOTAL_NBR_OF_LPORT; idx++)
    {
        delete_port = MLDSNP_TYPE_IsPortInPortBitMap(idx, port_list_ap);
        if (delete_port)
        {
            if (FALSE == MLDSNP_ENGINE_DeletePortStaticJoinGroup(vid, gip_ap, sip_ap, idx))
            {
                MLDSNP_BD(UI, "fail to delete port %d from group", idx);

                return MLDSNP_TYPE_RETURN_FAIL;
            }
        }
    }
    return MLDSNP_TYPE_RETURN_SUCCESS;
}/*End of MLDSNP_MGR_SetPortListStaticLeaveGroup*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_AddPortStaticJoinGroup
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set static port join group
 * INPUT   : vid          - the vlan id
 *           *gip_ap      - the group ip
 *           *sip_ap      - the source ip
 *           lport        - the static port list
 *           rec_type     - the include or exclude mode
 * OUTPUT  : None
 * RETURN  : MLDSNP_TYPE_RETURN_SUCCESS  - success
 *           MLDSNP_TYPE_RETURN_FAIL - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_AddPortStaticJoinGroup(
    UI16_T  vid,
    UI8_T *gip_ap,
    UI8_T *sip_ap,
    UI16_T  lport,
    MLDSNP_TYPE_RecordType_T rec_type)
{
    MLDSNP_BD_SHOW_GROUP_SRC(UI, "vid:%d", gip_ap, sip_ap, 1, vid);

    if ((NULL == gip_ap)
            || (NULL == sip_ap))
    {
        MLDSNP_BD(UI, "gip_ap or sip_ap is NULL");
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (MLDSNP_TYPE_IS_INCLUDE_MODE != rec_type
            && MLDSNP_TYPE_IS_EXCLUDE_MODE != rec_type)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (SWCTRL_LogicalPortExisting(lport) == FALSE)
    {
        MLDSNP_BD(UI, "lport not exist");
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    /*Check if the mip is valid multicast ip address */
    if (FALSE == MLDSNP_ENGINE_IsLegalGroupIP(gip_ap))
    {
        MLDSNP_BD(UI, "not multicast address");
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    /*if include rec_type, it shall specify src ip*/
    if (0 == memcmp(sip_ap, mldsnp_om_null_src_ip_a, MLDSNP_TYPE_IPV6_SRC_IP_LEN)
            && MLDSNP_TYPE_IS_INCLUDE_MODE == rec_type)
    {
        MLDSNP_BD(UI, "exclude rec_type by no src IP");
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    {
        UI32_T  vid_ifindex = 0;

        if (FALSE == VLAN_OM_ConvertToIfindex(vid, &vid_ifindex))
        {
            MLDSNP_BD(UI, "can't conver vid %d to vidifindex", vid);
            return MLDSNP_TYPE_RETURN_FAIL;
        }

        if (FALSE == VLAN_OM_IsPortVlanMember(vid_ifindex, lport))
        {
            MLDSNP_BD(UI, "lport %d is not vlan %d member", lport, vid);
            return MLDSNP_TYPE_RETURN_FAIL;
        }
    }

    if (TRUE == MLDSNP_OM_IsExistStaticPortJoinGroup(vid, gip_ap, sip_ap, lport))
    {
        MLDSNP_BD(UI, "already exist");
        return MLDSNP_TYPE_RETURN_SUCCESS;
    }

    if (FALSE == MLDSNP_OM_AddStaticPortJoinGroup(vid, gip_ap, sip_ap, lport, rec_type))
    {
        MLDSNP_BD(UI, "can't save to om");
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (FALSE == MLDSNP_ENGINE_IsVlanActive(vid))
        return MLDSNP_TYPE_RETURN_SUCCESS;
    
    if (FALSE == MLDSNP_ENGINE_AddPortStaticJoinGroup(vid, gip_ap, sip_ap, lport, rec_type))
    {
        MLDSNP_BD(UI, "port %d static join group fail", lport);
        /*delete from OM*/
        MLDSNP_OM_DeleteStaticPortJoinGroup(vid, gip_ap, sip_ap, lport);
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    return MLDSNP_TYPE_RETURN_SUCCESS;
}   /* End of MLDSNP_MGR_AddPortStaticJoinGroup() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_DeletePortStaticJoinGroup
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to delete static port from group
 * INPUT   : vid          - the vlan id
 *           *gip_ap      - the group ip
 *           *sip_ap      - the source ip
 *           lport        - the logical port
 * OUTPUT  : None
 * RETURN  : MLDSNP_TYPE_RETURN_SUCCESS  - success
 *           MLDSNP_TYPE_RETURN_FAIL     - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_DeletePortStaticJoinGroup(
    UI16_T  vid,
    UI8_T   *gip_ap,
    UI8_T   *sip_ap,
    UI16_T  lport)
{
    MLDSNP_BD_SHOW_GROUP_SRC(UI, "vid:%d", gip_ap, sip_ap, 1, vid);

    if ((NULL == gip_ap)
            || (NULL == sip_ap))
    {
        MLDSNP_BD(UI, "gip_ap or sip_ap is NULL");
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (SWCTRL_LogicalPortExisting(lport) == FALSE)
    {
        MLDSNP_BD(UI, "lport not exist");
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (vid < 1
            || vid > SYS_ADPT_MAX_VLAN_ID)
    {
        MLDSNP_BD(UI, "vid %d out of rang", vid);
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    /*Check if the mip is valid multicast ip address */
    if (FALSE == MLDSNP_ENGINE_IsLegalGroupIP(gip_ap))
    {
        MLDSNP_BD(UI, "not multicast address");
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (FALSE == MLDSNP_OM_DeleteStaticPortJoinGroup(vid, gip_ap, sip_ap, lport))
    {
        MLDSNP_BD(UI, "can't delete from om");
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    {
        MLDSNP_TYPE_MLDSNP_STATUS_T mldsnp_status;

        MLDSNP_OM_GetMldStatus(&mldsnp_status);

        /*when mldsnp disable the group already been removed.*/
        if (MLDSNP_TYPE_MLDSNP_DISABLED == mldsnp_status)
            return MLDSNP_TYPE_RETURN_SUCCESS;
    }
    if (FALSE == MLDSNP_ENGINE_DeletePortStaticJoinGroup(vid, gip_ap, sip_ap, lport))
    {
        MLDSNP_BD(UI, "fail to delete port join group");

        return MLDSNP_TYPE_RETURN_FAIL;
    }

    return MLDSNP_TYPE_RETURN_SUCCESS;
}/*End of MLDSNP_MGR_DeletePortStaticJoinGroup*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_AddStaticRouterPort
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to add the static router port
 * INPUT   : vid        - the vlan id
 *           lport      - the logical port
 * OUTPUT  : None
 * RETURN  : MLDSNP_TYPE_RETURN_SUCCESS  - success
 *           MLDSNP_TYPE_RETURN_FAIL     - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_AddStaticRouterPort(
    UI16_T  vid,
    UI16_T  lport)
{
    UI32_T  vid_ifindex = 0;

    MLDSNP_BD(UI, "vid:%d, lport:%d",  vid, lport);

    if (SWCTRL_LogicalPortExisting(lport) == FALSE)
    {
        MLDSNP_BD(UI, "vid:%d, lport:%d, lport not exist", vid, lport);
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (FALSE == VLAN_OM_ConvertToIfindex(vid, &vid_ifindex))
    {
        MLDSNP_BD(UI, "vid:%d, lport:%d, vlan ifindex is wrong", vid, lport);
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (FALSE == VLAN_OM_IsPortVlanMember(vid_ifindex, lport))
    {
        MLDSNP_BD(UI, "vid:%d, lport:%d, port not vlan member", vid, lport);
        return MLDSNP_TYPE_RETURN_FAIL;
    }


    if (FALSE == MLDSNP_QUERIER_AddStaticRouterPort(vid, lport))
    {
        MLDSNP_BD(UI, "vid:%d, lport:%d, add router port fail", vid, lport);
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (MLDSNP_ENGINE_IsVlanActive(vid))
        MLDSNP_ENGINE_RecoveryStaticJoin(vid);

    return MLDSNP_TYPE_RETURN_SUCCESS;
}/*End of MLDSNP_MGR_AddStaticRouterPort*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetStaticRouterPortlist
*------------------------------------------------------------------------------
* Purpose: This function set the static router port list
* INPUT  : *port_list_ap  - the static router port list
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetStaticRouterPortlist(
    UI16_T  vid,
    UI8_T   *port_list_ap)
{
    UI32_T vid_ifindex = 0;
    UI16_T idx = 0;
    UI8_T current_r_port_bitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    BOOL_T router_port_exist = FALSE, add_port = FALSE;

    MLDSNP_BD(UI, "vid:%d", vid);

    if (vid < 1
            || vid > SYS_ADPT_MAX_VLAN_ID)
    {
        MLDSNP_BD(UI, "vid:%d out of rang", vid);
        return MLDSNP_TYPE_RETURN_FAIL;
    }
    {/*get all router port current configured*/
        MLDSNP_OM_RouterPortInfo_T r_port_info;
        MLDSNP_OM_VlanInfo_T       vlan_info;

        if (FALSE == MLDSNP_OM_GetVlanInfo(vid, &vlan_info))
        {
            MLDSNP_BD(UI, "get vlan info fail");
            return MLDSNP_TYPE_RETURN_FAIL;
        }
        while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &r_port_info))
        {
            if (MLDSNP_TYPE_JOIN_STATIC == r_port_info.attribute)
                MLDSNP_TYPE_AddPortIntoPortBitMap(r_port_info.port_no, current_r_port_bitmap);
        }
    }

    if (FALSE == VLAN_OM_ConvertToIfindex(vid, &vid_ifindex))
    {
        MLDSNP_BD(UI, "can't conver to vid ifindex");
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    for (idx = 1; idx <= SYS_ADPT_TOTAL_NBR_OF_LPORT; idx ++)
    {
        router_port_exist = MLDSNP_TYPE_IsPortInPortBitMap(idx, current_r_port_bitmap);
        add_port          = MLDSNP_TYPE_IsPortInPortBitMap(idx, port_list_ap);

        if (add_port)
        {
            if (SWCTRL_LogicalPortExisting(idx) == FALSE)
            {
                return MLDSNP_TYPE_RETURN_FAIL;
            }

            if (FALSE == VLAN_OM_IsPortVlanMember(vid_ifindex, idx))
            {
                return MLDSNP_TYPE_RETURN_FAIL;
            }
        }

        if ((TRUE == router_port_exist)
                && (FALSE == add_port))
        {
            if (FALSE == MLDSNP_QUERIER_DeleteStaticRouterPort(vid, idx))
            {
                MLDSNP_BD(UI, "can't delete router port %d, vid %d", idx, vid);
            }
        }
        else if ((FALSE == router_port_exist)
                 && (TRUE == add_port))
        {
            if (FALSE == MLDSNP_QUERIER_AddStaticRouterPort(vid, idx))
            {
                MLDSNP_BD(UI, "can't add router port %d, vid %d", idx, vid);
            }
        }

    }

    if (TRUE == MLDSNP_ENGINE_IsVlanActive(vid))
        MLDSNP_ENGINE_RecoveryStaticJoin(vid);

    return MLDSNP_TYPE_RETURN_SUCCESS;
}/*End of MLDSNP_MGR_SetStaticRouterPortlist*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_DeleteStaticRouterPort
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to delete the static router port
 * INPUT   : vid        - the vlan id
 *           lport      - the logical port
 * OUTPUT  : None
 * RETURN  : MLDSNP_TYPE_RETURN_SUCCESS  - success
 *           MLDSNP_TYPE_RETURN_FAIL     - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_DeleteStaticRouterPort(
    UI16_T  vid,
    UI16_T  lport)
{
    MLDSNP_BD(UI, "vid:%d, lport:%d", vid, lport);

    if (vid < 1
            || vid > SYS_ADPT_MAX_VLAN_ID)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (SWCTRL_LogicalPortExisting(lport) == FALSE)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }


    if (FALSE == MLDSNP_QUERIER_DeleteStaticRouterPort(vid, lport))
    {

        return MLDSNP_TYPE_RETURN_FAIL;
    }

    return MLDSNP_TYPE_RETURN_SUCCESS;
}/*End of MLDSNP_MGR_DeleteStaticRouterPort*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_DeleteStaticRouterPortlist
*------------------------------------------------------------------------------
* Purpose: This function set the static router port list
* INPUT  : vid           - the vlan id
*          *port_list_ap - the static router port list
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_DeleteStaticRouterPortlist(
    UI16_T  vid,
    UI8_T   *port_list_ap)
{
    UI32_T idx = 0, vid_ifindex = 0;
    BOOL_T delete_port = FALSE;

    MLDSNP_BD(UI, "vid:%d", vid);

    if (vid < 1
            || vid > SYS_ADPT_MAX_VLAN_ID)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (FALSE == VLAN_OM_ConvertToIfindex(vid, &vid_ifindex))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    for (idx = 1; idx <= SYS_ADPT_TOTAL_NBR_OF_LPORT; idx++)
    {
        delete_port = MLDSNP_TYPE_IsPortInPortBitMap(idx, port_list_ap);

        if (delete_port)
        {
            if (SWCTRL_LogicalPortExisting(idx) == FALSE)
            {
                return MLDSNP_TYPE_RETURN_FAIL;
            }

            if (FALSE == VLAN_OM_IsPortVlanMember(vid_ifindex, idx))
            {
                return MLDSNP_TYPE_RETURN_FAIL;
            }

            MLDSNP_QUERIER_DeleteStaticRouterPort(vid, idx);
        }
    }

    return MLDSNP_TYPE_RETURN_SUCCESS;
}/*End of MLDSNP_MGR_DeleteStaticRouterPortlist*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_ProcessRcvdMldPdu
*------------------------------------------------------------------------------
* PURPOSE: Whenever Network Interface received a LACPDU packet,it calls
*  this function to handle the packet.
* INPUT  :
*    L_MM_Mref_Handle_T *mref_handle_p -- packet buffer and return buffer function pointer.
*    UI8_T  *dst_mac -- the destination MAC address of this packet.
*    UI8_T  *src_mac -- the source MAC address of this packet.
*    UI16_T tag_info -- tag information
*    UI16_T type
*    UI32_T  pkt_length -- the length of the packet payload.
*    UI32_T  ip_ext_opt_len--the extension length in ip header
*    UI32_T unit_no    -- user view unit number
*    UI32_T port_no    -- user view port number
* OUTPUT : None
* RETURN : None
* NOTE:
*------------------------------------------------------------------------------*/

void MLDSNP_MGR_ProcessRcvdMldPdu(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI8_T              dst_mac[6],
    UI8_T               src_mac[6],
    UI16_T             tag_info,
    UI16_T             type,
    UI32_T             pkt_length,
    UI32_T             ip_ext_opt_len,
    UI32_T             lport)
{
    MLDSNP_BD(TRACE, "\r\ndst=%02x:%02x:%02x:%02x:%02x:%02x\n\r"
              "src=%02x:%02x:%02x:%02x:%02x:%02x\n\r"
              "tag_info=%02x\n\r"
              "lport=%ld\n\r"
              "type=%x\n\r"
              "pkt_length=%ld:\n\r",
              dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5],
              src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],
              tag_info, lport, type, pkt_length);

    /*check the vlan exit, not implement*/
    if (FALSE == VLAN_OM_IsVlanExisted(tag_info&0x0fff))
    {
        MLDSNP_BD(RX, "vid is not exist, tag_info=%02x", tag_info);
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    MLDSNP_ENIGNE_ProcessRcvdPdu(mref_handle_p, dst_mac, src_mac, tag_info&0x0fff, lport , pkt_length, ip_ext_opt_len);

    /*free the memory*/
    L_MM_Mref_Release(&mref_handle_p);

    return;
}/*End of MLDSNP_MGR_ProcessRcvdMldPdu*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_ProcessTimerTick
*------------------------------------------------------------------------------
* Purpose: This function process the timer
* INPUT  : None
* OUTPUT : None
* RETURN : None
* NOTES  : None
*------------------------------------------------------------------------------*/
void MLDSNP_MGR_ProcessTimerTick()
{
    UI32_T passed_ticks =0, not_used_ticks=0, passed_sec =0, current_ticks=0, i;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    current_ticks = SYSFUN_GetSysTick();

    if(mldsnp_mgr_old_ticks ==0)
    {
        mldsnp_mgr_old_ticks = current_ticks;
        return;
    }

    if (current_ticks >= mldsnp_mgr_old_ticks)
        passed_ticks = current_ticks - mldsnp_mgr_old_ticks;
    else
        passed_ticks = (0xffffffffUL - mldsnp_mgr_old_ticks) + current_ticks;

    /*because l2mcast will have timer event each 10 old_ticks,
      but mldsnp only process 1 sec so divi.
    */
    passed_sec = passed_ticks / SYS_BLD_TICKS_PER_SECOND;
    not_used_ticks = passed_ticks % SYS_BLD_TICKS_PER_SECOND;

    if(current_ticks >= not_used_ticks)
        mldsnp_mgr_old_ticks = current_ticks - not_used_ticks;
    else /*(current_ticks < not_used_ticks)*/
        mldsnp_mgr_old_ticks = 0xffffffffUL-(not_used_ticks - current_ticks);

    if (MLDSNP_BACKDOOR_GetDebug(MLDSNP_BD_FLAG_TIMER))
    {
        return;
    }

    if(passed_sec >0)
    {
#if(SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE)
        MLDSNP_OM_ResetPortMldReportPerSec();
#endif
#if(SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
        MLDSNP_OM_ResetVlanMldReportPerSec();
#endif
    }

    for (i = 0; i < passed_sec; i++)
    {
        MLDSNP_TIMER_TimerTickProcess();
    }
}/*End of MLDSNP_MGR_ProcessTimerTick*/


#if (SYS_CPNT_MLDSNP_QUERY_DROP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_SetQueryDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set query quard status
 * INPUT   : lport  - which port to get information
 *           status - the enable or diable status
 * OUTPUT  : None
 * RETURN  : MVR_MGR_OK  - success
 *           MVR_MGR_CAN_NOT_SET - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetQueryDropStatus(UI32_T lport, UI32_T status)
{
    BOOL_T old_status = FALSE;

    if (status < VAL_mldSnoopQueryDrop_enable
            || status > VAL_mldSnoopQueryDrop_disable)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (FALSE == SWCTRL_LogicalPortExisting(lport))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    MLDSNP_OM_GetQueryDropStatus(lport, &old_status);

    /*current stored status*/
    if ((old_status
            && status == VAL_mldSnoopQueryDrop_enable)
            || (!old_status
                && status == VAL_mldSnoopQueryDrop_disable)
       )
    {
        return MLDSNP_TYPE_RETURN_SUCCESS;
    }

    MLDSNP_OM_SetQueryDropStatus(lport, status == VAL_mldSnoopQueryDrop_enable ? TRUE : FALSE);

    return MLDSNP_TYPE_RETURN_SUCCESS;
} /*End of MLDSNP_MGR_SetQueryDropStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_GetNextQueryDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get next port query guard status
 * INPUT   : lport  - which port to get information
 *           status - the enable or diable status
 * OUTPUT  : status  - the enabled or disable status
 * RETURN  : MVR_MGR_OK  - success
 *           MVR_MGR_CAN_NOT_GET - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_GetNextQueryDropStatus(UI32_T *lport, UI32_T  *status)
{
    BOOL_T result = FALSE;

    if (NULL == status || NULL == lport)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_GetNextLogicalPort(lport))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    MLDSNP_OM_GetQueryDropStatus(*lport, &result);

    *status = result ? VAL_mldSnoopQueryDrop_enable : VAL_mldSnoopQueryDrop_disable;

    return MLDSNP_TYPE_RETURN_SUCCESS;
} /*End of MLDSNP_MGR_GetNextQueryDropStatus*/

#endif
#if (SYS_CPNT_IPV6_MULTICAST_DATA_DROP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_SetMulticastDataDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set multicast data guard status
 * INPUT   : lport  - which port to get information
 *           status - the enable or diable status
 * OUTPUT  : None
 * RETURN  : MVR_MGR_OK  - success
 *           MVR_MGR_CAN_NOT_SET - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetMulticastDataDropStatus(UI32_T lport, UI32_T status)
{

    if (status < VAL_mldSnoopMulticastDataDrop_enable
            || status > VAL_mldSnoopMulticastDataDrop_disable)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (FALSE == SWCTRL_LogicalPortExisting(lport))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    {
        BOOL_T old_status;

        /*current stored status*/
        MLDSNP_OM_GetMulticastDataDropStatus(lport, &old_status);

        if ((old_status
                && status == VAL_mldSnoopMulticastDataDrop_enable)
                || (!old_status
                    && status == VAL_mldSnoopMulticastDataDrop_disable)
           )
        {
            return MLDSNP_TYPE_RETURN_SUCCESS;
        }
    }

    MLDSNP_OM_SetMulticastDataDropStatus(lport, status == VAL_mldSnoopMulticastDataDrop_enable ? TRUE : FALSE);

    /*set rule*/
    if (status == VAL_mldSnoopMulticastDataDrop_enable)
    {
        SWCTRL_PMGR_DropIpv6MulticastData(lport, TRUE);
    }
    else
    {
        SWCTRL_PMGR_DropIpv6MulticastData(lport, FALSE);
    }

    return MLDSNP_TYPE_RETURN_SUCCESS;
} /*End of MLDSNP_MGR_SetMulticastDataDropStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_GetNextMulticastDataDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get multicast data guard
 * INPUT   : lport  - which port to get information
 *           status - the enable or diable status
 * OUTPUT  : status  - the enabled or disable status
 * RETURN  : MVR_MGR_OK  - success
 *           MVR_MGR_CAN_NOT_GET - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_GetNextMulticastDataDropStatus(UI32_T *lport, UI32_T  *status)
{
    BOOL_T result;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (NULL == status || NULL == lport)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_GetNextLogicalPort(lport))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    MLDSNP_OM_GetMulticastDataDropStatus(*lport, &result);

    *status = result ? VAL_mldSnoopMulticastDataDrop_enable : VAL_mldSnoopMulticastDataDrop_disable;

    return MLDSNP_TYPE_RETURN_SUCCESS;
} /*End of MLDSNP_MGR_GetMulticastDataDropStatus*/
#endif

#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_RemoveMLDProfileFromPort(UI32_T lport)
{
    if (SWCTRL_LogicalPortExisting(lport) == FALSE)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (FALSE == MLDSNP_OM_RemoveMldProfileFromPort(lport))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    return MLDSNP_TYPE_RETURN_SUCCESS;
}

MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_AddMLDProfileToPort(UI32_T lport, UI32_T pid)
{
    if (SWCTRL_LogicalPortExisting(lport) == FALSE)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (FALSE == MLDSNP_OM_IsMldProfileExist(pid))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (FALSE == MLDSNP_OM_AddMldProfileToPort(lport, pid))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    return MLDSNP_TYPE_RETURN_SUCCESS;
}

MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_DeleteMLDProfileGroup(UI32_T pid, UI8_T *start_mip, UI8_T *end_mip)
{
    if (FALSE == MLDSNP_OM_IsMldProfileExist(pid))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (start_mip[0] != 0xff
            || end_mip[0] != 0xff
            || memcmp(start_mip, end_mip, SYS_ADPT_IPV6_ADDR_LEN) > 0)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (FALSE == MLDSNP_OM_DeleteMldProfileGroup(pid, start_mip, end_mip))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    return MLDSNP_TYPE_RETURN_SUCCESS;
}

MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_AddMLDProfileGroup(UI32_T pid, UI8_T *start_mip, UI8_T *end_mip)
{
    if (FALSE == MLDSNP_OM_IsMldProfileExist(pid))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (start_mip[0] != 0xff
            || end_mip[0] != 0xff
            || memcmp(start_mip, end_mip, SYS_ADPT_IPV6_ADDR_LEN) > 0)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (FALSE == MLDSNP_OM_AddMldProfileGroup(pid, start_mip, end_mip))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    return MLDSNP_TYPE_RETURN_SUCCESS;
}

MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetMLDProfileAccessMode(UI32_T pid, UI32_T access_mode)
{
    if (FALSE == MLDSNP_OM_IsMldProfileExist(pid))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (FALSE == MLDSNP_OM_SetMldProfileAccessMode(pid, access_mode))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    return MLDSNP_TYPE_RETURN_SUCCESS;
}

MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_DestroyMLDProfileEntry(UI32_T pid)
{
    if (FALSE == MLDSNP_OM_IsMldProfileExist(pid))
    {
        return MLDSNP_TYPE_RETURN_SUCCESS;
    }

    if (FALSE == MLDSNP_OM_RemoveMldProfileFromAllPort(pid))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (FALSE == MLDSNP_OM_DestroyMldProfileEntry(pid))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    return MLDSNP_TYPE_RETURN_SUCCESS;
}

MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_CreateMLDProfileEntry(UI32_T pid)
{
    if (TRUE == MLDSNP_OM_IsMldProfileExist(pid))
    {
        return MLDSNP_TYPE_RETURN_SUCCESS;
    }

    if (FALSE == MLDSNP_OM_CreateMldProfileEntry(pid))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    return MLDSNP_TYPE_RETURN_SUCCESS;
}

MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetMLDThrottlingActionToPort(UI32_T lport, UI32_T action_mode)
{
    if (SWCTRL_LogicalPortExisting(lport) == FALSE)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (FALSE == MLDSNP_OM_SetMldThrottlingActionToPort(lport, action_mode))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    return MLDSNP_TYPE_RETURN_SUCCESS;
}

MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetMldFilter(UI32_T status)
{
    if (FALSE == MLDSNP_OM_SetMldFilterStatus(status))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    /*limit each port to configured max-group*/
    if (status == VAL_mldSnoopFilterStatus_enabled)
    {
        UI32_T lport, current_count, throttling_number;
        I32_T remove_count;

        while (SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_GetNextLogicalPort(&lport))
        {
            MLDSNP_OM_GetPortDynamicGroupCount(lport, &current_count);
            MLDSNP_OM_GetPortMLDThrottlingNumber(lport, &throttling_number);
            if (current_count > throttling_number)
            {
                remove_count = current_count - throttling_number;
                MLDSNP_ENGINE_RemoveDynamicGroupbyCount(lport, remove_count > 0 ? remove_count : 0);
            }
        }
    }

    return MLDSNP_TYPE_RETURN_SUCCESS;
}

MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_SetMLDThrottlingNumberToPort(UI32_T lport, UI32_T throttling_number)
{
    UI32_T current_count, remove_count;

    if (SWCTRL_LogicalPortExisting(lport) == FALSE)
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if ((throttling_number < 0)
            || (throttling_number > SYS_ADPT_MLDSNP_MAX_NBR_OF_GROUP_ENTRY))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    MLDSNP_OM_GetPortDynamicGroupCount(lport, &current_count);

    if (current_count > throttling_number)
    {
        remove_count = current_count - throttling_number;
        MLDSNP_ENGINE_RemoveDynamicGroupbyCount(lport, remove_count);
    }


    MLDSNP_OM_SetMldThrottlingNumberToPort(lport, throttling_number);

    return MLDSNP_TYPE_RETURN_SUCCESS;
}


MLDSNP_TYPE_ReturnValue_T MLDSNP_MGR_GetNextPortMLDProfileID(UI32_T *lport, UI32_T *pid_p)
{
    if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_GetNextLogicalPort(lport))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (FALSE == MLDSNP_OM_GetPortMldProfileID(*lport, pid_p))
    {
        return MLDSNP_TYPE_RETURN_FAIL;
    }
    return MLDSNP_TYPE_RETURN_SUCCESS;
}

#endif

#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE || SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_SetMldReportLimitPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set mld report limit value per second
 * INPUT   : ifindex       - which port or vlan to get
 *           limit_per_sec - per second limit value
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_MGR_SetMldReportLimitPerSec(UI32_T ifindex, UI16_T limit_per_sec)
{
#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
    if (ifindex >= SYS_ADPT_VLAN_1_IF_INDEX_NUMBER)
    {
        UI32_T vid;

        if (limit_per_sec < SYS_ADPT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_MIN
                || limit_per_sec > SYS_ADPT_MDLSNP_MLD_REPORT_LIMIT_PER_SECOND_MAX)
        {
            return FALSE;
        }

        VLAN_IFINDEX_CONVERTTO_VID(ifindex, vid);
        if (FALSE == VLAN_OM_IsVlanExisted(vid)
                || VAL_dot1qVlanStaticRowStatus_active != VLAN_OM_GetDot1qVlanRowStatus(vid))
        {
            return FALSE;
        }
        MLDSNP_OM_SetVlanMldReportLimitPerSec(vid, limit_per_sec);
    }
#endif
#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE)
    if (ifindex <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
    {
        if (limit_per_sec < SYS_ADPT_IGMP_SNOOP_IGMP_REPORT_LIMIT_PER_SECOND_MIN
                || limit_per_sec > SYS_ADPT_IGMP_SNOOP_IGMP_REPORT_LIMIT_PER_SECOND_MAX)
        {
            return FALSE;
        }

        if (FALSE == SWCTRL_LogicalPortExisting(ifindex))
        {
            return FALSE;
        }

        MLDSNP_OM_SetPortMldReportLimitPerSec(ifindex, limit_per_sec);
    }
#endif

    return TRUE;
} /*End of MLDSNP_MGR_SetQueryDropStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_SetMldReportLimitPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set mld report limit value per second
 * INPUT   : ifindex       - which port or vlan to get
 * OUTPUT  : limit_per_sec - per second limit value
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_MGR_GetMldReportLimitPerSec(UI32_T ifindex, UI32_T  *limit_per_sec)
{
    if (NULL == limit_per_sec)
    {
        return FALSE;
    }

#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
    if (ifindex >= SYS_ADPT_VLAN_1_IF_INDEX_NUMBER)
    {
        UI32_T vid;

        VLAN_IFINDEX_CONVERTTO_VID(ifindex, vid);
        if (FALSE == VLAN_OM_IsVlanExisted(vid)
                || VAL_dot1qVlanStaticRowStatus_active != VLAN_OM_GetDot1qVlanRowStatus(vid))
        {
            return FALSE;
        }

        *limit_per_sec = MLDSNP_OM_GetVlanMldReportLimitPerSec(vid);
    }
#endif
#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE)
    if (ifindex <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
    {
        if (FALSE == SWCTRL_LogicalPortExisting(ifindex))
        {
            return FALSE;
        }

        *limit_per_sec = MLDSNP_OM_GetPortMldReportLimitPerSec(ifindex);
    }
#endif
    return TRUE;
} /*End of MLDSNP_MGR_GetQueryDropStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_GetNextMldReportLimitPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set mld report limit value per second
 * INPUT   : ifindex       - which port or vlan to get
 *           limit_per_sec - per second limit value
 * OUTPUT  : ifindex - next port or vlan
 *           limit_per_sec - per second limit value
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_MGR_GetNextMldReportLimitPerSec(UI32_T *ifindex, UI32_T  *limit_per_sec)
{
    if (NULL == limit_per_sec || NULL == ifindex)
    {
        return FALSE;
    }

#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
    if (*ifindex >= (SYS_ADPT_VLAN_1_IF_INDEX_NUMBER - 1)) /*for get next vlan ifindex (SYS_ADPT_VLAN_1_IF_INDEX_NUMBER-1) means vid 0*/
    {
        UI32_T vid;

        VLAN_IFINDEX_CONVERTTO_VID(*ifindex, vid);

        if (FALSE == VLAN_OM_GetNextVlanId(0, &vid))
        {
            return FALSE;
        }

        *limit_per_sec = MLDSNP_OM_GetVlanMldReportLimitPerSec(vid);
    }
#endif
#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE)
    if (*ifindex <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
    {
        if (FALSE == SWCTRL_GetNextLogicalPort(ifindex))
        {
            return FALSE;
        }

        *limit_per_sec = MLDSNP_OM_GetPortMldReportLimitPerSec(*ifindex);
    }
#endif
    return TRUE;
} /*End of MLDSNP_MGR_GetNextQueryDropStatus*/

#endif


BOOL_T MLDSNP_MGR_ClearAllDynamicGroup()
{
    UI32_T vid = 0;

    while (TRUE == VLAN_OM_GetNextVlanId(0, &vid))
    {
        if (FALSE == MLDSNP_ENGINE_DeleteAllDynamicGroupInVlan(vid))
        {
            return FALSE;
        }
    }
    return TRUE;
}

BOOL_T MLDSNP_MGR_ClearInterfaceStatistics(UI32_T ifindex)
{
    if (ifindex >= SYS_ADPT_VLAN_1_IF_INDEX_NUMBER) /*vid*/
    {
        UI32_T vid = 0;
        VLAN_IFINDEX_CONVERTTO_VID(ifindex,vid);
        MLDSNP_OM_ClearInterfaceStatistics(vid, TRUE);
        return TRUE;
    }
    else if (ifindex != 0) /*port*/
    {
        MLDSNP_OM_ClearInterfaceStatistics(ifindex, FALSE);
        return TRUE;
    }
    else if (ifindex == 0)/*all*/
    {
        UI32_T vid = 0, lport = 0;
        while (VLAN_OM_GetNextVlanId(0, &vid))
        {
            MLDSNP_OM_ClearInterfaceStatistics(vid, TRUE);
        }

        while (SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_GetNextLogicalPort(&lport))
        {
            MLDSNP_OM_ClearInterfaceStatistics(lport, FALSE);
        }
        return TRUE;
    }

    return FALSE;
}

BOOL_T MLDSNP_MGR_GetInterfaceStatistics(UI32_T ifindex, BOOL_T is_vlan, MLDSNP_MGR_InfStat_T *result_p)
{
    MLDSNP_OM_InfStat_T statistics;

    memset(result_p, 0, sizeof(MLDSNP_MGR_InfStat_T));
    memset(&statistics, 0, sizeof(MLDSNP_OM_InfStat_T));

    MLDSNP_OM_GetInterfaceStatistics(ifindex, is_vlan, &statistics);

    if (is_vlan)
    {
        if(ifindex < 1 || ifindex > SYS_ADPT_MAX_VLAN_ID)
          return FALSE;

        result_p->active_group_cnt = statistics.active_group_cnt;
        result_p->query_uptime    = statistics.query_uptime;        
        result_p->query_exptime    = statistics.query_exptime;
        result_p->other_query_uptime= statistics.other_query_uptime;
        result_p->other_query_expire= statistics.other_query_exptime;
        result_p->unsolict_expire   = statistics.unsolicit_exptime;
        MLDSNP_ENGINE_GetReportSrcIpv6Address(ifindex, result_p->host_ip_addr);
        IPV6_ADDR_COPY(result_p->other_querier_ip_addr, statistics.querier_ip_addr);
        MLDSNP_ENGINE_GetConfiguredIPv6Address(ifindex, result_p->self_querier_ip_addr);
    }
    else
    {
      if(SWCTRL_LogicalPortExisting(ifindex) == FALSE)
        return FALSE;
    }

    result_p->counter.num_invalid_mld_recv = statistics.counter.num_invalid_mld_recv;
    result_p->counter.num_grecs            = statistics.counter.num_grecs;
    result_p->counter.num_gq_recv          = statistics.counter.num_gq_recv;
    result_p->counter.num_gq_send          = statistics.counter.num_gq_send;
    result_p->counter.num_joins            = statistics.counter.num_joins;
    result_p->counter.num_joins_send       = statistics.counter.num_joins_send;
    result_p->counter.num_joins_succ       = statistics.counter.num_joins_succ;
    result_p->counter.num_leaves           = statistics.counter.num_leaves;
    result_p->counter.num_leaves_send      = statistics.counter.num_leaves_send;
    result_p->counter.num_sq_recv          = statistics.counter.num_sq_recv;
    result_p->counter.num_sq_send          = statistics.counter.num_sq_send;
    #if(SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    result_p->counter.num_drop_by_filter      = statistics.counter.num_drop_by_filter;
    #endif
    result_p->counter.num_drop_by_mroute_port = statistics.counter.num_drop_by_mroute_port;
    #if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE || SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
    result_p->counter.num_drop_by_rate_limit  = statistics.counter.num_drop_by_rate_limit;
    #endif
    return TRUE;
}


/*------------------------------------------------------------------------------
 * ROUTINE NAME : MLDSNP_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for csca om.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  --  There is a response need to send.
 *    FALSE --  No response need to send.
 *
 * NOTES:
 *    1.The size of ipcmsg_p.msg_buf must be large enough to carry any response
 *      messages.
 *------------------------------------------------------------------------------
 */
BOOL_T MLDSNP_MGR_HandleIPCReqMsg(
    SYSFUN_Msg_T* ipcmsg_p)
{
    MLDSNP_MGR_IPCMsg_T *msg_data_p;
    UI32_T              cmd;

    if (ipcmsg_p == NULL)
    {
        MLDSNP_BD(IPC, "ipcmsg_p ==NULL ");
        return TRUE;
    }

    msg_data_p = (MLDSNP_MGR_IPCMsg_T*)ipcmsg_p->msg_buf;
    cmd        = msg_data_p->type.cmd;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        MLDSNP_BD(IPC, "In transition mode");
        msg_data_p->type.result_ui32 = MLDSNP_TYPE_RETURN_FAIL;
        goto EXIT;
    }

    MLDSNP_BD(IPC, "ipcmsg_p->cmd=%ld", cmd);

    MLDSNP_MGR_LOCK();

    switch (cmd)
    {
        case MLDSNP_MGR_IPCCMD_ADDPORTSTATICJOINGROUP:
            ipcmsg_p->msg_size           = MLDSNP_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_MGR_AddPortStaticJoinGroup(
                                               msg_data_p->data.port_join_group.vid,
                                               msg_data_p->data.port_join_group.gip_ap,
                                               msg_data_p->data.port_join_group.sip_ap,
                                               msg_data_p->data.port_join_group.lport,
                                               msg_data_p->data.port_join_group.mode);
            break;

        case MLDSNP_MGR_IPCCMD_ADDSTATICROUTERPORT:
            ipcmsg_p->msg_size           = MLDSNP_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_MGR_AddStaticRouterPort(
                                               msg_data_p->data.u32a1_u32a2.u32_a1,
                                               msg_data_p->data.u32a1_u32a2.u32_a2);
            break;

        case MLDSNP_MGR_IPCCMD_DELETEPORTSTATICJOINGROUP:
            ipcmsg_p->msg_size           = MLDSNP_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_MGR_DeletePortStaticJoinGroup(
                                               msg_data_p->data.port_join_group.vid,
                                               msg_data_p->data.port_join_group.gip_ap,
                                               msg_data_p->data.port_join_group.sip_ap,
                                               msg_data_p->data.port_join_group.lport);
            break;

        case MLDSNP_MGR_IPCCMD_DELETESTATICROUTERPORT:
            ipcmsg_p->msg_size           = MLDSNP_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_MGR_DeleteStaticRouterPort(
                                               msg_data_p->data.u32a1_u32a2.u32_a1,
                                               msg_data_p->data.u32a1_u32a2.u32_a2);
            break;

        case MLDSNP_MGR_IPCCMD_SETIMMEDIATELEAVESTATUS:
            ipcmsg_p->msg_size           = MLDSNP_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_MGR_SetImmediateLeaveStatus(
                                               msg_data_p->data.u32a1_u32a2.u32_a1,
                                               msg_data_p->data.u32a1_u32a2.u32_a2);
            break;

        case MLDSNP_MGR_IPCCMD_SETIMMEDIATELEAVEBYHOSTSTATUS:
            ipcmsg_p->msg_size           = MLDSNP_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_MGR_SetImmediateLeaveByHostStatus(
                                               msg_data_p->data.u32a1_u32a2.u32_a1,
                                               msg_data_p->data.u32a1_u32a2.u32_a2);
            break;

        case MLDSNP_MGR_IPCCMD_SETLASTLISTENERQUERYINTERVAL:
            ipcmsg_p->msg_size           = MLDSNP_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_MGR_SetLastListenerQueryInterval(
                                               msg_data_p->data.ui32_v);
            break;

        case MLDSNP_MGR_IPCCMD_SETMLDSNPVER:
            ipcmsg_p->msg_size           = MLDSNP_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_MGR_SetMldSnpVer(
                                               msg_data_p->data.ui32_v);
            break;

        case MLDSNP_MGR_IPCCMD_SETMLDSTATUS:
            ipcmsg_p->msg_size           = MLDSNP_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_MGR_SetMldSnpStatus(
                                               msg_data_p->data.ui32_v);
            break;

        case MLDSNP_MGR_IPCCMD_SETQUERIERSTATUS:
            ipcmsg_p->msg_size           = MLDSNP_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_MGR_SetQuerierStatus(
                                               msg_data_p->data.ui32_v);
            break;

        case MLDSNP_MGR_IPCCMD_SETQUERYINTERVAL:
            ipcmsg_p->msg_size           = MLDSNP_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_MGR_SetQueryInterval(
                                               msg_data_p->data.ui32_v);
            break;

        case MLDSNP_MGR_IPCCMD_SETQUERYRESPONSEINTERVAL:
            ipcmsg_p->msg_size           = MLDSNP_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_MGR_SetQueryResponseInterval(
                                               msg_data_p->data.ui32_v);
            break;

        case MLDSNP_MGR_IPCCMD_SETROBUSTNESSVALUE:
            ipcmsg_p->msg_size = MLDSNP_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_MGR_SetRobustnessValue(
                                               msg_data_p->data.ui32_v);
            break;

        case MLDSNP_MGR_IPCCMD_SETROUTEREXPIRETIME:
            ipcmsg_p->msg_size           = MLDSNP_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_MGR_SetRouterExpireTime(
                                               msg_data_p->data.ui32_v);
            break;

        case MLDSNP_MGR_IPCCMD_SETUNKNOWNFLOODBEHAVIOR:
            ipcmsg_p->msg_size           = MLDSNP_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_MGR_SetUnknownFloodBehavior(
                                               msg_data_p->data.u32a1_u32a2.u32_a1,
                                               msg_data_p->data.u32a1_u32a2.u32_a2);
            break;

#if (SYS_CPNT_MLDSNP_PROXY == TRUE)
        case MLDSNP_MGR_IPCCMD_SET_PROXY_REPORTING:
        {
            MLDSNP_MGR_IPCMsg_GS1_T *data_p = MLDSNP_MGR_MSG_DATA(ipcmsg_p);
            MLDSNP_MGR_MSG_RETVAL(ipcmsg_p) = MLDSNP_MGR_SetProxyReporting(data_p->value1);
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case MLDSNP_MGR_IPCCMD_SET_UNSOLICITEDREPORTINTERVAL:
            ipcmsg_p->msg_size           = MLDSNP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            msg_data_p->type.result_ui32 = MLDSNP_MGR_SetUnsolicitedReportInterval(
                                               msg_data_p->data.ui32_v);
            break;
#endif
        case MLDSNP_MGR_IPCCMD_SETSTATICROUTERPORTLIST:
            ipcmsg_p->msg_size           = MLDSNP_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_MGR_SetStaticRouterPortlist(
                                               msg_data_p->data.router_portlist.vid,
                                               msg_data_p->data.router_portlist.port_list);
            break;
        case MLDSNP_MGR_IPCCMD_DELETESTATICROUTERPORTLIST:
            ipcmsg_p->msg_size           = MLDSNP_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_MGR_DeleteStaticRouterPortlist(
                                               msg_data_p->data.router_portlist.vid,
                                               msg_data_p->data.router_portlist.port_list);
            break;

        case MLDSNP_MGR_IPCCMD_SETPORTLISTSTATICJOINGROUP:
            ipcmsg_p->msg_size           = MLDSNP_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_MGR_SetPortListStaticJoinGroup(
                                               msg_data_p->data.static_portlist.vid,
                                               msg_data_p->data.static_portlist.gip_ap,
                                               msg_data_p->data.static_portlist.sip_ap,
                                               msg_data_p->data.static_portlist.port_list);
            break;

        case MLDSNP_MGR_IPCCMD_SETPORTLISTSTATICLEAVEGROUP:
            ipcmsg_p->msg_size           = MLDSNP_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = MLDSNP_MGR_SetPortListStaticLeaveGroup(
                                               msg_data_p->data.static_portlist.vid,
                                               msg_data_p->data.static_portlist.gip_ap,
                                               msg_data_p->data.static_portlist.sip_ap,
                                               msg_data_p->data.static_portlist.port_list);
            break;

        case MLDSNP_MGR_IPCCMD_SETMROUTESTATUS:
            ipcmsg_p->msg_size           = MLDSNP_MGR_MSGBUF_TYPE_SIZE;
            MLDSNP_QUERIER_SetMRouteStatus(msg_data_p->data.bool_v);
            break;

#if (SYS_CPNT_IPV6_MULTICAST_DATA_DROP == TRUE)
        case MLDSNP_MGR_IPCCMD_GETNEXT_MULTICAST_DATA_DROP_STATUS:
        {
            MLDSNP_MGR_IPCMsg_GS2_T *data_p = MLDSNP_MGR_MSG_DATA(ipcmsg_p);
            MLDSNP_MGR_MSG_RETVAL(ipcmsg_p) = MLDSNP_MGR_GetNextMulticastDataDropStatus(&data_p->value1, &data_p->value2);
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS2_T);
        }
        break;

        case MLDSNP_MGR_IPCCMD_SET_MULTICAST_DATA_DROP_STATUS:
        {
            MLDSNP_MGR_IPCMsg_GS2_T *data_p = MLDSNP_MGR_MSG_DATA(ipcmsg_p);
            MLDSNP_MGR_MSG_RETVAL(ipcmsg_p) = MLDSNP_MGR_SetMulticastDataDropStatus(data_p->value1, data_p->value2);
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        }
        break;
#endif
#if (SYS_CPNT_MLDSNP_QUERY_DROP == TRUE)
        case MLDSNP_MGR_IPCCMD_GETNEXT_QUERY_GUARD_STATUS:
        {
            MLDSNP_MGR_IPCMsg_GS2_T *data_p = MLDSNP_MGR_MSG_DATA(ipcmsg_p);
            MLDSNP_MGR_MSG_RETVAL(ipcmsg_p) = MLDSNP_MGR_GetNextQueryDropStatus(&data_p->value1, &data_p->value2);
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS2_T);
        }
        break;

        case MLDSNP_MGR_IPCCMD_SET_QUERY_GUARD_STATUS:
        {
            MLDSNP_MGR_IPCMsg_GS2_T *data_p = MLDSNP_MGR_MSG_DATA(ipcmsg_p);
            MLDSNP_MGR_MSG_RETVAL(ipcmsg_p) = MLDSNP_MGR_SetQueryDropStatus(data_p->value1, data_p->value2);
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        }
        break;
#endif

#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
        case MLDSNP_MGR_IPCCMD_SET_MLD_MAXGROUP_ACTION_PORT:
        {
            MLDSNP_MGR_IPCMsg_MldSnoopMaxGroupAction_T *data_p = MLDSNP_MGR_MSG_DATA(ipcmsg_p);
            MLDSNP_MGR_MSG_RETVAL(ipcmsg_p) = MLDSNP_MGR_SetMLDThrottlingActionToPort(data_p->port, data_p->action);
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopMaxGroupAction_T);
            break;
        }

        case MLDSNP_MGR_IPCCMD_SET_MLD_PRIFILE_PORT:
        {
            MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T *data_p = MLDSNP_MGR_MSG_DATA(ipcmsg_p);
            MLDSNP_MGR_MSG_RETVAL(ipcmsg_p) = MLDSNP_MGR_SetMLDThrottlingNumberToPort(data_p->port, data_p->mldsnp_Profile_id);
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T);
            break;
        }

        case MLDSNP_MGR_IPCCMD_REMOVE_MLD_PRIFILE_PORT:
        {
            MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T *data_p = MLDSNP_MGR_MSG_DATA(ipcmsg_p);
            MLDSNP_MGR_MSG_RETVAL(ipcmsg_p) = MLDSNP_MGR_RemoveMLDProfileFromPort(data_p->port);
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T);
            break;
        }

        case MLDSNP_MGR_IPCCMD_ADD_MLD_PRIFILE_PORT:
        {
            MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T *data_p = MLDSNP_MGR_MSG_DATA(ipcmsg_p);
            MLDSNP_MGR_MSG_RETVAL(ipcmsg_p) = MLDSNP_MGR_AddMLDProfileToPort(data_p->port, data_p->mldsnp_Profile_id);
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T);
            break;
        }

        case MLDSNP_MGR_IPCCMD_DELETE_MLD_PRIFILE_GROUP:
        {
            MLDSNP_MGR_IPCMsg_MldSnoopProfileRange_T *data_p = MLDSNP_MGR_MSG_DATA(ipcmsg_p);
            MLDSNP_MGR_MSG_RETVAL(ipcmsg_p) = MLDSNP_MGR_DeleteMLDProfileGroup(data_p->mldsnp_Profile_id, data_p->ip_begin, data_p->ip_end);
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfileRange_T);
            break;
        }

        case MLDSNP_MGR_IPCCMD_ADD_MLD_PRIFILE_GROUP:
        {
            MLDSNP_MGR_IPCMsg_MldSnoopProfileRange_T *data_p = MLDSNP_MGR_MSG_DATA(ipcmsg_p);
            MLDSNP_MGR_MSG_RETVAL(ipcmsg_p) = MLDSNP_MGR_AddMLDProfileGroup(data_p->mldsnp_Profile_id, data_p->ip_begin, data_p->ip_end);
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfileRange_T);
            break;
        }

        case MLDSNP_MGR_IPCCMD_SET_MLD_PRIFILE_MODE:
        {
            MLDSNP_MGR_IPCMsg_MldSnoopProfileMode_T *data_p = MLDSNP_MGR_MSG_DATA(ipcmsg_p);
            MLDSNP_MGR_MSG_RETVAL(ipcmsg_p) = MLDSNP_MGR_SetMLDProfileAccessMode(data_p->mldsnp_Profile_id, data_p->profile_mode);
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfileMode_T);
            break;
        }

        case MLDSNP_MGR_IPCCMD_SET_MLD_DESTROY_PRIFILE:
        {
            MLDSNP_MGR_IPCMsg_MldSnoopProfile_T *data_p = MLDSNP_MGR_MSG_DATA(ipcmsg_p);
            MLDSNP_MGR_MSG_RETVAL(ipcmsg_p) = MLDSNP_MGR_DestroyMLDProfileEntry(data_p->mldsnp_Profile_id);
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfile_T);
            break;
        }

        case MLDSNP_MGR_IPCCMD_SET_MLD_CREATE_PRIFILE:
        {
            MLDSNP_MGR_IPCMsg_MldSnoopProfile_T *data_p = MLDSNP_MGR_MSG_DATA(ipcmsg_p);
            MLDSNP_MGR_MSG_RETVAL(ipcmsg_p) = MLDSNP_MGR_CreateMLDProfileEntry(data_p->mldsnp_Profile_id);
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfile_T);
            break;
        }

        case MLDSNP_MGR_IPCCMD_SET_MLD_FILTER_STATUS:
        {
            MLDSNP_MGR_IPCMsg_MldSnoopFilter_T *data_p = MLDSNP_MGR_MSG_DATA(ipcmsg_p);
            MLDSNP_MGR_MSG_RETVAL(ipcmsg_p) = MLDSNP_MGR_SetMldFilter(data_p->mldsnp_FilterStatus);
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopFilter_T);
            break;
        }
        case MLDSNP_MGR_IPCCMD_GETNEXT_MLD_PRIFILE_PORT:
        {
            MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T *data_p = MLDSNP_MGR_MSG_DATA(ipcmsg_p);
            MLDSNP_MGR_MSG_RETVAL(ipcmsg_p) = MLDSNP_MGR_GetNextPortMLDProfileID(&data_p->port, &data_p->mldsnp_Profile_id);
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T);
            break;
        }
#endif
#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE || SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
        case MLDSNP_MGR_IPCCMD_SET_MLD_REPORT_LIMIT_PER_SECOND:
        {
            MLDSNP_MGR_IPCMsg_GS2_T *data_p = MLDSNP_MGR_MSG_DATA(ipcmsg_p);
            MLDSNP_MGR_MSG_RETVAL(ipcmsg_p) = MLDSNP_MGR_SetMldReportLimitPerSec(data_p->value1, data_p->value2);
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        }
        break;

        case MLDSNP_MGR_IPCCMD_GET_NEXT_MLD_REPORT_LIMIT_PER_SECOND:
        {
            MLDSNP_MGR_IPCMsg_GS2_T *data_p = MLDSNP_MGR_MSG_DATA(ipcmsg_p);
            MLDSNP_MGR_MSG_RETVAL(ipcmsg_p) = MLDSNP_MGR_GetNextMldReportLimitPerSec(&data_p->value1, &data_p->value2);
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS2_T);
        }
        break;
#endif
        case MLDSNP_MGR_IPCCMD_CLEAR_MLD_SNOOP_DYNAMIC_GROUP:
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            MLDSNP_MGR_MSG_RETVAL(ipcmsg_p) = MLDSNP_MGR_ClearAllDynamicGroup();
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;

        case MLDSNP_MGR_IPCCMD_CLEAR_IP_MLDSNOOPING_STATISTICS:
        {
            MLDSNP_MGR_IPCMsg_GS1_T *data_p = MLDSNP_MGR_MSG_DATA(ipcmsg_p);
            MLDSNP_MGR_MSG_RETVAL(ipcmsg_p) = MLDSNP_MGR_ClearInterfaceStatistics(data_p->value1);
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        }
        break;
        case MLDSNP_MGR_IPCCMD_GET_INF_STATISTICS:
        {
            MLDSNP_MGR_IPCMsg_InfStatistics *data_p = MLDSNP_MGR_MSG_DATA(ipcmsg_p);
            MLDSNP_MGR_MSG_RETVAL(ipcmsg_p) = MLDSNP_MGR_GetInterfaceStatistics(data_p->inf_id, data_p->is_vlan, &data_p->statistics);
            ipcmsg_p->msg_size = MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_InfStatistics);
        }
        break;
        default:
            MLDSNP_BD(IPC, "wrong command=%ld",  cmd);
            ipcmsg_p->msg_size           = MLDSNP_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32 = 0;
            goto REP_EXIT;
    }

    MLDSNP_MGR_UNLOCK();

EXIT:
    MLDSNP_BD(IPC, "retrun vlaue=%ld, cmd=%ld ",
              msg_data_p->type.result_ui32, cmd);
    /*Check sychronism or asychronism ipc. If it is sychronism(need to respond)then we return true.
     */
    if (cmd < MLDSNP_MGR_IPCCMD_FOLLOWISASYNCHRONISMIPC)
    {
        return TRUE;
    }

    return FALSE;

REP_EXIT:
    return TRUE;
}


/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_LportNotOperUpCallBack
*------------------------------------------------------------------------------
* Purpose: This function process the port not oeprate up
* INPUT  : lport  - the logical port
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_MGR_LportNotOperUpCallBack(
    UI32_T  lport)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    MLDSNP_BD(CALLBACK, "lport=%ld", lport);

    if (FALSE == MLDSNP_ENGINE_PortLeaveAllGroup(lport))
    {
        MLDSNP_BD(CALLBACK, "lport=%ld, leave all group fail", lport);
        return FALSE;
    }


    if (FALSE == MLDSNP_QUERIER_RemoveDynamicRouterPortOnSpecifyPort(0, lport))
    {
        MLDSNP_BD(CALLBACK, "Delete dynamic router port %ld", lport);
        return FALSE;
    }

    return TRUE;
}/*End of MLDSNP_MGR_LportNotOperUpCallBack*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_TrunkMemberAdd1sCallback
*------------------------------------------------------------------------------
* Purpose: This function process the a port become trunk first member
* INPUT  : trunk_ifindex  - the trunk ifindex
*          member_ifindex - the member ifindex
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_MGR_TrunkMemberAdd1sCallback(
    UI32_T  trunk_ifindex ,
    UI32_T  member_ifindex)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    MLDSNP_BD(CALLBACK, "trunk_ifindex=%ld, member_ifindex=%ld", trunk_ifindex, member_ifindex);

    /* before delete port, inherit the static group to trunk port */
    MLDSNP_ENGINE_CarryConfigFromOnePortToAnother(member_ifindex, trunk_ifindex);

    MLDSNP_ENGINE_DeletePort(member_ifindex);

#if 0
    if (FALSE == MLDSNP_ENGINE_PortLeaveAllGroup(member_ifindex))
    {
        MLDSNP_BD(CALLBACK, "member_ifindex=%ld, leave all group fail", member_ifindex);

        return FALSE;
    }
#endif
    MLDSNP_OM_ClearInterfaceStatistics(member_ifindex, FALSE);

#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    {
        UI32_T pid;
        UI32_T max_group_number, action_mode;

        MLDSNP_OM_GetPortMldProfileID(member_ifindex, &pid);
        MLDSNP_OM_AddMldProfileToPort(trunk_ifindex, pid);
        MLDSNP_OM_RemoveMldProfileFromPort(member_ifindex);
        MLDSNP_OM_GetPortMLDThrottlingNumber(member_ifindex, &max_group_number);
        MLDSNP_OM_GetMLDThrottlingAction(member_ifindex, &action_mode);
        MLDSNP_OM_SetMldThrottlingNumberToPort(member_ifindex, SYS_ADPT_MLDSNP_MAX_NBR_OF_GROUP_ENTRY);
        MLDSNP_OM_SetMldThrottlingActionToPort(member_ifindex, SYS_DFLT_MLD_THROTTLE_ACTION);
        MLDSNP_OM_SetMldThrottlingNumberToPort(trunk_ifindex, max_group_number);
        MLDSNP_OM_SetMldThrottlingActionToPort(trunk_ifindex, action_mode);
    }
#endif

#if(SYS_CPNT_MLDSNP_QUERY_DROP== TRUE)
    {
        BOOL_T status = FALSE;
        MLDSNP_OM_GetQueryDropStatus(member_ifindex, &status);
        MLDSNP_OM_SetQueryDropStatus(trunk_ifindex, status);
    }
#endif
#if(SYS_CPNT_IPV6_MULTICAST_DATA_DROP == TRUE)
    {
        BOOL_T member_status = FALSE, trunk_status = FALSE;
        MLDSNP_OM_GetMulticastDataDropStatus(member_ifindex, &member_status);
        MLDSNP_OM_GetMulticastDataDropStatus(trunk_ifindex, &trunk_status);

        if (member_status != trunk_status)
        {
            MLDSNP_OM_SetMulticastDataDropStatus(trunk_ifindex, member_status);
            SWCTRL_PMGR_DropIpv6MulticastData(trunk_ifindex, member_status);
        }
    }
#endif
    return TRUE;
}/*End of MLDSNP_MGR_TrunkMemberAdd1sCallback*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_TrunkMemberAddCallback
*------------------------------------------------------------------------------
* Purpose: This function process the port join the trunk port
* INPUT  : trunk_ifindex  - the trunk ifindex
*          member_ifindex - the member ifindex
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_MGR_TrunkMemberAddCallback(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    MLDSNP_BD(CALLBACK, "trunk_ifindex=%ld, member_ifindex=%ld", trunk_ifindex, member_ifindex);

    MLDSNP_ENGINE_DeletePort(member_ifindex);
#if 0
    if (FALSE == MLDSNP_ENGINE_PortLeaveAllGroup(member_ifindex))
    {
        MLDSNP_BD(CALLBACK, "member_ifindex=%ld, leave all group fail", member_ifindex);

        return FALSE;
    }
#endif
    MLDSNP_OM_ClearInterfaceStatistics(member_ifindex, FALSE);

#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    MLDSNP_OM_RemoveMldProfileFromPort(member_ifindex);
    MLDSNP_OM_SetMldThrottlingNumberToPort(member_ifindex, SYS_ADPT_MLDSNP_MAX_NBR_OF_GROUP_ENTRY);
    MLDSNP_OM_SetMldThrottlingActionToPort(member_ifindex, SYS_DFLT_MLD_THROTTLE_ACTION);
#endif

#if(SYS_CPNT_MLDSNP_QUERY_DROP== TRUE)
    {
        BOOL_T status = FALSE;
        MLDSNP_OM_GetQueryDropStatus(trunk_ifindex, &status);
        MLDSNP_OM_SetQueryDropStatus(member_ifindex, status);
    }
#endif
#if(SYS_CPNT_IPV6_MULTICAST_DATA_DROP == TRUE)
    {
        BOOL_T member_status = FALSE, trunk_status = FALSE;
        MLDSNP_OM_GetMulticastDataDropStatus(trunk_ifindex, &trunk_status);
        MLDSNP_OM_GetMulticastDataDropStatus(member_ifindex, &member_status);

        if (trunk_status != member_status)
        {
            SWCTRL_PMGR_DropIpv6MulticastData(trunk_ifindex, trunk_status);
        }
    }
#endif
    return TRUE;
}/*End of MLDSNP_MGR_TrunkMemberAddCallback*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_TrunkMemberDeleteCallback
*------------------------------------------------------------------------------
* Purpose: This function process the port leave the trunk port
* INPUT  : trunk_ifindex  - the trunk ifindex
*          member_ifindex - the member ifindex
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_MGR_TrunkMemberDeleteCallback(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    MLDSNP_BD(CALLBACK, "trunk_ifindex=%ld, member_ifindex=%ld", trunk_ifindex, member_ifindex);

    MLDSNP_ENGINE_InheritConfigFromTrunkToPort(member_ifindex, trunk_ifindex);
    
#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    {
        UI32_T pid;
        UI32_T max_group_number, action_mode;

        MLDSNP_OM_GetPortMldProfileID(trunk_ifindex, &pid);
        MLDSNP_OM_AddMldProfileToPort(member_ifindex, pid);
        MLDSNP_OM_GetPortMLDThrottlingNumber(trunk_ifindex, &max_group_number);
        MLDSNP_OM_GetMLDThrottlingAction(trunk_ifindex, &action_mode);
        MLDSNP_OM_SetMldThrottlingNumberToPort(member_ifindex, max_group_number);
        MLDSNP_OM_SetMldThrottlingActionToPort(member_ifindex, action_mode);
    }
#endif

#if(SYS_CPNT_MLDSNP_QUERY_DROP== TRUE)
    {
        BOOL_T status = FALSE;
        MLDSNP_OM_GetQueryDropStatus(trunk_ifindex, &status);
        MLDSNP_OM_SetQueryDropStatus(member_ifindex, status);
    }
#endif

#if(SYS_CPNT_IPV6_MULTICAST_DATA_DROP == TRUE)
    {
        BOOL_T member_status = FALSE, trunk_status = FALSE;
        MLDSNP_OM_GetMulticastDataDropStatus(trunk_ifindex, &trunk_status);
        MLDSNP_OM_GetMulticastDataDropStatus(member_ifindex, &member_status);
        
        if (trunk_status != member_status)
        {
            MLDSNP_OM_SetMulticastDataDropStatus(member_ifindex, trunk_status);
            SWCTRL_PMGR_DropIpv6MulticastData(member_ifindex, trunk_status);
        }
    }
#endif
    return TRUE;
}/*End of MLDSNP_MGR_TrunkMemberDeleteCallback*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_TrunkMemberLstDeleteCallback
*------------------------------------------------------------------------------
* Purpose: This function process the port last leave the trunk port
* INPUT  : trunk_ifindex  - the trunk ifindex
*          member_ifindex - the member ifindex
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_MGR_TrunkMemberLstDeleteCallback(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    MLDSNP_BD(CALLBACK, "trunk_ifindex=%ld, member_ifindex=%ld", trunk_ifindex, member_ifindex);

    MLDSNP_ENGINE_CarryConfigFromOnePortToAnother(trunk_ifindex, member_ifindex);

    MLDSNP_ENGINE_DeletePort(trunk_ifindex);
    MLDSNP_OM_ClearInterfaceStatistics(trunk_ifindex, FALSE);

#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    {
        UI32_T pid;
        UI32_T max_group_number, action_mode;

        MLDSNP_OM_GetPortMldProfileID(trunk_ifindex, &pid);
        MLDSNP_OM_AddMldProfileToPort(member_ifindex, pid);
        MLDSNP_OM_RemoveMldProfileFromPort(trunk_ifindex);
        MLDSNP_OM_GetPortMLDThrottlingNumber(trunk_ifindex, &max_group_number);
        MLDSNP_OM_GetMLDThrottlingAction(trunk_ifindex, &action_mode);
        MLDSNP_OM_SetMldThrottlingNumberToPort(member_ifindex, max_group_number);
        MLDSNP_OM_SetMldThrottlingActionToPort(member_ifindex, action_mode);
        MLDSNP_OM_SetMldThrottlingNumberToPort(trunk_ifindex, SYS_ADPT_MLDSNP_MAX_NBR_OF_GROUP_ENTRY);
        MLDSNP_OM_SetMldThrottlingActionToPort(trunk_ifindex, SYS_DFLT_MLD_THROTTLE_ACTION);
    }
#endif
#if(SYS_CPNT_MLDSNP_QUERY_DROP== TRUE)
    {
        BOOL_T status = FALSE;
        MLDSNP_OM_GetQueryDropStatus(trunk_ifindex, &status);
        MLDSNP_OM_SetQueryDropStatus(member_ifindex, status);

        MLDSNP_OM_SetQueryDropStatus(trunk_ifindex, FALSE);
    }
#endif
#if(SYS_CPNT_IPV6_MULTICAST_DATA_DROP == TRUE)
    {
        BOOL_T member_status = FALSE, trunk_status = FALSE;
        MLDSNP_OM_GetMulticastDataDropStatus(trunk_ifindex, &trunk_status);
        MLDSNP_OM_GetMulticastDataDropStatus(member_ifindex, &member_status);

        if(trunk_status != member_status)    
        {
            MLDSNP_OM_SetMulticastDataDropStatus(member_ifindex, trunk_status);
            SWCTRL_PMGR_DropIpv6MulticastData(member_ifindex, trunk_status);
        }
        MLDSNP_OM_SetMulticastDataDropStatus(trunk_ifindex, FALSE);
    }
#endif
    return TRUE;
}/*End of MLDSNP_MGR_TrunkMemberLstDeleteCallback*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_VlanMemberDeletedCallBack
*------------------------------------------------------------------------------
* Purpose: This function the port remove from vlan
* INPUT  : vlan_ifindex  - the vlan ifindex
*          lport         - the lport will remove from the vlan
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_MGR_VlanMemberDeletedCallBack(
    UI32_T  vlan_ifindex,
    UI32_T  lport)
{
    UI32_T  vid = 0;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    MLDSNP_BD(CALLBACK, "vid=%ld, lport=%ld", vlan_ifindex, lport);

    if (SWCTRL_LogicalPortExisting(lport) == FALSE)
    {
        MLDSNP_BD(CALLBACK, "\nlport=%ld not exist", lport);
        return MLDSNP_TYPE_RETURN_FAIL;
    }

    if (VLAN_OM_ConvertFromIfindex(vlan_ifindex, &vid) == FALSE)
    {
        return FALSE;
    }


    if (FALSE == MLDSNP_ENGINE_PortLeaveVlanAllGroup(vid, lport))
    {
        MLDSNP_BD(CALLBACK, "lport=%ld, leave all group fail", lport);

        return FALSE;
    }

    return TRUE;
}/*End of MLDSNP_MGR_VlanMemberDeletedCallBack*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_VlanDestroyCallBack
*------------------------------------------------------------------------------
* Purpose: This function process the vlan is destroy
* INPUT  : vlan_ifindex  - the vlan ifindex
*          vlan_status   - the vlan destroy way
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_MGR_VlanDestroyCallBack(
    UI32_T  vlan_ifindex,
    UI32_T  vlan_status)
{
    UI32_T  vid = 0;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    MLDSNP_BD(CALLBACK, "vid=%ld, status=%ld", vlan_ifindex, vlan_status);

    if (VLAN_OM_ConvertFromIfindex(vlan_ifindex, &vid) == FALSE)
    {
        return FALSE;
    }

    if (FALSE == MLDSNP_ENIGNE_VlanDestroy(vid))
    {
        return FALSE;
    }


    if (FALSE == MLDSNP_ENGINE_DeleteAllGroupInVlan(vid))
    {
        MLDSNP_BD(CALLBACK, "vid=%ld, delete all group in a valn fail", vid);

        return FALSE;
    }

    if(TRUE == MLDSNP_OM_IsMrouteEnabled())
        msl_pmgr_mldsnp_set_status(vid, VAL_mldSnoopStatus_disabled);

    MLDSNP_OM_ClearInterfaceStatistics(vid, TRUE);
    return TRUE;
}/*End of MLDSNP_MGR_VlanDestroyCallBack*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_VlanCreatedCallBack
*------------------------------------------------------------------------------
* Purpose: This function process the vlan created
* INPUT  : vlan_ifindex  - the vlan ifindex
*          vlan_status   - the vlan create way
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_MGR_VlanCreatedCallBack(
    UI32_T  vlan_ifindex,
    UI32_T  vlan_status)
{
    MLDSNP_TYPE_MLDSNP_STATUS_T mldsnp_status;
    UI32_T  vid = 0;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    MLDSNP_BD(CALLBACK, "vlan ifindex=%ld, status=%ld", vlan_ifindex, vlan_status);

    if (VLAN_OM_ConvertFromIfindex(vlan_ifindex, &vid) == FALSE)
    {
        MLDSNP_BD(CALLBACK, "wrong vlan ifindex=%ld", vlan_ifindex);
        return FALSE;
    }

    if (FALSE == MLDSNP_ENIGNE_VlanCreated(vid))
    {
        MLDSNP_BD(CALLBACK, "vid=%ld, vlan create faill", vid);

        return FALSE;
    }

    if (TRUE == MLDSNP_OM_IsMrouteEnabled()
        && MLDSNP_OM_GetMldStatus(&mldsnp_status))
    {
        if (mldsnp_status == MLDSNP_TYPE_MLDSNP_ENABLED)
            msl_pmgr_mldsnp_set_status(vid, VAL_mldSnoopStatus_enabled);
        else
            msl_pmgr_mldsnp_set_status(vid, VAL_mldSnoopStatus_disabled);
    }

    return TRUE;
}/*End of MLDSNP_MGR_VlanCreatedCallBack*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_ProcessStpTC
*------------------------------------------------------------------------------
* Purpose: This function process topology change
* INPUT  : xstp_id - the instance id
*          lport - the enter forwarding port
*          is_root- this switch is root bridge or not
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :rfc 4541 2.1.1 4) General query may be send out on all active non-router port in order
*           to reduce network convergence
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_MGR_ProcessStpTC(UI32_T xstp_id, UI16_T lport, BOOL_T is_root)
{

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    MLDSNP_BD(CALLBACK, "xstp id=%ld, port=%d, is_root=%d", xstp_id, lport, is_root);

    /*in mldsnp only root bridge will send out query, but I think it needn't root bridge,
       every bridge shall do this. macauley
      */

    return MLDSNP_QUERIER_ProcessTopologyChange(xstp_id, lport);
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_MGR_SetIMRouteStatus
*------------------------------------------------------------------------------
* Purpose: This function set multicast routing satus
* INPUT  : xstp_id - the instance id
*          lport - the enter forwarding port
*          is_root- this switch is root bridge or not
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
MLDSNP_TYPE_ReturnValue_T  MLDSNP_MGR_SetIMRouteStatus(BOOL_T is_enabled)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return MLDSNP_TYPE_RETURN_SUCCESS;
    }

    MLDSNP_BD(UI, "IPv6 multicast routing status %s", is_enabled ? "Enabled" : "Disabled");

    MLDSNP_QUERIER_SetMRouteStatus(is_enabled);

    return MLDSNP_TYPE_RETURN_SUCCESS;
}


/* ------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_HandleHotInsertion
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 * ------------------------------------------------------------------------
 */
void MLDSNP_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{

    MLDSNP_BD(UI, "start %ld, num %ld ", starting_port_ifindex, number_of_port);

}

/* -----------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_MGR_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void MLDSNP_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    UI16_T i;

    MLDSNP_BD(UI, "start %ld, num %ld ", starting_port_ifindex, number_of_port);

    for (i = 0; i < number_of_port; i++)
    {
        MLDSNP_ENGINE_DeletePort(i + starting_port_ifindex);
    }

    return;
}

