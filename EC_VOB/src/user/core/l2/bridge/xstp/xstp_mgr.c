/*-----------------------------------------------------------------------------
 * Module Name: xstp_mgr.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the XSTP MGR
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    06/12/2002 - kelly Chen, Created
 *    06/12/2002 - Allen Cheng, Added
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "l_hold_timer.h"
#include "l_mm.h"
#include "l_rstatus.h"
#include "l_bitmap.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#include "eh_mgr.h"
#include "eh_type.h"
#include "nmtr_pmgr.h"
#include "snmp_pmgr.h"
#include "swctrl.h"
#include "swctrl_pmgr.h"
#include "swdrv_type.h"
#include "sys_callback_mgr.h"
#include "sys_time.h"
#include "syslog_pmgr.h"
#include "syslog_type.h"
#include "trap_event.h"
#include "vlan_lib.h"
#include "vlan_om.h"
#include "vlan_pmgr.h"
#include "xstp_backdoor.h"
#include "xstp_engine.h"
#include "xstp_mgr.h"
#include "xstp_om.h"
#include "xstp_om_private.h"
#include "xstp_rx.h"
#include "xstp_type.h"
#include "xstp_uty.h"

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
#include "sysctrl_xor_mgr.h"
#endif

#if (SYS_CPNT_DEBUG == TRUE)
#include "debug_type.h"
#endif


/* NAMING CONSTANT DECLARATIONS
 */
#ifndef SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR
#define SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR     TRUE
#endif


/* MACRO FUNCTIONS DECLARACTION
 */
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
    #define XSTP_MGR_XOR_LOCK_AND_FAIL_RETURN(ret_val, expr) \
            do {                                        \
                SYSCTRL_XOR_MGR_GetSemaphore();         \
                if (!(expr))                            \
                {                                       \
                    SYSCTRL_XOR_MGR_ReleaseSemaphore(); \
                    return ret_val;                     \
                }                                       \
            } while (0)

    #define XSTP_MGR_XOR_UNLOCK_AND_RETURN(ret_val)     \
            do {                                        \
                SYSCTRL_XOR_MGR_ReleaseSemaphore();     \
                return ret_val;                         \
            } while (0)

#else
    #define XSTP_MGR_XOR_LOCK_AND_FAIL_RETURN(ret_val, expr)    \
            do {} while (0)

    #define XSTP_MGR_XOR_UNLOCK_AND_RETURN(ret_val)             \
            do {return ret_val;} while (0)

#endif /* #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */

static UI32_T XSTP_MGR_SetMstForwardDelay(XSTP_OM_InstanceData_T *om_ptr, UI32_T forward_delay);
static UI32_T XSTP_MGR_SetMstHelloTime(XSTP_OM_InstanceData_T *om_ptr, UI32_T hello_time);
static UI32_T XSTP_MGR_SetMstMaxAge(XSTP_OM_InstanceData_T *om_ptr, UI32_T max_age);
static UI32_T XSTP_MGR_SetMstTransmissionLimit(XSTP_OM_InstanceData_T *om_ptr, UI32_T tx_hold_count);
static BOOL_T XSTP_MGR_IsRootBridge (XSTP_OM_InstanceData_T *om_ptr);
static BOOL_T XSTP_MGR_PortStatusIsFullDuplex(UI32_T lport);
static UI32_T XSTP_MGR_SetMstpConfigurationName_(XSTP_OM_InstanceData_T *om_ptr, char *config_name);
static UI32_T XSTP_MGR_SetMstPriority_(XSTP_OM_InstanceData_T *om_ptr, UI32_T priority, BOOL_T is_static_config);
static UI32_T XSTP_MGR_SetMstPortPriority_(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, UI32_T priority);
static UI32_T XSTP_MGR_SetMstPortPathCost_(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, UI32_T path_cost);
static BOOL_T XSTP_MGR_GetDot1dMstEntry_(XSTP_OM_InstanceData_T *om_ptr, XSTP_MGR_Dot1dStpEntry_T  *entry);
static UI32_T XSTP_MGR_SetMstpMaxHop_(UI32_T hop_count);
static UI32_T XSTP_MGR_SetVlanToMstConfigTable_(UI32_T mstid, UI32_T vlan);
static UI32_T XSTP_MGR_AttachVlanToMstConfigTable_(UI32_T mstid, UI32_T vlan);
static UI32_T XSTP_MGR_SetVlanToMstMappingTable(UI32_T vlan_id);
static UI32_T XSTP_MGR_RemoveVlanFromMstMappingTable(UI32_T vlan_id);
static void XSTP_MGR_CreateSpanningTree(XSTP_OM_InstanceData_T *om_ptr);
static void XSTP_MGR_DeleteSpanningTree(XSTP_OM_InstanceData_T *om_ptr);
static void XSTP_MGR_InitPortPriorityVector(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static UI32_T XSTP_MGR_SetPortAutoPathCost_(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
#if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE)
static void XSTP_MGR_RecordAttribute(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static void XSTP_MGR_RestoreAttribute(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
#endif /* #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE) */
static void XSTP_MGR_SetRootBridge(XSTP_OM_InstanceData_T *om_ptr);
static void XSTP_MGR_ReleaseAllPortsMref();
#ifdef  XSTP_TYPE_PROTOCOL_MSTP
static void XSTP_MGR_RecoverExistingMSTI(void);
static void XSTP_MGR_SetPortPathCost_(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, UI32_T path_cost);
static UI32_T XSTP_MGR_SetMstPortAutoPathCost_(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static BOOL_T XSTP_MGR_SemanticCheck(void *om_ptr);
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

#if 0
#if ((SYS_CPNT_SWDRV_CACHE == TRUE) && (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP))
static void    XSTP_MGR_VlanCreatedForCache(UI32_T vid_ifidx);
static void    XSTP_MGR_VlanDestroyForCache(UI32_T vid_ifidx);
static void    XSTP_MGR_VlanMemberAddForCache(UI32_T vid_ifidx, UI32_T lport_ifidx);
static void    XSTP_MGR_VlanMemberDeleteForCache(UI32_T vid_ifidx, UI32_T lport_ifidx);
static UI32_T  XSTP_MGR_VlanMsgqId;
#endif
#endif

/* For hot-swappable optional module */
static void XSTP_MGR_ResetPortDatabase(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T set_default);
static void XSTP_MGR_InitHotModulePort(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, BOOL_T set_default);

#if (SYS_CPNT_SUPPORT_XSTP_BACKUP_ROOT == TRUE)
static BOOL_T  XSTP_MGR_HasRootBridgeDisappeared(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);
static BOOL_T  XSTP_MGR_BecomeNewRootBridge(XSTP_OM_InstanceData_T *om_ptr);
#endif /* SYS_CPNT_SUPPORT_XSTP_BACKUP_ROOT == TRUE */
static void XSTP_MGR_NotifyStpChangeVersion(UI32_T mode, UI32_T status);

/*=============================================================================
 * Temporarily placed here, should be removed if all questionable MGR Get
 * functions are moved
 *=============================================================================
 */
static BOOL_T XSTP_MGR_GetNextPortMemberOfInstance(XSTP_OM_InstanceData_T *om_ptr, UI32_T *lport);
static UI32_T XSTP_MGR_GetMstPortState_(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, UI32_T *state);
static BOOL_T XSTP_MGR_GetNextExistingInstance_(UI32_T *mstid);


/* STATIC VARIABLE DECLARATIONS
 */

static  XSTP_TYPE_LportList_T *XSTP_MGR_ChangeStatePortList             = NULL;

static  BOOL_T flooding_bpdu_flag = FALSE;

#if (SYS_CPNT_SUPPORT_XSTP_BACKUP_ROOT == TRUE)
static  BOOL_T backup_root_functionality = FALSE;
#endif /* SYS_CPNT_SUPPORT_XSTP_BACKUP_ROOT == TRUE */

/*  declare variables used for Transition mode  */
SYSFUN_DECLARE_CSC

#if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE)
typedef struct
{
    #ifdef  XSTP_TYPE_PROTOCOL_RSTP
    BOOL_T static_path_cost;
    UI32_T port_path_cost;
    #endif /* XSTP_TYPE_PROTOCOL_RSTP */

    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    BOOL_T  static_external_path_cost;
    BOOL_T  static_internal_path_cost;
    UI32_T  external_port_path_cost;
    UI32_T  internal_port_path_cost;
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */
} XSTP_MGR_BackupAttribute_T;

static XSTP_MGR_BackupAttribute_T     backup_values;
#endif /* #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE) */

/*=============================================================================
 * Moved from xstp_task.c
 *=============================================================================
 */
static L_HOLD_TIMER_Entry_T XSTP_TASK_HdTimerEntry;

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
static BOOL_T bpdu_detected[XSTP_TYPE_MAX_NUM_OF_LPORT];
#endif


/* EXPORTED SUBPROGRAM BODIES
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Create the CST and set XSTP op_state to "SYS_TYPE_STACKING_MASTER_MODE"
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void XSTP_MGR_EnterMasterMode(void)
{
    UI32_T  lport;
    XSTP_OM_InstanceData_T          *om_ptr;
    UI32_T                          current_st_status;

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);

    XSTP_OM_SetDefaultValue();
    lport   = 0;
    om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);

    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        /*SWCTRL_SetPortSTAState(0, lport, XSTP_TYPE_PORT_STATE_BLOCKING);*/
        XSTP_ENGINE_SetPortState(om_ptr, lport, FALSE, FALSE);
    } /* End of while (SWCTRL_GetNextLogicalPort) */

    current_st_status = XSTP_OM_GetSpanningTreeStatus();

    if (    (XSTP_TYPE_PROTOCOL_VERSION_ID == XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID)
        &&  (XSTP_TYPE_DEFAULT_STP_VERSION == XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID)
        &&  (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
       )
    {
        /* MST mode */
        #ifdef  XSTP_TYPE_PROTOCOL_MSTP
        if (!SWCTRL_PMGR_SetMstEnableStatus(SWCTRL_MST_ENABLE) )
        {
            printf("\r\nXSTP_MGR_EnterMasterMode: Fail in setting the MST mode!!");
        }
        #endif /* XSTP_TYPE_PROTOCOL_MSTP */
        XSTP_ENGINE_SetMstEnableStatus(TRUE);
    }
    else
    {
        XSTP_ENGINE_SetMstEnableStatus(FALSE);
    }

    /* Create the common spanning tree */
    XSTP_MGR_CreateSpanningTree(om_ptr);
    if (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_DISABLED)
    {
        XSTP_MGR_DeleteSpanningTree(om_ptr);
    }
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    /* set mgr in master mode */
    SYSFUN_ENTER_MASTER_MODE();

    lport   = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        XSTP_MGR_PortAdminEnable_CallBack(lport);
    }

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    for (lport = 0; lport < XSTP_TYPE_MAX_NUM_OF_LPORT; lport++)
    {
        bpdu_detected[lport] = FALSE;
    }
#endif

    return;
} /* End of XSTP_MGR_EnterMasterMode() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set XSTP op_state to "SYS_TYPE_STACKING_SLAVE_MODE"
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void XSTP_MGR_EnterSlaveMode(void)
{
    /* set mgr in slave mode */
    SYSFUN_ENTER_SLAVE_MODE();

    return;
} /* End of XSTP_MGR_EnterSlaveMode() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : This function set the transition mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void XSTP_MGR_SetTransitionMode(void)
{
    /* set transition flag to prevent calling request */
    SYSFUN_SET_TRANSITION_MODE();

    return;
}  /* End of L2MCAST_MGR_SetTransitionMode */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set XSTP op_state to "SYS_TYPE_STACKING_TRANSITION_MODE"
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void XSTP_MGR_EnterTransitionMode(void)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    UI32_T                  xstid;
    UI32_T                  lport;

    /* set mgr in transition mode */
    SYSFUN_ENTER_TRANSITION_MODE();

    xstid = XSTP_TYPE_CISTID;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    XSTP_OM_CleanDatabase();
    /* Need to delete all the spanning trees here */
    do
    {
        XSTP_OM_NullifyInstance(om_ptr);
        for (lport = 1; lport <= XSTP_TYPE_MAX_NUM_OF_LPORT; lport++)
        {
            XSTP_OM_NullifyPortOmEngineState(om_ptr, lport);
        }
    }while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &om_ptr));

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    /* Trap */
    SNMP_PMGR_NotifyStaTplgChanged();
    SYSLOG_PMGR_NotifyStaTplgChanged();

    return;
}/* End of XSTP_MGR_EnterTransitionMode() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get current dhcp operation mode (master / slave / transition).
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
}/* End of XSTP_MGR_GetOperationMode    */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_IsMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Check if the OM initializing is finished or not.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE if RSTP op_state is SYS_TYPE_STACKING_MASTER_MODE, else FALSE
 * Note     : None
 *-------------------------------------------------------------------------
 */
BOOL_T XSTP_MGR_IsMasterMode(void)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}/* End of XSTP_MGR_IsMasterMode() */



/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_ProcessTimerEvent
 *-------------------------------------------------------------------------
 * PURPOSE  : Process the timer event
 * INPUT    : xstid     -- spanning tree instance id
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_ProcessTimerEvent(void)
{
    UI32_T                  xstid;
    TRAP_EVENT_TrapData_T   trap_data;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    if (XSTP_OM_GetSpanningTreeStatus() == XSTP_TYPE_SYSTEM_ADMIN_STATE_DISABLED)
    {
        return;
    }
    xstid = XSTP_TYPE_CISTID;
    do
    {
        XSTP_MGR_GenerateOneSecondTickSignal(xstid);

        if ( XSTP_MGR_PTIStateMachineProgress(xstid) )
        {
            XSTP_MGR_StateMachineProgress(xstid);

            /* Process Trap */
            if (XSTP_OM_GetTrapFlagNewRoot() )
            {
                memset(&trap_data, 0, sizeof(trap_data));
                if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
                    BACKDOOR_MGR_Printf("\r\n XSTP_MGR_ProcessTimerEvent: request new root trap");

                /* Trap for new root */
                trap_data.trap_type = TRAP_EVENT_NEW_ROOT;
                trap_data.community_specified = FALSE;

                /* send trap to all community */
                SNMP_PMGR_ReqSendTrap(&trap_data);
                XSTP_OM_SetTrapFlagNewRoot(FALSE);
            }

            if (XSTP_OM_GetTrapFlagTc() )
            {
                memset(&trap_data, 0, sizeof(trap_data));
                if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
                    BACKDOOR_MGR_Printf("\n XSTP_MGR_ProcessTimerEvent: request TCN trap");

                /* Trap for topology change */
                trap_data.trap_type = TRAP_EVENT_TOPOLOGY_CHANGE;
                trap_data.community_specified = FALSE;
                trap_data.u.tc_cause_lport = XSTP_OM_GetTcCausePort();

                /* send trap to all community */
                SNMP_PMGR_ReqSendTrap(&trap_data);
                XSTP_OM_SetTrapFlagTc(FALSE);
            }
            if (XSTP_OM_GetTrapRxFlagTc() )
            {
                memset(&trap_data, 0, sizeof(trap_data));
                if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
                    BACKDOOR_MGR_Printf("\n XSTP_MGR_ProcessTimerEvent: request RX TC trap");
                /* Trap for topology change */
                trap_data.trap_type =TRAP_EVENT_TOPOLOGY_CHANGE_RECEIVE;
                trap_data.community_specified = FALSE;
                XSTP_OM_GetTcCausePortAndBrdgMac(&trap_data.u.tc.rx_lport, trap_data.u.tc.brg_mac);
                /* send trap to all community */
                SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_ONLY);
                XSTP_OM_SetTrapRxFlagTc(FALSE);
            }

            /* process auto recovery */
            if (xstid == XSTP_TYPE_CISTID)
            {
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
                XSTP_UTY_BpduGuardPortList_T *bg_list = NULL;
                XSTP_UTY_BpduGuardPortList_T *bg_current = NULL;
#endif
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
                bg_list = XSTP_UTY_GetBpduGuardRecoverPortList();
                while (bg_list != NULL)
                {
                    bg_current = bg_list;
                    SYS_CALLBACK_MGR_SetPortStatusCallback(SYS_MODULE_XSTP,
                        bg_current->lport, TRUE, SWCTRL_PORT_STATUS_SET_BY_XSTP_BPDUGUARD);
                    bg_list = bg_list->next;
                    L_MM_Free((void*)bg_current);
                }
#endif /* #if (SYS_CPNT_STP_BPDU_GUARD == TRUE) */
            }
        }
    } while(XSTP_OM_GetNextExistedInstance(&xstid));

    return;
} /* End of XSTP_MGR_ProcessTimerEvent */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_ProcessRcvdBpdu
 *-------------------------------------------------------------------------
 * PURPOSE  : Process the received BPDU
 * INPUT    : bpdu_msg_ptr  -- BPDU message pointer
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_ProcessRcvdBpdu(XSTP_TYPE_MSG_T *bpdu_msg_ptr)
{
    XSTP_TYPE_Bpdu_T            *pdu;
    XSTP_OM_InstanceData_T      *om_ptr;
    L_MM_Mref_Handle_T          *mref_handle_p;
    UI32_T                      lport, index, mstid, stp_status;
    BOOL_T                      valid_bpdu;
    UI32_T                      port_st_status, pdu_len;
    XSTP_OM_PortVar_T           *pom_ptr;
    UI16_T                      vlan_id;
    VLAN_OM_Vlan_Port_Info_T    port_info;
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;
    UI32_T time_mark = 0;
    BOOL_T                      is_tagged   = FALSE;
    /* release_mref is TRUE means MREF should be released. When any instance's delay_flag == TRUE,
     * system will process BPDU later, so MREF can't be free now. Hawk, 2006.4*/
    BOOL_T                      release_mref = TRUE;
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    BOOL_T                      bpdu_guard_status;
    TRAP_EVENT_TrapData_T       trap_data;
#endif

    vlan_id = 0x0FFF & (bpdu_msg_ptr->msg_type);
    mref_handle_p = bpdu_msg_ptr->mref_handle_p;
    lport   = (UI32_T)bpdu_msg_ptr->lport;

    /* Check tagged or untagged BPDU*/
    if (    (vlan_id != 0)
         && VLAN_PMGR_GetPortEntry(lport, &port_info)
         && (port_info.port_item.dot1q_pvid_index != vlan_id)
       )
    {
    /*EPR: ES3628BT-FLF-ZZ-00676
Problem: System:set the vlan 5 suspended cause trunk with native vlan 5 cannot receive stp pdu.
Rootcause:when suspend a vlan ,it will remove all the ports in the vlan ,and delete the vlan in chip
          if the port native vlan is it.the port will make it native vlan to defalut vlan in chip
          spanning-tree receive the pkt with default vlan,but it is not equal to the ports native vlan,spanning-tree will not process the pkt
Solution:spaning-tree will check if the native vlan of the port is suspend,if it is suspend ,stp will do it as untag pkt
Files:xstp_mgr.c*/
        if(!VLAN_OM_GetDot1qVlanCurrentEntry(time_mark, port_info.port_item.dot1q_pvid_index, &vlan_info))
         is_tagged   = FALSE;
        else
        {
          if(vlan_info.dot1q_vlan_static_row_status != VAL_dot1qVlanStaticRowStatus_notReady )
           is_tagged   = TRUE;
          else
           is_tagged   = FALSE;
        }
    }
    else
    {
        is_tagged   = FALSE;
    }

    pdu = (XSTP_TYPE_Bpdu_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if (pdu == NULL)
        return;

#if (SYS_CPNT_DEBUG == TRUE)
        XSTP_UTY_DebugPringBPDU(lport, bpdu_msg_ptr->saddr, is_tagged,
            bpdu_msg_ptr->pkt_length, (UI8_T*)pdu, FALSE);
#endif

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);

    port_st_status = pom_ptr->common->port_spanning_tree_status;
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    bpdu_guard_status = pom_ptr->common->bpdu_guard_status;
#endif
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    if ((pom_ptr->is_member == FALSE) || (pom_ptr->common->link_up == FALSE))
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    XSTP_OM_GetRunningSystemSpanningTreeStatus(&stp_status);
    if (    (stp_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
         && (port_st_status == VAL_staPortSystemStatus_enabled)
         && (!is_tagged)
       )
    {
        /* +++ EnterCriticalRegion +++ */
        XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
        om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
        valid_bpdu  = XSTP_RX_ValidateReceivedBpdu(om_ptr, lport, pdu, (UI32_T)bpdu_msg_ptr->pkt_length);
        /* +++ LeaveCriticalRegion +++ */
        XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

        if (valid_bpdu)
        {
            switch (pdu->bpdu_header.bpdu_type)
            {
                case XSTP_TYPE_BPDU_TYPE_CONFIG:
                case XSTP_TYPE_BPDU_TYPE_TCN:
                case XSTP_TYPE_BPDU_TYPE_XST:
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
                    if (bpdu_guard_status == TRUE)
                    {
                        if (bpdu_detected[lport-1] == FALSE)
                        {
                            trap_data.trap_type = TRAP_EVENT_STP_BPDU_GUARD_PORT_SHUTDOWN_TRAP;
                            trap_data.u.stp_bpdu_guard_port_shutdown.ifindex = lport;
                            trap_data.community_specified = FALSE;
                            trap_data.flag = TRAP_EVENT_SEND_TRAP_OPTION_DEFAULT;
                            SNMP_PMGR_ReqSendTrap(&trap_data);

                            SYS_CALLBACK_MGR_SetPortStatusCallback(SYS_MODULE_XSTP,
                                lport, FALSE, SWCTRL_PORT_STATUS_SET_BY_XSTP_BPDUGUARD);

                            bpdu_detected[lport-1] = TRUE;
                        }

                        /* +++ EnterCriticalRegion +++ */
                        XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
                        om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
                        pom_ptr = &(om_ptr->port_info[lport-1]);
                        if (pom_ptr->common->bpdu_guard_auto_recovery == XSTP_TYPE_PORT_BPDU_GUARD_AUTO_RECOVERY_ENABLED)
                        {
                            pom_ptr->common->bpdu_guard_auto_recovery_while = pom_ptr->common->bpdu_guard_auto_recovery_interval;
                        }
                        /* +++ LeaveCriticalRegion +++ */
                        XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

                        L_MM_Mref_Release(&mref_handle_p);
                        return;
                    }
#endif /* #if (SYS_CPNT_STP_BPDU_GUARD == TRUE) */

                    /* 9.3.4 */
                    /* +++ EnterCriticalRegion +++ */
                    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);

                    om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
                    XSTP_RX_ProcessBpdu(om_ptr, lport, &mref_handle_p);
                    om_ptr->delay_flag     = TRUE;
                    release_mref = FALSE;
                    /* +++ LeaveCriticalRegion +++ */
                    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

                    /* if mref_handle_p is released in XSTP_RX_ProcessBpdu, it will become NULL
                     */
                    if ( (mref_handle_p != NULL) &&
                         (XSTP_TYPE_PROTOCOL_VERSION_ID == XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID) &&
                         (pdu->bpdu_header.protocol_version_identifier == XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID)
                       )
                    {
                        index   = 0;
                        pdu = (XSTP_TYPE_Bpdu_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
                        if(pdu==NULL)
                            return;

                        while ( (mstid = XSTP_RX_GetMstidFromBpdu( index, (XSTP_TYPE_MstBpdu_T*)pdu) ) != XSTP_TYPE_CISTID )
                        {
                           /* +++ EnterCriticalRegion +++ */
                           XSTP_OM_EnterCriticalSection(mstid);
                           om_ptr  = XSTP_OM_GetInstanceInfoPtr(mstid);
                           om_ptr->delay_flag     = TRUE;
                           /* +++ LeaveCriticalRegion +++ */
                           XSTP_OM_LeaveCriticalSection(mstid);
                            index++;
                        } /* End of while (XSTP_RX_GetMstidFromBpdu) */
                    } /* End of if (MSTP BPDU) */
                    break;

                default:
                    /* Unknown BPDU Type */
                    break;
            } /* End of switch (pdu->bpdu_header.bpdu_type) */
        } /* End of if (valid bpdu) */
    } /* End of if (XSTP_MGR_IsXstpEnable) */
    if (TRUE == release_mref)
    {
        L_MM_Mref_Release(&mref_handle_p);
    }
    return;
} /* End of XSTP_MGR_ProcessRcvdBpdu */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_InitStateMachine
 *-------------------------------------------------------------------------
 * PURPOSE  : Initialize the state machines
 * INPUT    : xstid     -- spanning tree instance id
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_MGR_InitStateMachine(UI32_T xstid)
{
    XSTP_OM_InstanceData_T  *om_ptr;

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(xstid);

    om_ptr  = XSTP_OM_GetInstanceInfoPtr(xstid);

    /* 13.23.8 mst_config_id */
    if (xstid == XSTP_TYPE_CISTID)
    {
        XSTP_OM_InitMstConfigId(om_ptr);
    }

    XSTP_ENGINE_InitStateMachine(om_ptr, xstid);

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(xstid);

    return;
} /* XSTP_MGR_InitStateMachine */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_PTIStateMachineProgress
 *-------------------------------------------------------------------------
 * PURPOSE  : Motivate the port timer state machine
 * INPUT    : xstid -- Spanning Tree Instance ID
 * OUTPUT   : None
 * RETUEN   : TRUE if any of the timers is expired, else FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_MGR_PTIStateMachineProgress(UI32_T xstid)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  begin;
    BOOL_T                  result;

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(xstid);

    om_ptr  = XSTP_OM_GetInstanceInfoPtr(xstid);

    begin   = XSTP_ENGINE_CheckStateMachineBegin(om_ptr);
    result  = XSTP_ENGINE_PTIStateMachineProgress(om_ptr);
    result  = result || begin;

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(xstid);

    return result;
} /* End of XSTP_MGR_PTIStateMachineProgress */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_StateMachineProgress
 * ------------------------------------------------------------------------
 * PURPOSE  : Motivate the port/bridge state machine
 * INPUT    : xstid -- Spanning Tree Instance ID
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_StateMachineProgress(UI32_T xstid)
{
    XSTP_OM_InstanceData_T  *om_ptr;

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(xstid);

    om_ptr = XSTP_OM_GetInstanceInfoPtr(xstid);

    XSTP_ENGINE_StateMachineProgress(om_ptr);

    if (!om_ptr->bridge_info.common->begin)
    {
        om_ptr->dirty_bit = FALSE;
    }

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(xstid);

    return;
} /* End of XSTP_MGR_StateMachineProgress */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GenerateOneSecondTickSignal
 * ------------------------------------------------------------------------
 * PURPOSE  : Motivate the port/bridge state machine
 * INPUT    : xstid -- Spanning Tree Instance ID
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_GenerateOneSecondTickSignal(UI32_T xstid)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    UI32_T                  lport;

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(xstid);

    om_ptr              = XSTP_OM_GetInstanceInfoPtr(xstid);
    lport               = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr         = &(om_ptr->port_info[lport-1]);
        pom_ptr->common->tick   = TRUE;
    }

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(xstid);

    return;
} /* End of XSTP_MGR_GenerateOneSecondTickSignal */


/* ===================================================================== */
/* ===================================================================== */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_LportLinkup_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Maintain the XSTP when the port is linkup
 * INPUT    : UI32_T    lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function is invoked by SWCTRL.
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_LportLinkup_CallBack(UI32_T lport)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr = NULL;
    UI32_T                  mstid;
    BOOL_T                  sm_motivate;
    UI32_T                  force_version;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
        BACKDOOR_MGR_Printf("\r\nXSTP_MGR_LportLinkup_CallBack : lport = %ld", lport);


    force_version       = (UI32_T)XSTP_OM_GetForceVersion();

    mstid               = XSTP_TYPE_CISTID ;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    do
    {
        sm_motivate = FALSE;
        pom_ptr = &(om_ptr->port_info[lport-1]);

        if (om_ptr->instance_exist)
        {
            pom_ptr->common->link_up            = TRUE;

            /* auto-determinated path cost */
            XSTP_OM_RefreshPathCost(om_ptr, lport);

            if (pom_ptr->is_member)
            {
                if ( pom_ptr->common->port_spanning_tree_status == VAL_staPortSystemStatus_enabled)
                {
                    if (    (pom_ptr->common->port_enabled)
                         && (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DISABLED)
                       )
                    {
                        sm_motivate = TRUE;
                    }
                }
                else
                {
                    XSTP_ENGINE_SetPortState(om_ptr, lport, TRUE, TRUE);
                }
            }
        }
        else
        {
            if (om_ptr->instance_id == XSTP_TYPE_CISTID)
            {
                /* System Spanning Tree Status is disabled */
                pom_ptr->common->link_up    = TRUE;

                /* auto-determinated path cost */
                XSTP_OM_RefreshPathCost(om_ptr, lport);

                pom_ptr->role               = XSTP_ENGINE_PORTVAR_ROLE_DISABLED;
                XSTP_ENGINE_SetPortState(om_ptr, lport, TRUE, TRUE);
            }
            sm_motivate = FALSE;
        }

        XSTP_OM_RefreshOperLinkType(om_ptr, lport);

        if (    (mstid == XSTP_TYPE_CISTID)
            &&  (force_version >= 2)
            && (pom_ptr->common->port_spanning_tree_status!=VAL_staPortSystemStatus_disabled)
           )
        {
            pom_ptr->common->mcheck = TRUE;
        }

        if (sm_motivate)
            XSTP_ENGINE_StateMachineProgress(om_ptr);
    }while(XSTP_OM_GetNextInstanceInfoPtr(&mstid, &om_ptr));
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    return;
} /* End of XSTP_MGR_LportLinkup_CallBack */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_LportLinkdown_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Maintain the XSTP when the port is linkup
 * INPUT    : UI32_T    lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function is invoked by SWCTRL.

 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_LportLinkdown_CallBack(UI32_T lport)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr = NULL;
    UI32_T                  mstid;
    BOOL_T                  sm_motivate;
    #if (SYS_CPNT_SUPPORT_XSTP_BACKUP_ROOT == TRUE)
    BOOL_T                  is_trunk = FALSE;
    #endif /* SYS_CPNT_SUPPORT_XSTP_BACKUP_ROOT == TRUE */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
        BACKDOOR_MGR_Printf("\r\nXSTP_MGR_LportLinkdown_CallBack : lport = %ld", lport);

    #if (SYS_CPNT_SUPPORT_XSTP_BACKUP_ROOT == TRUE)
    is_trunk = SWCTRL_LogicalPortIsTrunkPort(lport);
    #endif /* SYS_CPNT_SUPPORT_XSTP_BACKUP_ROOT == TRUE */

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);

    mstid               = XSTP_TYPE_CISTID;
    do
    {
        sm_motivate = FALSE;
        pom_ptr = &(om_ptr->port_info[lport-1]);

        if (om_ptr->instance_exist)
        {
            pom_ptr->common->link_up    = FALSE;

            /* auto-determinated path cost */
            XSTP_OM_RefreshPathCost(om_ptr, lport);
            if (pom_ptr->common->port_spanning_tree_status == VAL_staPortSystemStatus_enabled)
            {
                if (    (pom_ptr->is_member)
                     && (pom_ptr->common->port_enabled)
                     && (pom_ptr->info_is != XSTP_ENGINE_PORTVAR_INFO_IS_DISABLED)
                   )
                {
                    pom_ptr->sms_pim        = XSTP_ENGINE_SM_PIM_STATE_DISABLED;
                    sm_motivate             = TRUE;
                }
            }
            else
            {
                pom_ptr->role               = XSTP_ENGINE_PORTVAR_ROLE_DISABLED;
                XSTP_ENGINE_SetPortState(om_ptr, lport, FALSE, FALSE);
            }
        }
        else
        {
            if (om_ptr->instance_id == XSTP_TYPE_CISTID)
            {
                /* System Spanning Tree Status is disabled */
                pom_ptr->common->link_up    = FALSE;

                /* auto-determinated path cost */
                XSTP_OM_RefreshPathCost(om_ptr, lport);
                pom_ptr->role               = XSTP_ENGINE_PORTVAR_ROLE_DISABLED;
                XSTP_ENGINE_SetPortState(om_ptr, lport, FALSE, FALSE);
            }
            sm_motivate = FALSE;
        }

        XSTP_OM_RefreshOperLinkType(om_ptr, lport);

        #if (SYS_CPNT_SUPPORT_XSTP_BACKUP_ROOT == TRUE)
        /* Support Backup Root feature */
        /* if it is NORMAL port, do this check here.
           else it is TRUNK port, do this check on receiving last trunk_member "oper_down" callback.
        */
        if (    (!is_trunk)
             && (XSTP_MGR_HasRootBridgeDisappeared(om_ptr, lport))
           )
        {
            XSTP_MGR_BecomeNewRootBridge(om_ptr);
            sm_motivate   = TRUE;
        }
        #endif /* SYS_CPNT_SUPPORT_XSTP_BACKUP_ROOT == TRUE */

        if (sm_motivate)
        {
            XSTP_ENGINE_StateMachineProgress(om_ptr);
        }
    }while(XSTP_OM_GetNextInstanceInfoPtr(&mstid, &om_ptr));

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    bpdu_detected[lport-1] = FALSE;
#endif

    return;
} /* End of XSTP_MGR_LportLinkdown_CallBack */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_PortAdminEnable_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the specified lport enable
 * INPUT    : UI32_T    lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function is invoked by SWCTRL.
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_PortAdminEnable_CallBack(UI32_T lport)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    UI32_T                  mstid;
    BOOL_T                  sm_motivate;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
        BACKDOOR_MGR_Printf("\r\nXSTP_MGR_PortAdminEnable_CallBack : lport = %ld", lport);


    mstid               = XSTP_TYPE_CISTID;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    do
    {
        sm_motivate = FALSE;

        if (om_ptr->instance_exist)
        {
            pom_ptr = &(om_ptr->port_info[lport-1]);

            if (mstid == XSTP_TYPE_CISTID)
            {
                pom_ptr->common->port_enabled                   = TRUE;
            }
            if (    (pom_ptr->is_member)
                 && (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DISABLED)
                 && (pom_ptr->common->link_up)
               )
            {
                pom_ptr->selected_role  = XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED;
                /* auto-determinated path cost */
                XSTP_OM_RefreshPathCost(om_ptr, lport);
                sm_motivate = TRUE;
            }
        }
        else
        {
            if (om_ptr->instance_id == XSTP_TYPE_CISTID)
            {
                /* System Spanning Tree Status is disabled */
                pom_ptr                             = &(om_ptr->port_info[lport-1]);
                pom_ptr->common->port_enabled       = TRUE;
                /* auto-determinated path cost */
                XSTP_OM_RefreshPathCost(om_ptr, lport);
            }
            sm_motivate = FALSE;
        }

        XSTP_OM_RefreshOperLinkType(om_ptr, lport);

        if (sm_motivate)
            XSTP_ENGINE_StateMachineProgress(om_ptr);
    } while(XSTP_OM_GetNextInstanceInfoPtr(&mstid, &om_ptr));

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
    return;
} /* End of XSTP_MGR_PortAdminEnable_CallBack() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_PortAdminDisable_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the specified lport disable
 * INPUT    : UI32_T    lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function is invoked by SWCTRL.
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_PortAdminDisable_CallBack(UI32_T lport)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    UI32_T                  mstid;
    BOOL_T                  sm_motivate;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
        BACKDOOR_MGR_Printf("\r\nXSTP_MGR_PortAdminDisable_CallBack : lport = %ld", lport);

    mstid               = XSTP_TYPE_CISTID;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    do
    {
        sm_motivate = FALSE;

        if (om_ptr->instance_exist)
        {
            pom_ptr = &(om_ptr->port_info[lport-1]);
            pom_ptr->common->port_enabled       = FALSE;
            /* auto-determinated path cost */
            XSTP_OM_RefreshPathCost(om_ptr, lport);
            if (pom_ptr->info_is != XSTP_ENGINE_PORTVAR_INFO_IS_DISABLED)
            {
                pom_ptr->sms_pim                = XSTP_ENGINE_SM_PIM_STATE_DISABLED;
            }
            if (!pom_ptr->common->init_pm)
            {
                pom_ptr->sms_ppm                = XSTP_ENGINE_SM_PPM_STATE_INIT;
            }
            if (pom_ptr->is_member)
                sm_motivate = TRUE;
        }
        else
        {
            if (om_ptr->instance_id == XSTP_TYPE_CISTID)
            {
                /* System Spanning Tree Status is disabled */
                pom_ptr                             = &(om_ptr->port_info[lport-1]);
                pom_ptr->common->port_enabled       = FALSE;
                /* auto-determinated path cost */
                XSTP_OM_RefreshPathCost(om_ptr, lport);
            }
            sm_motivate = FALSE;
        }

        XSTP_OM_RefreshOperLinkType(om_ptr, lport);

        if (sm_motivate)
            XSTP_ENGINE_StateMachineProgress(om_ptr);

    }while(XSTP_OM_GetNextInstanceInfoPtr(&mstid, &om_ptr));
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    return;
} /* End of XSTP_MGR_PortAdminDisable_CallBack() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_PortSpeedDuplex_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port speed-duplex
 *            is changed
 * INPUT    : UI32_T    lport
 *            UI32_T    speed_duplex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_PortSpeedDuplex_CallBack(UI32_T lport, UI32_T speed_duplex)
{
    UI32_T                  xstid;
    BOOL_T                  sm_motivate;
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
        BACKDOOR_MGR_Printf("\r\nXSTP_MGR_PortSpeedDuplex_CallBack : lport = %ld, speed_duplex = %ld", lport, speed_duplex);

    sm_motivate         = FALSE;
    xstid               = XSTP_TYPE_CISTID;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    do
    {
        pom_ptr             = &(om_ptr->port_info[lport-1]);
        if (om_ptr->instance_exist)
        {
            if (pom_ptr->is_member)
            {
                /* auto-determinated path cost */
                if (XSTP_OM_RefreshPathCost(om_ptr, lport))
                {
                    sm_motivate             = TRUE;
                }
            }
        }
        else
        {
            if (om_ptr->instance_id == XSTP_TYPE_CISTID)
            {
                /* System Spanning Tree Status is disabled */
                XSTP_OM_RefreshPathCost(om_ptr, lport);
            }
            sm_motivate = FALSE;
        }
        XSTP_OM_RefreshOperLinkType(om_ptr, lport);

        if (sm_motivate)
            XSTP_ENGINE_StateMachineProgress(om_ptr);
    }while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &om_ptr));

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    return;
} /* End of XSTP_MGR_PortSpeedDuplex_CallBack */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_TrunkMemberAdd_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port joins the trunk
 *            as the 2nd or the following member
 * INPUT    : UI32_T    trunk_ifindex
 *            UI32_T    member_ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1.member_ifindex is sure to be a normal port asserted by SWCTRL.
 *            2.Release member port's MREF,  becuase this member port will NOT exist.
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_TrunkMemberAdd_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *trunk_pom_ptr;
    XSTP_OM_PortVar_T       *member_pom_ptr;
    UI32_T                  xstid;
    UI32_T                  shutdown_reason;
    BOOL_T                  sm_motivate;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
        BACKDOOR_MGR_Printf("\r\nXSTP_MGR_TrunkMemberAdd_CallBack : trunk_ifindex = %ld, member_ifindex = %ld", trunk_ifindex, member_ifindex);

    xstid               = XSTP_TYPE_CISTID;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    do
    {
        trunk_pom_ptr       = &(om_ptr->port_info[trunk_ifindex-1]);
        member_pom_ptr      = &(om_ptr->port_info[member_ifindex-1]);

        XSTP_OM_MakeOmLportConsistency(om_ptr, trunk_ifindex, member_ifindex);
        XSTP_ENGINE_InitPortStateMachines(om_ptr, member_ifindex);
        member_pom_ptr->is_member                   = FALSE;
        /*only need set cist port info parent_index*/
        member_pom_ptr->parent_index = trunk_ifindex;
        XSTP_ENGINE_SetPortState(om_ptr, trunk_ifindex, trunk_pom_ptr->learning, trunk_pom_ptr->forwarding);

        if ( (om_ptr->instance_exist) && (trunk_pom_ptr->is_member) )
        {
            sm_motivate = TRUE;
        }
        else
        {
            sm_motivate = FALSE;
        }

        /* auto-determinated path cost */
        if (XSTP_OM_RefreshPathCost(om_ptr, trunk_ifindex))
        {
            if (  (trunk_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
                ||(trunk_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                #ifdef  XSTP_TYPE_PROTOCOL_MSTP
                ||(trunk_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_MASTER)
                #endif /* XSTP_TYPE_PROTOCOL_MSTP */
               )
            {
                /*XSTP_MGR_InitPortPriorityVector(om_ptr, trunk_ifindex);*/
                trunk_pom_ptr->reselect = TRUE;
            }
            sm_motivate = TRUE;
        }
        if (sm_motivate)
        {
            XSTP_ENGINE_StateMachineProgress(om_ptr);
        }
    }while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &om_ptr));

    if (member_pom_ptr->common->mref_handle_p != NULL)
    {
        L_MM_Mref_Release(&(member_pom_ptr->common->mref_handle_p));
        member_pom_ptr->common->mref_handle_p = NULL;
        member_pom_ptr->common->bpdu = NULL;
    }

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    if (SWCTRL_GetPortStatus(trunk_ifindex, &shutdown_reason) == FALSE)
    {
        return;
    }

    if (shutdown_reason & SWCTRL_PORT_STATUS_SET_BY_XSTP_LBD)
    {
        SYS_CALLBACK_MGR_SetPortStatusCallback(SYS_MODULE_XSTP, trunk_ifindex,
            FALSE, SWCTRL_PORT_STATUS_SET_BY_XSTP_LBD);
    }
    else
    {
        SYS_CALLBACK_MGR_SetPortStatusCallback(SYS_MODULE_XSTP, trunk_ifindex,
            TRUE, SWCTRL_PORT_STATUS_SET_BY_XSTP_LBD);
    }

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    if (shutdown_reason & SWCTRL_PORT_STATUS_SET_BY_XSTP_BPDUGUARD)
    {
        SYS_CALLBACK_MGR_SetPortStatusCallback(SYS_MODULE_XSTP, trunk_ifindex,
            FALSE, SWCTRL_PORT_STATUS_SET_BY_XSTP_BPDUGUARD);
    }
    else
    {
        SYS_CALLBACK_MGR_SetPortStatusCallback(SYS_MODULE_XSTP, trunk_ifindex,
            TRUE, SWCTRL_PORT_STATUS_SET_BY_XSTP_BPDUGUARD);
    }
#endif

    return;
} /* End of XSTP_MGR_TrunkMemberAdd_CallBack */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_TrunkMemberAdd1st_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port joins the trunk
 *            as the 1st member
 * INPUT    : UI32_T    trunk_ifindex
 *            UI32_T    member_ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1.member_ifindex is sure to be a normal port asserted by SWCTRL.
 *            2.Reset member port MREF pointer to NULL, because the new appeared
 *            trunk port inherits the first member port, including receives BPDU.
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_TrunkMemberAdd1st_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *trunk_pom_ptr;
    XSTP_OM_PortVar_T       *member_pom_ptr;
    UI32_T                  xstid;
    BOOL_T                  sm_motivate;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
        BACKDOOR_MGR_Printf("\r\nXSTP_MGR_TrunkMemberAdd1st_CallBack : trunk_ifindex = %ld, member_ifindex = %ld", trunk_ifindex, member_ifindex);

    xstid               = XSTP_TYPE_CISTID;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);

    do
    {
        trunk_pom_ptr       = &(om_ptr->port_info[trunk_ifindex-1]);
        member_pom_ptr      = &(om_ptr->port_info[member_ifindex-1]);
        #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE)
        {
            XSTP_MGR_RecordAttribute(om_ptr, trunk_ifindex);
        }
        #endif /* #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE) */

        XSTP_OM_ConvertPortOm(om_ptr, trunk_ifindex, member_ifindex);
        XSTP_OM_MakeOmLportConsistency(om_ptr, trunk_ifindex, member_ifindex);
        XSTP_ENGINE_InitPortStateMachines(om_ptr, member_ifindex);
        member_pom_ptr->is_member                   = FALSE;
        member_pom_ptr->parent_index = trunk_ifindex;

        #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE)
        {
            XSTP_MGR_RestoreAttribute(om_ptr, trunk_ifindex);
        }
        #endif  /* #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE) */

        if ( (om_ptr->instance_exist) && (trunk_pom_ptr->is_member) )
        {
            sm_motivate = TRUE;
        }
        else
        {
            sm_motivate = FALSE;
        }

        /* auto-determinated path cost */
        XSTP_OM_RefreshPathCost(om_ptr, trunk_ifindex);
        if (  (trunk_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
            ||(trunk_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
            #ifdef  XSTP_TYPE_PROTOCOL_MSTP
            ||(trunk_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_MASTER)
            #endif /* XSTP_TYPE_PROTOCOL_MSTP */
           )
        {
            trunk_pom_ptr->reselect = TRUE;
            sm_motivate = TRUE;
        }
        if (sm_motivate)
             XSTP_ENGINE_StateMachineProgress(om_ptr);
    }while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &om_ptr));
    member_pom_ptr->common->mref_handle_p = NULL;
    member_pom_ptr->common->bpdu = NULL;

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    return;
} /* End of XSTP_MGR_TrunkMemberAdd1st_CallBack */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_TrunkMemberDelete_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the 2nd or the following
 *            trunk member is removed from the trunk
 * INPUT    : UI32_T    trunk_ifindex
 *            UI32_T    member_ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1.member_ifindex exists but the corresponding lport doesn't.
 *            Hence, we have to convert ifindex to uport and then create
 *            the new lport in the following implementation.
 *            2.Reset member port MREF pointer to NULL, because this new appeared
 *            logical port doesn't receive BPDU.
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_TrunkMemberDelete_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *trunk_pom_ptr;
    XSTP_OM_PortVar_T       *member_pom_ptr;
    UI32_T                  xstid;
    BOOL_T                  sm_motivate;
    BOOL_T                  link_up;
    UI32_T                  port_oper_status;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
        BACKDOOR_MGR_Printf("\r\nXSTP_MGR_TrunkMemberDelete_CallBack : trunk_ifindex = %ld, member_ifindex = %ld", trunk_ifindex, member_ifindex);

    sm_motivate         = FALSE;
    xstid               = XSTP_TYPE_CISTID;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    do
    {
        trunk_pom_ptr       = &(om_ptr->port_info[trunk_ifindex-1]);
        member_pom_ptr      = &(om_ptr->port_info[member_ifindex-1]);
        /* Restart the port member_ifindex */
        link_up = FALSE;
        if (SWCTRL_GetPortOperStatus(member_ifindex, &port_oper_status))
        {
            if (port_oper_status == VAL_ifOperStatus_up)
            {
                if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
                {
                    BACKDOOR_MGR_Printf("\r\nXSTP_MGR_TrunkMemberDelete_CallBack : member_ifindex[%ld] is OperUp", member_ifindex);
                }
                link_up = TRUE;
            }
        }

        if (xstid == XSTP_TYPE_CISTID)
        {
            /*1. Init. trunk_member state
              2. Get and assign link_up state of trunk_member
              3. Inherit port_admin state of trunk_port
              To avoid callback sequence issue.
            */
            #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE)
            {
                XSTP_MGR_RecordAttribute(om_ptr, member_ifindex);
            }
            #endif /* #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE) */

            XSTP_OM_ConvertPortOm(om_ptr, member_ifindex, trunk_ifindex);
            XSTP_OM_MakeOmLportConsistency(om_ptr, member_ifindex, trunk_ifindex);
            XSTP_ENGINE_InitPortStateMachines(om_ptr, member_ifindex);
            member_pom_ptr->common->link_up         = link_up;
            member_pom_ptr->is_member               = TRUE;
            member_pom_ptr->parent_index = 0;
            #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE)
            {
                XSTP_MGR_RestoreAttribute(om_ptr, member_ifindex);
            }
            #endif  /* #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE) */

            if (XSTP_OM_GetSpanningTreeStatus() == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED
                &&( trunk_pom_ptr->common->port_spanning_tree_status == VAL_staPortSystemStatus_enabled))
            {
                XSTP_ENGINE_SetPortState(om_ptr, member_ifindex, FALSE, FALSE);
                sm_motivate = TRUE;
            }
            else
            {
                if (link_up)
                {
                    XSTP_ENGINE_SetPortState(om_ptr, member_ifindex, TRUE, TRUE);
                }
                else
                {
                    XSTP_ENGINE_SetPortState(om_ptr, member_ifindex, FALSE, FALSE);
                }
            }
        }
        else
        {
            if (om_ptr->instance_exist)
            {
                if (XSTP_OM_IsMemberPortOfInstance(om_ptr, trunk_ifindex))
                {

                    #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE)
                    {
                        XSTP_MGR_RecordAttribute(om_ptr, member_ifindex);
                    }
                    #endif /* #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE) */

                    XSTP_OM_ConvertPortOm(om_ptr, member_ifindex, trunk_ifindex);
                    XSTP_OM_MakeOmLportConsistency(om_ptr, member_ifindex, trunk_ifindex);
                    XSTP_ENGINE_InitPortStateMachines(om_ptr, member_ifindex);
                    member_pom_ptr->is_member   = TRUE;
                    member_pom_ptr->parent_index = 0;
                    #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE)
                    {
                        XSTP_MGR_RestoreAttribute(om_ptr, member_ifindex);
                    }
                    #endif  /* #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE) */

                    XSTP_ENGINE_SetPortState(om_ptr, member_ifindex, FALSE, FALSE);
                    sm_motivate = TRUE;
                }
            }
            else
            {
                sm_motivate = FALSE;
            }
        }

        /* auto-determinated path cost */
        if (XSTP_OM_RefreshPathCost(om_ptr, trunk_ifindex))
        {
            if (    (trunk_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                #ifdef  XSTP_TYPE_PROTOCOL_MSTP
                ||  (trunk_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_MASTER)
                #endif /* XSTP_TYPE_PROTOCOL_MSTP */
               )
            {
                /* XSTP_MGR_InitPortPriorityVector(om_ptr, trunk_ifindex); */
                trunk_pom_ptr->reselect = TRUE;
            }
            sm_motivate = TRUE;
        }
        if (sm_motivate)
        {
            XSTP_ENGINE_StateMachineProgress(om_ptr);
        }
    }while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &om_ptr));

    member_pom_ptr->common->mref_handle_p = NULL;
    member_pom_ptr->common->bpdu = NULL;

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    return;
} /* End of XSTP_MGR_TrunkMemberDelete_CallBack */



/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_TrunkMemberDeleteLst_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the last trunk member
 *            is removed from the trunk
 * INPUT    : UI32_T    trunk_ifindex
 *            UI32_T    member_ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1.member_ifindex exists but the corresponding lport doesn't.
 *            Hence, we have to convert ifindex to uport and then create
 *            the new lport in the following implementation.
 *            2.Reset trunk port MREF pointer to NULL, because it will not receive BPDU.
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_TrunkMemberDeleteLst_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *trunk_pom_ptr;
    XSTP_OM_PortVar_T       *member_pom_ptr;
    UI32_T                  xstid;
    BOOL_T                  sm_motivate;
    BOOL_T                  link_up;
    UI32_T                  port_oper_status;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
        BACKDOOR_MGR_Printf("\r\nXSTP_MGR_TrunkMemberDeleteLst_CallBack : trunk_ifindex = %ld, member_ifindex = %ld", trunk_ifindex, member_ifindex);

    sm_motivate         = FALSE;
    xstid               = XSTP_TYPE_CISTID;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    do
    {
        trunk_pom_ptr       = &(om_ptr->port_info[trunk_ifindex-1]);
        member_pom_ptr      = &(om_ptr->port_info[member_ifindex-1]);
        link_up = FALSE;
        if (SWCTRL_GetPortOperStatus(member_ifindex, &port_oper_status))
        {
            if (port_oper_status == VAL_ifOperStatus_up)
            {
                if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
                {
                    BACKDOOR_MGR_Printf("\r\nXSTP_MGR_TrunkMemberDeleteLst_CallBack : member_ifindex[%ld] is OperUp", member_ifindex);
                }
                link_up = TRUE;
            }
        }

        if (xstid == XSTP_TYPE_CISTID)
        {
            /*1. Inherit trunk_port state.
              2. Get and assign link_up state of trunk_member
              To avoid callback sequence issue.
            */

            #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE)
            {
                XSTP_MGR_RecordAttribute(om_ptr, member_ifindex);
            }
            #endif /* #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE) */

            XSTP_OM_ConvertPortOm(om_ptr, member_ifindex, trunk_ifindex);
            XSTP_OM_MakeOmLportConsistency(om_ptr, member_ifindex, trunk_ifindex);
            XSTP_ENGINE_InitPortStateMachines(om_ptr, member_ifindex);
            member_pom_ptr->common->link_up         = link_up;
            XSTP_ENGINE_InitPortStateMachines(om_ptr, trunk_ifindex);
            trunk_pom_ptr->is_member                = FALSE;
            member_pom_ptr->parent_index = 0;
#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
            trunk_pom_ptr->common->tc_prop_group_id = XSTP_TYPE_TC_PROP_DEFAULT_GROUP_ID;
#endif /*#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)*/

            #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE)
            {
                XSTP_MGR_RestoreAttribute(om_ptr, member_ifindex);
            }
            #endif /* #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE) */

            if (XSTP_OM_GetSpanningTreeStatus() == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED
                &&( trunk_pom_ptr->common->port_spanning_tree_status == VAL_staPortSystemStatus_enabled))
            {
                XSTP_ENGINE_SetPortState(om_ptr, member_ifindex, FALSE, FALSE);
                sm_motivate = TRUE;
            }
            else
            {
                if (link_up)
                {
                    XSTP_ENGINE_SetPortState(om_ptr, member_ifindex, TRUE, TRUE);
                }
                else
                {
                    XSTP_ENGINE_SetPortState(om_ptr, member_ifindex, FALSE, FALSE);
                }
            }
        }
        else
        {
            if (om_ptr->instance_exist)
            {
                #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE)
                {
                    XSTP_MGR_RecordAttribute(om_ptr, member_ifindex);
                }
                #endif /* #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE) */

                XSTP_OM_ConvertPortOm(om_ptr, member_ifindex, trunk_ifindex);
                XSTP_OM_MakeOmLportConsistency(om_ptr, member_ifindex, trunk_ifindex);
                XSTP_ENGINE_InitPortStateMachines(om_ptr, member_ifindex);
                XSTP_ENGINE_InitPortStateMachines(om_ptr, trunk_ifindex);
                trunk_pom_ptr->is_member                    = FALSE;

                #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE)
                {
                    XSTP_MGR_RestoreAttribute(om_ptr, member_ifindex);
                }
                #endif /* #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE) */

                if (member_pom_ptr->is_member)
                {
                    XSTP_ENGINE_SetPortState(om_ptr, member_ifindex, FALSE, FALSE);
                    sm_motivate = TRUE;
                }
            }
            else
            {
                sm_motivate = FALSE;
            }
        }

        if (sm_motivate)
            XSTP_ENGINE_StateMachineProgress(om_ptr);
    }while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &om_ptr));

    if (trunk_pom_ptr->common->mref_handle_p != NULL)
    {
        L_MM_Mref_Release(&(trunk_pom_ptr->common->mref_handle_p));
        trunk_pom_ptr->common->mref_handle_p = NULL;
        trunk_pom_ptr->common->bpdu = NULL;
        member_pom_ptr->common->mref_handle_p = NULL;
        member_pom_ptr->common->bpdu = NULL;
    }

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    return;
} /* End of XSTP_MGR_TrunkMemberDeleteLst_CallBack */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_TrunkMemberPortOperUp_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when when port oper status is oper_up
 * INPUT    : UI32_T    trunk_ifindex
 *            UI32_T    member_ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Refresh path cost
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_TrunkMemberPortOperUp_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{

    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *trunk_pom_ptr;
    UI32_T                  xstid;
    BOOL_T                  sm_motivate;


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }
    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
        BACKDOOR_MGR_Printf("\r\nXSTP_MGR_TrunkMemberPortOperUp_CallBack : trunk_ifindex = %ld, member_ifindex = %ld", trunk_ifindex, member_ifindex);

    sm_motivate         = FALSE;
    xstid               = XSTP_TYPE_CISTID;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    do
    {
        if (om_ptr->instance_exist)
        {
            if (XSTP_OM_RefreshPathCost(om_ptr, trunk_ifindex))
            {
                trunk_pom_ptr = &(om_ptr->port_info[trunk_ifindex-1]);
                if ((trunk_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
                    ||(trunk_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
                    ||  (trunk_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_MASTER)
                    #endif /* XSTP_TYPE_PROTOCOL_MSTP */
                    )
                {
                    /* XSTP_MGR_InitPortPriorityVector(om_ptr, trunk_ifindex);*/
                    trunk_pom_ptr->reselect = TRUE;
                }
                sm_motivate = TRUE;
            }
        }
        else
        {
            if (xstid == XSTP_TYPE_CISTID)
            {
                XSTP_OM_RefreshPathCost(om_ptr, trunk_ifindex);
            }
        }

        if (sm_motivate)
            XSTP_ENGINE_StateMachineProgress(om_ptr);
    }while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &om_ptr));

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);


    return;
}/* End of XSTP_MGR_TrunkMemberPortOperUp_CallBack */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_TrunkMemberPortNotOperUp_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when when port oper status is not oper_up
 * INPUT    : UI32_T    trunk_ifindex
 *            UI32_T    member_ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Refresh path cost
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_TrunkMemberPortNotOperUp_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *trunk_pom_ptr;
    UI32_T                  xstid;
    BOOL_T                  sm_motivate;
    #if (SYS_CPNT_SUPPORT_XSTP_BACKUP_ROOT == TRUE)
    UI32_T                  active_lportlist[SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK];
    UI32_T                  count = 0;
    BOOL_T                  is_last_member = FALSE;
    #endif /* SYS_CPNT_SUPPORT_XSTP_BACKUP_ROOT == TRUE */



    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }
    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
        BACKDOOR_MGR_Printf("\r\nXSTP_MGR_TrunkMemberPortNotOperUp_CallBack : trunk_ifindex = %ld, member_ifindex = %ld", trunk_ifindex, member_ifindex);

    #if (SYS_CPNT_SUPPORT_XSTP_BACKUP_ROOT == TRUE)
    {
        /* Check if it is last trunk_member. */
        if (SWCTRL_GetActiveTrunkMember(trunk_ifindex, active_lportlist, &count))
        {
            if (count == 0)
            {
                is_last_member = TRUE;
            }
        }
    }
    #endif /* SYS_CPNT_SUPPORT_XSTP_BACKUP_ROOT == TRUE */

    sm_motivate         = FALSE;
    xstid               = XSTP_TYPE_CISTID;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    do
    {
        if (om_ptr->instance_exist)
        {
            #if (SYS_CPNT_SUPPORT_XSTP_BACKUP_ROOT == TRUE)
            /* Support Backup Root feature */
            /* Do this check if it is last trunk_member */
            if (    (is_last_member)
                &&  (XSTP_MGR_HasRootBridgeDisappeared(om_ptr, trunk_ifindex))
               )
            {
                XSTP_MGR_BecomeNewRootBridge(om_ptr);
                sm_motivate   = TRUE;
            }
            #endif /* SYS_CPNT_SUPPORT_XSTP_BACKUP_ROOT == TRUE */

            if (XSTP_OM_RefreshPathCost(om_ptr, trunk_ifindex))
            {
                trunk_pom_ptr = &(om_ptr->port_info[trunk_ifindex-1]);
                if (    (trunk_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
                    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
                    ||  (trunk_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_MASTER)
                    #endif /* XSTP_TYPE_PROTOCOL_MSTP */
                   )
                {
                    trunk_pom_ptr->reselect = TRUE;
                }
                sm_motivate = TRUE;
            }
        }
        else
        {
            if (xstid == XSTP_TYPE_CISTID)
            {
                XSTP_OM_RefreshPathCost(om_ptr, trunk_ifindex);
            }
        }
        if (sm_motivate)
            XSTP_ENGINE_StateMachineProgress(om_ptr);
    }while(XSTP_OM_GetNextInstanceInfoPtr(&xstid, &om_ptr));

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);


    return;
}/* End of XSTP_MGR_TrunkMemberPortNotOperUp_CallBack */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_VlanCreated_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from VLAN_MGR when a vlan is created
 * INPUT    : vid_ifidx     -- specify which vlan has just been created
 *            vlan_status   --
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_VlanCreated_CallBack(UI32_T vid_ifidx, UI32_T vlan_status)
{
    UI32_T start_time = 0, end_time = 0;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }
    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
    {
        BACKDOOR_MGR_Printf("\r\nXSTP_MGR_VlanCreated_CallBack : vid_ifidx = %ld, vlan_status = %ld", vid_ifidx, vlan_status);
        start_time = SYS_TIME_GetSystemTicksBy10ms();
    }

    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
/*
    #if (SYS_CPNT_SWDRV_CACHE == TRUE)
    {
        XSTP_TYPE_VLAN_MSG_T    msg;

        msg.msg_type = XSTP_TYPE_VLAN_MSG_CREATED;
        msg.vid_ifidx = vid_ifidx;
        msg.lport_ifidx = 0;
        SYSFUN_SendMsgQ (XSTP_MGR_VlanMsgqId, (UI32_T*)&msg, FALSE, SYSFUN_TIMEOUT_NOWAIT);
    }
    #else
*/
    {   UI32_T  vid;

        VLAN_IFINDEX_CONVERTTO_VID(vid_ifidx, vid);
        XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
        XSTP_MGR_SetVlanToMstMappingTable(vid);
        XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
    }
    /*#endif *//* (SYS_CPNT_SWDRV_CACHE == TRUE) */
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
    {
        end_time = SYS_TIME_GetSystemTicksBy10ms();
        BACKDOOR_MGR_Printf("\r\nXSTP_MGR_VlanCreated_CallBack : Time consumption = %lu", (end_time - start_time));
    }

    return;
} /* End of XSTP_MGR_VlanCreated_CallBack */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_VlanDestroy_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from VLAN_MGR when a vlan is destroyed
 * INPUT    : vid_ifidx     -- specify which vlan has just been destroyed
 *            vlan_status   --
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_VlanDestroy_CallBack(UI32_T vid_ifidx, UI32_T vlan_status)
{
    UI32_T start_time = 0, end_time = 0;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }
    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
    {
        BACKDOOR_MGR_Printf("\r\nXSTP_MGR_VlanDestroy_CallBack : vid_ifidx = %ld, vlan_status = %ld", vid_ifidx, vlan_status);
        start_time = SYS_TIME_GetSystemTicksBy10ms();
    }

    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
/*
    #if (SYS_CPNT_SWDRV_CACHE == TRUE)
    {
        XSTP_TYPE_VLAN_MSG_T    msg;

        msg.msg_type = XSTP_TYPE_VLAN_MSG_DESTROY;
        msg.vid_ifidx = vid_ifidx;
        msg.lport_ifidx = 0;
        SYSFUN_SendMsgQ (XSTP_MGR_VlanMsgqId, (UI32_T*)&msg, FALSE, SYSFUN_TIMEOUT_NOWAIT);
    }
    #else
*/
    {
        UI32_T  vid;

        VLAN_IFINDEX_CONVERTTO_VID(vid_ifidx, vid);
        XSTP_MGR_DestroyVlanFromMstMappingTable(vid);
    }
    /*#endif*/ /* (SYS_CPNT_SWDRV_CACHE == TRUE) */
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
    {
        end_time = SYS_TIME_GetSystemTicksBy10ms();
        BACKDOOR_MGR_Printf("\r\nXSTP_MGR_VlanDestroy_CallBack : Time consumption = %lu", (end_time - start_time));
    }

    return;
} /* End of XSTP_MGR_VlanDestroy_CallBack */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_VlanMemberAdd_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from VLAN_MGR when a lport is added to a
 *            vlan's member set.
 * INPUT    : vid_ifidx     -- specify which vlan's member set to be add to
 *            lport_ifindex -- specify which port to be added to the member set
 *            vlan_status   --
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_VlanMemberAdd_CallBack(UI32_T vid_ifidx, UI32_T lport_ifidx, UI32_T vlan_status)
{
    UI32_T start_time = 0, end_time = 0;
    BOOL_T is_need_process = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }
    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
    {
        BACKDOOR_MGR_Printf("\r\nXSTP_MGR_VlanMemberAdd_CallBack : vid_ifidx = %ld, lport_ifidx = %ld, vlan_status = %ld", vid_ifidx, lport_ifidx, vlan_status);
        start_time = SYS_TIME_GetSystemTicksBy10ms();
    }

    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    if (XSTP_OM_IsMstFullMemberTopology())
    {
        /* do nothing */
        return;
    }
    {
        XSTP_OM_InstanceData_T  *om_ptr;
        XSTP_OM_PortVar_T       *pom_ptr;
        UI32_T                  mstid;
        UI32_T                  vid;
        UI32_T                  current_st_mode;

        VLAN_IFINDEX_CONVERTTO_VID(vid_ifidx, vid);

        XSTP_OM_GetMstidFromMstConfigurationTableByVlan(vid, &mstid);

        /* +++ EnterCriticalRegion +++ */
        XSTP_OM_EnterCriticalSection(mstid);
        om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
        pom_ptr             = &(om_ptr->port_info[lport_ifidx-1]);
        if(pom_ptr->is_member == FALSE){
            is_need_process = TRUE;
            pom_ptr->is_member  = TRUE;
            current_st_mode     = (UI32_T)XSTP_OM_GetForceVersion();
            if (current_st_mode == XSTP_TYPE_MSTP_MODE){

                if (pom_ptr->common->port_spanning_tree_status == VAL_staPortSystemStatus_enabled){
                    XSTP_ENGINE_SetPortState(om_ptr, lport_ifidx, pom_ptr->learning, pom_ptr->forwarding);
                }else{
                    /* Set port state if its spanning_tree_status is disabled */
                    if ((pom_ptr->common->link_up) && (pom_ptr->common->port_enabled)){
                        XSTP_ENGINE_SetPortState(om_ptr, lport_ifidx, TRUE, TRUE);
                    }else{
                        XSTP_ENGINE_SetPortState(om_ptr, lport_ifidx, FALSE, FALSE);
                    }
                }
            }
        }
        /* +++ LeaveCriticalRegion +++ */
        XSTP_OM_LeaveCriticalSection(mstid);

        if(is_need_process)
            XSTP_MGR_StateMachineProgress(mstid);
    }
    /*#endif*/ /* (SYS_CPNT_SWDRV_CACHE == TRUE) */
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
    {
        end_time = SYS_TIME_GetSystemTicksBy10ms();
        BACKDOOR_MGR_Printf("\r\nXSTP_MGR_VlanMemberAdd_CallBack : Time consumption = %lu", (end_time - start_time));
    }

    return;
} /* End of XSTP_MGR_VlanMemberAdd_CallBack */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_VlanMemberDelete_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from VLAN_MGR when a port is remove from
 *            vlan's member set.
 * INPUT    : vid_ifidx     -- specify which vlan's member set to be deleted from
 *            lport_ifindex -- specify which port to be deleted
 *            vlan_status   --
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_VlanMemberDelete_CallBack(UI32_T vid_ifidx, UI32_T lport_ifidx, UI32_T vlan_status)
{
    UI32_T start_time = 0, end_time = 0;
    BOOL_T is_need_process = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }
    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
    {
        BACKDOOR_MGR_Printf("\r\nXSTP_MGR_VlanMemberDelete_CallBack : vid_ifidx = %ld, lport_ifidx = %ld, vlan_status = %ld", vid_ifidx, lport_ifidx, vlan_status);
        start_time = SYS_TIME_GetSystemTicksBy10ms();
    }

    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    if (XSTP_OM_IsMstFullMemberTopology())
        return;

    {
        XSTP_OM_InstanceData_T  *om_ptr;
        XSTP_OM_PortVar_T       *pom_ptr;
        UI32_T                  mstid;
        UI32_T                  vid;
        UI32_T                  vlan_id, vid_ifindex;
        BOOL_T                  member_found;

        VLAN_IFINDEX_CONVERTTO_VID(vid_ifidx, vid);

        XSTP_OM_GetMstidFromMstConfigurationTableByVlan(vid, &mstid);
        if (mstid != XSTP_TYPE_CISTID)
        {
            /* +++ EnterCriticalRegion +++ */
            XSTP_OM_EnterCriticalSection(mstid);
            om_ptr = XSTP_OM_GetInstanceInfoPtr(mstid);

            vlan_id = 0;
            member_found    = FALSE;
            while ( (!member_found) && XSTP_OM_GetNextXstpMember(om_ptr, &vlan_id))
            {
                if (vid != vlan_id)
                {
                    VLAN_VID_CONVERTTO_IFINDEX(vlan_id, vid_ifindex);
                    if (VLAN_OM_IsPortVlanMember(vid_ifindex, lport_ifidx))
                    {
                        member_found    = TRUE;
                    }
                }
            }

            if (!member_found)
            {
                pom_ptr = &(om_ptr->port_info[lport_ifidx-1]);

                if(pom_ptr->is_member == TRUE){
                    pom_ptr->is_member  = FALSE;
                    XSTP_ENGINE_InitPortStateMachines(om_ptr, lport_ifidx);
                    if(pom_ptr->common->link_up)
                    {
                      is_need_process = TRUE;
                      XSTP_ENGINE_SetPortState(om_ptr, lport_ifidx, FALSE, FALSE);
                    }
                }
            }

            /* +++ LeaveCriticalRegion +++ */
            XSTP_OM_LeaveCriticalSection(mstid);
        }

        if(is_need_process)
            XSTP_MGR_StateMachineProgress(mstid);
    }
    /*#endif*/ /* (SYS_CPNT_SWDRV_CACHE == TRUE) */
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
    {
        end_time = SYS_TIME_GetSystemTicksBy10ms();
        BACKDOOR_MGR_Printf("\r\nXSTP_MGR_VlanMemberDelete_CallBack : Time consumption = %lu", (end_time - start_time));
    }

    return;
} /* End of XSTP_MGR_VlanMemberDelete_CallBack */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_AnnounceLportChangeState
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will announce the state to the upper module
 *            when the port enters/leaves the forwarding state.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void    XSTP_MGR_AnnounceLportChangeState(void)
{
    XSTP_TYPE_LportList_T       *lport_list;
    XSTP_TYPE_LportList_T       *this_lport;
    UI32_T                      xstid;
    UI32_T                      lport;
    BOOL_T                      enter_forwarding;

    lport_list  = XSTP_UTY_RetrieveChangeStateLportList();
    if(NULL!= lport_list)
    {
        SYS_CALLBACK_MGR_LportChangeStateCallback(SYS_MODULE_XSTP);
    }

    while (lport_list != NULL)
    {
        this_lport              = lport_list;
        xstid                   = this_lport->xstid;
        lport                   = this_lport->lport;
        enter_forwarding        = this_lport->enter_forwarding;
        if (enter_forwarding)
        {
            SYS_CALLBACK_MGR_LportEnterForwardingCallback(SYS_MODULE_XSTP, xstid, lport);
            if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
                BACKDOOR_MGR_Printf("\r\nLportEnterForwarding for Instance 0x%02lx at lport 0x%02lx", xstid, lport);
        }
        else
        {
            SYS_CALLBACK_MGR_LportLeaveForwardingCallback(SYS_MODULE_XSTP, xstid, lport);
            if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
                BACKDOOR_MGR_Printf("\r\nLportLeaveForwarding for Instance 0x%02lx at lport 0x%02lx", xstid, lport);
        }
        lport_list  = lport_list->next;
        L_MM_Free((void*)this_lport);
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
            BACKDOOR_MGR_Printf("\r\nXSTP_MGR_AnnounceLportChangeState for Instance 0x%02lx at lport 0x%02lx", xstid, lport);
    }

    return;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_NotifyLportChangeState
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will announce the state to the upper module
 *            when the port enters/leaves the forwarding state.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void    XSTP_MGR_NotifyLportChangeState(void)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    if (XSTP_MGR_ChangeStatePortList == NULL)
    {
        XSTP_MGR_ChangeStatePortList = XSTP_UTY_RetrieveChangeStateLportList();
    }

    if (XSTP_MGR_ChangeStatePortList != NULL)
    {
        SYS_CALLBACK_MGR_LportChangeStateCallback(SYS_MODULE_XSTP);
       /*gvrp and xstp in one process , no need send two message by different ipc
        * Change the IPC as below . the condition must be one of :
        * 1. the GARP Layer is upper than XSTP
        * or else
        * 2. this callback is Send event to GARP
        */
    }

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetChangeStatePortListForbidden
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will set the flag which controls whether the
 *            XSTP_MGR_ChangeStatePortList is allowed to be added new
 *            element.
 * INPUT    : flag    -- TRUE:disallowed, FALSE:allowed
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void    XSTP_MGR_SetChangeStatePortListForbidden(BOOL_T flag)
{
    XSTP_UTY_SetChangeStatePortListForbidden(flag);
} /* End of XSTP_MGR_SetChangeStatePortListControlFlag */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_RetrieveChangeStateLportList
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will retrieve the lport list which enters/leaves
 *            the forwarding state.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : XSTP_MGR_ChangeStatePortList
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
XSTP_TYPE_LportList_T    *XSTP_MGR_RetrieveChangeStateLportList(void)
{
    XSTP_TYPE_LportList_T    *lport_list;

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);

    lport_list  = XSTP_MGR_ChangeStatePortList;

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    return lport_list;
} /* End of XSTP_MGR_RetrieveChangeStateLportList */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_ResetChangeStateLportList
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will reset the lport list which enters/leaves
 *            the forwarding state. Then the XSTP_MGR_ChangeStatePortList
 *            is cleared.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void XSTP_MGR_ResetChangeStateLportList(void)
{

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);

    XSTP_MGR_ChangeStatePortList    = NULL;

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    return ;
} /* End of XSTP_MGR_ResetChangeStateLportList */

/* ===================================================================== */
/* ===================================================================== */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetSystemSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the global spanning tree status.
 * INPUT    :   UI32_T status           -- the status value
 *                                         VAL_xstpSystemStatus_enabled
 *                                         VAL_xstpSystemStatus_disabled
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR       -- not master mode
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetSystemSpanningTreeStatus(UI32_T status)
{
    XSTP_OM_InstanceData_T          *om_ptr;
    UI32_T                          current_st_status;
    UI32_T                          lport;
    UI32_T                          mstid;
    XSTP_OM_PortVar_T               *pom_ptr;
    UI32_T                          current_st_mode;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    current_st_status = XSTP_OM_GetSpanningTreeStatus();
    current_st_mode = (UI32_T)XSTP_OM_GetForceVersion();

    if(current_st_status == status)
    {
        return XSTP_TYPE_RETURN_OK;
    }
    if (status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
    {
        if (current_st_mode == XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID)
        {
            /* MST mode */
            #ifdef  XSTP_TYPE_PROTOCOL_MSTP
            if (!SWCTRL_PMGR_SetMstEnableStatus(SWCTRL_MST_ENABLE) )
            {
                if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
                {
                    BACKDOOR_MGR_Printf("\r\nXSTP_MGR_SetSystemSpanningTreeStatus: Fatal Error!! Set to MST mode failed.");
                }
                EH_MGR_Handle_Exception1(   SYS_MODULE_XSTP,
                                            XSTP_MGR_SetSystemSpanningTreeStatus__Fun_No,
                                            EH_TYPE_MSG_FAILED_TO,
                                            SYSLOG_LEVEL_INFO,
                                            "set spanning tree status"
                                        );
                        return XSTP_TYPE_RETURN_ERROR;
            }
            #endif /* XSTP_TYPE_PROTOCOL_MSTP */
            XSTP_ENGINE_SetMstEnableStatus(TRUE);
        }

        /* +++ EnterCriticalRegion +++ */
        XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
        XSTP_OM_SetSpanningTreeStatus(status);
        om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
        XSTP_MGR_CreateSpanningTree(om_ptr);
        /* +++ LeaveCriticalRegion +++ */
        XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

        #ifdef  XSTP_TYPE_PROTOCOL_MSTP
        /* Create other existing instances except CIST if mode is MSTP*/
        if (current_st_mode == XSTP_TYPE_MSTP_MODE)
        {
            /* +++ EnterCriticalRegion +++ */
            XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
            XSTP_MGR_RecoverExistingMSTI();
            /* +++ LeaveCriticalRegion +++ */
            XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
        }
        #endif /* XSTP_TYPE_PROTOCOL_MSTP */

        /*set xstp disabled port enter forwarding*/
        mstid = XSTP_TYPE_CISTID;

        XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
        do
        {
            om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);

            if (!om_ptr->instance_exist)
              continue;

            lport =0;

            while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
              pom_ptr = &(om_ptr->port_info[lport-1]);

              if (pom_ptr->is_member)
              {
                  if (pom_ptr->common->port_spanning_tree_status != VAL_staPortSystemStatus_enabled)
                  {
                      XSTP_ENGINE_SetPortState(om_ptr, lport, TRUE, TRUE);
                  }
              }
           }
        }while(XSTP_OM_GetNextInstanceInfoPtr(&mstid, &om_ptr));

       XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    }
    else
    {
        /* MST mode */
        #ifdef  XSTP_TYPE_PROTOCOL_MSTP
        if (!SWCTRL_PMGR_SetMstEnableStatus(SWCTRL_MST_DISABLE) )
        {
            if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
            {
                BACKDOOR_MGR_Printf("\r\nXSTP_MGR_SetSystemSpanningTreeVersion: Fatal Error!! Set to MST mode failed.");
            }
            EH_MGR_Handle_Exception1(   SYS_MODULE_XSTP,
                                        XSTP_MGR_SetSystemSpanningTreeStatus__Fun_No,
                                        EH_TYPE_MSG_FAILED_TO,
                                        SYSLOG_LEVEL_INFO,
                                        "set spanning tree status"
                                    );
                return XSTP_TYPE_RETURN_ERROR;
        }
        #endif /* XSTP_TYPE_PROTOCOL_MSTP */
        XSTP_ENGINE_SetMstEnableStatus(FALSE);

        /* +++ EnterCriticalRegion +++ */
        XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
        mstid               = XSTP_TYPE_CISTID;
        XSTP_OM_SetSpanningTreeStatus(status);
        om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
        do
        {
            if (om_ptr->instance_exist)
            {
                XSTP_MGR_DeleteSpanningTree(om_ptr);
            }
        } while(XSTP_OM_GetNextInstanceInfoPtr(&mstid, &om_ptr));

        lport   = 0;
        while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
        {
            om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
            pom_ptr             = &(om_ptr->port_info[lport-1]);
            pom_ptr->role       = XSTP_ENGINE_PORTVAR_ROLE_DISABLED;
            if ((pom_ptr->common->link_up) && (pom_ptr->common->port_enabled))
            {
                XSTP_ENGINE_SetPortState(om_ptr, lport, TRUE, TRUE);
            }
            else
            {
                XSTP_ENGINE_SetPortState(om_ptr, lport, FALSE, FALSE);
            }
        }
        /* +++ LeaveCriticalRegion +++ */
        XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
    }
    XSTP_MGR_NotifyStpChangeVersion(current_st_mode, status);
    return XSTP_TYPE_RETURN_OK;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetSystemSpanningTreeVersion
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the spanning tree mode.
 * OUTPUT   :   UI32_T mode         -- the mode value
 *                                     VAL_dot1dStpVersion_stpCompatible(0)
 *                                     VAL_dot1dStpVersion_rstp(2)
 *                                     VAL_dot1dStpVersion_mstp(3)
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK     -- set successfully
 *              XSTP_TYPE_RETURN_ERROR  -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_ST_STATUS_DISABLED -- status is disabled
 * NOTE     :   Default             -- SYS_DFLT_STP_PROTOCOL_TYPE
 *              Can't set mode when the status is disabled.
 * REF      :   draft-ietf-bridge-rstpmib-02/dot1dStp 16
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetSystemSpanningTreeVersion(UI32_T mode)
{
    UI32_T                  current_st_mode;
    UI32_T                  current_st_status;
    UI32_T                  mstid;
    XSTP_OM_InstanceData_T  *om_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }
    current_st_mode = (UI32_T)XSTP_OM_GetForceVersion();
    current_st_status = XSTP_OM_GetSpanningTreeStatus();
    if (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_DISABLED)
    {
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
        {
            BACKDOOR_MGR_Printf("\r\nXSTP_MGR_SetSystemSpanningTreeVersion::ERROR!! Can't set mode when the status is disabled");
        }
        EH_MGR_Handle_Exception(SYS_MODULE_XSTP, XSTP_MGR_SetSystemSpanningTreeVersion_Fun_No, EH_TYPE_MSG_STA_MODE_SET_FAILED, SYSLOG_LEVEL_INFO);
        return XSTP_TYPE_RETURN_ST_STATUS_DISABLED;
    }
    if ( (mode != XSTP_TYPE_STP_MODE) &&
#ifdef  XSTP_TYPE_PROTOCOL_MSTP
         (mode != XSTP_TYPE_MSTP_MODE) &&
#endif /* XSTP_TYPE_PROTOCOL_MSTP */
         (mode != XSTP_TYPE_RSTP_MODE) )
    {
        return XSTP_TYPE_RETURN_ERROR;
    }
    if (current_st_mode == mode)
    {
        return XSTP_TYPE_RETURN_OK;
    }

    if ((current_st_mode == XSTP_TYPE_MSTP_MODE) &&
        ((mode == XSTP_TYPE_STP_MODE) || (mode == XSTP_TYPE_RSTP_MODE)))
    {
        mstid               = XSTP_TYPE_CISTID;
        /* +++ EnterCriticalRegion +++ */
        XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
        om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
        while(XSTP_OM_GetNextInstanceInfoPtr(&mstid, &om_ptr))
        {
            if (om_ptr->instance_exist)
            {
                XSTP_MGR_DeleteSpanningTree(om_ptr);
            }
        }
        XSTP_OM_SetMaxInstanceNumber((UI32_T)1);
        XSTP_OM_SetForceVersion((UI8_T)mode);

        /* MST mode */
        #ifdef  XSTP_TYPE_PROTOCOL_MSTP
        XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
        if (!SWCTRL_PMGR_SetMstEnableStatus(SWCTRL_MST_DISABLE) )
        {
            if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
            {
                BACKDOOR_MGR_Printf("\r\nXSTP_MGR_SetSystemSpanningTreeVersion: Fatal Error!! Set to MST mode failed.");
            }
            EH_MGR_Handle_Exception1(   SYS_MODULE_XSTP,
                                        XSTP_MGR_SetSystemSpanningTreeVersion_Fun_No,
                                        EH_TYPE_MSG_FAILED_TO,
                                        SYSLOG_LEVEL_INFO,
                                        "set spanning tree mode"
                                    );
                return XSTP_TYPE_RETURN_ERROR;
        }
        XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
        #endif /* XSTP_TYPE_PROTOCOL_MSTP */

        XSTP_ENGINE_SetMstEnableStatus(FALSE);
        XSTP_MGR_Restart();
        /* +++ LeaveCriticalRegion +++ */
        XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
        XSTP_MGR_StateMachineProgress(XSTP_TYPE_CISTID);
    }
    else
    if (mode == XSTP_TYPE_MSTP_MODE) /* current_st_mode != XSTP_TYPE_MSTP_MODE, mode = XSTP_TYPE_MSTP_MODE */
    {
        /* +++ EnterCriticalRegion +++ */
        XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
        XSTP_MGR_SetMstpMaxHop_(XSTP_TYPE_DEFAULT_BRIDGE_MAXHOP);
        XSTP_OM_SetMaxInstanceNumber((UI32_T)XSTP_TYPE_MAX_INSTANCE_NUM);
        XSTP_OM_SetForceVersion((UI8_T)mode);
        /* +++ LeaveCriticalRegion +++ */
        XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

        /* MST mode */
        #ifdef  XSTP_TYPE_PROTOCOL_MSTP
        if (!SWCTRL_PMGR_SetMstEnableStatus(SWCTRL_MST_ENABLE) )
        {
            if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
            {
                BACKDOOR_MGR_Printf("\r\nXSTP_MGR_SetSystemSpanningTreeVersion: Fatal Error!! Set to MST mode failed.");
            }
            EH_MGR_Handle_Exception1(   SYS_MODULE_XSTP,
                                        XSTP_MGR_SetSystemSpanningTreeVersion_Fun_No,
                                        EH_TYPE_MSG_FAILED_TO,
                                        SYSLOG_LEVEL_INFO,
                                        "set spanning tree mode"
                                    );
                return XSTP_TYPE_RETURN_ERROR;
        }
        #endif /* XSTP_TYPE_PROTOCOL_MSTP */

        XSTP_ENGINE_SetMstEnableStatus(TRUE);

        #ifdef  XSTP_TYPE_PROTOCOL_MSTP
        /* Create other existing instances except CIST if status is enabled */
        if (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
        {
            /* +++ EnterCriticalRegion +++ */
            XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
            XSTP_MGR_RecoverExistingMSTI();
            /* +++ LeaveCriticalRegion +++ */
            XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
        }
        #endif /* XSTP_TYPE_PROTOCOL_MSTP */

        mstid = XSTP_TYPE_CISTID;
        /* +++ EnterCriticalRegion +++ */
        XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
        XSTP_MGR_Restart();
        mstid   = XSTP_TYPE_CISTID;
        om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
        do
        {
            if (om_ptr->instance_exist)
            {
                XSTP_ENGINE_StateMachineProgress(om_ptr);
            }
        }while(XSTP_OM_GetNextInstanceInfoPtr(&mstid, &om_ptr));

        /* +++ LeaveCriticalRegion +++ */
        XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
    }
    else     /* current_st_mode != XSTP_TYPE_MSTP_MODE, mode = XSTP_TYPE_STP_MODE or XSTP_TYPE_RSTP_MODE */
    {
        /* +++ EnterCriticalRegion +++ */
        XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
        XSTP_OM_SetForceVersion((UI8_T)mode);
        XSTP_MGR_Restart();
        /* +++ LeaveCriticalRegion +++ */
        XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

        XSTP_MGR_StateMachineProgress(XSTP_TYPE_CISTID);
    }
    XSTP_MGR_NotifyStpChangeVersion(mode, current_st_status);
    return XSTP_TYPE_RETURN_OK;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetForwardDelay
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the forward_delay time information.
 * INPUT    :   UI32_T forward_delay     -- the forward_delay value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK           -- set successfully
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_INDEX_OOR          -- forward_delay out
 *                                                     of range
 * NOTE     :   1. Time unit is 1/100 sec
 *              2. Range
 *                 -- XSTP_TYPE_MIN_FORWARD_DELAY
 *                 -- XSTP_TYPE_MAX_FORWARD_DELAY
 *              3. Default
 *                 -- XSTP_TYPE_DEFAULT_FORWARD_DELAY
 * REF      :   RFC-1493/dot1dStp 14
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetForwardDelay(UI32_T forward_delay)
{
    UI32_T                  result;
    UI32_T                  sec_forward_delay;
    XSTP_OM_InstanceData_T  *om_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }
    /* check range */
    if( forward_delay < MIN_dot1dStpBridgeForwardDelay || forward_delay > MAX_dot1dStpBridgeForwardDelay )
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetForwardDelay_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Forward-time (4-30");
        return XSTP_TYPE_RETURN_ERROR;
    }

    result              = XSTP_TYPE_RETURN_ERROR;
    sec_forward_delay = forward_delay/XSTP_TYPE_TICK_TIME_UNIT;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    if (om_ptr->instance_exist)
    {
        result = XSTP_MGR_SetMstForwardDelay(om_ptr, sec_forward_delay);
    }
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
    if (result == XSTP_TYPE_RETURN_OK)
    {
        XSTP_MGR_StateMachineProgress(XSTP_TYPE_CISTID);
    }
    return result;
}/* End of XSTP_MGR_SetForwardDelay */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetHelloTime
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the hello_time information.
 * INPUT    :   UI32_T hello_time       -- the hello_time value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR-- not master mode
 *              XSTP_TYPE_RETURN_INDEX_OOR   -- hello_time out of range
 * NOTE     :   1. Time unit is 1/100 sec
 *              2. Range
 *                 -- XSTP_TYPE_MIN_HELLO_TIME
 *                 -- XSTP_TYPE_MAX_HELLO_TIME
 *              3. Default
 *                 -- XSTP_TYPE_DEFAULT_HELLO_TIME
 * * REF    :   RFC-1493/dot1dStp 13
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetHelloTime(UI32_T hello_time)
{
    UI32_T                  result;
    UI32_T                  sec_hello_time;
    XSTP_OM_InstanceData_T  *om_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }
    /* check range */
    if( hello_time < MIN_dot1dStpBridgeHelloTime || hello_time > MAX_dot1dStpBridgeHelloTime )
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetHelloTime_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Hello-time (1-10)");
        return XSTP_TYPE_RETURN_ERROR;
    }

    result              = XSTP_TYPE_RETURN_ERROR;
    sec_hello_time      = hello_time/XSTP_TYPE_TICK_TIME_UNIT;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    if (om_ptr->instance_exist)
    {
        result = XSTP_MGR_SetMstHelloTime(om_ptr, sec_hello_time);
    }
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
    if (result == XSTP_TYPE_RETURN_OK)
    {
        XSTP_MGR_StateMachineProgress(XSTP_TYPE_CISTID);
    }
    return result;
}/* End of XSTP_MGR_SetHelloTime */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMaxAge
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the max_age information.
 * INPUT    :   UI32_T max_age           -- the max_age value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK           -- set successfully
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_INDEX_OOR    -- max_age out of range
 * NOTE     :   1. Time unit is 1/100 sec
 *              2. Range
 *                 -- XSTP_TYPE_MIN_MAXAGE
 *                 -- XSTP_TYPE_MAX_MAXAGE
 *              3. Default
 *                 -- XSTP_TYPE_DEFAULT_MAX_AGE
 * REF      :   RFC-1493/dot1dStp 12
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetMaxAge(UI32_T max_age)
{
    UI32_T                  result;
    UI32_T                  sec_max_age;
    XSTP_OM_InstanceData_T  *om_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }
    /* chack range */
    if( max_age < MIN_dot1dStpBridgeMaxAge || max_age > MAX_dot1dStpBridgeMaxAge )
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMaxAge_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Max-age (6-40)");
        return XSTP_TYPE_RETURN_ERROR;
    }

    result              = XSTP_TYPE_RETURN_ERROR;
    sec_max_age         = max_age/XSTP_TYPE_TICK_TIME_UNIT;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    if (om_ptr->instance_exist)
    {
        result = XSTP_MGR_SetMstMaxAge(om_ptr, sec_max_age);
    }
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    if (result == XSTP_TYPE_RETURN_OK)
    {
        XSTP_MGR_StateMachineProgress(XSTP_TYPE_CISTID);
    }
    return result;
}/* End of XSTP_MGR_SetMaxAge */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPathCostMethod
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the default path cost calculation method.
 * INPUT    :   UI32_T  pathcost_method  -- the method value
 *                      VAL_dot1dStpPathCostDefault_stp8021d1998(1)
 *                      VAL_dot1dStpPathCostDefault_stp8021t2001(2)
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK           -- set successfully
 *              XSTP_TYPE_RETURN_ERROR        -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 * NOTE     :   1. Long
 *                 -- 32-bit based values for default port path costs.
 *                 -- VAL_dot1dStpPathCostDefault_stp8021t2001
 *              2. Short
 *                 -- 16-bit based values for default port path costs.
 *                 -- VAL_dot1dStpPathCostDefault_stp8021d1998
 *              3. Default
 *                 -- The long method.
 * REF      :   draft-ietf-bridge-rstpmib-02/dot1dStp 18
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetPathCostMethod(UI32_T pathcost_method)
{
    UI32_T                  current_method;
    UI32_T                  mstid;
    UI32_T                  lport;
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }
    current_method = XSTP_OM_GetPathCostMethod();
    if ((pathcost_method != XSTP_TYPE_PATH_COST_DEFAULT_SHORT) &&
        (pathcost_method != XSTP_TYPE_PATH_COST_DEFAULT_LONG))
    {
        return XSTP_TYPE_RETURN_ERROR;
    }
    if (current_method==pathcost_method)
    {
        return XSTP_TYPE_RETURN_OK;
    }

    mstid               = XSTP_TYPE_CISTID;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    XSTP_OM_SetPathCostMethod(pathcost_method);

    do
    {
        if (om_ptr->instance_exist)
        {
            lport = 0;
            while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                pom_ptr         = &(om_ptr->port_info[lport-1]);
                if ((pom_ptr->is_member) && (XSTP_OM_RefreshPathCost(om_ptr, lport) ))
                {
                    XSTP_MGR_InitPortPriorityVector(om_ptr, lport);
                }
            } /* End of for (_all_entry_) */
        } /* End of if (_instance_exist_) */
    } while(XSTP_OM_GetNextInstanceInfoPtr(&mstid, &om_ptr));

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    return XSTP_TYPE_RETURN_OK;
} /* End of XSTP_MGR_SetPathCostMethod */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetTransmissionLimit
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the transmission limit count vlaue.
 * INPUT    :   UI32_T  tx_hold_count    -- the TXHoldCount value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK           -- set successfully
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_INDEX_OOR          -- tx_hold_count out
 *                                                     of range
 * NOTE     :   1. The value used by the Port Transmit state machine to
 *                 limit the maximum transmission rate.
 *              2. Range
 *                 -- XSTP_TYPE_MIN_TX_HOLD_COUNT
 *                 -- XSTP_TYPE_MAX_TX_HOLD_COUNT
 *              3. Default
 *                 -- XSTP_TYPE_DEFAULT_TX_HOLD_COUNT
 * REF      :   draft-ietf-bridge-rstpmib-02/dot1dStp 17
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetTransmissionLimit(UI32_T tx_hold_count)
{
    UI32_T                  result;
    XSTP_OM_InstanceData_T  *om_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    result              = XSTP_TYPE_RETURN_ERROR;

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    if (om_ptr->instance_exist)
    {
        result = XSTP_MGR_SetMstTransmissionLimit(om_ptr, tx_hold_count);
    }
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    if (result == XSTP_TYPE_RETURN_OK)
    {
        XSTP_MGR_StateMachineProgress(XSTP_TYPE_CISTID);
    }
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetSystemGlobalPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the global priority value
 *              when the switch is MST mode.
 * INPUT    :   UI32_T  priority         -- the priority value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK           -- set successfully
 *              XSTP_TYPE_RETURN_ERROR        -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *              XSTP_TYPE_RETURN_INDEX_OOR    -- priority out of range
 * NOTE     :   1. When mode is STP or RSTP
 *                 -- Only set the priority value for mstid = 0.
 *              2. When mode is MSTP
 *                 -- Set the priority value for all MST instances.
 *              3. Range :  0 ~ 61440 (in steps of 4096)
 *                          XSTP_TYPE_MIN_BRIDGE_PRIORITY
 *                          XSTP_TYPE_MAX_BRIDGE_PRIORITY_RSTP_MSTP
 *              4. DEFAULT  : 32768
 *                 -- XSTP_TYPE_DEFAULT_BRIDGE_PRIORITY
 * REF      :   RFC-1493/dot1dStp 2
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetSystemGlobalPriority(UI32_T priority)
{
    UI32_T                              mstid;
    UI32_T                              result;
    XSTP_OM_InstanceData_T              *om_ptr;
    BOOL_T                              iterative;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    iterative           = TRUE;
    result              = XSTP_TYPE_RETURN_ERROR;
    mstid               = XSTP_TYPE_CISTID;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    do
    {
        if (om_ptr->instance_exist)
        {
            result = XSTP_MGR_SetMstPriority_(om_ptr, priority, TRUE);
            if ( result != XSTP_TYPE_RETURN_OK)
            {
                iterative = FALSE;
            }
        }
        else
        {
            /* System Spanning Tree Status is disabled */
            if (mstid == XSTP_TYPE_CISTID)
            {
                result      = XSTP_MGR_SetMstPriority_(om_ptr, priority, TRUE);
                iterative   = FALSE;
            }
        }
    }while(XSTP_OM_GetNextInstanceInfoPtr(&mstid, &om_ptr) && iterative);

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetSystemBridgePriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the bridge priority value.
 * INPUT    :   UI32_T  priority         -- the priority value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK           -- set successfully
 *              XSTP_TYPE_RETURN_ERROR        -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *              XSTP_TYPE_RETURN_INDEX_OOR    -- priority out of range
 * NOTE     :   1. Only set the priority value for mstid = 0.
 *              2. Range :  0 ~ 61440 (in steps of 4096)
 *                          XSTP_TYPE_MIN_BRIDGE_PRIORITY
 *                          XSTP_TYPE_MAX_BRIDGE_PRIORITY_RSTP_MSTP
 *              3. DEFAULT  : 32768
 *                 -- XSTP_TYPE_DEFAULT_BRIDGE_PRIORITY
 * REF      :   RFC-1493/dot1dStp 2
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetSystemBridgePriority(UI32_T priority)
{
    UI32_T                              result;
    XSTP_OM_InstanceData_T              *om_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    result              = XSTP_TYPE_RETURN_ERROR;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    result              = XSTP_MGR_SetMstPriority_(om_ptr, priority, TRUE);
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    return result;
}/* End of XSTP_MGR_SetSystemBridgePriority() */

#ifdef  XSTP_TYPE_PROTOCOL_RSTP
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPortPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the path_cost of the port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T path_cost        -- the path_cost value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   1. When mode is STP or RSTP
 *                 -- For mstid = 0
 *                    Set the path cost value to the specified port.
 *              2. When mode is MSTP
 *                 -- For all instances
 *                    Set the path cost value to the specified port.
 *              3. In long mode (VAL_dot1dStpPathCostDefault_stp8021t2001)
 *                 -- Range : 0 ~ 200000000
 *                    XSTP_TYPE_MIN_PORT_PATH_COST
 *                    XSTP_TYPE_MAX_PORT_PATH_COST_32
 *              4. In short mode (VAL_dot1dStpPathCostDefault_stp8021d1998)
 *                 -- Range : 0 ~ 65535
 *                    XSTP_TYPE_MIN_PORT_PATH_COST
 *                    XSTP_TYPE_MAX_PORT_PATH_COST_16
 * REF      :   RFC-1493/dot1dStpPortEntry 5
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetPortPathCost(UI32_T lport,
                                UI32_T path_cost)
{
    UI32_T                  mstid;
    UI32_T                  result;
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  iterative;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetPortPathCost_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
        return XSTP_TYPE_RETURN_PORTNO_OOR;
    }

    iterative           = TRUE;
    result              = XSTP_TYPE_RETURN_ERROR;
    mstid               = XSTP_TYPE_CISTID;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    do
    {
        if (om_ptr->instance_exist)
        {
            result = XSTP_MGR_SetMstPortPathCost_(om_ptr, lport, path_cost);
            if ( result != XSTP_TYPE_RETURN_OK)
            {
                iterative = FALSE;
            }
        }
        else
        {
            /* System Spanning Tree Status is disabled */
            if (mstid == XSTP_TYPE_CISTID)
            {
                result      = XSTP_MGR_SetMstPortPathCost_(om_ptr, lport, path_cost);
                iterative   = FALSE;
            }
        }
    }while(XSTP_OM_GetNextInstanceInfoPtr(&mstid, &om_ptr) && iterative);

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    return result;
}
#endif /* XSTP_TYPE_PROTOCOL_RSTP */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetRunningMstPortPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the path_cost of the port.
 * INPUT    :   UI32_T lport            -- lport number
                UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *path_cost       -- pointer of the path_cost value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * REF      :   RFC-1493/dot1dStpPortEntry 5
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_GetRunningMstPortPathCost(UI32_T lport,
                                          UI32_T mstid,
                                          UI32_T *path_cost)
{
    UI32_T                  default_path_cost;
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    char                    arg_buf[20];
    BOOL_T                  static_path_cost_flag;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
        snprintf(arg_buf, 20, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_GetRunningMstPortPathCost_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_GetRunningMstPortPathCost_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(mstid);
    om_ptr = XSTP_OM_GetInstanceInfoPtr(mstid);
    pom_ptr = &(om_ptr->port_info[lport-1]);

    #ifdef  XSTP_TYPE_PROTOCOL_RSTP
    *path_cost              = (UI32_T)pom_ptr->port_path_cost;
    static_path_cost_flag   = pom_ptr->static_path_cost;
    #endif /* XSTP_TYPE_PROTOCOL_RSTP */

    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    *path_cost              = (UI32_T)pom_ptr->internal_port_path_cost;
    static_path_cost_flag   = pom_ptr->static_internal_path_cost;
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    XSTP_OM_GetLportDefaultPathCost(lport, &default_path_cost);
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(mstid);

    if (!static_path_cost_flag)
    {
        if (*path_cost == default_path_cost)
        {
                return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        }
        else
        {
                return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}/* XSTP_MGR_GetRunningMstPortPathCost */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetMstPortPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the path_cost of the port.
 * INPUT    :   UI32_T lport            -- lport number
                UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *path_cost       -- pointer of the path_cost value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * REF      :   RFC-1493/dot1dStpPortEntry 5
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_MGR_GetMstPortPathCost(UI32_T lport,
                                   UI32_T mstid,
                                   UI32_T *path_cost)
{
    UI32_T                              result;

    result = XSTP_MGR_GetRunningMstPortPathCost(lport, mstid, path_cost);
    if (result == SYS_TYPE_GET_RUNNING_CFG_FAIL)
    {
        return FALSE;
    }
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetLportDefaultPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the specified lport path cost.
 * INPUT    : UI32_T lport              -- the lport number
 * OUTPUT   : UI32_T *path_cost         -- pointer of the path cost value
 * RETURN   : XSTP_TYPE_RETURN_OK       -- OK
 *            XSTP_TYPE_RETURN_ERROR    -- failed
 * NOTE     : This value is calculated from specification.
 *-------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_GetLportDefaultPathCost(UI32_T lport,
                                        UI32_T *path_cost)
{
    UI32_T  cost;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if ((SWCTRL_LogicalPortExisting( (UI32_T)lport )==TRUE) &&
        (XSTP_OM_GetLportDefaultPathCost(lport, &cost) == XSTP_TYPE_RETURN_OK))
    {
        *path_cost = cost;
        return XSTP_TYPE_RETURN_OK;
    }
    else
    {
        return XSTP_TYPE_RETURN_ERROR;
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the port priority value.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 *              UI32_T priority         -- the priority value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- lport number out of range
 *              XSTP_TYPE_RETURN_INDEX_OOR   -- priority out of range
 * NOTE     :   1. When mode is STP or RSTP
 *                 -- For mstid = 0
 *                    Set the priority value to the specified port.
 *              2. When mode is MSTP
 *                 -- For all instances
 *                    Set the priority value to the specified port.
 *              3. Range :  0 ~ 240 (in steps of 16)
 *                          XSTP_TYPE_MIN_PORT_PRIORITY
 *                          XSTP_TYPE_MAX_PORT_PRIORITY_RSTP_MSTP
  *              4. Default             : 128
 *                 -- XSTP_TYPE_DEFAULT_PORT_PRIORITY
 * REF      :   RFC-1493/dot1dStpPortEntry 2
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetPortPriority(UI32_T lport,
                                UI32_T priority)
{
    UI32_T                  mstid;
    UI32_T                  result;
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  iterative;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetPortPriority_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
        return XSTP_TYPE_RETURN_PORTNO_OOR;
    }

    iterative           = TRUE;
    result              = XSTP_TYPE_RETURN_ERROR;
    mstid               = XSTP_TYPE_CISTID;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    do
    {
        if (om_ptr->instance_exist)
        {
            result = XSTP_MGR_SetMstPortPriority_(om_ptr, lport, priority);
            if ( result != XSTP_TYPE_RETURN_OK)
            {
                iterative = FALSE;
            }
        }
        else
        {   /* System Spanning Tree Status is disabled */
            if (mstid == XSTP_TYPE_CISTID)
            {
                result      = XSTP_MGR_SetMstPortPriority_(om_ptr, lport, priority);
                iterative   = FALSE;
            }
        }
    }while(XSTP_OM_GetNextInstanceInfoPtr(&mstid, &om_ptr) && iterative);

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPortLinkTypeMode
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set a link type for a port when mode is RSTP or MSTP.
 * INPUT    :   UI32_T lport -- lport number
 *              UI32_T mode  -- the status value
 *                           VAL_dot1dStpPortAdminPointToPoint_forceTrue(0)
 *                           VAL_dot1dStpPortAdminPointToPoint_forceFalse(1)
 *                           VAL_dot1dStpPortAdminPointToPoint_auto(2)
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK              -- set successfully
 *              XSTP_TYPE_RETURN_ERROR           -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_INDEX_OOR       -- mstid out of range
 *              XSTP_TYPE_RETURN_PORTNO_OOR      -- port number out of range
 * NOTE     :   Default value
 *              -- VAL_dot1dStpPortAdminPointToPoint_auto
 * REF      :   draft-ietf-bridge-rstpmib-02/dot1dStpExtPortEntry 4
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetPortLinkTypeMode(UI32_T lport,
                                    UI32_T mode)
{
    UI32_T                  current_mode = XSTP_TYPE_PORT_ADMIN_LINK_TYPE_AUTO;
    UI32_T                  mstid;
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetPortLinkTypeMode_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
        return XSTP_TYPE_RETURN_PORTNO_OOR;
    }

    if (mode != XSTP_TYPE_PORT_ADMIN_LINK_TYPE_POINT_TO_POINT && mode != XSTP_TYPE_PORT_ADMIN_LINK_TYPE_SHARED && mode != XSTP_TYPE_PORT_ADMIN_LINK_TYPE_AUTO)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr= &(om_ptr->port_info[lport-1]);
    if (pom_ptr->common->admin_point_to_point_mac_auto == TRUE)
    {
        current_mode = XSTP_TYPE_PORT_ADMIN_LINK_TYPE_AUTO;
    }
    else if (pom_ptr->common->admin_point_to_point_mac == TRUE)
    {
        current_mode = XSTP_TYPE_PORT_ADMIN_LINK_TYPE_POINT_TO_POINT;
    }
    else if (pom_ptr->common->admin_point_to_point_mac == FALSE)
    {
        current_mode = XSTP_TYPE_PORT_ADMIN_LINK_TYPE_SHARED;
    }
    if (current_mode == mode)
    {
        /* +++ LeaveCriticalRegion +++ */
        XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
        return XSTP_TYPE_RETURN_OK;
    }
    if (mode == XSTP_TYPE_PORT_ADMIN_LINK_TYPE_AUTO)
    {
        if (XSTP_MGR_PortStatusIsFullDuplex(lport)==TRUE)
        {
            pom_ptr->common->oper_point_to_point_mac    = TRUE;
        }
        else
        {
            pom_ptr->common->oper_point_to_point_mac    = FALSE;
        }
        pom_ptr->common->admin_point_to_point_mac       = FALSE;
        pom_ptr->common->admin_point_to_point_mac_auto  = TRUE;
    }
    else if (mode == XSTP_TYPE_PORT_ADMIN_LINK_TYPE_POINT_TO_POINT)
    {
        pom_ptr->common->oper_point_to_point_mac        = TRUE;
        pom_ptr->common->admin_point_to_point_mac       = TRUE;
        pom_ptr->common->admin_point_to_point_mac_auto  = FALSE;
    }
    else
    {
        pom_ptr->common->oper_point_to_point_mac        = FALSE;
        pom_ptr->common->admin_point_to_point_mac       = FALSE;
        pom_ptr->common->admin_point_to_point_mac_auto  = FALSE;
    }

    mstid               = XSTP_TYPE_CISTID;
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    do
    {
         if (om_ptr->instance_exist)
         {
            XSTP_ENGINE_StateMachineProgress(om_ptr);
         }
    }while(XSTP_OM_GetNextInstanceInfoPtr(&mstid, &om_ptr));
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
    return XSTP_TYPE_RETURN_OK;
}/* End of XSTP_MGR_SetPortLinkTypeMode */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPortProtocolMigration
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set protocol_migration status for a port.
 * INPUT    :   UI32_T lport -- lport number
 *              UI32_T mode  -- the mode value
 *                              VAL_dot1dStpPortProtocolMigration_true
 *                              VAL_dot1dStpPortProtocolMigration_false
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK              -- set successfully
 *              XSTP_TYPE_RETURN_ERROR           -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_INDEX_OOR       -- mstid out of range
 *              XSTP_TYPE_RETURN_PORTNO_OOR      -- port number out of range
 * NOTE     :   Default value
 *              -- FALSE
 *                 XSTP_TYPE_DEFAULT_PORT_PROTOCOL_MIGRATION_STATUS
 * REF      :   draft-ietf-bridge-rstpmib-02/dot1dStpExtPortEntry 1
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetPortProtocolMigration(UI32_T lport,
                                         UI32_T mode)
{
    BOOL_T                  temp_mode;
    BOOL_T                  current_mcheck;
    UI32_T                  mstid;
    UI32_T                  current_force_version;
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetPortProtocolMigration_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
        return XSTP_TYPE_RETURN_PORTNO_OOR;
    }
    if (mode == XSTP_TYPE_PORT_PROTOCOL_MIGRATION_ENABLED)
    {
        temp_mode = TRUE;
    }
    else if (mode == XSTP_TYPE_PORT_PROTOCOL_MIGRATION_DISABLED)
    {
        temp_mode = FALSE;
    }
    else
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    current_force_version = (UI32_T)XSTP_OM_GetForceVersion();
    if ((current_force_version < 2) && (temp_mode==TRUE))
    {
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
            BACKDOOR_MGR_Printf("\r\nXSTP_MGR_SetPortProtocolMigration::ERROR!! The value can't be set TRUE if the value of ForceVersion is less than 2");
        return XSTP_TYPE_RETURN_ERROR;
    }

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);
    current_mcheck = pom_ptr->common->mcheck;
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    if (current_mcheck == temp_mode)
    {
        return XSTP_TYPE_RETURN_OK;
    }

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    pom_ptr->common->mcheck = temp_mode;
    mstid                   = XSTP_TYPE_CISTID;
    om_ptr                  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    do
    {
        if (om_ptr->instance_exist)
        {
            XSTP_ENGINE_StateMachineProgress(om_ptr);
        }
    }while(XSTP_OM_GetNextInstanceInfoPtr(&mstid, &om_ptr));
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
    return XSTP_TYPE_RETURN_OK;
}/* End of XSTP_MGR_SetPortProtocolMigration */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPortAdminEdgePort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set edge_port status for a port.
 * INPUT    :   UI32_T lport -- lport number
 *              UI32_T mode  -- the mode value
 *                              VAL_dot1dStpPortAdminEdgePort_true
 *                              VAL_dot1dStpPortAdminEdgePort_false
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK              -- set successfully
 *              XSTP_TYPE_RETURN_ERROR           -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_INDEX_OOR       -- mstid out of range
 *              XSTP_TYPE_RETURN_PORTNO_OOR      -- port number out of range
 * NOTE     :   Default value
 *              -- FALSE
 *                 XSTP_TYPE_DEFAULT_PORT_ADMIN_EDGE_PORT
 * REF      :   draft-ietf-bridge-rstpmib-02/dot1dStpExtPortEntry 1
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetPortAdminEdgePort(UI32_T lport,
                                     UI32_T mode)
{
    BOOL_T                  current_admin_mode, current_oper_mode, current_auto_mode, temp_mode;
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    BOOL_T                  current_bpdu_guard;
#endif
#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
    BOOL_T                  current_bpdu_filter;
#endif
    UI32_T                  mstid;
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetPortAdminEdgePort_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
        return XSTP_TYPE_RETURN_PORTNO_OOR;
    }
    if (mode == XSTP_TYPE_PORT_ADMIN_EDGE_PORT_ENABLED)
    {
        temp_mode = TRUE;
    }
    else if (mode == XSTP_TYPE_PORT_ADMIN_EDGE_PORT_DISABLED)
    {
        temp_mode = FALSE;
    }
    else if (mode == XSTP_TYPE_PORT_ADMIN_EDGE_PORT_AUTO)
    {
        temp_mode = TRUE;
    }
    else
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);
    current_admin_mode = pom_ptr->common->admin_edge;
    current_oper_mode = pom_ptr->common->oper_edge;
    current_auto_mode = pom_ptr->common->auto_edge;
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    current_bpdu_guard = pom_ptr->common->bpdu_guard_status;
#endif
#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
    current_bpdu_filter = pom_ptr->common->bpdu_filter_status;
#endif
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    if ((mode == XSTP_TYPE_PORT_ADMIN_EDGE_PORT_AUTO) && (current_auto_mode == temp_mode))
    {
        return XSTP_TYPE_RETURN_OK;
    }
    else if (mode != XSTP_TYPE_PORT_ADMIN_EDGE_PORT_AUTO)
    {
        if (    (current_auto_mode == FALSE)
             && (current_admin_mode == temp_mode) && (current_admin_mode == current_oper_mode)
           )
        {
            return XSTP_TYPE_RETURN_OK;
        }
    }

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);

    if (mode == XSTP_TYPE_PORT_ADMIN_EDGE_PORT_AUTO)
    {
        pom_ptr->common->auto_edge = TRUE;
        pom_ptr->common->admin_edge = FALSE;
    }
    else
    {
        pom_ptr->common->auto_edge = FALSE;
        pom_ptr->common->admin_edge = temp_mode;
    }
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    if ((temp_mode == FALSE) && (current_bpdu_guard == TRUE))
    {
        pom_ptr->common->bpdu_guard_status = FALSE;
    }
#endif
#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
    if ((temp_mode == FALSE) && (current_bpdu_filter == TRUE))
    {
        pom_ptr->common->bpdu_filter_status = FALSE;
    }
#endif

    mstid                       = XSTP_TYPE_CISTID;
    om_ptr                      = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    do
    {
        if (om_ptr->instance_exist)
        {
            XSTP_ENGINE_StateMachineProgress(om_ptr);
        }
    } while (XSTP_OM_GetNextInstanceInfoPtr(&mstid, &om_ptr));
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
    return XSTP_TYPE_RETURN_OK;
}/* End of XSTP_MGR_SetPortAdminEdgePort */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetDot1dMstEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified spanning tree
 *              entry info can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   UI32_T mstid    -- instance value
 * OUTPUT   :   entry           -- the specified mst entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_MGR_GetDot1dMstEntry(UI32_T mstid,
                                 XSTP_MGR_Dot1dStpEntry_T  *entry)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  result;
    char                    arg_buf[20];

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
        snprintf(arg_buf, 20, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_GetDot1dMstEntry_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
        return FALSE;
    }

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(mstid);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
    result              = XSTP_MGR_GetDot1dMstEntry_(om_ptr, entry);
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(mstid);
    return result;

}/* End of XSTP_MGR_GetDot1dMstEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetNextDot1dMstEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next spanning tree
 *              entry info can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid           -- instance value
 * OUTPUT   :   entry           -- the specified mst entry info
 *              mstid           -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_MGR_GetNextDot1dMstEntry(UI32_T *mstid,
                                     XSTP_MGR_Dot1dStpEntry_T  *entry)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  found;
    BOOL_T                  result;
    char                    arg_buf[20];

    /*SYSFUN_USE_CSC(FALSE);*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    if ( (*mstid < 0 || *mstid > XSTP_TYPE_MAX_MSTID) && (*mstid!=XSTP_MSTP_GET_FIRST_INSTANCE_FOR_SNMP))
    {
        snprintf(arg_buf, 20, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_GetNextDot1dMstEntry_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
        return FALSE;
    }
    result  = FALSE;
    found   = FALSE;
    /* +++ EnterCriticalRegion +++ */
     XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    if (*mstid == XSTP_MSTP_GET_FIRST_INSTANCE_FOR_SNMP)
    {
        *mstid  = 0;
        found   = TRUE;
    }
    else if (XSTP_MGR_GetNextExistingInstance_(mstid))
    {
        found   = TRUE;
    }
    if (found)
    {
        om_ptr              = XSTP_OM_GetInstanceInfoPtr(*mstid);
        result              = XSTP_MGR_GetDot1dMstEntry_(om_ptr, entry);
    }
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetDot1dBaseEntry
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
BOOL_T XSTP_MGR_GetDot1dBaseEntry(XSTP_MGR_Dot1dBaseEntry_T *base_entry)
{
    UI32_T      total_port_num;

    /*SYSFUN_USE_CSC(FALSE);*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (!SWCTRL_GetCpuMac(base_entry->dot1d_base_bridge_address))
    {
        /* Error */
        return FALSE;
    }
    total_port_num = SWCTRL_GetLogicalPortNumber();
    base_entry->dot1d_base_num_ports        = (UI16_T)total_port_num;
    base_entry->dot1d_base_type             = VAL_dot1dBaseType_transparent_only;

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetDot1dBasePortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified base port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   base_port_entry->dot1d_base_port
 *                                          -- key to specify a unique base
 *                                             entry
 * OUTPUT   :   base_port_entry             -- base entry info of specified
 *                                             key
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_MGR_GetDot1dBasePortEntry(XSTP_MGR_Dot1dBasePortEntry_T *base_port_entry)
{
    UI32_T              lport;
    SWDRV_RmonStats_T   rmon_stats;

    /*SYSFUN_USE_CSC(FALSE);*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    lport = base_port_entry->dot1d_base_port;
    if (!NMTR_PMGR_GetRmonStats(lport, &rmon_stats) )
    {
        /* Error */
        return FALSE;
    }
    base_port_entry->dot1d_base_port_if_index   = base_port_entry->dot1d_base_port;
    base_port_entry->dot1d_base_port_circuit[0] = 0;
    base_port_entry->dot1d_base_port_circuit[1] = 0;
    base_port_entry->dot1d_base_port_delay_exceeded_discards    = 0;
    base_port_entry->dot1d_base_port_mtu_exceeded_discards
                                                =   rmon_stats.etherStatsOversizePkts
                                                  + rmon_stats.etherStatsJabbers;

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME -- XSTP_MGR_GetNextDot1dBasePortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next available base port
 *              entry info can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   base_port_entry->dot1d_base_port
 *                                          -- key to specify a unique base
 *                                             entry
 * OUTPUT   :   base_port_entry             -- xstp entry info of specified
 *                                             key
 * RETURN   :   TRUE/FALSE
 * NOTES    :   If next available stp entry is available, the
 *              base_port_entry->dot1d_base_port will be updated and the
 *              entry info will be retrieved from the table.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_MGR_GetNextDot1dBasePortEntry(XSTP_MGR_Dot1dBasePortEntry_T *base_port_entry)
{
    UI32_T             lport;
    SWDRV_RmonStats_T  rmon_stats;

    /*SYSFUN_USE_CSC(FALSE);*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    lport = base_port_entry->dot1d_base_port;
    if (    (SWCTRL_GetNextLogicalPort(&lport) == SWCTRL_LPORT_UNKNOWN_PORT)
        ||  (!NMTR_PMGR_GetRmonStats(lport, &rmon_stats))
       )
    {
        /* Error */
        return FALSE;
    }
    base_port_entry->dot1d_base_port            = (UI16_T)lport;
    base_port_entry->dot1d_base_port_if_index   = base_port_entry->dot1d_base_port;
    base_port_entry->dot1d_base_port_circuit[0] = 0;
    base_port_entry->dot1d_base_port_circuit[1] = 0;
    base_port_entry->dot1d_base_port_delay_exceeded_discards    = 0;
    base_port_entry->dot1d_base_port_mtu_exceeded_discards
                                                =   rmon_stats.etherStatsOversizePkts
                                                  + rmon_stats.etherStatsJabbers;

    return TRUE;
}


/*-------------------------------------------------------------------------
 * The following Functions only provide for MSTP
 *-------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMstPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the priority of the specified MST instance
 *              when the switch is MST mode.
 * INPUT    :   UI32_T  priority         -- the priority value
 *              UI32_T  mstid            -- instance value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK           -- set successfully
 *              XSTP_TYPE_RETURN_ERROR        -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_INDEX_OOR    -- mstid and priority out of range
 * NOTE     :   1. Range    : 0 ~ 61440 (in steps of 4096)
 *                 -- XSTP_TYPE_MIN_BRIDGE_PRIORITY
 *                 -- XSTP_TYPE_MAX_BRIDGE_PRIORITY_RSTP_MSTP
 *              2. DEFAULT  : 32768
 *                 -- XSTP_TYPE_DEFAULT_BRIDGE_PRIORITY
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetMstPriority(UI32_T mstid,
                               UI32_T priority)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    UI32_T                  result;
    char                    arg_buf[20];

    /*SYSFUN_USE_CSC(FALSE);*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
        snprintf(arg_buf, 20, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstPriority_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }
    result              = XSTP_TYPE_RETURN_ERROR;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(mstid);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
    result              = XSTP_MGR_SetMstPriority_(om_ptr, priority, TRUE);
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(mstid);
    return result;

}/* End of XSTP_MGR_SetMstPriority */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMstPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the port priority for the specified spanning tree
 *              when mode is MSTP.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 *              UI32_T priority         -- the priority value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- lport number out of range
 *              XSTP_TYPE_RETURN_INDEX_OOR   -- mstid and priority out of range
 * NOTE     :   1. Range    : 0 ~ 240 (in steps of 16)
 *                 -- XSTP_TYPE_MIN_PORT_PRIORITY
 *                 -- XSTP_TYPE_MAX_PORT_PRIORITY_RSTP_MSTP
 *              2. Default  : 128
 *                 -- XSTP_TYPE_DEFAULT_PORT_PRIORITY
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetMstPortPriority(UI32_T lport,
                                   UI32_T mstid,
                                   UI32_T priority)
{

    XSTP_OM_InstanceData_T  *om_ptr;
    UI32_T                  result;
    char                    arg_buf[20];

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }
    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
        snprintf(arg_buf, 20, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstPortPriority_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstPortPriority_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
        return XSTP_TYPE_RETURN_PORTNO_OOR;
    }

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(mstid);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
    result              = XSTP_MGR_SetMstPortPriority_(om_ptr, lport, priority); /* XXX_0917 */
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(mstid);
    return result;

}/* End of XSTP_MGR_SetMstPortPriority() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMstPortPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the path_cost of the port for specified spanning tree
 *              when mode is MSTP.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 *              UI32_T path_cost        -- the path_cost value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURNPORTNO_OOR  -- port number out of range
 *              XSTP_TYPE_RETURNINDEX_OOR   -- mstid and priority out of range
 * NOTE     :  1. In long mode (VAL_dot1dStpPathCostDefault_stp8021t2001)
 *                -- Range : 0 ~ 200000000
 *                   XSTP_TYPE_MIN_PORT_PATH_COST
 *                   XSTP_TYPE_MAX_PORT_PATH_COST_32
 *             2. In short mode (VAL_dot1dStpPathCostDefault_stp8021d1998)
 *                -- Range : 0 ~ 65535
 *                   XSTP_TYPE_MIN_PORT_PATH_COST
 *                   XSTP_TYPE_MAX_PORT_PATH_COST_16
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetMstPortPathCost(UI32_T lport,
                                   UI32_T mstid,
                                   UI32_T path_cost)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    UI32_T                  result;
    char                    arg_buf[20];

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }
    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
        snprintf(arg_buf, 20, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstPortPathCost_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstPortPathCost_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
        return XSTP_TYPE_RETURN_PORTNO_OOR;
    }
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(mstid);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
    result              = XSTP_MGR_SetMstPortPathCost_(om_ptr, lport, path_cost);
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(mstid);
    return result;

}/* End of XSTP_MGR_SetMstPortPathCost */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetVlanListToMstConfigTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   Map a vlan_list to an instance for mst configuration table.
 * INPUT    :   UI32_T mstid      -- instance value
 *              UI32_T range      -- range value
 *                                   XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_1K
 *                                   XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_2K
 *                                   XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_3K
 *                                   XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_4K
 *              UI8_T *vlan_list  -- point of vlan_list
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK             -- set successfully
 *              XSTP_TYPE_RETURN_ERROR          -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_ST_STATUS_DISABLED -- status is disabled
 *              XSTP_TYPE_RETURN_INDEX_OOR      -- mstid out of range
 *              XSTP_TYPE_RETURN_INDEX_NEX      -- vlan not existed
 * NOTE     :   For SNMP (MSB)
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetVlanListToMstConfigTable(UI32_T mstid,
                                            UI32_T range,
                                            UI8_T *vlan_list)
{
    UI32_T                  vlan;
    UI16_T                  min_byte, max_byte;
    UI16_T                  i, j;
    UI32_T                  result;
    BOOL_T                  iterative;
    char                    arg_buf[20];

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
        snprintf(arg_buf, 20, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetVlanListToMstConfigTable_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }

    switch(range)
    {
        case XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_1K:
            min_byte = 0;
            max_byte = 127;
            break;
        case XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_2K:
            min_byte = 128;
            max_byte = 255;
            break;
        case XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_3K:
            min_byte = 256;
            max_byte = 383;
            break;
        case XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_4K:
            min_byte = 384;
            max_byte = 511;
            break;
        default:
                return  XSTP_TYPE_RETURN_ERROR;
    }
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(mstid);
    iterative           = TRUE;
    result              = XSTP_TYPE_RETURN_ERROR;
    for (i=min_byte; i<=max_byte&&iterative; i++)
    {
        for (j=0; j<8&&iterative; j++)
        {
            if ( (vlan_list[i-min_byte] & (0x01<<j))!=0)
            {
                vlan = ((i*8)+(7-j));

                result = XSTP_MGR_SetVlanToMstConfigTable_(mstid, vlan);
                if (result != XSTP_TYPE_RETURN_OK)
                {
                    iterative = FALSE;
                }
            }
        }
    }

#ifdef  XSTP_TYPE_PROTOCOL_MSTP
    /* All CIST and MSTI transit to the initial state */
    XSTP_MGR_Restart();
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(mstid);
    return result;
}/* End of XSTP_MGR_SetVlanListToMstConfigTable*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_AttachVlanListToMstConfigTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   Attach the vlan(s) to the new instance.
 * INPUT    :   UI32_T mstid      -- instance value
 *              UI32_T range      -- range value
 *                                   XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_1K
 *                                   XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_2K
 *                                   XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_3K
 *                                   XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_4K
 *              UI8_T *vlan_list  -- point of vlan_list
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK             -- set successfully
 *              XSTP_TYPE_RETURN_ERROR          -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_ST_STATUS_DISABLED -- status is disabled
 *              XSTP_TYPE_RETURN_INDEX_OOR      -- mstid out of range
 *              XSTP_TYPE_RETURN_INDEX_NEX      -- vlan not existed
 * NOTE     :   1. All vlans will join MST instance 0 by default.
 *              2. This API will automatically move this VLAN from old instance to new instance.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_AttachVlanListToMstConfigTable(UI32_T mstid,
                                               UI32_T range,
                                               UI8_T *vlan_list)
{
    UI32_T                  vlan;
    UI16_T                  min_byte, max_byte;
    UI16_T                  i, j;
    UI32_T                  result;
    BOOL_T                  iterative;
    UI32_T                  old_mstid;
    BOOL_T                  changed;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    switch(range)
    {
        case XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_1K:
            min_byte = 0;
            max_byte = 127;
            break;
        case XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_2K:
            min_byte = 128;
            max_byte = 255;
            break;
        case XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_3K:
            min_byte = 256;
            max_byte = 383;
            break;
        case XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_4K:
            min_byte = 384;
            max_byte = 511;
            break;
        default:
                return  XSTP_TYPE_RETURN_ERROR;
    }
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(mstid);
    iterative           = TRUE;
    result              = XSTP_TYPE_RETURN_OK;
    changed             = FALSE;
    for (i=min_byte; i<=max_byte&&iterative; i++)
    {
        for (j=0; j<8&&iterative; j++)
        {
            vlan    = ((i*8)+(7-j));
            if (vlan == 0)
                continue;

            if ( (vlan_list[i-min_byte] & (0x01<<j))!=0)
            {
                result  = XSTP_MGR_AttachVlanToMstConfigTable_(mstid, vlan);
                if (result != XSTP_TYPE_RETURN_OK)
                {
                    iterative = FALSE;
                }
                else
                {
                    changed = TRUE;
                }
            }
            else
            {
                XSTP_OM_GetMstidFromMstConfigurationTableByVlan(vlan, &old_mstid);
                if (old_mstid == mstid)
                {
                    result  = XSTP_MGR_AttachVlanToMstConfigTable_(XSTP_TYPE_CISTID, vlan);
                    if (result != XSTP_TYPE_RETURN_OK)
                    {
                        iterative = FALSE;
                    }
                    else
                    {
                        changed = TRUE;
                    }
                }
            }
        }
    }
    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    /* Generate new configuration digest and restart all stat machines */
    if (changed == TRUE)
    {
        XSTP_OM_InstanceData_T  *om_ptr;
        om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
        XSTP_OM_GenerateConfigurationDigest(om_ptr);
        XSTP_MGR_Restart();
    }
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(mstid);
    return result;
}/* End of XSTP_MGR_AttachVlanListToMstConfigTable() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetVlanToMstConfigTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   Map vlan to an instance for mst configuration table.
 * INPUT    :   UI32_T mstid               -- instance value
 *              UI32_T vlan                -- vlan value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
 *              XSTP_TYPE_RETURN_ERROR              -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_ST_STATUS_DISABLED -- status is disabled
 *              XSTP_TYPE_RETURN_INDEX_OOR          -- mstid out of range
 *              XSTP_TYPE_RETURN_INDEX_NEX          -- vlan not existed
 * NOTE     :   1. All vlans will join MST instance 0 by default.
 *              2. This function is for the vlan which is attached to the MST instance 0.
 * ------------------------------------------------------------------------
 */

static UI32_T restart_xstp_timer = 0;

void  XSTP_MGR_Restart(){
    restart_xstp_timer = 300/XSTP_TYPE_TIMER_TICKS2SEC;
    return ;
}

void  XSTP_MGR_Decrease(void){
    if(restart_xstp_timer){

        restart_xstp_timer--;
        if(restart_xstp_timer == 0){
#ifdef XSTP_TYPE_PROTOCOL_MSTP
            XSTP_ENGINE_RestartStateMachine();
#endif /* #ifdef XSTP_TYPE_PROTOCOL_MSTP */
        }

    }
    return ;
}

UI32_T  XSTP_MGR_SetVlanToMstConfigTable(UI32_T mstid, UI32_T vlan)
{
    UI32_T                  result;
    char                    arg_buf[20];

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }
    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
        snprintf(arg_buf, 20, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetVlanToMstConfigTable_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(mstid);
    result              = XSTP_MGR_SetVlanToMstConfigTable_(mstid, vlan);

    XSTP_MGR_Restart();

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(mstid);
    return result;

}/* End of XSTP_MGR_SetVlanToMstConfigTable */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_AttachVlanToMstConfigTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   Attach the vlan to the new instance.
 * INPUT    :   UI32_T mstid               -- instance value
 *              UI32_T vlan                -- vlan value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
 *              XSTP_TYPE_RETURN_ERROR              -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_ST_STATUS_DISABLED -- status is disabled
 *              XSTP_TYPE_RETURN_INDEX_OOR          -- mstid out of range
 *              XSTP_TYPE_RETURN_INDEX_NEX          -- vlan not existed
 * NOTE     :   1. All vlans will join MST instance 0 by default.
 *              2. This API will automatically move this VLAN from old instance to new instance.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_AttachVlanToMstConfigTable(UI32_T mstid,
                                           UI32_T vlan)
{
    UI32_T                  result;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(mstid);
    result              = XSTP_MGR_AttachVlanToMstConfigTable_(mstid, vlan);
    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    /* Generate new configuration digest and restart all stat machines */
    if (result == XSTP_TYPE_RETURN_OK)
    {
        XSTP_OM_InstanceData_T  *om_ptr;
        om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
        XSTP_OM_GenerateConfigurationDigest(om_ptr);
        XSTP_MGR_Restart();
    }
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(mstid);
    return result;
}/* End of XSTP_MGR_AttachVlanToMstConfigTable() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_RemoveVlanFromMstConfigTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   Remove vlan from an instance for mst configuration table.
 * INPUT    :   UI32_T mstid               -- instance value
 *              UI32_T vlan                -- vlan value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK             -- set successfully
 *              XSTP_TYPE_RETURN_ERROR          -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_INDEX_OOR      -- mstid out of range
 *              XSTP_TYPE_RETURN_INDEX_NEX      -- vlan not existed
 * NOTE     :   If vlan = 0, remove the specified instance
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_RemoveVlanFromMstConfigTable(UI32_T mstid, UI32_T vlan)
{
    UI32_T                  vlan_id;
    UI32_T                  current_st_status;
    UI32_T                  current_st_mode;
    XSTP_OM_InstanceData_T  *om_ptr;
    char                    arg_buf[20];
    UI32_T                  saved_mstid;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE){

        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    if (mstid < 1 || mstid > XSTP_TYPE_MAX_MSTID){

        snprintf(arg_buf, 20, "Instance id (1-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_RemoveVlanFromMstConfigTable_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }

    if (vlan < 0 || vlan > XSTP_TYPE_SYS_MAX_VLAN_ID){

        snprintf(arg_buf, 20, "VLAN id (1-%d)",XSTP_TYPE_SYS_MAX_VLAN_ID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_RemoveVlanFromMstConfigTable_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }

    current_st_status = XSTP_OM_GetSpanningTreeStatus();
    current_st_mode = (UI32_T)XSTP_OM_GetForceVersion();

    /*the vlan =0 , means removing all vlan from mstid . so vlan(0) is not existing */
    if(vlan){
    /* Return ERROR if vlan is not member of instance by config table */
        XSTP_OM_GetMstidFromMstConfigurationTableByVlan(vlan, &saved_mstid);

        if (mstid != saved_mstid){

            return XSTP_TYPE_RETURN_ERROR;
        }
    }

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(mstid);
    /*
        * ERP:ES4827G-FLF-ZZ-00471
        *Problem:
        *     ref the bug discription (ES4827G-FLF-ZZ-00471)
        *Reason :
        *    xstp database of instance_vlans_mapped is not correct. the action in the older codes
        *        symmetrial.
        *Solution:
        *         modify vlan and instance bind and unbind sort
        *         adding steps as below:
        *        1. when create a new vlan, map the vlan to correct instance
        *         2. when config a vlan(existing vlan)to instance A, update the vlan bit to instance A,
        *            and unset the bit in cist instance
        *         removing steps as below:
        *         1, when destroy a existing vlan, remove the vlan bit map from correspending instance
        *         2, when un config a vlan(existing vlan) from instance A, unset the bit in A,
        *            and set the bit in cist instance
        *    the remove order is :
        *      when remove the vlan instance mapping,
        *      first remove from current databse,
        *      then from config table
        */

    if (vlan == 0){
        vlan_id = 0;
        while(XSTP_OM_GetNextXstpMemberFromMstConfigurationTable(mstid, &vlan_id)){

#ifdef  XSTP_TYPE_PROTOCOL_MSTP
            if (    (VLAN_OM_IsVlanExisted(vlan_id))
                &&  (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
                &&  (current_st_mode == XSTP_TYPE_MSTP_MODE)
               )
            {
                XSTP_MGR_RemoveVlanFromMstMappingTable(vlan_id);
            }
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

            XSTP_OM_SetMstidToMstConfigurationTableByVlan(vlan_id, XSTP_TYPE_CISTID);
        }

        XSTP_OM_SetEntryOfMstid(mstid, FALSE);
    }else{
        /* If the specified vlan has already existed, remove it from the MSTI
         * according to the configuration table when spanning tree is enabled and mode is mstp..
         */
        if (    (VLAN_OM_IsVlanExisted(vlan))
            &&  (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
            &&  (current_st_mode == XSTP_TYPE_MSTP_MODE)
           )
        {
            XSTP_MGR_RemoveVlanFromMstMappingTable(vlan);
        }

        XSTP_OM_SetMstidToMstConfigurationTableByVlan(vlan, XSTP_TYPE_CISTID);

        vlan_id = 0;
        if (!XSTP_OM_GetNextXstpMemberFromMstConfigurationTable(mstid, &vlan_id)){
            XSTP_OM_SetEntryOfMstid(mstid, FALSE);
        }
    }

#ifdef  XSTP_TYPE_PROTOCOL_MSTP
    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    XSTP_OM_GenerateConfigurationDigest(om_ptr);
    /* All CIST and MSTI transit to the initial state */
    XSTP_MGR_Restart();
    /* om_ptr->bridge_info.common->restart_state_machine = TRUE; */
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(mstid);
    return XSTP_TYPE_RETURN_OK;
}/* end of XSTP_MGR_RemoveVlanFromMstConfigTable */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_ResetVlanToMstConfigTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   Attach the specified vlan to the instance 0 in the mst configuration table.
 * INPUT    :   UI32_T vlan                -- vlan value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK             -- set successfully
 *              XSTP_TYPE_RETURN_ERROR          -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_INDEX_OOR      -- mstid out of range
 *              XSTP_TYPE_RETURN_INDEX_NEX      -- vlan not existed
 * NOTE     :
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_ResetVlanToMstConfigTable(UI32_T vlan)
{
    UI32_T                  vlan_id;
    UI32_T                  current_st_status;
    UI32_T                  current_st_mode;
    XSTP_OM_InstanceData_T  *om_ptr;
    char                    arg_buf[20];
    UI32_T                  old_mstid;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    if (vlan < 1 || vlan > XSTP_TYPE_SYS_MAX_VLAN_ID)
    {
        snprintf(arg_buf, 20, "VLAN id (1-%d)",XSTP_TYPE_SYS_MAX_VLAN_ID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_RemoveVlanFromMstConfigTable_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }

    current_st_status = XSTP_OM_GetSpanningTreeStatus();
    current_st_mode = (UI32_T)XSTP_OM_GetForceVersion();

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);

    /* If the specified vlan has already existed, remove it from the MSTI
     * according to the configuration table when spanning tree is enabled and mode is mstp..
     */
    if (    (VLAN_OM_IsVlanExisted(vlan))
        &&  (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
        &&  (current_st_mode == XSTP_TYPE_MSTP_MODE)
       )
    {
        XSTP_MGR_RemoveVlanFromMstMappingTable(vlan);
    }
    XSTP_OM_GetMstidFromMstConfigurationTableByVlan(vlan, &old_mstid);
    if (old_mstid != XSTP_TYPE_CISTID)
    {
        XSTP_OM_SetMstidToMstConfigurationTableByVlan(vlan, XSTP_TYPE_CISTID);

        if (    (VLAN_OM_IsVlanExisted(vlan))
            &&  (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
            &&  (current_st_mode == XSTP_TYPE_MSTP_MODE)
           )
        {

            XSTP_MGR_SetVlanToMstMappingTable(vlan);
        }

        vlan_id = 0;
        if (!XSTP_OM_GetNextXstpMemberFromMstConfigurationTable(old_mstid, &vlan_id))
        {
            XSTP_OM_SetEntryOfMstid(old_mstid, FALSE);
        }

        #ifdef  XSTP_TYPE_PROTOCOL_MSTP
        om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
        XSTP_OM_GenerateConfigurationDigest(om_ptr);
        /* All CIST and MSTI transit to the initial state */
        XSTP_MGR_Restart();
        #endif /* XSTP_TYPE_PROTOCOL_MSTP */
    }
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
    return XSTP_TYPE_RETURN_OK;
}/* end of XSTP_MGR_ResetVlanToMstConfigTable() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_DestroyVlanFromMstMappingTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   Remove vlan from an MSTI and CIST for the mst mapping table.
 * INPUT    :   UI32_T vlan                -- vlan value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK             -- set successfully
 *              XSTP_TYPE_RETURN_ERROR          -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_INDEX_OOR      -- mstid out of range
 *              XSTP_TYPE_RETURN_INDEX_NEX      -- vlan not existed
 * NOTE     :   1. If vlan = 0, remove the specified instance
 *              2. The specified vlan_id is guaranteed to have no member.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_DestroyVlanFromMstMappingTable(UI32_T vlan_id)
{
    UI32_T                  current_st_mode;
    UI32_T                  current_st_status;
    XSTP_OM_InstanceData_T  *om_ptr;
    UI32_T                  mstid;
    BOOL_T                  next_member_exist;
    UI32_T                  next_vlan_id;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    current_st_mode     = (UI32_T)XSTP_OM_GetForceVersion();
    if (current_st_mode != XSTP_TYPE_MSTP_MODE)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    XSTP_OM_GetMstidFromMstConfigurationTableByVlan(vlan_id, &mstid);

    if (mstid != XSTP_TYPE_CISTID)
    {
        /* +++ EnterCriticalRegion +++ */
        XSTP_OM_EnterCriticalSection(mstid);
        om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
        XSTP_OM_SetVlanToMstidMappingTable(om_ptr, vlan_id, FALSE);
        next_vlan_id = 0;
        next_member_exist   = XSTP_OM_GetNextXstpMember(om_ptr, &next_vlan_id);
        /* +++ LeaveCriticalRegion +++ */
        XSTP_OM_LeaveCriticalSection(mstid);

        current_st_status = XSTP_OM_GetSpanningTreeStatus();

        if (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
        {
            if (!next_member_exist)
            {
                /* +++ EnterCriticalRegion +++ */
                XSTP_OM_EnterCriticalSection(mstid);
                /* Delete this instance */
                XSTP_MGR_DeleteSpanningTree(om_ptr);
                /* +++ LeaveCriticalRegion +++ */
                XSTP_OM_LeaveCriticalSection(mstid);
            } /* End of if (!next_member_exist) */
            else
            {
                /* Remove those ports which leave this instance */
                UI32_T lport;

                /* +++ EnterCriticalRegion +++ */
                XSTP_OM_EnterCriticalSection(mstid);

                om_ptr = XSTP_OM_GetInstanceInfoPtr(mstid);
                lport = 0;
                while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
                {
                    BOOL_T  not_found;
                    UI32_T  vid_ifindex;

                    /* Check if this port is still a member of this instance */
                    next_vlan_id = 0;
                    not_found = TRUE;
                    while (XSTP_OM_GetNextXstpMember(om_ptr,&next_vlan_id))
                    {
                        VLAN_VID_CONVERTTO_IFINDEX(next_vlan_id, vid_ifindex);
                        if (VLAN_OM_IsPortVlanMember(vid_ifindex, lport))
                        {
                            not_found = FALSE;
                            break;
                        }
                    } /* End of while (not_found && GetNextXstpMember) */

                    if (not_found)
                    {
                        XSTP_OM_PortVar_T   *pom_ptr;

                        pom_ptr             = &(om_ptr->port_info[lport-1]);
                        pom_ptr->is_member  = FALSE;
                    } /* End of if (this_port_is_not_a_member_of_this_instance) */
                } /* End of while (SWCTRL_GetNextLogicalPort) */

                /* +++ LeaveCriticalRegion +++ */
                XSTP_OM_LeaveCriticalSection(mstid);
            }
        } /* End of if (SpanningTree_not_enabled) */

    } /* End of if (mstid != XSTP_TYPE_CISTID) */

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    XSTP_OM_SetVlanToMstidMappingTable(om_ptr, vlan_id, FALSE);
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    return XSTP_TYPE_RETURN_OK;
} /* End of XSTP_MGR_DestroyVlanFromMstMappingTable */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMstpConfigurationName
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set MSTP configurstion name.
 * INPUT    :   config_name             -- pointer of the config_name
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
 *              XSTP_TYPE_RETURN_ERROR              -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 * NOTE     :   Default : the bridage address
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetMstpConfigurationName(char *config_name)
{
    UI32_T                  current_st_mode;
    XSTP_OM_InstanceData_T  *om_ptr;
    UI32_T                  result;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    current_st_mode = (UI32_T)XSTP_OM_GetForceVersion();
    if (current_st_mode != XSTP_TYPE_MSTP_MODE)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstpConfigurationName_Fun_No, EH_TYPE_MSG_MSTP_SUPPORTED_ONLY, SYSLOG_LEVEL_INFO, "Name");
        return XSTP_TYPE_RETURN_ERROR;
    }

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    result = XSTP_MGR_SetMstpConfigurationName_(om_ptr, config_name);
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
    return result;
}/* End of XSTP_MGR_SetMstpConfigurationName()*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetRunningMstpConfigurationName
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the MSTP configurstion name.
 * INPUT    :
 * OUTPUT   :   *config_name                -- pointer of the config_name
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_GetRunningMstpConfigurationName(char *config_name)
{
    char               name[XSTP_TYPE_REGION_NAME_MAX_LENGTH+1];

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    memset(name, 0, XSTP_TYPE_REGION_NAME_MAX_LENGTH+1);
    XSTP_OM_CpuMacToString(name);
    memset(config_name, 0, XSTP_TYPE_REGION_NAME_MAX_LENGTH+1);
    XSTP_OM_GetRegionName(config_name);
    if (strcmp(name, config_name)== 0)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetMstpConfigurationName
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the MSTP configurstion name.
 * INPUT    :
 * OUTPUT   :   *config_name                -- pointer of the config_name
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_MGR_GetMstpConfigurationName(char *config_name)
{
    UI32_T                        result;

    result = XSTP_MGR_GetRunningMstpConfigurationName(config_name);
    if (result == SYS_TYPE_GET_RUNNING_CFG_FAIL)
    {
        return FALSE;
    }
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMstpRevisionLevel
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set MSTP revision level value.
 * INPUT    :   U32_T revision      -- revision value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK     -- set successfully
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_ERROR      -- failed
 * NOTE     :   Default : 0
 *              -- XSTP_TYPE_DEFAULT_CONFIG_REVISION
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetMstpRevisionLevel(UI32_T revision)
{
    UI32_T                      current_st_mode;
    UI32_T                      current_revision;

    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    XSTP_OM_InstanceData_T      *om_ptr;
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }
    if (revision < 0 || revision > 65535)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstpRevisionLevel_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Revision (0-65535)");
        return XSTP_TYPE_RETURN_ERROR;
    }
    current_revision = XSTP_OM_GetRegionRevision();
    current_st_mode = (UI32_T)XSTP_OM_GetForceVersion();
    if (current_st_mode != XSTP_TYPE_MSTP_MODE)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstpRevisionLevel_Fun_No, EH_TYPE_MSG_MSTP_SUPPORTED_ONLY, SYSLOG_LEVEL_INFO, "Revision");
        return XSTP_TYPE_RETURN_ERROR;
    }
    if(current_revision == revision)
    {
        return XSTP_TYPE_RETURN_OK;
    }

    XSTP_OM_SetRegionRevision(revision);

#ifdef  XSTP_TYPE_PROTOCOL_MSTP
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    om_ptr->bridge_info.common->mst_config_id.revision_level = revision;
    /* All CIST and MSTI transit to the initial state */
    XSTP_MGR_Restart();
    /* om_ptr->bridge_info.common->restart_state_machine = TRUE; */
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
#endif /* XSTP_TYPE_PROTOCOL_MSTP */
    return XSTP_TYPE_RETURN_OK;

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMstpMaxHop
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set MSTP Max_Hop count.
 * INPUT    :   UI32_T hop_count             -- max_hop value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK              -- set successfully
 *              XSTP_TYPE_RETURN_ERROR           -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_INDEX_OOR       -- hop_count out of range
 * NOTE     :   Range   : 1 ~ 40
 *              -- XSTP_TYPE_MSTP_MIN_MAXHOP
 *              -- XSTP_TYPE_MSTP_MAX_MAXHOP
 *              Default : 20
 *              -- XSTP_TYPE_DEFAULT_BRIDGE_MAXHOP
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetMstpMaxHop(UI32_T hop_count)
{
    UI32_T                  result;
    UI32_T                  current_st_mode;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    if (hop_count<XSTP_TYPE_MSTP_MIN_MAXHOP || hop_count>XSTP_TYPE_MSTP_MAX_MAXHOP)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstpMaxHop_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Max-hops (1-40)");
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }

    current_st_mode = (UI32_T)XSTP_OM_GetForceVersion();
    if (current_st_mode != XSTP_TYPE_MSTP_MODE)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstpMaxHop_Fun_No, EH_TYPE_MSG_MSTP_SUPPORTED_ONLY, SYSLOG_LEVEL_INFO, "Max-hops");
        return XSTP_TYPE_RETURN_ERROR;
    }

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    result = XSTP_MGR_SetMstpMaxHop_(hop_count);
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
    return result;

}/* End of XSTP_MGR_SetMstpMaxHop() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_VlanIsMstMember
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funcion returns true if the vlan is in the
 *              specified spanning tree. Otherwise, returns false.
 * INPUT    :   UI32_T mstid             -- the instance id
 *              UI32_T  lport            -- the lport number
 * OUTPUT   :   none
 * RETURN   :   TRUE/FALSE
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_MGR_VlanIsMstMember (UI32_T mstid,
                                 UI32_T vlan)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  is_member;

    /*SYSFUN_USE_CSC(FALSE);*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    is_member = FALSE;
    if(VLAN_OM_IsVlanExisted(vlan) == FALSE)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_VlanIsMstMember_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "VLAN ");
        return FALSE;
    }
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(mstid);

    om_ptr = XSTP_OM_GetInstanceInfoPtr(mstid);
    is_member = XSTP_OM_IsMemberVlanOfInstance(om_ptr, vlan);

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(mstid);
    return is_member;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetPortStateByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state in a specified vlan.
 * INPUT    :   UI32_T vid      -- vlan id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   U32_T  *state   -- the pointer of state value
 *                                 XSTP_TYPE_PORT_STATE_DISCARDING
 *                                 XSTP_TYPE_PORT_STATE_LEARNING
 *                                 XSTP_TYPE_PORT_STATE_FORWARDING
 * RETURN   :   TRUE/FALSE
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_MGR_GetPortStateByVlan(UI32_T vid,
                                   UI32_T lport,
                                   UI32_T *state)

{
    XSTP_OM_InstanceData_T  *om_ptr;
    UI32_T                  mstid;
    BOOL_T                  result;
    UI32_T                  current_st_status;
    UI32_T                  current_st_mode;

    /*SYSFUN_USE_CSC(FALSE);*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    if(VLAN_OM_IsVlanExisted(vid) == FALSE)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_GetPortStateByVlan_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "VLAN ");
        return FALSE;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_GetPortStateByVlan_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
        return FALSE;
    }

    {
        BOOL_T  is_blk;

        if (TRUE == XSTP_OM_GetEthRingPortStatus(lport, 0, &is_blk))
        {
            if (TRUE == is_blk)
                *state = VAL_dot1dStpPortState_blocking;
            else
                *state = VAL_dot1dStpPortState_forwarding;

            return TRUE;
        }
    }

    result  = FALSE;
    current_st_status   = XSTP_OM_GetSpanningTreeStatus();
    current_st_mode     = (UI32_T)XSTP_OM_GetForceVersion();
    if (    (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
        &&  (current_st_mode == XSTP_TYPE_MSTP_MODE)
       )
    {
        XSTP_OM_GetMstidFromMstConfigurationTableByVlan(vid, &mstid);
    }
    else
    {
        mstid = XSTP_TYPE_CISTID;
    }
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(mstid);
    om_ptr = XSTP_OM_GetInstanceInfoPtr(mstid);
    if (XSTP_MGR_GetMstPortState_(om_ptr, lport, state)==XSTP_TYPE_RETURN_OK)
        result = TRUE;
    else
        result = FALSE;
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(mstid);
    return result;
}/* End of XSTP_MGR_GetPortStateByVlan*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetNextPortMemberAndInstanceByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  : Get next port member and instance by a specified vlan
 * INPUT    : vid       -- vlan id
 *            mstid     -- instance value pointer
 *            lport     -- lport pointer
 * OUTPUT   : mstid     -- instance value pointer
 *            lport     -- lport pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the port list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_MGR_GetNextPortMemberAndInstanceByVlan(UI32_T vid,
                                                    UI32_T *mstid,
                                                    UI32_T *lport)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    UI32_T                  result;

    /*SYSFUN_USE_CSC(FALSE);*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    if (VLAN_OM_IsVlanExisted(vid)==FALSE)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_GetNextPortMemberAndInstanceByVlan_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "VLAN ");
        return FALSE;
    }
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_GetMstidFromMstConfigurationTableByVlan(vid, mstid);
    XSTP_OM_EnterCriticalSection(*mstid);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(*mstid);
    result              = XSTP_MGR_GetNextPortMemberOfInstance(om_ptr, lport);
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(*mstid);
    return result;

}/* End of XSTP_MGR_GetNextPortMemberAndInstanceByVlan*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetPortMemberListAndInstanceByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  : Get next port member and instance by a specified vlan
 * INPUT    : vid       -- vlan id
 *            mstid     -- instance value pointer
 *            portlist  -- portlist pointer
 * OUTPUT   : mstid     -- instance value pointer
 *            portlist  -- portlist pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the port list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_MGR_GetPortMemberListAndInstanceByVlan(UI32_T vid,
                                                    UI32_T *mstid,
                                                    UI8_T *portlist)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    UI32_T                  lport;
    UI32_T                  current_st_status;
    UI32_T                  current_st_mode;

    /*SYSFUN_USE_CSC(FALSE);*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    if (VLAN_OM_IsVlanExisted(vid)==FALSE)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_GetPortMemberListAndInstanceByVlan_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "VLAN ");
        return FALSE;
    }
    current_st_status   = XSTP_OM_GetSpanningTreeStatus();
    current_st_mode     = (UI32_T)XSTP_OM_GetForceVersion();
    if (    (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
        &&  (current_st_mode == XSTP_TYPE_MSTP_MODE)
       )
    {
        XSTP_OM_GetMstidFromMstConfigurationTableByVlan(vid, mstid);
    }
    else
    {
        *mstid = XSTP_TYPE_CISTID;
    }
    lport   = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        /* +++ EnterCriticalRegion +++ */
        XSTP_OM_EnterCriticalSection(*mstid);
        om_ptr              = XSTP_OM_GetInstanceInfoPtr(*mstid);
        if (XSTP_OM_IsMemberPortOfInstance(om_ptr, lport))
        {
            portlist[(lport-1)>>3] = portlist[(lport-1)>>3] | (0x80>>((lport-1)%8));
        }
        /* +++ LeaveCriticalRegion +++ */
        XSTP_OM_LeaveCriticalSection(*mstid);
    }
    return TRUE;
}/* End of XSTP_MGR_GetPortMemberListAndInstanceByVlan*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetMappedInstanceByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  : Get mapped instance by a specified vlan
 * INPUT    : vid       -- vlan id
 *            mstid     -- instance value pointer
 * OUTPUT   : mstid     -- instance value pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the port list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_MGR_GetMappedInstanceByVlan(UI32_T vid,
                                         UI32_T *mstid)
{
    UI32_T                  current_st_status;
    UI32_T                  current_st_mode;

    /*SYSFUN_USE_CSC(FALSE);*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    if (VLAN_OM_IsVlanExisted(vid)==FALSE)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_GetPortMemberListAndInstanceByVlan_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "VLAN ");
        return FALSE;
    }
    current_st_status   = XSTP_OM_GetSpanningTreeStatus();
    current_st_mode     = (UI32_T)XSTP_OM_GetForceVersion();
    if (    (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
        &&  (current_st_mode == XSTP_TYPE_MSTP_MODE)
       )
    {
        XSTP_OM_GetMstidFromMstConfigurationTableByVlan(vid, mstid);
    }
    else
    {
        *mstid = XSTP_TYPE_CISTID;
    }
    return TRUE;
}/* End of XSTP_MGR_GetMappedInstanceByVlan*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetForwardingPortMemberListByInstance
 * ------------------------------------------------------------------------
 * PURPOSE  : Get forwarding port member list by a specified instance
 * INPUT    : mstid     -- instance value
 *            portlist  -- portlist pointer
 * OUTPUT   : portlist  -- portlist pointer
 * RETURN   : TRUE/FALSE
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_MGR_GetForwardingPortMemberListByInstance(UI32_T mstid,
                                                       UI8_T *portlist)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    UI32_T                  lport;
    XSTP_OM_PortVar_T       *pom_ptr;
    BOOL_T                  result;
    UI32_T                  current_st_status;

    /*SYSFUN_USE_CSC(FALSE);*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
        return FALSE;
    }

    result              = FALSE;
    current_st_status   = XSTP_OM_GetSpanningTreeStatus();
    if (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
    {
        /* +++ EnterCriticalRegion +++ */
        XSTP_OM_EnterCriticalSection(mstid);
        om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
        if (om_ptr->instance_exist)
        {
            lport   = 0;
            while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                pom_ptr             = &(om_ptr->port_info[lport-1]);
                if (    XSTP_OM_IsMemberPortOfInstance(om_ptr, lport)
                    &&  (pom_ptr->learning)
                    &&  (pom_ptr->forwarding)
                   )
                {
                    portlist[(lport-1)>>3] = portlist[(lport-1)>>3] | (0x80>>((lport-1)%8));
                }
            }
            result = TRUE;
        }
        /* +++ LeaveCriticalRegion +++ */
        XSTP_OM_LeaveCriticalSection(mstid);
    }
    else
    {
        if (mstid == XSTP_TYPE_CISTID)
        {
            lport   = 0;
            while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                /* +++ EnterCriticalRegion +++ */
                XSTP_OM_EnterCriticalSection(mstid);
                om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
                pom_ptr             = &(om_ptr->port_info[lport-1]);
                if (    XSTP_OM_IsMemberPortOfInstance(om_ptr, lport)
                    &&  (pom_ptr->learning)
                    &&  (pom_ptr->forwarding)
                   )
                {
                    portlist[(lport-1)>>3] = portlist[(lport-1)>>3] | (0x80>>((lport-1)%8));
                }
                /* +++ LeaveCriticalRegion +++ */
                XSTP_OM_LeaveCriticalSection(mstid);
            }
            result = TRUE;
        }
    }
    return result;
}/* End of XSTP_MGR_GetForwardingPortMemberListByInstance*/


/* LOCAL SUBPROGRAM BODIES
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMstForwardDelay
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the forward_delay time information of the specified
 *              spanning tree.
 * INPUT    :   *om_ptr                  -- the pointer of info entry
 *              UI32_T forward_delay     -- the forward_delay value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK           -- set successfully
 *              XSTP_TYPE_RETURN_INDEX_OOR    -- forward_delay out of range
 * NOTE     :   1. Time unit is 1 sec
 *              2. Range
 *                 -- XSTP_TYPE_MIN_FORWARD_DELAY
 *                 -- XSTP_TYPE_MAX_FORWARD_DELAY
 *              3. Default
 *                 -- XSTP_TYPE_DEFAULT_FORWARD_DELAY
 * ------------------------------------------------------------------------
 */
static UI32_T XSTP_MGR_SetMstForwardDelay(XSTP_OM_InstanceData_T *om_ptr,
                                          UI32_T forward_delay)
{
    UI32_T                          current_forward_delay;
    UI32_T                          max_age;
    UI32_T                          min_time, max_time;
    UI32_T                          index;
    XSTP_OM_PortVar_T               *pom_ptr;

    max_age = (UI32_T)om_ptr->bridge_info.bridge_times.max_age;

    /* Dependency among Bridge_Forward_Delay, Bridge_Max_Age, and Bridge_Hello_Time :
     *      2*( Bridge_Forward_Delay - 1.0 seconds ) >= Bridge_Max_Age;
     *      Bridge_Max_Age >= 2*( Bridge_Hello_Time +1.0 seconds );
     */

    if ( (max_age+ 2 /*XSTP_TYPE_HUNDRED_SEC_TICK*/) > 2*(XSTP_TYPE_MIN_FORWARD_DELAY) )
    {
        min_time = max_age/2+ 1 /*XSTP_TYPE_HUNDRED_SEC_TICK*/;
    }
    else
    {
        min_time = XSTP_TYPE_MIN_FORWARD_DELAY;
    }
    max_time = XSTP_TYPE_MAX_FORWARD_DELAY;

    if( forward_delay < min_time || forward_delay > max_time )
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetForwardDelay_Fun_No, EH_TYPE_MSG_INVALID_VALUE, SYSLOG_LEVEL_INFO, "forward-time>=(max-age/2)+1");
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }
    current_forward_delay = (UI32_T)om_ptr->bridge_info.bridge_times.forward_delay;

    if (current_forward_delay == forward_delay)
    {
        return XSTP_TYPE_RETURN_OK;
    }
    om_ptr->bridge_info.bridge_times.forward_delay = (UI16_T)forward_delay;
    /* 20020717 if it is root bridge, update the forward_delay. */
    if (XSTP_MGR_IsRootBridge(om_ptr)==TRUE)
    {
        om_ptr->bridge_info.root_times.forward_delay = (UI16_T)forward_delay;
        for (index=0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++ )
        {
            pom_ptr = &(om_ptr->port_info[index]);
            pom_ptr->port_times.forward_delay = pom_ptr->designated_times.forward_delay   /* 17.18.3 */
                                              = (UI16_T)forward_delay;
        }
    }

    return XSTP_TYPE_RETURN_OK;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMstHelloTime
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the hello_time information of the specified spanning
 *              tree.
 * INPUT    :   *om_ptr                 -- the pointer of info entry
 *              UI32_T hello_time       -- the hello_time value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_INDEX_OOR   -- hello_time out of range
 * NOTE     :   1. Time unit is 1 sec
 *              2. Range
 *                 -- XSTP_TYPE_MIN_HELLO_TIME
 *                 -- XSTP_TYPE_MAX_HELLO_TIME
 *              3. Default
 *                 -- XSTP_TYPE_DEFAULT_HELLO_TIME
 * * REF    :   RFC-1493/dot1dStp 13
 * ------------------------------------------------------------------------
 */
static UI32_T XSTP_MGR_SetMstHelloTime(XSTP_OM_InstanceData_T *om_ptr,
                                       UI32_T hello_time)
{
    UI32_T                          current_hello_time;
    UI32_T                          max_age;
    UI32_T                          min_time, max_time;
    UI32_T                          index;
    XSTP_OM_PortVar_T               *pom_ptr;

    max_age = (UI32_T)om_ptr->bridge_info.bridge_times.max_age;

    /* Dependency among Bridge_Forward_Delay, Bridge_Max_Age, and Bridge_Hello_Time :
     *      2*( Bridge_Forward_Delay - 1.0 seconds ) >= Bridge_Max_Age;
     *      Bridge_Max_Age >= 2*( Bridge_Hello_Time +1.0 seconds );
     */

    if ( (max_age-2/*XSTP_TYPE_HUNDRED_SEC_TICK*/) < 2*XSTP_TYPE_MAX_HELLO_TIME )
    {
        max_time = max_age/2- 1 /*XSTP_TYPE_HUNDRED_SEC_TICK*/;
    }
    else
    {
        max_time = XSTP_TYPE_MAX_HELLO_TIME;
    }

    min_time = XSTP_TYPE_MIN_HELLO_TIME;

    if( hello_time < min_time || hello_time > max_time )
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetHelloTime_Fun_No, EH_TYPE_MSG_INVALID_VALUE, SYSLOG_LEVEL_INFO, "hello-time<=(max-age/2)-1");
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }

    current_hello_time = (UI32_T)om_ptr->bridge_info.bridge_times.hello_time;

    if (current_hello_time == hello_time)
    {
        return XSTP_TYPE_RETURN_OK;
    }
    om_ptr->bridge_info.bridge_times.hello_time = (UI16_T)hello_time;
    /* 20020717 if it is root bridge, update the hello_time. */
    if (XSTP_MGR_IsRootBridge(om_ptr)==TRUE)
    {
        om_ptr->bridge_info.root_times.hello_time = (UI16_T)hello_time;
        for (index=0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++ )
        {
            pom_ptr = &(om_ptr->port_info[index]);
            pom_ptr->port_times.hello_time = pom_ptr->designated_times.hello_time   /* 17.18.3 */
                                           = (UI16_T)hello_time;
        }
    }

    return XSTP_TYPE_RETURN_OK;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMstMaxAge
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the max_age information.of the specified spanning tree.
 * INPUT    :   *om_ptr                  -- the pointer of info entry
 *              UI32_T max_age           -- the max_age value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK           -- set successfully
 *              XSTP_TYPE_RETURN_INDEX_OOR    -- max_age out of range
 * NOTE     :   1. Time unit is 1 sec
 *              2. Range
 *                 -- XSTP_TYPE_MIN_MAXAGE
 *                 -- XSTP_TYPE_MAX_MAXAGE
 *              3. Default
 *                 -- XSTP_TYPE_DEFAULT_MAX_AGE
 * REF      :   RFC-1493/dot1dStp 12
 * ------------------------------------------------------------------------
 */
static UI32_T XSTP_MGR_SetMstMaxAge(XSTP_OM_InstanceData_T *om_ptr,
                                    UI32_T max_age)
{
    UI32_T                          current_max_age;
    UI32_T                          hello_time, forward_delay;
    UI32_T                          min_time, max_time;
    UI32_T                          index;
    XSTP_OM_PortVar_T               *pom_ptr;

    hello_time      = (UI32_T)om_ptr->bridge_info.bridge_times.hello_time;
    forward_delay   = (UI32_T)om_ptr->bridge_info.bridge_times.forward_delay;

    /* Dependency among Bridge_Forward_Delay, Bridge_Max_Age, and Bridge_Hello_Time :
     *      2*( Bridge_Forward_Delay - 1.0 seconds ) >= Bridge_Max_Age;
     *      Bridge_Max_Age >= 2*( Bridge_Hello_Time +1.0 seconds );
     */

    if ( (2*(hello_time+ 1 /*XSTP_TYPE_HUNDRED_SEC_TICK*/)) > XSTP_TYPE_MIN_MAXAGE )
    {
        min_time = 2*(hello_time+ 1 /*XSTP_TYPE_HUNDRED_SEC_TICK*/);
    }
    else
    {
        min_time = XSTP_TYPE_MIN_MAXAGE;
    }
    if ( (2*(forward_delay-1/*XSTP_TYPE_HUNDRED_SEC_TICK*/)) < XSTP_TYPE_MAX_MAXAGE )
    {
        max_time = 2*(forward_delay-1/*XSTP_TYPE_HUNDRED_SEC_TICK*/);
    }
    else
    {
        max_time = XSTP_TYPE_MAX_MAXAGE;
    }
    if( max_age < min_time || max_age > max_time )
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMaxAge_Fun_No, EH_TYPE_MSG_INVALID_VALUE, SYSLOG_LEVEL_INFO, "2*(hello-time+1)<=max-age<=2*(forward-time-1)");
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }
    current_max_age = (UI32_T)om_ptr->bridge_info.bridge_times.max_age;

    if (current_max_age == max_age)
    {
        return XSTP_TYPE_RETURN_OK;
    }
    om_ptr->bridge_info.bridge_times.max_age = (UI16_T)max_age;
    /* 20020717 if it is root bridge, update the max_age. */
    if (XSTP_MGR_IsRootBridge(om_ptr)==TRUE)
    {
        om_ptr->bridge_info.root_times.max_age = (UI16_T)max_age;
        for (index=0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++ )
        {
            pom_ptr = &(om_ptr->port_info[index]);
            pom_ptr->port_times.max_age = pom_ptr->designated_times.max_age   /* 17.18.3 */
                                        = (UI16_T)max_age;
        }
    }

    return XSTP_TYPE_RETURN_OK;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMstTransmissionLimit
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the transmission limit vlaue of the specified tree.
 * INPUT    :   *om_ptr                  -- the pointer of info entry
 *              UI32_T  tx_hold_count    -- the TXHoldCount value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK           -- set successfully
 *              XSTP_TYPE_RETURN_INDEX_OOR    -- tx_hold_count out of range
 * NOTE     :   1. The value used by the Port Transmit state machine to
 *                 limit the maximum transmission rate.
 *              2. Range
 *                 -- XSTP_TYPE_MIN_TX_HOLD_COUNT
 *                 -- XSTP_TYPE_MAX_TX_HOLD_COUNT
 *              3. Default
 *                 -- XSTP_TYPE_DEFAULT_TX_HOLD_COUNT
 * REF      :   draft-ietf-bridge-rstpmib-02/dot1dStp 17
 * ------------------------------------------------------------------------
 */
static UI32_T XSTP_MGR_SetMstTransmissionLimit(XSTP_OM_InstanceData_T *om_ptr,
                                               UI32_T tx_hold_count)
{
    UI32_T                              current_tx_hold_count;

    if (tx_hold_count < XSTP_TYPE_MIN_TX_HOLD_COUNT || tx_hold_count > XSTP_TYPE_MAX_TX_HOLD_COUNT)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetTransmissionLimit_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Transmission-limit (1-10)");
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }
    current_tx_hold_count = (UI32_T)om_ptr->bridge_info.common->tx_hold_count;

    if (current_tx_hold_count == tx_hold_count)
    {
        return XSTP_TYPE_RETURN_OK;
    }

    om_ptr->bridge_info.common->tx_hold_count = (UI8_T)tx_hold_count;

    return XSTP_TYPE_RETURN_OK;
}/* End of XSTP_MGR_SetMstTransmissionLimit */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_IsRootBridge
 * ------------------------------------------------------------------------
 * FUNCTION : Check whether the Bridge is the Root Bridge.
 * INPUT    : om_ptr
 * OUTPUT   : None
 * RETURN   : TRUE,  if the Bridge is the Root Bridge
 *            FALSE, if the Bridge is not the Root Bridge
 * NOTE     : If the Designated Root parameter held by the Bridge is the
 *            same as the Bridge ID of the Bridge, then conclude the
 *            Bridge to be the Root Bridge.
 * ------------------------------------------------------------------------
 */
static BOOL_T XSTP_MGR_IsRootBridge (XSTP_OM_InstanceData_T *om_ptr)
{
    I32_T cmp_a;

    #ifdef  XSTP_TYPE_PROTOCOL_RSTP
    XSTP_OM_CMP_BRIDGE_ID(cmp_a, (om_ptr->bridge_info.root_priority.root_bridge_id), (om_ptr->bridge_info.bridge_identifier));
    return((BOOL_T)(cmp_a == 0));
    #endif /* XSTP_TYPE_PROTOCOL_RSTP */

    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    if (om_ptr->instance_id == XSTP_TYPE_CISTID)    /* CIST */
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

}/* End of XSTP_MGR_IsRootBridge() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_PortStatusIsFullDuplex
 * ------------------------------------------------------------------------
 * PURPOSE  :   Return TRUE if speed duplex status is full_duplex for port.
 *              Return FALSE otherwise.
 * INPUT    :   lport
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
static BOOL_T XSTP_MGR_PortStatusIsFullDuplex(UI32_T lport)
{
    UI32_T                  speed_duplex_oper;
    Port_Info_T             port_info;


    SWCTRL_GetPortInfo(lport, &port_info);
    speed_duplex_oper = port_info.speed_duplex_oper;
    if (    (speed_duplex_oper == XSTP_TYPE_SPDPLX_STATUS_10FULL)
         || (speed_duplex_oper == XSTP_TYPE_SPDPLX_STATUS_100FULL)
         || (speed_duplex_oper == XSTP_TYPE_SPDPLX_STATUS_1000FULL)
         || (speed_duplex_oper == XSTP_TYPE_SPDPLX_STATUS_10GFULL)
         || (speed_duplex_oper == XSTP_TYPE_SPDPLX_STATUS_25GFULL)
         || (speed_duplex_oper == XSTP_TYPE_SPDPLX_STATUS_40GFULL)
         || (speed_duplex_oper == XSTP_TYPE_SPDPLX_STATUS_100GFULL)
       )
    {
        return TRUE;
    }
    return FALSE;
}/* End of XSTP_MGR_PortStatusIsFullDuplex */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMstpConfigurationName_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set MSTP configurstion name.
 * INPUT    :   om_ptr                  -- the pointer of the instance entry.
 *              config_name             -- pointer of the config_name
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK             -- set successfully
 *              XSTP_TYPE_RETURN_ERROR      -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 * NOTE     :   Default : the bridage address
 * ------------------------------------------------------------------------
 */
static UI32_T XSTP_MGR_SetMstpConfigurationName_(XSTP_OM_InstanceData_T *om_ptr,
                                                 char *config_name)
{
    char                    region_name[XSTP_TYPE_REGION_NAME_MAX_LENGTH+1];
    char                    temp_config_name[XSTP_TYPE_REGION_NAME_MAX_LENGTH+1];
    UI32_T                  name_len;

    memset(region_name, 0, XSTP_TYPE_REGION_NAME_MAX_LENGTH+1);
    memset(temp_config_name, 0, XSTP_TYPE_REGION_NAME_MAX_LENGTH+1);
    name_len = strlen(config_name);
    if (name_len > XSTP_TYPE_REGION_NAME_MAX_LENGTH)
    {
        name_len = XSTP_TYPE_REGION_NAME_MAX_LENGTH;
    }
    memcpy(temp_config_name, config_name, name_len);

    XSTP_OM_GetRegionName(region_name);
    if (strcmp(temp_config_name, region_name)!=0 )
    {
        XSTP_OM_SetRegionName(temp_config_name);

        #ifdef  XSTP_TYPE_PROTOCOL_MSTP
        memset(om_ptr->bridge_info.common->mst_config_id.config_name, 0, XSTP_TYPE_REGION_NAME_MAX_LENGTH);
        memcpy(om_ptr->bridge_info.common->mst_config_id.config_name, temp_config_name, name_len);
        if (region_name[0]!=0)
        {
            /* All CIST and MSTI transit to the initial state */
            XSTP_MGR_Restart();
            /* om_ptr->bridge_info.common->restart_state_machine = TRUE; */
        }
        #endif /* XSTP_TYPE_PROTOCOL_MSTP */
    }
    return XSTP_TYPE_RETURN_OK;
}/* End of XSTP_MGR_SetMstpConfigurationName_() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMstPriority_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the priority of the specified MST instance
 *              when the switch is MST mode.
 * INPUT    :   om_ptr             -- the pointer of the instance entry.
 *              UI32_T  priority   -- the priority value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK           -- set successfully
 *              XSTP_TYPE_RETURN_ERROR        -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_INDEX_OOR    -- mstid and priority out of range
 * NOTE     :   1. Range    : 0 ~ 61440 (in steps of 4096)
 *                 -- XSTP_TYPE_MIN_BRIDGE_PRIORITY
 *                 -- XSTP_TYPE_MAX_BRIDGE_PRIORITY_RSTP_MSTP
 *              2. DEFAULT  : 32768
 *                 -- XSTP_TYPE_DEFAULT_BRIDGE_PRIORITY
 * ------------------------------------------------------------------------
 */
static UI32_T XSTP_MGR_SetMstPriority_(XSTP_OM_InstanceData_T *om_ptr,
                                       UI32_T priority,
                                       BOOL_T is_static_config)
{
    UI16_T                  temp_priority;
    UI32_T                  current_priority;
    UI32_T                  current_st_status;
    XSTP_TYPE_BridgeId_T    current_root_bridge_id;
    I32_T                   cmp_a;
    TRAP_EVENT_TrapData_T   trap_data;

    current_st_status = XSTP_OM_GetSpanningTreeStatus();

    if (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
    {
        if (!om_ptr->instance_exist)
        {
            if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
                BACKDOOR_MGR_Printf("\r\nXSTP_MGR_SetMstPriority_::ERROR!! for invalid mstid.");
            EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstPriority_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Instance ID");
            return XSTP_TYPE_RETURN_ERROR;
        }
    }

    XSTP_OM_GET_BRIDGE_ID_PRIORITY(temp_priority, om_ptr->bridge_info.bridge_identifier);
    current_priority = (UI32_T)temp_priority;

    if (priority < XSTP_TYPE_MIN_BRIDGE_PRIORITY || priority > XSTP_TYPE_MAX_BRIDGE_PRIORITY_RSTP_MSTP )
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstPriority_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Priority (0-61440)");
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }
    if (priority%4096)
    {
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
            BACKDOOR_MGR_Printf("\r\nXSTP_MGR_SetMstPriority_::ERROR!! Must be in steps of 4096");
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstPriority_Fun_No, EH_TYPE_MSG_INVALID_VALUE, SYSLOG_LEVEL_INFO, "priority must be in increments of 4096");
        return XSTP_TYPE_RETURN_ERROR;
    }

    if (is_static_config)
    {
        om_ptr->bridge_info.static_bridge_priority = TRUE;
        om_ptr->bridge_info.admin_bridge_priority  = priority;
    }

    if (current_priority == priority)
    {
        return XSTP_TYPE_RETURN_OK;
    }

    /* Let priority recorded in OM consist with bridge_priority set by user*/

    #ifdef  XSTP_TYPE_PROTOCOL_RSTP
    memcpy(&current_root_bridge_id, &om_ptr->bridge_info.root_priority.root_bridge_id, XSTP_TYPE_BRIDGE_ID_LENGTH);
    XSTP_OM_MakeOmPriorityConsistency(om_ptr, priority);
    #endif /* XSTP_TYPE_PROTOCOL_RSTP */

    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    if (om_ptr->instance_id == XSTP_TYPE_CISTID)    /* CIST */
    {
        memcpy(&current_root_bridge_id, &om_ptr->bridge_info.root_priority.root_id, XSTP_TYPE_BRIDGE_ID_LENGTH);
        XSTP_OM_MakeOmPriorityConsistencyCist(om_ptr, priority);
    }
    else                                            /* MSTI */
    {
        memcpy(&current_root_bridge_id, &om_ptr->bridge_info.root_priority.r_root_id, XSTP_TYPE_BRIDGE_ID_LENGTH);
        XSTP_OM_MakeOmPriorityConsistencyMsti(om_ptr, priority);
    }
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    XSTP_MGR_SetRootBridge(om_ptr);
    XSTP_ENGINE_StateMachineProgress(om_ptr);

    if (memcmp(om_ptr->bridge_info.bridge_identifier.addr,
            current_root_bridge_id.addr, 6) == 0)
    {
        /* private trap: root bridge is changed */
        trap_data.trap_type = TRAP_EVENT_XSTP_ROOT_BRIDGE_CHANGED;
        XSTP_OM_GET_BRIDGE_ID_PRIORITY(
            trap_data.u.xstp_root_bridge_changed.priority,
            om_ptr->bridge_info.bridge_identifier);
        trap_data.u.xstp_root_bridge_changed.instance_id = om_ptr->instance_id;
        memcpy(trap_data.u.xstp_root_bridge_changed.bridge_address,
            &om_ptr->bridge_info.bridge_identifier.addr, 6);
        trap_data.community_specified = FALSE;
        trap_data.flag = TRAP_EVENT_SEND_TRAP_OPTION_DEFAULT;
        SNMP_PMGR_ReqSendTrap(&trap_data);

#if (SYS_CPNT_DEBUG == TRUE)
        XSTP_UTY_DebugPrintRoot(om_ptr, TRUE);
#endif
    }
    else
    {
        XSTP_OM_CMP_BRIDGE_ID(cmp_a, om_ptr->bridge_info.bridge_identifier, current_root_bridge_id);
        if (cmp_a < 0)
        {
            /* standard trap: become new root bridge */
            XSTP_OM_SetTrapFlagNewRoot(TRUE);

            /* private trap: root bridge is changed */
            trap_data.trap_type = TRAP_EVENT_XSTP_ROOT_BRIDGE_CHANGED;
            XSTP_OM_GET_BRIDGE_ID_PRIORITY(
                trap_data.u.xstp_root_bridge_changed.priority,
                om_ptr->bridge_info.bridge_identifier);
            trap_data.u.xstp_root_bridge_changed.instance_id = om_ptr->instance_id;
            memcpy(trap_data.u.xstp_root_bridge_changed.bridge_address,
                &om_ptr->bridge_info.bridge_identifier.addr, 6);
            trap_data.community_specified = FALSE;
            trap_data.flag = TRAP_EVENT_SEND_TRAP_OPTION_DEFAULT;
            SNMP_PMGR_ReqSendTrap(&trap_data);

#if (SYS_CPNT_DEBUG == TRUE)
            XSTP_UTY_DebugPrintRoot(om_ptr, TRUE);
#endif
        }
    }

    return XSTP_TYPE_RETURN_OK;
}/* End of XSTP_MGR_SetMstPriority_() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMstPortPriority_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the port priority for the specified spanning tree
 *              when mode is MSTP.
 * INPUT    :   om_ptr              -- the pointer of the instance entry.
 *              UI32_T lport        -- lport number
 *              UI32_T priority     -- the priority value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- lport number out of range
 *              XSTP_TYPE_RETURN_INDEX_OOR   -- mstid and priority out of range
 * NOTE     :   1. Range    : 0 ~ 240 (in steps of 16)
 *                 -- XSTP_TYPE_MIN_PORT_PRIORITY
 *                 -- XSTP_TYPE_MAX_PORT_PRIORITY_RSTP_MSTP
 *              2. Default  : 128
 *                 -- XSTP_TYPE_DEFAULT_PORT_PRIORITY
 * ------------------------------------------------------------------------
 */
static UI32_T XSTP_MGR_SetMstPortPriority_(XSTP_OM_InstanceData_T *om_ptr,
                                           UI32_T lport,
                                           UI32_T priority)
{
    UI32_T                  current_port_priority;
    XSTP_OM_PortVar_T       *pom_ptr;
    UI32_T                  root_port;
    XSTP_OM_PortVar_T       *root_pom_ptr;
    UI32_T                  current_st_status;
    I32_T                   cmp_a, cmp_b, cmp_c =0;
    UI8_T                   temp_priority;

    current_st_status = XSTP_OM_GetSpanningTreeStatus();

    if (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
    {
        if (!om_ptr->instance_exist)
        {
            if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
                BACKDOOR_MGR_Printf("\r\nXSTP_MGR_SetMstPortPriority_::ERROR!! for invalid mstid.");
            EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstPortPriority_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Instance ID");
            return XSTP_TYPE_RETURN_ERROR;
        }
    }

    if (priority < XSTP_TYPE_MIN_PORT_PRIORITY || priority > XSTP_TYPE_MAX_PORT_PRIORITY_RSTP_MSTP)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstPortPriority_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Port-priority (0-240)");
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }
    if (priority%16)
    {
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
            BACKDOOR_MGR_Printf("\r\nXSTP_MGR_SetMstPortPriority_::ERROR!! Must be in steps of 16");
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstPortPriority_Fun_No, EH_TYPE_MSG_INVALID_VALUE, SYSLOG_LEVEL_INFO, "port priority must be in increments of 16");
        return XSTP_TYPE_RETURN_ERROR;
    }

    pom_ptr = &(om_ptr->port_info[lport-1]);
    XSTP_OM_GET_PORT_ID_PRIORITY(temp_priority, pom_ptr->port_id);
    current_port_priority = (UI32_T)temp_priority;
    if (current_port_priority == priority)
    {
        return XSTP_TYPE_RETURN_OK;
    }
    else
    {
        pom_ptr->static_port_priority = TRUE;
    }

    XSTP_OM_MakeOmPortPriorityConsistency(om_ptr, lport, priority);
    root_port = (UI32_T) om_ptr->bridge_info.root_port_id.data.port_num;
    if (0 != root_port)
    {
        root_pom_ptr = &(om_ptr->port_info[root_port-1]);
        XSTP_OM_CMP_PORT_ID(cmp_c, (pom_ptr->port_priority.designated_port_id), (root_pom_ptr->port_priority.designated_port_id));
    }
    XSTP_OM_CMP_PORT_ID(cmp_a, (pom_ptr->port_id), (pom_ptr->port_priority.designated_port_id));
    XSTP_OM_CMP_PORT_ID(cmp_b, (pom_ptr->port_id), (om_ptr->bridge_info.root_port_id));
    if (    (   (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_BACKUP)
             && (cmp_a < 0)
            )
        ||  (   (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
             && (cmp_b < 0)
             && (cmp_c == 0)
            )
        ||  (   (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
             && (current_port_priority < priority)
            )
       )
    {
        /* Initialize the port_priority to motivate the PIM state machine for reselect port_role. */
        XSTP_MGR_InitPortPriorityVector(om_ptr, lport);
    }

    XSTP_ENGINE_StateMachineProgress(om_ptr);
    return XSTP_TYPE_RETURN_OK;

} /* End of XSTP_MGR_SetMstPortPriority_() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMstPortPathCost_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the path_cost of the port for specified spanning tree
 *              when mode is MSTP.
 * INPUT    :   om_ptr                  -- the pointer of the instance entry.
 *              UI32_T lport            -- lport number
 *              UI32_T path_cost        -- the path_cost value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURNPORTNO_OOR  -- port number out of range
 *              XSTP_TYPE_RETURNINDEX_OOR   -- mstid and priority out of range
 * NOTE     :  1. In long mode (VAL_dot1dStpPathCostDefault_stp8021t2001)
 *                -- Range : 0 ~ 200000000
 *                   XSTP_TYPE_MIN_PORT_PATH_COST
 *                   XSTP_TYPE_MAX_PORT_PATH_COST_32
 *             2. In short mode (VAL_dot1dStpPathCostDefault_stp8021d1998)
 *                -- Range : 0 ~ 65535
 *                   XSTP_TYPE_MIN_PORT_PATH_COST
 *                   XSTP_TYPE_MAX_PORT_PATH_COST_16
 * ------------------------------------------------------------------------
 */
static UI32_T XSTP_MGR_SetMstPortPathCost_(XSTP_OM_InstanceData_T *om_ptr,
                                           UI32_T lport,
                                           UI32_T path_cost)
{
    UI32_T                  current_path_cost_method;
    UI32_T                  root_path_cost;
    UI32_T                  max_path_cost;
    UI32_T                  index;
    XSTP_OM_PortVar_T       *index_pom_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    UI32_T                  root_lport;
    XSTP_OM_PortVar_T       *root_pom_ptr;
    UI32_T                  current_st_status;

    current_st_status = XSTP_OM_GetSpanningTreeStatus();

    if (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
    {
        if (!om_ptr->instance_exist)
        {
            if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
                BACKDOOR_MGR_Printf("\r\nXSTP_MGR_SetMstPortPathCost::ERROR!! for invalid mstid.");
            EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstPortPathCost_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Instance ID");
            return XSTP_TYPE_RETURN_ERROR;
        }
    }

    current_path_cost_method    = XSTP_OM_GetPathCostMethod();
    if (current_path_cost_method == XSTP_TYPE_PATH_COST_DEFAULT_SHORT)
    {
        max_path_cost = XSTP_TYPE_MAX_PORT_PATH_COST_16;
    }
    else /* current_path_cost_method == XSTP_TYPE_PATH_COST_DEFAULT_LONG */
    {
        max_path_cost = XSTP_TYPE_MAX_PORT_PATH_COST_32;
    }

    if (path_cost < XSTP_TYPE_MIN_PORT_PATH_COST || path_cost > max_path_cost )
    {
        if (current_path_cost_method == XSTP_TYPE_PATH_COST_DEFAULT_SHORT)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstPortPathCost_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Short cost (1-65535)");
        }
        else if (current_path_cost_method == XSTP_TYPE_PATH_COST_DEFAULT_LONG)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstPortPathCost_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Long cost (1-200000000)");
        }
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }

    pom_ptr                     = &(om_ptr->port_info[lport-1]);

    #ifdef  XSTP_TYPE_PROTOCOL_RSTP
    /* static path cost */
    pom_ptr->static_path_cost   = TRUE;
    pom_ptr->port_path_cost     = path_cost;
    /* Root port. For consistency */
    root_lport = (UI32_T)(om_ptr->bridge_info.root_port_id.port_id & 0x0FFF);
    if ( root_lport == lport )
    {
        root_path_cost = pom_ptr->port_priority.root_path_cost + pom_ptr->port_path_cost;
        om_ptr->bridge_info.root_priority.root_path_cost = root_path_cost;
        for (index=0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++ )
        {
            index_pom_ptr = &(om_ptr->port_info[index]);
            index_pom_ptr->designated_priority.root_path_cost = root_path_cost;
            if((index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
               || (index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DISABLED)
               || ((index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
                   && (root_path_cost  < index_pom_ptr->port_priority.root_path_cost))
              )
            {
                index_pom_ptr->port_priority.root_path_cost = root_path_cost;
            }
        }

        root_pom_ptr = &(om_ptr->port_info[root_lport-1]);
        root_pom_ptr->reselect = TRUE;
    }

    /* Alternate port */
    root_pom_ptr = &(om_ptr->port_info[root_lport-1]);
    if (    (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
         && ((pom_ptr->port_priority.root_path_cost +pom_ptr->port_path_cost) <= (root_pom_ptr->port_priority.root_path_cost + root_pom_ptr->port_path_cost))
       )
    {
        /* Initialize the port_priority to motivate the PIM state machine for reselect root_port. */
        XSTP_MGR_InitPortPriorityVector(om_ptr, lport);
        pom_ptr->reselect = TRUE;
    }
    #endif /* XSTP_TYPE_PROTOCOL_RSTP */

    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    /* static internal path cost */
    pom_ptr->static_internal_path_cost  = TRUE;
    pom_ptr->internal_port_path_cost    = path_cost;
    /* Root port. For consistency */
    root_lport = (UI32_T)(om_ptr->bridge_info.root_port_id.port_id & 0x0FFF);
    if ( root_lport == lport )
    {
        root_path_cost = pom_ptr->port_priority.int_root_path_cost + pom_ptr->internal_port_path_cost;
        om_ptr->bridge_info.root_priority.int_root_path_cost = root_path_cost;
        for (index=0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++ )
        {
            index_pom_ptr = &(om_ptr->port_info[index]);
            index_pom_ptr->designated_priority.int_root_path_cost = root_path_cost;
            if((index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
               || (index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DISABLED)
               || ((index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
                   && (root_path_cost  < index_pom_ptr->port_priority.int_root_path_cost))
              )
            index_pom_ptr->port_priority.int_root_path_cost = root_path_cost;
        }
        root_pom_ptr = &(om_ptr->port_info[root_lport-1]);
        root_pom_ptr->reselect = TRUE;
    }

    /* Alternate port */
    if((root_lport > 0)&&(root_lport <= XSTP_TYPE_MAX_NUM_OF_LPORT)){
        root_pom_ptr = &(om_ptr->port_info[root_lport-1]);
        if (    (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
             && ( (pom_ptr->port_priority.int_root_path_cost +pom_ptr->internal_port_path_cost ) <= (root_pom_ptr->port_priority.int_root_path_cost + root_pom_ptr->internal_port_path_cost))
           )
        {
            /* Initialize the port_priority to motivate the PIM state machine for reselect root_port. */
            XSTP_MGR_InitPortPriorityVector(om_ptr, lport);
            pom_ptr = &(om_ptr->port_info[root_lport-1]);
            pom_ptr->reselect = TRUE;
        }
    }
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    XSTP_ENGINE_StateMachineProgress(om_ptr);
    return XSTP_TYPE_RETURN_OK;

}/* End of XSTP_MGR_SetMstPortPathCost_() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetDot1dMstEntry_
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified spanning tree
 *              entry info can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   om_ptr          -- the pointer of the instance entry.
 * OUTPUT   :   entry           -- the specified mst entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
static BOOL_T XSTP_MGR_GetDot1dMstEntry_(XSTP_OM_InstanceData_T *om_ptr,
                                         XSTP_MGR_Dot1dStpEntry_T  *entry)
{
    UI32_T                  current_st_mode;
    UI16_T                  temp_priority;

    if (XSTP_OM_GetSpanningTreeStatus() == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
    {
        if (!om_ptr->instance_exist)
        {
            if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
                BACKDOOR_MGR_Printf("\r\nXSTP_MGR_SetMstPortPathCost::ERROR!! for invalid mstid.");
            return FALSE;
        }
    }
    memset(entry , 0 , sizeof(XSTP_MGR_Dot1dStpEntry_T));
    current_st_mode = (UI32_T)XSTP_OM_GetForceVersion();
    if (current_st_mode == XSTP_TYPE_STP_MODE)
    {
        entry->dot1d_stp_protocol_specification = XSTP_TYPE_SPEC_IEEE8021d;
    }
    else
    {
        entry->dot1d_stp_protocol_specification = XSTP_TYPE_SPEC_UNKNOWN;
    }
    entry->dot1d_stp_hold_time              = 1*XSTP_TYPE_TICK_TIME_UNIT;
    XSTP_OM_GET_BRIDGE_ID_PRIORITY(temp_priority, om_ptr->bridge_info.bridge_identifier);
    entry->dot1d_stp_priority = (UI32_T)temp_priority;
    entry->dot1d_stp_time_since_topology_change
                                            = (UI32_T)SYSFUN_GetSysTick()
                                              - om_ptr->bridge_info.time_since_topology_change;

    entry->dot1d_stp_top_changes        =(UI32_T)om_ptr->bridge_info.topology_change_count;

    #ifdef  XSTP_TYPE_PROTOCOL_RSTP
    memcpy((UI8_T *)&(entry->dot1d_stp_designated_root),
           (UI8_T *)&(om_ptr->bridge_info.root_priority.root_bridge_id), XSTP_TYPE_BRIDGE_ID_LENGTH);
    entry->dot1d_stp_root_cost          =(UI32_T)om_ptr->bridge_info.root_priority.root_path_cost;

    entry->dot1d_stp_max_age                =(UI16_T)(om_ptr->bridge_info.root_times.max_age)*XSTP_TYPE_TICK_TIME_UNIT;
    entry->dot1d_stp_hello_time             =(UI16_T)(om_ptr->bridge_info.root_times.hello_time)*XSTP_TYPE_TICK_TIME_UNIT;
    entry->dot1d_stp_forward_delay          =(UI16_T)(om_ptr->bridge_info.root_times.forward_delay)*XSTP_TYPE_TICK_TIME_UNIT;
    entry->dot1d_stp_bridge_max_age         =(UI16_T)(om_ptr->bridge_info.bridge_times.max_age)*XSTP_TYPE_TICK_TIME_UNIT;
    entry->dot1d_stp_bridge_hello_time      =(UI16_T)(om_ptr->bridge_info.bridge_times.hello_time)*XSTP_TYPE_TICK_TIME_UNIT;
    entry->dot1d_stp_bridge_forward_delay   =(UI16_T)(om_ptr->bridge_info.bridge_times.forward_delay)*XSTP_TYPE_TICK_TIME_UNIT;
    #endif /* XSTP_TYPE_PROTOCOL_RSTP */

    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    if (om_ptr->instance_id == XSTP_TYPE_CISTID)
    {
        memcpy((UI8_T *)&(entry->dot1d_stp_designated_root),
               (UI8_T *)&(om_ptr->bridge_info.root_priority.root_id), XSTP_TYPE_BRIDGE_ID_LENGTH);
        entry->dot1d_stp_root_cost          =(UI32_T)om_ptr->bridge_info.root_priority.ext_root_path_cost;

        entry->dot1d_stp_max_age                =(UI16_T)(om_ptr->bridge_info.root_times.max_age)*XSTP_TYPE_TICK_TIME_UNIT;
        entry->dot1d_stp_hello_time             =(UI16_T)(om_ptr->bridge_info.root_times.hello_time)*XSTP_TYPE_TICK_TIME_UNIT;
        entry->dot1d_stp_forward_delay          =(UI16_T)(om_ptr->bridge_info.root_times.forward_delay)*XSTP_TYPE_TICK_TIME_UNIT;
        entry->dot1d_stp_bridge_max_age         =(UI16_T)(om_ptr->bridge_info.bridge_times.max_age)*XSTP_TYPE_TICK_TIME_UNIT;
        entry->dot1d_stp_bridge_hello_time      =(UI16_T)(om_ptr->bridge_info.bridge_times.hello_time)*XSTP_TYPE_TICK_TIME_UNIT;
        entry->dot1d_stp_bridge_forward_delay   =(UI16_T)(om_ptr->bridge_info.bridge_times.forward_delay)*XSTP_TYPE_TICK_TIME_UNIT;
    }
    else
    {
        memcpy((UI8_T *)&(entry->dot1d_stp_designated_root),
               (UI8_T *)&(om_ptr->bridge_info.root_priority.r_root_id), XSTP_TYPE_BRIDGE_ID_LENGTH);
        entry->dot1d_stp_root_cost          =(UI32_T)om_ptr->bridge_info.root_priority.int_root_path_cost;

        entry->dot1d_stp_max_age                =(UI16_T)(om_ptr->bridge_info.cist->root_times.max_age)*XSTP_TYPE_TICK_TIME_UNIT;
        entry->dot1d_stp_hello_time             =(UI16_T)(om_ptr->bridge_info.cist->root_times.hello_time)*XSTP_TYPE_TICK_TIME_UNIT;
        entry->dot1d_stp_forward_delay          =(UI16_T)(om_ptr->bridge_info.cist->root_times.forward_delay)*XSTP_TYPE_TICK_TIME_UNIT;
        entry->dot1d_stp_bridge_max_age         =(UI16_T)(om_ptr->bridge_info.cist->bridge_times.max_age)*XSTP_TYPE_TICK_TIME_UNIT;
        entry->dot1d_stp_bridge_hello_time      =(UI16_T)(om_ptr->bridge_info.cist->bridge_times.hello_time)*XSTP_TYPE_TICK_TIME_UNIT;
        entry->dot1d_stp_bridge_forward_delay   =(UI16_T)(om_ptr->bridge_info.cist->bridge_times.forward_delay)*XSTP_TYPE_TICK_TIME_UNIT;
    }

    {
        XSTP_OM_InstanceData_T *cist_om_ptr;
        XSTP_OM_PortVar_T      *temp_pom_ptr;
        UI32_T                 next_lport;
        BOOL_T                 result;

        cist_om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
        memcpy((UI8_T *)&(entry->mstp_cist_regional_root_id),
               (UI8_T *)&(cist_om_ptr->bridge_info.root_priority.r_root_id), XSTP_TYPE_BRIDGE_ID_LENGTH);
        entry->mstp_cist_path_cost              =(UI32_T)cist_om_ptr->bridge_info.root_priority.int_root_path_cost;

        next_lport = 0;
        result     = FALSE;
        while ( (SWCTRL_GetNextLogicalPort(&next_lport) != SWCTRL_LPORT_UNKNOWN_PORT) && (!result))
        {
            temp_pom_ptr     = &(om_ptr->port_info[next_lport-1]);
            if (temp_pom_ptr->is_member)
            {
                if (temp_pom_ptr->tc_while != 0)
                {
                    result = TRUE;
                }
            }
        }
        entry->mstp_topology_change_in_progress = result;
    }
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    entry->dot1d_stp_root_port          =(UI16_T)(om_ptr->bridge_info.root_port_id.port_id & 0x0FFF);
    entry->dot1d_stp_version            =(UI8_T)current_st_mode;
    entry->dot1d_stp_tx_hold_count      =(UI8_T)om_ptr->bridge_info.common->tx_hold_count;
    entry->dot1d_stp_path_cost_default  = (UI8_T)XSTP_OM_GetPathCostMethod();

    return TRUE;

}/* End of XSTP_MGR_GetDot1dMstEntry_() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMstpMaxHop_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set MSTP Max_Hop count.
 * INPUT    :   UI32_T hop_count     -- max_hop value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK              -- set successfully
 *              XSTP_TYPE_RETURN_ERROR           -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_INDEX_OOR       -- hop_count out of range
 * NOTE     :   Range   : 1 ~ 40
 *              -- XSTP_TYPE_MSTP_MIN_MAXHOP
 *              -- XSTP_TYPE_MSTP_MAX_MAXHOP
 *              Default : 20
 *              -- XSTP_TYPE_DEFAULT_BRIDGE_MAXHOP
 * ------------------------------------------------------------------------
 */
static UI32_T XSTP_MGR_SetMstpMaxHop_(UI32_T hop_count)
{
    UI32_T                  current_max_hop;

    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    UI32_T                  mstid;
    XSTP_OM_InstanceData_T  *mstid_om_ptr;
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    current_max_hop = XSTP_OM_GetMaxHopCount();
    if(current_max_hop == hop_count)
    {
        return XSTP_TYPE_RETURN_OK;
    }

    XSTP_OM_SetMaxHopCount(hop_count);

    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    mstid               = XSTP_TYPE_CISTID;
    mstid_om_ptr        = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    do
    {
        if (mstid_om_ptr->instance_exist)
        {
            mstid_om_ptr->bridge_info.bridge_times.remaining_hops = (UI8_T)hop_count;
            if (XSTP_MGR_IsRootBridge(mstid_om_ptr)==TRUE)
            {
                UI16_T                  index;
                mstid_om_ptr->bridge_info.root_times.remaining_hops = (UI8_T)hop_count;
                for (index=0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++ )
                {
                    XSTP_OM_PortVar_T       *pom_ptr;
                    pom_ptr = &(mstid_om_ptr->port_info[index]);
                    pom_ptr->port_times.remaining_hops = pom_ptr->designated_times.remaining_hops
                                                       = (UI8_T)hop_count;
                }
            }
        }
    }while(XSTP_OM_GetNextInstanceInfoPtr(&mstid, &mstid_om_ptr));
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    return XSTP_TYPE_RETURN_OK;

}/* End of XSTP_MGR_SetMstpMaxHop_() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetVlanToMstConfigTable_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Map vlan to an instance for mst configuration table.
 * INPUT    :   mstid              -- the instance entry.
 *              UI32_T vlan         -- vlan value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK             -- set successfully
 *              XSTP_TYPE_RETURN_ERROR          -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_ST_STATUS_DISABLED -- status is disabled
 *              XSTP_TYPE_RETURN_INDEX_OOR      -- mstid out of range
 *              XSTP_TYPE_RETURN_INDEX_NEX      -- vlan not existed
 * NOTE     :   Default                    -- All vlans join MST instance 0
 * ------------------------------------------------------------------------
 */
static UI32_T  XSTP_MGR_SetVlanToMstConfigTable_(UI32_T mstid,
                                                 UI32_T vlan)
{
    UI32_T                  vlan_mstid;
    UI32_T                  current_st_mode;
    UI32_T                  current_st_status;
    char                    arg_buf[32];

    if (vlan < 1 || vlan > XSTP_TYPE_SYS_MAX_VLAN_ID){
        snprintf(arg_buf, 32, "VLAN id (1-%d)",XSTP_TYPE_SYS_MAX_VLAN_ID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetVlanToMstConfigTable_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }

    if (mstid == XSTP_TYPE_CISTID){
        return XSTP_TYPE_RETURN_OK;
    }

    /* per_vlan per_instance*/
    XSTP_OM_GetMstidFromMstConfigurationTableByVlan(vlan, &vlan_mstid);

    if ((vlan_mstid != XSTP_TYPE_CISTID) && (vlan_mstid != mstid))
    {
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
            BACKDOOR_MGR_Printf("\r\nXSTP_MGR_SetVlanToMstConfigTable::ERROR!! the vlan[%ld] has mapped to the instance[%ld]", (long)vlan, (long)vlan_mstid);
        snprintf(arg_buf, 32, "already a member of instance %ld",(long)vlan_mstid );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetVlanToMstConfigTable_Fun_No, EH_TYPE_MSG_VLAN_INST_ID_SET_FAIL, SYSLOG_LEVEL_INFO, arg_buf);
        return XSTP_TYPE_RETURN_ERROR;
    }

    if (!XSTP_OM_SetEntryOfMstid(mstid, TRUE)){
        return XSTP_TYPE_RETURN_ERROR;
    }
    XSTP_OM_SetMstidToMstConfigurationTableByVlan(vlan, mstid);

#ifdef  XSTP_TYPE_PROTOCOL_MSTP
    {
        XSTP_OM_InstanceData_T  *om_ptr;
        om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
        XSTP_OM_GenerateConfigurationDigest(om_ptr);
    }
    /* om_ptr->bridge_info.common->restart_state_machine =TRUE; */
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

    current_st_status = XSTP_OM_GetSpanningTreeStatus();
    current_st_mode = (UI32_T)XSTP_OM_GetForceVersion();

    /* If the specified vlan has already existed, let it join the MSTI
     * according to the configuration table when spanning tree is enabled and mode is mstp.
     */
    if (    (VLAN_OM_IsVlanExisted(vlan))
        &&  (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
        &&  (current_st_mode == XSTP_TYPE_MSTP_MODE)
       )
    {
        XSTP_MGR_SetVlanToMstMappingTable(vlan);
    }

    return XSTP_TYPE_RETURN_OK;

}/* End of XSTP_MGR_SetVlanToMstConfigTable_() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_AttachVlanToMstConfigTable_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Attach the vlan to the new instance.
 * INPUT    :   UI32_T mstid               -- instance value
 *              UI32_T vlan                -- vlan value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
 *              XSTP_TYPE_RETURN_ERROR              -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_ST_STATUS_DISABLED -- status is disabled
 *              XSTP_TYPE_RETURN_INDEX_OOR          -- mstid out of range
 *              XSTP_TYPE_RETURN_INDEX_NEX          -- vlan not existed
 * NOTE     :   1. All vlans will join MST instance 0 by default.
 *              2. This API will automatically move this VLAN from old instance to new instance.
 *              3. This API shall only be invoked by the folowing APIs.
 *                 -- XSTP_MGR_AttachVlanListToMstConfigTable()
 *                 -- XSTP_MGR_AttachVlanToMstConfigTable()
 * ------------------------------------------------------------------------
 */
static UI32_T XSTP_MGR_AttachVlanToMstConfigTable_(UI32_T mstid,
                                                   UI32_T vlan)
{
    UI32_T                  current_st_mode;
    UI32_T                  current_st_status;
    UI32_T                  result;
    char                    arg_buf[20];
    UI32_T                  old_mstid;
    UI32_T                  vlan_id;

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
        snprintf(arg_buf, 20, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_AttachVlanToMstConfigTable_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }

    if (vlan < 1 || vlan > XSTP_TYPE_SYS_MAX_VLAN_ID)
    {
        snprintf(arg_buf, 20, "VLAN id (1-%d)",XSTP_TYPE_SYS_MAX_VLAN_ID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_AttachVlanToMstConfigTable_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }

    result              = XSTP_TYPE_RETURN_OK;
    current_st_status = XSTP_OM_GetSpanningTreeStatus();
    current_st_mode     = (UI32_T)XSTP_OM_GetForceVersion();

    XSTP_OM_GetMstidFromMstConfigurationTableByVlan(vlan, &old_mstid);
    if (old_mstid != mstid)
    {
        /* 1. Remove specified vlan from old instance */
        if (old_mstid != XSTP_TYPE_CISTID)
        {
            /* 1.1 If the specified vlan has already existed, remove it from the MSTI
             *     according to the configuration table when spanning tree is enabled and mode is mstp.
             */
            if (    (VLAN_OM_IsVlanExisted(vlan))
                &&  (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
                &&  (current_st_mode == XSTP_TYPE_MSTP_MODE)
               )
            {
                XSTP_MGR_RemoveVlanFromMstMappingTable(vlan);
            }
            /* 1.2 Attach the specified vlan to the instance 0 in the mst configuration table */
            XSTP_OM_SetMstidToMstConfigurationTableByVlan(vlan, XSTP_TYPE_CISTID);
            /* 1.3 Release mstid entry if old instance does not have any vlan member in the mst configuration table*/
            vlan_id = 0;
            if (!XSTP_OM_GetNextXstpMemberFromMstConfigurationTable(old_mstid, &vlan_id))
            {
                XSTP_OM_SetEntryOfMstid(old_mstid, FALSE);
            }
        }
        /* 2. Set this vlan to new instance */
        /* 2.1 Allocate an unused mstid entry if mstid is not instance 0 */
        if (    (mstid != XSTP_TYPE_CISTID)
            &&  (!XSTP_OM_SetEntryOfMstid(mstid, TRUE))
           )
        {
            result = XSTP_TYPE_RETURN_ERROR;
        }
        else
        {
            /* 2.2 Attach the specified vlan to new instance in the mst configuration table */
            XSTP_OM_SetMstidToMstConfigurationTableByVlan(vlan, mstid);

            /* 2.3 If the specified vlan has already existed, let it join the MSTI
             *     according to the configuration table when spanning tree is enabled and mode is mstp..
             */
            if (    (VLAN_OM_IsVlanExisted(vlan))
                &&  (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
                &&  (current_st_mode == XSTP_TYPE_MSTP_MODE)
               )
            {
                XSTP_MGR_SetVlanToMstMappingTable(vlan);
            }
        }
    }
    return result;
}/* End of XSTP_MGR_AttachVlanToMstConfigTable_() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetVlanToMstMappingTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   Map an existent vlan to an instance for mst mapping table.
 * INPUT    :   UI32_T vlan                -- vlan value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK             -- set successfully
 *              XSTP_TYPE_RETURN_ERROR          -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_INDEX_NEX      -- vlan not existed
 * NOTE     :   1. All created vlans have to join the CIST.
 *              2. The MSTI which the vlan is trying to join is created if
 *                 it does not exist.
 *              3. The specified vlan_id should guarantee that it hasn't
 *                 joined any instance except CIST.
 * ------------------------------------------------------------------------
 */
static  UI32_T  XSTP_MGR_SetVlanToMstMappingTable(UI32_T vlan_id)
{
    UI32_T                  current_st_mode;
    UI32_T                  current_st_status;
    XSTP_OM_InstanceData_T  *om_ptr;
    UI32_T                  mstid;
    BOOL_T                  exist;
    UI32_T                  lport;
    XSTP_OM_PortVar_T       *pom_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    if(!VLAN_OM_IsVlanExisted(vlan_id)){
        return XSTP_TYPE_RETURN_INDEX_NEX;
    }

    current_st_mode     = (UI32_T)XSTP_OM_GetForceVersion();

    XSTP_OM_GetMstidFromMstConfigurationTableByVlan(vlan_id, &mstid);

    current_st_status = XSTP_OM_GetSpanningTreeStatus();
    /*
        * ERP:ES4827G-FLF-ZZ-00471
        *Problem:
        *     ref the bug discription (ES4827G-FLF-ZZ-00471)
        *Reason :
        *    xstp database of instance_vlans_mapped is not correct. the action in the older codes
        *        symmetrial.
        *Solution:
        *         modify vlan and instance bind and unbind sort
        *         adding steps as below:
        *        1. when create a new vlan, map the vlan to correct instance
        *         2. when config a vlan(existing vlan)to instance A, update the vlan bit to instance A,
        *            and unset the bit in cist instance
        *         removing steps as below:
        *         1, when destroy a existing vlan, remove the vlan bit map from correspending instance
        *         2, when un config a vlan(existing vlan) from instance A, unset the bit in A,
        *            and set the bit in cist instance
        *  the setting rules:
        *                 change the design, when vlan exists,
        *                 the vlan mapping is the same to mstp config
        *                  (rstp or stp , all vlans are mapped to cist )
        */

    if ((current_st_mode == XSTP_TYPE_MSTP_MODE)
        && (mstid != XSTP_TYPE_CISTID)){
        om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
        if(om_ptr)
            XSTP_OM_SetVlanToMstidMappingTable(om_ptr, vlan_id, FALSE);
        else
            return XSTP_TYPE_RETURN_ERROR;

        om_ptr = XSTP_OM_GetInstanceInfoPtr(mstid);
        if(om_ptr)
            XSTP_OM_SetVlanToMstidMappingTable(om_ptr, vlan_id, TRUE);
        else
            return XSTP_TYPE_RETURN_ERROR;

        exist   =   om_ptr->instance_exist;

        if (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED){
            if (exist){
                /* Put the member ports of the vlan into the instance
                             * if the port isn't a member of this instance
                             */
                lport   = 0;
                while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT){

                    UI32_T  vid_ifindex;
                    pom_ptr             = &(om_ptr->port_info[lport-1]);

                    VLAN_VID_CONVERTTO_IFINDEX(vlan_id, vid_ifindex);

                    if((XSTP_OM_IsMstFullMemberTopology())
                        || (VLAN_OM_IsPortVlanMember(vid_ifindex, lport))){

                        pom_ptr->is_member  = TRUE;
                        /* Set port state if its spanning_tree_status is disabled */
                        if (pom_ptr->common->port_spanning_tree_status == VAL_staPortSystemStatus_disabled){

                            if ((pom_ptr->common->link_up) && (pom_ptr->common->port_enabled)){

                                XSTP_ENGINE_SetPortState(om_ptr, lport, TRUE, TRUE);
                            }else{

                                XSTP_ENGINE_SetPortState(om_ptr, lport, FALSE, FALSE);
                            }
                        }
                    } /* End of if (VLAN_OM_IsPortVlanMember) */
#if 0
                    /* Remove unnecessary code. New design is that XSTP map vlan and set port state by mstidx.
                     * Can many vlans map to one STG in chip layer, so XSTP don't need to sync port state.
                     */
                    /* MSTI: Port state corresponding to this vlan shall be set for all the ports.*/
                    if (pom_ptr->is_member){

                        XSTP_ENGINE_SetPortState(om_ptr, lport, pom_ptr->learning, pom_ptr->forwarding);
                    }
#endif
                } /* End of while (SWCTRL_GetNextLogicalPort) */
            }
            else{

                XSTP_MGR_CreateSpanningTree(om_ptr);
            } /* End of if (Instance_not_exist) */

            /* It is unnecessary because state machine will be restarted.*/
#if 0
            XSTP_ENGINE_StateMachineProgress(om_ptr);
#endif
        } /* End of if (SpanningTree_not_enabled) */

    }else{
        om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
        XSTP_OM_SetVlanToMstidMappingTable(om_ptr, vlan_id, TRUE);
    }

#ifdef  XSTP_TYPE_PROTOCOL_MSTP
    {
        UI32_T mstidx;

        if(current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED){

            mstidx = (UI32_T) XSTP_OM_GetInstanceEntryId(mstid);
        }else{

            mstidx = (UI32_T) XSTP_OM_GetInstanceEntryId(XSTP_TYPE_CISTID);
        }

        if (!SWCTRL_PMGR_AddVlanToMst(vlan_id, mstidx)){

            return XSTP_TYPE_RETURN_ERROR;
        }
    }
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

    return XSTP_TYPE_RETURN_OK;
} /* End of XSTP_MGR_SetVlanToMstMappingTable */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_RemoveVlanFromMstMappingTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   Remove vlan from an MSTI for the mst mapping table.
 * INPUT    :   UI32_T vlan                -- vlan value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK             -- set successfully
 *              XSTP_TYPE_RETURN_ERROR          -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_INDEX_OOR      -- mstid out of range
 *              XSTP_TYPE_RETURN_INDEX_NEX      -- vlan not existed
 * NOTE     :   1. If vlan = 0, remove the specified instance
 *              2. An existing vlan_id is the member of CIST forever.
 * ------------------------------------------------------------------------
 */
static UI32_T XSTP_MGR_RemoveVlanFromMstMappingTable(UI32_T vlan_id)
{
    UI32_T                  current_st_mode;
    UI32_T                  current_st_status;
    XSTP_OM_InstanceData_T  *om_ptr;
    UI32_T                  mstid;
    BOOL_T                  next_member_exist;
    UI32_T                  next_vlan_id;


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE){

        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    current_st_mode     = (UI32_T)XSTP_OM_GetForceVersion();
    XSTP_OM_GetMstidFromMstConfigurationTableByVlan(vlan_id, &mstid);
    /*
        * ERP:ES4827G-FLF-ZZ-00471
        *Problem:
        *     ref the bug discription (ES4827G-FLF-ZZ-00471)
        *Reason :
        *    xstp database of instance_vlans_mapped is not correct. the action in the older codes
        *        symmetrial.
        *Solution:
        *         modify vlan and instance bind and unbind sort
        *         adding steps as below:
        *        1. when create a new vlan, map the vlan to correct instance
        *         2. when config a vlan(existing vlan)to instance A, update the vlan bit to instance A,
        *            and unset the bit in cist instance
        *         removing steps as below:
        *         1, when destroy a existing vlan, remove the vlan bit map from correspending instance
        *         2, when un config a vlan(existing vlan) from instance A, unset the bit in A,
        *            and set the bit in cist instance
        *    the remove order is :
        *      when remove the vlan instance mapping,
        *      first remove from current databse,
        *      then from config table
        */
    if ((current_st_mode == XSTP_TYPE_MSTP_MODE)
        && (mstid != XSTP_TYPE_CISTID)){

        om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
        XSTP_OM_SetVlanToMstidMappingTable(om_ptr, vlan_id, TRUE);

        om_ptr = XSTP_OM_GetInstanceInfoPtr(mstid);
        XSTP_OM_SetVlanToMstidMappingTable(om_ptr, vlan_id, FALSE);

        next_vlan_id = 0;
        next_member_exist = XSTP_OM_GetNextXstpMember(om_ptr, &next_vlan_id);

        current_st_status = XSTP_OM_GetSpanningTreeStatus();

        if (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED){

            if (next_member_exist){

                UI32_T lport;
                /* Remove those ports which leave this instance */
                om_ptr = XSTP_OM_GetInstanceInfoPtr(mstid);

                lport = 0;
                while(SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT){

                    UI32_T  vid_ifindex;
                    UI32_T  vid;

                    VLAN_VID_CONVERTTO_IFINDEX(vlan_id, vid_ifindex);
                    if((XSTP_OM_IsMstFullMemberTopology())
                        || (VLAN_OM_IsPortVlanMember(vid_ifindex, lport))){

                        BOOL_T  not_found;
                        /* Check if this port is still a member of this instance */
                        vid         = 0;
                        not_found   = TRUE;

                        while(XSTP_OM_GetNextXstpMember(om_ptr,&vid)){

                            UI32_T  temp_vid_ifindex;
                            VLAN_VID_CONVERTTO_IFINDEX(vid, temp_vid_ifindex);

                            if(VLAN_OM_IsPortVlanMember(temp_vid_ifindex, lport)){

                                not_found   = FALSE;
                                break;
                            }
                        } /* End of while (GetNextXstpMember) */

                        if (not_found){

                            XSTP_OM_PortVar_T   *pom_ptr;
                            pom_ptr             = &(om_ptr->port_info[lport-1]);
                            pom_ptr->is_member  = FALSE;
                        } /* End of if (this_port_is_not_a_member_of_this_instance) */
                    } /* End of if (VLAN_OM_IsPortVlanMember) */
                } /* End of while (SWCTRL_GetNextLogicalPort) */
            }else{
                /* Delete this instance */
                XSTP_MGR_DeleteSpanningTree(om_ptr);
            } /* End of if (!next_member_exist) */
        } /* End of if (SpanningTree_not_enabled) */

    } /* End of if (mstid != XSTP_TYPE_CISTID) */
#ifdef  XSTP_TYPE_PROTOCOL_MSTP
    {
        UI32_T mstidx;
        mstidx = (UI32_T)XSTP_OM_GetInstanceEntryId(XSTP_TYPE_CISTID);

        if (!SWCTRL_PMGR_AddVlanToMst(vlan_id,mstidx)){
            return XSTP_TYPE_RETURN_ERROR;
        }
    }
#endif /* XSTP_TYPE_PROTOCOL_MSTP */
    return XSTP_TYPE_RETURN_OK;
} /* End of XSTP_MGR_RemoveVlanFromMstMappingTable */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_CreateSpanningTree
 *-------------------------------------------------------------------------
 * PURPOSE  : Create the specified spanning tree
 * INPUT    : om_ptr              -- the pointer of the instance entry.
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void XSTP_MGR_CreateSpanningTree(XSTP_OM_InstanceData_T  *om_ptr)
{
    char                    region_name[XSTP_TYPE_REGION_NAME_MAX_LENGTH + 1];
    char                    cpu_mac[XSTP_TYPE_REGION_NAME_MAX_LENGTH+1];
    UI32_T                  xstid;
    UI32_T                  lport;
    XSTP_OM_PortVar_T       *pom_ptr;

    /* xstid begins at 0;
     * xstid = 0 is reserved for CST.
     */
    xstid               = om_ptr->instance_id;
    if (xstid > XSTP_TYPE_MAX_MSTID)
        return;

    XSTP_OM_CreateTreeOm(om_ptr);

    memset(region_name, 0, XSTP_TYPE_REGION_NAME_MAX_LENGTH+1);
    memset(cpu_mac, 0, XSTP_TYPE_REGION_NAME_MAX_LENGTH+1);
    XSTP_OM_GetRegionName(region_name);

    /* 13.23.8 mst_config_id */
    #ifdef XSTP_TYPE_PROTOCOL_RSTP
    if (    (xstid == XSTP_TYPE_CISTID)
        &&  (region_name[0] == 0)
        )
    {
        XSTP_OM_CpuMacToString(cpu_mac);
        XSTP_MGR_SetMstpConfigurationName_(om_ptr, cpu_mac);
    }
    #endif /* XSTP_TYPE_PROTOCOL_RSTP */

    #ifdef XSTP_TYPE_PROTOCOL_MSTP
    if (    (xstid == XSTP_TYPE_CISTID)
        &&  (   (region_name[0] == 0)
             || (om_ptr->bridge_info.common->mst_config_id.config_name[0] == 0)
            )
        )
    {
        XSTP_OM_CpuMacToString(cpu_mac);
        XSTP_MGR_SetMstpConfigurationName_(om_ptr, cpu_mac);
    }
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    XSTP_ENGINE_InitStateMachine(om_ptr, xstid);

    /* Notify Vlan that all ports enter blocking if the spanning tree has already been enabled */
    if (xstid == XSTP_TYPE_CISTID)
    {
        lport = 0;
        while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
        {
            VLAN_PMGR_NotifyForwardingState(0, lport, FALSE);
            if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
            {
                BACKDOOR_MGR_Printf("\r\nXSTP_MGR_CreateSpanningTree::Notify Leave Forwarding to Vlan:for lport %ld in all Vlans", lport);
            }
        }
    }

    /* Set port state if its spanning_tree_status is disabled */
    lport = 0;
    while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        pom_ptr = &(om_ptr->port_info[lport-1]);
        if (    (pom_ptr->is_member)
            &&  (pom_ptr->common->port_spanning_tree_status == VAL_staPortSystemStatus_disabled)
           )
        {
            if ((pom_ptr->common->link_up) && (pom_ptr->common->port_enabled))
            {
                XSTP_ENGINE_SetPortState(om_ptr, lport, TRUE, TRUE);
            }
            else
            {
                XSTP_ENGINE_SetPortState(om_ptr, lport, FALSE, FALSE);
            }
        }
    }
    return;
} /* End of XSTP_MGR_CreateSpanningTree */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_DeleteSpanningTree
 *-------------------------------------------------------------------------
 * PURPOSE  : Delete the specified spanning tree
 * INPUT    : om_ptr              -- the pointer of the instance entry.
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void XSTP_MGR_DeleteSpanningTree(XSTP_OM_InstanceData_T  *om_ptr)
{
    UI32_T                  xstid;
    UI32_T                  vid;

    /* xstid begins at 0;
     * xstid = 0 is reserved for CST.
     */
    xstid               = om_ptr->instance_id;
    if (xstid > XSTP_TYPE_MAX_MSTID)
        return;

    XSTP_OM_NullifyInstance(om_ptr);

    if (xstid == XSTP_TYPE_CISTID)
    {
        XSTP_OM_AddExistingMemberToInstance(om_ptr);
        for(vid =1; vid <= XSTP_TYPE_SYS_MAX_VLAN_ID; vid++)
        {
            if(VLAN_OM_IsVlanExisted(vid))
            {
                XSTP_OM_SetVlanToMstidMappingTable(om_ptr, vid, TRUE);
            }
        }
    }

    return;
} /* End of XSTP_MGR_DeleteSpanningTree */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_InitPortPriorityVector
 * ------------------------------------------------------------------------
 * PURPOSE  :   Initialize the port priority vectore variables
 * INPUT    :   om_ptr           -- the pointer of the instance entry.
 *              lport            -- lport number
 * OUTPUT   :   None.
 * RETURN   :   None.
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
static void XSTP_MGR_InitPortPriorityVector(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T               *pom_ptr;

    pom_ptr = &(om_ptr->port_info[lport-1]);
    #ifdef  XSTP_TYPE_PROTOCOL_RSTP
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
                                        0,
                                        om_ptr->bridge_info.bridge_identifier,
                                        0,
                                        om_ptr->bridge_info.bridge_identifier,
                                        pom_ptr->port_id,
                                        pom_ptr->port_id);
    }
    #endif  /* XSTP_TYPE_PROTOCOL_MSTP */
} /* End of XSTP_MGR_InitPortPriorityVector() */


/* ===================================================================== */
/* ===================================================================== */
/* IEEE 802.1S */
#ifdef  XSTP_TYPE_PROTOCOL_MSTP

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_RecoverExistingMSTI
 *-------------------------------------------------------------------------
 * PURPOSE  : Create other existing spanning trees except CIST.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void XSTP_MGR_RecoverExistingMSTI(void)
{
    UI32_T                  vid, lport, vid_ifindex;
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    UI32_T                  mstid;


    /*for(vid =1; vid <= XSTP_TYPE_SYS_MAX_VLAN_ID; vid++)*/
    vid = 0;
    while (VLAN_OM_GetNextVlanId(0, &vid) )
    {
        XSTP_OM_GetMstidFromMstConfigurationTableByVlan(vid, &mstid);
        if (mstid != XSTP_TYPE_CISTID)
        {
            if(VLAN_OM_IsVlanExisted(vid))
            {
                om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
                if (!om_ptr->instance_exist)
                {
                    XSTP_MGR_CreateSpanningTree(om_ptr);
                }
                XSTP_OM_SetVlanToMstidMappingTable(om_ptr, vid, TRUE);
                lport   = 0;
                while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
                {
                    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
                    if (    (XSTP_OM_IsMstFullMemberTopology())
                        ||  (VLAN_OM_IsPortVlanMember(vid_ifindex, lport))
                       )
                    {
                        pom_ptr             = &(om_ptr->port_info[lport-1]);
                        pom_ptr->is_member  = TRUE;
                    } /* End of if (VLAN_OM_IsPortVlanMember) */
                } /* End of while (SWCTRL_GetNextLogicalPort) */
                {
                    UI32_T                  mstidx;
                    mstidx = (UI32_T) XSTP_OM_GetInstanceEntryId(mstid);
                    SWCTRL_PMGR_AddVlanToMst(vid, mstidx);
                }
            }
        }
    }
    return;
} /* End of XSTP_MGR_RecoverExistingMSTI() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPortPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the path_cost of the port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T path_cost        -- the path_cost value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   1. When mode is STP or RSTP
 *                 -- For mstid = 0
 *                    Set the path cost value to the specified port.
 *              2. When mode is MSTP
 *                 -- For all instances
 *                    Set the path cost value to the specified port.
 *              3. In long mode (VAL_dot1dStpPathCostDefault_stp8021t2001)
 *                 -- Range : 0 ~ 200000000
 *                    XSTP_TYPE_MIN_PORT_PATH_COST
 *                    XSTP_TYPE_MAX_PORT_PATH_COST_32
 *              4. In short mode (VAL_dot1dStpPathCostDefault_stp8021d1998)
 *                 -- Range : 0 ~ 65535
 *                    XSTP_TYPE_MIN_PORT_PATH_COST
 *                    XSTP_TYPE_MAX_PORT_PATH_COST_16
 * REF      :   RFC-1493/dot1dStpPortEntry 5
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetPortPathCost(UI32_T lport,
                                UI32_T path_cost)
{
    UI32_T                  current_path_cost_method;
    UI32_T                  max_path_cost;
    XSTP_OM_InstanceData_T  *om_ptr;


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetPortPathCost_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
        return XSTP_TYPE_RETURN_PORTNO_OOR;
    }

    current_path_cost_method = XSTP_OM_GetPathCostMethod();
    if (current_path_cost_method == XSTP_TYPE_PATH_COST_DEFAULT_SHORT)
    {
        max_path_cost = XSTP_TYPE_MAX_PORT_PATH_COST_16;
    }
    else /* current_path_cost_method == XSTP_TYPE_PATH_COST_DEFAULT_LONG */
    {
        max_path_cost = XSTP_TYPE_MAX_PORT_PATH_COST_32;
    }

    if (path_cost < XSTP_TYPE_MIN_PORT_PATH_COST || path_cost > max_path_cost )
    {
        if (current_path_cost_method == XSTP_TYPE_PATH_COST_DEFAULT_SHORT)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetPortPathCost_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Short cost (1-65535)");
        }
        else if (current_path_cost_method == XSTP_TYPE_PATH_COST_DEFAULT_LONG)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetPortPathCost_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Long cost (1-200000000)");
        }
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    XSTP_MGR_SetPortPathCost_(om_ptr, lport, path_cost);
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
    XSTP_MGR_StateMachineProgress(XSTP_TYPE_CISTID);
    return XSTP_TYPE_RETURN_OK;
}/* End of XSTP_MGR_SetPortPathCost */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetRunningPortPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the external path_cost of the port for CIST.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *ext_path_cost   -- pointer of the path_cost value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * REF      :   RFC-1493/dot1dStpPortEntry 5
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_GetRunningPortPathCost(UI32_T lport,
                                       UI32_T *ext_path_cost)
{
    UI32_T                  default_path_cost;
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    BOOL_T                  static_path_cost_flag;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    if (XSTP_OM_IsMemberPortOfInstance (om_ptr, lport)==FALSE)
    {
        /* +++ LeaveCriticalRegion +++ */
        XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    pom_ptr = &(om_ptr->port_info[lport-1]);
    *ext_path_cost = (UI32_T)pom_ptr->common->external_port_path_cost;
    static_path_cost_flag = pom_ptr->common->static_external_path_cost;
    XSTP_OM_GetLportDefaultPathCost(lport, &default_path_cost);
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    if (!static_path_cost_flag)
    {
        if (*ext_path_cost == default_path_cost)
        {
                return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        }
        else
        {
                return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
}/* XSTP_MGR_GetRunningPortPathCost */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_RestartStateMachine
 *-------------------------------------------------------------------------
 * PURPOSE  : If restart_state_machine flag is TRUE, retart state machine.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : Call the function when user leave the spa mst config mode.
 * ------------------------------------------------------------------------
 */
void XSTP_MGR_RestartStateMachine(void)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    UI32_T                  current_st_mode;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);

    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    current_st_mode = (UI32_T)XSTP_OM_GetForceVersion();

    if(     (current_st_mode == XSTP_TYPE_MSTP_MODE)
       &&   (om_ptr->bridge_info.common->restart_state_machine)
      )
    {
        XSTP_ENGINE_RestartStateMachine();
        om_ptr->bridge_info.common->restart_state_machine = FALSE;
    }

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    return;
}/* XSTP_MGR_RestartStateMachine */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMstPortAutoPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Restore the default internal path_cost of the port for a instance.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   internal_port_path_cost
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetMstPortAutoPathCost(UI32_T lport,
                                       UI32_T mstid)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    UI32_T                  result;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstPortAutoPathCost_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
        return XSTP_TYPE_RETURN_PORTNO_OOR;
    }

    result = XSTP_TYPE_RETURN_OK;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(mstid);
    om_ptr = XSTP_OM_GetInstanceInfoPtr(mstid);
    result = XSTP_MGR_SetMstPortAutoPathCost_(om_ptr, lport);
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(mstid);
    XSTP_MGR_StateMachineProgress(mstid);
    return result;
}/* End of XSTP_MGR_SetMstPortAutoPathCost */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMstpRowStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if row status field of the entry can be
 *            set successfully.  Otherwise, return false.
 * INPUT    : mstid       -- instance value
 *            row_status  -- VAL_dot1qVlanStaticRowStatus_active
 *                           VAL_dot1qVlanStaticRowStatus_notInService
 *                           VAL_dot1qVlanStaticRowStatus_notReady
 *                           VAL_dot1qVlanStaticRowStatus_createAndGo
 *                           VAL_dot1qVlanStaticRowStatus_createAndWait
 *                           VAL_dot1qVlanStaticRowStatus_destroy
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T XSTP_MGR_SetMstpRowStatus(UI32_T mstid, UI32_T row_status)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    UI32_T                  next_vlan;
    BOOL_T                  result;
    char                    arg_buf[20];
    UI32_T                  current_row_status;

    /*SYSFUN_USE_CSC(FALSE);*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if ( mstid < 0 || mstid > XSTP_TYPE_MAX_MSTID )
    {
        snprintf(arg_buf, 20, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstpMstiRowStatus_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
        return FALSE;
    }

    if ((row_status < VAL_dot1qVlanStaticRowStatus_active) ||
        (row_status > VAL_dot1qVlanStaticRowStatus_destroy))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstpMstiRowStatus_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "row status");
        return FALSE;
    }

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(mstid);
    om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
    result              = TRUE;
    switch (row_status)
    {
        case VAL_dot1qVlanStaticRowStatus_active:
            if (om_ptr->row_status != VAL_dot1qVlanStaticRowStatus_active)
            {
                result = FALSE;
            }
            break;
        case VAL_dot1qVlanStaticRowStatus_createAndGo:
            break;
        case VAL_dot1qVlanStaticRowStatus_notInService:
        case VAL_dot1qVlanStaticRowStatus_notReady:
        case VAL_dot1qVlanStaticRowStatus_createAndWait:
            result = FALSE;
            break;
        case VAL_dot1qVlanStaticRowStatus_destroy:
            next_vlan = 0;
            if (XSTP_OM_GetNextXstpMemberFromMstConfigurationTable(mstid, &next_vlan))
            {
                result = FALSE;
            }
            break;
        default:
            result = FALSE;
            break;
    }/* end of switch */

    if (result)
    {
        current_row_status = om_ptr->row_status;
        switch (L_RSTATUS_Fsm(row_status, &current_row_status, XSTP_MGR_SemanticCheck, om_ptr))
        {
            case L_RSTATUS_NOTEXIST_2_ACTIVE:
            case L_RSTATUS_ACTIVE_2_NOTEXIST:
                om_ptr->row_status          = current_row_status;
                break;
            case L_RSTATUS_NOTEXIST_2_NOTEXIST:
            case L_RSTATUS_NOTEXIST_2_NOTREADY:
            case L_RSTATUS_NOTREADY_2_NOTEXIST:
            case L_RSTATUS_NOTREADY_2_NOTREADY:
            case L_RSTATUS_NOTREADY_2_ACTIVE:
            case L_RSTATUS_ACTIVE_2_NOTREADY:
            case L_RSTATUS_ACTIVE_2_ACTIVE:
                break;
            case L_RSTATUS_TRANSITION_STATE_ERROR:
            default:
                result = FALSE;
                break;
        }
    }
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(mstid);
    return result;
}/* End of XSTP_MGR_SetMstpRowStatus() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SemanticCheck
 *--------------------------------------------------------------------------
 * PURPOSE  : check whether row status can be "active" for this instance.
 * INPUT    : *om_ptr                  -- the pointer of info entry
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : row status can be made to "active" only under one of the following
 *            conditions:
 *            1.
 *            2.
 *-------------------------------------------------------------------------- */
static BOOL_T XSTP_MGR_SemanticCheck(void *om_ptr)
{
    return TRUE;
} /* XSTP_MGR_SemanticCheck() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPortPathCost_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the path_cost of the port.
 * INPUT    :   om_ptr    -- om pointer for this instance
 *              UI32_T lport            -- lport number
 *              UI32_T path_cost        -- the path_cost value
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   None
 * REF      :   RFC-1493/dot1dStpPortEntry 5
 * ------------------------------------------------------------------------
 */
static void XSTP_MGR_SetPortPathCost_(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, UI32_T path_cost)
{
    UI32_T                  ext_root_path_cost;
    XSTP_OM_PortVar_T       *pom_ptr;
    UI16_T                  index;
    XSTP_OM_PortVar_T       *index_pom_ptr;
    UI32_T                  root_lport;
    XSTP_OM_PortVar_T       *root_pom_ptr;

    pom_ptr = &(om_ptr->port_info[lport-1]);
    /* static external path cost */
    pom_ptr->common->static_external_path_cost  = TRUE;
    pom_ptr->common->external_port_path_cost    = path_cost;

    /* Root port. For consistency */
    root_lport = (UI32_T)(om_ptr->bridge_info.root_port_id.port_id & 0x0FFF);
    if ( root_lport == lport )
    {
        ext_root_path_cost = pom_ptr->port_priority.ext_root_path_cost + pom_ptr->common->external_port_path_cost;
        if (!pom_ptr->common->rcvd_internal)
        {
            om_ptr->bridge_info.root_priority.ext_root_path_cost = ext_root_path_cost;
            for (index=0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++ )
            {
                index_pom_ptr = &(om_ptr->port_info[index]);
                index_pom_ptr->designated_priority.ext_root_path_cost = ext_root_path_cost;
                if((index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                   || (index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DISABLED)
                   || ((index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
                       && (ext_root_path_cost  < index_pom_ptr->port_priority.ext_root_path_cost))
                  )
                index_pom_ptr->port_priority.ext_root_path_cost = ext_root_path_cost;
            }
            root_pom_ptr = &(om_ptr->port_info[root_lport-1]);
            root_pom_ptr->reselect = TRUE;
        }
    }

    /* Alternate port */
    root_pom_ptr = &(om_ptr->port_info[root_lport-1]);
    if (    (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
         && ((pom_ptr->port_priority.ext_root_path_cost + pom_ptr->common->external_port_path_cost) <= (root_pom_ptr->port_priority.ext_root_path_cost + root_pom_ptr->common->external_port_path_cost))
       )
    {
        pom_ptr = &(om_ptr->port_info[root_lport-1]);
        pom_ptr->reselect = TRUE;
        /* Initialize the port_priority to motivate the PIM state machine for reselect root_port. */
        XSTP_MGR_InitPortPriorityVector(om_ptr, lport);
    }

    return;

}/* End of XSTP_MGR_SetPortPathCost_() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMstPortAutoPathCost_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Restore the default internal path_cost of the port for a instance.
 * INPUT    :   om_ptr    -- om pointer for this instance
 *              UI32_T lport            -- lport number
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   internal_port_path_cost
 * ------------------------------------------------------------------------
 */
static UI32_T XSTP_MGR_SetMstPortAutoPathCost_(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    UI32_T                  root_path_cost;
    UI16_T                  index;
    XSTP_OM_PortVar_T       *index_pom_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    UI32_T                  default_path_cost;
    UI32_T                  root_lport;
    XSTP_OM_PortVar_T       *root_pom_ptr;
    UI32_T                  result;

    result = XSTP_TYPE_RETURN_OK;
    pom_ptr = &(om_ptr->port_info[lport-1]);
    XSTP_OM_GetLportDefaultPathCost(lport, &default_path_cost);

    if (    (om_ptr->instance_exist)
        ||  (om_ptr->instance_id == XSTP_TYPE_CISTID) /* System Spanning Tree Status is disabled */
       )
    {
        if (XSTP_OM_IsMemberPortOfInstance (om_ptr, lport))
        {
            /* Reset the static_path_cost flag.*/
            /* auto-determinated path cost     */
            pom_ptr->static_internal_path_cost = FALSE;

            pom_ptr->internal_port_path_cost = default_path_cost;
            root_lport = (UI32_T)(om_ptr->bridge_info.root_port_id.port_id & 0x0FFF);

            /* Root port. For consistency */
            if ( root_lport == lport )
            {
                root_path_cost = pom_ptr->port_priority.int_root_path_cost + pom_ptr->internal_port_path_cost;
                om_ptr->bridge_info.root_priority.int_root_path_cost = root_path_cost;
                for (index=0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++ )
                {
                    index_pom_ptr = &(om_ptr->port_info[index]);
                    index_pom_ptr->designated_priority.int_root_path_cost = root_path_cost;
                    if((index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                       || (index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DISABLED)
                       || ((index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
                           && (root_path_cost  < index_pom_ptr->port_priority.int_root_path_cost))
                      )
                    index_pom_ptr->port_priority.int_root_path_cost = root_path_cost;
                }
                root_pom_ptr = &(om_ptr->port_info[root_lport-1]);
                root_pom_ptr->reselect = TRUE;
            }

            /* Alternate port */
            root_pom_ptr = &(om_ptr->port_info[root_lport-1]);
            if (    (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
                 && ((pom_ptr->port_priority.int_root_path_cost +pom_ptr->internal_port_path_cost) <= (root_pom_ptr->port_priority.int_root_path_cost + root_pom_ptr->internal_port_path_cost))
               )
            {
               pom_ptr = &(om_ptr->port_info[root_lport-1]);
               pom_ptr->reselect = TRUE;
                /* Initialize the port_priority to motivate the PIM state machine for reselect root_port. */
                XSTP_MGR_InitPortPriorityVector(om_ptr, lport);
            }
        }
        else
        {
            if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
                BACKDOOR_MGR_Printf("\r\nXSTP_MGR_SetPortAutoPathCost::ERROR!! port is not member.");
            EH_MGR_Handle_Exception2(SYS_MODULE_XSTP, XSTP_MGR_SetMstPortAutoPathCost_Fun_No, EH_TYPE_MSG_NOT_MEMBER, SYSLOG_LEVEL_INFO, "Port", "specified instance ID");
            result = XSTP_TYPE_RETURN_ERROR;
        }
    }
    else
    {
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
            BACKDOOR_MGR_Printf("\r\nXSTP_MGR_SetPortAutoPathCost::ERROR!! instance is not existing.");
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetMstPortAutoPathCost_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Instance ID");
        result = XSTP_TYPE_RETURN_ERROR;
    }

    return result;
}/* End of XSTP_MGR_SetMstPortAutoPathCost_() */

#endif /* XSTP_TYPE_PROTOCOL_MSTP */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPortAdminPathCostAgent
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the admin path_cost of the port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T path_cost        -- the path_cost value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   Writing a value of '0' assigns the automatically calculated
 *              default Path Cost value to the port.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetPortAdminPathCostAgent(UI32_T lport, UI32_T path_cost)
{
    UI32_T  result;

    if (path_cost == 0)
    {
        result = XSTP_MGR_SetPortAutoPathCost(lport);
    }
    else
    {
        result = XSTP_MGR_SetPortPathCost(lport, path_cost);
    }
    return result;

}/* End of XSTP_MGR_SetPortAdminPathCostAgent() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPortAutoPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Restore the default path_cost of the port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   external_port_path_cost
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetPortAutoPathCost(UI32_T lport)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    UI32_T                  result;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }
    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetPortAutoPathCost_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR), "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
        return XSTP_TYPE_RETURN_PORTNO_OOR;
    }

    result = XSTP_TYPE_RETURN_OK;
    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    result = XSTP_MGR_SetPortAutoPathCost_(om_ptr, lport);
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
    XSTP_MGR_StateMachineProgress(XSTP_TYPE_CISTID);
    return result;
}/* End of XSTP_MGR_SetPortAutoPathCost */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMstPortAdminPathCostAgent
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the admin path_cost of the port for specified spanning tree
 *              when mode is MSTP.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 *              UI32_T path_cost        -- the path_cost value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURNPORTNO_OOR  -- port number out of range
 *              XSTP_TYPE_RETURNINDEX_OOR   -- mstid and priority out of range
 * NOTE     :   Writing a value of '0' assigns the automatically calculated
                default Path Cost value to the port.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetMstPortAdminPathCostAgent(UI32_T lport, UI32_T mstid, UI32_T path_cost)
{
    UI32_T  result;
    UI32_T  current_st_mode;

    current_st_mode = (UI32_T)XSTP_OM_GetForceVersion();
    if (    (current_st_mode != XSTP_TYPE_MSTP_MODE)
        &&  (mstid != XSTP_TYPE_CISTID)
       )
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    result = XSTP_TYPE_RETURN_ERROR;

    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    if (path_cost == 0)
    {
        result = XSTP_MGR_SetMstPortAutoPathCost(lport, mstid);
    }
    else
    {
        result = XSTP_MGR_SetMstPortPathCost(lport, mstid, path_cost);
    }
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    return result;
}/* End of XSTP_MGR_SetMstPortAdminPathCostAgent() */

#if 0
#if ((SYS_CPNT_SWDRV_CACHE == TRUE) && (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP))
/* for cache*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetVlanMsgqId
 * ------------------------------------------------------------------------
 * PURPOSE  : Call this function to let XSTP_MGR know vlan_msgq_id
 * INPUT    : msgq_id
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_SetVlanMsgqId(UI32_T msgq_id)
{
    XSTP_MGR_VlanMsgqId        = msgq_id;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetVlanMsgqId
 * ------------------------------------------------------------------------
 * PURPOSE  : Call this function to get vlan_msgq_id
 * INPUT    : msgq_id
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_GetVlanMsgqId(UI32_T *msgq_id)
{
    *msgq_id        = XSTP_MGR_VlanMsgqId;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_ProcessVlanCallbackEvent
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the vlan callback from SWDRV_CACHE
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_ProcessVlanCallbackEvent()
{
    XSTP_TYPE_VLAN_MSG_T    msg;
    UI32_T                  start_time, end_time;
    UI32_T                  create_count, destroy_count, add_count, delete_count;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }
    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
    {
        BACKDOOR_MGR_Printf("\r\nStart: XSTP_MGR_ProcessVlanCallbackEvent");
        start_time = SYS_TIME_GetSystemTicksBy10ms();
    }

    create_count = destroy_count = add_count = delete_count = 0;
    while (SYSFUN_ReceiveMsgQ((UI32_T)XSTP_MGR_VlanMsgqId, (UI32_T*)&msg, SYSFUN_TIMEOUT_NOWAIT) != SYSFUN_RESULT_NO_MESSAGE)
    {
        switch (msg.msg_type)
        {
            case XSTP_TYPE_VLAN_MSG_CREATED:
                XSTP_MGR_VlanCreatedForCache(msg.vid_ifidx);
                create_count++;
                break;
            case XSTP_TYPE_VLAN_MSG_DESTROY:
                XSTP_MGR_VlanDestroyForCache(msg.vid_ifidx);
                destroy_count++;
                break;
            case XSTP_TYPE_VLAN_MSG_MEMBER_ADD:
                XSTP_MGR_VlanMemberAddForCache(msg.vid_ifidx, msg.lport_ifidx);
                add_count++;
                break;
            case XSTP_TYPE_MSG_MISC_MEMBER_DELETE:
                XSTP_MGR_VlanMemberDeleteForCache(msg.vid_ifidx, msg.lport_ifidx);
                delete_count++;
                break;
            default:
                break;
        }
    }

    if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
    {
        end_time = SYS_TIME_GetSystemTicksBy10ms();
        BACKDOOR_MGR_Printf("\r\nFinish: XSTP_MGR_ProcessVlanCallbackEven : Create_count = %ld, Destroy_count = %ld, Add_count = %ld,  Delete_count = %ld, Time consumption = %lu",
               create_count, destroy_count, add_count, delete_count, (end_time - start_time));
    }

    return;
}/* End of XSTP_MGR_ProcessVlanCallbackEvent */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_VlanCreatedForCache
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from VLAN_MGR when a vlan is created
 * INPUT    : vid_ifidx     -- specify which vlan has just been created
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void    XSTP_MGR_VlanCreatedForCache(UI32_T vid_ifidx)
{
    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    {   UI32_T  vid;

        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
            BACKDOOR_MGR_Printf("\r\nXSTP_MGR_VlanCreatedForCache : vid_ifidx = %ld", vid_ifidx);

        VLAN_OM_ConvertFromIfindex(vid_ifidx, &vid);
        XSTP_MGR_SetVlanToMstMappingTable(vid);
    }
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */
    return;
} /* End of XSTP_MGR_VlanCreatedForCache */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_VlanDestroyForCache
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from VLAN_MGR when a vlan is destroyed
 * INPUT    : vid_ifidx     -- specify which vlan has just been destroyed
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void    XSTP_MGR_VlanDestroyForCache(UI32_T vid_ifidx)
{
    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    {
        UI32_T  vid;

        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
            BACKDOOR_MGR_Printf("\r\nXSTP_MGR_VlanDestroyForCache : vid_ifidx = %ld", vid_ifidx);

        VLAN_OM_ConvertFromIfindex(vid_ifidx, &vid);
        XSTP_MGR_DestroyVlanFromMstMappingTable(vid);
    }
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */
    return;
} /* End of XSTP_MGR_VlanDestroyForCache */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_VlanMemberAddForCache
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from VLAN_MGR when a lport is added to a
 *            vlan's member set.
 * INPUT    : vid_ifidx     -- specify which vlan's member set to be add to
 *            lport_ifindex -- specify which port to be added to the member set
 *            vlan_status   --
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void    XSTP_MGR_VlanMemberAddForCache(UI32_T vid_ifidx, UI32_T lport_ifidx)
{
    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    {
        XSTP_OM_InstanceData_T  *om_ptr;
        XSTP_OM_PortVar_T       *pom_ptr;
        UI32_T                  mstid;
            UI32_T                  vid;
        UI32_T                  current_st_mode;

        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
            BACKDOOR_MGR_Printf("\r\nXSTP_MGR_VlanMemberAddForCache : vid_ifidx = %ld, lport_ifidx = %ld", vid_ifidx, lport_ifidx);

        VLAN_OM_ConvertFromIfindex(vid_ifidx, &vid);

        XSTP_OM_GetMstidFromMstConfigurationTableByVlan(vid, &mstid);

        /* +++ EnterCriticalRegion +++ */
        XSTP_OM_EnterCriticalSection(mstid);
        om_ptr              = XSTP_OM_GetInstanceInfoPtr(mstid);
        pom_ptr             = &(om_ptr->port_info[lport_ifidx-1]);
        pom_ptr->is_member  = TRUE;
        current_st_mode     = (UI32_T)XSTP_OM_GetForceVersion();
        if (current_st_mode == XSTP_TYPE_MSTP_MODE)
        {
            if (pom_ptr->common->port_spanning_tree_status == VAL_staPortSystemStatus_enabled)
            {
                XSTP_ENGINE_SetPortState(om_ptr, lport_ifidx, pom_ptr->learning, pom_ptr->forwarding);
            }
            else
            {
                /* Set port state if its spanning_tree_status is disabled */
                if ((pom_ptr->common->link_up) && (pom_ptr->common->port_enabled))
                {
                    XSTP_ENGINE_SetPortState(om_ptr, lport_ifidx, TRUE, TRUE);
                }
                else
                {
                    XSTP_ENGINE_SetPortState(om_ptr, lport_ifidx, FALSE, FALSE);
                }
            }
        }
        /* +++ LeaveCriticalRegion +++ */
        XSTP_OM_LeaveCriticalSection(mstid);

        XSTP_MGR_StateMachineProgress(mstid);
    }
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    return;
} /* End of XSTP_MGR_VlanMemberAddForCache */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_VlanMemberDeleteForCache
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from VLAN_MGR when a port is remove from
 *            vlan's member set.
 * INPUT    : vid_ifidx     -- specify which vlan's member set to be deleted from
 *            lport_ifindex -- specify which port to be deleted
 *            vlan_status   --
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void    XSTP_MGR_VlanMemberDeleteForCache(UI32_T vid_ifidx, UI32_T lport_ifidx)
{
    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    {
        XSTP_OM_InstanceData_T  *om_ptr;
        XSTP_OM_PortVar_T       *pom_ptr;
        UI32_T                  mstid;
            UI32_T                  vid;
        UI32_T                  vlan_id, vid_ifindex;
        BOOL_T                  member_found;

        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
            BACKDOOR_MGR_Printf("\r\nXSTP_MGR_VlanMemberDeleteForCache : vid_ifidx = %ld, lport_ifidx = %ld", vid_ifidx, lport_ifidx);

        VLAN_OM_ConvertFromIfindex(vid_ifidx, &vid);

        XSTP_OM_GetMstidFromMstConfigurationTableByVlan(vid, &mstid);
        if (mstid != XSTP_TYPE_CISTID)
        {
            /* +++ EnterCriticalRegion +++ */
            XSTP_OM_EnterCriticalSection(mstid);
            om_ptr = XSTP_OM_GetInstanceInfoPtr(mstid);

            vlan_id = 0;
            member_found    = FALSE;
            while ( (!member_found) && XSTP_OM_GetNextXstpMember(om_ptr, &vlan_id))
            {
                if (vid != vlan_id)
                {
                    VLAN_OM_ConvertToIfindex(vlan_id, &vid_ifindex);
                    if (VLAN_OM_IsPortVlanMember(vid_ifindex, lport_ifidx))
                    {
                        member_found    = TRUE;
                    }
                }
            }

            if (!member_found)
            {
                pom_ptr = &(om_ptr->port_info[lport_ifidx-1]);
                pom_ptr->is_member  = FALSE;
                XSTP_ENGINE_InitPortStateMachines(om_ptr, lport_ifidx);
                XSTP_ENGINE_SetPortState(om_ptr, lport_ifidx, FALSE, FALSE);
            }

            /* +++ LeaveCriticalRegion +++ */
            XSTP_OM_LeaveCriticalSection(mstid);
        }
        XSTP_MGR_StateMachineProgress(mstid);
    }
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    return;
} /* End of XSTP_MGR_VlanMemberDeleteForCache */

#endif /* ((SYS_CPNT_SWDRV_CACHE == TRUE) && (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP))*/
#endif /* #if 0 */

/* per_port spanning tree */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPortSpanningTreeStatusEx
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the spanning tree status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T status           -- the status value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR       -- not master mode
 *              XSTP_TYPE_RETURN_ERROR  -- failed
 *              XSTP_TYPE_RETURN_ST_STATUS_DISABLED
 * NOTE     :   No xor checking.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetPortSpanningTreeStatusEx(UI32_T lport, UI32_T status)
{
    XSTP_OM_InstanceData_T          *om_ptr;
    UI32_T                          current_st_status;
    XSTP_OM_PortVar_T               *pom_ptr;
    UI32_T                          mstid;
    UI32_T                          index;
    BOOL_T                          progress;
    UI32_T                          old_role;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    current_st_status = XSTP_OM_GetSpanningTreeStatus();

    if (    (status!= VAL_staPortSystemStatus_enabled)
         && (status!= VAL_staPortSystemStatus_disabled)
       )
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);

    if(pom_ptr->common->port_spanning_tree_status == status)
    {
        /* +++ LeaveCriticalRegion +++ */
        XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
        return XSTP_TYPE_RETURN_OK;
    }

    if (status == VAL_staPortSystemStatus_disabled)
    {
        pom_ptr->common->port_spanning_tree_status = status;
        mstid               = XSTP_TYPE_CISTID;
        om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
        do
        {
            pom_ptr             = &(om_ptr->port_info[lport-1]);
            old_role            = pom_ptr->role;
            pom_ptr->role       /*= pom_ptr->selected_role*/ = XSTP_ENGINE_PORTVAR_ROLE_DISABLED;
            XSTP_ENGINE_InitPortStateMachines(om_ptr, lport);
            if ((pom_ptr->common->link_up) && (pom_ptr->common->port_enabled))
            {
                XSTP_ENGINE_SetPortState(om_ptr, lport, TRUE, TRUE);
            }
            else
            {
                XSTP_ENGINE_SetPortState(om_ptr, lport, FALSE, FALSE);
            }
            progress = FALSE;
            for (index=0; index<XSTP_TYPE_MAX_NUM_OF_LPORT; index++)
            {
                if (    (om_ptr->instance_exist)
                    &&  (om_ptr->port_info[index].is_member)
                    &&  (om_ptr->port_info[index].role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
                   )
                {
                    /* Initialize the port_priority to motivate the PIM state machine for reselect root_port. */
                    XSTP_MGR_InitPortPriorityVector(om_ptr, index+1);
                    progress = TRUE;
                }
            }
            if (    (!progress)
                &&  (old_role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
               )
            {
                XSTP_MGR_SetRootBridge(om_ptr);
                progress = TRUE;
            }
            if (progress)
            {
                XSTP_ENGINE_StateMachineProgress(om_ptr);
            }

        }while(XSTP_OM_GetNextInstanceInfoPtr(&mstid, &om_ptr));
    }
    else
    {
        pom_ptr->common->port_spanning_tree_status = status;
        mstid               = XSTP_TYPE_CISTID;
        om_ptr              = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
        do
        {
            pom_ptr = &(om_ptr->port_info[lport-1]);
            XSTP_ENGINE_InitPortStateMachines(om_ptr, lport);
            if (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
            {
                if (    (om_ptr->instance_exist)
                    &&  (pom_ptr->is_member)
                    )
                {
                    XSTP_ENGINE_StateMachineProgress(om_ptr);
                }
            }
            else
            {
                if ((pom_ptr->common->link_up) && (pom_ptr->common->port_enabled))
                {
                    XSTP_ENGINE_SetPortState(om_ptr, lport, TRUE, TRUE);
                }
                else
                {
                    XSTP_ENGINE_SetPortState(om_ptr, lport, FALSE, FALSE);
                }
            }
        }while(XSTP_OM_GetNextInstanceInfoPtr(&mstid, &om_ptr));
    }

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
    return XSTP_TYPE_RETURN_OK;
}/* End of XSTP_MGR_SetPortSpanningTreeStatusEx */

/* per_port spanning tree */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPortSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the spanning tree status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T status           -- the status value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR       -- not master mode
 *              XSTP_TYPE_RETURN_ERROR  -- failed
 *              XSTP_TYPE_RETURN_ST_STATUS_DISABLED
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetPortSpanningTreeStatus(UI32_T lport, UI32_T status)
{
    UI32_T  ret = XSTP_TYPE_RETURN_ERROR;

    /* patched for checking fail when handling hot insertion with erps ring port
     */
    if (VAL_staPortSystemStatus_disabled != status)
    {
        XSTP_MGR_XOR_LOCK_AND_FAIL_RETURN(
            ret,
            (TRUE == SYSCTRL_XOR_MGR_PermitBeingSetToXSTPPort(lport)));
    }

    ret = XSTP_MGR_SetPortSpanningTreeStatusEx(lport, status);

    /* patched for checking fail when handling hot insertion with erps ring port
     */
    if (VAL_staPortSystemStatus_disabled != status)
    {
        XSTP_MGR_XOR_UNLOCK_AND_RETURN(ret);
    }
    else
    {
        return ret;
    }
}/* End of XSTP_MGR_SetPortSpanningTreeStatus */

/* ===================================================================== */
/* ===================================================================== */
/* For hot-swappable optional module */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_HandleHotInsertion
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
void XSTP_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    UI32_T  current_st_status;
    UI32_T  current_st_mode;
    UI32_T  vid;

    current_st_status = XSTP_OM_GetSpanningTreeStatus();
    current_st_mode   = XSTP_OM_GetForceVersion();

    if (    (current_st_mode == XSTP_TYPE_MSTP_MODE)
        &&  (current_st_status == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
       )
    {
        /* MST mode */
        #ifdef  XSTP_TYPE_PROTOCOL_MSTP
        if (!SWCTRL_PMGR_SetMstEnableStatus(SWCTRL_MST_ENABLE) )
        {
            printf("\r\nXSTP_MGR_HandleHotInsertion: Fail in setting the MST mode!!");
        }
        #endif /* XSTP_TYPE_PROTOCOL_MSTP */
        XSTP_ENGINE_SetMstEnableStatus(TRUE);
    }
    else
    {
        /* MST mode */
        #ifdef  XSTP_TYPE_PROTOCOL_MSTP
        if (!SWCTRL_PMGR_SetMstEnableStatus(SWCTRL_MST_DISABLE) )
        {
            printf("\r\nXSTP_MGR_HandleHotInsertion: Fail in setting the MST mode!!");
        }
        #endif /* XSTP_TYPE_PROTOCOL_MSTP */
        XSTP_ENGINE_SetMstEnableStatus(FALSE);
    }

    XSTP_MGR_ResetPortDatabase(starting_port_ifindex, number_of_port, TRUE);

    vid = 0;
    while (VLAN_OM_GetNextVlanId(0, &vid) )
    {
        XSTP_MGR_VlanCreated_CallBack(vid+SYS_ADPT_VLAN_1_IF_INDEX_NUMBER-1, 0);
    }

    return;
}/* End of XSTP_MGR_HandleHotInsertion */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_HandleHotRemoval
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
void XSTP_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    XSTP_MGR_ResetPortDatabase(starting_port_ifindex, number_of_port, FALSE);
    return;
}/* End of XSTP_MGR_HandleHotRemoval */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_ResetPortDatabase
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize/Clear port OM.
 * INPUT    : starting_port_ifindex -- the ifindex of the first port
 *            number_of_port        -- the number of ports
 *            set_default           -- True: the option module is inserted.
 *                                  -- FALSE: the option module is removed.
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static void XSTP_MGR_ResetPortDatabase(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T set_default)
{
    UI32_T                      unit, port, trunk_id;
    SWCTRL_Lport_Type_T         result;
    XSTP_OM_InstanceData_T      *om_ptr;
    UI32_T                      entry_index;
    UI32_T                      next_lport, remaining_number;

    next_lport          = starting_port_ifindex;
    remaining_number    = number_of_port;

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);

    while(remaining_number)
    {
        result = SWCTRL_LogicalPortToUserPort(next_lport, &unit, &port, &trunk_id);
        switch(result)
        {
            case SWCTRL_LPORT_NORMAL_PORT:
            case SWCTRL_LPORT_TRUNK_PORT:
                for (entry_index = 0; entry_index <= XSTP_TYPE_MAX_INSTANCE_NUM; entry_index++)
                {
                    om_ptr  = XSTP_OM_GetInstanceEntryPtr(entry_index);
                    XSTP_MGR_InitHotModulePort(om_ptr, next_lport, set_default);
                }
                break;
            case SWCTRL_LPORT_TRUNK_PORT_MEMBER:
            case SWCTRL_LPORT_UNKNOWN_PORT:
            default:
                break;
        }
        next_lport++;
        remaining_number--;
    }
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    return;
}/* XSTP_MGR_ResetPortDatabase */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_InitHotModulePort
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport
 *            set_default           -- True: the option module is inserted.
 *                                  -- FALSE: the option module is removed.
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
static void XSTP_MGR_InitHotModulePort(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, BOOL_T set_default)
{
    XSTP_OM_PortVar_T               *pom_ptr;

    pom_ptr = &(om_ptr->port_info[lport-1]);

    /* 1. Init static flags */
    pom_ptr->static_port_priority   = FALSE;
    #ifdef  XSTP_TYPE_PROTOCOL_RSTP
    pom_ptr->static_path_cost       = FALSE;
    #endif /* XSTP_TYPE_PROTOCOL_RSTP */
    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    if (om_ptr->instance_id == XSTP_TYPE_CISTID)
    {
        pom_ptr->common->static_external_path_cost = FALSE;
    }
    pom_ptr->static_internal_path_cost = FALSE;
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    /* 2. Init common variables */
    if (om_ptr->instance_id == XSTP_TYPE_CISTID)
    {
        pom_ptr->common->auto_edge                      = (XSTP_TYPE_DEFAULT_PORT_ADMIN_EDGE_PORT == VAL_staPortAdminEdgePortWithAuto_auto) ? TRUE : FALSE;
        pom_ptr->common->admin_edge                     = (XSTP_TYPE_DEFAULT_PORT_ADMIN_EDGE_PORT == VAL_staPortAdminEdgePortWithAuto_true) ? TRUE : FALSE;
        pom_ptr->common->oper_edge                      = pom_ptr->common->admin_edge;
        pom_ptr->common->admin_point_to_point_mac       = FALSE;
        pom_ptr->common->admin_point_to_point_mac_auto  = TRUE;
        pom_ptr->common->oper_point_to_point_mac        = TRUE;
        XSTP_OM_NullifyPortOmEngineState(om_ptr, lport);
    }

    /* 3. Init port OM */
    XSTP_OM_NullifyPortOm(om_ptr, lport);
    /* 4. Init port state machine variables */
    XSTP_ENGINE_InitPortStateMachines(om_ptr, lport);

    /*5. Set is_member and port_enabled flags if the option module is inserted */
    if ( set_default && (om_ptr->instance_id == XSTP_TYPE_CISTID))
    {
        pom_ptr->is_member              = TRUE;
        pom_ptr->common->port_enabled   = TRUE;
    }
    return;
}/* XSTP_MGR_InitHotModulePort */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPortAutoPathCost_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Restore the default path_cost of the port.
 * INPUT    :   om_ptr    -- om pointer for this instance
 *              UI32_T lport            -- lport number
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   external_port_path_cost
 * ------------------------------------------------------------------------
 */
static UI32_T XSTP_MGR_SetPortAutoPathCost_(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    UI32_T                  root_path_cost;
    UI16_T                  index;
    XSTP_OM_PortVar_T       *index_pom_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;
    UI32_T                  default_path_cost;
    UI32_T                  root_lport;
    XSTP_OM_PortVar_T       *root_pom_ptr;
    UI32_T                  result;

    result = XSTP_TYPE_RETURN_OK;
    pom_ptr = &(om_ptr->port_info[lport-1]);
    XSTP_OM_GetLportDefaultPathCost(lport, &default_path_cost);

    if (    (om_ptr->instance_exist)
        ||  (om_ptr->instance_id == XSTP_TYPE_CISTID) /* System Spanning Tree Status is disabled */
       )
    {
        if (XSTP_OM_IsMemberPortOfInstance (om_ptr, lport))
        {
            /* Reset the static_path_cost flag.*/
            /* auto-determinated path cost     */
            #ifdef  XSTP_TYPE_PROTOCOL_RSTP
            pom_ptr->static_path_cost = FALSE;

            pom_ptr->port_path_cost = default_path_cost;
            /* Root port. For consistency */
            root_lport = (UI32_T)(om_ptr->bridge_info.root_port_id.port_id & 0x0FFF);
            if ( root_lport == lport )
            {
                root_path_cost = pom_ptr->port_priority.root_path_cost + pom_ptr->port_path_cost;
                om_ptr->bridge_info.root_priority.root_path_cost = root_path_cost;
                for (index=0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++ )
                {
                    index_pom_ptr = &(om_ptr->port_info[index]);
                    index_pom_ptr->designated_priority.root_path_cost = root_path_cost;
                    if((index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                       || (index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DISABLED)
                       || ((index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
                           && (root_path_cost  < index_pom_ptr->port_priority.root_path_cost))
                      )
                    index_pom_ptr->port_priority.root_path_cost = root_path_cost;
                }
                root_pom_ptr = &(om_ptr->port_info[root_lport-1]);
                root_pom_ptr->reselect = TRUE;
            }

            /* Alternate port */
            root_pom_ptr = &(om_ptr->port_info[root_lport-1]);
            if (    (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
                 && ((pom_ptr->port_priority.root_path_cost + pom_ptr->port_path_cost) <= (root_pom_ptr->port_priority.root_path_cost + root_pom_ptr->port_path_cost))
               )
            {
                pom_ptr = &(om_ptr->port_info[root_lport-1]);
                pom_ptr->reselect = TRUE;
                /* Initialize the port_priority to motivate the PIM state machine for reselect root_port. */
                XSTP_MGR_InitPortPriorityVector(om_ptr, lport);
            }
            #endif /* XSTP_TYPE_PROTOCOL_RSTP */

            #ifdef  XSTP_TYPE_PROTOCOL_MSTP
            pom_ptr->common->static_external_path_cost = FALSE;


            pom_ptr->common->external_port_path_cost = default_path_cost;

            /* Root port. For consistency */
            root_lport = (UI32_T)(om_ptr->bridge_info.root_port_id.port_id & 0x0FFF);
            if ( root_lport == lport )
            {
                root_path_cost = pom_ptr->port_priority.ext_root_path_cost + pom_ptr->common->external_port_path_cost;
                if (!pom_ptr->common->rcvd_internal)
                {
                    om_ptr->bridge_info.root_priority.ext_root_path_cost = root_path_cost;
                    for (index=0; index < XSTP_TYPE_MAX_NUM_OF_LPORT; index++ )
                    {
                        index_pom_ptr = &(om_ptr->port_info[index]);
                        index_pom_ptr->designated_priority.ext_root_path_cost = root_path_cost;
                        if((index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DESIGNATED)
                           || (index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_DISABLED)
                           || ((index_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
                               && (root_path_cost  < index_pom_ptr->port_priority.ext_root_path_cost))
                          )
                        index_pom_ptr->port_priority.ext_root_path_cost = root_path_cost;
                    }
                    root_pom_ptr = &(om_ptr->port_info[root_lport-1]);
                    root_pom_ptr->reselect = TRUE;
                }
            }

            if( (root_lport > 0)&&(root_lport <= XSTP_TYPE_MAX_NUM_OF_LPORT))
            {
                /* Alternate port */
                root_pom_ptr = &(om_ptr->port_info[root_lport-1]);
                if (    (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
                     && ((pom_ptr->port_priority.ext_root_path_cost +pom_ptr->common->external_port_path_cost) <= (root_pom_ptr->port_priority.ext_root_path_cost + root_pom_ptr->common->external_port_path_cost))
                   )
                {
                   pom_ptr = &(om_ptr->port_info[root_lport-1]);
                   pom_ptr->reselect = TRUE;
                    /* Initialize the port_priority to motivate the PIM state machine for reselect root_port. */
                    XSTP_MGR_InitPortPriorityVector(om_ptr, lport);
                }
            }
            #endif /* XSTP_TYPE_PROTOCOL_MSTP */
        }
        else
        {
            if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
                BACKDOOR_MGR_Printf("\r\nXSTP_MGR_SetPortAutoPathCost::ERROR!! port is not member.");
            EH_MGR_Handle_Exception2(SYS_MODULE_XSTP, XSTP_MGR_SetPortAutoPathCost_Fun_No, EH_TYPE_MSG_NOT_MEMBER, SYSLOG_LEVEL_INFO, "Port", "specified instance ID");
            result = XSTP_TYPE_RETURN_ERROR;
        }
    }
    else
    {
        if (XSTP_OM_Debug(XSTP_TYPE_DEBUG_FLAG_DBGMSG))
            BACKDOOR_MGR_Printf("\r\nXSTP_MGR_SetPortAutoPathCost::ERROR!! instance is not existing.");
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_SetPortAutoPathCost_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "Instance ID");
        result = XSTP_TYPE_RETURN_ERROR;
    }
    return result;
}/* End of XSTP_MGR_SetPortAutoPathCost_() */

#if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_RecordAttribute
 * ------------------------------------------------------------------------
 * PURPOSE  :   Record attributes of the port for the instance.
 * INPUT    :   om_ptr                  -- om pointer for this instance
 *              UI32_T lport            -- lport number
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   1. path_cost attribute
 * ------------------------------------------------------------------------
 */
static void XSTP_MGR_RecordAttribute(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;

    memset(&backup_values, 0, sizeof(XSTP_MGR_BackupAttribute_T));
    pom_ptr = &(om_ptr->port_info[lport-1]);
    #ifdef  XSTP_TYPE_PROTOCOL_RSTP
    backup_values.static_path_cost    = pom_ptr->static_path_cost;
    backup_values.port_path_cost      = pom_ptr->port_path_cost;
    #endif /* XSTP_TYPE_PROTOCOL_RSTP */

    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    backup_values.static_external_path_cost   = pom_ptr->common->static_external_path_cost;
    backup_values.static_internal_path_cost   = pom_ptr->static_internal_path_cost;
    backup_values.external_port_path_cost     = pom_ptr->common->external_port_path_cost;
    backup_values.internal_port_path_cost     = pom_ptr->internal_port_path_cost;
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */
}/* End of XSTP_MGR_RecordAttribute() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_RestoreAttribute
 * ------------------------------------------------------------------------
 * PURPOSE  :   Restore attributes of the port for the instance.
 * INPUT    :   om_ptr                  -- om pointer for this instance
 *              UI32_T lport            -- lport number
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   1. path_cost attribute
 * ------------------------------------------------------------------------
 */
static void XSTP_MGR_RestoreAttribute(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;

    pom_ptr = &(om_ptr->port_info[lport-1]);
    #ifdef  XSTP_TYPE_PROTOCOL_RSTP
    if (backup_values.static_path_cost)
    {
        XSTP_MGR_SetMstPortPathCost_(om_ptr, lport, backup_values.port_path_cost);
    }
    else
    {
        XSTP_MGR_SetPortAutoPathCost_(om_ptr, lport);
    }

    #endif /* XSTP_TYPE_PROTOCOL_RSTP */

    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    if (om_ptr->instance_id == XSTP_TYPE_CISTID)
    {
        if (backup_values.static_external_path_cost)
        {
            XSTP_MGR_SetPortPathCost_(om_ptr, lport, backup_values.external_port_path_cost);
        }
        else
        {
            XSTP_MGR_SetPortAutoPathCost_(om_ptr, lport);
        }
    }

    if (backup_values.static_internal_path_cost)
    {
        XSTP_MGR_SetMstPortPathCost_(om_ptr, lport, backup_values.internal_port_path_cost);
    }
    else
    {
        XSTP_MGR_SetMstPortAutoPathCost_(om_ptr, lport);
    }
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    memset(&backup_values, 0, sizeof(XSTP_MGR_BackupAttribute_T));
}/* End of XSTP_MGR_RestoreAttribute() */
#endif /* #if (SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR == FALSE) */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetFloodingBpduWhenStpDisabled
 * ------------------------------------------------------------------------
 * PURPOSE  : Set bpdu behavior when spanning tree is disabled.
 * INPUT    : flooding_bpdu -- True: Flood BPDUs when spanning tree is disabled.
 *                          -- FALSE: Don't flood BPDUs when spanning tree is disabled.
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_MGR_SetFloodingBpduWhenStpDisabled(BOOL_T flooding_bpdu)
{
    /*SYSFUN_USE_CSC(FALSE);*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

#if (SYS_ADPT_BPDU_FLOOD_ON_STP_DISABLED == TRUE)
    flooding_bpdu_flag = flooding_bpdu;
    return TRUE;
#else
    return FALSE;
#endif
}/* XSTP_MGR_SetFloodingBpduWhenStpDisabled() */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetBpduBehaviorWhenStpDisabled
 * ------------------------------------------------------------------------
 * PURPOSE  : Get bpdu behavior when spanning tree is disabled.
 * INPUT    : flooding_bpdu -- True: Flood BPDUs when spanning tree is disabled.
 *                          -- FALSE: Don't flood BPDUs when spanning tree is disabled.
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_MGR_GetBpduBehaviorWhenStpDisabled(BOOL_T *flooding_bpdu)
{
    /*SYSFUN_USE_CSC(FALSE);*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    *flooding_bpdu = flooding_bpdu_flag;

    return TRUE;
}/* XSTP_MGR_GetBpduBehaviorWhenStpDisabled() */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetRunningBpduBehaviorWhenStpDisabled
 * ------------------------------------------------------------------------
 * PURPOSE  : Get bpdu behavior when spanning tree is disabled.
 * INPUT    : flooding_bpdu -- True: Flood BPDUs when spanning tree is disabled.
 *                       -- FALSE: Don't flood BPDUs when spanning tree is disabled.
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_GetRunningBpduBehaviorWhenStpDisabled(BOOL_T *flooding_bpdu)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *flooding_bpdu = flooding_bpdu_flag;

    #if (SYS_ADPT_BPDU_FLOOD_ON_STP_DISABLED == TRUE)
    if (*flooding_bpdu == SYS_ADPT_BPDU_FLOOD_ON_STP_DISABLED)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    #else
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    #endif
}/* XSTP_MGR_GetRunningBpduBehaviorWhenStpDisabled() */


#if (SYS_CPNT_SUPPORT_XSTP_BACKUP_ROOT == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetBackupRootStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : Set backup root status.
 * INPUT    : is_enabled    -- True: Enable.
 *                          -- FALSE: Disable.
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_MGR_SetBackupRootStatus(BOOL_T is_enabled)
{
    /*SYSFUN_USE_CSC(FALSE);*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    backup_root_functionality = is_enabled;

    return TRUE;
}/* XSTP_MGR_SetBackupRootStatus() */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetBackupRootStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : Get backup root status.
 * INPUT    : None
 * OUTPUT   : BOOL_T *is_enabled     -- True: Enable.
 *                                   -- FALSE: Disable.
 * RETURN   : TRUE/FALSE
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_MGR_GetBackupRootStatus(BOOL_T *is_enabled)
{
    /*SYSFUN_USE_CSC(FALSE);*/
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    *is_enabled = backup_root_functionality;

    return TRUE;
}/* XSTP_MGR_GetBackupRootStatus() */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetRunningBackupRootStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : Get backup root status.
 * INPUT    : None
 * OUTPUT   : BOOL_T *is_enabled     -- True: Enable.
 *                                   -- FALSE: Disable.
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_GetRunningBackupRootStatus(BOOL_T *is_enabled)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *is_enabled = backup_root_functionality;

    if (*is_enabled == FALSE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

}/* XSTP_MGR_GetRunningBackupRootStatus() */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_HasRootBridgeDisappeared
 * ------------------------------------------------------------------------
 * PURPOSE  : Check if this bridge has lost contact with Root Bridge.
 * INPUT    : *om_ptr   -- the pointer of info entry
 *            lport     -- lport number
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
static BOOL_T  XSTP_MGR_HasRootBridgeDisappeared(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport)
{
    XSTP_OM_PortVar_T       *pom_ptr;
    UI32_T                  temp_lport;
    XSTP_OM_PortVar_T       *temp_pom_ptr;
    int                     cmp_a, cmp_b;

    if (om_ptr->instance_exist)
    {
        pom_ptr = &(om_ptr->port_info[lport-1]);

        /* The Backup Root Bridge shall have at least one direct-connected link to the Root Bridge,
        * otherwise the Root Bridge may have disappeared.
        * For the Backup Root Bridge we have to verify:
        * -- the link-down port is the root port connecting to the root bridge;
        * -- there is no other link-up alternated port connecting to the root bridge;
        * If both of the above two statements are true thus we do actions for the root disappearance.
        */

        /* Verify if the port is connecting to the root bridge. */
        #ifdef  XSTP_TYPE_PROTOCOL_RSTP
        XSTP_OM_CMP_BRIDGE_ID(cmp_a, om_ptr->bridge_info.root_priority.root_bridge_id, pom_ptr->port_priority.designated_bridge_id);
        #endif /* XSTP_TYPE_PROTOCOL_RSTP */
        #ifdef  XSTP_TYPE_PROTOCOL_MSTP
        if (om_ptr->instance_id == XSTP_TYPE_CISTID)
        {
            XSTP_OM_CMP_BRIDGE_ID(cmp_a, om_ptr->bridge_info.root_priority.root_id, pom_ptr->port_priority.designated_bridge_id);
        }
        else
        {
            XSTP_OM_CMP_BRIDGE_ID(cmp_a, om_ptr->bridge_info.root_priority.r_root_id, pom_ptr->port_priority.designated_bridge_id);
        }
        #endif /* XSTP_TYPE_PROTOCOL_MSTP */

        if (    (backup_root_functionality)
            &&  (cmp_a == 0)
            &&  (pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ROOT)
            )
        {
            /* The port is a root port connecting to the root bridge. */
            /* Need to verify if there is any other alternate port connecting to the root bridge. */
            temp_lport   = 0;
            while (SWCTRL_GetNextLogicalPort(&temp_lport) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                temp_pom_ptr = &(om_ptr->port_info[temp_lport-1]);
                #ifdef  XSTP_TYPE_PROTOCOL_RSTP
                XSTP_OM_CMP_BRIDGE_ID(cmp_b, om_ptr->bridge_info.root_priority.root_bridge_id, temp_pom_ptr->port_priority.designated_bridge_id);
                #endif /* XSTP_TYPE_PROTOCOL_RSTP */
                #ifdef  XSTP_TYPE_PROTOCOL_MSTP
                if (om_ptr->instance_id == XSTP_TYPE_CISTID)
                {
                    XSTP_OM_CMP_BRIDGE_ID(cmp_b, om_ptr->bridge_info.root_priority.root_id, temp_pom_ptr->port_priority.designated_bridge_id);
                }
                else
                {
                    XSTP_OM_CMP_BRIDGE_ID(cmp_b, om_ptr->bridge_info.root_priority.r_root_id, temp_pom_ptr->port_priority.designated_bridge_id);
                }
                #endif /* XSTP_TYPE_PROTOCOL_MSTP */

                if (    (temp_lport != lport)
                    &&  (cmp_b == 0)
                    &&  (temp_pom_ptr->role == XSTP_ENGINE_PORTVAR_ROLE_ALTERNATE)
                   )
                {
                    /* There exists one alternate port connecting to the root bridge. */
                    return FALSE;
                }
            }
            /* There is no alternated port connecting to the root bridge. */
            return TRUE;
        }
        /* This port is not a root port. */
        return FALSE;
    }
    else
    {
        /* This instance is not existed. */
        return FALSE;
    }
}/* End of XSTP_MGR_HasRootBridgeDisappeared() */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_BecomeNewRootBridge
 * ------------------------------------------------------------------------
 * PURPOSE  : Check if this bridge has lost contact with Root Bridge.
 * INPUT    : *om_ptr   -- the pointer of info entry
 *            lport     -- lport number
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
static BOOL_T  XSTP_MGR_BecomeNewRootBridge(XSTP_OM_InstanceData_T *om_ptr)
{
    UI32_T  current_root_priority;
    UI32_T  priority;

    #ifdef  XSTP_TYPE_PROTOCOL_RSTP
    current_root_priority = (UI32_T)((om_ptr->bridge_info.root_priority.root_bridge_id.priority)<<12);
    if (current_root_priority >= 4096)
    {
        priority = (UI32_T)(current_root_priority - 4096);
    }
    #endif /* XSTP_TYPE_PROTOCOL_RSTP */
    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    if (om_ptr->instance_id== XSTP_TYPE_CISTID)
    {
        current_root_priority = (UI32_T)((om_ptr->bridge_info.root_priority.root_id.priority)<<12);
        if (current_root_priority >= 4096)
        {
            priority = (UI32_T)(current_root_priority - 4096);
        }
    }
    else
    {
        current_root_priority = (UI32_T)((om_ptr->bridge_info.root_priority.r_root_id.priority)<<12);
        if (current_root_priority >= 4096)
        {
            priority = (UI32_T)(current_root_priority - 4096);
        }
    }
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */

    if (current_root_priority < 4096)
    {
        return TRUE;
    }
    else
    {
        return XSTP_MGR_SetMstPriority_(om_ptr, priority, FALSE);
    }
}/* End of XSTP_MGR_BecomeNewRootBridge() */
#endif /* SYS_CPNT_SUPPORT_XSTP_BACKUP_ROOT == TRUE */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetRootBridge
 * ------------------------------------------------------------------------
 * PURPOSE  :   To speed up convergence when priority changed, we enforce a bridge
 *              to be a root bridge and all its ports to be designated ports
 * INPUT    :   om_ptr           -- the pointer of the instance entry.
 * OUTPUT   :   None.
 * RETURN   :   None.
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
static void    XSTP_MGR_SetRootBridge(XSTP_OM_InstanceData_T *om_ptr)
{
    UI32_T                          lport;
    XSTP_OM_PortVar_T               *pom_ptr;

    /* root_priority_vector */
    memcpy(&om_ptr->bridge_info.root_priority, &om_ptr->bridge_info.bridge_priority, sizeof(XSTP_TYPE_PriorityVector_T));

    /* root_times */
    memcpy(&om_ptr->bridge_info.root_times, &om_ptr->bridge_info.bridge_times, sizeof(XSTP_TYPE_Timers_T));

    /* root_port_id */
    memset(&om_ptr->bridge_info.root_port_id, 0, sizeof(XSTP_TYPE_PortId_T));

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
                                            0,
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
            /* Because a MSTI might not receive any BPDU from the same region, enforcing the ports to
             * reselect port role can make boundry ports been selected to correct port role. (like Master port)
             */
            if(   (pom_ptr->is_member == TRUE)
#ifdef  XSTP_TYPE_PROTOCOL_MSTP
               && (pom_ptr->common->rcvd_internal == FALSE)
#endif
              )
            {
                pom_ptr->reselect = TRUE;
            }
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
}/* End of XSTP_MGR_SetRootBridge */


/*------------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_NotifyStpChangeVersion
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will call the CallBack function when the spanning tree
 *            version is changed.
 * INPUT    : mode      -- current spanning tree mode.
 *            status    -- current spanning tree status.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : It will be treated as the version changed when the Stp is
 *            enabled/disabled.
 *------------------------------------------------------------------------------
 */
static void XSTP_MGR_NotifyStpChangeVersion(UI32_T mode, UI32_T status)
{
    SYS_CALLBACK_MGR_StpChangeVersionCallback(SYS_MODULE_XSTP, mode, status);
    return;
} /* End of XSTP_MGR_NotifyStpChangeVersion()*/


/* ===================================================================== */
/* ===================================================================== */
/* ========                     Utilities                         =======*/
/* ===================================================================== */
/* ===================================================================== */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_ReleaseAllPortsMref
 * ------------------------------------------------------------------------
 * PURPOSE  : Release all ports' MREF after state machines have processed them
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : A garabage collection mechanism to prevent memory leakage.
 * ------------------------------------------------------------------------
 */
static void XSTP_MGR_ReleaseAllPortsMref()
{
    XSTP_OM_InstanceData_T *cist_p;
    XSTP_OM_PortVar_T      *aport_p;
    UI32_T  lport;

    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    cist_p = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    /* Force to scan every logical port no matter it currently exists or not
     * to ensure it frees all MREF.
     */
    for (lport = 1; lport <= XSTP_TYPE_MAX_NUM_OF_LPORT; lport++)
    {
        aport_p = &(cist_p->port_info[lport-1]);
        if (    (aport_p->common->rcvd_bpdu == FALSE)
             && (aport_p->common->mref_handle_p != NULL)
           )
        {
            L_MM_Mref_Release(&(aport_p->common->mref_handle_p));
            aport_p->common->mref_handle_p = NULL;
            aport_p->common->bpdu = NULL;
        }
    }
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_UpdateStateMachine
 * ------------------------------------------------------------------------
 * PURPOSE  : Update the port/bridge state machine for all instances.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_MGR_UpdateStateMachine(void)
{
    UI32_T                  xstid;
    XSTP_OM_InstanceData_T  *om_ptr;

    if (XSTP_OM_GetSpanningTreeStatus() == XSTP_TYPE_SYSTEM_ADMIN_STATE_DISABLED)
    {
        return;
    }
    xstid = XSTP_TYPE_CISTID;
    do
    {
        /* +++ EnterCriticalRegion +++ */
        XSTP_OM_EnterCriticalSection(xstid);
        om_ptr  = XSTP_OM_GetInstanceInfoPtr(xstid);
        if (om_ptr->delay_flag)
        {
            XSTP_ENGINE_StateMachineProgress(om_ptr);
            om_ptr->delay_flag = FALSE;
        }
        /* +++ LeaveCriticalRegion +++ */
        XSTP_OM_LeaveCriticalSection(xstid);
    } while(XSTP_OM_GetNextExistedInstance(&xstid));
    XSTP_MGR_ReleaseAllPortsMref();
    return;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: XSTP_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for xstp mgr.
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
BOOL_T XSTP_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    XSTP_MGR_IpcMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msg_p->type.ret_ui32 = XSTP_TYPE_RETURN_ERROR;
        msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    /* dispatch IPC message and call the corresponding XSTP_MGR function
     */
    switch (msg_p->type.cmd)
    {
        case XSTP_MGR_IPC_SETCHANGESTATEPORTLISTFORBIDDEN:
            XSTP_MGR_SetChangeStatePortListForbidden(msg_p->data.arg_bool);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETSYSTEMSPANNINGTREESTATUS:
            msg_p->type.ret_ui32 =
                XSTP_MGR_SetSystemSpanningTreeStatus(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETSYSTEMSPANNINGTREEVERSION:
            msg_p->type.ret_ui32 =
                XSTP_MGR_SetSystemSpanningTreeVersion(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETFORWARDDELAY:
            msg_p->type.ret_ui32 =
                XSTP_MGR_SetForwardDelay(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETHELLOTIME:
            msg_p->type.ret_ui32 =
                XSTP_MGR_SetHelloTime(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETMAXAGE:
            msg_p->type.ret_ui32 =
                XSTP_MGR_SetMaxAge(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETPATHCOSTMETHOD:
            msg_p->type.ret_ui32 =
                XSTP_MGR_SetPathCostMethod(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETTRANSMISSIONLIMIT:
            msg_p->type.ret_ui32 =
                XSTP_MGR_SetTransmissionLimit(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETSYSTEMGLOBALPRIORITY:
            msg_p->type.ret_ui32 =
                XSTP_MGR_SetSystemGlobalPriority(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETSYSTEMBRIDGEPRIORITY:
            msg_p->type.ret_ui32 =
                XSTP_MGR_SetSystemBridgePriority(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETPORTPATHCOST:
            msg_p->type.ret_ui32 = XSTP_MGR_SetPortPathCost(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETPORTPRIORITY:
            msg_p->type.ret_ui32 = XSTP_MGR_SetPortPriority(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETPORTLINKTYPEMODE:
            msg_p->type.ret_ui32 = XSTP_MGR_SetPortLinkTypeMode(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETPORTPROTOCOLMIGRATION:
            msg_p->type.ret_ui32 = XSTP_MGR_SetPortProtocolMigration(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETPORTADMINEDGEPORT:
            msg_p->type.ret_ui32 = XSTP_MGR_SetPortAdminEdgePort(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETMSTPRIORITY:
            msg_p->type.ret_ui32 = XSTP_MGR_SetMstPriority(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETMSTPORTPRIORITY:
            msg_p->type.ret_ui32 = XSTP_MGR_SetMstPortPriority(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2,
                msg_p->data.arg_grp2.arg3);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETMSTPORTPATHCOST:
            msg_p->type.ret_ui32 = XSTP_MGR_SetMstPortPathCost(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2,
                msg_p->data.arg_grp2.arg3);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_ATTACHVLANLISTTOMSTCONFIGTABLE:
            msg_p->type.ret_ui32 = XSTP_MGR_AttachVlanListToMstConfigTable(
                msg_p->data.arg_grp3.arg1, msg_p->data.arg_grp3.arg2,
                msg_p->data.arg_grp3.arg3);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETVLANTOMSTCONFIGTABLE:
            msg_p->type.ret_ui32 = XSTP_MGR_SetVlanToMstConfigTable(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_ATTACHVLANTOMSTCONFIGTABLE:
            msg_p->type.ret_ui32 = XSTP_MGR_AttachVlanToMstConfigTable(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_REMOVEVLANFROMMSTCONFIGTABLE:
            msg_p->type.ret_ui32 = XSTP_MGR_RemoveVlanFromMstConfigTable(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETMSTCONFIGNAME:
            msg_p->type.ret_ui32 =
                XSTP_MGR_SetMstpConfigurationName(msg_p->data.arg_ar1);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETMSTPREVISIONLEVEL:
            msg_p->type.ret_ui32 =
                XSTP_MGR_SetMstpRevisionLevel(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETMSTPMAXHOP:
            msg_p->type.ret_ui32 =
                XSTP_MGR_SetMstpMaxHop(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

#ifdef XSTP_TYPE_PROTOCOL_MSTP
        case XSTP_MGR_IPC_RESTARTSTATEMACHINE:
            XSTP_MGR_RestartStateMachine();
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETMSTPORTAUTOPATHCOST:
            msg_p->type.ret_ui32 = XSTP_MGR_SetMstPortAutoPathCost(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_GETRUNNINGPORTPATHCOST:
            msg_p->type.ret_ui32 = XSTP_MGR_GetRunningPortPathCost(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
            break;
#endif /* #ifdef XSTP_TYPE_PROTOCOL_MSTP */

        case XSTP_MGR_IPC_SETPORTADMINPATHCOSTAGENT:
            msg_p->type.ret_ui32 = XSTP_MGR_SetPortAdminPathCostAgent(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETPORTAUTOPATHCOST:
            msg_p->type.ret_ui32 =
                XSTP_MGR_SetPortAutoPathCost(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETMSTPORTADMINPATHCOSTAGENT:
            msg_p->type.ret_ui32 = XSTP_MGR_SetMstPortAdminPathCostAgent(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2,
                msg_p->data.arg_grp2.arg3);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETPORTSPANNINGTREESTATUS:
            msg_p->type.ret_ui32 = XSTP_MGR_SetPortSpanningTreeStatus(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_GETRUNNINGMSTPORTPATHCOST:
            msg_p->type.ret_ui32 = XSTP_MGR_GetRunningMstPortPathCost(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2,
                &msg_p->data.arg_grp2.arg3);
            msgbuf_p->msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp2);
            break;

        case XSTP_MGR_IPC_GETMSTPORTPATHCOST:
            msg_p->type.ret_bool = XSTP_MGR_GetMstPortPathCost(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2,
                &msg_p->data.arg_grp2.arg3);
            msgbuf_p->msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp2);
            break;

        case XSTP_MGR_IPC_GETLPORTDEFAULTPATHCOST:
            msg_p->type.ret_ui32 = XSTP_MGR_GetLportDefaultPathCost(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
            break;

        case XSTP_MGR_IPC_GETDOT1DMSTENTRY:
            msg_p->type.ret_bool = XSTP_MGR_GetDot1dMstEntry(
                msg_p->data.arg_grp4.arg1, &msg_p->data.arg_grp4.arg2);
            msgbuf_p->msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp4);
            break;

        case XSTP_MGR_IPC_GETNEXTDOT1DMSTENTRY:
            msg_p->type.ret_bool = XSTP_MGR_GetNextDot1dMstEntry(
                &msg_p->data.arg_grp4.arg1, &msg_p->data.arg_grp4.arg2);
            msgbuf_p->msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp4);
            break;

        case XSTP_MGR_IPC_GETDOT1DBASEENTRY:
            msg_p->type.ret_bool =
                XSTP_MGR_GetDot1dBaseEntry(&msg_p->data.arg_base_entry);
            msgbuf_p->msg_size = XSTP_MGR_GET_MSG_SIZE(arg_base_entry);
            break;

        case XSTP_MGR_IPC_GETDOT1DBASEPORTENTRY:
            msg_p->type.ret_bool =
                XSTP_MGR_GetDot1dBasePortEntry(&msg_p->data.arg_base_port_entry);
            msgbuf_p->msg_size = XSTP_MGR_GET_MSG_SIZE(arg_base_port_entry);
            break;

        case XSTP_MGR_IPC_GETNEXTDOT1DBASEPORTENTRY:
            msg_p->type.ret_bool =
                XSTP_MGR_GetNextDot1dBasePortEntry(&msg_p->data.arg_base_port_entry);
            msgbuf_p->msg_size = XSTP_MGR_GET_MSG_SIZE(arg_base_port_entry);
            break;

        case XSTP_MGR_IPC_GETRUNNINGMSTPCONFIGURATIONNAME:
            msg_p->type.ret_ui32 =
                XSTP_MGR_GetRunningMstpConfigurationName(msg_p->data.arg_ar1);
            msgbuf_p->msg_size = XSTP_MGR_GET_MSG_SIZE(arg_ar1);
            break;

        case XSTP_MGR_IPC_GETMSTPCONFIGURATIONNAME:
            msg_p->type.ret_bool =
                XSTP_MGR_GetMstpConfigurationName(msg_p->data.arg_ar1);
            msgbuf_p->msg_size = XSTP_MGR_GET_MSG_SIZE(arg_ar1);
            break;

        case XSTP_MGR_IPC_VLANISMSTMEMBER:
            msg_p->type.ret_bool = XSTP_MGR_VlanIsMstMember(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_GETPORTSTATEBYVLAN:
            msg_p->type.ret_bool = XSTP_MGR_GetPortStateByVlan(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2,
                &msg_p->data.arg_grp2.arg3);
            msgbuf_p->msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp2);
            break;

        case XSTP_MGR_IPC_GETMAPPEDINSTANCEBYVLAN:
            msg_p->type.ret_bool = XSTP_MGR_GetMappedInstanceByVlan(
                msg_p->data.arg_grp1.arg1, &msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
            break;

#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
        case XSTP_MGR_IPC_SETPORTROOTGUARDSTATUS:
            msg_p->type.ret_ui32 = XSTP_MGR_SetPortRootGuardStatus(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
        case XSTP_MGR_IPC_SETPORTBPDUGUARDSTATUS:
            msg_p->type.ret_ui32 = XSTP_MGR_SetPortBpduGuardStatus(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETPORTBPDUGUARDAUTORECOVERY:
            msg_p->type.ret_ui32 = XSTP_MGR_SetPortBPDUGuardAutoRecovery(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_SETPORTBPDUGUARDAUTORECOVERYINTERVAL:
            msg_p->type.ret_ui32 = XSTP_MGR_SetPortBPDUGuardAutoRecoveryInterval(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;
#endif /* #if (SYS_CPNT_STP_BPDU_GUARD == TRUE) */

#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
        case XSTP_MGR_IPC_SETPORTBPDUFILTERSTATUS:
            msg_p->type.ret_ui32 = XSTP_MGR_SetPortBpduFilterStatus(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;
#endif /* #if (SYS_CPNT_STP_BPDU_FILTER == TRUE) */

#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
        case XSTP_MGR_IPC_SETCISCOPRESTANDARDCOMPATIBILITY:
            msg_p->type.ret_ui32 = XSTP_MGR_SetCiscoPrestandardCompatibility(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;
#endif /* End of #if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE) */
#if (SYS_CPNT_XSTP_TC_PROP_STOP == TRUE)
        case XSTP_MGR_IPC_SETPORTTCPROPSTOP:
            msg_p->type.ret_ui32 = XSTP_MGR_SetPortTcPropStop(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;
#endif
#if (SYS_CPNT_EAPS == TRUE)
        case XSTP_MGR_IPC_SETETHRINGPORTROLE:
            msg_p->type.ret_bool = XSTP_MGR_SetEthRingPortRole(
                                    msg_p->data.arg_grp1.arg1,
                                    msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

#endif /* #if (SYS_CPNT_EAPS == TRUE) */

#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
        case XSTP_MGR_IPC_SETTCPROPGROUPPORTLIST:
            msg_p->type.ret_ui32 = XSTP_MGR_SetTcPropGroupPortList(
                msg_p->data.arg_grp5.arg1,
                msg_p->data.arg_grp5.arg2,
                msg_p->data.arg_grp5.arg3);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case XSTP_MGR_IPC_DELTCPROPGROUP:
            msg_p->type.ret_ui32 =
                XSTP_MGR_DelTcPropGroup(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
            break;
#endif

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_ui32 = XSTP_TYPE_RETURN_ERROR;
            msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of XSTP_MGR_HandleIPCReqMsg */


/*=============================================================================
 * Temporarily placed here, should be removed if all questionable MGR Get
 * functions are moved
 *=============================================================================
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetNextPortMemberOfInstance
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next lport member of this spanning tree instance.
 * INPUT    : mstid     -- mstid pointer
 * OUTPUT   : mstid     -- next mstid pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the instance list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T XSTP_MGR_GetNextPortMemberOfInstance(XSTP_OM_InstanceData_T *om_ptr,
                                                   UI32_T *lport)
{
    UI32_T  i;

    for(i= ((*lport)+1); i<XSTP_TYPE_MAX_NUM_OF_LPORT; i++)
    {
        if (XSTP_OM_IsMemberPortOfInstance(om_ptr, i)==TRUE)
        {
            *lport = i;
            return TRUE;
        }
    }
    return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetMstPortState_
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state in a specified spanning tree.
 * INPUT    :   om_ptr                  -- the pointer of the instance entry.
 *              UI32_T lport            -- lport number
 * OUTPUT   :   U32_T  *state           -- the pointer of state value
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- get successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- lport number out of range
 *              XSTP_TYPE_RETURN_INDEX_OOR   -- mstid  out of range
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
static UI32_T XSTP_MGR_GetMstPortState_(XSTP_OM_InstanceData_T *om_ptr,
                                        UI32_T lport,
                                        UI32_T *state)

{
    XSTP_OM_PortVar_T       *pom_ptr;

    pom_ptr= &(om_ptr->port_info[lport-1]);

#if 0
    if (XSTP_OM_GetSpanningTreeStatus() == XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED)
    {
        if (XSTP_OM_IsMemberPortOfInstance(om_ptr, lport)== FALSE)
        {
            return XSTP_TYPE_RETURN_ERROR;
        }
    }
#endif

    *state = (UI32_T)XSTP_TYPE_PORT_STATE_DISCARDING;
    if (!pom_ptr->common->port_enabled ) /* this port is "DISABLED". */
    {
        *state = XSTP_TYPE_PORT_STATE_DISCARDING;
    }
    else if ((pom_ptr->learning == TRUE) && (pom_ptr->forwarding == FALSE) )
    {
        *state = (UI32_T)XSTP_TYPE_PORT_STATE_LEARNING;
    }
    else if ((pom_ptr->learning == TRUE) && (pom_ptr->forwarding == TRUE) )
    {
        *state = (UI32_T)XSTP_TYPE_PORT_STATE_FORWARDING;
    }
    else if ((pom_ptr->learning ==FALSE) && (pom_ptr->forwarding ==TRUE))
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return XSTP_TYPE_RETURN_OK;

}/* End of XSTP_MGR_GetMstPortState_() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetNextExistingInstance_
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next existed MST instance(active) for mst mapping table.
 * INPUT    : mstid     -- mstid pointer
 * OUTPUT   : mstid     -- next mstid pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the instance list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T XSTP_MGR_GetNextExistingInstance_(UI32_T *mstid)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    BOOL_T                  result;
    char                    arg_buf[20];

    result      = FALSE;

    if ( (*mstid < 0 || *mstid > XSTP_TYPE_MAX_MSTID) && (*mstid!=XSTP_MSTP_GET_FIRST_INSTANCE_FOR_SNMP))
    {
        snprintf(arg_buf, 20, "Instance id (0-%d)",XSTP_TYPE_MAX_MSTID );
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP, XSTP_MGR_GetNextExistedInstance_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, arg_buf);
        return FALSE;
    }
    if (*mstid == XSTP_MSTP_GET_FIRST_INSTANCE_FOR_SNMP)
    {
        *mstid  = 0;
        result  = TRUE;
    }
    else
    {
        while(      XSTP_OM_GetNextInstanceInfoPtr(mstid, &om_ptr)
                &&  (!om_ptr->instance_exist)
             )
        {
            /* Do nothing */
        }
        result = om_ptr->instance_exist;
    }

    return result;

}/* End of XSTP_MGR_GetNextExistingInstance_() */


/*=============================================================================
 * Moved from xstp_task.c
 *=============================================================================
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_TASK_Init
 *-------------------------------------------------------------------------
 * FUNCTION: Init the RSTP task
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void XSTP_TASK_Init(void)
{
/*fix bug 387, just work aroud */
 //   XSTP_OM_InitSemaphore();

    /* Init OM : default state is disabled */
    XSTP_OM_Init();

    memset(&XSTP_TASK_HdTimerEntry, 0, sizeof(L_HOLD_TIMER_Entry_T));
    L_HOLD_TIMER_InitHoldTimer(&XSTP_TASK_HdTimerEntry, XSTP_TYPE_ST, XSTP_TYPE_LT);

    return;
} /* End of XSTP_TASK_Init() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_TASK_Create_InterCSC_Relation
 *-------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void XSTP_TASK_Create_InterCSC_Relation(void)
{
    /* Init backdoor call back functions
     */
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("xstp",
        SYS_BLD_STA_GROUP_IPCMSGQ_KEY, XSTP_BACKDOOR_Main);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_TASK_StartTimerEvent
 * ------------------------------------------------------------------------
 * FUNCTION : Service routine to start the periodic timer event for the
 *            spanning tree.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : The periodic timer event is sent to the task which creates
 *            the timer. Hence we have to set the timer by the spanning tree
 *            task itself.
 * ------------------------------------------------------------------------
 */
void    XSTP_TASK_StartTimerEvent(void)
{
    void*              timer_id;

    timer_id = SYSFUN_PeriodicTimer_Create();
    if (SYSFUN_PeriodicTimer_Start(timer_id, XSTP_TYPE_TIMER_TICKS2SEC,
            XSTP_TYPE_EVENT_TIMER) == FALSE)
    {
        printf("\r\n%s: Start timer failed!\r\n", __FUNCTION__);
    }

    /*XSTP_TASK_TimerId   = timer_id;*/

    timer_id = SYSFUN_PeriodicTimer_Create();
    if (SYSFUN_PeriodicTimer_Start(timer_id, XSTP_TYPE_PT,
            XSTP_TYPE_EVENT_HDTIMER) == FALSE)
    {
        printf("\r\n%s: Start hold timer failed!\r\n", __FUNCTION__);
    }

    /*XSTP_TASK_HdTimerId = timer_id;*/
    return;

}/* End of XSTP_TASK_StartTimerEvent */

/* modified from xstp_task.c */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_TASK_ProcessHdTimer
 * ------------------------------------------------------------------------
 * PURPOSE  : Handle Hold Timer.
 * INPUT    : option -- which kind of hold timer event
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void XSTP_TASK_ProcessHdTimer(UI32_T option)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    if (L_HOLD_TIMER_Handler(&XSTP_TASK_HdTimerEntry, option))
    {
        XSTP_MGR_UpdateStateMachine();
    }
    return ;
}/* End of XSTP_TASK_ProcessHdTimer() */


/* created for replacing xstp_task */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_BpduRcvd_Callback
 * ------------------------------------------------------------------------
 * PURPOSE  : Handle the callback event when receiving a BPDU packet.
 * INPUT    : mref_handle_p -- packet buffer and return buffer function pointer.
 *            dst_mac       -- the destination MAC address of this packet.
 *            src_mac       -- the source MAC address of this packet.
 *            tag_info      -- tag information
 *            type          -- packet type
 *            pkt_length    -- pdu length
 *            src_unit         -- unit
 *            src_port         -- src_port
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void XSTP_MGR_BpduRcvd_Callback(L_MM_Mref_Handle_T *mref_handle_p,
                                UI8_T *dst_mac,
                                UI8_T *src_mac,
                                UI16_T tag_info,
                                UI16_T type,
                                UI32_T pkt_length,
                                UI32_T  src_unit,
                                UI32_T  src_port)
{
    XSTP_TYPE_MSG_T msg;
    UI32_T  ifindex;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE){
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    if(SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_UserPortToLogicalPort(src_unit,src_port,&ifindex)){
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    msg.msg_type = ((0x000F & XSTP_TYPE_MSG_BPDU) << 12) | (0x0FFF & tag_info);
    memcpy(msg.saddr, src_mac, 6);
    msg.mref_handle_p = mref_handle_p;
    msg.pkt_length  = (UI16_T)type;
    msg.lport       = (UI16_T)ifindex;

    XSTP_TASK_ProcessHdTimer(L_HOLD_TIMER_NEW_JOB_EV);
    XSTP_MGR_ProcessRcvdBpdu(&msg);
}

#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPortRootGuardStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set root guard status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T status           -- the status value
 *                                         XSTP_TYPE_PORT_ROOT_GUARD_ENABLED
 *                                         XSTP_TYPE_PORT_ROOT_GUARD_DISABLED
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK
 *              XSTP_TYPE_RETURN_ERROR
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetPortRootGuardStatus(UI32_T lport, UI32_T status)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_XSTP,
                                 XSTP_MGR_SetPortRootGuardStatus_Fun_No,
                                 EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                 "lport (1-XSTP_TYPE_MAX_NUM_OF_LPORT)");
        return XSTP_TYPE_RETURN_PORTNO_OOR;
    }

    if (    (status !=  XSTP_TYPE_PORT_ROOT_GUARD_ENABLED)
         && (status !=  XSTP_TYPE_PORT_ROOT_GUARD_DISABLED)
       )
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);

    om_ptr  = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);

    if (pom_ptr->common->root_guard_status == status)
    {
        /* +++ LeaveCriticalRegion +++ */
        XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
        return XSTP_TYPE_RETURN_OK;
    }

    if (status == XSTP_TYPE_PORT_ROOT_GUARD_ENABLED)
    {
        pom_ptr->common->root_guard_status = TRUE;
    }
    else
    {
        pom_ptr->common->root_guard_status = FALSE;
        pom_ptr->common->root_inconsistent = FALSE;
    }

    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    return XSTP_TYPE_RETURN_OK;
}/* End of XSTP_MGR_SetPortRootGuardStatus() */
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPortBpduGuardStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the BPDU guard status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T status           -- the status value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK
 *              XSTP_TYPE_RETURN_ERROR
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetPortBpduGuardStatus(UI32_T lport, UI32_T status)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    if ((lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT))
    {
        return XSTP_TYPE_RETURN_PORTNO_OOR;
    }

    if (   (status != XSTP_TYPE_PORT_BPDU_GUARD_ENABLED)
        && (status != XSTP_TYPE_PORT_BPDU_GUARD_DISABLED)
       )
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);

    if (    (pom_ptr->common->admin_edge == FALSE)
#if (SYS_CPNT_STP_AUTO_EDGE_PORT == TRUE)
         && (pom_ptr->common->auto_edge == FALSE)
#endif
       )
    {
        XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
        return XSTP_TYPE_RETURN_ERROR;
    }

    pom_ptr->common->bpdu_guard_status = ((status == XSTP_TYPE_PORT_BPDU_GUARD_ENABLED) ? TRUE : FALSE);
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    return XSTP_TYPE_RETURN_OK;
} /* XSTP_MGR_SetPortBpduGuardStatus() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPortBPDUGuardAutoRecovery
 * ------------------------------------------------------------------------
 * PURPOSE :  Set BPDU guard auto recovery status for the specified port.
 * INPUT   :  lport  -- the logical port number
 *            status -- the status value
 * OUTPUT  :  None
 * RETURN  :  XSTP_TYPE_RETURN_CODE_E
 * NOTE    :  None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetPortBPDUGuardAutoRecovery(UI32_T lport, UI32_T status)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    /* BODY
     */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    if ((lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT))
    {
        return XSTP_TYPE_RETURN_PORTNO_OOR;
    }

    if (    (status != XSTP_TYPE_PORT_BPDU_GUARD_AUTO_RECOVERY_ENABLED)
         && (status != XSTP_TYPE_PORT_BPDU_GUARD_AUTO_RECOVERY_DISABLED)
       )
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);

    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);

    if (    (pom_ptr->common->admin_edge == FALSE)
#if (SYS_CPNT_STP_AUTO_EDGE_PORT == TRUE)
         && (pom_ptr->common->auto_edge == FALSE)
#endif
       )
    {
        XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
        return XSTP_TYPE_RETURN_ERROR;
    }

    pom_ptr->common->bpdu_guard_auto_recovery = status;

    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
    return XSTP_TYPE_RETURN_OK;
} /* End of XSTP_MGR_SetPortBPDUGuardAutoRecovery */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPortBPDUGuardAutoRecoveryInterval
 * ------------------------------------------------------------------------
 * PURPOSE :  Set BPDU guard auto recovery status for the specified port.
 * INPUT   :  lport    -- the logical port number
 *            interval -- the interval value
 * OUTPUT  :  None
 * RETURN  :  XSTP_TYPE_RETURN_CODE_E
 * NOTE    :  None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetPortBPDUGuardAutoRecoveryInterval(UI32_T lport, UI32_T interval)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    /* BODY
     */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    if ((lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT))
    {
        return XSTP_TYPE_RETURN_PORTNO_OOR;
    }

    if (    (interval < XSTP_TYPE_MIN_PORT_BPDU_GUARD_AUTO_RECOVERY_INTERVAL)
         || (interval > XSTP_TYPE_MAX_PORT_BPDU_GUARD_AUTO_RECOVERY_INTERVAL)
       )
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);

    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);

    if (    (pom_ptr->common->admin_edge == FALSE)
#if (SYS_CPNT_STP_AUTO_EDGE_PORT == TRUE)
         && (pom_ptr->common->auto_edge == FALSE)
#endif
       )
    {
        XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
        return XSTP_TYPE_RETURN_ERROR;
    }

    pom_ptr->common->bpdu_guard_auto_recovery_interval = interval;

    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
    return XSTP_TYPE_RETURN_OK;
} /* End of XSTP_MGR_SetPortBPDUGuardAutoRecoveryInterval */
#endif /* #if (SYS_CPNT_STP_BPDU_GUARD == TRUE) */

#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPortBpduFilterStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the BPDU filter status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T status           -- the status value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK
 *              XSTP_TYPE_RETURN_ERROR
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetPortBpduFilterStatus(UI32_T lport, UI32_T status)
{
    XSTP_OM_InstanceData_T  *om_ptr;
    XSTP_OM_PortVar_T       *pom_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    if ((lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT))
    {
        return FALSE;
    }

    if (   (status != XSTP_TYPE_PORT_BPDU_FILTER_ENABLED)
        && (status != XSTP_TYPE_PORT_BPDU_FILTER_DISABLED)
       )
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    om_ptr = XSTP_OM_GetInstanceInfoPtr(XSTP_TYPE_CISTID);
    pom_ptr = &(om_ptr->port_info[lport-1]);

    if (    (pom_ptr->common->admin_edge == FALSE)
#if (SYS_CPNT_STP_AUTO_EDGE_PORT == TRUE)
         && (pom_ptr->common->auto_edge == FALSE)
#endif
       )
    {
        XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
        return XSTP_TYPE_RETURN_ERROR;
    }

    pom_ptr->common->bpdu_filter_status = ((status == XSTP_TYPE_PORT_BPDU_FILTER_ENABLED) ? TRUE : FALSE);
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    return XSTP_TYPE_RETURN_OK;
} /* XSTP_MGR_SetPortBpduFilterStatus() */
#endif /* #if (SYS_CPNT_STP_BPDU_FILTER == TRUE) */

#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetCiscoPrestandardCompatibility
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the cisco prestandard compatibility status
 * INPUT    :   status    -- the status value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
 *              XSTP_TYPE_RETURN_ERROR              -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetCiscoPrestandardCompatibility(UI32_T status)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    if (    (status != XSTP_TYPE_CISCO_PRESTANDARD_COMPATIBILITY_ENABLED)
        &&  (status != XSTP_TYPE_CISCO_PRESTANDARD_COMPATIBILITY_DISABLED)
       )
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    /* +++ EnterCriticalRegion +++ */
    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    XSTP_OM_SetCiscoPrestandardCompatibility(status);
    /* +++ LeaveCriticalRegion +++ */
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    return XSTP_TYPE_RETURN_OK;
}/* End of XSTP_MGR_SetCiscoPrestandardCompatibilityStatus() */
#endif /* End of #if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE) */

#if (SYS_CPNT_XSTP_TC_PROP_STOP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPortTcPropStop
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the port tc prop status.
 * INPUT    :   lport            -- lport number
 *              enable_status    -- eanble or disabled
 * OUTPUT   :   None.
 * RETURN   :   XSTP_TYPE_RETURN_CODE_E
 * NOTES    :   none.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetPortTcPropStop(UI32_T lport, BOOL_T enable_status)
{
   if (lport < 1 || lport > XSTP_TYPE_MAX_NUM_OF_LPORT)
    {
        return XSTP_TYPE_RETURN_PORTNO_OOR;
    }

    XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
    XSTP_OM_SetPortTcPropStop(lport, enable_status);
    XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);

    return XSTP_TYPE_RETURN_OK;
}
#endif
#if (SYS_CPNT_EAPS == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetEthRingPortRole
 * ------------------------------------------------------------------------
 * PURPOSE  :   To get the port role for eth ring protocol.
 * INPUT    :   lport       -- lport number (1-based)
 * OUTPUT   :   port_role_p -- pointer to content of port role
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
 *              XSTP_TYPE_RETURN_ERROR              -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_GetEthRingPortRole(
    UI32_T  lport,
    UI32_T  *port_role_p)
{
    UI32_T  ret = XSTP_TYPE_RETURN_ERROR;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        ret = XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }
    else
    {
        XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
        if (TRUE == XSTP_OM_GetEthRingPortRole(lport, port_role_p))
            ret = XSTP_TYPE_RETURN_OK;

        XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
    }

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetEthRingPortRole
 * ------------------------------------------------------------------------
 * PURPOSE  :   To set the port role for eth ring protocol.
 * INPUT    :   lport     -- lport number (1-based)
 *              port_role -- port role to set
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
 *              XSTP_TYPE_RETURN_ERROR              -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetEthRingPortRole(
    UI32_T  lport,
    UI32_T  port_role)
{
    UI32_T  ret = XSTP_TYPE_RETURN_ERROR;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        ret = XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }
    else
    {
        if (port_role < XSTP_TYPE_ETH_RING_PORT_ROLE_MAX)
        {
            XSTP_OM_EnterCriticalSection(XSTP_TYPE_CISTID);
            if (TRUE == XSTP_OM_SetEthRingPortRole(lport, port_role))
                ret = XSTP_TYPE_RETURN_OK;

            XSTP_OM_LeaveCriticalSection(XSTP_TYPE_CISTID);
        }
    }

    return ret;
}

#endif /* End of #if (SYS_CPNT_EAPS == TRUE) */


#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetTcPropGroupPortList
 * ------------------------------------------------------------------------
 * PURPOSE  :   To add/remove the ports to/from a group.
 * INPUT    :   is_add       -- add or remove
 *              group_id     -- group ID
 *              portbitmap   -- ports
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
 *              XSTP_TYPE_RETURN_ERROR              -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetTcPropGroupPortList(BOOL_T is_add,
                              UI32_T group_id,
                              UI8_T portbitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST])
{
    UI32_T i;
    BOOL_T ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    if ((group_id < XSTP_TYPE_TC_PROP_MIN_GROUP_ID) || (group_id > XSTP_TYPE_TC_PROP_MAX_GROUP_ID))
    {
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }

    for(i=1; i <= SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
    {
        UI32_T current_group_id = XSTP_TYPE_TC_PROP_DEFAULT_GROUP_ID;

        if(L_BITMAP_PORT_ISSET(portbitmap, i))
        {
            if (!SWCTRL_LogicalPortExisting(i))
                return XSTP_TYPE_RETURN_INDEX_NEX;

            /* A port can only belong to one group. To check the port has recorded.
             */
            current_group_id = XSTP_OM_GetPropGropIdByPort(i);
            if (   (current_group_id != XSTP_TYPE_TC_PROP_DEFAULT_GROUP_ID)
                && (current_group_id != group_id)
               )
            {
                /* Can't add/delete port which belong to other group
                 */
                return XSTP_TYPE_RETURN_ERROR;
            }
        }
    }

    if (is_add)
    {
        ret = XSTP_OM_AddTcPropGroupPortList(group_id, portbitmap);
    }
    else
    {
        ret = XSTP_OM_DelTcPropGroupPortList(group_id, portbitmap);
    }

    return ((ret == TRUE) ? XSTP_TYPE_RETURN_OK : XSTP_TYPE_RETURN_ERROR);
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_DelTcPropGroup
 * ------------------------------------------------------------------------
 * PURPOSE  :   To delete a group.
 * INPUT    :   group_id     -- group ID
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
 *              XSTP_TYPE_RETURN_ERROR              -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_DelTcPropGroup(UI32_T group_id)
{
    BOOL_T has_port = FALSE;
    BOOL_T ret      = TRUE;
    UI8_T  portbitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return XSTP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    if ((group_id < XSTP_TYPE_TC_PROP_MIN_GROUP_ID) || (group_id > XSTP_TYPE_TC_PROP_MAX_GROUP_ID))
    {
        return XSTP_TYPE_RETURN_INDEX_OOR;
    }

    ret = XSTP_OM_GetTcPropGroupPortbitmap(group_id, portbitmap, &has_port);

    if ((ret == TRUE) && (has_port == TRUE))
    {
        ret = XSTP_OM_DelTcPropGroupPortList(group_id, portbitmap);
    }

    return ((ret == TRUE) ? XSTP_TYPE_RETURN_OK : XSTP_TYPE_RETURN_ERROR);
}

#endif /*#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)*/
/* End of XSTP_MGR.C */
