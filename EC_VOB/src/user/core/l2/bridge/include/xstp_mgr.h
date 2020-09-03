/*-------------------------------------------------------------------------
 * Module Name: xstp_imgr.h
 *-------------------------------------------------------------------------
 * PURPOSE: Definitions for the XSTP
 *-------------------------------------------------------------------------
 * NOTES:
 *
 *-------------------------------------------------------------------------
 * HISTORY:
 *    06/12/2002 - Kelly Chen, Created
 *
 *-------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-------------------------------------------------------------------------
 */

#ifndef _XSTP_MGR_H
#define _XSTP_MGR_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "sysfun.h"
#include "xstp_om.h"
#include "xstp_type.h"


/* NAMING CONSTANT DECLARATIONS
 */

#define XSTP_MSTP_REGION_NAME_LENGTH XSTP_TYPE_REGION_NAME_MAX_LENGTH

#define XSTP_MGR_IPCMSG_TYPE_SIZE sizeof(union XSTP_MGR_IpcMsg_Type_U)

/* For Exceptional Handler */
enum XSTP_MGR_FUN_NO_E
{
    XSTP_MGR_SetSystemSpanningTreeVersion_Fun_No     =   1,
    XSTP_MGR_SetForwardDelay_Fun_No,
    XSTP_MGR_SetHelloTime_Fun_No,
    XSTP_MGR_SetMaxAge_Fun_No,
    XSTP_MGR_SetTransmissionLimit_Fun_No,
    XSTP_MGR_SetPortPathCost_Fun_No,
    XSTP_MGR_GetRunningMstPortPathCost_Fun_No,
    XSTP_MGR_SetPortPriority_Fun_No,
    XSTP_MGR_GetRunningMstPortPriority_Fun_No,
    XSTP_MGR_SetPortLinkTypeMode_Fun_No,
    XSTP_MGR_GetRunningPortLinkTypeMode_Fun_No,
    XSTP_MGR_SetPortProtocolMigration_Fun_No,
    XSTP_MGR_GetRunningPortProtocolMigration_Fun_No,
    XSTP_MGR_SetPortAdminEdgePort_Fun_No,
    XSTP_MGR_GetRunningPortAdminEdgePort_Fun_No,
    XSTP_MGR_GetDot1dMstEntry_Fun_No,
    XSTP_MGR_GetNextDot1dMstEntry_Fun_No,
    XSTP_MGR_GetDot1dMstPortEntry_Fun_No,
    XSTP_MGR_GetNextDot1dMstPortEntry_Fun_No,
    XSTP_MGR_GetNextDot1dMstExtPortEntry_Fun_No,
    XSTP_MGR_SetMstPriority_Fun_No,
    XSTP_MGR_SetMstPortPriority_Fun_No,
    XSTP_MGR_SetMstPortPathCost_Fun_No,
    XSTP_MGR_GetDot1dMstExtPortEntry_Fun_No,
    XSTP_MGR_GetNextExistedInstance_Fun_No,
    XSTP_MGR_SetVlanListToMstConfigTable_Fun_No,
    XSTP_MGR_SetVlanToMstConfigTable_Fun_No,
    XSTP_MGR_RemoveVlanFromMstConfigTable_Fun_No,
    XSTP_MGR_SetMstpConfigurationName_Fun_No,
    XSTP_MGR_SetMstpRevisionLevel_Fun_No,
    XSTP_MGR_SetMstpMaxHop_Fun_No,
    XSTP_MGR_GetNextMstpInstanceVlanMapped_Fun_No,
    XSTP_MGR_VlanIsMstMember_Fun_No,
    XSTP_MGR_IsMstInstanceExistingInMstConfigTable_Fun_No,
    XSTP_MGR_GetMstPortRole_Fun_No,
    XSTP_MGR_GetMstPortState_Fun_No,
    XSTP_MGR_GetPortStateByVlan_Fun_No,
    XSTP_MGR_GetNextVlanMemberByInstance_Fun_No,
    XSTP_MGR_GetNextPortMemberAndInstanceByVlan_Fun_No,
    XSTP_MGR_GetPortMemberListAndInstanceByVlan_Fun_No,
    XSTP_MGR_SetSystemSpanningTreeStatus__Fun_No,
    XSTP_MGR_SetMstPortAutoPathCost_Fun_No,
    XSTP_MGR_SetPortAutoPathCost_Fun_No,
    XSTP_MGR_GetDesignatedRootId_Fun_No,
    XSTP_MGR_GetPortDesignatedRootId_Fun_No,
    XSTP_MGR_GetPortDesignatedBridgeId_Fun_No,
    XSTP_MGR_GetPortDesignatedPort_Fun_No,
    XSTP_MGR_GetBridgeIdComponent_Fun_No,
    XSTP_MGR_SetMstpRowStatus_Fun_No,
    XSTP_MGR_GetMstpRowStatus_Fun_No,
    XSTP_MGR_AttachVlanToMstConfigTable_Fun_No,
    XSTP_MGR_SetPortRootGuardStatus_Fun_No,
    XSTP_MGR_GetPortRootGuardStatus_Fun_No,
    XSTP_MGR_GetRunningPortRootGuardStatus_Fun_No,
};

/* command used in IPC message
 */
enum
{
    XSTP_MGR_IPC_SETCHANGESTATEPORTLISTFORBIDDEN,
    XSTP_MGR_IPC_SETSYSTEMSPANNINGTREESTATUS,
    XSTP_MGR_IPC_SETSYSTEMSPANNINGTREEVERSION,
    XSTP_MGR_IPC_SETFORWARDDELAY,
    XSTP_MGR_IPC_SETHELLOTIME,
    XSTP_MGR_IPC_SETMAXAGE,
    XSTP_MGR_IPC_SETPATHCOSTMETHOD,
    XSTP_MGR_IPC_SETTRANSMISSIONLIMIT,
    XSTP_MGR_IPC_SETSYSTEMGLOBALPRIORITY,
    XSTP_MGR_IPC_SETSYSTEMBRIDGEPRIORITY,
    XSTP_MGR_IPC_SETPORTPATHCOST,
    XSTP_MGR_IPC_SETPORTPRIORITY,
    XSTP_MGR_IPC_SETPORTLINKTYPEMODE,
    XSTP_MGR_IPC_SETPORTPROTOCOLMIGRATION,
    XSTP_MGR_IPC_SETPORTADMINEDGEPORT,
    XSTP_MGR_IPC_SETMSTPRIORITY,
    XSTP_MGR_IPC_SETMSTPORTPRIORITY,
    XSTP_MGR_IPC_SETMSTPORTPATHCOST,
    XSTP_MGR_IPC_ATTACHVLANLISTTOMSTCONFIGTABLE,
    XSTP_MGR_IPC_SETVLANTOMSTCONFIGTABLE,
    XSTP_MGR_IPC_ATTACHVLANTOMSTCONFIGTABLE,
    XSTP_MGR_IPC_REMOVEVLANFROMMSTCONFIGTABLE,
    XSTP_MGR_IPC_SETMSTCONFIGNAME,
    XSTP_MGR_IPC_SETMSTPREVISIONLEVEL,
    XSTP_MGR_IPC_SETMSTPMAXHOP,
    XSTP_MGR_IPC_RESTARTSTATEMACHINE,
    XSTP_MGR_IPC_SETMSTPORTAUTOPATHCOST,
    XSTP_MGR_IPC_SETPORTADMINPATHCOSTAGENT,
    XSTP_MGR_IPC_SETPORTAUTOPATHCOST,
    XSTP_MGR_IPC_SETMSTPORTADMINPATHCOSTAGENT,
    XSTP_MGR_IPC_SETPORTSPANNINGTREESTATUS,
#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
    XSTP_MGR_IPC_SETPORTROOTGUARDSTATUS,
#endif
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    XSTP_MGR_IPC_SETPORTBPDUGUARDSTATUS,
    XSTP_MGR_IPC_SETPORTBPDUGUARDAUTORECOVERY,
    XSTP_MGR_IPC_SETPORTBPDUGUARDAUTORECOVERYINTERVAL,
#endif
#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
    XSTP_MGR_IPC_SETPORTBPDUFILTERSTATUS,
#endif
#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
    XSTP_MGR_IPC_SETCISCOPRESTANDARDCOMPATIBILITY,
#endif /* End of #if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE) */
    /* For Get func */
    XSTP_MGR_IPC_GETRUNNINGMSTPORTPATHCOST,
    XSTP_MGR_IPC_GETMSTPORTPATHCOST,
    XSTP_MGR_IPC_GETLPORTDEFAULTPATHCOST,
    XSTP_MGR_IPC_GETDOT1DMSTENTRY,
    XSTP_MGR_IPC_GETNEXTDOT1DMSTENTRY,
    XSTP_MGR_IPC_GETDOT1DBASEENTRY,
    XSTP_MGR_IPC_GETDOT1DBASEPORTENTRY,
    XSTP_MGR_IPC_GETNEXTDOT1DBASEPORTENTRY,
    XSTP_MGR_IPC_GETRUNNINGMSTPCONFIGURATIONNAME,
    XSTP_MGR_IPC_GETMSTPCONFIGURATIONNAME,
    XSTP_MGR_IPC_VLANISMSTMEMBER,
    XSTP_MGR_IPC_GETPORTSTATEBYVLAN,
    XSTP_MGR_IPC_GETMAPPEDINSTANCEBYVLAN,
    XSTP_MGR_IPC_GETRUNNINGPORTPATHCOST,
       XSTP_MGR_IPC_SETFLODINGBEHAVIOR,
       XSTP_MGR_IPC_SETPORTBPDUFLOODING,
    XSTP_MGR_IPC_SETPORTTCPROPSTOP,
#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
    XSTP_MGR_IPC_SETTCPROPGROUPPORTLIST,
    XSTP_MGR_IPC_DELTCPROPGROUP,
#endif
};


/* MACRO FUNCTION DECLARATIONS
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in XSTP_MGR_IpcMsg_T.data
 */
#define XSTP_MGR_GET_MSG_SIZE(field_name)                       \
            (XSTP_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((XSTP_MGR_IpcMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */

/*-------------------------------------------------------------------------
 * All structures provide CLI, WWB, and SNMP to get specified information.
 *-------------------------------------------------------------------------
 */

typedef struct
{
    /* All R/O */
    UI8_T           dot1d_base_bridge_address[6];
    UI16_T          dot1d_base_num_ports;
    UI8_T           dot1d_base_type;
} XSTP_MGR_Dot1dBaseEntry_T;

typedef struct
{
    /* All R/O */
    /* key */
    UI16_T          dot1d_base_port;

    UI16_T          dot1d_base_port_if_index;
    UI32_T          dot1d_base_port_circuit[2];
    UI32_T          dot1d_base_port_delay_exceeded_discards;
    UI32_T          dot1d_base_port_mtu_exceeded_discards;
} XSTP_MGR_Dot1dBasePortEntry_T;

typedef struct
{
    UI8_T                       dot1d_stp_protocol_specification;
    UI32_T                      dot1d_stp_priority;
    UI32_T                      dot1d_stp_time_since_topology_change;
    UI32_T                      dot1d_stp_top_changes;
    XSTP_TYPE_BridgeId_T        dot1d_stp_designated_root;
    UI32_T                      dot1d_stp_root_cost;
    UI16_T                      dot1d_stp_root_port;
    UI16_T                      dot1d_stp_max_age;
    UI16_T                      dot1d_stp_hello_time;
    UI16_T                      dot1d_stp_hold_time;
    UI16_T                      dot1d_stp_forward_delay;
    UI16_T                      dot1d_stp_bridge_max_age;
    UI16_T                      dot1d_stp_bridge_hello_time;
    UI16_T                      dot1d_stp_bridge_forward_delay;
    UI8_T                       dot1d_stp_version;
    UI8_T                       dot1d_stp_tx_hold_count;
    UI8_T                       dot1d_stp_path_cost_default;   /* 802.1t */

    #ifdef  XSTP_TYPE_PROTOCOL_MSTP
    XSTP_TYPE_BridgeId_T        mstp_cist_regional_root_id;
    UI32_T                      mstp_cist_path_cost;
    BOOL_T                      mstp_topology_change_in_progress;
    #endif /* XSTP_TYPE_PROTOCOL_MSTP */
} XSTP_MGR_Dot1dStpEntry_T;

/* IPC message structure
 */
typedef struct
{
    union XSTP_MGR_IpcMsg_Type_U
    {
        UI32_T cmd;
        BOOL_T ret_bool;
        UI32_T ret_ui32;
    } type; /* the intended action or return value */

    union
    {
        BOOL_T                        arg_bool;
        UI32_T                        arg_ui32;
        char                          arg_ar1[XSTP_TYPE_REGION_NAME_MAX_LENGTH+1];
        XSTP_MGR_Dot1dBaseEntry_T     arg_base_entry;
        XSTP_MGR_Dot1dBasePortEntry_T arg_base_port_entry;
        struct
        {
            UI32_T arg1;
            UI32_T arg2;
        } arg_grp1;
        struct
        {
            UI32_T arg1;
            UI32_T arg2;
            UI32_T arg3;
        } arg_grp2;
        struct
        {
            UI32_T arg1;
            UI32_T arg2;
            UI8_T  arg3[128]; /* should give a constant name for 128 ? */
        } arg_grp3;

        /* for get func only */
        struct
        {
            UI32_T                   arg1;
            XSTP_MGR_Dot1dStpEntry_T arg2;
        } arg_grp4;
#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
        struct
        {
            BOOL_T arg1;
            UI32_T arg2;
            UI8_T  arg3[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
        } arg_grp5;
#endif
    } data; /* the argument(s) for the function corresponding to cmd */
} XSTP_MGR_IpcMsg_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_EnterMasterMode
 *-------------------------------------------------------------------------
 * FUNCTION : Create the CST and set XSTP op_state to "SYS_TYPE_STACKING_MASTER_MODE"
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void XSTP_MGR_EnterMasterMode(void);

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
void XSTP_MGR_EnterSlaveMode(void);

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
void XSTP_MGR_SetTransitionMode(void);

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
void XSTP_MGR_EnterTransitionMode(void);

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
UI32_T XSTP_MGR_GetOperationMode(void);

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
BOOL_T XSTP_MGR_IsMasterMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION : XSTP_MGR_ProcessTimerEvent
 *-------------------------------------------------------------------------
 * PURPOSE  : Process the timer event
 * INPUT    : xstid     -- spanning tree instance id
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void    XSTP_MGR_ProcessTimerEvent(void);

/*-------------------------------------------------------------------------
 * FUNCTION : XSTP_MGR_ProcessRcvdBpdu
 *-------------------------------------------------------------------------
 * PURPOSE  : Process the received BPDU
 * INPUT    : bpdu_msg_ptr  -- BPDU message pointer
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void    XSTP_MGR_ProcessRcvdBpdu(XSTP_TYPE_MSG_T *bpdu_msg_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_InitStateMachine
 *-------------------------------------------------------------------------
 * PURPOSE  : Initialize the state machines
 * INPUT    : xstid     -- spanning tree instance id
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void XSTP_MGR_InitStateMachine(UI32_T xstid);

/*-------------------------------------------------------------------------
 * FUNCTION : XSTP_MGR_PTIStateMachineProgress
 * PURPOSE  : Motivate the port timer state machine
 * INPUT    : xstid -- Spanning Tree Instance ID
 * OUTPUT   : None
 * RETUEN   : TRUE if any of the timers is expired, else FALSE
 * NOTES    : None
 */
BOOL_T  XSTP_MGR_PTIStateMachineProgress(UI32_T xstid);

/*-------------------------------------------------------------------------
 * FUNCTION : XSTP_MGR_StateMachineProgress
 * PURPOSE  : Motivate the port/bridge state machine
 * INPUT    : xstid -- Spanning Tree Instance ID
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 */
void    XSTP_MGR_StateMachineProgress(UI32_T xstid);

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
void    XSTP_MGR_GenerateOneSecondTickSignal(UI32_T xstid);

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
void    XSTP_MGR_LportLinkup_CallBack(UI32_T lport);

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
void    XSTP_MGR_LportLinkdown_CallBack(UI32_T lport);

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
void    XSTP_MGR_PortAdminEnable_CallBack(UI32_T lport);

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
void    XSTP_MGR_PortAdminDisable_CallBack(UI32_T lport);

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
void    XSTP_MGR_PortSpeedDuplex_CallBack(UI32_T lport, UI32_T speed_duplex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_TrunkMemberAdd_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port joins the trunk
 *            as the 2nd or the following member
 * INPUT    : UI32_T    trunk_ifindex
 *            UI32_T    member_ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : member_ifindex is sure to be a normal port asserted by SWCTRL.
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_TrunkMemberAdd_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_TrunkMemberAdd1st_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port joins the trunk
 *            as the 1st member
 * INPUT    : UI32_T    trunk_ifindex
 *            UI32_T    member_ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : member_ifindex is sure to be a normal port asserted by SWCTRL.
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_TrunkMemberAdd1st_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_TrunkMemberDelete_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the 2nd or the following
 *            trunk member is removed from the trunk
 * INPUT    : UI32_T    trunk_ifindex
 *            UI32_T    member_ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : member_ifindex exists but the corresponding lport doesn't.
 *            Hence, we have to convert ifindex to uport and then create
 *            the new lport in the following implementation.
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_TrunkMemberDelete_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_TrunkMemberDeleteLst_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the last trunk member
 *            is removed from the trunk
 * INPUT    : UI32_T    trunk_ifindex
 *            UI32_T    member_ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : member_ifindex exists but the corresponding lport doesn't.
 *            Hence, we have to convert ifindex to uport and then create
 *            the new lport in the following implementation.
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_TrunkMemberDeleteLst_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);

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
void    XSTP_MGR_TrunkMemberPortOperUp_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);

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
void    XSTP_MGR_TrunkMemberPortNotOperUp_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);

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
void    XSTP_MGR_VlanCreated_CallBack(UI32_T vid_ifidx, UI32_T vlan_status);

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
void    XSTP_MGR_VlanDestroy_CallBack(UI32_T vid_ifidx, UI32_T vlan_status);

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
void    XSTP_MGR_VlanMemberAdd_CallBack(UI32_T vid_ifidx, UI32_T lport_ifidx, UI32_T vlan_status);

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
void    XSTP_MGR_VlanMemberDelete_CallBack(UI32_T vid_ifidx, UI32_T lport_ifidx, UI32_T vlan_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_RegisterLportEnterForwarding_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : This is a callback register function providing for other
 *            modules to register their callback function invoked when
 *            a port enters the forwarding state.
 * INPUT    : xstid             -- index of the spanning tree
 *            void (*fun) ()    -- CallBack function pointer.
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
/*void    XSTP_MGR_RegisterLportEnterForwarding_CallBack( void (*fun)(UI32_T xstid, UI32_T lport) );*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_RegisterLportLeaveForwarding_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : This is a callback register function providing for other
 *            modules to register their callback function invoked when
 *            a port leaves the forwarding state.
 * INPUT    : xstid             -- index of the spanning tree
 *            void (*fun) ()    -- CallBack function pointer.
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
/*void    XSTP_MGR_RegisterLportLeaveForwarding_CallBack( void (*fun)(UI32_T xstid, UI32_T lport) );*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_RegisterLportChangeState_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : This is a callback register function providing for other
 *            modules to register their callback function invoked when
 *            a port enters/leaves the forwarding state.
 * INPUT    : void (*fun) ()    -- CallBack function pointer.
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
/*void    XSTP_MGR_RegisterLportChangeState_CallBack( void (*fun)(void) );*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_RegisterStpChangeVersion_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : This is a callback register function providing for other
 *            modules to register their callback function invoked when the
 *            Stp version is changed.
 * INPUT    : void (*fun) ()    -- CallBack function pointer.
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : It will be treated as the version changed when the Stp is
 *            enabled/disabled.
 *-------------------------------------------------------------------------
 */
/*void    XSTP_MGR_RegisterStpChangeVersion_CallBack( void (*fun)(UI32_T mode, UI32_T status) );*/

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
void    XSTP_MGR_AnnounceLportChangeState(void);

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
void    XSTP_MGR_NotifyLportChangeState(void);

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
void    XSTP_MGR_SetChangeStatePortListForbidden(BOOL_T flag);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_RetrieveChangeStateLportList
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will retrieve the lport list which enters/leaves
 *            the forwarding state. Then the XSTP_MGR_ChangeStatePortList
 *            is cleared.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : XSTP_MGR_ChangeStatePortList
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
XSTP_TYPE_LportList_T    *XSTP_MGR_RetrieveChangeStateLportList(void);

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
void XSTP_MGR_ResetChangeStateLportList(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetSystemSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the global spanning tree status.
 * INPUT    :   UI32_T status           -- the status value
 *                                         VAL_xstpSystemStatus_enabled
 *                                         VAL_xstpSystemStatus_disabled
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                      -- set successfully
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR       -- not master mode
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetSystemSpanningTreeStatus(UI32_T status);

/*-------------------------------------------------------------------------
 * PURPOSE  :   Set the spanning tree mode.
 * OUTPUT   :   UI32_T mode         -- the mode value
 *                                     VAL_dot1dStpVersion_stpCompatible(0)
 *                                     VAL_dot1dStpVersion_rstp(2)
 *                                     VAL_dot1dStpVersion_mstp(3)
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
 *              XSTP_TYPE_RETURN_ERROR              -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_ST_STATUS_DISABLED -- status is disabled
 * NOTE     :   Default             -- SYS_DFLT_STP_PROTOCOL_TYPE
 *              Can't set mode when the status is disabled.
 * REF      :   draft-ietf-bridge-rstpmib-02/dot1dStp 16
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetSystemSpanningTreeVersion(UI32_T mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetForwardDelay
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the forward_delay time information.
 * INPUT    :   UI32_T forward_delay     -- the forward_delay value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
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
UI32_T XSTP_MGR_SetForwardDelay(UI32_T forward_delay);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetHelloTime
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the hello_time information.
 * INPUT    :   UI32_T hello_time       -- the hello_time value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK               -- set successfully
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR-- not master mode
 *              XSTP_TYPE_RETURN_INDEX_OOR        -- hello_time out of range
 * NOTE     :   1. Time unit is 1/100 sec
 *              2. Range
 *                 -- XSTP_TYPE_MIN_HELLO_TIME
 *                 -- XSTP_TYPE_MAX_HELLO_TIME
 *              3. Default
 *                 -- XSTP_TYPE_DEFAULT_HELLO_TIME
 * * REF    :   RFC-1493/dot1dStp 13
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetHelloTime(UI32_T hello_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMaxAge
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the max_age information.
 * INPUT    :   UI32_T max_age           -- the max_age value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_INDEX_OOR          -- max_age out of range
 * NOTE     :   1. Time unit is 1/100 sec
 *              2. Range
 *                 -- XSTP_TYPE_MIN_MAXAGE
 *                 -- XSTP_TYPE_MAX_MAXAGE
 *              3. Default
 *                 -- XSTP_TYPE_DEFAULT_MAX_AGE
 * REF      :   RFC-1493/dot1dStp 12
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetMaxAge(UI32_T max_age);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPathCostMethod
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the default path cost calculation method.
 * INPUT    :   UI32_T  pathcost_method  -- the method value
 *                      VAL_dot1dStpPathCostDefault_stp8021d1998(1)
 *                      VAL_dot1dStpPathCostDefault_stp8021t2001(2)
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
 *              XSTP_TYPE_RETURN_ERROR              -- failed
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
UI32_T XSTP_MGR_SetPathCostMethod(UI32_T pathcost_method);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetTransmissionLimit
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the transmission limit count vlaue.
 * INPUT    :   UI32_T  tx_hold_count    -- the TXHoldCount value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
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
UI32_T XSTP_MGR_SetTransmissionLimit(UI32_T tx_hold_count);

/* ------------------------------------------------------------------------
 * PURPOSE  :   Set the global priority value
 *              when the switch is MST mode.
 * INPUT    :   UI32_T  priority         -- the priority value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                -- set successfully
 *              XSTP_TYPE_RETURN_ERROR             -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *              XSTP_TYPE_RETURN_INDEX_OOR         -- priority out of range
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
UI32_T XSTP_MGR_SetSystemGlobalPriority(UI32_T priority);

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
UI32_T XSTP_MGR_SetSystemBridgePriority(UI32_T priority);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPortPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the path_cost of the port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T path_cost        -- the path_cost value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
 *              XSTP_TYPE_RETURN_ERROR              -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR         -- port number out of range
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
                                UI32_T path_cost);

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
                                          UI32_T *path_cost);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetMstPortPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the path_cost of the port.
 * INPUT    :   UI32_T lport            -- lport number
                UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *path_cost       -- pointer of the path_cost value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * REF      :   RFC-1493/dot1dStpPortEntry 5
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_MGR_GetMstPortPathCost(UI32_T lport,
                                   UI32_T mstid,
                                   UI32_T *path_cost);

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
                                        UI32_T *path_cost);

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
                                UI32_T priority);

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
                                    UI32_T mode);

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
                                         UI32_T mode);

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
                                     UI32_T mode);

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
                                 XSTP_MGR_Dot1dStpEntry_T  *entry);

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
                                     XSTP_MGR_Dot1dStpEntry_T  *entry);

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
BOOL_T XSTP_MGR_GetDot1dBaseEntry(XSTP_MGR_Dot1dBaseEntry_T *base_entry);

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
BOOL_T XSTP_MGR_GetDot1dBasePortEntry(XSTP_MGR_Dot1dBasePortEntry_T *base_port_entry);

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
BOOL_T XSTP_MGR_GetNextDot1dBasePortEntry(XSTP_MGR_Dot1dBasePortEntry_T *base_port_entry);


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
                               UI32_T priority);

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
                                   UI32_T priority);

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
                                   UI32_T path_cost);

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
                                            UI8_T *vlan_list);

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
                                               UI8_T *vlan_list);

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
UI32_T XSTP_MGR_SetVlanToMstConfigTable(UI32_T mstid,
                                        UI32_T vlan);

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
                                           UI32_T vlan);

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
 * NOTE     :
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_RemoveVlanFromMstConfigTable(UI32_T mstid,
                                             UI32_T vlan);

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
UI32_T XSTP_MGR_ResetVlanToMstConfigTable(UI32_T vlan);

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
UI32_T XSTP_MGR_DestroyVlanFromMstMappingTable(UI32_T vlan_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMstpConfigurationName
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set MSTP configurstion name.
 * INPUT    :   config_name             -- pointer of the config_name
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK         -- set successfully
 *              XSTP_TYPE_RETURN_ERROR      -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 * NOTE     :   Default : the bridage address
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetMstpConfigurationName(char *config_name);

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
UI32_T XSTP_MGR_GetRunningMstpConfigurationName(char *config_name);

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
BOOL_T XSTP_MGR_GetMstpConfigurationName(char *config_name);

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
UI32_T XSTP_MGR_SetMstpRevisionLevel(UI32_T revision);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetMstpMaxHop
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set MSTP Max_Hop count.
 * INPUT    :   U32_T hop_count             -- max_hop value
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
UI32_T XSTP_MGR_SetMstpMaxHop(UI32_T hop_count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_VlanIsMstMember
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funcion returns true if the vlan is in the
 *              specified spanning tree. Otherwise, returns false.
 * INPUT    :   UI32_T mstid             -- the instance id
 *              UI32_T vlan              -- the vlan value
 * OUTPUT   :   none
 * RETURN   :   TRUE/FALSE
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_MGR_VlanIsMstMember (UI32_T mstid,
                                 UI32_T vlan);

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
                                   UI32_T *state);

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
                                                    UI32_T *lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetPortMemberListAndInstanceByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  : Get port member list and instance by a specified vlan
 * INPUT    : vid       -- vlan id
 *            mstid     -- instance value pointer
 *            portlist  -- portlist pointer
 * OUTPUT   : mstid     -- instance value pointer
 *            portlist  -- portlist pointer
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_MGR_GetPortMemberListAndInstanceByVlan(UI32_T vid,
                                                    UI32_T *mstid,
                                                    UI8_T *portlist);

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
                                         UI32_T *mstid);

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
                                                       UI8_T *portlist);

#ifdef  XSTP_TYPE_PROTOCOL_MSTP

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
                                       UI32_T *ext_path_cost);

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
void XSTP_MGR_RestartStateMachine(void);

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
                                       UI32_T mstid);

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
BOOL_T XSTP_MGR_SetMstpRowStatus(UI32_T mstid, UI32_T row_status);

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
UI32_T XSTP_MGR_SetPortAdminPathCostAgent(UI32_T lport, UI32_T path_cost);

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
UI32_T XSTP_MGR_SetPortAutoPathCost(UI32_T lport);

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
UI32_T XSTP_MGR_SetMstPortAdminPathCostAgent(UI32_T lport, UI32_T mstid, UI32_T path_cost);

#if ((SYS_CPNT_SWDRV_CACHE == TRUE) && (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP))

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
void    XSTP_MGR_SetVlanMsgqId(UI32_T msgq_id);

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
void    XSTP_MGR_GetVlanMsgqId(UI32_T *msgq_id);

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
void    XSTP_MGR_VlanCreatedForCache_CallBack(UI32_T vid_ifidx, UI32_T vlan_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_VlanDestroyForCache_CallBack
 * ------------------------------------------------------------------------
 * PURPOSE  : Service the callback from VLAN_MGR when a vlan is destroyed
 * INPUT    : vid_ifidx     -- specify which vlan has just been destroyed
 *            vlan_status   --
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void    XSTP_MGR_VlanDestroyForCache_CallBack(UI32_T vid_ifidx, UI32_T vlan_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_VlanMemberAddForCache_CallBack
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
void    XSTP_MGR_VlanMemberAddForCache_CallBack(UI32_T vid_ifidx, UI32_T lport_ifidx, UI32_T vlan_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_VlanMemberDeleteForCache_CallBack
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
void    XSTP_MGR_VlanMemberDeleteForCache_CallBack(UI32_T vid_ifidx, UI32_T lport_ifidx, UI32_T vlan_status);

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
void    XSTP_MGR_ProcessVlanCallbackEvent();

#endif /* ((SYS_CPNT_SWDRV_CACHE == TRUE) && (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP))*/

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
UI32_T XSTP_MGR_SetPortSpanningTreeStatus(UI32_T lport, UI32_T status);

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
void XSTP_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

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
void XSTP_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

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
BOOL_T XSTP_MGR_SetFloodingBpduWhenStpDisabled(BOOL_T flooding_bpdu);

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
BOOL_T XSTP_MGR_GetBpduBehaviorWhenStpDisabled(BOOL_T *flooding_bpdu);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetRunningBpduBehaviorWhenStpDisabled
 * ------------------------------------------------------------------------
 * PURPOSE  : Get bpdu behavior when spanning tree is disabled.
 * INPUT    : flooding_bpdu -- True: Flood BPDUs when spanning tree is disabled.
 *                          -- FALSE: Don't flood BPDUs when spanning tree is disabled.
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_GetRunningBpduBehaviorWhenStpDisabled(BOOL_T *flooding_bpdu);

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
BOOL_T XSTP_MGR_SetBackupRootStatus(BOOL_T is_enabled);

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
BOOL_T XSTP_MGR_GetBackupRootStatus(BOOL_T *is_enabled);

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
UI32_T XSTP_MGR_GetRunningBackupRootStatus(BOOL_T *is_enabled);

#endif /* SYS_CPNT_SUPPORT_XSTP_BACKUP_ROOT == TRUE */

#if (SYS_CPNT_MSTP_SUPPORT_PVST == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetVlanSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the spanning tree status for the specified vlan.
 * INPUT    :   UI32_T vid              -- vlan id
 *              BOOL_T is_enabled       -- TRUE  (enabled)
 *                                         FALSE (disabled)
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK             -- set successfully
 *              XSTP_TYPE_RETURN_ERROR          -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_INDEX_NEX      -- vlan not existed
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_SetVlanSpanningTreeStatus(UI32_T vid, BOOL_T is_enabled);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetRunningVlanSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the spanning tree status of the specified vlan.
 * INPUT    :   UI32_T vid              -- vlan id
 * OUTPUT   :   BOOL_T *status          -- pointer of the status value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_MGR_GetRunningVlanSpanningTreeStatus(UI32_T vid, BOOL_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_GetVlanSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the spanning tree status of the specified vlan.
 * INPUT    :   UI32_T vid              -- vlan id
 * OUTPUT   :   BOOL_T *status          -- pointer of the status value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_MGR_GetVlanSpanningTreeStatus(UI32_T vid, BOOL_T *status);

#endif /*(SYS_CPNT_MSTP_SUPPORT_PVST == TRUE)*/

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
void XSTP_MGR_UpdateStateMachine();

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
BOOL_T XSTP_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);


/*=============================================================================
 * Moved from xstp_task.h
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
void XSTP_TASK_Init(void);

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
void XSTP_TASK_Create_InterCSC_Relation(void);

/*=============================================================================
 * Moved from xstp_task.c
 *=============================================================================
 */

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
void    XSTP_TASK_StartTimerEvent(void);

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
void XSTP_TASK_ProcessHdTimer(UI32_T option);

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
 *             UI32_T  src_unit
 *             UI32_T  src_port
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
                                UI32_T  src_port);

void  XSTP_MGR_Decrease(void);
void  XSTP_MGR_Restart();

#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_MGR_SetPortRootGuardStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the port admin root guard status.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T status           -- the status value
 *                                         XSTP_TYPE_PORT_ROOT_GUARD_ENABLED
 *                                         XSTP_TYPE_PORT_ROOT_GUARD_DISABLED
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK
 *              XSTP_TYPE_RETURN_ERROR
 * NOTE     :   Only designated port can be enabled root guard function now.
 * ------------------------------------------------------------------------
 */
UI32_T  XSTP_MGR_SetPortRootGuardStatus(UI32_T lport, UI32_T status);
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
UI32_T XSTP_MGR_SetPortBpduGuardStatus(UI32_T lport, UI32_T status);

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
UI32_T XSTP_MGR_SetPortBPDUGuardAutoRecovery(UI32_T lport, UI32_T status);

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
UI32_T XSTP_MGR_SetPortBPDUGuardAutoRecoveryInterval(UI32_T lport, UI32_T interval);
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
UI32_T XSTP_MGR_SetPortBpduFilterStatus(UI32_T lport, UI32_T status);
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
UI32_T XSTP_MGR_SetCiscoPrestandardCompatibility(UI32_T status);
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
UI32_T XSTP_MGR_SetPortTcPropStop(UI32_T lport, BOOL_T enable_status);
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
    UI32_T  *port_role_p);

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
    UI32_T  port_role);

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
                              UI8_T portbitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);


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
UI32_T XSTP_MGR_DelTcPropGroup(UI32_T group_id);
#endif /*#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)*/

#endif /* _XSTP_MGR_H */

