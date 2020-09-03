/* -------------------------------------------------------------------------------------
 * FILE NAME:  VLAN_MGR.C
 * -------------------------------------------------------------------------------------
 * PURPOSE:  This package provides the sevices to manage the RFC2674 Q-bridge MIB.
 * NOTE:  1. The key functions of this module are to provide interfaces for the
 *           upper layer to configure VLAN, update database information base on the
 *           confiugration, and configure the lower layers(swctrl).
 *        2. The following tables defined in RFC2674 Q-Bridge MIB will not be supported
 *           in this package:

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
 * Modifier      Date          Version      Description
 * -------------------------------------------------------------------------------------
 * cpyang        6-19-2001       V1.0       First Created
 *
 * amytu         7-27-2001       V1.0       Callback Functions, Trunking
 *               9-21-2001       V1.0       Conformance requirement of RFC2674 Q-Bridge.
 *               6-26-2002       V2.0       1. Callback function move to vlan_mgr from vlan_task.
 *                                          2. Move all callback function out of critical section.
 *               7-02-2002       V2.0       SetOtherField for unexisted vlan returns FALSE.
 *               7-08-2002       V2.0       Add new api for private vlan
 *               7-12-2002       V2.0       Modify Ingress-Filtering API to support per
 *                                          system ingress filtering configuration(for ACD).
 *               8-12-2001       V2.0       Modify API after Unit test and private mib
 *               9-05-2002                  1. Fix Dell RTS+90 Issue
 *                                          2. Add PVID change callback
 *                                          3. Sync 1.0 and 2.0
 *               9-23-2002                  Add Transition mode for Stacking
 * Allen Cheng   10-23-2002      V2.0       Modify Transition mode for stacking
 * Allen Cheng   12-19-2002      V2.0       Revised for the trap mechanism changed
 * Kelly_Chen    05-28-2002      V2.0       Allow user to set tag/untag for a private_port
 * kelin         7-07-2003       V2.0       Add new api for Q-Trunk member.
 * Erica Li      12-30-2003      V3.0       Add new api for hot swap.
 * -------------------------------------------------------------------------------------
 * Copyright(C)                 Accton Technology Corp. 2001, 2002
 * -------------------------------------------------------------------------------------*/


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_module.h"
#include "sys_type.h"
#include "leaf_1493.h"
#include "leaf_2674q.h"
#include "leaf_2863.h"
#include "leaf_es3626a.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "l_cvrt.h"
#include "l_mm.h"
#include "l_rstatus.h"
#include "l_stdlib.h"
#include "sysfun.h"
#include "eh_mgr.h"
#include "eh_type.h"
#include "snmp_pmgr.h"
#include "swctrl.h"
#include "swdrv.h"  /*added by Jinhua Wei ,to remove warning ,becaued the relative function's head file not included */
#include "sys_callback_mgr.h"
#include "sys_time.h"
#include "syslog_mgr.h"
#include "syslog_om.h"
#include "syslog_pmgr.h"
#include "syslog_type.h"
#include "trap_event.h"
#include "trk_mgr.h"
#include "xstp_om.h"
#include "vlan_backdoor.h"
#include "vlan_lib.h"
#include "vlan_mgr.h"
#include "vlan_om.h"
#include "vlan_om_private.h"
#include "vlan_type.h"
#include "backdoor_mgr.h"
#include "swctrl_group.h"
#include "netcfg_netdevice.h"
#ifndef WIN32
/* linux heder files */
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#endif
#if (SYS_CPNT_ADD == TRUE)
#include "add_om.h"
#endif
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
#include "sysctrl_xor_mgr.h"
#endif
#if (SYS_CPNT_RSPAN == TRUE)
#include "rspan_om.h"
#endif
#if (SYS_CPNT_STACKING == TRUE)
#include "stktplg_om.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define VLAN_MGR_DOT1Q_DEFAULT_PVID_IFINDEX (VLAN_MGR_DOT1Q_DEFAULT_PVID + SYS_ADPT_VLAN_1_IF_INDEX_NUMBER - 1)


/* MACRO FUNCTION DECLARATIONS
 */
#define VLAN_MGR_RELEASE_CSC(cs_flag, ret_value)        \
{                                                       \
    return ret_value;                                   \
} /* end of VLAN_MGR_RELEASE_CSC() */

#define VLAN_MGR_RELEASE_CSC_FOR_VOID(cs_flag)          \
{                                                       \
    return;                                             \
} /* end of VLAN_MGR_RELEASE_CSC() */

#define VLAN_MGR_SET_IF_OPER_STATUS(if_entry, new_status)                   \
{                                                                           \
    if_entry.vlan_operation_status = new_status;                            \
    SYS_TIME_GetSystemUpTimeByTick(&if_entry.if_last_change);               \
} /* end of VLAN_MGR_SET_IF_OPER_STATUS() */

#define VLAN_MGR_SEND_IF_OPER_STATUS_CHANGED_EVENT(vid)                     \
{                                                                           \
    vlan_operstatus_changed[vid] = TRUE;                                    \
    SYSFUN_SendEvent(cscgroup_thread_id, VLAN_TYPE_PORTSTATE_EVENT);        \
} /* end of VLAN_MGR_SEND_IF_OPER_STATUS_CHANGED_EVENT() */

#if (SYS_CPNT_LLDP == TRUE)
#define VLAN_MGR_NOTIFY_LLDP_VLAN_NAME_CHANGED(vid)                SYS_CALLBACK_MGR_VlanNameChangedCallback(SYS_MODULE_VLAN, vid)
#define VLAN_MGR_NOTIFY_LLDP_PROTOVLAN_GID_BINDING_CHANGED(lport)  SYS_CALLBACK_MGR_ProtoVlanGroupIdBindingChangedCallback(SYS_MODULE_VLAN, lport)
#else
#define VLAN_MGR_NOTIFY_LLDP_VLAN_NAME_CHANGED(vid)
#define VLAN_MGR_NOTIFY_LLDP_PROTOVLAN_GID_BINDING_CHANGED(lport)
#endif

#if (SYS_CPNT_MAC_VLAN == TRUE)
#define IS_NULL_MAC(mac)        (memcmp((mac), null_mac, SYS_ADPT_MAC_ADDR_LEN) == 0)
#define IS_MULTICAST_MAC(mac)   ((mac)[0] & 0x01)
#define IS_BROADCAST_MAC(mac)   (memcmp((mac), broadcast_mac, SYS_ADPT_MAC_ADDR_LEN) == 0)
#endif

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
    #define VLAN_MGR_XOR_LOCK_AND_FAIL_RETURN(ret_val, expr)    \
        do {                                        \
            SYSCTRL_XOR_MGR_GetSemaphore();         \
            if (!(expr))          \
            {                                       \
                SYSCTRL_XOR_MGR_ReleaseSemaphore(); \
                return ret_val;                     \
            }                                       \
        } while (0)

    #define VLAN_MGR_XOR_UNLOCK_AND_RETURN(ret_val)                 \
        do {                                        \
            SYSCTRL_XOR_MGR_ReleaseSemaphore();     \
            return ret_val;                         \
        } while (0)

#else
    #define VLAN_MGR_XOR_LOCK_AND_FAIL_RETURN(ret_val, expr)    \
        do {} while (0)

    #define VLAN_MGR_XOR_UNLOCK_AND_RETURN(ret_val)                 \
        do {return ret_val;} while (0)

#endif /* #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */


/* DATATYPE DECLARATIONS
 */
enum
{
    VLAN_MGR_CS_FLAG_OFF,
    VLAN_MGR_CS_FLAG_ON
};

enum VLAN_PORT_INFO_FIELD_E
{
    PVID_FIELD,
    INGRESS_FIELD,
    ACCEPTABLE_FRAME_TYPE_FIELD,
    GVRP_STATUS_FIELD,
    VLAN_PORT_MODE_FIELD,
    ADMIN_PVID_FIELD,
    ADMIN_ACCEPTABLE_FRAME_TYPE_FIELD,
    ADMIN_INGRESS_FIELD
};

#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
enum VLAN_MGR_TRUNK_LINK_TYPE_E
{
    VLAN_MGR_NOT_TRUNK_LINK = 1,
    VLAN_MGR_TRUNK_LINK
};
#endif

enum
{
    VLAN_MGR_INFERIOR   = 1, /* permit been overwritten or removed       */
    VLAN_MGR_BELOW      = 2, /* permit been overwritten except removed   */
    VLAN_MGR_EQUAL      = 3, /* no limitation                            */
    VLAN_MGR_ABOVE      = 4, /* permit overwritting but not removing     */
    VLAN_MGR_SUPERIOR   = 5  /* permit overwritting or removing          */
};


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void VLAN_MGR_InitDefaultVlanEntry(void);
static void VLAN_MGR_Notify_VlanCreate(UI32_T vlan_ifindex, UI32_T vlan_status);
static void VLAN_MGR_Notify_VlanDestroy(VLAN_OM_Dot1qVlanCurrentEntry_T * vlan_entry, UI32_T vlan_status);
static void VLAN_MGR_Notify_VlanMemberAdd(UI32_T vlan_ifindex, UI32_T lport_ifindex, UI32_T vlan_status);
static void VLAN_MGR_Notify_VlanMemberDelete(UI32_T vlan_ifindex, UI32_T lport_ifindex, UI32_T vlan_status);
static void VLAN_MGR_Notify_IfOperStatusChanged(VLAN_OM_Dot1qVlanCurrentEntry_T * vlan_info, UI32_T oper_status, UI32_T trap_enabled, UI32_T admin_status);
static void VLAN_MGR_Notify_AddFirstTrunkMember(UI32_T vlan_ifindex, UI32_T trunk_ifindex, UI32_T member_ifindex);
static void VLAN_MGR_Notify_AddTrunkMember(UI32_T vlan_ifindex, UI32_T trunk_ifindex, UI32_T member_ifindex);
static void VLAN_MGR_Notify_DeleteTrunkMember(UI32_T vlan_ifindex, UI32_T trunk_ifindex, UI32_T member_ifindex);
static void VLAN_MGR_Notify_DeleteLastTrunkMember(UI32_T vlan_ifindex, UI32_T trunk_ifindex, UI32_T member_ifindex);
static void VLAN_MGR_Notify_FinishAddFirstTrunkMember(UI32_T trunk_ifindex, UI32_T member_ifindex);
static void VLAN_MGR_Notify_FinishAddTrunkMember(UI32_T trunk_ifindex, UI32_T member_ifindex);
static void VLAN_MGR_Notify_FinishDeleteTrunkMember(UI32_T trunk_ifindex, UI32_T member_ifindex);
static void VLAN_MGR_Notify_FinishDeleteLastTrunkMember(UI32_T trunk_ifindex, UI32_T member_ifindex);
static void VLAN_MGR_Notify_VlanPortModeChanged(UI32_T lport_ifindex, UI32_T vlan_port_mode);
static void VLAN_MGR_Notify_PvidChange(UI32_T lport_ifindex, UI32_T old_pvid, UI32_T new_pvid);
static void VLAN_MGR_Notify_VlanMemberDeleteByTrunk(UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T vlan_status);
static void VLAN_MGR_Notify_VlanMemberTagChanged(UI32_T vid_ifindex, UI32_T lport_ifindex);
static void VLAN_MGR_DetailForCreateVlan(UI32_T vid, UI32_T vlan_status, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_info);
static BOOL_T VLAN_MGR_SemanticCheck(void *vlan_info);
static BOOL_T VLAN_MGR_PreconditionForSetVlanInfo(UI32_T vid, UI32_T lport_ifindex);
static BOOL_T VLAN_MGR_PreconditionForSetVlanPortInfo(UI32_T lport_ifindex, UI32_T field, UI32_T field_value);
static BOOL_T VLAN_MGR_SetVlanRowStatus(UI32_T row_status_action, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_info);
static BOOL_T VLAN_MGR_SetTrunkMemberPortInfo(UI32_T,UI32_T, UI32_T);
#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
static BOOL_T VLAN_MGR_SetDefaultPortMembership(UI32_T vid, UI32_T port_num);
#endif
static BOOL_T VLAN_MGR_RemoveVlanMember(UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T member_option, UI32_T current_status, UI32_T vlan_status);
static BOOL_T VLAN_MGR_AddVlanMember(UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T member_option, UI32_T vlan_status);
static BOOL_T VLAN_MGR_RemoveVlan(UI32_T vid_ifindex, UI32_T current_status);
static BOOL_T VLAN_MGR_SetHybridMode(UI32_T lport_ifindex);
static BOOL_T VLAN_MGR_SetQTrunkMode(UI32_T lport_ifindex);
static BOOL_T VLAN_MGR_SetAccessMode(UI32_T lport_ifindex);
static BOOL_T VLAN_MGR_LocalSetVlanRowStatus(UI32_T vid, UI32_T row_status, UI32_T vlan_status);
static BOOL_T VLAN_MGR_VlanRemovable(UI32_T vid);
static BOOL_T VLAN_MGR_PortRemovable(UI32_T vid_ifindex, UI32_T lport_ifindex);
static BOOL_T VLAN_MGR_ValidateVlanFields(VLAN_MGR_Dot1qVlanStaticEntry_T *vlan_info);
static BOOL_T VLAN_MGR_IsPrivateVlanPortMember(UI32_T vid_ifindex, UI32_T lport_ifindex);
static void VLAN_MGR_SetVlanPortEntryToDefault(UI32_T lport_ifindex);
static void VLAN_MGR_SetVlanPortDefaultAttributeToChip(UI32_T lport_ifindex);
static BOOL_T VLAN_MGR_SetDot1qPortAcceptableFrameTypes_(UI32_T lport_ifindex, UI32_T acceptable_frame_types);

#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
static BOOL_T VLAN_MGR_LocalIsQTrunkPortMember(UI32_T lport_ifindex);
static BOOL_T VLAN_MGR_LocalSetPortTrunkLinkMode(UI32_T lport_ifindex, UI32_T trunk_link_mode);
static BOOL_T VLAN_MGR_LocalAddPortTrunkLinkToVlan(UI32_T vid_ifindex);
static BOOL_T VLAN_MGR_LocalSetDot1qPvidToDefault(UI32_T lport_ifindex);
#endif

#if (SYS_CPNT_VLAN_PROVIDING_AT_LEAST_ONE_UNTAGGED_MODE == TRUE)
static BOOL_T VLAN_MGR_ConformToAtLeastOneUntaggedVlan(UI32_T lport);
#endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_AT_LEAST_ONE_UNTAGGED_MODE == TRUE) */

static BOOL_T VLAN_MGR_ConformToSingleUntaggedVlan(UI32_T join_vlan, UI32_T lport, UI32_T vlan_status);

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
static BOOL_T VLAN_MGR_ConformToSingleMode(UI32_T join_vlan, UI32_T lport, UI32_T vlan_status, BOOL_T tagged);
static BOOL_T VLAN_MGR_IsPortDualModeOnVlan(UI32_T lport_ifindex, UI32_T dual_mode_vlan_id);
#endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)*/

static BOOL_T VLAN_MGR_SetDot1qPvidWithoutChangingMembership(UI32_T lport_ifindex, UI32_T pvid,BOOL_T check_port);

static BOOL_T VLAN_MGR_RemoveVlanMemberForDeletingVlan(UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T vlan_status);
static BOOL_T VLAN_MGR_ProcessAuthPort(UI32_T lport_ifindex, UI32_T pvid, VLAN_OM_VLIST_T *tagged_vlist, VLAN_OM_VLIST_T *untagged_vlist, BOOL_T is_guest);
static BOOL_T VLAN_MGR_ProcessUnauthPort(UI32_T lport_ifindex);
static BOOL_T VLAN_MGR_SetAuthorizedPvid(UI32_T lport_ifindex, UI32_T pvid);
static VLAN_OM_VLIST_T *VLAN_MGR_AddVlanList(VLAN_OM_VLIST_T *vlan_list, UI32_T vid);
static BOOL_T VLAN_MGR_IsVlanListMember(VLAN_OM_VLIST_T *vlan_list, UI32_T vid);
static void VLAN_MGR_FreeVlanList(VLAN_OM_VLIST_T *vlan_list);

/* Since we don't know how to use reserved VLAN, so we just prevent user to use.
 *
 * The approach is add the following APIs with suffix _1. And move the body from
 * its parent function, and just check the reserved range in its parent function.
 */
static BOOL_T VLAN_MGR_CreateVlan_1(UI32_T vid, UI32_T vlan_status);
static BOOL_T VLAN_MGR_SetDot1qVlanStaticEntry_1(VLAN_MGR_Dot1qVlanStaticEntry_T *vlan_entry);

static BOOL_T VLAN_MGR_SetMgmtIpStateForVlan(UI32_T vid, BOOL_T mgmt_state, UI8_T ip_state);

#if (SYS_DFLT_VLAN_CHECK_AT_LEAST_ONE_MEMBER_IN_MANAGEMEMT_VLAN == TRUE)
static UI32_T VLAN_MGR_CountVlanMember(UI8_T *portlist);
#endif /* end of #if (SYS_DFLT_VLAN_CHECK_AT_LEAST_ONE_MEMBER_IN_MANAGEMEMT_VLAN == TRUE) */

/*added by Jinhua Wei ,to remove warning ,becaued the following functions defined but never used*/
#if 0
static UI32_T VLAN_MGR_CreateVlanDev(UI32_T vid, UI8_T mac_addr[SYS_ADPT_MAC_ADDR_LEN]);
static UI32_T VLAN_MGR_DestroyVlanDev(UI32_T vid);
#endif
#if 0 /* DanXie, Tuesday, January 06, 2009 1:56:52 */
static UI32_T VLAN_MGR_SetVlanNetDevLinkStatus(UI32_T vid, BOOL_T link_status);
#endif /* #if 0 */

#if (SYS_CPNT_REFINE_ISC_MSG == TRUE)
static BOOL_T VLAN_MGR_RefineInitDefaultVlanEntry( UI8_T *port_list);
#endif
static BOOL_T VLAN_MGR_SetCorrectTagMemberForDefaultVlan(UI32_T vid);
static UI32_T VLAN_MGR_GetPortMemberType(VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_info_p,
                                         VLAN_OM_Vlan_Port_Info_T *port_info_p);

#if (SYS_CPNT_RSPAN == TRUE)
static BOOL_T VLAN_MGR_DeleteRspanVlanWithoutMembers(UI32_T vid, UI32_T vlan_status);
#endif

static BOOL_T VLAN_MGR_AddVlanMemberToChip(UI32_T lport, UI32_T vid,
                                           BOOL_T untagged, BOOL_T check);
static BOOL_T VLAN_MGR_Dot1vGroupActiveCheck(void *dot1v_group_entry);
static BOOL_T VLAN_MGR_Dot1vPortActiveCheck(void *dot1v_port_entry);

/* STATIC VARIABLE DECLARATIONS
 */
/*
static SYS_TYPE_CallBack_T *VlanCreateList;
static SYS_TYPE_CallBack_T *VlanDestroyList;
static SYS_TYPE_CallBack_T *VlanMemberAddList;
static SYS_TYPE_CallBack_T *VlanMemberDeleteList;
static SYS_TYPE_CallBack_T *IfOperStatusChangedList;
static SYS_TYPE_CallBack_T *AddFirstTrunkMemberList;
static SYS_TYPE_CallBack_T *AddTrunkMemberList;
static SYS_TYPE_CallBack_T *DeleteLastTrunkMemberList;
static SYS_TYPE_CallBack_T *DeleteTrunkMemberList;
static SYS_TYPE_CallBack_T *FinishAddFirstTrunkMemberList;
static SYS_TYPE_CallBack_T *FinishAddTrunkMemberList;
static SYS_TYPE_CallBack_T *FinishDeleteLastTrunkMemberList;
static SYS_TYPE_CallBack_T *FinishDeleteTrunkMemberList;
static SYS_TYPE_CallBack_T *AccessModeMemberList;
static SYS_TYPE_CallBack_T *PvidChangedMemberList;
static SYS_TYPE_CallBack_T *VlanMemberDeleteByTrunkList;
*/

/* Allen Cheng: deleted
static UI32_T   vlan_mgr_operation_mode;
*/

static BOOL_T   vlan_operstatus_changed[SYS_ADPT_MAX_VLAN_ID+1];
static UI32_T   VLAN_MGR_LinkUpDownTrapEnabled;
static BOOL_T   is_provision_complete = FALSE;
static BOOL_T   is_authenticating = FALSE;
static UI32_T   cscgroup_thread_id;
static UI32_T   voice_vlan_id;

/* The table records the precedence using for member addition by each vlan method.
 *  ------------------------------------------------------------
 * |orig\mod| NONE | OTHER | STATIC | GVRP | AUTO | VOICE | MVR |
 *  ------------------------------------------------------------
 * | NONE   |   X  |   X   |   X    |  X   |  X   |   X   |  X  |
 * | OTHER  |   X  |   X   |   X    |  X   |  X   |   X   |  X  |
 * | STATIC |   X  |   X   |   X    |  <   |  >*  |   <   |  <  |
 * | GVRP   |   X  |   X   |   >    |  X   |  >   |   >   |  >  |
 * | AUTO   |   X  |   X   |   <*   |  <   |  X   |   <   |  <  |
 * | VOICE  |   X  |   X   |   >    |  <   |  >   |   X   |  <  |
 * | MVR    |   X  |   X   |   >    |  <   |  >   |   >   |  X  |
 *  ------------------------------------------------------------
 */
static UI32_T   vlan_precedence_table[][VLAN_TYPE_VLAN_STATUS_MVR+1] =
{
    {VLAN_MGR_EQUAL,    VLAN_MGR_EQUAL,  VLAN_MGR_EQUAL,    VLAN_MGR_EQUAL,     VLAN_MGR_EQUAL,     VLAN_MGR_EQUAL,     VLAN_MGR_EQUAL      },
    {VLAN_MGR_EQUAL,    VLAN_MGR_EQUAL,  VLAN_MGR_EQUAL,    VLAN_MGR_EQUAL,     VLAN_MGR_EQUAL,     VLAN_MGR_EQUAL,     VLAN_MGR_EQUAL      },
    {VLAN_MGR_EQUAL,    VLAN_MGR_EQUAL,  VLAN_MGR_EQUAL,    VLAN_MGR_INFERIOR,  VLAN_MGR_ABOVE,     VLAN_MGR_INFERIOR,  VLAN_MGR_INFERIOR   },
    {VLAN_MGR_EQUAL,    VLAN_MGR_EQUAL,  VLAN_MGR_SUPERIOR, VLAN_MGR_EQUAL,     VLAN_MGR_SUPERIOR,  VLAN_MGR_SUPERIOR,  VLAN_MGR_SUPERIOR   },
    {VLAN_MGR_EQUAL,    VLAN_MGR_EQUAL,  VLAN_MGR_BELOW,    VLAN_MGR_INFERIOR,  VLAN_MGR_EQUAL,     VLAN_MGR_INFERIOR,  VLAN_MGR_INFERIOR   },
    {VLAN_MGR_EQUAL,    VLAN_MGR_EQUAL,  VLAN_MGR_SUPERIOR, VLAN_MGR_INFERIOR,  VLAN_MGR_SUPERIOR,  VLAN_MGR_EQUAL,     VLAN_MGR_INFERIOR   },
    {VLAN_MGR_EQUAL,    VLAN_MGR_EQUAL,  VLAN_MGR_SUPERIOR, VLAN_MGR_INFERIOR,  VLAN_MGR_SUPERIOR,  VLAN_MGR_SUPERIOR,  VLAN_MGR_EQUAL      }
};

#if (SYS_CPNT_MAC_VLAN == TRUE)
extern UI8_T    null_mac[];
extern UI8_T    broadcast_mac[];
#endif

SYSFUN_DECLARE_CSC


/* EXPORTED SUBPROGRAM BODIES
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
void VLAN_MGR_Initiate_System_Resources(void)
{
    VLAN_OM_Init();

    /* Allen Cheng: Deleted
    vlan_mgr_operation_mode = SYS_TYPE_STACKING_TRANSITION_MODE;
    */
    memset(vlan_operstatus_changed, 0, sizeof(vlan_operstatus_changed));

    VLAN_BACKDOOR_Init();

    return;
} /* end of VLAN_MGR_Init() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void VLAN_MGR_Create_InterCSC_Relation(void)
{
/*
    SWCTRL_Register_TrunkMemberAdd1st_CallBack( &VLAN_MGR_AddFirstTrunkMember_CallBack );
    SWCTRL_Register_TrunkMemberAdd_CallBack( &VLAN_MGR_AddTrunkMember_CallBack );
    SWCTRL_Register_TrunkMemberDelete_CallBack( &VLAN_MGR_DeleteTrunkMember_CallBack );
    SWCTRL_Register_TrunkMemberDeleteLst_CallBack( &VLAN_MGR_DeleteLastTrunkMember_CallBack );
*/
    VLAN_BACKDOOR_Create_InterCSC_Relation();
    return;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : It is used to enable the vlan operation while in master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void VLAN_MGR_EnterMasterMode(void)
{
    /* Allen Cheng: Deleted
    vlan_mgr_operation_mode = SYS_TYPE_STACKING_MASTER_MODE;
    */
    VLAN_MGR_InitDefaultVlanEntry();
    SYSFUN_ENTER_MASTER_MODE();
    return;
} /* End of VLAN_MGR_EnterMasterMode() */

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
void VLAN_MGR_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE();
    /* Allen Cheng: Deleted
    vlan_mgr_operation_mode = SYS_TYPE_STACKING_TRANSITION_MODE;
    */
    VLAN_OM_ClearDatabase();
    memset(vlan_operstatus_changed, 0, sizeof(vlan_operstatus_changed));

    is_provision_complete = FALSE;
    is_authenticating = FALSE;

#if (SYS_CPNT_MAC_VLAN == TRUE)
    VLAN_OM_ClearMacVlanDatabase();
#endif

} /* End of VLAN_MGR_EnterTransitionMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Disable the vlan operation while in slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------- */
void VLAN_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
    /* Allen Cheng: Deleted
    vlan_mgr_operation_mode = SYS_TYPE_STACKING_SLAVE_MODE;
    */

} /* End of VLAN_MGR_EnterSlaveMode() */

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
void  VLAN_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();

} /* end of VLAN_MGR_SetTransitionMode() */


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
SYS_TYPE_Stacking_Mode_T  VLAN_MGR_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
} /* end of VLAN_MGR_GetOperationMode() */

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
void  VLAN_MGR_ProvisionComplete(void)
{
    is_provision_complete = TRUE;
}

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
void  VLAN_MGR_PreProvisionComplete(void)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;
    UI32_T vid;
    UI8_T  uport_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

    vlan_info.dot1q_vlan_index = 0;

    while (VLAN_OM_GetNextVlanEntry(&vlan_info) == TRUE)
    {
        if (vlan_info.dot1q_vlan_static_row_status != VAL_dot1qVlanStaticRowStatus_active)
            continue;

        VLAN_IFINDEX_CONVERTTO_VID(vlan_info.dot1q_vlan_index, vid);

        SWCTRL_LportListToUportList(vlan_info.dot1q_vlan_current_egress_ports, uport_list);

        if (SWDRV_SetPortToVlanMemberSet(vid, uport_list) == FALSE)
        {
            printf("\r\n%s: call SWDRV_SetPortToVlanMemberSet return false!\r\n", __FUNCTION__);
            return;
        }

        SWCTRL_LportListToUportList(vlan_info.dot1q_vlan_current_untagged_ports, uport_list);

        if (SWDRV_SetPortToVlanUntaggedSet(vid, uport_list) == FALSE)
        {
            printf("\r\n%s: call SWDRV_SetPortToVlanUntaggedSet return false!\r\n", __FUNCTION__);
            return;
        }

        /*EPR: ES3628BT-FLF-ZZ-00380
    Problem:VLAN 1 tagged port send out untag ports after save config and reload
    RootCause:  1 bcm will add all ports to vlan 1 as untaged memeber
                           2 after provision, the vlan1 portbit map in vlan is correct,but it will never notify driver to del the ports
                              in vlan 1 changed from untag to tag
    Solution:    when the vlan is vlan,check if the ports is changed from untaged to taged
    File:vlan_mgr.c,swctrl.c,swctrl.h
*/
       if(VLAN_MGR_DOT1Q_DEFAULT_PVID == vid)
       {
         VLAN_MGR_SetCorrectTagMemberForDefaultVlan(vid);
       }
    }

    return;
}

/*EPR: ES3628BT-FLF-ZZ-00380
    Problem:VLAN 1 tagged port send out untag ports after save config and reload
    RootCause:  1 bcm will add all ports to vlan 1 as untaged memeber
                           2 after provision, the vlan1 portbit map in vlan is correct,but it will never notify driver to del the ports
                              in vlan 1 changed from untag to tag
    Solution:    when the vlan is vlan,check if the ports is changed from untaged to taged
    File:vlan_mgr.c
*/

static BOOL_T VLAN_MGR_SetCorrectTagMemberForDefaultVlan(UI32_T vid)
{
  VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;
  UI32_T vid_ifindex;
  UI32_T                       ifindex = 0;

  VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
  vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;
  if (!VLAN_OM_GetVlanEntry(&vlan_info))
  {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DeleteNormalVlan_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
  }

  while (SWCTRL_GetNextIndexFromPortList(&ifindex, vlan_info.dot1q_vlan_static_egress_ports))
  {
      /* SWCTRL_LPORT_UNKNOWN_PORT
       */
      if(!SWCTRL_IsAvailableConfiguredPort(ifindex))
          continue;

     /*PORT  tagged member,remove it from untagged*/
      if( !VLAN_OM_IsVlanUntagPortListMember(vid_ifindex,ifindex))
      {
           SWCTRL_DeletePortFromVlanUntaggedSet(ifindex, vid,TRUE);
      }
  }
  VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);

}
/* Dot1qBase group
 */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qGvrpStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton returns true if the GVRP status of the bridge can be
 *            successfully Set.  Otherwise, return false.
 * INPUT   :  gvrp_status - VAL_dot1qGvrpStatus_enabled \ VAL_dot1qGvrpStatus_disabled
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
BOOL_T VLAN_MGR_SetDot1qGvrpStatus(UI32_T gvrp_status)
{
    BOOL_T  ret;
    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if ((gvrp_status != VAL_dot1qGvrpStatus_enabled) &&
       (gvrp_status != VAL_dot1qGvrpStatus_disabled))
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    ret = VLAN_OM_SetGlobalGvrpStatus(gvrp_status);
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, ret);

} /* end of VLAN_MGR_SetDot1qGvrpStatus() */

/* Dot1qVlanCurrentTable
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetDot1qConstraintTypeDefault
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the constrain type for this device
 *            can be retrieve successfully.  Otherwise, return false.
 * INPUT    : None
 * OUTPUT   : VAL_dot1qConstraintTypeDefault_independent\
 *            VAL_dot1qConstraintTypeDefault_shared
 * RETURN   : TRUE \ FALSE
 * NOTES    : System_default is IVL.
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_GetDot1qConstraintTypeDefault(UI32_T *constrain_type)
{
    BOOL_T      ret;
    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    ret =  VLAN_OM_GetDot1qConstraintTypeDefault(constrain_type);
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, ret);

} /* end of VLAN_MGR_GetDot1qConstraintTypeDefault() */

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
BOOL_T VLAN_MGR_SetDot1qConstraintTypeDefault(UI32_T constrain_type)
{
    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    /* NOT SUPPORTED */
    EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                VLAN_MGR_SetDot1qConstraintTypeDefault_Fun_No,
                                EH_TYPE_MSG_NOT_SUPPORTED, SYSLOG_LEVEL_INFO,
                                "Dot1q constraint type"
                            );
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
} /* end of VLAN_MGR_SetDot1qConstraintTypeDefault() */


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
BOOL_T VLAN_MGR_CreateVlan(UI32_T vid, UI32_T vlan_status)
{
    BOOL_T  ret;

    if (    vid < 1
        ||  vid > SYS_DFLT_DOT1QMAXVLANID
       )
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_CreateVlan_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    SYSLOG_LEVEL_INFO, "VLAN ID"
                                );
        ret = FALSE;
    }
    else
    {
        ret = VLAN_MGR_CreateVlan_1(vid, vlan_status);
    }

    return ret;
}

static BOOL_T VLAN_MGR_CreateVlan_1(UI32_T vid, UI32_T vlan_status)
{
    UI32_T                                  vid_ifindex, vlan_num;
    VLAN_OM_Dot1qVlanCurrentEntry_T         vlan_info;
    char                                    arg_buf[15];
#if 0
    UI8_T                                   cpu_mac[SYS_ADPT_MAC_ADDR_LEN];
#endif
    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        BACKDOOR_MGR_Printf("VLAN_MGR_CreateVlan_1: vid %d, status %d\n",(UI16_T)vid, (UI16_T)vlan_status);

    /* Default vlan and vid beyond maximum vlan id range
       can not be created.
     */
    if (vid < 1 || vid > SYS_ADPT_MAX_VLAN_ID)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_CreateVlan_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    SYSLOG_LEVEL_INFO, "VLAN ID"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    snprintf(arg_buf, 15, "VLANs (%d)",SYS_ADPT_MAX_NBR_OF_VLAN);
    if ((vlan_num = VLAN_OM_GetDot1qNumVlans()) >= SYS_ADPT_MAX_NBR_OF_VLAN)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_CreateVlan_Fun_No,
                                    EH_TYPE_MSG_MAXIMUM_EXCEEDED,
                                    SYSLOG_LEVEL_INFO, "arg_buf"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    if (VLAN_OM_GetVlanEntry(&vlan_info))
    {
#if (SYS_CPNT_RSPAN == TRUE)
        if (vlan_info.rspan_status == VAL_vlanStaticExtRspanStatus_rspanVlan)
        {
            EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                        VLAN_MGR_CreateVlan_Fun_No,
                                        EH_TYPE_MSG_FAILED_TO_SET,
                                        SYSLOG_LEVEL_INFO,
                                        "vlan status of an existing vlan by RSPAN operation"
                                    );

            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        }
#endif /* #if (SYS_CPNT_RSPAN == TRUE) */

        /* Dynamic GVRP can not alter vlan status of an existing vlan.
         */
        if ((vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_permanent) &&
            (vlan_status == VAL_dot1qVlanStatus_dynamicGvrp))
        {
            EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                        VLAN_MGR_CreateVlan_Fun_No,
                                        EH_TYPE_MSG_FAILED_TO_SET,
                                        SYSLOG_LEVEL_INFO,
                                        "vlan status of an existing vlan by GVRP operation"
                                    );
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        } /* end of if */

        /* Static configuration has higher priority then dynamic configuration.
           Therefor, a dynamic created vlan can be modified and changed to
           static create vlan.
         */
        if ((vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_dynamicGvrp) &&
            (vlan_status == VAL_dot1qVlanStatus_permanent))
        {
            vlan_info.dot1q_vlan_status = VAL_dot1qVlanStatus_permanent;
        } /* end of if */

        SYS_TIME_GetSystemUpTimeByTick(&vlan_info.dot1q_vlan_time_mark);

        if (!VLAN_OM_SetVlanEntry(&vlan_info))
        {
            EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                        VLAN_MGR_CreateVlan_Fun_No,
                                        EH_TYPE_MSG_MAXIMUM_EXCEEDED,
                                        SYSLOG_LEVEL_INFO, arg_buf
                                    );
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        } /* end of if */

        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
    }
    else
    {
        VLAN_MGR_DetailForCreateVlan(vid, vlan_status, &vlan_info);

        /* Clear the vlan port state table */
        vlan_operstatus_changed[vid] = FALSE;

        VLAN_OM_EnterCriticalSection();
        SYS_TIME_GetSystemUpTimeByTick(VLAN_OM_GetIfTableLastUpdateTimePtr());
        VLAN_OM_LeaveCriticalSection();

        /* Pass vlan_info into FSM to check and update dot1q_vlan_static_row_status field.
           If transition of dot1q_vlan_static_row_status is correct, then update vlan_info
           into database.  Otherwise, return false.
         */
        switch (L_RSTATUS_Fsm(VAL_dot1qVlanStaticRowStatus_createAndWait,
                              &vlan_info.dot1q_vlan_static_row_status,
                              VLAN_MGR_SemanticCheck,
                              (void*)&vlan_info))
        {
            /* Update Database ONLY
             */
            case L_RSTATUS_NOTEXIST_2_NOTREADY:
                if(!VLAN_OM_SetVlanEntry(&vlan_info))
                    break;

#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
                if (VLAN_MGR_LocalAddPortTrunkLinkToVlan(vlan_info.dot1q_vlan_index) == FALSE)
                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
#endif
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);
            /* Update Database and Chip
             */
            case L_RSTATUS_NOTEXIST_2_ACTIVE:
                if (!SWCTRL_CreateVlan(vid))
                {
                    VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_CreateVlan_1");
                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                } /* end of if */

                SYS_TIME_GetSystemUpTimeByTick(&vlan_info.dot1q_vlan_time_mark);
#if 0
                /*the vlan interface in net stacking need be created by L3  */
                if (SWCTRL_GetCpuMac(cpu_mac) == FALSE)
                {
                    printf("\r\nSWCTRL_GetCpuMac() returns error!\r\n");
                    return FALSE;
                }

                if (VLAN_MGR_CreateVlanDev(vid, cpu_mac) != VLAN_TYPE_RETVAL_OK)
                {
                    printf("\r\nVLAN_MGR_CreateVlanDev() returns error!\r\n");
                    return FALSE;
                }
#endif
                if(VLAN_OM_SetVlanEntry(&vlan_info))
                {
#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
                    if (VLAN_MGR_LocalAddPortTrunkLinkToVlan(vlan_info.dot1q_vlan_index) == FALSE)
                        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
#endif
                    VLAN_MGR_Notify_VlanCreate(vlan_info.dot1q_vlan_index, vlan_info.dot1q_vlan_status);
                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);
                } /* end of if */
                break;
            default:
                VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, SWITCH_TO_DEFAULT_MESSAGE_INDEX, "VLAN_MGR_CreateVlan_1");
                break;

        } /* end of switch */
    }

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);

} /* end of VLAN_MGR_CreateVlan_1() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_DeleteVlanEx
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan is successfully deleted
 *            from the database.  Otherwise, false is returned.
 * INPUT    : vid   -- the existed vlan id
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : The specific vlan is removed from the database.
 * RETURN   : TRUE \ FALSE
 * NOTES    : No xor checking.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_DeleteVlanEx(UI32_T vid, UI32_T vlan_status)
{
    UI32_T      vid_ifindex, port_num;
    VLAN_OM_Dot1qVlanCurrentEntry_T         vlan_info;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        BACKDOOR_MGR_Printf("VLAN_MGR_DeleteVlan: vid %d, status %d\n",(UI16_T)vid,(UI16_T) vlan_status);

    /* Default vlan and vid beyond maximum vlan id range
       can not be deleted.
     */
    if (vid < 1 || vid > SYS_ADPT_MAX_VLAN_ID || vid == VLAN_OM_GetGlobalDefaultVlan())
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DeleteVlan_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    SYSLOG_LEVEL_INFO, "VLAN ID"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

#if (SYS_DFLT_VLAN_AVOID_DELETING_VOICE_VLAN == TRUE)
    if (vid == voice_vlan_id)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
#endif /* end of #if (SYS_DFLT_VLAN_AVOID_DELETING_VOICE_VLAN == TRUE) */

    /* vid is stored in the corresponding ifindex in the vlan database.
     */
    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

    /* Key to search for specific vlan_info in the database.
     */
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    /* returns error if vlan_info can not be retrieve successfully from the database.
     */
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DeleteVlan_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    /* Management vlan cannot be removed.
     */
    if (    (vlan_info.vlan_ip_state != VLAN_MGR_IP_STATE_NONE)
         && (is_provision_complete == TRUE)
       )
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DeleteVlan_Fun_No,
                                    EH_TYPE_MSG_FAILED_TO_DELETE,
                                    SYSLOG_LEVEL_INFO,
                                    "the management vlan"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    } /* end of if */

    /* Dynamic vlan can not be removed by static configuration
     */
    if ((vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_dynamicGvrp) &&
        (vlan_status == VAL_dot1qVlanStatus_permanent))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DeleteVlan_Fun_No,
                                    EH_TYPE_MSG_FAILED_TO_DELETE,
                                    SYSLOG_LEVEL_INFO,
                                    "dynamic vlan using the static configuration"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    } /* end of if */

    /* Dynamic GVRP can not remove static created vlan */
    if ((vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_permanent) &&
        (vlan_status == VAL_dot1qVlanStatus_dynamicGvrp))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DeleteVlan_Fun_No,
                                    EH_TYPE_MSG_FAILED_TO_DELETE,
                                    SYSLOG_LEVEL_INFO,
                                    "static vlan by GVRP operation"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    } /* end of if */


    switch (L_RSTATUS_Fsm(VAL_dot1qVlanStaticRowStatus_destroy,
                          &vlan_info.dot1q_vlan_static_row_status,
                          VLAN_MGR_SemanticCheck,
                          (void*)&vlan_info))
    {

        /* Remove entry from Database
         */
        case L_RSTATUS_ACTIVE_2_NOTREADY:
            if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                BACKDOOR_MGR_Printf("ERROR: L_RSTATUS_ACTIVE_2_NOTREADY in VLAN_MGR_DeleteVlan\n");

            break;

        /* Delete a suspended vlan
         */
        case L_RSTATUS_NOTREADY_2_NOTEXIST:

            if (!VLAN_MGR_VlanRemovable(vid))
            {
                if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                    BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_VlanRemovable in VLAN_MGR_DeleteVlan for case NotReady->NotExist\n");
                break;
            } /* end of if */

            port_num    = 0;
            while (SWCTRL_GetNextLogicalPort(&port_num) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, port_num))
                {
                    if (!VLAN_MGR_RemoveVlanMemberForDeletingVlan(vlan_info.dot1q_vlan_index,port_num, vlan_status))
                    {
                        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                    }

                    if (!VLAN_MGR_ProcessOrphanPortByLport(port_num))
                    {
                        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                            BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_ProcessOrphanPortByLport in VLAN_MGR_DeleteVlan\n");
                        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
                    }
                }
            } /* end of for */

            if (!VLAN_OM_DeleteVlanEntry(vid))
            {
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
            } /* end of if */
            VLAN_OM_EnterCriticalSection();
            SYS_TIME_GetSystemUpTimeByTick(VLAN_OM_GetIfTableLastUpdateTimePtr());
            VLAN_OM_LeaveCriticalSection();

            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);

            break;

        /* Remove entry from database and ASIC.
         */
        case L_RSTATUS_ACTIVE_2_NOTEXIST:

            if (!VLAN_MGR_VlanRemovable(vid))
            {
                if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                    BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_VlanRemovable in VLAN_MGR_DeleteVlan for case Active->NotExist\n");
                break;
            } /* end of if */

            port_num    = 0;
            while (SWCTRL_GetNextLogicalPort(&port_num) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, port_num))
                {
                    if (!VLAN_MGR_RemoveVlanMemberForDeletingVlan(vlan_info.dot1q_vlan_index,port_num, vlan_status))
                    {
                        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                    }

                    VLAN_MGR_Notify_VlanMemberDelete(vlan_info.dot1q_vlan_index, port_num, vlan_info.dot1q_vlan_status);
                    if (!VLAN_MGR_ProcessOrphanPortByLport(port_num))
                    {
                        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                            BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_ProcessOrphanPortByLport in VLAN_MGR_DeleteEgressPortMember\n");
                        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
                    }
                }
            } /* end of for */

            if (VLAN_MGR_RemoveVlan(vlan_info.dot1q_vlan_index,L_RSTATUS_ACTIVE_2_NOTEXIST))
            {
                VLAN_MGR_Notify_VlanDestroy(&vlan_info, vlan_status);
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);
            } /* end of if */
            else
            {
                if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                    BACKDOOR_MGR_Printf("ERROR: VLAN_OM_DeleteVlanEntry in VLAN_MGR_DeleteVlan for case Active->NotExist\n");
            }
            break;

        default:

            VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_ERR, SWITCH_TO_DEFAULT_MESSAGE_INDEX, "VLAN_MGR_DeleteVlan");
            break;

    } /* end of switch */
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

} /* end of VLAN_MGR_DeleteVlanEx() */


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
BOOL_T VLAN_MGR_DeleteVlan(UI32_T vid, UI32_T vlan_status)
{
    BOOL_T  ret = FALSE;

    VLAN_MGR_XOR_LOCK_AND_FAIL_RETURN(
        ret,
        (TRUE == SYSCTRL_XOR_MGR_PermitBeingDeleteVlan(vid)));

    ret = VLAN_MGR_DeleteVlanEx(vid, vlan_status);

    VLAN_MGR_XOR_UNLOCK_AND_RETURN(ret);
} /* end of VLAN_MGR_DeleteVlan() */


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
BOOL_T VLAN_MGR_DeleteNormalVlan(UI32_T vid, UI32_T vlan_status)
{
#if (SYS_CPNT_RSPAN == TRUE)
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    UI32_T                              vid_ifindex;
#endif


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

#if (SYS_CPNT_RSPAN == TRUE)
    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DeleteNormalVlan_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    if (vlan_info.rspan_status == VAL_vlanStaticExtRspanStatus_rspanVlan )
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DeleteNormalVlan_Fun_No,
                                    EH_TYPE_MSG_FAILED_TO_DELETE,
                                    SYSLOG_LEVEL_INFO,
                                    "the RSPAN vlan using the vlan command."
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
#endif /* end #if (SYS_CPNT_RSPAN == TRUE) */

    if (!VLAN_MGR_DeleteVlan(vid, vlan_status))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    if (vid == VLAN_OM_GetManagementVlanId())
    {
        VLAN_MGR_SetGlobalManagementVlan(SYS_DFLT_SWITCH_MANAGEMENT_VLAN);
    }

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);

} /* end of VLAN_MGR_DeleteNormalVlan() */


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
BOOL_T VLAN_MGR_SetDot1qVlanStaticName(UI32_T vid, char *vlan_name)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    UI32_T                              dot1q_vlan_index;
    BOOL_T                              changed = TRUE;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        BACKDOOR_MGR_Printf("VLAN_MGR_SetDot1qVlanStaticName: vid %d\n",(UI16_T)vid);

    if (vid < 1 || vid > SYS_ADPT_MAX_VLAN_ID)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticName_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN ID"
                                );
        return FALSE;
    }

    /* vid is stored in the corresponding ifindex in the vlan database.
     */
    VLAN_VID_CONVERTTO_IFINDEX(vid, dot1q_vlan_index);

    /* Key to search for specific vlan_info in the database.
     */
    vlan_info.dot1q_vlan_index = (UI16_T)dot1q_vlan_index;

    /* return error if vlan_info can not be retrieve successfully from the database.
     */
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticName_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        return FALSE;
    } /* End of if */

    if (strncmp(vlan_info.dot1q_vlan_static_name, vlan_name, SYS_ADPT_MAX_VLAN_NAME_LEN) == 0)
        changed = FALSE;

    memset(vlan_info.dot1q_vlan_static_name, 0, sizeof(vlan_info.dot1q_vlan_static_name));
    strncpy(vlan_info.dot1q_vlan_static_name, vlan_name, SYS_ADPT_MAX_VLAN_NAME_LEN);

    SYS_TIME_GetSystemUpTimeByTick(&vlan_info.dot1q_vlan_time_mark);

    /* return error if vlan_info can not be set successfully into the database.
     */
    if (!VLAN_OM_SetVlanEntry(&vlan_info))
    {
        char    arg_buf[15];
        snprintf(arg_buf, 15, "VLANs (%d)",SYS_ADPT_MAX_NBR_OF_VLAN);
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticName_Fun_No,
                                    EH_TYPE_MSG_MAXIMUM_EXCEEDED,
                                    SYSLOG_LEVEL_INFO,
                                    arg_buf
                                );
        return FALSE;
    } /* end of if */

    if (changed == TRUE)
    {   /*Notify LLDP, Name of VLAN changes*/
        VLAN_MGR_NOTIFY_LLDP_VLAN_NAME_CHANGED(vid);
    } /* end of if */

    return TRUE;
} /* end of VLAN_MGR_SetDot1qVlanStaticName() */
/*EPR:ES4827G-FLF-ZZ-00232
 *Problem: CLI:size of vlan name different in console and mib
 *Solution: add CLI command "alias" for interface set,the
 *          alias is different from name and port descrition,so
 *          need add new command.
 *modify file: cli_cmd.c,cli_cmd.h,cli_arg.c,cli_arg.h,cli_msg.c,
 *             cli_msg.h,cli_api_vlan.c,cli_api_vlan.h,cli_api_ehternet.c
 *             cli_api_ethernet.h,cli_api_port_channel.c,cli_api_port_channel.h,
 *             cli_running.c,rfc_2863.c,swctrl.h,trk_mgr.h,trk_pmgr.h,swctrl.c
 *             swctrl_pmgr.c,trk_mgr.c,trk_pmgr.c,vlan_mgr.h,vlan_pmgr.h,
 *             vlan_type.h,vlan_mgr.c,vlan_pmgr.c,if_mgr.c
 *Approved by:Hardsun
 *Fixed by:Dan Xie
 */

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
BOOL_T VLAN_MGR_SetDot1qVlanAlias(UI32_T vid, char *vlan_alias)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    UI32_T                              dot1q_vlan_index;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        BACKDOOR_MGR_Printf("VLAN_MGR_SetDot1qVlanStaticName: vid %d\n",(UI16_T)vid);

    if (vid < 1 || vid > SYS_ADPT_MAX_VLAN_ID)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticName_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN ID"
                                );
        return FALSE;
    }

    /* vid is stored in the corresponding ifindex in the vlan database.
     */
    VLAN_VID_CONVERTTO_IFINDEX(vid, dot1q_vlan_index);

    /* Key to search for specific vlan_info in the database.
     */
    vlan_info.dot1q_vlan_index = (UI16_T)dot1q_vlan_index;

    /* return error if vlan_info can not be retrieve successfully from the database.
     */
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticName_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        return FALSE;
    } /* End of if */

    if (strncmp(vlan_info.dot1q_vlan_alias, vlan_alias, MAXSIZE_ifAlias) == 0)
        return TRUE;

    strncpy(vlan_info.dot1q_vlan_alias, vlan_alias, MAXSIZE_ifAlias);
    vlan_info.dot1q_vlan_alias[MAXSIZE_ifAlias] = 0;

    if (!VLAN_OM_SetVlanEntry(&vlan_info))
    {
        char    arg_buf[15];
        snprintf(arg_buf, 15, "VLANs (%d)",SYS_ADPT_MAX_NBR_OF_VLAN);
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticName_Fun_No,
                                    EH_TYPE_MSG_MAXIMUM_EXCEEDED,
                                    SYSLOG_LEVEL_INFO,
                                    arg_buf
                                );
        return FALSE;
    }
    return TRUE;
}
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
BOOL_T VLAN_MGR_AddEgressPortMember(UI32_T vid, UI32_T lport_ifindex, UI32_T status)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    UI32_T                              vid_ifindex;
    BOOL_T                              vlan_member = FALSE;
    BOOL_T                              member_tag_changed;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    /* Returns error if pre-condition to add vlan member can not be satisfy
     */
    if (!VLAN_MGR_PreconditionForSetVlanInfo(vid, lport_ifindex))
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        BACKDOOR_MGR_Printf("VLAN_MGR_AddEgressPortMember: vid %d, port %d, status %d\n",(UI16_T)vid, (UI16_T)lport_ifindex, (UI16_T)status);

    /* vid is stored in the corresponding ifindex in the vlan database.
     */
    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

    /* key to search vlan_info record.
     */
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    /* Do a row create if vlan has never been created in the case of dynamic gvrp
     */
    if (!VLAN_OM_IsVlanExisted(vid))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

#if (SYS_CPNT_RSPAN == TRUE)
    if (vlan_info.rspan_status == VAL_vlanStaticExtRspanStatus_rspanVlan)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "tagged port member by RSPAN operation"
                                );

        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }
#endif

    vlan_member = VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex);

    member_tag_changed = FALSE;
    if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
    {
        member_tag_changed = TRUE;
    }

    if (!VLAN_MGR_AddVlanMember(vlan_info.dot1q_vlan_index,lport_ifindex, VLAN_MGR_TAGGED_ONLY,status))
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_AddVlanMember in VLAN_MGR_AddEgressPortMember\n");
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    VLAN_OM_SetIfStackLastUpdateTime(vlan_info.dot1q_vlan_time_mark);

    /* Dynamic vlan information will not be save to startup file. Therefore,
       join command from user configuration should also change vlan status from
       dynamic to static to solve the issue.  Otherwise, inconsistency will occur
       when the device reboot.
     */
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
    {
        /* Need to generate callback when port does not previously existed in vlan membership.
         */
        if (    (!vlan_member)
            &&  VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex)
           )
        {
            VLAN_MGR_Notify_VlanMemberAdd(vid_ifindex, lport_ifindex, status);
        }

        if (member_tag_changed == TRUE)
        {
            VLAN_MGR_Notify_VlanMemberTagChanged(vid_ifindex, lport_ifindex);
        }
    } /* End of if (_vlan_active_) */

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
    {
        VLAN_OM_Vlan_Port_Info_T            vlan_port_info;

        if (!VLAN_MGR_GetPortEntry(lport_ifindex, &vlan_port_info))
        {
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
        }

        if (    (vlan_port_info.vlan_port_entry.vlan_port_dual_mode)
            &&  (vid == vlan_port_info.vlan_port_entry.dual_mode_vlan_id)
           )
        {
            if (!VLAN_MGR_DisablePortDualMode(lport_ifindex))
            {
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
            } /* end of if */
        }
    }

    if(!VLAN_MGR_ConformToSingleMode(vid, lport_ifindex, status, TRUE))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    } /* end of if */
#endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)*/

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);

} /* end of VLAN_MGR_AddEgressPortMember() */


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
BOOL_T VLAN_MGR_AddEgressPortMemberForGVRP(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    UI32_T                              vid_ifindex;
    BOOL_T                              vlan_member = FALSE;
    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    /* Returns error if pre-condition to add vlan member can not be satisfy
     */
    if (!VLAN_MGR_PreconditionForSetVlanInfo(vid, lport_ifindex))
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        BACKDOOR_MGR_Printf("VLAN_MGR_AddEgressPortMember: vid %d, port %d, status %d\n",(UI16_T)vid, (UI16_T)lport_ifindex, (UI16_T)vlan_status);

    /* vid is stored in the corresponding ifindex in the vlan database.
     */
    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

    /* key to search vlan_info record.
     */
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    /* Do a row create if vlan has never been created in the case of dynamic gvrp
     */
    if (!VLAN_OM_IsVlanExisted(vid))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    vlan_member = VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex);

    if (!VLAN_MGR_AddVlanMember(vlan_info.dot1q_vlan_index,lport_ifindex, VLAN_MGR_TAGGED_ONLY,vlan_status))
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_AddVlanMember in VLAN_MGR_AddEgressPortMemberForGVRP\n");
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    VLAN_OM_SetIfStackLastUpdateTime(vlan_info.dot1q_vlan_time_mark);

    /* Dynamic vlan information will not be save to startup file. Therefore,
       join command from user configuration should also change vlan status from
       dynamic to static to solve the issue.  Otherwise, inconsistency will occur
       when the device reboot.
     */
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
    {
        /* Need to generate callback when port does not previously existed in vlan membership.
         */
        if (    (!vlan_member)
            &&  VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex)
           )
        {
            VLAN_MGR_Notify_VlanMemberAdd(vid_ifindex,lport_ifindex, VAL_dot1qVlanStatus_dynamicGvrp);
        }
    } /* End of if (_vlan_active_) */

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
    if(!VLAN_MGR_ConformToSingleMode(vid, lport_ifindex, vlan_status, TRUE))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    } /* end of if */
#endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)*/

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);

} /* end of VLAN_MGR_AddEgressPortMemberForGVRP() */

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
BOOL_T VLAN_MGR_DeleteEgressPortMember(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status)
{
    UI32_T          vid_ifindex;
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    BOOL_T                              is_member;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (!VLAN_MGR_PreconditionForSetVlanInfo(vid, lport_ifindex))
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        BACKDOOR_MGR_Printf("VLAN_MGR_DeleteEgressPortMember: vid %d, port %d, status %d\n",(UI16_T)vid, (UI16_T)lport_ifindex, (UI16_T)vlan_status);

    /* vid is stored in the corresponding ifindex in the vlan database.
     */
    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

    /* vid_ifindx is the key to search for vlan_info in the database
     */
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    /* returns false if vlan_info can not be retrieve from the database
     */
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DeleteEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* End of if */

    is_member = VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex);
    if (    (!is_member)
        &&  (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, lport_ifindex))
       )
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
    }

    /* Dynamic vlan information will not be save to startup file. Therefore,
       join command from user configuration should also change vlan status from
       dynamic to static to solve the issue.  Otherwise, inconsistency will occur
       when the device reboot.
     */
    if ((vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_dynamicGvrp) &&
       (vlan_status == VAL_dot1qVlanStatus_permanent))
    {
        vlan_info.dot1q_vlan_status = VAL_dot1qVlanStatus_permanent;
    } /* end of if */

#if (SYS_DFLT_VLAN_CHECK_AT_LEAST_ONE_MEMBER_IN_MANAGEMEMT_VLAN == TRUE)
    /* The last port member of management VLAN can not be removed. */
    if (    (vid == VLAN_OM_GetManagementVlanId())
         && (vlan_status == VAL_dot1qVlanStatus_permanent)
         && (VLAN_MGR_CountVlanMember(vlan_info.dot1q_vlan_static_egress_ports) == 1)
       )
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_VLAN,
                                 VLAN_MGR_DeleteEgressPortMember_Fun_No,
                                 EH_TYPE_MSG_FAILED_TO_DELETE,
                                 SYSLOG_LEVEL_INFO,
                                 "the last port member of management VLAN"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }
#endif /* end of #if (SYS_DFLT_VLAN_CHECK_AT_LEAST_ONE_MEMBER_IN_MANAGEMEMT_VLAN == TRUE) */

    /* Allow user to set tag/untag for a private_port */
    /* If vlan_status == VLAN_TYPE_VLAN_STATUS_AUTO, don't need to check VLAN_MGR_PortRemovable() */
    if (    VLAN_MGR_IsPrivateVlanPortMember(vid_ifindex, lport_ifindex)
         || (vlan_status == VLAN_TYPE_VLAN_STATUS_AUTO)
         || VLAN_MGR_PortRemovable(vid_ifindex, lport_ifindex)
       )
    {
        if (!VLAN_MGR_RemoveVlanMember(vlan_info.dot1q_vlan_index,lport_ifindex,VLAN_MGR_BOTH,L_RSTATUS_ACTIVE_2_NOTEXIST,vlan_status))
        {
            if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_RemoveVlanMember in VLAN_MGR_DeleteEgressPortMember\n");
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        } /* end of if */

    } /* end of if */
    else
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    VLAN_OM_SetIfStackLastUpdateTime(vlan_info.dot1q_vlan_time_mark);

    /* Refresh vlan_info since VLAN_MGR_RemoveVlanMember() may change vlan_operation_status.
     */
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DeleteEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* End of if */

    if (    (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
        &&  (is_member)
        &&  (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex))
       )
    {
        VLAN_MGR_Notify_VlanMemberDelete(vid_ifindex,lport_ifindex, vlan_status);
    }

#if (SYS_CPNT_VLAN_PROVIDING_AT_LEAST_ONE_UNTAGGED_MODE == TRUE)
    if (VLAN_MGR_ConformToAtLeastOneUntaggedVlan(lport_ifindex) == FALSE)
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
#endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_AT_LEAST_ONE_UNTAGGED_MODE == TRUE) */

    if (!VLAN_MGR_ProcessOrphanPortByLport(lport_ifindex))
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_ProcessOrphanPortByLport in VLAN_MGR_DeleteEgressPortMember\n");
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);
} /* End of VLAN_MGR_DeleteEgressPortMember() */


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
 *            3. WEB \ SNMP API
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetDot1qVlanStaticEgressPorts(UI32_T vid, UI8_T *portlist, UI32_T vlan_status)
{
    UI32_T      vid_ifindex;
    UI32_T      byte, port_num;
    UI8_T       compare_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    BOOL_T                              is_member;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (!VLAN_OM_IsVlanExisted(vid))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticEgressPorts_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    memcpy(&compare_list, portlist, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticEgressPorts_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

#if (SYS_CPNT_RSPAN == TRUE)
    if (vlan_info.rspan_status == VAL_vlanStaticExtRspanStatus_rspanVlan)
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }
#endif

    /* Dynamic vlan information will not be save to startup file. Therefore,
       join command from user configuration should also change vlan status from
       dynamic to static to solve the issue.  Otherwise, inconsistency will occur
       when the device reboot.
     */
    if ((vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_dynamicGvrp) &&
       (vlan_status == VAL_dot1qVlanStatus_permanent))
    {
        vlan_info.dot1q_vlan_status = VAL_dot1qVlanStatus_permanent;
    } /* end of if */

#if (SYS_DFLT_VLAN_CHECK_AT_LEAST_ONE_MEMBER_IN_MANAGEMEMT_VLAN == TRUE)
    /* The last port member of management VLAN can not be removed. */
    if (    (vid == VLAN_OM_GetManagementVlanId())
         && (VLAN_MGR_CountVlanMember(portlist) == 1)
       )
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_VLAN,
                                 VLAN_MGR_DeleteEgressPortMember_Fun_No,
                                 EH_TYPE_MSG_FAILED_TO_DELETE,
                                 SYSLOG_LEVEL_INFO,
                                 "the last port member of management VLAN"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }
#endif /* end of #if (SYS_DFLT_VLAN_CHECK_AT_LEAST_ONE_MEMBER_IN_MANAGEMEMT_VLAN == TRUE) */

    for (byte=0; byte < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; byte++)
        compare_list[byte] ^= vlan_info.dot1q_vlan_static_egress_ports[byte];

    port_num    = 0;
    while (SWCTRL_GetNextLogicalPort(&port_num) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        if (VLAN_OM_IS_MEMBER(compare_list, port_num))
        {
            if (!VLAN_MGR_PreconditionForSetVlanInfo(vid, port_num))
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

            /* Remove from Egress port list.
             */
            if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, port_num))
            {
                is_member = VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, port_num);
                /* Needs to guarantee pvid of the specific port is not set to
                   current vlan.  Otherwise, return FALSE.
                 */
                if (VLAN_MGR_PortRemovable(vid_ifindex, port_num))
                {
                    if (!VLAN_MGR_RemoveVlanMember(vlan_info.dot1q_vlan_index,port_num,VLAN_MGR_BOTH,L_RSTATUS_ACTIVE_2_NOTEXIST,vlan_status))
                    {
                        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                    } /* end of if */

                    if (!VLAN_OM_GetVlanEntry(&vlan_info))
                    {
                        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                                    VLAN_MGR_SetDot1qVlanStaticEntry_Fun_No,
                                                    EH_TYPE_MSG_NOT_EXIST,
                                                    SYSLOG_LEVEL_INFO,
                                                    "VLAN entry"
                                                );
                        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                    } /* end of if */

                    if (    (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
                        &&  (is_member)
                        &&  (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, port_num))
                       )
                    {
                        VLAN_MGR_Notify_VlanMemberDelete(vid_ifindex,port_num, vlan_status);
                    }

#if (SYS_CPNT_VLAN_PROVIDING_AT_LEAST_ONE_UNTAGGED_MODE == TRUE)
                    if (VLAN_MGR_ConformToAtLeastOneUntaggedVlan(port_num) == FALSE)
                    {
                        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
                    }
#endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_AT_LEAST_ONE_UNTAGGED_MODE == TRUE) */

                    if (!VLAN_MGR_ProcessOrphanPortByLport(port_num))
                    {
                        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
                    }
                } /* end of if */
                else
                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
            }
            /* Add to Egress port list.
             */
            else
            {
                if (!VLAN_MGR_AddVlanMember(vlan_info.dot1q_vlan_index,port_num, VLAN_MGR_TAGGED_ONLY, VAL_dot1qVlanStatus_permanent))
                {
                    if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                        BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_AddVlanMember in VLAN_MGR_SetDot1qVlanStaticEgressPorts\n");

                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                } /* end of if */

                if (!VLAN_OM_GetVlanEntry(&vlan_info))
                {
                    EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                                VLAN_MGR_SetDot1qVlanStaticEntry_Fun_No,
                                                EH_TYPE_MSG_NOT_EXIST,
                                                SYSLOG_LEVEL_INFO,
                                                "VLAN entry"
                                            );
                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                } /* end of if */

                if (    (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
                    &&  (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, port_num))
                   )
                {
                    VLAN_MGR_Notify_VlanMemberAdd(vid_ifindex,port_num, vlan_status);
                }

                #if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
                {
                    if (! VLAN_MGR_ConformToSingleMode(vid, port_num, vlan_status, TRUE))
                    {
                        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
                    } /* end of if */
                }
                #endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)*/
            } /* end of else */

        } /* end of if */
    } /* end of for */

    VLAN_OM_EnterCriticalSection();
    SYS_TIME_GetSystemUpTimeByTick(VLAN_OM_GetIfStackLastUpdateTimePtr());
    VLAN_OM_LeaveCriticalSection();
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);

} /* end of VLAN_MGR_SetDot1qVlanStaticEgressPorts() */


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

BOOL_T VLAN_MGR_AddForbiddenEgressPortMember(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    UI32_T                              vid_ifindex;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    /* Returns error if pre-condition to add vlan member can not be satisfy
     */
    if (!VLAN_MGR_PreconditionForSetVlanInfo(vid, lport_ifindex))
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        BACKDOOR_MGR_Printf("VLAN_MGR_AddForbiddenEgressPortMember: vid %d, port %d, status %d\n",(UI16_T)vid, (UI16_T)lport_ifindex,(UI16_T) vlan_status);

    /* vid is stored in the corresponding ifindex in the vlan database.
     */
    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

    vlan_info.dot1q_vlan_index = (UI16_T) vid_ifindex;
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddForbiddenEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    vlan_port_info.lport_ifindex = lport_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddForbiddenEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    if (vlan_port_info.port_item.auto_vlan_mode)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddForbiddenEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_FAILED_TO_ADD,
                                    SYSLOG_LEVEL_INFO,
                                    "the port because it is authorized"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    if (vlan_port_info.port_trunk_mode)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddForbiddenEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_FAILED_TO_ADD,
                                    SYSLOG_LEVEL_INFO,
                                    "the port because it is trunk member"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* End of if */

    if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, lport_ifindex))
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);

    if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex))
    {
        if (VLAN_MGR_PortRemovable(vid_ifindex, lport_ifindex))
        {
            /* If this port is dynamically added to the vlan, change this port to static
             * port so that it can be removed from this vlan
             */
            vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;
            if (!VLAN_OM_GetVlanEntry(&vlan_info))
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_AddForbiddenEgressPortMember_Fun_No,
                                            EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO,
                                            "VLAN entry"
                                        );
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
            } /* end of if */

            if (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, lport_ifindex))
            {
                if (!VLAN_MGR_AddVlanMember(vid_ifindex, lport_ifindex, VLAN_MGR_TAGGED_ONLY, vlan_status))
                {
                    if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                        BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_AddVlanMember in VLAN_MGR_AddForbiddenEgressPortMember\n");
                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                } /* end of if */
            } /* end of if */

#if (SYS_DFLT_VLAN_CHECK_AT_LEAST_ONE_MEMBER_IN_MANAGEMEMT_VLAN == TRUE)
            /* The last port member of management VLAN can not be removed. */
            if (    (vid == VLAN_OM_GetManagementVlanId())
                 && (VLAN_MGR_CountVlanMember(vlan_info.dot1q_vlan_static_egress_ports) == 1)
               )
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_VLAN,
                                         VLAN_MGR_DeleteEgressPortMember_Fun_No,
                                         EH_TYPE_MSG_FAILED_TO_DELETE,
                                         SYSLOG_LEVEL_INFO,
                                         "the last port member of management VLAN"
                                        );
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
            } /* end of if */
#endif /* end of #if (SYS_DFLT_VLAN_CHECK_AT_LEAST_ONE_MEMBER_IN_MANAGEMEMT_VLAN == TRUE) */

            if (!VLAN_MGR_RemoveVlanMember(vid_ifindex,lport_ifindex,VLAN_MGR_BOTH,L_RSTATUS_ACTIVE_2_NOTEXIST,vlan_status))
            {
                if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                    BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_RemoveVlanMember in VLAN_MGR_AddForbiddenEgressPortMember\n");
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
            } /* end of if */

            if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
                VLAN_MGR_Notify_VlanMemberDelete(vid_ifindex,lport_ifindex, vlan_status);
        } /* end of if */
        else
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    } /* end of if */

    /* returns false if vlan_info can not be retrieve from the database
     */
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddForbiddenEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* End of if */

    VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, lport_ifindex);
    SYS_TIME_GetSystemUpTimeByTick(&vlan_info.dot1q_vlan_time_mark);

    if (!VLAN_MGR_SetVlanRowStatus(L_RSTATUS_SET_OTHER_COLUMN, &vlan_info))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);

} /* end of VLAN_MGR_AddForbiddenEgressPortMember()*/


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
BOOL_T VLAN_MGR_DeleteForbiddenEgressPortMember(UI32_T vid, UI32_T lport_ifindex,UI32_T vlan_status)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    UI32_T                              vid_ifindex;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    /* Returns error if pre-condition to add vlan member can not be satisfy
     */
    if (!VLAN_MGR_PreconditionForSetVlanInfo(vid, lport_ifindex))
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        BACKDOOR_MGR_Printf("VLAN_MGR_DeleteForbiddenEgressPortMember: vid %d, port %d, status %d\n",(UI16_T)vid, (UI16_T)lport_ifindex, (UI16_T)vlan_status);

    /* vid is stored in the corresponding ifindex in the vlan database.
     */
    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

    /* vid_ifindx is the key to search for vlan_info in the database
     */
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    /* returns false if vlan_info can not be retrieve from the database
     */
    if(!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DeleteForbiddenEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* End of if */

    if (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, lport_ifindex))
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);

    /* Dynamic vlan information will not be save to startup file. Therefore,
       any vlan command from user configuration should also change vlan status from
       dynamic to static to solve the issue.  Otherwise, inconsistency will occur
       when the device reboot.
     */
    if (vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_dynamicGvrp)
    {
        vlan_info.dot1q_vlan_status = VAL_dot1qVlanStatus_permanent;

    } /* end of if */

    /* If
       1. lport_ifindex does not exist in vlan_info's forbiddenlist
       2. condition for Static entry can only be modified by management and
          dynamically entry can only be modified dynamically does not meet
       THEN return FALSE;
     */
    if (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, lport_ifindex))
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);

    VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, lport_ifindex);
    SYS_TIME_GetSystemUpTimeByTick(&vlan_info.dot1q_vlan_time_mark);

    if (!VLAN_MGR_SetVlanRowStatus(L_RSTATUS_SET_OTHER_COLUMN, &vlan_info))
    {
        char    arg_buf[15];
        snprintf(arg_buf, 15, "VLANs (%d)",SYS_ADPT_MAX_NBR_OF_VLAN);
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DeleteForbiddenEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_MAXIMUM_EXCEEDED,
                                    SYSLOG_LEVEL_INFO,
                                    arg_buf
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);

} /* end of VLAN_MGR_DeleteForbiddenEgressPortMember()*/


/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qVlanForbiddenEgressPorts
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if forbidden port list of the specific vlan
 *            is succesfully updated. Otherwise, return false.
 * INPUT    : vid        -- the vlan id
 *            portlist - a port list that contains the ports that are to be or not o be
 *                       forbidden from joining the specific vlan.
 *            vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : none.
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. A port may not be added in this set if it is already a member of
 *               the set of ports in the EgressPortlist
 *            2. lport_ifindex will not be permitted to join vlan's member list
 *               until it is removed from Forbidden_port list.
 *            3. WEB \ SNMP API
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetDot1qVlanForbiddenEgressPorts(UI32_T vid, UI8_T *portlist, UI32_T vlan_status)
{

    UI32_T      vid_ifindex, port_num, byte;
    UI8_T       compare_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    char                                arg_buf[15];
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    snprintf(arg_buf, 15, "VLANs (%d)",SYS_ADPT_MAX_NBR_OF_VLAN);

    memcpy(&compare_list, portlist, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanForbiddenEgressPorts_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    /* Dynamic vlan information will not be save to startup file. Therefore,
       any vlan command from user configuration should also change vlan status from
       dynamic to static to solve the issue.  Otherwise, inconsistency will occur
       when the device reboot.
     */

    for (byte=0; byte < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; byte++)
        compare_list[byte] ^= vlan_info.dot1q_vlan_forbidden_egress_ports[byte];

    port_num    = 0;
    while (SWCTRL_GetNextLogicalPort(&port_num) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        if (VLAN_OM_IS_MEMBER(compare_list, port_num))
        {
           if (!VLAN_MGR_PreconditionForSetVlanInfo(vid, port_num))
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

            /* Delete forbidden portlist member
             */
            if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, port_num))
            {
                if (!VLAN_OM_GetVlanEntry(&vlan_info))
                {
                    EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                                VLAN_MGR_SetDot1qVlanForbiddenEgressPorts_Fun_No,
                                                EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO,
                                                "VLAN entry"
                                            );
                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                } /* end of if */

                VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, port_num);
                if (!VLAN_OM_SetVlanEntry(&vlan_info))
                {
                    EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                                VLAN_MGR_SetDot1qVlanForbiddenEgressPorts_Fun_No,
                                                EH_TYPE_MSG_MAXIMUM_EXCEEDED,
                                                SYSLOG_LEVEL_INFO,
                                                arg_buf
                                            );
                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                } /* end of if */
            }
            else
            {
                /* authenticated port should not be add to forbidden list,
                 * because it will violate the purpose of VLAN authorization
                 */
                vlan_port_info.lport_ifindex = port_num;
                if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
                {
                    EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                                VLAN_MGR_SetDot1qVlanForbiddenEgressPorts_Fun_No,
                                                EH_TYPE_MSG_NOT_EXIST,
                                                SYSLOG_LEVEL_INFO,
                                                "VLAN port entry"
                                            );
                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                }

                if (vlan_port_info.port_item.auto_vlan_mode)
                {
                    EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                                VLAN_MGR_SetDot1qVlanForbiddenEgressPorts_Fun_No,
                                                EH_TYPE_MSG_FAILED_TO_ADD,
                                                SYSLOG_LEVEL_INFO,
                                                "the port because it is authorized"
                                            );
                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                }

                if (vlan_port_info.port_trunk_mode)
                {
                    EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                                VLAN_MGR_SetDot1qVlanForbiddenEgressPorts_Fun_No,
                                                EH_TYPE_MSG_FAILED_TO_ADD,
                                                SYSLOG_LEVEL_INFO,
                                                "the port because it is trunk member"
                                            );
                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                } /* End of if */

               /* If the specific port previous existed in vlan member list, the proper procedure
                   needs to be perform to remove it from vlan member list before it can join
                   forbidden portlist.
                 */
                if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, port_num))
                {
                    if (VLAN_MGR_PortRemovable(vid_ifindex, port_num))
                    {
                        /* If this port is dynamically added to the vlan, change this port to static
                           port so that it can be removed from this vlan
                         */
                        vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;
                        if (!VLAN_OM_GetVlanEntry(&vlan_info))
                        {
                            EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                                        VLAN_MGR_AddForbiddenEgressPortMember_Fun_No,
                                                        EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO,
                                                        "VLAN entry"
                                                    );
                            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                        } /* End of if */

                        if (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, port_num))
                        {
                            if (!VLAN_MGR_AddVlanMember(vlan_info.dot1q_vlan_index, port_num, VLAN_MGR_TAGGED_ONLY, vlan_status))
                            {
                                if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                                    BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_AddVlanMember in VLAN_MGR_SetDot1qVlanForbiddenEgressPorts\n");
                                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                            }
                        } /* end of if */

                        if (!VLAN_MGR_RemoveVlanMember(vlan_info.dot1q_vlan_index,port_num,VLAN_MGR_BOTH,L_RSTATUS_ACTIVE_2_NOTEXIST, vlan_status))
                        {
                            if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                                BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_RemoveVlanMember in VLAN_MGR_SetDot1qVlanForbiddenEgressPorts\n");

                            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                        } /* end of if */

                        if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
                            VLAN_MGR_Notify_VlanMemberDelete(vid_ifindex,port_num, vlan_status);
                    }
                    else
                        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
                } /* end of if */

                if (!VLAN_OM_GetVlanEntry(&vlan_info))
                {
                    EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                                VLAN_MGR_SetDot1qVlanForbiddenEgressPorts_Fun_No,
                                                EH_TYPE_MSG_NOT_EXIST,
                                                SYSLOG_LEVEL_INFO,
                                                "VLAN entry"
                                            );
                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                } /* end of if */

                VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, port_num);
                if (!VLAN_OM_SetVlanEntry(&vlan_info))
                {
                    EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                                VLAN_MGR_SetDot1qVlanForbiddenEgressPorts_Fun_No,
                                                EH_TYPE_MSG_MAXIMUM_EXCEEDED,
                                                SYSLOG_LEVEL_INFO,
                                                arg_buf
                                            );
                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                } /* end of if */
            } /* end of else */
        } /* end of if */
    } /* end of for */

    vlan_info.dot1q_vlan_index = (UI16_T) vid_ifindex;
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanForbiddenEgressPorts_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);

} /* end of VLAN_MGR_SetDot1qVlanForbiddenEgressPorts() */


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
BOOL_T VLAN_MGR_AddUntagPortMember(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    UI32_T      vid_ifindex;
    BOOL_T      vlan_member = FALSE;
    BOOL_T      member_tag_changed;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    /* Returns error if pre-condition to add vlan member can not be satisfy
     */
    if (!VLAN_MGR_PreconditionForSetVlanInfo(vid, lport_ifindex))
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (vlan_status == VAL_dot1qVlanStatus_dynamicGvrp)
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("ERROR: AddeUntagPortMember: vid %d, port %d, status %d\n",(UI16_T)vid,(UI16_T) lport_ifindex,(UI16_T) vlan_status);
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddUntagPortMember_Fun_No,
                                    EH_TYPE_MSG_FAILED_TO_ADD,
                                    SYSLOG_LEVEL_INFO,
                                    "untag port member by GVRP operation"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    } /* end of if */

    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddUntagPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

#if (SYS_CPNT_RSPAN == TRUE)
    if (vlan_info.rspan_status == VAL_vlanStaticExtRspanStatus_rspanVlan)
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            printf("ERROR: AddeUntagPortMember: vid %d, port %d, status %d\n",(UI16_T)vid,(UI16_T) lport_ifindex,(UI16_T) vlan_status);

        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddUntagPortMember_Fun_No,
                                    EH_TYPE_MSG_FAILED_TO_ADD,
                                    SYSLOG_LEVEL_INFO,
                                    "untag port member by RSPAN operation"
                                );

        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }
#endif

    vlan_port_info.lport_ifindex = lport_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
    if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
    {
        vlan_port_info.vlan_port_entry.vlan_port_dual_mode = FALSE;
        vlan_port_info.vlan_port_entry.dual_mode_vlan_id    = 0;

        if (!VLAN_OM_SetVlanPortEntry(&vlan_port_info))
        {
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        }
    }
#endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)*/

    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        BACKDOOR_MGR_Printf("VLAN_MGR_AddUntagPortMember: vid %d, port %d, status %d\n",(UI16_T)vid,(UI16_T) lport_ifindex, (UI16_T)vlan_status);

    vlan_member = VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex);

    member_tag_changed = FALSE;
    if (    (vlan_member == TRUE)
         && !VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex)
       )
    {
        member_tag_changed = TRUE;
    }

    if (!VLAN_MGR_AddVlanMember(vlan_info.dot1q_vlan_index,lport_ifindex, VLAN_MGR_BOTH, vlan_status))
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_AddVlanMember in VLAN_MGR_AddUntagPortMember\n");
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    VLAN_OM_SetIfStackLastUpdateTime(vlan_info.dot1q_vlan_time_mark);

    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddUntagPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
    {
        /* Need to generate callback when port does not previously existed in vlan membership.
         */
        if (    (!vlan_member)
            &&  VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex)
           )
        {
            VLAN_MGR_Notify_VlanMemberAdd(vid_ifindex, lport_ifindex, vlan_status);
        }

        if (member_tag_changed == TRUE)
        {
            VLAN_MGR_Notify_VlanMemberTagChanged(vid_ifindex, lport_ifindex);
        }
    } /* End of if (_vlan_active_) */

#if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE)
    if (    (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_access)
         && (!VLAN_MGR_ConformToSingleUntaggedVlan(vid, lport_ifindex, vlan_status))
       )
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
#endif /* #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE) */

#if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK == TRUE)
    if (    (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_hybrid)
         && (!VLAN_MGR_ConformToSingleUntaggedVlan(vid, lport_ifindex, vlan_status))
       )
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
#endif /* #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK == TRUE) */

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
    if (!VLAN_MGR_ConformToSingleMode(vid, lport_ifindex, vlan_status, FALSE))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
#endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE) */

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);
} /* End of VLAN_MGR_AddUntagPortMember() */


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
BOOL_T VLAN_MGR_DeleteUntagPortMember(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    UI32_T      vid_ifindex;
    BOOL_T      member_tag_changed;

    /* BODY */

    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        BACKDOOR_MGR_Printf("VLAN_MGR_DeleteUntagPortMember: vid %d, port %d, status %d\n",(UI16_T)vid, (UI16_T)lport_ifindex, (UI16_T)vlan_status);

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    /* Returns error if pre-condition to add vlan member can not be satisfy
     */
    if (!VLAN_MGR_PreconditionForSetVlanInfo(vid, lport_ifindex))
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (vlan_status == VAL_dot1qVlanStatus_dynamicGvrp)
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("ERROR: DeleteUntagPortMember: vid %d, port %d, status %d\n",(UI16_T)vid,(UI16_T) lport_ifindex, (UI16_T)vlan_status);
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DeleteUntagPortMember_Fun_No,
                                    EH_TYPE_MSG_FAILED_TO_DELETE,
                                    SYSLOG_LEVEL_INFO,
                                    "untag port member by GVRP operation"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    } /* end of if */

    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

    /* Allow user to set tag/untag for a private_port */
    if (    (!VLAN_MGR_IsPrivateVlanPortMember(vid_ifindex, lport_ifindex))
         && (!VLAN_MGR_PortRemovable(vid_ifindex, lport_ifindex))
       )
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    } /* end of if */

    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddUntagPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

#if (SYS_CPNT_RSPAN == TRUE)
    if (vlan_info.rspan_status == VAL_vlanStaticExtRspanStatus_rspanVlan)
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            printf("ERROR: VLAN_MGR_DeleteUntagPortMember: vid %d, port %d, status %d\n",(UI16_T)vid,(UI16_T) lport_ifindex, (UI16_T)vlan_status);

        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DeleteUntagPortMember_Fun_No,
                                    EH_TYPE_MSG_FAILED_TO_DELETE,
                                    SYSLOG_LEVEL_INFO,
                                    "untag port member by RSPAN operation"
                                );

        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }
#endif

    member_tag_changed = FALSE;
    if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
    {
        member_tag_changed = TRUE;
    }

    if (!VLAN_MGR_RemoveVlanMember(vid_ifindex,lport_ifindex,VLAN_MGR_UNTAGGED_ONLY,L_RSTATUS_ACTIVE_2_NOTEXIST,vlan_status))
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_RemoveVlanMember in VLAN_MGR_DeleteUntagPortMember\n");
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    VLAN_OM_EnterCriticalSection();
    SYS_TIME_GetSystemUpTimeByTick(VLAN_OM_GetIfStackLastUpdateTimePtr());
    VLAN_OM_LeaveCriticalSection();

    if (    (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
         && (member_tag_changed == TRUE)
       )
    {
        VLAN_MGR_Notify_VlanMemberTagChanged(vid_ifindex, lport_ifindex);
    }

#if (SYS_CPNT_VLAN_PROVIDING_AT_LEAST_ONE_UNTAGGED_MODE == TRUE)
    if (VLAN_MGR_ConformToAtLeastOneUntaggedVlan(lport_ifindex) == FALSE)
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
#endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_AT_LEAST_ONE_UNTAGGED_MODE == TRUE) */

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);
} /* end of VLAN_MGR_DeleteUntagPortMember() */


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
 * NOTES    : 1. lport_ifindex must exist in vlan's egress_port_list before it can join
 *               the untag_pot_list
 *            3. WEB \ SNMP API
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetDot1qVlanStaticUntaggedPorts(UI32_T vid, UI8_T *portlist, UI32_T vlan_status)
{
    UI32_T    port_num, vid_ifindex, byte;
    UI8_T     compare_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    BOOL_T    result = TRUE;
    BOOL_T    is_vlan_member;
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;

     /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (vlan_status == VAL_dot1qVlanStatus_dynamicGvrp)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticUntaggedPorts_Fun_No,
                                    EH_TYPE_MSG_FAILED_TO,
                                    SYSLOG_LEVEL_INFO,
                                    "update untagged vlan membership by GVRP operation"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    memcpy(&compare_list, portlist, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticUntaggedPorts_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

#if (SYS_CPNT_RSPAN == TRUE)
    if (vlan_info.rspan_status == VAL_vlanStaticExtRspanStatus_rspanVlan)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticUntaggedPorts_Fun_No,
                                    EH_TYPE_MSG_FAILED_TO,
                                    SYSLOG_LEVEL_INFO,
                                    "update untagged vlan membership by RSPAN operation"
                                );

        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }
#endif /* #if (SYS_CPNT_RSPAN == TRUE) */

    /* Dynamic vlan information will not be save to startup file. Therefore,
       vlan command from user configuration should also change vlan status from
       dynamic to static to solve the issue.  Otherwise, inconsistency will occur
       when the device reboot.
     */
    if (vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_dynamicGvrp)
    {
        vlan_info.dot1q_vlan_status = VAL_dot1qVlanStatus_permanent;

    } /* end of if */

    for (byte=0; byte < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; byte++)
        compare_list[byte] ^= vlan_info.dot1q_vlan_static_untagged_ports[byte];

    port_num    = 0;
    while (SWCTRL_GetNextLogicalPort(&port_num) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        if (VLAN_OM_IS_MEMBER(compare_list, port_num))
        {
           if (!VLAN_MGR_PreconditionForSetVlanInfo(vid, port_num))
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

            /* Remove from untagged member set
             */
            if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, port_num))
            {
                /* Allow user to set tag/untag for a private_port */
                if (    VLAN_MGR_IsPrivateVlanPortMember(vid_ifindex, port_num)
                     || VLAN_MGR_PortRemovable(vid_ifindex, port_num)
                   )
                {
                    if (!VLAN_MGR_RemoveVlanMember(vlan_info.dot1q_vlan_index,port_num,VLAN_MGR_UNTAGGED_ONLY,L_RSTATUS_ACTIVE_2_NOTEXIST, vlan_status))
                    {
                        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                    } /* end of if */

                    VLAN_MGR_Notify_VlanMemberTagChanged(vid_ifindex, port_num);

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
                    if (!VLAN_MGR_ConformToSingleMode(vid, port_num, vlan_status, TRUE))
                    {
                        break;
                    }
#endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)*/

#if (SYS_CPNT_VLAN_PROVIDING_AT_LEAST_ONE_UNTAGGED_MODE == TRUE)
                    if (VLAN_MGR_ConformToAtLeastOneUntaggedVlan(port_num) == FALSE)
                    {
                        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
                    }
#endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_AT_LEAST_ONE_UNTAGGED_MODE == TRUE) */
                }
                else
                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
            }
            else
            {
                is_vlan_member = VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, port_num);
                if (!VLAN_MGR_AddVlanMember(vlan_info.dot1q_vlan_index,port_num, VLAN_MGR_BOTH, vlan_status))
                {
                    if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                        BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_AddVlanMember in VLAN_MGR_AddUntagPortMember\n");
                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                } /* end of if */
                if (!VLAN_OM_GetVlanEntry(&vlan_info))
                {
                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                }

                vlan_port_info.lport_ifindex = port_num;
                if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
                {
                    EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                                VLAN_MGR_AddVlanMember_Fun_No,
                                                EH_TYPE_MSG_NOT_EXIST,
                                                SYSLOG_LEVEL_INFO,
                                                "VLAN port entry"
                                            );
                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                } /* end of if */

                if (is_vlan_member == FALSE)
                {
                    VLAN_MGR_Notify_VlanMemberAdd(vid_ifindex, port_num, vlan_status);
                }
                else
                {
                    VLAN_MGR_Notify_VlanMemberTagChanged(vid_ifindex, port_num);
                }

#if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE)
                if (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_access)
                {
                    result = VLAN_MGR_ConformToSingleUntaggedVlan(vid, port_num, vlan_status);
                }
#endif /* #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE) */

#if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK == TRUE)
                if (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_hybrid)
                {
                    result = VLAN_MGR_ConformToSingleUntaggedVlan(vid, port_num, vlan_status);
                }
#endif /* #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK == TRUE) */

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
                if (result)
                {
                    result = VLAN_MGR_ConformToSingleMode(vid, port_num, vlan_status, FALSE);
                }
#endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)*/

                if (!result)
                {
                    break;
                }
            } /* end of else */
        } /* end of if */
    } /* end of while */

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, result);
} /* End of VLAN_MGR_SetDot1qVlanStaticUntaggedPorts() */

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
BOOL_T VLAN_MGR_SetDot1qVlanStaticRowStatusForGVRP(UI32_T vid, UI32_T row_status)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (vid < 1 || vid > SYS_ADPT_MAX_VLAN_ID)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticRowStatusForGVRP_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN ID"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
    if ((row_status < VAL_dot1qVlanStaticRowStatus_active) ||
        (row_status > VAL_dot1qVlanStaticRowStatus_destroy))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticRowStatusForGVRP_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    SYSLOG_LEVEL_INFO,
                                    "row status"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    if (VLAN_MGR_LocalSetVlanRowStatus(vid, row_status, VAL_dot1qVlanStatus_dynamicGvrp) == TRUE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE)
    else
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

} /* end of VLAN_MGR_SetDot1qVlanStaticRowStatusForGVRP() */


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
BOOL_T VLAN_MGR_SetDot1qVlanStaticRowStatus(UI32_T vid, UI32_T row_status)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (vid < 1 || vid > SYS_ADPT_MAX_VLAN_ID)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticRowStatus_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN ID"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
    if ((row_status < VAL_dot1qVlanStaticRowStatus_active) ||
        (row_status > VAL_dot1qVlanStaticRowStatus_destroy))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticRowStatus_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    SYSLOG_LEVEL_INFO,
                                    "row status"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

#if(SYS_CPNT_ADD == TRUE)
    else if( row_status == VAL_dot1qVlanStaticRowStatus_notInService )
    {
        if(voice_vlan_id == vid)
        {
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
        }
    }
#endif

    if (VLAN_MGR_LocalSetVlanRowStatus(vid, row_status, VAL_dot1qVlanStatus_permanent) == TRUE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE)
    else
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

} /* end of VLAN_MGR_SetDot1qVlanStaticRowStatus() */


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
BOOL_T VLAN_MGR_SetVlanAddressMethod(UI32_T vid, UI32_T address_method)
{
    UI32_T      vid_ifindex;
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (vid < 1 || vid > SYS_ADPT_MAX_VLAN_ID)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetVlanAddressMethod_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN ID"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    if ((address_method != VAL_vlanAddressMethod_user) &&
       (address_method != VAL_vlanAddressMethod_bootp) &&
       (address_method != VAL_vlanAddressMethod_dhcp))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetVlanAddressMethod_Fun_No,
                                    EH_TYPE_MSG_INVALID,
                                    SYSLOG_LEVEL_INFO,
                                    "address method"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetVlanAddressMethod_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* End of if */

    /* Address method for dynamic vlan can not be set because
       vlan created by GVRP will be remove once the timer expire and
       the dynamic vlan no longer contains any members.  Therefore,
       ip address method for dynamic vlan should remain no accessible.
     */
    if (vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_dynamicGvrp)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetVlanAddressMethod_Fun_No,
                                    EH_TYPE_MSG_FAILED_TO_SET,
                                    SYSLOG_LEVEL_INFO,
                                    "address method for dynamic vlan"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */


    vlan_info.vlan_address_method = address_method;
    SYS_TIME_GetSystemUpTimeByTick(&vlan_info.dot1q_vlan_time_mark);

    if (!VLAN_OM_SetVlanEntry(&vlan_info))
    {
        char    arg_buf[15];
        snprintf(arg_buf, 15, "VLANs (%d)",SYS_ADPT_MAX_NBR_OF_VLAN);
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetVlanAddressMethod_Fun_No,
                                    EH_TYPE_MSG_MAXIMUM_EXCEEDED,
                                    SYSLOG_LEVEL_INFO,
                                    arg_buf
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* End of if */

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);

} /* end of VLAN_MGR_SetVlanAddressMethod() */


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
BOOL_T VLAN_MGR_SetDot1qVlanStaticEntry(VLAN_MGR_Dot1qVlanStaticEntry_T *vlan_entry)
{
    BOOL_T  ret = FALSE;

#if ((SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE) || (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK == TRUE))
    UI32_T                              vlan_ifindex = 0, lport = 0;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
#endif

    if (    (vlan_entry->dot1q_vlan_index < 1)
         || (vlan_entry->dot1q_vlan_index > SYS_DFLT_DOT1QMAXVLANID)
       )
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticEntry_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    SYSLOG_LEVEL_INFO, "VLAN ID"
                                );
        ret = FALSE;
    }
    else
    {
        ret = VLAN_MGR_SetDot1qVlanStaticEntry_1(vlan_entry);

#if ((SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE) || (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK == TRUE))
        VLAN_VID_CONVERTTO_IFINDEX(vlan_entry->dot1q_vlan_index, vlan_ifindex);

        while (    (ret == TRUE)
                && (VLAN_OM_GetNextVlanMember(vlan_ifindex, &lport, VLAN_OM_VlanMemberType_CurrentUntagged))
              )
        {
            vlan_port_info.lport_ifindex = lport;
            if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_AddVlanMember_Fun_No,
                                            EH_TYPE_MSG_NOT_EXIST,
                                            SYSLOG_LEVEL_INFO,
                                            "VLAN port entry"
                                        );
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
            } /* end of if */

    #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE)
            if (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_access)
            {
                ret = ret && VLAN_MGR_ConformToSingleUntaggedVlan(vlan_entry->dot1q_vlan_index, lport, VAL_dot1qVlanStatus_permanent);
            }
    #endif /* end of #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE) */

    #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK == TRUE)
            if (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_hybrid)
            {
                ret = ret && VLAN_MGR_ConformToSingleUntaggedVlan(vlan_entry->dot1q_vlan_index, lport, VAL_dot1qVlanStatus_permanent);
            }
    #endif /* end of #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK == TRUE) */
        } /* end of while */
#endif /* end of #if ((SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE) || (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK == TRUE)) */
    }

    return ret;
} /* End of VLAN_MGR_SetDot1qVlanStaticEntry() */

static BOOL_T VLAN_MGR_SetDot1qVlanStaticEntry_1(VLAN_MGR_Dot1qVlanStaticEntry_T *vlan_entry)
{
    UI32_T                                  vlan_num;
    VLAN_OM_Dot1qVlanCurrentEntry_T         vlan_info;
    UI32_T                                  member_option, port_num;
    VLAN_OM_Vlan_Port_Info_T                vlan_port_info;
    #if 0
    UI8_T                               cpu_mac[SYS_ADPT_MAC_ADDR_LEN];
    #endif
    /*added by Jinhua Wei ,to remove warning ,becaued above array never used*/


    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        BACKDOOR_MGR_Printf("VLAN_MGR_SetDot1qVlanStaticEntry: vid %d, status %d\n",(UI16_T)vlan_entry->dot1q_vlan_index, (UI16_T)VAL_dot1qVlanStatus_permanent);

    /* Default vlan and vid beyond maximum vlan id range
       can not be created.
     */
    if ((vlan_entry->dot1q_vlan_index < 1) || (vlan_entry->dot1q_vlan_index > SYS_ADPT_MAX_VLAN_ID))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticEntry_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN ID"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
    if ((vlan_num = VLAN_OM_GetDot1qNumVlans()) >= SYS_ADPT_MAX_NBR_OF_VLAN)
    {
        char    arg_buf[15];
        snprintf(arg_buf, 15, "VLANs (%d)",SYS_ADPT_MAX_NBR_OF_VLAN);
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticEntry_Fun_No,
                                    EH_TYPE_MSG_MAXIMUM_EXCEEDED,
                                    SYSLOG_LEVEL_INFO,
                                    arg_buf
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
    if (VLAN_OM_IsVlanExisted(vlan_entry->dot1q_vlan_index))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticEntry_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    memset(&vlan_info, 0, sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));

    VLAN_MGR_DetailForCreateVlan(vlan_entry->dot1q_vlan_index, VAL_dot1qVlanStatus_permanent, &vlan_info);

    VLAN_OM_EnterCriticalSection();
    SYS_TIME_GetSystemUpTimeByTick(VLAN_OM_GetIfTableLastUpdateTimePtr());
    VLAN_OM_LeaveCriticalSection();

    strcpy(vlan_info.dot1q_vlan_static_name, vlan_entry->dot1q_vlan_static_name);

    if (!VLAN_MGR_ValidateVlanFields(vlan_entry))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    /* Pass vlan_info into FSM to check and update dot1q_vlan_static_row_status field.
       If transition of dot1q_vlan_static_row_status is correct, then update vlan_info
       into database.  Otherwise, return false.
     */
    switch (L_RSTATUS_Fsm(vlan_entry->dot1q_vlan_static_row_status,
                          &vlan_info.dot1q_vlan_static_row_status,
                          VLAN_MGR_SemanticCheck,
                          (void*)&vlan_info))
    {
        /* Update Database ONLY
         */
        case L_RSTATUS_NOTEXIST_2_NOTREADY:
            memcpy(vlan_info.dot1q_vlan_current_egress_ports, vlan_entry->dot1q_vlan_static_egress_ports, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
            memcpy(vlan_info.dot1q_vlan_static_egress_ports, vlan_entry->dot1q_vlan_static_egress_ports, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
            memcpy(vlan_info.dot1q_vlan_current_untagged_ports, vlan_entry->dot1q_vlan_static_untagged_ports, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
            memcpy(vlan_info.dot1q_vlan_static_untagged_ports, vlan_entry->dot1q_vlan_static_untagged_ports, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
            memcpy(vlan_info.dot1q_vlan_forbidden_egress_ports, vlan_entry->dot1q_vlan_forbidden_egress_ports, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

            if(!VLAN_OM_SetVlanEntry(&vlan_info))
                break;

            port_num    = 0;
            while (SWCTRL_GetNextLogicalPort(&port_num) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                if (VLAN_OM_IS_MEMBER(vlan_entry->dot1q_vlan_static_egress_ports, port_num))
                {
                    vlan_port_info.lport_ifindex = port_num;
                    VLAN_OM_GetVlanPortEntry(&vlan_port_info);
                    vlan_port_info.port_item.static_joined_vlan_count++;
                    VLAN_OM_SetVlanPortEntry(&vlan_port_info);
                }
            }

#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
            if (VLAN_MGR_LocalAddPortTrunkLinkToVlan(vlan_info.dot1q_vlan_index) == FALSE)
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
#endif

            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);

        /* Update Database and Chip
         */
        case L_RSTATUS_NOTEXIST_2_ACTIVE:
            if (!SWCTRL_CreateVlan(vlan_entry->dot1q_vlan_index))
            {
                VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING,FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_SetDot1qVlanStaticEntry");
                break;
            } /* end of if */

#if 0 /* VaiWang, Thursday, March 27, 2008 10:11:22 */
            if (SWCTRL_GetCpuMac(cpu_mac) == FALSE)
            {
                printf("\r\nSWCTRL_GetCpuMac() returns error!\r\n");
                break;
            }
            if (VLAN_MGR_CreateVlanDev(vlan_entry->dot1q_vlan_index, cpu_mac) != VLAN_TYPE_RETVAL_OK)
            {
                printf("\r\nVLAN_MGR_CreateVlanDev() returns error!\r\n");
                break;
            }
#endif /*  ACCTON_METRO */

            memcpy(vlan_info.dot1q_vlan_forbidden_egress_ports, vlan_entry->dot1q_vlan_forbidden_egress_ports, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

            if(!VLAN_OM_SetVlanEntry(&vlan_info))
            {
                break;
            } /* end of if */

            VLAN_MGR_Notify_VlanCreate(vlan_info.dot1q_vlan_index, vlan_info.dot1q_vlan_status);

            port_num    = 0;
            while (SWCTRL_GetNextLogicalPort(&port_num) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                if (VLAN_OM_IS_MEMBER(vlan_entry->dot1q_vlan_static_egress_ports, port_num))
                {
                    if (VLAN_OM_IS_MEMBER(vlan_entry->dot1q_vlan_static_untagged_ports, port_num))
                        member_option = VLAN_MGR_BOTH;
                    else
                        member_option = VLAN_MGR_TAGGED_ONLY;

                    if (!VLAN_MGR_AddVlanMember(vlan_info.dot1q_vlan_index,port_num, member_option, VAL_dot1qVlanStatus_permanent))
                    {
                        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                            BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_AddVlanMember in VLAN_MGR_SetDot1qVlanStaticEntry\n");
                        break;
                    } /* end of if */

                    if (!VLAN_OM_GetVlanEntry(&vlan_info))
                    {
                        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                                    VLAN_MGR_SetDot1qVlanStaticEntry_Fun_No,
                                                    EH_TYPE_MSG_NOT_EXIST,
                                                    SYSLOG_LEVEL_INFO,
                                                    "VLAN entry"
                                                );
                        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                    } /* end of if */

                    if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, port_num))
                    {
                        VLAN_MGR_Notify_VlanMemberAdd(vlan_info.dot1q_vlan_index, port_num, VAL_dot1qVlanStatus_permanent);
                    }
                } /* end of if */
            } /* end of for */

#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
            if (VLAN_MGR_LocalAddPortTrunkLinkToVlan(vlan_info.dot1q_vlan_index) == FALSE)
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
#endif

            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);

        default:
            VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, SWITCH_TO_DEFAULT_MESSAGE_INDEX, "VLAN_MGR_SetDot1qVlanStaticEntry");
            break;

    } /* end of switch */
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);

} /* end of VLAN_MGR_SetDot1qVlanStaticEntry_1() */


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
BOOL_T VLAN_MGR_SetVlanAdminStatus(UI32_T vid, UI32_T admin_status)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                VLAN_MGR_SetVlanAdminStatus_Fun_No,
                                EH_TYPE_MSG_NOT_SUPPORTED,
                                SYSLOG_LEVEL_INFO,
                                "Vlan admin status"
                            );
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

} /* end of VLAN_MGR_SetVlanAdminStatus() */


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
BOOL_T VLAN_MGR_SetLinkUpDownTrapEnabled(UI32_T vid, UI32_T trap_status)
{
    UI32_T      vid_ifindex;
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if ((trap_status != VAL_ifLinkUpDownTrapEnable_enabled) &&
       (trap_status != VAL_ifLinkUpDownTrapEnable_disabled))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetLinkUpDownTrapEnabled_Fun_No,
                                    EH_TYPE_MSG_INVALID,
                                    SYSLOG_LEVEL_INFO,
                                    "trap status"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
    if (vid < 1 || vid > SYS_ADPT_MAX_VLAN_ID)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetLinkUpDownTrapEnabled_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN ID"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetLinkUpDownTrapEnabled_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* End of if */

    vlan_info.if_entry.link_up_down_trap_enabled = trap_status;

    VLAN_MGR_LinkUpDownTrapEnabled  = trap_status;

    SYS_TIME_GetSystemUpTimeByTick(&vlan_info.dot1q_vlan_time_mark);

    if (!VLAN_OM_SetVlanEntry(&vlan_info))
    {
        char    arg_buf[15];
        snprintf(arg_buf, 15, "VLANs (%d)",SYS_ADPT_MAX_NBR_OF_VLAN);
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetLinkUpDownTrapEnabled_Fun_No,
                                    EH_TYPE_MSG_MAXIMUM_EXCEEDED,
                                    SYSLOG_LEVEL_INFO,
                                    arg_buf
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* End of if */

    VLAN_OM_SetIfStackLastUpdateTime(vlan_info.dot1q_vlan_time_mark);

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);

} /* end of VLAN_MGR_SetLinkUpDownTrapEnabled() */

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
BOOL_T VLAN_MGR_SetDot1qPvid(UI32_T lport_ifindex, UI32_T pvid)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    UI32_T                              pvid_ifindex;
    SWCTRL_Lport_Type_T                 port_type;
    UI32_T                              unit, port, trunk_id, old_pvid;
#if 0
    BOOL_T                              notify_vlan_member_add;
#endif
    BOOL_T                              notify_pvid_change;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

#if 0
    notify_vlan_member_add = FALSE;
#endif
    notify_pvid_change = FALSE;

    /* Error if pre-condition to set port info can not be satisfied.
     */
    if (!VLAN_MGR_PreconditionForSetVlanPortInfo(lport_ifindex, PVID_FIELD, pvid))
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if ((port_type = SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit, &port, &trunk_id)) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qPvid_Fun_No,
                                    EH_TYPE_MSG_IS_INVALID,
                                    SYSLOG_LEVEL_INFO,
                                    "Port"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    vlan_port_info.lport_ifindex = lport_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qPvid_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);

    } /* end of if */

    VLAN_IFINDEX_CONVERTTO_VID(vlan_port_info.port_item.dot1q_pvid_index,old_pvid);

    if (old_pvid == pvid)
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);

    } /* end of if */

    /* Error if the specific vlan entry can not be retrieve from the database.
     */
    VLAN_VID_CONVERTTO_IFINDEX(pvid, pvid_ifindex);
    vlan_info.dot1q_vlan_index = (UI16_T)pvid_ifindex;
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
#if (SYS_CPNT_VLAN_AUTO_CREATE_VLAN_FOR_PVID == TRUE)
        if (VLAN_MGR_CreateVlan_1(pvid, VAL_dot1qVlanStatus_permanent) == FALSE)
        {
            return FALSE;
        }
        if (VLAN_MGR_LocalSetVlanRowStatus(pvid,
            VAL_dot1qVlanStaticRowStatus_active, VAL_dot1qVlanStatus_permanent)
            == FALSE)
        {
            return FALSE;
        }
        if (VLAN_OM_GetVlanEntry(&vlan_info) == FALSE)
        {
            return FALSE;
        }
#else
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qPvid_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
#endif /* #if (SYS_CPNT_VLAN_AUTO_CREATE_VLAN_FOR_PVID == TRUE) */
    } /* end of if */

#if (SYS_CPNT_RSPAN == TRUE)
    if (vlan_info.rspan_status == VAL_vlanStaticExtRspanStatus_rspanVlan)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qPvid_Fun_No,
                                    EH_TYPE_MSG_IS_INVALID,
                                    SYSLOG_LEVEL_INFO,
                                    "RSPAN VLAN"
                                );

        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }
#endif

#if 0
    /* All port will automatically join default vlan if pvid is to be set back to default.
     */
    if (pvid == VLAN_OM_GetGlobalDefaultVlan())
    {
        if (vlan_port_info.port_item.auto_vlan_mode && !is_authenticating)
        {
            if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, lport_ifindex))
                VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, lport_ifindex);

            if (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, lport_ifindex))
            {
                VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, lport_ifindex);
                vlan_port_info.port_item.static_joined_vlan_count++;
            }
            VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, lport_ifindex);

            if (!VLAN_OM_SetVlanEntry(&vlan_info))
            {
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
            } /* end of if */

            vlan_port_info.port_item.admin_pvid = pvid;
            if (!VLAN_OM_SetVlanPortEntry(&vlan_port_info))
            {
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
            } /* end of if */

            if (port_type == SWCTRL_LPORT_TRUNK_PORT)
            {
                VLAN_MGR_SetTrunkMemberPortInfo(lport_ifindex, ADMIN_PVID_FIELD, pvid);
            }

            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
        }
        else
        {
            /* Check if need to notify other registered components */
            if (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex))
            {
                notify_vlan_member_add = TRUE;
            }

            if (!VLAN_MGR_SetDefaultPortMembership(pvid, lport_ifindex))
            {
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
            } /* end of if */

            if(notify_vlan_member_add)
            {
                VLAN_MGR_Notify_VlanMemberAdd(pvid_ifindex, lport_ifindex, VAL_dot1qVlanStatus_permanent);
            }

            VLAN_MGR_Notify_PvidChange(lport_ifindex, old_pvid, pvid);

#if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE)
            if (    (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_access)
                 && (!VLAN_MGR_ConformToSingleUntaggedVlan(pvid, lport_ifindex, VAL_dot1qVlanStatus_permanent))
               )
            {
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
            }
#endif /* end of #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE) */

#if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK == TRUE)
            if (    (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_hybrid)
                 && (!VLAN_MGR_ConformToSingleUntaggedVlan(pvid, lport_ifindex, VAL_dot1qVlanStatus_permanent))
               )
            {
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
            }
#endif /* end of #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK == TRUE) */

            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);
        }
    } /* end of if */
#endif /* #if 0 */

    /* the port must have been tagged member of pvid VLAN for trunk mode,
     * untagged member of pvid VLAN for access/hybrid mode; otherwise,
     * pvid can not be assigned
     */
    if (vlan_port_info.port_item.auto_vlan_mode && !is_authenticating)
    {
        if (    (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_access)
             && (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, lport_ifindex))
           )
        {
            EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                        VLAN_MGR_SetDot1qPvid_Fun_No,
                                        EH_TYPE_MSG_FAILED_TO_SET,
                                        SYSLOG_LEVEL_INFO,
                                        "the native vlan because this port does not exist in the vlan's untagged list"
                                    );
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        }
#if 0
        if (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_dot1qTrunk)
        {
            if (    (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, lport_ifindex))
                 || (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, lport_ifindex))
               )
            {
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
            }
        }
        else
        {
            if (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, lport_ifindex))
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_SetDot1qPvid_Fun_No,
                                            EH_TYPE_MSG_FAILED_TO_SET,
                                            SYSLOG_LEVEL_INFO,
                                            "the native vlan because this port does not exist in the vlan's untagged list"
                                        );
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
            }
        }
#endif
    }
    else
    {
        if (    (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_access)
             && (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
           )
        {
            EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                        VLAN_MGR_SetDot1qPvid_Fun_No,
                                        EH_TYPE_MSG_FAILED_TO_SET,
                                        SYSLOG_LEVEL_INFO,
                                        "the native vlan because this port does not exist in the vlan's untagged list"
                                    );
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        }
#if 0
        if (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_dot1qTrunk)
        {
            if (    (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex))
                 || (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
               )
            {
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
            }
        }
        else
        {
            if (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_SetDot1qPvid_Fun_No,
                                            EH_TYPE_MSG_FAILED_TO_SET,
                                            SYSLOG_LEVEL_INFO,
                                            "the native vlan because this port does not exist in the vlan's untagged list"
                                        );
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
            }
        }
#endif

        if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
        {
            if (!SWCTRL_SetPortPVID(lport_ifindex, pvid, TRUE))
            {
                VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_SetDot1qPvid");
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
            }
            notify_pvid_change = TRUE;
        }
        else if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_notReady)
        {
            /* If port's native VLAN is suspended, reset to default VLAN 1 in chip. */
            if (!SWCTRL_SetPortPVID(lport_ifindex, VLAN_OM_GetGlobalDefaultVlan(),TRUE))
            {
                VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_SetDot1qPvid");
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
            }
            notify_pvid_change = TRUE;
        }

        if (port_type == SWCTRL_LPORT_TRUNK_PORT)
        {
            VLAN_MGR_SetTrunkMemberPortInfo(lport_ifindex,PVID_FIELD, pvid_ifindex);
        }

        vlan_port_info.port_item.dot1q_pvid_index = pvid_ifindex;
    }

    VLAN_MGR_SetTrunkMemberPortInfo(lport_ifindex, ADMIN_PVID_FIELD, pvid);
    vlan_port_info.port_item.admin_pvid = pvid;
    if (VLAN_OM_SetVlanPortEntry(&vlan_port_info))
    {
        if (vlan_info.dot1q_vlan_static_row_status != VAL_dot1qVlanStaticRowStatus_active)
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);

        if (notify_pvid_change)
            VLAN_MGR_Notify_PvidChange(lport_ifindex, old_pvid, pvid);

        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);
    }
    else
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }
} /* End of VLAN_MGR_SetDot1qPvid() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qPortAcceptableFrameTypes
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if port acceptable frame type is set
 *            successfully.  Otherwise, return false.
 * INPUT    : lport_ifindex       -- the port number
 *            acceptable_frame_types - VAL_dot1qPortAcceptableFrameTypes_admitAll \
 *                                     VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanTagged
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : The default value is VAL_dot1qPortAcceptableFrameTypes_admitAll.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetDot1qPortAcceptableFrameTypes(UI32_T lport_ifindex, UI32_T acceptable_frame_types)
{
    BOOL_T  result;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    result = VLAN_MGR_SetDot1qPortAcceptableFrameTypes_(lport_ifindex, acceptable_frame_types);
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, result);

} /* End of VLAN_MGR_SetDot1qPortAcceptableFrameTypes() */


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
BOOL_T VLAN_MGR_SetDot1qIngressFilter(UI32_T lport_ifindex, UI32_T ingress_filtering)
{
    VLAN_OM_Vlan_Port_Info_T        vlan_port_info;
    SWCTRL_Lport_Type_T             port_type;
    UI32_T                          unit, port, trunk_id;
#if (SYS_ADPT_INGRESS_FILTER_NOT_SUPPORT_PER_PORT == 1)
    UI32_T                          port_num = 0;
#endif

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    /* Error if pre-condition to set port info can not be satisfied.
     */
    if (!VLAN_MGR_PreconditionForSetVlanPortInfo(lport_ifindex, INGRESS_FIELD, ingress_filtering))
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if ((port_type = SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit, &port, &trunk_id)) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qIngressFilter_Fun_No,
                                    EH_TYPE_MSG_IS_INVALID,
                                    SYSLOG_LEVEL_INFO,
                                    "Port"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    vlan_port_info.lport_ifindex = lport_ifindex;
    if(!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qIngressFilter_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    if (!vlan_port_info.port_item.auto_vlan_mode ||  is_authenticating)
    {
        if ((ingress_filtering == VAL_dot1qPortIngressFiltering_true) &&
            (!SWCTRL_EnableIngressFilter(lport_ifindex)))
        {
            VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING,
                                         FUNCTION_RETURN_FAIL_INDEX,
                                         "VLAN_MGR_SetPortIngressFilter");
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        }

        if ((ingress_filtering == VAL_dot1qPortIngressFiltering_false) &&
            (!SWCTRL_DisableIngressFilter(lport_ifindex)))
        {
            VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING,
                                         FUNCTION_RETURN_FAIL_INDEX,
                                         "VLAN_MGR_SetPortIngressFilter");
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        } /* end of if */

        vlan_port_info.port_item.dot1q_port_ingress_filtering = ingress_filtering;

        if (port_type == SWCTRL_LPORT_TRUNK_PORT)
        {
            VLAN_MGR_SetTrunkMemberPortInfo(lport_ifindex,INGRESS_FIELD, ingress_filtering);
        }
    }

    if (!is_authenticating)
    {
        if (port_type == SWCTRL_LPORT_TRUNK_PORT)
        {
            VLAN_MGR_SetTrunkMemberPortInfo(lport_ifindex, ADMIN_INGRESS_FIELD, ingress_filtering);
        }
        vlan_port_info.port_item.admin_ingress_filtering = ingress_filtering;
    }

    if (!VLAN_OM_SetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticName_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                    "lport (1-SYS_ADPT_TOTAL_NBR_OF_LPORT"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

#if (SYS_ADPT_INGRESS_FILTER_NOT_SUPPORT_PER_PORT == 1)
    /* If ASIC does not support per port ingress filtering, this command
       requires vlan to update each port entry's ingress filtering status for each
       configuration.
     */
    while ((port_type = SWCTRL_GetNextLogicalPort(&port_num)) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        if (portport_num == lport_ifindex)
        {
            continue;
        }
        vlan_port_info.lport_ifindex = port_num;
        if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
        {
            EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                        VLAN_MGR_SetDot1qIngressFilter_Fun_No,
                                        EH_TYPE_MSG_NOT_EXIST,
                                        SYSLOG_LEVEL_INFO,
                                        "VLAN port entry"
                                    );
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        }
        vlan_port_info.port_item.dot1q_port_ingress_filtering = ingress_filtering;
        if (!VLAN_OM_SetVlanPortEntry(&vlan_port_info))
        {
            EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                        VLAN_MGR_SetDot1qVlanStaticName_Fun_No,
                                        EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                        (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                        "lport (1-SYS_ADPT_TOTAL_NBR_OF_LPORT"
                                    );
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        }
        if (port_type == SWCTRL_LPORT_TRUNK_PORT)
            VLAN_MGR_SetTrunkMemberPortInfo(port_num,INGRESS_FIELD, ingress_filtering);

    } /* end of while */

#endif
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);

} /* End of VLAN_MGR_SetPortIngressFilter() */


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
BOOL_T VLAN_MGR_SetVlanPortMode(UI32_T lport_ifindex, UI32_T vlan_port_mode)
{
    UI32_T                              unit, port, trunk_id;
    BOOL_T                              ret = FALSE;
    SWCTRL_Lport_Type_T                 port_type;
    VLAN_OM_Vlan_Port_Info_T            port_info;
#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
    UI32_T                              vid;
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
#endif /* end of #if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE) */
    VLAN_OM_Dot1qVlanCurrentEntry_T     pvid_vlan_info;
    BOOL_T                              is_new_member_added;
    BOOL_T                              is_member_tag_changed;

    /* BODY
     */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    /* This function only returns TRUE if the specific port is a normal port
     * or Trunk port.  Trunk_member port attribute can not be modify individually.
     */
    if (!VLAN_MGR_PreconditionForSetVlanPortInfo(lport_ifindex, VLAN_PORT_MODE_FIELD, vlan_port_mode))
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if ((port_type = SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit, &port, &trunk_id)) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetVlanPortMode_Fun_No,
                                    EH_TYPE_MSG_IS_INVALID,
                                    SYSLOG_LEVEL_INFO,
                                    "Port"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    } /* end of if */

    port_info.lport_ifindex = lport_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetVlanPortMode_Fun_No,
                                    EH_TYPE_MSG_IS_INVALID,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    } /* end of if */

    if (port_info.vlan_port_entry.vlan_port_mode == vlan_port_mode)
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);
    } /* end of if port mode not change */

    /* authenticated port should not change vlan port mode, because it will
     * violate the purpose of VLAN authorization
     */
    if (port_info.port_item.auto_vlan_mode)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetVlanPortMode_Fun_No,
                                    EH_TYPE_MSG_FAILED_TO_ADD,
                                    SYSLOG_LEVEL_INFO,
                                    "the port because it is authorized"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    } /* end of if */

#if (SYS_CPNT_RSPAN == TRUE)
    if (vlan_port_mode == VAL_vlanPortMode_access)
    {
        if ((RSPAN_OM_IsRspanMirroredPort(lport_ifindex) == TRUE) ||
            (RSPAN_OM_IsRspanUplinkPort(lport_ifindex) == TRUE) ||
            (RSPAN_OM_IsRspanMirrorToPort(lport_ifindex) == TRUE))
        {
            return FALSE;
        }
    }
#endif

    /* If port_mode information is set succesfully, then check
     * thru the vlan list and make necessary modification to
     * dot1q_vlan_current_egress_ports and dot1q_vlan_current_untagged_ports.
     */
    switch (vlan_port_mode)
    {
        case VAL_vlanPortMode_hybrid:
        case VAL_vlanPortMode_access:
#if (SYS_CPNT_MSTP_SUPPORT_PVST == TRUE && SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
            if (    (VAL_vlanPortMode_dot1qTrunk == port_info.vlan_port_entry.vlan_port_mode)
                 && (VAL_dot1qGvrpStatus_enabled == port_info.port_item.dot1q_port_gvrp_status)
               )
            {
                break;
            } /* end of if */
#endif /* end #if (SYS_CPNT_MSTP_SUPPORT_PVST == TRUE && SYS_CPNT_Q_TRUNK_MEMBER == TRUE) */

#if(SYS_CPNT_ADD == TRUE)
            {
                UI32_T voice_vlan_port_mode;
                ADD_OM_GetVoiceVlanPortMode(lport_ifindex, &voice_vlan_port_mode);
                if(     (voice_vlan_id != 0)
                     && (vlan_port_mode == VAL_vlanPortMode_access)
                     && (voice_vlan_port_mode != VAL_voiceVlanPortMode_none)
                  )
                {
                    ret = FALSE;
                    break;
                }
            }
#endif

            if (VAL_vlanPortMode_hybrid == vlan_port_mode)
            {
                ret = VLAN_MGR_SetHybridMode(lport_ifindex);
            }
            else
            {
                ret = VLAN_MGR_SetAccessMode(lport_ifindex);
            } /* end of if */

            pvid_vlan_info.dot1q_vlan_index = (UI16_T)port_info.port_item.dot1q_pvid_index;
            if (!VLAN_OM_GetVlanEntry(&pvid_vlan_info))
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_VLAN,
                                         VLAN_MGR_SetVlanPortMode_Fun_No,
                                         EH_TYPE_MSG_NOT_EXIST,
                                         SYSLOG_LEVEL_INFO,
                                         "VLAN entry"
                                        );
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
            } /* end of if */

            /* a port shall be an untagged member of pvid vlan in access or hybrid mode */
            is_new_member_added = FALSE;
            is_member_tag_changed = FALSE;
            if (!VLAN_OM_IS_MEMBER(pvid_vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex))
            {
                is_new_member_added = TRUE;
            }
            else if (!VLAN_OM_IS_MEMBER(pvid_vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
            {
                is_member_tag_changed = TRUE;
            }

            if (!VLAN_MGR_AddVlanMember(port_info.port_item.dot1q_pvid_index, lport_ifindex, VLAN_MGR_BOTH, VAL_dot1qVlanStatus_permanent))
            {
                if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                    BACKDOOR_MGR_Printf("\r\nERROR: VLAN_MGR_AddVlanMember in VLAN_MGR_SetVlanPortMode\r\n");
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
            }

            if (pvid_vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
            {
                if (is_new_member_added)
                {
                    /* new port member is added to pvid vlan */
                    VLAN_MGR_Notify_VlanMemberAdd(port_info.port_item.dot1q_pvid_index, lport_ifindex, VAL_dot1qVlanStatus_permanent);
                } /* end of if */

                if (is_member_tag_changed == TRUE)
                {
                    VLAN_MGR_Notify_VlanMemberTagChanged(port_info.port_item.dot1q_pvid_index, lport_ifindex);
                }
            }

#if ((SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE) || (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK == TRUE))
            {
                UI32_T  pvid;
                VLAN_IFINDEX_CONVERTTO_VID(port_info.port_item.dot1q_pvid_index, pvid);

    #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE)
                if (vlan_port_mode == VAL_vlanPortMode_access)
                {
                    ret = VLAN_MGR_ConformToSingleUntaggedVlan(pvid, lport_ifindex, VAL_dot1qVlanStatus_permanent);
                }
    #endif /* end of #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE) */

    #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK == TRUE)
                if (vlan_port_mode == VAL_vlanPortMode_hybrid)
                {
                    ret = VLAN_MGR_ConformToSingleUntaggedVlan(pvid, lport_ifindex, VAL_dot1qVlanStatus_permanent);
                }
    #endif /* end of #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK == TRUE) */
            }
#endif /* end of #if ((SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE) || (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK == TRUE)) */

            if (TRUE == ret)
            {
#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
                if (VLAN_MGR_LocalIsQTrunkPortMember(lport_ifindex) == FALSE)
                {
                    break;
                } /* end of if */

                VLAN_MGR_LocalSetPortTrunkLinkMode(lport_ifindex, VLAN_MGR_NOT_TRUNK_LINK);

    /* Change admin_acceptable_frame_types to "all_frame" */
    #if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
                VLAN_MGR_SetDot1qPortAcceptableFrameTypes_(lport_ifindex, VAL_dot1qPortAcceptableFrameTypes_admitAll);
    #endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE) */
                VLAN_MGR_LocalSetDot1qPvidToDefault(lport_ifindex);
                if (port_type == SWCTRL_LPORT_TRUNK_PORT)
                {
                    VLAN_MGR_SetTrunkMemberPortInfo(lport_ifindex, VLAN_PORT_MODE_FIELD, VAL_vlanPortMode_dot1qTrunk);
                } /* end of if */

                vlan_info->dot1q_vlan_index = 0;
                while (1)
                {
                    if (!VLAN_OM_GetNextVlanEntry(&vlan_info))
                    {
                        break;
                    } /* end of if */

                    if (vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_permanent)
                    {
                        VLAN_IFINDEX_CONVERTTO_VID(vlan_info.dot1q_vlan_index,vid);

                        if (vid == VLAN_OM_GetGlobalDefaultVlan())
                        {
                            BOOL_T  notify_vlan_member_add = FALSE;
                            BOOL_T  notify_member_tag_changed = FALSE;
                            UI32_T  old_pvid;

                            VLAN_IFINDEX_CONVERTTO_VID(port_info.port_item.dot1q_pvid_index, old_pvid);

                            /* Check if need to notify other registered components */
                            if (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex))
                            {
                                notify_vlan_member_add = TRUE;
                            }
                            else if (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
                            {
                                /* assume VLAN_MGR_SetDefaultPortMembership() join as untagged nember */
                                notify_member_tag_changed = TRUE;
                            }

                            if (!VLAN_MGR_SetDefaultPortMembership(vid, lport_ifindex))
                            {
                                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                            } /* end of if */

                            if(notify_vlan_member_add)
                            {
                                VLAN_MGR_Notify_VlanMemberAdd(vlan_info.dot1q_vlan_index, lport_ifindex, VAL_dot1qVlanStatus_permanent);
                            } /* end of if */

                            if (notify_member_tag_changed == TRUE)
                            {
                                VLAN_MGR_Notify_VlanMemberTagChanged(vlan_info.dot1q_vlan_index, lport_ifindex);
                            }

                            if (old_pvid != vid)
                            {
                                VLAN_MGR_Notify_PvidChange(lport_ifindex, old_pvid, vid);
                            } /* end of if */

    #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE)
                            if (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_access)
                            {
                                VLAN_MGR_ConformToSingleUntaggedVlan(vid, lport_ifindex, VAL_dot1qVlanStatus_permanent);
                            }
    #endif /* end of #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE) */

    #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK == TRUE)
                            if (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_hybrid)
                            {
                                VLAN_MGR_ConformToSingleUntaggedVlan(vid, lport_ifindex, VAL_dot1qVlanStatus_permanent);
                            }
    #endif /* end of #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK == TRUE) */
                        } /* end of if */

                        else
                        {
                            if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex))
                            {
                                if (!VLAN_MGR_RemoveVlanMember(vlan_info.dot1q_vlan_index,lport_ifindex,VLAN_MGR_BOTH,L_RSTATUS_ACTIVE_2_NOTEXIST, VAL_dot1qVlanStatus_permanent))
                                {
                                    if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                                        BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_RemoveVlanMember in VLAN_MGR_DeleteEgressPortMember\n");
                                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                                } /* end of if */

                                VLAN_OM_SetIfStackLastUpdateTime(vlan_info.dot1q_vlan_time_mark);

                                VLAN_MGR_Notify_VlanMemberDelete(vlan_info.dot1q_vlan_index, lport_ifindex, vlan_info.dot1q_vlan_status);
                            } /* end of if */
                        } /* end of else */

                        if (!VLAN_OM_GetVlanEntry(&vlan_info))
                        {
                            EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                                        VLAN_MGR_SetVlanPortMode_Fun_No,
                                                        EH_TYPE_MSG_NOT_EXIST,
                                                        SYSLOG_LEVEL_INFO,
                                                        "VLAN entry"
                                                    );
                            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                        } /* end of if */
                    } /* end of if */

                    VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, lport_ifindex);
                    vlan_info.dot1q_vlan_time_mark = SYS_TIME_GetSystemTicksBy10ms();
                    if (!VLAN_OM_SetVlanEntry(&vlan_info))
                    {
                        break;
                    } /* end of if */
                } /* end of while*/
#endif /* end of #if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE) */
            } /* end of if */
            break;

        case VAL_vlanPortMode_dot1qTrunk:
            if (TRUE == (ret = VLAN_MGR_SetQTrunkMode(lport_ifindex)))
            {
#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
                if (!VLAN_MGR_LocalIsQTrunkPortMember(lport_ifindex))
                {
                    VLAN_MGR_LocalSetPortTrunkLinkMode(lport_ifindex, VLAN_MGR_TRUNK_LINK);

    #if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
                    VLAN_MGR_SetDot1qPortAcceptableFrameTypes_(lport_ifindex, VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanTagged);
    #else
                    VLAN_MGR_LocalSetDot1qPvidToDefault(lport_ifindex);
    #endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE) */

                    if (port_type == SWCTRL_LPORT_TRUNK_PORT)
                    {
                        VLAN_MGR_SetTrunkMemberPortInfo(lport_ifindex, VLAN_PORT_MODE_FIELD, VAL_vlanPortMode_dot1qTrunk);
                    } /* end of if */

                    vlan_info->dot1q_vlan_index = 0;
                    while (1)
                    {
                        if (!VLAN_OM_GetNextVlanEntry(&vlan_info))
                        {
                            break;
                        } /* end of if */

    #if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
                        /* The default vlan contains all ports in its untagged list. */
                        if (vlan_info.dot1q_vlan_index == VLAN_MGR_DOT1Q_DEFAULT_PVID_IFINDEX)
                        {
                            continue;
                        } /* end of if */
    #endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE) */
                        if (!VLAN_MGR_LocalAddPortTrunkLinkToVlan(vlan_info.dot1q_vlan_index))
                        {
                            if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                                BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_LocalAddPortTrunkLinkToVlan in VLAN_MGR_SetVlanPortMode\n");
                            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
                        } /* end of if */
                    } /* end of while*/
                } /* end of if */
#endif /* end of #if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE) */
            } /* end of if */
            break;

        default:
            ret = FALSE;
            break;
    } /* end of switch */


    if(ret)
        VLAN_MGR_Notify_VlanPortModeChanged(lport_ifindex, vlan_port_mode);
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, ret);
} /* End of VLAN_MGR_SetVlanPortMode() */


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
BOOL_T VLAN_MGR_SetDot1qPortGvrpStatusEnabled(UI32_T lport_ifindex, UI32_T gvrp_status)
{
    VLAN_OM_Vlan_Port_Info_T    vlan_port_info;
    SWCTRL_Lport_Type_T         port_type;
    UI32_T                              unit, port, trunk_id;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (!VLAN_MGR_PreconditionForSetVlanPortInfo(lport_ifindex,
                                                 GVRP_STATUS_FIELD,
                                                 gvrp_status))
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if ((port_type = SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit, &port, &trunk_id)) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qPortGvrpStatusEnabled_Fun_No,
                                    EH_TYPE_MSG_IS_INVALID,
                                    SYSLOG_LEVEL_INFO,
                                    "Port"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
    /* lport_ifindex is the key to search for vlan entry in the database.
     */
    vlan_port_info.lport_ifindex = lport_ifindex;

    /* Error if vlan_port_info can not be properly retrieve from the database.
     */
    if(!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qPortGvrpStatusEnabled_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    vlan_port_info.port_item.dot1q_port_gvrp_status = gvrp_status;

    if(VLAN_OM_SetVlanPortEntry(&vlan_port_info))
    {
        if (port_type == SWCTRL_LPORT_TRUNK_PORT)
            VLAN_MGR_SetTrunkMemberPortInfo(lport_ifindex,GVRP_STATUS_FIELD, gvrp_status);
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
    } /* end of if */
    else
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticName_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                    "lport (1-SYS_ADPT_TOTAL_NBR_OF_LPORT"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

} /* end of VLAN_MGR_SetDot1qPortGvrpStatusEnabled() */


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
BOOL_T VLAN_MGR_SetDot1qPortGvrpFailedRegistrations(UI32_T lport_ifindex)
{
    VLAN_OM_Vlan_Port_Info_T        vlan_port_info;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    /* This function only returns TRUE if the specific port is a normal port
       or Trunk port.  Trunk_member port attribute can not be modify individually.
     */
    if (!SWCTRL_LogicalPortExisting(lport_ifindex))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qPortGvrpFailedRegistrations_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                    "lport"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    vlan_port_info.lport_ifindex = lport_ifindex;

    /* Error if vlan_port_info can not be properly retrieve from the database.
     */
    if(!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qPortGvrpFailedRegistrations_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    vlan_port_info.port_item.dot1q_port_gvrp_failed_registrations++;

    if(VLAN_OM_SetVlanPortEntry(&vlan_port_info))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);

    } /* end of if */
    else
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qVlanStaticName_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                    "lport (1-SYS_ADPT_TOTAL_NBR_OF_LPORT"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }
} /* end of VLAN_MGR_SetDot1qPortGvrpFailedRegistrations() */


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
BOOL_T VLAN_MGR_SetDot1qPortGvrpLastPduOrigin(UI32_T lport_ifindex,
                                              UI8_T *pdu_mac_address)
{
    VLAN_OM_Vlan_Port_Info_T    vlan_port_info;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    /* This function only returns TRUE if the specific port is a normal port
       or Trunk port.  Trunk_member port attribute can not be modify individually.
     */
    if (!SWCTRL_LogicalPortExisting(lport_ifindex))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qPortGvrpLastPduOrigin_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                    "lport"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
    vlan_port_info.lport_ifindex = lport_ifindex;

    /* Error if vlan_port_info can not be properly retrieve from the database.
     */
    if(!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qPortGvrpLastPduOrigin_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    memcpy(&vlan_port_info.port_item.dot1q_port_gvrp_last_pdu_origin, pdu_mac_address, SIZE_dot1qPortGvrpLastPduOrigin);

    if(VLAN_OM_SetVlanPortEntry(&vlan_port_info))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
    } /* end of if */
    else
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qPortGvrpLastPduOrigin_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                    "lport (1-SYS_ADPT_TOTAL_NBR_OF_LPORT"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }
} /* end of VLAN_MGR_SetDot1qPortGvrpLastPduOrigin() */


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
BOOL_T VLAN_MGR_GetPortEntry(UI32_T lport_ifindex, VLAN_OM_Vlan_Port_Info_T *vlan_port_info)
{
    VLAN_OM_Vlan_Port_Info_T        current_vlan_port_info;
    UI32_T                          pvid,unit, port, trunk_id;
    SWCTRL_Lport_Type_T             port_type;


    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    port_type = SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit,&port, &trunk_id);

    if (port_type == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_GetPortEntry_Fun_No,
                                    EH_TYPE_MSG_IS_INVALID,
                                    SYSLOG_LEVEL_INFO,
                                    "Port"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
    current_vlan_port_info.lport_ifindex = lport_ifindex;

    if (!VLAN_OM_GetVlanPortEntry(&current_vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_GetPortEntry_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    /* For Trunk Port Infomation, VLAN_MGR need to convert pvid_ifindex to pvid if the
       specific trunk has valid member (which implies it also has valid pvid_ifindex stored in
       the database).  Otherwise, for trunk with no member, pvid_ifindex
       does not exist and no convertion is needed.
     */
    if ((port_type == SWCTRL_LPORT_TRUNK_PORT) && (!TRK_MGR_GetTrunkMemberCounts(trunk_id)))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    VLAN_IFINDEX_CONVERTTO_VID(current_vlan_port_info.port_item.dot1q_pvid_index, pvid);
    current_vlan_port_info.port_item.dot1q_pvid_index = pvid;
    memcpy(vlan_port_info,&current_vlan_port_info, sizeof(VLAN_OM_Vlan_Port_Info_T));

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);

} /* End of VLAN_MGR_GetPortEntry() */


/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetNextPortEntry
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
BOOL_T VLAN_MGR_GetNextPortEntry(UI32_T *lport_ifindex, VLAN_OM_Vlan_Port_Info_T *vlan_port_info)
{
    /* BODY */

    if (VLAN_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    /* if user input value for lport_ifindex is less than the first entry,
       return the first lport entry.
     */
    if (*lport_ifindex < SYS_ADPT_ETHER_1_IF_INDEX_NUMBER)
    {
        *lport_ifindex = SYS_ADPT_ETHER_1_IF_INDEX_NUMBER;
        return  VLAN_MGR_GetPortEntry(*lport_ifindex, vlan_port_info);
    }
    else
    {
        if (SWCTRL_GetNextLogicalPort(lport_ifindex) != SWCTRL_LPORT_UNKNOWN_PORT)
            return  VLAN_MGR_GetPortEntry(*lport_ifindex, vlan_port_info);
        else
            return FALSE;
    }

} /* End of VLAN_MGR_GetNextPortEntry() */


/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetPortEntry
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan_port entry can
 *            be retrieve successfully.  Otherwiser, return false.
 * INPUT    : lport_ifindex   -- the logical port number
 * OUTPUT   : *entry   -- the whole data structure to store the value
 * RETURN   : TRUE  \ FALSE
 * NOTES    : None
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_GetVlanPortEntry(UI32_T lport_ifindex, VLAN_OM_VlanPortEntry_T *vlan_port_entry)
{
    VLAN_OM_Vlan_Port_Info_T        port_info;
    UI32_T                          unit, port, trunk_id;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);


    if (SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit,&port, &trunk_id) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_GetVlanPortEntry_Fun_No,
                                    EH_TYPE_MSG_IS_INVALID,
                                    SYSLOG_LEVEL_INFO,
                                    "Port"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
    port_info.lport_ifindex = lport_ifindex;

    if (!VLAN_OM_GetVlanPortEntry(&port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_GetVlanPortEntry_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    memcpy(vlan_port_entry, &port_info.vlan_port_entry, sizeof(VLAN_OM_VlanPortEntry_T));
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);

} /* end of VLAN_MGR_GetVlanPortEntry() */


/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetNextVlanPortEntry
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available vlan_port entry can
 *            be retrieve successfully.  Otherwiser, return false.
 * INPUT    : *lport_ifindex  -- the logical port number
 * OUTPUT   : *lport_ifindex  -- the next logical port number
 *            *entry   -- the whole data structure to store the value
 * RETURN   : TRUE \ FALSE
 * NOTES    : None
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_GetNextVlanPortEntry(UI32_T *lport_ifindex, VLAN_OM_VlanPortEntry_T *vlan_port_entry)
{
    /* BODY */

    if (VLAN_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    if (*lport_ifindex < SYS_ADPT_ETHER_1_IF_INDEX_NUMBER)
    {
        *lport_ifindex = SYS_ADPT_ETHER_1_IF_INDEX_NUMBER;
        return  VLAN_MGR_GetVlanPortEntry(*lport_ifindex , vlan_port_entry);
    }
    else
    {
        if (SWCTRL_GetNextLogicalPort(lport_ifindex) != SWCTRL_LPORT_UNKNOWN_PORT)
            return VLAN_MGR_GetVlanPortEntry(*lport_ifindex, vlan_port_entry);
        else
            return FALSE;
    }

} /* end of VLAN_MGR_GetNextVlanPortEntry() */


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
BOOL_T VLAN_MGR_GetDot1qPortVlanEntry(UI32_T lport_ifindex, VLAN_OM_Dot1qPortVlanEntry_T *vlan_port_entry)
{
    VLAN_OM_Vlan_Port_Info_T        vlan_port_info;
    UI32_T          pvid, unit, port, trunk_id;
    SWCTRL_Lport_Type_T     port_type;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);


    port_type = SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit,&port, &trunk_id);

    vlan_port_info.lport_ifindex = lport_ifindex;

    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_GetDot1qPortVlanEntry_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    /* For Trunk Port Infomation, VLAN_MGR need to convert pvid_ifindex to pvid if the
       specific trunk has valid member (which implies it also has valid pvid_ifindex stored in
       the database).  Otherwise, for trunk with no member, pvid_ifindex
       does not exist and no convertion is needed.
     */
    if ((port_type == SWCTRL_LPORT_TRUNK_PORT) && (!TRK_MGR_GetTrunkMemberCounts(trunk_id)))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    VLAN_IFINDEX_CONVERTTO_VID(vlan_port_info.port_item.dot1q_pvid_index, pvid);

    vlan_port_info.port_item.dot1q_pvid_index = pvid;

    memcpy(vlan_port_entry, &vlan_port_info.port_item, sizeof(VLAN_OM_Dot1qPortVlanEntry_T));
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);

}/* end of VLAN_MGR_GetDot1qPortVlanEntry()*/


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
BOOL_T VLAN_MGR_GetNextDot1qPortVlanEntry(UI32_T *lport_ifindex, VLAN_OM_Dot1qPortVlanEntry_T *vlan_port_entry)
{
    /* BODY */

    if (VLAN_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    if (*lport_ifindex < SYS_ADPT_ETHER_1_IF_INDEX_NUMBER)
    {
        *lport_ifindex = SYS_ADPT_ETHER_1_IF_INDEX_NUMBER;
    }
    else
    {
        if (SWCTRL_GetNextLogicalPort(lport_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
            return FALSE;
    }
    return VLAN_MGR_GetDot1qPortVlanEntry(*lport_ifindex, vlan_port_entry);

} /* end of VLAN_MGR_GetNextDot1qPortVlanEntry() */





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
BOOL_T VLAN_MGR_SetDot1dPortGarpJoinTime(UI32_T lport_ifindex, UI32_T join_time)
{
    VLAN_OM_Vlan_Port_Info_T    vlan_port_info;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (!SWCTRL_LogicalPortExisting(lport_ifindex))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1dPortGarpJoinTime_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                    "lport"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    vlan_port_info.lport_ifindex = lport_ifindex;

    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1dPortGarpJoinTime_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);

    } /* end of if */

    if ((vlan_port_info.garp_entry.dot1d_port_garp_leave_all_time < join_time) ||
        (vlan_port_info.garp_entry.dot1d_port_garp_leave_time < (2*join_time)))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1dPortGarpJoinTime_Fun_No,
                                    EH_TYPE_MSG_IS_INVALID,
                                    SYSLOG_LEVEL_INFO,
                                    "The join time"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    vlan_port_info.garp_entry.dot1d_port_garp_join_time = join_time;

    if (!VLAN_OM_SetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1dPortGarpJoinTime_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                    "lport (1-SYS_ADPT_TOTAL_NBR_OF_LPORT"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);

} /* end of VLAN_MGR_SetDot1dPortGarpJoinTime() */


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
BOOL_T VLAN_MGR_SetDot1dPortGarpLeaveTime(UI32_T lport_ifindex, UI32_T leave_time)
{
    VLAN_OM_Vlan_Port_Info_T    vlan_port_info;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (!SWCTRL_LogicalPortExisting(lport_ifindex))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1dPortGarpLeaveTime_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                    "lport"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    vlan_port_info.lport_ifindex = lport_ifindex;

    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1dPortGarpLeaveTime_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    if ((vlan_port_info.garp_entry.dot1d_port_garp_leave_all_time < leave_time) ||
        (2*vlan_port_info.garp_entry.dot1d_port_garp_join_time > leave_time))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1dPortGarpLeaveTime_Fun_No,
                                    EH_TYPE_MSG_IS_INVALID,
                                    SYSLOG_LEVEL_INFO,
                                    "The leave time"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    vlan_port_info.garp_entry.dot1d_port_garp_leave_time = leave_time;

    if (!VLAN_OM_SetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1dPortGarpLeaveTime_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                    "lport (1-SYS_ADPT_TOTAL_NBR_OF_LPORT"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);

} /* end of VLAN_MGR_SetDot1dPortGarpLeaveTime() */



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
BOOL_T VLAN_MGR_SetDot1dPortGarpLeaveAllTime(UI32_T lport_ifindex, UI32_T leave_all_time)
{
    VLAN_OM_Vlan_Port_Info_T    vlan_port_info;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (!SWCTRL_LogicalPortExisting(lport_ifindex))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1dPortGarpLeaveAllTime_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                    "lport"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    vlan_port_info.lport_ifindex = lport_ifindex;

    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1dPortGarpLeaveAllTime_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);

    } /* end of if */


    if ((vlan_port_info.garp_entry.dot1d_port_garp_leave_time > leave_all_time) ||
        (vlan_port_info.garp_entry.dot1d_port_garp_join_time > leave_all_time))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1dPortGarpLeaveAllTime_Fun_No,
                                    EH_TYPE_MSG_IS_INVALID,
                                    SYSLOG_LEVEL_INFO,
                                    "The leave all time"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    vlan_port_info.garp_entry.dot1d_port_garp_leave_all_time = leave_all_time;

    if (!VLAN_OM_SetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1dPortGarpLeaveAllTime_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                    "lport (1-SYS_ADPT_TOTAL_NBR_OF_LPORT"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);

} /* end of VLAN_MGR_SetDot1dPortGarpLeaveAllTime() */


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
BOOL_T VLAN_MGR_GetDot1dPortGarpEntry(UI32_T lport_ifindex, VLAN_OM_Dot1dPortGarpEntry_T *port_garp_entry)
{
    VLAN_OM_Vlan_Port_Info_T        vlan_port_info;
    UI32_T                          unit, port, trunk_id;
    SWCTRL_Lport_Type_T             port_type;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);


    port_type = SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit,&port, &trunk_id);

    if (port_type == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_GetDot1dPortGarpEntry_Fun_No,
                                    EH_TYPE_MSG_IS_INVALID,
                                    SYSLOG_LEVEL_INFO,
                                    "Port"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
    /* Trunk port infomation is only valid if the specific trunk_id contains valid trunk members.
       For Trunk with no members, the attribute are invalid.
     */
    if ((port_type == SWCTRL_LPORT_TRUNK_PORT) && (!TRK_MGR_GetTrunkMemberCounts(trunk_id)))
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    vlan_port_info.lport_ifindex = lport_ifindex;

    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_GetDot1dPortGarpEntry_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    memcpy(port_garp_entry, &vlan_port_info.garp_entry, sizeof(VLAN_OM_Dot1dPortGarpEntry_T));
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);

} /* end of VLAN_MGR_GetDot1dPortGarpEntry() */


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
BOOL_T VLAN_MGR_GetNextDot1dPortGarpEntry(UI32_T *lport_ifindex, VLAN_OM_Dot1dPortGarpEntry_T *port_garp_entry)
{
    /* BODY */

    if (VLAN_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    if (*lport_ifindex < SYS_ADPT_ETHER_1_IF_INDEX_NUMBER)
        *lport_ifindex = SYS_ADPT_ETHER_1_IF_INDEX_NUMBER;
    else
    {
        if (SWCTRL_GetNextLogicalPort(lport_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
            return FALSE;
    }
    return VLAN_MGR_GetDot1dPortGarpEntry(*lport_ifindex, port_garp_entry);

} /* end of VLAN_MGR_GetNextDot1dPortGarpEntry() */



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
 *               and this function will also set the vlan_ip_state flag in
 *               VLAN_OM_Dot1qVlanCurrentEntry_T for the purpose to identify which
 *               VLAN is IP interface;
 *            3. Restriction:
 *               a. Row status of management vlan cannot be suspended.
 *               b. Management VLAN must be based on static VLAN.  Dynamic vlan,
 *                  created by Dynamic GVRP, cannot be selected as a management vlan.
 *            4. To be phased out, please use VLAN_MGR_SetVlanMgmtIpState() instead
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetManagementVlan(UI32_T vid)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (VLAN_MGR_SetMgmtIpStateForVlan(vid, TRUE, VLAN_MGR_IP_STATE_IPV4) == FALSE)
    {
        /* +++ LeaveCriticalRegion +++ */
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    /* +++ LeaveCriticalRegion +++ */
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
} /* End of VLAN_MGR_SetManagementVlan() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetGlobalManagementVlan
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the specified VLAN as global management VLAN
 * INPUT    : vid   - the specific vlan id
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. if vid is 0, it means just clear the global management VLAN
 *            2. the VLAN that is IP interface could be global management VLAN
 *            3. this function is paired with VLAN_OM_GetManagementVlan()
 *            4. To be phased out, please use VLAN_MGR_SetVlanMgmtIpState() instead
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetGlobalManagementVlan(UI32_T vid)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (VLAN_MGR_SetMgmtIpStateForVlan(vid, TRUE, VLAN_MGR_IP_STATE_UNCHANGED) == FALSE)
    {
        /* +++ LeaveCriticalRegion +++ */
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    /* +++ LeaveCriticalRegion +++ */
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
} /* End of VLAN_MGR_SetGlobalManagementVlan() */

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
BOOL_T VLAN_MGR_LeaveManagementVlan(UI32_T vid)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (VLAN_MGR_SetMgmtIpStateForVlan(vid, FALSE, VLAN_MGR_IP_STATE_NONE) == FALSE)
    {
        /* +++ LeaveCriticalRegion +++ */
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    /* +++ LeaveCriticalRegion +++ */
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
} /* End of VLAN_MGR_LeaveManagementVlan() */

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
BOOL_T VLAN_MGR_SetIpInterface(UI32_T vid)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (VLAN_MGR_SetMgmtIpStateForVlan(vid, FALSE, VLAN_MGR_IP_STATE_IPV4) == FALSE)
    {
        /* +++ LeaveCriticalRegion +++ */
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    /* +++ LeaveCriticalRegion +++ */
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
} /* End of VLAN_MGR_SetIpInterface() */

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
BOOL_T VLAN_MGR_SetL3IPv6Vlan(UI32_T vid)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (VLAN_MGR_SetMgmtIpStateForVlan(vid, TRUE, VLAN_MGR_IP_STATE_IPV6) == FALSE)
    {
        /* +++ LeaveCriticalRegion +++ */
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    /* +++ LeaveCriticalRegion +++ */
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
} /* End of VLAN_MGR_SetL3IPv6Vlan() */

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
BOOL_T VLAN_MGR_LeaveL3IPv6Vlan(UI32_T vid)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (VLAN_MGR_SetMgmtIpStateForVlan(vid, FALSE, VLAN_MGR_IP_STATE_NONE) == FALSE)
    {
        /* +++ LeaveCriticalRegion +++ */
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    /* +++ LeaveCriticalRegion +++ */
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
} /* End of VLAN_MGR_LeaveL3IPv6Vlan() */

/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetVlanMgmtIpState
 * ----------------------------------------------------------------------------
 * PURPOSE  : Set management vlan state and ip interface state for a vlan.
 * INPUT    : vid        - the identifier of specified vlan
 *            mgmt_state - TRUE  -> set as management vlan
 *                         FALSE -> do nothing
 *            ip_state   - VLAN_MGR_IP_STATE_NONE
 *                         VLAN_MGR_IP_STATE_IPV4
 *                         VLAN_MGR_IP_STATE_IPV6
 *                         VLAN_MGR_IP_STATE_UNCHANGED => keep the original
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : None
 * ----------------------------------------------------------------------------
 */
BOOL_T VLAN_MGR_SetVlanMgmtIpState(UI32_T vid, BOOL_T mgmt_state, UI8_T ip_state)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (VLAN_MGR_SetMgmtIpStateForVlan(vid, mgmt_state, ip_state) == FALSE)
    {
        /* +++ LeaveCriticalRegion +++ */
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    /* +++ LeaveCriticalRegion +++ */
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
} /* End of VLAN_MGR_SetVlanMgmtIpState() */

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
UI32_T VLAN_MGR_VlanChangeToL3Type(UI32_T vid, UI32_T* vid_ifindex, UI32_T* if_status){

    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    /*check the operation mode */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, VLAN_MGR_OPER_MODE_ERROR);

    if (vid <1 || vid > SYS_ADPT_MAX_VLAN_ID){
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, VLAN_MGR_VALUE_OUT_OF_RANGE);
    }

    VLAN_VID_CONVERTTO_IFINDEX(vid, *vid_ifindex);

    vlan_info.dot1q_vlan_index = (UI16_T)(*vid_ifindex);

    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, VLAN_MGR_OM_GET_ERROR);
    } /* end of if */

    if (vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_dynamicGvrp)
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, SYS_TYPE_GET_RUNNING_CFG_FAIL);
    } /* end of if */

    if(vlan_info.if_entry.ifType == VLAN_L3_IP_IFTYPE){
        *if_status = vlan_info.if_entry.vlan_operation_status;
        return VLAN_MGR_RETURN_OK;
    }else{
        vlan_info.if_entry.ifType = VLAN_L3_IP_IFTYPE;
    }
    VLAN_OM_SetVlanEntry(&vlan_info);
    *if_status = vlan_info.if_entry.vlan_operation_status;
    return VLAN_MGR_RETURN_OK;
}
/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_VlanChangeToL3Type
 * ----------------------------------------------------------------------------
 * PURPOSE  : Set ip interface type  for a vlan.
 * INPUT    : vid        - the identifier of specified vlan
 * OUTPUT   : ifindex    - interface index
 *            if_status  - the interface status (up or down)
 * RETURN   : VLAN_MGR_RETURN_OK
 *            VLAN_MGR_OM_GET_ERROR
 * NOTES    : None
 * ----------------------------------------------------------------------------
 */
UI32_T VLAN_MGR_VlanChangeToL2Type(UI32_T vid){
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;
    UI32_T vlan_ifindex;
    /*check the operation mode */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, VLAN_MGR_OPER_MODE_ERROR);

    if (vid <1 || vid > SYS_ADPT_MAX_VLAN_ID){
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, VLAN_MGR_VALUE_OUT_OF_RANGE);
    }

    VLAN_VID_CONVERTTO_IFINDEX(vid,vlan_ifindex);

    vlan_info.dot1q_vlan_index = (UI16_T)vlan_ifindex;

    if (!VLAN_OM_GetVlanEntry(&vlan_info)){
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, VLAN_MGR_OM_GET_ERROR);
    } /* end of if */

    if(vlan_info.if_entry.ifType == VLAN_L2_IFTYPE){
        return VLAN_MGR_RETURN_OK;
        /*if need check , add */
    }else{
        vlan_info.if_entry.ifType = VLAN_L2_IFTYPE;
        /*init default vlan mac */
        /*Tony.Lei*/
        if (SWCTRL_GetCpuMac(vlan_info.cpu_mac) == FALSE){
            /*printf("error\n")*/
        }
    }
    VLAN_OM_SetVlanEntry(&vlan_info);
    return VLAN_MGR_RETURN_OK;
}

UI32_T  VLAN_MGR_VlanLogicalMacChange(UI32_T vid,UI8_T * vlan_logical_mac){
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;
    UI32_T vlan_ifindex;
    /*check the operation mode */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, VLAN_MGR_OPER_MODE_ERROR);

    if (vid <1 || vid > SYS_ADPT_MAX_VLAN_ID){
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, VLAN_MGR_VALUE_OUT_OF_RANGE);
    }

    VLAN_VID_CONVERTTO_IFINDEX(vid,vlan_ifindex);

    vlan_info.dot1q_vlan_index = (UI16_T)vlan_ifindex;

    if (!VLAN_OM_GetVlanEntry(&vlan_info)){
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, VLAN_MGR_OM_GET_ERROR);
    } /* end of if */
    if(vlan_info.if_entry.ifType == VLAN_L3_IP_IFTYPE)
       memcpy(vlan_info.cpu_mac,vlan_logical_mac,SYS_ADPT_MAC_ADDR_LEN);

    VLAN_OM_SetVlanEntry(&vlan_info);
    printf("VLAN_MGR_VlanLogicalMacChange vid %lu,iftype %lu ,mac %0x%0x%0x%0x%0x%0x\n",(unsigned long)vid,(unsigned long)vlan_info.if_entry.ifType,vlan_info.cpu_mac[0],vlan_info.cpu_mac[1],vlan_info.cpu_mac[2],vlan_info.cpu_mac[3],vlan_info.cpu_mac[4],vlan_info.cpu_mac[5]);

    return VLAN_MGR_RETURN_OK;
}

/*  Miselleneous API
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetVlanMac
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns the MAC address of the specific vlan.
 * INPUT    : vid_ifidx -- specify which vlan
 * OUTPUT   : *vlan_mac -- Mac address of the vlan
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_GetVlanMac(UI32_T dot1q_vlan_index, UI8_T *vlan_mac)
{
    /* BODY */
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, VLAN_MGR_OPER_MODE_ERROR);

    if (dot1q_vlan_index <SYS_ADPT_VLAN_1_IF_INDEX_NUMBER || dot1q_vlan_index >SYS_ADPT_VLAN_1_IF_INDEX_NUMBER+SYS_ADPT_MAX_VLAN_ID-1){
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, VLAN_MGR_VALUE_OUT_OF_RANGE);
    }

    //VLAN_VID_CONVERTTO_IFINDEX(dot1q_vlan_index,&vlan_info.dot1q_vlan_index);

    vlan_info.dot1q_vlan_index = (UI16_T)dot1q_vlan_index;
    if (!VLAN_OM_GetVlanEntry(&vlan_info)){
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, VLAN_MGR_OM_GET_ERROR);
    } /* end of if */

    memcpy(vlan_mac,vlan_info.cpu_mac,SYS_ADPT_MAC_ADDR_LEN);

    /* Because the response time takes too long when call SWCTRL_GetCpuMac() to get CPU Mac.
     * It may be caused by some reason (maybe semiphore lock).
     * According to wuli sir's advise, we call STKTPLG_MGR_GetLocalUnitBaseMac() directly
     * to get the CPU MAC and pass the SWCTRL.
     */

    return TRUE;
} /* End of VLAN_MGR_GetVlanMac()*/

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
void VLAN_MGR_ReportSyslogMessage(UI8_T error_type, UI8_T error_msg, char *function_name)
{
    SYSLOG_OM_RecordOwnerInfo_T       owner_info;

    /* BODY */

    owner_info.level = error_type;
    owner_info.module_no = SYS_MODULE_VLAN;

    switch(error_type)
    {
        case SYSLOG_LEVEL_ERR:
            owner_info.function_no = VLAN_TYPE_VLAN_MGR_ROWSTATUS_FUNCTION_NUMBER;
            owner_info.error_no = VLAN_TYPE_VLAN_MGR_ROWSTATUS_ERROR_NUMBER;
            SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, SWITCH_TO_DEFAULT_MESSAGE_INDEX, function_name, 0, 0);
            break;

        case SYSLOG_LEVEL_WARNING:
            owner_info.function_no = VLAN_TYPE_VLAN_OM_FUNCTION_NUMBER;
            owner_info.error_no = VLAN_TYPE_VLAN_OM_FUNCTION_ERROR_NUMBER;
            SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, FUNCTION_RETURN_FAIL_INDEX, function_name, 0, 0);
            break;

        case SYSLOG_LEVEL_CRIT:
            /* This level include Memory Allocate and Free failure and Create Task Failure
             */
            owner_info.function_no = VLAN_TYPE_VLAN_TASK_TASK_FUNCTION_NUMBER;
            owner_info.error_no = VLAN_TYPE_VLAN_TASK_ERROR_NUMBER;
            SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, error_msg, function_name, 0, 0);
            break;

        default:
            break;

    } /* end of switch */

    return;
} /* end of VLAN_MGR_ReportSyslogMessage() */


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
void VLAN_MGR_NotifyForwardingState(UI32_T vid, UI32_T lport_ifindex, BOOL_T port_state)
{
    UI8_T   active_uport_count_per_unit[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    UI8_T   total_active_uport_count, i;
    UI8_T   port_list [SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_entry;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC_FOR_VOID(VLAN_MGR_CS_FLAG_OFF);

    if (vid > SYS_ADPT_MAX_VLAN_ID)
        VLAN_MGR_RELEASE_CSC_FOR_VOID(VLAN_MGR_CS_FLAG_OFF);


    if (vid == 0){

        UI32_T      temp_vid = 0;

        vlan_entry.dot1q_vlan_index = 0;

        while (VLAN_OM_GetNext_Vlan_With_PortJoined(lport_ifindex, &vlan_entry))
        {
            VLAN_IFINDEX_CONVERTTO_VID(vlan_entry.dot1q_vlan_index, temp_vid);
            if (vlan_entry.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
            {
                if (port_state)
                {
                    if (VAL_ifOperStatus_down == vlan_entry.if_entry.vlan_operation_status)
                    {
                        VLAN_MGR_SET_IF_OPER_STATUS(vlan_entry.if_entry, VAL_ifOperStatus_up);
                        VLAN_OM_SetVlanEntry(&vlan_entry);
                        VLAN_MGR_SEND_IF_OPER_STATUS_CHANGED_EVENT(temp_vid);
                    }
                }
                else
                {

                    if (SWCTRL_LportListToActiveUportList(  temp_vid,
                                                            vlan_entry.dot1q_vlan_current_egress_ports,
                                                            active_uport_count_per_unit, port_list
                                                         )
                       )
                    {
                        total_active_uport_count    = 0;
                        for (i = 0; i < SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
                        {
                            total_active_uport_count    += active_uport_count_per_unit[i];
                        } /* end of for */

                        if (    total_active_uport_count == 0
                            &&  VAL_ifOperStatus_up == vlan_entry.if_entry.vlan_operation_status)
                        {
                            VLAN_MGR_SET_IF_OPER_STATUS(vlan_entry.if_entry, VAL_ifOperStatus_down);
                            VLAN_OM_SetVlanEntry(&vlan_entry);
                            VLAN_MGR_SEND_IF_OPER_STATUS_CHANGED_EVENT(temp_vid);
                        }
                    } /* end of if (SWCTRL_LportListToActiveUportList) */
                } /* end of if (port_state) */
            } /* End of if (_is_vlan_member_ && _active_) */

        } /* end of while */
    }
    else
    {
        UI32_T  vid_ifindex;
        VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
        vlan_entry.dot1q_vlan_index = (UI16_T)vid_ifindex;
        if (VLAN_OM_GetVlanEntry(&vlan_entry) )
        {
            if (    VLAN_OM_IS_MEMBER(vlan_entry.dot1q_vlan_current_egress_ports, lport_ifindex)
                &&  (vlan_entry.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
               )
            {
                if (port_state)
                {
                    if (VAL_ifOperStatus_down == vlan_entry.if_entry.vlan_operation_status)
                    {
                        VLAN_MGR_SET_IF_OPER_STATUS(vlan_entry.if_entry, VAL_ifOperStatus_up);
                        VLAN_OM_SetVlanEntry(&vlan_entry);
                        VLAN_MGR_SEND_IF_OPER_STATUS_CHANGED_EVENT(vid);
                    }
                }
                else
                {
                    if (SWCTRL_LportListToActiveUportList(  vid,
                                                            vlan_entry.dot1q_vlan_current_egress_ports,
                                                            active_uport_count_per_unit, port_list
                                                         )
                       )
                    {
                        total_active_uport_count    = 0;
                        for (i = 0; i < SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
                        {
                            total_active_uport_count    += active_uport_count_per_unit[i];
                        } /* end of for */

                        if (    total_active_uport_count == 0
                            &&  VAL_ifOperStatus_up == vlan_entry.if_entry.vlan_operation_status)
                        {
                            VLAN_MGR_SET_IF_OPER_STATUS(vlan_entry.if_entry, VAL_ifOperStatus_down)
                            VLAN_OM_SetVlanEntry(&vlan_entry);
                            VLAN_MGR_SEND_IF_OPER_STATUS_CHANGED_EVENT(vid);
                        }
                    } /* end of if (SWCTRL_LportListToActiveUportList) */

                } /* end of if (port_state) */
            } /* End of if (_is_vlan_member_ && _active_) */
        } /* End of if (VLAN_OM_GetVlanEntry) */
    } /* end of else */

    VLAN_MGR_RELEASE_CSC_FOR_VOID(VLAN_MGR_CS_FLAG_ON);

} /* end of VLAN_MGR_NotifyForwardingState() */

static void VLAN_MGR_SendIfStateChangeTrap(IF_BITMAP_T *if_bitmap,
                UI32_T oper_status, UI32_T trap_enabled, UI32_T admin_status)
{
    TRAP_EVENT_TrapData_T   trap;
    size_t                  size;

    if (!if_bitmap || !IF_BITMAP_IFCOUNT(if_bitmap->if_count))
        return;

    memset(&trap,   0, (size_t)(&((TRAP_EVENT_TrapData_T *)0)->u));
    memset(&trap.u, 0, sizeof (trap.u.link_state));
    trap.trap_type = TRAP_EVENT_LINK_STATE;

    if (trap_enabled == VAL_ifLinkUpDownTrapEnable_enabled) {
        trap.flag = TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP;
    } else {
        trap.flag = TRAP_EVENT_SEND_TRAP_OPTION_LOG_ONLY;
    }

    if (oper_status == VAL_ifOperStatus_up) {
        trap.u.link_state.real_type = TRAP_EVENT_LINK_UP;
    } else if (oper_status == VAL_ifOperStatus_down) {
        trap.u.link_state.real_type = TRAP_EVENT_LINK_DOWN;
    } else {
        return;
    }

    trap.u.link_state.admin_status = admin_status;
    trap.u.link_state.oper_status  = oper_status;

    size = sizeof (*if_bitmap);
    if (IF_BITMAP_IS_MAPPED(if_bitmap->if_count))
        size += IF_BITMAP_ARRAY_SIZE(if_bitmap->if_count);

    memcpy(&trap.u.link_state.if_bitmap, if_bitmap, size);
    SNMP_PMGR_ReqSendTrap(&trap);
}

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
BOOL_T VLAN_MGR_ProcessForwardingSignal(UI32_T *last_update_vid)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    VLAN_OM_IfEntry_T   trap_entry_1st;
    UI32_T  local_vid,index = 0,max_loop;
    BOOL_T will_loop = FALSE,is_1st_trap_entry = TRUE;
    IF_BITMAP_T  if_bit_map ={0} ;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return TRUE;

    local_vid = *last_update_vid + 1 ;
    max_loop = SYS_ADPT_MAX_VLAN_ID;

 Tony:
    while(local_vid <= max_loop)
    {
          if(vlan_operstatus_changed[local_vid] == TRUE)
          {
              vlan_operstatus_changed[local_vid] = FALSE;

              VLAN_VID_CONVERTTO_IFINDEX(local_vid,vlan_info.dot1q_vlan_index);

              if (VLAN_OM_GetVlanEntry(&vlan_info))
              {
                  VLAN_MGR_Notify_IfOperStatusChanged(&vlan_info,
                                                      (UI32_T)vlan_info.if_entry.vlan_operation_status,
                                                      vlan_info.if_entry.link_up_down_trap_enabled,
                                                      vlan_info.if_entry.admin_status);
                  if(is_1st_trap_entry)
                  {
Tony2:
                      trap_entry_1st = vlan_info.if_entry;
                      is_1st_trap_entry = FALSE;
                      if_bit_map.if_count = 1;
                      if_bit_map.if_start = vlan_info.dot1q_vlan_index;
                  }
                  else
                  {
                      if((vlan_info.dot1q_vlan_index != (if_bit_map.if_start + if_bit_map.if_count))/*vid is not successive, send trap*/
                          || (trap_entry_1st.vlan_operation_status  != vlan_info.if_entry.vlan_operation_status) /*status is not the same as already stored*/
                          || (trap_entry_1st.link_up_down_trap_enabled != vlan_info.if_entry.link_up_down_trap_enabled)
                          || (trap_entry_1st.admin_status != vlan_info.if_entry.admin_status))
                      {
                              VLAN_MGR_SendIfStateChangeTrap( &if_bit_map, trap_entry_1st.vlan_operation_status,
                                                trap_entry_1st.link_up_down_trap_enabled, trap_entry_1st.admin_status );
                              goto Tony2;
                        }

                       if_bit_map.if_count ++;

                  }
              }
              if(index++ > SYS_ADPT_MAX_NBR_OF_VLAN)
              {
                  if(if_bit_map.if_count)
                      VLAN_MGR_SendIfStateChangeTrap( &if_bit_map, trap_entry_1st.vlan_operation_status,
                                        trap_entry_1st.link_up_down_trap_enabled, trap_entry_1st.admin_status );

                  *last_update_vid = local_vid ;
                  return FALSE;
              }

           }
          /*find next*/
          local_vid ++;
    }

    if((local_vid > SYS_ADPT_MAX_VLAN_ID)&&(FALSE == will_loop))
    { /*loop again, why*/
        max_loop = *last_update_vid;
        local_vid = 1;
        will_loop = TRUE;
        goto Tony;
    }

    if(if_bit_map.if_count)
        VLAN_MGR_SendIfStateChangeTrap( &if_bit_map, trap_entry_1st.vlan_operation_status,
                          trap_entry_1st.link_up_down_trap_enabled, trap_entry_1st.admin_status );

    return TRUE;

} /* end of VLAN_MGR_ProcessForwardingSignal() */


/* Q-TRUNK API
 */

#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_IsQTrunkPortMember
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns TRUE if this port has been configured as
 *            a Q-Trunk port.  Otherwise, return FALSE.
 * INPUT    : lport_ifindex.
 * OUTPUT   : none
 * RETURN   : TRUE / FALSE
 * NOTES    :
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_IsQTrunkPortMember(UI32_T lport_ifindex)
{
    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (VLAN_MGR_LocalIsQTrunkPortMember(lport_ifindex) == TRUE)
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
    }
    else
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

} /* end of VLAN_MGR_IsQTrunkPortMember() */
#endif

#if (SYS_CPNT_SOFTBRIDGE == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetNextVlanList_ByLport
 *-------------------------------------------------------------------------
 * PURPOSE  : This funcion returns true if the next vlans in which the
 *            specified port joins is available. Otherwise, returns false.
 * INPUT    : lport     -- logical port number
 *            vid       -- the vlan ID
 *            num       -- number of the following vlans on demand
 * OUTPUT   : vlist     -- the pointer of the output buffer for the following
 *                         vlan list
 *            rtn_num   -- number of the returned vlans following the specified
 *                         vid
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T VLAN_MGR_GetNextVlanList_ByLport(UI32_T lport, UI16_T vid, UI32_T num, UI16_T *vlist, UI16_T *rtn_num)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    UI32_T      vid_ifindex;
    UI32_T      vlan_num;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if ((lport == 0) ||
        (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_GetNextVlanList_ByLport_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                    "lport (1~SYS_ADPT_TOTAL_NBR_OF_LPORT)");
        return FALSE;
    }

    if (vid >= SYS_ADPT_MAX_VLAN_ID)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_GetNextVlanList_ByLport_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                    "vid (0~(SYS_ADPT_MAX_VLAN_ID-1)");
        return FALSE;
    }

    memset(vlist, 0, (sizeof(UI16_T)*num));
    vlan_num = 0;

    /* dot1q_vlan_index is the key to search for vlan_info in the database.
     */
    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    while(VLAN_OM_GetNextVlanEntry(&vlan_info))
    {
        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport))
        {
            vlist[vlan_num] = vlan_info.dot1q_vlan_index -  SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + 1;
            vlan_num++;
            if (vlan_num >= num)
            {
                break;
            }
        }
    } /* end of while */

    *rtn_num = vlan_num;

    return (vlan_num > 0);
} /* end VLAN_MGR_GetNextVlanList_ByLport() */

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
BOOL_T VLAN_MGR_JoinMgmtPort(UI32_T vid_ifindex, VLAN_TYPE_MemberType_T member_option, UI32_T vlan_status)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    UI32_T      mgmt_port = SYS_ADPT_MGMT_PORT;
    UI32_T      vid;
    UI32_T      port_state;
    BOOL_T      ret = FALSE, is_vlan_member = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    if ((vid_ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER) ||
        (vid_ifindex > (SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + SYS_ADPT_MAX_VLAN_ID - 1)))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_JoinMgmtPort_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                    "vid_ifindex");
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_JoinMgmtPort_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry");
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    vlan_port_info.lport_ifindex = mgmt_port;
    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_JoinMgmtPort_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry");
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    if (VLAN_MGR_TAGGED_ONLY == member_option)
    {
        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, mgmt_port))
        {
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
        }
        is_vlan_member = FALSE;
    }
    else if (VLAN_MGR_BOTH == member_option)
    {
        if (VAL_dot1qVlanStatus_dynamicGvrp == vlan_status)
        {
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        }
        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, mgmt_port))
        {
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
        }
        is_vlan_member = VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, mgmt_port);
    }

    if ((VAL_vlanPortMode_dot1qTrunk == vlan_port_info.vlan_port_entry.vlan_port_mode) &&
       (VLAN_MGR_BOTH == member_option))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    if (vlan_port_info.port_trunk_mode)
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, mgmt_port))
    {
        if (VAL_dot1qVlanStatus_permanent == vlan_status)
        {
            VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, mgmt_port);
        }
        else
        {
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        }
    }

    if (VAL_dot1qVlanStatus_permanent == vlan_status)
    {
        if (VAL_dot1qVlanStatus_dynamicGvrp == vlan_info.dot1q_vlan_status)
        {
            vlan_info.dot1q_vlan_status = VAL_dot1qVlanStatus_permanent;
        }

        if (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, mgmt_port))
        {
            VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, mgmt_port);
            vlan_port_info.port_item.static_joined_vlan_count++;
        }
        if (VLAN_MGR_BOTH == member_option)
        {
            VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, mgmt_port);
        }
    }
    VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, mgmt_port);

    if (VLAN_MGR_BOTH == member_option)
    {
        VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, mgmt_port);
        vlan_port_info.port_item.untagged_joined_vlan_count++;
    }

    VLAN_IFINDEX_CONVERTTO_VID(vid_ifindex, vid);
    if (VAL_dot1qVlanStaticRowStatus_active == vlan_info.dot1q_vlan_static_row_status)
    {
        if (VAL_ifOperStatus_up != vlan_info.if_entry.vlan_operation_status)
        {
            if (!XSTP_OM_GetPortStateByVlan(vid, mgmt_port, &port_state))
            {
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
            }

            if (VAL_dot1dStpPortState_forwarding == port_state)
            {
                VLAN_MGR_SET_IF_OPER_STATUS(vlan_info.if_entry, VAL_ifOperStatus_up);
            }
        } /* end of if */
    }

    if (VLAN_MGR_SetVlanRowStatus(L_RSTATUS_SET_OTHER_COLUMN, &vlan_info))
    {
        ret = TRUE;

        VLAN_OM_SetVlanPortEntry(&vlan_port_info);

        VLAN_MGR_SEND_IF_OPER_STATUS_CHANGED_EVENT(vid);
        vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;
        if (!VLAN_OM_GetVlanEntry(&vlan_info))
        {
            EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                        VLAN_MGR_JoinMgmtPort_Fun_No,
                                        EH_TYPE_MSG_NOT_EXIST,
                                        SYSLOG_LEVEL_INFO,
                                        "VLAN entry");
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        }

        if (    VAL_dot1qVlanStaticRowStatus_active == vlan_info.dot1q_vlan_static_row_status
            &&  !is_vlan_member
           )
        {
            VLAN_MGR_Notify_VlanMemberAdd(vid_ifindex, mgmt_port, vlan_status);
        }
    } /* end if (VLAN_MGR_SetVlanRowStatus(L_RSTATUS_SET_OTHER_COLUMN, &vlan_info))*/

    return ret;
} /* End of VLAN_MGR_JoinMgmtPortToVlan() */

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
BOOL_T VLAN_MGR_DisjoinMgmtPort(UI32_T vid_ifindex, VLAN_TYPE_MemberType_T member_option, UI32_T vlan_status)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    UI32_T      mgmt_port = SYS_ADPT_MGMT_PORT;
    UI32_T      vid;
    BOOL_T      ret = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    if ((vid_ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER) ||
        (vid_ifindex > (SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + SYS_ADPT_MAX_VLAN_ID - 1)))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DisjoinMgmtPort_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                    "vid_ifindex");
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DisjoinMgmtPort_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry");
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    vlan_port_info.lport_ifindex = mgmt_port;
    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DisjoinMgmtPort_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry");
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    if (VLAN_MGR_BOTH == member_option)
    {
        if (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, mgmt_port))
        {
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
        }
    }
    else if (VLAN_MGR_UNTAGGED_ONLY == member_option)
    {
        if (VAL_dot1qVlanStatus_dynamicGvrp == vlan_status)
        {
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        }
        if (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, mgmt_port))
        {
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
        }
    }

    /* Dynamic vlan information will not be save to startup file. Therefore,
     * join command from user configuration should also change vlan status from
     * dynamic to static to solve the issue.  Otherwise, inconsistency will occur
     * when the device reboot.
     */
    if ((VAL_dot1qVlanStatus_dynamicGvrp == vlan_info.dot1q_vlan_status) &&
       (VAL_dot1qVlanStatus_permanent == vlan_status))
    {
        vlan_info.dot1q_vlan_status = VAL_dot1qVlanStatus_permanent;
    }

    if (VLAN_MGR_PortRemovable(vid_ifindex, mgmt_port))
    {
        BOOL_T      is_static_egress_ports;

        VLAN_IFINDEX_CONVERTTO_VID(vid_ifindex, vid);
        is_static_egress_ports = VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, mgmt_port);

        /* 1. Dynamic add vlan member can not be remove Statically.
         * 2. Static add vlan member can not be remove Dynamically.
         */
        if ((VAL_dot1qVlanStatus_permanent == vlan_status) && !is_static_egress_ports)
        {
            EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                        VLAN_MGR_DisjoinMgmtPort_Fun_No,
                                        EH_TYPE_MSG_FAILED_TO_DELETE,
                                        SYSLOG_LEVEL_INFO,
                                        "the dynamic vlan member using the static configuration");
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        }

        if ((VAL_dot1qVlanStatus_dynamicGvrp == vlan_status) && is_static_egress_ports)
        {
            EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                        VLAN_MGR_DisjoinMgmtPort_Fun_No,
                                        EH_TYPE_MSG_FAILED_TO_DELETE,
                                        SYSLOG_LEVEL_INFO,
                                        "the static vlan member by GVRP operation");
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        }

#if (SYS_DFLT_VLAN_CHECK_AT_LEAST_ONE_MEMBER_IN_MANAGEMEMT_VLAN == TRUE)
        /* The last port member of management VLAN can not be removed. */
        if (    (vid == VLAN_OM_GetManagementVlanId())
             && (VLAN_MGR_CountVlanMember(vlan_info.dot1q_vlan_static_egress_ports) == 1)
           )
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_VLAN,
                                     VLAN_MGR_DeleteEgressPortMember_Fun_No,
                                     EH_TYPE_MSG_FAILED_TO_DELETE,
                                     SYSLOG_LEVEL_INFO,
                                     "the last port member of management VLAN"
                                    );
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        }
#endif /* end of #if (SYS_DFLT_VLAN_CHECK_AT_LEAST_ONE_MEMBER_IN_MANAGEMEMT_VLAN == TRUE) */

        /* Delete untagged member
         */
        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, mgmt_port))
        {
            VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, mgmt_port);
            VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, mgmt_port);
            vlan_port_info.port_item.untagged_joined_vlan_count--;
        }

        /* Only deletes Egress member when user wishes to remove both vlan memberships
         */
        if (VLAN_MGR_UNTAGGED_ONLY != member_option)
        {
            if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, mgmt_port))
            {
                VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, mgmt_port);
            }
            if (is_static_egress_ports)
            {
                VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, mgmt_port);
                vlan_port_info.port_item.static_joined_vlan_count--;
            }
        } /* end of if */

        if ((VAL_ifOperStatus_up == vlan_info.if_entry.vlan_operation_status) &&
            (VLAN_MGR_BOTH == member_option))
        {
            UI32_T      port_state;

            if (!XSTP_OM_GetPortStateByVlan(vid, mgmt_port, &port_state))
            {
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
            }
            if (VAL_dot1dStpPortState_forwarding == port_state)
            {
                UI8_T       active_uport_count_per_unit[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
                UI8_T       port_list [SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
                UI8_T       total_active_uport_count, i;

                if (TRUE == SWCTRL_LportListToActiveUportList(  vid,
                                                                vlan_info.dot1q_vlan_current_egress_ports,
                                                                active_uport_count_per_unit, port_list))
                {

                    total_active_uport_count = 0;
                    for (i = 0; i < SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
                    {
                        total_active_uport_count += active_uport_count_per_unit[i];
                    }
                    if (total_active_uport_count == 0)
                    {
                        VLAN_MGR_SET_IF_OPER_STATUS(vlan_info.if_entry, VAL_ifOperStatus_down);
                    }
                } /* end of if */
            } /* end of if */
        } /* end of if */

        if (VLAN_MGR_SetVlanRowStatus(L_RSTATUS_SET_OTHER_COLUMN, &vlan_info))
        {
            ret = TRUE;

            VLAN_OM_SetVlanPortEntry(&vlan_port_info);
            VLAN_MGR_SEND_IF_OPER_STATUS_CHANGED_EVENT(vid);

            vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;
            if (!VLAN_OM_GetVlanEntry(&vlan_info))
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_DisjoinMgmtPort_Fun_No,
                                            EH_TYPE_MSG_NOT_EXIST,
                                            SYSLOG_LEVEL_INFO,
                                            "VLAN entry");
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
            }

            if (VAL_dot1qVlanStaticRowStatus_active == vlan_info.dot1q_vlan_static_row_status)
            {
                VLAN_MGR_Notify_VlanMemberDelete(vid_ifindex, mgmt_port, vlan_status);
            }

            VLAN_OM_SetIfStackLastUpdateTime(vlan_info.dot1q_vlan_time_mark);
        }
    } /* end of if (VLAN_MGR_PortRemovable(vid_ifindex, mgmt_port)) */

    return ret;
} /* End of VLAN_MGR_DisjoinMgmtPortToVlan() */
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
void VLAN_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
#if ((SYS_CPNT_UNIT_HOT_SWAP == TRUE) || (SYS_CPNT_EXPANSION_MODULE_HOT_SWAP == TRUE))
    UI32_T  lport_ifindex = 0;
    UI32_T  ending_port_ifindex = starting_port_ifindex + number_of_port - 1;
    UI32_T  vid = 0;
    UI32_T  default_vlan_id;
    UI32_T  default_vlan_ifindex;
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_entry;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC_FOR_VOID(VLAN_MGR_CS_FLAG_OFF);

    /* The initialization sequence of the vlan configuration when module is hot inserted
     * (1) create VLAN on hot inserted module
     * (2) add inserted module port to Default VLAN
     * (3) set VLAN port info by default
     * (4) set to chip
     */

    /* (1) create VLAN on hot inserted module
     */
    vlan_entry.dot1q_vlan_index = 0;
    while (VLAN_OM_GetNextVlanEntry(&vlan_entry))
    {
        VLAN_IFINDEX_CONVERTTO_VID(vlan_entry.dot1q_vlan_index, vid);
        if (!SWCTRL_CreateVlan(vid))
        {
            if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
            {
                BACKDOOR_MGR_Printf("VLAN_MGR_HandleHotInsertion: SWCTRL_CreateVlan() failed\n");
            }
        } /* end of if */
    } /* end while (VLAN_OM_GetNextVlanEntry(&vlan_entry)) */


    default_vlan_id = VLAN_OM_GetGlobalDefaultVlan();
    VLAN_VID_CONVERTTO_IFINDEX(default_vlan_id, default_vlan_ifindex);
    vlan_entry.dot1q_vlan_index = default_vlan_ifindex;

    if (VLAN_OM_GetVlanEntry(&vlan_entry))
    {
        for(lport_ifindex = starting_port_ifindex; lport_ifindex <= ending_port_ifindex; lport_ifindex++)
        {
            /* check if the port is stacking port */
#if (SYS_CPNT_STACKING == TRUE)
            {
                UI32_T unit, u_port, port_type;
                unit = STKTPLG_OM_IFINDEX_TO_UNIT(lport_ifindex);
                u_port = STKTPLG_OM_IFINDEX_TO_PORT(lport_ifindex);

                STKTPLG_OM_GetPortType(unit, u_port, &port_type);

                if (port_type == STKTPLG_PORT_TYPE_STACKING)
                    continue;
            }
#endif

            /* (2) add inserted module port to Default VLAN
             */
            VLAN_OM_ADD_MEMBER(vlan_entry.dot1q_vlan_current_egress_ports, lport_ifindex);
            VLAN_OM_ADD_MEMBER(vlan_entry.dot1q_vlan_current_untagged_ports, lport_ifindex);
            VLAN_OM_ADD_MEMBER(vlan_entry.dot1q_vlan_static_egress_ports, lport_ifindex);
            VLAN_OM_ADD_MEMBER(vlan_entry.dot1q_vlan_static_untagged_ports, lport_ifindex);

            /* (3) set VLAN port info by default
             */
            VLAN_MGR_SetVlanPortEntryToDefault(lport_ifindex);

            /* (4) set to chip
             */
            VLAN_MGR_SetVlanPortDefaultAttributeToChip(lport_ifindex);
            VLAN_MGR_Notify_VlanMemberAdd(vlan_entry.dot1q_vlan_index,lport_ifindex, VAL_dot1qVlanStatus_permanent);

        } /* end for(lport_ifindex = starting_port_ifindex; lport_ifindex <= ending_port_ifindex; lport_ifindex++) */

        VLAN_OM_SetVlanEntry(&vlan_entry);

    } /* end if (VLAN_OM_GetVlanEntry(&vlan_entry)) */

    VLAN_MGR_RELEASE_CSC_FOR_VOID(VLAN_MGR_CS_FLAG_OFF);
    return;
#else
    return;
#endif /* end #if ((SYS_CPNT_UNIT_HOT_SWAP == TRUE) || (SYS_CPNT_EXPANSION_MODULE_HOT_SWAP == TRUE)) */
}


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
void VLAN_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
#if ((SYS_CPNT_UNIT_HOT_SWAP == TRUE) || (SYS_CPNT_EXPANSION_MODULE_HOT_SWAP == TRUE))
    UI32_T  lport_ifindex = 0;
    UI32_T  ending_port_ifindex = starting_port_ifindex + number_of_port - 1;
    UI32_T  vid = 0;
    UI8_T   port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    UI8_T   active_uport_count_per_unit[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK] = {0};
    UI8_T   total_active_uport_count = 0;
    UI8_T   unit;
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_entry;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC_FOR_VOID(VLAN_MGR_CS_FLAG_OFF);

    /* The sequence of the vlan configuration when module is hot removed
     * I.  remove this port from all VLANs
     * II. set VLAN port info by default
     */

    for(lport_ifindex = starting_port_ifindex; lport_ifindex <= ending_port_ifindex; lport_ifindex++)
    {
        /* check if the port is stacking port */
#if (SYS_CPNT_STACKING == TRUE)
        {
            UI32_T unit, u_port, port_type;
            unit = STKTPLG_OM_IFINDEX_TO_UNIT(lport_ifindex);
            u_port = STKTPLG_OM_IFINDEX_TO_PORT(lport_ifindex);

            STKTPLG_OM_GetPortType(unit, u_port, &port_type);

            if (port_type == STKTPLG_PORT_TYPE_STACKING)
                continue;
        }
#endif

        /* I. remove this port from all VLANs and callback
         */


        vlan_entry.dot1q_vlan_index = 0;

        while (VLAN_OM_GetNextVlanEntry(&vlan_entry))
        {
            VLAN_IFINDEX_CONVERTTO_VID(vlan_entry.dot1q_vlan_index, vid);
            /* Static configuration has higher priority then dynamic configuration.
             * Therefor, a dynamic created vlan can be modified and changed to
             * static create vlan.
             */
            if (VLAN_OM_IS_MEMBER(vlan_entry.dot1q_vlan_current_egress_ports, lport_ifindex))
            {
                vlan_entry.dot1q_vlan_status = VAL_dot1qVlanStatus_permanent;
            }

            /* Delete untagged/tagged member
             */
            VLAN_OM_DEL_MEMBER(vlan_entry.dot1q_vlan_current_egress_ports, lport_ifindex);
            VLAN_OM_DEL_MEMBER(vlan_entry.dot1q_vlan_current_untagged_ports, lport_ifindex);
            VLAN_OM_DEL_MEMBER(vlan_entry.dot1q_vlan_static_egress_ports, lport_ifindex);
            VLAN_OM_DEL_MEMBER(vlan_entry.dot1q_vlan_static_untagged_ports, lport_ifindex);

            VLAN_OM_SetVlanEntry(&vlan_entry);

            if (VAL_ifOperStatus_up == vlan_entry.if_entry.vlan_operation_status)
            {
                if (TRUE == SWCTRL_LportListToActiveUportList(vid,
                                                              vlan_entry.dot1q_vlan_current_egress_ports,
                                                              active_uport_count_per_unit,
                                                              port_list))
                {
                    total_active_uport_count = 0;
                    for (unit = 0; unit < SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit++)
                    {
                        total_active_uport_count += active_uport_count_per_unit[unit];
                    }
                    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
                    {
                        BACKDOOR_MGR_Printf("VLAN_MGR_HandleHotRemoval: total_active_uport_count %d\n", total_active_uport_count);
                    }
                    if (total_active_uport_count == 0)
                    {
                        VLAN_MGR_SET_IF_OPER_STATUS(vlan_entry.if_entry, VAL_ifOperStatus_down);
                        VLAN_OM_SetVlanEntry(&vlan_entry);
                        VLAN_MGR_SEND_IF_OPER_STATUS_CHANGED_EVENT(vid);
                    }
                }
            }
            VLAN_MGR_Notify_VlanMemberDelete(vlan_entry.dot1q_vlan_index,lport_ifindex,VAL_dot1qVlanStatus_permanent);
        } /* end while (VLAN_OM_GetNextVlanEntry(&vlan_entry)) */

        /* II. set VLAN port info by default value
         */
        VLAN_MGR_SetVlanPortEntryToDefault(lport_ifindex);

    } /* end for(lport_ifindex = starting_port_ifindex; lport_ifindex <= ending_port_ifindex; lport_ifindex++) */

    VLAN_MGR_RELEASE_CSC_FOR_VOID(VLAN_MGR_CS_FLAG_OFF);
    return;
#else
    return;
#endif /* end #if ((SYS_CPNT_UNIT_HOT_SWAP == TRUE) || (SYS_CPNT_EXPANSION_MODULE_HOT_SWAP == TRUE)) */
}

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
 *            2. The API will only call these funstions which performed critical section.
 *            3. The API should not be called to restore value by using "no" command.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_SetNativeVlanAgent(UI32_T lport_ifindex, UI32_T pvid)
{
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    UI32_T                              old_pvid;
    UI32_T                              pvid_ifindex;

    vlan_port_info.lport_ifindex = lport_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        return FALSE;
    }

    if (vlan_port_info.port_item.auto_vlan_mode)
    {
        if (vlan_port_info.port_item.admin_pvid == pvid)
        {
            return TRUE;
        }
    }
    else
    {
        VLAN_IFINDEX_CONVERTTO_VID(vlan_port_info.port_item.dot1q_pvid_index, old_pvid);
        if (old_pvid == pvid)
        {
            return TRUE;
        }
    }

    VLAN_VID_CONVERTTO_IFINDEX(pvid, pvid_ifindex);
    if (    (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_access)
         && (VLAN_OM_IsVlanUntagPortListMember(pvid_ifindex, lport_ifindex) == FALSE)
       )
    {
        if (!VLAN_OM_IsVlanExisted(pvid))
        {
            if (!VLAN_MGR_CreateVlan(pvid, VAL_dot1qVlanStatus_permanent))
            {
                return FALSE;
            }
            if (!VLAN_MGR_SetDot1qVlanStaticRowStatus(pvid, VAL_dot1qVlanStaticRowStatus_active))
            {
                return FALSE;
            }
        }

        if (VLAN_MGR_AddUntagPortMember(pvid, lport_ifindex,
                VAL_dot1qVlanStatus_permanent) == FALSE)
        {
            return FALSE;
        }
    }
#if 0
    if (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_dot1qTrunk)
    {
        if (!VLAN_OM_IsPortVlanMember(pvid_ifindex, lport_ifindex))
        {
            if (!VLAN_MGR_AddEgressPortMember(pvid, lport_ifindex, VAL_dot1qVlanStatus_permanent))
            {
                return FALSE;
            }
        }
    }
    else
    {
        if (!VLAN_OM_IsVlanUntagPortListMember(pvid_ifindex, lport_ifindex))
        {
            if (!VLAN_MGR_AddUntagPortMember(pvid, lport_ifindex, VAL_dot1qVlanStatus_permanent))
            {
                return FALSE;
            }
        }
    }
#endif

    if (!VLAN_MGR_SetDot1qPvid(lport_ifindex, pvid))
    {
        return FALSE;
    }

    return TRUE;
}/* End of VLAN_MGR_SetNativeVlanAgent() */
#endif /*(SYS_CPNT_VLAN_AUTO_JOIN_VLAN_FOR_PVID == TRUE)*/

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
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
BOOL_T VLAN_MGR_GetDot1qVlanStaticEntryAgent(UI32_T vid, VLAN_MGR_Dot1qVlanStaticEntry_T *vlan_entry)
{
    BOOL_T ret = FALSE;

    if (VLAN_MGR_GetDot1qVlanStaticEntry(vid, vlan_entry))
    {
        UI32_T  port_num = 0;

        while (SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_GetNextLogicalPort(&port_num))
        {
            if (VLAN_MGR_IsPortDualModeOnVlan(port_num, vid))
            {
                VLAN_OM_DEL_MEMBER(vlan_entry->dot1q_vlan_static_untagged_ports, port_num);
            }
        }
        ret = TRUE;
    }

    return ret;
}

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
BOOL_T VLAN_MGR_GetNextDot1qVlanStaticEntryAgent(UI32_T *vid, VLAN_MGR_Dot1qVlanStaticEntry_T *vlan_entry)
{
    BOOL_T ret = FALSE;

    if (VLAN_MGR_GetNextDot1qVlanStaticEntry(vid, vlan_entry))
    {
        UI32_T  port_num = 0;

        while (SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_GetNextLogicalPort(&port_num))
        {
            if (VLAN_MGR_IsPortDualModeOnVlan(port_num, *vid))
            {
                VLAN_OM_DEL_MEMBER(vlan_entry->dot1q_vlan_static_untagged_ports, port_num);
            }
        }
        ret = TRUE;
    }

    return ret;
}

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
BOOL_T VLAN_MGR_GetDot1qVlanCurrentEntryAgent(UI32_T time_mark, UI32_T vid, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry)
{
    BOOL_T ret = FALSE;

    if (VLAN_MGR_GetDot1qVlanCurrentEntry(time_mark, vid, vlan_entry))
    {
        UI32_T  port_num = 0;

        while (SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_GetNextLogicalPort(&port_num))
        {
            if (VLAN_MGR_IsPortDualModeOnVlan(port_num, vid))
            {
                VLAN_OM_DEL_MEMBER(vlan_entry->dot1q_vlan_current_untagged_ports, port_num);
            }
        }
        ret = TRUE;
    }

    return ret;
}

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
BOOL_T VLAN_MGR_GetNextDot1qVlanCurrentEntryAgent(UI32_T time_mark, UI32_T *vid, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry)
{
    BOOL_T ret = FALSE;

    if (VLAN_OM_GetNextDot1qVlanCurrentEntry(time_mark, vid, vlan_entry))
    {
        UI32_T  port_num = 0;

        while (SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_GetNextLogicalPort(&port_num))
        {
            if (VLAN_MGR_IsPortDualModeOnVlan(port_num, *vid))
            {
                VLAN_OM_DEL_MEMBER(vlan_entry->dot1q_vlan_current_untagged_ports, port_num);
            }
        }
        ret = TRUE;
    }

    return ret;
}

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
BOOL_T VLAN_MGR_EnablePortDualMode(UI32_T lport, UI32_T vid)
{
    UI32_T  vid_ifindex;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    UI32_T                              dual_mode_vlan_ifindex;

    if (VLAN_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("\r\nERROR: VLAN_MGR_EnablePortDualMode: VLAN is not existed");
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    vlan_port_info.lport_ifindex = lport;
    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    if (    (vlan_port_info.vlan_port_entry.vlan_port_dual_mode)
        &&  (vid == vlan_port_info.vlan_port_entry.dual_mode_vlan_id)
        )
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
    }
    if (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_dot1qTrunk)
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    if (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport))
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("\r\nERROR: VLAN_MGR_EnablePortDualMode: Port is not vlan member");
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }
    else if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport))
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("\r\nERROR: VLAN_MGR_EnablePortDualMode: Dual-mode port only enable on tagged member ports");
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    if (vlan_port_info.vlan_port_entry.vlan_port_dual_mode)
    {
        VLAN_VID_CONVERTTO_IFINDEX(vlan_port_info.vlan_port_entry.dual_mode_vlan_id, dual_mode_vlan_ifindex);
        if (!VLAN_MGR_RemoveVlanMember(dual_mode_vlan_ifindex, lport, VLAN_MGR_UNTAGGED_ONLY, L_RSTATUS_ACTIVE_2_NOTEXIST, VAL_dot1qVlanStatus_permanent))
        {
            if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_RemoveVlanMember in VLAN_MGR_DeleteUntagPortMember\n");
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        }
        VLAN_OM_EnterCriticalSection();
        SYS_TIME_GetSystemUpTimeByTick(VLAN_OM_GetIfStackLastUpdateTimePtr());
        VLAN_OM_LeaveCriticalSection();
        vlan_port_info.vlan_port_entry.dual_mode_vlan_id    = vid;
    }
    else
    {
        vlan_port_info.vlan_port_entry.vlan_port_dual_mode  = TRUE;
        vlan_port_info.vlan_port_entry.dual_mode_vlan_id    = vid;
    }
    if (!VLAN_OM_SetVlanPortEntry(&vlan_port_info))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    if (!VLAN_MGR_AddUntagPortMember(vid, lport, VAL_dot1qVlanStatus_permanent))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    if (!VLAN_MGR_SetDot1qPortAcceptableFrameTypes(lport, VAL_dot1qPortAcceptableFrameTypes_admitAll))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);
} /* End of VLAN_MGR_SetPortDualMode() */


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
BOOL_T VLAN_MGR_DisablePortDualMode(UI32_T lport)
{
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    UI32_T                              pvid;

    if (VLAN_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    vlan_port_info.lport_ifindex = lport;

    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    VLAN_IFINDEX_CONVERTTO_VID(vlan_port_info.port_item.dot1q_pvid_index, pvid);

    if (!vlan_port_info.vlan_port_entry.vlan_port_dual_mode)
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);
    }
    else if (VLAN_OM_IsPortVlanMember(vlan_port_info.port_item.dot1q_pvid_index, lport))
    {
        if (!VLAN_MGR_DeleteUntagPortMember(pvid, lport, VAL_dot1qVlanStatus_permanent))
        {
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);
        }
    }

    vlan_port_info.lport_ifindex = lport;

    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    vlan_port_info.vlan_port_entry.vlan_port_dual_mode = FALSE;
    vlan_port_info.vlan_port_entry.dual_mode_vlan_id    = 0;

    if (!VLAN_OM_SetVlanPortEntry(&vlan_port_info))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, VLAN_MGR_SetDot1qPortAcceptableFrameTypes(lport, VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanTagged));
}/* End of VLAN_MGR_DisablePortDualMode() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetRunningPortDualMode
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dual-mode status and mapped vlan id for this lport.
 * INPUT    :   lport                       -- logical port number
 * OUTPUT   :   BOOL_T *dual_mode_status    -- pointer of dual_mode_status.
 *              UI32_T *vid                 -- pointer of mapped vlan id.
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T VLAN_MGR_GetRunningPortDualMode(UI32_T lport, BOOL_T *dual_mode_status, UI32_T *vid)
{
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (!VLAN_MGR_GetPortEntry(lport, &vlan_port_info))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    *dual_mode_status   = vlan_port_info.vlan_port_entry.vlan_port_dual_mode;
    *vid                = vlan_port_info.vlan_port_entry.dual_mode_vlan_id;

    if (vlan_port_info.vlan_port_entry.vlan_port_dual_mode == FALSE)
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
    }
    else
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
    }
}/* End of VLAN_MGR_GetRunningPortDualMode() */
#endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)*/

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
 *            2. The API will only call these funstions which performed critical section.
 *            3. The API should not be called to restore value by using "no" command.
 *            4. VLAN should be able to deleted even though it's being used by PVID on the port.
 *               -- Add port to member list of default vlan.
 *               -- Set pvid to default vlan.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_DeleteNormalVlanAgent(UI32_T vid, UI32_T vlan_status)
{
    UI32_T                          lport;
    UI32_T                          vid_ifindex;
    UI32_T                          mgt_vid;
    BOOL_T                          result;
    VLAN_OM_Vlan_Port_Info_T        port_info;
    BOOL_T                          trunk_mode;
    UI32_T                          default_vlan_id;

    result = FALSE;

    if (    VLAN_OM_GetManagementVlan(&mgt_vid)
        &&  mgt_vid != vid
       )
    {
        if (    VLAN_OM_GetGlobalDefaultVlan_Ex(&default_vlan_id)
            &&  default_vlan_id != vid
           )
        {
            result      = TRUE;
            lport       = 0;
            VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
            while ((SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT) && (result))
            {
                trunk_mode  = FALSE;
                if (!VLAN_MGR_GetPortEntry(lport, &port_info))
                {
                    result = FALSE;
                    continue;
                }

                if (    (VLAN_OM_IsPortVlanMember(vid_ifindex, lport))
                    &&  (!port_info.port_item.auto_vlan_mode)
                    &&  (vid == port_info.port_item.dot1q_pvid_index)
                   )
                {
                    if (port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_dot1qTrunk)
                    {
                        trunk_mode = TRUE;
                        VLAN_MGR_SetVlanPortMode(lport, VAL_vlanPortMode_hybrid);
                    }
                    if (    (!VLAN_MGR_AddUntagPortMember(default_vlan_id, lport, vlan_status))
                        ||  (!VLAN_MGR_SetDot1qPvid(lport, default_vlan_id))
                       )
                    {
                        result = FALSE;
                    }
                    if (trunk_mode)
                    {
                        VLAN_MGR_SetVlanPortMode(lport, VAL_vlanPortMode_dot1qTrunk);
                    }
                }
                else if(    (VLAN_OM_IsPortVlanStaticMember(vid, lport))
                        &&  (port_info.port_item.auto_vlan_mode)
                        &&  (vid == port_info.port_item.admin_pvid)
                   )
                {
                    if (    (!VLAN_MGR_AddUntagPortMember(default_vlan_id, lport, vlan_status))
                        ||  (!VLAN_MGR_SetDot1qPvid(lport, default_vlan_id))
                       )
                    {
                        result = FALSE;
                    }
                }
            }
        }
    }
    if (result)
    {
        result = VLAN_MGR_DeleteNormalVlan(vid, vlan_status);
    }
    return result;

}/* End of VLAN_MGR_DeleteNormalVlanAgent() */

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
BOOL_T VLAN_MGR_AddEgressPortMemberAgent(UI32_T vid, UI32_T lport, UI32_T vlan_status, BOOL_T tagged)
{
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    UI32_T                              vid_ifindex;
    BOOL_T                              result;

    result = TRUE;
    if (!VLAN_OM_IsVlanExisted(vid))
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("\r\nERROR: VLAN_MGR_AddEgressPortMemberAgent: VLan is not existed");
        return FALSE;
    }
    else if (!VLAN_MGR_GetPortEntry(lport, &vlan_port_info))
    {
        return FALSE;
    }

    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

    if (tagged)
    {
        if (!VLAN_OM_IsPortVlanMember(vid_ifindex, lport))
        {
            if (!VLAN_MGR_AddEgressPortMember(vid, lport, vlan_status))
            {
                result = FALSE;
            }
        }
        else if (VLAN_OM_IsVlanUntagPortListMember(vid_ifindex, lport))
        {
            if (!VLAN_MGR_DeleteUntagPortMember(vid, lport, vlan_status))
            {
                result = FALSE;
            }
        }
    }
    else
    {
        if (!VLAN_MGR_AddUntagPortMember(vid, lport, vlan_status))
        {
            result = FALSE;
        }

#if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE)
        if ( result && (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_access))
        {
            result = VLAN_MGR_ConformToSingleUntaggedVlan(vid, lport, vlan_status);
        }
#endif /* #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE) */

#if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK == TRUE)
        if ( result && (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_hybrid))
        {
            result = VLAN_MGR_ConformToSingleUntaggedVlan(vid, lport, vlan_status);
        }
#endif /* #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK == TRUE) */
    }

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
    if (result)
    {
        /* To make sure this port doesn't join tagged VLAN
         */
        result = VLAN_MGR_ConformToSingleMode(vid, lport, vlan_status, tagged);
    }
#endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)*/
    return result;
}/* End of VLAN_MGR_AddEgressPortMemberAgent() */


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
BOOL_T VLAN_MGR_DeleteEgressPortMemberAgent(UI32_T vid, UI32_T lport, UI32_T vlan_status, BOOL_T tagged)
{
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    UI32_T                              vid_ifindex;
    BOOL_T                              result;
    UI32_T                              default_vlan_id;

    result = FALSE;
    if (!VLAN_OM_IsVlanExisted(vid))
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("\r\nERROR: VLAN_MGR_AddEgressPortMemberAgent: VLan is not existed");
        return FALSE;
    }
    else if (!VLAN_MGR_GetPortEntry(lport, &vlan_port_info))
    {
        return FALSE;
    }
    else if (!VLAN_OM_GetGlobalDefaultVlan_Ex(&default_vlan_id))
    {
        return FALSE;
    }

    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

    if (    (   (!tagged)
            &&  (VLAN_OM_IsVlanUntagPortListMember(vid_ifindex, lport))
            )
        ||  (   (tagged)
            &&  (VLAN_OM_IsPortVlanMember(vid_ifindex, lport))
            &&  (!VLAN_OM_IsVlanUntagPortListMember(vid_ifindex, lport))
            )
       )

    {
#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
        if (vid != default_vlan_id)
#endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)*/
        {
            if (VLAN_MGR_DeleteEgressPortMember(vid, lport, vlan_status))
            {
                result = TRUE;
            }
        }
    }

    return result;
}/* End of VLAN_MGR_DeleteEgressPortMemberAgent() */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_ProcessOrphanPortByLport
 * ------------------------------------------------------------------------
 * PURPOSE  : If a port is not in any this vlan, it will join default vlan with untagged member.
 * INPUT    : lport_ifindex -- logical port number
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None.
 * ------------------------------------------------------------------------
 */
BOOL_T VLAN_MGR_ProcessOrphanPortByLport(UI32_T  lport_ifindex)
{
    UI32_T                              default_vlan_id;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;

    /* BODY */
    if (!VLAN_MGR_GetPortEntry(lport_ifindex, &vlan_port_info))
    {
        return FALSE;
    }

    if (vlan_port_info.port_item.static_joined_vlan_count == 0)
    {
        VLAN_OM_GetGlobalDefaultVlan_Ex(&default_vlan_id);
        if (VLAN_MGR_AddUntagPortMember(default_vlan_id, lport_ifindex, VAL_dot1qVlanStatus_permanent) == FALSE)
        {
            return FALSE;
        }

        if (     (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_access)
              && (VLAN_MGR_SetDot1qPvid(lport_ifindex, default_vlan_id) == FALSE)
           )
        {
            return FALSE;
        }
    }
    return TRUE;
}/* End of VLAN_MGR_ProcessOrphanPortByLport() */

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
BOOL_T  VLAN_MGR_SetAuthorizedVlanList(UI32_T lport_ifindex, UI32_T pvid, VLAN_OM_VLIST_T *tagged_vlist, VLAN_OM_VLIST_T *untagged_vlist, BOOL_T is_guest)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    UI32_T                              temp_vid = 0;
    VLAN_OM_VLIST_T                     *authorized_vlist = NULL;
    VLAN_OM_VLIST_T                     *vlan;
    BOOL_T                              ret = FALSE;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;

    /* If a port is forbidden, it cannot be authorized
     */
    while (VLAN_OM_GetNextDot1qVlanCurrentEntry(0, &temp_vid, &vlan_info))
    {
        if (VLAN_OM_IsVlanForbiddenPortListMember(vlan_info.dot1q_vlan_index, lport_ifindex))
        {
            return FALSE;
        }
    }

    /* If a port is access/trunk mode, it cannot be authorized
     */
    if (!VLAN_MGR_GetPortEntry(lport_ifindex, &vlan_port_info))
    {
        return FALSE;
    }
    if (VAL_vlanPortMode_hybrid != vlan_port_info.vlan_port_entry.vlan_port_mode)
    {
        return FALSE;
    }

#if (SYS_CPNT_RSPAN == TRUE)
    /* If a port join RSPAN VLAN, it cannot be authorized */
    temp_vid = 0;
    memset(&vlan_info, 0, sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
    while (VLAN_OM_GetNextDot1qVlanCurrentEntry(0, &temp_vid, &vlan_info))
    {
        if (    (vlan_info.rspan_status == VAL_vlanStaticExtRspanStatus_rspanVlan)
             && VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex)
           )
        {
            return FALSE;
        }
    }
#endif /* end of #if (SYS_CPNT_RSPAN == TRUE) */

    if (    (   VLAN_TYPE_DOT1Q_NULL_VLAN_ID != pvid
             && VLAN_TYPE_DOT1Q_RESERVED_VLAN_ID != pvid
             && NULL == tagged_vlist
             && NULL == untagged_vlist
            )
        ||
            (    (      VLAN_TYPE_DOT1Q_NULL_VLAN_ID == pvid
                  &&    !(      NULL == tagged_vlist
                          &&    NULL == untagged_vlist
                         )
                 )
            )
       )
    {
        ret = FALSE;
    }
    else
    {
        if (VLAN_TYPE_DOT1Q_NULL_VLAN_ID == pvid && NULL == tagged_vlist && NULL == untagged_vlist)
        {
            ret = VLAN_MGR_ProcessUnauthPort(lport_ifindex);
        }
        else
        {
            vlan = tagged_vlist;
            while (vlan != NULL)
            {
                authorized_vlist = VLAN_MGR_AddVlanList(authorized_vlist, vlan->vlan_id);
                vlan = vlan->next;
            }
            vlan = untagged_vlist;
            while (vlan != NULL)
            {
                authorized_vlist = VLAN_MGR_AddVlanList(authorized_vlist, vlan->vlan_id);
                vlan = vlan->next;
            }

            vlan = authorized_vlist;
            while (vlan != NULL)
            {
                if (!VLAN_OM_IsVlanExisted(vlan->vlan_id))
                {
#if (SYS_CPNT_VLAN_DYNAMICALLY_CREATE_AUTO_VLAN == TRUE)
                    /* Create VLAN
                     */
                    if (!VLAN_MGR_CreateVlan(vlan->vlan_id, VAL_dot1qVlanStatus_other))
                    {
                        VLAN_MGR_FreeVlanList(authorized_vlist);
                        return FALSE;
                    }
                    if (!VLAN_MGR_SetDot1qVlanStaticRowStatus(vlan->vlan_id, VAL_dot1qVlanStaticRowStatus_active))
                    {
                        return FALSE;
                    }
#else
                    return FALSE;
#endif
                }
                vlan = vlan->next;
            }
            VLAN_MGR_FreeVlanList(authorized_vlist);

            /* Process authorized port
             */
            is_authenticating = TRUE;
            ret = VLAN_MGR_ProcessAuthPort(lport_ifindex, pvid, tagged_vlist, untagged_vlist, is_guest);
            is_authenticating = FALSE;
        }
    }

    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_GetAuthorizedVlanList
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the authorized vlan list is returned
 *            successfully.  Otherwise, returns false.
 * INPUT    : lport_ifindex     -- specify the lport
 * OUTPUT   : pvid              -- the authorized pvid
 *            tagged_vlist      -- the authorized tagged vlan list
 *            untagged_vlist    -- the authorized untagged vlan list
 * RETURN   : TRUE/FALSE
 * NOTES    : caller should free memory of tagged_vlist and untagged_vlist
 *--------------------------------------------------------------------------*/
BOOL_T  VLAN_MGR_GetAuthorizedVlanList(UI32_T lport_ifindex, UI32_T *pvid, VLAN_OM_VLIST_T *tagged_vlist, VLAN_OM_VLIST_T *untagged_vlist)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    UI32_T                              temp_vid = 0;
    VLAN_OM_VLIST_T                     *tmp_auth_tagged_vlan_list = NULL;
    VLAN_OM_VLIST_T                     *tmp_auth_untagged_vlan_list = NULL;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    tagged_vlist = NULL;
    untagged_vlist = NULL;

    vlan_port_info.lport_ifindex = lport_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    if (!vlan_port_info.port_item.auto_vlan_mode)
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    VLAN_IFINDEX_CONVERTTO_VID(vlan_port_info.port_item.dot1q_pvid_index, *pvid);
    vlan_info.dot1q_vlan_index = 0;
    while (VLAN_OM_GetNext_Vlan_With_PortJoined(lport_ifindex, &vlan_info))
    {
        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
        {
            tmp_auth_tagged_vlan_list = VLAN_MGR_AddVlanList(tmp_auth_tagged_vlan_list, temp_vid);
        }
        else if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex))
        {
            tmp_auth_untagged_vlan_list = VLAN_MGR_AddVlanList(tmp_auth_untagged_vlan_list, temp_vid);
        }
    }

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
}

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
BOOL_T VLAN_MGR_SetPortAutoVlanMode(UI32_T lport_ifindex, BOOL_T state)
{
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    vlan_port_info.lport_ifindex = lport_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    vlan_port_info.port_item.auto_vlan_mode = state;

    if(!VLAN_OM_SetVlanPortEntry(&vlan_port_info))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
} /* end of VLAN_MGR_SetPortAutoVlanMode() */

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
BOOL_T VLAN_MGR_SetGlobalDefaultVlan(UI32_T vid)
{
    BOOL_T                              ret = FALSE;
    UI32_T                              vid_ifindex;
    UI32_T                              gvrp_status;
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    UI32_T                              vlan_num;
    #if 0
    UI8_T                               cpu_mac[SYS_ADPT_MAC_ADDR_LEN];
    #endif
    /*added by Jinhua Wei ,to remove warning ,becaued above array never used*/

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (vid < 1 || vid > SYS_DFLT_DOT1QMAXVLANID)
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    if (    VLAN_OM_GetGlobalGvrpStatus(&gvrp_status)
         && VLAN_OM_GetCurrentConfigVlan(&vlan_num)
       )
    {
        if (VAL_dot1qGvrpStatus_enabled != gvrp_status)
        {
            VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
            vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;
            if (VLAN_OM_GetVlanEntry(&vlan_info))
            {
                if (SWCTRL_SetGlobalDefaultVlan(vid))
                {
                    VLAN_OM_SetGlobalDefaultVlan(vid);
                    ret = TRUE;
                }
            }
            else
            {
                if (vlan_num < SYS_ADPT_MAX_NBR_OF_VLAN)
                {
                    VLAN_MGR_DetailForCreateVlan(vid, VAL_dot1qVlanStatus_permanent, &vlan_info);

                    /* Clear the vlan port state table */
                    vlan_operstatus_changed[vid] = FALSE;

                    VLAN_OM_EnterCriticalSection();
                    SYS_TIME_GetSystemUpTimeByTick(VLAN_OM_GetIfTableLastUpdateTimePtr());
                    VLAN_OM_LeaveCriticalSection();

                    /* Pass vlan_info into FSM to check and update dot1q_vlan_static_row_status field.
                       If transition of dot1q_vlan_static_row_status is correct, then update vlan_info
                       into database.  Otherwise, return false.
                     */
                    switch (L_RSTATUS_Fsm(VAL_dot1qVlanStaticRowStatus_createAndGo,
                                          &vlan_info.dot1q_vlan_static_row_status,
                                          VLAN_MGR_SemanticCheck,
                                          (void*)&vlan_info))
                    {
                        case L_RSTATUS_NOTEXIST_2_ACTIVE:
                            if (    SWCTRL_CreateVlan(vid)
                                &&  SWCTRL_SetGlobalDefaultVlan(vid)
                               )
                            {
#if 0 /* VaiWang, Thursday, March 27, 2008 10:11:48 */
                                if (SWCTRL_GetCpuMac(cpu_mac) == FALSE)
                                {
                                    printf("\r\nSWCTRL_GetCpuMac() returns error!\r\n");
                                    break;
                                }
                                if (VLAN_MGR_CreateVlanDev(vid, cpu_mac) != VLAN_TYPE_RETVAL_OK)
                                {
                                    printf("\r\nVLAN_MGR_CreateVlanDev() returns error!\r\n");
                                    break;
                                }
#endif /*  ACCTON_METRO */

                                VLAN_OM_SetVlanEntry(&vlan_info);
                                VLAN_OM_SetGlobalDefaultVlan(vid);
                                ret = TRUE;

#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
                                VLAN_MGR_LocalAddPortTrunkLinkToVlan(vlan_info.dot1q_vlan_index);
#endif

                                VLAN_MGR_Notify_VlanCreate(vlan_info.dot1q_vlan_index, vlan_info.dot1q_vlan_status);
                                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, ret);
                            } /* end of if */
                            break;
                        default:
                            break;
                    } /* end of switch */
                } /* end of if (vlan_num < SYS_ADPT_MAX_NBR_OF_VLAN) */
            } /* end of else of if (VLAN_OM_GetVlanEntry(&vlan_info)) */
        } /* end of if (VAL_dot1qGvrpStatus_enabled != gvrp_status) */
    } /* end of if (VLAN_OM_GetGlobalGvrpStatus(&gvrp_status) && VLAN_OM_GetCurrentConfigVlan(&vlan_num)) */

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, ret);
} /* End of VLAN_MGR_SetGlobalDefaultVlan() */

#if (SYS_CPNT_MAC_VLAN == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetMacVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the MAC VLAN entry
 * INPUT    : mac_address   - only allow unitcast address
 *                            use 0 to get the first entry
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
BOOL_T VLAN_MGR_SetMacVlanEntry(UI8_T *mac_address, UI8_T *mask, UI16_T vid, UI8_T priority)
{
    VLAN_TYPE_MacVlanEntry_T mac_vlan_entry;
    I32_T next_idx= -1, prefix_len=0;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    /*if input mac_address is a null address, multicast address, or broadcast address, return FALSE*/
    if( IS_NULL_MAC(mac_address)
     || IS_MULTICAST_MAC(mac_address)
     || IS_BROADCAST_MAC(mac_address)
      )
    {
        return FALSE;
    }

    if(vid < 1 || vid > SYS_DFLT_DOT1QMAXVLANID)
    {
        return FALSE;
    }

    if(priority < SYS_ADPT_MIN_MAC_VLAN_ENTRY_PRIORITY || priority > SYS_ADPT_MAX_MAC_VLAN_ENTRY_PRIORITY)
    {
        return FALSE;
    }

    /*check mask*/
    {
       BOOL_T not_successive = FALSE;

       for(prefix_len=0; prefix_len<SYS_ADPT_MAC_ADDR_LEN*8; prefix_len++)
       {
           if((mask[prefix_len/8]&(0x80>>(prefix_len%8))) == 0)
           {
               not_successive  = TRUE;
           }
           else if(not_successive == TRUE)
           {
            return FALSE;
           }
       }
    }

    /*we only process mac after mask*/
    for(prefix_len=0; prefix_len<SYS_ADPT_MAC_ADDR_LEN; prefix_len++)
    {
        mac_address[prefix_len] = mac_address[prefix_len]&mask[prefix_len];
    }

    /*if input mac_address is a null address, multicast address, or broadcast address, return FALSE*/
    if( IS_NULL_MAC(mac_address)
     || IS_MULTICAST_MAC(mac_address)
     || IS_BROADCAST_MAC(mac_address)
      )
    {
        return FALSE;
    }
    /*already exist, we don't allow modify old one*/
    memcpy(mac_vlan_entry.mac_address, mac_address, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(mac_vlan_entry.mask, mask, SYS_ADPT_MAC_ADDR_LEN);
    if(TRUE == VLAN_OM_GetMacVlanEntry(&mac_vlan_entry))
    {
        if(mac_vlan_entry.vid != vid
           ||mac_vlan_entry.priority != priority)
        {
            VLAN_OM_DeleteMacVlanEntry(mac_address, mask);
            SWCTRL_DeleteMacVlanEntry(mac_vlan_entry.mac_address, mac_vlan_entry.mask);
        }
        else
        {
             return TRUE;
        }
    }

    /*first check table is not full*/
    if(TRUE == VLAN_OM_IsMacVlanTableFull())
    {
        return FALSE;
    }

    next_idx=-1;
    while(TRUE == VLAN_OM_GetNextMacVlanEntryByIndex(&next_idx, &mac_vlan_entry))
    {
        SWCTRL_DeleteMacVlanEntry(mac_vlan_entry.mac_address, mac_vlan_entry.mask);
    }

    if(FALSE == VLAN_OM_SetMacVlanEntry(mac_address, mask, vid, priority))
    {
         return FALSE;
    }

    /*third set chip again*/
    memset(&mac_vlan_entry, 0, sizeof(VLAN_TYPE_MacVlanEntry_T));
    while(TRUE == VLAN_OM_GetNextMacVlanEntry(&mac_vlan_entry))
    {
        SWCTRL_SetMacVlanEntry(mac_vlan_entry.mac_address, mac_vlan_entry.mask, mac_vlan_entry.vid, mac_vlan_entry.priority);
    }

    return TRUE;
}

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
BOOL_T VLAN_MGR_DeleteMacVlanEntry(UI8_T *mac_address, UI8_T *mask)
{
    I32_T prefix_len=0;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    /*if input mac_address is a null address, multicast address, or broadcast address, return FALSE*/
    if( IS_NULL_MAC(mac_address)
     || IS_MULTICAST_MAC(mac_address)
     || IS_BROADCAST_MAC(mac_address)
      )
    {
        return FALSE;
    }

    /*we only process mac after mask*/
    for(prefix_len=0; prefix_len<SYS_ADPT_MAC_ADDR_LEN; prefix_len++)
    {
        mac_address[prefix_len] = mac_address[prefix_len]&mask[prefix_len];
    }

    /*delete from chip first, then delete from om. if both succeed, return TRUE*/
    if( SWCTRL_DeleteMacVlanEntry(mac_address, mask)
     && VLAN_OM_DeleteMacVlanEntry(mac_address, mask)
      )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

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
BOOL_T VLAN_MGR_DeleteAllMacVlanEntry(void)
{
    VLAN_TYPE_MacVlanEntry_T    mac_vlan_entry;
    I32_T next_idx=-1;


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    /*get all MAC VLAN entries and delete them*/
    while(VLAN_OM_GetNextMacVlanEntryByIndex(&next_idx, &mac_vlan_entry))
    {
        /*delete from chip first, then delete from om. if both succeed, return TRUE*/
        if(!( SWCTRL_DeleteMacVlanEntry(mac_vlan_entry.mac_address, mac_vlan_entry.mask)
         && VLAN_OM_DeleteMacVlanEntry(mac_vlan_entry.mac_address, mac_vlan_entry.mask))
          )
        {
            return FALSE;
        }
    }

    return TRUE;
}
#endif /*end of #if (SYS_CPNT_MAC_VLAN == TRUE)*/

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
void VLAN_MGR_AddFirstTrunkMember_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T         vlan_info;
    VLAN_OM_Vlan_Port_Info_T                trunk_port_info;
    VLAN_OM_Vlan_Port_Info_T                member_port_info;
    BOOL_T                                  portlist_changed = FALSE;

    /* BODY */

    if (VLAN_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return;

    if (VLAN_Debug(VLAN_DEBUG_FLAG_TRUNK))
        BACKDOOR_MGR_Printf("VLAN_MGR_ProcessAddFirstTrunkMember_CallBack: trunk_ifindex: %d, member_ifindex %d\n",(UI16_T)trunk_ifindex,(UI16_T)member_ifindex);

    /* Get location of member_ifindex and trunk_ifindex in vlan dot1q_vlan_current_egress_ports.
     */
    vlan_info.dot1q_vlan_index = 0;

    /* VLAN INFO */

    /* When member_ifindex becomes the first trunk port, trunk_ifindex should join every vlan
       member_ifindex joined, and member_ifindex should be remove from every vlan it has previously
       joined.
     */
    while (1)
    {
        if (!VLAN_OM_GetNextVlanEntry(&vlan_info))
        {
            break;
        }

        portlist_changed = FALSE;

        /* Check vlan member list: add trunk_ifindex and remove member_ifindex
         */
        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, member_ifindex))
        {
            VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, trunk_ifindex);
            VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, member_ifindex);

            if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, member_ifindex))
            {
                VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, trunk_ifindex);
                VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, member_ifindex);
            }

            portlist_changed = TRUE;
        } /* end of if */

        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, member_ifindex))
        {
            VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, trunk_ifindex);
            VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, member_ifindex);

            if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, member_ifindex))
            {
                VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, trunk_ifindex);
                VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, member_ifindex);
            }

            portlist_changed = TRUE;
        } /* end of if */

        /* Check vlan forbidden list
         */
        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, member_ifindex))
        {
            VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, trunk_ifindex);
            VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, member_ifindex);
            portlist_changed = TRUE;
        } /* end of if */

        /* If vlan_info is successfully set into the database, then notify other component
           of the changes to the vlan member.  Else, an error has occured.
         */
        if (portlist_changed == TRUE)
        {
            if(!VLAN_OM_SetVlanEntry(&vlan_info))
            {
                return;
            }
            VLAN_MGR_Notify_AddFirstTrunkMember(vlan_info.dot1q_vlan_index, trunk_ifindex, member_ifindex);
        }
     } /* end of while */


    /* VLAN PORT INFO */

    /* lport_ifindex is the key to search for vlan_port_entry in the database.
     */
    trunk_port_info.lport_ifindex = trunk_ifindex;

    if (!VLAN_OM_GetVlanPortEntry(&trunk_port_info))
    {
        return;
    } /* end of if */

    member_port_info.lport_ifindex = member_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&member_port_info))
    {
        return;
    } /* end of if */

    /* member_ifindex will now be prohibited to join other vlan until it is remove from the trunk
     */
    member_port_info.port_trunk_mode = TRUE;

    /* trunk_ifindex will copy the attribute of member_ifindex when member_ifindex
       becomes the first member of this trunk.
     */
    memcpy(&trunk_port_info.port_item, &member_port_info.port_item, sizeof(VLAN_OM_Dot1qPortVlanEntry_T));
    memcpy(&trunk_port_info.garp_entry, &member_port_info.garp_entry, sizeof(VLAN_OM_Dot1dPortGarpEntry_T));
    memcpy(&trunk_port_info.vlan_port_entry, &member_port_info.vlan_port_entry, sizeof(VLAN_OM_VlanPortEntry_T));

#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
    if (member_port_info.vlan_port_entry.vlan_port_trunk_link_mode == VLAN_MGR_TRUNK_LINK)
    {
        member_port_info.vlan_port_entry.vlan_port_trunk_link_mode = VLAN_MGR_NOT_TRUNK_LINK;
    }
#endif

    trunk_port_info.port_item.dot1q_port_gvrp_status = member_port_info.port_item.dot1q_port_gvrp_status;
    member_port_info.port_item.dot1q_port_gvrp_status = VAL_dot1qPortGvrpStatus_disabled;

    if(!VLAN_OM_SetVlanPortEntry(&member_port_info))
    {
        return;
    } /* end of if */

    if(!VLAN_OM_SetVlanPortEntry(&trunk_port_info))
    {
        return;
    } /* end of if */

    /* Operatin Successful
     */

    /* notify other component of the changes to the vlan member have finished.
     */
    VLAN_MGR_Notify_FinishAddFirstTrunkMember(trunk_ifindex, member_ifindex);

    return;

} /* end of VLAN_MGR_AddFirstTrunkMember_CallBack()*/

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
void VLAN_MGR_AddTrunkMember_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    VLAN_OM_Vlan_Port_Info_T            member_port_info, trunk_port_info;
    UI32_T                              vid;
    BOOL_T                              portlist_changed = FALSE;
    BOOL_T                              is_port_vlan_member = FALSE;

    /* BODY */

    if (VLAN_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return;

    if (VLAN_Debug(VLAN_DEBUG_FLAG_TRUNK))
        BACKDOOR_MGR_Printf("VLAN_MGR_ProcessAddTrunkMember_CallBack: trunk_ifindex: %d, member_ifindex %d\n",(UI16_T)trunk_ifindex,(UI16_T)member_ifindex);

    /* VLAN INFO */

    /* dot1q_vlan_index is the key to search for vlan_info in the database.
     */
    vlan_info.dot1q_vlan_index = 0;

    trunk_port_info.lport_ifindex = trunk_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&trunk_port_info))
    {
        return;
    }

    member_port_info.lport_ifindex = member_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&member_port_info))
    {
        return;
    }

    /* When member_ifindex becomes a trunk port, it should be deleted from every
       vlan it has previously joined.
     */
    while (1)
    {
        if (!VLAN_OM_GetNextVlanEntry(&vlan_info))
        {
            break;
        }
        VLAN_IFINDEX_CONVERTTO_VID(vlan_info.dot1q_vlan_index, vid);

        portlist_changed = FALSE;
        is_port_vlan_member = FALSE;

        /* Check vlan member list: remove member_ifindex
         */
        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, member_ifindex))
        {
            VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, member_ifindex);

            if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, member_ifindex))
            {
                VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, member_ifindex);

                if (    !VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, trunk_ifindex))
                {
                    if (is_provision_complete == TRUE)
                    {
                        SWCTRL_DeleteTrunkMemberPortFromVlanUntaggedSet(member_ifindex, vid);
                    }
                }
            }

            if (    !VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, trunk_ifindex))
            {
                if (is_provision_complete == TRUE)
                {
                    SWCTRL_DeleteTrunkMemberPortFromVlanMemberSet(member_ifindex, vid);
                }
            }

            portlist_changed = TRUE;
            is_port_vlan_member = TRUE;
        }
        /*member not in this vlan, but trunk port in this vlan*/
        else if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, trunk_ifindex))
        {
            if (is_provision_complete == TRUE)
            {
                SWCTRL_AddTrunkMemberPortToVlanMemberSet(member_ifindex, vid);
            }

            if(VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, trunk_ifindex))
            {
                if (is_provision_complete == TRUE)
                {
                    SWCTRL_AddTrunkMembersPortToVlanUntaggedSet(member_ifindex, vid);
                }
            }
        }

        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, member_ifindex))
        {
            VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, member_ifindex);

            if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, member_ifindex))
            {
                VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, member_ifindex);
            }

            portlist_changed = TRUE;
        } /* end of if */

        /* Check vlan forbidden list
         */
        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, member_ifindex))
        {
            VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, member_ifindex);
            portlist_changed = TRUE;
        } /* end of if */

        if (portlist_changed == TRUE)
        {
            if(!VLAN_OM_SetVlanEntry(&vlan_info))
            {
                return;
            }

            VLAN_MGR_Notify_AddTrunkMember(vlan_info.dot1q_vlan_index, trunk_ifindex, member_ifindex);
            if (TRUE == is_port_vlan_member)
            {
                VLAN_MGR_Notify_VlanMemberDeleteByTrunk(vlan_info.dot1q_vlan_index, member_ifindex, VAL_dot1qVlanStatus_permanent);
            }
        }
    } /* end of while */

    /* VLAN PORT INFO */

    {/*set to chip the vlan information of the member, it shall follow trunk port*/
        UI32_T pvid=0;
        /* PVID */
        if (VLAN_IFINDEX_CONVERTTO_VID(trunk_port_info.port_item.dot1q_pvid_index, pvid))
        {
        /*EPR: ES3628BT-FLF-ZZ-00772
Problem:RIP: LACP member add/remove will cause L3 packets lost.
Rootcause:when sync trunk info to trunk member,it will fail.
          (1)because the port added to trunk ,it will set trunk member info in port_info.
          (2)when sync trunk member vlan attribute,and add/remove from chip,it will check if trunk member ,if it is trunk

member it will return FALSE.
          (3)when set pvid,it will add the port to the vlan,and remove it .At last ,the port will not in the native vlan
Solution: when add trunk member to vlan when syn the attribute ,not check trunk member
          just set pvid ,not remove the port
Files:vlan_mgr.c,swctrl.h,swctrl.c*/
            if (!VLAN_MGR_SetDot1qPvidWithoutChangingMembership(member_ifindex, pvid,FALSE))
            {
                return;
            }
        }

        /* Acceptable Frame Types */
        if (    trunk_port_info.port_item.dot1q_port_acceptable_frame_types
            ==  VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanTagged
           )
        {
            if (!SWCTRL_AdmitVLANTaggedFramesOnlyForTrunkMember(member_ifindex))
            {
                return;
            } /* End of if */
        }
        else if(    trunk_port_info.port_item.dot1q_port_acceptable_frame_types
            ==  VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanUnTagged
           )
        {
            if (!SWCTRL_AdmitVLANUntaggedFramesOnly(trunk_ifindex))
            {
                return;
            } /* End of if */
        }
        else
        {
            if (!SWCTRL_AdmitAllFramesForTrunkMember(member_ifindex))
            {
                return;
            } /* End of if */
        } /* End of if (_acceptable_frame_tagged_only_) */

        /* Ingress Filtering */
        if (    trunk_port_info.port_item.dot1q_port_ingress_filtering
            ==  VAL_dot1qPortIngressFiltering_true
           )
        {
            if (!SWCTRL_EnableIngressFilterForTrunkMember(member_ifindex) )
            {
                return;
            } /* End of if */
        }
        else
        {
            if (!SWCTRL_DisableIngressFilterForTrunkMember(member_ifindex) )
            {
                return;
            } /* End of if */
        } /* End of if (_ingress_filtering_) */
    }

    /* member port will inherit the property of trunk port when member port joins the trunk.
     */
    memcpy(&member_port_info.port_item, &trunk_port_info.port_item, sizeof(VLAN_OM_Dot1qPortVlanEntry_T));
    memcpy(&member_port_info.garp_entry, &trunk_port_info.garp_entry, sizeof(VLAN_OM_Dot1dPortGarpEntry_T));
    memcpy(&member_port_info.vlan_port_entry, &trunk_port_info.vlan_port_entry, sizeof(VLAN_OM_VlanPortEntry_T));

#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
    if (member_port_info.vlan_port_entry.vlan_port_trunk_link_mode == VLAN_MGR_TRUNK_LINK)
    {
        member_port_info.vlan_port_entry.vlan_port_trunk_link_mode = VLAN_MGR_NOT_TRUNK_LINK;
    }
#endif

    member_port_info.port_item.dot1q_port_gvrp_status = VAL_dot1qPortGvrpStatus_disabled;
    member_port_info.port_trunk_mode = TRUE;

    if (!VLAN_OM_SetVlanPortEntry(&member_port_info))
    {
        return;
    } /* end of if */

    /* notify other component of the changes to the vlan member have finished.
     */
    VLAN_MGR_Notify_FinishAddTrunkMember(trunk_ifindex, member_ifindex);

    return;
} /* end of VLAN_MGR_AddTrunkMember_CallBack()*/

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
void VLAN_MGR_DeleteTrunkMember_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    VLAN_OM_Vlan_Port_Info_T            member_port_info, trunk_port_info;
    BOOL_T                              portlist_changed = FALSE;
    /* BODY */

    if (VLAN_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return;

    if (VLAN_Debug(VLAN_DEBUG_FLAG_TRUNK))
        BACKDOOR_MGR_Printf("VLAN_MGR_ProcessDeleteTrunkMember_CallBack: trunk_ifindex: %d, member_ifindex %d\n",(UI16_T)trunk_ifindex,(UI16_T)member_ifindex);

    /* VLAN INFO */

    /* dot1q_vlan_index is the key to search for vlan_info in the database.
     */
    vlan_info.dot1q_vlan_index = 0;

   /* When member_ifindex is remove from a trunk port, it will join every vlan
      trunk_ifindex previously joined.
    */
    while (1)
    {
        if (!VLAN_OM_GetNextVlanEntry(&vlan_info))
        {
            break;
        }

        portlist_changed = FALSE;

       /* Check vlan member list: member_ifindex will join every VLAN trunk_ifindex joins.
         */
        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, trunk_ifindex))
        {
            VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, member_ifindex);

            if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, trunk_ifindex))
            {
                VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, member_ifindex);
            }

            portlist_changed = TRUE;
        } /* end of if */

        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, trunk_ifindex))
        {
            VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, member_ifindex);

            if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, trunk_ifindex))
            {
                VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, member_ifindex);
            }

            portlist_changed = TRUE;
        } /* end of if */

        /* Check vlan forbidden list
         */
        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, trunk_ifindex))
        {
            VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, member_ifindex);
            portlist_changed = TRUE;
        }

        if (portlist_changed == TRUE)
        {
            if(!VLAN_OM_SetVlanEntry(&vlan_info))
            {
                return;
            }

            VLAN_MGR_Notify_DeleteTrunkMember(vlan_info.dot1q_vlan_index, trunk_ifindex, member_ifindex);

        } /* end of if */
    } /* end of while */

    /* VLAN_PORT_INFO */

    /* lport_ifindex is the key to search for vlan_port_info in the database.
     */
    member_port_info.lport_ifindex = member_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&member_port_info))
    {
        return;
    } /* end of if */

    trunk_port_info.lport_ifindex = trunk_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&trunk_port_info))
    {
        return;
    } /* end of if */

    /* member_ifindex will inherit the attribute of trunk_ifindex when it is removed from trunk.
     */
    memcpy(&member_port_info.port_item, &trunk_port_info.port_item, sizeof(VLAN_OM_Dot1qPortVlanEntry_T));
    memcpy(&member_port_info.garp_entry, &trunk_port_info.garp_entry, sizeof(VLAN_OM_Dot1dPortGarpEntry_T));
    memcpy(&member_port_info.vlan_port_entry, &trunk_port_info.vlan_port_entry, sizeof(VLAN_OM_VlanPortEntry_T));

    /* Indication that member_ifindex is no longer a trunk member
     */
    member_port_info.port_trunk_mode = FALSE;
    member_port_info.port_item.dot1q_port_gvrp_status = trunk_port_info.port_item.dot1q_port_gvrp_status;

    if (!VLAN_OM_SetVlanPortEntry(&member_port_info))
    {
        return;
    } /* end of if */

    /* notify other component of the changes to the vlan member have finished.
     */
    VLAN_MGR_Notify_FinishDeleteTrunkMember(trunk_ifindex, member_ifindex);

    return;

} /* end of VLAN_MGR_DeleteTrunkMember_CallBack() */

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
void VLAN_MGR_DeleteLastTrunkMember_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    VLAN_OM_Vlan_Port_Info_T            member_port_info, trunk_port_info;
    UI16_T                              vlan_id;
    BOOL_T                              portlist_changed[SYS_ADPT_MAX_VLAN_ID] = {FALSE};
    /* BODY */

    if (VLAN_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return;

    if (VLAN_Debug(VLAN_DEBUG_FLAG_TRUNK))
        BACKDOOR_MGR_Printf("VLAN_MGR_ProcessDeleteLastTrunkMember_CallBack: trunk_ifindex: %d, member_ifindex %d\n",(UI16_T)trunk_ifindex,(UI16_T)member_ifindex);

    /* VLAN INFO */

    /* dot1q_vlan_index is the key to search for vlan_port_info in the database.
     */
    vlan_info.dot1q_vlan_index = 0;

    /* When the last trunk member is removed, member_ifindex should join every vlan
       trunk_ifindex joins.  trunk_ifindex should be remove from every vlan it has
       previously joined.
     */
    while (1)
    {
        if (!VLAN_OM_GetNextVlanEntry(&vlan_info))
        {
            break;
        }
        VLAN_IFINDEX_CONVERTTO_VID(vlan_info.dot1q_vlan_index, vlan_id);
        if(vlan_id==0)
        {
            printf("%s(%d): Incorrect vlan ifindex:%d\n", __FUNCTION__, __LINE__, (int)(vlan_info.dot1q_vlan_index));
            continue;
        }

        portlist_changed[vlan_id-1] = FALSE;

        /* Check vlan member list: remove trunk_ifindex and add member_ifindex
         */
        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, trunk_ifindex))
        {
            VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, trunk_ifindex);
            VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, member_ifindex);

            if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, trunk_ifindex))
            {
                VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, trunk_ifindex);
                VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, member_ifindex);
            }

            portlist_changed[vlan_id-1] = TRUE;
        } /* end of if */

        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, trunk_ifindex))
        {
            VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, trunk_ifindex);
            VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, member_ifindex);

            if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, trunk_ifindex))
            {
                VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, trunk_ifindex);
                VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, member_ifindex);;
            }

            portlist_changed[vlan_id-1] = TRUE;
        } /* end of if */

        /* Check vlan forbidden list
         */
        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, trunk_ifindex))
        {
            VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, trunk_ifindex);
            VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, member_ifindex);
            portlist_changed[vlan_id-1] = TRUE;
        }

        if (portlist_changed[vlan_id-1] == TRUE)
        {
            if (!VLAN_OM_SetVlanEntry(&vlan_info))
            {
                return;
            }

            VLAN_MGR_Notify_DeleteLastTrunkMember(vlan_info.dot1q_vlan_index, trunk_ifindex, member_ifindex);
        }
     } /* end of while */


    /* VLAN_PORT_INFO */

    /* lport_ifindex is the key to search for vlan_port_info in the database.
     */
    member_port_info.lport_ifindex = member_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&member_port_info))
    {
        return;
    }

    trunk_port_info.lport_ifindex = trunk_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&trunk_port_info))
    {
        return;
    }

    /* member_ifindex will inherit the attribute value of trunk_ifindex
     */
    memcpy(&member_port_info.port_item, &trunk_port_info.port_item, sizeof(VLAN_OM_Dot1qPortVlanEntry_T));
    memcpy(&member_port_info.garp_entry, &trunk_port_info.garp_entry, sizeof(VLAN_OM_Dot1dPortGarpEntry_T));
    memcpy(&member_port_info.vlan_port_entry, &trunk_port_info.vlan_port_entry, sizeof(VLAN_OM_VlanPortEntry_T));

#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
    if (trunk_port_info.vlan_port_entry.vlan_port_trunk_link_mode == VLAN_MGR_TRUNK_LINK)
    {
        trunk_port_info.vlan_port_entry.vlan_port_trunk_link_mode = VLAN_MGR_NOT_TRUNK_LINK;
    }
#endif

    member_port_info.port_trunk_mode = FALSE;
    trunk_port_info.port_trunk_mode = FALSE;

    if (!VLAN_OM_SetVlanPortEntry(&member_port_info))
    {
        return;
    }

    if (!VLAN_OM_SetVlanPortEntry(&trunk_port_info))
    {
        return;
    }

    /* notify other component of the changes to the vlan member have finished.
     */
    VLAN_MGR_Notify_FinishDeleteLastTrunkMember(trunk_ifindex, member_ifindex);

    vlan_info.dot1q_vlan_index = 0;
    while (TRUE)
    {
        if (!VLAN_OM_GetNextVlanEntry(&vlan_info))
        {
            break;
        }

        VLAN_IFINDEX_CONVERTTO_VID(vlan_info.dot1q_vlan_index, vlan_id);
        if(vlan_id==0)
        {
            printf("%s(%d): Incorrect vlan ifindex:%d\n", __FUNCTION__, __LINE__, (int)(vlan_info.dot1q_vlan_index));
            continue;
        }

        if (portlist_changed[vlan_id-1] == TRUE)
        {
            if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
            {
                if ((vlan_info.if_entry.vlan_operation_status == VAL_ifOperStatus_up))
                {
                    int vid,i;
                    UI8_T   active_uport_count_per_unit[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
                    UI8_T   port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
                    int     total_active_uport_count;

                    VLAN_IFINDEX_CONVERTTO_VID(vlan_info.dot1q_vlan_index,vid);
                    if (SWCTRL_LportListToActiveUportList(vid,vlan_info.dot1q_vlan_current_egress_ports,
                                                           active_uport_count_per_unit, port_list) == TRUE)
                    {
                        total_active_uport_count = 0;

                        for (i = 0; i < SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
                        {
                             total_active_uport_count += active_uport_count_per_unit[i];
                        }

                        if (total_active_uport_count == 0)
                        {
                            VLAN_MGR_SET_IF_OPER_STATUS(vlan_info.if_entry, VAL_ifOperStatus_down);
                            VLAN_OM_SetVlanEntry(&vlan_info);
                            VLAN_MGR_SEND_IF_OPER_STATUS_CHANGED_EVENT(vid);
                        }
                    }
                }
            }
        }
    }
    return;

} /* end of VLAN_MGR_DeleteLastTrunkMember_CallBack()*/

/*----------------------------------------------------------------------------------
 * FUNCTION NAME: VLAN_MGR_SetGroupThreadId
 *----------------------------------------------------------------------------------
 * Purpose: Give the thread ID for VLAN_MGR to send event.
 * Input:   thread_id -- the mgr thread ID of CSC group which VLAN joins.
 * Output:  None.
 * Return:  None.
 *----------------------------------------------------------------------------------
 */
void VLAN_MGR_SetGroupThreadId(UI32_T thread_id)
{
    cscgroup_thread_id = thread_id;
}

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
BOOL_T VLAN_MGR_SetVoiceVlanId(UI32_T vvid)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;
    UI32_T vvid_ifindex;

    if (vvid > SYS_ADPT_MAX_VLAN_ID)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetVoiceVlanId_Fun_No,
                                    EH_TYPE_MSG_FAILED_TO_SET,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN ID "
                                 );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    if (vvid != 0)
    {
        VLAN_VID_CONVERTTO_IFINDEX(vvid, vvid_ifindex);
        vlan_info.dot1q_vlan_index = vvid_ifindex;
        if (VLAN_OM_GetVlanEntry(&vlan_info) == FALSE)
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);

        if (vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_dynamicGvrp)
        {
            vlan_info.dot1q_vlan_status = VAL_dot1qVlanStatus_permanent;
            if (!VLAN_OM_SetVlanEntry(&vlan_info))
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_SetVoiceVlanId_Fun_No,
                                            EH_TYPE_MSG_FAILED_TO_SET,
                                            SYSLOG_LEVEL_INFO,
                                            "VLAN entry "
                                        );
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
            } /* end of if */
        }
    }

    voice_vlan_id = vvid;
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
} /* end of VLAN_MGR_SetVoiceVlanId() */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetPortVlanList
 *-----------------------------------------------------------------------------
 * PURPOSE : Set the VLAN membership as the given VLAN list for a logical port.
 * INPUT   : lport       - the specified logical port
 *           vlan_list_p - pointer to list of VLAN IDs for the lport to set
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : 1. all untagged for hybrid mode, all tagged for trunk mode,
 *              untagged on largest VID for access mode
 *           2. fail if remove from PVID VLAN for access/hybrid mode
 *-----------------------------------------------------------------------------
 */
BOOL_T  VLAN_MGR_SetPortVlanList(UI32_T lport, UI8_T *vlan_list_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_entry;
    VLAN_OM_VlanPortEntry_T         vlan_port_entry;
    UI32_T                          i, j, vid;

    /* BODY
     */

    if (SWCTRL_LogicalPortExisting(lport) == FALSE)
    {
        return FALSE;
    }

    if (vlan_list_p == NULL)
    {
        return FALSE;
    }

    if (VLAN_OM_GetVlanPortEntryByIfindex(lport, &vlan_port_entry) == FALSE)
    {
        return FALSE;
    }

    /* add to VLANs in vlan_list for hybrid/trunk mode */
    vid = 0;
    for (i = 0; i < VLAN_TYPE_VLAN_LIST_SIZE; i++)
    {
        if (vlan_list_p[i] == 0)
        {
            continue;
        }

        for (j = 0; j < 8; j++)
        {
            if (vlan_list_p[i] & (1 << (7-j)))
            {
                vid = i * 8 + j + 1;

#if (SYS_CPNT_VLAN_AUTO_CREATE_STATIC_VLAN == TRUE)
                if (VLAN_OM_IsVlanExisted(vid) == FALSE)
                {
                    if (VLAN_MGR_CreateVlan(vid, VAL_dot1qVlanStatus_permanent)
                            == FALSE)
                    {
                        return FALSE;
                    }
                    if (VLAN_MGR_SetDot1qVlanStaticRowStatus(vid,
                            VAL_dot1qVlanStaticRowStatus_active) == FALSE)
                    {
                        return FALSE;
                    }
                }
#endif

                if (vlan_port_entry.vlan_port_mode ==
                        VAL_vlanPortMode_dot1qTrunk)
                {
                    if (VLAN_MGR_AddEgressPortMember(vid, lport,
                            VAL_dot1qVlanStatus_permanent) == FALSE)
                    {
                        return FALSE;
                    }
                }
                else if (vlan_port_entry.vlan_port_mode ==
                            VAL_vlanPortMode_hybrid)
                {
                    if (VLAN_MGR_AddUntagPortMember(vid, lport,
                            VAL_dot1qVlanStatus_permanent) == FALSE)
                    {
                        return FALSE;
                    }
                }
                else
                {
                    continue;
                }
            } /* if the bit on */
        } /* for j < 8 */
    } /* for i < VLAN_TYPE_VLAN_LIST_SIZE */

    /* add to VLAN with largest VID in vlan_list for access mode */
    if (    (vid != 0)
         && (vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_access)
       )
    {
        if (VLAN_MGR_AddUntagPortMember(vid, lport,
                VAL_dot1qVlanStatus_permanent) == FALSE)
        {
            return FALSE;
        }
    }

    /* remove from VLANs not in vlan_list */
    vid = 0;
    while (VLAN_OM_GetNextDot1qVlanCurrentEntry_With_PortJoined(0, &vid, lport,
                &vlan_entry) == TRUE)
    {
        if (L_CVRT_IS_MEMBER_OF_PORTLIST(vlan_list_p, vid) == FALSE)
        {
            if (VLAN_MGR_DeleteEgressPortMember(vid, lport,
                    VAL_dot1qVlanStatus_permanent) == FALSE)
            {
                return FALSE;
            }
        }
    }

    return TRUE;
} /* End of VLAN_MGR_SetPortVlanList */

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
BOOL_T VLAN_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    VLAN_MGR_IpcMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (VLAN_MGR_IpcMsg_T*)msgbuf_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msg_p->type.ret_bool = FALSE;
        msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    /* dispatch IPC message and call the corresponding VLAN_MGR function
     */
    switch (msg_p->type.cmd)
    {
        case VLAN_MGR_IPC_SETDOT1QGVRPSTATUS:
            msg_p->type.ret_bool =
                VLAN_MGR_SetDot1qGvrpStatus(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETDOT1QCONSTRAINTTYPEDEFAULT:
            msg_p->type.ret_bool =
                VLAN_MGR_SetDot1qConstraintTypeDefault(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_CREATEVLAN:
            msg_p->type.ret_bool = VLAN_MGR_CreateVlan(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_DELETEVLAN:
            msg_p->type.ret_bool = VLAN_MGR_DeleteVlan(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_DELETENORMALVLAN:
            msg_p->type.ret_bool = VLAN_MGR_DeleteNormalVlan(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETDOT1QVLANSTATICNAME:
            msg_p->type.ret_bool = VLAN_MGR_SetDot1qVlanStaticName(
                msg_p->data.arg_grp2.arg1, msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_ADDEGRESSPORTMEMBER:
            msg_p->type.ret_bool = VLAN_MGR_AddEgressPortMember(
                msg_p->data.arg_grp3.arg1, msg_p->data.arg_grp3.arg2,
                msg_p->data.arg_grp3.arg3);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_ADDEGRESSPORTMEMBERFORGVRP:
            msg_p->type.ret_bool = VLAN_MGR_AddEgressPortMemberForGVRP(
                msg_p->data.arg_grp3.arg1, msg_p->data.arg_grp3.arg2,
                msg_p->data.arg_grp3.arg3);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_DELETEEGRESSPORTMEMBER:
            msg_p->type.ret_bool = VLAN_MGR_DeleteEgressPortMember(
                msg_p->data.arg_grp3.arg1, msg_p->data.arg_grp3.arg2,
                msg_p->data.arg_grp3.arg3);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETDOT1QVLANSTATICEGRESSPORTS:
            msg_p->type.ret_bool = VLAN_MGR_SetDot1qVlanStaticEgressPorts(
                msg_p->data.arg_grp4.arg1, msg_p->data.arg_grp4.arg2,
                msg_p->data.arg_grp4.arg3);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_ADDFORBIDDENEGRESSPORTMEMBER:
            msg_p->type.ret_bool = VLAN_MGR_AddForbiddenEgressPortMember(
                msg_p->data.arg_grp3.arg1, msg_p->data.arg_grp3.arg2,
                msg_p->data.arg_grp3.arg3);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_DELETEFORBIDDENEGRESSPORTMEMBER:
            msg_p->type.ret_bool = VLAN_MGR_DeleteForbiddenEgressPortMember(
                msg_p->data.arg_grp3.arg1, msg_p->data.arg_grp3.arg2,
                msg_p->data.arg_grp3.arg3);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETDOT1QVLANFORBIDDENEGRESSPORTS:
            msg_p->type.ret_bool = VLAN_MGR_SetDot1qVlanForbiddenEgressPorts(
                msg_p->data.arg_grp4.arg1, msg_p->data.arg_grp4.arg2,
                msg_p->data.arg_grp4.arg3);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_ADDUNTAGPORTMEMBER:
            msg_p->type.ret_bool = VLAN_MGR_AddUntagPortMember(
                msg_p->data.arg_grp3.arg1, msg_p->data.arg_grp3.arg2,
                msg_p->data.arg_grp3.arg3);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_DELETEUNTAGPORTMEMBER:
            msg_p->type.ret_bool = VLAN_MGR_DeleteUntagPortMember(
                msg_p->data.arg_grp3.arg1, msg_p->data.arg_grp3.arg2,
                msg_p->data.arg_grp3.arg3);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETDOT1QVLANSTATICUNTAGGEDPORTS:
            msg_p->type.ret_bool = VLAN_MGR_SetDot1qVlanStaticUntaggedPorts(
                msg_p->data.arg_grp4.arg1, msg_p->data.arg_grp4.arg2,
                msg_p->data.arg_grp4.arg3);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETDOT1QVLANSTATICROWSTATUS:
            msg_p->type.ret_bool = VLAN_MGR_SetDot1qVlanStaticRowStatus(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETDOT1QVLANSTATICROWSTATUSFORGVRP:
            msg_p->type.ret_bool = VLAN_MGR_SetDot1qVlanStaticRowStatusForGVRP(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETVLANADDRESSMETHOD:
            msg_p->type.ret_bool = VLAN_MGR_SetVlanAddressMethod(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETDOT1QVLANSTATICENTRY:
            msg_p->type.ret_bool =
                VLAN_MGR_SetDot1qVlanStaticEntry(&msg_p->data.arg_vlan_entry);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETVLANADMINSTATUS:
            msg_p->type.ret_bool = VLAN_MGR_SetVlanAdminStatus(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETLINKUPDOWNTRAPENABLED:
            msg_p->type.ret_bool = VLAN_MGR_SetLinkUpDownTrapEnabled(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETDOT1QPVID:
            msg_p->type.ret_bool = VLAN_MGR_SetDot1qPvid(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETDOT1QPORTACCEPTABLEFRAMETYPES:
            msg_p->type.ret_bool = VLAN_MGR_SetDot1qPortAcceptableFrameTypes(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETDOT1QINGRESSFILTER:
            msg_p->type.ret_bool = VLAN_MGR_SetDot1qIngressFilter(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETVLANPORTMODE:
            msg_p->type.ret_bool = VLAN_MGR_SetVlanPortMode(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETDOT1QPORTGVRPSTATUSENABLED:
            msg_p->type.ret_bool = VLAN_MGR_SetDot1qPortGvrpStatusEnabled(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETDOT1QPORTGVRPFAILEDREGISTRATIONS:
            msg_p->type.ret_bool =
                VLAN_MGR_SetDot1qPortGvrpFailedRegistrations(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETDOT1QPORTGVRPLASTPDUORIGIN:
            msg_p->type.ret_bool = VLAN_MGR_SetDot1qPortGvrpLastPduOrigin(
                msg_p->data.arg_grp_ui32_mac.arg_ui32,
                msg_p->data.arg_grp_ui32_mac.arg_mac);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_GETPORTENTRY:
            msg_p->type.ret_bool = VLAN_MGR_GetPortEntry(
                msg_p->data.arg_grp5.arg1, &msg_p->data.arg_grp5.arg2);
            msgbuf_p->msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp5);
            break;

        case VLAN_MGR_IPC_GETVLANPORTENTRY:
            msg_p->type.ret_bool = VLAN_MGR_GetVlanPortEntry(
                msg_p->data.arg_grp6.arg1, &msg_p->data.arg_grp6.arg2);
            msgbuf_p->msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp6);
            break;

        case VLAN_MGR_IPC_GETNEXTVLANPORTENTRY:
            msg_p->type.ret_bool = VLAN_MGR_GetNextVlanPortEntry(
                &msg_p->data.arg_grp6.arg1, &msg_p->data.arg_grp6.arg2);
            msgbuf_p->msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp6);
            break;

        case VLAN_MGR_IPC_GETDOT1QPORTVLANENTRY:
            msg_p->type.ret_bool = VLAN_MGR_GetDot1qPortVlanEntry(
                msg_p->data.arg_grp7.arg1, &msg_p->data.arg_grp7.arg2);
            msgbuf_p->msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp7);
            break;

        case VLAN_MGR_IPC_GETNEXTDOT1QPORTVLANENTRY:
            msg_p->type.ret_bool = VLAN_MGR_GetNextDot1qPortVlanEntry(
                &msg_p->data.arg_grp7.arg1, &msg_p->data.arg_grp7.arg2);
            msgbuf_p->msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp7);
            break;

        case VLAN_MGR_IPC_SETMANAGEMENTVLAN:
            msg_p->type.ret_bool =
                VLAN_MGR_SetManagementVlan(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETGLOBALMANAGEMENTVLAN:
            msg_p->type.ret_bool =
                VLAN_MGR_SetGlobalManagementVlan(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_LEAVEMANAGEMENTVLAN:
            msg_p->type.ret_bool =
                VLAN_MGR_LeaveManagementVlan(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETIPINTERFACE:
            msg_p->type.ret_bool =
                VLAN_MGR_SetIpInterface(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETVLANMGMTIPSTATE:
            msg_p->type.ret_bool = VLAN_MGR_SetVlanMgmtIpState(
                msg_p->data.arg_grp8.arg1, msg_p->data.arg_grp8.arg2,
                msg_p->data.arg_grp8.arg3);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_GETVLANMAC:
            msg_p->type.ret_bool = VLAN_MGR_GetVlanMac(
                msg_p->data.arg_grp11.arg1, msg_p->data.arg_grp11.arg2);
            msgbuf_p->msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp11);
            break;

        case VLAN_MGR_IPC_NOTIFYFORWARDINGSTATE:
            VLAN_MGR_NotifyForwardingState(msg_p->data.arg_grp19.arg1,
                msg_p->data.arg_grp19.arg2, msg_p->data.arg_grp19.arg3);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

#if (SYS_CPNT_VLAN_AUTO_JOIN_VLAN_FOR_PVID == TRUE)
        case VLAN_MGR_IPC_SETNATIVEVLANAGENT:
            msg_p->type.ret_bool = VLAN_MGR_SetNativeVlanAgent(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;
#endif /*(SYS_CPNT_VLAN_AUTO_JOIN_VLAN_FOR_PVID == TRUE)*/

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)

        case VLAN_MGR_IPC_ENABLEPORTDUALMODE:
            msg_p->type.ret_bool = VLAN_MGR_EnablePortDualMode(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_DISABLEPORTDUALMODE:
            msg_p->type.ret_bool =
                VLAN_MGR_DisablePortDualMode(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_GETRUNNINGPORTDUALMODE:
            msg_p->type.ret_ui32 = VLAN_MGR_GetRunningPortDualMode(
                msg_p->data.arg_grp12.arg1, &msg_p->data.arg_grp12.arg2,
                &msg_p->data.arg_grp12.arg3);
            msgbuf_p->msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp12);
            break;

        case VLAN_MGR_IPC_GETDOT1QVLANSTATICENTRYAGENT:
            msg_p->type.ret_bool = VLAN_MGR_GetDot1qVlanStaticEntryAgent(
                msg_p->data.arg_grp13.arg1, &msg_p->data.arg_grp13.arg2);
            msgbuf_p->msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp13);
            break;

        case VLAN_MGR_IPC_GETDOT1QVLANCURRENTENTRYAGENT:
            msg_p->type.ret_bool = VLAN_MGR_GetDot1qVlanCurrentEntryAgent(
                msg_p->data.arg_grp14.arg1, msg_p->data.arg_grp14.arg2,
                &msg_p->data.arg_grp14.arg3);
            msgbuf_p->msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp14);
            break;

        case VLAN_MGR_IPC_GETNEXTDOT1QVLANCURRENTENTRYAGENT:
            msg_p->type.ret_bool = VLAN_MGR_GetNextDot1qVlanCurrentEntryAgent(
                msg_p->data.arg_grp14.arg1, &msg_p->data.arg_grp14.arg2,
                &msg_p->data.arg_grp14.arg3);
            msgbuf_p->msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp14);
            break;

#endif /* SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE */

        case VLAN_MGR_IPC_DELETENORMALVLANAGENT:
            msg_p->type.ret_bool = VLAN_MGR_DeleteNormalVlanAgent(
                msg_p->data.arg_grp1.arg1, msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_ADDEGRESSPORTMEMBERAGENT:
            msg_p->type.ret_bool = VLAN_MGR_AddEgressPortMemberAgent(
                msg_p->data.arg_grp18.arg1, msg_p->data.arg_grp18.arg2,
                msg_p->data.arg_grp18.arg3, msg_p->data.arg_grp18.arg4);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_DELETEEGRESSPORTMEMBERAGENT:
            msg_p->type.ret_bool = VLAN_MGR_DeleteEgressPortMemberAgent(
                msg_p->data.arg_grp18.arg1, msg_p->data.arg_grp18.arg2,
                msg_p->data.arg_grp18.arg3, msg_p->data.arg_grp18.arg4);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETAUTHORIZEDVLANLIST:
            msg_p->type.ret_bool = VLAN_MGR_SetAuthorizedVlanList(
                msg_p->data.arg_grp_ui32_ui32_vlist_vlist_bool.arg_ui32_1,
                msg_p->data.arg_grp_ui32_ui32_vlist_vlist_bool.arg_ui32_2,
                msg_p->data.arg_grp_ui32_ui32_vlist_vlist_bool.arg_vlist_1,
                msg_p->data.arg_grp_ui32_ui32_vlist_vlist_bool.arg_vlist_2,
                msg_p->data.arg_grp_ui32_ui32_vlist_vlist_bool.arg_bool);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETGLOBALDEFAULTVLAN:
            msg_p->type.ret_bool =
                VLAN_MGR_SetGlobalDefaultVlan(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_SETVOICEVLANID:
            msg_p->type.ret_bool =
                VLAN_MGR_SetVoiceVlanId(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VLAN_MGR_IPC_GEtPORTLISTBYVID:
            msg_p->type.ret_bool = VLAN_MGR_GetPortlistByVid(
                msg_p->data.arg_grp4.arg1, &msg_p->data.arg_grp4.arg3,
                msg_p->data.arg_grp4.arg2);
            msgbuf_p->msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp4);
            break;
        case VLAN_MGR_IPC_GETVLANMEMBERBYLPORT:
            msg_p->type.ret_bool = VLAN_MGR_GetVLANMemberByLport(
                msg_p->data.arg_grp20.arg1, &msg_p->data.arg_grp20.arg3,
                msg_p->data.arg_grp20.arg2);
            msgbuf_p->msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp20);
            break;
        case VLAN_MGR_IPC_CHANGEl2IF2L3IF :
            msg_p->type.ret_ui32 =
                VLAN_MGR_VlanChangeToL3Type(msg_p->data.arg_grp3.arg1,&msg_p->data.arg_grp3.arg2,&msg_p->data.arg_grp3.arg3);
            msgbuf_p->msg_size = VLAN_MGR_GET_MSG_SIZE(arg_grp3);
            break;

        case VLAN_MGR_IPC_CHANGEl3IF2L2IF:
            msg_p->type.ret_ui32 =
                VLAN_MGR_VlanChangeToL2Type(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;
        case VLAN_MGR_IPC_L3VLANLOGICALMACCHANGE:
            msg_p->type.ret_ui32 =
                VLAN_MGR_VlanLogicalMacChange(msg_p->data.arg_grp11.arg1,msg_p->data.arg_grp11.arg2);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;
        case VLAN_MGR_IPC_SETDOT1QVLANALIAS:
            msg_p->type.ret_bool = VLAN_MGR_SetDot1qVlanAlias(
                msg_p->data.arg_alias.vid, msg_p->data.arg_alias.alias);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

#if (SYS_CPNT_MAC_VLAN == TRUE)
        case VLAN_MGR_IPC_SETMACVLANENTRY:
            msg_p->type.ret_bool = VLAN_MGR_SetMacVlanEntry(
            msg_p->data.arg_mac_vid_pri.arg_mac, msg_p->data.arg_mac_vid_pri.arg_mask,
            msg_p->data.arg_mac_vid_pri.arg_vid, msg_p->data.arg_mac_vid_pri.arg_pri);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;
        case VLAN_MGR_IPC_DELETEMACVLANENTRY:
            msg_p->type.ret_bool = VLAN_MGR_DeleteMacVlanEntry(
            msg_p->data.arg_mac_vid_pri.arg_mac,
            msg_p->data.arg_mac_vid_pri.arg_mask);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;
        case VLAN_MGR_IPC_DELETEALLMACVLANENTRY:
            msg_p->type.ret_bool = VLAN_MGR_DeleteAllMacVlanEntry();
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

#endif

        case VLAN_MGR_IPC_SETPORTVLANLIST:
            msg_p->type.ret_bool = VLAN_MGR_SetPortVlanList(
                msg_p->data.arg_ui32_vlist.arg_ui32,
                msg_p->data.arg_ui32_vlist.arg_vlist);
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
            break;

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_bool = FALSE;
            msgbuf_p->msg_size = VLAN_MGR_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of VLAN_MGR_HandleIPCReqMsg */


/* LOCAL SUBPROGRAM IMPLEMENTATION
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_InitDefaultVlanEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Disable the vlan operation while in slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------- */
static void VLAN_MGR_InitDefaultVlanEntry(void)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    UI32_T                              port_num;
    Port_Info_T                         port_info;
    #if 0
    UI8_T                               cpu_mac[SYS_ADPT_MAC_ADDR_LEN];
    #endif
    /*added by Jinhua Wei ,to remove warning ,becaued above array never used*/

    /* BODY */

    memset (&vlan_info, 0, sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
    memset (&vlan_port_info, 0, sizeof(VLAN_OM_Vlan_Port_Info_T));

    /* Set global gvrp status
     */
    if (!VLAN_OM_SetGlobalGvrpStatus(SYS_DFLT_1Q_GVRP_STATUS))
        return;

    if (!VLAN_OM_SetDot1qConstraintTypeDefault(VAL_dot1qConstraintTypeDefault_independent))
        return;

    VLAN_OM_SetGlobalDefaultVlan(VLAN_MGR_DOT1Q_DEFAULT_PVID);

    /* 1. Initialize default value to default vlan
       2. By default, this trap status should have the value enabled(1) for
          interfaces which do not operate on 'top' of any other
          interface (as defined in the ifStackTable), and disabled(2)
          otherwise."
     */
    /* dot1qVlanStaticEntry
     */
    SYS_TIME_GetSystemUpTimeByTick(&vlan_info.dot1q_vlan_creation_time);
    vlan_info.dot1q_vlan_time_mark = vlan_info.dot1q_vlan_creation_time;
    vlan_info.dot1q_vlan_index = VLAN_MGR_DOT1Q_DEFAULT_PVID_IFINDEX;
    vlan_info.dot1q_vlan_fdb_id = VLAN_MGR_DOT1Q_DEFAULT_PVID;
    vlan_info.dot1q_vlan_status = VAL_dot1qVlanStatus_permanent;
    vlan_info.dot1q_vlan_static_row_status = L_RSTATUS_NOT_EXIST;

    /* RFC2863
     */
    vlan_info.if_entry.admin_status = SYS_DFLT_IF_ADMIN_STATUS;
    vlan_info.if_entry.link_up_down_trap_enabled = SYS_DFLT_IF_LINK_UP_DOWN_TRAP_ENABLE;
    vlan_info.if_entry.vlan_operation_status = VAL_ifOperStatus_down;

    VLAN_MGR_LinkUpDownTrapEnabled  = SYS_DFLT_IF_LINK_UP_DOWN_TRAP_ENABLE;

    /* Es3626a Private Mib
     */
    vlan_info.vlan_address_method = SYS_DFLT_VLAN_ADDRESS_METHOD;

    /* Management vlan is set to vlan 1 by default
     */
    vlan_info.vlan_ip_state = VLAN_MGR_IP_STATE_IPV4;

    VLAN_OM_SetManagementVlanId(SYS_DFLT_SWITCH_MANAGEMENT_VLAN);

    vlan_info.if_entry.ifType= VLAN_DEFAULT_IFTYPE;

    if (SWCTRL_GetCpuMac(vlan_info.cpu_mac) == FALSE)
    {
        return;
    };

    /* For chip that created vlan 1 before vlan_mgr enters mastermode shall not return
       false if vlan already existed.
     */

    if (!SWCTRL_CreateVlan(VLAN_MGR_DOT1Q_DEFAULT_PVID))
    {
        VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING,FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_InitDefaultVlanEntry");
        return;
    } /* end of if */

#if 0 /* VaiWang, Thursday, March 27, 2008 10:12:01 */
    if (SWCTRL_GetCpuMac(cpu_mac) == FALSE)
    {
        printf("\r\nSWCTRL_GetCpuMac() returns error!\r\n");
        return;
    }
    if (VLAN_MGR_CreateVlanDev(VLAN_MGR_DOT1Q_DEFAULT_PVID, cpu_mac) != VLAN_TYPE_RETVAL_OK)
    {
        printf("\r\nVLAN_MGR_CreateVlanDev() returns error!\r\n");
        return;
    }
#endif /*  ACCTON_METRO */

    /* Initialize default value to all vlan_port_info
     */
    for (port_num=0; SWCTRL_GetNextPortInfo(&port_num, &port_info);)
    {
        VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, port_num);
        VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, port_num);
        VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, port_num);
        VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, port_num);

        vlan_port_info.lport_ifindex = port_num;
        vlan_port_info.port_trunk_mode = FALSE;
        /* Dot1qPortVlanEntry
         */
        vlan_port_info.port_item.dot1q_pvid_index = VLAN_MGR_DOT1Q_DEFAULT_PVID_IFINDEX;
        vlan_port_info.port_item.dot1q_port_acceptable_frame_types = SYS_DFLT_1Q_PORT_ACCEPTABLE_FRAME_TYPES;
        vlan_port_info.port_item.dot1q_port_ingress_filtering = SYS_DFLT_1Q_PORT_INGRESS_FILTERING;
        vlan_port_info.port_item.dot1q_port_gvrp_status = SYS_DFLT_1Q_PORT_GVRP_STATUS;
        vlan_port_info.port_item.dot1q_port_gvrp_failed_registrations = 0;
        memset(&vlan_port_info.port_item.dot1q_port_gvrp_last_pdu_origin, 0, SIZE_dot1qPortGvrpLastPduOrigin);
        vlan_port_info.port_item.admin_pvid = VLAN_MGR_DOT1Q_DEFAULT_PVID;
        vlan_port_info.port_item.admin_acceptable_frame_types = SYS_DFLT_1Q_PORT_ACCEPTABLE_FRAME_TYPES;
        vlan_port_info.port_item.admin_ingress_filtering = SYS_DFLT_1Q_PORT_INGRESS_FILTERING;
        vlan_port_info.port_item.auto_vlan_mode = FALSE;
        vlan_port_info.port_item.static_joined_vlan_count = 1;
        vlan_port_info.port_item.untagged_joined_vlan_count = 1;

        /* Dot1dPortGarpEntry
         */
        vlan_port_info.garp_entry.dot1d_port_garp_join_time = SYS_DFLT_1D_PORT_GARP_JOIN_TIME;
        vlan_port_info.garp_entry.dot1d_port_garp_leave_time = SYS_DFLT_1D_PORT_GARP_LEAVE_TIME;
        vlan_port_info.garp_entry.dot1d_port_garp_leave_all_time = SYS_DFLT_1D_PORT_GARP_LEAVE_ALL_TIME;

        /* Es3626
         */
        vlan_port_info.vlan_port_entry.vlan_port_mode =SYS_DFLT_VLAN_PORT_MODE;

#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
        /* Default port trunk link mode
         */
        vlan_port_info.vlan_port_entry.vlan_port_trunk_link_mode = VLAN_MGR_NOT_TRUNK_LINK;
#endif
#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
        vlan_port_info.vlan_port_entry.vlan_port_dual_mode  = FALSE;
        vlan_port_info.vlan_port_entry.dual_mode_vlan_id    = 0;
#endif
        if(!VLAN_OM_SetVlanPortEntry(&vlan_port_info))
            return;

#if (SYS_CPNT_REFINE_ISC_MSG == FALSE)
        /*  Default value is all port just join the untagged set of default vlan
         */
        if (is_provision_complete == TRUE)
        {
            if(!VLAN_MGR_AddVlanMemberToChip(port_num, VLAN_MGR_DOT1Q_DEFAULT_PVID, FALSE, TRUE))
            {
                VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING,FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_InitDefaultVlanEntry");
                return;
            } /* end of if */
        }

        if (is_provision_complete == TRUE)
        {
            if(!VLAN_MGR_AddVlanMemberToChip(port_num, VLAN_MGR_DOT1Q_DEFAULT_PVID, TRUE, TRUE))
            {
                VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_InitDefaultVlanEntry");
                return;
            } /* end of if */
        }

        if (!SWCTRL_SetPortPVID(port_num, VLAN_MGR_DOT1Q_DEFAULT_PVID, TRUE))
        {
            VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_InitDefaultVlanEntry");
            return;
        } /* end of if */

        if (SYS_DFLT_1Q_PORT_INGRESS_FILTERING == VAL_dot1qPortIngressFiltering_true)
        {
            if (!SWCTRL_EnableIngressFilter(port_num))
            {
                VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_InitDefaultVlanEntry");
                return;
            } /* end of if */
        }
        else
        {
            if (!SWCTRL_DisableIngressFilter(port_num))
            {
                VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_InitDefaultVlanEntry");
                return;
            } /* end of if */
        } /* end of if */

        if (!SWCTRL_AdmitAllFrames(port_num))
        {
            VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_InitDefaultVlanEntry");
            return;
        } /* end of if */
#endif /* #if (SYS_CPNT_REFINE_ISC_MSG == FALSE) */
    } /* end of for */

#if (SYS_CPNT_REFINE_ISC_MSG == TRUE)
    if (VLAN_MGR_RefineInitDefaultVlanEntry(vlan_info.dot1q_vlan_current_egress_ports))
    {
        VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_InitDefaultVlanEntry");
        return;
    }
#endif

    memset (&vlan_port_info, 0, sizeof(VLAN_OM_Vlan_Port_Info_T));
    for (port_num = SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER; port_num < SYS_ADPT_RS232_1_IF_INDEX_NUMBER; port_num++)
    {
        vlan_port_info.lport_ifindex = port_num;

        if (!VLAN_OM_SetVlanPortEntry(&vlan_port_info))
        {
            return;
        }

        continue;
    } /* end of for */

    /* Pass vlan_info into FSM to check and update dot1q_vlan_static_row_status field.
       If transition of dot1q_vlan_static_row_status is correct, then update vlan_info
       into database.  Otherwise, return false.
     */
    switch (L_RSTATUS_Fsm(VAL_dot1qVlanStaticRowStatus_createAndGo,
                          &vlan_info.dot1q_vlan_static_row_status,
                          VLAN_MGR_SemanticCheck, &vlan_info))
    {
        case L_RSTATUS_NOTEXIST_2_ACTIVE:
        case L_RSTATUS_NOTEXIST_2_NOTREADY:
            if(!VLAN_OM_SetVlanEntry(&vlan_info))
                return;
            break;
        default:
            VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, SWITCH_TO_DEFAULT_MESSAGE_INDEX, "VLAN_MGR_InitDefaultVlanEntry");
            break;
    } /* end of switch */
    if (SWCTRL_IsAnyLportOperUp(vlan_info.dot1q_vlan_current_egress_ports))
    {
        VLAN_MGR_SET_IF_OPER_STATUS(vlan_info.if_entry, VAL_ifOperStatus_up);
        VLAN_OM_SetVlanEntry(&vlan_info);
        VLAN_MGR_SEND_IF_OPER_STATUS_CHANGED_EVENT(VLAN_MGR_DOT1Q_DEFAULT_PVID);
    }
    return;

} /* end of VLAN_MGR_InitDefaultVlanEntry() */

/* NOTIFY FUNCTIONS
 */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Notify_AddFirstTrunkMember
 *------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when the first member port is added to a
 *            trunk port.
 * INPUT    : dot1q_vlan_index     -- specify which vlan the member_ifindex port joined.
 *          : trunk_ifindex   -- specify the trunk port ifindex
 *            member_ifindex  -- specify the member port that joined trunking.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static void VLAN_MGR_Notify_AddFirstTrunkMember(UI32_T dot1q_vlan_index, UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    if (VLAN_Debug(VLAN_DEBUG_FLAG_NOTIFY))
    {
        BACKDOOR_MGR_Printf("\nVLAN_MGR_Notify_AddFirstTrunkMember: vid_ifindex %d, trunk_ifindex %d, member_ifindex %d\n",
                                                     (UI16_T) dot1q_vlan_index,(UI16_T) trunk_ifindex,(UI16_T)member_ifindex );
    }

    SYS_CALLBACK_MGR_VlanAddFirstTrunkMemberCallback(SYS_MODULE_VLAN,
        dot1q_vlan_index, trunk_ifindex, member_ifindex);

    return;

} /* End of VLAN_MGR_Notify_AddFirstTrunkMember() */


/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Notify_AddTrunkMember
 *------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when member port is added to a trunk port
 * INPUT    : dot1q_vlan_index     -- specify which vlan the member_ifindex port joined.
 *          : trunk_ifindex   -- specify the trunk port ifindex
 *            member_ifindex  -- specify the member port that joined trunking.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static void VLAN_MGR_Notify_AddTrunkMember(UI32_T dot1q_vlan_index, UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    if (VLAN_Debug(VLAN_DEBUG_FLAG_NOTIFY))
    {
        BACKDOOR_MGR_Printf("\nVLAN_MGR_Notify_AddTrunkMember: vid_ifindex %d, trunk_ifindex %d, member_ifindex %d\n",
                                                (UI16_T) dot1q_vlan_index,(UI16_T) trunk_ifindex,(UI16_T)member_ifindex );
    }

    SYS_CALLBACK_MGR_VlanAddTrunkMemberCallback(SYS_MODULE_VLAN,
        dot1q_vlan_index, trunk_ifindex, member_ifindex);

    return;
} /* End of VLAN_MGR_Notify_AddTrunkMember() */


/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Notify_DeleteTrunkMember
 *------------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when a member port is deleted from a trunk port.
 * INPUT    : dot1q_vlan_index     -- specify which vlan the trunk_ifindex port joined.
 *          : trunk_ifindex   -- specify the trunk port ifindex
 *            member_ifindex  -- specify the member port that is going to be
 *                             remove from the trunk.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------------*/
static void VLAN_MGR_Notify_DeleteTrunkMember(UI32_T dot1q_vlan_index, UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    if (VLAN_Debug(VLAN_DEBUG_FLAG_NOTIFY))
    {
        BACKDOOR_MGR_Printf("\nVLAN_MGR_Notify_DeleteTrunkMember: vid_ifindex %d, trunk_ifindex %d, member_ifindex %d\n",
                                                   (UI16_T) dot1q_vlan_index,(UI16_T) trunk_ifindex,(UI16_T)member_ifindex );
    }

    SYS_CALLBACK_MGR_VlanDeleteTrunkMemberCallback(SYS_MODULE_VLAN,
        dot1q_vlan_index, trunk_ifindex, member_ifindex);

    return;
} /* End of VLAN_MGR_Notify_DeleteTrunkMember() */


/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Notify_DeleteLastTrunkMember
 *------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when the last member port is remove from
 *            the trunk port.
 * INPUT    : void (*fun) () -- specify what's function to register
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : 1. Design for trunk
 *------------------------------------------------------------------------------*/
static void VLAN_MGR_Notify_DeleteLastTrunkMember(UI32_T dot1q_vlan_index, UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    if (VLAN_Debug(VLAN_DEBUG_FLAG_NOTIFY))
    {
        BACKDOOR_MGR_Printf("\nVLAN_MGR_Notify_DeleteLastTrunkMember: vid_ifindex %d, trunk_ifindex %d, member_ifindex %d\n",
                                                       (UI16_T) dot1q_vlan_index,(UI16_T) trunk_ifindex,(UI16_T)member_ifindex );
    }

    SYS_CALLBACK_MGR_VlanDeleteLastTrunkMemberCallback(SYS_MODULE_VLAN,
        dot1q_vlan_index, trunk_ifindex, member_ifindex);

    return;
} /* End of VLAN_MGR_Notify_DeleteLastTrunkMember() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Notify_FinishAddFirstTrunkMember
 *------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when the first member port is added to a
 *            trunk port.
 * INPUT    : dot1q_vlan_index     -- specify which vlan the member_ifindex port joined.
 *          : trunk_ifindex   -- specify the trunk port ifindex
 *            member_ifindex  -- specify the member port that joined trunking.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static void VLAN_MGR_Notify_FinishAddFirstTrunkMember(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    if (VLAN_Debug(VLAN_DEBUG_FLAG_NOTIFY))
    {
        BACKDOOR_MGR_Printf("\nVLAN_MGR_Notify_FinishAddFirstTrunkMember: trunk_ifindex %d, member_ifindex %d\n",
                                                     (UI16_T) trunk_ifindex, (UI16_T)member_ifindex );
    }

    SYS_CALLBACK_MGR_VlanFinishAddFirstTrunkMemberCallback(SYS_MODULE_VLAN,
        trunk_ifindex, member_ifindex);

    return;
} /* End of VLAN_MGR_Notify_FinishAddFirstTrunkMember() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Notify_FinishAddTrunkMember
 *------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when member port is added to a trunk port
 * INPUT    : dot1q_vlan_index     -- specify which vlan the member_ifindex port joined.
 *          : trunk_ifindex   -- specify the trunk port ifindex
 *            member_ifindex  -- specify the member port that joined trunking.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static void VLAN_MGR_Notify_FinishAddTrunkMember(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    if (VLAN_Debug(VLAN_DEBUG_FLAG_NOTIFY))
    {
        BACKDOOR_MGR_Printf("\nVLAN_MGR_Notify_FinishAddTrunkMember: trunk_ifindex %d, member_ifindex %d\n",
                                                (UI16_T) trunk_ifindex, (UI16_T)member_ifindex );
    }

    SYS_CALLBACK_MGR_VlanFinishAddTrunkMemberCallback(SYS_MODULE_VLAN,
        trunk_ifindex, member_ifindex);

    return;
} /* End of VLAN_MGR_Notify_FinishAddTrunkMember() */


/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Notify_FinishDeleteTrunkMember
 *------------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when a member port is deleted from a trunk port.
 * INPUT    : dot1q_vlan_index     -- specify which vlan the trunk_ifindex port joined.
 *          : trunk_ifindex   -- specify the trunk port ifindex
 *            member_ifindex  -- specify the member port that is going to be
 *                             remove from the trunk.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------------*/
static void VLAN_MGR_Notify_FinishDeleteTrunkMember(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    if (VLAN_Debug(VLAN_DEBUG_FLAG_NOTIFY))
    {
        BACKDOOR_MGR_Printf("\nVLAN_MGR_Notify_FinishDeleteTrunkMember: trunk_ifindex %d, member_ifindex %d\n",
                                                   (UI16_T) trunk_ifindex, (UI16_T)member_ifindex );
    }

    SYS_CALLBACK_MGR_VlanFinishDeleteTrunkMemberCallback(SYS_MODULE_VLAN,
        trunk_ifindex, member_ifindex);

    return;
} /* End of VLAN_MGR_Notify_FinishDeleteTrunkMember() */


/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Notify_FinishDeleteLastTrunkMember
 *------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when the last member port is remove from
 *            the trunk port.
 * INPUT    : void (*fun) () -- specify what's function to register
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : 1. Design for trunk
 *------------------------------------------------------------------------------*/
static void VLAN_MGR_Notify_FinishDeleteLastTrunkMember(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    if (VLAN_Debug(VLAN_DEBUG_FLAG_NOTIFY))
    {
        BACKDOOR_MGR_Printf("\nVLAN_MGR_Notify_FinishDeleteLastTrunkMember: trunk_ifindex %d, member_ifindex %d\n",
                                                       (UI16_T) trunk_ifindex, (UI16_T)member_ifindex );
    }

    SYS_CALLBACK_MGR_VlanFinishDeleteLastTrunkMemberCallback(SYS_MODULE_VLAN,
        trunk_ifindex, member_ifindex);

    return;
} /* End of VLAN_MGR_Notify_FinishDeleteLastTrunkMember() */


/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Notify_VlanPortModeChanged
 *------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when vlan port mode has been changed to
 *            access mode.
 * INPUT    : void (*fun) () -- specify what's function to register
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : 1. Design ACD to disable gvrp in case of vlan_port_mode in access mode.
 *------------------------------------------------------------------------------*/
static void VLAN_MGR_Notify_VlanPortModeChanged(UI32_T lport_ifindex, UI32_T vlan_port_mode)
{
    if (VLAN_Debug(VLAN_DEBUG_FLAG_NOTIFY))
    {
        BACKDOOR_MGR_Printf("\nVLAN_MGR_Notify_VlanPortModeChanged: port %d, vlan_port_mode %d\n",
                                                      (UI16_T)  lport_ifindex,(UI16_T) vlan_port_mode);
    }

    SYS_CALLBACK_MGR_VlanPortModeCallback(SYS_MODULE_VLAN, lport_ifindex, vlan_port_mode);

    return;
} /* End of VLAN_MGR_Notify_DeleteLastTrunkMember() */


/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Notify_VlanCreate
 *------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when a vlan is created.
 * INPUT    : dot1q_vlan_index -- specify which vlan has just been created
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static void VLAN_MGR_Notify_VlanCreate(UI32_T vid_ifindex, UI32_T vlan_status)
{
    if (VLAN_Debug(VLAN_DEBUG_FLAG_NOTIFY))
    {
        BACKDOOR_MGR_Printf("\nVLAN_MGR_Notify_VlanCreate: vid_ifindex %lu, vlan_status %lu\n", (unsigned long)vid_ifindex, (unsigned long)vlan_status);
    }

    /* do not notify GVRP_GROUP */
    SYS_CALLBACK_MGR_VlanCreateCallback(SYS_MODULE_VLAN, vid_ifindex, vlan_status);

    /* only notify GVRP_GROUP */
    if (vlan_status != VAL_dot1qVlanStatus_dynamicGvrp)
        SYS_CALLBACK_MGR_VlanCreateForGVRPCallback(SYS_MODULE_VLAN, vid_ifindex, vlan_status);

    return;
} /* End of VLAN_MGR_Notify_VlanCreate()*/

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Notify_VlanSuspendToActive
 *------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when a vlan is from suspended to active.
 * INPUT    : vid_ifindex
 *            vlan_status
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static void VLAN_MGR_Notify_VlanSuspendToActive(UI32_T vid_ifindex, UI32_T vlan_status)
{
    if (VLAN_Debug(VLAN_DEBUG_FLAG_NOTIFY))
    {
        BACKDOOR_MGR_Printf("\r\n VLAN_MGR_Notify_VlanSuspendToActive: vid_ifindex %lu, vlan_status %lu \r\n", (unsigned long)vid_ifindex, (unsigned long)vlan_status);
    }

    SWCTRL_GROUP_VlanSuspendToActiveCallbackHandler(vid_ifindex, vlan_status);

    /* do not notify GVRP_GROUP */
    SYS_CALLBACK_MGR_VlanCreateCallback(SYS_MODULE_VLAN, vid_ifindex, vlan_status);

    /* only notify GVRP_GROUP */
    if (vlan_status != VAL_dot1qVlanStatus_dynamicGvrp)
        SYS_CALLBACK_MGR_VlanCreateForGVRPCallback(SYS_MODULE_VLAN, vid_ifindex, vlan_status);

    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Notify_VlanDestroy
 *------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when a vlan is deleted.
 * INPUT    : dot1q_vlan_index -- specify which vlan has just been destroyed
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static void VLAN_MGR_Notify_VlanDestroy(VLAN_OM_Dot1qVlanCurrentEntry_T * vlan_entry, UI32_T vlan_status)
{
    if (VLAN_Debug(VLAN_DEBUG_FLAG_NOTIFY))
    {
        BACKDOOR_MGR_Printf("\nVLAN_MGR_Notify_VlanDestroy: vid_ifindex %lu, vlan_status %lu\n", (unsigned long)vlan_entry->dot1q_vlan_index, (unsigned long)vlan_status);
    }
    if(vlan_entry->if_entry.ifType == VLAN_L3_IP_IFTYPE){
        SYS_CALLBACK_MGR_L3VlanDestroyCallback(SYS_MODULE_VLAN,vlan_entry->dot1q_vlan_index,vlan_status);
    }

    /* do not notify GVRP_GROUP */
    SYS_CALLBACK_MGR_VlanDestroyCallback(SYS_MODULE_VLAN,vlan_entry->dot1q_vlan_index, vlan_status);

    /* only notify GVRP_GROUP */
    if (vlan_status != VAL_dot1qVlanStatus_dynamicGvrp)
        SYS_CALLBACK_MGR_VlanDestroyForGVRPCallback(SYS_MODULE_VLAN,vlan_entry->dot1q_vlan_index, vlan_status);

    return;
} /* End of VLAN_MGR_Notify_VlanDestroy ()*/

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Notify_VlanActiveToSuspend
 *------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when a vlan is suspended.
 * INPUT    : dot1q_vlan_index -- specify which vlan has just been suspended
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static void VLAN_MGR_Notify_VlanActiveToSuspend(VLAN_OM_Dot1qVlanCurrentEntry_T * vlan_entry, UI32_T vlan_status)
{
    if (VLAN_Debug(VLAN_DEBUG_FLAG_NOTIFY))
    {
        BACKDOOR_MGR_Printf("VLAN_MGR_Notify_VlanActiveToSuspend: vid_ifindex %lu, vlan_status %lu\n", (unsigned long)vlan_entry->dot1q_vlan_index, (unsigned long)vlan_status);
    }

    if((vlan_entry->if_entry.ifType == VLAN_L3_IP_IFTYPE) &&(vlan_entry->if_entry.vlan_operation_status == VAL_ifOperStatus_up)){
        SYS_CALLBACK_MGR_L3IfOperStatusChangedCallback(SYS_MODULE_VLAN, vlan_entry->dot1q_vlan_index, VAL_ifAdminStatus_down);
    }

    SWCTRL_GROUP_VlanActiveToSuspendCallbackHandler(vlan_entry->dot1q_vlan_index, vlan_status);

    /* do not notify GVRP_GROUP */
    SYS_CALLBACK_MGR_VlanDestroyCallback(SYS_MODULE_VLAN,vlan_entry->dot1q_vlan_index, vlan_status);

    /* only notify GVRP_GROUP */
    if (vlan_status != VAL_dot1qVlanStatus_dynamicGvrp)
        SYS_CALLBACK_MGR_VlanDestroyForGVRPCallback(SYS_MODULE_VLAN,vlan_entry->dot1q_vlan_index, vlan_status);

    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Notify_VlanMemberAdd
 *------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when a lport is added to a vlan's member set.
 * INPUT    : dot1q_vlan_index     -- specify which vlan's member set to be add to
 *          : lport_ifindex   -- specify which port to be join to the member set
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static void VLAN_MGR_Notify_VlanMemberAdd(UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T vlan_status)
{
   UI32_T gvrp_status;
    if (VLAN_Debug(VLAN_DEBUG_FLAG_NOTIFY))
    {
        BACKDOOR_MGR_Printf("\nVLAN_MGR_Notify_VlanMemberAdd: vid_ifindex %d, lport_ifindex %d, vlan_status %d\n",
                                               (UI16_T) vid_ifindex,(UI16_T) lport_ifindex,(UI16_T) vlan_status);
    }

    /* do not notify GVRP_GROUP */
    SYS_CALLBACK_MGR_VlanMemberAddCallback(SYS_MODULE_VLAN, vid_ifindex,
        lport_ifindex, vlan_status);

    /* only notify GVRP_GROUP */
    if (vlan_status != VAL_dot1qVlanStatus_dynamicGvrp)
    {
    /*
    todo:add send event to gvrp,and gvrp process event
    ES3628BT-FLF-ZZ-00507
    Problem: Dut crash after copy start run which contains 4093 vlans
    RootCause:Gvrp cannot process the port add to vlan msg quickly
    Solution: it is temp solution,not send msg to gvrp.Event will send to gvrp later
    File:VLAN_mgr.c

*/
         VLAN_OM_GetDot1qGvrpStatus(&gvrp_status);
         if(gvrp_status == VAL_dot1qGvrpStatus_enabled)
           SYS_CALLBACK_MGR_VlanMemberAddForGVRPCallback(SYS_MODULE_VLAN, vid_ifindex, lport_ifindex, vlan_status);
    }
    return;
} /* End of VLAN_MGR_Notify_VlanMemberAdd ()*/


/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Notify_VlanMemberDelete
 *------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when a port is remove from vlan's member set.
 * INPUT    : dot1q_vlan_index     -- specify which vlan's member set to be deleted
 *                              from
 *          : lport_ifindex   -- specify which port to be deleted.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static void VLAN_MGR_Notify_VlanMemberDelete(UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T vlan_status)
{
    UI32_T gvrp_status;
    if (VLAN_Debug(VLAN_DEBUG_FLAG_NOTIFY))
    {
        BACKDOOR_MGR_Printf("\nVLAN_MGR_Notify_VlanMemberDelete: vid_ifindex %d, lport_ifindex %d, vlan_status %d\n",
                                                 (UI16_T)  vid_ifindex, (UI16_T)lport_ifindex, (UI16_T)vlan_status);
    }

    /* do not notify GVRP_GROUP */
    SYS_CALLBACK_MGR_VlanMemberDeleteCallback(SYS_MODULE_VLAN, vid_ifindex,
        lport_ifindex, vlan_status);

    /* only notify GVRP_GROUP */
    if (vlan_status != VAL_dot1qVlanStatus_dynamicGvrp)
    {
    /*
    todo:add send event to gvrp,and gvrp process event
    ES3628BT-FLF-ZZ-00507
    Problem: Dut crash after copy start run which contains 4093 vlans
    RootCause:Gvrp cannot process the port add to vlan msg quickly
    Solution: it is temp solution,not send msg to gvrp.Event will send to gvrp later
    File:VLAN_mgr.c

*/
        VLAN_OM_GetDot1qGvrpStatus(&gvrp_status);
        if(gvrp_status == VAL_dot1qGvrpStatus_enabled)
          SYS_CALLBACK_MGR_VlanMemberDeleteForGVRPCallback(SYS_MODULE_VLAN, vid_ifindex, lport_ifindex, vlan_status);
    }
    return;
} /* End of VLAN_MGR_Notify_VlanMemberDelete() */


/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Notify_PvidChange
 *------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when pvid of a port changes
 * INPUT    : lport_ifindex - the specific port this modification is of.
 *            old_pvid - previous pvid before modification
 *            new_pvid - new and current pvid after modification
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static void VLAN_MGR_Notify_PvidChange(UI32_T lport_ifindex, UI32_T old_pvid, UI32_T new_pvid)
{
    if (VLAN_Debug(VLAN_DEBUG_FLAG_NOTIFY))
    {
        BACKDOOR_MGR_Printf("\nVLAN_MGR_Notify_PvidChanged: lport_ifindex %d, old_pvid %d, new_pvid %d\n",
                                                   (UI16_T)lport_ifindex, (UI16_T)old_pvid, (UI16_T)new_pvid);
    }

    SYS_CALLBACK_MGR_PvidChangeCallback(SYS_MODULE_VLAN, lport_ifindex,
        old_pvid, new_pvid);

    return;
} /* end of VLAN_MGR_Notify_PvidChange() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Notify_IfOperStatusChanged
 *------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when vlan's operation status has changed.
 * INPUT    : dot1q_vlan_index     -- specify which vlan's status changed
 *          : oper_status          -- specify the new status of vlan
 *                                 VAL_ifLinkUpDownTrapEnable_enabled  (1)
 *                                 VAL_ifLinkUpDownTrapEnable_disabled (2)
 *          : trap_enabled         -- specify whether the link up/down trap is enabled
 *                                 VAL_ifLinkUpDownTrapEnable_enabled
 *                                 VAL_ifLinkUpDownTrapEnable_disabled
 *          : admin_status         -- specify the admin status of vlan if entry
 *                                 VAL_ifAdminStatus_up
 *                                 VAL_ifAdminStatus_down
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static void VLAN_MGR_Notify_IfOperStatusChanged(VLAN_OM_Dot1qVlanCurrentEntry_T * vlan_info, UI32_T oper_status, UI32_T trap_enabled, UI32_T admin_status)
{
#if 0
    TRAP_EVENT_TrapData_T       trap_data;
    TRAP_EVENT_SendTrapOption_E trap_status;
#endif

    /* BODY */

    if (VLAN_Debug(VLAN_DEBUG_FLAG_NOTIFY))
    {
        BACKDOOR_MGR_Printf("\nVLAN_MGR_Notify_IfOperStatusChanged: vid_ifindex %d, vlan_status %d\n",(UI16_T) vlan_info->dot1q_vlan_index,(UI16_T) oper_status);
    }
    /*add byTony.Lei */
    if(vlan_info->if_entry.ifType == VLAN_L3_IP_IFTYPE){
        SYS_CALLBACK_MGR_L3IfOperStatusChangedCallback(SYS_MODULE_VLAN, vlan_info->dot1q_vlan_index, oper_status);
    }


#if 0
    if (trap_enabled == VAL_ifLinkUpDownTrapEnable_enabled)
    {
        /* 1 */
        trap_status = TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP;
    }
    else
    {
        /* 2 */
        trap_status = TRAP_EVENT_SEND_TRAP_OPTION_LOG_ONLY;
    }

    if (oper_status == VAL_ifOperStatus_up)
    {
        trap_data.trap_type = TRAP_EVENT_LINK_UP;
    } /* end of if */

    if (oper_status == VAL_ifOperStatus_down)
    {
        trap_data.trap_type = TRAP_EVENT_LINK_DOWN;
    } /* end of if */
    trap_data.community_specified               = FALSE;
    trap_data.u.link_up.instance_ifindex        = vlan_info->dot1q_vlan_index;
    trap_data.u.link_up.ifindex                 = vlan_info->dot1q_vlan_index;
    trap_data.u.link_up.instance_adminstatus    = vlan_info->dot1q_vlan_index;
    trap_data.u.link_up.adminstatus             = admin_status;
    trap_data.u.link_up.instance_operstatus     = vlan_info->dot1q_vlan_index;
    trap_data.u.link_up.operstatus              = oper_status;
    SNMP_PMGR_ReqSendTrapOptional(&trap_data, trap_status);

    return;
#else
    ((void)trap_enabled);
    ((void)admin_status);
#endif
} /* End of VLAN_MGR_Notify_IfOperStatusChanged() */

static void VLAN_MGR_Notify_VlanMemberDeleteByTrunk(UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T vlan_status)
{
    if (VLAN_Debug(VLAN_DEBUG_FLAG_NOTIFY))
    {
        BACKDOOR_MGR_Printf("\nVLAN_MGR_Notify_VlanMemberDeleteByTrunk: vid_ifindex %d, lport_ifindex %d, vlan_status %d\n",
                                                 (UI16_T)  vid_ifindex, (UI16_T)lport_ifindex, (UI16_T)vlan_status);
    }

    SYS_CALLBACK_MGR_VlanMemberDeleteByTrunkCallback(SYS_MODULE_VLAN,
        vid_ifindex, lport_ifindex, vlan_status);

    return;
}

/* FUNCTION NAME - VLAN_MGR_Notify_VlanMemberTagChanged
 * PURPOSE : Call CallBack function when tag type of a port member for a VLAN is
 *           changed.
 * INPUT   : vid_ifindex   -- the ifindex of the VLAN
 *         : lport_ifindex -- the ifindex the port
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : Untagged member -> Tagged member / Tagged member -> Untagged member
 */
static void VLAN_MGR_Notify_VlanMemberTagChanged(UI32_T vid_ifindex, UI32_T lport_ifindex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (VLAN_Debug(VLAN_DEBUG_FLAG_NOTIFY))
    {
        BACKDOOR_MGR_Printf("\r\nVLAN_MGR_Notify_VlanMemberTagChanged: "
            "vid_ifindex %lu, lport_ifindex %lu\r\n", (unsigned long)vid_ifindex, (unsigned long)lport_ifindex);
    }

    SYS_CALLBACK_MGR_VlanMemberTagChangedCallback(SYS_MODULE_VLAN, vid_ifindex, lport_ifindex);

    return;
} /* VLAN_MGR_Notify_VlanMemberTagChanged */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SemanticCheck
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if vlan entry is semantically correct.
 *            Otherwise, return false.
 * INPUT    : *vlan_info -- address of the vlan info to be checked
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : If all read-create object have default values, the row is placed
 *            in ACTIVE.  If some read-create object do not have default value,
 *            the row is placed in NOT-READY.  If the row does not exist, the
 *            row is placed in NOT_EXIST
 *--------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_SemanticCheck(void *vlan_info)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     *current_vlan_info;
    UI32_T  vid;

    /* BODY */

    current_vlan_info = (VLAN_OM_Dot1qVlanCurrentEntry_T*)vlan_info;

    VLAN_IFINDEX_CONVERTTO_VID(current_vlan_info->dot1q_vlan_index, vid);

    /* Error if dot1q_vlan_index exceed vid boundary define by the system.
     */
    if ((vid < 1) ||
        (vid > SYS_ADPT_MAX_VLAN_ID))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SemanticCheck_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN ID"
                                );
        return FALSE;
    }

    /* Check vlan name

    if (current_vlan_info->dot1q_vlan_static_name == NULL)
        return FALSE;
    */

    if ((current_vlan_info->dot1q_vlan_status != VAL_dot1qVlanStatus_other) &&
        (current_vlan_info->dot1q_vlan_status != VAL_dot1qVlanStatus_permanent) &&
        (current_vlan_info->dot1q_vlan_status != VAL_dot1qVlanStatus_dynamicGvrp))
    {

        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SemanticCheck_Fun_No,
                                    EH_TYPE_MSG_INVALID,
                                    SYSLOG_LEVEL_INFO,
                                    "vlan status"
                                );
        return FALSE;
    }

    return TRUE;

} /* end of VLAN_MGR_SemanticCheck()*/


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_PreconditionForSetVlanInfo
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the pre-condition to process vlan
 *            operation is satisfy.  Otherwise, return false
 * INPUT    : vid   -- the vid to be updated.
 *            lport -- the specified port
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_PreconditionForSetVlanInfo(UI32_T vid, UI32_T lport_ifindex)
{
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;

    /* BODY */

    /* This API only returns TRUE if lport_ifindex is either NORMAL port or Trunk_port.
       Trunk member port information can not be modify individually.
     */

    if (vid < 1 || vid > SYS_ADPT_MAX_VLAN_ID)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_PreconditionForSetVlanInfo_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN ID"
                                );
        return FALSE;
    }

    if (SWCTRL_LogicalPortExisting(lport_ifindex) == FALSE)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_PreconditionForSetVlanInfo_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                    "lport"
                                );
        return FALSE;
    }

    vlan_port_info.lport_ifindex = lport_ifindex;

    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_PreconditionForSetVlanInfo_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        return FALSE;
    }

    if (vlan_port_info.port_trunk_mode == TRUE)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_PreconditionForSetVlanInfo_Fun_No,
                                    EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO,
                                    "port that is trunk member"
                                );
        return FALSE;
    }

#if (SYS_CPNT_VLAN_ALLOW_AUTHORIZED_PORT_CONFIGURED == FALSE)
    if (vlan_port_info.port_item.auto_vlan_mode && !is_authenticating)
    {
        return FALSE;
    }
#endif

    return TRUE;

} /* end of VLAN_MGR_PreconditionForSetVlanInfo() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_PreconditionForSetVlanPortInfo
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the pre-condition to set vlan_port_info
 *            is satisfy.  Otherwise, return false.
 * INPUT    : lport_ifindex -- specify which port to update.
 *            field         -- specify which particular field to udpate.
 *            field_value   -- specify the value of the field to update.
 * OUTPUT   : port_type
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_PreconditionForSetVlanPortInfo(UI32_T lport_ifindex,
                                                      UI32_T field,
                                                      UI32_T field_value)
{
    /* BODY */

    /* This function only returns TRUE if the specific port is a normal port
       or Trunk port.  Trunk_member port attribute can not be modify individually.
     */
    if (!SWCTRL_LogicalPortExisting(lport_ifindex))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_PreconditionForSetVlanPortInfo_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                    "lport"
                                );
        return FALSE;
    }

    switch (field)
    {
        case PVID_FIELD:

            /* Error if pvid exceed the vid boundary defined by the system.
             */
            if (field_value <1 || field_value > SYS_ADPT_MAX_VLAN_ID)
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_PreconditionForSetVlanPortInfo_Fun_No,
                                            EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                            SYSLOG_LEVEL_INFO,
                                            "VLAN ID"
                                        );
                return FALSE;
            }
            break;

        case INGRESS_FIELD:

            /* Error if input dot1q_port_ingress_filtering value does not equal to the value define by the system.
             */
            if ((field_value != VAL_dot1qPortIngressFiltering_true) &&
               (field_value != VAL_dot1qPortIngressFiltering_false))
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_PreconditionForSetVlanPortInfo_Fun_No,
                                            EH_TYPE_MSG_INVALID,
                                            SYSLOG_LEVEL_INFO,
                                            "ingress filtering value"
                                        );
                return FALSE;
            }
           break;

        case ACCEPTABLE_FRAME_TYPE_FIELD:

            /* Error if input acceptable_frame_type does not equal to the value define by the system.
             */
            if (    (field_value != VAL_dot1qPortAcceptableFrameTypes_admitAll)
                 && (field_value != VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanTagged)
                 && (field_value != VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanUnTagged)
               )
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_PreconditionForSetVlanPortInfo_Fun_No,
                                            EH_TYPE_MSG_INVALID,
                                            SYSLOG_LEVEL_INFO,
                                            "acceptable frame type"
                                        );
                return FALSE;
            }
            break;
         case GVRP_STATUS_FIELD:
            if ((field_value != VAL_dot1qPortGvrpStatus_enabled) &&
                (field_value != VAL_dot1qPortGvrpStatus_disabled))
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_PreconditionForSetVlanPortInfo_Fun_No,
                                            EH_TYPE_MSG_INVALID,
                                            SYSLOG_LEVEL_INFO,
                                            "GVRP status"
                                        );
                return FALSE;
            }
            break;

         case VLAN_PORT_MODE_FIELD:
            if ((field_value != VAL_vlanPortMode_hybrid) &&
                (field_value != VAL_vlanPortMode_dot1qTrunk) &&
                (field_value != VAL_vlanPortMode_access))
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_PreconditionForSetVlanPortInfo_Fun_No,
                                            EH_TYPE_MSG_INVALID,
                                            SYSLOG_LEVEL_INFO,
                                            "port mode"
                                        );
                return FALSE;
            }
            break;

        default:
            EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                        VLAN_MGR_PreconditionForSetVlanPortInfo_Fun_No,
                                        EH_TYPE_MSG_INVALID,
                                        SYSLOG_LEVEL_INFO,
                                        "information"
                                    );
            return FALSE;
            break;

    } /* end of switch */

    return TRUE;

} /* end of VLAN_MGR_PreconditionForSetVlanPortInfo() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_DetailForCreateVlan
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will process the detail to create a new vlan entry.
 *            Since CreateVan and SetVlanRowStatus will both create vlan, this
 *            function will reduce the workload for those 2 functions.
 * INPUT    : vid   -- the specific vlan to be created.
 *            vlan_info -- vlan_info to be created.
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : call by VLAN_MGR_CreateVlan_1() and VLAN_MGR_SetVlanRowStatus()
 *--------------------------------------------------------------------------*/
static void VLAN_MGR_DetailForCreateVlan(UI32_T vid, UI32_T vlan_status, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_info)
{
    UI32_T                  vid_ifindex;

    /* BODY */

    if (NULL != vlan_info)
    {
        /* vid is stored in the corresponding ifindex in the vlan database.
         */
        VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

        memset(vlan_info, 0 , sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));

        SYS_TIME_GetSystemUpTimeByTick(&vlan_info->dot1q_vlan_creation_time);
        vlan_info->dot1q_vlan_time_mark = vlan_info->dot1q_vlan_creation_time;
        vlan_info->dot1q_vlan_index = (UI16_T)vid_ifindex;
        vlan_info->dot1q_vlan_fdb_id = vid;
        vlan_info->dot1q_vlan_static_row_status = L_RSTATUS_NOT_EXIST;
        vlan_info->dot1q_vlan_status = (UI8_T)vlan_status;

        if (vlan_status == VLAN_TYPE_VLAN_STATUS_STATIC)
        {
            snprintf(vlan_info->dot1q_vlan_static_name,
                SYS_ADPT_MAX_VLAN_NAME_LEN+1, "VLAN%04lu", vid);
        }

#if (SYS_CPNT_RSPAN == TRUE)
        vlan_info->rspan_status = VAL_vlanStaticExtRspanStatus_vlan;
#endif

        /* RFC2863
         */
        vlan_info->if_entry.ifType= VLAN_DEFAULT_IFTYPE;
        vlan_info->if_entry.vlan_operation_status = VAL_ifOperStatus_down;
        vlan_info->if_entry.admin_status = VAL_ifAdminStatus_up;
        vlan_info->if_entry.link_up_down_trap_enabled = VLAN_MGR_LinkUpDownTrapEnabled;
        vlan_info->vlan_address_method = SYS_DFLT_VLAN_ADDRESS_METHOD;
        vlan_info->vlan_ip_state = VLAN_MGR_IP_STATE_NONE;

        /*init the default vlan mac */
        /* vlan type change , vlan create and delete  is more than 4k times , I think if stkplg notify to me , it is better
         * Tony.Lei
         */
        if (SWCTRL_GetCpuMac(vlan_info->cpu_mac) == FALSE)
        {
            return;
        };
    }

    return;
} /* end of VLAN_MGR_DetailForCreateVlan() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetVlanRowStatus
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the row status field of the input
 *            vlan info can be set successfully.  Otherwise, return false.
 * INPUT    : row_status_action - the action of the row status
 *            *vlan_info - the address of the vlan info that needs to be set.
 * OUTPUT   : *vlan_info - the updated vlan_info.
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_SetVlanRowStatus(UI32_T row_status_action, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_info)
{
    /* BODY */

    /* Pass vlan_info into FSM to check and update dot1q_vlan_static_row_status field.
       If transition of dot1q_vlan_static_row_status is correct, then update vlan_info
       into database.  Otherwise, return false.
     */
    switch (L_RSTATUS_Fsm(row_status_action,
                          &vlan_info->dot1q_vlan_static_row_status,
                          VLAN_MGR_SemanticCheck,
                          (void*)vlan_info))
    {
        case L_RSTATUS_NOTREADY_2_ACTIVE:
        case L_RSTATUS_NOTREADY_2_NOTREADY:
        case L_RSTATUS_ACTIVE_2_ACTIVE:
        case L_RSTATUS_ACTIVE_2_NOTREADY:
            return VLAN_OM_SetVlanEntry(vlan_info);
        default:
            VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, SWITCH_TO_DEFAULT_MESSAGE_INDEX, "VLAN_MGR_SetVlanRowStatus");
            return FALSE;
    } /* end of switch */

} /* end of VLAN_MGR_SetVlanRowStatus() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetTrunkMemberPortInfo
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if port information for each trunk
 *            member can be set successfully.  Otherwise, return FALSE.
 * INPUT    : trunk_ifindex - specific trunk index
 *            field_type - dot1q_pvid_index \
 *                         dot1q_port_acceptable_frame_types \
 *                         dot1q_port_ingress_filtering \
 *                         dot1q_port_gvrp_status
 *            field_value - Value that needs to be update to vlan_om
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. This API should be call by VLAN_MGR_SetDot1qXXX if the specific
 *               user input index is a trunk index.
 *            2. Information for trunk member index should be consistent with
 *               trunk_index.
 *            3. VLAN_MGR only needs to update VLAN_OM for each trunk member index.
 *               VLAN_MGR does not need to call swctrl to modify field value for
 *               each trunk member index.
 *--------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_SetTrunkMemberPortInfo(UI32_T trunk_ifindex, UI32_T field_type, UI32_T field_value)
{
    TRK_MGR_TrunkEntry_T        trunk_info;
    VLAN_OM_Vlan_Port_Info_T    vlan_port_info;
    UI32_T         port_num;
    UI32_T          unit, port, trunk_id;


    /* BODY */


    SWCTRL_LogicalPortToUserPort(trunk_ifindex, &unit, &port, &trunk_id);

    trunk_info.trunk_index = trunk_id;

    if (!TRK_MGR_GetTrunkEntry(&trunk_info))
        return FALSE;

    for (port_num = 1; port_num <= (SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST * 8); port_num++)
    {
        if (VLAN_OM_IS_MEMBER(trunk_info.trunk_ports, port_num))
        {
            vlan_port_info.lport_ifindex = port_num;

            if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_SetTrunkMemberPortInfo_Fun_No,
                                            EH_TYPE_MSG_NOT_EXIST,
                                            SYSLOG_LEVEL_INFO,
                                            "VLAN port entry"
                                        );
                return FALSE;
            }
            switch (field_type)
            {
                case PVID_FIELD:
                    vlan_port_info.port_item.dot1q_pvid_index = field_value;
                    break;
                case INGRESS_FIELD:
                    vlan_port_info.port_item.dot1q_port_ingress_filtering = field_value;
                    break;
                case ACCEPTABLE_FRAME_TYPE_FIELD:
                    vlan_port_info.port_item.dot1q_port_acceptable_frame_types = field_value;
                    break;
                case GVRP_STATUS_FIELD:
                    vlan_port_info.port_item.dot1q_port_gvrp_status = field_value;
                    break;
                case VLAN_PORT_MODE_FIELD:
                    vlan_port_info.vlan_port_entry.vlan_port_mode = field_value;
                    break;
                case ADMIN_PVID_FIELD:
                    vlan_port_info.port_item.admin_pvid = field_value;
                    break;
                case ADMIN_ACCEPTABLE_FRAME_TYPE_FIELD:
                    vlan_port_info.port_item.admin_acceptable_frame_types = field_value;
                    break;
                case ADMIN_INGRESS_FIELD:
                    vlan_port_info.port_item.admin_ingress_filtering = field_value;
                    break;
                default:
                    return FALSE;
            }

            if (!VLAN_OM_SetVlanPortEntry(&vlan_port_info))
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_SetTrunkMemberPortInfo_Fun_No,
                                            EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                            (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                            "lport (1-SYS_ADPT_TOTAL_NBR_OF_LPORT"
                                        );
                return FALSE;
            }
        } /* end of if */
    } /* end of for */

    return TRUE;
}/* end of VLAN_MGR_SetTrunkMemberPortInfo() */


/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qPvidWithoutChangingMembership
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the pvid of the specific port can be
 *            set successfully. Otherwise, return false.
 * INPUT    : lport_ifindex   -- the port number
 *            pvid     -- the pvid value
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_SetDot1qPvidWithoutChangingMembership(UI32_T lport_ifindex, UI32_T pvid, BOOL_T check_port)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    UI32_T                              pvid_ifindex;
    BOOL_T                              ret = TRUE;

    VLAN_VID_CONVERTTO_IFINDEX(pvid, pvid_ifindex);
    vlan_info.dot1q_vlan_index = pvid_ifindex;
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        ret = FALSE;
    }
    else if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
    {
        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
        {
            if (!SWCTRL_SetPortPVID(lport_ifindex, pvid,check_port))
            {
                ret = FALSE;
            }
        }
        else if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex))
        {
            if (    (!VLAN_MGR_AddVlanMemberToChip(lport_ifindex, pvid, TRUE, check_port))
                ||  (!SWCTRL_SetPortPVID(lport_ifindex, pvid,check_port))
                ||  (!SWCTRL_DeletePortFromVlanUntaggedSet(lport_ifindex, pvid,check_port))
               )
            {
                ret = FALSE;
            }
        }
        else
        {
        /*EPR: ES3628BT-FLF-ZZ-00772
Problem:RIP: LACP member add/remove will cause L3 packets lost.
Rootcause:when sync trunk info to trunk member,it will fail.
          (1)because the port added to trunk ,it will set trunk member info in port_info.
          (2)when sync trunk member vlan attribute,and add/remove from chip,it will check if trunk member ,if it is trunk

member it will return FALSE.
          (3)when set pvid,it will add the port to the vlan,and remove it .At last ,the port will not in the native vlan
Solution: when add trunk member to vlan when syn the attribute ,not check trunk member
          just set pvid ,not remove the port
Files:vlan_mgr.c,swctrl.h,swctrl.c*/
            if ( !SWCTRL_SetPortPVID(lport_ifindex, pvid,check_port))
            {
                ret = FALSE;
            }
        }
    } /* end of if */
    else if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_notReady)
    {
        if (!SWCTRL_SetPortPVID(lport_ifindex, VLAN_OM_GetGlobalDefaultVlan(),check_port))
        {
            ret = FALSE;
        }
    }
    else
    {
        ret = FALSE;
    }

    return ret;
}

#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDefaultPortMembership
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the port information can be restore
 *            back to default value successfully.  Otherwise, return FALSE.
 * INPUT    : vid_ifindex - the specific vid lport_ifindex joined
 *            lport_ifindex - the specific port lport to be configured.
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_SetDefaultPortMembership(UI32_T vid, UI32_T lport_ifindex)
{
    UI32_T                              unit, port, trunk_id;
    UI32_T                              vid_ifindex;
    VLAN_OM_Dot1qVlanCurrentEntry_T     default_vlan_info;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;

    /* BODY */

    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
    default_vlan_info.dot1q_vlan_index = vid_ifindex;
    vlan_port_info.lport_ifindex = lport_ifindex;

    if (!VLAN_OM_GetVlanEntry(&default_vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDefaultPortMembership_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        return FALSE;
    }

    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDefaultPortMembership_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        return FALSE;
    }

    /* Set_Vlan_Info: */
    {
        if (VLAN_OM_IS_MEMBER(default_vlan_info.dot1q_vlan_forbidden_egress_ports, lport_ifindex))
            VLAN_OM_DEL_MEMBER(default_vlan_info.dot1q_vlan_forbidden_egress_ports, lport_ifindex);

        if (!VLAN_OM_IS_MEMBER(default_vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
        {
            if (!VLAN_OM_IS_MEMBER(default_vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex))
            {
                VLAN_OM_ADD_MEMBER(default_vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex);
                if (is_provision_complete == TRUE)
                {
                    if (!VLAN_MGR_AddVlanMemberToChip(lport_ifindex, vid, FALSE, TRUE))
                    {
                        VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING,
                                                     FUNCTION_RETURN_FAIL_INDEX,
                                                     "VLAN_MGR_SetDefaultPortMembership");
                        return FALSE;
                    }
                }
            } /* end of if */

            VLAN_OM_ADD_MEMBER(default_vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex);
            vlan_port_info.port_item.untagged_joined_vlan_count++;

            if (is_provision_complete == TRUE)
            {
                if (!VLAN_MGR_AddVlanMemberToChip(lport_ifindex, 1, TRUE, TRUE))
                {
                    VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING,
                                                 FUNCTION_RETURN_FAIL_INDEX,
                                                 "VLAN_MGR_SetDefaultPortMembership");
                    return FALSE;
                }
            }

            if (!is_authenticating)
            {
                if (!VLAN_OM_IS_MEMBER(default_vlan_info.dot1q_vlan_static_egress_ports, lport_ifindex))
                {
                    VLAN_OM_ADD_MEMBER(default_vlan_info.dot1q_vlan_static_egress_ports, lport_ifindex);
                    vlan_port_info.port_item.static_joined_vlan_count++;
                }
                VLAN_OM_ADD_MEMBER(default_vlan_info.dot1q_vlan_static_untagged_ports, lport_ifindex);
            }
        } /* end of if */
    } /* end of Set_Vlan_Info */

    /* Set_Port_Info: */
    {

        vlan_port_info.port_item.dot1q_pvid_index = vid_ifindex;
        if (!is_authenticating)
        {
            vlan_port_info.port_item.admin_pvid = vid;
        }

        if (!SWCTRL_SetPortPVID(lport_ifindex, vid,TRUE))
        {
            VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING,
                                         FUNCTION_RETURN_FAIL_INDEX,
                                         "VLAN_MGR_SetDefaultPortMembership");
            return FALSE;
        }

        if (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_dot1qTrunk)
        {
            vlan_port_info.vlan_port_entry.vlan_port_mode = VAL_vlanPortMode_hybrid;
        } /* end of if */

        if (!VLAN_OM_SetVlanPortEntry(&vlan_port_info))
        {
            EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                        VLAN_MGR_SetDefaultPortMembership_Fun_No,
                                        EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                        (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                        "lport (1-SYS_ADPT_TOTAL_NBR_OF_LPORT"
                                    );
            return FALSE;
        }
        if (SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit, &port, &trunk_id) == SWCTRL_LPORT_TRUNK_PORT)
        {
            if (!VLAN_MGR_SetTrunkMemberPortInfo(lport_ifindex, PVID_FIELD, vid_ifindex))
                return FALSE;
        } /* end of if */
    } /* end of Set_Port_Info */

    if (!VLAN_OM_SetVlanEntry(&default_vlan_info))
    {
        char    arg_buf[15];
        snprintf(arg_buf, 15, "VLANs (%d)",SYS_ADPT_MAX_NBR_OF_VLAN);
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDefaultPortMembership_Fun_No,
                                    EH_TYPE_MSG_MAXIMUM_EXCEEDED,
                                    SYSLOG_LEVEL_INFO,
                                    arg_buf
                                );

        return FALSE;
    }

    return TRUE;
} /* end of VLAN_MGR_SetDefaultPortMembership() */
#endif /* #if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE) */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_AddVlanMember
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true all the member of the specific vlan
 *            can be remove from ASIC successfully.  Otherwise, return FALSE.
 * INPUT    : vlan_info - the specific vlan information to be modified.
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The purpose for this function is to distinguish set vlan
 *               memberlist for suspend vlan and delete vlan.
 *--------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_AddVlanMember(UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T member_option, UI32_T vlan_status)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    UI32_T                              member_type;
    UI32_T                              vid;
    UI32_T                              port_state;

    /* BODY
     */
    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        BACKDOOR_MGR_Printf("VLAN_MGR_AddVlanMember: vid_ifindex %d, lport_ifindex %d, member_option %d, vlan_status %d\n",
               (UI16_T) vid_ifindex,(UI16_T) lport_ifindex,(UI16_T) member_option,(UI16_T) vlan_status);

    if (vlan_status < VLAN_TYPE_VLAN_STATUS_OTHER || vlan_status > VLAN_TYPE_VLAN_STATUS_MVR)
        return FALSE;

    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;
    vlan_port_info.lport_ifindex = lport_ifindex;

    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddVlanMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        return FALSE;
    }

    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddVlanMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        return FALSE;
    }

    if (    (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_access)
         && (member_option == VLAN_MGR_TAGGED_ONLY)
       )
        return FALSE;

    if (    (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_dot1qTrunk)
         && ((member_option == VLAN_MGR_BOTH) || (member_option == VLAN_MGR_UNTAGGED_ONLY))
       )
        return FALSE;

    if (vlan_port_info.port_trunk_mode)
        return FALSE;

    /* follow member addition precedence
     * 1. auto member shall not be overwritten by GVRP, Voice, MVR method
     * 2. static member shall not be overwritten by GVRP, Voice, MVR method
     * 3. Voice member shall not be overwritten by GVRP, MVR method
     * 4. MVR member shall not be overwritten by GVRP method
     */
    member_type = VLAN_MGR_GetPortMemberType(&vlan_info, &vlan_port_info);
    if (vlan_precedence_table[member_type][vlan_status] == VLAN_MGR_INFERIOR)
        return FALSE;

    VLAN_IFINDEX_CONVERTTO_VID(vid_ifindex, vid);

    if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, lport_ifindex))
    {
        if (vlan_status == VAL_dot1qVlanStatus_permanent)
            VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, lport_ifindex);
        else
            return FALSE;
    } /* end of if */

#if (SYS_CPNT_VLAN_NO_CHECK_VLAN_FOR_PVID == FALSE)
    if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
    {
        if (member_option == VLAN_MGR_TAGGED_ONLY)
        {
            /* Can not remove pvid vlan */
            if (vlan_port_info.port_item.dot1q_pvid_index == vid_ifindex)
            {
                return FALSE;
            }
        }
    }
#endif /* end of #if (SYS_CPNT_VLAN_NO_CHECK_VLAN_FOR_PVID == FALSE) */

    /* add port to current port bitmap only if the port is not authorized */
    if ((!vlan_port_info.port_item.auto_vlan_mode) || is_authenticating)
    {
        VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex);
    }

    if (member_option == VLAN_MGR_TAGGED_ONLY)
    {
        /* only if the port is member of current port bitmap can be set to chip */
        if (    (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
             && (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex))
           )
        {
            if (is_provision_complete == TRUE)
            {
                if (!VLAN_MGR_AddVlanMemberToChip(lport_ifindex, vid, FALSE, TRUE))
                {
                    VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING,
                                                 FUNCTION_RETURN_FAIL_INDEX,
                                                 "VLAN_MGR_AddVlanMember");
                    return FALSE;
                }
            }
        }

        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
        {
            VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex);
            vlan_port_info.port_item.untagged_joined_vlan_count--;
        }
    }
    else if (member_option == VLAN_MGR_BOTH)
    {
        /* add port to current port bitmap only if the port is not authorized */
        if ((!vlan_port_info.port_item.auto_vlan_mode) || is_authenticating)
        {
            if (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
            {
                VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex);
                vlan_port_info.port_item.untagged_joined_vlan_count++;
            }
        }

        /* only if the port is member of current port bitmap can be set to chip */
        if (    (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
             && (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
           )
        {
            if (is_provision_complete == TRUE)
            {
                if (!VLAN_MGR_AddVlanMemberToChip(lport_ifindex, vid, TRUE, TRUE))
                {
                    VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING,
                                                 FUNCTION_RETURN_FAIL_INDEX,
                                                 "VLAN_MGR_AddVlanMember");
                    return FALSE;
                } /* end of if */
            }
        }
    } /* end of if */

    if (    (vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_dynamicGvrp)
         && (vlan_status == VAL_dot1qVlanStatus_permanent)
       )
        vlan_info.dot1q_vlan_status = VAL_dot1qVlanStatus_permanent;

    switch (vlan_status)
    {
        case VLAN_TYPE_VLAN_STATUS_STATIC:
            if (member_type != VLAN_TYPE_VLAN_STATUS_STATIC)
            {
                VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, lport_ifindex);
                vlan_port_info.port_item.static_joined_vlan_count++;
            }

            if (member_option == VLAN_MGR_TAGGED_ONLY)
            {
                /* Delete untagged member */
                VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, lport_ifindex);
            }
            else if (member_option == VLAN_MGR_BOTH)
            {
                VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, lport_ifindex);
            }

            break;

        case VLAN_TYPE_VLAN_STATUS_AUTO:
            break;

        case VLAN_TYPE_VLAN_STATUS_VOICE:
            vlan_port_info.vlan_port_entry.voice_vid = vid;
            break;

        case VLAN_TYPE_VLAN_STATUS_MVR:
            vlan_port_info.vlan_port_entry.mvr_vid++ /* = vid*/;
            break;

        case VLAN_TYPE_VLAN_STATUS_GVRP:
        default:
            break;
    } /* end of switch */

    if (!VLAN_MGR_SetVlanRowStatus(L_RSTATUS_SET_OTHER_COLUMN, &vlan_info))
    {
        return FALSE;
    }

    VLAN_OM_SetVlanPortEntry(&vlan_port_info);

    if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
    {
        if (vlan_info.if_entry.vlan_operation_status != VAL_ifOperStatus_up)
        {
            /* Allen Cheng, Dec/30/2002, modified for changing to invoke the XSTP_SVC */
            /* if (!SWCTRL_GetPortSTAState(vid, lport_ifindex, &port_state)) */
            if (!XSTP_OM_GetPortStateByVlan(vid, lport_ifindex, &port_state))
                return FALSE;
            if (port_state == VAL_dot1dStpPortState_forwarding)
            {
                VLAN_MGR_SET_IF_OPER_STATUS(vlan_info.if_entry, VAL_ifOperStatus_up);
                VLAN_OM_SetVlanEntry(&vlan_info);
                VLAN_MGR_SEND_IF_OPER_STATUS_CHANGED_EVENT(vid);
            }
        } /* end of if */
    }

    return TRUE;
} /* VLAN_MGR_AddVlanMember() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_RemoveVlanMember
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true all the member of the specific vlan
 *            can be remove from ASIC successfully.  Otherwise, return FALSE.
 * INPUT    : vlan_info - the specific vlan information to be modified.
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The purpose for this function is to distinguish set vlan
 *               memberlist for suspend vlan and delete vlan.
 *--------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_RemoveVlanMember(UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T member_option,UI32_T current_status, UI32_T vlan_status)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    UI32_T  vid, port_state;
    UI8_T   active_uport_count_per_unit[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    UI8_T   port_list [SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T   total_active_uport_count, i;
    UI32_T  member_type;

    /* BODY */

    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        BACKDOOR_MGR_Printf("VLAN_MGR_RemoveVlanMember: vid_ifindex %d, lport_ifindex %d, member_option %d, current_status %d\n",
                                          (UI16_T) vid_ifindex,(UI16_T) lport_ifindex,(UI16_T) member_option,(UI16_T) current_status);

    vlan_info.dot1q_vlan_index =(UI16_T)vid_ifindex;

    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_RemoveVlanMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        return FALSE;
    }

    vlan_port_info.lport_ifindex = lport_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddVlanMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        return FALSE;
    }

    /* port member added by one method shall not be removed by any other method
     * except the unauthorized port is authenticating.
     */
    member_type = VLAN_MGR_GetPortMemberType(&vlan_info, &vlan_port_info);
    if (    (member_type != vlan_status)
         && (vlan_status != VLAN_TYPE_VLAN_STATUS_AUTO)
       )
    {
        return FALSE;
    }

    VLAN_IFINDEX_CONVERTTO_VID(vid_ifindex, vid);

    if ((vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_dynamicGvrp) &&
        (vlan_status == VAL_dot1qVlanStatus_permanent))
        vlan_info.dot1q_vlan_status = VAL_dot1qVlanStatus_permanent;

    /* delete port from both current and static port bitmap only if the port is not authorized
     * if port is authorized, shall only delete port from static egress port bitmap
     */

    if (member_option == VLAN_MGR_UNTAGGED_ONLY)
    {
        if (    (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
             && ((!vlan_port_info.port_item.auto_vlan_mode) || is_authenticating)
           )
        {
            if (current_status == L_RSTATUS_ACTIVE_2_NOTEXIST)
            {
                VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex);
                vlan_port_info.port_item.untagged_joined_vlan_count--;
            }

            if ((vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active) ||
                (current_status == L_RSTATUS_ACTIVE_2_NOTREADY))
            {
                if (is_provision_complete == TRUE)
                {
                    if (!SWCTRL_DeletePortFromVlanUntaggedSet(lport_ifindex, vid,TRUE))
                    {
                        VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING,
                                                     FUNCTION_RETURN_FAIL_INDEX,
                                                     "VLAN_MGR_DestroyVlanMembers");
                        return FALSE;
                    }
                }
            }
        }
    }
    else
    {
        /* delete port from both current and static port bitmap only if the port is not authorized
         * if port is authorized, shall only delete port from static egress port bitmap
         */
        if (    (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex))
             && ((!vlan_port_info.port_item.auto_vlan_mode) || is_authenticating)
           )
        {
            if (current_status == L_RSTATUS_ACTIVE_2_NOTEXIST)
            {
                if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
                {
                    VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex);
                    vlan_port_info.port_item.untagged_joined_vlan_count--;
                }
                VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex);
            }

            if ((vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active) ||
                (current_status == L_RSTATUS_ACTIVE_2_NOTREADY))
            {
                /* driver supports deleting from untag set automatically */
                if (!SWCTRL_DeletePortFromVlanMemberSet(lport_ifindex, vid,TRUE))
                {
                    VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING,
                                                 FUNCTION_RETURN_FAIL_INDEX,
                                                 "VLAN_MGR_DestroyVlanMembers");
                    return FALSE;
                }
            }
        }
    }

    if (    (vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_dynamicGvrp)
         && (vlan_status == VAL_dot1qVlanStatus_permanent)
       )
        vlan_info.dot1q_vlan_status = VAL_dot1qVlanStatus_permanent;

    if (current_status == L_RSTATUS_ACTIVE_2_NOTEXIST)
    {
        switch (vlan_status)
        {
            case VLAN_TYPE_VLAN_STATUS_STATIC:
                VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, lport_ifindex);
                if (member_option != VLAN_MGR_UNTAGGED_ONLY)
                {
                    VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, lport_ifindex);
                    vlan_port_info.port_item.static_joined_vlan_count--;
                }
                break;

            case VLAN_TYPE_VLAN_STATUS_AUTO:
                break;

            case VLAN_TYPE_VLAN_STATUS_VOICE:
                vlan_port_info.vlan_port_entry.voice_vid = 0;
                break;

            case VLAN_TYPE_VLAN_STATUS_MVR:
                vlan_port_info.vlan_port_entry.mvr_vid >0? (vlan_port_info.vlan_port_entry.mvr_vid--):(vlan_port_info.vlan_port_entry.mvr_vid= 0);
                break;

            case VLAN_TYPE_VLAN_STATUS_GVRP:
            default:
                break;
        } /* end of switch */
    } /* end of if */

    if (!VLAN_MGR_SetVlanRowStatus(L_RSTATUS_SET_OTHER_COLUMN, &vlan_info))
    {
        return FALSE;
    }

    VLAN_OM_SetVlanPortEntry(&vlan_port_info);

    if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
    {
        if ((vlan_info.if_entry.vlan_operation_status == VAL_ifOperStatus_up) &&
            (member_option == VLAN_MGR_BOTH))
        {
            VLAN_IFINDEX_CONVERTTO_VID(vlan_info.dot1q_vlan_index,vid);
            /* Allen Cheng, Dec/30/2002, modified for changing to invoke the XSTP_SVC */
            /* if (!SWCTRL_GetPortSTAState(vid, lport_ifindex, &port_state)) */
            if (!XSTP_OM_GetPortStateByVlan(vid, lport_ifindex, &port_state))
                return FALSE;
            if (port_state == VAL_dot1dStpPortState_forwarding)
            {
                /* Allen Cheng, Dec/30/2002, modified for SWCTRL adding an input entry vlan_id */
                if (SWCTRL_LportListToActiveUportList(  vid,
                                                        vlan_info.dot1q_vlan_current_egress_ports,
                                                        active_uport_count_per_unit, port_list
                                                     ) == TRUE
                   )
                {
                    total_active_uport_count    = 0;
                    for (i = 0; i < SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
                    {
                        total_active_uport_count    += active_uport_count_per_unit[i];
                    }
                    if (total_active_uport_count  == 0)
                    {
                        VLAN_MGR_SET_IF_OPER_STATUS(vlan_info.if_entry, VAL_ifOperStatus_down);
                        VLAN_OM_SetVlanEntry(&vlan_info);
                        VLAN_MGR_SEND_IF_OPER_STATUS_CHANGED_EVENT(vid);
                    }
                } /* end of if */
            } /* end of if */
        } /* end of if */
    }

    return TRUE;
} /* end of VLAN_MGR_RemoveVlanMember() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_RemoveVlan
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true all the member of the specific vlan
 *            can be remove from ASIC successfully.  Otherwise, return FALSE.
 * INPUT    : vlan_info - the specific vlan information to be modified.
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The purpose for this function is to distinguish set vlan
 *               memberlist for suspend vlan and delete vlan.
 *--------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_RemoveVlan(UI32_T vid_ifindex, UI32_T current_status)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    UI32_T  vid;

    /* BODY */

    VLAN_IFINDEX_CONVERTTO_VID(vid_ifindex, vid);
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    /* returns error if vlan_info can not be retrieve successfully from the database.
     */
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_RemoveVlan_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        return FALSE;
    } /* end of if */

    if (!SWCTRL_DestroyVlan(vid))
    {
        VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_RemoveVlan");
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("ERROR: SWCTRL_DestroyVlan in VLAN_MGR_RemoveVlan \n");
        return FALSE;
    } /* end of if */
#if 0
    if (VLAN_MGR_DestroyVlanDev(vid) != VLAN_TYPE_RETVAL_OK)
    {
        printf("\r\nVLAN_MGR_DestroyVlanDev() returns error!\r\n");
        return FALSE;
    }
#endif
    VLAN_OM_EnterCriticalSection();
    SYS_TIME_GetSystemUpTimeByTick(VLAN_OM_GetIfTableLastUpdateTimePtr());
    VLAN_OM_LeaveCriticalSection();

    if (current_status == L_RSTATUS_ACTIVE_2_NOTEXIST)
        return VLAN_OM_DeleteVlanEntry(vid);
    else if (current_status == L_RSTATUS_ACTIVE_2_NOTREADY)
    {
        UI32_T old_oper_status;

        old_oper_status = vlan_info.if_entry.vlan_operation_status;
        vlan_info.dot1q_vlan_static_row_status = VAL_dot1qVlanStaticRowStatus_notReady;
        VLAN_MGR_SET_IF_OPER_STATUS(vlan_info.if_entry, VAL_ifOperStatus_down);
        VLAN_OM_SetVlanEntry(&vlan_info);

        if (old_oper_status == VAL_ifOperStatus_up)
        {
            VLAN_MGR_SEND_IF_OPER_STATUS_CHANGED_EVENT(vid);
        }
        return TRUE;
    }
    else
        return FALSE;

} /* end of VLAN_MGR_RemoveVlan() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetHybridMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the detail operation for set port
 *            to hybrid mode can be perform successfully.  Otherwise, return
 *            false.
 * INPUT    : lport_ifindex - the specific port information to be updated
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_SetHybridMode(UI32_T lport_ifindex)
{
    VLAN_OM_Vlan_Port_Info_T    vlan_port_info;
    UI32_T                      unit, port, trunk_id;

    /* BODY
     */

    vlan_port_info.lport_ifindex = lport_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_VLAN,
                                 VLAN_MGR_SetHybridMode_Fun_No,
                                 EH_TYPE_MSG_NOT_EXIST,
                                 SYSLOG_LEVEL_INFO,
                                 "VLAN port entry"
                                );
        return FALSE;
    } /* end of if */

    vlan_port_info.vlan_port_entry.vlan_port_mode = VAL_vlanPortMode_hybrid;

    if (SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit, &port, &trunk_id) == SWCTRL_LPORT_TRUNK_PORT)
    {
        VLAN_MGR_SetTrunkMemberPortInfo(lport_ifindex,VLAN_PORT_MODE_FIELD, VAL_vlanPortMode_hybrid);
    } /* end of if */

    if (!VLAN_OM_SetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_VLAN,
                                 VLAN_MGR_SetHybridMode_Fun_No,
                                 EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                 "lport (1-SYS_ADPT_TOTAL_NBR_OF_LPORT"
                                );
        return FALSE;
    } /* end of if */

    return TRUE;
} /* End of VLAN_MGR_SetHybridMode() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetQTrunkMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the detail operation for set port
 *            to Q-Trunk mode can be perform successfully.  Otherwise, return
 *            false.
 * INPUT    : lport_ifindex - the specific port information to be updated
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. Port under Q-Trunk mode transmits and receives tagged only
 *               frames.
 *            2. Port under Q-Trunk mode can only join vlan tagged member set
 *            3. Acceptable frame type for port under Q-Trunk mode is automatically
 *               set to accept tagged only frames.
 *--------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_SetQTrunkMode(UI32_T lport_ifindex)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T    vlan_info;
    VLAN_OM_Vlan_Port_Info_T           vlan_port_info;
    UI32_T                             unit, port, trunk_id, vid;
    SWCTRL_Lport_Type_T                port_type;
    BOOL_T                             is_member_changed;

    /* BODY
     */

    vlan_port_info.lport_ifindex = lport_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetQTrunkMode_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        return FALSE;
    }

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
    /* The default vlan contains all ports in its untagged list,
     * so if a port only joins Default VLAN then it cannot be set as Q-Trunk mode.
     */
    {
        BOOL_T                  found = FALSE;
        while (VLAN_OM_GetNextVlanEntry(&vlan_info))
        {
            VLAN_IFINDEX_CONVERTTO_VID(vlan_info.dot1q_vlan_index, vid);

            if (    (vid != VLAN_OM_GetGlobalDefaultVlan())
                &&  (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex))
               )
            {
                found = TRUE;
                break;
            }
        }
        if(!found)
        {
            EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                        VLAN_MGR_SetQTrunkMode_Fun_No,
                                        EH_TYPE_MSG_INVALID,
                                        SYSLOG_LEVEL_INFO,
                                        "this port only joins Default VLAN"
                                    );
            return FALSE;
        }
        if (vlan_port_info.vlan_port_entry.vlan_port_dual_mode)
        {
            return FALSE;
        }
    }
#endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE) */

    vlan_port_info.vlan_port_entry.vlan_port_mode = VAL_vlanPortMode_dot1qTrunk;

    if ((port_type = SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit, &port, &trunk_id)) == SWCTRL_LPORT_TRUNK_PORT)
    {
        VLAN_MGR_SetTrunkMemberPortInfo(lport_ifindex,VLAN_PORT_MODE_FIELD, VAL_vlanPortMode_dot1qTrunk);
    } /* end of if */

    vlan_info.dot1q_vlan_index = 0;
    while (VLAN_OM_GetNextVlanEntry(&vlan_info))
    {
        is_member_changed = FALSE;
        VLAN_IFINDEX_CONVERTTO_VID(vlan_info.dot1q_vlan_index, vid);

        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
        {
            is_member_changed = TRUE;
            VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex);
            vlan_port_info.port_item.untagged_joined_vlan_count--;
            if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
            {
                if (is_provision_complete == TRUE)
                {
                    if(!SWCTRL_DeletePortFromVlanUntaggedSet(lport_ifindex, vid,TRUE))
                    {
                        VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_SetQTrunkMode");
                        return FALSE;
                    } /* end of if */
                }
            } /* end of if */
        }

        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, lport_ifindex))
        {
            is_member_changed = TRUE;
            VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, lport_ifindex);
        } /* end of if */

        if (is_member_changed)
        {
            SYS_TIME_GetSystemUpTimeByTick(&vlan_info.dot1q_vlan_time_mark);

            if (!VLAN_OM_SetVlanEntry(&vlan_info))
            {
                char    arg_buf[15];
                snprintf(arg_buf, 15, "VLANs (%d)",SYS_ADPT_MAX_NBR_OF_VLAN);
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_SetQTrunkMode_Fun_No,
                                            EH_TYPE_MSG_MAXIMUM_EXCEEDED,
                                            SYSLOG_LEVEL_INFO,
                                            arg_buf
                                        );
                return FALSE;
            } /* end of if */
        } /* end of if */
    } /* end of while */

    if (!VLAN_OM_SetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetQTrunkMode_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                    "lport (1-SYS_ADPT_TOTAL_NBR_OF_LPORT"
                                );
        return FALSE;
    } /* end of if */

    return TRUE;
} /* End of VLAN_MGR_SetQTrunkMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetAccessMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the detail operation for set port
 *            to access mode can be perform successfully.  Otherwise, return
 *            false.
 * INPUT    : lport_ifindex - the specific port information to be updated
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. Ports in access mode can only join valn as untagged member.
 *               Hybrid link indicates that there is no tagged vlan frames
 *               traveling thru the link.
 *--------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_SetAccessMode(UI32_T lport_ifindex)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T    vlan_info;
    VLAN_OM_Vlan_Port_Info_T           vlan_port_info;
    UI32_T                             unit, port, trunk_id, vid;
    SWCTRL_Lport_Type_T                port_type;
    BOOL_T                             is_member_changed;

    /* BODY
     */

    vlan_port_info.lport_ifindex = lport_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetAccessMode_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        return FALSE;
    } /* end of if */

    #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE)
    vlan_info.dot1q_vlan_index = 0;
    while (VLAN_OM_GetNextVlanEntry(&vlan_info))
    {
        /* check current table */
        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex)
            &&!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, lport_ifindex))
        {
            /*printf("There are dynamic joined VLAN on this port, later confirm to single access port will fail so return fail here first.\r\n");*/
            return FALSE;
        }
    }

    #endif

    vlan_port_info.vlan_port_entry.vlan_port_mode = VAL_vlanPortMode_access;

    if ((port_type = SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit, &port, &trunk_id)) == SWCTRL_LPORT_TRUNK_PORT)
    {
        VLAN_MGR_SetTrunkMemberPortInfo(lport_ifindex,VLAN_PORT_MODE_FIELD, VAL_vlanPortMode_access);
    } /* end of if */

    vlan_info.dot1q_vlan_index = 0;
    while (VLAN_OM_GetNextVlanEntry(&vlan_info))
    {
        is_member_changed = FALSE;
        VLAN_IFINDEX_CONVERTTO_VID(vlan_info.dot1q_vlan_index, vid);

        /* check current table */
        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex))
        {
            /* check current untagged list */
            if (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
            {
                /* join the current untagged list of joined vlan */
                is_member_changed = TRUE;
                VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex);
                vlan_port_info.port_item.untagged_joined_vlan_count++;

                if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
                {
                    if (is_provision_complete == TRUE)
                    {
                        if (!VLAN_MGR_AddVlanMemberToChip(lport_ifindex, vid, TRUE, TRUE))
                        {
                            VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING,
                                                         FUNCTION_RETURN_FAIL_INDEX,
                                                         "VLAN_MGR_SetAccessMode"
                                                        );
                            return FALSE;
                        } /* end of if */
                    }
                } /* end of if */
            } /* end of if */
        } /* end of check current table */

        /* check static table */
        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, lport_ifindex))
        {
            if (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, lport_ifindex))
            {
                /* join the static untagged list of joined vlan */
                is_member_changed = TRUE;
                VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, lport_ifindex);
            }
        } /* end of check static table */

        if (is_member_changed)
        {
            if (!VLAN_OM_SetVlanEntry(&vlan_info))
            {
                char    arg_buf[15];
                snprintf(arg_buf, 15, "VLANs (%d)",SYS_ADPT_MAX_NBR_OF_VLAN);
                EH_MGR_Handle_Exception1(SYS_MODULE_VLAN,
                                         VLAN_MGR_SetAccessMode_Fun_No,
                                         EH_TYPE_MSG_MAXIMUM_EXCEEDED,
                                         SYSLOG_LEVEL_INFO,
                                         arg_buf
                                        );
                return FALSE;
            } /* end of if */
        } /* end of if */
    } /* end of while */

    if (!VLAN_OM_SetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_VLAN,
                                 VLAN_MGR_SetAccessMode_Fun_No,
                                 EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                 "lport (1-SYS_ADPT_TOTAL_NBR_OF_LPORT"
                                );
        return FALSE;
    } /* end of if */

    return TRUE;
} /* End of VLAN_MGR_SetAccessMode() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_LocalSetVlanRowStatus
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
 *          : vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The API is admited entering/leaving critical section.
 *------------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_LocalSetVlanRowStatus(UI32_T vid, UI32_T row_status, UI32_T vlan_status)
{
    UI32_T  vid_ifindex, port_num;

    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    UI32_T                              member_option = 0;
    char                                arg_buf[15];
    #if 0
    UI8_T                               cpu_mac[SYS_ADPT_MAC_ADDR_LEN];
    #endif
    /*added by Jinhua Wei ,to remove warning ,becaued above array never used*/

   /* BODY */
    snprintf(arg_buf, 15, "VLANs (%d)",SYS_ADPT_MAX_NBR_OF_VLAN);

    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        BACKDOOR_MGR_Printf("VLAN_MGR_SetDot1qVlanRowStatus vid: %d, row status: %d, vlan_status: %d \n",(UI16_T) vid,(UI16_T) row_status,(UI16_T) vlan_status);


    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    switch (row_status)
    {
        /* createAndGo and createAndWait should be processed by VLAN_MGR_SetDot1qVlanStaticEntry()
         */
        case VAL_dot1qVlanStaticRowStatus_createAndGo:
        case VAL_dot1qVlanStaticRowStatus_createAndWait:
            return FALSE;
            break;

        case VAL_dot1qVlanStaticRowStatus_active:
        case VAL_dot1qVlanStaticRowStatus_notInService:
        case VAL_dot1qVlanStaticRowStatus_destroy:
            /* returns false if vlan_info can not be retrieve from the database
             */
            if (!VLAN_OM_GetVlanEntry(&vlan_info))
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_LocalSetVlanRowStatus_Fun_No,
                                            EH_TYPE_MSG_NOT_EXIST,
                                            SYSLOG_LEVEL_INFO,
                                            "VLAN entry"
                                        );
                return FALSE;
            } /* End of if */

            if ((vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_permanent) &&
               (vlan_status == VAL_dot1qVlanStatus_dynamicGvrp))
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_LocalSetVlanRowStatus_Fun_No,
                                            EH_TYPE_MSG_FAILED_TO_SET,
                                            SYSLOG_LEVEL_INFO,
                                            "vlan row status by GVRP operation"
                                        );
                return FALSE;
            }
            break;

        /* According to definition for Row-Status Textual Convention, the status
           column has six defined values: "active, notnService, notReady, createAndGo,
           createAndWait, and destroy".  Only 5 of the six values (except 'notReady')
           may be specified in a management protocol set operation.

         */
        default:
            return FALSE;
            break;
    } /* end of switch */

    /* Pass vlan_info into FSM to check and update dot1q_vlan_static_row_status field.
       If transition of dot1q_vlan_static_row_status is correct, then update vlan_info
       into database.  Otherwise, return false.

       ACTION  | ACTIVE | Suspand   | Destroy
       -------------------------------------
       vlan om | Exist  | Exist     | Not Exist
       ASIC    | Exist  | Not Exist | Not Exist

     */

    switch (L_RSTATUS_Fsm(row_status,
                          &vlan_info.dot1q_vlan_static_row_status,
                          VLAN_MGR_SemanticCheck,
                          (void*)&vlan_info))
    {
        case L_RSTATUS_NOTEXIST_2_ACTIVE:
        case L_RSTATUS_NOTEXIST_2_NOTREADY:
            return FALSE;
            break;

        /* Delete entry from database
         */
        case L_RSTATUS_NOTREADY_2_NOTEXIST:

            if (!VLAN_MGR_VlanRemovable(vid))
            {
                if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                    BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_VlanRemovable in VLAN_MGR_LocalSetVlanRowStatus for case NotReady->NotExist\n");
                return FALSE;
            } /* end of if */

            port_num    = 0;
            while (SWCTRL_GetNextLogicalPort(&port_num) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, port_num))
                {
                    if (!VLAN_MGR_RemoveVlanMemberForDeletingVlan(vlan_info.dot1q_vlan_index,port_num, vlan_status))
                    {
                        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                            BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_RemoveVlanMember in VLAN_MGR_LocalSetVlanRowStatus for case NotReady->NotExist\n");
                        return FALSE;
                    }

                    if (!VLAN_MGR_ProcessOrphanPortByLport(port_num))
                    {
                        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                            BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_ProcessOrphanPortByLport in VLAN_MGR_DeleteEgressPortMember\n");
                        return FALSE;
                    }
                }
            } /* end of for */

            if (!VLAN_OM_DeleteVlanEntry(vid))
            {
                if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                    BACKDOOR_MGR_Printf("ERROR: VLAN_OM_DeleteVlanEntry in VLAN_MGR_LocalSetVlanRowStatus for case NotReady->NotExist\n");
                return FALSE;
            } /* end of if */
            break;

        /* Update Database and Delete from chip
         */
        case L_RSTATUS_ACTIVE_2_NOTREADY:
            if (vid == VLAN_OM_GetGlobalDefaultVlan())
            {
                return FALSE;
            }

            /* Management VLAN cannot be removed.
             */
            if (    (vlan_info.vlan_ip_state != VLAN_MGR_IP_STATE_NONE)
                &&  (is_provision_complete == TRUE)
               )
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_LocalSetVlanRowStatus_Fun_No,
                                            EH_TYPE_MSG_FAILED_TO_DELETE,
                                            SYSLOG_LEVEL_INFO,
                                            "management VLAN"
                                        );
                return FALSE;
            }

            /* Need to remove all member port from ASIC before vlan entry can be remove from chip.
             */
            port_num    = 0;
            while (SWCTRL_GetNextLogicalPort(&port_num) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, port_num))
                {
                    if (!VLAN_MGR_RemoveVlanMember(vlan_info.dot1q_vlan_index,port_num,VLAN_MGR_BOTH,L_RSTATUS_ACTIVE_2_NOTREADY, vlan_status))
                    {
                        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                            BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_RemoveVlanMember in VLAN_MGR_LocalSetVlanRowStatus for case ACTIVE->NOTREADY\n");
                        return FALSE;
                    }

                    VLAN_MGR_Notify_VlanMemberDelete(vlan_info.dot1q_vlan_index, port_num, vlan_info.dot1q_vlan_status);
                    /*EPR:ES3628BT-FLF-ZZ-00516
                     *problem:system:set the vlan 5 suspended cause DUT1 display erorr message and lost  the lacp trunk.
                     *Root Cause: When suspend the vlan 5 will make the port remove from vlan 5 and destory
                     *            the vlan 5,but the port's pvid is still vlan 5,so make the packet could not
                     *            be received!
                     *Solution: When do this case,need set the port pvid to global default value,make the packet could be
                     *          received
                     *Modify file:vlan_mgr.c
                     *Fixed by:DanXie
                     */
                    vlan_port_info.lport_ifindex = port_num;
                    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
                    {
                        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                                    VLAN_MGR_LocalSetVlanRowStatus_Fun_No,
                                                    EH_TYPE_MSG_NOT_EXIST,
                                                    SYSLOG_LEVEL_INFO,
                                                    "VLAN port entry"
                                                );
                        return FALSE;
                    }
                    if (vlan_port_info.port_item.dot1q_pvid_index == vid_ifindex)
                    {
#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
                        if (!VLAN_MGR_SetDot1qPvidWithoutChangingMembership(port_num, VLAN_OM_GetGlobalDefaultVlan(),TRUE))
#else
                        if (!SWCTRL_SetPortPVID(port_num, VLAN_OM_GetGlobalDefaultVlan(),TRUE))
#endif
                        {
                            return FALSE;
                        }
                    }/* End of if (vlan_port_info.port_item.dot1q_pvid_index == vid_ifindex) */
                } /* end of if */
            } /* end of for */

            if (!VLAN_MGR_RemoveVlan(vlan_info.dot1q_vlan_index, L_RSTATUS_ACTIVE_2_NOTREADY))
            {
                if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                    BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_RemoveVlan in VLAN_MGR_LocalSetVlanRowStatus for case ACTIVE->NOTREADY\n");
                return FALSE;
            } /* end of if */

            VLAN_MGR_Notify_VlanActiveToSuspend(&vlan_info, vlan_status);

            break;

        /* Update Database and Add to Chip
         */
        case L_RSTATUS_NOTREADY_2_ACTIVE:
            if (!SWCTRL_CreateVlan(vid))
            {
                VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING,
                                             FUNCTION_RETURN_FAIL_INDEX,
                                             "VLAN_MGR_LocalSetVlanRowStatus");
                if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                    BACKDOOR_MGR_Printf("ERROR: SWCTRL_CreateVlan in VLAN_MGR_LocalSetVlanRowStatus for case NotReady->Active\n");

                return FALSE;
            } /* end of if */

#if 0 /* VaiWang, Thursday, March 27, 2008 10:12:12 */
            if (SWCTRL_GetCpuMac(cpu_mac) == FALSE)
            {
                printf("\r\nSWCTRL_GetCpuMac() returns error!\r\n");
                return FALSE;
            }
            if (VLAN_MGR_CreateVlanDev(vid, cpu_mac) != VLAN_TYPE_RETVAL_OK)
            {
                printf("\r\nVLAN_MGR_CreateVlanDev() returns error!\r\n");
                return FALSE;
            }
#endif /*  ACCTON_METRO */

            if (!VLAN_OM_SetVlanEntry(&vlan_info))
            {
                if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                    BACKDOOR_MGR_Printf("ERROR: VLAN_OM_SetVlanEntry in VLAN_MGR_LocalSetVlanRowStatus for case NotReady->Active\n");

                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_LocalSetVlanRowStatus_Fun_No,
                                            EH_TYPE_MSG_MAXIMUM_EXCEEDED,
                                            SYSLOG_LEVEL_INFO,
                                            arg_buf
                                        );
                return FALSE;
            } /* end of if */

            VLAN_MGR_Notify_VlanSuspendToActive(vid_ifindex, vlan_info.dot1q_vlan_status);

            port_num    = 0;
            while (SWCTRL_GetNextLogicalPort(&port_num) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, port_num))
                {
                    UI32_T                          pvid;

                    if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, port_num))
                        member_option = VLAN_MGR_BOTH;
                    else
                        member_option = VLAN_MGR_TAGGED_ONLY;

                    if (!VLAN_MGR_AddVlanMember(vlan_info.dot1q_vlan_index,port_num, member_option, VAL_dot1qVlanStatus_permanent))
                    {
                        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                            BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_AddVlanMember in VLAN_MGR_LocalSetVlanRowStatus for case NotReady->ACTIVE\n");

                        return FALSE;
                    } /* end of if */
                    vlan_port_info.lport_ifindex = port_num;
                    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
                    {
                        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                                    VLAN_MGR_LocalSetVlanRowStatus_Fun_No,
                                                    EH_TYPE_MSG_NOT_EXIST,
                                                    SYSLOG_LEVEL_INFO,
                                                    "VLAN port entry"
                                                );
                        return FALSE;
                    }
                    VLAN_MGR_Notify_VlanMemberAdd(vid_ifindex, port_num, vlan_info.dot1q_vlan_status);
                    VLAN_IFINDEX_CONVERTTO_VID(vlan_port_info.port_item.dot1q_pvid_index, pvid);
#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
                    if (!VLAN_MGR_SetDot1qPvidWithoutChangingMembership(port_num, pvid,TRUE))
#else
                    if (!SWCTRL_SetPortPVID(port_num, pvid,TRUE))
#endif
                    {
                        return FALSE;
                    }
                } /* end of if */
            } /* end of for */

            if (!VLAN_OM_GetVlanEntry(&vlan_info))
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_LocalSetVlanRowStatus_Fun_No,
                                            EH_TYPE_MSG_NOT_EXIST,
                                            SYSLOG_LEVEL_INFO,
                                            "VLAN entry"
                                        );
                return FALSE;
            } /* End of if */

            break;

        /*  No Change
         */
        case L_RSTATUS_ACTIVE_2_ACTIVE:
        case L_RSTATUS_NOTREADY_2_NOTREADY:
            break;

        /* Delete from Database and Chip
         */
        case L_RSTATUS_ACTIVE_2_NOTEXIST:
            if (vid == VLAN_OM_GetGlobalDefaultVlan())
            {
                return FALSE;
            }

            /* Management VLAN cannot be removed.
             */
            if (    (vlan_info.vlan_ip_state != VLAN_MGR_IP_STATE_NONE)
                &&  (is_provision_complete == TRUE)
               )
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_LocalSetVlanRowStatus_Fun_No,
                                            EH_TYPE_MSG_FAILED_TO_DELETE,
                                            SYSLOG_LEVEL_INFO,
                                            "management VLAN"
                                        );
                return FALSE;
            }

            if (!VLAN_MGR_VlanRemovable(vid))
            {
                if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
                    BACKDOOR_MGR_Printf("Designated pvid vlan shall not be removed.\n");
                return FALSE;
            } /* end of if */

            port_num    = 0;
            while (SWCTRL_GetNextLogicalPort(&port_num) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, port_num))
                {
                    if (!VLAN_MGR_RemoveVlanMemberForDeletingVlan(vlan_info.dot1q_vlan_index,port_num, vlan_status))
                    {
                        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                            BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_RemoveVlanMember in VLAN_MGR_LocalSetVlanRowStatus for case Active->NotExist\n");
                        return FALSE;
                    }

                    VLAN_MGR_Notify_VlanMemberDelete(vlan_info.dot1q_vlan_index, port_num, vlan_info.dot1q_vlan_status);
                    if (!VLAN_MGR_ProcessOrphanPortByLport(port_num))
                    {
                        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                            BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_ProcessOrphanPortByLport in VLAN_MGR_DeleteEgressPortMember\n");
                        return FALSE;
                    }
                }
            } /* end of for */

            if (!VLAN_MGR_RemoveVlan(vlan_info.dot1q_vlan_index,L_RSTATUS_ACTIVE_2_NOTEXIST))
            {
                if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                    BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_RemoveVlan in VLAN_MGR_LocalSetVlanRowStatus for case Active->NotExist\n");
                return FALSE;
            } /* end of if */
            VLAN_MGR_Notify_VlanDestroy(&vlan_info, vlan_status);
            break;

        /* Error condition.
         */
        case L_RSTATUS_NOTEXIST_2_NOTEXIST:
        case L_RSTATUS_TRANSITION_STATE_ERROR:
            return FALSE;
        default:
            VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING,
                                         SWITCH_TO_DEFAULT_MESSAGE_INDEX,
                                         "VLAN_MGR_LocalSetVlanRowStatus");
            return FALSE;
    } /* end of switch */

    return TRUE;
} /* end of VLAN_MGR_LocalSetVlanRowStatus() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_VlanRemovable
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if none of the port member in the
 *            vlan port list has its pvid set to the current vlan.
 *            Otherwise, return FALSE.
 * INPUT    : vid - the specific vlan
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_VlanRemovable(UI32_T vid)
{
    UI32_T  port_num;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    UI32_T                              vid_ifindex;
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;

    /* BODY */
    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_VlanRemovable_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        return FALSE;
    } /* end of if */

    port_num    = 0;
    while (SWCTRL_GetNextLogicalPort(&port_num) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        vlan_port_info.lport_ifindex = port_num;
        if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
        {
            EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                        VLAN_MGR_VlanRemovable_Fun_No,
                                        EH_TYPE_MSG_NOT_EXIST,
                                        SYSLOG_LEVEL_INFO,
                                        "VLAN port entry"
                                    );
            return FALSE;
        }

       /* if this VLAN is the PVID VLAN of any member, it cannot be deleted
        */
        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, port_num))
        {
            if (vlan_port_info.port_item.dot1q_pvid_index == vid_ifindex)
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_VlanRemovable_Fun_No,
                                            EH_TYPE_MSG_INVALID,
                                            SYSLOG_LEVEL_INFO,
                                            "port that is member of pvid vlan"
                                        );
                return FALSE;
            }

#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
            if (VAL_vlanPortMode_dot1qTrunk == vlan_port_info.vlan_port_entry.vlan_port_mode)
            {
                VLAN_OM_Dot1qVlanCurrentEntry_T     tmp_vlan_info;
                BOOL_T                              found_vlan;

                /* it is not allowable to delete VLAN when delete it would cause
                 * orphan port which is q-trunk port, because original mechanism
                 * cannot add a q-trunk orphan port back to default VLAN
                 */
                tmp_vlan_info->dot1q_vlan_index = 0;
                found_vlan = FALSE;
                while (VLAN_OM_GetNextVlanEntry(&tmp_vlan_info))
                {
                    if (    (tmp_vlan_info.dot1q_vlan_index != vid_ifindex)
                        &&  (VLAN_OM_IS_MEMBER(tmp_vlan_info.dot1q_vlan_static_egress_ports, port_num))
                       )
                    {
                        found_vlan = TRUE;
                        break;
                    }
                }
                if (!found_vlan)
                {
                    return FALSE;
                }
            }
#endif
        }

        /* if the VLAN owns authorized port, it cannot be deleted
         */
        if (vlan_port_info.port_item.auto_vlan_mode)
        {
            if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, port_num))
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_VlanRemovable_Fun_No,
                                            EH_TYPE_MSG_INVALID,
                                            SYSLOG_LEVEL_INFO,
                                            "VLAN with authorized port member"
                                        );
                return FALSE;
            }
            else if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, port_num))
            {
#if (SYS_CPNT_VLAN_ALLOW_AUTHORIZED_PORT_CONFIGURED == TRUE)
                if (vlan_port_info.port_item.admin_pvid == vid)
#endif
                {
                    EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                                VLAN_MGR_VlanRemovable_Fun_No,
                                                EH_TYPE_MSG_INVALID,
                                                SYSLOG_LEVEL_INFO,
                                                "port that is member of pvid vlan"
                                            );
                    return FALSE;
                }
            }
        }
    }
    return TRUE;
} /* end of VLAN_MGR_VlanRemovable() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_PortRemovable
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if none of the port member in the
 *            vlan port list has its pvid set to the current vlan.
 *            Otherwise, return FALSE.
 * INPUT    : vid_ifindex - the specific vlan
 *            vlan_egress_portlist - egress port list of the vid
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_PortRemovable(UI32_T vid_ifindex, UI32_T lport_ifindex)
{
    VLAN_OM_Vlan_Port_Info_T        vlan_port_info;

    /* BODY */

    vlan_port_info.lport_ifindex = lport_ifindex;

    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_PortRemovable_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        return FALSE;
    } /* End of if */

#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
    /* Can not remove Q-Trunk member
     */
    if (vlan_port_info.vlan_port_entry.vlan_port_trunk_link_mode == VLAN_MGR_TRUNK_LINK)
    {
#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
        UI32_T  default_vlan;
        UI32_T  default_vlan_ifindex;

        default_vlan = VLAN_OM_GetGlobalDefaultVlan();
        VLAN_VID_CONVERTTO_IFINDEX(default_vlan, default_vlan_ifindex);
        if (vid_ifindex != default_vlan_ifindex)
        {
            return FALSE;
        }
#else
        return FALSE;
#endif
    } /* End of if */
#endif

#if (SYS_CPNT_VLAN_NO_CHECK_VLAN_FOR_PVID == FALSE)
    /* Can not remove pvid vlan
     */
    if (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_access)
    {
        if (!vlan_port_info.port_item.auto_vlan_mode)
        {
            if (vlan_port_info.port_item.dot1q_pvid_index == vid_ifindex)
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_PortRemovable_Fun_No,
                                            EH_TYPE_MSG_INVALID,
                                            SYSLOG_LEVEL_INFO,
                                            "port that is member of pvid vlan"
                                        );
                return FALSE;
            }
        }
        else
        {
            UI32_T  vid;
            VLAN_IFINDEX_CONVERTTO_VID(vid_ifindex, vid);
            if (vlan_port_info.port_item.admin_pvid == vid)
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_PortRemovable_Fun_No,
                                            EH_TYPE_MSG_INVALID,
                                            SYSLOG_LEVEL_INFO,
                                            "port that is member of pvid vlan"
                                        );
                return FALSE;
            }
        }
    }
#endif

    return TRUE;

} /* end of VLAN_MGR_PortRemovable() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_ValidateVlanFields
 *--------------------------------------------------------------------------
 * PURPOSE  : This function checks if the fields enter for vlan is valid before
 *            they are set to vlan_om.
 * INPUT    : vlan_info - a specific vlan entry waiting to be validate
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_ValidateVlanFields(VLAN_MGR_Dot1qVlanStaticEntry_T *vlan_info)
{
    UI32_T  port_num, byte;
    VLAN_OM_Vlan_Port_Info_T      vlan_port_info;

    /* BODY */


    /* Instances that this function returns FALSE:
       1. Conflict between Forbidden_list and Egress_list.
       2. One of the member port is a trunk_member.
       3. One of the member is in Q-Trunk mode and it is specify to join untagged member list.
       4. One of the member is in Access mode and it is specify to join tagged member list.
       5. Member of untagged portlist must also be member of static portlist.
     */

    /* condition 1 */
    for (byte = 0; byte < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; byte++)
    {
        if ((vlan_info->dot1q_vlan_static_egress_ports[byte] &
            vlan_info->dot1q_vlan_forbidden_egress_ports[byte]) != 0)
        {
            EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                        VLAN_MGR_ValidateVlanFields_Fun_No,
                                        EH_TYPE_MSG_INVALID, SYSLOG_LEVEL_INFO,
                                        "VLAN information"
                                    );
            return FALSE;
        }
    } /* end of for */


    /* Condition 2 and 3 */
    port_num    = 0;
    while (SWCTRL_GetNextLogicalPort(&port_num) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        if (VLAN_OM_IS_MEMBER(vlan_info->dot1q_vlan_static_egress_ports, port_num))
        {
            vlan_port_info.lport_ifindex = port_num;
            if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_ValidateVlanFields_Fun_No,
                                            EH_TYPE_MSG_NOT_EXIST,
                                            SYSLOG_LEVEL_INFO,
                                            "VLAN port entry"
                                        );
                return FALSE;
            }

            /* Condition 2 */
            if (vlan_port_info.port_trunk_mode == TRUE)
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_ValidateVlanFields_Fun_No,
                                            EH_TYPE_MSG_INVALID,
                                            SYSLOG_LEVEL_INFO,
                                            "port that is a trunk_member"
                                        );
                return FALSE;
            }
            /* Condition 3*/
            if (    (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_dot1qTrunk)
                &&  (VLAN_OM_IS_MEMBER(vlan_info->dot1q_vlan_static_untagged_ports, port_num))
               )
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_ValidateVlanFields_Fun_No,
                                            EH_TYPE_MSG_INVALID,
                                            SYSLOG_LEVEL_INFO,
                                            "port that is in Q-Trunk mode and it is specify to join untagged member list"
                                        );
                return FALSE;
            }
            /* Condition 4 */
#if (SYS_CPNT_VLAN_ACCESS == TRUE)
            if ((vlan_port_info.port_trunk_mode == VAL_vlanPortMode_access) &&
                (!VLAN_OM_IS_MEMBER(vlan_info->dot1q_vlan_static_untagged_ports, port_num)))
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_ValidateVlanFields_Fun_No,
                                            EH_TYPE_MSG_INVALID,
                                            SYSLOG_LEVEL_INFO,
                                            "port that is in Access mode and it is specify to join tagged member list"
                                        );
                return FALSE;
            }
#endif

        } /* end of if */
        else
        {
            /* Condition 5*/
            if (VLAN_OM_IS_MEMBER(vlan_info->dot1q_vlan_static_untagged_ports, port_num))
            {
                EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                            VLAN_MGR_ValidateVlanFields_Fun_No,
                                            EH_TYPE_MSG_INVALID,
                                            SYSLOG_LEVEL_INFO,
                                            "port that is not member of static portlist"
                                        );
                return FALSE;
            }
        }

    } /* end of for */

    return TRUE;

} /* end of VLAN_MGR_ValidateVlanFields() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_IsPrivateVlanPortMember
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns TRUE if this port has been configured as
 *            a private port and is member of the specified private vlan.
 *            Otherwise, return FALSE.
 * INPUT    : vid_ifindex     -- vlan index number
 *            lport_ifindex   -- the specified port
 * OUTPUT   : none
 * RETURN   : TRUE / FALSE
 * NOTES    : 1. Private port is forbidden to participate in normal vlan
 *               operation as well as port attribute configuration.
 *            2. Private port field will be modify as result of pvlan_mgr activity.
 *--------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_IsPrivateVlanPortMember(UI32_T vid_ifindex, UI32_T lport_ifindex)
{
    return FALSE;
} /* end of VLAN_MGR_IsPrivateVlanPortMember() */

#if (SYS_CPNT_VLAN_PROVIDING_AT_LEAST_ONE_UNTAGGED_MODE == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_ConformToAtLeastOneUntaggedVlan
 *------------------------------------------------------------------------------
 * PURPOSE  : This function checks if a port joins at least one untagged member
 *            of some vlan. If not, join as untagged member of default vlan.
 * INPUT    : lport       -- the port to be checked
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. Use when SYS_CPNT_VLAN_PROVIDING_AT_LEAST_ONE_UNTAGGED_MODE is TRUE.
 *            2. The API is inhibited from entering/leaving critical section.
 *------------------------------------------------------------------------------
 */
static BOOL_T VLAN_MGR_ConformToAtLeastOneUntaggedVlan(UI32_T lport)
{
    VLAN_OM_Vlan_Port_Info_T vlan_port_info;
    UI32_T                   default_vlan;

    vlan_port_info.lport_ifindex = lport;
    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        return FALSE;
    }

    if (vlan_port_info.port_item.untagged_joined_vlan_count >= 1)
    {
        return TRUE;
    }

    else
    {
        if (!VLAN_OM_GetGlobalDefaultVlan_Ex(&default_vlan))
        {
            return FALSE;
        }

        if (!VLAN_MGR_AddUntagPortMember(default_vlan, lport, VAL_dot1qVlanStatus_permanent))
        {
            return FALSE;
        }

        return TRUE;
    }
} /* End of VLAN_MGR_ConformToAtLeastOneUntaggedVlan() */
#endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_AT_LEAST_ONE_UNTAGGED_MODE == TRUE) */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_ConformToSingleUntaggedVlan
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if successfully to meet single untagged
 *            rule. Otherwise, return false.
 * INPUT    : join_vlan   -- the joining vlan id
 *            lport       -- the joining port
 *          : vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. Use when SYS_CPNT_VLAN_PROVIDING_SINGLE_UNTAGGED_MODE is TRUE.
 *            2. The API is inhibited from entering/leaving critical section.
 *------------------------------------------------------------------------------
 */
static BOOL_T VLAN_MGR_ConformToSingleUntaggedVlan(UI32_T join_vlan, UI32_T lport, UI32_T vlan_status)
{
    BOOL_T                              result;
    UI32_T                              vid;
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;

    if (is_authenticating)
    {
        result = VLAN_MGR_SetAuthorizedPvid(lport, join_vlan);
    }
    else
    {
        result = VLAN_MGR_SetDot1qPvid(lport, join_vlan);
    } /* end of if */


    vlan_info.dot1q_vlan_index = 0;

    while (result && VLAN_OM_GetNextVlanEntry(&vlan_info))
    {
        VLAN_IFINDEX_CONVERTTO_VID(vlan_info.dot1q_vlan_index, vid);

        if (join_vlan == vid)
        {
            continue;
        }

        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport))
        {
            if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport))
            {
                if (!VLAN_MGR_DeleteEgressPortMember(vid, lport, vlan_status))
                {
                    result = FALSE;
                } /* end of if */
            } /* end of if */
        } /* end of if */
    } /* end of while */

    return result;
} /* End of VLAN_MGR_ConformToSingleUntaggedVlan() */

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_ConformToSingleMode
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if successfully to meet single mode
 *            rule. Otherwise, return false.
 * INPUT    : join_vlan   -- the joining vlan id
 *            lport       -- the joining port
 *          : vlan_status -- VAL_dot1qVlanStatus_other \
 *                           VAL_dot1qVlanStatus_permanent \
 *                           VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : The API is inhibited from entering/leaving critical section.
 *------------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_ConformToSingleMode(UI32_T join_vlan, UI32_T lport, UI32_T vlan_status, BOOL_T tagged)
{
    BOOL_T                              result = TRUE;
    UI32_T                              pvid = 0;
    UI32_T                              pvid_ifindex = 0;
    UI32_T                              default_vlan;
    UI32_T                              vid = 0;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    VLAN_OM_Dot1qVlanCurrentEntry_T     pvid_vlan_info;

    vlan_port_info.lport_ifindex = lport;
    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        return FALSE;
    }

    pvid_ifindex = vlan_port_info.port_item.dot1q_pvid_index;
    VLAN_IFINDEX_CONVERTTO_VID(pvid_ifindex, pvid);
    pvid_vlan_info.dot1q_vlan_index = pvid_ifindex;
    if (!VLAN_OM_GetVlanEntry(&pvid_vlan_info))
    {
        return FALSE;
    }

    if (!VLAN_OM_GetGlobalDefaultVlan_Ex(&default_vlan))
    {
        return FALSE;
    }

    if (   (TRUE == tagged)
        && (join_vlan == default_vlan)
       )
    {
        /* The default vlan should contain all ports in its untagged list,
         * so DO NOT allow port being tagged when this port is member of default VLAN.
         */
        result = FALSE;
        VLAN_MGR_DeleteEgressPortMember(join_vlan, lport, vlan_status);
    }
    else if (!(vlan_port_info.vlan_port_entry.vlan_port_dual_mode))
    {
        VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;


        vlan_info->dot1q_vlan_index = 0;

        while (VLAN_OM_GetNextVlanEntry(&vlan_info))
        {
            VLAN_IFINDEX_CONVERTTO_VID(vlan_info.dot1q_vlan_index, vid);
            if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport))
            {
                if (    (TRUE == tagged)
                    &&  (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport))
                   )
                {
                    /* leave all VLAN join with untagged type
                    */
                    if (!VLAN_MGR_DeleteEgressPortMember(vid, lport, vlan_status))
                    {
                        result = FALSE;
                        break;
                    }
                }
                else if (   (FALSE == tagged)
                         && (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport))
                        )
                {
                    /* leave all VLAN join with tagged type
                    */
                    if (!VLAN_MGR_DeleteEgressPortMember(vid, lport, vlan_status))
                    {
                        result = FALSE;
                        break;
                    }
                }
            }
        } /* end while */
        if (result)
        {
            if (tagged)
            {
                if (!VLAN_MGR_SetDot1qPortAcceptableFrameTypes(lport, VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanTagged))
                {
                    result = FALSE;
                }
            }
            else
            {
                if (!VLAN_MGR_SetDot1qPortAcceptableFrameTypes(lport, VAL_dot1qPortAcceptableFrameTypes_admitAll))
                {
                    result = FALSE;
                }
            }
        }
    }
    else
    {
        if (vlan_port_info.vlan_port_entry.dual_mode_vlan_id != pvid)
        {
            vlan_port_info.port_item.dot1q_pvid_index = pvid_ifindex;
            vlan_port_info.vlan_port_entry.vlan_port_dual_mode = FALSE;
            vlan_port_info.vlan_port_entry.dual_mode_vlan_id    = 0;

            if (!VLAN_OM_SetVlanPortEntry(&vlan_port_info))
            {
                result = FALSE;
            }
        }
    }
    return result;
}

static BOOL_T VLAN_MGR_IsPortDualModeOnVlan(UI32_T lport_ifindex, UI32_T dual_mode_vlan_id)
{
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    BOOL_T                              ret = FALSE;

    vlan_port_info.lport_ifindex = lport_ifindex;
    if (VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        if (    (vlan_port_info.vlan_port_entry.vlan_port_dual_mode)
            &&  (dual_mode_vlan_id == vlan_port_info.vlan_port_entry.dual_mode_vlan_id)
            )
        {
            ret = TRUE;
        }
    }

    return ret;
}
#endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)*/

#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
static BOOL_T VLAN_MGR_LocalIsQTrunkPortMember(UI32_T lport_ifindex)
{
    VLAN_OM_Vlan_Port_Info_T    vlan_port_info;

    /* BODY */

    vlan_port_info.lport_ifindex = lport_ifindex;

    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        return FALSE;
    } /* end of if */

    if (vlan_port_info.vlan_port_entry.vlan_port_trunk_link_mode == VLAN_MGR_TRUNK_LINK)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

} /* end of VLAN_MGR_LocalIsQTrunkPortMember() */


static BOOL_T VLAN_MGR_LocalSetPortTrunkLinkMode(UI32_T lport_ifindex, UI32_T trunk_link_mode)
{
    VLAN_OM_Vlan_Port_Info_T    vlan_port_info;

    /* BODY */

    if ((trunk_link_mode < VLAN_MGR_NOT_TRUNK_LINK) &&
        (trunk_link_mode > VLAN_MGR_TRUNK_LINK))
    {
        return FALSE;
    }

    vlan_port_info.lport_ifindex = lport_ifindex;

    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        return FALSE;
    } /* end of if */

     vlan_port_info.vlan_port_entry.vlan_port_trunk_link_mode = trunk_link_mode;

    if (!VLAN_OM_SetVlanPortEntry(&vlan_port_info))
    {
        return FALSE;
    } /* end of if */

    return TRUE;

} /* end of VLAN_MGR_LocalSetPortTrunkLinkMode() */


/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_LocalAddPortTrunkLinkToVlan
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if successfully to add trunk link port
 *            to specified VLAN. Otherwise, return false.
 * INPUT    : vid_ifindex -- the joining vid_ifindex
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The API is admited entering/leaving critical section.
 *------------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_LocalAddPortTrunkLinkToVlan(UI32_T vid_ifindex)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    UI32_T                              vid;
    UI32_T                              port_num;
    BOOL_T                              is_add_member = TRUE;
    BOOL_T                              member_tag_changed;

    /* BODY
     */


    VLAN_IFINDEX_CONVERTTO_VID(vid_ifindex, vid);
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        return FALSE;
    } /* end of if */

    if (vlan_info.dot1q_vlan_status != VAL_dot1qVlanStatus_permanent)
    {
        return TRUE;
    } /* end of if */

    port_num    = 0;
    while (SWCTRL_GetNextLogicalPort(&port_num) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        if (VLAN_MGR_LocalIsQTrunkPortMember(port_num) == TRUE)
        {
            is_add_member = FALSE;
            member_tag_changed = FALSE;
            if (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, port_num))
            {
                is_add_member = TRUE;
                if (!VLAN_MGR_AddVlanMember(vlan_info.dot1q_vlan_index, port_num, VLAN_MGR_TAGGED_ONLY, VAL_dot1qVlanStatus_permanent))
                {
                    if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                        BACKDOOR_MGR_Printf("ERROR: VLAN_MGR_AddVlanMember in VLAN_MGR_LocalAddPortTrunkLinkToVlan\n");

                    return FALSE;
                } /* end of if */

                if (!VLAN_OM_GetVlanEntry(&vlan_info))
                {
                    return FALSE;
                } /* end of if */
            }
            else if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, port_num))
            {
                if (is_provision_complete == TRUE)
                {
                    if(!SWCTRL_DeletePortFromVlanUntaggedSet(port_num, vid,TRUE))
                    {
                        return FALSE;
                    } /* end of if */
                }
                VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, port_num);
                vlan_port_info.port_item.untagged_joined_vlan_count--;
                member_tag_changed = TRUE;
            }

            if (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, port_num))
            {
                VLAN_OM_ADD_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, port_num);
                vlan_port_info.port_item.static_joined_vlan_count++;
            }
            else if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, port_num))
            {
                VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, port_num);
            }

            VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_forbidden_egress_ports, port_num);
            vlan_info.dot1q_vlan_time_mark = SYS_TIME_GetSystemTicksBy10ms();
            if (!VLAN_OM_SetVlanEntry(&vlan_info))
            {
                return FALSE;
            }

            VLAN_OM_SetVlanPortEntry(&vlan_port_info);

            VLAN_OM_EnterCriticalSection();
            SYS_TIME_GetSystemUpTimeByTick(VLAN_OM_GetIfStackLastUpdateTimePtr());
            VLAN_OM_LeaveCriticalSection();

            if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
            {
                if (is_add_member)
                {
                    VLAN_MGR_Notify_VlanMemberAdd(vlan_info.dot1q_vlan_index, port_num, VAL_dot1qVlanStatus_permanent);
                }

                if (member_tag_changed == TRUE)
                {
                    VLAN_MGR_Notify_VlanMemberTagChanged(vlan_info.dot1q_vlan_index, port_num);
                }
            }
        } /* end of if */
    } /* end of while */

    return TRUE;
} /* end of VLAN_MGR_LocalAddPortTrunkLinkToVlan() */

static BOOL_T VLAN_MGR_LocalSetDot1qPvidToDefault(UI32_T lport_ifindex)
{
    UI32_T                              unit, port, trunk_id;
    UI32_T                              default_vlan, default_vlan_ifindex;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;

    /* BODY */
    default_vlan = VLAN_OM_GetGlobalDefaultVlan();
    if (!VLAN_MGR_SetDot1qPvidWithoutChangingMembership(lport_ifindex, default_vlan,TRUE))
    {
        VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING,
                                     FUNCTION_RETURN_FAIL_INDEX,
                                     "VLAN_MGR_LocalSetDot1qPvidToDefault");
        return FALSE;
    }

    vlan_port_info.lport_ifindex = lport_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        return FALSE;
    } /* end of if */

    VLAN_VID_CONVERTTO_IFINDEX(default_vlan, default_vlan_ifindex);
    vlan_port_info.port_item.dot1q_pvid_index = default_vlan_ifindex;
    if (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_dot1qTrunk)
    {
        vlan_port_info.port_item.dot1q_port_acceptable_frame_types = VAL_dot1qPortAcceptableFrameTypes_admitAll;
    } /* end of if */

    if (!VLAN_OM_SetVlanPortEntry(&vlan_port_info))
        return FALSE;

    if (SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit, &port, &trunk_id) == SWCTRL_LPORT_TRUNK_PORT)
    {
        if (!VLAN_MGR_SetTrunkMemberPortInfo(lport_ifindex, PVID_FIELD, default_vlan))
            return FALSE;
    } /* end of if */

    return TRUE;

} /* End of VLAN_MGR_LocalSetDot1qPvidToDefault() */

#endif /* SYS_CPNT_Q_TRUNK_MEMBER == TRUE */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetVlanPortEntryToDefault
 * ------------------------------------------------------------------------
 * PURPOSE  : Set VLAN port info by default value
 * INPUT    : lport_ifindex
 * OUTPUT   : None
 * RETURN   : None
 * ------------------------------------------------------------------------
 */
static void VLAN_MGR_SetVlanPortEntryToDefault(UI32_T lport_ifindex)
{
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    UI32_T                              default_vlan_id;
    UI32_T                              default_vlan_ifindex;

    default_vlan_id = VLAN_OM_GetGlobalDefaultVlan();
    VLAN_VID_CONVERTTO_IFINDEX(default_vlan_id, default_vlan_ifindex);
    memset (&vlan_port_info, 0, sizeof(VLAN_OM_Vlan_Port_Info_T));
    vlan_port_info.lport_ifindex = lport_ifindex;
    vlan_port_info.port_trunk_mode = FALSE;
    vlan_port_info.port_item.dot1q_pvid_index = default_vlan_ifindex;
    vlan_port_info.port_item.admin_pvid = default_vlan_id;
    vlan_port_info.port_item.dot1q_port_acceptable_frame_types = SYS_DFLT_1Q_PORT_ACCEPTABLE_FRAME_TYPES;
    vlan_port_info.port_item.admin_acceptable_frame_types = SYS_DFLT_1Q_PORT_ACCEPTABLE_FRAME_TYPES;
    vlan_port_info.port_item.dot1q_port_ingress_filtering = SYS_DFLT_1Q_PORT_INGRESS_FILTERING;
    vlan_port_info.port_item.admin_ingress_filtering = SYS_DFLT_1Q_PORT_INGRESS_FILTERING;
    vlan_port_info.port_item.dot1q_port_gvrp_status = SYS_DFLT_1Q_PORT_GVRP_STATUS;
    vlan_port_info.port_item.dot1q_port_gvrp_failed_registrations = 0;
    vlan_port_info.port_item.static_joined_vlan_count = 1;
    vlan_port_info.port_item.untagged_joined_vlan_count = 1;
    memset(&vlan_port_info.port_item.dot1q_port_gvrp_last_pdu_origin, 0, SIZE_dot1qPortGvrpLastPduOrigin);
    vlan_port_info.garp_entry.dot1d_port_garp_join_time = SYS_DFLT_1D_PORT_GARP_JOIN_TIME;
    vlan_port_info.garp_entry.dot1d_port_garp_leave_time = SYS_DFLT_1D_PORT_GARP_LEAVE_TIME;
    vlan_port_info.garp_entry.dot1d_port_garp_leave_all_time = SYS_DFLT_1D_PORT_GARP_LEAVE_ALL_TIME;
    vlan_port_info.vlan_port_entry.vlan_port_mode =SYS_DFLT_VLAN_PORT_MODE;
#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
    vlan_port_info.vlan_port_entry.vlan_port_trunk_link_mode = VLAN_MGR_NOT_TRUNK_LINK;
#endif /* end #if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE) */

    VLAN_OM_SetVlanPortEntry(&vlan_port_info);
    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
    {
        BACKDOOR_MGR_Printf("VLAN_MGR_SetVlanPortEntryToDefault: VLAN_OM_SetVlanPortEntry: lport_ifindex=%lu\n", (unsigned long)lport_ifindex);
    }

    return;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetVlanPortDefaultAttributeToChip
 * ------------------------------------------------------------------------
 * PURPOSE  : Set VLAN port default attribute to chip
 * INPUT    : lport_ifindex
 * OUTPUT   : None
 * RETURN   : None
 * ------------------------------------------------------------------------
 */
static void VLAN_MGR_SetVlanPortDefaultAttributeToChip(UI32_T lport_ifindex)
{
    UI32_T                              default_vlan_id;

    default_vlan_id = VLAN_OM_GetGlobalDefaultVlan();
    /* Join the member set of default vlan
     */
    if (!is_provision_complete || VLAN_MGR_AddVlanMemberToChip(lport_ifindex, default_vlan_id, FALSE, TRUE))
    {
        /* Join the untagged set of default vlan
         */
        if (!is_provision_complete || VLAN_MGR_AddVlanMemberToChip(lport_ifindex, default_vlan_id, TRUE, TRUE))
        {
            if (SWCTRL_SetPortPVID(lport_ifindex, default_vlan_id,TRUE))
            {
                if (VAL_dot1qPortIngressFiltering_true == SYS_DFLT_1Q_PORT_INGRESS_FILTERING)
                {
                    if (!SWCTRL_EnableIngressFilter(lport_ifindex))
                    {
                        if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
                        {
                            BACKDOOR_MGR_Printf("VLAN_MGR_SetVlanPortDefaultAttributeToChip failed: SWCTRL_EnableIngressFilter(lport_ifindex=%lu)\n", (unsigned long)lport_ifindex);
                        }
                        return;
                    }
                }
                else
                {
                    if (!SWCTRL_DisableIngressFilter(lport_ifindex))
                    {
                        if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
                        {
                            BACKDOOR_MGR_Printf("VLAN_MGR_SetVlanPortDefaultAttributeToChip failed: SWCTRL_DisableIngressFilter(lport_ifindex=%lu)\n", (unsigned long)lport_ifindex);
                        }
                        return;
                    }
                }

                if (!SWCTRL_AdmitAllFrames(lport_ifindex))
                {
                    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
                    {
                        BACKDOOR_MGR_Printf("VLAN_MGR_SetVlanPortDefaultAttributeToChip failed: SWCTRL_AdmitAllFrames(lport_ifindex=%lu)\n", (unsigned long)lport_ifindex);
                    }
                    return;
                }
            }
            else
            {
                if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
                {
                    BACKDOOR_MGR_Printf("VLAN_MGR_SetVlanPortDefaultAttributeToChip failed: SWCTRL_SetPortPVID(lport_ifindex=%lu, PVID=%lu)\n", (unsigned long)lport_ifindex, (unsigned long)default_vlan_id);
                }
            } /* end of if (SWCTRL_SetPortPVID(lport_ifindex, default_vlan_id)) */
        }
        else
        {
            if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
            {
                BACKDOOR_MGR_Printf("VLAN_MGR_SetVlanPortDefaultAttributeToChip failed: VLAN_MGR_AddVlanMemberToChip(lport_ifindex=%lu, PVID=%lu, untagged)\n", (unsigned long)lport_ifindex, (unsigned long)default_vlan_id);
            }
        } /* end of if (VLAN_MGR_AddVlanMemberToChip(lport_ifindex, default_vlan_id, untagged)) */
    }
    else
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        {
            BACKDOOR_MGR_Printf("VLAN_MGR_SetVlanPortDefaultAttributeToChip failed: VLAN_MGR_AddVlanMemberToChip(lport_ifindex=%lu, PVID=%lu)\n", (unsigned long)lport_ifindex, (unsigned long)default_vlan_id);
        }
    } /* end of if (VLAN_MGR_AddVlanMemberToChip(lport_ifindex, default_vlan_id))*/

    return;
}


static BOOL_T VLAN_MGR_RemoveVlanMemberForDeletingVlan(UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T vlan_status)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    VLAN_OM_Vlan_Port_Info_T            port_info;
    UI32_T  member_type;
    UI32_T  vid, port_state;
    UI8_T   active_uport_count_per_unit[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    UI8_T   port_list [SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T   total_active_uport_count, i;

    /* BODY */

    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        BACKDOOR_MGR_Printf("VLAN_MGR_RemoveVlanMemberForDeletingVlan: vid_ifindex %d, lport_ifindex %d\n",
                                          (UI16_T) vid_ifindex,(UI16_T) lport_ifindex);

    vlan_info.dot1q_vlan_index =(UI16_T)vid_ifindex;
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_RemoveVlanMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        return FALSE;
    }

    port_info.lport_ifindex = lport_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_RemoveVlanMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        return FALSE;
    } /* end of if */

    /* port member added by one method shall not be removed by any other method
     * except the unauthorized port is authenticating.
     */
    member_type = VLAN_MGR_GetPortMemberType(&vlan_info, &port_info);
    if (    (member_type != vlan_status)
         && !(vlan_status == VLAN_TYPE_VLAN_STATUS_AUTO )
#if (SYS_CPNT_VLAN_STATICALLY_DELETE_DYNAMIC_MEMBER_ALLOWED == TRUE)
         && !((member_type == VLAN_TYPE_VLAN_STATUS_GVRP) && (vlan_status == VLAN_TYPE_VLAN_STATUS_STATIC))
#endif
       )
        return FALSE;

    VLAN_IFINDEX_CONVERTTO_VID(vid_ifindex, vid);

    /* Delete untagged member */
    if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
    {
        VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex);
        port_info.port_item.untagged_joined_vlan_count--;

        if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
        {
            if (is_provision_complete == TRUE)
            {
                if (!SWCTRL_DeletePortFromVlanUntaggedSet(lport_ifindex, vid,TRUE))
                {
                    VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING,
                                                 FUNCTION_RETURN_FAIL_INDEX,
                                                 "VLAN_MGR_DestroyVlanMembers");
                    return FALSE;
                } /* end of if */
            }
        } /* end of if */
    } /* end of if */

    if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex))
    {
        VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex);

        if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
        {
            if (!SWCTRL_DeletePortFromVlanMemberSet(lport_ifindex, vid,TRUE))
            {
                VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING,
                                             FUNCTION_RETURN_FAIL_INDEX,
                                             "VLAN_MGR_DestroyVlanMembers");
                return FALSE;
            }
        } /* end of if */
    } /* end of if */

    if (    (vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_dynamicGvrp)
         && (vlan_status == VAL_dot1qVlanStatus_permanent)
       )
        vlan_info.dot1q_vlan_status = VAL_dot1qVlanStatus_permanent;

    switch (vlan_status)
    {
        case VLAN_TYPE_VLAN_STATUS_STATIC:
            VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, lport_ifindex);
            VLAN_OM_DEL_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, lport_ifindex);
            port_info.port_item.static_joined_vlan_count--;
            break;

        case VLAN_TYPE_VLAN_STATUS_AUTO:
            break;

        case VLAN_TYPE_VLAN_STATUS_VOICE:
            port_info.vlan_port_entry.voice_vid = 0;
            break;

        case VLAN_TYPE_VLAN_STATUS_MVR:
            port_info.vlan_port_entry.mvr_vid >0? (port_info.vlan_port_entry.mvr_vid--):(port_info.vlan_port_entry.mvr_vid= 0);
            break;

        case VLAN_TYPE_VLAN_STATUS_GVRP:
        default:
            break;
    } /* end of switch */

    if (!VLAN_MGR_SetVlanRowStatus(L_RSTATUS_SET_OTHER_COLUMN, &vlan_info))
    {
        return FALSE;
    }

    VLAN_OM_SetVlanPortEntry(&port_info);

    if (vlan_info.if_entry.vlan_operation_status == VAL_ifOperStatus_up)
    {
        VLAN_IFINDEX_CONVERTTO_VID(vlan_info.dot1q_vlan_index,vid);
        if (!XSTP_OM_GetPortStateByVlan(vid, lport_ifindex, &port_state))
            return FALSE;
        if (port_state == VAL_dot1dStpPortState_forwarding)
        {
            if (SWCTRL_LportListToActiveUportList(  vid,
                                                    vlan_info.dot1q_vlan_current_egress_ports,
                                                    active_uport_count_per_unit, port_list
                                                 ) == TRUE
               )
            {
                total_active_uport_count    = 0;
                for (i = 0; i < SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
                {
                    total_active_uport_count    += active_uport_count_per_unit[i];
                }
                if (total_active_uport_count  == 0)
                {
                    VLAN_MGR_SET_IF_OPER_STATUS(vlan_info.if_entry, VAL_ifOperStatus_down);
                    VLAN_OM_SetVlanEntry(&vlan_info);
                    VLAN_MGR_SEND_IF_OPER_STATUS_CHANGED_EVENT(vid);
                }
            } /* end of if */
        } /* end of if */
    } /* end of if */

    return TRUE;
}

static BOOL_T VLAN_MGR_ProcessAuthPort(UI32_T lport_ifindex, UI32_T pvid, VLAN_OM_VLIST_T *tagged_vlist, VLAN_OM_VLIST_T *untagged_vlist, BOOL_T is_guest)
{
    VLAN_OM_VLIST_T                     *vlan;
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    UI32_T                              temp_vid = 0;
    UI32_T                              vid_ifindex;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    BOOL_T                              ret = TRUE;

    /* 1. Add port to authorized VLAN's current list and update chip, for the reason
     * that if join tagged VLAN first then it is possible fail because PVID is on
     * this VLAN, so the sequence is as follows:
     *      a. join untagged VLAN(for changing PVID).
     *      b. change PVID(for joining tagged VLAN).
     *      c. join tagged VLAN.
     *
     * 2. Remove port from non-authorized VLAN.
     *      a. the first time authorization should remove port that belongs to
     *         the VLAN which is not authorized from chip
     *      b. after the first time should remove port that belongs to the VLAN
     *         which is previous authorized from chip
     *
     * 3. Change to dynamic VLAN mode
     */

    /* 1a. join untagged VLAN(for changing PVID).
     */
    vlan = untagged_vlist;
    while (vlan != NULL && ret)
    {
        ret = VLAN_MGR_AddUntagPortMember( vlan->vlan_id, lport_ifindex, VLAN_TYPE_VLAN_STATUS_AUTO);
        vlan = vlan->next;
    }

    /* 1b. change PVID(for joining tagged VLAN).
     */
    if (ret)
    {
        if (VLAN_TYPE_DOT1Q_RESERVED_VLAN_ID == pvid)
        {
            /* When pvid equals VLAN_TYPE_DOT1Q_RESERVED_VLAN_ID,
             * 1. when tagged only, just admit tagged frame only
             * 2. when no tagged, trun on ingress filtering to drop all traffic
             */
            if (tagged_vlist)
            {
                ret = VLAN_MGR_SetDot1qPortAcceptableFrameTypes(lport_ifindex, VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanTagged);
            }
            else
            {
                ret = VLAN_MGR_SetDot1qIngressFilter(lport_ifindex, VAL_dot1qPortIngressFiltering_true);
            }
        }
        else
        {
            ret = VLAN_MGR_SetAuthorizedPvid(lport_ifindex, pvid);

            if (is_guest == TRUE)
            {
                ret = VLAN_MGR_SetDot1qIngressFilter(lport_ifindex, VAL_dot1qPortIngressFiltering_true);
            }
        }
    }
    else
    {
        vlan = untagged_vlist;
        while (vlan != NULL && ret)
        {
            VLAN_MGR_DeleteEgressPortMember( vlan->vlan_id, lport_ifindex, VLAN_TYPE_VLAN_STATUS_AUTO);
            vlan = vlan->next;
        }
    }

    /* 1c. join tagged VLAN.
     */
    if (ret)
    {
        vlan = tagged_vlist;
        while (vlan != NULL && ret)
        {
            VLAN_VID_CONVERTTO_IFINDEX(vlan->vlan_id, vid_ifindex);
            if (VLAN_OM_IsVlanUntagPortListMember(vid_ifindex, lport_ifindex))
            {
                ret = VLAN_MGR_DeleteUntagPortMember(vlan->vlan_id, lport_ifindex, VLAN_TYPE_VLAN_STATUS_AUTO);
            }
            else if (!VLAN_OM_IsPortVlanMember(vid_ifindex, lport_ifindex))
            {
                ret = VLAN_MGR_AddEgressPortMember( vlan->vlan_id, lport_ifindex, VLAN_TYPE_VLAN_STATUS_AUTO);
            }
            vlan = vlan->next;
        }
    }

    if (ret)
    {
        if (VLAN_MGR_GetPortEntry(lport_ifindex, &vlan_port_info))
        {
            if (vlan_port_info.port_item.auto_vlan_mode)
            {
                /* 2a. Remove port that belongs to the VLAN which is previous authorized from chip
                 */
                while (ret && VLAN_OM_GetNextDot1qVlanCurrentEntry(0, &temp_vid, &vlan_info))
                {
                    if (    (   VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex)
                             && !VLAN_MGR_IsVlanListMember(untagged_vlist, temp_vid)
                             && !VLAN_MGR_IsVlanListMember(tagged_vlist, temp_vid)
                            )
                        ||  (   !VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex)
                             && VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex)
                             && !VLAN_MGR_IsVlanListMember(tagged_vlist, temp_vid)
                            )
                       )
                    {
                        ret = VLAN_MGR_DeleteEgressPortMember( temp_vid, lport_ifindex, VLAN_TYPE_VLAN_STATUS_AUTO);
                    }
                }

                /* For rollback the setting of previous authorization
                 */
                if (VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanTagged == vlan_port_info.port_item.dot1q_port_acceptable_frame_types)
                {
                    if (    !(  VLAN_TYPE_DOT1Q_RESERVED_VLAN_ID == pvid
                              &&tagged_vlist
                             )
                       )
                    {
                        ret = VLAN_MGR_SetDot1qPortAcceptableFrameTypes(lport_ifindex, vlan_port_info.port_item.admin_acceptable_frame_types);
                    }
                }
                if (VAL_dot1qPortIngressFiltering_true == vlan_port_info.port_item.dot1q_port_ingress_filtering)
                {
                    if (    NULL != tagged_vlist
                        ||  NULL != untagged_vlist
                        ||  VLAN_TYPE_DOT1Q_RESERVED_VLAN_ID != pvid
                       )
                    {
                        ret = VLAN_MGR_SetDot1qIngressFilter(lport_ifindex, vlan_port_info.port_item.admin_ingress_filtering);
                    }
                }
            } /* end of if (vlan_port_info.port_item.auto_vlan_mode) */
            else
            {
                /* 2b. Delete port from non-authorized VLAN's current list and update chip
                 */
                while (ret && VLAN_OM_GetNextDot1qVlanCurrentEntry(0, &temp_vid, &vlan_info))
                {
                    if (    !VLAN_MGR_IsVlanListMember(untagged_vlist, temp_vid)
                        &&  !VLAN_MGR_IsVlanListMember(tagged_vlist, temp_vid)
                        &&  VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, lport_ifindex)
                       )
                    {
                        ret = VLAN_MGR_DeleteEgressPortMember( temp_vid, lport_ifindex, VLAN_TYPE_VLAN_STATUS_AUTO);
                    }
                    else if(    VLAN_MGR_IsVlanListMember(tagged_vlist, temp_vid)
                            &&  VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, lport_ifindex)
                           )
                    {
                        ret = VLAN_MGR_DeleteUntagPortMember( temp_vid, lport_ifindex, VLAN_TYPE_VLAN_STATUS_AUTO);
                    }
                } /* end of while (VLAN_OM_GetNextDot1qVlanCurrentEntry())*/

                /* 3. Change to dynamic VLAN mode
                 */
                if (ret)
                {
                    ret = VLAN_MGR_SetPortAutoVlanMode(lport_ifindex, TRUE);
                }
            } /* end of else of if (vlan_port_info.port_item.auto_vlan_mode) */
        } /* end of if (VLAN_MGR_GetPortEntry()) */
    }

    return ret;
}

static BOOL_T VLAN_MGR_ProcessUnauthPort(UI32_T lport_ifindex)
{
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    UI32_T                              temp_vid;
    BOOL_T                              ret = FALSE;

    if (VLAN_MGR_GetPortEntry(lport_ifindex, &vlan_port_info))
    {
        if (!vlan_port_info.port_item.auto_vlan_mode)
        {
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }

    /* Change dynamic VLAN mode & acceptable frame type & ingress filtering
     */
    ret = VLAN_MGR_SetPortAutoVlanMode(lport_ifindex, FALSE);
    if (ret)
    {
        if (vlan_port_info.port_item.admin_acceptable_frame_types != vlan_port_info.port_item.dot1q_port_acceptable_frame_types)
        {
            ret = VLAN_MGR_SetDot1qPortAcceptableFrameTypes(lport_ifindex, vlan_port_info.port_item.admin_acceptable_frame_types);
        }
    }
    if (ret)
    {
        if (vlan_port_info.port_item.admin_ingress_filtering != vlan_port_info.port_item.dot1q_port_ingress_filtering)
        {
            ret = VLAN_MGR_SetDot1qIngressFilter(lport_ifindex, vlan_port_info.port_item.admin_ingress_filtering);
        }
    }

    /* Add port to all VLAN's current list if the port is member of static list but not member of current list
     */
    if (ret)
    {
        temp_vid = 0;
        while (ret && VLAN_OM_GetNextDot1qVlanCurrentEntry(0, &temp_vid, &vlan_info))
        {
            if (    (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
                &&  (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, lport_ifindex))
               )
            {
                ret = VLAN_MGR_AddUntagPortMember( temp_vid, lport_ifindex, VLAN_TYPE_VLAN_STATUS_AUTO);
            }
            else if(    (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex))
                    &&  (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, lport_ifindex))
                   )
            {
                ret = VLAN_MGR_AddEgressPortMember( temp_vid, lport_ifindex, VLAN_TYPE_VLAN_STATUS_AUTO);
            }
        }
    }

    /* Restore PVID
     */
    if (ret)
    {
        ret = VLAN_MGR_SetDot1qPvid(lport_ifindex, vlan_port_info.port_item.admin_pvid);
    }

    /* Remove port from all VLAN's current list if the port is not member of static list
     */
    if (ret)
    {
        temp_vid = 0;
        while (ret && VLAN_OM_GetNextDot1qVlanCurrentEntry(0, &temp_vid, &vlan_info))
        {
            if (    (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
                &&  (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_untagged_ports, lport_ifindex))
               )
            {
                ret = VLAN_MGR_DeleteUntagPortMember( temp_vid, lport_ifindex, VLAN_TYPE_VLAN_STATUS_AUTO);
            }
            if (    (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex))
                &&  (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, lport_ifindex))
               )
            {
                ret = VLAN_MGR_DeleteEgressPortMember( temp_vid, lport_ifindex, VLAN_TYPE_VLAN_STATUS_AUTO);
            }
        }
    }

#if (SYS_CPNT_VLAN_DYNAMICALLY_CREATE_AUTO_VLAN == TRUE)
    if (ret)
    {
        /* Delete dynamically created VLAN
         */
        while (ret && VLAN_OM_GetNextDot1qVlanCurrentEntry(0, &temp_vid, &vlan_info))
        {
            if (vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_other)
            {
                ret = VLAN_MGR_DeleteVlan(temp_vid, VAL_dot1qVlanStatus_other);
            }
        }
    }
#endif

    return ret;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetAuthorizedPvid
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the pvid of the specific port can be
 *            set successfully. Otherwise, return false.
 * INPUT    : lport_ifindex   -- the port number
 *            pvid     -- the pvid value
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_SetAuthorizedPvid(UI32_T lport_ifindex, UI32_T pvid)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    UI32_T                              pvid_ifindex;
    SWCTRL_Lport_Type_T                 port_type;
    UI32_T                              unit, port, trunk_id, old_pvid;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    /* Error if pre-condition to set port info can not be satisfied.
     */
    if (!VLAN_MGR_PreconditionForSetVlanPortInfo(lport_ifindex, PVID_FIELD, pvid))
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if ((port_type = SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit, &port, &trunk_id)) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qPvid_Fun_No,
                                    EH_TYPE_MSG_IS_INVALID,
                                    SYSLOG_LEVEL_INFO,
                                    "Port"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    /* lport_ifindex is the key to search for vlan entry in the database.
     */
    vlan_port_info.lport_ifindex = lport_ifindex;

    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qPvid_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);

    } /* end of if */

    VLAN_IFINDEX_CONVERTTO_VID(vlan_port_info.port_item.dot1q_pvid_index, old_pvid);
    if (old_pvid == pvid)
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
    } /* end of if */

    VLAN_VID_CONVERTTO_IFINDEX(pvid, pvid_ifindex);
    vlan_info.dot1q_vlan_index = (UI16_T)pvid_ifindex;
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qPvid_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
    {
        if (!SWCTRL_SetPortPVID(lport_ifindex, pvid,TRUE))
        {
            VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_SetDot1qPvid");
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        } /* end of if */
    }

    vlan_port_info.port_item.dot1q_pvid_index = pvid_ifindex;
    if (VLAN_OM_SetVlanPortEntry(&vlan_port_info))
    {
        if (port_type == SWCTRL_LPORT_TRUNK_PORT)
            VLAN_MGR_SetTrunkMemberPortInfo(lport_ifindex,PVID_FIELD, pvid_ifindex);

        if (vlan_info.dot1q_vlan_static_row_status != VAL_dot1qVlanStaticRowStatus_active)
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);

        VLAN_MGR_Notify_PvidChange(lport_ifindex, old_pvid, pvid);
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);

    } /* end of if */

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
} /* End of VLAN_MGR_SetAuthorizedPvid() */

static VLAN_OM_VLIST_T *VLAN_MGR_AddVlanList(VLAN_OM_VLIST_T *vlan_list, UI32_T vid)
{
    VLAN_OM_VLIST_T    *tmp_list = vlan_list;
    VLAN_OM_VLIST_T    *prev = NULL;
    VLAN_OM_VLIST_T    *new_vlist;
    BOOL_T             found = FALSE;

    while (tmp_list != NULL)
    {
        if (tmp_list->vlan_id == vid)
        {
            found = TRUE;
            break;
        }
        prev = tmp_list;
        tmp_list = tmp_list->next;
    } /* End of while */

    if (!found)
    {
        new_vlist = (VLAN_OM_VLIST_T*)L_MM_Malloc(sizeof(VLAN_OM_VLIST_T), L_MM_USER_ID2(SYS_MODULE_VLAN, VLAN_TYPE_TRACE_ID_VLAN_MGR_ADDVLANLIST));
        if (new_vlist)
        {
            memset(new_vlist, 0, sizeof(VLAN_OM_VLIST_T));
            new_vlist->vlan_id = vid;
            new_vlist->next = NULL;
            if (prev == NULL)
            {
                vlan_list = new_vlist;
            }
            else
            {
                prev->next = new_vlist;
            }
        } /* End of if (new_vlist) */
    } /* End of if (not found) */
    return vlan_list;
}

static BOOL_T VLAN_MGR_IsVlanListMember(VLAN_OM_VLIST_T *vlan_list, UI32_T vid)
{
    VLAN_OM_VLIST_T    *tmp_list = vlan_list;
    BOOL_T             found = FALSE;

    while (tmp_list != NULL)
    {
        if (tmp_list->vlan_id == vid)
        {
            found = TRUE;
            break;
        }
        tmp_list = tmp_list->next;
    } /* End of while */

    return found;
}

static void VLAN_MGR_FreeVlanList(VLAN_OM_VLIST_T *vlan_list)
{
    VLAN_OM_VLIST_T    *free_vlan;

    while (vlan_list != NULL)
    {
        free_vlan = vlan_list;
        vlan_list = vlan_list->next;
        L_MM_Free((void*)free_vlan);
    }
    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetDot1qPortAcceptableFrameTypes_
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if port acceptable frame type is set
 *            successfully.  Otherwise, return false.
 * INPUT    : lport_ifindex       -- the port number
 *            acceptable_frame_types - VAL_dot1qPortAcceptableFrameTypes_admitAll \
 *                                     VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanTagged
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : The default value is VAL_dot1qPortAcceptableFrameTypes_admitAll.
 *-----------------------------------------------------------------------------
 */
static BOOL_T VLAN_MGR_SetDot1qPortAcceptableFrameTypes_(UI32_T lport_ifindex, UI32_T acceptable_frame_types)
{
    SWCTRL_Lport_Type_T                 port_type;
    UI32_T                              unit, port, trunk_id;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;

    /* Error if pre-condition to set port info can not be satisfied.
     */
    if (!VLAN_MGR_PreconditionForSetVlanPortInfo(lport_ifindex,
                                                 ACCEPTABLE_FRAME_TYPE_FIELD,
                                                 acceptable_frame_types))
        return FALSE;

    if ((port_type = SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit, &port, &trunk_id)) == SWCTRL_LPORT_UNKNOWN_PORT)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qPortAcceptableFrameTypes_Fun_No,
                                    EH_TYPE_MSG_IS_INVALID,
                                    SYSLOG_LEVEL_INFO,
                                    "Port"
                                );
        return FALSE;
    }

    vlan_port_info.lport_ifindex = lport_ifindex;

    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_SetDot1qPortAcceptableFrameTypes_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        return FALSE;
    }

    if (!vlan_port_info.port_item.auto_vlan_mode || is_authenticating)
    {
        if (acceptable_frame_types == VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanTagged)
        {
            if (!SWCTRL_AdmitVLANTaggedFramesOnly(lport_ifindex))
            {
                VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_SetDot1qPortAcceptableFrameTypes_");
                return FALSE;
            } /* end of if */
        }
        else if(acceptable_frame_types == VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanUnTagged)
        {
            if (!SWCTRL_AdmitVLANUntaggedFramesOnly(lport_ifindex))
            {
                VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_SetDot1qPortAcceptableFrameTypes_");
                return FALSE;
            } /* end of if */
        }
        else
        {
            if (!SWCTRL_AdmitAllFrames(lport_ifindex))
            {
                VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_SetDot1qPortAcceptableFrameTypes_");
                return FALSE;
            } /* end of if */
        }

        vlan_port_info.port_item.dot1q_port_acceptable_frame_types = acceptable_frame_types;
        if ((port_type = SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit, &port, &trunk_id)) == SWCTRL_LPORT_TRUNK_PORT)
        {
            VLAN_MGR_SetTrunkMemberPortInfo(lport_ifindex,ACCEPTABLE_FRAME_TYPE_FIELD, acceptable_frame_types);
        }
    }

    if (!is_authenticating)
    {
        if ((port_type = SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit, &port, &trunk_id)) == SWCTRL_LPORT_TRUNK_PORT)
        {
            VLAN_MGR_SetTrunkMemberPortInfo(lport_ifindex, ADMIN_ACCEPTABLE_FRAME_TYPE_FIELD, acceptable_frame_types);
        }
        vlan_port_info.port_item.admin_acceptable_frame_types = acceptable_frame_types;
    }

    if (!VLAN_OM_SetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_VLAN,
                                 VLAN_MGR_SetDot1qVlanStaticName_Fun_No,
                                 EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG),
                                 "lport (1-SYS_ADPT_TOTAL_NBR_OF_LPORT"
                                );
        return FALSE;
    }

    return TRUE;
} /* End of VLAN_MGR_SetDot1qPortAcceptableFrameTypes_() */

/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_SetMgmtIpStateForVlan
 * ----------------------------------------------------------------------------
 * PURPOSE  : Set management state and ip interface state for a vlan.
 * INPUT    : vid        - the identifier of vlan
 *            mgmt_state - whether is management vlan
 *            ip_state   - whether has ip interface and which version
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : None
 * ----------------------------------------------------------------------------
 */
static BOOL_T VLAN_MGR_SetMgmtIpStateForVlan(UI32_T vid, BOOL_T mgmt_state, UI8_T ip_state)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T  vlan_info;
    UI32_T                           vid_ifindex;
    char                             arg_buf[15];

    if (vid < 1 || vid > SYS_ADPT_MAX_VLAN_ID)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_VLAN,
                                 VLAN_MGR_SetMgmtIpStateForVlan_Fun_No,
                                 EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                 SYSLOG_LEVEL_INFO,
                                 "VLAN ID"
                                );
        return FALSE;
    }

    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        BACKDOOR_MGR_Printf("VLAN_MGR_SetVlanIpMgmtState: %lu\n", (unsigned long)vid);

    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_VLAN,
                                 VLAN_MGR_SetMgmtIpStateForVlan_Fun_No,
                                 EH_TYPE_MSG_NOT_EXIST,
                                 SYSLOG_LEVEL_INFO,
                                 "VLAN entry"
                                );
        return FALSE;
    }

    /* Dynamic vlan cannot be configured as management vlan */
    if (vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_dynamicGvrp)
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("FAIL: VLAN_MGR_SetVlanIpMgmtState (dynamic vlan): vid%lu\n", (unsigned long)vid);

        EH_MGR_Handle_Exception1(SYS_MODULE_VLAN,
                                 VLAN_MGR_SetMgmtIpStateForVlan_Fun_No,
                                 EH_TYPE_MSG_FAILED_TO_SET,
                                 SYSLOG_LEVEL_INFO,
                                 "management vlan because it is dynamic vlan"
                                );
        return FALSE;
    }

    if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_notReady)
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("FAIL: VLAN_MGR_SetVlanIpMgmtState (dynamic vlan): vid%lu\n", (unsigned long)vid);

        EH_MGR_Handle_Exception1(SYS_MODULE_VLAN,
                                 VLAN_MGR_SetMgmtIpStateForVlan_Fun_No,
                                 EH_TYPE_MSG_FAILED_TO_SET,
                                 SYSLOG_LEVEL_INFO,
                                 "management vlan because its row status is not ready"
                                );
        return FALSE;
    }

    if (ip_state != VLAN_MGR_IP_STATE_UNCHANGED)
        vlan_info.vlan_ip_state = ip_state;

    snprintf(arg_buf, 15, "VLANs (%d)", SYS_ADPT_MAX_NBR_OF_VLAN);

    if (!VLAN_OM_SetVlanEntry(&vlan_info))
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            BACKDOOR_MGR_Printf("FAIL: VLAN_MGR_SetVlanIpMgmtState: vid%lu\n", (unsigned long)vid);

        EH_MGR_Handle_Exception1(SYS_MODULE_VLAN,
                                 VLAN_MGR_SetMgmtIpStateForVlan_Fun_No,
                                 EH_TYPE_MSG_MAXIMUM_EXCEEDED,
                                 SYSLOG_LEVEL_INFO,
                                 arg_buf
                                );
        return FALSE;
    }

    if (mgmt_state == TRUE)
        VLAN_OM_SetManagementVlanId(vid);

    return TRUE;
} /* End of VLAN_MGR_SetMgmtIpStateForVlan() */

#if (SYS_DFLT_VLAN_CHECK_AT_LEAST_ONE_MEMBER_IN_MANAGEMEMT_VLAN == TRUE)
static UI32_T VLAN_MGR_CountVlanMember(UI8_T *portlist)
{
    UI8_T byte, bit;
    UI32_T counter = 0;

    for (byte = 0; byte < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; byte++)
    {
        for (bit = 0; bit < 8; bit++)
        {
            if (portlist[byte] & (1 << bit))
            {
                counter++;
            }
        }
    }

    return counter;
} /* End of VLAN_MGR_CountVlanMember() */
#endif /* end of #if (SYS_DFLT_VLAN_CHECK_AT_LEAST_ONE_MEMBER_IN_MANAGEMEMT_VLAN == TRUE) */

/*added by Jinhua Wei ,to remove warning ,becaued the following functions defined but never used*/
#if 0
/*-----------------------------------------------------------------------------
 * FUNCTION NAME : VLAN_MGR_CreateVlanDev
 *-----------------------------------------------------------------------------
 * PURPOSE :
 *     Create the vlan net device for the specified vid.
 *
 * INPUT:
 *     vid      -- The vid of the vlan net device which is going to be created.
 *     mac_addr -- The mac addr of the vlan net device which is going to be created.
 *
 * OUTPUT:
 *     None.
 *
 * RETURN:
 *     VLAN_TYPE_RETVAL_OK                   -- Success
 *     VLAN_TYPE_RETVAL_INVALID_ARG          -- Invalid arguments
 *     VLAN_TYPE_RETVAL_CREATE_VLAN_DEV_FAIL -- Fail to create vlan net device
 *
 * NOTES:
 *     None.
 *-----------------------------------------------------------------------------
 */
static UI32_T VLAN_MGR_CreateVlanDev(UI32_T vid, UI8_T mac_addr[SYS_ADPT_MAC_ADDR_LEN])
{
    if (mac_addr == NULL)
        return VLAN_TYPE_RETVAL_INVALID_ARG;

    return SYSFUN_Syscall(SYSFUN_SYSCALL_VLAN_MGR,
                          VLAN_TYPE_SYSCALL_CMD_CREATE_VLAN_DEV,
                          (UI32_T)vid,
                          (UI32_T)mac_addr,
                          0, 0);
}

/*--------------------------------------------------------------------------
 *  FUNCTION NAME : VLAN_MGR_DestroyVlanDev
 *--------------------------------------------------------------------------
 *  PURPOSE:
 *   Destroy the vlan net device for the specified vid.
 *
 *  INPUT:
 *   vid        --  The vid of the vlan device which is going to be created.
 *
 *  OUTPUT:
 *   None.
 *
 *  RETURN:
 *   VLAN_TYPE_RETVAL_OK                    --  Success
 *   VLAN_TYPE_RETVAL_UNKNOWN_ERROR         --  Unknown error
 *
 *  NOTES:
 *   None.
 *--------------------------------------------------------------------------
 */
static UI32_T VLAN_MGR_DestroyVlanDev(UI32_T vid)
{
    return SYSFUN_Syscall(SYSFUN_SYSCALL_VLAN_MGR,
                          VLAN_TYPE_SYSCALL_CMD_DESTROY_VLAN_DEV,
                          (UI32_T)vid,
                          0, 0, 0);
}
#endif

#if 0 /* DanXie, Tuesday, January 06, 2009 1:56:43 */
/*--------------------------------------------------------------------------
 *  FUNCTION NAME : VLAN_MGR_SetVlanNetDevLinkStatus
 *--------------------------------------------------------------------------
 *  PURPOSE:
 *   Set the link status for the specified vlan net device.
 *
 *  INPUT:
 *   vid          --  The vid of the vlan net device
 *   link_status  --  The link status to be set on the specified vlan net device
 *                    TRUE: link up, FALSE: link down
 *
 *  OUTPUT:
 *   None.
 *
 *  RETURN:
 *   VLAN_TYPE_RETVAL_OK              --  Success
 *   VLAN_TYPE_RETVAL_UNKNOWN_ERROR   --  Unknown error
 *
 *  NOTES:
 *   None.
 *--------------------------------------------------------------------------
 */
static UI32_T VLAN_MGR_SetVlanNetDevLinkStatus(UI32_T vid, BOOL_T link_status)
{
    struct ifreq ifr;
    int fd;

    NETCFG_NETDEVICE_GET_DEV_NAME_FROM_VID(ifr.ifr_name, (int)vid);
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
            BACKDOOR_MGR_Printf("socket fail\r\n");
        return VLAN_TYPE_RETVAL_UNKNOWN_ERROR;
    }

    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0)
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
            BACKDOOR_MGR_Printf("ioctl(SIOCGIFFLAGS) fail\r\n");
        close(fd);
        return VLAN_TYPE_RETVAL_UNKNOWN_ERROR;
    }

    if (link_status == TRUE)
        ifr.ifr_flags |= IFF_UP;
    else
        ifr.ifr_flags &= ~IFF_UP;

    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0)
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
            BACKDOOR_MGR_Printf("ioctl(SIOCSIFFLAGS) fail\r\n");
        close(fd);
        return VLAN_TYPE_RETVAL_UNKNOWN_ERROR;
    }

    close(fd);
    return VLAN_TYPE_RETVAL_OK;
}
#endif /* #if 0 */

/*--------------------------------------------------------------------------
 *  FUNCTION NAME : VLAN_MGR_GetVLANMemberByLport
 *--------------------------------------------------------------------------
 *  PURPOSE:
 *   Get the vlan list which port in
 *
 *  INPUT:
 *    lport_ifindex          --  The port index
 *
 *  OUTPUT:number    port number
 *                         vlan_list      vlan bit map
 *   None.
 *
 *  RETURN:
 *   TRUE     --  Success
 *   FALSE   --  Unknown error
 *
 *  NOTES:
 *   None.
 *--------------------------------------------------------------------------
 */

BOOL_T VLAN_MGR_GetVLANMemberByLport(UI32_T lport_ifindex,UI32_T *number,UI8_T *vlan_list)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    UI32_T                              vid;

   if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

   if (!SWCTRL_LogicalPortExisting(lport_ifindex))/*port not exist*/
       VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

   if(number == NULL || vlan_list == NULL)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);

   *number = 0;

   vlan_info.dot1q_vlan_index = 0;
   while(VLAN_OM_GetNextVlanEntry(&vlan_info))
   {

       if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex))
       {
           VLAN_IFINDEX_CONVERTTO_VID(vlan_info.dot1q_vlan_index,vid);
           VLAN_OM_ADD_MEMBER(vlan_list, vid);
           (*number)++;
       }
   } /* end of while */

  VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);

}
/*--------------------------------------------------------------------------
 *  FUNCTION NAME : VLAN_MGR_GetPortlistByVid
 *--------------------------------------------------------------------------
 *  PURPOSE:
 *   Get the port list of a vlan
 *
 *  INPUT:
 *    vid          --  The vid of the vlan net device
 *
 *  OUTPUT:number    port number
 *                         portlist      port bit map
 *   None.
 *
 *  RETURN:
 *   TRUE     --  Success
 *   FALSE   --  Unknown error
 *
 *  NOTES:
 *   None.
 *--------------------------------------------------------------------------
 */

BOOL_T VLAN_MGR_GetPortlistByVid(UI32_T vid,UI32_T * number,UI8_T *portlist)
{
    UI32_T      vid_ifindex, port_num;
    VLAN_OM_Dot1qVlanCurrentEntry_T         vlan_info;


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (vid < 1 || vid > SYS_ADPT_MAX_VLAN_ID )
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    if(number == NULL || portlist == NULL)
        {VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);}

    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;


    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    *number = 0;
    port_num    = 0;
    while (SWCTRL_GetNextLogicalPort(&port_num) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, port_num))
        {
            (* number)++;
        }
    }
    memcpy(portlist,vlan_info.dot1q_vlan_current_egress_ports,sizeof(vlan_info.dot1q_vlan_current_egress_ports));

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);

}

#if (SYS_CPNT_REFINE_ISC_MSG == TRUE)

static BOOL_T VLAN_MGR_RefineInitDefaultVlanEntry(UI8_T *port_list)
{
    UI8_T mask_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

   if(!port_list) return FALSE;

   printf("VLAN_MGR_RefineInitDefaultVlanEntry \n");
    memset(mask_port_list, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    SWCTRL_LportListToUportList(port_list, mask_port_list);

    if (is_provision_complete == TRUE)
    {
        if(!SWCTRL_AddPortToVlanMemberSet_PortList(mask_port_list, VLAN_MGR_DOT1Q_DEFAULT_PVID))
        {
            VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING,FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_InitDefaultVlanEntry");
               // return;
        } /* end of if */
    }

    if (is_provision_complete == TRUE)
    {
        if(!SWCTRL_AddPortToVlanUntaggedSet_PortList(mask_port_list, VLAN_MGR_DOT1Q_DEFAULT_PVID))
        {
            VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_InitDefaultVlanEntry");
                //return;
        } /* end of if */
    }

    if (!SWCTRL_SetPortPVID_PortList(mask_port_list, VLAN_MGR_DOT1Q_DEFAULT_PVID))
    {
        printf("SWCTRL_SetPortPVID_PortList error\n");
        VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_InitDefaultVlanEntry");
            //return;
    } /* end of if */

    if (SYS_DFLT_1Q_PORT_INGRESS_FILTERING == VAL_dot1qPortIngressFiltering_true)
    {
        if (!SWCTRL_EnableIngressFilter_PortList(mask_port_list))
        {
            VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_InitDefaultVlanEntry");
                //return;
        } /* end of if */
    }
    else
    {
        if (!SWCTRL_DisableIngressFilter_PortList(mask_port_list))
        {
            VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_InitDefaultVlanEntry");
                //return;
        } /* end of if */
    } /* end of if */

    if (!SWCTRL_AdmitAllFrames_PortList(mask_port_list))
    {
        VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_InitDefaultVlanEntry");
            //return;
    } /* end of if */
}
#endif

static UI32_T VLAN_MGR_GetPortMemberType(VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_info_p,
                                         VLAN_OM_Vlan_Port_Info_T *port_info_p)
{
    if (VLAN_OM_IS_MEMBER(vlan_info_p->dot1q_vlan_current_egress_ports, port_info_p->lport_ifindex))
    {
        if (port_info_p->port_item.auto_vlan_mode)
        {
            return VLAN_TYPE_VLAN_STATUS_AUTO;
        }
        else if (VLAN_OM_IS_MEMBER(vlan_info_p->dot1q_vlan_static_egress_ports, port_info_p->lport_ifindex))
        {
            return VLAN_TYPE_VLAN_STATUS_STATIC;
        }
        else if (port_info_p->vlan_port_entry.voice_vid > 0)
        {
            return VLAN_TYPE_VLAN_STATUS_VOICE;
        }
        else if (port_info_p->vlan_port_entry.mvr_vid > 0)
        {
            return VLAN_TYPE_VLAN_STATUS_MVR;
        }
        else
        {
            return VLAN_TYPE_VLAN_STATUS_GVRP;
        }
    }
    else
    {
        return VLAN_TYPE_VLAN_STATUS_NONE;
    }
} /* end of VLAN_MGR_GetPortMemberType() */

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
 * NOTES    : The API is only for RSPAN MGR.
 *------------------------------------------------------------------------------
 */
BOOL_T VLAN_MGR_CreateRspanVlan(UI32_T vid, UI32_T vlan_status)
{
    UI32_T      vid_ifindex, vlan_num;
    VLAN_OM_Dot1qVlanCurrentEntry_T         vlan_info;
    char                                    arg_buf[15];

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        printf("VLAN_MGR_CreateVlan_1: vid %d, status %d\n",(UI16_T)vid, (UI16_T)vlan_status);

    /* Default vlan and vid beyond maximum vlan id range
       can not be created.
     */
    if (vid < 1 || vid > SYS_ADPT_MAX_VLAN_ID)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_CreateVlan_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    SYSLOG_LEVEL_INFO, "VLAN ID"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    snprintf(arg_buf, 15, "VLANs (%d)", SYS_ADPT_MAX_NBR_OF_VLAN);
    if ((vlan_num = VLAN_OM_GetDot1qNumVlans()) >= SYS_ADPT_MAX_NBR_OF_VLAN)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_CreateVlan_Fun_No,
                                    EH_TYPE_MSG_MAXIMUM_EXCEEDED,
                                    SYSLOG_LEVEL_INFO, "arg_buf"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
    memset(&vlan_info, 0, sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    if (VLAN_OM_GetVlanEntry(&vlan_info)) /* If this vlan is already exists. */
    {
        /* Need to check if this vlan id is for normal vlan or rspan vlan.*/
        if ( vlan_info.rspan_status != VAL_vlanStaticExtRspanStatus_rspanVlan ) /* It's a normal vlan. */
        {
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        }

        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE); /* It's a span vlan. */
    }
    else
    {
        VLAN_MGR_DetailForCreateVlan(vid, vlan_status, &vlan_info); /* Original VLAN for SNMP */

        vlan_info.rspan_status = VAL_vlanStaticExtRspanStatus_rspanVlan ;

        /* Clear the vlan port state table */
        vlan_operstatus_changed[vid] = FALSE;

        SYS_TIME_GetSystemUpTimeByTick(VLAN_OM_GetIfStackLastUpdateTimePtr());

        /* Pass vlan_info into FSM to check and update dot1q_vlan_static_row_status field.
           If transition of dot1q_vlan_static_row_status is correct, then update vlan_info
           into database.  Otherwise, return false.
         */

        switch (L_RSTATUS_Fsm(VAL_dot1qVlanStaticRowStatus_createAndWait,
                              &vlan_info.dot1q_vlan_static_row_status,
                              VLAN_MGR_SemanticCheck,
                              (void*)&vlan_info))
        {
            /* Update Database ONLY
             */
            case L_RSTATUS_NOTEXIST_2_NOTREADY:
                if (!VLAN_OM_SetVlanEntry(&vlan_info))
                    break;

                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);
            /* Update Database and Chip
             */
            case L_RSTATUS_NOTEXIST_2_ACTIVE:
                if (!SWCTRL_CreateVlan(vid))
                {
                    VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING,FUNCTION_RETURN_FAIL_INDEX, "VLAN_MGR_CreateVlan_1");
                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
                } /* end of if */

                if (VLAN_OM_SetVlanEntry(&vlan_info))
                {
                    VLAN_MGR_Notify_VlanCreate(vlan_info.dot1q_vlan_index, vlan_info.dot1q_vlan_status);
                    /*  notify <stp, netcfg, gvrp> */
                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);
                } /* end of if */
                break;
            default:
                VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_WARNING, SWITCH_TO_DEFAULT_MESSAGE_INDEX, "VLAN_MGR_CreateVlan_1");
                break;
        } /* end of switch */
    }

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
}

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
BOOL_T VLAN_MGR_AddRspanUntagPortMember(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    UI32_T                              vid_ifindex;
    UI32_T                              pre_member_type, post_member_type;
    BOOL_T                              member_tag_changed;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

#if(SYS_CPNT_CLUSTER == TRUE)
    if(vid == SYS_DFLT_CLUSTER_DEFAULT_VLAN)
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
#endif

    /* Returns error if pre-condition to add vlan member can not be satisfy
     */
    if (!VLAN_MGR_PreconditionForSetVlanInfo(vid, lport_ifindex))
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);


    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        printf("VLAN_MGR_AddRspanUntagPortMember: vid %d, port %d, status %d\n",(UI16_T)vid,(UI16_T) lport_ifindex, (UI16_T)vlan_status);

    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddUntagPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    if ( vlan_info.rspan_status != VAL_vlanStaticExtRspanStatus_rspanVlan )
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN_MGR_AddRspanUntagPortMember doesn't add untagged members into non-rspan vlan."
                                );

        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    vlan_port_info.lport_ifindex = lport_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddUntagPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    member_tag_changed = FALSE;
    if (    VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex)
         && !VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex)
       )
    {
        member_tag_changed = TRUE;
    }

    pre_member_type = VLAN_MGR_GetPortMemberType(&vlan_info, &vlan_port_info);

    if (!VLAN_MGR_AddVlanMember(vlan_info.dot1q_vlan_index,lport_ifindex, VLAN_MGR_BOTH, vlan_status))
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            printf("ERROR: VLAN_MGR_AddVlanMember in VLAN_MGR_AddRspanUntagPortMember\n");
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    VLAN_OM_SetIfStackLastUpdateTime(vlan_info.dot1q_vlan_time_mark);

    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddUntagPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddUntagPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    post_member_type = VLAN_MGR_GetPortMemberType(&vlan_info, &vlan_port_info);

    if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
    {
        /* generate callback when port does not previously existed in vlan membership.
         */
        if (    (pre_member_type == VLAN_TYPE_VLAN_STATUS_NONE)
             && (post_member_type != VLAN_TYPE_VLAN_STATUS_NONE)
           )
        {
            VLAN_MGR_Notify_VlanMemberAdd(vid_ifindex,lport_ifindex, vlan_status);
        }
#if 0
        /* generate callback when port is changed from one member type to another member type
         */
        if (    (pre_member_type != VLAN_TYPE_VLAN_STATUS_NONE)
             && (post_member_type != VLAN_TYPE_VLAN_STATUS_NONE)
             && (post_member_type != pre_member_type)
           )
        {
            VLAN_MGR_Notify_VlanMemberTypeChanged(vid_ifindex, lport_ifindex, vlan_status);
        }
#endif

        if (member_tag_changed)
        {
            VLAN_MGR_Notify_VlanMemberTagChanged(vid_ifindex, lport_ifindex);
        }
    } /* End of if (_vlan_active_) */

#if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE)
    if (    (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_access)
         && (!VLAN_MGR_ConformToSingleUntaggedVlan(vid, lport_ifindex, vlan_status))
       )
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
#endif /* #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK == TRUE) */

#if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK == TRUE)
    if (    (vlan_port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_hybrid)
         && (!VLAN_MGR_ConformToSingleUntaggedVlan(vid, lport_ifindex, vlan_status))
       )
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
#endif /* #if (SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK == TRUE) */

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);
}

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
BOOL_T VLAN_MGR_AddRspanEgressPortMember(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    UI32_T                              vid_ifindex;
    UI32_T                              pre_member_type, post_member_type;
    BOOL_T                              member_tag_changed;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    /* Returns error if pre-condition to add vlan member can not be satisfy
     */
    if (!VLAN_MGR_PreconditionForSetVlanInfo(vid, lport_ifindex))
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        printf("VLAN_MGR_AddRspanEgressPortMember: vid %d, port %d, status %d\n",(UI16_T)vid, (UI16_T)lport_ifindex, (UI16_T)vlan_status);

    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    if ( vlan_info.rspan_status != VAL_vlanStaticExtRspanStatus_rspanVlan )
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN_MGR_AddRspanUntagPortMember doesn't add tagged members into non-rspan vlan."
                                );

        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    vlan_port_info.lport_ifindex = lport_ifindex;
    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    member_tag_changed = FALSE;
    if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
    {
        member_tag_changed = TRUE;
    }

    pre_member_type = VLAN_MGR_GetPortMemberType(&vlan_info, &vlan_port_info);

    if (!VLAN_MGR_AddVlanMember(vlan_info.dot1q_vlan_index,lport_ifindex,VLAN_MGR_TAGGED_ONLY,vlan_status))
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            printf("ERROR: VLAN_MGR_AddVlanMember in VLAN_MGR_AddRspanEgressPortMember\n");
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    VLAN_OM_SetIfStackLastUpdateTime(vlan_info.dot1q_vlan_time_mark);

    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    if (!VLAN_OM_GetVlanPortEntry(&vlan_port_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN port entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    post_member_type = VLAN_MGR_GetPortMemberType(&vlan_info, &vlan_port_info);

    if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
    {
        /* generate callback when port does not previously existed in vlan membership.
         */
        if (    (pre_member_type == VLAN_TYPE_VLAN_STATUS_NONE)
             && (post_member_type != VLAN_TYPE_VLAN_STATUS_NONE)
           )
        {
            VLAN_MGR_Notify_VlanMemberAdd(vid_ifindex, lport_ifindex, vlan_status);
        }
#if 0
        /* generate callback when port is changed from one member type to another member type
         */
        if (    (pre_member_type != VLAN_TYPE_VLAN_STATUS_NONE)
             && (post_member_type != VLAN_TYPE_VLAN_STATUS_NONE)
             && (post_member_type != pre_member_type)
           )
        {
            VLAN_MGR_Notify_VlanMemberTypeChanged(vid_ifindex, lport_ifindex, vlan_status);
        }
#endif

        if (member_tag_changed)
        {
            VLAN_MGR_Notify_VlanMemberTagChanged(vid_ifindex, lport_ifindex);
        }
    } /* End of if (_vlan_active_) */

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);
}

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
 * NOTES    : The API is only for RSPAN MGR.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_DeleteRspanUntagPortMember(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    UI32_T                              vid_ifindex;
    BOOL_T                              member_tag_changed;

    /* BODY */

    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        printf("VLAN_MGR_DeleteRspanUntagPortMember: vid %d, port %d, status %d\n",(UI16_T)vid, (UI16_T)lport_ifindex, (UI16_T)vlan_status);

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    /* Returns error if pre-condition to add vlan member can not be satisfy
     */
    if (!VLAN_MGR_PreconditionForSetVlanInfo(vid, lport_ifindex))
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

    /* Allow user to set tag/untag for a private_port */
    if (    (!VLAN_MGR_IsPrivateVlanPortMember(vid_ifindex, lport_ifindex))
         && (!VLAN_MGR_PortRemovable(vid_ifindex, lport_ifindex))
       )
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    } /* end of if */

    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddUntagPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    if ( vlan_info.rspan_status != VAL_vlanStaticExtRspanStatus_rspanVlan )
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN_MGR_AddRspanUntagPortMember doesn't delete untagged members from non-rspan vlan."
                                );

        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    member_tag_changed = FALSE;
    if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_untagged_ports, lport_ifindex))
    {
        member_tag_changed = TRUE;
    }

    if (!VLAN_MGR_RemoveVlanMember(vid_ifindex,lport_ifindex,VLAN_MGR_UNTAGGED_ONLY,L_RSTATUS_ACTIVE_2_NOTEXIST,vlan_status))
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            printf("ERROR: VLAN_MGR_RemoveVlanMember in VLAN_MGR_DeleteRspanUntagPortMember\n");
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    SYS_TIME_GetSystemUpTimeByTick(VLAN_OM_GetIfStackLastUpdateTimePtr());

    if (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
    {
        if (member_tag_changed)
        {
            VLAN_MGR_Notify_VlanMemberTagChanged(vid_ifindex, lport_ifindex);
        }
    }

#if (SYS_CPNT_VLAN_PROVIDING_AT_LEAST_ONE_UNTAGGED_MODE == TRUE)
    if (VLAN_MGR_ConformToAtLeastOneUntaggedVlan(lport_ifindex) == FALSE)
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
#endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_AT_LEAST_ONE_UNTAGGED_MODE == TRUE) */

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);
}

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
 * NOTES    : The API is only for RSPAN MGR.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_MGR_DeleteRspanEgressPortMember(UI32_T vid, UI32_T lport_ifindex, UI32_T vlan_status)
{
    UI32_T          vid_ifindex;
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    BOOL_T                              is_member;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

#if(SYS_CPNT_CLUSTER == TRUE)
    if(vid == SYS_DFLT_CLUSTER_DEFAULT_VLAN)
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
#endif

    if (!VLAN_MGR_PreconditionForSetVlanInfo(vid, lport_ifindex))
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        printf("VLAN_MGR_DeleteRspanEgressPortMember: vid %d, port %d, status %d\n",(UI16_T)vid, (UI16_T)lport_ifindex, (UI16_T)vlan_status);

    /* vid is stored in the corresponding ifindex in the vlan database.
     */
    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

    /* vid_ifindx is the key to search for vlan_info in the database
     */
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    /* returns false if vlan_info can not be retrieve from the database
     */
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DeleteEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* End of if */

    if (vlan_info.rspan_status != VAL_vlanStaticExtRspanStatus_rspanVlan)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN_MGR_AddRspanUntagPortMember doesn't deleted tagged members from non-rspan vlan."
                                );

        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    is_member = VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex);
    if (    (!is_member)
         && (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_static_egress_ports, lport_ifindex))
       )
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, TRUE);
    }

    /* Dynamic vlan information will not be save to startup file. Therefore,
       join command from user configuration should also change vlan status from
       dynamic to static to solve the issue.  Otherwise, inconsistency will occur
       when the device reboot.
     */
    if ((vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_dynamicGvrp) &&
        (vlan_status == VAL_dot1qVlanStatus_permanent))
    {
        vlan_info.dot1q_vlan_status = VAL_dot1qVlanStatus_permanent;
    } /* end of if */

#if (SYS_DFLT_VLAN_CHECK_AT_LEAST_ONE_MEMBER_IN_MANAGEMEMT_VLAN == TRUE)
    /* Only when static remove,
       need to check the last static vlan member of management VLAN can not be removed.
     */
    if (    (vlan_status == VAL_dot1qVlanStatus_permanent)
         && (vid == management_vlan_id)
         && (VLAN_MGR_CountVlanMember(vlan_info.dot1q_vlan_static_egress_ports) == 1)
       )
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_VLAN,
                                 VLAN_MGR_DeleteEgressPortMember_Fun_No,
                                 EH_TYPE_MSG_FAILED_TO_DELETE,
                                 SYSLOG_LEVEL_INFO,
                                 "the last port member of management VLAN"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }
#endif /* end of #if (SYS_DFLT_VLAN_CHECK_AT_LEAST_ONE_MEMBER_IN_MANAGEMEMT_VLAN == TRUE) */

    /* Allow user to set tag/untag for a private_port */
    if (    VLAN_MGR_IsPrivateVlanPortMember(vid_ifindex, lport_ifindex)
         || VLAN_MGR_PortRemovable(vid_ifindex, lport_ifindex)
       )
    {
        if (!VLAN_MGR_RemoveVlanMember(vlan_info.dot1q_vlan_index,lport_ifindex,VLAN_MGR_BOTH,L_RSTATUS_ACTIVE_2_NOTEXIST,vlan_status))
        {
            if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                printf("ERROR: VLAN_MGR_RemoveVlanMember in VLAN_MGR_DeleteRspanEgressPortMember\n");
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
        } /* end of if */

    } /* end of if */
    else
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    VLAN_OM_SetIfStackLastUpdateTime(vlan_info.dot1q_vlan_time_mark);

    /* Refresh vlan_info since VLAN_MGR_RemoveVlanMember() may change vlan_operation_status.
     */
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DeleteEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* End of if */

    if (vlan_info.rspan_status != VAL_vlanStaticExtRspanStatus_rspanVlan)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN_MGR_AddRspanUntagPortMember doesn't delete tagged members from non-rspan vlan."
                                );

        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    if (    (vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_active)
        &&  (is_member)
        &&  (!VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, lport_ifindex))
       )
    {
        VLAN_MGR_Notify_VlanMemberDelete(vid_ifindex,lport_ifindex, vlan_status);
    }

#if (SYS_CPNT_VLAN_PROVIDING_AT_LEAST_ONE_UNTAGGED_MODE == TRUE)
    if (VLAN_MGR_ConformToAtLeastOneUntaggedVlan(lport_ifindex) == FALSE)
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
#endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_AT_LEAST_ONE_UNTAGGED_MODE == TRUE) */

    if (!VLAN_MGR_ProcessOrphanPortByLport(lport_ifindex))
    {
        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
            printf("ERROR: VLAN_MGR_ProcessOrphanPortByLport in VLAN_MGR_DeleteRspanEgressPortMember\n");
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }
    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);
}

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
BOOL_T VLAN_MGR_DeleteRspanVlan(UI32_T vid, UI32_T vlan_status)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    UI32_T                              vid_ifindex;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);

    memset (&vlan_info, 0, sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DeleteNormalVlan_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    if (vlan_info.rspan_status != VAL_vlanStaticExtRspanStatus_rspanVlan)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DeleteNormalVlan_Fun_No,
                                    EH_TYPE_MSG_FAILED_TO_DELETE,
                                    SYSLOG_LEVEL_INFO,
                                    "the RSPAN vlan using the vlan command."
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    if (!VLAN_MGR_DeleteRspanVlanWithoutMembers(vid, vlan_status))
    {
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
    }

    if (vid == VLAN_OM_GetManagementVlanId())
    {
        VLAN_MGR_SetGlobalManagementVlan(SYS_DFLT_SWITCH_MANAGEMENT_VLAN);
    }

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_DeleteRspanVlanWithoutMembers
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific RSPAN vlan is successfully
 *            deleted from the database.  Otherwise, false is returned.
 * INPUT    : vid   -- the existed vlan id
 *            vlan_status -- VAL_dot1qVlanStatus_permanent
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : None
 *------------------------------------------------------------------------------*/
static BOOL_T VLAN_MGR_DeleteRspanVlanWithoutMembers(UI32_T vid, UI32_T vlan_status)
{
    UI32_T      vid_ifindex, port_num;
    VLAN_OM_Dot1qVlanCurrentEntry_T         vlan_info;

    /* BODY */
#if 0
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
#endif
    if (VLAN_Debug(VLAN_DEBUG_FLAG_VLAN))
        printf("VLAN_MGR_DeleteRspanVlan: vid %d, status %d\n",(UI16_T)vid,(UI16_T) vlan_status);

    /* Default vlan and vid beyond maximum vlan id range
       can not be deleted.
     */
    if (vid < 1 || vid > SYS_ADPT_MAX_VLAN_ID || vid == VLAN_OM_GetGlobalDefaultVlan())
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DeleteVlan_Fun_No,
                                    EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                    SYSLOG_LEVEL_INFO, "VLAN ID"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    /* vid is stored in the corresponding ifindex in the vlan database.
     */
    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

    /* Key to search for specific vlan_info in the database.
     */
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    /* returns error if vlan_info can not be retrieve successfully from the database.
     */
    if (!VLAN_OM_GetVlanEntry(&vlan_info))
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_DeleteVlan_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN entry"
                                );
        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    } /* end of if */

    if (vlan_info.rspan_status != VAL_vlanStaticExtRspanStatus_rspanVlan)
    {
        EH_MGR_Handle_Exception1(   SYS_MODULE_VLAN,
                                    VLAN_MGR_AddEgressPortMember_Fun_No,
                                    EH_TYPE_MSG_NOT_EXIST,
                                    SYSLOG_LEVEL_INFO,
                                    "VLAN_MGR_AddRspanUntagPortMember doesn't delete non-rspan vlan."
                                );

        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
    }

    switch (L_RSTATUS_Fsm(VAL_dot1qVlanStaticRowStatus_destroy,
                          &vlan_info.dot1q_vlan_static_row_status,
                          VLAN_MGR_SemanticCheck,
                          (void*)&vlan_info))
    {
        /* Remove entry from Database
         */
        case L_RSTATUS_ACTIVE_2_NOTREADY:
            if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                printf("ERROR: L_RSTATUS_ACTIVE_2_NOTREADY in VLAN_MGR_DeleteRspanVlan\n");

            break;

        /* Delete a suspended vlan
         */
        case L_RSTATUS_NOTREADY_2_NOTEXIST:

            /* Don't need to check "VLAN_MGR_VlanRemovable(vid)" because RSPAN VLAN can't be a pvid. Tien, 2007/07/31. */

            port_num    = 0;
            while (SWCTRL_GetNextLogicalPort(&port_num) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, port_num))
                {
                    /* Can't delete RSPAN VLAN; unless users removes the binding of remote vid for a RSPAN session. */
                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
#if 0               /* Don't consider this situation for now. Tien, 2007/07/31. */
                    if (!VLAN_MGR_ProcessOrphanPortByLport(port_num))
                    {
                        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                            printf("ERROR: VLAN_MGR_ProcessOrphanPortByLport in VLAN_MGR_DeleteVlan\n");
                        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
                    }
#endif
                }
            } /* end of for */

            if (!VLAN_OM_DeleteVlanEntry(vid))
            {
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
            } /* end of if */
            SYS_TIME_GetSystemUpTimeByTick(VLAN_OM_GetIfStackLastUpdateTimePtr());
            VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);

            break;

        /* Remove entry from database and ASIC.
         */
        case L_RSTATUS_ACTIVE_2_NOTEXIST:

            /* Don't need to check "VLAN_MGR_VlanRemovable(vid)" because RSPAN VLAN can't be a pvid. Tien, 2007/07/31. */

            port_num    = 0;
            while (SWCTRL_GetNextLogicalPort(&port_num) != SWCTRL_LPORT_UNKNOWN_PORT)
            {
                if (VLAN_OM_IS_MEMBER(vlan_info.dot1q_vlan_current_egress_ports, port_num))
                {
                    /* Can't delete RSPAN VLAN; unless users removes the binding of remote vid for a RSPAN session. */
                    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_ON, FALSE);
#if 0               /* Don't consider this situation for now. Tien, 2007/07/31. */
                    if (!VLAN_MGR_ProcessOrphanPortByLport(port_num))
                    {
                        if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                            printf("ERROR: VLAN_MGR_ProcessOrphanPortByLport in VLAN_MGR_DeleteEgressPortMember\n");
                        VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
                    }
#endif
                }
            } /* end of for */

            if (VLAN_MGR_RemoveVlan(vlan_info.dot1q_vlan_index,L_RSTATUS_ACTIVE_2_NOTEXIST))
            {
                VLAN_MGR_Notify_VlanDestroy(&vlan_info, vlan_status);
                VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, TRUE);
            } /* end of if */
            else
            {
                if (VLAN_Debug(VLAN_DEBUG_FLAG_ERRMSG))
                    printf("ERROR: VLAN_OM_DeleteRSPANVlanEntry in VLAN_MGR_DeleteRspanVlan for case Active->NotExist\n");
            }
            break;

        default:

            VLAN_MGR_ReportSyslogMessage(SYSLOG_LEVEL_ERR, SWITCH_TO_DEFAULT_MESSAGE_INDEX, "VLAN_MGR_DeleteRspanVlan");
            break;
    } /* end of switch */

    VLAN_MGR_RELEASE_CSC(VLAN_MGR_CS_FLAG_OFF, FALSE);
}
#endif /* end #if (SYS_CPNT_RSPAN == TRUE) */

/* FUNCTION NAME - VLAN_MGR_AddVlanMemberToChip
 * PURPOSE : Set a VLAN port member addition into chip (through SWCTRL).
 * INPUT   : lport    - the specified logical port
 *           vid      - the identifier of the specified VLAN
 *           untagged - TRUE(untagged set), FALSE(member set)
 *           check    - flag to indicate SWCTRL to check if lport is existing
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : The port must be member of current list of the VLAN before calling.
 */
static BOOL_T VLAN_MGR_AddVlanMemberToChip(UI32_T lport, UI32_T vid,
                                           BOOL_T untagged, BOOL_T check)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    VLAN_OM_Vlan_Port_Info_T    vlan_port_info;

    /* BODY
     */

    vlan_port_info.lport_ifindex = lport;
    if (VLAN_OM_GetVlanPortEntry(&vlan_port_info) == FALSE)
    {
        return FALSE;
    }

    if (untagged == TRUE)
    {
        /* driver supports adding to member set automatically */
        if (SWCTRL_AddPortToVlanUntaggedSet(lport, vid, check) == FALSE)
        {
            return FALSE;
        }
    }
    else
    {
        if (SWCTRL_AddPortToVlanMemberSet(lport, vid, check) == FALSE)
        {
            return FALSE;
        }
    }

    return TRUE;
} /* End of VLAN_MGR_AddVlanMemberToChip */

