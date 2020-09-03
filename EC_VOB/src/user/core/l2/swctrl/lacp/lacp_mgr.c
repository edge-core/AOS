/* ------------------------------------------------------------------------
 * Module Name: lacp_mgr.c
 * ------------------------------------------------------------------------
 * PURPOSE:
 * NOTES:
 *
 * MODIFICATION HISTORY:
 * Modifier             Date            Description
 * ------------------------------------------------------------------------
 * Lewis Kang           12-05-2001      First Created
 * Amy Tu               06-29-2002      Move callback functions from Task to MGR
 *                      07-29-2002      Each dynamic trunk per speed duplex should
 *                                      have different LAG ID
 * Allen Cheng          11-07-2002      Add functions for setting MIB entry
 *                      12-05-2002      Modify for trunk specification changed
 * ------------------------------------------------------------------------
 * Copyright(C)                              Accton Corporation, 2001, 2002
 * ------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_bld.h"
#include "sys_type.h"
#include "sysfun.h"
#include "sys_dflt.h"
#include "eh_mgr.h"
#include "eh_type.h"
#include "sys_module.h"
#include "syslog_type.h"

#include "lacp_type.h"
#include "lacp_om.h"
#include "lacp_om_private.h"
#include "lacp_mgr.h"
#include "lacp_util.h"
#include "swctrl.h"
#include "swctrl_pmgr.h"
#include "stktplg_pom.h"
#include "trk_lib.h"
#include "trk_pmgr.h"
#include "leaf_es3626a.h"
#include "leaf_ieee8023lag.h"

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
#include "sysctrl_xor_mgr.h"
#endif

#include "l_threadgrp.h"
#include "l2_l4_proc_comm.h"
#include "backdoor_mgr.h"

#if(SYS_CPNT_DEBUG == TRUE)
static char DEBUG_PageBuffer[1500];
static char DEBUG_LineBuffer[80];
#else
static char DEBUG_PageBuffer[1];
#endif/* #if(SYS_CPNT_DEBUG == TRUE) */

static L_THREADGRP_Handle_T tg_handle;
static UI32_T               backdoor_member_id;

/* Add by following STP's logic */
static BOOL_T   LACP_MGR_IsLeagalifIndex(UI32_T ifIndex);

static  BOOL_T  LACP_MGR_GetDot3adAggEntry_(LACP_MGR_Dot3adAggEntry_T *agg_entry);
static BOOL_T LACP_MGR_GetNextDot3adAggEntry_(LACP_MGR_Dot3adAggEntry_T *agg_entry);

static  BOOL_T  LACP_MGR_GetDot3adAggPortListEntry_(LACP_MGR_Dot3adAggPortListEntry_T *agg_port_list_entry);
static BOOL_T LACP_MGR_GetNextDot3adAggPortListEntry_(LACP_MGR_Dot3adAggPortListEntry_T *agg_port_list_entry);

static  BOOL_T  LACP_MGR_GetDot3adAggPortEntry_(LACP_MGR_Dot3adAggPortEntry_T *agg_port_entry);
static BOOL_T LACP_MGR_GetNextDot3adAggPortEntry_(LACP_MGR_Dot3adAggPortEntry_T *agg_port_entry);

static  BOOL_T  LACP_MGR_GetDot3adAggPortStatsEntry_(LACP_MGR_Dot3adAggPortStatsEntry_T *agg_port_stats_entry);
static BOOL_T LACP_MGR_GetNextDot3adAggPortStatsEntry_(LACP_MGR_Dot3adAggPortStatsEntry_T *agg_port_stats_entry);

static  BOOL_T  LACP_MGR_GetDot3adAggPortDebugEntry_(LACP_MGR_Dot3adAggPortDebugEntry_T *agg_port_debug_entry);
static BOOL_T LACP_MGR_GetNextDot3adAggPortDebugEntry_(LACP_MGR_Dot3adAggPortDebugEntry_T *agg_port_debug_entry);

static  BOOL_T  LACP_MGR_GetDot3adLagMibObjects_(LACP_MGR_LagMibObjects_T *lag_mib_objects);

static  UI32_T LACP_MGR_SetDot3adLacpPortEnabled_( UI32_T lport, UI32_T bEnabled);
static  UI32_T LACP_MGR_GetNextRunningDot3adLacpPortEnabled_(UI32_T *lport_ptr, UI32_T *bEnable);

/* For private MIB. */
static  BOOL_T  LACP_MGR_GetDot3adLacpPortEntry_(LACP_MGR_Dot3adLacpPortEntry_T *lacp_port_entry);
static BOOL_T LACP_MGR_GetNextDot3adLacpPortEntry_(LACP_MGR_Dot3adLacpPortEntry_T *lacp_port_entry);

static  LACP_RETURN_VALUE LACP_MGR_SetLACPState( UI32_T unit, UI32_T port, UI32_T lacp_state);
static  LACP_RETURN_VALUE LACP_MGR_GetLACPState( UI32_T unit, UI32_T port, UI32_T *lacp_state);

#define LACP_MGR_NONETAG     0xABCDABCD
static  UI32_T  none = LACP_MGR_NONETAG;
static void LACP_Print_BackdoorHelp(void);



/*  AmyTu 6-29-2002
  CallBack Functions for SWCTRL
 */
static  void    LACP_MGR_PortDuplexChange_Service(UI32_T unit, UI32_T port, UI32_T speed_duplex);
static  void    LACP_MGR_PortLinkup_Service(UI32_T unit, UI32_T port);
static  void    LACP_MGR_PortLinkdown_Service(UI32_T unit, UI32_T port);
static  void    LACP_MGR_PortAdminEnable_Service(UI32_T unit, UI32_T port);
static  void    LACP_MGR_PortAdminDisable_Service(UI32_T unit, UI32_T port);

#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
static  void    LACP_MGR_AddStaticTrunkMember_Service(UI32_T trunk_ifindex, UI32_T member_ifindex);
static  void    LACP_MGR_DelStaticTrunkMember_Service(UI32_T trunk_ifindex, UI32_T member_ifindex);
static  BOOL_T  LACP_MGR_SetAdminStatus(UI32_T if_index, BOOL_T enable_state);
static  BOOL_T  LACP_MGR_SetOperStatus(UI32_T if_index, BOOL_T enable_state);
#endif /* SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE */

/* For LACP MIB */
/* aggregatro: system priority */
static UI32_T  LACP_MGR_SetDot3adAggActorSystemPriority_(UI32_T agg_index, UI16_T priority);
/* aggregator: admin_key */
static UI32_T  LACP_MGR_SetDot3adAggActorAdminKey_(UI32_T agg_index, UI16_T admin_key);
static UI32_T  LACP_MGR_GetRunningDot3adAggActorAdminKey_(UI32_T agg_index, UI16_T *admin_key);
/* Actor: system_priority, admin_key, port_priority */
static UI32_T  LACP_MGR_SetDot3adAggPortActorSystemPriority_(UI16_T port_index, UI16_T priority);
static UI32_T  LACP_MGR_SetDot3adAggPortActorAdminKey_(UI16_T port_index, UI16_T admin_key);
static UI32_T  LACP_MGR_SetDot3adAggPortActorPortPriority_(UI16_T port_index, UI16_T priority);
static UI32_T  LACP_MGR_GetRunningDot3adAggPortActorSystemPriority_(UI16_T port_index, UI16_T *priority);
static UI32_T  LACP_MGR_GetRunningDot3adAggPortActorAdminKey_(UI16_T port_index, UI16_T *admin_key);
static UI32_T  LACP_MGR_GetRunningDot3adAggPortActorOperKey_(UI16_T port_index, UI16_T *oper_key);
static UI32_T  LACP_MGR_GetRunningDot3adAggPortActorPortPriority_(UI16_T port_index, UI16_T *priority);
static UI32_T  LACP_MGR_SetDefaultDot3adAggPortActorAdminKey_(UI16_T port_index);
static UI32_T  LACP_MGR_GetDefaultDot3adAggPortActorAdminKey_(UI16_T port_index, UI16_T *admin_key);
/* Partner: system_priority, admin_key, port_priority */
static UI32_T  LACP_MGR_SetDot3adAggPortPartnerAdminSystemPriority_(UI16_T port_index, UI16_T priority);
static UI32_T  LACP_MGR_SetDot3adAggPortPartnerAdminKey_(UI16_T port_index, UI16_T admin_key);
static UI32_T  LACP_MGR_SetDot3adAggPortPartnerAdminPortPriority_(UI16_T port_index, UI16_T priority);
static UI32_T  LACP_MGR_GetRunningDot3adAggPortPartnerAdminSystemPriority_(UI16_T port_index, UI16_T *priority);
static UI32_T  LACP_MGR_GetRunningDot3adAggPortPartnerAdminKey_(UI16_T port_index, UI16_T *admin_key);
static UI32_T  LACP_MGR_GetRunningDot3adAggPortPartnerAdminPortPriority_(UI16_T port_index, UI16_T *priority);

SYSFUN_DECLARE_CSC
/*--------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_Init
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will initialize system resouce for LACP.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void LACP_MGR_Init(void)
{
    /* 2. Initializ OM */
    /* 2.1 Init TaskManagement */
    /* Allen Cheng:
    LACP_OM_Tm_AccessOperationState(LACP_OM_TMINF_OPSTATE_TRANSITION, LACP_OM_TMINF_ACCESS_SET);
    */

    /* 2.2  Init OM : set default values */
    LACP_OM_Init();
} /* end of LACP_MGR_Init() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void LACP_MGR_Create_InterCSC_Relation(void)
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("lacp",
        SYS_BLD_LACP_GROUP_IPCMSGQ_KEY, LACP_MGR_BackDoorInfo);
} /* end of LACP_MGR_Init() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Enable lacp operation while in master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void LACP_MGR_EnterMasterMode(void)
{
    UI32_T  unit, port;
    LACP_PORT_T *port_ptr;

    LACP_OM_RefreshAllObject(); /* get factory default values and give initial values (may get from swctrl) */

    SYSFUN_ENTER_MASTER_MODE();
    for(unit = 1; unit <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK ; unit++ )
    {
        if (STKTPLG_POM_UnitExist(unit) == TRUE)
        {
            for( port = 1; port <= SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; port++)
            {
                port_ptr = LACP_FIND_PORT( unit, port);
                LACP_UTIL_RefreshPortSpeedDuplex(port_ptr);
                LACP_UTIL_RefreshOperStatusAndAdminKey(port_ptr);
            } /* End of for */
        }
    }
    return;
} /* end of LACP_MGR_EnterMasterMode() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : It's the temporary transition mode between system into master
 *            mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void LACP_MGR_EnterTransitionMode(void)
{
    UI32_T  index;
    LACP_SYSTEM_T       *lacp_system_ptr;

    /* stop processing LACP packets */
    lacp_system_ptr             = LACP_OM_GetSystemPtr();
    lacp_system_ptr->lacp_oper  = LACP_ADMIN_OFF;

    /* stop timers for state machines */

    /* Allen Cheng:
    oper_state = LACP_OM_TMINF_OPSTATE_TRANSITION;
    LACP_OM_Tm_AccessOperationState(oper_state, LACP_OM_TMINF_OPSTATE_TRANSITION);
    */
    SYSFUN_ENTER_TRANSITION_MODE();

    /* ========================================================== */
    LACP_OM_EnterCriticalRegion();
    for (index = 1; index <= MAX_PORTS; index++)
    {
        LACP_PORT_T *pPort;

        pPort = LACP_OM_GetPortPtr(index);

        pPort->LinkUp = FALSE;
        pPort->Port_OperStatus = FALSE;

        (pPort->AggActorAdminPort).key = SYS_DFLT_LACP_KEY_DEFAULT;

        LACP_Tx_Machine      ( pPort, LACP_RX_PORT_DOWN_EV);
        LACP_Periodic_Machine( pPort, LACP_RX_PORT_DOWN_EV);
        LACP_Rx_Machine      ( pPort, LACP_RX_PORT_DOWN_EV, NULL);
        LACP_Mux_Machine     ( pPort, LACP_NEW_INFO_EV);
        LACP_Mux_Machine     ( pPort, LACP_RX_PORT_DOWN_EV);
    }
    LACP_OM_LeaveCriticalRegion();
    /* ========================================================== */

    /*
    printf("LACP_MGR_EnterTransitionMode\n");
    */

} /* end of LACP_MGR_EnterTransitionMode() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Disable the lacp operation while in slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------- */
void LACP_MGR_EnterSlaveMode(void)
{
    LACP_SYSTEM_T       *lacp_system_ptr;

    /* stop processing LACP packets */
    lacp_system_ptr             = LACP_OM_GetSystemPtr();
    lacp_system_ptr->lacp_oper  = LACP_ADMIN_OFF;

    /* Allen Cheng:
    oper_state = LACP_OM_TMINF_OPSTATE_SLAVE;
    LACP_OM_Tm_AccessOperationState(oper_state, LACP_OM_TMINF_OPSTATE_SLAVE);
    */
    SYSFUN_ENTER_SLAVE_MODE();
} /* end of LACP_MGR_EnterSlaveMode() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void  LACP_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
    return;
} /* end of LACP_MGR_SetTransitionMode() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetOperationMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns the current operation mode of this component
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T    LACP_MGR_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
}/* End of LACP_MGR_GetOperationMode    */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_IsLeagalifIndex
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion check the input ifIndex is a legal ifIndex
 *              for LACP_MGR.
 * INPUT    :   agg_entry->dot3ad_agg_index
 * OUTPUT   :   agg_entry                  -- aggregator entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   Even the ifIndex is correct in SWCTRL, it does not means it
 *              is a legal index for LACP_MGR.
 * ------------------------------------------------------------------------
 */
static BOOL_T LACP_MGR_IsLeagalifIndex(UI32_T ifIndex)
{
    UI32_T slot     = 0,
           port     = 0,
           trunk_id = 0;

    SWCTRL_Lport_Type_T lport_type = SWCTRL_LPORT_UNKNOWN_PORT;

    lport_type = SWCTRL_LogicalPortToUserPort( ifIndex, &slot, &port, &trunk_id);

    if(SWCTRL_LPORT_UNKNOWN_PORT == lport_type)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_IsLeagalifIndex_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Input IfIndex");
        return FALSE;
    }

    return TRUE;
}

/* The dot3adAggEntry group (dot3adAggTable 1) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified Agg entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   agg_entry->dot3ad_agg_index
 * OUTPUT   :   agg_entry                  -- aggregator entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_MGR_GetDot3adAggEntry(LACP_MGR_Dot3adAggEntry_T *agg_entry)
{
    BOOL_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    retval=LACP_MGR_GetDot3adAggEntry_(agg_entry);

    return retval;
} /* LACP_MGR_GetDot3adAggEntry */

BOOL_T LACP_MGR_GetNextDot3adAggEntry(LACP_MGR_Dot3adAggEntry_T *agg_entry)
{
    BOOL_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    retval=LACP_MGR_GetNextDot3adAggEntry_(agg_entry);

    return retval;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetNextDot3adAggEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next aggregator index of
 *              the specified Agg entry info can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   agg_entry->dot3ad_agg_index
 * OUTPUT   :   agg_entry                  -- aggregator entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
static BOOL_T LACP_MGR_GetNextDot3adAggEntry_(LACP_MGR_Dot3adAggEntry_T *agg_entry)
{
    /* Here we first get the next index of aggregator then call get function. */
    UI32_T agg_index = agg_entry->dot3ad_agg_index;

    agg_index = LACP_GetNext_Appropriate_ifIndex_From_ifIndex( agg_index);

    if(!agg_index)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetNextDot3adAggEntry__Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Input IfIndex");
        return FALSE;
    }
    else
    {
        agg_entry->dot3ad_agg_index = agg_index;
    }

    return LACP_MGR_GetDot3adAggEntry_( agg_entry);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggEntry_
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified agg entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
static  BOOL_T  LACP_MGR_GetDot3adAggEntry_(LACP_MGR_Dot3adAggEntry_T *agg_entry)
{
    UI32_T agg_index = agg_entry->dot3ad_agg_index;
    UI8_T  aggregator_mac[6]       = {0};
    UI16_T actor_system_priority   = 0;
    UI8_T  actor_system_id[6]      = {0};
    UI8_T  aggregate_or_individual = 0;
    UI16_T actor_admin_key         = SYS_DFLT_LACP_KEY_NULL;
    UI16_T actor_oper_key          = SYS_DFLT_LACP_KEY_NULL;
    UI8_T  partner_system_id[6]    = {0};
    UI16_T partner_system_priority = 0;
    UI16_T partner_oper_key        = SYS_DFLT_LACP_KEY_NULL;
    UI16_T collector_max_delay     = 0;
    UI32_T agg_id;
    UI32_T trunk_id;
    UI32_T actor_timeout;


    #if 0
    if (SWCTRL_LogicalPortToUserPort(agg_index, &unit, &port, &trunk_id) != SWCTRL_LPORT_TRUNK_PORT)
    #endif
    if ( !TRK_OM_IfindexToTrunkId(agg_index, &trunk_id) )
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggEntry__Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Input IfIndex");
        return FALSE;
    }

    agg_id = (trunk_id - 1);

    if(FALSE == LACP_Get_dot3adAggMACAddress( agg_id, aggregator_mac))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Aggregator Mac address");
        return FALSE;
    }

    if(FALSE == LACP_Get_dot3adAggActorSystemPriority( agg_id, &actor_system_priority))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Actor system priority");
        return FALSE;
    }

    if(FALSE == LACP_Get_dot3adAggActorSystemID( agg_id, actor_system_id))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Actor system ID");
        return FALSE;
    }

    if(FALSE == LACP_Get_dot3adAggAggregateOrIndividual( agg_id, &aggregate_or_individual))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Aggregator_individual status");
        return FALSE;
    }

    if(FALSE == LACP_Get_dot3adAggActorAdminKey( agg_id, &actor_admin_key))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Actor admin key");
        return FALSE;
    }

    if(FALSE == LACP_Get_dot3adAggActorOperKey( agg_id, &actor_oper_key))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Actor oper key");
        return FALSE;
    }

    if(FALSE == LACP_Get_dot3adAggPartnerSystemID( agg_id, partner_system_id))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Partner system ID");
        return FALSE;
    }

    if(FALSE == LACP_Get_dot3adAggPartnerSystemPriority( agg_id, &partner_system_priority))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Partner system priority");
        return FALSE;
    }

    if(FALSE == LACP_Get_dot3adAggPartnerOperKey( agg_id, &partner_oper_key))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Partner oper key");
        return FALSE;
    }

    if(FALSE == LACP_Get_dot3adAggCollectorMaxDelay( agg_id, &collector_max_delay))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Collector max delay");
        return FALSE;
    }

    if(FALSE == LACP_Get_dot3adAggActorTimeout( agg_id, &actor_timeout))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Actor timeout");
        return FALSE;
    }

    /* Now we got all data we want, just fill it into user's data structure. */
    /* First aggregator MAC */
    agg_entry->dot3ad_agg_mac_address[0] = aggregator_mac[0];
    agg_entry->dot3ad_agg_mac_address[1] = aggregator_mac[1];
    agg_entry->dot3ad_agg_mac_address[2] = aggregator_mac[2];
    agg_entry->dot3ad_agg_mac_address[3] = aggregator_mac[3];
    agg_entry->dot3ad_agg_mac_address[4] = aggregator_mac[4];
    agg_entry->dot3ad_agg_mac_address[5] = aggregator_mac[5];

    /* Then Actor System Priority */
    agg_entry->dot3ad_agg_actor_system_priority = actor_system_priority;

    /* Actor system id */
    agg_entry->dot3ad_agg_actor_system_id[0] = actor_system_id[0];
    agg_entry->dot3ad_agg_actor_system_id[1] = actor_system_id[1];
    agg_entry->dot3ad_agg_actor_system_id[2] = actor_system_id[2];
    agg_entry->dot3ad_agg_actor_system_id[3] = actor_system_id[3];
    agg_entry->dot3ad_agg_actor_system_id[4] = actor_system_id[4];
    agg_entry->dot3ad_agg_actor_system_id[5] = actor_system_id[5];

    /* AggregateOrIndividual */
    agg_entry->dot3ad_agg_aggregate_or_individual = aggregate_or_individual;

    /* Actor Admin Key */
    agg_entry->dot3ad_agg_actor_admin_key = (UI32_T)actor_admin_key;

    /* Actor Oper. Key */
    agg_entry->dot3ad_agg_actor_oper_key = (UI32_T)actor_oper_key;

    /* Partner System Id */
    agg_entry->dot3ad_agg_partner_system_id[0] = partner_system_id[0];
    agg_entry->dot3ad_agg_partner_system_id[1] = partner_system_id[1];
    agg_entry->dot3ad_agg_partner_system_id[2] = partner_system_id[2];
    agg_entry->dot3ad_agg_partner_system_id[3] = partner_system_id[3];
    agg_entry->dot3ad_agg_partner_system_id[4] = partner_system_id[4];
    agg_entry->dot3ad_agg_partner_system_id[5] = partner_system_id[5];

    /* Partner System Priority */
    agg_entry->dot3ad_agg_partner_system_priority = partner_system_priority;

    /* Partner Oper. Key */
    agg_entry->dot3ad_agg_partner_oper_key = partner_oper_key;

    /* Collector Max. Delay */
    agg_entry->dot3ad_agg_collector_max_delay = collector_max_delay;

    /* Actor timeout */
    agg_entry->dot3ad_agg_actor_timeout = actor_timeout;

    return TRUE;
} /* LACP_MGR_GetDot3adAggEntry_ */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggPortListEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port list entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_MGR_GetDot3adAggPortListEntry(LACP_MGR_Dot3adAggPortListEntry_T *agg_port_list_entry)
{
    BOOL_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    retval=LACP_MGR_GetDot3adAggPortListEntry_(agg_port_list_entry);

    return retval;
} /* LACP_MGR_GetDot3adAggPortListEntry */

BOOL_T LACP_MGR_GetNextDot3adAggPortListEntry(LACP_MGR_Dot3adAggPortListEntry_T *agg_port_list_entry)
{
    BOOL_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    retval=LACP_MGR_GetNextDot3adAggPortListEntry_(agg_port_list_entry);

    return retval;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetNextDot3adAggPortListEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next aggregator index of
 *              the specified Agg entry info can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
static BOOL_T LACP_MGR_GetNextDot3adAggPortListEntry_(LACP_MGR_Dot3adAggPortListEntry_T *agg_port_list_entry)
{
    /* Here we first get the next index of aggregator then call get function. */
    UI32_T ifIndex = agg_port_list_entry->dot3ad_agg_index;

    ifIndex = LACP_GetNext_Appropriate_ifIndex_From_ifIndex( ifIndex);

    if(!ifIndex)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetNextDot3adAggPortListEntry__Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Input IfIndex");
        return FALSE;
    }
    else
    {
        agg_port_list_entry->dot3ad_agg_index = ifIndex;
    }

    return LACP_MGR_GetDot3adAggPortListEntry_( agg_port_list_entry);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggPortListEntry_
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port list entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
static  BOOL_T  LACP_MGR_GetDot3adAggPortListEntry_(LACP_MGR_Dot3adAggPortListEntry_T *agg_port_list_entry)
{
    UI32_T ifIndex = agg_port_list_entry->dot3ad_agg_index;
    UI8_T  port_list[LACP_PORT_LIST_OCTETS];
    UI32_T agg_id;
    UI32_T trunk_id;

    if(FALSE ==LACP_MGR_IsLeagalifIndex( ifIndex))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortListEntry__Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Input IfIndex");
        return FALSE;
    }

    if ( !TRK_OM_IfindexToTrunkId(ifIndex, &trunk_id) )
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortListEntry__Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Input IfIndex");
        return FALSE;
    }

    agg_id = (trunk_id - 1);

    if(FALSE == LACP_Get_dot3adAggPortListPorts( agg_id, port_list))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortListEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Aggregation port list");
        return FALSE;
    }

    memcpy( agg_port_list_entry->dot3ad_agg_port_list_ports, port_list, LACP_PORT_LIST_OCTETS);
    LACP_UTIL_SetDot3AggLastChange_Time();
    return TRUE;
} /* LACP_MGR_GetDot3adAggPortListEntry_ */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_MGR_GetDot3adAggPortEntry(LACP_MGR_Dot3adAggPortEntry_T *agg_port_entry)
{
    BOOL_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    retval=LACP_MGR_GetDot3adAggPortEntry_(agg_port_entry);

    return retval;
} /* LACP_MGR_GetDot3adAggPortEntry */

BOOL_T LACP_MGR_GetNextDot3adAggPortEntry(LACP_MGR_Dot3adAggPortEntry_T *agg_port_entry)
{
    BOOL_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    retval=LACP_MGR_GetNextDot3adAggPortEntry_(agg_port_entry);

    return retval;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetNextDot3adAggPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   agg_port_entry
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
static BOOL_T LACP_MGR_GetNextDot3adAggPortEntry_(LACP_MGR_Dot3adAggPortEntry_T *agg_port_entry)
{
    /* Here we first get the next index of aggregator then call get function. */
    UI32_T ifIndex = agg_port_entry->dot3ad_agg_port_index;

    ifIndex = LACP_GetNext_Appropriate_Physical_ifIndex_From_ifIndex( ifIndex);

    if(!ifIndex)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetNextDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Input IfIndex");
        return FALSE;
    }
    else
    {
        agg_port_entry->dot3ad_agg_port_index = ifIndex;
    }

    return LACP_MGR_GetDot3adAggPortEntry_( agg_port_entry);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggPortEntry_
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
static  BOOL_T  LACP_MGR_GetDot3adAggPortEntry_(LACP_MGR_Dot3adAggPortEntry_T *agg_port_entry)
{
    UI32_T ifIndex = agg_port_entry->dot3ad_agg_port_index;

    UI16_T actor_system_priority         = 0;
    UI8_T  actor_system_id[6]            = {0};
    UI16_T actor_admin_key               = 0;
    UI16_T actor_oper_key                = 0;
    UI16_T partner_admin_system_priority = 0;
    UI16_T partner_oper_system_priority  = 0;
    UI8_T  partner_admin_system_id[6]    = {0};
    UI8_T  partner_oper_system_id[6]     = {0};
    UI16_T partner_admin_key             = 0;
    UI16_T partner_oper_key              = 0;
    UI32_T selected_agg_id               = 0;
    UI32_T attached_agg_id               = 0;
    UI16_T actor_port                    = 0;
    UI16_T actor_port_priority           = 0;
    UI16_T partner_admin_port            = 0;
    UI16_T partner_oper_port             = 0;
    UI16_T partner_admin_port_priority   = 0;
    UI16_T partner_oper_port_priority    = 0;
    UI8_T  actor_admin_state             = 0;
    UI8_T  actor_oper_state              = 0;
    UI8_T  partner_admin_state           = 0;
    UI8_T  partner_oper_state            = 0;
    UI8_T  aggregate_or_individual       = 0;

    if(FALSE ==LACP_MGR_IsLeagalifIndex( ifIndex))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Input IfIndex");
        return FALSE;
    }

    if(FALSE == LACP_Get_dot3adAggPortActorSystemPriority( ifIndex, &actor_system_priority))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Actor system priority");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortActorSystemID( ifIndex, actor_system_id))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Actor system ID");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortActorAdminKey( ifIndex, &actor_admin_key))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Actor admin key");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortActorOperKey( ifIndex, &actor_oper_key))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Actor oper key");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortPartnerAdminSystemPriority( ifIndex, &partner_admin_system_priority))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Partner admin system priority");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortPartnerOperSystemPriority( ifIndex, &partner_oper_system_priority))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Partner oper system priority");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortPartnerAdminSystemID( ifIndex, partner_admin_system_id))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Partner admin system ID");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortPartnerOperSystemID( ifIndex, partner_oper_system_id))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Partner oper system ID");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortPartnerAdminKey( ifIndex, &partner_admin_key))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Partner admin key");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortPartnerOperKey( ifIndex, &partner_oper_key))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Partner oper key");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortSelectedAggID( ifIndex, &selected_agg_id))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Selected aggregator ID");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortAttachedAggID( ifIndex, &attached_agg_id))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Attached aggregator ID");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortActorPort( ifIndex, &actor_port))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Actor port");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortActorPortPriority( ifIndex, &actor_port_priority))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Actor port priority");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortPartnerAdminPort( ifIndex, &partner_admin_port))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Partner admin port");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortPartnerOperPort( ifIndex, &partner_oper_port))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Partner oper port");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortPartnerAdminPortPriority( ifIndex, &partner_admin_port_priority))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Partner admin port priority");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortPartnerOperPortPriority( ifIndex, &partner_oper_port_priority))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Partner oper port priority");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortActorAdminState( ifIndex, &actor_admin_state))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Actor admin state");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortActorOperState( ifIndex, &actor_oper_state))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Actor oper state");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortPartnerAdminState( ifIndex, &partner_admin_state))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Partner admin state");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortPartnerOperState( ifIndex, &partner_oper_state))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Partner oper state");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortAggregateOrIndividual( ifIndex, &aggregate_or_individual))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Aggregation individual state");
        return FALSE;
    }

    agg_port_entry->dot3ad_agg_port_actor_system_priority = actor_system_priority;

    agg_port_entry->dot3ad_agg_port_actor_system_id[0] = actor_system_id[0];
    agg_port_entry->dot3ad_agg_port_actor_system_id[1] = actor_system_id[1];
    agg_port_entry->dot3ad_agg_port_actor_system_id[2] = actor_system_id[2];
    agg_port_entry->dot3ad_agg_port_actor_system_id[3] = actor_system_id[3];
    agg_port_entry->dot3ad_agg_port_actor_system_id[4] = actor_system_id[4];
    agg_port_entry->dot3ad_agg_port_actor_system_id[5] = actor_system_id[5];

    agg_port_entry->dot3ad_agg_port_actor_admin_key = actor_admin_key;
    agg_port_entry->dot3ad_agg_port_actor_oper_key  = actor_oper_key;

    agg_port_entry->dot3ad_agg_port_partner_admin_system_priority = partner_admin_system_priority;
    agg_port_entry->dot3ad_agg_port_partner_oper_system_priority  = partner_oper_system_priority;

    agg_port_entry->dot3ad_agg_port_partner_admin_system_id[0] = partner_admin_system_id[0];
    agg_port_entry->dot3ad_agg_port_partner_admin_system_id[1] = partner_admin_system_id[1];
    agg_port_entry->dot3ad_agg_port_partner_admin_system_id[2] = partner_admin_system_id[2];
    agg_port_entry->dot3ad_agg_port_partner_admin_system_id[3] = partner_admin_system_id[3];
    agg_port_entry->dot3ad_agg_port_partner_admin_system_id[4] = partner_admin_system_id[4];
    agg_port_entry->dot3ad_agg_port_partner_admin_system_id[5] = partner_admin_system_id[5];

    agg_port_entry->dot3ad_agg_port_partner_oper_system_id[0] = partner_oper_system_id[0];
    agg_port_entry->dot3ad_agg_port_partner_oper_system_id[1] = partner_oper_system_id[1];
    agg_port_entry->dot3ad_agg_port_partner_oper_system_id[2] = partner_oper_system_id[2];
    agg_port_entry->dot3ad_agg_port_partner_oper_system_id[3] = partner_oper_system_id[3];
    agg_port_entry->dot3ad_agg_port_partner_oper_system_id[4] = partner_oper_system_id[4];
    agg_port_entry->dot3ad_agg_port_partner_oper_system_id[5] = partner_oper_system_id[5];

    agg_port_entry->dot3ad_agg_port_partner_admin_key = partner_admin_key;
    agg_port_entry->dot3ad_agg_port_partner_oper_key  = partner_oper_key;

    agg_port_entry->dot3ad_agg_port_selected_agg_id = selected_agg_id;
    agg_port_entry->dot3ad_agg_port_attached_agg_id = attached_agg_id;

    agg_port_entry->dot3ad_agg_port_actor_port          = actor_port;
    agg_port_entry->dot3ad_agg_port_actor_port_priority = actor_port_priority;

    agg_port_entry->dot3ad_agg_port_partner_admin_port = partner_admin_port;
    agg_port_entry->dot3ad_agg_port_partner_oper_port  = partner_oper_port;

    agg_port_entry->dot3ad_agg_port_partner_admin_port_priority = partner_admin_port_priority;
    agg_port_entry->dot3ad_agg_port_partner_oper_port_priority  = partner_oper_port_priority;

    agg_port_entry->dot3ad_agg_port_actor_admin_state = actor_admin_state;
    agg_port_entry->dot3ad_agg_port_actor_oper_state  = actor_oper_state;

    agg_port_entry->dot3ad_agg_port_partner_admin_state = partner_admin_state;
    agg_port_entry->dot3ad_agg_port_partner_oper_state  = partner_oper_state;

    agg_port_entry->dot3ad_agg_port_aggregate_or_individual = aggregate_or_individual;

    return TRUE;
} /* LACP_MGR_GetDot3adAggPortEntry_ */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggPortStatsEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port stats entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_MGR_GetDot3adAggPortStatsEntry(LACP_MGR_Dot3adAggPortStatsEntry_T *agg_port_stats_entry)
{
    BOOL_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    retval=LACP_MGR_GetDot3adAggPortStatsEntry_(agg_port_stats_entry);

    return retval;
} /* LACP_MGR_GetDot3adAggPortStatsEntry */

BOOL_T LACP_MGR_GetNextDot3adAggPortStatsEntry(LACP_MGR_Dot3adAggPortStatsEntry_T *agg_port_stats_entry)
{
    BOOL_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    retval=LACP_MGR_GetNextDot3adAggPortStatsEntry_(agg_port_stats_entry);

    return retval;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggPortStatsEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port stats entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
static BOOL_T LACP_MGR_GetNextDot3adAggPortStatsEntry_(LACP_MGR_Dot3adAggPortStatsEntry_T *agg_port_stats_entry)
{
    /* Here we first get the next index of aggregator then call get function. */
    UI32_T ifIndex = agg_port_stats_entry->dot3ad_agg_port_index;

    ifIndex = LACP_GetNext_Appropriate_Physical_ifIndex_From_ifIndex( ifIndex);

    if(!ifIndex)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetNextDot3adAggPortStatsEntry__Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Input IfIndex");
        return FALSE;
    }
    else
    {
        agg_port_stats_entry->dot3ad_agg_port_index = ifIndex;
    }

    return LACP_MGR_GetDot3adAggPortStatsEntry_( agg_port_stats_entry);
} /* LACP_MGR_GetNextDot3adAggPortStatsEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggPortStatsEntry_
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port stats entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
static  BOOL_T  LACP_MGR_GetDot3adAggPortStatsEntry_(LACP_MGR_Dot3adAggPortStatsEntry_T *agg_port_stats_entry)
{
    UI32_T ifIndex = agg_port_stats_entry->dot3ad_agg_port_index;

    UI32_T lac_pdus_rx             = 0;
    UI32_T marker_pdus_rx          = 0;
    UI32_T marker_response_pdus_rx = 0;
    UI32_T unknown_rx              = 0;
    UI32_T illegal_rx              = 0;
    UI32_T lac_pdus_tx             = 0;
    UI32_T marker_pdus_tx          = 0;
    UI32_T marker_response_pdus_tx = 0;

    if(FALSE ==LACP_MGR_IsLeagalifIndex( ifIndex))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortStatsEntry__Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Input IfIndex");
        return FALSE;
    }

    if(FALSE == LACP_Get_dot3adAggPortStatsLACPDUsRx( ifIndex, &lac_pdus_rx))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortStatsEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Lac pdus rx counter");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortStatsMarkerPDUsRx( ifIndex, &marker_pdus_rx))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortStatsEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Marker pdus rx counter");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortStatsMarkerResponsePDUsRx( ifIndex, &marker_response_pdus_rx))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortStatsEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Marker response pdus rx counter");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortStatsUnknownRx( ifIndex, &unknown_rx))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortStatsEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Unknown rx counter");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortStatsIllegalRx( ifIndex, &illegal_rx))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortStatsEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Illegal rx counter");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortStatsLACPDUsTx( ifIndex, &lac_pdus_tx))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortStatsEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Lac pdus tx counter");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortStatsMarkerPDUsTx( ifIndex, &marker_pdus_tx))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortStatsEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Marker response pdus tx counter");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortStatsMarkerResponsePDUsTx( ifIndex, &marker_response_pdus_tx))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortStatsEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "Marker response pdus tx counter");
        return FALSE;
    }

    agg_port_stats_entry->dot3ad_agg_port_stats_lac_pdus_rx             = lac_pdus_rx;
    agg_port_stats_entry->dot3ad_agg_port_stats_marker_pdus_rx          = marker_pdus_rx;
    agg_port_stats_entry->dot3ad_agg_port_stats_marker_response_pdus_rx = marker_response_pdus_rx;
    agg_port_stats_entry->dot3ad_agg_port_stats_unknown_rx              = unknown_rx;
    agg_port_stats_entry->dot3ad_agg_port_stats_illegal_rx              = illegal_rx;
    agg_port_stats_entry->dot3ad_agg_port_stats_lac_pdus_tx             = lac_pdus_tx;
    agg_port_stats_entry->dot3ad_agg_port_stats_marker_pdus_tx          = marker_pdus_tx;
    agg_port_stats_entry->dot3ad_agg_port_stats_marker_response_pdus_tx = marker_response_pdus_tx;

    return TRUE;
} /* LACP_MGR_GetDot3adAggPortStatsEntry_ */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggPortDebugEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port debug entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_MGR_GetDot3adAggPortDebugEntry(LACP_MGR_Dot3adAggPortDebugEntry_T *agg_port_debug_entry)
{
    BOOL_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    retval=LACP_MGR_GetDot3adAggPortDebugEntry_(agg_port_debug_entry);

    return retval;
} /* LACP_MGR_GetDot3adAggPortDebugEntry */

BOOL_T LACP_MGR_GetNextDot3adAggPortDebugEntry(LACP_MGR_Dot3adAggPortDebugEntry_T *agg_port_debug_entry)
{
    BOOL_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    retval=LACP_MGR_GetNextDot3adAggPortDebugEntry_(agg_port_debug_entry);

    return retval;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggPortDebugEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port debug entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
static BOOL_T LACP_MGR_GetNextDot3adAggPortDebugEntry_(LACP_MGR_Dot3adAggPortDebugEntry_T *agg_port_debug_entry)
{
    /* Here we first get the next index of aggregator then call get function. */
    UI32_T ifIndex = agg_port_debug_entry->dot3ad_agg_port_index;

    ifIndex = LACP_GetNext_Appropriate_Physical_ifIndex_From_ifIndex( ifIndex);

    if(!ifIndex)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetNextDot3adAggPortDebugEntry__Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Input IfIndex");
        return FALSE;
    }
    else
    {
        agg_port_debug_entry->dot3ad_agg_port_index = ifIndex;
    }

    return LACP_MGR_GetDot3adAggPortDebugEntry_( agg_port_debug_entry);

} /* LACP_MGR_GetNextDot3adAggPortDebugEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggPortDebugEntry_
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port debug entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
static  BOOL_T  LACP_MGR_GetDot3adAggPortDebugEntry_(LACP_MGR_Dot3adAggPortDebugEntry_T *agg_port_debug_entry)
{
    UI32_T ifIndex = agg_port_debug_entry->dot3ad_agg_port_index;

    UI8_T  rx_state                      = 0;
    UI32_T last_rx_time                  = 0;
    UI8_T  mux_state                     = 0;
    UI8_T  mux_reason[MUX_REASON_LENGTH] = {0};
    UI8_T  actor_churn_state             = 0;
    UI8_T  partner_churn_state           = 0;
    UI32_T actor_churn_count             = 0;
    UI32_T partner_churn_count           = 0;
    UI32_T actor_sync_transition_count   = 0;
    UI32_T partner_sync_transition_count = 0;
    UI32_T actor_change_count            = 0;
    UI32_T partner_change_count          = 0;

    /* UI32_T i; */

    if(FALSE ==LACP_MGR_IsLeagalifIndex( ifIndex))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortDebugEntry__Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Input IfIndex");
        return FALSE;
    }

    if(FALSE == LACP_Get_dot3adAggPortDebugRxState( ifIndex, &rx_state))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortDebugEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "rx state");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortDebugLastRxTime( ifIndex, &last_rx_time))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortDebugEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "last rx time");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortDebugMuxState( ifIndex, &mux_state))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortDebugEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "mux state");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortDebugMuxReason( ifIndex, mux_reason))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortDebugEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "mux reason");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortDebugActorChurnState( ifIndex, &actor_churn_state))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortDebugEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "actor churn reason");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortDebugPartnerChurnState( ifIndex, &partner_churn_state))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortDebugEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "partner churn reason");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortDebugActorChurnCount( ifIndex, &actor_churn_count))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortDebugEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "actor churn count");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortDebugPartnerChurnCount( ifIndex, &partner_churn_count))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortDebugEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "partner churn count");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortDebugActorSyncTransitionCount( ifIndex, &actor_sync_transition_count))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortDebugEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "actor sync transition count");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortDebugPartnerSyncTransitionCount( ifIndex, &partner_sync_transition_count))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortDebugEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "partner sync transition count");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortDebugActorChangeCount( ifIndex, &actor_change_count))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortDebugEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "actor change count");
        return FALSE;
    }
    if(FALSE == LACP_Get_dot3adAggPortDebugPartnerChangeCount( ifIndex, &partner_change_count))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adAggPortDebugEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "partner change count");
        return FALSE;
    }

    agg_port_debug_entry->dot3ad_agg_port_debug_rx_state     = rx_state;
    agg_port_debug_entry->dot3ad_agg_port_debug_last_rx_time = last_rx_time;
    agg_port_debug_entry->dot3ad_agg_port_debug_mux_state    = mux_state;

    memcpy( agg_port_debug_entry->dot3ad_agg_port_debug_mux_reason, mux_reason, MUX_REASON_LENGTH);

    agg_port_debug_entry->dot3ad_agg_port_debug_actor_churn_state   = actor_churn_state;
    agg_port_debug_entry->dot3ad_agg_port_debug_partner_churn_state = partner_churn_state;
    agg_port_debug_entry->dot3ad_agg_port_debug_actor_churn_count   = actor_churn_count;
    agg_port_debug_entry->dot3ad_agg_port_debug_partner_churn_count = partner_churn_count;

    agg_port_debug_entry->dot3ad_agg_port_debug_actor_sync_transition_count   = actor_sync_transition_count;
    agg_port_debug_entry->dot3ad_agg_port_debug_partner_sync_transition_count = partner_sync_transition_count;
    agg_port_debug_entry->dot3ad_agg_port_debug_actor_change_count            = actor_change_count;
    agg_port_debug_entry->dot3ad_agg_port_debug_partner_change_count          = partner_change_count;

    return TRUE;
} /* LACP_MGR_GetDot3adAggPortDebugEntry_ */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adLagMibObjects
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified lag mib objects
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_MGR_GetDot3adLagMibObjects(LACP_MGR_LagMibObjects_T *lag_mib_objects)
{
    BOOL_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    retval=LACP_MGR_GetDot3adLagMibObjects_(lag_mib_objects);

    return retval;
} /* LACP_MGR_GetDot3adLagMibObjects */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adLagMibObjects_
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified base entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
static  BOOL_T  LACP_MGR_GetDot3adLagMibObjects_(LACP_MGR_LagMibObjects_T *lag_mib_objects)
{
    UI32_T last_changed;

    if(FALSE == LACP_Get_dot3adTablesLastChanged( &last_changed))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adLagMibObjects__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "dot3ad table last changed");
        return FALSE;
    }

    lag_mib_objects->dot3ad_tables_last_changed = last_changed;

    return TRUE;
} /* LACP_MGR_GetDot3adLagMibObjects_ */

static void LACP_Print_BackdoorHelp(void)
{
    BACKDOOR_MGR_Printf("\n\t1 : Show Port Protocol Oper. Parameters");
    BACKDOOR_MGR_Printf("\n\t2 : Show Port Protocol Admin. Parameters");
    BACKDOOR_MGR_Printf("\n\t3 : Show Port Protocol Statistics");
    BACKDOOR_MGR_Printf("\n\t4 : Show Port Protocol State");
    BACKDOOR_MGR_Printf("\n\t5 : Show Raw Protocol info.");
    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("\n\t6 : Toggle Pdu debug flag.");
    BACKDOOR_MGR_Printf("\n\t7 : Toggle callback etc. debug flag.");
    BACKDOOR_MGR_Printf("\n\t8 : Toggle trunk debug message");
    BACKDOOR_MGR_Printf("\n\t9 : Toggle try to attach debug message");
    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("\n\ta : Reset Port Protocol Statistics");
    BACKDOOR_MGR_Printf("\n\tb : Show ports attach to aggregator");
    BACKDOOR_MGR_Printf("\n\tc : Show LastChangeTime");
    BACKDOOR_MGR_Printf("\n\td : Show Route Trace Path");
    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf("\n\tx : Exit LACP Backdoor function");
    BACKDOOR_MGR_Printf("\n");
}

void    LACP_MGR_BackDoorInfo(void)
{
    #define MAXLINE 255
    UI32_T unit, port;
    UI8_T ch = 'x';
    char line_buffer[MAXLINE];
    LACP_MGR_LagMibObjects_T last_changed;

    tg_handle = L2_L4_PROC_COMM_GetLacpGroupTGHandle();
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY,
            &backdoor_member_id) == FALSE)
    {
        printf("\n%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    BACKDOOR_MGR_Printf("\n LACP Backdoor Selection");

    while(1)
    {
        LACP_Print_BackdoorHelp();

        ch = BACKDOOR_MGR_GetChar();
        switch(ch)
        {
        case '1':
            BACKDOOR_MGR_Printf("\n slot: ");
            if (LACP_GetLine( line_buffer, MAXLINE) > 0)
            {
                unit = atoi( line_buffer);
                BACKDOOR_MGR_Printf("\n port: ");
                if (LACP_GetLine( line_buffer, MAXLINE) > 0)
                {
                    port = atoi( line_buffer);
                    LACP_Show_OperPortParameters( unit, port);
                }
            }
            break;
        case '2':
            BACKDOOR_MGR_Printf("\n slot: ");
            if (LACP_GetLine( line_buffer, MAXLINE) > 0)
            {
                unit = atoi( line_buffer);
                BACKDOOR_MGR_Printf("\n port: ");
                if (LACP_GetLine( line_buffer, MAXLINE) > 0)
                {
                    port = atoi(line_buffer);
                    LACP_Show_AdminPortParameters( unit, port);
                }
            }
            break;
        case '3':
            BACKDOOR_MGR_Printf("\n slot: ");
            if (LACP_GetLine( line_buffer, MAXLINE) > 0)
            {
                unit = atoi( line_buffer);
                BACKDOOR_MGR_Printf("\n port: ");
                if (LACP_GetLine( line_buffer, MAXLINE) > 0)
                {
                    port = atoi( line_buffer);
                    LACP_Show_PortStatistics( unit, port);
                }
            }
            break;
        case '4':
            BACKDOOR_MGR_Printf("\n slot: ");
            if (LACP_GetLine( line_buffer, MAXLINE) > 0)
            {
                unit = atoi( line_buffer);
                BACKDOOR_MGR_Printf("\n port: ");
                if (LACP_GetLine( line_buffer, MAXLINE) > 0)
                {
                    port = atoi( line_buffer);
                    LACP_Show_PortProtocolState( unit, port);
                }
            }
            break;
        case '5':
            BACKDOOR_MGR_Printf("\n slot: ");
            if (LACP_GetLine( line_buffer, MAXLINE) > 0)
            {
                unit = atoi( line_buffer);
                BACKDOOR_MGR_Printf("\n port: ");
                if (LACP_GetLine( line_buffer, MAXLINE) > 0)
                {
                    port = atoi( line_buffer);
                    LACP_Show_Information( unit, port);
                }
            }
            break;
        case '6':
            BACKDOOR_MGR_Printf("\nShow Pdu debug infomation on screen? [y:Yes]");
            ch = BACKDOOR_MGR_GetChar();
            /* Get execution permission from the thread group handler if necessary
             */
            L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
            if('y' == ch)
            {
                LACP_Enable_Debug_Pdu( TRUE);
            }
            else
            {
                LACP_Enable_Debug_Pdu( FALSE);
            }
            /* Release execution permission from the thread group handler if necessary
             */
            L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
            break;
        case '7':
            BACKDOOR_MGR_Printf("\nShow Detail LACP debug infomation on screen? [y:Yes]");
            ch = BACKDOOR_MGR_GetChar();
            /* Get execution permission from the thread group handler if necessary
             */
            L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
            if('y' == ch)
            {
                LACP_Enable_Debug_Message( TRUE);
            }
            else
            {
                LACP_Enable_Debug_Message( FALSE);
            }
            /* Release execution permission from the thread group handler if necessary
             */
            L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
            break;
        case '8':
            BACKDOOR_MGR_Printf("\nToggle trunk debug message? [y:Yes]");
            ch = BACKDOOR_MGR_GetChar();
            /* Get execution permission from the thread group handler if necessary
             */
            L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
            if('y' == ch)
            {
                LACP_Enable_Debug_TRK_MGR_Message( TRUE);
            }
            else
            {
                LACP_Enable_Debug_TRK_MGR_Message( FALSE);
            }
            /* Release execution permission from the thread group handler if necessary
             */
            L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
            break;
        case '9':
            BACKDOOR_MGR_Printf("\nToggle Try_To_Attach debug message? [y:Yes]");
            ch = BACKDOOR_MGR_GetChar();
            /* Get execution permission from the thread group handler if necessary
             */
            L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
            if('y' == ch)
            {
                LACP_Enable_Debug_Try_To_Attach_Message( TRUE);
            }
            else
            {
                LACP_Enable_Debug_Try_To_Attach_Message( FALSE);
            }
            /* Release execution permission from the thread group handler if necessary
             */
            L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
            break;
        case 'a':
            BACKDOOR_MGR_Printf("\nReset statistics of ALL PORTS? [y:Yes]");
            if('y' == BACKDOOR_MGR_GetChar())
            {
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                LACP_Reset_All_PortStatistics();
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
            }
            else
            {
                BACKDOOR_MGR_Printf("\n slot: ");
                if (LACP_GetLine(line_buffer, MAXLINE) > 0)
                {
                    unit = atoi(line_buffer);
                    BACKDOOR_MGR_Printf("\n port: ");
                    if (LACP_GetLine(line_buffer, MAXLINE) > 0)
                    {
                        port = atoi(line_buffer);
                        /* Get execution permission from the thread group handler if necessary
                         */
                        L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                        LACP_Reset_PortStatistics( unit, port);
                        /* Release execution permission from the thread group handler if necessary
                         */
                        L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
                    }
                }
            }
            break;
        case 'b':
            BACKDOOR_MGR_Printf("\n slot: ");
            if (LACP_GetLine( line_buffer, MAXLINE) > 0)
            {
                unit = atoi( line_buffer);
                BACKDOOR_MGR_Printf("\n port: ");
                if (LACP_GetLine( line_buffer, MAXLINE) > 0)
                {
                    port = atoi( line_buffer);
                    LACP_Show_All_Ports_In_Agg( unit, port);
                }
            }
            break;
        case 'c':
            BACKDOOR_MGR_Printf("\n");
            /* Get execution permission from the thread group handler if necessary
             */
            L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
            if(TRUE == LACP_MGR_GetDot3adLagMibObjects( &last_changed))
            {
                LACP_Print_TablesLastChanged( &last_changed);
            }
            else
            {
                BACKDOOR_MGR_Printf("\nFail to get MIB objects...");
            }
            /* Release execution permission from the thread group handler if necessary
             */
            L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
            break;
        case 'd':
            BACKDOOR_MGR_Printf("\nToggle Trace Route debug message? [y:Yes]");
            ch = BACKDOOR_MGR_GetChar();
            /* Get execution permission from the thread group handler if necessary
             */
            L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
            if('y' == ch)
            {
                LACP_Enable_Debug_Trace_Route_Message( TRUE);
            }
            else
            {
                LACP_Enable_Debug_Trace_Route_Message( FALSE);
            }
            /* Release execution permission from the thread group handler if necessary
             */
            L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
            break;

        default:
        case 'x':
            L_THREADGRP_Leave(tg_handle, backdoor_member_id);
            return;
        }

    }

#if 0

    printf("\r\nshow received LACP packets on screen? ");
    if (getchar()=='y')
        LACP_Enable_Debug_Pdu(TRUE);
    else
    LACP_Enable_Debug_Pdu(FALSE);

    printf("\r\nshow detailed debug info (include callback) on screen? ");
    if (getchar()=='y')
        LACP_Enable_Debug_Message(TRUE);
    else
    LACP_Enable_Debug_Message(FALSE);

    while (ch == 'y')
    {
        printf("\r\nunit: ");
        if (LACP_GetLine(line_buffer, MAXLINE) > 0)
        {
            unit = atoi(line_buffer);
            printf("\r\nport:");
            if (LACP_GetLine(line_buffer, MAXLINE) > 0)
            {
                port = atoi(line_buffer);
                LACP_Show_Information(unit, port);
            }
        }
        printf("\r\n continue(y/n)?");
        ch = getchar();
    }
#endif
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adLacpPortEnabled
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set LACP on port to be enable or disable.
 * INPUT    :   UI32_T lacp_state         -- LACP_ADMIN_ON or LACP_ADMIN_OFF
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS           -- if set successfully
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adLacpPortEnabled( UI32_T ifindex, UI32_T lacp_state)
{
    UI32_T retval;

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return LACP_RETURN_ERROR;

    if(LACP_ADMIN_ON == lacp_state)
    {
        SYSCTRL_XOR_MGR_GetSemaphore();

        /* The ifindex can be joined to trunk. */
        if (TRUE == SYSCTRL_XOR_MGR_PermitBeingJoinedToTrunk(ifindex))
        {
            retval = LACP_MGR_SetDot3adLacpPortEnabled_(ifindex, lacp_state);
        }
        else /* The ifindex can not be joined to trunk. */
        {
            retval = LACP_RETURN_ERROR;
        }

        SYSCTRL_XOR_MGR_ReleaseSemaphore();
        return retval;
    }

#endif

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return LACP_RETURN_ERROR;

    retval=LACP_MGR_SetDot3adLacpPortEnabled_(ifindex, lacp_state);

    return retval;
} /* STA_MGR_SetDot1dStpPriority */

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_MGR_SetLACPState
 *------------------------------------------------------------------------
 * FUNCTION: This function Enable/Disable LACP on port
 * INPUT   : unit: unit number
 *           port: port number
 * OUTPUT  : None
 * RETURN  : LACP_RETURN_VALUE : LACP_RETURN_SUCCESS -- SUCCESS
 *                               else         -- ERROR
 * NOTE    : None
 *------------------------------------------------------------------------*/
static LACP_RETURN_VALUE LACP_MGR_SetLACPState( UI32_T unit, UI32_T port, UI32_T lacp_state)
{
    LACP_PORT_T *pPort;
    UI32_T       ifIndex;
    UI32_T       TrkId;

    SWCTRL_Lport_Type_T l_type = SWCTRL_LPORT_UNKNOWN_PORT;

    if (lacp_state != LACP_ADMIN_ON && lacp_state != LACP_ADMIN_OFF)
        return LACP_RETURN_INVALID_VALUE;

    if(STKTPLG_POM_PortExist( unit, port) == FALSE)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_SetLACPState_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Input port");
        return LACP_RETURN_PORT_NOT_EXIST;
    }

    /* l_type = SWCTRL_UserPortToLogicalPort(unit, port, &ifIndex); */
    /* Changed due to API changed. */
    l_type = SWCTRL_UserPortToIfindex(unit, port, &ifIndex);

    if( (SWCTRL_LPORT_UNKNOWN_PORT == l_type) ||
        (SWCTRL_LPORT_TRUNK_PORT == l_type) )
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_SetLACPState_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Input port");
        return LACP_RETURN_PORT_NOT_EXIST;
    }

    if(SWCTRL_LPORT_TRUNK_PORT_MEMBER == SWCTRL_UserPortToTrunkPort( unit, port, &TrkId) && lacp_state == LACP_ADMIN_ON)
    {
        /* lacp enable on a static member-> remove this port from static trunk */
        if(FALSE == TRK_PMGR_IsDynamicTrunkId( TrkId))
        {
#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
            /*  Do nothing */
#else
            UI32_T  port_ifindex;
            /* if (TRK_PMGR_DeleteTrunkMember( TrkId, unit, port)) */
            if (SWCTRL_UserPortToIfindex(unit, port, &port_ifindex) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                /* Allen Cheng: Deleted, 11/29/2002, due to TRK_PMGR_AddDynamicTrunkMember spec changed
                if (TRK_PMGR_DeleteDynamicTrunkMember( TrkId, unit, port))
                */
                if (TRK_PMGR_DeleteDynamicTrunkMember( TrkId, port_ifindex))
                {
                    if(bLACP_DbgAthAL)
                    {
                        BACKDOOR_MGR_Printf("\nLACP_MGR_SetLACPState Delete Trunk id:%d member slot:%d port:%d", (int)TrkId, (int)unit, (int)port);
                    }

                    if (TRK_PMGR_GetTrunkMemberCounts(TrkId)==0)
                    {
                        /* TRK_PMGR_DestroyTrunk(TrkId); */ TRK_PMGR_FreeTrunkIdDestroyDynamic( TrkId);
                        if(bLACP_DbgAthAL)
                        {
                            BACKDOOR_MGR_Printf("\nLACP_MGR_SetLACPState Delete Trunk id:%d", (int)TrkId);
                        }

                    }
                }
                else
                {
                    return LACP_RETURN_CANNOT_DELETE_TRUNK_MEMBER;
                }
            }
            else
            {
                if(bLACP_DbgAthAL)
                {
                    BACKDOOR_MGR_Printf("\nLACP_MGR_SetLACPState :: Fail to Delete unknown Trunk member slot:%d port:%d ifindex:%d", (int)unit, (int)port, (int)port_ifindex);
                }
            } /* End of if (SWCTRL_UserPortToIfindex) */
#endif /* SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE */
        }
    }
    pPort = LACP_FIND_PORT(unit, port);

    if((pPort->LACPEnabled &&  lacp_state==LACP_ADMIN_ON) || (!(pPort->LACPEnabled) &&  lacp_state==LACP_ADMIN_OFF))
    {
        return LACP_RETURN_SUCCESS;
    }

    if (lacp_state==LACP_ADMIN_ON)
        pPort->LACPEnabled = TRUE;
    else
        pPort->LACPEnabled = FALSE;

    LACP_UTIL_RefreshOperStatusAndAdminKey(pPort);

    /* What should we do here?                               */
    /* In fact, either LACP or physical port is disabled, we */
    /* should just trigger the related event to let LACP     */
    /* finite state machine to do the related job.           */

    /*if(lacp_state == LACP_ADMIN_ON)*/
    if (pPort->LACP_OperStatus)
    {
#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
        LACP_MGR_SetAdminStatus(ifIndex, pPort->LACPEnabled);
        LACP_MGR_SetOperStatus(ifIndex, pPort->LACP_OperStatus);
#else
        SWCTRL_PMGR_SetPortLacpEnable( ifIndex, LACP_ADMIN_ON);
#endif /* SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE */

        LACP_Tx_Machine      ( pPort, LACP_PORT_ENABLED_EV);
        LACP_Periodic_Machine( pPort, LACP_PORT_ENABLED_EV);
        LACP_Rx_Machine      ( pPort, LACP_PORT_ENABLED_EV, NULL);
        LACP_Mux_Machine     ( pPort, LACP_NEW_INFO_EV);
        LACP_Mux_Machine     ( pPort, LACP_PORT_ENABLED_EV);
#if (SYS_CPNT_LACP_STATIC_JOIN_TRUNK == TRUE)
        LACP_UTIL_CheckPortKeyIsMatchAggKeyAndAddStaticTrunkMember(pPort, pPort->AggActorAdminPort.key);
#endif
    }
    else
    {
        LACP_Tx_Machine      ( pPort, LACP_PORT_DISABLED_EV);
        LACP_Periodic_Machine( pPort, LACP_PORT_DISABLED_EV);
        LACP_Rx_Machine      ( pPort, LACP_PORT_DISABLED_EV, NULL);
        LACP_Mux_Machine     ( pPort, LACP_NEW_INFO_EV);
        LACP_Mux_Machine     ( pPort, LACP_PORT_DISABLED_EV);
#if (SYS_CPNT_LACP_STATIC_JOIN_TRUNK == TRUE)
        LACP_UTIL_CheckPortKeyIsMatchAggKeyAndDeleteStaticTrunkMember(pPort, pPort->AggActorAdminPort.key);
#endif

#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
        LACP_MGR_SetAdminStatus(ifIndex, pPort->LACPEnabled);
        LACP_MGR_SetOperStatus(ifIndex, pPort->LACP_OperStatus);
#else
            SWCTRL_PMGR_SetPortLacpEnable( ifIndex, LACP_ADMIN_OFF);
#endif /* SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE */
    }

    return LACP_RETURN_SUCCESS;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_MGR_GetLACPState
 *------------------------------------------------------------------------
 * FUNCTION: This function get LACP state on port
 * INPUT   : slot    : slot number
 *           port    : port number
 *           pEnable :
 * OUTPUT  : None
 * RETURN  : LACP_RETURN_VALUE : LACP_RETURN_SUCCESS -- SUCCESS
 *                               else         -- ERROR
 * NOTE    : None
 *------------------------------------------------------------------------*/
static LACP_RETURN_VALUE LACP_MGR_GetLACPState( UI32_T slot, UI32_T port, UI32_T *lacp_state)
{
    LACP_PORT_T *pPort;

    if(STKTPLG_POM_PortExist( slot, port) == FALSE)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetLACPState_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Input port");
        return LACP_RETURN_PORT_NOT_EXIST;
    }

    pPort = LACP_FIND_PORT( slot, port);

    if (pPort->LACPEnabled)
        *lacp_state = LACP_ADMIN_ON;
    else
        *lacp_state = LACP_ADMIN_OFF;

    return LACP_RETURN_SUCCESS;
}

static  UI32_T LACP_MGR_SetDot3adLacpPortEnabled_( UI32_T ifindex, UI32_T lacp_state)
{
    UI32_T  slot = 0, port = 0;
    UI32_T  trunk_id = 0;
    SYS_TYPE_Uport_T unit_port;

    SWCTRL_Lport_Type_T l_type = SWCTRL_LPORT_UNKNOWN_PORT;

    l_type = SWCTRL_LogicalPortToUserPort( ifindex, &slot, &port, &trunk_id);

    DEBUG_CONFIG("%sSet port state of slot:%lu port:%lu to %s\r\n\r\n", DEBUG_HEADER(),
        (unsigned long)slot, (unsigned long)port, (lacp_state == LACP_ADMIN_ON) ? "enabled" : "disabled");

    if( (l_type == SWCTRL_LPORT_UNKNOWN_PORT) ||
        (l_type == SWCTRL_LPORT_TRUNK_PORT) )
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_SetDot3adLacpPortEnabled__Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Input port");
        return LACP_RETURN_PORT_NOT_EXIST;
    }

    /* src (Monitored) and dest (Analyzer) port can't be trunk member */
    unit_port.unit = (UI16_T)slot;
    unit_port.port = (UI16_T)port;
    if (   SWCTRL_IsAnalyzerPort(unit_port)
        || SWCTRL_IsMonitoredPort(unit_port)
       )
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_SetDot3adLacpPortEnabled__Fun_No, EH_TYPE_MSG_FAILED_TO, SYSLOG_LEVEL_INFO, "set Lacp state because src or dest port can't be trunk member.");
        return LACP_RETURN_ERROR;
    }

    /* ckhsu 2002/03/14.                                   */
    /* Add a common check for the link aggregation ability */
    /* of the port.                                        */
    if(FALSE == SWCTRL_AllowToBeTrunkMember( slot, port))
    {
        /* This port can not be added to trunk port, hence */
        /* LACP also can not be enabled on this port.      */
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_SetDot3adLacpPortEnabled__Fun_No, EH_TYPE_MSG_FAILED_TO, SYSLOG_LEVEL_INFO, "set Lacp state because it can't be trunk member.");
        return LACP_RETURN_ERROR;
    }


#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT != TRUE)
    /* Check whether it is a static trunk member or not. */
    if(l_type == SWCTRL_LPORT_TRUNK_PORT_MEMBER)
    {
        if( (FALSE == TRK_PMGR_IsDynamicTrunkId( trunk_id)) &&
            (LACP_ADMIN_ON == lacp_state) )
        {
            /* This is a static trunk port. */
            EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_SetDot3adLacpPortEnabled__Fun_No, EH_TYPE_MSG_FAILED_TO, SYSLOG_LEVEL_INFO, "set Lacp state because it is a static trunk member.");
            return LACP_RETURN_ERROR;
        }
    }
#endif /* SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT != TRUE */

    if(LACP_RETURN_SUCCESS == LACP_MGR_SetLACPState( slot, port, lacp_state))
    {
        return LACP_RETURN_SUCCESS;
    }

    return LACP_RETURN_ERROR;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetNextRunningDot3adLacpPortEnabled
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the next port lacp information.
 * INPUT    :   UI32_T *lport_ptr   -- the pointer of the lport number
 * OUTPUT   :   UI32_T *bEnable     -- pointer of the value
 *                                     LACP_ADMIN_ON or LACP_ADMIN_OFF
 *
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- Successfully
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL    -- End of the port list
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 *              3. If the next available port is available, the *lport_ptr
 *                 will be updated.
 *-------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetNextRunningDot3adLacpPortEnabled(UI32_T *ifindex, UI32_T *lacp_state)
{
    UI32_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    retval=LACP_MGR_GetNextRunningDot3adLacpPortEnabled_(ifindex, lacp_state);

    return retval;
} /* End of STA_MGR_GetNextRunningDot1dStpPortPriority */

static  UI32_T LACP_MGR_GetNextRunningDot3adLacpPortEnabled_(UI32_T *ifindex, UI32_T *lacp_state)
{
    UI32_T next_if_Index = *ifindex;
    UI32_T slot, port, trunk_id;

    LACP_PORT_T *pNext;
    Port_Info_T  port_info;
    SWCTRL_Lport_Type_T l_type = SWCTRL_LPORT_UNKNOWN_PORT;

    /*   = LACP_GetFirst_AggregatorPort(); */
    if(*ifindex == 0)
    {
        pNext = LACP_GetFirst_AggregatorPort();
        next_if_Index = LACP_LINEAR_ID( pNext->slot, pNext->port);
    }
    else
    {
        SWCTRL_GetNextPortInfo( &next_if_Index, &port_info);
    }

    while(1)
    {
        if(next_if_Index > MAX_PHYSICAL_PORTS)
        {
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
        }

        l_type = SWCTRL_LogicalPortToUserPort( next_if_Index, &slot, &port, &trunk_id);

        if( (l_type == SWCTRL_LPORT_UNKNOWN_PORT) ||
            (l_type == SWCTRL_LPORT_TRUNK_PORT) )
        {
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
        }

        pNext = LACP_FIND_PORT( slot, port);

        if((!(pNext->LACPEnabled) && LACP_DEFAULT_PORT_ADMIN ==LACP_ADMIN_ON) || (pNext->LACPEnabled && LACP_DEFAULT_PORT_ADMIN ==LACP_ADMIN_OFF))
        {
            *lacp_state = pNext->LACPEnabled? LACP_ADMIN_ON: LACP_ADMIN_OFF;

            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }

        SWCTRL_GetNextPortInfo( &next_if_Index, &port_info);
    }

    return SYS_TYPE_GET_RUNNING_CFG_FAIL;
}

/* The following are private MIB. */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adLacpPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   lacp_port_entry -- port entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_MGR_GetDot3adLacpPortEntry(LACP_MGR_Dot3adLacpPortEntry_T *lacp_port_entry)
{
    BOOL_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    retval=LACP_MGR_GetDot3adLacpPortEntry_(lacp_port_entry);

    return retval;
} /* LACP_MGR_GetDot3adLacpPortEntry */

BOOL_T LACP_MGR_GetNextDot3adLacpPortEntry(LACP_MGR_Dot3adLacpPortEntry_T *lacp_port_entry)
{
    BOOL_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    retval=LACP_MGR_GetNextDot3adLacpPortEntry_(lacp_port_entry);

    return retval;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetNextDot3adLacpPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next lacp port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   lacp_port_entry -- port entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
static BOOL_T LACP_MGR_GetNextDot3adLacpPortEntry_(LACP_MGR_Dot3adLacpPortEntry_T *lacp_port_entry)
{
    UI32_T ifIndex = lacp_port_entry->dot3ad_lacp_port_index;

    ifIndex = LACP_GetNext_Appropriate_Physical_ifIndex_From_ifIndex( ifIndex);

    if(!ifIndex)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetNextDot3adLacpPortEntry__Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Input IfIndex");
        return FALSE;
    }
    else
    {
        lacp_port_entry->dot3ad_lacp_port_index = ifIndex;
    }

    return LACP_MGR_GetDot3adLacpPortEntry_( lacp_port_entry);

}


static  BOOL_T  LACP_MGR_GetDot3adLacpPortEntry_(LACP_MGR_Dot3adLacpPortEntry_T *lacp_port_entry)
{
    UI32_T ifIndex = lacp_port_entry->dot3ad_lacp_port_index;
    UI32_T slot, port, trunk_id, lacp_state;

    SWCTRL_Lport_Type_T lport_type = SWCTRL_LPORT_UNKNOWN_PORT;

    lport_type = SWCTRL_LogicalPortToUserPort( ifIndex, &slot, &port, &trunk_id);

    if( (SWCTRL_LPORT_UNKNOWN_PORT == lport_type) ||
        (SWCTRL_LPORT_TRUNK_PORT == lport_type) )
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adLacpPortEntry__Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Input IfIndex");
        return FALSE;
    }

    if(LACP_RETURN_SUCCESS != LACP_MGR_GetLACPState( slot, port, &lacp_state))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_LACP, LACP_MGR_GetDot3adLacpPortEntry__Fun_No, EH_TYPE_MSG_FAILED_TO_GET, SYSLOG_LEVEL_INFO, "lacp state");
        return FALSE;
    }

    if(lacp_state == LACP_ADMIN_ON)
    {
        lacp_port_entry->dot3ad_lacp_port_status = VAL_lacpPortStatus_enabled;
    }
    else
    {
        lacp_port_entry->dot3ad_lacp_port_status = VAL_lacpPortStatus_disabled;
    }

    return TRUE;
}



/* AmyTu Modified 06-29-2002 for callback design change

 */

void    LACP_MGR_PortDuplexChange_CallBack(UI32_T unit, UI32_T port, UI32_T speed_duplex)
{
    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return;

/* Allen Cheng, Deleted
    return (    (void)LACP_MGR_Function(   (LACP_MGR_SvcFunc_T) LACP_MGR_PortDuplexChange_Service,
                                           (UI32_T)             unit,
                                           (UI32_T)             port,
                                           (UI32_T)             speed_duplex,
                                           (UI32_T)             none)
           );
*/
    LACP_OM_EnterCriticalRegion();
    LACP_MGR_PortDuplexChange_Service(unit, port, speed_duplex);
    LACP_OM_LeaveCriticalRegion();

    return;
} /* end of LACP_MGR_PortDuplexChange_CallBack() */


void    LACP_MGR_PortLinkup_CallBack(UI32_T unit, UI32_T port)
{
    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return;

/* Allen Cheng, Deleted
    return (    (void)LACP_MGR_Function(  (LACP_MGR_SvcFunc_T) LACP_MGR_PortLinkup_Service,
                                          (UI32_T)             unit,
                                          (UI32_T)             port,
                                          (UI32_T)             none,
                                          (UI32_T)             none)
           );
*/
    LACP_OM_EnterCriticalRegion();
    LACP_MGR_PortLinkup_Service(unit, port);
    LACP_OM_LeaveCriticalRegion();

    return;

} /* end of LACP_MGR_PortLinkup_CallBack() */


void    LACP_MGR_PortLinkdown_CallBack(UI32_T unit, UI32_T port)
{
    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return;

/* Allen Cheng, Deleted
    return (    (void)LACP_MGR_Function(  (LACP_MGR_SvcFunc_T) LACP_MGR_PortLinkdown_Service,
                                            (UI32_T)             unit,
                                            (UI32_T)             port,
                                            (UI32_T)             none,
                                            (UI32_T)             none)
           );
*/
    LACP_OM_EnterCriticalRegion();
    LACP_MGR_PortLinkdown_Service(unit, port);
    LACP_OM_LeaveCriticalRegion();

    return;
} /* end of LACP_MGR_PortLinkdown_CallBack() */


void    LACP_MGR_PortAdminEnable_CallBack(UI32_T unit, UI32_T port)
{
    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return;

/* Allen Cheng, Deleted
    return (    (void)LACP_MGR_Function(  (LACP_MGR_SvcFunc_T) LACP_MGR_PortAdminEnable_Service,
                                            (UI32_T)             unit,
                                            (UI32_T)             port,
                                            (UI32_T)             none,
                                            (UI32_T)             none)
           );
*/
    LACP_OM_EnterCriticalRegion();
    LACP_MGR_PortAdminEnable_Service(unit, port);
    LACP_OM_LeaveCriticalRegion();

    return;
} /* end of LACP_MGR_PortAdminEnable_CallBack() */

void    LACP_MGR_PortAdminDisable_CallBack(UI32_T unit, UI32_T port)
{
    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return;

/* Allen Cheng, Deleted
    return (    (void)LACP_MGR_Function(  (LACP_MGR_SvcFunc_T) LACP_MGR_PortAdminDisable_Service,
                                            (UI32_T)             unit,
                                            (UI32_T)             port,
                                            (UI32_T)             none,
                                            (UI32_T)             none)
           );
*/
    LACP_OM_EnterCriticalRegion();
    LACP_MGR_PortAdminDisable_Service(unit, port);
    LACP_OM_LeaveCriticalRegion();

    return;
} /* end of LACP_MGR_PortAdminDisable_CallBack() */


#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
void    LACP_MGR_AddStaticTrunkMember_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return;

    LACP_OM_EnterCriticalRegion();
    LACP_MGR_AddStaticTrunkMember_Service(trunk_ifindex, member_ifindex);
    LACP_OM_LeaveCriticalRegion();

    return;
} /* end of LACP_MGR_AddStaticTrunkMember_CallBack() */

void    LACP_MGR_DelStaticTrunkMember_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return;

    LACP_OM_EnterCriticalRegion();
    LACP_MGR_DelStaticTrunkMember_Service(trunk_ifindex, member_ifindex);
    LACP_OM_LeaveCriticalRegion();

    return;
} /* end of LACP_MGR_DelStaticTrunkMember_CallBack() */
#endif /* SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE */


/* Lewis: the experiment shows linkup is callbacked first then duplex change */
static  void    LACP_MGR_PortDuplexChange_Service(UI32_T unit, UI32_T port, UI32_T speed_duplex)
{
    /* 2001/12/06 ckhsu                                                         */
    /* Kang give me an advice that we can use some indications to achieve this. */
    /* After discuss with him, I agree that he is right so I decide to do that  */
    /* here to resolve the problem.                                             */
    LACP_PORT_T *pPort          = NULL;
    BOOL_T       bFDx           = FALSE;
    UI32_T       speed          = 0;
    UI32_T       current_speed  = 0;
    BOOL_T       current_FDx    = FALSE;
#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
    UI32_T       if_index;
#endif /* SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE */

    if(STKTPLG_POM_PortExist( unit, port) == FALSE)
    {
        return;
    }

    if (bLACP_DbgMsg)
        BACKDOOR_MGR_Printf("\n\t------>Port DuplexModeChange Callback on unit:%d, port:%d", (int)unit, (int)port);

    DEBUG_EVENT("%sPort DuplexModeChange Callback on unit:%lu, port:%lu\r\n\r\n", DEBUG_HEADER(), (unsigned long)unit, (unsigned long)port);

    /*SWCTRL_UserPortToLogicalPort( unit, port, &ifIndex);*/

    /* Since this port exist, it should always get status */
    /*SWCTRL_GetPortInfo( ifIndex, &port_info);*/

    /* Now decide the speed and duplex. */
    switch(speed_duplex)
    {
    case VAL_portSpeedDpxStatus_fullDuplex10:
        bFDx = TRUE;
    case VAL_portSpeedDpxStatus_halfDuplex10:
        speed = 10;
        break;
    case VAL_portSpeedDpxStatus_fullDuplex100:
        bFDx = TRUE;
    case VAL_portSpeedDpxStatus_halfDuplex100:
        speed = 100;
        break;
    case VAL_portSpeedDpxStatus_fullDuplex1000:
        bFDx = TRUE;
    case VAL_portSpeedDpxStatus_halfDuplex1000:
        speed = 1000;
        break;
    case VAL_portSpeedDpxStatus_fullDuplex10g:
        bFDx = TRUE;
    case VAL_portSpeedDpxStatus_halfDuplex10g:
        speed = 10000;
        break;
    case VAL_portSpeedDpxStatus_fullDuplex40g:
        bFDx = TRUE;
    case VAL_portSpeedDpxStatus_halfDuplex40g:
        speed  = 40000;
        break;
    case VAL_portSpeedDpxStatus_fullDuplex100g:
        bFDx = TRUE;
    case VAL_portSpeedDpxStatus_halfDuplex100g:
        speed  = 100000;
        break;
#ifdef VAL_portSpeedDpxStatus_fullDuplex25g
    case VAL_portSpeedDpxStatus_fullDuplex25g:
        bFDx = TRUE;
        speed  = 25000;
        break;
#endif
    }

    if (bLACP_DbgMsg)
    {
        if (bFDx)
            BACKDOOR_MGR_Printf(" %dFULL Duplex\n", (int)speed);
        else
            BACKDOOR_MGR_Printf(" %dHALF Duplex\n", (int)speed);
    }

    pPort = LACP_FIND_PORT( unit, port);

    /* Judge what we should do here */
    current_FDx   = pPort->FullDuplexMode;
    current_speed = pPort->PortSpeed;

    pPort->FullDuplexMode = bFDx;
    pPort->PortSpeed      = speed;

    LACP_UTIL_RefreshOperStatusAndAdminKey(pPort);
#if 0   /* Redesigned */
    if( (pPort->LACPEnabled) &&
        (pPort->FullDuplexMode) )
    {
        pPort->LACP_OperStatus = TRUE;

        if (pPort->static_admin_key)
        {
            if (bLACP_DbgMsg)
                BACKDOOR_MGR_Printf("\r\n (unit:%d, port:%d) -- static_admin_key is set", unit, port);
        }
        else
        {
            if (pPort->PortSpeed == 10)
                (pPort->AggActorAdminPort).key = SYS_DFLT_LACP_KEY_10FULL;
            else if (pPort->PortSpeed == 100)
               (pPort->AggActorAdminPort).key = SYS_DFLT_LACP_KEY_100FULL;
            else if (pPort->PortSpeed == 1000)
               (pPort->AggActorAdminPort).key = SYS_DFLT_LACP_KEY_1000FULL;
            else if (pPort->PortSpeed == 10000)
               (pPort->AggActorAdminPort).key = SYS_DFLT_LACP_KEY_10GFULL;
            else
                (pPort->AggActorAdminPort).key = SYS_DFLT_LACP_KEY_DEFAULT;
        }
    }
    else
    {
        pPort->LACP_OperStatus = FALSE;
    }
#endif

#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
    if (SWCTRL_UserPortToIfindex(unit, port, &if_index) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        LACP_MGR_SetOperStatus(if_index, pPort->LACP_OperStatus);
    }
#endif /* SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE */

    if(bLACP_DbgMsg)
    {
        BACKDOOR_MGR_Printf("\nLACP_MGR_PortDuplexChange_Service Slot:%d Port:%d", (int)pPort->slot, (int)pPort->port);
        BACKDOOR_MGR_Printf("\nLACP_OperStatus :%s ", ((pPort->LACP_OperStatus)?("True"):("False")));
        BACKDOOR_MGR_Printf("\nPort_OperStatus :%s ", ((pPort->Port_OperStatus)?("True"):("False")));
        BACKDOOR_MGR_Printf("\nPortEnabled     :%s ", ((pPort->PortEnabled)?("True"):("False")));
        BACKDOOR_MGR_Printf("\nLinkUp          :%s ", ((pPort->LinkUp)?("Up"):("Down")));
        BACKDOOR_MGR_Printf("\nLACPEnabled     :%s ", ((pPort->LACPEnabled)?("True"):("False")));
        BACKDOOR_MGR_Printf("\nFullDuplexMode  :%s ", ((pPort->FullDuplexMode)?("Full"):("Half")));
        BACKDOOR_MGR_Printf("\nPortSpeed       :%d ", (int)pPort->PortSpeed);
    }

    {
        DEBUG_PageBuffer[0]=0;
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tLACP_OperStatus :%s ", ((pPort->LACP_OperStatus)?("True"):("False")));
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tPort_OperStatus :%s ", ((pPort->Port_OperStatus)?("True"):("False")));
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tPortEnabled     :%s ", ((pPort->PortEnabled)?("True"):("False")));
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tLinkUp          :%s ", ((pPort->LinkUp)?("Up"):("Down")));
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tLACPEnabled     :%s ", ((pPort->LACPEnabled)?("True"):("False")));
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tFullDuplexMode  :%s ", ((pPort->FullDuplexMode)?("Full"):("Half")));
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tPortSpeed       :%d ", (int)pPort->PortSpeed);
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n");
        DEBUG_DATABASE("%sPort duplex changed on slot:%lu port:%lu,%s", DEBUG_HEADER(), (unsigned long)pPort->slot, (unsigned long)pPort->port, DEBUG_PageBuffer);
    }

    /* Inform state machine. */
    /* Once the speed or duplex is changed, we have to rerun the whole state machine. */
    /* It means we reinitialize it.                                                   */
    if(current_speed != pPort->PortSpeed)
    {
        /* It is speed change, we only need to reinitialize the machine. */
        LACP_Tx_Machine      ( pPort, LACP_SYS_INIT);
        LACP_Periodic_Machine( pPort, LACP_SYS_INIT);
        LACP_Rx_Machine      ( pPort, LACP_SYS_INIT, NULL);
        LACP_Mux_Machine     ( pPort, LACP_NEW_INFO_EV);
        LACP_Mux_Machine     ( pPort, LACP_SYS_INIT);
    }

 /*   if(current_FDx != pPort->FullDuplexMode) */ /*Lewis: if default is full, and callback with full -> still need to do the following */
    {
        /* Duplex mode is changed, we have to do something as LACP Enable/Disable */
        /* if (bFDx) */
        if (pPort->LACP_OperStatus)
        {
/*             if (pPort->LACPEnabled) */
/*            SWCTRL_PMGR_SetPortLacpEnable( LACP_LINEAR_ID(unit,port), LACP_ADMIN_ON); */
            LACP_Tx_Machine      ( pPort, LACP_PORT_ENABLED_EV);
            LACP_Periodic_Machine( pPort, LACP_PORT_ENABLED_EV);
            LACP_Rx_Machine      ( pPort, LACP_PORT_ENABLED_EV, NULL);
            LACP_Mux_Machine     ( pPort, LACP_NEW_INFO_EV);
            LACP_Mux_Machine     ( pPort, LACP_PORT_ENABLED_EV);
        }
        else
        {
            LACP_Tx_Machine      ( pPort, LACP_PORT_DISABLED_EV);
            LACP_Periodic_Machine( pPort, LACP_PORT_DISABLED_EV);
            LACP_Rx_Machine      ( pPort, LACP_PORT_DISABLED_EV, NULL);
            LACP_Mux_Machine     ( pPort, LACP_NEW_INFO_EV);
            LACP_Mux_Machine     ( pPort, LACP_PORT_DISABLED_EV);

/*            SWCTRL_PMGR_SetPortLacpEnable( LACP_LINEAR_ID(unit,port), LACP_ADMIN_OFF); */
        }
    }

    if (bLACP_DbgMsg)
    {
        BACKDOOR_MGR_Printf("\nLACP finishes LACP_MGR_PortDuplexChange_Service");
    }
} /* end of LACP_MGR_PortDuplexChange_Service() */


static  void    LACP_MGR_PortLinkup_Service(UI32_T unit, UI32_T port)
{
    LACP_PORT_T *pPort;

    if(STKTPLG_POM_PortExist( unit, port) == FALSE)
    {
        return;
    }

    if (bLACP_DbgMsg)
        BACKDOOR_MGR_Printf("\n\t====>LinkUp Callback on unit:%d, port:%d", (int)unit, (int)port);

    DEBUG_EVENT("%sPort link up on unit %lu port %lu\r\n\r\n", DEBUG_HEADER(), (unsigned long)unit, (unsigned long)port);

    pPort = LACP_FIND_PORT( unit, port);

    if (pPort == NULL)
    {
        if (bLACP_DbgMsg)
            BACKDOOR_MGR_Printf("\npPort is NULL ");
    }

    pPort->LinkUp = TRUE;

    LACP_UTIL_RefreshOperStatusAndAdminKey(pPort);
#if 0   /* Redesigned */
    if( (pPort->LinkUp) &&
        (pPort->PortEnabled) )
    {
        pPort->Port_OperStatus = TRUE;
    }
    else
    {
        pPort->Port_OperStatus = FALSE;
    }

    if (pPort->static_admin_key)
    {
        if (bLACP_DbgMsg)
            BACKDOOR_MGR_Printf("\r\n (unit:%d, port:%d) -- static_admin_key is set", unit, port);
    }
    else
    {
        if ((pPort->PortSpeed == 10) && (pPort->FullDuplexMode == TRUE))
            (pPort->AggActorAdminPort).key = SYS_DFLT_LACP_KEY_10FULL;
        else if ((pPort->PortSpeed == 100) && (pPort->FullDuplexMode == TRUE))
           (pPort->AggActorAdminPort).key = SYS_DFLT_LACP_KEY_100FULL;
        else if ((pPort->PortSpeed == 1000) && (pPort->FullDuplexMode == TRUE))
           (pPort->AggActorAdminPort).key = SYS_DFLT_LACP_KEY_1000FULL;
        else if ((pPort->PortSpeed == 10000) && (pPort->FullDuplexMode == TRUE))
           (pPort->AggActorAdminPort).key = SYS_DFLT_LACP_KEY_10GFULL;
        else
            (pPort->AggActorAdminPort).key = SYS_DFLT_LACP_KEY_DEFAULT;
    }
#endif

    if(bLACP_DbgMsg)
    {
        BACKDOOR_MGR_Printf("\nLACP_MGR_PortLinkup_Service Slot:%d Port:%d", (int)pPort->slot, (int)pPort->port);
        BACKDOOR_MGR_Printf("\nLACP_OperStatus :%s ", ((pPort->LACP_OperStatus)?("True"):("False")));
        BACKDOOR_MGR_Printf("\nPort_OperStatus :%s ", ((pPort->Port_OperStatus)?("True"):("False")));
        BACKDOOR_MGR_Printf("\nPortEnabled     :%s ", ((pPort->PortEnabled)?("True"):("False")));
        BACKDOOR_MGR_Printf("\nLinkUp          :%s ", ((pPort->LinkUp)?("Up"):("Down")));
        BACKDOOR_MGR_Printf("\nLACPEnabled     :%s ", ((pPort->LACPEnabled)?("True"):("False")));
        BACKDOOR_MGR_Printf("\nFullDuplexMode  :%s ", ((pPort->FullDuplexMode)?("Full"):("Half")));
        BACKDOOR_MGR_Printf("\nPortSpeed       :%d ", (int)pPort->PortSpeed);
    }

    {
        DEBUG_PageBuffer[0]=0;
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tLACP_OperStatus :%s ", ((pPort->LACP_OperStatus)?("True"):("False")));
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tPort_OperStatus :%s ", ((pPort->Port_OperStatus)?("True"):("False")));
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tPortEnabled     :%s ", ((pPort->PortEnabled)?("True"):("False")));
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tLinkUp          :%s ", ((pPort->LinkUp)?("Up"):("Down")));
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tLACPEnabled     :%s ", ((pPort->LACPEnabled)?("True"):("False")));
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tFullDuplexMode  :%s ", ((pPort->FullDuplexMode)?("Full"):("Half")));
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tPortSpeed       :%d ", (int)pPort->PortSpeed);
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n");
        DEBUG_DATABASE("%sPort link up on slot:%lu port:%lu,%s", DEBUG_HEADER(), (unsigned long)pPort->slot, (unsigned long)pPort->port, DEBUG_PageBuffer);
    }

    if (pPort->LACP_OperStatus)
    {
        LACP_Tx_Machine      ( pPort, LACP_RX_PORT_UP_EV);
        LACP_Periodic_Machine( pPort, LACP_RX_PORT_UP_EV);
        LACP_Rx_Machine      ( pPort, LACP_RX_PORT_UP_EV, NULL);
        LACP_Mux_Machine     ( pPort, LACP_NEW_INFO_EV);
        LACP_Mux_Machine     ( pPort, LACP_RX_PORT_UP_EV);
    }
    if (bLACP_DbgMsg)
    {
        BACKDOOR_MGR_Printf("\nLACP finishes LACP_MGR_PortLinkup_Service");
    }
} /* end of LACP_MGR_PortLinkup_Service () */


static  void    LACP_MGR_PortLinkdown_Service(UI32_T unit, UI32_T port)
{
    LACP_PORT_T *pPort;

    if(STKTPLG_POM_PortExist( unit, port) == FALSE)
    {
        return;
    }

    if (bLACP_DbgMsg)
        BACKDOOR_MGR_Printf("\n\t------>LinkDown Callback on unit:%d, port:%d", (int)unit, (int)port);

    DEBUG_EVENT("%sPort link down on unit %lu port %lu\r\n\r\n", DEBUG_HEADER(), (unsigned long)unit, (unsigned long)port);

    pPort = LACP_FIND_PORT( unit, port);

    pPort->LinkUp = FALSE;

    LACP_UTIL_RefreshOperStatusAndAdminKey(pPort);

    if(bLACP_DbgMsg)
    {
        BACKDOOR_MGR_Printf("\nLACP_MGR_PortLinkdown_Service Slot:%d Port:%d", (int)pPort->slot, (int)pPort->port);
        BACKDOOR_MGR_Printf("\nLACP_OperStatus :%s ", ((pPort->LACP_OperStatus)?("True"):("False")));
        BACKDOOR_MGR_Printf("\nPort_OperStatus :%s ", ((pPort->Port_OperStatus)?("True"):("False")));
        BACKDOOR_MGR_Printf("\nPortEnabled     :%s ", ((pPort->PortEnabled)?("True"):("False")));
        BACKDOOR_MGR_Printf("\nLinkUp          :%s ", ((pPort->LinkUp)?("Up"):("Down")));
        BACKDOOR_MGR_Printf("\nLACPEnabled     :%s ", ((pPort->LACPEnabled)?("True"):("False")));
        BACKDOOR_MGR_Printf("\nFullDuplexMode  :%s ", ((pPort->FullDuplexMode)?("Full"):("Half")));
        BACKDOOR_MGR_Printf("\nPortSpeed       :%d ", (int)pPort->PortSpeed);
    }

    {
        DEBUG_PageBuffer[0]=0;
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tLACP_OperStatus :%s ", ((pPort->LACP_OperStatus)?("True"):("False")));
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tPort_OperStatus :%s ", ((pPort->Port_OperStatus)?("True"):("False")));
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tPortEnabled     :%s ", ((pPort->PortEnabled)?("True"):("False")));
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tLinkUp          :%s ", ((pPort->LinkUp)?("Up"):("Down")));
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tLACPEnabled     :%s ", ((pPort->LACPEnabled)?("True"):("False")));
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tFullDuplexMode  :%s ", ((pPort->FullDuplexMode)?("Full"):("Half")));
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n\tPortSpeed       :%d ", (int)pPort->PortSpeed);
        DEBUG_STRCAT(DEBUG_PageBuffer,"\r\n");
        DEBUG_DATABASE("%sPort link down on slot:%lu port:%lu,%s", DEBUG_HEADER(), (unsigned long)pPort->slot, (unsigned long)pPort->port, DEBUG_PageBuffer);
    }

    LACP_Tx_Machine      ( pPort, LACP_RX_PORT_DOWN_EV);
    LACP_Periodic_Machine( pPort, LACP_RX_PORT_DOWN_EV);
    LACP_Rx_Machine      ( pPort, LACP_RX_PORT_DOWN_EV, NULL);
    LACP_Mux_Machine     ( pPort, LACP_NEW_INFO_EV);
    LACP_Mux_Machine     ( pPort, LACP_RX_PORT_DOWN_EV);

    if (bLACP_DbgMsg)
    {
        BACKDOOR_MGR_Printf("\nLACP finishes LACP_MGR_PortLinkdown_Service");
    }
} /* end of LACP_MGR_PortLinkdown_Service() */

static  void    LACP_MGR_PortAdminEnable_Service(UI32_T unit, UI32_T port)
{
    /* ckhsu, 2001/12/13                                                    */
    /* When you are here, it means we receive a message to enable the port. */
    LACP_PORT_T *pPort;

    if(STKTPLG_POM_PortExist( unit, port) == FALSE)
    {
        return;
    }

    pPort = LACP_FIND_PORT( unit, port);

    if(pPort->PortEnabled == TRUE)
    {
        return;
    }

    if (bLACP_DbgMsg)
        BACKDOOR_MGR_Printf("\n\t------>Port AdminEnable Callback on unit:%d, port:%d", (int)unit, (int)port);

    DEBUG_EVENT("%sPort admin_enable on unit %lu port %lu\r\n\r\n", DEBUG_HEADER(), (unsigned long)unit, (unsigned long)port);

    pPort->PortEnabled = TRUE;

    LACP_UTIL_RefreshOperStatusAndAdminKey(pPort);
#if 0   /* Redesigned */
    if( (pPort->LinkUp) &&
        (pPort->PortEnabled) )
    {
        pPort->Port_OperStatus = TRUE;
    }
    else
    {
        pPort->Port_OperStatus = FALSE;
    }
#endif

    if (pPort->LACP_OperStatus)
    {
        LACP_Tx_Machine      ( pPort, LACP_PORT_ENABLED_EV);
        LACP_Periodic_Machine( pPort, LACP_PORT_ENABLED_EV);
        LACP_Rx_Machine      ( pPort, LACP_PORT_ENABLED_EV, NULL);
        LACP_Mux_Machine     ( pPort, LACP_NEW_INFO_EV);
        LACP_Mux_Machine     ( pPort, LACP_PORT_ENABLED_EV);
    }
    if (bLACP_DbgMsg)
    {
        BACKDOOR_MGR_Printf("\nLACP finishes LACP_MGR_PortAdminEnable_Service");
    }
} /* end of LACP_MGR_PortAdminEnable_Service() */


static  void    LACP_MGR_PortAdminDisable_Service(UI32_T unit, UI32_T port)
{
    /* ckhsu, 2001/12/13                                                    */
    /* When you are here, it means we receive a message to disable the port. */
    LACP_PORT_T *pPort;

    if(STKTPLG_POM_PortExist( unit, port) == FALSE)
    {
        return;
    }

    pPort = LACP_FIND_PORT( unit, port);

    if(pPort->PortEnabled == FALSE)
    {
        return;
    }

    if (bLACP_DbgMsg)
        BACKDOOR_MGR_Printf("\n\t------>Port AdminDisable Callback on unit:%d, port:%d", (int)unit, (int)port);

    DEBUG_EVENT("%sPort admin_disable on unit %lu, port %lu\r\n\r\n", DEBUG_HEADER(), (unsigned long)unit, (unsigned long)port);

    pPort->PortEnabled = FALSE;

    LACP_UTIL_RefreshOperStatusAndAdminKey(pPort);
#if 0   /* Redesigned */
    if( (pPort->LinkUp) &&
        (pPort->PortEnabled))
    {
        pPort->Port_OperStatus = TRUE;
    }
    else
    {
        pPort->Port_OperStatus = FALSE;
    }
#endif

    LACP_Tx_Machine      ( pPort, LACP_PORT_DISABLED_EV);
    LACP_Periodic_Machine( pPort, LACP_PORT_DISABLED_EV);
    LACP_Rx_Machine      ( pPort, LACP_PORT_DISABLED_EV, NULL);
    LACP_Mux_Machine     ( pPort, LACP_NEW_INFO_EV);
    LACP_Mux_Machine     ( pPort, LACP_PORT_DISABLED_EV);

    if (bLACP_DbgMsg)
    {
        BACKDOOR_MGR_Printf("\nLACP finishes LACP_MGR_PortAdminDisable_Service");
    }
} /* end of LACP_MGR_PortAdminDisable_Service() */

#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
static  void    LACP_MGR_AddStaticTrunkMember_Service(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    LACP_PORT_T *pPort;
    UI32_T      unit, port, trunk_id, member_trunk_id;
    SWCTRL_Lport_Type_T port_type;

    port_type   = SWCTRL_LogicalPortToUserPort(member_ifindex, &unit, &port, &member_trunk_id);
    if (    (port_type == SWCTRL_LPORT_NORMAL_PORT)
        ||  (port_type == SWCTRL_LPORT_TRUNK_PORT_MEMBER)
       )
    {
        pPort = LACP_FIND_PORT(unit, port);
        /*if (pPort->LACPEnabled)*/
        {
            SWCTRL_Lport_Type_T trunk_port_type;
            trunk_port_type = SWCTRL_LogicalPortToUserPort(trunk_ifindex, &unit, &port, &trunk_id);
            if (trunk_port_type == SWCTRL_LPORT_TRUNK_PORT )
            {
/* gordon_kao: Callback will deplay until next dequeue the IPC message,
 *             so we do the essential action first then callback to lacp for
 *             lacp state machine
 */
#if 0
                if (TRK_PMGR_IsDynamicTrunkId(trunk_id) )
                {
                    TRK_MGR_TrunkEntry_T    trunk_entry;
                    trunk_entry.trunk_index = trunk_id;
                    /* This trunk is dynamic: delete it */
                    if (TRK_PMGR_GetTrunkEntry(&trunk_entry) )
                    {
                        UI32_T  dtm_ifindex;
                        for (dtm_ifindex = 1; dtm_ifindex <= SYS_ADPT_TOTAL_NBR_OF_LPORT; dtm_ifindex++)
                        {
                            BOOL_T  is_trunk_member;
                            is_trunk_member = (     (   trunk_entry.trunk_ports[(dtm_ifindex-1)/8]
                                                     &  (   (0x01) << (7 - ((dtm_ifindex - 1) % 8))
                                                        )
                                                    )
                                               !=   0
                                              );
                            if (is_trunk_member)
                            {
                                TRK_PMGR_DeleteDynamicTrunkMember(trunk_id, dtm_ifindex);
                            }
                        } /* End of for (_all_lport_) */
                        TRK_PMGR_FreeTrunkIdDestroyDynamic(trunk_id);
                    }
                } /* End of if (_dynamic_trunk_id_) */

                if (port_type == SWCTRL_LPORT_TRUNK_PORT_MEMBER)
                {
                    TRK_PMGR_DeleteDynamicTrunkMember(member_trunk_id, member_ifindex);
                } /* End of if (_trunk_member_) */
#endif
                if (pPort->LACP_OperStatus)
                {
                    pPort->LACP_OperStatus  = FALSE;

/* gordon_kao: Callback will deplay until next dequeue the IPC message,
 *             so we do the essential action first then callback to lacp for
 *             lacp state machine
 */
#if 0
                    LACP_MGR_SetOperStatus(member_ifindex, pPort->LACP_OperStatus);
#endif
                } /* End of if (_LACP_oper_enabled) */

                if (pPort->LACP_OperStatus)
                {
                    LACP_Tx_Machine      ( pPort, LACP_PORT_ENABLED_EV);
                    LACP_Periodic_Machine( pPort, LACP_PORT_ENABLED_EV);
                    LACP_Rx_Machine      ( pPort, LACP_PORT_ENABLED_EV, NULL);
                    LACP_Mux_Machine     ( pPort, LACP_NEW_INFO_EV);
                    LACP_Mux_Machine     ( pPort, LACP_PORT_ENABLED_EV);
                }
                else
                {
                    LACP_Tx_Machine      ( pPort, LACP_PORT_DISABLED_EV);
                    LACP_Periodic_Machine( pPort, LACP_PORT_DISABLED_EV);
                    LACP_Rx_Machine      ( pPort, LACP_PORT_DISABLED_EV, NULL);
                    LACP_Mux_Machine     ( pPort, LACP_NEW_INFO_EV);
                    LACP_Mux_Machine     ( pPort, LACP_PORT_DISABLED_EV);
                }
            } /* End of if (_trunk_port_type_) */
        } /* End of if (_LACP_enabled_) */

    }

    return;
} /* End of LACP_MGR_AddStaticTrunkMember_Service */

static  void    LACP_MGR_DelStaticTrunkMember_Service(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    LACP_PORT_T *pPort;
    UI32_T      unit, port, member_trunk_id;

    if ( SWCTRL_LogicalPortToUserPort(member_ifindex, &unit, &port, &member_trunk_id) != SWCTRL_LPORT_UNKNOWN_PORT )
    {
        pPort = LACP_FIND_PORT(unit, port);
        if (    (pPort->LACPEnabled)
            &&  (!pPort->LACP_OperStatus)
           )
        {
                pPort->LACP_OperStatus  = TRUE;
        }
    }
    else
        return;
/* gordon_kao: Callback will deplay until next dequeue the IPC message,
 *             so we do the essential action first then callback to lacp for
 *             lacp state machine
 */
#if 0
    LACP_MGR_SetOperStatus(member_ifindex, pPort->LACP_OperStatus);
#endif
    if (pPort->LACP_OperStatus)
    {
        LACP_Tx_Machine      ( pPort, LACP_PORT_ENABLED_EV);
        LACP_Periodic_Machine( pPort, LACP_PORT_ENABLED_EV);
        LACP_Rx_Machine      ( pPort, LACP_PORT_ENABLED_EV, NULL);
        LACP_Mux_Machine     ( pPort, LACP_NEW_INFO_EV);
        LACP_Mux_Machine     ( pPort, LACP_PORT_ENABLED_EV);
    }
    else
    {
        LACP_Tx_Machine      ( pPort, LACP_PORT_DISABLED_EV);
        LACP_Periodic_Machine( pPort, LACP_PORT_DISABLED_EV);
        LACP_Rx_Machine      ( pPort, LACP_PORT_DISABLED_EV, NULL);
        LACP_Mux_Machine     ( pPort, LACP_NEW_INFO_EV);
        LACP_Mux_Machine     ( pPort, LACP_PORT_DISABLED_EV);
    }

    return;
} /* End of LACP_MGR_DelStaticTrunkMember_Service */

static  BOOL_T  LACP_MGR_SetAdminStatus(UI32_T if_index, BOOL_T enable_state)
{
    BOOL_T  result;
    if (enable_state)
    {
        result  = SWCTRL_PMGR_SetPortLacpAdminEnable(if_index, VAL_lacpPortStatus_enabled);
    }
    else
    {
        result  = SWCTRL_PMGR_SetPortLacpAdminEnable(if_index, VAL_lacpPortStatus_disabled);
    }
    return  result;
} /* End of LACP_MGR_SetAdminStatus */

static  BOOL_T  LACP_MGR_SetOperStatus(UI32_T if_index, BOOL_T enable_state)
{
    BOOL_T  result;
    if (enable_state)
    {
        result  = SWCTRL_PMGR_SetPortLacpOperEnable(if_index, VAL_lacpPortStatus_enabled);
    }
    else
    {
        result  = SWCTRL_PMGR_SetPortLacpOperEnable(if_index, VAL_lacpPortStatus_disabled);
    }
    return  result;
} /* End of LACP_MGR_SetOperStatus */
#endif /* SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE */

/* ======================================================================== */
/* Functions for setting MIB entry */

/* Dot3adAggEntry */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggActorSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_actor_system_priority information.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 *              UI16_T  priority    -- the dot3ad_agg_actor_system_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggActorSystemPriority(UI16_T agg_index, UI16_T priority)
{
    UI32_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return LACP_RETURN_ERROR;

    retval=LACP_MGR_SetDot3adAggActorSystemPriority_(agg_index, priority);

    return  retval;
} /* End of LACP_MGR_SetDot3adAggActorSystemPriority */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_actor_admin_key information.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 *              UI16_T  admin_key   -- the dot3ad_agg_actor_admin_key value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggActorAdminKey(UI16_T agg_index, UI16_T admin_key)
{
    UI32_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return LACP_RETURN_ERROR;

    retval=LACP_MGR_SetDot3adAggActorAdminKey_(agg_index, admin_key);

    return retval;
}/* End of LACP_MGR_SetDot3adAggActorAdminKey */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDefaultDot3adAggActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_actor_admin_key information to default.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDefaultDot3adAggActorAdminKey(UI16_T agg_index)
{
    UI32_T  agg_id;
    UI32_T  trunk_id;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return LACP_RETURN_ERROR;

    if (TRK_OM_IfindexToTrunkId(agg_index, &trunk_id) == FALSE)
    {
        return LACP_RETURN_ERROR;
    }

    agg_id = (trunk_id - 1);

    if (LACP_UTIL_SetDot3adAggActorAdminKey(agg_id, SYS_DFLT_LACP_KEY_NULL,
            TRUE) == TRUE)
    {
        return LACP_RETURN_SUCCESS;
    }
    else
    {
        return LACP_RETURN_ERROR;
    }
}/* End of LACP_MGR_SetDefaultDot3adAggActorAdminKey */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_actor_admin_key information.
 * INPUT    :   UI16_T  agg_index       -- the dot3ad_agg_index
 * OUTPUT   :   UI32_T  *admin_key  -- the dot3ad_agg_actor_admin_key value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetRunningDot3adAggActorAdminKey(UI16_T agg_index, UI16_T *admin_key)
{
    UI32_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    retval=LACP_MGR_GetRunningDot3adAggActorAdminKey_(agg_index, admin_key);

    return retval;
} /* LACP_MGR_GetRunningDot3adAggActorAdminKey */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggCollectorMaxDelay
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_collector_max_delay information.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 *              UI32_T  max_delay   -- the dot3ad_agg_collector_max_delay value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggCollectorMaxDelay(UI16_T agg_index, UI32_T max_delay)
{
    /*only allow to set the default value, because we doesn't support this MIB node*/
    if(max_delay!=0)
        return LACP_RETURN_ERROR;

    return  LACP_RETURN_SUCCESS;
}

/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */
/* Dot3adAggPortEntry */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortActorSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_actor_system_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortActorSystemPriority(UI16_T port_index, UI16_T priority)
{
    UI32_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return LACP_RETURN_ERROR;

    retval=LACP_MGR_SetDot3adAggPortActorSystemPriority_(port_index, priority);

    return retval;
}/* LACP_MGR_SetDot3adAggPortActorSystemPriority */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortActorSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_actor_system_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetRunningDot3adAggPortActorSystemPriority(UI16_T port_index, UI16_T *priority)
{
    UI32_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    retval=LACP_MGR_GetRunningDot3adAggPortActorSystemPriority_(port_index, priority);

    return retval;
}/* LACP_MGR_GetRunningDot3adAggPortActorSystemPriority */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  admin_key   -- the dot3ad_agg_port_actor_admin_key value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortActorAdminKey(UI16_T port_index, UI16_T admin_key)
{
    UI32_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return LACP_RETURN_ERROR;

    retval=LACP_MGR_SetDot3adAggPortActorAdminKey_(port_index, admin_key);

    return retval;
}/* LACP_MGR_SetDot3adAggPortActorAdminKey */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *admin_key  -- the dot3ad_agg_port_actor_admin_key value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetRunningDot3adAggPortActorAdminKey(UI16_T port_index, UI16_T *admin_key)
{
    UI32_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    retval=LACP_MGR_GetRunningDot3adAggPortActorAdminKey_(port_index, admin_key);

    return retval;
}/* LACP_MGR_GetRunningDot3adAggPortActorAdminKey */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDefaultDot3adAggPortActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Restore the dot3ad_agg_port_actor_admin_key to default value.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDefaultDot3adAggPortActorAdminKey(UI16_T port_index)
{
    UI32_T retval;
    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return LACP_RETURN_ERROR;

    retval=LACP_MGR_SetDefaultDot3adAggPortActorAdminKey_(port_index);

    return retval;
}/* LACP_MGR_SetDot3adAggPortActorAdminKey */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortActorOperKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_oper_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  oper_key    -- the dot3ad_agg_port_actor_oper_key value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortActorOperKey(UI16_T port_index, UI16_T oper_key)
{
    return  LACP_RETURN_SUCCESS;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortActorOperKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_oper_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *oper_key   -- the dot3ad_agg_port_actor_oper_key value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetRunningDot3adAggPortActorOperKey(UI16_T port_index, UI16_T *oper_key)
{
    UI32_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    retval=LACP_MGR_GetRunningDot3adAggPortActorOperKey_(port_index, oper_key);

    return retval;
}/* LACP_MGR_GetRunningDot3adAggPortActorOperKey */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortPartnerAdminSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_partner_admin_system_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortPartnerAdminSystemPriority(UI16_T port_index, UI16_T priority)
{
    UI32_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return LACP_RETURN_ERROR;

    retval=LACP_MGR_SetDot3adAggPortPartnerAdminSystemPriority_(port_index, priority);

    return retval;
}/* LACP_MGR_SetDot3adAggPortPartnerAdminSystemPriority */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortPartnerAdminSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_partner_admin_system_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetRunningDot3adAggPortPartnerAdminSystemPriority(UI16_T port_index, UI16_T *priority)
{
    UI32_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    retval=LACP_MGR_GetRunningDot3adAggPortPartnerAdminSystemPriority_(port_index, priority);

    return retval;
}/* LACP_MGR_GetRunningDot3adAggPortPartnerAdminSystemPriority */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortPartnerAdminSystemId
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_system_id information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI8_T   *system_id  -- the dot3ad_agg_port_partner_admin_system_id value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortPartnerAdminSystemId(UI16_T port_index, UI8_T *system_id)
{
    return  LACP_RETURN_SUCCESS;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortPartnerAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  admin_key   -- the dot3ad_agg_port_partner_admin_key value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortPartnerAdminKey(UI16_T port_index, UI16_T admin_key)
{
    UI32_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return LACP_RETURN_ERROR;

    retval=LACP_MGR_SetDot3adAggPortPartnerAdminKey_(port_index, admin_key);

    return retval;
}/* LACP_MGR_SetDot3adAggPortPartnerAdminKey */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortPartnerAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *admin_key  -- the dot3ad_agg_port_partner_admin_key value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetRunningDot3adAggPortPartnerAdminKey(UI16_T port_index, UI16_T *admin_key)
{
    UI32_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    retval=LACP_MGR_GetRunningDot3adAggPortPartnerAdminKey_(port_index, admin_key);

    return retval;
}/* LACP_MGR_GetRunningDot3adAggPortPartnerAdminKey */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortActorPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_actor_port_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortActorPortPriority(UI16_T port_index, UI16_T priority)
{
    UI32_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return LACP_RETURN_ERROR;

    retval=LACP_MGR_SetDot3adAggPortActorPortPriority_(port_index, priority);

    return retval;
}/* LACP_MGR_SetDot3adAggPortActorPortPriority */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortActorPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_actor_port_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetRunningDot3adAggPortActorPortPriority(UI16_T port_index, UI16_T *priority)
{
    UI32_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    retval=LACP_MGR_GetRunningDot3adAggPortActorPortPriority_(port_index, priority);

    return retval;
}/* LACP_MGR_GetRunningDot3adAggPortActorPortPriority */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortPartnerAdminPort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_port information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  admin_port  -- the dot3ad_agg_port_partner_admin_port value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortPartnerAdminPort(UI16_T port_index, UI16_T admin_port)
{
    return  LACP_RETURN_SUCCESS;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortPartnerAdminPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_partner_admin_port_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortPartnerAdminPortPriority(UI16_T port_index, UI16_T priority)
{
    UI32_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return LACP_RETURN_ERROR;

    retval=LACP_MGR_SetDot3adAggPortPartnerAdminPortPriority_(port_index, priority);

    return retval;
}/* LACP_MGR_SetDot3adAggPortPartnerAdminPort */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortPartnerAdminPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_partner_admin_port_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetRunningDot3adAggPortPartnerAdminPortPriority(UI16_T port_index, UI16_T *priority)
{
    UI32_T retval;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    retval=LACP_MGR_GetRunningDot3adAggPortPartnerAdminPortPriority_(port_index, priority);

    return retval;
}/* LACP_MGR_GetRunningDot3adAggPortPartnerAdminPortPriority */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortActorAdminState
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_admin_state information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI8_T   admin_state -- the dot3ad_agg_port_actor_admin_state value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortActorAdminState(UI16_T port_index, UI8_T admin_state)
{
    return  LACP_RETURN_SUCCESS;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortPartnerAdminState
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_state information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI8_T   admin_state -- the dot3ad_agg_port_partner_admin_state value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDot3adAggPortPartnerAdminState(UI16_T port_index, UI8_T admin_state)
{
    return  LACP_RETURN_SUCCESS;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetSystemPtr
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : lport -- lport number
 * OUTPUT   : None
 * RETUEN   : Pointer to the system variables
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
LACP_SYSTEM_T*   LACP_MGR_GetSystemPtr(void)
{
    return LACP_OM_GetSystemPtr();
} /* End of LACP_MGR_GetSystemPtr */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_HandleHotInsertion
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
void LACP_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    LACP_PORT_T *pPort;
    Port_Info_T port_info;
    UI32_T      port_ifindex;

    LACP_OM_RefreshPortObject(starting_port_ifindex, number_of_port);
    for(port_ifindex = 0; port_ifindex < number_of_port ; port_ifindex++ )
    {
        pPort = LACP_OM_GetPortPtr(starting_port_ifindex+port_ifindex);

        if(SWCTRL_GetPortInfo(starting_port_ifindex+port_ifindex, &port_info) != FALSE)
        {
            switch(port_info.admin_state)
            {
                case VAL_ifAdminStatus_down:
                    /* If this port is "DISABLED". */
                    LACP_EnablePort( pPort, FALSE);
                    break;
                case VAL_ifAdminStatus_up:
                default:
                    /* Default it's considered as "ENABLED". */
                    LACP_UTIL_RefreshPortSpeedDuplex(pPort);
                    LACP_EnablePort( pPort, TRUE);
                    break;
            }
        }
    }

    return;
} /* End of LACP_MGR_HandleHotInsertion */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_HandleHotRemoval
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
void LACP_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    LACP_PORT_T *pPort;
    UI32_T      port_ifindex;

    for(port_ifindex = 0; port_ifindex < number_of_port ; port_ifindex++ )
    {
        pPort = LACP_OM_GetPortPtr(starting_port_ifindex+port_ifindex);
        LACP_EnablePort( pPort, FALSE);
    }

    return;
} /* End of LACP_MGR_HandleHotRemoval */


static UI32_T  LACP_MGR_SetDot3adAggActorSystemPriority_(UI32_T agg_index, UI16_T priority)
{
    UI16_T              actor_priority;
    UI32_T              agg_id;
    UI32_T              trunk_id;

    if ( !TRK_OM_IfindexToTrunkId(agg_index, &trunk_id) )
    {
        return LACP_RETURN_ERROR;
    }

    agg_id = (trunk_id - 1);

    if (!LACP_Get_dot3adAggActorSystemPriority(agg_id, &actor_priority))
    {
        return LACP_RETURN_ERROR;
    }

    if (actor_priority == priority)
    {
        return LACP_RETURN_SUCCESS;
    }

    if (LACP_UTIL_SetDot3adAggActorSystemPriority(agg_id, priority))
    {
        return LACP_RETURN_SUCCESS;
    }
    else
    {
        return LACP_RETURN_ERROR;
    }

} /* End of LACP_MGR_SetDot3adAggActorSystemPriority */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggActorAdminKey_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_actor_admin_key information.
 * INPUT    :   UI32_T  agg_index   -- the dot3ad_agg_index
 *              UI16_T  admin_key   -- the dot3ad_agg_actor_admin_key value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
static UI32_T  LACP_MGR_SetDot3adAggActorAdminKey_(UI32_T agg_index, UI16_T admin_key)
{
    UI32_T  agg_id;
    UI32_T  trunk_id;
    UI16_T  actor_admin_key;

    if (TRK_OM_IfindexToTrunkId(agg_index, &trunk_id) == FALSE)
    {
        return LACP_RETURN_ERROR;
    }

    DEBUG_CONFIG("%sSet admin key of trunk %lu to %u\r\n\r\n", DEBUG_HEADER(),
        (unsigned long)trunk_id, admin_key);

    agg_id = (trunk_id - 1);

    actor_admin_key = 0;
    if (LACP_Get_dot3adAggActorAdminKey(agg_id, &actor_admin_key) == FALSE)
    {
        return LACP_RETURN_ERROR;
    }

    if (    (actor_admin_key != SYS_DFLT_LACP_KEY_NULL)
         && (actor_admin_key == admin_key)
       )
    {
        return LACP_RETURN_SUCCESS;
    }

#if (SYS_CPNT_LACP_STATIC_JOIN_TRUNK == TRUE)
    {
        LACP_SYSTEM_T       *lacp_system_ptr;
        LACP_AGGREGATOR_T   *aggregator_ptr;
        UI16_T aggActorAdminKeyOld = 0;

        lacp_system_ptr = LACP_OM_GetSystemPtr();
        aggregator_ptr  = &(lacp_system_ptr->aggregator[agg_id]);
        if (aggregator_ptr)
        {
            aggActorAdminKeyOld = aggregator_ptr->AggActorAdminKey;

            if(TRUE == LACP_UTIL_CheckAggKeyIsSameWithOtherAgg(agg_id, admin_key))
            {
                return LACP_RETURN_ERROR;
            }
        }

        if ((0 == aggActorAdminKeyOld) && (0 != admin_key))
        {
            LACP_UTIL_CheckAggKeyIsMatchPortKeyAndAddStaticTrunkMember(trunk_id, admin_key);

        }
        else if ((0 != aggActorAdminKeyOld) && (0 == admin_key))
        {
            //the reason before LACP_UTIL_SetDot3adAggActorAdminKey is port key will be set by agg key
            LACP_UTIL_CheckAggKeyIsMatchPortKeyAndDeleteStaticTrunkMember(trunk_id, aggActorAdminKeyOld);
        }
        else
        {
            ;
        }
    }
#endif

    if (LACP_UTIL_SetDot3adAggActorAdminKey(agg_id, admin_key, FALSE) == TRUE)
    {
        return LACP_RETURN_SUCCESS;
    }
    else
    {
        return LACP_RETURN_ERROR;
    }
} /* End of LACP_MGR_SetDot3adAggActorAdminKey_ */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggActorAdminKey_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_actor_admin_key information.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 * OUTPUT   :   UI32_T  *admin_key  -- the dot3ad_agg_actor_admin_key value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
static UI32_T  LACP_MGR_GetRunningDot3adAggActorAdminKey_(UI32_T agg_index, UI16_T *admin_key)
{
    UI32_T              agg_id;
    UI32_T              trunk_id;
    LACP_AGGREGATOR_T   *aggregator_ptr;

    if (!TRK_OM_IfindexToTrunkId(agg_index, &trunk_id))
    {
        return LACP_RETURN_ERROR;
    }

    *admin_key = 0;
    agg_id = (trunk_id - 1);
    aggregator_ptr=LACP_OM_GetAggPtr(agg_id);

    if (aggregator_ptr->static_admin_key)
    {
        *admin_key = aggregator_ptr->AggActorAdminKey;
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
} /* LACP_MGR_GetRunningDot3adAggActorAdminKey_ */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortActorSystemPriority_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_actor_system_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
static UI32_T  LACP_MGR_SetDot3adAggPortActorSystemPriority_(UI16_T port_index, UI16_T priority)
{
    UI16_T              actor_system_priority;

    DEBUG_CONFIG("%sSet actor system priority of port %u to %u\r\n\r\n",
        DEBUG_HEADER(), port_index, priority);

/* always not hold
    if (    (priority < MIN_dot3adAggPortActorSystemPriority)
         || (priority > MAX_dot3adAggPortActorSystemPriority)
       )
    {
        return LACP_RETURN_ERROR;
    }
*/
    if (!LACP_Get_dot3adAggPortActorSystemPriority(port_index, &actor_system_priority))
    {
        return LACP_RETURN_ERROR;
    }

    if (actor_system_priority == priority)
    {
        return LACP_RETURN_SUCCESS;
    }

    if (LACP_UTIL_SetDot3adAggPortActorSystemPriority(port_index, priority))
    {
        return LACP_RETURN_SUCCESS;
    }
    else
    {
        return LACP_RETURN_ERROR;
    }

}/* LACP_MGR_SetDot3adAggPortActorSystemPriority_ */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortActorAdminKey_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  admin_key   -- the dot3ad_agg_port_actor_admin_key value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
static UI32_T  LACP_MGR_SetDot3adAggPortActorAdminKey_(UI16_T port_index, UI16_T admin_key)
{
    UI16_T              actor_admin_key;

    DEBUG_CONFIG("%sport %u; admin_key: %u\r\n\r\n", DEBUG_HEADER(), port_index,
        admin_key);

/* always not hold
    if (    (admin_key < MIN_dot3adAggPortActorAdminKey)
         || (admin_key > MAX_dot3adAggPortActorAdminKey)
       )
    {
        return LACP_RETURN_ERROR;
    }
*/
    if (!LACP_Get_dot3adAggPortActorAdminKey(port_index, &actor_admin_key))
    {
        return LACP_RETURN_ERROR;
    }

    if (LACP_UTIL_SetDot3adAggPortActorAdminKey(port_index, admin_key))
    {
#if (SYS_CPNT_LACP_STATIC_JOIN_TRUNK == TRUE)
        UI32_T trunk_id;
        LACP_PORT_T *port_ptr;
        UI32_T slot, port;
        SWCTRL_Lport_Type_T lport_type = SWCTRL_LPORT_UNKNOWN_PORT;

        lport_type = SWCTRL_LogicalPortToUserPort(port_index, &slot, &port, &trunk_id);
        if((SWCTRL_LPORT_UNKNOWN_PORT == lport_type)
           ||(SWCTRL_LPORT_TRUNK_PORT == lport_type))
        {
           return LACP_RETURN_SUCCESS;
        }

        port_ptr = LACP_FIND_PORT(slot, port);
        if(port_ptr
           && port_ptr->LACP_OperStatus == TRUE
           && actor_admin_key != admin_key)
        {
            LACP_UTIL_CheckPortKeyIsMatchAggKeyAndDeleteStaticTrunkMember(port_ptr, actor_admin_key);
            LACP_UTIL_CheckPortKeyIsMatchAggKeyAndAddStaticTrunkMember(port_ptr, admin_key);
        }
#endif
        return LACP_RETURN_SUCCESS;
    }
    else
    {
        return LACP_RETURN_ERROR;
    }

}/* LACP_MGR_SetDot3adAggPortActorAdminKey_ */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDefaultDot3adAggPortActorAdminKey_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Restore the dot3ad_agg_port_actor_admin_key to default value.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_SetDefaultDot3adAggPortActorAdminKey_(UI16_T port_index)
{
    UI16_T              actor_admin_key;
    UI16_T              actor_default_key;

    if (!LACP_Get_dot3adAggPortActorAdminKey(port_index, &actor_admin_key))
    {
        return LACP_RETURN_ERROR;
    }

    if(LACP_MGR_GetDefaultDot3adAggPortActorAdminKey_(port_index, &actor_default_key) != LACP_RETURN_SUCCESS)
    {
        return LACP_RETURN_ERROR;
    }

    if (actor_admin_key == actor_default_key)
    {
        LACP_UTIL_ClearStaticAdminKeyFlag((UI32_T)port_index);
        return LACP_RETURN_SUCCESS;
    }

    if (LACP_UTIL_SetDot3adAggPortActorAdminKey(port_index, actor_default_key))
    {
        LACP_UTIL_ClearStaticAdminKeyFlag((UI32_T)port_index);
        return LACP_RETURN_SUCCESS;
    }
    else
    {
        return LACP_RETURN_ERROR;
    }

}/* LACP_MGR_SetDefaultDot3adAggPortActorAdminKey_ */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDefaultDot3adAggPortActorAdminKey_
 * ------------------------------------------------------------------------
 * PURPOSE  :  Get default value of the dot3ad_agg_port_actor_admin_key.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *admin_key  -- the default admin key
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
static UI32_T LACP_MGR_GetDefaultDot3adAggPortActorAdminKey_(UI16_T port_index, UI16_T *admin_key)
{
    UI32_T              speed_duplex_oper;
    Port_Info_T         port_info;

    if (!SWCTRL_GetPortInfo(port_index, &port_info))
    {
        return LACP_RETURN_ERROR;
    }
    speed_duplex_oper = port_info.speed_duplex_oper;

    switch(speed_duplex_oper)
    {
        case VAL_portSpeedDpxStatus_halfDuplex10:
        case VAL_portSpeedDpxStatus_fullDuplex10:
            *admin_key = SYS_DFLT_LACP_KEY_10FULL;
            break;
        case VAL_portSpeedDpxStatus_halfDuplex100:
        case VAL_portSpeedDpxStatus_fullDuplex100:
            *admin_key = SYS_DFLT_LACP_KEY_100FULL;
            break;
        case VAL_portSpeedDpxStatus_halfDuplex1000:
        case VAL_portSpeedDpxStatus_fullDuplex1000:
            *admin_key = SYS_DFLT_LACP_KEY_1000FULL;
            break;
        case VAL_portSpeedDpxStatus_halfDuplex10g:
        case VAL_portSpeedDpxStatus_fullDuplex10g:
            *admin_key = SYS_DFLT_LACP_KEY_10GFULL;
            break;
        case VAL_portSpeedDpxStatus_halfDuplex40g:
        case VAL_portSpeedDpxStatus_fullDuplex40g:
            *admin_key = SYS_DFLT_LACP_KEY_40GFULL;
            break;
        case VAL_portSpeedDpxStatus_halfDuplex100g:
        case VAL_portSpeedDpxStatus_fullDuplex100g:
            *admin_key = SYS_DFLT_LACP_KEY_100GFULL;
            break;
#ifdef SYS_DFLT_LACP_KEY_25GFULL
        case VAL_portSpeedDpxStatus_fullDuplex25g:
            *admin_key = SYS_DFLT_LACP_KEY_25GFULL;
            break;
#endif
        default:
            *admin_key = SYS_DFLT_LACP_KEY_DEFAULT;
            break;
    }
    return LACP_RETURN_SUCCESS;

}/* LACP_MGR_GetDefaultDot3adAggPortActorAdminKey_ */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortActorPortPriority_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_actor_port_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
static UI32_T  LACP_MGR_SetDot3adAggPortActorPortPriority_(UI16_T port_index, UI16_T priority)
{
    UI16_T              actor_port_priority;

    DEBUG_CONFIG("%sSet actor port priority of port %u to %u\r\n\r\n", DEBUG_HEADER(),
        port_index, priority);

/* always not hold
    if (    (priority < MIN_dot3adAggPortActorPortPriority)
         || (priority > MAX_dot3adAggPortActorPortPriority)
       )
    {
        return LACP_RETURN_ERROR;
    }
*/
    if (!LACP_Get_dot3adAggPortActorPortPriority(port_index, &actor_port_priority))
    {
        return LACP_RETURN_ERROR;
    }

    if (actor_port_priority == priority)
    {
        return LACP_RETURN_SUCCESS;
    }

    if (LACP_UTIL_SetDot3adAggPortActorPortPriority(port_index, priority))
    {
        return LACP_RETURN_SUCCESS;
    }
    else
    {
        return LACP_RETURN_ERROR;
    }

}/* LACP_MGR_SetDot3adAggPortActorPortPriority_ */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortActorSystemPriority_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_actor_system_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
static UI32_T  LACP_MGR_GetRunningDot3adAggPortActorSystemPriority_(UI16_T port_index, UI16_T *priority)
{
    if (!LACP_Get_dot3adAggPortActorSystemPriority(port_index, priority))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (*priority == LACP_SYSTEM_DEFAULT_PRIORITY)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

}/* LACP_MGR_GetRunningDot3adAggPortActorSystemPriority_ */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortActorAdminKey_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *admin_key  -- the dot3ad_agg_port_actor_admin_key value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
static UI32_T  LACP_MGR_GetRunningDot3adAggPortActorAdminKey_(UI16_T port_index, UI16_T *admin_key)
{
    UI16_T              actor_default_key;
    LACP_PORT_T         *port_ptr;

    if (!LACP_Get_dot3adAggPortActorAdminKey(port_index, admin_key))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (LACP_MGR_GetDefaultDot3adAggPortActorAdminKey_(port_index, &actor_default_key) != LACP_RETURN_SUCCESS)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    port_ptr = LACP_OM_GetPortPtr(port_index);

    if (port_ptr->static_admin_key)
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}/* LACP_MGR_GetRunningDot3adAggPortActorAdminKey_ */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortActorOperKey_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_oper_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *oper_key   -- the dot3ad_agg_port_actor_oper_key value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
static UI32_T  LACP_MGR_GetRunningDot3adAggPortActorOperKey_(UI16_T port_index, UI16_T *oper_key)
{
    UI16_T              actor_default_key;

    if (!LACP_Get_dot3adAggPortActorOperKey(port_index, oper_key))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    if (LACP_MGR_GetDefaultDot3adAggPortActorAdminKey_(port_index, &actor_default_key) != LACP_RETURN_SUCCESS)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    if (*oper_key == actor_default_key)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}/* LACP_MGR_GetRunningDot3adAggPortActorOperKey_ */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortActorPortPriority_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_actor_port_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
static UI32_T  LACP_MGR_GetRunningDot3adAggPortActorPortPriority_(UI16_T port_index, UI16_T *priority)
{
    if (!LACP_Get_dot3adAggPortActorPortPriority(port_index, priority))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (*priority == LACP_PORT_DEFAULT_PRIORITY)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}/* LACP_MGR_GetRunningDot3adAggPortActorPortPriority_ */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortPartnerAdminSystemPriority_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_partner_admin_system_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
static UI32_T  LACP_MGR_SetDot3adAggPortPartnerAdminSystemPriority_(UI16_T port_index, UI16_T priority)
{
    UI16_T              partner_admin_system_priority;
/* always not hold
    if (    (priority < MIN_dot3adAggPortPartnerAdminSystemPriority)
         || (priority > MAX_dot3adAggPortPartnerAdminSystemPriority)
       )
    {
        return LACP_RETURN_ERROR;
    }
*/
    if (!LACP_Get_dot3adAggPortPartnerAdminSystemPriority(port_index, &partner_admin_system_priority))
    {
        return LACP_RETURN_ERROR;
    }

    if (partner_admin_system_priority == priority)
    {
        return LACP_RETURN_SUCCESS;
    }

    if (LACP_UTIL_SetDot3adAggPortPartnerAdminSystemPriority(port_index, priority))
    {
        return LACP_RETURN_SUCCESS;
    }
    else
    {
        return LACP_RETURN_ERROR;
    };
}/* LACP_MGR_SetDot3adAggPortPartnerAdminSystemPriority_ */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortPartnerAdminKey_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  admin_key   -- the dot3ad_agg_port_partner_admin_key value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
static UI32_T  LACP_MGR_SetDot3adAggPortPartnerAdminKey_(UI16_T port_index, UI16_T admin_key)
{
    UI16_T              partner_admin_key;
/*
    if (    (admin_key < MIN_dot3adAggPortPartnerAdminKey)
         || (admin_key > MAX_dot3adAggPortPartnerAdminKey)
       )
    {
        return LACP_RETURN_ERROR;
    }
*/
    if (!LACP_Get_dot3adAggPortPartnerAdminKey(port_index, &partner_admin_key))
    {
        return LACP_RETURN_ERROR;
    }

    if (partner_admin_key == admin_key)
    {
        return LACP_RETURN_SUCCESS;
    }

    if (LACP_UTIL_SetDot3adAggPortPartnerAdminKey(port_index, admin_key))
    {
        return LACP_RETURN_SUCCESS;
    }
    else
    {
        return LACP_RETURN_ERROR;
    }

}/* LACP_MGR_SetDot3adAggPortPartnerAdminKey_ */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortPartnerAdminPortPriority_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_partner_admin_port_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
static UI32_T  LACP_MGR_SetDot3adAggPortPartnerAdminPortPriority_(UI16_T port_index, UI16_T priority)
{
    UI16_T              partner_admin_port_priority;
/* always not hold
    if (    (priority < MIN_dot3adAggPortPartnerAdminPortPriority)
        ||  (priority > MAX_dot3adAggPortPartnerAdminPortPriority)
       )
    {
        return LACP_RETURN_ERROR;
    }
*/
    if (!LACP_Get_dot3adAggPortPartnerAdminPortPriority(port_index, &partner_admin_port_priority))
    {
        return LACP_RETURN_ERROR;
    }

    if (partner_admin_port_priority == priority)
    {
        return LACP_RETURN_SUCCESS;
    }

    if (LACP_UTIL_SetDot3adAggPortPartnerAdminPortPriority(port_index, priority))
    {
        return LACP_RETURN_SUCCESS;
    }
    else
    {
        return LACP_RETURN_ERROR;
    }

}/* LACP_MGR_SetDot3adAggPortPartnerAdminPortPriority_ */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortPartnerAdminSystemPriority_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_partner_admin_system_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
static UI32_T  LACP_MGR_GetRunningDot3adAggPortPartnerAdminSystemPriority_(UI16_T port_index, UI16_T *priority)
{
    if (!LACP_Get_dot3adAggPortPartnerAdminSystemPriority(port_index, priority))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (*priority == LACP_SYSTEM_DEFAULT_PRIORITY)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}/* LACP_MGR_GetRunningDot3adAggPortPartnerAdminSystemPriority_ */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortPartnerAdminKey_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *admin_key  -- the dot3ad_agg_port_partner_admin_key value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
static UI32_T  LACP_MGR_GetRunningDot3adAggPortPartnerAdminKey_(UI16_T port_index, UI16_T *admin_key)
{
    if (!LACP_Get_dot3adAggPortPartnerAdminKey(port_index, admin_key))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (*admin_key == SYS_DFLT_LACP_KEY_NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}/* LACP_MGR_GetRunningDot3adAggPortPartnerAdminKey_ */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortPartnerAdminPortPriority_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_partner_admin_port_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
static UI32_T  LACP_MGR_GetRunningDot3adAggPortPartnerAdminPortPriority_(UI16_T port_index, UI16_T *priority)
{
    if (!LACP_Get_dot3adAggPortPartnerAdminPortPriority(port_index, priority))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (*priority == LACP_PORT_DEFAULT_PRIORITY)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}/* LACP_MGR_GetRunningDot3adAggPortPartnerAdminPortPriority_ */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggPortActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   set the port with long term or short term timeout
 * INPUT    :   UI32_T  unit  -- the unit number
 *              UI32_T  port  -- ther port number
 *              UI32_T  timeout -- long or short term timeout
 * RETURN   :   LACP_RETURN_SUCCESS - success
 *              LACP_RETURN_ERROR - fail
 * NOTES    :
 * ------------------------------------------------------------------------
 */
UI32_T LACP_MGR_SetDot3adAggPortActorLACP_Timeout(UI32_T unit, UI32_T port, UI32_T timeout)
{
    UI32_T port_type;
    UI32_T ifindex;
    LACP_PORT_T *pPort;

    port_type=SWCTRL_UserPortToIfindex(unit,port,&ifindex);

    if ((SWCTRL_LPORT_NORMAL_PORT != port_type) && (SWCTRL_LPORT_TRUNK_PORT_MEMBER!=port_type))
    {
        return LACP_RETURN_ERROR;
    }
    if(timeout!=LACP_LONG_TIMEOUT && timeout!=LACP_SHORT_TIMEOUT)
    {
        return LACP_RETURN_ERROR;
    }

    if (FALSE == LACP_UTIL_SetDot3AggPortActorLacp_Timeout(ifindex,timeout))
    {
        return LACP_RETURN_ERROR;
    }

    pPort = LACP_OM_GetPortPtr(ifindex);

    LACP_Periodic_Machine( pPort, LACP_TIMER_EV);

    return LACP_RETURN_SUCCESS;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggPortActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   get the port with long term or short term timeout
 * INPUT    :   UI32_T  unit  -- the unit number
 *              UI32_T  port  -- ther port number
 *              UI32_T  timeout -- long or short term timeout
 * RETURN   :   LACP_RETURN_SUCCESS - success
 *              LACP_RETURN_ERROR - fail
 * NOTES    :
 * ------------------------------------------------------------------------
 */
UI32_T LACP_MGR_GetDot3adAggPortActorLACP_Timeout(UI32_T unit, UI32_T port, UI32_T *timeout)
{
    UI32_T port_type;
    UI32_T ifindex;

    port_type=SWCTRL_UserPortToIfindex(unit,port,&ifindex);

    if ((SWCTRL_LPORT_NORMAL_PORT != port_type) && (SWCTRL_LPORT_TRUNK_PORT_MEMBER!=port_type))
    {
        return LACP_RETURN_ERROR;
    }

    if (FALSE == LACP_UTIL_GetDot3AggPortActorLacp_Timeout(ifindex, timeout))
    {
        return LACP_RETURN_ERROR;
    }

    return LACP_RETURN_SUCCESS;
}


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggPortActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_port_lacp_timeout information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_actor_port_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetRunningDot3adAggPortActorLACP_Timeout(UI16_T port_index, UI32_T *timeout)
{
    if (!LACP_UTIL_GetDot3AggPortActorLacp_Timeout(port_index, timeout))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (*timeout == LACP_LONG_TIMEOUT)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
} /* LACP_MGR_GetRunningDot3adAggPortActorLACP_Timeout */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_SetDot3adAggActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   set the lacp port channel with long term or short term timeout.
 * INPUT    :   UI16_T agg_index -- the dot3ad_agg_index
 *              UI32_T  timeout -- long or short term timeout
 * RETURN   :   LACP_RETURN_SUCCESS - success
 *              LACP_RETURN_ERROR - fail
 * NOTES    :
 * ------------------------------------------------------------------------
 */
UI32_T LACP_MGR_SetDot3adAggActorLACP_Timeout(UI16_T agg_index, UI32_T timeout)
{
    UI32_T              agg_id;
    UI32_T              trunk_id;
    UI32_T              actor_timeout;

    if(timeout!=LACP_LONG_TIMEOUT && timeout!=LACP_SHORT_TIMEOUT)
    {
        return LACP_RETURN_ERROR;
    }

    if ( !TRK_OM_IfindexToTrunkId(agg_index, &trunk_id) )
    {
        return LACP_RETURN_ERROR;
    }

    DEBUG_CONFIG("%sSet timeout of trunk %u to %s\r\n\r\n", DEBUG_HEADER(),
        trunk_id, (timeout == LACP_LONG_TIMEOUT) ? "long-timeout" : "short-timeout");

    agg_id = (trunk_id - 1);

    if (!LACP_Get_dot3adAggActorTimeout(agg_id, &actor_timeout))
    {
        return LACP_RETURN_ERROR;
    }

    if (timeout == actor_timeout)
    {
        return LACP_RETURN_SUCCESS;
    }

    if (!LACP_UTIL_SetDot3adAggActorTimeout(agg_id, timeout))
        return LACP_RETURN_ERROR;

    return LACP_RETURN_SUCCESS;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adAggActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   get the lacp port channel with long term or short term timeout
 * INPUT    :   UI16_T agg_index  -- the dot3ad_agg_index
 *              UI32_T  timeout -- long or short term timeout
 * RETURN   :   LACP_RETURN_SUCCESS - success
 *              LACP_RETURN_ERROR - fail
 * NOTES    :
 * ------------------------------------------------------------------------
 */
UI32_T LACP_MGR_GetDot3adAggActorLACP_Timeout(UI16_T agg_index, UI32_T *timeout)
{
    UI32_T              agg_id;
    UI32_T              trunk_id;

    if ( !TRK_OM_IfindexToTrunkId(agg_index, &trunk_id) )
    {
        return LACP_RETURN_ERROR;
    }

    agg_id = (trunk_id - 1);

    if (FALSE == LACP_Get_dot3adAggActorTimeout(agg_id, timeout))
    {
        return LACP_RETURN_ERROR;
    }

    return LACP_RETURN_SUCCESS;
}


/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetRunningDot3adAggActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_lacp_timeout information.
 * INPUT    :   UI16_T agg_index  -- the dot3ad_agg_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_actor_port_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_MGR_GetRunningDot3adAggActorLACP_Timeout(UI16_T agg_index, UI32_T *timeout)
{
    UI32_T              agg_id;
    UI32_T              trunk_id;

    if ( !TRK_OM_IfindexToTrunkId(agg_index, &trunk_id) )
    {
        return LACP_RETURN_ERROR;
    }

    agg_id = (trunk_id - 1);

    if (!LACP_Get_dot3adAggActorTimeout(agg_id, timeout))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (*timeout == LACP_DEFAULT_SYSTEM_TIMEOUT)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
} /* LACP_MGR_GetRunningDot3adAggActorLACP_Timeout */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: LACP_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for LACP MGR.
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
BOOL_T LACP_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    LACP_MGR_IpcMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msg_p->type.ret_ui32 = LACP_RETURN_ERROR;
        msgbuf_p->msg_size = LACP_MGR_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    /* dispatch IPC message and call the corresponding LACP_MGR function
     */
    switch (msg_p->type.cmd)
    {
        case LACP_MGR_IPC_GETDOT3ADAGGENTRY:
            msg_p->type.ret_bool = LACP_MGR_GetDot3adAggEntry(
                &msg_p->data.arg_agg_entry);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_agg_entry);
            break;

        case LACP_MGR_IPC_GETNEXTDOT3ADAGGENTRY:
            msg_p->type.ret_bool = LACP_MGR_GetNextDot3adAggEntry(
                &msg_p->data.arg_agg_entry);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_agg_entry);
            break;

        case LACP_MGR_IPC_GETDOT3ADAGGPORTLISTENTRY:
            msg_p->type.ret_bool = LACP_MGR_GetDot3adAggPortListEntry(
                &msg_p->data.arg_agg_port_list_entry);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_agg_port_list_entry);
            break;

        case LACP_MGR_IPC_GETNEXTDOT3ADAGGPORTLISTENTRY:
            msg_p->type.ret_bool = LACP_MGR_GetNextDot3adAggPortListEntry(
                &msg_p->data.arg_agg_port_list_entry);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_agg_port_list_entry);
            break;

        case LACP_MGR_IPC_GETDOT3ADAGGPORTENTRY:
            msg_p->type.ret_bool = LACP_MGR_GetDot3adAggPortEntry(
                &msg_p->data.arg_agg_port_entry);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_agg_port_entry);
            break;

        case LACP_MGR_IPC_GETNEXTDOT3ADAGGPORTENTRY:
            msg_p->type.ret_bool = LACP_MGR_GetNextDot3adAggPortEntry(
                &msg_p->data.arg_agg_port_entry);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_agg_port_entry);
            break;

        case LACP_MGR_IPC_GETDOT3ADAGGPORTSTATSENTRY:
            msg_p->type.ret_bool = LACP_MGR_GetDot3adAggPortStatsEntry(
                &msg_p->data.arg_agg_port_stats_entry);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_agg_port_stats_entry);
            break;

        case LACP_MGR_IPC_GETNEXTDOT3ADAGGPORTSTATSENTRY:
            msg_p->type.ret_bool = LACP_MGR_GetNextDot3adAggPortStatsEntry(
                &msg_p->data.arg_agg_port_stats_entry);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_agg_port_stats_entry);
            break;

        case LACP_MGR_IPC_GETDOT3ADAGGPORTDEBUGENTRY:
            msg_p->type.ret_bool = LACP_MGR_GetDot3adAggPortDebugEntry(
                &msg_p->data.arg_agg_port_debug_entry);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_agg_port_debug_entry);
            break;

        case LACP_MGR_IPC_GETNEXTDOT3ADAGGPORTDEBUGENTRY:
            msg_p->type.ret_bool = LACP_MGR_GetNextDot3adAggPortDebugEntry(
                &msg_p->data.arg_agg_port_debug_entry);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_agg_port_debug_entry);
            break;

        case LACP_MGR_IPC_GETDOT3ADLAGMIBOBJECTS:
            msg_p->type.ret_bool = LACP_MGR_GetDot3adLagMibObjects(
                &msg_p->data.arg_lag_mib_objects);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_lag_mib_objects);
            break;

        case LACP_MGR_IPC_SETDOT3ADLACPPORTENABLED:
            msg_p->type.ret_ui32 = LACP_MGR_SetDot3adLacpPortEnabled(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = LACP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LACP_MGR_IPC_GETDOT3ADLACPPORTENTRY:
            msg_p->type.ret_bool = LACP_MGR_GetDot3adLacpPortEntry(
                &msg_p->data.arg_lacp_port_entry);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_lacp_port_entry);
            break;

        case LACP_MGR_IPC_GETNEXTDOT3ADLACPPORTENTRY:
            msg_p->type.ret_bool = LACP_MGR_GetNextDot3adLacpPortEntry(
                &msg_p->data.arg_lacp_port_entry);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_lacp_port_entry);
            break;

        case LACP_MGR_IPC_SETDOT3ADAGGACTORSYSTEMPRIORITY:
            msg_p->type.ret_ui32 = LACP_MGR_SetDot3adAggActorSystemPriority(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = LACP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LACP_MGR_IPC_SETDOT3ADAGGACTORADMINKEY:
            msg_p->type.ret_ui32 = LACP_MGR_SetDot3adAggActorAdminKey(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = LACP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LACP_MGR_IPC_SETDEFAULTDOT3ADAGGACTORADMINKEY:
            msg_p->type.ret_ui32 = LACP_MGR_SetDefaultDot3adAggActorAdminKey(
                msg_p->data.arg_ui16);
            msgbuf_p->msg_size = LACP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LACP_MGR_IPC_GETRUNNINGDOT3ADAGGACTORADMINKEY:
            msg_p->type.ret_ui32 = LACP_MGR_GetRunningDot3adAggActorAdminKey(
                msg_p->data.arg_grp2.arg1, &msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
            break;

        case LACP_MGR_IPC_SETDOT3ADAGGCOLLECTORMAXDELAY:
            msg_p->type.ret_ui32 = LACP_MGR_SetDot3adAggCollectorMaxDelay(
                msg_p->data.arg_grp3.arg1, msg_p->data.arg_grp3.arg2);
            msgbuf_p->msg_size = LACP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LACP_MGR_IPC_SETDOT3ADAGGPORTACTORSYSTEMPRIORITY:
            msg_p->type.ret_ui32 = LACP_MGR_SetDot3adAggPortActorSystemPriority(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = LACP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LACP_MGR_IPC_GETRUNNINGDOT3ADAGGPORTACTORSYSTEMPRIORITY:
            msg_p->type.ret_ui32 = LACP_MGR_GetRunningDot3adAggPortActorSystemPriority(
                msg_p->data.arg_grp2.arg1, &msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
            break;

        case LACP_MGR_IPC_SETDOT3ADAGGPORTACTORADMINKEY:
            msg_p->type.ret_ui32 = LACP_MGR_SetDot3adAggPortActorAdminKey(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = LACP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LACP_MGR_IPC_GETRUNNINGDOT3ADAGGPORTACTORADMINKEY:
            msg_p->type.ret_ui32 = LACP_MGR_GetRunningDot3adAggPortActorAdminKey(
                msg_p->data.arg_grp2.arg1, &msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
            break;

        case LACP_MGR_IPC_SETDEFAULTDOT3ADAGGPORTACTORADMINKEY:
            msg_p->type.ret_ui32 = LACP_MGR_SetDefaultDot3adAggPortActorAdminKey(
                msg_p->data.arg_ui16);
            msgbuf_p->msg_size = LACP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LACP_MGR_IPC_SETDOT3ADAGGPORTACTOROPERKEY:
            msg_p->type.ret_ui32 = LACP_MGR_SetDot3adAggPortActorOperKey(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = LACP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LACP_MGR_IPC_SETDOT3ADAGGPORTPARTNERADMINSYSTEMPRIORITY:
            msg_p->type.ret_ui32 = LACP_MGR_SetDot3adAggPortPartnerAdminSystemPriority(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = LACP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LACP_MGR_IPC_GETRUNNINGDOT3ADAGGPORTPARTNERADMINSYSTEMPRIORITY:
            msg_p->type.ret_ui32 = LACP_MGR_GetRunningDot3adAggPortPartnerAdminSystemPriority(
                msg_p->data.arg_grp2.arg1, &msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
            break;

        case LACP_MGR_IPC_SETDOT3ADAGGPORTPARTNERADMINSYSTEMID:
            msg_p->type.ret_ui32 = LACP_MGR_SetDot3adAggPortPartnerAdminSystemId(
                msg_p->data.arg_grp4.arg1, msg_p->data.arg_grp4.arg2);
            msgbuf_p->msg_size = LACP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LACP_MGR_IPC_SETDOT3ADAGGPORTPARTNERADMINKEY:
            msg_p->type.ret_ui32 = LACP_MGR_SetDot3adAggPortPartnerAdminKey(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = LACP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LACP_MGR_IPC_GETRUNNINGDOT3ADAGGPORTPARTNERADMINKEY:
            msg_p->type.ret_ui32 = LACP_MGR_GetRunningDot3adAggPortPartnerAdminKey(
                msg_p->data.arg_grp2.arg1, &msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
            break;

        case LACP_MGR_IPC_SETDOT3ADAGGPORTACTORPORTPRIORITY:
            msg_p->type.ret_ui32 = LACP_MGR_SetDot3adAggPortActorPortPriority(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = LACP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LACP_MGR_IPC_GETRUNNINGDOT3ADAGGPORTACTORPORTPRIORITY:
            msg_p->type.ret_ui32 = LACP_MGR_GetRunningDot3adAggPortActorPortPriority(
                msg_p->data.arg_grp2.arg1, &msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
            break;

        case LACP_MGR_IPC_SETDOT3ADAGGPORTPARTNERADMINPORT:
            msg_p->type.ret_ui32 = LACP_MGR_SetDot3adAggPortPartnerAdminPort(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = LACP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LACP_MGR_IPC_SETDOT3ADAGGPORTPARTNERADMINPORTPRIORITY:
            msg_p->type.ret_ui32 = LACP_MGR_SetDot3adAggPortPartnerAdminPortPriority(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = LACP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LACP_MGR_IPC_GETRUNNINGDOT3ADAGGPORTPARTNERADMINPORTPRIORITY:
            msg_p->type.ret_ui32 = LACP_MGR_GetRunningDot3adAggPortPartnerAdminPortPriority(
                msg_p->data.arg_grp2.arg1, &msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
            break;

        case LACP_MGR_IPC_SETDOT3ADAGGPORTACTORADMINSTATE:
            msg_p->type.ret_ui32 = LACP_MGR_SetDot3adAggPortActorAdminState(
                msg_p->data.arg_grp5.arg1, msg_p->data.arg_grp5.arg2);
            msgbuf_p->msg_size = LACP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LACP_MGR_IPC_SETDOT3ADAGGPORTPARTNERADMINSTATE:
            msg_p->type.ret_ui32 = LACP_MGR_SetDot3adAggPortPartnerAdminState(
                msg_p->data.arg_grp5.arg1, msg_p->data.arg_grp5.arg2);
            msgbuf_p->msg_size = LACP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LACP_MGR_IPC_SETDOT3ADAGGPORTACTORLACP_TIMEOUT:
            msg_p->type.ret_ui32 = LACP_MGR_SetDot3adAggPortActorLACP_Timeout(
                msg_p->data.arg_grp6.arg1, msg_p->data.arg_grp6.arg2,
                msg_p->data.arg_grp6.arg3);
            msgbuf_p->msg_size = LACP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LACP_MGR_IPC_GETDOT3ADAGGPORTACTORLACP_TIMEOUT:
            msg_p->type.ret_ui32 = LACP_MGR_GetDot3adAggPortActorLACP_Timeout(
                msg_p->data.arg_grp6.arg1, msg_p->data.arg_grp6.arg2,
                &msg_p->data.arg_grp6.arg3);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp6);
            break;

        case LACP_MGR_IPC_GETRUNNINGDOT3ADAGGPORTACTORLACP_TIMEOUT:
            msg_p->type.ret_ui32 = LACP_MGR_GetRunningDot3adAggPortActorLACP_Timeout(
                msg_p->data.arg_grp3.arg1, &msg_p->data.arg_grp3.arg2);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp3);
            break;

        case LACP_MGR_IPC_SETDOT3ADAGGACTORLACP_TIMEOUT:
            msg_p->type.ret_ui32 = LACP_MGR_SetDot3adAggActorLACP_Timeout(
                msg_p->data.arg_grp3.arg1,
                msg_p->data.arg_grp3.arg2);
            msgbuf_p->msg_size = LACP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LACP_MGR_IPC_GETDOT3ADAGGACTORLACP_TIMEOUT:
            msg_p->type.ret_ui32 = LACP_MGR_GetDot3adAggActorLACP_Timeout(
                msg_p->data.arg_grp3.arg1,
                &msg_p->data.arg_grp3.arg2);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp3);
            break;

        case LACP_MGR_IPC_GETRUNNINGDOT3ADAGGACTORLACP_TIMEOUT:
            msg_p->type.ret_ui32 = LACP_MGR_GetRunningDot3adAggActorLACP_Timeout(
                msg_p->data.arg_grp3.arg1, &msg_p->data.arg_grp3.arg2);
            msgbuf_p->msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp3);
            break;

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_ui32 = LACP_RETURN_ERROR;
            msgbuf_p->msg_size = LACP_MGR_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of LACP_MGR_HandleIPCReqMsg */

/*=============================================================================
 * Moved from lacp_task.c
 *=============================================================================
 */

/*------------------------------------------------------------------------
 * ROUTINE NAME - LACP_TASK_TimerTick
 *------------------------------------------------------------------------
 * FUNCTION: This function is what have to do in timer tick.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void LACP_TASK_TimerTick(void)
{
    UI32_T  slot, port;
    LACP_PORT_T *pPort;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    if (bLACP_DbgMsg)
    {
          BACKDOOR_MGR_Printf("\nLACP_TASK_TimerTick Event");
    }
    for( slot = 1; slot <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; slot++)
    {
        if(STKTPLG_POM_UnitExist( slot) == FALSE)
        {
            continue;
        }
        for( port = 1; port <= SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; port++)
        {
            if(STKTPLG_POM_PortExist( slot, port) == FALSE)
            {
                continue;
            }

            pPort = LACP_FIND_PORT( slot, port);

            if( (pPort->LACP_OperStatus == FALSE) ||
                (pPort->Port_OperStatus == FALSE) )
            {
                continue;
            }

            /* Only need to process timer event when LACP is "ENABLED". */
            /* When port is disabled, we should have another way.       */
            LACP_Rx_Machine      ( pPort, LACP_TIMER_EV, NULL);
            LACP_Mux_Machine     ( pPort, LACP_NEW_INFO_EV);
            LACP_Mux_Machine     ( pPort, LACP_TIMER_EV);
            LACP_Periodic_Machine( pPort, LACP_TIMER_EV);
            LACP_Tx_Machine      ( pPort, LACP_TIMER_EV);
        }
    }

    /* ckhsu 2001/12/04 comment                             */
    /* Every second, we have to reevaluate all ports in Mux */
    /* Re-evaluate all ports                                */
    LACP_Mux_Machine( NULL, LACP_MUX_REEVALUATE_EV);
}


/* created for replacing lacp_task */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_LacpduRcvd_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : Whenever Network Interface received a LACPDU packet,it calls
 *            this function to handle the packet.
 * INPUT    :
 *      L_MM_Mref_Handle_T *mref_handle_p   -- packet buffer and return buffer function pointer.
 *      UI8_T    *dst_mac   -- the destination MAC address of this packet.
 *      UI8_T    *src_mac   -- the source MAC address of this packet.
 *      UI16_T   tag_info   -- tag information
 *      UI16_T   type
 *      UI32_T   pkt_length -- the length of the packet payload.
 *      UI32_T   unit_no    -- user view unit number
 *      UI32_T   port_no    -- user view port number
 * RETURN   :   none
 * NOTE:
 *      This function is called at interrupt time, so it need to be fast.
 *      To reduce the processing time, this function just create a message
 *      that contain the buffer pointer and other packet information. Then
 *      it sends the message to the packet queue. And it sends an event to
 *      the LACP task to notify the arrival of the LACP packet. The LACP task
 *      will handle the packet after it get out the message from the packet
 *      message queue. It is the receiving routine's responsibility to call
 *      the L_MM_Mref_Release() function to free the receiving buffer.
 *-------------------------------------------------------------------------
 */
void LACP_MGR_LacpduRcvd_Callback(L_MM_Mref_Handle_T *mref_handle_p,
                                  UI8_T              dst_mac[6],
                                  UI8_T              src_mac[6],
                                  UI16_T             tag_info,
                                  UI16_T             type,
                                  UI32_T             pkt_length,
                                  UI32_T             unit_no,
                                  UI32_T             port_no)
{
    LACP_MSG_T             msg;
    LACP_PORT_T            *pPort = NULL;
    LACP_PORT_STATISTICS_T *p     = NULL;

    if (LACP_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    msg.msg_type = LACP_MSG_LACPDU;
    memcpy(msg.saddr, src_mac, 6);
    msg.mref_handle_p  = mref_handle_p;
    msg.unit_no  = (UI16_T)unit_no;
    msg.port_no  = (UI16_T)port_no;

    mref_handle_p->current_usr_id = SYS_MODULE_LACP; /* for L_MREF debugging purpose */

    if( (unit_no < 0) ||
        (unit_no > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK) ||
        (port_no < 0) ||
        (port_no > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT) )
    {
            if (bLACP_DbgMsg)
            {
                  BACKDOOR_MGR_Printf("\nError of LACP_TASK_AnnounceLacpPacket slot:%d port:%d", (int)unit_no, (int)port_no);
                  fflush(stdout);
            }
            L_MM_Mref_Release(&mref_handle_p);
            return;
    }
    else
    {
        pPort = LACP_FIND_PORT( unit_no, port_no);
        p     = &(pPort->AggPortStats);

        p->AnnouncePDUsRx++;
    }

    LACP_RX_HandlePacket(&msg);
    return;
}
