/* -------------------------------------------------------------------------------------
 * FILE NAME:  VLAN_MGR.h
 * -------------------------------------------------------------------------------------
 * PURPOSE:  This package provides the sevices to manage the RFC2674 Q-bridge MIB.
 * NOTE:  1. The key functions of this module are to provide interfaces for the
 *           upper layer to configure VLAN, update database information base on the
 *           confiugration, and configure the lower layers(swctrl).
 *        2. The following tables defined in RFC2674 Q-Bridge MIB will not be supported
 *           in this package:
 *
 *                  dot1qTpGroupTable
 *                  dot1qForwardAllTable
 *                  dot1qForwardUnregisteredTable
 *                  dot1qStaticMulticastTable
 *                  dot1qPortVlanStatisticsTable
 *                  dot1qPortVlanHCStatisticsTable
 *                  dot1qLearningConstraintsTable
 *
 *        3. This package shall be a reusable package for all the L2/L3 switchs.
 *
 * MODIFICATION HISTORY:
 * Modifier         Date                Description
 * -------------------------------------------------------------------------------------
 * cpyang          6-19-2001            First Created
 *
 * amytu           7-27-2001            Callback Functions, Trunking
 *                 9-21-2001            Conformance requirement of RFC2674 Q-Bridge.
 *                 6-26-2002            Remove callback function called by vlan_task.c
 *                 7-08-2002            Add new API for private vlan
 * Erica Li        4-12-2004            Add callback function for GVRP, to notify GVRP when
 *                                      a port joins trunk it disjoin and join which VLAN
 * -------------------------------------------------------------------------------------
 * Copyright(C)                 Accton Technology Corp. 2001
 * -------------------------------------------------------------------------------------*/

#ifndef _VLAN_MGR_H
#define _VLAN_MGR_H

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "vlan_type.h"


/* NAMING CONSTANT DECLARATIONS
 */
enum VLAN_MGR_FAILED_ERROR
{
		VLAN_MGR_RETURN_OK = 0,
		VLAN_MGR_OPER_MODE_ERROR = 1,
		VLAN_MGR_VALUE_OUT_OF_RANGE,
		VLAN_MGR_OM_GET_ERROR,
		VLAN_MGR_OM_VLAN_PARA_CHECK,
};

#define VLAN_MGR_IPCMSG_TYPE_SIZE sizeof(union VLAN_MGR_IpcMsg_Type_U)

/* command used in IPC message
 */
enum
{
    VLAN_MGR_IPC_SETDOT1QGVRPSTATUS,
    VLAN_MGR_IPC_SETDOT1QCONSTRAINTTYPEDEFAULT,
    VLAN_MGR_IPC_CREATEVLAN,
    VLAN_MGR_IPC_DELETEVLAN,
    VLAN_MGR_IPC_DELETENORMALVLAN,
    VLAN_MGR_IPC_SETDOT1QVLANSTATICNAME,
    VLAN_MGR_IPC_ADDEGRESSPORTMEMBER,
    VLAN_MGR_IPC_ADDEGRESSPORTMEMBERFORGVRP,
    VLAN_MGR_IPC_DELETEEGRESSPORTMEMBER,
    VLAN_MGR_IPC_SETDOT1QVLANSTATICEGRESSPORTS,
    VLAN_MGR_IPC_ADDFORBIDDENEGRESSPORTMEMBER,
    VLAN_MGR_IPC_DELETEFORBIDDENEGRESSPORTMEMBER,
    VLAN_MGR_IPC_SETDOT1QVLANFORBIDDENEGRESSPORTS,
    VLAN_MGR_IPC_ADDUNTAGPORTMEMBER,
    VLAN_MGR_IPC_DELETEUNTAGPORTMEMBER,
    VLAN_MGR_IPC_SETDOT1QVLANSTATICUNTAGGEDPORTS,
    VLAN_MGR_IPC_SETDOT1QVLANSTATICROWSTATUS,
    VLAN_MGR_IPC_SETDOT1QVLANSTATICROWSTATUSFORGVRP,
    VLAN_MGR_IPC_SETVLANADDRESSMETHOD,
    VLAN_MGR_IPC_SETDOT1QVLANSTATICENTRY,
    VLAN_MGR_IPC_SETVLANADMINSTATUS,
    VLAN_MGR_IPC_SETLINKUPDOWNTRAPENABLED,
    VLAN_MGR_IPC_SETDOT1QPVID,
    VLAN_MGR_IPC_SETDOT1QPORTACCEPTABLEFRAMETYPES,
    VLAN_MGR_IPC_SETDOT1QINGRESSFILTER,
    VLAN_MGR_IPC_SETVLANPORTMODE,
    VLAN_MGR_IPC_SETDOT1QPORTGVRPSTATUSENABLED,
    VLAN_MGR_IPC_SETDOT1QPORTGVRPFAILEDREGISTRATIONS,
    VLAN_MGR_IPC_SETDOT1QPORTGVRPLASTPDUORIGIN,
    VLAN_MGR_IPC_GETPORTENTRY,
    VLAN_MGR_IPC_GETVLANPORTENTRY,
    VLAN_MGR_IPC_GETNEXTVLANPORTENTRY,
    VLAN_MGR_IPC_GETDOT1QPORTVLANENTRY,
    VLAN_MGR_IPC_GETNEXTDOT1QPORTVLANENTRY,
    VLAN_MGR_IPC_SETMANAGEMENTVLAN,
    VLAN_MGR_IPC_SETGLOBALMANAGEMENTVLAN,
    VLAN_MGR_IPC_LEAVEMANAGEMENTVLAN,
    VLAN_MGR_IPC_SETIPINTERFACE,
    VLAN_MGR_IPC_SETVLANMGMTIPSTATE,
    VLAN_MGR_IPC_GETVLANMAC,
    VLAN_MGR_IPC_NOTIFYFORWARDINGSTATE,
    VLAN_MGR_IPC_SETNATIVEVLANAGENT,
    VLAN_MGR_IPC_ENABLEPORTDUALMODE,
    VLAN_MGR_IPC_DISABLEPORTDUALMODE,
    VLAN_MGR_IPC_GETRUNNINGPORTDUALMODE,
    VLAN_MGR_IPC_GETDOT1QVLANSTATICENTRYAGENT,
    VLAN_MGR_IPC_GETDOT1QVLANCURRENTENTRYAGENT,
    VLAN_MGR_IPC_GETNEXTDOT1QVLANCURRENTENTRYAGENT,
    VLAN_MGR_IPC_SETDOT1VPROTOCOLGROUPENTRY,
    VLAN_MGR_IPC_SETDOT1VPROTOCOLGROUPID,
    VLAN_MGR_IPC_SETDOT1VPROTOCOLGROUPROWSTATUS,
    VLAN_MGR_IPC_DELDOT1VPROTOCOLGROUPID,
    VLAN_MGR_IPC_RMVDOT1VPROTOCOLGROUPENTRY,
    VLAN_MGR_IPC_SETDOT1VPROTOCOLPORTENTRY,
    VLAN_MGR_IPC_SETDOT1VPROTOCOLPORTGROUPVID,
#if (SYS_CPNT_PROTOCOL_VLAN_PORT_SUPPORT_PRIORITY == TRUE)
    VLAN_MGR_IPC_SETDOT1VPROTOCOLPORTGROUPPRIORITY,
#endif
    VLAN_MGR_IPC_SETDOT1VPROTOCOLPORTROWSTATUS,
    VLAN_MGR_IPC_DELDOT1VPROTOCOLPORTGROUPVID,
    VLAN_MGR_IPC_DELETENORMALVLANAGENT,
    VLAN_MGR_IPC_ADDEGRESSPORTMEMBERAGENT,
    VLAN_MGR_IPC_DELETEEGRESSPORTMEMBERAGENT,
    VLAN_MGR_IPC_SETAUTHORIZEDVLANLIST,
    VLAN_MGR_IPC_SETGLOBALDEFAULTVLAN,
    VLAN_MGR_IPC_SETVOICEVLANID,
    VLAN_MGR_IPC_CHANGEl2IF2L3IF,
    VLAN_MGR_IPC_CHANGEl3IF2L2IF,
    VLAN_MGR_IPC_L3VLANLOGICALMACCHANGE,
    VLAN_MGR_IPC_GEtPORTLISTBYVID,
    VLAN_MGR_IPC_GETVLANMEMBERBYLPORT,
    VLAN_MGR_IPC_SETDOT1QVLANALIAS,
#if (SYS_CPNT_MAC_VLAN == TRUE)
    VLAN_MGR_IPC_GETMACVLANENTRY,
    VLAN_MGR_IPC_GETNEXTMACVLANENTRY,
    VLAN_MGR_IPC_SETMACVLANENTRY,
    VLAN_MGR_IPC_DELETEMACVLANENTRY,
    VLAN_MGR_IPC_DELETEALLMACVLANENTRY,
    VLAN_MGR_IPC_GETNEXTRUNNINGMACVLANENTRY,
#endif
    VLAN_MGR_IPC_SETPORTVLANLIST,
};

/* For Exceptional Handler */
enum VLAN_MGR_FUN_NO_E
{
    VLAN_MGR_CreateVlan_Fun_No     =   1,
    VLAN_MGR_DeleteVlan_Fun_No,
    VLAN_MGR_SetDot1qVlanStaticName_Fun_No,
    VLAN_MGR_AddEgressPortMember_Fun_No,
    VLAN_MGR_DeleteEgressPortMember_Fun_No,
    VLAN_MGR_SetDot1qVlanStaticEgressPorts_Fun_No,
    VLAN_MGR_AddForbiddenEgressPortMember_Fun_No,
    VLAN_MGR_DeleteForbiddenEgressPortMember_Fun_No,
    VLAN_MGR_SetDot1qVlanForbiddenEgressPorts_Fun_No,
    VLAN_MGR_AddUntagPortMember_Fun_No,
    VLAN_MGR_SetDot1qVlanStaticUntaggedPorts_Fun_No,
    VLAN_MGR_SetDot1qVlanStaticRowStatusForGVRP_Fun_No,
    VLAN_MGR_SetDot1qVlanStaticRowStatus_Fun_No,
    VLAN_MGR_SetVlanAddressMethod_Fun_No,
    VLAN_MGR_SetDot1qVlanStaticEntry_Fun_No,
    VLAN_MGR_SetLinkUpDownTrapEnabled_Fun_No,
    VLAN_MGR_GetDot1qVlanStaticEntry_Fun_No,
    VLAN_MGR_GetDot1qVlanCurrentEntry_Fun_No,
    VLAN_MGR_SetDot1qPvid_Fun_No,
    VLAN_MGR_SetDot1qPortAcceptableFrameTypes_Fun_No,
    VLAN_MGR_SetDot1qIngressFilter_Fun_No,
    VLAN_MGR_SetDot1qPortGvrpStatusEnabled_Fun_No,
    VLAN_MGR_SetDot1qPortGvrpFailedRegistrations_Fun_No,
    VLAN_MGR_SetDot1qPortGvrpLastPduOrigin_Fun_No,
    VLAN_MGR_GetPortEntry_Fun_No,
    VLAN_MGR_GetVlanPortEntry_Fun_No,
    VLAN_MGR_GetDot1qPortVlanEntry_Fun_No,
    VLAN_MGR_SetDot1dPortGarpJoinTime_Fun_No,
    VLAN_MGR_SetDot1dPortGarpLeaveTime_Fun_No,
    VLAN_MGR_SetDot1dPortGarpLeaveAllTime_Fun_No,
    VLAN_MGR_GetDot1dPortGarpEntry_Fun_No,
    VLAN_MGR_GetNextVlanId_Fun_No,
    VLAN_MGR_IsVlanExisted_Fun_No,
    VLAN_MGR_IsPortVlanMember_Fun_No,
    VLAN_MGR_IsVlanUntagPortListMember_Fun_No,
    VLAN_MGR_IsVlanForbiddenPortListMember_Fun_No,
    VLAN_MGR_CreatePrivateVlan_Fun_No,
    VLAN_MGR_IsPrivatePort_Fun_No,
    VLAN_MGR_SetPortPvlanMode_Fun_No,
    VLAN_MGR_SetPrivateVlan_Fun_No,
    VLAN_MGR_IsPortTrunkMember_Fun_No,
    VLAN_MGR_SemanticCheck_Fun_No,
    VLAN_MGR_PreconditionForSetVlanInfo_Fun_No,
    VLAN_MGR_SetTrunkMemberPortInfo_Fun_No,
    VLAN_MGR_SetDefaultPortMembership_Fun_No,
    VLAN_MGR_AddVlanMember_Fun_No,
    VLAN_MGR_RemoveVlanMember_Fun_No,
    VLAN_MGR_RemoveVlan_Fun_No,
    VLAN_MGR_SetHybridMode_Fun_No,
    VLAN_MGR_SetQTrunkMode_Fun_No,
    VLAN_MGR_SetAccessMode_Fun_No,
    VLAN_MGR_LocalSetVlanRowStatus_Fun_No,
    VLAN_MGR_VlanRemovable_Fun_No,
    VLAN_MGR_PortRemovable_Fun_No,
    VLAN_MGR_ValidateVlanFields_Fun_No,
    VLAN_MGR_DeleteNormalVlan_Fun_No,
    VLAN_MGR_GetNextVlanList_ByLport_Fun_No,
    VLAN_MGR_JoinMgmtPort_Fun_No,
    VLAN_MGR_DisjoinMgmtPort_Fun_No,
    VLAN_MGR_SetDot1qConstraintTypeDefault_Fun_No,
    VLAN_MGR_DeleteUntagPortMember_Fun_No,
    VLAN_MGR_SetVlanAdminStatus_Fun_No,
    VLAN_MGR_SetVlanPortMode_Fun_No,
    VLAN_MGR_VlanStaticMemberCount_Fun_No,
    VLAN_MGR_PreconditionForSetVlanPortInfo_Fun_No,
    VLAN_MGR_GetNextVlanId_ByLport_Fun_No,
    VLAN_MGR_SetMgmtIpStateForVlan_Fun_No,
    VLAN_MGR_GetMgmtIpStateOfVlan_Fun_No
};


/* MACRO FUNCTION DECLARATIONS
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in VLAN_MGR_IpcMsg_T.data
 */
#define VLAN_MGR_GET_MSG_SIZE(field_name)                       \
            (VLAN_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((VLAN_MGR_IpcMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    UI32_T  lport_ifindex;          /* KEY */

    /* NON-DEFAULT VALUE */
    UI32_T  port_garp_join_time;
    UI32_T  port_garp_leave_time;
    UI32_T  port_garp_leave_all_time;

    /* Changed Status */
    BOOL_T  garp_join_time_changed;
    BOOL_T  garp_leave_time_changed;
    BOOL_T  garp_leave_all_time_changed;

} VLAN_MGR_Garp_RunningCfg_T;

/* IPC message structure
 */
typedef struct
{
	union VLAN_MGR_IpcMsg_Type_U
	{
		UI32_T cmd;
		BOOL_T ret_bool;
		UI32_T ret_ui32;
	} type; /* the intended action or return value */

    union
    {
        UI32_T                          arg_ui32;
        VLAN_MGR_Dot1qVlanStaticEntry_T arg_vlan_entry;
#if(SYS_CPNT_MAC_VLAN == TRUE)
        VLAN_TYPE_MacVlanEntry_T arg_mac_vlan_entry;
#endif
        struct
        {
            UI32_T arg1;
            UI32_T arg2;
        } arg_grp1;

        struct
        {
            UI32_T arg1;
            char   arg2[SYS_ADPT_MAX_VLAN_NAME_LEN+1];
        } arg_grp2;

        struct
        {
            UI32_T vid;
            char   alias[VLAN_TYPE_ALIAS_NAME_LEN+1];
        } arg_alias;

        struct
        {
            UI32_T arg1;
            UI32_T arg2;
            UI32_T arg3;
        } arg_grp3;

        struct
        {
            UI32_T arg1;
            UI8_T  arg2[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
            UI32_T arg3;
        } arg_grp4;

        struct
        {
            UI32_T                   arg1;
            VLAN_OM_Vlan_Port_Info_T arg2;
        } arg_grp5;

        struct
        {
            UI32_T                  arg1;
            VLAN_OM_VlanPortEntry_T arg2;
        } arg_grp6;

        struct
        {
            UI32_T                       arg1;
            VLAN_OM_Dot1qPortVlanEntry_T arg2;
        } arg_grp7;

        struct
        {
            UI32_T arg1;
            BOOL_T arg2;
            UI8_T  arg3;
        } arg_grp8;

        struct
        {
            UI32_T arg1;
            UI8_T  arg2[SYS_ADPT_MAC_ADDR_LEN];
        } arg_grp11;

        struct
        {
            UI32_T arg1;
            BOOL_T arg2;
            UI32_T arg3;
        } arg_grp12;

        struct
        {
            UI32_T                          arg1;
            VLAN_MGR_Dot1qVlanStaticEntry_T arg2;
        } arg_grp13;

        struct
        {
            UI32_T                          arg1;
            UI32_T                          arg2;
            VLAN_OM_Dot1qVlanCurrentEntry_T arg3;
        } arg_grp14;

        struct
        {
            UI32_T arg1;
            UI8_T  arg2[VLAN_OM_1V_MAX_1V_PROTOCOL_VALUE_LENGTH];
            UI32_T arg3;
        } arg_grp15;

        struct
        {
            UI32_T arg1;
            UI8_T  arg2[VLAN_OM_1V_MAX_1V_PROTOCOL_VALUE_LENGTH];
        } arg_grp16;

        struct
        {
            UI32_T arg1;
            UI32_T arg2;
            UI8_T  arg3[VLAN_OM_1V_MAX_1V_PROTOCOL_VALUE_LENGTH];
        } arg_grp17;

        struct
        {
            UI32_T arg1;
            UI32_T arg2;
            UI32_T arg3;
            BOOL_T arg4;
        } arg_grp18;

        struct
        {
            UI32_T arg1;
            UI32_T arg2;
            BOOL_T arg3;
        } arg_grp19;

        struct
        {
            UI32_T arg1;
            UI8_T  arg2[(SYS_ADPT_MAX_VLAN_ID + 7) / 8];
            UI32_T arg3;
        } arg_grp20;

        struct
        {
            UI32_T arg1;
            UI32_T arg2;
            UI32_T arg3;
            UI32_T arg4;
        } arg_grp21;

        struct
        {
            UI32_T arg_ui32;
            UI8_T  arg_mac[SYS_ADPT_MAC_ADDR_LEN];
        } arg_grp_ui32_mac;
#if(SYS_CPNT_MAC_VLAN == TRUE)
        struct
        {
            UI8_T  arg_mac[SYS_ADPT_MAC_ADDR_LEN];
            UI8_T  arg_mask[SYS_ADPT_MAC_ADDR_LEN];
            UI16_T arg_vid;
            UI8_T arg_pri;
        } arg_mac_vid_pri;
#endif
        /* only used by PortAuthSrvc, and it is located in the same process
         * with VLAN currently, so it is suitable to pass pointer addree now
         * for the efficiency and simplicity of implementation
         */
        struct
        {
            UI32_T          arg_ui32_1;
            UI32_T          arg_ui32_2;
            VLAN_OM_VLIST_T *arg_vlist_1;
            VLAN_OM_VLIST_T *arg_vlist_2;
            BOOL_T          arg_bool;
        } arg_grp_ui32_ui32_vlist_vlist_bool;

        struct
        {
            UI32_T arg_ui32;
            BOOL_T arg_bool;
        } arg_ui32_bool;

        struct
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
            UI8_T  arg_ui8;
        } arg_ui32_ui32_ui8;

        struct
        {
            UI32_T arg_ui32;
            UI8_T  arg_vlist[VLAN_TYPE_VLAN_LIST_SIZE];
        } arg_ui32_vlist;
    } data;
} VLAN_MGR_IpcMsg_T;


/* EXPORTED SUBPROGRAM DECLARATION
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Initiate_System_Resources
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will initialize system resouce for vlan_om and mgr.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void VLAN_MGR_Initiate_System_Resources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void VLAN_MGR_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Enable vlan operation while in master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void VLAN_MGR_EnterMasterMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : It's the temporary transition mode between system into master
 *            mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : 1. Call a VLAN_InitAll() function, and this internal function
 *               will free all resources and reinit all databases.
 *--------------------------------------------------------------------------*/
void VLAN_MGR_EnterTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Disable the vlan operation while in slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------- */
void VLAN_MGR_EnterSlaveMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void  VLAN_MGR_SetTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetOperationMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This functions returns the current operation mode of this component
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T  VLAN_MGR_GetOperationMode(void);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - VLAN_MGR_ProvisionComplete
 *-------------------------------------------------------------------------
 * Purpose: This function will tell VLAN that provision is completed
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void  VLAN_MGR_ProvisionComplete(void);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - VLAN_MGR_PreProvisionComplete
 *-------------------------------------------------------------------------
 * Purpose: This function will tell VLAN that preprovision is completed
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void  VLAN_MGR_PreProvisionComplete(void);

/* Dot1qBase group
 */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qGvrpStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton returns true if the GVRP status of the bridge can be
 *            successfully Set.  Otherwise, return false.
 * INPUT    : gvrp_status - VAL_dot1qGvrpStatus_enabled \ VAL_dot1qGvrpStatus_disabled
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : The administrative status requested by management for GVRP.  The
 *            value enabled(1) indicates that GVRP should be enabled on this
 *            device, on all ports for which it has not been specifically disabled.
 *            When disabled(2), GVRP is disabled on all ports and all GVRP packets
 *            will be forwarded transparently.  This object affects all GVRP
 *            Applicant and Registrar state machines.  A transition from disabled(2)
 *            to enabled(1) will cause a reset of all GVRP state machines on all ports
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetDot1qGvrpStatus(UI32_T gvrp_status);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qConstraintTypeDefault
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the constrain type for this device
 *            can be retrieve successfully.  Otherwise, return false.
 * INPUT    : None
 * OUTPUT   : VAL_dot1qConstraintTypeDefault_independent\
 *            VAL_dot1qConstraintTypeDefault_shared
 * RETURN   : TRUE \ FALSE
 * NOTES    : System_default is IVL.
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetDot1qConstraintTypeDefault(UI32_T constrain_type);

/* Dot1qVlanStaticTable
 */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_CreateVlan
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if vlan is successfully created.
 *            Otherwise, false is returned.
 * INPUT    : vid         -- the new created vlan id
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : Vlan info is updated in the database.
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The vlan_status parameter is used to identify dynamic (by GVRP)
 *               or static (by management).
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_CreateVlan(UI32_T vid, UI32_T vlan_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_DeleteVlan
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan is successfully deleted
 *            from the database.  Otherwise, false is returned.
 * INPUT    : vid   -- the existed vlan id
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : The specific vlan is removed from the database.
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_DeleteVlan(UI32_T vid, UI32_T vlan_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_DeleteNormalVlan
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan that is not private
 *            vlan is successfully deleted from the database.
 *            Otherwise, false is returned.
 * INPUT    : vid   -- the existed vlan id
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : The specific vlan is removed from the database.
 * RETURN   : TRUE \ FALSE
 * NOTES    : If the specific vlan is private vlan, return FALSE.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_DeleteNormalVlan(UI32_T vid, UI32_T vlan_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qVlanStaticName
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if vlan name is updated.  False otherwise.
 * INPUT    : vid   -- the vlan id
 *            value -- the vlan name
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : vlan name length is restricted between MINSIZE_dot1qVlanStaticName and
 *            MAXSIZE_dot1qVlanStaticName
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetDot1qVlanStaticName(UI32_T vid, char *value);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qVlanAlias
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if vlan alias is updated.  False otherwise.
 * INPUT    : vid   -- the vlan id
 *            value -- the vlan alias
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : vlan alias length is restricted between 0 and
 *            MAXSIZE_ifAlias
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetDot1qVlanAlias(UI32_T vid, char *vlan_alias);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_AddEgressPortMember
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if a port is successfully join vlan's
 *            egress port list.  Otherwise, return false.
 * INPUT    : vid        -- the vlan id
 *            lport_ifindex -- the port number
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : lport_ifindex is successfully joined vlan's egress port list.
 * RETURN   : TRUE \ FALSE
 * NOTES    : A port may not be added in this set if it is already a member of
 *            the set of ports in ForbiddenEgressPorts.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_AddEgressPortMember(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_AddEgressPortMemberForGVRP
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if a port is successfully join vlan's
 *            egress port list.  Otherwise, return false.
 * INPUT    : vid        -- the vlan id
 *            lport_ifindex -- the port number
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : lport_ifindex is successfully joined vlan's egress port list.
 * RETURN   : TRUE \ FALSE
 * NOTES    : A port may not be added in this set if it is already a member of
 *            the set of ports in ForbiddenEgressPorts.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_AddEgressPortMemberForGVRP(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_DeleteEgressPortMember
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if a port is successfully remove from vlan's
 *            egress port list.  Otherwise, return false.
 * INPUT    : vid           -- the vlan id
 *            lport_ifindex -- the port number
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : lport_ifindex is remove from vlan's egress port list.
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_DeleteEgressPortMember(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qVlanStaticEgressPorts
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if egress port list of the specific vlan
 *            is succesfully updated. Otherwise, return false.
 * INPUT    : vid        -- the vlan id
 *            portlist   -- a port list that contains the ports that request to
 *                          join or leave the specific vlan.
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : none.
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. A port may not be added in egress list if it is already a member of
 *               the set of ports in ForbiddenEgressPorts.
 *            2. In the case of SNMP command, vlan_mgr will do a row create when
 *               the specific vid does not already existed in vlan_om.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetDot1qVlanStaticEgressPorts(UI32_T vid, UI8_T *portlist, UI32_T vlan_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_AddForbiddenEgressPortMember
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if a port is successfully included in  vlan's
 *            forbidden egress port list.  Otherwise, return false.
 * INPUT    : vid           -- the vlan id
 *            lport_ifindex -- the port number
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : lport_ifindex is prohibited to join vlan's egress port list
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1.A port may not be added in this set if it is already a member of
 *              the set of ports in the EgressPortlist
 *            2. lport_ifindex will not be permitted to join vlan's member list
 *            until it is removed from Forbidden_port list.
 *-----------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_AddForbiddenEgressPortMember(UI32_T vid, UI32_T lport_ifindex,UI32_T vlan_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_DeleteForbiddenEgressPortMember
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if a port is successfully remove from vlan's
 *            forbidden egress port list.  Otherwise, return false.
 * INPUT    : vid           -- the vlan id
 *            lport_ifindex -- the port number
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : lport_ifindex is permitted to join vlan's egress port list
 * RETURN   : TRUE \ FALSE
 * NOTES    : lport_ifindex will be permitted to join vlan's member list
 *-----------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_DeleteForbiddenEgressPortMember(UI32_T vid, UI32_T lport_ifindex,UI32_T vlan_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qVlanForbiddenEgressPorts
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if forbidden port list of the specific vlan
 *            is succesfully updated. Otherwise, return false.
 * INPUT    : vid        -- the vlan id
 *            portlist - a port list that contains the ports that are to be or not to be
 *                       forbidden from joining the specific vlan.
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : none.
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1.A port may not be added in this set if it is already a member of
 *              the set of ports in the EgressPortlist
 *            2. lport_ifindex will not be permitted to join vlan's member list
 *            until it is removed from Forbidden_port list.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetDot1qVlanForbiddenEgressPorts(UI32_T vid, UI8_T *portlist, UI32_T vlan_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_AddUntagPortMember
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if a port is successfully included in vlan's
 *            untag port list.  Otherwise, return false.
 * INPUT    : vid           -- the vlan id
 *            lport_ifindex -- the port number
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : lport_ifindex should transmit egress packets for this VLAN as untagged.
 * RETURN   : TRUE \ FALSE
 * NOTES    : lport_ifindex must exist in vlan's egress_port_list before it can join
 *            the untag_pot_list
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_AddUntagPortMember(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_DeleteUntagPortMember
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if a port is successfully remove from vlan's
 *            untag port list.  Otherwise, return false.
 * INPUT    : vid           -- the vlan id
 *            lport_ifindex -- the port number
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : lport_ifindex should transmit egress packets for this VLAN as tagged.
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_DeleteUntagPortMember(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qVlanStaticUntaggedPorts
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the untagged port list of the specific vlan
 *            is succesfully updated. Otherwise, return false.
 * INPUT    : vid        -- the vlan id
 *            portlist - a portlist of untagged port.
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : none.
 * RETURN   : TRUE \ FALSE
 * NOTES    : lport_ifindex must exist in vlan's egress_port_list before it can join
 *            the untag_pot_list
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetDot1qVlanStaticUntaggedPorts(UI32_T vid, UI8_T *portlist, UI32_T vlan_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qVlanStaticRowStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if row status field of the vlan entry can be
 *            set successfully.  Otherwise, return false.
 * INPUT    : vid         -- the vlan id
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
BOOL_T VLAN_MGR_SetDot1qVlanStaticRowStatus(UI32_T vid, UI32_T row_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qVlanStaticRowStatusForGVRP
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if row status field of the vlan entry can be
 *            set successfully.  Otherwise, return false.
 * INPUT    : vid         -- the vlan id
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
BOOL_T VLAN_MGR_SetDot1qVlanStaticRowStatusForGVRP(UI32_T vid, UI32_T row_status);

/* Es3626a Private Mib
 */

/*----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetVlanAddressMethod
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific static vlan entry is
              available.  Otherwise, return false.
 * INPUT    : vid             -- the specific vlan id.
 *          : address_method  -- VAL_vlanAddressMethod_user \
 *                               VAL_vlanAddressMethod_bootp \
 *                               VAL_vlanAddressMethod_dhcp
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : None
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetVlanAddressMethod(UI32_T vid, UI32_T address_method);

/*----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qVlanStaticEntry
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific static vlan entry can be
              created successfully.  Otherwise, return FALSE.
 * INPUT    : vid       -- the specific vlan id.
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : This API only supports CAW and CAG commands for set by record.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetDot1qVlanStaticEntry(VLAN_MGR_Dot1qVlanStaticEntry_T *vlan_entry);

/* RFC 2863 If_Entry
 */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetVlanAdminStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if port admin status is set
 *            successfully.  Otherwise, return false.
 * INPUT    : vid - the specific vlan id
 *            admin_status - VAL_ifAdminStatus_up \ VAL_ifAdminStatus_down
 *                           VAL_ifAdminStatus_testing
 * OUTPUT   : none
 * RETURN   : TRUE  \ FALSE
 * NOTES    : Currently Not Supported in this version.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetVlanAdminStatus(UI32_T vid, UI32_T admin_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetLinkUpDownTrapEnabled
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the trap status can be set successfully
 *            for the specific vlan.  Otherwise, return false.
 * INPUT    : vid - the specific vlan id
 *            trap_status - VAL_ifLinkUpDownTrapEnable_enabled \
 *                           VAL_ifLinkUpDownTrapEnable_disabled
 * OUTPUT   : none
 * RETURN   : TRUE  \ FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetLinkUpDownTrapEnabled(UI32_T vid, UI32_T trap_status);

/* Dot1qPortVlanTable
 */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qPvid
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the pvid of the specific port can be
 *            set successfully. Otherwise, return false.
 * INPUT    : lport_ifindex   -- the port number
 *            pvid     -- the pvid value
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetDot1qPvid(UI32_T lport_ifindex, UI32_T pvid);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qPortAcceptableFrameTypes
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if port acceptable frame type is set
 *            successfully.  Otherwise, return false.
 * INPUT    : lport_ifindex       -- the port number
 *            acceptable_frame_types - VAL_dot1qPortAcceptableFrameTypes_admitAll \
 *                             VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanTagged
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : The default value is VAL_dot1qPortAcceptableFrameTypes_admitAll.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetDot1qPortAcceptableFrameTypes(UI32_T lport_ifindex, UI32_T acceptable_frame_types);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qIngressFilter
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if port ingress filter state is set
 *            successfully.  Otherwise, return false.
 * INPUT    : lport_ifindex         -- the port number
 *            ingress_filter -- VAL_dot1qPortIngressFiltering_true \
 *                              VAL_dot1qPortIngressFiltering_false
 * OUTPUT   : none
 * RETURN   : TRUE  \ FALSE
 * NOTES    : 1. Default value is VAL_dot1qPortIngressFiltering_false
 *            2. This control does not affect VLAN independent BPDU frames, such
 *               as GVRP and STP.  It does affect VLAN dependent BPDU frames, such
 *               as GMRP.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetDot1qIngressFilter(UI32_T lport_ifindex, UI32_T ingress_filter);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetVlanPortMode
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if port_mode is set successfully.
 *            Otherwise, return false.
 * INPUT    : lport_ifindex  -- the port number
 *            port_mode -- VAL_vlanPortMode_hybrid \
 *                         VAL_vlanPortMode_dot1qTrunk
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The default value is "VLAN_MGR_HYBRID_LINK".
 *            2. Port_mode value is defined in leaf_es3626a.h
 *            3. Hybrid mode indicates the specific port can join both tagged and
 *               untagged vlan member list.
 *            4. Trunk mode indicates the specific port can only join tagged vlan
 *               member list and the Acceptable_frame_type field will automatically
 *               be set to VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanTagged.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetVlanPortMode(UI32_T lport_ifindex, UI32_T port_mode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qPortGvrpStatusEnabled
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the port gvrp status can be enable
 *            successfully.  Otherwise, return false.
 * INPUT    : lport_ifindex - the logical port number
 *            gvrp_status   - VAL_dot1qPortGvrpStatus_enabled \
 *                            VAL_dot1qPortGvrpStatus_disabled
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : None
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetDot1qPortGvrpStatusEnabled(UI32_T lport_ifindex, UI32_T gvrp_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qPortGvrpFailedRegistrations
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the gvrp failed registration counter
 *            can be set successfully.  Otherwise, return false.
 * INPUT    : lport_ifindex   -- the logical port number
 * OUTPUT   : dot1qPortGvrpFailedRegistrations in vlan_om is incremented.
 * RETURN   : TRUE \ FALSE
 * NOTES    : None
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetDot1qPortGvrpFailedRegistrations(UI32_T lport_ifindex);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qPortGvrpLastPduOrigin
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the last pdu origin of the port can be
 *            set successfully.  Otherwise, return false.
 * INPUT    : lport_ifindex   -- the logical port number
 *            pdu_mac_address - the source address of the last gvrp message receive
 *            on this port
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetDot1qPortGvrpLastPduOrigin(UI32_T lport_ifindex, UI8_T *pdu_mac_address);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetPortEntry
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific port info can
 *            be retrieve successfully.  Otherwiser, return false.
 * INPUT    : lport_ifindex   -- the logical port number
 * OUTPUT   : *entry   -- the whole data structure to store the value
 * RETURN   : TRUE  \ FALSE
 * NOTES    : The returned entry contains Dot1qPortVlanEntry as well as
 *            Dot1dPortGarpEntry
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_GetPortEntry(UI32_T lport_ifindex, VLAN_OM_Vlan_Port_Info_T *vlan_port_info);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetNextPortEntry
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available port ifno can
 *            be retrieve successfully.  Otherwiser, return false.
 * INPUT    : *lport_ifindex  -- the logical port number
 * OUTPUT   : *lport_ifindex  -- the next logical port number
 *            *entry   -- the whole data structure to store the value
 * RETURN   : TRUE \ FALSE
 * NOTES    : The returned entry contains Dot1qPortVlanEntry as well as
 *            Dot1dPortGarpEntry
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_GetNextPortEntry(UI32_T *lport_ifindex, VLAN_OM_Vlan_Port_Info_T *vlan_port_info);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetPortEntry
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan_port entry can
 *            be retrieve successfully.  Otherwiser, return false.
 * INPUT    : lport_ifindex   -- the logical port number
 * OUTPUT   : *entry   -- the whole data structure to store the value
 * RETURN   : TRUE  \ FALSE
 * NOTES    : The returned entry contains Dot1qPortVlanEntry as well as
 *            Dot1dPortGarpEntry
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_GetVlanPortEntry(UI32_T lport_ifindex, VLAN_OM_VlanPortEntry_T *vlan_port_entry);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetNextVlanPortEntry
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available vlan_port entry can
 *            be retrieve successfully.  Otherwiser, return false.
 * INPUT    : *lport_ifindex  -- the logical port number
 * OUTPUT   : *lport_ifindex  -- the next logical port number
 *            *entry   -- the whole data structure to store the value
 * RETURN   : TRUE \ FALSE
 * NOTES    : The returned entry contains Dot1qPortVlanEntry as well as
 *            Dot1dPortGarpEntry
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_GetNextVlanPortEntry(UI32_T *lport_ifindex, VLAN_OM_VlanPortEntry_T *vlan_port_entry);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetDot1qPortVlanEntry
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific port entry can be retrieve
 *            from vlan_om.  Otherwise, return false.
 * INPUT    : lport_ifindex   -- the logical port number
 * OUTPUT   : *entry   -- the whole data structure to store the value
 * RETURN   : TRUE \ FALSE
 * NOTES    : None
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_GetDot1qPortVlanEntry(UI32_T lport_ifindex, VLAN_OM_Dot1qPortVlanEntry_T *port_vlan_entry);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetNextDot1qPortVlanEntry
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available  port entry can be
 *            retrieve from vlan_om.  Otherwise, return false.
 * INPUT    : *lport_ifindex  -- the logical port number
 * OUTPUT   : *lport_ifindex  -- the next logical port number
 *            *entry   -- the whole data structure to store the value
 * RETURN   : TRUE  \ FALSE
 * NOTES    : 1. The logical port number would be stored in the lport_ifindex variable
 *               if get the next port information.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_GetNextDot1qPortVlanEntry(UI32_T *lport_ifindex, VLAN_OM_Dot1qPortVlanEntry_T *port_vlan_entry);

/* Dot1dPortGarpTable
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1dPortGarpJoinTime
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the garp join time of the specific
 *            port can be successfully set.  Otherwise, return false.
 * INPUT    : lport_ifindex - Port number of local switch.
 *            join_time     - GARP join time to be set to this port (Tick).
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The GARP Join time, in centiseconds
 *            2. The default value is 20.
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetDot1dPortGarpJoinTime(UI32_T lport_ifindex, UI32_T join_time);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1dPortGarpLeaveTime
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the garp leave time of the specific
 *            can be successfully set.  Otherwise, return false.
 * INPUT    : lport_ifindex - Port number of local switch.
 *            leave_time    - GARP leave time to be set to this port (Tick).
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The GARP Leave time, in centiseconds
 *            2. The default value is 60.
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetDot1dPortGarpLeaveTime(UI32_T lport_ifindex, UI32_T leave_time);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1dPortGarpLeaveAllTime
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the garp leave all time of the specific
 *            port can be successfully configured.  Otherwise, return false.
 * INPUT    : lport_ifindex  - Port number of local switch.
 *            leave_all_time - GARP leave all time to be set to this port (Tick).
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The GARP Leave time, in centiseconds
 *            2. The default value is 1000.
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetDot1dPortGarpLeaveAllTime(UI32_T lport_ifindex, UI32_T leave_all_time);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetDot1dPortGarpEntry
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the garp entry of the specific port
 *            can be succesfully retrieved from the database.  Otherwise, return
 *            false.
 * INPUT    : lport_ifindex   -- the logical port number
 * OUTPUT   : *entry   -- the whole data structure to store the value
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_GetDot1dPortGarpEntry(UI32_T lport_ifindex, VLAN_OM_Dot1dPortGarpEntry_T *port_garp_entry);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetNextDot1dPortGarpEntry
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available garp entry of the
 *            specific port can be succesfully retrieved from the database.
 *            Otherwise, return false.
 * INPUT    : *lport_ifindex  -- the logical port number
 * OUTPUT   : *lport_ifindex  -- the next logical port number
 *            *entry   -- the whole data structure to store the value
 * RETURN   : TRUE  \ FALSE
 * NOTES    : None
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_GetNextDot1dPortGarpEntry(UI32_T *lport_ifindex, VLAN_OM_Dot1dPortGarpEntry_T *port_garp_entry);

/* Management VLAN API
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetManagementVlan
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the specified VLAN as management VLAN
 * INPUT    : vid - the specific vlan id
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. For backward compatible. In the new design, when setting IP Address
 *               L2 product shall call VLAN_MGR_SetGlobalManagementVlan() and
 *               VLAN_MGR_SetIpInterface(), and the L3 product shall call
 *               VLAN_MGR_SetIpInterface();
 *            2. This function willchange the global management VLAN variable,
 *               and this function will also set the is_ip_interface flag in
 *               VLAN_OM_Dot1qVlanCurrentEntry_T for the purpose to identify which
 *               VLAN is IP interface;
 *            3. Restriction:
 *               a. Row status of management vlan cannot be suspended.
 *               b. Management VLAN must be based on static VLAN.  Dynamic vlan,
 *                  created by Dynamic GVRP, cannot be selected as a management vlan.
 *            4. To be phased out, please use VLAN_MGR_SetVlanMgmtIpState() instead
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetManagementVlan(UI32_T vid);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetGlobalManagementVlan
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the specified VLAN as global management VLAN
 * INPUT    : vid   - the specific vlan id
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. if vid is 0, it means just clear the global management VLAN
 *            2. the VLAN that is IP interface could be global management VLAN
 *            3. this function is paired with VLAN_MGR_GetManagementVlan()
 *            4. To be phased out, please use VLAN_MGR_SetVlanMgmtIpState() instead
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetGlobalManagementVlan(UI32_T vid);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_LeaveManagementVlan
 *--------------------------------------------------------------------------
 * PURPOSE  : Remove the IP interface label from the specified VLAN
 * INPUT    : vid - the specific vlan id
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. This function is coupled with VLAN_MGR_SetIpInterface()
 *               and VLAN_MGR_GetNextIpInterface()
 *            2. To be phased out, please use VLAN_MGR_SetVlanMgmtIpState() instead
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_LeaveManagementVlan(UI32_T vid);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetIpInterface
 *--------------------------------------------------------------------------
 * PURPOSE  : Label the specified VLAN as IP interface
 * INPUT    : vid   - the specific vlan id
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. When setting the IP Address, L2 product shall call this function and
 *               VLAN_MGR_SetGlobalManagementVlan(), and the L3 product shall just call
 *               this function
 *            2. Restrictions: IP interface VLAN must be based on static VLAN.  Dynamic vlan,
 *               cannot be set as IP interface.
 *            3. This function is coupled with VLAN_MGR_LeaveManagementVlan()
 *               and VLAN_MGR_GetNextIpInterface()
 *            4. To be phased out, please use VLAN_MGR_SetVlanMgmtIpState() instead
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetIpInterface(UI32_T vid);

/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetL3IPv6Vlan
 * ----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan can be set as
 *            as L3 IPV6 vlan.  Otherwise, return FALSE
 * INPUT    : vid - the specific vlan id
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. Restriction for L3 ipv6 vlan are as follows:
 *               a. There must be AT LEAST ONE static member existed in vlan member
 *                  list.  Members joined by dynamic GVRP is not consider as an
 *                  static vlan member.
 *               b. L3 ipv6 VLAN must be based on static VLAN.  Dynamic vlan,
 *                  created by Dynamic GVRP, cannot be selected as a L3 ipv6 vlan.
 *            2. To be phased out, please use VLAN_MGR_SetVlanMgmtIpState() instead
 * ----------------------------------------------------------------------------
 */
BOOL_T VLAN_MGR_SetL3IPv6Vlan(UI32_T vid);

/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_LeaveL3IPv6Vlan
 * ----------------------------------------------------------------------------
 * PURPOSE  : set specific vlan not a l3 ipv6 vlan.
 * INPUT    : vid - the specific vlan id
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : To be phased out, please use VLAN_MGR_SetVlanMgmtIpState() instead
 * ----------------------------------------------------------------------------
 */
BOOL_T VLAN_MGR_LeaveL3IPv6Vlan(UI32_T vid);

/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetVlanMgmtIpState
 * ----------------------------------------------------------------------------
 * PURPOSE  : Set management vlan state and ip interface state for a vlan.
 * INPUT    : vid        - the identifier of specified vlan
 *            mgmt_state - TRUE  -> set as management vlan
 *                         FALSE -> do nothing
 *            ip_state   - VLAN_MGR_IP_STATE_NONE (0)
 *                         VLAN_MGR_IP_STATE_IPV4 (1)
 *                         VLAN_MGR_IP_STATE_IPV6 (2)
 *                         VLAN_MGR_IP_STATE_UNCHANGED(3) => keep the original
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : None
 * ----------------------------------------------------------------------------
 */
BOOL_T VLAN_MGR_SetVlanMgmtIpState(UI32_T vid, BOOL_T mgmt_state, UI8_T ip_state);

/*  Miselleneous API
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetVlanMac
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns the MAC address of the specific vlan.
 * INPUT    : vid_ifindex -- specify which vlan
 * OUTPUT   : *vlan_mac -- Mac address of the vlan
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_GetVlanMac(UI32_T vid_ifindex, UI8_T *vlan_mac);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_ReportSyslogMessage
 *--------------------------------------------------------------------------
 * PURPOSE  : This function is another interface for each API to use syslog
 *            to report any error message.
 * INPUT    : error_type - the type of error.  Value defined in vlan_type.h
 *            error_msg - value defined in syslog_mgr.h
 *            function_name - the specific name of API to which error occured.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void VLAN_MGR_ReportSyslogMessage(UI8_T error_type, UI8_T error_msg, char *function_name);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_NotifyForwardingState
 *------------------------------------------------------------------------------
 * PURPOSE  : This function updates port forwarding state information in relationship
 *            to vlan_id.
 * INPUT    : vid - the specific vlan this lport_ifindex joins.  0 for all VLANs.
 *            lport_ifindex - the specific port which its forwarding state information
 *                            has changed.
 *            port_tate - forwarding or Not Forwarding.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void VLAN_MGR_NotifyForwardingState(UI32_T vid, UI32_T lport_ifindex, BOOL_T port_state);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_ProcessForwardingSignal
 *------------------------------------------------------------------------------
 * PURPOSE  : This function updates port forwarding state information in relationship
 *            to vlan_id.
 * INPUT    : last_update_vid - the vlan id which was last update on the list.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : In order to avoid VLAN TASK occupies too much resource, this API will
 *            process a fix number of VLAN for each cycle.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_ProcessForwardingSignal(UI32_T    *last_update_vid);

#if (SYS_CPNT_SOFTBRIDGE == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_JoinMgmtPort
 *-------------------------------------------------------------------------
 * PURPOSE  : This function returns TRUE if mgmt port join vlan successfully.
 *            Otherwise, return FALSE.
 * INPUT    : vid_ifindex   -- the vid ifindex
 *            member_option -- VLAN_MGR_TAGGED_ONLY
 *                             VLAN_MGR_BOTH
 *            vlan_status   -- VAL_dot1qVlanStatus_other
 *                             VAL_dot1qVlanStatus_permanent
 *                             VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : only set OM but not set into the ASIC
 * ------------------------------------------------------------------------
 */
BOOL_T VLAN_MGR_JoinMgmtPort(UI32_T vid_ifindex, VLAN_MEMBER_TYPE_E member_option, UI32_T vlan_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_DisjoinMgmtPort
 *-------------------------------------------------------------------------
 * PURPOSE  : This function returns TRUE if mgmt port disjoin vlan successfully.
 *            Otherwise, return FALSE.
 * INPUT    : vid_ifindex   -- the vid ifindex
 *            member_option -- VLAN_MGR_TAGGED_ONLY
 *                             VLAN_MGR_BOTH
 *            vlan_status   -- VAL_dot1qVlanStatus_other \
 *                             VAL_dot1qVlanStatus_permanent \
 *                             VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : only set OM but not set into the ASIC
 * ------------------------------------------------------------------------
 */
BOOL_T VLAN_MGR_DisjoinMgmtPort(UI32_T vid_ifindex, VLAN_MEMBER_TYPE_E member_option, UI32_T vlan_status);
#endif /* #if (SYS_CPNT_SOFTBRIDGE == TRUE) */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_HandleHotInsertion
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
void VLAN_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_HandleHotRemoval
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
void VLAN_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

#if (SYS_CPNT_VLAN_AUTO_JOIN_VLAN_FOR_PVID == TRUE)

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetNativeVlanAgent
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the pvid of the specific port can be
 *            set successfully. Otherwise, return false.
 * INPUT    : lport_ifindex   -- the port number
 *            pvid            -- the pvid value
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The API prohibited from entering/leaving critical section.
 *            2. The API prohibited from calling SYSFUN_USE_CSC() and VLAN_MGR_RELEASE_CSC()
 *            3. The API will only call these funstions which performed critical section.
 *            4. The API should not be called to restore value by using "no" command.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetNativeVlanAgent(UI32_T lport_ifindex, UI32_T pvid);

#endif /*(SYS_CPNT_VLAN_AUTO_JOIN_VLAN_FOR_PVID == TRUE)*/

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_EnablePortDualMode
 * ------------------------------------------------------------------------
 * PURPOSE  : Enable dual-mode and lport joins to untagged menber list.
 * INPUT    : lport             -- logical port number
 *                              -- the range of the value is [1..SYS_ADPT_TOTAL_NBR_OF_LPORT]
 *            vid               -- the vlan id
 * OUTPUT   : None
 * RETURN   : TRUE if successful, else FALSE if failed.
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T VLAN_MGR_EnablePortDualMode(UI32_T lport, UI32_T vid);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_DisablePortDualMode
 * ------------------------------------------------------------------------
 * PURPOSE  : Disable dual-mode and lport joins to tagged menber list.
 * INPUT    : lport             -- logical port number
 *                              -- the range of the value is [1..SYS_ADPT_TOTAL_NBR_OF_LPORT]
 * OUTPUT   : None
 * RETURN   : TRUE if successful, else FALSE if failed.
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T VLAN_MGR_DisablePortDualMode(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetRunningPortDualMode
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dual-mode status and mapped vlan id for this lport.
 * INPUT    :   lport                       -- logical port number
 *              BOOL_T *dual_mode_status    --pointer of dual_mode_status.
 * OUTPUT   :   UI32_T *vid                 -- pointer of mapped vlan id.
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T VLAN_MGR_GetRunningPortDualMode(UI32_T lport, BOOL_T *dual_mode_status, UI32_T *vid);

/*----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetDot1qVlanStaticEntryAgent
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific static vlan entry is
              available.  Otherwise, return false.
 * INPUT    : vid       -- the specific vlan id.
 * OUTPUT   : *vlan_entry -- returns the specific vlan info
 * RETURN   : True/FALSE
 * NOTES    : only for CLI/WEB/SNMP in Dual mode feature
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_GetDot1qVlanStaticEntryAgent(UI32_T vid, VLAN_MGR_Dot1qVlanStaticEntry_T *vlan_entry);

/*----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetNextDot1qVlanStaticEntryAgent
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available static vlan entry is
              available.  Otherwise, return false.
 * INPUT    : vid       --  the specific vlan id.
 * OUTPUT   : vid
 *            *vlan_entry -- returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : only for CLI/WEB/SNMP in Dual mode feature
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_GetNextDot1qVlanStaticEntryAgent(UI32_T *vid, VLAN_MGR_Dot1qVlanStaticEntry_T *vlan_entry);

/*----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetDot1qVlanCurrentEntryAgent
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific current vlan entry is
              available.  Otherwise, return false.
 * INPUT    : time_mark -- the primary search key
 *            vid       -- the secondary search key
 * OUTPUT   : *vlan_entry -- returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : only for CLI/WEB/SNMP in Dual mode feature
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_GetDot1qVlanCurrentEntryAgent(UI32_T time_mark, UI32_T vid, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry);

/*----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetNextDot1qVlanCurrentEntryAgent
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available current vlan entry is
              available.  Otherwise, return false.
 * INPUT    : time_mark -- the primary search key
 *            vid       -- the secondary search key
 * OUTPUT   : *vid      -- the next vlan id
 *            *vlan_entry -- returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : only for CLI/WEB/SNMP in Dual mode feature
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_GetNextDot1qVlanCurrentEntryAgent(UI32_T time_mark, UI32_T *vid, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry);
#endif /* SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE */

/* ===================================================================== */
/* ===================================================================== */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_DeleteNormalVlanAgent
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan that is not private
 *            vlan is successfully deleted from the database.
 *            Otherwise, false is returned.
 * INPUT    : vid   -- the existed vlan id
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The API prohibited from entering/leaving critical section.
 *            2. The API prohibited from calling SYSFUN_USE_CSC() and VLAN_MGR_RELEASE_CSC()
 *            3. The API will only call these funstions which performed critical section.
 *            4. The API should not be called to restore value by using "no" command.
 *            5. VLAN should be able to deleted even though it's being used by PVID on the port.
 *               -- Add port to member list of default vlan.
 *               -- Set pvid to default vlan.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_DeleteNormalVlanAgent(UI32_T vid, UI32_T vlan_status);


/* ===================================================================== */
/* ===================================================================== */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_AddEgressPortMemberAgent
 * ------------------------------------------------------------------------
 * PURPOSE  : lport joins to egress menber list.
 * INPUT    : vid           -- the vlan id
 *            lport_ifindex -- logical port number
 *            vlan_status   -- VAL_dot1qVlanStatus_other \
 *                             VAL_dot1qVlanStatus_permanent \
 *                             VAL_dot1qVlanStatus_dynamicGvrp
 *            tagged        -- TRUE[tagged member]
 *                             FALSE[untagged member]
 * OUTPUT   : None
 * RETURN   : TRUE if successful, else FALSE if failed.
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T VLAN_MGR_AddEgressPortMemberAgent(UI32_T vid, UI32_T lport, UI32_T vlan_status, BOOL_T tagged);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_DeleteEgressPortMemberAgent
 * ------------------------------------------------------------------------
 * PURPOSE  : lport removes from egress menber list.
 * INPUT    : vid           -- the vlan id
 *            lport_ifindex -- logical port number
 *            vlan_status   -- VAL_dot1qVlanStatus_other \
 *                             VAL_dot1qVlanStatus_permanent \
 *                             VAL_dot1qVlanStatus_dynamicGvrp
 *            tagged        -- TRUE[tagged member]
 *                             FALSE[untagged member]
 * OUTPUT   : None
 * RETURN   : TRUE if successful, else FALSE if failed.
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T VLAN_MGR_DeleteEgressPortMemberAgent(UI32_T vid, UI32_T lport, UI32_T vlan_status, BOOL_T tagged);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_ProcessOrphanPortByLport
 * ------------------------------------------------------------------------
 * PURPOSE  : If a port is not in any this vlan, it will join PVID vlan with untagged member.
 * INPUT    : lport_ifindex -- logical port number
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ------------------------------------------------------------------------
 */
BOOL_T VLAN_MGR_ProcessOrphanPortByLport(UI32_T  lport_ifindex);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetAuthorizedVlanList
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the authorized vlan list is set
 *            successfully.  Otherwise, returns false.
 * INPUT    : lport_ifindex     -- lport ifindex
 *            pvid              -- the authorized pvid
 *            tagged_vlist      -- the authorized tagged vlan list
 *            untagged_vlist    -- the authorized untagged vlan list
 *            is_guest          -- whether the port is assigned to Guest VLAN
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : vlan list for this port   | PVID                            | tagged-list   | untagged-list
 *            ---------------------------------------------------------------------------------------------
 *            both tagged and untagged    one member in untagged-list       tagged-list     untagged-list
 *            no tagged, no untagged      VLAN_TYPE_DOT1Q_RESERVED_VLAN_ID  0               0
 *            untagged only               one member in untagged-list       0               untagged-list
 *            tagged only(no PVID)        VLAN_TYPE_DOT1Q_RESERVED_VLAN_ID  tagged-list     0
 *            tagged only(with PVID)      one member in tagged-list         tagged-list     0
 *            restore to default          VLAN_TYPE_DOT1Q_NULL_VLAN_ID      0               0
 *--------------------------------------------------------------------------*/
BOOL_T  VLAN_MGR_SetAuthorizedVlanList(UI32_T lport_ifindex, UI32_T pvid, VLAN_OM_VLIST_T *tagged_vlist, VLAN_OM_VLIST_T *untagged_vlist, BOOL_T is_guest);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetAuthorizedVlanList
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the authorized vlan list is returned
 *            successfully.  Otherwise, returns false.
 * INPUT    : lport_ifindex     -- specify the lport
 * OUTPUT   : pvid              -- the authorized pvid
 *            tagged_vlist      -- the authorized tagged vlan list
 *            untagged_vlist    -- the authorized untagged vlan list
 * RETURN   : TRUE/FALSE
 * NOTES    : caller should free memory of tagged_vlist and untagged_vlist
 *-----------------------------------------------------------------------------
 */
BOOL_T  VLAN_MGR_GetAuthorizedVlanList(UI32_T lport_ifindex, UI32_T *pvid, VLAN_OM_VLIST_T *tagged_vlist, VLAN_OM_VLIST_T *untagged_vlist);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetPortAutoVlanMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set the auto vlan mode of a port.
 * INPUT    : lport_ifindex -- the port number
 *            state         -- the state of port auto vlan mode
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : The default value of auto_vlan_mode is FALSE.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetPortAutoVlanMode(UI32_T lport_ifindex, BOOL_T state);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetGlobalDefaultVlan
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the default VLAN for this device
 *            can be set successfully.  Otherwise, return false.
 * INPUT    : vid
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetGlobalDefaultVlan(UI32_T vid);

/*----------------------------------------------------------------------------------
 * FUNCTION NAME: VLAN_MGR_SetGroupThreadId
 *----------------------------------------------------------------------------------
 * Purpose: Give the thread ID for VLAN_MGR to send event.
 * Input:   thread_id -- the mgr thread ID of CSC group which VLAN joins.
 * Output:  None.
 * Return:  None.
 *----------------------------------------------------------------------------------
 */
void VLAN_MGR_SetGroupThreadId(UI32_T thread_id);

/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetVoiceVlanId
 * ----------------------------------------------------------------------------
 * PURPOSE : Set or reset the Voice VLAN ID
 * INPUT   : vvid - voice VLAN ID
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : Set vvid = 0 to reset
 * ----------------------------------------------------------------------------
 */
BOOL_T VLAN_MGR_SetVoiceVlanId(UI32_T vvid);

#if (SYS_CPNT_MAC_VLAN == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetMacVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the MAC VLAN entry
 * INPUT    : mac_address   - only allow unitcast address
 *            vid           - the VLAN ID
 *                            the valid value is 1 ~ SYS_DFLT_DOT1QMAXVLANID
 *            priority      - the priority
 *                            the valid value is 0 ~ 7
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE    - TRUE if successful;FALSE if failed
 * NOTES    : if SYS_CPNT_MAC_VLAN_WITH_PRIORITY == FALSE, it's recommanded
 *            that set input priority to 0.
 *-------------------------------------------------------------------------
 */
BOOL_T VLAN_MGR_SetMacVlanEntry(UI8_T *mac_address, UI8_T *mask, UI16_T vid, UI8_T priority);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_DeleteMacVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Delete MAC VLAN entry
 * INPUT    : mac_address   - only allow unitcast address
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE    - TRUE if successful;FALSE if failed
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T VLAN_MGR_DeleteMacVlanEntry(UI8_T *mac_address, UI8_T *mask);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_DeleteAllMacVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Delete all MAC VLAN entry
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE    - TRUE if successful;FALSE if failed
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T VLAN_MGR_DeleteAllMacVlanEntry();
#endif /* end of #if (SYS_CPNT_MAC_VLAN == TRUE) */

/* CALLBACK FUNCTIONS
 */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_AddFirstTrunkMember_CallBack
 *------------------------------------------------------------------------------
 * PURPOSE  : Process to add the first member into a Trunk port.
 * INPUT    : trunk_ifindex   -- specify which trunk to join.
 *            member_ifindex  -- specify which member port being add to trunk.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void VLAN_MGR_AddFirstTrunkMember_CallBack(UI32_T trunk_ifidx, UI32_T member_ifidx);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_AddTrunkMember_CallBack
 *------------------------------------------------------------------------------
 * PURPOSE  : Complete AddTrunkMember CallBack function.
 * INPUT    : trunk_ifindex   -- specify which trunk to join to
 *            member_ifindex  -- specify which member port being add to trunk
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void VLAN_MGR_AddTrunkMember_CallBack(UI32_T trunk_ifidx, UI32_T member_ifidx);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_DeleteTrunkMember_CallBack
 *------------------------------------------------------------------------------
 * PURPOSE  : Complete DeleteTrunkMember CallBack function.
 * INPUT    : trunk_ifindex   -- specify which trunk to join to
 *            member_ifindex  -- specify which member port being add to trunk
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void VLAN_MGR_DeleteTrunkMember_CallBack(UI32_T trunk_ifidx, UI32_T member_ifidx);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_DeleteLastTrunkMember_CallBack
 *------------------------------------------------------------------------------
 * PURPOSE  : Complete DeleteLastTrunkMember CallBack function.
 * INPUT    : trunk_ifindex   -- specify which trunk to join to
 *            member_ifindex  -- specify which member port being add to trunk
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void VLAN_MGR_DeleteLastTrunkMember_CallBack(UI32_T trunk_ifidx, UI32_T member_ifidx);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for VLAN MGR.
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
BOOL_T VLAN_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

BOOL_T VLAN_MGR_GetVLANMemberByLport(UI32_T lport_ifindex,UI32_T *number,UI8_T *vlan_list);

BOOL_T VLAN_MGR_GetPortlistByVid(UI32_T vid,UI32_T * number,UI8_T *portlist);
/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_VlanChangeToL3Type
 * ----------------------------------------------------------------------------
 * PURPOSE  : Set ip interface type  for a vlan.
 * INPUT    : vid        - the identifier of specified vlan
 * OUTPUT:
 *               ifindex  -> interface index
 *               if_status   - the interface status (up or down)
 * RETURN   : VLAN_MGR_RETURN_OK
 *                   VLAN_MGR_OM_GET_ERROR
 * NOTES    : None
 * ----------------------------------------------------------------------------
 */
UI32_T VLAN_MGR_VlanChangeToL3Type(UI32_T vid, UI32_T* vid_ifindex, UI32_T* if_status);

#if (SYS_CPNT_RSPAN == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_CreateRspanVlan
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if RSPAN VLAN is successfully created.
 *            Otherwise, false is returned.
 * INPUT    : vid         -- the new created vlan id
 *            vlan_status -- VAL_dot1qVlanStatus_permanent
 * OUTPUT   : Vlan info is updated in the database.
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_CreateRspanVlan(UI32_T vid, UI32_T vlan_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_AddRspanUntagPortMember
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if a port is successfully included in RSPAN
 *            vlan's untag port list.  Otherwise, return false.
 * INPUT    : vid           -- the vlan id
 *            lport_ifindex -- the port number
 *            vlan_status   -- VAL_dot1qVlanStatus_permanent
 * OUTPUT   : lport_ifindex should transmit egress packets for RSPAN VLAN as untagged.
 * RETURN   : TRUE \ FALSE
 * NOTES    : The API is only for RSPAN MGR.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_AddRspanUntagPortMember(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_AddRspanEgressPortMember
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if a port is successfully join RSPAN
 *            vlan's egress port list.  Otherwise, return false.
 * INPUT    : vid           -- the vlan id
 *            lport_ifindex -- the port number
 *            vlan_status   -- VAL_dot1qVlanStatus_permanent
 * OUTPUT   : lport_ifindex is successfully joined RSPAN vlan's egress port list.
 * RETURN   : TRUE \ FALSE
 * NOTES    : The API is only for RSPAN MGR.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_AddRspanEgressPortMember(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_DeleteRspanUntagPortMember
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if a port is successfully remove from RSPAN
 *            vlan's untag port list.  Otherwise, return false.
 * INPUT    : vid           -- the vlan id
 *            lport_ifindex -- the port number
 *            vlan_status   -- VAL_dot1qVlanStatus_permanent
 * OUTPUT   : lport_ifindex should transmit egress packets for this VLAN as tagged.
 * RETURN   : TRUE \ FALSE
 * NOTES    : For destination port
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_DeleteRspanUntagPortMember(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_DeleteRspanEgressPortMember
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if a port is successfully remove from RSPAN
 *            vlan's egress port list.  Otherwise, return false.
 * INPUT    : vid           -- the vlan id
 *            lport_ifindex -- the port number
 *            vlan_status   -- VAL_dot1qVlanStatus_permanent
 * OUTPUT   : lport_ifindex is remove from vlan's egress port list.
 * RETURN   : TRUE \ FALSE
 * NOTES    : For uplink ports and destination port
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_DeleteRspanEgressPortMember(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_DeleteRspanVlan
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan that is RSPAN vlan
 *            is successfully deleted from the database.
 *            Otherwise, false is returned.
 * INPUT    : vid   -- the existed RSPAN vlan id
 *            vlan_status -- VAL_dot1qVlanStatus_permanent
 * OUTPUT   : The specific RSPAN vlan is removed from the database.
 * RETURN   : TRUE \ FALSE
 * NOTES    : If the specific vlan is not RSPAN vlan, return FALSE.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_DeleteRspanVlan(UI32_T vid, UI32_T vlan_status) ;
#endif /* end #if (SYS_CPNT_RSPAN == TRUE) */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetPortVlanList
 *-----------------------------------------------------------------------------
 * PURPOSE : Set the VLAN membership as the given VLAN list for a logical port.
 * INPUT   : lport       - the specified logical port
 *           vlan_list_p - pointer to list of VLAN IDs for the lport to set
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : 1. all untagged for access/hybrid mode, all tagged for trunk mode
 *           2. fail if remove from PVID VLAN for access/hybrid mode
 *-----------------------------------------------------------------------------
 */
BOOL_T  VLAN_MGR_SetPortVlanList(UI32_T lport, UI8_T *vlan_list_p);

#endif  /* _VLAN_MGR_H */
