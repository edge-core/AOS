/*-----------------------------------------------------------------------------
 * Module Name: xstp_uty.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the XSTP utility
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    06/20/2001 - Allen Cheng, Created
 *    07/01/2002 - Kelly Chen, Added
 *    02-09-2004 - Kelly Chen, Revise the implementations of 802.1w/1s according to the IEEE 802.1D/D3
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include "sys_type.h"
#include "sys_time.h"
#include "sys_module.h"
#include "sys_dflt.h"
#include "snmp_pmgr.h"
#include "syslog_pmgr.h"
#include "l_mm.h"
#include "sysfun.h"
#include "l_stdlib.h"
#include "vlan_lib.h"
#include "vlan_om.h"
#include "vlan_pmgr.h"
#include "l2mux_mgr.h"
#include "amtr_pmgr.h"
#include "swctrl.h"
#include "swctrl_pmgr.h"
#include "xstp_type.h"
#include "xstp_engine.h"
#include "xstp_rx.h"
#include "xstp_om_private.h"
#include "xstp_uty.h"
#include "backdoor_mgr.h"
#include "sys_callback_mgr.h"  /*because the head file of SYS_CALLBACK_MGR_LportTcChangeCallback function does not included in the file */
#if(SYS_CPNT_AMTRL3 == TRUE)
#include "amtrl3_pmgr.h"
#endif
#include "trap_event.h"

#if (SYS_CPNT_DEBUG == TRUE)
#include "debug_mgr.h"
#include "debug_type.h"
#endif

#include "amtr_om.h"

static  BOOL_T  XSTP_UTY_ChangeStatePortListForbidden           = FALSE;
static  XSTP_TYPE_LportList_T *XSTP_UTY_ChangeStatePortList     = NULL;

static  BOOL_T  XSTP_UTY_MstEnable          = FALSE;
static  UI8_T   XSTP_UTY_BridgeGroupAddr[6] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x00};
#if 0
static  UI8_T   XSTP_UTY_InstanceVlansMapped[512];
#endif

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
static  XSTP_UTY_BpduGuardPortList_T *XSTP_UTY_BpduGuardRecoverPortList = NULL;
#endif

#if (SYS_CPNT_DEBUG == TRUE)
char   xstp_uty_str_tmp[100] = {0};
char   xstp_uty_str_cat[SYS_ADPT_MAX_NBR_OF_MST_INSTANCE*XSTP_TYPE_MST_CONFIG_MSG_LENGTH+150] = {0};
#endif

static  XSTP_TYPE_LportList_T    *XSTP_UTY_AddIntoLportList(XSTP_TYPE_LportList_T *list, UI32_T xstid, UI32_T lport, BOOL_T enter_forwarding);

static  void    XSTP_UTY_SendConfigBpdu(UI32_T lport, XSTP_TYPE_ConfigBpdu_T *bpdu);
static  void    XSTP_UTY_SendRstpBpdu(UI32_T lport, XSTP_TYPE_RstBpdu_T *bpdu);
static  void    XSTP_UTY_SendTcnBpdu(UI32_T lport);
static  void    XSTP_UTY_SendPacket(L_MM_Mref_Handle_T *mref_handle_p,
                                    UI8_T       dst_mac[6],
                                    UI8_T       src_mac[6],
                                    UI16_T      type,
                                    UI16_T      tag_info,
                                    UI32_T      packet_length,
                                    UI32_T      lport,
                                    BOOL_T      is_tagged,
                                    UI32_T      cos_value);
#ifdef XSTP_TYPE_PROTOCOL_MSTP
static UI16_T  XSTP_UTY_RcvInfoCist(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static UI16_T  XSTP_UTY_RcvInfoMsti(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static void  XSTP_UTY_RecordAgreementCist(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static void  XSTP_UTY_RecordAgreementMsti(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static void  XSTP_UTY_RecordMasteredCist(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static void  XSTP_UTY_RecordMasteredMsti(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static void  XSTP_UTY_RecordProposalCist(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static void  XSTP_UTY_RecordProposalMsti(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static void  XSTP_UTY_UpdtRcvdInfoWhileCist(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static void  XSTP_UTY_UpdtRcvdInfoWhileMsti(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static void  XSTP_UTY_UpdtRolesCist(XSTP_OM_InstanceData_T *om_ptr);
static void  XSTP_UTY_UpdtRolesMsti(XSTP_OM_InstanceData_T *om_ptr);
static  void XSTP_UTY_SendMstpBpdu(UI32_T lport, XSTP_TYPE_MstBpdu_T *bpdu);
static UI16_T  XSTP_UTY_GetVersion3length(UI32_T lport);
static void  XSTP_UTY_ChangedMaster(void);
#endif /* XSTP_TYPE_PROTOCOL_MSTP */
static  BOOL_T  XSTP_UTY_SetVlanPortState(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, BOOL_T forwarding);
static  void    XSTP_UTY_IncTopologyChangeCount(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static void    XSTP_UTY_RecordDispute(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
#if 0
static void    XSTP_UTY_SetRootBridge(XSTP_OM_InstanceData_T *om_ptr);
static void    XSTP_UTY_SetDesignatedPort(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
#endif

/* ===================================================================== */
/* ===================================================================== */
/* For both IEEE 802.1w and IEEE 802.1s */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_DisableForwarding
 * ------------------------------------------------------------------------
 * PURPOSE  : Disable forwarding
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.c, IEEE Std 802.1s(D14-1)-2002
 *            Ref to the description in 17.19.3, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_DisableForwarding(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    BOOL_T              state_changed;

    pom_ptr         = &(om_ptr->port_info[lport-1]);
    state_changed   = (pom_ptr->forwarding);

    /* Update forwarding flag before notify */
    pom_ptr->forwarding = FALSE;

    if (state_changed)
    {
       /*the vlan status may be not correct when xstp mode change*/

        /* +++ LeaveCriticalRegion +++ */
        XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

        /* Notify Leave_Forwarding */
        XSTP_UTY_NotifyLportLeaveForwarding(om_ptr->instance_id, lport);

        XSTP_UTY_SetVlanPortState(om_ptr, lport, FALSE);

        /* +++ EnterCriticalRegion +++ */
        XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    }

    return;
} /* End of XSTP_UTY_DisableForwarding */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_DisableLearning
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.3, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_DisableLearning(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_UTY_SetPortState(om_ptr, lport, XSTP_TYPE_PORT_STATE_BLOCKING);

    return;
} /* End of XSTP_UTY_DisableLearning */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_EnableForwarding
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.4, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_EnableForwarding(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;

    pom_ptr = &(om_ptr->port_info[lport-1]);

    if (XSTP_UTY_SetPortState(om_ptr, lport, XSTP_TYPE_PORT_STATE_FORWARDING) )
    {
        BOOL_T  state_changed;
        state_changed   = (!pom_ptr->forwarding);


        /* Update forwarding flag before notify */
        pom_ptr->forwarding = TRUE;

        if (state_changed)
        {
            pom_ptr->port_forward_transitions++;

            /* +++ LeaveCriticalRegion +++ */
            XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

            /* This notification only informs that at least one of the port enters forwarding state. */
            /* Trap */
            SNMP_PMGR_NotifyStaTplgStabled();
            SYSLOG_PMGR_NotifyStaTplgStabled();

            /* Notify Enter_Forwarding */
            XSTP_UTY_NotifyLportEnterForwarding(om_ptr->instance_id, lport);

            XSTP_UTY_SetVlanPortState(om_ptr, lport, TRUE);

            /* +++ EnterCriticalRegion +++ */
            XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
        }
    }

    return;
} /* End of XSTP_UTY_EnableForwarding */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_EnableLearning
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.5, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_EnableLearning(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_UTY_SetPortState(om_ptr, lport, XSTP_TYPE_PORT_STATE_LEARNING);

    return;
} /* End of XSTP_UTY_EnableLearning */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_Flush
 * ------------------------------------------------------------------------
 * PURPOSE  : remove all Dynamic Filtering Entries in the Filtering
 *            Database that contain information learned on this port,
 *            unless this Port is an edge Port.
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.6, IEEE Std 802.1w-2001, and
 *            the description in 13.26.f, IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_Flush(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;

    pom_ptr     = &(om_ptr->port_info[lport-1]);

    if(pom_ptr->common->link_up == FALSE)
    {
        return;
    }

    if (!pom_ptr->common->oper_edge)
    {
        #if (SYS_CPNT_XSTP_MSTP_DELETE_MAC_ONLY_BY_PORT == FALSE) /*clear by vlan cause too much time when the vlan no. is to large. just clear by port and relearn the mac.*/
        if (XSTP_UTY_MstEnable)
        {
            /* +++ LeaveCriticalRegion +++ */
            XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

            AMTR_PMGR_DeleteAddrByLifeTimeAndMstIDAndLPort(lport, om_ptr->instance_id,AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);
#if(SYS_CPNT_AMTRL3 == TRUE)
            AMTRL3_PMGR_MACTableDeleteByMstidOnPort(om_ptr->instance_id, lport);
#endif

            /* +++ EnterCriticalRegion +++ */
            XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);

            if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
            {
                BACKDOOR_MGR_Printf("\r\nXSTP_UTY_Flush:: clear dynamic address table entries at lport = %ld, mstid = %ld", lport, om_ptr->instance_id);
            }
        }
        else
        #endif
        {
            #if (SYS_CPNT_XSTP_MSTP_DELETE_MAC_ONLY_BY_PORT == TRUE)
            if (AMTR_OM_GetDynCounterByPort(lport) == 0)
                return;
            #endif

            /* +++ LeaveCriticalRegion +++ */
            XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

            AMTR_PMGR_DeleteAddrByLifeTimeAndLPort(lport, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);

            /* +++ EnterCriticalRegion +++ */
            XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);

            if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
            {
                BACKDOOR_MGR_Printf("\r\nXSTP_UTY_Flush:: clear dynamic address table entries at lport = %ld", lport);
            }
        }
    }
    return;
} /* End of XSTP_UTY_Flush */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_FlushPort
 *-------------------------------------------------------------------------
 * PURPOSE  : Flush port's MAC dispite of VLAN according to BPDU's flag
 * INPUT    : bpdu_msg_ptr  -- BPDU message pointer
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : none
 * ------------------------------------------------------------------------
 */
static void XSTP_UTY_FlushPort(UI32_T lport, XSTP_TYPE_MSG_T *bpdu_msg_ptr)
{
    XSTP_TYPE_RstBpdu_T     *bpdu;
    UI32_T pdu_len;
    BOOL_T                  rcvd_tc = FALSE;
    BOOL_T                  rcvd_tcn = FALSE;

    bpdu = (XSTP_TYPE_RstBpdu_T*) L_MM_Mref_GetPdu(bpdu_msg_ptr->mref_handle_p, &pdu_len);
    if (bpdu == NULL)
    {
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
        {
            printf("\r\n%s: L_MM_Mref_GetPdu failed", __FUNCTION__);
        }
        return;
    }

    switch (bpdu->bpdu_type)
    {
        case XSTP_TYPE_BPDU_TYPE_XST:
        case XSTP_TYPE_BPDU_TYPE_CONFIG:
            rcvd_tc        = ( (bpdu->flags & XSTP_TYPE_BPDU_FLAGS_TC)     != 0);
            break;

        case XSTP_TYPE_BPDU_TYPE_TCN:
            rcvd_tcn   = TRUE;
            break;
    }

    if (rcvd_tc || rcvd_tcn)
    {
         AMTR_PMGR_DeleteAddrByLifeTimeAndLPort(lport, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);

        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
            printf("\r\nFlush MAC on STP disabled lport %ld for rcv TC", (long)lport);
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetChangeStatePortListForbidden
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will set the flag which controls whether the
 *            XSTP_UTY_ChangeStatePortList is allowed to be added new
 *            element.
 * INPUT    : flag    -- TRUE:disallowed, FALSE:allowed
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetChangeStatePortListForbidden(BOOL_T flag)
{
    XSTP_TYPE_LportList_T *this_lport;

    XSTP_UTY_ChangeStatePortListForbidden = flag;

    if (XSTP_UTY_ChangeStatePortListForbidden)
    {
        while (XSTP_UTY_ChangeStatePortList != NULL)
        {   /* release all allocated memory of the list */
            this_lport = XSTP_UTY_ChangeStatePortList;
            XSTP_UTY_ChangeStatePortList = XSTP_UTY_ChangeStatePortList->next;
            L_MM_Free((void*)this_lport);
        } /* end of while */
    } /* end of if */
} /* End of XSTP_UTY_SetChangeStatePortListControlFlag */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RetrieveChangeStateLportList
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will retrieve the lport list which enters/leaves
 *            the forwarding state. Then the XSTP_UTY_ChangeStatePortList
 *            is cleared.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : XSTP_UTY_ChangeStatePortList
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
XSTP_TYPE_LportList_T    *XSTP_UTY_RetrieveChangeStateLportList(void)
{
    XSTP_TYPE_LportList_T    *lport_list;

    lport_list  = XSTP_UTY_ChangeStatePortList;
    XSTP_UTY_ChangeStatePortList    = NULL;

    return lport_list;
} /* End of XSTP_UTY_RetrieveChangeStateLportList */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_NotifyLportEnterForwarding
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will record the lport number which enters the
 *            the forwarding state.
 * INPUT    : xstid             -- index of the spanning tree
 *            lport             -- lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_NotifyLportEnterForwarding(UI32_T xstid, UI32_T lport)
{
    XSTP_OM_InstanceData_T *om_ptr;
    XSTP_OM_PortVar_T   *pom_ptr;

    om_ptr  = XSTP_OM_GetInstanceInfoPtr(xstid);
    pom_ptr     = &(om_ptr->port_info[lport-1]);

    if(pom_ptr->common->port_spanning_tree_status != VAL_staPortSystemStatus_disabled
       &&XSTP_OM_GetSpanningTreeStatus() == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
    {
        TRAP_EVENT_TrapData_T trap_data;

        trap_data.trap_type = TRAP_EVENT_XSTP_PORT_STATE_CHANGE;
        trap_data.community_specified = FALSE;
        trap_data.flag = TRAP_EVENT_SEND_TRAP_OPTION_DEFAULT;
        trap_data.u.xstp_port_state_change.mstid = xstid;
        trap_data.u.xstp_port_state_change.lport = lport;
        trap_data.u.xstp_port_state_change.forwarding = TRUE;
        SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_ONLY);
    }

    XSTP_UTY_ChangeStatePortList    = XSTP_UTY_AddIntoLportList(XSTP_UTY_ChangeStatePortList, xstid, lport, TRUE);

    SYS_CALLBACK_MGR_LportEnterForwardingCallback(SYS_MODULE_XSTP, xstid, lport);

} /* End of XSTP_UTY_NotifyLportEnterForwarding */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_NotifyLportLeaveForwarding
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will record the lport number which leaves the
 *            forwarding state.
 * INPUT    : xstid             -- index of the spanning tree
 *            lport             -- lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_NotifyLportLeaveForwarding(UI32_T xstid, UI32_T lport)
{
    XSTP_OM_InstanceData_T *om_ptr;
    XSTP_OM_PortVar_T   *pom_ptr;

    om_ptr  = XSTP_OM_GetInstanceInfoPtr(xstid);
    pom_ptr     = &(om_ptr->port_info[lport-1]);

    if(pom_ptr->common->port_spanning_tree_status != VAL_staPortSystemStatus_disabled
       &&XSTP_OM_GetSpanningTreeStatus() == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
    {
        TRAP_EVENT_TrapData_T trap_data;

        trap_data.trap_type = TRAP_EVENT_XSTP_PORT_STATE_CHANGE;
        trap_data.community_specified = FALSE;
        trap_data.flag = TRAP_EVENT_SEND_TRAP_OPTION_DEFAULT;
        trap_data.u.xstp_port_state_change.mstid = xstid;
        trap_data.u.xstp_port_state_change.lport = lport;
        trap_data.u.xstp_port_state_change.forwarding = FALSE;
        SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_ONLY);
    }

    XSTP_UTY_ChangeStatePortList    = XSTP_UTY_AddIntoLportList(XSTP_UTY_ChangeStatePortList, xstid, lport, FALSE);

    SYS_CALLBACK_MGR_LportLeaveForwardingCallback(SYS_MODULE_XSTP, xstid, lport);

} /* End of XSTP_UTY_NotifyLportLeaveForwarding */

/* ===================================================================== */
/* ===================================================================== */
/* For IEEE 802.1w */
#ifdef XSTP_TYPE_PROTOCOL_RSTP

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_Cist
 * ------------------------------------------------------------------------
 * PURPOSE  : TRUE only for CIST state machines, i.e. FALSE for MSTI state
 *            machine instances.
 * INPUT    : om_ptr        -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref to the description in 13.25.2, IEEE Std 802.1s(D13)-2002.
 *            TRUE returned for RSTP
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_Cist(XSTP_OM_InstanceData_T *om_ptr)
{
    return TRUE;
} /* End of XSTP_UTY_Cist */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_ClearReselectBridge
 * ------------------------------------------------------------------------
 * PURPOSE  : Clear reselect bridge
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.1, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_ClearReselectBridge(XSTP_OM_InstanceData_T *om_ptr)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI32_T              lport;

    /* Performance Improvement
    for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
    */
    lport = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr     = &(om_ptr->port_info[lport-1]);
        if (pom_ptr->is_member)
        {
            pom_ptr->reselect = FALSE;
        }
    }

    return;
} /* End of XSTP_UTY_ClearReselectBridge */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_NewTcWhile
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport
 * OUTPUT   : tc_while  -- topology change timer
 * RETURN   : None
 * NOTE     : Ref to the description in 17.21.7, IEEE Std 802.1D-2004
 *-------------------------------------------------------------------------
 */
UI16_T  XSTP_UTY_NewTcWhile(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI16_T              tc_while;

    XSTP_UTY_IncTopologyChangeCount(om_ptr, lport);

    pom_ptr     = &(om_ptr->port_info[lport-1]);

    if(pom_ptr->tc_while == 0)
    {
    if (    (pom_ptr->common->oper_point_to_point_mac)
        &&  (pom_ptr->common->send_rstp)
       )
    {
            tc_while    = pom_ptr->designated_times.hello_time + 1;
            pom_ptr->new_info     = TRUE;
    }
    else
    {
        tc_while    =   om_ptr->bridge_info.root_times.max_age
                      + om_ptr->bridge_info.root_times.forward_delay;
        }
    }
    else
    {
        tc_while = pom_ptr->tc_while;
    }
    return tc_while;
} /* End of XSTP_UTY_NewTcWhile */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_NewEdgeDelayWhile
 * ------------------------------------------------------------------------
 * PURPOSE  : Return the edge delay upon a port's link type.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport
 * OUTPUT   : edge_delay_while  -- edge delay timer
 * RETURN   : None
 * NOTE     : Ref to the description in 17.20.4 IEEE Std 802.1D-2004
 *-------------------------------------------------------------------------
 */
UI16_T  XSTP_UTY_NewEdgeDelayWhile(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI16_T              edge_delay_while;

    pom_ptr     = &(om_ptr->port_info[lport-1]);

    if (pom_ptr->common->oper_point_to_point_mac == TRUE)
        edge_delay_while = XSTP_TYPE_DEFAULT_MIGRATE_TIME;
    else
        edge_delay_while = pom_ptr->designated_times.max_age;

    return edge_delay_while;
}

#if 0
/* Old implementation: patch condition when port_role is root port */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RcvBpdu
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. Ref to the description in 17.19.8, IEEE Std 802.1w-2001
 *               Ref to the description in 17.21.8, IEEE Std 802.1D/D3-2003
 *            2. Patch condition when port_role is root port
 *               -- Only compare root_bridge_id and root_path_cost componments.
 *-------------------------------------------------------------------------
 */
UI16_T  XSTP_UTY_RcvBpdu(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_BpduHeader_T  *bpdu_header;
    XSTP_TYPE_RstBpdu_T     *bpdu;
    UI8_T                   bpdu_type;
    UI8_T                   port_role;
    int                     cmp_a, cmp_b, cmp_c, cmp_d, cmp_e;
    UI16_T                  result;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    bpdu_header = (XSTP_TYPE_BpduHeader_T*)pom_ptr->common->bpdu;
    bpdu_type   = bpdu_header->bpdu_type;

    switch (bpdu_type)
    {
        case XSTP_TYPE_BPDU_TYPE_XST:
            bpdu        = (XSTP_TYPE_RstBpdu_T*)pom_ptr->common->bpdu;
            port_role   = bpdu->flags & XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_MASK;
            break;

        case XSTP_TYPE_BPDU_TYPE_CONFIG:
        case XSTP_TYPE_BPDU_TYPE_TCN:
            port_role   = XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_UNKNOWN;
            break;
    }

    XSTP_OM_CMP_PRIORITY_VECTOR(cmp_a, pom_ptr->msg_priority, pom_ptr->port_priority);
    XSTP_OM_CMP_TIMERS(cmp_b, pom_ptr->msg_times, pom_ptr->port_times);
    XSTP_OM_CMP_BRIDGE_ID(cmp_c, pom_ptr->msg_priority.designated_bridge_id, pom_ptr->port_priority.designated_bridge_id);
    XSTP_OM_CMP_PORT_ID(cmp_d, pom_ptr->msg_priority.designated_port_id, pom_ptr->port_priority.designated_port_id);
    XSTP_OM_CMP_BRIDGE_ID(cmp_e, pom_ptr->msg_priority.root_bridge_id, pom_ptr->port_priority.root_bridge_id);

    if (    (   (bpdu_type == XSTP_TYPE_BPDU_TYPE_XST)
             && (   (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED)
                 || (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_UNKNOWN)   /* 9.2.9 */
                )
            )
         || (bpdu_type == XSTP_TYPE_BPDU_TYPE_CONFIG)
       )
    {
        if (    (cmp_a < 0)
/* Follow IEEE P802.1D/D3, page 159, 17.21.8 rcvinfo().
             || (   (cmp_c == 0)
                 && (cmp_d == 0)
                 && (   (cmp_a != 0)
                     || (cmp_b != 0)
                    )
                )
*/
             || (   (cmp_a == 0)
                 && (cmp_b != 0)
                )
           )
        {
            result  = XSTP_ENGINE_PORTVAR_RCVD_MSG_SUPERIOR_DESIGNATED_MSG;
        }
        else
        if (    (cmp_a == 0)
             && (cmp_b == 0)
           )
        {
            result  = XSTP_ENGINE_PORTVAR_RCVD_MSG_REPEATED_DESIGNATED_MSG;
        }
        else
        {
            /* Follow IEEE P802.1D/D3, page 159, 17.21.8 rcvinfo().*/
            if (    (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED)
                 && (cmp_a > 0)
               )
            {
                XSTP_UTY_RecordDispute(om_ptr, lport);
            }
            result  = XSTP_ENGINE_PORTVAR_RCVD_MSG_OTHER_MSG;
        }
    }
    else
    if (    (pom_ptr->common->oper_point_to_point_mac)
         && (   (bpdu_type == XSTP_TYPE_BPDU_TYPE_XST)
             && (   (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ROOT)
                 || (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ALT_BAK)
                )
             && (   (cmp_e > 0)
                 || (   (cmp_e == 0)
                     && ((pom_ptr->msg_priority.root_path_cost) >= (pom_ptr->port_priority.root_path_cost))
                    )
                )
            )
         && ( (bpdu->flags & XSTP_TYPE_BPDU_FLAGS_AGREEMENT) != 0)
       )
    {
        result  = XSTP_ENGINE_PORTVAR_RCVD_MSG_CONFIRMED_ROOT_MSG;
    }
    else
    {
        /* Follow IEEE P802.1D/D3, page 159, 17.21.8 rcvinfo().*/
        if (    (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED)
             && (cmp_a > 0)
           )
        {
            XSTP_UTY_RecordDispute(om_ptr, lport);
        }
        result  = XSTP_ENGINE_PORTVAR_RCVD_MSG_OTHER_MSG;
    }

    return result;
} /* End of XSTP_UTY_RcvBpdu */
#endif

/* New implementation according to IEEE Std 802.1D/D3 June 11, 2003, page 159.
   -- XSTP_UTY_RcvBpdu()
*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RcvBpdu
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. Ref to the description in 17.19.8, IEEE Std 802.1w-2001
 *               Ref to the description in 17.21.8, IEEE Std 802.1D/D3-2003
 *-------------------------------------------------------------------------
 */
UI16_T  XSTP_UTY_RcvBpdu(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_BpduHeader_T  *bpdu_header;
    XSTP_TYPE_RstBpdu_T     *bpdu;
    UI8_T                   bpdu_type;
    UI8_T                   port_role;
    int                     cmp_a, cmp_b, cmp_c, cmp_d, cmp_e;
    UI16_T                  result;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    bpdu_header = (XSTP_TYPE_BpduHeader_T*)pom_ptr->common->bpdu;
    bpdu_type   = bpdu_header->bpdu_type;

    switch (bpdu_type)
    {
        case XSTP_TYPE_BPDU_TYPE_XST:
            bpdu        = (XSTP_TYPE_RstBpdu_T*)pom_ptr->common->bpdu;
            port_role   = bpdu->flags & XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_MASK;
            break;

        case XSTP_TYPE_BPDU_TYPE_CONFIG:
        case XSTP_TYPE_BPDU_TYPE_TCN:
            port_role   = XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_UNKNOWN;
            break;
    }

    XSTP_OM_CMP_PRIORITY_VECTOR(cmp_a, pom_ptr->msg_priority, pom_ptr->port_priority);
    XSTP_OM_CMP_TIMERS(cmp_b, pom_ptr->msg_times, pom_ptr->port_times);
    XSTP_OM_CMP_BRIDGE_ID(cmp_c, pom_ptr->msg_priority.designated_bridge_id, pom_ptr->port_priority.designated_bridge_id);
    XSTP_OM_CMP_PORT_ID(cmp_d, pom_ptr->msg_priority.designated_port_id, pom_ptr->port_priority.designated_port_id);
    cmp_e = memcmp( &pom_ptr->msg_priority.designated_bridge_id.addr,
                    &pom_ptr->port_priority.designated_bridge_id.addr,
                    6
                  );

    if (    (   (bpdu_type == XSTP_TYPE_BPDU_TYPE_XST)
             && (   (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED)
                 || (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_UNKNOWN)   /* 9.2.9 */
                )
            )
         || (bpdu_type == XSTP_TYPE_BPDU_TYPE_CONFIG)
       )
    {
        if (    (cmp_a < 0)
             || (   (cmp_e == 0)        /* 17.4.2.2 */
                 && (pom_ptr->msg_priority.designated_port_id.data.port_num == pom_ptr->port_priority.designated_port_id.data.port_num)
                 && (   (cmp_a != 0)
                     || (cmp_b != 0)
                    )
                )
             || (   (cmp_a == 0)
                 && (cmp_b != 0)
                )
           )
        {
            result  = XSTP_ENGINE_PORTVAR_RCVD_MSG_SUPERIOR_DESIGNATED_MSG;
        }
        else
        if (    (cmp_a == 0)
             && (cmp_b == 0)
           )
        {
            result  = XSTP_ENGINE_PORTVAR_RCVD_MSG_REPEATED_DESIGNATED_MSG;
        }
        else
        {
            /* Follow IEEE P802.1D/D3, page 159, 17.21.8 rcvinfo().*/
            if (    (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED)
                 && (cmp_a > 0)
               )
            {
                XSTP_UTY_RecordDispute(om_ptr, lport);
            }
            result  = XSTP_ENGINE_PORTVAR_RCVD_MSG_OTHER_MSG;
        }
    }
    else
    if (    (pom_ptr->common->oper_point_to_point_mac)
         && (   (bpdu_type == XSTP_TYPE_BPDU_TYPE_XST)
             && (   (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ROOT)
                 || (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ALT_BAK)
                )
             && (cmp_a >= 0)
            )
         && ( (bpdu->flags & XSTP_TYPE_BPDU_FLAGS_AGREEMENT) != 0)
       )
    {
        result  = XSTP_ENGINE_PORTVAR_RCVD_MSG_CONFIRMED_ROOT_MSG;
    }
    else
    {
        /* Follow IEEE P802.1D/D3, page 159, 17.21.8 rcvinfo().*/
        if (    (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED)
             && (cmp_a > 0)
           )
        {
            XSTP_UTY_RecordDispute(om_ptr, lport);
        }
        result  = XSTP_ENGINE_PORTVAR_RCVD_MSG_OTHER_MSG;
    }

    return result;
} /* End of XSTP_UTY_RcvBpdu */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RecordProposed
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.9, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_RecordProposed(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_RstBpdu_T     *bpdu;
    BOOL_T                  result;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    bpdu        = (XSTP_TYPE_RstBpdu_T*)pom_ptr->common->bpdu;

    if (    (bpdu->bpdu_type == XSTP_TYPE_BPDU_TYPE_XST)
         && ( (bpdu->flags & XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED) != 0)
         && ( (bpdu->flags & XSTP_TYPE_BPDU_FLAGS_PROPOSAL) != 0)
         && (pom_ptr->common->oper_point_to_point_mac)
       )
    {
        result  = TRUE;
    }
    else
    {
        result  = FALSE;
    }

    return result;
} /* End of XSTP_UTY_RecordProposed */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetSyncBridge
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.10, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetSyncBridge(XSTP_OM_InstanceData_T *om_ptr)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI32_T              lport;

    /* Performance Improvement
    for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
    */
    lport = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr     = &(om_ptr->port_info[lport-1]);
        if (pom_ptr->is_member)
        {
            pom_ptr->sync = TRUE;
        }
    }

    return;
} /* End of XSTP_UTY_SetSyncBridge */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetReRootBridge
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.11, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetReRootBridge(XSTP_OM_InstanceData_T *om_ptr)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI32_T              lport;

    /* Performance Improvement
    for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
    */
    lport = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr     = &(om_ptr->port_info[lport-1]);
        if (pom_ptr->is_member)
        {
            pom_ptr->re_root = TRUE;
        }
    }

    return;
} /* End of XSTP_UTY_SetReRootBridge */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetSelectedBridge
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.12, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetSelectedBridge(XSTP_OM_InstanceData_T *om_ptr)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI32_T              lport;

    if (!XSTP_UTY_ReselectForAnyPort(om_ptr))
    {
        /* Performance Improvement
        for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
        */
        lport = 0;
        while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
        {
            pom_ptr     = &(om_ptr->port_info[lport-1]);
            if (pom_ptr->is_member)
            {
                pom_ptr->selected = TRUE;
            }
        }
    }

    return;
} /* End of XSTP_UTY_SetSelectedBridge */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetTcFlags
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.13, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetTcFlags(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_BpduHeader_T  *bpdu_header;
    XSTP_TYPE_RstBpdu_T     *bpdu;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    bpdu_header = (XSTP_TYPE_BpduHeader_T*)pom_ptr->common->bpdu;
    bpdu        = (XSTP_TYPE_RstBpdu_T*)pom_ptr->common->bpdu;

    switch (bpdu_header->bpdu_type)
    {
        case XSTP_TYPE_BPDU_TYPE_XST:
        case XSTP_TYPE_BPDU_TYPE_CONFIG:
            pom_ptr->rcvd_tc        = ( (bpdu->flags & XSTP_TYPE_BPDU_FLAGS_TC)     != 0);
            pom_ptr->rcvd_tc_ack    = ( (bpdu->flags & XSTP_TYPE_BPDU_FLAGS_TCA)    != 0);
            break;

        case XSTP_TYPE_BPDU_TYPE_TCN:
            pom_ptr->rcvd_tcn   = TRUE;
            break;
    }

    return;
} /* End of XSTP_UTY_SetTcFlags */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetTcPropBridge
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport (1..max)
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.14, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetTcPropBridge(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI32_T              next_lport;
#if (SYS_CPNT_XSTP_TC_PROP_GROUP==TRUE)
    UI32_T tc_group_id;
#endif

#if (SYS_CPNT_XSTP_TC_PROP_GROUP==TRUE)
    tc_group_id = XSTP_OM_GetPropGropIdByPort(lport);
#endif

    /* Performance Improvement
    for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
    */
    next_lport = 0;
    while (SWCTRL_GetNextLogicalPort(&next_lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr     = &(om_ptr->port_info[next_lport-1]);
        if ( (next_lport != lport) && pom_ptr->is_member
#if (SYS_CPNT_XSTP_TC_PROP_STOP == TRUE)
            && !XSTP_OM_IsPortTcPropStop(next_lport)
#endif
#if (SYS_CPNT_XSTP_TC_PROP_GROUP==TRUE)
            &&(tc_group_id == XSTP_OM_GetPropGropIdByPort(next_lport))
#endif
           )
        {
            pom_ptr->tc_prop = TRUE;
        }
    }
    return;
} /* End of XSTP_UTY_SetTcPropBridge */

#if 0
/* Old implementation according to IEEE Std 802.1w-2001 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_TxConfig
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.15, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_TxConfig(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_ConfigBpdu_T  bpdu;

    pom_ptr     = &(om_ptr->port_info[lport-1]);

    /* Priority Vector */
    XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu.root_identifier, pom_ptr->port_priority.root_bridge_id);
    bpdu.root_path_cost = L_STDLIB_Hton32(pom_ptr->port_priority.root_path_cost);
    XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu.bridge_identifier, pom_ptr->port_priority.designated_bridge_id);
    bpdu.port_identifier.port_id = L_STDLIB_Hton16(pom_ptr->port_priority.designated_port_id.port_id);

    /* Flags */
    bpdu.flags  = 0;
    if (pom_ptr->tc_while != 0)
    {
        bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_TC;
    }
    if (pom_ptr->tc_ack)
    {
        bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_TCA;
    }

    /* Timers */
    bpdu.message_age    = L_STDLIB_Hton16(pom_ptr->port_times.message_age   * XSTP_TYPE_BPDU_TIME_UNIT);
    bpdu.max_age        = L_STDLIB_Hton16(pom_ptr->port_times.max_age       * XSTP_TYPE_BPDU_TIME_UNIT);
    bpdu.hello_time     = L_STDLIB_Hton16(pom_ptr->port_times.hello_time    * XSTP_TYPE_BPDU_TIME_UNIT);
    bpdu.forward_delay  = L_STDLIB_Hton16(pom_ptr->port_times.forward_delay * XSTP_TYPE_BPDU_TIME_UNIT);

    XSTP_UTY_SendConfigBpdu(lport, &bpdu);

    return;
} /* End of XSTP_UTY_TxConfig */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_TxRstp
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.16, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_TxRstp(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_RstBpdu_T     bpdu;

    pom_ptr     = &(om_ptr->port_info[lport-1]);

    /* Priority Vector */
    XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu.root_identifier, pom_ptr->port_priority.root_bridge_id);
    bpdu.root_path_cost = L_STDLIB_Hton32(pom_ptr->port_priority.root_path_cost);
    XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu.bridge_identifier, pom_ptr->port_priority.designated_bridge_id);
    bpdu.port_identifier.port_id = L_STDLIB_Hton16(pom_ptr->port_priority.designated_port_id.port_id);

    /* Flags */
    bpdu.flags  = 0;
    switch (pom_ptr->role)
    {
        case XSTP_ENGINE_PORTVAR_ROLE_ROOT:
            bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ROOT;
            break;
        case XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED:
            bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED;
            break;
        case XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE:
        case XSTP_ENGINE_PORTVAR_ROLE_BACKUP:
            bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ALT_BAK;
            break;
    }
    if (pom_ptr->synced)
    {
        bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_AGREEMENT;
    }
    if (pom_ptr->proposing)
    {
        bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_PROPOSAL;
    }
    if (pom_ptr->tc_while != 0)
    {
        bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_TC;
    }
    if (pom_ptr->forwarding)
    {
        bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_FORWARDING;
    }
    if (pom_ptr->learning)
    {
        bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_LEARNING;
    }

    /* Timers */
    bpdu.message_age    = L_STDLIB_Hton16(pom_ptr->port_times.message_age   * XSTP_TYPE_BPDU_TIME_UNIT);
    bpdu.max_age        = L_STDLIB_Hton16(pom_ptr->port_times.max_age       * XSTP_TYPE_BPDU_TIME_UNIT);
    bpdu.hello_time     = L_STDLIB_Hton16(pom_ptr->port_times.hello_time    * XSTP_TYPE_BPDU_TIME_UNIT);
    bpdu.forward_delay  = L_STDLIB_Hton16(pom_ptr->port_times.forward_delay * XSTP_TYPE_BPDU_TIME_UNIT);

    /* Length */
    bpdu.version_1_length   = 0x00;

    XSTP_UTY_SendRstpBpdu(lport, &bpdu);

    return;
} /* End of XSTP_UTY_TxRstp */
#endif

/* New implementation according to IEEE Std 802.1D/D3 June 11, 2003, page 161.
   -- XSTP_UTY_TxConfig()
   -- XSTP_UTY_TxRstp()
*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_TxConfig
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.21.19, IEEE Std 802.1D/D3 June 11, 2003
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_TxConfig(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_ConfigBpdu_T  bpdu;

    pom_ptr     = &(om_ptr->port_info[lport-1]);

    /* Priority Vector: designated priority vector. Follow 1D/D3, page 161 */
    XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu.root_identifier, pom_ptr->designated_priority.root_bridge_id);
    bpdu.root_path_cost = L_STDLIB_Hton32(pom_ptr->designated_priority.root_path_cost);
    XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu.bridge_identifier, pom_ptr->designated_priority.designated_bridge_id);
    bpdu.port_identifier.port_id = L_STDLIB_Hton16(pom_ptr->designated_priority.designated_port_id.port_id);

    /* Flags */
    bpdu.flags  = 0;
    if (pom_ptr->tc_while != 0)
    {
        bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_TC;
    }
    if (pom_ptr->tc_ack)
    {
        bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_TCA;
    }

    /* Timers: designated_times. Follow 1D/D3, page 161 */
    bpdu.message_age    = L_STDLIB_Hton16(pom_ptr->designated_times.message_age   * XSTP_TYPE_BPDU_TIME_UNIT);
    bpdu.max_age        = L_STDLIB_Hton16(pom_ptr->designated_times.max_age       * XSTP_TYPE_BPDU_TIME_UNIT);
    bpdu.hello_time     = L_STDLIB_Hton16(pom_ptr->designated_times.hello_time    * XSTP_TYPE_BPDU_TIME_UNIT);
    bpdu.forward_delay  = L_STDLIB_Hton16(pom_ptr->designated_times.forward_delay * XSTP_TYPE_BPDU_TIME_UNIT);

    XSTP_UTY_SendConfigBpdu(lport, &bpdu);

    return;
} /* End of XSTP_UTY_TxConfig */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_TxRstp
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.21.20, IEEE Std 802.1D/D3 June 11, 2003
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_TxRstp(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_RstBpdu_T     bpdu;

    pom_ptr     = &(om_ptr->port_info[lport-1]);

    /* Priority Vector: designated priority vector. Follow 1D/D3, page 161 */
    XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu.root_identifier, pom_ptr->designated_priority.root_bridge_id);
    bpdu.root_path_cost = L_STDLIB_Hton32(pom_ptr->designated_priority.root_path_cost);
    XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu.bridge_identifier, pom_ptr->designated_priority.designated_bridge_id);
    bpdu.port_identifier.port_id = L_STDLIB_Hton16(pom_ptr->designated_priority.designated_port_id.port_id);

    /* Flags */
    bpdu.flags  = 0;
    switch (pom_ptr->role)
    {
        case XSTP_ENGINE_PORTVAR_ROLE_ROOT:
            bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ROOT;
            break;
        case XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED:
            bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED;
            break;
        case XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE:
        case XSTP_ENGINE_PORTVAR_ROLE_BACKUP:
            bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ALT_BAK;
            break;
    }
    if (pom_ptr->synced)
    {
        bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_AGREEMENT;
    }
    if (pom_ptr->proposing)
    {
        bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_PROPOSAL;
    }
    if (pom_ptr->tc_while != 0)
    {
        bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_TC;
    }
    if (pom_ptr->forwarding)
    {
        bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_FORWARDING;
    }
    if (pom_ptr->learning)
    {
        bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_LEARNING;
    }

    /* Timers: designated_times. Follow 1D/D3, page 161 */
    bpdu.message_age    = L_STDLIB_Hton16(pom_ptr->designated_times.message_age   * XSTP_TYPE_BPDU_TIME_UNIT);
    bpdu.max_age        = L_STDLIB_Hton16(pom_ptr->designated_times.max_age       * XSTP_TYPE_BPDU_TIME_UNIT);
    bpdu.hello_time     = L_STDLIB_Hton16(pom_ptr->designated_times.hello_time    * XSTP_TYPE_BPDU_TIME_UNIT);
    bpdu.forward_delay  = L_STDLIB_Hton16(pom_ptr->designated_times.forward_delay * XSTP_TYPE_BPDU_TIME_UNIT);

    /* Length */
    bpdu.version_1_length   = 0x00;

    XSTP_UTY_SendRstpBpdu(lport, &bpdu);

    return;
} /* End of XSTP_UTY_TxRstp */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_TxTcn
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.17, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_TxTcn(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_UTY_SendTcnBpdu(lport);

    return;
} /* End of XSTP_UTY_TxTcn */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtBpduVersion
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.18, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_UpdtBpduVersion(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_BpduHeader_T  *bpdu_header;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    bpdu_header = (XSTP_TYPE_BpduHeader_T*)pom_ptr->common->bpdu;

    switch (bpdu_header->bpdu_type)
    {
        case XSTP_TYPE_BPDU_TYPE_XST:
            if (    (   ((XSTP_TYPE_RstBpdu_T*)(pom_ptr->common->bpdu))->flags
                     &  XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_MASK
                    )
                ==  XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_UNKNOWN
               )
            {
                /* 9.2.9 */
                pom_ptr->rcvd_stp   = TRUE;
            }
            else
            if (    (bpdu_header->protocol_version_identifier >= XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID)
                 && (XSTP_OM_GetForceVersion() >= XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID)
               )
            {
                pom_ptr->rcvd_rstp  = TRUE;
            }

            break;

        case XSTP_TYPE_BPDU_TYPE_CONFIG:
        case XSTP_TYPE_BPDU_TYPE_TCN:
            if (bpdu_header->protocol_version_identifier < XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID)
            {
                pom_ptr->rcvd_stp   = TRUE;
            }
            break;
    }


    return;
} /* End of XSTP_UTY_UpdtBpduVersion */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtRcvdInfoWhile
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.21.23, IEEE Std 802.1D-2004
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_UpdtRcvdInfoWhile(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI16_T              effective_age, increment, max_age, hello_time;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    hello_time  = pom_ptr->port_times.hello_time;
    max_age     = pom_ptr->port_times.max_age;
    increment   = 1;

    /* Follow IEEE Std 802.1D-2004, page 168, 17.21.23 updtRcvdInfoWhile().
       rcvdInfoWhile is the three times the Hello Time, if Message Age,
       incremented by 1 second and rounded to the nearest whole second,
       does not exceed Max Age, and is zero otherwise.
    */
    effective_age   = pom_ptr->port_times.message_age + increment;

    if (effective_age <= pom_ptr->port_times.max_age)
    {
        pom_ptr->rcvd_info_while    = 3 * hello_time;
    }
    else
    {
        pom_ptr->rcvd_info_while    = 0;
    }

    return;
} /* End of XSTP_UTY_UpdtRcvdInfoWhile */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtRoleDisabledBridge
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.20, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_UpdtRoleDisabledBridge(XSTP_OM_InstanceData_T *om_ptr)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI32_T              lport;

    /* Performance Improvement
    for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
    */
    lport = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr     = &(om_ptr->port_info[lport-1]);
        if (pom_ptr->is_member)
        {
            pom_ptr->selected_role = XSTP_ENGINE_PORTVAR_ROLE_DISABLED;
        }
    }

    return;
} /* End of XSTP_UTY_UpdtRoleDisabledBridge */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtRolesBridge
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.21, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_UpdtRolesBridge(XSTP_OM_InstanceData_T *om_ptr)
{
    XSTP_TYPE_PriorityVector_T  root_priority_vector;
    XSTP_TYPE_PriorityVector_T  root_path_priority_vector;
    XSTP_TYPE_Timers_T          root_times;
    XSTP_TYPE_Timers_T          root_path_times;
    XSTP_TYPE_Timers_T          root_port_times;
    XSTP_OM_PortVar_T           *pom_ptr;
    UI32_T                      lport;
    UI32_T                      root_lport;
    UI16_T                      increment, max_age;
    BOOL_T                      is_root_bridge;
    I32_T                       cmp_a, cmp_b, cmp_c, cmp_d, cmp_e, cmp_f;

    /* Check if this bridge is root before selection */
    /* For new_root trap */
    is_root_bridge = XSTP_UTY_RootBridge(om_ptr);

    /* Determine the root_priority_vector */
    /* root_priority_vector :=
     * <Best of> {bridge_priority_vector} U {root_path_priority_vector|for all ports}
     */
    /* 17.19.21 :: (a) ~ (d) */
    memcpy(&root_priority_vector, &om_ptr->bridge_info.bridge_priority, XSTP_TYPE_PRIORITY_VECTOR_LENGTH);
    memcpy(&root_times, &om_ptr->bridge_info.bridge_times, XSTP_TYPE_TIMERS_LENGTH);
    lport           = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr     = &(om_ptr->port_info[lport-1]);
        if (pom_ptr->is_member)
        {
            /* Follow IEEE Std 802.1D-2004, page 168, 17.21.23 updtRcvdInfoWhile().
               Follow IEEE Std 802.1Q-2005, page 177, 13.26.22 updtRcvdInfoWhile().
               Message Age should be incremented by 1 second.
             */
            increment   = 1;

            /* 17.19.21 (a) */
            XSTP_OM_PRIORITY_VECTOR_FORMAT( root_path_priority_vector,
                                            pom_ptr->port_priority.root_bridge_id,
                                            pom_ptr->port_priority.root_path_cost + pom_ptr->port_path_cost,
                                            pom_ptr->port_priority.designated_bridge_id,
                                            pom_ptr->port_priority.designated_port_id,
                                            pom_ptr->port_id);

            /* 17.19.21 (b) */
            XSTP_OM_TIMERS_FORMAT(  root_path_times,
                                pom_ptr->port_times.message_age,
                                pom_ptr->port_times.max_age,
                                pom_ptr->port_times.hello_time,
                                pom_ptr->port_times.forward_delay);

            XSTP_OM_CMP_PRIORITY_VECTOR(cmp_a, root_priority_vector, root_path_priority_vector);

            if (    (pom_ptr->info_is != XSTP_ENGINE_PORTVAR_INFO_IS_DISABLED)
                 && (pom_ptr->rcvd_info_while > 0)
                 && (memcmp(&root_path_priority_vector.designated_bridge_id.addr,
                            &om_ptr->bridge_info.bridge_priority.designated_bridge_id.addr,
                            6
                           ) != 0
                    )
                 && (cmp_a > 0)
               )
            {
                /* 17.19.21 (c) */
                memcpy( &root_priority_vector,
                        &root_path_priority_vector,
                        XSTP_TYPE_PRIORITY_VECTOR_LENGTH
                      );
                /* 17.19.21 (d) */
                XSTP_OM_TIMERS_FORMAT(  root_times,
                                        root_path_times.message_age + increment,
                                        root_path_times.max_age,
                                        root_path_times.hello_time,
                                        root_path_times.forward_delay);

            }
        } /* End of if (pom_ptr->is_member) */
    } /* End of while (lport is existent) */

    XSTP_OM_CMP_BRIDGE_ID(cmp_a, root_priority_vector.root_bridge_id, om_ptr->bridge_info.root_priority.root_bridge_id);
    if (cmp_a != 0)
    {
        /* root bridge is changed */
        TRAP_EVENT_TrapData_T   trap_data;

        trap_data.trap_type = TRAP_EVENT_XSTP_ROOT_BRIDGE_CHANGED;
        XSTP_OM_GET_BRIDGE_ID_PRIORITY(
            trap_data.u.xstp_root_bridge_changed.priority,
            root_priority_vector.root_bridge_id);
        trap_data.u.xstp_root_bridge_changed.instance_id = om_ptr->instance_id;
        memcpy(trap_data.u.xstp_root_bridge_changed.bridge_address,
            &root_priority_vector.root_bridge_id.addr, 6);
        trap_data.community_specified = FALSE;
        trap_data.flag = TRAP_EVENT_SEND_TRAP_OPTION_DEFAULT;
        SNMP_PMGR_ReqSendTrap(&trap_data);
    }

    /* root_priority */
    memcpy( &om_ptr->bridge_info.root_priority,
            &root_priority_vector,
            XSTP_TYPE_PRIORITY_VECTOR_LENGTH
          );
    /* root_port_id */
    memcpy( &om_ptr->bridge_info.root_port_id,
            &root_priority_vector.bridge_port_id,
            XSTP_TYPE_PORT_ID_LENGTH
          );
    /* root_times */
    memcpy( &om_ptr->bridge_info.root_times,
            &root_times,
            XSTP_TYPE_TIMERS_LENGTH
          );

    if (!is_root_bridge && XSTP_UTY_RootBridge(om_ptr))
    {
        /* This bridge is the new_root */
        XSTP_OM_SetTrapFlagNewRoot(TRUE);
#if (SYS_CPNT_DEBUG == TRUE)
        XSTP_UTY_DebugPrintRoot(om_ptr, TRUE);
    }
    else if (cmp_a != 0)
    {
        XSTP_UTY_DebugPrintRoot(om_ptr, FALSE);
#endif
    }

    root_lport      = (UI32_T)(om_ptr->bridge_info.root_port_id.port_id & 0x0FFF);
    lport           = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr     = &(om_ptr->port_info[lport-1]);

        if (pom_ptr->is_member)
        {
            /* 17.19.21 (e) */
            XSTP_OM_PRIORITY_VECTOR_FORMAT( pom_ptr->designated_priority,
                                            root_priority_vector.root_bridge_id,
                                            root_priority_vector.root_path_cost,
                                            om_ptr->bridge_info.bridge_identifier,
                                            pom_ptr->port_id,
                                            pom_ptr->port_id);

            /* 17.19.21 (f) */
            memcpy( &pom_ptr->designated_times,
                    &root_times,
                    XSTP_TYPE_TIMERS_LENGTH
                  );

            switch (pom_ptr->info_is)
            {
                /* 17.19.21 (g) */
                case XSTP_ENGINE_PORTVAR_INFO_IS_DISABLED:
                    pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_DISABLED;
                    break;

                /* 17.19.21 (h) */
                case XSTP_ENGINE_PORTVAR_INFO_IS_AGED:
                    pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED;
                    pom_ptr->updt_info      = TRUE;
                    break;

                /* 17.19.21 (i) */
                case XSTP_ENGINE_PORTVAR_INFO_IS_MINE:
                    pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED;
                    if (root_lport == 0)
                    {
                        memcpy( &root_port_times,
                                &om_ptr->bridge_info.bridge_times,
                                XSTP_TYPE_TIMERS_LENGTH
                              );
                    }
                    else
                    {
                        XSTP_OM_PortVar_T   *root_pom_ptr;
                        root_pom_ptr    = &(om_ptr->port_info[root_lport-1]);
                        memcpy( &root_port_times,
                                &root_pom_ptr->port_times,
                                XSTP_TYPE_TIMERS_LENGTH
                              );
                    }
                    XSTP_OM_CMP_PRIORITY_VECTOR(cmp_b, pom_ptr->port_priority, pom_ptr->designated_priority);
                    XSTP_OM_CMP_TIMERS(cmp_c, pom_ptr->port_times, root_port_times);
                    if (    (cmp_b != 0)
                         || (cmp_c != 0)
                       )
                    {
                        pom_ptr->updt_info  = TRUE;
                    }
                    break;

                /* 17.19.21 (j, k, l) */
                case XSTP_ENGINE_PORTVAR_INFO_IS_RECEIVED:
                    if (lport == root_lport)
                    {
                        /* 17.19.21 (j) */
                        pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_ROOT;
                        pom_ptr->updt_info      = FALSE;
                    }
                    else
                    {
                        XSTP_OM_CMP_PRIORITY_VECTOR(cmp_d, pom_ptr->designated_priority, pom_ptr->port_priority);
                        if (cmp_d >= 0)
                        {
                            XSTP_OM_CMP_BRIDGE_ID(cmp_e, pom_ptr->port_priority.designated_bridge_id, om_ptr->bridge_info.bridge_identifier);
                            XSTP_OM_CMP_PORT_ID(cmp_f, pom_ptr->port_priority.designated_port_id, pom_ptr->port_id);
                            if (cmp_e != 0)
                            {
                                /* 17.19.21 (k) */
                                pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE;
                                pom_ptr->updt_info      = FALSE;
                            }
                            else
                            if (cmp_f != 0)
                            {
                                /* 17.19.21 (l) */
                                pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_BACKUP;
                                pom_ptr->updt_info      = FALSE;
                            }
                        }
                        else
                        {
                            /* Retired root port */
                            pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED;
                            pom_ptr->updt_info      = TRUE;
                        }
                    }
                    break;
            } /* End of switch (pom_ptr->info_is) */
        } /* End of if(pom_ptr->is_member) */
    } /* End of while(lport is existent) */

    return;
} /* End of XSTP_UTY_UpdtRolesBridge */

/* ===================================================================== */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_DecreaseTimer
 * ------------------------------------------------------------------------
 * PURPOSE  : Decrease the specified timer
 * INPUT    : timer     -- value of timer
 * OUTPUT   : None
 * RETURN   : TRUE if the timer expires, else FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_DecreaseTimer(UI16_T *timer)
{
    if ( (*timer) != 0)
    {
        (*timer)--;
        if ( (*timer) == 0)
            return TRUE;
    }

    return FALSE;
} /* End of XSTP_UTY_DecreaseTimer */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_DecreaseTimer32
 * ------------------------------------------------------------------------
 * PURPOSE  : Decrease the specified timer
 * INPUT    : timer     -- value of timer
 * OUTPUT   : None
 * RETURN   : TRUE if the timer expires, else FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_DecreaseTimer32(UI32_T *timer)
{
    if ( (*timer) != 0)
    {
        (*timer)--;
        if ( (*timer) == 0)
            return TRUE;
    }

    return FALSE;
} /* End of XSTP_UTY_DecreaseTimer32 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_ReselectForAnyPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Check if the reselect variable of any port is set
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : TRUE if the reselect variable of any port is TRUE, else FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_ReselectForAnyPort(XSTP_OM_InstanceData_T *om_ptr)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI32_T              lport;
    BOOL_T              reselect;

    reselect = FALSE;
    /* Performance Improvement
    for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
    */
    lport = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr     = &(om_ptr->port_info[lport-1]);
        if (pom_ptr->is_member)
        {
            reselect = reselect || pom_ptr->reselect;
        }
    }

    return reselect;
} /* End of XSTP_UTY_ReselectForAnyPort */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_AllSyncedForOthers
 * ------------------------------------------------------------------------
 * PURPOSE  : Check if all ports except the specified one are synced
 * INPUT    : om_ptr        -- om pointer for this instance
 *            this_lport    -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE if all ports other than the specified one are synced,
 *            else FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_AllSyncedForOthers(XSTP_OM_InstanceData_T *om_ptr, UI32_T this_lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI32_T              lport;
    BOOL_T              all_synced;

    all_synced          = TRUE;
    lport               = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        if (lport != this_lport)
        {
            pom_ptr     = &(om_ptr->port_info[lport-1]);
            if (    (pom_ptr->is_member)
                &&  (pom_ptr->common->port_spanning_tree_status == VAL_staPortSystemStatus_enabled)
               )
            {
                all_synced  = all_synced && pom_ptr->synced;
            }
        }
    }

    return all_synced;
} /* End of XSTP_UTY_AllSyncedForOthers */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_ReRootedForOthers
 * ------------------------------------------------------------------------
 * PURPOSE  : Check if all ports except the specified one are synced
 * INPUT    : om_ptr        -- om pointer for this instance
 *            this_lport    -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE if all ports other than the specified one are synced,
 *            else FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_ReRootedForOthers(XSTP_OM_InstanceData_T *om_ptr, UI32_T this_lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI32_T              lport;
    BOOL_T              re_rooted;

    re_rooted           = TRUE;
    lport               = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        if (lport != this_lport)
        {
            pom_ptr     = &(om_ptr->port_info[lport-1]);
            if (    (pom_ptr->is_member)
                &&  (pom_ptr->common->port_spanning_tree_status == VAL_staPortSystemStatus_enabled)
               )
            {
                re_rooted   = re_rooted && (pom_ptr->rr_while == 0);
            }
        }
    }

    return re_rooted;
} /* End of XSTP_UTY_ReRootedForOthers */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_BetterOrSameInfo
 * ------------------------------------------------------------------------
 * PURPOSE  : 1. return TRUE if the received CIST priority vector is better
 *               than or the same as (13.10) the CIST port priority vector.
 *            2. returns TRUE if the MSTI priority vector is better than or
 *               the same as (13.11) the MSTI port priority vector.
 * INPUT    : om_ptr        -- om pointer for this instance
 *            lport         -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref to the description in 13.26.1, 13.26.2,
 *            IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_BetterOrSameInfo(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    BOOL_T              better_or_same_info;
    int                 cmp_result=1;

    better_or_same_info = FALSE;
    pom_ptr     = &(om_ptr->port_info[lport-1]);

    /*a*/
    if(pom_ptr->info_is  == XSTP_ENGINE_PORTVAR_INFO_IS_RECEIVED)
    {
        XSTP_OM_CMP_PRIORITY_VECTOR(cmp_result, pom_ptr->msg_priority, pom_ptr->port_priority);
    }
    else /*b*/
    if(pom_ptr->info_is  == XSTP_ENGINE_PORTVAR_INFO_IS_MINE)
    {
        XSTP_OM_CMP_PRIORITY_VECTOR(cmp_result, pom_ptr->designated_priority, pom_ptr->port_priority);
    }

    if (cmp_result <= 0)
        better_or_same_info = TRUE;

    return better_or_same_info;
} /* End of XSTP_UTY_BetterOrSameInfoXst */
#endif /* XSTP_TYPE_PROTOCOL_RSTP */

/* ===================================================================== */
/* Private */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_AddIntoLportList
 * ------------------------------------------------------------------------
 * PURPOSE  : Increase the Topology Change Count value
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport value.
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static  XSTP_TYPE_LportList_T    *XSTP_UTY_AddIntoLportList(XSTP_TYPE_LportList_T *list, UI32_T xstid, UI32_T lport, BOOL_T enter_forwarding)
{
    XSTP_TYPE_LportList_T    *lport_list;
    XSTP_TYPE_LportList_T    *prev;
    XSTP_TYPE_LportList_T    *this_lport;

    /* check whether allow to add new element to the list */
    if (XSTP_UTY_ChangeStatePortListForbidden)
        return list;

    lport_list  = list;
    prev        = NULL;
    while (lport_list != NULL)
    {
        prev            = lport_list;
        lport_list      = lport_list->next;
    } /* End of while */

    this_lport  = (XSTP_TYPE_LportList_T*)L_MM_Malloc(sizeof(XSTP_TYPE_LportList_T), L_MM_USER_ID2(SYS_MODULE_XSTP, XSTP_TYPE_TRACE_ID_XSTP_UTY_ADDINTOLPORTLIST));
    if (this_lport)
    {
        memset(this_lport, 0, sizeof(XSTP_TYPE_LportList_T));
        this_lport->xstid   = xstid;
        this_lport->lport   = lport;
        this_lport->enter_forwarding = enter_forwarding;

        this_lport->next    = NULL;
        if (prev == NULL)
        {
            list            = this_lport;
        }
        else
        {
            prev->next      = this_lport;
        }
    } /* End of if (this_lport) */

    return list;
} /* End of XSTP_UTY_AddIntoLportList */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SendConfigBpdu
 * ------------------------------------------------------------------------
 * PURPOSE  : Send configuration BPDU
 * INPUT    : lport     -- lport
 *            bpdu      -- Configuration BPDU content
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static  void    XSTP_UTY_SendConfigBpdu(UI32_T lport, XSTP_TYPE_ConfigBpdu_T *bpdu)
{
    L_MM_Mref_Handle_T      *mref_handle_p;
    UI8_T                   my_mac_addr[6];
    XSTP_TYPE_ConfigBpdu_T  *config_ptr;
    UI32_T                  pdu_len;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(XSTP_TYPE_ConfigBpdu_T),
                                          L_MM_USER_ID2(SYS_MODULE_XSTP, XSTP_TYPE_TRACE_ID_XSTP_UTY_SENDCONFIGBPDU));/*,
                                          NULL, NULL);*/
    if(mref_handle_p==NULL)
    {
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("\r\n%s::ERROR!! for L_MM_AllocateTxBuffer return NULL", __FUNCTION__);
        return;
    }

    config_ptr = (XSTP_TYPE_ConfigBpdu_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if(config_ptr==NULL)
    {
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("\r\n%s::ERROR!! for L_MM_Mref_GetPdu return NULL", __FUNCTION__);
        return;
    }

    memcpy(config_ptr, bpdu, sizeof(XSTP_TYPE_ConfigBpdu_T) );

    config_ptr->sap                         = L_STDLIB_Hton16(XSTP_TYPE_STP_SAP);
    config_ptr->ctrl_byte                   = XSTP_TYPE_UI_CONTROL_BYTE;
    config_ptr->protocol_identifier         = L_STDLIB_Hton16(XSTP_TYPE_PROTOCOL_ID);
    config_ptr->protocol_version_identifier = XSTP_TYPE_STP_PROTOCOL_VERSION_ID;
    config_ptr->bpdu_type                   = XSTP_TYPE_BPDU_TYPE_CONFIG;

    /* === Check  link === */
    {
        UI32_T link_status;
        Port_Info_T port_info;
        if (!SWCTRL_GetPortInfo(lport, &port_info))
        {
            L_MM_Mref_Release(&mref_handle_p);
            return;
        }
        link_status = port_info.link_status;
        if (link_status == SWCTRL_LINK_DOWN)
        {
            L_MM_Mref_Release(&mref_handle_p);
            return;
        }
    }

    /* === Get My Mac === */
    if (!SWCTRL_GetPortMac(lport, my_mac_addr))
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    /* === Send packet === */
    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_TXCFG))
    {
        UI8_T                   i;
        UI32_T                  current_time;

        SYS_TIME_GetRealTimeBySec(&current_time);

        BACKDOOR_MGR_Printf("\r\n");
        BACKDOOR_MGR_Printf("\r\nTXConfig::");
        BACKDOOR_MGR_Printf("[TxToLport=%ld]", lport);
        BACKDOOR_MGR_Printf("[Flags = 0x%02x]", config_ptr->flags);
        BACKDOOR_MGR_Printf("[RootId=%4x", config_ptr->root_identifier.bridge_id_priority.bridge_priority);
        for (i=0; i<6; i++) BACKDOOR_MGR_Printf(":%02x", config_ptr->root_identifier.addr[i]);
        BACKDOOR_MGR_Printf("][RPC = 0x%04lx]", config_ptr->root_path_cost);
        BACKDOOR_MGR_Printf("[BridgeId=%4x", config_ptr->bridge_identifier.bridge_id_priority.bridge_priority);
        for (i=0; i<6; i++) BACKDOOR_MGR_Printf(":%02x", config_ptr->bridge_identifier.addr[i]);
        BACKDOOR_MGR_Printf("][PortId=%4x]", config_ptr->port_identifier.port_id);
        BACKDOOR_MGR_Printf("[TxTime = %ld]", current_time);
    }

    XSTP_UTY_SendPacket(mref_handle_p,
                        XSTP_UTY_BridgeGroupAddr,
                        my_mac_addr,
                        sizeof(XSTP_TYPE_ConfigBpdu_T),         /* type */
                        SYS_TYPE_IGNORE_VID_CHECK,
                        sizeof(XSTP_TYPE_ConfigBpdu_T),         /* size */
                        lport,
                        0,
                        XSTP_TYPE_BPDU_TXCOS);

    return;
} /* End of XSTP_UTY_SendConfigBpdu */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SendRstpBpdu
 * ------------------------------------------------------------------------
 * PURPOSE  : Send RSTP BPDU
 * INPUT    : lport     -- lport
 *            bpdu      -- Rstp BPDU content
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static  void    XSTP_UTY_SendRstpBpdu(UI32_T lport, XSTP_TYPE_RstBpdu_T *bpdu)
{
    L_MM_Mref_Handle_T      *mref_handle_p;
    UI8_T                   my_mac_addr[6];
    XSTP_TYPE_RstBpdu_T     *bpdu_ptr;
    UI32_T                  pdu_len;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(XSTP_TYPE_RstBpdu_T),
                                          L_MM_USER_ID2(SYS_MODULE_XSTP, XSTP_TYPE_TRACE_ID_XSTP_UTY_SENDRSTPBPDU));/*,
                                          NULL, NULL);*/
    if(mref_handle_p==NULL)
    {
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("\r\n%s::ERROR!! for L_MM_AllocateTxBuffer return NULL", __FUNCTION__);
        return;
    }

    bpdu_ptr = (XSTP_TYPE_RstBpdu_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if(bpdu_ptr==NULL)
    {
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("\r\n%s::ERROR!! for L_MM_Mref_GetPdu return NULL", __FUNCTION__);
        return;
    }

    memcpy(bpdu_ptr, bpdu, sizeof(XSTP_TYPE_RstBpdu_T) );

    bpdu_ptr->sap                           = L_STDLIB_Hton16(XSTP_TYPE_STP_SAP);
    bpdu_ptr->ctrl_byte                     = XSTP_TYPE_UI_CONTROL_BYTE;
    bpdu_ptr->protocol_identifier           = L_STDLIB_Hton16(XSTP_TYPE_PROTOCOL_ID);
    bpdu_ptr->protocol_version_identifier   = XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID;
    bpdu_ptr->bpdu_type                     = XSTP_TYPE_BPDU_TYPE_XST;

    /* === Check  link === */
    {
        UI32_T link_status;
        Port_Info_T port_info;
        if (!SWCTRL_GetPortInfo(lport, &port_info))
        {
            L_MM_Mref_Release(&mref_handle_p);
            return;
        }
        link_status = port_info.link_status;
        if (link_status == SWCTRL_LINK_DOWN)
        {
            L_MM_Mref_Release(&mref_handle_p);
            return;
        }
    }

    /* === Get My Mac === */
    if (!SWCTRL_GetPortMac(lport, my_mac_addr))
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    /* === Send packet === */
    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_TXRSTP))
    {
        UI8_T                   i;
        UI32_T                  current_time;

        SYS_TIME_GetRealTimeBySec(&current_time);

        BACKDOOR_MGR_Printf("\r\n");
        BACKDOOR_MGR_Printf("\r\nTXRSTP::");
        BACKDOOR_MGR_Printf("[TxToLport=%ld]", lport);
        BACKDOOR_MGR_Printf("[Flags = 0x%02x]", bpdu_ptr->flags);
        BACKDOOR_MGR_Printf("[RootId=%4x", bpdu_ptr->root_identifier.bridge_id_priority.bridge_priority);
        for (i=0; i<6; i++) BACKDOOR_MGR_Printf(":%02x", bpdu_ptr->root_identifier.addr[i]);
        BACKDOOR_MGR_Printf("][RPC = 0x%04lx]", bpdu_ptr->root_path_cost);
        BACKDOOR_MGR_Printf("[BridgeId=%4x", bpdu_ptr->bridge_identifier.bridge_id_priority.bridge_priority);
        for (i=0; i<6; i++) BACKDOOR_MGR_Printf(":%02x", bpdu_ptr->bridge_identifier.addr[i]);
        BACKDOOR_MGR_Printf("][PortId = 0x%02x]", bpdu_ptr->port_identifier.port_id);
        BACKDOOR_MGR_Printf("[TxTime = %ld]", current_time);
    }

    XSTP_UTY_SendPacket(mref_handle_p,
                        XSTP_UTY_BridgeGroupAddr,
                        my_mac_addr,
                        sizeof(XSTP_TYPE_RstBpdu_T),            /* type */
                        SYS_TYPE_IGNORE_VID_CHECK,
                        sizeof(XSTP_TYPE_RstBpdu_T),            /* size */
                        lport,
                        0,
                        XSTP_TYPE_BPDU_TXCOS);

    return;
} /* End of XSTP_UTY_SendRstpBpdu */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SendTcnBpdu
 * ------------------------------------------------------------------------
 * PURPOSE  : Send Topology change notification BPDU
 * INPUT    : lport     -- lport
 * OUTPUT   : None
 * RETURN   : TRUE if the reselect variable of any port is TRUE, else FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static  void    XSTP_UTY_SendTcnBpdu(UI32_T lport)
{
    L_MM_Mref_Handle_T      *mref_handle_p;
    UI8_T                   my_mac_addr[6];
    XSTP_TYPE_TcnBpdu_T     *tcn_ptr;
    UI32_T                  pdu_len;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(XSTP_TYPE_TcnBpdu_T),
                                          L_MM_USER_ID2(SYS_MODULE_XSTP, XSTP_TYPE_TRACE_ID_XSTP_UTY_SENDTCNBPDU));/*,
                                          NULL, NULL);*/
    if(mref_handle_p==NULL)
    {
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("\r\n%s::ERROR!! for L_MM_AllocateTxBuffer return NULL", __FUNCTION__);
        return;
    }

    tcn_ptr = (XSTP_TYPE_TcnBpdu_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if(tcn_ptr==NULL)
    {
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("\r\n%s::ERROR!! for L_MM_Mref_GetPdu return NULL", __FUNCTION__);
        return;
    }

    tcn_ptr->sap                            = L_STDLIB_Hton16(XSTP_TYPE_STP_SAP);
    tcn_ptr->ctrl_byte                      = XSTP_TYPE_UI_CONTROL_BYTE;
    tcn_ptr->protocol_identifier            = L_STDLIB_Hton16(XSTP_TYPE_PROTOCOL_ID);
    tcn_ptr->protocol_version_identifier    = XSTP_TYPE_STP_PROTOCOL_VERSION_ID;
    tcn_ptr->bpdu_type                      = XSTP_TYPE_BPDU_TYPE_TCN;

    /* === Check  link === */
    {
        UI32_T link_status;
        Port_Info_T port_info;
        if (!SWCTRL_GetPortInfo(lport, &port_info))
        {
            L_MM_Mref_Release(&mref_handle_p);
            return;
        }
        link_status = port_info.link_status;
        if (link_status == SWCTRL_LINK_DOWN)
        {
            L_MM_Mref_Release(&mref_handle_p);
            return;
        }
    }

    /* === Get My Mac === */
    if (!SWCTRL_GetPortMac(lport, my_mac_addr))
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    /* === Send packet === */
    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_TXTCN))
    {
        UI8_T                   i;
        UI32_T                  current_time;

        SYS_TIME_GetRealTimeBySec(&current_time);

        BACKDOOR_MGR_Printf("\r\nTXTCN::[TxToLport=%ld][MyMacAddr=", lport);
        for (i=0; i<6; i++) BACKDOOR_MGR_Printf(":%02x", my_mac_addr[i]);
        BACKDOOR_MGR_Printf("][Time = %ld]", current_time);
    }

    XSTP_UTY_SendPacket(mref_handle_p,
                        XSTP_UTY_BridgeGroupAddr,
                        my_mac_addr,
                        sizeof(XSTP_TYPE_TcnBpdu_T),            /* type */
                        SYS_TYPE_IGNORE_VID_CHECK,
                        sizeof(XSTP_TYPE_TcnBpdu_T),            /* size */
                        lport,
                        0,
                        XSTP_TYPE_BPDU_TXCOS);

    return;
} /* End of XSTP_UTY_SendTcnBpdu */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SendPacket
 *-------------------------------------------------------------------------
 * FUNCTION : Send packet for lport via primary member/all members
 * INPUT    : (See the description described in LAN_SendPacket except lport)
 *            UI32_T lport: lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : In XSTP the lport is used. Here we convert the lport to the
 *            unit and port in user view.
 *-------------------------------------------------------------------------
 */
static  void    XSTP_UTY_SendPacket(L_MM_Mref_Handle_T *mref_handle_p,
                                    UI8_T       dst_mac[6],
                                    UI8_T       src_mac[6],
                                    UI16_T      type,
                                    UI16_T      tag_info,
                                    UI32_T      packet_length,
                                    UI32_T      lport,
                                    BOOL_T      is_tagged,
                                    UI32_T      cos_value)
{
    if (mref_handle_p == NULL)
        return;

#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);

    if (    (pom_ptr->common->bpdu_filter_status == TRUE)
         && (pom_ptr->common->oper_edge == TRUE)
       )
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }
}
#endif

    mref_handle_p->next_usr_id = SYS_MODULE_LAN;

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

#if (SYS_CPNT_DEBUG == TRUE)
{
    UI8_T *ptr = NULL;
    UI32_T len;

    ptr = (UI8_T*)L_MM_Mref_GetPdu(mref_handle_p, &len);
    if (ptr == NULL)
        return;

    XSTP_UTY_DebugPringBPDU(lport, src_mac, FALSE, packet_length, ptr, TRUE);
}
#endif

    L2MUX_MGR_SendBPDU( mref_handle_p,
                    dst_mac,
                    src_mac,
                    type,
                    tag_info,
                    packet_length,
                    lport,
                    is_tagged,
                    cos_value,
                    FALSE);

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);

    return;
} /* End of XSTP_UTY_SendPacket() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RootBridge
 * ------------------------------------------------------------------------
 * FUNCTION : Check whether the Bridge is the Root Bridge.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE if the Bridge is the Root Bridge, else FALSE.
 * NOTE     : If the Designated Root parameter held by the Bridge is the
 *            same as the Bridge ID of the Bridge, then conclude the
 *            Bridge to be the Root Bridge.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_RootBridge(XSTP_OM_InstanceData_T *om_ptr)
{
    I32_T cmp_a;

    #ifdef XSTP_TYPE_PROTOCOL_RSTP
    XSTP_OM_CMP_BRIDGE_ID(cmp_a, (om_ptr->bridge_info.root_priority.root_bridge_id), (om_ptr->bridge_info.bridge_identifier));
    return((BOOL_T)(cmp_a == 0));
    #endif /* XSTP_TYPE_PROTOCOL_RSTP */

    #ifdef XSTP_TYPE_PROTOCOL_MSTP
    if (XSTP_UTY_Cist(om_ptr) )                     /* CIST */
    {
        XSTP_OM_CMP_BRIDGE_ID(cmp_a, (om_ptr->bridge_info.root_priority.root_id), (om_ptr->bridge_info.bridge_identifier));
        return((BOOL_T)(cmp_a == 0));
    }
    else                                            /* MSTI */
    {
        XSTP_OM_CMP_BRIDGE_ID(cmp_a, (om_ptr->bridge_info.root_priority.r_root_id), (om_ptr->bridge_info.bridge_identifier));
        return((BOOL_T)(cmp_a == 0));
    }
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

}/* End of XSTP_UTY_RootBridge() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_IncTopologyChangeCount
 * ------------------------------------------------------------------------
 * PURPOSE  : Increase the Topology Change Count value
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport value.
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static  void    XSTP_UTY_IncTopologyChangeCount(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI32_T              next_lport;
    XSTP_OM_PortVar_T   *temp_pom_ptr;
    BOOL_T              result;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    result      = FALSE;
    /* === Check  link === */
    if (!pom_ptr->common->link_up)
        return;
    else
    {
        next_lport = 0;
        while ( (SWCTRL_GetNextLogicalPort(&next_lport) != SWCTRL_LPORT_UNKNOWN_PORT) && (!result))
        {
            temp_pom_ptr     = &(om_ptr->port_info[next_lport-1]);
            if (    (temp_pom_ptr->is_member)
                &&  (temp_pom_ptr->common->port_spanning_tree_status == VAL_staPortSystemStatus_enabled)
               )
            {
                if (temp_pom_ptr->tc_while != 0)
                {
                    result = TRUE;
                }
            }
        }

        if (!result)
        {
            om_ptr->bridge_info.topology_change_count++;
            om_ptr->bridge_info.time_since_topology_change  = (UI32_T)SYSFUN_GetSysTick();

#if (SYS_CPNT_DEBUG == TRUE)
            XSTP_UTY_DebugPrintEvents(lport);
#endif
        }
    }
    return ;
} /* End of XSTP_UTY_IncTopologyChangeCount */

/* ===================================================================== */
/* ===================================================================== */
/* IEEE 802.1s */
#ifdef XSTP_TYPE_PROTOCOL_MSTP

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_AllSynced
 * ------------------------------------------------------------------------
 * PURPOSE  : Check if all ports are synced for the given tree.
 * INPUT    : om_ptr        -- om pointer for this instance
 *            this_lport    -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE if all ports are synced for the given tree,
 *            else FALSE
 * NOTE     : Ref to the description in 13.25.1, IEEE Std 802.1Q-2005
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_AllSynced(XSTP_OM_InstanceData_T *om_ptr, UI32_T this_lport)
{
    XSTP_OM_PortVar_T   *pom_ptr, *this_pom_ptr;
    UI32_T              lport;
    BOOL_T              all_synced;
    UI32_T              no_check_port;

    all_synced          = FALSE;
    this_pom_ptr        = &(om_ptr->port_info[this_lport-1]);
    lport               = 0;

    /* Std 802.1Q-2005 13.25.1 allSynced */
    if (    (this_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
        ||  (this_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
        ||  (this_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
        ||  (this_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_MASTER)
       )
    {
        if (this_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
        {
            no_check_port = (UI32_T)(om_ptr->bridge_info.root_port_id.port_id & 0x0FFF);
        }
        else
        {
            no_check_port = this_lport;
        }
        all_synced  = TRUE;
        while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
        {
            pom_ptr     = &(om_ptr->port_info[lport-1]);
            if (    (pom_ptr->is_member)
                &&  (pom_ptr->common->port_spanning_tree_status == VAL_staPortSystemStatus_enabled)
               )
            {
                if (    (!pom_ptr->selected)
                    ||  (pom_ptr->role != pom_ptr->selected_role)
                    ||  (pom_ptr->updt_info)
                    ||  (   (lport != no_check_port)
                         && (!pom_ptr->synced)
                        )
                   )
                {
                    all_synced  = FALSE;
                    break;
                }
            } /* End of is_member*/
        } /* End of while*/
    } /* End of this_pom_ptr->role */

    return all_synced;
} /* End of XSTP_UTY_AllSynced */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_Cist
 * ------------------------------------------------------------------------
 * PURPOSE  : TRUE only for CIST state machines, i.e. FALSE for MSTI state
 *            machine instances.
 * INPUT    : om_ptr        -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref to the description in 13.25.2, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_Cist(XSTP_OM_InstanceData_T *om_ptr)
{
    return (om_ptr->bridge_info.cist == (XSTP_OM_MstBridgeVar_T*)NULL);
} /* End of XSTP_UTY_Cist */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_CistRootPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Return TRUE if the CIST role for the given Port is RootPort.
 *            Return FALSE otherwise.
 * INPUT    : om_ptr        -- om pointer for this instance
 *            lport         -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.25.3, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_CistRootPort(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    BOOL_T              cist_root_port;

    cist_root_port = FALSE;
    pom_ptr     = &(om_ptr->port_info[lport-1]);
    if (pom_ptr->cist == NULL)
    {
        /* CIST */
        if (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
        {
            cist_root_port = TRUE;
        }
    }
    else
    {
        /* MSTI */
        if (pom_ptr->cist->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
        {
            cist_root_port = TRUE;
        }
    }
    return cist_root_port;

} /* End of XSTP_UTY_CistRootPort */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_CistDesignatedPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Return TRUE if the CIST role for the given Port is
 *            DesignatedPort. Return FALSE otherwise.
 * INPUT    : om_ptr        -- om pointer for this instance
 *            lport         -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.25.4, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_CistDesignatedPort(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    BOOL_T              cist_designated_port;

    cist_designated_port = FALSE;
    pom_ptr     = &(om_ptr->port_info[lport-1]);

    if (pom_ptr->cist == NULL)
    {
        /* CIST */
        if (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
        {
            cist_designated_port = TRUE;
        }
    }
    else
    {
        /* MSTI */
        if (pom_ptr->cist->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
        {
            cist_designated_port = TRUE;
        }
    }

    return cist_designated_port;

} /* End of XSTP_UTY_CistDesignatedPort */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_MstiRootPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Return TRUE if the role for any MSTI for the given Port is
 *            root port. Return FALSE otherwise.
 * INPUT    : lport         -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref to the description in 13.25.5, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_MstiRootPort(UI32_T lport)
{
    UI32_T                  xstid;
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    xstid               = XSTP_TYPE_CISTID;
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &om_ptr))   /* exclude CIST */
    {
        if (om_ptr->instance_exist)
        {
            pom_ptr         = &(om_ptr->port_info[lport-1]);
            if (    pom_ptr->is_member
                &&  (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
               )
            {
                return TRUE;
            }
        }
    }
    return FALSE;

} /* End of XSTP_UTY_MstiRootPort */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_MstiDesignatedPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Return TRUE if the role for any MSTI for the given Port is
 *            designated port. Return FALSE otherwise.
 * INPUT    : lport         -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref to the description in 13.25.6, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_MstiDesignatedPort(UI32_T lport)
{
    UI32_T                  xstid;
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    xstid               = XSTP_TYPE_CISTID;
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &om_ptr))   /* exclude CIST */
    {
        if (om_ptr->instance_exist)
        {
            pom_ptr         = &(om_ptr->port_info[lport-1]);
            if (    pom_ptr->is_member
                &&  (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
               )
            {
                return TRUE;
            }
        }
    }
    return FALSE;

} /* End of XSTP_UTY_MstiDesignatedPort */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RcvdAnyMsg
 * ------------------------------------------------------------------------
 * PURPOSE  : return TRUE for a given Port if rcvd_msg is TRUE for the CIST
 *            or any MSTI for that Port, else FALSE.
 * INPUT    : lport         -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref to the description in 13.25.7, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_RcvdAnyMsg(UI32_T lport)
{
    UI32_T                  xstid;
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    xstid               = XSTP_TYPE_CISTID;
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    do
    {
        if (om_ptr->instance_exist)
        {
            pom_ptr     = &(om_ptr->port_info[lport-1]);
            if (pom_ptr->is_member && pom_ptr->rcvd_msg )
                return TRUE;
        }
    }while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &om_ptr));
    return FALSE;
} /* End of XSTP_UTY_RcvdAnyMsg */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RcvdXstInfo
 * ------------------------------------------------------------------------
 * PURPOSE  : 1. return TRUE for a given Port if and only if rcvd_msg is TRUE
 *               for the CIST for that port.
 *            2. return TRUE for a given port and MSTI if and only if rcvd_msg
 *               is FALSE for the CIST for that port and rcvd_msg is TRUE for
 *               the MSTI for that port.
 * INPUT    : om_ptr        -- om pointer for this instance
 *            lport         -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref to the description in 13.25.8, IEEE Std 802.1s(D13)-2002
 *            Ref to the description in 13.25.9, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_RcvdXstInfo(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    BOOL_T              rcvd_xst_info;

    rcvd_xst_info   = FALSE;
    pom_ptr         = &(om_ptr->port_info[lport-1]);
    if (pom_ptr->cist == NULL)
    {
        /* 13.25.8 rcvdCistInfo */
        if (pom_ptr->rcvd_msg)
        {
            rcvd_xst_info = TRUE;
        }
    }
    else
    {
        /* 13.25.9 rcvdMstiInfo */
        if (!(pom_ptr->cist->rcvd_msg) && (pom_ptr->rcvd_msg))
        {
            rcvd_xst_info = TRUE;
        }
    }

    return rcvd_xst_info;
} /* End of XSTP_UTY_RcvdXstInfo */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_ReRooted
 * ------------------------------------------------------------------------
 * PURPOSE  : TRUE if the rr_while timer is clear for all ports for the
 *            given tree other than the given port.
 * INPUT    : om_ptr        -- om pointer for this instance
 *            this_lport    -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref to the description in 13.25.10, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_ReRooted(XSTP_OM_InstanceData_T *om_ptr, UI32_T this_lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI32_T              lport;
    BOOL_T              re_rooted;

    re_rooted           = TRUE;
    lport               = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        if (lport != this_lport)
        {
            pom_ptr     = &(om_ptr->port_info[lport-1]);
            if (    (pom_ptr->is_member)
                &&  (pom_ptr->common->port_spanning_tree_status == VAL_staPortSystemStatus_enabled)
               )
            {
                re_rooted   = re_rooted && (pom_ptr->rr_while == 0);
            }
        }
    }

    return re_rooted;
} /* End of XSTP_UTY_ReRooted */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtXstInfo
 * ------------------------------------------------------------------------
 * PURPOSE  : 1. return TRUE for a given port if and only if updt_info is
 *               TRUE for the CIST for that port.
 *            2. return TRUE for a given Port and MSTI if and only if
 *               updt_info is TRUE for the MSTI for that port or either
 *               updt_info or selected are TRUE for the CIST for that port.
 * INPUT    : om_ptr        -- om pointer for this instance
 *            lport         -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref to the description in 13.25.11, IEEE Std 802.1s(D13)-2002
 *            Ref to the description in 13.25.12, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_UpdtXstInfo(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    BOOL_T              updt_xst_info;

    updt_xst_info = FALSE;
    pom_ptr     = &(om_ptr->port_info[lport-1]);
    if (pom_ptr->cist == NULL)
    {
        /* 13.25.11 updtCistInfo */
        if (pom_ptr->updt_info)
        {
            updt_xst_info = TRUE;
        }
    }
    else
    {
        /* 13.25.12 updtMstiInfo */
        if (    pom_ptr->updt_info
             || (   pom_ptr->cist->updt_info
                 && pom_ptr->cist->selected
                )
           )
        {
            updt_xst_info = TRUE;
        }
    }

    return updt_xst_info;
} /* End of XSTP_UTY_UpdtXstInfo */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_TxTcn
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.a, IEEE Std 802.1s(D13)-2002
 *            Ref to the description in 17.19.17, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_TxTcn(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    if (XSTP_UTY_Cist(om_ptr) )
        XSTP_UTY_SendTcnBpdu(lport);

    return;
} /* End of XSTP_UTY_TxTcn */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtBpduVersion
 * ------------------------------------------------------------------------
 * PURPOSE  : Set rcvd_stp TRUE if the BPDU received is a version 0 or
 *            version 1 PDU, either a TCN or a Config BPDU. It sets rcvd_rstp
 *            TRUE if the received BPDU is an RST BPDU and (ForceVersion >= 2).
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. Ref to the description in 13.26.g, IEEE Std 802.1s(D13)-2002
 *            2. Ref to the description in 17.19.18, IEEE Std 802.1w-2001
 *            3. This procedure is invoked only for CIST because PRX state
 *               machine which invokes this procedure is implemented as a
 *               common state machine triggered in CIST.
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_UpdtBpduVersion(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_BpduHeader_T  *bpdu_header;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    bpdu_header = (XSTP_TYPE_BpduHeader_T*)pom_ptr->common->bpdu;

    switch (bpdu_header->bpdu_type)
    {
        case XSTP_TYPE_BPDU_TYPE_XST:
            if (    (   ((XSTP_TYPE_MstBpdu_T*)(pom_ptr->common->bpdu))->cist_flags
                     &  XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_MASK
                    )
                ==  XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_UNKNOWN
               )
            {
                /* 9.2.9 */
                pom_ptr->common->rcvd_stp       = TRUE;
            }
            else
            if (    (bpdu_header->protocol_version_identifier >= XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID)
                 && (XSTP_OM_GetForceVersion() >= XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID)
               )
            {
                pom_ptr->common->rcvd_rstp  = TRUE;
            }

            break;

        case XSTP_TYPE_BPDU_TYPE_CONFIG:
        case XSTP_TYPE_BPDU_TYPE_TCN:
            if (bpdu_header->protocol_version_identifier < XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID)
            {
                pom_ptr->common->rcvd_stp   = TRUE;
            }
            break;
    }
    return;
} /* End of XSTP_UTY_UpdtBpduVersion */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_BetterOrSameInfoXst
 * ------------------------------------------------------------------------
 * PURPOSE  : 1. return TRUE if the received CIST priority vector is better
 *               than or the same as (13.10) the CIST port priority vector.
 *            2. returns TRUE if the MSTI priority vector is better than or
 *               the same as (13.11) the MSTI port priority vector.
 * INPUT    : om_ptr        -- om pointer for this instance
 *            lport         -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref to the description in 13.26.1, 13.26.2,
 *            IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_BetterOrSameInfoXst(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    BOOL_T              better_or_same_info_xst;
    int                 cmp_result;

    better_or_same_info_xst = FALSE;
    pom_ptr     = &(om_ptr->port_info[lport-1]);

    XSTP_OM_CMP_PRIORITY_VECTOR(cmp_result, pom_ptr->msg_priority, pom_ptr->port_priority);

    if (cmp_result <= 0)
        better_or_same_info_xst = TRUE;

    return better_or_same_info_xst;
} /* End of XSTP_UTY_BetterOrSameInfoXst */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_ClearAllRcvdMsgs
 * ------------------------------------------------------------------------
 * PURPOSE  : Clears rcvd_msg for the CIST and all MSTIs, for all Ports.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.3, IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
void  XSTP_UTY_ClearAllRcvdMsgs(void)
{
    UI32_T                  xstid;
    UI32_T                  lport;
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    xstid               = XSTP_TYPE_CISTID;
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    do
    {
        if (om_ptr->instance_exist)
        {
            /* Performance Improvement
            for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
            */
            lport = 0;
            while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                pom_ptr     = &(om_ptr->port_info[lport-1]);
                if (pom_ptr->is_member)
                    pom_ptr->rcvd_msg = FALSE;
            }
        }
    }while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &om_ptr));

    return;
} /* End of XSTP_UTY_ClearAllRcvdMsgs */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_ClearReselectTree
 * ------------------------------------------------------------------------
 * PURPOSE  : Clear reselect for the tree for all ports of the bridge.
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.4, IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_ClearReselectTree(XSTP_OM_InstanceData_T *om_ptr)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI32_T              lport;

    /* Performance Improvement
    for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
    */
    lport = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr     = &(om_ptr->port_info[lport-1]);
        if (pom_ptr->is_member)
        {
            pom_ptr->reselect = FALSE;
        }
    }

    return;
} /* End of XSTP_UTY_ClearReselectTree */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_FromSameRegion
 * ------------------------------------------------------------------------
 * PURPOSE  : Return TRUE if rcvd_rstp is TRUE, and the received BPDU
 *            conveys an MST Configuration Identifier that matches that
 *            held for the Bridge. Return FALSE otherwise.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : TRUE/FALSE
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.5, IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_FromSameRegion(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    return XSTP_OM_FromSameRegion(om_ptr, lport);

} /* End of XSTP_UTY_FromSameRegion */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_FromSameRegionAndUpdateReselect
 * ------------------------------------------------------------------------
 * PURPOSE  : Return TRUE if rcvd_rstp is TRUE, and the received BPDU
 *            conveys an MST Configuration Identifier that matches that
 *            held for the Bridge. Return FALSE otherwise.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : TRUE/FALSE
 * RETURN   : None
 * NOTE     : 1. Ref to the description in 13.26.5, IEEE Std 802.1s(D14.1)-2002
 *            2. If rcvd_internal is changed,
 *               (1)Set "reselect" flag, and
 *               (2)Reset "selected" flag.
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_FromSameRegionAndUpdateReselect(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    BOOL_T                  is_same_region;

    pom_ptr         = &(om_ptr->port_info[lport-1]);
    is_same_region  = XSTP_OM_FromSameRegion(om_ptr, lport);
    if (pom_ptr->common->rcvd_internal != is_same_region)
    {
        pom_ptr->reselect   = TRUE;
        pom_ptr->selected   = FALSE;
    }
    return is_same_region;

} /* End of XSTP_UTY_FromSameRegionAndUpdateReselect */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_NewTcWhile
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the value of tc_while
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport
 * OUTPUT   : tc_while  -- topology change timer
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.5 newTcWhile() IEEE Std 802.1Q-2005
 *-------------------------------------------------------------------------
 */
UI16_T  XSTP_UTY_NewTcWhile(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI16_T              tc_while;

    XSTP_UTY_IncTopologyChangeCount(om_ptr, lport);

    pom_ptr     = &(om_ptr->port_info[lport-1]);

    if (pom_ptr->tc_while == 0)
    {
        if (    (pom_ptr->common->oper_point_to_point_mac)
            &&  (pom_ptr->common->send_rstp)
           )
        {
            if (XSTP_UTY_Cist(om_ptr))
            {
                tc_while                           = pom_ptr->port_times.hello_time + 1;
                pom_ptr->common->new_info_cist     = TRUE;
            }
            else
            {
                tc_while                           = pom_ptr->cist->port_times.hello_time + 1;
                pom_ptr->common->new_info_msti     = TRUE;
            }
        }
        else
        {
            if (XSTP_UTY_Cist(om_ptr))
            {
                tc_while    =   om_ptr->bridge_info.root_times.max_age
                              + om_ptr->bridge_info.root_times.forward_delay;
            }
            else
            {
                tc_while    =   om_ptr->bridge_info.cist->root_times.max_age
                              + om_ptr->bridge_info.cist->root_times.forward_delay;
            }
        }
    }
    else
    {
        tc_while = pom_ptr->tc_while;
    }
    return tc_while;
} /* End of XSTP_UTY_NewTcWhile */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_NewEdgeDelayWhile
 * ------------------------------------------------------------------------
 * PURPOSE  : Return the edge delay upon a port's link type.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport
 * OUTPUT   : edge_delay_while  -- edge delay timer
 * RETURN   : None
 * NOTE     : Ref to the description in 17.20.4 IEEE Std 802.1D-2004
 *-------------------------------------------------------------------------
 */
UI16_T  XSTP_UTY_NewEdgeDelayWhile(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI16_T              edge_delay_while;

    pom_ptr     = &(om_ptr->port_info[lport-1]);

    if (pom_ptr->common->oper_point_to_point_mac == TRUE)
        edge_delay_while = XSTP_TYPE_DEFAULT_MIGRATE_TIME;
    else
        edge_delay_while = pom_ptr->designated_times.max_age;

    return edge_delay_while;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RcvInfoXst
 * ------------------------------------------------------------------------
 * PURPOSE  : 1. rcvInfoCist()
 *            2. rcvInfoMsti()
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : rcv_info
 *              -- XSTP_ENGINE_PORTVAR_RCVD_INFO_SUPERIOR_DESIGNATED_INFO
 *              -- XSTP_ENGINE_PORTVAR_RCVD_INFO_REPEATED_DESIGNATED_INFO
 *              -- XSTP_ENGINE_PORTVAR_RCVD_INFO_ROOT_INFO
 *              -- XSTP_ENGINE_PORTVAR_RCVD_INFO_OTHER_INFO
 * NOTE     : Ref to the description in 13.26.7, 13.26.8,
 *            IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
UI16_T  XSTP_UTY_RcvInfoXst(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    UI16_T              rvc_info;

    if (XSTP_UTY_Cist(om_ptr))
        rvc_info = XSTP_UTY_RcvInfoCist(om_ptr, lport);
    else
        rvc_info = XSTP_UTY_RcvInfoMsti(om_ptr, lport);

    return rvc_info;
}/* End of XSTP_UTY_RcvInfoXst */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RecordAgreementXst
 * ------------------------------------------------------------------------
 * PURPOSE  : 1. recordAgreementCist()
 *            2. recordAgreementCist()
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.9, 13.26.10,
 *            IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
void  XSTP_UTY_RecordAgreementXst(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    if (XSTP_UTY_Cist(om_ptr))
        XSTP_UTY_RecordAgreementCist(om_ptr, lport);
    else
        XSTP_UTY_RecordAgreementMsti(om_ptr, lport);

    return;
}/* End of XSTP_UTY_RecordAgreementXst */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RecordMasteredXst
 * ------------------------------------------------------------------------
 * PURPOSE  : 1. recordMasteredCist()
 *            2. recordMasteredMsti()
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.11, IEEE Std 802.1s(D13)-2002
 *            Ref to the description in 13.26.12, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
void  XSTP_UTY_RecordMasteredXst(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    if (pom_ptr->cist == NULL)
        XSTP_UTY_RecordMasteredCist(om_ptr, lport);
    else
        XSTP_UTY_RecordMasteredMsti(om_ptr, lport);

    return;
} /* End of XSTP_UTY_RecordMasteredXst */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RecordProposalXst
 * ------------------------------------------------------------------------
 * PURPOSE  : 1. recordProposalCist
 *            2. recordProposalMsti
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.13, IEEE Std 802.1s(D13)-2002
 *            Ref to the description in 13.26.14, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
void  XSTP_UTY_RecordProposalXst(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    if (pom_ptr->cist == NULL)
        XSTP_UTY_RecordProposalCist(om_ptr, lport);
    else
        XSTP_UTY_RecordProposalMsti(om_ptr, lport);

    return;
} /* End of XSTP_UTY_RecordProposalXst */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetRcvdMsgs
 * ------------------------------------------------------------------------
 * PURPOSE  : Set rcvdMsg for CIST and related MSTI
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. Ref to the description in 13.26.15, IEEE Std 802.1s(D14.1)-2002
 *            2. For our implementation consideration the procedure is placed
 *               in XSTP_RX_RecordBpdu for processing the received BPDU according
 *               to 802.1w and 802.1s.
 *-------------------------------------------------------------------------
 */
void  XSTP_UTY_SetRcvdMsgs(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    XSTP_TYPE_Bpdu_T    *bpdu;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    bpdu        = pom_ptr->common->bpdu;

    XSTP_RX_RecordBpdu(om_ptr, lport, bpdu);

    return;
}/* End of XSTP_UTY_SetRcvdMsgs */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetReRootTree
 * ------------------------------------------------------------------------
 * PURPOSE  : Set re_root TRUE for this tree for all ports of the bridge.
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.16, IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetReRootTree(XSTP_OM_InstanceData_T *om_ptr)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI32_T              lport;

    /* Performance Improvement
    for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
    */
    lport = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr     = &(om_ptr->port_info[lport-1]);
        if (pom_ptr->is_member)
        {
            pom_ptr->re_root = TRUE;
        }
    }

    return;
} /* End of XSTP_UTY_SetReRootTree */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetSelectedTree
 * ------------------------------------------------------------------------
 * PURPOSE  : Sets selected TRUE for this tree for all Ports of the Bridge.
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.17, IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetSelectedTree(XSTP_OM_InstanceData_T *om_ptr)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI32_T              lport;

    /* Performance Improvement
    for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
    */
    lport = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr     = &(om_ptr->port_info[lport-1]);
        if (pom_ptr->is_member)
        {
            pom_ptr->selected = TRUE;
        }
    }

    return;
} /* End of XSTP_UTY_SetSelectedTree */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetSyncTree
 * ------------------------------------------------------------------------
 * PURPOSE  : Sets sync TRUE for this tree for all Ports of the Bridge.
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.18, IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetSyncTree(XSTP_OM_InstanceData_T *om_ptr)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI32_T              lport;

    /* Performance Improvement
    for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
    */
    lport = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr     = &(om_ptr->port_info[lport-1]);
        if (pom_ptr->is_member)
        {
            pom_ptr->sync = TRUE;
        }
    }

    return;
} /* End of XSTP_UTY_SetSyncTree */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetTcFlags
 * ------------------------------------------------------------------------
 * PURPOSE  : Set tc flags
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. Ref to the description in 13.26.19, IEEE Std 802.1s(D14.1)-2002
 *            2. This procedure is invoked only for CIST because PRX state
 *               machine which invokes this procedure is implemented as a
 *               common state machine triggered in CIST.
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetTcFlags(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_InstanceData_T      *temp_om_ptr;
    XSTP_OM_PortVar_T           *temp_pom_ptr;
    XSTP_OM_PortVar_T           *pom_ptr;
    XSTP_TYPE_BpduHeader_T      *bpdu_header;
    XSTP_TYPE_MstBpdu_T         *bpdu;
    XSTP_TYPE_MstiConfigMsg_T   *msti_config_msg;
    UI32_T                      xstid;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    bpdu_header = (XSTP_TYPE_BpduHeader_T*)pom_ptr->common->bpdu;
    bpdu        = (XSTP_TYPE_MstBpdu_T*)pom_ptr->common->bpdu;

    switch (bpdu_header->bpdu_type)
    {
        case XSTP_TYPE_BPDU_TYPE_TCN:
            pom_ptr->common->rcvd_tcn   = TRUE;

            xstid               = XSTP_TYPE_CISTID;
            temp_om_ptr         = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);

            while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &temp_om_ptr))
            {
                if (temp_om_ptr->instance_exist)
                {
                    temp_pom_ptr                    = &(temp_om_ptr->port_info[lport-1]);
                    if (temp_pom_ptr->is_member)
                        temp_pom_ptr->rcvd_tc       = TRUE;
                }
            }
            break;

        case XSTP_TYPE_BPDU_TYPE_CONFIG:
        case XSTP_TYPE_BPDU_TYPE_XST:
            /* 13.26.19 (a) */
            if ((bpdu->cist_flags & XSTP_TYPE_BPDU_FLAGS_TCA) != 0)
                pom_ptr->common->rcvd_tc_ack = TRUE;

            /* 13.26.19 (c) */
            if (pom_ptr->common->rcvd_internal)
            {
                if ((bpdu->cist_flags & XSTP_TYPE_BPDU_FLAGS_TC) != 0)
                {
                    /* Set rcvdTc for the CIST if the Topology Change flag is set in the CST message. */
                    temp_om_ptr             = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
                    temp_pom_ptr            = &(temp_om_ptr->port_info[lport-1]);
                    temp_pom_ptr->rcvd_tc   = TRUE;
                } /* End of if (TC bit is set in the flag of the CST message) */

                /* Set rcvdTc for each MSTI for which the Topology Change flag is set in the corresponding MSTI message.*/
                xstid               = XSTP_TYPE_CISTID;
                temp_om_ptr         = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);

                while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &temp_om_ptr))
                {
                    if (temp_om_ptr->instance_exist)
                    {
                        temp_pom_ptr    = &(temp_om_ptr->port_info[lport-1]);
                        if (    (temp_pom_ptr->is_member)
                             && (temp_pom_ptr->rcvd_msg)
                           )
                        {
                            msti_config_msg = temp_pom_ptr->msti_config_msg;
                            if ((msti_config_msg->msti_flags & XSTP_TYPE_BPDU_FLAGS_TC) != 0)
                            {
                                temp_pom_ptr->rcvd_tc   = TRUE;
                            } /* End of if (TC bit is set in the flag of this MSTI configuration message) */
                        } /* End of if (the port is a member of this MSTI and the BPDU carries message for this MSTI) */
                    } /* End of if (this instance is existent) */
                } /* End of for (all MSTI) */
            }
            else
            /* 13.26.19 (b) */
            {
                if ((bpdu->cist_flags & XSTP_TYPE_BPDU_FLAGS_TC) != 0)
                {
                    xstid               = XSTP_TYPE_CISTID;
                    temp_om_ptr         = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
                    do
                    {
                        if (temp_om_ptr->instance_exist)
                        {
                            temp_pom_ptr                = &(temp_om_ptr->port_info[lport-1]);
                            if (temp_pom_ptr->is_member)
                                temp_pom_ptr->rcvd_tc   = TRUE;
                        } /* End of if (this instance is existent) */
                    }while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &temp_om_ptr));
                } /* End of if (TC bit is set in the flag of the CST message) */
            }

            break;
    } /* End of switch (bpdu_type) */

    return;
}/* End of XSTP_UTY_SetTcFlags */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetTcPropTree
 * ------------------------------------------------------------------------
 * PURPOSE  : Sets tc_prop TRUE for the given tree for all ports
 *            except the Port.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport (1..max)
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.20, IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetTcPropTree(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI32_T              next_lport;
#if (SYS_CPNT_XSTP_TC_PROP_GROUP==TRUE)
    UI32_T tc_group_id;
#endif

#if (SYS_CPNT_XSTP_TC_PROP_GROUP==TRUE)
    tc_group_id = XSTP_OM_GetPropGropIdByPort(lport);
#endif

    /* Performance Improvement
    for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
    */
    next_lport = 0;
    while (SWCTRL_GetNextLogicalPort(&next_lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr     = &(om_ptr->port_info[next_lport-1]);
        if ( (next_lport != lport) && pom_ptr->is_member
#if (SYS_CPNT_XSTP_TC_PROP_STOP == TRUE)
            && !XSTP_OM_IsPortTcPropStop(next_lport)
#endif
#if (SYS_CPNT_XSTP_TC_PROP_GROUP==TRUE)
            &&(tc_group_id == XSTP_OM_GetPropGropIdByPort(next_lport))
#endif
           )
        {
            pom_ptr->tc_prop = TRUE;
        }
    }

    return;
} /* End of XSTP_UTY_SetTcPropTree */

#if 0
/* Old implementation according to IEEE Std 802.1s-2002 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_TxConfig
 * ------------------------------------------------------------------------
 * PURPOSE  : Transmits a Configuration BPDU.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. Ref to the description in 13.26.21, IEEE Std 802.1s(D14.1)-2002
 *            2. This procedure is invoked only for CIST.
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_TxConfig(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_ConfigBpdu_T  bpdu;

    if (XSTP_UTY_Cist(om_ptr) == FALSE)
        return;

    pom_ptr     = &(om_ptr->port_info[lport-1]);

    /* Priority Vector */
    XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu.root_identifier, pom_ptr->port_priority.root_id);
    bpdu.root_path_cost = L_STDLIB_Hton32(pom_ptr->port_priority.ext_root_path_cost);
    XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu.bridge_identifier, pom_ptr->port_priority.designated_bridge_id);
    bpdu.port_identifier.port_id = L_STDLIB_Hton16(pom_ptr->port_priority.designated_port_id.port_id);

    /* Flags */
    bpdu.flags  = 0;
    if (pom_ptr->tc_while != 0)
    {
        bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_TC;
    }
    if (pom_ptr->common->tc_ack)
    {
        bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_TCA;
    }

    /* Timers */
    bpdu.message_age    = L_STDLIB_Hton16(pom_ptr->port_times.message_age   * XSTP_TYPE_BPDU_TIME_UNIT);
    bpdu.max_age        = L_STDLIB_Hton16(pom_ptr->port_times.max_age       * XSTP_TYPE_BPDU_TIME_UNIT);
    bpdu.hello_time     = L_STDLIB_Hton16(pom_ptr->port_times.hello_time    * XSTP_TYPE_BPDU_TIME_UNIT);
    bpdu.forward_delay  = L_STDLIB_Hton16(pom_ptr->port_times.forward_delay * XSTP_TYPE_BPDU_TIME_UNIT);

    XSTP_UTY_SendConfigBpdu(lport, &bpdu);

    return;
} /* End of XSTP_UTY_TxConfig */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_TxMstp
 * ------------------------------------------------------------------------
 * PURPOSE  : Transmits an MST BPDU
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.22, IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_TxMstp(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_InstanceData_T      *cist_om_ptr;
    XSTP_OM_PortVar_T           *cist_pom_ptr;
    XSTP_OM_InstanceData_T      *temp_om_ptr;
    XSTP_OM_PortVar_T           *temp_pom_ptr;
    XSTP_TYPE_MstBpdu_T         *bpdu;
    XSTP_TYPE_MstiConfigMsg_T   *msti_config_msg;
    UI16_T                      version_3_length;
    UI32_T                      mstid, index;
    UI8_T                       force_version;
    XSTP_TYPE_BridgeId_T        bridge_identifier;

    force_version = XSTP_OM_GetForceVersion();
    if (force_version < 2)
    {
        return;
    }
    /* version 3 length */
    version_3_length    = XSTP_UTY_GetVersion3length(lport);
    if ((bpdu = L_MM_Malloc(version_3_length + 38 + 3, L_MM_USER_ID2(SYS_MODULE_XSTP, XSTP_TYPE_TRACE_ID_XSTP_UTY_TXMSTP)) == NULL)
    {
        /* Error */
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("\r\nXSTP_UTY_TxMstp::ERROR!! for L_MM_Malloc return NULL");
        return;
    }
    cist_om_ptr     = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    cist_pom_ptr    = &(cist_om_ptr->port_info[lport-1]);

    /* Flags */
    bpdu->cist_flags    = 0;
    /* 14.6(a) */
    /* Bit 1 of Octet 5 */
    if (cist_pom_ptr->tc_while != 0)
    {
        bpdu->cist_flags |= XSTP_TYPE_BPDU_FLAGS_TC;
    }
    /* 14.6(b) */
    /* Bit 2 of Octet 5 */
    if (cist_pom_ptr->proposing)
    {
        bpdu->cist_flags |= XSTP_TYPE_BPDU_FLAGS_PROPOSAL;
    }
    /* 14.6(c) */
    /* Bits 3, 4 of Octet 5 */
    switch (cist_pom_ptr->role)
    {
        case XSTP_ENGINE_PORTVAR_ROLE_ROOT:
            bpdu->cist_flags |= XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ROOT;
            break;
        case XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED:
            bpdu->cist_flags |= XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED;
            break;
        case XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE:
        case XSTP_ENGINE_PORTVAR_ROLE_BACKUP:
            bpdu->cist_flags |= XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ALT_BAK;
            break;
    }
    /* 14.6(d) */
    /* Bit 5 of Octet 5 */
    if (cist_pom_ptr->learning)
    {
        bpdu->cist_flags |= XSTP_TYPE_BPDU_FLAGS_LEARNING;
    }
    /* 14.6(e) */
    /* Bit 6 of Octet 5 */
    if (cist_pom_ptr->forwarding)
    {
        bpdu->cist_flags |= XSTP_TYPE_BPDU_FLAGS_FORWARDING;
    }
    /* 14.6(f) */
    /* Bit 7 of Octet 5 */
    /* change agreed to agree */
    if (cist_pom_ptr->agree)
    {
        bpdu->cist_flags |= XSTP_TYPE_BPDU_FLAGS_AGREEMENT;
    }
    /* 14.6(g) */
    /* Bit 8 of Octet 5 */
    /* Unused : shall be 0 */

    /* Priority Vector */
    /* 14.6(h) */
    /* Octets  6 ~ 13 */
    XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu->cist_root_identifier, cist_pom_ptr->port_priority.root_id);
    /* 14.6(i) */
    /* Octets 14 ~ 17 */
    bpdu->cist_external_path_cost = L_STDLIB_Hton32(cist_pom_ptr->port_priority.ext_root_path_cost);
    /* 14.6(j) */
    /* Octets 18 ~ 25 */
    XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu->cist_regional_root_identifier, cist_pom_ptr->port_priority.r_root_id);
    /* 14.6(k) */
    /* Octets 26, 27 : convey the CIST Port ID of the transmitting Bridge Port */
#if (SYS_CPNT_XSTP_PATCH_BPDU_FORMAT)
    /* The old method: to convey the CIST Port ID of the Designated Port */
    bpdu->cist_port_identifier.port_id = L_STDLIB_Hton16(cist_pom_ptr->port_priority.designated_port_id.port_id);
#else
    bpdu->cist_port_identifier.port_id = L_STDLIB_Hton16(cist_pom_ptr->port_id.port_id);
#endif

    /* Timers */
    /* 14.6(l) */
    /* Octets 28, 29 */
    bpdu->message_age    = L_STDLIB_Hton16(cist_pom_ptr->port_times.message_age   * XSTP_TYPE_BPDU_TIME_UNIT);
    /* 14.6(m) */
    /* Octets 30, 31 */
    bpdu->max_age        = L_STDLIB_Hton16(cist_pom_ptr->port_times.max_age       * XSTP_TYPE_BPDU_TIME_UNIT);
    /* 14.6(n) */
    /* Octets 32, 33 */
    bpdu->hello_time     = L_STDLIB_Hton16(cist_pom_ptr->port_times.hello_time    * XSTP_TYPE_BPDU_TIME_UNIT);
    /* 14.6(o) */
    /* Octets 34, 35 */
    bpdu->forward_delay  = L_STDLIB_Hton16(cist_pom_ptr->port_times.forward_delay * XSTP_TYPE_BPDU_TIME_UNIT);

    /* 14.6(p) */
    /* Octet 36 */
    /* Length */
    bpdu->version_1_length   = 0x00;

    if (force_version == 2)
    {
        XSTP_TYPE_RstBpdu_T     rstp_bpdu_ptr;
        memcpy(&rstp_bpdu_ptr, bpdu, sizeof(XSTP_TYPE_RstBpdu_T) );
        L_MM_Free(bpdu);
        XSTP_UTY_SendRstpBpdu(lport, &rstp_bpdu_ptr);
    }
    else if (force_version == 3)
    {
        /* 13.26.22 (a) version 3 length */
        /* 14.6(q) */
        /* Octets 37, 38 */
        bpdu->version_3_length   = L_STDLIB_Hton16(version_3_length);

        /* 13.26.22 (b) mst_config_id */
        /* 14.6(r) */
        /* Octets 39 ~ 89 */
        bpdu->mst_configuration_identifier.config_id_format_selector = om_ptr->bridge_info.common->mst_config_id.config_id_format_selector;
        memcpy( (UI8_T *)&(bpdu->mst_configuration_identifier.config_name),
                (UI8_T *)&(om_ptr->bridge_info.common->mst_config_id.config_name),
                XSTP_TYPE_REGION_NAME_MAX_LENGTH);
        bpdu->mst_configuration_identifier.revision_level = L_STDLIB_Hton16(om_ptr->bridge_info.common->mst_config_id.revision_level);
        memcpy( (UI8_T *)&(bpdu->mst_configuration_identifier.config_digest),
                (UI8_T *)&(om_ptr->bridge_info.common->mst_config_id.config_digest),
                16);

        /* 13.26.22 (c) cist internal root path cost */
        /* 14.6(s) */
        /* Octets 90 ~ 93 */
        bpdu->cist_internal_root_path_cost = L_STDLIB_Hton32(cist_pom_ptr->port_priority.int_root_path_cost);

        /* 13.26.22 (d) cist bridge identifier */
        /* 14.6(t) the CIST Bridge Identifier of the transmitting Bridge */
        /* Octets 94 ~ 101 */
#if (SYS_CPNT_XSTP_PATCH_BPDU_FORMAT)
        /* Using the designated bridge ID as the CIST Bridge Identifier */
        XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu->cist_bridge_identifier, cist_pom_ptr->port_priority.designated_bridge_id);
#else
        /* Using the transmitting bridge as the CIST Bridge Identifier */
        memcpy(&bridge_identifier,
               &om_ptr->bridge_info.bridge_identifier,
               XSTP_TYPE_BRIDGE_ID_LENGTH);
        bridge_identifier.bridge_id_priority.data.system_id_ext = 0;
        XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu->cist_bridge_identifier, bridge_identifier);
#endif

        /* 13.26.22 (e) remaining_hops */
        /* 14.6(u) */
        /* Octet 102 */
        bpdu->cist_remaining_hops  = cist_pom_ptr->port_times.remaining_hops;

        /* 13.26.22 (f) MSTI message, encoded in MSTID order XXX */
        mstid               = XSTP_TYPE_CISTID;
        index               = 0;
        temp_om_ptr         = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
        while(XSTP_OM_GetNextInstanceInfoPtr(&mstid, &temp_om_ptr))
        {
            if (temp_om_ptr->instance_exist)
            {
                temp_pom_ptr    = &(temp_om_ptr->port_info[lport-1]);
                if (temp_pom_ptr->is_member)
                {
                    /* 14.6.1 */
                    msti_config_msg = &(bpdu->msti_configureation_message[index]);

                    /* Flags */
                    msti_config_msg->msti_flags = 0;
                    /* Bit 1 of Octet 1 */
                    if (temp_pom_ptr->tc_while != 0)
                    {
                        msti_config_msg->msti_flags |= XSTP_TYPE_BPDU_FLAGS_TC;
                    }
                    /* Bit 2 of Octet 1 */
                    if (temp_pom_ptr->proposing)
                    {
                        msti_config_msg->msti_flags |= XSTP_TYPE_BPDU_FLAGS_PROPOSAL;
                    }
                    /* Bits 3, 4 of Octet 1 */
                    switch (temp_pom_ptr->role)
                    {
                        case XSTP_ENGINE_PORTVAR_ROLE_ROOT:
                            msti_config_msg->msti_flags |= XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ROOT;
                            break;
                        case XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED:
                            msti_config_msg->msti_flags |= XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED;
                            break;
                        case XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE:
                        case XSTP_ENGINE_PORTVAR_ROLE_BACKUP:
                            msti_config_msg->msti_flags |= XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ALT_BAK;
                            break;
                    }
                    /* Bit 5 of Octet 1 */
                    if (temp_pom_ptr->learning)
                    {
                        msti_config_msg->msti_flags |= XSTP_TYPE_BPDU_FLAGS_LEARNING;
                    }
                    /* Bit 6 of Octet 1 */
                    if (temp_pom_ptr->forwarding)
                    {
                        msti_config_msg->msti_flags |= XSTP_TYPE_BPDU_FLAGS_FORWARDING;
                    }
                    /* Bit 7 of Octet 1 */
                    /* change agreed to agree */
                    if (temp_pom_ptr->agree)
                    {
                        msti_config_msg->msti_flags |= XSTP_TYPE_BPDU_FLAGS_AGREEMENT;
                    }
                    /* Bit 8 of Octet 1 */
                    if (temp_pom_ptr->msti_master)
                    {
                        msti_config_msg->msti_flags |= XSTP_TYPE_BPDU_FLAGS_MASTER;
                    }

                    /* 14.6.1(b) */
                    /* Octets 2 ~ 9 */
                    /* MSTI Regional Root ID */
                    XSTP_OM_CPY_HTON_BRIDGE_ID(msti_config_msg->msti_regional_root_identifier, temp_pom_ptr->port_priority.r_root_id);
                    /* 14.6.1(c) */
                    /* Octets 10 ~ 13 */
                    /* MSTI Internal Root Path Cost */
                    msti_config_msg->msti_internal_root_path_cost
                        = L_STDLIB_Hton32(temp_pom_ptr->port_priority.int_root_path_cost);

                    #if (SYS_CPNT_XSTP_PATCH_BPDU_FORMAT)
                    {
                        /* 14.6.1(d) */
                        /* Octet 14 ~ 21 */
                        /* MSTI Bridge Identifier */
                        XSTP_OM_CPY_HTON_BRIDGE_ID(msti_config_msg->msti_bridge_id, temp_pom_ptr->port_priority.designated_bridge_id);
                        /* Octet 22 ~ 23 */
                        /* MSTI Port Identifier */
                        msti_config_msg->msti_port_id.port_id = L_STDLIB_Hton16(temp_pom_ptr->port_priority.designated_port_id.port_id);
                    }
                    #else
                    {
                        UI16_T  temp_priority;
                        /* 14.6.1(d) */
                        /* Bits 5 ~ 8 of Octet 14 */
                        /* MSTI Bridge Identifier Priority */
                        XSTP_OM_GET_BRIDGE_ID_PRIORITY(temp_priority, temp_om_ptr->bridge_info.bridge_identifier);
                        msti_config_msg->msti_bridge_priority = 0xF0 & ((UI8_T)(temp_priority >> 8));
                        /* 14.6.1(e) */
                        /* Bits 5 ~ 8 of Octet 15 */
                        /* MSTI Port Identifier Priority */
                        XSTP_OM_GET_PORT_ID_PRIORITY(msti_config_msg->msti_port_priority, temp_pom_ptr->port_id);
                        msti_config_msg->msti_port_priority &= 0xF0;
                    }
                    #endif

                    /* 14.6.1(f) */
                    /* No patch: Octet 16
                       Patch: Octet 24
                    */
                    /* MSTI Remaining Hops */
                    msti_config_msg->msti_remaining_hops
                        = temp_pom_ptr->port_times.remaining_hops;

                    index++;
                }
            }
        } /* End of for */

        XSTP_UTY_SendMstpBpdu(lport, bpdu);
    }
    return;
}/* End of XSTP_UTY_TxMstp */
#endif

/* New implementation according to IEEE Std 802.1D/D3 June 11, 2003, page 161.
   -- XSTP_UTY_TxConfig()
   -- XSTP_UTY_TxMstp()
*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_TxConfig
 * ------------------------------------------------------------------------
 * PURPOSE  : Transmits a Configuration BPDU.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. Ref to the description in 13.26.21, IEEE Std 802.1s(D14.1)-2002
 *               Ref to the description in 17.21.19, IEEE Std 802.1D/D3 June 11, 2003
 *            2. This procedure is invoked only for CIST.
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_TxConfig(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_ConfigBpdu_T  bpdu;

    if (XSTP_UTY_Cist(om_ptr) == FALSE)
        return;

    pom_ptr     = &(om_ptr->port_info[lport-1]);

    /* Priority Vector: designated priority vector. Follow 1D/D3, page 161 */
    XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu.root_identifier, pom_ptr->designated_priority.root_id);
    bpdu.root_path_cost = L_STDLIB_Hton32(pom_ptr->designated_priority.ext_root_path_cost);
    XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu.bridge_identifier, pom_ptr->designated_priority.designated_bridge_id);
    bpdu.port_identifier.port_id = L_STDLIB_Hton16(pom_ptr->designated_priority.designated_port_id.port_id);

    /* Flags */
    bpdu.flags  = 0;
    if (pom_ptr->tc_while != 0)
    {
        bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_TC;
    }
    if (pom_ptr->common->tc_ack)
    {
        bpdu.flags |= XSTP_TYPE_BPDU_FLAGS_TCA;
    }

    /* Timers: designated_times. Follow 1D/D3, page 161 */
    bpdu.message_age    = L_STDLIB_Hton16(pom_ptr->designated_times.message_age   * XSTP_TYPE_BPDU_TIME_UNIT);
    bpdu.max_age        = L_STDLIB_Hton16(pom_ptr->designated_times.max_age       * XSTP_TYPE_BPDU_TIME_UNIT);
    bpdu.hello_time     = L_STDLIB_Hton16(pom_ptr->designated_times.hello_time    * XSTP_TYPE_BPDU_TIME_UNIT);
    bpdu.forward_delay  = L_STDLIB_Hton16(pom_ptr->designated_times.forward_delay * XSTP_TYPE_BPDU_TIME_UNIT);

    XSTP_UTY_SendConfigBpdu(lport, &bpdu);

    return;
} /* End of XSTP_UTY_TxConfig */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_TxMstp
 * ------------------------------------------------------------------------
 * PURPOSE  : Transmits an MST BPDU
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.22, IEEE Std 802.1s(D14.1)-2002
*             Ref to the description in 17.21.19, IEEE Std 802.1D/D3 June 11, 2003
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_TxMstp(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_InstanceData_T      *cist_om_ptr;
    XSTP_OM_PortVar_T           *cist_pom_ptr;
    XSTP_OM_InstanceData_T      *temp_om_ptr;
    XSTP_OM_PortVar_T           *temp_pom_ptr;
    XSTP_TYPE_MstBpdu_T         *bpdu;
    XSTP_TYPE_MstiConfigMsg_T   *msti_config_msg;
    UI16_T                      version_3_length;
    UI32_T                      mstid, index;
    UI8_T                       force_version;
    UI16_T                      temp_priority;
#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
    UI32_T                      cisco_prestandard_status;
#endif /* End of #if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE) */

    force_version = XSTP_OM_GetForceVersion();
    if (force_version < 2)
    {
        return;
    }
    /* version 3 length */
    version_3_length    = XSTP_UTY_GetVersion3length(lport);
    if ((bpdu = L_MM_Malloc(version_3_length + 38 + 3, L_MM_USER_ID2(SYS_MODULE_XSTP, XSTP_TYPE_TRACE_ID_XSTP_UTY_TXMSTP))) == NULL)
    {
        /* Error */
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("\r\nXSTP_UTY_TxMstp::ERROR!! for L_MM_Malloc return NULL");
        return;
    }
    cist_om_ptr     = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    cist_pom_ptr    = &(cist_om_ptr->port_info[lport-1]);

    /* Flags */
    bpdu->cist_flags    = 0;
    /* 14.6(a) */
    /* Bit 1 of Octet 5 */
    if (cist_pom_ptr->tc_while != 0)
    {
        bpdu->cist_flags |= XSTP_TYPE_BPDU_FLAGS_TC;
    }
    /* 14.6(b) */
    /* Bit 2 of Octet 5 */
    if (cist_pom_ptr->proposing)
    {
        bpdu->cist_flags |= XSTP_TYPE_BPDU_FLAGS_PROPOSAL;
    }
    /* 14.6(c) */
    /* Bits 3, 4 of Octet 5 */
    switch (cist_pom_ptr->role)
    {
        case XSTP_ENGINE_PORTVAR_ROLE_ROOT:
            bpdu->cist_flags |= XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ROOT;
            break;
        case XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED:
            bpdu->cist_flags |= XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED;
            break;
        case XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE:
        case XSTP_ENGINE_PORTVAR_ROLE_BACKUP:
            bpdu->cist_flags |= XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ALT_BAK;
            break;
    }
    /* 14.6(d) */
    /* Bit 5 of Octet 5 */
    if (cist_pom_ptr->learning)
    {
        bpdu->cist_flags |= XSTP_TYPE_BPDU_FLAGS_LEARNING;
    }
    /* 14.6(e) */
    /* Bit 6 of Octet 5 */
    if (cist_pom_ptr->forwarding)
    {
        bpdu->cist_flags |= XSTP_TYPE_BPDU_FLAGS_FORWARDING;
    }
    /* 14.6(f) */
    /* Bit 7 of Octet 5 */
    /* change agreed to agree */
    if (cist_pom_ptr->agree)
    {
        bpdu->cist_flags |= XSTP_TYPE_BPDU_FLAGS_AGREEMENT;
    }
    /* 14.6(g) */
    /* Bit 8 of Octet 5 */
    /* Unused : shall be 0 */

#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
    XSTP_OM_GetCiscoPrestandardCompatibility(&cisco_prestandard_status);
    if (    (cisco_prestandard_status == XSTP_TYPE_CISCO_PRESTANDARD_COMPATIBILITY_DISABLED)
        ||  (   (cisco_prestandard_status == XSTP_TYPE_CISCO_PRESTANDARD_COMPATIBILITY_ENABLED)
             && (cist_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
            )
       )
#endif /* End of #if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE) */
    {
        /* Priority Vector: designated priority vector. Follow 1D/D3, page 161 */
        /* 14.6(h) */
        /* Octets  6 ~ 13 */
        XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu->cist_root_identifier, cist_pom_ptr->designated_priority.root_id);
        /* 14.6(i) */
        /* Octets 14 ~ 17 */
        bpdu->cist_external_path_cost = L_STDLIB_Hton32(cist_pom_ptr->designated_priority.ext_root_path_cost);
        /* 14.6(j) */
        /* Octets 18 ~ 25 */
        XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu->cist_regional_root_identifier, cist_pom_ptr->designated_priority.r_root_id);
        /* 14.6(k) */
        /* Octets 26, 27 : convey the CIST Port ID of the transmitting Bridge Port */
        bpdu->cist_port_identifier.port_id = L_STDLIB_Hton16(cist_pom_ptr->designated_priority.designated_port_id.port_id);

        /* Timers: designated_times. Follow 1D/D3, page 161 */
        /* 14.6(l) */
        /* Octets 28, 29 */
        bpdu->message_age    = L_STDLIB_Hton16(cist_pom_ptr->designated_times.message_age   * XSTP_TYPE_BPDU_TIME_UNIT);
        /* 14.6(m) */
        /* Octets 30, 31 */
        bpdu->max_age        = L_STDLIB_Hton16(cist_pom_ptr->designated_times.max_age       * XSTP_TYPE_BPDU_TIME_UNIT);
        /* 14.6(n) */
        /* Octets 32, 33 */
        bpdu->hello_time     = L_STDLIB_Hton16(cist_pom_ptr->designated_times.hello_time    * XSTP_TYPE_BPDU_TIME_UNIT);
        /* 14.6(o) */
        /* Octets 34, 35 */
        bpdu->forward_delay  = L_STDLIB_Hton16(cist_pom_ptr->designated_times.forward_delay * XSTP_TYPE_BPDU_TIME_UNIT);
    }
#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
    else
    {
        /* Priority Vector: use port priority vector.
         * Cisco switch runs prestandard mstp set agreed flag, if and only if
         * -- Received message priority vector is same as port priority vector itself.
         * In order to let cisco designated port accept BPDUs with aggrement flag on and then enter forwarding immediately.
         * The first six components of CIST message priority vector conveyed in the BPDU are set to the value of
         * CIST port priority vector for this port.
         */
        /* 14.6(h) */
        /* Octets  6 ~ 13 */
        XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu->cist_root_identifier, cist_pom_ptr->port_priority.root_id);
        /* 14.6(i) */
        /* Octets 14 ~ 17 */
        bpdu->cist_external_path_cost = L_STDLIB_Hton32(cist_pom_ptr->port_priority.ext_root_path_cost);
        /* 14.6(j) */
        /* Octets 18 ~ 25 */
        XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu->cist_regional_root_identifier, cist_pom_ptr->port_priority.r_root_id);
        /* 14.6(k) */
        /* Octets 26, 27 : convey the CIST Port ID of the transmitting Bridge Port */
        bpdu->cist_port_identifier.port_id = L_STDLIB_Hton16(cist_pom_ptr->port_priority.designated_port_id.port_id);

        /* Timers: port_times. */
        /* 14.6(l) */
        /* Octets 28, 29 */
        bpdu->message_age    = L_STDLIB_Hton16(cist_pom_ptr->port_times.message_age   * XSTP_TYPE_BPDU_TIME_UNIT);
        /* 14.6(m) */
        /* Octets 30, 31 */
        bpdu->max_age        = L_STDLIB_Hton16(cist_pom_ptr->port_times.max_age       * XSTP_TYPE_BPDU_TIME_UNIT);
        /* 14.6(n) */
        /* Octets 32, 33 */
        bpdu->hello_time     = L_STDLIB_Hton16(cist_pom_ptr->port_times.hello_time    * XSTP_TYPE_BPDU_TIME_UNIT);
        /* 14.6(o) */
        /* Octets 34, 35 */
        bpdu->forward_delay  = L_STDLIB_Hton16(cist_pom_ptr->port_times.forward_delay * XSTP_TYPE_BPDU_TIME_UNIT);
    } /* End of cisco_prestandard_status */
#endif /* End of #if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE) */
    /* 14.6(p) */
    /* Octet 36 */
    /* Length */
    bpdu->version_1_length   = 0x00;

    if (force_version == 2)
    {
        XSTP_TYPE_RstBpdu_T     rstp_bpdu_ptr;
        memcpy(&rstp_bpdu_ptr, bpdu, sizeof(XSTP_TYPE_RstBpdu_T) );
        L_MM_Free(bpdu);
        XSTP_UTY_SendRstpBpdu(lport, &rstp_bpdu_ptr);
    }
    else if (force_version == 3)
    {
        /* 13.26.22 (a) version 3 length */
        /* 14.6(q) */
        /* Octets 37, 38 */
        bpdu->version_3_length   = L_STDLIB_Hton16(version_3_length);

        /* 13.26.22 (b) mst_config_id */
        /* 14.6(r) */
        /* Octets 39 ~ 89 */
        bpdu->mst_configuration_identifier.config_id_format_selector = om_ptr->bridge_info.common->mst_config_id.config_id_format_selector;
        memcpy( (UI8_T *)&bpdu->mst_configuration_identifier.config_name,
                (UI8_T *)&om_ptr->bridge_info.common->mst_config_id.config_name,
                XSTP_TYPE_REGION_NAME_MAX_LENGTH);
        bpdu->mst_configuration_identifier.revision_level = L_STDLIB_Hton16(om_ptr->bridge_info.common->mst_config_id.revision_level);
        memcpy( (UI8_T *)&bpdu->mst_configuration_identifier.config_digest,
                (UI8_T *)&om_ptr->bridge_info.common->mst_config_id.config_digest,
                16);

        /* 13.26.22 (c) cist internal root path cost */
        /* 14.6(s) */
        /* Octets 90 ~ 93 */
#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
    if (    (cisco_prestandard_status == XSTP_TYPE_CISCO_PRESTANDARD_COMPATIBILITY_DISABLED)
        ||  (   (cisco_prestandard_status == XSTP_TYPE_CISCO_PRESTANDARD_COMPATIBILITY_ENABLED)
             && (cist_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
            )
       )
#endif /* End of #if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE) */
    {
            bpdu->cist_internal_root_path_cost = L_STDLIB_Hton32(cist_pom_ptr->designated_priority.int_root_path_cost);

            /* 13.26.22 (d) cist bridge identifier */
            /* 14.6(t) the CIST Bridge Identifier of the transmitting Bridge */
            /* Octets 94 ~ 101 */
            /* Using the transmitting bridge as the CIST Bridge Identifier */
            XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu->cist_bridge_identifier, cist_pom_ptr->designated_priority.designated_bridge_id);

            /* 13.26.22 (e) remaining_hops */
            /* 14.6(u) */
            /* Octet 102 */
            bpdu->cist_remaining_hops  = cist_pom_ptr->designated_times.remaining_hops;
    }
#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
    else
    {
            bpdu->cist_internal_root_path_cost = L_STDLIB_Hton32(cist_pom_ptr->port_priority.int_root_path_cost);

            /* 13.26.22 (d) cist bridge identifier */
            /* 14.6(t) the CIST Bridge Identifier of the transmitting Bridge */
            /* Octets 94 ~ 101 */
            /* Using the transmitting bridge as the CIST Bridge Identifier */
            XSTP_OM_CPY_HTON_BRIDGE_ID(bpdu->cist_bridge_identifier, cist_pom_ptr->port_priority.designated_bridge_id);

            /* 13.26.22 (e) remaining_hops */
            /* 14.6(u) */
            /* Octet 102 */
            bpdu->cist_remaining_hops  = cist_pom_ptr->port_times.remaining_hops;
    } /* End of cisco_prestandard_status */
#endif /* End of #if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE) */

        /* 13.26.22 (f) MSTI message, encoded in MSTID order XXX */
        mstid               = XSTP_TYPE_CISTID;
        index               = 0;
        temp_om_ptr         = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
        while(XSTP_OM_GetNextInstanceInfoPtr(&mstid, &temp_om_ptr))
        {
            if (temp_om_ptr->instance_exist && !temp_om_ptr->dirty_bit)
            {
                temp_pom_ptr    = &(temp_om_ptr->port_info[lport-1]);
                if (temp_pom_ptr->is_member)
                {
                    /* 14.6.1 */
                    msti_config_msg = &(bpdu->msti_configureation_message[index]);

                    /* Flags */
                    msti_config_msg->msti_flags = 0;
                    /* Bit 1 of Octet 1 */
                    if (temp_pom_ptr->tc_while != 0)
                    {
                        msti_config_msg->msti_flags |= XSTP_TYPE_BPDU_FLAGS_TC;
                    }
                    /* Bit 2 of Octet 1 */
                    if (temp_pom_ptr->proposing)
                    {
                        msti_config_msg->msti_flags |= XSTP_TYPE_BPDU_FLAGS_PROPOSAL;
                    }
                    /* Bits 3, 4 of Octet 1 */
                    switch (temp_pom_ptr->role)
                    {
                        case XSTP_ENGINE_PORTVAR_ROLE_ROOT:
                            msti_config_msg->msti_flags |= XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ROOT;
                            break;
                        case XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED:
                            msti_config_msg->msti_flags |= XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED;
                            break;
                        case XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE:
                        case XSTP_ENGINE_PORTVAR_ROLE_BACKUP:
                            msti_config_msg->msti_flags |= XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ALT_BAK;
                            break;
                    }
                    /* Bit 5 of Octet 1 */
                    if (temp_pom_ptr->learning)
                    {
                        msti_config_msg->msti_flags |= XSTP_TYPE_BPDU_FLAGS_LEARNING;
                    }
                    /* Bit 6 of Octet 1 */
                    if (temp_pom_ptr->forwarding)
                    {
                        msti_config_msg->msti_flags |= XSTP_TYPE_BPDU_FLAGS_FORWARDING;
                    }
                    /* Bit 7 of Octet 1 */
                    /* change agreed to agree */
                    if (temp_pom_ptr->agree)
                    {
                        msti_config_msg->msti_flags |= XSTP_TYPE_BPDU_FLAGS_AGREEMENT;
                    }
                    /* Bit 8 of Octet 1 */
                    if (temp_pom_ptr->msti_master)
                    {
                        msti_config_msg->msti_flags |= XSTP_TYPE_BPDU_FLAGS_MASTER;
                    }

                    /* 14.6.1(b) */
                    /* Octets 2 ~ 9 */
                    /* MSTI Regional Root ID */
                    XSTP_OM_CPY_HTON_BRIDGE_ID(msti_config_msg->msti_regional_root_identifier, temp_pom_ptr->designated_priority.r_root_id);
                    /* 14.6.1(c) */
                    /* Octets 10 ~ 13 */
                    /* MSTI Internal Root Path Cost */
                    msti_config_msg->msti_internal_root_path_cost
                        = L_STDLIB_Hton32(temp_pom_ptr->designated_priority.int_root_path_cost);

                    /* 14.6.1(d) */
                    /* Bits 5 ~ 8 of Octet 14 */
                    /* MSTI Bridge Identifier Priority */
                    XSTP_OM_GET_BRIDGE_ID_PRIORITY(temp_priority, temp_om_ptr->bridge_info.bridge_identifier);
                    msti_config_msg->msti_bridge_priority = 0xF0 & ((UI8_T)(temp_priority >> 8));
                    /* 14.6.1(e) */
                    /* Bits 5 ~ 8 of Octet 15 */
                    /* MSTI Port Identifier Priority */
                    XSTP_OM_GET_PORT_ID_PRIORITY(msti_config_msg->msti_port_priority, temp_pom_ptr->port_id);
                    msti_config_msg->msti_port_priority &= 0xF0;

                    /* 14.6.1(f) */
                    /* No patch: Octet 16
                       Patch: Octet 24
                    */
                    /* MSTI Remaining Hops */
                    msti_config_msg->msti_remaining_hops
                        = temp_pom_ptr->designated_times.remaining_hops;

                    index++;
                }
            }
        } /* End of for */

        XSTP_UTY_SendMstpBpdu(lport, bpdu);
    }
    return;
}/* End of XSTP_UTY_TxMstp */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtRcvdInfoWhileXst
 * ------------------------------------------------------------------------
 * PURPOSE  : 1. updtRcvdInfoWhileCist()
 *            2. updtRcvdInfoWhileMsti()
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.23, IEEE Std 802.1s(D14.1)-2002
 *            Ref to the description in 13.26.24, IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_UpdtRcvdInfoWhileXst(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    if (XSTP_UTY_Cist(om_ptr))
        XSTP_UTY_UpdtRcvdInfoWhileCist(om_ptr, lport);
    else
        XSTP_UTY_UpdtRcvdInfoWhileMsti(om_ptr, lport);

    return;
} /* End of XSTP_UTY_UpdtRcvdInfoWhileXst */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtRolesXst
 * ------------------------------------------------------------------------
 * PURPOSE  : 1. updtRolesCist()
 *            2. updtRolesMsti()
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.25, IEEE Std 802.1s(D14.1)-2002
 *            Ref to the description in 13.26.26, IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_UpdtRolesXst(XSTP_OM_InstanceData_T *om_ptr)
{
    if (XSTP_UTY_Cist(om_ptr))
        XSTP_UTY_UpdtRolesCist(om_ptr);
    else
        XSTP_UTY_UpdtRolesMsti(om_ptr);

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtRoleDisabledTree
 * ------------------------------------------------------------------------
 * PURPOSE  : Set selected_role to DisabledPort for all ports of the bridge
 *            for a given tree
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.27, IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_UpdtRoleDisabledTree(XSTP_OM_InstanceData_T *om_ptr)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI32_T              lport;

    /* Performance Improvement
    for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
    */
    lport = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr     = &(om_ptr->port_info[lport-1]);
        if (pom_ptr->is_member)
        {
            pom_ptr->selected_role = XSTP_ENGINE_PORTVAR_ROLE_DISABLED;
        }
    }

    return;
} /* End of XSTP_UTY_UpdtRoleDisabledTree */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_GetHelloTime
 * ------------------------------------------------------------------------
 * PURPOSE  : Get hello_time value of CIST port_times.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : hello time.
 * NOTE     : According to the definition in 17.16.3 for RSTP, the HelloTime
 *            is the Bridge Hello Time component of Bridge Times.
 *            In the implementation we use the Hello Time component of
 *            CIST Port Times in order to have the consistent behavior with
 *            802.1d (STP).
 *            In 802.1d (STP), the Hello Time is dominated by the Bridge
 *            Hello Time of the Root Bridge via received BPDU from the root.
 *-------------------------------------------------------------------------
 */
UI16_T  XSTP_UTY_GetHelloTime(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;

    pom_ptr         = &(om_ptr->port_info[lport-1]);
    if (XSTP_UTY_Cist(om_ptr))                          /* CIST */
    {
        return (pom_ptr->port_times.hello_time);
    }
    return (pom_ptr->cist->port_times.hello_time);      /* MSTI */
}/* End of XSTP_UTY_GetHelloTime */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_GetFwdDelay
 * ------------------------------------------------------------------------
 * PURPOSE  : Get forward_delay value of CIST port_times.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : forward_delay.
 * NOTE     : According to the definition in 17.16.2 for RSTP, the FwdDelay
 *            is the Bridge Forward Delay component of Bridge Times.
 *            In the implementation we use the Forward Delay component of
 *            CIST Port Times in order to have the consistent behavior with
 *            802.1d (STP).
 *            In 802.1d (STP), the Forward Delay is dominated by the Bridge
 *            Forward Delay of the Root Bridge via received BPDU from the root.
 *-------------------------------------------------------------------------
 */
UI16_T  XSTP_UTY_GetFwdDelay(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;

    pom_ptr         = &(om_ptr->port_info[lport-1]);
    if (XSTP_UTY_Cist(om_ptr))                          /* CIST */
    {
        return (pom_ptr->port_times.forward_delay);
    }
    return (pom_ptr->cist->port_times.forward_delay);   /* MSTI */
}/* End of XSTP_UTY_GetFwdDelay */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_DecreaseTimer
 * ------------------------------------------------------------------------
 * PURPOSE  : Decrease the specified timer
 * INPUT    : timer     -- value of timer
 * OUTPUT   : None
 * RETURN   : TRUE if the timer expires, else FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_DecreaseTimer(UI16_T *timer)
{
    if ( (*timer) != 0)
    {
        (*timer)--;
        if ( (*timer) == 0)
            return TRUE;
    }

    return FALSE;
} /* End of XSTP_UTY_DecreaseTimer */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_DecreaseTimer32
 * ------------------------------------------------------------------------
 * PURPOSE  : Decrease the specified timer
 * INPUT    : timer     -- value of timer
 * OUTPUT   : None
 * RETURN   : TRUE if the timer expires, else FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_DecreaseTimer32(UI32_T *timer)
{
    if ( (*timer) != 0)
    {
        (*timer)--;
        if ( (*timer) == 0)
            return TRUE;
    }

    return FALSE;
} /* End of XSTP_UTY_DecreaseTimer32 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_ReselectForAnyPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Check if the reselect variable of any port is set
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : TRUE if the reselect variable of any port is TRUE, else FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_ReselectForAnyPort(XSTP_OM_InstanceData_T *om_ptr)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI32_T              lport;
    BOOL_T              reselect;

    reselect = FALSE;
    /* Performance Improvement
    for (index = 0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
    */
    lport = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr     = &(om_ptr->port_info[lport-1]);
        if (pom_ptr->is_member)
        {
            reselect = reselect || pom_ptr->reselect;
        }
    }

    return ( (om_ptr->bridge_info.cist_role_updated) || reselect);
} /* End of XSTP_UTY_ReselectForAnyPort */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_MstiTcWhileIsZero
 * ------------------------------------------------------------------------
 * PURPOSE  : Return TRUE if the tc_while is equal to zero for any MSTI for
 *            the given Port. Return FALSE otherwise.
 * INPUT    : lport         -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_MstiTcWhileIsZero(UI32_T lport)
{
    UI32_T                      xstid;
    XSTP_OM_InstanceData_T      *om_ptr;
    XSTP_OM_PortVar_T           *pom_ptr;

    xstid               = XSTP_TYPE_CISTID;
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &om_ptr))   /* exclude CIST */
    {
        if (om_ptr->instance_exist)
        {
            pom_ptr         = &(om_ptr->port_info[lport-1]);
            if (    pom_ptr->is_member
                && (pom_ptr->tc_while!=0)
               )
            {
                return FALSE;
            }
        }
    }
    return TRUE;

} /* End of XSTP_UTY_MstiTcWhileIsZero */

#if 0
/* Old implementation: patch condition when port_role is root port */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RcvInfoCist
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. Ref to the description in 13.26.7, IEEE Std 802.1s-2002
 *               Ref to the description in 17.21.8, IEEE Std 802.1D/D3-2003
 *            2. Patch condition when port_role is root port
 *               -- External: Only compare root_id and ext_root_path_cost componments.
 *               -- Internal: Compare first four componments.
 *-------------------------------------------------------------------------
 */
static UI16_T  XSTP_UTY_RcvInfoCist(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T           *pom_ptr;
    XSTP_TYPE_BpduHeader_T      *bpdu_header;
    XSTP_TYPE_MstBpdu_T         *bpdu;
    UI8_T                       bpdu_type;
    UI8_T                       port_role;
    int                         cmp_a, cmp_b, cmp_c, cmp_d, cmp_e, cmp_f;
    UI16_T                      result;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    bpdu_header = (XSTP_TYPE_BpduHeader_T*)pom_ptr->common->bpdu;
    bpdu_type   = bpdu_header->bpdu_type;

    if (!pom_ptr->is_member)
    {
        return XSTP_ENGINE_PORTVAR_RCVD_INFO_OTHER_INFO;
    }

    switch (bpdu_type)
    {
        case XSTP_TYPE_BPDU_TYPE_XST:
            bpdu        = (XSTP_TYPE_MstBpdu_T*)pom_ptr->common->bpdu;
            port_role   = bpdu->cist_flags & XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_MASK;
            break;

        case XSTP_TYPE_BPDU_TYPE_CONFIG:
        case XSTP_TYPE_BPDU_TYPE_TCN:
            port_role   = XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_UNKNOWN;
            break;
    }

    XSTP_OM_CMP_PRIORITY_VECTOR(cmp_a, pom_ptr->msg_priority, pom_ptr->port_priority);
    XSTP_OM_CMP_TIMERS(cmp_b, pom_ptr->msg_times, pom_ptr->port_times);
    XSTP_OM_CMP_BRIDGE_ID(cmp_c, pom_ptr->msg_priority.designated_bridge_id, pom_ptr->port_priority.designated_bridge_id);
    XSTP_OM_CMP_PORT_ID(cmp_d, pom_ptr->msg_priority.designated_port_id, pom_ptr->port_priority.designated_port_id);
    XSTP_OM_CMP_BRIDGE_ID(cmp_e, pom_ptr->msg_priority.root_id, pom_ptr->port_priority.root_id);
    XSTP_OM_CMP_BRIDGE_ID(cmp_f, pom_ptr->msg_priority.r_root_id, pom_ptr->port_priority.r_root_id);

    if (    (   (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ROOT)
             || (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ALT_BAK)
            )
        &&  (   (   (!pom_ptr->common->rcvd_internal)
                 && (   (cmp_e > 0)
                     || (   (cmp_e == 0)
                         && ((pom_ptr->msg_priority.ext_root_path_cost) >= (pom_ptr->port_priority.ext_root_path_cost))
                        )
                    )
                )
             ||
                (   (pom_ptr->common->rcvd_internal)
                 && (   (cmp_e > 0)
                     || (   (cmp_e == 0)
                         && ((pom_ptr->msg_priority.ext_root_path_cost) > (pom_ptr->port_priority.ext_root_path_cost))
                        )
                     || (   (cmp_e == 0)
                         && ((pom_ptr->msg_priority.ext_root_path_cost) == (pom_ptr->port_priority.ext_root_path_cost))
                         && (cmp_f > 0)
                        )
                     || (   (cmp_e == 0)
                         && ((pom_ptr->msg_priority.ext_root_path_cost) == (pom_ptr->port_priority.ext_root_path_cost))
                         && (cmp_f == 0)
                         && ((pom_ptr->msg_priority.int_root_path_cost) >= (pom_ptr->port_priority.int_root_path_cost))
                        )
                    )
                )
            )
       )
    {
        result = XSTP_ENGINE_PORTVAR_RCVD_INFO_ROOT_INFO;
    }
    else
    if (    (   (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED)
             || (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_UNKNOWN)
            )
        &&  (   (   (cmp_a < 0)
/* Follow IEEE P802.1D/D3, page 159, 17.21.8 rcvinfo().
                 || (   (cmp_c == 0)
                     && (cmp_d == 0)
                     && (   (cmp_a != 0)
                         || (cmp_b != 0)
                        )
                    )
*/
                )
             || (   (cmp_a == 0)
                 && (cmp_b != 0)
                )
            )
       )
    {
        result = XSTP_ENGINE_PORTVAR_RCVD_INFO_SUPERIOR_DESIGNATED_INFO;
    }
    else
    if (    (cmp_a == 0)
        &&  (cmp_b == 0)
       )
    {
        result = XSTP_ENGINE_PORTVAR_RCVD_INFO_REPEATED_DESIGNATED_INFO;
    }
    else
    {
        /* Follow IEEE P802.1D/D3, page 159, 17.21.8 rcvinfo().*/
        if (    (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED)
             && (cmp_a > 0)
           )
        {
            XSTP_UTY_RecordDispute(om_ptr, lport);
        }
        result  = XSTP_ENGINE_PORTVAR_RCVD_INFO_OTHER_INFO;
    }

    return result;
} /* End of XSTP_UTY_RcvInfoCist */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RcvInfoMsti
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. Ref to the description in 13.26.8, IEEE Std 802.1s-2002
 *               Ref to the description in 17.21.8, IEEE Std 802.1D/D3-2003
 *            2. Patch condition when port_role is root port
 *               -- Only compare r_root_id and int_root_path_cost componments.
 *-------------------------------------------------------------------------
 */
static UI16_T  XSTP_UTY_RcvInfoMsti(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T           *pom_ptr;
    XSTP_TYPE_BpduHeader_T      *bpdu_header;
    XSTP_TYPE_MstiConfigMsg_T   *msti_config_msg;
    UI8_T                       bpdu_type;
    UI8_T                       port_role;
    int                         cmp_a, cmp_b, cmp_c, cmp_d, cmp_e;
    UI16_T                      result;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    bpdu_header = (XSTP_TYPE_BpduHeader_T*)pom_ptr->common->bpdu;
    bpdu_type   = bpdu_header->bpdu_type;

    if (!pom_ptr->is_member)
    {
        return XSTP_ENGINE_PORTVAR_RCVD_INFO_OTHER_INFO;
    }

    switch (bpdu_type)
    {
        case XSTP_TYPE_BPDU_TYPE_XST:
            msti_config_msg = pom_ptr->msti_config_msg;
            port_role   = msti_config_msg->msti_flags & XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_MASK;
            break;

        case XSTP_TYPE_BPDU_TYPE_CONFIG:
        case XSTP_TYPE_BPDU_TYPE_TCN:
            port_role   = XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_UNKNOWN;
            break;
    }

    XSTP_OM_CMP_PRIORITY_VECTOR(cmp_a, pom_ptr->msg_priority, pom_ptr->port_priority);
    XSTP_OM_CMP_TIMERS(cmp_b, pom_ptr->msg_times, pom_ptr->port_times);
    XSTP_OM_CMP_BRIDGE_ID(cmp_c, pom_ptr->msg_priority.designated_bridge_id, pom_ptr->port_priority.designated_bridge_id);
    XSTP_OM_CMP_PORT_ID(cmp_d, pom_ptr->msg_priority.designated_port_id, pom_ptr->port_priority.designated_port_id);
    XSTP_OM_CMP_BRIDGE_ID(cmp_e, pom_ptr->msg_priority.r_root_id, pom_ptr->port_priority.r_root_id);

    if (    (   (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ROOT)
             || (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ALT_BAK)
            )
        &&  (   (cmp_e > 0)
             || (   (cmp_e == 0)
                 && ((pom_ptr->msg_priority.int_root_path_cost) >= (pom_ptr->port_priority.int_root_path_cost))
                )
            )
       )
    {
        result = XSTP_ENGINE_PORTVAR_RCVD_INFO_ROOT_INFO;
    }
    else
    if (    (   (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED)
             || (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_UNKNOWN)
            )
        &&  (   (   (cmp_a < 0)
/* Follow IEEE P802.1D/D3, page 159, 17.21.8 rcvinfo().
                 || (   (cmp_c == 0)
                     && (cmp_d == 0)
                     && (   (cmp_a != 0)
                         || (cmp_b != 0)
                        )
                    )
*/
                )
             || (   (cmp_a == 0)
                 && (cmp_b != 0)
                )
            )
       )
    {
        result = XSTP_ENGINE_PORTVAR_RCVD_INFO_SUPERIOR_DESIGNATED_INFO;
    }
    else
    if (    (cmp_a == 0)
        &&  (cmp_b == 0)
       )
    {
        result = XSTP_ENGINE_PORTVAR_RCVD_INFO_REPEATED_DESIGNATED_INFO;
    }
    else
    {
        /* Follow IEEE P802.1D/D3, page 159, 17.21.8 rcvinfo().*/
        if (    (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED)
             && (cmp_a > 0)
           )
        {
            XSTP_UTY_RecordDispute(om_ptr, lport);
        }
        result  = XSTP_ENGINE_PORTVAR_RCVD_INFO_OTHER_INFO;
    }

    return result;
} /* End of XSTP_UTY_RcvInfoMsti */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RecordAgreementCist
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. Ref to the description in 13.26.9, IEEE Std 802.1s-2002
 *            2. Patch condition when port_role is root port
 *               -- External: Only compare root_id and ext_root_path_cost componments.
 *               -- Internal: Compare first four componments.
 *-------------------------------------------------------------------------
 */
static void  XSTP_UTY_RecordAgreementCist(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_InstanceData_T  *xstid_om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_MstBpdu_T     *bpdu;
    BOOL_T                  cist_agreed_flag;
    UI32_T                  xstid;
    int                     cmp_result, cmp_a, cmp_b;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    bpdu        = (XSTP_TYPE_MstBpdu_T*)pom_ptr->common->bpdu;

    XSTP_OM_CMP_PRIORITY_VECTOR(cmp_result, pom_ptr->msg_priority, pom_ptr->port_priority);
    XSTP_OM_CMP_BRIDGE_ID(cmp_a, pom_ptr->msg_priority.root_id, pom_ptr->port_priority.root_id);
    XSTP_OM_CMP_BRIDGE_ID(cmp_b, pom_ptr->msg_priority.r_root_id, pom_ptr->port_priority.r_root_id);

    if (    (bpdu->bpdu_type == XSTP_TYPE_BPDU_TYPE_XST)
         && ( (bpdu->cist_flags & XSTP_TYPE_BPDU_FLAGS_AGREEMENT) != 0)
         && (pom_ptr->common->oper_point_to_point_mac)
         && (   (   (   (bpdu->cist_flags & XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_MASK)
                     == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ROOT
                    )
                 && (   (   (!pom_ptr->common->rcvd_internal)
                         && (   (cmp_a > 0)
                             || (   (cmp_a == 0)
                                 && ((pom_ptr->msg_priority.ext_root_path_cost) >= (pom_ptr->port_priority.ext_root_path_cost))
                                )
                            )
                        )
                     ||
                        (   (pom_ptr->common->rcvd_internal)
                         && (   (cmp_a > 0)
                             || (   (cmp_a == 0)
                                 && ((pom_ptr->msg_priority.ext_root_path_cost) > (pom_ptr->port_priority.ext_root_path_cost))
                                )
                             || (   (cmp_a == 0)
                                 && ((pom_ptr->msg_priority.ext_root_path_cost) == (pom_ptr->port_priority.ext_root_path_cost))
                                 && (cmp_b > 0)
                                )
                             || (   (cmp_a == 0)
                                 && ((pom_ptr->msg_priority.ext_root_path_cost) == (pom_ptr->port_priority.ext_root_path_cost))
                                 && (cmp_b == 0)
                                 && ((pom_ptr->msg_priority.int_root_path_cost) >= (pom_ptr->port_priority.int_root_path_cost))
                                )
                            )
                        )
                    )
                )
             || (   (   (bpdu->cist_flags & XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_MASK)
                     == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED
                    )
                 && (cmp_result <= 0)
                )
            )
       )
    {
        pom_ptr->agreed = TRUE;
    }
    else
    {
        pom_ptr->agreed = FALSE;
    }

    if (!pom_ptr->common->rcvd_internal)
    {
        cist_agreed_flag = pom_ptr->agreed;

        xstid               = XSTP_TYPE_CISTID;
        xstid_om_ptr        = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
        while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &xstid_om_ptr))
        {
            if (xstid_om_ptr->instance_exist)
            {
                pom_ptr         = &(xstid_om_ptr->port_info[lport-1]);
                if (pom_ptr->is_member)
                    pom_ptr->agreed = cist_agreed_flag;
            }
        }
    }
    return;

} /* End of XSTP_UTY_RecordAgreementCist */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RecordAgreementMsti
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. Ref to the description in 13.26.10, IEEE Std 802.1s-2002
 *            2. Patch condition when port_role is root port
 *               -- Only compare r_root_id and int_root_path_cost componments.
 *-------------------------------------------------------------------------
 */
static void  XSTP_UTY_RecordAgreementMsti(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T           *pom_ptr;
    XSTP_TYPE_MstBpdu_T         *bpdu;
    int                         cmp_cist_root_id, cmp_cist_r_root_id, cmp_result, cmp_a;
    UI32_T                      mstid;
    XSTP_TYPE_MstiConfigMsg_T   *msti_message;

    pom_ptr         = &(om_ptr->port_info[lport-1]);
    bpdu            = (XSTP_TYPE_MstBpdu_T*)pom_ptr->common->bpdu;
    mstid           = (UI32_T)om_ptr->bridge_info.bridge_identifier.bridge_id_priority.data.system_id_ext;
    msti_message    = pom_ptr->msti_config_msg;

    XSTP_OM_CMP_BRIDGE_ID(cmp_cist_root_id, bpdu->cist_root_identifier, pom_ptr->cist->port_priority.root_id);
    XSTP_OM_CMP_BRIDGE_ID(cmp_cist_r_root_id, bpdu->cist_regional_root_identifier, pom_ptr->cist->port_priority.r_root_id);
    XSTP_OM_CMP_PRIORITY_VECTOR(cmp_result, pom_ptr->msg_priority, pom_ptr->port_priority);
    XSTP_OM_CMP_BRIDGE_ID(cmp_a, pom_ptr->msg_priority.r_root_id, pom_ptr->port_priority.r_root_id);

    if (    (bpdu->bpdu_type == XSTP_TYPE_BPDU_TYPE_XST)
         && (pom_ptr->common->oper_point_to_point_mac)
         && (   ( cmp_cist_root_id == 0)
             && ( bpdu->cist_external_path_cost == pom_ptr->cist->port_priority.ext_root_path_cost)
             && ( cmp_cist_r_root_id == 0)
            )
         && (   ((msti_message->msti_flags & XSTP_TYPE_BPDU_FLAGS_AGREEMENT) != 0)
             && (   (   (   (msti_message->msti_flags & XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_MASK)
                         == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ROOT
                        )
                     &&  (   (cmp_a > 0)
                          || (   (cmp_a == 0)
                              && ((pom_ptr->msg_priority.int_root_path_cost) >= (pom_ptr->port_priority.int_root_path_cost))
                             )
                         )
                    )
                 || (   (   (msti_message->msti_flags & XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_MASK)
                         == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED
                        )
                     && (cmp_result <= 0)
                    )
                )
            )
        )
    {
        pom_ptr->agreed = TRUE;
    }
    else
    {
        pom_ptr->agreed = FALSE;
    }
    return;
} /* End of XSTP_UTY_RecordAgreementMsti */
#endif

/* New implementation according to IEEE Std 802.1D/D3 June 11, 2003, page 159.
   -- XSTP_UTY_RcvInfoCist()
   -- XSTP_UTY_RcvInfoMsti()
   -- XSTP_UTY_RecordAgreementCist()
   -- XSTP_UTY_RecordAgreementMsti()
*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RcvInfoCist
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. Ref to the description in 13.26.7, IEEE Std 802.1s-2002
 *            2. Ref to the description in 17.21.8, IEEE Std 802.1D/D3-2003
 *-------------------------------------------------------------------------
 */
static UI16_T  XSTP_UTY_RcvInfoCist(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T           *pom_ptr;
    XSTP_TYPE_BpduHeader_T      *bpdu_header;
    XSTP_TYPE_MstBpdu_T         *bpdu;
    UI8_T                       bpdu_type;
    UI8_T                       port_role;
    int                         cmp_a, cmp_b, cmp_c, cmp_d;
    UI16_T                      result;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    bpdu_header = (XSTP_TYPE_BpduHeader_T*)pom_ptr->common->bpdu;
    if (bpdu_header == NULL)
    {
        return XSTP_ENGINE_PORTVAR_RCVD_INFO_OTHER_INFO;
    }
    bpdu_type   = bpdu_header->bpdu_type;

    if (!pom_ptr->is_member)
    {
        return XSTP_ENGINE_PORTVAR_RCVD_INFO_OTHER_INFO;
    }

    switch (bpdu_type)
    {
        case XSTP_TYPE_BPDU_TYPE_XST:
            bpdu        = (XSTP_TYPE_MstBpdu_T*)pom_ptr->common->bpdu;
            port_role   = bpdu->cist_flags & XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_MASK;

            if (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_UNKNOWN) /* 802.1D-2004, 9.2.9 NOTE and 802.1Q-2005, 13.26.6, p.174, NOTE */
                port_role = XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED;
            break;

        case XSTP_TYPE_BPDU_TYPE_CONFIG:
            port_role   = XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED;  /* 802.1Q-2005, 13.26.6, p.174, NOTE */
            break;

        case XSTP_TYPE_BPDU_TYPE_TCN:
            port_role   = XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_UNKNOWN;
            break;

        default:
            return XSTP_ENGINE_PORTVAR_RCVD_INFO_OTHER_INFO;
    }

    XSTP_OM_CMP_PRIORITY_VECTOR(cmp_a, pom_ptr->msg_priority, pom_ptr->port_priority);
    XSTP_OM_CMP_TIMERS(cmp_b, pom_ptr->msg_times, pom_ptr->port_times);
    XSTP_OM_CMP_BRIDGE_ID(cmp_c, pom_ptr->msg_priority.designated_bridge_id, pom_ptr->port_priority.designated_bridge_id);
    cmp_d = memcmp( &pom_ptr->msg_priority.designated_bridge_id.addr,
                    &pom_ptr->port_priority.designated_bridge_id.addr,
                    6
                  );

    if (    (   (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ROOT)
             || (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ALT_BAK)
            )
        &&  ( cmp_a >= 0)
       )
    {
        result = XSTP_ENGINE_PORTVAR_RCVD_INFO_ROOT_INFO;
    }
    else
    if (    (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED)
        &&  (   (cmp_a < 0)
             || (   (cmp_d == 0)            /* 13.10 */
                 && (pom_ptr->msg_priority.designated_port_id.data.port_num == pom_ptr->port_priority.designated_port_id.data.port_num)
                 && (   (cmp_a != 0)
                     || (cmp_b != 0)
                    )
                )
             || (   (cmp_a == 0)
                 && (cmp_b != 0)
                )
            )
       )
    {
        result = XSTP_ENGINE_PORTVAR_RCVD_INFO_SUPERIOR_DESIGNATED_INFO;
    }
    else
    if (    (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED) /* 802.1Q-2005, 13.26.6, p.174, b) */
         && (    (cmp_a == 0)
              && (cmp_b == 0)
            )
         && (pom_ptr->info_is == XSTP_ENGINE_PORTVAR_INFO_IS_RECEIVED)
       )
    {
        result = XSTP_ENGINE_PORTVAR_RCVD_INFO_REPEATED_DESIGNATED_INFO;
    }
    else
    {
        /* Follow IEEE P802.1D/D3, page 159, 17.21.8 rcvinfo().*/
        if (    (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED)
             && (cmp_a > 0)
           )
        {
            XSTP_UTY_RecordDispute(om_ptr, lport);
        }
        result  = XSTP_ENGINE_PORTVAR_RCVD_INFO_OTHER_INFO;
    }

    return result;
} /* End of XSTP_UTY_RcvInfoCist */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RcvInfoMsti
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. Ref to the description in 13.26.8, IEEE Std 802.1s-2002
 *            2. Ref to the description in 17.21.8, IEEE Std 802.1D/D3-2003
 *-------------------------------------------------------------------------
 */
static UI16_T  XSTP_UTY_RcvInfoMsti(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T           *pom_ptr;
    XSTP_TYPE_BpduHeader_T      *bpdu_header;
    XSTP_TYPE_MstiConfigMsg_T   *msti_config_msg;
    UI8_T                       bpdu_type;
    UI8_T                       port_role;
    int                         cmp_a, cmp_b, cmp_c, cmp_d;
    UI16_T                      result;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    bpdu_header = (XSTP_TYPE_BpduHeader_T*)pom_ptr->common->bpdu;
    if (bpdu_header == NULL)
    {
        return XSTP_ENGINE_PORTVAR_RCVD_INFO_OTHER_INFO;
    }
    bpdu_type   = bpdu_header->bpdu_type;

    if (!pom_ptr->is_member)
    {
        return XSTP_ENGINE_PORTVAR_RCVD_INFO_OTHER_INFO;
    }

    switch (bpdu_type)
    {
        case XSTP_TYPE_BPDU_TYPE_XST:
            msti_config_msg = pom_ptr->msti_config_msg;
            port_role   = msti_config_msg->msti_flags & XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_MASK;
            break;

        case XSTP_TYPE_BPDU_TYPE_CONFIG:
            port_role   = XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED;  /* 802.1Q-2005, 13.26.6, p.174, NOTE */
            break;

        case XSTP_TYPE_BPDU_TYPE_TCN:
            port_role   = XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_UNKNOWN;
            break;

        default:
            return XSTP_ENGINE_PORTVAR_RCVD_INFO_OTHER_INFO;
    }

    XSTP_OM_CMP_PRIORITY_VECTOR(cmp_a, pom_ptr->msg_priority, pom_ptr->port_priority);
    XSTP_OM_CMP_TIMERS(cmp_b, pom_ptr->msg_times, pom_ptr->port_times);
    XSTP_OM_CMP_BRIDGE_ID(cmp_c, pom_ptr->msg_priority.designated_bridge_id, pom_ptr->port_priority.designated_bridge_id);
    cmp_d = memcmp( &pom_ptr->msg_priority.designated_bridge_id.addr,
                    &pom_ptr->port_priority.designated_bridge_id.addr,
                    6
                  );

    if (    (   (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ROOT)
             || (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ALT_BAK)
            )
        &&  ( cmp_a >= 0)
       )
    {
        result = XSTP_ENGINE_PORTVAR_RCVD_INFO_ROOT_INFO;
    }
    else
    if (    (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED)
        &&  (   (cmp_a < 0)
             || (   (cmp_d == 0)            /* 13.11 */
                 && (pom_ptr->msg_priority.designated_port_id.data.port_num == pom_ptr->port_priority.designated_port_id.data.port_num)
                 && (   (cmp_a != 0)
                     || (cmp_b != 0)
                    )
                )
             || (   (cmp_a == 0)
                 && (cmp_b != 0)
                )
            )
       )
    {
        result = XSTP_ENGINE_PORTVAR_RCVD_INFO_SUPERIOR_DESIGNATED_INFO;
    }
    else
    if (    (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED) /* 802.1Q-2005, 13.26.6, p.174, b) */
         && (    (cmp_a == 0)
              && (cmp_b == 0)
            )
         && (pom_ptr->info_is == XSTP_ENGINE_PORTVAR_INFO_IS_RECEIVED)
       )
    {
        result = XSTP_ENGINE_PORTVAR_RCVD_INFO_REPEATED_DESIGNATED_INFO;
    }
    else
    {
        /* Follow IEEE P802.1D/D3, page 159, 17.21.8 rcvinfo().*/
        if (    (port_role == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED)
             && (cmp_a > 0)
           )
        {
            XSTP_UTY_RecordDispute(om_ptr, lport);
        }
        result  = XSTP_ENGINE_PORTVAR_RCVD_INFO_OTHER_INFO;
    }

    return result;
} /* End of XSTP_UTY_RcvInfoMsti */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RecordAgreementCist
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.9, IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
static void  XSTP_UTY_RecordAgreementCist(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_InstanceData_T  *xstid_om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_MstBpdu_T     *bpdu;
    BOOL_T                  cist_agreed_flag, cist_proposing_flag;
    UI32_T                  xstid;
    int                     cmp_result;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    bpdu        = (XSTP_TYPE_MstBpdu_T*)pom_ptr->common->bpdu;

    XSTP_OM_CMP_PRIORITY_VECTOR(cmp_result, pom_ptr->msg_priority, pom_ptr->port_priority);

    if (    (bpdu->bpdu_type == XSTP_TYPE_BPDU_TYPE_XST)
         && ( (bpdu->cist_flags & XSTP_TYPE_BPDU_FLAGS_AGREEMENT) != 0)
         && (pom_ptr->common->oper_point_to_point_mac)
/* 802.1Q-2005 13.26.7 p.174
         && (   (   (   (bpdu->cist_flags & XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_MASK)
                     == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ROOT
                    )
                 && (cmp_result >= 0)
                )
             || (   (   (bpdu->cist_flags & XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_MASK)
                     == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED
                    )
                 && (cmp_result <= 0)
                )
            )
*/
       )
    {
        pom_ptr->agreed     = TRUE;
        pom_ptr->proposing  = FALSE; /* 802.1Q-2005 13.26.7 p.174 */
    }
    else
    {
        pom_ptr->agreed = FALSE;
    }

    if (!pom_ptr->common->rcvd_internal)
    {
        cist_agreed_flag    = pom_ptr->agreed;
        cist_proposing_flag = pom_ptr->proposing; /* 802.1Q-2005 13.26.7 p.174 */

        xstid               = XSTP_TYPE_CISTID;
        xstid_om_ptr        = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
        while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &xstid_om_ptr))
        {
            if (xstid_om_ptr->instance_exist)
            {
                pom_ptr         = &(xstid_om_ptr->port_info[lport-1]);
                if (pom_ptr->is_member)
                {
                    pom_ptr->agreed     = cist_agreed_flag;
                    pom_ptr->proposing  = cist_proposing_flag; /* 802.1Q-2005 13.26.7 p.174 */
                }
            }
        }
    }
    return;

} /* End of XSTP_UTY_RecordAgreementCist */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RecordAgreementMsti
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.10, IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
static void  XSTP_UTY_RecordAgreementMsti(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T           *pom_ptr;
    XSTP_TYPE_MstBpdu_T         *bpdu;
    XSTP_TYPE_MstiConfigMsg_T   *msti_message;
    int                         cmp_cist_root_id, cmp_cist_r_root_id, cmp_result;

    pom_ptr         = &(om_ptr->port_info[lport-1]);
    bpdu            = (XSTP_TYPE_MstBpdu_T*)pom_ptr->common->bpdu;
    msti_message    = pom_ptr->msti_config_msg;

    XSTP_OM_CMP_BRIDGE_ID(cmp_cist_root_id, bpdu->cist_root_identifier, pom_ptr->cist->port_priority.root_id);
    XSTP_OM_CMP_BRIDGE_ID(cmp_cist_r_root_id, bpdu->cist_regional_root_identifier, pom_ptr->cist->port_priority.r_root_id);
    XSTP_OM_CMP_PRIORITY_VECTOR(cmp_result, pom_ptr->msg_priority, pom_ptr->port_priority);

    if (    (bpdu->bpdu_type == XSTP_TYPE_BPDU_TYPE_XST)
         && (pom_ptr->common->oper_point_to_point_mac)
         && (   ( cmp_cist_root_id == 0)
             && ( bpdu->cist_external_path_cost == pom_ptr->cist->port_priority.ext_root_path_cost)
             && ( cmp_cist_r_root_id == 0)
            )
         && (   ((msti_message->msti_flags & XSTP_TYPE_BPDU_FLAGS_AGREEMENT) != 0)
/* 802.1Q-2005 13.26.7 p.174
             && (   (   (   (msti_message->msti_flags & XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_MASK)
                         == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ROOT
                        )
                     && (cmp_result >= 0)
                    )
                 || (   (   (msti_message->msti_flags & XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_MASK)
                         == XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED
                        )
                     && (cmp_result <= 0)
                    )
                )
*/
            )
        )
    {
        pom_ptr->agreed     = TRUE;
        pom_ptr->proposing  = FALSE; /* 802.1Q-2005 13.26.7 p.174 */
    }
    else
    {
        pom_ptr->agreed = FALSE;
    }
    return;
} /* End of XSTP_UTY_RecordAgreementMsti */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RecordMasteredCist
 * ------------------------------------------------------------------------
 * PURPOSE  : If the rcvd_internal flag is clear, the msti_mastered variable
 *            for this Port is cleared for all MSTIs.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.11, IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
static void  XSTP_UTY_RecordMasteredCist(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    UI32_T                  xstid;
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_OM_InstanceData_T  *xstid_om_ptr;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    if (!pom_ptr->common->rcvd_internal)
    {
        xstid               = XSTP_TYPE_CISTID;
        xstid_om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
        while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &xstid_om_ptr))
        {
            pom_ptr                 = &(xstid_om_ptr->port_info[lport-1]);
            if (pom_ptr->is_member)
                pom_ptr->msti_mastered  = FALSE;
        }
    }
    return;
} /* End of XSTP_UTY_RecordMasteredCist */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RecordMasteredMsti
 * ------------------------------------------------------------------------
 * PURPOSE  : If the MSTI Message was received on a point to point link and
 *            the MSTI Message has the Master flag set, set the
 *            msti_mastered variable for the MSTI. Otherwise reset the
 *            msti_mastered variable.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.12, IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
static void  XSTP_UTY_RecordMasteredMsti(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T           *pom_ptr;
    XSTP_TYPE_MstBpdu_T         *bpdu;
    XSTP_TYPE_MstiConfigMsg_T   *msti_message;

    pom_ptr         = &(om_ptr->port_info[lport-1]);
    bpdu            = (XSTP_TYPE_MstBpdu_T*)pom_ptr->common->bpdu;
    msti_message    = pom_ptr->msti_config_msg;

    if (pom_ptr->is_member)
    {
        if (    (bpdu->bpdu_type == XSTP_TYPE_BPDU_TYPE_XST)
             && (pom_ptr->common->oper_point_to_point_mac)
             && ((msti_message->msti_flags & XSTP_TYPE_BPDU_FLAGS_MASTER) != 0)
            )
        {
            pom_ptr->msti_mastered = TRUE;
        }
        else
        {
            pom_ptr->msti_mastered = FALSE;
        }
    } /* End of if (pom_ptr->is_member) */

    return;
} /* End of XSTP_UTY_RecordMasteredMsti */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RecordProposalCist
 * ------------------------------------------------------------------------
 * PURPOSE  : If the CIST Message was a Configuration Message received on a
 *            point to point link and has the Proposal flag set, the CIST
 *            proposed flag is set. Otherwise the CIST proposed flag is
 *            cleared.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.13, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
static void  XSTP_UTY_RecordProposalCist(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    XSTP_TYPE_MstBpdu_T     *bpdu;
    XSTP_OM_InstanceData_T  *xstid_om_ptr;
    BOOL_T                  cist_proposed_flag;
    UI32_T                  xstid;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    bpdu        = (XSTP_TYPE_MstBpdu_T*)pom_ptr->common->bpdu;

    if (    (bpdu->bpdu_type == XSTP_TYPE_BPDU_TYPE_XST)
         && (pom_ptr->common->oper_point_to_point_mac)
         && ((bpdu->cist_flags & XSTP_TYPE_BPDU_FLAGS_PROPOSAL) != 0)
       )
    {
        pom_ptr->proposed = TRUE;
    }
    else
    {
        pom_ptr->proposed = FALSE;
    }

    if (!pom_ptr->common->rcvd_internal)
    {
        cist_proposed_flag = pom_ptr->proposed;

        xstid               = XSTP_TYPE_CISTID;
        xstid_om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
        while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &xstid_om_ptr))
        {
            if (xstid_om_ptr->instance_exist)
            {
                pom_ptr             = &(xstid_om_ptr->port_info[lport-1]);
                if (pom_ptr->is_member)
                    pom_ptr->proposed   = cist_proposed_flag;
            }
        }
    }
    return;
} /* End of XSTP_UTY_RecordProposalCist */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RecordProposalMsti
 * ------------------------------------------------------------------------
 * PURPOSE  : If the MSTI Message was received on a point to point link and
 *            has the Proposal flag set, the MSTI proposed flag is set.
 *            Otherwise the MSTI proposed flag is cleared.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.14, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
static void  XSTP_UTY_RecordProposalMsti(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T           *pom_ptr;
    XSTP_TYPE_MstBpdu_T         *bpdu;
    XSTP_TYPE_MstiConfigMsg_T   *msti_message;

    pom_ptr         = &(om_ptr->port_info[lport-1]);
    bpdu            = (XSTP_TYPE_MstBpdu_T*)pom_ptr->common->bpdu;
    msti_message    = pom_ptr->msti_config_msg;

    if (    (bpdu->bpdu_type == XSTP_TYPE_BPDU_TYPE_XST)
         && (pom_ptr->common->oper_point_to_point_mac)
         && ((msti_message->msti_flags & XSTP_TYPE_BPDU_FLAGS_PROPOSAL) != 0)
        )
    {
        pom_ptr->proposed = TRUE;
    }
    else
    {
        pom_ptr->proposed = FALSE;
    }
    return;
}/* End of XSTP_UTY_RecordProposalMsti */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtRcvdInfoWhileCist
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.23, IEEE Std 802.1s(D14.1)-2002
 *            Ref to the description in 17.21.23, IEEE Std 802.1D-2004
 *-------------------------------------------------------------------------
 */
static void    XSTP_UTY_UpdtRcvdInfoWhileCist(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI16_T              effective_age, increment;
    UI8_T               remaining_hops;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    increment   = 1;

    /* Follow IEEE Std 802.1D-2004, page 168, 17.21.23 updtRcvdInfoWhile().
       rcvdInfoWhile is the three times the Hello Time, if Message Age,
       incremented by 1 second and rounded to the nearest whole second,
       does not exceed Max Age, and is zero otherwise.
    */
    effective_age   = pom_ptr->port_times.message_age + increment;

    if (!pom_ptr->common->rcvd_internal)
    {
        if (effective_age <= pom_ptr->port_times.max_age)
        {
            pom_ptr->rcvd_info_while    = 3 * pom_ptr->port_times.hello_time;
        }
        else
        {
            pom_ptr->rcvd_info_while    = 0;
        }
    }
    else
    {
        if (pom_ptr->msg_times.remaining_hops)
        {
            remaining_hops  = pom_ptr->msg_times.remaining_hops - 1;
        }
        else
        {
            remaining_hops  = 0;
        }

        if (    (effective_age <= pom_ptr->port_times.max_age)
             && (remaining_hops > 0)
           )
        {
            pom_ptr->rcvd_info_while    = 3 * pom_ptr->port_times.hello_time;
        }
        else
        {
            pom_ptr->rcvd_info_while    = 0;
        }

    }
    return;
}/* End of XSTP_UTY_RecordProposalCist */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtRcvdInfoWhileMsti
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.23, IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
static void    XSTP_UTY_UpdtRcvdInfoWhileMsti(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    UI8_T               remaining_hops;

    pom_ptr     = &(om_ptr->port_info[lport-1]);

    if (pom_ptr->msg_times.remaining_hops)
    {
        remaining_hops  = pom_ptr->msg_times.remaining_hops - 1;
    }
    else
    {
        remaining_hops  = 0;
    }
    if (remaining_hops > 0)
    {
        pom_ptr->rcvd_info_while    = 3 * pom_ptr->cist->port_times.hello_time;
    }
    else
    {
        pom_ptr->rcvd_info_while    = 0;
    }
    return;
}/* End of XSTP_UTY_UpdtRcvdInfoWhileMsti */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtRolesCist
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.25, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
static void    XSTP_UTY_UpdtRolesCist(XSTP_OM_InstanceData_T *om_ptr)
{
    XSTP_TYPE_PriorityVector_T  root_priority_vector;
    XSTP_TYPE_PriorityVector_T  root_path_priority_vector;
    XSTP_TYPE_Timers_T          root_times;
    XSTP_TYPE_Timers_T          root_port_times;
    XSTP_OM_PortVar_T           *pom_ptr;
    UI32_T                      lport;
    UI32_T                      root_lport;
    BOOL_T                      is_root_bridge;
    I32_T                       cmp_result, cmp_a, cmp_b, cmp_c, cmp_d, cmp_e, cmp_f, cmp_g, cmp_h;
    UI32_T                      mstid;
    XSTP_OM_InstanceData_T      *temp_om_ptr;

    /* Check if this bridge is root before selection */
    /* For new_root trap */
    is_root_bridge = XSTP_UTY_RootBridge(om_ptr);


    /* Determine the root_priority_vector */
    /* root_priority_vector :=
     * <Best of> {bridge_priority_vector} U {root_path_priority_vector|for all ports}
     */
    /* 13.26.25 :: (a) ~ (d) */
    memcpy(&root_priority_vector, &om_ptr->bridge_info.bridge_priority, XSTP_TYPE_PRIORITY_VECTOR_LENGTH);
    memcpy(&root_times, &om_ptr->bridge_info.bridge_times, XSTP_TYPE_TIMERS_LENGTH);
    lport           = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr     = &(om_ptr->port_info[lport-1]);
        if (pom_ptr->is_member)
        {
            /* 13.26.25 (a) */
            if (pom_ptr->common->rcvd_internal)
            {
                XSTP_OM_PRIORITY_VECTOR_FORMAT( root_path_priority_vector,
                                                pom_ptr->port_priority.root_id,
                                                pom_ptr->port_priority.ext_root_path_cost,
                                                pom_ptr->port_priority.r_root_id,
                                                pom_ptr->port_priority.int_root_path_cost + pom_ptr->internal_port_path_cost,
                                                pom_ptr->port_priority.designated_bridge_id,
                                                pom_ptr->port_priority.designated_port_id,
                                                pom_ptr->port_id);
            }
            else
            {
                XSTP_OM_PRIORITY_VECTOR_FORMAT( root_path_priority_vector,
                                                pom_ptr->port_priority.root_id,
                                                pom_ptr->port_priority.ext_root_path_cost + pom_ptr->common->external_port_path_cost,
                                                om_ptr->bridge_info.bridge_identifier,
                                                0,
                                                pom_ptr->port_priority.designated_bridge_id,
                                                pom_ptr->port_priority.designated_port_id,
                                                pom_ptr->port_id);
            }

            /* 13.26.25 (b) */
            XSTP_OM_CMP_PRIORITY_VECTOR(cmp_a, root_priority_vector, root_path_priority_vector);
            if (    (pom_ptr->info_is != XSTP_ENGINE_PORTVAR_INFO_IS_DISABLED)
                 && (pom_ptr->rcvd_info_while > 0)
                 && (memcmp(&root_path_priority_vector.designated_bridge_id.addr,
                            &om_ptr->bridge_info.bridge_priority.designated_bridge_id.addr,
                            6
                           ) != 0
                    )
                 && (cmp_a > 0)
               )
            {
                XSTP_OM_CMP_PRIORITY_VECTOR(cmp_result, om_ptr->bridge_info.bridge_priority, root_path_priority_vector);
                if (cmp_result < 0)
                {
                    memcpy( &root_priority_vector,
                            &om_ptr->bridge_info.bridge_priority,
                            XSTP_TYPE_PRIORITY_VECTOR_LENGTH
                          );
                }
                else
                {
                    memcpy( &root_priority_vector,
                            &root_path_priority_vector,
                            XSTP_TYPE_PRIORITY_VECTOR_LENGTH
                          );
                }

                /* 13.26.25 (c) */
                XSTP_OM_CMP_PRIORITY_VECTOR(cmp_b, om_ptr->bridge_info.bridge_priority, root_priority_vector);
                if (cmp_b == 0)
                {
                    memcpy( &root_times,
                            &om_ptr->bridge_info.bridge_times,
                            XSTP_TYPE_TIMERS_LENGTH
                          );
                }
                else
                {
                    /* 13.23.7 cistRootTimes */
                    memcpy( &root_times,
                            &pom_ptr->port_times,
                            XSTP_TYPE_TIMERS_LENGTH
                          );
                    if (!pom_ptr->common->rcvd_internal)
                    {
                        UI16_T      increment;
                        /* Follow IEEE Std 802.1D-2004, page 168, 17.21.23 updtRcvdInfoWhile().
                           Follow IEEE Std 802.1Q-2005, page 177, 13.26.22 updtRcvdInfoWhile().
                           Message Age should be incremented by 1 second.
                         */
                        increment   = 1;
                        root_times.message_age      = root_times.message_age + increment;
                        root_times.remaining_hops   = om_ptr->bridge_info.bridge_times.remaining_hops;
                    }
                    else
                    {
                        root_times.remaining_hops  = root_times.remaining_hops - 1;
                    }
                }
            }
        } /* End of if (pom_ptr->is_member) */
    } /* End of while (lport is existent) */

    /* 13.24.3 changedMaster */
    XSTP_OM_CMP_BRIDGE_ID(cmp_c, root_priority_vector.r_root_id, om_ptr->bridge_info.root_priority.r_root_id);
    if(     (cmp_c != 0)
       &&   (   (root_priority_vector.ext_root_path_cost != 0)
             || (om_ptr->bridge_info.root_priority.ext_root_path_cost != 0)
            )
      )
    {
        XSTP_UTY_ChangedMaster();
    }

    XSTP_OM_CMP_BRIDGE_ID(cmp_a, root_priority_vector.root_id, om_ptr->bridge_info.root_priority.root_id);
    if (cmp_a != 0)
    {
        /* root bridge is changed */
        TRAP_EVENT_TrapData_T   trap_data;

        trap_data.trap_type = TRAP_EVENT_XSTP_ROOT_BRIDGE_CHANGED;
        XSTP_OM_GET_BRIDGE_ID_PRIORITY(
            trap_data.u.xstp_root_bridge_changed.priority,
            root_priority_vector.root_id);
        trap_data.u.xstp_root_bridge_changed.instance_id = om_ptr->instance_id;
        memcpy(trap_data.u.xstp_root_bridge_changed.bridge_address,
            &root_priority_vector.root_id.addr, 6);
        trap_data.community_specified = FALSE;
        trap_data.flag = TRAP_EVENT_SEND_TRAP_OPTION_DEFAULT;
        SNMP_PMGR_ReqSendTrap(&trap_data);
    }

    /* root_priority */
    memcpy( &om_ptr->bridge_info.root_priority,
            &root_priority_vector,
            XSTP_TYPE_PRIORITY_VECTOR_LENGTH
          );
    /* root_port_id */
    memcpy( &om_ptr->bridge_info.root_port_id,
            &root_priority_vector.rcv_port_id,
            XSTP_TYPE_PORT_ID_LENGTH
          );
    /* root_times */
    memcpy( &om_ptr->bridge_info.root_times,
            &root_times,
            XSTP_TYPE_TIMERS_LENGTH
          );

    if (!is_root_bridge && XSTP_UTY_RootBridge(om_ptr))
    {
        /* This bridge is the new_root */
        XSTP_OM_SetTrapFlagNewRoot(TRUE);
#if (SYS_CPNT_DEBUG == TRUE)
        XSTP_UTY_DebugPrintRoot(om_ptr, TRUE);
    }
    else if (cmp_a != 0)
    {
        XSTP_UTY_DebugPrintRoot(om_ptr, FALSE);
#endif
    }

    root_lport      = (UI32_T)(om_ptr->bridge_info.root_port_id.port_id & 0x0FFF);
    lport           = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr     = &(om_ptr->port_info[lport-1]);

        if (pom_ptr->is_member)
        {
            /* 13.26.25 (d) */
            if (pom_ptr->common->rcvd_stp) /* 13.10 */
            {
                XSTP_OM_PRIORITY_VECTOR_FORMAT( pom_ptr->designated_priority,
                                                root_priority_vector.root_id,
                                                root_priority_vector.ext_root_path_cost,
                                                om_ptr->bridge_info.bridge_identifier,
                                                root_priority_vector.int_root_path_cost,
                                                om_ptr->bridge_info.bridge_identifier,
                                                pom_ptr->port_id,
                                                pom_ptr->port_id);
            }
            else
            {
                XSTP_OM_PRIORITY_VECTOR_FORMAT( pom_ptr->designated_priority,
                                                root_priority_vector.root_id,
                                                root_priority_vector.ext_root_path_cost,
                                                root_priority_vector.r_root_id,
                                                root_priority_vector.int_root_path_cost,
                                                om_ptr->bridge_info.bridge_identifier,
                                                pom_ptr->port_id,
                                                pom_ptr->port_id);
            }
            /* 13.26.25 (e) */
            memcpy( &pom_ptr->designated_times,
                    &root_times,
                    XSTP_TYPE_TIMERS_LENGTH
                  );

            switch (pom_ptr->info_is)
            {
                /* 13.26.25 (f) */
                case XSTP_ENGINE_PORTVAR_INFO_IS_DISABLED:
                    pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_DISABLED;
                    break;

                /* 13.26.25 (g) */
                case XSTP_ENGINE_PORTVAR_INFO_IS_AGED:
                    pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED;
                    pom_ptr->updt_info      = TRUE;
                    break;

                /* 13.26.25 (h) */
                case XSTP_ENGINE_PORTVAR_INFO_IS_MINE:
                    pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED;
                    if (root_lport == 0)
                    {
                        memcpy( &root_port_times,
                                &om_ptr->bridge_info.bridge_times,
                                XSTP_TYPE_TIMERS_LENGTH
                              );
                    }
                    else
                    {
                        XSTP_OM_PortVar_T   *root_pom_ptr;
                        root_pom_ptr    = &(om_ptr->port_info[root_lport-1]);
                        memcpy( &root_port_times,
                                &root_pom_ptr->port_times,
                                XSTP_TYPE_TIMERS_LENGTH
                              );
                    }
                    XSTP_OM_CMP_PRIORITY_VECTOR(cmp_d, pom_ptr->port_priority, pom_ptr->designated_priority);
                    XSTP_OM_CMP_TIMERS(cmp_e, pom_ptr->port_times, root_port_times);
                    if (    (cmp_d != 0)
                         || (cmp_e != 0)
                       )
                    {
                        pom_ptr->updt_info  = TRUE;
                    }
                    break;

                /* 13.26.25 (i, j, k) */
                case XSTP_ENGINE_PORTVAR_INFO_IS_RECEIVED:
                    if (lport == root_lport)
                    {
                        /* 13.26.25 (i) */
                        pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_ROOT;
                        pom_ptr->updt_info      = FALSE;
                    }
                    else
                    {
                        XSTP_OM_CMP_PRIORITY_VECTOR(cmp_f, pom_ptr->designated_priority, pom_ptr->port_priority);
                        if (cmp_f >= 0)
                        {
                            XSTP_OM_CMP_BRIDGE_ID(cmp_g, pom_ptr->port_priority.designated_bridge_id, om_ptr->bridge_info.bridge_identifier);
                            XSTP_OM_CMP_PORT_ID(cmp_h, pom_ptr->port_priority.designated_port_id, pom_ptr->port_id);
                            if (cmp_g != 0)
                            {
                                /* 13.26.25 (j) */
                                pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE;
                                pom_ptr->updt_info      = FALSE;
                            }
                            else
                            if (cmp_h != 0)
                            {
                                /* 13.26.25 (k) */
                                pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_BACKUP;
                                pom_ptr->updt_info      = FALSE;
                            }
                        }
                        else
                        {
                            /* 13.26.25 (l) Retired root port */
                            pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED;
                            pom_ptr->updt_info      = TRUE;
                        }
                    }
                    break;
            } /* End of switch (pom_ptr->info_is) */
        } /* End of if(pom_ptr->is_member) */
    } /* End of while(lport is existent) */

    mstid               = XSTP_TYPE_CISTID;
    temp_om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    while(XSTP_OM_GetNextInstanceInfoPtr(&mstid, &temp_om_ptr))
    {
        temp_om_ptr->bridge_info.cist_role_updated   = TRUE;
    }
    return;
}/* End of XSTP_UTY_UpdtRolesCist */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtRolesMsti
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.26, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
static void    XSTP_UTY_UpdtRolesMsti(XSTP_OM_InstanceData_T *om_ptr)
{
    XSTP_TYPE_PriorityVector_T  root_priority_vector;
    XSTP_TYPE_PriorityVector_T  root_path_priority_vector;
    XSTP_TYPE_Timers_T          root_times;
    XSTP_TYPE_Timers_T          root_port_times;
    XSTP_OM_PortVar_T           *pom_ptr;
    UI32_T                      lport;
    UI32_T                      root_lport;
    BOOL_T                      is_root_bridge;
    XSTP_TYPE_BridgeId_T        bridge_id;
    I32_T                       cmp_result, cmp_a, cmp_b, cmp_c, cmp_d, cmp_e, cmp_f, cmp_g, cmp_h, cmp_i;

    /* Patch by Allen Cheng */
    om_ptr->bridge_info.cist_role_updated   = FALSE;

    /* Check if this bridge is root before selection */
    /* For new_root trap */
    is_root_bridge = XSTP_UTY_RootBridge(om_ptr);


    /* Determine the root_priority_vector */
    /* root_priority_vector :=
     * <Best of> {bridge_priority_vector} U {root_path_priority_vector|for all ports}
     */
    /* 13.26.26 :: (a) ~ (d) */
    memcpy(&root_priority_vector, &om_ptr->bridge_info.bridge_priority, XSTP_TYPE_PRIORITY_VECTOR_LENGTH);
    memcpy(&root_times, &om_ptr->bridge_info.bridge_times, XSTP_TYPE_TIMERS_LENGTH);
    lport           = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr     = &(om_ptr->port_info[lport-1]);
        if (pom_ptr->is_member)
        {
            if (    (!pom_ptr->common->info_internal)
                &&  (pom_ptr->info_is == XSTP_ENGINE_PORTVAR_INFO_IS_RECEIVED)
               )
            {
                XSTP_OM_PRIORITY_VECTOR_FORMAT( pom_ptr->port_priority,
                                                bridge_id,
                                                0,
                                                om_ptr->bridge_info.bridge_identifier,
                                                0,
                                                om_ptr->bridge_info.bridge_identifier,
                                                pom_ptr->port_id,
                                                pom_ptr->port_id);
            }

            /* 13.26.26 (a) */
            memset(&bridge_id, 0, XSTP_TYPE_BRIDGE_ID_LENGTH);
            XSTP_OM_PRIORITY_VECTOR_FORMAT( root_path_priority_vector,
                                            bridge_id,
                                            0,
                                            pom_ptr->port_priority.r_root_id,
                                            pom_ptr->port_priority.int_root_path_cost + pom_ptr->internal_port_path_cost,
                                            pom_ptr->port_priority.designated_bridge_id,
                                            pom_ptr->port_priority.designated_port_id,
                                            pom_ptr->port_id);

            /* 13.26.26 (b) */
            XSTP_OM_CMP_PRIORITY_VECTOR(cmp_a, root_priority_vector, root_path_priority_vector);
            if (    (pom_ptr->info_is != XSTP_ENGINE_PORTVAR_INFO_IS_DISABLED)
                 && (pom_ptr->rcvd_info_while > 0)
                 && (memcmp(&root_path_priority_vector.designated_bridge_id.addr,
                            &om_ptr->bridge_info.bridge_priority.designated_bridge_id.addr,
                            6
                           ) != 0
                    )
                 && (cmp_a > 0)
               )
            {
                XSTP_OM_CMP_PRIORITY_VECTOR(cmp_result, om_ptr->bridge_info.bridge_priority, root_path_priority_vector);
                if (cmp_result < 0)
                {
                    memcpy( &root_priority_vector,
                            &om_ptr->bridge_info.bridge_priority,
                            XSTP_TYPE_PRIORITY_VECTOR_LENGTH
                          );
                }
                else
                {
                    memcpy( &root_priority_vector,
                            &root_path_priority_vector,
                            XSTP_TYPE_PRIORITY_VECTOR_LENGTH
                          );
                }

                /* 13.26.26 (c) */
                XSTP_OM_CMP_PRIORITY_VECTOR(cmp_b, om_ptr->bridge_info.bridge_priority, root_priority_vector);
                if (cmp_b == 0)
                {
                    memcpy( &root_times,
                            &om_ptr->bridge_info.bridge_times,
                            XSTP_TYPE_TIMERS_LENGTH
                          );
                }
                else
                {
                    /* 13.23.13 mstiRootTimes */
                    memcpy( &root_times,
                            &pom_ptr->port_times,
                            XSTP_TYPE_TIMERS_LENGTH
                          );
                    root_times.remaining_hops  = root_times.remaining_hops - 1;

                }
            }
        } /* End of if (pom_ptr->is_member) */
    } /* End of while (lport is existent) */

    /* private trap: root bridge changed */
    XSTP_OM_CMP_BRIDGE_ID(cmp_a, root_priority_vector.r_root_id, om_ptr->bridge_info.root_priority.r_root_id);
    if (cmp_a != 0)
    {
        /* root bridge is changed */
        TRAP_EVENT_TrapData_T   trap_data;

        trap_data.trap_type = TRAP_EVENT_XSTP_ROOT_BRIDGE_CHANGED;
        XSTP_OM_GET_BRIDGE_ID_PRIORITY(
            trap_data.u.xstp_root_bridge_changed.priority,
            root_priority_vector.r_root_id);
        trap_data.u.xstp_root_bridge_changed.instance_id = om_ptr->instance_id;
        memcpy(trap_data.u.xstp_root_bridge_changed.bridge_address,
            &root_priority_vector.r_root_id.addr, 6);
        trap_data.community_specified = FALSE;
        trap_data.flag = TRAP_EVENT_SEND_TRAP_OPTION_DEFAULT;
        SNMP_PMGR_ReqSendTrap(&trap_data);
    }

    /* root_priority */
    memcpy( &om_ptr->bridge_info.root_priority,
            &root_priority_vector,
            XSTP_TYPE_PRIORITY_VECTOR_LENGTH
          );
    /* root_port_id */
    memcpy( &om_ptr->bridge_info.root_port_id,
            &root_priority_vector.rcv_port_id,
            XSTP_TYPE_PORT_ID_LENGTH
          );
    /* root_times */
    memcpy( &om_ptr->bridge_info.root_times,
            &root_times,
            XSTP_TYPE_TIMERS_LENGTH
          );

    if (!is_root_bridge && XSTP_UTY_RootBridge(om_ptr))
    {
        /* This bridge is the new_root */
        XSTP_OM_SetTrapFlagNewRoot(TRUE);
#if (SYS_CPNT_DEBUG == TRUE)
        XSTP_UTY_DebugPrintRoot(om_ptr, TRUE);
    }
    else if (cmp_a != 0)
    {
        XSTP_UTY_DebugPrintRoot(om_ptr, FALSE);
#endif
    }

    root_lport      = (UI32_T)(om_ptr->bridge_info.root_port_id.port_id & 0x0FFF);
    lport           = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr     = &(om_ptr->port_info[lport-1]);

        if (pom_ptr->is_member)
        {
            /* 13.26.26 (d) */
            memset(&bridge_id, 0, XSTP_TYPE_BRIDGE_ID_LENGTH);
            XSTP_OM_PRIORITY_VECTOR_FORMAT( pom_ptr->designated_priority,
                                            bridge_id,
                                            0,
                                            root_priority_vector.r_root_id,
                                            root_priority_vector.int_root_path_cost,
                                            om_ptr->bridge_info.bridge_identifier,
                                            pom_ptr->port_id,
                                            pom_ptr->port_id);

            /* 13.26.26 (e) */
            memcpy( &pom_ptr->designated_times,
                    &root_times,
                    XSTP_TYPE_TIMERS_LENGTH
                  );

            /* 13.26.26 (f) */
            if (pom_ptr->info_is == XSTP_ENGINE_PORTVAR_INFO_IS_DISABLED)
            {
                pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_DISABLED;
            }
            else /* 13.26.26 (g) (h) */
            if (    (pom_ptr->cist->info_is == XSTP_ENGINE_PORTVAR_INFO_IS_RECEIVED)
                &&  !(pom_ptr->common->info_internal)
               )
            {
                if (pom_ptr->cist->selected_role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                {
                    pom_ptr->selected_role = XSTP_ENGINE_PORTVAR_ROLE_MASTER;
                }
                else
                if (pom_ptr->cist->selected_role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
                {
                    pom_ptr->selected_role = XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE;
                }

                if (root_lport == 0)
                {
                    memcpy( &root_port_times,
                            &om_ptr->bridge_info.bridge_times,
                            XSTP_TYPE_TIMERS_LENGTH
                          );
                }
                else
                {
                    XSTP_OM_PortVar_T   *root_pom_ptr;
                    root_pom_ptr    = &(om_ptr->port_info[root_lport-1]);
                    memcpy( &root_port_times,
                            &root_pom_ptr->port_times,
                            XSTP_TYPE_TIMERS_LENGTH
                          );
                }
                XSTP_OM_CMP_PRIORITY_VECTOR(cmp_c, pom_ptr->port_priority, pom_ptr->designated_priority);
                XSTP_OM_CMP_TIMERS(cmp_d, pom_ptr->port_times, root_port_times);
                if (    (cmp_c != 0)
                     || (cmp_d != 0)
                    )
                {
                    pom_ptr->updt_info  = TRUE;
                }

            }
            else /* 13.26.26 (i) (j) (k) (l) (m) (n) */
            if (    (pom_ptr->cist->info_is != XSTP_ENGINE_PORTVAR_INFO_IS_RECEIVED)
                ||  (pom_ptr->common->info_internal)
               )
            {
                switch (pom_ptr->info_is)
                {
                    /* 13.26.26 (i) */
                    case XSTP_ENGINE_PORTVAR_INFO_IS_AGED:
                        pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED;
                        pom_ptr->updt_info      = TRUE;
                        break;
                    /* 13.26.26 (j) */
                    case XSTP_ENGINE_PORTVAR_INFO_IS_MINE:
                        pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED;
                        if (root_lport == 0)
                        {
                            memcpy( &root_port_times,
                                    &om_ptr->bridge_info.bridge_times,
                                    XSTP_TYPE_TIMERS_LENGTH
                                   );
                        }
                        else
                        {
                            XSTP_OM_PortVar_T   *root_pom_ptr;
                            root_pom_ptr    = &(om_ptr->port_info[root_lport-1]);
                            memcpy( &root_port_times,
                                    &root_pom_ptr->port_times,
                                    XSTP_TYPE_TIMERS_LENGTH
                                  );
                        }
                        XSTP_OM_CMP_PRIORITY_VECTOR(cmp_e, pom_ptr->port_priority, pom_ptr->designated_priority);
                        XSTP_OM_CMP_TIMERS(cmp_f, pom_ptr->port_times, root_port_times);
                        if (    (cmp_e != 0)
                             || (cmp_f != 0)
                            )
                        {
                            pom_ptr->updt_info  = TRUE;
                        }
                        break;
                case XSTP_ENGINE_PORTVAR_INFO_IS_RECEIVED:
                    if (lport == root_lport)
                    {
                        /* 13.26.26 (k) */
                        pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_ROOT;
                        pom_ptr->updt_info      = FALSE;
                    }
                    else
                    {
                        XSTP_OM_CMP_PRIORITY_VECTOR(cmp_g, pom_ptr->designated_priority, pom_ptr->port_priority);
                        if (cmp_g >= 0)
                        {
                            XSTP_OM_CMP_BRIDGE_ID(cmp_h, pom_ptr->port_priority.designated_bridge_id, om_ptr->bridge_info.bridge_identifier);
                            XSTP_OM_CMP_PORT_ID(cmp_i, pom_ptr->port_priority.designated_port_id, pom_ptr->port_id);
                            if (cmp_h != 0)
                            {
                                /* 13.26.26 (l) */
                                pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE;
                                pom_ptr->updt_info      = FALSE;
                            }
                            else
                            if (cmp_i != 0)
                            {
                                /* 13.26.26 (m) */
                                pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_BACKUP;
                                pom_ptr->updt_info      = FALSE;
                            }
                        }
                        else
                        {
                            /* 13.26.26 (n) Retired root port */
                            pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED;
                            pom_ptr->updt_info      = TRUE;
                        }
                    }
                    break;
                }/* End of switch (pom_ptr->info_is) */
            }
        }/* End of if(pom_ptr->is_member) */
    }/* End of while(lport is existent) */
    return;
}/* End of XSTP_UTY_UpdtRolesMsti */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SendMstpBpdu
 * ------------------------------------------------------------------------
 * PURPOSE  : Send MSTP BPDU
 * INPUT    : lport     -- lport
 *            bpdu      -- Mstp BPDU content
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static  void    XSTP_UTY_SendMstpBpdu(UI32_T lport, XSTP_TYPE_MstBpdu_T *bpdu)
{
    L_MM_Mref_Handle_T      *mref_handle_p;
    UI8_T                   my_mac_addr[6];
    XSTP_TYPE_MstBpdu_T     *mstp_bpdu_ptr;
    UI16_T                  version_3_length;
    UI16_T                  packet_size;
    UI32_T                  pdu_len;

    version_3_length    = XSTP_UTY_GetVersion3length(lport);
    packet_size         = version_3_length+38+3+18;

    mref_handle_p = L_MM_AllocateTxBuffer(packet_size,
                                          L_MM_USER_ID2(SYS_MODULE_XSTP, XSTP_TYPE_TRACE_ID_XSTP_UTY_SENDMSTPBPDU));/*,
                                          NULL, NULL);*/
    if(mref_handle_p==NULL)
    {
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("\r\n%s::ERROR!! for L_MM_AllocateTxBuffer return NULL", __FUNCTION__);
        L_MM_Free(bpdu);
        return;
    }

    mstp_bpdu_ptr = (XSTP_TYPE_MstBpdu_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if(mstp_bpdu_ptr==NULL)
    {
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("\r\n%s::ERROR!! for L_MM_Mref_GetPdu return NULL", __FUNCTION__);
        L_MM_Free(bpdu);
        return;
    }

    memcpy(mstp_bpdu_ptr, bpdu, (packet_size-18) );
    L_MM_Free(bpdu);

    mstp_bpdu_ptr->sap                           = L_STDLIB_Hton16(XSTP_TYPE_STP_SAP);
    mstp_bpdu_ptr->ctrl_byte                     = XSTP_TYPE_UI_CONTROL_BYTE;
    mstp_bpdu_ptr->protocol_identifier           = L_STDLIB_Hton16(XSTP_TYPE_PROTOCOL_ID);
    mstp_bpdu_ptr->protocol_version_identifier   = XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID;
    mstp_bpdu_ptr->bpdu_type                     = XSTP_TYPE_BPDU_TYPE_XST;

    /* === Check  link === */
    {
        UI32_T link_status;
        Port_Info_T port_info;
        if (!SWCTRL_GetPortInfo(lport, &port_info))
        {
            L_MM_Mref_Release(&mref_handle_p);
            return;
        }
        link_status = port_info.link_status;
        if (link_status == SWCTRL_LINK_DOWN)
        {
            L_MM_Mref_Release(&mref_handle_p);
            return;
        }
    }

    /* === Get My Mac === */
    if (!SWCTRL_GetPortMac(lport, my_mac_addr))
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    /* === Send packet === */
    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_TXRSTP))
    {
        UI8_T                   i;
        UI32_T                  current_time;

        SYS_TIME_GetRealTimeBySec(&current_time);

        BACKDOOR_MGR_Printf("\r\n");
        BACKDOOR_MGR_Printf("\r\nTXMSTP::");
        BACKDOOR_MGR_Printf("[TxToLport=%ld]", lport);
        BACKDOOR_MGR_Printf("[Flags = 0x%02x]", mstp_bpdu_ptr->cist_flags);
        BACKDOOR_MGR_Printf("[RootId=%4x", mstp_bpdu_ptr->cist_root_identifier.bridge_id_priority.bridge_priority);
        for (i=0; i<6; i++) BACKDOOR_MGR_Printf(":%02x", mstp_bpdu_ptr->cist_root_identifier.addr[i]);
        BACKDOOR_MGR_Printf("][CistERC = 0x%04lx]", mstp_bpdu_ptr->cist_external_path_cost);
        BACKDOOR_MGR_Printf("[RRoot=%4x", mstp_bpdu_ptr->cist_regional_root_identifier.bridge_id_priority.bridge_priority);
        for (i=0; i<6; i++) BACKDOOR_MGR_Printf(":%02x", mstp_bpdu_ptr->cist_regional_root_identifier.addr[i]);
        BACKDOOR_MGR_Printf("][CistIRC = 0x%04lx]", mstp_bpdu_ptr->cist_internal_root_path_cost);
        BACKDOOR_MGR_Printf("[BridgeId=%4x", mstp_bpdu_ptr->cist_bridge_identifier.bridge_id_priority.bridge_priority);
        for (i=0; i<6; i++) BACKDOOR_MGR_Printf(":%02x", mstp_bpdu_ptr->cist_bridge_identifier.addr[i]);
        BACKDOOR_MGR_Printf("][PortId = 0x%02x]", mstp_bpdu_ptr->cist_port_identifier.port_id);
        BACKDOOR_MGR_Printf("[TxTime = %ld]", current_time);
    }

    XSTP_UTY_SendPacket(mref_handle_p,
                        XSTP_UTY_BridgeGroupAddr,
                        my_mac_addr,
                        (packet_size-18),
                        SYS_TYPE_IGNORE_VID_CHECK,
                        (packet_size-18),
                        lport,
                        0,
                        XSTP_TYPE_BPDU_TXCOS);

    return;
}/* End of XSTP_UTY_SendMstpBpdu */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_GetVersion3length
 * ------------------------------------------------------------------------
 * PURPOSE  : Get version 3 length value.
 * INPUT    : lport     -- lport
 * OUTPUT   : None
 * RETURN   : the version 3 length value.
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static UI16_T  XSTP_UTY_GetVersion3length(UI32_T lport)
{
    UI32_T                  xstid;
    XSTP_OM_InstanceData_T  *om_ptr;
    UI16_T                  version_3_length;
    UI8_T                   count;

    version_3_length = count = 0;
    xstid               = XSTP_TYPE_CISTID;
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &om_ptr))
    {
        if (    (om_ptr->instance_exist)
             && (!om_ptr->dirty_bit)
             && (XSTP_OM_IsMemberPortOfInstance(om_ptr, lport) == TRUE)
           )
        {
            count++;
        }
    }
    version_3_length = (count*XSTP_TYPE_MST_CONFIG_MSG_LENGTH+102-38);
    return version_3_length;
}/* End of XSTP_UTY_GetVersion3length */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_ChangedMaster
 * ------------------------------------------------------------------------
 * PURPOSE  : Set, for all Ports for all MSTIs, by the CIST Port Role
 *            Selection state machine using the updtRoleCist() pro-cedure
 *            if the root priority vector selected has a different Regional
 *            Root Identifier than that previously selected, and has or had
 *            a non-zero CIST External Path Cost. changedMaster is always
 *            FALSE for the CIST.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.24.3, IEEE Std 802.1s(D15)-2002
 *-------------------------------------------------------------------------
 */
static void  XSTP_UTY_ChangedMaster(void)
{
    UI32_T                  mstid;
    UI32_T                  lport;
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    mstid               = XSTP_TYPE_CISTID;
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    while(XSTP_OM_GetNextInstanceInfoPtr(&mstid, &om_ptr))   /* exclude CIST */
    {
        if (om_ptr->instance_exist)
        {
            lport               = 0;
            while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                pom_ptr         = &(om_ptr->port_info[lport-1]);
                if (pom_ptr->is_member)
                {
                    pom_ptr->changed_master = TRUE;
                }
            }
        }
    }
    return;

} /* End of XSTP_UTY_ChangedMaster */

#endif /* XSTP_TYPE_PROTOCOL_MSTP */


/* ===================================================================== */
/* ===================================================================== */
/* Common utilities used by both RSTP and MSTP */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetMstEnableStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the XSTP_UTY_MstEnable flag
 * INPUT    : state         -- TRUE/FALSE
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : If the protocol version is less than MSTP's, XSTP_UTY_MstEnable
 *            is always set to FALSE.
 */
void    XSTP_UTY_SetMstEnableStatus(BOOL_T state)
{
    if (XSTP_TYPE_PROTOCOL_VERSION_ID == XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID)
    {
        XSTP_UTY_MstEnable  = state;
    }
    else
    {
        XSTP_UTY_MstEnable  = FALSE;
    }

    return;
} /* End of XSTP_UTY_SetMstEnableStatus */

#if 0
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetPortState
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the spanning port state to SWCTRL
 * INPUT    : om_ptr        -- om pointer for this instance
 *            lport         -- lport
 *            state         -- stp port state
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : If the XSTP_UTY_MstEnable flag is set then the SWCTRL is invoked
 *            per vlan for the specified port.
 */
BOOL_T  XSTP_UTY_SetPortState(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, UI32_T state)
{
    XSTP_OM_PortVar_T   *pom_ptr;
    BOOL_T              result = FALSE;

    pom_ptr = &(om_ptr->port_info[lport-1]);
    if (XSTP_UTY_MstEnable)
    {
        /* MST mode */
        UI32_T          vid, vid_ifindex;

        vid = 0;
        result  = FALSE;
        if (XSTP_OM_IsMemberPortOfInstance(om_ptr, lport) )
        {
            while (XSTP_OM_GetNextXstpMember(om_ptr, &vid) )
            {
                VLAN_OM_ConvertToIfindex(vid, &vid_ifindex);
                if (VLAN_OM_IsPortVlanMember(vid_ifindex, lport))
                {
                    UI32_T  mstid;
                    XSTP_OM_GetMstidFromMstConfigurationTableByVlan(vid, &mstid);
                    if (om_ptr->instance_id == mstid)
                    {
                        /* Set the port state to the lower layer (chip)
                         * if this is the om of an MSTI.
                         * For the om of the CIST, the state is not set
                         * to the lower layer if the specified port joins
                         * an MSTI.
                         */
                        SWCTRL_PMGR_SetPortSTAState(vid, lport, state);
                        result  = TRUE;
                    } /* End of if */
                } /* if the vlan in this instance contains the specified port */
            } /* End of while */
        } /* End of if */
    }
    else
    {
        result  = SWCTRL_PMGR_SetPortSTAState(0, lport, state);
    }

    return result;
} /* End of XSTP_UTY_SetPortState */

#endif

#if 0
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetPortState
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the spanning port state to SWCTRL
 * INPUT    : om_ptr        -- om pointer for this instance
 *            lport         -- lport
 *            state         -- stp port state
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : If the XSTP_UTY_MstEnable flag is set then the SWCTRL is invoked
 *            per vlan for the specified port.
 */
BOOL_T  XSTP_UTY_SetPortState(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, UI32_T state)
{
    BOOL_T  result;
    UI32_T  mstid;
    UI32_T  vid;

    mstid   = om_ptr->instance_id;
    vid     = 0;
    if (mstid == XSTP_TYPE_CISTID)
    {
        if (XSTP_UTY_MstEnable)
        {
            memset(XSTP_UTY_InstanceVlansMapped, 0, 512);
            while(XSTP_OM_GetNextExistingMemberVidByMstid(mstid, &vid))
            {
                XSTP_UTY_InstanceVlansMapped[vid >> 3] = XSTP_UTY_InstanceVlansMapped[vid >> 3] | (0x01 << (vid % 8));

                if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
                {
                    BACKDOOR_MGR_Printf("\r\nXSTP_UTY_SetPortState : mstid = %ld, lport = %ld, vlan_id = %ld , state = %ld", om_ptr->instance_id, lport, vid, state);
                }
            }
            result = SWCTRL_SetPortXstpState(om_ptr->instance_id, XSTP_UTY_InstanceVlansMapped, lport, state);
        }
        else
        {
            result = SWCTRL_SetPortXstpState(om_ptr->instance_id, om_ptr->instance_vlans_mapped, lport, state);
        }
    }
    else
    {
        result = SWCTRL_SetPortXstpState(om_ptr->instance_id, om_ptr->instance_vlans_mapped, lport, state);
    }

    return result;
} /* End of XSTP_UTY_SetPortState */
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetPortState
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the spanning port state to SWCTRL
 * INPUT    : om_ptr        -- om pointer for this instance
 *            lport         -- lport
 *            state         -- stp port state
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : If the XSTP_UTY_MstEnable flag is set then the SWCTRL is invoked
 *            per vlan for the specified port.
 */
BOOL_T  XSTP_UTY_SetPortState(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, UI32_T state)
{
    BOOL_T  result;
    UI32_T  mstidx;

    mstidx = (UI32_T) XSTP_OM_GetInstanceEntryId(om_ptr->instance_id);

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    result = SWCTRL_PMGR_SetPortStateWithMstidx(mstidx, lport, state);

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);

    return result;
} /* End of XSTP_UTY_SetPortState */



/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetVlanPortState
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the spanning port enter/leave forwarding state to VLAN_MGR
 * INPUT    : om_ptr        -- om pointer for this instance
 *            lport         -- lport
 *            forwarding    -- TRUE if enter_forwarding, else FALSE for leave_forwarding
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : If the XSTP_UTY_MstEnable flag is set then the VLAN_MGR is invoked
 *            per vlan for the specified port, else set vid=0 for all vlans.
 */
static  BOOL_T  XSTP_UTY_SetVlanPortState(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, BOOL_T forwarding)
{
    /* Notify Leave_Forwarding */
    if (XSTP_UTY_MstEnable){
        /* MST mode */
        UI32_T          vid, vid_ifindex;
        vid = 0;
        while (XSTP_OM_GetNextXstpMember(om_ptr, &vid) ){

            VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
            if (VLAN_OM_IsPortVlanMember(vid_ifindex, lport)){
                    VLAN_PMGR_NotifyForwardingState(vid, lport, forwarding);
                    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG)){
                        if (forwarding){
                            BACKDOOR_MGR_Printf("\r\nXSTP_UTY_SetVlanPortState::Notify Enter Forwarding to Vlan:for lport %ld in vlan %ld", lport, vid);
                        }else{
                            BACKDOOR_MGR_Printf("\r\nXSTP_UTY_SetVlanPortState::Notify Leave Forwarding to Vlan:for lport %ld in vlan %ld", lport, vid);
                        }
                    }
            }
         }
    }else{
        VLAN_PMGR_NotifyForwardingState(0, lport, forwarding);
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG)){
            if (forwarding){
                BACKDOOR_MGR_Printf("\r\nXSTP_UTY_SetVlanPortState::Notify Enter Forwarding to Vlan:for lport %ld at all Vlans", lport);
            }else{
                BACKDOOR_MGR_Printf("\r\nXSTP_UTY_SetVlanPortState::Notify Leave Forwarding to Vlan:for lport %ld at all Vlans", lport);
            }
        }
    }

    return TRUE;
} /* End of XSTP_UTY_SetVlanPortState */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_FloodingBpdu
 *-------------------------------------------------------------------------
 * PURPOSE  : Flooding this packet to all the ports other than the receiver itself
 * INPUT    : bpdu_msg_ptr  -- BPDU message pointer
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void    XSTP_UTY_FloodingBpdu(UI32_T lport, XSTP_TYPE_MSG_T *bpdu_msg_ptr, BOOL_T is_tagged
                              ,UI32_T global_status, UI32_T rcv_port_status)
{
    XSTP_OM_InstanceData_T      *om_ptr;
    UI32_T  original_pdu_len;
    UI32_T  current_pdu_len;
    UI16_T  vid;
    UI32_T  tmp_lport;
    BOOL_T                      flooding_stp_disabled_port;

    if (L_MM_Mref_GetPdu(bpdu_msg_ptr->mref_handle_p, &original_pdu_len) == NULL)
        return;

    vid = 0x0FFF & (bpdu_msg_ptr->msg_type);

    /* Check tagged or untagged BPDU*/
    if (is_tagged == FALSE)
    {
        vid = SYS_TYPE_IGNORE_VID_CHECK;
    }

    flooding_stp_disabled_port = (global_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)&& (rcv_port_status == VAL_staPortSystemStatus_disabled);

    tmp_lport = 0;
    while (SWCTRL_GetNextLogicalPort(&tmp_lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        if ((tmp_lport != lport) && (SWCTRL_isPortLinkUp(tmp_lport)))
        {
            om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
            /* Global enabled, port disabled, only flood to other disabled port*/
            if (   flooding_stp_disabled_port
                && (om_ptr->port_info[tmp_lport-1].common->port_spanning_tree_status != VAL_staPortSystemStatus_disabled))
            {
                continue;
            }

            if (L_MM_Mref_AddRefCount(bpdu_msg_ptr->mref_handle_p, 1) == TRUE)
            {
                XSTP_UTY_SendPacket(bpdu_msg_ptr->mref_handle_p,
                                    XSTP_UTY_BridgeGroupAddr,
                                    bpdu_msg_ptr->saddr,
                                    bpdu_msg_ptr->pkt_length,
                                    vid,
                                    bpdu_msg_ptr->pkt_length,
                                    tmp_lport,
                                    is_tagged,
                                    XSTP_TYPE_BPDU_TXCOS);
            }
            else
            {
                SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_AddRefCount fail", __FUNCTION__);
                break;
            }

            /* LAN will change pdu position of mref_handle_p for sending packet
             * need to move it back to original place
             */
            if (L_MM_Mref_GetPdu(bpdu_msg_ptr->mref_handle_p, &current_pdu_len) != NULL)
            {
                L_MM_Mref_MovePdu(bpdu_msg_ptr->mref_handle_p, (I32_T)current_pdu_len - (I32_T)original_pdu_len, &current_pdu_len);
            }
            else
            {
                SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu fail", __FUNCTION__);
                break;
            }

            /* Flush MAC if received BPDU with TC bit*/
            if (flooding_stp_disabled_port)
            {
                XSTP_UTY_FlushPort(tmp_lport, bpdu_msg_ptr);
            }
        }
    } /* End of while */
    return;
} /* End of XSTP_UTY_FloodingBpdu */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RecordDispute
 *-------------------------------------------------------------------------
 * PURPOSE  : Sets the disputed variable and clears the agreed variable
 *            for the port if an RST BPDU with the learning flag set has
 *            been received.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. Ref to the description in 17.21.10, IEEE Std 802.1D/D3-2003
 * ------------------------------------------------------------------------
 */
static void    XSTP_UTY_RecordDispute(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    BOOL_T                   learning = FALSE;

    pom_ptr     = &(om_ptr->port_info[lport-1]);
    #ifdef XSTP_TYPE_PROTOCOL_RSTP
    {
        XSTP_TYPE_RstBpdu_T     *bpdu;
        bpdu        = (XSTP_TYPE_RstBpdu_T*)pom_ptr->common->bpdu;
        if ((bpdu->flags & XSTP_TYPE_BPDU_FLAGS_LEARNING) != 0 )
        {
            learning = TRUE;
        }
    }
    #endif /* XSTP_TYPE_PROTOCOL_RSTP */

    #ifdef XSTP_TYPE_PROTOCOL_MSTP
    {
        XSTP_TYPE_MstBpdu_T     *bpdu;
        bpdu        = (XSTP_TYPE_MstBpdu_T*)pom_ptr->common->bpdu;
        if (XSTP_UTY_Cist(om_ptr))
        {
            if ((bpdu->cist_flags & XSTP_TYPE_BPDU_FLAGS_LEARNING) != 0 )
            {
                learning = TRUE;
            }
        }
        else
        {
            if ((pom_ptr->msti_config_msg->msti_flags & XSTP_TYPE_BPDU_FLAGS_LEARNING) != 0 )
            {
                learning = TRUE;
            }
        }
    }
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    if (learning)
    {
        pom_ptr->disputed       = TRUE;
        pom_ptr->agreed         = FALSE;
    }
#if 0
    if (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
    {
        XSTP_UTY_SetRootBridge(om_ptr);
    }
    if (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
    {
        XSTP_UTY_SetDesignatedPort(om_ptr, lport);
    }
#endif
    return;
}/* End of XSTP_UTY_RecordDispute */

#if 0
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetRootBridge
 * ------------------------------------------------------------------------
 * PURPOSE  :   This Bridge will be selected as Root bridge for this instance.
 * INPUT    :   om_ptr           -- the pointer of the instance entry.
 * OUTPUT   :   None.
 * RETURN   :   None.
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
 static void    XSTP_UTY_SetRootBridge(XSTP_OM_InstanceData_T *om_ptr)
 {
    UI32_T                          lport;
    XSTP_OM_PortVar_T               *pom_ptr;

    /* root_priority_vector */
    memcpy(&om_ptr->bridge_info.root_priority, &om_ptr->bridge_info.bridge_priority, sizeof(XSTP_TYPE_PriorityVector_T));

    /* root_times */
    memcpy(&om_ptr->bridge_info.root_times, &om_ptr->bridge_info.bridge_times, sizeof(XSTP_TYPE_Timers_T));

    lport   = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {

        pom_ptr = &(om_ptr->port_info[lport-1]);

        #ifdef  XSTP_TYPE_PROTOCOL_RSTP
        /* port_priority */
        XSTP_OM_PRIORITY_VECTOR_FORMAT( pom_ptr->port_priority,
                                        om_ptr->bridge_info.bridge_identifier,
                                        0,
                                        om_ptr->bridge_info.bridge_identifier,
                                        pom_ptr->port_id,
                                        pom_ptr->port_id);
        #endif  /* XSTP_TYPE_PROTOCOL_RSTP */

        #ifdef  XSTP_TYPE_PROTOCOL_MSTP
        if (om_ptr->instance_id == XSTP_TYPE_CISTID)    /* CIST */
        {
            XSTP_OM_PRIORITY_VECTOR_FORMAT( pom_ptr->port_priority,
                                            om_ptr->bridge_info.bridge_identifier,
                                            0,
                                            om_ptr->bridge_info.bridge_identifier,
                                            0,
                                            om_ptr->bridge_info.bridge_identifier,
                                            pom_ptr->port_id,
                                            pom_ptr->port_id);
        }
        else                                            /* MSTI */
        {
            XSTP_TYPE_BridgeId_T    null_bridge_id;
            memset(&null_bridge_id, 0, XSTP_TYPE_BRIDGE_ID_LENGTH);
            XSTP_OM_PRIORITY_VECTOR_FORMAT( pom_ptr->port_priority,
                                            null_bridge_id,
                                            NULL,
                                            om_ptr->bridge_info.bridge_identifier,
                                            0,
                                            om_ptr->bridge_info.bridge_identifier,
                                            pom_ptr->port_id,
                                            pom_ptr->port_id);
        }
        #endif  /* XSTP_TYPE_PROTOCOL_MSTP */

        /* port_times */
        memcpy(&pom_ptr->port_times, &om_ptr->bridge_info.bridge_times, sizeof(XSTP_TYPE_Timers_T));
        /* designated_priority */
        memcpy(&pom_ptr->designated_priority, &pom_ptr->port_priority, sizeof(XSTP_TYPE_PriorityVector_T));
        /* designated_times */
        memcpy(&pom_ptr->designated_times, &pom_ptr->port_times, sizeof(XSTP_TYPE_Timers_T));
        /* msg_priority */
        memcpy(&pom_ptr->msg_priority, &pom_ptr->port_priority, sizeof(XSTP_TYPE_PriorityVector_T));
        /* msg_times */
        memcpy(&pom_ptr->msg_times, &pom_ptr->port_times, sizeof(XSTP_TYPE_Timers_T));
        if (pom_ptr->role != XSTP_ENGINE_PORTVAR_ROLE_DISABLED)
        {
            pom_ptr->selected_role = XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED;
            #ifdef  XSTP_TYPE_PROTOCOL_RSTP
            pom_ptr->new_info   = TRUE;
            #endif  /* XSTP_TYPE_PROTOCOL_RSTP */
            #ifdef  XSTP_TYPE_PROTOCOL_MSTP
            if (om_ptr->instance_id == XSTP_TYPE_CISTID)
                pom_ptr->common->new_info_cist     = TRUE;
            else
                pom_ptr->common->new_info_msti     = TRUE;
            #endif  /* XSTP_TYPE_PROTOCOL_MSTP */
        }
    }
 }/* End of XSTP_UTY_SetRootBridge */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetDesignatedPort
 * ------------------------------------------------------------------------
 * PURPOSE  :   This port will be selected as designated port for this instance.
 * INPUT    :   om_ptr           -- the pointer of the instance entry.
 *              lport            -- specified lport
 * OUTPUT   :   None.
 * RETURN   :   None.
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
 static void    XSTP_UTY_SetDesignatedPort(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
 {
    XSTP_OM_PortVar_T               *pom_ptr;

    pom_ptr = &(om_ptr->port_info[lport-1]);
    /* port_priority */
    memcpy(&pom_ptr->port_priority, &pom_ptr->designated_priority, sizeof(XSTP_TYPE_PriorityVector_T));
    /* port_times */
    memcpy(&pom_ptr->port_times, &pom_ptr->designated_times, sizeof(XSTP_TYPE_Timers_T));

    pom_ptr->selected_role = XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED;
    #ifdef  XSTP_TYPE_PROTOCOL_RSTP
    pom_ptr->new_info   = TRUE;
    #endif  /* XSTP_TYPE_PROTOCOL_RSTP */
    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    if (om_ptr->instance_id == XSTP_TYPE_CISTID)
        pom_ptr->common->new_info_cist     = TRUE;
    else
        pom_ptr->common->new_info_msti     = TRUE;
    #endif  /* XSTP_TYPE_PROTOCOL_MSTP */
 }/* End of XSTP_UTY_SetDesignatedPort */
#endif

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetBpduGuardRecoverPortList
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the port into a port list for BPDU guard auto recovery.
 * INPUT    :   lport -- lport number
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
void XSTP_UTY_SetBpduGuardRecoverPortList(UI32_T lport)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    XSTP_UTY_BpduGuardPortList_T *new_element;

    /* BODY
     */

    new_element  = (XSTP_UTY_BpduGuardPortList_T*)L_MM_Malloc(
                    sizeof(XSTP_UTY_BpduGuardPortList_T),
                    L_MM_USER_ID2(SYS_MODULE_XSTP, XSTP_TYPE_TRACE_ID_XSTP_UTY_SETBPDUGUARDRECOVERPORTLIST));
    if (new_element != NULL)
    {
        new_element->lport                  = lport;
        new_element->next                   = XSTP_UTY_BpduGuardRecoverPortList;
        XSTP_UTY_BpduGuardRecoverPortList   = new_element;
    }

    return;
} /* End of XSTP_UTY_SetBpduGuardRecoverPortList */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_GetBpduGuardRecoverPortList
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port list for BPDU guard auto recovery.
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   XSTP_UTY_BpduGuardPortList_T*
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
XSTP_UTY_BpduGuardPortList_T *XSTP_UTY_GetBpduGuardRecoverPortList(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    XSTP_UTY_BpduGuardPortList_T *port_list;

    /* BODY
     */

    port_list = XSTP_UTY_BpduGuardRecoverPortList;
    XSTP_UTY_BpduGuardRecoverPortList = NULL;
    return port_list;
} /* End of XSTP_UTY_GetBpduGuardRecoverPortList */
#endif /* #if (SYS_CPNT_STP_BPDU_GUARD == TRUE) */

#if (SYS_CPNT_DEBUG == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_DebugPringBPDU
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function print the BPDU content
 * INPUT    :   lport   -- the port send or receive the BPDU
 *              src_addr-- the source mac address
 *              is_tagged -- the BPDU is tagged BPDU or not
 *              pkt_length--the packet length
 *              pdu     -- the PDU content
 *              is_tx   -- identify print send BPDU or recevied BPDU
 * OUTPUT   :   None.
 * RETURN   :   None.
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
void XSTP_UTY_DebugPringBPDU(UI32_T lport, UI8_T *src_addr, BOOL_T is_tagged, UI8_T pkt_length, UI8_T *pdu, BOOL_T is_tx)
{
    UI32_T unit=0, port=0, trunk_id=0;
    UI16_T idx, stp_ver=0;
    int year, month, day, hour, minute, second;

    memset(xstp_uty_str_tmp, 0, sizeof(xstp_uty_str_tmp));
    memset(xstp_uty_str_cat, 0, sizeof(xstp_uty_str_cat));

    sprintf(xstp_uty_str_tmp, "%s", "\r\n");
    strcat(xstp_uty_str_cat, xstp_uty_str_tmp);

    /*print time*/
    SYS_TIME_GetRealTimeClock(&year, &month, &day, &hour, &minute, &second);
    sprintf(xstp_uty_str_tmp, "%02d:%02d:%02d, ", hour, minute, second);
    strcat(xstp_uty_str_cat, xstp_uty_str_tmp);

    /*print verison*/
    stp_ver = XSTP_OM_GetForceVersion();

    if ((stp_ver == XSTP_TYPE_STP_MODE) ||(stp_ver == XSTP_TYPE_RSTP_MODE))
    {
        sprintf(xstp_uty_str_tmp, "%s",  "STP/RSTP: ");
        strcat(xstp_uty_str_cat, xstp_uty_str_tmp);
    }
#ifdef  XSTP_TYPE_PROTOCOL_MSTP
    else if (stp_ver == XSTP_TYPE_MSTP_MODE)
    {
        sprintf(xstp_uty_str_tmp, "%s",  "MSTP: ");
        strcat(xstp_uty_str_cat, xstp_uty_str_tmp);
    }
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

    if (SWCTRL_LPORT_TRUNK_PORT == SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        sprintf(xstp_uty_str_tmp, "%s BPDU from Port-Channel %ld, %s \r\n",(is_tx?"tx":"rx"), (long)trunk_id, (is_tagged?"dropped":""));
        strcat(xstp_uty_str_cat, xstp_uty_str_tmp);
    }
    else
    {
        sprintf(xstp_uty_str_tmp, "%s BPDU from Ethernet %ld/%ld, %s \r\n", (is_tx?"tx":"rx"), (long)unit, (long)port, (is_tagged?"dropped":""));
        strcat(xstp_uty_str_cat, xstp_uty_str_tmp);
    }

    if ((stp_ver == XSTP_TYPE_STP_MODE) ||(stp_ver == XSTP_TYPE_RSTP_MODE))
    {
        sprintf(xstp_uty_str_tmp, "%s", "STP/RSTP: ");
        strcat(xstp_uty_str_cat, xstp_uty_str_tmp);
    }
#ifdef  XSTP_TYPE_PROTOCOL_MSTP
    else if (stp_ver == XSTP_TYPE_MSTP_MODE)
    {
        sprintf(xstp_uty_str_tmp, "%s", "MSTP: ");
        strcat(xstp_uty_str_cat, xstp_uty_str_tmp);
    }
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

    /*print packet*/
    sprintf(xstp_uty_str_tmp, "0180C2000000%02x%02x%02x%02x%02x%02x",src_addr[0], src_addr[1], src_addr[2], src_addr[3], src_addr[4], src_addr[5]);
    strcat(xstp_uty_str_cat, xstp_uty_str_tmp);

    for (idx = 0; idx < pkt_length; idx++)
    {
        sprintf(xstp_uty_str_tmp, "%02x", pdu[idx]);
        strcat(xstp_uty_str_cat, xstp_uty_str_tmp);
    }

    sprintf(xstp_uty_str_tmp, "%s", "\r\n");
    strcat(xstp_uty_str_cat, xstp_uty_str_tmp);

    DEBUG_MGR_Printf(DEBUG_TYPE_XSTP, DEBUG_TYPT_MATCH_ALL, DEBUG_TYPE_XSTP_BPDU, 0, "%s", xstp_uty_str_cat);

    return;
} /*End of XSTP_UTY_DebugPringBPDU*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_DebugPrintRoot
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function print the root change or this now is root.
 * INPUT    :   *om_ptr     -- the port information pointer
 *              is_root_bridge-- identify this switch is root or not
 * OUTPUT   :   None.
 * RETURN   :   None.
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
void XSTP_UTY_DebugPrintRoot(XSTP_OM_InstanceData_T *om_ptr , BOOL_T is_root_bridge)
{
    UI32_T unit = 0, port = 0, trunk_id = 0;
    UI32_T root_lport = (UI32_T)(om_ptr->bridge_info.root_port_id.port_id & 0x0FFF);
    UI32_T instance_id = om_ptr->instance_id;
    UI32_T bridge_priority = 0;
    UI16_T stp_ver = 0;
    UI8_T *bridge_id;
    int year, month, day, hour, minute, second;

    memset(xstp_uty_str_tmp, 0, sizeof(xstp_uty_str_tmp));
    memset(xstp_uty_str_cat, 0, sizeof(xstp_uty_str_cat));

    if (root_lport == 0)
    {
        is_root_bridge = TRUE;
    }

    sprintf(xstp_uty_str_tmp, "%s", "\r\n");
    strcat(xstp_uty_str_cat, xstp_uty_str_tmp);

    /*print time*/
    SYS_TIME_GetRealTimeClock(&year, &month, &day, &hour, &minute, &second);
    sprintf(xstp_uty_str_tmp, "%02d:%02d:%02d, ", hour, minute, second);
    strcat(xstp_uty_str_cat, xstp_uty_str_tmp);

    /*print verison*/
    stp_ver = XSTP_OM_GetForceVersion();

    if ((stp_ver == XSTP_TYPE_STP_MODE) ||(stp_ver == XSTP_TYPE_RSTP_MODE))
    {
        sprintf(xstp_uty_str_tmp, "%s", "STP/RSTP: ");
        strcat(xstp_uty_str_cat, xstp_uty_str_tmp);
    }
#ifdef  XSTP_TYPE_PROTOCOL_MSTP
    else if (stp_ver == XSTP_TYPE_MSTP_MODE)
    {
        sprintf(xstp_uty_str_tmp, "%s", "MSTP: ");
        strcat(xstp_uty_str_cat, xstp_uty_str_tmp);
    }
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

    if (is_root_bridge)
    {
        if ((stp_ver == XSTP_TYPE_STP_MODE) || (stp_ver == XSTP_TYPE_RSTP_MODE))
        {
            sprintf(xstp_uty_str_tmp, "%s", "This switch is the spanning tree root bridge\r\n");
            strcat(xstp_uty_str_cat, xstp_uty_str_tmp);
        }
#ifdef  XSTP_TYPE_PROTOCOL_MSTP
        else if (stp_ver == XSTP_TYPE_MSTP_MODE)
        {
            sprintf(xstp_uty_str_tmp, "This switch is the spanning tree root bridge at instance %ld\r\n", (long)instance_id);
            strcat(xstp_uty_str_cat, xstp_uty_str_tmp);
        }
#endif /* XSTP_TYPE_PROTOCOL_MSTP */
    }
    else
    {
        if (XSTP_UTY_Cist(om_ptr))
        {
#ifdef  XSTP_TYPE_PROTOCOL_MSTP
            bridge_priority=om_ptr->bridge_info.root_priority.root_id.bridge_id_priority.bridge_priority;
            bridge_id=om_ptr->bridge_info.root_priority.root_id.addr;
#else
            bridge_priority=om_ptr->bridge_info.root_priority.root_bridge_id.bridge_id_priority.bridge_priority;
            bridge_id=om_ptr->bridge_info.root_priority.root_bridge_id.addr;
#endif
        }
        else
        {
#ifdef  XSTP_TYPE_PROTOCOL_MSTP
            bridge_priority=om_ptr->bridge_info.root_priority.r_root_id.bridge_id_priority.bridge_priority;
            bridge_id=om_ptr->bridge_info.root_priority.r_root_id.addr;
#endif
        }

        sprintf(xstp_uty_str_tmp, "recevied root %ld.%02x%02x%02x%02x%02x%02x ", (long)bridge_priority, bridge_id[0],bridge_id[1], bridge_id[2], bridge_id[3], bridge_id[4], bridge_id[5]);
        strcat(xstp_uty_str_cat, xstp_uty_str_tmp);

#ifdef  XSTP_TYPE_PROTOCOL_MSTP
        if (stp_ver == XSTP_TYPE_MSTP_MODE)
        {
            sprintf(xstp_uty_str_tmp, "at instance %ld ", (long)instance_id);
            strcat(xstp_uty_str_cat, xstp_uty_str_tmp);
        }
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

        if (SWCTRL_LPORT_TRUNK_PORT == SWCTRL_LogicalPortToUserPort(root_lport, &unit, &port, &trunk_id))
        {
            sprintf(xstp_uty_str_tmp, "on Port-Channel %ld\r\n", (long)trunk_id);
            strcat(xstp_uty_str_cat, xstp_uty_str_tmp);
        }
        else
        {
            sprintf(xstp_uty_str_tmp, "on Ethernet %ld/%ld\r\n", (long)unit, (long)port);
            strcat(xstp_uty_str_cat, xstp_uty_str_tmp);
        }
    }

    DEBUG_MGR_Printf(DEBUG_TYPE_XSTP, DEBUG_TYPT_MATCH_ALL, DEBUG_TYPE_XSTP_ROOT, 0, "%s", xstp_uty_str_cat);

    return;
}/*End of XSTP_UTY_DebugPrintRoot*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_DebugPrintEvents
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function print the spanning tree event.
 * INPUT    :   lport   -- the port send or receive the BPDU
 * OUTPUT   :   None.
 * RETURN   :   None.
 * NOTES    :   now only print the TCN
 * ------------------------------------------------------------------------
 */
void XSTP_UTY_DebugPrintEvents(UI32_T lport)
{
    UI32_T unit=0, port=0, trunk_id=0;
    UI16_T stp_ver=0;
    int year, month, day, hour, minute, second;

    memset(xstp_uty_str_tmp, 0, sizeof(xstp_uty_str_tmp));
    memset(xstp_uty_str_cat, 0, sizeof(xstp_uty_str_cat));

    sprintf(xstp_uty_str_tmp, "%s", "\r\n");

    /*print time*/
    SYS_TIME_GetRealTimeClock(&year, &month, &day, &hour, &minute, &second);
    sprintf(xstp_uty_str_tmp, "%02d:%02d:%02d, ", hour, minute, second);
    strcat(xstp_uty_str_cat, xstp_uty_str_tmp);

    /*print verison*/
    stp_ver=XSTP_OM_GetForceVersion();

    if ( (stp_ver == XSTP_TYPE_STP_MODE) ||(stp_ver == XSTP_TYPE_RSTP_MODE))
    {
        sprintf(xstp_uty_str_tmp, "%s", "STP/RSTP: ");
        strcat(xstp_uty_str_cat, xstp_uty_str_tmp);
    }
#ifdef  XSTP_TYPE_PROTOCOL_MSTP
    else if(stp_ver == XSTP_TYPE_MSTP_MODE)
    {
        sprintf(xstp_uty_str_tmp, "%s", "MSTP: ");
        strcat(xstp_uty_str_cat, xstp_uty_str_tmp);
    }
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

    if(SWCTRL_LPORT_TRUNK_PORT == SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
    {
        sprintf(xstp_uty_str_tmp, "Sent Topology Change Notice on Port-Channel %ld\r\n", (long)trunk_id);
        strcat(xstp_uty_str_cat, xstp_uty_str_tmp);
    }
    else
    {
        sprintf(xstp_uty_str_tmp, "Sent Topology Change Notice on Ethernet %ld/%ld\r\n", (long)unit, (long)port);
        strcat(xstp_uty_str_cat, xstp_uty_str_tmp);
    }

    DEBUG_MGR_Printf(DEBUG_TYPE_XSTP, DEBUG_TYPT_MATCH_ALL, DEBUG_TYPE_XSTP_EVENTS, 0, "%s", xstp_uty_str_cat);

    return;
} /*End of XSTP_UTY_DebugPrintEvents*/
#endif
